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
#include "util/log.h"
#include "util/net_help.h"

/** max length of parse lines */
#define MAXLEN 10240

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
        return pos + strlen(sub);
}

/** 
 * Parse comments 
 * @param str: to parse
 * @param ta: trust key autotrust metadata
 * @param tp: trust anchor
 * @return false on failure.
 */
static int
parse_comments(char* str, struct autr_ta_data* ta, struct trust_anchor* tp)
{
        char* comment = (char*) malloc(sizeof(char)*MAXLEN);
        char* comments = comment;
        int len = strlen(str), pos = 0, timestamp = 0;
        if (len >= MAXLEN) {
		log_err("line too long");
                return 0;
	}
        while (*str != '\0' && *str != ';')
                str++;
        /* comments */
        if (*str == ';')
                str++;
        while (*str != '\0')
        {
                *comments = *str;
                comments++;
                str++;
        }
        *comments = '\0';

        comments = comment;

        if (!ta || !comments) {
		log_err("malloc failure in parse");
                return 0;
	}
        /* read state */
        pos = position_in_string(comments, "state=");
        if (pos >= (int) strlen(comments))
        {
		log_err("parse error");
                free(comment);
                return 0;
        }
        if (pos <= 0)
                ta->s = AUTR_STATE_START;
        else
        {
                int s = (int) comments[pos] - '0';

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
                                log_nametypeclass(0, "warning: trust anchor "
					"has undefined state, considered "
					"NewKey", tp->name, 
					LDNS_RR_TYPE_DNSKEY, tp->dclass);
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
                ta->pending_count = atoi(comments);
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
                log_warn("trust anchor has no timestamp, considered NOW");
                free(str);
                ta->last_change = time(NULL);
        }
        else
                ta->last_change = timestamp;

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
ta_dnskey_flags(struct ta_key* ta)
{
	uint16_t f;
	if(ta->type != LDNS_RR_TYPE_DNSKEY)
		return 0;
	if(ta->len < 2+2)
		return 0;
	memmove(&f, ta->data+2, 2);
	f = ntohs(f);
	return f;
}


/** Check if KSK DNSKEY */
static int
rr_is_dnskey_sep(struct ta_key* ta)
{
	return (ta_dnskey_flags(ta)&DNSKEY_BIT_SEP);
}

/**
 * Add new trust anchor from a string in file.
 * @param anchors: all anchors
 * @param str: string with comments before the anchor, if any comments.
 * @param tp: trust point returned.
 * @return new key in trust point.
 */
static struct ta_key*
add_trustanchor_frm_str(struct val_anchors* anchors, char* str, 
	struct trust_anchor** tp)
{
	if(!str_contains_data(str, ';'))
		return NULL; /* empty line */
}

/** 
 * Load single anchor 
 * @param anchors: all points.
 * @param str: comments line
 * @return false on failure.
 */
static int
load_trustanchor(struct val_anchors* anchors, char* str)
{
        int status_ok = 1;
        struct ta_key* ta = NULL;
        struct trust_anchor* tp = NULL;

        if (!str_contains_data(str, ';'))
                return status_ok; /* empty lines allowed */
        ta = add_trustanchor_frm_str(anchors, str, &tp);
        if (ta) {
		status_ok = parse_comments(str, ta->autr, tp);
		if (rr_is_dnskey_sep(ta)) {
			if (ta->autr->s == AUTR_STATE_VALID)
				tp->autr->valid ++;
			else if (ta->autr->s == AUTR_STATE_MISSING)
				tp->autr->missing ++;
		}
        }
        else
                return 0;
        return status_ok;
}

int autr_read_file(struct val_anchors* anchors, ldns_buffer* parsebuf,
	const char* nm)
{
        /* the file descriptor */
        FILE* fd;
        /* keep track of line numbers */
        int line_nr = 0;
        /* single line */
        char line[MAXLEN];

        if (!(fd = fopen(nm, "r"))) {
                log_err("unable to open %s for reading: %s", 
			nm, strerror(errno));
                return 0;
        }
        verbose(VERB_ALGO, "reading trust anchor file %s", nm);
        while (fgets(line, MAXLEN, fd) != NULL) {
                line_nr++;
                if (!load_trustanchor(anchors, line)) {
                        log_err("failed to load trust anchor from %s "
				"at line %i, skipping", nm, line_nr);
                        /* try to do the rest */
                }
        }

        fclose(fd);
	return 1;
}

