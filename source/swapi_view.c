/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_view.h"
#include "swapi_window.h"

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

int swapi_view_init(swapi_view_t *view, struct swapi_window *win, int x, int y, int w, int h){
	view->sv_win = win;	
	if(swapi_canvas_init_from_surface(&view->sv_canvas, x, y, w, h, _window_get_surface(win)) != 0){
		swapi_log_warn("init canvas_fail!\n");
		return -1;
	}

	view->on_draw			 = view_default_on_draw;
	view->on_key_down		 = view_default_on_key_down;
	view->on_key_up		 = view_default_on_key_up;
	view->on_key_multiple  = view_default_on_key_multiple;
	view->on_key_longpress = view_default_on_key_longpress;

	view->on_touch		= view_default_on_touch;
	view->on_focus		= view_default_on_focus;

	return 0;
}

int swapi_view_fini(swapi_view_t *sv){
	swapi_canvas_fini(&sv->sv_canvas);
	return 0;
}

int swapi_view_create(swapi_window_t *win, int x, int y, int width, int height,
		swapi_view_t **sw){

	swapi_view_t	*view;

	ASSERT((win != NULL) && (sw != NULL));

	view = swapi_heap_alloc(sizeof(*view));
	if(view == NULL){
		swapi_log_warn("alloc memory for view fail!\n");
		return -ENOMEM;
	}
	memset(view, 0, sizeof(*view));

	swapi_view_init(view, win, x, y, width, height);
	
	*sw = view;

	return 0;
}

int swapi_view_destroy(swapi_view_t *sw){
	ASSERT(sw != NULL);
	
	swapi_view_fini(sw);

	swapi_heap_free(sw);

	return 0;
}

void swapi_view_draw(swapi_view_t *sw){
	ASSERT(sw != NULL);

	sw->on_draw(sw, &sw->sv_canvas);

	swapi_window_render(sw->sv_win);
}

