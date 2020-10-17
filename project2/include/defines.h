#ifndef __DEFINE_H__
#define __DEFINE_H__

#include<stdint.h>

// Default order is 4.
#define DEFAULT_ORDER 4

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20
#define LEAF_ORDER 32
#define INTERNAL_ORDER 249

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
#define DUPLICATE_KEYS 1
#define TREE_IS_EMPTY 2
#define NO_SUCH_DATA 3
#define FILE_NOT_OPEN 4

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


#endif /* __DEFINE_H__ */
