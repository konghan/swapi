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

int swapi_canvas_draw_text_line(swapi_canvas_t *cvs, const char *utf8, int len, float x, float y,
		float width, float height, float mlr, float mtb, unsigned int style){
	cairo_scaled_font_t		*scfont;
	cairo_text_extents_t	ext;
	cairo_glyph_t			*glyphs = NULL, *ge = NULL;
	cairo_status_t			status;
	float					nx = 0, ny = 0;
	int						nums, ne;
	int						i;

	scfont = cairo_get_scaled_font(cvs->sc_cr);
	if(scfont == NULL){
		swapi_log_warn("get scaled font from cr fail\n");
		return -1;
	}

	status = cairo_scaled_font_text_to_glyphs(scfont, x, y, utf8, len,
			&glyphs, &nums, NULL, NULL, NULL);
	if(status != CAIRO_STATUS_SUCCESS){
		swapi_log_warn("text to glyph fail:%s\n", cairo_status_to_string(status));
		return -1;
	}
	
	cairo_scaled_font_text_to_glyphs(scfont, x, y, ".", 1, &ge, &ne, NULL, NULL, NULL);

	cairo_scaled_font_glyph_extents(scfont, glyphs, nums, &ext);
	if(ext.width > width)
		ext.width = width;
	
	if(style & SWAPI_TEXT_STYLE_ALIGN_RIGHT){
		nx = width - ext.width - mlr*2;
		ny = mtb + ext.height;
	}else if(style & SWAPI_TEXT_STYLE_ALIGN_CENTER){
		nx = (width - ext.width - mlr*2)/2;
		ny = (height - ext.height - mtb*2)/2 + ext.height;
	}else{
		nx = mlr;
		ny = mtb + ext.height;
	}

	if(nx <= 0)
		nx = mlr;

	for(i = 0; i < nums; i++){
		if(((glyphs[i]).x + nx) > (x + width)){
			nums = i - 1;

			if(nums >= 3){
				(glyphs[nums-1]).index = ge->index;
				(glyphs[nums-2]).index = ge->index;
				(glyphs[nums-3]).index = ge->index;
			}
			break;
		}

		(glyphs[i]).x += nx;
		(glyphs[i]).y = ny;
	}

	cairo_show_glyphs(cvs->sc_cr, glyphs, nums);

	cairo_glyph_free(glyphs);
	cairo_glyph_free(ge);

	return 0;
}

int swapi_canvas_draw_text_rect(swapi_canvas_t *cvs, const char *utf8, int len, float x, float y,
		float width, float height, float mlr, float mtb){
	cairo_scaled_font_t		*scfont;
	cairo_text_extents_t	ext;
	cairo_glyph_t			*glyphs = NULL, *ge = NULL;
	cairo_status_t			status;
	int						nums, ne;
	int						i, j;
	float					ny = mtb, nx = 0;

	scfont = cairo_get_scaled_font(cvs->sc_cr);
	if(scfont == NULL){
		swapi_log_warn("cairo without scaled font\n");
		return -1;
	}

	status = cairo_scaled_font_text_to_glyphs(scfont, x+mlr, y+mtb, utf8, len,
			&glyphs, &nums, NULL, NULL, NULL);
	if(status != CAIRO_STATUS_SUCCESS){
		swapi_log_warn("text to glyph fail:%s\n", cairo_status_to_string(status));
		return -1;
	}

	cairo_scaled_font_text_to_glyphs(scfont, x, y, ".", 1, &ge, &ne, NULL, NULL, NULL);
	cairo_scaled_font_glyph_extents(scfont, glyphs, nums, &ext);
 
	width -= ext.width / nums;
	if(width < 0)
		width = 0;

	// first line
	nx = 0;
	ny = ext.height;

	for(i = 0, j = 1; i < nums; i++){
		if(((glyphs[i]).x + mlr) >= (width + x + mlr)){
			nx = (glyphs[i]).x - (glyphs[0]).x;
			ny += j*ext.height + 2;
			j++;
		}

		(glyphs[i]).x -= nx;
		(glyphs[i]).y += ny;
	}
		
	cairo_show_glyphs(cvs->sc_cr, glyphs, nums);
	cairo_glyph_free(glyphs);
	cairo_glyph_free(ge);

	return 0;
}


