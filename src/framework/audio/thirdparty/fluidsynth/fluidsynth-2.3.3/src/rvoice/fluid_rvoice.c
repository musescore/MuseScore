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

#include "fluid_rvoice.h"
#include "fluid_conv.h"
#include "fluid_sys.h"


static void fluid_rvoice_noteoff_LOCAL(fluid_rvoice_t *voice, unsigned int min_ticks);

/**
 * @return -1 if voice is quiet, 0 if voice has finished, 1 otherwise
 */
static FLUID_INLINE int
fluid_rvoice_calc_amp(fluid_rvoice_t *voice)
{
    fluid_real_t target_amp;	/* target amplitude */

    if(fluid_adsr_env_get_section(&voice->envlfo.volenv) == FLUID_VOICE_ENVDELAY)
    {
        return -1;    /* The volume amplitude is in hold phase. No sound is produced. */
    }

    if(fluid_adsr_env_get_section(&voice->envlfo.volenv) == FLUID_VOICE_ENVATTACK)
    {
        /* the envelope is in the attack section: ramp linearly to max value.
         * A positive modlfo_to_vol should increase volume (negative attenuation).
         */
        target_amp = fluid_cb2amp(voice->dsp.attenuation)
                     * fluid_cb2amp(fluid_lfo_get_val(&voice->envlfo.modlfo) * -voice->envlfo.modlfo_to_vol)
                     * fluid_adsr_env_get_val(&voice->envlfo.volenv);
    }
    else
    {
        fluid_real_t amplitude_that_reaches_noise_floor;
        fluid_real_t amp_max;

        target_amp = fluid_cb2amp(voice->dsp.attenuation)
                     * fluid_cb2amp(FLUID_PEAK_ATTENUATION * (1.0f - fluid_adsr_env_get_val(&voice->envlfo.volenv))
                                    + fluid_lfo_get_val(&voice->envlfo.modlfo) * -voice->envlfo.modlfo_to_vol);

        /* We turn off a voice, if the volume has dropped low enough. */

        /* A voice can be turned off, when an estimate for the volume
         * (upper bound) falls below that volume, that will drop the
         * sample below the noise floor.
         */

        /* If the loop amplitude is known, we can use it if the voice loop is within
         * the sample loop
         */

        /* Is the playing pointer already in the loop? */
        if(voice->dsp.has_looped)
        {
            amplitude_that_reaches_noise_floor = voice->dsp.amplitude_that_reaches_noise_floor_loop;
        }
        else
        {
            amplitude_that_reaches_noise_floor = voice->dsp.amplitude_that_reaches_noise_floor_nonloop;
        }

        /* voice->attenuation_min is a lower boundary for the attenuation
         * now and in the future (possibly 0 in the worst case).  Now the
         * amplitude of sample and volenv cannot exceed amp_max (since
         * volenv_val can only drop):
         */

        amp_max = fluid_cb2amp(voice->dsp.min_attenuation_cB) *
                  fluid_adsr_env_get_val(&voice->envlfo.volenv);

        /* And if amp_max is already smaller than the known amplitude,
         * which will attenuate the sample below the noise floor, then we
         * can safely turn off the voice. Duh. */
        if(amp_max < amplitude_that_reaches_noise_floor)
        {
            return 0;
        }
    }

    /* Volume increment to go from voice->amp to target_amp in FLUID_BUFSIZE steps */
    voice->dsp.amp_incr = (target_amp - voice->dsp.amp) / FLUID_BUFSIZE;

    fluid_check_fpe("voice_write amplitude calculation");

    /* no volume and not changing? - No need to process */
    if((voice->dsp.amp == 0.0f) && (voice->dsp.amp_incr == 0.0f))
    {
        return -1;
    }

    return 1;
}


/* these should be the absolute minimum that FluidSynth can deal with */
#define FLUID_MIN_LOOP_SIZE 2
#define FLUID_MIN_LOOP_PAD 0

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)

/* Purpose:
 *
 * Make sure, that sample start / end point and loop points are in
 * proper order. When starting up, calculate the initial phase.
 * TODO: Investigate whether this can be moved from rvoice to voice.
 */
static void
fluid_rvoice_check_sample_sanity(fluid_rvoice_t *voice)
{
    int min_index_nonloop = (int) voice->dsp.sample->start;
    int max_index_nonloop = (int) voice->dsp.sample->end;

    /* make sure we have enough samples surrounding the loop */
    int min_index_loop = (int) voice->dsp.sample->start + FLUID_MIN_LOOP_PAD;
    int max_index_loop = (int) voice->dsp.sample->end - FLUID_MIN_LOOP_PAD + 1;	/* 'end' is last valid sample, loopend can be + 1 */
    fluid_check_fpe("voice_check_sample_sanity start");

#if 0
    printf("Sample from %i to %i\n", voice->dsp.sample->start, voice->dsp.sample->end);
    printf("Sample loop from %i %i\n", voice->dsp.sample->loopstart, voice->dsp.sample->loopend);
    printf("Playback from %i to %i\n", voice->dsp.start, voice->dsp.end);
    printf("Playback loop from %i to %i\n", voice->dsp.loopstart, voice->dsp.loopend);
#endif

    /* Keep the start point within the sample data */
    if(voice->dsp.start < min_index_nonloop)
    {
        voice->dsp.start = min_index_nonloop;
    }
    else if(voice->dsp.start > max_index_nonloop)
    {
        voice->dsp.start = max_index_nonloop;
    }

    /* Keep the end point within the sample data */
    if(voice->dsp.end < min_index_nonloop)
    {
        voice->dsp.end = min_index_nonloop;
    }
    else if(voice->dsp.end > max_index_nonloop)
    {
        voice->dsp.end = max_index_nonloop;
    }

    /* Keep start and end point in the right order */
    if(voice->dsp.start > voice->dsp.end)
    {
        int temp = voice->dsp.start;
        voice->dsp.start = voice->dsp.end;
        voice->dsp.end = temp;
        /*FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Changing order of start / end points!"); */
    }

    /* Zero length? */
    if(voice->dsp.start == voice->dsp.end)
    {
        fluid_rvoice_voiceoff(voice, NULL);
        return;
    }

    if((voice->dsp.samplemode == FLUID_LOOP_UNTIL_RELEASE)
            || (voice->dsp.samplemode == FLUID_LOOP_DURING_RELEASE))
    {
        /* Keep the loop start point within the sample data */
        if(voice->dsp.loopstart < min_index_loop)
        {
            voice->dsp.loopstart = min_index_loop;
        }
        else if(voice->dsp.loopstart > max_index_loop)
        {
            voice->dsp.loopstart = max_index_loop;
        }

        /* Keep the loop end point within the sample data */
        if(voice->dsp.loopend < min_index_loop)
        {
            voice->dsp.loopend = min_index_loop;
        }
        else if(voice->dsp.loopend > max_index_loop)
        {
            voice->dsp.loopend = max_index_loop;
        }

        /* Keep loop start and end point in the right order */
        if(voice->dsp.loopstart > voice->dsp.loopend)
        {
            int temp = voice->dsp.loopstart;
            voice->dsp.loopstart = voice->dsp.loopend;
            voice->dsp.loopend = temp;
            /*FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Changing order of loop points!"); */
        }

        /* Loop too short? Then don't loop. */
        if(voice->dsp.loopend < voice->dsp.loopstart + FLUID_MIN_LOOP_SIZE)
        {
            voice->dsp.samplemode = FLUID_UNLOOPED;
        }

        /* The loop points may have changed. Obtain a new estimate for the loop volume. */
        /* Is the voice loop within the sample loop? */
        if((int)voice->dsp.loopstart >= (int)voice->dsp.sample->loopstart
                && (int)voice->dsp.loopend <= (int)voice->dsp.sample->loopend)
        {
            /* Is there a valid peak amplitude available for the loop, and can we use it? */
            if(voice->dsp.sample->amplitude_that_reaches_noise_floor_is_valid && voice->dsp.samplemode == FLUID_LOOP_DURING_RELEASE)
            {
                voice->dsp.amplitude_that_reaches_noise_floor_loop = voice->dsp.sample->amplitude_that_reaches_noise_floor / voice->dsp.synth_gain;
            }
            else
            {
                /* Worst case */
                voice->dsp.amplitude_that_reaches_noise_floor_loop = voice->dsp.amplitude_that_reaches_noise_floor_nonloop;
            };
        };

    } /* if sample mode is looped */

    /* Run startup specific code (only once, when the voice is started) */
    if(voice->dsp.check_sample_sanity_flag & FLUID_SAMPLESANITY_STARTUP)
    {
        if(max_index_loop - min_index_loop < FLUID_MIN_LOOP_SIZE)
        {
            if((voice->dsp.samplemode == FLUID_LOOP_UNTIL_RELEASE)
                    || (voice->dsp.samplemode == FLUID_LOOP_DURING_RELEASE))
            {
                voice->dsp.samplemode = FLUID_UNLOOPED;
            }
        }

        /* Set the initial phase of the voice (using the result from the
        start offset modulators). */
        fluid_phase_set_int(voice->dsp.phase, voice->dsp.start);
    } /* if startup */

    /* Is this voice run in loop mode, or does it run straight to the
       end of the waveform data? */
    if(((voice->dsp.samplemode == FLUID_LOOP_UNTIL_RELEASE) &&
            (fluid_adsr_env_get_section(&voice->envlfo.volenv) < FLUID_VOICE_ENVRELEASE))
            || (voice->dsp.samplemode == FLUID_LOOP_DURING_RELEASE))
    {
        /* Yes, it will loop as soon as it reaches the loop point.  In
         * this case we must prevent, that the playback pointer (phase)
         * happens to end up beyond the 2nd loop point, because the
         * point has moved.  The DSP algorithm is unable to cope with
         * that situation.  So if the phase is beyond the 2nd loop
         * point, set it to the start of the loop. No way to avoid some
         * noise here.  Note: If the sample pointer ends up -before the
         * first loop point- instead, then the DSP loop will just play
         * the sample, enter the loop and proceed as expected => no
         * actions required.
         */
        int index_in_sample = fluid_phase_index(voice->dsp.phase);

        if(index_in_sample >= voice->dsp.loopend)
        {
            /* FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Phase after 2nd loop point!"); */
            fluid_phase_set_int(voice->dsp.phase, voice->dsp.loopstart);
        }
    }

    /*    FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Sample from %i to %i, loop from %i to %i", voice->dsp.start, voice->dsp.end, voice->dsp.loopstart, voice->dsp.loopend); */

    /* Sample sanity has been assured. Don't check again, until some
       sample parameter is changed by modulation. */
    voice->dsp.check_sample_sanity_flag = 0;
#if 0
    printf("Sane? playback loop from %i to %i\n", voice->dsp.loopstart, voice->dsp.loopend);
#endif
    fluid_check_fpe("voice_check_sample_sanity");
}


/**
 * Synthesize a voice to a buffer.
 *
 * @param voice rvoice to synthesize
 * @param dsp_buf Audio buffer to synthesize to (#FLUID_BUFSIZE in length)
 * @return Count of samples written to dsp_buf. (-1 means voice is currently
 * quiet, 0 .. #FLUID_BUFSIZE-1 means voice finished.)
 *
 * Panning, reverb and chorus are processed separately. The dsp interpolation
 * routine is in (fluid_rvoice_dsp.c).
 */
int
fluid_rvoice_write(fluid_rvoice_t *voice, fluid_real_t *dsp_buf)
{
    int ticks = voice->envlfo.ticks;
    int count, is_looping;
    fluid_real_t modenv_val;

    /******************* sample sanity check **********/

    if(!voice->dsp.sample)
    {
        return 0;
    }

    if(voice->dsp.check_sample_sanity_flag)
    {
        fluid_rvoice_check_sample_sanity(voice);
    }

    /******************* noteoff check ****************/

    if(voice->envlfo.noteoff_ticks != 0 &&
            voice->envlfo.ticks >= voice->envlfo.noteoff_ticks)
    {
        fluid_rvoice_noteoff_LOCAL(voice, 0);
    }

    voice->envlfo.ticks += FLUID_BUFSIZE;

    /******************* vol env **********************/

    fluid_adsr_env_calc(&voice->envlfo.volenv);
    fluid_check_fpe("voice_write vol env");

    if(fluid_adsr_env_get_section(&voice->envlfo.volenv) == FLUID_VOICE_ENVFINISHED)
    {
        return 0;
    }

    /******************* mod env **********************/

    fluid_adsr_env_calc(&voice->envlfo.modenv);
    fluid_check_fpe("voice_write mod env");

    /******************* lfo **********************/

    fluid_lfo_calc(&voice->envlfo.modlfo, ticks);
    fluid_check_fpe("voice_write mod LFO");
    fluid_lfo_calc(&voice->envlfo.viblfo, ticks);
    fluid_check_fpe("voice_write vib LFO");

    /******************* amplitude **********************/

    count = fluid_rvoice_calc_amp(voice);

    if(count <= 0)
    {
        return count; /* return -1 if voice is quiet, 0 if voice has finished */
    }

    /******************* phase **********************/

    /* SF2.04 section 8.1.2 #26:
     * attack of modEnv is convex ?!?
     */
    modenv_val = (fluid_adsr_env_get_section(&voice->envlfo.modenv) == FLUID_VOICE_ENVATTACK)
                 ? fluid_convex(127 * fluid_adsr_env_get_val(&voice->envlfo.modenv))
                 : fluid_adsr_env_get_val(&voice->envlfo.modenv);
    /* Calculate the number of samples, that the DSP loop advances
     * through the original waveform with each step in the output
     * buffer. It is the ratio between the frequencies of original
     * waveform and output waveform.*/
    voice->dsp.phase_incr = fluid_ct2hz_real(voice->dsp.pitch +
                            voice->dsp.pitchoffset +
                            fluid_lfo_get_val(&voice->envlfo.modlfo) * voice->envlfo.modlfo_to_pitch
                            + fluid_lfo_get_val(&voice->envlfo.viblfo) * voice->envlfo.viblfo_to_pitch
                            + modenv_val * voice->envlfo.modenv_to_pitch)
                            / voice->dsp.root_pitch_hz;

    /******************* portamento ****************/
    /* pitchoffset is updated if enabled.
       Pitchoffset will be added to dsp pitch at next phase calculation time */

    /* In most cases portamento will be disabled. Thus first verify that portamento is
     * enabled before updating pitchoffset and before disabling portamento when necessary,
     * in order to keep the performance loss at minimum.
     * If the algorithm would first update pitchoffset and then verify if portamento
     * needs to be disabled, there would be a significant performance drop on a x87 FPU
     */
    if(voice->dsp.pitchinc > 0.0f)
    {
        /* portamento is enabled, so update pitchoffset */
        voice->dsp.pitchoffset += voice->dsp.pitchinc;

        /* when pitchoffset reaches 0.0f, portamento is disabled */
        if(voice->dsp.pitchoffset > 0.0f)
        {
            voice->dsp.pitchoffset = voice->dsp.pitchinc = 0.0f;
        }
    }
    else if(voice->dsp.pitchinc < 0.0f)
    {
        /* portamento is enabled, so update pitchoffset */
        voice->dsp.pitchoffset += voice->dsp.pitchinc;

        /* when pitchoffset reaches 0.0f, portamento is disabled */
        if(voice->dsp.pitchoffset < 0.0f)
        {
            voice->dsp.pitchoffset = voice->dsp.pitchinc = 0.0f;
        }
    }

    fluid_check_fpe("voice_write phase calculation");

    /* if phase_incr is not advancing, set it to the minimum fraction value (prevent stuckage) */
    if(voice->dsp.phase_incr == 0)
    {
        voice->dsp.phase_incr = 1;
    }

    /* voice is currently looping? */
    is_looping = voice->dsp.samplemode == FLUID_LOOP_DURING_RELEASE
                 || (voice->dsp.samplemode == FLUID_LOOP_UNTIL_RELEASE
                     && fluid_adsr_env_get_section(&voice->envlfo.volenv) < FLUID_VOICE_ENVRELEASE);

    /*********************** run the dsp chain ************************
     * The sample is mixed with the output buffer.
     * The buffer has to be filled from 0 to FLUID_BUFSIZE-1.
     * Depending on the position in the loop and the loop size, this
     * may require several runs. */

    switch(voice->dsp.interp_method)
    {
    case FLUID_INTERP_NONE:
        count = fluid_rvoice_dsp_interpolate_none(&voice->dsp, dsp_buf, is_looping);
        break;

    case FLUID_INTERP_LINEAR:
        count = fluid_rvoice_dsp_interpolate_linear(&voice->dsp, dsp_buf, is_looping);
        break;

    case FLUID_INTERP_4THORDER:
    default:
        count = fluid_rvoice_dsp_interpolate_4th_order(&voice->dsp, dsp_buf, is_looping);
        break;

    case FLUID_INTERP_7THORDER:
        count = fluid_rvoice_dsp_interpolate_7th_order(&voice->dsp, dsp_buf, is_looping);
        break;
    }

    fluid_check_fpe("voice_write interpolation");

    if(count == 0)
    {
        return count;
    }

    /*************** resonant filter ******************/

    fluid_iir_filter_calc(&voice->resonant_filter, voice->dsp.output_rate,
                          fluid_lfo_get_val(&voice->envlfo.modlfo) * voice->envlfo.modlfo_to_fc +
                          modenv_val * voice->envlfo.modenv_to_fc);

    fluid_iir_filter_apply(&voice->resonant_filter, dsp_buf, count);

    /* additional custom filter - only uses the fixed modulator, no lfos... */
    fluid_iir_filter_calc(&voice->resonant_custom_filter, voice->dsp.output_rate, 0);
    fluid_iir_filter_apply(&voice->resonant_custom_filter, dsp_buf, count);

    return count;
}

/**
 * Initialize buffers up to (and including) bufnum
 */
static int
fluid_rvoice_buffers_check_bufnum(fluid_rvoice_buffers_t *buffers, unsigned int bufnum)
{
    unsigned int i;

    if(bufnum < buffers->count)
    {
        return FLUID_OK;
    }

    if(bufnum >= FLUID_RVOICE_MAX_BUFS)
    {
        return FLUID_FAILED;
    }

    for(i = buffers->count; i <= bufnum; i++)
    {
        buffers->bufs[i].target_amp = 0.0f;
        buffers->bufs[i].current_amp = 0.0f;
    }

    buffers->count = bufnum + 1;
    return FLUID_OK;
}


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_buffers_set_amp)
{
    fluid_rvoice_buffers_t *buffers = obj;
    unsigned int bufnum = param[0].i;
    fluid_real_t value = param[1].real;

    if(fluid_rvoice_buffers_check_bufnum(buffers, bufnum) != FLUID_OK)
    {
        return;
    }

    buffers->bufs[bufnum].target_amp = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_buffers_set_mapping)
{
    fluid_rvoice_buffers_t *buffers = obj;
    unsigned int bufnum = param[0].i;
    int mapping = param[1].i;

    if(fluid_rvoice_buffers_check_bufnum(buffers, bufnum) != FLUID_OK)
    {
        return;
    }

    buffers->bufs[bufnum].mapping = mapping;
}


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_reset)
{
    fluid_rvoice_t *voice = obj;

    voice->dsp.has_looped = 0;
    voice->envlfo.ticks = 0;
    voice->envlfo.noteoff_ticks = 0;
    voice->dsp.amp = 0.0f; /* The last value of the volume envelope, used to
                            calculate the volume increment during
                            processing */

    /* legato initialization */
    voice->dsp.pitchoffset = 0.0;   /* portamento initialization */
    voice->dsp.pitchinc = 0.0;

    /* mod env initialization*/
    fluid_adsr_env_reset(&voice->envlfo.modenv);

    /* vol env initialization */
    fluid_adsr_env_reset(&voice->envlfo.volenv);

    /* Fixme: Retrieve from any other existing
       voice on this channel to keep LFOs in
       unison? */
    fluid_lfo_reset(&voice->envlfo.viblfo);
    fluid_lfo_reset(&voice->envlfo.modlfo);

    /* Clear sample history in filter */
    fluid_iir_filter_reset(&voice->resonant_filter);
    fluid_iir_filter_reset(&voice->resonant_custom_filter);

    /* Force setting of the phase at the first DSP loop run
     * This cannot be done earlier, because it depends on modulators.
       [DH] Is that comment really true? */
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_STARTUP;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_noteoff)
{
    fluid_rvoice_t *rvoice = obj;
    unsigned int min_ticks = param[0].i;

    fluid_rvoice_noteoff_LOCAL(rvoice, min_ticks);
}

static void
fluid_rvoice_noteoff_LOCAL(fluid_rvoice_t *voice, unsigned int min_ticks)
{
    if(min_ticks > voice->envlfo.ticks)
    {
        /* Delay noteoff */
        voice->envlfo.noteoff_ticks = min_ticks;
        return;
    }

    voice->envlfo.noteoff_ticks = 0;

    if(fluid_adsr_env_get_section(&voice->envlfo.volenv) == FLUID_VOICE_ENVATTACK)
    {
        /* A voice is turned off during the attack section of the volume
         * envelope.  The attack section ramps up linearly with
         * amplitude. The other sections use logarithmic scaling. Calculate new
         * volenv_val to achieve equivalent amplitude during the release phase
         * for seamless volume transition.
         */
        if(fluid_adsr_env_get_val(&voice->envlfo.volenv) > 0)
        {
            fluid_real_t lfo = fluid_lfo_get_val(&voice->envlfo.modlfo) * -voice->envlfo.modlfo_to_vol;
            fluid_real_t amp = fluid_adsr_env_get_val(&voice->envlfo.volenv) * fluid_cb2amp(lfo);
            fluid_real_t env_value = - (((-200.f / FLUID_M_LN10) * FLUID_LOGF(amp) - lfo) / FLUID_PEAK_ATTENUATION - 1);
            fluid_clip(env_value, 0.0f, 1.0f);
            fluid_adsr_env_set_val(&voice->envlfo.volenv, env_value);
        }
    }

	if(fluid_adsr_env_get_section(&voice->envlfo.modenv) == FLUID_VOICE_ENVATTACK)
    {
        /* A voice is turned off during the attack section of the modulation
         * envelope. The attack section use convex scaling with pitch and filter
		 * frequency cutoff (see fluid_rvoice_write(): modenv_val = fluid_convex(127 * modenv.val)
		 * The other sections use linear scaling: modenv_val = modenv.val
		 * 
		 * Calculate new modenv.val to achieve equivalent modenv_val during the release phase
         * for seamless pitch and filter frequency cutoff transition.
         */
        if(fluid_adsr_env_get_val(&voice->envlfo.modenv) > 0)
        {
            fluid_real_t env_value = fluid_convex(127 * fluid_adsr_env_get_val(&voice->envlfo.modenv));
            fluid_clip(env_value, 0.0, 1.0);
            fluid_adsr_env_set_val(&voice->envlfo.modenv, env_value);
        }
    }

    fluid_adsr_env_set_section(&voice->envlfo.volenv, FLUID_VOICE_ENVRELEASE);
    fluid_adsr_env_set_section(&voice->envlfo.modenv, FLUID_VOICE_ENVRELEASE);
}

/**
 * skips to Attack section
 *
 * Updates vol and attack data
 * Correction on volume val to achieve equivalent amplitude at noteOn legato
 *
 * @param voice the synthesis voice to be updated
*/
static FLUID_INLINE void fluid_rvoice_local_retrigger_attack(fluid_rvoice_t *voice)
{
    /* skips to Attack section */
    /* Once in Attack section, current count must be reset, to be sure
    that the section will be not be prematurely finished. */
    fluid_adsr_env_set_section(&voice->envlfo.volenv, FLUID_VOICE_ENVATTACK);
    {
        /* Correction on volume val to achieve equivalent amplitude at noteOn legato */
        fluid_env_data_t *env_data;
        fluid_real_t peak = fluid_cb2amp(voice->dsp.attenuation);
        fluid_real_t prev_peak = fluid_cb2amp(voice->dsp.prev_attenuation);
        voice->envlfo.volenv.val = (voice->envlfo.volenv.val  * prev_peak) / peak;
        /* Correction on slope direction for Attack section */
        env_data = &voice->envlfo.volenv.data[FLUID_VOICE_ENVATTACK];

        if(voice->envlfo.volenv.val <= 1.0f)
        {
            /* slope attack for legato note needs to be positive from val  up to 1 */
            env_data->increment = 1.0f / env_data->count;
            env_data->min = -1.0f;
            env_data->max =  1.0f;
        }
        else
        {
            /* slope attack for legato note needs to be negative: from val  down to 1 */
            env_data->increment = -voice->envlfo.volenv.val / env_data->count;
            env_data->min = 1.0f;
            env_data->max = voice->envlfo.volenv.val;
        }
    }
}

/**
 * Used by legato Mode : multi_retrigger
 *  see fluid_synth_noteon_mono_legato_multi_retrigger()
 * @param voice the synthesis voice to be updated
*/
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_multi_retrigger_attack)
{
    fluid_rvoice_t *voice = obj;
    int section; /* volume or modulation section */

    /*-------------------------------------------------------------------------
     Section skip for volume envelope
    --------------------------------------------------------------------------*/
    section = fluid_adsr_env_get_section(&voice->envlfo.volenv);
    if(section >= FLUID_VOICE_ENVHOLD)
    {
        /* DECAY, SUSTAIN,RELEASE section use logarithmic scaling. Calculates new
        volenv_val to achieve equivalent amplitude during the attack phase
        for seamless volume transition. */
        fluid_real_t amp_cb, env_value;
        amp_cb = FLUID_PEAK_ATTENUATION *
                 (1.0f - fluid_adsr_env_get_val(&voice->envlfo.volenv));
        env_value = fluid_cb2amp(amp_cb); /* a bit of optimization */
        fluid_clip(env_value, 0.0, 1.0);
        fluid_adsr_env_set_val(&voice->envlfo.volenv, env_value);
        /* next, skips to Attack section */
    }

    /* skips to Attack section from any section */
    /* Update vol and  attack data */
    fluid_rvoice_local_retrigger_attack(voice);
    
    /*-------------------------------------------------------------------------
     Section skip for modulation envelope
    --------------------------------------------------------------------------*/
    section = fluid_adsr_env_get_section(&voice->envlfo.modenv);
    if(section >= FLUID_VOICE_ENVHOLD)
    {
        /* DECAY, SUSTAIN,RELEASE section use linear scaling. 
        Since v 2.1 , as recommended by soundfont 2.01/2.4 spec, ATTACK section
        uses convex shape (see fluid_rvoice_write() - fluid_convex()).
        Calculate new modenv value (new_value) for seamless attack transition.
        Here we need the inverse of fluid_convex() function defined as:
        new_value = pow(10, (1 - current_val) . FLUID_PEAK_ATTENUATION / -200 . 2.0)
        For performance reason we use fluid_cb2amp(Val) = pow(10, val/-200) with
        val = (1 - current_val) . FLUID_PEAK_ATTENUATION / 2.0 
        */
        fluid_real_t new_value; /* new modenv value */
        new_value = fluid_cb2amp((1.0f - fluid_adsr_env_get_val(&voice->envlfo.modenv))
                                  * FLUID_PEAK_ATTENUATION / 2.0);
        fluid_clip(new_value, 0.0, 1.0);
        fluid_adsr_env_set_val(&voice->envlfo.modenv, new_value);
    }
    /* Skips from any section to ATTACK section */
    fluid_adsr_env_set_section(&voice->envlfo.modenv, FLUID_VOICE_ENVATTACK);
}

/**
 * sets the portamento dsp parameters: dsp.pitchoffset, dsp.pitchinc
 * @param voice rvoice to set portamento.
 * @param countinc increment count number.
 * @param pitchoffset pitch offset to apply to voice dsp.pitch.
 *
 * Notes:
 * 1) To get continuous portamento between consecutive noteOn (n1,n2,n3...),
 *   pitchoffset is accumulated in current dsp pitchoffset.
 * 2) And to get constant portamento duration, dsp pitch increment is updated.
*/
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_portamento)
{
    fluid_rvoice_t *voice = obj;
    unsigned int countinc = param[0].i;
    fluid_real_t pitchoffset = param[1].real;

    if(countinc)
    {
        voice->dsp.pitchoffset += pitchoffset;
        voice->dsp.pitchinc = - voice->dsp.pitchoffset / countinc;
    }

    /* Then during the voice processing (in fluid_rvoice_write()),
    dsp.pitchoffset will be incremented by dsp pitchinc. */
}


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_output_rate)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.output_rate = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_interp_method)
{
    fluid_rvoice_t *voice = obj;
    int value = param[0].i;

    voice->dsp.interp_method = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_root_pitch_hz)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.root_pitch_hz = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_pitch)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.pitch = value;
}


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_attenuation)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.prev_attenuation = voice->dsp.attenuation;
    voice->dsp.attenuation = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_min_attenuation_cB)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.min_attenuation_cB = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_viblfo_to_pitch)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.viblfo_to_pitch = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_pitch)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.modlfo_to_pitch = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_vol)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.modlfo_to_vol = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modlfo_to_fc)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.modlfo_to_fc = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modenv_to_fc)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.modenv_to_fc = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_modenv_to_pitch)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->envlfo.modenv_to_pitch = value;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_synth_gain)
{
    fluid_rvoice_t *voice = obj;
    fluid_real_t value = param[0].real;

    voice->dsp.synth_gain = value;

    /* For a looped sample, this value will be overwritten as soon as the
     * loop parameters are initialized (they may depend on modulators).
     * This value can be kept, it is a worst-case estimate.
     */
    voice->dsp.amplitude_that_reaches_noise_floor_nonloop = FLUID_NOISE_FLOOR / value;
    voice->dsp.amplitude_that_reaches_noise_floor_loop = FLUID_NOISE_FLOOR / value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_start)
{
    fluid_rvoice_t *voice = obj;
    int value = param[0].i;

    voice->dsp.start = value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_end)
{
    fluid_rvoice_t *voice = obj;
    int value = param[0].i;

    voice->dsp.end = value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_loopstart)
{
    fluid_rvoice_t *voice = obj;
    int value = param[0].i;

    voice->dsp.loopstart = value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_loopend)
{
    fluid_rvoice_t *voice = obj;
    int value = param[0].i;

    voice->dsp.loopend = value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_samplemode)
{
    fluid_rvoice_t *voice = obj;
    enum fluid_loop value = param[0].i;

    voice->dsp.samplemode = value;
    voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_CHECK;
}


DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_set_sample)
{
    fluid_rvoice_t *voice = obj;
    fluid_sample_t *value = param[0].ptr;

    voice->dsp.sample = value;

    if(value)
    {
        voice->dsp.check_sample_sanity_flag |= FLUID_SAMPLESANITY_STARTUP;
    }
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_voiceoff)
{
    fluid_rvoice_t *voice = obj;

    fluid_adsr_env_set_section(&voice->envlfo.volenv, FLUID_VOICE_ENVFINISHED);
    fluid_adsr_env_set_section(&voice->envlfo.modenv, FLUID_VOICE_ENVFINISHED);
}


