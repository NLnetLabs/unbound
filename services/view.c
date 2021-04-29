/*
 * services/view.c - named views containing local zones authority service.
 *
 * Copyright (c) 2016, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains functions to enable named views that can hold local zone
 * authority service.
 */
#include "config.h"
#include "services/view.h"
#include "services/localzone.h"
#include "services/cache/dns.h"
#include "services/cache/rrset.h"
#include "iterator/iter_fwd.h"
#include "iterator/iter_hints.h"
#include "util/config_file.h"
#include "daemon/daemon.h"

int 
view_cmp(const void* v1, const void* v2)
{
	return strcmp(v1, v2);
}

struct views* 
views_create(void)
{
	struct views *vs;

	if ((vs = calloc(1, sizeof(*vs))) != NULL) {
		rbtree_init(&vs->vtree, view_cmp);
		lock_rw_init(&vs->lock);
		lock_protect(&vs->lock, &vs->vtree, sizeof(vs->vtree));
	}

	return vs;
}

/** This prototype is defined in in respip.h, but we want to avoid
  * unnecessary dependencies */

extern void respip_set_delete(struct respip_set *set);

static void
view_cleanup(struct view *v)
{
	lock_rw_destroy(&v->lock);
	local_zones_delete(v->local_zones);
	respip_set_delete(v->respip_set);
	forwards_delete(v->fwds);
	hints_delete(v->hints);
	rrset_cache_delete(v->rrset_cache);
	slabhash_delete(v->msg_cache);
	free(v->name);
}

static void
view_node_delete(struct view_node *vn)
{
	if (vn != NULL) {
		view_cleanup(&vn->vinfo);
		free(vn);
	}
}

static void
delviewnode(rbnode_type* n, void* ATTR_UNUSED(arg))
{
	view_node_delete((struct view_node *) n);
}

void 
views_delete(struct views* vs)
{
	if (vs != NULL) {
		// TODO: Clean up the default view once all the other references
		//       to the data elements are removed from the code
		//
		// view_cleanup(&vs->server_view);
		lock_rw_destroy(&vs->lock);
		traverse_postorder(&vs->vtree, delviewnode, NULL);
		free(vs);
	}
}

/** create a new view */
static struct view_node *
view_node_create(char* name)
{
	struct view_node *vn;

	if ((vn = calloc(1, sizeof(*vn))) != NULL) {
		struct view *v = &vn->vinfo;

		if ((v->name = strdup(name)) != NULL) {
			vn->node.key = v->name;
			lock_rw_init(&v->lock);
			lock_protect(&v->lock, v, sizeof(*v));
		} else {
			free(vn);
			vn = NULL;
		}
	}

	return vn;
}

/** enter a new view returns with WRlock */
static struct view *
views_enter_view_name(struct views *vs, char *name)
{
	struct view_node *vn;
	struct view *v;

	if ((vn = view_node_create(name)) != NULL) {
		/* add to rbtree */
		lock_rw_wrlock(&vs->lock);
		lock_rw_wrlock(&vn->vinfo.lock);

		if (rbtree_insert(&vs->vtree, &vn->node) != NULL) {
			v = &vn->vinfo;
		} else {
			log_warn("duplicate view: %s", name);
			lock_rw_unlock(&vn->vinfo.lock);
			view_node_delete(vn);
			v = NULL;
		}

		lock_rw_unlock(&vs->lock);
	} else {
		log_err("out of memory");
		v = NULL;
	}

	return v;
}

static int
view_apply_localzones(struct view *v, struct config_file *cfg, int server)
{
	if (!server && cfg->local_zones == NULL && cfg->local_data == NULL ) {
		return 1;
	}
	if ((v->local_zones = local_zones_create()) == NULL){
		lock_rw_unlock(&v->lock);
		return 0;
	}
	if (!server && v->server_view != NULL) {
		struct config_strlist *nd;

		// Add nodefault zones to list of zones to add, so they will be
		// used as if they are configured as type transparent

		for (nd = cfg->local_zones_nodefault; nd != NULL; nd = nd->next) {
			char *nd_str, *nd_type;

			if ((nd_str = strdup(nd->str)) == NULL) {
				log_err("out of memory");
				lock_rw_unlock(&v->lock);
				return 0;
			}
			if ((nd_type = strdup("nodefault")) == NULL) {
				log_err("out of memory");
				free(nd_str);
				lock_rw_unlock(&v->lock);
				return 0;
			}
			if (!cfg_str2list_insert(&cfg->local_zones,
			                         nd_str,
			                         nd_type)) {
				log_err("failed to insert default zones into local-zone list");
				lock_rw_unlock(&v->lock);
				return 0;
			}
		}
	}
	if (!local_zones_apply_cfg(v->local_zones, cfg)){
		lock_rw_unlock(&v->lock);
		return 0;
	}

	// local_zones, local_zones_nodefault and local_data 
	// are free'd from config_view by local_zones_apply_cfg.
	// Set pointers to NULL.

	cfg->local_zones = NULL;
	cfg->local_data = NULL;
	cfg->local_zones_nodefault = NULL;

	return 1;
}

static int
view_apply_cfg(struct view *v,
               struct config_file *cfg,
               struct alloc_cache *alloc)
{
	int server = (v->server_view == v);

	if (!view_apply_localzones(v, cfg, server)) {
		return 0;
	}
	if (alloc != NULL && cfg->msg_cache_size != 0) {
		if ((v->rrset_cache = rrset_cache_create(cfg, alloc)) == NULL) {
			log_err("failed to create rrset cache for %s view",
			        server ? "server" : v->name);
			return (0);
		}
		if ((v->msg_cache = msg_cache_adjust(NULL, cfg)) == NULL) {
			log_err("failed to create message cache for %s view",
			        server ? "server" : v->name);
			return (0);
		}
	}

	// TODO: instantiate other view-specific data

	return (1);
}

int
views_configure(struct views *vs,
                struct config_file *cfg,
                struct alloc_cache *alloc)
{
	struct view* v = &vs->server_view;
	struct config_view* cv;

	// Construct the server view by hand, including locking

	lock_rw_init(&v->lock);
	lock_protect(&v->lock, v, sizeof(*v));
	lock_rw_wrlock(&v->lock);

	// Mark this view as the server view by pointing to ourselves.

	v->server_view = v;
	v->view_cfg    = cfg;

	if (!view_apply_cfg(&vs->server_view, cfg, alloc)) {
		return (0);
	}

	lock_rw_unlock(&v->lock);

	for (cv = cfg->views; cv != NULL; cv = cv->next) {
		/* create and configure view */

		if ((v = views_enter_view_name(vs, cv->name)) == NULL) {
			return (0);
		}

		v->view_cfg = &cv->cfg_view;

		if (cv->set_server) {
			// Point to the server view to trigger localzone fallback

			v->server_view = &vs->server_view;
		}
		if (!view_apply_cfg(v, &cv->cfg_view, alloc)) {
			return 0;
		}

		lock_rw_unlock(&v->lock);
	}

	return 1;
}

int
views_apply_cfg(struct views* vs, struct config_file* cfg)
{
	// Legacy interface for unbound-checkconf, et al. Doesn't allocate
	// caches

	return (views_configure(vs, cfg, NULL));
}

int
daemon_views(struct daemon *daemon)
{
	if(!(daemon->views = views_create())) {
		fatal_exit("Could not create views: out of memory");
	}
	if (!views_configure(daemon->views, daemon->cfg, &daemon->superalloc)) {
        fatal_exit("Could not set up views");
	}

	daemon->env->current_view_env = &daemon->views->server_view;
	daemon->env->msg_cache = daemon->views->server_view.msg_cache;
	daemon->env->rrset_cache = daemon->views->server_view.rrset_cache;
	return 1;
}

/** find a view by name */
struct view*
views_find_view(struct views* vs, const char* name, int write)
{
	struct view_node *vn;
	struct view *v;

	lock_rw_rdlock(&vs->lock);

	vn = (struct view_node *) rbtree_search(&vs->vtree, name);

	if (vn == NULL) {
		v = NULL;
	} else {
		v = &vn->vinfo;

		if (write) {
			lock_rw_wrlock(&v->lock);
		} else {
			lock_rw_rdlock(&v->lock);
		}
	}

	lock_rw_unlock(&vs->lock);

	return v;
}
