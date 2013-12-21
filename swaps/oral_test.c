/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "oral_model.h"

#include <stdio.h>
#include <stdlib.h>

int oral_user_test(){
	oral_user_t		*u1, *u2;
	oral_user_t		*p;

	u1 = malloc(sizeof(*u1) + 128);
	u1->ou_name = (char *)(u1 + 1);
	u1->ou_picn = u1->ou_name + 64;

	memcpy(&u1->ou_uid, "uuid-23049809726", 16);
	u1->ou_name_len = 6;
	memcpy(u1->ou_name, "mother", 6);
	u1->ou_picn_len = 10;
	memcpy(u1->ou_picn, "mother.png", 10);

	u2 = malloc(sizeof(*u1) + 128);
	u2->ou_name = (char *)(u1 + 1);
	u2->ou_picn = u1->ou_name + 64;

	memcpy(&u2->ou_uid, "uuid-258949809726", 16);
	u2->ou_name_len = 6;
	memcpy(u2->ou_name, "father", 6);
	u2->ou_picn_len = 10;
	memcpy(u2->ou_picn, "father.png", 10);

//	oral_user_del(&u2->ou_uid);

#if 0
	printf("befor add u1\n");
	if(oral_user_add(u1) != 0){
		printf("add user 1 fail!\n");
	}
	printf("after add u1\n");
	
	if(oral_user_add(u2) != 0){
		printf("add user 2 fail!\n");
	}
#endif
	return 0;
}

static int oral_vmsg_text(){
	oral_vmsg_ctrl_t	ovc;
	oral_vmsg_t			*ov;
	swapi_uuid_t		uid;
	int					i;

	memset(&uid, 0, sizeof(uid));
	strcpy((char *)&uid, "uuid-9982");
	oral_vmsg_ctrl_init(&ovc, &uid);

	if(oral_vmsg_load(&ovc) != 0){
		printf("load ovc fail!\n");
		return -1;
	}

#if 1
	for(i = 0; i < 10; i++){
		ov = malloc(sizeof(*ov) + 128);
		if(ov == NULL){
			printf("alloc memory for ov fail!\n");
			return -1;
		}
		memset(ov, 0, sizeof(*ov) + 128);

		swapi_uuid_cpy(&ov->ov_uid, &uid);

		ov->ov_text = (char *)(ov + 1);
		strcpy(ov->ov_text, "text0text1text2text4");
		ov->ov_text_len = 21;

		ov->ov_voice = ov->ov_text + 64;
		strcpy(ov->ov_voice, "texv0texv1texv2texv4");
		ov->ov_voice_len = 21;

		
		ov->ov_seq = i;
		ov->ov_type = kORAL_VMSG_TYPE_TEXT|kORAL_VMSG_TYPE_VOICE;

		if(oral_vmsg_add(&ovc, ov) != 0){
			printf("add ov fail!\n");
		}else{
			printf("add ov %d \n", i);
		}
	}
#endif

#if 1
	oral_vmsg_pack(&ovc);
#endif

	return 0;
}


int main(){
	oral_model_init();

	printf("model init ok\n");
	
	oral_user_test();

	oral_vmsg_text();

	oral_model_fini();

	return 0;
}

