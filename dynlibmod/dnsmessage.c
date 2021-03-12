#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#include <config.h>
#include <util/module.h>
#include <sldns/parseutil.h>
#include <dynlibmod/dynlibmod.h>

#include <sldns/sbuffer.h>
#include <sldns/str2wire.h>
#include <pythonmod/pythonmod_utils.h>
#include <services/cache/dns.h>

#include "dnsmessage.h"

struct dynlibmod_section_txt {
  char *txt;
  struct dynlibmod_section_txt *next;
};

struct dynlibmod_dnsmessage {
  char *rr_name;
  sldns_rr_type rr_type;
  sldns_rr_class rr_class;
  uint32_t default_ttl;
  uint16_t flags;

  // Question, Answer, Authority, and Additional sections
  struct dynlibmod_section_txt *sections[4];
};

struct dynlibmod_dnsmessage *dynlibmod_dnsmessage_new(char *rr_name, sldns_rr_type rr_type, sldns_rr_class rr_class, uint16_t flags, uint32_t default_ttl) {
    struct dynlibmod_dnsmessage *dns_message = malloc(sizeof(struct dynlibmod_dnsmessage));

    if (!dns_message) {
        log_err("%s:%d malloc failure allocating dns_message", __FUNCTION__, __LINE__);
        goto error;
    }

    memset(dns_message, 0, sizeof(struct dynlibmod_dnsmessage));

    if (rr_name) {
        dns_message->rr_name = strdup(rr_name);

        if (!dns_message->rr_name) {
            log_err("%s:%d malloc failure allocating rr_name", __FUNCTION__, __LINE__);
            goto error;
        }
    }

    dns_message->rr_type = rr_type;
    dns_message->rr_class = rr_class;
    dns_message->flags = flags;
    dns_message->default_ttl = default_ttl;

    return(dns_message);

error:
    if (dns_message)
        free(rr_name);

    free(dns_message);
    return NULL;
}

void dynlibmod_dnsmessage_free(struct dynlibmod_dnsmessage *dnsmessage) {
    if (!dnsmessage)
        return;

    free(dnsmessage->rr_name);
    for (int i = 0; i < 4; i++)
        for (struct dynlibmod_section_txt *ptr = dnsmessage->sections[i]; ptr;) {
            free(ptr->txt);
            struct dynlibmod_section_txt *tmp = ptr;
            ptr = ptr->next;
            free(tmp);
        }

    free(dnsmessage);
}

void dynlibmod_msg_append(struct dynlibmod_dnsmessage *msg, enum enum_section section, const char *format, ...) {
    va_list args;

    va_start(args, format);
    dynlibmod_vmsg_append(msg, section, format, args);
    va_end( args );
}

void dynlibmod_vmsg_append(struct dynlibmod_dnsmessage *msg, enum enum_section section, const char *format, va_list args) {
    struct dynlibmod_section_txt *txt = malloc(sizeof(struct dynlibmod_section_txt));

    if (!txt) {
        log_err("%s:%d malloc failure allocating section txt structure", __FUNCTION__, __LINE__);
        goto error;
    }

    memset(txt, 0, sizeof(struct dynlibmod_section_txt));

    va_list cpy_args;
    va_copy(cpy_args, args);

    int len = vsnprintf(NULL, 0, format, args);
    len++;
    txt->txt = malloc(len);

    if (!txt->txt) {
        va_end(cpy_args);
        log_err("%s:%d malloc failure allocating section txt member", __FUNCTION__, __LINE__);
        goto error;
    }

    vsnprintf(txt->txt, len, format, cpy_args);
    va_end(cpy_args);

    if (!msg->sections[section]) {
        msg->sections[section] = txt;
        return;
    }

    struct dynlibmod_section_txt *ptr = NULL;
    for (ptr = msg->sections[section]; ptr->next; ptr = ptr->next)
        ;

    ptr->next = txt;
    return;

error:
    if (txt)
        free(txt->txt);
    free(txt);

    return;
}

/**
 * Enumerate a section from a dynlibmod_dnsmessage structure and add all of it's members to the dns message
 */
static int _dynlibmod_pushRRList(sldns_buffer* qb, struct dynlibmod_section_txt *txt, uint32_t default_ttl, int qsec, size_t count_offset) {
    size_t len;
    char* s;

    for (; txt; txt = txt->next) {
		s = txt->txt;

        len = sldns_buffer_remaining(qb);
        if(qsec) {
            if(sldns_str2wire_rr_question_buf(s, sldns_buffer_current(qb), &len, NULL, NULL, 0, NULL, 0)!= 0)
                return 0;
        } else {
            if(sldns_str2wire_rr_buf(s, sldns_buffer_current(qb), &len, NULL, default_ttl, NULL, 0, NULL, 0) != 0)
                return 0;
        }
        sldns_buffer_skip(qb, len);

        sldns_buffer_write_u16_at(qb, count_offset,
        sldns_buffer_read_u16_at(qb, count_offset)+1);

    }
    return 1;
}

/**
 * A low level function for preparing a DNS answer by parsing a dynlibmod_dnsmessage.
 */
int dynlibmod_set_return_msg(struct module_qstate* qstate, struct dynlibmod_dnsmessage *msg) {
     sldns_buffer *qb = 0;
     int res = 1;
     size_t l;

     if ((qb = sldns_buffer_new(LDNS_RR_BUF_SIZE)) == 0) return 0;

     /* write header */
     sldns_buffer_write_u16(qb, 0); /* ID */
     sldns_buffer_write_u16(qb, 0); /* flags */
     sldns_buffer_write_u16(qb, 1); /* qdcount */
     sldns_buffer_write_u16(qb, 0); /* ancount */
     sldns_buffer_write_u16(qb, 0); /* nscount */
     sldns_buffer_write_u16(qb, 0); /* arcount */
     if ((msg->flags&PKT_QR)) LDNS_QR_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_AA)) LDNS_AA_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_TC)) LDNS_TC_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_RD)) LDNS_RD_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_CD)) LDNS_CD_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_RA)) LDNS_RA_SET(sldns_buffer_begin(qb));
     if ((msg->flags&PKT_AD)) LDNS_AD_SET(sldns_buffer_begin(qb));

     /* write the query */
     l = sldns_buffer_remaining(qb);
     if(sldns_str2wire_dname_buf(msg->rr_name, sldns_buffer_current(qb), &l) != 0) {
             sldns_buffer_free(qb);
             return 0;
     }
     sldns_buffer_skip(qb, l);
     if (msg->rr_type == 0) { msg->rr_type = LDNS_RR_TYPE_A; }
     if (msg->rr_class == 0) { msg->rr_class = LDNS_RR_CLASS_IN; }
     sldns_buffer_write_u16(qb, msg->rr_type);
     sldns_buffer_write_u16(qb, msg->rr_class);

     /* write RR sections */
     if(res && msg->sections[SECTION_QUESTION] && !_dynlibmod_pushRRList(qb, msg->sections[SECTION_QUESTION], msg->default_ttl, 1, LDNS_QDCOUNT_OFF))
             res = 0;
     if(res && msg->sections[SECTION_ANSWER] && !_dynlibmod_pushRRList(qb, msg->sections[SECTION_ANSWER], msg->default_ttl, 0, LDNS_ANCOUNT_OFF))
             res = 0;
     if(res && msg->sections[SECTION_AUTHORITY] && !_dynlibmod_pushRRList(qb, msg->sections[SECTION_AUTHORITY], msg->default_ttl, 0, LDNS_NSCOUNT_OFF))
             res = 0;
     if(res && msg->sections[SECTION_ADDITIONAL] && !_dynlibmod_pushRRList(qb, msg->sections[SECTION_ADDITIONAL], msg->default_ttl, 0, LDNS_ARCOUNT_OFF))
             res = 0;

     if (res) res = createResponse(qstate, qb);

     if (qb) sldns_buffer_free(qb);
     return res;
}

