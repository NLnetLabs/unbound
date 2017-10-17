/*
 * services/authzone.h - authoritative zone that is locally hosted.
 *
 * Copyright (c) 2017, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains the functions for an authority zone.  This zone
 * is queried by the iterator, just like a stub or forward zone, but then
 * the data is locally held.
 */

#ifndef SERVICES_AUTHZONE_H
#define SERVICES_AUTHZONE_H
#include "util/rbtree.h"
#include "util/locks.h"
struct ub_packed_rrset_key;
struct regional;
struct config_file;
struct config_auth;
struct query_info;
struct dns_msg;
struct edns_data;
struct module_env;
struct worker;
struct comm_point;
struct comm_timer;
struct auth_rrset;
struct auth_nextprobe;
struct auth_probe;
struct auth_transfer;
struct auth_master;
struct auth_chunk;

/**
 * Authoritative zones, shared.
 */
struct auth_zones {
	/** lock on the authzone trees */
	lock_rw_type lock;
	/** rbtree of struct auth_zone */
	rbtree_type ztree;
	/** rbtree of struct auth_xfer */
	rbtree_type xtree;
	/** do we have downstream enabled */
	int have_downstream;
};

/**
 * Auth zone.  Authoritative data, that is fetched from instead of sending
 * packets to the internet.
 */
struct auth_zone {
	/** rbtree node, key is name and class */
	rbnode_type node;

	/** zone name, in uncompressed wireformat */
	uint8_t* name;
	/** length of zone name */
	size_t namelen;
	/** number of labels in zone name */
	int namelabs;
	/** the class of this zone, in host byteorder.
	 * uses 'dclass' to not conflict with c++ keyword class. */
	uint16_t dclass;

	/** lock on the data in the structure
	 * For the node, parent, name, namelen, namelabs, dclass, you
	 * need to also hold the zones_tree lock to change them (or to
	 * delete this zone) */
	lock_rw_type lock;

	/** auth data for this zone
	 * rbtree of struct auth_data */
	rbtree_type data;

	/** zonefile name (or NULL for no zonefile) */
	char* zonefile;
	/** fallback to the internet on failure or ttl-expiry of auth zone */
	int fallback_enabled;
	/** the zone has expired (enabled by the xfer worker), fallback
	 * happens if that option is enabled. */
	int zone_expired;
	/** zone is a slave zone (it has masters) */
	int zone_is_slave;
	/** for downstream: this zone answers queries towards the downstream
	 * clients */
	int for_downstream;
	/** for upstream: this zone answers queries that unbound intends to
	 * send upstream. */
	int for_upstream;
};

/**
 * Auth data. One domain name, and the RRs to go with it.
 */
struct auth_data {
	/** rbtree node, key is name only */
	rbnode_type node;
	/** domain name */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** number of labels in name */
	int namelabs;
	/** the data rrsets, with different types, linked list.
	 * if the list if NULL the node would be an empty non-terminal,
	 * but in this data structure such nodes that represent an empty
	 * non-terminal are not needed; they just don't exist.
	 */
	struct auth_rrset* rrsets;
};

/**
 * A auth data RRset
 */
struct auth_rrset {
	/** next in list */
	struct auth_rrset* next;
	/** RR type in host byteorder */
	uint16_t type;
	/** RRset data item */
	struct packed_rrset_data* data;
};

/**
 * Authoritative zone transfer structure.
 * Create and destroy needs the auth_zones* biglock.
 * The structure consists of different tasks.  Each can be unowned (-1) or
 * owner by a worker (worker-num).  A worker can pick up a task and then do
 * it.  This means the events (timeouts, sockets) are for that worker.
 * 
 * (move this to tasks).
 * They don't have locks themselves, the worker (that owns it) uses it,
 * also as part of callbacks, hence it has separate zonename pointers for
 * lookup in the main zonetree.  If the zone has no transfers, this
 * structure is not created.
 */
struct auth_xfer {
	/** rbtree node, key is name and class */
	rbnode_type node;

	/** lock on this structure, and on the workernum elements of the
	 * tasks.  First hold the tree-lock in auth_zones, find the auth_xfer,
	 * lock this lock.  Then a worker can reassign itself to fill up
	 * one of the tasks. 
	 * Once it has the task assigned to it, the worker can access the
	 * other elements of the task structure without a lock, because that
	 * is necessary for the eventloop and callbacks from that. */
	lock_basic_type lock;

	/** zone name, in uncompressed wireformat */
	uint8_t* name;
	/** length of zone name */
	size_t namelen;
	/** number of labels in zone name */
	int namelabs;
	/** the class of this zone, in host byteorder.
	 * uses 'dclass' to not conflict with c++ keyword class. */
	uint16_t dclass;

	/** task to wait for next-probe-timeout,
	 * once timeouted, see if a SOA probe is needed, or already
	 * in progress */
	struct auth_nextprobe* task_nextprobe;

	/** task for SOA probe.  Check if the zone can be updated */
	struct auth_probe* task_probe;

	/** Task for transfer.  Transferring and updating the zone.  This
	 * includes trying (potentially) several upstream masters.  Downloading
	 * and storing the zone */
	struct auth_transfer* task_transfer;

	/** a notify was received, but a zone transfer or probe was already
	 * acted on.
	 * However, the zone transfer could signal a newer serial number.
	 * The serial number of that notify is saved below.  The transfer and
	 * probe tasks should check this once done to see if they need to
	 * restart the transfer task for the newer notify serial.
	 * Hold the lock to access this member (and the serial).
	 */
	int notify_received;
	/** serial number of the notify */
	uint32_t notify_serial;

	/* protected by the lock on the structure, information about
	 * the loaded authority zone. */
	/** is the zone currently considered expired? after expiry also older
         * serial numbers are allowed (not just newer) */
	int zone_expired;
	/** do we have a zone (if 0, no zone data at all) */
	int have_zone;

	/** current serial (from SOA), if we have no zone, 0 */
	uint32_t serial;
	/** retry time (from SOA), time to wait with next_probe
	 * if no master responds */
	time_t retry;
	/** refresh time (from SOA), time to wait with next_probe
	 * if everything is fine */
	time_t refresh;
	/** expiry time (from SOA), time until zone data is not considered
	 * valid any more, if no master responds within this time, either
	 * with the current zone or a new zone. */
	time_t expiry;
};

/**
 * The next probe task.
 * This task consists of waiting for the probetimeout.  It is a task because
 * it needs an event in the eventtable.  Once the timeout has passed, that
 * worker can (potentially) become the auth_probe worker, or if another worker
 * is already doing that, do nothing.  Tasks becomes unowned.
 * The probe worker, if it detects nothing has to be done picks up this task,
 * if unowned.
 */
struct auth_nextprobe {
	/** worker num (or -1 unowned) that is performing this task */
	int workernum;
	/* Worker pointer. Used by the worker during callbacks. */
	struct worker* worker;

	/** Timeout for next probe (for SOA) */
	time_t next_probe;
	/** zone lease start time (start+expiry is expiration time).
	 * this is renewed every SOA probe and transfer.  On zone load
	 * from zonefile it is also set (with probe set soon to check) */
	time_t lease_time;
	/** timeout callback for next_probe or expiry(if that is sooner).
	 * it is on the worker's event_base */
	struct comm_timer* timer;
};

/**
 * The probe task.
 * Send a SOA UDP query to see if the zone needs to be updated (or similar,
 * potential, HTTP probe query) and check serial number.
 * If yes, start the auth_transfer task.  If no, make sure auth_nextprobe
 * timeout wait task is running.
 * Needs to be a task, because the UDP query needs an event entry.
 * This task could also be started by eg. a NOTIFY being received, even though
 * another worker is performing the nextprobe task (and that worker keeps
 * waiting uninterrupted).
 */
struct auth_probe {
	/** worker num (or -1 unowned) that is performing this task */
	int workernum;
	/* Worker pointer. Used by the worker during callbacks. */
	struct worker* worker;

	/** list of upstream masters for this zone, from config */
	struct auth_master* masters;

	/** once notified, or the timeout has been reached. a scan starts. */
	/** the scan specific target (notify source), or NULL if none */
	struct auth_master* scan_specific;
	/** scan tries all the upstream masters. the scan current target. 
	 * or NULL if not working on sequential scan */
	struct auth_master* scan_target;

	/** the SOA probe udp event.
	 * on the workers event base. */
	struct comm_point* cp;
};

/**
 * The transfer task.
 * Once done, make sure the nextprobe waiting task is running, whether done
 * with failure or success.  If failure, use shorter timeout for wait time.
 */
struct auth_transfer {
	/** worker num (or -1 unowned) that is performing this task */
	int workernum;
	/* Worker pointer. Used by the worker during callbacks. */
	struct worker* worker;

	/** xfer data that has been transferred, the data is applied
	 * once the transfer has completed correctly */
	struct auth_chunk* chunks_first;
	/** last element in chunks list (to append new data at the end) */
	struct auth_chunk* chunks_last;

	/** list of upstream masters for this zone, from config */
	struct auth_master* masters;

	/** once notified, or the timeout has been reached. a scan starts. */
	/** the scan specific target (notify source), or NULL if none */
	struct auth_master* scan_specific;
	/** scan tries all the upstream masters. the scan current target. 
	 * or NULL if not working on sequential scan */
	struct auth_master* scan_target;
	/** the zone transfer in progress (or NULL if in scan).  It is
	 * from this master */
	struct auth_master* master;

	/** failed ixfr transfer, retry with axfr (to the current master),
	 * the IXFR was 'REFUSED', 'SERVFAIL', 'NOTIMPL' or the contents of
	 * the IXFR did not apply cleanly (out of sync, delete of nonexistent
	 * data or add of duplicate data).  Flag is cleared once the retry
	 * with axfr is done. */
	int ixfr_fail;

	/** the transfer (TCP) to the master.
	 * on the workers event base. */
	struct comm_point* cp;
};

/** auth zone master upstream, and the config settings for it */
struct auth_master {
	/** next master in list */
	struct auth_master* next;
	/** master IP address (and port), or hostname, string */
	char* host;
	/** for http, filename */
	char* file;
	/** use HTTP for this master */
	int http;
	/** use IXFR for this master */
	int ixfr;
	/** use ssl for channel */
	int ssl;
};

/** auth zone master zone transfer data chunk */
struct auth_chunk {
	/** next chunk in list */
	struct auth_chunk* next;
	/** the data from this chunk, this is what was received.
	 * for an IXFR that means results from comm_net tcp actions,
	 * packets. also for an AXFR. For HTTP a zonefile chunk. */
	uint8_t* data;
	/** length of allocated data */
	size_t len;
};

/**
 * Create auth zones structure
 */
struct auth_zones* auth_zones_create(void);

/**
 * Apply configuration to auth zones.  Reads zonefiles.
 */
int auth_zones_apply_cfg(struct auth_zones* az, struct config_file* cfg);

/**
 * Delete auth zones structure
 */
void auth_zones_delete(struct auth_zones* az);

/**
 * Write auth zone data to file, in zonefile format.
 */
int auth_zone_write_file(struct auth_zone* z, const char* fname);

/**
 * Use auth zones to lookup the answer to a query.
 * The query is from the iterator.  And the auth zones attempts to provide
 * the answer instead of going to the internet.
 *
 * @param az: auth zones structure.
 * @param qinfo: query info to lookup.
 * @param region: region to use to allocate the reply in.
 * @param msg: reply is stored here (if one).
 * @param fallback: if true, fallback to making a query to the internet.
 * @param dp_nm: name of delegation point to look for.  This zone is used
 *	to answer the query.
 *	If the dp_nm is not found, fallback is set to true and false returned.
 * @param dp_nmlen: length of dp_nm.
 * @return 0: failure (an error of some sort, like servfail).
 *         if 0 and fallback is true, fallback to the internet.
 *         if 0 and fallback is false, like getting servfail.
 *         If true, an answer is available.
 */
int auth_zones_lookup(struct auth_zones* az, struct query_info* qinfo,
	struct regional* region, struct dns_msg** msg, int* fallback,
	uint8_t* dp_nm, size_t dp_nmlen);

/**
 * Answer query from auth zone.  Create authoritative answer.
 * @param az: auth zones structure.
 * @param env: the module environment.
 * @param qinfo: query info (parsed).
 * @param edns: edns info (parsed).
 * @param buf: buffer with query ID and flags, also for reply.
 * @param temp: temporary storage region.
 * @return false if not answered
 */
int auth_zones_answer(struct auth_zones* az, struct module_env* env,
	struct query_info* qinfo, struct edns_data* edns, struct sldns_buffer* buf,
	struct regional* temp);

/** 
 * Find the auth zone that is above the given qname.
 * Return NULL when there is no auth_zone above the give name, otherwise
 * returns the closest auth_zone above the qname that pertains to it.
 * @param az: auth zones structure.
 * @param qinfo: query info to lookup.
 * @return NULL or auth_zone that pertains to the query.
 */
struct auth_zone* auth_zones_find_zone(struct auth_zones* az,
	struct query_info* qinfo);

/** find an auth zone by name (exact match by name or NULL returned) */
struct auth_zone* auth_zone_find(struct auth_zones* az, uint8_t* nm,
	size_t nmlen, uint16_t dclass);

/** find an xfer zone by name (exact match by name or NULL returned) */
struct auth_xfer* auth_xfer_find(struct auth_zones* az, uint8_t* nm,
	size_t nmlen, uint16_t dclass);


/** create an auth zone. returns wrlocked zone. caller must have wrlock
 * on az. returns NULL on malloc failure */
struct auth_zone* auth_zone_create(struct auth_zones* az, uint8_t* nm,
	size_t nmlen, uint16_t dclass);

/** set auth zone zonefile string. caller must have lock on zone */
int auth_zone_set_zonefile(struct auth_zone* z, char* zonefile);

/** set auth zone fallback. caller must have lock on zone.
 * fallbackstr is "yes" or "no". false on parse failure. */
int auth_zone_set_fallback(struct auth_zone* z, char* fallbackstr);

/** read auth zone from zonefile. caller must lock zone. false on failure */
int auth_zone_read_zonefile(struct auth_zone* z);

/** compare auth_zones for sorted rbtree */
int auth_zone_cmp(const void* z1, const void* z2);

/** compare auth_data for sorted rbtree */
int auth_data_cmp(const void* z1, const void* z2);

/** compare auth_xfer for sorted rbtree */
int auth_xfer_cmp(const void* z1, const void* z2);

/** Create auth_xfer structure.
 * Caller must have wrlock on az. Returns locked xfer zone.
 * @param az: zones structure.
 * @param z: zone with name and class
 * @return xfer zone or NULL
 */
struct auth_xfer* auth_xfer_create(struct auth_zones* az, struct auth_zone* z);

/**
 * Set masters in auth xfer structure from config.
 * @param list: pointer to start of list.  The malloced list is returned here.
 * @param c: the config items to copy over.
 * @return false on failure.
 */
int xfer_set_masters(struct auth_master** list, struct config_auth* c);

/**
 * Setup auth_xfer zone.  Populates timeouts.
 * @param z: locked by caller, and modified for setup
 * @param x: locked by caller, and modified, timers and timeouts.
 * @param env: module env with time.
 * @return false on failure.
 */
int auth_xfer_setup(struct auth_zone* z, struct auth_xfer* xfr,
	struct module_env* env);

/**
 * Setup all zones
 * @param az: auth zones structure
 * @param env: module env with time.
 * @return false on failure.
 */
int auth_zones_setup_zones(struct auth_zones* az, struct module_env* env);

#endif /* SERVICES_AUTHZONE_H */
