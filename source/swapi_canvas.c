/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_canvas.h"
#include "swapi_window.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

#include <cairo/cairo.h>


int swapi_canvas_create_image(int width, int height, int format, swapi_canvas_t **cvs){
	swapi_canvas_t		*c;

	ASSERT(cvs != NULL);

	c = swapi_heap_alloc(sizeof(*c));
	if(c == NULL){
		swapi_log_warn("alloc memory for canvas fail!\n");
		return -ENOMEM;
	}
	memset(c, 0, sizeof(*c));

	if(swapi_surface_init(&c->sc_surface, width, height, format) != 0){
		swapi_log_warn("surface init fail!\n");
		swapi_heap_free(c);

		return -1;
	}
	c->sc_sfptr = &c->sc_surface;

	c->sc_type = kSWAPI_CANVAS_TYPE_LOCAL_SURFACE;
	c->sc_x = 0;
	c->sc_y = 0;
	c->sc_width = width;
	c->sc_height = height;
	c->sc_format = format;
	atomic_inc(&c->sc_ref);
	
	*cvs = c;

	return 0;
}

int swapi_canvas_destroy(swapi_canvas_t *cvs){
	ASSERT(cvs != NULL);

	if(atomic_dec(&cvs->sc_ref) == 0){
		if(cvs->sc_type == kSWAPI_CANVAS_TYPE_LOCAL_SURFACE){
			swapi_surface_fini(&cvs->sc_surface);
		}else{
			swapi_canvas_rmvref(cvs->sc_cvsp);
		}

		if(!(cvs->sc_type & kSWAPI_CANVAS_TYPE_LOCAL)){
			swapi_heap_free(cvs);
		}
	}

	return 0;
}

int _canvas_init(struct swapi_canvas *cvsp, swapi_canvas_t *cvs, int x, int y,
		int width, int height, int format){
	ASSERT(cvs != NULL);

	if(cvsp == NULL){
		cvs->sc_type = kSWAPI_CANVAS_TYPE_LOCAL_SURFACE | kSWAPI_CANVAS_TYPE_LOCAL;

		if(swapi_surface_init(&cvs->sc_surface, width, height, format) != 0){
			swapi_log_warn("surface init fail!\n");
			return -1;
		}
		cvs->sc_sfptr = &cvs->sc_surface;
		cvs->sc_cvsp  = NULL;

		cvs->sc_x = 0;
		cvs->sc_y = 0;
	}else{
		cvs->sc_type = kSWAPI_CANVAS_TYPE_POINT_SURFACE;

		cvs->sc_cvsp = cvsp;
		if(cvsp->sc_type == kSWAPI_CANVAS_TYPE_POINT_SURFACE){
			cvs->sc_sfptr = cvsp->sc_sfptr;
		}else{
			cvs->sc_sfptr = &cvsp->sc_surface;
		}
		atomic_inc(&cvsp->sc_ref);

		cvs->sc_x = x;
		cvs->sc_y = y;
	}

	cvs->sc_ref = 1;
	cvs->sc_width = width;
	cvs->sc_height = height;
	cvs->sc_format = format;

	return 0;
}

int _canvas_fini(swapi_canvas_t *cvs){
	ASSERT(cvs != NULL);

	if(atomic_dec(&cvs->sc_ref) == 0){
		if(cvs->sc_type == kSWAPI_CANVAS_TYPE_LOCAL_SURFACE){
			swapi_surface_fini(&cvs->sc_surface);
		}else{
			swapi_canvas_rmvref(cvs->sc_cvsp);
		}

		if(!(cvs->sc_type & kSWAPI_CANVAS_TYPE_LOCAL)){
			swapi_heap_free(cvs);
		}
	}

	return 0;
}

/*
 *
 */
int swapi_canvas_set_color(swapi_canvas_t *cvs, int red, int green, int blue, int alpha){
	const int rgbmax = 256;
	const double rgbradio = 255.0;

	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_set_source_rgba(cvs->sc_sfptr->ss_cr, (red%rgbmax)/rgbradio, (green%rgbmax)/rgbradio,
			(blue%rgbmax)/rgbradio, (alpha%rgbmax)/rgbradio);

	return 0;
}

int swapi_canvas_set_line(swapi_canvas_t *cvs, int width){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));
	
	cairo_set_line_width(cvs->sc_sfptr->ss_cr, width);

	return 0;
}

int swapi_canvas_draw_line(swapi_canvas_t *cvs, float sx, float sy, float ex, float ey){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_move_to(cvs->sc_sfptr->ss_cr, cvs->sc_x + sx, cvs->sc_y + sy);
	cairo_line_to(cvs->sc_sfptr->ss_cr, cvs->sc_x + ex, cvs->sc_y + ey);

	return 0;
}

int swapi_canvas_draw_arc(swapi_canvas_t *cvs, float xc, float yc, float radius,
		float angle1, float angle2){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));
	
	cairo_arc(cvs->sc_sfptr->ss_cr, cvs->sc_x + xc, cvs->sc_y + yc, radius, angle1, angle2);

	return 0;
}

int swapi_canvas_draw_rectangle(swapi_canvas_t *cvs, float x, float y, float width, float height){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_rectangle(cvs->sc_sfptr->ss_cr, cvs->sc_x + x, cvs->sc_y + y, width, height);

	return 0;
}

int swapi_canvas_draw_canvas(swapi_canvas_t *cvs, float x, float y, swapi_canvas_t *cvspaint){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_set_source_surface(cvs->sc_sfptr->ss_cr, cvspaint->sc_sfptr->ss_sf,
			cvs->sc_x + x, cvs->sc_y + y);
	cairo_paint(cvs->sc_sfptr->ss_cr);

	return 0;
}

int swapi_canvas_draw_color(swapi_canvas_t *cvs, int r, int g, int b, int alpha){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	swapi_canvas_set_color(cvs, r, g, b, alpha);
	cairo_paint(cvs->sc_sfptr->ss_cr);

	return 0;
}

int swapi_canvas_draw_image(swapi_canvas_t *cvs, swapi_image_t *img, float x, float y){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_set_source_surface(cvs->sc_sfptr->ss_cr, img->si_surface.ss_sf, x, y);
	cairo_paint(cvs->sc_sfptr->ss_cr);

	return 0;
}

int swapi_canvas_fill(swapi_canvas_t *cvs){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_fill(cvs->sc_sfptr->ss_cr);

	return 0;
}

int swapi_canvas_stroke(swapi_canvas_t *cvs){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_stroke(cvs->sc_sfptr->ss_cr);

	return 0;
}


int swapi_canvas_font_set_size(swapi_canvas_t *cvs, float size){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_set_font_size(cvs->sc_sfptr->ss_cr, size);

	return 0;
}

int swapi_canvas_draw_text(swapi_canvas_t *cvs, const char *text, int len,float x, float y){
	ASSERT((cvs != NULL) && (cvs->sc_sfptr != NULL) && (cvs->sc_sfptr->ss_cr != NULL));

	cairo_move_to(cvs->sc_sfptr->ss_cr, cvs->sc_x + x, cvs->sc_y + y);
	cairo_show_text(cvs->sc_sfptr->ss_cr, text);
	
	return 0;
}


