/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_SYS_LOGGER_H__
#define __SWAPI_SYS_LOGGER_H__

#ifdef __cplusplus
extern "C"{
#endif

#define SWAPI_LOGGER_MAX_BUF	    128

enum{
    SWAPI_LOGGER_FATAL = 0,
    SWAPI_LOGGER_ERROR,
    SWAPI_LOGGER_WARN,
    SWAPI_LOGGER_INFO,
    SWAPI_LOGGER_DEBUG,
    SWAPI_LOGGER_TRACE,
    SWAPI_LOGGER_UNKOWN,
};


int swapi_logger_print(int level, const char *file, const int line, char *fmt, ...);

#define swapi_log_fatal(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_FATAL,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

#define swapi_log_error(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_ERROR,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

#define swapi_log_warn(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_WARN,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

#define swapi_log_info(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_INFO,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

#define swapi_log_debug(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_DEBUG,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

#define swapi_log_trace(__fmt,...)	\
    swapi_logger_print(SWAPI_LOGGER_TRACE,__FILE__,__LINE__,__fmt,##__VA_ARGS__)

int swapi_log_module_init();
int swapi_log_module_fini();

#ifdef __cplusplus
}
#endif

#endif // __SWAPI_SYS_LOGGER_H__


