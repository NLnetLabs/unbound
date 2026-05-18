/**
 * \file
 * This file implements the ipset/nftset module.  It can handle packets by
 * putting the A and AAAA addresses that are configured in unbound.conf as
 * type ipset (local-zone statements) into a firewall set.  For firewall
 * blacklist and whitelist usage.
 *
 * The same module entry points serve two backends, distinguished by the
 * section name in unbound.conf:
 *   ipset:   legacy iptables ipset on Linux, or PF tables on BSD.
 *   nftset:  nftables sets on Linux (Linux-only).
 *
 * Both Linux backends speak netlink directly via libmnl.  Sends are
 * fire-and-forget; after every send the kernel error queue is drained
 * non-blocking and any reported errors are logged.
 */
#include "config.h"
#include "ipset/ipset.h"
#include "util/regional.h"
#include "util/net_help.h"
#include "util/config_file.h"

#include "services/cache/dns.h"

#include "sldns/sbuffer.h"
#include "sldns/wire2str.h"
#include "sldns/parseutil.h"

#if defined(HAVE_NET_PFVAR_H) && defined(USE_NFTSET)
#error "nftset cannot be compiled with PF (BSD) support. nftset is Linux-only."
#endif

#ifdef HAVE_NET_PFVAR_H
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/pfvar.h>
typedef intptr_t filter_dev;
#else
#include <libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#ifdef USE_IPSET
#include <linux/netfilter/ipset/ip_set.h>
#endif
#ifdef USE_NFTSET
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#endif
typedef struct mnl_socket * filter_dev;

/* NETLINK_EXT_ACK and the NLMSGERR attributes were added in Linux 4.12.
 * Provide fallbacks so older build environments still compile. */
#ifndef NETLINK_EXT_ACK
#define NETLINK_EXT_ACK 11
#endif
#ifndef NLM_F_CAPPED
#define NLM_F_CAPPED 0x100
#endif
#ifndef NLM_F_ACK_TLVS
#define NLM_F_ACK_TLVS 0x200
#endif
#ifndef NLMSGERR_ATTR_MSG
#define NLMSGERR_ATTR_MSG 1
#endif

/* A 2KB stack buffer by far large enough to hold the netlink request messages
 * that we build, but additionally needs to be large enough to hold the the 
 * netlink NLMSG_ERROR that may be produced by netlink_drain_errors.
 * This includes the entire original message plus an error string. */
#define NETLINK_BUFF_LEN 2048
#endif

#define DNAME_BUFF_LEN 256

/**
 * Return an error
 * @param qstate: our query state
 * @param id: module id
 * @param rcode: error code (DNS errcode).
 * @return: 0 for use by caller, to make notation easy, like:
 * 	return error_response(..).
 */
static int error_response(struct module_qstate* qstate, int id, int rcode) {
	verbose(VERB_QUERY, "return error response %s",
		sldns_lookup_by_id(sldns_rcodes, rcode)?
		sldns_lookup_by_id(sldns_rcodes, rcode)->name:"??");
	qstate->return_rcode = rcode;
	qstate->return_msg = NULL;
	qstate->ext_state[id] = module_finished;
	return 0;
}

#ifdef HAVE_NET_PFVAR_H
static void * open_filter(void) {
	filter_dev dev;

	dev = open("/dev/pf", O_RDWR);
	if (dev == -1) {
		log_err("open(\"/dev/pf\") failed: %s", strerror(errno));
		return NULL;
	}
	else
		return (void *)dev;
}
#else
static void * open_filter(void) {
	filter_dev dev;
	int on = 1;

	dev = mnl_socket_open(NETLINK_NETFILTER);
	if (!dev) {
		log_err("ipset: could not open netfilter.");
		return NULL;
	}

	if (mnl_socket_bind(dev, 0, MNL_SOCKET_AUTOPID) < 0) {
		mnl_socket_close(dev);
		log_err("ipset: could not bind netfilter.");
		return NULL;
	}

	/* Ask the kernel for a human-readable error string when an add is
	 * rejected.  Best-effort: ignore failure on older kernels. */
	(void)setsockopt(mnl_socket_get_fd(dev), SOL_NETLINK,
		NETLINK_EXT_ACK, &on, sizeof(on));

	return (void *)dev;
}

/* Drain any pending kernel replies on the netlink socket non-blocking and
 * log any NLMSG_ERROR messages.  Called after every send.  Best-effort:
 * caller / per-message context is not preserved, just the kernel error
 * string is enough for diagnosing misconfiguration. */
static void
netlink_drain_errors(filter_dev dev, char* buf, size_t buflen)
{
       int fd = mnl_socket_get_fd(dev);
       ssize_t r;
       int n;
       struct nlmsghdr *nlh;

       for (;;) {
               r = recv(fd, buf, buflen, MSG_DONTWAIT);
               if (r < 0) {
                       if (errno == EINTR)
                               continue;
                       break;
               }
               if (r == 0)
                       break;
               if ((size_t)r == buflen) {
                       log_warn("ipset: netlink error report possibly truncated");
               }
               n = (int)r;
               for (nlh = (struct nlmsghdr *)buf; mnl_nlmsg_ok(nlh, n); nlh = mnl_nlmsg_next(nlh, &n)) {
                       struct nlmsgerr *e;
                       const char *msg = NULL;
                       size_t hlen, total, alen;
                       void *start;
                       struct nlattr *attr;

                       if (nlh->nlmsg_type != NLMSG_ERROR)
                               continue;

                       e = mnl_nlmsg_get_payload(nlh);
                       if (!e->error)
                               continue;

                       hlen = sizeof(*e);
                       total = mnl_nlmsg_get_payload_len(nlh);
                       if (!(nlh->nlmsg_flags & NLM_F_CAPPED))
                               hlen += mnl_nlmsg_get_payload_len(&e->msg);

                       if ((nlh->nlmsg_flags & NLM_F_ACK_TLVS) && hlen < total) {
                               start = (char *)e + hlen;
                               alen = total - hlen;
                               mnl_attr_for_each_payload(start, alen) {
                                       if (mnl_attr_get_type(attr) == NLMSGERR_ATTR_MSG) {
                                               msg = mnl_attr_get_str(attr);
                                               break;
                                       }
                               }
                       }
                       log_err("ipset: kernel reported error: %s%s%s",
                               strerror(-e->error),
                               msg ? ": " : "",
                               msg ? msg : "");
               }
       }
}
#endif

#ifndef HAVE_NET_PFVAR_H
static struct nlmsghdr *
netlink_put_hdr(char *buf, uint16_t type, uint16_t family, uint16_t flags,
	uint32_t seq, uint16_t res_id)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = NLM_F_REQUEST | flags;
	nlh->nlmsg_seq = seq;

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = family;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = htons(res_id);

	return nlh;
}
#endif

#if defined(HAVE_NET_PFVAR_H) && defined(USE_IPSET)
static int add_to_ipset(filter_dev dev, const char *setname, const void *ipaddr, int af) {
	struct pfioc_table io;
	struct pfr_addr addr;
	const char *p;
	int i;

	bzero(&io, sizeof(io));
	bzero(&addr, sizeof(addr));

	p = strrchr(setname, '/');
	if (p) {
		i = p - setname;
		if (i >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return -1;
		}
		memcpy(io.pfrio_table.pfrt_anchor, setname, i);
		if (i < PATH_MAX)
			io.pfrio_table.pfrt_anchor[i] = '\0';
		p++;
	}
	else
		p = setname;

	if (strlen(p) >= PF_TABLE_NAME_SIZE) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strlcpy(io.pfrio_table.pfrt_name, p, PF_TABLE_NAME_SIZE);

	io.pfrio_buffer = &addr;
	io.pfrio_size = 1;
	io.pfrio_esize = sizeof(addr);

	switch (af) {
		case AF_INET:
			addr.pfra_ip4addr = *(struct in_addr *)ipaddr;
			addr.pfra_net = 32;
			break;
		case AF_INET6:
			addr.pfra_ip6addr = *(struct in6_addr *)ipaddr;
			addr.pfra_net = 128;
			break;
		default:
		errno = EAFNOSUPPORT;
		return -1;
	}
	addr.pfra_af = af;

	if (ioctl(dev, DIOCRADDADDRS, &io) == -1) {
		log_err("ioctl failed: %s", strerror(errno));
		return -1;
	}
	return 0;
}
#elif defined(USE_IPSET)
static int add_to_ipset(filter_dev dev, const char *setname, const void *ipaddr, int af) {
	struct nlmsghdr *nlh;
	struct nlattr *nested[2];
	char buffer[NETLINK_BUFF_LEN];

	if (strlen(setname) >= IPSET_MAXNAMELEN) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (af != AF_INET && af != AF_INET6) {
		errno = EAFNOSUPPORT;
		return -1;
	}

	nlh = netlink_put_hdr(buffer, IPSET_CMD_ADD | (NFNL_SUBSYS_IPSET << 8),
		af, NLM_F_ACK | NLM_F_EXCL, 0, 0);

	mnl_attr_put_u8(nlh, IPSET_ATTR_PROTOCOL, IPSET_PROTOCOL);
	mnl_attr_put(nlh, IPSET_ATTR_SETNAME, strlen(setname) + 1, setname);
	nested[0] = mnl_attr_nest_start(nlh, IPSET_ATTR_DATA);
	nested[1] = mnl_attr_nest_start(nlh, IPSET_ATTR_IP);
	mnl_attr_put(nlh, (af == AF_INET ? IPSET_ATTR_IPADDR_IPV4 : IPSET_ATTR_IPADDR_IPV6)
			| NLA_F_NET_BYTEORDER, (af == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr)), ipaddr);
	mnl_attr_nest_end(nlh, nested[1]);
	mnl_attr_nest_end(nlh, nested[0]);

	if (mnl_socket_sendto(dev, nlh, nlh->nlmsg_len) < 0) {
		return -1;
	}
	netlink_drain_errors(dev, buffer, sizeof(buffer));
	return 0;
}
#endif /* USE_IPSET */

#ifdef USE_NFTSET
static int nft_parse_family(const char *name) {
	if (!name || !name[0])         return NFPROTO_INET;
	if (strcmp(name, "inet") == 0) return NFPROTO_INET;
	if (strcmp(name, "ip")   == 0) return NFPROTO_IPV4;
	if (strcmp(name, "ip6")  == 0) return NFPROTO_IPV6;
	return -1;
}

static int add_to_nftset(filter_dev dev, const char *table,
	const char *setname, const void *ipaddr, int af, int nfproto,
	uint32_t *seq)
{
	char buffer[NETLINK_BUFF_LEN];
	struct nlmsghdr *nlh;
	struct nlattr *nested[3];
	size_t off = 0, addr_size;
	uint8_t end_addr[sizeof(struct in6_addr)];
	int i, overflow;

	if (!table || !table[0] || !setname || !setname[0]) {
		errno = EINVAL;
		return -1;
	}
	if (strlen(table) >= NFT_TABLE_MAXNAMELEN ||
		strlen(setname) >= NFT_SET_MAXNAMELEN) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (af == AF_INET) {
		addr_size = sizeof(struct in_addr);
	} else if (af == AF_INET6) {
		addr_size = sizeof(struct in6_addr);
	} else {
		errno = EAFNOSUPPORT;
		return -1;
	}

	/* Compute exclusive interval end (ipaddr + 1, network byte order). */
	memcpy(end_addr, ipaddr, addr_size);
	overflow = 1;
	for (i = (int)addr_size - 1; i >= 0 && overflow; i--) {
		overflow += (unsigned char)end_addr[i];
		end_addr[i] = overflow & 0xff;
		overflow >>= 8;
	}

	nlh = netlink_put_hdr(buffer, NFNL_MSG_BATCH_BEGIN, NFPROTO_UNSPEC,
		0, (*seq)++, NFNL_SUBSYS_NFTABLES);
	off += nlh->nlmsg_len;

	nlh = netlink_put_hdr(buffer + off,
		(NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWSETELEM,
		nfproto, NLM_F_CREATE | NLM_F_ACK, (*seq)++, 0);

	mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_TABLE, table);
	mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_SET, setname);

	nested[0] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_LIST_ELEMENTS);

	nested[1] = mnl_attr_nest_start(nlh, NLA_F_NESTED | 1);
	nested[2] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_KEY);
	mnl_attr_put(nlh, NFTA_DATA_VALUE | NLA_F_NET_BYTEORDER,
		addr_size, ipaddr);
	mnl_attr_nest_end(nlh, nested[2]);
	mnl_attr_nest_end(nlh, nested[1]);

	/* Interval sets need an explicit exclusive end (ipaddr+1, INTERVAL_END
	 * flag); without it the kernel creates an open-ended interval.
	 * If the addition overflows (e.g., 255.255.255.255 + 1), the interval
	 * inherently ends at the boundary of the address space, so no explicit
	 * INTERVAL_END is needed. */
	if (!overflow) {
		nested[1] = mnl_attr_nest_start(nlh, NLA_F_NESTED | 2);
		mnl_attr_put_u32(nlh, NFTA_SET_ELEM_FLAGS,
			htonl(NFT_SET_ELEM_INTERVAL_END));
		nested[2] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_KEY);
		mnl_attr_put(nlh, NFTA_DATA_VALUE | NLA_F_NET_BYTEORDER,
			addr_size, end_addr);
		mnl_attr_nest_end(nlh, nested[2]);
		mnl_attr_nest_end(nlh, nested[1]);
	}

	mnl_attr_nest_end(nlh, nested[0]);
	off += nlh->nlmsg_len;

	nlh = netlink_put_hdr(buffer + off, NFNL_MSG_BATCH_END, NFPROTO_UNSPEC,
		0, (*seq)++, NFNL_SUBSYS_NFTABLES);
	off += nlh->nlmsg_len;

	if (mnl_socket_sendto(dev, buffer, off) < 0) {
		return -1;
	}
	netlink_drain_errors(dev, buffer, sizeof(buffer));
	return 0;
}
#endif /* USE_NFTSET */

static void
ipset_add_rrset_data(struct ipset_env *ie,
	struct packed_rrset_data *d, const char* setname, int af,
	const char* dname)
{
	int ret;
	size_t j, rr_len, rd_len;
	uint8_t *rr_data;

	/* to d->count, not d->rrsig_count, because we do not want to add the RRSIGs, only the addresses */
	for (j = 0; j < d->count; j++) {
		rr_len = d->rr_len[j];
		rr_data = d->rr_data[j];

		rd_len = sldns_read_uint16(rr_data);
		if(af == AF_INET && rd_len != INET_SIZE)
			continue;
		if(af == AF_INET6 && rd_len != INET6_SIZE)
			continue;
		if (rr_len - 2 >= rd_len) {
			if(verbosity >= VERB_QUERY) {
				char ip[128];
				if(inet_ntop(af, rr_data+2, ip, (socklen_t)sizeof(ip)) == 0)
					snprintf(ip, sizeof(ip), "(inet_ntop_error)");
				verbose(VERB_QUERY, "ipset: add %s to %s for %s", ip, setname, dname);
			}
#ifdef USE_NFTSET
			if (ie->use_nft) {
				ret = add_to_nftset((filter_dev)ie->dev,
					ie->table, setname, rr_data + 2,
					af, ie->nfproto, &ie->seq);
			} else
#endif
			{
#ifdef USE_IPSET
				ret = add_to_ipset((filter_dev)ie->dev,
					setname, rr_data + 2, af);
#else
				(void)setname;
				ret = -1;
#endif
			}
			if (ret < 0) {
				log_err("ipset: could not add %s into %s", dname, setname);

#if HAVE_NET_PFVAR_H
				/* don't close as we might not be able to open again due to dropped privs */
#else
				mnl_socket_close((filter_dev)ie->dev);
				ie->dev = NULL;
#endif
				break;
			}
		}
	}
}

static int
ipset_check_zones_for_rrset(struct module_env *env, struct ipset_env *ie,
	struct ub_packed_rrset_key *rrset, const char *qname, int qlen,
	const char *setname, int af)
{
	char dname[DNAME_BUFF_LEN];
	const char *ds, *qs;
	int dlen, plen;

	struct config_strlist *p;
	struct packed_rrset_data *d;

	dlen = sldns_wire2str_dname_buf(rrset->rk.dname, rrset->rk.dname_len, dname, DNAME_BUFF_LEN);
	if (dlen == 0) {
		log_err("bad domain name");
		return -1;
	}
	if (dlen > 0 && dname[dlen - 1] == '.') {
		dlen--;
	}
	if (qlen > 0 && qname[qlen - 1] == '.') {
		qlen--;
	}

	for (p = env->cfg->local_zones_ipset; p; p = p->next) {
		ds = NULL;
		qs = NULL;
		plen = strlen(p->str);
		if (plen > 0 && p->str[plen - 1] == '.') {
			plen--;
		}

		if (dlen == plen || (dlen > plen && dname[dlen - plen - 1] == '.' )) {
			ds = dname + (dlen - plen);
		}
		if (qlen == plen || (qlen > plen && qname[qlen - plen - 1] == '.' )) {
			qs = qname + (qlen - plen);
		}
		if ((ds && strncasecmp(p->str, ds, plen) == 0)
			|| (qs && strncasecmp(p->str, qs, plen) == 0)) {
			const char *use_setname = setname; /* global fallback */
			struct config_str3list *zp;
			int zplen;
			/* Check for a per-zone set: entry matching this zone */
			for (zp = env->cfg->ipset_zones; zp; zp = zp->next) {
				zplen = strlen(zp->str);
				if (zplen > 0 && zp->str[zplen - 1] == '.') zplen--;
				if (plen == zplen &&
					strncasecmp(p->str, zp->str, plen) == 0) {
					use_setname = (af == AF_INET) ?
						zp->str2 : zp->str3;
					break;
				}
			}
			if (use_setname && strlen(use_setname) > 0) {
				d = (struct packed_rrset_data*)rrset->entry.data;
				ipset_add_rrset_data(ie, d, use_setname, af, dname);
			}
			break;
		}
	}
	return 0;
}

static int ipset_update(struct module_env *env, struct dns_msg *return_msg,
	struct query_info qinfo, struct ipset_env *ie)
{
	size_t i;
	const char *setname;
	struct ub_packed_rrset_key *rrset;
	int af;
	char qname[DNAME_BUFF_LEN];
	int qlen;

#ifdef HAVE_NET_PFVAR_H
#else
	if (!ie->dev) {
		/* retry to create mnl socket */
		ie->dev = open_filter();
		if (!ie->dev) {
			log_warn("ipset open_filter failed");
			return -1;
		}
	}
#endif

	qlen = sldns_wire2str_dname_buf(qinfo.qname, qinfo.qname_len,
		qname, DNAME_BUFF_LEN);
	if(qlen == 0) {
		log_err("bad domain name");
		return -1;
	}

	for(i = 0; i < return_msg->rep->rrset_count; i++) {
		int type_matched = 0;
		setname = NULL;
		af = 0;
		rrset = return_msg->rep->rrsets[i];
		if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_A &&
			ie->v4_enabled == 1) {
			af = AF_INET;
			setname = ie->name_v4;
			type_matched = 1;
		} else if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_AAAA &&
			ie->v6_enabled == 1) {
			af = AF_INET6;
			setname = ie->name_v6;
			type_matched = 1;
		}

		/* Enter zone lookup when this RRset type is enabled, even if no
		 * global setname is configured — per-zone entries may supply the
		 * set name, and ipset_check_zones_for_rrset() handles NULL
		 * setname (no global fallback). */
		if (type_matched) {
			if(ipset_check_zones_for_rrset(env, ie, rrset, qname,
				qlen, setname, af) == -1)
				return -1;
		}
	}

	return 0;
}

int ipset_startup(struct module_env* env, int id) {
	struct ipset_env *ipset_env;

	ipset_env = (struct ipset_env *)calloc(1, sizeof(struct ipset_env));
	if (!ipset_env) {
		log_err("malloc failure");
		return 0;
	}

	env->modinfo[id] = (void *)ipset_env;

#ifdef HAVE_NET_PFVAR_H
	ipset_env->dev = open_filter();
	if (!ipset_env->dev) {
		log_err("ipset open_filter failed");
		return 0;
	}
#else
	ipset_env->dev = NULL;
#endif
#ifdef USE_NFTSET
	ipset_env->seq = 1;
#endif
	return 1;
}

void ipset_destartup(struct module_env* env, int id) {
	filter_dev dev;
	struct ipset_env *ipset_env;

	if (!env || !env->modinfo[id]) {
		return;
	}
	ipset_env = (struct ipset_env*)env->modinfo[id];

	dev = (filter_dev)ipset_env->dev;
	if (dev) {
#if HAVE_NET_PFVAR_H
		close(dev);
#else
		mnl_socket_close(dev);
#endif
		ipset_env->dev = NULL;
	}

	free(ipset_env);
	env->modinfo[id] = NULL;
}

int ipset_init(struct module_env* env, int id) {
	struct ipset_env *ipset_env = env->modinfo[id];

#ifdef USE_NFTSET
	ipset_env->use_nft = env->cfg->ipset_use_nft;
	if (ipset_env->use_nft) {
		ipset_env->family = env->cfg->ipset_family ?
			env->cfg->ipset_family : "inet";
		ipset_env->table = env->cfg->ipset_table;
		if (!ipset_env->table || !ipset_env->table[0]) {
			log_err("nftset: 'table:' is required");
			return 0;
		}
		ipset_env->nfproto = nft_parse_family(ipset_env->family);
		if (ipset_env->nfproto < 0) {
			log_err("nftset: invalid family '%s' (expected "
				"inet, ip, or ip6)", ipset_env->family);
			return 0;
		}
	} else
#endif
	{
#ifdef USE_IPSET
		if (env->cfg->ipset_family || env->cfg->ipset_table) {
			log_err("ipset: 'family:' and 'table:' are not supported "
				"by the ipset backend");
			return 0;
		}
#else
		log_err("ipset: ip backend not compiled in");
		return 0;
#endif
	}

	ipset_env->name_v4 = env->cfg->ipset_name_v4;
	ipset_env->name_v6 = env->cfg->ipset_name_v6;

	/* Enable based on configuration: any global name OR any per-zone entry
	 * makes the corresponding family active. The per-zone match in
	 * ipset_check_zones_for_rrset() will choose the actual set name. */
	ipset_env->v4_enabled = ((ipset_env->name_v4 &&
		strlen(ipset_env->name_v4) > 0) ||
		env->cfg->ipset_zones) ? 1 : 0;
	ipset_env->v6_enabled = ((ipset_env->name_v6 &&
		strlen(ipset_env->name_v6) > 0) ||
		env->cfg->ipset_zones) ? 1 : 0;

	/* OK if per-zone set: entries are present even without global name-v4/v6 */
	if ((ipset_env->v4_enabled < 1) && (ipset_env->v6_enabled < 1) &&
		!env->cfg->ipset_zones) {
		log_err("ipset: no set names configured; add 'name-v4:'/'name-v6:' "
			"for a global set or 'set: <zone> <v4> <v6>' for per-zone sets");
		return 0;
	}

	return 1;
}

void ipset_deinit(struct module_env *ATTR_UNUSED(env), int ATTR_UNUSED(id)) {
	/* nothing */
}

static int ipset_new(struct module_qstate* qstate, int id) {
	struct ipset_qstate *iq = (struct ipset_qstate *)regional_alloc(
		qstate->region, sizeof(struct ipset_qstate));
	qstate->minfo[id] = iq;
	if (!iq) {
		return 0;
	}

	memset(iq, 0, sizeof(*iq));
	/* initialise it */
	/* TODO */

	return 1;
}

void ipset_operate(struct module_qstate *qstate, enum module_ev event, int id,
	struct outbound_entry *outbound) {
	struct ipset_env *ie = (struct ipset_env *)qstate->env->modinfo[id];
	struct ipset_qstate *iq = (struct ipset_qstate *)qstate->minfo[id];
	verbose(VERB_QUERY, "ipset[module %d] operate: extstate:%s event:%s",
		id, strextstate(qstate->ext_state[id]), strmodulevent(event));
	if (iq) {
		log_query_info(VERB_QUERY, "ipset operate: query", &qstate->qinfo);
	}

	/* perform ipset state machine */
	if ((event == module_event_new || event == module_event_pass) && !iq) {
		if (!ipset_new(qstate, id)) {
			(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
			return;
		}
		iq = (struct ipset_qstate*)qstate->minfo[id];
	}

	if (iq && (event == module_event_pass || event == module_event_new)) {
		qstate->ext_state[id] = module_wait_module;
		return;
	}

	if (iq && (event == module_event_moddone)) {
		if (qstate->return_msg && qstate->return_msg->rep) {
			ipset_update(qstate->env, qstate->return_msg, qstate->qinfo, ie);
		}
		qstate->ext_state[id] = module_finished;
		return;
	}

	if (iq && outbound) {
		/* ipset does not need to process responses at this time
		 * ignore it.
		ipset_process_response(qstate, iq, ie, id, outbound, event);
		*/
		return;
	}

	if (event == module_event_error) {
		verbose(VERB_ALGO, "got called with event error, giving up");
		(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		return;
	}

	if (!iq && (event == module_event_moddone)) {
		/* during priming, module done but we never started */
		qstate->ext_state[id] = module_finished;
		return;
	}

	log_err("bad event for ipset");
	(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
}

void ipset_inform_super(struct module_qstate *ATTR_UNUSED(qstate),
	int ATTR_UNUSED(id), struct module_qstate *ATTR_UNUSED(super)) {
	/* ipset does not use subordinate requests at this time */
	verbose(VERB_ALGO, "ipset inform_super was called");
}

void ipset_clear(struct module_qstate *qstate, int id) {
	if (!qstate) {
		return;
	}
	qstate->minfo[id] = NULL;
}

size_t ipset_get_mem(struct module_env *env, int id) {
	struct ipset_env *ie = (struct ipset_env *)env->modinfo[id];
	if (!ie) {
		return 0;
	}
	return sizeof(*ie);
}

#ifdef USE_IPSET
/**
 * The ipset function block
 */
static struct module_func_block ipset_block = {
	"ipset",
	&ipset_startup, &ipset_destartup, &ipset_init, &ipset_deinit,
	&ipset_operate, &ipset_inform_super, &ipset_clear, &ipset_get_mem
};

struct module_func_block * ipset_get_funcblock(void) {
	return &ipset_block;
}
#endif

#ifdef USE_NFTSET
/**
 * The nftset function block — same functions, different name so that
 * module_factory matches it against module-config: "nftset ...".
 */
static struct module_func_block nftset_block = {
	"nftset",
	&ipset_startup, &ipset_destartup, &ipset_init, &ipset_deinit,
	&ipset_operate, &ipset_inform_super, &ipset_clear, &ipset_get_mem
};

struct module_func_block * nftset_get_funcblock(void) {
	return &nftset_block;
}
#endif