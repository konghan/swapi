/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __ORAL_VIEW_H__
#define __ORAL_VIEW_H__

#include "swapi_window.h"
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

typedef struct oral_view_rcrd{
	swapi_view_t		or_view;

	oral_vmsg_item_t	*or_item;
}oral_view_rcrd_t;

int oral_view_user_init(oral_view_user_t *ovu, swapi_window_t *win);
int oral_view_user_fini(oral_view_user_t *ovu);

int oral_view_user_next(oral_view_user_t *ovu);
int oral_view_user_prev(oral_view_user_t *ovu);

#ifdef __cplusplus
}
#endif

#endif //__ORAL_VIEW_H__

