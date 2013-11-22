/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_WINDOW_H__
#define __SWAPI_WINDOW_H__

#include "swapi_message.h"
#include "swapi_surface.h"
#include "swapi_widget.h"

#include "swapi_sys_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_window{
	swapi_widget_t		sw_wdgt;

	swapi_surface_t		sw_sf;
	
	int					sw_width;
	int					sw_height;
	int					sw_rgbtype;

	swapi_widget_t		*sw_focus;

	swapi_spinlock_t	sw_lock;

	struct list_head	sw_widgets;

	struct list_head	sw_node;
};

int swapi_window_create(swapi_window_t **win);
int swapi_window_destroy(swapi_window_t *win);

int swapi_window_invoke(swapi_window_t *win, swapi_message_t *msg);
int swapi_window_draw(swapi_window_t *win);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_WINDOW_H__

