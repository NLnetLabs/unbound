/*
 * validator/autotrust.c - RFC5011 trust anchor management for unbound.
 *
 * Copyright (c) 2009, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * Contains autotrust implementation.
 */
#include "config.h"
#include "validator/autotrust.h"
#include "validator/val_anchor.h"
#include "validator/val_utils.h"
#include "validator/val_sigcrypt.h"
#include "util/data/dname.h"
#include "util/data/packed_rrset.h"
#include "util/log.h"
#include "util/module.h"
#include "util/net_help.h"

struct autr_global_data* autr_global_create(void)
{
	struct autr_global_data* global;
	global = (struct autr_global_data*)malloc(sizeof(*global));
	if(!global) 
		return NULL;
	rbtree_init(&global->probetree, &probetree_cmp);
	return global;
}

void autr_global_delete(struct autr_global_data* global)
{
	if(!global)
		return;
	/* elements deleted by parent, nothing to do */
	memset(global, 0, sizeof(*global));
}

int probetree_cmp(const void* x, const void* y)
{
	struct trust_anchor* a = (struct trust_anchor*)x;
	struct trust_anchor* b = (struct trust_anchor*)y;
	if(a->autr->next_probe_time < b->autr->next_probe_time)
		return -1;
	if(a->autr->next_probe_time > b->autr->next_probe_time)
		return 1;
	return 0;
}

/** Position in string */
static int
position_in_string(char *str, const char* sub)
{
        int pos = -1, i = 0, j = 0;
        char* walk;
        const char* restore = sub;
        while (*str != '\0')
        {
                walk = str;
                j = 0;
                while (*sub != '\0' && *walk == *sub)
                {
                        sub++;
                        walk++;
                        j++;
                }

                if (*sub == '\0' && j > 0)
                        pos = i;

                sub = restore;
                j = 0;
                i++;
                str++;
        }
        if (pos < 0)
                return pos;
        return pos + (int)strlen(sub);
}

/** 
 * Parse comments 
 * @param str: to parse
 * @param ta: trust key autotrust metadata
 * @return false on failure.
 */
static int
parse_comments(char* str, struct autr_ta* ta)
{
        int len = (int)strlen(str), pos = 0, timestamp = 0;
        char* comment = (char*) malloc(sizeof(char)*len+1);
        char* comments = comment;
	if(!comment) {
		log_err("malloc failure in parse");
                return 0;
	}
	/* skip over whitespace and data at start of line */
        while (*str != '\0' && *str != ';')
                str++;
        if (*str == ';')
                str++;
        /* copy comments */
        while (*str != '\0')
        {
                *comments = *str;
                comments++;
                str++;
        }
        *comments = '\0';

        comments = comment;

        /* read state */
        pos = position_in_string(comments, "state=");
        if (pos >= (int) strlen(comments))
        {
		log_err("parse error");
                free(comment);
                return 0;
        }
        if (pos <= 0)
                ta->s = AUTR_STATE_VALID;
        else
        {
                int s = (int) comments[pos] - '0';
		char* str = ldns_rdf2str(ldns_rr_owner(ta->rr));

                switch(s)
                {
                        case AUTR_STATE_START:
                        case AUTR_STATE_ADDPEND:
                        case AUTR_STATE_VALID:
                        case AUTR_STATE_MISSING:
                        case AUTR_STATE_REVOKED:
                        case AUTR_STATE_REMOVED:
                                ta->s = s;
                                break;
                        default:
				log_warn("trust anchor [%s, DNSKEY, id=%i] has "
					"undefined state, considered NewKey",
					str, ldns_calc_keytag(ta->rr));
                                ta->s = AUTR_STATE_START;
                                break;
                }

                free(str);
        }
        /* read pending count */
        pos = position_in_string(comments, "count=");
        if (pos >= (int) strlen(comments))
        {
		log_err("parse error");
                free(comment);
                return 0;
        }
        if (pos <= 0)
                ta->pending_count = 0;
        else
        {
                comments += pos;
                ta->pending_count = (uint8_t)atoi(comments);
        }

        /* read last change */
        pos = position_in_string(comments, "lastchange=");
        if (pos >= (int) strlen(comments))
        {
		log_err("parse error");
                free(comment);
                return 0;
        }
        if (pos >= 0)
        {
                comments += pos;
                timestamp = atoi(comments);
        }
        if (pos < 0 || !timestamp)
        {
		/* Should we warn about this? It happens for key priming.
		ldns_rdf* owner = ldns_rr_owner(ta->rr);
		char* str = ldns_rdf2str(owner);
		log_warn("trust anchor [%s, DNSKEY, id=%i] has no timestamp, "
			"considered NOW", str, ldns_calc_keytag(ta->rr));
		free(str);
		*/
		/* cannot use event base timeptr, because not inited yet */
		ta->last_change = (uint32_t)time(NULL);
        }
        else
                ta->last_change = (uint32_t)timestamp;

        free(comment);
        return 1;
}

/** Check if a line contains data (besides comments) */
static int
str_contains_data(char* str, char comment)
{
        while (*str != '\0') {
                if (*str == comment || *str == '\n')
                        return 0;
                if (*str != ' ' && *str != '\t')
                        return 1;
                str++;
        }
        return 0;
}

/** Get DNSKEY flags */
static int
dnskey_flags(ldns_rr* rr)
{
	if(ldns_rr_get_type(rr) != LDNS_RR_TYPE_DNSKEY)
		return 0;
	return (int)ldns_read_uint16(ldns_rdf_data(ldns_rr_dnskey_flags(rr)));
}


/** Check if KSK DNSKEY */
static int
rr_is_dnskey_sep(ldns_rr* rr)
{
	return (dnskey_flags(rr)&DNSKEY_BIT_SEP);
}

/** Check if REVOKED DNSKEY */
static int
rr_is_dnskey_revoked(ldns_rr* rr)
{
	return (dnskey_flags(rr)&LDNS_KEY_REVOKE_KEY);
}

/** create ta */
static struct autr_ta*
autr_ta_create(ldns_rr* rr)
{
	struct autr_ta* ta = (struct autr_ta*)calloc(1, sizeof(*ta));
	if(!ta) {
		ldns_rr_free(rr);
		return NULL;
	}
	ta->rr = rr;
	return ta;
}

/** create tp */
static struct trust_anchor*
autr_tp_create(struct val_anchors* anchors, ldns_rr* rr)
{
	ldns_rdf* own = ldns_rr_owner(rr);
	struct trust_anchor* tp = (struct trust_anchor*)calloc(1, sizeof(*tp));
	if(!tp) return NULL;
	tp->name = memdup(ldns_rdf_data(own), ldns_rdf_size(own));
	if(!tp->name) {
		free(tp);
		return NULL;
	}
	tp->namelen = ldns_rdf_size(own);
	tp->namelabs = dname_count_labels(tp->name);
	tp->node.key = tp;
	tp->dclass = ldns_rr_get_class(rr);
	tp->autr = (struct autr_point_data*)calloc(1, sizeof(*tp->autr));
	if(!tp->autr) {
		free(tp->name);
		free(tp);
		return NULL;
	}
	tp->autr->pnode.key = tp;

	lock_basic_lock(&anchors->lock);
	(void)rbtree_insert(anchors->tree, &tp->node);
	lock_basic_unlock(&anchors->lock);
	lock_basic_init(&tp->lock);
	lock_protect(&tp->lock, tp, sizeof(*tp));
	lock_protect(&tp->lock, tp->autr, sizeof(*tp->autr));
	return tp;
}

/** delete assembled rrsets */
static void
autr_rrset_delete(struct trust_anchor* tp)
{
	if(tp->ds_rrset) {
		free(tp->ds_rrset->rk.dname);
		free(tp->ds_rrset->entry.data);
		free(tp->ds_rrset);
	}
	if(tp->dnskey_rrset) {
		free(tp->dnskey_rrset->rk.dname);
		free(tp->dnskey_rrset->entry.data);
		free(tp->dnskey_rrset);
	}
}


void autr_point_delete(struct trust_anchor* tp)
{
	if(!tp)
		return;
	lock_unprotect(&tp->lock, tp);
	lock_unprotect(&tp->lock, tp->autr);
	lock_basic_destroy(&tp->lock);
	autr_rrset_delete(tp);
	free(tp->autr);
	free(tp->name);
	free(tp);
}

/** find or add a new trust point for autotrust */
static struct trust_anchor*
find_add_tp(struct val_anchors* anchors, ldns_rr* rr)
{
	struct trust_anchor* tp;
	ldns_rdf* own = ldns_rr_owner(rr);
	tp = anchor_find(anchors, ldns_rdf_data(own), 
		dname_count_labels(ldns_rdf_data(own)),
		ldns_rdf_size(own), ldns_rr_get_class(rr));
	if(tp) {
		if(!tp->autr) {
			log_err("anchor cannot be with and without autotrust");
			lock_basic_unlock(&tp->lock);
			return NULL;
		}
		return tp;
	}
	tp = autr_tp_create(anchors, rr);
	lock_basic_lock(&tp->lock);
	return tp;
}

/** Add trust anchor from RR */
static struct autr_ta*
add_trustanchor_frm_rr(struct val_anchors* anchors, ldns_rr* rr, 
	struct trust_anchor** tp)
{
	struct autr_ta* ta = autr_ta_create(rr);
	if(!ta) 
		return NULL;
	*tp = find_add_tp(anchors, rr);
	/* add ta to tp */
	ta->next = (*tp)->autr->keys;
	(*tp)->autr->keys = ta;
	lock_basic_unlock(&(*tp)->lock);
	return ta;
}

/**
 * Add new trust anchor from a string in file.
 * @param anchors: all anchors
 * @param str: string with anchor and comments, if any comments.
 * @param tp: trust point returned.
 * @return new key in trust point.
 */
static struct autr_ta*
add_trustanchor_frm_str(struct val_anchors* anchors, char* str, 
	struct trust_anchor** tp)
{
        ldns_rr* rr;
        struct autr_ta* ta = NULL;
	ldns_status lstatus;
        if (!str_contains_data(str, ';'))
                return NULL; /* empty line */
        if (LDNS_STATUS_OK !=
                (lstatus = ldns_rr_new_frm_str(&rr, str, 0, NULL, NULL)))
        {
        	log_err("ldns error while converting string to RR: %s",
			ldns_get_errorstr_by_id(lstatus));
                return NULL;
        }
        ta = add_trustanchor_frm_rr(anchors, rr, tp);
        return ta;
}

/** 
 * Load single anchor 
 * @param anchors: all points.
 * @param str: comments line
 * @param fname: filename
 * @return false on failure, otherwise the tp read.
 */
static struct trust_anchor*
load_trustanchor(struct val_anchors* anchors, char* str, const char* fname)
{
        struct autr_ta* ta = NULL;
        struct trust_anchor* tp = NULL;

        ta = add_trustanchor_frm_str(anchors, str, &tp);
	if(!ta)
		return NULL;
	lock_basic_lock(&tp->lock);
	if(!parse_comments(str, ta)) {
		lock_basic_unlock(&tp->lock);
		return NULL;
	}
	if (rr_is_dnskey_sep(ta->rr)) {
		if (ta->s == AUTR_STATE_VALID)
			tp->autr->valid ++;
		else if (ta->s == AUTR_STATE_MISSING)
			tp->autr->missing ++;
	}
	if(!tp->autr->file) {
		/* TODO insert tp into probe tree */
		tp->autr->file = strdup(fname);
		if(!tp->autr->file) {
			lock_basic_unlock(&tp->lock);
			return NULL;
		}
	}
	lock_basic_unlock(&tp->lock);
        return tp;
}

/**
 * Assemble the trust anchors into DS and DNSKEY packed rrsets.
 * Read the ldns_rrs and builds packed rrsets
 * @param tp: the trust point. Must be locked.
 * @return false on malloc failure.
 */
static int 
autr_assemble(struct trust_anchor* tp)
{
	ldns_rr_list* ds, *dnskey;
	struct autr_ta* ta;
	struct ub_packed_rrset_key* ubds=NULL, *ubdnskey=NULL;

	ds = ldns_rr_list_new();
	dnskey = ldns_rr_list_new();
	if(!ds || !dnskey) {
		ldns_rr_list_free(ds);
		ldns_rr_list_free(dnskey);
		return 0;
	}
	for(ta = tp->autr->keys; ta; ta = ta->next) {
		if(ldns_rr_get_type(ta->rr) == LDNS_RR_TYPE_DS) {
			if(!ldns_rr_list_push_rr(ds, ta->rr)) {
				ldns_rr_list_free(ds);
				ldns_rr_list_free(dnskey);
				return 0;
			}
		} else {
			if(!ldns_rr_list_push_rr(dnskey, ta->rr)) {
				ldns_rr_list_free(ds);
				ldns_rr_list_free(dnskey);
				return 0;
			}
		}
	}

	/* make packed rrset keys - malloced with no ID number, they
	 * are not in the cache */
	/* make packed rrset data (if there is a key) */
	if(ds) {
		ubds = ub_packed_rrset_heap_key(ds);
		if(!ubds) 
			goto error_cleanup;
		ubds->entry.data = packed_rrset_heap_data(ds);
		if(!ubds->entry.data)
			goto error_cleanup;
	}
	if(dnskey) {
		ubdnskey = ub_packed_rrset_heap_key(dnskey);
		if(!ubdnskey)
			goto error_cleanup;
		ubdnskey->entry.data = packed_rrset_heap_data(dnskey);
		if(!ubdnskey->entry.data) {
		error_cleanup:
			if(ubds) {
				free(ubds->rk.dname);
				free(ubds->entry.data);
				free(ubds);
			}
			if(ubdnskey) {
				free(ubdnskey->rk.dname);
				free(ubdnskey->entry.data);
				free(ubdnskey);
			}
			ldns_rr_list_free(ds);
			ldns_rr_list_free(dnskey);
			return 0;
		}
	}

	/* free the old data */
	autr_rrset_delete(tp);

	/* assign the data to replace the old */
	tp->ds_rrset = ubds;
	tp->dnskey_rrset = ubdnskey;

	ldns_rr_list_free(ds);
	ldns_rr_list_free(dnskey);
	return 1;
}

int autr_read_file(struct val_anchors* anchors, const char* nm)
{
        /* the file descriptor */
        FILE* fd;
        /* keep track of line numbers */
        int line_nr = 0;
        /* single line */
        char line[10240];
	/* trust point being read */
	struct trust_anchor *tp = NULL, *tp2;

        if (!(fd = fopen(nm, "r"))) {
                log_err("unable to open %s for reading: %s", 
			nm, strerror(errno));
                return 0;
        }
        verbose(VERB_ALGO, "reading trust anchor file %s", nm);
	/* TODO: read line to see if special marker for revoked tp */
	/* TODO: read next probe time (if in file, otherwise now+0-100s) */
        while (fgets(line, (int)sizeof(line), fd) != NULL) {
                line_nr++;
        	if (!str_contains_data(line, ';'))
                	continue; /* empty lines allowed */
                if (!(tp2=load_trustanchor(anchors, line, nm))) {
                        log_err("failed to load trust anchor from %s "
				"at line %i, skipping", nm, line_nr);
                        /* try to do the rest */
			continue;
                }
		if(tp && tp != tp2) {
			log_err("file %s has mismatching data inside", nm);
        		fclose(fd);
			return 0;
		}
		tp = tp2;
        }
        fclose(fd);
	if(!tp) {
		log_err("failed to read %s", nm);
		return 0;
	}

	/* now assemble the data into DNSKEY and DS packed rrsets */
	lock_basic_lock(&tp->lock);
	autr_assemble(tp);
	lock_basic_unlock(&tp->lock);

	return 1;
}

void autr_write_file(struct trust_anchor* tp)
{
	/* write pretty header */
	/* write revoked tp special marker */
	/* write next probe time */
	/* write anchors */
}

/** verify if dnskey works for trust point 
 * @param env: environment (with time) for verification
 * @param ve: validator environment (with options) for verification.
 * @param tp: trust point to verify with
 * @param rrset: DNSKEY rrset to verify.
 * @return false on failure, true if verification successful.
 */
static int
verify_dnskey(struct module_env* env, struct val_env* ve,
        struct trust_anchor* tp, struct ub_packed_rrset_key* rrset)
{
	if(tp->ds_rrset) {
		/* verify with ds, any will do to prime autotrust */
		enum sec_status sec = val_verify_DNSKEY_with_DS(
			env, ve, rrset, tp->ds_rrset);
		verbose(VERB_ALGO, "autotrust: validate DNSKEY with DS: %s",
			sec_status_to_string(sec));
		if(sec == sec_status_secure) {
			return 1;
		}
	}
	if(tp->dnskey_rrset) {
		/* verify with keys */
		enum sec_status sec = val_verify_rrset(env, ve, rrset,
			tp->dnskey_rrset);
		verbose(VERB_ALGO, "autotrust: DNSKEY is %s",
			sec_status_to_string(sec));
		if(sec == sec_status_secure) {
			return 1;
		}
	}
	return 0;
}

/** Find minimum expiration interval from signatures */
static uint32_t
rrsig_min_exp_time(struct module_env* env, ldns_rr_list* rrset)
{
	size_t i;
	uint32_t t, r = 15 * 24 * 3600; /* 15 days max */
	for(i=0; i<ldns_rr_list_rr_count(rrset); i++) {
		ldns_rr* rr = ldns_rr_list_rr(rrset, i);
		if(ldns_rr_get_type(rr) != LDNS_RR_TYPE_RRSIG)
			continue;
		t = ldns_rdf2native_int32(ldns_rr_rrsig_expiration(rr));
		if(t > *env->now) {
			t = t - *env->now;
			if(t < r)
				r = t;
		}
	}
	return r;
}

/** Is rr self-signed revoked key */
static int
rr_is_selfsigned_revoked(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key* dnskey_rrset, size_t i)
{
	enum sec_status sec;
	sec = dnskey_verify_rrset(env, ve, dnskey_rrset, dnskey_rrset, i);
	return (sec == sec_status_secure);
}

/** Set fetched value */
static void
seen_trustanchor(struct autr_ta* ta, uint8_t seen)
{
	ta->fetched = seen;
	ta->pending_count++;
}

/** set revoked value */
static void
seen_revoked_trustanchor(struct autr_ta* ta, uint8_t revoked)
{
	ta->revoked = revoked;
}

/* Compare two RR buffers, skipping the REVOKED bit */
static int
ldns_rr_compare_wire_skip_revbit(ldns_buffer* rr1_buf, ldns_buffer* rr2_buf)
{
	size_t rr1_len, rr2_len, min_len, i, offset;
	rr1_len = ldns_buffer_capacity(rr1_buf);
	rr2_len = ldns_buffer_capacity(rr2_buf);
	/* jump past dname (checked in earlier part) and especially past TTL */
	offset = 0;
	while (offset < rr1_len && *ldns_buffer_at(rr1_buf, offset) != 0)
		offset += *ldns_buffer_at(rr1_buf, offset) + 1;
	/* jump to rdata section (PAST the rdata length field */
	offset += 11;
	min_len = (rr1_len < rr2_len) ? rr1_len : rr2_len;
	/* compare RRs RDATA byte for byte. */
	for(i = offset; i < min_len; i++)
	{
		uint8_t *rdf1, *rdf2;
		rdf1 = ldns_buffer_at(rr1_buf, i);
		rdf2 = ldns_buffer_at(rr2_buf, i);
		if (i==(offset+1))
		{
			/* this is the second part of the flags field */
			*rdf1 = *rdf1 | LDNS_KEY_REVOKE_KEY;
			*rdf2 = *rdf2 | LDNS_KEY_REVOKE_KEY;
		}
		if (*rdf1 < *rdf2)	return -1;
		else if (*rdf1 > *rdf2)	return 1;
        }
	return 0;
}

/** Compare two RRs skipping the REVOKED bit */
static int
ldns_rr_compare_skip_revbit(const ldns_rr* rr1, const ldns_rr* rr2)
{
	int result;
	size_t rr1_len, rr2_len;
	ldns_buffer* rr1_buf;
	ldns_buffer* rr2_buf;

	result = ldns_rr_compare_no_rdata(rr1, rr2);
	if (result == 0)
	{
		rr1_len = ldns_rr_uncompressed_size(rr1);
		rr2_len = ldns_rr_uncompressed_size(rr2);
		rr1_buf = ldns_buffer_new(rr1_len);
		rr2_buf = ldns_buffer_new(rr2_len);
		if(!rr1_buf || !rr2_buf) {
			ldns_buffer_free(rr1_buf);
			ldns_buffer_free(rr2_buf);
			return 0;
		}
		if (ldns_rr2buffer_wire_canonical(rr1_buf, rr1,
			LDNS_SECTION_ANY) != LDNS_STATUS_OK)
		{
			ldns_buffer_free(rr1_buf);
			ldns_buffer_free(rr2_buf);
			return 0;
		}
		if (ldns_rr2buffer_wire_canonical(rr2_buf, rr2,
			LDNS_SECTION_ANY) != LDNS_STATUS_OK) {
			ldns_buffer_free(rr1_buf);
			ldns_buffer_free(rr2_buf);
			return 0;
		}
		result = ldns_rr_compare_wire_skip_revbit(rr1_buf, rr2_buf);
		ldns_buffer_free(rr1_buf);
		ldns_buffer_free(rr2_buf);
	}
	return result;
}


/** compare two trust anchors */
static int
ta_compare(ldns_rr* a, ldns_rr* b)
{
	int result;
	if (!a && !b)	result = 0;
	else if (!a)	result = -1;
	else if (!b)	result = 1;
	else if (ldns_rr_get_type(a) != ldns_rr_get_type(b))
		result = (int)ldns_rr_get_type(a) - (int)ldns_rr_get_type(b);
	else if (ldns_rr_get_type(a) == LDNS_RR_TYPE_DNSKEY)
		result = ldns_rr_compare_skip_revbit(a, b);
	else if (ldns_rr_get_type(a) == LDNS_RR_TYPE_DS)
		result = ldns_rr_compare(a, b);
	else    result = -1;
	return result;
}

/** find key */
static struct autr_ta*
find_key(struct trust_anchor* tp, ldns_rr* rr)
{
	struct autr_ta* ta;
	if(!tp || !rr)
		return NULL;
	for(ta=tp->autr->keys; ta; ta=ta->next) {
		if(ta_compare(ta->rr, rr) == 0)
			return ta;
	}
	return NULL;
}

/** add key and clone RR and tp already locked */
static struct autr_ta*
add_key(struct trust_anchor* tp, ldns_rr* rr)
{
	ldns_rr* c;
	struct autr_ta* ta;
	c = ldns_rr_clone(rr);
	if(!c) return NULL;
	ta = autr_ta_create(c);
	if(!ta) {
		ldns_rr_free(c);
		return NULL;
	}
	/* link in, tp already locked */
	ta->next = tp->autr->keys;
	tp->autr->keys = ta;
	return ta;
}

/** update the time values for the trustpoint */
static void
set_tp_times(struct trust_anchor* tp, uint32_t rrsig_exp_interval, 
	struct ub_packed_rrset_key* k)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)k->entry.data;
	uint32_t origttl = d->ttl;
	uint32_t x;
	
	verbose(VERB_ALGO, "orig_ttl is %d", (int)origttl);
	verbose(VERB_ALGO, "rrsig_exp_interval is %d", (int)rrsig_exp_interval);

	/* x = MIN(15days, ttl/2, expire/2) */
	x = 15 * 24 * 3600;
	if(origttl/2 < x)
		x = origttl/2;
	if(rrsig_exp_interval/2 < x)
		x = rrsig_exp_interval/2;
	/* MAX(1hr, x) */
	if(x < 3600)
		tp->autr->query_interval = 3600;
	else	tp->autr->query_interval = x;

	/* x= MIN(1day, ttl/10, expire/10) */
	x = 24 * 3600;
	if(origttl/10 < x)
		x = origttl/10;
	if(rrsig_exp_interval/10 < x)
		x = rrsig_exp_interval/10;
	/* MAX(1hr, x) */
	if(x < 3600)
		tp->autr->retry_time = 3600;
	else	tp->autr->retry_time = x;

	verbose(VERB_ALGO, "query_interval: %d, retry_time: %d",
		(int)tp->autr->query_interval, (int)tp->autr->retry_time);
}

/** init events to zero */
static void
init_events(struct trust_anchor* tp)
{
	struct autr_ta* ta;
	for(ta=tp->autr->keys; ta; ta=ta->next) {
		ta->fetched = 0;
	}
}

/** Set update events */
static int
update_events(struct module_env* env, struct val_env* ve,
	struct trust_anchor* tp, struct ub_packed_rrset_key* dnskey_rrset)
{
	ldns_rr_list* r = packed_rrset_to_rr_list(dnskey_rrset, 
		env->scratch_buffer);
	size_t i;
	if(!r) 
		return 0;
	init_events(tp);
	for(i=0; i<ldns_rr_list_rr_count(r); i++) {
		ldns_rr* rr = ldns_rr_list_rr(r, i);
		struct autr_ta* ta = NULL;
		if(ldns_rr_get_type(rr) != LDNS_RR_TYPE_DNSKEY)
			continue;
		if(!rr_is_dnskey_sep(rr))
			continue;
		/* is it new? if revocation bit set, find the unrevoked key */
		ta = find_key(tp, rr);
		if(!ta)
			ta = add_key(tp, rr);
		if(!ta) {
			ldns_rr_list_deep_free(r);
			return 0;
		}
		if(rr_is_dnskey_revoked(rr) && 
			rr_is_selfsigned_revoked(env, ve, dnskey_rrset, i)) {
			/* checked if there is an rrsig signed by this key. */
			log_assert(dnskey_calc_keytag(dnskey_rrset, i) ==
				ldns_calc_keytag(rr));
			if(verbosity >= VERB_ALGO)
				verbose(VERB_ALGO, "DNSKEY id=%d is "
					"self-signed revoked", (int)
					dnskey_calc_keytag(dnskey_rrset, i));
			seen_revoked_trustanchor(ta, 1);
		} else {
			seen_trustanchor(ta, 1);
			if(verbosity >= VERB_ALGO)
				verbose(VERB_ALGO, "DNSKEY id=%d in DNS "
					"response", (int)ldns_calc_keytag(rr));
		}
	}
	set_tp_times(tp, rrsig_min_exp_time(env, r), dnskey_rrset);
	ldns_rr_list_deep_free(r);
	return 1;
}

int autr_process_prime(struct module_env* env, struct val_env* ve,
	struct trust_anchor* tp, struct ub_packed_rrset_key* dnskey_rrset)
{
	struct val_anchors* anchors = env->anchors;
	log_assert(tp->autr);
	/* autotrust update trust anchors */
	/* the tp is locked, and stays locked unless it is deleted */

	/* query_dnskeys(): */
	tp->autr->last_queried = *env->now;

	log_nametypeclass(VERB_ALGO, "autotrust process for",
		tp->name, LDNS_RR_TYPE_DNSKEY, tp->dclass);
	if(!dnskey_rrset) {
		verbose(VERB_ALGO, "autotrust: no dnskey rrset");
		tp->autr->query_failed += 1;
		return 1; /* trust point exists */
	}
	/* verify the dnskey rrset and see if it is valid. */
	if(!verify_dnskey(env, ve, tp, dnskey_rrset)) {
		verbose(VERB_ALGO, "autotrust: dnskey did not verify.");
		tp->autr->query_failed += 1;
		return 1; /* trust point exists */
	}

	tp->autr->query_failed = 0;

	/* update_events(): 
	 * - find minimum rrsig expiration interval
	 * - add new trust anchors to the data structure
	 * - note which trust anchors are seen this probe.
	 * - note revoked (selfsigned) anchors.
	 * Set trustpoint query_interval and retry_time.
	 */
	if(!update_events(env, ve, tp, dnskey_rrset))
		return 1; /* trust point unchanged, so exists */

	/* do_statetable(): 
	 * - for every SEP key do the 5011 statetable.
	 * - remove missing trustanchors (if too many).
	 */
	/* do_statetable(env, tp); */

	autr_assemble(tp);

	return 1;
}
