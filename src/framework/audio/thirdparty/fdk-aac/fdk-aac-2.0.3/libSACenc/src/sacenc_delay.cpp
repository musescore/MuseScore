/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2018 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.

 1.    INTRODUCTION
The Fraunhofer FDK AAC Codec Library for Android ("FDK AAC Codec") is software
that implements the MPEG Advanced Audio Coding ("AAC") encoding and decoding
scheme for digital audio. This FDK AAC Codec software is intended to be used on
a wide variety of Android devices.

AAC's HE-AAC and HE-AAC v2 versions are regarded as today's most efficient
general perceptual audio codecs. AAC-ELD is considered the best-performing
full-bandwidth communications codec by independent studies and is widely
deployed. AAC has been standardized by ISO and IEC as part of the MPEG
specifications.

Patent licenses for necessary patent claims for the FDK AAC Codec (including
those of Fraunhofer) may be obtained through Via Licensing
(www.vialicensing.com) or through the respective patent owners individually for
the purpose of encoding or decoding bit streams in products that are compliant
with the ISO/IEC MPEG audio standards. Please note that most manufacturers of
Android devices already license these patent claims through Via Licensing or
directly from the patent owners, and therefore FDK AAC Codec software may
already be covered under those patent licenses when it is used for those
licensed purposes only.

Commercially-licensed AAC software libraries, including floating-point versions
with enhanced sound quality, are also available from Fraunhofer. Users are
encouraged to check the Fraunhofer website for additional applications
information and documentation.

2.    COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

You must retain the complete text of this software license in redistributions of
the FDK AAC Codec or your modifications thereto in source code form.

You must retain the complete text of this software license in the documentation
and/or other materials provided with redistributions of the FDK AAC Codec or
your modifications thereto in binary form. You must make available free of
charge copies of the complete source code of the FDK AAC Codec and your
modifications thereto to recipients of copies in binary form.

The name of Fraunhofer may not be used to endorse or promote products derived
from this library without prior written permission.

You may not charge copyright license fees for anyone to use, copy or distribute
the FDK AAC Codec software or your modifications thereto.

Your modified versions of the FDK AAC Codec must carry prominent notices stating
that you changed the software and the date of any change. For modified versions
of the FDK AAC Codec, the term "Fraunhofer FDK AAC Codec Library for Android"
must be replaced by the term "Third-Party Modified Version of the Fraunhofer FDK
AAC Codec Library for Android."

3.    NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software.

You may use this FDK AAC Codec software or modifications thereto only for
purposes that are authorized by appropriate patent licenses.

4.    DISCLAIMER

This FDK AAC Codec software is provided by Fraunhofer on behalf of the copyright
holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
including but not limited to the implied warranties of merchantability and
fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary,
or consequential damages, including but not limited to procurement of substitute
goods or services; loss of use, data, or profits, or business interruption,
however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of
this software, even if advised of the possibility of such damage.

5.    CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Audio and Multimedia Departments - FDK AAC LL
Am Wolfsmantel 33
91058 Erlangen, Germany

www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
----------------------------------------------------------------------------- */

/*********************** MPEG surround encoder library *************************

   Author(s):   Christian Goettlinger

   Description: Encoder Library Interface
                delay management of the encoder

*******************************************************************************/

/**************************************************************************/ /**
 \file
 This file contains all delay infrastructure
 ******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_delay.h"
#include "sacenc_const.h"
#include "FDK_matrixCalloc.h"

/* Defines *******************************************************************/

/* Data Types ****************************************************************/
struct DELAY {
  struct DELAY_CONFIG {
    /* Routing Config Switches*/
    INT bDmxAlign;
    INT bTimeDomDmx;
    INT bMinimizeDelay;
    INT bSacTimeAlignmentDynamicOut;

    /* Needed Input Variables*/
    INT nQmfLen;
    INT nFrameLen;
    INT nSurroundDelay;
    INT nArbDmxDelay;
    INT nLimiterDelay;
    INT nCoreCoderDelay;
    INT nSacStreamMuxDelay;
    INT nSacTimeAlignment; /* Overwritten, if bSacTimeAlignmentDynamicOut */
  } config;

  /* Variable Delaybuffers -> Delays */
  INT nDmxAlignBuffer;
  INT nSurroundAnalysisBuffer;
  INT nArbDmxAnalysisBuffer;
  INT nOutputAudioBuffer;
  INT nBitstreamFrameBuffer;
  INT nOutputAudioQmfFrameBuffer;
  INT nDiscardOutFrames;

  /* Variable Delaybuffers Computation Variables */
  INT nBitstreamFrameBufferSize;

  /* Output: Infos */
  INT nInfoDmxDelay; /* Delay of the downmixed signal after the space encoder */
  INT nInfoCodecDelay; /* Delay of the whole en-/decoder including CoreCoder */
  INT nInfoDecoderDelay; /* Delay of the Mpeg Surround decoder */
};

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/

/*-----------------------------------------------------------------------------
functionname: fdk_sacenc_delay_Open()
description:  initializes Delays
returns:      noError on success, an apropriate error code else
-----------------------------------------------------------------------------*/
FDK_SACENC_ERROR fdk_sacenc_delay_Open(HANDLE_DELAY *phDelay) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == phDelay) {
    error = SACENC_INVALID_HANDLE;
  } else {
    FDK_ALLOCATE_MEMORY_1D(*phDelay, 1, struct DELAY);
  }
  return error;

bail:
  fdk_sacenc_delay_Close(phDelay);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

/*-----------------------------------------------------------------------------
functionname: fdk_sacenc_delay_Close()
description:  destructs Delay
returns:      noError on success, an apropriate error code else
-----------------------------------------------------------------------------*/
FDK_SACENC_ERROR fdk_sacenc_delay_Close(HANDLE_DELAY *phDelay) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == phDelay) {
    error = SACENC_INVALID_HANDLE;
  } else {
    if (NULL != *phDelay) {
      FDK_FREE_MEMORY_1D(*phDelay);
    }
  }
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_delay_Init(HANDLE_DELAY hDelay, const INT nQmfLen,
                                       const INT nFrameLen,
                                       const INT nCoreCoderDelay,
                                       const INT nSacStreamMuxDelay) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == hDelay) {
    error = SACENC_INVALID_HANDLE;
  } else {
    /* Fill structure before calculation */
    FDKmemclear(&hDelay->config, sizeof(hDelay->config));

    hDelay->config.nQmfLen = nQmfLen;
    hDelay->config.nFrameLen = nFrameLen;
    hDelay->config.nCoreCoderDelay = nCoreCoderDelay;
    hDelay->config.nSacStreamMuxDelay = nSacStreamMuxDelay;
  }
  return error;
}

/*-----------------------------------------------------------------------------
functionname: fdk_sacenc_delay_SubCalulateBufferDelays()
description:  Calculates the Delays of the buffers
returns:      Error Code
-----------------------------------------------------------------------------*/
FDK_SACENC_ERROR fdk_sacenc_delay_SubCalulateBufferDelays(HANDLE_DELAY hDel) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == hDel) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int nEncoderAnDelay, nEncoderSynDelay, nEncoderWinDelay, nDecoderAnDelay,
        nDecoderSynDelay, nResidualCoderFrameDelay,
        nArbDmxResidualCoderFrameDelay;

    if (hDel->config.bSacTimeAlignmentDynamicOut > 0) {
      hDel->config.nSacTimeAlignment = 0;
    }

    {
      nEncoderAnDelay =
          2 * hDel->config.nQmfLen +
          hDel->config.nQmfLen / 2; /* Only Ld-QMF Delay, no hybrid */
      nEncoderSynDelay = 1 * hDel->config.nQmfLen + hDel->config.nQmfLen / 2;
      nDecoderAnDelay = 2 * hDel->config.nQmfLen + hDel->config.nQmfLen / 2;
      nDecoderSynDelay = 1 * hDel->config.nQmfLen + hDel->config.nQmfLen / 2;
      nEncoderWinDelay =
          hDel->config.nFrameLen / 2; /* WindowLookahead is just half a frame */
    }

    { nResidualCoderFrameDelay = 0; }

    { nArbDmxResidualCoderFrameDelay = 0; }

    /* Calculate variable Buffer-Delays */
    if (hDel->config.bTimeDomDmx == 0) {
      /* ArbitraryDmx and TdDmx off */
      int tempDelay;

      hDel->nSurroundAnalysisBuffer = 0;
      hDel->nArbDmxAnalysisBuffer = 0;
      tempDelay = nEncoderSynDelay + hDel->config.nLimiterDelay +
                  hDel->config.nCoreCoderDelay +
                  hDel->config.nSacTimeAlignment + nDecoderAnDelay;
      tempDelay = (nResidualCoderFrameDelay * hDel->config.nFrameLen) +
                  hDel->config.nSacStreamMuxDelay - tempDelay;

      if (tempDelay > 0) {
        hDel->nBitstreamFrameBuffer = 0;
        hDel->nOutputAudioBuffer = tempDelay;
      } else {
        tempDelay = -tempDelay;
        hDel->nBitstreamFrameBuffer =
            (tempDelay + hDel->config.nFrameLen - 1) / hDel->config.nFrameLen;
        hDel->nOutputAudioBuffer =
            (hDel->nBitstreamFrameBuffer * hDel->config.nFrameLen) - tempDelay;
      }

      hDel->nOutputAudioQmfFrameBuffer =
          (hDel->nOutputAudioBuffer + (hDel->config.nQmfLen / 2) - 1) /
          hDel->config.nQmfLen;

      if (hDel->config.bDmxAlign > 0) {
        tempDelay = nEncoderWinDelay + nEncoderAnDelay + nEncoderSynDelay +
                    hDel->nOutputAudioBuffer + hDel->config.nLimiterDelay +
                    hDel->config.nCoreCoderDelay;
        hDel->nDiscardOutFrames =
            (tempDelay + hDel->config.nFrameLen - 1) / hDel->config.nFrameLen;
        hDel->nDmxAlignBuffer =
            hDel->nDiscardOutFrames * hDel->config.nFrameLen - tempDelay;
      } else {
        hDel->nDiscardOutFrames = 0;
        hDel->nDmxAlignBuffer = 0;
      }

      /* Output: Info-Variables */
      hDel->nInfoDmxDelay = hDel->nSurroundAnalysisBuffer + nEncoderAnDelay +
                            nEncoderWinDelay + nEncoderSynDelay +
                            hDel->nOutputAudioBuffer +
                            hDel->config.nLimiterDelay;
      hDel->nInfoCodecDelay =
          hDel->nInfoDmxDelay + hDel->config.nCoreCoderDelay +
          hDel->config.nSacTimeAlignment + nDecoderAnDelay + nDecoderSynDelay;

    } else {
      /* ArbitraryDmx or TdDmx on */
      int tempDelay1, tempDelay2, tempDelay12, tempDelay3;

      tempDelay1 = hDel->config.nArbDmxDelay - hDel->config.nSurroundDelay;

      if (tempDelay1 >= 0) {
        hDel->nSurroundAnalysisBuffer = tempDelay1;
        hDel->nArbDmxAnalysisBuffer = 0;
      } else {
        hDel->nSurroundAnalysisBuffer = 0;
        hDel->nArbDmxAnalysisBuffer = -tempDelay1;
      }

      tempDelay1 = nEncoderWinDelay + hDel->config.nSurroundDelay +
                   hDel->nSurroundAnalysisBuffer +
                   nEncoderAnDelay; /*Surround Path*/
      tempDelay2 = nEncoderWinDelay + hDel->config.nArbDmxDelay +
                   hDel->nArbDmxAnalysisBuffer +
                   nEncoderAnDelay; /* ArbDmx Compare Path */
      tempDelay3 = hDel->config.nArbDmxDelay + hDel->config.nLimiterDelay +
                   hDel->config.nCoreCoderDelay +
                   hDel->config.nSacTimeAlignment +
                   nDecoderAnDelay; /* ArbDmx Passthrough*/

      tempDelay12 =
          FDKmax(nResidualCoderFrameDelay, nArbDmxResidualCoderFrameDelay) *
          hDel->config.nFrameLen;
      tempDelay12 += hDel->config.nSacStreamMuxDelay;

      if (tempDelay1 > tempDelay2) {
        tempDelay12 += tempDelay1;
      } else {
        tempDelay12 += tempDelay2;
      }

      if (tempDelay3 > tempDelay12) {
        if (hDel->config.bMinimizeDelay > 0) {
          hDel->nBitstreamFrameBuffer =
              (tempDelay3 - tempDelay12) / hDel->config.nFrameLen; /*floor*/
          hDel->nOutputAudioBuffer = 0;
          hDel->nSurroundAnalysisBuffer +=
              (tempDelay3 - tempDelay12 -
               (hDel->nBitstreamFrameBuffer * hDel->config.nFrameLen));
          hDel->nArbDmxAnalysisBuffer +=
              (tempDelay3 - tempDelay12 -
               (hDel->nBitstreamFrameBuffer * hDel->config.nFrameLen));
        } else {
          hDel->nBitstreamFrameBuffer =
              ((tempDelay3 - tempDelay12) + hDel->config.nFrameLen - 1) /
              hDel->config.nFrameLen;
          hDel->nOutputAudioBuffer =
              hDel->nBitstreamFrameBuffer * hDel->config.nFrameLen +
              tempDelay12 - tempDelay3;
        }
      } else {
        hDel->nBitstreamFrameBuffer = 0;
        hDel->nOutputAudioBuffer = tempDelay12 - tempDelay3;
      }

      if (hDel->config.bDmxAlign > 0) {
        int tempDelay = hDel->config.nArbDmxDelay + hDel->nOutputAudioBuffer +
                        hDel->config.nLimiterDelay +
                        hDel->config.nCoreCoderDelay;
        hDel->nDiscardOutFrames =
            (tempDelay + hDel->config.nFrameLen - 1) / hDel->config.nFrameLen;
        hDel->nDmxAlignBuffer =
            hDel->nDiscardOutFrames * hDel->config.nFrameLen - tempDelay;
      } else {
        hDel->nDiscardOutFrames = 0;
        hDel->nDmxAlignBuffer = 0;
      }

      /* Output: Info-Variables */
      hDel->nInfoDmxDelay = hDel->config.nArbDmxDelay +
                            hDel->nOutputAudioBuffer +
                            hDel->config.nLimiterDelay;
      hDel->nInfoCodecDelay =
          hDel->nInfoDmxDelay + hDel->config.nCoreCoderDelay +
          hDel->config.nSacTimeAlignment + nDecoderAnDelay + nDecoderSynDelay;
      hDel->nInfoDecoderDelay = nDecoderAnDelay + nDecoderSynDelay;

    } /* ArbitraryDmx or TdDmx on */

    /* Additonal Variables needed for Computation Issues */
    hDel->nBitstreamFrameBufferSize = hDel->nBitstreamFrameBuffer + 1;
  }

  return error;
}

static FDK_SACENC_ERROR assignParameterInRange(
    const INT startRange, /* including startRange */
    const INT stopRange,  /* including stopRange */
    const INT value,      /* value to write*/
    INT *const ptr        /* destination pointer*/
) {
  FDK_SACENC_ERROR error = SACENC_INVALID_CONFIG;

  if ((startRange <= value) && (value <= stopRange)) {
    *ptr = value;
    error = SACENC_OK;
  }

  return error;
}

FDK_SACENC_ERROR fdk_sacenc_delay_SetDmxAlign(HANDLE_DELAY hDelay,
                                              const INT bDmxAlignIn) {
  return (assignParameterInRange(0, 1, bDmxAlignIn, &hDelay->config.bDmxAlign));
}

FDK_SACENC_ERROR fdk_sacenc_delay_SetTimeDomDmx(HANDLE_DELAY hDelay,
                                                const INT bTimeDomDmxIn) {
  return (
      assignParameterInRange(0, 1, bTimeDomDmxIn, &hDelay->config.bTimeDomDmx));
}

FDK_SACENC_ERROR fdk_sacenc_delay_SetSacTimeAlignmentDynamicOut(
    HANDLE_DELAY hDelay, const INT bSacTimeAlignmentDynamicOutIn) {
  return (assignParameterInRange(0, 1, bSacTimeAlignmentDynamicOutIn,
                                 &hDelay->config.bSacTimeAlignmentDynamicOut));
}

FDK_SACENC_ERROR fdk_sacenc_delay_SetNSacTimeAlignment(
    HANDLE_DELAY hDelay, const INT nSacTimeAlignmentIn) {
  return (assignParameterInRange(-32768, 32767, nSacTimeAlignmentIn,
                                 &hDelay->config.nSacTimeAlignment));
}

FDK_SACENC_ERROR fdk_sacenc_delay_SetMinimizeDelay(HANDLE_DELAY hDelay,
                                                   const INT bMinimizeDelay) {
  return (assignParameterInRange(0, 1, bMinimizeDelay,
                                 &hDelay->config.bMinimizeDelay));
}

INT fdk_sacenc_delay_GetOutputAudioBufferDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nOutputAudioBuffer);
}

INT fdk_sacenc_delay_GetSurroundAnalysisBufferDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nSurroundAnalysisBuffer);
}

INT fdk_sacenc_delay_GetArbDmxAnalysisBufferDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nArbDmxAnalysisBuffer);
}

INT fdk_sacenc_delay_GetBitstreamFrameBufferSize(HANDLE_DELAY hDelay) {
  return (hDelay->nBitstreamFrameBufferSize);
}

INT fdk_sacenc_delay_GetDmxAlignBufferDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nDmxAlignBuffer);
}

INT fdk_sacenc_delay_GetDiscardOutFrames(HANDLE_DELAY hDelay) {
  return (hDelay->nDiscardOutFrames);
}

INT fdk_sacenc_delay_GetInfoDmxDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nInfoDmxDelay);
}

INT fdk_sacenc_delay_GetInfoCodecDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nInfoCodecDelay);
}

INT fdk_sacenc_delay_GetInfoDecoderDelay(HANDLE_DELAY hDelay) {
  return (hDelay->nInfoDecoderDelay);
}
