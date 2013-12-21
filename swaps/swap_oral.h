/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAP_ORAL_H__
#define __SWAP_ORAL_H__

#include "swapi_swap.h"
#include "oral_view.h"

#ifdef __cplusplus
extern "C" {
#endif

enum{
	kORAL_VIEW_USER = 0,
	kORAL_VIEW_VMSG,
	kORAL_VIEW_TEXT,
	kORAL_VIEW_RCRD
};

int swap_oral_init(swapi_swap_t **swap);
int swap_oral_fini();

int oral_view_vmsg_set(swapi_uuid_t *uid);
int oral_view_text_set(oral_vmsg_t *ov);

int oral_view_switch(int vid);

#ifdef __cplusplus
}
#endif

#endif //__SWAP_ORAL_H__

