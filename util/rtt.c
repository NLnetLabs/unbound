/*
 * util/rtt.c - UDP round trip time estimator for resend timeouts.
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
 * This file contains a data type and functions to help estimate good
 * round trip times for UDP resend timeout values.
 */
#include "config.h"
#include "util/rtt.h"
#include "util/log.h"

/** calculate RTO from rtt information */
static int
calc_rto(const struct rtt_info* rtt)
{
	/* From Stevens, Unix Network Programming, p.598 */
	int rto = rtt->srtt + 4*rtt->rttvar;
	if(rto < RTT_MIN_TIMEOUT)
		rto = RTT_MIN_TIMEOUT;
	if(rto > RTT_MAX_TIMEOUT)
		rto = RTT_MAX_TIMEOUT;
	return rto;
}

void 
rtt_init(struct rtt_info* rtt)
{
	rtt->srtt = 0;
	rtt->rttvar = 750;
	rtt->rto = calc_rto(rtt);
	/* first RTO is 0 + 4*0.75 = 3 seconds */
}

int 
rtt_timeout(const struct rtt_info* rtt)
{
	return rtt->rto;
}

void 
rtt_update(struct rtt_info* rtt, int ms)
{
	int delta = ms - rtt->srtt;
	rtt->srtt += delta / 8; /* g = 1/8 */
	if(delta < 0)
		delta = -delta; /* |delta| */
	rtt->rttvar += (delta - rtt->rttvar) / 4; /* h = 1/4 */
	rtt->rto = calc_rto(rtt);
}

void 
rtt_lost(struct rtt_info* rtt)
{
	/* exponential backoff */
	rtt->rto *= 2;
	if(rtt->rto > RTT_MAX_TIMEOUT)
		rtt->rto = RTT_MAX_TIMEOUT;
}
