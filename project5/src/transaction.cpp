#include <string.h>
#include <atomic>
#include "../include/buffer.h"
#include "../include/transaction.h"

std::unordered_map<int, trx_t> trx_table;
int global_trx_id = 1;
pthread_mutex_t trx_manager_latch = PTHREAD_MUTEX_INITIALIZER;

int trx_manager_begin(){
    pthread_mutex_lock(&trx_manager_latch);
	int trx_id;
    trx_table[trx_id = global_trx_id++].trx_latch = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&trx_manager_latch);
    return trx_id;
}

int release_all(int trx_id){
    // move to lock manager layer
    auto it = LCK.begin();
    while(it != LCK.end()){
        if(it->second != NULL)
            lock_release(it->second);
        it++;
    }
    return trx_id;
}

void rollback_results(int trx_id){
    for(auto i = LOG.rbegin(); i != LOG.rend(); i++){
        if(i->log_type == UPDATE){
            int idx = buf_read_page(i->table_id, i->pagenum);
            page_t* page = &buf[idx].frame;
            strcpy(page->_leaf.data[i->record_idx].value, i->value);
            buf_write_page(idx);
            free(i->value);
        }
    }
}

// value must be allocated
int record_log(int trx_id, int log_type, int table_id, 
               pagenum_t pagenum, int record_idx, char* value){
    pthread_mutex_lock(&trx_manager_latch);
    log_t new_log;
    new_log.log_type = log_type;
    if(log_type == UPDATE){
        new_log.table_id = table_id;
        new_log.pagenum = pagenum;
        new_log.record_idx = record_idx;
        new_log.value = value;
    }
    LOG.push_back(new_log);
    pthread_mutex_unlock(&trx_manager_latch);
}

// return 0 if no deadlock, return trx_id if deadlock exists
int find_deadlock(int start, int now){
    int ret_val;
    if(start == now){
        for(auto i:trx_table)
            trx_table[i.first].visited = false;
        trx_table[now].visited = true;
    }
    for(auto i:trx_table[now].outdeg){
        if(i.first == start){
            return start;
        }
        if(trx_table[i.first].visited)
            continue;
        trx_table[i.first].visited = true;
        ret_val = find_deadlock(start, i.first);
        if(ret_val){
            return ret_val;
        }
    }
    return 0;
}

bool is_abort(int trx_id){
    if(!trx_id)
        // trx_id = 0 -> insert or delete
        return false;
    pthread_mutex_lock(&trx_manager_latch);
    auto it = trx_table.find(trx_id);
    if(it == trx_table.end()){
        pthread_mutex_unlock(&trx_manager_latch);
        return true;
    }
    pthread_mutex_unlock(&trx_manager_latch);
    return false;
}

// trx_manager_latch -> lock_manager_latch
int trx_manager_abort(int trx_id){
    pthread_mutex_lock(&trx_manager_latch);
    if(trx_table.find(trx_id) == trx_table.end()){
        pthread_mutex_unlock(&trx_manager_latch);
        return 0;
    }
    rollback_results(trx_id);
    int ret_val = release_all(trx_id);
    trx_table.erase(trx_id);
    pthread_mutex_unlock(&trx_manager_latch);
    return ret_val;
}

// trx_manager_latch -> lock_manager_latch
int trx_manager_commit(int trx_id){
    pthread_mutex_lock(&trx_manager_latch);
    if(trx_table.find(trx_id) == trx_table.end()){
        pthread_mutex_unlock(&trx_manager_latch);
        return 0;
    }
    int ret_val = release_all(trx_id);
    trx_table.erase(trx_id);
    pthread_mutex_unlock(&trx_manager_latch);
    return ret_val;
}