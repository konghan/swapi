/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */
#include "swapi.h"

#include "swapi_loop.h"
#include "swapi_queue.h"
#include "swapi_handler.h"
#include "swapi_view.h"
#include "swapi_swap.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

#include "list.h"

extern int swapi_surface_module_init();

static swapi_thread_t	__gs_main_loop_thrd;

void swapi_debug(){
	while (1){
		Sleep(1000);
	}
}

static int swapi_module_init(void *p){

	swapi_surface_module_init();

	swapi_queue_module_init();

	swapi_handler_module_init();

	swapi_swap_module_init();

	swapi_loop_module_init();
	
	return swapi_loop_run(NULL);
}

int swapi_init(){
	int		ret;

	// FIXME: need pre-alloc memory for cache
	swapi_cache_module_init(NULL, 0);
	
	// add smatchos initialize code here:
	// init logger
	swapi_log_module_init();

	// smatchos is running...
	swapi_log_info("smatchos is running ...\n");

	// smatchos : loop for event
	ret = swapi_thread_create(&__gs_main_loop_thrd, swapi_module_init, NULL);
	if(ret != 0){
		swapi_log_warn("smatchos create main loop fail\n");
		return -1;
	}

	return 0;
}

int swapi_fini(){
	swapi_log_info("smatchos is stopping ...\n");
	swapi_log_module_fini();

	swapi_thread_destroy(__gs_main_loop_thrd);

	return 0;
}

