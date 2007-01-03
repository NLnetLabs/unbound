/*
 * daemon/unbound.c - main program for unbound DNS resolver daemon.
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */

#include "config.h"
#include "util/log.h"

int 
main(int argc, char* argv[])
{
	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		printf("\tstart unbound daemon DNS resolver.\n");
		return 1;
	}
	log_init();
	log_info("Start of %s.", PACKAGE_STRING);
	return 0;
}
