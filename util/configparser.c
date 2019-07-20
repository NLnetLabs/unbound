/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20140715

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 39 "./util/configparser.y"
#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "sldns/str2wire.h"

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

#line 68 "./util/configparser.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
	char*	str;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 61 "util/configparser.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define SPACE 257
#define LETTER 258
#define NEWLINE 259
#define COMMENT 260
#define COLON 261
#define ANY 262
#define ZONESTR 263
#define STRING_ARG 264
#define VAR_SERVER 265
#define VAR_VERBOSITY 266
#define VAR_NUM_THREADS 267
#define VAR_PORT 268
#define VAR_OUTGOING_RANGE 269
#define VAR_INTERFACE 270
#define VAR_DO_IP4 271
#define VAR_DO_IP6 272
#define VAR_PREFER_IP6 273
#define VAR_DO_UDP 274
#define VAR_DO_TCP 275
#define VAR_TCP_MSS 276
#define VAR_OUTGOING_TCP_MSS 277
#define VAR_TCP_IDLE_TIMEOUT 278
#define VAR_EDNS_TCP_KEEPALIVE 279
#define VAR_EDNS_TCP_KEEPALIVE_TIMEOUT 280
#define VAR_CHROOT 281
#define VAR_USERNAME 282
#define VAR_DIRECTORY 283
#define VAR_LOGFILE 284
#define VAR_PIDFILE 285
#define VAR_MSG_CACHE_SIZE 286
#define VAR_MSG_CACHE_SLABS 287
#define VAR_NUM_QUERIES_PER_THREAD 288
#define VAR_RRSET_CACHE_SIZE 289
#define VAR_RRSET_CACHE_SLABS 290
#define VAR_OUTGOING_NUM_TCP 291
#define VAR_INFRA_HOST_TTL 292
#define VAR_INFRA_LAME_TTL 293
#define VAR_INFRA_CACHE_SLABS 294
#define VAR_INFRA_CACHE_NUMHOSTS 295
#define VAR_INFRA_CACHE_LAME_SIZE 296
#define VAR_NAME 297
#define VAR_STUB_ZONE 298
#define VAR_STUB_HOST 299
#define VAR_STUB_ADDR 300
#define VAR_TARGET_FETCH_POLICY 301
#define VAR_HARDEN_SHORT_BUFSIZE 302
#define VAR_HARDEN_LARGE_QUERIES 303
#define VAR_FORWARD_ZONE 304
#define VAR_FORWARD_HOST 305
#define VAR_FORWARD_ADDR 306
#define VAR_DO_NOT_QUERY_ADDRESS 307
#define VAR_HIDE_IDENTITY 308
#define VAR_HIDE_VERSION 309
#define VAR_IDENTITY 310
#define VAR_VERSION 311
#define VAR_HARDEN_GLUE 312
#define VAR_MODULE_CONF 313
#define VAR_TRUST_ANCHOR_FILE 314
#define VAR_TRUST_ANCHOR 315
#define VAR_VAL_OVERRIDE_DATE 316
#define VAR_BOGUS_TTL 317
#define VAR_VAL_CLEAN_ADDITIONAL 318
#define VAR_VAL_PERMISSIVE_MODE 319
#define VAR_INCOMING_NUM_TCP 320
#define VAR_MSG_BUFFER_SIZE 321
#define VAR_KEY_CACHE_SIZE 322
#define VAR_KEY_CACHE_SLABS 323
#define VAR_TRUSTED_KEYS_FILE 324
#define VAR_VAL_NSEC3_KEYSIZE_ITERATIONS 325
#define VAR_USE_SYSLOG 326
#define VAR_OUTGOING_INTERFACE 327
#define VAR_ROOT_HINTS 328
#define VAR_DO_NOT_QUERY_LOCALHOST 329
#define VAR_CACHE_MAX_TTL 330
#define VAR_HARDEN_DNSSEC_STRIPPED 331
#define VAR_ACCESS_CONTROL 332
#define VAR_LOCAL_ZONE 333
#define VAR_LOCAL_DATA 334
#define VAR_INTERFACE_AUTOMATIC 335
#define VAR_STATISTICS_INTERVAL 336
#define VAR_DO_DAEMONIZE 337
#define VAR_USE_CAPS_FOR_ID 338
#define VAR_STATISTICS_CUMULATIVE 339
#define VAR_OUTGOING_PORT_PERMIT 340
#define VAR_OUTGOING_PORT_AVOID 341
#define VAR_DLV_ANCHOR_FILE 342
#define VAR_DLV_ANCHOR 343
#define VAR_NEG_CACHE_SIZE 344
#define VAR_HARDEN_REFERRAL_PATH 345
#define VAR_PRIVATE_ADDRESS 346
#define VAR_PRIVATE_DOMAIN 347
#define VAR_REMOTE_CONTROL 348
#define VAR_CONTROL_ENABLE 349
#define VAR_CONTROL_INTERFACE 350
#define VAR_CONTROL_PORT 351
#define VAR_SERVER_KEY_FILE 352
#define VAR_SERVER_CERT_FILE 353
#define VAR_CONTROL_KEY_FILE 354
#define VAR_CONTROL_CERT_FILE 355
#define VAR_CONTROL_USE_CERT 356
#define VAR_EXTENDED_STATISTICS 357
#define VAR_LOCAL_DATA_PTR 358
#define VAR_JOSTLE_TIMEOUT 359
#define VAR_STUB_PRIME 360
#define VAR_UNWANTED_REPLY_THRESHOLD 361
#define VAR_LOG_TIME_ASCII 362
#define VAR_DOMAIN_INSECURE 363
#define VAR_PYTHON 364
#define VAR_PYTHON_SCRIPT 365
#define VAR_VAL_SIG_SKEW_MIN 366
#define VAR_VAL_SIG_SKEW_MAX 367
#define VAR_CACHE_MIN_TTL 368
#define VAR_VAL_LOG_LEVEL 369
#define VAR_AUTO_TRUST_ANCHOR_FILE 370
#define VAR_KEEP_MISSING 371
#define VAR_ADD_HOLDDOWN 372
#define VAR_DEL_HOLDDOWN 373
#define VAR_SO_RCVBUF 374
#define VAR_EDNS_BUFFER_SIZE 375
#define VAR_PREFETCH 376
#define VAR_PREFETCH_KEY 377
#define VAR_SO_SNDBUF 378
#define VAR_SO_REUSEPORT 379
#define VAR_HARDEN_BELOW_NXDOMAIN 380
#define VAR_IGNORE_CD_FLAG 381
#define VAR_LOG_QUERIES 382
#define VAR_LOG_REPLIES 383
#define VAR_LOG_LOCAL_ACTIONS 384
#define VAR_TCP_UPSTREAM 385
#define VAR_SSL_UPSTREAM 386
#define VAR_SSL_SERVICE_KEY 387
#define VAR_SSL_SERVICE_PEM 388
#define VAR_SSL_PORT 389
#define VAR_FORWARD_FIRST 390
#define VAR_STUB_SSL_UPSTREAM 391
#define VAR_FORWARD_SSL_UPSTREAM 392
#define VAR_TLS_CERT_BUNDLE 393
#define VAR_STUB_FIRST 394
#define VAR_MINIMAL_RESPONSES 395
#define VAR_RRSET_ROUNDROBIN 396
#define VAR_MAX_UDP_SIZE 397
#define VAR_DELAY_CLOSE 398
#define VAR_UNBLOCK_LAN_ZONES 399
#define VAR_INSECURE_LAN_ZONES 400
#define VAR_INFRA_CACHE_MIN_RTT 401
#define VAR_DNS64_PREFIX 402
#define VAR_DNS64_SYNTHALL 403
#define VAR_DNS64_IGNORE_AAAA 404
#define VAR_DNSTAP 405
#define VAR_DNSTAP_ENABLE 406
#define VAR_DNSTAP_SOCKET_PATH 407
#define VAR_DNSTAP_SEND_IDENTITY 408
#define VAR_DNSTAP_SEND_VERSION 409
#define VAR_DNSTAP_IDENTITY 410
#define VAR_DNSTAP_VERSION 411
#define VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES 412
#define VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES 413
#define VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES 414
#define VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES 415
#define VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES 416
#define VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES 417
#define VAR_RESPONSE_IP_TAG 418
#define VAR_RESPONSE_IP 419
#define VAR_RESPONSE_IP_DATA 420
#define VAR_HARDEN_ALGO_DOWNGRADE 421
#define VAR_IP_TRANSPARENT 422
#define VAR_DISABLE_DNSSEC_LAME_CHECK 423
#define VAR_IP_RATELIMIT 424
#define VAR_IP_RATELIMIT_SLABS 425
#define VAR_IP_RATELIMIT_SIZE 426
#define VAR_RATELIMIT 427
#define VAR_RATELIMIT_SLABS 428
#define VAR_RATELIMIT_SIZE 429
#define VAR_RATELIMIT_FOR_DOMAIN 430
#define VAR_RATELIMIT_BELOW_DOMAIN 431
#define VAR_IP_RATELIMIT_FACTOR 432
#define VAR_RATELIMIT_FACTOR 433
#define VAR_SEND_CLIENT_SUBNET 434
#define VAR_CLIENT_SUBNET_ZONE 435
#define VAR_CLIENT_SUBNET_ALWAYS_FORWARD 436
#define VAR_CLIENT_SUBNET_OPCODE 437
#define VAR_MAX_CLIENT_SUBNET_IPV4 438
#define VAR_MAX_CLIENT_SUBNET_IPV6 439
#define VAR_MIN_CLIENT_SUBNET_IPV4 440
#define VAR_MIN_CLIENT_SUBNET_IPV6 441
#define VAR_MAX_ECS_TREE_SIZE_IPV4 442
#define VAR_MAX_ECS_TREE_SIZE_IPV6 443
#define VAR_CAPS_WHITELIST 444
#define VAR_CACHE_MAX_NEGATIVE_TTL 445
#define VAR_PERMIT_SMALL_HOLDDOWN 446
#define VAR_QNAME_MINIMISATION 447
#define VAR_QNAME_MINIMISATION_STRICT 448
#define VAR_IP_FREEBIND 449
#define VAR_DEFINE_TAG 450
#define VAR_LOCAL_ZONE_TAG 451
#define VAR_ACCESS_CONTROL_TAG 452
#define VAR_LOCAL_ZONE_OVERRIDE 453
#define VAR_ACCESS_CONTROL_TAG_ACTION 454
#define VAR_ACCESS_CONTROL_TAG_DATA 455
#define VAR_VIEW 456
#define VAR_ACCESS_CONTROL_VIEW 457
#define VAR_VIEW_FIRST 458
#define VAR_SERVE_EXPIRED 459
#define VAR_SERVE_EXPIRED_TTL 460
#define VAR_SERVE_EXPIRED_TTL_RESET 461
#define VAR_FAKE_DSA 462
#define VAR_FAKE_SHA1 463
#define VAR_LOG_IDENTITY 464
#define VAR_HIDE_TRUSTANCHOR 465
#define VAR_TRUST_ANCHOR_SIGNALING 466
#define VAR_AGGRESSIVE_NSEC 467
#define VAR_USE_SYSTEMD 468
#define VAR_SHM_ENABLE 469
#define VAR_SHM_KEY 470
#define VAR_ROOT_KEY_SENTINEL 471
#define VAR_DNSCRYPT 472
#define VAR_DNSCRYPT_ENABLE 473
#define VAR_DNSCRYPT_PORT 474
#define VAR_DNSCRYPT_PROVIDER 475
#define VAR_DNSCRYPT_SECRET_KEY 476
#define VAR_DNSCRYPT_PROVIDER_CERT 477
#define VAR_DNSCRYPT_PROVIDER_CERT_ROTATED 478
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE 479
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS 480
#define VAR_DNSCRYPT_NONCE_CACHE_SIZE 481
#define VAR_DNSCRYPT_NONCE_CACHE_SLABS 482
#define VAR_IPSECMOD_ENABLED 483
#define VAR_IPSECMOD_HOOK 484
#define VAR_IPSECMOD_IGNORE_BOGUS 485
#define VAR_IPSECMOD_MAX_TTL 486
#define VAR_IPSECMOD_WHITELIST 487
#define VAR_IPSECMOD_STRICT 488
#define VAR_CACHEDB 489
#define VAR_CACHEDB_BACKEND 490
#define VAR_CACHEDB_SECRETSEED 491
#define VAR_CACHEDB_REDISHOST 492
#define VAR_CACHEDB_REDISPORT 493
#define VAR_CACHEDB_REDISTIMEOUT 494
#define VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM 495
#define VAR_FOR_UPSTREAM 496
#define VAR_AUTH_ZONE 497
#define VAR_ZONEFILE 498
#define VAR_MASTER 499
#define VAR_URL 500
#define VAR_FOR_DOWNSTREAM 501
#define VAR_FALLBACK_ENABLED 502
#define VAR_TLS_ADDITIONAL_PORT 503
#define VAR_LOW_RTT 504
#define VAR_LOW_RTT_PERMIL 505
#define VAR_FAST_SERVER_PERMIL 506
#define VAR_FAST_SERVER_NUM 507
#define VAR_ALLOW_NOTIFY 508
#define VAR_TLS_WIN_CERT 509
#define VAR_TCP_CONNECTION_LIMIT 510
#define VAR_ANSWER_COOKIE 511
#define VAR_COOKIE_SECRET 512
#define VAR_FORWARD_NO_CACHE 513
#define VAR_STUB_NO_CACHE 514
#define VAR_LOG_SERVFAIL 515
#define VAR_DENY_ANY 516
#define VAR_UNKNOWN_SERVER_TIME_LIMIT 517
#define VAR_LOG_TAG_QUERYREPLY 518
#define VAR_STREAM_WAIT_SIZE 519
#define VAR_TLS_CIPHERS 520
#define VAR_TLS_CIPHERSUITES 521
#define VAR_TLS_SESSION_TICKET_KEYS 522
#define VAR_IPSET 523
#define VAR_IPSET_NAME_V4 524
#define VAR_IPSET_NAME_V6 525
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    3,    3,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,    4,    5,    5,
  216,  216,  216,  216,  216,  216,  216,    6,    7,    7,
  224,  224,  224,  224,  224,  224,   14,   15,   15,  231,
  231,  231,  231,  231,  231,  231,   22,   23,   23,  239,
  239,  239,  239,  239,  239,  239,  239,   25,   26,   88,
   91,  100,  185,  186,   27,  152,  153,  154,  155,  156,
  157,  158,  159,  160,  161,   39,   79,   28,   92,   93,
   50,   72,   87,   29,   30,   32,   33,   31,   34,   35,
   36,   37,   38,  123,  197,  124,  126,  127,  128,  199,
  204,  200,  211,  212,  213,  181,   89,   78,  104,  121,
  122,  209,  206,  125,   40,   41,   42,   43,   44,   80,
   94,   95,  110,   66,   76,   67,  189,  190,  105,   60,
   61,  188,   62,   63,  114,  118,  132,  141,  166,  210,
  115,   73,   45,   46,   47,  102,  133,  134,  135,   48,
   49,   51,   52,   54,   55,   53,  139,   56,   57,   58,
   64,   83,  119,   97,  140,   90,  162,   98,   99,  116,
  117,  207,  103,   59,   81,   84,   65,   68,  106,  107,
   82,  163,  108,   69,   70,   71,  198,  120,  176,  177,
  178,  179,  187,  109,   77,  111,  112,  113,  164,   74,
   75,   96,   85,   86,  101,  129,  130,  208,  131,  136,
  137,  138,  167,  168,  170,  172,  173,  171,  174,  182,
  142,  143,  146,  147,  144,  145,  148,  149,  151,  150,
  201,  203,  202,  165,  175,  191,  193,  192,  194,  195,
  196,  217,  218,  219,  221,  222,  223,  220,  225,  226,
  227,  228,  229,  230,  240,  241,  242,  243,  247,  244,
  245,  246,  232,  233,  236,  237,  234,  238,  235,   10,
   11,   11,  248,  248,  248,  248,  248,  248,  248,  248,
  249,  251,  250,  256,  252,  253,  254,  255,   12,   13,
   13,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  258,  259,  260,  261,  262,  263,  264,
  265,  266,  267,  268,  269,    8,    9,    9,  270,  271,
  169,  180,  183,  184,   16,   17,   17,  272,  272,  272,
  272,  272,  272,  272,  272,  272,  272,  273,  274,  275,
  277,  278,  276,  279,  280,  281,  282,   18,   19,   19,
  283,  283,  283,  283,  283,  284,  285,  286,  287,  288,
  205,  214,  215,   20,   21,   21,  289,  289,  290,  291,
};
static const YYINT yylen[] = {                            2,
    0,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    1,    2,    0,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    2,    0,
    1,    1,    1,    1,    1,    1,    1,    1,    2,    0,
    1,    1,    1,    1,    1,    1,    1,    2,    0,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    0,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    3,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    3,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    3,    3,    4,    4,    4,    3,    3,
    2,    2,    2,    2,    2,    2,    3,    3,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    3,    3,    3,    2,    2,    2,    1,
    2,    0,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    2,    2,    2,    2,    2,    2,    2,    1,    2,
    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    1,    2,    0,    1,    2,
    2,    2,    3,    3,    1,    2,    0,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    1,    2,    0,
    1,    1,    1,    1,    1,    2,    2,    2,    2,    2,
    3,    2,    2,    1,    2,    0,    1,    1,    2,    2,
};
static const YYINT yydefred[] = {                         1,
    0,   14,  208,  218,  460,  506,  479,  227,  515,  538,
  237,  554,    2,   16,  210,  220,  508,  462,  481,  229,
  517,  540,  556,  239,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   15,   17,   18,   19,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,
   40,   41,   42,   43,   44,   45,   46,   47,   48,   49,
   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,
   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,
   90,   91,   92,   93,   94,   95,   96,   97,   98,   99,
  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,
  110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
  120,  121,  122,  123,  124,  125,  126,  127,  128,  129,
  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
  140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
  160,  161,  162,  163,  164,  165,  166,  167,  168,  169,
  170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,
  190,  191,  192,  193,  194,  195,  196,  197,  198,  199,
  200,  201,  202,  203,  204,  205,  206,  207,    0,    0,
    0,    0,    0,    0,    0,  209,  211,  212,  213,  214,
  215,  216,  217,    0,    0,    0,    0,    0,    0,  219,
  221,  222,  223,  224,  225,  226,    0,  507,  509,    0,
    0,    0,    0,    0,    0,    0,    0,  461,  463,  464,
  465,  466,  467,  468,  469,  470,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  480,  482,
  483,  484,  485,  486,  487,  488,  489,  490,  491,  492,
  493,    0,    0,    0,    0,    0,    0,    0,  228,  230,
  231,  232,  233,  234,  235,  236,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  516,  518,  519,  520,
  521,  522,  523,  524,  525,  526,  527,    0,    0,    0,
    0,    0,  539,  541,  542,  543,  544,  545,    0,    0,
  555,  557,  558,    0,    0,    0,    0,    0,    0,    0,
    0,  238,  240,  241,  242,  243,  244,  245,  246,  247,
  249,  248,  255,  268,  266,  274,  275,  278,  276,  277,
  279,  280,  281,  282,  283,  305,  306,  307,  308,  309,
  333,  334,  335,  340,  341,  271,  342,  343,  346,  344,
  345,  348,  349,  350,  364,  320,  321,  323,  324,  351,
  367,  314,  316,  368,  374,  375,  376,  272,  332,  390,
  391,  315,  385,  298,  267,  310,  365,  371,  352,    0,
    0,  394,  273,  250,  297,  356,  251,  269,  270,  311,
  312,  392,  354,  358,  359,  252,  395,  336,  363,  299,
  319,  369,  370,  373,  384,  313,  388,  386,  387,  325,
  331,  360,  361,  326,  327,  353,  378,  300,  301,  304,
  284,  286,  287,  288,  289,  290,  396,  397,  399,  337,
  338,  339,  347,  400,  401,  402,    0,    0,    0,  355,
  328,  511,  411,  415,  413,  412,  416,  414,    0,    0,
  419,  420,  256,  257,  258,  259,  260,  261,  262,  263,
  264,  265,  357,  372,  389,  424,  425,  329,  403,    0,
    0,    0,    0,    0,    0,  379,  380,  381,  382,  383,
  512,  322,  317,  377,  296,  253,  254,  318,  426,  428,
  427,  429,  430,  431,  285,  292,  421,  423,  422,  291,
    0,  552,  553,  303,  362,  398,  302,  330,  293,  294,
  295,  432,  433,  434,  438,  437,  435,  436,  439,  440,
  441,  442,  444,  443,  510,  471,  473,  472,  475,  476,
  477,  478,  474,  494,  495,  496,  497,  498,  499,  500,
  501,  502,  503,  504,  505,  453,    0,  457,  458,    0,
    0,  459,  528,  529,  530,  533,  531,  532,  534,  535,
  536,  537,  546,  547,  548,  549,  550,  559,  560,  445,
  451,  446,  447,  448,  450,  452,  449,  366,  393,  410,
  513,  514,  417,  418,  404,  405,    0,    0,    0,  409,
  551,  454,  455,  456,  408,  406,  407,
};
static const YYINT yydgoto[] = {                          1,
   13,   14,   25,   15,   26,   16,   27,   17,   28,   18,
   29,   19,   30,   20,   31,   21,   32,   22,   33,   23,
   34,   24,   35,  227,  228,  229,  230,  231,  232,  233,
  234,  235,  236,  237,  238,  239,  240,  241,  242,  243,
  244,  245,  246,  247,  248,  249,  250,  251,  252,  253,
  254,  255,  256,  257,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  294,  295,  296,  297,  298,  299,  300,  301,  302,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  426,  427,  428,  429,  430,
  431,  432,  433,  440,  441,  442,  443,  444,  445,  446,
  499,  500,  501,  502,  503,  504,  505,  506,  552,  553,
  554,  555,  556,  557,  558,  559,  560,  458,  459,  460,
  461,  462,  463,  464,  465,  466,  479,  480,  481,  482,
  483,  484,  485,  486,  487,  488,  489,  490,  491,  448,
  449,  517,  518,  519,  520,  521,  522,  523,  524,  525,
  526,  527,  533,  534,  535,  536,  537,  538,  541,  542,
  543,
};
static const YYINT yysindex[] = {                         0,
 -265,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  335, -289, -284, -353, -264, -359,
 -288, -414, -464, -507, -290, -250, -245, -244, -241, -240,
 -233, -232, -230, -229, -228, -227, -226, -224, -223, -222,
 -221, -220, -195, -192, -191, -189, -188, -186, -185, -184,
 -183, -182, -180, -171, -170, -169, -168, -167, -166, -164,
 -163, -161, -160, -157, -155, -154, -153, -152, -151, -150,
 -149, -148, -147, -146, -145, -144, -143, -142, -141, -140,
 -139, -138, -137, -136, -135, -134, -131, -130, -129, -128,
 -127, -126, -125, -123, -122, -121, -120, -119, -118, -117,
 -116, -115, -114, -113, -112, -111, -110, -109, -108, -107,
 -106, -105, -104, -103, -102, -101, -100,  -99,  -98,  -97,
  -96,  -95,  -93,  -92,  -91,  -90,  -89,  -88,  -87,  -86,
  -85,  -84,  -83,  -82,  -81,  -80,  -79,  -78,  -77,  -76,
  -75,  -74,  -72,  -71,  -70,  -69,  -68,  -67,  -66,  -65,
  -64,  -63,  -62,  -61,  -60,  -59,  -51,  -50,  -49,  -48,
  -47,  -45,  -44,  -43,  -42,  -41,  -38,  -37,  -36,  -34,
  -33,  -31,  -30,  -29,  -28,  -27,  -26,  -25,  -24,  -23,
  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,
  -12,  -11,  -10,   -9,   -8,   -7,   -5,   -4,   -3,   -2,
   -1,    8,    9,   10,   11,   12,   13,   14,   18,   19,
   20,   21,   22,   23,   24,   27,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   28,   29,
   30,   31,   32,   33,   34,    0,    0,    0,    0,    0,
    0,    0,    0,   47,   48,   51,   52,   53,   54,    0,
    0,    0,    0,    0,    0,    0,   57,    0,    0,   58,
   60,   61,   62,   63,   64,   66,   67,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   68,   69,   70,   71,
   72,   73,   76,   79,   80,   81,   82,   83,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   84,   91,   92,   93,   94,   95,   96,    0,    0,
    0,    0,    0,    0,    0,    0,   97,   98,  107,  110,
  112,  113,  118,  119,  120,  121,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  122,  123,  124,
  126,  127,    0,    0,    0,    0,    0,    0,  128,  129,
    0,    0,    0,  130,  131,  132,  133,  134,  135,  136,
  137,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  138,
  139,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  140,  141,  148,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  149,  150,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  151,
  152,  153,  154,  155,  159,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  160,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  162,    0,    0,  163,
  164,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  165,  167,  168,    0,
    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    1,    2,    3,    4,    5,    6,
   15,   16,   25,   74,   77,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
#define YYTABLESIZE 857
static const YYINT yytable[] = {                          2,
    3,    4,    5,    6,    7,    8,  544,  419,  492,  420,
  421,  447,  434,  561,    9,   10,  539,  540,  562,  563,
  435,  436,  564,  565,   11,  528,  529,  530,  531,  532,
  566,  567,    3,  568,  569,  570,  571,  572,    4,  573,
  574,  575,  576,  577,  493,  494,  467,  468,  469,  470,
  471,  472,  473,  474,  475,  476,  477,  478,  507,  508,
  509,  510,  511,  512,  513,  514,  515,  516,  578,  495,
  422,  579,  580,   12,  581,  582,   13,  583,  584,  585,
  586,  587,    5,  588,  450,  451,  452,  453,  454,  455,
  456,  457,  589,  590,  591,  592,  593,  594,    6,  595,
  596,  423,  597,  598,  424,  437,  599,  438,  600,  601,
  602,  603,  604,  605,  606,  607,  608,  609,  610,  611,
  612,  613,  614,  615,  616,  617,  618,  619,  620,  621,
  496,  497,  622,  623,  624,  625,  626,  627,  628,    7,
  629,  630,  631,  632,  633,  634,  635,  636,  637,  638,
  639,  640,  641,  642,  643,  644,  645,  646,  647,  648,
  649,  650,  651,  652,  653,  654,  655,  656,  657,  498,
  658,  659,  660,  661,  662,  663,  664,  665,  666,  667,
  668,  669,  670,  671,  672,  673,  674,  675,  676,  677,
    8,  678,  679,  680,  681,  682,  683,  684,  685,  686,
  687,  688,  689,  690,  691,  545,    9,  546,  547,  548,
  549,  550,  692,  693,  694,  695,  696,  551,  697,  698,
  699,  700,  701,   10,  425,  702,  703,  704,  439,  705,
  706,   11,  707,  708,  709,  710,  711,  712,  713,  714,
  715,  716,  717,  718,  719,  720,  721,  722,  723,  724,
  725,  726,  727,  728,  729,  730,  731,   12,  732,  733,
  734,  735,  736,    0,    0,    3,    4,    5,    6,    7,
    8,  737,  738,  739,  740,  741,  742,  743,    0,    9,
   10,  744,  745,  746,  747,  748,  749,  750,    0,   11,
  751,  752,  753,  754,  755,  756,  757,  758,    3,    4,
    5,    6,    7,    8,    3,    4,    5,    6,    7,    8,
  759,  760,    9,   10,  761,  762,  763,  764,    9,   10,
  765,  766,   11,  767,  768,  769,  770,  771,   11,  772,
  773,  774,  775,  776,  777,  778,  779,    0,   12,  780,
    0,   13,  781,  782,  783,  784,  785,  786,    3,    4,
    5,    6,    7,    8,  787,  788,  789,  790,  791,  792,
  793,  794,    9,   10,    3,    4,    5,    6,    7,    8,
  795,   12,   11,  796,   13,  797,  798,   12,    9,   10,
   13,  799,  800,  801,  802,  803,  804,  805,   11,  806,
  807,  808,  809,  810,  811,  812,  813,  814,  815,  816,
  817,  818,  819,  820,  821,    3,    4,    5,    6,    7,
    8,  822,  823,  824,  825,  826,  827,  828,  829,    9,
   10,   12,  830,  831,   13,  832,  833,  834,  835,   11,
  836,  837,    0,    0,    0,    0,    0,   12,    0,    0,
   13,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    3,    4,    5,    6,
    7,    8,    0,    0,    0,    0,    0,    0,    0,    0,
    9,   10,    3,    4,    5,    6,    7,    8,   12,    0,
   11,   13,    0,    0,    0,    0,    9,   10,    0,    3,
    4,    5,    6,    7,    8,    0,   11,    3,    4,    5,
    6,    7,    8,    9,   10,    0,    0,    0,    0,    0,
    0,    9,   10,   11,    0,    0,    0,    0,    0,    0,
    0,   11,    0,    3,    4,    5,    6,    7,    8,   12,
    0,    0,   13,    0,    0,    0,    0,    9,   10,    0,
    0,    0,    0,    0,    0,   12,    0,   11,   13,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   12,    0,    0,   13,    0,    0,    0,    0,
   12,    0,    0,   13,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   12,    0,    0,   13,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,
   56,   57,   58,   59,   60,   61,   62,   63,   64,   65,
   66,    0,    0,    0,    0,   67,   68,   69,    0,    0,
    0,   70,   71,   72,   73,   74,   75,   76,   77,   78,
   79,   80,   81,   82,   83,   84,   85,   86,   87,   88,
   89,   90,   91,   92,   93,   94,   95,   96,   97,   98,
   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,
  109,  110,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  111,  112,  113,    0,  114,  115,  116,    0,    0,
  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,
  127,  128,  129,  130,  131,  132,  133,  134,  135,  136,
  137,  138,  139,  140,    0,    0,    0,  141,    0,  142,
  143,  144,  145,  146,  147,  148,  149,  150,  151,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  152,  153,  154,  155,  156,  157,  158,  159,
  160,  161,  162,  163,  164,  165,  166,  167,  168,  169,
  170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,
    0,  190,    0,  191,  192,  193,  194,  195,  196,  197,
  198,  199,  200,  201,  202,  203,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  204,  205,  206,
  207,  208,  209,    0,    0,    0,    0,    0,    0,  210,
    0,    0,    0,    0,    0,    0,    0,  211,  212,    0,
  213,  214,    0,  215,  216,  217,  218,    0,    0,  219,
  220,  221,  222,  223,  224,  225,  226,
};
static const YYINT yycheck[] = {                        265,
    0,    0,    0,    0,    0,    0,  297,  297,  297,  299,
  300,  365,  297,  264,    0,    0,  524,  525,  264,  264,
  305,  306,  264,  264,    0,  490,  491,  492,  493,  494,
  264,  264,  298,  264,  264,  264,  264,  264,  304,  264,
  264,  264,  264,  264,  333,  334,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  473,  474,
  475,  476,  477,  478,  479,  480,  481,  482,  264,  358,
  360,  264,  264,    0,  264,  264,    0,  264,  264,  264,
  264,  264,  348,  264,  349,  350,  351,  352,  353,  354,
  355,  356,  264,  264,  264,  264,  264,  264,  364,  264,
  264,  391,  264,  264,  394,  390,  264,  392,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  419,  420,  264,  264,  264,  264,  264,  264,  264,  405,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  458,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  456,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  496,  472,  498,  499,  500,
  501,  502,  264,  264,  264,  264,  264,  508,  264,  264,
  264,  264,  264,  489,  514,  264,  264,  264,  513,  264,
  264,  497,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  523,  264,  264,
  264,  264,  264,   -1,   -1,  265,  265,  265,  265,  265,
  265,  264,  264,  264,  264,  264,  264,  264,   -1,  265,
  265,  264,  264,  264,  264,  264,  264,  264,   -1,  265,
  264,  264,  264,  264,  264,  264,  264,  264,  298,  298,
  298,  298,  298,  298,  304,  304,  304,  304,  304,  304,
  264,  264,  298,  298,  264,  264,  264,  264,  304,  304,
  264,  264,  298,  264,  264,  264,  264,  264,  304,  264,
  264,  264,  264,  264,  264,  264,  264,   -1,  265,  264,
   -1,  265,  264,  264,  264,  264,  264,  264,  348,  348,
  348,  348,  348,  348,  264,  264,  264,  264,  264,  264,
  264,  264,  348,  348,  364,  364,  364,  364,  364,  364,
  264,  298,  348,  264,  298,  264,  264,  304,  364,  364,
  304,  264,  264,  264,  264,  264,  264,  264,  364,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  405,  405,  405,  405,  405,
  405,  264,  264,  264,  264,  264,  264,  264,  264,  405,
  405,  348,  264,  264,  348,  264,  264,  264,  264,  405,
  264,  264,   -1,   -1,   -1,   -1,   -1,  364,   -1,   -1,
  364,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  456,  456,  456,  456,
  456,  456,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  456,  456,  472,  472,  472,  472,  472,  472,  405,   -1,
  456,  405,   -1,   -1,   -1,   -1,  472,  472,   -1,  489,
  489,  489,  489,  489,  489,   -1,  472,  497,  497,  497,
  497,  497,  497,  489,  489,   -1,   -1,   -1,   -1,   -1,
   -1,  497,  497,  489,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  497,   -1,  523,  523,  523,  523,  523,  523,  456,
   -1,   -1,  456,   -1,   -1,   -1,   -1,  523,  523,   -1,
   -1,   -1,   -1,   -1,   -1,  472,   -1,  523,  472,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  489,   -1,   -1,  489,   -1,   -1,   -1,   -1,
  497,   -1,   -1,  497,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  523,   -1,   -1,  523,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  289,  290,  291,  292,  293,  294,  295,
  296,   -1,   -1,   -1,   -1,  301,  302,  303,   -1,   -1,
   -1,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  357,  358,  359,   -1,  361,  362,  363,   -1,   -1,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,   -1,   -1,   -1,  393,   -1,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  418,  419,  420,  421,  422,  423,  424,  425,
  426,  427,  428,  429,  430,  431,  432,  433,  434,  435,
  436,  437,  438,  439,  440,  441,  442,  443,  444,  445,
  446,  447,  448,  449,  450,  451,  452,  453,  454,  455,
   -1,  457,   -1,  459,  460,  461,  462,  463,  464,  465,
  466,  467,  468,  469,  470,  471,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  483,  484,  485,
  486,  487,  488,   -1,   -1,   -1,   -1,   -1,   -1,  495,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  503,  504,   -1,
  506,  507,   -1,  509,  510,  511,  512,   -1,   -1,  515,
  516,  517,  518,  519,  520,  521,  522,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 525
#define YYUNDFTOKEN 819
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"SPACE","LETTER","NEWLINE",
"COMMENT","COLON","ANY","ZONESTR","STRING_ARG","VAR_SERVER","VAR_VERBOSITY",
"VAR_NUM_THREADS","VAR_PORT","VAR_OUTGOING_RANGE","VAR_INTERFACE","VAR_DO_IP4",
"VAR_DO_IP6","VAR_PREFER_IP6","VAR_DO_UDP","VAR_DO_TCP","VAR_TCP_MSS",
"VAR_OUTGOING_TCP_MSS","VAR_TCP_IDLE_TIMEOUT","VAR_EDNS_TCP_KEEPALIVE",
"VAR_EDNS_TCP_KEEPALIVE_TIMEOUT","VAR_CHROOT","VAR_USERNAME","VAR_DIRECTORY",
"VAR_LOGFILE","VAR_PIDFILE","VAR_MSG_CACHE_SIZE","VAR_MSG_CACHE_SLABS",
"VAR_NUM_QUERIES_PER_THREAD","VAR_RRSET_CACHE_SIZE","VAR_RRSET_CACHE_SLABS",
"VAR_OUTGOING_NUM_TCP","VAR_INFRA_HOST_TTL","VAR_INFRA_LAME_TTL",
"VAR_INFRA_CACHE_SLABS","VAR_INFRA_CACHE_NUMHOSTS","VAR_INFRA_CACHE_LAME_SIZE",
"VAR_NAME","VAR_STUB_ZONE","VAR_STUB_HOST","VAR_STUB_ADDR",
"VAR_TARGET_FETCH_POLICY","VAR_HARDEN_SHORT_BUFSIZE","VAR_HARDEN_LARGE_QUERIES",
"VAR_FORWARD_ZONE","VAR_FORWARD_HOST","VAR_FORWARD_ADDR",
"VAR_DO_NOT_QUERY_ADDRESS","VAR_HIDE_IDENTITY","VAR_HIDE_VERSION",
"VAR_IDENTITY","VAR_VERSION","VAR_HARDEN_GLUE","VAR_MODULE_CONF",
"VAR_TRUST_ANCHOR_FILE","VAR_TRUST_ANCHOR","VAR_VAL_OVERRIDE_DATE",
"VAR_BOGUS_TTL","VAR_VAL_CLEAN_ADDITIONAL","VAR_VAL_PERMISSIVE_MODE",
"VAR_INCOMING_NUM_TCP","VAR_MSG_BUFFER_SIZE","VAR_KEY_CACHE_SIZE",
"VAR_KEY_CACHE_SLABS","VAR_TRUSTED_KEYS_FILE",
"VAR_VAL_NSEC3_KEYSIZE_ITERATIONS","VAR_USE_SYSLOG","VAR_OUTGOING_INTERFACE",
"VAR_ROOT_HINTS","VAR_DO_NOT_QUERY_LOCALHOST","VAR_CACHE_MAX_TTL",
"VAR_HARDEN_DNSSEC_STRIPPED","VAR_ACCESS_CONTROL","VAR_LOCAL_ZONE",
"VAR_LOCAL_DATA","VAR_INTERFACE_AUTOMATIC","VAR_STATISTICS_INTERVAL",
"VAR_DO_DAEMONIZE","VAR_USE_CAPS_FOR_ID","VAR_STATISTICS_CUMULATIVE",
"VAR_OUTGOING_PORT_PERMIT","VAR_OUTGOING_PORT_AVOID","VAR_DLV_ANCHOR_FILE",
"VAR_DLV_ANCHOR","VAR_NEG_CACHE_SIZE","VAR_HARDEN_REFERRAL_PATH",
"VAR_PRIVATE_ADDRESS","VAR_PRIVATE_DOMAIN","VAR_REMOTE_CONTROL",
"VAR_CONTROL_ENABLE","VAR_CONTROL_INTERFACE","VAR_CONTROL_PORT",
"VAR_SERVER_KEY_FILE","VAR_SERVER_CERT_FILE","VAR_CONTROL_KEY_FILE",
"VAR_CONTROL_CERT_FILE","VAR_CONTROL_USE_CERT","VAR_EXTENDED_STATISTICS",
"VAR_LOCAL_DATA_PTR","VAR_JOSTLE_TIMEOUT","VAR_STUB_PRIME",
"VAR_UNWANTED_REPLY_THRESHOLD","VAR_LOG_TIME_ASCII","VAR_DOMAIN_INSECURE",
"VAR_PYTHON","VAR_PYTHON_SCRIPT","VAR_VAL_SIG_SKEW_MIN","VAR_VAL_SIG_SKEW_MAX",
"VAR_CACHE_MIN_TTL","VAR_VAL_LOG_LEVEL","VAR_AUTO_TRUST_ANCHOR_FILE",
"VAR_KEEP_MISSING","VAR_ADD_HOLDDOWN","VAR_DEL_HOLDDOWN","VAR_SO_RCVBUF",
"VAR_EDNS_BUFFER_SIZE","VAR_PREFETCH","VAR_PREFETCH_KEY","VAR_SO_SNDBUF",
"VAR_SO_REUSEPORT","VAR_HARDEN_BELOW_NXDOMAIN","VAR_IGNORE_CD_FLAG",
"VAR_LOG_QUERIES","VAR_LOG_REPLIES","VAR_LOG_LOCAL_ACTIONS","VAR_TCP_UPSTREAM",
"VAR_SSL_UPSTREAM","VAR_SSL_SERVICE_KEY","VAR_SSL_SERVICE_PEM","VAR_SSL_PORT",
"VAR_FORWARD_FIRST","VAR_STUB_SSL_UPSTREAM","VAR_FORWARD_SSL_UPSTREAM",
"VAR_TLS_CERT_BUNDLE","VAR_STUB_FIRST","VAR_MINIMAL_RESPONSES",
"VAR_RRSET_ROUNDROBIN","VAR_MAX_UDP_SIZE","VAR_DELAY_CLOSE",
"VAR_UNBLOCK_LAN_ZONES","VAR_INSECURE_LAN_ZONES","VAR_INFRA_CACHE_MIN_RTT",
"VAR_DNS64_PREFIX","VAR_DNS64_SYNTHALL","VAR_DNS64_IGNORE_AAAA","VAR_DNSTAP",
"VAR_DNSTAP_ENABLE","VAR_DNSTAP_SOCKET_PATH","VAR_DNSTAP_SEND_IDENTITY",
"VAR_DNSTAP_SEND_VERSION","VAR_DNSTAP_IDENTITY","VAR_DNSTAP_VERSION",
"VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES",
"VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES",
"VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES",
"VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES",
"VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES",
"VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES","VAR_RESPONSE_IP_TAG",
"VAR_RESPONSE_IP","VAR_RESPONSE_IP_DATA","VAR_HARDEN_ALGO_DOWNGRADE",
"VAR_IP_TRANSPARENT","VAR_DISABLE_DNSSEC_LAME_CHECK","VAR_IP_RATELIMIT",
"VAR_IP_RATELIMIT_SLABS","VAR_IP_RATELIMIT_SIZE","VAR_RATELIMIT",
"VAR_RATELIMIT_SLABS","VAR_RATELIMIT_SIZE","VAR_RATELIMIT_FOR_DOMAIN",
"VAR_RATELIMIT_BELOW_DOMAIN","VAR_IP_RATELIMIT_FACTOR","VAR_RATELIMIT_FACTOR",
"VAR_SEND_CLIENT_SUBNET","VAR_CLIENT_SUBNET_ZONE",
"VAR_CLIENT_SUBNET_ALWAYS_FORWARD","VAR_CLIENT_SUBNET_OPCODE",
"VAR_MAX_CLIENT_SUBNET_IPV4","VAR_MAX_CLIENT_SUBNET_IPV6",
"VAR_MIN_CLIENT_SUBNET_IPV4","VAR_MIN_CLIENT_SUBNET_IPV6",
"VAR_MAX_ECS_TREE_SIZE_IPV4","VAR_MAX_ECS_TREE_SIZE_IPV6","VAR_CAPS_WHITELIST",
"VAR_CACHE_MAX_NEGATIVE_TTL","VAR_PERMIT_SMALL_HOLDDOWN",
"VAR_QNAME_MINIMISATION","VAR_QNAME_MINIMISATION_STRICT","VAR_IP_FREEBIND",
"VAR_DEFINE_TAG","VAR_LOCAL_ZONE_TAG","VAR_ACCESS_CONTROL_TAG",
"VAR_LOCAL_ZONE_OVERRIDE","VAR_ACCESS_CONTROL_TAG_ACTION",
"VAR_ACCESS_CONTROL_TAG_DATA","VAR_VIEW","VAR_ACCESS_CONTROL_VIEW",
"VAR_VIEW_FIRST","VAR_SERVE_EXPIRED","VAR_SERVE_EXPIRED_TTL",
"VAR_SERVE_EXPIRED_TTL_RESET","VAR_FAKE_DSA","VAR_FAKE_SHA1","VAR_LOG_IDENTITY",
"VAR_HIDE_TRUSTANCHOR","VAR_TRUST_ANCHOR_SIGNALING","VAR_AGGRESSIVE_NSEC",
"VAR_USE_SYSTEMD","VAR_SHM_ENABLE","VAR_SHM_KEY","VAR_ROOT_KEY_SENTINEL",
"VAR_DNSCRYPT","VAR_DNSCRYPT_ENABLE","VAR_DNSCRYPT_PORT",
"VAR_DNSCRYPT_PROVIDER","VAR_DNSCRYPT_SECRET_KEY","VAR_DNSCRYPT_PROVIDER_CERT",
"VAR_DNSCRYPT_PROVIDER_CERT_ROTATED","VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE",
"VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS","VAR_DNSCRYPT_NONCE_CACHE_SIZE",
"VAR_DNSCRYPT_NONCE_CACHE_SLABS","VAR_IPSECMOD_ENABLED","VAR_IPSECMOD_HOOK",
"VAR_IPSECMOD_IGNORE_BOGUS","VAR_IPSECMOD_MAX_TTL","VAR_IPSECMOD_WHITELIST",
"VAR_IPSECMOD_STRICT","VAR_CACHEDB","VAR_CACHEDB_BACKEND",
"VAR_CACHEDB_SECRETSEED","VAR_CACHEDB_REDISHOST","VAR_CACHEDB_REDISPORT",
"VAR_CACHEDB_REDISTIMEOUT","VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM",
"VAR_FOR_UPSTREAM","VAR_AUTH_ZONE","VAR_ZONEFILE","VAR_MASTER","VAR_URL",
"VAR_FOR_DOWNSTREAM","VAR_FALLBACK_ENABLED","VAR_TLS_ADDITIONAL_PORT",
"VAR_LOW_RTT","VAR_LOW_RTT_PERMIL","VAR_FAST_SERVER_PERMIL",
"VAR_FAST_SERVER_NUM","VAR_ALLOW_NOTIFY","VAR_TLS_WIN_CERT",
"VAR_TCP_CONNECTION_LIMIT","VAR_ANSWER_COOKIE","VAR_COOKIE_SECRET",
"VAR_FORWARD_NO_CACHE","VAR_STUB_NO_CACHE","VAR_LOG_SERVFAIL","VAR_DENY_ANY",
"VAR_UNKNOWN_SERVER_TIME_LIMIT","VAR_LOG_TAG_QUERYREPLY","VAR_STREAM_WAIT_SIZE",
"VAR_TLS_CIPHERS","VAR_TLS_CIPHERSUITES","VAR_TLS_SESSION_TICKET_KEYS",
"VAR_IPSET","VAR_IPSET_NAME_V4","VAR_IPSET_NAME_V6",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : toplevelvars",
"toplevelvars :",
"toplevelvars : toplevelvars toplevelvar",
"toplevelvar : serverstart contents_server",
"toplevelvar : stubstart contents_stub",
"toplevelvar : forwardstart contents_forward",
"toplevelvar : pythonstart contents_py",
"toplevelvar : rcstart contents_rc",
"toplevelvar : dtstart contents_dt",
"toplevelvar : viewstart contents_view",
"toplevelvar : dnscstart contents_dnsc",
"toplevelvar : cachedbstart contents_cachedb",
"toplevelvar : ipsetstart contents_ipset",
"toplevelvar : authstart contents_auth",
"serverstart : VAR_SERVER",
"contents_server : contents_server content_server",
"contents_server :",
"content_server : server_num_threads",
"content_server : server_verbosity",
"content_server : server_port",
"content_server : server_outgoing_range",
"content_server : server_do_ip4",
"content_server : server_do_ip6",
"content_server : server_prefer_ip6",
"content_server : server_do_udp",
"content_server : server_do_tcp",
"content_server : server_tcp_mss",
"content_server : server_outgoing_tcp_mss",
"content_server : server_tcp_idle_timeout",
"content_server : server_tcp_keepalive",
"content_server : server_tcp_keepalive_timeout",
"content_server : server_interface",
"content_server : server_chroot",
"content_server : server_username",
"content_server : server_directory",
"content_server : server_logfile",
"content_server : server_pidfile",
"content_server : server_msg_cache_size",
"content_server : server_msg_cache_slabs",
"content_server : server_num_queries_per_thread",
"content_server : server_rrset_cache_size",
"content_server : server_rrset_cache_slabs",
"content_server : server_outgoing_num_tcp",
"content_server : server_infra_host_ttl",
"content_server : server_infra_lame_ttl",
"content_server : server_infra_cache_slabs",
"content_server : server_infra_cache_numhosts",
"content_server : server_infra_cache_lame_size",
"content_server : server_target_fetch_policy",
"content_server : server_harden_short_bufsize",
"content_server : server_harden_large_queries",
"content_server : server_do_not_query_address",
"content_server : server_hide_identity",
"content_server : server_hide_version",
"content_server : server_identity",
"content_server : server_version",
"content_server : server_harden_glue",
"content_server : server_module_conf",
"content_server : server_trust_anchor_file",
"content_server : server_trust_anchor",
"content_server : server_val_override_date",
"content_server : server_bogus_ttl",
"content_server : server_val_clean_additional",
"content_server : server_val_permissive_mode",
"content_server : server_incoming_num_tcp",
"content_server : server_msg_buffer_size",
"content_server : server_key_cache_size",
"content_server : server_key_cache_slabs",
"content_server : server_trusted_keys_file",
"content_server : server_val_nsec3_keysize_iterations",
"content_server : server_use_syslog",
"content_server : server_outgoing_interface",
"content_server : server_root_hints",
"content_server : server_do_not_query_localhost",
"content_server : server_cache_max_ttl",
"content_server : server_harden_dnssec_stripped",
"content_server : server_access_control",
"content_server : server_local_zone",
"content_server : server_local_data",
"content_server : server_interface_automatic",
"content_server : server_statistics_interval",
"content_server : server_do_daemonize",
"content_server : server_use_caps_for_id",
"content_server : server_statistics_cumulative",
"content_server : server_outgoing_port_permit",
"content_server : server_outgoing_port_avoid",
"content_server : server_dlv_anchor_file",
"content_server : server_dlv_anchor",
"content_server : server_neg_cache_size",
"content_server : server_harden_referral_path",
"content_server : server_private_address",
"content_server : server_private_domain",
"content_server : server_extended_statistics",
"content_server : server_local_data_ptr",
"content_server : server_jostle_timeout",
"content_server : server_unwanted_reply_threshold",
"content_server : server_log_time_ascii",
"content_server : server_domain_insecure",
"content_server : server_val_sig_skew_min",
"content_server : server_val_sig_skew_max",
"content_server : server_cache_min_ttl",
"content_server : server_val_log_level",
"content_server : server_auto_trust_anchor_file",
"content_server : server_add_holddown",
"content_server : server_del_holddown",
"content_server : server_keep_missing",
"content_server : server_so_rcvbuf",
"content_server : server_edns_buffer_size",
"content_server : server_prefetch",
"content_server : server_prefetch_key",
"content_server : server_so_sndbuf",
"content_server : server_harden_below_nxdomain",
"content_server : server_ignore_cd_flag",
"content_server : server_log_queries",
"content_server : server_log_replies",
"content_server : server_tcp_upstream",
"content_server : server_ssl_upstream",
"content_server : server_log_local_actions",
"content_server : server_ssl_service_key",
"content_server : server_ssl_service_pem",
"content_server : server_ssl_port",
"content_server : server_minimal_responses",
"content_server : server_rrset_roundrobin",
"content_server : server_max_udp_size",
"content_server : server_so_reuseport",
"content_server : server_delay_close",
"content_server : server_unblock_lan_zones",
"content_server : server_insecure_lan_zones",
"content_server : server_dns64_prefix",
"content_server : server_dns64_synthall",
"content_server : server_dns64_ignore_aaaa",
"content_server : server_infra_cache_min_rtt",
"content_server : server_harden_algo_downgrade",
"content_server : server_ip_transparent",
"content_server : server_ip_ratelimit",
"content_server : server_ratelimit",
"content_server : server_ip_ratelimit_slabs",
"content_server : server_ratelimit_slabs",
"content_server : server_ip_ratelimit_size",
"content_server : server_ratelimit_size",
"content_server : server_ratelimit_for_domain",
"content_server : server_ratelimit_below_domain",
"content_server : server_ratelimit_factor",
"content_server : server_ip_ratelimit_factor",
"content_server : server_send_client_subnet",
"content_server : server_client_subnet_zone",
"content_server : server_client_subnet_always_forward",
"content_server : server_client_subnet_opcode",
"content_server : server_max_client_subnet_ipv4",
"content_server : server_max_client_subnet_ipv6",
"content_server : server_min_client_subnet_ipv4",
"content_server : server_min_client_subnet_ipv6",
"content_server : server_max_ecs_tree_size_ipv4",
"content_server : server_max_ecs_tree_size_ipv6",
"content_server : server_caps_whitelist",
"content_server : server_cache_max_negative_ttl",
"content_server : server_permit_small_holddown",
"content_server : server_qname_minimisation",
"content_server : server_ip_freebind",
"content_server : server_define_tag",
"content_server : server_local_zone_tag",
"content_server : server_disable_dnssec_lame_check",
"content_server : server_access_control_tag",
"content_server : server_local_zone_override",
"content_server : server_access_control_tag_action",
"content_server : server_access_control_tag_data",
"content_server : server_access_control_view",
"content_server : server_qname_minimisation_strict",
"content_server : server_serve_expired",
"content_server : server_serve_expired_ttl",
"content_server : server_serve_expired_ttl_reset",
"content_server : server_fake_dsa",
"content_server : server_log_identity",
"content_server : server_use_systemd",
"content_server : server_response_ip_tag",
"content_server : server_response_ip",
"content_server : server_response_ip_data",
"content_server : server_shm_enable",
"content_server : server_shm_key",
"content_server : server_fake_sha1",
"content_server : server_hide_trustanchor",
"content_server : server_trust_anchor_signaling",
"content_server : server_root_key_sentinel",
"content_server : server_ipsecmod_enabled",
"content_server : server_ipsecmod_hook",
"content_server : server_ipsecmod_ignore_bogus",
"content_server : server_ipsecmod_max_ttl",
"content_server : server_ipsecmod_whitelist",
"content_server : server_ipsecmod_strict",
"content_server : server_udp_upstream_without_downstream",
"content_server : server_aggressive_nsec",
"content_server : server_tls_cert_bundle",
"content_server : server_tls_additional_port",
"content_server : server_low_rtt",
"content_server : server_fast_server_permil",
"content_server : server_fast_server_num",
"content_server : server_tls_win_cert",
"content_server : server_tcp_connection_limit",
"content_server : server_log_servfail",
"content_server : server_deny_any",
"content_server : server_unknown_server_time_limit",
"content_server : server_log_tag_queryreply",
"content_server : server_stream_wait_size",
"content_server : server_tls_ciphers",
"content_server : server_tls_ciphersuites",
"content_server : server_tls_session_ticket_keys",
"content_server : server_answer_cookie",
"content_server : server_cookie_secret",
"stubstart : VAR_STUB_ZONE",
"contents_stub : contents_stub content_stub",
"contents_stub :",
"content_stub : stub_name",
"content_stub : stub_host",
"content_stub : stub_addr",
"content_stub : stub_prime",
"content_stub : stub_first",
"content_stub : stub_no_cache",
"content_stub : stub_ssl_upstream",
"forwardstart : VAR_FORWARD_ZONE",
"contents_forward : contents_forward content_forward",
"contents_forward :",
"content_forward : forward_name",
"content_forward : forward_host",
"content_forward : forward_addr",
"content_forward : forward_first",
"content_forward : forward_no_cache",
"content_forward : forward_ssl_upstream",
"viewstart : VAR_VIEW",
"contents_view : contents_view content_view",
"contents_view :",
"content_view : view_name",
"content_view : view_local_zone",
"content_view : view_local_data",
"content_view : view_first",
"content_view : view_response_ip",
"content_view : view_response_ip_data",
"content_view : view_local_data_ptr",
"authstart : VAR_AUTH_ZONE",
"contents_auth : contents_auth content_auth",
"contents_auth :",
"content_auth : auth_name",
"content_auth : auth_zonefile",
"content_auth : auth_master",
"content_auth : auth_url",
"content_auth : auth_for_downstream",
"content_auth : auth_for_upstream",
"content_auth : auth_fallback_enabled",
"content_auth : auth_allow_notify",
"server_num_threads : VAR_NUM_THREADS STRING_ARG",
"server_verbosity : VAR_VERBOSITY STRING_ARG",
"server_statistics_interval : VAR_STATISTICS_INTERVAL STRING_ARG",
"server_statistics_cumulative : VAR_STATISTICS_CUMULATIVE STRING_ARG",
"server_extended_statistics : VAR_EXTENDED_STATISTICS STRING_ARG",
"server_shm_enable : VAR_SHM_ENABLE STRING_ARG",
"server_shm_key : VAR_SHM_KEY STRING_ARG",
"server_port : VAR_PORT STRING_ARG",
"server_send_client_subnet : VAR_SEND_CLIENT_SUBNET STRING_ARG",
"server_client_subnet_zone : VAR_CLIENT_SUBNET_ZONE STRING_ARG",
"server_client_subnet_always_forward : VAR_CLIENT_SUBNET_ALWAYS_FORWARD STRING_ARG",
"server_client_subnet_opcode : VAR_CLIENT_SUBNET_OPCODE STRING_ARG",
"server_max_client_subnet_ipv4 : VAR_MAX_CLIENT_SUBNET_IPV4 STRING_ARG",
"server_max_client_subnet_ipv6 : VAR_MAX_CLIENT_SUBNET_IPV6 STRING_ARG",
"server_min_client_subnet_ipv4 : VAR_MIN_CLIENT_SUBNET_IPV4 STRING_ARG",
"server_min_client_subnet_ipv6 : VAR_MIN_CLIENT_SUBNET_IPV6 STRING_ARG",
"server_max_ecs_tree_size_ipv4 : VAR_MAX_ECS_TREE_SIZE_IPV4 STRING_ARG",
"server_max_ecs_tree_size_ipv6 : VAR_MAX_ECS_TREE_SIZE_IPV6 STRING_ARG",
"server_interface : VAR_INTERFACE STRING_ARG",
"server_outgoing_interface : VAR_OUTGOING_INTERFACE STRING_ARG",
"server_outgoing_range : VAR_OUTGOING_RANGE STRING_ARG",
"server_outgoing_port_permit : VAR_OUTGOING_PORT_PERMIT STRING_ARG",
"server_outgoing_port_avoid : VAR_OUTGOING_PORT_AVOID STRING_ARG",
"server_outgoing_num_tcp : VAR_OUTGOING_NUM_TCP STRING_ARG",
"server_incoming_num_tcp : VAR_INCOMING_NUM_TCP STRING_ARG",
"server_interface_automatic : VAR_INTERFACE_AUTOMATIC STRING_ARG",
"server_do_ip4 : VAR_DO_IP4 STRING_ARG",
"server_do_ip6 : VAR_DO_IP6 STRING_ARG",
"server_do_udp : VAR_DO_UDP STRING_ARG",
"server_do_tcp : VAR_DO_TCP STRING_ARG",
"server_prefer_ip6 : VAR_PREFER_IP6 STRING_ARG",
"server_tcp_mss : VAR_TCP_MSS STRING_ARG",
"server_outgoing_tcp_mss : VAR_OUTGOING_TCP_MSS STRING_ARG",
"server_tcp_idle_timeout : VAR_TCP_IDLE_TIMEOUT STRING_ARG",
"server_tcp_keepalive : VAR_EDNS_TCP_KEEPALIVE STRING_ARG",
"server_tcp_keepalive_timeout : VAR_EDNS_TCP_KEEPALIVE_TIMEOUT STRING_ARG",
"server_tcp_upstream : VAR_TCP_UPSTREAM STRING_ARG",
"server_udp_upstream_without_downstream : VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM STRING_ARG",
"server_ssl_upstream : VAR_SSL_UPSTREAM STRING_ARG",
"server_ssl_service_key : VAR_SSL_SERVICE_KEY STRING_ARG",
"server_ssl_service_pem : VAR_SSL_SERVICE_PEM STRING_ARG",
"server_ssl_port : VAR_SSL_PORT STRING_ARG",
"server_tls_cert_bundle : VAR_TLS_CERT_BUNDLE STRING_ARG",
"server_tls_win_cert : VAR_TLS_WIN_CERT STRING_ARG",
"server_tls_additional_port : VAR_TLS_ADDITIONAL_PORT STRING_ARG",
"server_tls_ciphers : VAR_TLS_CIPHERS STRING_ARG",
"server_tls_ciphersuites : VAR_TLS_CIPHERSUITES STRING_ARG",
"server_tls_session_ticket_keys : VAR_TLS_SESSION_TICKET_KEYS STRING_ARG",
"server_use_systemd : VAR_USE_SYSTEMD STRING_ARG",
"server_do_daemonize : VAR_DO_DAEMONIZE STRING_ARG",
"server_use_syslog : VAR_USE_SYSLOG STRING_ARG",
"server_log_time_ascii : VAR_LOG_TIME_ASCII STRING_ARG",
"server_log_queries : VAR_LOG_QUERIES STRING_ARG",
"server_log_replies : VAR_LOG_REPLIES STRING_ARG",
"server_log_tag_queryreply : VAR_LOG_TAG_QUERYREPLY STRING_ARG",
"server_log_servfail : VAR_LOG_SERVFAIL STRING_ARG",
"server_log_local_actions : VAR_LOG_LOCAL_ACTIONS STRING_ARG",
"server_chroot : VAR_CHROOT STRING_ARG",
"server_username : VAR_USERNAME STRING_ARG",
"server_directory : VAR_DIRECTORY STRING_ARG",
"server_logfile : VAR_LOGFILE STRING_ARG",
"server_pidfile : VAR_PIDFILE STRING_ARG",
"server_root_hints : VAR_ROOT_HINTS STRING_ARG",
"server_dlv_anchor_file : VAR_DLV_ANCHOR_FILE STRING_ARG",
"server_dlv_anchor : VAR_DLV_ANCHOR STRING_ARG",
"server_auto_trust_anchor_file : VAR_AUTO_TRUST_ANCHOR_FILE STRING_ARG",
"server_trust_anchor_file : VAR_TRUST_ANCHOR_FILE STRING_ARG",
"server_trusted_keys_file : VAR_TRUSTED_KEYS_FILE STRING_ARG",
"server_trust_anchor : VAR_TRUST_ANCHOR STRING_ARG",
"server_trust_anchor_signaling : VAR_TRUST_ANCHOR_SIGNALING STRING_ARG",
"server_root_key_sentinel : VAR_ROOT_KEY_SENTINEL STRING_ARG",
"server_domain_insecure : VAR_DOMAIN_INSECURE STRING_ARG",
"server_hide_identity : VAR_HIDE_IDENTITY STRING_ARG",
"server_hide_version : VAR_HIDE_VERSION STRING_ARG",
"server_hide_trustanchor : VAR_HIDE_TRUSTANCHOR STRING_ARG",
"server_identity : VAR_IDENTITY STRING_ARG",
"server_version : VAR_VERSION STRING_ARG",
"server_so_rcvbuf : VAR_SO_RCVBUF STRING_ARG",
"server_so_sndbuf : VAR_SO_SNDBUF STRING_ARG",
"server_so_reuseport : VAR_SO_REUSEPORT STRING_ARG",
"server_ip_transparent : VAR_IP_TRANSPARENT STRING_ARG",
"server_ip_freebind : VAR_IP_FREEBIND STRING_ARG",
"server_stream_wait_size : VAR_STREAM_WAIT_SIZE STRING_ARG",
"server_edns_buffer_size : VAR_EDNS_BUFFER_SIZE STRING_ARG",
"server_msg_buffer_size : VAR_MSG_BUFFER_SIZE STRING_ARG",
"server_msg_cache_size : VAR_MSG_CACHE_SIZE STRING_ARG",
"server_msg_cache_slabs : VAR_MSG_CACHE_SLABS STRING_ARG",
"server_num_queries_per_thread : VAR_NUM_QUERIES_PER_THREAD STRING_ARG",
"server_jostle_timeout : VAR_JOSTLE_TIMEOUT STRING_ARG",
"server_delay_close : VAR_DELAY_CLOSE STRING_ARG",
"server_unblock_lan_zones : VAR_UNBLOCK_LAN_ZONES STRING_ARG",
"server_insecure_lan_zones : VAR_INSECURE_LAN_ZONES STRING_ARG",
"server_rrset_cache_size : VAR_RRSET_CACHE_SIZE STRING_ARG",
"server_rrset_cache_slabs : VAR_RRSET_CACHE_SLABS STRING_ARG",
"server_infra_host_ttl : VAR_INFRA_HOST_TTL STRING_ARG",
"server_infra_lame_ttl : VAR_INFRA_LAME_TTL STRING_ARG",
"server_infra_cache_numhosts : VAR_INFRA_CACHE_NUMHOSTS STRING_ARG",
"server_infra_cache_lame_size : VAR_INFRA_CACHE_LAME_SIZE STRING_ARG",
"server_infra_cache_slabs : VAR_INFRA_CACHE_SLABS STRING_ARG",
"server_infra_cache_min_rtt : VAR_INFRA_CACHE_MIN_RTT STRING_ARG",
"server_target_fetch_policy : VAR_TARGET_FETCH_POLICY STRING_ARG",
"server_harden_short_bufsize : VAR_HARDEN_SHORT_BUFSIZE STRING_ARG",
"server_harden_large_queries : VAR_HARDEN_LARGE_QUERIES STRING_ARG",
"server_harden_glue : VAR_HARDEN_GLUE STRING_ARG",
"server_harden_dnssec_stripped : VAR_HARDEN_DNSSEC_STRIPPED STRING_ARG",
"server_harden_below_nxdomain : VAR_HARDEN_BELOW_NXDOMAIN STRING_ARG",
"server_harden_referral_path : VAR_HARDEN_REFERRAL_PATH STRING_ARG",
"server_harden_algo_downgrade : VAR_HARDEN_ALGO_DOWNGRADE STRING_ARG",
"server_use_caps_for_id : VAR_USE_CAPS_FOR_ID STRING_ARG",
"server_caps_whitelist : VAR_CAPS_WHITELIST STRING_ARG",
"server_private_address : VAR_PRIVATE_ADDRESS STRING_ARG",
"server_private_domain : VAR_PRIVATE_DOMAIN STRING_ARG",
"server_prefetch : VAR_PREFETCH STRING_ARG",
"server_prefetch_key : VAR_PREFETCH_KEY STRING_ARG",
"server_deny_any : VAR_DENY_ANY STRING_ARG",
"server_unwanted_reply_threshold : VAR_UNWANTED_REPLY_THRESHOLD STRING_ARG",
"server_do_not_query_address : VAR_DO_NOT_QUERY_ADDRESS STRING_ARG",
"server_do_not_query_localhost : VAR_DO_NOT_QUERY_LOCALHOST STRING_ARG",
"server_access_control : VAR_ACCESS_CONTROL STRING_ARG STRING_ARG",
"server_module_conf : VAR_MODULE_CONF STRING_ARG",
"server_val_override_date : VAR_VAL_OVERRIDE_DATE STRING_ARG",
"server_val_sig_skew_min : VAR_VAL_SIG_SKEW_MIN STRING_ARG",
"server_val_sig_skew_max : VAR_VAL_SIG_SKEW_MAX STRING_ARG",
"server_cache_max_ttl : VAR_CACHE_MAX_TTL STRING_ARG",
"server_cache_max_negative_ttl : VAR_CACHE_MAX_NEGATIVE_TTL STRING_ARG",
"server_cache_min_ttl : VAR_CACHE_MIN_TTL STRING_ARG",
"server_bogus_ttl : VAR_BOGUS_TTL STRING_ARG",
"server_val_clean_additional : VAR_VAL_CLEAN_ADDITIONAL STRING_ARG",
"server_val_permissive_mode : VAR_VAL_PERMISSIVE_MODE STRING_ARG",
"server_aggressive_nsec : VAR_AGGRESSIVE_NSEC STRING_ARG",
"server_ignore_cd_flag : VAR_IGNORE_CD_FLAG STRING_ARG",
"server_serve_expired : VAR_SERVE_EXPIRED STRING_ARG",
"server_serve_expired_ttl : VAR_SERVE_EXPIRED_TTL STRING_ARG",
"server_serve_expired_ttl_reset : VAR_SERVE_EXPIRED_TTL_RESET STRING_ARG",
"server_fake_dsa : VAR_FAKE_DSA STRING_ARG",
"server_fake_sha1 : VAR_FAKE_SHA1 STRING_ARG",
"server_val_log_level : VAR_VAL_LOG_LEVEL STRING_ARG",
"server_val_nsec3_keysize_iterations : VAR_VAL_NSEC3_KEYSIZE_ITERATIONS STRING_ARG",
"server_add_holddown : VAR_ADD_HOLDDOWN STRING_ARG",
"server_del_holddown : VAR_DEL_HOLDDOWN STRING_ARG",
"server_keep_missing : VAR_KEEP_MISSING STRING_ARG",
"server_permit_small_holddown : VAR_PERMIT_SMALL_HOLDDOWN STRING_ARG",
"server_key_cache_size : VAR_KEY_CACHE_SIZE STRING_ARG",
"server_key_cache_slabs : VAR_KEY_CACHE_SLABS STRING_ARG",
"server_neg_cache_size : VAR_NEG_CACHE_SIZE STRING_ARG",
"server_local_zone : VAR_LOCAL_ZONE STRING_ARG STRING_ARG",
"server_local_data : VAR_LOCAL_DATA STRING_ARG",
"server_local_data_ptr : VAR_LOCAL_DATA_PTR STRING_ARG",
"server_minimal_responses : VAR_MINIMAL_RESPONSES STRING_ARG",
"server_rrset_roundrobin : VAR_RRSET_ROUNDROBIN STRING_ARG",
"server_unknown_server_time_limit : VAR_UNKNOWN_SERVER_TIME_LIMIT STRING_ARG",
"server_max_udp_size : VAR_MAX_UDP_SIZE STRING_ARG",
"server_dns64_prefix : VAR_DNS64_PREFIX STRING_ARG",
"server_dns64_synthall : VAR_DNS64_SYNTHALL STRING_ARG",
"server_dns64_ignore_aaaa : VAR_DNS64_IGNORE_AAAA STRING_ARG",
"server_define_tag : VAR_DEFINE_TAG STRING_ARG",
"server_local_zone_tag : VAR_LOCAL_ZONE_TAG STRING_ARG STRING_ARG",
"server_access_control_tag : VAR_ACCESS_CONTROL_TAG STRING_ARG STRING_ARG",
"server_access_control_tag_action : VAR_ACCESS_CONTROL_TAG_ACTION STRING_ARG STRING_ARG STRING_ARG",
"server_access_control_tag_data : VAR_ACCESS_CONTROL_TAG_DATA STRING_ARG STRING_ARG STRING_ARG",
"server_local_zone_override : VAR_LOCAL_ZONE_OVERRIDE STRING_ARG STRING_ARG STRING_ARG",
"server_access_control_view : VAR_ACCESS_CONTROL_VIEW STRING_ARG STRING_ARG",
"server_response_ip_tag : VAR_RESPONSE_IP_TAG STRING_ARG STRING_ARG",
"server_ip_ratelimit : VAR_IP_RATELIMIT STRING_ARG",
"server_ratelimit : VAR_RATELIMIT STRING_ARG",
"server_ip_ratelimit_size : VAR_IP_RATELIMIT_SIZE STRING_ARG",
"server_ratelimit_size : VAR_RATELIMIT_SIZE STRING_ARG",
"server_ip_ratelimit_slabs : VAR_IP_RATELIMIT_SLABS STRING_ARG",
"server_ratelimit_slabs : VAR_RATELIMIT_SLABS STRING_ARG",
"server_ratelimit_for_domain : VAR_RATELIMIT_FOR_DOMAIN STRING_ARG STRING_ARG",
"server_ratelimit_below_domain : VAR_RATELIMIT_BELOW_DOMAIN STRING_ARG STRING_ARG",
"server_ip_ratelimit_factor : VAR_IP_RATELIMIT_FACTOR STRING_ARG",
"server_ratelimit_factor : VAR_RATELIMIT_FACTOR STRING_ARG",
"server_low_rtt : VAR_LOW_RTT STRING_ARG",
"server_fast_server_num : VAR_FAST_SERVER_NUM STRING_ARG",
"server_fast_server_permil : VAR_FAST_SERVER_PERMIL STRING_ARG",
"server_qname_minimisation : VAR_QNAME_MINIMISATION STRING_ARG",
"server_qname_minimisation_strict : VAR_QNAME_MINIMISATION_STRICT STRING_ARG",
"server_ipsecmod_enabled : VAR_IPSECMOD_ENABLED STRING_ARG",
"server_ipsecmod_ignore_bogus : VAR_IPSECMOD_IGNORE_BOGUS STRING_ARG",
"server_ipsecmod_hook : VAR_IPSECMOD_HOOK STRING_ARG",
"server_ipsecmod_max_ttl : VAR_IPSECMOD_MAX_TTL STRING_ARG",
"server_ipsecmod_whitelist : VAR_IPSECMOD_WHITELIST STRING_ARG",
"server_ipsecmod_strict : VAR_IPSECMOD_STRICT STRING_ARG",
"stub_name : VAR_NAME STRING_ARG",
"stub_host : VAR_STUB_HOST STRING_ARG",
"stub_addr : VAR_STUB_ADDR STRING_ARG",
"stub_first : VAR_STUB_FIRST STRING_ARG",
"stub_no_cache : VAR_STUB_NO_CACHE STRING_ARG",
"stub_ssl_upstream : VAR_STUB_SSL_UPSTREAM STRING_ARG",
"stub_prime : VAR_STUB_PRIME STRING_ARG",
"forward_name : VAR_NAME STRING_ARG",
"forward_host : VAR_FORWARD_HOST STRING_ARG",
"forward_addr : VAR_FORWARD_ADDR STRING_ARG",
"forward_first : VAR_FORWARD_FIRST STRING_ARG",
"forward_no_cache : VAR_FORWARD_NO_CACHE STRING_ARG",
"forward_ssl_upstream : VAR_FORWARD_SSL_UPSTREAM STRING_ARG",
"auth_name : VAR_NAME STRING_ARG",
"auth_zonefile : VAR_ZONEFILE STRING_ARG",
"auth_master : VAR_MASTER STRING_ARG",
"auth_url : VAR_URL STRING_ARG",
"auth_allow_notify : VAR_ALLOW_NOTIFY STRING_ARG",
"auth_for_downstream : VAR_FOR_DOWNSTREAM STRING_ARG",
"auth_for_upstream : VAR_FOR_UPSTREAM STRING_ARG",
"auth_fallback_enabled : VAR_FALLBACK_ENABLED STRING_ARG",
"view_name : VAR_NAME STRING_ARG",
"view_local_zone : VAR_LOCAL_ZONE STRING_ARG STRING_ARG",
"view_response_ip : VAR_RESPONSE_IP STRING_ARG STRING_ARG",
"view_response_ip_data : VAR_RESPONSE_IP_DATA STRING_ARG STRING_ARG",
"view_local_data : VAR_LOCAL_DATA STRING_ARG",
"view_local_data_ptr : VAR_LOCAL_DATA_PTR STRING_ARG",
"view_first : VAR_VIEW_FIRST STRING_ARG",
"rcstart : VAR_REMOTE_CONTROL",
"contents_rc : contents_rc content_rc",
"contents_rc :",
"content_rc : rc_control_enable",
"content_rc : rc_control_interface",
"content_rc : rc_control_port",
"content_rc : rc_server_key_file",
"content_rc : rc_server_cert_file",
"content_rc : rc_control_key_file",
"content_rc : rc_control_cert_file",
"content_rc : rc_control_use_cert",
"rc_control_enable : VAR_CONTROL_ENABLE STRING_ARG",
"rc_control_port : VAR_CONTROL_PORT STRING_ARG",
"rc_control_interface : VAR_CONTROL_INTERFACE STRING_ARG",
"rc_control_use_cert : VAR_CONTROL_USE_CERT STRING_ARG",
"rc_server_key_file : VAR_SERVER_KEY_FILE STRING_ARG",
"rc_server_cert_file : VAR_SERVER_CERT_FILE STRING_ARG",
"rc_control_key_file : VAR_CONTROL_KEY_FILE STRING_ARG",
"rc_control_cert_file : VAR_CONTROL_CERT_FILE STRING_ARG",
"dtstart : VAR_DNSTAP",
"contents_dt : contents_dt content_dt",
"contents_dt :",
"content_dt : dt_dnstap_enable",
"content_dt : dt_dnstap_socket_path",
"content_dt : dt_dnstap_send_identity",
"content_dt : dt_dnstap_send_version",
"content_dt : dt_dnstap_identity",
"content_dt : dt_dnstap_version",
"content_dt : dt_dnstap_log_resolver_query_messages",
"content_dt : dt_dnstap_log_resolver_response_messages",
"content_dt : dt_dnstap_log_client_query_messages",
"content_dt : dt_dnstap_log_client_response_messages",
"content_dt : dt_dnstap_log_forwarder_query_messages",
"content_dt : dt_dnstap_log_forwarder_response_messages",
"dt_dnstap_enable : VAR_DNSTAP_ENABLE STRING_ARG",
"dt_dnstap_socket_path : VAR_DNSTAP_SOCKET_PATH STRING_ARG",
"dt_dnstap_send_identity : VAR_DNSTAP_SEND_IDENTITY STRING_ARG",
"dt_dnstap_send_version : VAR_DNSTAP_SEND_VERSION STRING_ARG",
"dt_dnstap_identity : VAR_DNSTAP_IDENTITY STRING_ARG",
"dt_dnstap_version : VAR_DNSTAP_VERSION STRING_ARG",
"dt_dnstap_log_resolver_query_messages : VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES STRING_ARG",
"dt_dnstap_log_resolver_response_messages : VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES STRING_ARG",
"dt_dnstap_log_client_query_messages : VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES STRING_ARG",
"dt_dnstap_log_client_response_messages : VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES STRING_ARG",
"dt_dnstap_log_forwarder_query_messages : VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES STRING_ARG",
"dt_dnstap_log_forwarder_response_messages : VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES STRING_ARG",
"pythonstart : VAR_PYTHON",
"contents_py : contents_py content_py",
"contents_py :",
"content_py : py_script",
"py_script : VAR_PYTHON_SCRIPT STRING_ARG",
"server_disable_dnssec_lame_check : VAR_DISABLE_DNSSEC_LAME_CHECK STRING_ARG",
"server_log_identity : VAR_LOG_IDENTITY STRING_ARG",
"server_response_ip : VAR_RESPONSE_IP STRING_ARG STRING_ARG",
"server_response_ip_data : VAR_RESPONSE_IP_DATA STRING_ARG STRING_ARG",
"dnscstart : VAR_DNSCRYPT",
"contents_dnsc : contents_dnsc content_dnsc",
"contents_dnsc :",
"content_dnsc : dnsc_dnscrypt_enable",
"content_dnsc : dnsc_dnscrypt_port",
"content_dnsc : dnsc_dnscrypt_provider",
"content_dnsc : dnsc_dnscrypt_secret_key",
"content_dnsc : dnsc_dnscrypt_provider_cert",
"content_dnsc : dnsc_dnscrypt_provider_cert_rotated",
"content_dnsc : dnsc_dnscrypt_shared_secret_cache_size",
"content_dnsc : dnsc_dnscrypt_shared_secret_cache_slabs",
"content_dnsc : dnsc_dnscrypt_nonce_cache_size",
"content_dnsc : dnsc_dnscrypt_nonce_cache_slabs",
"dnsc_dnscrypt_enable : VAR_DNSCRYPT_ENABLE STRING_ARG",
"dnsc_dnscrypt_port : VAR_DNSCRYPT_PORT STRING_ARG",
"dnsc_dnscrypt_provider : VAR_DNSCRYPT_PROVIDER STRING_ARG",
"dnsc_dnscrypt_provider_cert : VAR_DNSCRYPT_PROVIDER_CERT STRING_ARG",
"dnsc_dnscrypt_provider_cert_rotated : VAR_DNSCRYPT_PROVIDER_CERT_ROTATED STRING_ARG",
"dnsc_dnscrypt_secret_key : VAR_DNSCRYPT_SECRET_KEY STRING_ARG",
"dnsc_dnscrypt_shared_secret_cache_size : VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE STRING_ARG",
"dnsc_dnscrypt_shared_secret_cache_slabs : VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS STRING_ARG",
"dnsc_dnscrypt_nonce_cache_size : VAR_DNSCRYPT_NONCE_CACHE_SIZE STRING_ARG",
"dnsc_dnscrypt_nonce_cache_slabs : VAR_DNSCRYPT_NONCE_CACHE_SLABS STRING_ARG",
"cachedbstart : VAR_CACHEDB",
"contents_cachedb : contents_cachedb content_cachedb",
"contents_cachedb :",
"content_cachedb : cachedb_backend_name",
"content_cachedb : cachedb_secret_seed",
"content_cachedb : redis_server_host",
"content_cachedb : redis_server_port",
"content_cachedb : redis_timeout",
"cachedb_backend_name : VAR_CACHEDB_BACKEND STRING_ARG",
"cachedb_secret_seed : VAR_CACHEDB_SECRETSEED STRING_ARG",
"redis_server_host : VAR_CACHEDB_REDISHOST STRING_ARG",
"redis_server_port : VAR_CACHEDB_REDISPORT STRING_ARG",
"redis_timeout : VAR_CACHEDB_REDISTIMEOUT STRING_ARG",
"server_tcp_connection_limit : VAR_TCP_CONNECTION_LIMIT STRING_ARG STRING_ARG",
"server_answer_cookie : VAR_ANSWER_COOKIE STRING_ARG",
"server_cookie_secret : VAR_COOKIE_SECRET STRING_ARG",
"ipsetstart : VAR_IPSET",
"contents_ipset : contents_ipset content_ipset",
"contents_ipset :",
"content_ipset : ipset_name_v4",
"content_ipset : ipset_name_v6",
"ipset_name_v4 : VAR_IPSET_NAME_V4 STRING_ARG",
"ipset_name_v6 : VAR_IPSET_NAME_V6 STRING_ARG",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 3049 "./util/configparser.y"

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


#line 1714 "util/configparser.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 14:
#line 186 "./util/configparser.y"
	{ 
		OUTYY(("\nP(server:)\n")); 
	}
break;
case 208:
#line 278 "./util/configparser.y"
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
break;
case 218:
#line 295 "./util/configparser.y"
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
break;
case 227:
#line 312 "./util/configparser.y"
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
break;
case 237:
#line 331 "./util/configparser.y"
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
break;
case 248:
#line 353 "./util/configparser.y"
	{ 
		OUTYY(("P(server_num_threads:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 249:
#line 362 "./util/configparser.y"
	{ 
		OUTYY(("P(server_verbosity:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 250:
#line 371 "./util/configparser.y"
	{ 
		OUTYY(("P(server_statistics_interval:%s)\n", yystack.l_mark[0].str)); 
		if(strcmp(yystack.l_mark[0].str, "") == 0 || strcmp(yystack.l_mark[0].str, "0") == 0)
			cfg_parser->cfg->stat_interval = 0;
		else if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->stat_interval = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 251:
#line 382 "./util/configparser.y"
	{
		OUTYY(("P(server_statistics_cumulative:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 252:
#line 391 "./util/configparser.y"
	{
		OUTYY(("P(server_extended_statistics:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 253:
#line 400 "./util/configparser.y"
	{
		OUTYY(("P(server_shm_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->shm_enable = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 254:
#line 409 "./util/configparser.y"
	{ 
		OUTYY(("P(server_shm_key:%s)\n", yystack.l_mark[0].str)); 
		if(strcmp(yystack.l_mark[0].str, "") == 0 || strcmp(yystack.l_mark[0].str, "0") == 0)
			cfg_parser->cfg->shm_key = 0;
		else if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->shm_key = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 255:
#line 420 "./util/configparser.y"
	{
		OUTYY(("P(server_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 256:
#line 429 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_send_client_subnet:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet, yystack.l_mark[0].str))
			fatal_exit("out of memory adding client-subnet");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
	}
break;
case 257:
#line 440 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_zone:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet_zone,
			yystack.l_mark[0].str))
			fatal_exit("out of memory adding client-subnet-zone");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
	}
break;
case 258:
#line 453 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_always_forward:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->client_subnet_always_forward =
				(strcmp(yystack.l_mark[0].str, "yes")==0);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 259:
#line 468 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(client_subnet_opcode:%s)\n", yystack.l_mark[0].str));
		OUTYY(("P(Deprecated option, ignoring)\n"));
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 260:
#line 479 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv4:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi(yystack.l_mark[0].str) > 32)
			cfg_parser->cfg->max_client_subnet_ipv4 = 32;
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->max_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv4 = (uint8_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 261:
#line 496 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_client_subnet_ipv6:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi(yystack.l_mark[0].str) > 128)
			cfg_parser->cfg->max_client_subnet_ipv6 = 128;
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->max_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->max_client_subnet_ipv6 = (uint8_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 262:
#line 513 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv4:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("IPv4 subnet length expected");
		else if (atoi(yystack.l_mark[0].str) > 32)
			cfg_parser->cfg->min_client_subnet_ipv4 = 32;
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->min_client_subnet_ipv4 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv4 = (uint8_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 263:
#line 530 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(min_client_subnet_ipv6:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("Ipv6 subnet length expected");
		else if (atoi(yystack.l_mark[0].str) > 128)
			cfg_parser->cfg->min_client_subnet_ipv6 = 128;
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->min_client_subnet_ipv6 = 0;
		else cfg_parser->cfg->min_client_subnet_ipv6 = (uint8_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 264:
#line 547 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv4:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("IPv4 ECS tree size expected");
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv4 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv4 = (uint32_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 265:
#line 562 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(max_ecs_tree_size_ipv6:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("IPv6 ECS tree size expected");
		else if (atoi(yystack.l_mark[0].str) < 0)
			cfg_parser->cfg->max_ecs_tree_size_ipv6 = 0;
		else cfg_parser->cfg->max_ecs_tree_size_ipv6 = (uint32_t)atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 266:
#line 577 "./util/configparser.y"
	{
		OUTYY(("P(server_interface:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = yystack.l_mark[0].str;
	}
break;
case 267:
#line 590 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_interface:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->num_out_ifs == 0)
			cfg_parser->cfg->out_ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->out_ifs = realloc(
			cfg_parser->cfg->out_ifs, 
			(cfg_parser->cfg->num_out_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->out_ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->out_ifs[
				cfg_parser->cfg->num_out_ifs++] = yystack.l_mark[0].str;
	}
break;
case 268:
#line 605 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_range:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 269:
#line 614 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_port_permit:%s)\n", yystack.l_mark[0].str));
		if(!cfg_mark_ports(yystack.l_mark[0].str, 1, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yystack.l_mark[0].str);
	}
break;
case 270:
#line 623 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", yystack.l_mark[0].str));
		if(!cfg_mark_ports(yystack.l_mark[0].str, 0, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yystack.l_mark[0].str);
	}
break;
case 271:
#line 632 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 272:
#line 641 "./util/configparser.y"
	{
		OUTYY(("P(server_incoming_num_tcp:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 273:
#line 650 "./util/configparser.y"
	{
		OUTYY(("P(server_interface_automatic:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 274:
#line 659 "./util/configparser.y"
	{
		OUTYY(("P(server_do_ip4:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 275:
#line 668 "./util/configparser.y"
	{
		OUTYY(("P(server_do_ip6:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 276:
#line 677 "./util/configparser.y"
	{
		OUTYY(("P(server_do_udp:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 277:
#line 686 "./util/configparser.y"
	{
		OUTYY(("P(server_do_tcp:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 278:
#line 695 "./util/configparser.y"
	{
		OUTYY(("P(server_prefer_ip6:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip6 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 279:
#line 704 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_mss:%s)\n", yystack.l_mark[0].str));
                if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
                        yyerror("number expected");
                else cfg_parser->cfg->tcp_mss = atoi(yystack.l_mark[0].str);
                free(yystack.l_mark[0].str);
	}
break;
case 280:
#line 713 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_tcp_mss:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_tcp_mss = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 281:
#line 722 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_idle_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else if (atoi(yystack.l_mark[0].str) > 120000)
			cfg_parser->cfg->tcp_idle_timeout = 120000;
		else if (atoi(yystack.l_mark[0].str) < 1)
			cfg_parser->cfg->tcp_idle_timeout = 1;
		else cfg_parser->cfg->tcp_idle_timeout = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 282:
#line 735 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_keepalive:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp_keepalive = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 283:
#line 744 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_keepalive_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else if (atoi(yystack.l_mark[0].str) > 6553500)
			cfg_parser->cfg->tcp_keepalive_timeout = 6553500;
		else if (atoi(yystack.l_mark[0].str) < 1)
			cfg_parser->cfg->tcp_keepalive_timeout = 0;
		else cfg_parser->cfg->tcp_keepalive_timeout = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 284:
#line 757 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tcp_upstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 285:
#line 766 "./util/configparser.y"
	{
		OUTYY(("P(server_udp_upstream_without_downstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_upstream_without_downstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 286:
#line 775 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ssl_upstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 287:
#line 784 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_service_key:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->ssl_service_key);
		cfg_parser->cfg->ssl_service_key = yystack.l_mark[0].str;
	}
break;
case 288:
#line 791 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_service_pem:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->ssl_service_pem);
		cfg_parser->cfg->ssl_service_pem = yystack.l_mark[0].str;
	}
break;
case 289:
#line 798 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->ssl_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 290:
#line 807 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_cert_bundle:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = yystack.l_mark[0].str;
	}
break;
case 291:
#line 814 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_win_cert:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_win_cert = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 292:
#line 823 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_additional_port:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->tls_additional_port,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 293:
#line 831 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_ciphers:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_ciphers);
		cfg_parser->cfg->tls_ciphers = yystack.l_mark[0].str;
	}
break;
case 294:
#line 838 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_ciphersuites:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_ciphersuites);
		cfg_parser->cfg->tls_ciphersuites = yystack.l_mark[0].str;
	}
break;
case 295:
#line 845 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_session_ticket_keys:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append(&cfg_parser->cfg->tls_session_ticket_keys,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 296:
#line 853 "./util/configparser.y"
	{
		OUTYY(("P(server_use_systemd:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_systemd = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 297:
#line 862 "./util/configparser.y"
	{
		OUTYY(("P(server_do_daemonize:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 298:
#line 871 "./util/configparser.y"
	{
		OUTYY(("P(server_use_syslog:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp(yystack.l_mark[0].str, "yes")==0);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if(strcmp(yystack.l_mark[0].str, "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free(yystack.l_mark[0].str);
	}
break;
case 299:
#line 885 "./util/configparser.y"
	{
		OUTYY(("P(server_log_time_ascii:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 300:
#line 894 "./util/configparser.y"
	{
		OUTYY(("P(server_log_queries:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_queries = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 301:
#line 903 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_replies:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_replies = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 302:
#line 912 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_tag_queryreply:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_tag_queryreply = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 303:
#line 921 "./util/configparser.y"
	{
		OUTYY(("P(server_log_servfail:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_servfail = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 304:
#line 930 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_local_actions:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_local_actions = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 305:
#line 939 "./util/configparser.y"
	{
		OUTYY(("P(server_chroot:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = yystack.l_mark[0].str;
	}
break;
case 306:
#line 946 "./util/configparser.y"
	{
		OUTYY(("P(server_username:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = yystack.l_mark[0].str;
	}
break;
case 307:
#line 953 "./util/configparser.y"
	{
		OUTYY(("P(server_directory:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = yystack.l_mark[0].str;
		/* change there right away for includes relative to this */
		if(yystack.l_mark[0].str[0]) {
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
case 308:
#line 979 "./util/configparser.y"
	{
		OUTYY(("P(server_logfile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = yystack.l_mark[0].str;
		cfg_parser->cfg->use_syslog = 0;
	}
break;
case 309:
#line 987 "./util/configparser.y"
	{
		OUTYY(("P(server_pidfile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = yystack.l_mark[0].str;
	}
break;
case 310:
#line 994 "./util/configparser.y"
	{
		OUTYY(("P(server_root_hints:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 311:
#line 1001 "./util/configparser.y"
	{
		OUTYY(("P(server_dlv_anchor_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dlv_anchor_file);
		cfg_parser->cfg->dlv_anchor_file = yystack.l_mark[0].str;
	}
break;
case 312:
#line 1008 "./util/configparser.y"
	{
		OUTYY(("P(server_dlv_anchor:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dlv_anchor_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 313:
#line 1015 "./util/configparser.y"
	{
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 314:
#line 1023 "./util/configparser.y"
	{
		OUTYY(("P(server_trust_anchor_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 315:
#line 1031 "./util/configparser.y"
	{
		OUTYY(("P(server_trusted_keys_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 316:
#line 1039 "./util/configparser.y"
	{
		OUTYY(("P(server_trust_anchor:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 317:
#line 1046 "./util/configparser.y"
	{
		OUTYY(("P(server_trust_anchor_signaling:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->trust_anchor_signaling =
				(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 318:
#line 1057 "./util/configparser.y"
	{
		OUTYY(("P(server_root_key_sentinel:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->root_key_sentinel =
				(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 319:
#line 1068 "./util/configparser.y"
	{
		OUTYY(("P(server_domain_insecure:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 320:
#line 1075 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_identity:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 321:
#line 1084 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_version:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 322:
#line 1093 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_trustanchor:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_trustanchor = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 323:
#line 1102 "./util/configparser.y"
	{
		OUTYY(("P(server_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = yystack.l_mark[0].str;
	}
break;
case 324:
#line 1109 "./util/configparser.y"
	{
		OUTYY(("P(server_version:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = yystack.l_mark[0].str;
	}
break;
case 325:
#line 1116 "./util/configparser.y"
	{
		OUTYY(("P(server_so_rcvbuf:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->so_rcvbuf))
			yyerror("buffer size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 326:
#line 1124 "./util/configparser.y"
	{
		OUTYY(("P(server_so_sndbuf:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->so_sndbuf))
			yyerror("buffer size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 327:
#line 1132 "./util/configparser.y"
	{
        OUTYY(("P(server_so_reuseport:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->so_reuseport =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 328:
#line 1142 "./util/configparser.y"
	{
        OUTYY(("P(server_ip_transparent:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_transparent =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 329:
#line 1152 "./util/configparser.y"
	{
        OUTYY(("P(server_ip_freebind:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_freebind =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 330:
#line 1162 "./util/configparser.y"
	{
		OUTYY(("P(server_stream_wait_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->stream_wait_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 331:
#line 1170 "./util/configparser.y"
	{
		OUTYY(("P(server_edns_buffer_size:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else if (atoi(yystack.l_mark[0].str) < 12)
			yyerror("edns buffer size too small");
		else if (atoi(yystack.l_mark[0].str) > 65535)
			cfg_parser->cfg->edns_buffer_size = 65535;
		else cfg_parser->cfg->edns_buffer_size = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 332:
#line 1183 "./util/configparser.y"
	{
		OUTYY(("P(server_msg_buffer_size:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else if (atoi(yystack.l_mark[0].str) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else cfg_parser->cfg->msg_buffer_size = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 333:
#line 1194 "./util/configparser.y"
	{
		OUTYY(("P(server_msg_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 334:
#line 1202 "./util/configparser.y"
	{
		OUTYY(("P(server_msg_cache_slabs:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->msg_cache_slabs = atoi(yystack.l_mark[0].str);
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 335:
#line 1215 "./util/configparser.y"
	{
		OUTYY(("P(server_num_queries_per_thread:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 336:
#line 1224 "./util/configparser.y"
	{
		OUTYY(("P(server_jostle_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 337:
#line 1233 "./util/configparser.y"
	{
		OUTYY(("P(server_delay_close:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->delay_close = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 338:
#line 1242 "./util/configparser.y"
	{
		OUTYY(("P(server_unblock_lan_zones:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->unblock_lan_zones = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 339:
#line 1252 "./util/configparser.y"
	{
		OUTYY(("P(server_insecure_lan_zones:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->insecure_lan_zones = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 340:
#line 1262 "./util/configparser.y"
	{
		OUTYY(("P(server_rrset_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 341:
#line 1270 "./util/configparser.y"
	{
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->rrset_cache_slabs = atoi(yystack.l_mark[0].str);
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 342:
#line 1283 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_host_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 343:
#line 1292 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_lame_ttl:%s)\n", yystack.l_mark[0].str));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 344:
#line 1300 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 345:
#line 1309 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", yystack.l_mark[0].str));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 346:
#line 1317 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_slabs:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->infra_cache_slabs = atoi(yystack.l_mark[0].str);
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 347:
#line 1330 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_min_rtt:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_min_rtt = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 348:
#line 1339 "./util/configparser.y"
	{
		OUTYY(("P(server_target_fetch_policy:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = yystack.l_mark[0].str;
	}
break;
case 349:
#line 1346 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_short_bufsize:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 350:
#line 1356 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_large_queries:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 351:
#line 1366 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_glue:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 352:
#line 1376 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 353:
#line 1386 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_below_nxdomain:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_below_nxdomain = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 354:
#line 1396 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_referral_path:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 355:
#line 1406 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_algo_downgrade:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_algo_downgrade = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 356:
#line 1416 "./util/configparser.y"
	{
		OUTYY(("P(server_use_caps_for_id:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 357:
#line 1426 "./util/configparser.y"
	{
		OUTYY(("P(server_caps_whitelist:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 358:
#line 1433 "./util/configparser.y"
	{
		OUTYY(("P(server_private_address:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 359:
#line 1440 "./util/configparser.y"
	{
		OUTYY(("P(server_private_domain:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 360:
#line 1447 "./util/configparser.y"
	{
		OUTYY(("P(server_prefetch:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 361:
#line 1456 "./util/configparser.y"
	{
		OUTYY(("P(server_prefetch_key:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch_key = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 362:
#line 1465 "./util/configparser.y"
	{
		OUTYY(("P(server_deny_any:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->deny_any = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 363:
#line 1474 "./util/configparser.y"
	{
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 364:
#line 1483 "./util/configparser.y"
	{
		OUTYY(("P(server_do_not_query_address:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 365:
#line 1490 "./util/configparser.y"
	{
		OUTYY(("P(server_do_not_query_localhost:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 366:
#line 1500 "./util/configparser.y"
	{
		OUTYY(("P(server_access_control:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "deny")!=0 && strcmp(yystack.l_mark[0].str, "refuse")!=0 &&
			strcmp(yystack.l_mark[0].str, "deny_non_local")!=0 &&
			strcmp(yystack.l_mark[0].str, "refuse_non_local")!=0 &&
			strcmp(yystack.l_mark[0].str, "allow_setrd")!=0 && 
			strcmp(yystack.l_mark[0].str, "allow")!=0 && 
			strcmp(yystack.l_mark[0].str, "allow_snoop")!=0 &&
			strcmp(yystack.l_mark[0].str, "allow_cookie")!=0) {
			yyerror("expected deny, refuse, deny_non_local, "
				"refuse_non_local, allow, allow_setrd, "
				"allow_snoop or allow_cookie in access control "
				"action");
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding acl");
		}
	}
break;
case 367:
#line 1522 "./util/configparser.y"
	{
		OUTYY(("P(server_module_conf:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = yystack.l_mark[0].str;
	}
break;
case 368:
#line 1529 "./util/configparser.y"
	{
		OUTYY(("P(server_val_override_date:%s)\n", yystack.l_mark[0].str));
		if(*yystack.l_mark[0].str == '\0' || strcmp(yystack.l_mark[0].str, "0") == 0) {
			cfg_parser->cfg->val_date_override = 0;
		} else if(strlen(yystack.l_mark[0].str) == 14) {
			cfg_parser->cfg->val_date_override = 
				cfg_convert_timeval(yystack.l_mark[0].str);
			if(!cfg_parser->cfg->val_date_override)
				yyerror("bad date/time specification");
		} else {
			if(atoi(yystack.l_mark[0].str) == 0)
				yyerror("number expected");
			cfg_parser->cfg->val_date_override = atoi(yystack.l_mark[0].str);
		}
		free(yystack.l_mark[0].str);
	}
break;
case 369:
#line 1547 "./util/configparser.y"
	{
		OUTYY(("P(server_val_sig_skew_min:%s)\n", yystack.l_mark[0].str));
		if(*yystack.l_mark[0].str == '\0' || strcmp(yystack.l_mark[0].str, "0") == 0) {
			cfg_parser->cfg->val_sig_skew_min = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_min = atoi(yystack.l_mark[0].str);
			if(!cfg_parser->cfg->val_sig_skew_min)
				yyerror("number expected");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 370:
#line 1560 "./util/configparser.y"
	{
		OUTYY(("P(server_val_sig_skew_max:%s)\n", yystack.l_mark[0].str));
		if(*yystack.l_mark[0].str == '\0' || strcmp(yystack.l_mark[0].str, "0") == 0) {
			cfg_parser->cfg->val_sig_skew_max = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_max = atoi(yystack.l_mark[0].str);
			if(!cfg_parser->cfg->val_sig_skew_max)
				yyerror("number expected");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 371:
#line 1573 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_max_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 372:
#line 1582 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_max_negative_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_negative_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 373:
#line 1591 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_min_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 374:
#line 1600 "./util/configparser.y"
	{
		OUTYY(("P(server_bogus_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 375:
#line 1609 "./util/configparser.y"
	{
		OUTYY(("P(server_val_clean_additional:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 376:
#line 1619 "./util/configparser.y"
	{
		OUTYY(("P(server_val_permissive_mode:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 377:
#line 1629 "./util/configparser.y"
	{
		OUTYY(("P(server_aggressive_nsec:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->aggressive_nsec =
				(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 378:
#line 1640 "./util/configparser.y"
	{
		OUTYY(("P(server_ignore_cd_flag:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ignore_cd = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 379:
#line 1649 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 380:
#line 1658 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 381:
#line 1667 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_ttl_reset:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired_ttl_reset = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 382:
#line 1676 "./util/configparser.y"
	{
		OUTYY(("P(server_fake_dsa:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
#ifdef HAVE_SSL
		else fake_dsa = (strcmp(yystack.l_mark[0].str, "yes")==0);
		if(fake_dsa)
			log_warn("test option fake_dsa is enabled");
#endif
		free(yystack.l_mark[0].str);
	}
break;
case 383:
#line 1689 "./util/configparser.y"
	{
		OUTYY(("P(server_fake_sha1:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
#ifdef HAVE_SSL
		else fake_sha1 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		if(fake_sha1)
			log_warn("test option fake_sha1 is enabled");
#endif
		free(yystack.l_mark[0].str);
	}
break;
case 384:
#line 1702 "./util/configparser.y"
	{
		OUTYY(("P(server_val_log_level:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->val_log_level = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 385:
#line 1711 "./util/configparser.y"
	{
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = yystack.l_mark[0].str;
	}
break;
case 386:
#line 1718 "./util/configparser.y"
	{
		OUTYY(("P(server_add_holddown:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->add_holddown = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 387:
#line 1727 "./util/configparser.y"
	{
		OUTYY(("P(server_del_holddown:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->del_holddown = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 388:
#line 1736 "./util/configparser.y"
	{
		OUTYY(("P(server_keep_missing:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->keep_missing = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 389:
#line 1745 "./util/configparser.y"
	{
		OUTYY(("P(server_permit_small_holddown:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->permit_small_holddown =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 390:
#line 1754 "./util/configparser.y"
	{
		OUTYY(("P(server_key_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 391:
#line 1762 "./util/configparser.y"
	{
		OUTYY(("P(server_key_cache_slabs:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->key_cache_slabs = atoi(yystack.l_mark[0].str);
			if(!is_pow2(cfg_parser->cfg->key_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 392:
#line 1775 "./util/configparser.y"
	{
		OUTYY(("P(server_neg_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 393:
#line 1783 "./util/configparser.y"
	{
		OUTYY(("P(server_local_zone:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "static")!=0 && strcmp(yystack.l_mark[0].str, "deny")!=0 &&
		   strcmp(yystack.l_mark[0].str, "refuse")!=0 && strcmp(yystack.l_mark[0].str, "redirect")!=0 &&
		   strcmp(yystack.l_mark[0].str, "transparent")!=0 && strcmp(yystack.l_mark[0].str, "nodefault")!=0
		   && strcmp(yystack.l_mark[0].str, "typetransparent")!=0
		   && strcmp(yystack.l_mark[0].str, "always_transparent")!=0
		   && strcmp(yystack.l_mark[0].str, "always_refuse")!=0
		   && strcmp(yystack.l_mark[0].str, "always_nxdomain")!=0
		   && strcmp(yystack.l_mark[0].str, "noview")!=0
		   && strcmp(yystack.l_mark[0].str, "inform")!=0 && strcmp(yystack.l_mark[0].str, "inform_deny")!=0
		   && strcmp(yystack.l_mark[0].str, "inform_redirect") != 0
			 && strcmp(yystack.l_mark[0].str, "ipset") != 0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"inform_redirect, always_transparent, "
				"always_refuse, always_nxdomain, noview "
				", nodefault or ipset");
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		} else if(strcmp(yystack.l_mark[0].str, "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, yystack.l_mark[-1].str))
				fatal_exit("out of memory adding local-zone");
			free(yystack.l_mark[0].str);
#ifdef USE_IPSET
		} else if(strcmp(yystack.l_mark[0].str, "ipset")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_ipset, yystack.l_mark[-1].str))
				fatal_exit("out of memory adding local-zone");
			free(yystack.l_mark[0].str);
#endif
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones, 
				yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding local-zone");
		}
	}
break;
case 394:
#line 1824 "./util/configparser.y"
	{
		OUTYY(("P(server_local_data:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, yystack.l_mark[0].str))
			fatal_exit("out of memory adding local-data");
	}
break;
case 395:
#line 1831 "./util/configparser.y"
	{
		char* ptr;
		OUTYY(("P(server_local_data_ptr:%s)\n", yystack.l_mark[0].str));
		ptr = cfg_ptr_reverse(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
break;
case 396:
#line 1846 "./util/configparser.y"
	{
		OUTYY(("P(server_minimal_responses:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->minimal_responses =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 397:
#line 1856 "./util/configparser.y"
	{
		OUTYY(("P(server_rrset_roundrobin:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->rrset_roundrobin =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 398:
#line 1866 "./util/configparser.y"
	{
		OUTYY(("P(server_unknown_server_time_limit:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->unknown_server_time_limit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 399:
#line 1873 "./util/configparser.y"
	{
		OUTYY(("P(server_max_udp_size:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->max_udp_size = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 400:
#line 1880 "./util/configparser.y"
	{
		OUTYY(("P(dns64_prefix:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dns64_prefix);
		cfg_parser->cfg->dns64_prefix = yystack.l_mark[0].str;
	}
break;
case 401:
#line 1887 "./util/configparser.y"
	{
		OUTYY(("P(server_dns64_synthall:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dns64_synthall = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 402:
#line 1896 "./util/configparser.y"
	{
		OUTYY(("P(dns64_ignore_aaaa:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dns64_ignore_aaaa,
			yystack.l_mark[0].str))
			fatal_exit("out of memory adding dns64-ignore-aaaa");
	}
break;
case 403:
#line 1904 "./util/configparser.y"
	{
		char* p, *s = yystack.l_mark[0].str;
		OUTYY(("P(server_define_tag:%s)\n", yystack.l_mark[0].str));
		while((p=strsep(&s, " \t\n")) != NULL) {
			if(*p) {
				if(!config_add_tag(cfg_parser->cfg, p))
					yyerror("could not define-tag, "
						"out of memory");
			}
		}
		free(yystack.l_mark[0].str);
	}
break;
case 404:
#line 1918 "./util/configparser.y"
	{
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, yystack.l_mark[0].str,
			&len);
		free(yystack.l_mark[0].str);
		OUTYY(("P(server_local_zone_tag:%s)\n", yystack.l_mark[-1].str));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free(yystack.l_mark[-1].str);
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->local_zone_tags,
				yystack.l_mark[-1].str, bitlist, len)) {
				yyerror("out of memory");
				free(yystack.l_mark[-1].str);
			}
		}
	}
break;
case 405:
#line 1939 "./util/configparser.y"
	{
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, yystack.l_mark[0].str,
			&len);
		free(yystack.l_mark[0].str);
		OUTYY(("P(server_access_control_tag:%s)\n", yystack.l_mark[-1].str));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free(yystack.l_mark[-1].str);
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->acl_tags,
				yystack.l_mark[-1].str, bitlist, len)) {
				yyerror("out of memory");
				free(yystack.l_mark[-1].str);
			}
		}
	}
break;
case 406:
#line 1960 "./util/configparser.y"
	{
		OUTYY(("P(server_access_control_tag_action:%s %s %s)\n", yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_actions,
			yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str)) {
			yyerror("out of memory");
			free(yystack.l_mark[-2].str);
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		}
	}
break;
case 407:
#line 1972 "./util/configparser.y"
	{
		OUTYY(("P(server_access_control_tag_data:%s %s %s)\n", yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_datas,
			yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str)) {
			yyerror("out of memory");
			free(yystack.l_mark[-2].str);
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		}
	}
break;
case 408:
#line 1984 "./util/configparser.y"
	{
		OUTYY(("P(server_local_zone_override:%s %s %s)\n", yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(!cfg_str3list_insert(&cfg_parser->cfg->local_zone_overrides,
			yystack.l_mark[-2].str, yystack.l_mark[-1].str, yystack.l_mark[0].str)) {
			yyerror("out of memory");
			free(yystack.l_mark[-2].str);
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		}
	}
break;
case 409:
#line 1996 "./util/configparser.y"
	{
		OUTYY(("P(server_access_control_view:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acl_view,
			yystack.l_mark[-1].str, yystack.l_mark[0].str)) {
			yyerror("out of memory");
		}
	}
break;
case 410:
#line 2005 "./util/configparser.y"
	{
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, yystack.l_mark[0].str,
			&len);
		free(yystack.l_mark[0].str);
		OUTYY(("P(response_ip_tag:%s)\n", yystack.l_mark[-1].str));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free(yystack.l_mark[-1].str);
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->respip_tags,
				yystack.l_mark[-1].str, bitlist, len)) {
				yyerror("out of memory");
				free(yystack.l_mark[-1].str);
			}
		}
	}
break;
case 411:
#line 2026 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ip_ratelimit:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 412:
#line 2036 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ratelimit:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 413:
#line 2045 "./util/configparser.y"
	{
  	OUTYY(("P(server_ip_ratelimit_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->ip_ratelimit_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 414:
#line 2053 "./util/configparser.y"
	{
		OUTYY(("P(server_ratelimit_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->ratelimit_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 415:
#line 2061 "./util/configparser.y"
	{
  	OUTYY(("P(server_ip_ratelimit_slabs:%s)\n", yystack.l_mark[0].str));
  	if(atoi(yystack.l_mark[0].str) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->ip_ratelimit_slabs = atoi(yystack.l_mark[0].str);
  		if(!is_pow2(cfg_parser->cfg->ip_ratelimit_slabs))
  			yyerror("must be a power of 2");
  	}
  	free(yystack.l_mark[0].str);
  }
break;
case 416:
#line 2074 "./util/configparser.y"
	{
		OUTYY(("P(server_ratelimit_slabs:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->ratelimit_slabs = atoi(yystack.l_mark[0].str);
			if(!is_pow2(cfg_parser->cfg->ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free(yystack.l_mark[0].str);
	}
break;
case 417:
#line 2087 "./util/configparser.y"
	{
		OUTYY(("P(server_ratelimit_for_domain:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0) {
			yyerror("number expected");
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_for_domain, yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding "
					"ratelimit-for-domain");
		}
	}
break;
case 418:
#line 2102 "./util/configparser.y"
	{
		OUTYY(("P(server_ratelimit_below_domain:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0) {
			yyerror("number expected");
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_below_domain, yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding "
					"ratelimit-below-domain");
		}
	}
break;
case 419:
#line 2117 "./util/configparser.y"
	{ 
  	OUTYY(("P(server_ip_ratelimit_factor:%s)\n", yystack.l_mark[0].str)); 
  	if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
  		yyerror("number expected");
  	else cfg_parser->cfg->ip_ratelimit_factor = atoi(yystack.l_mark[0].str);
  	free(yystack.l_mark[0].str);
	}
break;
case 420:
#line 2126 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ratelimit_factor:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit_factor = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 421:
#line 2135 "./util/configparser.y"
	{ 
		OUTYY(("P(low-rtt option is deprecated, use fast-server-num instead)\n"));
		free(yystack.l_mark[0].str);
	}
break;
case 422:
#line 2141 "./util/configparser.y"
	{ 
		OUTYY(("P(server_fast_server_num:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) <= 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_num = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 423:
#line 2150 "./util/configparser.y"
	{ 
		OUTYY(("P(server_fast_server_permil:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_permil = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 424:
#line 2159 "./util/configparser.y"
	{
		OUTYY(("P(server_qname_minimisation:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 425:
#line 2169 "./util/configparser.y"
	{
		OUTYY(("P(server_qname_minimisation_strict:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation_strict = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 426:
#line 2179 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_enabled:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_enabled = (strcmp(yystack.l_mark[0].str, "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 427:
#line 2192 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_ignore_bogus:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_ignore_bogus = (strcmp(yystack.l_mark[0].str, "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 428:
#line 2205 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_hook:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->ipsecmod_hook);
		cfg_parser->cfg->ipsecmod_hook = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 429:
#line 2217 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_max_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ipsecmod_max_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 430:
#line 2231 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_whitelist:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->ipsecmod_whitelist, yystack.l_mark[0].str))
			yyerror("out of memory");
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 431:
#line 2243 "./util/configparser.y"
	{
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_strict:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_strict = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 432:
#line 2257 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = yystack.l_mark[0].str;
	}
break;
case 433:
#line 2267 "./util/configparser.y"
	{
		OUTYY(("P(stub-host:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 434:
#line 2274 "./util/configparser.y"
	{
		OUTYY(("P(stub-addr:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 435:
#line 2281 "./util/configparser.y"
	{
		OUTYY(("P(stub-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 436:
#line 2290 "./util/configparser.y"
	{
		OUTYY(("P(stub-no-cache:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->no_cache=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 437:
#line 2299 "./util/configparser.y"
	{
		OUTYY(("P(stub-ssl-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->ssl_upstream = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 438:
#line 2309 "./util/configparser.y"
	{
		OUTYY(("P(stub-prime:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 439:
#line 2319 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = yystack.l_mark[0].str;
	}
break;
case 440:
#line 2329 "./util/configparser.y"
	{
		OUTYY(("P(forward-host:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 441:
#line 2336 "./util/configparser.y"
	{
		OUTYY(("P(forward-addr:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 442:
#line 2343 "./util/configparser.y"
	{
		OUTYY(("P(forward-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 443:
#line 2352 "./util/configparser.y"
	{
		OUTYY(("P(forward-no-cache:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->no_cache=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 444:
#line 2361 "./util/configparser.y"
	{
		OUTYY(("P(forward-ssl-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->ssl_upstream = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 445:
#line 2371 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = yystack.l_mark[0].str;
	}
break;
case 446:
#line 2381 "./util/configparser.y"
	{
		OUTYY(("P(zonefile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = yystack.l_mark[0].str;
	}
break;
case 447:
#line 2388 "./util/configparser.y"
	{
		OUTYY(("P(master:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 448:
#line 2395 "./util/configparser.y"
	{
		OUTYY(("P(url:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 449:
#line 2402 "./util/configparser.y"
	{
		OUTYY(("P(allow-notify:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 450:
#line 2410 "./util/configparser.y"
	{
		OUTYY(("P(for-downstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_downstream =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 451:
#line 2420 "./util/configparser.y"
	{
		OUTYY(("P(for-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_upstream =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 452:
#line 2430 "./util/configparser.y"
	{
		OUTYY(("P(fallback-enabled:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->fallback_enabled =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 453:
#line 2440 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->views->name)
			yyerror("view name override, there must be one "
				"name for one view");
		free(cfg_parser->cfg->views->name);
		cfg_parser->cfg->views->name = yystack.l_mark[0].str;
	}
break;
case 454:
#line 2450 "./util/configparser.y"
	{
		OUTYY(("P(view_local_zone:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "static")!=0 && strcmp(yystack.l_mark[0].str, "deny")!=0 &&
		   strcmp(yystack.l_mark[0].str, "refuse")!=0 && strcmp(yystack.l_mark[0].str, "redirect")!=0 &&
		   strcmp(yystack.l_mark[0].str, "transparent")!=0 && strcmp(yystack.l_mark[0].str, "nodefault")!=0
		   && strcmp(yystack.l_mark[0].str, "typetransparent")!=0
		   && strcmp(yystack.l_mark[0].str, "always_transparent")!=0
		   && strcmp(yystack.l_mark[0].str, "always_refuse")!=0
		   && strcmp(yystack.l_mark[0].str, "always_nxdomain")!=0
		   && strcmp(yystack.l_mark[0].str, "noview")!=0
		   && strcmp(yystack.l_mark[0].str, "inform")!=0 && strcmp(yystack.l_mark[0].str, "inform_deny")!=0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"always_transparent, always_refuse, "
				"always_nxdomain, noview or nodefault");
			free(yystack.l_mark[-1].str);
			free(yystack.l_mark[0].str);
		} else if(strcmp(yystack.l_mark[0].str, "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_zones_nodefault, yystack.l_mark[-1].str))
				fatal_exit("out of memory adding local-zone");
			free(yystack.l_mark[0].str);
#ifdef USE_IPSET
		} else if(strcmp(yystack.l_mark[0].str, "ipset")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_zones_ipset, yystack.l_mark[-1].str))
				fatal_exit("out of memory adding local-zone");
			free(yystack.l_mark[0].str);
#endif
		} else {
			if(!cfg_str2list_insert(
				&cfg_parser->cfg->views->local_zones, 
				yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding local-zone");
		}
	}
break;
case 455:
#line 2489 "./util/configparser.y"
	{
		OUTYY(("P(view_response_ip:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		validate_respip_action(yystack.l_mark[0].str);
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_actions, yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding per-view "
				"response-ip action");
	}
break;
case 456:
#line 2499 "./util/configparser.y"
	{
		OUTYY(("P(view_response_ip_data:%s)\n", yystack.l_mark[-1].str));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_data, yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip-data");
	}
break;
case 457:
#line 2507 "./util/configparser.y"
	{
		OUTYY(("P(view_local_data:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->views->local_data, yystack.l_mark[0].str)) {
			fatal_exit("out of memory adding local-data");
		}
	}
break;
case 458:
#line 2515 "./util/configparser.y"
	{
		char* ptr;
		OUTYY(("P(view_local_data_ptr:%s)\n", yystack.l_mark[0].str));
		ptr = cfg_ptr_reverse(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->views->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
break;
case 459:
#line 2530 "./util/configparser.y"
	{
		OUTYY(("P(view-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->views->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 460:
#line 2539 "./util/configparser.y"
	{ 
		OUTYY(("\nP(remote-control:)\n")); 
	}
break;
case 471:
#line 2550 "./util/configparser.y"
	{
		OUTYY(("P(control_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 472:
#line 2560 "./util/configparser.y"
	{
		OUTYY(("P(control_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 473:
#line 2569 "./util/configparser.y"
	{
		OUTYY(("P(control_interface:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 474:
#line 2576 "./util/configparser.y"
	{
		OUTYY(("P(control_use_cert:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->control_use_cert = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 475:
#line 2583 "./util/configparser.y"
	{
		OUTYY(("P(rc_server_key_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = yystack.l_mark[0].str;
	}
break;
case 476:
#line 2590 "./util/configparser.y"
	{
		OUTYY(("P(rc_server_cert_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = yystack.l_mark[0].str;
	}
break;
case 477:
#line 2597 "./util/configparser.y"
	{
		OUTYY(("P(rc_control_key_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = yystack.l_mark[0].str;
	}
break;
case 478:
#line 2604 "./util/configparser.y"
	{
		OUTYY(("P(rc_control_cert_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = yystack.l_mark[0].str;
	}
break;
case 479:
#line 2611 "./util/configparser.y"
	{
		OUTYY(("\nP(dnstap:)\n"));
	}
break;
case 494:
#line 2628 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 495:
#line 2637 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_socket_path:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_socket_path);
		cfg_parser->cfg->dnstap_socket_path = yystack.l_mark[0].str;
	}
break;
case 496:
#line 2644 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_send_identity:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_identity = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 497:
#line 2653 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_send_version:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_version = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 498:
#line 2662 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_identity);
		cfg_parser->cfg->dnstap_identity = yystack.l_mark[0].str;
	}
break;
case 499:
#line 2669 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_version:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_version);
		cfg_parser->cfg->dnstap_version = yystack.l_mark[0].str;
	}
break;
case 500:
#line 2676 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_resolver_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 501:
#line 2686 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_resolver_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 502:
#line 2696 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_client_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 503:
#line 2706 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_client_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 504:
#line 2716 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_forwarder_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 505:
#line 2726 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_forwarder_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 506:
#line 2736 "./util/configparser.y"
	{ 
		OUTYY(("\nP(python:)\n")); 
	}
break;
case 510:
#line 2745 "./util/configparser.y"
	{
		OUTYY(("P(python-script:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->python_script, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 511:
#line 2751 "./util/configparser.y"
	{
		OUTYY(("P(disable_dnssec_lame_check:%s)\n", yystack.l_mark[0].str));
		if (strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_dnssec_lame_check =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 512:
#line 2761 "./util/configparser.y"
	{
		OUTYY(("P(server_log_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = yystack.l_mark[0].str;
	}
break;
case 513:
#line 2768 "./util/configparser.y"
	{
		OUTYY(("P(server_response_ip:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		validate_respip_action(yystack.l_mark[0].str);
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_actions,
			yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip");
	}
break;
case 514:
#line 2777 "./util/configparser.y"
	{
		OUTYY(("P(server_response_ip_data:%s)\n", yystack.l_mark[-1].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_data,
			yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip-data");
	}
break;
case 515:
#line 2785 "./util/configparser.y"
	{
		OUTYY(("\nP(dnscrypt:)\n"));
	}
break;
case 528:
#line 2801 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnscrypt = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 529:
#line 2811 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->dnscrypt_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 530:
#line 2820 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnscrypt_provider);
		cfg_parser->cfg->dnscrypt_provider = yystack.l_mark[0].str;
	}
break;
case 531:
#line 2827 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider_cert:%s)\n", yystack.l_mark[0].str));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_provider_cert, yystack.l_mark[0].str))
			log_warn("dnscrypt-provider-cert %s is a duplicate", yystack.l_mark[0].str);
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-provider-cert");
	}
break;
case 532:
#line 2836 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider_cert_rotated:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert_rotated, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-provider-cert-rotated");
	}
break;
case 533:
#line 2843 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_secret_key:%s)\n", yystack.l_mark[0].str));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_secret_key, yystack.l_mark[0].str))
			log_warn("dnscrypt-secret-key: %s is a duplicate", yystack.l_mark[0].str);
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_secret_key, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-secret-key");
	}
break;
case 534:
#line 2852 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_shared_secret_cache_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->dnscrypt_shared_secret_cache_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 535:
#line 2860 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_shared_secret_cache_slabs:%s)\n", yystack.l_mark[0].str));
  	if(atoi(yystack.l_mark[0].str) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs = atoi(yystack.l_mark[0].str);
  		if(!is_pow2(cfg_parser->cfg->dnscrypt_shared_secret_cache_slabs))
  			yyerror("must be a power of 2");
  	}
  	free(yystack.l_mark[0].str);
  }
break;
case 536:
#line 2873 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_nonce_cache_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->dnscrypt_nonce_cache_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 537:
#line 2881 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_nonce_cache_slabs:%s)\n", yystack.l_mark[0].str));
  	if(atoi(yystack.l_mark[0].str) == 0)
  		yyerror("number expected");
  	else {
  		cfg_parser->cfg->dnscrypt_nonce_cache_slabs = atoi(yystack.l_mark[0].str);
  		if(!is_pow2(cfg_parser->cfg->dnscrypt_nonce_cache_slabs))
  			yyerror("must be a power of 2");
  	}
  	free(yystack.l_mark[0].str);
  }
break;
case 538:
#line 2894 "./util/configparser.y"
	{
		OUTYY(("\nP(cachedb:)\n"));
	}
break;
case 546:
#line 2904 "./util/configparser.y"
	{
	#ifdef USE_CACHEDB
		OUTYY(("P(backend:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->cachedb_backend)
			yyerror("cachedb backend override, there must be one "
				"backend");
		free(cfg_parser->cfg->cachedb_backend);
		cfg_parser->cfg->cachedb_backend = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 547:
#line 2919 "./util/configparser.y"
	{
	#ifdef USE_CACHEDB
		OUTYY(("P(secret-seed:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->cachedb_secret)
			yyerror("cachedb secret-seed override, there must be "
				"only one secret");
		free(cfg_parser->cfg->cachedb_secret);
		cfg_parser->cfg->cachedb_secret = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 548:
#line 2934 "./util/configparser.y"
	{
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_server_host:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->redis_server_host);
		cfg_parser->cfg->redis_server_host = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 549:
#line 2946 "./util/configparser.y"
	{
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		int port;
		OUTYY(("P(redis_server_port:%s)\n", yystack.l_mark[0].str));
		port = atoi(yystack.l_mark[0].str);
		if(port == 0 || port < 0 || port > 65535)
			yyerror("valid redis server port number expected");
		else cfg_parser->cfg->redis_server_port = port;
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 550:
#line 2961 "./util/configparser.y"
	{
	#if defined(USE_CACHEDB) && defined(USE_REDIS)
		OUTYY(("P(redis_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("redis timeout value expected");
		else cfg_parser->cfg->redis_timeout = atoi(yystack.l_mark[0].str);
	#else
		OUTYY(("P(Compiled without cachedb or redis, ignoring)\n"));
	#endif
		free(yystack.l_mark[0].str);
	}
break;
case 551:
#line 2974 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_connection_limit:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if (atoi(yystack.l_mark[0].str) < 0)
			yyerror("positive number expected");
		else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->tcp_connection_limits, yystack.l_mark[-1].str, yystack.l_mark[0].str))
				fatal_exit("out of memory adding tcp connection limit");
		}
	}
break;
case 552:
#line 2985 "./util/configparser.y"
	{
		OUTYY(("P(server_answer_cookie:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_answer_cookie = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 553:
#line 2994 "./util/configparser.y"
	{
		uint8_t secret[32];
		size_t secret_len = sizeof(secret);

		OUTYY(("P(server_cookie_secret:%s)\n", yystack.l_mark[0].str));
		if (sldns_str2wire_hex_buf(yystack.l_mark[0].str, secret, &secret_len)
		|| (  secret_len != 16 && secret_len != 24 && secret_len != 32))
			yyerror("expected 128, 192 or 256 bit hex string");
		else {
			cfg_parser->cfg->cookie_secret_len = secret_len;
			memcpy(cfg_parser->cfg->cookie_secret, secret,
				sizeof(cfg_parser->cfg->cookie_secret));
		}
		free(yystack.l_mark[0].str);
	}
break;
case 554:
#line 3010 "./util/configparser.y"
	{
			OUTYY(("\nP(ipset:)\n"));
		}
break;
case 559:
#line 3019 "./util/configparser.y"
	{
		#ifdef USE_IPSET
			OUTYY(("P(name-v4:%s)\n", yystack.l_mark[0].str));
			if(cfg_parser->cfg->ipset_name_v4)
				yyerror("ipset name v4 override, there must be one "
					"name for ip v4");
			free(cfg_parser->cfg->ipset_name_v4);
			cfg_parser->cfg->ipset_name_v4 = yystack.l_mark[0].str;
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free(yystack.l_mark[0].str);
		#endif
		}
break;
case 560:
#line 3034 "./util/configparser.y"
	{
		#ifdef USE_IPSET
			OUTYY(("P(name-v6:%s)\n", yystack.l_mark[0].str));
			if(cfg_parser->cfg->ipset_name_v6)
				yyerror("ipset name v6 override, there must be one "
					"name for ip v6");
			free(cfg_parser->cfg->ipset_name_v6);
			cfg_parser->cfg->ipset_name_v6 = yystack.l_mark[0].str;
		#else
			OUTYY(("P(Compiled without ipset, ignoring)\n"));
			free(yystack.l_mark[0].str);
		#endif
		}
break;
#line 4897 "util/configparser.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                yys = yyname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
