/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_window.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

#include "natv_surface.h"

int swapi_window_init(swapi_window_t *win, int width, int height, int format){
	ASSERT(win != NULL);

	memset(win, 0, sizeof(*win));

	win->sw_sf = cairo_image_surface_create(format, width, height);
	if( win->sw_sf == NULL){
		swapi_log_warn("init surface fail!\n");

		return -1;
	}

	win->sw_width = width;
	win->sw_height = height;
	win->sw_format = format;

	return 0;
}

int swapi_window_fini(swapi_window_t *win){
	ASSERT(win != NULL);

	cairo_surface_destroy(win->sw_sf);

	return 0;
}

void swapi_window_render(){
	natv_surface_render();
}

int swapi_window_draw(swapi_window_t *win){

	ASSERT(win != NULL);

	if(win->sw_focus != NULL){
		swapi_view_draw(win->sw_focus);
	}

	swapi_window_render();

	return 0;
}

int swapi_window_invoke(swapi_window_t *win, swapi_message_t *msg){
	swapi_view_t		*sw;
	int					kcode, kact;
	int					ret;

	ASSERT((win != NULL) && (win->sw_focus != NULL) && (msg != NULL));

	switch(msg->sm_type){
	case kSWAPI_MSGTYPE_KEYBOARD:
		swapi_key_unpack(msg, &kcode, &kact);
		
		sw = win->sw_focus;

		switch(kact){
		case kNATV_KEY_DOWN:
			ret = sw->on_key_down(sw, kcode);
			break;

		case kNATV_KEY_UP:
			ret = sw->on_key_up(sw, kcode);
			break;

		case kNATV_KEY_LONGPRESS:
			ret = sw->on_key_longpress(sw, kcode);
			break;

		case kNATV_KEY_MULTIPLE:
			ret = sw->on_key_multiple(sw, kcode);
			break;

		case kNATV_KEY_CLICK:
			ret = sw->on_key_click(sw, kcode);
			break;

		default:
			swapi_log_warn("key action unkonw!\n");
			return -1;
		}
		return ret;
		
	case kSWAPI_MSGTYPE_DRAW:
		swapi_window_draw(win);
		return 0;

	case kSWAPI_MSGTYPE_TOUCH:
		return -1;

	default:
		swapi_log_warn("unkonw message type:%d\n", msg->sm_type);
		return -1;
	}

	return 0;
}


