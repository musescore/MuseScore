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


/* fluid_jack.c
 *
 * Driver for the JACK
 *
 * This code is derived from the simple_client example in the JACK
 * source distribution. Many thanks to Paul Davis.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#if JACK_SUPPORT

#include <jack/jack.h>
#include <jack/midiport.h>

#include "fluid_lash.h"


typedef struct _fluid_jack_audio_driver_t fluid_jack_audio_driver_t;
typedef struct _fluid_jack_midi_driver_t fluid_jack_midi_driver_t;


/* Clients are shared for drivers using the same server. */
typedef struct
{
    jack_client_t *client;
    char *server;                 /* Jack server name used */
    fluid_jack_audio_driver_t *audio_driver;
    fluid_jack_midi_driver_t *midi_driver;
} fluid_jack_client_t;

/* Jack audio driver instance */
struct _fluid_jack_audio_driver_t
{
    fluid_audio_driver_t driver;
    fluid_jack_client_t *client_ref;

    jack_port_t **output_ports;
    int num_output_ports;
    float **output_bufs;

    jack_port_t **fx_ports;
    int num_fx_ports;
    float **fx_bufs;

    fluid_audio_func_t callback;
    void *data;
};

/* Jack MIDI driver instance */
struct _fluid_jack_midi_driver_t
{
    fluid_midi_driver_t driver;
    fluid_jack_client_t *client_ref;
    int midi_port_count;
    jack_port_t **midi_port; // array of midi port handles
    fluid_midi_parser_t *parser;
    int autoconnect_inputs;
    fluid_atomic_int_t autoconnect_is_outdated;
};

static fluid_jack_client_t *new_fluid_jack_client(fluid_settings_t *settings,
        int isaudio, void *driver);
static int fluid_jack_client_register_ports(void *driver, int isaudio,
        jack_client_t *client,
        fluid_settings_t *settings);

void fluid_jack_driver_shutdown(void *arg);
int fluid_jack_driver_srate(jack_nframes_t nframes, void *arg);
int fluid_jack_driver_bufsize(jack_nframes_t nframes, void *arg);
int fluid_jack_driver_process(jack_nframes_t nframes, void *arg);
void fluid_jack_port_registration(jack_port_id_t port, int is_registering, void *arg);

static fluid_mutex_t last_client_mutex = FLUID_MUTEX_INIT;     /* Probably not necessary, but just in case drivers are created by multiple threads */
static fluid_jack_client_t *last_client = NULL;       /* Last unpaired client. For audio/MIDI driver pairing. */


void
fluid_jack_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.jack.id", "fluidsynth", 0);
    fluid_settings_register_int(settings, "audio.jack.multi", 0, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_int(settings, "audio.jack.autoconnect", 0, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_str(settings, "audio.jack.server", "", 0);
}

/*
 * Connect all midi input ports to all terminal midi output ports
 */
void
fluid_jack_midi_autoconnect(jack_client_t *client, fluid_jack_midi_driver_t *midi_driver)
{
    int i, j;
    const char **midi_source_ports;

    midi_source_ports = jack_get_ports(client, NULL, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput | JackPortIsTerminal);

    if(midi_source_ports != NULL)
    {
        for(j = 0; midi_source_ports[j] != NULL; j++)
        {
            for(i = 0; i < midi_driver->midi_port_count; i++)
            {
                FLUID_LOG(FLUID_INFO, "jack midi autoconnect \"%s\" to \"%s\"", midi_source_ports[j], jack_port_name(midi_driver->midi_port[i]));
                jack_connect(client, midi_source_ports[j], jack_port_name(midi_driver->midi_port[i]));
            }
        }

        jack_free(midi_source_ports);
    }

    fluid_atomic_int_set(&midi_driver->autoconnect_is_outdated, FALSE);
}

/*
 * Create Jack client as necessary, share clients of the same server.
 * @param settings Settings object
 * @param isaudio TRUE if audio driver, FALSE if MIDI
 * @param driver fluid_jack_audio_driver_t or fluid_jack_midi_driver_t
 * @param data The user data instance associated with the driver (fluid_synth_t for example)
 * @return New or paired Audio/MIDI Jack client
 */
static fluid_jack_client_t *
new_fluid_jack_client(fluid_settings_t *settings, int isaudio, void *driver)
{
    fluid_jack_client_t *client_ref = NULL;
    char *server = NULL;
    char *client_name;
    char name[64];

    if(fluid_settings_dupstr(settings, isaudio ? "audio.jack.server"          /* ++ alloc server name */
                             : "midi.jack.server", &server) != FLUID_OK)
    {
        return NULL;
    }

    fluid_mutex_lock(last_client_mutex);      /* ++ lock last_client */

    /* If the last client uses the same server and is not the same type (audio or MIDI),
     * then re-use the client. */
    if(last_client &&
            (last_client->server != NULL && server != NULL && FLUID_STRCMP(last_client->server, server) == 0) &&
            ((!isaudio && last_client->midi_driver == NULL) || (isaudio && last_client->audio_driver == NULL)))
    {
        client_ref = last_client;

        /* Register ports */
        if(fluid_jack_client_register_ports(driver, isaudio, client_ref->client, settings) == FLUID_OK)
        {
            last_client = NULL; /* No more pairing for this client */

            if(isaudio)
            {
                fluid_atomic_pointer_set(&client_ref->audio_driver, driver);
            }
            else
            {
                fluid_atomic_pointer_set(&client_ref->midi_driver, driver);
            }
        }
        else
        {
            // do not free client_ref and do not goto error_recovery
            // client_ref is being used by another audio or midi driver. Freeing it here will lead to a double free.
            client_ref = NULL;
        }

        fluid_mutex_unlock(last_client_mutex);        /* -- unlock last_client */

        if(server)
        {
            FLUID_FREE(server);
        }

        return client_ref;
    }

    /* No existing client for this Jack server */
    client_ref = FLUID_NEW(fluid_jack_client_t);

    if(!client_ref)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        goto error_recovery;
    }

    FLUID_MEMSET(client_ref, 0, sizeof(fluid_jack_client_t));

    fluid_settings_dupstr(settings, isaudio ? "audio.jack.id"    /* ++ alloc client name */
                          : "midi.jack.id", &client_name);

    if(client_name != NULL && client_name[0] != 0)
    {
        FLUID_SNPRINTF(name, sizeof(name), "%s", client_name);
    }
    else
    {
        FLUID_STRNCPY(name, "fluidsynth", sizeof(name));
    }

    name[63] = '\0';

    if(client_name)
    {
        FLUID_FREE(client_name);    /* -- free client name */
    }

    /* Open a connection to the Jack server and use the server name if specified */
    if(server && server[0] != '\0')
    {
        client_ref->client = jack_client_open(name, JackServerName, NULL, server);
    }
    else
    {
        client_ref->client = jack_client_open(name, JackNullOption, NULL);
    }

    if(!client_ref->client)
    {
        FLUID_LOG(FLUID_ERR, "Failed to connect to Jack server.");
        goto error_recovery;
    }

    jack_set_port_registration_callback(client_ref->client, fluid_jack_port_registration, client_ref);
    jack_set_process_callback(client_ref->client, fluid_jack_driver_process, client_ref);
    jack_set_buffer_size_callback(client_ref->client, fluid_jack_driver_bufsize, client_ref);
    jack_set_sample_rate_callback(client_ref->client, fluid_jack_driver_srate, client_ref);
    jack_on_shutdown(client_ref->client, fluid_jack_driver_shutdown, client_ref);

    /* Register ports */
    if(fluid_jack_client_register_ports(driver, isaudio, client_ref->client, settings) != FLUID_OK)
    {
        goto error_recovery;
    }

    /* tell the JACK server that we are ready to roll */
    if(jack_activate(client_ref->client))
    {
        FLUID_LOG(FLUID_ERR, "Failed to activate Jack client");
        goto error_recovery;
    }

    /* tell the lash server our client name */
#ifdef HAVE_LASH
    {
        int enable_lash = 0;
        fluid_settings_getint(settings, "lash.enable", &enable_lash);

        if(enable_lash)
        {
            fluid_lash_jack_client_name(fluid_lash_client, name);
        }
    }
#endif /* HAVE_LASH */

    client_ref->server = server;        /* !! takes over allocation */
    server = NULL;      /* Set to NULL so it doesn't get freed below */

    last_client = client_ref;

    if(isaudio)
    {
        fluid_atomic_pointer_set(&client_ref->audio_driver, driver);
    }
    else
    {
        fluid_atomic_pointer_set(&client_ref->midi_driver, driver);
    }

    fluid_mutex_unlock(last_client_mutex);        /* -- unlock last_client */

    if(server)
    {
        FLUID_FREE(server);
    }

    return client_ref;

error_recovery:

    fluid_mutex_unlock(last_client_mutex);        /* -- unlock clients list */

    if(server)
    {
        FLUID_FREE(server);    /* -- free server name */
    }

    if(client_ref)
    {
        if(client_ref->client)
        {
            jack_client_close(client_ref->client);
        }

        FLUID_FREE(client_ref);
    }

    return NULL;
}

static int
fluid_jack_client_register_ports(void *driver, int isaudio, jack_client_t *client,
                                 fluid_settings_t *settings)
{
    fluid_jack_audio_driver_t *dev;
    char name[64];
    int multi;
    int i;
    unsigned long jack_srate;
    double sample_rate;

    if(!isaudio)
    {
        fluid_jack_midi_driver_t *dev = driver;
        int midi_channels, ports;

        fluid_settings_getint(settings, "synth.midi-channels", &midi_channels);
        ports = midi_channels / 16;

        if((dev->midi_port = FLUID_ARRAY(jack_port_t *, ports)) == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            return FLUID_FAILED;
        }

        for(i = 0; i < ports; i++)
        {
            FLUID_SNPRINTF(name, sizeof(name), "midi_%02d", i);
            dev->midi_port[i] = jack_port_register(client, name, JACK_DEFAULT_MIDI_TYPE,
                                                   JackPortIsInput | JackPortIsTerminal, 0);

            if(dev->midi_port[i] == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Failed to create Jack MIDI port '%s'", name);
                FLUID_FREE(dev->midi_port);
                dev->midi_port = NULL;
                return FLUID_FAILED;
            }
        }

        dev->midi_port_count = ports;
        return FLUID_OK;
    }

    dev = driver;

    fluid_settings_getint(settings, "audio.jack.multi", &multi);

    if(!multi)
    {
        /* create the two audio output ports */
        dev->num_output_ports = 1;
        dev->num_fx_ports = 0;

        dev->output_ports = FLUID_ARRAY(jack_port_t *, 2 * dev->num_output_ports);

        if(dev->output_ports == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            return FLUID_FAILED;
        }

        dev->output_bufs = FLUID_ARRAY(float *, 2 * dev->num_output_ports);
        FLUID_MEMSET(dev->output_ports, 0, 2 * dev->num_output_ports * sizeof(jack_port_t *));

        dev->output_ports[0]
            = jack_port_register(client, "left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        dev->output_ports[1]
            = jack_port_register(client, "right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        if(dev->output_ports[0] == NULL || dev->output_ports[1] == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Failed to create Jack audio port '%s'",
                      (dev->output_ports[0] == NULL ? (dev->output_ports[1] == NULL ? "left & right" : "left") : "right"));
            goto error_recovery;
        }
    }
    else
    {
        fluid_settings_getint(settings, "synth.audio-channels", &dev->num_output_ports);

        dev->output_ports = FLUID_ARRAY(jack_port_t *, 2 * dev->num_output_ports);

        if(dev->output_ports == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            return FLUID_FAILED;
        }

        dev->output_bufs = FLUID_ARRAY(float *, 2 * dev->num_output_ports);

        if(dev->output_bufs == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            goto error_recovery;
        }

        FLUID_MEMSET(dev->output_ports, 0, 2 * dev->num_output_ports * sizeof(jack_port_t *));

        for(i = 0; i < dev->num_output_ports; i++)
        {
            sprintf(name, "l_%02d", i);

            if((dev->output_ports[2 * i]
                    = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Failed to create Jack audio port '%s'", name);
                goto error_recovery;
            }

            sprintf(name, "r_%02d", i);

            if((dev->output_ports[2 * i + 1]
                    = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Failed to create Jack audio port '%s'", name);
                goto error_recovery;
            }
        }

        fluid_settings_getint(settings, "synth.effects-channels", &dev->num_fx_ports);
        fluid_settings_getint(settings, "synth.effects-groups", &i);

        dev->num_fx_ports *= i;
        dev->fx_ports = FLUID_ARRAY(jack_port_t *, 2 * dev->num_fx_ports);

        if(dev->fx_ports == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            goto error_recovery;
        }

        dev->fx_bufs = FLUID_ARRAY(float *, 2 * dev->num_fx_ports);

        if(dev->fx_bufs == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            goto error_recovery;
        }

        FLUID_MEMSET(dev->fx_ports, 0, 2 * dev->num_fx_ports * sizeof(jack_port_t *));

        for(i = 0; i < dev->num_fx_ports; i++)
        {
            sprintf(name, "fx_l_%02d", i);

            if((dev->fx_ports[2 * i]
                    = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Failed to create Jack fx audio port '%s'", name);
                goto error_recovery;
            }

            sprintf(name, "fx_r_%02d", i);

            if((dev->fx_ports[2 * i + 1]
                    = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Failed to create Jack fx audio port '%s'", name);
                goto error_recovery;
            }
        }
    }

    /* Adjust sample rate to match JACK's */
    jack_srate = jack_get_sample_rate(client);
    FLUID_LOG(FLUID_DBG, "Jack engine sample rate: %lu", jack_srate);

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

    if((unsigned long)sample_rate != jack_srate)
    {
        fluid_synth_t* synth;
        if(fluid_jack_obtain_synth(settings, &synth) == FLUID_OK)
        {
            FLUID_LOG(FLUID_INFO, "Jack sample rate mismatch, adjusting."
                  " (synth.sample-rate=%lu, jackd=%lu)", (unsigned long)sample_rate, jack_srate);
            fluid_synth_set_sample_rate_immediately(synth, jack_srate);
        }
        else
        {
            FLUID_LOG(FLUID_WARN, "Jack sample rate mismatch (synth.sample-rate=%lu, jackd=%lu)"
            " impossible to adjust, because the settings object provided to new_fluid_audio_driver2() was not used to create a synth."
            , (unsigned long)sample_rate, jack_srate);
        }
    }

    return FLUID_OK;

error_recovery:

    FLUID_FREE(dev->output_ports);
    dev->output_ports = NULL;
    FLUID_FREE(dev->fx_ports);
    dev->fx_ports = NULL;
    FLUID_FREE(dev->output_bufs);
    dev->output_bufs = NULL;
    FLUID_FREE(dev->fx_bufs);
    dev->fx_bufs = NULL;
    return FLUID_FAILED;
}

static void
fluid_jack_client_close(fluid_jack_client_t *client_ref, void *driver)
{
    if(client_ref->audio_driver == driver)
    {
        fluid_atomic_pointer_set(&client_ref->audio_driver, NULL);
    }
    else if(client_ref->midi_driver == driver)
    {
        fluid_atomic_pointer_set(&client_ref->midi_driver, NULL);
    }

    if(client_ref->audio_driver || client_ref->midi_driver)
    {
        fluid_msleep(100);  /* FIXME - Hack to make sure that resources don't get freed while Jack callback is active */
        return;
    }

    fluid_mutex_lock(last_client_mutex);

    if(client_ref == last_client)
    {
        last_client = NULL;
    }

    fluid_mutex_unlock(last_client_mutex);

    if(client_ref->client)
    {
        jack_client_close(client_ref->client);
    }

    if(client_ref->server)
    {
        FLUID_FREE(client_ref->server);
    }

    FLUID_FREE(client_ref);
}


fluid_audio_driver_t *
new_fluid_jack_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    return new_fluid_jack_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_jack_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    fluid_jack_audio_driver_t *dev = NULL;
    jack_client_t *client;
    const char **jack_ports;      /* for looking up ports */
    int autoconnect = 0;
    int i;

    dev = FLUID_NEW(fluid_jack_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_jack_audio_driver_t));

    dev->callback = func;
    dev->data = data;

    dev->client_ref = new_fluid_jack_client(settings, TRUE, dev);

    if(!dev->client_ref)
    {
        FLUID_FREE(dev);
        return NULL;
    }

    client = dev->client_ref->client;

    /* connect the ports. */

    /* find some physical ports and connect to them */
    fluid_settings_getint(settings, "audio.jack.autoconnect", &autoconnect);

    if(autoconnect)
    {
        jack_ports = jack_get_ports(client, NULL, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput | JackPortIsPhysical);

        if(jack_ports && jack_ports[0])
        {
            int err, o = 0;
            int connected = 0;

            for(i = 0; i < 2 * dev->num_output_ports; ++i)
            {
                err = jack_connect(client, jack_port_name(dev->output_ports[i]), jack_ports[o++]);

                if(err)
                {
                    FLUID_LOG(FLUID_ERR, "Error connecting jack port");
                }
                else
                {
                    connected++;
                }

                if(!jack_ports[o])
                {
                    o = 0;
                }
            }

            o = 0;
            for(i = 0; i < 2 * dev->num_fx_ports; ++i)
            {
                err = jack_connect(client, jack_port_name(dev->fx_ports[i]), jack_ports[o++]);

                if(err)
                {
                    FLUID_LOG(FLUID_ERR, "Error connecting jack port");
                }
                else
                {
                    connected++;
                }

                if(!jack_ports[o])
                {
                    o = 0;
                }
            }

            jack_free(jack_ports);   /* free jack ports array (not the port values!) */
        }
        else
        {
            FLUID_LOG(FLUID_WARN, "Could not connect to any physical jack ports; fluidsynth is unconnected");
        }
    }

    return (fluid_audio_driver_t *) dev;
}

/*
 * delete_fluid_jack_audio_driver
 */
void
delete_fluid_jack_audio_driver(fluid_audio_driver_t *p)
{
    fluid_jack_audio_driver_t *dev = (fluid_jack_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    if(dev->client_ref != NULL)
    {
        fluid_jack_client_close(dev->client_ref, dev);
    }

    FLUID_FREE(dev->output_bufs);
    FLUID_FREE(dev->output_ports);
    FLUID_FREE(dev->fx_bufs);
    FLUID_FREE(dev->fx_ports);
    FLUID_FREE(dev);
}

/* Process function for audio and MIDI Jack drivers */
int
fluid_jack_driver_process(jack_nframes_t nframes, void *arg)
{
    fluid_jack_client_t *client = (fluid_jack_client_t *)arg;
    fluid_jack_audio_driver_t *audio_driver;
    fluid_jack_midi_driver_t *midi_driver;
    float *left, *right;
    int i;

    jack_midi_event_t midi_event;
    fluid_midi_event_t *evt;
    void *midi_buffer;
    jack_nframes_t event_count;
    jack_nframes_t event_index;
    unsigned int u;

    /* Process MIDI events first, so that they take effect before audio synthesis */
    midi_driver = fluid_atomic_pointer_get(&client->midi_driver);

    if(midi_driver)
    {
        if(fluid_atomic_int_get(&midi_driver->autoconnect_is_outdated))
        {
            fluid_jack_midi_autoconnect(client->client, midi_driver);
        }

        for(i = 0; i < midi_driver->midi_port_count; i++)
        {
            midi_buffer = jack_port_get_buffer(midi_driver->midi_port[i], 0);
            event_count = jack_midi_get_event_count(midi_buffer);

            for(event_index = 0; event_index < event_count; event_index++)
            {
                jack_midi_event_get(&midi_event, midi_buffer, event_index);

                /* let the parser convert the data into events */
                for(u = 0; u < midi_event.size; u++)
                {
                    evt = fluid_midi_parser_parse(midi_driver->parser, midi_event.buffer[u]);

                    /* send the event to the next link in the chain */
                    if(evt != NULL)
                    {
                        fluid_midi_event_set_channel(evt, fluid_midi_event_get_channel(evt) + i * 16);
                        midi_driver->driver.handler(midi_driver->driver.data, evt);
                    }
                }
            }
        }
    }

    audio_driver = fluid_atomic_pointer_get(&client->audio_driver);

    if(audio_driver == NULL)
    {
        // shutting down
        return FLUID_OK;
    }

    if(audio_driver->callback == NULL && audio_driver->num_output_ports == 1 && audio_driver->num_fx_ports == 0)  /* i.e. audio.jack.multi=no */
    {
        left = (float *) jack_port_get_buffer(audio_driver->output_ports[0], nframes);
        right = (float *) jack_port_get_buffer(audio_driver->output_ports[1], nframes);

        return fluid_synth_write_float(audio_driver->data, nframes, left, 0, 1, right, 0, 1);
    }
    else
    {
        int res;
        fluid_audio_func_t callback = (audio_driver->callback != NULL) ? audio_driver->callback : (fluid_audio_func_t) fluid_synth_process;

        for(i = 0; i < audio_driver->num_output_ports; i++)
        {
            int k = i * 2;

            audio_driver->output_bufs[k] = (float *)jack_port_get_buffer(audio_driver->output_ports[k], nframes);
            FLUID_MEMSET(audio_driver->output_bufs[k], 0, nframes * sizeof(float));

            k = 2 * i + 1;
            audio_driver->output_bufs[k] = (float *)jack_port_get_buffer(audio_driver->output_ports[k], nframes);
            FLUID_MEMSET(audio_driver->output_bufs[k], 0, nframes * sizeof(float));
        }

        for(i = 0; i < audio_driver->num_fx_ports; i++)
        {
            int k = i * 2;

            audio_driver->fx_bufs[k] = (float *) jack_port_get_buffer(audio_driver->fx_ports[k], nframes);
            FLUID_MEMSET(audio_driver->fx_bufs[k], 0, nframes * sizeof(float));

            k = 2 * i + 1;
            audio_driver->fx_bufs[k] = (float *) jack_port_get_buffer(audio_driver->fx_ports[k], nframes);
            FLUID_MEMSET(audio_driver->fx_bufs[k], 0, nframes * sizeof(float));
        }

        res = callback(audio_driver->data,
                        nframes,
                        audio_driver->num_fx_ports * 2,
                        audio_driver->fx_bufs,
                        audio_driver->num_output_ports * 2,
                        audio_driver->output_bufs);
        if(res != FLUID_OK)
        {
            const char *cb_func_name = (audio_driver->callback != NULL) ? "Custom audio callback function" : "fluid_synth_process()";
            FLUID_LOG(FLUID_PANIC, "%s returned an error. As a consequence, fluidsynth will now be removed from Jack's processing loop.", cb_func_name);
        }
        return res;
    }
}

int
fluid_jack_driver_bufsize(jack_nframes_t nframes, void *arg)
{
    /*   printf("the maximum buffer size is now %lu\n", nframes); */
    return 0;
}

int
fluid_jack_driver_srate(jack_nframes_t nframes, void *arg)
{
    /*   printf("the sample rate is now %lu/sec\n", nframes); */
    /* FIXME: change the sample rate of the synthesizer! */
    return 0;
}

void
fluid_jack_driver_shutdown(void *arg)
{
//  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) arg;
    FLUID_LOG(FLUID_ERR, "Help! Lost the connection to the JACK server");
    /*   exit (1); */
}

void
fluid_jack_port_registration(jack_port_id_t port, int is_registering, void *arg)
{
    fluid_jack_client_t *client_ref = (fluid_jack_client_t *)arg;

    if(client_ref->midi_driver != NULL)
    {
        fluid_atomic_int_set(&client_ref->midi_driver->autoconnect_is_outdated, client_ref->midi_driver->autoconnect_inputs && is_registering != 0);
    }
}

void fluid_jack_midi_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "midi.jack.id", "fluidsynth-midi", 0);
    fluid_settings_register_str(settings, "midi.jack.server", "", 0);
}

/*
 * new_fluid_jack_midi_driver
 */
fluid_midi_driver_t *
new_fluid_jack_midi_driver(fluid_settings_t *settings,
                           handle_midi_event_func_t handler, void *data)
{
    fluid_jack_midi_driver_t *dev;

    fluid_return_val_if_fail(handler != NULL, NULL);

    /* allocate the device */
    dev = FLUID_NEW(fluid_jack_midi_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_jack_midi_driver_t));

    dev->driver.handler = handler;
    dev->driver.data = data;

    /* allocate one event to store the input data */
    dev->parser = new_fluid_midi_parser();

    if(dev->parser == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        goto error_recovery;
    }

    fluid_settings_getint(settings, "midi.autoconnect", &dev->autoconnect_inputs);
    fluid_atomic_int_set(&dev->autoconnect_is_outdated, dev->autoconnect_inputs);

    dev->client_ref = new_fluid_jack_client(settings, FALSE, dev);

    if(!dev->client_ref)
    {
        goto error_recovery;
    }

    return (fluid_midi_driver_t *)dev;

error_recovery:
    delete_fluid_jack_midi_driver((fluid_midi_driver_t *)dev);
    return NULL;
}

void
delete_fluid_jack_midi_driver(fluid_midi_driver_t *p)
{
    fluid_jack_midi_driver_t *dev = (fluid_jack_midi_driver_t *)p;
    fluid_return_if_fail(dev != NULL);

    if(dev->client_ref != NULL)
    {
        fluid_jack_client_close(dev->client_ref, dev);
    }

    delete_fluid_midi_parser(dev->parser);
    FLUID_FREE(dev->midi_port);
    FLUID_FREE(dev);
}

int fluid_jack_obtain_synth(fluid_settings_t *settings, fluid_synth_t **synth)
{
    void *data;

    if(!fluid_settings_is_realtime(settings, "synth.gain") ||
       (data = fluid_settings_get_user_data(settings, "synth.gain")) == NULL)
    {
        return FLUID_FAILED;
    }

    *synth = data;
    return FLUID_OK;
}

#endif /* JACK_SUPPORT */
