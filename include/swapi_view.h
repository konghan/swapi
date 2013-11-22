/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_VIEW_H__
#define __SWAPI_VIEW_H__

#include "swapi_canvas.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_window;
	
typedef struct swapi_view{
	struct swapi_window	*sv_win;

	struct list_head	sv_node;

	swapi_canvas_t		sv_canvas;
	
	int					sv_x;
	int					sv_y;
	int					sv_width;
	int					sv_height;

	void (*on_draw)(struct swapi_view *sv, swapi_canvas_t *sc);
	int  (*on_key_down)(struct swapi_view *sv, int key);
	int	 (*on_key_up)(struct swapi_view *sv, int key);
	int  (*on_key_multiple)(struct swapi_view *sv, int key);
	int  (*on_key_longpress)(struct swapi_view *sv, int key);
	int  (*on_touch)(struct swapi_view *sv, int motion);
	void (*on_focus)(struct swapi_view *sv, int focus);
}swapi_view_t;

int swapi_view_create(struct swapi_window *win, int x, int y, int width, int height,
		swapi_view_t **sv);
int swapi_view_destroy(struct swapi_window *win, swapi_view_t *sv);

void swapi_view_draw(swapi_view_t *sv);

// used by swapi internally
int _view_init(struct swapi_window *win, swapi_view_t *sv, int x, int y,
		int width, int height);
int _view_fini(swapi_view_t *sv);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_VIEW_H__

