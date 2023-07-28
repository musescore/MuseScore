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

#include "fluid_rvoice_mixer.h"
#include "fluid_rvoice.h"
#include "fluid_sys.h"
#include "fluid_rev.h"
#include "fluid_chorus.h"
#include "fluid_ladspa.h"
#include "fluid_synth.h"

// If less than x voices, the thread overhead is larger than the gain,
// so don't activate the thread(s).
#define VOICES_PER_THREAD 8

typedef struct _fluid_mixer_buffers_t fluid_mixer_buffers_t;

struct _fluid_mixer_buffers_t
{
    fluid_rvoice_mixer_t *mixer; /**< Owner of object */
#if ENABLE_MIXER_THREADS
    fluid_thread_t *thread;     /**< Thread object */
    fluid_atomic_int_t ready;   /**< Atomic: buffers are ready for mixing */
#endif

    fluid_rvoice_t **finished_voices; /* List of voices who have finished */
    int finished_voice_count;

    fluid_real_t *local_buf;

    int buf_count;
    int fx_buf_count;

    /** buffer to store the left part of a stereo channel to.
     * Specifically a two dimensional array, containing \c buf_count sample buffers
     * (i.e. for each synth.audio-groups), of which each contains
     * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT audio items (=samples)
     * @note Each sample buffer is aligned to the FLUID_DEFAULT_ALIGNMENT
     * boundary provided that this pointer points to an aligned buffer.
     * So make sure to access the sample buffer by first aligning this
     * pointer using fluid_align_ptr()
     */
    fluid_real_t *left_buf;

    /** dito, but for right part of a stereo channel */
    fluid_real_t *right_buf;

    /** buffer to store the left part of a stereo effects channel to.
     * Specifically a two dimensional array, containing \c fx_buf_count buffers
     * (i.e. for each synth.effects-channels), of which each buffer contains
     * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT audio items (=samples)
     */
    fluid_real_t *fx_left_buf;
    fluid_real_t *fx_right_buf;
};

typedef struct _fluid_mixer_fx_t fluid_mixer_fx_t;

struct _fluid_mixer_fx_t
{
    fluid_revmodel_t *reverb; /**< Reverb unit */
    /* reverb shadow parameters here will be returned if queried */
    double reverb_param[FLUID_REVERB_PARAM_LAST];
    int reverb_on; /* reverb on/off */

    fluid_chorus_t *chorus; /**< Chorus unit */
    /* chorus shadow parameters here will be returned if queried */
    double chorus_param[FLUID_CHORUS_PARAM_LAST];
    int chorus_on; /* chorus on/off */
};

struct _fluid_rvoice_mixer_t
{
    fluid_mixer_fx_t *fx;

    fluid_mixer_buffers_t buffers; /**< Used by mixer only: own buffers */
    fluid_rvoice_eventhandler_t *eventhandler;

    fluid_rvoice_t **rvoices; /**< Read-only: Voices array, sorted so that all nulls are last */
    int polyphony; /**< Read-only: Length of voices array */
    int active_voices; /**< Read-only: Number of non-null voices */
    int current_blockcount;      /**< Read-only: how many blocks to process this time */
    int fx_units;
    int with_reverb;        /**< Should the synth use the built-in reverb unit? */
    int with_chorus;        /**< Should the synth use the built-in chorus unit? */
    int mix_fx_to_out;      /**< Should the effects be mixed in with the primary output? */

#ifdef LADSPA
    fluid_ladspa_fx_t *ladspa_fx; /**< Used by mixer only: Effects unit for LADSPA support. Never created or freed */
#endif

#if ENABLE_MIXER_THREADS
//  int sleeping_threads;        /**< Atomic: number of threads currently asleep */
//  int active_threads;          /**< Atomic: number of threads in the thread loop */
    fluid_atomic_int_t threads_should_terminate; /**< Atomic: Set to TRUE when threads should terminate */
    fluid_atomic_int_t current_rvoice;           /**< Atomic: for the threads to know next voice to  */
    fluid_cond_t *wakeup_threads; /**< Signalled when the threads should wake up */
    fluid_cond_mutex_t *wakeup_threads_m; /**< wakeup_threads mutex companion */
    fluid_cond_t *thread_ready; /**< Signalled from thread, when the thread has a buffer ready for mixing */
    fluid_cond_mutex_t *thread_ready_m; /**< thread_ready mutex companion */

    int thread_count;            /**< Number of extra mixer threads for multi-core rendering */
    fluid_mixer_buffers_t *threads;    /**< Array of mixer threads (thread_count in length) */
#endif
};

#if ENABLE_MIXER_THREADS
static void delete_rvoice_mixer_threads(fluid_rvoice_mixer_t *mixer);
static int fluid_rvoice_mixer_set_threads(fluid_rvoice_mixer_t *mixer, int thread_count, int prio_level);
#endif

static FLUID_INLINE void
fluid_rvoice_mixer_process_fx(fluid_rvoice_mixer_t *mixer, int current_blockcount)
{
    // Making those variables const causes gcc to fail with "variable is predetermined ‘shared’ for ‘shared’".
    // Not explicitly marking them shared makes it fail for clang and MSVC...
    /*const*/ int fx_channels_per_unit = mixer->buffers.fx_buf_count / mixer->fx_units;
    /*const*/ int dry_count = mixer->buffers.buf_count; /* dry buffers count */
    /*const*/ int mix_fx_to_out = mixer->mix_fx_to_out; /* get mix_fx_to_out mode */
    
    void (*reverb_process_func)(fluid_revmodel_t *rev, const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);
    void (*chorus_process_func)(fluid_chorus_t *chorus, const fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);

    fluid_real_t *out_rev_l, *out_rev_r, *out_ch_l, *out_ch_r;

    // all dry unprocessed mono input is stored in the left channel
    fluid_real_t *in_rev = fluid_align_ptr(mixer->buffers.fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
    fluid_real_t *in_ch = in_rev;

    fluid_profile_ref_var(prof_ref);

#ifdef LADSPA

    /* Run the signal through the LADSPA Fx unit. The buffers have already been
     * set up in fluid_rvoice_mixer_set_ladspa. */
    if(mixer->ladspa_fx)
    {
        fluid_ladspa_run(mixer->ladspa_fx, current_blockcount, FLUID_BUFSIZE);
        fluid_check_fpe("LADSPA");
    }

#endif

    if(mix_fx_to_out)
    {
        // mix effects to first stereo channel
        out_ch_l = out_rev_l = fluid_align_ptr(mixer->buffers.left_buf, FLUID_DEFAULT_ALIGNMENT);
        out_ch_r = out_rev_r = fluid_align_ptr(mixer->buffers.right_buf, FLUID_DEFAULT_ALIGNMENT);

        reverb_process_func = fluid_revmodel_processmix;
        chorus_process_func = fluid_chorus_processmix;
    }
    else
    {
        // replace effects into respective stereo effects channel
        out_ch_l = out_rev_l = fluid_align_ptr(mixer->buffers.fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
        out_ch_r = out_rev_r = fluid_align_ptr(mixer->buffers.fx_right_buf, FLUID_DEFAULT_ALIGNMENT);

        reverb_process_func = fluid_revmodel_processreplace;
        chorus_process_func = fluid_chorus_processreplace;
    }

    if(mixer->with_reverb || mixer->with_chorus)
    {
#if ENABLE_MIXER_THREADS && !defined(WITH_PROFILING)
        int fx_mixer_threads = mixer->fx_units;
        fluid_clip(fx_mixer_threads, 1, mixer->thread_count + 1);
        #pragma omp parallel default(none) shared(mixer, reverb_process_func, chorus_process_func, dry_count, current_blockcount, mix_fx_to_out, fx_channels_per_unit) firstprivate(in_rev, in_ch, out_rev_l, out_rev_r, out_ch_l, out_ch_r) num_threads(fx_mixer_threads)
#endif
        {
            int i, f;
            int buf_idx;  /* buffer index */
            int samp_idx; /* sample index in buffer */
            int dry_idx = 0; /* dry buffer index */
            int sample_count; /* sample count to process */
            if(mixer->with_reverb)
            {
#if ENABLE_MIXER_THREADS && !defined(WITH_PROFILING)
                #pragma omp for schedule(static)
#endif
                for(f = 0; f < mixer->fx_units; f++)
                {
                    if(!mixer->fx[f].reverb_on)
                    {
                        continue; /* this reverb unit is disabled */
                    }

                    buf_idx = f * fx_channels_per_unit + SYNTH_REVERB_CHANNEL;
                    samp_idx = buf_idx * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE;
                    sample_count = current_blockcount * FLUID_BUFSIZE;

                    /* in mix mode, map fx out_rev at index f to a dry buffer at index dry_idx */
                    if(mix_fx_to_out)
                    {
                        /* dry buffer mapping, should be done more flexible in the future */
                        dry_idx = (f % dry_count) * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE;
                    }

                    for(i = 0; i < sample_count; i += FLUID_BUFSIZE, samp_idx += FLUID_BUFSIZE)
                    {
                        reverb_process_func(mixer->fx[f].reverb,
                                            &in_rev[samp_idx],
                                            mix_fx_to_out ? &out_rev_l[dry_idx + i] : &out_rev_l[samp_idx],
                                            mix_fx_to_out ? &out_rev_r[dry_idx + i] : &out_rev_r[samp_idx]);
                    }
                } // implicit omp barrier - required, because out_rev_l aliases with out_ch_l

                fluid_profile(FLUID_PROF_ONE_BLOCK_REVERB, prof_ref, 0,
                            current_blockcount * FLUID_BUFSIZE);
            }

            if(mixer->with_chorus)
            {
#if ENABLE_MIXER_THREADS && !defined(WITH_PROFILING)
                #pragma omp for schedule(static)
#endif
                for(f = 0; f < mixer->fx_units; f++)
                {
                    if(!mixer->fx[f].chorus_on)
                    {
                        continue; /* this chorus unit is disabled */
                    }

                    buf_idx = f * fx_channels_per_unit + SYNTH_CHORUS_CHANNEL;
                    samp_idx = buf_idx * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE;
                    sample_count = current_blockcount * FLUID_BUFSIZE;

                    /* in mix mode, map fx out_ch at index f to a dry buffer at index dry_idx */
                    if(mix_fx_to_out)
                    {
                        /* dry buffer mapping, should be done more flexible in the future */
                        dry_idx = (f % dry_count) * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE;
                    }

                    for(i = 0; i < sample_count; i += FLUID_BUFSIZE, samp_idx += FLUID_BUFSIZE)
                    {
                        chorus_process_func(mixer->fx[f].chorus,
                                            &in_ch [samp_idx],
                                            mix_fx_to_out ? &out_ch_l[dry_idx + i] : &out_ch_l[samp_idx],
                                            mix_fx_to_out ? &out_ch_r[dry_idx + i] : &out_ch_r[samp_idx]);
                    }
                }

                fluid_profile(FLUID_PROF_ONE_BLOCK_CHORUS, prof_ref, 0,
                            current_blockcount * FLUID_BUFSIZE);
            }
        }
    }
}

/**
 * Glue to get fluid_rvoice_buffers_mix what it wants
 * Note: Make sure outbufs has 2 * (buf_count + fx_buf_count) elements before calling
 */
static FLUID_INLINE int
fluid_mixer_buffers_prepare(fluid_mixer_buffers_t *buffers, fluid_real_t **outbufs)
{
    fluid_real_t *base_ptr;
    int i;
    const int fx_channels_per_unit = buffers->fx_buf_count / buffers->mixer->fx_units;
    const int offset = buffers->buf_count * 2;
    int with_reverb = buffers->mixer->with_reverb;
    int with_chorus = buffers->mixer->with_chorus;

    /* Set up the reverb and chorus buffers only when the effect is enabled or
     * when LADSPA is active. Nonexisting buffers are detected in the DSP loop.
     * Not sending the effect signals saves some time in that case. */
#ifdef LADSPA
    int with_ladspa = (buffers->mixer->ladspa_fx != NULL);
    with_reverb = (with_reverb | with_ladspa);
    with_chorus = (with_chorus | with_ladspa);
#endif

    // all the dry, non-processed mono audio for effects is to be stored in the left buffers
    base_ptr = fluid_align_ptr(buffers->fx_left_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < buffers->mixer->fx_units; i++)
    {
        int fx_idx = i * fx_channels_per_unit;

        outbufs[offset + fx_idx + SYNTH_REVERB_CHANNEL] =
            (with_reverb)
            ? &base_ptr[(fx_idx + SYNTH_REVERB_CHANNEL) * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT]
            : NULL;

        outbufs[offset + fx_idx + SYNTH_CHORUS_CHANNEL] =
            (with_chorus)
            ? &base_ptr[(fx_idx + SYNTH_CHORUS_CHANNEL) * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT]
            : NULL;
    }

    /* The output associated with a MIDI channel is wrapped around
     * using the number of audio groups as modulo divider.  This is
     * typically the number of output channels on the 'sound card',
     * as long as the LADSPA Fx unit is not used. In case of LADSPA
     * unit, think of it as subgroups on a mixer.
     *
     * For example: Assume that the number of groups is set to 2.
     * Then MIDI channel 1, 3, 5, 7 etc. go to output 1, channels 2,
     * 4, 6, 8 etc to output 2.  Or assume 3 groups: Then MIDI
     * channels 1, 4, 7, 10 etc go to output 1; 2, 5, 8, 11 etc to
     * output 2, 3, 6, 9, 12 etc to output 3.
     */
    base_ptr = fluid_align_ptr(buffers->left_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < buffers->buf_count; i++)
    {
        outbufs[i * 2] = &base_ptr[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT];
    }

    base_ptr = fluid_align_ptr(buffers->right_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < buffers->buf_count; i++)
    {
        outbufs[i * 2 + 1] = &base_ptr[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT];
    }

    return offset + buffers->fx_buf_count;
}


static FLUID_INLINE void
fluid_finish_rvoice(fluid_mixer_buffers_t *buffers, fluid_rvoice_t *rvoice)
{
    if(buffers->finished_voice_count < buffers->mixer->polyphony)
    {
        buffers->finished_voices[buffers->finished_voice_count++] = rvoice;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Exceeded finished voices array, try increasing polyphony");
    }
}

static void
fluid_mixer_buffer_process_finished_voices(fluid_mixer_buffers_t *buffers)
{
    int i, j;

    for(i = 0; i < buffers->finished_voice_count; i++)
    {
        fluid_rvoice_t *v = buffers->finished_voices[i];
        int av = buffers->mixer->active_voices;

        for(j = 0; j < av; j++)
        {
            if(v == buffers->mixer->rvoices[j])
            {
                av--;

                /* Pack the array */
                if(j < av)
                {
                    buffers->mixer->rvoices[j] = buffers->mixer->rvoices[av];
                }
            }
        }

        buffers->mixer->active_voices = av;

        fluid_rvoice_eventhandler_finished_voice_callback(buffers->mixer->eventhandler, v);
    }

    buffers->finished_voice_count = 0;
}

static FLUID_INLINE void fluid_rvoice_mixer_process_finished_voices(fluid_rvoice_mixer_t *mixer)
{
#if ENABLE_MIXER_THREADS
    int i;

    for(i = 0; i < mixer->thread_count; i++)
    {
        fluid_mixer_buffer_process_finished_voices(&mixer->threads[i]);
    }

#endif
    fluid_mixer_buffer_process_finished_voices(&mixer->buffers);
}


static FLUID_INLINE fluid_real_t *
get_dest_buf(fluid_rvoice_buffers_t *buffers, int index,
             fluid_real_t **dest_bufs, int dest_bufcount)
{
    int j = buffers->bufs[index].mapping;

    if(j >= dest_bufcount || j < 0)
    {
        return NULL;
    }

    return dest_bufs[j];
}

/**
 * Mix samples down from internal dsp_buf to output buffers
 *
 * @param buffers Destination buffer(s)
 * @param dsp_buf Mono sample source
 * @param start_block starting sample in dsp_buf
 * @param sample_count number of samples to mix following \c start_block
 * @param dest_bufs Array of buffers to mixdown to
 * @param dest_bufcount Length of dest_bufs (i.e count of buffers)
 */
static void
fluid_rvoice_buffers_mix(fluid_rvoice_buffers_t *buffers,
                         const fluid_real_t *FLUID_RESTRICT dsp_buf,
                         int start_block, int sample_count,
                         fluid_real_t **dest_bufs, int dest_bufcount)
{
    /* buffers count to mixdown to */
    int bufcount = buffers->count;
    int i, dsp_i;

    /* if there is nothing to mix, return immediately */
    if(sample_count <= 0 || dest_bufcount <= 0)
    {
        return;
    }

    FLUID_ASSERT((uintptr_t)dsp_buf % FLUID_DEFAULT_ALIGNMENT == 0);
    FLUID_ASSERT((uintptr_t)(&dsp_buf[start_block * FLUID_BUFSIZE]) % FLUID_DEFAULT_ALIGNMENT == 0);

    /* mixdown for each buffer */
    for(i = 0; i < bufcount; i++)
    {
        fluid_real_t *FLUID_RESTRICT buf = get_dest_buf(buffers, i, dest_bufs, dest_bufcount);
        fluid_real_t target_amp = buffers->bufs[i].target_amp;
        fluid_real_t current_amp = buffers->bufs[i].current_amp;
        fluid_real_t amp_incr;

        if(buf == NULL || (current_amp == 0.0f && target_amp == 0.0f))
        {
            continue;
        }

        amp_incr = (target_amp - current_amp) / FLUID_BUFSIZE;

        FLUID_ASSERT((uintptr_t)buf % FLUID_DEFAULT_ALIGNMENT == 0);

        /* Mixdown sample_count samples in the current buffer buf
         *
         * For the first FLUID_BUFSIZE samples, we linearly interpolate the buffers amplitude to
         * avoid clicks/pops when rapidly changing the channels panning (issue 768).
         * 
         * We could have squashed this into one single loop by using an if clause within the loop body.
         * But it seems like having two separate loops is easier for compilers to understand, and therefore
         * auto-vectorizing the loops.
         */
        if(sample_count < FLUID_BUFSIZE)
        {
            // scalar loop variant, the voice will have finished afterwards
            for(dsp_i = 0; dsp_i < sample_count; dsp_i++)
            {
                buf[start_block * FLUID_BUFSIZE + dsp_i] += current_amp * dsp_buf[start_block * FLUID_BUFSIZE + dsp_i];
                current_amp += amp_incr;
            }
        }
        else
        {
            // here goes the vectorizable loop
            #pragma omp simd aligned(dsp_buf,buf:FLUID_DEFAULT_ALIGNMENT)
            for(dsp_i = 0; dsp_i < FLUID_BUFSIZE; dsp_i++)
            {
                // We cannot simply increment current_amp by amp_incr during every iteration, as this would create a dependency and prevent vectorization.
                buf[start_block * FLUID_BUFSIZE + dsp_i] += (current_amp + amp_incr * dsp_i) * dsp_buf[start_block * FLUID_BUFSIZE + dsp_i];
            }
            
            // we have reached the target_amp
            if(target_amp > 0)
            {
                /* Note, that this loop could be unrolled by FLUID_BUFSIZE elements */
                #pragma omp simd aligned(dsp_buf,buf:FLUID_DEFAULT_ALIGNMENT)
                for(dsp_i = FLUID_BUFSIZE; dsp_i < sample_count; dsp_i++)
                {
                    // Index by blocks (not by samples) to let the compiler know that we always start accessing
                    // buf and dsp_buf at the FLUID_BUFSIZE*sizeof(fluid_real_t) byte boundary and never somewhere
                    // in between.
                    // A good compiler should understand: Aha, so I don't need to add a peel loop when vectorizing
                    // this loop. Great.
                    buf[start_block * FLUID_BUFSIZE + dsp_i] += target_amp * dsp_buf[start_block * FLUID_BUFSIZE + dsp_i];
                }
            }
        }
        
        buffers->bufs[i].current_amp = target_amp;
    }
}

/**
 * Synthesize one voice and add to buffer.
 * NOTE: If return value is less than blockcount*FLUID_BUFSIZE, that means
 * voice has been finished, removed and possibly replaced with another voice.
 */
static FLUID_INLINE void
fluid_mixer_buffers_render_one(fluid_mixer_buffers_t *buffers,
                               fluid_rvoice_t *rvoice, fluid_real_t **dest_bufs,
                               unsigned int dest_bufcount, fluid_real_t *src_buf, int blockcount)
{
    int i, total_samples = 0, last_block_mixed = 0;

    for(i = 0; i < blockcount; i++)
    {
        /* render one block in src_buf */
        int s = fluid_rvoice_write(rvoice, &src_buf[FLUID_BUFSIZE * i]);

        if(s == -1)
        {
            /* the voice is silent, mix back all the previously rendered sound */
            fluid_rvoice_buffers_mix(&rvoice->buffers, src_buf, last_block_mixed,
                                     total_samples - (last_block_mixed * FLUID_BUFSIZE),
                                     dest_bufs, dest_bufcount);

            last_block_mixed = i + 1; /* future block start index to mix from */
            total_samples += FLUID_BUFSIZE; /* accumulate samples count rendered */
        }
        else
        {
            /* the voice wasn't quiet. Some samples have been rendered [0..FLUID_BUFSIZE] */
            total_samples += s;

            if(s < FLUID_BUFSIZE)
            {
                /* voice has finished */
                break;
            }
        }
    }

    /* Now mix the remaining blocks from last_block_mixed to total_sample */
    fluid_rvoice_buffers_mix(&rvoice->buffers, src_buf, last_block_mixed,
                             total_samples - (last_block_mixed * FLUID_BUFSIZE),
                             dest_bufs, dest_bufcount);

    if(total_samples < blockcount * FLUID_BUFSIZE)
    {
        /* voice has finished */
        fluid_finish_rvoice(buffers, rvoice);
    }
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_add_voice)
{
    int i;
    fluid_rvoice_mixer_t *mixer = obj;
    fluid_rvoice_t *voice = param[0].ptr;

    if(mixer->active_voices < mixer->polyphony)
    {
        mixer->rvoices[mixer->active_voices++] = voice;
        return; // success
    }

    /* See if any voices just finished, if so, take its place.
       This can happen in voice overflow conditions. */
    for(i = 0; i < mixer->active_voices; i++)
    {
        if(mixer->rvoices[i] == voice)
        {
            FLUID_LOG(FLUID_ERR, "Internal error: Trying to replace an existing rvoice in fluid_rvoice_mixer_add_voice?!");
            return;
        }

        if(mixer->rvoices[i]->envlfo.volenv.section == FLUID_VOICE_ENVFINISHED)
        {
            fluid_finish_rvoice(&mixer->buffers, mixer->rvoices[i]);
            mixer->rvoices[i] = voice;
            return; // success
        }
    }

    /* This should never happen */
    FLUID_LOG(FLUID_ERR, "Trying to exceed polyphony in fluid_rvoice_mixer_add_voice");
}

static int
fluid_mixer_buffers_update_polyphony(fluid_mixer_buffers_t *buffers, int value)
{
    void *newptr;

    if(buffers->finished_voice_count > value)
    {
        return FLUID_FAILED;
    }

    newptr = FLUID_REALLOC(buffers->finished_voices, value * sizeof(fluid_rvoice_t *));

    if(newptr == NULL && value > 0)
    {
        return FLUID_FAILED;
    }

    buffers->finished_voices = newptr;
    return FLUID_OK;
}

/**
 * Update polyphony - max number of voices (NOTE: not hard real-time capable)
 * @return FLUID_OK or FLUID_FAILED
 */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_polyphony)
{
    void *newptr;
    fluid_rvoice_mixer_t *handler = obj;
    int value = param[0].i;

    if(handler->active_voices > value)
    {
        return /*FLUID_FAILED*/;
    }

    newptr = FLUID_REALLOC(handler->rvoices, value * sizeof(fluid_rvoice_t *));

    if(newptr == NULL)
    {
        return /*FLUID_FAILED*/;
    }

    handler->rvoices = newptr;

    if(fluid_mixer_buffers_update_polyphony(&handler->buffers, value)
            == FLUID_FAILED)
    {
        return /*FLUID_FAILED*/;
    }

#if ENABLE_MIXER_THREADS
    {
        int i;

        for(i = 0; i < handler->thread_count; i++)
        {
            if(fluid_mixer_buffers_update_polyphony(&handler->threads[i], value)
                    == FLUID_FAILED)
            {
                return /*FLUID_FAILED*/;
            }
        }
    }
#endif

    handler->polyphony = value;
    /*return FLUID_OK*/;
}


static void
fluid_render_loop_singlethread(fluid_rvoice_mixer_t *mixer, int blockcount)
{
    int i;
    FLUID_DECLARE_VLA(fluid_real_t *, bufs,
                      mixer->buffers.buf_count * 2 + mixer->buffers.fx_buf_count * 2);
    int bufcount = fluid_mixer_buffers_prepare(&mixer->buffers, bufs);

    fluid_real_t *local_buf = fluid_align_ptr(mixer->buffers.local_buf, FLUID_DEFAULT_ALIGNMENT);

    fluid_profile_ref_var(prof_ref);

    for(i = 0; i < mixer->active_voices; i++)
    {
        fluid_mixer_buffers_render_one(&mixer->buffers, mixer->rvoices[i], bufs,
                                       bufcount, local_buf, blockcount);
        fluid_profile(FLUID_PROF_ONE_BLOCK_VOICE, prof_ref, 1,
                      blockcount * FLUID_BUFSIZE);
    }
}

static FLUID_INLINE void
fluid_mixer_buffers_zero(fluid_mixer_buffers_t *buffers, int current_blockcount)
{
    int i, size = current_blockcount * FLUID_BUFSIZE * sizeof(fluid_real_t);

    /* TODO: Optimize by only zero out the buffers we actually use later on. */
    int buf_count = buffers->buf_count, fx_buf_count = buffers->fx_buf_count;

    fluid_real_t *FLUID_RESTRICT buf_l = fluid_align_ptr(buffers->left_buf, FLUID_DEFAULT_ALIGNMENT);
    fluid_real_t *FLUID_RESTRICT buf_r = fluid_align_ptr(buffers->right_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < buf_count; i++)
    {
        FLUID_MEMSET(&buf_l[i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE], 0, size);
        FLUID_MEMSET(&buf_r[i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE], 0, size);
    }

    buf_l = fluid_align_ptr(buffers->fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
    buf_r = fluid_align_ptr(buffers->fx_right_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < fx_buf_count; i++)
    {
        FLUID_MEMSET(&buf_l[i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE], 0, size);
        FLUID_MEMSET(&buf_r[i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE], 0, size);
    }
}

static int
fluid_mixer_buffers_init(fluid_mixer_buffers_t *buffers, fluid_rvoice_mixer_t *mixer)
{
    static const int samplecount = FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT;

    buffers->mixer = mixer;
    buffers->buf_count = mixer->buffers.buf_count;
    buffers->fx_buf_count = mixer->buffers.fx_buf_count;

    /* Local mono voice buf */
    buffers->local_buf = FLUID_ARRAY_ALIGNED(fluid_real_t, samplecount, FLUID_DEFAULT_ALIGNMENT);

    /* Left and right audio buffers */

    buffers->left_buf = FLUID_ARRAY_ALIGNED(fluid_real_t, buffers->buf_count * samplecount, FLUID_DEFAULT_ALIGNMENT);
    buffers->right_buf = FLUID_ARRAY_ALIGNED(fluid_real_t, buffers->buf_count * samplecount, FLUID_DEFAULT_ALIGNMENT);

    if((buffers->local_buf == NULL) || (buffers->left_buf == NULL) || (buffers->right_buf == NULL))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return 0;
    }

    /* Effects audio buffers */

    buffers->fx_left_buf = FLUID_ARRAY_ALIGNED(fluid_real_t, buffers->fx_buf_count * samplecount, FLUID_DEFAULT_ALIGNMENT);
    buffers->fx_right_buf = FLUID_ARRAY_ALIGNED(fluid_real_t, buffers->fx_buf_count * samplecount, FLUID_DEFAULT_ALIGNMENT);

    if((buffers->fx_left_buf == NULL) || (buffers->fx_right_buf == NULL))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return 0;
    }

    buffers->finished_voices = NULL;

    if(fluid_mixer_buffers_update_polyphony(buffers, mixer->polyphony)
            == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return 0;
    }

    return 1;
}

/**
 * Note: Not hard real-time capable (calls malloc)
 */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_samplerate)
{
    fluid_rvoice_mixer_t *mixer = obj;
    fluid_real_t samplerate = param[1].real; // because fluid_synth_update_mixer() puts real into arg2

    int i;

    for(i = 0; i < mixer->fx_units; i++)
    {
        if(mixer->fx[i].chorus)
        {
            fluid_chorus_samplerate_change(mixer->fx[i].chorus, samplerate);
        }

        if(mixer->fx[i].reverb)
        {
            fluid_revmodel_samplerate_change(mixer->fx[i].reverb, samplerate);

            /*
              fluid_revmodel_samplerate_change() shouldn't fail if the reverb was created
              with sample_rate_max set to the maximum sample rate indicated in the settings.
              If this condition isn't respected, the reverb will continue to work but with
              lost of quality.
            */
        }
    }

#if LADSPA

    if(mixer->ladspa_fx != NULL)
    {
        fluid_ladspa_set_sample_rate(mixer->ladspa_fx, samplerate);
    }

#endif
}


/**
 * @param buf_count number of primary stereo buffers
 * @param fx_buf_count number of stereo effect buffers
 */
fluid_rvoice_mixer_t *
new_fluid_rvoice_mixer(int buf_count, int fx_buf_count, int fx_units,
                       fluid_real_t sample_rate_max,
                       fluid_real_t sample_rate,
                       fluid_rvoice_eventhandler_t *evthandler,
                       int extra_threads, int prio)
{
    int i;
    fluid_rvoice_mixer_t *mixer = FLUID_NEW(fluid_rvoice_mixer_t);

    if(mixer == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(mixer, 0, sizeof(fluid_rvoice_mixer_t));
    mixer->eventhandler = evthandler;
    mixer->fx_units = fx_units;
    mixer->buffers.buf_count = buf_count;
    mixer->buffers.fx_buf_count = fx_buf_count * fx_units;

    /* allocate the reverb module */
    mixer->fx = FLUID_ARRAY(fluid_mixer_fx_t, fx_units);

    if(mixer->fx == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    FLUID_MEMSET(mixer->fx, 0, fx_units * sizeof(*mixer->fx));

    for(i = 0; i < fx_units; i++)
    {
        /* create reverb and chorus units */
        mixer->fx[i].reverb = new_fluid_revmodel(sample_rate_max, sample_rate);
        mixer->fx[i].chorus = new_fluid_chorus(sample_rate);

        if(mixer->fx[i].reverb == NULL || mixer->fx[i].chorus == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
    }

    if(!fluid_mixer_buffers_init(&mixer->buffers, mixer))
    {
        goto error_recovery;
    }

#if ENABLE_MIXER_THREADS
    mixer->thread_ready = new_fluid_cond();
    mixer->wakeup_threads = new_fluid_cond();
    mixer->thread_ready_m = new_fluid_cond_mutex();
    mixer->wakeup_threads_m = new_fluid_cond_mutex();

    if(!mixer->thread_ready || !mixer->wakeup_threads ||
            !mixer->thread_ready_m || !mixer->wakeup_threads_m)
    {
        goto error_recovery;
    }

    if(fluid_rvoice_mixer_set_threads(mixer, extra_threads, prio) != FLUID_OK)
    {
        goto error_recovery;
    }

#endif

    return mixer;

error_recovery:
    delete_fluid_rvoice_mixer(mixer);
    return NULL;
}

static void
fluid_mixer_buffers_free(fluid_mixer_buffers_t *buffers)
{
    FLUID_FREE(buffers->finished_voices);

    /* free all the sample buffers */
    FLUID_FREE(buffers->local_buf);
    FLUID_FREE(buffers->left_buf);
    FLUID_FREE(buffers->right_buf);
    FLUID_FREE(buffers->fx_left_buf);
    FLUID_FREE(buffers->fx_right_buf);
}

void delete_fluid_rvoice_mixer(fluid_rvoice_mixer_t *mixer)
{
    int i;

    fluid_return_if_fail(mixer != NULL);

#if ENABLE_MIXER_THREADS
    delete_rvoice_mixer_threads(mixer);

    if(mixer->thread_ready)
    {
        delete_fluid_cond(mixer->thread_ready);
    }

    if(mixer->wakeup_threads)
    {
        delete_fluid_cond(mixer->wakeup_threads);
    }

    if(mixer->thread_ready_m)
    {
        delete_fluid_cond_mutex(mixer->thread_ready_m);
    }

    if(mixer->wakeup_threads_m)
    {
        delete_fluid_cond_mutex(mixer->wakeup_threads_m);
    }

#endif
    fluid_mixer_buffers_free(&mixer->buffers);


    for(i = 0; i < mixer->fx_units; i++)
    {
        if(mixer->fx[i].reverb)
        {
            delete_fluid_revmodel(mixer->fx[i].reverb);
        }

        if(mixer->fx[i].chorus)
        {
            delete_fluid_chorus(mixer->fx[i].chorus);
        }
    }

    FLUID_FREE(mixer->fx);
    FLUID_FREE(mixer->rvoices);
    FLUID_FREE(mixer);
}

#ifdef LADSPA
/**
 * Set a LADSPS fx instance to be used by the mixer and assign the mixer buffers
 * as LADSPA host buffers with sensible names */
void fluid_rvoice_mixer_set_ladspa(fluid_rvoice_mixer_t *mixer,
                                   fluid_ladspa_fx_t *ladspa_fx, int audio_groups)
{
    mixer->ladspa_fx = ladspa_fx;

    if(ladspa_fx == NULL)
    {
        return;
    }
    else
    {
        fluid_real_t *main_l = fluid_align_ptr(mixer->buffers.left_buf, FLUID_DEFAULT_ALIGNMENT);
        fluid_real_t *main_r = fluid_align_ptr(mixer->buffers.right_buf, FLUID_DEFAULT_ALIGNMENT);

        fluid_real_t *rev = fluid_align_ptr(mixer->buffers.fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
        fluid_real_t *chor = rev;

        rev = &rev[SYNTH_REVERB_CHANNEL * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT];
        chor = &chor[SYNTH_CHORUS_CHANNEL * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT];

        fluid_ladspa_add_host_ports(ladspa_fx, "Main:L", audio_groups,
                                    main_l,
                                    FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT);

        fluid_ladspa_add_host_ports(ladspa_fx, "Main:R", audio_groups,
                                    main_r,
                                    FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT);

        fluid_ladspa_add_host_ports(ladspa_fx, "Reverb:Send", 1,
                                    rev,
                                    FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT);

        fluid_ladspa_add_host_ports(ladspa_fx, "Chorus:Send", 1,
                                    chor,
                                    FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT);
    }
}
#endif

/**
 * set one or more reverb shadow parameters for one fx group.
 * These parameters will be returned if queried.
 * (see fluid_rvoice_mixer_reverb_get_param())
 *
 * @param mixer that contains all fx units.
 * @param fx_group index of the fx group to which parameters must be set.
 *  must be in the range [-1..mixer->fx_units[. If -1 the changes are applied to
 *  all fx units.
 * @param set Flags indicating which parameters should be set (#fluid_revmodel_set_t)
 * @param values table of parameters values.
 */
void
fluid_rvoice_mixer_set_reverb_full(const fluid_rvoice_mixer_t *mixer,
                                   int fx_group, int set, const double values[])
{
    fluid_mixer_fx_t *fx = mixer->fx;
    int nr_units = mixer->fx_units;

    if(fx_group >= 0) /* apply parameters to this fx group only */
    {
        nr_units = fx_group + 1;
    }
    else /* apply parameters to all fx groups */
    {
        fx_group = 0;
    }

    for(; fx_group < nr_units; fx_group++)
    {
        int param;

        for(param = 0; param < FLUID_REVERB_PARAM_LAST; param++)
        {
            if(set & FLUID_REVPARAM_TO_SETFLAG(param))
            {
                fx[fx_group].reverb_param[param] = values[param];
            }
        }
    }
}

/**
 * get one reverb shadow parameter for one fx group.
 * (see fluid_rvoice_mixer_set_reverb_full())
 *
 * @param mixer that contains all fx group units.
 * @param fx_group index of the fx group to get parameter from.
 *  must be in the range [0..mixer->fx_units[.
 * @param enum indicating the parameter to get.
 *  FLUID_REVERB_ROOMSIZE, reverb room size value.
 *  FLUID_REVERB_DAMP, reverb damping value.
 *  FLUID_REVERB_WIDTH, reverb width value.
 *  FLUID_REVERB_LEVEL, reverb level value.
 * @return value.
 */
double
fluid_rvoice_mixer_reverb_get_param(const fluid_rvoice_mixer_t *mixer,
                                    int fx_group, int param)
{
    return mixer->fx[fx_group].reverb_param[param];
}

/**
 * set one or more chorus shadow parameters for one fx group.
 * These parameters will be returned if queried.
 * (see fluid_rvoice_mixer_chorus_get_param())
 *
 * @param mixer that contains all fx units.
 * @param fx_group index of the fx group to which parameters must be set.
 *  must be in the range [-1..mixer->fx_units[. If -1 the changes are applied
 *  to all fx group.
 * Keep in mind, that the needed CPU time is proportional to 'nr'.
 * @param set Flags indicating which parameters to set (#fluid_chorus_set_t)
 * @param values table of pararameters.
 */
void
fluid_rvoice_mixer_set_chorus_full(const fluid_rvoice_mixer_t *mixer,
                                   int fx_group, int set, const double values[])
{
    fluid_mixer_fx_t *fx = mixer->fx;
    int nr_units = mixer->fx_units;

    if(fx_group >= 0) /* apply parameters to this group fx only */
    {
        nr_units = fx_group + 1;
    }
    else /* apply parameters to all fx units*/
    {
        fx_group = 0;
    }

    for(; fx_group < nr_units; fx_group++)
    {
        int param;

        for(param = 0; param < FLUID_CHORUS_PARAM_LAST; param++)
        {
            if(set & FLUID_CHORPARAM_TO_SETFLAG(param))
            {
                fx[fx_group].chorus_param[param] = values[param];
            }
        }
    }
}

/**
 * get one chorus shadow parameter for one fx group.
 * (see fluid_rvoice_mixer_set_chorus_full())
 *
 * @param mixer that contains all fx groups units.
 * @param fx_group index of the fx group to get parameter from.
 *  must be in the range [0..mixer->fx_units[.
 * @param get Flags indicating which parameter to get (#fluid_chorus_set_t)
 * @return the parameter value (0.0 is returned if error)
 */
double
fluid_rvoice_mixer_chorus_get_param(const fluid_rvoice_mixer_t *mixer,
                                    int fx_group, int param)
{
    return mixer->fx[fx_group].chorus_param[param];
}

/* @deprecated: use fluid_rvoice_mixer_reverb_enable instead */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_reverb_enabled)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int on = param[0].i;

    mixer->with_reverb = on;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reverb_enable)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int fx_group = param[0].i; /* reverb fx group index */
    int on = param[1].i;       /* on/off */

    int nr_units = mixer->fx_units;

    /* does on/off must be applied only to fx group at index fx_group ? */
    if(fx_group >= 0)
    {
        mixer->fx[fx_group].reverb_on = on;
    }
    /* on/off must be applied to all fx groups */
    else
    {
        for(fx_group = 0; fx_group < nr_units; fx_group++)
        {
            mixer->fx[fx_group].reverb_on = on;
        }
    }

    /* set with_reverb if at least one reverb unit is on */
    for(fx_group = 0; fx_group < nr_units; fx_group++)
    {
        on = mixer->fx[fx_group].reverb_on;

        if(on)
        {
            break;
        }
    }

    mixer->with_reverb = on;
}

/* @deprecated: use fluid_rvoice_mixer_chorus_enable instead */
DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_chorus_enabled)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int on = param[0].i;
    mixer->with_chorus = on;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_chorus_enable)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int fx_group = param[0].i; /* chorus fx group index */
    int on = param[1].i;       /* on/off */

    int nr_units = mixer->fx_units;

    /* does on/off must be applied only to fx group at index fx_group ? */
    if(fx_group >= 0)
    {
        mixer->fx[fx_group].chorus_on = on;
    }
    /* on/off must be applied to all fx groups */
    else
    {
        for(fx_group = 0; fx_group < nr_units; fx_group++)
        {
            mixer->fx[fx_group].chorus_on = on;
        }
    }

    /* set with_chorus if at least one chorus unit is on */
    for(fx_group = 0; fx_group < nr_units; fx_group++)
    {
        on = mixer->fx[fx_group].chorus_on;

        if(on)
        {
            break;
        }
    }

    mixer->with_chorus = on;
}

void fluid_rvoice_mixer_set_mix_fx(fluid_rvoice_mixer_t *mixer, int on)
{
    mixer->mix_fx_to_out = on;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_chorus_params)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int i = param[0].i;
    int set = param[1].i;
    int nr = param[2].i;
    fluid_real_t level = param[3].real;
    fluid_real_t speed = param[4].real;
    fluid_real_t depth_ms = param[5].real;
    int type = param[6].i;

    int nr_units = mixer->fx_units;

    /* does parameters must be applied only to fx group i ? */
    if(i >= 0)
    {
        nr_units = i + 1;
    }
    else
    {
        i = 0; /* parameters must be applied to all fx groups */
    }

    while(i < nr_units)
    {
        fluid_chorus_set(mixer->fx[i++].chorus, set, nr, level, speed, depth_ms, type);
    }
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_set_reverb_params)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int i = param[0].i; /* fx group index */
    int set = param[1].i;
    fluid_real_t roomsize = param[2].real;
    fluid_real_t damping = param[3].real;
    fluid_real_t width = param[4].real;
    fluid_real_t level = param[5].real;

    int nr_units = mixer->fx_units;

    /* does parameters change should be applied only to fx group i ? */
    if(i >= 0)
    {
        nr_units = i + 1; /* parameters change must be applied to fx groups i */
    }
    else
    {
        i = 0; /* parameters change must be applied to all fx groups */
    }

    while(i < nr_units)
    {
        fluid_revmodel_set(mixer->fx[i++].reverb, set, roomsize, damping, width, level);
    }
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reset_reverb)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int i;

    for(i = 0; i < mixer->fx_units; i++)
    {
        fluid_revmodel_reset(mixer->fx[i].reverb);
    }
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_rvoice_mixer_reset_chorus)
{
    fluid_rvoice_mixer_t *mixer = obj;
    int i;

    for(i = 0; i < mixer->fx_units; i++)
    {
        fluid_chorus_reset(mixer->fx[i].chorus);
    }
}

int fluid_rvoice_mixer_get_bufs(fluid_rvoice_mixer_t *mixer,
                                fluid_real_t **left, fluid_real_t **right)
{
    *left = fluid_align_ptr(mixer->buffers.left_buf, FLUID_DEFAULT_ALIGNMENT);
    *right = fluid_align_ptr(mixer->buffers.right_buf, FLUID_DEFAULT_ALIGNMENT);
    return mixer->buffers.buf_count;
}

int fluid_rvoice_mixer_get_fx_bufs(fluid_rvoice_mixer_t *mixer,
                                   fluid_real_t **fx_left, fluid_real_t **fx_right)
{
    *fx_left = fluid_align_ptr(mixer->buffers.fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
    *fx_right = fluid_align_ptr(mixer->buffers.fx_right_buf, FLUID_DEFAULT_ALIGNMENT);
    return mixer->buffers.fx_buf_count;
}

int fluid_rvoice_mixer_get_bufcount(fluid_rvoice_mixer_t *mixer)
{
    return FLUID_MIXER_MAX_BUFFERS_DEFAULT;
}

#if WITH_PROFILING
int fluid_rvoice_mixer_get_active_voices(fluid_rvoice_mixer_t *mixer)
{
    return mixer->active_voices;
}
#endif

#if ENABLE_MIXER_THREADS

static FLUID_INLINE fluid_rvoice_t *
fluid_mixer_get_mt_rvoice(fluid_rvoice_mixer_t *mixer)
{
    int i = fluid_atomic_int_exchange_and_add(&mixer->current_rvoice, 1);

    if(i >= mixer->active_voices)
    {
        return NULL;
    }

    return mixer->rvoices[i];
}

#define THREAD_BUF_PROCESSING 0
#define THREAD_BUF_VALID 1
#define THREAD_BUF_NODATA 2
#define THREAD_BUF_TERMINATE 3

/* Core thread function (processes voices in parallel to primary synthesis thread) */
static fluid_thread_return_t
fluid_mixer_thread_func(void *data)
{
    fluid_mixer_buffers_t *buffers = data;
    fluid_rvoice_mixer_t *mixer = buffers->mixer;
    int hasValidData = 0;
    FLUID_DECLARE_VLA(fluid_real_t *, bufs, buffers->buf_count * 2 + buffers->fx_buf_count * 2);
    int bufcount = 0;
    int current_blockcount = 0;
    fluid_real_t *local_buf = fluid_align_ptr(buffers->local_buf, FLUID_DEFAULT_ALIGNMENT);

    while(!fluid_atomic_int_get(&mixer->threads_should_terminate))
    {
        fluid_rvoice_t *rvoice = fluid_mixer_get_mt_rvoice(mixer);

        if(rvoice == NULL)
        {
            // if no voices: signal rendered buffers, sleep
            fluid_atomic_int_set(&buffers->ready, hasValidData ? THREAD_BUF_VALID : THREAD_BUF_NODATA);
            fluid_cond_mutex_lock(mixer->thread_ready_m);
            fluid_cond_signal(mixer->thread_ready);
            fluid_cond_mutex_unlock(mixer->thread_ready_m);

            fluid_cond_mutex_lock(mixer->wakeup_threads_m);

            while(1)
            {
                int j = fluid_atomic_int_get(&buffers->ready);

                if(j == THREAD_BUF_PROCESSING || j == THREAD_BUF_TERMINATE)
                {
                    break;
                }

                fluid_cond_wait(mixer->wakeup_threads, mixer->wakeup_threads_m);
            }

            fluid_cond_mutex_unlock(mixer->wakeup_threads_m);

            hasValidData = 0;
        }
        else
        {
            // else: if buffer is not zeroed, zero buffers
            if(!hasValidData)
            {
                // blockcount may have changed, since thread was put to sleep
                current_blockcount = mixer->current_blockcount;
                fluid_mixer_buffers_zero(buffers, current_blockcount);
                bufcount = fluid_mixer_buffers_prepare(buffers, bufs);
                hasValidData = 1;
            }

            // then render voice to buffers
            fluid_mixer_buffers_render_one(buffers, rvoice, bufs, bufcount, local_buf, current_blockcount);
        }
    }

    return FLUID_THREAD_RETURN_VALUE;
}

static void
fluid_mixer_buffers_mix(fluid_mixer_buffers_t *dst, fluid_mixer_buffers_t *src, int current_blockcount)
{
    int i, j;
    int scount = current_blockcount * FLUID_BUFSIZE;
    int minbuf;
    fluid_real_t *FLUID_RESTRICT base_src;
    fluid_real_t *FLUID_RESTRICT base_dst;

    minbuf = dst->buf_count;

    if(minbuf > src->buf_count)
    {
        minbuf = src->buf_count;
    }

    base_src = fluid_align_ptr(src->left_buf, FLUID_DEFAULT_ALIGNMENT);
    base_dst = fluid_align_ptr(dst->left_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < minbuf; i++)
    {
        #pragma omp simd aligned(base_dst,base_src:FLUID_DEFAULT_ALIGNMENT)

        for(j = 0; j < scount; j++)
        {
            int dsp_i = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;
            base_dst[dsp_i] += base_src[dsp_i];
        }
    }

    base_src = fluid_align_ptr(src->right_buf, FLUID_DEFAULT_ALIGNMENT);
    base_dst = fluid_align_ptr(dst->right_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < minbuf; i++)
    {
        #pragma omp simd aligned(base_dst,base_src:FLUID_DEFAULT_ALIGNMENT)

        for(j = 0; j < scount; j++)
        {
            int dsp_i = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;
            base_dst[dsp_i] += base_src[dsp_i];
        }
    }

    minbuf = dst->fx_buf_count;

    if(minbuf > src->fx_buf_count)
    {
        minbuf = src->fx_buf_count;
    }

    base_src = fluid_align_ptr(src->fx_left_buf, FLUID_DEFAULT_ALIGNMENT);
    base_dst = fluid_align_ptr(dst->fx_left_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < minbuf; i++)
    {
        #pragma omp simd aligned(base_dst,base_src:FLUID_DEFAULT_ALIGNMENT)

        for(j = 0; j < scount; j++)
        {
            int dsp_i = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;
            base_dst[dsp_i] += base_src[dsp_i];
        }
    }

    base_src = fluid_align_ptr(src->fx_right_buf, FLUID_DEFAULT_ALIGNMENT);
    base_dst = fluid_align_ptr(dst->fx_right_buf, FLUID_DEFAULT_ALIGNMENT);

    for(i = 0; i < minbuf; i++)
    {
        #pragma omp simd aligned(base_dst,base_src:FLUID_DEFAULT_ALIGNMENT)

        for(j = 0; j < scount; j++)
        {
            int dsp_i = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;
            base_dst[dsp_i] += base_src[dsp_i];
        }
    }
}


/**
 * Go through all threads and see if someone is finished for mixing
 */
static int
fluid_mixer_mix_in(fluid_rvoice_mixer_t *mixer, int extra_threads, int current_blockcount)
{
    int i, result, hasmixed;

    do
    {
        hasmixed = 0;
        result = 0;

        for(i = 0; i < extra_threads; i++)
        {
            int j = fluid_atomic_int_get(&mixer->threads[i].ready);

            switch(j)
            {
            case THREAD_BUF_PROCESSING:
                result = 1;
                break;

            case THREAD_BUF_VALID:
                fluid_atomic_int_set(&mixer->threads[i].ready, THREAD_BUF_NODATA);
                fluid_mixer_buffers_mix(&mixer->buffers, &mixer->threads[i], current_blockcount);
                hasmixed = 1;
                break;
            }
        }
    }
    while(hasmixed);

    return result;
}

static void
fluid_render_loop_multithread(fluid_rvoice_mixer_t *mixer, int current_blockcount)
{
    int i, bufcount;
    fluid_real_t *local_buf = fluid_align_ptr(mixer->buffers.local_buf, FLUID_DEFAULT_ALIGNMENT);

    FLUID_DECLARE_VLA(fluid_real_t *, bufs,
                      mixer->buffers.buf_count * 2 + mixer->buffers.fx_buf_count * 2);
    // How many threads should we start this time?
    int extra_threads = mixer->active_voices / VOICES_PER_THREAD;

    if(extra_threads > mixer->thread_count)
    {
        extra_threads = mixer->thread_count;
    }

    if(extra_threads == 0)
    {
        // No extra threads? No thread overhead!
        fluid_render_loop_singlethread(mixer, current_blockcount);
        return;
    }

    bufcount = fluid_mixer_buffers_prepare(&mixer->buffers, bufs);

    // Prepare voice list
    fluid_cond_mutex_lock(mixer->wakeup_threads_m);
    fluid_atomic_int_set(&mixer->current_rvoice, 0);

    for(i = 0; i < extra_threads; i++)
    {
        fluid_atomic_int_set(&mixer->threads[i].ready, THREAD_BUF_PROCESSING);
    }

    // Signal threads to wake up
    fluid_cond_broadcast(mixer->wakeup_threads);
    fluid_cond_mutex_unlock(mixer->wakeup_threads_m);

    // If thread is finished, mix it in
    while(fluid_mixer_mix_in(mixer, extra_threads, current_blockcount))
    {
        // Otherwise get a voice and render it
        fluid_rvoice_t *rvoice = fluid_mixer_get_mt_rvoice(mixer);

        if(rvoice != NULL)
        {
            fluid_profile_ref_var(prof_ref);
            fluid_mixer_buffers_render_one(&mixer->buffers, rvoice, bufs, bufcount, local_buf, current_blockcount);
            fluid_profile(FLUID_PROF_ONE_BLOCK_VOICE, prof_ref, 1,
                          current_blockcount * FLUID_BUFSIZE);
            //test++;
        }
        else
        {
            // If no voices, wait for mixes. Make sure one is still processing to avoid deadlock
            int is_processing = 0;
            //waits++;
            fluid_cond_mutex_lock(mixer->thread_ready_m);

            for(i = 0; i < extra_threads; i++)
            {
                if(fluid_atomic_int_get(&mixer->threads[i].ready) ==
                        THREAD_BUF_PROCESSING)
                {
                    is_processing = 1;
                }
            }

            if(is_processing)
            {
                fluid_cond_wait(mixer->thread_ready, mixer->thread_ready_m);
            }

            fluid_cond_mutex_unlock(mixer->thread_ready_m);
        }
    }

    //FLUID_LOG(FLUID_DBG, "Blockcount: %d, mixed %d of %d voices myself, waits = %d",
    //	    current_blockcount, test, mixer->active_voices, waits);
}

static void delete_rvoice_mixer_threads(fluid_rvoice_mixer_t *mixer)
{
    int i;

    // if no threads have been created yet (e.g. because a previous error prevented creation of threads
    // mutexes and condition variables), skip terminating threads
    if(mixer->thread_count != 0)
    {
        fluid_atomic_int_set(&mixer->threads_should_terminate, 1);
        // Signal threads to wake up
        fluid_cond_mutex_lock(mixer->wakeup_threads_m);

        for(i = 0; i < mixer->thread_count; i++)
        {
            fluid_atomic_int_set(&mixer->threads[i].ready, THREAD_BUF_TERMINATE);
        }

        fluid_cond_broadcast(mixer->wakeup_threads);
        fluid_cond_mutex_unlock(mixer->wakeup_threads_m);

        for(i = 0; i < mixer->thread_count; i++)
        {
            if(mixer->threads[i].thread)
            {
                fluid_thread_join(mixer->threads[i].thread);
                delete_fluid_thread(mixer->threads[i].thread);
            }

            fluid_mixer_buffers_free(&mixer->threads[i]);
        }
    }

    FLUID_FREE(mixer->threads);
    mixer->thread_count = 0;
    mixer->threads = NULL;
}

/**
 * Update amount of extra mixer threads.
 * @param thread_count Number of extra mixer threads for multi-core rendering
 * @param prio_level real-time prio level for the extra mixer threads
 */
static int fluid_rvoice_mixer_set_threads(fluid_rvoice_mixer_t *mixer, int thread_count, int prio_level)
{
    char name[16];
    int i;

    // Kill all existing threads first
    if(mixer->thread_count)
    {
        delete_rvoice_mixer_threads(mixer);
    }

    if(thread_count == 0)
    {
        return FLUID_OK;
    }

    // Now prepare the new threads
    fluid_atomic_int_set(&mixer->threads_should_terminate, 0);
    mixer->threads = FLUID_ARRAY(fluid_mixer_buffers_t, thread_count);

    if(mixer->threads == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return FLUID_FAILED;
    }

    FLUID_MEMSET(mixer->threads, 0, thread_count * sizeof(fluid_mixer_buffers_t));
    mixer->thread_count = thread_count;

    for(i = 0; i < thread_count; i++)
    {
        fluid_mixer_buffers_t *b = &mixer->threads[i];

        if(!fluid_mixer_buffers_init(b, mixer))
        {
            return FLUID_FAILED;
        }

        fluid_atomic_int_set(&b->ready, THREAD_BUF_NODATA);
        FLUID_SNPRINTF(name, sizeof(name), "mixer%d", i);
        b->thread = new_fluid_thread(name, fluid_mixer_thread_func, b, prio_level, 0);

        if(!b->thread)
        {
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}
#endif

/**
 * Synthesize audio into buffers
 * @param blockcount number of blocks to render, each having FLUID_BUFSIZE samples
 * @return number of blocks rendered
 */
int
fluid_rvoice_mixer_render(fluid_rvoice_mixer_t *mixer, int blockcount)
{
    fluid_profile_ref_var(prof_ref);

    mixer->current_blockcount = blockcount;

    // Zero buffers
    fluid_mixer_buffers_zero(&mixer->buffers, blockcount);
    fluid_profile(FLUID_PROF_ONE_BLOCK_CLEAR, prof_ref, mixer->active_voices,
                  blockcount * FLUID_BUFSIZE);

#if ENABLE_MIXER_THREADS

    if(mixer->thread_count > 0)
    {
        fluid_render_loop_multithread(mixer, blockcount);
    }
    else
#endif
    {
        fluid_render_loop_singlethread(mixer, blockcount);
    }

    fluid_profile(FLUID_PROF_ONE_BLOCK_VOICES, prof_ref, mixer->active_voices,
                  blockcount * FLUID_BUFSIZE);


    // Process reverb & chorus
    fluid_rvoice_mixer_process_fx(mixer, blockcount);

    // Call the callback and pack active voice array
    fluid_rvoice_mixer_process_finished_voices(mixer);

    return blockcount;
}
