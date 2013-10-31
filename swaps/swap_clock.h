/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAP_CLOCK_H__
#define __SWAP_CLOCK_H__

#include "swapi_swap.h"

#ifdef __cplusplus
extern "C" {
#endif

int swap_clock_init(swapi_swap_t **swap);
int swap_clock_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAP_CLOCK_H__
