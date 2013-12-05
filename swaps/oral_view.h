/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __ORAL_VIEW_H__
#define __ORAL_VIEW_H__

#include "swapi_view.h"

#include "oral_model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oral_view_user{
	swapi_view_t	ou_view;

	swap_user_t		*ou_user;
}oral_view_user_t;

typedef struct oral_view_vmsg{
	swapi_view_t	ov_view;

	oral_vmsg_t		*ov_vmsg;
}oral_view_vmsg_t;

typedef struct oral_view_text{
	swapi_view_t		ot_view;

	oral_vmsg_item_t	*ot_item;
}oral_view_text_t;

typedef struct oral_view_record{
	swapi_view_t		or_view;

	oral_vmsg_item_t	*or_item;
}oral_view_record_t;

/*
 * interface of user
 */
static inline swapi_view_t *oral_view_user_get(oral_view_user_t *ou){
	return &ou->ou_view;
}

oral_view_user_t *oral_view_user_create(swapi_window_t *win);
int oral_view_user_destroy(oral_view_user_t *ou);

swap_user_t *oral_view_user_current(oral_view_user_t *ou);

/*
 * interface of vmsg
 */
static inline swapi_view_t *oral_view_vmsg_get(oral_view_vmsg_t *ov){
	return &ov->ov_view;
}

/*
 * interface of text
 */
static inline swapi_view_t *oral_view_text_get(oral_view_text_t *ot){
	return &ot->ot_view;
}

/*
 * interface of record
 */
static inline swapi_view_t *oral_view_record_get(oral_view_record_t *vr){
	return &vr->or_view;
}


#ifdef __cplusplus
}
#endif

#endif //__ORAL_VIEW_H__

