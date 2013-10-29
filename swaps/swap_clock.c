/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#include "swap_clock.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"

typedef struct swap_clock {
	swapi_spinlock_t	sc_lock;
	int					sc_init;

	swapi_swap_t		*sc_swap;

}swap_clock_t;

static int clock_on_create(swapi_swap_t *swa, int argc, char *argv[]);
static int clock_on_destroy(swapi_swap_t *swa);

static int clock_on_start(swapi_swap_t *swa);
static int clock_on_stop(swapi_swap_t *swa);

static int clock_on_pause(swapi_swap_t *swa);
static int clock_on_resume(swapi_swap_t *swa);

static swapi_swap_cbs_t		__gs_swap_clock_cbs = {
	clock_on_create,
	clock_on_destroy,
	clock_on_start,
	clock_on_stop,
	clock_on_pause,
	clock_on_resume,

	/*
	.on_create		= clock_on_create,
	.on_destroy		= clock_on_destroy,
	.on_start		= clock_on_start,
	.on_stop		= clock_on_stop,
	.on_pause		= clock_on_pause,
	.on_resume		= clock_on_resume,
	*/
};

static swap_clock_t		__gs_swap_clock; // = {};

static inline swap_clock_t *get_clock(){
	return &__gs_swap_clock;
}

static inline swapi_swap_cbs_t *get_cbs(){
	return &__gs_swap_clock_cbs;
}

static int clock_on_create(swapi_swap_t *swa, int argc, char *argv[]){
	swapi_log_info("clock app on create\n");
	return 0;
}

static int clock_on_destroy(swapi_swap_t *swa){
	swapi_log_info("clock app on destroy\n");
	return 0;
}

static int clock_on_start(swapi_swap_t *swa){
	swapi_log_info("clock app on start\n");
	return 0;
}

static int clock_on_stop(swapi_swap_t *swa){
	swapi_log_info("clock app on stop\n");
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

int swap_clock_init(){
	swap_clock_t	*sc;
	
	sc = get_clock();

	swapi_spin_init(&sc->sc_lock);
	if(swapi_swap_create("clock", get_cbs(), &sc->sc_swap) != 0){
		swapi_log_warn("create swap clock fail!\n");
		return -1;
	}

	sc->sc_init = 1;

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

