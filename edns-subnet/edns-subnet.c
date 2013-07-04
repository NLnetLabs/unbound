/*
 * edns-subnet
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */
 
#include "config.h"

#ifdef CLIENT_SUBNET /* keeps splint happy */
#include "edns-subnet/edns-subnet.h"
#include <string.h>

/** Opcode for edns subnet option, is TBD. */
uint16_t EDNSSUBNET_OPCODE = 0x50fa;
uint8_t EDNSSUBNET_MAX_SUBNET_IP4 = 24;
uint8_t EDNSSUBNET_MAX_SUBNET_IP6 = 64;

int
copy_clear(uint8_t* dst, size_t dstlen, uint8_t* src, size_t srclen, size_t n)
{
	size_t intpart = n / 8;  /* bytes */
	size_t fracpart = n % 8; /* bits */
	size_t written = intpart;
	if (intpart > dstlen || intpart > srclen)
		return 1;
	if (fracpart && (intpart+1 > dstlen || intpart+1 > srclen))
		return 1;
	memcpy(dst, src, intpart);
	if (fracpart) {
		dst[intpart] = src[intpart] & ~(0xFF >> fracpart);
		written++;
	}
	memset(dst + written, 0, dstlen - written);
	return 0;
}

#endif /* CLIENT_SUBNET */
