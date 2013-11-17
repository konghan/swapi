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


typedef struct natv_tm{
	int	tm_sec;		/* Seconds: 0-59 (K&R says 0-61?) */
	int	tm_min;		/* Minutes: 0-59 */
	int	tm_hour;	/* Hours since midnight: 0-23 */
	int	tm_mday;	/* Day of the month: 1-31 */
	int	tm_mon;		/* Months *since* january: 0-11 */
	int	tm_year;	/* Years since 1900 */
	int	tm_wday;	/* Days since Sunday (0-6) */
	int	tm_yday;	/* Days since Jan. 1: 0-365 */
	int	tm_isdst;	/* +1 Daylight Savings Time, 0 No DST,
				 * -1 don't know */
}natv_tm_t;

int natv_time_localtime(natv_tm_t *tm);

int natv_time_module_init();
int natv_time_module_fini();


#endif // __NATV_TIME_H__

