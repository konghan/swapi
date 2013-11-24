/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */
#include "swapi_window.h"

#include "swapi_sys_cache.h"
#include "swapi_sys_logger.h"

#include "natv_surface.h"

int swapi_window_create(swapi_window_t **win){
	swapi_window_t			*w;
	natv_surface_info_t		nsi;
	
	ASSERT(win != NULL);

	w = swapi_heap_alloc(sizeof(*w));
	if(w == NULL){
		swapi_log_warn("alloc memory for window fail!\n");
		return -ENOMEM;
	}
	memset(w, 0, sizeof(*w));

	natv_surface_getinfo(&nsi);

	w->sw_width  = nsi.nsi_width;
	w->sw_height = nsi.nsi_height;
	w->sw_rgbtype= nsi.nsi_type;

	if(swapi_surface_init(&w->sw_sf, w->sw_width, w->sw_height, w->sw_rgbtype) != 0){
		swapi_log_warn("window create surface fail!\n");
		
		swapi_heap_free(w);
		return -1;
	}
	
	INIT_LIST_HEAD(&w->sw_widgets);
	INIT_LIST_HEAD(&w->sw_node);

	swapi_spin_init(&w->sw_lock);

	if(_widget_init(w, &w->sw_wdgt, 0, 0, w->sw_width, w->sw_height) != 0){
		swapi_log_warn("window init widget fail!\n");

		swapi_surface_fini(&w->sw_sf);
		swapi_heap_free(w);
		return -1;
	}

	w->sw_focus = &w->sw_wdgt;

	*win = w;

	return 0;
}

int swapi_window_destroy(swapi_window_t *win){
	swapi_widget_t		*pos, *temp;

	ASSERT(win != NULL);

	list_for_each_entry_safe(pos, temp, &win->sw_widgets, sw_node){
		list_del(&pos->sw_node);

		swapi_widget_destroy(win, pos);
	}

	_widget_fini(&win->sw_wdgt);

	swapi_spin_fini(&win->sw_lock);

	swapi_heap_free(win);

	return 0;
}

void _window_render_rectangle(swapi_window_t *win, int x, int y, int width, int height){
}

int swapi_window_draw(swapi_window_t *win){
	swapi_widget_t		*pos;

	ASSERT(win != NULL);

	swapi_widget_draw(&win->sw_wdgt);

	list_for_each_entry(pos, &win->sw_widgets, sw_node){
		swapi_widget_draw(pos);
	}

	_window_render_rectangle(win, 0, 0, win->sw_width, win->sw_height);

	return 0;
}

int swapi_window_invoke(swapi_window_t *win, swapi_message_t *msg){
	swapi_widget_t		*sw;
	int					kcode, kact;
	int					ret = 0;

	ASSERT((win != NULL) && (win->sw_focus != NULL) && (msg != NULL));

	switch(msg->sm_type){
	case kSWAPI_MSGTYPE_KEYBOARD:
		swapi_key_unpack(msg, &kcode, &kact);
		sw = win->sw_focus;
key_proc:
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

		default:
			swapi_log_warn("key action unkonw!\n");
			return -1;
		}

		if((ret != 0) && (sw != &win->sw_wdgt)){
			sw = &win->sw_wdgt;
			goto key_proc;
		}
		return 0;

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


