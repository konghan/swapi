/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAP_USER_H__
#define __SWAP_USER_H__

#include "swapi_types.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAP_USER_NAME_LEN		16

enum {
	kSWAP_USER_PICID_DEFAULT = 0,
};

typedef struct swap_user{
	struct list_head	su_node;

	swapi_uuid_t		su_uid;
	char				su_name[kSWAP_USER_NAME_LEN];
	int					su_picid;
}swap_user_t;

int swap_user_add(swap_user_t *su);
int swap_user_del(swap_user_t *su);

int swap_user_count();

swap_user_t *swap_user_first();
swap_user_t *swap_user_last();

swap_user_t *swap_user_next(swap_user_t *su);
swap_user_t *swap_user_prev(swap_user_t *su);

int swap_user_init();
int swap_user_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAP_USER_H__

