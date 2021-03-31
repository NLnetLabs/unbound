#include "config.h"
#include "util/module.h"
#include "util/regional.h"
#include "util/net_help.h"
#include "sldns/parseutil.h"
#include "dynlibmod/dynlibmod.h"

#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "services/cache/dns.h"

#include "dynlibmod/dnsmessage.h"

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
        log_err("dynlibmod_dnsmessage_new: malloc failure allocating dns_message");
        goto error;
    }

    memset(dns_message, 0, sizeof(struct dynlibmod_dnsmessage));

    if (rr_name) {
        dns_message->rr_name = strdup(rr_name);

        if (!dns_message->rr_name) {
            log_err("dynlibmod_dnsmessage_new: malloc failure allocating rr_name");
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

void dynlibmod_msg_append(struct dynlibmod_dnsmessage *msg, sldns_pkt_section section, const char *format, ...) {
    va_list args;

    va_start(args, format);
    dynlibmod_vmsg_append(msg, section, format, args);
    va_end( args );
}

void dynlibmod_vmsg_append(struct dynlibmod_dnsmessage *msg, sldns_pkt_section section, const char *format, va_list args) {
    struct dynlibmod_section_txt *txt = malloc(sizeof(struct dynlibmod_section_txt));

    if (!txt) {
        log_err("dynlibmod_vmsg_append: malloc failure allocating section txt structure");
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
        log_err("dynlibmod_vmsg_append: malloc failure allocating section txt member");
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

static int
create_response(struct module_qstate* qstate, sldns_buffer* pkt)
{
	struct msg_parse* prs;
	struct edns_data edns;

	/* parse message */
	prs = (struct msg_parse*) regional_alloc(qstate->env->scratch,
		sizeof(struct msg_parse));
	if(!prs) {
		log_err("create_response: out of memory on incoming message");
		return 0;
	}

	memset(prs, 0, sizeof(*prs));
	memset(&edns, 0, sizeof(edns));

	sldns_buffer_set_position(pkt, 0);
	if(parse_packet(pkt, prs, qstate->env->scratch) != LDNS_RCODE_NOERROR) {
		verbose(VERB_ALGO, "create_response: parse error on reply packet");
		return 0;
	}
	/* edns is not examined, but removed from message to help cache */
	if(parse_extract_edns(prs, &edns, qstate->env->scratch) !=
		LDNS_RCODE_NOERROR)
		return 0;

	/* remove CD-bit, we asked for in case we handle validation ourself */
	prs->flags &= ~BIT_CD;

	/* allocate response dns_msg in region */
	qstate->return_msg = (struct dns_msg*) regional_alloc(qstate->region,
		sizeof(struct dns_msg));
	if(!qstate->return_msg)
		return 0;

	memset(qstate->return_msg, 0, sizeof(*qstate->return_msg));
	if(!parse_create_msg(pkt, prs, NULL, &qstate->return_msg->qinfo,
		&qstate->return_msg->rep, qstate->region)) {
		log_err("create_response: malloc failure: allocating incoming dns_msg");
		return 0;
	}

	/* If the AA flag was set, make sure to reach the client. */
	qstate->return_msg->rep->authoritative =
		(uint8_t)(qstate->return_msg->rep->flags&BIT_AA?1:0);

	if(verbosity >= VERB_ALGO)
		log_dns_msg("create_response: packet:", &qstate->return_msg->qinfo,
			qstate->return_msg->rep);

	return 1;
}

/**
 * A low level function for preparing a DNS answer by parsing a
 * dynlibmod_dnsmessage.
 */
int
dynlibmod_set_return_msg(struct module_qstate* qstate, struct dynlibmod_dnsmessage *msg) {
     sldns_buffer *qb = 0;
     int res = 1;
     size_t l;

     if ((qb = sldns_buffer_new(LDNS_RR_BUF_SIZE)) == NULL) return 0;

     /* write header */
     sldns_buffer_write_u16(qb, 0); /* ID */
     sldns_buffer_write_u16(qb, msg->flags); /* flags */
     sldns_buffer_write_u16(qb, 1); /* qdcount */
     sldns_buffer_write_u16(qb, 0); /* ancount */
     sldns_buffer_write_u16(qb, 0); /* nscount */
     sldns_buffer_write_u16(qb, 0); /* arcount */

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
     if(res && msg->sections[LDNS_SECTION_QUESTION] && !_dynlibmod_pushRRList(qb, msg->sections[LDNS_SECTION_QUESTION], msg->default_ttl, 1, LDNS_QDCOUNT_OFF))
             res = 0;
     if(res && msg->sections[LDNS_SECTION_ANSWER] && !_dynlibmod_pushRRList(qb, msg->sections[LDNS_SECTION_ANSWER], msg->default_ttl, 0, LDNS_ANCOUNT_OFF))
             res = 0;
     if(res && msg->sections[LDNS_SECTION_AUTHORITY] && !_dynlibmod_pushRRList(qb, msg->sections[LDNS_SECTION_AUTHORITY], msg->default_ttl, 0, LDNS_NSCOUNT_OFF))
             res = 0;
     if(res && msg->sections[LDNS_SECTION_ADDITIONAL] && !_dynlibmod_pushRRList(qb, msg->sections[LDNS_SECTION_ADDITIONAL], msg->default_ttl, 0, LDNS_ARCOUNT_OFF))
             res = 0;

     if (res) res = create_response(qstate, qb);

     if (qb) sldns_buffer_free(qb);
     return res;
}
