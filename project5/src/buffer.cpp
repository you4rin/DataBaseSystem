#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string>
#include <string.h>
#include <stdint.h>
#include "../include/file.h"
#include "../include/buffer.h"
#include "../include/lock_table.h"
#include "../include/db.h"

buffer_t* buf = NULL;
std::unordered_map<int, std::unordered_map<pagenum_t, int>> hash_table;
int buf_size = 0;
int buf_cnt = 0;
pthread_mutex_t buffer_manager_latch = PTHREAD_MUTEX_INITIALIZER;

/*
std::string make_hash(int table_id, pagenum_t pagenum){
    return std::to_string(table_id) + " " + std::to_string(pagenum);
}
*/

int buf_init(int buf_num){
    buf = (buffer_t*)malloc(sizeof(buffer_t) * (buf_num + 2));
    if(buf == NULL){
        perror("Init db");
        exit(EXIT_FAILURE);
    }
    buf_size = buf_num;
    buf_cnt = 0;
    buf[0].next = buf[0].prev = 0;
    buf[0].table_id = buf[buf_size + 1].table_id = 0;
    buf[0].is_dirty = buf[0].trigger = false;
    buf[buf_size + 1].is_dirty = buf[buf_size + 1].trigger = false;
    buf[0].page_latch = PTHREAD_MUTEX_INITIALIZER;
    buf[buf_size + 1].page_latch = PTHREAD_MUTEX_INITIALIZER;
    for(int i = 1; i < buf_size + 2; i++){
        buf[i].is_dirty = buf[i].trigger = false;
        buf[i].page_latch = PTHREAD_MUTEX_INITIALIZER;
        buf[i].table_id = 0;
        buf[i].next = (i % (buf_size + 1)) + 1;
        buf[i].prev = (i + buf_size - 1) % (buf_size + 1) + 1;
    }
    return 0;
}

int buf_open(char* path){
    pthread_mutex_lock(&buffer_manager_latch);
    if(buf == NULL){
        pthread_mutex_unlock(&buffer_manager_latch);
        return BUFFER_NOT_ALLOCATED;
    }
    int table_num = file_open(path);
    if(table_num < 1){
        pthread_mutex_unlock(&buffer_manager_latch);
        return table_num;
    }
    if(file_empty(table_num)){
        int idx = buf_get_idx(table_num, 0, 0, false);
        page_t* header = &buf[idx].frame;
        header->_header.free_page = 0;
        header->_header.root_page = 0;
        header->_header.num_of_pages = 1;
        buf_write_page(idx);
    }
    else
        pthread_mutex_unlock(&buffer_manager_latch);
    return table_num;
}

int buf_close(int table_id){
    if(table_id > table_cnt){
        return INDEX_EXCEEDED;
    }
    if(!file_is_open(table_id)){
        return FILE_NOT_OPEN;
    }
    buf_remove(table_id);
    file_close(table_id);
    return 0;
}

int buf_shutdown(){
    if(buf == NULL)
        return BUFFER_NOT_ALLOCATED;
    buf_remove(0);
    for(int i = 1; i <= TABLE_SIZE; i++){
        memset((void*)&table_ids[i].path, 0, PATH_LENGTH);
        if(!file_is_open(i))
            continue;
        file_close(i);
    }
    free(buf);
    buf = NULL;
    buf_size = 0;
    buf_cnt = 0;
    table_cnt = 0;
    return 0;
}

bool buf_is_open(int table_id){
    return file_is_open(table_id);
}

void pop_lru(int idx){
    buf[buf[idx].next].prev = buf[idx].prev;
    buf[buf[idx].prev].next = buf[idx].next;
}

// necessary
/*
void copy_page(page_t* dest, page_t* src){
    if(src->_leaf.is_leaf){
        dest->_leaf.parent_page = src->_leaf.parent_page;
        dest->_leaf.is_leaf = src->_leaf.is_leaf;
        dest->_leaf.num_of_keys = src->_leaf.num_of_keys;
        dest->_leaf.right_sibling = src->_leaf.right_sibling;
        for(int i = 0; i < dest->_leaf.num_of_keys; i++){
            dest->_leaf.data[i].key = src->_leaf.data[i].key;
            strcpy(dest->_leaf.data[i].value, src->_leaf.data[i].value);
        }
    }
    else
        *dest = *src;
}
*/

// returns 0 if failure, returns buffer index if succeed
// necessary
int buf_find_page(int table_id, pagenum_t pagenum){
    //std::string str = make_hash(table_id, pagenum);
    int flag = pthread_mutex_trylock(&buffer_manager_latch);
    auto table_it = hash_table.find(table_id);
    if(table_it == hash_table.end()){
        if(!flag)
            pthread_mutex_unlock(&buffer_manager_latch);
        return 0;
    }
    auto page_it = table_it->second.find(pagenum);
    if(page_it == table_it->second.end()){
        if(!flag)
            pthread_mutex_unlock(&buffer_manager_latch);
        return 0;
    }
    if(!flag)
        pthread_mutex_unlock(&buffer_manager_latch);
    return page_it->second;
}

void update_lru(int idx){
    buf[buf[0].next].prev = idx;
    buf[idx].next = buf[0].next;
    buf[0].next = idx;
    buf[idx].prev = 0;
}

void free_lru(int idx){
    buf[buf[buf_size + 1].next].prev = idx;
    buf[idx].next = buf[buf_size + 1].next;
    buf[idx].prev = buf_size + 1; 
    buf[buf_size + 1].next = idx;
}


// true: read, false: allocate
int buf_get_idx(int table_id, pagenum_t pagenum, int fail, bool is_read){
    int idx = 0;
    //std::string str;
    if(buf_cnt == buf_size){
        idx = buf[idx].prev;
        while(!idx || pthread_mutex_trylock(&buf[idx].page_latch))
            idx = buf[idx].prev;
        if(buf[idx].is_dirty){
            //assert(buf[idx].table_id==1);
            file_write_page(buf[idx].table_id, buf[idx].pagenum, &buf[idx].frame);
        }
        //str = make_hash(buf[idx].table_id, buf[idx].pagenum);
        hash_table[buf[idx].table_id][buf[idx].pagenum] = 0; 
    }
    else{
        idx = buf[buf_size + 1].next;
        buf_cnt++;
        add_pin(idx);
    }
    //assert(buf[idx].pin_count==1);

    pop_lru(idx);
    //printf("buf_read executed\n");
    buf_read(idx, table_id, pagenum, &buf[idx].frame, is_read);

    //str = make_hash(buf[idx].table_id, buf[idx].pagenum);
    hash_table[table_id][pagenum] = idx;
    if(!fail)
        pthread_mutex_unlock(&buffer_manager_latch);
    return idx;
}

// caution: this function doesn't pin_count variable!!
// necessary
void buf_read(int idx, int table_id, pagenum_t pagenum, page_t* page, bool is_read){
    memset((void*)&(buf[idx].frame), 0, PAGE_SIZE);
    //if(!buf[idx].frame._internal.is_leaf)printf("br: %d %llu %lld\n",idx, pagenum, buf[idx].frame._internal.data[1].key);
    if(is_read){
        file_read_page(table_id, pagenum, page);
        buf[idx].is_dirty = false;
    }
    else
        make_dirty(idx);
    
    buf[idx].pagenum = pagenum;
    buf[idx].table_id = table_id;
    buf[idx].trigger = false;

    update_lru(idx);
}

// caution: DO NOT call this function for the same client twice!!
int buf_read_page(int table_id, pagenum_t pagenum){
    int fail = pthread_mutex_trylock(&buffer_manager_latch);
    int idx = 0;
    if(idx = buf_find_page(table_id, pagenum)){
        // if the page with given table_id and pagenum already exists in buffer
        // add pin here
        add_pin(idx);
        //assert(buf[idx].pin_count==1);
        if(!fail)
            pthread_mutex_unlock(&buffer_manager_latch);
        return idx;
    }

    return buf_get_idx(table_id, pagenum, fail, true);
}

// actually not writing: just terminate using buffer page
int buf_write_page(int idx){
    make_dirty(idx);
    remove_pin(idx);
}

// return buffer idx of allocated page
// caution: pin of the allocated buffer idx is NOT removed!!
int buf_alloc_page(int table_id){
    pthread_mutex_lock(&buffer_manager_latch);
    int header_idx = buf_read_page(table_id, 0), src_idx = 0;
    page_t *header = &buf[header_idx].frame, *src;
    //std::string str;
    pagenum_t pagenum;

    //assert(buf[header_idx].pin_count==1);
    
    // start with : pincount of header_idx = n+1
    if(header->_header.free_page){
        // pincount of src_idx = n+1
        src_idx = buf_read_page(table_id, header->_header.free_page);
        src = &buf[src_idx].frame;
        //assert(buf[src_idx].pin_count==1);

        header->_header.free_page = src->_free.free_page;
        buf_write_page(header_idx);
        //assert(buf[header_idx].pin_count==0);
        // header_idx pin count = n;

        memset((void*)src, 0, PAGE_SIZE);
        pthread_mutex_unlock(&buffer_manager_latch);
        return src_idx;
    }
    // modify header data
    pagenum = header->_header.num_of_pages++;
    buf_write_page(header_idx);
    //assert(buf[header_idx].pin_count==0);

    return buf_get_idx(table_id, pagenum, 0, false);
}

// caution: this function decreases pin_count!!
void buf_free_page(int idx){
    pthread_mutex_lock(&buffer_manager_latch);
    int header_idx = buf_read_page(buf[idx].table_id, 0);
    page_t* header = &buf[header_idx].frame;

    buf[idx].frame._free.free_page = buf[header_idx].frame._header.free_page;
    buf[header_idx].frame._header.free_page = buf[idx].pagenum;
    buf_write_page(idx);
    buf_write_page(header_idx);
    pthread_mutex_unlock(&buffer_manager_latch);
}

// in shutdown, table_id = 0
void buf_remove(int table_id){
    pthread_mutex_lock(&buffer_manager_latch);
    int cur = 0;
    int pin_count = 0;
    cur = buf[cur].next;
    while(cur || pin_count){
        // case: table id is different
        if((table_id && buf[cur].table_id != table_id) || !buf[cur].table_id){
            cur = buf[cur].next;
            continue;
        }
        // case: the page is pinned
        if(pthread_mutex_trylock(&buf[cur].page_latch)){
            if(!buf[cur].trigger){
                buf[cur].trigger = true;
                pin_count++;
            }
            //printf("Waiting for pinned page\n");
            cur = buf[cur].next;
            continue;
        }
        // page is not pinned
        if(buf[cur].is_dirty){
            //printf("writing page %lld\n", buf[cur].pagenum);
            file_write_page(buf[cur].table_id, buf[cur].pagenum, &buf[cur].frame);
        }
        if(buf[cur].trigger){
            buf[cur].trigger = false;
            pin_count--;
        }
        cur = buf[cur].next;
        pthread_mutex_unlock(&buf[buf[cur].prev].page_latch);
        buf_remove_page(buf[cur].prev);
        buf_cnt--;
    }
    pthread_mutex_unlock(&buffer_manager_latch);
}

int buf_remove_page(int idx){
    if(idx > buf_size || idx <= 0)
        return INDEX_EXCEEDED;
    //std::string str = make_hash(buf[idx].table_id, buf[idx].pagenum);
    hash_table[buf[idx].table_id][buf[idx].pagenum] = 0;
    pop_lru(idx);
    free_lru(idx);
}

/*
void refresh_pin(int idx){
    buf[idx].pin_count = 0;
}
*/

void add_pin(int idx){
    pthread_mutex_lock(&buf[idx].page_latch);
}

void remove_pin(int idx){
    pthread_mutex_unlock(&buf[idx].page_latch);
}

void make_dirty(int idx){
    buf[idx].is_dirty = true;
}
