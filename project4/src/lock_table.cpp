#include <lock_table.h>

struct lock_t{
	struct lock_t* next = nullptr;
	struct lock_t* prev = nullptr;
	struct lock_table_t* sentinel = nullptr;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
};

struct lock_table_t{
	int table_id;
	int64_t key;
	lock_t* head;
	lock_t* tail;
};

typedef struct lock_t lock_t;
typedef struct lock_table_t lock_table_t;

std::unordered_map<std::pair<int, int64_t>, lock_table_t*> hash_table;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int init_table_idx(int table_id, int64_t key){
	ENTRY = (lock_table_t*)malloc(sizeof(lock_table_t));
	HEAD = (lock_t*)malloc(sizeof(lock_t));
	TAIL = (lock_t*)malloc(sizeof(lock_t));
	if(ENTRY == nullptr || HEAD == nullptr || TAIL == nullptr)
		return ALLOCATION_FAILURE;
	ENTRY->table_id = table_id;
	ENTRY->key = key;
	HEAD->next = TAIL;
	HEAD->prev = TAIL;
	TAIL->next = HEAD;
	TAIL->prev = HEAD;
	return 0;
}

void push_lock_table(int table_id, int64_t key, lock_t* new_lock){
	new_lock->sentinel = ENTRY;
	TAIL->prev->next = new_lock;
	new_lock->prev = TAIL->prev;
	TAIL->prev = new_lock;
	new_lock->next = TAIL;
}

void pop_lock_table(int table_id, int64_t key, lock_t* lock_obj){
	lock_obj->prev->next = lock_obj->next;
	lock_obj->next->prev = lock_obj->prev;
}

int
init_lock_table()
{
	// Do Nothing
	return 0;
}

lock_t*
lock_acquire(int table_id, int64_t key)
{
	pthread_mutex_lock(&mut);
	lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
	int state = 0;
	bool flag = false;
	if(new_lock == nullptr)
		return nullptr;
	auto it = hash_table.find(std::make_pair(table_id, key));
	if(it == hash_table.end())
		state = init_table_idx(table_id, key);
	if(state)
		return nullptr;
	push_lock_table(table_id, key, new_lock);

	// Sleep if predecessor exists
	if(new_lock != HEAD->next)
		flag = true;
	if(flag)
		pthread_cond_wait(&new_lock->cond, &mut);

	pthread_mutex_unlock(&mut);
	return new_lock;
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&mut);
	int table_id = lock_obj->sentinel->table_id;
	int64_t key = lock_obj->sentinel->key;
	if(lock_obj != HEAD->next){
		pthread_mutex_unlock(&mut);
		return UNABLE_TO_RELEASE;
	}

	pop_lock_table(table_id, key, lock_obj);

	if(lock_obj->next != TAIL)
		pthread_cond_signal(&lock_obj->next->cond);

	free(lock_obj);

	pthread_mutex_unlock(&mut);
	return 0;
}
