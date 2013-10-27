/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SWAP_H__
#define __SWAPI_SWAP_H__

#include "swapi_queue.h"
#include "swapi_handler.h"

#include "swapi_view.h"

#include "swapi_sys_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_SWAP_NAME_LEN		32

#define kSWAPI_SWAP_TYPE_NATIVE			0x00000001
#define kSWAPI_SWAP_TYPE_SCRIPT			0x00000002

struct swapi_swap;
typedef struct swapi_swap swapi_swap_t;

typedef struct swapi_swap_cbs{
	int (*on_create)(swapi_swap_t *swap, int argc, char *argv[]);
	int (*on_destroy)(swapi_swap_t *swap);

	int (*on_start)(swapi_swap_t *swap);
	int (*on_stop)(swapi_swap_t *swap);

	int (*on_pause)(swapi_swap_t *swap);
	int (*on_resume)(swapi_swap_t *swap);

//	int (*on_idle)(struct sw_app_data *swad);
}swapi_swap_cbs_t;

typedef struct swapi_swap{
	swapi_queue_t		*ss_queue;
	swapi_handler_t		*ss_handler;

	struct list_head	ss_views;
	swapi_view_t		*ss_vwcur;

	// link to main loop
	struct list_head	ss_node;

	swapi_spinlock_t	ss_lock;
	char				ss_name[kSWAPI_SWAP_NAME_LEN];
	swapi_swap_cbs_t	*ss_cbs;

	swapi_thread_t		ss_thrd;
	int					ss_status;

	unsigned int		ss_type;
}swapi_swapi_t;

int swapi_swap_create(const char *name, swapi_swap_cbs_t *cbs, swapi_swap_t **swap);
int swapi_swap_destroy(swapi_swap_t *swap);

int swapi_swap_post(swapi_swap_t *swap, swapi_message_t *msg);

int swapi_swap_add_handler(swapi_swap_t *ss, int type, swapi_handler_entry_t *she);

int swapi_swap_push_view(swapi_swap_t *swap, swapi_view_t *view);
int swapi_swap_pop_view(swapi_swap_t *swap, swapi_view_t **view);

swapi_view_t *swapi_swap_topview(swapi_swap_t *swap);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_SWAP_H__
