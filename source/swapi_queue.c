/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_queue.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

struct swapi_queue{
	// modified condition variable, just for internal use.
	__swapi_convar_t	sq_cv;
	
	swapi_spinlock_t	sq_lock;
	int				sq_head;
	int				sq_tail;

	int				sq_size;
	int				sq_numb;

	// for statistic
	uint64_t		sq_in;
	uint64_t		sq_out;
	
	char			sq_slots[1];
};


int swapi_queue_create(int isize, int numb, swapi_queue_t **sq){
	size_t			len;
	int				ret;
	swapi_queue_t	*q;

	ASSERT(sq != NULL);
	
	if((isize <= 0) || (numb <= 0)){
		return -1;
	}

	len = sizeof(swapi_queue_t) + isize*numb;
	q = swapi_heap_alloc(len);
	if(q == NULL){
		swapi_log_warn("no more memory for swapi queue\n");
		return -ENOMEM;
	}
	memset(q, 0, len);

	ret = __swapi_convar_init(&q->sq_cv);
	if(ret != 0){
		swapi_log_warn("init queue fail : %d\n", ret);
		swapi_heap_free(q);
		return ret;
	}

	swapi_spin_init(&q->sq_lock);

	q->sq_size = isize;
	q->sq_numb = numb;

	*sq = q;

	return 0;
}

int swapi_queue_destroy(swapi_queue_t *sq){
	swapi_spin_fini(&sq->sq_lock);
	__swapi_convar_fini(&sq->sq_cv);

	swapi_heap_free(sq);

	return 0;
}

int swapi_queue_post(swapi_queue_t *sq, void *msg){
	int				pos;

	ASSERT(sq != NULL);
	ASSERT(msg != NULL);

	swapi_spin_lock(&sq->sq_lock);

	pos = sq->sq_head + 1;
	if(pos >= sq->sq_numb){
		pos -= sq->sq_numb;
	}

	if(pos == sq->sq_tail){
		swapi_log_warn("queue is full %d\n", (unsigned int)sq);
		swapi_spin_unlock(&sq->sq_lock);
		return -ERANGE;
	}
	
	memcpy(&(sq->sq_slots[sq->sq_head*sq->sq_size]), msg, sq->sq_size);
	sq->sq_head = pos;

	sq->sq_in ++;

	swapi_spin_unlock(&sq->sq_lock);

	__swapi_convar_signal(&sq->sq_cv);

	return 0;
}

int swapi_queue_wait(swapi_queue_t *sq, void *msg){
	int				pos;

	ASSERT(sq != NULL);
	ASSERT(msg != NULL);

	__swapi_convar_wait(&sq->sq_cv);

	swapi_spin_lock(&sq->sq_lock);

	if(sq->sq_tail == sq->sq_head){
		swapi_log_warn("queue without message %d\n", (unsigned int)sq);
		swapi_spin_unlock(&sq->sq_lock);
		return -ENOENT;
	}
	
	memcpy(msg, &(sq->sq_slots[sq->sq_tail*sq->sq_size]), sq->sq_size);

	pos = sq->sq_tail + 1;
	if(pos >= sq->sq_numb){
		pos -= sq->sq_numb;
	}
	sq->sq_tail = pos;

	sq->sq_out ++;

	swapi_spin_unlock(&sq->sq_lock);

	return 0;
}

