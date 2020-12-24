#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
#include <utility>

#define UNABLE_TO_RELEASE 1
#define ALLOCATION_FAILURE 2
#define ENTRY hash_table[{table_id, key}]
#define HEAD hash_table[{table_id, key}]->head
#define TAIL hash_table[{table_id, key}]->tail

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

extern std::unordered_map<std::pair<int, int64_t>, lock_table_t*> hash_table;
extern pthread_mutex_t mut;

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
