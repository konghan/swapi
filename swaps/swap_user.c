/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swap_user.h"

#include "natv_io.h"

#define kSWAP_USER_NAME			"user"
#define kSWAP_USER_DATA			"user.db"

struct user_phy{
	uuid_t				up_uid;
	char				up_name[kSWAP_USER_NAME_LEN];
	int					up_pic;
}__attribute__((packed));
typedef struct user_phy user_phy_t;

typedef struct user_main{
	swapi_spinlock_t	um_lock;

	int					um_init;
	int					um_count;
	struct list_head	um_users;

	natv_io_t			*um_io;
	natv_file_t			*um_file;
}user_main_t;

static user_main_t		__gs_user = {};
static inline user_main_t *get_user(){
	return &__gs_user;
}

static inline int user_save(user_main_t *um){
	user_phy_t		up;
	swap_user_t		*su;

	natv_io_seek(um->um_file, 0, kNATV_IO_SET);

	list_for_each_entry(su, &um->um_users, su_node){
		memset(&up, 0, sizeof(up));

		memcpy(su->su_uid, up.up_uid, sizeof(uuid_t));
		up.up_pic =  su->su_pic;
		strcpy(su->su_name, up.up_name);

		natv_io_dwrite(um->um_file, &up, sizeof(up));
	}

	return 0;
}

int swap_user_add(swap_user_t *su){
	user_main_t		*um = get_user();

	ASSERT(su != NULL);

	INIT_LIST_HEAD(&su->su_node);

	swapi_spin_lock(&um->um_lock);
	list_add_tail(&su->su_node, &um->um_users);
	um->um_count++;
	swapi_spin_unlock(&um->um_lock);

	return user_save(um);
}

int swap_user_del(swap_user_t *su){
	user_main_t		*um = get_user();

	ASSERT(su != NULL);

	swapi_spin_lock(&um->um_lock);
	list_del(&su->su_node);
	um->um_count--;
	swapi_spin_unlock(&um->um_lock);

	return user_save(um);
}

int swap_user_count(){
	user_main_t		*um = get_user();
	int				count;

	swapi_spin_lock(&um->um_lock);
	count = um->um_count;
	swapi_spin_unlock(&um->um_lock);

	return count;
}

swap_user_t *swap_user_first(){
	user_main_t		*um = get_user();
	swap_user_t		*su = NULL;

	swapi_spin_lock(&um->um_lock);

	if(!list_empty(&um->um_users)){
		su = list_first_entry(&um->um_users, swap_user_t, su_node);
	}

	swapi_spin_unlock(&um->um_lock);

	return su;
}

swap_user_t *swap_user_last(){
	user_main_t		*um = get_user();
	swap_user_t		*su = NULL;

	swapi_spin_lock(&um->um_lock);

	if(!list_empty(&um->um_users)){
		su = list_last_entry(&um->um_users, swap_user_t, su_node);
	}

	swapi_spin_unlock(&um->um_lock);

	return su;
}

swap_user_t *swap_user_next(swap_user_t *su){
	user_main_t		*um = get_user();
	swap_user_t		*nx = NULL;

	ASSERT(su != NULL);

	swapi_spin_lock(&um->um_lock);

	if(su->su_node.next == &um->um_users){
		nx = list_first_entry(&um->um_users, swap_user_t, su_node);
	}else{
		nx = list_first_entry(&su->su_node, swap_user_t, su_node);
	}

	swapi_spin_unlock(&um->um_lock);

	return nx;
}

swap_user_t *swap_user_prev(swap_user_t *su){
	user_main_t		*um = get_user();
	swap_user_t		*nx = NULL;

	swapi_spin_lock(&um->um_lock);

	if(su->su_node.prev == &um->um_users){
		nx = list_last_entry(&um->um_users, swap_user_t, su_node);
	}else{
		nx = list_last_entry(&su->su_node, swap_user_t, su_node);
	}

	swapi_spin_unlock(&um->um_lock);

	return nx;
}

int swap_user_init(){
	user_main_t		*um  = get_user();
	user_phy_t		up;
	swap_user_t		*su;
	size_t			sz;
	int				fail = 0;

	swapi_spin_init(&um->um_lock);
	INIT_LIST_HEAD(&um->um_users);

	if(natv_io_open(kSWAP_USER_NAME, &um->um_io) != 0){
		swapi_log_warn("open user swap dir fail!\n");

		swapi_spin_fini(&um->um_lock);
		return -1;
	}

	if(natv_io_dopen(um->um_io, kSWAP_USER_DATA, &um->um_file) != 0){
		swapi_log_warn("open user data file fail\n");

		natv_io_close(um->um_io);
		swapi_spin_fini(&um->um_lock);
		return -1;
	}

	while(1){
		sz = natv_io_dread(um->um_file, &up, sizeof(up));
		if(sz != sizeof(up)){
			break;
		}

		su = heap_alloc(sizeof(*su));
		if(su == NULL){
			fail = 1;
			break;
		}
		memset(su, 0, sizeof(*su));

		INIT_LIST_HEAD(&su->su_node);
		strcpy(su->su_name, up.up_name);
		memcpy(su->su_uid, up.up_uid, sizeof(uuid_t));

		swapi_spin_lock(&um->um_lock);
		list_add_tail(&su->su_node, &um->um_users);
		um->um_count ++;
		swapi_spin_unlock(&um->um_lock);
	}

	um->um_init = 1;
	
	if(fail){
		swap_user_fini();
		return -1;
	}

	return 0;
}

int swap_user_fini(){
	swap_user_t		*pos, *temp;
	
	um->um_init = 0;
	swapi_spin_fini(&um->um_lock);
	
	natv_io_dclose(um->um_file);
	natv_io_close(um->um_io);

	list_for_each_entry_safe(pos, temp, &um->um_users, su_node){
		heap_free(pos);
	}

	return 0;
}

