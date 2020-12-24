#ifndef __FILE_H__
#define __FILE_H__
#include "defines.h"

extern file_matcher table_ids[TABLE_SIZE + 1];
extern int table_cnt;

// Check if file is empty
bool file_empty(int table_num);

// Check if file is opened
bool file_is_open(int table_id);

// Open file and return unique table id
int file_open(char* path);

// Close file
int file_close(int table_id);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int table_id);

// Free an on-disk page to the free page list
void file_free_page(int table_id, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src)
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src);

#endif /* __FILE_H__ */
