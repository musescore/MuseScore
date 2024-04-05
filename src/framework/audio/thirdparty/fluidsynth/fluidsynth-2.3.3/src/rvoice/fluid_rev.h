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


#ifndef _FLUID_REV_H
#define _FLUID_REV_H

#include "fluidsynth_priv.h"

typedef struct _fluid_revmodel_t fluid_revmodel_t;

/* enum describing each reverb parameter */
enum fluid_reverb_param
{
    FLUID_REVERB_ROOMSIZE,  /**< reverb time */
    FLUID_REVERB_DAMP,      /**< high frequency damping */
    FLUID_REVERB_WIDTH,     /**< stereo width */
    FLUID_REVERB_LEVEL,     /**< output level */
    FLUID_REVERB_PARAM_LAST /* number of enum fluid_reverb_param */
};

/* return a bit flag from param: 2^param */
#define FLUID_REVPARAM_TO_SETFLAG(param) (1 << param)

/** Flags for fluid_revmodel_set() */
typedef enum
{
    FLUID_REVMODEL_SET_ROOMSIZE       = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_ROOMSIZE),
    FLUID_REVMODEL_SET_DAMPING        = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_DAMP),
    FLUID_REVMODEL_SET_WIDTH          = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_WIDTH),
    FLUID_REVMODEL_SET_LEVEL          = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_LEVEL),

    /** Value for fluid_revmodel_set() which sets all reverb parameters. */
    FLUID_REVMODEL_SET_ALL            =   FLUID_REVMODEL_SET_LEVEL
                                          | FLUID_REVMODEL_SET_WIDTH
                                          | FLUID_REVMODEL_SET_DAMPING
                                          | FLUID_REVMODEL_SET_ROOMSIZE,
} fluid_revmodel_set_t;

/*
 * reverb preset
 */
typedef struct _fluid_revmodel_presets_t
{
    const char *name;
    fluid_real_t roomsize;
    fluid_real_t damp;
    fluid_real_t width;
    fluid_real_t level;
} fluid_revmodel_presets_t;


/*
 * reverb
 */
fluid_revmodel_t *
new_fluid_revmodel(fluid_real_t sample_rate_max, fluid_real_t sample_rate);

void delete_fluid_revmodel(fluid_revmodel_t *rev);

void fluid_revmodel_processmix(fluid_revmodel_t *rev, const fluid_real_t *in,
                               fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_processreplace(fluid_revmodel_t *rev, const fluid_real_t *in,
                                   fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_reset(fluid_revmodel_t *rev);

void fluid_revmodel_set(fluid_revmodel_t *rev, int set, fluid_real_t roomsize,
                        fluid_real_t damping, fluid_real_t width, fluid_real_t level);

int fluid_revmodel_samplerate_change(fluid_revmodel_t *rev, fluid_real_t sample_rate);

#endif /* _FLUID_REV_H */
