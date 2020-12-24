#ifndef __DB_H__
#define __DB_H__

#include "bpt.h"
#include "file.h"
#include "defines.h"

int init_db(int buf_num);
int open_table (char* pathname);

int db_insert (int table_id, key_type key, char * value);
int db_find (int table_id, key_type key, char* ret_val);
int db_delete (int table_id, key_type key);

int close_table(int table_id);
int shutdown_db(void);

#endif /* __DB_H__ */