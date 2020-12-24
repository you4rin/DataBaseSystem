#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../include/file.h"
#include "../include/db.h"
#include "../include/buffer.h"

file_matcher table_ids[TABLE_SIZE + 1];
int table_cnt = 0;

bool file_empty(int table_num){
    if(lseek(table_ids[table_num].fd, 0, SEEK_END) < sizeof(page_t)){
        return 1;
    }
    return 0;
}

bool file_is_open(int table_id){
    return table_ids[table_id].fd!=-1;
}

int file_open(char* path){
    int table_num = -1, i;
    for(i = 1; i <= table_cnt; i++){
        if(!strcmp(table_ids[i].path, path)){
            table_num = i;
            if(table_ids[table_num].fd != -1)
                return table_num;
            break;
        }
    }
    if(table_num == -1){
        if(table_cnt == TABLE_SIZE)
            return CANNOT_CREATE_FILE;
        else table_num = ++table_cnt;
    }

    table_ids[table_num].fd = open(path, O_CREAT|O_RDWR, 
            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    //printf("%d %d\n",table_ids[table_num].fd, cnt);
    if(table_ids[table_num].fd < 0)
        return CANNOT_CREATE_FILE;

    strcpy(table_ids[table_num].path, path);
    return table_num;
}

int file_close(int table_id){
    int state = close(table_ids[table_id].fd);
    table_ids[table_id].fd = -1;
    if(state < 0){
        perror("File close");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Allocate an on-disk page from the free page list
// almost not in use in project3
pagenum_t file_alloc_page(int table_id){
    page_t *header, *src;
    pagenum_t ret_val;
    
    header = (page_t*)malloc(sizeof(page_t));
    src = (page_t*)malloc(sizeof(page_t));
    memset(src, 0, sizeof(page_t));
    file_read_page(table_id, 0, header);

    if(header->_header.free_page){
        ret_val = header->_header.free_page;
        file_read_page(table_id, ret_val, src);
        header->_header.free_page = src->_free.free_page;
        file_write_page(table_id, 0, header);
        memset(src, 0, 4096);
        file_write_page(table_id, ret_val, src);
        free(header);
        header = NULL;
        free(src);
        src = NULL;
        return ret_val;
    }
    
    //printf("num_of_pages: %lld\n",header->_header.num_of_pages);
    ret_val = header->_header.num_of_pages++;
    file_write_page(table_id, 0, header);
    file_write_page(table_id, ret_val, src);
  
    free(header);
    header = NULL;
    free(src);
    src = NULL;
    return ret_val;
}

// Free an on-disk page to the free page list
// almost not in use in project3
void file_free_page(int table_id, pagenum_t pagenum){
    page_t *header, *cur;
    
    header = (page_t*)malloc(sizeof(page_t));
    cur = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, 0, header);
    file_read_page(table_id, pagenum, cur);

    cur->_free.free_page = header->_header.free_page;
    header->_header.free_page = pagenum;

    file_write_page(table_id, 0, header);
    file_write_page(table_id, pagenum, cur);

    free(header);
    header = NULL;
    free(cur);
    cur = NULL;
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest){
    if(table_id > table_cnt)
        return;
    lseek(table_ids[table_id].fd, pagenum * PAGE_SIZE, SEEK_SET);
    ssize_t size = read(table_ids[table_id].fd, (void*)dest, PAGE_SIZE);
    if(size < PAGE_SIZE || size == (ssize_t)-1){
        perror("Read page");
        exit(EXIT_FAILURE);
    }
}

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src){
    if(table_id > table_cnt)
        return;
    lseek(table_ids[table_id].fd, pagenum * PAGE_SIZE, SEEK_SET);
    ssize_t size = write(table_ids[table_id].fd, (const void*)src, PAGE_SIZE);
    int state = fsync(table_ids[table_id].fd);
    if(state < 0 || size < PAGE_SIZE || size == (ssize_t)-1){
        perror("Write page");
        exit(EXIT_FAILURE);
    }
}
