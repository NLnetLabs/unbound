/*
 * util/module.c - module interface
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
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
 * Implementation of module.h.
 */

#include "config.h"
#include "util/module.h"

const char* 
strextstate(enum module_ext_state s)
{
	switch(s) {
	case module_state_initial: return "module_state_initial";
	case module_wait_reply: return "module_wait_reply";
	case module_wait_module: return "module_wait_module";
	case module_wait_subquery: return "module_wait_subquery";
	case module_error: return "module_error";
	case module_finished: return "module_finished";
	}
	return "bad_extstate_value";
}

const char* 
strmodulevent(enum module_ev e)
{
	switch(e) {
	case module_event_new: return "module_event_new";
	case module_event_pass: return "module_event_pass";
	case module_event_reply: return "module_event_reply";
	case module_event_timeout: return "module_event_timeout";
	case module_event_mod_done: return "module_event_mod_done";
	case module_event_subq_done: return "module_event_subq_done";
	case module_event_subq_error: return "module_event_subq_error";
	case module_event_error: return "module_event_error";
	}
	return "bad_event_value";
}

void 
module_subreq_remove(struct module_qstate** head, struct module_qstate* sub)
{
	if(!sub || !head) 
		return;
	/* snip off double linked list */
	if(sub->subquery_prev)
		sub->subquery_prev->subquery_next = sub->subquery_next;
	else	*head = sub->subquery_next;
	if(sub->subquery_next)
		sub->subquery_next->subquery_prev = sub->subquery_prev;
	/* cleanup values for robustness */
	sub->subquery_next = NULL;
	sub->subquery_prev = NULL;
}

void 
module_subreq_insert(struct module_qstate** head, struct module_qstate* sub)
{
	if(*head)
		(*head)->subquery_prev = sub;
	sub->subquery_next = *head;
	sub->subquery_prev = NULL;
	*head = sub;
}

int 
module_subreq_num(struct module_qstate* q)
{
	int n = 0;
	while(q) {
		n++;
		q = q->subquery_next;
	}
	return n;
}
