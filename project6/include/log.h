#ifndef __LOG_H__
#define __LOG_H__

int log_init(int flag, int log_num, char* log_path, char* logmsg_path);
int flush();
int flush(log_t* log_obj);
int record_log(int trx_id, int log_type);
int record_log(int trx_id, int log_type, int table_id, pagenum_t pagenum, 
               int offset, char* old_image, char* new_image);
int record_log(int trx_id, int log_type, int table_id, pagenum_t pagenum, 
               int offset, char* old_image, char* new_image, int next_undo_LSN);
#endif /* __LOG_H__ */