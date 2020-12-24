#include <string.h>
#include <atomic>
#include "../include/buffer.h"
#include "../include/transaction.h"
#include "../include/log.h"

std::unordered_map<int, trx_t> trx_table;
int global_trx_id = 1;
pthread_mutex_t trx_manager_latch = PTHREAD_MUTEX_INITIALIZER;

int trx_manager_begin(){
    pthread_mutex_lock(&trx_manager_latch);
	int trx_id;
    trx_table[trx_id = global_trx_id++].trx_latch = PTHREAD_MUTEX_INITIALIZER;
    trx_table[trx_id].LSN = 0;
    record_log(trx_id, BEGIN);
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

void delete_log(int trx_id, bool abort){
    for(auto i = LOG.rbegin(); i != LOG.rend(); i++){
        if(abort && (*i)->_rest.type == UPDATE){
            int idx = buf_read_page((*i)->_update.table_id, (*i)->_update.pagenum);
            page_t* page = &buf[idx].frame;
            strcpy(page->_leaf.data[INDEX((*i)->_update.offset)].value, (*i)->_update.old_image);
            buf_write_page(idx);
            
            record_log(trx_id, COMPENSATE, (*i)->_update.table_id, (*i)->_update.pagenum,
                       (*i)->_update.offset, (*i)->_update.new_image, 
                       (*i)->_update.old_image, (*i)->_update.prevLSN);
                       
        }
        free(*i);
    }
}



// return 0 if no deadlock, return trx_id if deadlock exists
int find_deadlock(int start, int now){
    int ret_val;
    if(start == now){
        for(auto it = trx_table.begin(); it != trx_table.end(); ++it)
            trx_table[it->first].visited = false;
        trx_table[now].visited = true;
    }
    for(auto it = trx_table[now].outdeg.begin(); it != trx_table[now].outdeg.end(); ++it){
        if(it->first == start){
            return start;
        }
        if(trx_table[it->first].visited)
            continue;
        trx_table[it->first].visited = true;
        ret_val = find_deadlock(start, it->first);
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
    delete_log(trx_id, true);
    int ret_val = release_all(trx_id);
    trx_table.erase(trx_id);
    record_log(trx_id, ROLLBACK);
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
    delete_log(trx_id, false); // this function acutally doesn't rollback
    int ret_val = release_all(trx_id);
    trx_table.erase(trx_id);
    record_log(trx_id, COMMIT);
    pthread_mutex_unlock(&trx_manager_latch);
    return ret_val;
}