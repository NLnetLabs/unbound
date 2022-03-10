/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     VAR_TCP_REUSE_TIMEOUT = 360,
     VAR_MAX_REUSE_TCP_QUERIES = 361,
     VAR_EXTENDED_STATISTICS = 362,
     VAR_LOCAL_DATA_PTR = 363,
     VAR_JOSTLE_TIMEOUT = 364,
     VAR_STUB_PRIME = 365,
     VAR_UNWANTED_REPLY_THRESHOLD = 366,
     VAR_LOG_TIME_ASCII = 367,
     VAR_DOMAIN_INSECURE = 368,
     VAR_PYTHON = 369,
     VAR_PYTHON_SCRIPT = 370,
     VAR_VAL_SIG_SKEW_MIN = 371,
     VAR_VAL_SIG_SKEW_MAX = 372,
     VAR_VAL_MAX_RESTART = 373,
     VAR_CACHE_MIN_TTL = 374,
     VAR_VAL_LOG_LEVEL = 375,
     VAR_AUTO_TRUST_ANCHOR_FILE = 376,
     VAR_KEEP_MISSING = 377,
     VAR_ADD_HOLDDOWN = 378,
     VAR_DEL_HOLDDOWN = 379,
     VAR_SO_RCVBUF = 380,
     VAR_EDNS_BUFFER_SIZE = 381,
     VAR_PREFETCH = 382,
     VAR_PREFETCH_KEY = 383,
     VAR_SO_SNDBUF = 384,
     VAR_SO_REUSEPORT = 385,
     VAR_HARDEN_BELOW_NXDOMAIN = 386,
     VAR_IGNORE_CD_FLAG = 387,
     VAR_LOG_QUERIES = 388,
     VAR_LOG_REPLIES = 389,
     VAR_LOG_LOCAL_ACTIONS = 390,
     VAR_TCP_UPSTREAM = 391,
     VAR_SSL_UPSTREAM = 392,
     VAR_TCP_AUTH_QUERY_TIMEOUT = 393,
     VAR_SSL_SERVICE_KEY = 394,
     VAR_SSL_SERVICE_PEM = 395,
     VAR_SSL_PORT = 396,
     VAR_FORWARD_FIRST = 397,
     VAR_STUB_SSL_UPSTREAM = 398,
     VAR_FORWARD_SSL_UPSTREAM = 399,
     VAR_TLS_CERT_BUNDLE = 400,
     VAR_STUB_TCP_UPSTREAM = 401,
     VAR_FORWARD_TCP_UPSTREAM = 402,
     VAR_HTTPS_PORT = 403,
     VAR_HTTP_ENDPOINT = 404,
     VAR_HTTP_MAX_STREAMS = 405,
     VAR_HTTP_QUERY_BUFFER_SIZE = 406,
     VAR_HTTP_RESPONSE_BUFFER_SIZE = 407,
     VAR_HTTP_NODELAY = 408,
     VAR_HTTP_NOTLS_DOWNSTREAM = 409,
     VAR_STUB_FIRST = 410,
     VAR_MINIMAL_RESPONSES = 411,
     VAR_RRSET_ROUNDROBIN = 412,
     VAR_MAX_UDP_SIZE = 413,
     VAR_DELAY_CLOSE = 414,
     VAR_UDP_CONNECT = 415,
     VAR_UNBLOCK_LAN_ZONES = 416,
     VAR_INSECURE_LAN_ZONES = 417,
     VAR_INFRA_CACHE_MIN_RTT = 418,
     VAR_INFRA_KEEP_PROBING = 419,
     VAR_DNS64_PREFIX = 420,
     VAR_DNS64_SYNTHALL = 421,
     VAR_DNS64_IGNORE_AAAA = 422,
     VAR_DNSTAP = 423,
     VAR_DNSTAP_ENABLE = 424,
     VAR_DNSTAP_SOCKET_PATH = 425,
     VAR_DNSTAP_IP = 426,
     VAR_DNSTAP_TLS = 427,
     VAR_DNSTAP_TLS_SERVER_NAME = 428,
     VAR_DNSTAP_TLS_CERT_BUNDLE = 429,
     VAR_DNSTAP_TLS_CLIENT_KEY_FILE = 430,
     VAR_DNSTAP_TLS_CLIENT_CERT_FILE = 431,
     VAR_DNSTAP_SEND_IDENTITY = 432,
     VAR_DNSTAP_SEND_VERSION = 433,
     VAR_DNSTAP_BIDIRECTIONAL = 434,
     VAR_DNSTAP_IDENTITY = 435,
     VAR_DNSTAP_VERSION = 436,
     VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES = 437,
     VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES = 438,
     VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES = 439,
     VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES = 440,
     VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES = 441,
     VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES = 442,
     VAR_RESPONSE_IP_TAG = 443,
     VAR_RESPONSE_IP = 444,
     VAR_RESPONSE_IP_DATA = 445,
     VAR_HARDEN_ALGO_DOWNGRADE = 446,
     VAR_IP_TRANSPARENT = 447,
     VAR_IP_DSCP = 448,
     VAR_DISABLE_DNSSEC_LAME_CHECK = 449,
     VAR_IP_RATELIMIT = 450,
     VAR_IP_RATELIMIT_SLABS = 451,
     VAR_IP_RATELIMIT_SIZE = 452,
     VAR_RATELIMIT = 453,
     VAR_RATELIMIT_SLABS = 454,
     VAR_RATELIMIT_SIZE = 455,
     VAR_OUTBOUND_MSG_RETRY = 456,
     VAR_RATELIMIT_FOR_DOMAIN = 457,
     VAR_RATELIMIT_BELOW_DOMAIN = 458,
     VAR_IP_RATELIMIT_FACTOR = 459,
     VAR_RATELIMIT_FACTOR = 460,
     VAR_IP_RATELIMIT_BACKOFF = 461,
     VAR_RATELIMIT_BACKOFF = 462,
     VAR_SEND_CLIENT_SUBNET = 463,
     VAR_CLIENT_SUBNET_ZONE = 464,
     VAR_CLIENT_SUBNET_ALWAYS_FORWARD = 465,
     VAR_CLIENT_SUBNET_OPCODE = 466,
     VAR_MAX_CLIENT_SUBNET_IPV4 = 467,
     VAR_MAX_CLIENT_SUBNET_IPV6 = 468,
     VAR_MIN_CLIENT_SUBNET_IPV4 = 469,
     VAR_MIN_CLIENT_SUBNET_IPV6 = 470,
     VAR_MAX_ECS_TREE_SIZE_IPV4 = 471,
     VAR_MAX_ECS_TREE_SIZE_IPV6 = 472,
     VAR_CAPS_WHITELIST = 473,
     VAR_CACHE_MAX_NEGATIVE_TTL = 474,
     VAR_PERMIT_SMALL_HOLDDOWN = 475,
     VAR_QNAME_MINIMISATION = 476,
     VAR_QNAME_MINIMISATION_STRICT = 477,
     VAR_IP_FREEBIND = 478,
     VAR_DEFINE_TAG = 479,
     VAR_LOCAL_ZONE_TAG = 480,
     VAR_ACCESS_CONTROL_TAG = 481,
     VAR_LOCAL_ZONE_OVERRIDE = 482,
     VAR_ACCESS_CONTROL_TAG_ACTION = 483,
     VAR_ACCESS_CONTROL_TAG_DATA = 484,
     VAR_VIEW = 485,
     VAR_ACCESS_CONTROL_VIEW = 486,
     VAR_VIEW_FIRST = 487,
     VAR_SERVE_EXPIRED = 488,
     VAR_SERVE_EXPIRED_TTL = 489,
     VAR_SERVE_EXPIRED_TTL_RESET = 490,
     VAR_SERVE_EXPIRED_REPLY_TTL = 491,
     VAR_SERVE_EXPIRED_CLIENT_TIMEOUT = 492,
     VAR_SERVE_EXPIRED_EDE = 493,
     VAR_SERVE_ORIGINAL_TTL = 494,
     VAR_FAKE_DSA = 495,
     VAR_FAKE_SHA1 = 496,
     VAR_LOG_IDENTITY = 497,
     VAR_HIDE_TRUSTANCHOR = 498,
     VAR_HIDE_HTTP_USER_AGENT = 499,
     VAR_HTTP_USER_AGENT = 500,
     VAR_TRUST_ANCHOR_SIGNALING = 501,
     VAR_AGGRESSIVE_NSEC = 502,
     VAR_USE_SYSTEMD = 503,
     VAR_SHM_ENABLE = 504,
     VAR_SHM_KEY = 505,
     VAR_ROOT_KEY_SENTINEL = 506,
     VAR_DNSCRYPT = 507,
     VAR_DNSCRYPT_ENABLE = 508,
     VAR_DNSCRYPT_PORT = 509,
     VAR_DNSCRYPT_PROVIDER = 510,
     VAR_DNSCRYPT_SECRET_KEY = 511,
     VAR_DNSCRYPT_PROVIDER_CERT = 512,
     VAR_DNSCRYPT_PROVIDER_CERT_ROTATED = 513,
     VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE = 514,
     VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS = 515,
     VAR_DNSCRYPT_NONCE_CACHE_SIZE = 516,
     VAR_DNSCRYPT_NONCE_CACHE_SLABS = 517,
     VAR_PAD_RESPONSES = 518,
     VAR_PAD_RESPONSES_BLOCK_SIZE = 519,
     VAR_PAD_QUERIES = 520,
     VAR_PAD_QUERIES_BLOCK_SIZE = 521,
     VAR_IPSECMOD_ENABLED = 522,
     VAR_IPSECMOD_HOOK = 523,
     VAR_IPSECMOD_IGNORE_BOGUS = 524,
     VAR_IPSECMOD_MAX_TTL = 525,
     VAR_IPSECMOD_WHITELIST = 526,
     VAR_IPSECMOD_STRICT = 527,
     VAR_CACHEDB = 528,
     VAR_CACHEDB_BACKEND = 529,
     VAR_CACHEDB_SECRETSEED = 530,
     VAR_CACHEDB_REDISHOST = 531,
     VAR_CACHEDB_REDISPORT = 532,
     VAR_CACHEDB_REDISTIMEOUT = 533,
     VAR_CACHEDB_REDISEXPIRERECORDS = 534,
     VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM = 535,
     VAR_FOR_UPSTREAM = 536,
     VAR_AUTH_ZONE = 537,
     VAR_ZONEFILE = 538,
     VAR_MASTER = 539,
     VAR_URL = 540,
     VAR_FOR_DOWNSTREAM = 541,
     VAR_FALLBACK_ENABLED = 542,
     VAR_TLS_ADDITIONAL_PORT = 543,
     VAR_LOW_RTT = 544,
     VAR_LOW_RTT_PERMIL = 545,
     VAR_FAST_SERVER_PERMIL = 546,
     VAR_FAST_SERVER_NUM = 547,
     VAR_ALLOW_NOTIFY = 548,
     VAR_TLS_WIN_CERT = 549,
     VAR_TCP_CONNECTION_LIMIT = 550,
     VAR_FORWARD_NO_CACHE = 551,
     VAR_STUB_NO_CACHE = 552,
     VAR_LOG_SERVFAIL = 553,
     VAR_DENY_ANY = 554,
     VAR_UNKNOWN_SERVER_TIME_LIMIT = 555,
     VAR_LOG_TAG_QUERYREPLY = 556,
     VAR_STREAM_WAIT_SIZE = 557,
     VAR_TLS_CIPHERS = 558,
     VAR_TLS_CIPHERSUITES = 559,
     VAR_TLS_USE_SNI = 560,
     VAR_IPSET = 561,
     VAR_IPSET_NAME_V4 = 562,
     VAR_IPSET_NAME_V6 = 563,
     VAR_TLS_SESSION_TICKET_KEYS = 564,
     VAR_RPZ = 565,
     VAR_TAGS = 566,
     VAR_RPZ_ACTION_OVERRIDE = 567,
     VAR_RPZ_CNAME_OVERRIDE = 568,
     VAR_RPZ_LOG = 569,
     VAR_RPZ_LOG_NAME = 570,
     VAR_DYNLIB = 571,
     VAR_DYNLIB_FILE = 572,
     VAR_EDNS_CLIENT_STRING = 573,
     VAR_EDNS_CLIENT_STRING_OPCODE = 574,
     VAR_NSID = 575,
     VAR_ZONEMD_PERMISSIVE_MODE = 576,
     VAR_ZONEMD_CHECK = 577,
     VAR_ZONEMD_REJECT_ABSENCE = 578,
     VAR_RPZ_SIGNAL_NXDOMAIN_RA = 579,
     VAR_INTERFACE_AUTOMATIC_PORTS = 580,
     VAR_DO_EDE = 581
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
#define VAR_TCP_REUSE_TIMEOUT 360
#define VAR_MAX_REUSE_TCP_QUERIES 361
#define VAR_EXTENDED_STATISTICS 362
#define VAR_LOCAL_DATA_PTR 363
#define VAR_JOSTLE_TIMEOUT 364
#define VAR_STUB_PRIME 365
#define VAR_UNWANTED_REPLY_THRESHOLD 366
#define VAR_LOG_TIME_ASCII 367
#define VAR_DOMAIN_INSECURE 368
#define VAR_PYTHON 369
#define VAR_PYTHON_SCRIPT 370
#define VAR_VAL_SIG_SKEW_MIN 371
#define VAR_VAL_SIG_SKEW_MAX 372
#define VAR_VAL_MAX_RESTART 373
#define VAR_CACHE_MIN_TTL 374
#define VAR_VAL_LOG_LEVEL 375
#define VAR_AUTO_TRUST_ANCHOR_FILE 376
#define VAR_KEEP_MISSING 377
#define VAR_ADD_HOLDDOWN 378
#define VAR_DEL_HOLDDOWN 379
#define VAR_SO_RCVBUF 380
#define VAR_EDNS_BUFFER_SIZE 381
#define VAR_PREFETCH 382
#define VAR_PREFETCH_KEY 383
#define VAR_SO_SNDBUF 384
#define VAR_SO_REUSEPORT 385
#define VAR_HARDEN_BELOW_NXDOMAIN 386
#define VAR_IGNORE_CD_FLAG 387
#define VAR_LOG_QUERIES 388
#define VAR_LOG_REPLIES 389
#define VAR_LOG_LOCAL_ACTIONS 390
#define VAR_TCP_UPSTREAM 391
#define VAR_SSL_UPSTREAM 392
#define VAR_TCP_AUTH_QUERY_TIMEOUT 393
#define VAR_SSL_SERVICE_KEY 394
#define VAR_SSL_SERVICE_PEM 395
#define VAR_SSL_PORT 396
#define VAR_FORWARD_FIRST 397
#define VAR_STUB_SSL_UPSTREAM 398
#define VAR_FORWARD_SSL_UPSTREAM 399
#define VAR_TLS_CERT_BUNDLE 400
#define VAR_STUB_TCP_UPSTREAM 401
#define VAR_FORWARD_TCP_UPSTREAM 402
#define VAR_HTTPS_PORT 403
#define VAR_HTTP_ENDPOINT 404
#define VAR_HTTP_MAX_STREAMS 405
#define VAR_HTTP_QUERY_BUFFER_SIZE 406
#define VAR_HTTP_RESPONSE_BUFFER_SIZE 407
#define VAR_HTTP_NODELAY 408
#define VAR_HTTP_NOTLS_DOWNSTREAM 409
#define VAR_STUB_FIRST 410
#define VAR_MINIMAL_RESPONSES 411
#define VAR_RRSET_ROUNDROBIN 412
#define VAR_MAX_UDP_SIZE 413
#define VAR_DELAY_CLOSE 414
#define VAR_UDP_CONNECT 415
#define VAR_UNBLOCK_LAN_ZONES 416
#define VAR_INSECURE_LAN_ZONES 417
#define VAR_INFRA_CACHE_MIN_RTT 418
#define VAR_INFRA_KEEP_PROBING 419
#define VAR_DNS64_PREFIX 420
#define VAR_DNS64_SYNTHALL 421
#define VAR_DNS64_IGNORE_AAAA 422
#define VAR_DNSTAP 423
#define VAR_DNSTAP_ENABLE 424
#define VAR_DNSTAP_SOCKET_PATH 425
#define VAR_DNSTAP_IP 426
#define VAR_DNSTAP_TLS 427
#define VAR_DNSTAP_TLS_SERVER_NAME 428
#define VAR_DNSTAP_TLS_CERT_BUNDLE 429
#define VAR_DNSTAP_TLS_CLIENT_KEY_FILE 430
#define VAR_DNSTAP_TLS_CLIENT_CERT_FILE 431
#define VAR_DNSTAP_SEND_IDENTITY 432
#define VAR_DNSTAP_SEND_VERSION 433
#define VAR_DNSTAP_BIDIRECTIONAL 434
#define VAR_DNSTAP_IDENTITY 435
#define VAR_DNSTAP_VERSION 436
#define VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES 437
#define VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES 438
#define VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES 439
#define VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES 440
#define VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES 441
#define VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES 442
#define VAR_RESPONSE_IP_TAG 443
#define VAR_RESPONSE_IP 444
#define VAR_RESPONSE_IP_DATA 445
#define VAR_HARDEN_ALGO_DOWNGRADE 446
#define VAR_IP_TRANSPARENT 447
#define VAR_IP_DSCP 448
#define VAR_DISABLE_DNSSEC_LAME_CHECK 449
#define VAR_IP_RATELIMIT 450
#define VAR_IP_RATELIMIT_SLABS 451
#define VAR_IP_RATELIMIT_SIZE 452
#define VAR_RATELIMIT 453
#define VAR_RATELIMIT_SLABS 454
#define VAR_RATELIMIT_SIZE 455
#define VAR_OUTBOUND_MSG_RETRY 456
#define VAR_RATELIMIT_FOR_DOMAIN 457
#define VAR_RATELIMIT_BELOW_DOMAIN 458
#define VAR_IP_RATELIMIT_FACTOR 459
#define VAR_RATELIMIT_FACTOR 460
#define VAR_IP_RATELIMIT_BACKOFF 461
#define VAR_RATELIMIT_BACKOFF 462
#define VAR_SEND_CLIENT_SUBNET 463
#define VAR_CLIENT_SUBNET_ZONE 464
#define VAR_CLIENT_SUBNET_ALWAYS_FORWARD 465
#define VAR_CLIENT_SUBNET_OPCODE 466
#define VAR_MAX_CLIENT_SUBNET_IPV4 467
#define VAR_MAX_CLIENT_SUBNET_IPV6 468
#define VAR_MIN_CLIENT_SUBNET_IPV4 469
#define VAR_MIN_CLIENT_SUBNET_IPV6 470
#define VAR_MAX_ECS_TREE_SIZE_IPV4 471
#define VAR_MAX_ECS_TREE_SIZE_IPV6 472
#define VAR_CAPS_WHITELIST 473
#define VAR_CACHE_MAX_NEGATIVE_TTL 474
#define VAR_PERMIT_SMALL_HOLDDOWN 475
#define VAR_QNAME_MINIMISATION 476
#define VAR_QNAME_MINIMISATION_STRICT 477
#define VAR_IP_FREEBIND 478
#define VAR_DEFINE_TAG 479
#define VAR_LOCAL_ZONE_TAG 480
#define VAR_ACCESS_CONTROL_TAG 481
#define VAR_LOCAL_ZONE_OVERRIDE 482
#define VAR_ACCESS_CONTROL_TAG_ACTION 483
#define VAR_ACCESS_CONTROL_TAG_DATA 484
#define VAR_VIEW 485
#define VAR_ACCESS_CONTROL_VIEW 486
#define VAR_VIEW_FIRST 487
#define VAR_SERVE_EXPIRED 488
#define VAR_SERVE_EXPIRED_TTL 489
#define VAR_SERVE_EXPIRED_TTL_RESET 490
#define VAR_SERVE_EXPIRED_REPLY_TTL 491
#define VAR_SERVE_EXPIRED_CLIENT_TIMEOUT 492
#define VAR_SERVE_EXPIRED_EDE 493
#define VAR_SERVE_ORIGINAL_TTL 494
#define VAR_FAKE_DSA 495
#define VAR_FAKE_SHA1 496
#define VAR_LOG_IDENTITY 497
#define VAR_HIDE_TRUSTANCHOR 498
#define VAR_HIDE_HTTP_USER_AGENT 499
#define VAR_HTTP_USER_AGENT 500
#define VAR_TRUST_ANCHOR_SIGNALING 501
#define VAR_AGGRESSIVE_NSEC 502
#define VAR_USE_SYSTEMD 503
#define VAR_SHM_ENABLE 504
#define VAR_SHM_KEY 505
#define VAR_ROOT_KEY_SENTINEL 506
#define VAR_DNSCRYPT 507
#define VAR_DNSCRYPT_ENABLE 508
#define VAR_DNSCRYPT_PORT 509
#define VAR_DNSCRYPT_PROVIDER 510
#define VAR_DNSCRYPT_SECRET_KEY 511
#define VAR_DNSCRYPT_PROVIDER_CERT 512
#define VAR_DNSCRYPT_PROVIDER_CERT_ROTATED 513
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE 514
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS 515
#define VAR_DNSCRYPT_NONCE_CACHE_SIZE 516
#define VAR_DNSCRYPT_NONCE_CACHE_SLABS 517
#define VAR_PAD_RESPONSES 518
#define VAR_PAD_RESPONSES_BLOCK_SIZE 519
#define VAR_PAD_QUERIES 520
#define VAR_PAD_QUERIES_BLOCK_SIZE 521
#define VAR_IPSECMOD_ENABLED 522
#define VAR_IPSECMOD_HOOK 523
#define VAR_IPSECMOD_IGNORE_BOGUS 524
#define VAR_IPSECMOD_MAX_TTL 525
#define VAR_IPSECMOD_WHITELIST 526
#define VAR_IPSECMOD_STRICT 527
#define VAR_CACHEDB 528
#define VAR_CACHEDB_BACKEND 529
#define VAR_CACHEDB_SECRETSEED 530
#define VAR_CACHEDB_REDISHOST 531
#define VAR_CACHEDB_REDISPORT 532
#define VAR_CACHEDB_REDISTIMEOUT 533
#define VAR_CACHEDB_REDISEXPIRERECORDS 534
#define VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM 535
#define VAR_FOR_UPSTREAM 536
#define VAR_AUTH_ZONE 537
#define VAR_ZONEFILE 538
#define VAR_MASTER 539
#define VAR_URL 540
#define VAR_FOR_DOWNSTREAM 541
#define VAR_FALLBACK_ENABLED 542
#define VAR_TLS_ADDITIONAL_PORT 543
#define VAR_LOW_RTT 544
#define VAR_LOW_RTT_PERMIL 545
#define VAR_FAST_SERVER_PERMIL 546
#define VAR_FAST_SERVER_NUM 547
#define VAR_ALLOW_NOTIFY 548
#define VAR_TLS_WIN_CERT 549
#define VAR_TCP_CONNECTION_LIMIT 550
#define VAR_FORWARD_NO_CACHE 551
#define VAR_STUB_NO_CACHE 552
#define VAR_LOG_SERVFAIL 553
#define VAR_DENY_ANY 554
#define VAR_UNKNOWN_SERVER_TIME_LIMIT 555
#define VAR_LOG_TAG_QUERYREPLY 556
#define VAR_STREAM_WAIT_SIZE 557
#define VAR_TLS_CIPHERS 558
#define VAR_TLS_CIPHERSUITES 559
#define VAR_TLS_USE_SNI 560
#define VAR_IPSET 561
#define VAR_IPSET_NAME_V4 562
#define VAR_IPSET_NAME_V6 563
#define VAR_TLS_SESSION_TICKET_KEYS 564
#define VAR_RPZ 565
#define VAR_TAGS 566
#define VAR_RPZ_ACTION_OVERRIDE 567
#define VAR_RPZ_CNAME_OVERRIDE 568
#define VAR_RPZ_LOG 569
#define VAR_RPZ_LOG_NAME 570
#define VAR_DYNLIB 571
#define VAR_DYNLIB_FILE 572
#define VAR_EDNS_CLIENT_STRING 573
#define VAR_EDNS_CLIENT_STRING_OPCODE 574
#define VAR_NSID 575
#define VAR_ZONEMD_PERMISSIVE_MODE 576
#define VAR_ZONEMD_CHECK 577
#define VAR_ZONEMD_REJECT_ABSENCE 578
#define VAR_RPZ_SIGNAL_NXDOMAIN_RA 579
#define VAR_INTERFACE_AUTOMATIC_PORTS 580
#define VAR_DO_EDE 581




/* Copy the first part of user declarations.  */
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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 66 "./util/configparser.y"
{
	char*	str;
}
/* Line 193 of yacc.c.  */
#line 781 "util/configparser.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 794 "util/configparser.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   693

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  327
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  352
/* YYNRULES -- Number of rules.  */
#define YYNRULES  681
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1012

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   581

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
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
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    16,    19,    22,
      25,    28,    31,    34,    37,    40,    43,    46,    48,    50,
      52,    55,    56,    58,    60,    62,    64,    66,    68,    70,
      72,    74,    76,    78,    80,    82,    84,    86,    88,    90,
      92,    94,    96,    98,   100,   102,   104,   106,   108,   110,
     112,   114,   116,   118,   120,   122,   124,   126,   128,   130,
     132,   134,   136,   138,   140,   142,   144,   146,   148,   150,
     152,   154,   156,   158,   160,   162,   164,   166,   168,   170,
     172,   174,   176,   178,   180,   182,   184,   186,   188,   190,
     192,   194,   196,   198,   200,   202,   204,   206,   208,   210,
     212,   214,   216,   218,   220,   222,   224,   226,   228,   230,
     232,   234,   236,   238,   240,   242,   244,   246,   248,   250,
     252,   254,   256,   258,   260,   262,   264,   266,   268,   270,
     272,   274,   276,   278,   280,   282,   284,   286,   288,   290,
     292,   294,   296,   298,   300,   302,   304,   306,   308,   310,
     312,   314,   316,   318,   320,   322,   324,   326,   328,   330,
     332,   334,   336,   338,   340,   342,   344,   346,   348,   350,
     352,   354,   356,   358,   360,   362,   364,   366,   368,   370,
     372,   374,   376,   378,   380,   382,   384,   386,   388,   390,
     392,   394,   396,   398,   400,   402,   404,   406,   408,   410,
     412,   414,   416,   418,   420,   422,   424,   426,   428,   430,
     432,   434,   436,   438,   440,   442,   444,   446,   448,   450,
     452,   454,   456,   458,   460,   462,   464,   466,   468,   470,
     472,   474,   476,   478,   480,   482,   484,   486,   488,   490,
     492,   494,   496,   498,   500,   502,   504,   506,   509,   510,
     512,   514,   516,   518,   520,   522,   524,   526,   528,   531,
     532,   534,   536,   538,   540,   542,   544,   546,   548,   551,
     552,   554,   556,   558,   560,   562,   564,   566,   568,   571,
     572,   574,   576,   578,   580,   582,   584,   586,   588,   590,
     592,   595,   598,   601,   604,   607,   610,   612,   615,   616,
     618,   620,   622,   624,   626,   628,   630,   632,   634,   636,
     638,   640,   643,   646,   649,   652,   655,   658,   661,   664,
     667,   670,   673,   676,   679,   682,   685,   688,   691,   694,
     697,   700,   703,   706,   709,   712,   715,   718,   721,   724,
     727,   730,   733,   736,   739,   742,   745,   748,   751,   754,
     757,   760,   763,   766,   769,   772,   775,   778,   781,   784,
     787,   790,   793,   796,   799,   802,   805,   808,   811,   814,
     817,   820,   823,   826,   829,   832,   835,   838,   841,   844,
     847,   850,   853,   856,   859,   862,   865,   868,   871,   874,
     877,   880,   883,   886,   889,   892,   895,   898,   901,   904,
     907,   910,   913,   916,   919,   922,   925,   928,   931,   934,
     937,   940,   943,   946,   949,   952,   955,   958,   961,   964,
     967,   970,   973,   976,   979,   982,   985,   988,   991,   994,
     997,  1000,  1003,  1006,  1009,  1012,  1015,  1018,  1021,  1024,
    1027,  1030,  1033,  1036,  1039,  1042,  1045,  1048,  1051,  1055,
    1058,  1061,  1064,  1067,  1070,  1073,  1076,  1079,  1082,  1085,
    1088,  1091,  1094,  1097,  1100,  1103,  1106,  1109,  1112,  1115,
    1118,  1121,  1124,  1127,  1130,  1133,  1136,  1139,  1142,  1145,
    1148,  1151,  1155,  1158,  1161,  1164,  1167,  1170,  1173,  1176,
    1179,  1182,  1185,  1189,  1193,  1198,  1203,  1208,  1212,  1216,
    1219,  1222,  1225,  1228,  1231,  1234,  1238,  1242,  1245,  1248,
    1251,  1254,  1257,  1260,  1263,  1266,  1269,  1272,  1275,  1278,
    1281,  1284,  1287,  1290,  1293,  1296,  1299,  1302,  1306,  1309,
    1312,  1315,  1318,  1321,  1324,  1327,  1330,  1333,  1336,  1339,
    1342,  1345,  1348,  1351,  1354,  1357,  1360,  1363,  1366,  1369,
    1372,  1375,  1378,  1381,  1384,  1387,  1390,  1394,  1398,  1402,
    1405,  1408,  1411,  1413,  1416,  1417,  1419,  1421,  1423,  1425,
    1427,  1429,  1431,  1433,  1436,  1439,  1442,  1445,  1448,  1451,
    1454,  1457,  1459,  1462,  1463,  1465,  1467,  1469,  1471,  1473,
    1475,  1477,  1479,  1481,  1483,  1485,  1487,  1489,  1491,  1493,
    1495,  1497,  1499,  1501,  1504,  1507,  1510,  1513,  1516,  1519,
    1522,  1525,  1528,  1531,  1534,  1537,  1540,  1543,  1546,  1549,
    1552,  1555,  1558,  1560,  1563,  1564,  1566,  1569,  1571,  1574,
    1575,  1577,  1580,  1583,  1586,  1590,  1594,  1596,  1599,  1600,
    1602,  1604,  1606,  1608,  1610,  1612,  1614,  1616,  1618,  1620,
    1623,  1626,  1629,  1632,  1635,  1638,  1641,  1644,  1647,  1650,
    1652,  1655,  1656,  1658,  1660,  1662,  1664,  1666,  1668,  1671,
    1674,  1677,  1680,  1683,  1686,  1690,  1692,  1695,  1696,  1698,
    1700,  1703
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     328,     0,    -1,    -1,   328,   329,    -1,   331,   332,    -1,
     334,   335,    -1,   337,   338,    -1,   639,   640,    -1,   606,
     607,    -1,   617,   618,    -1,   340,   341,    -1,   651,   652,
      -1,   664,   665,    -1,   674,   675,    -1,   343,   344,    -1,
     352,   353,    -1,   643,   644,    -1,   330,    -1,    11,    -1,
      12,    -1,   332,   333,    -1,    -1,   355,    -1,   356,    -1,
     362,    -1,   375,    -1,   382,    -1,   383,    -1,   386,    -1,
     387,    -1,   384,    -1,   385,    -1,   388,    -1,   389,    -1,
     390,    -1,   394,    -1,   395,    -1,   373,    -1,   425,    -1,
     426,    -1,   427,    -1,   428,    -1,   429,    -1,   457,    -1,
     458,    -1,   459,    -1,   465,    -1,   466,    -1,   378,    -1,
     467,    -1,   468,    -1,   471,    -1,   469,    -1,   470,    -1,
     474,    -1,   475,    -1,   476,    -1,   490,    -1,   440,    -1,
     441,    -1,   444,    -1,   445,    -1,   443,    -1,   446,    -1,
     477,    -1,   493,    -1,   434,    -1,   436,    -1,   494,    -1,
     501,    -1,   502,    -1,   503,    -1,   379,    -1,   456,    -1,
     522,    -1,   523,    -1,   435,    -1,   516,    -1,   418,    -1,
     374,    -1,   430,    -1,   491,    -1,   498,    -1,   478,    -1,
     492,    -1,   525,    -1,   526,    -1,   380,    -1,   357,    -1,
     417,    -1,   482,    -1,   358,    -1,   376,    -1,   377,    -1,
     431,    -1,   432,    -1,   524,    -1,   480,    -1,   484,    -1,
     485,    -1,   359,    -1,   527,    -1,   460,    -1,   489,    -1,
     419,    -1,   439,    -1,   495,    -1,   496,    -1,   497,    -1,
     500,    -1,   515,    -1,   433,    -1,   518,    -1,   519,    -1,
     520,    -1,   448,    -1,   455,    -1,   486,    -1,   487,    -1,
     449,    -1,   479,    -1,   505,    -1,   420,    -1,   421,    -1,
     396,    -1,   398,    -1,   424,    -1,   399,    -1,   400,    -1,
     401,    -1,   409,    -1,   410,    -1,   411,    -1,   412,    -1,
     413,    -1,   414,    -1,   415,    -1,   528,    -1,   529,    -1,
     531,    -1,   450,    -1,   461,    -1,   462,    -1,   463,    -1,
     464,    -1,   532,    -1,   533,    -1,   534,    -1,   472,    -1,
     481,    -1,   451,    -1,   543,    -1,   544,    -1,   453,    -1,
     473,    -1,   547,    -1,   548,    -1,   545,    -1,   546,    -1,
     549,    -1,   550,    -1,   552,    -1,   551,    -1,   554,    -1,
     553,    -1,   555,    -1,   363,    -1,   364,    -1,   365,    -1,
     366,    -1,   367,    -1,   368,    -1,   369,    -1,   370,    -1,
     371,    -1,   372,    -1,   483,    -1,   499,    -1,   521,    -1,
     559,    -1,   452,    -1,   535,    -1,   536,    -1,   647,    -1,
     537,    -1,   540,    -1,   538,    -1,   539,    -1,   541,    -1,
     560,    -1,   561,    -1,   562,    -1,   563,    -1,   564,    -1,
     506,    -1,   507,    -1,   508,    -1,   509,    -1,   510,    -1,
     511,    -1,   512,    -1,   513,    -1,   648,    -1,   416,    -1,
     542,    -1,   649,    -1,   650,    -1,   360,    -1,   361,    -1,
     514,    -1,   442,    -1,   437,    -1,   438,    -1,   565,    -1,
     567,    -1,   566,    -1,   568,    -1,   569,    -1,   570,    -1,
     397,    -1,   504,    -1,   402,    -1,   404,    -1,   556,    -1,
     558,    -1,   557,    -1,   403,    -1,   673,    -1,   423,    -1,
     488,    -1,   530,    -1,   422,    -1,   454,    -1,   405,    -1,
     406,    -1,   407,    -1,   408,    -1,   571,    -1,   572,    -1,
     447,    -1,   517,    -1,   391,    -1,   392,    -1,   393,    -1,
     381,    -1,   573,    -1,    46,    -1,   335,   336,    -1,    -1,
     574,    -1,   575,    -1,   576,    -1,   581,    -1,   577,    -1,
     578,    -1,   579,    -1,   580,    -1,    52,    -1,   338,   339,
      -1,    -1,   582,    -1,   583,    -1,   584,    -1,   585,    -1,
     586,    -1,   587,    -1,   588,    -1,   230,    -1,   341,   342,
      -1,    -1,   599,    -1,   600,    -1,   603,    -1,   605,    -1,
     601,    -1,   602,    -1,   604,    -1,   282,    -1,   344,   345,
      -1,    -1,   589,    -1,   590,    -1,   591,    -1,   592,    -1,
     596,    -1,   597,    -1,   598,    -1,   593,    -1,   594,    -1,
     595,    -1,   311,    10,    -1,   312,    10,    -1,   313,    10,
      -1,   314,    10,    -1,   315,    10,    -1,   324,    10,    -1,
     310,    -1,   353,   354,    -1,    -1,   589,    -1,   590,    -1,
     346,    -1,   591,    -1,   592,    -1,   593,    -1,   347,    -1,
     348,    -1,   349,    -1,   350,    -1,   351,    -1,   596,    -1,
      14,    10,    -1,    13,    10,    -1,    84,    10,    -1,    87,
      10,    -1,   107,    10,    -1,   249,    10,    -1,   250,    10,
      -1,    15,    10,    -1,   208,    10,    -1,   209,    10,    -1,
     210,    10,    -1,   211,    10,    -1,   212,    10,    -1,   213,
      10,    -1,   214,    10,    -1,   215,    10,    -1,   216,    10,
      -1,   217,    10,    -1,    17,    10,    -1,    75,    10,    -1,
      16,    10,    -1,    88,    10,    -1,    89,    10,    -1,    39,
      10,    -1,    68,    10,    -1,    83,    10,    -1,   325,    10,
      -1,    19,    10,    -1,    20,    10,    -1,    22,    10,    -1,
      23,    10,    -1,    18,    10,    -1,    21,    10,    -1,    24,
      10,    -1,    25,    10,    -1,    26,    10,    -1,   106,    10,
      -1,   105,    10,    -1,   138,    10,    -1,    27,    10,    -1,
      28,    10,    -1,   136,    10,    -1,   280,    10,    -1,   137,
      10,    -1,   139,    10,    -1,   140,    10,    -1,   141,    10,
      -1,   145,    10,    -1,   294,    10,    -1,   288,    10,    -1,
     303,    10,    -1,   304,    10,    -1,   309,    10,    -1,   305,
      10,    -1,   148,    10,    -1,   149,    10,    -1,   150,    10,
      -1,   151,    10,    -1,   152,    10,    -1,   153,    10,    -1,
     154,    10,    -1,   248,    10,    -1,    85,    10,    -1,    74,
      10,    -1,   112,    10,    -1,   133,    10,    -1,   134,    10,
      -1,   301,    10,    -1,   298,    10,    -1,   135,    10,    -1,
      29,    10,    -1,    30,    10,    -1,    31,    10,    -1,    32,
      10,    -1,    33,    10,    -1,    76,    10,    -1,    90,    10,
      -1,    91,    10,    -1,   121,    10,    -1,    62,    10,    -1,
      72,    10,    -1,    63,    10,    -1,   246,    10,    -1,   251,
      10,    -1,   113,    10,    -1,    56,    10,    -1,    57,    10,
      -1,   243,    10,    -1,   244,    10,    -1,    58,    10,    -1,
      59,    10,    -1,   245,    10,    -1,   320,    10,    -1,   125,
      10,    -1,   129,    10,    -1,   130,    10,    -1,   192,    10,
      -1,   223,    10,    -1,   193,    10,    -1,   302,    10,    -1,
     126,    10,    -1,    69,    10,    -1,    34,    10,    -1,    35,
      10,    -1,    36,    10,    -1,   109,    10,    -1,   159,    10,
      -1,   160,    10,    -1,   161,    10,    -1,   162,    10,    -1,
      37,    10,    -1,    38,    10,    -1,    40,    10,    -1,    41,
      10,    -1,    43,    10,    -1,    44,    10,    -1,    42,    10,
      -1,   163,    10,    -1,   164,    10,    -1,    49,    10,    -1,
      50,    10,    -1,    51,    10,    -1,    60,    10,    -1,    79,
      10,    -1,   131,    10,    -1,    93,    10,    -1,   191,    10,
      -1,    86,    10,    -1,   218,    10,    -1,    94,    10,    -1,
      95,    10,    -1,   127,    10,    -1,   128,    10,    -1,   299,
      10,    -1,   111,    10,    -1,    55,    10,    -1,    77,    10,
      -1,    80,    10,    10,    -1,    61,    10,    -1,    64,    10,
      -1,   116,    10,    -1,   117,    10,    -1,   118,    10,    -1,
      78,    10,    -1,   219,    10,    -1,   119,    10,    -1,    65,
      10,    -1,    66,    10,    -1,    67,    10,    -1,   247,    10,
      -1,   132,    10,    -1,   233,    10,    -1,   234,    10,    -1,
     235,    10,    -1,   236,    10,    -1,   237,    10,    -1,   238,
      10,    -1,   239,    10,    -1,   240,    10,    -1,   241,    10,
      -1,   120,    10,    -1,    73,    10,    -1,   321,    10,    -1,
     123,    10,    -1,   124,    10,    -1,   122,    10,    -1,   220,
      10,    -1,    70,    10,    -1,    71,    10,    -1,    92,    10,
      -1,    81,    10,    10,    -1,    82,    10,    -1,   108,    10,
      -1,   156,    10,    -1,   157,    10,    -1,   300,    10,    -1,
     158,    10,    -1,   165,    10,    -1,   166,    10,    -1,   167,
      10,    -1,   224,    10,    -1,   225,    10,    10,    -1,   226,
      10,    10,    -1,   228,    10,    10,    10,    -1,   229,    10,
      10,    10,    -1,   227,    10,    10,    10,    -1,   231,    10,
      10,    -1,   188,    10,    10,    -1,   195,    10,    -1,   198,
      10,    -1,   197,    10,    -1,   200,    10,    -1,   196,    10,
      -1,   199,    10,    -1,   202,    10,    10,    -1,   203,    10,
      10,    -1,   204,    10,    -1,   205,    10,    -1,   206,    10,
      -1,   207,    10,    -1,   201,    10,    -1,   289,    10,    -1,
     292,    10,    -1,   291,    10,    -1,   221,    10,    -1,   222,
      10,    -1,   263,    10,    -1,   264,    10,    -1,   265,    10,
      -1,   266,    10,    -1,   267,    10,    -1,   269,    10,    -1,
     268,    10,    -1,   270,    10,    -1,   271,    10,    -1,   272,
      10,    -1,   318,    10,    10,    -1,   319,    10,    -1,   326,
      10,    -1,    45,    10,    -1,    47,    10,    -1,    48,    10,
      -1,   155,    10,    -1,   297,    10,    -1,   143,    10,    -1,
     146,    10,    -1,   110,    10,    -1,    45,    10,    -1,    53,
      10,    -1,    54,    10,    -1,   142,    10,    -1,   296,    10,
      -1,   144,    10,    -1,   147,    10,    -1,    45,    10,    -1,
     283,    10,    -1,   284,    10,    -1,   285,    10,    -1,   293,
      10,    -1,   322,    10,    -1,   323,    10,    -1,   286,    10,
      -1,   281,    10,    -1,   287,    10,    -1,    45,    10,    -1,
      81,    10,    10,    -1,   189,    10,    10,    -1,   190,    10,
      10,    -1,    82,    10,    -1,   108,    10,    -1,   232,    10,
      -1,    96,    -1,   607,   608,    -1,    -1,   609,    -1,   611,
      -1,   610,    -1,   613,    -1,   614,    -1,   615,    -1,   616,
      -1,   612,    -1,    97,    10,    -1,    99,    10,    -1,    98,
      10,    -1,   104,    10,    -1,   100,    10,    -1,   101,    10,
      -1,   102,    10,    -1,   103,    10,    -1,   168,    -1,   618,
     619,    -1,    -1,   620,    -1,   622,    -1,   621,    -1,   623,
      -1,   624,    -1,   625,    -1,   626,    -1,   627,    -1,   628,
      -1,   629,    -1,   630,    -1,   631,    -1,   632,    -1,   633,
      -1,   634,    -1,   635,    -1,   636,    -1,   637,    -1,   638,
      -1,   169,    10,    -1,   179,    10,    -1,   170,    10,    -1,
     171,    10,    -1,   172,    10,    -1,   173,    10,    -1,   174,
      10,    -1,   175,    10,    -1,   176,    10,    -1,   177,    10,
      -1,   178,    10,    -1,   180,    10,    -1,   181,    10,    -1,
     182,    10,    -1,   183,    10,    -1,   184,    10,    -1,   185,
      10,    -1,   186,    10,    -1,   187,    10,    -1,   114,    -1,
     640,   641,    -1,    -1,   642,    -1,   115,    10,    -1,   316,
      -1,   644,   645,    -1,    -1,   646,    -1,   317,    10,    -1,
     194,    10,    -1,   242,    10,    -1,   189,    10,    10,    -1,
     190,    10,    10,    -1,   252,    -1,   652,   653,    -1,    -1,
     654,    -1,   655,    -1,   656,    -1,   659,    -1,   657,    -1,
     658,    -1,   660,    -1,   661,    -1,   662,    -1,   663,    -1,
     253,    10,    -1,   254,    10,    -1,   255,    10,    -1,   257,
      10,    -1,   258,    10,    -1,   256,    10,    -1,   259,    10,
      -1,   260,    10,    -1,   261,    10,    -1,   262,    10,    -1,
     273,    -1,   665,   666,    -1,    -1,   667,    -1,   668,    -1,
     669,    -1,   670,    -1,   671,    -1,   672,    -1,   274,    10,
      -1,   275,    10,    -1,   276,    10,    -1,   277,    10,    -1,
     278,    10,    -1,   279,    10,    -1,   295,    10,    10,    -1,
     306,    -1,   675,   676,    -1,    -1,   677,    -1,   678,    -1,
     307,    10,    -1,   308,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   195,   195,   195,   196,   196,   197,   197,   198,   198,
     198,   199,   199,   200,   200,   201,   201,   202,   204,   210,
     215,   216,   217,   217,   217,   218,   218,   219,   219,   219,
     220,   220,   221,   221,   221,   222,   222,   223,   223,   223,
     224,   224,   224,   225,   225,   226,   226,   227,   227,   228,
     228,   229,   229,   230,   230,   231,   231,   232,   232,   233,
     233,   233,   234,   234,   235,   235,   235,   236,   236,   236,
     237,   237,   238,   238,   239,   239,   240,   240,   241,   241,
     241,   242,   242,   243,   243,   244,   244,   244,   245,   245,
     246,   246,   247,   247,   248,   248,   248,   249,   249,   250,
     250,   251,   251,   252,   252,   253,   253,   254,   254,   255,
     255,   256,   256,   257,   257,   257,   258,   258,   258,   259,
     259,   259,   260,   260,   260,   260,   261,   262,   262,   262,
     263,   263,   263,   264,   264,   265,   265,   266,   266,   266,
     267,   267,   267,   268,   268,   269,   269,   269,   270,   270,
     271,   271,   271,   272,   272,   273,   273,   274,   274,   275,
     276,   276,   277,   277,   278,   278,   279,   279,   280,   280,
     281,   281,   282,   282,   283,   283,   284,   284,   285,   285,
     286,   286,   286,   287,   287,   288,   288,   289,   289,   290,
     291,   291,   292,   292,   293,   294,   294,   295,   295,   296,
     296,   296,   297,   297,   298,   298,   298,   299,   299,   299,
     300,   300,   301,   302,   302,   303,   303,   304,   304,   305,
     305,   306,   306,   306,   307,   307,   307,   308,   308,   308,
     309,   309,   310,   310,   311,   311,   312,   312,   313,   313,
     314,   314,   315,   315,   316,   316,   319,   332,   333,   334,
     334,   334,   334,   334,   335,   335,   335,   337,   350,   351,
     352,   352,   352,   352,   353,   353,   353,   355,   370,   371,
     372,   372,   372,   372,   373,   373,   373,   375,   395,   396,
     397,   397,   397,   397,   398,   398,   398,   399,   399,   399,
     402,   421,   438,   446,   456,   463,   473,   491,   492,   493,
     493,   493,   493,   493,   494,   494,   494,   495,   495,   495,
     495,   497,   506,   515,   526,   535,   544,   553,   564,   573,
     585,   599,   614,   625,   642,   659,   676,   693,   708,   723,
     736,   751,   760,   769,   778,   787,   796,   805,   812,   821,
     830,   839,   848,   857,   866,   875,   884,   897,   908,   919,
     930,   939,   952,   961,   970,   979,   986,   993,  1002,  1009,
    1018,  1026,  1033,  1040,  1048,  1057,  1065,  1081,  1089,  1097,
    1105,  1113,  1121,  1130,  1139,  1153,  1162,  1171,  1180,  1189,
    1198,  1207,  1214,  1221,  1247,  1255,  1262,  1269,  1276,  1283,
    1291,  1299,  1307,  1314,  1325,  1336,  1343,  1352,  1361,  1370,
    1379,  1386,  1393,  1400,  1416,  1424,  1432,  1442,  1452,  1462,
    1476,  1484,  1497,  1508,  1516,  1529,  1538,  1547,  1556,  1565,
    1575,  1585,  1593,  1606,  1615,  1623,  1632,  1640,  1653,  1662,
    1672,  1679,  1689,  1699,  1709,  1719,  1729,  1739,  1749,  1759,
    1766,  1773,  1780,  1789,  1798,  1807,  1816,  1823,  1833,  1853,
    1860,  1878,  1891,  1904,  1917,  1926,  1935,  1944,  1953,  1963,
    1973,  1984,  1993,  2002,  2011,  2020,  2029,  2038,  2047,  2056,
    2069,  2082,  2091,  2098,  2107,  2116,  2125,  2134,  2143,  2151,
    2164,  2172,  2227,  2234,  2249,  2259,  2269,  2276,  2283,  2290,
    2299,  2307,  2321,  2342,  2363,  2375,  2387,  2399,  2408,  2429,
    2438,  2447,  2455,  2463,  2476,  2489,  2504,  2519,  2528,  2537,
    2547,  2557,  2566,  2572,  2581,  2590,  2600,  2610,  2620,  2629,
    2639,  2648,  2661,  2674,  2686,  2700,  2712,  2726,  2735,  2746,
    2755,  2765,  2772,  2779,  2788,  2797,  2807,  2817,  2827,  2837,
    2844,  2851,  2860,  2869,  2879,  2889,  2899,  2906,  2913,  2920,
    2928,  2938,  2948,  2958,  2968,  2978,  2988,  3044,  3054,  3062,
    3070,  3085,  3094,  3099,  3100,  3101,  3101,  3101,  3102,  3102,
    3102,  3103,  3103,  3105,  3115,  3124,  3131,  3138,  3145,  3152,
    3159,  3166,  3171,  3172,  3173,  3173,  3173,  3174,  3174,  3174,
    3175,  3176,  3176,  3177,  3177,  3178,  3178,  3179,  3180,  3181,
    3182,  3183,  3184,  3186,  3195,  3205,  3212,  3219,  3228,  3235,
    3242,  3249,  3256,  3265,  3274,  3281,  3288,  3298,  3308,  3318,
    3328,  3338,  3348,  3353,  3354,  3355,  3357,  3363,  3368,  3369,
    3370,  3372,  3378,  3388,  3395,  3404,  3412,  3417,  3418,  3420,
    3420,  3420,  3421,  3421,  3422,  3423,  3424,  3425,  3426,  3428,
    3438,  3447,  3454,  3463,  3470,  3479,  3487,  3500,  3508,  3521,
    3526,  3527,  3528,  3528,  3529,  3529,  3529,  3530,  3532,  3544,
    3556,  3568,  3583,  3596,  3609,  3620,  3625,  3626,  3627,  3627,
    3629,  3644
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "VAR_CONTROL_USE_CERT", "VAR_TCP_REUSE_TIMEOUT",
  "VAR_MAX_REUSE_TCP_QUERIES", "VAR_EXTENDED_STATISTICS",
  "VAR_LOCAL_DATA_PTR", "VAR_JOSTLE_TIMEOUT", "VAR_STUB_PRIME",
  "VAR_UNWANTED_REPLY_THRESHOLD", "VAR_LOG_TIME_ASCII",
  "VAR_DOMAIN_INSECURE", "VAR_PYTHON", "VAR_PYTHON_SCRIPT",
  "VAR_VAL_SIG_SKEW_MIN", "VAR_VAL_SIG_SKEW_MAX", "VAR_VAL_MAX_RESTART",
  "VAR_CACHE_MIN_TTL", "VAR_VAL_LOG_LEVEL", "VAR_AUTO_TRUST_ANCHOR_FILE",
  "VAR_KEEP_MISSING", "VAR_ADD_HOLDDOWN", "VAR_DEL_HOLDDOWN",
  "VAR_SO_RCVBUF", "VAR_EDNS_BUFFER_SIZE", "VAR_PREFETCH",
  "VAR_PREFETCH_KEY", "VAR_SO_SNDBUF", "VAR_SO_REUSEPORT",
  "VAR_HARDEN_BELOW_NXDOMAIN", "VAR_IGNORE_CD_FLAG", "VAR_LOG_QUERIES",
  "VAR_LOG_REPLIES", "VAR_LOG_LOCAL_ACTIONS", "VAR_TCP_UPSTREAM",
  "VAR_SSL_UPSTREAM", "VAR_TCP_AUTH_QUERY_TIMEOUT", "VAR_SSL_SERVICE_KEY",
  "VAR_SSL_SERVICE_PEM", "VAR_SSL_PORT", "VAR_FORWARD_FIRST",
  "VAR_STUB_SSL_UPSTREAM", "VAR_FORWARD_SSL_UPSTREAM",
  "VAR_TLS_CERT_BUNDLE", "VAR_STUB_TCP_UPSTREAM",
  "VAR_FORWARD_TCP_UPSTREAM", "VAR_HTTPS_PORT", "VAR_HTTP_ENDPOINT",
  "VAR_HTTP_MAX_STREAMS", "VAR_HTTP_QUERY_BUFFER_SIZE",
  "VAR_HTTP_RESPONSE_BUFFER_SIZE", "VAR_HTTP_NODELAY",
  "VAR_HTTP_NOTLS_DOWNSTREAM", "VAR_STUB_FIRST", "VAR_MINIMAL_RESPONSES",
  "VAR_RRSET_ROUNDROBIN", "VAR_MAX_UDP_SIZE", "VAR_DELAY_CLOSE",
  "VAR_UDP_CONNECT", "VAR_UNBLOCK_LAN_ZONES", "VAR_INSECURE_LAN_ZONES",
  "VAR_INFRA_CACHE_MIN_RTT", "VAR_INFRA_KEEP_PROBING", "VAR_DNS64_PREFIX",
  "VAR_DNS64_SYNTHALL", "VAR_DNS64_IGNORE_AAAA", "VAR_DNSTAP",
  "VAR_DNSTAP_ENABLE", "VAR_DNSTAP_SOCKET_PATH", "VAR_DNSTAP_IP",
  "VAR_DNSTAP_TLS", "VAR_DNSTAP_TLS_SERVER_NAME",
  "VAR_DNSTAP_TLS_CERT_BUNDLE", "VAR_DNSTAP_TLS_CLIENT_KEY_FILE",
  "VAR_DNSTAP_TLS_CLIENT_CERT_FILE", "VAR_DNSTAP_SEND_IDENTITY",
  "VAR_DNSTAP_SEND_VERSION", "VAR_DNSTAP_BIDIRECTIONAL",
  "VAR_DNSTAP_IDENTITY", "VAR_DNSTAP_VERSION",
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
  "VAR_OUTBOUND_MSG_RETRY", "VAR_RATELIMIT_FOR_DOMAIN",
  "VAR_RATELIMIT_BELOW_DOMAIN", "VAR_IP_RATELIMIT_FACTOR",
  "VAR_RATELIMIT_FACTOR", "VAR_IP_RATELIMIT_BACKOFF",
  "VAR_RATELIMIT_BACKOFF", "VAR_SEND_CLIENT_SUBNET",
  "VAR_CLIENT_SUBNET_ZONE", "VAR_CLIENT_SUBNET_ALWAYS_FORWARD",
  "VAR_CLIENT_SUBNET_OPCODE", "VAR_MAX_CLIENT_SUBNET_IPV4",
  "VAR_MAX_CLIENT_SUBNET_IPV6", "VAR_MIN_CLIENT_SUBNET_IPV4",
  "VAR_MIN_CLIENT_SUBNET_IPV6", "VAR_MAX_ECS_TREE_SIZE_IPV4",
  "VAR_MAX_ECS_TREE_SIZE_IPV6", "VAR_CAPS_WHITELIST",
  "VAR_CACHE_MAX_NEGATIVE_TTL", "VAR_PERMIT_SMALL_HOLDDOWN",
  "VAR_QNAME_MINIMISATION", "VAR_QNAME_MINIMISATION_STRICT",
  "VAR_IP_FREEBIND", "VAR_DEFINE_TAG", "VAR_LOCAL_ZONE_TAG",
  "VAR_ACCESS_CONTROL_TAG", "VAR_LOCAL_ZONE_OVERRIDE",
  "VAR_ACCESS_CONTROL_TAG_ACTION", "VAR_ACCESS_CONTROL_TAG_DATA",
  "VAR_VIEW", "VAR_ACCESS_CONTROL_VIEW", "VAR_VIEW_FIRST",
  "VAR_SERVE_EXPIRED", "VAR_SERVE_EXPIRED_TTL",
  "VAR_SERVE_EXPIRED_TTL_RESET", "VAR_SERVE_EXPIRED_REPLY_TTL",
  "VAR_SERVE_EXPIRED_CLIENT_TIMEOUT", "VAR_SERVE_EXPIRED_EDE",
  "VAR_SERVE_ORIGINAL_TTL", "VAR_FAKE_DSA", "VAR_FAKE_SHA1",
  "VAR_LOG_IDENTITY", "VAR_HIDE_TRUSTANCHOR", "VAR_HIDE_HTTP_USER_AGENT",
  "VAR_HTTP_USER_AGENT", "VAR_TRUST_ANCHOR_SIGNALING",
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
  "VAR_ZONEMD_PERMISSIVE_MODE", "VAR_ZONEMD_CHECK",
  "VAR_ZONEMD_REJECT_ABSENCE", "VAR_RPZ_SIGNAL_NXDOMAIN_RA",
  "VAR_INTERFACE_AUTOMATIC_PORTS", "VAR_DO_EDE", "$accept", "toplevelvars",
  "toplevelvar", "force_toplevel", "serverstart", "contents_server",
  "content_server", "stubstart", "contents_stub", "content_stub",
  "forwardstart", "contents_forward", "content_forward", "viewstart",
  "contents_view", "content_view", "authstart", "contents_auth",
  "content_auth", "rpz_tag", "rpz_action_override", "rpz_cname_override",
  "rpz_log", "rpz_log_name", "rpz_signal_nxdomain_ra", "rpzstart",
  "contents_rpz", "content_rpz", "server_num_threads", "server_verbosity",
  "server_statistics_interval", "server_statistics_cumulative",
  "server_extended_statistics", "server_shm_enable", "server_shm_key",
  "server_port", "server_send_client_subnet", "server_client_subnet_zone",
  "server_client_subnet_always_forward", "server_client_subnet_opcode",
  "server_max_client_subnet_ipv4", "server_max_client_subnet_ipv6",
  "server_min_client_subnet_ipv4", "server_min_client_subnet_ipv6",
  "server_max_ecs_tree_size_ipv4", "server_max_ecs_tree_size_ipv6",
  "server_interface", "server_outgoing_interface", "server_outgoing_range",
  "server_outgoing_port_permit", "server_outgoing_port_avoid",
  "server_outgoing_num_tcp", "server_incoming_num_tcp",
  "server_interface_automatic", "server_interface_automatic_ports",
  "server_do_ip4", "server_do_ip6", "server_do_udp", "server_do_tcp",
  "server_prefer_ip4", "server_prefer_ip6", "server_tcp_mss",
  "server_outgoing_tcp_mss", "server_tcp_idle_timeout",
  "server_max_reuse_tcp_queries", "server_tcp_reuse_timeout",
  "server_tcp_auth_query_timeout", "server_tcp_keepalive",
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
  "server_hide_http_user_agent", "server_identity", "server_version",
  "server_http_user_agent", "server_nsid", "server_so_rcvbuf",
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
  "server_val_max_restart", "server_cache_max_ttl",
  "server_cache_max_negative_ttl", "server_cache_min_ttl",
  "server_bogus_ttl", "server_val_clean_additional",
  "server_val_permissive_mode", "server_aggressive_nsec",
  "server_ignore_cd_flag", "server_serve_expired",
  "server_serve_expired_ttl", "server_serve_expired_ttl_reset",
  "server_serve_expired_reply_ttl", "server_serve_expired_client_timeout",
  "server_serve_expired_ede", "server_serve_original_ttl",
  "server_fake_dsa", "server_fake_sha1", "server_val_log_level",
  "server_val_nsec3_keysize_iterations", "server_zonemd_permissive_mode",
  "server_add_holddown", "server_del_holddown", "server_keep_missing",
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
  "server_ratelimit_factor", "server_ip_ratelimit_backoff",
  "server_ratelimit_backoff", "server_outbound_msg_retry",
  "server_low_rtt", "server_fast_server_num", "server_fast_server_permil",
  "server_qname_minimisation", "server_qname_minimisation_strict",
  "server_pad_responses", "server_pad_responses_block_size",
  "server_pad_queries", "server_pad_queries_block_size",
  "server_ipsecmod_enabled", "server_ipsecmod_ignore_bogus",
  "server_ipsecmod_hook", "server_ipsecmod_max_ttl",
  "server_ipsecmod_whitelist", "server_ipsecmod_strict",
  "server_edns_client_string", "server_edns_client_string_opcode",
  "server_do_ede", "stub_name", "stub_host", "stub_addr", "stub_first",
  "stub_no_cache", "stub_ssl_upstream", "stub_tcp_upstream", "stub_prime",
  "forward_name", "forward_host", "forward_addr", "forward_first",
  "forward_no_cache", "forward_ssl_upstream", "forward_tcp_upstream",
  "auth_name", "auth_zonefile", "auth_master", "auth_url",
  "auth_allow_notify", "auth_zonemd_check", "auth_zonemd_reject_absence",
  "auth_for_downstream", "auth_for_upstream", "auth_fallback_enabled",
  "view_name", "view_local_zone", "view_response_ip",
  "view_response_ip_data", "view_local_data", "view_local_data_ptr",
  "view_first", "rcstart", "contents_rc", "content_rc",
  "rc_control_enable", "rc_control_port", "rc_control_interface",
  "rc_control_use_cert", "rc_server_key_file", "rc_server_cert_file",
  "rc_control_key_file", "rc_control_cert_file", "dtstart", "contents_dt",
  "content_dt", "dt_dnstap_enable", "dt_dnstap_bidirectional",
  "dt_dnstap_socket_path", "dt_dnstap_ip", "dt_dnstap_tls",
  "dt_dnstap_tls_server_name", "dt_dnstap_tls_cert_bundle",
  "dt_dnstap_tls_client_key_file", "dt_dnstap_tls_client_cert_file",
  "dt_dnstap_send_identity", "dt_dnstap_send_version",
  "dt_dnstap_identity", "dt_dnstap_version",
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
  "content_ipset", "ipset_name_v4", "ipset_name_v6", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   327,   328,   328,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   329,   329,   329,   330,   331,
     332,   332,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   334,   335,   335,   336,
     336,   336,   336,   336,   336,   336,   336,   337,   338,   338,
     339,   339,   339,   339,   339,   339,   339,   340,   341,   341,
     342,   342,   342,   342,   342,   342,   342,   343,   344,   344,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   353,   354,
     354,   354,   354,   354,   354,   354,   354,   354,   354,   354,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,   530,   531,   532,   533,
     534,   535,   536,   537,   538,   539,   540,   541,   542,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,   581,   582,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   599,   600,   601,   602,   603,
     604,   605,   606,   607,   607,   608,   608,   608,   608,   608,
     608,   608,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   618,   619,   619,   619,   619,   619,   619,
     619,   619,   619,   619,   619,   619,   619,   619,   619,   619,
     619,   619,   619,   620,   621,   622,   623,   624,   625,   626,
     627,   628,   629,   630,   631,   632,   633,   634,   635,   636,
     637,   638,   639,   640,   640,   641,   642,   643,   644,   644,
     645,   646,   647,   648,   649,   650,   651,   652,   652,   653,
     653,   653,   653,   653,   653,   653,   653,   653,   653,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   665,   666,   666,   666,   666,   666,   666,   667,   668,
     669,   670,   671,   672,   673,   674,   675,   675,   676,   676,
     677,   678
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
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
       1,     1,     1,     1,     1,     1,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     3,     4,     4,     4,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     3,     3,     2,
       2,     2,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     2,     0,     1,     2,     1,     2,     0,
       1,     2,     2,     2,     3,     3,     1,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       2,     0,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     3,     1,     2,     0,     1,     1,
       2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,    18,    19,   246,   257,   562,   622,   581,
     267,   636,   659,   277,   675,   296,   627,     3,    17,    21,
     248,   259,   269,   279,   298,   564,   583,   624,   629,   638,
     661,   677,     4,     5,     6,    10,    14,    15,     8,     9,
       7,    16,    11,    12,    13,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    20,
      22,    23,    88,    91,   100,   207,   208,    24,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,    37,    79,
      25,    92,    93,    48,    72,    87,   244,    26,    27,    30,
      31,    28,    29,    32,    33,    34,   241,   242,   243,    35,
      36,   124,   219,   125,   127,   128,   129,   221,   226,   222,
     233,   234,   235,   236,   130,   131,   132,   133,   134,   135,
     136,   203,    89,    78,   104,   122,   123,   231,   228,   126,
      38,    39,    40,    41,    42,    80,    94,    95,   111,    66,
      76,    67,   211,   212,   105,    58,    59,   210,    62,    60,
      61,    63,   239,   115,   119,   140,   150,   180,   153,   232,
     116,    73,    43,    44,    45,   102,   141,   142,   143,   144,
      46,    47,    49,    50,    52,    53,    51,   148,   154,    54,
      55,    56,    64,    83,   120,    97,   149,    90,   176,    98,
      99,   117,   118,   229,   103,    57,    81,    84,    65,    68,
     106,   107,   108,    82,   177,   109,    69,    70,    71,   220,
     121,   194,   195,   196,   197,   198,   199,   200,   201,   209,
     110,    77,   240,   112,   113,   114,   178,    74,    75,    96,
      85,    86,   101,   137,   138,   230,   139,   145,   146,   147,
     181,   182,   184,   186,   187,   185,   188,   204,   151,   152,
     157,   158,   155,   156,   159,   160,   162,   161,   164,   163,
     165,   223,   225,   224,   179,   189,   190,   191,   192,   193,
     213,   215,   214,   216,   217,   218,   237,   238,   245,   183,
     202,   205,   206,   227,     0,     0,     0,     0,     0,     0,
       0,     0,   247,   249,   250,   251,   253,   254,   255,   256,
     252,     0,     0,     0,     0,     0,     0,     0,   258,   260,
     261,   262,   263,   264,   265,   266,     0,     0,     0,     0,
       0,     0,     0,   268,   270,   271,   274,   275,   272,   276,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   278,   280,   281,   282,   283,   287,   288,   289,   284,
     285,   286,     0,     0,     0,     0,     0,     0,   301,   305,
     306,   307,   308,   309,   297,   299,   300,   302,   303,   304,
     310,     0,     0,     0,     0,     0,     0,     0,     0,   563,
     565,   567,   566,   572,   568,   569,   570,   571,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   582,   584,   586,
     585,   587,   588,   589,   590,   591,   592,   593,   594,   595,
     596,   597,   598,   599,   600,   601,   602,     0,   623,   625,
       0,   628,   630,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   637,   639,   640,   641,   643,   644,   642,
     645,   646,   647,   648,     0,     0,     0,     0,     0,     0,
     660,   662,   663,   664,   665,   666,   667,     0,     0,   676,
     678,   679,   312,   311,   318,   331,   329,   342,   338,   339,
     343,   340,   341,   344,   345,   346,   350,   351,   381,   382,
     383,   384,   385,   413,   414,   415,   421,   422,   334,   423,
     424,   427,   425,   426,   430,   431,   432,   446,   396,   397,
     400,   401,   433,   449,   390,   392,   450,   457,   458,   459,
     335,   412,   478,   479,   391,   472,   374,   330,   386,   447,
     454,   434,     0,     0,   482,   336,   313,   373,   438,   314,
     332,   333,   387,   388,   480,   436,   440,   441,   348,   347,
     315,   483,   416,   445,   375,   395,   451,   452,   453,   456,
     471,   389,   476,   474,   475,   404,   411,   442,   443,   405,
     406,   435,   461,   376,   377,   380,   352,   354,   349,   355,
     356,   357,   358,   365,   366,   367,   368,   369,   370,   371,
     484,   485,   487,   417,   418,   419,   420,   428,   429,   488,
     489,   490,     0,     0,     0,   437,   407,   409,   632,   499,
     503,   501,   500,   504,   502,   511,     0,     0,   507,   508,
     509,   510,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   439,   455,   477,   515,   516,   408,   491,     0,
       0,     0,     0,     0,     0,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   633,   398,   399,   402,   393,   460,
     372,   316,   317,   394,   517,   518,   519,   520,   521,   523,
     522,   524,   525,   526,   353,   360,   512,   514,   513,   359,
       0,   379,   444,   486,   378,   410,   361,   362,   364,   363,
       0,   528,   403,   473,   337,   529,   530,   531,   532,   537,
     535,   536,   533,   534,   538,   539,   540,   541,   543,   544,
     542,   555,     0,   559,   560,     0,     0,   561,   545,   553,
     546,   547,   548,   552,   554,   549,   550,   551,   290,   291,
     292,   293,   294,   295,   573,   575,   574,   577,   578,   579,
     580,   576,   603,   605,   606,   607,   608,   609,   610,   611,
     612,   613,   604,   614,   615,   616,   617,   618,   619,   620,
     621,   626,   631,   649,   650,   651,   654,   652,   653,   655,
     656,   657,   658,   668,   669,   670,   671,   672,   673,   680,
     681,   448,   481,   498,   634,   635,   505,   506,   492,   493,
       0,     0,     0,   497,   674,   527,   556,   557,   558,   496,
     494,   495
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    17,    18,    19,    32,   269,    20,    33,   502,
      21,    34,   518,    22,    35,   533,    23,    36,   551,   568,
     569,   570,   571,   572,   573,    24,    37,   574,   270,   271,
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
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   408,   409,   410,   411,
     412,   413,   414,   415,   416,   417,   418,   419,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   503,   504,   505,
     506,   507,   508,   509,   510,   519,   520,   521,   522,   523,
     524,   525,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   534,   535,   536,   537,   538,   539,   540,    25,
      38,   589,   590,   591,   592,   593,   594,   595,   596,   597,
      26,    39,   617,   618,   619,   620,   621,   622,   623,   624,
     625,   626,   627,   628,   629,   630,   631,   632,   633,   634,
     635,   636,    27,    40,   638,   639,    28,    41,   641,   642,
     489,   490,   491,   492,    29,    42,   653,   654,   655,   656,
     657,   658,   659,   660,   661,   662,   663,    30,    43,   670,
     671,   672,   673,   674,   675,   676,   493,    31,    44,   679,
     680,   681
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -311
static const yytype_int16 yypact[] =
{
    -311,     0,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   304,   -39,   -32,   -43,   -30,   -44,   -42,   -97,
    -110,  -310,  -230,  -234,  -304,     4,     6,     7,     8,     9,
      10,    23,    24,    25,    26,    27,    37,    38,    39,    40,
      41,    43,    44,    53,    54,    56,    57,    58,    59,    60,
      81,    82,    83,    84,    85,    87,    88,    89,    90,    91,
      92,    93,    95,    96,    98,    99,   101,   103,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   221,   222,   223,
     224,   225,   226,   227,   228,   233,   234,   235,   236,   237,
     238,   240,   249,   250,   251,   252,   255,   256,   262,   264,
     265,   266,   267,   268,   269,   271,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   297,   298,   299,
     301,   302,   303,   305,   339,   340,   341,   342,   346,   347,
     348,   390,   391,   392,   393,   394,   395,   396,   397,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,   398,   404,   408,   409,   436,   437,
     438,   440,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,   441,   449,   462,   463,   464,   465,   466,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,   467,   468,   469,   470,
     471,   472,   473,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,   474,   475,   476,   477,   478,   479,   480,   481,   524,
     526,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   546,   547,   548,   549,   550,   551,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,   552,   553,   554,   555,   556,   567,   568,   569,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,   570,   571,
     572,   573,   575,   576,   577,   578,   579,   580,   581,   584,
     587,   590,   591,   600,   601,   602,   604,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,   605,  -311,  -311,
     606,  -311,  -311,   607,   608,   609,   610,   611,   616,   617,
     618,   621,   622,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,   623,   624,   625,   626,   627,   628,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,   629,   630,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   631,   632,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   633,   634,   635,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,   636,   637,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,   638,
     639,   640,   641,   642,   643,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
     644,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
     645,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   646,  -311,  -311,   647,   648,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
     649,   650,   651,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,   -27,   652,   653,   654,   655,  -311,  -311,   656,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,  -311,
    -311,  -311
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
       2,   541,   526,   677,   678,   637,   494,   640,   495,   496,
     575,     3,     4,   511,   682,   541,   683,   684,   685,   686,
     687,   512,   513,   643,   644,   645,   646,   647,   648,   649,
     650,   651,   652,   688,   689,   690,   691,   692,   527,   528,
     664,   665,   666,   667,   668,   669,     5,   693,   694,   695,
     696,   697,     6,   698,   699,   581,   582,   583,   584,   585,
     586,   587,   588,   700,   701,   529,   702,   703,   704,   705,
     706,   497,   598,   599,   600,   601,   602,   603,   604,   605,
     606,   607,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   707,   708,   709,   710,   711,     7,   712,   713,   714,
     715,   716,   717,   718,   498,   719,   720,   499,   721,   722,
     514,   723,   515,   724,     8,   516,   500,   725,   726,   727,
     728,   729,   730,   731,   732,   733,   734,   735,   736,   737,
     738,   739,   740,   741,   742,   743,   744,   745,   746,   747,
     748,   749,   750,   751,   752,   753,   530,   531,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,   768,   769,   770,   771,   772,   773,     9,   774,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   532,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     804,   805,   806,   807,   808,   809,   810,   811,   812,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   823,
     824,   825,   826,   827,   828,   829,   830,   831,   832,   833,
      10,   834,   835,   836,   837,   838,   839,   840,   841,   543,
     544,   545,   546,   842,   843,   844,   845,   846,   847,   548,
     848,   542,    11,   543,   544,   545,   546,   547,   501,   849,
     850,   851,   852,   548,   517,   853,   854,   562,   563,   564,
     565,   566,   855,    12,   856,   857,   858,   859,   860,   861,
     567,   862,    13,   863,   864,   865,   866,   867,   868,   869,
     870,   871,   549,   550,   872,   873,   874,   875,   876,   877,
     878,   879,   880,   881,   882,   883,    14,   884,   885,   886,
      15,   887,   888,   889,     0,   890,    16,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,   891,
     892,   893,   894,    77,    78,    79,   895,   896,   897,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     898,   899,   900,   901,   902,   903,   904,   905,   906,   121,
     122,   123,   124,   125,   907,   126,   127,   128,   908,   909,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   910,   911,   912,   155,
     913,   914,   156,   157,   158,   159,   160,   161,   162,   915,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   916,   917,   918,   919,   920,   921,   922,   923,
     924,   925,   926,   927,   928,   929,   930,   931,   932,   933,
     934,   935,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   936,   217,   937,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   938,   939,   940,   941,
     942,   943,   944,   945,   946,   947,   948,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   949,   950,   951,
     952,   953,   954,   955,   247,   956,   957,   958,   959,   960,
     961,   962,   248,   249,   963,   250,   251,   964,   252,   253,
     965,   966,   254,   255,   256,   257,   258,   259,   260,   261,
     967,   968,   969,   262,   970,   971,   972,   973,   974,   975,
     976,   977,   263,   264,   265,   266,   978,   979,   980,   267,
     268,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   576,
     577,   578,   579,   580
};

static const yytype_int16 yycheck[] =
{
       0,    45,    45,   307,   308,   115,    45,   317,    47,    48,
      37,    11,    12,    45,    10,    45,    10,    10,    10,    10,
      10,    53,    54,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,    10,    10,    10,    10,    10,    81,    82,
     274,   275,   276,   277,   278,   279,    46,    10,    10,    10,
      10,    10,    52,    10,    10,    97,    98,    99,   100,   101,
     102,   103,   104,    10,    10,   108,    10,    10,    10,    10,
      10,   110,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,    10,    10,    10,    10,    10,    96,    10,    10,    10,
      10,    10,    10,    10,   143,    10,    10,   146,    10,    10,
     142,    10,   144,    10,   114,   147,   155,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,   189,   190,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,   168,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,   232,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
     230,    10,    10,    10,    10,    10,    10,    10,    10,   283,
     284,   285,   286,    10,    10,    10,    10,    10,    10,   293,
      10,   281,   252,   283,   284,   285,   286,   287,   297,    10,
      10,    10,    10,   293,   296,    10,    10,   311,   312,   313,
     314,   315,    10,   273,    10,    10,    10,    10,    10,    10,
     324,    10,   282,    10,    10,    10,    10,    10,    10,    10,
      10,    10,   322,   323,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,   306,    10,    10,    10,
     310,    10,    10,    10,    -1,    10,   316,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    10,
      10,    10,    10,    49,    50,    51,    10,    10,    10,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      10,    10,    10,    10,    10,    10,    10,    10,    10,   105,
     106,   107,   108,   109,    10,   111,   112,   113,    10,    10,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,    10,    10,    10,   145,
      10,    10,   148,   149,   150,   151,   152,   153,   154,    10,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,    10,   231,    10,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,    10,    10,    10,
      10,    10,    10,    10,   280,    10,    10,    10,    10,    10,
      10,    10,   288,   289,    10,   291,   292,    10,   294,   295,
      10,    10,   298,   299,   300,   301,   302,   303,   304,   305,
      10,    10,    10,   309,    10,    10,    10,    10,    10,    10,
      10,    10,   318,   319,   320,   321,    10,    10,    10,   325,
     326,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      37,    37,    37,    37
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   328,     0,    11,    12,    46,    52,    96,   114,   168,
     230,   252,   273,   282,   306,   310,   316,   329,   330,   331,
     334,   337,   340,   343,   352,   606,   617,   639,   643,   651,
     664,   674,   332,   335,   338,   341,   344,   353,   607,   618,
     640,   644,   652,   665,   675,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    49,    50,    51,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,   105,   106,   107,   108,   109,   111,   112,   113,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   145,   148,   149,   150,   151,
     152,   153,   154,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   231,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   280,   288,   289,
     291,   292,   294,   295,   298,   299,   300,   301,   302,   303,
     304,   305,   309,   318,   319,   320,   321,   325,   326,   333,
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
     565,   566,   567,   568,   569,   570,   571,   572,   573,   647,
     648,   649,   650,   673,    45,    47,    48,   110,   143,   146,
     155,   297,   336,   574,   575,   576,   577,   578,   579,   580,
     581,    45,    53,    54,   142,   144,   147,   296,   339,   582,
     583,   584,   585,   586,   587,   588,    45,    81,    82,   108,
     189,   190,   232,   342,   599,   600,   601,   602,   603,   604,
     605,    45,   281,   283,   284,   285,   286,   287,   293,   322,
     323,   345,   589,   590,   591,   592,   593,   594,   595,   596,
     597,   598,   311,   312,   313,   314,   315,   324,   346,   347,
     348,   349,   350,   351,   354,   589,   590,   591,   592,   593,
     596,    97,    98,    99,   100,   101,   102,   103,   104,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   619,   620,   621,
     622,   623,   624,   625,   626,   627,   628,   629,   630,   631,
     632,   633,   634,   635,   636,   637,   638,   115,   641,   642,
     317,   645,   646,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   653,   654,   655,   656,   657,   658,   659,
     660,   661,   662,   663,   274,   275,   276,   277,   278,   279,
     666,   667,   668,   669,   670,   671,   672,   307,   308,   676,
     677,   678,    10,    10,    10,    10,    10,    10,    10,    10,
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
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 18:
#line 205 "./util/configparser.y"
    {
		OUTYY(("\nP(force-toplevel)\n"));
	}
    break;

  case 19:
#line 211 "./util/configparser.y"
    {
		OUTYY(("\nP(server:)\n"));
	}
    break;

  case 246:
#line 320 "./util/configparser.y"
    {
		struct config_stub* s;
		OUTYY(("\nP(stub_zone:)\n"));
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->stubs;
			cfg_parser->cfg->stubs = s;
		} else {
			yyerror("out of memory");
		}
	}
    break;

  case 257:
#line 338 "./util/configparser.y"
    {
		struct config_stub* s;
		OUTYY(("\nP(forward_zone:)\n"));
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->forwards;
			cfg_parser->cfg->forwards = s;
		} else {
			yyerror("out of memory");
		}
	}
    break;

  case 267:
#line 356 "./util/configparser.y"
    {
		struct config_view* s;
		OUTYY(("\nP(view:)\n"));
		s = (struct config_view*)calloc(1, sizeof(struct config_view));
		if(s) {
			s->next = cfg_parser->cfg->views;
			if(s->next && !s->next->name)
				yyerror("view without name");
			cfg_parser->cfg->views = s;
		} else {
			yyerror("out of memory");
		}
	}
    break;

  case 277:
#line 376 "./util/configparser.y"
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
			s->zonemd_check = 0;
			s->zonemd_reject_absence = 0;
			s->isrpz = 0;
		} else {
			yyerror("out of memory");
		}
	}
    break;

  case 290:
#line 403 "./util/configparser.y"
    {
		uint8_t* bitlist;
		size_t len = 0;
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[(2) - (2)].str)));
		bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[(2) - (2)].str),
			&len);
		free((yyvsp[(2) - (2)].str));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
		}
		if(bitlist) {
			cfg_parser->cfg->auths->rpz_taglist = bitlist;
			cfg_parser->cfg->auths->rpz_taglistlen = len;

		}
	}
    break;

  case 291:
#line 422 "./util/configparser.y"
    {
		OUTYY(("P(rpz_action_override:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "nxdomain")!=0 && strcmp((yyvsp[(2) - (2)].str), "nodata")!=0 &&
		   strcmp((yyvsp[(2) - (2)].str), "passthru")!=0 && strcmp((yyvsp[(2) - (2)].str), "drop")!=0 &&
		   strcmp((yyvsp[(2) - (2)].str), "cname")!=0 && strcmp((yyvsp[(2) - (2)].str), "disabled")!=0) {
			yyerror("rpz-action-override action: expected nxdomain, "
				"nodata, passthru, drop, cname or disabled");
			free((yyvsp[(2) - (2)].str));
			cfg_parser->cfg->auths->rpz_action_override = NULL;
		}
		else {
			cfg_parser->cfg->auths->rpz_action_override = (yyvsp[(2) - (2)].str);
		}
	}
    break;

  case 292:
#line 439 "./util/configparser.y"
    {
		OUTYY(("P(rpz_cname_override:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->auths->rpz_cname);
		cfg_parser->cfg->auths->rpz_cname = (yyvsp[(2) - (2)].str);
	}
    break;

  case 293:
#line 447 "./util/configparser.y"
    {
		OUTYY(("P(rpz_log:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->rpz_log = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 294:
#line 457 "./util/configparser.y"
    {
		OUTYY(("P(rpz_log_name:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->auths->rpz_log_name);
		cfg_parser->cfg->auths->rpz_log_name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 295:
#line 464 "./util/configparser.y"
    {
		OUTYY(("P(rpz_signal_nxdomain_ra:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->rpz_signal_nxdomain_ra = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 296:
#line 474 "./util/configparser.y"
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
		} else {
			yyerror("out of memory");
		}
	}
    break;

  case 311:
#line 498 "./util/configparser.y"
    {
		OUTYY(("P(server_num_threads:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 312:
#line 507 "./util/configparser.y"
    {
		OUTYY(("P(server_verbosity:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 313:
#line 516 "./util/configparser.y"
    {
		OUTYY(("P(server_statistics_interval:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "") == 0 || strcmp((yyvsp[(2) - (2)].str), "0") == 0)
			cfg_parser->cfg->stat_interval = 0;
		else if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->stat_interval = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 314:
#line 527 "./util/configparser.y"
    {
		OUTYY(("P(server_statistics_cumulative:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 315:
#line 536 "./util/configparser.y"
    {
		OUTYY(("P(server_extended_statistics:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 316:
#line 545 "./util/configparser.y"
    {
		OUTYY(("P(server_shm_enable:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->shm_enable = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 317:
#line 554 "./util/configparser.y"
    {
		OUTYY(("P(server_shm_key:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "") == 0 || strcmp((yyvsp[(2) - (2)].str), "0") == 0)
			cfg_parser->cfg->shm_key = 0;
		else if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->shm_key = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 318:
#line 565 "./util/configparser.y"
    {
		OUTYY(("P(server_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 319:
#line 574 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_send_client_subnet:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet, (yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding client-subnet");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 320:
#line 586 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_zone:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet_zone,
			(yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding client-subnet-zone");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 321:
#line 600 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_always_forward:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->client_subnet_always_forward =
				(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 322:
#line 615 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(client_subnet_opcode:%s)\n", (yyvsp[(2) - (2)].str)));
		OUTYY(("P(Deprecated option, ignoring)\n"));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 323:
#line 626 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv4:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 32)
			cfg_parser->cfg->max_client_subnet_ipv4 = 32;
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->max_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv4 = (uint8_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 324:
#line 643 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv6:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 128)
			cfg_parser->cfg->max_client_subnet_ipv6 = 128;
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->max_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv6 = (uint8_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 325:
#line 660 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv4:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 32)
			cfg_parser->cfg->min_client_subnet_ipv4 = 32;
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->min_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv4 = (uint8_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 326:
#line 677 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv6:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 128)
			cfg_parser->cfg->min_client_subnet_ipv6 = 128;
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->min_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv6 = (uint8_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 327:
#line 694 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv4:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("IPv4 ECS tree size expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv4 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv4 = (uint32_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 328:
#line 709 "./util/configparser.y"
    {
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv6:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("IPv6 ECS tree size expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv6 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv6 = (uint32_t)atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 329:
#line 724 "./util/configparser.y"
    {
		OUTYY(("P(server_interface:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = (yyvsp[(2) - (2)].str);
	}
    break;

  case 330:
#line 737 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_interface:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->num_out_ifs == 0)
			cfg_parser->cfg->out_ifs = calloc(1, sizeof(char*));
		else cfg_parser->cfg->out_ifs = realloc(
			cfg_parser->cfg->out_ifs,
			(cfg_parser->cfg->num_out_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->out_ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->out_ifs[
				cfg_parser->cfg->num_out_ifs++] = (yyvsp[(2) - (2)].str);
	}
    break;

  case 331:
#line 752 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_range:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 332:
#line 761 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_port_permit:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_mark_ports((yyvsp[(2) - (2)].str), 1,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 333:
#line 770 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_mark_ports((yyvsp[(2) - (2)].str), 0,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 334:
#line 779 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 335:
#line 788 "./util/configparser.y"
    {
		OUTYY(("P(server_incoming_num_tcp:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 336:
#line 797 "./util/configparser.y"
    {
		OUTYY(("P(server_interface_automatic:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 337:
#line 806 "./util/configparser.y"
    {
		OUTYY(("P(server_interface_automatic_ports:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->if_automatic_ports);
		cfg_parser->cfg->if_automatic_ports = (yyvsp[(2) - (2)].str);
	}
    break;

  case 338:
#line 813 "./util/configparser.y"
    {
		OUTYY(("P(server_do_ip4:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 339:
#line 822 "./util/configparser.y"
    {
		OUTYY(("P(server_do_ip6:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 340:
#line 831 "./util/configparser.y"
    {
		OUTYY(("P(server_do_udp:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 341:
#line 840 "./util/configparser.y"
    {
		OUTYY(("P(server_do_tcp:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 342:
#line 849 "./util/configparser.y"
    {
		OUTYY(("P(server_prefer_ip4:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip4 = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 343:
#line 858 "./util/configparser.y"
    {
		OUTYY(("P(server_prefer_ip6:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip6 = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 344:
#line 867 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_mss:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
				yyerror("number expected");
		else cfg_parser->cfg->tcp_mss = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 345:
#line 876 "./util/configparser.y"
    {
		OUTYY(("P(server_outgoing_tcp_mss:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_tcp_mss = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 346:
#line 885 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_idle_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 120000)
			cfg_parser->cfg->tcp_idle_timeout = 120000;
		else if (atoi((yyvsp[(2) - (2)].str)) < 1)
			cfg_parser->cfg->tcp_idle_timeout = 1;
		else cfg_parser->cfg->tcp_idle_timeout = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 347:
#line 898 "./util/configparser.y"
    {
		OUTYY(("P(server_max_reuse_tcp_queries:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 1)
			cfg_parser->cfg->max_reuse_tcp_queries = 0;
		else cfg_parser->cfg->max_reuse_tcp_queries = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 348:
#line 909 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_reuse_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 1)
			cfg_parser->cfg->tcp_reuse_timeout = 0;
		else cfg_parser->cfg->tcp_reuse_timeout = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 349:
#line 920 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_auth_query_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 1)
			cfg_parser->cfg->tcp_auth_query_timeout = 0;
		else cfg_parser->cfg->tcp_auth_query_timeout = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 350:
#line 931 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_keepalive:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp_keepalive = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 351:
#line 940 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_keepalive_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 6553500)
			cfg_parser->cfg->tcp_keepalive_timeout = 6553500;
		else if (atoi((yyvsp[(2) - (2)].str)) < 1)
			cfg_parser->cfg->tcp_keepalive_timeout = 0;
		else cfg_parser->cfg->tcp_keepalive_timeout = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 352:
#line 953 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_upstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tcp_upstream = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 353:
#line 962 "./util/configparser.y"
    {
		OUTYY(("P(server_udp_upstream_without_downstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_upstream_without_downstream = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 354:
#line 971 "./util/configparser.y"
    {
		OUTYY(("P(server_ssl_upstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ssl_upstream = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 355:
#line 980 "./util/configparser.y"
    {
		OUTYY(("P(server_ssl_service_key:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->ssl_service_key);
		cfg_parser->cfg->ssl_service_key = (yyvsp[(2) - (2)].str);
	}
    break;

  case 356:
#line 987 "./util/configparser.y"
    {
		OUTYY(("P(server_ssl_service_pem:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->ssl_service_pem);
		cfg_parser->cfg->ssl_service_pem = (yyvsp[(2) - (2)].str);
	}
    break;

  case 357:
#line 994 "./util/configparser.y"
    {
		OUTYY(("P(server_ssl_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->ssl_port = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 358:
#line 1003 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_cert_bundle:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = (yyvsp[(2) - (2)].str);
	}
    break;

  case 359:
#line 1010 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_win_cert:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_win_cert = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 360:
#line 1019 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_additional_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->tls_additional_port,
			(yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 361:
#line 1027 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_ciphers:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->tls_ciphers);
		cfg_parser->cfg->tls_ciphers = (yyvsp[(2) - (2)].str);
	}
    break;

  case 362:
#line 1034 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_ciphersuites:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->tls_ciphersuites);
		cfg_parser->cfg->tls_ciphersuites = (yyvsp[(2) - (2)].str);
	}
    break;

  case 363:
#line 1041 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_session_ticket_keys:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->tls_session_ticket_keys,
			(yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 364:
#line 1049 "./util/configparser.y"
    {
		OUTYY(("P(server_tls_use_sni:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_use_sni = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 365:
#line 1058 "./util/configparser.y"
    {
		OUTYY(("P(server_https_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->https_port = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 366:
#line 1066 "./util/configparser.y"
    {
		OUTYY(("P(server_http_endpoint:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->http_endpoint);
		if((yyvsp[(2) - (2)].str) && (yyvsp[(2) - (2)].str)[0] != '/') {
			cfg_parser->cfg->http_endpoint = malloc(strlen((yyvsp[(2) - (2)].str))+2);
			if(!cfg_parser->cfg->http_endpoint)
				yyerror("out of memory");
			cfg_parser->cfg->http_endpoint[0] = '/';
			memmove(cfg_parser->cfg->http_endpoint+1, (yyvsp[(2) - (2)].str),
				strlen((yyvsp[(2) - (2)].str))+1);
			free((yyvsp[(2) - (2)].str));
		} else {
			cfg_parser->cfg->http_endpoint = (yyvsp[(2) - (2)].str);
		}
	}
    break;

  case 367:
#line 1082 "./util/configparser.y"
    {
		OUTYY(("P(server_http_max_streams:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->http_max_streams = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 368:
#line 1090 "./util/configparser.y"
    {
		OUTYY(("P(server_http_query_buffer_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str),
			&cfg_parser->cfg->http_query_buffer_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 369:
#line 1098 "./util/configparser.y"
    {
		OUTYY(("P(server_http_response_buffer_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str),
			&cfg_parser->cfg->http_response_buffer_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 370:
#line 1106 "./util/configparser.y"
    {
		OUTYY(("P(server_http_nodelay:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->http_nodelay = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 371:
#line 1114 "./util/configparser.y"
    {
		OUTYY(("P(server_http_notls_downstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->http_notls_downstream = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 372:
#line 1122 "./util/configparser.y"
    {
		OUTYY(("P(server_use_systemd:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_systemd = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 373:
#line 1131 "./util/configparser.y"
    {
		OUTYY(("P(server_do_daemonize:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 374:
#line 1140 "./util/configparser.y"
    {
		OUTYY(("P(server_use_syslog:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if(strcmp((yyvsp[(2) - (2)].str), "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 375:
#line 1154 "./util/configparser.y"
    {
		OUTYY(("P(server_log_time_ascii:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 376:
#line 1163 "./util/configparser.y"
    {
		OUTYY(("P(server_log_queries:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_queries = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 377:
#line 1172 "./util/configparser.y"
    {
		OUTYY(("P(server_log_replies:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_replies = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 378:
#line 1181 "./util/configparser.y"
    {
		OUTYY(("P(server_log_tag_queryreply:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_tag_queryreply = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 379:
#line 1190 "./util/configparser.y"
    {
		OUTYY(("P(server_log_servfail:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_servfail = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 380:
#line 1199 "./util/configparser.y"
    {
		OUTYY(("P(server_log_local_actions:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_local_actions = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 381:
#line 1208 "./util/configparser.y"
    {
		OUTYY(("P(server_chroot:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = (yyvsp[(2) - (2)].str);
	}
    break;

  case 382:
#line 1215 "./util/configparser.y"
    {
		OUTYY(("P(server_username:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = (yyvsp[(2) - (2)].str);
	}
    break;

  case 383:
#line 1222 "./util/configparser.y"
    {
		OUTYY(("P(server_directory:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = (yyvsp[(2) - (2)].str);
		/* change there right away for includes relative to this */
		if((yyvsp[(2) - (2)].str)[0]) {
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
    break;

  case 384:
#line 1248 "./util/configparser.y"
    {
		OUTYY(("P(server_logfile:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = (yyvsp[(2) - (2)].str);
		cfg_parser->cfg->use_syslog = 0;
	}
    break;

  case 385:
#line 1256 "./util/configparser.y"
    {
		OUTYY(("P(server_pidfile:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = (yyvsp[(2) - (2)].str);
	}
    break;

  case 386:
#line 1263 "./util/configparser.y"
    {
		OUTYY(("P(server_root_hints:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 387:
#line 1270 "./util/configparser.y"
    {
		OUTYY(("P(server_dlv_anchor_file:%s)\n", (yyvsp[(2) - (2)].str)));
		log_warn("option dlv-anchor-file ignored: DLV is decommissioned");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 388:
#line 1277 "./util/configparser.y"
    {
		OUTYY(("P(server_dlv_anchor:%s)\n", (yyvsp[(2) - (2)].str)));
		log_warn("option dlv-anchor ignored: DLV is decommissioned");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 389:
#line 1284 "./util/configparser.y"
    {
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 390:
#line 1292 "./util/configparser.y"
    {
		OUTYY(("P(server_trust_anchor_file:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 391:
#line 1300 "./util/configparser.y"
    {
		OUTYY(("P(server_trusted_keys_file:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 392:
#line 1308 "./util/configparser.y"
    {
		OUTYY(("P(server_trust_anchor:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 393:
#line 1315 "./util/configparser.y"
    {
		OUTYY(("P(server_trust_anchor_signaling:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->trust_anchor_signaling =
				(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 394:
#line 1326 "./util/configparser.y"
    {
		OUTYY(("P(server_root_key_sentinel:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->root_key_sentinel =
				(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 395:
#line 1337 "./util/configparser.y"
    {
		OUTYY(("P(server_domain_insecure:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 396:
#line 1344 "./util/configparser.y"
    {
		OUTYY(("P(server_hide_identity:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 397:
#line 1353 "./util/configparser.y"
    {
		OUTYY(("P(server_hide_version:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 398:
#line 1362 "./util/configparser.y"
    {
		OUTYY(("P(server_hide_trustanchor:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_trustanchor = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 399:
#line 1371 "./util/configparser.y"
    {
		OUTYY(("P(server_hide_user_agent:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_http_user_agent = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 400:
#line 1380 "./util/configparser.y"
    {
		OUTYY(("P(server_identity:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = (yyvsp[(2) - (2)].str);
	}
    break;

  case 401:
#line 1387 "./util/configparser.y"
    {
		OUTYY(("P(server_version:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = (yyvsp[(2) - (2)].str);
	}
    break;

  case 402:
#line 1394 "./util/configparser.y"
    {
		OUTYY(("P(server_http_user_agent:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->http_user_agent);
		cfg_parser->cfg->http_user_agent = (yyvsp[(2) - (2)].str);
	}
    break;

  case 403:
#line 1401 "./util/configparser.y"
    {
		OUTYY(("P(server_nsid:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->nsid_cfg_str);
		cfg_parser->cfg->nsid_cfg_str = (yyvsp[(2) - (2)].str);
		free(cfg_parser->cfg->nsid);
		cfg_parser->cfg->nsid = NULL;
		cfg_parser->cfg->nsid_len = 0;
		if (*(yyvsp[(2) - (2)].str) == 0)
			; /* pass; empty string is not setting nsid */
		else if (!(cfg_parser->cfg->nsid = cfg_parse_nsid(
					(yyvsp[(2) - (2)].str), &cfg_parser->cfg->nsid_len)))
			yyerror("the NSID must be either a hex string or an "
			    "ascii character string prepended with ascii_.");
	}
    break;

  case 404:
#line 1417 "./util/configparser.y"
    {
		OUTYY(("P(server_so_rcvbuf:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->so_rcvbuf))
			yyerror("buffer size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 405:
#line 1425 "./util/configparser.y"
    {
		OUTYY(("P(server_so_sndbuf:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->so_sndbuf))
			yyerror("buffer size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 406:
#line 1433 "./util/configparser.y"
    {
		OUTYY(("P(server_so_reuseport:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->so_reuseport =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 407:
#line 1443 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_transparent:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_transparent =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 408:
#line 1453 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_freebind:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_freebind =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 409:
#line 1463 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_dscp:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) > 63)
			yyerror("value too large (max 63)");
		else if (atoi((yyvsp[(2) - (2)].str)) < 0)
			yyerror("value too small (min 0)");
		else
			cfg_parser->cfg->ip_dscp = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 410:
#line 1477 "./util/configparser.y"
    {
		OUTYY(("P(server_stream_wait_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->stream_wait_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 411:
#line 1485 "./util/configparser.y"
    {
		OUTYY(("P(server_edns_buffer_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 12)
			yyerror("edns buffer size too small");
		else if (atoi((yyvsp[(2) - (2)].str)) > 65535)
			cfg_parser->cfg->edns_buffer_size = 65535;
		else cfg_parser->cfg->edns_buffer_size = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 412:
#line 1498 "./util/configparser.y"
    {
		OUTYY(("P(server_msg_buffer_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[(2) - (2)].str)) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else cfg_parser->cfg->msg_buffer_size = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 413:
#line 1509 "./util/configparser.y"
    {
		OUTYY(("P(server_msg_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 414:
#line 1517 "./util/configparser.y"
    {
		OUTYY(("P(server_msg_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->msg_cache_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 415:
#line 1530 "./util/configparser.y"
    {
		OUTYY(("P(server_num_queries_per_thread:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 416:
#line 1539 "./util/configparser.y"
    {
		OUTYY(("P(server_jostle_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 417:
#line 1548 "./util/configparser.y"
    {
		OUTYY(("P(server_delay_close:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->delay_close = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 418:
#line 1557 "./util/configparser.y"
    {
		OUTYY(("P(server_udp_connect:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_connect = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 419:
#line 1566 "./util/configparser.y"
    {
		OUTYY(("P(server_unblock_lan_zones:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->unblock_lan_zones =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 420:
#line 1576 "./util/configparser.y"
    {
		OUTYY(("P(server_insecure_lan_zones:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->insecure_lan_zones =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 421:
#line 1586 "./util/configparser.y"
    {
		OUTYY(("P(server_rrset_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 422:
#line 1594 "./util/configparser.y"
    {
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->rrset_cache_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 423:
#line 1607 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_host_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 424:
#line 1616 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_lame_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", (yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 425:
#line 1624 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 426:
#line 1633 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", (yyvsp[(2) - (2)].str)));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", (yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 427:
#line 1641 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->infra_cache_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 428:
#line 1654 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_min_rtt:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_min_rtt = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 429:
#line 1663 "./util/configparser.y"
    {
		OUTYY(("P(server_infra_keep_probing:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->infra_keep_probing =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 430:
#line 1673 "./util/configparser.y"
    {
		OUTYY(("P(server_target_fetch_policy:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = (yyvsp[(2) - (2)].str);
	}
    break;

  case 431:
#line 1680 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_short_bufsize:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 432:
#line 1690 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_large_queries:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 433:
#line 1700 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_glue:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 434:
#line 1710 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 435:
#line 1720 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_below_nxdomain:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_below_nxdomain =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 436:
#line 1730 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_referral_path:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 437:
#line 1740 "./util/configparser.y"
    {
		OUTYY(("P(server_harden_algo_downgrade:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_algo_downgrade =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 438:
#line 1750 "./util/configparser.y"
    {
		OUTYY(("P(server_use_caps_for_id:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 439:
#line 1760 "./util/configparser.y"
    {
		OUTYY(("P(server_caps_whitelist:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 440:
#line 1767 "./util/configparser.y"
    {
		OUTYY(("P(server_private_address:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 441:
#line 1774 "./util/configparser.y"
    {
		OUTYY(("P(server_private_domain:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 442:
#line 1781 "./util/configparser.y"
    {
		OUTYY(("P(server_prefetch:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 443:
#line 1790 "./util/configparser.y"
    {
		OUTYY(("P(server_prefetch_key:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch_key = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 444:
#line 1799 "./util/configparser.y"
    {
		OUTYY(("P(server_deny_any:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->deny_any = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 445:
#line 1808 "./util/configparser.y"
    {
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 446:
#line 1817 "./util/configparser.y"
    {
		OUTYY(("P(server_do_not_query_address:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 447:
#line 1824 "./util/configparser.y"
    {
		OUTYY(("P(server_do_not_query_localhost:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 448:
#line 1834 "./util/configparser.y"
    {
		OUTYY(("P(server_access_control:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(strcmp((yyvsp[(3) - (3)].str), "deny")!=0 && strcmp((yyvsp[(3) - (3)].str), "refuse")!=0 &&
			strcmp((yyvsp[(3) - (3)].str), "deny_non_local")!=0 &&
			strcmp((yyvsp[(3) - (3)].str), "refuse_non_local")!=0 &&
			strcmp((yyvsp[(3) - (3)].str), "allow_setrd")!=0 &&
			strcmp((yyvsp[(3) - (3)].str), "allow")!=0 &&
			strcmp((yyvsp[(3) - (3)].str), "allow_snoop")!=0) {
			yyerror("expected deny, refuse, deny_non_local, "
				"refuse_non_local, allow, allow_setrd or "
				"allow_snoop in access control action");
			free((yyvsp[(2) - (3)].str));
			free((yyvsp[(3) - (3)].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding acl");
		}
	}
    break;

  case 449:
#line 1854 "./util/configparser.y"
    {
		OUTYY(("P(server_module_conf:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = (yyvsp[(2) - (2)].str);
	}
    break;

  case 450:
#line 1861 "./util/configparser.y"
    {
		OUTYY(("P(server_val_override_date:%s)\n", (yyvsp[(2) - (2)].str)));
		if(*(yyvsp[(2) - (2)].str) == '\0' || strcmp((yyvsp[(2) - (2)].str), "0") == 0) {
			cfg_parser->cfg->val_date_override = 0;
		} else if(strlen((yyvsp[(2) - (2)].str)) == 14) {
			cfg_parser->cfg->val_date_override =
				cfg_convert_timeval((yyvsp[(2) - (2)].str));
			if(!cfg_parser->cfg->val_date_override)
				yyerror("bad date/time specification");
		} else {
			if(atoi((yyvsp[(2) - (2)].str)) == 0)
				yyerror("number expected");
			cfg_parser->cfg->val_date_override = atoi((yyvsp[(2) - (2)].str));
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 451:
#line 1879 "./util/configparser.y"
    {
		OUTYY(("P(server_val_sig_skew_min:%s)\n", (yyvsp[(2) - (2)].str)));
		if(*(yyvsp[(2) - (2)].str) == '\0' || strcmp((yyvsp[(2) - (2)].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_min = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_min = atoi((yyvsp[(2) - (2)].str));
			if(!cfg_parser->cfg->val_sig_skew_min)
				yyerror("number expected");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 452:
#line 1892 "./util/configparser.y"
    {
		OUTYY(("P(server_val_sig_skew_max:%s)\n", (yyvsp[(2) - (2)].str)));
		if(*(yyvsp[(2) - (2)].str) == '\0' || strcmp((yyvsp[(2) - (2)].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_max = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_max = atoi((yyvsp[(2) - (2)].str));
			if(!cfg_parser->cfg->val_sig_skew_max)
				yyerror("number expected");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 453:
#line 1905 "./util/configparser.y"
    {
		OUTYY(("P(server_val_max_restart:%s)\n", (yyvsp[(2) - (2)].str)));
		if(*(yyvsp[(2) - (2)].str) == '\0' || strcmp((yyvsp[(2) - (2)].str), "0") == 0) {
			cfg_parser->cfg->val_max_restart = 0;
		} else {
			cfg_parser->cfg->val_max_restart = atoi((yyvsp[(2) - (2)].str));
			if(!cfg_parser->cfg->val_max_restart)
				yyerror("number expected");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 454:
#line 1918 "./util/configparser.y"
    {
		OUTYY(("P(server_cache_max_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 455:
#line 1927 "./util/configparser.y"
    {
		OUTYY(("P(server_cache_max_negative_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_negative_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 456:
#line 1936 "./util/configparser.y"
    {
		OUTYY(("P(server_cache_min_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 457:
#line 1945 "./util/configparser.y"
    {
		OUTYY(("P(server_bogus_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 458:
#line 1954 "./util/configparser.y"
    {
		OUTYY(("P(server_val_clean_additional:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 459:
#line 1964 "./util/configparser.y"
    {
		OUTYY(("P(server_val_permissive_mode:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 460:
#line 1974 "./util/configparser.y"
    {
		OUTYY(("P(server_aggressive_nsec:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->aggressive_nsec =
				(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 461:
#line 1985 "./util/configparser.y"
    {
		OUTYY(("P(server_ignore_cd_flag:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ignore_cd = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 462:
#line 1994 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 463:
#line 2003 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 464:
#line 2012 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired_ttl_reset:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired_ttl_reset = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 465:
#line 2021 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired_reply_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_reply_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 466:
#line 2030 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired_client_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_client_timeout = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 467:
#line 2039 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_expired_ede:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired_ede = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 468:
#line 2048 "./util/configparser.y"
    {
		OUTYY(("P(server_serve_original_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_original_ttl = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 469:
#line 2057 "./util/configparser.y"
    {
		OUTYY(("P(server_fake_dsa:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_dsa = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		if(fake_dsa)
			log_warn("test option fake_dsa is enabled");
#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 470:
#line 2070 "./util/configparser.y"
    {
		OUTYY(("P(server_fake_sha1:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_sha1 = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		if(fake_sha1)
			log_warn("test option fake_sha1 is enabled");
#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 471:
#line 2083 "./util/configparser.y"
    {
		OUTYY(("P(server_val_log_level:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->val_log_level = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 472:
#line 2092 "./util/configparser.y"
    {
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = (yyvsp[(2) - (2)].str);
	}
    break;

  case 473:
#line 2099 "./util/configparser.y"
    {
		OUTYY(("P(server_zonemd_permissive_mode:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else	cfg_parser->cfg->zonemd_permissive_mode = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 474:
#line 2108 "./util/configparser.y"
    {
		OUTYY(("P(server_add_holddown:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->add_holddown = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 475:
#line 2117 "./util/configparser.y"
    {
		OUTYY(("P(server_del_holddown:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->del_holddown = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 476:
#line 2126 "./util/configparser.y"
    {
		OUTYY(("P(server_keep_missing:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->keep_missing = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 477:
#line 2135 "./util/configparser.y"
    {
		OUTYY(("P(server_permit_small_holddown:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->permit_small_holddown =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 478:
#line 2144 "./util/configparser.y"
    {
		OUTYY(("P(server_key_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 479:
#line 2152 "./util/configparser.y"
    {
		OUTYY(("P(server_key_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->key_cache_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->key_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 480:
#line 2165 "./util/configparser.y"
    {
		OUTYY(("P(server_neg_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 481:
#line 2173 "./util/configparser.y"
    {
		OUTYY(("P(server_local_zone:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(strcmp((yyvsp[(3) - (3)].str), "static")!=0 && strcmp((yyvsp[(3) - (3)].str), "deny")!=0 &&
		   strcmp((yyvsp[(3) - (3)].str), "refuse")!=0 && strcmp((yyvsp[(3) - (3)].str), "redirect")!=0 &&
		   strcmp((yyvsp[(3) - (3)].str), "transparent")!=0 && strcmp((yyvsp[(3) - (3)].str), "nodefault")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "typetransparent")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_transparent")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_refuse")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_nodata")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_deny")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_null")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "noview")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "inform")!=0 && strcmp((yyvsp[(3) - (3)].str), "inform_deny")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "inform_redirect") != 0
		   && strcmp((yyvsp[(3) - (3)].str), "ipset") != 0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"inform_redirect, always_transparent, "
				"always_refuse, always_nxdomain, "
				"always_nodata, always_deny, always_null, "
				"noview, nodefault or ipset");
			free((yyvsp[(2) - (3)].str));
			free((yyvsp[(3) - (3)].str));
		} else if(strcmp((yyvsp[(3) - (3)].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, (yyvsp[(2) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[(3) - (3)].str));
#ifdef USE_IPSET
		} else if(strcmp((yyvsp[(3) - (3)].str), "ipset")==0) {
			size_t len = strlen((yyvsp[(2) - (3)].str));
			/* Make sure to add the trailing dot.
			 * These are str compared to domain names. */
			if((yyvsp[(2) - (3)].str)[len-1] != '.') {
				if(!((yyvsp[(2) - (3)].str) = realloc((yyvsp[(2) - (3)].str), len+2))) {
					fatal_exit("out of memory adding local-zone");
				}
				(yyvsp[(2) - (3)].str)[len] = '.';
				(yyvsp[(2) - (3)].str)[len+1] = 0;
			}
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_ipset, (yyvsp[(2) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[(3) - (3)].str));
#endif
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones,
				(yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
    break;

  case 482:
#line 2228 "./util/configparser.y"
    {
		OUTYY(("P(server_local_data:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, (yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding local-data");
	}
    break;

  case 483:
#line 2235 "./util/configparser.y"
    {
		char* ptr;
		OUTYY(("P(server_local_data_ptr:%s)\n", (yyvsp[(2) - (2)].str)));
		ptr = cfg_ptr_reverse((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
    break;

  case 484:
#line 2250 "./util/configparser.y"
    {
		OUTYY(("P(server_minimal_responses:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->minimal_responses =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 485:
#line 2260 "./util/configparser.y"
    {
		OUTYY(("P(server_rrset_roundrobin:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->rrset_roundrobin =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 486:
#line 2270 "./util/configparser.y"
    {
		OUTYY(("P(server_unknown_server_time_limit:%s)\n", (yyvsp[(2) - (2)].str)));
		cfg_parser->cfg->unknown_server_time_limit = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 487:
#line 2277 "./util/configparser.y"
    {
		OUTYY(("P(server_max_udp_size:%s)\n", (yyvsp[(2) - (2)].str)));
		cfg_parser->cfg->max_udp_size = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 488:
#line 2284 "./util/configparser.y"
    {
		OUTYY(("P(dns64_prefix:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dns64_prefix);
		cfg_parser->cfg->dns64_prefix = (yyvsp[(2) - (2)].str);
	}
    break;

  case 489:
#line 2291 "./util/configparser.y"
    {
		OUTYY(("P(server_dns64_synthall:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dns64_synthall = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 490:
#line 2300 "./util/configparser.y"
    {
		OUTYY(("P(dns64_ignore_aaaa:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dns64_ignore_aaaa,
			(yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding dns64-ignore-aaaa");
	}
    break;

  case 491:
#line 2308 "./util/configparser.y"
    {
		char* p, *s = (yyvsp[(2) - (2)].str);
		OUTYY(("P(server_define_tag:%s)\n", (yyvsp[(2) - (2)].str)));
		while((p=strsep(&s, " \t\n")) != NULL) {
			if(*p) {
				if(!config_add_tag(cfg_parser->cfg, p))
					yyerror("could not define-tag, "
						"out of memory");
			}
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 492:
#line 2322 "./util/configparser.y"
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[(3) - (3)].str),
			&len);
		free((yyvsp[(3) - (3)].str));
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[(2) - (3)].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[(2) - (3)].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->local_zone_tags,
				(yyvsp[(2) - (3)].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[(2) - (3)].str));
			}
		}
	}
    break;

  case 493:
#line 2343 "./util/configparser.y"
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[(3) - (3)].str),
			&len);
		free((yyvsp[(3) - (3)].str));
		OUTYY(("P(server_access_control_tag:%s)\n", (yyvsp[(2) - (3)].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[(2) - (3)].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->acl_tags,
				(yyvsp[(2) - (3)].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[(2) - (3)].str));
			}
		}
	}
    break;

  case 494:
#line 2364 "./util/configparser.y"
    {
		OUTYY(("P(server_access_control_tag_action:%s %s %s)\n", (yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_actions,
			(yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str))) {
			yyerror("out of memory");
			free((yyvsp[(2) - (4)].str));
			free((yyvsp[(3) - (4)].str));
			free((yyvsp[(4) - (4)].str));
		}
	}
    break;

  case 495:
#line 2376 "./util/configparser.y"
    {
		OUTYY(("P(server_access_control_tag_data:%s %s %s)\n", (yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_datas,
			(yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str))) {
			yyerror("out of memory");
			free((yyvsp[(2) - (4)].str));
			free((yyvsp[(3) - (4)].str));
			free((yyvsp[(4) - (4)].str));
		}
	}
    break;

  case 496:
#line 2388 "./util/configparser.y"
    {
		OUTYY(("P(server_local_zone_override:%s %s %s)\n", (yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->local_zone_overrides,
			(yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].str))) {
			yyerror("out of memory");
			free((yyvsp[(2) - (4)].str));
			free((yyvsp[(3) - (4)].str));
			free((yyvsp[(4) - (4)].str));
		}
	}
    break;

  case 497:
#line 2400 "./util/configparser.y"
    {
		OUTYY(("P(server_access_control_view:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acl_view,
			(yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str))) {
			yyerror("out of memory");
		}
	}
    break;

  case 498:
#line 2409 "./util/configparser.y"
    {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[(3) - (3)].str),
			&len);
		free((yyvsp[(3) - (3)].str));
		OUTYY(("P(response_ip_tag:%s)\n", (yyvsp[(2) - (3)].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[(2) - (3)].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->respip_tags,
				(yyvsp[(2) - (3)].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[(2) - (3)].str));
			}
		}
	}
    break;

  case 499:
#line 2430 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_ratelimit:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 500:
#line 2439 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 501:
#line 2448 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_ratelimit_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->ip_ratelimit_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 502:
#line 2456 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->ratelimit_size))
			yyerror("memory size expected");
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 503:
#line 2464 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_ratelimit_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->ip_ratelimit_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->ip_ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 504:
#line 2477 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->ratelimit_slabs = atoi((yyvsp[(2) - (2)].str));
			if(!is_pow2(cfg_parser->cfg->ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 505:
#line 2490 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_for_domain:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(atoi((yyvsp[(3) - (3)].str)) == 0 && strcmp((yyvsp[(3) - (3)].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[(2) - (3)].str));
			free((yyvsp[(3) - (3)].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_for_domain, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding "
					"ratelimit-for-domain");
		}
	}
    break;

  case 506:
#line 2505 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_below_domain:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(atoi((yyvsp[(3) - (3)].str)) == 0 && strcmp((yyvsp[(3) - (3)].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[(2) - (3)].str));
			free((yyvsp[(3) - (3)].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_below_domain, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding "
					"ratelimit-below-domain");
		}
	}
    break;

  case 507:
#line 2520 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_ratelimit_factor:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit_factor = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 508:
#line 2529 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_factor:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit_factor = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 509:
#line 2538 "./util/configparser.y"
    {
		OUTYY(("P(server_ip_ratelimit_backoff:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_ratelimit_backoff =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 510:
#line 2548 "./util/configparser.y"
    {
		OUTYY(("P(server_ratelimit_backoff:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ratelimit_backoff =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 511:
#line 2558 "./util/configparser.y"
    {
		OUTYY(("P(server_outbound_msg_retry:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outbound_msg_retry = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 512:
#line 2567 "./util/configparser.y"
    {
		OUTYY(("P(low-rtt option is deprecated, use fast-server-num instead)\n"));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 513:
#line 2573 "./util/configparser.y"
    {
		OUTYY(("P(server_fast_server_num:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) <= 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_num = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 514:
#line 2582 "./util/configparser.y"
    {
		OUTYY(("P(server_fast_server_permil:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_permil = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 515:
#line 2591 "./util/configparser.y"
    {
		OUTYY(("P(server_qname_minimisation:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 516:
#line 2601 "./util/configparser.y"
    {
		OUTYY(("P(server_qname_minimisation_strict:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation_strict =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 517:
#line 2611 "./util/configparser.y"
    {
		OUTYY(("P(server_pad_responses:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->pad_responses = 
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 518:
#line 2621 "./util/configparser.y"
    {
		OUTYY(("P(server_pad_responses_block_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->pad_responses_block_size = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 519:
#line 2630 "./util/configparser.y"
    {
		OUTYY(("P(server_pad_queries:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->pad_queries = 
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 520:
#line 2640 "./util/configparser.y"
    {
		OUTYY(("P(server_pad_queries_block_size:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->pad_queries_block_size = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 521:
#line 2649 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_enabled:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_enabled = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 522:
#line 2662 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_ignore_bogus:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_ignore_bogus = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 523:
#line 2675 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_hook:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->ipsecmod_hook);
		cfg_parser->cfg->ipsecmod_hook = (yyvsp[(2) - (2)].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 524:
#line 2687 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_max_ttl:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ipsecmod_max_ttl = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 525:
#line 2701 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_whitelist:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->ipsecmod_whitelist, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 526:
#line 2713 "./util/configparser.y"
    {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_strict:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_strict = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 527:
#line 2727 "./util/configparser.y"
    {
		OUTYY(("P(server_edns_client_string:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->edns_client_strings, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
			fatal_exit("out of memory adding "
				"edns-client-string");
	}
    break;

  case 528:
#line 2736 "./util/configparser.y"
    {
		OUTYY(("P(edns_client_string_opcode:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0 && strcmp((yyvsp[(2) - (2)].str), "0") != 0)
			yyerror("option code expected");
		else if(atoi((yyvsp[(2) - (2)].str)) > 65535 || atoi((yyvsp[(2) - (2)].str)) < 0)
			yyerror("option code must be in interval [0, 65535]");
		else cfg_parser->cfg->edns_client_string_opcode = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 529:
#line 2747 "./util/configparser.y"
    {
		OUTYY(("P(server_do_ede:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ede = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 530:
#line 2756 "./util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 531:
#line 2766 "./util/configparser.y"
    {
		OUTYY(("P(stub-host:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 532:
#line 2773 "./util/configparser.y"
    {
		OUTYY(("P(stub-addr:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 533:
#line 2780 "./util/configparser.y"
    {
		OUTYY(("P(stub-first:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isfirst=(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 534:
#line 2789 "./util/configparser.y"
    {
		OUTYY(("P(stub-no-cache:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->no_cache=(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 535:
#line 2798 "./util/configparser.y"
    {
		OUTYY(("P(stub-ssl-upstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->ssl_upstream =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 536:
#line 2808 "./util/configparser.y"
    {
                OUTYY(("P(stub-tcp-upstream:%s)\n", (yyvsp[(2) - (2)].str)));
                if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
                        yyerror("expected yes or no.");
                else cfg_parser->cfg->stubs->tcp_upstream =
                        (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
                free((yyvsp[(2) - (2)].str));
        }
    break;

  case 537:
#line 2818 "./util/configparser.y"
    {
		OUTYY(("P(stub-prime:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 538:
#line 2828 "./util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 539:
#line 2838 "./util/configparser.y"
    {
		OUTYY(("P(forward-host:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 540:
#line 2845 "./util/configparser.y"
    {
		OUTYY(("P(forward-addr:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 541:
#line 2852 "./util/configparser.y"
    {
		OUTYY(("P(forward-first:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->isfirst=(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 542:
#line 2861 "./util/configparser.y"
    {
		OUTYY(("P(forward-no-cache:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->no_cache=(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 543:
#line 2870 "./util/configparser.y"
    {
		OUTYY(("P(forward-ssl-upstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->ssl_upstream =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 544:
#line 2880 "./util/configparser.y"
    {
                OUTYY(("P(forward-tcp-upstream:%s)\n", (yyvsp[(2) - (2)].str)));
                if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
                        yyerror("expected yes or no.");
                else cfg_parser->cfg->forwards->tcp_upstream =
                        (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
                free((yyvsp[(2) - (2)].str));
        }
    break;

  case 545:
#line 2890 "./util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 546:
#line 2900 "./util/configparser.y"
    {
		OUTYY(("P(zonefile:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = (yyvsp[(2) - (2)].str);
	}
    break;

  case 547:
#line 2907 "./util/configparser.y"
    {
		OUTYY(("P(master:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 548:
#line 2914 "./util/configparser.y"
    {
		OUTYY(("P(url:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 549:
#line 2921 "./util/configparser.y"
    {
		OUTYY(("P(allow-notify:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			(yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 550:
#line 2929 "./util/configparser.y"
    {
		OUTYY(("P(zonemd-check:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->zonemd_check =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 551:
#line 2939 "./util/configparser.y"
    {
		OUTYY(("P(zonemd-reject-absence:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->zonemd_reject_absence =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 552:
#line 2949 "./util/configparser.y"
    {
		OUTYY(("P(for-downstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_downstream =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 553:
#line 2959 "./util/configparser.y"
    {
		OUTYY(("P(for-upstream:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_upstream =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 554:
#line 2969 "./util/configparser.y"
    {
		OUTYY(("P(fallback-enabled:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->fallback_enabled =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 555:
#line 2979 "./util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_parser->cfg->views->name)
			yyerror("view name override, there must be one "
				"name for one view");
		free(cfg_parser->cfg->views->name);
		cfg_parser->cfg->views->name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 556:
#line 2989 "./util/configparser.y"
    {
		OUTYY(("P(view_local_zone:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if(strcmp((yyvsp[(3) - (3)].str), "static")!=0 && strcmp((yyvsp[(3) - (3)].str), "deny")!=0 &&
		   strcmp((yyvsp[(3) - (3)].str), "refuse")!=0 && strcmp((yyvsp[(3) - (3)].str), "redirect")!=0 &&
		   strcmp((yyvsp[(3) - (3)].str), "transparent")!=0 && strcmp((yyvsp[(3) - (3)].str), "nodefault")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "typetransparent")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_transparent")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_refuse")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_nodata")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_deny")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "always_null")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "noview")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "inform")!=0 && strcmp((yyvsp[(3) - (3)].str), "inform_deny")!=0
		   && strcmp((yyvsp[(3) - (3)].str), "inform_redirect") != 0
		   && strcmp((yyvsp[(3) - (3)].str), "ipset") != 0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"inform_redirect, always_transparent, "
				"always_refuse, always_nxdomain, "
				"always_nodata, always_deny, always_null, "
				"noview, nodefault or ipset");
			free((yyvsp[(2) - (3)].str));
			free((yyvsp[(3) - (3)].str));
		} else if(strcmp((yyvsp[(3) - (3)].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_zones_nodefault, (yyvsp[(2) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[(3) - (3)].str));
#ifdef USE_IPSET
		} else if(strcmp((yyvsp[(3) - (3)].str), "ipset")==0) {
			size_t len = strlen((yyvsp[(2) - (3)].str));
			/* Make sure to add the trailing dot.
			 * These are str compared to domain names. */
			if((yyvsp[(2) - (3)].str)[len-1] != '.') {
				if(!((yyvsp[(2) - (3)].str) = realloc((yyvsp[(2) - (3)].str), len+2))) {
					fatal_exit("out of memory adding local-zone");
				}
				(yyvsp[(2) - (3)].str)[len] = '.';
				(yyvsp[(2) - (3)].str)[len+1] = 0;
			}
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_zones_ipset, (yyvsp[(2) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[(3) - (3)].str));
#endif
		} else {
			if(!cfg_str2list_insert(
				&cfg_parser->cfg->views->local_zones,
				(yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
    break;

  case 557:
#line 3045 "./util/configparser.y"
    {
		OUTYY(("P(view_response_ip:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		validate_respip_action((yyvsp[(3) - (3)].str));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_actions, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
			fatal_exit("out of memory adding per-view "
				"response-ip action");
	}
    break;

  case 558:
#line 3055 "./util/configparser.y"
    {
		OUTYY(("P(view_response_ip_data:%s)\n", (yyvsp[(2) - (3)].str)));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_data, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
			fatal_exit("out of memory adding response-ip-data");
	}
    break;

  case 559:
#line 3063 "./util/configparser.y"
    {
		OUTYY(("P(view_local_data:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->views->local_data, (yyvsp[(2) - (2)].str))) {
			fatal_exit("out of memory adding local-data");
		}
	}
    break;

  case 560:
#line 3071 "./util/configparser.y"
    {
		char* ptr;
		OUTYY(("P(view_local_data_ptr:%s)\n", (yyvsp[(2) - (2)].str)));
		ptr = cfg_ptr_reverse((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
    break;

  case 561:
#line 3086 "./util/configparser.y"
    {
		OUTYY(("P(view-first:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->views->isfirst=(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 562:
#line 3095 "./util/configparser.y"
    {
		OUTYY(("\nP(remote-control:)\n"));
	}
    break;

  case 573:
#line 3106 "./util/configparser.y"
    {
		OUTYY(("P(control_enable:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 574:
#line 3116 "./util/configparser.y"
    {
		OUTYY(("P(control_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 575:
#line 3125 "./util/configparser.y"
    {
		OUTYY(("P(control_interface:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 576:
#line 3132 "./util/configparser.y"
    {
		OUTYY(("P(control_use_cert:%s)\n", (yyvsp[(2) - (2)].str)));
		cfg_parser->cfg->control_use_cert = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 577:
#line 3139 "./util/configparser.y"
    {
		OUTYY(("P(rc_server_key_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 578:
#line 3146 "./util/configparser.y"
    {
		OUTYY(("P(rc_server_cert_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 579:
#line 3153 "./util/configparser.y"
    {
		OUTYY(("P(rc_control_key_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 580:
#line 3160 "./util/configparser.y"
    {
		OUTYY(("P(rc_control_cert_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 581:
#line 3167 "./util/configparser.y"
    {
		OUTYY(("\nP(dnstap:)\n"));
	}
    break;

  case 603:
#line 3187 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_enable:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 604:
#line 3196 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_bidirectional:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_bidirectional =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 605:
#line 3206 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_socket_path:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_socket_path);
		cfg_parser->cfg->dnstap_socket_path = (yyvsp[(2) - (2)].str);
	}
    break;

  case 606:
#line 3213 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_ip:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_ip);
		cfg_parser->cfg->dnstap_ip = (yyvsp[(2) - (2)].str);
	}
    break;

  case 607:
#line 3220 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_tls:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_tls = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 608:
#line 3229 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_tls_server_name:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_tls_server_name);
		cfg_parser->cfg->dnstap_tls_server_name = (yyvsp[(2) - (2)].str);
	}
    break;

  case 609:
#line 3236 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_tls_cert_bundle:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_tls_cert_bundle);
		cfg_parser->cfg->dnstap_tls_cert_bundle = (yyvsp[(2) - (2)].str);
	}
    break;

  case 610:
#line 3243 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_tls_client_key_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_tls_client_key_file);
		cfg_parser->cfg->dnstap_tls_client_key_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 611:
#line 3250 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_tls_client_cert_file:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_tls_client_cert_file);
		cfg_parser->cfg->dnstap_tls_client_cert_file = (yyvsp[(2) - (2)].str);
	}
    break;

  case 612:
#line 3257 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_send_identity:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_identity = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 613:
#line 3266 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_send_version:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_version = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 614:
#line 3275 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_identity:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_identity);
		cfg_parser->cfg->dnstap_identity = (yyvsp[(2) - (2)].str);
	}
    break;

  case 615:
#line 3282 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_version:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnstap_version);
		cfg_parser->cfg->dnstap_version = (yyvsp[(2) - (2)].str);
	}
    break;

  case 616:
#line 3289 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_resolver_query_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_query_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 617:
#line 3299 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_resolver_response_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_response_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 618:
#line 3309 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_client_query_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_query_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 619:
#line 3319 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_client_response_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_response_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 620:
#line 3329 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_forwarder_query_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_query_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 621:
#line 3339 "./util/configparser.y"
    {
		OUTYY(("P(dt_dnstap_log_forwarder_response_messages:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_response_messages =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 622:
#line 3349 "./util/configparser.y"
    {
		OUTYY(("\nP(python:)\n"));
	}
    break;

  case 626:
#line 3358 "./util/configparser.y"
    {
		OUTYY(("P(python-script:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->python_script, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 627:
#line 3364 "./util/configparser.y"
    { 
		OUTYY(("\nP(dynlib:)\n")); 
	}
    break;

  case 631:
#line 3373 "./util/configparser.y"
    {
		OUTYY(("P(dynlib-file:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->dynlib_file, (yyvsp[(2) - (2)].str)))
			yyerror("out of memory");
	}
    break;

  case 632:
#line 3379 "./util/configparser.y"
    {
		OUTYY(("P(disable_dnssec_lame_check:%s)\n", (yyvsp[(2) - (2)].str)));
		if (strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_dnssec_lame_check =
			(strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 633:
#line 3389 "./util/configparser.y"
    {
		OUTYY(("P(server_log_identity:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = (yyvsp[(2) - (2)].str);
	}
    break;

  case 634:
#line 3396 "./util/configparser.y"
    {
		OUTYY(("P(server_response_ip:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		validate_respip_action((yyvsp[(3) - (3)].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_actions,
			(yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
			fatal_exit("out of memory adding response-ip");
	}
    break;

  case 635:
#line 3405 "./util/configparser.y"
    {
		OUTYY(("P(server_response_ip_data:%s)\n", (yyvsp[(2) - (3)].str)));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_data,
			(yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
			fatal_exit("out of memory adding response-ip-data");
	}
    break;

  case 636:
#line 3413 "./util/configparser.y"
    {
		OUTYY(("\nP(dnscrypt:)\n"));
	}
    break;

  case 649:
#line 3429 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_enable:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnscrypt = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 650:
#line 3439 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_port:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->dnscrypt_port = atoi((yyvsp[(2) - (2)].str));
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 651:
#line 3448 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_provider:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->dnscrypt_provider);
		cfg_parser->cfg->dnscrypt_provider = (yyvsp[(2) - (2)].str);
	}
    break;

  case 652:
#line 3455 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_provider_cert:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[(2) - (2)].str)))
			log_warn("dnscrypt-provider-cert %s is a duplicate", (yyvsp[(2) - (2)].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert, (yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert");
	}
    break;

  case 653:
#line 3464 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_provider_cert_rotated:%s)\n", (yyvsp[(2) - (2)].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert_rotated, (yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding dnscrypt-provider-cert-rotated");
	}
    break;

  case 654:
#line 3471 "./util/configparser.y"
    {
		OUTYY(("P(dnsc_dnscrypt_secret_key:%s)\n", (yyvsp[(2) - (2)].str)));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[(2) - (2)].str)))
			log_warn("dnscrypt-secret-key: %s is a duplicate", (yyvsp[(2) - (2)].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_secret_key, (yyvsp[(2) - (2)].str)))
			fatal_exit("out of memory adding dnscrypt-secret-key");
	}
    break;

  case 655:
#line 3480 "./util/configparser.y"
    {
	OUTYY(("P(dnscrypt_shared_secret_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
	if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->dnscrypt_shared_secret_cache_size))
		yyerror("memory size expected");
	free((yyvsp[(2) - (2)].str));
  }
    break;

  case 656:
#line 3488 "./util/configparser.y"
    {
	OUTYY(("P(dnscrypt_shared_secret_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
	if(atoi((yyvsp[(2) - (2)].str)) == 0) {
		yyerror("number expected");
	} else {
		cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs = atoi((yyvsp[(2) - (2)].str));
		if(!is_pow2(cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs))
			yyerror("must be a power of 2");
	}
	free((yyvsp[(2) - (2)].str));
  }
    break;

  case 657:
#line 3501 "./util/configparser.y"
    {
	OUTYY(("P(dnscrypt_nonce_cache_size:%s)\n", (yyvsp[(2) - (2)].str)));
	if(!cfg_parse_memsize((yyvsp[(2) - (2)].str), &cfg_parser->cfg->dnscrypt_nonce_cache_size))
		yyerror("memory size expected");
	free((yyvsp[(2) - (2)].str));
  }
    break;

  case 658:
#line 3509 "./util/configparser.y"
    {
	OUTYY(("P(dnscrypt_nonce_cache_slabs:%s)\n", (yyvsp[(2) - (2)].str)));
	if(atoi((yyvsp[(2) - (2)].str)) == 0) {
		yyerror("number expected");
	} else {
		cfg_parser->cfg->dnscrypt_nonce_cache_slabs = atoi((yyvsp[(2) - (2)].str));
		if(!is_pow2(cfg_parser->cfg->dnscrypt_nonce_cache_slabs))
			yyerror("must be a power of 2");
	}
	free((yyvsp[(2) - (2)].str));
  }
    break;

  case 659:
#line 3522 "./util/configparser.y"
    {
		OUTYY(("\nP(cachedb:)\n"));
	}
    break;

  case 668:
#line 3533 "./util/configparser.y"
    {
	#ifdef USE_CACHEDB
		OUTYY(("P(backend:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->cachedb_backend);
		cfg_parser->cfg->cachedb_backend = (yyvsp[(2) - (2)].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 669:
#line 3545 "./util/configparser.y"
    {
	#ifdef USE_CACHEDB
		OUTYY(("P(secret-seed:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->cachedb_secret);
		cfg_parser->cfg->cachedb_secret = (yyvsp[(2) - (2)].str);
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 670:
#line 3557 "./util/configparser.y"
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_server_host:%s)\n", (yyvsp[(2) - (2)].str)));
		free(cfg_parser->cfg->redis_server_host);
		cfg_parser->cfg->redis_server_host = (yyvsp[(2) - (2)].str);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
		free((yyvsp[(2) - (2)].str));
	#endif
	}
    break;

  case 671:
#line 3569 "./util/configparser.y"
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		int port;
		OUTYY(("P(redis_server_port:%s)\n", (yyvsp[(2) - (2)].str)));
		port = atoi((yyvsp[(2) - (2)].str));
		if(port == 0 || port < 0 || port > 65535)
			yyerror("valid redis server port number expected");
		else cfg_parser->cfg->redis_server_port = port;
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 672:
#line 3584 "./util/configparser.y"
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_timeout:%s)\n", (yyvsp[(2) - (2)].str)));
		if(atoi((yyvsp[(2) - (2)].str)) == 0)
			yyerror("redis timeout value expected");
		else cfg_parser->cfg->redis_timeout = atoi((yyvsp[(2) - (2)].str));
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 673:
#line 3597 "./util/configparser.y"
    {
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_expire_records:%s)\n", (yyvsp[(2) - (2)].str)));
		if(strcmp((yyvsp[(2) - (2)].str), "yes") != 0 && strcmp((yyvsp[(2) - (2)].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->redis_expire_records = (strcmp((yyvsp[(2) - (2)].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free((yyvsp[(2) - (2)].str));
	}
    break;

  case 674:
#line 3610 "./util/configparser.y"
    {
		OUTYY(("P(server_tcp_connection_limit:%s %s)\n", (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)));
		if (atoi((yyvsp[(3) - (3)].str)) < 0)
			yyerror("positive number expected");
		else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->tcp_connection_limits, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str)))
				fatal_exit("out of memory adding tcp connection limit");
		}
	}
    break;

  case 675:
#line 3621 "./util/configparser.y"
    {
			OUTYY(("\nP(ipset:)\n"));
		}
    break;

  case 680:
#line 3630 "./util/configparser.y"
    {
		#ifdef USE_IPSET
			OUTYY(("P(name-v4:%s)\n", (yyvsp[(2) - (2)].str)));
			if(cfg_parser->cfg->ipset_name_v4)
				yyerror("ipset name v4 override, there must be one "
					"name for ip v4");
			free(cfg_parser->cfg->ipset_name_v4);
			cfg_parser->cfg->ipset_name_v4 = (yyvsp[(2) - (2)].str);
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free((yyvsp[(2) - (2)].str));
		#endif
		}
    break;

  case 681:
#line 3645 "./util/configparser.y"
    {
		#ifdef USE_IPSET
			OUTYY(("P(name-v6:%s)\n", (yyvsp[(2) - (2)].str)));
			if(cfg_parser->cfg->ipset_name_v6)
				yyerror("ipset name v6 override, there must be one "
					"name for ip v6");
			free(cfg_parser->cfg->ipset_name_v6);
			cfg_parser->cfg->ipset_name_v6 = (yyvsp[(2) - (2)].str);
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free((yyvsp[(2) - (2)].str));
		#endif
		}
    break;


/* Line 1267 of yacc.c.  */
#line 7175 "util/configparser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 3659 "./util/configparser.y"


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



