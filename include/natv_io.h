/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under GNU v3 license, see the LICENSE file.
 */

#ifndef __NATV_IO_H__
#define __NATV_IO_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct natv_io;
typedef struct natv_io natv_io_t;

struct natv_file;
typedef struct natv_file natv_file_t;

int natv_io_open(const char *swap, natv_io_t **fio);
int natv_io_close(natv_io_t *fio);

int natv_io_fopen(natv_io_t *fio, const char *file, natv_file_t **nf);
int natv_io_fclose(natv_file_t *nf);

size_t natv_io_fread(natv_file_t *nf, char *buf, size_t size);

int natv_io_seek(natv_file_t *nf, long offset, int original);

int natv_io_dopen(natv_io_t *fio, const char *file, natv_file_t **nf);
int natv_io_dclose(natv_file_t *nf);

size_t natv_io_dread(natv_file_t *nf, char *buf, size_t size);
size_t natv_io_dwrite(natv_file_t *nf, const char *buf, size_t size);

int natv_io_module_init();
int natv_io_module_fini();

#ifdef __cplusplus
}
#endif

#endif //__NATV_IO_H__

