/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_WIDGET_H__
#define __SWAPI_WIDGET_H__

#include "swapi_canvas.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_window;
	
typedef struct swapi_widget{
	struct swapi_window	*sw_win;

	struct list_head	sw_node;

	swapi_canvas_t		sw_canvas;
	
	int					sw_x;
	int					sw_y;
	int					sw_width;
	int					sw_height;

	void (*on_draw)(struct swapi_widget *sw, swapi_canvas_t *sc);
	int  (*on_key_down)(struct swapi_widget *sw, int key);
	int	 (*on_key_up)(struct swapi_widget *sw, int key);
	int  (*on_key_multiple)(struct swapi_widget *sw, int key);
	int  (*on_key_longpress)(struct swapi_widget *sw, int key);
	int  (*on_touch)(struct swapi_widget *sw, int motion);
	void (*on_focus)(struct swapi_widget *sw, int focus);
}swapi_widget_t;

int swapi_widget_create(struct swapi_window *win, int x, int y, int width, int height,
		swapi_widget_t **sw);
int swapi_widget_destroy(struct swapi_window *win, swapi_widget_t *sw);

void swapi_widget_draw(swapi_widget_t *sw);

// used by swapi internally
int _widget_init(struct swapi_window *win, swapi_widget_t *sw, int x, int y,
		int width, int height);
int _widget_fini(swapi_widget_t *sw);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_WIDGET_H__

