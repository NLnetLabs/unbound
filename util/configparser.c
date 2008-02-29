/* A Bison parser, made from util/configparser.y, by GNU bison 1.75.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON	1

/* Pure parsers.  */
#define YYPURE	0

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
     STRING = 265,
     VAR_SERVER = 266,
     VAR_VERBOSITY = 267,
     VAR_NUM_THREADS = 268,
     VAR_PORT = 269,
     VAR_OUTGOING_PORT = 270,
     VAR_OUTGOING_RANGE = 271,
     VAR_INTERFACE = 272,
     VAR_DO_IP4 = 273,
     VAR_DO_IP6 = 274,
     VAR_DO_UDP = 275,
     VAR_DO_TCP = 276,
     VAR_CHROOT = 277,
     VAR_USERNAME = 278,
     VAR_DIRECTORY = 279,
     VAR_LOGFILE = 280,
     VAR_PIDFILE = 281,
     VAR_MSG_CACHE_SIZE = 282,
     VAR_MSG_CACHE_SLABS = 283,
     VAR_NUM_QUERIES_PER_THREAD = 284,
     VAR_RRSET_CACHE_SIZE = 285,
     VAR_RRSET_CACHE_SLABS = 286,
     VAR_OUTGOING_NUM_TCP = 287,
     VAR_INFRA_HOST_TTL = 288,
     VAR_INFRA_LAME_TTL = 289,
     VAR_INFRA_CACHE_SLABS = 290,
     VAR_INFRA_CACHE_NUMHOSTS = 291,
     VAR_INFRA_CACHE_LAME_SIZE = 292,
     VAR_NAME = 293,
     VAR_STUB_ZONE = 294,
     VAR_STUB_HOST = 295,
     VAR_STUB_ADDR = 296,
     VAR_TARGET_FETCH_POLICY = 297,
     VAR_HARDEN_SHORT_BUFSIZE = 298,
     VAR_HARDEN_LARGE_QUERIES = 299,
     VAR_FORWARD_ZONE = 300,
     VAR_FORWARD_HOST = 301,
     VAR_FORWARD_ADDR = 302,
     VAR_DO_NOT_QUERY_ADDRESS = 303,
     VAR_HIDE_IDENTITY = 304,
     VAR_HIDE_VERSION = 305,
     VAR_IDENTITY = 306,
     VAR_VERSION = 307,
     VAR_HARDEN_GLUE = 308,
     VAR_MODULE_CONF = 309,
     VAR_TRUST_ANCHOR_FILE = 310,
     VAR_TRUST_ANCHOR = 311,
     VAR_VAL_OVERRIDE_DATE = 312,
     VAR_BOGUS_TTL = 313,
     VAR_VAL_CLEAN_ADDITIONAL = 314,
     VAR_VAL_PERMISSIVE_MODE = 315,
     VAR_INCOMING_NUM_TCP = 316,
     VAR_MSG_BUFFER_SIZE = 317,
     VAR_KEY_CACHE_SIZE = 318,
     VAR_KEY_CACHE_SLABS = 319,
     VAR_TRUSTED_KEYS_FILE = 320,
     VAR_VAL_NSEC3_KEYSIZE_ITERATIONS = 321,
     VAR_USE_SYSLOG = 322,
     VAR_OUTGOING_INTERFACE = 323,
     VAR_ROOT_HINTS = 324,
     VAR_DO_NOT_QUERY_LOCALHOST = 325,
     VAR_CACHE_MAX_TTL = 326,
     VAR_HARDEN_DNNSEC_STRIPPED = 327,
     VAR_ACCESS_CONTROL = 328,
     VAR_LOCAL_ZONE = 329,
     VAR_LOCAL_DATA = 330,
     VAR_INTERFACE_AUTOMATIC = 331,
     VAR_STATISTICS_INTERVAL = 332,
     VAR_DO_DAEMONIZE = 333,
     VAR_USE_CAPS_FOR_ID = 334
   };
#endif
#define SPACE 258
#define LETTER 259
#define NEWLINE 260
#define COMMENT 261
#define COLON 262
#define ANY 263
#define ZONESTR 264
#define STRING 265
#define VAR_SERVER 266
#define VAR_VERBOSITY 267
#define VAR_NUM_THREADS 268
#define VAR_PORT 269
#define VAR_OUTGOING_PORT 270
#define VAR_OUTGOING_RANGE 271
#define VAR_INTERFACE 272
#define VAR_DO_IP4 273
#define VAR_DO_IP6 274
#define VAR_DO_UDP 275
#define VAR_DO_TCP 276
#define VAR_CHROOT 277
#define VAR_USERNAME 278
#define VAR_DIRECTORY 279
#define VAR_LOGFILE 280
#define VAR_PIDFILE 281
#define VAR_MSG_CACHE_SIZE 282
#define VAR_MSG_CACHE_SLABS 283
#define VAR_NUM_QUERIES_PER_THREAD 284
#define VAR_RRSET_CACHE_SIZE 285
#define VAR_RRSET_CACHE_SLABS 286
#define VAR_OUTGOING_NUM_TCP 287
#define VAR_INFRA_HOST_TTL 288
#define VAR_INFRA_LAME_TTL 289
#define VAR_INFRA_CACHE_SLABS 290
#define VAR_INFRA_CACHE_NUMHOSTS 291
#define VAR_INFRA_CACHE_LAME_SIZE 292
#define VAR_NAME 293
#define VAR_STUB_ZONE 294
#define VAR_STUB_HOST 295
#define VAR_STUB_ADDR 296
#define VAR_TARGET_FETCH_POLICY 297
#define VAR_HARDEN_SHORT_BUFSIZE 298
#define VAR_HARDEN_LARGE_QUERIES 299
#define VAR_FORWARD_ZONE 300
#define VAR_FORWARD_HOST 301
#define VAR_FORWARD_ADDR 302
#define VAR_DO_NOT_QUERY_ADDRESS 303
#define VAR_HIDE_IDENTITY 304
#define VAR_HIDE_VERSION 305
#define VAR_IDENTITY 306
#define VAR_VERSION 307
#define VAR_HARDEN_GLUE 308
#define VAR_MODULE_CONF 309
#define VAR_TRUST_ANCHOR_FILE 310
#define VAR_TRUST_ANCHOR 311
#define VAR_VAL_OVERRIDE_DATE 312
#define VAR_BOGUS_TTL 313
#define VAR_VAL_CLEAN_ADDITIONAL 314
#define VAR_VAL_PERMISSIVE_MODE 315
#define VAR_INCOMING_NUM_TCP 316
#define VAR_MSG_BUFFER_SIZE 317
#define VAR_KEY_CACHE_SIZE 318
#define VAR_KEY_CACHE_SLABS 319
#define VAR_TRUSTED_KEYS_FILE 320
#define VAR_VAL_NSEC3_KEYSIZE_ITERATIONS 321
#define VAR_USE_SYSLOG 322
#define VAR_OUTGOING_INTERFACE 323
#define VAR_ROOT_HINTS 324
#define VAR_DO_NOT_QUERY_LOCALHOST 325
#define VAR_CACHE_MAX_TTL 326
#define VAR_HARDEN_DNNSEC_STRIPPED 327
#define VAR_ACCESS_CONTROL 328
#define VAR_LOCAL_ZONE 329
#define VAR_LOCAL_DATA 330
#define VAR_INTERFACE_AUTOMATIC 331
#define VAR_STATISTICS_INTERVAL 332
#define VAR_DO_DAEMONIZE 333
#define VAR_USE_CAPS_FOR_ID 334




/* Copy the first part of user declarations.  */
#line 38 "util/configparser.y"

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

#ifndef YYSTYPE
#line 64 "util/configparser.y"
typedef union {
	char*	str;
} yystype;
/* Line 188 of /usr/local/share/bison/yacc.c.  */
#line 262 "util/configparser.c"
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} yyltype;
# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* Copy the second part of user declarations.  */


/* Line 213 of /usr/local/share/bison/yacc.c.  */
#line 283 "util/configparser.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];	\
	}					\
      while (0)
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
#define YYLAST   139

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  79
/* YYNRULES -- Number of rules. */
#define YYNRULES  149
/* YYNRULES -- Number of states. */
#define YYNSTATES  219

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   334

#define YYTRANSLATE(X) \
  ((unsigned)(X) <= YYMAXUTOK ? yytranslate[X] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
      75,    76,    77,    78,    79
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    16,    18,    21,
      22,    24,    26,    28,    30,    32,    34,    36,    38,    40,
      42,    44,    46,    48,    50,    52,    54,    56,    58,    60,
      62,    64,    66,    68,    70,    72,    74,    76,    78,    80,
      82,    84,    86,    88,    90,    92,    94,    96,    98,   100,
     102,   104,   106,   108,   110,   112,   114,   116,   118,   120,
     122,   124,   126,   128,   130,   132,   134,   136,   138,   140,
     142,   144,   146,   149,   150,   152,   154,   156,   158,   161,
     162,   164,   166,   168,   171,   174,   177,   180,   183,   186,
     189,   192,   195,   198,   201,   204,   207,   210,   213,   216,
     219,   222,   225,   228,   231,   234,   237,   240,   243,   246,
     249,   252,   255,   258,   261,   264,   267,   270,   273,   276,
     279,   282,   285,   288,   291,   294,   297,   300,   303,   306,
     309,   312,   315,   319,   322,   325,   328,   331,   334,   337,
     340,   343,   346,   350,   353,   356,   359,   362,   365,   368
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      81,     0,    -1,    -1,    81,    82,    -1,    83,    84,    -1,
      86,    87,    -1,    89,    90,    -1,    11,    -1,    84,    85,
      -1,    -1,    92,    -1,    93,    -1,    95,    -1,    98,    -1,
      99,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
      96,    -1,   109,    -1,   110,    -1,   111,    -1,   112,    -1,
     113,    -1,   123,    -1,   124,    -1,   125,    -1,   126,    -1,
     127,    -1,   100,    -1,   128,    -1,   129,    -1,   132,    -1,
     130,    -1,   131,    -1,   133,    -1,   134,    -1,   135,    -1,
     139,    -1,   118,    -1,   119,    -1,   120,    -1,   121,    -1,
     136,    -1,   142,    -1,   115,    -1,   117,    -1,   143,    -1,
     145,    -1,   146,    -1,   147,    -1,   101,    -1,   122,    -1,
     149,    -1,   150,    -1,   116,    -1,   148,    -1,   108,    -1,
      97,    -1,   114,    -1,   140,    -1,   144,    -1,   137,    -1,
     141,    -1,   151,    -1,   152,    -1,   102,    -1,    94,    -1,
     107,    -1,   138,    -1,    39,    -1,    87,    88,    -1,    -1,
     153,    -1,   154,    -1,   155,    -1,    45,    -1,    90,    91,
      -1,    -1,   156,    -1,   157,    -1,   158,    -1,    13,    10,
      -1,    12,    10,    -1,    77,    10,    -1,    14,    10,    -1,
      17,    10,    -1,    68,    10,    -1,    15,    10,    -1,    16,
      10,    -1,    32,    10,    -1,    61,    10,    -1,    76,    10,
      -1,    18,    10,    -1,    19,    10,    -1,    20,    10,    -1,
      21,    10,    -1,    78,    10,    -1,    67,    10,    -1,    22,
      10,    -1,    23,    10,    -1,    24,    10,    -1,    25,    10,
      -1,    26,    10,    -1,    69,    10,    -1,    55,    10,    -1,
      65,    10,    -1,    56,    10,    -1,    49,    10,    -1,    50,
      10,    -1,    51,    10,    -1,    52,    10,    -1,    62,    10,
      -1,    27,    10,    -1,    28,    10,    -1,    29,    10,    -1,
      30,    10,    -1,    31,    10,    -1,    33,    10,    -1,    34,
      10,    -1,    36,    10,    -1,    37,    10,    -1,    35,    10,
      -1,    42,    10,    -1,    43,    10,    -1,    44,    10,    -1,
      53,    10,    -1,    72,    10,    -1,    79,    10,    -1,    48,
      10,    -1,    70,    10,    -1,    73,    10,    10,    -1,    54,
      10,    -1,    57,    10,    -1,    71,    10,    -1,    58,    10,
      -1,    59,    10,    -1,    60,    10,    -1,    66,    10,    -1,
      63,    10,    -1,    64,    10,    -1,    74,    10,    10,    -1,
      75,    10,    -1,    38,    10,    -1,    40,    10,    -1,    41,
      10,    -1,    38,    10,    -1,    46,    10,    -1,    47,    10,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    94,    94,    94,    95,    95,    95,   100,   105,   106,
     107,   107,   107,   107,   108,   108,   108,   109,   109,   109,
     110,   110,   110,   111,   111,   111,   112,   112,   113,   113,
     114,   114,   115,   115,   116,   116,   117,   117,   118,   118,
     119,   119,   120,   120,   120,   121,   121,   121,   122,   122,
     122,   123,   123,   124,   124,   125,   125,   126,   126,   127,
     127,   127,   128,   128,   129,   129,   130,   130,   130,   131,
     131,   134,   146,   147,   148,   148,   148,   150,   162,   163,
     164,   164,   164,   166,   175,   184,   195,   204,   217,   232,
     241,   250,   259,   268,   277,   286,   295,   304,   313,   322,
     336,   343,   350,   357,   365,   372,   379,   387,   395,   402,
     411,   420,   427,   434,   445,   453,   466,   475,   483,   496,
     505,   514,   523,   532,   545,   552,   562,   572,   582,   592,
     602,   609,   619,   632,   639,   657,   666,   675,   685,   695,
     702,   710,   723,   742,   749,   756,   763,   770,   777,   784
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SPACE", "LETTER", "NEWLINE", "COMMENT", 
  "COLON", "ANY", "ZONESTR", "STRING", "VAR_SERVER", "VAR_VERBOSITY", 
  "VAR_NUM_THREADS", "VAR_PORT", "VAR_OUTGOING_PORT", 
  "VAR_OUTGOING_RANGE", "VAR_INTERFACE", "VAR_DO_IP4", "VAR_DO_IP6", 
  "VAR_DO_UDP", "VAR_DO_TCP", "VAR_CHROOT", "VAR_USERNAME", 
  "VAR_DIRECTORY", "VAR_LOGFILE", "VAR_PIDFILE", "VAR_MSG_CACHE_SIZE", 
  "VAR_MSG_CACHE_SLABS", "VAR_NUM_QUERIES_PER_THREAD", 
  "VAR_RRSET_CACHE_SIZE", "VAR_RRSET_CACHE_SLABS", "VAR_OUTGOING_NUM_TCP", 
  "VAR_INFRA_HOST_TTL", "VAR_INFRA_LAME_TTL", "VAR_INFRA_CACHE_SLABS", 
  "VAR_INFRA_CACHE_NUMHOSTS", "VAR_INFRA_CACHE_LAME_SIZE", "VAR_NAME", 
  "VAR_STUB_ZONE", "VAR_STUB_HOST", "VAR_STUB_ADDR", 
  "VAR_TARGET_FETCH_POLICY", "VAR_HARDEN_SHORT_BUFSIZE", 
  "VAR_HARDEN_LARGE_QUERIES", "VAR_FORWARD_ZONE", "VAR_FORWARD_HOST", 
  "VAR_FORWARD_ADDR", "VAR_DO_NOT_QUERY_ADDRESS", "VAR_HIDE_IDENTITY", 
  "VAR_HIDE_VERSION", "VAR_IDENTITY", "VAR_VERSION", "VAR_HARDEN_GLUE", 
  "VAR_MODULE_CONF", "VAR_TRUST_ANCHOR_FILE", "VAR_TRUST_ANCHOR", 
  "VAR_VAL_OVERRIDE_DATE", "VAR_BOGUS_TTL", "VAR_VAL_CLEAN_ADDITIONAL", 
  "VAR_VAL_PERMISSIVE_MODE", "VAR_INCOMING_NUM_TCP", 
  "VAR_MSG_BUFFER_SIZE", "VAR_KEY_CACHE_SIZE", "VAR_KEY_CACHE_SLABS", 
  "VAR_TRUSTED_KEYS_FILE", "VAR_VAL_NSEC3_KEYSIZE_ITERATIONS", 
  "VAR_USE_SYSLOG", "VAR_OUTGOING_INTERFACE", "VAR_ROOT_HINTS", 
  "VAR_DO_NOT_QUERY_LOCALHOST", "VAR_CACHE_MAX_TTL", 
  "VAR_HARDEN_DNNSEC_STRIPPED", "VAR_ACCESS_CONTROL", "VAR_LOCAL_ZONE", 
  "VAR_LOCAL_DATA", "VAR_INTERFACE_AUTOMATIC", "VAR_STATISTICS_INTERVAL", 
  "VAR_DO_DAEMONIZE", "VAR_USE_CAPS_FOR_ID", "$accept", "toplevelvars", 
  "toplevelvar", "serverstart", "contents_server", "content_server", 
  "stubstart", "contents_stub", "content_stub", "forwardstart", 
  "contents_forward", "content_forward", "server_num_threads", 
  "server_verbosity", "server_statistics_interval", "server_port", 
  "server_interface", "server_outgoing_interface", "server_outgoing_port", 
  "server_outgoing_range", "server_outgoing_num_tcp", 
  "server_incoming_num_tcp", "server_interface_automatic", 
  "server_do_ip4", "server_do_ip6", "server_do_udp", "server_do_tcp", 
  "server_do_daemonize", "server_use_syslog", "server_chroot", 
  "server_username", "server_directory", "server_logfile", 
  "server_pidfile", "server_root_hints", "server_trust_anchor_file", 
  "server_trusted_keys_file", "server_trust_anchor", 
  "server_hide_identity", "server_hide_version", "server_identity", 
  "server_version", "server_msg_buffer_size", "server_msg_cache_size", 
  "server_msg_cache_slabs", "server_num_queries_per_thread", 
  "server_rrset_cache_size", "server_rrset_cache_slabs", 
  "server_infra_host_ttl", "server_infra_lame_ttl", 
  "server_infra_cache_numhosts", "server_infra_cache_lame_size", 
  "server_infra_cache_slabs", "server_target_fetch_policy", 
  "server_harden_short_bufsize", "server_harden_large_queries", 
  "server_harden_glue", "server_harden_dnssec_stripped", 
  "server_use_caps_for_id", "server_do_not_query_address", 
  "server_do_not_query_localhost", "server_access_control", 
  "server_module_conf", "server_val_override_date", 
  "server_cache_max_ttl", "server_bogus_ttl", 
  "server_val_clean_additional", "server_val_permissive_mode", 
  "server_val_nsec3_keysize_iterations", "server_key_cache_size", 
  "server_key_cache_slabs", "server_local_zone", "server_local_data", 
  "stub_name", "stub_host", "stub_addr", "forward_name", "forward_host", 
  "forward_addr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    80,    81,    81,    82,    82,    82,    83,    84,    84,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    86,    87,    87,    88,    88,    88,    89,    90,    90,
      91,    91,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     1,     1,     1,     1,     2,     0,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     2,     2,     2,     2,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     7,    71,    77,     3,     9,    73,    79,
       4,     5,     6,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     8,    10,    11,    68,    12,    19,
      59,    13,    14,    30,    52,    67,    15,    16,    17,    18,
      69,    58,    20,    21,    22,    23,    24,    60,    46,    56,
      47,    40,    41,    42,    43,    53,    25,    26,    27,    28,
      29,    31,    32,    34,    35,    33,    36,    37,    38,    44,
      63,    70,    39,    61,    64,    45,    48,    62,    49,    50,
      51,    57,    54,    55,    65,    66,     0,     0,     0,    72,
      74,    75,    76,     0,     0,     0,    78,    80,    81,    82,
      84,    83,    86,    89,    90,    87,    94,    95,    96,    97,
     100,   101,   102,   103,   104,   114,   115,   116,   117,   118,
      91,   119,   120,   123,   121,   122,   124,   125,   126,   130,
     109,   110,   111,   112,   127,   133,   106,   108,   134,   136,
     137,   138,    92,   113,   140,   141,   107,   139,    99,    88,
     105,   131,   135,   128,     0,     0,   143,    93,    85,    98,
     129,   144,   145,   146,   147,   148,   149,   132,   142
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     1,     6,     7,    10,    74,     8,    11,   139,     9,
      12,   146,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   140,   141,   142,   147,   148,   149
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -13
static const short yypact[] =
{
     -13,    68,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -12,    32,    31,    16,    17,    18,    19,    23,    24,    25,
      61,    64,    65,    66,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   122,   123,   124,   -13,
     -13,   -13,   -13,   125,   126,   127,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   128,   129,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13,   -13
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, parse error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,   150,   151,   152,   153,
      39,    40,    41,   154,   155,   156,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     2,   143,
     136,   157,   137,   138,   158,   159,   160,   144,   145,     3,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,     4,   188,   189,
     190,   191,   192,     5,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218
};

static const unsigned char yycheck[] =
{
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    10,    10,    10,    10,
      42,    43,    44,    10,    10,    10,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     0,    38,
      38,    10,    40,    41,    10,    10,    10,    46,    47,    11,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    39,    10,    10,
      10,    10,    10,    45,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    81,     0,    11,    39,    45,    82,    83,    86,    89,
      84,    87,    90,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    42,
      43,    44,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    85,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,    38,    40,    41,    88,
     153,   154,   155,    38,    46,    47,    91,   156,   157,   158,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

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
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#define YYLEX	yylex ()

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
} while (0)
# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*-----------------------------.
| Print this symbol on YYOUT.  |
`-----------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yysymprint (FILE* yyout, int yytype, YYSTYPE yyvalue)
#else
yysymprint (yyout, yytype, yyvalue)
    FILE* yyout;
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyout, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyout, yytoknum[yytype], yyvalue);
# endif
    }
  else
    YYFPRINTF (yyout, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyout, ")");
}
#endif /* YYDEBUG. */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yydestruct (int yytype, YYSTYPE yyvalue)
#else
yydestruct (yytype, yyvalue)
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  switch (yytype)
    {
      default:
        break;
    }
}



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of parse errors so far.  */
int yynerrs;


int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
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

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with.  */

  if (yychar <= 0)		/* This means end of input.  */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more.  */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

      /* We have to keep this `#if YYDEBUG', since we use variables
	 which are defined only if `YYDEBUG' is set.  */
      YYDPRINTF ((stderr, "Next token is "));
      YYDSYMPRINT ((stderr, yychar1, yylval));
      YYDPRINTF ((stderr, "\n"));
    }

  /* If the proper action on seeing token YYCHAR1 is to reduce or to
     detect an error, take that action.  */
  yyn += yychar1;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yychar1)
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

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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



#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn - 1, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] >= 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif
  switch (yyn)
    {
        case 7:
#line 101 "util/configparser.y"
    { 
		OUTYY(("\nP(server:)\n")); 
	}
    break;

  case 71:
#line 135 "util/configparser.y"
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

  case 77:
#line 151 "util/configparser.y"
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

  case 83:
#line 167 "util/configparser.y"
    { 
		OUTYY(("P(server_num_threads:%s)\n", yyvsp[0].str)); 
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 84:
#line 176 "util/configparser.y"
    { 
		OUTYY(("P(server_verbosity:%s)\n", yyvsp[0].str)); 
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 85:
#line 185 "util/configparser.y"
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

  case 86:
#line 196 "util/configparser.y"
    {
		OUTYY(("P(server_port:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 87:
#line 205 "util/configparser.y"
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

  case 88:
#line 218 "util/configparser.y"
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

  case 89:
#line 233 "util/configparser.y"
    {
		OUTYY(("P(server_outgoing_port:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->outgoing_base_port = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 90:
#line 242 "util/configparser.y"
    {
		OUTYY(("P(server_outgoing_range:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 91:
#line 251 "util/configparser.y"
    {
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 92:
#line 260 "util/configparser.y"
    {
		OUTYY(("P(server_incoming_num_tcp:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 93:
#line 269 "util/configparser.y"
    {
		OUTYY(("P(server_interface_automatic:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 94:
#line 278 "util/configparser.y"
    {
		OUTYY(("P(server_do_ip4:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 95:
#line 287 "util/configparser.y"
    {
		OUTYY(("P(server_do_ip6:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 96:
#line 296 "util/configparser.y"
    {
		OUTYY(("P(server_do_udp:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 97:
#line 305 "util/configparser.y"
    {
		OUTYY(("P(server_do_tcp:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 98:
#line 314 "util/configparser.y"
    {
		OUTYY(("P(server_do_daemonize:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 99:
#line 323 "util/configparser.y"
    {
		OUTYY(("P(server_use_syslog:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp(yyvsp[0].str, "yes")==0);
#ifndef HAVE_SYSLOG_H
		if(strcmp(yyvsp[0].str, "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free(yyvsp[0].str);
	}
    break;

  case 100:
#line 337 "util/configparser.y"
    {
		OUTYY(("P(server_chroot:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = yyvsp[0].str;
	}
    break;

  case 101:
#line 344 "util/configparser.y"
    {
		OUTYY(("P(server_username:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = yyvsp[0].str;
	}
    break;

  case 102:
#line 351 "util/configparser.y"
    {
		OUTYY(("P(server_directory:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = yyvsp[0].str;
	}
    break;

  case 103:
#line 358 "util/configparser.y"
    {
		OUTYY(("P(server_logfile:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = yyvsp[0].str;
		cfg_parser->cfg->use_syslog = 0;
	}
    break;

  case 104:
#line 366 "util/configparser.y"
    {
		OUTYY(("P(server_pidfile:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = yyvsp[0].str;
	}
    break;

  case 105:
#line 373 "util/configparser.y"
    {
		OUTYY(("P(server_root_hints:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 106:
#line 380 "util/configparser.y"
    {
		OUTYY(("P(server_trust_anchor_file:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 107:
#line 388 "util/configparser.y"
    {
		OUTYY(("P(server_trusted_keys_file:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 108:
#line 396 "util/configparser.y"
    {
		OUTYY(("P(server_trust_anchor:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 109:
#line 403 "util/configparser.y"
    {
		OUTYY(("P(server_hide_identity:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 110:
#line 412 "util/configparser.y"
    {
		OUTYY(("P(server_hide_version:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 111:
#line 421 "util/configparser.y"
    {
		OUTYY(("P(server_identity:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = yyvsp[0].str;
	}
    break;

  case 112:
#line 428 "util/configparser.y"
    {
		OUTYY(("P(server_version:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = yyvsp[0].str;
	}
    break;

  case 113:
#line 435 "util/configparser.y"
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

  case 114:
#line 446 "util/configparser.y"
    {
		OUTYY(("P(server_msg_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
    break;

  case 115:
#line 454 "util/configparser.y"
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

  case 116:
#line 467 "util/configparser.y"
    {
		OUTYY(("P(server_num_queries_per_thread:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 117:
#line 476 "util/configparser.y"
    {
		OUTYY(("P(server_rrset_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
    break;

  case 118:
#line 484 "util/configparser.y"
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

  case 119:
#line 497 "util/configparser.y"
    {
		OUTYY(("P(server_infra_host_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 120:
#line 506 "util/configparser.y"
    {
		OUTYY(("P(server_infra_lame_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->lame_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 121:
#line 515 "util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 122:
#line 524 "util/configparser.y"
    {
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->
			infra_cache_lame_size))
			yyerror("number expected");
		free(yyvsp[0].str);
	}
    break;

  case 123:
#line 533 "util/configparser.y"
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

  case 124:
#line 546 "util/configparser.y"
    {
		OUTYY(("P(server_target_fetch_policy:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = yyvsp[0].str;
	}
    break;

  case 125:
#line 553 "util/configparser.y"
    {
		OUTYY(("P(server_harden_short_bufsize:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 126:
#line 563 "util/configparser.y"
    {
		OUTYY(("P(server_harden_large_queries:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 127:
#line 573 "util/configparser.y"
    {
		OUTYY(("P(server_harden_glue:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 128:
#line 583 "util/configparser.y"
    {
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 129:
#line 593 "util/configparser.y"
    {
		OUTYY(("P(server_use_caps_for_id:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 130:
#line 603 "util/configparser.y"
    {
		OUTYY(("P(server_do_not_query_address:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 131:
#line 610 "util/configparser.y"
    {
		OUTYY(("P(server_do_not_query_localhost:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 132:
#line 620 "util/configparser.y"
    {
		OUTYY(("P(server_access_control:%s %s)\n", yyvsp[-1].str, yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "deny")!=0 && strcmp(yyvsp[0].str, "refuse")!=0 &&
			strcmp(yyvsp[0].str, "allow")!=0) {
			yyerror("expected deny, refuse or allow in "
				"access control action");
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->acls, yyvsp[-1].str, yyvsp[0].str))
				fatal_exit("out of memory adding acl");
		}
	}
    break;

  case 133:
#line 633 "util/configparser.y"
    {
		OUTYY(("P(server_module_conf:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = yyvsp[0].str;
	}
    break;

  case 134:
#line 640 "util/configparser.y"
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

  case 135:
#line 658 "util/configparser.y"
    {
		OUTYY(("P(server_cache_max_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 136:
#line 667 "util/configparser.y"
    {
		OUTYY(("P(server_bogus_ttl:%s)\n", yyvsp[0].str));
		if(atoi(yyvsp[0].str) == 0 && strcmp(yyvsp[0].str, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi(yyvsp[0].str);
		free(yyvsp[0].str);
	}
    break;

  case 137:
#line 676 "util/configparser.y"
    {
		OUTYY(("P(server_val_clean_additional:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 138:
#line 686 "util/configparser.y"
    {
		OUTYY(("P(server_val_permissive_mode:%s)\n", yyvsp[0].str));
		if(strcmp(yyvsp[0].str, "yes") != 0 && strcmp(yyvsp[0].str, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode = 
			(strcmp(yyvsp[0].str, "yes")==0);
		free(yyvsp[0].str);
	}
    break;

  case 139:
#line 696 "util/configparser.y"
    {
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = yyvsp[0].str;
	}
    break;

  case 140:
#line 703 "util/configparser.y"
    {
		OUTYY(("P(server_key_cache_size:%s)\n", yyvsp[0].str));
		if(!cfg_parse_memsize(yyvsp[0].str, &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free(yyvsp[0].str);
	}
    break;

  case 141:
#line 711 "util/configparser.y"
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

  case 142:
#line 724 "util/configparser.y"
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
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones, 
				yyvsp[-1].str, yyvsp[0].str))
				fatal_exit("out of memory adding local-zone");
		}
	}
    break;

  case 143:
#line 743 "util/configparser.y"
    {
		OUTYY(("P(server_local_data:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, yyvsp[0].str))
			fatal_exit("out of memory adding local-data");
	}
    break;

  case 144:
#line 750 "util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = yyvsp[0].str;
	}
    break;

  case 145:
#line 757 "util/configparser.y"
    {
		OUTYY(("P(stub-host:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 146:
#line 764 "util/configparser.y"
    {
		OUTYY(("P(stub-addr:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 147:
#line 771 "util/configparser.y"
    {
		OUTYY(("P(name:%s)\n", yyvsp[0].str));
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = yyvsp[0].str;
	}
    break;

  case 148:
#line 778 "util/configparser.y"
    {
		OUTYY(("P(forward-host:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;

  case 149:
#line 785 "util/configparser.y"
    {
		OUTYY(("P(forward-addr:%s)\n", yyvsp[0].str));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, yyvsp[0].str))
			yyerror("out of memory");
	}
    break;


    }

/* Line 1016 of /usr/local/share/bison/yacc.c.  */
#line 2141 "util/configparser.c"

  yyvsp -= yylen;
  yyssp -= yylen;


#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyssp > yyss)
	    {
	      YYDPRINTF ((stderr, "Error: popping "));
	      YYDSYMPRINT ((stderr,
			    yystos[*yyssp],
			    *yyvsp));
	      YYDPRINTF ((stderr, "\n"));
	      yydestruct (yystos[*yyssp], *yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yydestruct (yychar1, yylval);
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

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

      YYDPRINTF ((stderr, "Error: popping "));
      YYDSYMPRINT ((stderr,
		    yystos[*yyssp], *yyvsp));
      YYDPRINTF ((stderr, "\n"));

      yydestruct (yystos[yystate], *yyvsp);
      yyvsp--;
      yystate = *--yyssp;


#if YYDEBUG
      if (yydebug)
	{
	  short *yyssp1 = yyss - 1;
	  YYFPRINTF (stderr, "Error: state stack now");
	  while (yyssp1 != yyssp)
	    YYFPRINTF (stderr, " %d", *++yyssp1);
	  YYFPRINTF (stderr, "\n");
	}
#endif
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 791 "util/configparser.y"


/* parse helper routines could be here */

