/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_RENDER_H__
#define __SWAPI_RENDER_H__


#ifdef __cplusplus
extern "C" {
#endif

int swapi_render_init();
int swapi_render_fini();

int swapi_render_flush();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_RENDER_H__
