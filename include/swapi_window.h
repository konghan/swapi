/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swapi_message.h"
#include "swapi_canvas.h"
#include "swapi_view.h"

#include "swapi_sys_thread.h"

#include "list.h"

#ifndef __SWAPI_WINDOW_H__
#define __SWAPI_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swapi_window{
	cairo_surface_t		*sw_sf;
	
	int					sw_width;
	int					sw_height;
	int					sw_format;

	swapi_view_t		*sw_focus;
}swapi_window_t;

int swapi_window_init(swapi_window_t *win, int width, int height, int format);
int swapi_window_fini(swapi_window_t *win);

static inline void swapi_window_set_view(swapi_window_t *win, swapi_view_t *view){
	view->sv_win = win;
	win->sw_focus = view;
}

static inline swapi_view_t *swapi_window_get_view(swapi_window_t *win){
	return win->sw_focus;
}

static inline int swapi_window_get_width(swapi_window_t *win){
	return win->sw_width;
}

static inline int swapi_window_get_height(swapi_window_t *win){
	return win->sw_height;
}

static inline int swapi_window_get_format(swapi_window_t *win){
	return win->sw_format;
}

int swapi_window_invoke(swapi_window_t *win, swapi_message_t *msg);

int swapi_window_draw(swapi_window_t *win);

void swapi_window_render(swapi_window_t *win);

// used by swapi internally
static inline cairo_surface_t *_window_get_surface(swapi_window_t *win){
	return win->sw_sf;
}
#ifdef __cplusplus
}
#endif

#endif //__SWAPI_WINDOW_H__

