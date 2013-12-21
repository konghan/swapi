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
#include "swap_oral.h"

typedef struct swapi_loop{
	swapi_spinlock_t	sl_lock;
	int					sl_init;

	swapi_swap_t		*sl_cur;

	struct list_head	sl_swaps;

	swapi_swap_t		*sl_clock;	// default clock
	swapi_swap_t		*sl_oral;	// voice messager app
	swapi_swap_t		*sl_srvs;	// services:gps ..., not in sl_swaps

	swapi_handler_t		*sl_handler;
	swapi_queue_t		*sl_queue;
}swapi_loop_t;

/*
 * main data
 */
static swapi_loop_t		__gs_data = {};
static inline swapi_loop_t *get_loop(){
	return &__gs_data;
}

static int swapi_loop_on_key(swapi_message_t *msg, void *data);
static int swapi_loop_on_timer(swapi_message_t *msg, void *data);
static int swapi_loop_on_gsensor(swapi_message_t *msg, void *data);
static int swapi_loop_on_gps(swapi_message_t *msg, void *data);
static int swapi_loop_on_call(swapi_message_t *msg, void *data);
static int swapi_loop_on_msg(swapi_message_t *msg, void *data);
static int swapi_loop_on_app_data(swapi_message_t *msg, void *data);
static int swapi_loop_on_app_pri(swapi_message_t *msg, void *data);
static int swapi_loop_on_default(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handlers[] = {
	{ kSWAPI_MSGTYPE_KEYBOARD,		{NULL, NULL},	swapi_loop_on_key,		&__gs_data },
	{ kSWAPI_MSGTYPE_TIMER,			{NULL, NULL},	swapi_loop_on_timer,	&__gs_data },
	{ kSWAPI_MSGTYPE_GSENSOR,		{NULL, NULL},	swapi_loop_on_gsensor,	&__gs_data },
	{ kSWAPI_MSGTYPE_GPS,			{NULL, NULL},	swapi_loop_on_gps,		&__gs_data },
	{ kSWAPI_MSGTYPE_PHONE_CALL,	{NULL, NULL},	swapi_loop_on_call,		&__gs_data },
	{ kSWAPI_MSGTYPE_PHONE_MSG,		{NULL, NULL},	swapi_loop_on_msg,		&__gs_data },
	{ kSWAPI_MSGTYPE_SWAP_DATA,		{NULL, NULL},	swapi_loop_on_app_data,	&__gs_data },
	{ kSWAPI_MSGTYPE_SWAP_PRIVATE,	{NULL, NULL},	swapi_loop_on_app_pri,	&__gs_data },
	{ kSWAPI_MSGTYPE_DEFAULT,		{NULL, NULL},	swapi_loop_on_default,	NULL	   }
};

static swapi_handler_entry_t *get_handlers(){
	return __gs_handlers;
}

static int swapi_loop_setcur(swapi_loop_t *sl, swapi_swap_t *swap);

static void swapi_loop_prev_swap(){
	swapi_loop_t	*sl = get_loop();
	swapi_swap_t	*swap = sl->sl_cur;

	swapi_spin_lock(&sl->sl_lock);
	if(swap->ss_node.prev == &sl->sl_swaps){
		swap = list_last_entry(&sl->sl_swaps, swapi_swap_t, ss_node);
	}else{
		swap = list_last_entry(&swap->ss_node, swapi_swap_t, ss_node);
	}

	swapi_loop_setcur(sl, swap);
	swapi_spin_unlock(&sl->sl_lock);
}

static void swapi_loop_next_swap(){
	swapi_loop_t	*sl = get_loop();
	swapi_swap_t	*swap = sl->sl_cur;

	swapi_spin_lock(&sl->sl_lock);
	if(swap->ss_node.next == &sl->sl_swaps){
		swap = list_first_entry(&sl->sl_swaps, swapi_swap_t, ss_node);
	}else{
		swap = list_first_entry(&swap->ss_node, swapi_swap_t, ss_node);
	}

	swapi_loop_setcur(sl, swap);
	swapi_spin_unlock(&sl->sl_lock);

}

static int swapi_loop_on_key(swapi_message_t *msg, void *data){
	swapi_swap_t *swap = swapi_loop_topswap();
	int		key, action;

	swapi_key_unpack(msg, &key, &action);

	switch(key){
	case kNATV_KEYDRV_ENTERUP:
		swapi_loop_prev_swap();
		break;

	case kNATV_KEYDRV_ENTERDOWN:
		swapi_loop_next_swap();
		break;

	default:
		swapi_swap_post(swap, msg);
	}

	return 0;
}

static int swapi_loop_on_timer(swapi_message_t *msg, void *data){
	swapi_loop_t	*sl = get_loop();
	swapi_swap_t	*swap;

	swapi_spin_lock(&sl->sl_lock);
	list_for_each_entry(swap, &sl->sl_swaps, ss_node){
		swapi_log_info("post message %d to %s\n", msg->sm_type, swap->ss_name);
		swapi_swap_post(swap, msg);
	}
	swapi_spin_unlock(&sl->sl_lock);
	
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

int swapi_loop_module_init(){
	swapi_loop_t				*sl = get_loop();
	swapi_handler_entry_t		*she;

	ASSERT(sl != NULL);

	INIT_LIST_HEAD(&sl->sl_swaps);
	
	if(swapi_spin_init(&sl->sl_lock) != 0){
		swapi_log_warn("swapi loop init lock fail!\n");
		return -1;
	}

	if(swapi_queue_create(sizeof(swapi_message_t), kSWAPI_QUEUE_DEFAULT_LENGTH, &sl->sl_queue) != 0){
		swapi_log_warn("swapi create queue fail!\n");
		goto exit_queue;
	}

	if(swapi_handler_create(kSWAPI_HANDLER_DEFAULT_SLOTS, &sl->sl_handler) != 0){
		swapi_log_warn("swapi create handler fail!\n");
		goto exit_handler;
	}

	she = get_handlers();
	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		swapi_handler_add(sl->sl_handler, she->she_type, she);
		she++;
	}
	sl->sl_cur  = NULL;

	if(swap_clock_init(&sl->sl_clock) != 0){
		swapi_log_warn("swap clock init fail\n");
		goto exit_clock;
	}
	swapi_loop_add_swap(sl->sl_clock);

	if(swap_oral_init(&sl->sl_oral) != 0){
		swapi_log_warn("swap oral init fail\n");
		goto exit_oral;
	}
	swapi_loop_add_swap(sl->sl_oral);

	sl->sl_init = 1;

	return 0;
	
exit_oral:
	swap_clock_fini(sl->sl_clock);

exit_clock:
	swapi_handler_destroy(sl->sl_handler);

exit_handler:
	swapi_queue_destroy(sl->sl_queue);

exit_queue:
	swapi_spin_fini(&sl->sl_lock);

	return -1;
}


static int swapi_loop_setcur(swapi_loop_t *sl, swapi_swap_t *swap){
	swapi_message_t		msg;

	// post on_resume message to swap
	// FIXME: draw top swap
	if(sl->sl_cur != NULL){
		swapi_swap_status_change(sl->sl_cur, kSWAP_MSGTYPE_PAUSE);
	}

	swapi_log_info("change top swap to %s \n", swap->ss_name);

	sl->sl_cur = swap;
	swapi_swap_status_change(swap, kSWAP_MSGTYPE_RESUME);
		
	msg.sm_type = kSWAPI_MSGTYPE_DRAW;
	msg.sm_size = 0;
	swapi_swap_post(swap, &msg);
	
	return -1;
}

int swapi_loop_add_swap(swapi_swap_t *swap){
	swapi_loop_t *sl = get_loop();

	ASSERT(swap != NULL);

	swapi_spin_lock(&sl->sl_lock);

	list_add(&swap->ss_node, &sl->sl_swaps);
	
	swapi_loop_setcur(sl, swap);

	swapi_spin_unlock(&sl->sl_lock);

	return 0;
}

int swapi_loop_module_fini(){
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

	while(1){
		if(swapi_queue_wait(sl->sl_queue, &msg) != 0){
			swapi_log_warn("swapi loop wait message fail\n");
			break;
		}

		swapi_handler_invoke(sl->sl_handler, &msg);
	}

	return 0;
}

swapi_swap_t* swapi_loop_topswap(){
	swapi_loop_t	*sl = get_loop();

	return sl->sl_cur;
}

