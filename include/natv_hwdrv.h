/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

#ifndef __NATV_HWDRV_H__
#define __NATV_HWDRV_H__

/*
 * battery
 */
enum {
	kNATV_BATTERY_CAPACITY = 0,
	kNATV_BATTERY_STATUS,
};

int natv_battery_get(const int type);

/*
 * signal
 */
int natv_signal_get();

int natv_hwdrv_module_init();
int natv_hwdrv_module_fini();

#endif // __NATV_HWDRV_H__

