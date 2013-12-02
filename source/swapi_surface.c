/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_surface.h"
#include "swapi_sys_thread.h"

#include "swapi_sys_logger.h"

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define kSURFACE_FONT_DEFAULT	"../rootfs/cour.ttf"

typedef struct _surface_module{
	int			sm_init;
	FT_Library	sm_ftlib;
	
	FT_Face		sm_face;

	cairo_font_face_t	*sm_font;
}surface_module_t;

static surface_module_t __gs_sm = {};

static surface_module_t *get_sf() {
	return &__gs_sm;
}

int swapi_surface_module_init(){
	surface_module_t *sm = get_sf();
	if(FT_Init_FreeType(&sm->sm_ftlib)){
		swapi_log_warn("init free type library fail!\n");
		return -1;
	}

	if(FT_New_Face(sm->sm_ftlib, kSURFACE_FONT_DEFAULT, 0, &sm->sm_face)){
		FT_Done_FreeType(sm->sm_ftlib);
		swapi_log_warn("load default font fail!\n");
		return -1;
	}

	sm->sm_font = cairo_ft_font_face_create_for_ft_face(sm->sm_face, 0);
	if(sm->sm_font == NULL){
		swapi_log_warn("create cairo font fail!\n");
		
		FT_Done_Face(sm->sm_face);
		FT_Done_FreeType(sm->sm_ftlib);
		return -1;
	}

	sm->sm_init = 1;

	swapi_log_info("create cairo font ok!\n");

	return 0;
}

int swapi_surface_module_fini(){
	surface_module_t *sm = get_sf();

	if(sm->sm_init){
		sm->sm_init = 0;

		// FIXME: free cairo font
		FT_Done_Face(sm->sm_face);
		FT_Done_FreeType(sm->sm_ftlib);
	}

	return 0;
}

int swapi_surface_init(swapi_surface_t *ss, int width, int height, int format){
	ASSERT(ss != NULL);

	ss->ss_init = 0;

	ss->ss_sf = cairo_image_surface_create(format, width, height);
	if(ss->ss_sf == NULL){
		swapi_log_warn("create image surface fail!\n");

		return -1;
	}

	ss->ss_cr = cairo_create(ss->ss_sf);
	if(ss->ss_cr == NULL){
		swapi_log_warn("create cairo context fail!\n");
		cairo_surface_destroy(ss->ss_sf);

		return -1;
	}

	ss->ss_font = get_sf()->sm_font;
	cairo_set_font_face(ss->ss_cr, ss->ss_font);

	ss->ss_init = 1;

	return 0;
}

int swapi_surface_fini(swapi_surface_t *ss){
	ASSERT(ss != NULL);

	cairo_destroy(ss->ss_cr);
	cairo_surface_destroy(ss->ss_sf);

	return 0;
}

