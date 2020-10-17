#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../include/file.h"

// Unique table ID
int table_id = -1;

bool file_empty(){
    if(lseek(table_id, 0, SEEK_END) < sizeof(page_t)){
        return 1;
    }
    return 0;
}

int file_open(char* path){
    table_id = open(path, O_CREAT|O_RDWR, 
            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if(file_empty()){
        page_t* header = (page_t*)malloc(sizeof(page_t));
        memset(header, 0, 4096);
        header->_header.free_page = 0;
        header->_header.root_page = 0;
        header->_header.num_of_pages = 1;
        file_write_page(0, header);
        free(header);
    }
    return table_id;
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(){
    page_t *header, *src;
    pagenum_t ret_val;
    
    header = (page_t*)malloc(sizeof(page_t));
    src = (page_t*)malloc(sizeof(page_t));
    memset(src, 0, sizeof(page_t));
    file_read_page(0, header);

    if(header->_header.free_page){
        ret_val = header->_header.free_page;
        file_read_page(ret_val, src);
        header->_header.free_page = src->_free.free_page;
        file_write_page(0, header);
        memset(src, 0, 4096);
        file_write_page(ret_val, src);
        free(header);
        header = NULL;
        free(src);
        src = NULL;
        return ret_val;
    }
    
    //printf("num_of_pages: %lld\n",header->_header.num_of_pages);
    ret_val = header->_header.num_of_pages++;
    file_write_page(0, header);
    file_write_page(ret_val, src);
  
    free(header);
    header = NULL;
    free(src);
    src = NULL;
    return ret_val;
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum){
    page_t *header, *cur;
    
    header = (page_t*)malloc(sizeof(page_t));
    cur = (page_t*)malloc(sizeof(page_t));
    file_read_page(0, header);
    file_read_page(pagenum, cur);

    cur->_free.free_page = header->_header.free_page;
    header->_header.free_page = pagenum;

    file_write_page(0, header);
    file_write_page(pagenum, cur);

    free(header);
    header = NULL;
    free(cur);
    cur = NULL;
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest){
    lseek(table_id, pagenum * sizeof(page_t), SEEK_SET);
    ssize_t state = read(table_id, (void*)dest, sizeof(page_t));
    if(state < 0){
        perror("Read page");
        exit(EXIT_FAILURE);
    }
}

void file_write_page(pagenum_t pagenum, const page_t* src){
    lseek(table_id, pagenum * sizeof(page_t), SEEK_SET);
    write(table_id, (const void*)src, sizeof(page_t));
    ssize_t state = fsync(table_id);
    if(state < 0){
        perror("Write page");
        exit(EXIT_FAILURE);
    }
}
