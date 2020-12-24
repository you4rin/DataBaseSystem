#include <cassert>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include <iostream>
#include <string>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mutex>
#include <stdbool.h>
#include "../include/defines.h"
#include "../include/bpt.h"
#include "../include/db.h"
#include "../include/file.h"
#include "../include/buffer.h"
#include "../include/transaction.h";

// MAIN
//int a[10000001];

#define DATABASE_BUFFER_SIZE	(100)


typedef int			TableId;
typedef int64_t		Key;
typedef int			TransactionId;
typedef char		Value[120];

TableId	table_id_array[20];

typedef struct {
	TableId	table_id;
	Key		key;
} TableIdKey;

/* compare pairs of table id & key */
int compare_tik(const void* first, const void* second)
{
	TableIdKey* left = (TableIdKey*) first;
	TableIdKey* right = (TableIdKey*) second;

    if (left->table_id < right->table_id)
		return -1;
    if (left->table_id > right->table_id)
		return 1;
	/* table id is same */
	if (left->key < right->key)
		return -1;
	if (left->key > right->key)
		return 1;
	return 0;
}

/* sort pairs of table id & key */
void
sort_table_id_key(TableId table_ids[], Key keys[], int count)
{
	TableIdKey* tik = (TableIdKey*) malloc(sizeof(TableIdKey) * count);
	/* length of array >= count * 2 */
	for (int i = 0; i < count; i++) {
		tik[i].table_id = table_ids[i];
		tik[i].key = keys[i];
	}
	qsort(tik, count, sizeof(TableIdKey), compare_tik);
	for (int i = 0; i < count; i++) {
		table_ids[i] = tik[i].table_id;
		keys[i] = tik[i].key;
	}
	free(tik);
}





/******************************************************************************
 * single thread test (STT)
 */

#define SST_TABLE_NUMBER		(1)
#define SST_TABLE_SIZE			(100)
#define SST_OPERATION_NUMBER	(100)

pthread_mutex_t SST_mutex;
int64_t SST_operation_count;

void*
SST_func(void* args)
{
	int64_t			operation_count;
	TableId			table_id;
	Key				key1, key2;
	Value			value;
	Value			ret_val;
	TransactionId	transaction_id;
	int				ret;


	for (;;) {
		pthread_mutex_lock(&SST_mutex);
		operation_count = SST_operation_count++;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count > SST_OPERATION_NUMBER)
			break;

		table_id = table_id_array[rand() % SST_TABLE_NUMBER];
		key1 = rand() % SST_TABLE_SIZE;
		key2 = rand() % SST_TABLE_SIZE;
		sprintf(value, "%ld", key2);

		if (key1 == key2)
			/* Avoid accessing the same record twice. */
			continue;

		transaction_id = trx_begin();

		ret = db_find(table_id, key1, ret_val, transaction_id);
		if (ret != 0) {
			printf("INCORRECT: fail to db_find()\n");
			return NULL;
		}
		if (atoi(ret_val) != 0 && atoi(ret_val) != key1) {
			printf("INCORRECT: value is wrong\n");
			return NULL;
		}

		ret = db_update(table_id, key2, value, transaction_id);
		if (ret != 0) {
			printf("INCORRECT: fail to db_update()\n");
			return NULL;
		}

		trx_commit(transaction_id);
	}

	return NULL;
}

/* simple single thread test */
void
single_thread_test()
{
	pthread_t	thread;
	int64_t		operation_count_0;
	int64_t		operation_count_1;

	/* Initiate variables for test. */
	SST_operation_count = 0;
	pthread_mutex_init(&SST_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE);

	/* open table */
	for (int i = 0; i < SST_TABLE_NUMBER; i++) {
		char* str = (char*) malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%02d.db", i);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < SST_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	pthread_create(&thread, 0, SST_func, NULL);

	for (;;) {
		pthread_mutex_lock(&SST_mutex);
		operation_count_0 = SST_operation_count;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count_0 > SST_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&SST_mutex);
		operation_count_1 = SST_operation_count;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count_1 > SST_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	pthread_join(thread, NULL);

	/* close table */
	for (int i = 0; i < SST_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}


/******************************************************************************
 * s-lock test (SLT)
 * s-lock only test
 */

#define SLT_TABLE_NUMBER		(1)
#define SLT_TABLE_SIZE			(100)
#define SLT_THREAD_NUMBER		(10)

#define SLT_FIND_NUMBER			(10)

#define SLT_OPERATION_NUMBER	(100000)

pthread_mutex_t SLT_mutex;
int64_t SLT_operation_count;

void*
SLT_func(void* args)
{
	int64_t			operation_count;
	TableId			table_ids[SLT_FIND_NUMBER];
	Key				keys[SLT_FIND_NUMBER];
	Value			ret_val;
	TransactionId	transaction_id;
	int				ret;


	for (;;) {
		pthread_mutex_lock(&SLT_mutex);
		operation_count = SLT_operation_count++;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count > SLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < SLT_FIND_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % SLT_TABLE_NUMBER];
			keys[i] = rand() % SLT_TABLE_SIZE;
		}
		sort_table_id_key(table_ids, keys, SLT_FIND_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < SLT_FIND_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i-1] && keys[i] == keys[i-1])
				/* Avoid accessing the same record twice. */
				continue;

			ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
			if (ret != 0) {
				printf("INCORRECT: fail to db_find()\n");
				return NULL;
			}
			//printf("real value: %d, my value: %d\n", keys[i], atoi(ret_val));
			if (atoi(ret_val) != keys[i]) {
				printf("INCORRECT: value is wrong\n");
				return NULL;
			}
		}

		/* transaction commit */
		trx_commit(transaction_id);
	}

	return NULL;
}

/* s-lock test */
void
slock_test()
{
	pthread_t	threads[SLT_THREAD_NUMBER];
	int64_t		operation_count_0;
	int64_t		operation_count_1;

	/* Initiate variables for test. */
	SLT_operation_count = 0;
	pthread_mutex_init(&SLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE);

	/* open table */
	for (int i = 0; i < SLT_TABLE_NUMBER; i++) {
		char* str = (char*) malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%02d.db", i);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < SLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < SLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, SLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&SLT_mutex);
		operation_count_0 = SLT_operation_count;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count_0 > SLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&SLT_mutex);
		operation_count_1 = SLT_operation_count;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count_1 > SLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	for (int i = 0; i < SLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < SLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}


/******************************************************************************
 * x-lock test (SLT)
 * x-lock only test without deadlock
 */

#define XLT_TABLE_NUMBER		(1)
#define XLT_TABLE_SIZE			(100)
#define XLT_THREAD_NUMBER		(10)

#define XLT_UPDATE_NUMBER		(10)

#define XLT_OPERATION_NUMBER	(100000)

pthread_mutex_t XLT_mutex;
int64_t XLT_operation_count;

void*
XLT_func(void* args)
{
	int64_t			operation_count;
	TableId			table_ids[XLT_UPDATE_NUMBER];
	Key				keys[XLT_UPDATE_NUMBER];
	Value			val;
	TransactionId	transaction_id;
	int				ret;


	for (;;) {
		pthread_mutex_lock(&XLT_mutex);
		operation_count = XLT_operation_count++;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count > XLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < XLT_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % XLT_TABLE_NUMBER];
			keys[i] = rand() % XLT_TABLE_SIZE;
		}
		/* sorting for avoiding deadlock */
		sort_table_id_key(table_ids, keys, XLT_UPDATE_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < XLT_UPDATE_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i-1] && keys[i] == keys[i-1])
				/* Avoid accessing the same record twice. */
				continue;

			sprintf(val, "%ld", keys[i]);
			ret = db_update(table_ids[i], keys[i], val, transaction_id);
			if (ret != 0) {
				printf("INCORRECT: fail to db_find()\n");
				return NULL;
			}
		}

		/* transaction commit */
		trx_commit(transaction_id);
	}

	return NULL;
}

/* x-lock test */
void
xlock_test()
{
	pthread_t	threads[XLT_THREAD_NUMBER];
	int64_t		operation_count_0;
	int64_t		operation_count_1;

	/* Initiate variables for test. */
	XLT_operation_count = 0;
	pthread_mutex_init(&XLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE);

	/* open table */
	for (int i = 0; i < XLT_TABLE_NUMBER; i++) {
		char* str = (char*) malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%02d.db", i);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < XLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key) 0);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < XLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, XLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&XLT_mutex);
		operation_count_0 = XLT_operation_count;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count_0 > XLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&XLT_mutex);
		operation_count_1 = XLT_operation_count;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count_1 > XLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	for (int i = 0; i < XLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < XLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}


/******************************************************************************
 * mix-lock test (MLT)
 * mix-lock test without deadlock
 */

#define MLT_TABLE_NUMBER		(1)
#define MLT_TABLE_SIZE			(100)
#define MLT_THREAD_NUMBER		(10)

#define MLT_FIND_UPDATE_NUMBER	(20)

#define MLT_OPERATION_NUMBER	(100000)

pthread_mutex_t MLT_mutex;
int64_t MLT_operation_count;

void*
MLT_func(void* args)
{
	int64_t			operation_count;
	TableId			table_ids[MLT_FIND_UPDATE_NUMBER];
	Key				keys[MLT_FIND_UPDATE_NUMBER];
	Value			val;
	Value			ret_val;
	TransactionId	transaction_id;
	int				ret;


	for (;;) {
		pthread_mutex_lock(&MLT_mutex);
		operation_count = MLT_operation_count++;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count > MLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < MLT_FIND_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % MLT_TABLE_NUMBER];
			keys[i] = rand() % MLT_TABLE_SIZE;
		}
		/* sorting for avoiding deadlock */
		sort_table_id_key(table_ids, keys, MLT_FIND_UPDATE_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < MLT_FIND_UPDATE_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i-1] && keys[i] == keys[i-1])
				/* Avoid accessing the same record twice. */
				continue;

			if (rand() % 2 == 0) {
				/* db_find */
				ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
				if (ret != 0) {
					printf("INCORRECT: fail to db_find()\n");
					return NULL;
				}
				if (keys[i] != 0 && (atoi(ret_val) % keys[i]) != 0) {
					printf("INCORRECT: value is wrong\n");
					return NULL;
				}
			} else {
				/* db_update */
				sprintf(val, "%ld", keys[i] * (rand() % 100));
				ret = db_update(table_ids[i], keys[i], val, transaction_id);
				if (ret != 0) {
					printf("INCORRECT: fail to db_find()\n");
					return NULL;
				}
			}

		}

		/* transaction commit */
		trx_commit(transaction_id);
	}

	return NULL;
}

/* mixed lock test */
void
mlock_test()
{
	pthread_t	threads[MLT_THREAD_NUMBER];
	int64_t		operation_count_0;
	int64_t		operation_count_1;

	/* Initiate variables for test. */
	MLT_operation_count = 0;
	pthread_mutex_init(&MLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE);

	/* open table */
	for (int i = 0; i < MLT_TABLE_NUMBER; i++) {
		char* str = (char*) malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%02d.db", i);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < MLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key) key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < MLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, MLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&MLT_mutex);
		operation_count_0 = MLT_operation_count;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count_0 > MLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&MLT_mutex);
		operation_count_1 = MLT_operation_count;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count_1 > MLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	for (int i = 0; i < MLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < MLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}


/******************************************************************************
 * deadlock test (DLT)
 * mix-lock test with deadlock
 */

#define DLT_TABLE_NUMBER		(1)
#define DLT_TABLE_SIZE			(100)
#define DLT_THREAD_NUMBER		(5)

#define DLT_FIND_UPDATE_NUMBER	(5)

#define DLT_OPERATION_NUMBER	(100000)

pthread_mutex_t DLT_mutex;
int64_t DLT_operation_count;

void*
DLT_func(void* args)
{
	int64_t			operation_count;
	TableId			table_ids[DLT_FIND_UPDATE_NUMBER];
	Key				keys[DLT_FIND_UPDATE_NUMBER];
	Value			val;
	Value			ret_val;
	TransactionId	transaction_id;
	int				ret;
	bool			flag;


	for (;;) {
		pthread_mutex_lock(&DLT_mutex);
		operation_count = DLT_operation_count++;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count > DLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < DLT_FIND_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % DLT_TABLE_NUMBER];
			keys[i] = rand() % DLT_TABLE_SIZE;
		}

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < DLT_FIND_UPDATE_NUMBER; i++) {
			flag = false;
			for (int j = 0; j < i; j++) {
				if (table_ids[i] == table_ids[j] && keys[i] == keys[j]) {
					flag = true;
				}
			}
			if (flag == true)
				/* avoid accessing same record twice */
				continue;

			if (rand() % 2 == 0) {
				/* db_find */
				ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
				if (ret != 0) {
					/* abort */
					break;
				}
				if (keys[i] != 0 && (atoi(ret_val) % keys[i]) != 0) {
					printf("INCORRECT: value is wrong\n");
					return NULL;
				}
			} else {
				/* db_update */
				sprintf(val, "%ld", keys[i] * (rand() % 100));
				ret = db_update(table_ids[i], keys[i], val, transaction_id);
				if (ret != 0) {
					/* abort */
					break;
				}
			}
		}

		trx_commit(transaction_id);
	}

	return NULL;
}

/* dead lock test */
void
deadlock_test()
{
	pthread_t	threads[DLT_THREAD_NUMBER];
	int64_t		operation_count_0;
	int64_t		operation_count_1;

	/* Initiate variables for test. */
	DLT_operation_count = 0;
	pthread_mutex_init(&DLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE);

	/* open table */
	for (int i = 0; i < DLT_TABLE_NUMBER; i++) {
		char* str = (char*) malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%02d.db", i);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < DLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key) key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < DLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, DLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&DLT_mutex);
		operation_count_0 = DLT_operation_count;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count_0 > DLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&DLT_mutex);
		operation_count_1 = DLT_operation_count;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count_1 > DLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	for (int i = 0; i < DLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < DLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}

void* func1(void* arg){
    char* value = (char*)malloc(sizeof(char) * VALUE_LENGTH);
    int trx_id = trx_begin();
    if(trx_id == 1){
        if(db_find(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("find complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
        sleep(4);
        strcpy(value, (std::to_string(0) + " value").c_str());
        if(db_update(1, 2, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 2, value, trx_id);
        }
    }
    if(trx_id == 2){
        if(db_find(1, 2, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("find complete: key: %d, value: %s, trx_id: %d\n", 2, value, trx_id);
        }
        sleep(4);
        strcpy(value, (std::to_string(0) + " value").c_str());
        if(db_update(1, 3, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 3, value, trx_id);
        }
    }
    if(trx_id == 3){
        if(db_find(1, 3, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("find complete: key: %d, value: %s, trx_id: %d\n", 3, value, trx_id);
        }
        sleep(4);
        strcpy(value, (std::to_string(0) + " value").c_str());
        if(db_update(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
    }
    trx_commit(trx_id);
    free(value);
}



void* func2(void* arg){
    char* value = (char*)malloc(sizeof(char) * VALUE_LENGTH);
    int trx_id = trx_begin();
    if(trx_id%5000){
        if(db_find(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("find complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
    }
    else{
        strcpy(value, (std::to_string(trx_id) + " value").c_str());
        if(db_update(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
    }
    trx_commit(trx_id);
    printf("trx %d committed\n", trx_id);
    free(value);
}


void* func3(void* arg){
    char* value = (char*)malloc(sizeof(char) * VALUE_LENGTH);
    int trx_id = trx_begin();
    if(trx_id == 1){
        strcpy(value, (std::to_string(111) + " value").c_str());
        for(int i = 2; i <= 100; ++i){
            if(db_update(1, i, value, trx_id)==ABORT){
                printf("trx %d aborted..\n", trx_id);
            }
            else{
                printf("update complete: key: %d, value: %s, trx_id: %d\n", i, value, trx_id);
            }
        }
        if(db_find(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("find complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
        sleep(2);
        strcpy(value, (std::to_string(111) + " value").c_str());
        if(db_update(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
    }
    else{
        strcpy(value, (std::to_string(222) + " value").c_str());
        if(db_update(1, 1, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            printf("update complete: key: %d, value: %s, trx_id: %d\n", 1, value, trx_id);
        }
    }
    if(trx_commit(trx_id))
        printf("trx %d committed\n", trx_id);
    free(value);
}

void* func4(void* arg){
    char* value = (char*)malloc(sizeof(char) * VALUE_LENGTH);
    int trx_id = trx_begin();
    int key = (rand()%1000)+1;
    int modify = !!(rand()%100);
    
    if(!modify){
        strcpy(value, (std::to_string(trx_id) + " modified").c_str());
        if(db_update(1, key, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            //printf("update complete: key: %d, value: %s, trx_id: %d\n", key, value, trx_id);
        }
    }
    else{
        if(db_find(1, key, value, trx_id)==ABORT){
            printf("trx %d aborted..\n", trx_id);
        }
        else{
            //("find complete: key: %d, value: %s, trx_id: %d\n", key, value, trx_id);
        }
    }
    trx_commit(trx_id);
    free(value);
}

int main(int argc, char** argv) {

    char* input_file;
    char output_file[2000];
    FILE* fp;
    node* root;
    page_t* header;
    int input, range2;
    key_type key;
    char* value = (char*)malloc(sizeof(char) * VALUE_LENGTH);
    char instruction;
    char license_part;
    int ret_val;

    root = NULL;
    verbose_output = false;

    srand(time(NULL));

    /*
    int trx_arr[10001];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for(int i=1;i<=10000;++i){
        trx_arr[i] = trx_begin();
        assert(trx_arr[i] == i);
    }
    for(int i=1;i<=10000;++i){
        ret_val = db_find(tid1, 1, value, trx_arr[i]);
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[i]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[i], value);
    }
    for(int i=1;i<=10000;++i){
        ret_val = trx_commit(trx_arr[i]);
        assert(ret_val == i);
    }
    shutdown_db();
    */
    /*
    int trx_arr[10001];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for(int i=1;i<=10000;++i){
        trx_arr[i] = trx_begin();
        assert(trx_arr[i] == i);
    }
    for(int i=1;i<=4999;++i){
        ret_val = db_find(tid1, 1, value, trx_arr[i]);
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[i]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[i], value);
    }
    printf("checkpoint 1\n");

    strcpy(value, "2 value");
    ret_val = db_update(tid1, 1, value, trx_arr[5000]);
    if(ret_val == ABORT)
        printf("trx_id %d aborted..\n", trx_arr[5000]);
    else
        printf("trx_id: %d, value: %s\n", trx_arr[5000], value);
    printf("checkpoint 2\n");

    for(int i=5001;i<=9999;++i){
        ret_val = db_find(tid1, 1, value, trx_arr[i]);
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[i]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[i], value);
    }
    printf("checkpoint 3\n");
    
    strcpy(value, "3 value");
    ret_val = db_update(tid1, 1, value, trx_arr[10000]);
    if(ret_val == ABORT)
        printf("trx_id %d aborted..\n", trx_arr[10000]);
    else
        printf("trx_id: %d, value: %s\n", trx_arr[10000], value);
    printf("checkpoint 4\n");

    for(int i=4999;i>0;--i){
        ret_val = trx_commit(trx_arr[i]);
        assert(ret_val == i);
    }
    printf("checkpoint 5\n");

    ret_val = trx_commit(trx_arr[5000]);
    assert(ret_val == 5000);
    printf("checkpoint 6\n");

    for(int i=9999;i>5000;--i){
        ret_val = trx_commit(trx_arr[i]);
        assert(ret_val == i);
    }
    printf("checkpoint 7\n");

    ret_val = trx_commit(trx_arr[10000]);
    assert(ret_val == 10000);
    printf("checkpoint 8\n");

    shutdown_db();
    */
    /*
    int trx_arr[10001];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for(int i=1;i<=4;++i){
        trx_arr[i] = trx_begin();
        assert(trx_arr[i] == i);
    }
    for(int i=1;i<=5000;++i){
        ret_val = db_find(tid1, 1, value, trx_arr[1]);
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[1]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[1], value);
    }
    printf("checkpoint 1\n");

    for(int i=1;i<=5000;++i){
        strcpy(value, (std::to_string(i) + " value").c_str());
        ret_val = db_update(tid1, 2, value, trx_arr[2]);
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[2]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[2], value);
    }
    printf("checkpoint 2\n");

    for(int i=1;i<=5000;++i){
        if(i == 1){
            strcpy(value, (std::to_string(i) + " value").c_str());
            ret_val = db_update(tid1, 3, value, trx_arr[3]);
        }
        else
            ret_val = db_find(tid1, 3, value, trx_arr[3]);
        
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[3]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[3], value);
    }
    printf("checkpoint 3\n");

    for(int i=1;i<=5000;++i){
        if(i != 1){
            strcpy(value, (std::to_string(i) + " value").c_str());
            ret_val = db_update(tid1, 4, value, trx_arr[4]);
        }
        else
            ret_val = db_find(tid1, 4, value, trx_arr[4]);
        
        if(ret_val == ABORT)
            printf("trx_id %d aborted..\n", trx_arr[4]);
        else
            printf("trx_id: %d, value: %s\n", trx_arr[4], value);
    }
    printf("checkpoint 4\n");

    for(int i=1;i<=4;++i){
        ret_val = trx_commit(trx_arr[i]);
        assert(ret_val == i);
    }

    shutdown_db();
    */
    /*
    pthread_t trx_threads[3];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for (int i = 0; i < 3; i++) {
		pthread_create(&trx_threads[i], 0, func1, NULL);
        sleep(1);
    }

    for(int i = 0; i < 3; i++){
        pthread_join(trx_threads[i], NULL);
    }
    int scan_trx = trx_begin();
    for(int i = 1; i < 4; i++){
        db_find(1, i, value, scan_trx);
        printf("key: %d, value: %s\n", i, value);
        strcpy(value, (std::to_string(100) + " value").c_str());
        db_update(1, i, value, scan_trx);
        printf("key: %d, value: %s\n", i, value);
        db_find(1, i, value, scan_trx);
        printf("key: %d, value: %s\n", i, value);
    }
    trx_commit(scan_trx);
    */
    
    /*
    pthread_t trx_threads[10001];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for(int i = 0; i < 10000; ++i){
        pthread_create(&trx_threads[i], 0, func2, NULL);
    }

    for(int i = 0; i < 10000; ++i){
        pthread_join(trx_threads[i], NULL);
    }
    */
    
    /*
    pthread_t trx_threads[2];
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for (int i = 0; i < 2; i++) {
		pthread_create(&trx_threads[i], 0, func3, NULL);
        sleep(2);
    }
    for(int i = 0; i < 2; i++){
        pthread_join(trx_threads[i], NULL);
    }
    int scan_trx = trx_begin();
    for(int i = 1; i < 101; i++){
        db_find(1, i, value, scan_trx);
        printf("%s\n", value);
    }
    */

    /*
    pthread_t trx_threads[10001];
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    for(int i = 0; i < 10000; ++i){
        pthread_create(&trx_threads[i], 0, func4, NULL);
    }

    for(int i = 0; i < 10000; ++i){
        pthread_join(trx_threads[i], NULL);
    }
    
    int scan_trx = trx_begin();
    for(int i = 1; i < 1001; i++){
        db_find(1, i, value, scan_trx);
        printf("%s\n", value);
    }
    trx_commit(scan_trx);
    shutdown_db();
    */
    
    free(value);

    srand(123);

	if (argc != 2) {
		printf("Choose a workload\n");
		printf("./test single_thread_test\n");
		printf("./test slock_test\n");
		printf("./test xlock_test\n");
		printf("./test mlock_test\n");
		printf("./test deadlock_test\n");
		return 0;
	}

	if (strcmp("single_thread_test", argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
		single_thread_test();
	}
	if (strcmp("slock_test", argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
		slock_test();
	}
	if (strcmp("xlock_test", argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
		xlock_test();
	}
	if (strcmp("mlock_test", argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
		mlock_test();
	}
	if (strcmp("deadlock_test", argv[1]) == 0 || strcmp("all", argv[1]) == 0) {
		deadlock_test();
	}

	return 0;

    return EXIT_SUCCESS;
}
