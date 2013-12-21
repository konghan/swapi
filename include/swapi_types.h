/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_TYPES_H__
#define __SWAPI_TYPES_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL		((void *)0)
#endif

typedef struct swapi_uuid {
	char su_uuid[16];
}swapi_uuid_t;

static inline int swapi_uuid_cmp(swapi_uuid_t *u1, swapi_uuid_t *u2){
	return memcmp(u1, u2, sizeof(*u1));
}

static inline int swapi_uuid_cpy(swapi_uuid_t *dest, swapi_uuid_t *src){
	memcpy(dest, src, sizeof(*src));
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_TYPES_H__

