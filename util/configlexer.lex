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

#define YY_NO_UNPUT

#if 0
#define LEXOUT(s)  printf s /* used ONLY when debugging */
#else
#define LEXOUT(s)
#endif

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

SPACE   [ \t]
LETTER  [a-zA-Z]
UNQUOTEDLETTER [^\"\n\r \t\\]|\\.
NEWLINE [\r\n]
COMMENT \#
COLON 	\:
ANY     [^\"\n\r\\]|\\.

%x	quotedstring include include_quoted

%%
{SPACE}* 		{ LEXOUT(("SP ")); /* ignore */ }
{SPACE}*{COMMENT}.* 	{ LEXOUT(("comment(%s) ", yytext)); /* ignore */ }
server{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_SERVER;}
num-threads{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_NUM_THREADS;}
verbosity{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_VERBOSITY;}
port{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_PORT;}
outgoing-port{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_OUTGOING_PORT;}
outgoing-range{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_OUTGOING_RANGE;}
do-ip4{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_DO_IP4;}
do-ip6{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_DO_IP6;}
do-udp{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_DO_UDP;}
do-tcp{COLON}		{ LEXOUT(("v(%s) ", yytext)); return VAR_DO_TCP;}
forward-to{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_FORWARD_TO;}
forward-to-port{COLON}	{ LEXOUT(("v(%s) ", yytext)); return VAR_FORWARD_TO_PORT;}
{NEWLINE}		{ LEXOUT(("NL\n")); cfg_parser->line++;}

	/* Quoted strings. Strip leading and ending quotes */
\"			{ BEGIN(quotedstring); LEXOUT(("QS ")); }
<quotedstring><<EOF>>   {
        yyerror("EOF inside quoted string");
        BEGIN(INITIAL);
}
<quotedstring>{ANY}*    { LEXOUT(("STR(%s) ", yytext)); yymore(); }
<quotedstring>\n        { cfg_parser->line++; yymore(); }
<quotedstring>\" {
        LEXOUT(("QE "));
        BEGIN(INITIAL);
        yytext[yyleng - 1] = '\0';
	yylval.str = strdup(yytext);
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
<include_quoted>{ANY}*	{ LEXOUT(("ISTR(%s) ", yytext)); yymore(); }
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
