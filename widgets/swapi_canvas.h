/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_CANVAS_H__
#define __SWAPI_CANVAS_H__

#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swapi_surface{
	cairo_surface_t		*ss_sf;
	cairo_t				*ss_cr;
}swapi_surface_t;

typedef struct swapi_canvas{
	swapi_surface_t		*sc_sf;

	int					sc_x;	// x in parent surface
	int					sc_y;
	int					sc_width;
	int					sc_height;
}swapi_canvas_t;

struct swapi_window;

int swapi_canvas_init(struct swapi_window *win, swapi_canvas_t *canvas, int x, int y,
		int width, int height);
int swapi_canvas_fini(swapi_canvas_t *canvas);

int swapi_surface_init(swapi_surface_t *sf, int width, int height, int rgbtype);
int swapi_surface_fini(swapi_surface_t *sf);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_CANVAS_H__

