/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.6
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

#ifndef _TLSF_H_
#define _TLSF_H_

#include <sys/types.h>

extern size_t rtl_init_memory_pool(size_t, void *);
extern size_t rtl_get_used_size(void *);
extern size_t rtl_get_max_size(void *);
extern void rtl_destroy_memory_pool(void *);
extern size_t rtl_add_new_area(void *, size_t, void *);
extern void *rtl_malloc_ex(size_t, void *);
extern void rtl_free_ex(void *, void *);
extern void *rtl_realloc_ex(void *, size_t, void *);
extern void *rtl_calloc_ex(size_t, size_t, void *);

extern void *rtl_tlsf_malloc(size_t size);
extern void rtl_tlsf_free(void *ptr);
extern void *rtl_tlsf_realloc(void *ptr, size_t size);
extern void *rtl_tlsf_calloc(size_t nelem, size_t elem_size);

#endif
