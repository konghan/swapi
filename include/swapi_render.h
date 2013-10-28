/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_RENDER_H__
#define __SWAPI_RENDER_H__


#ifdef __cplusplus
extern "C" {
#endif

#define kSWAPI_RENDER_SHELL_UPDATE		0x00000001
#define kSWAPI_RENDER_SWAP_UPDATE		0x00000002

#define kSWAPI_RENDER_ALL_UPDATE	(kSWAPI_RENDER_SHELL_UPDATE|kSWAPI_RENDER_SWAP_UPDATE)

int swapi_render_init();
int swapi_render_fini();

int swapi_render_flush(int type);

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_RENDER_H__
