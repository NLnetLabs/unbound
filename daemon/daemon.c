/*
 * daemon/daemon.c - collection of workers that handles requests.
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
 * The daemon consists of global settings and a number of workers.
 */

/** buffer size for network connections */
#define BUFSZ 65552

#include "config.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "util/log.h"
#include "util/config_file.h"
#include "services/listen_dnsport.h"

struct daemon* 
daemon_init()
{
	struct daemon* daemon = (struct daemon*)calloc(1, 
		sizeof(struct daemon));
	if(!daemon)
		return NULL;
	lock_basic_init(&daemon->lock);
	daemon->need_to_exit = 0;
	return daemon;	
}

int 
daemon_open_shared_ports(struct daemon* daemon, struct config_file* cfg)
{
	log_assert(daemon);
	daemon->cfg = cfg;
	if(daemon->cfg->port == daemon->listening_port)
		return 1;
	listening_ports_free(daemon->ports);
	if(!(daemon->ports=listening_ports_open(daemon->cfg)))
		return 0;
	daemon->listening_port = daemon->cfg->port;
	return 1;
}

void 
daemon_fork(struct daemon* daemon)
{
	/* only one thread for now */
	log_assert(daemon);
	daemon->num = 1;
	daemon->workers = (struct worker**)calloc((size_t)daemon->num, 
		sizeof(struct worker*));
	if(!(daemon->workers[0] = worker_init(daemon->cfg, daemon->ports, 
		BUFSZ)))
		fatal_exit("could not initialize thread # %d", 0);
	daemon->workers[0]->daemon = daemon;
	daemon->workers[0]->thread_num = 0;

	log_info("start of service (%s).", PACKAGE_STRING);
	worker_work(daemon->workers[0]);
	if(daemon->workers[0]->need_to_restart)
		daemon->need_to_exit = 0;
	else	daemon->need_to_exit = 1;
}

void 
daemon_cleanup(struct daemon* daemon)
{
	int i;
	log_assert(daemon);
	for(i=0; i<daemon->num; i++)
		worker_delete(daemon->workers[i]);
	free(daemon->workers);
	daemon->workers = NULL;
	daemon->num = 0;
	daemon->cfg = NULL;
}

void 
daemon_delete(struct daemon* daemon)
{
	if(!daemon)
		return;
	listening_ports_free(daemon->ports);
	lock_basic_destroy(&daemon->lock);
	free(daemon);
}
