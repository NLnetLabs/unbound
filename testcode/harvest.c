/*
 * testcode/harvest.c - debug program to get relevant data to a set of queries.
 *
 * Copyright (c) 2008, NLnet Labs. All rights reserved.
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
 *
 * This program downloads relevant DNS data to a set of queries.
 * This means that the queries are asked to root, TLD, SLD servers and
 * the results stored per zone.
 * The following data is pertinent:
 *
 * At each label:
 *	SOA
 *	NS
 *	DNSKEY
 *	DS
 * For the whole query:
 *	the result.
 * For NS-records:
 *	their label data
 *	and the A and AAAA records for it.
 *	(as if the name, with A and AAAA query type is in the list,
 *	 referred to as recursion depth+1)
 * Any NSEC, NSEC3, SOA records or additional data found in answers.
 *
 * All of this is data that would be encountered during an iterative lookup
 * for the queries in the list. It is saved to enable a replay of iterative
 * lookups for performance testing.
 *
 * A number of assumptions are made.
 * 1) configuration is correct.
 *    The parent has the same NS records as the child.
 *    All nameservers carry the same data.
 * 2) EDNS/nonEDNS responses and other behaviour is ignored.
 *    Only the data is saved.
 * This creates a snapshot that represents the data as this resolver saw it.
 */

#include <signal.h>
#include "config.h"
#include "libunbound/unbound.h"
struct todo_item;
struct labdata;

/** this represents the data that has been collected 
 * as well as a todo list and some settings */
struct harvest_data {
	/** the unbound context */
	struct ub_ctx* ctx;

	/** a tree per label; thus this first one is one root entry,
	 * that has a tree of TLD labels. Those have trees of SLD labels. */
	struct labdata* root;
	/** the original query list */
	struct todo_item* orig_list;
	/** the query list todo */
	struct todo_item* todo_list;
	/** last item in todo list */
	struct todo_item* todo_last;
	/** number of todo items */
	int numtodo;

	/** where to store the results */
	char* resultdir;
	/** maximum recursion depth */
	int maxdepth;
	/** current recursion depth */
	int curdepth;

	/** max depth of labels */
	int maxlabels;
};

/**
 * Todo item
 */
struct todo_item {
	/** the next item */
	struct todo_item* next;

	/** query as rdf */
	ldns_rdf* qname;
	/** the query type */
	int qtype;
	/** query class */
	int qclass;

	/** recursion depth of todo item (orig list is 0) */
	int depth;
	/** the label associated with the query */
	struct labdata* lab;
};

/** 
 * Every label has a sest of sublabels, that have sets of sublabels ...
 * Per label is stored also a set of data items, and todo information
 */
struct labdata {
	/** node in ldns rbtree */
	ldns_rbnode_t node;
	/** the name of this label */
	ldns_rdf* label;
	/** full name of point in domain tree */
	ldns_rdf* name;

	/** parent in label tree (NULL for root) */
	struct labdata* parent;
	/** tree of sublabels (if any) */
	ldns_rbtree_t* sublabels;

	/** list of RRs for this label */
	ldns_rr_list* rrlist;
	/** have queries for this label been queued */
	int done;
};

/** usage information for harvest */
static void usage(char* nm) 
{
	printf("usage: %s [options]\n", nm);
	printf("-f fnm	query list to read from file\n");
	printf("	every line has format: qname qclass qtype\n");
	exit(1);
}

/** verbosity for harvest */
static int hverb = 1;

/** exit with error */
static void error_exit(char* str)
{
	printf("error: %s\n", str);
	exit(1);
}

/** read a query file */
static void
qlist_read_file(struct harvest_data* data, char* fname)
{
	char buf[1024];
	char nm[1024], cl[1024], tp[1024];
	int r;
	int num = 0;
	FILE* in = fopen(fname, "r");
	struct todo_item* t;
	if(!in) {
		perror(fname);
		error_exit("could not open file");
	}
	while(fgets(buf, (int)sizeof(buf), in)) {
		if(buf[0] == 0) continue;
		if(buf[0] == '\n') continue;
		/* allow some comments */
		if(buf[0] == ';') continue;
		if(buf[0] == '#') continue;
		nm[0] = 0; cl[0] = 0; tp[0] = 0;
		r = sscanf(buf, " %1023s %1023s %1023s", nm, cl, tp);
		if(r == 0) continue;
		t = (struct todo_item*)calloc(1, sizeof(*t));
		if(!t) error_exit("out of memory");
		t->qname = ldns_dname_new_frm_str(nm);
		if(!t->qname) {
			printf("parse error: %s\n", nm);
			error_exit("bad qname");
		}
		t->depth = 0;
		t->qtype = LDNS_RR_TYPE_A;
		t->qclass = LDNS_RR_CLASS_IN;
		if(r >= 2) {
			if(strcmp(cl, "IN") == 0 || strcmp(cl, "CH") == 0)
				t->qclass = ldns_get_rr_class_by_name(cl);
			else	t->qtype = ldns_get_rr_type_by_name(cl);
		}
		if(r >= 3) {
			if(strcmp(tp, "IN") == 0 || strcmp(tp, "CH") == 0)
				t->qclass = ldns_get_rr_class_by_name(tp);
			else	t->qtype = ldns_get_rr_type_by_name(tp);
		}
		num++;

		t->next = data->orig_list;
		data->orig_list = t;
	}
	printf("read %s: %d queries\n", fname, num);
	fclose(in);
}

/** compare two labels */
static int
lab_cmp(const void *x, const void *y)
{
	return ldns_dname_compare((const ldns_rdf*)x, (const ldns_rdf*)y);
}

/** create label entry */
static struct labdata*
lab_create(char* name)
{
	struct labdata* lab = (struct labdata*)calloc(1, sizeof(*lab));
	if(!lab) error_exit("out of memory");
	lab->label = ldns_dname_new_frm_str(name);
	if(!lab->label) error_exit("out of memory");
	lab->name = ldns_dname_new_frm_str(name);
	if(!lab->name) error_exit("out of memory");
	lab->node.key = lab->label;
	lab->node.data = lab;
	lab->sublabels = ldns_rbtree_create(lab_cmp);
	if(!lab->sublabels) error_exit("out of memory");
	lab->rrlist = ldns_rr_list_new();
	if(!lab->rrlist) error_exit("out of memory");

	return lab;
}

/** for this name, lookup the label, create if does not exist */
static struct labdata*
find_create_lab(struct harvest_data* data, ldns_rdf* name)
{
	struct labdata* lab = data->root;
	struct labdata* nextlab;
	ldns_rdf* next;
	uint8_t numlab = ldns_dname_label_count(name);
	if((int)numlab > data->maxlabels)
		data->maxlabels = (int)numlab;
	while(numlab--) {
		next = ldns_dname_label(name, numlab);
		if(!next) error_exit("ldns_dname_label");
		
		nextlab = (struct labdata*)
			ldns_rbtree_search(lab->sublabels, next);
		if(!nextlab) {
			/* create it */
			nextlab = (struct labdata*)calloc(1, sizeof(*lab));
			if(!nextlab) error_exit("out of memory");
			nextlab->label = ldns_rdf_clone(next);
			if(!nextlab->label) error_exit("out of memory");
			nextlab->node.key = nextlab->label;
			nextlab->node.data = nextlab;
			nextlab->sublabels = ldns_rbtree_create(lab_cmp);
			if(!nextlab->sublabels) error_exit("out of memory");
			nextlab->parent = lab;
			nextlab->name = ldns_rdf_clone(next);
			if(!nextlab->name) error_exit("out of memory");
			if(ldns_dname_cat(nextlab->name, lab->name) 
				!= LDNS_STATUS_OK) error_exit("outofmem");
			nextlab->rrlist = ldns_rr_list_new();
			if(!nextlab->rrlist) error_exit("out of memory");
			(void)ldns_rbtree_insert(lab->sublabels, 
				&nextlab->node);
			if(hverb) {
				printf("new label: ");
				ldns_rdf_print(stdout, nextlab->name);
				printf("\n");
			}
		}
		lab = nextlab;
		ldns_rdf_deep_free(next);
	}
	return lab;
}

/** for given query, create todo items, and labels if needed */
static void
new_todo_item(struct harvest_data* data, ldns_rdf* qname, int qtype, 
	int qclass, int depth)
{
	struct labdata* lab = find_create_lab(data, qname);
	struct todo_item* it;
	if(!lab) error_exit("out of memory creating new label");
	it = (struct todo_item*)calloc(1, sizeof(*it));
	it->qname = ldns_rdf_clone(qname);
	it->qtype = qtype;
	it->qclass = qclass;
	it->depth = depth;
	it->lab = lab;
	it->next = NULL;
	if(data->todo_last)
		data->todo_last->next = it;
	else data->todo_list = it;
	data->todo_last = it;
	data->numtodo ++;
	if(hverb) {
		printf("new todo: ");
		ldns_rdf_print(stdout, it->qname);
		if(ldns_rr_descript((uint16_t)it->qtype) && 
			ldns_rr_descript((uint16_t)it->qtype)->_name)
			printf(" %s", ldns_rr_descript((uint16_t)
			it->qtype)->_name);
		if(ldns_lookup_by_id(ldns_rr_classes, it->qclass) && 
			ldns_lookup_by_id(ldns_rr_classes, it->qclass)->name) 
			printf(" %s", ldns_lookup_by_id(ldns_rr_classes, 
				it->qclass)->name);
		printf("\n");
	}
}

/** add infra todo items for this query */
static void
new_todo_infra(struct harvest_data* data, struct todo_item* it)
{
	struct labdata* lab;
	for(lab = it->lab; lab; lab = lab->parent) {
		if(lab->done)
			return;
		new_todo_item(data, lab->name, LDNS_RR_TYPE_NS, 
			LDNS_RR_CLASS_IN, it->depth);
		new_todo_item(data, lab->name, LDNS_RR_TYPE_SOA, 
			LDNS_RR_CLASS_IN, it->depth);
		new_todo_item(data, lab->name, LDNS_RR_TYPE_DNSKEY, 
			LDNS_RR_CLASS_IN, it->depth);
		new_todo_item(data, lab->name, LDNS_RR_TYPE_DS, 
			LDNS_RR_CLASS_IN, it->depth);
		lab->done = 1;
	}
}

/** make todo items for initial data */
static void
make_todo(struct harvest_data* data)
{
	struct todo_item* it;
	for(it=data->orig_list; it; it = it->next) {
		/* create todo item for this query itself */
		new_todo_item(data, it->qname, it->qtype, it->qclass, 0);
		/* create todo items for infra queries to support it */
		new_todo_infra(data, data->todo_list);
	}
}

/** get result and store it */
static void
process(struct harvest_data* data, struct todo_item* it)
{
	if(hverb) {
		printf("process: ");
		ldns_rdf_print(stdout, it->qname);
		if(ldns_rr_descript((uint16_t)it->qtype) && 
			ldns_rr_descript((uint16_t)it->qtype)->_name)
			printf(" %s", ldns_rr_descript((uint16_t)
			it->qtype)->_name);
		if(ldns_lookup_by_id(ldns_rr_classes, it->qclass) && 
			ldns_lookup_by_id(ldns_rr_classes, it->qclass)->name) 
			printf(" %s", ldns_lookup_by_id(ldns_rr_classes, 
				it->qclass)->name);
		printf("\n");
	}
	/* do lookup */


	/* create ldns pkt */
	/* create recursive todo items */
	/* store results */
}

/** perform main harvesting */
static void
harvest_main(struct harvest_data* data)
{
	struct todo_item* it;
	/* register todo queries for all original queries */
	make_todo(data);
	printf("depth 0: todo list %d\n", data->numtodo);
	/* pick up a todo item and process it */
	while(data->todo_list) {
		it = data->todo_list;
		data->todo_list = it->next;
		if(!data->todo_list) data->todo_last = NULL;
		if(it->depth > data->curdepth) {
			data->curdepth = it->depth;
			printf("depth %d: todo list %d\n", it->depth,
				data->numtodo);
		}
		data->numtodo--;
		process(data, it);
	}
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** main program for harvest */
int main(int argc, char* argv[]) 
{
	struct harvest_data data;
	char* nm = argv[0];
	struct ub_ctx* ctx = ub_ctx_create();
	int c;

	/* defaults */
	memset(&data, 0, sizeof(data));
	data.resultdir = strdup("harvested_zones");
	if(!data.resultdir) error_exit("out of memory");
	data.maxdepth = 10;

	/* parse the options */
	while( (c=getopt(argc, argv, "hf:")) != -1) {
		switch(c) {
		case 'f':
			qlist_read_file(&data, optarg);
			break;
		case '?':
		case 'h':
		default:
			usage(nm);
		}
	}
	argc -= optind;
	argv += optind;
	if(argc != 0)
		usage(nm);
	if(data.orig_list == NULL)
		error_exit("No queries to make, use -f (help with -h).");
	data.root = lab_create(".");
	if(!data.root) error_exit("out of memory");
	
	/* harvest the data */
	harvest_main(&data);

	/* no cleanup except the context (to close open sockets) */
	ub_ctx_delete(ctx);
	return 0;
}
