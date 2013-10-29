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
	kSWAPI_MSGTYPE_DEFAULT  = 0,
	kSWAPI_MSGTYPE_KEYBOARD,
	kSWAPI_MSGTYPE_TIMER,
	kSWAPI_MSGTYPE_GSENSOR,
	kSWAPI_MSGTYPE_GPS,
	kSWAPI_MSGTYPE_PHONE_CALL,
	kSWAPI_MSGTYPE_PHONE_MSG,
	kSWAPI_MSGTYPE_SWAP,

	kSWAPI_MSGTYPE_APP_DATA,
	kSWAPI_MSGTYPE_APP_PRIVATE,
	kSWAPI_MSGTYPE_END,
};

/*
 * used by swap & loop, for swap's four status.
 * this type is used in sm_data
 */
enum swapi_swap_msgtype{
	kSWAP_MSGTYPE_CREATE = 0,
	kSWAP_MSGTYPE_DESTROY,
	
	kSWAP_MSGTYPE_PAUSE,
	kSWAP_MSGTYPE_RESUME,
};

typedef struct swapi_message{
	unsigned short		sm_type;
	unsigned short		sm_size;

	union {
		void			*sm_ptr;
		unsigned long	sm_data;
	};
}swapi_message_t;

int swapi_message_module_init();
int swapi_message_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__SWAPI_MESSAGE_H__
