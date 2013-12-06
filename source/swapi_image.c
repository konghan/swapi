/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swapi_image.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_thread.h"

int swapi_image_init_from_png(swapi_image_t *si, const char *png){
	ASSERT(si != NULL);

	si->si_sf = cairo_image_surface_create_from_png(png);
	if(si->si_sf == NULL){
		swapi_log_warn("create image from png fail!\n");
		return -1;
	}

	return 0;
}

int swapi_image_fini(swapi_image_t *si){
	ASSERT(si != NULL);

	if(si->si_sf != NULL){
		cairo_surface_destroy(si->si_sf);
	}
	si->si_sf = NULL;

	return 0;
}

