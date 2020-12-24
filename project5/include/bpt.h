#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#ifdef WINDOWS
#endif

/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
};



// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int internal_order;
extern int leaf_order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern node * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
extern bool verbose_output;


// FUNCTION PROTOTYPES.

// Output and utility.

void get_path(void);
void license_notice(void);
void print_license(int licence_part);
void usage_1(void);
void usage_2(void);
void usage_3(void);
void enqueue(node* new_node);
node* dequeue(void);
int index_open(char* path);
int height(node* root);
int path_to_root(node* root, node* child);
//void print_leaves(node* root);
//void print_tree(node* root);
void record_not_found(key_type key);
void print_result(key_type key, char* value);
//void find_and_print(node * root, int key, bool verbose); 
//void find_and_print_range(node* root, int range1, int range2, bool verbose); 
//int find_range(node* root, int key_start, int key_end, bool verbose,
//        int returned_keys[], void* returned_pointers[]); 

int bpt_init(int buf_num);
int bpt_open(char* path);
int bpt_close(int table_id);
int bpt_shutdown();

int bpt_update(int table_id, key_type key, char* values, int trx_id);

pagenum_t find_leaf(int table_id, key_type key, int trx_id, bool mode);
int bpt_find(int table_id, int64_t key, char* ret_val, int trx_id, bool update);
int cut(int length);

// Insertion.

record* make_record(key_type key, char* value);
int make_page(int table_id);
int make_leaf(int table_id);
int get_left_child(node* parent, node* left);
int insert_into_leaf(int table_id, int leaf_idx, 
                     key_type key, char* value);
int insert_into_leaf_after_splitting(int table_id, int leaf_idx,
                                     key_type key, char* value);
int insert_into_node(int table_id, int parent_idx, int left_child_idx, 
                     key_type key, int right_idx);
int insert_into_node_after_splitting(int table_id, pagenum_t pagenum, 
               page_t* parent, int left_child_idx, key_type key, page_t* right);
int insert_into_parent(int table_id, int left_idx, 
                       key_type key, int right_idx);
int insert_into_new_root(int table_id, int left_idx, 
                         key_type key, int right_idx);
int start_new_tree(int table_id, key_type key, char* value);
int bpt_insert(int table_id, key_type key, char * value);

// Deletion.

int get_neighbor_child_idx(int table_id, pagenum_t pagenum, 
                           page_t* parent, page_t* n);
int adjust_root(int table_id, int root_idx);
int coalesce_nodes(int table_id, int n_idx, int neighbor_idx, 
                   int neighbor_child_idx, key_type k_prime);
int redistribute_nodes(int table_id, int n_idx, int neighbor_idx, 
     int neighbor_child_idx, int k_prime_index, key_type k_prime);
int delete_entry(int table_id, int n_idx, key_type key);
int bpt_delete(int table_id, int64_t key);

//void destroy_tree_nodes(pagenum_t pagenum);
//int destroy_tree(page_t* header);

#endif /* __BPT_H__*/
