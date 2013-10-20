/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_LOOP_H__
#define __SWAPI_LOOP_H__

#include "swapi_swap.h"
#include "swapi_message.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_QUEUE_MAX_MSGS		32
#define kSWAPI_HANDLER_MAX_SLOTS	32

int swapi_loop_run(void *p);

int swapi_loop_add_swap(swapi_swap_t *swap);

int swapi_loop_load_swap(const char *swap);

int swapi_loop_post(swapi_message_t *msg);

int swapi_loop_module_init();
int swapi_loop_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_LOOP_H__
