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

/* fluid_coremidi.c
 *
 * Driver for Mac OSX native MIDI
 * Pedro Lopez-Cabanillas, Jan 2009
 */

#include "fluidsynth_priv.h"

#if COREMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

/* Work around for OSX 10.4 */

/* enum definition in OpenTransportProviders.h defines these tokens
   which are #defined from <netinet/tcp.h> */
#ifdef TCP_NODELAY
#undef TCP_NODELAY
#endif
#ifdef TCP_MAXSEG
#undef TCP_MAXSEG
#endif
#ifdef TCP_KEEPALIVE
#undef TCP_KEEPALIVE
#endif

/* End work around */

#include <unistd.h>
#include <os/log.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/MIDIServices.h>

typedef struct
{
    fluid_midi_driver_t driver;
    MIDIClientRef client;
    MIDIEndpointRef endpoint;
    MIDIPortRef input_port;
    fluid_midi_parser_t *parser;
    int autoconn_inputs;
} fluid_coremidi_driver_t;

static const MIDIClientRef invalid_client = (MIDIClientRef)-1;
static const MIDIEndpointRef invalid_endpoint = (MIDIEndpointRef)-1;
static const MIDIPortRef invalid_port = (MIDIPortRef)-1;

void fluid_coremidi_callback(const MIDIPacketList *list, void *p, void *src);

void fluid_coremidi_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "midi.coremidi.id", "pid", 0);
}

static void fluid_coremidi_autoconnect(fluid_coremidi_driver_t *dev, MIDIPortRef input_port)
{
    int i;
    int source_count = MIDIGetNumberOfSources();
    for(i = 0; i < source_count; ++i)
    {
        MIDIEndpointRef source = MIDIGetSource(i);

        CFStringRef externalName;
        OSStatus result = MIDIObjectGetStringProperty(source, kMIDIPropertyName, &externalName);
        const char *source_name = CFStringGetCStringPtr(externalName, kCFStringEncodingASCII);
        CFRelease(externalName);

        result = MIDIPortConnectSource(input_port, source, NULL);
        if(result != noErr)
        {
            FLUID_LOG(FLUID_ERR, "Failed to connect \"%s\" device to input port.", source_name);
        }
        else
        {
            FLUID_LOG(FLUID_DBG, "Connected input port to \"%s\".", source_name);
        }
    }
}

/*
 * new_fluid_coremidi_driver
 */
fluid_midi_driver_t *
new_fluid_coremidi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *data)
{
    fluid_coremidi_driver_t *dev;
    MIDIClientRef client;
    MIDIEndpointRef endpoint;
    char clientid[32];
    char *portname;
    char *id;
    CFStringRef str_portname;
    CFStringRef str_clientname;
    OSStatus result;
    CFStringRef str_input_portname;

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    dev = FLUID_MALLOC(sizeof(fluid_coremidi_driver_t));

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    dev->client = invalid_client;
    dev->endpoint = invalid_endpoint;
    dev->input_port = invalid_port;
    dev->parser = NULL;
    dev->driver.handler = handler;
    dev->driver.data = data;

    dev->parser = new_fluid_midi_parser();

    if(dev->parser == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    fluid_settings_dupstr(settings, "midi.coremidi.id", &id);     /* ++ alloc id string */
    memset(clientid, 0, sizeof(clientid));

    if(id != NULL)
    {
        if(FLUID_STRCMP(id, "pid") == 0)
        {
            FLUID_SNPRINTF(clientid, sizeof(clientid), " (%d)", getpid());
        }
        else
        {
            FLUID_SNPRINTF(clientid, sizeof(clientid), " (%s)", id);
        }

        FLUID_FREE(id);   /* -- free id string */
    }

    str_clientname = CFStringCreateWithFormat(NULL, NULL,
                     CFSTR("FluidSynth%s"), clientid);

    fluid_settings_dupstr(settings, "midi.portname", &portname);  /* ++ alloc port name */

    if(!portname || strlen(portname) == 0)
        str_portname = CFStringCreateWithFormat(NULL, NULL,
                                                CFSTR("FluidSynth virtual port%s"),
                                                clientid);
    else
        str_portname = CFStringCreateWithCString(NULL, portname,
                       kCFStringEncodingASCII);

    if(portname)
    {
        FLUID_FREE(portname);    /* -- free port name */
    }

    result = MIDIClientCreate(str_clientname, NULL, NULL, &client);
    CFRelease(str_clientname);

    if(result != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the MIDI input client");
        goto error_recovery;
    }

    dev->client = client;

    result = MIDIDestinationCreate(client, str_portname,
                                   fluid_coremidi_callback,
                                   (void *)dev, &endpoint);

    if(result != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the MIDI input port. MIDI input not available.");
        goto error_recovery;
    }

    str_input_portname = CFSTR("input");
    result = MIDIInputPortCreate(client, str_input_portname,
                                 fluid_coremidi_callback,
                                 (void *)dev, &dev->input_port);
    CFRelease(str_input_portname);

    if(result != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create input port.");
        goto error_recovery;
    }

    fluid_settings_getint(settings, "midi.autoconnect", &dev->autoconn_inputs);

    if(dev->autoconn_inputs)
    {
        fluid_coremidi_autoconnect(dev, dev->input_port);
    }

    dev->endpoint = endpoint;

    return (fluid_midi_driver_t *) dev;

error_recovery:
    delete_fluid_coremidi_driver((fluid_midi_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_coremidi_driver
 */
void
delete_fluid_coremidi_driver(fluid_midi_driver_t *p)
{
    fluid_coremidi_driver_t *dev = (fluid_coremidi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    if(dev->input_port != invalid_port)
    {
        MIDIPortDispose(dev->input_port);
    }

    if(dev->client != invalid_client)
    {
        MIDIClientDispose(dev->client);
    }

    if(dev->endpoint != invalid_endpoint)
    {
        MIDIEndpointDispose(dev->endpoint);
    }

    if(dev->parser != NULL)
    {
        delete_fluid_midi_parser(dev->parser);
    }

    FLUID_FREE(dev);
}

void
fluid_coremidi_callback(const MIDIPacketList *list, void *p, void *src)
{
    unsigned int i, j;
    fluid_midi_event_t *event;
    fluid_coremidi_driver_t *dev = (fluid_coremidi_driver_t *)p;
    const MIDIPacket *packet = &list->packet[0];

    for(i = 0; i < list->numPackets; ++i)
    {
        for(j = 0; j < packet->length; ++j)
        {
            event = fluid_midi_parser_parse(dev->parser, packet->data[j]);

            if(event != NULL)
            {
                (*dev->driver.handler)(dev->driver.data, event);
            }
        }

        packet = MIDIPacketNext(packet);
    }
}

#endif /* COREMIDI_SUPPORT */
