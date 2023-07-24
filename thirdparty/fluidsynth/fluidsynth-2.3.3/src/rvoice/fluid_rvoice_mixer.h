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


#ifndef _FLUID_RVOICE_MIXER_H
#define _FLUID_RVOICE_MIXER_H

#include "fluidsynth_priv.h"
#include "fluid_rvoice.h"
#include "fluid_ladspa.h"

typedef struct _fluid_rvoice_mixer_t fluid_rvoice_mixer_t;

int fluid_rvoice_mixer_render(fluid_rvoice_mixer_t *mixer, int blockcount);
int fluid_rvoice_mixer_get_bufs(fluid_rvoice_mixer_t *mixer,
                                fluid_real_t **left, fluid_real_t **right);
int fluid_rvoice_mixer_get_fx_bufs(fluid_rvoice_mixer_t *mixer,
                                   fluid_real_t **fx_left, fluid_real_t **fx_right);
int fluid_rvoice_mixer_get_bufcount(fluid_rvoice_mixer_t *mixer);
#if WITH_PROFILING
int fluid_rvoice_mixer_get_active_voices(fluid_rvoice_mixer_t *mixer);
#endif
fluid_rvoice_mixer_t *new_fluid_rvoice_mixer(int buf_count, int fx_buf_count, int fx_units,
        fluid_real_t sample_rate_max, fluid_real_t sample_rate,
        fluid_rvoice_eventhandler_t *, int, int);

void delete_fluid_rvoice_mixer(fluid_rvoice_mixer_t *);

void
fluid_rvoice_mixer_set_reverb_full(const fluid_rvoice_mixer_t *mixer,
                                   int fx_group, int set, const double values[]);

double
fluid_rvoice_mixer_reverb_get_param(const fluid_rvoice_mixer_t *mixer,
                                    int fx_group, int param);
void
fluid_rvoice_mixer_set_chorus_full(const fluid_rvoice_mixer_t *mixer,
                                   int fx_group, int set, const double values[]);
double
fluid_rvoice_mixer_chorus_get_param(const fluid_rvoice_mixer_t *mixer,
                                    int fx_group, int param);


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_add_voice);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_samplerate);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_polyphony);

/* @deprecated */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_chorus_enabled);
/* @deprecated */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_reverb_enabled);

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reverb_enable);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_chorus_enable);

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_chorus_params);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_reverb_params);

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reset_reverb);
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reset_chorus);



void fluid_rvoice_mixer_set_mix_fx(fluid_rvoice_mixer_t *mixer, int on);
#ifdef LADSPA
void fluid_rvoice_mixer_set_ladspa(fluid_rvoice_mixer_t *mixer,
                                   fluid_ladspa_fx_t *ladspa_fx, int audio_groups);
#endif

#endif

