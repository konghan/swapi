/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_swap.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

static int swapi_swap_on_swapmessage(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{.she_type = kSWAPI_MSGTYPE_SWAP, .she_node = { }, 
		.she_cbfunc = swapi_swap_on_swapmessage, .she_data = NULL},
	{.she_type = kSWAPI_MSGTYPE_DEFAULT, .she_node = { }, 
		.she_cbfunc = NULL, .she_data = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static int swapi_swap_on_swapmessage(swapi_message_t *msg, void *data){
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

static int swapi_swap_thread_routine(void *p){
	swapi_swap_t		*ss = (swapi_swap_t *)p;
	swapi_view_t		*pos, *vw;
	swapi_handler_t		*handler;
	swapi_message_t		msg;

	ASSERT(ss != NULL);

	swapi_log_info("swap thread is running...\n");

	while(ss->ss_status){
		if(swapi_queue_wait(ss->ss_queue, &msg) != 0){
			swapi_log_warn("swap wait message fail\n");
			break;
		}

		switch(msg.sm_type) {
			case kSWAPI_MSGTYPE_SWAP:
			case kSWAPI_MSGTYPE_TIMER:
				swapi_handler_invoke(ss->ss_handler, &msg);
				break;

			case kSWAPI_MSGTYPE_KEYBOARD:
				vw = swapi_swap_topview(ss);
				handler = swapi_view_get_handler(vw);

				swapi_handler_invoke(handler, &msg);
				break;

			default:
				break;
		}
	}

	// FIXME: free app's resource
	
	list_for_each_entry_safe(pos, vw, &ss->ss_views, sv_node){
		swapi_view_destroy(vw);
	}
	
	swapi_spin_fini(&ss->ss_lock);
	swapi_handler_destroy(ss->ss_handler);
	swapi_queue_destroy(ss->ss_queue);
	swapi_heap_free(ss);

	return 0;
}


int swapi_swap_create(const char *name, swapi_swap_cbs_t *cbs, swapi_swap_t **swap){
	swapi_swap_t			*ss;
	swapi_view_t			*vw;
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

	INIT_LIST_HEAD(&ss->ss_views);
	INIT_LIST_HEAD(&ss->ss_node);
	strncpy(ss->ss_name, name, kSWAPI_SWAP_NAME_LEN-1);
	ss->ss_cbs = cbs;
	ss->ss_status =  1;

	// create default view
	if(swapi_view_create(kSWAPI_VIEW_FULLSCREEN, &vw) != 0){
		swapi_log_warn("swap create default view fail\n");
		goto exit_view;
	}
	swapi_swap_push_view(ss, vw);

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
	swapi_view_destroy(vw);

exit_view:
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

int swapi_swap_module_init(){
	return 0;
}

int swapi_swap_module_fini(){
	return 0;
}

