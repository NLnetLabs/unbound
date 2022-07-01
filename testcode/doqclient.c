/*
 * testcode/doqclient.c - debug program. Perform multiple DNS queries using DoQ.
 *
 * Copyright (c) 2022, NLnet Labs. All rights reserved.
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
 * Simple DNS-over-QUIC client. For testing and debugging purposes.
 * No authentication of TLS cert.
 */

#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_NGTCP2
#include <ngtcp2/ngtcp2.h>
#include <ngtcp2/ngtcp2_crypto.h>
#include <openssl/rand.h>
#include "util/locks.h"
#include "util/net_help.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "sldns/wire2str.h"
#include "util/data/msgreply.h"
#include "util/data/msgencode.h"
#include "util/data/msgparse.h"
#include "util/random.h"

/** the local client data for the DoQ connection */
struct doq_client_data {
	/** file descriptor */
	int fd;
	/** the ngtcp2 connection information */
	struct ngtcp2_conn* conn;
	/** random state */
	struct ub_randstate* rnd;
	/** server connected to as a string */
	const char* svr;
	/** the static secret */
	uint8_t* static_secret_data;
	/** the static secret size */
	size_t static_secret_size;
	/** destination address sockaddr */
	struct sockaddr_storage dest_addr;
	/** length of dest addr */
	socklen_t dest_addr_len;
	/** local address sockaddr */
	struct sockaddr_storage local_addr;
	/** length of local addr */
	socklen_t local_addr_len;
};

/** usage of doqclient */
static void usage(char* argv[])
{
	printf("usage: %s [options] name type class ...\n", argv[0]);
	printf("	sends the name-type-class queries over "
			"DNS-over-QUIC.\n");
	printf("-s server	IP address to send the queries to, "
			"default: 127.0.0.1\n");
	printf("-p		Port to connect to, default: %d\n",
		UNBOUND_DNS_OVER_QUIC_PORT);
	printf("-v 		verbose output\n");
	printf("-h 		This help text\n");
	exit(1);
}

/** get the dest address */
static void
get_dest_addr(struct doq_client_data* data, const char* svr, int port)
{
	if(!ipstrtoaddr(svr, port, &data->dest_addr, &data->dest_addr_len)) {
		printf("fatal: bad server specs '%s'\n", svr);
		exit(1);
	}
}

/** open UDP socket to svr */
static int
open_svr_udp(struct doq_client_data* data)
{
	int fd = -1;
	int r;
	fd = socket(addr_is_ip6(&data->dest_addr, data->dest_addr_len)?
		PF_INET6:PF_INET, SOCK_DGRAM, 0);
	if(fd == -1) {
		perror("socket() error");
		exit(1);
	}
	r = connect(fd, (struct sockaddr*)&data->dest_addr,
		data->dest_addr_len);
	if(r < 0 && r != EINPROGRESS) {
		perror("connect() error");
		exit(1);
	}
	return fd;
}

/** get the local address of the connection */
static void
get_local_addr(struct doq_client_data* data)
{
	memset(&data->local_addr, 0, sizeof(data->local_addr));
	data->local_addr_len = (socklen_t)sizeof(data->local_addr);
	if(getsockname(data->fd, (struct sockaddr*)&data->local_addr,
		&data->local_addr_len) == -1) {
		perror("getsockname() error");
		exit(1);
	}
	log_addr(1, "local_addr", &data->local_addr, data->local_addr_len);
	log_addr(1, "dest_addr", &data->dest_addr, data->dest_addr_len);
}

static sldns_buffer*
make_query(char* qname, char* qtype, char* qclass)
{
	struct query_info qinfo;
	struct edns_data edns;
	sldns_buffer* buf = sldns_buffer_new(65553);
	if(!buf) fatal_exit("out of memory");
	qinfo.qname = sldns_str2wire_dname(qname, &qinfo.qname_len);
	if(!qinfo.qname) {
		printf("cannot parse query name: '%s'\n", qname);
		exit(1);
	}

	qinfo.qtype = sldns_get_rr_type_by_name(qtype);
	qinfo.qclass = sldns_get_rr_class_by_name(qclass);
	qinfo.local_alias = NULL;

	qinfo_query_encode(buf, &qinfo); /* flips buffer */
	free(qinfo.qname);
	sldns_buffer_write_u16_at(buf, 0, 0x0000);
	sldns_buffer_write_u16_at(buf, 2, BIT_RD);
	memset(&edns, 0, sizeof(edns));
	edns.edns_present = 1;
	edns.bits = EDNS_DO;
	edns.udp_size = 4096;
	if(sldns_buffer_capacity(buf) >=
		sldns_buffer_limit(buf)+calc_edns_field_size(&edns))
		attach_edns_record(buf, &edns);
	return buf;
}

/** fill a buffer with random data */
static void fill_rand(struct ub_randstate* rnd, uint8_t* buf, size_t len)
{
	if(RAND_bytes(buf, len) != 1) {
		size_t i;
		for(i=0; i<len; i++)
			buf[i] = ub_random(rnd)&0xff;
	}
}

/** create the static secret */
static void generate_static_secret(struct doq_client_data* data, size_t len)
{
	data->static_secret_data = malloc(len);
	if(!data->static_secret_data)
		fatal_exit("malloc failed: out of memory");
	data->static_secret_size = len;
	fill_rand(data->rnd, data->static_secret_data, len);
}

/** fill cid structure with random data */
static void cid_randfill(struct ngtcp2_cid* cid, size_t datalen,
	struct ub_randstate* rnd)
{
	uint8_t buf[32];
	if(datalen > sizeof(buf))
		datalen = sizeof(buf);
	fill_rand(rnd, buf, datalen);
	ngtcp2_cid_init(cid, buf, datalen);
}

/** the rand callback routine from ngtcp2 */
static void rand_cb(uint8_t* dest, size_t destlen,
	const ngtcp2_rand_ctx* rand_ctx)
{
	struct ub_randstate* rnd = (struct ub_randstate*)
		rand_ctx->native_handle;
	fill_rand(rnd, dest, destlen);
}

/** the get_new_connection_id callback routine from ngtcp2 */
static int get_new_connection_id_cb(struct ngtcp2_conn* ATTR_UNUSED(conn),
	struct ngtcp2_cid* cid, uint8_t* token, size_t cidlen, void* user_data)
{
	struct doq_client_data* data = (struct doq_client_data*)user_data;
	cid_randfill(cid, cidlen, data->rnd);
	if(ngtcp2_crypto_generate_stateless_reset_token(token,
		data->static_secret_data, data->static_secret_size, cid) != 0)
		return NGTCP2_ERR_CALLBACK_FAILURE;
	return 0;
}

/** copy sockaddr into ngtcp2 addr */
static void
copy_ngaddr(struct ngtcp2_addr* ngaddr, struct sockaddr_storage* addr,
	socklen_t addrlen)
{
	if(addr_is_ip6(addr, addrlen)) {
#if defined(NGTCP2_USE_GENERIC_SOCKADDR) || defined(NGTCP2_USE_GENERIC_IPV6_SOCKADDR)
		struct sockaddr_in* i6 = (struct sockaddr_in6*)addr;
		struct ngtcp2_sockaddr_in6 a6;
		ngaddr->addr = calloc(sizeof(a6), 1);
		if(!ngaddr->addr) fatal_exit("calloc failed: out of memory");
		ngaddr->addrlen = sizeof(a6);
		memset(&a6, 0, sizeof(a6));
		a6.sin6_family = i6->sin6_family;
		a6.sin6_port = i6->sin6_port;
		a6.sin6_flowinfo = i6->sin6_flowinfo;
		memmove(&a6.sin6_addr, i6->sin6_addr, sizeof(a6.sin6_addr);
		a6.sin6_scope_id = i6->sin6_scope_id;
		memmove(ngaddr->addr, &a6, sizeof(a6));
#else
		ngaddr->addr = (ngtcp2_sockaddr*)addr;
		ngaddr->addrlen = addrlen;
#endif
	} else {
#ifdef NGTCP2_USE_GENERIC_SOCKADDR
		struct sockaddr_in* i4 = (struct sockaddr_in*)addr;
		struct ngtcp2_sockaddr_in a4;
		ngaddr->addr = calloc(sizeof(a4), 1);
		if(!ngaddr->addr) fatal_exit("calloc failed: out of memory");
		ngaddr->addrlen = sizeof(a4);
		memset(&a4, 0, sizeof(a4));
		a4.sin_family = i4->sin_family;
		a4.sin_port = i4->sin_port;
		memmove(&a4.sin_addr, i4->sin_addr, sizeof(a4.sin_addr);
		memmove(ngaddr->addr, &a4, sizeof(a4));
#else
		ngaddr->addr = (ngtcp2_sockaddr*)addr;
		ngaddr->addrlen = addrlen;
#endif
	}
}

/** debug log printf for ngtcp2 connections */
static void log_printf_for_doq(void* ATTR_UNUSED(user_data),
	const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

/** get a timestamp in nanoseconds */
static ngtcp2_tstamp get_timestamp_nanosec(void)
{
	struct timespec tp;
	memset(&tp, 0, sizeof(tp));
	if(clock_gettime(CLOCK_MONOTONIC, &tp) == -1) {
		if(clock_gettime(CLOCK_REALTIME, &tp) == -1) {
			log_err("clock_gettime failed: %s", strerror(errno));
		}
	}
	return ((uint64_t)tp.tv_sec)*((uint64_t)1000000000) +
		((uint64_t)tp.tv_nsec);
}

/** create ngtcp2 client connection and set up. */
static struct ngtcp2_conn* conn_client_setup(struct doq_client_data* data)
{
	struct ngtcp2_conn* conn = NULL;
	int rv;
	struct ngtcp2_cid dcid, scid;
	struct ngtcp2_path path;
	uint32_t client_chosen_version = NGTCP2_PROTO_VER_V1;
	struct ngtcp2_callbacks cbs;
	struct ngtcp2_settings settings;
	struct ngtcp2_transport_params params;

	memset(&cbs, 0, sizeof(cbs));
	memset(&settings, 0, sizeof(settings));
	memset(&params, 0, sizeof(params));
	memset(&dcid, 0, sizeof(dcid));
	memset(&scid, 0, sizeof(scid));
	memset(&path, 0, sizeof(path));

	ngtcp2_settings_default(&settings);
	if(str_is_ip6(data->svr))
		settings.max_udp_payload_size = 1232;
	settings.rand_ctx.native_handle = data->rnd;
	if(verbosity > 0) {
		/* make debug logs */
		settings.log_printf = log_printf_for_doq;
	}
	settings.initial_ts = get_timestamp_nanosec();
	ngtcp2_transport_params_default(&params);
	cid_randfill(&dcid, 16, data->rnd);
	cid_randfill(&scid, 16, data->rnd);
	cbs.client_initial = ngtcp2_crypto_client_initial_cb;
	cbs.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
	cbs.encrypt = ngtcp2_crypto_encrypt_cb;
	cbs.decrypt = ngtcp2_crypto_decrypt_cb;
	cbs.hp_mask = ngtcp2_crypto_hp_mask_cb;
	cbs.recv_retry = ngtcp2_crypto_recv_retry_cb;
	cbs.update_key = ngtcp2_crypto_update_key_cb;
	cbs.rand = rand_cb;
	cbs.get_new_connection_id = get_new_connection_id_cb;
	cbs.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
	cbs.delete_crypto_cipher_ctx = ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
	cbs.get_path_challenge_data = ngtcp2_crypto_get_path_challenge_data_cb;
	cbs.version_negotiation = ngtcp2_crypto_version_negotiation_cb;
	copy_ngaddr(&path.local, &data->local_addr, data->local_addr_len);
	copy_ngaddr(&path.remote, &data->dest_addr, data->dest_addr_len);

	rv = ngtcp2_conn_client_new(&conn, &dcid, &scid, &path,
		client_chosen_version, &cbs, &settings, &params,
		NULL, /* ngtcp2_mem allocator, use default */
		data /* callback argument */);
	if(!conn) fatal_exit("could not ngtcp2_conn_client_new: %s",
		ngtcp2_strerror(rv));
	return conn;
}

/* run the dohclient queries */
static void run(const char* svr, int port, char** qs, int count)
{
	struct doq_client_data* data;
	int i;
	struct sldns_buffer* buf = NULL;
	char* str;
	data = calloc(sizeof(*data), 1);
	if(!data) fatal_exit("calloc failed: out of memory");
	data->rnd = ub_initstate(NULL);
	if(!data->rnd) fatal_exit("ub_initstate failed: out of memory");
	data->svr = svr;
	get_dest_addr(data, svr, port);
	data->fd = open_svr_udp(data);
	generate_static_secret(data, 32);
	get_local_addr(data);
	data->conn = conn_client_setup(data);

	/* handle query */
	for(i=0; i<count; i+=3) {
		buf = make_query(qs[i], qs[i+1], qs[i+2]);
		log_buf(1, "send query", buf);
		str = sldns_wire2str_pkt(sldns_buffer_begin(buf),
			sldns_buffer_limit(buf));
		if(!str) verbose(1, "could not sldns_wire2str_pkt");
		else verbose(1, "send query: %s", str);
		free(str);
	}

#if defined(NGTCP2_USE_GENERIC_SOCKADDR) || defined(NGTCP2_USE_GENERIC_IPV6_SOCKADDR)
	if(addr_is_ip6(&data->dest_addr, data->dest_addr_len)) {
#  if defined(NGTCP2_USE_GENERIC_SOCKADDR) || defined(NGTCP2_USE_GENERIC_IPV6_SOCKADDR)
		const struct ngtcp2_path* path = ngtcp2_conn_get_path(data->conn);
		free(path->local.addr);
		free(path->remote.addr);
#  endif
	} else {
#  if defined(NGTCP2_USE_GENERIC_SOCKADDR)
		const struct ngtcp2_path* path = ngtcp2_conn_get_path(data->conn);
		free(path->local.addr);
		free(path->remote.addr);
#  endif
	}
#endif
	ngtcp2_conn_del(data->conn);
	sock_close(data->fd);
	ub_randfree(data->rnd);
	free(data->static_secret_data);
	free(data);
}
#endif /* HAVE_NGTCP2 */

#ifdef HAVE_NGTCP2
/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;
int main(int ATTR_UNUSED(argc), char** ATTR_UNUSED(argv))
{
	int c;
	int port = UNBOUND_DNS_OVER_QUIC_PORT;
	const char* svr = "127.0.0.1";
#ifdef USE_WINSOCK
	WSADATA wsa_data;
	if(WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}
#endif
	checklock_start();
	log_init(0, 0, 0);

	while((c=getopt(argc, argv, "hp:s:v")) != -1) {
		switch(c) {
			case 'p':
				if(atoi(optarg)==0 && strcmp(optarg,"0")!=0) {
					printf("error parsing port, "
					    "number expected: %s\n", optarg);
					return 1;
				}
				port = atoi(optarg);
				break;
			case 's':
				svr = optarg;
				break;
			case 'v':
				verbosity++;
				break;
			case 'h':
			case '?':
			default:
				usage(argv);
		}
	}

	argc -= optind;
	argv += optind;

	if(argc%3!=0) {
		printf("Invalid input. Specify qname, qtype, and qclass.\n");
		return 1;
	}

	run(svr, port, argv, argc/3);

	checklock_stop();
#ifdef USE_WINSOCK
	WSACleanup();
#endif
	return 0;
}
#else /* HAVE_NGTCP2 */
int main(int ATTR_UNUSED(argc), char** ATTR_UNUSED(argv))
{
	printf("Compiled without ngtcp2 for QUIC, cannot run doqclient.\n");
	return 1;
}
#endif /* HAVE_NGTCP2 */
