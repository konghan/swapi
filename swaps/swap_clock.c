/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swap_clock.h"

#include "swapi_swap.h"
#include "swapi_canvas.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"

#include "natv_time.h"
#include "natv_hwdrv.h"
#include "natv_surface.h"

#include "list.h"

#include <math.h>

#define kSWAP_CLOCK_PKG_DIR			"../rootfs/swaps/clock"
#define kSWAP_CLOCK_BKGD			"../rootfs/swaps/clock/clock-bkgd03.png"

typedef struct clock_analog_view{
	swapi_view_t		cav_view;
	swapi_canvas_t		cav_cvs;

	// clock radis, x,y
	double				cav_cr;
	double				cav_cx;
	double				cav_cy;

	// battery radis, x,y
	double				cav_br;
	double				cav_bx;
	double				cav_by;

	// signal radis, x,y
	double				cav_sr;
	double				cav_sx;
	double				cav_sy;
}clock_analog_view_t;

static int clock_analog_view_init(clock_analog_view_t *cav, swapi_window_t *win);
static void clock_analog_view_fini(clock_analog_view_t *cav);
static void clock_analog_view_draw(swapi_view_t *cav, swapi_canvas_t *cvs);

static void clock_digital_view_draw(swapi_view_t *view, swapi_canvas_t *cvs);

/*
 * clock swap container
 */
enum {
	kCLOCK_VIEW_ANALOG = 0,
	kCLOCK_VIEW_DIGITAL
};

typedef struct swap_clock {
	swapi_spinlock_t		sc_lock;
	int						sc_init;

	int						sc_vtype;

	clock_analog_view_t		sc_cav;
	swapi_view_t			sc_cdv;

	swapi_swap_t			*sc_swap;

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
	{.she_type = kSWAPI_MSGTYPE_TIMER, .she_cbfunc = clock_on_timer},
	{.she_type = kSWAPI_MSGTYPE_DEFAULT},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

/*
 * implementation of main body
 */
static void clock_draw(swap_clock_t *sc){
	swapi_window_t		*sw;

	ASSERT(sc != NULL);

	sw = swapi_swap_get_window(sc->sc_swap);
	if(sw == NULL){
		swapi_log_warn("clock swap without view!\n");
		return ;
	}

	swapi_window_draw(sw);
}

static int clock_on_timer(swapi_message_t *msg, void *data){
	swap_clock_t	*sc = (swap_clock_t *)data;

	ASSERT((msg != NULL) && (sc != NULL));

	swapi_log_info("clock receive timer message!\n");
	clock_draw(sc);

	return 0;
}

static void clock_set_view(swap_clock_t *sc, int vtype){
	ASSERT(sc != NULL);

	switch(vtype){
	case kCLOCK_VIEW_ANALOG:
		swapi_window_set_view(swapi_swap_get_window(sc->sc_swap), &sc->sc_cav.cav_view);
		break;

	case kCLOCK_VIEW_DIGITAL:
		swapi_window_set_view(swapi_swap_get_window(sc->sc_swap), &sc->sc_cdv);
		break;

	default:
		return;
	}

	sc->sc_vtype = vtype;

	clock_draw(sc);
}

static int clock_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	swap_clock_t			*sc;
	
	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}

	// init swap message callbacks
	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		she->she_data = sc;
		swapi_swap_add_handler(sw, she->she_type, she);

		she++;
	}

	if(clock_analog_view_init(&sc->sc_cav, swapi_swap_get_window(sw)) != 0){
		return -1;
	}

	if(swapi_view_init(&sc->sc_cdv, swapi_swap_get_window(sw), 0, 0,
				swapi_window_get_width(swapi_swap_get_window(sw)),
				swapi_window_get_height(swapi_swap_get_window(sw))) != 0){
		return -1;
	}

	sc->sc_cdv.on_draw = clock_digital_view_draw;

	clock_set_view(sc, kCLOCK_VIEW_DIGITAL);

	return 0;
}

static int clock_on_destroy(swapi_swap_t *sw){
	swap_clock_t			*sc;

	ASSERT(sw != NULL);

	sc = (swap_clock_t *)swapi_swap_get(sw);
	if(sc == NULL){
		swapi_log_warn("swap clock not setting!\n");
		return -1;
	}

	clock_analog_view_fini(&sc->sc_cav);
	swapi_view_fini(&sc->sc_cdv);

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

	swapi_swap_set(sc->sc_swap, sc);

	sc->sc_init = 1;
	*swap = sc->sc_swap;

	swapi_swap_kick(sc->sc_swap);

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
 * clock view implementations
 */
static int clock_analog_view_init(clock_analog_view_t *cav, swapi_window_t *win){
	float				rclock, rradio, len;
	float				cx, cy, rrx, rry, rbx, rby;
	float				xs, ys, xe, ye;
	int					i;

	int width = swapi_window_get_width(win);
	int height = swapi_window_get_height(win);

	ASSERT(cav != NULL);

	if(swapi_view_init(&cav->cav_view, win, 0, 0, swapi_window_get_width(win),
				swapi_window_get_height(win)) != 0){
		swapi_log_warn("analog init view fail!\n");
		return -1;
	}

	cav->cav_view.on_draw = clock_analog_view_draw;

	if(swapi_canvas_init_image(&cav->cav_cvs, swapi_window_get_width(win),
				swapi_window_get_height(win), swapi_window_get_format(win)) != 0){
		swapi_log_warn("analog init canvas fail!\n");
		return -1;
	}
	
	// draw background
	swapi_canvas_draw_color(&cav->cav_cvs, 255, 224, 224, 255);

	// draw clock face
	rclock = ((width >= height) ? height : width) / 2 - 10;
	cx = width / 2;
	cy = height / 2 + 10;
	
	cav->cav_cr = rclock;
	cav->cav_cx = cx;
	cav->cav_cy = cy - 1;

	rradio = (sqrt(width*width + height*height) / 2 - rclock) / 2;
	rrx = rry = rradio;

	cav->cav_sr = rradio;
	cav->cav_sx = rrx;
	cav->cav_sy = rry;

	rbx = width - rradio;
	rby = rry;
	
	cav->cav_br = rradio;
	cav->cav_bx = rbx;
	cav->cav_by = rby;
	
	// draw battery & signal
	swapi_canvas_set_color(&cav->cav_cvs, 100, 100, 100, 140);
	swapi_canvas_set_line(&cav->cav_cvs, 3);
	
	swapi_canvas_draw_arc(&cav->cav_cvs, rrx, rry, rradio - 2, 0, 2*kMATH_PI);
	swapi_canvas_stroke(&cav->cav_cvs);

	swapi_canvas_draw_arc(&cav->cav_cvs, rbx, rby, rradio - 2, 0, 2*kMATH_PI);
	swapi_canvas_stroke(&cav->cav_cvs);

	// draw red line
	swapi_canvas_set_color(&cav->cav_cvs, 255, 0, 0, 100);
	swapi_canvas_draw_arc(&cav->cav_cvs, rrx, rry, rradio - 2, -225*kMATH_PI/180, -165*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);
	swapi_canvas_draw_arc(&cav->cav_cvs, rbx, rby, rradio - 2, -15*kMATH_PI/180, 45*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);
	
	// draw yellow line
	swapi_canvas_set_color(&cav->cav_cvs, 255, 255, 0, 100);
	swapi_canvas_draw_arc(&cav->cav_cvs, rrx, rry, rradio - 2, -165*kMATH_PI/180, -105*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);
	swapi_canvas_draw_arc(&cav->cav_cvs, rbx, rby, rradio - 2, -75*kMATH_PI/180, -15*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);

	// draw green line
	swapi_canvas_set_color(&cav->cav_cvs, 0, 255, 0, 100);
	swapi_canvas_draw_arc(&cav->cav_cvs, rrx, rry, rradio - 2, -105*kMATH_PI/180, -45*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);
	swapi_canvas_draw_arc(&cav->cav_cvs, rbx, rby, rradio - 2, -135*kMATH_PI/180, -75*kMATH_PI/180);
	swapi_canvas_stroke(&cav->cav_cvs);

	// draw clock face
	swapi_canvas_set_color(&cav->cav_cvs, 25, 25, 26, 140);
	swapi_canvas_set_line(&cav->cav_cvs, 3);

	swapi_canvas_draw_arc(&cav->cav_cvs, cx, cy, rclock - 1, 0, 2*kMATH_PI);
	swapi_canvas_stroke(&cav->cav_cvs);

	len = rclock - 5;
	for(i = 0; i < 360; i+= 30){
		if((i % 90) == 0){
			swapi_canvas_set_line(&cav->cav_cvs, 3);
		}else{
			swapi_canvas_set_line(&cav->cav_cvs, 2);
		}

		xe = rclock * cos(i*kMATH_PI/180) + cx;
		ye = rclock * sin(i*kMATH_PI/180) + cy;

		xs = len * cos(i*kMATH_PI/180) + cx;
		ys = len * sin(i*kMATH_PI/180) + cy;

		swapi_canvas_draw_line(&cav->cav_cvs, xs, ys, xe, ye);
		swapi_canvas_stroke(&cav->cav_cvs);
	}

	return 0;
}

static void clock_analog_view_fini(clock_analog_view_t *cav){
	ASSERT(cav != NULL);
	
	swapi_view_fini(&cav->cav_view);
	swapi_canvas_fini(&cav->cav_cvs);
}


#define kCLOCK_ANALOG_HOURLEN		30
#define kCLOCK_ANALOG_MINUTELEN		40
#define kCLOCK_ANALOG_BATTERY		12
#define kCLOCK_ANALOG_SIGNAL		12
static void clock_analog_view_draw(swapi_view_t *sv, swapi_canvas_t *cvs){
	clock_analog_view_t		*cav;
	natv_tm_t				tm;
	float					xs, ys, xe, ye;
	float					arc;

	ASSERT(sv != NULL);

	cav = container_of(sv, clock_analog_view_t, cav_view);

	swapi_canvas_draw_canvas(cvs, 0, 0, &cav->cav_cvs);

	if(natv_time_localtime(&tm) != 0){
		swapi_log_warn("clock analog get localtime fail!\n");
		return ;
	}
	swapi_log_info("time : %d y %d m %d d %d h %d m\n", tm.tm_year, tm.tm_mon,
			tm.tm_mday, tm.tm_hour, tm.tm_min);

	// draw hour
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);

	arc = ((double)((tm.tm_hour) % 12) + ((double)tm.tm_min / 60))*30 - 90;
	xe = kCLOCK_ANALOG_HOURLEN*cos(arc*kMATH_PI/180) + cav->cav_cx;
	ye = kCLOCK_ANALOG_HOURLEN*sin(arc*kMATH_PI/180) + cav->cav_cy;
	xs = 8*cos((180 + arc)*kMATH_PI/180) + cav->cav_cx;
	ys = 8*sin((180 + arc)*kMATH_PI/180) + cav->cav_cy;
	
	swapi_canvas_set_line(cvs, 3);
	swapi_canvas_draw_line(cvs, xs, ys, xe, ye);
	swapi_canvas_stroke(cvs);

	// draw minute
	arc = tm.tm_min * 6 - 90;
	xe = kCLOCK_ANALOG_MINUTELEN*cos(arc*kMATH_PI/180) + cav->cav_cx;
	ye = kCLOCK_ANALOG_MINUTELEN*sin(arc*kMATH_PI/180) + cav->cav_cy;
	xs = 8*cos((180 + arc)*kMATH_PI/180) + cav->cav_cx;
	ys = 8*sin((180 + arc)*kMATH_PI/180) + cav->cav_cy;
	
	swapi_canvas_set_line(cvs, 2);
	swapi_canvas_draw_line(cvs, xs, ys, xe, ye);
	swapi_canvas_stroke(cvs);
	
	// draw center point
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_draw_arc(cvs, cav->cav_cx, cav->cav_cy, 2, 0, 2*kMATH_PI);
	swapi_canvas_fill(cvs);

	// draw battery
	arc = natv_battery_get(kNATV_BATTERY_CAPACITY);
	arc = (arc * 180)/100 - 135;
	xe = kCLOCK_ANALOG_BATTERY*cos(arc*kMATH_PI/180) + cav->cav_bx;
	ye = kCLOCK_ANALOG_BATTERY*sin(arc*kMATH_PI/180) + cav->cav_by;
	xs = cav->cav_bx;
	ys = cav->cav_by;
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_set_line(cvs, 2);
	swapi_canvas_draw_line(cvs, xs, ys, xe, ye);
	swapi_canvas_stroke(cvs);

	// draw battery center point
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_draw_arc(cvs, cav->cav_bx, cav->cav_by, 2, 0, 2*kMATH_PI);
	swapi_canvas_fill(cvs);

	// draw signal
	arc = natv_signal_get();
	arc = (arc * 180)/100 - 225;
	xe = kCLOCK_ANALOG_SIGNAL*cos(arc*kMATH_PI/180) + cav->cav_sx;
	ye = kCLOCK_ANALOG_SIGNAL*sin(arc*kMATH_PI/180) + cav->cav_sy;
	xs = cav->cav_sx;
	ys = cav->cav_sy;
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_set_line(cvs, 2);
	swapi_canvas_draw_line(cvs, xs, ys, xe, ye);
	swapi_canvas_stroke(cvs);

	// draw battery center point
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_draw_arc(cvs, cav->cav_sx, cav->cav_sy, 2, 0, 2*kMATH_PI);
	swapi_canvas_fill(cvs);
}

/*
 * digital view
 */

static const char *__gs_weekdays[] = {
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun",
	"\0"
};

static const char *clock_weekday(int i){
	if((i < 0 )||(i >6)) {
		return NULL;
	}

	return __gs_weekdays[i];
}

static const char *__gs_moths[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
	"\0",
};

static const char *clock_moth(int i){
	if((i < 0) || (i > 11)){
		return NULL;
	}

	return __gs_moths[i];
}

static void clock_digital_view_draw(swapi_view_t *sv, swapi_canvas_t *cvs){
	natv_tm_t		tm;

	char			buf[32];
	
	ASSERT((sv != NULL) && (cvs != NULL));

	// draw background
	swapi_canvas_draw_color(cvs, 100, 100, 100, 255);

	// draw frame
	swapi_canvas_set_line(cvs, 1);
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	
	swapi_canvas_draw_line(cvs, 0, 32, 128, 32);
	swapi_canvas_draw_line(cvs, 0, 96, 128, 96);

	swapi_canvas_stroke(cvs);

	if(natv_time_localtime(&tm) != 0){
		swapi_log_warn("clock analog get localtime fail!\n");
		return ;
	}

	// draw clock
	sprintf(buf, "%0d:%0d", tm.tm_hour, tm.tm_min);
	swapi_canvas_font_set_size(cvs, 40.0);
	swapi_canvas_set_color(cvs, 255, 0, 0, 255);
	swapi_canvas_draw_text(cvs, buf, 0, 5, 70);

	sprintf(buf, "%d %s, %s", tm.tm_mday, clock_moth(tm.tm_mon), clock_weekday(tm.tm_wday));
	swapi_canvas_font_set_size(cvs, 16);
	swapi_canvas_set_color(cvs, 0, 0, 0, 255);
	swapi_canvas_draw_text(cvs, buf, 0, 5, 90);


	swapi_canvas_stroke(cvs);
}

