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
#include <ngtcp2/ngtcp2_crypto_openssl.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include "util/locks.h"
#include "util/net_help.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "sldns/wire2str.h"
#include "util/data/msgreply.h"
#include "util/data/msgencode.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include "util/random.h"
#include "util/ub_event.h"
struct doq_client_stream_list;
struct doq_client_stream;

/** the local client data for the DoQ connection */
struct doq_client_data {
	/** file descriptor */
	int fd;
	/** the event base for the events */
	struct ub_event_base* base;
	/** the ub event */
	struct ub_event* ev;
	/** the expiry timer */
	struct ub_event* expire_timer;
	/** is the expire_timer added */
	int expire_timer_added;
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
	/** SSL context */
	SSL_CTX* ctx;
	/** SSL object */
	SSL* ssl;
	/** the quic version to use */
	uint32_t quic_version;
	/** the last error */
	struct ngtcp2_connection_close_error last_error;
	/** the recent tls alert error code */
	uint8_t tls_alert;
	/** the buffer for packet operations */
	struct sldns_buffer* pkt_buf;
	/** The list of queries to start. They have no stream associated.
	 * Once they do, they move to the send list. */
	struct doq_client_stream_list* query_list_start;
	/** The list of queries to send. They have a stream, and they are
	 * sending data. Data could also be received, like errors. */
	struct doq_client_stream_list* query_list_send;
	/** The list of queries to receive. They have a stream, and the
	 * send is done, it is possible to read data. */
	struct doq_client_stream_list* query_list_receive;
	/** The list of queries that are stopped. They have no stream
	 * active any more. Write and read are done. The query is done,
	 * and it may be in error and then have no answer or partial answer. */
	struct doq_client_stream_list* query_list_stop;
};

/** the local client stream list, for appending streams to */
struct doq_client_stream_list {
	/** first and last members of the list */
	struct doq_client_stream* first, *last;
};

/** the local client data for a DoQ stream */
struct doq_client_stream {
	/** next stream in list, and prev in list */
	struct doq_client_stream* next, *prev;
	/** the data buffer */
	uint8_t* data;
	/** length of the data buffer */
	size_t data_len;
	/** if the client query has a stream, that is active, associated with
	 * it. The stream_id is in stream_id. */
	int has_stream;
	/** the stream id */
	int64_t stream_id;
	/** data written position */
	size_t nwrite;
	/** if the write of the query data is done. That means the
	 * write channel has FIN, is closed for writing. */
	int write_is_done;
};

/** the quic method struct, must remain valid during the QUIC connection. */
static SSL_QUIC_METHOD quic_method;

/** write handle routine */
static void on_write(struct doq_client_data* data);
/** update the timer */
static void update_timer(struct doq_client_data* data);

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

/** create client stream structure */
static struct doq_client_stream*
client_stream_create(struct sldns_buffer* query_data)
{
	struct doq_client_stream* str = calloc(1, sizeof(*str));
	if(!str)
		fatal_exit("calloc failed: out of memory");
	str->data = memdup(sldns_buffer_begin(query_data),
		sldns_buffer_limit(query_data));
	if(!str->data)
		fatal_exit("alloc data failed: out of memory");
	str->data_len = sldns_buffer_limit(query_data);
	str->stream_id = -1;
	return str;
}

/** free client stream structure */
static void
client_stream_free(struct doq_client_stream* str)
{
	if(!str)
		return;
	free(str->data);
	free(str);
}

/** setup the stream to start the write process */
static void
client_stream_start_setup(struct doq_client_stream* str, uint64_t stream_id)
{
	str->has_stream = 1;
	str->stream_id = stream_id;
	str->nwrite = 0;
}

/** Return string for log purposes with query name. */
static char*
client_stream_string(struct doq_client_stream* str)
{
	char* s;
	size_t dname_len;
	char dname[256], tpstr[32], result[256+32+16];
	uint16_t tp;
	if(str->data_len <= LDNS_HEADER_SIZE) {
		s = strdup("query_with_no_question");
		if(!s)
			fatal_exit("strdup failed: out of memory");
		return s;
	}
	dname_len = dname_valid(str->data+LDNS_HEADER_SIZE,
		str->data_len-LDNS_HEADER_SIZE);
	if(!dname_len) {
		s = strdup("query_dname_not_valid");
		if(!s)
			fatal_exit("strdup failed: out of memory");
		return s;
	}
	(void)sldns_wire2str_dname_buf(str->data+LDNS_HEADER_SIZE, dname_len,
		dname, sizeof(dname));
	tp = sldns_wirerr_get_type(str->data+LDNS_HEADER_SIZE,
		str->data_len-LDNS_HEADER_SIZE, dname_len);
	(void)sldns_wire2str_type_buf(tp, tpstr, sizeof(tpstr));
	snprintf(result, sizeof(result), "%s %s", dname, tpstr);
	s = strdup(result);
	if(!s)
		fatal_exit("strdup failed: out of memory");
	return s;
}

/** create query stream list */
static struct doq_client_stream_list*
stream_list_create(void)
{
	struct doq_client_stream_list* list = calloc(1, sizeof(*list));
	if(!list)
		fatal_exit("calloc failed: out of memory");
	return list;
}

/** free the query stream list */
static void
stream_list_free(struct doq_client_stream_list* list)
{
	struct doq_client_stream* str;
	if(!list)
		return;
	str = list->first;
	while(str) {
		struct doq_client_stream* next = str->next;
		client_stream_free(str);
		str = next;
	}
	free(list);
}

/** append item to list */
static void
stream_list_append(struct doq_client_stream_list* list,
	struct doq_client_stream* str)
{
	if(list->last) {
		str->prev = list->last;
		list->last->next = str;
	} else {
		str->prev = NULL;
		list->first = str;
	}
	str->next = NULL;
	list->last = str;
}

/** delete the item from the list */
static void
stream_list_delete(struct doq_client_stream_list* list,
	struct doq_client_stream* str)
{
	if(str->next) {
		str->next->prev = str->prev;
	} else {
		list->last = str->prev;
	}
	if(str->prev) {
		str->prev->next = str->next;
	} else {
		list->first = str->next;
	}
	str->prev = NULL;
	str->next = NULL;
}

/** move the item from list1 to list2 */
static void
stream_list_move(struct doq_client_stream* str,
	struct doq_client_stream_list* list1,
	struct doq_client_stream_list* list2)
{
	stream_list_delete(list1, str);
	stream_list_append(list2, str);
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

/** send buf on the client stream */
static int
client_bidi_stream(struct doq_client_data* data, uint64_t* ret_stream_id)
{
	int64_t stream_id;
	int rv;

	/* open new bidirectional stream */
	rv = ngtcp2_conn_open_bidi_stream(data->conn, &stream_id, NULL);
	if(rv != 0) {
		if(rv == NGTCP2_ERR_STREAM_ID_BLOCKED) {
			/* no bidi stream count for this new stream */
			return 0;
		}
		fatal_exit("could not ngtcp2_conn_open_bidi_stream: %s",
			ngtcp2_strerror(rv));
	}
	*ret_stream_id = stream_id;
	return 1;
}

/** See if we can start query streams, by creating bidirectional streams
 * on the QUIC transport for them. */
static void
query_streams_start(struct doq_client_data* data)
{
	while(data->query_list_start->first) {
		struct doq_client_stream* str = data->query_list_start->first;
		uint64_t stream_id = 0;
		if(!client_bidi_stream(data, &stream_id)) {
			/* no more bidi streams allowed */
			break;
		}
		if(verbosity > 0) {
			char* logs = client_stream_string(str);
			verbose(1, "query %s start on bidi stream id %lld",
				logs, (long long int)stream_id);
			free(logs);
		}
		/* setup the stream to start */
		client_stream_start_setup(str, stream_id);
		/* move the query entry to the send list to write it */
		stream_list_move(str, data->query_list_start,
			data->query_list_send);
	}
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

/** the handshake completed callback from ngtcp2 */
static int
handshake_completed(ngtcp2_conn* ATTR_UNUSED(conn), void* user_data)
{
	struct doq_client_data* data = (struct doq_client_data*)user_data;
	verbose(1, "handshake_completed callback");
	verbose(1, "ngtcp2_conn_get_max_data_left is %d",
		(int)ngtcp2_conn_get_max_data_left(data->conn));
	verbose(1, "ngtcp2_conn_get_max_local_streams_uni is %d",
		(int)ngtcp2_conn_get_max_local_streams_uni(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_uni_left is %d",
		(int)ngtcp2_conn_get_streams_uni_left(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_bidi_left is %d",
		(int)ngtcp2_conn_get_streams_bidi_left(data->conn));
	return 0;
}

/** the extend_max_local_streams_bidi callback from ngtcp2 */
static int
extend_max_local_streams_bidi(ngtcp2_conn* ATTR_UNUSED(conn),
	uint64_t max_streams, void* user_data)
{
	struct doq_client_data* data = (struct doq_client_data*)user_data;
	verbose(1, "extend_max_local_streams_bidi callback, %d max_streams",
		(int)max_streams);
	verbose(1, "ngtcp2_conn_get_max_data_left is %d",
		(int)ngtcp2_conn_get_max_data_left(data->conn));
	verbose(1, "ngtcp2_conn_get_max_local_streams_uni is %d",
		(int)ngtcp2_conn_get_max_local_streams_uni(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_uni_left is %d",
		(int)ngtcp2_conn_get_streams_uni_left(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_bidi_left is %d",
		(int)ngtcp2_conn_get_streams_bidi_left(data->conn));
	query_streams_start(data);
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
	fprintf(stderr, "libngtcp2: ");
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

	data->quic_version = client_chosen_version;
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
	cbs.handshake_completed = handshake_completed;
	cbs.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
	cbs.encrypt = ngtcp2_crypto_encrypt_cb;
	cbs.decrypt = ngtcp2_crypto_decrypt_cb;
	cbs.hp_mask = ngtcp2_crypto_hp_mask_cb;
	cbs.recv_retry = ngtcp2_crypto_recv_retry_cb;
	cbs.extend_max_local_streams_bidi = extend_max_local_streams_bidi;
	cbs.update_key = ngtcp2_crypto_update_key_cb;
	cbs.rand = rand_cb;
	cbs.get_new_connection_id = get_new_connection_id_cb;
	cbs.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
	cbs.delete_crypto_cipher_ctx =
		ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
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

/** applicatation rx key callback, this is where the rx key is set,
 * and streams can be opened, like http3 unidirectional streams, like
 * the http3 control and http3 qpack encode and decoder streams. */
static int
application_rx_key_cb(struct doq_client_data* data)
{
	verbose(1, "application_rx_key_cb callback");
	verbose(1, "ngtcp2_conn_get_max_data_left is %d",
		(int)ngtcp2_conn_get_max_data_left(data->conn));
	verbose(1, "ngtcp2_conn_get_max_local_streams_uni is %d",
		(int)ngtcp2_conn_get_max_local_streams_uni(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_uni_left is %d",
		(int)ngtcp2_conn_get_streams_uni_left(data->conn));
	verbose(1, "ngtcp2_conn_get_streams_bidi_left is %d",
		(int)ngtcp2_conn_get_streams_bidi_left(data->conn));
	return 1;
}

/** quic_method set_encryption_secrets function */
static int
set_encryption_secrets(SSL *ssl, OSSL_ENCRYPTION_LEVEL ossl_level,
	const uint8_t *read_secret, const uint8_t *write_secret,
	size_t secret_len)
{
	struct doq_client_data* data = (struct doq_client_data*)
		SSL_get_app_data(ssl);
	ngtcp2_crypto_level level =
		ngtcp2_crypto_openssl_from_ossl_encryption_level(ossl_level);

	if(read_secret) {
		if(ngtcp2_crypto_derive_and_install_rx_key(data->conn, NULL,
			NULL, NULL, level, read_secret, secret_len) != 0) {
			log_err("ngtcp2_crypto_derive_and_install_rx_key failed");
			return 0;
		}
		if(level == NGTCP2_CRYPTO_LEVEL_APPLICATION) {
			if(!application_rx_key_cb(data))
				return 0;
		}
	}

	if(write_secret) {
		if(ngtcp2_crypto_derive_and_install_tx_key(data->conn, NULL,
			NULL, NULL, level, write_secret, secret_len) != 0) {
			log_err("ngtcp2_crypto_derive_and_install_tx_key failed");
			return 0;
		}
	}
	return 1;
}

/** quic_method add_handshake_data function */
static int
add_handshake_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL ossl_level,
	const uint8_t *data, size_t len)
{
	struct doq_client_data* doqdata = (struct doq_client_data*)
		SSL_get_app_data(ssl);
	ngtcp2_crypto_level level =
		ngtcp2_crypto_openssl_from_ossl_encryption_level(ossl_level);
	int rv;

	rv = ngtcp2_conn_submit_crypto_data(doqdata->conn, level, data, len);
	if(rv != 0) {
		log_err("ngtcp2_conn_submit_crypto_data failed: %s",
			ngtcp2_strerror(rv));
		ngtcp2_conn_set_tls_error(doqdata->conn, rv);
		return 0;
	}
	return 1;
}

/** quic_method flush_flight function */
static int
flush_flight(SSL* ATTR_UNUSED(ssl))
{
	return 1;
}

/** quic_method send_alert function */
static int
send_alert(SSL *ssl, enum ssl_encryption_level_t ATTR_UNUSED(level),
	uint8_t alert)
{
	struct doq_client_data* data = (struct doq_client_data*)
		SSL_get_app_data(ssl);
	data->tls_alert = alert;
	return 1;
}

/** setup the TLS context */
static SSL_CTX*
ctx_client_setup(void)
{
	SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
	if(!ctx) {
		log_crypto_err("Could not SSL_CTX_new");
		exit(1);
	}
	SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
	SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
	SSL_CTX_set_default_verify_paths(ctx);
	memset(&quic_method, 0, sizeof(quic_method));
	quic_method.set_encryption_secrets = &set_encryption_secrets;
	quic_method.add_handshake_data = &add_handshake_data;
	quic_method.flush_flight = &flush_flight;
	quic_method.send_alert = &send_alert;
	SSL_CTX_set_quic_method(ctx, &quic_method);
#ifdef HAVE_SSL_CTX_SET_ALPN_PROTOS
	SSL_CTX_set_alpn_protos(ctx, (const unsigned char *)"\x03doq", 4);
#endif
	return ctx;
}

/* setup the TLS object */
static SSL*
ssl_client_setup(struct doq_client_data* data)
{
	SSL* ssl = SSL_new(data->ctx);
	if(!ssl) {
		log_crypto_err("Could not SSL_new");
		exit(1);
	}
	SSL_set_app_data(ssl, data);
	SSL_set_connect_state(ssl);
	if(!SSL_set_fd(ssl, data->fd)) {
		log_crypto_err("Could not SSL_set_fd");
		exit(1);
	}
	if((data->quic_version & 0xff000000) == 0xff000000) {
		SSL_set_quic_use_legacy_codepoint(ssl, 1);
	} else {
		SSL_set_quic_use_legacy_codepoint(ssl, 0);
	}
	SSL_set_alpn_protos(ssl, (const unsigned char *)"\x03doq", 4);
	/* send the SNI host name */
	SSL_set_tlsext_host_name(ssl, "localhost");
	return ssl;
}

/** get packet ecn information */
static uint32_t
msghdr_get_ecn(struct msghdr* msg, int family)
{
#ifndef S_SPLINT_S
	struct cmsghdr* cmsg;
	if(family == AF_INET6) {
		for(cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
			cmsg = CMSG_NXTHDR(msg, cmsg)) {
			if(cmsg->cmsg_level == IPPROTO_IPV6 &&
				cmsg->cmsg_type == IPV6_TCLASS &&
				cmsg->cmsg_len != 0) {
				uint8_t* ecn = (uint8_t*)CMSG_DATA(cmsg);
				return *ecn;
			}
		}
		return 0;
	}
	for(cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
		cmsg = CMSG_NXTHDR(msg, cmsg)) {
		if(cmsg->cmsg_level == IPPROTO_IP &&
			cmsg->cmsg_type == IP_TOS &&
			cmsg->cmsg_len != 0) {
			uint8_t* ecn = (uint8_t*)CMSG_DATA(cmsg);
			return *ecn;
		}
	}
	return 0;
#endif /* S_SPLINT_S */
}

/** set the ecn on the transmission */
static void
set_ecn(int fd, int family, uint32_t ecn)
{
	unsigned int val = ecn;
	if(family == AF_INET6) {
		if(setsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &val,
			(socklen_t)sizeof(val)) == -1) {
			log_err("setsockopt(.. IPV6_TCLASS ..): %s",
				strerror(errno));
		}
		return;
	}
	if(setsockopt(fd, IPPROTO_IP, IP_TOS, &val,
		(socklen_t)sizeof(val)) == -1) {
		log_err("setsockopt(.. IP_TOS ..): %s",
			strerror(errno));
	}
}

/** send a packet */
static int
doq_send_pkt(struct doq_client_data* data, uint32_t ecn, uint8_t* buf,
	size_t buf_len)
{
	struct msghdr msg;
	struct iovec iov[1];
	ssize_t ret;
	iov[0].iov_base = buf;
	iov[0].iov_len = buf_len;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void*)&data->dest_addr;
	msg.msg_namelen = data->dest_addr_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	set_ecn(data->fd, data->dest_addr.ss_family, ecn);

	for(;;) {
		ret = sendmsg(data->fd, &msg, 0);
		if(ret == -1 && errno == EINTR)
			continue;
		break;
	}
	if(ret == -1) {
		if(errno == EAGAIN)
			return 0;
		log_err("doq sendmsg: %s", strerror(errno));
		return 0;
	}
	return 1;
}

/** change event write on fd to when we have data or when congested */
static void
event_change_write(struct doq_client_data* data, int do_write)
{
	ub_event_del(data->ev);
	if(do_write) {
		ub_event_add_bits(data->ev, UB_EV_WRITE);
	} else {
		ub_event_del_bits(data->ev, UB_EV_WRITE);
	}
	if(ub_event_add(data->ev, NULL) != 0) {
		fatal_exit("could not ub_event_add");
	}
}

/** write the connection close, with possible error */
static void
write_conn_close(struct doq_client_data* data)
{
	struct ngtcp2_path_storage ps;
	struct ngtcp2_pkt_info pi;
	ngtcp2_ssize ret;
	if(!data->conn || ngtcp2_conn_is_in_closing_period(data->conn) ||
		ngtcp2_conn_is_in_draining_period(data->conn))
		return;
	if(data->last_error.type ==
		NGTCP2_CONNECTION_CLOSE_ERROR_CODE_TYPE_TRANSPORT_IDLE_CLOSE) {
		/* do not call ngtcp2_conn_write_connection_close on the
		 * connection because the ngtcp2_conn_handle_expiry call
		 * has returned NGTCP2_ERR_IDLE_CLOSE. But continue to close
		 * the connection. */
		return;
	}
	verbose(1, "write connection close");
	ngtcp2_path_storage_zero(&ps);
	sldns_buffer_clear(data->pkt_buf);
	ret = ngtcp2_conn_write_connection_close(
		data->conn, &ps.path, &pi, sldns_buffer_begin(data->pkt_buf),
		sldns_buffer_remaining(data->pkt_buf), &data->last_error,
		get_timestamp_nanosec());
	if(ret < 0) {
		log_err("ngtcp2_conn_write_connection_close failed: %s",
			ngtcp2_strerror(ret));
		return;
	}
	verbose(1, "write connection close packet length %d", (int)ret);
	if(ret == 0)
		return;
	doq_send_pkt(data, pi.ecn, sldns_buffer_begin(data->pkt_buf), ret);
}

/** disconnect we are done */
static void
disconnect(struct doq_client_data* data)
{
	verbose(1, "disconnect");
	write_conn_close(data);
	ub_event_base_loopexit(data->base);
}

/** the expire timer callback */
void expire_timer_cb(int ATTR_UNUSED(fd),
	short ATTR_UNUSED(bits), void* arg)
{
	struct doq_client_data* data = (struct doq_client_data*)arg;
	ngtcp2_tstamp now = get_timestamp_nanosec();
	int rv;

	verbose(1, "doq expire_timer");
	data->expire_timer_added = 0;
	rv = ngtcp2_conn_handle_expiry(data->conn, now);
	if(rv != 0) {
		log_err("ngtcp2_conn_handle_expiry failed: %s",
			ngtcp2_strerror(rv));
		ngtcp2_connection_close_error_set_transport_error_liberr(
			&data->last_error, rv, NULL, 0);
		disconnect(data);
		return;
	}
	update_timer(data);
	on_write(data);
}

/** update the timers */
static void
update_timer(struct doq_client_data* data)
{
	ngtcp2_tstamp expiry = ngtcp2_conn_get_expiry(data->conn);
	ngtcp2_tstamp now = get_timestamp_nanosec();
	ngtcp2_tstamp t;
	struct timeval tv;

	if(expiry <= now) {
		/* the timer has already expired */
		expire_timer_cb(-1, UB_EV_TIMEOUT, data);
		return;
	}
	t = expiry - now;

	/* set the timer */
	if(data->expire_timer_added) {
		ub_timer_del(data->expire_timer);
		data->expire_timer_added = 0;
	}
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = t / NGTCP2_SECONDS;
	tv.tv_usec = (t / NGTCP2_MICROSECONDS)%1000000;
	verbose(1, "update_timer in %d.%6.6d secs", (int)tv.tv_sec,
		(int)tv.tv_usec);
	if(ub_timer_add(data->expire_timer, data->base,
		&expire_timer_cb, data, &tv) != 0) {
		log_err("timer_add failed: could not add expire timer");
		return;
	}
	data->expire_timer_added = 1;
}

/** perform read operations on fd */
static void
on_read(struct doq_client_data* data)
{
	struct sockaddr_storage addr;
	struct iovec iov[1];
	struct msghdr msg;
	union {
		struct cmsghdr hdr;
		char buf[256];
	} ancil;
	int i;
	ssize_t rcv;
	ngtcp2_pkt_info pi;
	int rv;
	struct ngtcp2_path path;

	for(i=0; i<10; i++) {
		msg.msg_name = &addr;
		msg.msg_namelen = (socklen_t)sizeof(addr);
		iov[0].iov_base = sldns_buffer_begin(data->pkt_buf);
		iov[0].iov_len = sldns_buffer_remaining(data->pkt_buf);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		msg.msg_control = ancil.buf;
#ifndef S_SPLINT_S
		msg.msg_controllen = sizeof(ancil.buf);
#endif /* S_SPLINT_S */
		msg.msg_flags = 0;

		rcv = recvmsg(data->fd, &msg, 0);
		if(rcv == -1) {
			if(errno == EINTR || errno == EAGAIN)
				break;
			log_err("doq recvmsg: %s", strerror(errno));
			break;
		}

		pi.ecn = msghdr_get_ecn(&msg, addr.ss_family);
		verbose(1, "recvmsg %d ecn=0x%x", (int)rcv, (int)pi.ecn);

		memset(&path, 0, sizeof(path));
		path.local.addr = (void*)&data->local_addr;
		path.local.addrlen = data->local_addr_len;
		path.remote.addr = (void*)msg.msg_name;
		path.remote.addrlen = msg.msg_namelen;
		rv = ngtcp2_conn_read_pkt(data->conn, &path, &pi,
			iov[0].iov_base, rcv, get_timestamp_nanosec());
		if(rv != 0) {
			log_err("ngtcp2_conn_read_pkt failed: %s",
				ngtcp2_strerror(rv));
			if(data->last_error.error_code == 0) {
				if(rv == NGTCP2_ERR_CRYPTO) {
					/* in picotls the tls alert may need
					 * to be copied, but this is with
					 * openssl. And we have the value
					 * data.tls_alert. */
					ngtcp2_connection_close_error_set_transport_error_tls_alert(
						&data->last_error,
						data->tls_alert, NULL, 0);
				} else {
					ngtcp2_connection_close_error_set_transport_error_liberr(
						&data->last_error, rv, NULL,
						0);
				}
			}
			disconnect(data);
			return;
		}
	}

	update_timer(data);
}

/** the write of this query has completed, it has spooled to packets,
 * set it to have the write done and move it to the list of receive streams. */
static void
query_write_is_done(struct doq_client_data* data,
	struct doq_client_stream* str)
{
	if(verbosity > 0) {
		char* logs = client_stream_string(str);
		verbose(1, "query %s write is done", logs);
		free(logs);
	}
	str->write_is_done = 1;
	stream_list_move(str, data->query_list_send, data->query_list_receive);
}

/** write the data streams, if possible */
static int
write_streams(struct doq_client_data* data)
{
	ngtcp2_path_storage ps;
	ngtcp2_tstamp ts = get_timestamp_nanosec();
	struct doq_client_stream* str, *next;
	uint32_t flags;
	ngtcp2_path_storage_zero(&ps);
	str = data->query_list_send->first;

	/* loop like this, because at the start, the send list is empty,
	 * and we want to send handshake packets. But when there is a
	 * send_list, loop through that. */
	for(;;) {

		uint64_t stream_id;
		ngtcp2_pkt_info pi;
		ngtcp2_vec datav[2];
		size_t datav_count = 0;
		int fin;
		ngtcp2_ssize ret;
		ngtcp2_ssize ndatalen = 0;
		uint16_t dnslen = 0;

		if(str) {
			/* pick up next in case this one is deleted */
			next = str->next;
			if(verbosity > 0) {
				char* logs = client_stream_string(str);
				verbose(1, "query %s write stream", logs);
				free(logs);
			}
			stream_id = str->stream_id;
			fin = 1;
			if(str->nwrite < 2) {
				dnslen = htons(str->data_len);
				datav[0].base = ((uint8_t*)&dnslen)+str->nwrite;
				datav[0].len = 2-str->nwrite;
				datav[1].base = str->data;
				datav[1].len = str->data_len;
				datav_count = 2;
			} else {
				datav[0].base = str->data + (str->nwrite-2);
				datav[0].len = str->data_len - (str->nwrite-2);
				datav_count = 1;
			}
		} else {
			next = NULL;
			verbose(1, "write stream -1.");
			stream_id = -1;
			fin = 0;
			datav[0].base = NULL;
			datav[0].len = 0;
			datav_count = 1;
		}

		flags = 0;
		if(str && str->next != NULL) {
			/* Coalesce more data from more streams into this
			 * packet, if possible */
			flags |= NGTCP2_WRITE_STREAM_FLAG_MORE;
		}
		if(fin) {
			/* This is the final part of data for this stream */
			flags |= NGTCP2_WRITE_STREAM_FLAG_FIN;
		}
		sldns_buffer_clear(data->pkt_buf);
		ret = ngtcp2_conn_writev_stream(data->conn, &ps.path, &pi,
			sldns_buffer_begin(data->pkt_buf),
			sldns_buffer_remaining(data->pkt_buf), &ndatalen,
			flags, stream_id, datav, datav_count, ts);
		if(ret < 0) {
			if(ret == NGTCP2_ERR_STREAM_DATA_BLOCKED) {
			} else if(ret == NGTCP2_ERR_STREAM_SHUT_WR) {
			} else if(ret == NGTCP2_ERR_WRITE_MORE) {
				if(str) {
					str->nwrite += ndatalen;
					if(str->nwrite >= str->data_len+2)
						query_write_is_done(data, str);
					str = next;
					continue;
				}
			}
			log_err("ngtcp2_conn_writev_stream failed: %s",
				ngtcp2_strerror(ret));
			ngtcp2_connection_close_error_set_transport_error_liberr(
				&data->last_error, ret, NULL, 0);
			disconnect(data);
			return 0;
		}
		verbose(1, "writev_stream pkt size %d ndatawritten %d",
			(int)ret, (int)ndatalen);
		if(ndatalen >= 0 && str) {
			/* add the new write offset */
			str->nwrite += ndatalen;
			if(str->nwrite >= str->data_len+2)
				query_write_is_done(data, str);
		}
		if(ret == 0) {
			/* congestion limited */
			ngtcp2_conn_update_pkt_tx_time(data->conn, ts);
			event_change_write(data, 0);
			/* update the timer to wait until we can write again */
			update_timer(data);
			return 0;
		}
		if(!doq_send_pkt(data, pi.ecn,
			sldns_buffer_begin(data->pkt_buf), ret))
			return 0;
		/* continue */
		str = next;
		if(str == NULL)
			break;
	}
	ngtcp2_conn_update_pkt_tx_time(data->conn, ts);
	event_change_write(data, 1);
	return 1;
}

/** perform write operations, if any, on fd */
static void
on_write(struct doq_client_data* data)
{
	if(!write_streams(data))
		return;
	update_timer(data);
}

/** callback for main listening file descriptor */
void
doq_client_ev_cb(int ATTR_UNUSED(fd), short bits, void* arg)
{
	struct doq_client_data* data = (struct doq_client_data*)arg;
	verbose(1, "doq_client_ev_cb %s%s%s",
		((bits&UB_EV_READ)!=0?"EV_READ":""),
		((bits&(UB_EV_READ|UB_EV_WRITE))==(UB_EV_READ|UB_EV_WRITE)?
		" ":""),
		((bits&UB_EV_WRITE)!=0?"EV_WRITE":""));
	if((bits&UB_EV_READ)) {
		on_read(data);
	}
	/* Perform the write operation anyway. The read operation may
	 * have produced data, or there is content waiting and it is possible
	 * to write that. */
	on_write(data);
}

/** create doq_client_data */
static struct doq_client_data*
create_doq_client_data(const char* svr, int port, struct ub_event_base* base)
{
	struct doq_client_data* data;
	data = calloc(sizeof(*data), 1);
	if(!data) fatal_exit("calloc failed: out of memory");
	data->base = base;
	data->rnd = ub_initstate(NULL);
	if(!data->rnd) fatal_exit("ub_initstate failed: out of memory");
	data->svr = svr;
	get_dest_addr(data, svr, port);
	data->pkt_buf = sldns_buffer_new(65552);
	if(!data->pkt_buf)
		fatal_exit("sldns_buffer_new failed: out of memory");
	data->fd = open_svr_udp(data);
	get_local_addr(data);
	data->conn = conn_client_setup(data);

	generate_static_secret(data, 32);
	data->ctx = ctx_client_setup();
	data->ssl = ssl_client_setup(data);
	ngtcp2_conn_set_tls_native_handle(data->conn, data->ssl);

	data->ev = ub_event_new(base, data->fd, UB_EV_READ | UB_EV_WRITE |
		UB_EV_PERSIST, doq_client_ev_cb, data);
	if(!data->ev) {
		fatal_exit("could not ub_event_new");
	}
	if(ub_event_add(data->ev, NULL) != 0) {
		fatal_exit("could not ub_event_add");
	}
	data->expire_timer = ub_event_new(data->base, -1,
		UB_EV_TIMEOUT, &expire_timer_cb, data);
	if(!data->expire_timer)
		fatal_exit("could not ub_event_new");
	data->query_list_start = stream_list_create();
	data->query_list_send = stream_list_create();
	data->query_list_receive = stream_list_create();
	data->query_list_stop = stream_list_create();
	return data;
}

/** delete doq_client_data */
static void
delete_doq_client_data(struct doq_client_data* data)
{
	if(!data)
		return;
#if defined(NGTCP2_USE_GENERIC_SOCKADDR) || defined(NGTCP2_USE_GENERIC_IPV6_SOCKADDR)
	if(data->conn && data->dest_addr_len != 0) {
		if(addr_is_ip6(&data->dest_addr, data->dest_addr_len)) {
#  if defined(NGTCP2_USE_GENERIC_SOCKADDR) || defined(NGTCP2_USE_GENERIC_IPV6_SOCKADDR)
			const struct ngtcp2_path* path6 = ngtcp2_conn_get_path(data->conn);
			free(path6->local.addr);
			free(path6->remote.addr);
#  endif
		} else {
#  if defined(NGTCP2_USE_GENERIC_SOCKADDR)
			const struct ngtcp2_path* path = ngtcp2_conn_get_path(data->conn);
			free(path->local.addr);
			free(path->remote.addr);
#  endif
		}
	}
#endif
	ngtcp2_conn_del(data->conn);
	SSL_free(data->ssl);
	sldns_buffer_free(data->pkt_buf);
	if(data->fd != -1)
		sock_close(data->fd);
	SSL_CTX_free(data->ctx);
	stream_list_free(data->query_list_start);
	stream_list_free(data->query_list_send);
	stream_list_free(data->query_list_receive);
	stream_list_free(data->query_list_stop);
	ub_randfree(data->rnd);
	if(data->ev) {
		ub_event_del(data->ev);
		ub_event_free(data->ev);
	}
	if(data->expire_timer_added)
		ub_timer_del(data->expire_timer);
	ub_event_free(data->expire_timer);
	free(data->static_secret_data);
	free(data);
}

/** create the event base that registers events and timers */
static struct ub_event_base*
create_event_base(time_t* secs, struct timeval* now)
{
	struct ub_event_base* base;
	const char *evnm="event", *evsys="", *evmethod="";

	memset(&now, 0, sizeof(now));
	base = ub_default_event_base(1, secs, now);
	if(!base) fatal_exit("could not create ub_event base");

	ub_get_event_sys(base, &evnm, &evsys, &evmethod);
	if(verbosity) log_info("%s %s uses %s method", evnm, evsys, evmethod);

	return base;
}

/** enter a query into the query list */
static void
client_enter_query_buf(struct doq_client_data* data, struct sldns_buffer* buf)
{
	struct doq_client_stream* str;
	str = client_stream_create(buf);
	if(!str)
		fatal_exit("client_stream_create failed: out of memory");
	stream_list_append(data->query_list_start, str);
}

/** enter the queries into the query list */
static void
client_enter_queries(struct doq_client_data* data, char** qs, int count)
{
	int i;
	for(i=0; i<count; i+=3) {
		struct sldns_buffer* buf = NULL;
		buf = make_query(qs[i], qs[i+1], qs[i+2]);
		if(verbosity > 0) {
			char* str;
			log_buf(1, "send query", buf);
			str = sldns_wire2str_pkt(sldns_buffer_begin(buf),
				sldns_buffer_limit(buf));
			if(!str) verbose(1, "could not sldns_wire2str_pkt");
			else verbose(1, "send query: %s", str);
			free(str);
		}
		client_enter_query_buf(data, buf);
		sldns_buffer_free(buf);
	}
}

/** run the dohclient queries */
static void run(const char* svr, int port, char** qs, int count)
{
	time_t secs = 0;
	struct timeval now;
	struct ub_event_base* base;
	struct doq_client_data* data;

	/* setup */
	base = create_event_base(&secs, &now);
	data = create_doq_client_data(svr, port, base);
	client_enter_queries(data, qs, count);

	/* run the queries */
	ub_event_base_dispatch(base);

	/* cleanup */
	delete_doq_client_data(data);
	ub_event_base_free(base);
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
	if(port == 53) {
		printf("Error: port number 53 not for DNS over QUIC. Port number 53 is not allowed to be used with DNS over QUIC. It is used for DNS datagrams.\n");
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
