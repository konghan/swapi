/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_swap.h"
#include "natv_surface.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"
#include "swapi_sys_thread.h"

static int swap_default_on_swap(swapi_message_t *msg, swapi_swap_t *swap){

	ASSERT((msg != NULL) && (swap != NULL));

	switch(msg->sm_size){
		case kSWAP_MSGTYPE_CREATE:
			swap->ss_cbs->on_create(swap, 0, NULL);
			break;
			
		case kSWAP_MSGTYPE_DESTROY:
			swap->ss_status = 0;
//			swap->ss_cbs->on_destroy(swap);
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

static int swap_default_on_key(swapi_message_t *msg, swapi_swap_t *swap){
	
	ASSERT((msg != NULL) && (swap != NULL));

	swapi_window_invoke(&swap->ss_win, msg);

	return 0;
}

static int swap_default_on_touch(swapi_message_t *msg, swapi_swap_t	*swap){
	
	ASSERT((msg != NULL) && (swap != NULL));
	
	swapi_window_invoke(&swap->ss_win, msg);
	return 0;
}

static int swap_default_on_draw(swapi_message_t *msg, swapi_swap_t *swap){
	
	ASSERT((msg != NULL) && (swap != NULL));

	swapi_log_info("swap %s is on drawing...\n", swap->ss_name);
	
	swapi_window_invoke(&swap->ss_win, msg);
	return 0;
}

static int swapi_swap_thread_routine(void *p){
	swapi_swap_t		*ss = (swapi_swap_t *)p;
	swapi_message_t		msg;

	ASSERT(ss != NULL);

	swapi_log_info("swap %s waiting for kick\n", ss->ss_name);
	__swapi_convar_wait(&ss->ss_cv);
	swapi_log_info("swap %s is kick starting...\n", ss->ss_name);

	while(ss->ss_status){
		if(swapi_queue_wait(ss->ss_queue, &msg) != 0){
			swapi_log_warn("swap wait message fail\n");
			break;
		}

		swapi_log_info("swap %s handle message %d\n", ss->ss_name, msg.sm_type);
		
		switch(msg.sm_type){
		case kSWAPI_MSGTYPE_SWAP:
			swap_default_on_swap(&msg, ss);
			break;

		case kSWAPI_MSGTYPE_KEYBOARD:
			swap_default_on_key(&msg, ss);
			break;

		case kSWAPI_MSGTYPE_TOUCH:
			swap_default_on_touch(&msg, ss);
			break;

		case kSWAPI_MSGTYPE_DRAW:
			swap_default_on_draw(&msg, ss);
			break;

		default:
			swapi_handler_invoke(ss->ss_handler, &msg);
		}
	}

	ss->ss_cbs->on_destroy(ss);

	__swapi_convar_fini(&ss->ss_cv);
	swapi_spin_fini(&ss->ss_lock);
	swapi_handler_destroy(ss->ss_handler);
	swapi_queue_destroy(ss->ss_queue);
	swapi_heap_free(ss);

	return 0;
}

int swapi_swap_kick(swapi_swap_t *swap){
	ASSERT(swap != NULL);

	__swapi_convar_signal(&swap->ss_cv);

	return 0;
}

int swapi_swap_create(const char *name, swapi_swap_cbs_t *cbs, swapi_swap_t **swap){
	swapi_swap_t			*ss;
	natv_surface_info_t		nsi;

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

	if(__swapi_convar_init(&ss->ss_cv) != 0){
		swapi_log_warn("swap init cv fail!\n");
		goto exit_cv;
	}

	INIT_LIST_HEAD(&ss->ss_node);
	strncpy(ss->ss_name, name, kSWAPI_SWAP_NAME_LEN-1);
	ss->ss_cbs = cbs;
	ss->ss_status =  1;

	// create default window
	natv_surface_getinfo(&nsi);

	if(swapi_window_init(&ss->ss_win, nsi.nsi_width, nsi.nsi_height, nsi.nsi_type) != 0){
		swapi_log_warn("swap init window fail\n");
		goto exit_window;
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
	swapi_window_fini(&ss->ss_win);

exit_window:
	__swapi_convar_fini(&ss->ss_cv);

exit_cv:
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
	ASSERT((ss != NULL) && (msg != NULL));

	return swapi_queue_post(ss->ss_queue, msg);
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

