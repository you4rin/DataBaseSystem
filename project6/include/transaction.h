#ifndef __TRANSACTION_H__
#define __TRANSACTION_H__

#include <pthread.h>
#include <unordered_map>
#include <map>
#include <vector>
#include "../include/defines.h"
#include "../include/lock_table.h"
#define TRX trx_table[trx_id]
#define LOG trx_table[trx_id].log
#define LCK trx_table[trx_id].lock

struct trx_t{
    std::vector<log_t*> log;
    std::unordered_map<int, int> outdeg;
    std::unordered_map<std::pair<int, key_type>, lock_t*> lock;
    pthread_mutex_t trx_latch;
    int LSN;
    bool visited = false;
    bool abort = false;
};

extern std::unordered_map<int, trx_t> trx_table;
extern pthread_mutex_t trx_manager_latch;

int trx_manager_begin();
int find_deadlock(int start, int now);
bool is_abort(int trx_id);
int trx_manager_abort(int trx_id);
int trx_manager_commit(int trx_id);

#endif /* __TRANSACTION_H__ */