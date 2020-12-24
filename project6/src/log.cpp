#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <stack>
#include "../include/defines.h"
#include "../include/db.h"
#include "../include/buffer.h"
#include "../include/transaction.h"
#include "../include/log.h"

file_matcher log_file;
FILE* logmsg_file = NULL;
uint64_t flushedLSN, log_tail;
int64_t file_size = 0;
std::stack<uint64_t> undoLSN;
log_t* log_buf = NULL;
int log_buf_size = DEFAULT_LOG_BUF_SIZE, log_buf_num = 0;
pthread_mutex_t log_buffer_latch = PTHREAD_MUTEX_INITIALIZER;

void log_alloc(log_t* log_obj){
    int ret = pthread_mutex_trylock(&log_buffer_latch);
    log_buf[log_buf_num]._rest.log_size = log_obj->_rest.log_size;
    log_buf[log_buf_num]._rest.LSN = log_obj->_rest.LSN;
    log_buf[log_buf_num]._rest.prevLSN = log_obj->_rest.LSN;
    log_buf[log_buf_num]._rest.trx_id = log_obj->_rest.trx_id;
    log_buf[log_buf_num]._rest.type = log_obj->_rest.type;
    if(log_obj->_rest.type == UPDATE || log_obj->_rest.type == COMPENSATE){
        log_buf[log_buf_num]._update.table_id = log_obj->_update.table_id;
        log_buf[log_buf_num]._update.pagenum = log_obj->_update.pagenum;
        log_buf[log_buf_num]._update.offset = log_obj->_update.offset;
        log_buf[log_buf_num]._update.data_length = log_obj->_update.data_length;
        strcpy(log_buf[log_buf_num]._update.old_image, log_obj->_update.old_image);
        strcpy(log_buf[log_buf_num]._update.new_image, log_obj->_update.new_image);
    }
    if(log_obj->_rest.type == COMPENSATE)
        log_buf[log_buf_num]._compensate.next_undo_LSN = log_obj->_compensate.next_undo_LSN;
    log_buf_num++;
    if(!ret)
        pthread_mutex_unlock(&log_buffer_latch);
}

int log_init(int flag, int log_num, char* log_path, char* logmsg_path){
    log_file.fd = -1;
    log_file.fd = open(log_path, O_CREAT|O_RDWR, 
                       S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if(log_file.fd < 0)
        return CANNOT_CREATE_FILE;
    logmsg_file = fopen(logmsg_path, "w");
    if(logmsg_file == NULL)
        return CANNOT_CREATE_FILE;

    // need to do something more about log
    log_buf = (log_t*)malloc(sizeof(log_t) * log_buf_size);
    if(log_buf == NULL){
        perror("Init db");
        exit(EXIT_FAILURE);
    }
    file_size = lseek(log_file.fd, 0, SEEK_END);
    int log_cnt = 1, log_header;
    std::vector<int> winner, loser;
    log_t* log_obj = (log_t*)malloc(sizeof(log_t));
    log_obj->_rest.LSN = 0; 
    undoLSN.push(0);

    fprintf(logmsg_file, "[ANALYSIS] Analysis pass start\n");
    while(log_obj->_rest.LSN < file_size){
        lseek(log_file.fd, log_obj->_rest.LSN, SEEK_SET);
        ssize_t size = read(log_file.fd, &log_header, sizeof(int));
        if(size == (ssize_t)-1){
            perror("Read log header");
            exit(EXIT_FAILURE);
        }
        lseek(log_file.fd, log_obj->_rest.LSN, SEEK_SET);
        size = read(log_file.fd, &log_obj, log_header);
        if(size == (ssize_t)-1){
            perror("Read log header");
            exit(EXIT_FAILURE);
        }
        if(log_obj->_rest.type == BEGIN)
            loser.push_back(log_obj->_rest.trx_id);
        else if(log_obj->_rest.type == COMMIT || log_obj->_rest.type == ROLLBACK){
            auto it = std::find(loser.begin(), loser.end(), log_obj->_rest.trx_id);
            loser.erase(it);
            winner.push_back(log_obj->_rest.trx_id);
        }        
        log_cnt++;
    }
    std::sort(winner.begin(), winner.end());
    std::sort(loser.begin(), loser.end());
    fprintf(logmsg_file, "[ANALYSIS] Analysis success. Winner:");
    for(auto i:winner)
        fprintf(logmsg_file, " %d", i);
    fprintf(logmsg_file, ", Losers");
    for(auto i:loser)
        fprintf(logmsg_file, " %d", i);
    fprintf(logmsg_file, "\n");
    log_obj->_rest.LSN = 0;

    fprintf(logmsg_file, "[REDO] Redo pass start\n");
    while(log_obj->_rest.LSN < file_size){
        lseek(log_file.fd, log_obj->_rest.LSN, SEEK_SET);
        ssize_t size = read(log_file.fd, &log_header, sizeof(int));
        if(size == (ssize_t)-1){
            perror("Read log header");
            exit(EXIT_FAILURE);
        }
        lseek(log_file.fd, log_obj->_rest.LSN, SEEK_SET);
        size = read(log_file.fd, &log_obj, log_header);
        if(size == (ssize_t)-1){
            perror("Read log header");
            exit(EXIT_FAILURE);
        }

        if(log_obj->_rest.type == BEGIN){
            fprintf(logmsg_file, "LSN %llu [BEGIN] Transaction id %d\n", 
                    log_obj->_begin.LSN, log_obj->_begin.trx_id);
        }
        else if(log_obj->_rest.type == UPDATE){
            for(auto i:loser){
                if(log_obj->_update.trx_id == i)
                    undoLSN.push(log_obj->_update.LSN);
            }
            int idx = buf_read_page(log_obj->_update.table_id, log_obj->_update.pagenum);
            page_t* page = &buf[idx].frame;
            if(page->_leaf.pageLSN < log_obj->_update.LSN){
                strcpy(page->_leaf.data[INDEX(log_obj->_update.offset)].value, log_obj->_update.new_image);
                page->_leaf.pageLSN = log_obj->_update.LSN;
                buf_write_page(idx);
                fprintf(logmsg_file, "LSN %llu [UPDATE] Transaction id %d redo apply\n", 
                        log_obj->_update.LSN, log_obj->_update.trx_id);
            }
            else{
                remove_pin(idx);
                fprintf(logmsg_file, "LSN %llu [CONSIDER-REDO] Transaction id %d\n", 
                        log_obj->_update.LSN, log_obj->_update.trx_id);
            }
        }
        else if(log_obj->_rest.type == COMMIT){
            fprintf(logmsg_file, "LSN %llu [COMMIT] Transaction id %d\n", 
                    log_obj->_commit.LSN, log_obj->_commit.trx_id);
        }
        else if(log_obj->_rest.type == ROLLBACK){
            fprintf(logmsg_file, "LSN %llu [ROLLBACK] Transaction id %d\n", 
                    log_obj->_rollback.LSN, log_obj->_rollback.trx_id);
        }
        else{
            int idx = buf_read_page(log_obj->_compensate.table_id, log_obj->_compensate.pagenum);
            page_t* page = &buf[idx].frame;
            strcpy(page->_leaf.data[INDEX(log_obj->_compensate.offset)].value, log_obj->_compensate.new_image);
            page->_leaf.pageLSN = log_obj->_compensate.LSN;
            buf_write_page(idx);
            fprintf(logmsg_file, "LSN %llu [CLR] next undo lsn %llu\n", 
                    log_obj->_compensate.LSN, log_obj->_compensate.next_undo_LSN);
        }

        if(log_buf_size == log_buf_num){
            flush();
        }
        else{
            log_alloc(log_obj);
        }

        if(flag == REDO_CRASH && log_cnt == log_num){
            flush(); // implement here
            exit(EXIT_FAILURE);
        }
        log_cnt++;
    }
    fprintf(logmsg_file, "[REDO] Redo pass end\n");
    log_obj->_rest.LSN = 0;
    fprintf(logmsg_file, "[UNDO] Undo pass start\n");
    return 0;
}

int flush(){
    int ret = pthread_mutex_trylock(&log_buffer_latch);
    for(int i = 0; i < log_buf_num; ++i){
        if(log_buf[i]._rest.type == UPDATE){
            
        }
        else if(log_buf[i]._rest.type == COMPENSATE){

        }
        else{
            
        }
    }
    log_buf_num = 0;

    if(!ret)
        pthread_mutex_unlock(&log_buffer_latch);
}

int flush(log_t* log_obj){
    flush();
    log_alloc(log_obj);
    return 0;
}

int record_log(int trx_id, int log_type){
    pthread_mutex_lock(&log_buffer_latch);
    log_t* new_log = (log_t*)malloc(sizeof(log_t));
    new_log->_rest.log_size = DEFAULT_LOG_SIZE;
    new_log->_rest.LSN = flushedLSN + new_log->_rest.log_size;
    flushedLSN = new_log->_rest.LSN;
    new_log->_rest.prevLSN = trx_table[trx_id].LSN;
    trx_table[trx_id].LSN = new_log->_rest.LSN;
    new_log->_rest.type = log_type;
    if(log_buf_size == log_buf_num){
        flush(new_log);
    }
    else{
        log_alloc(new_log);
    }
    pthread_mutex_unlock(&log_buffer_latch);
}

int record_log(int trx_id, int log_type, int table_id, pagenum_t pagenum, 
               int offset, char* old_image, char* new_image){
    pthread_mutex_lock(&trx_manager_latch);
    pthread_mutex_lock(&log_buffer_latch);
    log_t* new_log = (log_t*)malloc(sizeof(log_t));
    new_log->_update.log_size = UPDATE_LOG_SIZE;
    new_log->_update.LSN = flushedLSN + new_log->_update.log_size;
    flushedLSN = new_log->_update.LSN;
    new_log->_update.prevLSN = trx_table[trx_id].LSN;
    trx_table[trx_id].LSN = new_log->_update.LSN;
    new_log->_update.type = log_type;
    new_log->_update.table_id = table_id;
    new_log->_update.pagenum = pagenum;
    new_log->_update.offset = offset;
    new_log->_update.data_length = VALUE_LENGTH;
    strcpy(new_log->_update.old_image, old_image);
    strcpy(new_log->_update.new_image, new_image);
    LOG.push_back(new_log);
    if(log_buf_size == log_buf_num){
        flush(new_log);
    }
    else{
        log_alloc(new_log);
    }
    pthread_mutex_unlock(&trx_manager_latch);
    pthread_mutex_unlock(&log_buffer_latch);
}

int record_log(int trx_id, int log_type, int table_id, pagenum_t pagenum, 
               int offset, char* old_image, char* new_image, int next_undo_LSN){
    int ret = pthread_mutex_trylock(&trx_manager_latch); // think of this in compensate log
    pthread_mutex_lock(&log_buffer_latch);
    log_t* new_log = (log_t*)malloc(sizeof(log_t));
    new_log->_compensate.log_size = COMPENSATE_LOG_SIZE;
    new_log->_compensate.LSN = flushedLSN + new_log->_compensate.log_size;
    flushedLSN = new_log->_update.LSN;
    new_log->_compensate.prevLSN = trx_table[trx_id].LSN;
    trx_table[trx_id].LSN = new_log->_compensate.LSN;
    new_log->_compensate.type = log_type;
    new_log->_compensate.table_id = table_id;
    new_log->_compensate.pagenum = pagenum;
    new_log->_compensate.offset = offset;
    new_log->_compensate.data_length = VALUE_LENGTH;
    strcpy(new_log->_compensate.old_image, old_image);
    strcpy(new_log->_compensate.new_image, new_image);
    new_log->_compensate.next_undo_LSN = next_undo_LSN;
    if(log_buf_size == log_buf_num){
        flush(new_log);
    }
    else{
        log_alloc(new_log);
    }
    if(!ret)
        pthread_mutex_unlock(&trx_manager_latch);
    pthread_mutex_unlock(&log_buffer_latch);
}