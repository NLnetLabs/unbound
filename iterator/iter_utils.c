/*
 * iterator/iter_utils.c - iterative resolver module utility functions.
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
 * This file contains functions to assist the iterator module.
 * Configuration options. Forward zones. 
 */
#include "config.h"
#include "iterator/iter_utils.h"
#include "iterator/iterator.h"
#include "util/net_help.h"
#include "util/log.h"
#include "util/config_file.h"

int 
iter_apply_cfg(struct iter_env* iter_env, struct config_file* cfg)
{
	int i;
	/* target fetch policy */
	iter_env->max_dependency_depth = 4;
	iter_env->target_fetch_policy = (int*)calloc(
		(size_t)iter_env->max_dependency_depth+1, sizeof(int));
	if(iter_env->max_dependency_depth >= 1)
		iter_env->target_fetch_policy[1] = 3;
	if(iter_env->max_dependency_depth >= 2)
		iter_env->target_fetch_policy[2] = 1;
	/* TODO read setting from config */
	if(!iter_env->target_fetch_policy)
		return 0;
	for(i=0; i<iter_env->max_dependency_depth+1; i++)
		verbose(VERB_DETAIL, "target fetch policy for level %d is %d",
			i, iter_env->target_fetch_policy[i]);

	/* forwarder address */
	if(cfg->fwd_address && cfg->fwd_address[0]) {
		if(!ipstrtoaddr(cfg->fwd_address, cfg->fwd_port,
			&iter_env->fwd_addr, &iter_env->fwd_addrlen)) {
			log_err("iterator: could not set forwarder address");
			return 0;
		}
		verbose(VERB_ALGO, "iterator: fwd queries to: %s %d",
		cfg->fwd_address, cfg->fwd_port);
	}
	return 1;
}

