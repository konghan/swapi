/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_HANDLER_H__
#define __SWAPI_HANDLER_H__

#include "swapi_message.h"

#include "swapi_sys_thread.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_HANDLER_DEFAULT_SLOTS		32

struct swapi_handler;
typedef struct swapi_handler swapi_handler_t;

typedef int (*swapi_handler_func)(swapi_message_t *msg, void *data);

typedef struct swapi_handler_entry{
	int						she_type;
	
	struct list_head		she_node;
	
	swapi_handler_func		she_cbfunc;
	void					*she_data;
}swapi_handler_entry_t;

int swapi_handler_create(int slots, swapi_handler_t **sh);
int swapi_handler_destroy(swapi_handler_t *sh);

int swapi_handler_add(swapi_handler_t *sh, int type, swapi_handler_entry_t *she);
int swapi_handler_del(swapi_handler_t *sh, int type, swapi_handler_entry_t *she);

int swapi_handler_invoke(swapi_handler_t *sh, swapi_message_t *msg);

int swapi_handler_module_init();
int swapi_handler_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_HANDLER_H__
