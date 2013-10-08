/*
 * addrtree -- radix tree for vandergaast cache.
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */

/** \file 
 * see addrtree.h 
 */
#include "config.h"
#include "util/log.h"
#include "util/data/msgreply.h"
#include "util/module.h"
#include "addrtree.h"

/** 
 * Create a new edge
 * @param node: Child node this edge will connect to.
 * @param addr: full key to this edge.
 * @param addrlen: length of relevant part of key for this node
 * @return new addredge or NULL on failure
 */
static struct addredge* 
edge_create(struct addrnode* node, const addrkey_t* addr, addrlen_t addrlen)
{
	size_t n;
	struct addredge* edge = (struct addredge*)malloc( sizeof (*edge) );
	if (!edge)
		return NULL;
	edge->node = node;
	edge->len = addrlen;
	n = (size_t)((addrlen / KEYWIDTH) + ((addrlen % KEYWIDTH != 0)?1:0)); /*ceil()*/
	edge->str = (addrkey_t*)calloc(n, sizeof (addrkey_t));
	if (!edge->str) {
		free(edge);
		return NULL;
	}
	memcpy(edge->str, addr, n * sizeof (addrkey_t));
	return edge;
}

/** 
 * Create a new node
 * @param elem: Element to store at this node
 * @param scope: Scopemask from server reply
 * @param ttl: Element is valid up to this time. Absolute, seconds
 * @return new addrnode or NULL on failure
 */
static struct addrnode * 
node_create(struct addrtree *tree, void* elem, addrlen_t scope, 
	time_t ttl)
{
	struct addrnode* node = (struct addrnode*)malloc( sizeof (*node) );
	if (!node)
		return NULL;
	node->elem = elem;
	if (elem) tree->elem_count++;
	node->scope = scope;
	node->ttl = ttl;
	node->edge[0] = NULL;
	node->edge[1] = NULL;
	return node;
}

struct addrtree* addrtree_create(addrlen_t max_depth, 
	void (*delfunc)(void *, void *), size_t (*sizefunc)(void *), void *env)
{
	struct addrtree* tree;
	log_assert(delfunc != NULL);
	log_assert(sizefunc != NULL);
	tree = (struct addrtree*)malloc( sizeof (*tree) );
	if(!tree)
		return NULL;
	tree->root = node_create(tree, NULL, 0, 0);
	if (!tree->root) {
		free(tree);
		return NULL;
	}
	tree->max_depth = max_depth;
	tree->delfunc = delfunc;
	tree->sizefunc = sizefunc;
	tree->env = env;
	tree->elem_count = 0;
	return tree;
}

/**
 * Recursively calculate size of tree from this node on downwards.
 * */
static size_t tree_size(const struct addrtree* tree, 
	const struct addrnode* node)
{
	size_t s, i;
	
	if (!node) return 0;
	s = sizeof(struct addrnode);
	if (node->elem && tree->sizefunc)
		s += tree->sizefunc(node->elem);
	for (i = 0; i < 2; i++) {
		if (!node->edge[i]) continue;
		s += sizeof (struct addredge);
		s += (node->edge[i]->len / KEYWIDTH);
		s += (node->edge[i]->len % KEYWIDTH) != 0;
		s += tree_size(tree, node->edge[i]->node);
	}
	return s;
}

size_t addrtree_size(const struct addrtree* tree)
{
	if (!tree) return 0;
	return sizeof (struct addrtree) + tree_size(tree, tree->root);
}

/** 
 * Scrub a node clean of elem
 * @param tree: Tree the node lives in.
 * @param node: Node to be cleaned
 */
static void
clean_node(struct addrtree* tree, struct addrnode* node)
{
	if (!node->elem) return;
	tree->delfunc(tree->env, node->elem);
	node->elem = NULL;
	tree->elem_count--;
}

/** 
 * Purge a node from the tree. Node and parentedge are cleaned and 
 * free'd.
 * @param tree: Tree the node lives in.
 * @param node: Node to be freed
 * @param parentnode: parent of node, may be NULL if node is rootnode.
 * @param parentedge: edge between parentnode and node. May be NULL iff
 *        parentnode is NULL
 */
static void
purge_node(struct addrtree* tree, struct addrnode* node, 
	struct addrnode* parentnode, struct addredge* parentedge)
{
	struct addredge *child_edge = NULL;
	int childcount = (node->edge[0] != NULL) + (node->edge[1] != NULL);
	clean_node(tree, node);
	if (childcount == 2 || !parentnode)
		return;
	log_assert(parentedge); /* parent node implies parent edge */
	if (childcount == 1) { /* Fix reference */
		child_edge = node->edge[!node->edge[0]];
	}
	parentnode->edge[parentedge != parentnode->edge[0]] = child_edge;
	free(parentedge->str);
	free(parentedge);
}

/** 
 * Free node and all nodes below
 * @param tree: Tree the node lives in.
 * @param node: Node to be freed
 */
static void
freenode_recursive(struct addrtree* tree, struct addrnode* node)
{
	struct addredge* edge;
	int i;
	
	for (i = 0; i < 2; i++) {
		edge = node->edge[i];
		if (edge) {
			log_assert(edge->node != NULL);
			log_assert(edge->str != NULL);
			freenode_recursive(tree, edge->node);
			free(edge->str);
		}
		free(edge);
	}
	clean_node(tree, node);
	free(node);
}

/** 
 * Free complete addrtree structure
 * @param tree: Tree to free
 */
void addrtree_delete(struct addrtree* tree)
{
	if (!tree) return;
	if (tree->root)
		freenode_recursive(tree, tree->root);
	free(tree);
}

/** Get N'th bit from address 
 * @param addr: address to inspect
 * @param addrlen: length of addr in bits
 * @param n: index of bit to test. Must be in range [0, addrlen)
 * @return 0 or 1
 */
static int 
getbit(const addrkey_t* addr, addrlen_t addrlen, addrlen_t n)
{
	log_assert(addrlen > n);
	return (int)(addr[n/KEYWIDTH]>>((KEYWIDTH-1)-(n%KEYWIDTH))) & 1;
}

/** Test for equality on N'th bit.
 * @return 0 for equal, 1 otherwise 
 */
static inline int 
cmpbit(const addrkey_t* key1, const addrkey_t* key2, addrlen_t n)
{
	addrkey_t c = key1[n/KEYWIDTH] ^ key2[n/KEYWIDTH];
	return (int)(c >> ((KEYWIDTH-1)-(n%KEYWIDTH))) & 1;
}

/**
 * Common number of bits in prefix
 * @param s1: 
 * @param l1: Length of s1 in bits
 * @param s2:
 * @param l2: Length of s2 in bits
 * @param skip: Nr of bits already checked.
 * @return Common number of bits.
 */
static addrlen_t 
bits_common(const addrkey_t* s1, addrlen_t l1, 
	const addrkey_t* s2, addrlen_t l2, addrlen_t skip)
{
	addrlen_t len, i;
	len = (l1 > l2) ? l2 : l1;
	log_assert(skip < len);
	for (i = skip; i < len; i++) {
		if (cmpbit(s1, s2, i)) return i;
	}
	return len;
} 

/**
 * Tests if s1 is a substring of s2
 * @param s1: 
 * @param l1: Length of s1 in bits
 * @param s2:
 * @param l2: Length of s2 in bits
 * @param skip: Nr of bits already checked.
 * @return 1 for substring, 0 otherwise 
 */
static int 
issub(const addrkey_t* s1, addrlen_t l1, 
	const addrkey_t* s2, addrlen_t l2,  addrlen_t skip)
{
	return bits_common(s1, l1, s2, l2, skip) == l1;
}

void
addrtree_insert(struct addrtree* tree, const addrkey_t* addr, 
	addrlen_t sourcemask, addrlen_t scope, void* elem, time_t ttl, 
	time_t now)
{
	struct addrnode* newnode, *node;
	struct addredge* edge, *newedge;
	uint8_t index;
	addrlen_t common, depth;

	node = tree->root;
	log_assert(node != NULL);

	/* Protect our cache against too much fine-grained data */
	if (tree->max_depth < scope) scope = tree->max_depth;
	/* Server answer was less specific than question */
	if (scope < sourcemask) sourcemask = scope;

	depth = 0;
	while (1) {
		log_assert(depth <= sourcemask);
		/* Case 1: update existing node */
		if (depth == sourcemask) {
			/* update this node's scope and data */
			clean_node(tree, node);
			tree->elem_count++;
			node->elem = elem;
			node->scope = scope;
			return;
		}
		index = (uint8_t)getbit(addr, sourcemask, depth);
		/* Get an edge to an unexpired node */
		edge = node->edge[index];
		while (edge) {
			/* Purge all expired nodes on path */
			if (!edge->node->elem || edge->node->ttl >= now)
				break;
			purge_node(tree, edge->node, node, edge);
			edge = node->edge[index];
		}
		/* Case 2: New leafnode */
		if (!edge) {
			newnode = node_create(tree, elem, scope, ttl);
			node->edge[index] = edge_create(newnode, addr, sourcemask);
			if (!node->edge[index])
				free(newnode);
			return;
		}
		/* Case 3: Traverse edge */
		common = bits_common(edge->str, edge->len, addr, sourcemask, depth);
		if (common == edge->len) {
			/* We update the scope of intermediate nodes. Apparently the
			 * authority changed its mind. If we would not do this we 
			 * might not be able to reach our new node */
			node->scope = scope;
			depth = edge->len;
			node = edge->node;
			continue;
		}
		/* Case 4: split. */
		if (!(newnode = node_create(tree, NULL, 0, 0)))
			return;
		if (!(newedge = edge_create(newnode, addr, common))) {
			clean_node(tree, newnode);
			free(newnode);
			return;
		}		
		node->edge[index] = newedge;
		index = (uint8_t)getbit(edge->str, edge->len, common);
		newnode->edge[index] = edge;
		
		if (common == sourcemask) {
			/* Data is stored in the node */
			newnode->elem = elem;
			newnode->scope = scope;
			newnode->ttl = ttl;
		} else {
			/* Data is stored in other leafnode */
			node = newnode;
			newnode = node_create(tree, elem, scope, ttl);
			node->edge[index^1] = edge_create(newnode, addr, sourcemask);
		}
		return;
	}
}

struct addrnode *
addrtree_find(const struct addrtree* tree, const addrkey_t* addr, 
	addrlen_t sourcemask, time_t now)
{
	struct addrnode *parent_node = NULL, *node = tree->root;
	struct addredge *edge = NULL;
	addrlen_t depth = 0;

	log_assert(node != NULL);
	while (1) {
		/* Current node more specific then question. */
		log_assert(depth <= sourcemask);
		/* does this node have data? if yes, see if we have a match */
		if (node->elem && node->ttl >= now) {
			log_assert(node->scope >= depth); /* saved at wrong depth */
			if (depth == node->scope ||
					(node->scope > sourcemask && depth == sourcemask)) {
				/* Authority indicates it does not have a more precise
				 * answer or we cannot ask a more specific question. */
				return node;
			}
		}
		/* This is our final depth, but we haven't found an answer. */
		if (depth == sourcemask)
			return NULL;
		/* Find an edge to traverse */
		edge = node->edge[getbit(addr, sourcemask, depth)];
		if (!edge || !edge->node)
			return NULL;
		if (edge->len > sourcemask )
			return NULL;
		if (!issub(edge->str, edge->len, addr, sourcemask, depth))
			return NULL;
		log_assert(depth < edge->len);
		parent_node = node;
		depth = edge->len;
		node = edge->node;
		
	}
}

/** Wrappers for static functions to unit test */
int unittest_wrapper_addrtree_cmpbit(const addrkey_t* key1, 
		const addrkey_t* key2, addrlen_t n) {
	return cmpbit(key1, key2, n);
}
addrlen_t unittest_wrapper_addrtree_bits_common(const addrkey_t* s1, 
		addrlen_t l1, const addrkey_t* s2, addrlen_t l2, addrlen_t skip) {
	return bits_common(s1, l1, s2, l2, skip);
}
int unittest_wrapper_addrtree_getbit(const addrkey_t* addr, 
		addrlen_t addrlen, addrlen_t n) {
	return getbit(addr, addrlen, n);
}
int unittest_wrapper_addrtree_issub(const addrkey_t* s1, addrlen_t l1, 
		const addrkey_t* s2, addrlen_t l2,  addrlen_t skip) {
	return issub(s1, l1, s2, l2, skip);
}
