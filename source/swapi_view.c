/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_view.h"

#include "swapi_handler.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_cache.h"


struct swapi_view{
	cairo_t				*sv_cairo;
	cairo_surface_t		*sv_surface;
	
	int					sv_width;
	int					sv_height;

	struct list_head	sv_node;

	swapi_hanlder_t		*sv_handlers;
};

cairo_t *swapi_view_get_cairo(swapi_view_t *sv){
	return sv->sv_cairo;
}

cairo_surface_t *swapi_view_get_surface(swapi_view_t *sv){
	return cv->sv_surface;
}

int swapi_view_get_width(swapi_view_t *sv){
	return sv->sv_width;
}

int swapi_view_get_height(swapi_view_t *sv){
	return sv->sv_height;
}

swapi_handler_t *swapi_view_get_handlers(swapi_view_t *sv){
	return sv->sv_handlers;
}


int swapi_view_create(int fullscreen, swapi_view_t **sv){
	swapi_view_t			*v;
	native_graphic_info_t	ngi;
	int						h,w;

	ASSERT(sv != NULL);
	
	native_graphic_getinfo(&ngi);
	switch(fullscreen){
		case kSWAPI_VIEW_FULLSCREEN:
			h = ngi.ngi_height;
			w = ngi.ngi_width;
			break;
		case kSWAPI_VIEW_APPSCREEN:
			h = ngi.ngi_height - kSWAPI_SHELL_HEIGHT;
			w = ngi.ngi_width;
			break;

		default:
			return -EINVAL;
	}


	v = swapi_heap_alloc(sizeof(swapi_view_t));
	if(v == NULL){
		swapi_log_warn("create view fail: no more memory\n");
		return -ENOMEM;
	}
	memset(v, 0, sizeof(swapi_view_t));

	v->sv_surface = cairo_image_surface_create(ngi.ngi_rgbtype, w, h);
	if(v->sv_surface == NULL){
		swapi_log_warn("create surface fail!\n");
		goto exit_surface;
	}

	v->sv_cairo = cairo_create(v->sv_surface);
	if(v->sv_cairo == NULL){
		swapi_log_warn("create cairo fail!\n");
		goto exit_cairo;
	}

	v->sv_width = w;
	v->sv_height = h;
	INIT_LIST_HEAD(&v->sv_node);

	if(swapi_handler_create(kSWAPI_HANDLER_DEFAULT, &v->sv_handlers) != 0){
		swapi_log_warn("create handler fail!\n");
		goto exit_handler;
	}

	*sv = v;

	return 0;

exit_handler:
	cairo_destroy(v->sv_cairo);
	
exit_cairo:
	cairo_surface_destroy(v->sv_surface);

exit_surface:
	swapi_heap_free(v);

	return -1;
}

int swapi_view_destroy(swapi_view_t *sv){
	
	ASSERT(sv != NULL);

	list_del(&sv->sv_node);
	swapi_handler_destroy(sv->sv_handlers);

	cairo_destroy(v->sv_cairo);
	cairo_surface_destroy(v->sv_surface);
	
	swapi_heap_free(v);

	return 0;
}

