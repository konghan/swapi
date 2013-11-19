/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */
#include "natv_hwdrv.h"

/*
 * battery
 */
int natv_battery_get(const int type){
	return 60;
}

/*
 * signal
 */
int natv_signal_get(){
	return 80;
}

int natv_hwdrv_module_init(){
	return 0;
}

int natv_hwdrv_module_fini(){
	return 0;
}


