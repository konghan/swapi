/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __NATIVE_LOGGER_H__
#define __NATIVE_LOGGER_H__


#ifdef __cplusplus
extern "C" {
#endif

int native_logger_output(const char *log, int size);

int native_logger_init();
int native_logger_fini();

#ifdef __cplusplus
}
#endif

#endif //__NATIVE_LOGGER_H__
