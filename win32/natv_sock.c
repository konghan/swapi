/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */
#include <stdio.h>
#include <stdarg.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define kNATV_SOCK_ADDR_SIZE			20

#define kNATV_SOCK_SEGMENT				"natv-sock"
#define kNATV_SOCK_ADDR_KEY				"server-ip"
#define kNATV_SOCK_ADDR_PORT			"server-port"

typedef struct natv_sock{
    int					ns_init;
	int					ns_status;

	// should be mutex
	swapi_spinlock_t	ns_lock;
    SOCKET				ns_sock;

	swapi_thread_t		ns_thread;

    struct sockaddr_in	ns_serv;
}natv_sock_t;

static natv_sock_t	    __gs_sock = {};

static inline natv_sock_t *get_sock(){
	return &__gs_sock;
}

static int natv_sock_routine(void *p){
	natv_sock_t		*sock = (natv_sock_t *)p;

	ASSERT(sock != NULL);

	while(1){
		// FIXME: check gprs driver

		if(connect(sock->ns_sock, (struct sockaddr *)&sock->ns_serv,
			sizeof(struct sockaddr_in)) < 0){
			if(!sock->ns_status){
				break;
			}

			swapi_log_warn("connecting to server fail, try again ...\n");
			// wait 3 seconds
			sleep(3);
			continue;
		}
		sock->ns_init = 1;

		// read data & dispatch to loop
		while(sock->ns_status){
		}
    }


	return 0;
}

int natv_sock_send(struct swap_id *sid, struct swap_uid, size_t size, void *data){
}

int natv_sock_send_iov(struct swap_id *sid, struct swap_uid, int num, struct swap_iov *ios){
}

int natv_sock_module_init(){
    natv_sock_t		*sock = get_sock();
	char			serv[kNATV_SOCK_ADDR_SIZE];
	int				vsize = kNATV_SOCK_ADDR_SIZE;
	int				type, port;
	
    sock->ns_serv.sin_family = AF_INET;

	if(natv_cfgsrv_get(kNATV_SOCK_SEGMENT, kNATV_SOCK_ADDR_KEY,
				serv, &vsize, &type) != 0){
		swapi_log_warn("cfgsrv get sock configuration fail!\n");
		return -1;
	}
    sock->ns_serv.sin_addr.s_addr = inet_addr(serv);

	vsize =  sizeof(int);
	if(natv_cfgsrv_get(kNATV_SOCK_SEGMENT, kNATV_SOCK_ADDR_PORT,
				&port, &vsize, &type) != 0){
		swapi_log_warn("cfgsrv get sock configuration fail!\n");
		return -1;
	}
    natv->ns_serv.sin_port = htons(port);
    
	sock->ns_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock->ns_sock < 0){
		swapi_log_warn("create sock client fail!\n");
		return -1;
    }
    
	swapi_spin_init(&sock->ns_lock);

	sock->ns_status = 1;

	if(swapi_thread_create(&sock->ns_thread, natv_sock_routine, sock) != 0){
		swapi_log_warn("create sock thread fail!\n");
		closesocket(sock->ns_sock);
		return -1;
	}
	
	return 0;
}

int natv_sock_module_fini(){
	// FIXME:
    return 0;
}

