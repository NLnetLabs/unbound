/**
 * \file
 * This file implements the ipset module.  It can handle packets by putting
 * the A and AAAA addresses that are configured in unbound.conf as type
 * ipset (local-zone statements) into a firewall rule IPSet.  For firewall
 * blacklist and whitelist usage.
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

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/ipset/ip_set.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter.h>

#define BUFF_LEN 256

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

static struct mnl_socket * open_mnl_socket() {
	struct mnl_socket *mnl;

	mnl = mnl_socket_open(NETLINK_NETFILTER);
	if (!mnl) {
		log_err("ipset: could not open netfilter.");
		return NULL;
	}

	if (mnl_socket_bind(mnl, 0, MNL_SOCKET_AUTOPID) < 0) {
		mnl_socket_close(mnl);
		log_err("ipset: could not bind netfilter.");
		return NULL;
	}
	return mnl;
}

static int add_to_ipset(struct ipset_env *ie, const void *ipaddr, int af) {
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfg;
	struct nlattr *nested[2];
	char *buffer = (char*)(ie + 1);
	const char *setname;

	if (af != AF_INET && af != AF_INET6) {
		errno = EAFNOSUPPORT;
		return -1;
	}

	setname = (af == AF_INET) ? ie->name_v4 : ie->name_v6;
	if (!setname) {
		errno = EINVAL;
		return -1;
	}

	if (strlen(setname) >= IPSET_MAXNAMELEN) {
		errno = ENAMETOOLONG;
		return -1;
	}

	nlh = mnl_nlmsg_put_header(buffer);
	nlh->nlmsg_type = IPSET_CMD_ADD | (NFNL_SUBSYS_IPSET << 8);
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL;

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfg->nfgen_family = af;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = htons(0);

	mnl_attr_put_u8(nlh, IPSET_ATTR_PROTOCOL, IPSET_PROTOCOL);
	mnl_attr_put_strz(nlh, IPSET_ATTR_SETNAME, setname);

	nested[0] = mnl_attr_nest_start(nlh, IPSET_ATTR_DATA);
	nested[1] = mnl_attr_nest_start(nlh, IPSET_ATTR_IP);
	mnl_attr_put(nlh, (af == AF_INET ? IPSET_ATTR_IPADDR_IPV4 : IPSET_ATTR_IPADDR_IPV6)
			| NLA_F_NET_BYTEORDER, (af == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr)), ipaddr);
	mnl_attr_nest_end(nlh, nested[1]);
	mnl_attr_nest_end(nlh, nested[0]);

	if (mnl_socket_sendto(ie->mnl, nlh, nlh->nlmsg_len) < 0) {
		return -1;
	}
	return 0;
}


static struct nlmsghdr *
__nftnl_nlmsg_build_hdr(char *buf, uint16_t type, uint16_t family,
						uint16_t flags, uint32_t seq,	uint16_t res_id)
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

static int add_to_nftset(struct ipset_env *ie, const void *ipaddr, int af) {
	struct nlmsghdr *nlh;
	struct nlattr *nested[3];
	char *b = (char*)(ie + 1);
	size_t l = 0, addr_size;
	int err;
	const char *tablename, *setname;
	uint16_t nlf = NFPROTO_INET;

	if (af == AF_INET) {
		tablename = ie->table_v4;
		setname   = ie->name_v4;
		addr_size = sizeof(struct in_addr);
	} else if (af == AF_INET6) {
		tablename = ie->table_v6;
		setname = ie->name_v6;
		addr_size = sizeof(struct in6_addr);
	} else {
		errno = EAFNOSUPPORT;
		return -1;
	}

	if (!(tablename && setname)) {
		errno = EINVAL;
		return -1;
	}

	if ((strlen(setname) >= NFT_SET_MAXNAMELEN) ||
		 (strlen(tablename) >= NFT_TABLE_MAXNAMELEN)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	nlh = __nftnl_nlmsg_build_hdr(b, NFNL_MSG_BATCH_BEGIN,
                                NFPROTO_UNSPEC, 0, ie->seq++, NFNL_SUBSYS_NFTABLES);

	if (!nlh) {
		errno = ENOMEM;
		return -1;
	};

	b += nlh->nlmsg_len;
	l += nlh->nlmsg_len;

	nlh = __nftnl_nlmsg_build_hdr(b, (NFNL_SUBSYS_NFTABLES << 8) | NFT_MSG_NEWSETELEM,
                                nlf, NLM_F_CREATE|NLM_F_EXCL, ie->seq++, 0);
	if (!nlh) {
		errno = ENOMEM;
		return -1;
	};

	mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_TABLE, tablename);
	mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_SET,  setname);

	mnl_attr_put_u32(nlh, NFTA_SET_ELEM_LIST_SET_ID, htonl(1));
	nested[0] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_LIST_ELEMENTS);
	nested[1] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_KEY);
	nested[2] = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_KEY);

	mnl_attr_put(nlh, NFTA_DATA_VALUE | NLA_F_NET_BYTEORDER, addr_size, ipaddr);
	mnl_attr_nest_end(nlh, nested[2]);
	mnl_attr_nest_end(nlh, nested[1]);
	mnl_attr_nest_end(nlh, nested[0]);

	b += nlh->nlmsg_len;
	l += nlh->nlmsg_len;

	nlh = __nftnl_nlmsg_build_hdr(b, NFNL_MSG_BATCH_END,
                                AF_UNSPEC, 0, ie->seq++, NFNL_SUBSYS_NFTABLES);

	if (!nlh) {
		errno = ENOMEM;
		return -1;
	};

	b += nlh->nlmsg_len;
	l += nlh->nlmsg_len;

	err = mnl_socket_sendto(ie->mnl, ie + 1, l);
	if (err < 0) {
		log_err("ipset: can't add address into %s->%s (%i)", tablename, setname, err);
		errno = -err;
		return -1;
	}

	return 0;

}

static void
ipset_add_rrset_data(struct ipset_env *ie,
	struct packed_rrset_data *d, int af,
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
				verbose(VERB_QUERY, "ipset: add %s for %s", ip, dname);
			}
			if (ie->mode == 0)
				ret = add_to_ipset(ie, rr_data + 2, af);
			else  {
				ret = add_to_nftset(ie,  rr_data + 2, af);
			}
			if (ret < 0) {
				log_err("ipset: could not add %s", dname);

				mnl_socket_close(ie->mnl);
				ie->mnl = NULL;
				break;
			}
		}
	}
}

static int
ipset_check_zones_for_rrset(struct module_env *env, struct ipset_env *ie,
	struct ub_packed_rrset_key *rrset,
	const char *qname, const int qlen, int af)
{
	static char dname[BUFF_LEN];
	const char *ds, *qs;
	int dlen, plen;

	struct config_strlist *p;
	struct packed_rrset_data *d;

	dlen = sldns_wire2str_dname_buf(rrset->rk.dname, rrset->rk.dname_len, dname, BUFF_LEN);
	if (dlen == 0) {
		log_err("bad domain name");
		return -1;
	}

	for (p = env->cfg->local_zones_ipset; p; p = p->next) {
		ds = NULL;
		qs = NULL;
		plen = strlen(p->str);

		if (dlen >= plen) {
			ds = dname + (dlen - plen);
		}
		if (qlen >= plen) {
			qs = qname + (qlen - plen);
		}
		if ((ds && strncasecmp(p->str, ds, plen) == 0)
			|| (qs && strncasecmp(p->str, qs, plen) == 0)) {
			d = (struct packed_rrset_data*)rrset->entry.data;
			ipset_add_rrset_data(ie, d, af, dname);
			break;
		}
	}
	return 0;
}

static int ipset_update(struct module_env *env, struct dns_msg *return_msg,
	struct query_info qinfo, struct ipset_env *ie)
{
	struct mnl_socket *mnl;
	size_t i;
	struct ub_packed_rrset_key *rrset;
	int af;
	static char qname[BUFF_LEN];
	int qlen;

	mnl = (struct mnl_socket *)ie->mnl;
	if (!mnl) {
		/* retry to create mnl socket */
		mnl = open_mnl_socket();
		if (!mnl) {
			return -1;
		}
		ie->mnl = mnl;
		ie->seq = 1;
	}

	qlen = sldns_wire2str_dname_buf(qinfo.qname, qinfo.qname_len,
		qname, BUFF_LEN);
	if(qlen == 0) {
		log_err("bad domain name");
		return -1;
	}

	for(i = 0; i < return_msg->rep->rrset_count; i++) {
		rrset = return_msg->rep->rrsets[i];
		if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_A &&
			ie->v4_enabled == 1) {
			af = AF_INET;
		} else if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_AAAA &&
			ie->v6_enabled == 1) {
			af = AF_INET6;
		} else
		  continue;

		if(ipset_check_zones_for_rrset(env, ie, rrset, qname, qlen, af) == -1)
			return -1;
	}

	return 0;
}

int ipset_init(struct module_env* env, int id) {
	struct ipset_env *ipset_env;

	ipset_env = (struct ipset_env *)calloc(1, sizeof(struct ipset_env) + BUFF_LEN);
	if (!ipset_env) {
		log_err("malloc failure");
		return 0;
	}

	env->modinfo[id] = (void *)ipset_env;

	ipset_env->mnl = NULL;

	ipset_env->name_v4 = env->cfg->ipset_name_v4;
	ipset_env->name_v6 = env->cfg->ipset_name_v6;
	ipset_env->table_v4 = env->cfg->ipset_table_v4;
	ipset_env->table_v6 = env->cfg->ipset_table_v6;
	ipset_env->mode = env->cfg->ipset_mode;


	ipset_env->v4_enabled = !ipset_env->name_v4 || (strlen(ipset_env->name_v4) == 0) ? 0 : 1;
	ipset_env->v6_enabled = !ipset_env->name_v6 || (strlen(ipset_env->name_v6) == 0) ? 0 : 1;

	if ((ipset_env->v4_enabled < 1) && (ipset_env->v6_enabled < 1)) {
		log_err("ipset: set name no configuration?");
		return 0;
	}

	return 1;
}

void ipset_deinit(struct module_env *env, int id) {
	struct mnl_socket *mnl;
	struct ipset_env *ipset_env;

	if (!env || !env->modinfo[id]) {
		return;
	}

	ipset_env = (struct ipset_env *)env->modinfo[id];

	mnl = (struct mnl_socket *)ipset_env->mnl;
	if (mnl) {
		mnl_socket_close(mnl);
		ipset_env->mnl = NULL;
	}

	free(ipset_env);
	env->modinfo[id] = NULL;
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
	struct cachedb_qstate *iq;
	if (!qstate) {
		return;
	}
	iq = (struct cachedb_qstate *)qstate->minfo[id];
	if (iq) {
		/* free contents of iq */
		/* TODO */
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

/**
 * The ipset function block
 */
static struct module_func_block ipset_block = {
	"ipset",
	&ipset_init, &ipset_deinit, &ipset_operate,
	&ipset_inform_super, &ipset_clear, &ipset_get_mem
};

struct module_func_block * ipset_get_funcblock(void) {
	return &ipset_block;
}

