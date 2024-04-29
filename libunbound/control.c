#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#endif
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif
#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif
#include "util/log.h"
#include "util/config_file.h"
#include "util/locks.h"
#include "util/net_help.h"
#include "util/shm_side/shm_main.h"
#include "util/timeval_func.h"
#include "daemon/stats.h"
#include "sldns/wire2str.h"
#include "sldns/pkthdr.h"
#include "services/rpz.h"
#include "services/listen_dnsport.h"

#ifdef HAVE_SYS_IPC_H
#include "sys/ipc.h"
#endif
#ifdef HAVE_SYS_SHM_H
#include "sys/shm.h"
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_TARGETCONDITIONALS_H
#include <TargetConditionals.h>
#endif

static void ssl_err(const char* s) ATTR_NORETURN;
static void ssl_path_err(const char* s, const char *path) ATTR_NORETURN;

/** timeout to wait for connection over stream, in msec */
#define UNBOUND_CONTROL_CONNECT_TIMEOUT 5000

/** exit with ssl error */
static void ssl_err(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
	ERR_print_errors_fp(stderr);
	exit(1);
}

/** exit with ssl error related to a file path */
static void ssl_path_err(const char* s, const char *path)
{
	unsigned long err;
	err = ERR_peek_error();
	if(ERR_GET_LIB(err) == ERR_LIB_SYS) {
		fprintf(stderr, "error: %s\n%s: %s\n",
			s, path, ERR_reason_error_string(err));
		exit(1);
	} else {
		ssl_err(s);
	}
}

/** setup SSL context */
static SSL_CTX*
setup_ctx(struct config_file* cfg)
{
	char* s_cert=NULL, *c_key=NULL, *c_cert=NULL;
	SSL_CTX* ctx;

	if(!(options_remote_is_address(cfg) && cfg->control_use_cert))
		return NULL;
	s_cert = fname_after_chroot(cfg->server_cert_file, cfg, 1);
	c_key = fname_after_chroot(cfg->control_key_file, cfg, 1);
	c_cert = fname_after_chroot(cfg->control_cert_file, cfg, 1);
	if(!s_cert || !c_key || !c_cert)
		fatal_exit("out of memory");
	ctx = SSL_CTX_new(SSLv23_client_method());
	if(!ctx)
		ssl_err("could not allocate SSL_CTX pointer");
#if SSL_OP_NO_SSLv2 != 0
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2) & SSL_OP_NO_SSLv2)
		!= SSL_OP_NO_SSLv2)
		ssl_err("could not set SSL_OP_NO_SSLv2");
#endif
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3) & SSL_OP_NO_SSLv3)
		!= SSL_OP_NO_SSLv3)
		ssl_err("could not set SSL_OP_NO_SSLv3");
#if defined(SSL_OP_NO_RENEGOTIATION)
	/* disable client renegotiation */
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_RENEGOTIATION) &
		SSL_OP_NO_RENEGOTIATION) != SSL_OP_NO_RENEGOTIATION)
		ssl_err("could not set SSL_OP_NO_RENEGOTIATION");
#endif
	if(!SSL_CTX_use_certificate_chain_file(ctx,c_cert))
		ssl_path_err("Error setting up SSL_CTX client cert", c_cert);
	if(!SSL_CTX_use_PrivateKey_file(ctx,c_key,SSL_FILETYPE_PEM))
		ssl_path_err("Error setting up SSL_CTX client key", c_key);
	if(!SSL_CTX_check_private_key(ctx))
		ssl_err("Error setting up SSL_CTX client key");
	if(SSL_CTX_load_verify_locations(ctx, s_cert, NULL) != 1)
		ssl_path_err("Error setting up SSL_CTX verify, server cert",
			     s_cert);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	free(s_cert);
	free(c_key);
	free(c_cert);
	return ctx;
}

/** check connect error */
static void
checkconnecterr(int err, const char* svr, struct sockaddr_storage* addr,
	socklen_t addrlen, int statuscmd, int useport)
{
#ifndef USE_WINSOCK
	if(!useport) log_err("connect: %s for %s", strerror(err), svr);
	else log_err_addr("connect", strerror(err), addr, addrlen);
	if(err == ECONNREFUSED && statuscmd) {
		printf("unbound is stopped\n");
		exit(3);
	}
#else
	int wsaerr = err;
	if(!useport) log_err("connect: %s for %s", wsa_strerror(wsaerr), svr);
	else log_err_addr("connect", wsa_strerror(wsaerr), addr, addrlen);
	if(wsaerr == WSAECONNREFUSED && statuscmd) {
		printf("unbound is stopped\n");
		exit(3);
	}
#endif
	exit(1);
}

/** contact the server with TCP connect */
static int
contact_server(const char* svr, struct config_file* cfg, int statuscmd)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int addrfamily = 0, proto = IPPROTO_TCP;
	int fd, useport = 1;
	char** rcif = NULL;
	int num_rcif = 0;
	/* use svr or the first config entry */
	if(!svr) {
		if(cfg->control_ifs.first) {
			struct sockaddr_storage addr2;
			socklen_t addrlen2;
			if(extstrtoaddr(cfg->control_ifs.first->str, &addr2,
				&addrlen2, UNBOUND_DNS_PORT)) {
				svr = cfg->control_ifs.first->str;
			} else {
				if(!resolve_interface_names(NULL, 0,
					cfg->control_ifs.first, &rcif,
					&num_rcif)) {
					fatal_exit("could not resolve interface names");
				}
				if(rcif == NULL || num_rcif == 0) {
					fatal_exit("no control interfaces");
				}
				svr = rcif[0];
			}
		} else if(cfg->do_ip4) {
			svr = "127.0.0.1";
		} else {
			svr = "::1";
		}
		/* config 0 addr (everything), means ask localhost */
		if(strcmp(svr, "0.0.0.0") == 0)
			svr = "127.0.0.1";
		else if(strcmp(svr, "::0") == 0 ||
			strcmp(svr, "0::0") == 0 ||
			strcmp(svr, "0::") == 0 ||
			strcmp(svr, "::") == 0)
			svr = "::1";
	}
	if(strchr(svr, '@')) {
		if(!extstrtoaddr(svr, &addr, &addrlen, UNBOUND_DNS_PORT))
			fatal_exit("could not parse IP@port: %s", svr);
#ifdef HAVE_SYS_UN_H
	} else if(svr[0] == '/') {
		struct sockaddr_un* usock = (struct sockaddr_un *) &addr;
		usock->sun_family = AF_LOCAL;
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
		usock->sun_len = (unsigned)sizeof(usock);
#endif
		(void)strlcpy(usock->sun_path, svr, sizeof(usock->sun_path));
		addrlen = (socklen_t)sizeof(struct sockaddr_un);
		addrfamily = AF_LOCAL;
		useport = 0;
		proto = 0;
#endif
	} else {
		if(!ipstrtoaddr(svr, cfg->control_port, &addr, &addrlen))
			fatal_exit("could not parse IP: %s", svr);
	}

	if(addrfamily == 0)
		addrfamily = addr_is_ip6(&addr, addrlen)?PF_INET6:PF_INET;
	fd = socket(addrfamily, SOCK_STREAM, proto);
	if(fd == -1) {
		fatal_exit("socket: %s", sock_strerror(errno));
	}
	fd_set_nonblock(fd);
	if(connect(fd, (struct sockaddr*)&addr, addrlen) < 0) {
#ifndef USE_WINSOCK
#ifdef EINPROGRESS
		if(errno != EINPROGRESS) {
			checkconnecterr(errno, svr, &addr,
				addrlen, statuscmd, useport);
		}
#endif
#else
		if(WSAGetLastError() != WSAEINPROGRESS &&
			WSAGetLastError() != WSAEWOULDBLOCK) {
			checkconnecterr(WSAGetLastError(), svr, &addr,
				addrlen, statuscmd, useport);
		}
#endif
	}
	while(1) {
		fd_set rset, wset, eset;
		struct timeval tv;
		FD_ZERO(&rset);
		FD_SET(FD_SET_T fd, &rset);
		FD_ZERO(&wset);
		FD_SET(FD_SET_T fd, &wset);
		FD_ZERO(&eset);
		FD_SET(FD_SET_T fd, &eset);
		tv.tv_sec = UNBOUND_CONTROL_CONNECT_TIMEOUT/1000;
		tv.tv_usec= (UNBOUND_CONTROL_CONNECT_TIMEOUT%1000)*1000;
		if(select(fd+1, &rset, &wset, &eset, &tv) == -1) {
			fatal_exit("select: %s", sock_strerror(errno));
		}
		if(!FD_ISSET(fd, &rset) && !FD_ISSET(fd, &wset) &&
			!FD_ISSET(fd, &eset)) {
			fatal_exit("timeout: could not connect to server");
		} else {
			/* check nonblocking connect error */
			int error = 0;
			socklen_t len = (socklen_t)sizeof(error);
			if(getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&error,
				&len) < 0) {
#ifndef USE_WINSOCK
				error = errno; /* on solaris errno is error */
#else
				error = WSAGetLastError();
#endif
			}
			if(error != 0) {
#ifndef USE_WINSOCK
#ifdef EINPROGRESS
				if(error == EINPROGRESS)
					continue; /* try again later */
#endif
#ifdef EWOULDBLOCK
				if(error == EWOULDBLOCK)
					continue; /* try again later */
#endif
#else
				if(error == WSAEINPROGRESS)
					continue; /* try again later */
				if(error == WSAEWOULDBLOCK)
					continue; /* try again later */
#endif
				checkconnecterr(error, svr, &addr, addrlen,
					statuscmd, useport);
			}
		}
		break;
	}
	fd_set_block(fd);
	config_del_strarray(rcif, num_rcif);
	return fd;
}

/** setup SSL on the connection */
static SSL*
setup_ssl(SSL_CTX* ctx, int fd)
{
	SSL* ssl;
	X509* x;
	int r;

	if(!ctx) return NULL;
	ssl = SSL_new(ctx);
	if(!ssl)
		ssl_err("could not SSL_new");
	SSL_set_connect_state(ssl);
	(void)SSL_set_mode(ssl, (long)SSL_MODE_AUTO_RETRY);
	if(!SSL_set_fd(ssl, fd))
		ssl_err("could not SSL_set_fd");
	while(1) {
		ERR_clear_error();
		if( (r=SSL_do_handshake(ssl)) == 1)
			break;
		r = SSL_get_error(ssl, r);
		if(r != SSL_ERROR_WANT_READ && r != SSL_ERROR_WANT_WRITE)
			ssl_err("SSL handshake failed");
		/* wants to be called again */
	}

	/* check authenticity of server */
	if(SSL_get_verify_result(ssl) != X509_V_OK)
		ssl_err("SSL verification failed");
	x = SSL_get_peer_certificate(ssl);
	if(!x)
		ssl_err("Server presented no peer certificate");
	X509_free(x);

	return ssl;
}

/** read from ssl or fd, fatalexit on error, 0 EOF, 1 success */
static int
remote_read(SSL* ssl, int fd, char* buf, size_t len)
{
	if(ssl) {
		int r, e;
		ERR_clear_error();
		if((r = SSL_read(ssl, buf, (int)len-1)) <= 0) {
			if((e = SSL_get_error(ssl, r)) == SSL_ERROR_ZERO_RETURN) {
				/* EOF */
				return 0;
			}
            printf("ret = %d, err = %d", r, e);
			ssl_err("could not SSL_read");
		}
		buf[r] = 0;
	} else {
		ssize_t rr = recv(fd, buf, len-1, 0);
		if(rr <= 0) {
			if(rr == 0) {
				/* EOF */
				return 0;
			}
			fatal_exit("could not recv: %s", sock_strerror(errno));
		}
		buf[rr] = 0;
	}
	return 1;
}

/** write to ssl or fd, fatalexit on error */
static void
remote_write(SSL* ssl, int fd, const char* buf, size_t len)
{
	if(ssl) {
		if(SSL_write(ssl, buf, (int)len) <= 0)
			ssl_err("could not SSL_write");
	} else {
		if(send(fd, buf, len, 0) < (ssize_t)len) {
			fatal_exit("could not send: %s", sock_strerror(errno));
		}
	}
}

/** check args, to see if too many args. Because when a file is sent it
 * would wait for the terminal, and we can check for too many arguments,
 * eg. user put arguments on the commandline. */
static void
check_args_for_listcmd(int argc, char* argv[])
{
	if(argc >= 1 && (strcmp(argv[0], "local_zones") == 0 ||
		strcmp(argv[0], "local_zones_remove") == 0 ||
		strcmp(argv[0], "local_datas") == 0 ||
		strcmp(argv[0], "local_datas_remove") == 0) &&
		argc >= 2) {
		fatal_exit("too many arguments for command '%s', "
			"content is piped in from stdin", argv[0]);
	}
	if(argc >= 1 && (strcmp(argv[0], "view_local_datas") == 0 ||
		strcmp(argv[0], "view_local_datas_remove") == 0) &&
		argc >= 3) {
		fatal_exit("too many arguments for command '%s', "
			"content is piped in from stdin", argv[0]);
	}
}

/** send stdin to server */
static void
send_file(SSL* ssl, int fd, FILE* in, char* buf, size_t sz)
{
	while(fgets(buf, (int)sz, in)) {
		remote_write(ssl, fd, buf, strlen(buf));
	}
}

/** send end-of-file marker to server */
static void
send_eof(SSL* ssl, int fd)
{
	char e[] = {0x04, 0x0a};
	remote_write(ssl, fd, e, sizeof(e));
}

/** send command and display result */
static int
go_cmd(SSL* ssl, int fd, int quiet, int argc, char* argv[])
{
	char pre[10];
	const char* space=" ";
	const char* newline="\n";
	int was_error = 0, first_line = 1;
	int i;
	char buf[1024];
	snprintf(pre, sizeof(pre), "UBCT%d ", UNBOUND_CONTROL_VERSION);
	remote_write(ssl, fd, pre, strlen(pre));
	for(i=0; i<argc; i++) {
		remote_write(ssl, fd, space, strlen(space));
		remote_write(ssl, fd, argv[i], strlen(argv[i]));
	}
	remote_write(ssl, fd, newline, strlen(newline));

	if(argc == 1 && strcmp(argv[0], "load_cache") == 0) {
		send_file(ssl, fd, stdin, buf, sizeof(buf));
	}
	else if(argc >= 1 && (strcmp(argv[0], "local_zones") == 0 ||
		strcmp(argv[0], "local_zones_remove") == 0 ||
		strcmp(argv[0], "local_datas") == 0 ||
		strcmp(argv[0], "view_local_datas") == 0 ||
		strcmp(argv[0], "local_datas_remove") == 0 ||
		strcmp(argv[0], "view_local_datas_remove") == 0)) {
		send_file(ssl, fd, stdin, buf, sizeof(buf));
		send_eof(ssl, fd);
	}

	while(1) {
		if(remote_read(ssl, fd, buf, sizeof(buf)) == 0) {
			break; /* EOF */
		}
		if(first_line && strncmp(buf, "error", 5) == 0) {
			printf("%s", buf);
			was_error = 1;
		} else if(!quiet) {
			printf("%s", buf);
		}

		first_line = 0;
	}
	return was_error;
}

struct ubc_ctx *
ubc_ctx_create(const char* cfgfile, const char *svr)
{
    struct ubc_ctx* ctx;
#ifdef USE_WINSOCK
	int r;
	WSADATA wsa_data;
#endif

	log_ident_set("libunbound");
	log_init(NULL, 0, NULL);

#ifdef USE_WINSOCK
	if((r = WSAStartup(MAKEWORD(2,2), &wsa_data)) != 0)
		fatal_exit("WSAStartup failed: %s", wsa_strerror(r));
#endif

#ifdef HAVE_ERR_LOAD_CRYPTO_STRINGS
	ERR_load_crypto_strings();
#endif
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

	if(!RAND_status()) {
		/* try to seed it */
		unsigned char buf[256];
		unsigned int seed=(unsigned)time(NULL) ^ (unsigned)getpid();
		unsigned int v = seed;
		size_t i;
		for(i=0; i<256/sizeof(v); i++) {
			memmove(buf+i*sizeof(v), &v, sizeof(v));
			v = v*seed + (unsigned int)i;
		}
		RAND_seed(buf, 256);
		log_warn("no entropy, seeding openssl PRNG with time\n");
	}

    if ((ctx = (struct ubc_ctx*)malloc(sizeof(*ctx))) == NULL)
        fatal_exit("out of memory");

    if (!(ctx->cfg = config_create()))
        fatal_exit("out of memory");

    if (!cfgfile)
        cfgfile = CONFIGFILE;

    if (!config_read(ctx->cfg, cfgfile, NULL))
        fatal_exit("could not read config file");

    if (!ctx->cfg->remote_control_enable)
        log_warn("control-enable is 'no' in the config file.");

#ifdef UB_ON_WINDOWS
    w_config_adjust_directory(ctx->cfg);
#endif

    ctx->ssl_ctx = setup_ctx(ctx->cfg);

    ctx->quiet = 0;

    return (ctx);
}

void
ubc_ctx_delete(struct ubc_ctx* ctx)
{
    if (ctx->ssl_ctx)
        SSL_CTX_free(ctx->ssl_ctx);

    if (ctx->cfg)
        config_delete(ctx->cfg);

    free(ctx);

#ifdef USE_WINSOCK
	WSACleanup();
#endif
}

int
ubc_ctx_do_cmd(struct ubc_ctx* ctx, int argc, char** argv)
{
    int fd;
    SSL* ssl;

    /* contact server */
    fd = contact_server(NULL, ctx->cfg, 0);
    ssl = setup_ssl(ctx->ssl_ctx, fd);

    /* execute the cmd */
    int ret = go_cmd(ssl, fd, ctx->quiet, argc, argv);

    /* cleanup */
    SSL_free(ssl);
    sock_close(fd);

    return (ret);
}

int
ubc_ctx_data_add(struct ubc_ctx* ctx, char* data)
{
    char *argv[] = {
        "local_data",
        data
    };

    return (ubc_ctx_do_cmd(ctx, 2, argv));
}

int
ubc_ctx_get_quiet(struct ubc_ctx* ctx)
{
    return (ctx->quiet);
}

void
ubc_ctx_set_quiet(struct ubc_ctx *ctx, int quiet)
{
    ctx->quiet = quiet;
}
