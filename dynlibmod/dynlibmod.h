/*
 * dynlibmod.h: module header file
 * 
 * Copyright (c) 2019, Peter Munch-Ellingsen (peterme AT peterme.net)
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 * 
 *    * Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 * 
 *    * Neither the name of the organization nor the names of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
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
 * Dynamic loading module for unbound.  Loads dynamic library.
 */
#ifndef DYNLIBMOD_H
#define DYNLIBMOD_H
#include "util/module.h"
#include "services/outbound_list.h"

/**
 * Get the module function block.
 * @return: function block with function pointers to module methods.
 */
struct module_func_block* dynlibmod_get_funcblock(void);

/** dynlib module init */
int dynlibmod_init(struct module_env* env, int id);

/** dynlib module deinit */
void dynlibmod_deinit(struct module_env* env, int id);

/** dynlib module operate on a query */
void dynlibmod_operate(struct module_qstate* qstate, enum module_ev event,
	int id, struct outbound_entry* outbound);

/** dynlib module  */
void dynlibmod_inform_super(struct module_qstate* qstate, int id,
	struct module_qstate* super);

/** dynlib module cleanup query state */
void dynlibmod_clear(struct module_qstate* qstate, int id);

/** dynlib module alloc size routine */
size_t dynlibmod_get_mem(struct module_env* env, int id);

#endif /* DYNLIBMOD_H */
