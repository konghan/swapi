/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_QUEUE_H__
#define __SWAPI_QUEUE_H__

#include "swapi_sys_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_QUEUE_DEFAULT_LENGTH		32

struct swapi_queue;
typedef struct swapi_queue swapi_queue_t;

int swapi_queue_create(int isize, int numb, swapi_queue_t **sq);
int swapi_queue_destroy(swapi_queue_t *sq);

int swapi_queue_post(swapi_queue_t *sq, void *msg);
int swapi_queue_wait(swapi_queue_t *sq, void *msg);

int swapi_queue_getinfo(swapi_queue_t *sq, int *isize, int *numb, int *msgs);

int swapi_queue_module_init();
int swapi_queue_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_QUEUE_H__
