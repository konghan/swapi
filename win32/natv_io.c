/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#include "natv_io.h"
#include "swapi_sys_atomic.h"
#include "swapi_sys_cache.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define kNATV_IO_ROOTFS			"/home/konghan/swapi/rootfs/"
#define kNATV_IO_SWAPS			"swaps/"
#define kNATV_IO_DATA			"/data/"

#define kNATV_IO_MAX_PATH		256

struct natv_io{
	int			ni_status;
	atomic_t	ni_refs;
	char		ni_path[kNATV_IO_MAX_PATH];
};

enum {
	kNATV_IO_FILE_TYPE_READ = 0,
	kNATV_IO_FILE_TYPE_WRITE,
};

struct natv_file{
	int		nf_type;
	char	nf_name[kNATV_IO_MAX_PATH];

	int		nf_pos;

	FILE	*nf_file;
};

int natv_io_open(const char *swap, natv_io_t **fio){
	natv_io_t	*ni;

	if((strlen(swap) + strlen(kNATV_IO_ROOTFS) + strlen(kNATV_IO_SWAPS)) 
			>= kNATV_IO_MAX_PATH){
		return -1;
	}

	ni = swapi_heap_alloc(sizeof(*ni));
	if(ni == NULL){
		return -1;
	}
	memset(ni, 0, sizeof(*ni));

	strcpy(ni->ni_path, kNATV_IO_ROOTFS);
	strcat(ni->ni_path, kNATV_IO_SWAPS);
	strcat(ni->ni_path, swap);

	if(_access(ni->ni_path, 0) != 0){
		swapi_heap_free(ni);
		return -1;
	}

	ni->ni_status = 1;

	*fio = ni;

	return 0;
}

int natv_io_close(natv_io_t *fio){
	ASSERT(fio != NULL);

	if(fio->ni_status && (fio->ni_refs == 0)){
		fio->ni_status = 0;
		swapi_heap_free(fio);
		return 0;
	}

	return -1;
}

int natv_io_fopen(natv_io_t *fio, const char *file, natv_file_t **nf){
	natv_file_t		*f;
	
	ASSERT((fio != NULL) && (file != NULL) && (nf != NULL));

	if((strlen(file) + strlen(fio->ni_path)) >= (kNATV_IO_MAX_PATH - 1)){
		return -1;
	}

	f = swapi_heap_alloc(sizeof(*f));
	if(f == NULL){
		return -ENOMEM;
	}
	memset(f, 0, sizeof(*f));

	strcpy(f->nf_name, fio->ni_path);
	strcat(f->nf_name, "/");
	strcat(f->nf_name, file);

	f->nf_file = fopen(f->nf_name, "r");
	if(f->nf_file == NULL){
		swapi_heap_free(f);
		return -1;
	}

	atomic_inc(&fio->ni_refs);

	*nf = f;

	return 0;
}

int natv_io_fclose(natv_file_t *nf){
	ASSERT(nf != NULL);

	if(nf->nf_file != NULL){
		fclose(nf->nf_file);
		nf->nf_file = NULL;
	}

	swapi_heap_free(nf);

	return 0;
}

size_t natv_io_fread(natv_file_t *nf, char *buf, size_t size){
	ASSERT((nf != NULL) && (buf != NULL));
	ASSERT(nf->nf_file != NULL);

	return fread(buf, size, 0, nf->nf_file);
}

int natv_io_fseek(natv_file_t *nf, long offset, int origin){
	ASSERT(nf != NULL);
	ASSERT(nf->nf_file != NULL);

	return fseek(nf->nf_file, offset, origin);
}

int natv_io_dopen(natv_io_t *fio, const char *file, natv_file_t **nf){
	natv_file_t		*f;
	
	ASSERT((fio != NULL) && (file != NULL) && (nf != NULL));

	if((strlen(file) + strlen(fio->ni_path) + strlen(kNATV_IO_DATA)) 
			>= (kNATV_IO_MAX_PATH - 1)){
		return -1;
	}

	f = swapi_heap_alloc(sizeof(*f));
	if(f == NULL){
		return -ENOMEM;
	}
	memset(f, 0, sizeof(*f));

	strcpy(f->nf_name, fio->ni_path);
	strcat(f->nf_name, kNATV_IO_DATA);
	strcat(f->nf_name, file);

	f->nf_file = fopen(f->nf_name, "r+");
	if(f->nf_file == NULL){
		swapi_heap_free(f);
		return -1;
	}

	atomic_inc(&fio->ni_refs);

	*nf = f;

	return 0;
}

int natv_io_dclose(natv_file_t *nf){
	ASSERT(nf != NULL);

	if(nf->nf_file != NULL){
		fclose(nf->nf_file);
		nf->nf_file = NULL;
	}

	swapi_heap_free(nf);

	return 0;
}

size_t natv_io_dread(natv_file_t *nf, void *buf, size_t size){
	ASSERT((nf != NULL) && (buf != NULL));
	ASSERT(nf->nf_file != NULL);

	return fread(buf, size, 0, nf->nf_file);
}

size_t natv_io_dwrite(natv_file_t *nf, const void *buf, size_t size){
	ASSERT((nf != NULL) && (nf->nf_file != NULL));
	ASSERT(buf != NULL);

	return fwrite(buf, size, 1, nf->nf_file);
}

int natv_io_module_init(){
	return 0;
}

int natv_io_module_fini(){
	return 0;
}

