/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_canvas.h"

#include <cairo/cairo.h>


int swapi_canvas_init(struct swapi_window *win, swapi_canvas_t *canvas, int x, int y,
		int width, int height){
	ASSERT((win != NULL) && (canvas != NULL));

	canvas->sc_sf = &win->sc_sf;
	canvas->sc_x = x;
	canvas->sc_y = y;
	canvas->sc_width = width;
	canvas->sc_height = height;

	// FIXME:cairo surface inc ref

	return 0;
}

int swapi_canvas_fini(swapi_canvas_t *canvas){
	// FIXME:cairo surface dec ref
	return 0;
}

