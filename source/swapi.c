/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */
#include "swapi.h"

#include "swapi_loop.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"
#include "list.h"

static swapi_thread_t	__gs_main_loop_thrd;

int swapi_init(){
	int		ret;

	// add smatchos initialize code here:
	// init logger
	swapi_log_module_init();

	// smatchos is running...
	swapi_log_info("smatchos is running ...\n");

	// smatchos : loop for event
	ret = swapi_thread_create(&__gs_main_loop_thrd, swapi_loop_run, NULL);
	if(ret != 0){
		swapi_log_warn("smatchos create main loop fail\n");
		return -1;
	}

	return 0;
}

int swapi_fini(){
	swapi_log_info("smatchos is stopping ...\n");
	swapi_log_module_fini();

	return 0;
}

