/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __ORAL_MODEL_H__
#define __ORAL_MODEL_H__

#include "swap_user.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kORAL_USER_NAME_LEN		16
#define kORAL_USER_PICN_LEN		32
#define kORAL_VMSG_TEXT_LEN		144
#define kORAL_VMSG_VOICE_LEN	(1024*128)

#define kORAL_VMSG_ITEM_MAX		5

enum{
	kORAL_VMSG_TYPE_TEXT  = 0x00000001,
	kORAL_VMSG_TYPE_VOICE = 0x00000002,

	kORAL_VMSG_TYPE_SEND  = 0x00010000,
	kORAL_VMSG_TYPE_RECV  = 0x00020000,
};

typedef struct oral_user{
	struct list_head	ou_node;

	swapi_uuid_t		ou_uid;	// key
	int					ou_name_len;
	int					ou_picn_len;
	char				*ou_name;
	char				*ou_picn;	// picture of user's file name
}oral_user_t;

typedef struct oral_vmsg{
	struct list_head	ov_node;

	swapi_uuid_t		ov_uid;		// key
	int					ov_type;
	int					ov_seq;

	int					ov_text_len;
	int					ov_voice_len;
	char				*ov_text;
	char				*ov_voice; // voice message file name
}oral_vmsg_t;

typedef struct oral_vmsg_ctrl{
	struct list_head	ovc_vmsgs;
	swapi_uuid_t		ovc_uid;

	int					ovc_load;

	unsigned int		ovc_count;
	oral_vmsg_t			*ovc_cur;
}oral_vmsg_ctrl_t;

int oral_user_add(oral_user_t *ou);
int oral_user_del(swapi_uuid_t *uid);
oral_user_t *oral_user_current();
oral_user_t *oral_user_next(oral_user_t *ou);
oral_user_t *oral_user_prev(oral_user_t *ou);

static inline void oral_vmsg_ctrl_init(oral_vmsg_ctrl_t *ovc){
	INIT_LIST_HEAD(&ovc->ovc_vmsgs);
	ovc->ovc_count = 0;
	ovc->ovc_cur = NULL;
	ovc->ovc_load = 0;
}

int oral_vmsg_load(oral_vmsg_ctrl_t *ovc);
int oral_vmsg_clear(oral_vmsg_ctrl_t *ovc);

int oral_vmsg_add(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov);
int oral_vmsg_pack(oral_vmsg_ctrl_t *ovc);

oral_vmsg_t *oral_vmsg_current(oral_vmsg_ctrl_t *ovc);

oral_vmsg_t *oral_vmsg_first(oral_vmsg_ctrl_t *ovc);
oral_vmsg_t *oral_vmsg_last(oral_vmsg_ctrl_t *ovc);

oral_vmsg_t *oral_vmsg_next(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov);
oral_vmsg_t *oral_vmsg_prev(oral_vmsg_ctrl_t *ovc, oral_vmsg_t *ov);

int oral_model_init();
int oral_model_fini();

#ifdef __cplusplus
}
#endif

#endif //__ORAL_MODEL_H__

