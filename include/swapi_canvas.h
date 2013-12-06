/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_CANVAS_H__
#define __SWAPI_CANVAS_H__

#include "swapi_surface.h"
#include "swapi_image.h"


#ifdef __cplusplus
extern "C" {
#endif

#define kMATH_PI					3.1415926

typedef struct swapi_canvas{
	int						sc_init;

	cairo_surface_t			*sc_sf;
	cairo_t					*sc_cr;
}swapi_canvas_t;

int swapi_canvas_init_image(swapi_canvas_t *cvs, int width, int height, int format);

int swapi_canvas_init_from_surface(swapi_canvas_t *cvs, int x, int y,
		int width, int height, cairo_surface_t *sf);

int swapi_canvas_fini(swapi_canvas_t *cvs);

int swapi_canvas_set_color(swapi_canvas_t *cvs, int red, int green, int blue, int alpha);
int swapi_canvas_set_line(swapi_canvas_t *cvs, int width);

int swapi_canvas_font_set_size(swapi_canvas_t *cvs, float size);

int swapi_canvas_draw_line(swapi_canvas_t *cvs, float sx, float sy, float ex, float ey);
int swapi_canvas_draw_arc(swapi_canvas_t *cvs, float xc, float yc,
		float radius, float angle1, float angle2);

int swapi_canvas_draw_rectangle(swapi_canvas_t *cvs, float x, float y, float width, float height);

int swapi_canvas_draw_color(swapi_canvas_t *cvs, int r, int g, int b, int alpha);
int swapi_canvas_draw_text(swapi_canvas_t *cvs, const char *text, int len,float x, float y);

int swapi_canvas_draw_image(swapi_canvas_t *cvs, swapi_image_t *img, float x, float y);
int swapi_canvas_draw_canvas(swapi_canvas_t *cvs, float x, float y, swapi_canvas_t *cvspaint);

int swapi_canvas_fill(swapi_canvas_t *cvs);
int swapi_canvas_stroke(swapi_canvas_t *cvs);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_CANVAS_H__

