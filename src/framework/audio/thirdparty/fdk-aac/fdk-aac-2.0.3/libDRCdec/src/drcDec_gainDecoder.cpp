/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_preprocess.h"
#include "drcGainDec_init.h"
#include "drcGainDec_process.h"
#include "drcDec_tools.h"

/*******************************************/
/* static functions                        */
/*******************************************/

static int _fitsLocation(DRC_INSTRUCTIONS_UNI_DRC* pInst,
                         const GAIN_DEC_LOCATION drcLocation) {
  int downmixId = pInst->drcApplyToDownmix ? pInst->downmixId[0] : 0;
  switch (drcLocation) {
    case GAIN_DEC_DRC1:
      return (downmixId == 0);
    case GAIN_DEC_DRC1_DRC2:
      return ((downmixId == 0) || (downmixId == DOWNMIX_ID_ANY_DOWNMIX));
    case GAIN_DEC_DRC2:
      return (downmixId == DOWNMIX_ID_ANY_DOWNMIX);
    case GAIN_DEC_DRC3:
      return ((downmixId != 0) && (downmixId != DOWNMIX_ID_ANY_DOWNMIX));
    case GAIN_DEC_DRC2_DRC3:
      return (downmixId != 0);
  }
  return 0;
}

static void _setChannelGains(HANDLE_DRC_GAIN_DECODER hGainDec,
                             const int numChannelGains,
                             const FIXP_DBL* channelGainDb) {
  int i, channelGain_e;
  FIXP_DBL channelGain;
  FDK_ASSERT(numChannelGains <= 8);
  for (i = 0; i < numChannelGains; i++) {
    if (channelGainDb[i] == (FIXP_DBL)MINVAL_DBL) {
      hGainDec->channelGain[i] = (FIXP_DBL)0;
    } else {
      /* add loudness normalisation gain (dB) to channel gain (dB) */
      FIXP_DBL tmp_channelGainDb = (channelGainDb[i] >> 1) +
                                   (hGainDec->loudnessNormalisationGainDb >> 2);
      tmp_channelGainDb =
          SATURATE_LEFT_SHIFT(tmp_channelGainDb, 1, DFRACT_BITS);
      channelGain = dB2lin(tmp_channelGainDb, 8, &channelGain_e);
      hGainDec->channelGain[i] = scaleValue(channelGain, channelGain_e - 8);
    }
  }
}

/*******************************************/
/* public functions                        */
/*******************************************/

DRC_ERROR
drcDec_GainDecoder_Open(HANDLE_DRC_GAIN_DECODER* phGainDec) {
  DRC_GAIN_DECODER* hGainDec = NULL;

  hGainDec = (DRC_GAIN_DECODER*)FDKcalloc(1, sizeof(DRC_GAIN_DECODER));
  if (hGainDec == NULL) return DE_MEMORY_ERROR;

  hGainDec->multiBandActiveDrcIndex = -1;
  hGainDec->channelGainActiveDrcIndex = -1;

  *phGainDec = hGainDec;

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Init(HANDLE_DRC_GAIN_DECODER hGainDec) {
  DRC_ERROR err = DE_OK;

  err = initGainDec(hGainDec);
  if (err) return err;

  initDrcGainBuffers(hGainDec->frameSize, &hGainDec->drcGainBuffers);

  return err;
}

DRC_ERROR
drcDec_GainDecoder_SetParam(HANDLE_DRC_GAIN_DECODER hGainDec,
                            const GAIN_DEC_PARAM paramType,
                            const int paramValue) {
  switch (paramType) {
    case GAIN_DEC_FRAME_SIZE:
      if (paramValue < 0) return DE_PARAM_OUT_OF_RANGE;
      hGainDec->frameSize = paramValue;
      break;
    case GAIN_DEC_SAMPLE_RATE:
      if (paramValue < 0) return DE_PARAM_OUT_OF_RANGE;
      hGainDec->deltaTminDefault = getDeltaTmin(paramValue);
      break;
    default:
      return DE_PARAM_INVALID;
  }
  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_SetCodecDependentParameters(
    HANDLE_DRC_GAIN_DECODER hGainDec, const DELAY_MODE delayMode,
    const int timeDomainSupported,
    const SUBBAND_DOMAIN_MODE subbandDomainSupported) {
  if ((delayMode != DM_REGULAR_DELAY) && (delayMode != DM_LOW_DELAY)) {
    return DE_NOT_OK;
  }
  hGainDec->delayMode = delayMode;
  hGainDec->timeDomainSupported = timeDomainSupported;
  hGainDec->subbandDomainSupported = subbandDomainSupported;

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Config(HANDLE_DRC_GAIN_DECODER hGainDec,
                          HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                          const UCHAR numSelectedDrcSets,
                          const SCHAR* selectedDrcSetIds,
                          const UCHAR* selectedDownmixIds) {
  DRC_ERROR err = DE_OK;
  int a;

  hGainDec->nActiveDrcs = 0;
  hGainDec->multiBandActiveDrcIndex = -1;
  hGainDec->channelGainActiveDrcIndex = -1;
  for (a = 0; a < numSelectedDrcSets; a++) {
    err = initActiveDrc(hGainDec, hUniDrcConfig, selectedDrcSetIds[a],
                        selectedDownmixIds[a]);
    if (err) return err;
  }

  err = initActiveDrcOffset(hGainDec);
  if (err) return err;

  return err;
}

DRC_ERROR
drcDec_GainDecoder_Close(HANDLE_DRC_GAIN_DECODER* phGainDec) {
  if (*phGainDec != NULL) {
    FDKfree(*phGainDec);
    *phGainDec = NULL;
  }

  return DE_OK;
}

DRC_ERROR
drcDec_GainDecoder_Preprocess(HANDLE_DRC_GAIN_DECODER hGainDec,
                              HANDLE_UNI_DRC_GAIN hUniDrcGain,
                              const FIXP_DBL loudnessNormalizationGainDb,
                              const FIXP_SGL boost, const FIXP_SGL compress) {
  DRC_ERROR err = DE_OK;
  int a, c;

  /* lnbPointer is the index on the most recent node buffer */
  hGainDec->drcGainBuffers.lnbPointer++;
  if (hGainDec->drcGainBuffers.lnbPointer >= NUM_LNB_FRAMES)
    hGainDec->drcGainBuffers.lnbPointer = 0;

  for (a = 0; a < hGainDec->nActiveDrcs; a++) {
    /* prepare gain interpolation of sequences used by copying and modifying
     * nodes in node buffers */
    err = prepareDrcGain(hGainDec, hUniDrcGain, compress, boost,
                         loudnessNormalizationGainDb, a);
    if (err) return err;
  }

  for (a = 0; a < MAX_ACTIVE_DRCS; a++) {
    for (c = 0; c < 8; c++) {
      hGainDec->activeDrc[a]
          .lnbIndexForChannel[c][hGainDec->drcGainBuffers.lnbPointer] =
          -1; /* "no DRC processing" */
    }
    hGainDec->activeDrc[a].subbandGainsReady = 0;
  }

  for (c = 0; c < 8; c++) {
    hGainDec->drcGainBuffers
        .channelGain[c][hGainDec->drcGainBuffers.lnbPointer] =
        FL2FXCONST_DBL(1.0f / (float)(1 << 8));
  }

  return err;
}

/* create gain sequence out of gain sequences of last frame for concealment and
 * flushing */
DRC_ERROR
drcDec_GainDecoder_Conceal(HANDLE_DRC_GAIN_DECODER hGainDec,
                           HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                           HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  int seq, gainSequenceCount;
  DRC_COEFFICIENTS_UNI_DRC* pCoef =
      selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);
  if (pCoef && pCoef->gainSequenceCount) {
    gainSequenceCount = fMin(pCoef->gainSequenceCount, (UCHAR)12);
  } else {
    gainSequenceCount = 1;
  }

  for (seq = 0; seq < gainSequenceCount; seq++) {
    int lastNodeIndex = 0;
    FIXP_SGL lastGainDb = (FIXP_SGL)0;

    lastNodeIndex = hUniDrcGain->nNodes[seq] - 1;
    if ((lastNodeIndex >= 0) && (lastNodeIndex < 16)) {
      lastGainDb = hUniDrcGain->gainNode[seq][lastNodeIndex].gainDb;
    }

    hUniDrcGain->nNodes[seq] = 1;
    if (lastGainDb > (FIXP_SGL)0) {
      hUniDrcGain->gainNode[seq][0].gainDb =
          FX_DBL2FX_SGL(fMult(FL2FXCONST_SGL(0.9f), lastGainDb));
    } else {
      hUniDrcGain->gainNode[seq][0].gainDb =
          FX_DBL2FX_SGL(fMult(FL2FXCONST_SGL(0.98f), lastGainDb));
    }
    hUniDrcGain->gainNode[seq][0].time = hGainDec->frameSize - 1;
  }
  return DE_OK;
}

void drcDec_GainDecoder_SetChannelGains(HANDLE_DRC_GAIN_DECODER hGainDec,
                                        const int numChannels,
                                        const int frameSize,
                                        const FIXP_DBL* channelGainDb,
                                        const int audioBufferChannelOffset,
                                        FIXP_DBL* audioBuffer) {
  int c, i;

  if (hGainDec->channelGainActiveDrcIndex >= 0) {
    /* channel gains will be applied in drcDec_GainDecoder_ProcessTimeDomain or
     * drcDec_GainDecoder_ProcessSubbandDomain, respectively. */
    _setChannelGains(hGainDec, numChannels, channelGainDb);

    if (!hGainDec->status) { /* overwrite previous channel gains at startup */
      DRC_GAIN_BUFFERS* pDrcGainBuffers = &hGainDec->drcGainBuffers;
      for (c = 0; c < numChannels; c++) {
        for (i = 0; i < NUM_LNB_FRAMES; i++) {
          pDrcGainBuffers->channelGain[c][i] = hGainDec->channelGain[c];
        }
      }
      hGainDec->status = 1;
    }
  } else {
    /* smooth and apply channel gains */
    FIXP_DBL prevChannelGain[8];
    for (c = 0; c < numChannels; c++) {
      prevChannelGain[c] = hGainDec->channelGain[c];
    }

    _setChannelGains(hGainDec, numChannels, channelGainDb);

    if (!hGainDec->status) { /* overwrite previous channel gains at startup */
      for (c = 0; c < numChannels; c++)
        prevChannelGain[c] = hGainDec->channelGain[c];
      hGainDec->status = 1;
    }

    for (c = 0; c < numChannels; c++) {
      INT n_min = fMin(fMin(CntLeadingZeros(prevChannelGain[c]),
                            CntLeadingZeros(hGainDec->channelGain[c])) -
                           1,
                       9);
      FIXP_DBL gain = prevChannelGain[c] << n_min;
      FIXP_DBL stepsize = ((hGainDec->channelGain[c] << n_min) - gain);
      if (stepsize != (FIXP_DBL)0) {
        if (frameSize == 1024)
          stepsize = stepsize >> 10;
        else
          stepsize = (LONG)stepsize / frameSize;
      }
      n_min = 9 - n_min;
#ifdef FUNCTION_drcDec_GainDecoder_SetChannelGains_func1
      drcDec_GainDecoder_SetChannelGains_func1(audioBuffer, gain, stepsize,
                                               n_min, frameSize);
#else
      for (i = 0; i < frameSize; i++) {
        audioBuffer[i] = fMultDiv2(audioBuffer[i], gain) << n_min;
        gain += stepsize;
      }
#endif
      audioBuffer += audioBufferChannelOffset;
    }
  }
}

DRC_ERROR
drcDec_GainDecoder_ProcessTimeDomain(
    HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples,
    const GAIN_DEC_LOCATION drcLocation, const int channelOffset,
    const int drcChannelOffset, const int numChannelsProcessed,
    const int timeDataChannelOffset, FIXP_DBL* audioIOBuffer) {
  DRC_ERROR err = DE_OK;
  int a;

  if (!hGainDec->timeDomainSupported) {
    return DE_NOT_OK;
  }

  for (a = 0; a < hGainDec->nActiveDrcs; a++) {
    if (!_fitsLocation(hGainDec->activeDrc[a].pInst, drcLocation)) continue;

    /* Apply DRC */
    err = processDrcTime(hGainDec, a, delaySamples, channelOffset,
                         drcChannelOffset, numChannelsProcessed,
                         timeDataChannelOffset, audioIOBuffer);
    if (err) return err;
  }

  return err;
}

DRC_ERROR
drcDec_GainDecoder_ProcessSubbandDomain(
    HANDLE_DRC_GAIN_DECODER hGainDec, const int delaySamples,
    const GAIN_DEC_LOCATION drcLocation, const int channelOffset,
    const int drcChannelOffset, const int numChannelsProcessed,
    const int processSingleTimeslot, FIXP_DBL* audioIOBufferReal[],
    FIXP_DBL* audioIOBufferImag[]) {
  DRC_ERROR err = DE_OK;
  int a;

  if (hGainDec->subbandDomainSupported == SDM_OFF) {
    return DE_NOT_OK;
  }

  for (a = 0; a < hGainDec->nActiveDrcs; a++) {
    if (!_fitsLocation(hGainDec->activeDrc[a].pInst, drcLocation)) continue;

    /* Apply DRC */
    err = processDrcSubband(hGainDec, a, delaySamples, channelOffset,
                            drcChannelOffset, numChannelsProcessed,
                            processSingleTimeslot, audioIOBufferReal,
                            audioIOBufferImag);
    if (err) return err;
  }

  return err;
}

DRC_ERROR
drcDec_GainDecoder_SetLoudnessNormalizationGainDb(
    HANDLE_DRC_GAIN_DECODER hGainDec, FIXP_DBL loudnessNormalizationGainDb) {
  hGainDec->loudnessNormalisationGainDb = loudnessNormalizationGainDb;

  return DE_OK;
}

int drcDec_GainDecoder_GetFrameSize(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return -1;

  return hGainDec->frameSize;
}

int drcDec_GainDecoder_GetDeltaTminDefault(HANDLE_DRC_GAIN_DECODER hGainDec) {
  if (hGainDec == NULL) return -1;

  return hGainDec->deltaTminDefault;
}
