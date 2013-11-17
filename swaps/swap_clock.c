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

#include "native_graphic.h"

#include "list.h"

#include <math.h>
#include <cairo/cairo.h>

#define kSWAP_CLOCK_PKG_DIR			"../rootfs/swaps/clock"
#define kSWAP_CLOCK_FACE			"../rootfs/swaps/clock/clock06.png"
#define kMATH_PI					3.1415926

/*
 * clock face relative decl.
 */
struct clock_face;
typedef void (*clock_face_init)(struct clock_face *cf, int width, int height, int rgb);
typedef void (*clock_face_fini)(struct clock_face *cf);
typedef void (*clock_face_draw)(struct clock_face *cf, cairo_t *cr);

enum {
	kCLOCK_FACE_ANALOG = 0,
	kCLOCK_FACE_DIGITAL,
	kCLOCK_FACE_PHOTO,
	kCLOCK_FACE_END
};

typedef struct clock_face{
	int					cf_id;
	struct list_head	cf_node;
	cairo_surface_t		*cf_surface;

	clock_face_init		cf_init;
	clock_face_fini		cf_fini;
	clock_face_draw		cf_draw;

}clock_face_t;

static void clock_face_analog_init(clock_face_t *cf, int width, int height, int rgb);
static void clock_face_analog_fini(clock_face_t *cf);
static void clock_face_analog_draw(clock_face_t *cf, cairo_t *cr);

static void clock_face_digital_init(clock_face_t *cf, int width, int height, int rgb);
static void clock_face_digital_fini(clock_face_t *cf);
static void clock_face_digital_draw(clock_face_t *cf, cairo_t *cr);

static void clock_face_photo_init(clock_face_t *cf, int width, int height, int rgb);
static void clock_face_photo_fini(clock_face_t *cf);
static void clock_face_photo_draw(clock_face_t *cf, cairo_t *cr);

static clock_face_t	__gs_faces[] = {
	{.cf_id = kCLOCK_FACE_ANALOG, .cf_node = {},  .cf_surface = NULL,
		.cf_init = clock_face_analog_init, .cf_fini = clock_face_analog_fini,
		.cf_draw = clock_face_analog_draw },

	{.cf_id = kCLOCK_FACE_DIGITAL, .cf_node = {},  .cf_surface = NULL,
		.cf_init = clock_face_analog_init, .cf_fini = clock_face_analog_fini,
		.cf_draw = clock_face_analog_draw },

	{.cf_id = kCLOCK_FACE_PHOTO, .cf_node = {},  .cf_surface = NULL,
		.cf_init = clock_face_analog_init, .cf_fini = clock_face_analog_fini,
		.cf_draw = clock_face_analog_draw },

	{.cf_id = kCLOCK_FACE_END, .cf_node = {},  .cf_surface = NULL }
};

static inline clock_face_t *get_face(){
	return __gs_faces;
}

/*
 * clock swap container
 */
typedef struct swap_clock {
	swapi_spinlock_t	sc_lock;
	int					sc_init;

	struct list_head	sc_faces;

	clock_face_t		*sc_cur;

	swapi_swap_t		*sc_swap;

}swap_clock_t;

static swap_clock_t		__gs_swap_clock = {};

static inline swap_clock_t *get_clock(){
	return &__gs_swap_clock;
}

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

static inline swapi_swap_cbs_t *get_cbs(){
	return &__gs_swap_clock_cbs;
}

/*
 * clock message queue handlers
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

/*
 * implementation of main body
 */
static void clock_draw(swap_clock_t *sc){
	cairo_t				*cr;
	swapi_view_t		*sv;

	ASSERT((sc != NULL) && (sc->sc_cur != NULL) && (sc->sc_swap != NULL));

	sv = swapi_swap_topview(sc->sc_swap);
	if(sv == NULL){
		swapi_log_warn("clock swap without view!\n");
		return ;
	}

	cr = swapi_view_get_cairo(sv);
	sc->sc_cur->cf_draw(sc->sc_cur, cr);

	swapi_render_flush(kSWAPI_RENDER_SWAP_UPDATE);
}

static int clock_on_timer(swapi_message_t *msg, void *data){
	swap_clock_t	*sc = (swap_clock_t *)data;

	ASSERT((msg != NULL) && (sc != NULL));

	swapi_log_info("clock receive timer message!\n");
	clock_draw(sc);

	return 0;
}

static int clock_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	clock_face_t			*cf = get_face();

	swap_clock_t			*sc;
	native_graphic_info_t	ngi;

	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}

	// init swap message callbacks
	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		swapi_swap_add_handler(sw, she->she_type, she);
		she++;
	}

	// init clock face
	native_graphic_getinfo(&ngi);
	while(cf->cf_id != kCLOCK_FACE_END){
		INIT_LIST_HEAD(&cf->cf_node);

		cf->cf_init(cf, ngi.ngi_width, ngi.ngi_height, ngi.ngi_rgbtype);

		swapi_spin_lock(&sc->sc_lock);
		list_add_tail(&cf->cf_node, &sc->sc_faces);
		swapi_spin_unlock(&sc->sc_lock);

		cf++;
	}

	sc->sc_cur = get_face();

	return 0;
}

static int clock_on_destroy(swapi_swap_t *sw){
	clock_face_t			*pos, *temp;
	swap_clock_t			*sc;

	ASSERT(sw != NULL);

	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}

	list_for_each_entry_safe(pos, temp, &sc->sc_faces, cf_node){
		swapi_spin_lock(&sc->sc_lock);
		list_del(&pos->cf_node);
		swapi_spin_unlock(&sc->sc_lock);

		pos->cf_fini(pos);
	}

	return 0;
}

static int clock_on_pause(swapi_swap_t *sw){
	swapi_log_info("clock app on pause\n");
	return 0;
}

static int clock_on_resume(swapi_swap_t *sw){
	swap_clock_t			*sc;

	ASSERT(sw != NULL);

	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}

	clock_draw(sc);
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
	INIT_LIST_HEAD(&sc->sc_faces);
	sc->sc_cur = NULL;
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

/*
 * clock face implementations
 */
static void clock_face_analog_init(clock_face_t *cf, int width, int height, int rgb){
	cairo_t		*cr;
	double		rclock, rradio, len;
	double		cx, cy, rrx, rry, rbx, rby;

	ASSERT(cf != NULL);

	cf->cf_surface = cairo_image_surface_create(rgb, width, height);
	if(cf->cf_surface == NULL){
		swapi_log_warn("analog face create surface fail!\n");
		return ;
	}
	
	cr = cairo_create(cf->cf_surface);
	if(cf == NULL){
		swapi_log_warn("analog face create cairo context fail!\n");
		return ;
	}

	// draw background
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	// draw clock face
	rclock = ((width >= height) ? height : width) / 2 - 10;
	cx = width / 2;
	cy = height / 2 + 10;
	
	rradio = (sqrt(width*width + height*height) / 2 - rclock) / 2;
	rrx = rry = rradio;

	rbx = width - rradio;
	rby = rry;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 1.0);

	cairo_arc(cr, cx, cy, rclock, 0, 2*kMATH_PI);
	cairo_stroke(cr);

	cairo_arc(cr, cx, cy, 2, 0, 2*kMATH_PI);
	cairo_fill(cr);

	cairo_arc(cr, rrx, rry, rradio, 0, 2*kMATH_PI);
	cairo_stroke(cr);

	cairo_arc(cr, rbx, rby, rradio, 0, 2*kMATH_PI);
	cairo_stroke(cr);

	cairo_destroy(cr);
}

static void clock_face_analog_fini(clock_face_t *cf){
	ASSERT(cf != NULL);
	
	cairo_surface_destroy(cf->cf_surface);
}

static void clock_face_analog_draw(clock_face_t *cf, cairo_t *cr){
	ASSERT((cf != NULL) && (cr != NULL));

	cairo_set_source_surface(cr, cf->cf_surface, 0, 0);
	cairo_paint(cr);
}

static void clock_face_digital_init(clock_face_t *cf, int width, int height, int rgb){
}

static void clock_face_digital_fini(clock_face_t *cf){
}

static void clock_face_digital_draw(clock_face_t *cf, cairo_t *cr){
}

static void clock_face_photo_init(clock_face_t *cf, int width, int height, int rgb){
}

static void clock_face_photo_fini(clock_face_t *cf){
}

static void clock_face_photo_draw(clock_face_t *cf, cairo_t *cr){
}

