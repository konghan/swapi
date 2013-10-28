/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SHELL_H__
#define __SWAPI_SHELL_H__

#include "swapi_message.h"

#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_SHELL_HEIGHT		10

int swapi_shell_module_init();
int swapi_shell_module_fini();

int swapi_shell_post(swapi_message_t *msg);

cairo_surface_t *swapi_shell_get_surface();


#ifdef __cplusplus
}
#endif

#endif //__SWAPI_SHELL_H__
