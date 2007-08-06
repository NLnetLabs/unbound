/*
 * validator/validator.h - secure validator DNS query response module
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
 *
 * This file contains a module that performs validation of DNS queries.
 * According to RFC 4034.
 */

#ifndef VALIDATOR_VALIDATOR_H
#define VALIDATOR_VALIDATOR_H
struct module_func_block;
#include "util/data/msgreply.h"
struct val_anchors;
struct key_cache;

/**
 * Global state for the validator. 
 */
struct val_env {
	/** trusted key storage; these are the configured keys */
	struct val_anchors* anchors;

	/** key cache; these are validated keys. trusted keys only
	 * end up here after being primed. */
	struct key_cache* kcache;
};

/**
 * State of the validator for a query.
 */
enum val_state {
	/** initial state for validation */
	VAL_INIT_STATE = 0,
	/** handle response to trust anchor priming query */
	VAL_PRIME_RESP_STATE,
	/** find the proper keys for validation, follow trust chain */
	VAL_FINDKEY_STATE,
	/** handle response to DS query to make trust chain */
	VAL_FINDKEY_DS_RESP_STATE,
	/** handle response to DNSKEY query to make trust chain */
	VAL_FINDKEY_DNSKEY_RESP_STATE,
	/** validate the answer, using found key entry */
	VAL_VALIDATE_STATE,
	/** finish up */
	VAL_FINISHED_STATE
};

/**
 * Per query state for the validator module.
 */
struct val_qstate {
	/** 
	 * State of the validator module.
	 */
	enum val_state state;

	/**
	 * The original message we have been given to validate.
	 */
	struct dns_msg* orig_msg;

	/**
	 * The query name we have chased to; qname after following CNAMEs
	 */
	struct query_info qchase;

	/**
	 * The chased reply, extract from original message. Can be:
	 * 	o CNAME
	 * 	o DNAME + CNAME
	 * 	o answer plus authority, additional (nsecs).
	 */
	struct reply_info* chase_reply;
};

/**
 * Get the validator function block.
 * @return: function block with function pointers to validator methods.
 */
struct module_func_block* val_get_funcblock();

/**
 * Get validator state as a string
 * @param state: to convert
 * @return constant string that is printable.
 */
const char* val_state_to_string(enum val_state state);

#endif /* VALIDATOR_VALIDATOR_H */
