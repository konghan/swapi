/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SYS_THREAD_H__
#define __SWAPI_SYS_THREAD_H__

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <Windows.h>

#ifdef __cplusplus
extern "C"{
#endif

#define ASSERT	    assert

// condition variable, used internally
typedef struct __swapi_convar_data{
	HANDLE		cd_handle;
}__swapi_convar_t;

static inline  int __swapi_convar_init(__swapi_convar_t *cv){
	cv->cd_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(cv->cd_handle != NULL){
		return 0;
	}else{
		return -1;
	}
}

static inline int __swapi_convar_fini(__swapi_convar_t *cv){
	CloseHandle(cv->cd_handle);
	cv->cd_handle = NULL;

    return 0;
}

static inline int __swapi_convar_signal(__swapi_convar_t *cv){
	return PulseEvent(cv->cd_handle) ? 0 : -1;
}

static inline int __swapi_convar_wait(__swapi_convar_t *cv){
	return (WaitForSingleObject(cv->cd_handle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
}

static inline int __swapi_convar_timedwait(__swapi_convar_t *cv, uint32_t ms){
	DWORD ret;
	ret = WaitForSingleObject(cv->cd_handle, ms);
	switch(ret){
	case WAIT_TIMEOUT:
		return -1;
//	    return -ETIME;

	case WAIT_OBJECT_0:
		return 0;

	default:
		return -1;
	}
}

// OS independ interface
typedef int (*thread_routine)(void *p);
typedef struct __swapi_THREAD_DATA{
	HANDLE				hThread;
	thread_routine		proc;
	void				*data;
}swapi_thread_t;

static inline DWORD WINAPI win32_ThreadProc(void *param){
	swapi_thread_t	*t = (swapi_thread_t *)param;
	return (DWORD)t->proc(t->data);
}

static inline int swapi_thread_create(swapi_thread_t *thrd,
	int (*thread_routine)(void *), void *data){

		thrd->data = data;
		thrd->proc = thread_routine;

		thrd->hThread = CreateThread(NULL, 0, win32_ThreadProc, thrd, 0, NULL);

		return (thrd->hThread != NULL) ? 0 : -1;
}

static inline int swapi_thread_destroy(swapi_thread_t thrd){
	return TerminateThread(thrd.data, 0) ? 0 : -1;
}


typedef CRITICAL_SECTION	swapi_spinlock_t;
static inline int swapi_spin_init(swapi_spinlock_t *lock){
	return InitializeCriticalSectionAndSpinCount(lock, 4000) ? 0 : -1;
}

static inline int swapi_spin_fini(swapi_spinlock_t *lock){
	DeleteCriticalSection(lock);
	return 0;
}

static inline int swapi_spin_lock(swapi_spinlock_t *lock){
	EnterCriticalSection(lock);
	return 0;
}

static inline int swapi_spin_unlock(swapi_spinlock_t *lock){
	LeaveCriticalSection(lock);
	return 0;
}

static inline int swapi_spin_trylock(swapi_spinlock_t *lock){
	return TryEnterCriticalSection(lock) ? 0 : -1;
}

#if 0
// use by edp_loop internally.
static inline void __swapi_sleep(int second){
    sleep(second);
}
#endif

#ifdef __cplusplus
}
#endif

#endif // __SWAPI_SYS_THREAD_H__


