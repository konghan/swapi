/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_canvas.h"
#include "swapi_window.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define kCANVAS_FONT_DEFAULT	"../rootfs/cour.ttf"

typedef struct _canvas_data{
	FT_Library			cd_ftlib;
	FT_Face				cd_face;
	cairo_font_face_t	*cd_font;
}canvas_data_t;

static canvas_data_t __gs_cd = {};

static canvas_data_t *get_cd() {
	return &__gs_cd;
}

static inline cairo_font_face_t *_canvas_get_default_font_face(){
	canvas_data_t *cd = get_cd();

	return cd->cd_font;
}

int swapi_canvas_module_init(){
	canvas_data_t *cd = get_cd();

	if(FT_Init_FreeType(&cd->cd_ftlib)){
		swapi_log_warn("init free type library fail!\n");
		return -1;
	}

	if(FT_New_Face(cd->cd_ftlib, kCANVAS_FONT_DEFAULT, 0, &cd->cd_face)){
		FT_Done_FreeType(cd->cd_ftlib);
		swapi_log_warn("load default font fail!\n");
		return -1;
	}

	cd->cd_font = cairo_ft_font_face_create_for_ft_face(cd->cd_face, 0);
	if(cd->cd_font == NULL){
		swapi_log_warn("create cairo font fail!\n");
		
		FT_Done_Face(cd->cd_face);
		FT_Done_FreeType(cd->cd_ftlib);
		return -1;
	}

	swapi_log_info("create cairo font ok!\n");

	return 0;
}

int swapi_canvas_module_fini(){
	canvas_data_t *cd = get_cd();

	cairo_font_face_destroy(cd->cd_font);
	
	FT_Done_Face(cd->cd_face);
	FT_Done_FreeType(cd->cd_ftlib);
	
	return 0;
}

int swapi_canvas_init_image(swapi_canvas_t *cvs, int width, int height, int format){

	ASSERT(cvs != NULL);

	cvs->sc_sf = cairo_image_surface_create(format, width, height);
	if(cvs->sc_sf == NULL){
		swapi_log_warn("create surface  fail!\n");
		return -1;
	}

	cvs->sc_cr = cairo_create(cvs->sc_sf);
	if(cvs->sc_cr == NULL){
		swapi_log_warn("create cairo context fail!\n");
		cairo_surface_destroy(cvs->sc_sf);
		return -1;
	}

	cairo_set_font_face(cvs->sc_cr, _canvas_get_default_font_face());

	cvs->sc_init = 1;

	return 0;
}

int swapi_canvas_init_from_surface(swapi_canvas_t *cvs, int x, int y,
		int width, int height, cairo_surface_t *sf){

	ASSERT((cvs != NULL) || (sf != NULL));

	cvs->sc_sf = cairo_surface_create_for_rectangle(sf, x, y, width, height);
	if(cvs->sc_sf == NULL){
		swapi_log_warn("create surface  fail!\n");
		return -1;
	}

	cvs->sc_cr = cairo_create(cvs->sc_sf);
	if(cvs->sc_cr == NULL){
		swapi_log_warn("create cairo context fail!\n");
		cairo_surface_destroy(cvs->sc_sf);
		return -1;
	}

	cairo_set_font_face(cvs->sc_cr, _canvas_get_default_font_face());

	cvs->sc_init = 1;

	return 0;
}

int swapi_canvas_fini(swapi_canvas_t *cvs){
	ASSERT(cvs != NULL);

	cairo_destroy(cvs->sc_cr);
	cairo_surface_destroy(cvs->sc_sf);

	return 0;
}

/*
 *
 */
int swapi_canvas_set_color(swapi_canvas_t *cvs, int red, int green, int blue, int alpha){
	const int rgbmax = 256;
	const double rgbradio = 255.0;

	ASSERT((cvs != NULL) && (cvs->sc_init));

	cairo_set_source_rgba(cvs->sc_cr, (red%rgbmax)/rgbradio, (green%rgbmax)/rgbradio,
			(blue%rgbmax)/rgbradio, (alpha%rgbmax)/rgbradio);

	return 0;
}

int swapi_canvas_set_line(swapi_canvas_t *cvs, int width){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_set_line_width(cvs->sc_cr, width);

	return 0;
}

int swapi_canvas_draw_line(swapi_canvas_t *cvs, float sx, float sy, float ex, float ey){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_move_to(cvs->sc_cr, sx, sy);
	cairo_line_to(cvs->sc_cr, ex, ey);

	return 0;
}

int swapi_canvas_draw_arc(swapi_canvas_t *cvs, float xc, float yc, float radius,
		float angle1, float angle2){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_arc(cvs->sc_cr, xc, yc, radius, angle1, angle2);

	return 0;
}

int swapi_canvas_draw_rectangle(swapi_canvas_t *cvs, float x, float y, float width, float height){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_rectangle(cvs->sc_cr, x, y, width, height);

	return 0;
}

int swapi_canvas_draw_canvas(swapi_canvas_t *cvs, float x, float y, swapi_canvas_t *cvspaint){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	ASSERT((cvspaint != NULL) && (cvspaint->sc_init));

	cairo_set_source_surface(cvs->sc_cr, cvspaint->sc_sf, x, y);
	cairo_paint(cvs->sc_cr);

	return 0;
}

int swapi_canvas_draw_color(swapi_canvas_t *cvs, int r, int g, int b, int alpha){
	ASSERT((cvs != NULL) && (cvs->sc_init));

	swapi_canvas_set_color(cvs, r, g, b, alpha);
	cairo_paint(cvs->sc_cr);

	return 0;
}

int swapi_canvas_draw_image(swapi_canvas_t *cvs, swapi_image_t *img, float x, float y){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_set_source_surface(cvs->sc_cr, img->si_sf, x, y);
	cairo_paint(cvs->sc_cr);

	return 0;
}

int swapi_canvas_fill(swapi_canvas_t *cvs){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_fill(cvs->sc_cr);

	return 0;
}

int swapi_canvas_stroke(swapi_canvas_t *cvs){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_stroke(cvs->sc_cr);

	return 0;
}


int swapi_canvas_font_set_size(swapi_canvas_t *cvs, float size){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_set_font_size(cvs->sc_cr, size);

	return 0;
}

int swapi_canvas_draw_text(swapi_canvas_t *cvs, const char *text, int len,float x, float y){
	ASSERT((cvs != NULL) && (cvs->sc_init));
	
	cairo_move_to(cvs->sc_cr, x, y);
	cairo_show_text(cvs->sc_cr, text);
	
	return 0;
}


