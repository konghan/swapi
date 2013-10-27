/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_render.h"

#include "swapi_sys_thread.h"

#include "native_graphic.h"

#include <cairo/cairo.h>

typedef struct swapi_render{
	swapi_spinlock_t	sr_lock;
	int					sr_init;

	cairo_t				*sr_cairo;
	cairo_surface_t		*sr_surface;

	int					sr_width;
	int					sr_height;
	int					sr_rgbtype;
}swapi_render_t;

static swapi_render_t	__gs_sr = {};

static swapi_render_t *get_render(){
	return &__gs_sr;
}

int swapi_render_init(){
	swapi_render_t			*sr;
	native_graphic_info_t	info;

	native_graphic_getinfo(&info);

	sr = get_render();
	sr->sr_width = info.ngi_width;
	sr->sr_height = info.ngi_height;
	sr->sr_rgbtype = info.ngi_rgbtype;

	sr->sr_surface = cairo_image_surface_create(sr->sr_rgbtype,
			sr->sr_width, sr->sr_height);
	if(sr->sr_surface == NULL){
		return -1;
	}

	sr->sr_cairo = cairo_create(sr->sr_surface);
	if(sr->sr_cairo == NULL){
		cairo_surface_destroy(sr->sr_surface);
		return -1;
	}

	swapi_spin_init(&sr->sr_lock);
	sr->sr_init =  1;

	return 0;
}

int swapi_render_fini(){
	swapi_render_t *sr = get_render();
	
	if(sr->sr_init){
		sr->sr_init = 0;
		cairo_destroy(sr->sr_cairo);
		cairo_surface_destroy(sr->sr_surface);
		swapi_spin_fini(&sr->sr_lock);
	}

	return 0;
}

int swapi_render_flush(){
	swapi_render_t		*sr = get_render();
	swapi_swap_t		*swap;
	swapi_view_t		*view;
	cairo_surface_t		*surface;

	ASSERT(sr->sr_init != 0);

	swapi_spin_lock(&sr->sr_lock);

	swap = swapi_loop_topswap();
	if(swap == NULL){
		swapi_log_warn("no swap on the top\n");
		swapi_spin_unlock(&sr->sr_lock);
		return -1;
	}
	view = swapi_swap_topview(swap);
	if(view == NULL){
		swapi_log_warn("swap without veiw\n");
		swapi_spin_unlock(&sr->sr_lock);
		return -1;
	}

	surface = swapi_shell_get_surface();
	if(surface == NULL){
		swapi_log_warn("shell without surface\n");
		swapi_spin_unlock(&sr->sr_lock);
		return -1;
	}

	if(swapi_view_is_fullscreen(view)){
		native_graphic_draw(swapi_view_get_surface(view), 0, 0, sr->sr_width, sr->sr_height);
	}else{
		native_graphic_draw(surface, 0, 0, sr->sr_width, kSWAPI_SHELL_HEIGHT);
		native_graphic_draw(swapi_view_get_surface(view), 0, kSWAPI_SHELL_HEIGHT,
				sr->sr_width, sr->sr_height - kSWAPI_SHELL_HEIGHT);
	}

	native_flush_device();

	swapi_spin_unlock(&sr->sr_lock);

	return 0;
}

