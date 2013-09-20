/*
 * testcode/unitvandergaast.c - unit test for vandergaast routines.
 *
 * Copyright (c) 2013, NLnet Labs. All rights reserved.
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
 *
 */
 
/**
 * \file
 * Calls vandergaast related unit tests. Exits with code 1 on a failure. 
 */

#include "config.h"

#ifdef CLIENT_SUBNET

#include "util/log.h"
#include "testcode/unitmain.h"
#include "edns-subnet/addrtree.h"

static void issub_test(void)
{
	unit_show_func("edns-subnet/addrtree.h", "issub");
	addrkey_t k1[] = {0x55, 0x55, 0x5A};
	addrkey_t k2[] = {0x55, 0x5D, 0x5A};
	unit_assert( !unittest_wrapper_addrtree_issub(k1, 24, k2, 24,  0) );
	unit_assert(  unittest_wrapper_addrtree_issub(k1,  8, k2, 16,  0) );
	unit_assert(  unittest_wrapper_addrtree_issub(k2, 12, k1, 13,  0) );
	unit_assert( !unittest_wrapper_addrtree_issub(k1, 16, k2, 12,  0) );
	unit_assert(  unittest_wrapper_addrtree_issub(k1, 12, k2, 12,  0) );
	unit_assert( !unittest_wrapper_addrtree_issub(k1, 13, k2, 13,  0) );
	unit_assert(  unittest_wrapper_addrtree_issub(k1, 24, k2, 24, 13) );
	unit_assert( !unittest_wrapper_addrtree_issub(k1, 24, k2, 20, 13) );
	unit_assert(  unittest_wrapper_addrtree_issub(k1, 20, k2, 24, 13) );
}

static void getbit_test(void)
{
	unit_show_func("edns-subnet/addrtree.h", "getbit");
	addrkey_t k1[] = {0x55, 0x55, 0x5A};
	int i;
	for(i = 0; i<20; i++) {
		unit_assert( unittest_wrapper_addrtree_getbit(k1, 20, i) == (i&1) );
	}
}

static void bits_common_test(void)
{
	unit_show_func("edns-subnet/addrtree.h", "bits_common");
	addrkey_t k1[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
	addrkey_t k2[] = {0,0,0,0,0,0,0,0};
	int i;
	
	for(i = 0; i<64; i++) {
		unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k1, 64, i) == 64 );
	}
	for(i = 0; i<8; i++) {
		k2[i] = k1[i]^(1<<i);
	}
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64,  0) == 0*8+7 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64,  8) == 1*8+6 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 16) == 2*8+5 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 24) == 3*8+4 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 32) == 4*8+3 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 40) == 5*8+2 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 48) == 6*8+1 );
	unit_assert( unittest_wrapper_addrtree_bits_common(k1, 64, k2, 64, 56) == 7*8+0 );
}

static void cmpbit_test(void)
{
	unit_show_func("edns-subnet/addrtree.h", "cmpbit");
	addrkey_t k1[] = {0xA5, 0x0F};
	addrkey_t k2[] = {0x5A, 0xF0};
	int i;
	
	for(i = 0; i<16; i++) {
		unit_assert( !unittest_wrapper_addrtree_cmpbit(k1,k1,i) );
		unit_assert(  unittest_wrapper_addrtree_cmpbit(k1,k2,i) );
	}
}

void vandergaast_test(void)
{
	unit_show_feature("vandergaast");
	cmpbit_test();
	bits_common_test();
	getbit_test();
	issub_test();
}
#endif /* CLIENT_SUBNET */

