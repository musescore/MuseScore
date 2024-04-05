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


/* fluid_oss.c
 *
 * Drivers for the Open (?) Sound System
 */

#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if OSS_SUPPORT

#if defined(HAVE_SYS_SOUNDCARD_H)
#include <sys/soundcard.h>
#elif defined(HAVE_LINUX_SOUNDCARD_H)
#include <linux/soundcard.h>
#else
#include <machine/soundcard.h>
#endif

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#define BUFFER_LENGTH 512

// Build issue on some systems (OSS 4.0)?
#if (defined(SOUND_VERSION) && SOUND_VERSION >= 0x040000) || (!defined(SOUND_PCM_WRITE_CHANNELS) && defined(SNDCTL_DSP_CHANNELS))
#define OSS_CHANNELS_PLACEHOLDER        SNDCTL_DSP_CHANNELS
#else
#define OSS_CHANNELS_PLACEHOLDER        SOUND_PCM_WRITE_CHANNELS
#endif

/** fluid_oss_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    fluid_synth_t *synth;
    fluid_audio_callback_t read;
    void *buffer;
    fluid_thread_t *thread;
    int cont;
    int dspfd;
    int buffer_size;
    int buffer_byte_size;
    int bigendian;
    int formats;
    int format;
    int caps;
    fluid_audio_func_t callback;
    void *data;
    float *buffers[2];
} fluid_oss_audio_driver_t;


/* local utilities */
static int fluid_oss_set_queue_size(fluid_oss_audio_driver_t *dev, int ss, int ch, int qs, int bs);
static fluid_thread_return_t fluid_oss_audio_run(void *d);
static fluid_thread_return_t fluid_oss_audio_run2(void *d);


typedef struct
{
    fluid_midi_driver_t driver;
    int fd;
    fluid_thread_t *thread;
    int status;
    unsigned char buffer[BUFFER_LENGTH];
    fluid_midi_parser_t *parser;
} fluid_oss_midi_driver_t;

static fluid_thread_return_t fluid_oss_midi_run(void *d);


void
fluid_oss_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.oss.device", "/dev/dsp", 0);
}

/*
 * new_fluid_oss_audio_driver
 */
fluid_audio_driver_t *
new_fluid_oss_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_oss_audio_driver_t *dev = NULL;
    int channels, sr, sample_size = 0, oss_format;
    struct stat devstat;
    int queuesize;
    double sample_rate;
    int periods, period_size;
    int realtime_prio = 0;
    char *devname = NULL;
    int format;

    dev = FLUID_NEW(fluid_oss_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_oss_audio_driver_t));

    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);

    dev->dspfd = -1;
    dev->synth = synth;
    dev->callback = NULL;
    dev->data = NULL;
    dev->cont = 1;
    dev->buffer_size = (int) period_size;
    queuesize = (int)(periods * period_size);

    if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        sample_size = 16;
        oss_format = AFMT_S16_LE;
        dev->read = fluid_synth_write_s16;
        dev->buffer_byte_size = dev->buffer_size * 4;

    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        sample_size = 32;
        oss_format = -1;
        dev->read = fluid_synth_write_float;
        dev->buffer_byte_size = dev->buffer_size * 8;

    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unknown sample format");
        goto error_recovery;
    }

    dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);

    if(dev->buffer == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    if(fluid_settings_dupstr(settings, "audio.oss.device", &devname) != FLUID_OK || !devname)            /* ++ alloc device name */
    {
        devname = FLUID_STRDUP("/dev/dsp");

        if(devname == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
    }

    dev->dspfd = open(devname, O_WRONLY, 0);

    if(dev->dspfd == -1)
    {
        FLUID_LOG(FLUID_ERR, "Device <%s> could not be opened for writing: %s",
                  devname, g_strerror(errno));
        goto error_recovery;
    }

    if(fstat(dev->dspfd, &devstat) == -1)
    {
        FLUID_LOG(FLUID_ERR, "fstat failed on device <%s>: %s", devname, g_strerror(errno));
        goto error_recovery;
    }

    if((devstat.st_mode & S_IFCHR) != S_IFCHR)
    {
        FLUID_LOG(FLUID_ERR, "Device <%s> is not a device file", devname);
        goto error_recovery;
    }

    if(fluid_oss_set_queue_size(dev, sample_size, 2, queuesize, period_size) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set device buffer size");
        goto error_recovery;
    }

    format = oss_format;

    if(ioctl(dev->dspfd, SNDCTL_DSP_SETFMT, &oss_format) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample format");
        goto error_recovery;
    }

    if(oss_format != format)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample format");
        goto error_recovery;
    }

    channels = 2;

    if(ioctl(dev->dspfd, OSS_CHANNELS_PLACEHOLDER, &channels) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
        goto error_recovery;
    }

    if(channels != 2)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
        goto error_recovery;
    }

    sr = sample_rate;

    if(ioctl(dev->dspfd, SNDCTL_DSP_SPEED, &sr) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
        goto error_recovery;
    }

    if((sr < 0.95 * sample_rate) ||
            (sr > 1.05 * sample_rate))
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
        goto error_recovery;
    }

    /* Create the audio thread */
    dev->thread = new_fluid_thread("oss-audio", fluid_oss_audio_run, dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        goto error_recovery;
    }

    if(devname)
    {
        FLUID_FREE(devname);    /* -- free device name */
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:

    if(devname)
    {
        FLUID_FREE(devname);    /* -- free device name */
    }

    delete_fluid_oss_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

fluid_audio_driver_t *
new_fluid_oss_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    fluid_oss_audio_driver_t *dev = NULL;
    int channels, sr;
    struct stat devstat;
    int queuesize;
    double sample_rate;
    int periods, period_size;
    char *devname = NULL;
    int realtime_prio = 0;
    int format;

    dev = FLUID_NEW(fluid_oss_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_oss_audio_driver_t));

    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);

    dev->dspfd = -1;
    dev->synth = NULL;
    dev->read = NULL;
    dev->callback = func;
    dev->data = data;
    dev->cont = 1;
    dev->buffer_size = (int) period_size;
    queuesize = (int)(periods * period_size);
    dev->buffer_byte_size = dev->buffer_size * 2 * 2; /* 2 channels * 16 bits audio */


    if(fluid_settings_dupstr(settings, "audio.oss.device", &devname) != FLUID_OK || !devname)
    {
        devname = FLUID_STRDUP("/dev/dsp");

        if(!devname)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
    }

    dev->dspfd = open(devname, O_WRONLY, 0);

    if(dev->dspfd == -1)
    {
        FLUID_LOG(FLUID_ERR, "Device <%s> could not be opened for writing: %s",
                  devname, g_strerror(errno));
        goto error_recovery;
    }

    if(fstat(dev->dspfd, &devstat) == -1)
    {
        FLUID_LOG(FLUID_ERR, "fstat failed on device <%s>: %s", devname, g_strerror(errno));
        goto error_recovery;
    }

    if((devstat.st_mode & S_IFCHR) != S_IFCHR)
    {
        FLUID_LOG(FLUID_ERR, "Device <%s> is not a device file", devname);
        goto error_recovery;
    }

    if(fluid_oss_set_queue_size(dev, 16, 2, queuesize, period_size) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set device buffer size");
        goto error_recovery;
    }

    format = AFMT_S16_LE;

    if(ioctl(dev->dspfd, SNDCTL_DSP_SETFMT, &format) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample format");
        goto error_recovery;
    }

    if(format != AFMT_S16_LE)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample format");
        goto error_recovery;
    }

    channels = 2;

    if(ioctl(dev->dspfd, OSS_CHANNELS_PLACEHOLDER, &channels) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
        goto error_recovery;
    }

    if(channels != 2)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the number of channels");
        goto error_recovery;
    }

    sr = sample_rate;

    if(ioctl(dev->dspfd, SNDCTL_DSP_SPEED, &sr) < 0)
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
        goto error_recovery;
    }

    if((sr < 0.95 * sample_rate) ||
            (sr > 1.05 * sample_rate))
    {
        FLUID_LOG(FLUID_ERR, "Can't set the sample rate");
        goto error_recovery;
    }

    /* allocate the buffers. */
    dev->buffer = FLUID_MALLOC(dev->buffer_byte_size);
    dev->buffers[0] = FLUID_ARRAY(float, dev->buffer_size);
    dev->buffers[1] = FLUID_ARRAY(float, dev->buffer_size);

    if((dev->buffer == NULL) || (dev->buffers[0] == NULL) || (dev->buffers[1] == NULL))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    /* Create the audio thread */
    dev->thread = new_fluid_thread("oss-audio", fluid_oss_audio_run2, dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        goto error_recovery;
    }

    if(devname)
    {
        FLUID_FREE(devname);    /* -- free device name */
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:

    if(devname)
    {
        FLUID_FREE(devname);    /* -- free device name */
    }

    delete_fluid_oss_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_oss_audio_driver
 */
void
delete_fluid_oss_audio_driver(fluid_audio_driver_t *p)
{
    fluid_oss_audio_driver_t *dev = (fluid_oss_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    dev->cont = 0;

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->dspfd >= 0)
    {
        close(dev->dspfd);
    }

    FLUID_FREE(dev->buffer);
    FLUID_FREE(dev->buffers[0]);
    FLUID_FREE(dev->buffers[1]);
    FLUID_FREE(dev);
}

/**
 *  fluid_oss_set_queue_size
 *
 *  Set the internal buffersize of the output device.
 *
 *  @param ss Sample size in bits
 *  @param ch Number of channels
 *  @param qs The queue size in frames
 *  @param bs The synthesis buffer size in frames
 */
int
fluid_oss_set_queue_size(fluid_oss_audio_driver_t *dev, int ss, int ch, int qs, int bs)
{
    unsigned int fragmentSize;
    unsigned int fragSizePower;
    unsigned int fragments;
    unsigned int fragmentsPower;

    fragmentSize = (unsigned int)(bs * ch * ss / 8);

    fragSizePower = 0;

    while(0 < fragmentSize)
    {
        fragmentSize = (fragmentSize >> 1);
        fragSizePower++;
    }

    fragSizePower--;

    fragments = (unsigned int)(qs / bs);

    if(fragments < 2)
    {
        fragments = 2;
    }

    /* make sure fragments is a power of 2 */
    fragmentsPower = 0;

    while(0 < fragments)
    {
        fragments = (fragments >> 1);
        fragmentsPower++;
    }

    fragmentsPower--;

    fragments = (1 << fragmentsPower);
    fragments = (fragments << 16) + fragSizePower;

    return ioctl(dev->dspfd, SNDCTL_DSP_SETFRAGMENT, &fragments);
}

/*
 * fluid_oss_audio_run
 */
fluid_thread_return_t
fluid_oss_audio_run(void *d)
{
    fluid_oss_audio_driver_t *dev = (fluid_oss_audio_driver_t *) d;
    fluid_synth_t *synth = dev->synth;
    void *buffer = dev->buffer;
    int len = dev->buffer_size;

    /* it's as simple as that: */
    while(dev->cont)
    {
        dev->read(synth, len, buffer, 0, 2, buffer, 1, 2);

        if(write(dev->dspfd, buffer, dev->buffer_byte_size) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error writing to OSS sound device: %s",
                      g_strerror(errno));
            break;
        }
    }

    FLUID_LOG(FLUID_DBG, "Audio thread finished");

    return FLUID_THREAD_RETURN_VALUE;
}


/*
 * fluid_oss_audio_run
 */
fluid_thread_return_t
fluid_oss_audio_run2(void *d)
{
    fluid_oss_audio_driver_t *dev = (fluid_oss_audio_driver_t *) d;
    short *buffer = (short *) dev->buffer;
    float *left = dev->buffers[0];
    float *right = dev->buffers[1];
    int buffer_size = dev->buffer_size;
    int dither_index = 0;

    FLUID_LOG(FLUID_DBG, "Audio thread running");

    /* it's as simple as that: */
    while(dev->cont)
    {
        FLUID_MEMSET(left, 0, buffer_size * sizeof(float));
        FLUID_MEMSET(right, 0, buffer_size * sizeof(float));

        (*dev->callback)(dev->data, buffer_size, 0, NULL, 2, dev->buffers);

        fluid_synth_dither_s16(&dither_index, buffer_size, left, right,
                               buffer, 0, 2, buffer, 1, 2);

        if(write(dev->dspfd, buffer, dev->buffer_byte_size) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error writing to OSS sound device: %s",
                      g_strerror(errno));
            break;
        }
    }

    FLUID_LOG(FLUID_DBG, "Audio thread finished");

    return FLUID_THREAD_RETURN_VALUE;
}


void fluid_oss_midi_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "midi.oss.device", "/dev/midi", 0);
}

/*
 * new_fluid_oss_midi_driver
 */
fluid_midi_driver_t *
new_fluid_oss_midi_driver(fluid_settings_t *settings,
                          handle_midi_event_func_t handler, void *data)
{
    fluid_oss_midi_driver_t *dev;
    int realtime_prio = 0;
    char *device = NULL;

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    /* allocate the device */
    dev = FLUID_NEW(fluid_oss_midi_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_oss_midi_driver_t));
    dev->fd = -1;

    dev->driver.handler = handler;
    dev->driver.data = data;

    /* allocate one event to store the input data */
    dev->parser = new_fluid_midi_parser();

    if(dev->parser == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    /* get the device name. if none is specified, use the default device. */
    fluid_settings_dupstr(settings, "midi.oss.device", &device);  /* ++ alloc device name */

    if(device == NULL)
    {
        device = FLUID_STRDUP("/dev/midi");

        if(!device)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
    }

    fluid_settings_getint(settings, "midi.realtime-prio", &realtime_prio);

    /* open the default hardware device. only use midi in. */
    dev->fd = open(device, O_RDONLY, 0);

    if(dev->fd < 0)
    {
        perror(device);
        goto error_recovery;
    }

    if(fcntl(dev->fd, F_SETFL, O_NONBLOCK) == -1)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set OSS MIDI device to non-blocking: %s",
                  g_strerror(errno));
        goto error_recovery;
    }

    dev->status = FLUID_MIDI_READY;

    /* create MIDI thread */
    dev->thread = new_fluid_thread("oss-midi", fluid_oss_midi_run, dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        goto error_recovery;
    }

    if(device)
    {
        FLUID_FREE(device);    /* ++ free device */
    }

    return (fluid_midi_driver_t *) dev;

error_recovery:

    if(device)
    {
        FLUID_FREE(device);    /* ++ free device */
    }

    delete_fluid_oss_midi_driver((fluid_midi_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_oss_midi_driver
 */
void
delete_fluid_oss_midi_driver(fluid_midi_driver_t *p)
{
    fluid_oss_midi_driver_t *dev = (fluid_oss_midi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    /* cancel the thread and wait for it before cleaning up */
    dev->status = FLUID_MIDI_DONE;

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->fd >= 0)
    {
        close(dev->fd);
    }

    delete_fluid_midi_parser(dev->parser);
    FLUID_FREE(dev);
}

/*
 * fluid_oss_midi_run
 */
fluid_thread_return_t
fluid_oss_midi_run(void *d)
{
    fluid_oss_midi_driver_t *dev = (fluid_oss_midi_driver_t *) d;
    fluid_midi_event_t *evt;
    struct pollfd fds;
    int n, i;

    /* go into a loop until someone tells us to stop */
    dev->status = FLUID_MIDI_LISTENING;

    fds.fd = dev->fd;
    fds.events = POLLIN;
    fds.revents = 0;

    while(dev->status == FLUID_MIDI_LISTENING)
    {

        n = poll(&fds, 1, 100);

        if(n == 0)
        {
            continue;
        }

        if(n < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error waiting for MIDI input: %s", g_strerror(errno));
            break;
        }

        /* read new data */
        n = read(dev->fd, dev->buffer, BUFFER_LENGTH);

        if(n == -EAGAIN)
        {
            continue;
        }

        if(n < 0)
        {
            perror("read");
            FLUID_LOG(FLUID_ERR, "Failed to read the midi input");
            break;
        }

        /* let the parser convert the data into events */
        for(i = 0; i < n; i++)
        {
            evt = fluid_midi_parser_parse(dev->parser, dev->buffer[i]);

            if(evt != NULL)
            {
                /* send the event to the next link in the chain */
                (*dev->driver.handler)(dev->driver.data, evt);
            }
        }
    }

    return FLUID_THREAD_RETURN_VALUE;
}

#endif /*#if OSS_SUPPORT */
