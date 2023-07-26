/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/*
 * @file fluidsynth_priv.h
 *
 * lightweight part of fluid_sys.h, containing forward declarations of fluidsynth's private types and private macros
 *
 * include this one file in fluidsynth's private header files
 */

#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

#include "fluidsynthconfig.h"

#ifndef NO_GLIB
#include <glib.h>
#endif

#if HAVE_MATH_H
#include <math.h> // M_PI, MLN2, M_LN10
#endif

#if HAVE_STDLIB_H
#include <stdlib.h> // malloc, free
#endif

#if HAVE_STDIO_H
#include <stdio.h> // printf
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STRINGS_H
#include <strings.h>
#endif

#include "fluidsynth.h"

#ifdef NO_GLIB
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#endif


#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 *         BASIC TYPES
 */

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif

#if defined(SUPPORTS_VLA)
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type _name[_len]
#else

#if defined(NO_GLIB)
#include <stdlib.h>
#ifdef _MSC_VER
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type* _name = _alloca(_len*sizeof(_type))
#else
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type* _name = alloca(_len*sizeof(_type))
#endif

#else // NO_GLIB
#  define FLUID_DECLARE_VLA(_type, _name, _len) \
     _type* _name = g_newa(_type, (_len))
#endif //NO_GLIB

#endif //SUPPORTS_VLA

/** Atomic types  */
typedef int fluid_atomic_int_t;
typedef unsigned int fluid_atomic_uint_t;
typedef float fluid_atomic_float_t;

/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_adriver_definition_t fluid_adriver_definition_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_hashtable_t fluid_hashtable_t;
typedef struct _fluid_client_t fluid_client_t;
typedef struct _fluid_server_socket_t fluid_server_socket_t;
typedef struct _fluid_sample_timer_t fluid_sample_timer_t;
typedef struct _fluid_zone_range_t fluid_zone_range_t;
typedef struct _fluid_rvoice_eventhandler_t fluid_rvoice_eventhandler_t;

/* Declare rvoice related typedefs here instead of fluid_rvoice.h, as it's needed
 * in fluid_lfo.c and fluid_adsr.c as well */
typedef union _fluid_rvoice_param_t
{
    void* ptr;
    int i;
    fluid_real_t real;
} fluid_rvoice_param_t;
enum { MAX_EVENT_PARAMS = 7 }; /**< Maximum number of #fluid_rvoice_param_t to be passed to an #fluid_rvoice_function_t */
typedef void (*fluid_rvoice_function_t)(void *obj, const fluid_rvoice_param_t param[MAX_EVENT_PARAMS]);

/* Macro for declaring an rvoice event function (#fluid_rvoice_function_t). The functions may only access
 * those params that were previously set in fluid_voice.c
 */
#define DECLARE_FLUID_RVOICE_FUNCTION(name) void name(void* obj, const fluid_rvoice_param_t param[MAX_EVENT_PARAMS])

/***************************************************************
 *
 *                      CONSTANTS
 */

#define FLUID_BUFSIZE                64         /**< FluidSynth internal buffer size (in samples) */
#define FLUID_MIXER_MAX_BUFFERS_DEFAULT (8192 / FLUID_BUFSIZE) /**< Number of buffers that can be processed in one rendering run */
#define FLUID_MAX_EVENTS_PER_BUFSIZE 1024       /**< Maximum queued MIDI events per #FLUID_BUFSIZE */
#define FLUID_MAX_RETURN_EVENTS      1024       /**< Maximum queued synthesis thread return events */
#define FLUID_MAX_EVENT_QUEUES       16         /**< Maximum number of unique threads queuing events */
#define FLUID_DEFAULT_AUDIO_RT_PRIO  60         /**< Default setting for audio.realtime-prio */
#define FLUID_DEFAULT_MIDI_RT_PRIO   50         /**< Default setting for midi.realtime-prio */
#define FLUID_NUM_MOD                64         /**< Maximum number of modulators in a voice */

/***************************************************************
 *
 *                      SYSTEM INTERFACE
 */

/* Math constants */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530941723212145818
#endif

#ifndef M_LN10
#define M_LN10 2.3025850929940456840179914546844
#endif

#define FLUID_M_PI      ((fluid_real_t)M_PI)
#define FLUID_M_LN2     ((fluid_real_t)M_LN2)
#define FLUID_M_LN10    ((fluid_real_t)M_LN10)

/* Math functions */
#if defined WITH_FLOAT && defined HAVE_SINF
#define FLUID_SIN   sinf
#else
#define FLUID_SIN   (fluid_real_t)sin
#endif

#if defined WITH_FLOAT && defined HAVE_COSF
#define FLUID_COS   cosf
#else
#define FLUID_COS   (fluid_real_t)cos
#endif

#if defined WITH_FLOAT && defined HAVE_FABSF
#define FLUID_FABS  fabsf
#else
#define FLUID_FABS  (fluid_real_t)fabs
#endif

#if defined WITH_FLOAT && defined HAVE_POWF
#define FLUID_POW   powf
#else
#define FLUID_POW   (fluid_real_t)pow
#endif

#if defined WITH_FLOAT && defined HAVE_SQRTF
#define FLUID_SQRT  sqrtf
#else
#define FLUID_SQRT  (fluid_real_t)sqrt
#endif

#if defined WITH_FLOAT && defined HAVE_LOGF
#define FLUID_LOGF  logf
#else
#define FLUID_LOGF  (fluid_real_t)log
#endif

/* Memory allocation */
#define FLUID_MALLOC(_n)             fluid_alloc(_n)
#define FLUID_REALLOC(_p,_n)         realloc(_p,_n)
#define FLUID_FREE(_p)               fluid_free(_p)
#define FLUID_NEW(_t)                (_t*)FLUID_MALLOC(sizeof(_t))
#define FLUID_ARRAY_ALIGNED(_t,_n,_a) (_t*)FLUID_MALLOC((_n) * sizeof(_t) + ((unsigned int)_a - 1u))
#define FLUID_ARRAY(_t,_n)           FLUID_ARRAY_ALIGNED(_t,_n,1u)

void* fluid_alloc(size_t len);

/* File access */
#define FLUID_FOPEN(_f,_m)           fluid_fopen(_f,_m)
#define FLUID_FCLOSE(_f)             fclose(_f)
#define FLUID_FREAD(_p,_s,_n,_f)     fread(_p,_s,_n,_f)

FILE *fluid_fopen(const char *filename, const char *mode);

#ifdef _WIN32
#define FLUID_FSEEK(_f,_n,_set)      _fseeki64(_f,_n,_set)
#else
#define FLUID_FSEEK(_f,_n,_set)      fseek(_f,_n,_set)
#endif

#define FLUID_FTELL(_f)              fluid_file_tell(_f)

/* Memory functions */
#define FLUID_MEMCPY(_dst,_src,_n)   memcpy(_dst,_src,_n)
#define FLUID_MEMSET(_s,_c,_n)       memset(_s,_c,_n)

/* String functions */
#define FLUID_STRLEN(_s)             strlen(_s)
#define FLUID_STRCMP(_s,_t)          strcmp(_s,_t)
#define FLUID_STRNCMP(_s,_t,_n)      strncmp(_s,_t,_n)
#define FLUID_STRCPY(_dst,_src)      strcpy(_dst,_src)
#define FLUID_STRTOL(_s,_e,_b)       strtol(_s,_e,_b)

#define FLUID_STRNCPY(_dst,_src,_n) \
do { strncpy(_dst,_src,_n-1); \
    (_dst)[(_n)-1]='\0'; \
}while(0)

#define FLUID_STRCHR(_s,_c)          strchr(_s,_c)
#define FLUID_STRRCHR(_s,_c)         strrchr(_s,_c)

#ifdef strdup
#define FLUID_STRDUP(s)          strdup(s)
#else
#define FLUID_STRDUP(s)          FLUID_STRCPY(FLUID_MALLOC(FLUID_STRLEN(s) + 1), s)
#endif

#define FLUID_SPRINTF                sprintf
#define FLUID_FPRINTF                fprintf

//#if (defined(_WIN32) && _MSC_VER < 1900) || defined(MINGW32)
///* need to make sure we use a C99 compliant implementation of (v)snprintf(),
// * i.e. not microsofts non compliant extension _snprintf() as it doesn't
// * reliably null-terminate the buffer
// */
//#define FLUID_SNPRINTF           g_snprintf
//#else
#define FLUID_SNPRINTF           snprintf
//#endif

//#if (defined(_WIN32) && _MSC_VER < 1500) || defined(MINGW32)
//#define FLUID_VSNPRINTF          g_vsnprintf
//#else
#define FLUID_VSNPRINTF          vsnprintf
//#endif

#if defined(_WIN32) && !defined(MINGW32)
#define FLUID_STRCASECMP         _stricmp
#else
#define FLUID_STRCASECMP         strcasecmp
#endif

#if defined(_WIN32) && !defined(MINGW32)
#define FLUID_STRNCASECMP         _strnicmp
#else
#define FLUID_STRNCASECMP         strncasecmp
#endif

#define fluid_clip(_val, _min, _max) \
    { (_val) = ((_val) < (_min)) ? (_min) : (((_val) > (_max)) ? (_max) : (_val)); }

#if WITH_FTS
#define FLUID_PRINTF                 post
#define FLUID_FLUSH()
#else
#define FLUID_PRINTF                 printf
#define FLUID_FLUSH()                fflush(stdout)
#endif

/* People who want to reduce the size of the may do this by entirely
 * removing the logging system. This will cause all log messages to
 * be discarded at compile time, allowing to save about 80 KiB for
 * the compiled binary.
 */
#if 0
#define FLUID_LOG                    (void)sizeof
#else
#define FLUID_LOG                    fluid_log
#endif

#ifndef NO_GLIB
#ifdef DEBUG
#define FLUID_ASSERT(a) g_assert(a)
#else
#define FLUID_ASSERT(a)
#endif

#define FLUID_LIKELY G_LIKELY
#define FLUID_UNLIKELY G_UNLIKELY

#else
#define FLUID_ASSERT(a)

#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 303)
#define FLUID_LIKELY(x) __builtin_expect(!!(x), 1)
#define FLUID_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define FLUID_LIKELY(x) (x)
#define FLUID_UNLIKELY(x) (x)
#endif
#endif

/* Misc */
#if defined(__INTEL_COMPILER)
#define FLUID_RESTRICT restrict
#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FLUID_RESTRICT __restrict__
#elif defined(_MSC_VER) && _MSC_VER >= 1400
#define FLUID_RESTRICT __restrict
#else
#warning "Dont know how this compiler handles restrict pointers, refuse to use them."
#define FLUID_RESTRICT
#endif

#define FLUID_N_ELEMENTS(struct)  (sizeof(struct) / sizeof(struct[0]))
#define FLUID_MEMBER_SIZE(struct, member)  (sizeof(((struct*)0)->member))

#define fluid_return_if_fail(cond) \
    if (cond) \
    ; \
    else \
    return

#define fluid_return_val_if_fail(cond, val) \
 fluid_return_if_fail(cond) (val)

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_PRIV_H */
