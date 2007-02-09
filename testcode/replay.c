/*
 * testcode/replay.c - store and use a replay of events for the DNS resolver.
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 * 
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * Store and use a replay of events for the DNS resolver.
 * Used to test known scenarios to get known outcomes.
 */

#include "config.h"
#include "util/log.h"
#include "testcode/replay.h"
#include "testcode/ldns-testpkts.h"

/** max length of lines in file */
#define MAX_LINE_LEN 10240

/** parse keyword in string. true if found, false if not.
 * if found, the line is advanced to after the keyword. */
static int parse_keyword(char** line, char* keyword)
{
	size_t len = (size_t)strlen(keyword);
	if(strncmp(*line, keyword, len) == 0) {
		*line += len;
		return 1;
	}
	return 0;
}

/** delete moment */
static void
replay_moment_delete(struct replay_moment* mom)
{
	if(!mom)
		return;
	if(mom->match) {
		delete_entry(mom->match);
	}
	free(mom);
}

/** delete range */
static void
replay_range_delete(struct replay_range* rng)
{
	if(!rng)
		return;
	delete_entry(rng->match);
	free(rng);
}

/** 
 * Read a range from file. 
 * @param remain: Rest of line (after RANGE keyword).
 * @param in: file to read from.
 * @param lineno: incremented as lines are read.
 * @param line: line buffer.
 * @param ttl: for readentry
 * @param or: for readentry
 * @param prev: for readentry
 * @return: range object to add to list, or NULL on error.
 */
static struct replay_range*
replay_range_read(char* remain, FILE* in, int* lineno, char* line,
	uint16_t* ttl, ldns_rdf** or, ldns_rdf** prev)
{
	struct replay_range* rng = (struct replay_range*)malloc(
		sizeof(struct replay_range));
	off_t pos;
	char *parse;
	struct entry* entry;
	if(!rng)
		return NULL;
	memset(rng, 0, sizeof(*rng));
	/* read time range */
	if(sscanf(remain, " %d %d", &rng->start_step, &rng->end_step)!=2) {
		log_err("Could not read time range: %s", line);
		free(rng);
		return NULL;
	}
	/* read entries */
	pos = ftello(in);
	while(fgets(line, MAX_LINE_LEN-1, in)) {
		(*lineno)++;
		parse = line;
		while(isspace(*parse))
			parse++;
		if(!*parse || *parse == ';')
			continue;
		if(parse_keyword(&parse, "RANGE_END")) {
			return rng;
		}
		/* set position before line; read entry */
		(*lineno)--;
		fseeko(in, pos, SEEK_SET);
		entry = read_entry(in, "datafile", lineno, ttl, or, prev);
		if(!entry)
			fatal_exit("%d: bad entry", *lineno);
		entry->next = rng->match;
		rng->match = entry;

		pos = ftello(in);
	}
	replay_range_delete(rng);
	return NULL;
}

/** 
 * Read a replay moment 'STEP' from file. 
 * @param remain: Rest of line (after STEP keyword).
 * @param in: file to read from.
 * @param lineno: incremented as lines are read.
 * @param ttl: for readentry
 * @param or: for readentry
 * @param prev: for readentry
 * @return: range object to add to list, or NULL on error.
 */
static struct replay_moment*
replay_moment_read(char* remain, FILE* in, int* lineno, 
	uint16_t* ttl, ldns_rdf** or, ldns_rdf** prev)
{
	struct replay_moment* mom = (struct replay_moment*)malloc(
		sizeof(struct replay_moment));
	int skip = 0;
	int readentry = 0;
	if(!mom)
		return NULL;
	memset(mom, 0, sizeof(*mom));
	if(sscanf(remain, " %d%n", &mom->time_step, &skip) != 1) {
		log_err("%d: cannot read value: %s", *lineno, remain);
		free(mom);
		return NULL;
	}
	remain += skip;
	while(isspace(*remain))
		remain++;
	if(parse_keyword(&remain, "NOTHING")) {
		mom->evt_type = repevt_nothing;
	} else if(parse_keyword(&remain, "QUERY")) {
		mom->evt_type = repevt_front_query;
		readentry = 1;
	} else if(parse_keyword(&remain, "CHECK_ANSWER")) {
		mom->evt_type = repevt_front_reply;
		readentry = 1;
	} else if(parse_keyword(&remain, "CHECK_OUT_QUERY")) {
		mom->evt_type = repevt_back_query;
		readentry = 1;
	} else if(parse_keyword(&remain, "REPLY")) {
		mom->evt_type = repevt_back_reply;
		readentry = 1;
	} else if(parse_keyword(&remain, "TIMEOUT")) {
		mom->evt_type = repevt_timeout;
	} else if(parse_keyword(&remain, "ERROR")) {
		mom->evt_type = repevt_back_query;
	} else {
		log_err("%d: unknown event type %s", *lineno, remain);
		free(mom);
		return NULL;
	}

	if(readentry) {
		mom->match = read_entry(in, "datafile", lineno, ttl, or, prev);
		free(mom);
		if(!mom->match)
			return NULL;
	}

	return mom;
}

/** makes scenario with title on rest of line. */
static struct replay_scenario*
make_scenario(char* line)
{
	struct replay_scenario* scen;
	while(isspace(*line))
		line++;
	if(!*line) {
		log_err("scenario: no title given");
		return NULL;
	}
	scen = (struct replay_scenario*)malloc(sizeof(struct replay_scenario));
	if(!scen)
		return NULL;
	memset(scen, 0, sizeof(*scen));
	scen->title = strdup(line);
	if(!scen->title) {
		free(scen);
		return NULL;
	}
	return scen;
}

struct replay_scenario* 
replay_scenario_read(FILE* in)
{
	char line[MAX_LINE_LEN];
	char *parse;
	int lineno = 0;
	struct replay_scenario* scen = NULL;
	uint16_t ttl = 3600;
	ldns_rdf* or = NULL;
	ldns_rdf* prev = NULL;
	line[MAX_LINE_LEN-1]=0;

	while(fgets(line, sizeof(line)-1, in)) {
		parse=line;
		lineno++;
		while(isspace(*parse))
			parse++;
		if(!*parse) 
			continue; /* empty line */
		if(parse_keyword(&parse, ";"))
			continue; /* comment */
		if(parse_keyword(&parse, "SCENARIO_BEGIN")) {
			scen = make_scenario(parse);
			if(!scen)
				fatal_exit("%d: could not make scen", lineno);
			continue;
		} 
		if(!scen)
			fatal_exit("%d: expected SCENARIO", lineno);
		if(parse_keyword(&parse, "RANGE_BEGIN")) {
			struct replay_range* newr = replay_range_read(
				parse, in, &lineno, line, &ttl, &or, &prev);
			if(!newr)
				fatal_exit("%d: bad range", lineno);
			newr->next_range = scen->range_list;
			scen->range_list = newr;
		} else if(parse_keyword(&parse, "STEP")) {
			struct replay_moment* mom = replay_moment_read(
				parse, in, &lineno, &ttl, &or, &prev);
			if(!mom)
				fatal_exit("%d: bad moment", lineno);
			if(scen->mom_last)
				scen->mom_last->mom_next = mom;
			else	scen->mom_first = mom;
			scen->mom_last = mom;
		} else if(parse_keyword(&parse, "SCENARIO_END")) {
			return scen;
		}
	}
	replay_scenario_delete(scen);
	return NULL;
}

void 
replay_scenario_delete(struct replay_scenario* scen)
{
	struct replay_moment* mom, *momn;
	struct replay_range* rng, *rngn;
	if(!scen)
		return;
	if(scen->title)
		free(scen->title);
	mom = scen->mom_first;
	while(mom) {
		momn = mom->mom_next;
		replay_moment_delete(mom);
		mom = momn;
	}
	rng = scen->range_list;
	while(rng) {
		rngn = rng->next_range;
		replay_range_delete(rng);
		rng = rngn;
	}
	free(scen);
}
