/**
 * ipset.h
 *
 * Author: Kevin Chou
 * Email: k9982874@gmail.com
 */
#ifndef IPSET_H
#define IPSET_H
/** \file
 *
 * This file implements the ipset/nftset module.  It can handle packets by
 * putting the A and AAAA addresses that are configured in unbound.conf as
 * type ipset (local-zone statements) into a firewall set.  For firewall
 * blacklist and whitelist usage.
 *
 * Two backends are selected by the section name in unbound.conf:
 *
 *   ipset:                  legacy iptables ipset on Linux (NFNL_SUBSYS_IPSET)
 *     name-v4: "blacklist"  or PF tables on BSD.
 *     name-v6: "blacklist6"
 *
 *   nftset:                 nftables sets on Linux (NFNL_SUBSYS_NFTABLES).
 *     family: "inet"        Adds two extra fields:
 *     table:  "fw4"           family: "inet" | "ip" | "ip6" (default "inet")
 *     name-v4: "blacklist"    table:  required
 *     name-v6: "blacklist6"
 *
 * Addresses go in via netlink (libmnl) on Linux. Sends are fire-and-forget;
 * the kernel error queue is drained after every send and any reported
 * errors are logged best-effort. Caller / per-message context is not
 * preserved — the kernel error string (NETLINK_EXT_ACK) is enough to
 * diagnose misconfiguration.
 */

#include "util/module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ipset_env {
	void* dev;

	int v4_enabled;
	int v6_enabled;

	const char *name_v4;
	const char *name_v6;

#ifdef USE_NFTSET
	/* nft-only runtime state — unused for ip backend. */
	int use_nft;        /* 1 if this env is the nft backend */
	int nfproto;        /* NFPROTO_INET / NFPROTO_IPV4 / NFPROTO_IPV6 */
	const char *family; /* original family string from config */
	const char *table;  /* nftables table name */
	uint32_t seq;       /* monotonic netlink seq counter */
#endif
};

struct ipset_qstate {
	int dummy;
};

/** Startup the ipset module */
int ipset_startup(struct module_env* env, int id);
/** Destartup the ipset module */
void ipset_destartup(struct module_env* env, int id);
/** Init the ipset module */
int ipset_init(struct module_env* env, int id);
/** Deinit the ipset module */
void ipset_deinit(struct module_env* env, int id);
/** Operate on an event on a query (in qstate). */
void ipset_operate(struct module_qstate* qstate, enum module_ev event,
	int id, struct outbound_entry* outbound);
/** Subordinate query done, inform this super request of its conclusion */
void ipset_inform_super(struct module_qstate* qstate, int id,
	struct module_qstate* super);
/** clear the ipset query-specific contents out of qstate */
void ipset_clear(struct module_qstate* qstate, int id);
/** return memory estimate for ipset module */
size_t ipset_get_mem(struct module_env* env, int id);

#ifdef USE_IPSET
/**
 * Get the function block with pointers to the ipset functions
 * @return the function block for "ipset".
 */
struct module_func_block* ipset_get_funcblock(void);
#endif

#ifdef USE_NFTSET
/**
 * Get the function block for the nftset module.
 * Same functions as ipset, but the block name is "nftset" so that
 * modstack_call_init can match it against module-config: "nftset ...".
 * @return the function block for "nftset".
 */
struct module_func_block* nftset_get_funcblock(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IPSET_H */
