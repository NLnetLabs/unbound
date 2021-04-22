/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 38 "./util/configparser.y"

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/net_help.h"

int ub_c_lex(void);
void ub_c_error(const char *message);

static void validate_respip_action(const char* action);

/* these need to be global, otherwise they cannot be used inside yacc */
extern struct config_parser_state* cfg_parser;

#if 0
#define OUTYY(s)  printf s /* used ONLY when debugging */
#else
#define OUTYY(s)
#endif


#line 99 "util/configparser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_UTIL_CONFIGPARSER_H_INCLUDED
# define YY_YY_UTIL_CONFIGPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SPACE = 258,
    LETTER = 259,
    NEWLINE = 260,
    COMMENT = 261,
    COLON = 262,
    ANY = 263,
    ZONESTR = 264,
    STRING_ARG = 265,
    VAR_FORCE_TOPLEVEL = 266,
    VAR_SERVER = 267,
    VAR_VERBOSITY = 268,
    VAR_NUM_THREADS = 269,
    VAR_PORT = 270,
    VAR_OUTGOING_RANGE = 271,
    VAR_INTERFACE = 272,
    VAR_PREFER_IP4 = 273,
    VAR_DO_IP4 = 274,
    VAR_DO_IP6 = 275,
    VAR_PREFER_IP6 = 276,
    VAR_DO_UDP = 277,
    VAR_DO_TCP = 278,
    VAR_TCP_MSS = 279,
    VAR_OUTGOING_TCP_MSS = 280,
    VAR_TCP_IDLE_TIMEOUT = 281,
    VAR_EDNS_TCP_KEEPALIVE = 282,
    VAR_EDNS_TCP_KEEPALIVE_TIMEOUT = 283,
    VAR_CHROOT = 284,
    VAR_USERNAME = 285,
    VAR_DIRECTORY = 286,
    VAR_LOGFILE = 287,
    VAR_PIDFILE = 288,
    VAR_MSG_CACHE_SIZE = 289,
    VAR_MSG_CACHE_SLABS = 290,
    VAR_NUM_QUERIES_PER_THREAD = 291,
    VAR_RRSET_CACHE_SIZE = 292,
    VAR_RRSET_CACHE_SLABS = 293,
    VAR_OUTGOING_NUM_TCP = 294,
    VAR_INFRA_HOST_TTL = 295,
    VAR_INFRA_LAME_TTL = 296,
    VAR_INFRA_CACHE_SLABS = 297,
    VAR_INFRA_CACHE_NUMHOSTS = 298,
    VAR_INFRA_CACHE_LAME_SIZE = 299,
    VAR_NAME = 300,
    VAR_STUB_ZONE = 301,
    VAR_STUB_HOST = 302,
    VAR_STUB_ADDR = 303,
    VAR_TARGET_FETCH_POLICY = 304,
    VAR_HARDEN_SHORT_BUFSIZE = 305,
    VAR_HARDEN_LARGE_QUERIES = 306,
    VAR_FORWARD_ZONE = 307,
    VAR_FORWARD_HOST = 308,
    VAR_FORWARD_ADDR = 309,
    VAR_DO_NOT_QUERY_ADDRESS = 310,
    VAR_HIDE_IDENTITY = 311,
    VAR_HIDE_VERSION = 312,
    VAR_IDENTITY = 313,
    VAR_VERSION = 314,
    VAR_HARDEN_GLUE = 315,
    VAR_MODULE_CONF = 316,
    VAR_TRUST_ANCHOR_FILE = 317,
    VAR_TRUST_ANCHOR = 318,
    VAR_VAL_OVERRIDE_DATE = 319,
    VAR_BOGUS_TTL = 320,
    VAR_VAL_CLEAN_ADDITIONAL = 321,
    VAR_VAL_PERMISSIVE_MODE = 322,
    VAR_INCOMING_NUM_TCP = 323,
    VAR_MSG_BUFFER_SIZE = 324,
    VAR_KEY_CACHE_SIZE = 325,
    VAR_KEY_CACHE_SLABS = 326,
    VAR_TRUSTED_KEYS_FILE = 327,
    VAR_VAL_NSEC3_KEYSIZE_ITERATIONS = 328,
    VAR_USE_SYSLOG = 329,
    VAR_OUTGOING_INTERFACE = 330,
    VAR_ROOT_HINTS = 331,
    VAR_DO_NOT_QUERY_LOCALHOST = 332,
    VAR_CACHE_MAX_TTL = 333,
    VAR_HARDEN_DNSSEC_STRIPPED = 334,
    VAR_ACCESS_CONTROL = 335,
    VAR_LOCAL_ZONE = 336,
    VAR_LOCAL_DATA = 337,
    VAR_INTERFACE_AUTOMATIC = 338,
    VAR_STATISTICS_INTERVAL = 339,
    VAR_DO_DAEMONIZE = 340,
    VAR_USE_CAPS_FOR_ID = 341,
    VAR_STATISTICS_CUMULATIVE = 342,
    VAR_OUTGOING_PORT_PERMIT = 343,
    VAR_OUTGOING_PORT_AVOID = 344,
    VAR_DLV_ANCHOR_FILE = 345,
    VAR_DLV_ANCHOR = 346,
    VAR_NEG_CACHE_SIZE = 347,
    VAR_HARDEN_REFERRAL_PATH = 348,
    VAR_PRIVATE_ADDRESS = 349,
    VAR_PRIVATE_DOMAIN = 350,
    VAR_REMOTE_CONTROL = 351,
    VAR_CONTROL_ENABLE = 352,
    VAR_CONTROL_INTERFACE = 353,
    VAR_CONTROL_PORT = 354,
    VAR_SERVER_KEY_FILE = 355,
    VAR_SERVER_CERT_FILE = 356,
    VAR_CONTROL_KEY_FILE = 357,
    VAR_CONTROL_CERT_FILE = 358,
    VAR_CONTROL_USE_CERT = 359,
    VAR_EXTENDED_STATISTICS = 360,
    VAR_LOCAL_DATA_PTR = 361,
    VAR_JOSTLE_TIMEOUT = 362,
    VAR_STUB_PRIME = 363,
    VAR_UNWANTED_REPLY_THRESHOLD = 364,
    VAR_LOG_TIME_ASCII = 365,
    VAR_DOMAIN_INSECURE = 366,
    VAR_PYTHON = 367,
    VAR_PYTHON_SCRIPT = 368,
    VAR_VAL_SIG_SKEW_MIN = 369,
    VAR_VAL_SIG_SKEW_MAX = 370,
    VAR_CACHE_MIN_TTL = 371,
    VAR_VAL_LOG_LEVEL = 372,
    VAR_AUTO_TRUST_ANCHOR_FILE = 373,
    VAR_KEEP_MISSING = 374,
    VAR_ADD_HOLDDOWN = 375,
    VAR_DEL_HOLDDOWN = 376,
    VAR_SO_RCVBUF = 377,
    VAR_EDNS_BUFFER_SIZE = 378,
    VAR_PREFETCH = 379,
    VAR_PREFETCH_KEY = 380,
    VAR_SO_SNDBUF = 381,
    VAR_SO_REUSEPORT = 382,
    VAR_HARDEN_BELOW_NXDOMAIN = 383,
    VAR_IGNORE_CD_FLAG = 384,
    VAR_LOG_QUERIES = 385,
    VAR_LOG_REPLIES = 386,
    VAR_LOG_LOCAL_ACTIONS = 387,
    VAR_TCP_UPSTREAM = 388,
    VAR_SSL_UPSTREAM = 389,
    VAR_SSL_SERVICE_KEY = 390,
    VAR_SSL_SERVICE_PEM = 391,
    VAR_SSL_PORT = 392,
    VAR_FORWARD_FIRST = 393,
    VAR_STUB_SSL_UPSTREAM = 394,
    VAR_FORWARD_SSL_UPSTREAM = 395,
    VAR_TLS_CERT_BUNDLE = 396,
    VAR_HTTPS_PORT = 397,
    VAR_HTTP_ENDPOINT = 398,
    VAR_HTTP_MAX_STREAMS = 399,
    VAR_HTTP_QUERY_BUFFER_SIZE = 400,
    VAR_HTTP_RESPONSE_BUFFER_SIZE = 401,
    VAR_HTTP_NODELAY = 402,
    VAR_HTTP_NOTLS_DOWNSTREAM = 403,
    VAR_STUB_FIRST = 404,
    VAR_MINIMAL_RESPONSES = 405,
    VAR_RRSET_ROUNDROBIN = 406,
    VAR_MAX_UDP_SIZE = 407,
    VAR_DELAY_CLOSE = 408,
    VAR_UDP_CONNECT = 409,
    VAR_UNBLOCK_LAN_ZONES = 410,
    VAR_INSECURE_LAN_ZONES = 411,
    VAR_INFRA_CACHE_MIN_RTT = 412,
    VAR_INFRA_KEEP_PROBING = 413,
    VAR_DNS64_PREFIX = 414,
    VAR_DNS64_SYNTHALL = 415,
    VAR_DNS64_IGNORE_AAAA = 416,
    VAR_DNSTAP = 417,
    VAR_DNSTAP_ENABLE = 418,
    VAR_DNSTAP_SOCKET_PATH = 419,
    VAR_DNSTAP_IP = 420,
    VAR_DNSTAP_TLS = 421,
    VAR_DNSTAP_TLS_SERVER_NAME = 422,
    VAR_DNSTAP_TLS_CERT_BUNDLE = 423,
    VAR_DNSTAP_TLS_CLIENT_KEY_FILE = 424,
    VAR_DNSTAP_TLS_CLIENT_CERT_FILE = 425,
    VAR_DNSTAP_SEND_IDENTITY = 426,
    VAR_DNSTAP_SEND_VERSION = 427,
    VAR_DNSTAP_BIDIRECTIONAL = 428,
    VAR_DNSTAP_IDENTITY = 429,
    VAR_DNSTAP_VERSION = 430,
    VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES = 431,
    VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES = 432,
    VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES = 433,
    VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES = 434,
    VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES = 435,
    VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES = 436,
    VAR_RESPONSE_IP_TAG = 437,
    VAR_RESPONSE_IP = 438,
    VAR_RESPONSE_IP_DATA = 439,
    VAR_HARDEN_ALGO_DOWNGRADE = 440,
    VAR_IP_TRANSPARENT = 441,
    VAR_IP_DSCP = 442,
    VAR_DISABLE_DNSSEC_LAME_CHECK = 443,
    VAR_IP_RATELIMIT = 444,
    VAR_IP_RATELIMIT_SLABS = 445,
    VAR_IP_RATELIMIT_SIZE = 446,
    VAR_RATELIMIT = 447,
    VAR_RATELIMIT_SLABS = 448,
    VAR_RATELIMIT_SIZE = 449,
    VAR_RATELIMIT_FOR_DOMAIN = 450,
    VAR_RATELIMIT_BELOW_DOMAIN = 451,
    VAR_IP_RATELIMIT_FACTOR = 452,
    VAR_RATELIMIT_FACTOR = 453,
    VAR_SEND_CLIENT_SUBNET = 454,
    VAR_CLIENT_SUBNET_ZONE = 455,
    VAR_CLIENT_SUBNET_ALWAYS_FORWARD = 456,
    VAR_CLIENT_SUBNET_OPCODE = 457,
    VAR_MAX_CLIENT_SUBNET_IPV4 = 458,
    VAR_MAX_CLIENT_SUBNET_IPV6 = 459,
    VAR_MIN_CLIENT_SUBNET_IPV4 = 460,
    VAR_MIN_CLIENT_SUBNET_IPV6 = 461,
    VAR_MAX_ECS_TREE_SIZE_IPV4 = 462,
    VAR_MAX_ECS_TREE_SIZE_IPV6 = 463,
    VAR_CAPS_WHITELIST = 464,
    VAR_CACHE_MAX_NEGATIVE_TTL = 465,
    VAR_PERMIT_SMALL_HOLDDOWN = 466,
    VAR_QNAME_MINIMISATION = 467,
    VAR_QNAME_MINIMISATION_STRICT = 468,
    VAR_IP_FREEBIND = 469,
    VAR_DEFINE_TAG = 470,
    VAR_LOCAL_ZONE_TAG = 471,
    VAR_ACCESS_CONTROL_TAG = 472,
    VAR_LOCAL_ZONE_OVERRIDE = 473,
    VAR_ACCESS_CONTROL_TAG_ACTION = 474,
    VAR_ACCESS_CONTROL_TAG_DATA = 475,
    VAR_VIEW = 476,
    VAR_ACCESS_CONTROL_VIEW = 477,
    VAR_MATCH_CLIENTS = 478,
    VAR_MATCH_DESTINATIONS = 479,
    VAR_VIEW_FIRST = 480,
    VAR_SERVE_EXPIRED = 481,
    VAR_SERVE_EXPIRED_TTL = 482,
    VAR_SERVE_EXPIRED_TTL_RESET = 483,
    VAR_SERVE_EXPIRED_REPLY_TTL = 484,
    VAR_SERVE_EXPIRED_CLIENT_TIMEOUT = 485,
    VAR_SERVE_ORIGINAL_TTL = 486,
    VAR_FAKE_DSA = 487,
    VAR_FAKE_SHA1 = 488,
    VAR_LOG_IDENTITY = 489,
    VAR_HIDE_TRUSTANCHOR = 490,
    VAR_TRUST_ANCHOR_SIGNALING = 491,
    VAR_AGGRESSIVE_NSEC = 492,
    VAR_USE_SYSTEMD = 493,
    VAR_SHM_ENABLE = 494,
    VAR_SHM_KEY = 495,
    VAR_ROOT_KEY_SENTINEL = 496,
    VAR_DNSCRYPT = 497,
    VAR_DNSCRYPT_ENABLE = 498,
    VAR_DNSCRYPT_PORT = 499,
    VAR_DNSCRYPT_PROVIDER = 500,
    VAR_DNSCRYPT_SECRET_KEY = 501,
    VAR_DNSCRYPT_PROVIDER_CERT = 502,
    VAR_DNSCRYPT_PROVIDER_CERT_ROTATED = 503,
    VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE = 504,
    VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS = 505,
    VAR_DNSCRYPT_NONCE_CACHE_SIZE = 506,
    VAR_DNSCRYPT_NONCE_CACHE_SLABS = 507,
    VAR_PAD_RESPONSES = 508,
    VAR_PAD_RESPONSES_BLOCK_SIZE = 509,
    VAR_PAD_QUERIES = 510,
    VAR_PAD_QUERIES_BLOCK_SIZE = 511,
    VAR_IPSECMOD_ENABLED = 512,
    VAR_IPSECMOD_HOOK = 513,
    VAR_IPSECMOD_IGNORE_BOGUS = 514,
    VAR_IPSECMOD_MAX_TTL = 515,
    VAR_IPSECMOD_WHITELIST = 516,
    VAR_IPSECMOD_STRICT = 517,
    VAR_CACHEDB = 518,
    VAR_CACHEDB_BACKEND = 519,
    VAR_CACHEDB_SECRETSEED = 520,
    VAR_CACHEDB_REDISHOST = 521,
    VAR_CACHEDB_REDISPORT = 522,
    VAR_CACHEDB_REDISTIMEOUT = 523,
    VAR_CACHEDB_REDISEXPIRERECORDS = 524,
    VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM = 525,
    VAR_FOR_UPSTREAM = 526,
    VAR_AUTH_ZONE = 527,
    VAR_ZONEFILE = 528,
    VAR_MASTER = 529,
    VAR_URL = 530,
    VAR_FOR_DOWNSTREAM = 531,
    VAR_FALLBACK_ENABLED = 532,
    VAR_TLS_ADDITIONAL_PORT = 533,
    VAR_LOW_RTT = 534,
    VAR_LOW_RTT_PERMIL = 535,
    VAR_FAST_SERVER_PERMIL = 536,
    VAR_FAST_SERVER_NUM = 537,
    VAR_ALLOW_NOTIFY = 538,
    VAR_TLS_WIN_CERT = 539,
    VAR_TCP_CONNECTION_LIMIT = 540,
    VAR_FORWARD_NO_CACHE = 541,
    VAR_STUB_NO_CACHE = 542,
    VAR_LOG_SERVFAIL = 543,
    VAR_DENY_ANY = 544,
    VAR_UNKNOWN_SERVER_TIME_LIMIT = 545,
    VAR_LOG_TAG_QUERYREPLY = 546,
    VAR_STREAM_WAIT_SIZE = 547,
    VAR_TLS_CIPHERS = 548,
    VAR_TLS_CIPHERSUITES = 549,
    VAR_TLS_USE_SNI = 550,
    VAR_IPSET = 551,
    VAR_IPSET_NAME_V4 = 552,
    VAR_IPSET_NAME_V6 = 553,
    VAR_TLS_SESSION_TICKET_KEYS = 554,
    VAR_RPZ = 555,
    VAR_TAGS = 556,
    VAR_RPZ_ACTION_OVERRIDE = 557,
    VAR_RPZ_CNAME_OVERRIDE = 558,
    VAR_RPZ_LOG = 559,
    VAR_RPZ_LOG_NAME = 560,
    VAR_DYNLIB = 561,
    VAR_DYNLIB_FILE = 562,
    VAR_EDNS_CLIENT_STRING = 563,
    VAR_EDNS_CLIENT_STRING_OPCODE = 564,
    VAR_NSID = 565
  };
#endif
/* Tokens.  */
#define SPACE 258
#define LETTER 259
#define NEWLINE 260
#define COMMENT 261
#define COLON 262
#define ANY 263
#define ZONESTR 264
#define STRING_ARG 265
#define VAR_FORCE_TOPLEVEL 266
#define VAR_SERVER 267
#define VAR_VERBOSITY 268
#define VAR_NUM_THREADS 269
#define VAR_PORT 270
#define VAR_OUTGOING_RANGE 271
#define VAR_INTERFACE 272
#define VAR_PREFER_IP4 273
#define VAR_DO_IP4 274
#define VAR_DO_IP6 275
#define VAR_PREFER_IP6 276
#define VAR_DO_UDP 277
#define VAR_DO_TCP 278
#define VAR_TCP_MSS 279
#define VAR_OUTGOING_TCP_MSS 280
#define VAR_TCP_IDLE_TIMEOUT 281
#define VAR_EDNS_TCP_KEEPALIVE 282
#define VAR_EDNS_TCP_KEEPALIVE_TIMEOUT 283
#define VAR_CHROOT 284
#define VAR_USERNAME 285
#define VAR_DIRECTORY 286
#define VAR_LOGFILE 287
#define VAR_PIDFILE 288
#define VAR_MSG_CACHE_SIZE 289
#define VAR_MSG_CACHE_SLABS 290
#define VAR_NUM_QUERIES_PER_THREAD 291
#define VAR_RRSET_CACHE_SIZE 292
#define VAR_RRSET_CACHE_SLABS 293
#define VAR_OUTGOING_NUM_TCP 294
#define VAR_INFRA_HOST_TTL 295
#define VAR_INFRA_LAME_TTL 296
#define VAR_INFRA_CACHE_SLABS 297
#define VAR_INFRA_CACHE_NUMHOSTS 298
#define VAR_INFRA_CACHE_LAME_SIZE 299
#define VAR_NAME 300
#define VAR_STUB_ZONE 301
#define VAR_STUB_HOST 302
#define VAR_STUB_ADDR 303
#define VAR_TARGET_FETCH_POLICY 304
#define VAR_HARDEN_SHORT_BUFSIZE 305
#define VAR_HARDEN_LARGE_QUERIES 306
#define VAR_FORWARD_ZONE 307
#define VAR_FORWARD_HOST 308
#define VAR_FORWARD_ADDR 309
#define VAR_DO_NOT_QUERY_ADDRESS 310
#define VAR_HIDE_IDENTITY 311
#define VAR_HIDE_VERSION 312
#define VAR_IDENTITY 313
#define VAR_VERSION 314
#define VAR_HARDEN_GLUE 315
#define VAR_MODULE_CONF 316
#define VAR_TRUST_ANCHOR_FILE 317
#define VAR_TRUST_ANCHOR 318
#define VAR_VAL_OVERRIDE_DATE 319
#define VAR_BOGUS_TTL 320
#define VAR_VAL_CLEAN_ADDITIONAL 321
#define VAR_VAL_PERMISSIVE_MODE 322
#define VAR_INCOMING_NUM_TCP 323
#define VAR_MSG_BUFFER_SIZE 324
#define VAR_KEY_CACHE_SIZE 325
#define VAR_KEY_CACHE_SLABS 326
#define VAR_TRUSTED_KEYS_FILE 327
#define VAR_VAL_NSEC3_KEYSIZE_ITERATIONS 328
#define VAR_USE_SYSLOG 329
#define VAR_OUTGOING_INTERFACE 330
#define VAR_ROOT_HINTS 331
#define VAR_DO_NOT_QUERY_LOCALHOST 332
#define VAR_CACHE_MAX_TTL 333
#define VAR_HARDEN_DNSSEC_STRIPPED 334
#define VAR_ACCESS_CONTROL 335
#define VAR_LOCAL_ZONE 336
#define VAR_LOCAL_DATA 337
#define VAR_INTERFACE_AUTOMATIC 338
#define VAR_STATISTICS_INTERVAL 339
#define VAR_DO_DAEMONIZE 340
#define VAR_USE_CAPS_FOR_ID 341
#define VAR_STATISTICS_CUMULATIVE 342
#define VAR_OUTGOING_PORT_PERMIT 343
#define VAR_OUTGOING_PORT_AVOID 344
#define VAR_DLV_ANCHOR_FILE 345
#define VAR_DLV_ANCHOR 346
#define VAR_NEG_CACHE_SIZE 347
#define VAR_HARDEN_REFERRAL_PATH 348
#define VAR_PRIVATE_ADDRESS 349
#define VAR_PRIVATE_DOMAIN 350
#define VAR_REMOTE_CONTROL 351
#define VAR_CONTROL_ENABLE 352
#define VAR_CONTROL_INTERFACE 353
#define VAR_CONTROL_PORT 354
#define VAR_SERVER_KEY_FILE 355
#define VAR_SERVER_CERT_FILE 356
#define VAR_CONTROL_KEY_FILE 357
#define VAR_CONTROL_CERT_FILE 358
#define VAR_CONTROL_USE_CERT 359
#define VAR_EXTENDED_STATISTICS 360
#define VAR_LOCAL_DATA_PTR 361
#define VAR_JOSTLE_TIMEOUT 362
#define VAR_STUB_PRIME 363
#define VAR_UNWANTED_REPLY_THRESHOLD 364
#define VAR_LOG_TIME_ASCII 365
#define VAR_DOMAIN_INSECURE 366
#define VAR_PYTHON 367
#define VAR_PYTHON_SCRIPT 368
#define VAR_VAL_SIG_SKEW_MIN 369
#define VAR_VAL_SIG_SKEW_MAX 370
#define VAR_CACHE_MIN_TTL 371
#define VAR_VAL_LOG_LEVEL 372
#define VAR_AUTO_TRUST_ANCHOR_FILE 373
#define VAR_KEEP_MISSING 374
#define VAR_ADD_HOLDDOWN 375
#define VAR_DEL_HOLDDOWN 376
#define VAR_SO_RCVBUF 377
#define VAR_EDNS_BUFFER_SIZE 378
#define VAR_PREFETCH 379
#define VAR_PREFETCH_KEY 380
#define VAR_SO_SNDBUF 381
#define VAR_SO_REUSEPORT 382
#define VAR_HARDEN_BELOW_NXDOMAIN 383
#define VAR_IGNORE_CD_FLAG 384
#define VAR_LOG_QUERIES 385
#define VAR_LOG_REPLIES 386
#define VAR_LOG_LOCAL_ACTIONS 387
#define VAR_TCP_UPSTREAM 388
#define VAR_SSL_UPSTREAM 389
#define VAR_SSL_SERVICE_KEY 390
#define VAR_SSL_SERVICE_PEM 391
#define VAR_SSL_PORT 392
#define VAR_FORWARD_FIRST 393
#define VAR_STUB_SSL_UPSTREAM 394
#define VAR_FORWARD_SSL_UPSTREAM 395
#define VAR_TLS_CERT_BUNDLE 396
#define VAR_HTTPS_PORT 397
#define VAR_HTTP_ENDPOINT 398
#define VAR_HTTP_MAX_STREAMS 399
#define VAR_HTTP_QUERY_BUFFER_SIZE 400
#define VAR_HTTP_RESPONSE_BUFFER_SIZE 401
#define VAR_HTTP_NODELAY 402
#define VAR_HTTP_NOTLS_DOWNSTREAM 403
#define VAR_STUB_FIRST 404
#define VAR_MINIMAL_RESPONSES 405
#define VAR_RRSET_ROUNDROBIN 406
#define VAR_MAX_UDP_SIZE 407
#define VAR_DELAY_CLOSE 408
#define VAR_UDP_CONNECT 409
#define VAR_UNBLOCK_LAN_ZONES 410
#define VAR_INSECURE_LAN_ZONES 411
#define VAR_INFRA_CACHE_MIN_RTT 412
#define VAR_INFRA_KEEP_PROBING 413
#define VAR_DNS64_PREFIX 414
#define VAR_DNS64_SYNTHALL 415
#define VAR_DNS64_IGNORE_AAAA 416
#define VAR_DNSTAP 417
#define VAR_DNSTAP_ENABLE 418
#define VAR_DNSTAP_SOCKET_PATH 419
#define VAR_DNSTAP_IP 420
#define VAR_DNSTAP_TLS 421
#define VAR_DNSTAP_TLS_SERVER_NAME 422
#define VAR_DNSTAP_TLS_CERT_BUNDLE 423
#define VAR_DNSTAP_TLS_CLIENT_KEY_FILE 424
#define VAR_DNSTAP_TLS_CLIENT_CERT_FILE 425
#define VAR_DNSTAP_SEND_IDENTITY 426
#define VAR_DNSTAP_SEND_VERSION 427
#define VAR_DNSTAP_BIDIRECTIONAL 428
#define VAR_DNSTAP_IDENTITY 429
#define VAR_DNSTAP_VERSION 430
#define VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES 431
#define VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES 432
#define VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES 433
#define VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES 434
#define VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES 435
#define VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES 436
#define VAR_RESPONSE_IP_TAG 437
#define VAR_RESPONSE_IP 438
#define VAR_RESPONSE_IP_DATA 439
#define VAR_HARDEN_ALGO_DOWNGRADE 440
#define VAR_IP_TRANSPARENT 441
#define VAR_IP_DSCP 442
#define VAR_DISABLE_DNSSEC_LAME_CHECK 443
#define VAR_IP_RATELIMIT 444
#define VAR_IP_RATELIMIT_SLABS 445
#define VAR_IP_RATELIMIT_SIZE 446
#define VAR_RATELIMIT 447
#define VAR_RATELIMIT_SLABS 448
#define VAR_RATELIMIT_SIZE 449
#define VAR_RATELIMIT_FOR_DOMAIN 450
#define VAR_RATELIMIT_BELOW_DOMAIN 451
#define VAR_IP_RATELIMIT_FACTOR 452
#define VAR_RATELIMIT_FACTOR 453
#define VAR_SEND_CLIENT_SUBNET 454
#define VAR_CLIENT_SUBNET_ZONE 455
#define VAR_CLIENT_SUBNET_ALWAYS_FORWARD 456
#define VAR_CLIENT_SUBNET_OPCODE 457
#define VAR_MAX_CLIENT_SUBNET_IPV4 458
#define VAR_MAX_CLIENT_SUBNET_IPV6 459
#define VAR_MIN_CLIENT_SUBNET_IPV4 460
#define VAR_MIN_CLIENT_SUBNET_IPV6 461
#define VAR_MAX_ECS_TREE_SIZE_IPV4 462
#define VAR_MAX_ECS_TREE_SIZE_IPV6 463
#define VAR_CAPS_WHITELIST 464
#define VAR_CACHE_MAX_NEGATIVE_TTL 465
#define VAR_PERMIT_SMALL_HOLDDOWN 466
#define VAR_QNAME_MINIMISATION 467
#define VAR_QNAME_MINIMISATION_STRICT 468
#define VAR_IP_FREEBIND 469
#define VAR_DEFINE_TAG 470
#define VAR_LOCAL_ZONE_TAG 471
#define VAR_ACCESS_CONTROL_TAG 472
#define VAR_LOCAL_ZONE_OVERRIDE 473
#define VAR_ACCESS_CONTROL_TAG_ACTION 474
#define VAR_ACCESS_CONTROL_TAG_DATA 475
#define VAR_VIEW 476
#define VAR_ACCESS_CONTROL_VIEW 477
#define VAR_MATCH_CLIENTS 478
#define VAR_MATCH_DESTINATIONS 479
#define VAR_VIEW_FIRST 480
#define VAR_SERVE_EXPIRED 481
#define VAR_SERVE_EXPIRED_TTL 482
#define VAR_SERVE_EXPIRED_TTL_RESET 483
#define VAR_SERVE_EXPIRED_REPLY_TTL 484
#define VAR_SERVE_EXPIRED_CLIENT_TIMEOUT 485
#define VAR_SERVE_ORIGINAL_TTL 486
#define VAR_FAKE_DSA 487
#define VAR_FAKE_SHA1 488
#define VAR_LOG_IDENTITY 489
#define VAR_HIDE_TRUSTANCHOR 490
#define VAR_TRUST_ANCHOR_SIGNALING 491
#define VAR_AGGRESSIVE_NSEC 492
#define VAR_USE_SYSTEMD 493
#define VAR_SHM_ENABLE 494
#define VAR_SHM_KEY 495
#define VAR_ROOT_KEY_SENTINEL 496
#define VAR_DNSCRYPT 497
#define VAR_DNSCRYPT_ENABLE 498
#define VAR_DNSCRYPT_PORT 499
#define VAR_DNSCRYPT_PROVIDER 500
#define VAR_DNSCRYPT_SECRET_KEY 501
#define VAR_DNSCRYPT_PROVIDER_CERT 502
#define VAR_DNSCRYPT_PROVIDER_CERT_ROTATED 503
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE 504
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS 505
#define VAR_DNSCRYPT_NONCE_CACHE_SIZE 506
#define VAR_DNSCRYPT_NONCE_CACHE_SLABS 507
#define VAR_PAD_RESPONSES 508
#define VAR_PAD_RESPONSES_BLOCK_SIZE 509
#define VAR_PAD_QUERIES 510
#define VAR_PAD_QUERIES_BLOCK_SIZE 511
#define VAR_IPSECMOD_ENABLED 512
#define VAR_IPSECMOD_HOOK 513
#define VAR_IPSECMOD_IGNORE_BOGUS 514
#define VAR_IPSECMOD_MAX_TTL 515
#define VAR_IPSECMOD_WHITELIST 516
#define VAR_IPSECMOD_STRICT 517
#define VAR_CACHEDB 518
#define VAR_CACHEDB_BACKEND 519
#define VAR_CACHEDB_SECRETSEED 520
#define VAR_CACHEDB_REDISHOST 521
#define VAR_CACHEDB_REDISPORT 522
#define VAR_CACHEDB_REDISTIMEOUT 523
#define VAR_CACHEDB_REDISEXPIRERECORDS 524
#define VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM 525
#define VAR_FOR_UPSTREAM 526
#define VAR_AUTH_ZONE 527
#define VAR_ZONEFILE 528
#define VAR_MASTER 529
#define VAR_URL 530
#define VAR_FOR_DOWNSTREAM 531
#define VAR_FALLBACK_ENABLED 532
#define VAR_TLS_ADDITIONAL_PORT 533
#define VAR_LOW_RTT 534
#define VAR_LOW_RTT_PERMIL 535
#define VAR_FAST_SERVER_PERMIL 536
#define VAR_FAST_SERVER_NUM 537
#define VAR_ALLOW_NOTIFY 538
#define VAR_TLS_WIN_CERT 539
#define VAR_TCP_CONNECTION_LIMIT 540
#define VAR_FORWARD_NO_CACHE 541
#define VAR_STUB_NO_CACHE 542
#define VAR_LOG_SERVFAIL 543
#define VAR_DENY_ANY 544
#define VAR_UNKNOWN_SERVER_TIME_LIMIT 545
#define VAR_LOG_TAG_QUERYREPLY 546
#define VAR_STREAM_WAIT_SIZE 547
#define VAR_TLS_CIPHERS 548
#define VAR_TLS_CIPHERSUITES 549
#define VAR_TLS_USE_SNI 550
#define VAR_IPSET 551
#define VAR_IPSET_NAME_V4 552
#define VAR_IPSET_NAME_V6 553
#define VAR_TLS_SESSION_TICKET_KEYS 554
#define VAR_RPZ 555
#define VAR_TAGS 556
#define VAR_RPZ_ACTION_OVERRIDE 557
#define VAR_RPZ_CNAME_OVERRIDE 558
#define VAR_RPZ_LOG 559
#define VAR_RPZ_LOG_NAME 560
#define VAR_DYNLIB 561
#define VAR_DYNLIB_FILE 562
#define VAR_EDNS_CLIENT_STRING 563
#define VAR_EDNS_CLIENT_STRING_OPCODE 564
#define VAR_NSID 565

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 66 "./util/configparser.y"

	char*	str;
	int           ival;
	unsigned int  uval;
	long          lval;
	unsigned long ulval;
	size_t        sval;

#line 780 "util/configparser.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_UTIL_CONFIGPARSER_H_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   998

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  311
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  339
/* YYNRULES -- Number of rules.  */
#define YYNRULES  656
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  963

#define YYUNDEFTOK  2
#define YYMAXUTOK   565


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   196,   196,   203,   203,   204,   204,   205,   205,   206,
     206,   206,   207,   207,   208,   208,   209,   209,   210,   212,
     218,   231,   249,   262,   276,   289,   300,   305,   306,   307,
     307,   307,   308,   308,   309,   309,   309,   310,   310,   311,
     311,   311,   312,   312,   313,   313,   313,   314,   314,   314,
     315,   315,   316,   316,   317,   317,   318,   318,   319,   319,
     320,   320,   321,   321,   322,   322,   323,   323,   323,   324,
     324,   324,   325,   325,   325,   326,   326,   327,   327,   328,
     328,   329,   329,   330,   330,   330,   331,   331,   332,   332,
     333,   333,   333,   334,   334,   335,   335,   336,   336,   337,
     337,   337,   338,   338,   339,   339,   340,   340,   341,   341,
     342,   342,   343,   343,   343,   344,   344,   345,   345,   345,
     346,   346,   346,   347,   347,   347,   348,   348,   348,   348,
     349,   350,   350,   350,   351,   351,   351,   352,   352,   353,
     353,   354,   354,   354,   355,   355,   355,   356,   356,   357,
     357,   357,   358,   358,   359,   359,   359,   360,   360,   361,
     361,   362,   362,   363,   364,   364,   365,   365,   366,   366,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     372,   373,   373,   373,   374,   374,   375,   375,   376,   376,
     377,   378,   378,   379,   379,   380,   381,   381,   382,   382,
     383,   383,   384,   384,   385,   385,   385,   386,   386,   386,
     387,   387,   388,   389,   389,   390,   390,   391,   391,   392,
     392,   393,   393,   393,   394,   394,   394,   395,   395,   395,
     396,   396,   397,   397,   398,   398,   399,   399,   400,   400,
     402,   414,   415,   416,   416,   416,   416,   416,   417,   417,
     419,   431,   432,   433,   433,   433,   433,   434,   434,   437,
     436,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   488,   504,   515,
     523,   533,   550,   551,   552,   552,   552,   552,   553,   553,
     553,   554,   557,   576,   593,   601,   608,   616,   633,   634,
     635,   635,   635,   635,   635,   636,   636,   636,   637,   637,
     639,   645,   651,   657,   663,   669,   675,   681,   690,   702,
     716,   726,   737,   754,   771,   788,   805,   820,   835,   848,
     863,   869,   878,   887,   893,   899,   905,   911,   917,   923,
     929,   935,   941,   947,   953,   964,   970,   981,   987,   994,
    1000,  1007,  1014,  1023,  1030,  1036,  1044,  1051,  1058,  1066,
    1072,  1080,  1096,  1101,  1106,  1111,  1116,  1121,  1127,  1133,
    1145,  1151,  1157,  1163,  1169,  1175,  1181,  1188,  1195,  1221,
    1229,  1236,  1243,  1250,  1257,  1265,  1273,  1281,  1288,  1294,
    1300,  1307,  1313,  1319,  1325,  1332,  1339,  1355,  1361,  1367,
    1373,  1379,  1385,  1394,  1400,  1411,  1420,  1426,  1432,  1438,
    1444,  1450,  1456,  1462,  1468,  1474,  1480,  1486,  1494,  1500,
    1508,  1514,  1520,  1526,  1533,  1539,  1545,  1551,  1557,  1564,
    1570,  1576,  1582,  1589,  1596,  1603,  1609,  1615,  1621,  1627,
    1634,  1640,  1660,  1667,  1685,  1691,  1697,  1703,  1709,  1715,
    1721,  1727,  1733,  1739,  1745,  1751,  1757,  1763,  1769,  1775,
    1781,  1791,  1801,  1807,  1814,  1820,  1826,  1832,  1837,  1843,
    1849,  1855,  1900,  1907,  1922,  1928,  1934,  1940,  1946,  1953,
    1959,  1967,  1981,  2002,  2023,  2035,  2047,  2059,  2068,  2089,
    2096,  2102,  2108,  2114,  2120,  2126,  2141,  2156,  2162,  2168,
    2174,  2180,  2186,  2192,  2198,  2204,  2210,  2216,  2222,  2232,
    2242,  2254,  2264,  2276,  2286,  2295,  2304,  2314,  2321,  2328,
    2334,  2340,  2346,  2352,  2362,  2369,  2376,  2382,  2388,  2394,
    2404,  2411,  2418,  2425,  2433,  2439,  2445,  2451,  2456,  2457,
    2458,  2458,  2458,  2459,  2459,  2459,  2460,  2460,  2462,  2468,
    2477,  2484,  2490,  2497,  2504,  2511,  2518,  2523,  2524,  2525,
    2525,  2525,  2526,  2526,  2526,  2527,  2528,  2528,  2529,  2529,
    2530,  2530,  2531,  2532,  2533,  2534,  2535,  2536,  2538,  2544,
    2550,  2557,  2564,  2570,  2577,  2584,  2591,  2598,  2604,  2610,
    2617,  2624,  2631,  2638,  2645,  2652,  2659,  2666,  2671,  2672,
    2673,  2675,  2681,  2686,  2687,  2688,  2690,  2696,  2702,  2709,
    2718,  2726,  2731,  2732,  2734,  2734,  2734,  2735,  2735,  2736,
    2737,  2738,  2739,  2740,  2742,  2749,  2758,  2765,  2774,  2781,
    2790,  2796,  2802,  2808,  2814,  2819,  2820,  2821,  2821,  2822,
    2822,  2822,  2823,  2825,  2837,  2849,  2861,  2875,  2888,  2898,
    2909,  2914,  2915,  2916,  2916,  2918,  2933
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SPACE", "LETTER", "NEWLINE", "COMMENT",
  "COLON", "ANY", "ZONESTR", "STRING_ARG", "VAR_FORCE_TOPLEVEL",
  "VAR_SERVER", "VAR_VERBOSITY", "VAR_NUM_THREADS", "VAR_PORT",
  "VAR_OUTGOING_RANGE", "VAR_INTERFACE", "VAR_PREFER_IP4", "VAR_DO_IP4",
  "VAR_DO_IP6", "VAR_PREFER_IP6", "VAR_DO_UDP", "VAR_DO_TCP",
  "VAR_TCP_MSS", "VAR_OUTGOING_TCP_MSS", "VAR_TCP_IDLE_TIMEOUT",
  "VAR_EDNS_TCP_KEEPALIVE", "VAR_EDNS_TCP_KEEPALIVE_TIMEOUT", "VAR_CHROOT",
  "VAR_USERNAME", "VAR_DIRECTORY", "VAR_LOGFILE", "VAR_PIDFILE",
  "VAR_MSG_CACHE_SIZE", "VAR_MSG_CACHE_SLABS",
  "VAR_NUM_QUERIES_PER_THREAD", "VAR_RRSET_CACHE_SIZE",
  "VAR_RRSET_CACHE_SLABS", "VAR_OUTGOING_NUM_TCP", "VAR_INFRA_HOST_TTL",
  "VAR_INFRA_LAME_TTL", "VAR_INFRA_CACHE_SLABS",
  "VAR_INFRA_CACHE_NUMHOSTS", "VAR_INFRA_CACHE_LAME_SIZE", "VAR_NAME",
  "VAR_STUB_ZONE", "VAR_STUB_HOST", "VAR_STUB_ADDR",
  "VAR_TARGET_FETCH_POLICY", "VAR_HARDEN_SHORT_BUFSIZE",
  "VAR_HARDEN_LARGE_QUERIES", "VAR_FORWARD_ZONE", "VAR_FORWARD_HOST",
  "VAR_FORWARD_ADDR", "VAR_DO_NOT_QUERY_ADDRESS", "VAR_HIDE_IDENTITY",
  "VAR_HIDE_VERSION", "VAR_IDENTITY", "VAR_VERSION", "VAR_HARDEN_GLUE",
  "VAR_MODULE_CONF", "VAR_TRUST_ANCHOR_FILE", "VAR_TRUST_ANCHOR",
  "VAR_VAL_OVERRIDE_DATE", "VAR_BOGUS_TTL", "VAR_VAL_CLEAN_ADDITIONAL",
  "VAR_VAL_PERMISSIVE_MODE", "VAR_INCOMING_NUM_TCP", "VAR_MSG_BUFFER_SIZE",
  "VAR_KEY_CACHE_SIZE", "VAR_KEY_CACHE_SLABS", "VAR_TRUSTED_KEYS_FILE",
  "VAR_VAL_NSEC3_KEYSIZE_ITERATIONS", "VAR_USE_SYSLOG",
  "VAR_OUTGOING_INTERFACE", "VAR_ROOT_HINTS", "VAR_DO_NOT_QUERY_LOCALHOST",
  "VAR_CACHE_MAX_TTL", "VAR_HARDEN_DNSSEC_STRIPPED", "VAR_ACCESS_CONTROL",
  "VAR_LOCAL_ZONE", "VAR_LOCAL_DATA", "VAR_INTERFACE_AUTOMATIC",
  "VAR_STATISTICS_INTERVAL", "VAR_DO_DAEMONIZE", "VAR_USE_CAPS_FOR_ID",
  "VAR_STATISTICS_CUMULATIVE", "VAR_OUTGOING_PORT_PERMIT",
  "VAR_OUTGOING_PORT_AVOID", "VAR_DLV_ANCHOR_FILE", "VAR_DLV_ANCHOR",
  "VAR_NEG_CACHE_SIZE", "VAR_HARDEN_REFERRAL_PATH", "VAR_PRIVATE_ADDRESS",
  "VAR_PRIVATE_DOMAIN", "VAR_REMOTE_CONTROL", "VAR_CONTROL_ENABLE",
  "VAR_CONTROL_INTERFACE", "VAR_CONTROL_PORT", "VAR_SERVER_KEY_FILE",
  "VAR_SERVER_CERT_FILE", "VAR_CONTROL_KEY_FILE", "VAR_CONTROL_CERT_FILE",
  "VAR_CONTROL_USE_CERT", "VAR_EXTENDED_STATISTICS", "VAR_LOCAL_DATA_PTR",
  "VAR_JOSTLE_TIMEOUT", "VAR_STUB_PRIME", "VAR_UNWANTED_REPLY_THRESHOLD",
  "VAR_LOG_TIME_ASCII", "VAR_DOMAIN_INSECURE", "VAR_PYTHON",
  "VAR_PYTHON_SCRIPT", "VAR_VAL_SIG_SKEW_MIN", "VAR_VAL_SIG_SKEW_MAX",
  "VAR_CACHE_MIN_TTL", "VAR_VAL_LOG_LEVEL", "VAR_AUTO_TRUST_ANCHOR_FILE",
  "VAR_KEEP_MISSING", "VAR_ADD_HOLDDOWN", "VAR_DEL_HOLDDOWN",
  "VAR_SO_RCVBUF", "VAR_EDNS_BUFFER_SIZE", "VAR_PREFETCH",
  "VAR_PREFETCH_KEY", "VAR_SO_SNDBUF", "VAR_SO_REUSEPORT",
  "VAR_HARDEN_BELOW_NXDOMAIN", "VAR_IGNORE_CD_FLAG", "VAR_LOG_QUERIES",
  "VAR_LOG_REPLIES", "VAR_LOG_LOCAL_ACTIONS", "VAR_TCP_UPSTREAM",
  "VAR_SSL_UPSTREAM", "VAR_SSL_SERVICE_KEY", "VAR_SSL_SERVICE_PEM",
  "VAR_SSL_PORT", "VAR_FORWARD_FIRST", "VAR_STUB_SSL_UPSTREAM",
  "VAR_FORWARD_SSL_UPSTREAM", "VAR_TLS_CERT_BUNDLE", "VAR_HTTPS_PORT",
  "VAR_HTTP_ENDPOINT", "VAR_HTTP_MAX_STREAMS",
  "VAR_HTTP_QUERY_BUFFER_SIZE", "VAR_HTTP_RESPONSE_BUFFER_SIZE",
  "VAR_HTTP_NODELAY", "VAR_HTTP_NOTLS_DOWNSTREAM", "VAR_STUB_FIRST",
  "VAR_MINIMAL_RESPONSES", "VAR_RRSET_ROUNDROBIN", "VAR_MAX_UDP_SIZE",
  "VAR_DELAY_CLOSE", "VAR_UDP_CONNECT", "VAR_UNBLOCK_LAN_ZONES",
  "VAR_INSECURE_LAN_ZONES", "VAR_INFRA_CACHE_MIN_RTT",
  "VAR_INFRA_KEEP_PROBING", "VAR_DNS64_PREFIX", "VAR_DNS64_SYNTHALL",
  "VAR_DNS64_IGNORE_AAAA", "VAR_DNSTAP", "VAR_DNSTAP_ENABLE",
  "VAR_DNSTAP_SOCKET_PATH", "VAR_DNSTAP_IP", "VAR_DNSTAP_TLS",
  "VAR_DNSTAP_TLS_SERVER_NAME", "VAR_DNSTAP_TLS_CERT_BUNDLE",
  "VAR_DNSTAP_TLS_CLIENT_KEY_FILE", "VAR_DNSTAP_TLS_CLIENT_CERT_FILE",
  "VAR_DNSTAP_SEND_IDENTITY", "VAR_DNSTAP_SEND_VERSION",
  "VAR_DNSTAP_BIDIRECTIONAL", "VAR_DNSTAP_IDENTITY", "VAR_DNSTAP_VERSION",
  "VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES",
  "VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES",
  "VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES", "VAR_RESPONSE_IP_TAG",
  "VAR_RESPONSE_IP", "VAR_RESPONSE_IP_DATA", "VAR_HARDEN_ALGO_DOWNGRADE",
  "VAR_IP_TRANSPARENT", "VAR_IP_DSCP", "VAR_DISABLE_DNSSEC_LAME_CHECK",
  "VAR_IP_RATELIMIT", "VAR_IP_RATELIMIT_SLABS", "VAR_IP_RATELIMIT_SIZE",
  "VAR_RATELIMIT", "VAR_RATELIMIT_SLABS", "VAR_RATELIMIT_SIZE",
  "VAR_RATELIMIT_FOR_DOMAIN", "VAR_RATELIMIT_BELOW_DOMAIN",
  "VAR_IP_RATELIMIT_FACTOR", "VAR_RATELIMIT_FACTOR",
  "VAR_SEND_CLIENT_SUBNET", "VAR_CLIENT_SUBNET_ZONE",
  "VAR_CLIENT_SUBNET_ALWAYS_FORWARD", "VAR_CLIENT_SUBNET_OPCODE",
  "VAR_MAX_CLIENT_SUBNET_IPV4", "VAR_MAX_CLIENT_SUBNET_IPV6",
  "VAR_MIN_CLIENT_SUBNET_IPV4", "VAR_MIN_CLIENT_SUBNET_IPV6",
  "VAR_MAX_ECS_TREE_SIZE_IPV4", "VAR_MAX_ECS_TREE_SIZE_IPV6",
  "VAR_CAPS_WHITELIST", "VAR_CACHE_MAX_NEGATIVE_TTL",
  "VAR_PERMIT_SMALL_HOLDDOWN", "VAR_QNAME_MINIMISATION",
  "VAR_QNAME_MINIMISATION_STRICT", "VAR_IP_FREEBIND", "VAR_DEFINE_TAG",
  "VAR_LOCAL_ZONE_TAG", "VAR_ACCESS_CONTROL_TAG",
  "VAR_LOCAL_ZONE_OVERRIDE", "VAR_ACCESS_CONTROL_TAG_ACTION",
  "VAR_ACCESS_CONTROL_TAG_DATA", "VAR_VIEW", "VAR_ACCESS_CONTROL_VIEW",
  "VAR_MATCH_CLIENTS", "VAR_MATCH_DESTINATIONS", "VAR_VIEW_FIRST",
  "VAR_SERVE_EXPIRED", "VAR_SERVE_EXPIRED_TTL",
  "VAR_SERVE_EXPIRED_TTL_RESET", "VAR_SERVE_EXPIRED_REPLY_TTL",
  "VAR_SERVE_EXPIRED_CLIENT_TIMEOUT", "VAR_SERVE_ORIGINAL_TTL",
  "VAR_FAKE_DSA", "VAR_FAKE_SHA1", "VAR_LOG_IDENTITY",
  "VAR_HIDE_TRUSTANCHOR", "VAR_TRUST_ANCHOR_SIGNALING",
  "VAR_AGGRESSIVE_NSEC", "VAR_USE_SYSTEMD", "VAR_SHM_ENABLE",
  "VAR_SHM_KEY", "VAR_ROOT_KEY_SENTINEL", "VAR_DNSCRYPT",
  "VAR_DNSCRYPT_ENABLE", "VAR_DNSCRYPT_PORT", "VAR_DNSCRYPT_PROVIDER",
  "VAR_DNSCRYPT_SECRET_KEY", "VAR_DNSCRYPT_PROVIDER_CERT",
  "VAR_DNSCRYPT_PROVIDER_CERT_ROTATED",
  "VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE",
  "VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS",
  "VAR_DNSCRYPT_NONCE_CACHE_SIZE", "VAR_DNSCRYPT_NONCE_CACHE_SLABS",
  "VAR_PAD_RESPONSES", "VAR_PAD_RESPONSES_BLOCK_SIZE", "VAR_PAD_QUERIES",
  "VAR_PAD_QUERIES_BLOCK_SIZE", "VAR_IPSECMOD_ENABLED",
  "VAR_IPSECMOD_HOOK", "VAR_IPSECMOD_IGNORE_BOGUS", "VAR_IPSECMOD_MAX_TTL",
  "VAR_IPSECMOD_WHITELIST", "VAR_IPSECMOD_STRICT", "VAR_CACHEDB",
  "VAR_CACHEDB_BACKEND", "VAR_CACHEDB_SECRETSEED", "VAR_CACHEDB_REDISHOST",
  "VAR_CACHEDB_REDISPORT", "VAR_CACHEDB_REDISTIMEOUT",
  "VAR_CACHEDB_REDISEXPIRERECORDS", "VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM",
  "VAR_FOR_UPSTREAM", "VAR_AUTH_ZONE", "VAR_ZONEFILE", "VAR_MASTER",
  "VAR_URL", "VAR_FOR_DOWNSTREAM", "VAR_FALLBACK_ENABLED",
  "VAR_TLS_ADDITIONAL_PORT", "VAR_LOW_RTT", "VAR_LOW_RTT_PERMIL",
  "VAR_FAST_SERVER_PERMIL", "VAR_FAST_SERVER_NUM", "VAR_ALLOW_NOTIFY",
  "VAR_TLS_WIN_CERT", "VAR_TCP_CONNECTION_LIMIT", "VAR_FORWARD_NO_CACHE",
  "VAR_STUB_NO_CACHE", "VAR_LOG_SERVFAIL", "VAR_DENY_ANY",
  "VAR_UNKNOWN_SERVER_TIME_LIMIT", "VAR_LOG_TAG_QUERYREPLY",
  "VAR_STREAM_WAIT_SIZE", "VAR_TLS_CIPHERS", "VAR_TLS_CIPHERSUITES",
  "VAR_TLS_USE_SNI", "VAR_IPSET", "VAR_IPSET_NAME_V4", "VAR_IPSET_NAME_V6",
  "VAR_TLS_SESSION_TICKET_KEYS", "VAR_RPZ", "VAR_TAGS",
  "VAR_RPZ_ACTION_OVERRIDE", "VAR_RPZ_CNAME_OVERRIDE", "VAR_RPZ_LOG",
  "VAR_RPZ_LOG_NAME", "VAR_DYNLIB", "VAR_DYNLIB_FILE",
  "VAR_EDNS_CLIENT_STRING", "VAR_EDNS_CLIENT_STRING_OPCODE", "VAR_NSID",
  "$accept", "all", "toplevelvars", "toplevelvar", "force_toplevel",
  "yes_or_no", "empty_or_nnn", "nonzero", "pow2", "nnn", "memsize",
  "serverstart", "contents_server", "content_server", "stubstart",
  "contents_stub", "content_stub", "forwardstart", "contents_forward",
  "content_forward", "view_block", "$@1", "contents_view", "content_view",
  "view_name", "view_first", "view_match_clients",
  "view_match_destinations", "authstart", "contents_auth", "content_auth",
  "rpz_tag", "rpz_action_override", "rpz_cname_override", "rpz_log",
  "rpz_log_name", "rpzstart", "contents_rpz", "content_rpz",
  "server_num_threads", "server_verbosity", "server_statistics_interval",
  "server_statistics_cumulative", "server_extended_statistics",
  "server_shm_enable", "server_shm_key", "server_port",
  "server_send_client_subnet", "server_client_subnet_zone",
  "server_client_subnet_always_forward", "server_client_subnet_opcode",
  "server_max_client_subnet_ipv4", "server_max_client_subnet_ipv6",
  "server_min_client_subnet_ipv4", "server_min_client_subnet_ipv6",
  "server_max_ecs_tree_size_ipv4", "server_max_ecs_tree_size_ipv6",
  "server_interface", "server_outgoing_interface", "server_outgoing_range",
  "server_outgoing_port_permit", "server_outgoing_port_avoid",
  "server_outgoing_num_tcp", "server_incoming_num_tcp",
  "server_interface_automatic", "server_do_ip4", "server_do_ip6",
  "server_do_udp", "server_do_tcp", "server_prefer_ip4",
  "server_prefer_ip6", "server_tcp_mss", "server_outgoing_tcp_mss",
  "server_tcp_idle_timeout", "server_tcp_keepalive",
  "server_tcp_keepalive_timeout", "server_tcp_upstream",
  "server_udp_upstream_without_downstream", "server_ssl_upstream",
  "server_ssl_service_key", "server_ssl_service_pem", "server_ssl_port",
  "server_tls_cert_bundle", "server_tls_win_cert",
  "server_tls_additional_port", "server_tls_ciphers",
  "server_tls_ciphersuites", "server_tls_session_ticket_keys",
  "server_tls_use_sni", "server_https_port", "server_http_endpoint",
  "server_http_max_streams", "server_http_query_buffer_size",
  "server_http_response_buffer_size", "server_http_nodelay",
  "server_http_notls_downstream", "server_use_systemd",
  "server_do_daemonize", "server_use_syslog", "server_log_time_ascii",
  "server_log_queries", "server_log_replies", "server_log_tag_queryreply",
  "server_log_servfail", "server_log_local_actions", "server_chroot",
  "server_username", "server_directory", "server_logfile",
  "server_pidfile", "server_root_hints", "server_dlv_anchor_file",
  "server_dlv_anchor", "server_auto_trust_anchor_file",
  "server_trust_anchor_file", "server_trusted_keys_file",
  "server_trust_anchor", "server_trust_anchor_signaling",
  "server_root_key_sentinel", "server_domain_insecure",
  "server_hide_identity", "server_hide_version", "server_hide_trustanchor",
  "server_identity", "server_version", "server_nsid", "server_so_rcvbuf",
  "server_so_sndbuf", "server_so_reuseport", "server_ip_transparent",
  "server_ip_freebind", "server_ip_dscp", "server_stream_wait_size",
  "server_edns_buffer_size", "server_msg_buffer_size",
  "server_msg_cache_size", "server_msg_cache_slabs",
  "server_num_queries_per_thread", "server_jostle_timeout",
  "server_delay_close", "server_udp_connect", "server_unblock_lan_zones",
  "server_insecure_lan_zones", "server_rrset_cache_size",
  "server_rrset_cache_slabs", "server_infra_host_ttl",
  "server_infra_lame_ttl", "server_infra_cache_numhosts",
  "server_infra_cache_lame_size", "server_infra_cache_slabs",
  "server_infra_cache_min_rtt", "server_infra_keep_probing",
  "server_target_fetch_policy", "server_harden_short_bufsize",
  "server_harden_large_queries", "server_harden_glue",
  "server_harden_dnssec_stripped", "server_harden_below_nxdomain",
  "server_harden_referral_path", "server_harden_algo_downgrade",
  "server_use_caps_for_id", "server_caps_whitelist",
  "server_private_address", "server_private_domain", "server_prefetch",
  "server_prefetch_key", "server_deny_any",
  "server_unwanted_reply_threshold", "server_do_not_query_address",
  "server_do_not_query_localhost", "server_access_control",
  "server_module_conf", "server_val_override_date",
  "server_val_sig_skew_min", "server_val_sig_skew_max",
  "server_cache_max_ttl", "server_cache_max_negative_ttl",
  "server_cache_min_ttl", "server_bogus_ttl",
  "server_val_clean_additional", "server_val_permissive_mode",
  "server_aggressive_nsec", "server_ignore_cd_flag",
  "server_serve_expired", "server_serve_expired_ttl",
  "server_serve_expired_ttl_reset", "server_serve_expired_reply_ttl",
  "server_serve_expired_client_timeout", "server_serve_original_ttl",
  "server_fake_dsa", "server_fake_sha1", "server_val_log_level",
  "server_val_nsec3_keysize_iterations", "server_add_holddown",
  "server_del_holddown", "server_keep_missing",
  "server_permit_small_holddown", "server_key_cache_size",
  "server_key_cache_slabs", "server_neg_cache_size", "server_local_zone",
  "server_local_data", "server_local_data_ptr", "server_minimal_responses",
  "server_rrset_roundrobin", "server_unknown_server_time_limit",
  "server_max_udp_size", "server_dns64_prefix", "server_dns64_synthall",
  "server_dns64_ignore_aaaa", "server_define_tag", "server_local_zone_tag",
  "server_access_control_tag", "server_access_control_tag_action",
  "server_access_control_tag_data", "server_local_zone_override",
  "server_access_control_view", "server_response_ip_tag",
  "server_ip_ratelimit", "server_ratelimit", "server_ip_ratelimit_size",
  "server_ratelimit_size", "server_ip_ratelimit_slabs",
  "server_ratelimit_slabs", "server_ratelimit_for_domain",
  "server_ratelimit_below_domain", "server_ip_ratelimit_factor",
  "server_ratelimit_factor", "server_low_rtt", "server_fast_server_num",
  "server_fast_server_permil", "server_qname_minimisation",
  "server_qname_minimisation_strict", "server_pad_responses",
  "server_pad_responses_block_size", "server_pad_queries",
  "server_pad_queries_block_size", "server_ipsecmod_enabled",
  "server_ipsecmod_ignore_bogus", "server_ipsecmod_hook",
  "server_ipsecmod_max_ttl", "server_ipsecmod_whitelist",
  "server_ipsecmod_strict", "server_edns_client_string",
  "server_edns_client_string_opcode", "stub_name", "stub_host",
  "stub_addr", "stub_first", "stub_no_cache", "stub_ssl_upstream",
  "stub_prime", "forward_name", "forward_host", "forward_addr",
  "forward_first", "forward_no_cache", "forward_ssl_upstream", "auth_name",
  "auth_zonefile", "auth_master", "auth_url", "auth_allow_notify",
  "auth_for_downstream", "auth_for_upstream", "auth_fallback_enabled",
  "rcstart", "contents_rc", "content_rc", "rc_control_enable",
  "rc_control_port", "rc_control_interface", "rc_control_use_cert",
  "rc_server_key_file", "rc_server_cert_file", "rc_control_key_file",
  "rc_control_cert_file", "dtstart", "contents_dt", "content_dt",
  "dt_dnstap_enable", "dt_dnstap_bidirectional", "dt_dnstap_socket_path",
  "dt_dnstap_ip", "dt_dnstap_tls", "dt_dnstap_tls_server_name",
  "dt_dnstap_tls_cert_bundle", "dt_dnstap_tls_client_key_file",
  "dt_dnstap_tls_client_cert_file", "dt_dnstap_send_identity",
  "dt_dnstap_send_version", "dt_dnstap_identity", "dt_dnstap_version",
  "dt_dnstap_log_resolver_query_messages",
  "dt_dnstap_log_resolver_response_messages",
  "dt_dnstap_log_client_query_messages",
  "dt_dnstap_log_client_response_messages",
  "dt_dnstap_log_forwarder_query_messages",
  "dt_dnstap_log_forwarder_response_messages", "pythonstart",
  "contents_py", "content_py", "py_script", "dynlibstart", "contents_dl",
  "content_dl", "dl_file", "server_disable_dnssec_lame_check",
  "server_log_identity", "server_response_ip", "server_response_ip_data",
  "dnscstart", "contents_dnsc", "content_dnsc", "dnsc_dnscrypt_enable",
  "dnsc_dnscrypt_port", "dnsc_dnscrypt_provider",
  "dnsc_dnscrypt_provider_cert", "dnsc_dnscrypt_provider_cert_rotated",
  "dnsc_dnscrypt_secret_key", "dnsc_dnscrypt_shared_secret_cache_size",
  "dnsc_dnscrypt_shared_secret_cache_slabs",
  "dnsc_dnscrypt_nonce_cache_size", "dnsc_dnscrypt_nonce_cache_slabs",
  "cachedbstart", "contents_cachedb", "content_cachedb",
  "cachedb_backend_name", "cachedb_secret_seed", "redis_server_host",
  "redis_server_port", "redis_timeout", "redis_expire_records",
  "server_tcp_connection_limit", "ipsetstart", "contents_ipset",
  "content_ipset", "ipset_name_v4", "ipset_name_v6", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565
};
# endif

#define YYPACT_NINF (-294)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -294,     7,   152,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,   688,    -3,    57,   -34,   -16,   -79,
     181,  -101,  -293,  -168,   -38,  -288,   187,     5,     5,     5,
       5,     6,    16,    16,    16,    16,    16,    16,     5,     5,
       5,    16,     5,    22,    25,    26,    27,    28,    33,    36,
      37,    33,    36,     5,     5,    39,    36,    37,    40,    42,
      16,    16,    44,    16,    16,    45,    46,    16,    48,    52,
      53,    54,     5,    16,    16,     5,    37,    33,    36,    55,
      56,    16,    58,    59,    16,     5,    16,    61,    63,    75,
      16,    78,    16,    16,    16,    87,    88,    89,    91,    33,
      16,    94,   105,    16,   107,     5,     5,    16,   110,    78,
      78,     5,     5,   111,     5,     5,     5,    33,    37,    16,
      16,    33,    16,    16,    16,    16,    16,    16,    16,    16,
     117,   118,     5,   119,     5,   124,     5,    33,    33,    16,
      16,    16,    16,     5,     5,    16,    16,    16,     5,    16,
     125,    16,   127,   128,   133,   135,    16,    16,     5,    16,
       5,    36,    33,     5,    36,    33,   137,   138,     5,     5,
     143,   144,    16,   147,   148,   158,   169,   173,   174,   178,
     179,     5,    16,    16,    16,    16,   180,   182,   191,   192,
     195,   196,   199,    16,     5,    16,     5,     5,    16,    16,
      16,   200,    16,    16,    16,    16,    16,    78,    16,    16,
      37,    16,    37,    16,   201,    16,     5,   202,    16,    16,
     205,   213,     5,    37,    16,   223,    16,    16,     5,    16,
      33,   225,   241,    16,   242,   244,     5,   246,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
     251,   262,   263,    16,    16,    16,    16,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   264,   265,   266,    16,    16,
      16,  -294,  -294,  -294,  -294,  -294,  -294,  -294,   267,    16,
     268,   269,   270,    16,    16,   271,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   272,   280,   281,    16,   282,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,    16,   284,     5,   285,   287,   290,   291,    16,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,    16,   292,
     293,    16,   294,   295,   296,   297,    16,    16,    16,   298,
     299,    16,    16,    16,    16,    16,    16,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,   300,  -294,  -294,
     301,  -294,  -294,    16,     5,   303,   305,   306,   307,    33,
      36,    33,    36,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,   309,   310,   311,     5,     5,    16,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,   312,   313,  -294,
    -294,  -294,   315,   316,   317,    16,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,   318,   319,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,   321,   322,   323,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,   324,   325,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,   326,   327,   329,
     330,   331,   332,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   353,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   354,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,   355,   356,   357,  -294,  -294,  -294,
    -294,  -294,  -294
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       3,     0,     2,     1,    19,    26,   240,   250,   537,   597,
     556,   259,   611,   634,   281,   650,   297,   602,     4,    18,
      28,   242,   252,    11,   283,   299,   539,   558,   599,   604,
     613,   636,   652,   262,     5,     6,     7,    15,    16,     9,
      10,     8,    17,    12,    13,    14,   260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    29,
      30,    93,    96,   105,   207,   208,    31,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,    44,    84,    32,
      97,    98,    55,    77,    92,    33,    34,    37,    38,    35,
      36,    39,    40,    41,    42,    43,   128,   219,   129,   131,
     132,   133,   221,   226,   222,   233,   234,   235,   236,   134,
     135,   136,   137,   138,   139,   140,   203,    94,    83,   109,
     126,   127,   231,   228,   130,    45,    46,    47,    48,    49,
      85,    99,   100,   115,    71,    81,    72,   211,   212,   110,
      65,    66,   210,    67,    68,   239,   119,   123,   144,   154,
     181,   157,   232,   120,    78,    50,    51,    52,   107,   145,
     146,   147,   148,    53,    54,    56,    57,    59,    60,    58,
     152,   158,    61,    62,    63,    69,    88,   124,   102,   153,
      95,   177,   103,   104,   121,   122,   229,   108,    64,    86,
      89,    70,    73,   111,   112,    87,   178,   113,    74,    75,
      76,   220,   125,   195,   196,   197,   198,   199,   200,   201,
     209,   114,    82,   116,   117,   118,   179,    79,    80,   101,
      90,    91,   106,   141,   142,   230,   143,   149,   150,   151,
     182,   183,   185,   187,   188,   186,   189,   204,   155,   156,
     161,   162,   159,   160,   163,   164,   166,   165,   223,   225,
     224,   180,   190,   191,   192,   193,   194,   213,   215,   214,
     216,   217,   218,   237,   238,   184,   202,   205,   206,   227,
       0,     0,     0,     0,     0,     0,     0,   241,   243,   244,
     245,   247,   248,   249,   246,     0,     0,     0,     0,     0,
       0,   251,   253,   254,   255,   256,   257,   258,     0,     0,
       0,     0,     0,     0,     0,     0,   282,   284,   285,   286,
     287,   291,   288,   289,   290,     0,     0,     0,     0,     0,
     302,   306,   307,   308,   309,   298,   300,   301,   303,   304,
     305,     0,     0,     0,     0,     0,     0,     0,     0,   538,
     540,   542,   541,   547,   543,   544,   545,   546,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   557,   559,   561,
     560,   562,   563,   564,   565,   566,   567,   568,   569,   570,
     571,   572,   573,   574,   575,   576,   577,     0,   598,   600,
       0,   603,   605,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   612,   614,   615,   616,   618,   619,   617,
     620,   621,   622,   623,     0,     0,     0,     0,     0,     0,
     635,   637,   638,   639,   640,   641,   642,     0,     0,   651,
     653,   654,     0,     0,     0,     0,   261,   263,   264,   265,
     266,   276,   274,   275,   272,   273,   267,   268,   271,   269,
     270,    24,   311,   310,   317,   330,   328,    20,   340,   336,
     337,   341,   338,   339,   342,   343,   344,   345,   346,   376,
     377,   378,   379,   380,    25,   406,    23,   407,    22,   408,
     414,   415,   333,   416,   417,   420,   418,   419,   423,   424,
     425,   439,   391,   392,   394,   395,   426,   442,   385,   387,
     443,   449,   450,   451,   334,   405,   468,   469,   386,   463,
     369,   329,   381,   440,   446,   427,     0,     0,   472,   335,
      21,   312,   368,   431,   313,   331,   332,   382,   383,   470,
     429,   433,   434,   314,   473,   409,   438,   370,   390,   444,
     445,   448,   462,   384,   466,   464,   465,   397,   404,   435,
     436,   398,   399,   428,   453,   371,   372,   375,   347,   349,
     350,   351,   352,   353,   360,   361,   362,   363,   364,   365,
     366,   474,   475,   477,   410,   411,   412,   413,   421,   422,
     478,   479,   480,     0,     0,     0,   430,   400,   402,   607,
     489,   493,   491,   490,   494,   492,     0,     0,   497,   498,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     432,   447,   467,   502,   503,   401,   481,     0,     0,     0,
       0,     0,     0,   454,   455,   456,   457,   458,   459,   460,
     461,   608,   393,   388,   452,   367,   315,   316,   389,   504,
     505,   506,   507,   508,   510,   509,   511,   512,   513,   348,
     355,   499,   501,   500,   354,     0,   374,   437,   476,   373,
     403,   356,   357,   359,   358,     0,   515,   396,   516,   517,
     518,   522,   521,   519,   520,   523,   524,   525,   526,   528,
     527,   529,   535,   530,   531,   532,   534,   536,   533,   292,
     293,   294,   295,   296,   548,   550,   549,   552,   553,   554,
     555,   551,   578,   580,   581,   582,   583,   584,   585,   586,
     587,   588,   579,   589,   590,   591,   592,   593,   594,   595,
     596,   601,   606,   624,   625,   626,   629,   627,   628,   630,
     631,   632,   633,   643,   644,   645,   646,   647,   648,   655,
     656,   277,   279,   280,   278,   441,   471,   488,   609,   610,
     495,   496,   482,   483,     0,     0,     0,   487,   649,   514,
     486,   484,   485
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -294,  -294,  -294,  -294,  -294,   -53,  -124,    23,   -59,    82,
      62,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
     -13,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   328,   333,  -294,  -294,  -294,
    -294,  -294,  -294,   334,   335,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
     336,   337,   338,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,    34,   339,
     340,   347,   348,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,   341,
     342,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294,  -294
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    18,    19,   658,   721,   679,   677,   652,
     675,    20,    34,   258,    21,    35,   477,    22,    36,   491,
      23,    33,    46,   636,   637,   638,   639,   640,    24,    37,
     506,   520,   521,   522,   523,   524,    25,    38,   525,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   478,   479,   480,   481,   482,
     483,   484,   492,   493,   494,   495,   496,   497,   507,   508,
     509,   510,   511,   512,   513,   514,    26,    39,   539,   540,
     541,   542,   543,   544,   545,   546,   547,    27,    40,   567,
     568,   569,   570,   571,   572,   573,   574,   575,   576,   577,
     578,   579,   580,   581,   582,   583,   584,   585,   586,    28,
      41,   588,   589,    29,    42,   591,   592,   465,   466,   467,
     468,    30,    43,   603,   604,   605,   606,   607,   608,   609,
     610,   611,   612,   613,    31,    44,   620,   621,   622,   623,
     624,   625,   626,   469,    32,    45,   629,   630,   631
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     659,   660,   661,   662,   663,   739,   740,     3,   667,   627,
     628,   498,   587,   681,   590,   651,   656,   685,   531,   532,
     533,   534,   535,   536,   537,   538,   657,   689,   690,   498,
     692,   693,   669,   641,   696,   670,   671,   672,   673,   707,
     702,   703,   470,   674,   471,   472,   676,   678,   710,   684,
     687,   713,   688,   715,   691,   694,   695,   719,   697,   722,
     723,   724,   698,   699,   700,   708,   709,   730,   711,   712,
     733,   716,   526,   717,   737,   593,   594,   595,   596,   597,
     598,   599,   600,   601,   602,   718,   749,   750,   720,   752,
     753,   754,   755,   756,   757,   758,   759,   725,   726,   727,
     686,   728,   485,   837,   731,   473,   769,   770,   771,   772,
     486,   487,   775,   776,   777,   732,   779,   734,   781,   705,
     738,   743,   791,   786,   787,   794,   789,   760,   761,   763,
     653,   654,   655,   680,   765,   780,   474,   782,   783,   802,
     664,   665,   666,   784,   668,   785,   475,   796,   797,   812,
     813,   814,   815,   800,   801,   682,   683,   803,   804,   706,
     823,   748,   825,     4,     5,   828,   829,   830,   805,   832,
     833,   834,   835,   836,   701,   838,   839,   704,   841,   806,
     843,   729,   845,   807,   808,   848,   849,   714,   809,   810,
     816,   854,   817,   856,   857,   488,   859,   489,     6,   747,
     863,   818,   819,   751,     7,   820,   821,   735,   736,   822,
     831,   844,   847,   741,   742,   850,   744,   745,   746,   767,
     768,    68,    69,   851,    71,    72,   614,   615,   616,   617,
     618,   619,   632,   855,   762,   861,   764,   499,   766,   500,
     501,   502,   503,   504,   792,   773,   774,   795,     8,   505,
     778,   862,   864,   840,   865,   842,   867,   500,   501,   502,
     788,   868,   790,   103,     9,   793,   853,   505,   108,   109,
     798,   799,   869,   870,   875,   876,   877,   881,   883,   884,
     885,   888,   889,   811,   476,   515,   516,   517,   518,   519,
     890,   891,   893,   124,   895,   897,   824,   898,   826,   827,
     899,   900,   903,   904,   906,   907,   908,   909,   913,   914,
     921,   922,   860,   925,    10,   926,   927,   928,   846,   933,
     934,   935,   939,   940,   852,   941,   942,   943,   945,   946,
     858,   947,   948,   949,   950,   951,   952,   953,   866,   954,
     955,   956,   957,   490,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   958,   959,   960,   961,   962,     0,     0,
     174,   175,     0,    11,   642,     0,     0,   527,   528,   643,
     644,   645,   646,   647,   648,   529,   530,   649,   650,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     633,   634,   635,     0,     0,    13,     0,     0,     0,     0,
     871,   872,   873,   874,    14,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   878,   879,   880,     0,     0,
       0,     0,     0,     0,     0,     0,   882,     0,    15,     0,
     886,   887,    16,     0,     0,     0,     0,     0,    17,     0,
       0,     0,     0,     0,     0,   892,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   894,     0,
       0,     0,     0,     0,     0,   901,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   902,     0,     0,   905,     0,
       0,     0,     0,   910,   911,   912,     0,     0,   915,   916,
     917,   918,   919,   920,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     923,   930,     0,   932,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   938,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   944,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   896,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   929,     0,   931,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   924,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   936,
     937,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,     0,     0,     0,     0,    79,    80,    81,
       0,     0,     0,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   123,   124,   125,     0,   126,   127,   128,
       0,     0,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,     0,   153,
     154,   155,   156,   157,   158,   159,   160,     0,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,     0,
     212,     0,     0,     0,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,     0,     0,     0,     0,     0,     0,     0,   239,     0,
       0,     0,     0,     0,     0,     0,   240,   241,     0,   242,
     243,     0,   244,   245,     0,     0,   246,   247,   248,   249,
     250,   251,   252,   253,     0,     0,     0,   254,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   256,   257
};

static const yytype_int16 yycheck[] =
{
      53,    54,    55,    56,    57,   129,   130,     0,    61,   297,
     298,    45,   113,    72,   307,    10,    10,    76,    97,    98,
      99,   100,   101,   102,   103,   104,    10,    80,    81,    45,
      83,    84,    10,    46,    87,    10,    10,    10,    10,    98,
      93,    94,    45,    10,    47,    48,    10,    10,   101,    10,
      10,   104,    10,   106,    10,    10,    10,   110,    10,   112,
     113,   114,    10,    10,    10,    10,    10,   120,    10,    10,
     123,    10,    38,    10,   127,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,    10,   139,   140,    10,   142,
     143,   144,   145,   146,   147,   148,   149,    10,    10,    10,
      77,    10,    45,   227,    10,   108,   159,   160,   161,   162,
      53,    54,   165,   166,   167,    10,   169,    10,   171,    96,
      10,    10,   181,   176,   177,   184,   179,    10,    10,    10,
      48,    49,    50,    71,    10,    10,   139,    10,    10,   192,
      58,    59,    60,    10,    62,    10,   149,    10,    10,   202,
     203,   204,   205,    10,    10,    73,    74,    10,    10,    97,
     213,   138,   215,    11,    12,   218,   219,   220,    10,   222,
     223,   224,   225,   226,    92,   228,   229,    95,   231,    10,
     233,   119,   235,    10,    10,   238,   239,   105,    10,    10,
      10,   244,    10,   246,   247,   138,   249,   140,    46,   137,
     253,    10,    10,   141,    52,    10,    10,   125,   126,    10,
      10,    10,    10,   131,   132,    10,   134,   135,   136,   157,
     158,    34,    35,    10,    37,    38,   264,   265,   266,   267,
     268,   269,    45,    10,   152,    10,   154,   271,   156,   273,
     274,   275,   276,   277,   182,   163,   164,   185,    96,   283,
     168,    10,    10,   230,    10,   232,    10,   273,   274,   275,
     178,    10,   180,    76,   112,   183,   243,   283,    81,    82,
     188,   189,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,   201,   287,   301,   302,   303,   304,   305,
      10,    10,    10,   106,    10,    10,   214,    10,   216,   217,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,   250,    10,   162,    10,    10,    10,   236,    10,
      10,    10,    10,    10,   242,    10,    10,    10,    10,    10,
     248,    10,    10,    10,    10,    10,    10,    10,   256,    10,
      10,    10,    10,   286,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,    10,    10,    10,    10,    10,    -1,    -1,
     183,   184,    -1,   221,    46,    -1,    -1,    38,    38,    46,
      46,    46,    46,    46,    46,    38,    38,    46,    46,    -1,
      -1,    -1,    -1,    -1,   242,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     223,   224,   225,    -1,    -1,   263,    -1,    -1,    -1,    -1,
     473,   474,   475,   476,   272,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   488,   489,   490,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   499,    -1,   296,    -1,
     503,   504,   300,    -1,    -1,    -1,    -1,    -1,   306,    -1,
      -1,    -1,    -1,    -1,    -1,   518,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   531,    -1,
      -1,    -1,    -1,    -1,    -1,   538,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   548,    -1,    -1,   551,    -1,
      -1,    -1,    -1,   556,   557,   558,    -1,    -1,   561,   562,
     563,   564,   565,   566,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     593,   600,    -1,   602,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   619,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   635,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   533,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   599,    -1,   601,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   594,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   617,
     618,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,   106,   107,    -1,   109,   110,   111,
      -1,    -1,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,    -1,
     222,    -1,    -1,    -1,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   270,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   278,   279,    -1,   281,
     282,    -1,   284,   285,    -1,    -1,   288,   289,   290,   291,
     292,   293,   294,   295,    -1,    -1,    -1,   299,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   308,   309,   310
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   312,   313,     0,    11,    12,    46,    52,    96,   112,
     162,   221,   242,   263,   272,   296,   300,   306,   314,   315,
     322,   325,   328,   331,   339,   347,   577,   588,   610,   614,
     622,   635,   645,   332,   323,   326,   329,   340,   348,   578,
     589,   611,   615,   623,   636,   646,   333,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    49,
      50,    51,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,   105,   106,   107,   109,   110,   111,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   141,   142,   143,   144,   145,   146,   147,
     148,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   222,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   270,
     278,   279,   281,   282,   284,   285,   288,   289,   290,   291,
     292,   293,   294,   295,   299,   308,   309,   310,   324,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,   410,
     411,   412,   413,   414,   415,   416,   417,   418,   419,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   535,   536,   537,   538,   539,   540,
     541,   542,   543,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   618,   619,   620,   621,   644,
      45,    47,    48,   108,   139,   149,   287,   327,   556,   557,
     558,   559,   560,   561,   562,    45,    53,    54,   138,   140,
     286,   330,   563,   564,   565,   566,   567,   568,    45,   271,
     273,   274,   275,   276,   277,   283,   341,   569,   570,   571,
     572,   573,   574,   575,   576,   301,   302,   303,   304,   305,
     342,   343,   344,   345,   346,   349,   569,   570,   571,   572,
     573,    97,    98,    99,   100,   101,   102,   103,   104,   579,
     580,   581,   582,   583,   584,   585,   586,   587,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   590,   591,   592,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   113,   612,   613,
     307,   616,   617,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   624,   625,   626,   627,   628,   629,   630,
     631,   632,   633,   634,   264,   265,   266,   267,   268,   269,
     637,   638,   639,   640,   641,   642,   643,   297,   298,   647,
     648,   649,    45,   223,   224,   225,   334,   335,   336,   337,
     338,   421,   446,   447,   454,   455,   511,   512,   513,   620,
     621,    10,   320,   320,   320,   320,    10,    10,   316,   316,
     316,   316,   316,   316,   320,   320,   320,   316,   320,    10,
      10,    10,    10,    10,    10,   321,    10,   319,    10,   318,
     321,   319,   320,   320,    10,   319,   318,    10,    10,   316,
     316,    10,   316,   316,    10,    10,   316,    10,    10,    10,
      10,   320,   316,   316,   320,   318,   321,   319,    10,    10,
     316,    10,    10,   316,   320,   316,    10,    10,    10,   316,
      10,   317,   316,   316,   316,    10,    10,    10,    10,   321,
     316,    10,    10,   316,    10,   320,   320,   316,    10,   317,
     317,   320,   320,    10,   320,   320,   320,   321,   318,   316,
     316,   321,   316,   316,   316,   316,   316,   316,   316,   316,
      10,    10,   320,    10,   320,    10,   320,   321,   321,   316,
     316,   316,   316,   320,   320,   316,   316,   316,   320,   316,
      10,   316,    10,    10,    10,    10,   316,   316,   320,   316,
     320,   319,   321,   320,   319,   321,    10,    10,   320,   320,
      10,    10,   316,    10,    10,    10,    10,    10,    10,    10,
      10,   320,   316,   316,   316,   316,    10,    10,    10,    10,
      10,    10,    10,   316,   320,   316,   320,   320,   316,   316,
     316,    10,   316,   316,   316,   316,   316,   317,   316,   316,
     318,   316,   318,   316,    10,   316,   320,    10,   316,   316,
      10,    10,   320,   318,   316,    10,   316,   316,   320,   316,
     321,    10,    10,   316,    10,    10,   320,    10,    10,    10,
      10,   316,   316,   316,   316,    10,    10,    10,   316,   316,
     316,    10,   316,    10,    10,    10,   316,   316,    10,    10,
      10,    10,   316,    10,   316,    10,   320,    10,    10,    10,
      10,   316,   316,    10,    10,   316,    10,    10,    10,    10,
     316,   316,   316,    10,    10,   316,   316,   316,   316,   316,
     316,    10,    10,   316,   320,    10,    10,    10,    10,   321,
     319,   321,   319,    10,    10,    10,   320,   320,   316,    10,
      10,    10,    10,    10,   316,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   311,   312,   313,   313,   314,   314,   314,   314,   314,
     314,   314,   314,   314,   314,   314,   314,   314,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   323,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     325,   326,   326,   327,   327,   327,   327,   327,   327,   327,
     328,   329,   329,   330,   330,   330,   330,   330,   330,   332,
     331,   333,   333,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   335,   336,   337,
     338,   339,   340,   340,   341,   341,   341,   341,   341,   341,
     341,   341,   342,   343,   344,   345,   346,   347,   348,   348,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   537,   538,   539,
     540,   541,   542,   543,   544,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,   564,   565,   566,   567,   568,   569,
     570,   571,   572,   573,   574,   575,   576,   577,   578,   578,
     579,   579,   579,   579,   579,   579,   579,   579,   580,   581,
     582,   583,   584,   585,   586,   587,   588,   589,   589,   590,
     590,   590,   590,   590,   590,   590,   590,   590,   590,   590,
     590,   590,   590,   590,   590,   590,   590,   590,   591,   592,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   611,
     612,   613,   614,   615,   615,   616,   617,   618,   619,   620,
     621,   622,   623,   623,   624,   624,   624,   624,   624,   624,
     624,   624,   624,   624,   625,   626,   627,   628,   629,   630,
     631,   632,   633,   634,   635,   636,   636,   637,   637,   637,
     637,   637,   637,   638,   639,   640,   641,   642,   643,   644,
     645,   646,   646,   647,   647,   648,   649
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     2,     2,     2,
       2,     1,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     1,     1,     1,     1,     1,     1,     0,
       3,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     2,
       2,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     3,     4,     4,     4,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       1,     2,     1,     2,     0,     1,     2,     2,     2,     3,
       3,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     3,
       1,     2,     0,     1,     1,     2,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 197 "./util/configparser.y"
        {
		if (config_finalize(cfg_parser->cfg) != 0) {
			yyerror("global configuration error");
		}
	}
#line 3002 "util/configparser.c"
    break;

  case 19:
#line 213 "./util/configparser.y"
        {
		OUTYY(("\nP(force-toplevel)\n"));
	}
#line 3010 "util/configparser.c"
    break;

  case 20:
#line 219 "./util/configparser.y"
        {
        if (strcasecmp((yyvsp[0].str), "yes") == 0) {
            (yyval.uval) = 1;
        } else if (strcasecmp((yyvsp[0].str), "no") == 0) {
            (yyval.uval) = 0;
        } else {
            yyerror("expect yes or no");
        }

        free((yyvsp[0].str));
    }
#line 3026 "util/configparser.c"
    break;

  case 21:
#line 232 "./util/configparser.y"
        {
		if (*(yyvsp[0].str) == '\0') {
			(yyval.lval) = 0;
		} else {
			char *ep;
			long  v;

			if ((v = strtol((yyvsp[0].str), &ep, 0)) < 0 || *ep != '\0') {
				yyerror("number expected");
			}

			(yyval.lval) = v;
		}

		free((yyvsp[0].str));
	}
#line 3047 "util/configparser.c"
    break;

  case 22:
#line 250 "./util/configparser.y"
        {
		char *ep;
		long  v;

		if ((v = strtol((yyvsp[0].str), &ep, 0)) <= 0 || *ep != '\0') {
			yyerror("number expected");
		}

		(yyval.lval) = v;
		free((yyvsp[0].str));
	}
#line 3063 "util/configparser.c"
    break;

  case 23:
#line 263 "./util/configparser.y"
        {
		char *ep;
		long  v;

		if ((v = strtol((yyvsp[0].str), &ep, 0)) <= 0 || *ep != '\0') {
			yyerror("number expected");
		} else if (!is_pow2(v)) {
			yyerror("must be a power of 2");
		}

		(yyval.lval) = v;
		free((yyvsp[0].str));
	}
#line 3081 "util/configparser.c"
    break;

  case 24:
#line 277 "./util/configparser.y"
        {
		char *ep;
		long  v;

		if ((v = strtol((yyvsp[0].str), &ep, 0)) < 0 || *ep != '\0') {
			yyerror("number expected");
		}

		(yyval.lval) = v;
		free((yyvsp[0].str));
	}
#line 3097 "util/configparser.c"
    break;

  case 25:
#line 290 "./util/configparser.y"
        {
		if(!cfg_parse_memsize((yyvsp[0].str), &(yyval.sval))) {
			yyerror("memory size expected");
		}

		free((yyvsp[0].str));
	}
#line 3109 "util/configparser.c"
    break;

  case 26:
#line 301 "./util/configparser.y"
        {
		OUTYY(("\nP(server:)\n"));
	}
#line 3117 "util/configparser.c"
    break;

  case 240:
#line 403 "./util/configparser.y"
        {
		struct config_stub* s;
		OUTYY(("\nP(stub_zone:)\n"));
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->stubs;
			cfg_parser->cfg->stubs = s;
		} else
			yyerror("out of memory");
	}
#line 3132 "util/configparser.c"
    break;

  case 250:
#line 420 "./util/configparser.y"
        {
		struct config_stub* s;
		OUTYY(("\nP(forward_zone:)\n"));
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->forwards;
			cfg_parser->cfg->forwards = s;
		} else
			yyerror("out of memory");
	}
#line 3147 "util/configparser.c"
    break;

  case 259:
#line 437 "./util/configparser.y"
        {
		OUTYY(("\nP(view:)\n"));

		struct config_view* view;

		if ((view = config_view_create(cfg_parser->cfg)) != NULL) {
			// Future configuration values will go here until the view ends

			cfg_parser->cfg = &view->cfg_view;
			cfg_parser->view_cfg = view;
		} else {
			yyerror("out of memory");
		}
	}
#line 3166 "util/configparser.c"
    break;

  case 260:
#line 452 "./util/configparser.y"
        {
		struct config_view *view   = cfg_parser->view_cfg;
		struct config_file *server = cfg_parser->server_cfg;

		// Successfully parsed the view - link in the new view and restore
		// the server configuration to the active position

		if (view == NULL) {
			yyerror("empty view");
		} else if (config_view_validate(view) != 0) {
			yyerror("view: invalid configuration");
		} else {
			cfg_parser->view_cfg = NULL;
			view->next           = server->views;
			server->views        = view;
			cfg_parser->cfg      = server;
		}
	}
#line 3189 "util/configparser.c"
    break;

  case 277:
#line 489 "./util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if ((yyvsp[0].str)[0] == '\0') {
			// This could be legal, but don't allow it. Too confusing

			yyerror("Invalid view name \"\"");
		}
		if (cfg_parser->view_cfg->name != NULL) {
			yyerror("view name override, there must be one "
			        "name for one view");
			free(cfg_parser->view_cfg->name);
		}
		cfg_parser->view_cfg->name = (yyvsp[0].str);
	}
#line 3208 "util/configparser.c"
    break;

  case 278:
#line 505 "./util/configparser.y"
        {
		OUTYY(("P(view-first:%u)\n", (yyvsp[0].uval)));

		// If we're setting view-first, make sure we disable the default
		// local zones for the view. If clearing, include the defaults.

		cfg_parser->view_cfg->isfirst = (yyvsp[0].uval);
		cfg_parser->cfg->local_zones_disable_default = (yyvsp[0].uval);
	}
#line 3222 "util/configparser.c"
    break;

  case 279:
#line 516 "./util/configparser.y"
        {
		OUTYY(("P(match-clients:%s)\n", (yyvsp[0].str)));
		if (!cfg_strlist_insert(&cfg_parser->view_cfg->match_clients, (yyvsp[0].str))) {
			yyerror("out of memory");
		}
	}
#line 3233 "util/configparser.c"
    break;

  case 280:
#line 524 "./util/configparser.y"
        {
		OUTYY(("P(match-destinations:%s)\n", (yyvsp[0].str)));
		if (!cfg_strobjlist_insert(&cfg_parser->view_cfg->match_destinations,
		                           (yyvsp[0].str),
		                           cfg_parser->view_cfg)) {
			yyerror("out of memory");
		}
	}
#line 3246 "util/configparser.c"
    break;

  case 281:
#line 534 "./util/configparser.y"
        {
		struct config_auth* s;
		OUTYY(("\nP(auth_zone:)\n"));
		s = (struct config_auth*)calloc(1, sizeof(struct config_auth));
		if(s) {
			s->next = cfg_parser->cfg->auths;
			cfg_parser->cfg->auths = s;
			/* defaults for auth zone */
			s->for_downstream = 1;
			s->for_upstream = 1;
			s->fallback_enabled = 0;
			s->isrpz = 0;
		} else
			yyerror("out of memory");
	}
#line 3266 "util/configparser.c"
    break;

  case 292:
#line 558 "./util/configparser.y"
        {
		uint8_t* bitlist;
		size_t len = 0;
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[0].str)));
		bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
		}
		if(bitlist) {
			cfg_parser->cfg->auths->rpz_taglist = bitlist;
			cfg_parser->cfg->auths->rpz_taglistlen = len;

		}
	}
#line 3287 "util/configparser.c"
    break;

  case 293:
#line 577 "./util/configparser.y"
        {
		OUTYY(("P(rpz_action_override:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "nxdomain")!=0 && strcmp((yyvsp[0].str), "nodata")!=0 &&
		   strcmp((yyvsp[0].str), "passthru")!=0 && strcmp((yyvsp[0].str), "drop")!=0 &&
		   strcmp((yyvsp[0].str), "cname")!=0 && strcmp((yyvsp[0].str), "disabled")!=0) {
			yyerror("rpz-action-override action: expected nxdomain, "
				"nodata, passthru, drop, cname or disabled");
			free((yyvsp[0].str));
			cfg_parser->cfg->auths->rpz_action_override = NULL;
		}
		else {
			cfg_parser->cfg->auths->rpz_action_override = (yyvsp[0].str);
		}
	}
#line 3306 "util/configparser.c"
    break;

  case 294:
#line 594 "./util/configparser.y"
        {
		OUTYY(("P(rpz_cname_override:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->auths->rpz_cname);
		cfg_parser->cfg->auths->rpz_cname = (yyvsp[0].str);
	}
#line 3316 "util/configparser.c"
    break;

  case 295:
#line 602 "./util/configparser.y"
        {
		OUTYY(("P(rpz_log:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->auths->rpz_log = (yyvsp[0].uval);
	}
#line 3325 "util/configparser.c"
    break;

  case 296:
#line 609 "./util/configparser.y"
        {
		OUTYY(("P(rpz_log_name:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->auths->rpz_log_name);
		cfg_parser->cfg->auths->rpz_log_name = (yyvsp[0].str);
	}
#line 3335 "util/configparser.c"
    break;

  case 297:
#line 617 "./util/configparser.y"
        {
		struct config_auth* s;
		OUTYY(("\nP(rpz:)\n"));
		s = (struct config_auth*)calloc(1, sizeof(struct config_auth));
		if(s) {
			s->next = cfg_parser->cfg->auths;
			cfg_parser->cfg->auths = s;
			/* defaults for RPZ auth zone */
			s->for_downstream = 0;
			s->for_upstream = 0;
			s->fallback_enabled = 0;
			s->isrpz = 1;
		} else
			yyerror("out of memory");
	}
#line 3355 "util/configparser.c"
    break;

  case 310:
#line 640 "./util/configparser.y"
        {
		OUTYY(("P(server_num_threads:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->num_threads = (yyvsp[0].lval);
	}
#line 3364 "util/configparser.c"
    break;

  case 311:
#line 646 "./util/configparser.y"
        {
		OUTYY(("P(server_verbosity:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->verbosity = (yyvsp[0].lval);
	}
#line 3373 "util/configparser.c"
    break;

  case 312:
#line 652 "./util/configparser.y"
        {
		OUTYY(("P(server_statistics_interval:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->stat_interval = (yyvsp[0].lval);
	}
#line 3382 "util/configparser.c"
    break;

  case 313:
#line 658 "./util/configparser.y"
        {
		OUTYY(("P(server_statistics_cumulative:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stat_cumulative = (yyvsp[0].uval);
	}
#line 3391 "util/configparser.c"
    break;

  case 314:
#line 664 "./util/configparser.y"
        {
		OUTYY(("P(server_extended_statistics:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stat_extended = (yyvsp[0].uval);
	}
#line 3400 "util/configparser.c"
    break;

  case 315:
#line 670 "./util/configparser.y"
        {
		OUTYY(("P(server_shm_enable:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->shm_enable = (yyvsp[0].uval);
	}
#line 3409 "util/configparser.c"
    break;

  case 316:
#line 676 "./util/configparser.y"
        {
		OUTYY(("P(server_shm_key:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->shm_key = (yyvsp[0].lval);
	}
#line 3418 "util/configparser.c"
    break;

  case 317:
#line 682 "./util/configparser.y"
        {
		OUTYY(("P(server_port:%lu)\n", (yyvsp[0].lval)));
		if((yyvsp[0].lval) == 0)
			yyerror("port number expected");
		else
			cfg_parser->cfg->port = (yyvsp[0].lval);
	}
#line 3430 "util/configparser.c"
    break;

  case 318:
#line 691 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_send_client_subnet:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet, (yyvsp[0].str)))
			fatal_exit("out of memory adding client-subnet");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 3445 "util/configparser.c"
    break;

  case 319:
#line 703 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_zone:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet_zone,
			(yyvsp[0].str)))
			fatal_exit("out of memory adding client-subnet-zone");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 3461 "util/configparser.c"
    break;

  case 320:
#line 717 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_always_forward:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->client_subnet_always_forward = (yyvsp[0].uval);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
	}
#line 3474 "util/configparser.c"
    break;

  case 321:
#line 727 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(client_subnet_opcode:%s)\n", (yyvsp[0].str)));
		OUTYY(("P(Deprecated option, ignoring)\n"));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3488 "util/configparser.c"
    break;

  case 322:
#line 738 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv4:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi((yyvsp[0].str)) > 32)
			cfg_parser->cfg->max_client_subnet_ipv4 = 32;
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->max_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv4 = (uint8_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3508 "util/configparser.c"
    break;

  case 323:
#line 755 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv6:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi((yyvsp[0].str)) > 128)
			cfg_parser->cfg->max_client_subnet_ipv6 = 128;
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->max_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv6 = (uint8_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3528 "util/configparser.c"
    break;

  case 324:
#line 772 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv4:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi((yyvsp[0].str)) > 32)
			cfg_parser->cfg->min_client_subnet_ipv4 = 32;
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->min_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv4 = (uint8_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3548 "util/configparser.c"
    break;

  case 325:
#line 789 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv6:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi((yyvsp[0].str)) > 128)
			cfg_parser->cfg->min_client_subnet_ipv6 = 128;
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->min_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv6 = (uint8_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3568 "util/configparser.c"
    break;

  case 326:
#line 806 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv4:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("IPv4 ECS tree size expected");
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv4 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv4 = (uint32_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3586 "util/configparser.c"
    break;

  case 327:
#line 821 "./util/configparser.y"
        {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv6:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("IPv6 ECS tree size expected");
		else if (atoi((yyvsp[0].str)) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv6 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv6 = (uint32_t)atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 3604 "util/configparser.c"
    break;

  case 328:
#line 836 "./util/configparser.y"
        {
		OUTYY(("P(server_interface:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = (yyvsp[0].str);
	}
#line 3620 "util/configparser.c"
    break;

  case 329:
#line 849 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_interface:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->num_out_ifs == 0)
			cfg_parser->cfg->out_ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->out_ifs = realloc(
			cfg_parser->cfg->out_ifs,
			(cfg_parser->cfg->num_out_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->out_ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->out_ifs[
				cfg_parser->cfg->num_out_ifs++] = (yyvsp[0].str);
	}
#line 3638 "util/configparser.c"
    break;

  case 330:
#line 864 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_range:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->outgoing_num_ports = (yyvsp[0].lval);
	}
#line 3647 "util/configparser.c"
    break;

  case 331:
#line 870 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_port_permit:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 1,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 3659 "util/configparser.c"
    break;

  case 332:
#line 879 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 0,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 3671 "util/configparser.c"
    break;

  case 333:
#line 888 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_num_tcp:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->outgoing_num_tcp = (yyvsp[0].lval);
	}
#line 3680 "util/configparser.c"
    break;

  case 334:
#line 894 "./util/configparser.y"
        {
		OUTYY(("P(server_incoming_num_tcp:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->incoming_num_tcp = (yyvsp[0].lval);
	}
#line 3689 "util/configparser.c"
    break;

  case 335:
#line 900 "./util/configparser.y"
        {
		OUTYY(("P(server_interface_automatic:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->if_automatic = (yyvsp[0].uval);
	}
#line 3698 "util/configparser.c"
    break;

  case 336:
#line 906 "./util/configparser.y"
        {
		OUTYY(("P(server_do_ip4:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_ip4 = (yyvsp[0].uval);
	}
#line 3707 "util/configparser.c"
    break;

  case 337:
#line 912 "./util/configparser.y"
        {
		OUTYY(("P(server_do_ip6:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_ip6 = (yyvsp[0].uval);
	}
#line 3716 "util/configparser.c"
    break;

  case 338:
#line 918 "./util/configparser.y"
        {
		OUTYY(("P(server_do_udp:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_udp = (yyvsp[0].uval);
	}
#line 3725 "util/configparser.c"
    break;

  case 339:
#line 924 "./util/configparser.y"
        {
		OUTYY(("P(server_do_tcp:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_tcp = (yyvsp[0].uval);
	}
#line 3734 "util/configparser.c"
    break;

  case 340:
#line 930 "./util/configparser.y"
        {
		OUTYY(("P(server_prefer_ip4:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->prefer_ip4 = (yyvsp[0].uval);
	}
#line 3743 "util/configparser.c"
    break;

  case 341:
#line 936 "./util/configparser.y"
        {
		OUTYY(("P(server_prefer_ip6:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->prefer_ip6 = (yyvsp[0].uval);
	}
#line 3752 "util/configparser.c"
    break;

  case 342:
#line 942 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_mss:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->tcp_mss = (yyvsp[0].lval);
	}
#line 3761 "util/configparser.c"
    break;

  case 343:
#line 948 "./util/configparser.y"
        {
		OUTYY(("P(server_outgoing_tcp_mss:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->outgoing_tcp_mss = (yyvsp[0].lval);
	}
#line 3770 "util/configparser.c"
    break;

  case 344:
#line 954 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_idle_timeout:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) >= 120000)
			cfg_parser->cfg->tcp_idle_timeout = 120000;
		else if ((yyvsp[0].lval) <= 1)
			cfg_parser->cfg->tcp_idle_timeout = 1;
		else
			cfg_parser->cfg->tcp_idle_timeout = (yyvsp[0].lval);
	}
#line 3784 "util/configparser.c"
    break;

  case 345:
#line 965 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_keepalive:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_tcp_keepalive = (yyvsp[0].uval);
	}
#line 3793 "util/configparser.c"
    break;

  case 346:
#line 971 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_keepalive_timeout:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) >= 6553500)
			cfg_parser->cfg->tcp_keepalive_timeout = 6553500;
		else if ((yyvsp[0].lval) < 1)
			cfg_parser->cfg->tcp_keepalive_timeout = 0;
		else
			cfg_parser->cfg->tcp_keepalive_timeout = (yyvsp[0].lval);
	}
#line 3807 "util/configparser.c"
    break;

  case 347:
#line 982 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_upstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->tcp_upstream = (yyvsp[0].uval);
	}
#line 3816 "util/configparser.c"
    break;

  case 348:
#line 989 "./util/configparser.y"
        {
		OUTYY(("P(server_udp_upstream_without_downstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->udp_upstream_without_downstream = (yyvsp[0].uval);
	}
#line 3825 "util/configparser.c"
    break;

  case 349:
#line 995 "./util/configparser.y"
        {
		OUTYY(("P(server_ssl_upstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->ssl_upstream = (yyvsp[0].uval);
	}
#line 3834 "util/configparser.c"
    break;

  case 350:
#line 1001 "./util/configparser.y"
        {
		OUTYY(("P(server_ssl_service_key:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ssl_service_key);
		cfg_parser->cfg->ssl_service_key = (yyvsp[0].str);
	}
#line 3844 "util/configparser.c"
    break;

  case 351:
#line 1008 "./util/configparser.y"
        {
		OUTYY(("P(server_ssl_service_pem:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ssl_service_pem);
		cfg_parser->cfg->ssl_service_pem = (yyvsp[0].str);
	}
#line 3854 "util/configparser.c"
    break;

  case 352:
#line 1015 "./util/configparser.y"
        {
		OUTYY(("P(server_ssl_port:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) == 0)
			yyerror("port number expected");
		else
			cfg_parser->cfg->ssl_port = (yyvsp[0].lval);
	}
#line 3866 "util/configparser.c"
    break;

  case 353:
#line 1024 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_cert_bundle:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = (yyvsp[0].str);
	}
#line 3876 "util/configparser.c"
    break;

  case 354:
#line 1031 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_win_cert:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->tls_win_cert = (yyvsp[0].uval);
	}
#line 3885 "util/configparser.c"
    break;

  case 355:
#line 1037 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_additional_port:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->tls_additional_port,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3896 "util/configparser.c"
    break;

  case 356:
#line 1045 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_ciphers:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->tls_ciphers);
		cfg_parser->cfg->tls_ciphers = (yyvsp[0].str);
	}
#line 3906 "util/configparser.c"
    break;

  case 357:
#line 1052 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_ciphersuites:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->tls_ciphersuites);
		cfg_parser->cfg->tls_ciphersuites = (yyvsp[0].str);
	}
#line 3916 "util/configparser.c"
    break;

  case 358:
#line 1059 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_session_ticket_keys:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->tls_session_ticket_keys,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3927 "util/configparser.c"
    break;

  case 359:
#line 1067 "./util/configparser.y"
        {
		OUTYY(("P(server_tls_use_sni:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->tls_use_sni = (yyvsp[0].uval);
	}
#line 3936 "util/configparser.c"
    break;

  case 360:
#line 1073 "./util/configparser.y"
        {
		OUTYY(("P(server_https_port:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) == 0)
			yyerror("port number expected");
		else
			cfg_parser->cfg->https_port = (yyvsp[0].lval);
	}
#line 3948 "util/configparser.c"
    break;

  case 361:
#line 1081 "./util/configparser.y"
        {
		OUTYY(("P(server_http_endpoint:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->http_endpoint);
		if((yyvsp[0].str) && (yyvsp[0].str)[0] != '/') {
			cfg_parser->cfg->http_endpoint = malloc(strlen((yyvsp[0].str))+2);
			if(!cfg_parser->cfg->http_endpoint)
				yyerror("out of memory");
			cfg_parser->cfg->http_endpoint[0] = '/';
			memmove(cfg_parser->cfg->http_endpoint+1, (yyvsp[0].str),
				strlen((yyvsp[0].str))+1);
			free((yyvsp[0].str));
		} else {
			cfg_parser->cfg->http_endpoint = (yyvsp[0].str);
		}
	}
#line 3968 "util/configparser.c"
    break;

  case 362:
#line 1097 "./util/configparser.y"
        {
		OUTYY(("P(server_http_max_streams:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->http_max_streams = (yyvsp[0].lval);
	}
#line 3977 "util/configparser.c"
    break;

  case 363:
#line 1102 "./util/configparser.y"
        {
		OUTYY(("P(server_http_query_buffer_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->http_query_buffer_size = (yyvsp[0].sval);
	}
#line 3986 "util/configparser.c"
    break;

  case 364:
#line 1107 "./util/configparser.y"
        {
		OUTYY(("P(server_http_response_buffer_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->http_response_buffer_size = (yyvsp[0].sval);
	}
#line 3995 "util/configparser.c"
    break;

  case 365:
#line 1112 "./util/configparser.y"
        {
		OUTYY(("P(server_http_nodelay:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->http_nodelay = (yyvsp[0].uval);
	}
#line 4004 "util/configparser.c"
    break;

  case 366:
#line 1117 "./util/configparser.y"
        {
		OUTYY(("P(server_http_notls_downstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->http_notls_downstream = (yyvsp[0].uval);
	}
#line 4013 "util/configparser.c"
    break;

  case 367:
#line 1122 "./util/configparser.y"
        {
		OUTYY(("P(server_use_systemd:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->use_systemd = (yyvsp[0].uval);
	}
#line 4022 "util/configparser.c"
    break;

  case 368:
#line 1128 "./util/configparser.y"
        {
		OUTYY(("P(server_do_daemonize:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->do_daemonize = (yyvsp[0].uval);
	}
#line 4031 "util/configparser.c"
    break;

  case 369:
#line 1134 "./util/configparser.y"
        {
		OUTYY(("P(server_use_syslog:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->use_syslog = (yyvsp[0].uval);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if ((yyvsp[0].uval)) {
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
		}
#endif
	}
#line 4046 "util/configparser.c"
    break;

  case 370:
#line 1146 "./util/configparser.y"
        {
		OUTYY(("P(server_log_time_ascii:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->log_time_ascii = (yyvsp[0].uval);
	}
#line 4055 "util/configparser.c"
    break;

  case 371:
#line 1152 "./util/configparser.y"
        {
		OUTYY(("P(server_log_queries:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->log_queries = (yyvsp[0].uval);
	}
#line 4064 "util/configparser.c"
    break;

  case 372:
#line 1158 "./util/configparser.y"
  {
  	OUTYY(("P(server_log_replies:%u)\n", (yyvsp[0].uval)));
  	cfg_parser->cfg->log_replies = (yyvsp[0].uval);
  }
#line 4073 "util/configparser.c"
    break;

  case 373:
#line 1164 "./util/configparser.y"
  {
  	OUTYY(("P(server_log_tag_queryreply:%u)\n", (yyvsp[0].uval)));
  	cfg_parser->cfg->log_tag_queryreply = (yyvsp[0].uval);
  }
#line 4082 "util/configparser.c"
    break;

  case 374:
#line 1170 "./util/configparser.y"
        {
		OUTYY(("P(server_log_servfail:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->log_servfail = (yyvsp[0].uval);
	}
#line 4091 "util/configparser.c"
    break;

  case 375:
#line 1176 "./util/configparser.y"
  {
  	OUTYY(("P(server_log_local_actions:%u)\n", (yyvsp[0].uval)));
  	cfg_parser->cfg->log_local_actions = (yyvsp[0].uval);
  }
#line 4100 "util/configparser.c"
    break;

  case 376:
#line 1182 "./util/configparser.y"
        {
		OUTYY(("P(server_chroot:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = (yyvsp[0].str);
	}
#line 4110 "util/configparser.c"
    break;

  case 377:
#line 1189 "./util/configparser.y"
        {
		OUTYY(("P(server_username:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = (yyvsp[0].str);
	}
#line 4120 "util/configparser.c"
    break;

  case 378:
#line 1196 "./util/configparser.y"
        {
		OUTYY(("P(server_directory:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = (yyvsp[0].str);
		/* change there right away for includes relative to this */
		if((yyvsp[0].str)[0]) {
			char* d;
#ifdef UB_ON_WINDOWS
			w_config_adjust_directory(cfg_parser->cfg);
#endif
			d = cfg_parser->cfg->directory;
			/* adjust directory if we have already chroot,
			 * like, we reread after sighup */
			if(cfg_parser->chroot && cfg_parser->chroot[0] &&
				strncmp(d, cfg_parser->chroot, strlen(
				cfg_parser->chroot)) == 0)
				d += strlen(cfg_parser->chroot);
			if(d[0]) {
			    if(chdir(d))
				log_err("cannot chdir to directory: %s (%s)",
					d, strerror(errno));
			}
		}
	}
#line 4149 "util/configparser.c"
    break;

  case 379:
#line 1222 "./util/configparser.y"
        {
		OUTYY(("P(server_logfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = (yyvsp[0].str);
		cfg_parser->cfg->use_syslog = 0;
	}
#line 4160 "util/configparser.c"
    break;

  case 380:
#line 1230 "./util/configparser.y"
        {
		OUTYY(("P(server_pidfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = (yyvsp[0].str);
	}
#line 4170 "util/configparser.c"
    break;

  case 381:
#line 1237 "./util/configparser.y"
        {
		OUTYY(("P(server_root_hints:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4180 "util/configparser.c"
    break;

  case 382:
#line 1244 "./util/configparser.y"
        {
		OUTYY(("P(server_dlv_anchor_file:%s)\n", (yyvsp[0].str)));
		log_warn("option dlv-anchor-file ignored: DLV is decommissioned");
		free((yyvsp[0].str));
	}
#line 4190 "util/configparser.c"
    break;

  case 383:
#line 1251 "./util/configparser.y"
        {
		OUTYY(("P(server_dlv_anchor:%s)\n", (yyvsp[0].str)));
		log_warn("option dlv-anchor ignored: DLV is decommissioned");
		free((yyvsp[0].str));
	}
#line 4200 "util/configparser.c"
    break;

  case 384:
#line 1258 "./util/configparser.y"
        {
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4211 "util/configparser.c"
    break;

  case 385:
#line 1266 "./util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4222 "util/configparser.c"
    break;

  case 386:
#line 1274 "./util/configparser.y"
        {
		OUTYY(("P(server_trusted_keys_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4233 "util/configparser.c"
    break;

  case 387:
#line 1282 "./util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4243 "util/configparser.c"
    break;

  case 388:
#line 1289 "./util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor_signaling:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->trust_anchor_signaling = (yyvsp[0].uval);
	}
#line 4252 "util/configparser.c"
    break;

  case 389:
#line 1295 "./util/configparser.y"
        {
		OUTYY(("P(server_root_key_sentinel:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->root_key_sentinel = (yyvsp[0].uval);
	}
#line 4261 "util/configparser.c"
    break;

  case 390:
#line 1301 "./util/configparser.y"
        {
		OUTYY(("P(server_domain_insecure:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4271 "util/configparser.c"
    break;

  case 391:
#line 1308 "./util/configparser.y"
        {
		OUTYY(("P(server_hide_identity:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->hide_identity = (yyvsp[0].uval);
	}
#line 4280 "util/configparser.c"
    break;

  case 392:
#line 1314 "./util/configparser.y"
        {
		OUTYY(("P(server_hide_version:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->hide_version = (yyvsp[0].uval);
	}
#line 4289 "util/configparser.c"
    break;

  case 393:
#line 1320 "./util/configparser.y"
        {
		OUTYY(("P(server_hide_trustanchor:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->hide_trustanchor = (yyvsp[0].uval);
	}
#line 4298 "util/configparser.c"
    break;

  case 394:
#line 1326 "./util/configparser.y"
        {
		OUTYY(("P(server_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = (yyvsp[0].str);
	}
#line 4308 "util/configparser.c"
    break;

  case 395:
#line 1333 "./util/configparser.y"
        {
		OUTYY(("P(server_version:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = (yyvsp[0].str);
	}
#line 4318 "util/configparser.c"
    break;

  case 396:
#line 1340 "./util/configparser.y"
        {
		OUTYY(("P(server_nsid:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->nsid_cfg_str);
		cfg_parser->cfg->nsid_cfg_str = (yyvsp[0].str);
		free(cfg_parser->cfg->nsid);
		cfg_parser->cfg->nsid = NULL;
		cfg_parser->cfg->nsid_len = 0;
		if (*(yyvsp[0].str) == 0)
			; /* pass; empty string is not setting nsid */
		else if (!(cfg_parser->cfg->nsid = cfg_parse_nsid(
					(yyvsp[0].str), &cfg_parser->cfg->nsid_len)))
			yyerror("the NSID must be either a hex string or an "
			    "ascii character string prepended with ascii_.");
	}
#line 4337 "util/configparser.c"
    break;

  case 397:
#line 1356 "./util/configparser.y"
        {
		OUTYY(("P(server_so_rcvbuf:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->so_rcvbuf = (yyvsp[0].sval);
	}
#line 4346 "util/configparser.c"
    break;

  case 398:
#line 1362 "./util/configparser.y"
        {
		OUTYY(("P(server_so_sndbuf:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->so_sndbuf = (yyvsp[0].sval);
	}
#line 4355 "util/configparser.c"
    break;

  case 399:
#line 1368 "./util/configparser.y"
    {
        OUTYY(("P(server_so_reuseport:%u)\n", (yyvsp[0].uval)));
        cfg_parser->cfg->so_reuseport = (yyvsp[0].uval);
    }
#line 4364 "util/configparser.c"
    break;

  case 400:
#line 1374 "./util/configparser.y"
    {
        OUTYY(("P(server_ip_transparent:%u)\n", (yyvsp[0].uval)));
        cfg_parser->cfg->ip_transparent = (yyvsp[0].uval);
    }
#line 4373 "util/configparser.c"
    break;

  case 401:
#line 1380 "./util/configparser.y"
    {
        OUTYY(("P(server_ip_freebind:%u)\n", (yyvsp[0].uval)));
        cfg_parser->cfg->ip_freebind = (yyvsp[0].uval);
    }
#line 4382 "util/configparser.c"
    break;

  case 402:
#line 1386 "./util/configparser.y"
        {
		OUTYY(("P(server_ip_dscp:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) > 63)
			yyerror("value too large (max 63)");
		else
			cfg_parser->cfg->ip_dscp = (yyvsp[0].lval);
	}
#line 4394 "util/configparser.c"
    break;

  case 403:
#line 1395 "./util/configparser.y"
        {
		OUTYY(("P(server_stream_wait_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->stream_wait_size = (yyvsp[0].sval);
	}
#line 4403 "util/configparser.c"
    break;

  case 404:
#line 1401 "./util/configparser.y"
        {
		OUTYY(("P(server_edns_buffer_size:%s)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) < 12)
			yyerror("edns buffer size too small");
		else if ((yyvsp[0].lval) >= 65535)
			cfg_parser->cfg->edns_buffer_size = 65535;
		else
			cfg_parser->cfg->edns_buffer_size =(yyvsp[0].lval);
	}
#line 4417 "util/configparser.c"
    break;

  case 405:
#line 1412 "./util/configparser.y"
        {
		OUTYY(("P(server_msg_buffer_size:%lu)\n", (yyvsp[0].lval)));
		if ((yyvsp[0].lval) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else
			cfg_parser->cfg->msg_buffer_size = (yyvsp[0].lval);
	}
#line 4429 "util/configparser.c"
    break;

  case 406:
#line 1421 "./util/configparser.y"
        {
		OUTYY(("P(server_msg_cache_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->msg_cache_size = (yyvsp[0].sval);
	}
#line 4438 "util/configparser.c"
    break;

  case 407:
#line 1427 "./util/configparser.y"
        {
		OUTYY(("P(server_msg_cache_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->msg_cache_slabs = (yyvsp[0].lval);
	}
#line 4447 "util/configparser.c"
    break;

  case 408:
#line 1433 "./util/configparser.y"
        {
		OUTYY(("P(server_num_queries_per_thread:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->num_queries_per_thread = (yyvsp[0].lval);
	}
#line 4456 "util/configparser.c"
    break;

  case 409:
#line 1439 "./util/configparser.y"
        {
		OUTYY(("P(server_jostle_timeout:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->jostle_time = (yyvsp[0].lval);
	}
#line 4465 "util/configparser.c"
    break;

  case 410:
#line 1445 "./util/configparser.y"
        {
		OUTYY(("P(server_delay_close:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->delay_close = (yyvsp[0].lval);
	}
#line 4474 "util/configparser.c"
    break;

  case 411:
#line 1451 "./util/configparser.y"
        {
		OUTYY(("P(server_udp_connect:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->udp_connect = (yyvsp[0].uval);
	}
#line 4483 "util/configparser.c"
    break;

  case 412:
#line 1457 "./util/configparser.y"
        {
		OUTYY(("P(server_unblock_lan_zones:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->unblock_lan_zones = (yyvsp[0].uval);
	}
#line 4492 "util/configparser.c"
    break;

  case 413:
#line 1463 "./util/configparser.y"
        {
		OUTYY(("P(server_insecure_lan_zones:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->insecure_lan_zones = (yyvsp[0].uval);
	}
#line 4501 "util/configparser.c"
    break;

  case 414:
#line 1469 "./util/configparser.y"
        {
		OUTYY(("P(server_rrset_cache_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->rrset_cache_size = (yyvsp[0].sval);
	}
#line 4510 "util/configparser.c"
    break;

  case 415:
#line 1475 "./util/configparser.y"
        {
		OUTYY(("P(server_rrset_cache_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->rrset_cache_slabs = (yyvsp[0].lval);
	}
#line 4519 "util/configparser.c"
    break;

  case 416:
#line 1481 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_host_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->host_ttl = (yyvsp[0].lval);
	}
#line 4528 "util/configparser.c"
    break;

  case 417:
#line 1487 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_lame_ttl:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4539 "util/configparser.c"
    break;

  case 418:
#line 1495 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_numhosts:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->infra_cache_numhosts = (yyvsp[0].lval);
	}
#line 4548 "util/configparser.c"
    break;

  case 419:
#line 1501 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4559 "util/configparser.c"
    break;

  case 420:
#line 1509 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_slabs:%s)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->infra_cache_slabs = (yyvsp[0].lval);
	}
#line 4568 "util/configparser.c"
    break;

  case 421:
#line 1515 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_min_rtt:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->infra_cache_min_rtt = (yyvsp[0].lval);
	}
#line 4577 "util/configparser.c"
    break;

  case 422:
#line 1521 "./util/configparser.y"
        {
		OUTYY(("P(server_infra_keep_probing:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->infra_keep_probing = (yyvsp[0].uval);
	}
#line 4586 "util/configparser.c"
    break;

  case 423:
#line 1527 "./util/configparser.y"
        {
		OUTYY(("P(server_target_fetch_policy:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = (yyvsp[0].str);
	}
#line 4596 "util/configparser.c"
    break;

  case 424:
#line 1534 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_short_bufsize:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_short_bufsize = (yyvsp[0].uval);
	}
#line 4605 "util/configparser.c"
    break;

  case 425:
#line 1540 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_large_queries:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_large_queries = (yyvsp[0].uval);
	}
#line 4614 "util/configparser.c"
    break;

  case 426:
#line 1546 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_glue:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_glue = (yyvsp[0].uval);
	}
#line 4623 "util/configparser.c"
    break;

  case 427:
#line 1552 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_dnssec_stripped:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_dnssec_stripped = (yyvsp[0].uval);
	}
#line 4632 "util/configparser.c"
    break;

  case 428:
#line 1558 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_below_nxdomain:%u)\n", (yyvsp[0].uval)));

		cfg_parser->cfg->harden_below_nxdomain = (yyvsp[0].uval);
	}
#line 4642 "util/configparser.c"
    break;

  case 429:
#line 1565 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_referral_path:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_referral_path = (yyvsp[0].uval);
	}
#line 4651 "util/configparser.c"
    break;

  case 430:
#line 1571 "./util/configparser.y"
        {
		OUTYY(("P(server_harden_algo_downgrade:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->harden_algo_downgrade = (yyvsp[0].uval);
	}
#line 4660 "util/configparser.c"
    break;

  case 431:
#line 1577 "./util/configparser.y"
        {
		OUTYY(("P(server_use_caps_for_id:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->use_caps_bits_for_id = (yyvsp[0].uval);
	}
#line 4669 "util/configparser.c"
    break;

  case 432:
#line 1583 "./util/configparser.y"
        {
		OUTYY(("P(server_caps_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4679 "util/configparser.c"
    break;

  case 433:
#line 1590 "./util/configparser.y"
        {
		OUTYY(("P(server_private_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4689 "util/configparser.c"
    break;

  case 434:
#line 1597 "./util/configparser.y"
        {
		OUTYY(("P(server_private_domain:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4699 "util/configparser.c"
    break;

  case 435:
#line 1604 "./util/configparser.y"
        {
		OUTYY(("P(server_prefetch:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->prefetch = (yyvsp[0].uval);
	}
#line 4708 "util/configparser.c"
    break;

  case 436:
#line 1610 "./util/configparser.y"
        {
		OUTYY(("P(server_prefetch_key:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->prefetch_key = (yyvsp[0].uval);
	}
#line 4717 "util/configparser.c"
    break;

  case 437:
#line 1616 "./util/configparser.y"
        {
		OUTYY(("P(server_deny_any:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->deny_any = (yyvsp[0].uval);
	}
#line 4726 "util/configparser.c"
    break;

  case 438:
#line 1622 "./util/configparser.y"
        {
		OUTYY(("P(server_unwanted_reply_threshold:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->unwanted_threshold = (yyvsp[0].lval);
	}
#line 4735 "util/configparser.c"
    break;

  case 439:
#line 1628 "./util/configparser.y"
        {
		OUTYY(("P(server_do_not_query_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4745 "util/configparser.c"
    break;

  case 440:
#line 1635 "./util/configparser.y"
        {
		OUTYY(("P(server_do_not_query_localhost:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->donotquery_localhost = (yyvsp[0].uval);
	}
#line 4754 "util/configparser.c"
    break;

  case 441:
#line 1641 "./util/configparser.y"
        {
		OUTYY(("P(server_access_control:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "deny")!=0 && strcmp((yyvsp[0].str), "refuse")!=0 &&
			strcmp((yyvsp[0].str), "deny_non_local")!=0 &&
			strcmp((yyvsp[0].str), "refuse_non_local")!=0 &&
			strcmp((yyvsp[0].str), "allow_setrd")!=0 &&
			strcmp((yyvsp[0].str), "allow")!=0 &&
			strcmp((yyvsp[0].str), "allow_snoop")!=0) {
			yyerror("expected deny, refuse, deny_non_local, "
				"refuse_non_local, allow, allow_setrd or "
				"allow_snoop in access control action");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding acl");
		}
	}
#line 4777 "util/configparser.c"
    break;

  case 442:
#line 1661 "./util/configparser.y"
        {
		OUTYY(("P(server_module_conf:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = (yyvsp[0].str);
	}
#line 4787 "util/configparser.c"
    break;

  case 443:
#line 1668 "./util/configparser.y"
        {
		OUTYY(("P(server_val_override_date:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_date_override = 0;
		} else if(strlen((yyvsp[0].str)) == 14) {
			cfg_parser->cfg->val_date_override =
				cfg_convert_timeval((yyvsp[0].str));
			if(!cfg_parser->cfg->val_date_override)
				yyerror("bad date/time specification");
		} else {
			if(atoi((yyvsp[0].str)) == 0)
				yyerror("number expected");
			cfg_parser->cfg->val_date_override = atoi((yyvsp[0].str));
		}
		free((yyvsp[0].str));
	}
#line 4808 "util/configparser.c"
    break;

  case 444:
#line 1686 "./util/configparser.y"
        {
		OUTYY(("P(server_val_sig_skew_min:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->val_sig_skew_min = (yyvsp[0].lval);
	}
#line 4817 "util/configparser.c"
    break;

  case 445:
#line 1692 "./util/configparser.y"
        {
		OUTYY(("P(server_val_sig_skew_max:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->val_sig_skew_max = (yyvsp[0].lval);
	}
#line 4826 "util/configparser.c"
    break;

  case 446:
#line 1698 "./util/configparser.y"
        {
		OUTYY(("P(server_cache_max_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->max_ttl = (yyvsp[0].lval);
	}
#line 4835 "util/configparser.c"
    break;

  case 447:
#line 1704 "./util/configparser.y"
        {
		OUTYY(("P(server_cache_max_negative_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->max_negative_ttl = (yyvsp[0].lval);
	}
#line 4844 "util/configparser.c"
    break;

  case 448:
#line 1710 "./util/configparser.y"
        {
		OUTYY(("P(server_cache_min_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->min_ttl = (yyvsp[0].lval);
	}
#line 4853 "util/configparser.c"
    break;

  case 449:
#line 1716 "./util/configparser.y"
        {
		OUTYY(("P(server_bogus_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->bogus_ttl = (yyvsp[0].lval);
	}
#line 4862 "util/configparser.c"
    break;

  case 450:
#line 1722 "./util/configparser.y"
        {
		OUTYY(("P(server_val_clean_additional:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->val_clean_additional = (yyvsp[0].uval);
	}
#line 4871 "util/configparser.c"
    break;

  case 451:
#line 1728 "./util/configparser.y"
        {
		OUTYY(("P(server_val_permissive_mode:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->val_permissive_mode = (yyvsp[0].uval);
	}
#line 4880 "util/configparser.c"
    break;

  case 452:
#line 1734 "./util/configparser.y"
        {
		OUTYY(("P(server_aggressive_nsec:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->aggressive_nsec = (yyvsp[0].uval);
	}
#line 4889 "util/configparser.c"
    break;

  case 453:
#line 1740 "./util/configparser.y"
        {
		OUTYY(("P(server_ignore_cd_flag:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->ignore_cd = (yyvsp[0].uval);
	}
#line 4898 "util/configparser.c"
    break;

  case 454:
#line 1746 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_expired:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->serve_expired = (yyvsp[0].uval);
	}
#line 4907 "util/configparser.c"
    break;

  case 455:
#line 1752 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->serve_expired_ttl = (yyvsp[0].lval);
	}
#line 4916 "util/configparser.c"
    break;

  case 456:
#line 1758 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_ttl_reset:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->serve_expired_ttl_reset = (yyvsp[0].uval);
	}
#line 4925 "util/configparser.c"
    break;

  case 457:
#line 1764 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_reply_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->serve_expired_reply_ttl = (yyvsp[0].lval);
	}
#line 4934 "util/configparser.c"
    break;

  case 458:
#line 1770 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_client_timeout:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->serve_expired_client_timeout = (yyvsp[0].lval);
	}
#line 4943 "util/configparser.c"
    break;

  case 459:
#line 1776 "./util/configparser.y"
        {
		OUTYY(("P(server_serve_original_ttl:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->serve_original_ttl = (yyvsp[0].uval);
	}
#line 4952 "util/configparser.c"
    break;

  case 460:
#line 1782 "./util/configparser.y"
        {
		OUTYY(("P(server_fake_dsa:%u)\n", (yyvsp[0].uval)));
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		if ((yyvsp[0].uval)) {
			log_warn("test option fake_dsa is enabled");
		}
#endif
	}
#line 4965 "util/configparser.c"
    break;

  case 461:
#line 1792 "./util/configparser.y"
        {
		OUTYY(("P(server_fake_sha1:%u)\n", (yyvsp[0].uval)));
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		if ((yyvsp[0].uval)) {
			log_warn("test option fake_sha1 is enabled");
		}
#endif
	}
#line 4978 "util/configparser.c"
    break;

  case 462:
#line 1802 "./util/configparser.y"
        {
		OUTYY(("P(server_val_log_level:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->val_log_level = (yyvsp[0].lval);
	}
#line 4987 "util/configparser.c"
    break;

  case 463:
#line 1808 "./util/configparser.y"
        {
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = (yyvsp[0].str);
	}
#line 4997 "util/configparser.c"
    break;

  case 464:
#line 1815 "./util/configparser.y"
        {
		OUTYY(("P(server_add_holddown:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->add_holddown = (yyvsp[0].lval);
	}
#line 5006 "util/configparser.c"
    break;

  case 465:
#line 1821 "./util/configparser.y"
        {
		OUTYY(("P(server_del_holddown:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->del_holddown = (yyvsp[0].lval);
	}
#line 5015 "util/configparser.c"
    break;

  case 466:
#line 1827 "./util/configparser.y"
        {
		OUTYY(("P(server_keep_missing:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->keep_missing = (yyvsp[0].lval);
	}
#line 5024 "util/configparser.c"
    break;

  case 467:
#line 1833 "./util/configparser.y"
        {
		OUTYY(("P(server_permit_small_holddown:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->permit_small_holddown = (yyvsp[0].uval);
	}
#line 5033 "util/configparser.c"
    break;

  case 468:
#line 1838 "./util/configparser.y"
        {
		OUTYY(("P(server_key_cache_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->key_cache_size = (yyvsp[0].sval);
	}
#line 5042 "util/configparser.c"
    break;

  case 469:
#line 1844 "./util/configparser.y"
        {
		OUTYY(("P(server_key_cache_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->key_cache_slabs = (yyvsp[0].lval);
	}
#line 5051 "util/configparser.c"
    break;

  case 470:
#line 1850 "./util/configparser.y"
        {
		OUTYY(("P(server_neg_cache_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->neg_cache_size = (yyvsp[0].sval);
	}
#line 5060 "util/configparser.c"
    break;

  case 471:
#line 1856 "./util/configparser.y"
        {
		OUTYY(("P(server_local_zone:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "static")!=0 && strcmp((yyvsp[0].str), "deny")!=0 &&
		   strcmp((yyvsp[0].str), "refuse")!=0 && strcmp((yyvsp[0].str), "redirect")!=0 &&
		   strcmp((yyvsp[0].str), "transparent")!=0 && strcmp((yyvsp[0].str), "nodefault")!=0
		   && strcmp((yyvsp[0].str), "typetransparent")!=0
		   && strcmp((yyvsp[0].str), "always_transparent")!=0
		   && strcmp((yyvsp[0].str), "always_refuse")!=0
		   && strcmp((yyvsp[0].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[0].str), "always_nodata")!=0
		   && strcmp((yyvsp[0].str), "always_deny")!=0
		   && strcmp((yyvsp[0].str), "always_null")!=0
		   && strcmp((yyvsp[0].str), "noview")!=0
		   && strcmp((yyvsp[0].str), "inform")!=0 && strcmp((yyvsp[0].str), "inform_deny")!=0
		   && strcmp((yyvsp[0].str), "inform_redirect") != 0
			 && strcmp((yyvsp[0].str), "ipset") != 0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"inform_redirect, always_transparent, "
				"always_refuse, always_nxdomain, "
				"always_nodata, always_deny, always_null, "
				"noview, nodefault or ipset");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else if(strcmp((yyvsp[0].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
#ifdef USE_IPSET
		} else if(strcmp((yyvsp[0].str), "ipset")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_ipset, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
#endif
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones,
				(yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
#line 5108 "util/configparser.c"
    break;

  case 472:
#line 1901 "./util/configparser.y"
        {
		OUTYY(("P(server_local_data:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, (yyvsp[0].str)))
			fatal_exit("out of memory adding local-data");
	}
#line 5118 "util/configparser.c"
    break;

  case 473:
#line 1908 "./util/configparser.y"
        {
		char* ptr;
		OUTYY(("P(server_local_data_ptr:%s)\n", (yyvsp[0].str)));
		ptr = cfg_ptr_reverse((yyvsp[0].str));
		free((yyvsp[0].str));
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
#line 5136 "util/configparser.c"
    break;

  case 474:
#line 1923 "./util/configparser.y"
        {
		OUTYY(("P(server_minimal_responses:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->minimal_responses = (yyvsp[0].uval);
	}
#line 5145 "util/configparser.c"
    break;

  case 475:
#line 1929 "./util/configparser.y"
        {
		OUTYY(("P(server_rrset_roundrobin:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->rrset_roundrobin = (yyvsp[0].uval);
	}
#line 5154 "util/configparser.c"
    break;

  case 476:
#line 1935 "./util/configparser.y"
        {
		OUTYY(("P(server_unknown_server_time_limit:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->unknown_server_time_limit = (yyvsp[0].lval);
	}
#line 5163 "util/configparser.c"
    break;

  case 477:
#line 1941 "./util/configparser.y"
        {
		OUTYY(("P(server_max_udp_size:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->max_udp_size = (yyvsp[0].lval);
	}
#line 5172 "util/configparser.c"
    break;

  case 478:
#line 1947 "./util/configparser.y"
        {
		OUTYY(("P(dns64_prefix:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dns64_prefix);
		cfg_parser->cfg->dns64_prefix = (yyvsp[0].str);
	}
#line 5182 "util/configparser.c"
    break;

  case 479:
#line 1954 "./util/configparser.y"
        {
		OUTYY(("P(server_dns64_synthall:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dns64_synthall = (yyvsp[0].uval);
	}
#line 5191 "util/configparser.c"
    break;

  case 480:
#line 1960 "./util/configparser.y"
        {
		OUTYY(("P(dns64_ignore_aaaa:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dns64_ignore_aaaa,
			(yyvsp[0].str)))
			fatal_exit("out of memory adding dns64-ignore-aaaa");
	}
#line 5202 "util/configparser.c"
    break;

  case 481:
#line 1968 "./util/configparser.y"
        {
		char* p, *s = (yyvsp[0].str);
		OUTYY(("P(server_define_tag:%s)\n", (yyvsp[0].str)));
		while((p=strsep(&s, " \t\n")) != NULL) {
			if(*p) {
				if(!config_add_tag(cfg_parser->cfg, p))
					yyerror("could not define-tag, "
						"out of memory");
			}
		}
		free((yyvsp[0].str));
	}
#line 5219 "util/configparser.c"
    break;

  case 482:
#line 1982 "./util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->local_zone_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 5243 "util/configparser.c"
    break;

  case 483:
#line 2003 "./util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_access_control_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->acl_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 5267 "util/configparser.c"
    break;

  case 484:
#line 2024 "./util/configparser.y"
        {
		OUTYY(("P(server_access_control_tag_action:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_actions,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 5282 "util/configparser.c"
    break;

  case 485:
#line 2036 "./util/configparser.y"
        {
		OUTYY(("P(server_access_control_tag_data:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_datas,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 5297 "util/configparser.c"
    break;

  case 486:
#line 2048 "./util/configparser.y"
        {
		OUTYY(("P(server_local_zone_override:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->local_zone_overrides,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 5312 "util/configparser.c"
    break;

  case 487:
#line 2060 "./util/configparser.y"
        {
		OUTYY(("P(server_access_control_view:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acl_view,
			(yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
		}
	}
#line 5324 "util/configparser.c"
    break;

  case 488:
#line 2069 "./util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(response_ip_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->respip_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 5348 "util/configparser.c"
    break;

  case 489:
#line 2090 "./util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ip_ratelimit = (yyvsp[0].lval);
	}
#line 5357 "util/configparser.c"
    break;

  case 490:
#line 2097 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ratelimit = (yyvsp[0].lval);
	}
#line 5366 "util/configparser.c"
    break;

  case 491:
#line 2103 "./util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->ip_ratelimit_size = (yyvsp[0].sval);
	}
#line 5375 "util/configparser.c"
    break;

  case 492:
#line 2109 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_size:%llu)\n", (yyvsp[0].sval)));
		cfg_parser->cfg->ratelimit_size = (yyvsp[0].sval);
	}
#line 5384 "util/configparser.c"
    break;

  case 493:
#line 2115 "./util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ip_ratelimit_slabs = (yyvsp[0].lval);
	}
#line 5393 "util/configparser.c"
    break;

  case 494:
#line 2121 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ratelimit_slabs = (yyvsp[0].lval);
	}
#line 5402 "util/configparser.c"
    break;

  case 495:
#line 2127 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_for_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_for_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-for-domain");
		}
	}
#line 5420 "util/configparser.c"
    break;

  case 496:
#line 2142 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_below_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_below_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-below-domain");
		}
	}
#line 5438 "util/configparser.c"
    break;

  case 497:
#line 2157 "./util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_factor:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ip_ratelimit_factor = (yyvsp[0].lval);
	}
#line 5447 "util/configparser.c"
    break;

  case 498:
#line 2163 "./util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_factor:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ratelimit_factor = (yyvsp[0].lval);
	}
#line 5456 "util/configparser.c"
    break;

  case 499:
#line 2169 "./util/configparser.y"
        {
		OUTYY(("P(low-rtt option is deprecated, use fast-server-num instead)\n"));
		free((yyvsp[0].str));
	}
#line 5465 "util/configparser.c"
    break;

  case 500:
#line 2175 "./util/configparser.y"
        {
		OUTYY(("P(server_fast_server_num:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->fast_server_num = (yyvsp[0].lval);
	}
#line 5474 "util/configparser.c"
    break;

  case 501:
#line 2181 "./util/configparser.y"
        {
		OUTYY(("P(server_fast_server_permil:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->fast_server_permil = (yyvsp[0].lval);
	}
#line 5483 "util/configparser.c"
    break;

  case 502:
#line 2187 "./util/configparser.y"
        {
		OUTYY(("P(server_qname_minimisation:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->qname_minimisation = (yyvsp[0].uval);
	}
#line 5492 "util/configparser.c"
    break;

  case 503:
#line 2193 "./util/configparser.y"
        {
		OUTYY(("P(server_qname_minimisation_strict:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->qname_minimisation_strict = (yyvsp[0].uval);
	}
#line 5501 "util/configparser.c"
    break;

  case 504:
#line 2199 "./util/configparser.y"
        {
		OUTYY(("P(server_pad_responses:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->pad_responses = (yyvsp[0].uval);
	}
#line 5510 "util/configparser.c"
    break;

  case 505:
#line 2205 "./util/configparser.y"
        {
		OUTYY(("P(server_pad_responses_block_size:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->pad_responses_block_size = (yyvsp[0].lval);
	}
#line 5519 "util/configparser.c"
    break;

  case 506:
#line 2211 "./util/configparser.y"
        {
		OUTYY(("P(server_pad_queries:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->pad_queries = (yyvsp[0].uval);
	}
#line 5528 "util/configparser.c"
    break;

  case 507:
#line 2217 "./util/configparser.y"
        {
		OUTYY(("P(server_pad_queries_block_size:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->pad_queries_block_size = (yyvsp[0].lval);
	}
#line 5537 "util/configparser.c"
    break;

  case 508:
#line 2223 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_enabled:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->ipsecmod_enabled = (yyvsp[0].uval);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 5550 "util/configparser.c"
    break;

  case 509:
#line 2233 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_ignore_bogus:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->ipsecmod_ignore_bogus = (yyvsp[0].uval);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 5563 "util/configparser.c"
    break;

  case 510:
#line 2243 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_hook:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ipsecmod_hook);
		cfg_parser->cfg->ipsecmod_hook = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5578 "util/configparser.c"
    break;

  case 511:
#line 2255 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_max_ttl:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->ipsecmod_max_ttl = (yyvsp[0].lval);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 5591 "util/configparser.c"
    break;

  case 512:
#line 2265 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->ipsecmod_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5606 "util/configparser.c"
    break;

  case 513:
#line 2277 "./util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_strict:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->ipsecmod_strict = (yyvsp[0].uval);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 5619 "util/configparser.c"
    break;

  case 514:
#line 2287 "./util/configparser.y"
        {
		OUTYY(("P(server_edns_client_string:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->edns_client_strings, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding "
				"edns-client-string");
	}
#line 5631 "util/configparser.c"
    break;

  case 515:
#line 2296 "./util/configparser.y"
        {
		OUTYY(("P(edns_client_string_opcode:%s)\n", (yyvsp[0].lval)));
		if((yyvsp[0].lval) > 65535)
			yyerror("option code must be in interval [0, 65535]");
		else
			cfg_parser->cfg->edns_client_string_opcode = (yyvsp[0].lval);
	}
#line 5643 "util/configparser.c"
    break;

  case 516:
#line 2305 "./util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = (yyvsp[0].str);
	}
#line 5656 "util/configparser.c"
    break;

  case 517:
#line 2315 "./util/configparser.y"
        {
		OUTYY(("P(stub-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5666 "util/configparser.c"
    break;

  case 518:
#line 2322 "./util/configparser.y"
        {
		OUTYY(("P(stub-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5676 "util/configparser.c"
    break;

  case 519:
#line 2329 "./util/configparser.y"
        {
		OUTYY(("P(stub-first:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stubs->isfirst = (yyvsp[0].uval);
	}
#line 5685 "util/configparser.c"
    break;

  case 520:
#line 2335 "./util/configparser.y"
        {
		OUTYY(("P(stub-no-cache:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stubs->no_cache = (yyvsp[0].uval);
	}
#line 5694 "util/configparser.c"
    break;

  case 521:
#line 2341 "./util/configparser.y"
        {
		OUTYY(("P(stub-ssl-upstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stubs->ssl_upstream = (yyvsp[0].uval);
	}
#line 5703 "util/configparser.c"
    break;

  case 522:
#line 2347 "./util/configparser.y"
        {
		OUTYY(("P(stub-prime:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->stubs->isprime = (yyvsp[0].uval);
	}
#line 5712 "util/configparser.c"
    break;

  case 523:
#line 2353 "./util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = (yyvsp[0].str);
	}
#line 5725 "util/configparser.c"
    break;

  case 524:
#line 2363 "./util/configparser.y"
        {
		OUTYY(("P(forward-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5735 "util/configparser.c"
    break;

  case 525:
#line 2370 "./util/configparser.y"
        {
		OUTYY(("P(forward-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5745 "util/configparser.c"
    break;

  case 526:
#line 2377 "./util/configparser.y"
        {
		OUTYY(("P(forward-first:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->forwards->isfirst = (yyvsp[0].uval);
	}
#line 5754 "util/configparser.c"
    break;

  case 527:
#line 2383 "./util/configparser.y"
        {
		OUTYY(("P(forward-no-cache:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->forwards->no_cache = (yyvsp[0].uval);
	}
#line 5763 "util/configparser.c"
    break;

  case 528:
#line 2389 "./util/configparser.y"
        {
		OUTYY(("P(forward-ssl-upstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->forwards->ssl_upstream = (yyvsp[0].uval);
	}
#line 5772 "util/configparser.c"
    break;

  case 529:
#line 2395 "./util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = (yyvsp[0].str);
	}
#line 5785 "util/configparser.c"
    break;

  case 530:
#line 2405 "./util/configparser.y"
        {
		OUTYY(("P(zonefile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = (yyvsp[0].str);
	}
#line 5795 "util/configparser.c"
    break;

  case 531:
#line 2412 "./util/configparser.y"
        {
		OUTYY(("P(master:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5805 "util/configparser.c"
    break;

  case 532:
#line 2419 "./util/configparser.y"
        {
		OUTYY(("P(url:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5815 "util/configparser.c"
    break;

  case 533:
#line 2426 "./util/configparser.y"
        {
		OUTYY(("P(allow-notify:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5826 "util/configparser.c"
    break;

  case 534:
#line 2434 "./util/configparser.y"
        {
		OUTYY(("P(for-downstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->auths->for_downstream = (yyvsp[0].uval);
	}
#line 5835 "util/configparser.c"
    break;

  case 535:
#line 2440 "./util/configparser.y"
        {
		OUTYY(("P(for-upstream:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->auths->for_upstream = (yyvsp[0].uval);
	}
#line 5844 "util/configparser.c"
    break;

  case 536:
#line 2446 "./util/configparser.y"
        {
		OUTYY(("P(fallback-enabled:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->auths->fallback_enabled = (yyvsp[0].uval);
	}
#line 5853 "util/configparser.c"
    break;

  case 537:
#line 2452 "./util/configparser.y"
        {
		OUTYY(("\nP(remote-control:)\n"));
	}
#line 5861 "util/configparser.c"
    break;

  case 548:
#line 2463 "./util/configparser.y"
        {
		OUTYY(("P(control_enable:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->remote_control_enable = (yyvsp[0].uval);
	}
#line 5870 "util/configparser.c"
    break;

  case 549:
#line 2469 "./util/configparser.y"
        {
		OUTYY(("P(control_port:%lu)\n", (yyvsp[0].lval)));
		if((yyvsp[0].lval) == 0)
			yyerror("control port number expected");
		else
			cfg_parser->cfg->control_port = (yyvsp[0].lval);
	}
#line 5882 "util/configparser.c"
    break;

  case 550:
#line 2478 "./util/configparser.y"
        {
		OUTYY(("P(control_interface:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5892 "util/configparser.c"
    break;

  case 551:
#line 2485 "./util/configparser.y"
        {
		OUTYY(("P(control_use_cert:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->control_use_cert = (yyvsp[0].uval);
	}
#line 5901 "util/configparser.c"
    break;

  case 552:
#line 2491 "./util/configparser.y"
        {
		OUTYY(("P(rc_server_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = (yyvsp[0].str);
	}
#line 5911 "util/configparser.c"
    break;

  case 553:
#line 2498 "./util/configparser.y"
        {
		OUTYY(("P(rc_server_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = (yyvsp[0].str);
	}
#line 5921 "util/configparser.c"
    break;

  case 554:
#line 2505 "./util/configparser.y"
        {
		OUTYY(("P(rc_control_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = (yyvsp[0].str);
	}
#line 5931 "util/configparser.c"
    break;

  case 555:
#line 2512 "./util/configparser.y"
        {
		OUTYY(("P(rc_control_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = (yyvsp[0].str);
	}
#line 5941 "util/configparser.c"
    break;

  case 556:
#line 2519 "./util/configparser.y"
        {
		OUTYY(("\nP(dnstap:)\n"));
	}
#line 5949 "util/configparser.c"
    break;

  case 578:
#line 2539 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_enable:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap = (yyvsp[0].uval);
	}
#line 5958 "util/configparser.c"
    break;

  case 579:
#line 2545 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_bidirectional:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_bidirectional = (yyvsp[0].uval);
	}
#line 5967 "util/configparser.c"
    break;

  case 580:
#line 2551 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_socket_path:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_socket_path);
		cfg_parser->cfg->dnstap_socket_path = (yyvsp[0].str);
	}
#line 5977 "util/configparser.c"
    break;

  case 581:
#line 2558 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_ip:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_ip);
		cfg_parser->cfg->dnstap_ip = (yyvsp[0].str);
	}
#line 5987 "util/configparser.c"
    break;

  case 582:
#line 2565 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_tls:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_tls = (yyvsp[0].uval);
	}
#line 5996 "util/configparser.c"
    break;

  case 583:
#line 2571 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_tls_server_name:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_tls_server_name);
		cfg_parser->cfg->dnstap_tls_server_name = (yyvsp[0].str);
	}
#line 6006 "util/configparser.c"
    break;

  case 584:
#line 2578 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_tls_cert_bundle:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_tls_cert_bundle);
		cfg_parser->cfg->dnstap_tls_cert_bundle = (yyvsp[0].str);
	}
#line 6016 "util/configparser.c"
    break;

  case 585:
#line 2585 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_tls_client_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_tls_client_key_file);
		cfg_parser->cfg->dnstap_tls_client_key_file = (yyvsp[0].str);
	}
#line 6026 "util/configparser.c"
    break;

  case 586:
#line 2592 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_tls_client_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_tls_client_cert_file);
		cfg_parser->cfg->dnstap_tls_client_cert_file = (yyvsp[0].str);
	}
#line 6036 "util/configparser.c"
    break;

  case 587:
#line 2599 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_send_identity:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_send_identity = (yyvsp[0].uval);
	}
#line 6045 "util/configparser.c"
    break;

  case 588:
#line 2605 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_send_version:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_send_version = (yyvsp[0].uval);
	}
#line 6054 "util/configparser.c"
    break;

  case 589:
#line 2611 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_identity);
		cfg_parser->cfg->dnstap_identity = (yyvsp[0].str);
	}
#line 6064 "util/configparser.c"
    break;

  case 590:
#line 2618 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_version:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_version);
		cfg_parser->cfg->dnstap_version = (yyvsp[0].str);
	}
#line 6074 "util/configparser.c"
    break;

  case 591:
#line 2626 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_resolver_query_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_resolver_query_messages = (yyvsp[0].uval);
	}
#line 6083 "util/configparser.c"
    break;

  case 592:
#line 2633 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_resolver_response_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_resolver_response_messages = (yyvsp[0].uval);
	}
#line 6092 "util/configparser.c"
    break;

  case 593:
#line 2640 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_client_query_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_client_query_messages = (yyvsp[0].uval);
	}
#line 6101 "util/configparser.c"
    break;

  case 594:
#line 2647 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_client_response_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_client_response_messages = (yyvsp[0].uval);
	}
#line 6110 "util/configparser.c"
    break;

  case 595:
#line 2654 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_forwarder_query_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_forwarder_query_messages = (yyvsp[0].uval);
	}
#line 6119 "util/configparser.c"
    break;

  case 596:
#line 2661 "./util/configparser.y"
        {
		OUTYY(("P(dt_dnstap_log_forwarder_response_messages:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnstap_log_forwarder_response_messages = (yyvsp[0].uval);
	}
#line 6128 "util/configparser.c"
    break;

  case 597:
#line 2667 "./util/configparser.y"
        {
		OUTYY(("\nP(python:)\n"));
	}
#line 6136 "util/configparser.c"
    break;

  case 601:
#line 2676 "./util/configparser.y"
        {
		OUTYY(("P(python-script:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->python_script, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 6146 "util/configparser.c"
    break;

  case 602:
#line 2682 "./util/configparser.y"
        {
		OUTYY(("\nP(dynlib:)\n"));
	}
#line 6154 "util/configparser.c"
    break;

  case 606:
#line 2691 "./util/configparser.y"
        {
		OUTYY(("P(dynlib-file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->dynlib_file, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 6164 "util/configparser.c"
    break;

  case 607:
#line 2697 "./util/configparser.y"
        {
		OUTYY(("P(disable_dnssec_lame_check:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->disable_dnssec_lame_check = (yyvsp[0].uval);
	}
#line 6173 "util/configparser.c"
    break;

  case 608:
#line 2703 "./util/configparser.y"
        {
		OUTYY(("P(server_log_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = (yyvsp[0].str);
	}
#line 6183 "util/configparser.c"
    break;

  case 609:
#line 2710 "./util/configparser.y"
        {
		OUTYY(("P(server_response_ip:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		validate_respip_action((yyvsp[0].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_actions,
			(yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding response-ip");
	}
#line 6195 "util/configparser.c"
    break;

  case 610:
#line 2719 "./util/configparser.y"
        {
		OUTYY(("P(server_response_ip_data:%s)\n", (yyvsp[-1].str)));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_data,
			(yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding response-ip-data");
	}
#line 6206 "util/configparser.c"
    break;

  case 611:
#line 2727 "./util/configparser.y"
        {
		OUTYY(("\nP(dnscrypt:)\n"));
	}
#line 6214 "util/configparser.c"
    break;

  case 624:
#line 2743 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_enable:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->dnscrypt = (yyvsp[0].uval);
	}
#line 6223 "util/configparser.c"
    break;

  case 625:
#line 2750 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_port:%lu)\n", (yyvsp[0].lval)));
		if((yyvsp[0].lval) == 0)
			yyerror("port number expected");
		else
			cfg_parser->cfg->dnscrypt_port = (yyvsp[0].lval);
	}
#line 6235 "util/configparser.c"
    break;

  case 626:
#line 2759 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_provider:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnscrypt_provider);
		cfg_parser->cfg->dnscrypt_provider = (yyvsp[0].str);
	}
#line 6245 "util/configparser.c"
    break;

  case 627:
#line 2766 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_provider_cert:%s)\n", (yyvsp[0].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[0].str)))
			log_warn("dnscrypt-provider-cert %s is a duplicate", (yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert");
	}
#line 6257 "util/configparser.c"
    break;

  case 628:
#line 2775 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_provider_cert_rotated:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert_rotated, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert-rotated");
	}
#line 6267 "util/configparser.c"
    break;

  case 629:
#line 2782 "./util/configparser.y"
        {
		OUTYY(("P(dnsc_dnscrypt_secret_key:%s)\n", (yyvsp[0].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[0].str)))
			log_warn("dnscrypt-secret-key: %s is a duplicate", (yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-secret-key");
	}
#line 6279 "util/configparser.c"
    break;

  case 630:
#line 2791 "./util/configparser.y"
  {
  	OUTYY(("P(dnscrypt_shared_secret_cache_size:%llu)\n", (yyvsp[0].sval)));
  	cfg_parser->cfg->dnscrypt_shared_secret_cache_size = (yyvsp[0].sval);
  }
#line 6288 "util/configparser.c"
    break;

  case 631:
#line 2797 "./util/configparser.y"
        {
		OUTYY(("P(dnscrypt_shared_secret_cache_slabs:%lu)\n", (yyvsp[0].lval)));
		cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs = (yyvsp[0].lval);
	}
#line 6297 "util/configparser.c"
    break;

  case 632:
#line 2803 "./util/configparser.y"
  {
  	OUTYY(("P(dnscrypt_nonce_cache_size:%llu)\n", (yyvsp[0].sval)));
  	cfg_parser->cfg->dnscrypt_nonce_cache_size = (yyvsp[0].sval);
  }
#line 6306 "util/configparser.c"
    break;

  case 633:
#line 2809 "./util/configparser.y"
  {
  	OUTYY(("P(dnscrypt_nonce_cache_slabs:%lu)\n", (yyvsp[0].lval)));
	cfg_parser->cfg->dnscrypt_nonce_cache_slabs = (yyvsp[0].lval);
  }
#line 6315 "util/configparser.c"
    break;

  case 634:
#line 2815 "./util/configparser.y"
        {
		OUTYY(("\nP(cachedb:)\n"));
	}
#line 6323 "util/configparser.c"
    break;

  case 643:
#line 2826 "./util/configparser.y"
        {
	#ifdef USE_CACHEDB
		OUTYY(("P(backend:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->cachedb_backend);
		cfg_parser->cfg->cachedb_backend = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 6338 "util/configparser.c"
    break;

  case 644:
#line 2838 "./util/configparser.y"
        {
	#ifdef USE_CACHEDB
		OUTYY(("P(secret-seed:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->cachedb_secret);
		cfg_parser->cfg->cachedb_secret = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 6353 "util/configparser.c"
    break;

  case 645:
#line 2850 "./util/configparser.y"
        {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_server_host:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->redis_server_host);
		cfg_parser->cfg->redis_server_host = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 6368 "util/configparser.c"
    break;

  case 646:
#line 2862 "./util/configparser.y"
        {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_server_port:%lu)\n", (yyvsp[0].lval)));

		if((yyvsp[0].lval) == 0 || (yyvsp[0].lval) > 65535)
			yyerror("valid redis server port number expected");
		else
			cfg_parser->cfg->redis_server_port = (yyvsp[0].lval);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
	}
#line 6385 "util/configparser.c"
    break;

  case 647:
#line 2876 "./util/configparser.y"
        {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_timeout:%s)\n", (yyvsp[0].lval)));
		if((yyvsp[0].lval) == 0)
			yyerror("redis timeout value expected");
		else
			cfg_parser->cfg->redis_timeout = (yyvsp[0].lval);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
	}
#line 6401 "util/configparser.c"
    break;

  case 648:
#line 2889 "./util/configparser.y"
        {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_expire_records:%u)\n", (yyvsp[0].uval)));
		cfg_parser->cfg->redis_expire_records = (yyvsp[0].uval);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
	}
#line 6414 "util/configparser.c"
    break;

  case 649:
#line 2899 "./util/configparser.y"
        {
		OUTYY(("P(server_tcp_connection_limit:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if (atoi((yyvsp[0].str)) < 0)
			yyerror("positive number expected");
		else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->tcp_connection_limits, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding tcp connection limit");
		}
	}
#line 6428 "util/configparser.c"
    break;

  case 650:
#line 2910 "./util/configparser.y"
                {
			OUTYY(("\nP(ipset:)\n"));
		}
#line 6436 "util/configparser.c"
    break;

  case 655:
#line 2919 "./util/configparser.y"
                {
		#ifdef USE_IPSET
			OUTYY(("P(name-v4:%s)\n", (yyvsp[0].str)));
			if(cfg_parser->cfg->ipset_name_v4)
				yyerror("ipset name v4 override, there must be one "
					"name for ip v4");
			free(cfg_parser->cfg->ipset_name_v4);
			cfg_parser->cfg->ipset_name_v4 = (yyvsp[0].str);
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free((yyvsp[0].str));
		#endif
		}
#line 6454 "util/configparser.c"
    break;

  case 656:
#line 2934 "./util/configparser.y"
        {
		#ifdef USE_IPSET
			OUTYY(("P(name-v6:%s)\n", (yyvsp[0].str)));
			if(cfg_parser->cfg->ipset_name_v6)
				yyerror("ipset name v6 override, there must be one "
					"name for ip v6");
			free(cfg_parser->cfg->ipset_name_v6);
			cfg_parser->cfg->ipset_name_v6 = (yyvsp[0].str);
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free((yyvsp[0].str));
		#endif
		}
#line 6472 "util/configparser.c"
    break;


#line 6476 "util/configparser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 2948 "./util/configparser.y"


/* parse helper routines could be here */
static void
validate_respip_action(const char* action)
{
	if(strcmp(action, "deny")!=0 &&
		strcmp(action, "redirect")!=0 &&
		strcmp(action, "inform")!=0 &&
		strcmp(action, "inform_deny")!=0 &&
		strcmp(action, "always_transparent")!=0 &&
		strcmp(action, "always_refuse")!=0 &&
		strcmp(action, "always_nxdomain")!=0)
	{
		yyerror("response-ip action: expected deny, redirect, "
			"inform, inform_deny, always_transparent, "
			"always_refuse or always_nxdomain");
	}
}


