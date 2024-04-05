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

#include "fluid_sys.h"
#include "fluid_voice.h"
#include "fluid_mod.h"
#include "fluid_chan.h"
#include "fluid_conv.h"
#include "fluid_synth.h"
#include "fluid_sys.h"
#include "fluid_sfont.h"
#include "fluid_rvoice_event.h"
#include "fluid_defsfont.h"

/* used for filter turn off optimization - if filter cutoff is above the
   specified value and filter q is below the other value, turn filter off */
#define FLUID_MAX_AUDIBLE_FILTER_FC 19000.0f
#define FLUID_MIN_AUDIBLE_FILTER_Q 1.2f

/* min vol envelope release (to stop clicks) in SoundFont timecents */
#define FLUID_MIN_VOLENVRELEASE -7200.0f /* ~16ms */


static const int32_t INT24_MAX = (1 << (16 + 8 - 1));

static int fluid_voice_calculate_runtime_synthesis_parameters(fluid_voice_t *voice);
static int calculate_hold_decay_buffers(fluid_voice_t *voice, int gen_base,
                                        int gen_key2base, int is_decay);
static fluid_real_t
fluid_voice_get_lower_boundary_for_attenuation(fluid_voice_t *voice);

#define UPDATE_RVOICE0(proc) \
  do { \
      fluid_rvoice_param_t param[MAX_EVENT_PARAMS]; \
      fluid_rvoice_eventhandler_push(voice->eventhandler, proc, voice->rvoice, param); \
  } while (0)

#define UPDATE_RVOICE_GENERIC_R1(proc, obj, rarg) \
  do { \
      fluid_rvoice_param_t param[MAX_EVENT_PARAMS]; \
      param[0].real = rarg; \
      fluid_rvoice_eventhandler_push(voice->eventhandler, proc, obj, param); \
  } while (0)

#define UPDATE_RVOICE_GENERIC_I1(proc, obj, iarg) \
  do { \
      fluid_rvoice_param_t param[MAX_EVENT_PARAMS]; \
      param[0].i = iarg; \
      fluid_rvoice_eventhandler_push(voice->eventhandler, proc, obj, param); \
  } while (0)

#define UPDATE_RVOICE_GENERIC_I2(proc, obj, iarg1, iarg2) \
  do { \
      fluid_rvoice_param_t param[MAX_EVENT_PARAMS]; \
      param[0].i = iarg1; \
      param[1].i = iarg2; \
      fluid_rvoice_eventhandler_push(voice->eventhandler, proc, obj, param); \
  } while (0)

#define UPDATE_RVOICE_GENERIC_IR(proc, obj, iarg, rarg) \
  do { \
      fluid_rvoice_param_t param[MAX_EVENT_PARAMS]; \
      param[0].i = iarg; \
      param[1].real = rarg; \
      fluid_rvoice_eventhandler_push(voice->eventhandler, proc, obj, param); \
  } while (0)


#define UPDATE_RVOICE_R1(proc, arg1) UPDATE_RVOICE_GENERIC_R1(proc, voice->rvoice, arg1)
#define UPDATE_RVOICE_I1(proc, arg1) UPDATE_RVOICE_GENERIC_I1(proc, voice->rvoice, arg1)

#define UPDATE_RVOICE_BUFFERS_AMP(proc, iarg, rarg) UPDATE_RVOICE_GENERIC_IR(proc, &voice->rvoice->buffers, iarg, rarg)
#define UPDATE_RVOICE_ENVLFO_R1(proc, envp, rarg) UPDATE_RVOICE_GENERIC_R1(proc, &voice->rvoice->envlfo.envp, rarg)
#define UPDATE_RVOICE_ENVLFO_I1(proc, envp, iarg) UPDATE_RVOICE_GENERIC_I1(proc, &voice->rvoice->envlfo.envp, iarg)

static FLUID_INLINE void
fluid_voice_update_volenv(fluid_voice_t *voice,
                          int enqueue,
                          fluid_adsr_env_section_t section,
                          unsigned int count,
                          fluid_real_t coeff,
                          fluid_real_t increment,
                          fluid_real_t min,
                          fluid_real_t max)
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];

    param[0].i = section;
    param[1].i = count;
    param[2].real = coeff;
    param[3].real = increment;
    param[4].real = min;
    param[5].real = max;

    if(enqueue)
    {
        fluid_rvoice_eventhandler_push(voice->eventhandler,
                                       fluid_adsr_env_set_data,
                                       &voice->rvoice->envlfo.volenv,
                                       param);
    }
    else
    {
        fluid_adsr_env_set_data(&voice->rvoice->envlfo.volenv, param);
    }
}

static FLUID_INLINE void
fluid_voice_update_modenv(fluid_voice_t *voice,
                          int enqueue,
                          fluid_adsr_env_section_t section,
                          unsigned int count,
                          fluid_real_t coeff,
                          fluid_real_t increment,
                          fluid_real_t min,
                          fluid_real_t max)
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];

    param[0].i = section;
    param[1].i = count;
    param[2].real = coeff;
    param[3].real = increment;
    param[4].real = min;
    param[5].real = max;

    if(enqueue)
    {
        fluid_rvoice_eventhandler_push(voice->eventhandler,
                                       fluid_adsr_env_set_data,
                                       &voice->rvoice->envlfo.modenv,
                                       param);
    }
    else
    {
        fluid_adsr_env_set_data(&voice->rvoice->envlfo.modenv, param);
    }
}

static FLUID_INLINE void fluid_voice_sample_unref(fluid_sample_t **sample)
{
    if(*sample != NULL)
    {
        fluid_sample_decr_ref(*sample);
        *sample = NULL;
    }
}

/*
 * Swaps the current rvoice with the current overflow_rvoice
 */
static void fluid_voice_swap_rvoice(fluid_voice_t *voice)
{
    fluid_rvoice_t *rtemp = voice->rvoice;
    int ctemp = voice->can_access_rvoice;
    voice->rvoice = voice->overflow_rvoice;
    voice->can_access_rvoice = voice->can_access_overflow_rvoice;
    voice->overflow_rvoice = rtemp;
    voice->can_access_overflow_rvoice = ctemp;
    voice->overflow_sample = voice->sample;
}

static void fluid_voice_initialize_rvoice(fluid_voice_t *voice, fluid_real_t output_rate)
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];

    FLUID_MEMSET(voice->rvoice, 0, sizeof(fluid_rvoice_t));

    /* The 'sustain' and 'finished' segments of the volume / modulation
     * envelope are constant. They are never affected by any modulator
     * or generator. Therefore it is enough to initialize them once
     * during the lifetime of the synth.
     */
    fluid_voice_update_volenv(voice, FALSE, FLUID_VOICE_ENVSUSTAIN,
                              0xffffffff, 1.0f, 0.0f, -1.0f, 2.0f);
    fluid_voice_update_volenv(voice, FALSE, FLUID_VOICE_ENVFINISHED,
                              0xffffffff, 0.0f, 0.0f, -1.0f, 1.0f);
    fluid_voice_update_modenv(voice, FALSE, FLUID_VOICE_ENVSUSTAIN,
                              0xffffffff, 1.0f, 0.0f, -1.0f, 2.0f);
    fluid_voice_update_modenv(voice, FALSE, FLUID_VOICE_ENVFINISHED,
                              0xffffffff, 0.0f, 0.0f, -1.0f, 1.0f);

#ifdef ENABLE_DEFAULT_COMPRESSION
    param[0].i = FLUID_IIR_LOWPASS;
#else
    param[0].i = FLUID_IIR_DISABLED;
#endif
    param[1].i = 0;
    fluid_iir_filter_init(&voice->rvoice->resonant_filter, param);

    param[0].i = FLUID_IIR_DISABLED;
    fluid_iir_filter_init(&voice->rvoice->resonant_custom_filter, param);

    param[0].real = output_rate;
    fluid_rvoice_set_output_rate(voice->rvoice, param);
}

/*
 * new_fluid_voice
 */
fluid_voice_t *
new_fluid_voice(fluid_rvoice_eventhandler_t *handler, fluid_real_t output_rate)
{
    fluid_voice_t *voice;
    voice = FLUID_NEW(fluid_voice_t);

    if(voice == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    voice->can_access_rvoice = TRUE;
    voice->can_access_overflow_rvoice = TRUE;

    voice->rvoice = FLUID_NEW(fluid_rvoice_t);
    voice->overflow_rvoice = FLUID_NEW(fluid_rvoice_t);

    if(voice->rvoice == NULL || voice->overflow_rvoice == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        delete_fluid_voice(voice);
        return NULL;
    }

    voice->status = FLUID_VOICE_CLEAN;
    voice->chan = NO_CHANNEL;
    voice->key = 0;
    voice->vel = 0;
    voice->eventhandler = handler;
    voice->channel = NULL;
    voice->sample = NULL;
    voice->overflow_sample = NULL;
    voice->output_rate = output_rate;

    /* Initialize both the rvoice and overflow_rvoice */
    fluid_voice_initialize_rvoice(voice, output_rate);
    fluid_voice_swap_rvoice(voice);
    fluid_voice_initialize_rvoice(voice, output_rate);

    return voice;
}

/*
 * delete_fluid_voice
 */
void
delete_fluid_voice(fluid_voice_t *voice)
{
    fluid_return_if_fail(voice != NULL);

    if(!voice->can_access_rvoice || !voice->can_access_overflow_rvoice)
    {
        FLUID_LOG(FLUID_WARN, "Deleting voice %u which has locked rvoices!", voice->id);
    }

    FLUID_FREE(voice->overflow_rvoice);
    FLUID_FREE(voice->rvoice);
    FLUID_FREE(voice);
}

/* fluid_voice_init
 *
 * Initialize the synthesis process
 * inst_zone, the Instrument Zone contains the sample, Keyrange,Velrange
 * of the voice.
 * When playing legato (n1,n2) in mono mode, n2 will use n1 voices
 * as far as n2 still enters in Keyrange,Velrange of n1.
 */
int
fluid_voice_init(fluid_voice_t *voice, fluid_sample_t *sample,
                 fluid_zone_range_t *inst_zone_range,
                 fluid_channel_t *channel, int key, int vel, unsigned int id,
                 unsigned int start_time, fluid_real_t gain)
{
    /* Note: The voice parameters will be initialized later, when the
     * generators have been retrieved from the sound font. Here, only
     * the 'working memory' of the voice (position in envelopes, history
     * of IIR filters, position in sample etc) is initialized. */
    int i;

    if(!voice->can_access_rvoice)
    {
        if(voice->can_access_overflow_rvoice)
        {
            fluid_voice_swap_rvoice(voice);
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Internal error: Cannot access an rvoice in fluid_voice_init!");
            return FLUID_FAILED;
        }
    }

    /* We are now guaranteed to have access to the rvoice */

    if(voice->sample)
    {
        fluid_voice_off(voice);
    }

    voice->zone_range = inst_zone_range; /* Instrument zone range for legato */
    voice->id = id;
    voice->chan = fluid_channel_get_num(channel);
    voice->key = (unsigned char) key;
    voice->vel = (unsigned char) vel;
    voice->channel = channel;
    voice->mod_count = 0;
    voice->start_time = start_time;
    voice->has_noteoff = 0;
    UPDATE_RVOICE0(fluid_rvoice_reset);

    /*
       We increment the reference count of the sample to indicate that this
       sample is about to be owned by the rvoice. This will prevent the
       unloading of the soundfont while this rvoice is playing.
    */
    fluid_sample_incr_ref(sample);
    fluid_rvoice_eventhandler_push_ptr(voice->eventhandler, fluid_rvoice_set_sample, voice->rvoice, sample);
    voice->sample = sample;

    i = fluid_channel_get_interp_method(channel);
    UPDATE_RVOICE_I1(fluid_rvoice_set_interp_method, i);

    /* Set all the generators to their default value, according to SF
     * 2.01 section 8.1.3 (page 48). The value of NRPN messages are
     * copied from the channel to the voice's generators. The sound font
     * loader overwrites them. The generator values are later converted
     * into voice parameters in
     * fluid_voice_calculate_runtime_synthesis_parameters.  */
    fluid_gen_init(&voice->gen[0], channel);
    UPDATE_RVOICE_I1(fluid_rvoice_set_samplemode, _SAMPLEMODE(voice));

    voice->synth_gain = gain;

    /* avoid division by zero later*/
    if(voice->synth_gain < 0.0000001f)
    {
        voice->synth_gain = 0.0000001f;
    }

    UPDATE_RVOICE_R1(fluid_rvoice_set_synth_gain, voice->synth_gain);

    /* Set up buffer mapping, should be done more flexible in the future. */
    i = 2 * channel->synth->audio_groups;
    i += (voice->chan % channel->synth->effects_groups) * channel->synth->effects_channels;
    UPDATE_RVOICE_GENERIC_I2(fluid_rvoice_buffers_set_mapping, &voice->rvoice->buffers, 2, i + SYNTH_REVERB_CHANNEL);
    UPDATE_RVOICE_GENERIC_I2(fluid_rvoice_buffers_set_mapping, &voice->rvoice->buffers, 3, i + SYNTH_CHORUS_CHANNEL);

    i = 2 * (voice->chan % channel->synth->audio_groups);
    UPDATE_RVOICE_GENERIC_I2(fluid_rvoice_buffers_set_mapping, &voice->rvoice->buffers, 0, i);
    UPDATE_RVOICE_GENERIC_I2(fluid_rvoice_buffers_set_mapping, &voice->rvoice->buffers, 1, i + 1);

    return FLUID_OK;
}


/**
 * Update sample rate.
 *
 * @note If the voice is active, it will be turned off.
 */
void
fluid_voice_set_output_rate(fluid_voice_t *voice, fluid_real_t value)
{
    if(fluid_voice_is_playing(voice))
    {
        fluid_voice_off(voice);
    }

    voice->output_rate = value;
    UPDATE_RVOICE_GENERIC_R1(fluid_rvoice_set_output_rate, voice->rvoice, value);
    UPDATE_RVOICE_GENERIC_R1(fluid_rvoice_set_output_rate, voice->overflow_rvoice, value);
}


/**
 * Set the value of a generator.
 *
 * @param voice Voice instance
 * @param i Generator ID (#fluid_gen_type)
 * @param val Generator value
 */
void
fluid_voice_gen_set(fluid_voice_t *voice, int i, float val)
{
    voice->gen[i].val = val;
    voice->gen[i].flags = GEN_SET;

    if(i == GEN_SAMPLEMODE)
    {
        UPDATE_RVOICE_I1(fluid_rvoice_set_samplemode, (int) val);
    }
}

/**
 * Offset the value of a generator.
 *
 * @param voice Voice instance
 * @param i Generator ID (#fluid_gen_type)
 * @param val Value to add to the existing value
 */
void
fluid_voice_gen_incr(fluid_voice_t *voice, int i, float val)
{
    voice->gen[i].val += val;
    voice->gen[i].flags = GEN_SET;
}

/**
 * Get the value of a generator.
 *
 * @param voice Voice instance
 * @param gen Generator ID (#fluid_gen_type)
 * @return Current generator value
 */
float
fluid_voice_gen_get(fluid_voice_t *voice, int gen)
{
    return voice->gen[gen].val;
}

fluid_real_t fluid_voice_gen_value(const fluid_voice_t *voice, int num)
{
    return (fluid_real_t)(voice->gen[num].val + voice->gen[num].mod + voice->gen[num].nrpn);
}

/*
 * fluid_voice_start
 */
void fluid_voice_start(fluid_voice_t *voice)
{
    /* The maximum volume of the loop is calculated and cached once for each
     * sample with its nominal loop settings. This happens, when the sample is used
     * for the first time.*/

    fluid_voice_calculate_runtime_synthesis_parameters(voice);

#ifdef WITH_PROFILING
    voice->ref = fluid_profile_ref();
#endif

    voice->status = FLUID_VOICE_ON;

    /* Increment voice count */
    voice->channel->synth->active_voice_count++;
}

/**
 * Calculate the amplitude of a voice.
 *
 * @param gain The gain value in the range [0.0 ; 1.0]
 * @return An amplitude used by rvoice_mixer's buffers
 */
static FLUID_INLINE fluid_real_t
fluid_voice_calculate_gain_amplitude(const fluid_voice_t *voice, fluid_real_t gain)
{
    /* we use 24bit samples in fluid_rvoice_dsp. in order to normalize float
     * samples to [0.0;1.0] divide samples by the max. value of an int24 and
     * amplify them with the gain */
    return gain * voice->synth_gain / (INT24_MAX * 1.0f);
}

/* Useful to return the nominal pitch of a key */
/* The nominal pitch is dependent of voice->root_pitch,tuning, and
   GEN_SCALETUNE generator.
   This is useful to set the value of GEN_PITCH generator on noteOn.
   This is useful to get the beginning/ending pitch for portamento.
*/
fluid_real_t fluid_voice_calculate_pitch(fluid_voice_t *voice, int key)
{
    fluid_tuning_t *tuning;
    fluid_real_t x, pitch;

    /* Now the nominal pitch of the key is returned.
     * Note about SCALETUNE: SF2.01 8.1.3 says, that this generator is a
     * non-realtime parameter. So we don't allow modulation (as opposed
     * to fluid_voice_gen_value(voice, GEN_SCALETUNE) When the scale tuning is varied,
     * one key remains fixed. Here C3 (MIDI number 60) is used.
     */
    if(fluid_channel_has_tuning(voice->channel))
    {
        tuning = fluid_channel_get_tuning(voice->channel);
        x = fluid_tuning_get_pitch(tuning, (int)(voice->root_pitch / 100.0f));
        pitch = voice->gen[GEN_SCALETUNE].val / 100.0f *
                (fluid_tuning_get_pitch(tuning, key) - x) + x;
    }
    else
    {
        pitch = voice->gen[GEN_SCALETUNE].val
                * (key - voice->root_pitch / 100.0f) + voice->root_pitch;
    }

    return pitch;
}

void
fluid_voice_calculate_gen_pitch(fluid_voice_t *voice)
{
    voice->gen[GEN_PITCH].val = fluid_voice_calculate_pitch(voice, fluid_voice_get_actual_key(voice));
}

/*
 * fluid_voice_calculate_runtime_synthesis_parameters
 *
 * in this function we calculate the values of all the parameters. the
 * parameters are converted to their most useful unit for the DSP
 * algorithm, for example, number of samples instead of
 * timecents. Some parameters keep their "perceptual" unit and
 * conversion will be done in the DSP function. This is the case, for
 * example, for the pitch since it is modulated by the controllers in
 * cents. */
static int
fluid_voice_calculate_runtime_synthesis_parameters(fluid_voice_t *voice)
{
    int i;
    unsigned int n;

    static int const list_of_generators_to_initialize[] =
    {
        GEN_STARTADDROFS,                    /* SF2.01 page 48 #0   */
        GEN_ENDADDROFS,                      /*                #1   */
        GEN_STARTLOOPADDROFS,                /*                #2   */
        GEN_ENDLOOPADDROFS,                  /*                #3   */
        /* GEN_STARTADDRCOARSEOFS see comment below [1]        #4   */
        GEN_MODLFOTOPITCH,                   /*                #5   */
        GEN_VIBLFOTOPITCH,                   /*                #6   */
        GEN_MODENVTOPITCH,                   /*                #7   */
        GEN_FILTERFC,                        /*                #8   */
        GEN_FILTERQ,                         /*                #9   */
        GEN_MODLFOTOFILTERFC,                /*                #10  */
        GEN_MODENVTOFILTERFC,                /*                #11  */
        /* GEN_ENDADDRCOARSEOFS [1]                            #12  */
        GEN_MODLFOTOVOL,                     /*                #13  */
        /* not defined                                         #14  */
        GEN_CHORUSSEND,                      /*                #15  */
        GEN_REVERBSEND,                      /*                #16  */
        GEN_PAN,                             /*                #17  */
        /* not defined                                         #18  */
        /* not defined                                         #19  */
        /* not defined                                         #20  */
        GEN_MODLFODELAY,                     /*                #21  */
        GEN_MODLFOFREQ,                      /*                #22  */
        GEN_VIBLFODELAY,                     /*                #23  */
        GEN_VIBLFOFREQ,                      /*                #24  */
        GEN_MODENVDELAY,                     /*                #25  */
        GEN_MODENVATTACK,                    /*                #26  */
        GEN_MODENVHOLD,                      /*                #27  */
        GEN_MODENVDECAY,                     /*                #28  */
        /* GEN_MODENVSUSTAIN [1]                               #29  */
        GEN_MODENVRELEASE,                   /*                #30  */
        /* GEN_KEYTOMODENVHOLD [1]                             #31  */
        /* GEN_KEYTOMODENVDECAY [1]                            #32  */
        GEN_VOLENVDELAY,                     /*                #33  */
        GEN_VOLENVATTACK,                    /*                #34  */
        GEN_VOLENVHOLD,                      /*                #35  */
        GEN_VOLENVDECAY,                     /*                #36  */
        /* GEN_VOLENVSUSTAIN [1]                               #37  */
        GEN_VOLENVRELEASE,                   /*                #38  */
        /* GEN_KEYTOVOLENVHOLD [1]                             #39  */
        /* GEN_KEYTOVOLENVDECAY [1]                            #40  */
        /* GEN_STARTLOOPADDRCOARSEOFS [1]                      #45  */
        GEN_KEYNUM,                          /*                #46  */
        GEN_VELOCITY,                        /*                #47  */
        GEN_ATTENUATION,                     /*                #48  */
        /* GEN_ENDLOOPADDRCOARSEOFS [1]                        #50  */
        /* GEN_COARSETUNE           [1]                        #51  */
        /* GEN_FINETUNE             [1]                        #52  */
        GEN_OVERRIDEROOTKEY,                 /*                #58  */
        GEN_PITCH,                           /*                ---  */
        GEN_CUSTOM_BALANCE,                  /*                ---  */
        GEN_CUSTOM_FILTERFC,                 /*                ---  */
        GEN_CUSTOM_FILTERQ                   /*                ---  */
    };

    /* When the voice is made ready for the synthesis process, a lot of
     * voice-internal parameters have to be calculated.
     *
     * At this point, the sound font has already set the -nominal- value
     * for all generators (excluding GEN_PITCH). Most generators can be
     * modulated - they include a nominal value and an offset (which
     * changes with velocity, note number, channel parameters like
     * aftertouch, mod wheel...) Now this offset will be calculated as
     * follows:
     *
     *  - Process each modulator once.
     *  - Calculate its output value.
     *  - Find the target generator.
     *  - Add the output value to the modulation value of the generator.
     *
     * Note: The generators have been initialized with
     * fluid_gen_init().
     */

    for(i = 0; i < voice->mod_count; i++)
    {
        fluid_mod_t *mod = &voice->mod[i];
        fluid_real_t modval = fluid_mod_get_value(mod, voice);
        int dest_gen_index = mod->dest;
        fluid_gen_t *dest_gen = &voice->gen[dest_gen_index];
        dest_gen->mod += modval;
        /*      fluid_dump_modulator(mod); */
    }

    /* Now the generators are initialized, nominal and modulation value.
     * The voice parameters (which depend on generators) are calculated
     * with fluid_voice_update_param. Processing the list of generator
     * changes will calculate each voice parameter once.
     *
     * Note [1]: Some voice parameters depend on several generators. For
     * example, the pitch depends on GEN_COARSETUNE, GEN_FINETUNE and
     * GEN_PITCH.  voice->pitch.  Unnecessary recalculation is avoided
     * by removing all but one generator from the list of voice
     * parameters.  Same with GEN_XXX and GEN_XXXCOARSE: the
     * initialisation list contains only GEN_XXX.
     */

    /* Calculate the voice parameter(s) dependent on each generator. */
    for(n = 0; n < FLUID_N_ELEMENTS(list_of_generators_to_initialize); n++)
    {
        fluid_voice_update_param(voice, list_of_generators_to_initialize[n]);
    }

    /* Start portamento if enabled */
    {
        /* fromkey note comes from "GetFromKeyPortamentoLegato()" detector.
        When fromkey is set to ValidNote , portamento is started */
        /* Return fromkey portamento */
        int fromkey = voice->channel->synth->fromkey_portamento;

        if(fluid_channel_is_valid_note(fromkey))
        {
            /* Send portamento parameters to the voice dsp */
            fluid_voice_update_portamento(voice, fromkey, fluid_voice_get_actual_key(voice));
        }
    }

    /* Make an estimate on how loud this voice can get at any time (attenuation). */
    UPDATE_RVOICE_R1(fluid_rvoice_set_min_attenuation_cB,
                     fluid_voice_get_lower_boundary_for_attenuation(voice));
    return FLUID_OK;
}

/*
 * calculate_hold_decay_buffers
 */
static int
calculate_hold_decay_buffers(fluid_voice_t *voice, int gen_base,
                             int gen_key2base, int is_decay)
{
    /* Purpose:
     *
     * Returns the number of DSP loops, that correspond to the hold
     * (is_decay=0) or decay (is_decay=1) time.
     * gen_base=GEN_VOLENVHOLD, GEN_VOLENVDECAY, GEN_MODENVHOLD,
     * GEN_MODENVDECAY gen_key2base=GEN_KEYTOVOLENVHOLD,
     * GEN_KEYTOVOLENVDECAY, GEN_KEYTOMODENVHOLD, GEN_KEYTOMODENVDECAY
     */

    fluid_real_t keysteps;
    fluid_real_t timecents;
    fluid_real_t seconds;
    int buffers;

    /* SF2.01 section 8.4.3 # 31, 32, 39, 40
     * GEN_KEYTOxxxENVxxx uses key 60 as 'origin'.
     * The unit of the generator is timecents per key number.
     * If KEYTOxxxENVxxx is 100, a key one octave over key 60 (72)
     * will cause (60-72)*100=-1200 timecents of time variation.
     * The time is cut in half.
     */

    keysteps = 60.0f - fluid_channel_get_key_pitch(voice->channel, fluid_voice_get_actual_key(voice)) / 100.0f;
    timecents = fluid_voice_gen_value(voice, gen_base) + fluid_voice_gen_value(voice, gen_key2base) * keysteps;

    /* Range checking */
    if(is_decay)
    {
        /* SF 2.01 section 8.1.3 # 28, 36 */
        if(timecents > 8000.f)
        {
            timecents = 8000.f;
        }
    }
    else
    {
        /* SF 2.01 section 8.1.3 # 27, 35 */
        if(timecents > 5000.f)
        {
            timecents = 5000.f;
        }

        /* SF 2.01 section 8.1.2 # 27, 35:
         * The most negative number indicates no hold time
         */
        if(timecents <= -32768.f)
        {
            return 0;
        }
    }

    /* SF 2.01 section 8.1.3 # 27, 28, 35, 36 */
    if(timecents < -12000.f)
    {
        timecents = -12000.f;
    }

    seconds = fluid_tc2sec(timecents);
    /* Each DSP loop processes FLUID_BUFSIZE samples. */

    /* round to next full number of buffers */
    buffers = (int)(((fluid_real_t)voice->output_rate * seconds)
                    / (fluid_real_t)FLUID_BUFSIZE
                    + 0.5f);

    return buffers;
}

/*
 * The value of a generator (gen) has changed.  (The different
 * generators are listed in fluidsynth.h, or in SF2.01 page 48-49)
 * Now the dependent 'voice' parameters are calculated.
 *
 * fluid_voice_update_param can be called during the setup of the
 * voice (to calculate the initial value for a voice parameter), or
 * during its operation (a generator has been changed due to
 * real-time parameter modifications like pitch-bend).
 *
 * Note: The generator holds three values: The base value .val, an
 * offset caused by modulators .mod, and an offset caused by the
 * NRPN system. fluid_voice_gen_value(voice, generator_enumerator) returns the sum
 * of all three.
 */

/**
 * Update all the synthesis parameters which depend on generator \a gen.
 *
 * @param voice Voice instance
 * @param gen Generator id (#fluid_gen_type)
 *
 * Calling this function is only necessary after changing a generator of an already playing voice.
 */
void
fluid_voice_update_param(fluid_voice_t *voice, int gen)
{
    unsigned int count, z;
    fluid_real_t x = fluid_voice_gen_value(voice, gen);

    switch(gen)
    {

    case GEN_PAN:
    case GEN_CUSTOM_BALANCE:
        /* range checking is done in the fluid_pan and fluid_balance functions */
        voice->pan = fluid_voice_gen_value(voice, GEN_PAN);
        voice->balance = fluid_voice_gen_value(voice, GEN_CUSTOM_BALANCE);

        /* left amp */
        UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 0,
                                  fluid_voice_calculate_gain_amplitude(voice,
                                          fluid_pan(voice->pan, 1) * fluid_balance(voice->balance, 1)));

        /* right amp */
        UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 1,
                                  fluid_voice_calculate_gain_amplitude(voice,
                                          fluid_pan(voice->pan, 0) * fluid_balance(voice->balance, 0)));
        break;

    case GEN_ATTENUATION:
        voice->attenuation = x;

        /* Range: SF2.01 section 8.1.3 # 48
         * Motivation for range checking:
         * OHPiano.SF2 sets initial attenuation to a whooping -96 dB */
        fluid_clip(voice->attenuation, 0.f, 1440.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_attenuation, voice->attenuation);
        break;

    /* The pitch is calculated from three different generators.
     * Read comment in fluidsynth.h about GEN_PITCH.
     */
    case GEN_PITCH:
    case GEN_COARSETUNE:
    case GEN_FINETUNE:
        /* The testing for allowed range is done in 'fluid_ct2hz' */
        voice->pitch = (fluid_voice_gen_value(voice, GEN_PITCH)
                        + 100.0f * fluid_voice_gen_value(voice, GEN_COARSETUNE)
                        + fluid_voice_gen_value(voice, GEN_FINETUNE));
        UPDATE_RVOICE_R1(fluid_rvoice_set_pitch, voice->pitch);
        break;

    case GEN_REVERBSEND:
        /* The generator unit is 'tenths of a percent'. */
        voice->reverb_send = x / 1000.0f;
        fluid_clip(voice->reverb_send, 0.f, 1.f);
        UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 2, fluid_voice_calculate_gain_amplitude(voice, voice->reverb_send));
        break;

    case GEN_CHORUSSEND:
        /* The generator unit is 'tenths of a percent'. */
        voice->chorus_send = x / 1000.0f;
        fluid_clip(voice->chorus_send, 0.f, 1.f);
        UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 3, fluid_voice_calculate_gain_amplitude(voice, voice->chorus_send));
        break;

    case GEN_OVERRIDEROOTKEY:

        /* This is a non-realtime parameter. Therefore the .mod part of the generator
         * can be neglected.
         * NOTE: origpitch sets MIDI root note while pitchadj is a fine tuning amount
         * which offsets the original rate.  This means that the fine tuning is
         * inverted with respect to the root note (so subtract it, not add).
         */
        if(voice->sample != NULL)
        {
            if(voice->gen[GEN_OVERRIDEROOTKEY].val > -1)    //FIXME: use flag instead of -1
            {
                voice->root_pitch = voice->gen[GEN_OVERRIDEROOTKEY].val * 100.0f
                                    - voice->sample->pitchadj;
            }
            else
            {
                voice->root_pitch = voice->sample->origpitch * 100.0f - voice->sample->pitchadj;
            }

            x = (fluid_ct2hz_real(voice->root_pitch) * ((fluid_real_t) voice->output_rate / voice->sample->samplerate));
        }
        else
        {
            if(voice->gen[GEN_OVERRIDEROOTKEY].val > -1)     //FIXME: use flag instead of -1
            {
                voice->root_pitch = voice->gen[GEN_OVERRIDEROOTKEY].val * 100.0f;
            }
            else
            {
                voice->root_pitch = 0;
            }

            x = fluid_ct2hz_real(voice->root_pitch);
        }

        /* voice->pitch depends on voice->root_pitch, so calculate voice->pitch now */
        fluid_voice_calculate_gen_pitch(voice);
        UPDATE_RVOICE_R1(fluid_rvoice_set_root_pitch_hz, x);

        break;

    case GEN_FILTERFC:
        /* The resonance frequency is converted from absolute cents to
         * midicents .val and .mod are both used, this permits real-time
         * modulation.  The allowed range is tested in the 'fluid_ct2hz'
         * function [PH,20021214]
         */
        UPDATE_RVOICE_GENERIC_R1(fluid_iir_filter_set_fres, &voice->rvoice->resonant_filter, x);
        break;

    case GEN_FILTERQ:
        UPDATE_RVOICE_GENERIC_R1(fluid_iir_filter_set_q, &voice->rvoice->resonant_filter, x);
        break;

    /* same as the two above, only for the custom filter */
    case GEN_CUSTOM_FILTERFC:
        UPDATE_RVOICE_GENERIC_R1(fluid_iir_filter_set_fres, &voice->rvoice->resonant_custom_filter, x);
        break;

    case GEN_CUSTOM_FILTERQ:
        UPDATE_RVOICE_GENERIC_R1(fluid_iir_filter_set_q, &voice->rvoice->resonant_custom_filter, x);
        break;

    case GEN_MODLFOTOPITCH:
        fluid_clip(x, -12000.f, 12000.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_pitch, x);
        break;

    case GEN_MODLFOTOVOL:
        fluid_clip(x, -960.f, 960.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_vol, x);
        break;

    case GEN_MODLFOTOFILTERFC:
        fluid_clip(x, -12000.f, 12000.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_modlfo_to_fc, x);
        break;

    case GEN_MODLFODELAY:
        fluid_clip(x, -12000.0f, 5000.0f);
        z = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
        UPDATE_RVOICE_ENVLFO_I1(fluid_lfo_set_delay, modlfo, z);
        break;

    case GEN_MODLFOFREQ:
        /* - the frequency is converted into a delta value, per buffer of FLUID_BUFSIZE samples
         * - the delay into a sample delay
         */
        fluid_clip(x, -16000.0f, 4500.0f);
        x = (4.0f * FLUID_BUFSIZE * fluid_ct2hz_real(x) / voice->output_rate);
        UPDATE_RVOICE_ENVLFO_R1(fluid_lfo_set_incr, modlfo, x);
        break;

    case GEN_VIBLFOFREQ:
        /* vib lfo
         *
         * - the frequency is converted into a delta value, per buffer of FLUID_BUFSIZE samples
         * - the delay into a sample delay
         */
        fluid_clip(x, -16000.0f, 4500.0f);
        x = 4.0f * FLUID_BUFSIZE * fluid_ct2hz_real(x) / voice->output_rate;
        UPDATE_RVOICE_ENVLFO_R1(fluid_lfo_set_incr, viblfo, x);
        break;

    case GEN_VIBLFODELAY:
        fluid_clip(x, -12000.0f, 5000.0f);
        z = (unsigned int)(voice->output_rate * fluid_tc2sec_delay(x));
        UPDATE_RVOICE_ENVLFO_I1(fluid_lfo_set_delay, viblfo, z);
        break;

    case GEN_VIBLFOTOPITCH:
        fluid_clip(x, -12000.f, 12000.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_viblfo_to_pitch, x);
        break;

    case GEN_KEYNUM:
        /* GEN_KEYNUM: SF2.01 page 46, item 46
         *
         * If this generator is active, it forces the key number to its
         * value.  Non-realtime controller.
         *
         * There is a flag, which should indicate, whether a generator is
         * enabled or not.  But here we rely on the default value of -1.
         */

        /* 2017-09-02: do not change the voice's key here, otherwise it will
         * never be released on a noteoff event
         */
#if 0
        x = fluid_voice_gen_value(voice, GEN_KEYNUM);

        if(x >= 0)
        {
            voice->key = x;
        }

#endif
        break;

    case GEN_VELOCITY:
        /* GEN_VELOCITY: SF2.01 page 46, item 47
         *
         * If this generator is active, it forces the velocity to its
         * value. Non-realtime controller.
         *
         * There is a flag, which should indicate, whether a generator is
         * enabled or not. But here we rely on the default value of -1.
         */
        /* 2017-09-02: do not change the voice's velocity here, use
         * fluid_voice_get_actual_velocity() to get the value of this generator
         * if active.
         */
#if 0
        x = fluid_voice_gen_value(voice, GEN_VELOCITY);

        if(x > 0)
        {
            voice->vel = x;
        }

#endif
        break;

    case GEN_MODENVTOPITCH:
        fluid_clip(x, -12000.f, 12000.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_modenv_to_pitch, x);
        break;

    case GEN_MODENVTOFILTERFC:
        /* Range: SF2.01 section 8.1.3 # 1
         * Motivation for range checking:
         * Filter is reported to make funny noises now and then
         */
        fluid_clip(x, -12000.f, 12000.f);
        UPDATE_RVOICE_R1(fluid_rvoice_set_modenv_to_fc, x);
        break;


    /* sample start and ends points
     *
     * Range checking is initiated via the
     * voice->check_sample_sanity flag,
     * because it is impossible to check here:
     * During the voice setup, all modulators are processed, while
     * the voice is inactive. Therefore, illegal settings may
     * occur during the setup (for example: First move the loop
     * end point ahead of the loop start point => invalid, then
     * move the loop start point forward => valid again.
     */
    case GEN_STARTADDROFS:              /* SF2.01 section 8.1.3 # 0 */
    case GEN_STARTADDRCOARSEOFS:        /* SF2.01 section 8.1.3 # 4 */
        if(voice->sample != NULL)
        {
            fluid_real_t start_fine = fluid_voice_gen_value(voice, GEN_STARTADDROFS);
            fluid_real_t start_coar = fluid_voice_gen_value(voice, GEN_STARTADDRCOARSEOFS);

            z = voice->sample->start + (int)start_fine + 32768 * (int)start_coar;
            UPDATE_RVOICE_I1(fluid_rvoice_set_start, z);
        }

        break;

    case GEN_ENDADDROFS:                 /* SF2.01 section 8.1.3 # 1 */
    case GEN_ENDADDRCOARSEOFS:           /* SF2.01 section 8.1.3 # 12 */
        if(voice->sample != NULL)
        {
            fluid_real_t end_fine = fluid_voice_gen_value(voice, GEN_ENDADDROFS);
            fluid_real_t end_coar = fluid_voice_gen_value(voice, GEN_ENDADDRCOARSEOFS);

            z = voice->sample->end + (int)end_fine + 32768 * (int)end_coar;
            UPDATE_RVOICE_I1(fluid_rvoice_set_end, z);
        }

        break;

    case GEN_STARTLOOPADDROFS:           /* SF2.01 section 8.1.3 # 2 */
    case GEN_STARTLOOPADDRCOARSEOFS:     /* SF2.01 section 8.1.3 # 45 */
        if(voice->sample != NULL)
        {
            fluid_real_t lstart_fine = fluid_voice_gen_value(voice, GEN_STARTLOOPADDROFS);
            fluid_real_t lstart_coar = fluid_voice_gen_value(voice, GEN_STARTLOOPADDRCOARSEOFS);

            z = voice->sample->loopstart + (int)lstart_fine + 32768 * (int)lstart_coar;
            UPDATE_RVOICE_I1(fluid_rvoice_set_loopstart, z);
        }

        break;

    case GEN_ENDLOOPADDROFS:             /* SF2.01 section 8.1.3 # 3 */
    case GEN_ENDLOOPADDRCOARSEOFS:       /* SF2.01 section 8.1.3 # 50 */
        if(voice->sample != NULL)
        {
            fluid_real_t lend_fine = fluid_voice_gen_value(voice, GEN_ENDLOOPADDROFS);
            fluid_real_t lend_coar = fluid_voice_gen_value(voice, GEN_ENDLOOPADDRCOARSEOFS);

            z = voice->sample->loopend + (int)lend_fine + 32768 * (int)lend_coar;
            UPDATE_RVOICE_I1(fluid_rvoice_set_loopend, z);
        }

        break;

        /* Conversion functions differ in range limit */
#define NUM_BUFFERS_DELAY(_v)   (unsigned int) (voice->output_rate * fluid_tc2sec_delay(_v) / FLUID_BUFSIZE)
#define NUM_BUFFERS_ATTACK(_v)  (unsigned int) (voice->output_rate * fluid_tc2sec_attack(_v) / FLUID_BUFSIZE)
#define NUM_BUFFERS_RELEASE(_v) (unsigned int) (voice->output_rate * fluid_tc2sec_release(_v) / FLUID_BUFSIZE)

    /* volume envelope
     *
     * - delay and hold times are converted to absolute number of samples
     * - sustain is converted to its absolute value
     * - attack, decay and release are converted to their increment per sample
     */
    case GEN_VOLENVDELAY:                /* SF2.01 section 8.1.3 # 33 */
        fluid_clip(x, -12000.0f, 5000.0f);
        count = NUM_BUFFERS_DELAY(x);
        fluid_voice_update_volenv(voice, TRUE, FLUID_VOICE_ENVDELAY,
                                  count, 0.0f, 0.0f, -1.0f, 1.0f);
        break;

    case GEN_VOLENVATTACK:               /* SF2.01 section 8.1.3 # 34 */
        fluid_clip(x, -12000.0f, 8000.0f);
        count = 1 + NUM_BUFFERS_ATTACK(x);
        fluid_voice_update_volenv(voice, TRUE, FLUID_VOICE_ENVATTACK,
                                  count, 1.0f, 1.0f / count, -1.0f, 1.0f);
        break;

    case GEN_VOLENVHOLD:                 /* SF2.01 section 8.1.3 # 35 */
    case GEN_KEYTOVOLENVHOLD:            /* SF2.01 section 8.1.3 # 39 */
        count = calculate_hold_decay_buffers(voice, GEN_VOLENVHOLD, GEN_KEYTOVOLENVHOLD, 0); /* 0 means: hold */
        fluid_voice_update_volenv(voice, TRUE, FLUID_VOICE_ENVHOLD,
                                  count, 1.0f, 0.0f, -1.0f, 2.0f);
        break;

    case GEN_VOLENVDECAY:               /* SF2.01 section 8.1.3 # 36 */
    case GEN_VOLENVSUSTAIN:             /* SF2.01 section 8.1.3 # 37 */
    case GEN_KEYTOVOLENVDECAY:          /* SF2.01 section 8.1.3 # 40 */
        x = 1.0f - 0.001f * fluid_voice_gen_value(voice, GEN_VOLENVSUSTAIN);
        fluid_clip(x, 0.0f, 1.0f);
        count = calculate_hold_decay_buffers(voice, GEN_VOLENVDECAY, GEN_KEYTOVOLENVDECAY, 1); /* 1 for decay */
        fluid_voice_update_volenv(voice, TRUE, FLUID_VOICE_ENVDECAY,
                                  count, 1.0f, count ? -1.0f / count : 0.0f, x, 2.0f);
        break;

    case GEN_VOLENVRELEASE:             /* SF2.01 section 8.1.3 # 38 */
        fluid_clip(x, FLUID_MIN_VOLENVRELEASE, 8000.0f);
        count = 1 + NUM_BUFFERS_RELEASE(x);
        fluid_voice_update_volenv(voice, TRUE, FLUID_VOICE_ENVRELEASE,
                                  count, 1.0f, -1.0f / count, 0.0f, 1.0f);
        break;

    /* Modulation envelope */
    case GEN_MODENVDELAY:               /* SF2.01 section 8.1.3 # 25 */
        fluid_clip(x, -12000.0f, 5000.0f);
        count = NUM_BUFFERS_DELAY(x);
        fluid_voice_update_modenv(voice, TRUE, FLUID_VOICE_ENVDELAY,
                                  count, 0.0f, 0.0f, -1.0f, 1.0f);
        break;

    case GEN_MODENVATTACK:               /* SF2.01 section 8.1.3 # 26 */
        fluid_clip(x, -12000.0f, 8000.0f);
        count = 1 + NUM_BUFFERS_ATTACK(x);
        fluid_voice_update_modenv(voice, TRUE, FLUID_VOICE_ENVATTACK,
                                  count, 1.0f, 1.0f / count, -1.0f, 1.0f);
        break;

    case GEN_MODENVHOLD:               /* SF2.01 section 8.1.3 # 27 */
    case GEN_KEYTOMODENVHOLD:          /* SF2.01 section 8.1.3 # 31 */
        count = calculate_hold_decay_buffers(voice, GEN_MODENVHOLD, GEN_KEYTOMODENVHOLD, 0); /* 1 means: hold */
        fluid_voice_update_modenv(voice, TRUE, FLUID_VOICE_ENVHOLD,
                                  count, 1.0f, 0.0f, -1.0f, 2.0f);
        break;

    case GEN_MODENVDECAY:                                   /* SF 2.01 section 8.1.3 # 28 */
    case GEN_MODENVSUSTAIN:                                 /* SF 2.01 section 8.1.3 # 29 */
    case GEN_KEYTOMODENVDECAY:                              /* SF 2.01 section 8.1.3 # 32 */
        count = calculate_hold_decay_buffers(voice, GEN_MODENVDECAY, GEN_KEYTOMODENVDECAY, 1); /* 1 for decay */
        x = 1.0f - 0.001f * fluid_voice_gen_value(voice, GEN_MODENVSUSTAIN);
        fluid_clip(x, 0.0f, 1.0f);
        fluid_voice_update_modenv(voice, TRUE, FLUID_VOICE_ENVDECAY,
                                  count, 1.0f, count ? -1.0f / count : 0.0f, x, 2.0f);
        break;

    case GEN_MODENVRELEASE:                                  /* SF 2.01 section 8.1.3 # 30 */
        fluid_clip(x, -12000.0f, 8000.0f);
        count = 1 + NUM_BUFFERS_RELEASE(x);
        fluid_voice_update_modenv(voice, TRUE, FLUID_VOICE_ENVRELEASE,
                                  count, 1.0f, -1.0f / count, 0.0f, 2.0f);

        break;

    } /* switch gen */
}

/**
 * Recalculate voice parameters for a given control.
 *
 * @param voice the synthesis voice
 * @param cc flag to distinguish between a continuous control and a channel control (pitch bend, ...)
 * @param ctrl the control number:
 *   when >=0, only modulators's destination having ctrl as source are updated.
 *   when -1, all modulators's destination are updated (regardless of ctrl).
 *
 * In this implementation, I want to make sure that all controllers
 * are event based: the parameter values of the DSP algorithm should
 * only be updates when a controller event arrived and not at every
 * iteration of the audio cycle (which would probably be feasible if
 * the synth was made in silicon).
 *
 * The update is done in two steps:
 *
 * - step 1: first, we look for all the modulators that have the changed
 * controller as a source. This will yield a generator that will be changed
 * because of the controller event.
 *
 * - step 2: For this generator, calculate its new value. This is the
 * sum of its original value plus the values of all the attached modulators.
 * The generator flag is set to indicate the parameters must be updated.
 * This avoid the risk to call 'fluid_voice_update_param' several
 * times for the same generator if several modulators have that generator as
 * destination. So every changed generators are updated only once.
 */

 /* bit table for each generator being updated. The bits are packed in variables
  Each variable have NBR_BIT_BY_VAR bits represented by NBR_BIT_BY_VAR_LN2.
  The size of the table is the number of variables: SIZE_UPDATED_GEN_BIT.
 
  Note: In this implementation NBR_BIT_BY_VAR_LN2 is set to 5 (convenient for 32 bits cpu)
  but this could be set to 6 for 64 bits cpu.
 */

#define NBR_BIT_BY_VAR_LN2 5	/* for 32 bits variables */
#define NBR_BIT_BY_VAR  (1 << NBR_BIT_BY_VAR_LN2)
#define NBR_BIT_BY_VAR_ANDMASK (NBR_BIT_BY_VAR - 1)
#define	SIZE_UPDATED_GEN_BIT  ((GEN_LAST + NBR_BIT_BY_VAR_ANDMASK) / NBR_BIT_BY_VAR)

#define is_gen_updated(bit,gen)  (bit[gen >> NBR_BIT_BY_VAR_LN2] &  (1 << (gen & NBR_BIT_BY_VAR_ANDMASK)))
#define set_gen_updated(bit,gen) (bit[gen >> NBR_BIT_BY_VAR_LN2] |= (1 << (gen & NBR_BIT_BY_VAR_ANDMASK)))

int fluid_voice_modulate(fluid_voice_t *voice, int cc, int ctrl)
{
    int i, k;
    fluid_mod_t *mod;
    uint32_t gen;
    fluid_real_t modval;

    /* Clears registered bits table of updated generators */
    uint32_t updated_gen_bit[SIZE_UPDATED_GEN_BIT] = {0};

    /*    printf("Chan=%d, CC=%d, Src=%d, Val=%d\n", voice->channel->channum, cc, ctrl, val); */

    for(i = 0; i < voice->mod_count; i++)
    {
        mod = &voice->mod[i];

        /* step 1: find all the modulators that have the changed controller
           as input source. When ctrl is -1 all modulators destination
           are updated */
        if(ctrl < 0 || fluid_mod_has_source(mod, cc, ctrl))
        {
            gen = fluid_mod_get_dest(mod);

            /* Skip if this generator has already been updated */
            if(!is_gen_updated(updated_gen_bit, gen))
            {
                modval = 0.0;

                /* step 2: for every attached modulator, calculate the modulation
                 * value for the generator gen */
                for(k = 0; k < voice->mod_count; k++)
                {
                    if(fluid_mod_has_dest(&voice->mod[k], gen))
                    {
                        modval += fluid_mod_get_value(&voice->mod[k], voice);
                    }
                }

                fluid_gen_set_mod(&voice->gen[gen], modval);

                /* now recalculate the parameter values that are derived from the
                   generator */
                fluid_voice_update_param(voice, gen);

                /* set the bit that indicates this generator is updated */
                set_gen_updated(updated_gen_bit, gen);
            }
        }
    }

    return FLUID_OK;
}

/**
 * Update all the modulators.
 *
 * This function is called after a ALL_CTRL_OFF MIDI message has been received (CC 121).
 * All destinations of all modulators will be updated.
 */
int fluid_voice_modulate_all(fluid_voice_t *voice)
{
    return fluid_voice_modulate(voice, 0, -1);
}

/* legato update functions --------------------------------------------------*/

/* Updates voice portamento parameters
 *
 * @voice voice the synthesis voice
 * @fromkey the beginning pitch of portamento.
 * @tokey the ending pitch of portamento.
 *
 * The function calculates pitch offset and increment, then these parameters
 * are send to the dsp.
*/
void fluid_voice_update_portamento(fluid_voice_t *voice, int fromkey, int tokey)

{
    fluid_channel_t *channel = voice->channel;

    /* calculates pitch offset */
    fluid_real_t PitchBeg = fluid_voice_calculate_pitch(voice, fromkey);
    fluid_real_t PitchEnd = fluid_voice_calculate_pitch(voice, tokey);
    fluid_real_t pitchoffset = PitchBeg - PitchEnd;

    /* Calculates increment countinc */
    /* Increment is function of PortamentoTime (ms)*/
    unsigned int countinc = (unsigned int)(((fluid_real_t)voice->output_rate *
                                            0.001f *
                                            (fluid_real_t)fluid_channel_portamentotime(channel))  /
                                           (fluid_real_t)FLUID_BUFSIZE  + 0.5f);

    /* Send portamento parameters to the voice dsp */
    UPDATE_RVOICE_GENERIC_IR(fluid_rvoice_set_portamento, voice->rvoice, countinc, pitchoffset);
}

/*---------------------------------------------------------------*/
/*legato mode 1: multi_retrigger
 *
 * Modulates all generators dependent of key,vel.
 * Forces the voice envelopes in the attack section (legato mode 1).
 *
 * @voice voice the synthesis voice
 * @tokey the new key to be applied to this voice.
 * @vel the new velocity to be applied to this voice.
 */
void fluid_voice_update_multi_retrigger_attack(fluid_voice_t *voice,
        int tokey, int vel)
{
    voice->key = tokey;  /* new note */
    voice->vel = vel; /* new velocity */
    /* Updates generators dependent of velocity */
    /* Modulates GEN_ATTENUATION (and others ) before calling
       fluid_rvoice_multi_retrigger_attack().*/
    fluid_voice_modulate(voice, FALSE, FLUID_MOD_VELOCITY);

    /* Updates generator dependent of voice->key */
    fluid_voice_update_param(voice, GEN_KEYTOMODENVHOLD);
    fluid_voice_update_param(voice, GEN_KEYTOMODENVDECAY);
    fluid_voice_update_param(voice, GEN_KEYTOVOLENVHOLD);
    fluid_voice_update_param(voice, GEN_KEYTOVOLENVDECAY);

    /* Updates pitch generator  */
    fluid_voice_calculate_gen_pitch(voice);
    fluid_voice_update_param(voice, GEN_PITCH);

    /* updates adsr generator */
    UPDATE_RVOICE0(fluid_rvoice_multi_retrigger_attack);
}
/** end of legato update functions */

/*
 Force the voice into release stage. Useful anywhere a voice
 needs to be damped even if pedals (sustain sostenuto) are depressed.
 See fluid_synth_damp_voices_by_sustain_LOCAL(),
 fluid_synth_damp_voices_by_sostenuto_LOCAL,
 fluid_voice_noteoff().
*/
void
fluid_voice_release(fluid_voice_t *voice)
{
    unsigned int at_tick = fluid_channel_get_min_note_length_ticks(voice->channel);
    UPDATE_RVOICE_I1(fluid_rvoice_noteoff, at_tick);
    voice->has_noteoff = 1; // voice is marked as noteoff occurred
}

/*
 * fluid_voice_noteoff
 *
 * Sending a noteoff event will advance the envelopes to section 5 (release).
 * The function is convenient for polyphonic or monophonic note
 */
void
fluid_voice_noteoff(fluid_voice_t *voice)
{
    fluid_channel_t *channel;

    fluid_profile(FLUID_PROF_VOICE_NOTE, voice->ref, 0, 0);

    channel = voice->channel;

    /* Sustain a note under Sostenuto pedal */
    if(fluid_channel_sostenuto(channel) &&
            channel->sostenuto_orderid > voice->id)
    {
        // Sostenuto depressed after note
        voice->status = FLUID_VOICE_HELD_BY_SOSTENUTO;
    }
    /* Or sustain a note under Sustain pedal */
    else if(fluid_channel_sustained(channel))
    {
        voice->status = FLUID_VOICE_SUSTAINED;
    }
    /* Or force the voice to release stage */
    else
    {
        fluid_voice_release(voice);
    }
}

/*
 * fluid_voice_kill_excl
 *
 * Percussion sounds can be mutually exclusive: for example, a 'closed
 * hihat' sound will terminate an 'open hihat' sound ringing at the
 * same time. This behaviour is modeled using 'exclusive classes',
 * turning on a voice with an exclusive class other than 0 will kill
 * all other voices having that exclusive class within the same preset
 * or channel.  fluid_voice_kill_excl gets called, when 'voice' is to
 * be killed for that reason.
 */

int
fluid_voice_kill_excl(fluid_voice_t *voice)
{

    unsigned int at_tick;

    if(!fluid_voice_is_playing(voice))
    {
        return FLUID_OK;
    }

    /* Turn off the exclusive class information for this voice,
       so that it doesn't get killed twice
    */
    fluid_voice_gen_set(voice, GEN_EXCLUSIVECLASS, 0);

    /* Speed up the volume envelope */
    /* The value was found through listening tests with hi-hat samples. */
    fluid_voice_gen_set(voice, GEN_VOLENVRELEASE, -200);
    fluid_voice_update_param(voice, GEN_VOLENVRELEASE);

    /* Speed up the modulation envelope */
    fluid_voice_gen_set(voice, GEN_MODENVRELEASE, -200);
    fluid_voice_update_param(voice, GEN_MODENVRELEASE);

    at_tick = fluid_channel_get_min_note_length_ticks(voice->channel);
    UPDATE_RVOICE_I1(fluid_rvoice_noteoff, at_tick);


    return FLUID_OK;
}

/*
 * Unlock the overflow rvoice of the voice.
 * Decrement the reference count of the sample owned by this rvoice.
 *
 * Called by fluid_synth when the overflow rvoice has finished by itself.
 * Must be called also explicitly at synth destruction to ensure that
 * the soundfont be unloaded successfully.
 */
void fluid_voice_overflow_rvoice_finished(fluid_voice_t *voice)
{
    voice->can_access_overflow_rvoice = 1;

    /* Decrement the reference count of the sample to indicate
       that this sample isn't owned by the rvoice anymore */
    fluid_voice_sample_unref(&voice->overflow_sample);
}

/*
 * fluid_voice_off
 *
 * Force the voice into finished stage. Useful anywhere a voice
 * needs to be cancelled from MIDI API.
 */
void fluid_voice_off(fluid_voice_t *voice)
{
    UPDATE_RVOICE0(fluid_rvoice_voiceoff); /* request to finish the voice */
}

/*
 * fluid_voice_stop
 *
 * Purpose:
 * Turns off a voice, meaning that it is not processed anymore by the
 * DSP loop, i.e. contrary part to fluid_voice_start().
 */
void
fluid_voice_stop(fluid_voice_t *voice)
{
    fluid_profile(FLUID_PROF_VOICE_RELEASE, voice->ref, 0, 0);

    voice->chan = NO_CHANNEL;

    /* Decrement the reference count of the sample, to indicate
       that this sample isn't owned by the rvoice anymore.
    */
    fluid_voice_sample_unref(&voice->sample);

    voice->status = FLUID_VOICE_OFF;
    voice->has_noteoff = 1;

    /* Decrement voice count */
    voice->channel->synth->active_voice_count--;
}

/**
 * Adds a modulator to the voice if the modulator has valid sources.
 *
 * @param voice Voice instance.
 * @param mod Modulator info (copied).
 * @param mode Determines how to handle an existing identical modulator.
 *   #FLUID_VOICE_ADD to add (offset) the modulator amounts,
 *   #FLUID_VOICE_OVERWRITE to replace the modulator,
 *   #FLUID_VOICE_DEFAULT when adding a default modulator - no duplicate should
 *   exist so don't check.
 */
void
fluid_voice_add_mod(fluid_voice_t *voice, fluid_mod_t *mod, int mode)
{
    /* Ignore the modulator if its sources inputs are invalid */
    if(fluid_mod_check_sources(mod, "api fluid_voice_add_mod mod"))
    {
        fluid_voice_add_mod_local(voice, mod, mode, FLUID_NUM_MOD);
    }
}

/**
 * Adds a modulator to the voice.
 * local version of fluid_voice_add_mod function. Called at noteon time.
 * @param voice, mod, mode, same as for fluid_voice_add_mod() (see above).
 * @param check_limit_count is the modulator number limit to handle with existing
 *   identical modulator(i.e mode FLUID_VOICE_OVERWRITE, FLUID_VOICE_ADD).
 *   - When FLUID_NUM_MOD, all the voices modulators (since the previous call)
 *     are checked for identity.
 *   - When check_count_limit is below the actual number of voices modulators
 *   (voice->mod_count), this will restrict identity check to this number,
 *   This is useful when we know by advance that there is no duplicate with
 *   modulators at index above this limit. This avoid wasting cpu cycles at noteon.
 */
void
fluid_voice_add_mod_local(fluid_voice_t *voice, fluid_mod_t *mod, int mode, int check_limit_count)
{
    int i;

    /* check_limit_count cannot be above voice->mod_count */
    if(check_limit_count > voice->mod_count)
    {
        check_limit_count = voice->mod_count;
    }

    if(mode == FLUID_VOICE_ADD)
    {

        /* if identical modulator exists, add them */
        for(i = 0; i < check_limit_count; i++)
        {
            if(fluid_mod_test_identity(&voice->mod[i], mod))
            {
                //		printf("Adding modulator...\n");
                voice->mod[i].amount += mod->amount;
                return;
            }
        }

    }
    else if(mode == FLUID_VOICE_OVERWRITE)
    {

        /* if identical modulator exists, replace it (only the amount has to be changed) */
        for(i = 0; i < check_limit_count; i++)
        {
            if(fluid_mod_test_identity(&voice->mod[i], mod))
            {
                //		printf("Replacing modulator...amount is %f\n",mod->amount);
                voice->mod[i].amount = mod->amount;
                return;
            }
        }
    }

    /* Add a new modulator (No existing modulator to add / overwrite).
       Also, default modulators (FLUID_VOICE_DEFAULT) are added without
       checking, if the same modulator already exists. */
    if(voice->mod_count < FLUID_NUM_MOD)
    {
        fluid_mod_clone(&voice->mod[voice->mod_count++], mod);
    }
    else
    {
        FLUID_LOG(FLUID_WARN, "Voice %i has more modulators than supported, ignoring.", voice->id);
    }
}

/**
 * Get the unique ID of the noteon-event.
 *
 * @param voice Voice instance
 * @return Note on unique ID
 *
 * A SoundFont loader may store pointers to voices it has created for
 * real-time control during the operation of a voice (for example: parameter
 * changes in SoundFont editor). The synth uses a pool of voices internally which are
 * 'recycled' and never deallocated.
 *
 * However, before modifying an existing voice, check
 * - that its state is still 'playing'
 * - that the ID is still the same
 *
 * Otherwise the voice has finished playing.
 */
unsigned int fluid_voice_get_id(const fluid_voice_t *voice)
{
    return voice->id;
}

/**
 * Check if a voice is producing sound.
 *
 * Like fluid_voice_is_on() this will return TRUE once a call to 
 * fluid_synth_start_voice() has been made. Contrary to fluid_voice_is_on(),
 * this might also return TRUE after the voice received a noteoff event, as it may
 * still be playing in release phase, or because it has been sustained or
 * sostenuto'ed.
 *
 * @param voice Voice instance
 * @return TRUE if playing, FALSE otherwise
 */
int fluid_voice_is_playing(const fluid_voice_t *voice)
{
    return (voice->status == FLUID_VOICE_ON)
           || fluid_voice_is_sustained(voice)
           || fluid_voice_is_sostenuto(voice);

}

/**
 * Check if a voice is ON.
 *
 * A voice is in ON state as soon as a call to fluid_synth_start_voice() has been made
 * (which is typically done in a fluid_preset_t's noteon function).
 * A voice stays ON as long as it has not received a noteoff event.
 *
 * @param voice Voice instance
 * @return TRUE if on, FALSE otherwise
 *
 * @since 1.1.7
 */
int fluid_voice_is_on(const fluid_voice_t *voice)
{
    return (voice->status == FLUID_VOICE_ON && !voice->has_noteoff);
}

/**
 * Check if a voice keeps playing after it has received a noteoff due to being held by sustain.
 *
 * @param voice Voice instance
 * @return TRUE if sustained, FALSE otherwise
 *
 * @since 1.1.7
 */
int fluid_voice_is_sustained(const fluid_voice_t *voice)
{
    return (voice->status == FLUID_VOICE_SUSTAINED);
}

/**
 * Check if a voice keeps playing after it has received a noteoff due to being held by sostenuto.
 *
 * @param voice Voice instance
 * @return TRUE if sostenuto, FALSE otherwise
 *
 * @since 1.1.7
 */
int fluid_voice_is_sostenuto(const fluid_voice_t *voice)
{
    return (voice->status == FLUID_VOICE_HELD_BY_SOSTENUTO);
}

/**
 * Return the MIDI channel the voice is playing on.
 *
 * @param voice Voice instance
 * @return The channel assigned to this voice
 *
 * @note The result of this function is only valid if the voice is playing.
 *
 * @since 1.1.7
 */
int fluid_voice_get_channel(const fluid_voice_t *voice)
{
    return voice->chan;
}

/**
 * Return the effective MIDI key of the playing voice.
 *
 * @param voice Voice instance
 * @return The MIDI key this voice is playing at
 *
 * If the voice was started from an instrument which uses a fixed key generator, it returns that.
 * Otherwise returns the same value as \c fluid_voice_get_key.
 *
 * @note The result of this function is only valid if the voice is playing.
 *
 * @since 1.1.7
 */
int fluid_voice_get_actual_key(const fluid_voice_t *voice)
{
    fluid_real_t x = fluid_voice_gen_value(voice, GEN_KEYNUM);

    if(x >= 0)
    {
        return (int)x;
    }
    else
    {
        return fluid_voice_get_key(voice);
    }
}

/**
 * Return the MIDI key from the starting noteon event.
 *
 * @param voice Voice instance
 * @return The MIDI key of the noteon event that originally turned on this voice
 *
 * @note The result of this function is only valid if the voice is playing.
 *
 * @since 1.1.7
 */
int fluid_voice_get_key(const fluid_voice_t *voice)
{
    return voice->key;
}

/**
 * Return the effective MIDI velocity of the playing voice.
 *
 * @param voice Voice instance
 * @return The MIDI velocity this voice is playing at
 *
 * If the voice was started from an instrument which uses a fixed velocity generator, it returns that.
 * Otherwise it returns the same value as \c fluid_voice_get_velocity.
 *
 * @note The result of this function is only valid if the voice is playing.
 *
 * @since 1.1.7
 */
int fluid_voice_get_actual_velocity(const fluid_voice_t *voice)
{
    fluid_real_t x = fluid_voice_gen_value(voice, GEN_VELOCITY);

    if(x > 0)
    {
        return (int)x;
    }
    else
    {
        return fluid_voice_get_velocity(voice);
    }
}

/**
 * Return the MIDI velocity from the starting noteon event.
 *
 * @param voice Voice instance
 * @return The MIDI velocity which originally turned on this voice
 *
 * @note The result of this function is only valid if the voice is playing.
 *
 * @since 1.1.7
 */
int fluid_voice_get_velocity(const fluid_voice_t *voice)
{
    return voice->vel;
}

/*
 * fluid_voice_get_lower_boundary_for_attenuation
 *
 * Purpose:
 *
 * A lower boundary for the attenuation (as in 'the minimum
 * attenuation of this voice, with volume pedals, modulators
 * etc. resulting in minimum attenuation, cannot fall below x cB) is
 * calculated.  This has to be called during fluid_voice_start, after
 * all modulators have been run on the voice once.  Also,
 * voice->attenuation has to be initialized.
 * (see fluid_voice_calculate_runtime_synthesis_parameters())
 */
static fluid_real_t
fluid_voice_get_lower_boundary_for_attenuation(fluid_voice_t *voice)
{
    int i;
    fluid_mod_t *mod;
    fluid_real_t possible_att_reduction_cB = 0;
    fluid_real_t lower_bound;

    for(i = 0; i < voice->mod_count; i++)
    {
        mod = &voice->mod[i];

        /* Modulator has attenuation as target and can change over time? */
        if((mod->dest == GEN_ATTENUATION)
                && ((mod->flags1 & FLUID_MOD_CC)
                    || (mod->flags2 & FLUID_MOD_CC)
                    || (mod->src1 == FLUID_MOD_CHANNELPRESSURE)
                    || (mod->src1 == FLUID_MOD_KEYPRESSURE)
                    || (mod->src1 == FLUID_MOD_PITCHWHEEL)
                    || (mod->src2 == FLUID_MOD_CHANNELPRESSURE)
                    || (mod->src2 == FLUID_MOD_KEYPRESSURE)
                    || (mod->src2 == FLUID_MOD_PITCHWHEEL)))
        {

            fluid_real_t current_val = fluid_mod_get_value(mod, voice);
            /* min_val is the possible minimum value for this modulator.
               it depends of 3 things :
               1)the minimum values of src1,src2 (i.e -1 if mapping is bipolar
                 or 0 if mapping is unipolar).
               2)the sign of amount.
               3)absolute value of amount.

               When at least one source mapping is bipolar:
			     min_val is -|amount| regardless the sign of amount.
               When both sources mapping are unipolar:
                 min_val is -|amount|, if amount is negative.
                 min_val is 0, if amount is positive
             */
            fluid_real_t min_val = fabs(mod->amount);

            /* Can this modulator produce a negative contribution? */
            if((mod->flags1 & FLUID_MOD_BIPOLAR)
                    || (mod->flags2 & FLUID_MOD_BIPOLAR)
                    || (mod->amount < 0))
            {
                min_val = -min_val; /* min_val = - |amount|*/
            }
            else
            {
                /* No negative value possible. But still, the minimum contribution is 0. */
                min_val = 0;
            }

            /* For example:
             * - current_val=100
             * - min_val=-4000
             * - possible reduction contribution of this modulator = current_val - min_val = 4100
             */
            if(current_val > min_val)
            {
                possible_att_reduction_cB += (current_val - min_val);
            }
        }
    }

    lower_bound = voice->attenuation - possible_att_reduction_cB;

    /* SF2.01 specs do not allow negative attenuation */
    if(lower_bound < 0)
    {
        lower_bound = 0;
    }

    return lower_bound;
}




int fluid_voice_set_param(fluid_voice_t *voice, int gen, fluid_real_t nrpn_value)
{
    voice->gen[gen].nrpn = nrpn_value;
    voice->gen[gen].flags = GEN_SET;
    fluid_voice_update_param(voice, gen);
    return FLUID_OK;
}

int fluid_voice_set_gain(fluid_voice_t *voice, fluid_real_t gain)
{
    fluid_real_t left, right, reverb, chorus;

    /* avoid division by zero*/
    if(gain < 0.0000001f)
    {
        gain = 0.0000001f;
    }

    voice->synth_gain = gain;
    left = fluid_voice_calculate_gain_amplitude(voice,
            fluid_pan(voice->pan, 1) * fluid_balance(voice->balance, 1));
    right = fluid_voice_calculate_gain_amplitude(voice,
            fluid_pan(voice->pan, 0) * fluid_balance(voice->balance, 0));
    reverb = fluid_voice_calculate_gain_amplitude(voice, voice->reverb_send);
    chorus = fluid_voice_calculate_gain_amplitude(voice, voice->chorus_send);

    UPDATE_RVOICE_R1(fluid_rvoice_set_synth_gain, gain);
    UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 0, left);
    UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 1, right);
    UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 2, reverb);
    UPDATE_RVOICE_BUFFERS_AMP(fluid_rvoice_buffers_set_amp, 3, chorus);

    return FLUID_OK;
}

/* - Scan the loop
 * - determine the peak level
 * - Calculate, what factor will make the loop inaudible
 * - Store in sample
 */

/**
 * Calculate the peak volume of a sample for voice off optimization.
 *
 * @param s Sample to optimize
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * If the peak volume during the loop is known, then the voice can
 * be released earlier during the release phase. Otherwise, the
 * voice will operate (inaudibly), until the envelope is at the
 * nominal turnoff point.  So it's a good idea to call
 * fluid_voice_optimize_sample() on each sample once.
 */
int
fluid_voice_optimize_sample(fluid_sample_t *s)
{
    int32_t peak_max = 0;
    int32_t peak_min = 0;
    int32_t peak;
    fluid_real_t normalized_amplitude_during_loop;
    double result;
    unsigned int i;

    /* ignore disabled samples */
    if(s->start == s->end)
    {
        return (FLUID_OK);
    }

    if(!s->amplitude_that_reaches_noise_floor_is_valid)    /* Only once */
    {
        /* Scan the loop */
        for(i = s->loopstart; i < s->loopend; i++)
        {
            int32_t val = fluid_rvoice_get_sample(s->data, s->data24, i);

            if(val > peak_max)
            {
                peak_max = val;
            }
            else if(val < peak_min)
            {
                peak_min = val;
            }
        }

        /* Determine the peak level */
        if(peak_max > -peak_min)
        {
            peak = peak_max;
        }
        else
        {
            peak = -peak_min;
        }

        if(peak == 0)
        {
            /* Avoid division by zero */
            peak = 1;
        }

        /* Calculate what factor will make the loop inaudible
         * For example: Take a peak of 3277 (10 % of 32768).  The
         * normalized amplitude is 0.1 (10 % of 32768).  An amplitude
         * factor of 0.0001 (as opposed to the default 0.00001) will
         * drop this sample to the noise floor.
         */

        /* 16 bits => 96+4=100 dB dynamic range => 0.00001 */
        normalized_amplitude_during_loop = ((fluid_real_t)peak) / (INT24_MAX * 1.0f);
        result = FLUID_NOISE_FLOOR / normalized_amplitude_during_loop;

        /* Store in sample */
        s->amplitude_that_reaches_noise_floor = (double)result;
        s->amplitude_that_reaches_noise_floor_is_valid = 1;
#if 0
        printf("Sample peak detection: factor %f\n", (double)result);
#endif
    }

    return FLUID_OK;
}

float
fluid_voice_get_overflow_prio(fluid_voice_t *voice,
                              fluid_overflow_prio_t *score,
                              unsigned int cur_time)
{
    float this_voice_prio = 0;
    int channel;

    /* Are we already overflowing? */
    if(!voice->can_access_overflow_rvoice)
    {
        return OVERFLOW_PRIO_CANNOT_KILL;
    }

    /* Is this voice on the drum channel?
     * Then it is very important.
     * Also skip the released and sustained scores.
     */
    if(voice->channel->channel_type == CHANNEL_TYPE_DRUM)
    {
        this_voice_prio += score->percussion;
    }
    else if(voice->has_noteoff)
    {
        /* Noteoff has */
        this_voice_prio += score->released;
    }
    else if(fluid_voice_is_sustained(voice) || fluid_voice_is_sostenuto(voice))
    {
        /* This voice is still active, since the sustain pedal is held down.
         * Consider it less important than non-sustained channels.
         * This decision is somehow subjective. But usually the sustain pedal
         * is used to play 'more-voices-than-fingers', so it shouldn't hurt
         * if we kill one voice.
         */
        this_voice_prio += score->sustained;
    }

    /* We are not enthusiastic about releasing voices, which have just been started.
     * Otherwise hitting a chord may result in killing notes belonging to that very same
     * chord. So give newer voices a higher score. */
    if(score->age)
    {
        cur_time -= voice->start_time;

        if(cur_time < 1)
        {
            cur_time = 1; // Avoid div by zero
        }

        this_voice_prio += (score->age * voice->output_rate) / cur_time;
    }

    /* take a rough estimate of loudness into account. Louder voices are more important. */
    if(score->volume)
    {
        fluid_real_t a = voice->attenuation;

        if(voice->has_noteoff)
        {
            // FIXME: Should take into account where on the envelope we are...?
        }

        if(a < 0.1f)
        {
            a = 0.1f; // Avoid div by zero
        }

        this_voice_prio += score->volume / a;
    }

    /* Check if this voice is on an important channel. If so, then add the
     * score for important channels */
    channel = fluid_voice_get_channel(voice);

    if(channel < score->num_important_channels && score->important_channels[channel])
    {
        this_voice_prio += score->important;
    }

    return this_voice_prio;
}


void fluid_voice_set_custom_filter(fluid_voice_t *voice, enum fluid_iir_filter_type type, enum fluid_iir_filter_flags flags)
{
    UPDATE_RVOICE_GENERIC_I2(fluid_iir_filter_init, &voice->rvoice->resonant_custom_filter, type, flags);
}

