/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

#ifndef __SWAP_ALARM_H__
#define __SWAP_ALARM_H__

#include "swapi_swap.h"

#ifdef __cplusplus
extern "C" {
#endif

int swap_alarm_init(swapi_swap_t **swap);
int swap_alarm_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAP_ALARM_H__

