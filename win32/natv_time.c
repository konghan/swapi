/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

#include "natv_time.h"

#include "swapi_sys_thread.h"
#include "list.h"

#include <windows.h>

struct natv_timer{
	unsigned long		nt_elapse;

	natv_timer_proc		nt_proc;
	void				*nt_data;

	HANDLE				nt_handle;

	struct list_head	nt_node;
};

typedef struct natv_timer_win32{
	int					ntw_init;
	HANDLE				ntw_timer_queue;

	swapi_spinlock_t	ntw_lock;
	struct list_head	ntw_timers;
}natv_timer_win32_t;

static natv_timer_win32_t	__gs_timer = {};

static inline natv_timer_win32_t *get_timer(){
	return &__gs_timer;
}

static void CALLBACK timer_routine(PVOID lpParam, BOOLEAN TimerOrWaitFired){
	struct natv_timer	*t = (struct natv_timer *)lpParam;
	ASSERT(t != NULL);

	t->nt_proc(t, t->nt_data);
}

void *natv_timer_get_data(natv_timer_t timer){
	ASSERT(timer != NULL);

	return timer->nt_data;
}

unsigned long natv_timer_get_elapse(natv_timer_t timer){
	ASSERT(timer != NULL);

	return timer->nt_elapse;
}

int natv_timer_create(unsigned long elapse, natv_timer_proc proc, void *data,
		natv_timer_t *timer){
	natv_timer_win32_t	*ntw = get_timer();
	struct natv_timer	*t;

	t = malloc(sizeof(*t));
	if(t == NULL){
		return -ENOMEM;
	}
	memset(t, 0, sizeof(*t));

	t->nt_proc = proc;
	t->nt_elapse = elapse;
	t->nt_data = data;
	INIT_LIST_HEAD(&t->nt_node);

	if(!CreateTimerQueueTimer(&t->nt_handle, ntw->ntw_timer_queue,
				(WAITORTIMERCALLBACK)timer_routine, t, elapse, elapse, 0)){
		free(t);
		return -1;
	}

	swapi_spin_lock(&ntw->ntw_lock);
	list_add(&t->nt_node, &ntw->ntw_timers);
	swapi_spin_unlock(&ntw->ntw_lock);

	*timer = t;

	return 0;
}

int natv_timer_destroy(natv_timer_t timer){
	struct natv_timer	*t = timer;
	natv_timer_win32_t	*ntw = get_timer();

	DeleteTimerQueueTimer(ntw->ntw_timer_queue, t->nt_handle, NULL);

	free(t);

	return 0;
}

int natv_timer_module_init(){
	natv_timer_win32_t	*ntw = get_timer();

	ntw->ntw_timer_queue = CreateTimerQueue();
	if(ntw->ntw_timer_queue == NULL){
		return -1;
	}

	swapi_spin_init(&ntw->ntw_lock);
	INIT_LIST_HEAD(&ntw->ntw_timers);
	ntw->ntw_init = 1;
	
	return 0;
}

int natv_timer_module_fini(){
	natv_timer_win32_t	*ntw = get_timer();

	ntw->ntw_init = 0;
	swapi_spin_fini(&ntw->ntw_lock);
	DeleteTimerQueue(ntw->ntw_timer_queue);
		
	return 0;
}

