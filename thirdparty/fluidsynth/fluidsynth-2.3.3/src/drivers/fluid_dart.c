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

/* fluid_dart.c
 *
 * Driver for OS/2 DART
 *
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"
#include "fluid_sys.h"

#if DART_SUPPORT

#define INCL_DOS
#include <os2.h>

#define INCL_OS2MM
#include <os2me.h>

#define NUM_MIX_BUFS        2

/** fluid_dart_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    fluid_synth_t *synth;
    int frame_size;
    USHORT usDeviceID;                          /* Amp Mixer device id     */
    MCI_MIX_BUFFER MixBuffers[NUM_MIX_BUFS];    /* Device buffers          */
    MCI_MIXSETUP_PARMS MixSetupParms;           /* Mixer parameters        */
    MCI_BUFFER_PARMS BufferParms;               /* Device buffer parms     */
} fluid_dart_audio_driver_t;

static HMODULE m_hmodMDM = NULLHANDLE;
static ULONG(APIENTRY *m_pfnmciSendCommand)(USHORT, USHORT, ULONG, PVOID, USHORT) = NULL;
static LONG APIENTRY fluid_dart_audio_run(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags);

/**************************************************************
 *
 *        DART audio driver
 *
 */

void fluid_dart_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.dart.device", "default", 0);
}


fluid_audio_driver_t *
new_fluid_dart_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_dart_audio_driver_t *dev;
    double sample_rate;
    int periods, period_size;
    UCHAR szFailedName[ 256 ];
    MCI_AMP_OPEN_PARMS AmpOpenParms;
    int i;
    ULONG rc;

    dev = FLUID_NEW(fluid_dart_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_dart_audio_driver_t));

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* check the format */
    if(!fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        goto error_recovery;
    }

    dev->synth = synth;
    dev->frame_size = 2 * sizeof(short);

    /* Load only once
     */
    if(m_hmodMDM == NULLHANDLE)
    {
        rc = DosLoadModule(szFailedName, sizeof(szFailedName), "MDM", &m_hmodMDM);

        if(rc != 0)
        {
            FLUID_LOG(FLUID_ERR, "Cannot load MDM.DLL for DART due to %s", szFailedName);
            goto error_recovery;
        }

        rc = DosQueryProcAddr(m_hmodMDM, 1, NULL, (PFN *)&m_pfnmciSendCommand);

        if(rc != 0)
        {
            FLUID_LOG(FLUID_ERR, "Cannot find mciSendCommand() in MDM.DLL");
            DosFreeModule(m_hmodMDM);
            m_hmodMDM = NULLHANDLE;
            goto error_recovery;
        }
    }

    /* open the mixer device
     */
    FLUID_MEMSET(&AmpOpenParms, 0, sizeof(MCI_AMP_OPEN_PARMS));
    AmpOpenParms.usDeviceID = (USHORT)0;
    AmpOpenParms.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;

    rc = m_pfnmciSendCommand(0, MCI_OPEN,
                             MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
                             (PVOID)&AmpOpenParms, 0);

    if(rc != MCIERR_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Cannot open DART, rc = %lu", rc);
        goto error_recovery;
    }

    dev->usDeviceID = AmpOpenParms.usDeviceID;

    /* Set the MixSetupParms data structure to match the requirements.
     * This is a global that is used to setup the mixer.
     */
    dev->MixSetupParms.ulBitsPerSample = BPS_16;
    dev->MixSetupParms.ulFormatTag = MCI_WAVE_FORMAT_PCM;
    dev->MixSetupParms.ulSamplesPerSec = sample_rate;
    dev->MixSetupParms.ulChannels = 2;

    /* Setup the mixer for playback of wave data
     */
    dev->MixSetupParms.ulFormatMode = MCI_PLAY;
    dev->MixSetupParms.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
    dev->MixSetupParms.pmixEvent    = fluid_dart_audio_run;

    rc = m_pfnmciSendCommand(dev->usDeviceID, MCI_MIXSETUP,
                             MCI_WAIT | MCI_MIXSETUP_INIT,
                             (PVOID)&dev->MixSetupParms, 0);

    if(rc != MCIERR_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Cannot setup DART, rc = %lu", rc);
        goto error_recovery;
    }

    /* Set up the BufferParms data structure and allocate
     * device buffers from the Amp-Mixer
     */
    dev->BufferParms.ulNumBuffers = NUM_MIX_BUFS;
    dev->BufferParms.ulBufferSize = periods * period_size * dev->frame_size;
    dev->BufferParms.pBufList = dev->MixBuffers;

    rc = m_pfnmciSendCommand(dev->usDeviceID, MCI_BUFFER,
                             MCI_WAIT | MCI_ALLOCATE_MEMORY,
                             (PVOID)&dev->BufferParms, 0);

    if((USHORT)rc != MCIERR_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Cannot allocate memory for DART, rc = %lu", rc);
        goto error_recovery;
    }

    /* Initialize all device buffers.
     */
    for(i = 0; i < NUM_MIX_BUFS; i++)
    {
        FLUID_MEMSET(dev->MixBuffers[i].pBuffer, 0, dev->BufferParms.ulBufferSize);
        dev->MixBuffers[i].ulBufferLength = dev->BufferParms.ulBufferSize;
        dev->MixBuffers[i].ulFlags = 0;
        dev->MixBuffers[i].ulUserParm = (ULONG)dev;
        fluid_synth_write_s16(dev->synth, dev->MixBuffers[i].ulBufferLength / dev->frame_size,
                              dev->MixBuffers[i].pBuffer, 0, 2, dev->MixBuffers[i].pBuffer, 1, 2);
    }

    /* Write buffers to kick off the amp mixer.
     */
    dev->MixSetupParms.pmixWrite(dev->MixSetupParms.ulMixHandle,
                                 dev->MixBuffers,
                                 NUM_MIX_BUFS);

    return (fluid_audio_driver_t *) dev;

error_recovery:

    delete_fluid_dart_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

void delete_fluid_dart_audio_driver(fluid_audio_driver_t *p)
{
    fluid_dart_audio_driver_t *dev = (fluid_dart_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    if(dev->usDeviceID != 0)
    {
        MCI_GENERIC_PARMS    GenericParms;

        /* Send message to stop the audio device
         */
        m_pfnmciSendCommand(dev->usDeviceID, MCI_STOP, MCI_WAIT,
                            (PVOID)&GenericParms, 0);

        /* Deallocate device buffers
         */
        m_pfnmciSendCommand(dev->usDeviceID, MCI_BUFFER,
                            MCI_WAIT | MCI_DEALLOCATE_MEMORY,
                            (PVOID)&dev->BufferParms, 0);

        /* Close device the mixer device
         */
        m_pfnmciSendCommand(dev->usDeviceID, MCI_CLOSE, MCI_WAIT,
                            (PVOID)&GenericParms, 0);
    }

    FLUID_FREE(dev);
}

static LONG APIENTRY fluid_dart_audio_run(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags)
{
    fluid_dart_audio_driver_t *dev = (fluid_dart_audio_driver_t *)pBuffer->ulUserParm;

    switch(ulFlags)
    {
    case MIX_STREAM_ERROR | MIX_WRITE_COMPLETE: /* error occur in device */
    case MIX_WRITE_COMPLETE:                    /* for playback  */
        FLUID_MEMSET(pBuffer->pBuffer, 0, pBuffer->ulBufferLength);
        fluid_synth_write_s16(dev->synth, pBuffer->ulBufferLength / dev->frame_size,
                              pBuffer->pBuffer, 0, 2, pBuffer->pBuffer, 1, 2);
        dev->MixSetupParms.pmixWrite(dev->MixSetupParms.ulMixHandle, pBuffer, 1);
        break;
    }

    return TRUE;
}

#endif /* #if DART_SUPPORT */
