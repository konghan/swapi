/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_sys_cache.h"
#include "swapi_sys_atomic.h"

#include "swapi_sys_logger.h"

struct swapi_cache{
    size_t	sc_size;
    size_t	sc_align;
    int		sc_flags;

    atomic_t	sc_allocs;
    atomic_t	sc_frees;
};

static void *align_alloc(size_t size, size_t align){
    void* pStart, *pAligned;
 
    pStart = malloc(size + align - 1 + sizeof(uintptr_t)); 
    if(pStart == NULL)
        return NULL;
 
    uintptr_t addr = (uintptr_t)pStart + align - 1 + sizeof(uintptr_t);
    pAligned = (void*) (addr - (addr % align));
 
    *((uintptr_t*)pAligned -1) = (uintptr_t)pStart;
 
    return pAligned;
}

static void align_free(void *ptr){
    if(!ptr)
		return;

	void* pTrue = (void*)*((uintptr_t*)ptr - 1);
	free(pTrue);
}

int swapi_cache_create(size_t size, size_t align, int flags, swapi_cache_t *mc){
    struct swapi_cache	*m;

    m = swapi_heap_alloc(sizeof(*m));
    if(m == NULL){
	return -ENOMEM;
    }
    memset(m, 0, sizeof(*m));

    m->sc_size	= size;
    m->sc_align	= (sizeof(void*) >= align) ? sizeof(void*) : align;
    m->sc_flags	= flags;

    *mc = m;

    return 0;
}

int swapi_cache_destroy(swapi_cache_t mc){
    struct swapi_cache *m = mc;

    ASSERT(m != NULL);

    if(m->sc_allocs != m->sc_frees){
	return -EINVAL;
    }

    swapi_heap_free(m);

    return 0;
}

void *swapi_cache_alloc(swapi_cache_t mc){
    struct swapi_cache *m = mc;
    void    *ptr;

    ASSERT(m != NULL);

    ptr = align_alloc(m->sc_size, m->sc_align);
    if(ptr != NULL){
	atomic_inc(&m->sc_allocs);
    }

    return ptr;
}

void swapi_cache_free(swapi_cache_t mc, void *ptr){
    struct swapi_cache *m = mc;

    ASSERT(m != NULL);

    atomic_inc(&m->sc_frees);
    align_free(ptr);
}

void *swapi_heap_alloc(size_t size){
    return malloc(size);
}

void swapi_heap_free(void *ptr){
    free(ptr);
}

int swapi_cache_init(void *start, uint64_t size){
    return 0;
}

int swapi_cache_fini(){
    return 0;
}

