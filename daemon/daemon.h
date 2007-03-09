/*
 * daemon/daemon.h - collection of workers that handles requests.
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

#ifndef DAEMON_H
#define DAEMON_H

#include "util/locks.h"
#include "util/alloc.h"
struct config_file;
struct worker;
struct listen_port;

/**
 * Structure holding worker list.
 * Holds globally visible information.
 */
struct daemon {
	/** The config settings */
	struct config_file* cfg;
	/** current working directory */
	char* cwd;
	/** pidfile that is used */
	char* pidfile;
	/** port number that has ports opened. */
	int listening_port;
	/** listening ports, opened, to be shared by threads */
	struct listen_port* ports;
	/** num threads allocated */
	int num;
	/** the worker entries */
	struct worker** workers;
	/** do we need to exit unbound (or is it only a reload?) */
	int need_to_exit;
	/** master allocation cache */
	struct alloc_cache superalloc;
};

/**
 * Initialize daemon structure.
 * @return: The daemon structure, or NULL on error.
 */
struct daemon* daemon_init();

/**
 * Open shared listening ports (if needed).
 * The cfg member pointer must have been set for the daemon.
 * @param daemon: the daemon.
 * @return: false on error.
 */
int daemon_open_shared_ports(struct daemon* daemon);

/**
 * Fork workers and start service.
 * When the routine exits, it is no longer forked.
 * @param daemon: the daemon.
 */
void daemon_fork(struct daemon* daemon);

/**
 * Close off the worker thread information.
 * Bring the daemon back into state ready for daemon_fork again.
 * @param daemon: the daemon.
 */
void daemon_cleanup(struct daemon* daemon);

/**
 * Delete workers, close listening ports.
 * @param daemon: the daemon.
 */
void daemon_delete(struct daemon* daemon);

#endif /* DAEMON_H */
