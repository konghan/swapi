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
	struct swapi_window		*sv_win;
	swapi_canvas_t			sv_canvas;

	void (*on_draw)(struct swapi_view *sv, swapi_canvas_t *cvs);
	
	int  (*on_key_down)(struct swapi_view *sv, int key);
	int	 (*on_key_up)(struct swapi_view *sv, int key);
	int  (*on_key_multiple)(struct swapi_view *sv, int key);
	int  (*on_key_longpress)(struct swapi_view *sv, int key);
	int	 (*on_key_click)(struct swapi_view *sv, int key);

	int  (*on_touch)(struct swapi_view *sv, int motion);
	void (*on_focus)(struct swapi_view *sv, int focus);
}swapi_view_t;

int swapi_view_init(swapi_view_t *sv, struct swapi_window *win, int x, int y, int w, int h);
int swapi_view_fini(swapi_view_t *sv);

int swapi_view_create(struct swapi_window *win, int x, int y, int w, int h, swapi_view_t **sv);
int swapi_view_destroy(swapi_view_t *sv);

void swapi_view_draw(swapi_view_t *sv);

static inline swapi_canvas_t *swapi_view_get_canvas(swapi_view_t *view){
	return &view->sv_canvas;
}

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_VIEW_H__

