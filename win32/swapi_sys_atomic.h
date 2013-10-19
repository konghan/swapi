/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_ATOMIC_H__
#define __SWAPI_ATOMIC_H__

#include "swapi_sys_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t	    atom64_t;
typedef LONG	    atomic_t;


static __inline atomic_t atomic_inc(atomic_t *atm){
    return InterlockedIncrement(atm);
}

static __inline atomic_t atomic_dec(atomic_t *atm){
    return InterlockedDecrement(atm);
}

#if 0
static __inline atomic_t atomic_add(atomic_t *atm, atomic_t val){
    return InterlockedAdd64(atm, val);
}

static __inline atomic_t atomic_sub(atomic_t *atm, atomic_t val){
    return __sync_sub_and_fetch(atm, val);
}
static __inline atomic_t atomic_reset(atomic_t *atm){
    return __sync_fetch_and_and(atm, 0);
}
#endif

#ifdef __cplusplus
}
#endif

#endif // __SWAPI_ATOMIC_H__


