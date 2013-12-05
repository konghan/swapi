/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "oral_view.h"

#define kORAL_IMAGE_FILE_LEN		32

static void user_on_draw(struct swapi_view *sv, swapi_canvas_t *cvs){
	oral_view_user_t	*ou;
	swap_user_t			*u;
	swapi_image_t		*img;
	char				png[32]

	ASSERT(sv != NULL);

	ou = container_of(sv, oral_view_user_t, ou_view);
	u = ou->ou_user;

	if(u == NULL){
		swapi_canvas_draw_text(cvs, "No User", 64, 32);
	}else{
		// draw picture of user + name
		snprintf(png, "%d.png", u->su_picid);
		if(swapi_image_create_from_png(png, &img) == 0){
			swapi_canvas_draw_image(cvs, img, 16, 16);
			swapi_image_destroy(img);
		}

		swapi_canvas_draw_text(cvs, u->su_name, 64, 32);
	}
}

static int user_on_key_click(struct swapi_view *sv, int key){
	oral_view_user_t	*ou;

	ASSERT(sv != NULL);

	ou = container_of(sv, oral_view_user_t, ou_view);

	switch(key){
	case kNATV_KEYDRV_ENTER:
		// open user's vmsg
		// oral_view_set();
		break;
		
	case kNATV_KEYDRV_ESCAPE:
		// FIXME: return to next swap
		break;
		
	case kNATV_KEYDRV_UP:
		ou->ou_user = swap_user_prev(ou->ou_user);
		swapi_view_draw(sv);
		break;
		
	case kNATV_KEYDRV_DOWN:
		ou->ou_user = swap_user_next(ou->ou_user);
		swapi_view_draw(sv);
		break;

	default:
		swapi_log_warn("key %d is unkonw\n", key);
		return -1;
	}
}

oral_view_user_t *oral_view_user_create(swapi_window_t *win){
	oral_view_user_t *ou;

	ASSERT(win != NULL);

	ou = swapi_heap_alloc(sizeof(*ou));
	if(ou == NULL){
		swapi_log_warn("no more memory for oral view!\n");
		return NULL;
	}
	memset(ou, 0, sizeof(*ou));

	if(_view_init(win, &ou->ou_view, 0, 0, win->sw_width, win->sw_height) != 0){
		swapi_log_warn("init view fail!\n");
		swapi_heap_free(ou);
		return NULL;
	}

	ou->ou_user = swap_user_first();

	ou->ou_view.on_draw = user_on_draw;
	ou->ou_view.on_key_click = user_on_key_click;

	return ou;
}

int oral_view_user_destroy(oral_view_user_t *ou){
	ASSERT(ou != NULL);

	_view_fini(&ou->ou_view);

	swapi_heap_free(ou);

	return 0;
}

swap_user_t *oral_view_user_current(oral_view_user_t *ou){
	ASSERT(ou != NULL);

	return ou->ou_user;
}

