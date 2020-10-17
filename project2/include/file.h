#ifndef __FILE_H__
#define __FILE_H__
#include "defines.h"

extern int table_id;

typedef uint64_t pagenum_t;
struct page_t{
    union{
        struct{
            pagenum_t free_page;
            pagenum_t root_page;
            pagenum_t num_of_pages;
            char reserved[4072];
        }header_page;

        struct{
            pagenum_t parent_page;
            bool is_leaf;
            char padding[3];
            int num_of_keys;
            char reserved[96];
            // First key is dummy
            internal_data data[249];
        }internal_page;

        struct{
            pagenum_t parent_page;
            bool is_leaf;
            char padding[3];
            int num_of_keys;
            char reserved[104];
            pagenum_t right_sibling;
            record data[31];
        }leaf_page;
    }page_content;
};

// Open file and return unique table id
int file_open(char* path);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page();

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src)
void file_write_page(pagenum_t pagenum, const page_t* src);

#endif /* __FILE_H__ */
