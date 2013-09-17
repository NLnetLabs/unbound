/*
 * addrtree -- radix tree for vandergaast cache.
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */

/** 
 * \file
 * The addrtree is a radix tree designed for vandergaast. Most notable
 * is the addition of 'scope' to a node. Scope is only relevant for 
 * nodes with elem set, it indicates the number of bits the authority
 * desires.
 * 
 * For retreiving data one needs an address and address length 
 * (sourcemask). While traversing the tree the first matching node is 
 * returned. A node matches when 
 * 		node.scope<=sourcemask && node.elem!=NULL 
 * 		(This is the most specific answer the authority has.)
 * or
 * 		node.sourcemask==sourcemask && node.elem!=NULL
 * 		(This is the most specific question the client can ask.)
 * 
 * Insertion needs an address, sourcemask and scope. The length of the 
 * address is capped by min(sourcemask, scope). While traversing the 
 * tree the scope of all visited nodes is updated. This ensures we are 
 * always able to find the most specific answer available.
 * 
 * At this time the tree does not support deletion of nodes, but elem 
 * can be set to NULL to ignore the node.
 */

#ifndef ADDRTREE_H
#define ADDRTREE_H

typedef uint8_t addrlen_t;
typedef uint8_t addrkey_t;
#define KEYWIDTH 8

struct addrtree {
	struct addrnode* root;
	/** Maximum prefix length we are willing to cache. */
	addrlen_t max_depth;
	struct module_env* env;
	
};

struct addrnode {
	/** Payload of node, may be NULL */
	struct reply_info* elem;
	/** Number of significant bits in address. */
	addrlen_t scope;
	/** A node can have 0-2 edges, set to NULL for unused */
	struct addredge* edge[2];
};

struct addredge {
	/** address of connected node */
	addrkey_t* str;
	/** lenght in bits of str */
	addrlen_t len;
	/** child node this edge is connected to */
	struct addrnode* node;
};

/** 
 * Create a new tree
 * @param max_depth: Tree will cap keys to this length.
 * @param env: Module environment for alloc information
 * @return new addrtree or NULL on failure
 */
struct addrtree* addrtree_create(addrlen_t max_depth, struct module_env* env);

/** 
 * Free tree and all nodes below
 * @param tree: Tree to be freed
 */
void addrtree_delete(struct addrtree* tree);

/** 
 * Free data stored at node
 * @param tree: Tree the node lives in.
 * @param node: Node to be scrubbed
 */
void addrtree_clean_node(const struct addrtree* tree, struct addrnode* node);

/**
 * Insert an element in the tree. Failures are silent. Sourcemask and
 * scope might be changed according to local policy.
 * 
 * @param tree: Tree insert elem in
 * @param addr: key for element lookup
 * @param sourcemask: Length of addr in bits
 * @param scope: Number of significant bits in addr
 * @param elem: data to store in the tree
 */
void addrtree_insert(struct addrtree* tree, const addrkey_t* addr, 
	addrlen_t sourcemask, addrlen_t scope, struct reply_info* elem);

/**
 * Find a node containing an element in the tree.
 * 
 * @param tree: Tree to search
 * @param addr: key for element lookup
 * @param sourcemask: Length of addr in bits
 * @return addrnode or NULL on miss
 */
struct addrnode* addrtree_find(const struct addrtree* tree, 
	const addrkey_t* addr, addrlen_t sourcemask);

/** Wrappers for static functions to unit test */
int unittest_wrapper_addrtree_cmpbit(const addrkey_t* key1, 
	const addrkey_t* key2, addrlen_t n);
addrlen_t unittest_wrapper_addrtree_bits_common(const addrkey_t* s1, 
	addrlen_t l1, const addrkey_t* s2, addrlen_t l2, addrlen_t skip);
#endif /* ADDRTREE_H */
