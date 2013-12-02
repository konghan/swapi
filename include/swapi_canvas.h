/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_CANVAS_H__
#define __SWAPI_CANVAS_H__

#include "swapi_surface.h"
#include "swapi_sys_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kMATH_PI					3.1415926

enum {
	kSWAPI_CANVAS_TYPE_LOCAL_SURFACE = 0x00001,
	kSWAPI_CANVAS_TYPE_POINT_SURFACE = 0x00002,
	kSWAPI_CANVAS_TYPE_LOCAL		 = 0x10000,
};

typedef struct swapi_canvas{
	int					sc_type;

	struct swapi_surface	*sc_sfptr;
	struct swapi_canvas		*sc_cvsp;
	
	struct swapi_surface	sc_surface;
	
	atomic_t			sc_ref;

	int					sc_x;	// x in parent surface
	int					sc_y;
	int					sc_width;
	int					sc_height;
	int					sc_format;
}swapi_canvas_t;

int swapi_canvas_create_image(int width, int height, int format, swapi_canvas_t **cvs);
int swapi_canvas_destroy(swapi_canvas_t *cvs);

int swapi_canvas_set_color(swapi_canvas_t *cvs, int red, int green, int blue, int alpha);
int swapi_canvas_set_line(swapi_canvas_t *cvs, int width);

int swapi_canvas_font_set_size(swapi_canvas_t *cvs, float size);

int swapi_canvas_draw_line(swapi_canvas_t *cvs, float sx, float sy, float ex, float ey);
int swapi_canvas_draw_arc(swapi_canvas_t *cvs, float xc, float yc,
		float radius, float angle1, float angle2);

int swapi_canvas_draw_rectangle(swapi_canvas_t *cvs, float x, float y, float width, float height);
int swapi_canvas_draw_canvas(swapi_canvas_t *cvs, float x, float y, swapi_canvas_t *cvspaint);

int swapi_canvas_draw_color(swapi_canvas_t *cvs, int r, int g, int b, int alpha);
int swapi_canvas_draw_text(swapi_canvas_t *cvs, const char *text, int len,float x, float y);

int swapi_canvas_fill(swapi_canvas_t *cvs);
int swapi_canvas_stroke(swapi_canvas_t *cvs);

static inline int swapi_canvas_get_width(swapi_canvas_t *cvs){
	return cvs->sc_width;
}

static inline int swapi_canvas_get_height(swapi_canvas_t *cvs){
	return cvs->sc_height;
}

static inline int swapi_canvas_get_format(swapi_canvas_t *cvs){
	return cvs->sc_format;
}

static inline int swapi_canvas_addref(swapi_canvas_t *cvs){
	atomic_inc(&cvs->sc_ref);
	return 0;
}

static inline int swapi_canvas_rmvref(swapi_canvas_t *cvs){
	if(atomic_dec(&cvs->sc_ref) == 0){
		swapi_canvas_destroy(cvs);
	}
	return 0;
}

// used by swapi internally
int _canvas_init(swapi_canvas_t *cvsp, swapi_canvas_t *cvs,
		int x, int y, int width, int height, int format);
int _canvas_fini(swapi_canvas_t *cvs);

static inline struct swapi_surface *_canvas_get_surface(swapi_canvas_t *cvs){
	return cvs->sc_sfptr;
};

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_CANVAS_H__

