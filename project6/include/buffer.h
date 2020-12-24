#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <pthread.h>
#include "defines.h"

extern buffer_t* buf;
extern std::unordered_map<int, std::unordered_map<pagenum_t, int>> hash_table;
extern int buf_size;
extern int buf_cnt;
extern pthread_mutex_t buffer_manager_latch;

int buf_init(int buf_num, int flag, int log_num, char* log_path, char* logmsg_path);
int buf_open(char* path);
int buf_close(int table_id);
int buf_shutdown();

bool buf_is_open(int table_id);

int buf_find_page(int table_id, pagenum_t pagenum);
//void copy_page(page_t* dest, page_t* src);
int buf_get_idx(int table_id, pagenum_t pagenum, int fail, bool is_read);
void buf_read(int idx, int table_id, pagenum_t pagenum, page_t* page, bool is_read);
int buf_read_page(int table_id, pagenum_t pagenum);
int buf_write_page(int idx);
int buf_alloc_page(int table_id);
void buf_free_page(int idx);
void buf_remove(int table_id);
int buf_remove_page(int idx);

//void refresh_pin(int idx);
void add_pin(int idx);
void remove_pin(int idx);
void make_dirty(int idx);
/*
int buf_find_key(int table_id, key_type key);
int buf_find_leaf(int table_id, key_type key);
void buf_read(int idx, int table_id, pagenum_t pagenum, page_t* page);
void move_buf(int src, int dest);
int buf_alloc_page(int table_id, pagenum_t pagenum);
int buf_remove_page(int idx);
*/

#endif /* __BUFFER_H__ */