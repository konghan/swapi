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
	swapi_spinlock_t	so_lock;
	int					so_init;

	swapi_swap_t		*so_swap;

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

static inline swapi_swap_cbs_t *get_cbs(){
	return &__gs_swap_oral_cbs;
}

/*
 * swap message handlers
 */
static int oral_on_swapdata(swapi_message_t *msg, void *data);

static swapi_handler_entry_t	__gs_handler_entry[] = {
	{ .she_type = kSWAPI_MSGTYPE_SWAP_DATA, .she_cbfunc = oral_on_swapdata },
	{ .she_type = kSWAPI_MSGTYPE_DEFAULT,   .she_cbfunc = NULL}
};

static inline swapi_handler_entry_t *get_handler(){
	return __gs_handler_entry;
}

static int oral_on_swapdata(swapi_message_t *msg, void *data){
	return 0;
}

static int oral_recording_draw(swapi_view_t *sv, oral_vmsg_t *ov){
	return -1;
}

static int oral_message_draw(swapi_view_t *sv, const char *utf8, int len){
	return -1;
}

static int oral_vmsg_draw(swapi_view_t *sv, oral_vmsg_t *ov){
	return -1;
}

static int oral_user_draw(swapi_view_t *sv, oral_user_t *ou){
	swapi_canvas_t *cvs;

	ASSERT(sv != NULL);

	cvs = 
}

static int oral_on_create(swapi_swap_t *sw, int argc, char *argv[]){
	swapi_handler_entry_t	*she = get_handler();
	swap_oral_t				*so  = get_oral();
	swapi_window_t			*win;
	swapi_view_t			*vw;

	swapi_log_info("oral swap on create\n");

	while(she->she_type != kSWAPI_MSGTYPE_DEFAULT){
		INIT_LIST_HEAD(&she->she_node);
		she->she_data = so;
		swapi_swap_add_handler(sw, she->she_type, she);
		she++;
	}

	win = swapi_swap_top_window(sw);
	ASSERT(win != NULL);

	vw = swapi_window_get_view(win);

	oral_user_draw(vw, NULL);

	return 0;
}

static int oral_on_destroy(swapi_swap_t *swa){
	swapi_log_info("oral swap on destroy\n");
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
	
	so = get_oral();

	swapi_spin_init(&so->so_lock);
	if(swapi_swap_create("oral", get_cbs(), &so->so_swap) != 0){
		swapi_log_warn("create oral swap fail!\n");
		return -1;
	}

	so->so_init = 1;

	*swap = so->so_swap;

	return 0;
}

int swap_oral_fini(){
	swap_oral_t	*so;
	
	so = get_oral();

	swapi_spin_lock(&so->so_lock);
	so->so_init = 0;
	swapi_spin_unlock(&so->so_lock);

	swapi_spin_fini(&so->so_lock);
	swapi_swap_destroy(so->so_swap);

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


