/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swap_user.h"
#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"

#include "natv_io.h"

#define kSWAP_USER_NAME			"user"
#define kSWAP_USER_DATA			"user.db"

struct user_phy{
	swapi_uuid_t		up_uid;
	char				up_name[kSWAP_USER_NAME_LEN];
	int					up_pic;
}__attribute__((packed));
typedef struct user_phy user_phy_t;

typedef struct user_inst{
	swapi_spinlock_t	ui_lock;

	int					ui_init;
	int					ui_count;

	struct list_head	ui_users;

	natv_io_t			*ui_io;
	natv_file_t			*ui_file;
}user_inst_t;

static user_inst_t		__gs_user = {};
static inline user_inst_t *get_user(){
	return &__gs_user;
}

static inline int user_save(user_inst_t *ui){
	user_phy_t		up;
	swap_user_t		*su;

	natv_io_seek(ui->ui_file, 0, kNATV_IO_SET);

	list_for_each_entry(su, &ui->ui_users, su_node){
		memset(&up, 0, sizeof(up));

		memcpy(&up.up_uid, &su->su_uid, sizeof(uuid_t));
		up.up_pic =  su->su_picid;
		strncpy(up.up_name, su->su_name, kSWAP_USER_NAME_LEN);

		natv_io_dwrite(ui->ui_file, &up, sizeof(up));
	}

	return 0;
}

int swap_user_add(swap_user_t *su){
	user_inst_t		*ui = get_user();

	ASSERT(su != NULL);

	INIT_LIST_HEAD(&su->su_node);

	swapi_spin_lock(&ui->ui_lock);
	list_add_tail(&su->su_node, &ui->ui_users);
	ui->ui_count++;
	swapi_spin_unlock(&ui->ui_lock);

	return user_save(ui);
}

int swap_user_del(swap_user_t *su){
	user_inst_t		*ui = get_user();

	ASSERT(su != NULL);

	swapi_spin_lock(&ui->ui_lock);
	list_del(&su->su_node);
	ui->ui_count--;
	swapi_spin_unlock(&ui->ui_lock);

	return user_save(ui);
}

int swap_user_count(){
	user_inst_t		*ui = get_user();
	int				count;

	swapi_spin_lock(&ui->ui_lock);
	count = ui->ui_count;
	swapi_spin_unlock(&ui->ui_lock);

	return count;
}

swap_user_t *swap_user_first(){
	user_inst_t		*ui = get_user();
	swap_user_t		*su = NULL;

	if(ui->ui_init == 0){
		swapi_log_warn("swap user is not inited!\n");
		return NULL;
	}

	swapi_spin_lock(&ui->ui_lock);

	if(!list_empty(&ui->ui_users)){
		su = list_first_entry(&ui->ui_users, swap_user_t, su_node);
	}

	swapi_spin_unlock(&ui->ui_lock);

	return su;
}

swap_user_t *swap_user_last(){
	user_inst_t		*ui = get_user();
	swap_user_t		*su = NULL;

	if(ui->ui_init == 0){
		swapi_log_warn("swap user is not inited!\n");
		return NULL;
	}

	swapi_spin_lock(&ui->ui_lock);

	if(!list_empty(&ui->ui_users)){
		su = list_last_entry(&ui->ui_users, swap_user_t, su_node);
	}

	swapi_spin_unlock(&ui->ui_lock);

	return su;
}

swap_user_t *swap_user_next(swap_user_t *su){
	user_inst_t		*ui = get_user();
	swap_user_t		*nx = NULL;

	if(ui->ui_init == 0){
		swapi_log_warn("swap user is not inited!\n");
		return NULL;
	}

	if(su == NULL)
		return swap_user_first();

	swapi_spin_lock(&ui->ui_lock);

	if(su->su_node.next == &ui->ui_users){
		nx = list_first_entry(&ui->ui_users, swap_user_t, su_node);
	}else{
		nx = list_first_entry(&su->su_node, swap_user_t, su_node);
	}

	swapi_spin_unlock(&ui->ui_lock);

	return nx;
}

swap_user_t *swap_user_prev(swap_user_t *su){
	user_inst_t		*ui = get_user();
	swap_user_t		*nx = NULL;

	if(ui->ui_init == 0){
		swapi_log_warn("swap user is not inited!\n");
		return NULL;
	}

	if(su == NULL)
		return swap_user_last();

	swapi_spin_lock(&ui->ui_lock);

	if(su->su_node.prev == &ui->ui_users){
		nx = list_last_entry(&ui->ui_users, swap_user_t, su_node);
	}else{
		nx = list_last_entry(&su->su_node, swap_user_t, su_node);
	}

	swapi_spin_unlock(&ui->ui_lock);

	return nx;
}

int swap_user_init(){
	user_inst_t		*ui  = get_user();
	user_phy_t		up;
	swap_user_t		*su;
	size_t			sz;
	int				fail = 0;

	if(ui->ui_init){
		return 0;
	}

	swapi_spin_init(&ui->ui_lock);
	INIT_LIST_HEAD(&ui->ui_users);

	if(natv_io_open(kSWAP_USER_NAME, &ui->ui_io) != 0){
		swapi_log_warn("open user swap dir fail!\n");

		swapi_spin_fini(&ui->ui_lock);
		return -1;
	}

	if(natv_io_dopen(ui->ui_io, kSWAP_USER_DATA, &ui->ui_file) != 0){
		swapi_log_warn("open user data file fail\n");

		natv_io_close(ui->ui_io);
		swapi_spin_fini(&ui->ui_lock);
		return -1;
	}

	while(1){
		sz = natv_io_dread(ui->ui_file, &up, sizeof(up));
		if(sz != sizeof(up)){
			swapi_log_warn("read user file fail!\n");
			break;
		}

		su = swapi_heap_alloc(sizeof(*su));
		if(su == NULL){
			fail = 1;
			break;
		}
		memset(su, 0, sizeof(*su));

		INIT_LIST_HEAD(&su->su_node);
		strcpy(su->su_name, up.up_name);
		memcpy(&su->su_uid, &up.up_uid, sizeof(uuid_t));

		swapi_spin_lock(&ui->ui_lock);
		list_add_tail(&su->su_node, &ui->ui_users);
		ui->ui_count ++;
		swapi_spin_unlock(&ui->ui_lock);
	}

	ui->ui_init = 1;
	
	if(fail){
		swap_user_fini();
		return -1;
	}

	swapi_log_info("init swap user ok!\n");

	return 0;
}

int swap_user_fini(){
	swap_user_t		*pos, *temp;
	user_inst_t		*ui  = get_user();
	
	ui->ui_init = 0;
	swapi_spin_fini(&ui->ui_lock);
	
	natv_io_dclose(ui->ui_file);
	natv_io_close(ui->ui_io);

	list_for_each_entry_safe(pos, temp, &ui->ui_users, su_node){
		swapi_heap_free(pos);
	}

	return 0;
}

