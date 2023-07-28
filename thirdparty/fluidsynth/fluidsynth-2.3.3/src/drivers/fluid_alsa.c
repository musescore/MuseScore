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

/* fluid_alsa.c
 *
 * Driver for the Advanced Linux Sound Architecture
 *
 */

#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if ALSA_SUPPORT

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <poll.h>
#include <math.h>

#include "fluid_lash.h"

#define FLUID_ALSA_DEFAULT_MIDI_DEVICE  "default"
#define FLUID_ALSA_DEFAULT_SEQ_DEVICE   "default"

#define BUFFER_LENGTH 512

/** fluid_alsa_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    snd_pcm_t *pcm;
    fluid_audio_func_t callback;
    void *data;
    int buffer_size;
    fluid_thread_t *thread;
    int cont;
} fluid_alsa_audio_driver_t;


static fluid_thread_return_t fluid_alsa_audio_run_float(void *d);
static fluid_thread_return_t fluid_alsa_audio_run_s16(void *d);


typedef struct
{
    char *name;
    snd_pcm_format_t format;
    snd_pcm_access_t access;
    fluid_thread_func_t run;
} fluid_alsa_formats_t;

static const fluid_alsa_formats_t fluid_alsa_formats[] =
{
    {
        "s16, rw, interleaved",
        SND_PCM_FORMAT_S16,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        fluid_alsa_audio_run_s16
    },
    {
        "float, rw, non interleaved",
        SND_PCM_FORMAT_FLOAT,
        SND_PCM_ACCESS_RW_NONINTERLEAVED,
        fluid_alsa_audio_run_float
    },
    { NULL, 0, 0, NULL }
};



/*
 * fluid_alsa_rawmidi_driver_t
 *
 */
typedef struct
{
    fluid_midi_driver_t driver;
    snd_rawmidi_t *rawmidi_in;
    struct pollfd *pfd;
    int npfd;
    fluid_thread_t *thread;
    fluid_atomic_int_t should_quit;
    unsigned char buffer[BUFFER_LENGTH];
    fluid_midi_parser_t *parser;
} fluid_alsa_rawmidi_driver_t;


static fluid_thread_return_t fluid_alsa_midi_run(void *d);


/*
 * fluid_alsa_seq_driver_t
 *
 */
typedef struct
{
    fluid_midi_driver_t driver;
    snd_seq_t *seq_handle;
    struct pollfd *pfd;
    int npfd;
    fluid_thread_t *thread;
    fluid_atomic_int_t should_quit;
    int port_count;
    int autoconn_inputs;
    snd_seq_addr_t autoconn_dest;
} fluid_alsa_seq_driver_t;

static fluid_thread_return_t fluid_alsa_seq_run(void *d);

/**************************************************************
 *
 *        Alsa audio driver
 *
 */

void fluid_alsa_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.alsa.device", "default", 0);
}


fluid_audio_driver_t *
new_fluid_alsa_audio_driver(fluid_settings_t *settings,
                            fluid_synth_t *synth)
{
    return new_fluid_alsa_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_alsa_audio_driver2(fluid_settings_t *settings,
                             fluid_audio_func_t func, void *data)
{
    fluid_alsa_audio_driver_t *dev;
    double sample_rate;
    int periods, period_size;
    char *device = NULL;
    int realtime_prio = 0;
    int i, err, dir = 0;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams = NULL;
    snd_pcm_uframes_t uframes;
    unsigned int tmp;

    dev = FLUID_NEW(fluid_alsa_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_audio_driver_t));

    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_dupstr(settings, "audio.alsa.device", &device);   /* ++ dup device name */
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);

    dev->data = data;
    dev->callback = func;
    dev->cont = 1;
    dev->buffer_size = period_size;

    /* Open the PCM device */
    if((err = snd_pcm_open(&dev->pcm, device ? device : "default",
                           SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) != 0)
    {
        if(err == -EBUSY)
        {
            FLUID_LOG(FLUID_ERR, "The \"%s\" audio device is used by another application",
                      device ? device : "default");
            goto error_recovery;
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Failed to open the \"%s\" audio device",
                      device ? device : "default");
            goto error_recovery;
        }
    }

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    /* Set hardware parameters. We continue trying access methods and
       sample formats until we have one that works. For example, if
       memory mapped access fails we try regular IO methods. (not
       finished, yet). */

    for(i = 0; fluid_alsa_formats[i].name != NULL; i++)
    {

        snd_pcm_hw_params_any(dev->pcm, hwparams);

        if(snd_pcm_hw_params_set_access(dev->pcm, hwparams, fluid_alsa_formats[i].access) < 0)
        {
            FLUID_LOG(FLUID_DBG, "snd_pcm_hw_params_set_access() failed with audio format '%s'", fluid_alsa_formats[i].name);
            continue;
        }

        if(snd_pcm_hw_params_set_format(dev->pcm, hwparams, fluid_alsa_formats[i].format) < 0)
        {
            FLUID_LOG(FLUID_DBG, "snd_pcm_hw_params_set_format() failed with audio format '%s'", fluid_alsa_formats[i].name);
            continue;
        }

        if((err = snd_pcm_hw_params_set_channels(dev->pcm, hwparams, 2)) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to set the channels: %s",
                      snd_strerror(err));
            goto error_recovery;
        }

        tmp = (unsigned int) sample_rate;

        if((err = snd_pcm_hw_params_set_rate_near(dev->pcm, hwparams, &tmp, NULL)) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to set the sample rate: %s",
                      snd_strerror(err));
            goto error_recovery;
        }

        if(tmp != sample_rate)
        {
            /* There's currently no way to change the sampling rate of the
            synthesizer after it's been created. */
            FLUID_LOG(FLUID_WARN, "Requested sample rate of %u, got %u instead, "
                      "synthesizer likely out of tune!", (unsigned int) sample_rate, tmp);
        }

        uframes = period_size;

        if(snd_pcm_hw_params_set_period_size_near(dev->pcm, hwparams, &uframes, &dir) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to set the period size");
            goto error_recovery;
        }

        if(uframes != (unsigned long) period_size)
        {
            FLUID_LOG(FLUID_WARN, "Requested a period size of %d, got %d instead",
                      period_size, (int) uframes);
            dev->buffer_size = (int) uframes;
            period_size = uframes;	/* period size is used below, so set it to the real value */
        }

        tmp = periods;

        if(snd_pcm_hw_params_set_periods_near(dev->pcm, hwparams, &tmp, &dir) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to set the number of periods");
            goto error_recovery;
        }

        if(tmp != (unsigned int) periods)
        {
            FLUID_LOG(FLUID_WARN, "Requested %d periods, got %d instead",
                      periods, (int) tmp);
        }

        if(snd_pcm_hw_params(dev->pcm, hwparams) < 0)
        {
            FLUID_LOG(FLUID_WARN, "Audio device hardware configuration failed");
            continue;
        }

        break;
    }

    if(fluid_alsa_formats[i].name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to find an audio format supported by alsa");
        goto error_recovery;
    }

    /* Set the software params */
    snd_pcm_sw_params_current(dev->pcm, swparams);

    if(snd_pcm_sw_params_set_start_threshold(dev->pcm, swparams, period_size) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set start threshold.");
    }

    if(snd_pcm_sw_params_set_avail_min(dev->pcm, swparams, period_size) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Software setup for minimum available frames failed.");
    }

    if(snd_pcm_sw_params(dev->pcm, swparams) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Software setup failed.");
    }

    if(snd_pcm_nonblock(dev->pcm, 0) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set the audio device to blocking mode");
        goto error_recovery;
    }

    /* Create the audio thread */
    dev->thread = new_fluid_thread("alsa-audio", fluid_alsa_formats[i].run, dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        FLUID_LOG(FLUID_ERR, "Failed to start the alsa-audio thread.");
        goto error_recovery;
    }

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    delete_fluid_alsa_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

void delete_fluid_alsa_audio_driver(fluid_audio_driver_t *p)
{
    fluid_alsa_audio_driver_t *dev = (fluid_alsa_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    dev->cont = 0;

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->pcm)
    {
        snd_pcm_close(dev->pcm);
    }

    FLUID_FREE(dev);
}

/* handle error after an ALSA write call */
static int fluid_alsa_handle_write_error(snd_pcm_t *pcm, int errval)
{
    switch(errval)
    {
    case -EAGAIN:
        snd_pcm_wait(pcm, 1);
        break;
// on some BSD variants ESTRPIPE is defined as EPIPE.
// not sure why, maybe because this version of alsa doesn't support
// suspending pcm streams. anyway, since EPIPE seems to be more
// likely than ESTRPIPE, so ifdef it out in case.
#if ESTRPIPE == EPIPE
#warning "ESTRPIPE defined as EPIPE. This may cause trouble with ALSA playback."
#else

    case -ESTRPIPE:
        if(snd_pcm_resume(pcm) != 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to resume the audio device");
            return FLUID_FAILED;
        }

#endif

    /* fall through ... */
    /* ... since the stream got resumed, but still has to be prepared */
    case -EPIPE:
    case -EBADFD:
        if(snd_pcm_prepare(pcm) != 0)
        {
            FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
            return FLUID_FAILED;
        }

        break;

    default:
        FLUID_LOG(FLUID_ERR, "The audio device error: %s", snd_strerror(errval));
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

static fluid_thread_return_t fluid_alsa_audio_run_float(void *d)
{
    fluid_alsa_audio_driver_t *dev = (fluid_alsa_audio_driver_t *) d;
    fluid_synth_t *synth = (fluid_synth_t *)(dev->data);
    float *left;
    float *right;
    float *handle[2];
    int n, buffer_size, offset;

    buffer_size = dev->buffer_size;

    left = FLUID_ARRAY(float, buffer_size);
    right = FLUID_ARRAY(float, buffer_size);

    if((left == NULL) || (right == NULL))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    if(snd_pcm_prepare(dev->pcm) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
        goto error_recovery;
    }

    /* use separate loops depending on if callback supplied or not (overkill?) */
    if(dev->callback)
    {
        while(dev->cont)
        {
            FLUID_MEMSET(left, 0, buffer_size * sizeof(*left));
            FLUID_MEMSET(right, 0, buffer_size * sizeof(*right));

            handle[0] = left;
            handle[1] = right;

            (*dev->callback)(synth, buffer_size, 0, NULL, 2, handle);

            offset = 0;

            while(offset < buffer_size)
            {
                handle[0] = left + offset;
                handle[1] = right + offset;

                n = snd_pcm_writen(dev->pcm, (void *)handle, buffer_size - offset);

                if(n < 0)	/* error occurred? */
                {
                    if(fluid_alsa_handle_write_error(dev->pcm, n) != FLUID_OK)
                    {
                        goto error_recovery;
                    }
                }
                else
                {
                    offset += n;    /* no error occurred */
                }
            }	/* while (offset < buffer_size) */
        }	/* while (dev->cont) */
    }
    else	/* no user audio callback (faster) */
    {
        while(dev->cont)
        {
            fluid_synth_write_float(dev->data, buffer_size, left, 0, 1, right, 0, 1);

            offset = 0;

            while(offset < buffer_size)
            {
                handle[0] = left + offset;
                handle[1] = right + offset;

                n = snd_pcm_writen(dev->pcm, (void *)handle, buffer_size - offset);

                if(n < 0)	/* error occurred? */
                {
                    if(fluid_alsa_handle_write_error(dev->pcm, n) != FLUID_OK)
                    {
                        goto error_recovery;
                    }
                }
                else
                {
                    offset += n;    /* no error occurred */
                }
            }	/* while (offset < buffer_size) */
        }	/* while (dev->cont) */
    }

error_recovery:

    FLUID_FREE(left);
    FLUID_FREE(right);

    return FLUID_THREAD_RETURN_VALUE;
}

static fluid_thread_return_t fluid_alsa_audio_run_s16(void *d)
{
    fluid_alsa_audio_driver_t *dev = (fluid_alsa_audio_driver_t *) d;
    float *left;
    float *right;
    short *buf;
    float *handle[2];
    int n, buffer_size, offset;

    buffer_size = dev->buffer_size;

    left = FLUID_ARRAY(float, buffer_size);
    right = FLUID_ARRAY(float, buffer_size);
    buf = FLUID_ARRAY(short, 2 * buffer_size);

    if((left == NULL) || (right == NULL) || (buf == NULL))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    handle[0] = left;
    handle[1] = right;

    if(snd_pcm_prepare(dev->pcm) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to prepare the audio device");
        goto error_recovery;
    }

    /* use separate loops depending on if callback supplied or not */
    if(dev->callback)
    {
        int dither_index = 0;

        while(dev->cont)
        {
            FLUID_MEMSET(left, 0, buffer_size * sizeof(*left));
            FLUID_MEMSET(right, 0, buffer_size * sizeof(*right));

            (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, handle);

            /* convert floating point data to 16 bit (with dithering) */
            fluid_synth_dither_s16(&dither_index, buffer_size, left, right,
                                   buf, 0, 2, buf, 1, 2);
            offset = 0;

            while(offset < buffer_size)
            {
                n = snd_pcm_writei(dev->pcm, (void *)(buf + 2 * offset),
                                   buffer_size - offset);

                if(n < 0)	/* error occurred? */
                {
                    if(fluid_alsa_handle_write_error(dev->pcm, n) != FLUID_OK)
                    {
                        goto error_recovery;
                    }
                }
                else
                {
                    offset += n;    /* no error occurred */
                }
            }	/* while (offset < buffer_size) */
        }	/* while (dev->cont) */
    }
    else	/* no user audio callback, dev->data is the synth instance */
    {
        fluid_synth_t *synth = (fluid_synth_t *)(dev->data);

        while(dev->cont)
        {
            fluid_synth_write_s16(synth, buffer_size, buf, 0, 2, buf, 1, 2);

            offset = 0;

            while(offset < buffer_size)
            {
                n = snd_pcm_writei(dev->pcm, (void *)(buf + 2 * offset),
                                   buffer_size - offset);

                if(n < 0)	/* error occurred? */
                {
                    if(fluid_alsa_handle_write_error(dev->pcm, n) != FLUID_OK)
                    {
                        goto error_recovery;
                    }
                }
                else
                {
                    offset += n;    /* no error occurred */
                }
            }	/* while (offset < buffer_size) */
        }	/* while (dev->cont) */
    }

error_recovery:

    FLUID_FREE(left);
    FLUID_FREE(right);
    FLUID_FREE(buf);

    return FLUID_THREAD_RETURN_VALUE;
}


/**************************************************************
 *
 *        Alsa MIDI driver
 *
 */


void fluid_alsa_rawmidi_driver_settings(fluid_settings_t *settings)
{
    int card = -1;
    int err = 0;
    snd_rawmidi_info_t *info;

    fluid_settings_register_str(settings, "midi.alsa.device", "default", 0);
    fluid_settings_add_option(settings, "midi.alsa.device", "default");

    snd_rawmidi_info_alloca(&info);

    /* Enumeration of ALSA Raw MIDI devices */
    err = snd_card_next(&card);

    while((err == 0) && (card >= 0))
    {
        int device = -1;
        snd_ctl_t *ctl = NULL;
        char card_name[32];

        FLUID_SNPRINTF(card_name, sizeof(card_name), "hw:%d", card);
        err = snd_ctl_open(&ctl, card_name, 0);

        if(err == 0)
        {
            for(;;)
            {
                int num_subs;
                int subdevice;
                char newoption[64];
                const char *name;
                const char *sub_name;

                err = snd_ctl_rawmidi_next_device(ctl, &device);

                if((err < 0) || (device < 0))
                {
                    break;
                }

                snd_rawmidi_info_set_device(info, device);
                snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
                err = snd_ctl_rawmidi_info(ctl, info);

                if(err == 0)
                {
                    num_subs = snd_rawmidi_info_get_subdevices_count(info);
                }
                else
                {
                    break;
                }

                for(subdevice = 0; subdevice < num_subs; ++subdevice)
                {
                    snd_rawmidi_info_set_subdevice(info, subdevice);
                    err = snd_ctl_rawmidi_info(ctl, info);

                    if(err == 0)
                    {
                        name = snd_rawmidi_info_get_name(info);
                        sub_name = snd_rawmidi_info_get_subdevice_name(info);

                        if(subdevice == 0 && sub_name[0] == '\0')
                        {
                            FLUID_SNPRINTF(newoption, sizeof(newoption), "hw:%d,%d    %s", card, device, name);
                            fluid_settings_add_option(settings, "midi.alsa.device", newoption);
                            break;
                        }
                        else
                        {
                            FLUID_SNPRINTF(newoption, sizeof(newoption), "hw:%d,%d,%d  %s", card, device, subdevice, sub_name);
                            fluid_settings_add_option(settings, "midi.alsa.device", newoption);
                        }
                    }
                }
            }
        }

        if(ctl)
        {
            snd_ctl_close(ctl);
            ctl = NULL;
        }

        err = snd_card_next(&card);
    }
}

/*
 * new_fluid_alsa_rawmidi_driver
 */
fluid_midi_driver_t *
new_fluid_alsa_rawmidi_driver(fluid_settings_t *settings,
                              handle_midi_event_func_t handler,
                              void *data)
{
    int i, err;
    fluid_alsa_rawmidi_driver_t *dev;
    int realtime_prio = 0;
    int count;
    struct pollfd *pfd = NULL;
    char *device = NULL;

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    /* allocate the device */
    dev = FLUID_NEW(fluid_alsa_rawmidi_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_rawmidi_driver_t));

    dev->driver.handler = handler;
    dev->driver.data = data;

    /* allocate one event to store the input data */
    dev->parser = new_fluid_midi_parser();

    if(dev->parser == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    fluid_settings_getint(settings, "midi.realtime-prio", &realtime_prio);

    /* get the device name. if none is specified, use the default device. */
    fluid_settings_dupstr(settings, "midi.alsa.device", &device);         /* ++ alloc device name */

    /* open the hardware device. only use midi in. */
    if((err = snd_rawmidi_open(&dev->rawmidi_in, NULL, device ? device : "default",
                               SND_RAWMIDI_NONBLOCK)) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Error opening ALSA raw MIDI port: %s", snd_strerror(err));
        goto error_recovery;
    }

    snd_rawmidi_nonblock(dev->rawmidi_in, 1);

    /* get # of MIDI file descriptors */
    count = snd_rawmidi_poll_descriptors_count(dev->rawmidi_in);

    if(count > 0)  		/* make sure there are some */
    {
        pfd = FLUID_MALLOC(sizeof(struct pollfd) * count);
        dev->pfd = FLUID_MALLOC(sizeof(struct pollfd) * count);
        /* grab file descriptor POLL info structures */
        count = snd_rawmidi_poll_descriptors(dev->rawmidi_in, pfd, count);
    }

    /* copy the input FDs */
    for(i = 0; i < count; i++)  		/* loop over file descriptors */
    {
        if(pfd[i].events & POLLIN)    /* use only the input FDs */
        {
            dev->pfd[dev->npfd].fd = pfd[i].fd;
            dev->pfd[dev->npfd].events = POLLIN;
            dev->pfd[dev->npfd].revents = 0;
            dev->npfd++;
        }
    }

    FLUID_FREE(pfd);

    fluid_atomic_int_set(&dev->should_quit, 0);

    /* create the MIDI thread */
    dev->thread = new_fluid_thread("alsa-midi-raw", fluid_alsa_midi_run, dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        goto error_recovery;
    }

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    return (fluid_midi_driver_t *) dev;

error_recovery:

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    delete_fluid_alsa_rawmidi_driver((fluid_midi_driver_t *) dev);
    return NULL;

}

/*
 * delete_fluid_alsa_rawmidi_driver
 */
void
delete_fluid_alsa_rawmidi_driver(fluid_midi_driver_t *p)
{
    fluid_alsa_rawmidi_driver_t *dev = (fluid_alsa_rawmidi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    /* cancel the thread and wait for it before cleaning up */
    fluid_atomic_int_set(&dev->should_quit, 1);

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->rawmidi_in)
    {
        snd_rawmidi_close(dev->rawmidi_in);
    }

    if(dev->parser != NULL)
    {
        delete_fluid_midi_parser(dev->parser);
    }

    FLUID_FREE(dev);
}

/*
 * fluid_alsa_midi_run
 */
fluid_thread_return_t
fluid_alsa_midi_run(void *d)
{
    fluid_midi_event_t *evt;
    fluid_alsa_rawmidi_driver_t *dev = (fluid_alsa_rawmidi_driver_t *) d;
    int n, i;

    /* go into a loop until someone tells us to stop */
    while(!fluid_atomic_int_get(&dev->should_quit))
    {

        /* is there something to read? */
        n = poll(dev->pfd, dev->npfd, 100); /* use a 100 milliseconds timeout */

        if(n < 0)
        {
            perror("poll");
        }
        else if(n > 0)
        {

            /* read new data */
            n = snd_rawmidi_read(dev->rawmidi_in, dev->buffer, BUFFER_LENGTH);

            if((n < 0) && (n != -EAGAIN))
            {
                FLUID_LOG(FLUID_ERR, "Failed to read the midi input");
                fluid_atomic_int_set(&dev->should_quit, 1);
            }

            /* let the parser convert the data into events */
            for(i = 0; i < n; i++)
            {
                evt = fluid_midi_parser_parse(dev->parser, dev->buffer[i]);

                if(evt != NULL)
                {
                    (*dev->driver.handler)(dev->driver.data, evt);
                }
            }
        }
    }

    return FLUID_THREAD_RETURN_VALUE;
}

/**************************************************************
 *
 *        Alsa sequencer
 *
 */


void fluid_alsa_seq_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "midi.alsa_seq.device", "default", 0);
    fluid_settings_register_str(settings, "midi.alsa_seq.id", "pid", 0);
}


static char *fluid_alsa_seq_full_id(char *id, char *buf, int len)
{
    if(id != NULL)
    {
        if(FLUID_STRCMP(id, "pid") == 0)
        {
            FLUID_SNPRINTF(buf, len, "FLUID Synth (%d)", getpid());
        }
        else
        {
            FLUID_SNPRINTF(buf, len, "FLUID Synth (%s)", id);
        }
    }
    else
    {
        FLUID_SNPRINTF(buf, len, "FLUID Synth");
    }

    return buf;
}

static char *fluid_alsa_seq_full_name(char *id, int port, char *buf, int len)
{
    if(id != NULL)
    {
        if(FLUID_STRCMP(id, "pid") == 0)
        {
            FLUID_SNPRINTF(buf, len, "Synth input port (%d:%d)", getpid(), port);
        }
        else
        {
            FLUID_SNPRINTF(buf, len, "Synth input port (%s:%d)", id, port);
        }
    }
    else
    {
        FLUID_SNPRINTF(buf, len, "Synth input port");
    }

    return buf;
}

// Connect a single port_info to autoconnect_dest if it has right type/capabilities
static void fluid_alsa_seq_autoconnect_port_info(fluid_alsa_seq_driver_t *dev, snd_seq_port_info_t *pinfo)
{
    snd_seq_port_subscribe_t *subs;
    snd_seq_t *seq = dev->seq_handle;
    const unsigned int needed_type = SND_SEQ_PORT_TYPE_MIDI_GENERIC;
    const unsigned int needed_cap = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
    const snd_seq_addr_t *sender = snd_seq_port_info_get_addr(pinfo);
    const char *pname = snd_seq_port_info_get_name(pinfo);

    if((snd_seq_port_info_get_type(pinfo) & needed_type) != needed_type)
    {
        return;
    }

    if((snd_seq_port_info_get_capability(pinfo) & needed_cap) != needed_cap)
    {
        return;
    }

    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, sender);
    snd_seq_port_subscribe_set_dest(subs, &dev->autoconn_dest);

    if(snd_seq_get_port_subscription(seq, subs) == 0)
    {
        FLUID_LOG(FLUID_WARN, "Connection %s is already subscribed", pname);
        return;
    }

    if(snd_seq_subscribe_port(seq, subs) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Connection of %s failed (%s)", pname, snd_strerror(errno));
        return;
    }

    FLUID_LOG(FLUID_INFO, "Connection of %s succeeded", pname);

    /* Calculate the next port to be auto-connected. When all ports have been connected
     * in sequence, start again from port 0 for the next auto-connection. */
    dev->autoconn_dest.port++;

    if(dev->autoconn_dest.port >= dev->port_count)
    {
        dev->autoconn_dest.port  = 0;
    }
}

// Autoconnect a single client port (by id) to autoconnect_dest if it has right type/capabilities
static void fluid_alsa_seq_autoconnect_port(fluid_alsa_seq_driver_t *dev, int client_id, int port_id)
{
    int err;
    snd_seq_t *seq = dev->seq_handle;
    snd_seq_port_info_t *pinfo;

    snd_seq_port_info_alloca(&pinfo);

    if((err = snd_seq_get_any_port_info(seq, client_id, port_id, pinfo)) < 0)
    {
        FLUID_LOG(FLUID_ERR, "snd_seq_get_any_port_info() failed: %s", snd_strerror(err));
        return;
    }

    fluid_alsa_seq_autoconnect_port_info(dev, pinfo);
}

// Connect available ALSA MIDI inputs
static void fluid_alsa_seq_autoconnect(fluid_alsa_seq_driver_t *dev)
{
    int err;
    snd_seq_t *seq = dev->seq_handle;
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    /* Subscribe to system:announce for future new clients/ports showing up.
     * This subscription is made to the first port, and does not rotate
     * dev->autoconn_dest, because this subscription never provides
     * MIDI channel messages, only system (ALSA) messages. */
    if((err = snd_seq_connect_from(seq, dev->autoconn_dest.port,
                                   SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE)) < 0)
    {
        FLUID_LOG(FLUID_ERR, "snd_seq_connect_from() failed: %s", snd_strerror(err));
    }

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);

    while(snd_seq_query_next_client(seq, cinfo) >= 0)
    {
        snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
        snd_seq_port_info_set_port(pinfo, -1);

        while(snd_seq_query_next_port(seq, pinfo) >= 0)
        {
            fluid_alsa_seq_autoconnect_port_info(dev, pinfo);
        }
    }
}

/*
 * new_fluid_alsa_seq_driver
 */
fluid_midi_driver_t *
new_fluid_alsa_seq_driver(fluid_settings_t *settings,
                          handle_midi_event_func_t handler, void *data)
{
    int i, err;
    fluid_alsa_seq_driver_t *dev;
    int realtime_prio = 0;
    int count;
    struct pollfd *pfd = NULL;
    char *device = NULL;
    char *id = NULL;
    char *portname = NULL;
    char full_id[64];
    char full_name[64];
    snd_seq_port_info_t *port_info = NULL;
    int midi_channels;

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    /* allocate the device */
    dev = FLUID_NEW(fluid_alsa_seq_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_alsa_seq_driver_t));
    dev->driver.data = data;
    dev->driver.handler = handler;

    fluid_settings_getint(settings, "midi.realtime-prio", &realtime_prio);

    /* get the device name. if none is specified, use the default device. */
    if(fluid_settings_dupstr(settings, "midi.alsa_seq.device", &device) != FLUID_OK)     /* ++ alloc device name */
    {
        goto error_recovery;
    }

    if(fluid_settings_dupstr(settings, "midi.alsa_seq.id", &id) != FLUID_OK)     /* ++ alloc id string */
    {
        goto error_recovery;
    }

    if(id == NULL)
    {
        id = FLUID_MALLOC(32);

        if(!id)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }

        sprintf(id, "%d", getpid());
    }

    /* get the midi portname */
    fluid_settings_dupstr(settings, "midi.portname", &portname);

    if(portname && FLUID_STRLEN(portname) == 0)
    {
        FLUID_FREE(portname);       /* -- free port name */
        portname = NULL;
    }

    /* open the sequencer INPUT only */
    err = snd_seq_open(&dev->seq_handle, device ? device : "default", SND_SEQ_OPEN_INPUT, 0);

    if(err < 0)
    {
        FLUID_LOG(FLUID_ERR, "Error opening ALSA sequencer");
        goto error_recovery;
    }

    snd_seq_nonblock(dev->seq_handle, 1);

    /* get # of MIDI file descriptors */
    count = snd_seq_poll_descriptors_count(dev->seq_handle, POLLIN);

    if(count > 0)  		/* make sure there are some */
    {
        pfd = FLUID_MALLOC(sizeof(struct pollfd) * count);
        dev->pfd = FLUID_MALLOC(sizeof(struct pollfd) * count);
        /* grab file descriptor POLL info structures */
        count = snd_seq_poll_descriptors(dev->seq_handle, pfd, count, POLLIN);
    }

    /* copy the input FDs */
    for(i = 0; i < count; i++)  		/* loop over file descriptors */
    {
        if(pfd[i].events & POLLIN)    /* use only the input FDs */
        {
            dev->pfd[dev->npfd].fd = pfd[i].fd;
            dev->pfd[dev->npfd].events = POLLIN;
            dev->pfd[dev->npfd].revents = 0;
            dev->npfd++;
        }
    }

    FLUID_FREE(pfd);

    /* set the client name */
    if(!portname)
    {
        snd_seq_set_client_name(dev->seq_handle, fluid_alsa_seq_full_id(id, full_id, 64));
    }
    else
    {
        snd_seq_set_client_name(dev->seq_handle, portname);
    }


    /* create the ports */
    snd_seq_port_info_alloca(&port_info);
    FLUID_MEMSET(port_info, 0, snd_seq_port_info_sizeof());

    fluid_settings_getint(settings, "synth.midi-channels", &midi_channels);
    dev->port_count = ceil(midi_channels / 16.0f);

    snd_seq_port_info_set_capability(port_info,
                                     SND_SEQ_PORT_CAP_WRITE |
                                     SND_SEQ_PORT_CAP_SUBS_WRITE);
    snd_seq_port_info_set_type(port_info,
                               SND_SEQ_PORT_TYPE_MIDI_GM      |
                               SND_SEQ_PORT_TYPE_SYNTHESIZER  |
                               SND_SEQ_PORT_TYPE_APPLICATION  |
                               SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    snd_seq_port_info_set_midi_channels(port_info, 16);
    snd_seq_port_info_set_port_specified(port_info, 1);

    for(i = 0; i < dev->port_count; i++)
    {

        if(!portname)
        {
            snd_seq_port_info_set_name(port_info, fluid_alsa_seq_full_name(id, i, full_name, 64));
        }
        else
        {
            snd_seq_port_info_set_name(port_info, portname);
        }

        snd_seq_port_info_set_port(port_info, i);

        err = snd_seq_create_port(dev->seq_handle, port_info);

        if(err  < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error creating ALSA sequencer port");
            goto error_recovery;
        }
    }

    fluid_settings_getint(settings, "midi.autoconnect", &dev->autoconn_inputs);

    if(dev->autoconn_inputs)
    {
        /* The next port to be auto-connected, starting by port 0 */
        dev->autoconn_dest = *snd_seq_port_info_get_addr(port_info);
        dev->autoconn_dest.port = 0;
        fluid_alsa_seq_autoconnect(dev);
    }

    /* tell the lash server our client id */
#ifdef HAVE_LASH
    {
        int enable_lash = 0;
        fluid_settings_getint(settings, "lash.enable", &enable_lash);

        if(enable_lash)
        {
            fluid_lash_alsa_client_id(fluid_lash_client, snd_seq_client_id(dev->seq_handle));
        }
    }
#endif /* HAVE_LASH */

    fluid_atomic_int_set(&dev->should_quit, 0);

    /* create the MIDI thread */
    dev->thread = new_fluid_thread("alsa-midi-seq", fluid_alsa_seq_run, dev, realtime_prio, FALSE);

    if(portname)
    {
        FLUID_FREE(portname);
    }

    if(id)
    {
        FLUID_FREE(id);
    }

    if(device)
    {
        FLUID_FREE(device);
    }

    return (fluid_midi_driver_t *) dev;

error_recovery:

    if(portname)
    {
        FLUID_FREE(portname);
    }

    if(id)
    {
        FLUID_FREE(id);
    }

    if(device)
    {
        FLUID_FREE(device);
    }

    delete_fluid_alsa_seq_driver((fluid_midi_driver_t *) dev);

    return NULL;
}

/*
 * delete_fluid_alsa_seq_driver
 */
void
delete_fluid_alsa_seq_driver(fluid_midi_driver_t *p)
{
    fluid_alsa_seq_driver_t *dev = (fluid_alsa_seq_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    /* cancel the thread and wait for it before cleaning up */
    fluid_atomic_int_set(&dev->should_quit, 1);

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->seq_handle)
    {
        snd_seq_close(dev->seq_handle);
    }

    if(dev->pfd)
    {
        FLUID_FREE(dev->pfd);
    }

    FLUID_FREE(dev);
}

/*
 * fluid_alsa_seq_run
 */
fluid_thread_return_t
fluid_alsa_seq_run(void *d)
{
    int n, ev;
    snd_seq_event_t *seq_ev;
    fluid_midi_event_t evt;
    fluid_alsa_seq_driver_t *dev = (fluid_alsa_seq_driver_t *) d;

    /* go into a loop until someone tells us to stop */
    while(!fluid_atomic_int_get(&dev->should_quit))
    {

        /* is there something to read? */
        n = poll(dev->pfd, dev->npfd, 100); /* use a 100 milliseconds timeout */

        if(n < 0)
        {
            perror("poll");
        }
        else if(n > 0)           /* check for pending events */
        {
            do
            {
                ev = snd_seq_event_input(dev->seq_handle, &seq_ev);	/* read the events */

                if(ev == -EAGAIN)
                {
                    break;
                }

                /* Negative value indicates an error, ignore interrupted system call
                 * (-EPERM) and input event buffer overrun (-ENOSPC) */
                if(ev < 0)
                {
                    /* FIXME - report buffer overrun? */
                    if(ev != -EPERM && ev != -ENOSPC)
                    {
                        FLUID_LOG(FLUID_ERR, "Error while reading ALSA sequencer (code=%d)", ev);
                        fluid_atomic_int_set(&dev->should_quit, 1);
                    }

                    break;
                }

                switch(seq_ev->type)
                {
                case SND_SEQ_EVENT_NOTEON:
                    evt.type = NOTE_ON;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
                    evt.param1 = seq_ev->data.note.note;
                    evt.param2 = seq_ev->data.note.velocity;
                    break;

                case SND_SEQ_EVENT_NOTEOFF:
                    evt.type = NOTE_OFF;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
                    evt.param1 = seq_ev->data.note.note;
                    evt.param2 = seq_ev->data.note.velocity;
                    break;

                case SND_SEQ_EVENT_KEYPRESS:
                    evt.type = KEY_PRESSURE;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.note.channel;
                    evt.param1 = seq_ev->data.note.note;
                    evt.param2 = seq_ev->data.note.velocity;
                    break;

                case SND_SEQ_EVENT_CONTROLLER:
                    evt.type = CONTROL_CHANGE;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
                    evt.param1 = seq_ev->data.control.param;
                    evt.param2 = seq_ev->data.control.value;
                    break;

                case SND_SEQ_EVENT_PITCHBEND:
                    evt.type = PITCH_BEND;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;

                    /* ALSA pitch bend is -8192 - 8191, we adjust it here */
                    evt.param1 = seq_ev->data.control.value + 8192;
                    break;

                case SND_SEQ_EVENT_PGMCHANGE:
                    evt.type = PROGRAM_CHANGE;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
                    evt.param1 = seq_ev->data.control.value;
                    break;

                case SND_SEQ_EVENT_CHANPRESS:
                    evt.type = CHANNEL_PRESSURE;
                    evt.channel = seq_ev->dest.port * 16 + seq_ev->data.control.channel;
                    evt.param1 = seq_ev->data.control.value;
                    break;

                case SND_SEQ_EVENT_SYSEX:
                    if(seq_ev->data.ext.len < 2)
                    {
                        continue;
                    }

                    fluid_midi_event_set_sysex(&evt, (char *)(seq_ev->data.ext.ptr) + 1,
                                               seq_ev->data.ext.len - 2, FALSE);
                    break;

                case SND_SEQ_EVENT_PORT_START:
                {
                    if(dev->autoconn_inputs)
                    {
                        fluid_alsa_seq_autoconnect_port(dev, seq_ev->data.addr.client, seq_ev->data.addr.port);
                    }
                }
                break;

                case SND_SEQ_EVENT_START:
                    evt.type = MIDI_START;
                    break;

                case SND_SEQ_EVENT_CONTINUE:
                    evt.type = MIDI_CONTINUE;
                    break;

                case SND_SEQ_EVENT_STOP:
                    evt.type = MIDI_STOP;
                    break;

                case SND_SEQ_EVENT_CLOCK:
                    evt.type = MIDI_SYNC;
                    break;

                case SND_SEQ_EVENT_RESET:
                    evt.type = MIDI_SYSTEM_RESET;
                    break;

                default:
                    continue;		/* unhandled event, next loop iteration */
                }

                /* send the events to the next link in the chain */
                (*dev->driver.handler)(dev->driver.data, &evt);
            }
            while(ev > 0);
        }	/* if poll() > 0 */
    }	/* while (!dev->should_quit) */

    return FLUID_THREAD_RETURN_VALUE;
}

#endif /* #if ALSA_SUPPORT */
