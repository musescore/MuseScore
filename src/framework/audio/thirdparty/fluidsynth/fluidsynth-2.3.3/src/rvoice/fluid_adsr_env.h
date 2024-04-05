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

#ifndef _FLUID_ADSR_ENVELOPE_H
#define _FLUID_ADSR_ENVELOPE_H

#include "fluidsynth_priv.h"
#include "fluid_sys.h"

/*
 * envelope data
 */
struct _fluid_env_data_t
{
    unsigned int count;
    fluid_real_t coeff;
    fluid_real_t increment;
    fluid_real_t min;
    fluid_real_t max;
};

/* Indices for envelope tables */
enum fluid_voice_envelope_index
{
    FLUID_VOICE_ENVDELAY,
    FLUID_VOICE_ENVATTACK,
    FLUID_VOICE_ENVHOLD,
    FLUID_VOICE_ENVDECAY,
    FLUID_VOICE_ENVSUSTAIN,
    FLUID_VOICE_ENVRELEASE,
    FLUID_VOICE_ENVFINISHED,
    FLUID_VOICE_ENVLAST
};

typedef enum fluid_voice_envelope_index fluid_adsr_env_section_t;

typedef struct _fluid_adsr_env_t fluid_adsr_env_t;

struct _fluid_adsr_env_t
{
    fluid_env_data_t data[FLUID_VOICE_ENVLAST];
    unsigned int count;
    fluid_real_t val;         /* the current value of the envelope */
    fluid_adsr_env_section_t section;
};

/* For performance, all functions are inlined */

static FLUID_INLINE void
fluid_adsr_env_calc(fluid_adsr_env_t *env)
{
    fluid_env_data_t *env_data;
    fluid_real_t x;

    env_data = &env->data[env->section];

    /* skip to the next section of the envelope if necessary */
    while(env->count >= env_data->count)
    {
        // If we're switching envelope stages from decay to sustain, force the value to be the end value of the previous stage
        // Hmm, should this only apply to volenv? It was so before refactoring, so keep it for now. [DH]
        // No, must apply to both, otherwise some voices may sound detuned. [TM] (https://github.com/FluidSynth/fluidsynth/issues/1059)
        if(env->section == FLUID_VOICE_ENVDECAY)
        {
            env->val = env_data->min * env_data->coeff;
        }

        env_data = &env->data[++env->section];
        env->count = 0;
    }

    /* calculate the envelope value and check for valid range */
    x = env_data->coeff * env->val + env_data->increment;

    if(x < env_data->min)
    {
        x = env_data->min;
        env->section++;
        env->count = 0;
    }
    else if(x > env_data->max)
    {
        x = env_data->max;
        env->section++;
        env->count = 0;
    }
    else
    {
        env->count++;
    }

    env->val = x;
}

/* This one cannot be inlined since it is referenced in
   the event queue */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_adsr_env_set_data);

static FLUID_INLINE void
fluid_adsr_env_reset(fluid_adsr_env_t *env)
{
    env->count = 0;
    env->section = FLUID_VOICE_ENVDELAY;
    env->val = 0.0f;
}

static FLUID_INLINE fluid_real_t
fluid_adsr_env_get_val(fluid_adsr_env_t *env)
{
    return env->val;
}

static FLUID_INLINE void
fluid_adsr_env_set_val(fluid_adsr_env_t *env, fluid_real_t val)
{
    env->val = val;
}

static FLUID_INLINE fluid_adsr_env_section_t
fluid_adsr_env_get_section(fluid_adsr_env_t *env)
{
    return env->section;
}

static FLUID_INLINE void
fluid_adsr_env_set_section(fluid_adsr_env_t *env,
                           fluid_adsr_env_section_t section)
{
    env->section = section;
    env->count = 0;
}

/* Used for determining which voice to kill.
   Returns max amplitude from now, and forward in time.
*/
static FLUID_INLINE fluid_real_t
fluid_adsr_env_get_max_val(fluid_adsr_env_t *env)
{
    if(env->section > FLUID_VOICE_ENVATTACK)
    {
        return env->val * 1000;
    }
    else
    {
        return env->data[FLUID_VOICE_ENVATTACK].max;
    }
}

#endif

