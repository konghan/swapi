/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SHELL_H__
#define __SWAPI_SHELL_H__

#include "swapi_shell.h"

typedef struct swapi_shell_trayicon{
	struct list_head		sst_node;
	
	int						sst_width;
	int						sst_height;

	int						sst_icon_size;
	char					*sst_data;
}swapi_shell_trayicon_t;

typedef struct swapi_shell{
	int		ss_battery;
	int		ss_signals;
	int		ss_message;
	int		ss_tips;
	int		ss_alarm;
	
	struct list_head	ss_swaps;
	
	cairo_surface_t		*ss_surface;
	cairo_t				*ss_context;
	int					ss_width;
	int					ss_height;

	swapi_queue_t		*ss_queue;
	swapi_handler_t		*ss_handlers;
}swapi_shell_t;

static swapi_shell_t	__gs_ss = {};
static swapi_shell_t *get_shell(){
	return &__gs_ss;
}

int swapi_shell_init(){
	return -1;
}

int swapi_shell_fini(){
	return -1;
}

int swapi_shell_post(swapi_message_t *msg){
	return swapi_queue_post(get_shell()->ss_queue, msg);
}

cairo_surface_t *swapi_shell_get_surface(){
	return get_shell()->ss_surface;
}

