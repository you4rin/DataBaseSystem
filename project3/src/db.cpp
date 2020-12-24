#include <cassert>
#include <string.h>
#include <string>
#include <stdint.h>
#include "../include/db.h"
#include "../include/buffer.h"

// allocates N+2 buffer_t
// idx 0 is a dummy for lru list
// idx n + 1 is a dummy for free space list 
int init_db(int buf_num){
    return bpt_init(buf_num);
}

int open_table(char* pathname){
    return bpt_open(pathname);
}

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */

int db_insert(int table_id, key_type key, char * value){
    return bpt_insert(table_id, key, value);
}

/* Finds and returns the record to which
 * a key refers.
 */

int db_find(int table_id, key_type key, char* ret_val){
    return bpt_find(table_id, key, ret_val);
}

/* Master deletion function.
 */

int db_delete(int table_id, key_type key){
    return bpt_delete(table_id, key);
}

int close_table(int table_id){
    return bpt_close(table_id);
}

int shutdown_db(void){
    return bpt_shutdown();
}