/**
 * ipset.h
 *
 * Author: Kevin Chou
 * Email: k9982874@gmail.com
 */
#ifndef IPSET_H
#define IPSET_H

#include "util/module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ipset_env {
    void* mnl;

	int v4_enabled;
	int v6_enabled;

	const char *name_v4;
	const char *name_v6;
};

struct ipset_qstate {
	int dummy;
};

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

/**
 * Get the function block with pointers to the ipset functions
 * @return the function block for "ipset".
 */
struct module_func_block* ipset_get_funcblock(void);

#ifdef __cplusplus
}
#endif

#endif /* IPSET_H */

