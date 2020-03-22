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
#define VAR_PREFER_IP4 271
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
#define VAR_DNSTAP_IP 409
#define VAR_DNSTAP_TLS 410
#define VAR_DNSTAP_TLS_SERVER_NAME 411
#define VAR_DNSTAP_TLS_CERT_BUNDLE 412
#define VAR_DNSTAP_TLS_CLIENT_KEY_FILE 413
#define VAR_DNSTAP_TLS_CLIENT_CERT_FILE 414
#define VAR_DNSTAP_SEND_IDENTITY 415
#define VAR_DNSTAP_SEND_VERSION 416
#define VAR_DNSTAP_IDENTITY 417
#define VAR_DNSTAP_VERSION 418
#define VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES 419
#define VAR_DNSTAP_LOG_RESOLVER_RESPONSE_MESSAGES 420
#define VAR_DNSTAP_LOG_CLIENT_QUERY_MESSAGES 421
#define VAR_DNSTAP_LOG_CLIENT_RESPONSE_MESSAGES 422
#define VAR_DNSTAP_LOG_FORWARDER_QUERY_MESSAGES 423
#define VAR_DNSTAP_LOG_FORWARDER_RESPONSE_MESSAGES 424
#define VAR_RESPONSE_IP_TAG 425
#define VAR_RESPONSE_IP 426
#define VAR_RESPONSE_IP_DATA 427
#define VAR_HARDEN_ALGO_DOWNGRADE 428
#define VAR_IP_TRANSPARENT 429
#define VAR_DISABLE_DNSSEC_LAME_CHECK 430
#define VAR_IP_RATELIMIT 431
#define VAR_IP_RATELIMIT_SLABS 432
#define VAR_IP_RATELIMIT_SIZE 433
#define VAR_RATELIMIT 434
#define VAR_RATELIMIT_SLABS 435
#define VAR_RATELIMIT_SIZE 436
#define VAR_RATELIMIT_FOR_DOMAIN 437
#define VAR_RATELIMIT_BELOW_DOMAIN 438
#define VAR_IP_RATELIMIT_FACTOR 439
#define VAR_RATELIMIT_FACTOR 440
#define VAR_SEND_CLIENT_SUBNET 441
#define VAR_CLIENT_SUBNET_ZONE 442
#define VAR_CLIENT_SUBNET_ALWAYS_FORWARD 443
#define VAR_CLIENT_SUBNET_OPCODE 444
#define VAR_MAX_CLIENT_SUBNET_IPV4 445
#define VAR_MAX_CLIENT_SUBNET_IPV6 446
#define VAR_MIN_CLIENT_SUBNET_IPV4 447
#define VAR_MIN_CLIENT_SUBNET_IPV6 448
#define VAR_MAX_ECS_TREE_SIZE_IPV4 449
#define VAR_MAX_ECS_TREE_SIZE_IPV6 450
#define VAR_CAPS_WHITELIST 451
#define VAR_CACHE_MAX_NEGATIVE_TTL 452
#define VAR_PERMIT_SMALL_HOLDDOWN 453
#define VAR_QNAME_MINIMISATION 454
#define VAR_QNAME_MINIMISATION_STRICT 455
#define VAR_IP_FREEBIND 456
#define VAR_DEFINE_TAG 457
#define VAR_LOCAL_ZONE_TAG 458
#define VAR_ACCESS_CONTROL_TAG 459
#define VAR_LOCAL_ZONE_OVERRIDE 460
#define VAR_ACCESS_CONTROL_TAG_ACTION 461
#define VAR_ACCESS_CONTROL_TAG_DATA 462
#define VAR_VIEW 463
#define VAR_ACCESS_CONTROL_VIEW 464
#define VAR_VIEW_FIRST 465
#define VAR_SERVE_EXPIRED 466
#define VAR_SERVE_EXPIRED_TTL 467
#define VAR_SERVE_EXPIRED_TTL_RESET 468
#define VAR_SERVE_EXPIRED_REPLY_TTL 469
#define VAR_SERVE_EXPIRED_CLIENT_TIMEOUT 470
#define VAR_FAKE_DSA 471
#define VAR_FAKE_SHA1 472
#define VAR_LOG_IDENTITY 473
#define VAR_HIDE_TRUSTANCHOR 474
#define VAR_TRUST_ANCHOR_SIGNALING 475
#define VAR_AGGRESSIVE_NSEC 476
#define VAR_USE_SYSTEMD 477
#define VAR_SHM_ENABLE 478
#define VAR_SHM_KEY 479
#define VAR_ROOT_KEY_SENTINEL 480
#define VAR_DNSCRYPT 481
#define VAR_DNSCRYPT_ENABLE 482
#define VAR_DNSCRYPT_PORT 483
#define VAR_DNSCRYPT_PROVIDER 484
#define VAR_DNSCRYPT_SECRET_KEY 485
#define VAR_DNSCRYPT_PROVIDER_CERT 486
#define VAR_DNSCRYPT_PROVIDER_CERT_ROTATED 487
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE 488
#define VAR_DNSCRYPT_SHARED_SECRET_CACHE_SLABS 489
#define VAR_DNSCRYPT_NONCE_CACHE_SIZE 490
#define VAR_DNSCRYPT_NONCE_CACHE_SLABS 491
#define VAR_IPSECMOD_ENABLED 492
#define VAR_IPSECMOD_HOOK 493
#define VAR_IPSECMOD_IGNORE_BOGUS 494
#define VAR_IPSECMOD_MAX_TTL 495
#define VAR_IPSECMOD_WHITELIST 496
#define VAR_IPSECMOD_STRICT 497
#define VAR_CACHEDB 498
#define VAR_CACHEDB_BACKEND 499
#define VAR_CACHEDB_SECRETSEED 500
#define VAR_CACHEDB_REDISHOST 501
#define VAR_CACHEDB_REDISPORT 502
#define VAR_CACHEDB_REDISTIMEOUT 503
#define VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM 504
#define VAR_FOR_UPSTREAM 505
#define VAR_AUTH_ZONE 506
#define VAR_ZONEFILE 507
#define VAR_MASTER 508
#define VAR_URL 509
#define VAR_FOR_DOWNSTREAM 510
#define VAR_FALLBACK_ENABLED 511
#define VAR_TLS_ADDITIONAL_PORT 512
#define VAR_LOW_RTT 513
#define VAR_LOW_RTT_PERMIL 514
#define VAR_FAST_SERVER_PERMIL 515
#define VAR_FAST_SERVER_NUM 516
#define VAR_ALLOW_NOTIFY 517
#define VAR_TLS_WIN_CERT 518
#define VAR_TCP_CONNECTION_LIMIT 519
#define VAR_ANSWER_COOKIE 520
#define VAR_COOKIE_SECRET 521
#define VAR_FORWARD_NO_CACHE 522
#define VAR_STUB_NO_CACHE 523
#define VAR_LOG_SERVFAIL 524
#define VAR_DENY_ANY 525
#define VAR_UNKNOWN_SERVER_TIME_LIMIT 526
#define VAR_LOG_TAG_QUERYREPLY 527
#define VAR_STREAM_WAIT_SIZE 528
#define VAR_TLS_CIPHERS 529
#define VAR_TLS_CIPHERSUITES 530
#define VAR_IPSET 531
#define VAR_IPSET_NAME_V4 532
#define VAR_IPSET_NAME_V6 533
#define VAR_TLS_SESSION_TICKET_KEYS 534
#define VAR_RPZ 535
#define VAR_TAGS 536
#define VAR_RPZ_ACTION_OVERRIDE 537
#define VAR_RPZ_CNAME_OVERRIDE 538
#define VAR_RPZ_LOG 539
#define VAR_RPZ_LOG_NAME 540
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    2,    3,    3,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,
   26,    4,    5,    5,  221,  221,  221,  221,  221,  221,
  221,    6,    7,    7,  229,  229,  229,  229,  229,  229,
   14,   15,   15,  236,  236,  236,  236,  236,  236,  236,
   22,   23,   23,  244,  244,  244,  244,  244,  244,  244,
  244,  253,  254,  255,  256,  257,   24,   25,   25,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,   27,
   28,   91,   94,  103,  190,  191,   29,  155,  156,  157,
  158,  159,  160,  161,  162,  163,  164,   42,   82,   30,
   95,   96,   53,   75,   90,   31,   32,   35,   36,   33,
   34,   37,   38,   39,   40,   41,  126,  202,  127,  129,
  130,  131,  204,  209,  205,  216,  217,  218,  186,   92,
   81,  107,  124,  125,  214,  211,  128,   43,   44,   45,
   46,   47,   83,   97,   98,  113,   69,   79,   70,  194,
  195,  108,   63,   64,  193,   65,   66,  117,  121,  135,
  144,  169,  215,  118,   76,   48,   49,   50,  105,  136,
  137,  138,   51,   52,   54,   55,   57,   58,   56,  142,
   59,   60,   61,   67,   86,  122,  100,  143,   93,  165,
  101,  102,  119,  120,  212,  106,   62,   84,   87,   68,
   71,  109,  110,   85,  166,  111,   72,   73,   74,  203,
  123,  179,  180,  181,  182,  183,  184,  192,  112,   80,
  114,  115,  116,  167,   77,   78,   99,   88,   89,  104,
  132,  133,  213,  134,  139,  140,  141,  170,  171,  173,
  175,  176,  174,  177,  187,  145,  146,  149,  150,  147,
  148,  151,  152,  154,  153,  206,  208,  207,  168,  178,
  196,  198,  197,  199,  200,  201,  222,  223,  224,  226,
  227,  228,  225,  230,  231,  232,  233,  234,  235,  245,
  246,  247,  248,  252,  249,  250,  251,  237,  238,  241,
  242,  239,  243,  240,   10,   11,   11,  259,  259,  259,
  259,  259,  259,  259,  259,  260,  262,  261,  267,  263,
  264,  265,  266,   12,   13,   13,  268,  268,  268,  268,
  268,  268,  268,  268,  268,  268,  268,  268,  268,  268,
  268,  268,  268,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
  285,  286,    8,    9,    9,  287,  288,  172,  185,  188,
  189,   16,   17,   17,  289,  289,  289,  289,  289,  289,
  289,  289,  289,  289,  290,  291,  292,  294,  295,  293,
  296,  297,  298,  299,   18,   19,   19,  300,  300,  300,
  300,  300,  301,  302,  303,  304,  305,  210,  219,  220,
   20,   21,   21,  306,  306,  307,  308,
};
static const YYINT yylen[] = {                            2,
    0,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    1,    2,    0,    1,    1,    1,
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
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    0,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    0,    1,    1,    1,    1,    1,    1,
    1,    2,    0,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    0,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    2,    2,    2,    2,    1,    2,    0,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    2,
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
    2,    2,    2,    2,    2,    2,    2,    2,    3,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    3,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    3,    3,
    4,    4,    4,    3,    3,    2,    2,    2,    2,    2,
    2,    3,    3,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    3,    3,
    3,    2,    2,    2,    1,    2,    0,    1,    1,    1,
    1,    1,    1,    1,    1,    2,    2,    2,    2,    2,
    2,    2,    2,    1,    2,    0,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    1,    2,    0,    1,    2,    2,    2,    3,
    3,    1,    2,    0,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    1,    2,    0,    1,    1,    1,
    1,    1,    2,    2,    2,    2,    2,    3,    2,    2,
    1,    2,    0,    1,    1,    2,    2,
};
static const YYINT yydefred[] = {                         1,
    0,   15,  212,  222,  485,  543,  504,  231,  552,  575,
  241,  591,  257,    2,   17,  214,  224,  545,  487,  506,
  233,  554,  577,  593,  243,  259,    0,    0,    0,    0,
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
    0,    0,   16,   18,   19,   20,   21,   22,   23,   24,
   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   47,   48,   49,   50,   51,   52,   53,   54,
   55,   56,   57,   58,   59,   60,   61,   62,   63,   64,
   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,
   75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
   95,   96,   97,   98,   99,  100,  101,  102,  103,  104,
  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,
  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,
  125,  126,  127,  128,  129,  130,  131,  132,  133,  134,
  135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
  145,  146,  147,  148,  149,  150,  151,  152,  153,  154,
  155,  156,  157,  158,  159,  160,  161,  162,  163,  164,
  165,  166,  167,  168,  169,  170,  171,  172,  173,  174,
  175,  176,  177,  178,  179,  180,  181,  182,  183,  184,
  185,  186,  187,  188,  189,  190,  191,  192,  193,  194,
  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,
  205,  206,  207,  208,  209,  210,  211,    0,    0,    0,
    0,    0,    0,    0,  213,  215,  216,  217,  218,  219,
  220,  221,    0,    0,    0,    0,    0,    0,  223,  225,
  226,  227,  228,  229,  230,    0,  544,  546,    0,    0,
    0,    0,    0,    0,    0,    0,  486,  488,  489,  490,
  491,  492,  493,  494,  495,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  505,  507,  508,  509,  510,  511,  512,
  513,  514,  515,  516,  517,  518,  519,  520,  521,  522,
  523,  524,    0,    0,    0,    0,    0,    0,    0,  232,
  234,  235,  236,  237,  238,  239,  240,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  553,  555,  556,
  557,  558,  559,  560,  561,  562,  563,  564,    0,    0,
    0,    0,    0,  576,  578,  579,  580,  581,  582,    0,
    0,  592,  594,  595,    0,    0,    0,    0,    0,    0,
    0,    0,  242,  244,  245,  246,  247,  248,  249,  250,
  251,    0,    0,    0,    0,    0,  260,  261,  263,  264,
  265,  262,  266,  267,  268,  269,  258,  271,  270,  277,
  290,  288,  300,  296,  297,  301,  298,  299,  302,  303,
  304,  305,  306,  328,  329,  330,  331,  332,  356,  357,
  358,  363,  364,  293,  365,  366,  369,  367,  368,  371,
  372,  373,  387,  343,  344,  346,  347,  374,  390,  337,
  339,  391,  397,  398,  399,  294,  355,  415,  416,  338,
  410,  321,  289,  333,  388,  394,  375,    0,    0,  419,
  295,  272,  320,  379,  273,  291,  292,  334,  335,  417,
  377,  381,  382,  274,  420,  359,  386,  322,  342,  392,
  393,  396,  409,  336,  413,  411,  412,  348,  354,  383,
  384,  349,  350,  376,  401,  323,  324,  327,  307,  309,
  310,  311,  312,  313,  421,  422,  424,  360,  361,  362,
  370,  425,  426,  427,    0,    0,    0,  378,  351,  548,
  436,  440,  438,  437,  441,  439,    0,    0,  444,  445,
  278,  279,  280,  281,  282,  283,  284,  285,  286,  287,
  380,  395,  414,  449,  450,  352,  428,    0,    0,    0,
    0,    0,    0,  402,  403,  404,  405,  406,  407,  408,
  549,  345,  340,  400,  319,  275,  276,  341,  451,  453,
  452,  454,  455,  456,  308,  315,  446,  448,  447,  314,
    0,  589,  590,  326,  385,  423,  325,  353,  316,  317,
  318,  457,  458,  459,  463,  462,  460,  461,  464,  465,
  466,  467,  469,  468,  547,  496,  498,  497,  500,  501,
  502,  503,  499,  525,  526,  527,  528,  529,  530,  531,
  532,  533,  534,  535,  536,  537,  538,  539,  540,  541,
  542,  478,    0,  482,  483,    0,    0,  484,  565,  566,
  567,  570,  568,  569,  571,  572,  573,  574,  583,  584,
  585,  586,  587,  596,  597,  470,  476,  471,  472,  473,
  475,  477,  474,  252,  253,  254,  255,  256,  389,  418,
  435,  550,  551,  442,  443,  429,  430,    0,    0,    0,
  434,  588,  479,  480,  481,  433,  431,  432,
};
static const YYINT yydgoto[] = {                          1,
   14,   15,   27,   16,   28,   17,   29,   18,   30,   19,
   31,   20,   32,   21,   33,   22,   34,   23,   35,   24,
   36,   25,   37,   26,   38,  233,  234,  235,  236,  237,
  238,  239,  240,  241,  242,  243,  244,  245,  246,  247,
  248,  249,  250,  251,  252,  253,  254,  255,  256,  257,
  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,  294,  295,  296,  297,
  298,  299,  300,  301,  302,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  420,  421,  422,  423,  424,  425,  426,  427,
  435,  436,  437,  438,  439,  440,  441,  442,  449,  450,
  451,  452,  453,  454,  455,  520,  521,  522,  523,  524,
  525,  526,  527,  573,  574,  575,  576,  577,  578,  579,
  580,  581,  592,  593,  594,  595,  596,  597,  467,  468,
  469,  470,  471,  472,  473,  474,  475,  494,  495,  496,
  497,  498,  499,  500,  501,  502,  503,  504,  505,  506,
  507,  508,  509,  510,  511,  512,  457,  458,  538,  539,
  540,  541,  542,  543,  544,  545,  546,  547,  548,  554,
  555,  556,  557,  558,  559,  562,  563,  564,
};
static const YYINT yysindex[] = {                         0,
 -260,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  445, -281, -270, -366,
 -269, -361, -292, -414, -470, -525, -276, -289, -254, -253,
 -252, -246, -243, -241, -240, -238, -237, -230, -229, -226,
 -224, -223, -220, -200, -199, -198, -186, -185, -174, -173,
 -171, -170, -169, -168, -167, -166, -165, -164, -163, -162,
 -161, -160, -158, -157, -156, -155, -154, -152, -151, -149,
 -148, -147, -146, -145, -144, -142, -139, -138, -137, -136,
 -135, -134, -133, -132, -131, -128, -127, -126, -125, -124,
 -123, -122, -121, -120, -119, -117, -116, -115, -114, -113,
 -112, -111, -110, -109, -108, -107, -106, -105, -104, -103,
 -102, -101, -100,  -99,  -98,  -97,  -96,  -95,  -94,  -93,
  -92,  -90,  -88,  -87,  -86,  -85,  -84,  -83,  -82,  -81,
  -80,  -79,  -78,  -77,  -76,  -75,  -74,  -73,  -72,  -71,
  -70,  -69,  -68,  -67,  -66,  -65,  -64,  -63,  -62,  -60,
  -59,  -58,  -57,  -56,  -55,  -54,  -53,  -52,  -51,  -50,
  -49,  -48,  -47,  -42,  -41,  -40,  -39,  -38,  -37,  -34,
  -28,  -27,  -25,  -24,  -21,  -20,  -19,  -11,  -10,   -9,
   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1,    6,    8,
    9,   10,   12,   18,   19,   20,   21,   22,   23,   24,
   27,   28,   29,   30,   31,   32,   33,   34,   35,   40,
   41,   46,   47,   52,   53,   58,   59,   61,   62,   63,
   64,   65,    0,    0,    0,    0,    0,    0,    0,    0,
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
    0,    0,    0,    0,    0,    0,    0,   67,   68,   69,
   70,   71,   72,   73,    0,    0,    0,    0,    0,    0,
    0,    0,   74,   75,   76,   77,   78,   79,    0,    0,
    0,    0,    0,    0,    0,   80,    0,    0,   81,   82,
   83,   84,   85,   90,   91,   94,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   95,   96,   97,  106,  107,
  108,  109,  111,  112,  113,  118,  119,  120,  121,  122,
  123,  128,  129,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  130,  131,  132,  134,  135,  136,  137,    0,
    0,    0,    0,    0,    0,    0,    0,  138,  139,  140,
  141,  142,  147,  148,  149,  150,  151,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  152,  153,
  154,  160,  161,    0,    0,    0,    0,    0,    0,  162,
  163,    0,    0,    0,  164,  166,  168,  169,  170,  171,
  172,  173,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  174,  178,  179,  180,  181,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  182,  183,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  184,  185,  186,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  187,  188,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  189,  190,  191,
  192,  194,  195,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  196,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  197,    0,    0,  198,  199,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  204,  205,  206,
    0,    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    1,    2,    3,    4,
   13,   14,   15,   16,   25,   92,  124,  175,    0,    0,
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
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
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
    0,    0,    0,    0,  226,  227,  239,  251,    0,    0,
    0,  318,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 979
static const YYINT yytable[] = {                        456,
    3,    4,    5,    6,    2,  513,  560,  561,  565,  598,
  599,  600,    7,    8,    9,   10,  428,  601,  429,  430,
  602,  565,  603,  604,   11,  605,  606,  443,  549,  550,
  551,  552,  553,  607,  608,  444,  445,  609,    3,  610,
  611,  514,  515,  612,    4,  476,  477,  478,  479,  480,
  481,  482,  483,  484,  485,  486,  487,  488,  489,  490,
  491,  492,  493,  613,  614,  615,  516,  528,  529,  530,
  531,  532,  533,  534,  535,  536,  537,  616,  617,  431,
  459,  460,  461,  462,  463,  464,  465,  466,    5,  618,
  619,   12,  620,  621,  622,  623,  624,  625,  626,  627,
  628,  629,  630,  631,    6,  632,  633,  634,  635,  636,
  432,  637,  638,  433,  639,  640,  641,  642,  643,  644,
  446,  645,  447,   13,  646,  647,  648,  649,  650,  651,
  652,  653,  654,  517,  518,  655,  656,  657,  658,  659,
  660,  661,  662,  663,  664,    7,  665,  666,  667,  668,
  669,  670,  671,  672,  673,  674,  675,  676,  677,  678,
  679,  680,  681,  682,  683,  684,  685,  686,  687,  688,
  689,  690,  519,  691,   14,  692,  693,  694,  695,  696,
  697,  698,  699,  700,  701,  702,  703,  704,  705,  706,
  707,  708,  709,  710,  711,  712,  713,  714,  715,  716,
  717,  718,    8,  719,  720,  721,  722,  723,  724,  725,
  726,  727,  728,  729,  730,  731,  732,  567,  568,  569,
    9,  733,  734,  735,  736,  737,  738,  572,  566,  739,
  567,  568,  569,  570,  571,  740,  741,   10,  742,  743,
  572,  434,  744,  745,  746,   11,  582,  583,  584,  585,
  586,  448,  747,  748,  749,  750,  751,  752,  753,  754,
  755,  756,  757,  587,  588,    3,    4,    5,    6,  758,
   12,  759,  760,  761,   13,  762,  589,    7,    8,    9,
   10,  763,  764,  765,  766,  767,  768,  769,  590,   11,
  770,  771,  772,  773,  774,  775,  776,  777,  778,    3,
    4,    5,    6,  779,  780,    3,    4,    5,    6,  781,
  782,    7,    8,    9,   10,  783,  784,    7,    8,    9,
   10,  785,  786,   11,  787,  788,  789,  790,  791,   11,
  792,  793,  794,  795,  796,  797,  798,  799,  800,  801,
  802,  803,  804,  805,  806,  807,  808,  809,  810,    3,
    4,    5,    6,  811,  812,  591,   12,  813,  814,  815,
  816,    7,    8,    9,   10,    3,    4,    5,    6,  817,
  818,  819,  820,   11,  821,  822,  823,    7,    8,    9,
   10,  824,  825,  826,  827,  828,  829,    0,   13,   11,
   12,  830,  831,  832,  833,  834,   12,  835,  836,  837,
  838,  839,  840,  841,  842,  843,    3,    4,    5,    6,
  844,  845,  846,  847,  848,  849,  850,  851,    7,    8,
    9,   10,   13,  852,  853,  854,  855,  856,   13,  857,
   11,  858,  859,  860,  861,  862,  863,  864,    0,   14,
   12,  865,  866,  867,  868,  869,  870,  871,  872,  873,
  874,  875,  876,  877,  878,  879,   12,  880,  881,  882,
  883,  884,  885,    3,    4,    5,    6,  886,  887,  888,
    0,    0,   13,   14,    0,    7,    8,    9,   10,   14,
    0,    3,    4,    5,    6,    0,    0,   11,   13,    0,
    0,    0,    0,    7,    8,    9,   10,   12,    3,    4,
    5,    6,    0,    0,    0,   11,    3,    4,    5,    6,
    7,    8,    9,   10,    0,    0,    0,    0,    7,    8,
    9,   10,   11,   14,    0,    0,    0,    0,    0,   13,
   11,    3,    4,    5,    6,    3,    4,    5,    6,   14,
    0,    0,    0,    7,    8,    9,   10,    7,    8,    9,
   10,    0,    0,    0,   12,   11,    0,    0,    0,   11,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   12,    0,    0,    0,    0,    0,    0,    0,
   14,    0,    0,    0,    0,    0,   13,    0,    0,   12,
    0,    0,    0,    0,    0,    0,    0,   12,    0,    0,
    0,    0,    0,    0,   13,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   13,   12,    0,    0,    0,   12,    0,    0,   13,
    0,    0,    0,    0,    0,    0,    0,   14,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   13,   14,    0,    0,   13,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   14,    0,    0,    0,    0,    0,    0,    0,
   14,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   14,    0,    0,    0,   14,
   39,   40,   41,   42,   43,   44,   45,   46,   47,   48,
   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,
   59,   60,   61,   62,   63,   64,   65,   66,   67,   68,
   69,   70,    0,    0,    0,    0,   71,   72,   73,    0,
    0,    0,   74,   75,   76,   77,   78,   79,   80,   81,
   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,
   92,   93,   94,   95,   96,   97,   98,   99,  100,  101,
  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
  112,  113,  114,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  115,  116,  117,    0,  118,  119,  120,    0,
    0,  121,  122,  123,  124,  125,  126,  127,  128,  129,
  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
  140,  141,  142,  143,  144,    0,    0,    0,  145,    0,
  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  156,
  157,  158,  159,  160,  161,  162,  163,  164,  165,  166,
  167,  168,  169,  170,  171,  172,  173,  174,  175,  176,
  177,  178,  179,  180,  181,  182,  183,  184,  185,  186,
  187,  188,  189,  190,  191,  192,  193,    0,  194,    0,
  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,
  205,  206,  207,  208,  209,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  210,  211,  212,  213,
  214,  215,    0,    0,    0,    0,    0,    0,  216,    0,
    0,    0,    0,    0,    0,    0,  217,  218,    0,  219,
  220,    0,  221,  222,  223,  224,    0,    0,  225,  226,
  227,  228,  229,  230,  231,    0,    0,    0,  232,
};
static const YYINT yycheck[] = {                        366,
    0,    0,    0,    0,  265,  298,  532,  533,  298,  264,
  264,  264,    0,    0,    0,    0,  298,  264,  300,  301,
  264,  298,  264,  264,    0,  264,  264,  298,  499,  500,
  501,  502,  503,  264,  264,  306,  307,  264,  299,  264,
  264,  334,  335,  264,  305,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  420,  421,
  422,  423,  424,  264,  264,  264,  359,  482,  483,  484,
  485,  486,  487,  488,  489,  490,  491,  264,  264,  361,
  350,  351,  352,  353,  354,  355,  356,  357,  349,  264,
  264,    0,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  365,  264,  264,  264,  264,  264,
  392,  264,  264,  395,  264,  264,  264,  264,  264,  264,
  391,  264,  393,    0,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  426,  427,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  406,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  465,  264,    0,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  463,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  507,  508,  509,
  481,  264,  264,  264,  264,  264,  264,  517,  505,  264,
  507,  508,  509,  510,  511,  264,  264,  498,  264,  264,
  517,  523,  264,  264,  264,  506,  536,  537,  538,  539,
  540,  522,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,   38,   38,  265,  265,  265,  265,  264,
  531,  264,  264,  264,  535,  264,   38,  265,  265,  265,
  265,  264,  264,  264,  264,  264,  264,  264,   38,  265,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  299,
  299,  299,  299,  264,  264,  305,  305,  305,  305,  264,
  264,  299,  299,  299,  299,  264,  264,  305,  305,  305,
  305,  264,  264,  299,  264,  264,  264,  264,  264,  305,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  349,
  349,  349,  349,  264,  264,   38,  265,  264,  264,  264,
  264,  349,  349,  349,  349,  365,  365,  365,  365,  264,
  264,  264,  264,  349,  264,  264,  264,  365,  365,  365,
  365,  264,  264,  264,  264,  264,  264,   -1,  265,  365,
  299,  264,  264,  264,  264,  264,  305,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  406,  406,  406,  406,
  264,  264,  264,  264,  264,  264,  264,  264,  406,  406,
  406,  406,  299,  264,  264,  264,  264,  264,  305,  264,
  406,  264,  264,  264,  264,  264,  264,  264,   -1,  265,
  349,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  365,  264,  264,  264,
  264,  264,  264,  463,  463,  463,  463,  264,  264,  264,
   -1,   -1,  349,  299,   -1,  463,  463,  463,  463,  305,
   -1,  481,  481,  481,  481,   -1,   -1,  463,  365,   -1,
   -1,   -1,   -1,  481,  481,  481,  481,  406,  498,  498,
  498,  498,   -1,   -1,   -1,  481,  506,  506,  506,  506,
  498,  498,  498,  498,   -1,   -1,   -1,   -1,  506,  506,
  506,  506,  498,  349,   -1,   -1,   -1,   -1,   -1,  406,
  506,  531,  531,  531,  531,  535,  535,  535,  535,  365,
   -1,   -1,   -1,  531,  531,  531,  531,  535,  535,  535,
  535,   -1,   -1,   -1,  463,  531,   -1,   -1,   -1,  535,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  481,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  406,   -1,   -1,   -1,   -1,   -1,  463,   -1,   -1,  498,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  506,   -1,   -1,
   -1,   -1,   -1,   -1,  481,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  498,  531,   -1,   -1,   -1,  535,   -1,   -1,  506,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  463,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  531,  481,   -1,   -1,  535,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  498,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  506,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  531,   -1,   -1,   -1,  535,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  289,  290,  291,  292,  293,  294,  295,
  296,  297,   -1,   -1,   -1,   -1,  302,  303,  304,   -1,
   -1,   -1,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  358,  359,  360,   -1,  362,  363,  364,   -1,
   -1,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,   -1,   -1,   -1,  394,   -1,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  425,
  426,  427,  428,  429,  430,  431,  432,  433,  434,  435,
  436,  437,  438,  439,  440,  441,  442,  443,  444,  445,
  446,  447,  448,  449,  450,  451,  452,  453,  454,  455,
  456,  457,  458,  459,  460,  461,  462,   -1,  464,   -1,
  466,  467,  468,  469,  470,  471,  472,  473,  474,  475,
  476,  477,  478,  479,  480,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  492,  493,  494,  495,
  496,  497,   -1,   -1,   -1,   -1,   -1,   -1,  504,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  512,  513,   -1,  515,
  516,   -1,  518,  519,  520,  521,   -1,   -1,  524,  525,
  526,  527,  528,  529,  530,   -1,   -1,   -1,  534,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 540
#define YYUNDFTOKEN 851
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
"VAR_NUM_THREADS","VAR_PORT","VAR_OUTGOING_RANGE","VAR_INTERFACE",
"VAR_PREFER_IP4","VAR_DO_IP4","VAR_DO_IP6","VAR_PREFER_IP6","VAR_DO_UDP",
"VAR_DO_TCP","VAR_TCP_MSS","VAR_OUTGOING_TCP_MSS","VAR_TCP_IDLE_TIMEOUT",
"VAR_EDNS_TCP_KEEPALIVE","VAR_EDNS_TCP_KEEPALIVE_TIMEOUT","VAR_CHROOT",
"VAR_USERNAME","VAR_DIRECTORY","VAR_LOGFILE","VAR_PIDFILE","VAR_MSG_CACHE_SIZE",
"VAR_MSG_CACHE_SLABS","VAR_NUM_QUERIES_PER_THREAD","VAR_RRSET_CACHE_SIZE",
"VAR_RRSET_CACHE_SLABS","VAR_OUTGOING_NUM_TCP","VAR_INFRA_HOST_TTL",
"VAR_INFRA_LAME_TTL","VAR_INFRA_CACHE_SLABS","VAR_INFRA_CACHE_NUMHOSTS",
"VAR_INFRA_CACHE_LAME_SIZE","VAR_NAME","VAR_STUB_ZONE","VAR_STUB_HOST",
"VAR_STUB_ADDR","VAR_TARGET_FETCH_POLICY","VAR_HARDEN_SHORT_BUFSIZE",
"VAR_HARDEN_LARGE_QUERIES","VAR_FORWARD_ZONE","VAR_FORWARD_HOST",
"VAR_FORWARD_ADDR","VAR_DO_NOT_QUERY_ADDRESS","VAR_HIDE_IDENTITY",
"VAR_HIDE_VERSION","VAR_IDENTITY","VAR_VERSION","VAR_HARDEN_GLUE",
"VAR_MODULE_CONF","VAR_TRUST_ANCHOR_FILE","VAR_TRUST_ANCHOR",
"VAR_VAL_OVERRIDE_DATE","VAR_BOGUS_TTL","VAR_VAL_CLEAN_ADDITIONAL",
"VAR_VAL_PERMISSIVE_MODE","VAR_INCOMING_NUM_TCP","VAR_MSG_BUFFER_SIZE",
"VAR_KEY_CACHE_SIZE","VAR_KEY_CACHE_SLABS","VAR_TRUSTED_KEYS_FILE",
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
"VAR_DNSTAP_ENABLE","VAR_DNSTAP_SOCKET_PATH","VAR_DNSTAP_IP","VAR_DNSTAP_TLS",
"VAR_DNSTAP_TLS_SERVER_NAME","VAR_DNSTAP_TLS_CERT_BUNDLE",
"VAR_DNSTAP_TLS_CLIENT_KEY_FILE","VAR_DNSTAP_TLS_CLIENT_CERT_FILE",
"VAR_DNSTAP_SEND_IDENTITY","VAR_DNSTAP_SEND_VERSION","VAR_DNSTAP_IDENTITY",
"VAR_DNSTAP_VERSION","VAR_DNSTAP_LOG_RESOLVER_QUERY_MESSAGES",
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
"VAR_SERVE_EXPIRED_TTL_RESET","VAR_SERVE_EXPIRED_REPLY_TTL",
"VAR_SERVE_EXPIRED_CLIENT_TIMEOUT","VAR_FAKE_DSA","VAR_FAKE_SHA1",
"VAR_LOG_IDENTITY","VAR_HIDE_TRUSTANCHOR","VAR_TRUST_ANCHOR_SIGNALING",
"VAR_AGGRESSIVE_NSEC","VAR_USE_SYSTEMD","VAR_SHM_ENABLE","VAR_SHM_KEY",
"VAR_ROOT_KEY_SENTINEL","VAR_DNSCRYPT","VAR_DNSCRYPT_ENABLE",
"VAR_DNSCRYPT_PORT","VAR_DNSCRYPT_PROVIDER","VAR_DNSCRYPT_SECRET_KEY",
"VAR_DNSCRYPT_PROVIDER_CERT","VAR_DNSCRYPT_PROVIDER_CERT_ROTATED",
"VAR_DNSCRYPT_SHARED_SECRET_CACHE_SIZE",
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
"VAR_TLS_CIPHERS","VAR_TLS_CIPHERSUITES","VAR_IPSET","VAR_IPSET_NAME_V4",
"VAR_IPSET_NAME_V6","VAR_TLS_SESSION_TICKET_KEYS","VAR_RPZ","VAR_TAGS",
"VAR_RPZ_ACTION_OVERRIDE","VAR_RPZ_CNAME_OVERRIDE","VAR_RPZ_LOG",
"VAR_RPZ_LOG_NAME",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
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
"toplevelvar : rpzstart contents_rpz",
"serverstart : VAR_SERVER",
"contents_server : contents_server content_server",
"contents_server :",
"content_server : server_num_threads",
"content_server : server_verbosity",
"content_server : server_port",
"content_server : server_outgoing_range",
"content_server : server_do_ip4",
"content_server : server_do_ip6",
"content_server : server_prefer_ip4",
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
"content_server : server_serve_expired_reply_ttl",
"content_server : server_serve_expired_client_timeout",
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
"rpz_tag : VAR_TAGS STRING_ARG",
"rpz_action_override : VAR_RPZ_ACTION_OVERRIDE STRING_ARG",
"rpz_cname_override : VAR_RPZ_CNAME_OVERRIDE STRING_ARG",
"rpz_log : VAR_RPZ_LOG STRING_ARG",
"rpz_log_name : VAR_RPZ_LOG_NAME STRING_ARG",
"rpzstart : VAR_RPZ",
"contents_rpz : contents_rpz content_rpz",
"contents_rpz :",
"content_rpz : auth_name",
"content_rpz : auth_zonefile",
"content_rpz : rpz_tag",
"content_rpz : auth_master",
"content_rpz : auth_url",
"content_rpz : auth_allow_notify",
"content_rpz : rpz_action_override",
"content_rpz : rpz_cname_override",
"content_rpz : rpz_log",
"content_rpz : rpz_log_name",
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
"server_prefer_ip4 : VAR_PREFER_IP4 STRING_ARG",
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
"server_serve_expired_reply_ttl : VAR_SERVE_EXPIRED_REPLY_TTL STRING_ARG",
"server_serve_expired_client_timeout : VAR_SERVE_EXPIRED_CLIENT_TIMEOUT STRING_ARG",
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
"content_dt : dt_dnstap_ip",
"content_dt : dt_dnstap_tls",
"content_dt : dt_dnstap_tls_server_name",
"content_dt : dt_dnstap_tls_cert_bundle",
"content_dt : dt_dnstap_tls_client_key_file",
"content_dt : dt_dnstap_tls_client_cert_file",
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
"dt_dnstap_ip : VAR_DNSTAP_IP STRING_ARG",
"dt_dnstap_tls : VAR_DNSTAP_TLS STRING_ARG",
"dt_dnstap_tls_server_name : VAR_DNSTAP_TLS_SERVER_NAME STRING_ARG",
"dt_dnstap_tls_cert_bundle : VAR_DNSTAP_TLS_CERT_BUNDLE STRING_ARG",
"dt_dnstap_tls_client_key_file : VAR_DNSTAP_TLS_CLIENT_KEY_FILE STRING_ARG",
"dt_dnstap_tls_client_cert_file : VAR_DNSTAP_TLS_CLIENT_CERT_FILE STRING_ARG",
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
#line 3211 "./util/configparser.y"

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


#line 1821 "util/configparser.c"

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
case 15:
#line 191 "./util/configparser.y"
	{ 
		OUTYY(("\nP(server:)\n")); 
	}
break;
case 212:
#line 284 "./util/configparser.y"
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
case 222:
#line 301 "./util/configparser.y"
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
case 231:
#line 318 "./util/configparser.y"
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
case 241:
#line 337 "./util/configparser.y"
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
break;
case 252:
#line 361 "./util/configparser.y"
	{
		uint8_t* bitlist;
		size_t len = 0;
		OUTYY(("P(server_local_zone_tag:%s)\n", yystack.l_mark[0].str));
		bitlist = config_parse_taglist(cfg_parser->cfg, yystack.l_mark[0].str,
			&len);
		free(yystack.l_mark[0].str);
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
		}
		if(bitlist) {
			cfg_parser->cfg->auths->rpz_taglist = bitlist;
			cfg_parser->cfg->auths->rpz_taglistlen = len;

		}
	}
break;
case 253:
#line 380 "./util/configparser.y"
	{
		OUTYY(("P(rpz_action_override:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "nxdomain")!=0 && strcmp(yystack.l_mark[0].str, "nodata")!=0 &&
		   strcmp(yystack.l_mark[0].str, "passthru")!=0 && strcmp(yystack.l_mark[0].str, "drop")!=0 &&
		   strcmp(yystack.l_mark[0].str, "cname")!=0 && strcmp(yystack.l_mark[0].str, "disabled")!=0) {
			yyerror("rpz-action-override action: expected nxdomain, "
				"nodata, passthru, drop, cname or disabled");
			free(yystack.l_mark[0].str);
			cfg_parser->cfg->auths->rpz_action_override = NULL;
		}
		else {
			cfg_parser->cfg->auths->rpz_action_override = yystack.l_mark[0].str;
		}
	}
break;
case 254:
#line 397 "./util/configparser.y"
	{
		OUTYY(("P(rpz_cname_override:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->auths->rpz_cname);
		cfg_parser->cfg->auths->rpz_cname = yystack.l_mark[0].str;
	}
break;
case 255:
#line 405 "./util/configparser.y"
	{
		OUTYY(("P(rpz_log:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->rpz_log = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 256:
#line 415 "./util/configparser.y"
	{
		OUTYY(("P(rpz_log_name:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->auths->rpz_log_name);
		cfg_parser->cfg->auths->rpz_log_name = yystack.l_mark[0].str;
	}
break;
case 257:
#line 423 "./util/configparser.y"
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
break;
case 270:
#line 446 "./util/configparser.y"
	{ 
		OUTYY(("P(server_num_threads:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 271:
#line 455 "./util/configparser.y"
	{ 
		OUTYY(("P(server_verbosity:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 272:
#line 464 "./util/configparser.y"
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
case 273:
#line 475 "./util/configparser.y"
	{
		OUTYY(("P(server_statistics_cumulative:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 274:
#line 484 "./util/configparser.y"
	{
		OUTYY(("P(server_extended_statistics:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 275:
#line 493 "./util/configparser.y"
	{
		OUTYY(("P(server_shm_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->shm_enable = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 276:
#line 502 "./util/configparser.y"
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
case 277:
#line 513 "./util/configparser.y"
	{
		OUTYY(("P(server_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 278:
#line 522 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_send_client_subnet:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet, yystack.l_mark[0].str))
			fatal_exit("out of memory adding client-subnet");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 279:
#line 534 "./util/configparser.y"
	{
	#ifdef CLIENT_SUBNET
		OUTYY(("P(server_client_subnet_zone:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->client_subnet_zone,
			yystack.l_mark[0].str))
			fatal_exit("out of memory adding client-subnet-zone");
	#else
		OUTYY(("P(Compiled without edns subnet option, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 280:
#line 548 "./util/configparser.y"
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
case 281:
#line 563 "./util/configparser.y"
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
case 282:
#line 574 "./util/configparser.y"
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
case 283:
#line 591 "./util/configparser.y"
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
case 284:
#line 608 "./util/configparser.y"
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
case 285:
#line 625 "./util/configparser.y"
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
case 286:
#line 642 "./util/configparser.y"
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
case 287:
#line 657 "./util/configparser.y"
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
case 288:
#line 672 "./util/configparser.y"
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
case 289:
#line 685 "./util/configparser.y"
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
case 290:
#line 700 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_range:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 291:
#line 709 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_port_permit:%s)\n", yystack.l_mark[0].str));
		if(!cfg_mark_ports(yystack.l_mark[0].str, 1, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yystack.l_mark[0].str);
	}
break;
case 292:
#line 718 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", yystack.l_mark[0].str));
		if(!cfg_mark_ports(yystack.l_mark[0].str, 0, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yystack.l_mark[0].str);
	}
break;
case 293:
#line 727 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 294:
#line 736 "./util/configparser.y"
	{
		OUTYY(("P(server_incoming_num_tcp:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 295:
#line 745 "./util/configparser.y"
	{
		OUTYY(("P(server_interface_automatic:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 296:
#line 754 "./util/configparser.y"
	{
		OUTYY(("P(server_do_ip4:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 297:
#line 763 "./util/configparser.y"
	{
		OUTYY(("P(server_do_ip6:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 298:
#line 772 "./util/configparser.y"
	{
		OUTYY(("P(server_do_udp:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 299:
#line 781 "./util/configparser.y"
	{
		OUTYY(("P(server_do_tcp:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 300:
#line 790 "./util/configparser.y"
	{
		OUTYY(("P(server_prefer_ip4:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip4 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 301:
#line 799 "./util/configparser.y"
	{
		OUTYY(("P(server_prefer_ip6:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip6 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 302:
#line 808 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_mss:%s)\n", yystack.l_mark[0].str));
                if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
                        yyerror("number expected");
                else cfg_parser->cfg->tcp_mss = atoi(yystack.l_mark[0].str);
                free(yystack.l_mark[0].str);
	}
break;
case 303:
#line 817 "./util/configparser.y"
	{
		OUTYY(("P(server_outgoing_tcp_mss:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_tcp_mss = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 304:
#line 826 "./util/configparser.y"
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
case 305:
#line 839 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_keepalive:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp_keepalive = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 306:
#line 848 "./util/configparser.y"
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
case 307:
#line 861 "./util/configparser.y"
	{
		OUTYY(("P(server_tcp_upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tcp_upstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 308:
#line 870 "./util/configparser.y"
	{
		OUTYY(("P(server_udp_upstream_without_downstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_upstream_without_downstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 309:
#line 879 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ssl_upstream = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 310:
#line 888 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_service_key:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->ssl_service_key);
		cfg_parser->cfg->ssl_service_key = yystack.l_mark[0].str;
	}
break;
case 311:
#line 895 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_service_pem:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->ssl_service_pem);
		cfg_parser->cfg->ssl_service_pem = yystack.l_mark[0].str;
	}
break;
case 312:
#line 902 "./util/configparser.y"
	{
		OUTYY(("P(server_ssl_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->ssl_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 313:
#line 911 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_cert_bundle:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = yystack.l_mark[0].str;
	}
break;
case 314:
#line 918 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_win_cert:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_win_cert = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 315:
#line 927 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_additional_port:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->tls_additional_port,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 316:
#line 935 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_ciphers:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_ciphers);
		cfg_parser->cfg->tls_ciphers = yystack.l_mark[0].str;
	}
break;
case 317:
#line 942 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_ciphersuites:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->tls_ciphersuites);
		cfg_parser->cfg->tls_ciphersuites = yystack.l_mark[0].str;
	}
break;
case 318:
#line 949 "./util/configparser.y"
	{
		OUTYY(("P(server_tls_session_ticket_keys:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append(&cfg_parser->cfg->tls_session_ticket_keys,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 319:
#line 957 "./util/configparser.y"
	{
		OUTYY(("P(server_use_systemd:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_systemd = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 320:
#line 966 "./util/configparser.y"
	{
		OUTYY(("P(server_do_daemonize:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 321:
#line 975 "./util/configparser.y"
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
case 322:
#line 989 "./util/configparser.y"
	{
		OUTYY(("P(server_log_time_ascii:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 323:
#line 998 "./util/configparser.y"
	{
		OUTYY(("P(server_log_queries:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_queries = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 324:
#line 1007 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_replies:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_replies = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 325:
#line 1016 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_tag_queryreply:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_tag_queryreply = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 326:
#line 1025 "./util/configparser.y"
	{
		OUTYY(("P(server_log_servfail:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_servfail = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 327:
#line 1034 "./util/configparser.y"
	{
  	OUTYY(("P(server_log_local_actions:%s)\n", yystack.l_mark[0].str));
  	if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
  		yyerror("expected yes or no.");
  	else cfg_parser->cfg->log_local_actions = (strcmp(yystack.l_mark[0].str, "yes")==0);
  	free(yystack.l_mark[0].str);
  }
break;
case 328:
#line 1043 "./util/configparser.y"
	{
		OUTYY(("P(server_chroot:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = yystack.l_mark[0].str;
	}
break;
case 329:
#line 1050 "./util/configparser.y"
	{
		OUTYY(("P(server_username:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = yystack.l_mark[0].str;
	}
break;
case 330:
#line 1057 "./util/configparser.y"
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
case 331:
#line 1083 "./util/configparser.y"
	{
		OUTYY(("P(server_logfile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = yystack.l_mark[0].str;
		cfg_parser->cfg->use_syslog = 0;
	}
break;
case 332:
#line 1091 "./util/configparser.y"
	{
		OUTYY(("P(server_pidfile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = yystack.l_mark[0].str;
	}
break;
case 333:
#line 1098 "./util/configparser.y"
	{
		OUTYY(("P(server_root_hints:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 334:
#line 1105 "./util/configparser.y"
	{
		OUTYY(("P(server_dlv_anchor_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dlv_anchor_file);
		cfg_parser->cfg->dlv_anchor_file = yystack.l_mark[0].str;
	}
break;
case 335:
#line 1112 "./util/configparser.y"
	{
		OUTYY(("P(server_dlv_anchor:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dlv_anchor_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 336:
#line 1119 "./util/configparser.y"
	{
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 337:
#line 1127 "./util/configparser.y"
	{
		OUTYY(("P(server_trust_anchor_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 338:
#line 1135 "./util/configparser.y"
	{
		OUTYY(("P(server_trusted_keys_file:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 339:
#line 1143 "./util/configparser.y"
	{
		OUTYY(("P(server_trust_anchor:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 340:
#line 1150 "./util/configparser.y"
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
case 341:
#line 1161 "./util/configparser.y"
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
case 342:
#line 1172 "./util/configparser.y"
	{
		OUTYY(("P(server_domain_insecure:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 343:
#line 1179 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_identity:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 344:
#line 1188 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_version:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 345:
#line 1197 "./util/configparser.y"
	{
		OUTYY(("P(server_hide_trustanchor:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_trustanchor = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 346:
#line 1206 "./util/configparser.y"
	{
		OUTYY(("P(server_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = yystack.l_mark[0].str;
	}
break;
case 347:
#line 1213 "./util/configparser.y"
	{
		OUTYY(("P(server_version:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = yystack.l_mark[0].str;
	}
break;
case 348:
#line 1220 "./util/configparser.y"
	{
		OUTYY(("P(server_so_rcvbuf:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->so_rcvbuf))
			yyerror("buffer size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 349:
#line 1228 "./util/configparser.y"
	{
		OUTYY(("P(server_so_sndbuf:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->so_sndbuf))
			yyerror("buffer size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 350:
#line 1236 "./util/configparser.y"
	{
        OUTYY(("P(server_so_reuseport:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->so_reuseport =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 351:
#line 1246 "./util/configparser.y"
	{
        OUTYY(("P(server_ip_transparent:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_transparent =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 352:
#line 1256 "./util/configparser.y"
	{
        OUTYY(("P(server_ip_freebind:%s)\n", yystack.l_mark[0].str));
        if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
            yyerror("expected yes or no.");
        else cfg_parser->cfg->ip_freebind =
            (strcmp(yystack.l_mark[0].str, "yes")==0);
        free(yystack.l_mark[0].str);
    }
break;
case 353:
#line 1266 "./util/configparser.y"
	{
		OUTYY(("P(server_stream_wait_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->stream_wait_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 354:
#line 1274 "./util/configparser.y"
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
case 355:
#line 1287 "./util/configparser.y"
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
case 356:
#line 1298 "./util/configparser.y"
	{
		OUTYY(("P(server_msg_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 357:
#line 1306 "./util/configparser.y"
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
case 358:
#line 1319 "./util/configparser.y"
	{
		OUTYY(("P(server_num_queries_per_thread:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 359:
#line 1328 "./util/configparser.y"
	{
		OUTYY(("P(server_jostle_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 360:
#line 1337 "./util/configparser.y"
	{
		OUTYY(("P(server_delay_close:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->delay_close = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 361:
#line 1346 "./util/configparser.y"
	{
		OUTYY(("P(server_unblock_lan_zones:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->unblock_lan_zones = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 362:
#line 1356 "./util/configparser.y"
	{
		OUTYY(("P(server_insecure_lan_zones:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->insecure_lan_zones = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 363:
#line 1366 "./util/configparser.y"
	{
		OUTYY(("P(server_rrset_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 364:
#line 1374 "./util/configparser.y"
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
case 365:
#line 1387 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_host_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 366:
#line 1396 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_lame_ttl:%s)\n", yystack.l_mark[0].str));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 367:
#line 1404 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 368:
#line 1413 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", yystack.l_mark[0].str));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 369:
#line 1421 "./util/configparser.y"
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
case 370:
#line 1434 "./util/configparser.y"
	{
		OUTYY(("P(server_infra_cache_min_rtt:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_min_rtt = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 371:
#line 1443 "./util/configparser.y"
	{
		OUTYY(("P(server_target_fetch_policy:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = yystack.l_mark[0].str;
	}
break;
case 372:
#line 1450 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_short_bufsize:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 373:
#line 1460 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_large_queries:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 374:
#line 1470 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_glue:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 375:
#line 1480 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 376:
#line 1490 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_below_nxdomain:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_below_nxdomain = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 377:
#line 1500 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_referral_path:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 378:
#line 1510 "./util/configparser.y"
	{
		OUTYY(("P(server_harden_algo_downgrade:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_algo_downgrade = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 379:
#line 1520 "./util/configparser.y"
	{
		OUTYY(("P(server_use_caps_for_id:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 380:
#line 1530 "./util/configparser.y"
	{
		OUTYY(("P(server_caps_whitelist:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 381:
#line 1537 "./util/configparser.y"
	{
		OUTYY(("P(server_private_address:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 382:
#line 1544 "./util/configparser.y"
	{
		OUTYY(("P(server_private_domain:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 383:
#line 1551 "./util/configparser.y"
	{
		OUTYY(("P(server_prefetch:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 384:
#line 1560 "./util/configparser.y"
	{
		OUTYY(("P(server_prefetch_key:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch_key = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 385:
#line 1569 "./util/configparser.y"
	{
		OUTYY(("P(server_deny_any:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->deny_any = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 386:
#line 1578 "./util/configparser.y"
	{
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 387:
#line 1587 "./util/configparser.y"
	{
		OUTYY(("P(server_do_not_query_address:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 388:
#line 1594 "./util/configparser.y"
	{
		OUTYY(("P(server_do_not_query_localhost:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 389:
#line 1604 "./util/configparser.y"
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
case 390:
#line 1626 "./util/configparser.y"
	{
		OUTYY(("P(server_module_conf:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = yystack.l_mark[0].str;
	}
break;
case 391:
#line 1633 "./util/configparser.y"
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
case 392:
#line 1651 "./util/configparser.y"
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
case 393:
#line 1664 "./util/configparser.y"
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
case 394:
#line 1677 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_max_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 395:
#line 1686 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_max_negative_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_negative_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 396:
#line 1695 "./util/configparser.y"
	{
		OUTYY(("P(server_cache_min_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 397:
#line 1704 "./util/configparser.y"
	{
		OUTYY(("P(server_bogus_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 398:
#line 1713 "./util/configparser.y"
	{
		OUTYY(("P(server_val_clean_additional:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 399:
#line 1723 "./util/configparser.y"
	{
		OUTYY(("P(server_val_permissive_mode:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 400:
#line 1733 "./util/configparser.y"
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
case 401:
#line 1744 "./util/configparser.y"
	{
		OUTYY(("P(server_ignore_cd_flag:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ignore_cd = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 402:
#line 1753 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 403:
#line 1762 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 404:
#line 1771 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_ttl_reset:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired_ttl_reset = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 405:
#line 1780 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_reply_ttl:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_reply_ttl = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 406:
#line 1789 "./util/configparser.y"
	{
		OUTYY(("P(server_serve_expired_client_timeout:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_client_timeout = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 407:
#line 1798 "./util/configparser.y"
	{
		OUTYY(("P(server_fake_dsa:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_dsa = (strcmp(yystack.l_mark[0].str, "yes")==0);
		if(fake_dsa)
			log_warn("test option fake_dsa is enabled");
#endif
		free(yystack.l_mark[0].str);
	}
break;
case 408:
#line 1811 "./util/configparser.y"
	{
		OUTYY(("P(server_fake_sha1:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_sha1 = (strcmp(yystack.l_mark[0].str, "yes")==0);
		if(fake_sha1)
			log_warn("test option fake_sha1 is enabled");
#endif
		free(yystack.l_mark[0].str);
	}
break;
case 409:
#line 1824 "./util/configparser.y"
	{
		OUTYY(("P(server_val_log_level:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->val_log_level = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 410:
#line 1833 "./util/configparser.y"
	{
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = yystack.l_mark[0].str;
	}
break;
case 411:
#line 1840 "./util/configparser.y"
	{
		OUTYY(("P(server_add_holddown:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->add_holddown = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 412:
#line 1849 "./util/configparser.y"
	{
		OUTYY(("P(server_del_holddown:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->del_holddown = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 413:
#line 1858 "./util/configparser.y"
	{
		OUTYY(("P(server_keep_missing:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->keep_missing = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 414:
#line 1867 "./util/configparser.y"
	{
		OUTYY(("P(server_permit_small_holddown:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->permit_small_holddown =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 415:
#line 1876 "./util/configparser.y"
	{
		OUTYY(("P(server_key_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 416:
#line 1884 "./util/configparser.y"
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
case 417:
#line 1897 "./util/configparser.y"
	{
		OUTYY(("P(server_neg_cache_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 418:
#line 1905 "./util/configparser.y"
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
case 419:
#line 1946 "./util/configparser.y"
	{
		OUTYY(("P(server_local_data:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, yystack.l_mark[0].str))
			fatal_exit("out of memory adding local-data");
	}
break;
case 420:
#line 1953 "./util/configparser.y"
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
case 421:
#line 1968 "./util/configparser.y"
	{
		OUTYY(("P(server_minimal_responses:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->minimal_responses =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 422:
#line 1978 "./util/configparser.y"
	{
		OUTYY(("P(server_rrset_roundrobin:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->rrset_roundrobin =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 423:
#line 1988 "./util/configparser.y"
	{
		OUTYY(("P(server_unknown_server_time_limit:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->unknown_server_time_limit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 424:
#line 1995 "./util/configparser.y"
	{
		OUTYY(("P(server_max_udp_size:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->max_udp_size = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 425:
#line 2002 "./util/configparser.y"
	{
		OUTYY(("P(dns64_prefix:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dns64_prefix);
		cfg_parser->cfg->dns64_prefix = yystack.l_mark[0].str;
	}
break;
case 426:
#line 2009 "./util/configparser.y"
	{
		OUTYY(("P(server_dns64_synthall:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dns64_synthall = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 427:
#line 2018 "./util/configparser.y"
	{
		OUTYY(("P(dns64_ignore_aaaa:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dns64_ignore_aaaa,
			yystack.l_mark[0].str))
			fatal_exit("out of memory adding dns64-ignore-aaaa");
	}
break;
case 428:
#line 2026 "./util/configparser.y"
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
case 429:
#line 2040 "./util/configparser.y"
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
case 430:
#line 2061 "./util/configparser.y"
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
case 431:
#line 2082 "./util/configparser.y"
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
case 432:
#line 2094 "./util/configparser.y"
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
case 433:
#line 2106 "./util/configparser.y"
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
case 434:
#line 2118 "./util/configparser.y"
	{
		OUTYY(("P(server_access_control_view:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acl_view,
			yystack.l_mark[-1].str, yystack.l_mark[0].str)) {
			yyerror("out of memory");
		}
	}
break;
case 435:
#line 2127 "./util/configparser.y"
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
case 436:
#line 2148 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ip_ratelimit:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 437:
#line 2158 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ratelimit:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 438:
#line 2167 "./util/configparser.y"
	{
  	OUTYY(("P(server_ip_ratelimit_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->ip_ratelimit_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 439:
#line 2175 "./util/configparser.y"
	{
		OUTYY(("P(server_ratelimit_size:%s)\n", yystack.l_mark[0].str));
		if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->ratelimit_size))
			yyerror("memory size expected");
		free(yystack.l_mark[0].str);
	}
break;
case 440:
#line 2183 "./util/configparser.y"
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
case 441:
#line 2196 "./util/configparser.y"
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
case 442:
#line 2209 "./util/configparser.y"
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
case 443:
#line 2224 "./util/configparser.y"
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
case 444:
#line 2239 "./util/configparser.y"
	{ 
  	OUTYY(("P(server_ip_ratelimit_factor:%s)\n", yystack.l_mark[0].str)); 
  	if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
  		yyerror("number expected");
  	else cfg_parser->cfg->ip_ratelimit_factor = atoi(yystack.l_mark[0].str);
  	free(yystack.l_mark[0].str);
	}
break;
case 445:
#line 2248 "./util/configparser.y"
	{ 
		OUTYY(("P(server_ratelimit_factor:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit_factor = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 446:
#line 2257 "./util/configparser.y"
	{ 
		OUTYY(("P(low-rtt option is deprecated, use fast-server-num instead)\n"));
		free(yystack.l_mark[0].str);
	}
break;
case 447:
#line 2263 "./util/configparser.y"
	{ 
		OUTYY(("P(server_fast_server_num:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) <= 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_num = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 448:
#line 2272 "./util/configparser.y"
	{ 
		OUTYY(("P(server_fast_server_permil:%s)\n", yystack.l_mark[0].str)); 
		if(atoi(yystack.l_mark[0].str) == 0 && strcmp(yystack.l_mark[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_permil = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 449:
#line 2281 "./util/configparser.y"
	{
		OUTYY(("P(server_qname_minimisation:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 450:
#line 2291 "./util/configparser.y"
	{
		OUTYY(("P(server_qname_minimisation_strict:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation_strict = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 451:
#line 2301 "./util/configparser.y"
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
case 452:
#line 2314 "./util/configparser.y"
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
case 453:
#line 2327 "./util/configparser.y"
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
case 454:
#line 2339 "./util/configparser.y"
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
case 455:
#line 2353 "./util/configparser.y"
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
case 456:
#line 2365 "./util/configparser.y"
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
case 457:
#line 2379 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = yystack.l_mark[0].str;
	}
break;
case 458:
#line 2389 "./util/configparser.y"
	{
		OUTYY(("P(stub-host:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 459:
#line 2396 "./util/configparser.y"
	{
		OUTYY(("P(stub-addr:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 460:
#line 2403 "./util/configparser.y"
	{
		OUTYY(("P(stub-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 461:
#line 2412 "./util/configparser.y"
	{
		OUTYY(("P(stub-no-cache:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->no_cache=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 462:
#line 2421 "./util/configparser.y"
	{
		OUTYY(("P(stub-ssl-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->ssl_upstream = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 463:
#line 2431 "./util/configparser.y"
	{
		OUTYY(("P(stub-prime:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 464:
#line 2441 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = yystack.l_mark[0].str;
	}
break;
case 465:
#line 2451 "./util/configparser.y"
	{
		OUTYY(("P(forward-host:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 466:
#line 2458 "./util/configparser.y"
	{
		OUTYY(("P(forward-addr:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 467:
#line 2465 "./util/configparser.y"
	{
		OUTYY(("P(forward-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 468:
#line 2474 "./util/configparser.y"
	{
		OUTYY(("P(forward-no-cache:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->no_cache=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 469:
#line 2483 "./util/configparser.y"
	{
		OUTYY(("P(forward-ssl-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->ssl_upstream = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 470:
#line 2493 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = yystack.l_mark[0].str;
	}
break;
case 471:
#line 2503 "./util/configparser.y"
	{
		OUTYY(("P(zonefile:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = yystack.l_mark[0].str;
	}
break;
case 472:
#line 2510 "./util/configparser.y"
	{
		OUTYY(("P(master:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 473:
#line 2517 "./util/configparser.y"
	{
		OUTYY(("P(url:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 474:
#line 2524 "./util/configparser.y"
	{
		OUTYY(("P(allow-notify:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 475:
#line 2532 "./util/configparser.y"
	{
		OUTYY(("P(for-downstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_downstream =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 476:
#line 2542 "./util/configparser.y"
	{
		OUTYY(("P(for-upstream:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_upstream =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 477:
#line 2552 "./util/configparser.y"
	{
		OUTYY(("P(fallback-enabled:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->fallback_enabled =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 478:
#line 2562 "./util/configparser.y"
	{
		OUTYY(("P(name:%s)\n", yystack.l_mark[0].str));
		if(cfg_parser->cfg->views->name)
			yyerror("view name override, there must be one "
				"name for one view");
		free(cfg_parser->cfg->views->name);
		cfg_parser->cfg->views->name = yystack.l_mark[0].str;
	}
break;
case 479:
#line 2572 "./util/configparser.y"
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
case 480:
#line 2611 "./util/configparser.y"
	{
		OUTYY(("P(view_response_ip:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		validate_respip_action(yystack.l_mark[0].str);
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_actions, yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding per-view "
				"response-ip action");
	}
break;
case 481:
#line 2621 "./util/configparser.y"
	{
		OUTYY(("P(view_response_ip_data:%s)\n", yystack.l_mark[-1].str));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->views->respip_data, yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip-data");
	}
break;
case 482:
#line 2629 "./util/configparser.y"
	{
		OUTYY(("P(view_local_data:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->views->local_data, yystack.l_mark[0].str)) {
			fatal_exit("out of memory adding local-data");
		}
	}
break;
case 483:
#line 2637 "./util/configparser.y"
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
case 484:
#line 2652 "./util/configparser.y"
	{
		OUTYY(("P(view-first:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->views->isfirst=(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 485:
#line 2661 "./util/configparser.y"
	{ 
		OUTYY(("\nP(remote-control:)\n")); 
	}
break;
case 496:
#line 2672 "./util/configparser.y"
	{
		OUTYY(("P(control_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable = 
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 497:
#line 2682 "./util/configparser.y"
	{
		OUTYY(("P(control_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 498:
#line 2691 "./util/configparser.y"
	{
		OUTYY(("P(control_interface:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 499:
#line 2698 "./util/configparser.y"
	{
		OUTYY(("P(control_use_cert:%s)\n", yystack.l_mark[0].str));
		cfg_parser->cfg->control_use_cert = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 500:
#line 2705 "./util/configparser.y"
	{
		OUTYY(("P(rc_server_key_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = yystack.l_mark[0].str;
	}
break;
case 501:
#line 2712 "./util/configparser.y"
	{
		OUTYY(("P(rc_server_cert_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = yystack.l_mark[0].str;
	}
break;
case 502:
#line 2719 "./util/configparser.y"
	{
		OUTYY(("P(rc_control_key_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = yystack.l_mark[0].str;
	}
break;
case 503:
#line 2726 "./util/configparser.y"
	{
		OUTYY(("P(rc_control_cert_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = yystack.l_mark[0].str;
	}
break;
case 504:
#line 2733 "./util/configparser.y"
	{
		OUTYY(("\nP(dnstap:)\n"));
	}
break;
case 525:
#line 2753 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 526:
#line 2762 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_socket_path:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_socket_path);
		cfg_parser->cfg->dnstap_socket_path = yystack.l_mark[0].str;
	}
break;
case 527:
#line 2769 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_ip:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_ip);
		cfg_parser->cfg->dnstap_ip = yystack.l_mark[0].str;
	}
break;
case 528:
#line 2776 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_tls:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_tls = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 529:
#line 2785 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_tls_server_name:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_tls_server_name);
		cfg_parser->cfg->dnstap_tls_server_name = yystack.l_mark[0].str;
	}
break;
case 530:
#line 2792 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_tls_cert_bundle:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_tls_cert_bundle);
		cfg_parser->cfg->dnstap_tls_cert_bundle = yystack.l_mark[0].str;
	}
break;
case 531:
#line 2799 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_tls_client_key_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_tls_client_key_file);
		cfg_parser->cfg->dnstap_tls_client_key_file = yystack.l_mark[0].str;
	}
break;
case 532:
#line 2806 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_tls_client_cert_file:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_tls_client_cert_file);
		cfg_parser->cfg->dnstap_tls_client_cert_file = yystack.l_mark[0].str;
	}
break;
case 533:
#line 2813 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_send_identity:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_identity = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 534:
#line 2822 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_send_version:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_send_version = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 535:
#line 2831 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_identity);
		cfg_parser->cfg->dnstap_identity = yystack.l_mark[0].str;
	}
break;
case 536:
#line 2838 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_version:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnstap_version);
		cfg_parser->cfg->dnstap_version = yystack.l_mark[0].str;
	}
break;
case 537:
#line 2845 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_resolver_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 538:
#line 2855 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_resolver_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_resolver_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 539:
#line 2865 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_client_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 540:
#line 2875 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_client_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_client_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 541:
#line 2885 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_forwarder_query_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_query_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 542:
#line 2895 "./util/configparser.y"
	{
		OUTYY(("P(dt_dnstap_log_forwarder_response_messages:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnstap_log_forwarder_response_messages =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 543:
#line 2905 "./util/configparser.y"
	{ 
		OUTYY(("\nP(python:)\n")); 
	}
break;
case 547:
#line 2914 "./util/configparser.y"
	{
		OUTYY(("P(python-script:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_append_ex(&cfg_parser->cfg->python_script, yystack.l_mark[0].str))
			yyerror("out of memory");
	}
break;
case 548:
#line 2920 "./util/configparser.y"
	{
		OUTYY(("P(disable_dnssec_lame_check:%s)\n", yystack.l_mark[0].str));
		if (strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_dnssec_lame_check =
			(strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 549:
#line 2930 "./util/configparser.y"
	{
		OUTYY(("P(server_log_identity:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = yystack.l_mark[0].str;
	}
break;
case 550:
#line 2937 "./util/configparser.y"
	{
		OUTYY(("P(server_response_ip:%s %s)\n", yystack.l_mark[-1].str, yystack.l_mark[0].str));
		validate_respip_action(yystack.l_mark[0].str);
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_actions,
			yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip");
	}
break;
case 551:
#line 2946 "./util/configparser.y"
	{
		OUTYY(("P(server_response_ip_data:%s)\n", yystack.l_mark[-1].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->respip_data,
			yystack.l_mark[-1].str, yystack.l_mark[0].str))
			fatal_exit("out of memory adding response-ip-data");
	}
break;
case 552:
#line 2954 "./util/configparser.y"
	{
		OUTYY(("\nP(dnscrypt:)\n"));
	}
break;
case 565:
#line 2970 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_enable:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dnscrypt = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 566:
#line 2980 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_port:%s)\n", yystack.l_mark[0].str));
		if(atoi(yystack.l_mark[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->dnscrypt_port = atoi(yystack.l_mark[0].str);
		free(yystack.l_mark[0].str);
	}
break;
case 567:
#line 2989 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->dnscrypt_provider);
		cfg_parser->cfg->dnscrypt_provider = yystack.l_mark[0].str;
	}
break;
case 568:
#line 2996 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider_cert:%s)\n", yystack.l_mark[0].str));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_provider_cert, yystack.l_mark[0].str))
			log_warn("dnscrypt-provider-cert %s is a duplicate", yystack.l_mark[0].str);
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-provider-cert");
	}
break;
case 569:
#line 3005 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_provider_cert_rotated:%s)\n", yystack.l_mark[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_provider_cert_rotated, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-provider-cert-rotated");
	}
break;
case 570:
#line 3012 "./util/configparser.y"
	{
		OUTYY(("P(dnsc_dnscrypt_secret_key:%s)\n", yystack.l_mark[0].str));
		if(cfg_strlist_find(cfg_parser->cfg->dnscrypt_secret_key, yystack.l_mark[0].str))
			log_warn("dnscrypt-secret-key: %s is a duplicate", yystack.l_mark[0].str);
		if(!cfg_strlist_insert(&cfg_parser->cfg->dnscrypt_secret_key, yystack.l_mark[0].str))
			fatal_exit("out of memory adding dnscrypt-secret-key");
	}
break;
case 571:
#line 3021 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_shared_secret_cache_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->dnscrypt_shared_secret_cache_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 572:
#line 3029 "./util/configparser.y"
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
case 573:
#line 3042 "./util/configparser.y"
	{
  	OUTYY(("P(dnscrypt_nonce_cache_size:%s)\n", yystack.l_mark[0].str));
  	if(!cfg_parse_memsize(yystack.l_mark[0].str, &cfg_parser->cfg->dnscrypt_nonce_cache_size))
  		yyerror("memory size expected");
  	free(yystack.l_mark[0].str);
  }
break;
case 574:
#line 3050 "./util/configparser.y"
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
case 575:
#line 3063 "./util/configparser.y"
	{
		OUTYY(("\nP(cachedb:)\n"));
	}
break;
case 583:
#line 3073 "./util/configparser.y"
	{
	#ifdef USE_CACHEDB
		OUTYY(("P(backend:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->cachedb_backend);
		cfg_parser->cfg->cachedb_backend = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 584:
#line 3085 "./util/configparser.y"
	{
	#ifdef USE_CACHEDB
		OUTYY(("P(secret-seed:%s)\n", yystack.l_mark[0].str));
		free(cfg_parser->cfg->cachedb_secret);
		cfg_parser->cfg->cachedb_secret = yystack.l_mark[0].str;
	#else
		OUTYY(("P(Compiled without cachedb, ignoring)\n"));
		free(yystack.l_mark[0].str);
	#endif
	}
break;
case 585:
#line 3097 "./util/configparser.y"
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
case 586:
#line 3109 "./util/configparser.y"
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
case 587:
#line 3124 "./util/configparser.y"
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
case 588:
#line 3137 "./util/configparser.y"
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
case 589:
#line 3148 "./util/configparser.y"
	{
		OUTYY(("P(server_answer_cookie:%s)\n", yystack.l_mark[0].str));
		if(strcmp(yystack.l_mark[0].str, "yes") != 0 && strcmp(yystack.l_mark[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_answer_cookie = (strcmp(yystack.l_mark[0].str, "yes")==0);
		free(yystack.l_mark[0].str);
	}
break;
case 590:
#line 3157 "./util/configparser.y"
	{
		uint8_t secret[32];
		size_t secret_len = sizeof(secret);

		OUTYY(("P(server_cookie_secret:%s)\n", yystack.l_mark[0].str));
		if (sldns_str2wire_hex_buf(yystack.l_mark[0].str, secret, &secret_len)
		|| (  secret_len != 16))
			yyerror("expected 128 bit hex string");
		else {
			cfg_parser->cfg->cookie_secret_len = secret_len;
			memcpy(cfg_parser->cfg->cookie_secret, secret, sizeof(secret));
		}
		free(yystack.l_mark[0].str);
	}
break;
case 591:
#line 3172 "./util/configparser.y"
	{
			OUTYY(("\nP(ipset:)\n"));
		}
break;
case 596:
#line 3181 "./util/configparser.y"
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
case 597:
#line 3196 "./util/configparser.y"
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
#line 5160 "util/configparser.c"
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
