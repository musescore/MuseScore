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


#ifndef _FLUID_CHORUS_H
#define _FLUID_CHORUS_H

#include "fluidsynth_priv.h"


typedef struct _fluid_chorus_t fluid_chorus_t;

/* enum describing each chorus parameter */
enum fluid_chorus_param
{
    FLUID_CHORUS_NR,        /**< number of delay line */
    FLUID_CHORUS_LEVEL,     /**< output level */
    FLUID_CHORUS_SPEED,     /**< lfo frequency */
    FLUID_CHORUS_DEPTH,     /**< modulation depth */
    FLUID_CHORUS_TYPE,      /**< type of waveform */
    FLUID_CHORUS_PARAM_LAST /* number of enum fluid_chorus_param */
};

/* return a bit flag from param: 2^param */
#define FLUID_CHORPARAM_TO_SETFLAG(param) (1 << param)

/** Flags for fluid_chorus_set() */
typedef enum
{
    FLUID_CHORUS_SET_NR    = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_NR),
    FLUID_CHORUS_SET_LEVEL = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_LEVEL),
    FLUID_CHORUS_SET_SPEED = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_SPEED),
    FLUID_CHORUS_SET_DEPTH = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_DEPTH),
    FLUID_CHORUS_SET_TYPE  = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_TYPE),

    /** Value for fluid_chorus_set() which sets all chorus parameters. */
    FLUID_CHORUS_SET_ALL   =   FLUID_CHORUS_SET_NR
                               | FLUID_CHORUS_SET_LEVEL
                               | FLUID_CHORUS_SET_SPEED
                               | FLUID_CHORUS_SET_DEPTH
                               | FLUID_CHORUS_SET_TYPE,
} fluid_chorus_set_t;

/*
 * chorus
 */
fluid_chorus_t *new_fluid_chorus(fluid_real_t sample_rate);
void delete_fluid_chorus(fluid_chorus_t *chorus);
void fluid_chorus_reset(fluid_chorus_t *chorus);

void fluid_chorus_set(fluid_chorus_t *chorus, int set, int nr, fluid_real_t level,
                      fluid_real_t speed, fluid_real_t depth_ms, int type);
void
fluid_chorus_samplerate_change(fluid_chorus_t *chorus, fluid_real_t sample_rate);

void fluid_chorus_processmix(fluid_chorus_t *chorus, const fluid_real_t *in,
                             fluid_real_t *left_out, fluid_real_t *right_out);
void fluid_chorus_processreplace(fluid_chorus_t *chorus, const fluid_real_t *in,
                                 fluid_real_t *left_out, fluid_real_t *right_out);



#endif /* _FLUID_CHORUS_H */
