/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
     VAR_DO_DAEMONIZE = 333
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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 64 "util/configparser.y"
{
	char*	str;
}
/* Line 1489 of yacc.c.  */
#line 209 "util/configparser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

