/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

#ifndef __NATV_TIME_H__
#define __NATV_TIME_H__

struct natv_timer;
typedef struct natv_timer	*natv_timer_t;

typedef void (*natv_timer_proc)(natv_timer_t timer, void *data);

unsigned long natv_timer_get_elapse(natv_timer_t timer);
void *natv_timer_get_data(natv_timer_t timer);

// elapse in ms
int natv_timer_create(unsigned long elapse, natv_timer_proc proc, void *data,
		natv_timer_t *timer);
int natv_timer_destroy(natv_timer_t timer);


int natv_time_module_init();
int natv_time_module_fini();

#endif // __NATV_TIME_H__

