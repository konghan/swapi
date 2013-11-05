/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swap_clock.h"

#include "swapi_view.h"
#include "swapi_swap.h"
#include "swapi_render.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"

#include <cairo/cairo.h>

typedef struct swap_oral {
	swapi_spinlock_t	so_lock;
	int					so_init;

	swapi_swap_t		*so_swap;

}swap_oral_t;

/*
 * swap framework callbacks
 */
static int oral_on_create(swapi_swap_t *swa, int argc, char *argv[]);
static int oral_on_destroy(swapi_swap_t *swa);

static int oral_on_pause(swapi_swap_t *swa);
static int oral_on_resume(swapi_swap_t *swa);

static swapi_swap_cbs_t		__gs_swap_oral_cbs = {
	.on_create		= oral_on_create,
	.on_destroy		= oral_on_destroy,
	.on_pause		= oral_on_pause,
	.on_resume		= oral_on_resume,
};

static swap_oral_t		__gs_swap = {};

static inline swap_oral_t *get_swap(){
	return &__gs_swap;
}

static inline swapi_swap_cbs_t *get_cbs(){
	return &__gs_swap_oral_cbs;
}

/*
 * clock handlers
 */

static int oral_on_timer(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{.she_type = kSWAPI_MSGTYPE_TIMER, .she_node = { }, 
		.she_cbfunc = oral_on_timer, .she_data = &__gs_swap_clock},
	{.she_type = kSWAPI_MSGTYPE_DEFAULT, .she_node = { }, 
		.she_cbfunc = NULL, .she_data = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static int oral_on_timer(swapi_message_t *msg, void *data){
	swap_oral_t		*sc = (swap_oral_t *)data;
	swapi_view_t		*sv;
	cairo_surface_t		*surface;
	cairo_t				*context;

	ASSERT((msg != NULL) && (data != NULL));

	swapi_log_info("clock get timer message =======\n");

	sv = swapi_swap_topview(sc->sc_swap);
	if(sv == NULL){
		swapi_log_warn("clock swap without view!\n");
		return -1;
	}

	surface = swapi_view_get_surface(sv);
	context = swapi_view_get_cairo(sv);
	
	if((surface == NULL) || (context == NULL)){
		swapi_log_warn("clock view without cairo!\n");
		return -1;
	}
	
	// draw back ground
	cairo_set_source_rgb(context, 0.8, 0.8, 0.8);
	cairo_paint(context);

	// draw clock face
	cairo_set_source_rgb(context, 0.1, 0.1, 0.1);
	cairo_arc(context, 64, 59, 56, 0, 2*3.14);
	cairo_stroke(context);

	// draw hour/minute
	cairo_set_line_width(context, 3.0);
	cairo_move_to(context, 64, 59);
	cairo_line_to(context, 100, 59);
	cairo_stroke(context);
	
	cairo_set_line_width(context, 2.0);
	cairo_move_to(context, 64, 59);
	cairo_line_to(context, 14, 59);
	cairo_stroke(context);

	swapi_render_flush(kSWAPI_RENDER_SWAP_UPDATE);

	return 0;
}

static int oral_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	swapi_view_t			*vw;

	swapi_log_info("clock app on create\n");

	vw = swapi_swap_topview(sw);

	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		swapi_view_add_handler(vw, she->she_type, she);
		she++;
	}

	return 0;
}

static int oral_on_destroy(swapi_swap_t *swa){
	swapi_log_info("clock app on destroy\n");
	return 0;
}

static int oral_on_pause(swapi_swap_t *swa){
	swapi_log_info("clock app on pause\n");
	return 0;
}

static int oral_on_resume(swapi_swap_t *swa){
	swapi_log_info("clock app on resume\n");
	return 0;
}

int swap_oral_init(swapi_swap_t **swap){
	swap_oral_t	*sc;

	ASSERT(swap != NULL);
	
	sc = get_clock();

	swapi_spin_init(&sc->sc_lock);
	if(swapi_swap_create("clock", get_cbs(), &sc->sc_swap) != 0){
		swapi_log_warn("create swap clock fail!\n");
		return -1;
	}

	sc->sc_init = 1;

	*swap = sc->sc_swap;

	return 0;
}

int swap_oral_fini(){
	swap_oral_t	*sc;
	
	sc = get_clock();

	swapi_spin_lock(&sc->sc_lock);
	sc->sc_init = 0;
	swapi_spin_unlock(&sc->sc_lock);

	swapi_spin_fini(&sc->sc_lock);
	swapi_swap_destroy(sc->sc_swap);

	return 0;
}


/*
 * voice message data
 */
typedef struct oral_data{
	uuid_t			od_uid;
	uint64_t		od_time;
	int				od_idx;

	int				od_size;
	char			od_data[];
}oral_data_t;

static int oral_data_init(){
}

static int oral_data_fini(){
}


