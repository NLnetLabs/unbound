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
 *
 * Several ways to use this interface from an application wishing
 * to perform (validated) DNS lookups.
 *
 * All start with
 *	ctx = ub_val_ctx_create();
 *	err = ub_val_ctx_add_ta(ctx, "...");
 *	err = ub_val_ctx_add_ta(ctx, "...");
 *	... some lookups
 *	... call ub_val_ctx_delete(ctx); when you want to stop.
 *
 * Application not threaded. Blocking.
 *	int err = ub_val_resolve(ctx, "www.example.com", ...
 *	if(err) fprintf(stderr, "lookup error: %s\n", ub_val_strerror(err));
 *	... use the answer
 *
 * Application not threaded. Non-blocking ('asynchronous').
 *      err = ub_val_resolve_async(ctx, "www.example.com", ... my_callback);
 *	... application resumes processing ...
 *	... and when either ub_val_ctx_poll(ctx) is true
 *	... or when the file descriptor ub_val_ctx_fd(ctx) is readable,
 *	... or whenever, the app calls ...
 *	ub_val_ctx_process(ctx);
 *	... if no result is ready, the app resumes processing above,
 *	... or process() calls my_callback() with results.
 *
 *      ... if the application has nothing more to do, wait for answer
 *      ub_val_ctx_wait(ctx); 
 *
 * Application threaded. Blocking.
 *	Blocking, same as above. The current thread does the work.
 *	Multiple threads can use the *same context*, each does work and uses
 *	shared cache data from the context.
 *
 * Application threaded. Non-blocking ('asynchronous').
 *	err = ub_val_ctx_async(ctx, 1);
 *	... same as async for non-threaded
 *	... the callbacks are called in the thread that calls process(ctx)
 *
 * If not threading is compiled in, the above async example uses fork(2) to
 * create a process to perform the work. The forked process exits when the 
 * calling process exits, or ctx_delete() is called.
 * Otherwise, for asynchronous with threading, a worker thread is created.
 *
 * The blocking calls use shared ctx-cache when threaded. Thus
 * ub_val_resolve() and ub_val_resolve_async() && ub_val_ctx_wait() are
 * not the same. The first makes the current thread do the work, setting
 * up buffers, etc, to perform its thing (but using shared cache data).
 * The second calls another worker thread (or process) to perform the work.
 * And no buffers need to be setup, but a context-switch happens.
 */
#ifndef _UB_UNBOUND_H
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
	/** The original question, name text string. */
	char* qname;
	/** the type asked for */
	int qtype;
	/** the class asked for */
	int qclass;

	/** 
	 * a list of network order DNS rdata items, terminated with a 
	 * NULL pointer, so that data[0] is the first result entry,
	 * data[1] the second, and the last entry is NULL. 
	 * If there was no data, data[0] is NULL.
	 */
	char** data;

	/** the length in bytes of the data items, len[i] for data[i] */
	int* len;

	/** 
	 * canonical name for the result (the final cname). 
	 * zero terminated string.
	 * May be NULL if no canonical name exists.
	 */
	char* canonname;

	/**
	 * DNS RCODE for the result. May contain additional error code if
	 * there was no data due to an error. 0 (NOERROR) if okay.
	 */
	int rcode;

	/** 
	 * If there was no data, and the domain did not exist, this is true.
	 * If it is false, and there was no data, then the domain name 
	 * is purported to exist, but the requested data type is not available.
	 */
	int nxdomain;

	/** 
	 * If the result was not secure (secure==0), and this result is due 
	 * to a security failure, bogus is true.
	 * This means the data has been actively tampered with, signatures
	 * failed, expected signatures were not present, timestamps on 
	 * signatures were out of date and so on.
	 *
	 * If !secure and !bogus, this can happen if the data is not secure 
	 * because security is disabled for that domain name. 
	 * This means the data is from a domain where data is not signed.
	 */
	int bogus;
};

/**
 * Callback for results of async queries.
 * The readable function definition looks like:
 * void my_callback(void* my_arg, int err, int secure, int havedata,
 *	struct ub_val_result* result);
 * It is called with
 *	my_arg: your pointer to a (struct of) data of your choice, or NULL.
 *	err: if 0 all is OK, otherwise an error occured and no results
 *	     are forthcoming.
 *	secure: if true, the result is validated securely.
 *	havedata: if true, there was data, false if no data.
 *	result: pointer to more detailed result structure.
 *		This structure is allocated on the heap and needs to be
 *		freed with ub_val_result_free(result);
 */
typedef void (*ub_val_callback_t)(void*, int, int, int, struct ub_val_result*);

/**
 * Create a resolving and validation context.
 * @return a new context. default initialisation.
 * 	returns NULL on error.
 */
struct ub_val_ctx* ub_val_ctx_create();

/**
 * Destroy a validation context and free all its resources.
 * Outstanding async queries are killed and callbacks are not called for them.
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
 *	At this time it is only possible to add trusted keys before the
 *	first resolve is done.
 * @param ta: string, with zone-format RR on one line.
 * 	[domainname] [TTL optional] [type] [class optional] [rdata contents]
 * @return 0 if OK, else error.
 */
int ub_val_ctx_add_ta(struct ub_val_ctx* ctx, char* ta);

/**
 * Add trust anchors to the given context.
 * Pass name of a file with DS and DNSKEY records (like from dig or drill).
 * @param ctx: context.
 *	At this time it is only possible to add trusted keys before the
 *	first resolve is done.
 * @param fname: filename of file with keyfile with trust anchors.
 * @return 0 if OK, else error.
 */
int ub_val_ctx_add_ta_file(struct ub_val_ctx* ctx, char* fname);

/**
 * Add trust anchors to the given context.
 * Pass the name of a bind-style config file with trusted-keys{}.
 * @param ctx: context.
 *	At this time it is only possible to add trusted keys before the
 *	first resolve is done.
 * @param fname: filename of file with bind-style config entries with trust
 * 	anchors.
 * @return 0 if OK, else error.
 */
int ub_val_ctx_trustedkeys(struct ub_val_ctx* ctx, char* fname);

/**
 * Set debug verbosity for the context
 * Output is directed to stderr.
 * @param ctx: context.
 * @param d: debug level, 0 is off, 1 is very minimal, 2 is detailed, 
 *	and 3 is lots.
 * @return 0 if OK, else error.
 */
int ub_val_ctx_debuglevel(struct ub_val_ctx* ctx, int d);

/**
 * Set a context behaviour for asynchronous action.
 * @param ctx: context.
 * @param dothread: if true, enables threading and a call to resolve_async() 
 *	creates a thread to handle work in the background.
 *	If false, a process is forked to handle work in the background.
 *	Changes to this setting after async() calls have been made have 
 *	no effect (delete and re-create the context to change).
 * @return 0 if OK, else error.
 */
int ub_val_ctx_async(struct ub_val_ctx* ctx, int dothread);

/**
 * Poll a context to see if it has any new results
 * Do not poll in a loop, instead extract the fd below to poll for readiness,
 * and then check, or wait using the wait routine.
 * @param ctx: context.
 * @return: 0 if nothing to read, or nonzero if a result is available.
 * 	If nonzero, call ctx_process() to do callbacks.
 */
int ub_val_ctx_poll(struct ub_val_ctx* ctx);

/**
 * Wait for a context to finish with results. Calls ctx_process() after
 * the wait for you. After the wait, there are no more outstanding 
 * asynchronous queries.
 * @param ctx: context.
 * @return: 0 if OK, else error.
 */
int ub_val_ctx_wait(struct ub_val_ctx* ctx);

/**
 * Get file descriptor. Wait for it to become readable, at this point
 * answers are returned from the asynchronous validating resolver.
 * Then call the ub_val_ctx_process to continue processing.
 * This routine works immediately after context creation, the fd
 * does not change.
 * @param ctx: context.
 * @return: -1 on error, or file descriptor to use select(2) with.
 */
int ub_val_ctx_fd(struct ub_val_ctx* ctx);

/**
 * Call this routine to continue processing results from the validating
 * resolver (when the fd becomes readable).
 * Will perform necessary callbacks.
 * @param ctx: context
 * @return: 0 if OK, else error.
 */
int ub_val_ctx_process(struct ub_val_ctx* ctx);

/**
 * Perform resolution and validation of the target name.
 * @param ctx: context.
 *	The context is finalized, and can no longer accept config changes.
 * @param name: domain name in text format (a zero terminated text string).
 * @param rrtype: type of RR in host order, 1 is A (address).
 * @param rrclass: class of RR in host order, 1 is IN (for internet).
 * @param secure: returns true if the answer validated securely.
 * 	false if not.
 * 	It is possible to get a result with no data (data is false),
 * 	and secure is true. This means that the non-existance of the data
 * 	was cryptographically proven (with signatures).
 * @param data: returns false if there was no data, or the domain did not exist,
 * 	else true.
 * @param result: the result data is returned in a newly allocated result
 * 	structure.
 * @return 0 if OK, else error.
 */
int ub_val_resolve(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, int* secure, int* data, struct ub_val_result** result);

/**
 * Perform resolution and validation of the target name.
 * Asynchronous, after a while, the callback will be called with your
 * data and the result + secure status.
 * @param ctx: context.
 *	If no thread or process has been created yet to perform the
 *	work in the background, it is created now.
 *	The context is finalized, and can no longer accept config changes.
 * @param name: domain name in text format (a string).
 * @param rrtype: type of RR in host order, 1 is A.
 * @param rrclass: class of RR in host order, 1 is IN (for internet).
 * @param mydata: this data is your own data (you can pass NULL),
 * 	and is passed on to the callback function.
 * @param callback: this is called on completion of the resolution.
 * 	It is called as:
 * 	void callback(void* mydata, int err, int secure, int havedata, 
 * 		struct ub_val_result* result)
 * 	with mydata, the same as passed here,
 * 	with err is 0 when a result has been found.
 * 	with secure true if the answer validated securely.
 * 	with havedata true if any data was found.
 * 	with result newly allocated result structure.
 *
 * 	If an error happens during processing, your callback will be called
 * 	with error set to a nonzero value (and secure=0, data=0, result=0).
 * @param async_id: if you pass a non-NULL value, an identifier number is
 *	returned for the query as it is in progress. It can be used to 
 *	cancel the query.
 * @return 0 if OK, else error.
 */
int ub_val_resolve_async(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, void* mydata, ub_val_callback_t callback, int* async_id);

/**
 * Cancel an async query in progress.
 * Its callback will not be called.
 *
 * @param ctx: context.
 * @param async_id: which query to cancel.
 * @return 0 if OK, else error.
 */
int ub_val_cancel(struct ub_val_ctx* ctx, int async_id);

/**
 * Free storage associated with a result structure.
 * @param result: to free
 */
void ub_val_result_free(struct ub_val_result* result);

/** 
 * Convert error value to a human readable string.
 * @param err: error code from one of the ub_val* functions.
 * @return pointer to constant text string, zero terminated.
 */
const char* ub_val_strerror(int err);

#endif /* _UB_UNBOUND_H */
