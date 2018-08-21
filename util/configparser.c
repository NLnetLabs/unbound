/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 38 "./util/configparser.y" /* yacc.c:339  */

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


#line 95 "util/configparser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "configparser.h".  */
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
    VAR_SERVER = 266,
    VAR_VERBOSITY = 267,
    VAR_NUM_THREADS = 268,
    VAR_PORT = 269,
    VAR_OUTGOING_RANGE = 270,
    VAR_INTERFACE = 271,
    VAR_DO_IP4 = 272,
    VAR_DO_IP6 = 273,
    VAR_PREFER_IP6 = 274,
    VAR_DO_UDP = 275,
    VAR_DO_TCP = 276,
    VAR_TCP_MSS = 277,
    VAR_OUTGOING_TCP_MSS = 278,
    VAR_TCP_IDLE_TIMEOUT = 279,
    VAR_EDNS_TCP_KEEPALIVE = 280,
    VAR_EDNS_TCP_KEEPALIVE_TIMEOUT = 281,
    VAR_CHROOT = 282,
    VAR_USERNAME = 283,
    VAR_DIRECTORY = 284,
    VAR_LOGFILE = 285,
    VAR_PIDFILE = 286,
    VAR_MSG_CACHE_SIZE = 287,
    VAR_MSG_CACHE_SLABS = 288,
    VAR_NUM_QUERIES_PER_THREAD = 289,
    VAR_RRSET_CACHE_SIZE = 290,
    VAR_RRSET_CACHE_SLABS = 291,
    VAR_OUTGOING_NUM_TCP = 292,
    VAR_INFRA_HOST_TTL = 293,
    VAR_INFRA_LAME_TTL = 294,
    VAR_INFRA_CACHE_SLABS = 295,
    VAR_INFRA_CACHE_NUMHOSTS = 296,
    VAR_INFRA_CACHE_LAME_SIZE = 297,
    VAR_NAME = 298,
    VAR_STUB_ZONE = 299,
    VAR_STUB_HOST = 300,
    VAR_STUB_ADDR = 301,
    VAR_TARGET_FETCH_POLICY = 302,
    VAR_HARDEN_SHORT_BUFSIZE = 303,
    VAR_HARDEN_LARGE_QUERIES = 304,
    VAR_FORWARD_ZONE = 305,
    VAR_FORWARD_HOST = 306,
    VAR_FORWARD_ADDR = 307,
    VAR_DO_NOT_QUERY_ADDRESS = 308,
    VAR_HIDE_IDENTITY = 309,
    VAR_HIDE_VERSION = 310,
    VAR_IDENTITY = 311,
    VAR_VERSION = 312,
    VAR_HARDEN_GLUE = 313,
    VAR_MODULE_CONF = 314,
    VAR_TRUST_ANCHOR_FILE = 315,
    VAR_TRUST_ANCHOR = 316,
    VAR_VAL_OVERRIDE_DATE = 317,
    VAR_BOGUS_TTL = 318,
    VAR_VAL_CLEAN_ADDITIONAL = 319,
    VAR_VAL_PERMISSIVE_MODE = 320,
    VAR_INCOMING_NUM_TCP = 321,
    VAR_MSG_BUFFER_SIZE = 322,
    VAR_KEY_CACHE_SIZE = 323,
    VAR_KEY_CACHE_SLABS = 324,
    VAR_TRUSTED_KEYS_FILE = 325,
    VAR_VAL_NSEC3_KEYSIZE_ITERATIONS = 326,
    VAR_USE_SYSLOG = 327,
    VAR_OUTGOING_INTERFACE = 328,
    VAR_ROOT_HINTS = 329,
    VAR_DO_NOT_QUERY_LOCALHOST = 330,
    VAR_CACHE_MAX_TTL = 331,
    VAR_HARDEN_DNSSEC_STRIPPED = 332,
    VAR_ACCESS_CONTROL = 333,
    VAR_LOCAL_ZONE = 334,
    VAR_LOCAL_DATA = 335,
    VAR_INTERFACE_AUTOMATIC = 336,
    VAR_STATISTICS_INTERVAL = 337,
    VAR_DO_DAEMONIZE = 338,
    VAR_USE_CAPS_FOR_ID = 339,
    VAR_STATISTICS_CUMULATIVE = 340,
    VAR_OUTGOING_PORT_PERMIT = 341,
    VAR_OUTGOING_PORT_AVOID = 342,
    VAR_DLV_ANCHOR_FILE = 343,
    VAR_DLV_ANCHOR = 344,
    VAR_NEG_CACHE_SIZE = 345,
    VAR_HARDEN_REFERRAL_PATH = 346,
    VAR_PRIVATE_ADDRESS = 347,
    VAR_PRIVATE_DOMAIN = 348,
    VAR_REMOTE_CONTROL = 349,
    VAR_CONTROL_ENABLE = 350,
    VAR_CONTROL_INTERFACE = 351,
    VAR_CONTROL_PORT = 352,
    VAR_SERVER_KEY_FILE = 353,
    VAR_SERVER_CERT_FILE = 354,
    VAR_CONTROL_KEY_FILE = 355,
    VAR_CONTROL_CERT_FILE = 356,
    VAR_CONTROL_USE_CERT = 357,
    VAR_EXTENDED_STATISTICS = 358,
    VAR_LOCAL_DATA_PTR = 359,
    VAR_JOSTLE_TIMEOUT = 360,
    VAR_STUB_PRIME = 361,
    VAR_UNWANTED_REPLY_THRESHOLD = 362,
    VAR_LOG_TIME_ASCII = 363,
    VAR_DOMAIN_INSECURE = 364,
    VAR_PYTHON = 365,
    VAR_PYTHON_SCRIPT = 366,
    VAR_VAL_SIG_SKEW_MIN = 367,
    VAR_VAL_SIG_SKEW_MAX = 368,
    VAR_CACHE_MIN_TTL = 369,
    VAR_VAL_LOG_LEVEL = 370,
    VAR_AUTO_TRUST_ANCHOR_FILE = 371,
    VAR_KEEP_MISSING = 372,
    VAR_ADD_HOLDDOWN = 373,
    VAR_DEL_HOLDDOWN = 374,
    VAR_SO_RCVBUF = 375,
    VAR_EDNS_BUFFER_SIZE = 376,
    VAR_PREFETCH = 377,
    VAR_PREFETCH_KEY = 378,
    VAR_SO_SNDBUF = 379,
    VAR_SO_REUSEPORT = 380,
    VAR_HARDEN_BELOW_NXDOMAIN = 381,
    VAR_IGNORE_CD_FLAG = 382,
    VAR_LOG_QUERIES = 383,
    VAR_LOG_REPLIES = 384,
    VAR_LOG_LOCAL_ACTIONS = 385,
    VAR_TCP_UPSTREAM = 386,
    VAR_SSL_UPSTREAM = 387,
    VAR_SSL_SERVICE_KEY = 388,
    VAR_SSL_SERVICE_PEM = 389,
    VAR_SSL_PORT = 390,
    VAR_FORWARD_FIRST = 391,
    VAR_STUB_SSL_UPSTREAM = 392,
    VAR_FORWARD_SSL_UPSTREAM = 393,
    VAR_TLS_CERT_BUNDLE = 394,
    VAR_STUB_FIRST = 395,
    VAR_MINIMAL_RESPONSES = 396,
    VAR_RRSET_ROUNDROBIN = 397,
    VAR_MAX_UDP_SIZE = 398,
    VAR_DELAY_CLOSE = 399,
    VAR_UNBLOCK_LAN_ZONES = 400,
    VAR_INSECURE_LAN_ZONES = 401,
    VAR_INFRA_CACHE_MIN_RTT = 402,
    VAR_DNS64_PREFIX = 403,
    VAR_DNS64_SYNTHALL = 404,
    VAR_DNS64_IGNORE_AAAA = 405,
    VAR_DNSTAP = 406,
    VAR_DNSTAP_ENABLE = 407,
    VAR_DNSTAP_SOCKET_PATH = 408,
    VAR_DNSTAP_SEND_IDENTITY = 409,
    VAR_DNSTAP_SEND_VERSION = 410,
    VAR_DNSTAP_IDENTITY = 411,
    VAR_DNSTAP_VERSION = 412,
    VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES = 413,
    VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES = 414,
    VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES = 415,
    VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES = 416,
    VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES = 417,
    VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES = 418,
    VAR_RESPONSE_IP_TAG = 419,
    VAR_RESPONSE_IP = 420,
    VAR_RESPONSE_IP_DATA = 421,
    VAR_HARDEN_ALGO_DOWNGRADE = 422,
    VAR_IP_TRANSPARENT = 423,
    VAR_DISABLE_DNSSEC_LAME_CHECK = 424,
    VAR_IP_RATELIMIT = 425,
    VAR_IP_RATELIMIT_SLABS = 426,
    VAR_IP_RATELIMIT_SIZE = 427,
    VAR_RATELIMIT = 428,
    VAR_RATELIMIT_SLABS = 429,
    VAR_RATELIMIT_SIZE = 430,
    VAR_RATELIMIT_FOR_DOMAIN = 431,
    VAR_RATELIMIT_BELOW_DOMAIN = 432,
    VAR_IP_RATELIMIT_FACTOR = 433,
    VAR_RATELIMIT_FACTOR = 434,
    VAR_SEND_CLIENT_SUBNET = 435,
    VAR_CLIENT_SUBNET_ZONE = 436,
    VAR_CLIENT_SUBNET_ALWAYS_FORWARD = 437,
    VAR_CLIENT_SUBNET_OPCODE = 438,
    VAR_MAX_CLIENT_SUBNET_IPV4 = 439,
    VAR_MAX_CLIENT_SUBNET_IPV6 = 440,
    VAR_CAPS_WHITELIST = 441,
    VAR_CACHE_MAX_NEGATIVE_TTL = 442,
    VAR_PERMIT_SMALL_HOLDDOWN = 443,
    VAR_QNAME_MINIMISATION = 444,
    VAR_QNAME_MINIMISATION_STRICT = 445,
    VAR_IP_FREEBIND = 446,
    VAR_DEFINE_TAG = 447,
    VAR_LOCAL_ZONE_TAG = 448,
    VAR_ACCESS_CONTROL_TAG = 449,
    VAR_LOCAL_ZONE_OVERRIDE = 450,
    VAR_ACCESS_CONTROL_TAG_ACTION = 451,
    VAR_ACCESS_CONTROL_TAG_DATA = 452,
    VAR_VIEW = 453,
    VAR_ACCESS_CONTROL_VIEW = 454,
    VAR_VIEW_FIRST = 455,
    VAR_SERVE_EXPIRED = 456,
    VAR_FAKE_DSA = 457,
    VAR_FAKE_SHA1 = 458,
    VAR_LOG_IDENTITY = 459,
    VAR_HIDE_TRUSTANCHOR = 460,
    VAR_TRUST_ANCHOR_SIGNALING = 461,
    VAR_AGGRESSIVE_NSEC = 462,
    VAR_USE_SYSTEMD = 463,
    VAR_SHM_ENABLE = 464,
    VAR_SHM_KEY = 465,
    VAR_ROOT_KEY_SENTINEL = 466,
    VAR_DNSCRYPT = 467,
    VAR_DNSCRYPT_ENABLE = 468,
    VAR_DNSCRYPT_PORT = 469,
    VAR_DNSCRYPT_PROVIDER = 470,
    VAR_DNSCRYPT_SECRET_KEY = 471,
    VAR_DNSCRYPT_PROVIDER_CERT = 472,
    VAR_DNSCRYPT_PROVIDER_CERT_ROTATED = 473,
    VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE = 474,
    VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS = 475,
    VAR_DNSCRYPT_NONCE_CACHE_SIZE = 476,
    VAR_DNSCRYPT_NONCE_CACHE_SLABS = 477,
    VAR_IPSECMOD_ENABLED = 478,
    VAR_IPSECMOD_HOOK = 479,
    VAR_IPSECMOD_IGNORE_BOGUS = 480,
    VAR_IPSECMOD_MAX_TTL = 481,
    VAR_IPSECMOD_WHITELIST = 482,
    VAR_IPSECMOD_STRICT = 483,
    VAR_CACHEDB = 484,
    VAR_CACHEDB_BACKEND = 485,
    VAR_CACHEDB_SECRETSEED = 486,
    VAR_CACHEDB_REDISHOST = 487,
    VAR_CACHEDB_REDISPORT = 488,
    VAR_CACHEDB_REDISTIMEOUT = 489,
    VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM = 490,
    VAR_FOR_UPSTREAM = 491,
    VAR_AUTH_ZONE = 492,
    VAR_ZONEFILE = 493,
    VAR_MASTER = 494,
    VAR_URL = 495,
    VAR_FOR_DOWNSTREAM = 496,
    VAR_FALLBACK_ENABLED = 497,
    VAR_TLS_ADDITIONAL_PORT = 498,
    VAR_LOW_RTT = 499,
    VAR_LOW_RTT_PERMIL = 500,
    VAR_ALLOW_NOTIFY = 501,
    VAR_TLS_WIN_CERT = 502,
    VAR_TCP_CONNECTION_LIMIT = 503,
    VAR_FORWARD_NO_CACHE = 504,
    VAR_STUB_NO_CACHE = 505,
    VAR_LOG_SERVFAIL = 506
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
#define VAR_SERVER 266
#define VAR_VERBOSITY 267
#define VAR_NUM_THREADS 268
#define VAR_PORT 269
#define VAR_OUTGOING_RANGE 270
#define VAR_INTERFACE 271
#define VAR_DO_IP4 272
#define VAR_DO_IP6 273
#define VAR_PREFER_IP6 274
#define VAR_DO_UDP 275
#define VAR_DO_TCP 276
#define VAR_TCP_MSS 277
#define VAR_OUTGOING_TCP_MSS 278
#define VAR_TCP_IDLE_TIMEOUT 279
#define VAR_EDNS_TCP_KEEPALIVE 280
#define VAR_EDNS_TCP_KEEPALIVE_TIMEOUT 281
#define VAR_CHROOT 282
#define VAR_USERNAME 283
#define VAR_DIRECTORY 284
#define VAR_LOGFILE 285
#define VAR_PIDFILE 286
#define VAR_MSG_CACHE_SIZE 287
#define VAR_MSG_CACHE_SLABS 288
#define VAR_NUM_QUERIES_PER_THREAD 289
#define VAR_RRSET_CACHE_SIZE 290
#define VAR_RRSET_CACHE_SLABS 291
#define VAR_OUTGOING_NUM_TCP 292
#define VAR_INFRA_HOST_TTL 293
#define VAR_INFRA_LAME_TTL 294
#define VAR_INFRA_CACHE_SLABS 295
#define VAR_INFRA_CACHE_NUMHOSTS 296
#define VAR_INFRA_CACHE_LAME_SIZE 297
#define VAR_NAME 298
#define VAR_STUB_ZONE 299
#define VAR_STUB_HOST 300
#define VAR_STUB_ADDR 301
#define VAR_TARGET_FETCH_POLICY 302
#define VAR_HARDEN_SHORT_BUFSIZE 303
#define VAR_HARDEN_LARGE_QUERIES 304
#define VAR_FORWARD_ZONE 305
#define VAR_FORWARD_HOST 306
#define VAR_FORWARD_ADDR 307
#define VAR_DO_NOT_QUERY_ADDRESS 308
#define VAR_HIDE_IDENTITY 309
#define VAR_HIDE_VERSION 310
#define VAR_IDENTITY 311
#define VAR_VERSION 312
#define VAR_HARDEN_GLUE 313
#define VAR_MODULE_CONF 314
#define VAR_TRUST_ANCHOR_FILE 315
#define VAR_TRUST_ANCHOR 316
#define VAR_VAL_OVERRIDE_DATE 317
#define VAR_BOGUS_TTL 318
#define VAR_VAL_CLEAN_ADDITIONAL 319
#define VAR_VAL_PERMISSIVE_MODE 320
#define VAR_INCOMING_NUM_TCP 321
#define VAR_MSG_BUFFER_SIZE 322
#define VAR_KEY_CACHE_SIZE 323
#define VAR_KEY_CACHE_SLABS 324
#define VAR_TRUSTED_KEYS_FILE 325
#define VAR_VAL_NSEC3_KEYSIZE_ITERATIONS 326
#define VAR_USE_SYSLOG 327
#define VAR_OUTGOING_INTERFACE 328
#define VAR_ROOT_HINTS 329
#define VAR_DO_NOT_QUERY_LOCALHOST 330
#define VAR_CACHE_MAX_TTL 331
#define VAR_HARDEN_DNSSEC_STRIPPED 332
#define VAR_ACCESS_CONTROL 333
#define VAR_LOCAL_ZONE 334
#define VAR_LOCAL_DATA 335
#define VAR_INTERFACE_AUTOMATIC 336
#define VAR_STATISTICS_INTERVAL 337
#define VAR_DO_DAEMONIZE 338
#define VAR_USE_CAPS_FOR_ID 339
#define VAR_STATISTICS_CUMULATIVE 340
#define VAR_OUTGOING_PORT_PERMIT 341
#define VAR_OUTGOING_PORT_AVOID 342
#define VAR_DLV_ANCHOR_FILE 343
#define VAR_DLV_ANCHOR 344
#define VAR_NEG_CACHE_SIZE 345
#define VAR_HARDEN_REFERRAL_PATH 346
#define VAR_PRIVATE_ADDRESS 347
#define VAR_PRIVATE_DOMAIN 348
#define VAR_REMOTE_CONTROL 349
#define VAR_CONTROL_ENABLE 350
#define VAR_CONTROL_INTERFACE 351
#define VAR_CONTROL_PORT 352
#define VAR_SERVER_KEY_FILE 353
#define VAR_SERVER_CERT_FILE 354
#define VAR_CONTROL_KEY_FILE 355
#define VAR_CONTROL_CERT_FILE 356
#define VAR_CONTROL_USE_CERT 357
#define VAR_EXTENDED_STATISTICS 358
#define VAR_LOCAL_DATA_PTR 359
#define VAR_JOSTLE_TIMEOUT 360
#define VAR_STUB_PRIME 361
#define VAR_UNWANTED_REPLY_THRESHOLD 362
#define VAR_LOG_TIME_ASCII 363
#define VAR_DOMAIN_INSECURE 364
#define VAR_PYTHON 365
#define VAR_PYTHON_SCRIPT 366
#define VAR_VAL_SIG_SKEW_MIN 367
#define VAR_VAL_SIG_SKEW_MAX 368
#define VAR_CACHE_MIN_TTL 369
#define VAR_VAL_LOG_LEVEL 370
#define VAR_AUTO_TRUST_ANCHOR_FILE 371
#define VAR_KEEP_MISSING 372
#define VAR_ADD_HOLDDOWN 373
#define VAR_DEL_HOLDDOWN 374
#define VAR_SO_RCVBUF 375
#define VAR_EDNS_BUFFER_SIZE 376
#define VAR_PREFETCH 377
#define VAR_PREFETCH_KEY 378
#define VAR_SO_SNDBUF 379
#define VAR_SO_REUSEPORT 380
#define VAR_HARDEN_BELOW_NXDOMAIN 381
#define VAR_IGNORE_CD_FLAG 382
#define VAR_LOG_QUERIES 383
#define VAR_LOG_REPLIES 384
#define VAR_LOG_LOCAL_ACTIONS 385
#define VAR_TCP_UPSTREAM 386
#define VAR_SSL_UPSTREAM 387
#define VAR_SSL_SERVICE_KEY 388
#define VAR_SSL_SERVICE_PEM 389
#define VAR_SSL_PORT 390
#define VAR_FORWARD_FIRST 391
#define VAR_STUB_SSL_UPSTREAM 392
#define VAR_FORWARD_SSL_UPSTREAM 393
#define VAR_TLS_CERT_BUNDLE 394
#define VAR_STUB_FIRST 395
#define VAR_MINIMAL_RESPONSES 396
#define VAR_RRSET_ROUNDROBIN 397
#define VAR_MAX_UDP_SIZE 398
#define VAR_DELAY_CLOSE 399
#define VAR_UNBLOCK_LAN_ZONES 400
#define VAR_INSECURE_LAN_ZONES 401
#define VAR_INFRA_CACHE_MIN_RTT 402
#define VAR_DNS64_PREFIX 403
#define VAR_DNS64_SYNTHALL 404
#define VAR_DNS64_IGNORE_AAAA 405
#define VAR_DNSTAP 406
#define VAR_DNSTAP_ENABLE 407
#define VAR_DNSTAP_SOCKET_PATH 408
#define VAR_DNSTAP_SEND_IDENTITY 409
#define VAR_DNSTAP_SEND_VERSION 410
#define VAR_DNSTAP_IDENTITY 411
#define VAR_DNSTAP_VERSION 412
#define VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES 413
#define VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES 414
#define VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES 415
#define VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES 416
#define VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES 417
#define VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES 418
#define VAR_RESPONSE_IP_TAG 419
#define VAR_RESPONSE_IP 420
#define VAR_RESPONSE_IP_DATA 421
#define VAR_HARDEN_ALGO_DOWNGRADE 422
#define VAR_IP_TRANSPARENT 423
#define VAR_DISABLE_DNSSEC_LAME_CHECK 424
#define VAR_IP_RATELIMIT 425
#define VAR_IP_RATELIMIT_SLABS 426
#define VAR_IP_RATELIMIT_SIZE 427
#define VAR_RATELIMIT 428
#define VAR_RATELIMIT_SLABS 429
#define VAR_RATELIMIT_SIZE 430
#define VAR_RATELIMIT_FOR_DOMAIN 431
#define VAR_RATELIMIT_BELOW_DOMAIN 432
#define VAR_IP_RATELIMIT_FACTOR 433
#define VAR_RATELIMIT_FACTOR 434
#define VAR_SEND_CLIENT_SUBNET 435
#define VAR_CLIENT_SUBNET_ZONE 436
#define VAR_CLIENT_SUBNET_ALWAYS_FORWARD 437
#define VAR_CLIENT_SUBNET_OPCODE 438
#define VAR_MAX_CLIENT_SUBNET_IPV4 439
#define VAR_MAX_CLIENT_SUBNET_IPV6 440
#define VAR_CAPS_WHITELIST 441
#define VAR_CACHE_MAX_NEGATIVE_TTL 442
#define VAR_PERMIT_SMALL_HOLDDOWN 443
#define VAR_QNAME_MINIMISATION 444
#define VAR_QNAME_MINIMISATION_STRICT 445
#define VAR_IP_FREEBIND 446
#define VAR_DEFINE_TAG 447
#define VAR_LOCAL_ZONE_TAG 448
#define VAR_ACCESS_CONTROL_TAG 449
#define VAR_LOCAL_ZONE_OVERRIDE 450
#define VAR_ACCESS_CONTROL_TAG_ACTION 451
#define VAR_ACCESS_CONTROL_TAG_DATA 452
#define VAR_VIEW 453
#define VAR_ACCESS_CONTROL_VIEW 454
#define VAR_VIEW_FIRST 455
#define VAR_SERVE_EXPIRED 456
#define VAR_FAKE_DSA 457
#define VAR_FAKE_SHA1 458
#define VAR_LOG_IDENTITY 459
#define VAR_HIDE_TRUSTANCHOR 460
#define VAR_TRUST_ANCHOR_SIGNALING 461
#define VAR_AGGRESSIVE_NSEC 462
#define VAR_USE_SYSTEMD 463
#define VAR_SHM_ENABLE 464
#define VAR_SHM_KEY 465
#define VAR_ROOT_KEY_SENTINEL 466
#define VAR_DNSCRYPT 467
#define VAR_DNSCRYPT_ENABLE 468
#define VAR_DNSCRYPT_PORT 469
#define VAR_DNSCRYPT_PROVIDER 470
#define VAR_DNSCRYPT_SECRET_KEY 471
#define VAR_DNSCRYPT_PROVIDER_CERT 472
#define VAR_DNSCRYPT_PROVIDER_CERT_ROTATED 473
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE 474
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS 475
#define VAR_DNSCRYPT_NONCE_CACHE_SIZE 476
#define VAR_DNSCRYPT_NONCE_CACHE_SLABS 477
#define VAR_IPSECMOD_ENABLED 478
#define VAR_IPSECMOD_HOOK 479
#define VAR_IPSECMOD_IGNORE_BOGUS 480
#define VAR_IPSECMOD_MAX_TTL 481
#define VAR_IPSECMOD_WHITELIST 482
#define VAR_IPSECMOD_STRICT 483
#define VAR_CACHEDB 484
#define VAR_CACHEDB_BACKEND 485
#define VAR_CACHEDB_SECRETSEED 486
#define VAR_CACHEDB_REDISHOST 487
#define VAR_CACHEDB_REDISPORT 488
#define VAR_CACHEDB_REDISTIMEOUT 489
#define VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM 490
#define VAR_FOR_UPSTREAM 491
#define VAR_AUTH_ZONE 492
#define VAR_ZONEFILE 493
#define VAR_MASTER 494
#define VAR_URL 495
#define VAR_FOR_DOWNSTREAM 496
#define VAR_FALLBACK_ENABLED 497
#define VAR_TLS_ADDITIONAL_PORT 498
#define VAR_LOW_RTT 499
#define VAR_LOW_RTT_PERMIL 500
#define VAR_ALLOW_NOTIFY 501
#define VAR_TLS_WIN_CERT 502
#define VAR_TCP_CONNECTION_LIMIT 503
#define VAR_FORWARD_NO_CACHE 504
#define VAR_STUB_NO_CACHE 505
#define VAR_LOG_SERVFAIL 506

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 66 "./util/configparser.y" /* yacc.c:355  */

	char*	str;

#line 641 "util/configparser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_UTIL_CONFIGPARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 658 "util/configparser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   508

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  252
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  272
/* YYNRULES -- Number of rules.  */
#define YYNRULES  521
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  781

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   506

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
     245,   246,   247,   248,   249,   250,   251
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   165,   165,   165,   166,   166,   167,   167,   168,   168,
     168,   169,   169,   170,   174,   179,   180,   181,   181,   181,
     182,   182,   183,   183,   184,   184,   185,   185,   185,   186,
     186,   187,   187,   187,   188,   188,   188,   189,   189,   190,
     190,   191,   191,   192,   192,   193,   193,   194,   194,   195,
     195,   196,   196,   197,   197,   197,   198,   198,   198,   199,
     199,   199,   200,   200,   201,   201,   202,   202,   203,   203,
     204,   204,   204,   205,   205,   206,   206,   207,   207,   207,
     208,   208,   209,   209,   210,   210,   211,   211,   211,   212,
     212,   213,   213,   214,   214,   215,   215,   216,   216,   217,
     217,   217,   218,   218,   219,   219,   219,   220,   220,   220,
     221,   221,   221,   222,   222,   222,   222,   223,   224,   224,
     224,   225,   225,   225,   226,   226,   227,   227,   228,   228,
     228,   229,   229,   230,   230,   230,   231,   231,   232,   232,
     233,   234,   234,   235,   235,   236,   236,   237,   238,   238,
     239,   239,   240,   240,   241,   241,   241,   242,   242,   243,
     243,   244,   244,   245,   245,   246,   246,   246,   247,   247,
     247,   248,   248,   248,   249,   249,   250,   251,   251,   252,
     252,   253,   253,   254,   254,   255,   255,   255,   256,   256,
     257,   257,   259,   271,   272,   273,   273,   273,   273,   273,
     274,   274,   276,   288,   289,   290,   290,   290,   290,   291,
     291,   293,   307,   308,   309,   309,   309,   309,   310,   310,
     310,   312,   328,   329,   330,   330,   330,   330,   331,   331,
     331,   332,   334,   343,   352,   363,   372,   381,   390,   401,
     410,   421,   434,   449,   460,   477,   494,   507,   522,   531,
     540,   549,   558,   567,   576,   585,   594,   603,   612,   621,
     630,   639,   652,   661,   674,   683,   692,   701,   708,   715,
     724,   731,   740,   748,   757,   766,   780,   789,   798,   807,
     816,   825,   832,   839,   865,   873,   880,   887,   894,   901,
     909,   917,   925,   932,   943,   954,   961,   970,   979,   988,
     995,  1002,  1010,  1018,  1028,  1038,  1048,  1061,  1072,  1080,
    1093,  1102,  1111,  1120,  1130,  1140,  1148,  1161,  1170,  1178,
    1187,  1195,  1208,  1217,  1224,  1234,  1244,  1254,  1264,  1274,
    1284,  1294,  1304,  1311,  1318,  1325,  1334,  1343,  1352,  1359,
    1369,  1387,  1394,  1412,  1425,  1438,  1447,  1456,  1465,  1474,
    1484,  1494,  1505,  1514,  1523,  1536,  1549,  1558,  1565,  1574,
    1583,  1592,  1601,  1609,  1622,  1630,  1659,  1666,  1681,  1691,
    1701,  1708,  1715,  1724,  1732,  1746,  1765,  1784,  1796,  1808,
    1820,  1831,  1850,  1860,  1869,  1877,  1885,  1898,  1911,  1924,
    1937,  1946,  1955,  1964,  1973,  1983,  1993,  2006,  2019,  2030,
    2043,  2054,  2067,  2077,  2084,  2091,  2100,  2109,  2119,  2129,
    2139,  2146,  2153,  2162,  2171,  2181,  2191,  2198,  2205,  2212,
    2220,  2230,  2240,  2250,  2260,  2290,  2300,  2308,  2317,  2332,
    2341,  2346,  2347,  2348,  2348,  2348,  2349,  2349,  2349,  2350,
    2350,  2352,  2362,  2371,  2378,  2385,  2392,  2399,  2406,  2413,
    2418,  2419,  2420,  2420,  2421,  2421,  2422,  2422,  2423,  2424,
    2425,  2426,  2427,  2428,  2430,  2438,  2445,  2453,  2461,  2468,
    2475,  2484,  2493,  2502,  2511,  2520,  2529,  2534,  2535,  2536,
    2538,  2544,  2554,  2561,  2570,  2578,  2584,  2585,  2587,  2587,
    2587,  2588,  2588,  2589,  2590,  2591,  2592,  2593,  2595,  2605,
    2615,  2622,  2631,  2638,  2647,  2655,  2668,  2676,  2689,  2694,
    2695,  2696,  2696,  2697,  2697,  2697,  2699,  2713,  2728,  2740,
    2755,  2768
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SPACE", "LETTER", "NEWLINE", "COMMENT",
  "COLON", "ANY", "ZONESTR", "STRING_ARG", "VAR_SERVER", "VAR_VERBOSITY",
  "VAR_NUM_THREADS", "VAR_PORT", "VAR_OUTGOING_RANGE", "VAR_INTERFACE",
  "VAR_DO_IP4", "VAR_DO_IP6", "VAR_PREFER_IP6", "VAR_DO_UDP", "VAR_DO_TCP",
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
  "VAR_FORWARD_SSL_UPSTREAM", "VAR_TLS_CERT_BUNDLE", "VAR_STUB_FIRST",
  "VAR_MINIMAL_RESPONSES", "VAR_RRSET_ROUNDROBIN", "VAR_MAX_UDP_SIZE",
  "VAR_DELAY_CLOSE", "VAR_UNBLOCK_LAN_ZONES", "VAR_INSECURE_LAN_ZONES",
  "VAR_INFRA_CACHE_MIN_RTT", "VAR_DNS64_PREFIX", "VAR_DNS64_SYNTHALL",
  "VAR_DNS64_IGNORE_AAAA", "VAR_DNSTAP", "VAR_DNSTAP_ENABLE",
  "VAR_DNSTAP_SOCKET_PATH", "VAR_DNSTAP_SEND_IDENTITY",
  "VAR_DNSTAP_SEND_VERSION", "VAR_DNSTAP_IDENTITY", "VAR_DNSTAP_VERSION",
  "VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES",
  "VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES",
  "VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES",
  "VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES", "VAR_RESPONSE_IP_TAG",
  "VAR_RESPONSE_IP", "VAR_RESPONSE_IP_DATA", "VAR_HARDEN_ALGO_DOWNGRADE",
  "VAR_IP_TRANSPARENT", "VAR_DISABLE_DNSSEC_LAME_CHECK",
  "VAR_IP_RATELIMIT", "VAR_IP_RATELIMIT_SLABS", "VAR_IP_RATELIMIT_SIZE",
  "VAR_RATELIMIT", "VAR_RATELIMIT_SLABS", "VAR_RATELIMIT_SIZE",
  "VAR_RATELIMIT_FOR_DOMAIN", "VAR_RATELIMIT_BELOW_DOMAIN",
  "VAR_IP_RATELIMIT_FACTOR", "VAR_RATELIMIT_FACTOR",
  "VAR_SEND_CLIENT_SUBNET", "VAR_CLIENT_SUBNET_ZONE",
  "VAR_CLIENT_SUBNET_ALWAYS_FORWARD", "VAR_CLIENT_SUBNET_OPCODE",
  "VAR_MAX_CLIENT_SUBNET_IPV4", "VAR_MAX_CLIENT_SUBNET_IPV6",
  "VAR_CAPS_WHITELIST", "VAR_CACHE_MAX_NEGATIVE_TTL",
  "VAR_PERMIT_SMALL_HOLDDOWN", "VAR_QNAME_MINIMISATION",
  "VAR_QNAME_MINIMISATION_STRICT", "VAR_IP_FREEBIND", "VAR_DEFINE_TAG",
  "VAR_LOCAL_ZONE_TAG", "VAR_ACCESS_CONTROL_TAG",
  "VAR_LOCAL_ZONE_OVERRIDE", "VAR_ACCESS_CONTROL_TAG_ACTION",
  "VAR_ACCESS_CONTROL_TAG_DATA", "VAR_VIEW", "VAR_ACCESS_CONTROL_VIEW",
  "VAR_VIEW_FIRST", "VAR_SERVE_EXPIRED", "VAR_FAKE_DSA", "VAR_FAKE_SHA1",
  "VAR_LOG_IDENTITY", "VAR_HIDE_TRUSTANCHOR", "VAR_TRUST_ANCHOR_SIGNALING",
  "VAR_AGGRESSIVE_NSEC", "VAR_USE_SYSTEMD", "VAR_SHM_ENABLE",
  "VAR_SHM_KEY", "VAR_ROOT_KEY_SENTINEL", "VAR_DNSCRYPT",
  "VAR_DNSCRYPT_ENABLE", "VAR_DNSCRYPT_PORT", "VAR_DNSCRYPT_PROVIDER",
  "VAR_DNSCRYPT_SECRET_KEY", "VAR_DNSCRYPT_PROVIDER_CERT",
  "VAR_DNSCRYPT_PROVIDER_CERT_ROTATED",
  "VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE",
  "VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS",
  "VAR_DNSCRYPT_NONCE_CACHE_SIZE", "VAR_DNSCRYPT_NONCE_CACHE_SLABS",
  "VAR_IPSECMOD_ENABLED", "VAR_IPSECMOD_HOOK", "VAR_IPSECMOD_IGNORE_BOGUS",
  "VAR_IPSECMOD_MAX_TTL", "VAR_IPSECMOD_WHITELIST", "VAR_IPSECMOD_STRICT",
  "VAR_CACHEDB", "VAR_CACHEDB_BACKEND", "VAR_CACHEDB_SECRETSEED",
  "VAR_CACHEDB_REDISHOST", "VAR_CACHEDB_REDISPORT",
  "VAR_CACHEDB_REDISTIMEOUT", "VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM",
  "VAR_FOR_UPSTREAM", "VAR_AUTH_ZONE", "VAR_ZONEFILE", "VAR_MASTER",
  "VAR_URL", "VAR_FOR_DOWNSTREAM", "VAR_FALLBACK_ENABLED",
  "VAR_TLS_ADDITIONAL_PORT", "VAR_LOW_RTT", "VAR_LOW_RTT_PERMIL",
  "VAR_ALLOW_NOTIFY", "VAR_TLS_WIN_CERT", "VAR_TCP_CONNECTION_LIMIT",
  "VAR_FORWARD_NO_CACHE", "VAR_STUB_NO_CACHE", "VAR_LOG_SERVFAIL",
  "$accept", "toplevelvars", "toplevelvar", "serverstart",
  "contents_server", "content_server", "stubstart", "contents_stub",
  "content_stub", "forwardstart", "contents_forward", "content_forward",
  "viewstart", "contents_view", "content_view", "authstart",
  "contents_auth", "content_auth", "server_num_threads",
  "server_verbosity", "server_statistics_interval",
  "server_statistics_cumulative", "server_extended_statistics",
  "server_shm_enable", "server_shm_key", "server_port",
  "server_send_client_subnet", "server_client_subnet_zone",
  "server_client_subnet_always_forward", "server_client_subnet_opcode",
  "server_max_client_subnet_ipv4", "server_max_client_subnet_ipv6",
  "server_interface", "server_outgoing_interface", "server_outgoing_range",
  "server_outgoing_port_permit", "server_outgoing_port_avoid",
  "server_outgoing_num_tcp", "server_incoming_num_tcp",
  "server_interface_automatic", "server_do_ip4", "server_do_ip6",
  "server_do_udp", "server_do_tcp", "server_prefer_ip6", "server_tcp_mss",
  "server_outgoing_tcp_mss", "server_tcp_idle_timeout",
  "server_tcp_keepalive", "server_tcp_keepalive_timeout",
  "server_tcp_upstream", "server_udp_upstream_without_downstream",
  "server_ssl_upstream", "server_ssl_service_key",
  "server_ssl_service_pem", "server_ssl_port", "server_tls_cert_bundle",
  "server_tls_win_cert", "server_tls_additional_port",
  "server_use_systemd", "server_do_daemonize", "server_use_syslog",
  "server_log_time_ascii", "server_log_queries", "server_log_replies",
  "server_log_servfail", "server_log_local_actions", "server_chroot",
  "server_username", "server_directory", "server_logfile",
  "server_pidfile", "server_root_hints", "server_dlv_anchor_file",
  "server_dlv_anchor", "server_auto_trust_anchor_file",
  "server_trust_anchor_file", "server_trusted_keys_file",
  "server_trust_anchor", "server_trust_anchor_signaling",
  "server_root_key_sentinel", "server_domain_insecure",
  "server_hide_identity", "server_hide_version", "server_hide_trustanchor",
  "server_identity", "server_version", "server_so_rcvbuf",
  "server_so_sndbuf", "server_so_reuseport", "server_ip_transparent",
  "server_ip_freebind", "server_edns_buffer_size",
  "server_msg_buffer_size", "server_msg_cache_size",
  "server_msg_cache_slabs", "server_num_queries_per_thread",
  "server_jostle_timeout", "server_delay_close",
  "server_unblock_lan_zones", "server_insecure_lan_zones",
  "server_rrset_cache_size", "server_rrset_cache_slabs",
  "server_infra_host_ttl", "server_infra_lame_ttl",
  "server_infra_cache_numhosts", "server_infra_cache_lame_size",
  "server_infra_cache_slabs", "server_infra_cache_min_rtt",
  "server_target_fetch_policy", "server_harden_short_bufsize",
  "server_harden_large_queries", "server_harden_glue",
  "server_harden_dnssec_stripped", "server_harden_below_nxdomain",
  "server_harden_referral_path", "server_harden_algo_downgrade",
  "server_use_caps_for_id", "server_caps_whitelist",
  "server_private_address", "server_private_domain", "server_prefetch",
  "server_prefetch_key", "server_unwanted_reply_threshold",
  "server_do_not_query_address", "server_do_not_query_localhost",
  "server_access_control", "server_module_conf",
  "server_val_override_date", "server_val_sig_skew_min",
  "server_val_sig_skew_max", "server_cache_max_ttl",
  "server_cache_max_negative_ttl", "server_cache_min_ttl",
  "server_bogus_ttl", "server_val_clean_additional",
  "server_val_permissive_mode", "server_aggressive_nsec",
  "server_ignore_cd_flag", "server_serve_expired", "server_fake_dsa",
  "server_fake_sha1", "server_val_log_level",
  "server_val_nsec3_keysize_iterations", "server_add_holddown",
  "server_del_holddown", "server_keep_missing",
  "server_permit_small_holddown", "server_key_cache_size",
  "server_key_cache_slabs", "server_neg_cache_size", "server_local_zone",
  "server_local_data", "server_local_data_ptr", "server_minimal_responses",
  "server_rrset_roundrobin", "server_max_udp_size", "server_dns64_prefix",
  "server_dns64_synthall", "server_dns64_ignore_aaaa", "server_define_tag",
  "server_local_zone_tag", "server_access_control_tag",
  "server_access_control_tag_action", "server_access_control_tag_data",
  "server_local_zone_override", "server_access_control_view",
  "server_response_ip_tag", "server_ip_ratelimit", "server_ratelimit",
  "server_ip_ratelimit_size", "server_ratelimit_size",
  "server_ip_ratelimit_slabs", "server_ratelimit_slabs",
  "server_ratelimit_for_domain", "server_ratelimit_below_domain",
  "server_ip_ratelimit_factor", "server_ratelimit_factor",
  "server_low_rtt", "server_low_rtt_permil", "server_qname_minimisation",
  "server_qname_minimisation_strict", "server_ipsecmod_enabled",
  "server_ipsecmod_ignore_bogus", "server_ipsecmod_hook",
  "server_ipsecmod_max_ttl", "server_ipsecmod_whitelist",
  "server_ipsecmod_strict", "stub_name", "stub_host", "stub_addr",
  "stub_first", "stub_no_cache", "stub_ssl_upstream", "stub_prime",
  "forward_name", "forward_host", "forward_addr", "forward_first",
  "forward_no_cache", "forward_ssl_upstream", "auth_name", "auth_zonefile",
  "auth_master", "auth_url", "auth_allow_notify", "auth_for_downstream",
  "auth_for_upstream", "auth_fallback_enabled", "view_name",
  "view_local_zone", "view_response_ip", "view_response_ip_data",
  "view_local_data", "view_local_data_ptr", "view_first", "rcstart",
  "contents_rc", "content_rc", "rc_control_enable", "rc_control_port",
  "rc_control_interface", "rc_control_use_cert", "rc_server_key_file",
  "rc_server_cert_file", "rc_control_key_file", "rc_control_cert_file",
  "dtstart", "contents_dt", "content_dt", "dt_dnstap_enable",
  "dt_dnstap_socket_path", "dt_dnstap_send_identity",
  "dt_dnstap_send_version", "dt_dnstap_identity", "dt_dnstap_version",
  "dt_dnstap_log_resolver_query_messages",
  "dt_dnstap_log_resolver_response_messages",
  "dt_dnstap_log_client_query_messages",
  "dt_dnstap_log_client_response_messages",
  "dt_dnstap_log_forwarder_query_messages",
  "dt_dnstap_log_forwarder_response_messages", "pythonstart",
  "contents_py", "content_py", "py_script",
  "server_disable_dnssec_lame_check", "server_log_identity",
  "server_response_ip", "server_response_ip_data", "dnscstart",
  "contents_dnsc", "content_dnsc", "dnsc_dnscrypt_enable",
  "dnsc_dnscrypt_port", "dnsc_dnscrypt_provider",
  "dnsc_dnscrypt_provider_cert", "dnsc_dnscrypt_provider_cert_rotated",
  "dnsc_dnscrypt_secret_key", "dnsc_dnscrypt_shared_secret_cache_size",
  "dnsc_dnscrypt_shared_secret_cache_slabs",
  "dnsc_dnscrypt_nonce_cache_size", "dnsc_dnscrypt_nonce_cache_slabs",
  "cachedbstart", "contents_cachedb", "content_cachedb",
  "cachedb_backend_name", "cachedb_secret_seed", "redis_server_host",
  "redis_server_port", "redis_timeout", "server_tcp_connection_limit", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
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
     505,   506
};
# endif

#define YYPACT_NINF -148

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-148)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -148,   206,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,   -12,    97,   157,   145,    39,    49,
     105,   -80,   104,  -147,    22,    23,    24,    28,    29,    30,
      78,    79,    80,    84,    88,    89,   114,   115,   116,   118,
     129,   131,   176,   191,   192,   194,   195,   197,   200,   208,
     209,   210,   211,   212,   216,   217,   218,   219,   220,   228,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   241,
     242,   243,   244,   245,   259,   260,   261,   262,   263,   264,
     266,   272,   273,   274,   276,   277,   278,   279,   280,   281,
     282,   284,   286,   287,   288,   289,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   302,   303,   304,   305,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   336,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   395,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   409,   410,   411,   412,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,   413,   414,   415,   416,   417,
     418,   419,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
     420,   421,   422,   423,   424,   426,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,   427,   428,   429,   430,   431,   432,   434,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,   435,   436,
     437,   438,   439,   440,   441,   442,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,   443,   444,   445,   446,   447,
     448,   449,   450,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,   463,  -148,  -148,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,   474,   475,   476,   477,   478,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,   479,   480,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,   481,   482,
     483,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
     484,   485,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,   486,   487,   488,
     489,   490,   491,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,   492,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,   493,  -148,  -148,   494,   495,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
     496,   497,   498,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,    14,   192,   202,   430,   476,   449,   211,
     485,   508,   221,     3,    16,   194,   204,   213,   223,   432,
     451,   478,   487,   510,     4,     5,     6,    10,    13,     8,
       9,     7,    11,    12,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      17,    18,    80,    83,    92,   171,   172,    19,   144,   145,
     146,   147,   148,   149,    31,    71,    20,    84,    85,    42,
      64,    79,    21,    22,    24,    25,    23,    26,    27,    28,
      29,    30,   115,   183,   116,   118,   119,   120,   185,   189,
     186,   167,    81,    70,    96,   113,   114,   191,   117,    32,
      33,    34,    35,    36,    72,    86,    87,   102,    58,    68,
      59,   175,   176,    97,    52,    53,   174,    54,    55,   106,
     110,   124,   133,   154,   107,    65,    37,    38,    39,    94,
     125,   126,   127,    40,    41,    43,    44,    46,    47,    45,
     131,    48,    49,    50,    56,    75,   111,    89,   132,    82,
     150,    90,    91,   108,   109,    95,    51,    73,    76,    57,
      60,    98,    99,    74,   151,   100,    61,    62,    63,   184,
     112,   164,   165,   173,   101,    69,   103,   104,   105,   152,
      66,    67,    88,    77,    78,    93,   121,   122,   123,   128,
     129,   130,   155,   156,   158,   160,   161,   159,   162,   168,
     134,   135,   138,   139,   136,   137,   140,   141,   143,   142,
     187,   188,   153,   163,   177,   179,   178,   180,   181,   182,
     157,   166,   169,   170,   190,     0,     0,     0,     0,     0,
       0,     0,   193,   195,   196,   197,   199,   200,   201,   198,
       0,     0,     0,     0,     0,     0,   203,   205,   206,   207,
     208,   209,   210,     0,     0,     0,     0,     0,     0,     0,
     212,   214,   215,   218,   219,   216,   220,   217,     0,     0,
       0,     0,     0,     0,     0,     0,   222,   224,   225,   226,
     227,   231,   228,   229,   230,     0,     0,     0,     0,     0,
       0,     0,     0,   431,   433,   435,   434,   440,   436,   437,
     438,   439,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   450,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,     0,   477,   479,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     486,   488,   489,   490,   492,   493,   491,   494,   495,   496,
     497,     0,     0,     0,     0,     0,   509,   511,   512,   513,
     514,   515,   233,   232,   239,   248,   246,   254,   255,   258,
     256,   257,   259,   260,   261,   262,   263,   281,   282,   283,
     284,   285,   308,   309,   310,   315,   316,   251,   317,   318,
     321,   319,   320,   323,   324,   325,   338,   296,   297,   299,
     300,   326,   341,   290,   292,   342,   348,   349,   350,   252,
     307,   362,   363,   291,   357,   275,   247,   286,   339,   345,
     327,     0,     0,   366,   253,   234,   274,   331,   235,   249,
     250,   287,   288,   364,   329,   333,   334,   236,   367,   311,
     337,   276,   295,   343,   344,   347,   356,   289,   360,   358,
     359,   301,   306,   335,   336,   302,   303,   328,   352,   277,
     278,   280,   264,   266,   267,   268,   269,   270,   368,   369,
     370,   312,   313,   314,   322,   371,   372,   373,     0,     0,
       0,   330,   304,   481,   382,   386,   384,   383,   387,   385,
       0,     0,   390,   391,   240,   241,   242,   243,   244,   245,
     332,   346,   361,   394,   395,   305,   374,     0,     0,     0,
       0,     0,     0,   353,   354,   355,   482,   298,   293,   351,
     273,   237,   238,   294,   396,   398,   397,   399,   400,   401,
     265,   272,   392,   393,   271,     0,   279,   402,   403,   404,
     408,   407,   405,   406,   409,   410,   411,   412,   414,   413,
     423,     0,   427,   428,     0,     0,   429,   415,   421,   416,
     417,   418,   420,   422,   419,   441,   443,   442,   445,   446,
     447,   448,   444,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   480,   498,   499,   500,   503,
     501,   502,   504,   505,   506,   507,   516,   517,   518,   519,
     520,   340,   365,   381,   483,   484,   388,   389,   375,   376,
       0,     0,     0,   380,   521,   424,   425,   426,   379,   377,
     378
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    13,    14,    24,   209,    15,    25,   392,    16,
      26,   406,    17,    27,   420,    18,    28,   436,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   393,   394,
     395,   396,   397,   398,   399,   407,   408,   409,   410,   411,
     412,   437,   438,   439,   440,   441,   442,   443,   444,   421,
     422,   423,   424,   425,   426,   427,    19,    29,   453,   454,
     455,   456,   457,   458,   459,   460,   461,    20,    30,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,    21,    31,   488,   489,   380,   381,   382,   383,
      22,    32,   500,   501,   502,   503,   504,   505,   506,   507,
     508,   509,   510,    23,    33,   516,   517,   518,   519,   520,
     521,   384
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,   487,   522,   523,   524,    65,    66,    67,   525,   526,
     527,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   428,   511,   512,   513,   514,   515,   528,   529,
     530,   109,   110,   111,   531,   112,   113,   114,   532,   533,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   534,   535,   536,   139,   537,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   538,
     385,   539,   386,   387,   445,   446,   447,   448,   449,   450,
     451,   452,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   540,   184,   413,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     400,   541,   542,   388,   543,   544,     2,   545,   401,   402,
     546,   196,   197,   198,   199,   200,   201,     3,   547,   548,
     549,   550,   551,   202,   414,   415,   552,   553,   554,   555,
     556,   203,   204,   205,   389,   206,   207,   390,   557,   208,
     558,   559,   560,   561,   562,   563,   564,   565,   566,   416,
       4,   567,   568,   569,   570,   571,     5,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   572,
     573,   574,   575,   576,   577,   429,   578,   430,   431,   432,
     433,   434,   579,   580,   581,   435,   582,   583,   584,   585,
     586,   587,   588,   403,   589,   404,   590,   591,   592,   593,
       6,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     417,   418,   603,   604,   605,   606,     7,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   607,   608,   609,
     610,   611,   612,   613,   614,   615,   616,   617,   618,   619,
     620,   621,   622,   623,   624,   419,   625,   391,   626,   627,
     628,   629,   630,   631,   632,   633,   634,     8,   635,   636,
     637,   638,   639,   640,   641,   642,   643,   644,   645,   646,
     647,   648,   649,   650,   651,   652,   653,   654,   655,   656,
     657,   658,   659,   660,   661,   662,   663,   664,   665,   666,
     667,   668,   669,   670,   671,   672,   673,   674,   675,   676,
     677,   678,   679,   680,     9,   681,   405,   682,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,    10,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,    11,   709,   710,   711,   712,
     713,   714,   715,    12,   716,   717,   718,   719,   720,   721,
     722,   723,   724,   725,   726,   727,   728,   729,   730,   731,
     732,   733,   734,   735,   736,   737,   738,   739,   740,   741,
     742,   743,   744,   745,   746,   747,   748,   749,   750,   751,
     752,   753,   754,   755,   756,   757,   758,   759,   760,   761,
     762,   763,   764,   765,   766,   767,   768,   769,   770,   771,
     772,   773,   774,   775,   776,   777,   778,   779,   780
};

static const yytype_uint8 yycheck[] =
{
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,   111,    10,    10,    10,    47,    48,    49,    10,    10,
      10,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    43,   230,   231,   232,   233,   234,    10,    10,
      10,   103,   104,   105,    10,   107,   108,   109,    10,    10,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,    10,    10,    10,   139,    10,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,    10,
      43,    10,    45,    46,    95,    96,    97,    98,    99,   100,
     101,   102,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    10,   199,    43,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
      43,    10,    10,   106,    10,    10,     0,    10,    51,    52,
      10,   223,   224,   225,   226,   227,   228,    11,    10,    10,
      10,    10,    10,   235,    79,    80,    10,    10,    10,    10,
      10,   243,   244,   245,   137,   247,   248,   140,    10,   251,
      10,    10,    10,    10,    10,    10,    10,    10,    10,   104,
      44,    10,    10,    10,    10,    10,    50,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,    10,
      10,    10,    10,    10,    10,   236,    10,   238,   239,   240,
     241,   242,    10,    10,    10,   246,    10,    10,    10,    10,
      10,    10,    10,   136,    10,   138,    10,    10,    10,    10,
      94,    10,    10,    10,    10,    10,    10,    10,    10,    10,
     165,   166,    10,    10,    10,    10,   110,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,   200,    10,   250,    10,    10,
      10,    10,    10,    10,    10,    10,    10,   151,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,   198,    10,   249,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,   212,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,   229,    10,    10,    10,    10,
      10,    10,    10,   237,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   253,     0,    11,    44,    50,    94,   110,   151,   198,
     212,   229,   237,   254,   255,   258,   261,   264,   267,   468,
     479,   494,   502,   515,   256,   259,   262,   265,   268,   469,
     480,   495,   503,   516,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    47,    48,    49,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,   103,
     104,   105,   107,   108,   109,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   139,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   199,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   223,   224,   225,   226,
     227,   228,   235,   243,   244,   245,   247,   248,   251,   257,
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
     498,   499,   500,   501,   523,    43,    45,    46,   106,   137,
     140,   250,   260,   440,   441,   442,   443,   444,   445,   446,
      43,    51,    52,   136,   138,   249,   263,   447,   448,   449,
     450,   451,   452,    43,    79,    80,   104,   165,   166,   200,
     266,   461,   462,   463,   464,   465,   466,   467,    43,   236,
     238,   239,   240,   241,   242,   246,   269,   453,   454,   455,
     456,   457,   458,   459,   460,    95,    96,    97,    98,    99,
     100,   101,   102,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   111,   496,   497,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   230,   231,   232,   233,   234,   517,   518,   519,   520,
     521,   522,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   252,   253,   253,   254,   254,   254,   254,   254,   254,
     254,   254,   254,   254,   255,   256,   256,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     257,   257,   258,   259,   259,   260,   260,   260,   260,   260,
     260,   260,   261,   262,   262,   263,   263,   263,   263,   263,
     263,   264,   265,   265,   266,   266,   266,   266,   266,   266,
     266,   267,   268,   268,   269,   269,   269,   269,   269,   269,
     269,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     418,   419,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   469,   470,   470,   470,   470,   470,   470,   470,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   480,   481,   481,   481,   481,   481,   481,   481,   481,
     481,   481,   481,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   503,   504,   504,
     504,   504,   504,   504,   504,   504,   504,   504,   505,   506,
     507,   508,   509,   510,   511,   512,   513,   514,   515,   516,
     516,   517,   517,   517,   517,   517,   518,   519,   520,   521,
     522,   523
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     2,     0,     1,     1,     1,
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
       1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
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
       3,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     3,     4,     4,     4,
       3,     3,     2,     2,     2,     2,     2,     2,     3,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     3,     3,     3,     2,     2,     2,
       1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     0,     1,
       2,     2,     2,     3,     3,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     2,
       0,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

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
      int yyn = yypact[*yyssp];
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
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
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
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
          yyp++;
          yyformat++;
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

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
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
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
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
        case 14:
#line 175 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("\nP(server:)\n")); 
	}
#line 2519 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 192:
#line 260 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2534 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 202:
#line 277 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2549 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 211:
#line 294 "./util/configparser.y" /* yacc.c:1646  */
    {
		struct config_view* s;
		OUTYY(("\nP(view:)\n")); 
		s = (struct config_view*)calloc(1, sizeof(struct config_view));
		if(s) {
			s->next = cfg_parser->cfg->views;
			if(s->next && !s->next->name)
				yyerror("view without name");
			cfg_parser->cfg->views = s;
		} else 
			yyerror("out of memory");
	}
#line 2566 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 221:
#line 313 "./util/configparser.y" /* yacc.c:1646  */
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
		} else 
			yyerror("out of memory");
	}
#line 2585 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 232:
#line 335 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_num_threads:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2597 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 233:
#line 344 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_verbosity:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2609 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 234:
#line 353 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_statistics_interval:%s)\n", (yyvsp[0].str))); 
		if(strcmp((yyvsp[0].str), "") == 0 || strcmp((yyvsp[0].str), "0") == 0)
			cfg_parser->cfg->stat_interval = 0;
		else if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->stat_interval = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2623 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 235:
#line 364 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_statistics_cumulative:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2635 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 236:
#line 373 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_extended_statistics:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2647 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 237:
#line 382 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_shm_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->shm_enable = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2659 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 238:
#line 391 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_shm_key:%s)\n", (yyvsp[0].str))); 
		if(strcmp((yyvsp[0].str), "") == 0 || strcmp((yyvsp[0].str), "0") == 0)
			cfg_parser->cfg->shm_key = 0;
		else if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->shm_key = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2673 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 239:
#line 402 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_port:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2685 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 240:
#line 411 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_send_client_subnet:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet, (yyvsp[0].str)))
			fatal_exit("out of memory adding client-subnet");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
	}
#line 2699 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 241:
#line 422 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_zone:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet_zone,
			(yyvsp[0].str)))
			fatal_exit("out of memory adding client-subnet-zone");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
	}
#line 2714 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 242:
#line 435 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_always_forward:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->client_subnet_always_forward =
				(strcmp((yyvsp[0].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 2732 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 243:
#line 450 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(client_subnet_opcode:%s)\n", (yyvsp[0].str)));
		OUTYY(("P(Deprecated option, ignoring)\n"));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 2746 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 244:
#line 461 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2766 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 245:
#line 478 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2786 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 246:
#line 495 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2802 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 247:
#line 508 "./util/configparser.y" /* yacc.c:1646  */
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
#line 2820 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 248:
#line 523 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_outgoing_range:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2832 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 249:
#line 532 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_outgoing_port_permit:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 1, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 2844 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 250:
#line 541 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 0, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 2856 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 251:
#line 550 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2868 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 252:
#line 559 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_incoming_num_tcp:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2880 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 253:
#line 568 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_interface_automatic:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2892 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 254:
#line 577 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_ip4:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2904 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 255:
#line 586 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_ip6:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2916 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 256:
#line 595 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_udp:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2928 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 257:
#line 604 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_tcp:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2940 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 258:
#line 613 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_prefer_ip6:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip6 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2952 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 259:
#line 622 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_mss:%s)\n", (yyvsp[0].str)));
                if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
                        yyerror("number expected");
                else cfg_parser->cfg->tcp_mss = atoi((yyvsp[0].str));
                free((yyvsp[0].str));
	}
#line 2964 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 260:
#line 631 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_outgoing_tcp_mss:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_tcp_mss = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2976 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 261:
#line 640 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_idle_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 120000)
			cfg_parser->cfg->tcp_idle_timeout = 120000;
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_idle_timeout = 1;
		else cfg_parser->cfg->tcp_idle_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2992 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 262:
#line 653 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_keepalive:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp_keepalive = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3004 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 263:
#line 662 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_keepalive_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 6553500)
			cfg_parser->cfg->tcp_keepalive_timeout = 6553500;
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_keepalive_timeout = 0;
		else cfg_parser->cfg->tcp_keepalive_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3020 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 264:
#line 675 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tcp_upstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3032 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 265:
#line 684 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_udp_upstream_without_downstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_upstream_without_downstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3044 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 266:
#line 693 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ssl_upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ssl_upstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3056 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 267:
#line 702 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ssl_service_key:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ssl_service_key);
		cfg_parser->cfg->ssl_service_key = (yyvsp[0].str);
	}
#line 3066 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 268:
#line 709 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ssl_service_pem:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ssl_service_pem);
		cfg_parser->cfg->ssl_service_pem = (yyvsp[0].str);
	}
#line 3076 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 269:
#line 716 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ssl_port:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->ssl_port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3088 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 270:
#line 725 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tls_cert_bundle:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = (yyvsp[0].str);
	}
#line 3098 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 271:
#line 732 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tls_win_cert:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_win_cert = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3110 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 272:
#line 741 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tls_additional_port:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->tls_additional_port,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3121 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 273:
#line 749 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_use_systemd:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_systemd = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3133 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 274:
#line 758 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_daemonize:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3145 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 275:
#line 767 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_use_syslog:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp((yyvsp[0].str), "yes")==0);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if(strcmp((yyvsp[0].str), "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free((yyvsp[0].str));
	}
#line 3162 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 276:
#line 781 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_log_time_ascii:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3174 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 277:
#line 790 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_log_queries:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_queries = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3186 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 278:
#line 799 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(server_log_replies:%s)\n", (yyvsp[0].str)));
  	if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_replies = (strcmp((yyvsp[0].str), "yes")==0);
  	free((yyvsp[0].str));
  }
#line 3198 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 279:
#line 808 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_log_servfail:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_servfail = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3210 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 280:
#line 817 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(server_log_local_actions:%s)\n", (yyvsp[0].str)));
  	if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_local_actions = (strcmp((yyvsp[0].str), "yes")==0);
  	free((yyvsp[0].str));
  }
#line 3222 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 281:
#line 826 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_chroot:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = (yyvsp[0].str);
	}
#line 3232 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 282:
#line 833 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_username:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = (yyvsp[0].str);
	}
#line 3242 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 283:
#line 840 "./util/configparser.y" /* yacc.c:1646  */
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
#line 3271 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 284:
#line 866 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_logfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = (yyvsp[0].str);
		cfg_parser->cfg->use_syslog = 0;
	}
#line 3282 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 285:
#line 874 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_pidfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = (yyvsp[0].str);
	}
#line 3292 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 286:
#line 881 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_root_hints:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3302 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 287:
#line 888 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_dlv_anchor_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dlv_anchor_file);
		cfg_parser->cfg->dlv_anchor_file = (yyvsp[0].str);
	}
#line 3312 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 288:
#line 895 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_dlv_anchor:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dlv_anchor_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3322 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 289:
#line 902 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3333 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 290:
#line 910 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3344 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 291:
#line 918 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_trusted_keys_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3355 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 292:
#line 926 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_trust_anchor:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3365 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 293:
#line 933 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_trust_anchor_signaling:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->trust_anchor_signaling =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3379 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 294:
#line 944 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_root_key_sentinel:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->root_key_sentinel =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3393 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 295:
#line 955 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_domain_insecure:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3403 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 296:
#line 962 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_hide_identity:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3415 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 297:
#line 971 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_hide_version:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3427 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 298:
#line 980 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_hide_trustanchor:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_trustanchor = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3439 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 299:
#line 989 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = (yyvsp[0].str);
	}
#line 3449 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 300:
#line 996 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_version:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = (yyvsp[0].str);
	}
#line 3459 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 301:
#line 1003 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_so_rcvbuf:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->so_rcvbuf))
			yyerror("buffer size expected");
		free((yyvsp[0].str));
	}
#line 3470 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1011 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_so_sndbuf:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->so_sndbuf))
			yyerror("buffer size expected");
		free((yyvsp[0].str));
	}
#line 3481 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1019 "./util/configparser.y" /* yacc.c:1646  */
    {
        OUTYY(("P(server_so_reuseport:%s)\n", (yyvsp[0].str)));
        if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->so_reuseport =
            (strcmp((yyvsp[0].str), "yes")==0);
        free((yyvsp[0].str));
    }
#line 3494 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1029 "./util/configparser.y" /* yacc.c:1646  */
    {
        OUTYY(("P(server_ip_transparent:%s)\n", (yyvsp[0].str)));
        if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_transparent =
            (strcmp((yyvsp[0].str), "yes")==0);
        free((yyvsp[0].str));
    }
#line 3507 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1039 "./util/configparser.y" /* yacc.c:1646  */
    {
        OUTYY(("P(server_ip_freebind:%s)\n", (yyvsp[0].str)));
        if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_freebind =
            (strcmp((yyvsp[0].str), "yes")==0);
        free((yyvsp[0].str));
    }
#line 3520 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1049 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_edns_buffer_size:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 12)
			yyerror("edns buffer size too small");
		else if (atoi((yyvsp[0].str)) > 65535)
			cfg_parser->cfg->edns_buffer_size = 65535;
		else cfg_parser->cfg->edns_buffer_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3536 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1062 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_msg_buffer_size:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else cfg_parser->cfg->msg_buffer_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3550 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1073 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_msg_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 3561 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1081 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_msg_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->msg_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3577 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 310:
#line 1094 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_num_queries_per_thread:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3589 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 311:
#line 1103 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_jostle_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3601 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 312:
#line 1112 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_delay_close:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->delay_close = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3613 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 313:
#line 1121 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_unblock_lan_zones:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->unblock_lan_zones = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3626 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 314:
#line 1131 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_insecure_lan_zones:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->insecure_lan_zones = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3639 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 315:
#line 1141 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_rrset_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 3650 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 316:
#line 1149 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->rrset_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3666 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 317:
#line 1162 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_host_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3678 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 318:
#line 1171 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_lame_ttl:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3689 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 319:
#line 1179 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3701 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 320:
#line 1188 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3712 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 321:
#line 1196 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->infra_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3728 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 322:
#line 1209 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_infra_cache_min_rtt:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_min_rtt = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3740 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 323:
#line 1218 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_target_fetch_policy:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = (yyvsp[0].str);
	}
#line 3750 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 324:
#line 1225 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_short_bufsize:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3763 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 325:
#line 1235 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_large_queries:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3776 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 326:
#line 1245 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_glue:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3789 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 327:
#line 1255 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3802 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 328:
#line 1265 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_below_nxdomain:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_below_nxdomain = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3815 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 329:
#line 1275 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_referral_path:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3828 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 330:
#line 1285 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_harden_algo_downgrade:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_algo_downgrade = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3841 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 331:
#line 1295 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_use_caps_for_id:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3854 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 332:
#line 1305 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_caps_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3864 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 333:
#line 1312 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_private_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3874 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 334:
#line 1319 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_private_domain:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3884 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 335:
#line 1326 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_prefetch:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3896 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 336:
#line 1335 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_prefetch_key:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch_key = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3908 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 337:
#line 1344 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3920 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 338:
#line 1353 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_not_query_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3930 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1360 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_do_not_query_localhost:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3943 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 340:
#line 1370 "./util/configparser.y" /* yacc.c:1646  */
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
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding acl");
		}
	}
#line 3964 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 341:
#line 1388 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_module_conf:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = (yyvsp[0].str);
	}
#line 3974 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 342:
#line 1395 "./util/configparser.y" /* yacc.c:1646  */
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
#line 3995 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1413 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_sig_skew_min:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_min = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_min = atoi((yyvsp[0].str));
			if(!cfg_parser->cfg->val_sig_skew_min)
				yyerror("number expected");
		}
		free((yyvsp[0].str));
	}
#line 4011 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 344:
#line 1426 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_sig_skew_max:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_max = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_max = atoi((yyvsp[0].str));
			if(!cfg_parser->cfg->val_sig_skew_max)
				yyerror("number expected");
		}
		free((yyvsp[0].str));
	}
#line 4027 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 345:
#line 1439 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_cache_max_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4039 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1448 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_cache_max_negative_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_negative_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4051 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1457 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_cache_min_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4063 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 348:
#line 1466 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_bogus_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4075 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1475 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_clean_additional:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4088 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 350:
#line 1485 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_permissive_mode:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4101 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1495 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_aggressive_nsec:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->aggressive_nsec =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4115 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1506 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ignore_cd_flag:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ignore_cd = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4127 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1515 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_serve_expired:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4139 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 354:
#line 1524 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_fake_dsa:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
#ifdef HAVE_SSL
		else fake_dsa = (strcmp((yyvsp[0].str), "yes")==0);
		if(fake_dsa)
			log_warn("test option fake_dsa is enabled");
#endif
		free((yyvsp[0].str));
	}
#line 4155 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 355:
#line 1537 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_fake_sha1:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
#ifdef HAVE_SSL
		else fake_sha1 = (strcmp((yyvsp[0].str), "yes")==0);
		if(fake_sha1)
			log_warn("test option fake_sha1 is enabled");
#endif
		free((yyvsp[0].str));
	}
#line 4171 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 356:
#line 1550 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_log_level:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->val_log_level = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4183 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 357:
#line 1559 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = (yyvsp[0].str);
	}
#line 4193 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 358:
#line 1566 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_add_holddown:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->add_holddown = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4205 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 359:
#line 1575 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_del_holddown:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->del_holddown = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4217 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 360:
#line 1584 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_keep_missing:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->keep_missing = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4229 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 361:
#line 1593 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_permit_small_holddown:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->permit_small_holddown =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4242 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 362:
#line 1602 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_key_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4253 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 363:
#line 1610 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_key_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->key_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->key_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 4269 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 364:
#line 1623 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_neg_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4280 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 365:
#line 1631 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_local_zone:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "static")!=0 && strcmp((yyvsp[0].str), "deny")!=0 &&
		   strcmp((yyvsp[0].str), "refuse")!=0 && strcmp((yyvsp[0].str), "redirect")!=0 &&
		   strcmp((yyvsp[0].str), "transparent")!=0 && strcmp((yyvsp[0].str), "nodefault")!=0
		   && strcmp((yyvsp[0].str), "typetransparent")!=0
		   && strcmp((yyvsp[0].str), "always_transparent")!=0
		   && strcmp((yyvsp[0].str), "always_refuse")!=0
		   && strcmp((yyvsp[0].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[0].str), "noview")!=0
		   && strcmp((yyvsp[0].str), "inform")!=0 && strcmp((yyvsp[0].str), "inform_deny")!=0)
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"always_transparent, always_refuse, "
				"always_nxdomain, noview or nodefault");
		else if(strcmp((yyvsp[0].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones, 
				(yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
#line 4312 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 366:
#line 1660 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_local_data:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, (yyvsp[0].str)))
			fatal_exit("out of memory adding local-data");
	}
#line 4322 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 367:
#line 1667 "./util/configparser.y" /* yacc.c:1646  */
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
#line 4340 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 368:
#line 1682 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_minimal_responses:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->minimal_responses =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4353 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 369:
#line 1692 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_rrset_roundrobin:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->rrset_roundrobin =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4366 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 370:
#line 1702 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_max_udp_size:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->max_udp_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4376 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 371:
#line 1709 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dns64_prefix:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dns64_prefix);
		cfg_parser->cfg->dns64_prefix = (yyvsp[0].str);
	}
#line 4386 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 372:
#line 1716 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_dns64_synthall:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dns64_synthall = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4398 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 373:
#line 1725 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dns64_ignore_aaaa:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dns64_ignore_aaaa,
			(yyvsp[0].str)))
			fatal_exit("out of memory adding dns64-ignore-aaaa");
	}
#line 4409 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 374:
#line 1733 "./util/configparser.y" /* yacc.c:1646  */
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
#line 4426 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 375:
#line 1747 "./util/configparser.y" /* yacc.c:1646  */
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist)
			yyerror("could not parse tags, (define-tag them first)");
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->local_zone_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4448 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 376:
#line 1766 "./util/configparser.y" /* yacc.c:1646  */
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_access_control_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist)
			yyerror("could not parse tags, (define-tag them first)");
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->acl_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4470 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 377:
#line 1785 "./util/configparser.y" /* yacc.c:1646  */
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
#line 4485 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 378:
#line 1797 "./util/configparser.y" /* yacc.c:1646  */
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
#line 4500 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 379:
#line 1809 "./util/configparser.y" /* yacc.c:1646  */
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
#line 4515 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 380:
#line 1821 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_access_control_view:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acl_view,
			(yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4529 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 381:
#line 1832 "./util/configparser.y" /* yacc.c:1646  */
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(response_ip_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist)
			yyerror("could not parse tags, (define-tag them first)");
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->respip_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4551 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 382:
#line 1851 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_ip_ratelimit:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4563 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 383:
#line 1861 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_ratelimit:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4575 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 384:
#line 1870 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(server_ip_ratelimit_size:%s)\n", (yyvsp[0].str)));
  	if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->ip_ratelimit_size))
  		yyerror("memory size expected");
  	free((yyvsp[0].str));
  }
#line 4586 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 385:
#line 1878 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ratelimit_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->ratelimit_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4597 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 386:
#line 1886 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(server_ip_ratelimit_slabs:%s)\n", (yyvsp[0].str)));
  	if(atoi((yyvsp[0].str)) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->ip_ratelimit_slabs = atoi((yyvsp[0].str));
  		if(!is_pow2(cfg_parser->cfg->ip_ratelimit_slabs))
  			yyerror("must be a power of 2");
  	}
  	free((yyvsp[0].str));
  }
#line 4613 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 387:
#line 1899 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ratelimit_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->ratelimit_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 4629 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 388:
#line 1912 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ratelimit_for_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_for_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-for-domain");
		}
	}
#line 4645 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 389:
#line 1925 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_ratelimit_below_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_below_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-below-domain");
		}
	}
#line 4661 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 390:
#line 1938 "./util/configparser.y" /* yacc.c:1646  */
    { 
  	OUTYY(("P(server_ip_ratelimit_factor:%s)\n", (yyvsp[0].str))); 
  	if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
  		yyerror("number expected");
  	else cfg_parser->cfg->ip_ratelimit_factor = atoi((yyvsp[0].str));
  	free((yyvsp[0].str));
	}
#line 4673 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 391:
#line 1947 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_ratelimit_factor:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit_factor = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4685 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 392:
#line 1956 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_low_rtt:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->low_rtt = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4697 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 393:
#line 1965 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("P(server_low_rtt_permil:%s)\n", (yyvsp[0].str))); 
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->low_rtt_permil = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4709 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 394:
#line 1974 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_qname_minimisation:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4722 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 395:
#line 1984 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_qname_minimisation_strict:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation_strict = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4735 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 396:
#line 1994 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_enabled:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_enabled = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4751 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 397:
#line 2007 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_ignore_bogus:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_ignore_bogus = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4767 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 398:
#line 2020 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_hook:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ipsecmod_hook);
		cfg_parser->cfg->ipsecmod_hook = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4781 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 399:
#line 2031 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_max_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ipsecmod_max_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4797 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 400:
#line 2044 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->ipsecmod_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4811 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 401:
#line 2055 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_strict:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_strict = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
	}
#line 4827 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 402:
#line 2068 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = (yyvsp[0].str);
	}
#line 4840 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 403:
#line 2078 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4850 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 404:
#line 2085 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4860 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 405:
#line 2092 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-first:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isfirst=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4872 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 406:
#line 2101 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-no-cache:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->no_cache=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4884 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 407:
#line 2110 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-ssl-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->ssl_upstream = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4897 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 408:
#line 2120 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(stub-prime:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4910 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 409:
#line 2130 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = (yyvsp[0].str);
	}
#line 4923 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 410:
#line 2140 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(forward-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4933 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 411:
#line 2147 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(forward-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4943 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 412:
#line 2154 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(forward-first:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->isfirst=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4955 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 413:
#line 2163 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(forward-no-cache:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->no_cache=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4967 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 414:
#line 2172 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(forward-ssl-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->ssl_upstream = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4980 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 415:
#line 2182 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = (yyvsp[0].str);
	}
#line 4993 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 416:
#line 2192 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(zonefile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = (yyvsp[0].str);
	}
#line 5003 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 417:
#line 2199 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(master:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5013 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 418:
#line 2206 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(url:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5023 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 419:
#line 2213 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(allow-notify:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5034 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 420:
#line 2221 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(for-downstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_downstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5047 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 421:
#line 2231 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(for-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_upstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5060 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 422:
#line 2241 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(fallback-enabled:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->fallback_enabled =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5073 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 423:
#line 2251 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->views->name)
			yyerror("view name override, there must be one "
				"name for one view");
		free(cfg_parser->cfg->views->name);
		cfg_parser->cfg->views->name = (yyvsp[0].str);
	}
#line 5086 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 424:
#line 2261 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(view_local_zone:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "static")!=0 && strcmp((yyvsp[0].str), "deny")!=0 &&
		   strcmp((yyvsp[0].str), "refuse")!=0 && strcmp((yyvsp[0].str), "redirect")!=0 &&
		   strcmp((yyvsp[0].str), "transparent")!=0 && strcmp((yyvsp[0].str), "nodefault")!=0
		   && strcmp((yyvsp[0].str), "typetransparent")!=0
		   && strcmp((yyvsp[0].str), "always_transparent")!=0
		   && strcmp((yyvsp[0].str), "always_refuse")!=0
		   && strcmp((yyvsp[0].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[0].str), "noview")!=0
		   && strcmp((yyvsp[0].str), "inform")!=0 && strcmp((yyvsp[0].str), "inform_deny")!=0)
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"always_transparent, always_refuse, "
				"always_nxdomain, noview or nodefault");
		else if(strcmp((yyvsp[0].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_zones_nodefault, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(
				&cfg_parser->cfg->views->local_zones, 
				(yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
#line 5119 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 425:
#line 2291 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(view_response_ip:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		validate_respip_action((yyvsp[0].str));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_actions, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding per-view "
				"response-ip action");
	}
#line 5132 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 426:
#line 2301 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(view_response_ip_data:%s)\n", (yyvsp[-1].str)));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_data, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding response-ip-data");
	}
#line 5143 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 427:
#line 2309 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(view_local_data:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->views->local_data, (yyvsp[0].str))) {
			fatal_exit("out of memory adding local-data");
			free((yyvsp[0].str));
		}
	}
#line 5155 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 428:
#line 2318 "./util/configparser.y" /* yacc.c:1646  */
    {
		char* ptr;
		OUTYY(("P(view_local_data_ptr:%s)\n", (yyvsp[0].str)));
		ptr = cfg_ptr_reverse((yyvsp[0].str));
		free((yyvsp[0].str));
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
#line 5173 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 429:
#line 2333 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(view-first:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->views->isfirst=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5185 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 430:
#line 2342 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("\nP(remote-control:)\n")); 
	}
#line 5193 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 441:
#line 2353 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(control_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable = 
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5206 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 442:
#line 2363 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(control_port:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5218 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 443:
#line 2372 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(control_interface:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5228 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 444:
#line 2379 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(control_use_cert:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->control_use_cert = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5238 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 445:
#line 2386 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(rc_server_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = (yyvsp[0].str);
	}
#line 5248 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 446:
#line 2393 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(rc_server_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = (yyvsp[0].str);
	}
#line 5258 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 447:
#line 2400 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(rc_control_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = (yyvsp[0].str);
	}
#line 5268 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 448:
#line 2407 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(rc_control_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = (yyvsp[0].str);
	}
#line 5278 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 449:
#line 2414 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("\nP(dnstap:)\n"));
	}
#line 5286 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 464:
#line 2431 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap = (strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5297 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 465:
#line 2439 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_socket_path:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_socket_path);
		cfg_parser->cfg->dnstap_socket_path = (yyvsp[0].str);
	}
#line 5307 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 466:
#line 2446 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_send_identity:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_identity = (strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5318 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 467:
#line 2454 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_send_version:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_version = (strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5329 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 468:
#line 2462 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_identity);
		cfg_parser->cfg->dnstap_identity = (yyvsp[0].str);
	}
#line 5339 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 469:
#line 2469 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_version:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnstap_version);
		cfg_parser->cfg->dnstap_version = (yyvsp[0].str);
	}
#line 5349 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 470:
#line 2476 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_resolver_query_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_query_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5361 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 471:
#line 2485 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_resolver_response_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_response_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5373 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 472:
#line 2494 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_client_query_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_query_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5385 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 473:
#line 2503 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_client_response_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_response_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5397 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 474:
#line 2512 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_forwarder_query_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_query_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5409 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 475:
#line 2521 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dt_dnstap_log_forwarder_response_messages:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_response_messages =
			(strcmp((yyvsp[0].str), "yes")==0);
	}
#line 5421 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 476:
#line 2530 "./util/configparser.y" /* yacc.c:1646  */
    { 
		OUTYY(("\nP(python:)\n")); 
	}
#line 5429 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 480:
#line 2539 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(python-script:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->python_script);
		cfg_parser->cfg->python_script = (yyvsp[0].str);
	}
#line 5439 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 481:
#line 2545 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(disable_dnssec_lame_check:%s)\n", (yyvsp[0].str)));
		if (strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_dnssec_lame_check =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5452 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 482:
#line 2555 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_log_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = (yyvsp[0].str);
	}
#line 5462 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 483:
#line 2562 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_response_ip:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		validate_respip_action((yyvsp[0].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_actions,
			(yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding response-ip");
	}
#line 5474 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 484:
#line 2571 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_response_ip_data:%s)\n", (yyvsp[-1].str)));
			if(!cfg_str2list_insert(&cfg_parser->cfg->respip_data,
				(yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding response-ip-data");
	}
#line 5485 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 485:
#line 2579 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("\nP(dnscrypt:)\n"));
		OUTYY(("\nP(dnscrypt:)\n"));
	}
#line 5494 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 498:
#line 2596 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnscrypt = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5506 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 499:
#line 2606 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_port:%s)\n", (yyvsp[0].str)));

		if(atoi((yyvsp[0].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->dnscrypt_port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5519 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 500:
#line 2616 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_provider:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->dnscrypt_provider);
		cfg_parser->cfg->dnscrypt_provider = (yyvsp[0].str);
	}
#line 5529 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 501:
#line 2623 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_provider_cert:%s)\n", (yyvsp[0].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[0].str)))
			log_warn("dnscrypt-provider-cert %s is a duplicate", (yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert");
	}
#line 5541 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 502:
#line 2632 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_provider_cert_rotated:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert_rotated, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert-rotated");
	}
#line 5551 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 503:
#line 2639 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(dnsc_dnscrypt_secret_key:%s)\n", (yyvsp[0].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[0].str)))
			log_warn("dnscrypt-secret-key: %s is a duplicate", (yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[0].str)))
			fatal_exit("out of memory adding dnscrypt-secret-key");
	}
#line 5563 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 504:
#line 2648 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(dnscrypt_shared_secret_cache_size:%s)\n", (yyvsp[0].str)));
  	if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->dnscrypt_shared_secret_cache_size))
  		yyerror("memory size expected");
  	free((yyvsp[0].str));
  }
#line 5574 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 505:
#line 2656 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(dnscrypt_shared_secret_cache_slabs:%s)\n", (yyvsp[0].str)));
  	if(atoi((yyvsp[0].str)) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs = atoi((yyvsp[0].str));
  		if(!is_pow2(cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs))
  			yyerror("must be a power of 2");
  	}
  	free((yyvsp[0].str));
  }
#line 5590 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 506:
#line 2669 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(dnscrypt_nonce_cache_size:%s)\n", (yyvsp[0].str)));
  	if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->dnscrypt_nonce_cache_size))
  		yyerror("memory size expected");
  	free((yyvsp[0].str));
  }
#line 5601 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 507:
#line 2677 "./util/configparser.y" /* yacc.c:1646  */
    {
  	OUTYY(("P(dnscrypt_nonce_cache_slabs:%s)\n", (yyvsp[0].str)));
  	if(atoi((yyvsp[0].str)) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->dnscrypt_nonce_cache_slabs = atoi((yyvsp[0].str));
  		if(!is_pow2(cfg_parser->cfg->dnscrypt_nonce_cache_slabs))
  			yyerror("must be a power of 2");
  	}
  	free((yyvsp[0].str));
  }
#line 5617 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 508:
#line 2690 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("\nP(cachedb:)\n"));
	}
#line 5625 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 516:
#line 2700 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_CACHEDB
		OUTYY(("P(backend:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->cachedb_backend)
			yyerror("cachedb backend override, there must be one "
				"backend");
		free(cfg_parser->cfg->cachedb_backend);
		cfg_parser->cfg->cachedb_backend = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
	#endif
	}
#line 5642 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 517:
#line 2714 "./util/configparser.y" /* yacc.c:1646  */
    {
	#ifdef USE_CACHEDB
		OUTYY(("P(secret-seed:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->cachedb_secret)
			yyerror("cachedb secret-seed override, there must be "
				"only one secret");
		free(cfg_parser->cfg->cachedb_secret);
		cfg_parser->cfg->cachedb_secret = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5660 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 518:
#line 2729 "./util/configparser.y" /* yacc.c:1646  */
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
#line 5675 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 519:
#line 2741 "./util/configparser.y" /* yacc.c:1646  */
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		int port;
		OUTYY(("P(redis_server_port:%s)\n", (yyvsp[0].str)));
		port = atoi((yyvsp[0].str));
		if(port == 0 || port < 0 || port > 65535)
			yyerror("valid redis server port number expected");
		else cfg_parser->cfg->redis_server_port = port;
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 5693 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 520:
#line 2756 "./util/configparser.y" /* yacc.c:1646  */
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("redis timeout value expected");
		else cfg_parser->cfg->redis_timeout = atoi((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 5709 "util/configparser.c" /* yacc.c:1646  */
    break;

  case 521:
#line 2769 "./util/configparser.y" /* yacc.c:1646  */
    {
		OUTYY(("P(server_tcp_connection_limit:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if (atoi((yyvsp[0].str)) < 0)
			yyerror("positive number expected");
		else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->tcp_connection_limits, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding tcp connection limit");
		}
	}
#line 5723 "util/configparser.c" /* yacc.c:1646  */
    break;


#line 5727 "util/configparser.c" /* yacc.c:1646  */
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

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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
                  yystos[*yyssp], yyvsp);
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
#line 2779 "./util/configparser.y" /* yacc.c:1906  */


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
