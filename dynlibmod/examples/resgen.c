/**
 * \file
 *
 * This is an example to show how dynamic libraries can be made to work with
 * unbound. To build a .so file simply run:
 *   gcc -I../.. -shared -Wall -Werror -fpic  -o resgen.so resgen.c
 * And to build for windows, first make unbound with the --with-dynlibmod
 * switch, then use this command:
 *   x86_64-w64-mingw32-gcc -m64 -I../.. -shared -Wall -Werror -fpic
 *      -o resgen.dll resgen.c -L../.. -l:libunbound.dll.a
 * to cross-compile a 64-bit Windows DLL.  The libunbound.dll.a is produced
 * by the compile step that makes unbound.exe and allows the dynlib dll to
 * access definitions in unbound.exe.
 */

#include "config.h"
#include "dynlibmod/dynlibmod.h"
#include "dynlibmod/dnsmessage.h"
#include "util/module.h"
#include "util/net_help.h"
#include "sldns/parseutil.h"
#include "sldns/pkthdr.h"
#include "sldns/wire2str.h"
#include "services/cache/dns.h"


/* Declare the EXPORT macro that expands to exporting the symbol for DLLs when
 * compiling for Windows. All procedures marked with EXPORT in this example are
 * called directly by the dynlib module and must be present for the module to
 * load correctly. */
#ifdef HAVE_WINDOWS_H
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

int endswith(char *str, const char *suffix, char **ptr) {
    size_t lstr = strlen(str);
    size_t lsuffix = strlen(suffix);

    // String too long
    if (lsuffix > lstr)
        return 0;

    int ret = memcmp(str + (lstr - lsuffix), suffix, lsuffix);

    if (ret) // no match
        return 0;

    if (ptr)
        *ptr = str + (lstr - lsuffix);

    return 1;
}

/* Init is called when the module is first loaded. It should be used to set up
 * the environment for this module and do any other initialisation required. */
EXPORT int init(struct module_env* env, int id) {
    log_info("dynlib: hello world from init");
    return 1;
}

/* Deinit is run as the program is shutting down. It should be used to clean up
 * the environment and any left over data. */
EXPORT void deinit(struct module_env* env, int id) {
    log_info("dynlib: hello world from deinit");
}

/* Operate is called every time a query passes by this module. The event can be
 * used to determine which direction in the module chain it came from. */
EXPORT void operate(struct module_qstate* qstate, enum module_ev event,
                    int id, struct outbound_entry* entry) {
    char *qname = sldns_wire2str_dname(qstate->qinfo.qname, qstate->qinfo.qname_len);
    struct dynlibmod_dnsmessage *msg = NULL;

    log_info("dynlib: hello world from operate");
    log_info("dynlib: incoming query: %s %s(%d) %s(%d)",
            qname,
            sldns_lookup_by_id(sldns_rr_classes, qstate->qinfo.qclass)->name,
            qstate->qinfo.qclass,
            sldns_rr_descript(qstate->qinfo.qtype)->_name,
            qstate->qinfo.qtype);

    switch(event) {
        case module_event_new:
        case module_event_pass:
            if (endswith(qname, ".resgentest.", NULL)) {
                msg = dynlibmod_dnsmessage_new(qname, (sldns_rr_type) qstate->qinfo.qtype, LDNS_RR_CLASS_IN, BIT_QR | BIT_RA | BIT_AA, 0);
                dynlibmod_msg_append(msg, LDNS_SECTION_ANSWER, "%s 10 IN A 127.1.2.3", qname);
                if (!dynlibmod_set_return_msg(qstate, msg)) {
                    log_info("Failed to set return message");
                    qstate->ext_state[id] = module_error;
                    goto cleanup;
                }

                qstate->return_msg->rep->security = sec_status_indeterminate;
                qstate->return_rcode = LDNS_RCODE_NOERROR;
                qstate->ext_state[id] = module_finished;
                goto cleanup;
            }
            qstate->ext_state[id] = module_wait_module;
            goto cleanup;

        case module_event_moddone:
            qstate->ext_state[id] = module_finished;
            goto cleanup;

        default:
            qstate->ext_state[id] = module_error;
            goto cleanup;
    }

cleanup:
    dynlibmod_dnsmessage_free(msg);
    free(qname);
}

/* Inform super is called when a query is completed or errors out, but only if
 * a sub-query has been registered to it by this module. Look at
 * mesh_attach_sub in services/mesh.h to see how this is done. */
EXPORT void inform_super(struct module_qstate* qstate, int id,
                         struct module_qstate* super) {
    log_info("dynlib: hello world from inform_super");
}

/* Clear is called once a query is complete and the response has been sent
 * back. It is used to clear up any per-query allocations. */
EXPORT void clear(struct module_qstate* qstate, int id) {
    log_info("dynlib: hello world from clear");
}

/* Get mem is called when Unbound is printing performance information. This
 * only happens explicitly and is only used to show memory usage to the user. */
EXPORT size_t get_mem(struct module_env* env, int id) {
    log_info("dynlib: hello world from get_mem");
    return 0;
}
