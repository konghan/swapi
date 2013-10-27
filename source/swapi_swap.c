/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_swap.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

static int swapi_swap_thread_routine(void *p){
	swapi_swap_t	*ss = (swapi_swap_t *)p;
	swapi_message_t	msg;

	ASSERT(ss != NULL);

	while(ss->ss_status){
		if(swapi_queue_wait(ss->ss_queue, &msg) != 0){
			swapi_log_warn("swap wait message failn");
			break;
		}

		// FIXME: check msg == destroy


		swapi_handler_invoke(ss->ss_handler, &msg);
	}

	// FIXME: free app's resource
	swapi_spin_fini(&ss->ss_lock);
	swapi_handler_destroy(ss->ss_handler);
	swapi_queue_destroy(ss->ss_queue);
	swapi_heap_free(ss);

	return 0;
}


int swapi_swap_create(const char *name, swapi_swap_cbs_t *cbs, swapi_swap_t **swap){
	swapi_swap_t	*ss;

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

	INIT_LIST_HEAD(&ss->ss_node);
	strncpy(ss->ss_name, name, kSWAPI_SWAP_NAME_LEN-1);
	ss->ss_cbs = cbs;
	ss->ss_status =  1;

	// FIXME: post oncreate / onstart message
	
	if(swapi_thread_create(&ss->ss_thrd, swapi_swap_thread_routine, ss) != 0){
		swapi_log_warn("swap create swap thread fail!\n");
		goto exit_thread;
	}

	*swap = ss;

	return 0;

exit_thread:
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

	// FIXME : post destroy message to ss

	return 0;
}

int swapi_swap_post(swapi_swap_t *ss, swapi_message_t *msg){
	ASSERT(ss != NULL);
	ASSERT(msg != NULL);

	return swapi_queue_post(ss->ss_queue, msg);
}

int swapi_swap_push_view(swapi_swap_t *swap, swapi_view_t *view){
	ASSERT(swap != NULL);
	ASSERT(view != NULL);

	list_add(&view->sv_node, &swap->ss_views);

	swap->ss_vwcur = view;

	return 0;
}

int swapi_swap_pop_view(swapi_swap_t *swap, swapi_view_t **view){
	swapi_view_t	*v;

	ASSERT(swap != NULL);
	ASSERT(view != NULL);

	v = list_first_entry(&swap->ss_views, swapi_view_t, sv_node);
	if(swap->ss_vwcur != v){
		return -EINVAL;
	}

	if(list_is_last(&v->sv_node, &swap->ss_views)){
		return -EINVAL;
	}

	list_del_init(&v->sv_node);
	*view = v;

	v = list_first_entry(&swap->ss_views, swapi_view_t, sv_node);
	ASSERT(v != NULL);
	swap->ss_vwcur = v;

	return 0;
}

swapi_view_t *swapi_swap_topview(swapi_swap_t *swap){
	ASSERT(swap != NULL);

	return swap->ss_vwcur;
}

int swapi_swap_add_handler(swapi_swap_t *swap, int type, swapi_handler_entry_t *she){
	ASSERT((swap != NULL) && (she != NULL));

	return swapi_handler_add(swap->ss_handler, type, she);
}

