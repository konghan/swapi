/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_SWUI_H__
#define __SWAPI_SWUI_H__

#include "swapi_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_widget{
	swapi_window_t		*sw_win;

	struct list_head	sw_node;

	swapi_canvas_t		sw_canvas;
	
	int					sw_x;
	int					sw_y;
	int					sw_width;
	int					sw_height;

	void (*on_draw)(swapi_widget_t *swdg, swapi_canvas_t *sc);
	int  (*on_key_down)(swapi_widget_t *swdg, int key);
	int	 (*on_key_up)(swapi_widget_t *swdg, int key);
	int  (*on_key_multiple)(swapi_widget_t *swdg, int key);
	int  (*on_key_longpress)(swapi_widget_t *swdg, int key);
	int  (*on_touch)(swapi_widget_t *swdg, int motion);
	void (*on_focus)(swapi_widget_t *swdg, int focus);
};

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


int swapi_widget_create(swapi_window_t *win, int x, int y, int width, int height,
		swapi_widget_t **sw);
int swapi_widget_destroy(swapi_widget_t *sw);
int swapi_widget_draw(swapi_widget_t *sw);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_SWUI_H__

