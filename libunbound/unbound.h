/*
 * unbound.h - unbound validating resolver public API
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
 * This file contains functions to resolve DNS queries and 
 * validate the answers. Synchonously and asynchronously.
 */
#ifdef _UB_UNBOUND_H
#define _UB_UNBOUND_H

/**
 * The validation context is created to hold the resolver status,
 * validation keys and a small cache (containing messages, rrsets,
 * roundtrip times, trusted keys, lameness information).
 *
 * Its contents are internally defined.
 */
struct ub_val_ctx;

/**
 * The validation and resolution results.
 * Allocated by the resolver, and need to be freed by the application
 * with ub_val_result_free().
 */
struct ub_val_result {
	/** a list of network order DNS rdata items, terminated with a 
	 * NULL pointer, so that data[0] is the first result entry,
	 * data[1] the second, and the last entry is NULL. */
	char** data;
	/** the length in bytes of the data items */
	size_t* len;
};

/**
 * Create a resolving and validation context.
 * @return a new context. default initialisation.
 * 	returns NULL on error (malloc failure).
 */
struct ub_val_ctx* ub_val_ctx_create();

/**
 * Destroy a validation context and free all its resources.
 * @param ctx: context to delete.
 */
void ub_val_ctx_delete(struct ub_val_ctx* ctx);

/**
 * setup configuration for the given context.
 * @param ctx: context.
 * @param fname: unbound config file (not all settings applicable).
 * 	This is a power-users interface that lets you specify all sorts
 * 	of options.
 * 	For some specific options, such as adding trust anchors, special
 * 	routines exist.
 * @return: 0 if OK, else error.
 */
int ub_val_ctx_config(struct ub_val_ctx* ctx, char* fname);

/**
 * Add a trust anchor to the given context.
 * The trust anchor is a string, on one line, that holds a valid DNSKEY or
 * DS RR. 
 * @param ctx: context.
 * @param ta: string, with zone-format RR on one line.
 * 	<domainname> <TTL optional> <type> <class optional> <rdata contents>
 * @return 0 if OK, else error.
 */
int ub_val_ctx_add_ta(struct ub_val_ctx* ctx, char* ta);

/**
 * Add trust anchors to the given context.
 * The trust anchor the name of a bind-style config file with trusted-keys{}.
 * @param ctx: context.
 * @param fname: filename of file with bind-style config entries with trust
 * 	anchors.
 * @return 0 if OK, else error.
 */
int ub_val_ctx_trustedkeys(struct ub_val_ctx* ctx, char* fname);

/**
 * Set a context to be synchronous or asynchronous.
 * @param ctx: context.
 * @param async: set false if it should be synchronous (waiting) or
 * 	true if it should be asynchronous (resolving and validating in the
 * 	background).
 * @return 0 if OK, else error.
 */
int ub_val_ctx_async(struct ub_val_ctx* ctx, int async);

/**
 * Poll a context to see if it has any new results
 * Do not poll in a loop, instead extract the fd below to poll for readiness,
 * and then check, or wait using the wait routine.
 * @param ctx: asynchronous context.
 * @return: 0 if nothing to read, or nonzero if a result is available.
 * 	If nonzero, call ctx_process() to get do any callbacks.
 */
int ub_val_ctx_poll(struct ub_val_ctx* ctx);

/**
 * Wait for a context to finish with results. Calls ctx_process() after
 * the wait for you. After the wait, there are no more outstanding queries.
 * @param ctx: asynchronous context.
 * @return: 0 if OK, else error.
 */
int ub_val_ctx_wait(struct ub_val_ctx* ctx);

/**
 * Get file descriptor. Wait for it to become readable, at this point
 * answers are returned from the asynchronous validating resolver.
 * Then call the ub_val_ctx_process to continue processing.
 * @param ctx: asynchronous context.
 * @return: -1 on error, or file descriptor to use select(2) with.
 */
int ub_val_ctx_fd(struct ub_val_ctx* ctx);

/**
 * Call this routine to continue processing results from the validating
 * resolver (when the fd becomes readable).
 * Will perform necessary callbacks.
 * @param ctx: context, asynchronous
 * @return: 0 if OK, else error.
 */
int ub_val_ctx_process(struct ub_val_ctx* ctx);

/**
 * Perform resolution and validation of the target name.
 * @param ctx: context.
 * @param name: domain name in text format (a string).
 * @param rrtype: type of RR in host order, 1 is A.
 * @param rrclass: class of RR in host order, 1 is IN (for internet).
 * @param secure: returns true if the answer validated securely.
 * 	false if not.
 * @param data: returns false if there was no data, or the domain did not exist,
 * 	else true.
 * @param result: the result data is returned in a newly allocated result
 * 	structure.
 * @return 0 if OK, else error.
 */
int ub_val_resolve(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, int* secure, int* data, struct ub_val_result** data);

/**
 * Perform resolution and validation of the target name.
 * Asynchronous, after a while, the callback will be called with your
 * data and the result + secure status.
 * @param ctx: context, asynchronous.
 * @param name: domain name in text format (a string).
 * @param rrtype: type of RR in host order, 1 is A.
 * @param rrclass: class of RR in host order, 1 is IN (for internet).
 * @param mydata: this data is your own data (you can pass NULL),
 * 	and is passed on to the callback function.
 * @param callback: this is called on completion of the resolution.
 * 	It is called as:
 * 	void callback(void* mydata, int secure, int data, 
 * 		struct ub_val_result* result)
 * 	with mydata, the same as passed here,
 * 	with secure true if the answer validated securely.
 * 	with data true if any data was found.
 * 	with result newly allocated result structure.
 * 	TODO return errors in async case.
 * @return 0 if OK, else error.
 */
int ub_val_resolve_async(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, void* mydata, void (*callback)(void*, int, int, 
	struct ub_val_result*));

/* function to get dns result message in its entirety (a buf) */
/* convenience function to get A */
/* convenience to get AAAA */
/* convenience to get PTR */
/* convenience to get 'addrinfo', A, AAAA, canonname */

/* neat error; with errnumber to string conversion. the enum is hidden. */

/* more detail function. with lots of information */

#endif /* _UB_UNBOUND_H */
