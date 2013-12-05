/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_SURFACE_H__
#define __SWAPI_SURFACE_H__

#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swapi_surface{
	int					ss_init;

	cairo_surface_t		*ss_sf;
	cairo_t				*ss_cr;
	
	cairo_font_face_t	*ss_font;
}swapi_surface_t;

int swapi_surface_init(swapi_surface_t *sf, int width, int height, int format);
int swapi_surface_fini(swapi_surface_t *sf);

int swapi_surface_init_from_png(swapi_surface_t *sf, const char *png);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_SURFACE_H__

