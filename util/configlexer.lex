%{
/*
 * configlexer.lex - lexical analyzer for unbound config file
 *
 * Copyright (c) 2001-2006, NLnet Labs. All rights reserved
 *
 * See LICENSE for the license.
 *
 */

#include "config.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/configparser.h"
void ub_c_error(const char *message);

#if 0
#define LEXOUT(s)  printf s /* used ONLY when debugging */
#else
#define LEXOUT(s)
#endif

#define YDOUT LEXOUT(("v(%s )", yytext))

struct inc_state {
	char* filename;
	int line;
};
static struct inc_state parse_stack[MAXINCLUDES];
static YY_BUFFER_STATE include_stack[MAXINCLUDES];
static int config_include_stack_ptr = 0;

static void config_start_include(const char* filename)
{
	FILE *input;
	if(strlen(filename) == 0) {
		ub_c_error_msg("empty include file name");
		return;
	}
	if(config_include_stack_ptr >= MAXINCLUDES) {
		ub_c_error_msg("includes nested too deeply, skipped (>%d)", MAXINCLUDES);
		return;
	}
	input = fopen(filename, "r");
	if(!input) {
		ub_c_error_msg("cannot open include file '%s': %s",
			filename, strerror(errno));
		return;
	}
	LEXOUT(("switch_to_include_file(%s) ", filename));
	parse_stack[config_include_stack_ptr].filename = cfg_parser->filename;
	parse_stack[config_include_stack_ptr].line = cfg_parser->line;
	include_stack[config_include_stack_ptr] = YY_CURRENT_BUFFER;
	cfg_parser->filename = strdup(filename);
	cfg_parser->line = 1;
	yy_switch_to_buffer(yy_create_buffer(input, YY_BUF_SIZE));
	++config_include_stack_ptr;
}

static void config_end_include(void)
{
	--config_include_stack_ptr;
	free(cfg_parser->filename);
	cfg_parser->filename = parse_stack[config_include_stack_ptr].filename;
	cfg_parser->line = parse_stack[config_include_stack_ptr].line;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer(include_stack[config_include_stack_ptr]);
}

#ifndef yy_set_bol /* compat definition, for flex 2.4.6 */
#define yy_set_bol(at_bol) \
        { \
	        if ( ! yy_current_buffer ) \
	                yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE ); \
	        yy_current_buffer->yy_ch_buf[0] = ((at_bol)?'\n':' '); \
        }
#endif

%}
%option noinput
%option nounput
%{
#ifndef YY_NO_UNPUT
#define YY_NO_UNPUT 1
#endif
#ifndef YY_NO_INPUT
#define YY_NO_INPUT 1
#endif
%}

SPACE   [ \t]
LETTER  [a-zA-Z]
UNQUOTEDLETTER [^\'\"\n\r \t\\]|\\.
NEWLINE [\r\n]
COMMENT \#
COLON 	\:
DQANY     [^\"\n\r\\]|\\.
SQANY     [^\'\n\r\\]|\\.

%x	quotedstring singlequotedstr include include_quoted

%%
{SPACE}* 		{ LEXOUT(("SP ")); /* ignore */ }
{SPACE}*{COMMENT}.* 	{ LEXOUT(("comment(%s) ", yytext)); /* ignore */ }
server{COLON}		{ YDOUT; return VAR_SERVER;}
num-threads{COLON}	{ YDOUT; return VAR_NUM_THREADS;}
verbosity{COLON}	{ YDOUT; return VAR_VERBOSITY;}
port{COLON}		{ YDOUT; return VAR_PORT;}
outgoing-range{COLON}	{ YDOUT; return VAR_OUTGOING_RANGE;}
outgoing-port-permit{COLON}	{ YDOUT; return VAR_OUTGOING_PORT_PERMIT;}
outgoing-port-avoid{COLON}	{ YDOUT; return VAR_OUTGOING_PORT_AVOID;}
outgoing-num-tcp{COLON}	{ YDOUT; return VAR_OUTGOING_NUM_TCP;}
incoming-num-tcp{COLON}	{ YDOUT; return VAR_INCOMING_NUM_TCP;}
do-ip4{COLON}		{ YDOUT; return VAR_DO_IP4;}
do-ip6{COLON}		{ YDOUT; return VAR_DO_IP6;}
do-udp{COLON}		{ YDOUT; return VAR_DO_UDP;}
do-tcp{COLON}		{ YDOUT; return VAR_DO_TCP;}
do-daemonize{COLON}		{ YDOUT; return VAR_DO_DAEMONIZE;}
interface{COLON}	{ YDOUT; return VAR_INTERFACE;}
outgoing-interface{COLON}	{ YDOUT; return VAR_OUTGOING_INTERFACE;}
interface-automatic{COLON}	{ YDOUT; return VAR_INTERFACE_AUTOMATIC;}
chroot{COLON}		{ YDOUT; return VAR_CHROOT;}
username{COLON}		{ YDOUT; return VAR_USERNAME;}
directory{COLON}	{ YDOUT; return VAR_DIRECTORY;}
logfile{COLON}		{ YDOUT; return VAR_LOGFILE;}
pidfile{COLON}		{ YDOUT; return VAR_PIDFILE;}
root-hints{COLON}	{ YDOUT; return VAR_ROOT_HINTS;}
msg-buffer-size{COLON}	{ YDOUT; return VAR_MSG_BUFFER_SIZE;}
msg-cache-size{COLON}	{ YDOUT; return VAR_MSG_CACHE_SIZE;}
msg-cache-slabs{COLON}	{ YDOUT; return VAR_MSG_CACHE_SLABS;}
rrset-cache-size{COLON}	{ YDOUT; return VAR_RRSET_CACHE_SIZE;}
rrset-cache-slabs{COLON}	{ YDOUT; return VAR_RRSET_CACHE_SLABS;}
cache-max-ttl{COLON}     	{ YDOUT; return VAR_CACHE_MAX_TTL;}
infra-host-ttl{COLON}	{ YDOUT; return VAR_INFRA_HOST_TTL;}
infra-lame-ttl{COLON}	{ YDOUT; return VAR_INFRA_LAME_TTL;}
infra-cache-slabs{COLON}	{ YDOUT; return VAR_INFRA_CACHE_SLABS;}
infra-cache-numhosts{COLON}	{ YDOUT; return VAR_INFRA_CACHE_NUMHOSTS;}
infra-cache-lame-size{COLON}	{ YDOUT; return VAR_INFRA_CACHE_LAME_SIZE;}
num-queries-per-thread{COLON}	{ YDOUT; return VAR_NUM_QUERIES_PER_THREAD;}
target-fetch-policy{COLON}	{ YDOUT; return VAR_TARGET_FETCH_POLICY;}
harden-short-bufsize{COLON}	{ YDOUT; return VAR_HARDEN_SHORT_BUFSIZE;}
harden-large-queries{COLON}	{ YDOUT; return VAR_HARDEN_LARGE_QUERIES;}
harden-glue{COLON}	{ YDOUT; return VAR_HARDEN_GLUE;}
harden-dnssec-stripped{COLON}	{ YDOUT; return VAR_HARDEN_DNNSEC_STRIPPED;}
harden-referral-path{COLON}	{ YDOUT; return VAR_HARDEN_REFERRAL_PATH;}
use-caps-for-id{COLON}	{ YDOUT; return VAR_USE_CAPS_FOR_ID;}
private-address{COLON}	{ YDOUT; return VAR_PRIVATE_ADDRESS;}
private-domain{COLON}	{ YDOUT; return VAR_PRIVATE_DOMAIN;}
stub-zone{COLON}	{ YDOUT; return VAR_STUB_ZONE;}
name{COLON}		{ YDOUT; return VAR_NAME;}
stub-addr{COLON}	{ YDOUT; return VAR_STUB_ADDR;}
stub-host{COLON}	{ YDOUT; return VAR_STUB_HOST;}
forward-zone{COLON}	{ YDOUT; return VAR_FORWARD_ZONE;}
forward-addr{COLON}	{ YDOUT; return VAR_FORWARD_ADDR;}
forward-host{COLON}	{ YDOUT; return VAR_FORWARD_HOST;}
do-not-query-address{COLON}	{ YDOUT; return VAR_DO_NOT_QUERY_ADDRESS;}
do-not-query-localhost{COLON}	{ YDOUT; return VAR_DO_NOT_QUERY_LOCALHOST;}
access-control{COLON}	{ YDOUT; return VAR_ACCESS_CONTROL;}
hide-identity{COLON}	{ YDOUT; return VAR_HIDE_IDENTITY;}
hide-version{COLON}     { YDOUT; return VAR_HIDE_VERSION;}
identity{COLON}		{ YDOUT; return VAR_IDENTITY;}
version{COLON}     	{ YDOUT; return VAR_VERSION;}
module-config{COLON}     	{ YDOUT; return VAR_MODULE_CONF;}
dlv-anchor{COLON}	{ YDOUT; return VAR_DLV_ANCHOR;}
dlv-anchor-file{COLON}	{ YDOUT; return VAR_DLV_ANCHOR_FILE;}
trust-anchor-file{COLON}	{ YDOUT; return VAR_TRUST_ANCHOR_FILE;}
trusted-keys-file{COLON}	{ YDOUT; return VAR_TRUSTED_KEYS_FILE;}
trust-anchor{COLON}	{ YDOUT; return VAR_TRUST_ANCHOR;}
val-override-date{COLON}	{ YDOUT; return VAR_VAL_OVERRIDE_DATE;}
val-bogus-ttl{COLON}	{ YDOUT; return VAR_BOGUS_TTL;}
val-clean-additional{COLON}	{ YDOUT; return VAR_VAL_CLEAN_ADDITIONAL;}
val-permissive-mode{COLON}	{ YDOUT; return VAR_VAL_PERMISSIVE_MODE;}
key-cache-size{COLON}	{ YDOUT; return VAR_KEY_CACHE_SIZE;}
key-cache-slabs{COLON}	{ YDOUT; return VAR_KEY_CACHE_SLABS;}
neg-cache-size{COLON}	{ YDOUT; return VAR_NEG_CACHE_SIZE;}
val-nsec3-keysize-iterations{COLON}	{ YDOUT; return VAR_VAL_NSEC3_KEYSIZE_ITERATIONS;}
use-syslog{COLON}	{ YDOUT; return VAR_USE_SYSLOG;}
local-zone{COLON}	{ YDOUT; return VAR_LOCAL_ZONE;}
local-data{COLON}	{ YDOUT; return VAR_LOCAL_DATA;}
statistics-interval{COLON}	{ YDOUT; return VAR_STATISTICS_INTERVAL;}
statistics-cumulative{COLON}	{ YDOUT; return VAR_STATISTICS_CUMULATIVE;}
remote-control{COLON}	{ YDOUT; return VAR_REMOTE_CONTROL; }
control-enable{COLON}	{ YDOUT; return VAR_CONTROL_ENABLE; }
control-interface{COLON}	{ YDOUT; return VAR_CONTROL_INTERFACE; }
control-port{COLON}	{ YDOUT; return VAR_CONTROL_PORT; }
server-key-file{COLON}	{ YDOUT; return VAR_SERVER_KEY_FILE; }
server-cert-file{COLON}	{ YDOUT; return VAR_SERVER_CERT_FILE; }
control-key-file{COLON}	{ YDOUT; return VAR_CONTROL_KEY_FILE; }
control-cert-file{COLON}	{ YDOUT; return VAR_CONTROL_CERT_FILE; }
{NEWLINE}		{ LEXOUT(("NL\n")); cfg_parser->line++;}

	/* Quoted strings. Strip leading and ending quotes */
\"			{ BEGIN(quotedstring); LEXOUT(("QS ")); }
<quotedstring><<EOF>>   {
        yyerror("EOF inside quoted string");
        BEGIN(INITIAL);
}
<quotedstring>{DQANY}*  { LEXOUT(("STR(%s) ", yytext)); yymore(); }
<quotedstring>\n        { cfg_parser->line++; yymore(); }
<quotedstring>\" {
        LEXOUT(("QE "));
        BEGIN(INITIAL);
        yytext[yyleng - 1] = '\0';
	yylval.str = strdup(yytext);
	if(!yylval.str)
		yyerror("out of memory");
        return STRING;
}

	/* Single Quoted strings. Strip leading and ending quotes */
\'			{ BEGIN(singlequotedstr); LEXOUT(("SQS ")); }
<singlequotedstr><<EOF>>   {
        yyerror("EOF inside quoted string");
        BEGIN(INITIAL);
}
<singlequotedstr>{SQANY}*  { LEXOUT(("STR(%s) ", yytext)); yymore(); }
<singlequotedstr>\n        { cfg_parser->line++; yymore(); }
<singlequotedstr>\' {
        LEXOUT(("SQE "));
        BEGIN(INITIAL);
        yytext[yyleng - 1] = '\0';
	yylval.str = strdup(yytext);
	if(!yylval.str)
		yyerror("out of memory");
        return STRING;
}

	/* include: directive */
include{COLON}		{ LEXOUT(("v(%s) ", yytext)); BEGIN(include); }
<include><<EOF>>	{
        yyerror("EOF inside include directive");
        BEGIN(INITIAL);
}
<include>{SPACE}*	{ LEXOUT(("ISP ")); /* ignore */ }
<include>{NEWLINE}	{ LEXOUT(("NL\n")); cfg_parser->line++;}
<include>\"		{ LEXOUT(("IQS ")); BEGIN(include_quoted); }
<include>{UNQUOTEDLETTER}*	{
	LEXOUT(("Iunquotedstr(%s) ", yytext));
	config_start_include(yytext);
	BEGIN(INITIAL);
}
<include_quoted><<EOF>>	{
        yyerror("EOF inside quoted string");
        BEGIN(INITIAL);
}
<include_quoted>{DQANY}*	{ LEXOUT(("ISTR(%s) ", yytext)); yymore(); }
<include_quoted>{NEWLINE}	{ cfg_parser->line++; yymore(); }
<include_quoted>\"	{
	LEXOUT(("IQE "));
	yytext[yyleng - 1] = '\0';
	config_start_include(yytext);
	BEGIN(INITIAL);
}
<INITIAL><<EOF>>	{
	yy_set_bol(1); /* Set beginning of line, so "^" rules match.  */
	if (config_include_stack_ptr == 0) {
		yyterminate();
	} else {
		fclose(yyin);
		config_end_include();
	}
}

{UNQUOTEDLETTER}*	{ LEXOUT(("unquotedstr(%s) ", yytext)); 
			yylval.str = strdup(yytext); return STRING; }

%%
