/*
 * validator/val_utils.h - validator utility functions.
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
 * This file contains helper functions for the validator module.
 */

#ifndef VALIDATOR_VAL_UTILS_H
#define VALIDATOR_VAL_UTILS_H
struct query_info;
struct reply_info;

/**
 * Response classifications for the validator. The different types of proofs.
 */
enum val_classification {
	/** Not subtyped yet. */
	VAL_CLASS_UNTYPED = 0,
	/** Not a recognized subtype. */
	VAL_CLASS_UNKNOWN,
	/** A positive, direct, response */
	VAL_CLASS_POSITIVE,
	/** A positive response, with a CNAME/DNAME chain. */
	VAL_CLASS_CNAME,
	/** A NOERROR/NODATA response. */
	VAL_CLASS_NODATA,
	/** A NXDOMAIN response. */
	VAL_CLASS_NAMEERROR,
	/** A response to a qtype=ANY query. */
	VAL_CLASS_ANY
};

/**
 * Given a response, classify ANSWER responses into a subtype.
 * @param qinf: query info
 * @param rep: response
 * @return A subtype ranging from UNKNOWN to NAMEERROR.
 */
enum val_classification val_classify_response(struct query_info* qinf,
	struct reply_info* rep);

/**
 * Given a response, determine the name of the "signer". This is primarily
 * to determine if the response is, in fact, signed at all, and, if so, what
 * is the name of the most pertinent keyset.
 *
 * @param qinf: query
 * @param rep: response to that
 * @param signer_name:  signer name, if the response is signed 
 * 	(even partially), or null if the response isn't signed.
 * @param signer_len: length of signer_name of 0 if signer_name is NULL.
 */
void val_find_signer(struct query_info* qinf, struct reply_info* rep,
	uint8_t** signer_name, size_t* signer_len);

#endif /* VALIDATOR_VAL_UTILS_H */
