/*
 * iterator/iter_priv.c - iterative resolver private address and domain store
 *
 * Copyright (c) 2008, NLnet Labs. All rights reserved.
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
 * Keep track of the private addresses and lookup fast.
 */

#include "config.h"
#include "iterator/iter_priv.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/data/dname.h"
#include "util/net_help.h"
#include "util/storage/dnstree.h"

struct iter_priv* priv_create()
{
	struct iter_priv* priv = (struct iter_priv*)calloc(1, sizeof(*priv));
	if(!priv)
		return NULL;
	priv->region = regional_create();
	if(!priv->region) {
		priv_delete(priv);
		return NULL;
	}
	addr_tree_init(&priv->a);
	name_tree_init(&priv->n);
	return priv;
}

void priv_delete(struct iter_priv* priv)
{
	if(!priv) return;
	regional_destroy(priv->region);
	free(priv);
}

/** Read private-addr declarations from config */
static int read_addrs(struct iter_priv* priv, struct config_file* cfg)
{
	/* parse addresses, report errors, insert into tree */
	struct config_strlist* p;
	struct addr_tree_node* n;
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;

	for(p = cfg->private_address; p; p = p->next) {
		log_assert(p->str);
		if(!netblockstrtoaddr(p->str, UNBOUND_DNS_PORT, &addr, 
			&addrlen, &net)) {
			log_err("cannot parse private-address: %s", p->str);
			return 0;
		}
		n = (struct addr_tree_node*)regional_alloc(priv->region,
			sizeof(*n));
		if(!n) {
			log_err("out of memory");
			return 0;
		}
		if(!addr_tree_insert(&priv->a, n, &addr, addrlen, net)) {
			verbose(VERB_QUERY, "ignoring duplicate "
				"private-address: %s", p->str);
		}
	}
	return 1;
}

/** Read private-domain declarations from config */
static int read_names(struct iter_priv* priv, struct config_file* cfg)
{
	/* parse names, report errors, insert into tree */
	struct config_strlist* p;
	struct name_tree_node* n;
	uint8_t* nm;
	size_t nm_len;
	int nm_labs;
	ldns_rdf* rdf;

	for(p = cfg->private_address; p; p = p->next) {
		log_assert(p->str);
		rdf = ldns_dname_new_frm_str(p->str);
		if(!rdf) {
			log_err("cannot parse private-domain: %s", p->str);
			return 0;
		}
		nm = ldns_rdf_data(rdf);
		nm_labs = dname_count_size_labels(nm, &nm_len);
		nm = (uint8_t*)regional_alloc_init(priv->region, nm, nm_len);
		ldns_rdf_deep_free(rdf);
		if(!nm) {
			log_err("out of memory");
			return 0;
		}
		n = (struct name_tree_node*)regional_alloc(priv->region,
			sizeof(*n));
		if(!n) {
			log_err("out of memory");
			return 0;
		}
		if(!name_tree_insert(&priv->n, n, nm, nm_len, nm_labs,
			LDNS_RR_CLASS_IN)) {
			verbose(VERB_QUERY, "ignoring duplicate "
				"private-domain: %s", p->str);
		}
	}
	return 1;
}

int priv_apply_cfg(struct iter_priv* priv, struct config_file* cfg)
{
	/* empty the current contents */
	regional_free_all(priv->region);
	addr_tree_init(&priv->a);
	name_tree_init(&priv->n);

	/* read new contents */
	if(!read_addrs(priv, cfg))
		return 0;
	if(!read_names(priv, cfg))
		return 0;

	/* prepare for lookups */
	addr_tree_init_parents(&priv->a);
	name_tree_init_parents(&priv->n);
	return 1;
}

int priv_lookup_addr(struct iter_priv* priv, struct sockaddr_storage* addr,
	socklen_t addrlen)
{
	return addr_tree_lookup(&priv->a, addr, addrlen) != NULL;
}

int priv_lookup_name(struct iter_priv* priv, uint8_t* name, uint16_t dclass)
{
	size_t len;
	int labs = dname_count_size_labels(name, &len);
	return name_tree_lookup(&priv->n, name, len, labs, dclass) != NULL;
}

size_t priv_get_mem(struct iter_priv* priv)
{
	if(!priv) return 0;
	return sizeof(*priv) + regional_get_mem(priv->region);
}
