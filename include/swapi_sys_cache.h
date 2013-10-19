/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SYS_CACHE_H__
#define __SWAPI_SYS_CACHE_H__

#include "swapi_sys_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWAPI_CACHE_FLAGS_NOWAIT	0X00000000
#define SWAPI_CACHE_FLAGS_WAIT	0x00000001

#define SWAPI_CACHE_FLAGS_CORE	0X00010000

//struct mem_pool;
struct swapi_cache;

//typedef struct mem_pool* mpool_t;
typedef struct swapi_cache* swapi_cache_t;

//int mpool_create(void *start, uint64_t size, mpool_t *mp);
//int mpool_destroy(mpool_t mp);

int swapi_cache_create(size_t size, size_t align, int flags, swapi_cache_t *mc);
int swapi_cache_destroy(swapi_cache_t mc);

void *swapi_cache_alloc(swapi_cache_t mc);
void swapi_cache_free(swapi_cache_t mc, void *ptr);

void *swapi_heap_alloc(size_t size);
void swapi_heap_free(void *ptr);

int swapi_cache_module_init(void *start, uint64_t size);
int swapi_cache_module_fini();

#ifdef __cplusplus
}
#endif

#endif // __SWAPI_SYS_CACHE_H__


