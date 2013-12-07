/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_handler.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

struct swapi_handler{
	swapi_spinlock_t	sh_lock;
	
	int					sh_slots;
	struct list_head	sh_entries[];
};

int swapi_handler_create(int slots, swapi_handler_t **sh){
	swapi_handler_t		*h;
	int					i;
	int					len;

	ASSERT(sh != NULL);

	len = sizeof(*h) + sizeof(struct list_head)*slots;
	h = swapi_heap_alloc(len);
	if(h == NULL){
		swapi_log_warn("no more memory for handler\n");
		return -ENOMEM;
	}
	memset(h, 0, len);

	swapi_spin_init(&h->sh_lock);
	h->sh_slots = slots;

	for(i = 0; i < slots; i++){
		INIT_LIST_HEAD(&(h->sh_entries[i]));
	}

	*sh = h;

	return 0;
}

int swapi_handler_destroy(swapi_handler_t *sh){

	ASSERT(sh != NULL);

	swapi_spin_fini(&sh->sh_lock);

	// FIXME: delete all entries

	swapi_heap_free(sh);

	return 0;
}

int swapi_handler_add(swapi_handler_t *sh, int type, swapi_handler_entry_t *she){
	ASSERT(sh != NULL);
	ASSERT(she != NULL);

	if(type >= sh->sh_slots){
		swapi_log_warn("type is out range:%d\n", type);
		return -ERANGE;
	}
	INIT_LIST_HEAD(&she->she_node);

	swapi_spin_lock(&sh->sh_lock);

	list_add_tail(&she->she_node, &(sh->sh_entries[type]));

	swapi_spin_unlock(&sh->sh_lock);

	return 0;
}

int swapi_handler_del(swapi_handler_t *sh, int type, swapi_handler_entry_t *she){
	ASSERT(sh != NULL);
	ASSERT(she != NULL);

	if(type >= sh->sh_slots){
		swapi_log_warn("msg_type is out range:%d\n", type);
		return -ERANGE;
	}

	swapi_spin_lock(&sh->sh_lock);

	list_del(&she->she_node);
	
	swapi_spin_unlock(&sh->sh_lock);

	return 0;
}

int swapi_handler_invoke(swapi_handler_t *sh, swapi_message_t *msg){
	swapi_handler_entry_t	*pos;
	int						invoked = 0;

	ASSERT(sh != NULL);
	ASSERT(msg != NULL);

	if(msg->sm_type >= sh->sh_slots){
		swapi_log_warn("message : %d out range\n", (unsigned int)msg);
		return -ERANGE;
	}

	list_for_each_entry(pos, &(sh->sh_entries[msg->sm_type]), she_node){
		pos->she_cbfunc(msg, pos->she_data);

		invoked = 1;
	}

	if(!invoked){
		swapi_log_warn("message : %d is without handler\n", msg->sm_type); 
	}

	// FIXME: if no entry ,call default func

	return 0;
}

int swapi_handler_module_init(){
	return 0;
}

int swapi_handler_module_fini(){
	return 0;
}

