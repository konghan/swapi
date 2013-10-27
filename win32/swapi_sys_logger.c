/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */
#include <stdio.h>
#include <stdarg.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>


#include "swapi_sys_logger.h"
#include "native_logger.h"

typedef struct swapi_logger{
    int		    log_init;
}swapi_logger_t;

struct swapi_logger_ltos {
    int	    ll_level;
    char    *ll_string;
};

static struct swapi_logger_ltos __log_ltos[] = {
    {SWAPI_LOGGER_FATAL,  "FATAL"},
    {SWAPI_LOGGER_ERROR,  "ERROR"},
    {SWAPI_LOGGER_WARN,   " WORN"},
    {SWAPI_LOGGER_INFO,   " INFO"},
    {SWAPI_LOGGER_DEBUG,  "DEBUG"},
    {SWAPI_LOGGER_TRACE,  "TRACE"},
    {SWAPI_LOGGER_UNKOWN, "UNKOWN"},
};
static swapi_logger_t	    __log_data;

static char *log_ltos(int level){
    if((level > SWAPI_LOGGER_TRACE)||(level < 0)){
		return (__log_ltos[SWAPI_LOGGER_UNKOWN]).ll_string;
    }

    return (__log_ltos[level]).ll_string;
}

int swapi_logger_print(int level, char *fmt, ...){
    swapi_logger_t	*log = &__log_data;
    char			buf[SWAPI_LOGGER_MAX_BUF];
    int				size = 0;
    va_list			args;

    if(!log->log_init){
		return -1;
    }

    size = _snprintf(buf, SWAPI_LOGGER_MAX_BUF, "%s:", log_ltos(level));

    va_start(args, fmt);
    size += _vsnprintf(buf+size, SWAPI_LOGGER_MAX_BUF-size, fmt, args);
    va_end(args);

	return native_logger_output(buf, size);
}

int swapi_log_module_init(){
    swapi_logger_t	*log = &__log_data;

    log->log_init = 1;

    return 0;
}

int swapi_log_module_fini(){
    swapi_logger_t	*log = &__log_data;

	log->log_init = 0;

    return 0;
}

