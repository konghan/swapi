/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "swap_oral.h"
#include "oral_view.h"

#include "swapi_view.h"
#include "swapi_swap.h"

#include "swapi_sys_thread.h"
#include "swapi_sys_logger.h"

/*
 * swap:oral data
 */
typedef struct swap_oral {
	int						so_init;

	swapi_swap_t			*so_swap;

	oral_view_user_t		so_user;
	oral_view_vmsg_t		so_vmsg;
	oral_view_text_t		so_text;
	oral_view_rcrd_t		so_rcrd;

}swap_oral_t;

static swap_oral_t		__gs_oral = {};

static inline swap_oral_t *get_oral(){
	return &__gs_oral;
}

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

static inline swapi_swap_cbs_t *get_oral_cbs(){
	return &__gs_swap_oral_cbs;
}

/*
 * swap message handlers
 */
static int oral_on_swapdata(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_oh_entry[] = {
	{ .she_type = kSWAPI_MSGTYPE_SWAP_DATA, .she_cbfunc = oral_on_swapdata },
	{ .she_type = kSWAPI_MSGTYPE_DEFAULT,   .she_cbfunc = NULL},
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_oh_entry;
}

static int oral_on_swapdata(swapi_message_t *msg, void *data){
	return 0;
}

static int oral_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	swap_oral_t				*so  = get_oral();
	swapi_window_t			*win;

	swapi_log_info("oral swap on create\n");

	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		she->she_data = so;
		swapi_swap_add_handler(sw, she->she_type, she);

		she++;
	}

	win = swapi_swap_get_window(sw);
	ASSERT(win != NULL);

	oral_view_user_init(&so->so_user, win);
	swapi_window_set_view(win, &so->so_user.ou_view);

	return 0;
}

static int oral_on_destroy(swapi_swap_t *swa){
	swap_oral_t				*so  = get_oral();
	
	swapi_log_info("oral swap on destroy\n");
	oral_view_user_fini(&so->so_user);

	return 0;
}

static int oral_on_pause(swapi_swap_t *swa){
	swapi_log_info("oral swap on pause\n");
	return 0;
}

static int oral_on_resume(swapi_swap_t *swa){
	swapi_log_info("oral swap on resume\n");
	return 0;
}

int swap_oral_init(swapi_swap_t **swap){
	swap_oral_t	*so;

	ASSERT(swap != NULL);

	swap_user_init();
	
	so = get_oral();

	if(swapi_swap_create("oral", get_oral_cbs(), &so->so_swap) != 0){
		swapi_log_warn("create oral swap fail!\n");
		return -1;
	}

	so->so_init = 1;

	swapi_swap_set(so->so_swap, so);

	*swap = so->so_swap;

	swapi_swap_kick(so->so_swap);

	return 0;
}

int swap_oral_fini(){
	swap_oral_t	*so;

	swap_user_fini();
	
	so = get_oral();

	so->so_init = 0;
	swapi_swap_destroy(so->so_swap);

	return 0;
}

