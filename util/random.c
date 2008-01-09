/*
 * util/random.c - thread safe random generator, which is reasonably secure.
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
 * Thread safe random functions. Similar to arc4random() with an explicit
 * initialisation routine.
 *
 * The code in this file is based on arc4random from
 * openssh-4.0p1/openbsd-compat/bsd-arc4random.c
 * That code is also BSD licensed.
 */
#include "config.h"
#include "util/random.h"
#include "util/log.h"
#include <openssl/rand.h>
#include <openssl/rc4.h>
#include <openssl/err.h>

/**
 * Struct with per-thread random state.
 * Keeps SSL types away from the header file.
 */
struct ub_hiddenstate {
	/** key used for arc4random generation */
	RC4_KEY rc4;
	/** keeps track of key usage */
	int rc4_ready;
};

/** Size of key to use */
#define SEED_SIZE 20

/** Number of bytes to reseed after */
#define REKEY_BYTES	(1 << 24)

/** reseed random generator */
static void
ub_arc4random_stir(struct ub_hiddenstate* s)
{
	unsigned char rand_buf[SEED_SIZE];
	int i;

	memset(&s->rc4, 0, sizeof(s->rc4));
	if (RAND_bytes(rand_buf, (int)sizeof(rand_buf)) <= 0)
		fatal_exit("Couldn't obtain random bytes (error %ld)",
			    ERR_get_error());
	RC4_set_key(&s->rc4, (int)sizeof(rand_buf), rand_buf);

	/*
	 * Discard early keystream, as per recommendations in:
	 * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	 */
	for(i = 0; i <= 256; i += sizeof(rand_buf))
		RC4(&s->rc4, sizeof(rand_buf), rand_buf, rand_buf);

	memset(rand_buf, 0, sizeof(rand_buf));

	s->rc4_ready = REKEY_BYTES;
}

int 
ub_initstate(unsigned int seed, struct ub_randstate* state, 
	unsigned long ATTR_UNUSED(n))
{
	state->s = calloc(1, sizeof(*state->s));
	if(!state->s) {
		log_err("malloc failure in random init");
		return 0;
	}

	/* RAND_ is threadsafe, by the way */
	if(!RAND_status()) {
		/* try to seed it */
		RAND_seed(&seed, (int)sizeof(seed));
		if(!RAND_status()) {
			log_err("Random generator has no entropy (error %ld)",
				ERR_get_error());
			return 0;
		}
	}
	ub_arc4random_stir(state->s);
	return 1;
}

long int 
ub_random(struct ub_randstate* state)
{
	unsigned int r = 0;
	if (state->s->rc4_ready <= 0) {
		ub_arc4random_stir(state->s);
	}

	RC4(&state->s->rc4, sizeof(r), 
		(unsigned char *)&r, (unsigned char *)&r);
	state->s->rc4_ready -= sizeof(r);
	return (long int)((r) % (((unsigned)RAND_MAX + 1)));
}

void 
ub_randfree(struct ub_randstate* state)
{
	if(state)
		free(state->s);
	RAND_cleanup();
}
