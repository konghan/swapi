/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_view.h"
#include "swapi_window.h"

#include "natv_surface.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"


static void view_default_on_draw(swapi_view_t *sw, swapi_canvas_t *sc){
}

static int view_default_on_key_down(swapi_view_t *sw, int key){
	return 1;
}

static int view_default_on_key_up(swapi_view_t *sw, int key){
	return 1;
}

static int view_default_on_key_multiple(swapi_view_t *sw, int key){
	return 1;
}

static int view_default_on_key_longpress(swapi_view_t *sw, int key){
	return 1;
}

static int view_default_on_touch(swapi_view_t *sw, int motion){
	return 1;
}

static void view_default_on_focus(swapi_view_t *sw, int focus){
	return ;
}

int _view_init(swapi_window_t *win, swapi_view_t *sw, int x, int y,
		int width, int height){
	ASSERT((win != NULL) && (sw != NULL));

	sw->sv_win = win;
	INIT_LIST_HEAD(&sw->sv_node);

	_canvas_init(&win->sw_canvas, &sw->sv_canvas, x, y, width, height, 0);

	sw->sv_x = x;
	sw->sv_y = y;
	sw->sv_width = width;
	sw->sv_height = height;

	sw->on_draw			 = view_default_on_draw;
	sw->on_key_down		 = view_default_on_key_down;
	sw->on_key_up		 = view_default_on_key_up;
	sw->on_key_multiple  = view_default_on_key_multiple;
	sw->on_key_longpress = view_default_on_key_longpress;

	sw->on_touch		= view_default_on_touch;
	sw->on_focus		= view_default_on_focus;
	
	swapi_spin_lock(&win->sw_lock);
	// FIXME: sort in location
	list_add_tail(&sw->sv_node, &win->sw_views);
	swapi_spin_unlock(&win->sw_lock);

	return 0;
}

int _view_fini(swapi_view_t *sw){
	
	swapi_spin_lock(&sw->sv_win->sw_lock);
	list_del(&sw->sv_node);
	swapi_spin_unlock(&sw->sv_win->sw_lock);

	_canvas_fini(&sw->sv_canvas);
	return 0;
}

int swapi_view_create(swapi_window_t *win, int x, int y, int width, int height,
		swapi_view_t **sw){
	swapi_view_t	*wg;

	ASSERT((win != NULL) && (sw != NULL));

	wg = swapi_heap_alloc(sizeof(*wg));
	if(wg == NULL){
		swapi_log_warn("alloc memory for view fail!\n");
		return -ENOMEM;
	}
	
	_view_init(win, wg, x, y, width, height);

	*sw = wg;

	return 0;
}

int swapi_view_destroy(swapi_view_t *sw){
	ASSERT(sw != NULL);

	_view_fini(sw);

	swapi_heap_free(sw);

	return 0;
}

void swapi_view_draw(swapi_view_t *sw){
	ASSERT(sw != NULL);

	sw->on_draw(sw, &sw->sv_canvas);

	_window_render_rectangle(sw->sv_win, sw->sv_x, sw->sv_y, sw->sv_width, sw->sv_height);
}


