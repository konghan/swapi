/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __NATV_LOGGER_H__
#define __NATV_LOGGER_H__


#ifdef __cplusplus
extern "C" {
#endif

int natv_logger_output(const char *log, int size);

int natv_logger_init();
int natv_logger_fini();

#ifdef __cplusplus
}
#endif

#endif //__NATV_LOGGER_H__
