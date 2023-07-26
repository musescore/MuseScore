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


/* fluid_winmidi.c
 *
 * Driver for Windows MIDI
 *
 * NOTE: Unfortunately midiInAddBuffer(), for SYSEX data, should not be called
 * from within the MIDI input callback, despite many examples contrary to that
 * on the Internet.  Some MIDI devices will deadlock.  Therefore we add MIDIHDR
 * pointers to a queue and re-add them in a separate thread.  Lame-o API! :(
 *
 * Multiple/single devices handling capabilities:
 * This driver is able to handle multiple devices chosen by the user through
 * the settings midi.winmidi.device.
 * For example, let the following device names:
 * 0:Port MIDI SB Live! [CE00], 1:SB PCI External MIDI, default, x[;y;z;..]
 * Then the driver is able receive MIDI messages coming from distinct devices
 * and forward these messages on distinct MIDI channels set.
 * 1.1)For example, if the user chooses 2 devices at index 0 and 1, the user
 * must specify this by putting the name "0;1" in midi.winmidi.device setting.
 * We get a fictive device composed of real devices (0,1). This fictive device
 * behaves like a device with 32 MIDI channels whose messages are forwarded to
 * driver output as this:
 * - MIDI messages from real device 0 are output to MIDI channels set 0 to 15.
 * - MIDI messages from real device 1 are output to MIDI channels set 15 to 31.
 * The above example assumes the setting synth.midi-channels value 32, but the
 * default value for this setting is 16, in which case there will be no mapping.
 *
 * 1.2)Now another example with the name "1;0". The driver will forward
 * MIDI messages as this:
 * - MIDI messages from real device 1 are output to MIDI channels set 0 to 15.
 * - MIDI messages from real device 0 are output to MIDI channels set 15 to 31.
 * So, the device order specified in the setting allows the user to choose the
 * MIDI channel set associated with this real device at the driver output
 * according this formula: output_channel = input_channel + device_order * 16.
 *
 * 2)Note also that the driver handles single device by putting the device name
 * in midi.winmidi.device setting.
 * The user can set the device name "0:Port MIDI SB Live! [CE00]" in the setting.
 * or use the multi device naming "0" (specifying only device index 0).
 * Both naming choice allows the driver to handle the same single device.
 *
 * 3)If the device name is "default" and the setting "midi.autoconnect" is enabled,
 * then all the available devices are opened, applying the appropriate channel
 * mappings to each device (the first device is mapped to the 16 first channels,
 * the second one to the next 16 channels, and so on with the limit of the
 * synth.midi-channels setting. After arriving to the channels limit, the mapping
 * restars with the channels 1-16.
 * If the device name is specified, then midi.autoconnect setting is ignored.
 */

#include "fluidsynth_priv.h"

#if WINMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#define MIDI_SYSEX_MAX_SIZE     512
#define MIDI_SYSEX_BUF_COUNT    16

typedef struct fluid_winmidi_driver fluid_winmidi_driver_t;

/* device infos structure for only one midi device */
typedef struct device_infos
{
    fluid_winmidi_driver_t *dev; /* driver structure*/
    unsigned char midi_num;      /* device order number */
    unsigned char channel_map;   /* MIDI channel mapping from input to output */
    UINT dev_idx;                /* device index */
    HMIDIIN hmidiin;             /* device handle */
    /* MIDI HDR for SYSEX buffer */
    MIDIHDR sysExHdrs[MIDI_SYSEX_BUF_COUNT];
    /* Sysex data buffer */
    unsigned char sysExBuf[MIDI_SYSEX_BUF_COUNT * MIDI_SYSEX_MAX_SIZE];
} device_infos_t;

/* driver structure */
struct fluid_winmidi_driver
{
    fluid_midi_driver_t driver;

    /* Thread for SYSEX re-add thread */
    HANDLE hThread;
    DWORD  dwThread;

    /* devices information table */
    int dev_count;   /* device information count in dev_infos[] table */
    device_infos_t dev_infos[1];
};

#define msg_type(_m)  ((unsigned char)(_m & 0xf0))
#define msg_chan(_m)  ((unsigned char)(_m & 0x0f))
#define msg_p1(_m)    ((_m >> 8) & 0x7f)
#define msg_p2(_m)    ((_m >> 16) & 0x7f)

static char *
fluid_winmidi_input_error(char *strError, MMRESULT no)
{
#ifdef _UNICODE
    WCHAR wStr[MAXERRORLENGTH];

    midiInGetErrorText(no, wStr, MAXERRORLENGTH);
    WideCharToMultiByte(CP_UTF8, 0, wStr, -1, strError, MAXERRORLENGTH, 0, 0);
#else
    midiInGetErrorText(no, strError, MAXERRORLENGTH);
#endif

    return strError;
}

/*
  callback function called by any MIDI device sending a MIDI message.
  @param dwInstance, pointer on device_infos structure of this
  device.
*/
static void CALLBACK
fluid_winmidi_callback(HMIDIIN hmi, UINT wMsg, DWORD_PTR dwInstance,
                       DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    device_infos_t *dev_infos = (device_infos_t *) dwInstance;
    fluid_winmidi_driver_t *dev = dev_infos->dev;
    fluid_midi_event_t event;
    LPMIDIHDR pMidiHdr;
    unsigned char *data;
    unsigned int msg_param = (unsigned int) dwParam1;

    switch(wMsg)
    {
    case MIM_OPEN:
        break;

    case MIM_CLOSE:
        break;

    case MIM_DATA:
        if(msg_type(msg_param) < 0xf0)      /* Voice category message */
        {
            event.type = msg_type(msg_param);
            event.channel = msg_chan(msg_param) + dev_infos->channel_map;

            FLUID_LOG(FLUID_DBG, "\ndevice at index %d sending MIDI message on channel %d, forwarded on channel: %d",
                      dev_infos->dev_idx, msg_chan(msg_param), event.channel);

            if(event.type != PITCH_BEND)
            {
                event.param1 = msg_p1(msg_param);
                event.param2 = msg_p2(msg_param);
            }
            else      /* Pitch bend is a 14 bit value */
            {
                event.param1 = (msg_p2(msg_param) << 7) | msg_p1(msg_param);
                event.param2 = 0;
            }
        }
        else                    /* System message */
        {
            event.type = (unsigned char)(msg_param & 0xff);
        }

        (*dev->driver.handler)(dev->driver.data, &event);
        break;

    case MIM_LONGDATA:    /* SYSEX data */
        FLUID_LOG(FLUID_DBG, "\ndevice at index %d sending MIDI sysex message",
                  dev_infos->dev_idx);

        if(dev->hThread == NULL)
        {
            break;
        }

        pMidiHdr = (LPMIDIHDR)dwParam1;
        data = (unsigned char *)(pMidiHdr->lpData);

        /* We only process complete SYSEX messages (discard those that are too small or too large) */
        if(pMidiHdr->dwBytesRecorded > 2 && data[0] == 0xF0
                && data[pMidiHdr->dwBytesRecorded - 1] == 0xF7)
        {
            fluid_midi_event_set_sysex(&event, pMidiHdr->lpData + 1,
                                       pMidiHdr->dwBytesRecorded - 2, FALSE);
            (*dev->driver.handler)(dev->driver.data, &event);
        }

        /* request the sysex thread to re-add this buffer into the device dev_infos->midi_num */
        PostThreadMessage(dev->dwThread, MM_MIM_LONGDATA, dev_infos->midi_num, dwParam1);
        break;

    case MIM_ERROR:
        break;

    case MIM_LONGERROR:
        break;

    case MIM_MOREDATA:
        break;
    }
}

/**
 * build a device name prefixed by its index. The format of the returned
 * name is: dev_idx:dev_name
 * The name returned is convenient for midi.winmidi.device setting.
 * It allows the user to identify a device index through its name or vice
 * versa. This allows the user to specify a multi device name using a list of
 * devices index (see fluid_winmidi_midi_driver_settings()).
 *
 * @param dev_idx, device index.
 * @param dev_name, name of the device.
 * @return the new device name (that must be freed when finish with it) or
 *  NULL if memory allocation error.
 */
static char *fluid_winmidi_get_device_name(int dev_idx, char *dev_name)
{
    char *new_dev_name;

    int i =  dev_idx;
    size_t size = 0; /* index size */

    do
    {
        size++;
        i = i / 10 ;
    }
    while(i);

    /* index size + separator + name length + zero termination */
    new_dev_name = FLUID_MALLOC(size + 2 + FLUID_STRLEN(dev_name));

    if(new_dev_name)
    {
        /* the name is filled if allocation is successful */
        FLUID_SPRINTF(new_dev_name, "%d:%s", dev_idx, dev_name);
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
    }

    return new_dev_name;
}

/*
 Add setting midi.winmidi.device in the settings.

 MIDI devices names are enumerated and added to midi.winmidi.device setting
 options. Example:
 0:Port MIDI SB Live! [CE00], 1:SB PCI External MIDI, default, x[;y;z;..]

 Devices name prefixed by index (i.e 1:SB PCI External MIDI) are real devices.
 "default" name is the default device.
 "x[;y;z;..]" is the multi device naming. Its purpose is to indicate to
 the user how he must specify a multi device name in the setting.
 A multi devices name must be a list of real devices index separated by semicolon:
 Example: "5;3;0"
*/
void fluid_winmidi_midi_driver_settings(fluid_settings_t *settings)
{
    MMRESULT res;
    MIDIINCAPS in_caps;
    UINT i, num;

    /* register midi.winmidi.device */
    fluid_settings_register_str(settings, "midi.winmidi.device", "default", 0);
    num = midiInGetNumDevs();

    if(num > 0)
    {
        fluid_settings_add_option(settings, "midi.winmidi.device", "default");

        /* add real devices names in options list */
        for(i = 0; i < num; i++)
        {
            res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

            if(res == MMSYSERR_NOERROR)
            {
                /* add new device name (prefixed by its index) */
                char *new_dev_name = fluid_winmidi_get_device_name(i, in_caps.szPname);

                if(!new_dev_name)
                {
                    break;
                }

                fluid_settings_add_option(settings, "midi.winmidi.device",
                                          new_dev_name);
                FLUID_FREE(new_dev_name);
            }
        }
    }
}

/* Thread for re-adding SYSEX buffers */
static DWORD WINAPI fluid_winmidi_add_sysex_thread(void *data)
{
    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *)data;
    MSG msg;
    int code;

    for(;;)
    {
        code = GetMessage(&msg, NULL, 0, 0);

        if(code < 0)
        {
            FLUID_LOG(FLUID_ERR, "fluid_winmidi_add_sysex_thread: GetMessage() failed.");
            break;
        }

        if(msg.message == WM_CLOSE)
        {
            break;
        }

        switch(msg.message)
        {
        case MM_MIM_LONGDATA:
            /* re-add the buffer into the device designed by msg.wParam parameter */
            midiInAddBuffer(dev->dev_infos[msg.wParam].hmidiin,
                            (LPMIDIHDR)msg.lParam, sizeof(MIDIHDR));
            break;
        }
    }

    return 0;
}

/**
 * Parse device name
 * @param dev if not NULL pointer on driver structure in which device index
 *  are returned.
 * @param dev_name device name which is expected to be:
 *  - a multi devices naming (i.e "1;0;2") or
 *  - a single device name (i.e "0:Port MIDI SB Live! [CE00]"
 * @return count of devices parsed or 0 if device name doesn't exist.
 */
static int
fluid_winmidi_parse_device_name(fluid_winmidi_driver_t *dev, char *dev_name)
{
    int dev_count = 0; /* device count */
    int dev_idx;       /* device index */
    char *cur_idx, *next_idx;      /* current and next ascii index pointer */
    char cpy_dev_name[MAXPNAMELEN];
    int num = midiInGetNumDevs(); /* get number of real devices installed */

    /* look for a multi device naming */
    /* multi devices name "x;[y;..]". parse devices index: x;y;..
       Each ascii index are separated by a semicolon character.
    */
    FLUID_STRCPY(cpy_dev_name, dev_name); /* fluid_strtok() will overwrite */
    next_idx = cpy_dev_name;

    while(NULL != (cur_idx = fluid_strtok(&next_idx, " ;")))
    {
        /* try to convert current ascii index */
        char *end_idx = cur_idx;
        dev_idx = FLUID_STRTOL(cur_idx, &end_idx, 10);

        if(cur_idx == end_idx      /* not an integer number */
           || dev_idx < 0          /* invalid device index */
           || dev_idx >= num       /* invalid device index */
          )
        {
            if(dev)
            {
                dev->dev_count = 0;
            }

            dev_count = 0; /* error, end of parsing */
            break;
        }

        /* memorize device index in dev_infos table */
        if(dev)
        {
            dev->dev_infos[dev->dev_count++].dev_idx = dev_idx;
        }

        dev_count++;
    }

    /* look for single device if multi devices not found */
    if(!dev_count)
    {
        /* default device index: dev_idx = 0, dev_count = 1 */
        dev_count = 1;
        dev_idx = 0;

        if(FLUID_STRCASECMP("default", dev_name) != 0)
        {
            int i;
            dev_count = 0; /* reset count of devices found */

            for(i = 0; i < num; i++)
            {
                char strError[MAXERRORLENGTH];
                MIDIINCAPS in_caps;
                MMRESULT res;
                res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

                if(res == MMSYSERR_NOERROR)
                {
                    int str_cmp_res;
                    char *new_dev_name = fluid_winmidi_get_device_name(i, in_caps.szPname);

                    if(!new_dev_name)
                    {
                        break;
                    }

#ifdef _UNICODE
                    WCHAR wDevName[MAXPNAMELEN];
                    MultiByteToWideChar(CP_UTF8, 0, dev_name, -1, wDevName, MAXPNAMELEN);

                    str_cmp_res = wcsicmp(wDevName, new_dev_name);
#else
                    str_cmp_res = FLUID_STRCASECMP(dev_name, new_dev_name);
#endif

                    FLUID_LOG(FLUID_DBG, "Testing midi device \"%s\"", new_dev_name);
                    FLUID_FREE(new_dev_name);

                    if(str_cmp_res == 0)
                    {
                        FLUID_LOG(FLUID_DBG, "Selected midi device number: %u", i);
                        dev_idx = i;
                        dev_count = 1;
                        break;
                    }
                }
                else
                {
                    FLUID_LOG(FLUID_DBG, "Error testing midi device %u of %u: %s (error %d)",
                              i, num, fluid_winmidi_input_error(strError, res), res);
                }
            }
        }

        if(dev && dev_count)
        {
            dev->dev_infos[0].dev_idx = dev_idx;
            dev->dev_count = 1;
        }
    }

    if(num < dev_count)
    {
        FLUID_LOG(FLUID_ERR, "not enough MIDI in devices found. Expected:%d found:%d",
                  dev_count, num);
        dev_count = 0;
    }

    return dev_count;
}

static void fluid_winmidi_autoconnect_build_name(char *name)
{
    char new_name[MAXPNAMELEN] = { 0 };
    int i, j, n = 0;
    int num = midiInGetNumDevs();

    for (i = 0; i < num; ++i)
    {
        char x[4];
        j = FLUID_SNPRINTF(x, sizeof(x), "%d;", i);
        n += j;
        if (n >= sizeof(new_name))
        {
            FLUID_LOG(FLUID_DBG, "winmidi: autoconnect dev name exceeds MAXPNAMELEN (%d), num (%d), n (%d)", MAXPNAMELEN, num, n);
            return;
        }
        strncat(new_name, x, j);
    }

    name[n - 1] = 0;

    FLUID_MEMSET(name, 0, MAXPNAMELEN);
    FLUID_STRCPY(name, new_name);
}

/*
 * new_fluid_winmidi_driver
 */
fluid_midi_driver_t *
new_fluid_winmidi_driver(fluid_settings_t *settings,
                         handle_midi_event_func_t handler, void *data)
{
    fluid_winmidi_driver_t *dev;
    MMRESULT res;
    int i, j;
    int max_devices;  /* maximum number of devices to handle */
    int autoconnect_inputs = 0;
    int midi_channels = 16;
    int ch_map = 0;
    char strError[MAXERRORLENGTH];
    char dev_name[MAXPNAMELEN];

    /* not much use doing anything */
    if(handler == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Invalid argument");
        return NULL;
    }

    /* get the device name. if none is specified, use the default device. */
    if(fluid_settings_copystr(settings, "midi.winmidi.device", dev_name, MAXPNAMELEN) != FLUID_OK)
    {
        FLUID_LOG(FLUID_DBG, "No MIDI in device selected, using \"default\"");
        FLUID_STRCPY(dev_name, "default");
    }

    fluid_settings_getint(settings, "midi.autoconnect", &autoconnect_inputs);
    fluid_settings_getint(settings, "synth.midi-channels", &midi_channels);

    if((strcmp(dev_name, "default") == 0) && (autoconnect_inputs != 0))
    {
        fluid_winmidi_autoconnect_build_name(dev_name);
        FLUID_LOG(FLUID_DBG, "winmidi: autoconnect device name is now '%s'", dev_name);
    }

    /* parse device name, get the maximum number of devices to handle */
    max_devices = fluid_winmidi_parse_device_name(NULL, dev_name);

    /* check if any device has be found	*/
    if(!max_devices)
    {
        FLUID_LOG(FLUID_ERR, "Device \"%s\" does not exists", dev_name);
        return NULL;
    }

    /* allocation of driver structure size dependent of max_devices */
    i = sizeof(fluid_winmidi_driver_t) + (max_devices - 1) * sizeof(device_infos_t);
    dev = FLUID_MALLOC(i);

    if(dev == NULL)
    {
        return NULL;
    }

    FLUID_MEMSET(dev, 0, i); /* reset structure members */

    /* parse device name, get devices index  */
    fluid_winmidi_parse_device_name(dev, dev_name);

    dev->driver.handler = handler;
    dev->driver.data = data;

    /* try opening the devices */
    for(i = 0; i < dev->dev_count; i++)
    {
        device_infos_t *dev_infos = &dev->dev_infos[i];
        dev_infos->dev = dev;            /* driver structure */
        dev_infos->midi_num = i;         /* device order number */
        dev_infos->channel_map = ch_map; /* map from input to output */
        /* calculate the next channel mapping, up to synth.midi-channels */
        ch_map += 16;

        if(ch_map >= midi_channels)
        {
            ch_map = 0;
        }

        FLUID_LOG(FLUID_DBG, "opening device at index %d", dev_infos->dev_idx);
        res = midiInOpen(&dev_infos->hmidiin, dev_infos->dev_idx,
                         (DWORD_PTR) fluid_winmidi_callback,
                         (DWORD_PTR) dev_infos, CALLBACK_FUNCTION);

        if(res != MMSYSERR_NOERROR)
        {
            FLUID_LOG(FLUID_ERR, "Couldn't open MIDI input: %s (error %d)",
                      fluid_winmidi_input_error(strError, res), res);
            goto error_recovery;
        }

        /* Prepare and add SYSEX buffers */
        for(j = 0; j < MIDI_SYSEX_BUF_COUNT; j++)
        {
            MIDIHDR *hdr = &dev_infos->sysExHdrs[j];

            hdr->lpData = (LPSTR)&dev_infos->sysExBuf[j * MIDI_SYSEX_MAX_SIZE];
            hdr->dwBufferLength = MIDI_SYSEX_MAX_SIZE;

            /* Prepare a buffer for SYSEX data and add it */
            res = midiInPrepareHeader(dev_infos->hmidiin, hdr, sizeof(MIDIHDR));

            if(res == MMSYSERR_NOERROR)
            {
                res = midiInAddBuffer(dev_infos->hmidiin, hdr, sizeof(MIDIHDR));

                if(res != MMSYSERR_NOERROR)
                {
                    FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                              fluid_winmidi_input_error(strError, res), res);
                    midiInUnprepareHeader(dev_infos->hmidiin, hdr, sizeof(MIDIHDR));
                }
            }
            else
                FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                          fluid_winmidi_input_error(strError, res), res);
        }
    }


    /* Create thread which processes re-adding SYSEX buffers */
    dev->hThread = CreateThread(
                       NULL,
                       0,
                       (LPTHREAD_START_ROUTINE)
                       fluid_winmidi_add_sysex_thread,
                       dev,
                       0,
                       &dev->dwThread);

    if(dev->hThread == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create SYSEX buffer processing thread");
        goto error_recovery;
    }

    /* Start the MIDI input interface */
    for(i = 0; i < dev->dev_count; i++)
    {
        if(midiInStart(dev->dev_infos[i].hmidiin) != MMSYSERR_NOERROR)
        {
            FLUID_LOG(FLUID_ERR, "Failed to start the MIDI input. MIDI input not available.");
            goto error_recovery;
        }
    }

    return (fluid_midi_driver_t *) dev;

error_recovery:

    delete_fluid_winmidi_driver((fluid_midi_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_winmidi_driver
 */
void
delete_fluid_winmidi_driver(fluid_midi_driver_t *p)
{
    int i, j;

    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    /* request the sysex thread to terminate */
    if(dev->hThread != NULL)
    {
        PostThreadMessage(dev->dwThread, WM_CLOSE, 0, 0);
        WaitForSingleObject(dev->hThread, INFINITE);

        CloseHandle(dev->hThread);
        dev->hThread = NULL;
    }

    /* stop MIDI in devices and free allocated buffers */
    for(i = 0; i < dev->dev_count; i++)
    {
        device_infos_t *dev_infos = &dev->dev_infos[i];

        if(dev_infos->hmidiin != NULL)
        {
            /* stop the device and mark any pending data blocks as being done */
            midiInReset(dev_infos->hmidiin);

            /* free allocated buffers associated to this device */
            for(j = 0; j < MIDI_SYSEX_BUF_COUNT; j++)
            {
                MIDIHDR *hdr = &dev_infos->sysExHdrs[j];

                if((hdr->dwFlags & MHDR_PREPARED))
                {
                    midiInUnprepareHeader(dev_infos->hmidiin, hdr, sizeof(MIDIHDR));
                }
            }

            /* close the device */
            midiInClose(dev_infos->hmidiin);
        }
    }

    FLUID_FREE(dev);
}

#endif /* WINMIDI_SUPPORT */
