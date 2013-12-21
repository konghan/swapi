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

	oral_user_t		*ou_user;
}oral_view_user_t;

typedef struct oral_view_vmsg{
	swapi_view_t		ov_view;

	unsigned int		ov_idx;

	oral_vmsg_ctrl_t	ov_ctrl;
}oral_view_vmsg_t;

typedef struct oral_view_text{
	swapi_view_t	ot_view;

	oral_vmsg_t		*ot_vmsg;
}oral_view_text_t;

typedef struct oral_view_rcrd{
	swapi_view_t		or_view;

	oral_vmsg_t			*or_vmsg;
}oral_view_rcrd_t;

int oral_view_user_init(oral_view_user_t *ovu, swapi_window_t *win);
int oral_view_user_fini(oral_view_user_t *ovu);

int oral_view_vmsg_init(oral_view_vmsg_t *ovv, swapi_window_t *win);
int oral_view_vmsg_fini(oral_view_vmsg_t *ovv);

int oral_view_text_init(oral_view_text_t *ovt, swapi_window_t *win);
int oral_view_text_fini(oral_view_text_t *ovt);

int oral_view_rcrd_init(oral_view_rcrd_t *ovr, swapi_window_t *win);
int oral_view_rcrd_fini(oral_view_rcrd_t *ovr);

#ifdef __cplusplus
}
#endif

#endif //__ORAL_VIEW_H__

