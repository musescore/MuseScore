/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * @ingroup lavu_mem
 * Memory handling functions
 */

#ifndef AVUTIL_MEM_H
#define AVUTIL_MEM_H

#include <limits.h>
#include <stdint.h>

#include "attributes.h"
#include "avutil.h"
#include "version.h"

/**
 * @addtogroup lavu_mem
 * Utilities for manipulating memory.
 *
 * FFmpeg has several applications of memory that are not required of a typical
 * program. For example, the computing-heavy components like video decoding and
 * encoding can be sped up significantly through the use of aligned memory.
 *
 * However, for each of FFmpeg's applications of memory, there might not be a
 * recognized or standardized API for that specific use. Memory alignment, for
 * instance, varies wildly depending on operating systems, architectures, and
 * compilers. Hence, this component of @ref libavutil is created to make
 * dealing with memory consistently possible on all platforms.
 *
 * @{
 */

#if defined(FF_API_DECLARE_ALIGNED) && FF_API_DECLARE_ALIGNED
/**
 *
 * @defgroup lavu_mem_macros Alignment Macros
 * Helper macros for declaring aligned variables.
 * @{
 */

#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1110 || defined(__SUNPRO_C)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_ALIGNED(n,t,v)  t __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_CONST(n,t,v)    const t __attribute__ ((aligned (n))) v
#elif defined(__DJGPP__)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (FFMIN(n, 16)))) v
    #define DECLARE_ASM_ALIGNED(n,t,v)  t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
    #define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (FFMIN(n, 16)))) v
#elif defined(__GNUC__) || defined(__clang__)
    #define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_ALIGNED(n,t,v)  t av_used __attribute__ ((aligned (n))) v
    #define DECLARE_ASM_CONST(n,t,v)    static const t av_used __attribute__ ((aligned (n))) v
#elif defined(_MSC_VER)
    #define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
    #define DECLARE_ASM_ALIGNED(n,t,v)  __declspec(align(n)) t v
    #define DECLARE_ASM_CONST(n,t,v)    __declspec(align(n)) static const t v
#else
    #define DECLARE_ALIGNED(n,t,v)      t v
    #define DECLARE_ASM_ALIGNED(n,t,v)  t v
    #define DECLARE_ASM_CONST(n,t,v)    static const t v
#endif

/**
 * @}
 */
#endif

/**
 * @defgroup lavu_mem_attrs Function Attributes
 * @{
 */

#if AV_GCC_VERSION_AT_LEAST(3,1)
    #define av_malloc_attrib __attribute__((__malloc__))
#else
    #define av_malloc_attrib
#endif

#if AV_GCC_VERSION_AT_LEAST(4,3)
    #define av_alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
#else
    #define av_alloc_size(...)
#endif

/**
 * @}
 */

/**
 * @defgroup lavu_mem_funcs Heap Management
 * @{
 */

void *av_malloc(size_t size) av_malloc_attrib av_alloc_size(1);
void *av_mallocz(size_t size) av_malloc_attrib av_alloc_size(1);
av_alloc_size(1, 2) void *av_malloc_array(size_t nmemb, size_t size);
void *av_calloc(size_t nmemb, size_t size) av_malloc_attrib av_alloc_size(1, 2);
void *av_realloc(void *ptr, size_t size) av_alloc_size(2);
av_warn_unused_result
int av_reallocp(void *ptr, size_t size);
void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);
av_alloc_size(2, 3) void *av_realloc_array(void *ptr, size_t nmemb, size_t size);
int av_reallocp_array(void *ptr, size_t nmemb, size_t size);
void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_mallocz(void *ptr, unsigned int *size, size_t min_size);
void av_free(void *ptr);
void av_freep(void *ptr);
char *av_strdup(const char *s) av_malloc_attrib;
char *av_strndup(const char *s, size_t len) av_malloc_attrib;
void *av_memdup(const void *p, size_t size);
void av_memcpy_backptr(uint8_t *dst, int back, int cnt);
void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem);
av_warn_unused_result
int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem);
void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
                       const uint8_t *elem_data);
int av_size_mult(size_t a, size_t b, size_t *r);
void av_max_alloc(size_t max);

/**
 * @}
 * @}
 */

#endif /* AVUTIL_MEM_H */
