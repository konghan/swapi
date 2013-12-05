/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAPI_IMAGE_H__
#define __SWAPI_IMAGE_H__

#include "swapi_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swapi_image{
	swapi_surface_t		si_surface;
}swapi_image_t;

int swapi_image_create_from_png(const char *png, swapi_image_t **si);
int swapi_image_destroy(swapi_image_t *si);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_IMAGE_H__

