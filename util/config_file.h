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
struct config_str2list;

/**
 * The configuration options.
 * Strings are malloced.
 */
struct config_file {
	/** verbosity level as specified in the config file */
	int verbosity;
	/** statistics interval (in seconds) */
	int stat_interval;

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
	/** number of incoming tcp buffers per (per thread) */
	size_t incoming_num_tcp;

	/** number of bytes buffer size for DNS messages */
	size_t msg_buffer_size;
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
	/** max size of lame zones per host in the infra cache */
	size_t infra_cache_lame_size;

	/** the target fetch policy for the iterator */
	char* target_fetch_policy;

	/** automatic interface for incoming messages. Uses ipv6 remapping,
	 * and recvmsg/sendmsg ancillary data to detect interfaces, boolean */
	int if_automatic;

	/** number of interfaces to open. If 0 default all interfaces. */
	int num_ifs;
	/** interface description strings (IP addresses) */
	char **ifs;

	/** number of outgoing interfaces to open. 
	 * If 0 default all interfaces. */
	int num_out_ifs;
	/** outgoing interface description strings (IP addresses) */
	char **out_ifs;

	/** the root hints */
	struct config_strlist* root_hints;
	/** the stub definitions, linked list */
	struct config_stub* stubs;
	/** the forward zone definitions, linked list */
	struct config_stub* forwards;
	/** list of donotquery addresses, linked list */
	struct config_strlist* donotqueryaddrs;
	/** list of access control entries, linked list */
	struct config_str2list* acls;
	/** use default localhost donotqueryaddr entries */
	int donotquery_localhost;

	/** harden against very small edns buffer sizes */
	int harden_short_bufsize;
	/** harden against very large query sizes */
	int harden_large_queries;
	/** harden against spoofed glue (out of zone data) */
	int harden_glue;
	/** harden against receiving no DNSSEC data for trust anchor */
	int harden_dnssec_stripped;

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

	/** should log messages be sent to syslogd */
	int use_syslog;

	/** do not report identity (id.server, hostname.bind) */
	int hide_identity;
	/** do not report version (version.server, version.bind) */
	int hide_version;
	/** identity, hostname is returned if "". */
	char* identity;
	/** version, package version returned if "". */
	char* version;

	/** the module configuration string */
	char* module_conf;
	
	/** files with trusted DS and DNSKEYs in zonefile format, list */
	struct config_strlist* trust_anchor_file_list;
	/** list of trustanchor keys, linked list */
	struct config_strlist* trust_anchor_list;
	/** files with trusted DNSKEYs in named.conf format, list */
	struct config_strlist* trusted_keys_file_list;

	/** the number of seconds maximal TTL used for RRsets and messages */
	int max_ttl;
	/** if not 0, this value is the validation date for RRSIGs */
	int32_t val_date_override;
	/** this value sets the number of seconds before revalidating bogus */
	int bogus_ttl; 
	/** should validator clean additional section for secure msgs */
	int val_clean_additional;
	/** should validator allow bogus messages to go through */
	int val_permissive_mode;
	/** nsec3 maximum iterations per key size, string */
	char* val_nsec3_key_iterations;

	/** size of the key cache */
	size_t key_cache_size;
	/** slabs in the key cache. */
	size_t key_cache_slabs;

	/** local zones config */
	struct config_str2list* local_zones;
	/** local zones nodefault list */
	struct config_strlist* local_zones_nodefault;
	/** local data RRs configged */
	struct config_strlist* local_data;

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
 * List of two strings for config options
 */
struct config_str2list {
	/** next item in list */
	struct config_str2list* next;
	/** first string */
	char* str;
	/** second string */
	char* str2;
};

/**
 * Create config file structure. Filled with default values.
 * @return: the new structure or NULL on memory error.
 */
struct config_file* config_create();

/**
 * Create config file structure for library use. Filled with default values.
 * @return: the new structure or NULL on memory error.
 */
struct config_file* config_create_forlib();

/**
 * Read the config file from the specified filename.
 * @param config: where options are stored into, must be freshly created.
 * @param filename: name of configfile. If NULL nothing is done.
 * @return: false on error.
 */
int config_read(struct config_file* config, char* filename);

/**
 * Destroy the config file structure.
 * @param config: to delete.
 */
void config_delete(struct config_file* config);

/**
 * Apply config to global constants; this routine is called in single thread.
 * @param config: to apply. Side effect: global constants change.
 */
void config_apply(struct config_file* config);

/**
 * Set the given keyword to the given value.
 * @param config: where to store config
 * @param option: option name, including the ':' character.
 * @param value: value, this string is copied if needed, or parsed.
 * 	The caller owns the value string.
 * @return 0 on error (malloc or syntax error).
 */
int config_set_option(struct config_file* config, const char* option,
	const char* value);

/**
 * Insert string into strlist.
 * @param head: pointer to strlist head variable.
 * @param item: new item. malloced by caller. If NULL the insertion fails.
 * @return: true on success.
 */
int cfg_strlist_insert(struct config_strlist** head, char* item);

/**
 * Insert string into str2list.
 * @param head: pointer to str2list head variable.
 * @param item: new item. malloced by caller. If NULL the insertion fails.
 * @param i2: 2nd string, malloced by caller. If NULL the insertion fails.
 * @return: true on success.
 */
int cfg_str2list_insert(struct config_str2list** head, char* item, char* i2);

/**
 * Delete items in config string list.
 * @param list: list.
 */
void config_delstrlist(struct config_strlist* list);

/**
 * Delete items in config double string list.
 * @param list: list.
 */
void config_deldblstrlist(struct config_str2list* list);

/**
 * Delete items in config stub list.
 * @param list: list.
 */
void config_delstubs(struct config_stub* list);

/**
 * Convert 14digit to time value
 * @param str: string of 14 digits
 * @return time value or 0 for error.
 */
uint32_t cfg_convert_timeval(const char* str);

/**
 * Count number of values in the string.
 * format ::= (sp num)+ sp
 * num ::= [-](0-9)+
 * sp ::= (space|tab)*
 *
 * @param str: string
 * @return: 0 on parse error, or empty string, else
 *	number of integer values in the string.
 */
int cfg_count_numbers(const char* str);

/**
 * Convert a 'nice' memory or file size into a bytecount
 * From '100k' to 102400. and so on. Understands kKmMgG.
 * k=1024, m=1024*1024, g=1024*1024*1024.
 * @param str: string
 * @param res: result is stored here, size in bytes.
 * @return: true if parsed correctly, or 0 on a parse error (and an error 
 * is logged).
 */
int cfg_parse_memsize(const char* str, size_t* res);

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
};

/** global config parser object used during config parsing */
extern struct config_parser_state* cfg_parser;
/** parsing helpers: print error with file and line numbers */
void ub_c_error(const char* msg);
/** parsing helpers: print error with file and line numbers */
void ub_c_error_msg(const char* fmt, ...) ATTR_FORMAT(printf, 1, 2);

#endif /* UTIL_CONFIG_FILE_H */
