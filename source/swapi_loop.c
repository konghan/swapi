/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_loop.h"
#include "swapi_queue.h"
#include "swapi_handler.h"
#include "swapi_message.h"
#include "swapi_swap.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

#include "list.h"

#include "swap_clock.h"

typedef struct swapi_loop{
	swapi_spinlock_t	sl_lock;
	int					sl_init;

	swapi_swap_t		*sl_cur;

	struct list_head	sl_swaps;

	swapi_handler_t		*sl_handler;
	swapi_queue_t		*sl_queue;
}swapi_loop_t;

/*
 * main data
 */
static swapi_loop_t		__gs_data; // = {};
static inline swapi_loop_t *get_loop(){
	return &__gs_data;
}

/*
 * default message processor
 */
typedef struct swapi_loop_handler_entry{
	int						she_type;
	swapi_handler_entry_t	she_entry;
}swapi_loop_handler_entry_t;

static int swapi_loop_on_key(swapi_message_t *msg, void *data);
static int swapi_loop_on_timer(swapi_message_t *msg, void *data);
static int swapi_loop_on_gsensor(swapi_message_t *msg, void *data);
static int swapi_loop_on_gps(swapi_message_t *msg, void *data);
static int swapi_loop_on_call(swapi_message_t *msg, void *data);
static int swapi_loop_on_msg(swapi_message_t *msg, void *data);
static int swapi_loop_on_app_data(swapi_message_t *msg, void *data);
static int swapi_loop_on_app_pri(swapi_message_t *msg, void *data);
static int swapi_loop_on_default(swapi_message_t *msg, void *data);

static swapi_loop_handler_entry_t	__gs_handlers[] = {
	{ SWAPI_MSGTYPE_KEYBOARD,		{{NULL, NULL},	swapi_loop_on_key,		&__gs_data }},
	{ SWAPI_MSGTYPE_TIMER,			{{NULL, NULL},	swapi_loop_on_timer,	&__gs_data }},
	{ SWAPI_MSGTYPE_GSENSOR,		{{NULL, NULL},	swapi_loop_on_gsensor,	&__gs_data }},
	{ SWAPI_MSGTYPE_GPS,			{{NULL, NULL},	swapi_loop_on_gps,		&__gs_data }},
	{ SWAPI_MSGTYPE_PHONE_CALL,		{{NULL, NULL},	swapi_loop_on_call,		&__gs_data }},
	{ SWAPI_MSGTYPE_PHONE_MSG,		{{NULL, NULL},	swapi_loop_on_msg,		&__gs_data }},
	{ SWAPI_MSGTYPE_APP_DATA,		{{NULL, NULL},	swapi_loop_on_app_data,	&__gs_data }},
	{ SWAPI_MSGTYPE_APP_PRIVATE,	{{NULL, NULL},	swapi_loop_on_app_pri,	&__gs_data }},
	{ -1,							{{NULL, NULL},	swapi_loop_on_default,	NULL	   }}
};

static swapi_loop_handler_entry_t *get_handlers(){
	return __gs_handlers;
}

/*
 * app init array
 */
typedef int (*swap_init_func)();
typedef int (*swap_fini_func)();
typedef struct swapi_loop_swap{
	swap_init_func		swap_init;
	swap_fini_func		swap_fini;
}swapi_loop_swap_t;

static int swap_default(){
	return 0;
}

static swapi_loop_swap_t	__gs_swaps[] = {
	{ swap_clock_init,	swap_clock_fini},
	{ swap_default,		swap_default}
};
static swapi_loop_swap_t *get_swaps(){
	return __gs_swaps;
}



static int swapi_loop_on_key(swapi_message_t *msg, void *data){
//	switch(msg->mm_type
	return 0;
}

static int swapi_loop_on_timer(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_gsensor(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_gps(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_call(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_msg(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_app_data(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_app_pri(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_on_default(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_loop_init(){
	swapi_loop_t				*sl = get_loop();
	swapi_loop_handler_entry_t	*she;
	swapi_loop_swap_t			*sls;

	ASSERT(sl != NULL);

	INIT_LIST_HEAD(&sl->sl_swaps);
	
	if(swapi_spin_init(&sl->sl_lock) != 0){
		swapi_log_warn("swapi loop init lock fail!\n");
		return -1;
	}

	if(swapi_queue_create(sizeof(swapi_message_t), kSWAPI_QUEUE_DEFAULT_LENGTH, &sl->sl_queue) != 0){
		swapi_log_warn("swapi create queue fail!\n");
		swapi_spin_fini(&sl->sl_lock);
		return -1;
	}

	if(swapi_handler_create(kSWAPI_HANDLER_DEFAULT_SLOTS, &sl->sl_handler) != 0){
		swapi_log_warn("swapi create handler fail!\n");
		swapi_queue_destroy(sl->sl_queue);
		swapi_spin_fini(&sl->sl_lock);
		return -1;
	}

	she = get_handlers();
	while(she->she_type != -1){
		INIT_LIST_HEAD(&she->she_entry.she_node);
		swapi_handler_add(sl->sl_handler, she->she_type, &she->she_entry);
		she++;
	}
	
	sl->sl_cur  = NULL;
	sls = get_swaps();
	while(sls->swap_init != swap_default){
		sls->swap_init();
	}

//	sl->sl_cur = list_first_entry(&sl->sl_swaps, swapi_swap_t, ss_node);

	sl->sl_init = 1;
	
	return 0;
}


static int swapi_loop_setcur(swapi_loop_t *sl, swapi_swap_t *swap){
	// post on_resume message to swap
	return -1;
}

int swapi_loop_add_swap(swapi_swap_t *swap){
	swapi_loop_t *sl = get_loop();

	ASSERT(swap != NULL);

	swapi_spin_lock(&sl->sl_lock);

	list_add(&swap->ss_node, &sl->sl_swaps);
	
	// post on_create message to swap
	
	// post on_start message to swap

	swapi_loop_setcur(sl, swap);

	swapi_spin_unlock(&sl->sl_lock);

	return 0;
}

static int swapi_loop_fini(){
	swapi_loop_t *sl = get_loop();

	ASSERT(sl != NULL);

	sl->sl_init = 0;
	swapi_handler_destroy(sl->sl_handler);
	swapi_queue_destroy(sl->sl_queue);
	swapi_spin_fini(&sl->sl_lock);

	return 0;
}

int swapi_loop_post(swapi_message_t *msg){
	swapi_loop_t *sl = get_loop();

	ASSERT(msg != NULL);

	return swapi_queue_post(sl->sl_queue, msg);
}

int swapi_loop_run(void *p){
	swapi_loop_t	*sl = get_loop();
	swapi_message_t	msg;

	if(swapi_loop_init() != 0){
		swapi_log_warn("swapi loop init fail\n");
		return -1;
	}

	// FIXME: load lua swap

	while(1){
		if(swapi_queue_wait(sl->sl_queue, &msg) != 0){
			swapi_log_warn("swapi loop wait message fail\n");
			break;
		}

		swapi_handler_invoke(sl->sl_handler, &msg);
	}

	swapi_loop_fini();

	return 0;
}