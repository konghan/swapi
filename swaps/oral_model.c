/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "oral_model.h"

#include "swapi_sys_thread.h"

#include "natv_io.h"

#ifndef ORAL_TEST
#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"
#endif

#include "list.h"
#include "sqlite3.h"

#define kORAL_DB_FILE		"../rootfs/sys/db.sqlite"

#ifdef ORAL_TEST

#define swapi_log_warn		printf
#define swapi_heap_alloc	malloc
#define swapi_heap_free		free

#endif

typedef struct oral_model{
	sqlite3				*om_db;
	
	int					om_init;
	swapi_mutex_t		om_lock;

	struct list_head	om_users;
	unsigned int		om_count;
	oral_user_t			*om_cur;
}oral_model_t;

static oral_model_t __gs_om = {};
static inline oral_model_t *get_model(){
	return &__gs_om;
}

#define kORAL_USER_TABLE_CREATE		"CREATE TABLE IF NOT EXISTS oral_user(uid BLOB primary key, uname varchar(16), upicn varchar(32))"

#define kORAL_USER_TABLE_INSERT		"INSERT INTO oral_user(uid,uname,upicn)VALUES(?1,?2,?3)"
#define kORAL_USER_TABLE_DELETE		"DELETE FROM oral_user where uid=?1"
#define kORAL_USER_TABLE_SELECT		"SELECT * FROM oral_user"

#define kORAL_VMSG_TABLE_CREATE 	"CREATE TABLE IF NOT EXISTS oral_vmsg(_vmsgid INTEGER primary key autoincrement,"\
	"uid BLOB,vtype INTEGER, vseq INTEGER, vtext TEXT, voice TEXT)" 
#define kORAL_VMSG_TABLE_INSERT		"INSERT INTO oral_vmsg(uid,vtype,vseq,vtext,voice)VALUES(?1,?2,?3,?4,?5)"
#define kORAL_VMSG_TABLE_DELETE		"DELETE FROM oral_vmsg WHERE uid = ?1 and vseq=?2"
#define kORAL_VMSG_TABLE_SELECT		"SELECT * FROM oral_vmsg WHERE uid = ?1 ORDER by vseq DESC"

enum{
	kUSER_TABLE_COLUMN_UID	= 0,
	kUSER_TABLE_COLUMN_NAME,
	kUSER_TABLE_COLUMN_PICN
};

enum{
	kVMSG_TABLE_COLUMN_VMSGID = 0,
	kVMSG_TABLE_COLUMN_UID,
	kVMSG_TABLE_COLUMN_TYPE,
	kVMSG_TABLE_COLUMN_SEQ,
	kVMSG_TABLE_COLUMN_TEXT,
	kVMSG_TABLE_COLUMN_VOICE
};

int oral_user_add(oral_user_t *ou){
	oral_model_t	*om = get_model();
	oral_user_t		*pos;
	sqlite3_stmt	*stmt;
	int				ret;

	ASSERT(ou != NULL);

	INIT_LIST_HEAD(&ou->ou_node);

	list_for_each_entry(pos, &om->om_users, ou_node){
		if(swapi_uuid_cmp(&ou->ou_uid, &pos->ou_uid) == 0){
			swapi_log_warn("user info is exist!\n");
			return -EEXIST;
		}
	}

	ret = sqlite3_prepare(om->om_db, kORAL_USER_TABLE_INSERT, -1, &stmt, NULL);
	if(SQLITE_OK != ret){
		swapi_log_warn("prepare user insert fail!\n");
		return -1;
	}

	ret = sqlite3_bind_blob(stmt, 1, &ou->ou_uid, sizeof(swapi_uuid_t), SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind uid fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_bind_text(stmt, 2, ou->ou_name, ou->ou_name_len, SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind name fail!\n");
		goto exit_bind;
	}
	
	ret = sqlite3_bind_text(stmt, 3, ou->ou_picn, ou->ou_picn_len, SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind picn fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_step(stmt);
	if(SQLITE_DONE == ret){
		list_add_tail(&ou->ou_node, &om->om_users);
		om->om_count++;
		if(om->om_cur == NULL){
			om->om_cur = ou;
		}
	}

exit_bind:
	sqlite3_finalize(stmt);
	return (ret == SQLITE_DONE)? 0: -1;
}

int oral_user_del(swapi_uuid_t *uid){
	oral_model_t	*om = get_model();
	oral_user_t		*pos;
	sqlite3_stmt	*stmt;
	int				ret = -1;

	list_for_each_entry(pos, &om->om_users, ou_node){
		if(swapi_uuid_cmp(uid, &pos->ou_uid) == 0){
			list_del(&pos->ou_node);
//			swapi_heap_free(pos);
			om->om_count --;
			break;
		}
	}

	if(list_empty(&om->om_users)){
		om->om_cur = NULL;
	}else{
		om->om_cur = list_first_entry(&om->om_users, oral_user_t, ou_node);
	}

	ret = sqlite3_prepare(om->om_db, kORAL_USER_TABLE_DELETE, -1, &stmt, NULL);
	if(SQLITE_OK != ret){
		swapi_log_warn("prepare user delete fail!\n");
		goto exit_prepare;
	}

	ret = sqlite3_bind_blob(stmt, 1, uid, sizeof(swapi_uuid_t), SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind uid fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_step(stmt);

	natv_io_rm(pos->ou_picn);

exit_bind:
	sqlite3_finalize(stmt);

exit_prepare:
	swapi_heap_free(pos);

	return (ret == SQLITE_DONE)? 0: -1;
}

oral_user_t *oral_user_current(){
	oral_model_t	*om = get_model();

	return om->om_cur;
}

oral_user_t *oral_user_next(oral_user_t *ou){
	oral_model_t	*om = get_model();

	ASSERT(ou != NULL);

	if(list_empty(&om->om_users)){
		om->om_cur = NULL;
		return om->om_cur;
	}

	if(ou->ou_node.next == &om->om_users){
		om->om_cur = list_first_entry(&om->om_users, oral_user_t, ou_node);
	}else{
		om->om_cur = list_entry(ou->ou_node.next, oral_user_t, ou_node);
	}

	return om->om_cur;
}

oral_user_t *oral_user_prev(oral_user_t *ou){
	oral_model_t	*om = get_model();

	ASSERT(ou != NULL);

	if(list_empty(&om->om_users)){
		om->om_cur = NULL;
		return om->om_cur;
	}

	if(ou->ou_node.prev == &om->om_users){
		om->om_cur = list_last_entry(&om->om_users, oral_user_t, ou_node);
	}else{
		om->om_cur = list_entry(ou->ou_node.prev, oral_user_t, ou_node);
	}

	return om->om_cur;
}

/*
 * implement of vmsg
 */
static int oral_vmsg_del(oral_vmsg_t *ov){
	oral_model_t	*om = get_model();
	sqlite3_stmt	*stmt;
	int				ret;

	ret = sqlite3_prepare(om->om_db, kORAL_VMSG_TABLE_DELETE, -1, &stmt, NULL);
	if(SQLITE_OK != ret){
		swapi_log_warn("prepare vmsg delete fail!\n");
		return -1;
	}

	ret = sqlite3_bind_blob(stmt, 1, &ov->ov_uid, sizeof(swapi_uuid_t), SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind uid fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_bind_int(stmt, 2, ov->ov_seq);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind seq fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_step(stmt);

	if(ov->ov_type & kORAL_VMSG_TYPE_VOICE){
		natv_io_rm(ov->ov_voice);
	}

exit_bind:
	sqlite3_finalize(stmt);
	return (ret == SQLITE_DONE)? 0: -1;
}

int oral_vmsg_add(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov){
	oral_model_t		*om = get_model();
	sqlite3_stmt		*stmt;
	oral_vmsg_t			*ovlast;
	int					ret;

	ASSERT((ovc != NULL) && (ov != NULL));

	INIT_LIST_HEAD(&ov->ov_node);

	ret = sqlite3_prepare(om->om_db, kORAL_VMSG_TABLE_INSERT, -1, &stmt, NULL);
	if(SQLITE_OK != ret){
		swapi_log_warn("prepare vmsg insert fail!\n");
		return -1;
	}

	ret = sqlite3_bind_blob(stmt, 1, &ov->ov_uid, sizeof(swapi_uuid_t), SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind uid fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_bind_int(stmt, 2, ov->ov_type);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind type fail!\n");
		goto exit_bind;
	}
	
	ret = sqlite3_bind_int(stmt, 3, ov->ov_seq);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind seq fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_bind_text(stmt, 4, ov->ov_text, ov->ov_text_len, SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind text fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_bind_text(stmt, 5, ov->ov_voice, ov->ov_voice_len, SQLITE_STATIC);
	if(SQLITE_OK != ret){
		swapi_log_warn("sqlite bind voice fail!\n");
		goto exit_bind;
	}

	ret = sqlite3_step(stmt);
	if(SQLITE_DONE == ret){
		list_add(&ov->ov_node, &ovc->ovc_vmsgs);
		ovc->ovc_count ++;
		ovc->ovc_cur = ov;

		if(ovc->ovc_count > kORAL_VMSG_ITEM_MAX){
			ovlast = list_last_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
			oral_vmsg_del(ovlast);
		}
	}else{
		swapi_log_warn("add vmsg item fail:%d\n", ret);
	}

exit_bind:
	sqlite3_finalize(stmt);
	return (ret == SQLITE_DONE)? 0: -1;
}

int oral_vmsg_pack(oral_vmsg_ctrl_t *ovc){
	oral_vmsg_t		*pos, *temp;
	int				count = 0;

	ASSERT(ovc != NULL);

	list_for_each_entry_safe(pos, temp, &ovc->ovc_vmsgs, ov_node){
		if(++count > kORAL_VMSG_ITEM_MAX){
			oral_vmsg_del(pos);
			list_del(&pos->ov_node);
			ovc->ovc_count--;
		}
	}

	if(list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = NULL;
	}else{
		ovc->ovc_cur = list_first_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
	}

	return 0;
}

int oral_vmsg_clear(oral_vmsg_ctrl_t *ovc){
	oral_vmsg_t		*pos, *temp;

	ASSERT(ovc != NULL);

	list_for_each_entry_safe(pos, temp, &ovc->ovc_vmsgs, ov_node){
		oral_vmsg_del(pos);
		list_del(&pos->ov_node);
		ovc->ovc_count--;
	}

	ovc->ovc_cur = NULL;
	ovc->ovc_load = 0;

	return 0;
}

oral_vmsg_t *oral_vmsg_current(oral_vmsg_ctrl_t *ovc){
	ASSERT(ovc != NULL);

	return ovc->ovc_cur;
}

oral_vmsg_t *oral_vmsg_first(oral_vmsg_ctrl_t *ovc){
	if(list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = NULL;
		return NULL;
	}

	return list_first_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
}

oral_vmsg_t *oral_vmsg_last(oral_vmsg_ctrl_t *ovc){
	if(list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = NULL;
		return NULL;
	}

	return list_last_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
}

oral_vmsg_t *oral_vmsg_next(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov){
	ASSERT((ovc != NULL) && (ov != NULL));

	if(list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = NULL;
		return NULL;
	}

	if(ov->ov_node.next == &ovc->ovc_vmsgs){
		ovc->ovc_cur = list_first_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
	}else{
		ovc->ovc_cur = list_entry(ov->ov_node.next, oral_vmsg_t, ov_node);
	}

	return ovc->ovc_cur;
}

oral_vmsg_t *oral_vmsg_prev(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov){
	ASSERT((ovc != NULL) && (ov != NULL));

	if(list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = NULL;
		return NULL;
	}

	if(ov->ov_node.prev == &ovc->ovc_vmsgs){
		ovc->ovc_cur = list_last_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
	}else{
		ovc->ovc_cur = list_entry(ov->ov_node.prev, oral_vmsg_t, ov_node);
	}

	return ovc->ovc_cur;
}

int oral_vmsg_load(oral_vmsg_ctrl_t *ovc){
	oral_model_t	*om = get_model();
	sqlite3_stmt	*stmt;
	oral_vmsg_t		*ov;
	int				tlen, vlen;
	int				ret;

	ret = sqlite3_prepare(om->om_db, kORAL_VMSG_TABLE_SELECT, -1, &stmt, NULL);
	if(ret != SQLITE_OK){
		swapi_log_warn("prepare sql for load vmsg fail:%d\n", ret);
		return -1;
	}

	ret = sqlite3_bind_blob(stmt, 1, &ovc->ovc_uid, sizeof(swapi_uuid_t), SQLITE_STATIC);
	if(ret != SQLITE_OK){
		swapi_log_warn("bind uuid to vmsg loading fail!\n");
		sqlite3_finalize(stmt);
		return -1;
	}

	while(1){
		ret = sqlite3_step(stmt);
		switch(ret){
		case SQLITE_ROW:
			tlen = sqlite3_column_bytes(stmt, kVMSG_TABLE_COLUMN_TEXT);
			vlen = sqlite3_column_bytes(stmt, kVMSG_TABLE_COLUMN_VOICE);

			ov = swapi_heap_alloc(sizeof(*ov) + tlen + vlen + 2);
			if(ov == NULL){
				swapi_log_warn("no enough memory for vmsg!\n");
				break;
			}
			memset(ov, 0, sizeof(*ov) + tlen + vlen + 2);

			memcpy(&ov->ov_uid, sqlite3_column_text(stmt,kVMSG_TABLE_COLUMN_UID),
					sizeof(swapi_uuid_t));

			ov->ov_text = (char *)(ov + 1);
			ov->ov_voice = ov->ov_text + tlen + 1;
			ov->ov_text_len = tlen;
			ov->ov_voice_len = vlen;
			memcpy(ov->ov_text, sqlite3_column_text(stmt, kVMSG_TABLE_COLUMN_TEXT), tlen);
			memcpy(ov->ov_voice, sqlite3_column_text(stmt, kVMSG_TABLE_COLUMN_VOICE), vlen);

			ov->ov_type = sqlite3_column_int(stmt, kVMSG_TABLE_COLUMN_TYPE);
			ov->ov_seq  = sqlite3_column_int(stmt, kVMSG_TABLE_COLUMN_SEQ);
			
			INIT_LIST_HEAD(&ov->ov_node);
			list_add_tail(&ov->ov_node, &ovc->ovc_vmsgs);
			ovc->ovc_count ++;

			swapi_log_warn("get vmsg : %d - %s\n", ov->ov_seq, ov->ov_text);

			continue;

		case SQLITE_DONE:
			swapi_log_warn("get vmsg is done\n");
			break;

		default:
			swapi_log_warn("sqlite step fail:%d\n", ret);
			break;
		}
		break;
	}

	if(!list_empty(&ovc->ovc_vmsgs)){
		ovc->ovc_cur = list_first_entry(&ovc->ovc_vmsgs, oral_vmsg_t, ov_node);
	}

	sqlite3_finalize(stmt);

	if(SQLITE_DONE == ret){
		ovc->ovc_load = 1;
		return 0;
	}else{
		ovc->ovc_load = 0;
		return -1;
	}
}

static int oral_load_user(oral_model_t *om){
	sqlite3_stmt	*stmt;
	oral_user_t		*ou;
	int				nlen, plen;
	int				ret;

	ret = sqlite3_prepare(om->om_db, kORAL_USER_TABLE_SELECT, -1, &stmt, NULL);
	if(ret != SQLITE_OK){
		return -1;
	}

	while(1){
		ret = sqlite3_step(stmt);
		switch(ret){
		case SQLITE_ROW:
			nlen = sqlite3_column_bytes(stmt, kUSER_TABLE_COLUMN_NAME);
			plen = sqlite3_column_bytes(stmt, kUSER_TABLE_COLUMN_PICN);

			ou = swapi_heap_alloc(sizeof(*ou) + nlen + plen + 2);
			if(ou == NULL){
				swapi_log_warn("no enough memory for user!\n");
				break;
			}
			memset(ou, 0, sizeof(*ou) + nlen + plen + 2);

			memcpy(&ou->ou_uid, sqlite3_column_text(stmt,kUSER_TABLE_COLUMN_UID), sizeof(swapi_uuid_t));
			ou->ou_name = (char *)(ou + 1);
			ou->ou_picn = ou->ou_name + nlen + 1;
			ou->ou_name_len = nlen;
			ou->ou_picn_len = plen;
			memcpy(ou->ou_name, sqlite3_column_text(stmt, kUSER_TABLE_COLUMN_NAME), nlen);
			memcpy(ou->ou_picn, sqlite3_column_text(stmt, kUSER_TABLE_COLUMN_PICN), plen);
			
			INIT_LIST_HEAD(&ou->ou_node);
			list_add_tail(&ou->ou_node, &om->om_users);
			om->om_count ++;

			swapi_log_warn("get user : %s - %s\n", ou->ou_name, ou->ou_picn);

			continue;

		case SQLITE_DONE:
			swapi_log_warn("get user is done\n");
			break;

		default:
			swapi_log_warn("sqlite step fail:%d\n", ret);
			break;
		}
		break;
	}

	if(!list_empty(&om->om_users)){
		om->om_cur = list_first_entry(&om->om_users, oral_user_t, ou_node);
	}

	sqlite3_finalize(stmt);

	return (SQLITE_DONE == ret) ? 0 : -1;
}

static int oral_clear_user(oral_model_t *om){
	oral_user_t	*pos, *temp;

	list_for_each_entry_safe(pos, temp, &om->om_users, ou_node){
		list_del(&pos->ou_node);
		swapi_heap_free(pos);
	}

	return 0;
}

static int oral_init_table(oral_model_t *om){
	sqlite3_stmt	*stmt;
	int				ret;

	// init user table
	ret = sqlite3_prepare(om->om_db, kORAL_USER_TABLE_CREATE, -1, &stmt, NULL);
	if(ret != SQLITE_OK){
		swapi_log_warn("prepare for create table fail:%d\n", ret);

		return -1;
	}

	ret = sqlite3_step(stmt);
	if(ret != SQLITE_DONE){
		swapi_log_warn("create table fail!\n");
	}

	sqlite3_finalize(stmt);

	// init vmsg table
	ret = sqlite3_prepare(om->om_db, kORAL_VMSG_TABLE_CREATE, -1, &stmt, NULL);
	if(ret != SQLITE_OK){
		swapi_log_warn("prepare for create table fail:%d\n", ret);

		return -1;
	}

	ret = sqlite3_step(stmt);
	if(ret != SQLITE_DONE){
		swapi_log_warn("create table fail!\n");
	}

	sqlite3_finalize(stmt);

	return (ret == SQLITE_DONE) ? 0 : -1;
}

int oral_model_init(){
	oral_model_t	*om = get_model();
	int				ret;

	if(om->om_init){
		swapi_log_warn("re-init oral model!\n");
		return 0;
	}

	sqlite3_initialize();

	ret = sqlite3_open(kORAL_DB_FILE, &om->om_db);
	if(SQLITE_OK != ret){
		swapi_log_warn("open sqlite db %s fail!\n", kORAL_DB_FILE);
		return -1;
	}
	oral_init_table(om);

	if(swapi_mutex_init(&om->om_lock) != 0){
		swapi_log_warn("init mutex fail!\n");
		sqlite3_close(om->om_db);
		return -1;
	}

	INIT_LIST_HEAD(&om->om_users);
	om->om_count = 0;
	om->om_cur = NULL;
	om->om_init = 1;

	oral_load_user(om);

	return 0;
}

int oral_model_fini(){
	oral_model_t	*om = get_model();

	if(om->om_init == 0){
		return -1;
	}
	om->om_init = 0;

	swapi_mutex_fini(&om->om_lock);

	sqlite3_close(om->om_db);
	om->om_count = 0;
	om->om_cur = 0;

	oral_clear_user(om);

	sqlite3_shutdown();

	return 0;
}


