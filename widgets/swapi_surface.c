/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_surface.h"

#include <cairo/cairo.h>

int swapi_surface_init(swapi_surface_t *ss, int width, int height, int rgbtype){
	ASSERT(ss != NULL);

	ss->ss_sf = cairo_image_surface_create(rgbtype, width, height);
	if(ss->ss_sf == NULL){
		swapi_log_warn("create image surface fail!\n");
		return -1;
	}

	ss->ss_cr = cairo_create(ss->ss_sf);
	if(ss->ss_cr == NULL){
		swapi_log_warn("create cairo context fail!\n");
		cairo_surface_destroy(ss->ss_sf);
		return -1;
	}

	return 0;
}

int swapi_surface_fini(swapi_surface_t *ss){
	ASSERT(ss != NULL);

	cairo_destroy(ss->ss_cr);
	cairo_destroy(ss->ss_sf);

	return 0;
}

