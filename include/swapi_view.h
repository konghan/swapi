/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_VIEW_H__
#define __SWAPI_VIEW_H__

#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	kSWAPI_VIEW_FULLSCREEN = 0,
	kSWAPI_VIEW_APPSCREEN,
};

struct swapi_view;
typedef struct swapi_view swapi_view_t;

int swapi_view_create(int fullscreen, swapi_view_t **sv);
int swapi_view_destroy(swapi_view_t *sv);

cairo_t *swapi_view_get_cairo(swapi_view_t *sv);
cairo_surface_t *swapi_view_get_surface(swapi_view_t *sv);
int swapi_view_get_width(swapi_view_t *sv);
int swapi_view_get_height(swapi_view_t *sv);
swapi_handler_t *swapi_view_get_handlers(swapi_view_t *sv);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_VIEW_H__

