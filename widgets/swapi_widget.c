/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_widget.h"
#include "swapi_window.h"

#include "natv_surface.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"


static void widget_default_on_draw(swapi_widget_t *sw, swapi_canvas_t *sc){
}

static int widget_default_on_key_down(swapi_widget_t *sw, int key){
	return 1;
}

static int widget_default_on_key_up(swapi_widget_t *sw, int key){
	return 1;
}

static int widget_default_on_key_multiple(swapi_widget_t *sw, int key){
	return 1;
}

static int widget_default_on_key_longpress(swapi_widget_t *sw, int key){
	return 1;
}

static int widget_default_on_touch(swapi_widget_t *sw, int motion){
	return 1;
}

static void widget_default_on_focus(swapi_widget_t *sw, int focus){
	return ;
}

int _widget_init(swapi_window_t *win, swapi_widget_t *sw, int x, int y,
		int width, int height){
	ASSERT((win != NULL) && (sw != NULL));

	sw->sw_win = win;
	INIT_LIST_HEAD(&sw->sw_node);

	swapi_canvas_init(win, &sw->sw_canvas, x, y, width, height);

	sw->sw_x = x;
	sw->sw_y = y;
	sw->sw_width = width;
	sw->sw_height = height;

	sw->on_draw = widget_default_on_draw;

	sw->on_key_down		 = widget_default_on_key_down;
	sw->on_key_up		 = widget_default_on_key_up;
	sw->on_key_multiple  = widget_default_on_key_multiple;
	sw->on_key_longpress = widget_default_on_key_longpress;

	sw->on_touch = widget_default_on_touch;
	sw->on_focus = widget_default_on_focus;

	return 0;
}

int _widget_fini(swapi_widget_t *sw){
	swapi_canvas_fini(&sw->sw_canvas);
	return 0;
}

int swapi_widget_create(swapi_window_t *win, int x, int y, int width, int height,
		swapi_widget_t **sw){
	swapi_widget_t	*wg;

	ASSERT((win != NULL) && (sw != NULL));

	wg = swapi_heap_alloc(sizeof(*wg));
	if(wg == NULL){
		swapi_log_warn("alloc memory for widget fail!\n");
		return -ENOMEM;
	}
	
	_widget_init(win, wg, x, y, width, height);

	swapi_spin_lock(&win->sw_lock);
	// FIXME: sort in location
	list_add_tail(&wg->sw_node, &win->sw_widgets);
	swapi_spin_unlock(&win->sw_lock);

	*sw = wg;

	return 0;
}

int swapi_widget_destroy(swapi_window_t *win, swapi_widget_t *sw){
	ASSERT(sw != NULL);

	swapi_spin_lock(&win->sw_lock);
	list_del(&sw->sw_node);
	swapi_spin_unlock(&win->sw_lock);

	_widget_fini(sw);

	swapi_heap_free(sw);

	return 0;
}

void swapi_widget_draw(swapi_widget_t *sw){
	ASSERT(sw != NULL);

	sw->on_draw(sw, &sw->sw_canvas);

	_window_render_rectangle(sw->sw_win, sw->sw_x, sw->sw_y, sw->sw_width, sw->sw_height);
}


