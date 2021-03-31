/**
 * @file Implements the dnsmessage api in C for unbound plugins using the dynlib module
 */

#ifndef DYNLIBMOD_DNSMESSAGE_H
#define DYNLIBMOD_DNSMESSAGE_H

/**
 * Structure encapsulates a new DNS Message.
 * Created by dnylibmod_dnsmessage, it's sections
 * are filled out by dnlibmod_msg_append.
 * Then it is finalized by dynlibmod_set_return_msg
 */
struct dynlibmod_dnsmessage;

/**
 * Allocate a new dnsmessage structure and store the details provided.  Must
 * be freed by dynlibmod_dnsmessage_free after the call to dynlibmod_set_resturn_msg. (The message
 * structure is no longer required at that point)
 */
struct dynlibmod_dnsmessage *dynlibmod_dnsmessage_new(char *rr_name, sldns_rr_type rr_type, sldns_rr_class rr_class, uint16_t flags, uint32_t default_ttl);

/**
 * Free a dnsmessage structure.
 */
void dynlibmod_dnsmessage_free(struct dynlibmod_dnsmessage *dnsmessage);

/**
 * Print an record to one of the 4 DNS sections.  Uses printf conventions.   See example code for usage.
 */
void dynlibmod_msg_append(struct dynlibmod_dnsmessage *msg, sldns_pkt_section section, const char *format, ...);

/**
 * An alternative to dynlibmod_msg_append for use in creating wrapper functions
 */
void dynlibmod_vmsg_append(struct dynlibmod_dnsmessage *msg, sldns_pkt_section section, const char *format, va_list args);

/**
 * Finalize the DNS message and store it in the qstate for return to unbound
 */
int dynlibmod_set_return_msg(struct module_qstate* qstate, struct dynlibmod_dnsmessage *msg);

#endif /* DYNLIBMOD_DNSMESSAGE_H */
