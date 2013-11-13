/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the GNU v3 license, see the LICENSE file.
 */

/*
 * os configurations module
 * all configurations should be load here
 */

#ifndef __NATV_CFGSRV_H__
#define __NATV_CFGSRV_H__

int natv_cfgsrv_get(const char *segment, const char *key,
					void *val, int *vsize, int *type);

int natv_cfgsrv_module_init();
int natv_cfgsrv_module_fini();

#endif // __NATV_CFGSRV_H__

