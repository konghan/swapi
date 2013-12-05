/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swapi_image.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

int swapi_image_create_from_png(const char *png, swapi_image_t **si){
	swapi_image_t	*img;

	ASSERT(si != NULL);

	img = swapi_heap_alloc(sizeof(*img));
	if(img == NULL){
		swapi_log_warn("no memory for swapi image!\n");
		return -ENOMEM;
	}

	if(swapi_surface_init_from_png(&img->si_surface, png) != 0){
		swapi_log_warn("init surface from png fail!\n");
		swapi_heap_free(img);
		return -1;
	}

	*si = img;

	return 0;
}

int swapi_image_destroy(swapi_image_t *si){
	ASSERT(si != NULL);

	swapi_surface_fini(&si->si_surface);

	swapi_heap_free(si);

	return 0;
}

