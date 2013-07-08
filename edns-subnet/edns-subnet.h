/*
 * edns-subnet
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */

/**
 * \file
 * Subnet option related constants. 
 */

#ifndef EDNSSUBNET_EDNSSUBNET_H
#define EDNSSUBNET_EDNSSUBNET_H

/** In use by the edns subnet option code, as assigned by IANA */
#define EDNSSUBNET_ADDRFAM_IP4 1
#define EDNSSUBNET_ADDRFAM_IP6 2

/** Opcode for edns subnet option, is TBD. */
extern uint16_t EDNSSUBNET_OPCODE;
/** Maximum number of bits we are willing to expose */
extern uint8_t EDNSSUBNET_MAX_SUBNET_IP4;
extern uint8_t EDNSSUBNET_MAX_SUBNET_IP6;

/** 
 * copy the first n BITS from src to dst iff both src and dst 
 * are large enough, return 0 on succes
 * */
int
copy_clear(uint8_t* dst, size_t dstlen, uint8_t* src, size_t srclen, size_t n);

#endif /* EDNSSUBNET_EDNSSUBNET_H */
