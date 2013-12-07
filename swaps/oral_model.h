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

enum{
	kORAL_VMSG_TYPE_TEXT  = 0x00000001,
	kORAL_VMSG_TYPE_VOICE = 0x00000002,

	kORAL_VMSG_TYPE_SEND  = 0x00010000,
	kORAL_VMSG_TYPE_RECV  = 0x00020000,
};

typedef struct oral_vmsg_item{
	struct list_head		vi_node;

	int						vi_seq;	// sequence number

	int						vi_type; // type of this msg

	int						vi_tlen;
	int						vi_vlen;

	char					*vi_text;
	void					*vi_voice;
}oral_vmsg_item_t;

typedef struct oral_vmsg{
	swapi_uuid_t			ov_uid;

	struct list_head		ov_node;	// link to user
	struct list_head		ov_items;	// item container
}oral_vmsg_t;

int oral_vmsg_load(swapi_uuid_t *uid, oral_vmsg_t **ov);
int oral_vmsg_save(swapi_uuid_t *uid, oral_vmsg_t *ov);

int oral_vmsg_add(oral_vmsg_t *ov, oral_vmsg_item_t *ovi);
int oral_vmsg_del(oral_vmsg_t *ov, oral_vmsg_item_t *ovi);
int oral_vmsg_pack(oral_vmsg_t *ov);

#ifdef __cplusplus
}
#endif

#endif //__ORAL_MODEL_H__

