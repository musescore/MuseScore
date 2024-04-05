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


#ifndef _FLUID_RVOICE_H
#define _FLUID_RVOICE_H

#include "fluidsynth_priv.h"
#include "fluid_iir_filter.h"
#include "fluid_adsr_env.h"
#include "fluid_lfo.h"
#include "fluid_phase.h"
#include "fluid_sfont.h"

typedef struct _fluid_rvoice_envlfo_t fluid_rvoice_envlfo_t;
typedef struct _fluid_rvoice_dsp_t fluid_rvoice_dsp_t;
typedef struct _fluid_rvoice_buffers_t fluid_rvoice_buffers_t;
typedef struct _fluid_rvoice_t fluid_rvoice_t;

/* Smallest amplitude that can be perceived (full scale is +/- 0.5)
 * 16 bits => 96+4=100 dB dynamic range => 0.00001
 * 24 bits => 144-4 = 140 dB dynamic range => 1.e-7
 * 1.e-7 * 2 == 2.e-7 :)
 */
#define FLUID_NOISE_FLOOR ((fluid_real_t)2.e-7)

enum fluid_loop
{
    FLUID_UNLOOPED = 0,
    FLUID_LOOP_DURING_RELEASE = 1,
    FLUID_NOTUSED = 2,
    FLUID_LOOP_UNTIL_RELEASE = 3
};

/*
 * rvoice ticks-based parameters
 * These parameters must be updated even if the voice is currently quiet.
 */
struct _fluid_rvoice_envlfo_t
{
    /* Note-off minimum length */
    unsigned int ticks;
    unsigned int noteoff_ticks;

    /* vol env */
    fluid_adsr_env_t volenv;

    /* mod env */
    fluid_adsr_env_t modenv;
    fluid_real_t modenv_to_fc;
    fluid_real_t modenv_to_pitch;

    /* mod lfo */
    fluid_lfo_t modlfo;
    fluid_real_t modlfo_to_fc;
    fluid_real_t modlfo_to_pitch;
    fluid_real_t modlfo_to_vol;

    /* vib lfo */
    fluid_lfo_t viblfo;
    fluid_real_t viblfo_to_pitch;
};

/*
 * rvoice parameters needed for dsp interpolation
 */
struct _fluid_rvoice_dsp_t
{
    /* interpolation method, as in fluid_interp in fluidsynth.h */
    enum fluid_interp interp_method;
    enum fluid_loop samplemode;

    /* Flag that is set as soon as the first loop is completed. */
    char has_looped;

    /* Flag that initiates, that sample-related parameters have to be checked. */
    char check_sample_sanity_flag;

    fluid_sample_t *sample;

    /* sample and loop start and end points (offset in sample memory).  */
    int start;
    int end;
    int loopstart;
    int loopend;	/* Note: first point following the loop (superimposed on loopstart) */

    /* Stuff needed for portamento calculations */
    fluid_real_t pitchoffset;        /* the portamento range in midicents */
    fluid_real_t pitchinc;           /* the portamento increment in midicents */

    /* Stuff needed for phase calculations */

    fluid_real_t pitch;              /* the pitch in midicents */
    fluid_real_t root_pitch_hz;
    fluid_real_t output_rate;

    /* Stuff needed for amplitude calculations */

    fluid_real_t attenuation;        /* the attenuation in centibels */
    fluid_real_t prev_attenuation;   /* the previous attenuation in centibels
					used by fluid_rvoice_multi_retrigger_attack() */
    fluid_real_t min_attenuation_cB; /* Estimate on the smallest possible attenuation
					  * during the lifetime of the voice */
    fluid_real_t amplitude_that_reaches_noise_floor_nonloop;
    fluid_real_t amplitude_that_reaches_noise_floor_loop;
    fluid_real_t synth_gain; 	/* master gain */

    /* Dynamic input to the interpolator below */

    fluid_real_t amp;                /* current linear amplitude */
    fluid_real_t amp_incr;		/* amplitude increment value for the next FLUID_BUFSIZE samples */

    fluid_phase_t phase;             /* the phase (current sample offset) of the sample wave */
    fluid_real_t phase_incr;	/* the phase increment for the next FLUID_BUFSIZE samples */
};

/* Currently left, right, reverb, chorus. To be changed if we
   ever add surround positioning, or stereo reverb/chorus */
#define FLUID_RVOICE_MAX_BUFS (4)

/*
 * rvoice mixer-related parameters
 */
struct _fluid_rvoice_buffers_t
{
    unsigned int count; /* Number of records in "bufs" */
    struct
    {
        /* the actual, linearly interpolated amplitude with which the dsp sample should be mixed into the buf */
        fluid_real_t current_amp;

        /* the desired amplitude [...] mixed into the buf (directly set by e.g. rapidly changing PAN events) */
        fluid_real_t target_amp;

        /* Mapping to mixdown buffer index */
        int mapping;
    } bufs[FLUID_RVOICE_MAX_BUFS];
};


/*
 * Hard real-time parameters needed to synthesize a voice
 */
struct _fluid_rvoice_t
{
    fluid_rvoice_envlfo_t envlfo;
    fluid_rvoice_dsp_t dsp;
    fluid_iir_filter_t resonant_filter; /* IIR resonant dsp filter */
    fluid_iir_filter_t resonant_custom_filter; /* optional custom/general-purpose IIR resonant filter */
    fluid_rvoice_buffers_t buffers;
};


int fluid_rvoice_write(fluid_rvoice_t *voice, fluid_real_t *dsp_buf);

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_buffers_set_amp);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_buffers_set_mapping);

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_noteoff);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_voiceoff);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_reset);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_multi_retrigger_attack);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_portamento);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_output_rate);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_interp_method);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_root_pitch_hz);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_pitch);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_attenuation);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_min_attenuation_cB);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_viblfo_to_pitch);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_pitch);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_vol);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_fc);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modenv_to_fc);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modenv_to_pitch);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_synth_gain);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_start);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_end);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_loopstart);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_loopend);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_samplemode);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_sample);

/* defined in fluid_rvoice_dsp.c */
void fluid_rvoice_dsp_config(void);
int fluid_rvoice_dsp_interpolate_none(fluid_rvoice_dsp_t *voice, fluid_real_t *FLUID_RESTRICT dsp_buf, int is_looping);
int fluid_rvoice_dsp_interpolate_linear(fluid_rvoice_dsp_t *voice, fluid_real_t *FLUID_RESTRICT dsp_buf, int is_looping);
int fluid_rvoice_dsp_interpolate_4th_order(fluid_rvoice_dsp_t *voice, fluid_real_t *FLUID_RESTRICT dsp_buf, int is_looping);
int fluid_rvoice_dsp_interpolate_7th_order(fluid_rvoice_dsp_t *voice, fluid_real_t *FLUID_RESTRICT dsp_buf, int is_looping);


/*
 * Combines the most significant 16 bit part of a sample with a potentially present
 * least sig. 8 bit part in order to create a 24 bit sample.
 */
static FLUID_INLINE int32_t
fluid_rvoice_get_sample(const short int *dsp_msb, const char *dsp_lsb, unsigned int idx)
{
    /* cast sample to unsigned type, so we can safely shift and bitwise or
     * without relying on undefined behaviour (should never happen anyway ofc...) */
    uint32_t msb = (uint32_t)dsp_msb[idx];
    uint8_t lsb = 0U;

    /* most soundfonts have 16 bit samples, assume that it's unlikely we
     * experience 24 bit samples here */
    if(FLUID_UNLIKELY(dsp_lsb != NULL))
    {
        lsb = (uint8_t)dsp_lsb[idx];
    }

    return (int32_t)((msb << 8) | lsb);
}

#endif
