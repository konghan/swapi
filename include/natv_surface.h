/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __NATV_SURFACE_H__
#define __NATV_SURFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

struct swapi_surface;

typedef struct natv_surface_info{
	int		nsi_width;
	int		nsi_height;
	int		nsi_type;
}natv_surface_info_t;

int natv_surface_getinfo(natv_surface_info_t *info);

int natv_surface_draw(struct swapi_surface *sf, int x, int y, int width, int height);
int natv_surface_rendering();


int natv_surface_module_init();
int natv_surface_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__NATV_SURFACE_H__
