/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __NATIVE_GRAPHIC_H__
#define __NATIVE_GRAPHIC_H__

#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct native_graphic_info{
	int		ngi_width;
	int		ngi_height;
	int		ngi_rgbtype;
}native_graphic_info_t;

int native_graphic_getinfo(native_graphic_info_t *info);

int native_graphic_init();
int native_graphic_fini();

cairo_surface_t *native_graphic_get();

int native_graphic_draw(cairo_surface_t *surface, int x, int y, int width, int height);

int native_flush_device();

#ifdef __cplusplus
}
#endif

#endif //__NATIVE_GRAPHIC_H__
