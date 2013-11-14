/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swap_clock.h"

#include "swapi_view.h"
#include "swapi_swap.h"
#include "swapi_render.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"

#include <cairo/cairo.h>

#define kSWAP_CLOCK_PKG_DIR			"../rootfs/swaps/clock"
#define kSWAP_CLOCK_FACE			"../rootfs/swaps/clock/clock06.png"

typedef struct swap_clock {
	swapi_spinlock_t	sc_lock;
	int					sc_init;

	cairo_surface_t		*sc_face;
	swapi_swap_t		*sc_swap;

}swap_clock_t;

/*
 * clock swap framework callbacks
 */
static int clock_on_create(swapi_swap_t *swa, int argc, char *argv[]);
static int clock_on_destroy(swapi_swap_t *swa);

static int clock_on_pause(swapi_swap_t *swa);
static int clock_on_resume(swapi_swap_t *swa);

static swapi_swap_cbs_t		__gs_swap_clock_cbs = {
	.on_create		= clock_on_create,
	.on_destroy		= clock_on_destroy,
	.on_pause		= clock_on_pause,
	.on_resume		= clock_on_resume,
};

static swap_clock_t		__gs_swap_clock; // = {};

static inline swap_clock_t *get_clock(){
	return &__gs_swap_clock;
}

static inline swapi_swap_cbs_t *get_cbs(){
	return &__gs_swap_clock_cbs;
}

/*
 * clock handlers
 */

static int clock_on_timer(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{.she_type = kSWAPI_MSGTYPE_TIMER, .she_node = { }, 
		.she_cbfunc = clock_on_timer, .she_data = &__gs_swap_clock},
	{.she_type = kSWAPI_MSGTYPE_DEFAULT, .she_node = { }, 
		.she_cbfunc = NULL, .she_data = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static void clock_draw_hour(cairo_t *cr, int hour){

	cairo_set_source_rgb(cr, 1, 0, 0);
	cairo_set_line_width(cr, 3.0);
	
	cairo_move_to(cr, 64, 59);
	cairo_line_to(cr, 100, 59);

}

static void clock_draw_minute(cairo_t *cr, int minute){

	cairo_set_line_width(cr, 2.0);

	cairo_move_to(cr, 64, 59);
	cairo_line_to(cr, 14, 59);
}

static void clock_draw_second(cairo_t *cr, int second){
}

static int clock_on_timer(swapi_message_t *msg, void *data){
	swap_clock_t		*sc = (swap_clock_t *)data;
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

	if(sc->sc_face != NULL){
		cairo_set_source_surface(context, sc->sc_face, 0, 0);
		cairo_paint(context);
	}else{
		// draw back ground
		cairo_set_source_rgb(context, 0.8, 0.8, 0.8);
		cairo_paint(context);

		// draw clock face
		cairo_set_source_rgb(context, 0.1, 0.1, 0.1);
		cairo_arc(context, 64, 59, 56, 0, 2*3.14);
		cairo_stroke(context);
	}

	clock_draw_hour(context, 11);
	clock_draw_minute(context, 25);
	
	cairo_stroke(context);

	swapi_render_flush(kSWAPI_RENDER_SWAP_UPDATE);

	return 0;
}

static int clock_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	swapi_view_t			*vw;
	swap_clock_t			*sc;

	swapi_log_info("clock app on create\n");

	// init top view callbacks
	vw = swapi_swap_topview(sw);

	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		swapi_view_add_handler(vw, she->she_type, she);
		she++;
	}

	// load clock face
	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}
	sc->sc_face = cairo_image_surface_create_from_png(kSWAP_CLOCK_FACE);
	if(sc->sc_face == NULL){
		swapi_log_warn("swap clock load png face fail!\n");
		return -1;
	}

	return 0;
}

static int clock_on_destroy(swapi_swap_t *swa){
	swapi_log_info("clock app on destroy\n");
	return 0;
}

static int clock_on_pause(swapi_swap_t *swa){
	swapi_log_info("clock app on pause\n");
	return 0;
}

static int clock_on_resume(swapi_swap_t *swa){
	swapi_log_info("clock app on resume\n");
	return 0;
}

int swap_clock_init(swapi_swap_t **swap){
	swap_clock_t	*sc;

	ASSERT(swap != NULL);
	
	sc = get_clock();

	swapi_spin_init(&sc->sc_lock);
	if(swapi_swap_create("clock", get_cbs(), &sc->sc_swap) != 0){
		swapi_log_warn("create swap clock fail!\n");
		return -1;
	}

	sc->sc_init = 1;
	swapi_swap_set(sc->sc_swap, sc);

	*swap = sc->sc_swap;

	return 0;
}

int swap_clock_fini(){
	swap_clock_t	*sc;
	
	sc = get_clock();

	swapi_spin_lock(&sc->sc_lock);
	sc->sc_init = 0;
	swapi_spin_unlock(&sc->sc_lock);

	swapi_spin_fini(&sc->sc_lock);
	swapi_swap_destroy(sc->sc_swap);

	return 0;
}

