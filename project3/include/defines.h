#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <stdint.h>

// Default order is 4.
#define DEFAULT_ORDER 4

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20
#define LEAF_ORDER 32
#define INTERNAL_ORDER 249

// sizes

#define TABLE_SIZE 10
#define PATH_LENGTH 21
#define PAGE_SIZE sizeof(page_t)

// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// Abbreviates for file manager
#define _header page_content.header_page
#define _free page_content.header_page
#define _internal page_content.internal_page
#define _leaf page_content.leaf_page

// Error states
#define CANNOT_CREATE_FILE -1
#define BUFFER_NOT_ALLOCATED -1
#define DUPLICATE_KEYS 1
#define TREE_IS_EMPTY 2
#define NO_SUCH_DATA 3
#define FILE_NOT_OPEN 4
#define INDEX_EXCEEDED 5

// Typedefs
typedef int64_t key_type;
typedef uint64_t pagenum_t;

// Structs
struct record{
    key_type key;
    char value[120];
};

struct internal_data{
    key_type key;
    pagenum_t value;
};

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

struct file_matcher{
    int fd = -1;
    char path[21];
};

struct buffer_t{
    page_t frame;
    int table_id = 0;
    pagenum_t pagenum;
    bool is_dirty = false;
    int pin_count = 0;
    int next = 0;
    int prev = 0;
    bool trigger = false;
};

#endif /* __DEFINE_H__ */
