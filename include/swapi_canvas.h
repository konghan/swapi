/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_CANVAS_H__
#define __SWAPI_CANVAS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_surface;
struct swapi_window;

typedef struct swapi_canvas{
	struct swapi_surface *sc_sf;

	int					sc_x;	// x in parent surface
	int					sc_y;
	int					sc_width;
	int					sc_height;
}swapi_canvas_t;

int swapi_canvas_init(struct swapi_window *win, swapi_canvas_t *canvas,
		int x, int y, int width, int height);
int swapi_canvas_fini(swapi_canvas_t *canvas);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_CANVAS_H__

