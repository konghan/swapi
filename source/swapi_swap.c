/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_swap.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

static int swapi_swap_on_swap(swapi_message_t *msg, void *data);
static int swapi_swap_on_key(swapi_message_t *msg, void *data);
static int swapi_swap_on_touch(swapi_message_t *msg, void *data);
static int swapi_swap_on_draw(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{.she_type = kSWAPI_MSGTYPE_SWAP, .she_node = { }, 
		.she_cbfunc = swapi_swap_on_swap, .she_data = NULL},

	{.she_type = kSWAPI_MSGTYPE_KEYBOARD, .she_node = { }, 
		.she_cbfunc = swapi_swap_on_key, .she_data = NULL},

	{.she_type = kSWAPI_MSGTYPE_TOUCH, .she_node = { }, 
		.she_cbfunc = swapi_swap_on_touch, .she_data = NULL},

	{.she_type = kSWAPI_MSGTYPE_DRAW, .she_node = { }, 
		.she_cbfunc = swapi_swap_on_draw, .she_data = NULL},

	{.she_type = kSWAPI_MSGTYPE_DEFAULT, .she_node = { }, 
		.she_cbfunc = NULL, .she_data = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static int swapi_swap_on_swap(swapi_message_t *msg, void *data){
	swapi_swap_t	*swap = (swapi_swap_t *)data;

	ASSERT(swap != NULL);

	swapi_log_info("swap on swap message !\n");
	
	switch(msg->sm_size){
		case kSWAP_MSGTYPE_CREATE:
			swap->ss_cbs->on_create(swap, 0, NULL);
			break;
			
		case kSWAP_MSGTYPE_DESTROY:
			swap->ss_cbs->on_destroy(swap);
			break;

		case kSWAP_MSGTYPE_PAUSE:
			swap->ss_cbs->on_pause(swap);
			break;
			
		case kSWAP_MSGTYPE_RESUME:
			swap->ss_cbs->on_resume(swap);
			break;
	}

	return 0;
}

static int swapi_swap_on_key(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_swap_on_touch(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_swap_on_draw(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_swap_thread_routine(void *p){
	swapi_swap_t		*ss = (swapi_swap_t *)p;
	swapi_window_t		*pos, *win;
	swapi_message_t		msg;

	ASSERT(ss != NULL);

	swapi_log_info("swap thread is running...\n");

	while(ss->ss_status){
		if(swapi_queue_wait(ss->ss_queue, &msg) != 0){
			swapi_log_warn("swap wait message fail\n");
			break;
		}

		swapi_handler_invoke(ss->ss_handler, &msg);
	}

	// FIXME: free app's resource
	
	list_for_each_entry_safe(pos, win, &ss->ss_wins, sw_node){
		swapi_window_destroy(win);
	}
	
	swapi_spin_fini(&ss->ss_lock);
	swapi_handler_destroy(ss->ss_handler);
	swapi_queue_destroy(ss->ss_queue);
	swapi_heap_free(ss);

	return 0;
}


int swapi_swap_create(const char *name, swapi_swap_cbs_t *cbs, swapi_swap_t **swap){
	swapi_swap_t			*ss;
	swapi_window_t			*win;
	swapi_handler_entry_t	*she;

	ASSERT(swap != NULL);
	ASSERT(cbs != NULL);
	ASSERT(name != NULL);

	ss = swapi_heap_alloc(sizeof(swapi_swap_t));
	if(ss == NULL){
		swapi_log_warn("swap alloc memory fail!\n");
		return -1;
	}
	memset(ss, 0, sizeof(swapi_swap_t));

	if(swapi_queue_create(sizeof(swapi_message_t), kSWAPI_QUEUE_DEFAULT_LENGTH, &ss->ss_queue) != 0){
		swapi_log_warn("swap create queue fail!\n");
		goto exit_queue;
	}

	if(swapi_handler_create(kSWAPI_HANDLER_DEFAULT_SLOTS, &ss->ss_handler) != 0){
		swapi_log_warn("swap create handler fail!\n");
		goto exit_handler;
	}

	if(swapi_spin_init(&ss->ss_lock) != 0){
		swapi_log_warn("swap init lock fail!\n");
		goto exit_spin;
	}

	INIT_LIST_HEAD(&ss->ss_wins);
	INIT_LIST_HEAD(&ss->ss_node);
	strncpy(ss->ss_name, name, kSWAPI_SWAP_NAME_LEN-1);
	ss->ss_cbs = cbs;
	ss->ss_status =  1;

	// create default window
	if(swapi_window_create(&win) != 0){
		swapi_log_warn("swap create default view fail\n");
		goto exit_window;
	}
	swapi_swap_push_window(ss, win);

	// add default handler
	she = get_handler();
	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		she->she_data = ss;

		swapi_handler_add(ss->ss_handler, she->she_type, she);

		she++;
	}

	// swap in lifecycle
	swapi_swap_status_change(ss, kSWAP_MSGTYPE_CREATE);

	if(swapi_thread_create(&ss->ss_thrd, swapi_swap_thread_routine, ss) != 0){
		swapi_log_warn("swap create swap thread fail!\n");
		goto exit_thread;
	}

	*swap = ss;

	return 0;

exit_thread:
	swapi_window_destroy(win);

exit_window:
	swapi_spin_fini(&ss->ss_lock);

exit_spin:
	swapi_handler_destroy(ss->ss_handler);

exit_handler:
	swapi_queue_destroy(ss->ss_queue);

exit_queue:
	swapi_heap_free(ss);

	return -1;
}

int swapi_swap_destroy(swapi_swap_t *ss){
	ASSERT(ss != NULL);

	swapi_swap_status_change(ss, kSWAP_MSGTYPE_DESTROY);

	return 0;
}

int swapi_swap_post(swapi_swap_t *ss, swapi_message_t *msg){
	ASSERT(ss != NULL);
	ASSERT(msg != NULL);

	return swapi_queue_post(ss->ss_queue, msg);
}

int swapi_swap_push_window(swapi_swap_t *swap, swapi_window_t *win){
	ASSERT((swap != NULL) && (win != NULL));

	swapi_spin_lock(&swap->ss_lock);
	list_add(&win->sw_node, &swap->ss_wins);
	swapi_spin_unlock(&swap->ss_lock);

	swap->ss_wincur = win;

	return 0;
}

int swapi_swap_pop_window(swapi_swap_t *swap, swapi_window_t **win){
	swapi_window_t	*w;

	ASSERT((swap != NULL) && (win != NULL));

	w = list_first_entry(&swap->ss_wins, swapi_window_t, sw_node);
	if(swap->ss_wincur != w){
		return -EINVAL;
	}

	if(list_is_last(&w->sw_node, &swap->ss_wins)){
		return -EINVAL;
	}

	swapi_spin_lock(&swap->ss_lock);
	list_del_init(&w->sw_node);
	swapi_spin_unlock(&swap->ss_lock);

	*win = w;

	w = list_first_entry(&swap->ss_wins, swapi_window_t, sw_node);
	ASSERT(w != NULL);
	swap->ss_wincur = w;

	return 0;
}

swapi_window_t *swapi_swap_top_window(swapi_swap_t *swap){
	ASSERT(swap != NULL);

	return swap->ss_wincur;
}

int swapi_swap_add_handler(swapi_swap_t *swap, int type, swapi_handler_entry_t *she){
	ASSERT((swap != NULL) && (she != NULL));

	return swapi_handler_add(swap->ss_handler, type, she);
}

int swapi_swap_module_init(){
	return 0;
}

int swapi_swap_module_fini(){
	return 0;
}

