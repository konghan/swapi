/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "oral_view.h"

#include "swap_oral.h"
#include "swapi_image.h"

#include "swapi_sys_logger.h"

#define kORAL_USER_DEFAULT_PNG		"../rootfs/sys/user.png"

/*
 * oral user view implementation
 */
#define kORAL_IMAGE_FILE_LEN		32

static int oral_view_user_next(oral_view_user_t *ovu);
static int oral_view_user_prev(oral_view_user_t *ovu);

static void oral_view_user_draw(swapi_view_t *sv, swapi_canvas_t *cvs){
	oral_view_user_t	*ovu;
	oral_user_t			*ou;
	swapi_image_t		img;

	ASSERT((sv != NULL) && (cvs != NULL));

	swapi_log_info("oral swap is drawing...\n");

	ovu = container_of(sv, oral_view_user_t, ou_view);
	ou = ovu->ou_user;

	swapi_canvas_draw_color(cvs, 100, 100, 100, 255);
	swapi_canvas_font_set_size(cvs, 20);
	swapi_canvas_set_line(cvs, 10);

	if(ou == NULL){
		swapi_canvas_draw_text(cvs, "No User", 0, 32.0, 32.0);
		return ;
	}

	// draw picture of user + name
	if(swapi_image_init_from_png(&img, ou->ou_picn) != 0){
		swapi_image_init_from_png(&img, kORAL_USER_DEFAULT_PNG);
	}

	swapi_canvas_draw_image(cvs, &img, 0, 0);
	swapi_image_fini(&img);
	
	swapi_canvas_draw_text(cvs, ou->ou_name, 0, 64.0, 32.0);
}

int oral_view_user_on_key_click(struct swapi_view *sv, int key){
	oral_view_user_t *ovu;

	ASSERT(sv != NULL);
	
	ovu = container_of(sv, oral_view_user_t, ou_view);
	
	swapi_log_info("oral swap on key %d \n", key);

	switch(key){
	case kNATV_KEYDRV_ENTER:
		// open user's vmsg
		oral_view_vmsg_set(&ovu->ou_user->ou_uid);
		oral_view_switch(kORAL_VIEW_VMSG);
		break;
	
	case kNATV_KEYDRV_ESCAPE:
		// switch swaps by loop
		break;
		
	case kNATV_KEYDRV_UP:
		oral_view_user_next(ovu);
		break;
		
	case kNATV_KEYDRV_DOWN:
		oral_view_user_prev(ovu);
		break;

	default:
		break;
	}

	return 0;
}

int oral_view_user_init(oral_view_user_t *ovu, swapi_window_t *win){
	ASSERT((ovu != NULL) && (win != NULL));

	if(swapi_view_init(&ovu->ou_view, win, 0, 0, swapi_window_get_width(win),
			swapi_window_get_height(win)) != 0){
		swapi_log_warn("init oral view user fail!\n");
		return -1;
	}
	
	ovu->ou_view.on_draw = oral_view_user_draw;
	ovu->ou_view.on_key_click = oral_view_user_on_key_click;

	ovu->ou_user = oral_user_current();
	
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

static int oral_view_user_next(oral_view_user_t *ovu){
	ASSERT(ovu != NULL);

	ovu->ou_user = oral_user_next(ovu->ou_user);

	swapi_view_draw(&ovu->ou_view);

	return 0;
}

static int oral_view_user_prev(oral_view_user_t *ovu){
	ASSERT(ovu != NULL);

	ovu->ou_user = oral_user_prev(ovu->ou_user);

	swapi_view_draw(&ovu->ou_view);

	return 0;
}

/*
 * oral vmsg view implementation
 */
static void oral_view_vmsg_draw(swapi_view_t *view, swapi_canvas_t *cvs){
	oral_view_vmsg_t	*ovv;
	oral_vmsg_t			*ov;
	oral_vmsg_ctrl_t	*ctrl;
	int					i;
	float				lm = 2;

	ASSERT((view != NULL) && (cvs != NULL));

	ovv = container_of(view, oral_view_vmsg_t, ov_view);

	swapi_canvas_draw_color(cvs, 100, 100, 100, 255);

	swapi_canvas_font_set_size(cvs, 16);
	swapi_canvas_draw_text(cvs, "+", 0, 63, 5);

	if((ovv->ov_ctrl.ovc_load == 0) || (ovv->ov_ctrl.ovc_count <= 0)){
		swapi_canvas_draw_rectangle(cvs, lm, 5, 18, 120);
		swapi_canvas_draw_text(cvs, "No Message", 0, 32, 25);
		swapi_canvas_stroke(cvs);
		return ;
	}

	ctrl = &ovv->ov_ctrl;

	ov = oral_vmsg_first(ctrl);
	for(i = 0; i < ctrl->ovc_count; i++){
		if(ov == NULL)
			break;

		if(ov->ov_type & kORAL_VMSG_TYPE_SEND){
			swapi_canvas_draw_text(cvs, "by self", 0, 64, 25 + i*20);
		}else{
			swapi_canvas_draw_text(cvs, ov->ov_text, 0, lm, 25 + i*20);
		}
	}

	swapi_canvas_draw_rectangle(cvs, lm, 5 + ovv->ov_idx*20, 18, 120);
	swapi_canvas_stroke(cvs);
}

static int oral_view_vmsg_next(oral_view_vmsg_t *ovv){
	ovv->ov_ctrl.ovc_cur = oral_vmsg_next(&ovv->ov_ctrl, ovv->ov_ctrl.ovc_cur);
	return 0;
}

static int oral_view_vmsg_prev(oral_view_vmsg_t *ovv){
	ovv->ov_ctrl.ovc_cur = oral_vmsg_prev(&ovv->ov_ctrl, ovv->ov_ctrl.ovc_cur);
	return 0;
}

static int oral_view_vmsg_on_key_click(swapi_view_t *view, int key){
	oral_view_vmsg_t	*ovv;
	oral_vmsg_t			*ov;

	ASSERT(view != NULL);
	
	ovv = container_of(view, oral_view_vmsg_t, ov_view);
	
	swapi_log_info("oral swap on key %d \n", key);

	switch(key){
	case kNATV_KEYDRV_ENTER:
		// open user's vmsg
		ov = ovv->ov_ctrl.ovc_cur;
		if(ov != NULL){
			oral_view_text_set(ov);
			oral_view_switch(kORAL_VIEW_TEXT);
		}
		break;
	
	case kNATV_KEYDRV_ESCAPE:
		// switch swaps by loop
		oral_view_switch(kORAL_VIEW_USER);
		break;
		
	case kNATV_KEYDRV_UP:
		if(ovv->ov_idx == 0)
			break;

		if(ovv->ov_idx > 1){
			oral_view_vmsg_next(ovv);
		}
		ovv->ov_idx--;

		swapi_view_draw(view);

		break;
		
	case kNATV_KEYDRV_DOWN:

		if(ovv->ov_idx == kORAL_VMSG_ITEM_MAX)
			break;

		oral_view_vmsg_prev(ovv);
		ovv->ov_idx++;
		swapi_view_draw(view);
		break;

	default:
		break;
	}

	return 0;
}

int oral_view_vmsg_init(oral_view_vmsg_t *ovv, swapi_window_t *win){
	ASSERT((ovv != NULL) && (win != NULL));

	oral_vmsg_ctrl_init(&ovv->ov_ctrl);

	if(swapi_view_init(&ovv->ov_view, win, 0, 0, swapi_window_get_width(win),
			swapi_window_get_height(win)) != 0){
		swapi_log_warn("init oral view vmsg fail!\n");
		return -1;
	}
	
	ovv->ov_view.on_draw = oral_view_vmsg_draw;
	ovv->ov_view.on_key_click = oral_view_vmsg_on_key_click;
	
	return 0;
}

int oral_view_vmsg_fini(oral_view_vmsg_t *ovv){
	ASSERT(ovv != NULL);
	
	swapi_view_fini(&ovv->ov_view);

	return 0;
}

/*
 * view text implement
 */
static void oral_view_text_draw(swapi_view_t *view, swapi_canvas_t *cvs){
	oral_view_text_t	*ovt;

	ASSERT((view != NULL) && (cvs != NULL));

	ovt = container_of(view, oral_view_text_t, ot_view);

	ASSERT(ovt->ot_vmsg != NULL);

	swapi_canvas_draw_color(cvs, 100, 100, 100, 255);

	swapi_canvas_draw_text_rect(cvs, ovt->ot_vmsg->ov_text, ovt->ot_vmsg->ov_text_len,
			0, 0, 128, 128, 2, 2);
}

static int oral_view_text_on_key_click(swapi_view_t *view, int key){
//	oral_view_text_t	*ovt;
//	oral_vmsg_t			*ov;

	ASSERT(view != NULL);
	
//	ovt = container_of(view, oral_view_text_t, ot_view);
	
	swapi_log_info("oral swap on key %d \n", key);

	switch(key){
	case kNATV_KEYDRV_ENTER:
		oral_view_switch(kORAL_VIEW_VMSG);
		break;
	
	case kNATV_KEYDRV_ESCAPE:
		// switch swaps by loop
		oral_view_switch(kORAL_VIEW_VMSG);
		break;
		
	case kNATV_KEYDRV_UP:

		break;
		
	case kNATV_KEYDRV_DOWN:
		break;

	default:
		break;
	}

	return 0;
}

int oral_view_text_init(oral_view_text_t *ovt, swapi_window_t *win){
	ASSERT((ovt != NULL) && (win != NULL));

	if(swapi_view_init(&ovt->ot_view, win, 0, 0, swapi_window_get_width(win),
			swapi_window_get_height(win)) != 0){
		swapi_log_warn("init oral view text fail!\n");
		return -1;
	}

	ovt->ot_vmsg = NULL;
	ovt->ot_view.on_draw = oral_view_text_draw;
	ovt->ot_view.on_key_click = oral_view_text_on_key_click;
	
	return 0;

}

int oral_view_text_fini(oral_view_text_t *ovt){
	ASSERT(ovt != NULL);

	swapi_view_fini(&ovt->ot_view);

	return 0;
}

