/*
 * testcode/fake_event.c - fake event handling that replays existing scenario.
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
 * Event service that replays a scenario.
 * This implements the same exported symbols as the files:
 * util/netevent.c
 * services/listen_dnsport.c
 * services/outside_network.c
 * But these do not actually access the network or events, instead
 * the scenario is played.
 */

#include "config.h"
#include "testcode/fake_event.h"
#include "util/netevent.h"
#include "services/listen_dnsport.h"
#include "services/outside_network.h"
#include "testcode/replay.h"

/** Global variable: the scenario */
static struct replay_scenario* scenario = NULL;

void 
fake_event_init(struct replay_scenario* scen)
{
	scenario = scen;
}

void 
fake_event_cleanup()
{
	replay_scenario_delete(scenario);
	scenario = NULL;
}

/*********** Dummy routines ***********/

struct listen_dnsport* 
listen_create(struct comm_base* ATTR_UNUSED(base),
	int ATTR_UNUSED(num_ifs), const char* ATTR_UNUSED(ifs[]), 
	const char* ATTR_UNUSED(port),
	int ATTR_UNUSED(do_ip4), int ATTR_UNUSED(do_ip6), 
	int ATTR_UNUSED(do_udp), int ATTR_UNUSED(do_tcp),
	size_t ATTR_UNUSED(bufsize), comm_point_callback_t* ATTR_UNUSED(cb), 
	void* ATTR_UNUSED(cb_arg))
{
	return malloc(1);
}

void 
listen_delete(struct listen_dnsport* listen)
{
	free(listen);
}

struct comm_base* comm_base_create()
{
	return malloc(1);
}

void comm_base_delete(struct comm_base* b)
{
	free(b);
}

void comm_base_dispatch(struct comm_base* b)
{
	/* TODO run the scenario ! */
}

void comm_base_exit(struct comm_base* ATTR_UNUSED(b))
{
	/* some sort of failure */
	exit(1);
}

struct comm_signal* comm_signal_create(struct comm_base* ATTR_UNUSED(base),
        void ATTR_UNUSED((*callback)(int, void*)), void* ATTR_UNUSED(cb_arg))
{
	return malloc(1);
}

int comm_signal_bind(struct comm_signal* ATTR_UNUSED(comsig), int 
	ATTR_UNUSED(sig))
{
	return 1;
}

void comm_signal_delete(struct comm_signal* comsig)
{
	free(comsig);
}

void 
comm_point_send_reply(struct comm_reply* repinfo)
{
	/* TODO see if this is checked */
}

void 
comm_point_drop_reply(struct comm_reply* repinfo)
{
	/* TODO */
}

struct outside_network* 
outside_network_create(struct comm_base* ATTR_UNUSED(base),
	size_t ATTR_UNUSED(bufsize), size_t ATTR_UNUSED(num_ports), 
	const char** ATTR_UNUSED(ifs), int ATTR_UNUSED(num_ifs),
	int ATTR_UNUSED(do_ip4), int ATTR_UNUSED(do_ip6), 
	int ATTR_UNUSED(port_base))
{
	return malloc(1);
}

void 
outside_network_delete(struct outside_network* outnet)
{
	free(outnet);
}

void pending_udp_query(struct outside_network* outnet, ldns_buffer* packet,
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg)
{
}

/*********** End of Dummy routines ***********/
