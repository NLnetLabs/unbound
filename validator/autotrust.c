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
#include "util/data/dname.h"
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

void autr_point_delete(struct trust_anchor* tp)
{
	if(!tp)
		return;
	lock_unprotect(&tp->lock, tp);
	lock_unprotect(&tp->lock, tp->autr);
	lock_basic_destroy(&tp->lock);
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

/** Add trust anchor from RR. */
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
 * @return false on failure.
 */
static int
load_trustanchor(struct val_anchors* anchors, char* str, const char* fname)
{
        struct autr_ta* ta = NULL;
        struct trust_anchor* tp = NULL;

        ta = add_trustanchor_frm_str(anchors, str, &tp);
	if(!ta)
		return 0;
	lock_basic_lock(&tp->lock);
	if(!parse_comments(str, ta)) {
		lock_basic_unlock(&tp->lock);
		return 0;
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
			return 0;
		}
	}
	lock_basic_unlock(&tp->lock);
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
                if (!load_trustanchor(anchors, line, nm)) {
                        log_err("failed to load trust anchor from %s "
				"at line %i, skipping", nm, line_nr);
                        /* try to do the rest */
                }
        }

        fclose(fd);
	return 1;
}

void autr_write_file(struct trust_anchor* tp)
{
	/* write pretty header */
	/* write revoked tp special marker */
	/* write next probe time */
	/* write anchors */
}

int autr_process_prime(struct module_env* env, struct val_env* ve,
	struct trust_anchor* tp, struct ub_packed_rrset_key* dnskey_rrset)
{
	struct val_anchors* anchors = env->anchors;
	/* autotrust update trust anchors */

	return 1;
}
