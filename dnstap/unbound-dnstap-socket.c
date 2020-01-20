/*
 * dnstap/unbound-dnstap-socket.c - debug program that listens for DNSTAP logs.
 *
 * Copyright (c) 2020, NLnet Labs. All rights reserved.
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
 * This program listens on a DNSTAP socket for logged messages.
 */
#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include "dnstap/dtstream.h"
#include "util/log.h"
#include "util/ub_event.h"
#include "util/net_help.h"
#include "services/listen_dnsport.h"
#include "sldns/sbuffer.h"
#include "sldns/wire2str.h"
#include <protobuf-c/protobuf-c.h>
#include "dnstap/dnstap.pb-c.h"

#define DNSTAP_CONTENT_TYPE             "protobuf:dnstap.Dnstap"

/** usage information for streamtcp */
static void usage(char* argv[])
{
	printf("usage: %s [options]\n", argv[0]);
	printf(" 	Listen to dnstap messages\n");
	printf("stdout has dnstap log, stderr has verbose server log\n");
	printf("-u <socketpath> use unix socket with this file name\n");
	printf("-l 		long format for DNS printout\n");
	printf("-v 		more verbose log output\n");
	printf("-h 		this help text\n");
	exit(1);
}

/** long format option, for multiline printout per message */
static int longformat = 0;

/** tap callback variables */
struct tap_data {
	/** the fd */
	int fd;
	/** the ub event */
	struct ub_event* ev;
	/** have we read the length, and how many bytes of it */
	int len_done;
	/** have we read the data, and how many bytes of it */
	size_t data_done;
	/** are we reading a control frame */
	int control_frame;
	/** are we bi-directional (if false, uni-directional) */
	int is_bidirectional;
	/** data of the frame */
	uint8_t* frame;
	/** length of this frame */
	size_t len;
};

/** log control frame contents */
static void log_control_frame(uint8_t* pkt, size_t len)
{
	uint32_t frametype = 0;
	char buf[256];
	size_t at = 0, remain;
	uint8_t* pos;
	if(verbosity == 0) return;
	if(len < 4) {
		log_err("malformed control frame, too short, len=%d", (int)len);
		return;
	}
	buf[0]=0;
	frametype = sldns_read_uint32(pkt);
	if(frametype == FSTRM_CONTROL_FRAME_ACCEPT) {
		at+=snprintf(buf+at, sizeof(buf)-at, "accept");
	} else if(frametype == FSTRM_CONTROL_FRAME_START) {
		at+=snprintf(buf+at, sizeof(buf)-at, "start");
	} else if(frametype == FSTRM_CONTROL_FRAME_STOP) {
		at+=snprintf(buf+at, sizeof(buf)-at, "stop");
	} else if(frametype == FSTRM_CONTROL_FRAME_READY) {
		at+=snprintf(buf+at, sizeof(buf)-at, "ready");
	} else if(frametype == FSTRM_CONTROL_FRAME_FINISH) {
		at+=snprintf(buf+at, sizeof(buf)-at, "finish");
	} else {
		at+=snprintf(buf+at, sizeof(buf)-at, "type%d",
			(int)frametype);
	}

	/* show the content type options */
	pos = pkt + 4;
	remain = len - 4;
	while(remain >= 8) {
		uint32_t field_type = sldns_read_uint32(pos);
		uint32_t field_len = sldns_read_uint32(pos+4);
		if(remain < field_len) {
			at+=snprintf(buf+at, sizeof(buf)-at, "malformed_field");
			break;
		}
		if(field_type == FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE) {
			at+=snprintf(buf+at, sizeof(buf)-at, " content-type(");
			if(at+field_len < sizeof(buf)) {
				memmove(buf+at, pos+8, field_len);
				at += field_len;
			}
			at+=snprintf(buf+at, sizeof(buf)-at, ")");
		}
		pos += 8 + field_len;
		remain -= (8 + field_len);
	}
	log_info("control frame %s", buf);
}

/** convert mtype to string */
static const char* mtype_to_str(enum _Dnstap__Message__Type mtype)
{
	switch(mtype) {
		case DNSTAP__MESSAGE__TYPE__AUTH_QUERY:
			return "AUTH_QUERY";
		case DNSTAP__MESSAGE__TYPE__AUTH_RESPONSE:
			return "AUTH_RESPONSE";
		case DNSTAP__MESSAGE__TYPE__RESOLVER_QUERY:
			return "RESOLVER_QUERY";
		case DNSTAP__MESSAGE__TYPE__RESOLVER_RESPONSE:
			return "RESOLVER_RESPONSE";
		case DNSTAP__MESSAGE__TYPE__CLIENT_QUERY:
			return "CLIENT_QUERY";
		case DNSTAP__MESSAGE__TYPE__CLIENT_RESPONSE:
			return "CLIENT_RESPONSE";
		case DNSTAP__MESSAGE__TYPE__FORWARDER_QUERY:
			return "FORWARDER_QUERY";
		case DNSTAP__MESSAGE__TYPE__FORWARDER_RESPONSE:
			return "FORWARDER_RESPONSE";
		case DNSTAP__MESSAGE__TYPE__STUB_QUERY:
			return "STUB_QUERY";
		case DNSTAP__MESSAGE__TYPE__STUB_RESPONSE:
			return "STUB_RESPONSE";
		default: break;
	}
	return "unknown_message_type";
}

/** convert type address to a string ip4 or ip6, malloced or NULL on fail */
static char* str_of_addr(ProtobufCBinaryData address)
{
	char buf[64];
	socklen_t len = sizeof(buf);
	if(address.len == 4) {
		if(inet_ntop(AF_INET, address.data, buf, len)!=0)
			return strdup(buf);
	} else if(address.len == 16) {
		if(inet_ntop(AF_INET6, address.data, buf, len)!=0)
			return strdup(buf);
	}
	return NULL;
}

/** convert message buffer (of dns bytes) to the first qname, type, class,
 * malloced or NULL on fail */
static char* q_of_msg(ProtobufCBinaryData message)
{
	char buf[300];
	/* header, name, type, class minimum to get the query tuple */
	if(message.len < 12 + 1 + 4 + 4) return NULL;
	if(sldns_wire2str_rrquestion_buf(message.data+12, message.len-12,
		buf, sizeof(buf)) != 0) {
		/* remove trailing newline, tabs to spaces */
		if(buf[0] != 0) buf[strlen(buf)-1]=0;
		if(strrchr(buf, '\t')) *strrchr(buf, '\t')=' ';
		if(strrchr(buf, '\t')) *strrchr(buf, '\t')=' ';
		return strdup(buf);
	}
	return NULL;
}

/** log data frame contents */
static void log_data_frame(uint8_t* pkt, size_t len)
{
	Dnstap__Dnstap* d = dnstap__dnstap__unpack(NULL, len, pkt);
	const char* mtype = NULL;
	char* maddr=NULL, *qinf=NULL;
	if(!d) {
		log_err("could not unpack");
		return;
	}
	if(d->base.descriptor != &dnstap__dnstap__descriptor) {
		log_err("wrong base descriptor");
		dnstap__dnstap__free_unpacked(d, NULL);
		return;
	}
	if(d->type != DNSTAP__DNSTAP__TYPE__MESSAGE) {
		log_err("dnstap type not type_message");
		dnstap__dnstap__free_unpacked(d, NULL);
		return;
	}
	if(d->message) {
		mtype = mtype_to_str(d->message->type);
		if(d->message->has_query_address)
			maddr = str_of_addr(d->message->query_address);
		else if(d->message->has_response_address)
			maddr = str_of_addr(d->message->response_address);
		if(d->message->has_query_message)
			qinf = q_of_msg(d->message->query_message);
		else if(d->message->has_response_message)
			qinf = q_of_msg(d->message->response_message);

	} else {
		mtype = "nomessage";
	}
	
	printf("%s%s%s%s%s\n", mtype, (maddr?" ":""), (maddr?maddr:""),
		(qinf?" ":""), (qinf?qinf:""));
	free(maddr);
	free(qinf);

	if(longformat) {
		if(d->has_identity) {
		}
		if(d->has_version) {
		}
	}
	dnstap__dnstap__free_unpacked(d, NULL);
}

/** receive bytes from fd, prints errors if bad,
 * returns 0: closed/error, -1: continue, >0 number of bytes */
static ssize_t receive_bytes(int fd, void* buf, size_t len)
{
	ssize_t ret = recv(fd, buf, len, 0);
	if(ret == 0) {
		/* closed */
		if(verbosity) log_info("dnstap client stream closed");
		return 0;
	} else if(ret == -1) {
		/* error */
#ifndef USE_WINSOCK
		if(errno == EINTR || errno == EAGAIN)
			return -1;
		log_err("could not recv: %s", strerror(errno));
#else /* USE_WINSOCK */
		if(WSAGetLastError() == WSAEINPROGRESS)
			return -1;
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			ub_winsock_tcp_wouldblock(data->ev,
				UB_EV_READ);
			return -1;
		}
		log_err("could not recv: %s",
			wsa_strerror(WSAGetLastError()));
#endif
		if(verbosity) log_info("dnstap client stream closed");
		return 0;
	}
	return ret;
}

/** delete the tap structure */
void tap_data_free(struct tap_data* data)
{
	ub_event_del(data->ev);
	ub_event_free(data->ev);
	close(data->fd);
	free(data->frame);
	free(data);
}

/** reply with ACCEPT control frame to bidirectional client,
 * returns 0 on error */
static int reply_with_accept(int fd)
{
	/* control frame on reply:
	 * 4 bytes 0 escape
	 * 4 bytes bigendian length of frame
	 * 4 bytes bigendian type ACCEPT
	 * 4 bytes bigendian frame option content type
	 * 4 bytes bigendian length of string
	 * string of content type.
	 */
	uint32_t* acceptframe;
	/* len includes the escape and framelength */
	size_t len = 4+4+4+4+4+strlen(DNSTAP_CONTENT_TYPE);
	acceptframe = calloc(1, len);
	if(!acceptframe) {
		log_err("out of memory");
		return 0;
	}
	acceptframe[0] = 0;
	acceptframe[1] = htonl(4+4+4+strlen(DNSTAP_CONTENT_TYPE));
	acceptframe[2] = htonl(FSTRM_CONTROL_FRAME_ACCEPT);
	acceptframe[3] = htonl(FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE);
	acceptframe[4] = htonl(strlen(DNSTAP_CONTENT_TYPE));
	memmove(&acceptframe[5], DNSTAP_CONTENT_TYPE,
		strlen(DNSTAP_CONTENT_TYPE));

	fd_set_block(fd);
	if(send(fd, acceptframe, len, 0) == -1) {
#ifndef USE_WINSOCK
		log_err("send failed: %s", strerror(errno));
#else
		log_err("send failed: %s", wsa_strerror(WSAGetLastError()));
#endif
		fd_set_nonblock(fd);
		free(acceptframe);
		return 0;
	}
	if(verbosity) log_info("sent control frame(accept) content-type:(%s)",
			DNSTAP_CONTENT_TYPE);

	fd_set_nonblock(fd);
	free(acceptframe);
	return 1;
}

/** reply with FINISH control frame to bidirectional client,
 * returns 0 on error */
static int reply_with_finish(int fd)
{
	/* control frame on reply:
	 * 4 bytes 0 escape
	 * 4 bytes bigendian length of frame
	 * 4 bytes bigendian type FINISH
	 */
	uint32_t finishframe[3];
	/* len includes the escape and framelength */
	size_t len = 4+4+4;
	finishframe[0] = 0;
	finishframe[1] = htonl(4);
	finishframe[2] = htonl(FSTRM_CONTROL_FRAME_FINISH);

	fd_set_block(fd);
	if(send(fd, finishframe, len, 0) == -1) {
#ifndef USE_WINSOCK
		log_err("send failed: %s", strerror(errno));
#else
		log_err("send failed: %s", wsa_strerror(WSAGetLastError()));
#endif
		fd_set_nonblock(fd);
		return 0;
	}
	if(verbosity) log_info("sent control frame(finish)");

	fd_set_nonblock(fd);
	return 1;
}

/** callback for dnstap listener */
static void tap_callback(int fd, short ATTR_UNUSED(bits), void* arg)
{
	struct tap_data* data = (struct tap_data*)arg;
	if(verbosity>=3) log_info("tap callback");
	while(data->len_done < 4) {
		uint32_t l = (uint32_t)data->len;
		ssize_t ret = receive_bytes(fd,
			((uint8_t*)&l)+data->len_done, 4-data->len_done);
		if(verbosity>=4) log_info("s recv %d", (int)ret);
		if(ret == 0) {
			/* closed or error */
			tap_data_free(data);
			return;
		} else if(ret == -1) {
			/* continue later */
			return;
		}
		data->len_done += ret;
		data->len = (size_t)l;
		if(data->len_done < 4)
			return; /* continue later */
		data->len = (size_t)(ntohl(l));
		if(verbosity>=3) log_info("length is %d", (int)data->len);
		if(data->len == 0) {
			/* it is a control frame */
			data->control_frame = 1;
			/* read controlframelen */
			data->len_done = 0;
		} else {
			/* allocate frame size */
			data->frame = calloc(1, data->len);
			if(!data->frame) {
				log_err("out of memory");
				tap_data_free(data);
				return;
			}
		}
	}

	/* we want to read the full length now */
	if(data->data_done < data->len) {
		ssize_t r = receive_bytes(fd, data->frame + data->data_done,
			data->len - data->data_done);
		if(verbosity>=4) log_info("f recv %d", (int)r);
		if(r == 0) {
			/* closed or error */
			tap_data_free(data);
			return;
		} else if(r == -1) {
			/* continue later */
			return;
		}
		data->data_done += r;
		if(data->data_done < data->len)
			return; /* continue later */
	}

	/* we are done with a frame */
	if(verbosity>=3) log_info("received %sframe len %d",
		(data->control_frame?"control ":""), (int)data->len);
	if(data->control_frame)
		log_control_frame(data->frame, data->len);
	else	log_data_frame(data->frame, data->len);

	if(data->len >= 4 && sldns_read_uint32(data->frame) ==
		FSTRM_CONTROL_FRAME_READY) {
		data->is_bidirectional = 1;
		if(verbosity) log_info("bidirectional stream");
		if(!reply_with_accept(fd)) {
			tap_data_free(data);
		}
	} else if(data->len >= 4 && sldns_read_uint32(data->frame) ==
		FSTRM_CONTROL_FRAME_STOP && data->is_bidirectional) {
		if(!reply_with_finish(fd)) {
			tap_data_free(data);
			return;
		}
	}

	/* prepare for next frame */
	free(data->frame);
	data->frame = NULL;
	data->control_frame = 0;
	data->len = 0;
	data->len_done = 0;
	data->data_done = 0;

}

/** callback for main listening file descriptor */
void mainfdcallback(int fd, short ATTR_UNUSED(bits), void* arg)
{
	struct tap_data* data;
	struct sockaddr_storage addr;
	socklen_t addrlen = (socklen_t)sizeof(addr);
	struct ub_event_base* base = (struct ub_event_base*)arg;
	int s = accept(fd, (struct sockaddr*)&addr, &addrlen);
	if(s == -1) {
#ifndef USE_WINSOCK
                /* EINTR is signal interrupt. others are closed connection. */
                if(     errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
                        || errno == EWOULDBLOCK
#endif
#ifdef ECONNABORTED
                        || errno == ECONNABORTED
#endif
#ifdef EPROTO
                        || errno == EPROTO
#endif /* EPROTO */
                        )
                        return;
		log_err_addr("accept failed", strerror(errno), &addr, addrlen);
#else /* USE_WINSOCK */
                if(WSAGetLastError() == WSAEINPROGRESS ||
                        WSAGetLastError() == WSAECONNRESET)
                        return;
                if(WSAGetLastError() == WSAEWOULDBLOCK) {
                        ub_winsock_tcp_wouldblock(c->ev->ev, UB_EV_READ);
                        return;
                }
                log_err_addr("accept failed", wsa_strerror(WSAGetLastError()),
                        addr, *addrlen);
#endif
                return;
	}
	fd_set_nonblock(s);
	if(verbosity) {
		if(addr.ss_family == AF_LOCAL) {
			struct sockaddr_un* usock = calloc(1, sizeof(struct sockaddr_un) + 1);
			if(usock) {
				socklen_t ulen = sizeof(struct sockaddr_un);
				if(getsockname(fd, (struct sockaddr*)usock, &ulen) != -1) {
					log_info("accepted new dnstap client from %s", usock->sun_path);
				} else {
					log_info("accepted new dnstap client");
				}
				free(usock);
			} else {
				log_info("accepted new dnstap client");
			}
		} else {
			log_info("accepted new dnstap client");
		}
	}
	
	data = calloc(1, sizeof(*data));
	if(!data) fatal_exit("out of memory");
	data->fd = s;
	data->ev = ub_event_new(base, s, UB_EV_READ | UB_EV_PERSIST,
		&tap_callback, data);
	if(!data->ev) fatal_exit("could not ub_event_new");
	if(ub_event_add(data->ev, NULL) != 0) fatal_exit("could not ub_event_add");
}

/** create file descriptor to listen on */
static int
setup_fd(char* socketpath)
{
	return create_local_accept_sock(socketpath, NULL, 0);
}

/** setup and run the server to listen to DNSTAP messages */
static void
setup_and_run(char* socketpath)
{
	int fd;
	time_t secs = 0;
	struct timeval now;
	struct ub_event_base* base;
	const char *evnm="event", *evsys="", *evmethod="";
	struct ub_event *ev;

	memset(&now, 0, sizeof(now));
	base = ub_default_event_base(1, &secs, &now);
	if(!base) fatal_exit("could not create ub_event base");
	fd = setup_fd(socketpath);
	ub_get_event_sys(base, &evnm, &evsys, &evmethod);
	if(verbosity) log_info("%s %s uses %s method", evnm, evsys, evmethod);
	ev = ub_event_new(base, fd, UB_EV_READ | UB_EV_PERSIST,
		&mainfdcallback, base);
	if(!ev) fatal_exit("could not ub_event_new");
	if(ub_event_add(ev, NULL) != 0) fatal_exit("could not ub_event_add");

	ub_event_base_dispatch(base);

	ub_event_del(ev);
	ub_event_free(ev);
	ub_event_base_free(base);
	close(fd);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** main program for streamtcp */
int main(int argc, char** argv) 
{
	int c;
	int usessl = 0;
	char* socketpath = NULL;
#ifdef USE_WINSOCK
	WSADATA wsa_data;
	if(WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}
#endif

	/* lock debug start (if any) */
	log_ident_set("unbound-dnstap-socket");
	log_init(0, 0, 0);
	checklock_start();

#ifdef SIGPIPE
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("could not install signal handler for SIGPIPE");
		return 1;
	}
#endif

	/* command line options */
	while( (c=getopt(argc, argv, "hlu:v")) != -1) {
		switch(c) {
			case 'u':
				socketpath = optarg;
				break;
			case 'l':
				longformat = 1;
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

	if(usessl) {
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_SSL)
		ERR_load_SSL_strings();
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_CRYPTO)
#  ifndef S_SPLINT_S
		OpenSSL_add_all_algorithms();
#  endif
#else
		OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS
			| OPENSSL_INIT_ADD_ALL_DIGESTS
			| OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_SSL)
		(void)SSL_library_init();
#else
		(void)OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
#endif
	}
	setup_and_run(socketpath);
	checklock_stop();
#ifdef USE_WINSOCK
	WSACleanup();
#endif
	return 0;
}
