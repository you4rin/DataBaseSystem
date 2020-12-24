#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <stdint.h>
#include <pthread.h>

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
#define VALUE_LENGTH 120
#define RECORD_SIZE 128
#define DEFAULT_LOG_BUF_SIZE 1000 
#define PAGE_SIZE sizeof(page_t)

#define DEFAULT_LOG_SIZE 28
#define UPDATE_LOG_SIZE 288
#define COMPENSATE_LOG_SIZE 296

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

// Abbreviates for log manager
#define _begin log_content.rest
#define _update log_content.update
#define _commit log_content.rest
#define _rest log_content.rest
#define _rollback log_content.rest
#define _compensate log_content.compensate

// Error states
#define INVALID_FILE_NAME -2
#define CANNOT_CREATE_FILE -1
#define BUFFER_NOT_ALLOCATED -1
#define DUPLICATE_KEYS 1
#define TREE_IS_EMPTY 2
#define NO_SUCH_DATA 3
#define FILE_NOT_OPEN 4
#define INDEX_EXCEEDED 5
#define ABORT 6

// Lock modes
#define SHARED 0
#define EXCLUSIVE 1

// Log types
#define BEGIN 0
#define UPDATE 1
#define COMMIT 2
#define ROLLBACK 3
#define COMPENSATE 4

// Return values of lock_acquire
#define ACQUIRED 0
#define NEED_TO_WAIT 1
#define DEADLOCK 2

// init_db flags
#define NORMAL 0
#define REDO_CRASH 1
#define UNDO_CRASH 2

// casting between offset and record index
#define OFFSET(x) (((RECORD_SIZE) * (x + 1)) + sizeof(key_type))
#define INDEX(x) (((x - sizeof(key_type)) / RECORD_SIZE) - 1)

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
            uint64_t pageLSN;
            char reserved[4064];
        }header_page;

        struct{
            pagenum_t parent_page;
            bool is_leaf;
            char padding[3];
            int num_of_keys;
            uint64_t pageLSN;
            char reserved[88];
            // First key is dummy
            internal_data data[249];
        }internal_page;

        struct{
            pagenum_t parent_page;
            bool is_leaf;
            char padding[3];
            int num_of_keys;
            uint64_t pageLSN;
            char reserved[96];
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
    int table_id;
    pagenum_t pagenum;
    bool is_dirty;
    pthread_mutex_t page_latch;
    int next = 0;
    int prev = 0;
    bool trigger;
};

struct lock_t{
	struct lock_t* next;
	struct lock_t* prev;
	struct lock_table_t* sentinel;
	pthread_cond_t cond;
	int lock_mode;
	int owner_id;
};

struct lock_table_t{
	int table_id;
	int64_t key;
	lock_t* head;
	lock_t* tail;
	int x_cnt;
};

#pragma pack(push, 4)

struct log_t{
    union{
        struct{
            int log_size; // log record size
            uint64_t LSN;
            uint64_t prevLSN; // previous log LSN of same trx_id
            int trx_id;
            int type; // type of log record

            int table_id;
            pagenum_t pagenum;
            int offset; // start offset of the modified area
            int data_length; // length of the modified area
            char old_image[120];
            char new_image[120];
        }update; // update, compensate
        struct{
            int log_size; // log record size
            uint64_t LSN;
            uint64_t prevLSN; // previous log LSN of same trx_id
            int trx_id;
            int type; // type of log record

            int table_id;
            pagenum_t pagenum;
            int offset; // start offset of the modified area
            int data_length; // length of the modified area
            char old_image[120];
            char new_image[120];

            uint64_t next_undo_LSN; // next undo point
        }compensate; // update, compensate
        struct{
            int log_size; // log record size
            uint64_t LSN;
            uint64_t prevLSN; // previous log LSN of same trx_id
            int trx_id;
            int type; // type of log record
        }rest; // rest
    }log_content;
};

#pragma pack(pop)

#endif /* __DEFINE_H__ */
