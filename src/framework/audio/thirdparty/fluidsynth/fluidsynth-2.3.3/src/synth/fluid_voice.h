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


#ifndef _FLUID_VOICE_H
#define _FLUID_VOICE_H

#include "fluid_phase.h"
#include "fluid_gen.h"
#include "fluid_mod.h"
#include "fluid_iir_filter.h"
#include "fluid_adsr_env.h"
#include "fluid_lfo.h"
#include "fluid_rvoice.h"
#include "fluid_rvoice_event.h"

#define NO_CHANNEL             0xff

typedef struct _fluid_overflow_prio_t fluid_overflow_prio_t;

struct _fluid_overflow_prio_t
{
    float percussion; /**< Is this voice on the drum channel? Then add this score */
    float released; /**< Is this voice in release stage? Then add this score (usually negative) */
    float sustained; /**< Is this voice sustained? Then add this score (usually negative) */
    float volume; /**< Multiply current (or future) volume (a value between 0 and 1) */
    float age; /**< This score will be divided by the number of seconds the voice has lasted */
    float important; /**< This score will be added to all important channels */
    char *important_channels; /**< "important" flags indexed by MIDI channel number */
    int num_important_channels; /**< Number of elements in the important_channels array */
};

enum fluid_voice_status
{
    FLUID_VOICE_CLEAN,
    FLUID_VOICE_ON,
    FLUID_VOICE_SUSTAINED,         /* Sustained by Sustain pedal */
    FLUID_VOICE_HELD_BY_SOSTENUTO, /* Sustained by Sostenuto pedal */
    FLUID_VOICE_OFF
};


/*
 * fluid_voice_t
 */
struct _fluid_voice_t
{
    unsigned int id;                /* the id is incremented for every new noteon.
					   it's used for noteoff's  */
    unsigned char status;
    unsigned char chan;             /* the channel number, quick access for channel messages */
    unsigned char key;              /* the key of the noteon event, quick access for noteoff */
    unsigned char vel;              /* the velocity of the noteon event */
    fluid_channel_t *channel;
    fluid_rvoice_eventhandler_t *eventhandler;
    fluid_zone_range_t *zone_range;  /* instrument zone range*/
    fluid_sample_t *sample;          /* Pointer to sample (dupe in rvoice) */
    fluid_sample_t *overflow_sample; /* Pointer to sample (dupe in overflow_rvoice) */

    unsigned int start_time;
    int mod_count;
    fluid_mod_t mod[FLUID_NUM_MOD];
    fluid_gen_t gen[GEN_LAST];

    /* basic parameters */
    fluid_real_t output_rate;        /* the sample rate of the synthesizer (dupe in rvoice) */

    /* basic parameters */
    fluid_real_t pitch;              /* the pitch in midicents (dupe in rvoice) */
    fluid_real_t attenuation;        /* the attenuation in centibels (dupe in rvoice) */
    fluid_real_t root_pitch;

    /* master gain (dupe in rvoice) */
    fluid_real_t synth_gain;

    /* pan */
    fluid_real_t pan;

    /* balance */
    fluid_real_t balance;

    /* reverb */
    fluid_real_t reverb_send;

    /* chorus */
    fluid_real_t chorus_send;

    /* rvoice control */
    fluid_rvoice_t *rvoice;
    fluid_rvoice_t *overflow_rvoice; /* Used temporarily and only in overflow situations */
    char can_access_rvoice; /* False if rvoice is being rendered in separate thread */
    char can_access_overflow_rvoice; /* False if overflow_rvoice is being rendered in separate thread */
    char has_noteoff; /* Flag set when noteoff has been sent */

#ifdef WITH_PROFILING
    /* for debugging */
    double ref;
#endif
};


fluid_voice_t *new_fluid_voice(fluid_rvoice_eventhandler_t *handler, fluid_real_t output_rate);
void delete_fluid_voice(fluid_voice_t *voice);

void fluid_voice_start(fluid_voice_t *voice);
void  fluid_voice_calculate_gen_pitch(fluid_voice_t *voice);

int fluid_voice_init(fluid_voice_t *voice, fluid_sample_t *sample,
                     fluid_zone_range_t *inst_zone_range,
                     fluid_channel_t *channel, int key, int vel,
                     unsigned int id, unsigned int time, fluid_real_t gain);

int fluid_voice_modulate(fluid_voice_t *voice, int cc, int ctrl);
int fluid_voice_modulate_all(fluid_voice_t *voice);

/** Set the NRPN value of a generator. */
int fluid_voice_set_param(fluid_voice_t *voice, int gen, fluid_real_t value);


/** Set the gain. */
int fluid_voice_set_gain(fluid_voice_t *voice, fluid_real_t gain);

void fluid_voice_set_output_rate(fluid_voice_t *voice, fluid_real_t value);


/** Update all the synthesis parameters, which depend on generator
    'gen'. This is only necessary after changing a generator of an
    already operating voice.  Most applications will not need this
    function.*/
void fluid_voice_update_param(fluid_voice_t *voice, int gen);

/** legato modes */
/* force in the attack section for legato mode multi_retrigger: 1 */
void fluid_voice_update_multi_retrigger_attack(fluid_voice_t *voice, int tokey, int vel);
/* Update portamento parameter */
void fluid_voice_update_portamento(fluid_voice_t *voice, int fromkey, int tokey);


void fluid_voice_release(fluid_voice_t *voice);
void fluid_voice_noteoff(fluid_voice_t *voice);
void fluid_voice_off(fluid_voice_t *voice);
void fluid_voice_stop(fluid_voice_t *voice);
void fluid_voice_add_mod_local(fluid_voice_t *voice, fluid_mod_t *mod, int mode, int check_limit_count);
void fluid_voice_overflow_rvoice_finished(fluid_voice_t *voice);

int fluid_voice_kill_excl(fluid_voice_t *voice);
float fluid_voice_get_overflow_prio(fluid_voice_t *voice,
                                    fluid_overflow_prio_t *score,
                                    unsigned int cur_time);

#define OVERFLOW_PRIO_CANNOT_KILL 999999.

/**
 * Locks the rvoice for rendering, so it can't be modified directly
 */
static FLUID_INLINE void
fluid_voice_lock_rvoice(fluid_voice_t *voice)
{
    voice->can_access_rvoice = 0;
}

/**
 * Unlocks the rvoice for rendering, so it can be modified directly
 */
static FLUID_INLINE void
fluid_voice_unlock_rvoice(fluid_voice_t *voice)
{
    voice->can_access_rvoice = 1;
}

#define _AVAILABLE(voice)  ((voice)->can_access_rvoice && \
 (((voice)->status == FLUID_VOICE_CLEAN) || ((voice)->status == FLUID_VOICE_OFF)))
//#define _RELEASED(voice)  ((voice)->chan == NO_CHANNEL)
#define _SAMPLEMODE(voice) ((int)(voice)->gen[GEN_SAMPLEMODE].val)


fluid_real_t fluid_voice_gen_value(const fluid_voice_t *voice, int num);
void fluid_voice_set_custom_filter(fluid_voice_t *voice, enum fluid_iir_filter_type type, enum fluid_iir_filter_flags flags);


#endif /* _FLUID_VOICE_H */
