#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20070509

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 39 "util/configparser.y"
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

/* these need to be global, otherwise they cannot be used inside yacc */
extern struct config_parser_state* cfg_parser;

#if 0
#define OUTYY(s)  printf s /* used ONLY when debugging */
#else
#define OUTYY(s)
#endif

#line 64 "util/configparser.y"
typedef union {
	char*	str;
} YYSTYPE;
#line 52 "util/configparser.c"
#define SPACE 257
#define LETTER 258
#define NEWLINE 259
#define COMMENT 260
#define COLON 261
#define ANY 262
#define ZONESTR 263
#define STRING 264
#define VAR_SERVER 265
#define VAR_VERBOSITY 266
#define VAR_NUM_THREADS 267
#define VAR_PORT 268
#define VAR_OUTGOING_RANGE 269
#define VAR_INTERFACE 270
#define VAR_DO_IP4 271
#define VAR_DO_IP6 272
#define VAR_DO_UDP 273
#define VAR_DO_TCP 274
#define VAR_CHROOT 275
#define VAR_USERNAME 276
#define VAR_DIRECTORY 277
#define VAR_LOGFILE 278
#define VAR_PIDFILE 279
#define VAR_MSG_CACHE_SIZE 280
#define VAR_MSG_CACHE_SLABS 281
#define VAR_NUM_QUERIES_PER_THREAD 282
#define VAR_RRSET_CACHE_SIZE 283
#define VAR_RRSET_CACHE_SLABS 284
#define VAR_OUTGOING_NUM_TCP 285
#define VAR_INFRA_HOST_TTL 286
#define VAR_INFRA_LAME_TTL 287
#define VAR_INFRA_CACHE_SLABS 288
#define VAR_INFRA_CACHE_NUMHOSTS 289
#define VAR_INFRA_CACHE_LAME_SIZE 290
#define VAR_NAME 291
#define VAR_STUB_ZONE 292
#define VAR_STUB_HOST 293
#define VAR_STUB_ADDR 294
#define VAR_TARGET_FETCH_POLICY 295
#define VAR_HARDEN_SHORT_BUFSIZE 296
#define VAR_HARDEN_LARGE_QUERIES 297
#define VAR_FORWARD_ZONE 298
#define VAR_FORWARD_HOST 299
#define VAR_FORWARD_ADDR 300
#define VAR_DO_NOT_QUERY_ADDRESS 301
#define VAR_HIDE_IDENTITY 302
#define VAR_HIDE_VERSION 303
#define VAR_IDENTITY 304
#define VAR_VERSION 305
#define VAR_HARDEN_GLUE 306
#define VAR_MODULE_CONF 307
#define VAR_TRUST_ANCHOR_FILE 308
#define VAR_TRUST_ANCHOR 309
#define VAR_VAL_OVERRIDE_DATE 310
#define VAR_BOGUS_TTL 311
#define VAR_VAL_CLEAN_ADDITIONAL 312
#define VAR_VAL_PERMISSIVE_MODE 313
#define VAR_INCOMING_NUM_TCP 314
#define VAR_MSG_BUFFER_SIZE 315
#define VAR_KEY_CACHE_SIZE 316
#define VAR_KEY_CACHE_SLABS 317
#define VAR_TRUSTED_KEYS_FILE 318
#define VAR_VAL_NSEC3_KEYSIZE_ITERATIONS 319
#define VAR_USE_SYSLOG 320
#define VAR_OUTGOING_INTERFACE 321
#define VAR_ROOT_HINTS 322
#define VAR_DO_NOT_QUERY_LOCALHOST 323
#define VAR_CACHE_MAX_TTL 324
#define VAR_HARDEN_DNNSEC_STRIPPED 325
#define VAR_ACCESS_CONTROL 326
#define VAR_LOCAL_ZONE 327
#define VAR_LOCAL_DATA 328
#define VAR_INTERFACE_AUTOMATIC 329
#define VAR_STATISTICS_INTERVAL 330
#define VAR_DO_DAEMONIZE 331
#define VAR_USE_CAPS_FOR_ID 332
#define VAR_STATISTICS_CUMULATIVE 333
#define VAR_OUTGOING_PORT_PERMIT 334
#define VAR_OUTGOING_PORT_AVOID 335
#define VAR_DLV_ANCHOR_FILE 336
#define VAR_DLV_ANCHOR 337
#define VAR_NEG_CACHE_SIZE 338
#define VAR_HARDEN_REFERRAL_PATH 339
#define VAR_PRIVATE_ADDRESS 340
#define VAR_PRIVATE_DOMAIN 341
#define VAR_REMOTE_CONTROL 342
#define VAR_CONTROL_ENABLE 343
#define VAR_CONTROL_INTERFACE 344
#define VAR_CONTROL_PORT 345
#define VAR_SERVER_KEY_FILE 346
#define VAR_SERVER_CERT_FILE 347
#define VAR_CONTROL_KEY_FILE 348
#define VAR_CONTROL_CERT_FILE 349
#define VAR_EXTENDED_STATISTICS 350
#define VAR_LOCAL_DATA_PTR 351
#define VAR_JOSTLE_TIMEOUT 352
#define VAR_STUB_PRIME 353
#define VAR_UNWANTED_REPLY_THRESHOLD 354
#define VAR_LOG_TIME_ASCII 355
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    1,    1,    1,    1,    2,    3,    3,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,    4,    5,    5,   85,   85,   85,   85,
    6,    7,    7,   90,   90,   90,   11,   12,   68,   71,
   80,   13,   19,   59,   14,   72,   73,   30,   52,   67,
   15,   16,   17,   18,   69,   58,   84,   20,   21,   22,
   23,   24,   60,   74,   75,   46,   56,   47,   40,   41,
   42,   43,   53,   25,   26,   27,   82,   28,   29,   31,
   32,   34,   35,   33,   36,   37,   38,   44,   63,   77,
   70,   78,   79,   83,   39,   61,   64,   45,   48,   62,
   49,   50,   51,   57,   54,   55,   76,   65,   66,   81,
   86,   87,   88,   89,   91,   92,   93,    8,    9,    9,
   94,   94,   94,   94,   94,   94,   94,   95,   97,   96,
   98,   99,  100,  101,
};
short yylen[] = {                                         2,
    0,    2,    2,    2,    2,    2,    1,    2,    0,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    2,    0,    1,    1,    1,    1,
    1,    2,    0,    1,    1,    1,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    3,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    3,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    1,    2,    0,
    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,
    2,    2,    2,    2,
};
short yydefred[] = {                                      1,
    0,    7,   84,   91,  178,    2,    9,   86,   93,  180,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    8,   10,
   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,
   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,
   61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
   71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
   81,   82,   83,    0,    0,    0,    0,   85,   87,   88,
   89,   90,    0,    0,    0,   92,   94,   95,   96,    0,
    0,    0,    0,    0,    0,    0,  179,  181,  182,  183,
  184,  185,  186,  187,   98,   97,  102,  105,  103,  111,
  112,  113,  114,  118,  119,  120,  121,  122,  134,  135,
  136,  138,  139,  108,  140,  141,  144,  142,  143,  145,
  146,  147,  155,  129,  130,  131,  132,  148,  158,  126,
  128,  159,  161,  162,  163,  109,  133,  165,  166,  127,
  164,  116,  104,  123,  156,  160,  149,    0,    0,  169,
  110,   99,  115,  151,  100,  106,  107,  124,  125,  167,
  150,  152,  153,  101,  170,  137,  154,  117,  171,  172,
  173,  174,  175,  176,  177,  188,  190,  189,  191,  192,
  193,  194,  157,  168,
};
short yydgoto[] = {                                       1,
    6,    7,   11,    8,   12,    9,   13,   10,   14,   89,
   90,   91,   92,   93,   94,   95,   96,   97,   98,   99,
  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,
  110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
  120,  121,  122,  123,  124,  125,  126,  127,  128,  129,
  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
  140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
  160,  161,  162,  163,  168,  169,  170,  171,  172,  176,
  177,  178,  179,  187,  188,  189,  190,  191,  192,  193,
  194,
};
short yysindex[] = {                                      0,
 -184,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -261, -209, -204, -246, -264, -234, -233, -232, -231, -227,
 -226, -225, -181, -178, -176, -172, -160, -159, -158, -157,
 -155, -154, -153, -152, -151, -149, -148, -147, -146, -145,
 -144, -143, -142, -141, -140, -139, -138, -137, -136, -135,
 -134, -133, -132, -131, -130, -129, -128, -127, -126, -125,
 -124, -123, -122, -121, -119, -118, -117, -116, -115, -114,
 -113, -112, -111, -110, -109, -108, -107, -105, -104, -103,
 -102, -101, -100,  -99,  -98,  -97,  -96,  -95,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -94,  -93,  -92,  -91,    0,    0,    0,
    0,    0,  -90,  -89,  -88,    0,    0,    0,    0,  -87,
  -86,  -85,  -84,  -83,  -82,  -81,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -80,  -79,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    1,    2,    3,    4,    0,    0,    0,    0,    0,    0,
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
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
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
#define YYTABLESIZE 346
short yytable[] = {                                     195,
    3,    4,    5,    6,   15,   16,   17,   18,   19,   20,
   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
   31,   32,   33,   34,   35,   36,   37,   38,   39,  196,
  197,  198,  199,   40,   41,   42,  200,  201,  202,   43,
   44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,
   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
    2,  164,  203,  165,  166,  204,  173,  205,   84,   85,
   86,  206,   87,   88,  174,  175,  180,  181,  182,  183,
  184,  185,  186,  207,  208,  209,  210,    3,  211,  212,
  213,  214,  215,    4,  216,  217,  218,  219,  220,  221,
  222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
  232,  233,  234,  235,  236,  237,  238,  239,  240,  241,
  242,  243,  244,  167,  245,  246,  247,  248,  249,  250,
  251,  252,  253,  254,  255,  256,  257,    5,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    3,    4,    5,    6,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    3,    4,    5,    6,    0,    0,    3,    4,
    5,    6,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    3,    4,    5,    6,
};
short yycheck[] = {                                     264,
    0,    0,    0,    0,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,  285,  286,  287,  288,  289,  290,  264,
  264,  264,  264,  295,  296,  297,  264,  264,  264,  301,
  302,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  265,  291,  264,  293,  294,  264,  291,  264,  350,  351,
  352,  264,  354,  355,  299,  300,  343,  344,  345,  346,
  347,  348,  349,  264,  264,  264,  264,  292,  264,  264,
  264,  264,  264,  298,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  353,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  342,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  265,  265,  265,  265,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  292,  292,  292,  292,   -1,   -1,  298,  298,
  298,  298,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  342,  342,  342,  342,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 355
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"SPACE","LETTER","NEWLINE",
"COMMENT","COLON","ANY","ZONESTR","STRING","VAR_SERVER","VAR_VERBOSITY",
"VAR_NUM_THREADS","VAR_PORT","VAR_OUTGOING_RANGE","VAR_INTERFACE","VAR_DO_IP4",
"VAR_DO_IP6","VAR_DO_UDP","VAR_DO_TCP","VAR_CHROOT","VAR_USERNAME",
"VAR_DIRECTORY","VAR_LOGFILE","VAR_PIDFILE","VAR_MSG_CACHE_SIZE",
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
"VAR_HARDEN_DNNSEC_STRIPPED","VAR_ACCESS_CONTROL","VAR_LOCAL_ZONE",
"VAR_LOCAL_DATA","VAR_INTERFACE_AUTOMATIC","VAR_STATISTICS_INTERVAL",
"VAR_DO_DAEMONIZE","VAR_USE_CAPS_FOR_ID","VAR_STATISTICS_CUMULATIVE",
"VAR_OUTGOING_PORT_PERMIT","VAR_OUTGOING_PORT_AVOID","VAR_DLV_ANCHOR_FILE",
"VAR_DLV_ANCHOR","VAR_NEG_CACHE_SIZE","VAR_HARDEN_REFERRAL_PATH",
"VAR_PRIVATE_ADDRESS","VAR_PRIVATE_DOMAIN","VAR_REMOTE_CONTROL",
"VAR_CONTROL_ENABLE","VAR_CONTROL_INTERFACE","VAR_CONTROL_PORT",
"VAR_SERVER_KEY_FILE","VAR_SERVER_CERT_FILE","VAR_CONTROL_KEY_FILE",
"VAR_CONTROL_CERT_FILE","VAR_EXTENDED_STATISTICS","VAR_LOCAL_DATA_PTR",
"VAR_JOSTLE_TIMEOUT","VAR_STUB_PRIME","VAR_UNWANTED_REPLY_THRESHOLD",
"VAR_LOG_TIME_ASCII",
};
char *yyrule[] = {
"$accept : toplevelvars",
"toplevelvars :",
"toplevelvars : toplevelvars toplevelvar",
"toplevelvar : serverstart contents_server",
"toplevelvar : stubstart contents_stub",
"toplevelvar : forwardstart contents_forward",
"toplevelvar : rcstart contents_rc",
"serverstart : VAR_SERVER",
"contents_server : contents_server content_server",
"contents_server :",
"content_server : server_num_threads",
"content_server : server_verbosity",
"content_server : server_port",
"content_server : server_outgoing_range",
"content_server : server_do_ip4",
"content_server : server_do_ip6",
"content_server : server_do_udp",
"content_server : server_do_tcp",
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
"stubstart : VAR_STUB_ZONE",
"contents_stub : contents_stub content_stub",
"contents_stub :",
"content_stub : stub_name",
"content_stub : stub_host",
"content_stub : stub_addr",
"content_stub : stub_prime",
"forwardstart : VAR_FORWARD_ZONE",
"contents_forward : contents_forward content_forward",
"contents_forward :",
"content_forward : forward_name",
"content_forward : forward_host",
"content_forward : forward_addr",
"server_num_threads : VAR_NUM_THREADS STRING",
"server_verbosity : VAR_VERBOSITY STRING",
"server_statistics_interval : VAR_STATISTICS_INTERVAL STRING",
"server_statistics_cumulative : VAR_STATISTICS_CUMULATIVE STRING",
"server_extended_statistics : VAR_EXTENDED_STATISTICS STRING",
"server_port : VAR_PORT STRING",
"server_interface : VAR_INTERFACE STRING",
"server_outgoing_interface : VAR_OUTGOING_INTERFACE STRING",
"server_outgoing_range : VAR_OUTGOING_RANGE STRING",
"server_outgoing_port_permit : VAR_OUTGOING_PORT_PERMIT STRING",
"server_outgoing_port_avoid : VAR_OUTGOING_PORT_AVOID STRING",
"server_outgoing_num_tcp : VAR_OUTGOING_NUM_TCP STRING",
"server_incoming_num_tcp : VAR_INCOMING_NUM_TCP STRING",
"server_interface_automatic : VAR_INTERFACE_AUTOMATIC STRING",
"server_do_ip4 : VAR_DO_IP4 STRING",
"server_do_ip6 : VAR_DO_IP6 STRING",
"server_do_udp : VAR_DO_UDP STRING",
"server_do_tcp : VAR_DO_TCP STRING",
"server_do_daemonize : VAR_DO_DAEMONIZE STRING",
"server_use_syslog : VAR_USE_SYSLOG STRING",
"server_log_time_ascii : VAR_LOG_TIME_ASCII STRING",
"server_chroot : VAR_CHROOT STRING",
"server_username : VAR_USERNAME STRING",
"server_directory : VAR_DIRECTORY STRING",
"server_logfile : VAR_LOGFILE STRING",
"server_pidfile : VAR_PIDFILE STRING",
"server_root_hints : VAR_ROOT_HINTS STRING",
"server_dlv_anchor_file : VAR_DLV_ANCHOR_FILE STRING",
"server_dlv_anchor : VAR_DLV_ANCHOR STRING",
"server_trust_anchor_file : VAR_TRUST_ANCHOR_FILE STRING",
"server_trusted_keys_file : VAR_TRUSTED_KEYS_FILE STRING",
"server_trust_anchor : VAR_TRUST_ANCHOR STRING",
"server_hide_identity : VAR_HIDE_IDENTITY STRING",
"server_hide_version : VAR_HIDE_VERSION STRING",
"server_identity : VAR_IDENTITY STRING",
"server_version : VAR_VERSION STRING",
"server_msg_buffer_size : VAR_MSG_BUFFER_SIZE STRING",
"server_msg_cache_size : VAR_MSG_CACHE_SIZE STRING",
"server_msg_cache_slabs : VAR_MSG_CACHE_SLABS STRING",
"server_num_queries_per_thread : VAR_NUM_QUERIES_PER_THREAD STRING",
"server_jostle_timeout : VAR_JOSTLE_TIMEOUT STRING",
"server_rrset_cache_size : VAR_RRSET_CACHE_SIZE STRING",
"server_rrset_cache_slabs : VAR_RRSET_CACHE_SLABS STRING",
"server_infra_host_ttl : VAR_INFRA_HOST_TTL STRING",
"server_infra_lame_ttl : VAR_INFRA_LAME_TTL STRING",
"server_infra_cache_numhosts : VAR_INFRA_CACHE_NUMHOSTS STRING",
"server_infra_cache_lame_size : VAR_INFRA_CACHE_LAME_SIZE STRING",
"server_infra_cache_slabs : VAR_INFRA_CACHE_SLABS STRING",
"server_target_fetch_policy : VAR_TARGET_FETCH_POLICY STRING",
"server_harden_short_bufsize : VAR_HARDEN_SHORT_BUFSIZE STRING",
"server_harden_large_queries : VAR_HARDEN_LARGE_QUERIES STRING",
"server_harden_glue : VAR_HARDEN_GLUE STRING",
"server_harden_dnssec_stripped : VAR_HARDEN_DNNSEC_STRIPPED STRING",
"server_harden_referral_path : VAR_HARDEN_REFERRAL_PATH STRING",
"server_use_caps_for_id : VAR_USE_CAPS_FOR_ID STRING",
"server_private_address : VAR_PRIVATE_ADDRESS STRING",
"server_private_domain : VAR_PRIVATE_DOMAIN STRING",
"server_unwanted_reply_threshold : VAR_UNWANTED_REPLY_THRESHOLD STRING",
"server_do_not_query_address : VAR_DO_NOT_QUERY_ADDRESS STRING",
"server_do_not_query_localhost : VAR_DO_NOT_QUERY_LOCALHOST STRING",
"server_access_control : VAR_ACCESS_CONTROL STRING STRING",
"server_module_conf : VAR_MODULE_CONF STRING",
"server_val_override_date : VAR_VAL_OVERRIDE_DATE STRING",
"server_cache_max_ttl : VAR_CACHE_MAX_TTL STRING",
"server_bogus_ttl : VAR_BOGUS_TTL STRING",
"server_val_clean_additional : VAR_VAL_CLEAN_ADDITIONAL STRING",
"server_val_permissive_mode : VAR_VAL_PERMISSIVE_MODE STRING",
"server_val_nsec3_keysize_iterations : VAR_VAL_NSEC3_KEYSIZE_ITERATIONS STRING",
"server_key_cache_size : VAR_KEY_CACHE_SIZE STRING",
"server_key_cache_slabs : VAR_KEY_CACHE_SLABS STRING",
"server_neg_cache_size : VAR_NEG_CACHE_SIZE STRING",
"server_local_zone : VAR_LOCAL_ZONE STRING STRING",
"server_local_data : VAR_LOCAL_DATA STRING",
"server_local_data_ptr : VAR_LOCAL_DATA_PTR STRING",
"stub_name : VAR_NAME STRING",
"stub_host : VAR_STUB_HOST STRING",
"stub_addr : VAR_STUB_ADDR STRING",
"stub_prime : VAR_STUB_PRIME STRING",
"forward_name : VAR_NAME STRING",
"forward_host : VAR_FORWARD_HOST STRING",
"forward_addr : VAR_FORWARD_ADDR STRING",
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
"rc_control_enable : VAR_CONTROL_ENABLE STRING",
"rc_control_port : VAR_CONTROL_PORT STRING",
"rc_control_interface : VAR_CONTROL_INTERFACE STRING",
"rc_server_key_file : VAR_SERVER_KEY_FILE STRING",
"rc_server_cert_file : VAR_SERVER_CERT_FILE STRING",
"rc_control_key_file : VAR_CONTROL_KEY_FILE STRING",
"rc_control_cert_file : VAR_CONTROL_CERT_FILE STRING",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
#line 998 "util/configparser.y"

/* parse helper routines could be here */
#line 669 "util/configparser.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

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

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
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
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
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

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
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
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 7:
#line 109 "util/configparser.y"
{ 
		OUTYY(("\nP(server:)\n")); 
	}
break;
case 84:
#line 149 "util/configparser.y"
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
case 91:
#line 165 "util/configparser.y"
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
case 97:
#line 181 "util/configparser.y"
{ 
		OUTYY(("P(server_num_threads:%s)\n", yyvsp[0].str)); 
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 98:
#line 190 "util/configparser.y"
{ 
		OUTYY(("P(server_verbosity:%s)\n", yyvsp[0].str)); 
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 99:
#line 199 "util/configparser.y"
{ 
		OUTYY(("P(server_statistics_interval:%s)\n", yyvsp[0].str)); 
		if(strcmp(yyvsp[0].str, "") == 0 || strcmp(yyvsp[0].str, "0") == 0)
			cfg_parser->cfg->stat_interval = 0;
		else if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->stat_interval = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 100:
#line 210 "util/configparser.y"
{
		OUTYY(("P(server_statistics_cumulative:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 101:
#line 219 "util/configparser.y"
{
		OUTYY(("P(server_extended_statistics:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 102:
#line 228 "util/configparser.y"
{
		OUTYY(("P(server_port:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 103:
#line 237 "util/configparser.y"
{
		OUTYY(("P(server_interface:%s)\n", yyvsp[0].str));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = yyvsp[0].str;
	}
break;
case 104:
#line 250 "util/configparser.y"
{
		OUTYY(("P(server_outgoing_interface:%s)\n", yyvsp[0].str));
		if(cfg_parser->cfg->num_out_ifs == 0)
			cfg_parser->cfg->out_ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->out_ifs = realloc(
			cfg_parser->cfg->out_ifs, 
			(cfg_parser->cfg->num_out_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->out_ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->out_ifs[
				cfg_parser->cfg->num_out_ifs++] = yyvsp[0].str;
	}
break;
case 105:
#line 265 "util/configparser.y"
{
		OUTYY(("P(server_outgoing_range:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 106:
#line 274 "util/configparser.y"
{
		OUTYY(("P(server_outgoing_port_permit:%s)\n", yyvsp[0].str));
		if(!cfg_mark_ports(yyvsp[0].str, 1, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yyvsp[0].str);
	}
break;
case 107:
#line 283 "util/configparser.y"
{
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", yyvsp[0].str));
		if(!cfg_mark_ports(yyvsp[0].str, 0, 
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free(yyvsp[0].str);
	}
break;
case 108:
#line 292 "util/configparser.y"
{
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 109:
#line 301 "util/configparser.y"
{
		OUTYY(("P(server_incoming_num_tcp:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 110:
#line 310 "util/configparser.y"
{
		OUTYY(("P(server_interface_automatic:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 111:
#line 319 "util/configparser.y"
{
		OUTYY(("P(server_do_ip4:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 112:
#line 328 "util/configparser.y"
{
		OUTYY(("P(server_do_ip6:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 113:
#line 337 "util/configparser.y"
{
		OUTYY(("P(server_do_udp:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 114:
#line 346 "util/configparser.y"
{
		OUTYY(("P(server_do_tcp:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 115:
#line 355 "util/configparser.y"
{
		OUTYY(("P(server_do_daemonize:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 116:
#line 364 "util/configparser.y"
{
		OUTYY(("P(server_use_syslog:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp(yyvsp[0].str, "yes")==0);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if(strcmp(yyvsp[0].str, "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free(yyvsp[0].str);
	}
break;
case 117:
#line 378 "util/configparser.y"
{
		OUTYY(("P(server_log_time_ascii:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 118:
#line 387 "util/configparser.y"
{
		OUTYY(("P(server_chroot:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = yyvsp[0].str;
	}
break;
case 119:
#line 394 "util/configparser.y"
{
		OUTYY(("P(server_username:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = yyvsp[0].str;
	}
break;
case 120:
#line 401 "util/configparser.y"
{
		OUTYY(("P(server_directory:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = yyvsp[0].str;
	}
break;
case 121:
#line 408 "util/configparser.y"
{
		OUTYY(("P(server_logfile:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = yyvsp[0].str;
		cfg_parser->cfg->use_syslog = 0;
	}
break;
case 122:
#line 416 "util/configparser.y"
{
		OUTYY(("P(server_pidfile:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = yyvsp[0].str;
	}
break;
case 123:
#line 423 "util/configparser.y"
{
		OUTYY(("P(server_root_hints:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 124:
#line 430 "util/configparser.y"
{
		OUTYY(("P(server_dlv_anchor_file:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->dlv_anchor_file);
		cfg_parser->cfg->dlv_anchor_file = yyvsp[0].str;
	}
break;
case 125:
#line 437 "util/configparser.y"
{
		OUTYY(("P(server_dlv_anchor:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->dlv_anchor_list, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 126:
#line 444 "util/configparser.y"
{
		OUTYY(("P(server_trust_anchor_file:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 127:
#line 452 "util/configparser.y"
{
		OUTYY(("P(server_trusted_keys_file:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 128:
#line 460 "util/configparser.y"
{
		OUTYY(("P(server_trust_anchor:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 129:
#line 467 "util/configparser.y"
{
		OUTYY(("P(server_hide_identity:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 130:
#line 476 "util/configparser.y"
{
		OUTYY(("P(server_hide_version:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 131:
#line 485 "util/configparser.y"
{
		OUTYY(("P(server_identity:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = yyvsp[0].str;
	}
break;
case 132:
#line 492 "util/configparser.y"
{
		OUTYY(("P(server_version:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = yyvsp[0].str;
	}
break;
case 133:
#line 499 "util/configparser.y"
{
		OUTYY(("P(server_msg_buffer_size:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else if (atoi(yyvsp[0].str) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else cfg_parser->cfg->msg_buffer_size = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 134:
#line 510 "util/configparser.y"
{
		OUTYY(("P(server_msg_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
break;
case 135:
#line 518 "util/configparser.y"
{
		OUTYY(("P(server_msg_cache_slabs:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->msg_cache_slabs = atoi(yyvsp[0].str);
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yyvsp[0].str);
	}
break;
case 136:
#line 531 "util/configparser.y"
{
		OUTYY(("P(server_num_queries_per_thread:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 137:
#line 540 "util/configparser.y"
{
		OUTYY(("P(server_jostle_timeout:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 138:
#line 549 "util/configparser.y"
{
		OUTYY(("P(server_rrset_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
break;
case 139:
#line 557 "util/configparser.y"
{
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->rrset_cache_slabs = atoi(yyvsp[0].str);
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yyvsp[0].str);
	}
break;
case 140:
#line 570 "util/configparser.y"
{
		OUTYY(("P(server_infra_host_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 141:
#line 579 "util/configparser.y"
{
		OUTYY(("P(server_infra_lame_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->lame_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 142:
#line 588 "util/configparser.y"
{
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 143:
#line 597 "util/configparser.y"
{
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->
			infra_cache_lame_size))
			yyerror("number expected");
		free(yyvsp[0].str);
	}
break;
case 144:
#line 606 "util/configparser.y"
{
		OUTYY(("P(server_infra_cache_slabs:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->infra_cache_slabs = atoi(yyvsp[0].str);
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yyvsp[0].str);
	}
break;
case 145:
#line 619 "util/configparser.y"
{
		OUTYY(("P(server_target_fetch_policy:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = yyvsp[0].str;
	}
break;
case 146:
#line 626 "util/configparser.y"
{
		OUTYY(("P(server_harden_short_bufsize:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 147:
#line 636 "util/configparser.y"
{
		OUTYY(("P(server_harden_large_queries:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 148:
#line 646 "util/configparser.y"
{
		OUTYY(("P(server_harden_glue:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 149:
#line 656 "util/configparser.y"
{
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 150:
#line 666 "util/configparser.y"
{
		OUTYY(("P(server_harden_referral_path:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 151:
#line 676 "util/configparser.y"
{
		OUTYY(("P(server_use_caps_for_id:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 152:
#line 686 "util/configparser.y"
{
		OUTYY(("P(server_private_address:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 153:
#line 693 "util/configparser.y"
{
		OUTYY(("P(server_private_domain:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 154:
#line 700 "util/configparser.y"
{
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 155:
#line 709 "util/configparser.y"
{
		OUTYY(("P(server_do_not_query_address:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 156:
#line 716 "util/configparser.y"
{
		OUTYY(("P(server_do_not_query_localhost:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 157:
#line 726 "util/configparser.y"
{
		OUTYY(("P(server_access_control:%s %s)\n", yyvsp[-1].str, yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "deny")!=0 && strcmp(yyvsp[0].str, "refuse")!=0 &&
			strcmp(yyvsp[0].str, "allow")!=0 && 
			strcmp(yyvsp[0].str, "allow_snoop")!=0) {
			yyerror("expected deny, refuse, allow or allow_snoop "
				"in access control action");
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, yyvsp[-1].str, yyvsp[0].str))
				fatal_exit("out of memory adding acl");
		}
	}
break;
case 158:
#line 740 "util/configparser.y"
{
		OUTYY(("P(server_module_conf:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = yyvsp[0].str;
	}
break;
case 159:
#line 747 "util/configparser.y"
{
		OUTYY(("P(server_val_override_date:%s)\n", yyvsp[0].str));
		if(strlen(yyvsp[0].str) == 0 || strcmp(yyvsp[0].str, "0") == 0) {
			cfg_parser->cfg->val_date_override = 0;
		} else if(strlen(yyvsp[0].str) == 14) {
			cfg_parser->cfg->val_date_override = 
				cfg_convert_timeval(yyvsp[0].str);
			if(!cfg_parser->cfg->val_date_override)
				yyerror("bad date/time specification");
		} else {
			if(atoi(yyvsp[0].str) == 0)
				yyerror("number expected");
			cfg_parser->cfg->val_date_override = atoi(yyvsp[0].str);
		}
		free(yyvsp[0].str);
	}
break;
case 160:
#line 765 "util/configparser.y"
{
		OUTYY(("P(server_cache_max_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 161:
#line 774 "util/configparser.y"
{
		OUTYY(("P(server_bogus_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 162:
#line 783 "util/configparser.y"
{
		OUTYY(("P(server_val_clean_additional:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 163:
#line 793 "util/configparser.y"
{
		OUTYY(("P(server_val_permissive_mode:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 164:
#line 803 "util/configparser.y"
{
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = yyvsp[0].str;
	}
break;
case 165:
#line 810 "util/configparser.y"
{
		OUTYY(("P(server_key_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
break;
case 166:
#line 818 "util/configparser.y"
{
		OUTYY(("P(server_key_cache_slabs:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->key_cache_slabs = atoi(yyvsp[0].str);
			if(!is_pow2(cfg_parser->cfg->key_cache_slabs))
				yyerror("must be a power of 2");
		}
		free(yyvsp[0].str);
	}
break;
case 167:
#line 831 "util/configparser.y"
{
		OUTYY(("P(server_neg_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
break;
case 168:
#line 839 "util/configparser.y"
{
		OUTYY(("P(server_local_zone:%s %s)\n", yyvsp[-1].str, yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "static")!=0 && strcmp(yyvsp[0].str, "deny")!=0 &&
		   strcmp(yyvsp[0].str, "refuse")!=0 && strcmp(yyvsp[0].str, "redirect")!=0 &&
		   strcmp(yyvsp[0].str, "transparent")!=0 && strcmp(yyvsp[0].str, "nodefault")!=0)
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent or nodefault");
		else if(strcmp(yyvsp[0].str, "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, yyvsp[-1].str))
				fatal_exit("out of memory adding local-zone");
			free(yyvsp[0].str);
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones, 
				yyvsp[-1].str, yyvsp[0].str))
				fatal_exit("out of memory adding local-zone");
		}
	}
break;
case 169:
#line 859 "util/configparser.y"
{
		OUTYY(("P(server_local_data:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, yyvsp[0].str))
			fatal_exit("out of memory adding local-data");
	}
break;
case 170:
#line 866 "util/configparser.y"
{
		char* ptr;
		OUTYY(("P(server_local_data_ptr:%s)\n", yyvsp[0].str));
		ptr = cfg_ptr_reverse(yyvsp[0].str);
		free(yyvsp[0].str);
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
break;
case 171:
#line 881 "util/configparser.y"
{
		OUTYY(("P(name:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = yyvsp[0].str;
	}
break;
case 172:
#line 888 "util/configparser.y"
{
		OUTYY(("P(stub-host:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 173:
#line 895 "util/configparser.y"
{
		OUTYY(("P(stub-addr:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 174:
#line 902 "util/configparser.y"
{
		OUTYY(("P(stub-prime:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 175:
#line 912 "util/configparser.y"
{
		OUTYY(("P(name:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = yyvsp[0].str;
	}
break;
case 176:
#line 919 "util/configparser.y"
{
		OUTYY(("P(forward-host:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 177:
#line 926 "util/configparser.y"
{
		OUTYY(("P(forward-addr:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 178:
#line 933 "util/configparser.y"
{ 
		OUTYY(("\nP(remote-control:)\n")); 
	}
break;
case 188:
#line 944 "util/configparser.y"
{
		OUTYY(("P(control_enable:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
break;
case 189:
#line 954 "util/configparser.y"
{
		OUTYY(("P(control_port:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
break;
case 190:
#line 963 "util/configparser.y"
{
		OUTYY(("P(control_interface:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->control_ifs, yyvsp[0].str))
			yyerror("out of memory");
	}
break;
case 191:
#line 970 "util/configparser.y"
{
		OUTYY(("P(rc_server_key_file:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = yyvsp[0].str;
	}
break;
case 192:
#line 977 "util/configparser.y"
{
		OUTYY(("P(rc_server_cert_file:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = yyvsp[0].str;
	}
break;
case 193:
#line 984 "util/configparser.y"
{
		OUTYY(("P(rc_control_key_file:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = yyvsp[0].str;
	}
break;
case 194:
#line 991 "util/configparser.y"
{
		OUTYY(("P(rc_control_cert_file:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = yyvsp[0].str;
	}
break;
#line 1782 "util/configparser.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
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
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
