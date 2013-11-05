/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAP_USER_H__
#define __SWAP_USER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAP_USER_NAME_LEN		16

enum {
	kSWAP_USER_PICID_DEFAULT = 0,
};

typedef struct uuid { char uuid[16]; }uuid_t;

typedef struct swap_user{
	uuid_t		su_uid;
	char		su_name[kSWAP_USER_NAME_LEN];
	int			su_pic;
}swap_user_t;

int swap_user_add(swap_user_t *su);
int swap_user_del(swap_user_t *su);

int swap_user_count();

int swap_user_iter_init(int *iter);
int swap_user_iter_fini(int iter);
swap_user_t *swap_user_iter_next(int iter);
swap_user_t *swap_user_iter_prev(int iter);


int swap_user_init();
int swap_user_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAP_USER_H__

