/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_canvas.h"
#include "swapi_window.h"

#include "swapi_sys_thread.h"

#include <cairo/cairo.h>


int swapi_canvas_init(struct swapi_window *win, swapi_canvas_t *cvs, int x, int y,
		int width, int height){
	ASSERT((win != NULL) && (cvs != NULL));

	cvs->sc_sf = &win->sw_sf;
	cvs->sc_x = x;
	cvs->sc_y = y;
	cvs->sc_width = width;
	cvs->sc_height = height;

	// FIXME:cairo surface inc ref

	return 0;
}

int swapi_canvas_fini(swapi_canvas_t *cvs){
	// FIXME:cairo surface dec ref
	return 0;
}

