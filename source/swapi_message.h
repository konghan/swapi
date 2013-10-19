/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __SWAPI_MESSAGE_H__
#define __SWAPI_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif
	
enum {
	SWAPI_MSGTYPE_KEYBOARD = 0,
	SWAPI_MSGTYPE_TIMER,
	SWAPI_MSGTYPE_GSENSOR,
	SWAPI_MSGTYPE_GPS,
	SWAPI_MSGTYPE_PHONE_CALL,
	SWAPI_MSGTYPE_PHONE_MSG,

	SWAPI_MSGTYPE_APP_DATA,
	SWAPI_MSGTYPE_APP_PRIVATE,
	SWAPI_MSGTYPE_END,
};

typedef struct swapi_message{
	unsigned short		sm_type;
	unsigned short		sm_size;

	union {
		void			*sm_ptr;
		unsigned long	sm_data;
	}mq_u;
}swapi_message_t;

int swapi_message_module_init();
int swapi_message_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_MESSAGE_H__
