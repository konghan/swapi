/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "oral_view.h"

#include "swapi_image.h"

#include "swapi_sys_logger.h"

#define kORAL_IMAGE_FILE_LEN		32

static void oral_view_user_draw(swapi_view_t *sv, swapi_canvas_t *cvs){
	oral_view_user_t	*ou;
	swap_user_t			*u;
	swapi_image_t		img;
	char				png[kORAL_IMAGE_FILE_LEN];

	ASSERT(sv != NULL);
	swapi_log_info("oral swap is drawing...\n");

	ou = container_of(sv, oral_view_user_t, ou_view);
	u = ou->ou_user;

	swapi_canvas_draw_color(cvs, 100, 100, 100, 255);
	
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);

	swapi_canvas_font_set_size(cvs, 20);
	swapi_canvas_set_line(cvs, 10);

	if(u == NULL){
		swapi_canvas_draw_text(cvs, "No User", 0, 32.0, 32.0);
	}else{
		// draw picture of user + name
		snprintf(png, kORAL_IMAGE_FILE_LEN, "%d.png", u->su_picid);
		if(swapi_image_init_from_png(&img, png) == 0){
			swapi_canvas_draw_image(cvs, &img, 16, 16);
			swapi_image_fini(&img);
		}

		swapi_canvas_draw_text(cvs, u->su_name, 0, 64.0, 32.0);
	}
}

int oral_view_user_init(oral_view_user_t *ovu, swapi_window_t *win){
	ASSERT((ovu != NULL) && (win != NULL));

	if(swapi_view_init(&ovu->ou_view, win, 0, 0, swapi_window_get_width(win),
			swapi_window_get_height(win)) != 0){
		swapi_log_warn("init oral view user fail!\n");
		return -1;
	}
	ovu->ou_view.on_draw = oral_view_user_draw;

	ovu->ou_user = swap_user_first();
	
	if(ovu->ou_user == NULL){
		swapi_log_info("without users!\n");
	}

	return 0;
}

int oral_view_user_fini(oral_view_user_t *ovu){
	ASSERT(ovu != NULL);

	swapi_view_fini(&ovu->ou_view);
	return 0;
}

int oral_view_user_next(oral_view_user_t *ovu){
	ASSERT(ovu != NULL);

	ovu->ou_user = swap_user_next(ovu->ou_user);

	swapi_view_draw(&ovu->ou_view);

	return 0;
}

int oral_view_user_prev(oral_view_user_t *ovu){
	ASSERT(ovu != NULL);

	ovu->ou_user = swap_user_prev(ovu->ou_user);

	swapi_view_draw(&ovu->ou_view);

	return 0;
}

