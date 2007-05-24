/*
 * util/config_file.h - reads and stores the config file for unbound.
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
 * This file contains functions for the config file.
 */

#ifndef UTIL_CONFIG_FILE_H
#define UTIL_CONFIG_FILE_H
struct config_stub;
struct config_strlist;

/**
 * The configuration options.
 * Strings are malloced.
 */
struct config_file {
	/** verbosity level as specified in the config file */
	int verbosity;

	/** number of threads to create */
	int num_threads;

	/** port on which queries are answered. */
	int port;
	/** do ip4 query support. */
	int do_ip4;
	/** do ip6 query support. */
	int do_ip6;
	/** do udp query support. */
	int do_udp;
	/** do tcp query support. */
	int do_tcp;

	/** outgoing port range base number */
	int outgoing_base_port;
	/** outgoing port range number of ports (per thread, per if) */
	int outgoing_num_ports;
	/** number of outgoing tcp buffers per (per thread) */
	size_t outgoing_num_tcp;

	/** size of the message cache */
	size_t msg_cache_size;
	/** slabs in the message cache. */
	size_t msg_cache_slabs;
	/** number of queries every thread can service */
	size_t num_queries_per_thread;
	/** size of the rrset cache */
	size_t rrset_cache_size;
	/** slabs in the rrset cache */
	size_t rrset_cache_slabs;
	/** host cache ttl in seconds */
	int host_ttl;
	/** host is lame for a zone ttl, in seconds */
	int lame_ttl;
	/** number of slabs in the infra host cache */
	size_t infra_cache_slabs;
	/** max number of hosts in the infra cache */
	size_t infra_cache_numhosts;
	/** max number of lame zones per host in the infra cache */
	size_t infra_cache_numlame;

	/** forwarder address. string. If not NULL fwder mode is enabled. */
	char* fwd_address;
	/** forwarder port */
	int fwd_port;

	/** number of interfaces to open. If 0 default all interfaces. */
	int num_ifs;
	/** interface description strings (IP addresses) */
	char **ifs;

	/** the stub definitions, linked list */
	struct config_stub* stubs;

	/** chrootdir, if not "" or chroot will be done */
	char* chrootdir;
	/** username to change to, if not "". */
	char* username;
	/** working directory */
	char* directory;
	/** filename to log to. */
	char* logfile;
	/** pidfile to write pid to. */
	char* pidfile;
	
	/** daemonize, i.e. fork into the background. */
	int do_daemonize;
};

/**
 * Stub config options
 */
struct config_stub {
	/** next in list */
	struct config_stub* next;
	/** domain name (in text) of the stub apex domain */
	char* name;
	/** list of stub nameserver hosts (domain name) */
	struct config_strlist* hosts;
	/** list of stub nameserver addresses (IP address) */
	struct config_strlist* addrs;
};

/**
 * List of strings for config options
 */
struct config_strlist {
	/** next item in list */
	struct config_strlist* next;
	/** config option string */
	char* str;
};

/**
 * Create config file structure. Filled with default values.
 * @return: the new structure or NULL on memory error.
 */
struct config_file* config_create();

/**
 * Read the config file from the specified filename.
 * @param config: where options are stored into, must be freshly created.
 * @param filename: name of configfile. If NULL nothing is done.
 * @return: false on error.
 */
int config_read(struct config_file* config, const char* filename);

/**
 * Destroy the config file structure.
 * @param config: to delete.
 */
void config_delete(struct config_file* config);

/**
 * Used during options parsing
 */
struct config_parser_state {
	/** name of file being parser */
	char* filename;
	/** line number in the file, starts at 1 */
	int line;
	/** number of errors encountered */
	int errors;
	/** the result of parsing is stored here. */
	struct config_file* cfg;
	/** has server: already been seen. */
	int server_settings_seen;
};

/** global config parser object used during config parsing */
extern struct config_parser_state* cfg_parser;
/** parsing helpers: print error with file and line numbers. */
void ub_c_error(const char* msg);
/** parsing helpers: print error with file and line numbers. */
void ub_c_error_msg(const char* fmt, ...) ATTR_FORMAT(printf, 1, 2);

#endif /* UTIL_CONFIG_FILE_H */
