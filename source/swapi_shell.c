/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swapi_shell.h"
#include "swapi_queue.h"
#include "swapi_handler.h"
#include "swapi_message.h"
#include "swapi_render.h"

#include "swapi_sys_logger.h"
#include "swapi_sys_thread.h"
#include "list.h"

#include "native_graphic.h"

#include <cairo/cairo.h>

typedef struct swapi_shell_trayicon{
	struct list_head		sst_node;
	
	int						sst_width;
	int						sst_height;

	int						sst_icon_size;
	char					*sst_data;
}swapi_shell_trayicon_t;

typedef struct swapi_shell{
	int					ss_battery;
	int					ss_signals;
	int					ss_message;
	int					ss_tips;
	int					ss_alarm;
	
	struct list_head	ss_swaps;
	
	cairo_surface_t		*ss_surface;
	cairo_t				*ss_context;
	int					ss_width;
	int					ss_height;

	swapi_queue_t		*ss_queue;
	swapi_handler_t		*ss_handler;

	swapi_thread_t		ss_thread;

	int					ss_status;
}swapi_shell_t;

static swapi_shell_t	__gs_ss = {};
static swapi_shell_t *get_shell(){
	return &__gs_ss;
}

#define kSWAPI_SHELL_SIGNAL_ARC_NUM			5

/*
 * shell handlers
 */
static int swapi_shell_timer_handler(swapi_message_t *msg, void *data);
static int swapi_shell_default_handler(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{.she_type = kSWAPI_MSGTYPE_TIMER, .she_node = { }, 
		.she_cbfunc = swapi_shell_timer_handler, .she_data = &__gs_ss},
	{.she_type = kSWAPI_MSGTYPE_DEFAULT, .she_node = { }, 
		.she_cbfunc = swapi_shell_default_handler, .she_data = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static int swapi_shell_timer_handler(swapi_message_t *msg, void *data){
	swapi_shell_t	*sh = (swapi_shell_t *)data;

	int				i;

	ASSERT((msg != NULL) && (data != NULL));

	// FIXME: update system status value

	// paint backgroud color
	cairo_set_source_rgb(sh->ss_context, 1, 1, 1);
	cairo_paint(sh->ss_context);
//	cairo_rectangle(sh->ss_context, 0, 0, sh->ss_width, sh->ss_height);
//	cairo_fill(sh->ss_context);

	// draw battery
	cairo_set_source_rgb(sh->ss_context, 0.2, 0.2, 0.2);
	cairo_rectangle(sh->ss_context, 2, 1, 20, sh->ss_height - 2);
	cairo_rectangle(sh->ss_context, 22, 3, 2, 4);
	cairo_fill(sh->ss_context);

	// draw signal
	sh->ss_signals = 60;
	cairo_set_line_width(sh->ss_context, 1.0);
	for(i = 0; i < kSWAPI_SHELL_SIGNAL_ARC_NUM; i++){
		if(sh->ss_signals <= kSWAPI_SHELL_SIGNAL_ARC_NUM*i*4){
			cairo_arc(sh->ss_context, 80+i*10, 5, 3, 0, 2*3.14);
			cairo_stroke(sh->ss_context);
		}else{
			cairo_arc(sh->ss_context, 80+i*10, 5, 4, 0, 2*3.14);
			cairo_fill(sh->ss_context);
		}
	}
//	cairo_fill_preserve(sh->ss_context);
	
	swapi_log_warn("shell timer handler\n");

	swapi_render_flush(kSWAPI_RENDER_SHELL_UPDATE);

	return 0;
}

static int swapi_shell_default_handler(swapi_message_t *msg, void *data){
	return 0;
}

static int swapi_shell_thread_routine(void *p){
	swapi_shell_t	*sh = (swapi_shell_t *)p;
	swapi_message_t	msg;

	ASSERT(sh != NULL);

	while(sh->ss_status){
		if(swapi_queue_wait(sh->ss_queue, &msg) != 0){
			swapi_log_warn("swap wait message fail!\n");
			break;
		}

		// FIXME: check msg == destroy

		swapi_log_info("shell got message\n");

		swapi_handler_invoke(sh->ss_handler, &msg);
	}
	
	swapi_handler_destroy(sh->ss_handler);
	swapi_queue_destroy(sh->ss_queue);
	cairo_destroy(sh->ss_context);
	cairo_surface_destroy(sh->ss_surface);

	return 0;
}


int swapi_shell_module_init(){
	native_graphic_info_t	ngi;
	swapi_shell_t			*sh = get_shell();
	swapi_handler_entry_t	*she;
	
	native_graphic_getinfo(&ngi);
	sh->ss_width = ngi.ngi_width;
	sh->ss_height = kSWAPI_SHELL_HEIGHT;

	sh->ss_surface = cairo_image_surface_create(ngi.ngi_rgbtype, sh->ss_width,
			sh->ss_height);
	if(sh->ss_surface == NULL){
		swapi_log_warn("shell create shell surface fail!\n");
		return -1;
	}
	sh->ss_context = cairo_create(sh->ss_surface);
	if(sh->ss_context == NULL){
		swapi_log_warn("shell create cairo context fail!\n");
		goto exit_cairo;
	}

	if(swapi_queue_create(sizeof(swapi_message_t), kSWAPI_QUEUE_DEFAULT_LENGTH,
				&sh->ss_queue) != 0){
		swapi_log_warn("shell create queue fail!\n");
		goto exit_queue;
	}

	if(swapi_handler_create(kSWAPI_HANDLER_DEFAULT_SLOTS, &sh->ss_handler) != 0){
		swapi_log_warn("shell create handler fail!\n");
		goto exit_handler;
	}

	// add message handler
	she = get_handler();
	ASSERT(she != NULL);
	while(she->she_cbfunc != swapi_shell_default_handler){
		INIT_LIST_HEAD(&she->she_node);	
		swapi_handler_add(sh->ss_handler, she->she_type, she);
		
		she++;
	}

	INIT_LIST_HEAD(&sh->ss_swaps);
	
	if(swapi_thread_create(&sh->ss_thread, swapi_shell_thread_routine, sh) != 0){
		swapi_log_warn("swap create swap thread fail!\n");
		goto exit_thread;
	}

	sh->ss_status = 1;

	return 0;

exit_thread:
	swapi_handler_destroy(sh->ss_handler);

exit_handler:
	swapi_queue_destroy(sh->ss_queue);

exit_queue:
	cairo_destroy(sh->ss_context);

exit_cairo:
	cairo_surface_destroy(sh->ss_surface);

	return -1;
}

int swapi_shell_module_fini(){
	swapi_shell_t	*sh = get_shell();

	sh->ss_status = 0;

	// FIXME: send destroy message to queue

	swapi_thread_destroy(sh->ss_thread);

	return 0;
}

int swapi_shell_post(swapi_message_t *msg){
	return swapi_queue_post(get_shell()->ss_queue, msg);
}

cairo_surface_t *swapi_shell_get_surface(){
	return get_shell()->ss_surface;
}

