/*
 *  bpt.c  
 */
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */

#include <algorithm>
#include "../include/bpt.h"
#include "../include/file.h"

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
int internal_order = INTERNAL_ORDER;
int leaf_order = LEAF_ORDER;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
node * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;


// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 */
void license_notice( void ) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
            "type `show w'.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; type `show c' for details.\n\n");
}

/* Get the path of output file from user.
 */

void get_path(void){
    printf("Write the path of file to store the data\n");
    printf("path: ");
}

/* Routine to print portion of GPL license to stdout.
 */
void print_license( int license_part ) {
    int start, end, line;
    FILE * fp;
    char buffer[0x100];

    switch(license_part) {
    case LICENSE_WARRANTEE:
        start = LICENSE_WARRANTEE_START;
        end = LICENSE_WARRANTEE_END;
        break;
    case LICENSE_CONDITIONS:
        start = LICENSE_CONDITIONS_START;
        end = LICENSE_CONDITIONS_END;
        break;
    default:
        return;
    }

    fp = fopen(LICENSE_FILE, "r");
    if (fp == NULL) {
        perror("print_license: fopen");
        exit(EXIT_FAILURE);
    }
    for (line = 0; line < start; line++)
        fgets(buffer, sizeof(buffer), fp);
    for ( ; line < end; line++) {
        fgets(buffer, sizeof(buffer), fp);
        printf("%s", buffer);
    }
    fclose(fp);
}


/* First message to the user.
 */
void usage_1( void ) {
    printf("B+ Tree of internal order %d, leaf order %d.\n", internal_order, leaf_order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n"
           "To build a B+ tree of a different order, start again and enter "
           "the order\n"
           "as an integer argument:  bpt <leaf_order> <internal_order>.\n");
    printf("To start with input from a file of newline-delimited integers, \n"
           "start again and enter the order followed by the filename:\n"
           "bpt <leaf_order> <internal_order> <inputfile>.\n");
    printf("To start with finding data from a file, \n"
           "start again and enter the order followed by the filenames:\n"
           "bpt <leaf_order> <internal_order> <inputfile> <findingfile>.\n");
    printf("To start with deleting data from a file, \n"
           "start again and enter the order followed by the filename:\n"
           "bpt <leaf_order> <internal_order> <inputfile> <findingfile> <deletefile>.\n");

}


/* Second message to the user.
 */
void usage_2( void ) {
    /*
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
    */    
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k> <v> -- Insert <k> <v> as key and value each.\n"
    "\tf <k>     -- Find the value under key <k>.\n"
    "\td <k>     -- Delete key <k> and its associated value.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");

}


/* Brief usage note.
 */
void usage_3( void ) {
    printf("Usage: ./bpt [<order>]\n");
    printf("\twhere %d <= order <= %d .\n", MIN_ORDER, MAX_ORDER);
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue( node * new_node ) {
    node * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
node* dequeue( void ) {
    node * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}

/* Opens a table
 * Return a unique table id
 */

int index_open(char* path){
    return file_open(path);
}

int open_table(char* path){
    return index_open(path);
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */

/*
void print_leaves( node * root ) {
    int i;
    node * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = (node*)c->pointers[0];
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            printf("%d ", c->keys[i]);
        }
        if (verbose_output)
            printf("%lx ", (unsigned long)c->pointers[leaf_order - 1]);
        if (c->pointers[leaf_order - 1] != NULL) {
            printf(" | ");
            c = (node*)c->pointers[leaf_order - 1];
        }
        else
            break;
    }
    printf("\n");
}
*/


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int height( node * root ) {
    int h = 0;
    node * c = root;
    while (!c->is_leaf) {
        c = (node*)c->pointers[0];
        h++;
    }
    return h;
}


/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root( node * root, node * child ) {
    int length = 0;
    node * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
/*
void print_tree( node * root ) {

    node * n = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while( queue != NULL ) {
        n = dequeue();
        if (n->parent != NULL && n == n->parent->pointers[0]) {
            new_rank = path_to_root( root, n );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output) 
            printf("(%lx)", (unsigned long)n);
        for (i = 0; i < n->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)n->pointers[i]);
            printf("%d ", n->keys[i]);
        }
        if (!n->is_leaf)
            for (i = 0; i <= n->num_keys; i++)
                enqueue((node*)n->pointers[i]);
        if (verbose_output) {
            if (n->is_leaf) 
                printf("%lx ", (unsigned long)n->pointers[leaf_order - 1]);
            else
                printf("%lx ", (unsigned long)n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}
*/

void record_not_found(key_type key){
    printf("Record not found under key %lld.\n", key);
}

void print_result(key_type key, char* value){
    printf("key of the record: %lld\n", key);
    printf("value: %s\n", value);
}

/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
/*
void find_and_print(node * root, int key, bool verbose) {
    record * r = db_find(root, key, verbose);
    if (r == NULL)
        printf("Record not found under key %d.\n", key);
    else 
        printf("Record at %lx -- key %d, value %d.\n",
                (unsigned long)r, key, r->value);
}
*/

/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.
 */
/*
void find_and_print_range( node * root, int key_start, int key_end,
        bool verbose ) {
    int i;
    int array_size = key_end - key_start + 1;
    int returned_keys[array_size];
    void * returned_pointers[array_size];
    int num_found = find_range( root, key_start, key_end, verbose,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Location: %lx  Value: %d\n",
                    returned_keys[i],
                    (unsigned long)returned_pointers[i],
                    ((record *)
                     returned_pointers[i])->value);
    }
}
*/

/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.
 */
/*
int find_range(node* root, int key_start, int key_end, bool verbose,
        int returned_keys[], void* returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    node* n = find_leaf( root, key_start, verbose );
    if (n == NULL) return 0;
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = (node*)n->pointers[leaf_order - 1];
        i = 0;
    }
    return num_found;
}
*/

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
pagenum_t find_leaf(key_type key) {
    int s, e;
    page_t *header, *root;
    pagenum_t leaf_num;

    //printf("fl %lld\n",key);
    header = (page_t*)malloc(sizeof(page_t));
    file_read_page(0, header);
    leaf_num = header->_header.root_page;
    if (!leaf_num) {
        free(header);
        return 0;
    }
    root = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf_num, root);
    //printf("root: %llx\n",leaf_num);
    //for(int i=1;i<=root->_internal.num_of_keys;++i)
    //    printf("%llx ",root->_internal.data[i].key);
    //printf("\n");

    while (!root->_internal.is_leaf) {
        s = 1, e = root->_internal.num_of_keys;
        //for(int i=1;i<=root->_internal.num_of_keys;i++)
        //    printf("%llx ",root->_internal.data[i].key);
        //printf("\n");
        while(s < e) {
            int m = (s + e) / 2;
            //printf("%d %llx\n",m,root->_internal.data[m].key);
            if (key > root->_internal.data[m].key)s = m + 1;
            else if(key < root->_internal.data[m].key) e = m - 1;
            else{
                s = m;
                break;
            }
        }
        //printf("%d %llx\n",s,root->_internal.data[s].key);
        if(s && root->_internal.data[s].key > key)s--;
        leaf_num = root->_internal.data[s].value;
        //printf("selected key: %llx\n",root->_internal.data[s].key);
        file_read_page(leaf_num, root);
    }
    free(header);
    free(root);
    return leaf_num;
}


/* Finds and returns the record to which
 * a key refers.
 */
int db_find(int64_t key, char* ret_val) {
    int s = 0, e;
    page_t* c;

    if(table_id < 0)
        return FILE_NOT_OPEN;

    pagenum_t pagenum = find_leaf(key);
    c = (page_t*)malloc(sizeof(page_t));
    file_read_page(pagenum, c);
    if (!c->_header.root_page){
        free(c);
        c = NULL;
        return TREE_IS_EMPTY;
    }
    e = c->_leaf.num_of_keys - 1;
    while(s < e){
        int m = (s + e) / 2;
        if(c->_leaf.data[m].key > key)
            e = m - 1;
        else if(c->_leaf.data[m].key < key) 
            s = m + 1;
        else{
            s = m;
            break;
        }
    }
    if (s < 0 || s >= c->_leaf.num_of_keys || c->_leaf.data[s].key != key){ 
        free(c);
        return NO_SUCH_DATA;
    }
    else{
        strcpy(ret_val, c->_leaf.data[s].value);
        free(c);
        return 0;
    }
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    return (length + 1) / 2;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
record* make_record(key_type key, char* value) {
    record* new_record = (record*)malloc(sizeof(record));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->key = key;
        strcpy(new_record->value,value);
    }
    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
pagenum_t make_page(page_t* new_page) {
    pagenum_t new_page_num = file_alloc_page();
    //printf("new_page_num: %lld\n",new_page_num);
    file_read_page(new_page_num, new_page);
    new_page->_internal.is_leaf = false;
    new_page->_internal.num_of_keys = 0;
    new_page->_internal.parent_page = 0;
    return new_page_num;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
pagenum_t make_leaf(page_t* leaf) {
    pagenum_t leaf_num = file_alloc_page();
    file_read_page(leaf_num, leaf);
    leaf->_leaf.is_leaf = true;
    leaf->_leaf.num_of_keys = 0;
    leaf->_leaf.parent_page = 0;
    return leaf_num;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(pagenum_t pagenum, page_t* parent, page_t* left) {
    int left_index = 0;
    while (left_index < parent->_internal.num_of_keys && 
            parent->_internal.data[left_index].value != pagenum)
        left_index++;
    //printf("left index: %d %lld %lld\n",left_index,parent->_internal.data[left_index].value,pagenum);
    return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_leaf(pagenum_t leaf_num, page_t* leaf, key_type key, char* value) {
    int i, insertion_point, s, e;

    //printf("iil %lld\n",key);

    s = 0, e = leaf->_leaf.num_of_keys;
    while(s < e){
        int m = (s + e) / 2;
        if(leaf->_leaf.data[m].key > key)
            e = m - 1;
        else if(leaf->_leaf.data[m].key < key)
            s = m + 1;
        else{
            s = m;
            break;
        }
    }
    if(s != leaf->_leaf.num_of_keys && leaf->_leaf.data[s].key < key)s++;
    insertion_point = s;

    for (i = leaf->_leaf.num_of_keys; i > insertion_point; i--) {
        leaf->_leaf.data[i].key = leaf->_leaf.data[i - 1].key;
        strcpy(leaf->_leaf.data[i].value, leaf->_leaf.data[i - 1].value);
    }
    
    //printf("%llx\n",leaf_num);

    leaf->_leaf.data[insertion_point].key = key;
    strcpy(leaf->_leaf.data[insertion_point].value, value);
    leaf->_leaf.num_of_keys++;
    file_write_page(leaf_num, leaf);
    free(leaf);

    return 0;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
int insert_into_leaf_after_splitting(pagenum_t leaf_num, page_t* leaf, key_type key, char* value) {
    page_t* new_leaf;
    pagenum_t new_leaf_num;
    record* temp_records;
    int insertion_index, split, new_key, i, j, s, e;

    //printf("iilas %lld\n",key);

    new_leaf = (page_t*)malloc(sizeof(page_t));
    new_leaf_num = make_leaf(new_leaf);
    //printf("left & right: %llx %llx\n",leaf_num,new_leaf_num);

    temp_records = (record*)malloc(leaf_order * sizeof(record));
    if (temp_records == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    s = 0, e = leaf_order - 1;
    while(s < e){
        int m = (s + e) / 2;
        if(leaf->_leaf.data[m].key > key)
            e = m - 1;
        else if(leaf->_leaf.data[m].key < key)
            s = m + 1;
        else{
            s = m;
            break;
        }
    }
    if(s != leaf->_leaf.num_of_keys && leaf->_leaf.data[s].key < key)s++;
    insertion_index = s;

    for (i = 0, j = 0; i < leaf->_leaf.num_of_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_records[j].key = leaf->_leaf.data[i].key;
        strcpy(temp_records[j].value, leaf->_leaf.data[i].value);
    }

    temp_records[insertion_index].key = key;
    strcpy(temp_records[insertion_index].value, value);

    leaf->_leaf.num_of_keys = 0;

    split = cut(leaf_order - 1);

    for (i = 0; i < split; i++) {
        strcpy(leaf->_leaf.data[i].value, temp_records[i].value);
        leaf->_leaf.data[i].key = temp_records[i].key;
        leaf->_leaf.num_of_keys++;
    }

    for (i = split, j = 0; i < leaf_order; i++, j++) {
        strcpy(new_leaf->_leaf.data[j].value, temp_records[i].value);
        new_leaf->_leaf.data[j].key = temp_records[i].key;
        new_leaf->_leaf.num_of_keys++;
    }

    free(temp_records);
    temp_records = NULL;

    new_leaf->_leaf.right_sibling = leaf->_leaf.right_sibling;
    leaf->_leaf.right_sibling = new_leaf_num;
    /*
    for (i = leaf->_leaf.num_of_keys; i < leaf_order - 1; i++){
        leaf->_leaf.data[i].key = 0;
        memset(leaf->_leaf.data[i].value, 0, 120);
    }
    for (i = new_leaf->_leaf.num_of_keys; i < leaf_order - 1; i++){
        new_leaf->_leaf.data[i].key = 0;
        memset(new_leaf->_leaf.data[i].value, 0, 120);
    }
    */
    new_leaf->_leaf.parent_page = leaf->_leaf.parent_page;
    //printf("parent: %lld\n",new_leaf->_leaf.parent_page);
    new_key = new_leaf->_leaf.data[0].key;
    
    // need to free: leaf, new_leaf

    return insert_into_parent(leaf_num, new_leaf_num, leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
int insert_into_node(pagenum_t right_num, page_t* parent, 
                     int left_index, key_type key, page_t* right) {
    int i;
    //printf("iin %lld %d\n",key, left_index);
    for (i = parent->_internal.num_of_keys; i > left_index; i--) {
        parent->_internal.data[i + 1].value = parent->_internal.data[i].value;
        parent->_internal.data[i + 1].key = parent->_internal.data[i].key;
    }
    parent->_internal.data[left_index + 1].value = right_num;
    parent->_internal.data[left_index + 1].key = key;
    parent->_internal.num_of_keys++;

    file_write_page(right->_internal.parent_page, parent);
    file_write_page(right_num, right);

    free(right);
    free(parent);

    right = NULL;
    parent = NULL;
    
    return 0;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
int insert_into_node_after_splitting(pagenum_t pagenum, page_t* old_page, 
                                     int left_index, key_type key, page_t* right) {

    int i, j, split, k_prime;
    page_t *new_page, *child;
    pagenum_t parent_num = right->_internal.parent_page;
    pagenum_t new_page_num;
    internal_data* temp_internals;

    //printf("iinas %lld\n",key);

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */
    new_page = (page_t*)malloc(sizeof(page_t));
    child = (page_t*)malloc(sizeof(page_t));
    temp_internals = (internal_data*)malloc((internal_order + 1) * sizeof(internal_data));

    if (temp_internals == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    for (i = 0, j = 0; i < old_page->_internal.num_of_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_internals[j].value = old_page->_internal.data[i].value;
    }
    for (i = 1, j = 1; i < old_page->_internal.num_of_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_internals[j].key = old_page->_internal.data[i].key;
    }
    temp_internals[left_index + 1].value = pagenum;
    temp_internals[left_index + 1].key = key;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(internal_order);
    new_page_num = make_page(new_page);
    //printf("internal left & right: %llx %llx\n",parent_num,new_page_num);
    //printf("old: ");
    old_page->_internal.num_of_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_page->_internal.data[i].value = temp_internals[i].value;
        old_page->_internal.data[i + 1].key = temp_internals[i + 1].key;
        //printf("%lld ",old_page->_internal.data[i+1].key);
        old_page->_internal.num_of_keys++;
    }
    //printf("\nnew: ");
    old_page->_internal.data[i].value = temp_internals[i].value;
    k_prime = temp_internals[split].key;

    for (++i, j = 0; i < internal_order; i++, j++) {
        new_page->_internal.data[j].value = temp_internals[i].value;
        new_page->_internal.data[j + 1].key = temp_internals[i + 1].key;
        //printf("%lld ",new_page->_internal.data[j+1].key);
        new_page->_internal.num_of_keys++;
    }
    //printf("\n");
    new_page->_internal.data[j].value = temp_internals[i].value;
    free(temp_internals);
    temp_internals = NULL;
    new_page->_internal.parent_page = old_page->_internal.parent_page;
    /*
    for(i = old_page->_internal.num_of_keys + 1; i < internal_order; i++)
        old_page->_internal.data[i].key = old_page->_internal.data[i].value = 0;
    for(i = new_page->_internal.num_of_keys + 1; i < internal_order; i++)
        new_page->_internal.data[i].key = old_page->_internal.data[i].value = 0;
    */
    file_write_page(pagenum, right);

    for (i = 0; i <= new_page->_internal.num_of_keys; i++) {
        file_read_page(new_page->_internal.data[i].value, child);
        child->_internal.parent_page = new_page_num;
        file_write_page(new_page->_internal.data[i].value, child);
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    free(child);
    free(right);
    child = NULL;
    right = NULL;

    return insert_into_parent(parent_num, new_page_num,
                              old_page, k_prime, new_page);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int insert_into_parent(pagenum_t left_num, pagenum_t right_num, 
                       page_t* left, key_type key, page_t* right) {
    int left_index;
    pagenum_t parent_num;
    page_t* parent;

    //printf("iip %lld\n", key);

    // need to free: left, right, parent

    parent = (page_t*)malloc(sizeof(page_t));
    parent_num = left->_leaf.parent_page;
    //printf("left & right & parent: %llx %llx %llx\n",left_num,right_num,parent_num);

    /* Case: new root. */

    if (!parent_num){
        free(parent);
        parent = NULL;
        return insert_into_new_root(left_num, right_num, left, key, right);
    }
    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    file_read_page(parent_num, parent);

    left_index = get_left_index(left_num, parent, left);


    /* Simple case: the new key fits into the node. 
     */
    file_write_page(left_num, left);
    free(left);
    left = NULL;

    if (parent->_internal.num_of_keys < internal_order - 1)
        return insert_into_node(right_num, parent, left_index, key, right);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(right_num, parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
int insert_into_new_root(pagenum_t left_num, pagenum_t right_num, 
                         page_t* left, key_type key, page_t* right) {
    page_t *root, *header;
    pagenum_t pagenum;

    //printf("iinr %lld\n",key);
    
    header = (page_t*)malloc(sizeof(page_t));
    root = (page_t*)malloc(sizeof(page_t));

    pagenum = make_page(root);
    //printf("new root: %llx\n",pagenum);
    file_read_page(0, header);

    //printf("left: ");
    //for(int i=1;i<=left->_internal.num_of_keys;++i)
    //    printf("%lld ",left->_internal.data[i].key);
    //printf("\nright: ");
    //for(int i=1;i<=right->_internal.num_of_keys;++i)
    //    printf("%lld ",right->_internal.data[i].key);
    //printf("\n");

    root->_internal.data[1].key = key;
    root->_internal.data[0].value = left_num;
    root->_internal.data[1].value = right_num;
    root->_internal.num_of_keys++;
    root->_internal.parent_page = 0;
    header->_header.root_page = pagenum;
    left->_leaf.parent_page = pagenum;
    right->_leaf.parent_page = pagenum;

    //printf("reserved check 3: %d\n",right->_internal.reserved[72]);

    //printf("%llx %llx %llx\n",left_num,right_num,pagenum);

    file_write_page(0, header);
    file_write_page(pagenum, root);
    file_write_page(right_num, right);
    file_write_page(left_num, left);

    free(header);
    free(left);
    free(right);
    free(root);

    header = NULL;
    left = NULL;
    right = NULL;
    root = NULL;
    return 0;
}



/* First insertion:
 * start a new tree.
 */
int start_new_tree(key_type key, char* value) {
    page_t *header, *root;
    pagenum_t pagenum;
    
    header = (page_t*)malloc(sizeof(page_t));
    root = (page_t*)malloc(sizeof(page_t));
    pagenum = make_leaf(root);

    file_read_page(0, header);
    
    header->_header.root_page = pagenum;
    file_write_page(0, header);

    root->_leaf.data[0].key = key;
    strcpy(root->_leaf.data[0].value, value);
    root->_leaf.right_sibling = 0;
    root->_leaf.parent_page = 0;
    root->_leaf.num_of_keys++;
    file_write_page(pagenum, root);

    free(header);
    free(root);
    header = NULL;
    root = NULL;

    return 0;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int db_insert(int64_t key, char* value) {
    page_t *leaf, *header;
    pagenum_t leaf_num;

    if(table_id < 0)
        return FILE_NOT_OPEN;

    //printf("%lld %lld\n",key,header->_header.root_page);

    //printf("di %lld\n",key);

    /* The current implementation ignores
     * duplicates.
     */

    // Fix here after fixing db_find
    if (!db_find(key, value))
        return DUPLICATE_KEYS; // duplicate key = 1

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    header = (page_t*)malloc(sizeof(page_t));
    file_read_page(0, header);

    if (!header->_header.root_page){
        free(header);
        return start_new_tree(key, value);
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    leaf = (page_t*)malloc(sizeof(page_t));
    leaf_num = find_leaf(key);
    //printf("leaf: %llx\n",leaf_num);
    file_read_page(leaf_num, leaf);

    free(header);
    header = NULL;

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->_leaf.num_of_keys < leaf_order - 1) {
        return insert_into_leaf(leaf_num, leaf, key, value);
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(leaf_num, leaf, key, value);
}




// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index(pagenum_t pagenum, page_t* parent, page_t* n) {

    int i, s, e;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */

    for (i = 0; i <= parent->_internal.num_of_keys; i++)
        if (parent->_internal.data[i].value == pagenum)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


int remove_entry_from_node(pagenum_t pagenum, page_t* n, key_type key) {

    int i, num_pointers, s, e;

    //printf("refn %lld\n", key);

    // Remove the key & value and shift others accordingly.
    if(n->_internal.is_leaf){
        s = 0, e = n->_leaf.num_of_keys - 1;
        while(s < e){
            int m = (s + e) / 2;
            if(n->_leaf.data[m].key > key)
                e = m - 1;
            else if(n->_leaf.data[m].key < key)
                s = m + 1;
            else{
                s = m;
                break;
            }
        }
        i = s;
        for(++i; i < n->_leaf.num_of_keys; i++){
            n->_leaf.data[i - 1].key = n->_leaf.data[i].key;
            strcpy(n->_leaf.data[i - 1].value, n->_leaf.data[i].value);
        }
    }
    else{
        s = 1, e = n->_internal.num_of_keys;
        while(s < e){
            int m = (s + e) / 2;
            if(n->_internal.data[m].key > key)
                e = m - 1;
            else if(n->_internal.data[m].key < key)
                s = m + 1;
            else{
                s = m;
                break;
            }
        }
        i = s;
        for(++i; i <= n->_internal.num_of_keys; i++){
            n->_internal.data[i - 1].key = n->_internal.data[i].key;
            n->_internal.data[i - 1].value = n->_internal.data[i].value;
        }
    }


    // One key fewer.
    n->_internal.num_of_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    /*
    if (n->_internal.is_leaf)
        for (i = n->_leaf.num_of_keys; i < leaf_order - 1; i++){
            n->_leaf.data[i].key = 0;
            memset(n->_leaf.data[i].value, 0, 120);
        }
    else
        for (i = n->_internal.num_of_keys + 1; i < internal_order; i++){
            n->_internal.data[i].key = 0;
            n->_internal.data[i].value = 0;
        }
    */
    file_write_page(pagenum, n);
    //printf("num of keys: %d\n",n->_internal.num_of_keys);
    return 0;
}


int adjust_root(pagenum_t pagenum, page_t* root) {
    pagenum_t new_page_num;
    page_t *new_page, *header;

    //printf("ar %llu\n", pagenum);

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->_internal.num_of_keys > 0){
        file_write_page(pagenum, root);
        free(root);
        root = NULL;
        return 0;
    }

    /* Case: empty root. 
     */

    new_page = (page_t*)malloc(sizeof(page_t));
    header = (page_t*)malloc(sizeof(page_t));

    if (!root->_internal.is_leaf) {
        // If it has a child, promote 
        // the first (only) child
        // as the new root.


        new_page_num = root->_internal.data[0].value;
        file_read_page(new_page_num, new_page);
        file_read_page(0, header);
        new_page->_internal.parent_page = 0;
        header->_header.root_page = new_page_num;
        file_write_page(new_page_num, new_page);
        file_write_page(0, header);
    }
    else{
        // If it is a leaf (has no children),
        // then the whole tree is empty.

        file_read_page(0, header);
        header->_header.root_page = 0;
        file_write_page(0, header);
    }

    file_free_page(pagenum);
    free(root);
    free(new_page);
    free(header);
    new_page = NULL;
    header = NULL;
    root = NULL;
    return 0;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
int coalesce_nodes(pagenum_t pagenum, pagenum_t neighbor_page_num,
        page_t* n, page_t* neighbor, int neighbor_index, key_type k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    page_t *tmp, *parent;
    pagenum_t tmpnum, parent_num;

    //printf("cn %lld\n", k_prime);

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
        tmpnum = pagenum;
        pagenum = neighbor_page_num;
        neighbor_page_num = tmpnum;
    }
    //printf("n page num: %llx\n", pagenum);
    parent_num = n->_internal.parent_page;
    tmp = (page_t*)malloc(sizeof(page_t));
    parent = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent_num, parent);

    //printf("parent num: %llx\n",parent_num);
    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = neighbor->_internal.num_of_keys;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!n->_internal.is_leaf) {

        /* Append k_prime.
         */

        neighbor->_internal.data[neighbor_insertion_index + 1].key = k_prime;
        neighbor->_internal.num_of_keys++;


        n_end = n->_internal.num_of_keys;
        
        //printf("key: ");
        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            //printf("%lld ",n->_internal.data[j+1].key);
            neighbor->_internal.data[i + 1].key = n->_internal.data[j + 1].key;
            neighbor->_internal.data[i].value = n->_internal.data[j].value;
            neighbor->_internal.num_of_keys++;
            n->_internal.num_of_keys--;
        }
        //printf("\n");

        /* The number of pointers is always
         * one more than the number of keys.
         */

        neighbor->_internal.data[i].value= n->_internal.data[j].value;

        /* All children must now point up to the same parent.
         */

        for (i = 0; i <= neighbor->_internal.num_of_keys; i++) {
            file_read_page(neighbor->_internal.data[i].value, tmp);
            tmp->_leaf.parent_page = neighbor_page_num;
            file_write_page(neighbor->_internal.data[i].value, tmp);
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        //printf("key: ");
        for (i = neighbor_insertion_index, j = 0; j < n->_leaf.num_of_keys; i++, j++) {
            //printf("%lld ",n->_leaf.data[j].key);
            neighbor->_leaf.data[i].key = n->_leaf.data[j].key;
            strcpy(neighbor->_leaf.data[i].value, n->_leaf.data[j].value);
            neighbor->_leaf.num_of_keys++;
        }
        //printf("\n");
        neighbor->_leaf.right_sibling = n->_leaf.right_sibling;
    }
    file_free_page(pagenum);
    file_write_page(neighbor_page_num, neighbor);

    free(n);
    n = NULL;
    free(neighbor);
    neighbor = NULL;
    free(tmp);
    tmp = NULL;
    return delete_entry(parent_num, parent, k_prime);
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */

int redistribute_nodes(pagenum_t pagenum, pagenum_t neighbor_page_num,
        page_t* n, page_t* neighbor, int neighbor_index, 
        int k_prime_index, key_type k_prime) {  

    int i;
    page_t* tmp = (page_t*)malloc(sizeof(page_t));

    //printf("rn %lld %lld %lld\n",k_prime, pagenum, neighbor_page_num);
    /*
     * Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     *
     */

    if (neighbor_index != -1) {
        n->_internal.data[n->_internal.num_of_keys + 1].value
            = n->_internal.data[n->_internal.num_of_keys].value;
        for(i = n->_internal.num_of_keys; i > 0; i--){
            n->_internal.data[i + 1].key = n->_internal.data[i].key;
            n->_internal.data[i].value = n->_internal.data[i].value;
        }
        n->_internal.data[0].value 
            = neighbor->_internal.data[neighbor->_internal.num_of_keys].value;
        file_read_page(n->_internal.data[0].value, tmp);
        tmp->_leaf.parent_page = pagenum;
        file_write_page(n->_internal.data[0].value, tmp);
        n->_internal.data[0].key = k_prime;
        file_read_page(n->_internal.parent_page, tmp);
        tmp->_internal.data[k_prime_index + 1].key
            = neighbor->_internal.data[neighbor->_internal.num_of_keys].key;
        file_write_page(n->_internal.parent_page, tmp);  
    }
    /*
     * Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     *
     */

    else {  
        n->_internal.data[n->_internal.num_of_keys + 1].value
            = neighbor->_internal.data[0].value;
        file_read_page(n->_internal.data[n->_internal.num_of_keys + 1].value, tmp);
        tmp->_internal.parent_page = pagenum;
        if(tmp->_internal.is_leaf)
            n->_internal.data[n->_internal.num_of_keys + 1].key 
                = tmp->_leaf.data[0].key;
        else
            n->_internal.data[n->_internal.num_of_keys + 1].key
                = tmp->_internal.data[1].key;

        file_write_page(n->_internal.data[n->_internal.num_of_keys + 1].value, tmp);
        file_read_page(n->_internal.parent_page, tmp);
        tmp->_internal.data[k_prime_index + 1].key = neighbor->_internal.data[1].key;
        file_write_page(n->_internal.parent_page, tmp);
        for(i = 1; i < neighbor->_internal.num_of_keys; i++){
            neighbor->_internal.data[i].key = neighbor->_internal.data[i + 1].key;
            neighbor->_internal.data[i - 1].value = neighbor->_internal.data[i].value;
        }
        neighbor->_internal.data[i - 1].value = neighbor->_internal.data[i].value;
    }  
    /*
     * n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->_internal.num_of_keys++;
    neighbor->_internal.num_of_keys--;
    file_write_page(pagenum, n);
    file_write_page(neighbor_page_num, neighbor);

    free(n);
    n = NULL;
    free(neighbor);
    neighbor = NULL;
    free(tmp);
    tmp = NULL;

    return 0;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
int delete_entry(pagenum_t pagenum, page_t* n, key_type key) {
    page_t *neighbor, *parent;
    pagenum_t neighbor_page_num;
    int neighbor_index;
    int k_prime_index;
    key_type k_prime;
    int capacity;

    //printf("de %lld\n", key);
    //printf("de pagenum: %llx\n",pagenum);

    // Remove key and pointer from node.
    
    file_read_page(pagenum, n);
    remove_entry_from_node(pagenum, n, key);

    /* Case:  deletion from the root. 
     */

    //printf("de parent num: %llx\n",n->_internal.parent_page);

    if (!n->_internal.parent_page) 
        return adjust_root(pagenum, n);


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */


    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (n->_internal.num_of_keys){
        free(n);
        n = NULL;
        return 0;
    }

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */
    parent = (page_t*)malloc(sizeof(page_t));
    neighbor = (page_t*)malloc(sizeof(page_t));

    file_read_page(n->_internal.parent_page, parent);
    neighbor_index = get_neighbor_index(pagenum, parent, n);
    neighbor_page_num = neighbor_index == -1 ? parent->_internal.data[1].value 
                   : parent->_internal.data[neighbor_index].value;
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = parent->_internal.data[k_prime_index + 1].key;
    file_read_page(neighbor_page_num, neighbor);
    //printf("%d %d\n", leaf_order, internal_order);
    capacity = n->_internal.is_leaf ? leaf_order : internal_order - 1;

    free(parent);
    parent = NULL;

    /* Coalescence. */
   
    if (neighbor->_internal.num_of_keys + n->_internal.num_of_keys < capacity)
        return coalesce_nodes(pagenum, neighbor_page_num, n, neighbor, neighbor_index, k_prime);
    
    /* Redistribution. */
    
    else
        return redistribute_nodes(pagenum, neighbor_page_num, n, neighbor, 
                neighbor_index, k_prime_index, k_prime);
            
}



/* Master deletion function.
 */
int db_delete(int64_t key) {
    page_t* key_leaf;
    pagenum_t pagenum;
    char* value = (char*)malloc(sizeof(char)*120);
    int ret_val;
    
    if(table_id < 0)
        return FILE_NOT_OPEN;
    
    //printf("dd %lld\n", key);

    key_leaf = (page_t*)malloc(sizeof(page_t));

    pagenum = find_leaf(key);
    //printf("pagenum: %llu\n", pagenum);
    if (!(ret_val = db_find(key, value)) && pagenum){
        free(value);
        value = NULL;
        return delete_entry(pagenum, key_leaf, key);
    }
    
    free(key_leaf);
    key_leaf = NULL;
    free(value);
    value = NULL;
    //printf("Did you find it? %d %lld\n",ret_val,pagenum);
    return ret_val;
}


void destroy_tree_nodes(pagenum_t pagenum) {
    int i;
    page_t* root;
    file_read_page(pagenum, root);
    if(root->_internal.is_leaf){
        file_free_page(pagenum);
    }
    else{
        for (i = 0; i < root->_internal.num_of_keys + 1; i++)
            destroy_tree_nodes(root->_internal.data[i].value);
    }
}


int destroy_tree(page_t* header) {
    page_t* root;
    file_read_page(0, header);
    if(header->_header.root_page)
        destroy_tree_nodes(header->_header.root_page);
    return 0;
}

