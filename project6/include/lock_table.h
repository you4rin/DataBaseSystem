#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
#include <utility>
#include "defines.h"

#define UNABLE_TO_RELEASE 1
#define ALLOCATION_FAILURE 2
#define ENTRY lock_table[{table_id, key}]
#define HEAD lock_table[{table_id, key}]->head
#define TAIL lock_table[{table_id, key}]->tail

typedef struct lock_t lock_t;
typedef struct lock_table_t lock_table_t;

namespace std{
    template <>
    struct hash<pair<int, int64_t>>{
        size_t operator()(const pair<int, int64_t>& p) const{
	        auto h1 = hash<int>{}(p.first);
	        auto h2 = hash<int64_t>{}(p.second);
	        return h1 ^ h2;
        }
    };
}

extern pthread_mutex_t lock_manager_latch;

/* APIs for lock table */
int is_locked(pthread_mutex_t* mutex);
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode);
int lock_release(lock_t* lock_obj);
void lock_wait(lock_t* lock_obj);

/* APIs for transaction */

#endif /* __LOCK_TABLE_H__ */
