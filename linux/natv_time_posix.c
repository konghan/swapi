/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

#include "natv_time.h"

#include "swapi_sys_thread.h"

#include <time.h>


struct natv_timer{
	unsigned long		nt_elapse;

	natv_timer_proc		nt_proc;

	void				*nt_data;

	timer_t				nt_timer;
};

static void timer_routine(union sigval){
	struct natv_timer	*t;

	t = (struct natv_timer *)sigval.sival_ptr;
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
	struct natv_timer	*t;
	struct sigevent		sev;
	struct itimerspec	it;

	ASSERT((proc != NULL) && (timer != NULL));

	t = malloc(sizeof(struct natv_timer));
	if(t == NULL){
		return -ENOMEM;
	}
	memset(t, 0, sizeof(struct natv_timer));

	t->nt_elapse	= elapse;
	t->nt_proc		= proc;
	t->nt_data		= data;

	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_value.sival_ptr = t;
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = timer_routine;

	if(timer_create(CLOCK_REALTIME, &sev, &t->nt_timer) != 0){
		free(t);
		return -1;
	}

	it.it_interval.tv_sec	= elapse / 1000;
	it.it_interval.tv_nsec	= (elapse % 1000) * 1000;

	it.it_value.tv_sec	= it.it_interval.tv_sec;
	it.it_value.tv_nsec = it.it_value.tv_nsec;
		
	if(timer_settime(t->nt_timer, 0, &it, NULL) != 0){
		timer_delete(t->nt_timer);
		free(t);
		return -1;
	}

	*timer = t;

	return 0;
}

int natv_timer_destroy(natv_timer_t timer){
	struct natv_timer *t = timer;

	timer_delete(t->nt_timer);

	free(t);

	return 0;
}

int natv_timer_module_init(){
	return 0;
}

int natv_timer_module_fini(){
	return 0;
}

