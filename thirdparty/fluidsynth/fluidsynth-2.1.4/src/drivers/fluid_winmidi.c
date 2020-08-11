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
 */

#include "fluidsynth_priv.h"

#if WINMIDI_SUPPORT

#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#define MIDI_SYSEX_MAX_SIZE     512
#define MIDI_SYSEX_BUF_COUNT    16

typedef struct
{
    fluid_midi_driver_t driver;
    HMIDIIN hmidiin;

    /* MIDI HDR for SYSEX buffer */
    MIDIHDR sysExHdrs[MIDI_SYSEX_BUF_COUNT];

    /* Thread for SYSEX re-add thread */
    HANDLE hThread;
    DWORD  dwThread;

    /* Sysex data buffer */
    unsigned char sysExBuf[MIDI_SYSEX_BUF_COUNT * MIDI_SYSEX_MAX_SIZE];

} fluid_winmidi_driver_t;

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

static void CALLBACK
fluid_winmidi_callback(HMIDIIN hmi, UINT wMsg, DWORD_PTR dwInstance,
                       DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *) dwInstance;
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
        event.type = msg_type(msg_param);
        event.channel = msg_chan(msg_param);

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

        (*dev->driver.handler)(dev->driver.data, &event);
        break;

    case MIM_LONGDATA:    /* SYSEX data */
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

        PostThreadMessage(dev->dwThread, MM_MIM_LONGDATA, 0, dwParam1);
        break;

    case MIM_ERROR:
        break;

    case MIM_LONGERROR:
        break;

    case MIM_MOREDATA:
        break;
    }
}

void fluid_winmidi_midi_driver_settings(fluid_settings_t *settings)
{
    MMRESULT res;
    MIDIINCAPS in_caps;
    UINT i, num;
    fluid_settings_register_str(settings, "midi.winmidi.device", "default", 0);
    num = midiInGetNumDevs();

    if(num > 0)
    {
        fluid_settings_add_option(settings, "midi.winmidi.device", "default");

        for(i = 0; i < num; i++)
        {
            res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

            if(res == MMSYSERR_NOERROR)
            {
                fluid_settings_add_option(settings, "midi.winmidi.device", in_caps.szPname);
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
            midiInAddBuffer(dev->hmidiin, (LPMIDIHDR)msg.lParam, sizeof(MIDIHDR));
            break;
        }
    }

    return 0;
}

/*
 * new_fluid_winmidi_driver
 */
fluid_midi_driver_t *
new_fluid_winmidi_driver(fluid_settings_t *settings,
                         handle_midi_event_func_t handler, void *data)
{
    fluid_winmidi_driver_t *dev;
    MIDIHDR *hdr;
    MMRESULT res;
    UINT i, num, midi_num = 0;
    MIDIINCAPS in_caps;
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

    /* check if there any midi devices installed */
    num = midiInGetNumDevs();

    if(num == 0)
    {
        FLUID_LOG(FLUID_ERR, "no MIDI in devices found");
        return NULL;
    }

    /* find the device */
    if(FLUID_STRCASECMP("default", dev_name) != 0)
    {
        for(i = 0; i < num; i++)
        {
            res = midiInGetDevCaps(i, &in_caps, sizeof(MIDIINCAPS));

            if(res == MMSYSERR_NOERROR)
            {
                int str_cmp_res;
#ifdef _UNICODE
                WCHAR wDevName[MAXPNAMELEN];
                MultiByteToWideChar(CP_UTF8, 0, dev_name, -1, wDevName, MAXPNAMELEN);

                str_cmp_res = wcsicmp(wDevName, in_caps.szPname);
#else
                str_cmp_res = FLUID_STRCASECMP(dev_name, in_caps.szPname);
#endif

                FLUID_LOG(FLUID_DBG, "Testing midi device \"%s\"", in_caps.szPname);

                if(str_cmp_res == 0)
                {
                    FLUID_LOG(FLUID_DBG, "Selected midi device number: %u", i);
                    midi_num = i;
                    break;
                }
            }
            else
            {
                FLUID_LOG(FLUID_DBG, "Error testing midi device %u of %u: %s (error %d)", i, num, fluid_winmidi_input_error(strError, res), res);
            }
        }

        if(midi_num != i)
        {
            FLUID_LOG(FLUID_ERR, "Device \"%s\" does not exists", dev_name);
            return NULL;
        }
    }

    dev = FLUID_MALLOC(sizeof(fluid_winmidi_driver_t));

    if(dev == NULL)
    {
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_winmidi_driver_t));

    dev->hmidiin = NULL;
    dev->driver.handler = handler;
    dev->driver.data = data;

    /* try opening the device */
    res = midiInOpen(&dev->hmidiin, midi_num,
                     (DWORD_PTR) fluid_winmidi_callback,
                     (DWORD_PTR) dev, CALLBACK_FUNCTION);

    if(res != MMSYSERR_NOERROR)
    {
        FLUID_LOG(FLUID_ERR, "Couldn't open MIDI input: %s (error %d)",
                  fluid_winmidi_input_error(strError, res), res);
        goto error_recovery;
    }

    /* Prepare and add SYSEX buffers */
    for(i = 0; i < MIDI_SYSEX_BUF_COUNT; i++)
    {
        hdr = &dev->sysExHdrs[i];

        hdr->lpData = (LPSTR)&dev->sysExBuf[i * MIDI_SYSEX_MAX_SIZE];
        hdr->dwBufferLength = MIDI_SYSEX_MAX_SIZE;

        /* Prepare a buffer for SYSEX data and add it */
        res = midiInPrepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));

        if(res == MMSYSERR_NOERROR)
        {
            res = midiInAddBuffer(dev->hmidiin, hdr, sizeof(MIDIHDR));

            if(res != MMSYSERR_NOERROR)
            {
                FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                          fluid_winmidi_input_error(strError, res), res);
                midiInUnprepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));
            }
        }
        else
            FLUID_LOG(FLUID_WARN, "Failed to prepare MIDI SYSEX buffer: %s (error %d)",
                      fluid_winmidi_input_error(strError, res), res);
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
    if(midiInStart(dev->hmidiin) != MMSYSERR_NOERROR)
    {
        FLUID_LOG(FLUID_ERR, "Failed to start the MIDI input. MIDI input not available.");
        goto error_recovery;
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
    int i;

    fluid_winmidi_driver_t *dev = (fluid_winmidi_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    if(dev->hThread != NULL)
    {
        PostThreadMessage(dev->dwThread, WM_CLOSE, 0, 0);
        WaitForSingleObject(dev->hThread, INFINITE);

        CloseHandle(dev->hThread);
        dev->hThread = NULL;
    }

    if(dev->hmidiin != NULL)
    {
        midiInStop(dev->hmidiin);
        midiInReset(dev->hmidiin);

        for(i = 0; i < MIDI_SYSEX_BUF_COUNT; i++)
        {
            MIDIHDR *hdr = &dev->sysExHdrs[i];

            if ((hdr->dwFlags & MHDR_PREPARED))
            {
                midiInUnprepareHeader(dev->hmidiin, hdr, sizeof(MIDIHDR));
            }
        }

        midiInClose(dev->hmidiin);
    }

    FLUID_FREE(dev);
}

#endif /* WINMIDI_SUPPORT */
