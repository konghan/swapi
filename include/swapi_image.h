/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_IMAGE_H__
#define __SWAPI_IMAGE_H__

#include  <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swapi_image{
	cairo_surface_t  *si_sf;
}swapi_image_t;

int swapi_image_init_from_png(swapi_image_t *si, const char *png);
int swapi_image_fini(swapi_image_t *si);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_IMAGE_H__

