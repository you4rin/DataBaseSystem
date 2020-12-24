#include <string.h>
#include "../include/lock_table.h"
#include "../include/defines.h"
#include "../include/buffer.h"
#include "../include/transaction.h"

std::unordered_map<std::pair<int, int64_t>, lock_table_t*> lock_table;
pthread_mutex_t lock_manager_latch = PTHREAD_MUTEX_INITIALIZER;

int is_locked(pthread_mutex_t* mutex){
	int ret_val = pthread_mutex_trylock(mutex);
	if(ret_val == EINVAL || ret_val == EFAULT){
		perror("is_locked");
		exit(EXIT_FAILURE);
	}
	if(ret_val == EBUSY)
		return 1;
	pthread_mutex_unlock(mutex);
	return 0;
}

int init_table_idx(int table_id, int64_t key){
	ENTRY = (lock_table_t*)malloc(sizeof(lock_table_t));
	HEAD = (lock_t*)malloc(sizeof(lock_t));
	TAIL = (lock_t*)malloc(sizeof(lock_t));
	if(ENTRY == NULL || HEAD == NULL || TAIL == NULL)
		return ALLOCATION_FAILURE;
	ENTRY->table_id = table_id;
	ENTRY->key = key;
	ENTRY->x_cnt = 0;
	HEAD->next = TAIL;
	HEAD->prev = TAIL;
	HEAD->sentinel = ENTRY;
	TAIL->next = HEAD;
	TAIL->prev = HEAD;
	TAIL->sentinel = ENTRY;
	return 0;
}

void push_lock_table(int table_id, int64_t key, lock_t* new_lock){
	new_lock->sentinel = ENTRY;
	TAIL->prev->next = new_lock;
	new_lock->prev = TAIL->prev;
	TAIL->prev = new_lock;
	new_lock->next = TAIL;
}

void pop_lock_table(lock_t* lock_obj){
	lock_obj->prev->next = lock_obj->next;
	lock_obj->next->prev = lock_obj->prev;
}

int
init_lock_table()
{
	// Do Nothing
	return 0;
}

// acquired: page latch
lock_t*
lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode)
{
	pthread_mutex_lock(&trx_manager_latch);
	pthread_mutex_lock(&lock_manager_latch);
	auto is_new = lock_table.find({table_id, key});
	lock_t *new_lock = (lock_t*)malloc(sizeof(lock_t)), *temp = NULL;
	new_lock->cond = PTHREAD_COND_INITIALIZER;
	std::vector<int> edgelog;
	new_lock->lock_mode = lock_mode;
	new_lock->owner_id = trx_id;
	int state = 0, abort_num;
	bool flag = false, deadlock = false;
	if(new_lock == NULL){
		pthread_mutex_unlock(&trx_manager_latch);
		pthread_mutex_unlock(&lock_manager_latch);
		return NULL;
	}

	//LCK[{table_id, key}] = new_lock;
	//if(ENTRY->lock_mode != 1 && ENTRY->lock_mode != 0)
	//	printf("ckp2: %d\n", ENTRY->lock_mode);

	auto it = lock_table.find({table_id, key});
	if(it == lock_table.end())
		state = init_table_idx(table_id, key);
	if(state){
		pthread_mutex_unlock(&trx_manager_latch);
		pthread_mutex_unlock(&lock_manager_latch);
		return NULL;
	}
	auto exists = LCK.find({table_id, key});
	if(!(exists == LCK.end()) && exists->second != NULL){
		// X - X -> return
		// S - S -> return
		// S - X -> deadlock if X exists, else replace
		// X - S -> return

		temp = exists->second->next;
		if(exists->second->lock_mode != SHARED || lock_mode != EXCLUSIVE){
			free(new_lock);
			pthread_mutex_unlock(&lock_manager_latch);
			pthread_mutex_unlock(&trx_manager_latch);
			return exists->second;
		}
		else{
			while(temp != TAIL){
				if(temp->lock_mode == EXCLUSIVE){
					deadlock = true;
					break;
				}
				temp = temp->next;
			}
			if(deadlock){
				free(new_lock);
				new_lock = NULL;
				TRX.abort = true;
				pthread_mutex_unlock(&lock_manager_latch);
				pthread_mutex_unlock(&trx_manager_latch);
				trx_manager_abort(trx_id);
				return NULL;
			}
			else{
				pop_lock_table(exists->second);
				free(exists->second);
			}
		}
	}
	
	LCK[{table_id, key}] = new_lock;

	
	//if(ENTRY->lock_mode != 1 && ENTRY->lock_mode != 0)
	//	printf("ckp3: %d\n", ENTRY->lock_mode);

	push_lock_table(table_id, key, new_lock);

	if(HEAD->next != new_lock && ENTRY->x_cnt)
		flag = true;

	temp = new_lock->prev;
	// Entry's lock mode is EXCLUSIVE if one or more EXCLUSIVE lock_t exists
	if(new_lock->lock_mode == EXCLUSIVE){
		if(temp != HEAD && temp->lock_mode == EXCLUSIVE){
			// X - X
			TRX.outdeg[temp->owner_id] = 1;
			edgelog.push_back(temp->owner_id);
		}
		while(temp != HEAD && temp->lock_mode == SHARED){
			TRX.outdeg[temp->owner_id] = 1;
			edgelog.push_back(temp->owner_id);
			temp = temp->prev;
		}
		ENTRY->x_cnt++;
		if(new_lock != HEAD->next)
			flag = true;
	}
	else if(ENTRY->x_cnt){
		while(temp != HEAD && temp->lock_mode == SHARED)
			temp = temp->prev;
		if(temp != HEAD){
			TRX.outdeg[temp->owner_id] = 1;
			edgelog.push_back(temp->owner_id);
			temp = temp->prev;
		}
	}

	//if(ENTRY->lock_mode != 1 && ENTRY->lock_mode != 0)
	//	printf("ckp4: %d\n", ENTRY->lock_mode);

	if(find_deadlock(trx_id, trx_id)){
		TRX.abort = true;
		for(auto i:edgelog){
			TRX.outdeg.erase(i);
		}
		if(LCK[{table_id, key}] == new_lock)
			LCK[{table_id, key}] = NULL;
		pop_lock_table(new_lock);
		if(lock_mode == EXCLUSIVE)
			ENTRY->x_cnt--;
		free(new_lock);
		pthread_mutex_unlock(&trx_manager_latch);
		pthread_mutex_unlock(&lock_manager_latch);
		trx_manager_abort(trx_id);
		return NULL;
	}

	//if(ENTRY->lock_mode != 1 && ENTRY->lock_mode != 0)
	//	printf("ckp5: %d\n", ENTRY->lock_mode);

	// Sleep if predecessor exists
	pthread_mutex_unlock(&trx_manager_latch);
	if(flag)
		pthread_cond_wait(&new_lock->cond, &lock_manager_latch);

	pthread_mutex_unlock(&lock_manager_latch);
	return new_lock;
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&lock_manager_latch);
	int table_id = lock_obj->sentinel->table_id;
	int trx_id = lock_obj->owner_id;
	int64_t key = lock_obj->sentinel->key;
	lock_t* temp;
	bool flag = false;

	temp = lock_obj->next;
	if(lock_obj != HEAD->next || (lock_obj->lock_mode == SHARED && lock_obj->next->lock_mode == SHARED)){
		if(ENTRY->x_cnt){
			while(temp != TAIL && temp->lock_mode == SHARED)
				temp = temp->next;
			if(temp != TAIL){
				trx_table[temp->owner_id].outdeg.erase(trx_id);
			}
		}
		flag = true;
	}

	pop_lock_table(lock_obj);

	while(temp != TAIL && !flag){
		if(temp->lock_mode == EXCLUSIVE || 
		  (temp->next != TAIL && temp->next->lock_mode == EXCLUSIVE))
			flag = true;
		trx_table[temp->owner_id].outdeg.erase(trx_id);
		pthread_cond_signal(&temp->cond);
		if(!flag)
			temp = temp->next;
	}
	if(lock_obj->lock_mode == EXCLUSIVE)
		ENTRY->x_cnt--;

	LCK[{table_id, key}] = NULL;
	free(lock_obj);

	pthread_mutex_unlock(&lock_manager_latch);
	return 0;
}

void lock_wait(lock_t* lock_obj){
	
}
