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

/**************************** AAC encoder library ******************************

   Author(s):   M.Werner

   Description: Psychoaccoustic major function block

*******************************************************************************/

#include "psy_const.h"

#include "block_switch.h"
#include "transform.h"
#include "spreading.h"
#include "pre_echo_control.h"
#include "band_nrg.h"
#include "psy_configuration.h"
#include "psy_data.h"
#include "ms_stereo.h"
#include "interface.h"
#include "psy_main.h"
#include "grp_data.h"
#include "tns_func.h"
#include "pns_func.h"
#include "tonality.h"
#include "aacEnc_ram.h"
#include "intensity.h"

/* blending to reduce gibbs artifacts */
#define FADE_OUT_LEN 6
static const FIXP_DBL fadeOutFactor[FADE_OUT_LEN] = {
    1840644096, 1533870080, 1227096064, 920322048, 613548032, 306774016};

/* forward definitions */

/*****************************************************************************

    functionname: FDKaacEnc_PsyNew
    description:  allocates memory for psychoacoustic
    returns:      an error code
    input:        pointer to a psych handle

*****************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_PsyNew(PSY_INTERNAL **phpsy, const INT nElements,
                                   const INT nChannels, UCHAR *dynamic_RAM) {
  AAC_ENCODER_ERROR ErrorStatus;
  PSY_INTERNAL *hPsy;
  INT i;

  hPsy = GetRam_aacEnc_PsyInternal();
  *phpsy = hPsy;
  if (hPsy == NULL) {
    ErrorStatus = AAC_ENC_NO_MEMORY;
    goto bail;
  }

  for (i = 0; i < nElements; i++) {
    /* PSY_ELEMENT */
    hPsy->psyElement[i] = GetRam_aacEnc_PsyElement(i);
    if (hPsy->psyElement[i] == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto bail;
    }
  }

  for (i = 0; i < nChannels; i++) {
    /* PSY_STATIC */
    hPsy->pStaticChannels[i] = GetRam_aacEnc_PsyStatic(i);
    if (hPsy->pStaticChannels[i] == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto bail;
    }
    /* AUDIO INPUT BUFFER */
    hPsy->pStaticChannels[i]->psyInputBuffer = GetRam_aacEnc_PsyInputBuffer(i);
    if (hPsy->pStaticChannels[i]->psyInputBuffer == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto bail;
    }
  }

  /* reusable psych memory */
  hPsy->psyDynamic = GetRam_aacEnc_PsyDynamic(0, dynamic_RAM);

  return AAC_ENC_OK;

bail:
  FDKaacEnc_PsyClose(phpsy, NULL);

  return ErrorStatus;
}

/*****************************************************************************

    functionname: FDKaacEnc_PsyOutNew
    description:  allocates memory for psyOut struc
    returns:      an error code
    input:        pointer to a psych handle

*****************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_PsyOutNew(PSY_OUT **phpsyOut, const INT nElements,
                                      const INT nChannels, const INT nSubFrames,
                                      UCHAR *dynamic_RAM) {
  AAC_ENCODER_ERROR ErrorStatus;
  int n, i;
  int elInc = 0, chInc = 0;

  for (n = 0; n < nSubFrames; n++) {
    phpsyOut[n] = GetRam_aacEnc_PsyOut(n);

    if (phpsyOut[n] == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto bail;
    }

    for (i = 0; i < nChannels; i++) {
      phpsyOut[n]->pPsyOutChannels[i] = GetRam_aacEnc_PsyOutChannel(chInc++);
      if (NULL == phpsyOut[n]->pPsyOutChannels[i]) {
        ErrorStatus = AAC_ENC_NO_MEMORY;
        goto bail;
      }
    }

    for (i = 0; i < nElements; i++) {
      phpsyOut[n]->psyOutElement[i] = GetRam_aacEnc_PsyOutElements(elInc++);
      if (phpsyOut[n]->psyOutElement[i] == NULL) {
        ErrorStatus = AAC_ENC_NO_MEMORY;
        goto bail;
      }
    }
  } /* nSubFrames */

  return AAC_ENC_OK;

bail:
  FDKaacEnc_PsyClose(NULL, phpsyOut);
  return ErrorStatus;
}

AAC_ENCODER_ERROR FDKaacEnc_psyInitStates(PSY_INTERNAL *hPsy,
                                          PSY_STATIC *psyStatic,
                                          AUDIO_OBJECT_TYPE audioObjectType) {
  /* init input buffer */
  FDKmemclear(psyStatic->psyInputBuffer,
              MAX_INPUT_BUFFER_SIZE * sizeof(INT_PCM));

  FDKaacEnc_InitBlockSwitching(&psyStatic->blockSwitchingControl,
                               isLowDelay(audioObjectType));

  return AAC_ENC_OK;
}

AAC_ENCODER_ERROR FDKaacEnc_psyInit(PSY_INTERNAL *hPsy, PSY_OUT **phpsyOut,
                                    const INT nSubFrames,
                                    const INT nMaxChannels,
                                    const AUDIO_OBJECT_TYPE audioObjectType,
                                    CHANNEL_MAPPING *cm) {
  AAC_ENCODER_ERROR ErrorStatus = AAC_ENC_OK;
  int i, ch, n, chInc = 0, resetChannels = 3;

  if ((nMaxChannels > 2) && (cm->nChannels == 2)) {
    chInc = 1;
    FDKaacEnc_psyInitStates(hPsy, hPsy->pStaticChannels[0], audioObjectType);
  }

  if ((nMaxChannels == 2)) {
    resetChannels = 0;
  }

  for (i = 0; i < cm->nElements; i++) {
    for (ch = 0; ch < cm->elInfo[i].nChannelsInEl; ch++) {
      hPsy->psyElement[i]->psyStatic[ch] = hPsy->pStaticChannels[chInc];
      if (cm->elInfo[i].elType != ID_LFE) {
        if (chInc >= resetChannels) {
          FDKaacEnc_psyInitStates(hPsy, hPsy->psyElement[i]->psyStatic[ch],
                                  audioObjectType);
        }
        mdct_init(&(hPsy->psyElement[i]->psyStatic[ch]->mdctPers), NULL, 0);
        hPsy->psyElement[i]->psyStatic[ch]->isLFE = 0;
      } else {
        hPsy->psyElement[i]->psyStatic[ch]->isLFE = 1;
      }
      chInc++;
    }
  }

  for (n = 0; n < nSubFrames; n++) {
    chInc = 0;
    for (i = 0; i < cm->nElements; i++) {
      for (ch = 0; ch < cm->elInfo[i].nChannelsInEl; ch++) {
        phpsyOut[n]->psyOutElement[i]->psyOutChannel[ch] =
            phpsyOut[n]->pPsyOutChannels[chInc++];
      }
    }
  }

  return ErrorStatus;
}

/*****************************************************************************

    functionname: FDKaacEnc_psyMainInit
    description:  initializes psychoacoustic
    returns:      an error code

*****************************************************************************/

AAC_ENCODER_ERROR FDKaacEnc_psyMainInit(
    PSY_INTERNAL *hPsy, AUDIO_OBJECT_TYPE audioObjectType, CHANNEL_MAPPING *cm,
    INT sampleRate, INT granuleLength, INT bitRate, INT tnsMask, INT bandwidth,
    INT usePns, INT useIS, INT useMS, UINT syntaxFlags, ULONG initFlags) {
  AAC_ENCODER_ERROR ErrorStatus;
  int i, ch;
  int channelsEff = cm->nChannelsEff;
  int tnsChannels = 0;
  FB_TYPE filterBank;

  switch (FDKaacEnc_GetMonoStereoMode(cm->encMode)) {
    /* ... and map to tnsChannels */
    case EL_MODE_MONO:
      tnsChannels = 1;
      break;
    case EL_MODE_STEREO:
      tnsChannels = 2;
      break;
    default:
      tnsChannels = 0;
  }

  switch (audioObjectType) {
    default:
      filterBank = FB_LC;
      break;
    case AOT_ER_AAC_LD:
      filterBank = FB_LD;
      break;
    case AOT_ER_AAC_ELD:
      filterBank = FB_ELD;
      break;
  }

  hPsy->granuleLength = granuleLength;

  ErrorStatus = FDKaacEnc_InitPsyConfiguration(
      bitRate / channelsEff, sampleRate, bandwidth, LONG_WINDOW,
      hPsy->granuleLength, useIS, useMS, &(hPsy->psyConf[0]), filterBank);
  if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

  ErrorStatus = FDKaacEnc_InitTnsConfiguration(
      (bitRate * tnsChannels) / channelsEff, sampleRate, tnsChannels,
      LONG_WINDOW, hPsy->granuleLength, isLowDelay(audioObjectType),
      (syntaxFlags & AC_SBR_PRESENT) ? 1 : 0, &(hPsy->psyConf[0].tnsConf),
      &hPsy->psyConf[0], (INT)(tnsMask & 2), (INT)(tnsMask & 8));

  if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

  if (granuleLength > 512) {
    ErrorStatus = FDKaacEnc_InitPsyConfiguration(
        bitRate / channelsEff, sampleRate, bandwidth, SHORT_WINDOW,
        hPsy->granuleLength, useIS, useMS, &hPsy->psyConf[1], filterBank);

    if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

    ErrorStatus = FDKaacEnc_InitTnsConfiguration(
        (bitRate * tnsChannels) / channelsEff, sampleRate, tnsChannels,
        SHORT_WINDOW, hPsy->granuleLength, isLowDelay(audioObjectType),
        (syntaxFlags & AC_SBR_PRESENT) ? 1 : 0, &hPsy->psyConf[1].tnsConf,
        &hPsy->psyConf[1], (INT)(tnsMask & 1), (INT)(tnsMask & 4));

    if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;
  }

  for (i = 0; i < cm->nElements; i++) {
    for (ch = 0; ch < cm->elInfo[i].nChannelsInEl; ch++) {
      if (initFlags) {
        /* reset states */
        FDKaacEnc_psyInitStates(hPsy, hPsy->psyElement[i]->psyStatic[ch],
                                audioObjectType);
      }

      FDKaacEnc_InitPreEchoControl(
          hPsy->psyElement[i]->psyStatic[ch]->sfbThresholdnm1,
          &hPsy->psyElement[i]->psyStatic[ch]->calcPreEcho,
          hPsy->psyConf[0].sfbCnt, hPsy->psyConf[0].sfbPcmQuantThreshold,
          &hPsy->psyElement[i]->psyStatic[ch]->mdctScalenm1);
    }
  }

  ErrorStatus = FDKaacEnc_InitPnsConfiguration(
      &hPsy->psyConf[0].pnsConf, bitRate / channelsEff, sampleRate, usePns,
      hPsy->psyConf[0].sfbCnt, hPsy->psyConf[0].sfbOffset,
      cm->elInfo[0].nChannelsInEl, (hPsy->psyConf[0].filterbank == FB_LC));
  if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

  if (granuleLength > 512) {
    ErrorStatus = FDKaacEnc_InitPnsConfiguration(
        &hPsy->psyConf[1].pnsConf, bitRate / channelsEff, sampleRate, usePns,
        hPsy->psyConf[1].sfbCnt, hPsy->psyConf[1].sfbOffset,
        cm->elInfo[1].nChannelsInEl, (hPsy->psyConf[1].filterbank == FB_LC));
    if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;
  }

  return ErrorStatus;
}

/*****************************************************************************

    functionname: FDKaacEnc_psyMain
    description:  psychoacoustic
    returns:      an error code

        This function assumes that enough input data is in the modulo buffer.

*****************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_psyMain(INT channels, PSY_ELEMENT *psyElement,
                                    PSY_DYNAMIC *psyDynamic,
                                    PSY_CONFIGURATION *psyConf,
                                    PSY_OUT_ELEMENT *RESTRICT psyOutElement,
                                    INT_PCM *pInput, const UINT inputBufSize,
                                    INT *chIdx, INT totalChannels) {
  const INT commonWindow = 1;
  INT maxSfbPerGroup[(2)];
  INT mdctSpectrum_e;
  INT ch;   /* counts through channels          */
  INT w;    /* counts through windows           */
  INT sfb;  /* counts through scalefactor bands */
  INT line; /* counts through lines             */

  PSY_CONFIGURATION *RESTRICT hPsyConfLong = &psyConf[0];
  PSY_CONFIGURATION *RESTRICT hPsyConfShort = &psyConf[1];
  PSY_OUT_CHANNEL **RESTRICT psyOutChannel = psyOutElement->psyOutChannel;
  FIXP_SGL sfbTonality[(2)][MAX_SFB_LONG];

  PSY_STATIC **RESTRICT psyStatic = psyElement->psyStatic;

  PSY_DATA *RESTRICT psyData[(2)];
  TNS_DATA *RESTRICT tnsData[(2)];
  PNS_DATA *RESTRICT pnsData[(2)];

  INT zeroSpec = TRUE; /* means all spectral lines are zero */

  INT blockSwitchingOffset;

  PSY_CONFIGURATION *RESTRICT hThisPsyConf[(2)];
  INT windowLength[(2)];
  INT nWindows[(2)];
  INT wOffset;

  INT maxSfb[(2)];
  INT *pSfbMaxScaleSpec[(2)];
  FIXP_DBL *pSfbEnergy[(2)];
  FIXP_DBL *pSfbSpreadEnergy[(2)];
  FIXP_DBL *pSfbEnergyLdData[(2)];
  FIXP_DBL *pSfbEnergyMS[(2)];
  FIXP_DBL *pSfbThreshold[(2)];

  INT isShortWindow[(2)];

  /* number of incoming time samples to be processed */
  const INT nTimeSamples = psyConf->granuleLength;

  switch (hPsyConfLong->filterbank) {
    case FB_LC:
      blockSwitchingOffset =
          nTimeSamples + (9 * nTimeSamples / (2 * TRANS_FAC));
      break;
    case FB_LD:
    case FB_ELD:
      blockSwitchingOffset = nTimeSamples;
      break;
    default:
      return AAC_ENC_UNSUPPORTED_FILTERBANK;
  }

  for (ch = 0; ch < channels; ch++) {
    psyData[ch] = &psyDynamic->psyData[ch];
    tnsData[ch] = &psyDynamic->tnsData[ch];
    pnsData[ch] = &psyDynamic->pnsData[ch];

    psyData[ch]->mdctSpectrum = psyOutChannel[ch]->mdctSpectrum;
  }

  /* block switching */
  if (hPsyConfLong->filterbank != FB_ELD) {
    int err;

    for (ch = 0; ch < channels; ch++) {
      C_ALLOC_SCRATCH_START(pTimeSignal, INT_PCM, (1024))

      /* copy input data and use for block switching */
      FDKmemcpy(pTimeSignal, pInput + chIdx[ch] * inputBufSize,
                nTimeSamples * sizeof(INT_PCM));

      FDKaacEnc_BlockSwitching(&psyStatic[ch]->blockSwitchingControl,
                               nTimeSamples, psyStatic[ch]->isLFE, pTimeSignal);

      /* fill up internal input buffer, to 2xframelength samples */
      FDKmemcpy(psyStatic[ch]->psyInputBuffer + blockSwitchingOffset,
                pTimeSignal,
                (2 * nTimeSamples - blockSwitchingOffset) * sizeof(INT_PCM));

      C_ALLOC_SCRATCH_END(pTimeSignal, INT_PCM, (1024))
    }

    /* synch left and right block type */
    err = FDKaacEnc_SyncBlockSwitching(
        &psyStatic[0]->blockSwitchingControl,
        (channels > 1) ? &psyStatic[1]->blockSwitchingControl : NULL, channels,
        commonWindow);

    if (err) {
      return AAC_ENC_UNSUPPORTED_AOT; /* mixed up LC and LD */
    }

  } else {
    for (ch = 0; ch < channels; ch++) {
      /* copy input data and use for block switching */
      FDKmemcpy(psyStatic[ch]->psyInputBuffer + blockSwitchingOffset,
                pInput + chIdx[ch] * inputBufSize,
                nTimeSamples * sizeof(INT_PCM));
    }
  }

  for (ch = 0; ch < channels; ch++)
    isShortWindow[ch] =
        (psyStatic[ch]->blockSwitchingControl.lastWindowSequence ==
         SHORT_WINDOW);

  /* set parameters according to window length */
  for (ch = 0; ch < channels; ch++) {
    if (isShortWindow[ch]) {
      hThisPsyConf[ch] = hPsyConfShort;
      windowLength[ch] = psyConf->granuleLength / TRANS_FAC;
      nWindows[ch] = TRANS_FAC;
      maxSfb[ch] = MAX_SFB_SHORT;

      pSfbMaxScaleSpec[ch] = psyData[ch]->sfbMaxScaleSpec.Short[0];
      pSfbEnergy[ch] = psyData[ch]->sfbEnergy.Short[0];
      pSfbSpreadEnergy[ch] = psyData[ch]->sfbSpreadEnergy.Short[0];
      pSfbEnergyLdData[ch] = psyData[ch]->sfbEnergyLdData.Short[0];
      pSfbEnergyMS[ch] = psyData[ch]->sfbEnergyMS.Short[0];
      pSfbThreshold[ch] = psyData[ch]->sfbThreshold.Short[0];

    } else {
      hThisPsyConf[ch] = hPsyConfLong;
      windowLength[ch] = psyConf->granuleLength;
      nWindows[ch] = 1;
      maxSfb[ch] = MAX_GROUPED_SFB;

      pSfbMaxScaleSpec[ch] = psyData[ch]->sfbMaxScaleSpec.Long;
      pSfbEnergy[ch] = psyData[ch]->sfbEnergy.Long;
      pSfbSpreadEnergy[ch] = psyData[ch]->sfbSpreadEnergy.Long;
      pSfbEnergyLdData[ch] = psyData[ch]->sfbEnergyLdData.Long;
      pSfbEnergyMS[ch] = psyData[ch]->sfbEnergyMS.Long;
      pSfbThreshold[ch] = psyData[ch]->sfbThreshold.Long;
    }
  }

  /* Transform and get mdctScaling for all channels and windows. */
  for (ch = 0; ch < channels; ch++) {
    /* update number of active bands */
    if (psyStatic[ch]->isLFE) {
      psyData[ch]->sfbActive = hThisPsyConf[ch]->sfbActiveLFE;
      psyData[ch]->lowpassLine = hThisPsyConf[ch]->lowpassLineLFE;
    } else {
      psyData[ch]->sfbActive = hThisPsyConf[ch]->sfbActive;
      psyData[ch]->lowpassLine = hThisPsyConf[ch]->lowpassLine;
    }

    if (hThisPsyConf[ch]->filterbank == FB_ELD) {
      if (FDKaacEnc_Transform_Real_Eld(
              psyStatic[ch]->psyInputBuffer, psyData[ch]->mdctSpectrum,
              psyStatic[ch]->blockSwitchingControl.lastWindowSequence,
              psyStatic[ch]->blockSwitchingControl.windowShape,
              &psyStatic[ch]->blockSwitchingControl.lastWindowShape,
              nTimeSamples, &mdctSpectrum_e, hThisPsyConf[ch]->filterbank,
              psyStatic[ch]->overlapAddBuffer) != 0) {
        return AAC_ENC_UNSUPPORTED_FILTERBANK;
      }
    } else {
      if (FDKaacEnc_Transform_Real(
              psyStatic[ch]->psyInputBuffer, psyData[ch]->mdctSpectrum,
              psyStatic[ch]->blockSwitchingControl.lastWindowSequence,
              psyStatic[ch]->blockSwitchingControl.windowShape,
              &psyStatic[ch]->blockSwitchingControl.lastWindowShape,
              &psyStatic[ch]->mdctPers, nTimeSamples, &mdctSpectrum_e,
              hThisPsyConf[ch]->filterbank) != 0) {
        return AAC_ENC_UNSUPPORTED_FILTERBANK;
      }
    }

    for (w = 0; w < nWindows[ch]; w++) {
      wOffset = w * windowLength[ch];

      /* Low pass / highest sfb */
      FDKmemclear(
          &psyData[ch]->mdctSpectrum[psyData[ch]->lowpassLine + wOffset],
          (windowLength[ch] - psyData[ch]->lowpassLine) * sizeof(FIXP_DBL));

      if ((hPsyConfLong->filterbank != FB_LC) &&
          (psyData[ch]->lowpassLine >= FADE_OUT_LEN)) {
        /* Do blending to reduce gibbs artifacts */
        for (int i = 0; i < FADE_OUT_LEN; i++) {
          psyData[ch]->mdctSpectrum[psyData[ch]->lowpassLine + wOffset -
                                    FADE_OUT_LEN + i] =
              fMult(psyData[ch]->mdctSpectrum[psyData[ch]->lowpassLine +
                                              wOffset - FADE_OUT_LEN + i],
                    fadeOutFactor[i]);
        }
      }

      /* Check for zero spectrum. These loops will usually terminate very, very
       * early. */
      for (line = 0; (line < psyData[ch]->lowpassLine) && (zeroSpec == TRUE);
           line++) {
        if (psyData[ch]->mdctSpectrum[line + wOffset] != (FIXP_DBL)0) {
          zeroSpec = FALSE;
          break;
        }
      }

    } /* w loop */

    psyData[ch]->mdctScale = mdctSpectrum_e;

    /* rotate internal time samples */
    FDKmemmove(psyStatic[ch]->psyInputBuffer,
               psyStatic[ch]->psyInputBuffer + nTimeSamples,
               nTimeSamples * sizeof(INT_PCM));

    /* ... and get remaining samples from input buffer */
    FDKmemcpy(psyStatic[ch]->psyInputBuffer + nTimeSamples,
              pInput + (2 * nTimeSamples - blockSwitchingOffset) +
                  chIdx[ch] * inputBufSize,
              (blockSwitchingOffset - nTimeSamples) * sizeof(INT_PCM));

  } /* ch */

  /* Do some rescaling to get maximum possible accuracy for energies */
  if (zeroSpec == FALSE) {
    /* Calc possible spectrum leftshift for each sfb (1 means: 1 bit left shift
     * is possible without overflow) */
    INT minSpecShift = MAX_SHIFT_DBL;
    INT nrgShift = MAX_SHIFT_DBL;
    INT finalShift = MAX_SHIFT_DBL;
    FIXP_DBL currNrg = 0;
    FIXP_DBL maxNrg = 0;

    for (ch = 0; ch < channels; ch++) {
      for (w = 0; w < nWindows[ch]; w++) {
        wOffset = w * windowLength[ch];
        FDKaacEnc_CalcSfbMaxScaleSpec(
            psyData[ch]->mdctSpectrum + wOffset, hThisPsyConf[ch]->sfbOffset,
            pSfbMaxScaleSpec[ch] + w * maxSfb[ch], psyData[ch]->sfbActive);

        for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++)
          minSpecShift = fixMin(minSpecShift,
                                (pSfbMaxScaleSpec[ch] + w * maxSfb[ch])[sfb]);
      }
    }

    /* Calc possible energy leftshift for each sfb (1 means: 1 bit left shift is
     * possible without overflow) */
    for (ch = 0; ch < channels; ch++) {
      for (w = 0; w < nWindows[ch]; w++) {
        wOffset = w * windowLength[ch];
        currNrg = FDKaacEnc_CheckBandEnergyOptim(
            psyData[ch]->mdctSpectrum + wOffset,
            pSfbMaxScaleSpec[ch] + w * maxSfb[ch], hThisPsyConf[ch]->sfbOffset,
            psyData[ch]->sfbActive, pSfbEnergy[ch] + w * maxSfb[ch],
            pSfbEnergyLdData[ch] + w * maxSfb[ch], minSpecShift - 4);

        maxNrg = fixMax(maxNrg, currNrg);
      }
    }

    if (maxNrg != (FIXP_DBL)0) {
      nrgShift = (CountLeadingBits(maxNrg) >> 1) + (minSpecShift - 4);
    }

    /* 2check: Hasn't this decision to be made for both channels? */
    /* For short windows 1 additional bit headroom is necessary to prevent
     * overflows when summing up energies in FDKaacEnc_groupShortData() */
    if (isShortWindow[0]) nrgShift--;

    /* both spectrum and energies mustn't overflow */
    finalShift = fixMin(minSpecShift, nrgShift);

    /* do not shift more than 3 bits more to the left than signal without
     * blockfloating point would be to avoid overflow of scaled PCM quantization
     * thresholds */
    if (finalShift > psyData[0]->mdctScale + 3)
      finalShift = psyData[0]->mdctScale + 3;

    FDK_ASSERT(finalShift >= 0); /* right shift is not allowed */

    /* correct sfbEnergy and sfbEnergyLdData with new finalShift */
    FIXP_DBL ldShift = finalShift * FL2FXCONST_DBL(2.0 / 64);
    for (ch = 0; ch < channels; ch++) {
      INT maxSfb_ch = maxSfb[ch];
      INT w_maxSfb_ch = 0;
      for (w = 0; w < nWindows[ch]; w++) {
        for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
          INT scale = fixMax(0, (pSfbMaxScaleSpec[ch] + w_maxSfb_ch)[sfb] - 4);
          scale = fixMin((scale - finalShift) << 1, DFRACT_BITS - 1);
          if (scale >= 0)
            (pSfbEnergy[ch] + w_maxSfb_ch)[sfb] >>= (scale);
          else
            (pSfbEnergy[ch] + w_maxSfb_ch)[sfb] <<= (-scale);
          (pSfbThreshold[ch] + w_maxSfb_ch)[sfb] =
              fMult((pSfbEnergy[ch] + w_maxSfb_ch)[sfb], C_RATIO);
          (pSfbEnergyLdData[ch] + w_maxSfb_ch)[sfb] += ldShift;
        }
        w_maxSfb_ch += maxSfb_ch;
      }
    }

    if (finalShift != 0) {
      for (ch = 0; ch < channels; ch++) {
        INT wLen = windowLength[ch];
        INT lowpassLine = psyData[ch]->lowpassLine;
        wOffset = 0;
        FIXP_DBL *mdctSpectrum = &psyData[ch]->mdctSpectrum[0];
        for (w = 0; w < nWindows[ch]; w++) {
          FIXP_DBL *spectrum = &mdctSpectrum[wOffset];
          for (line = 0; line < lowpassLine; line++) {
            spectrum[line] <<= finalShift;
          }
          wOffset += wLen;

          /* update sfbMaxScaleSpec */
          for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++)
            (pSfbMaxScaleSpec[ch] + w * maxSfb[ch])[sfb] -= finalShift;
        }
        /* update mdctScale */
        psyData[ch]->mdctScale -= finalShift;
      }
    }

  } else {
    /* all spectral lines are zero */
    for (ch = 0; ch < channels; ch++) {
      psyData[ch]->mdctScale =
          0; /* otherwise mdctScale would be for example 7 and PCM quantization
              * thresholds would be shifted 14 bits to the right causing some of
              * them to become 0 (which causes problems later) */
      /* clear sfbMaxScaleSpec */
      for (w = 0; w < nWindows[ch]; w++) {
        for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
          (pSfbMaxScaleSpec[ch] + w * maxSfb[ch])[sfb] = 0;
          (pSfbEnergy[ch] + w * maxSfb[ch])[sfb] = (FIXP_DBL)0;
          (pSfbEnergyLdData[ch] + w * maxSfb[ch])[sfb] = FL2FXCONST_DBL(-1.0f);
          (pSfbThreshold[ch] + w * maxSfb[ch])[sfb] = (FIXP_DBL)0;
        }
      }
    }
  }

  /* Advance psychoacoustics: Tonality and TNS */
  if ((channels >= 1) && (psyStatic[0]->isLFE)) {
    tnsData[0]->dataRaw.Long.subBlockInfo.tnsActive[HIFILT] = 0;
    tnsData[0]->dataRaw.Long.subBlockInfo.tnsActive[LOFILT] = 0;
  } else {
    for (ch = 0; ch < channels; ch++) {
      if (!isShortWindow[ch]) {
        /* tonality */
        FDKaacEnc_CalculateFullTonality(
            psyData[ch]->mdctSpectrum, pSfbMaxScaleSpec[ch],
            pSfbEnergyLdData[ch], sfbTonality[ch], psyData[ch]->sfbActive,
            hThisPsyConf[ch]->sfbOffset, hThisPsyConf[ch]->pnsConf.usePns);
      }
    } /* ch */

    if (hPsyConfLong->tnsConf.tnsActive || hPsyConfShort->tnsConf.tnsActive) {
      INT tnsActive[TRANS_FAC] = {0};
      INT nrgScaling[2] = {0, 0};
      INT tnsSpecShift = 0;

      for (ch = 0; ch < channels; ch++) {
        for (w = 0; w < nWindows[ch]; w++) {
          wOffset = w * windowLength[ch];
          /* TNS */
          FDKaacEnc_TnsDetect(
              tnsData[ch], &hThisPsyConf[ch]->tnsConf,
              &psyOutChannel[ch]->tnsInfo, hThisPsyConf[ch]->sfbCnt,
              psyData[ch]->mdctSpectrum + wOffset, w,
              psyStatic[ch]->blockSwitchingControl.lastWindowSequence);
        }
      }

      if (channels == 2) {
        FDKaacEnc_TnsSync(
            tnsData[1], tnsData[0], &psyOutChannel[1]->tnsInfo,
            &psyOutChannel[0]->tnsInfo,

            psyStatic[1]->blockSwitchingControl.lastWindowSequence,
            psyStatic[0]->blockSwitchingControl.lastWindowSequence,
            &hThisPsyConf[1]->tnsConf);
      }

      if (channels >= 1) {
        FDK_ASSERT(1 == commonWindow); /* all checks for TNS do only work for
                                          common windows (which is always set)*/
        for (w = 0; w < nWindows[0]; w++) {
          if (isShortWindow[0])
            tnsActive[w] =
                tnsData[0]->dataRaw.Short.subBlockInfo[w].tnsActive[HIFILT] ||
                tnsData[0]->dataRaw.Short.subBlockInfo[w].tnsActive[LOFILT] ||
                tnsData[channels - 1]
                    ->dataRaw.Short.subBlockInfo[w]
                    .tnsActive[HIFILT] ||
                tnsData[channels - 1]
                    ->dataRaw.Short.subBlockInfo[w]
                    .tnsActive[LOFILT];
          else
            tnsActive[w] =
                tnsData[0]->dataRaw.Long.subBlockInfo.tnsActive[HIFILT] ||
                tnsData[0]->dataRaw.Long.subBlockInfo.tnsActive[LOFILT] ||
                tnsData[channels - 1]
                    ->dataRaw.Long.subBlockInfo.tnsActive[HIFILT] ||
                tnsData[channels - 1]
                    ->dataRaw.Long.subBlockInfo.tnsActive[LOFILT];
        }
      }

      for (ch = 0; ch < channels; ch++) {
        if (tnsActive[0] && !isShortWindow[ch]) {
          /* Scale down spectrum if tns is active in one of the two channels
           * with same lastWindowSequence */
          /* first part of threshold calculation; it's not necessary to update
           * sfbMaxScaleSpec */
          INT shift = 1;
          for (sfb = 0; sfb < hThisPsyConf[ch]->lowpassLine; sfb++) {
            psyData[ch]->mdctSpectrum[sfb] =
                psyData[ch]->mdctSpectrum[sfb] >> shift;
          }

          /* update thresholds */
          for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
            pSfbThreshold[ch][sfb] >>= (2 * shift);
          }

          psyData[ch]->mdctScale += shift; /* update mdctScale */

          /* calc sfbEnergies after tnsEncode again ! */
        }
      }

      for (ch = 0; ch < channels; ch++) {
        for (w = 0; w < nWindows[ch]; w++) {
          wOffset = w * windowLength[ch];
          FDKaacEnc_TnsEncode(
              &psyOutChannel[ch]->tnsInfo, tnsData[ch],
              hThisPsyConf[ch]->sfbCnt, &hThisPsyConf[ch]->tnsConf,
              hThisPsyConf[ch]->sfbOffset[psyData[ch]->sfbActive],
              /*hThisPsyConf[ch]->lowpassLine*/ /* filter stops
                                                   before that
                                                   line ! */
                  psyData[ch]->mdctSpectrum +
                  wOffset,
              w, psyStatic[ch]->blockSwitchingControl.lastWindowSequence);

          if (tnsActive[w]) {
            /* Calc sfb-bandwise mdct-energies for left and right channel again,
             */
            /* if tns active in current channel or in one channel with same
             * lastWindowSequence left and right */
            FDKaacEnc_CalcSfbMaxScaleSpec(psyData[ch]->mdctSpectrum + wOffset,
                                          hThisPsyConf[ch]->sfbOffset,
                                          pSfbMaxScaleSpec[ch] + w * maxSfb[ch],
                                          psyData[ch]->sfbActive);
          }
        }
      }

      for (ch = 0; ch < channels; ch++) {
        for (w = 0; w < nWindows[ch]; w++) {
          if (tnsActive[w]) {
            if (isShortWindow[ch]) {
              FDKaacEnc_CalcBandEnergyOptimShort(
                  psyData[ch]->mdctSpectrum + w * windowLength[ch],
                  pSfbMaxScaleSpec[ch] + w * maxSfb[ch],
                  hThisPsyConf[ch]->sfbOffset, psyData[ch]->sfbActive,
                  pSfbEnergy[ch] + w * maxSfb[ch]);
            } else {
              nrgScaling[ch] = /* with tns, energy calculation can overflow; ->
                                  scaling */
                  FDKaacEnc_CalcBandEnergyOptimLong(
                      psyData[ch]->mdctSpectrum, pSfbMaxScaleSpec[ch],
                      hThisPsyConf[ch]->sfbOffset, psyData[ch]->sfbActive,
                      pSfbEnergy[ch], pSfbEnergyLdData[ch]);
              tnsSpecShift =
                  fixMax(tnsSpecShift, nrgScaling[ch]); /* nrgScaling is set
                                                           only if nrg would
                                                           have an overflow */
            }
          } /* if tnsActive */
        }
      } /* end channel loop */

      /* adapt scaling to prevent nrg overflow, only for long blocks */
      for (ch = 0; ch < channels; ch++) {
        if ((tnsSpecShift != 0) && !isShortWindow[ch]) {
          /* scale down spectrum, nrg's and thresholds, if there was an overflow
           * in sfbNrg calculation after tns */
          for (line = 0; line < hThisPsyConf[ch]->lowpassLine; line++) {
            psyData[ch]->mdctSpectrum[line] >>= tnsSpecShift;
          }
          INT scale = (tnsSpecShift - nrgScaling[ch]) << 1;
          for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
            pSfbEnergyLdData[ch][sfb] -=
                scale * FL2FXCONST_DBL(1.0 / LD_DATA_SCALING);
            pSfbEnergy[ch][sfb] >>= scale;
            pSfbThreshold[ch][sfb] >>= (tnsSpecShift << 1);
          }
          psyData[ch]->mdctScale += tnsSpecShift; /* update mdctScale; not
                                                     necessary to update
                                                     sfbMaxScaleSpec */
        }
      } /* end channel loop */

    } /* TNS active */
    else {
      /* In case of disable TNS, reset its dynamic data. Some of its elements is
       * required in PNS detection below. */
      FDKmemclear(psyDynamic->tnsData, sizeof(psyDynamic->tnsData));
    }
  } /* !isLFE */

  /* Advance thresholds */
  for (ch = 0; ch < channels; ch++) {
    INT headroom;

    FIXP_DBL clipEnergy;
    INT energyShift = psyData[ch]->mdctScale * 2;
    INT clipNrgShift = energyShift - THR_SHIFTBITS;
    if (isShortWindow[ch])
      headroom = 6;
    else
      headroom = 0;

    if (clipNrgShift >= 0)
      clipEnergy = hThisPsyConf[ch]->clipEnergy >> clipNrgShift;
    else if (clipNrgShift >= -headroom)
      clipEnergy = hThisPsyConf[ch]->clipEnergy << -clipNrgShift;
    else
      clipEnergy = (FIXP_DBL)MAXVAL_DBL;

    for (w = 0; w < nWindows[ch]; w++) {
      INT i;
      /* limit threshold to avoid clipping */
      for (i = 0; i < psyData[ch]->sfbActive; i++) {
        *(pSfbThreshold[ch] + w * maxSfb[ch] + i) =
            fixMin(*(pSfbThreshold[ch] + w * maxSfb[ch] + i), clipEnergy);
      }

      /* spreading */
      FDKaacEnc_SpreadingMax(psyData[ch]->sfbActive,
                             hThisPsyConf[ch]->sfbMaskLowFactor,
                             hThisPsyConf[ch]->sfbMaskHighFactor,
                             pSfbThreshold[ch] + w * maxSfb[ch]);

      /* PCM quantization threshold */
      energyShift += PCM_QUANT_THR_SCALE;
      if (energyShift >= 0) {
        energyShift = fixMin(DFRACT_BITS - 1, energyShift);
        for (i = 0; i < psyData[ch]->sfbActive; i++) {
          *(pSfbThreshold[ch] + w * maxSfb[ch] + i) = fixMax(
              *(pSfbThreshold[ch] + w * maxSfb[ch] + i) >> THR_SHIFTBITS,
              (hThisPsyConf[ch]->sfbPcmQuantThreshold[i] >> energyShift));
        }
      } else {
        energyShift = fixMin(DFRACT_BITS - 1, -energyShift);
        for (i = 0; i < psyData[ch]->sfbActive; i++) {
          *(pSfbThreshold[ch] + w * maxSfb[ch] + i) = fixMax(
              *(pSfbThreshold[ch] + w * maxSfb[ch] + i) >> THR_SHIFTBITS,
              (hThisPsyConf[ch]->sfbPcmQuantThreshold[i] << energyShift));
        }
      }

      if (!psyStatic[ch]->isLFE) {
        /* preecho control */
        if (psyStatic[ch]->blockSwitchingControl.lastWindowSequence ==
            STOP_WINDOW) {
          /* prevent FDKaacEnc_PreEchoControl from comparing stop
             thresholds with short thresholds */
          for (i = 0; i < psyData[ch]->sfbActive; i++) {
            psyStatic[ch]->sfbThresholdnm1[i] = (FIXP_DBL)MAXVAL_DBL;
          }

          psyStatic[ch]->mdctScalenm1 = 0;
          psyStatic[ch]->calcPreEcho = 0;
        }

        FDKaacEnc_PreEchoControl(
            psyStatic[ch]->sfbThresholdnm1, psyStatic[ch]->calcPreEcho,
            psyData[ch]->sfbActive, hThisPsyConf[ch]->maxAllowedIncreaseFactor,
            hThisPsyConf[ch]->minRemainingThresholdFactor,
            pSfbThreshold[ch] + w * maxSfb[ch], psyData[ch]->mdctScale,
            &psyStatic[ch]->mdctScalenm1);

        psyStatic[ch]->calcPreEcho = 1;

        if (psyStatic[ch]->blockSwitchingControl.lastWindowSequence ==
            START_WINDOW) {
          /* prevent FDKaacEnc_PreEchoControl in next frame to compare start
             thresholds with short thresholds */
          for (i = 0; i < psyData[ch]->sfbActive; i++) {
            psyStatic[ch]->sfbThresholdnm1[i] = (FIXP_DBL)MAXVAL_DBL;
          }

          psyStatic[ch]->mdctScalenm1 = 0;
          psyStatic[ch]->calcPreEcho = 0;
        }
      }

      /* spread energy to avoid hole detection */
      FDKmemcpy(pSfbSpreadEnergy[ch] + w * maxSfb[ch],
                pSfbEnergy[ch] + w * maxSfb[ch],
                psyData[ch]->sfbActive * sizeof(FIXP_DBL));

      FDKaacEnc_SpreadingMax(psyData[ch]->sfbActive,
                             hThisPsyConf[ch]->sfbMaskLowFactorSprEn,
                             hThisPsyConf[ch]->sfbMaskHighFactorSprEn,
                             pSfbSpreadEnergy[ch] + w * maxSfb[ch]);
    }
  }

  /* Calc bandwise energies for mid and side channel. Do it only if 2 channels
   * exist */
  if (channels == 2) {
    for (w = 0; w < nWindows[1]; w++) {
      wOffset = w * windowLength[1];
      FDKaacEnc_CalcBandNrgMSOpt(
          psyData[0]->mdctSpectrum + wOffset,
          psyData[1]->mdctSpectrum + wOffset,
          pSfbMaxScaleSpec[0] + w * maxSfb[0],
          pSfbMaxScaleSpec[1] + w * maxSfb[1], hThisPsyConf[1]->sfbOffset,
          psyData[0]->sfbActive, pSfbEnergyMS[0] + w * maxSfb[0],
          pSfbEnergyMS[1] + w * maxSfb[1],
          (psyStatic[1]->blockSwitchingControl.lastWindowSequence !=
           SHORT_WINDOW),
          psyData[0]->sfbEnergyMSLdData, psyData[1]->sfbEnergyMSLdData);
    }
  }

  /* group short data (maxSfb[ch] for short blocks is determined here) */
  for (ch = 0; ch < channels; ch++) {
    if (isShortWindow[ch]) {
      int sfbGrp;
      int noSfb = psyStatic[ch]->blockSwitchingControl.noOfGroups *
                  hPsyConfShort->sfbCnt;
      /* At this point, energies and thresholds are copied/regrouped from the
       * ".Short" to the ".Long" arrays */
      FDKaacEnc_groupShortData(
          psyData[ch]->mdctSpectrum, &psyData[ch]->sfbThreshold,
          &psyData[ch]->sfbEnergy, &psyData[ch]->sfbEnergyMS,
          &psyData[ch]->sfbSpreadEnergy, hPsyConfShort->sfbCnt,
          psyData[ch]->sfbActive, hPsyConfShort->sfbOffset,
          hPsyConfShort->sfbMinSnrLdData, psyData[ch]->groupedSfbOffset,
          &maxSfbPerGroup[ch], psyOutChannel[ch]->sfbMinSnrLdData,
          psyStatic[ch]->blockSwitchingControl.noOfGroups,
          psyStatic[ch]->blockSwitchingControl.groupLen,
          psyConf[1].granuleLength);

      /* calculate ldData arrays (short values are in .Long-arrays after
       * FDKaacEnc_groupShortData) */
      for (sfbGrp = 0; sfbGrp < noSfb; sfbGrp += hPsyConfShort->sfbCnt) {
        LdDataVector(&psyData[ch]->sfbEnergy.Long[sfbGrp],
                     &psyOutChannel[ch]->sfbEnergyLdData[sfbGrp],
                     psyData[ch]->sfbActive);
      }

      /* calc sfbThrld and set Values smaller 2^-31 to 2^-33*/
      for (sfbGrp = 0; sfbGrp < noSfb; sfbGrp += hPsyConfShort->sfbCnt) {
        LdDataVector(&psyData[ch]->sfbThreshold.Long[sfbGrp],
                     &psyOutChannel[ch]->sfbThresholdLdData[sfbGrp],
                     psyData[ch]->sfbActive);
        for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
          psyOutChannel[ch]->sfbThresholdLdData[sfbGrp + sfb] =
              fixMax(psyOutChannel[ch]->sfbThresholdLdData[sfbGrp + sfb],
                     FL2FXCONST_DBL(-0.515625f));
        }
      }

      if (channels == 2) {
        for (sfbGrp = 0; sfbGrp < noSfb; sfbGrp += hPsyConfShort->sfbCnt) {
          LdDataVector(&psyData[ch]->sfbEnergyMS.Long[sfbGrp],
                       &psyData[ch]->sfbEnergyMSLdData[sfbGrp],
                       psyData[ch]->sfbActive);
        }
      }

      FDKmemcpy(psyOutChannel[ch]->sfbOffsets, psyData[ch]->groupedSfbOffset,
                (MAX_GROUPED_SFB + 1) * sizeof(INT));

    } else {
      int i;
      /* maxSfb[ch] for long blocks */
      for (sfb = psyData[ch]->sfbActive - 1; sfb >= 0; sfb--) {
        for (line = hPsyConfLong->sfbOffset[sfb + 1] - 1;
             line >= hPsyConfLong->sfbOffset[sfb]; line--) {
          if (psyData[ch]->mdctSpectrum[line] != FL2FXCONST_SGL(0.0f)) break;
        }
        if (line > hPsyConfLong->sfbOffset[sfb]) break;
      }
      maxSfbPerGroup[ch] = sfb + 1;
      maxSfbPerGroup[ch] =
          fixMax(fixMin(5, psyData[ch]->sfbActive), maxSfbPerGroup[ch]);

      /* sfbNrgLdData is calculated in FDKaacEnc_advancePsychLong, copy in
       * psyOut structure */
      FDKmemcpy(psyOutChannel[ch]->sfbEnergyLdData,
                psyData[ch]->sfbEnergyLdData.Long,
                psyData[ch]->sfbActive * sizeof(FIXP_DBL));

      FDKmemcpy(psyOutChannel[ch]->sfbOffsets, hPsyConfLong->sfbOffset,
                (MAX_GROUPED_SFB + 1) * sizeof(INT));

      /* sfbMinSnrLdData modified in adjust threshold, copy necessary */
      FDKmemcpy(psyOutChannel[ch]->sfbMinSnrLdData,
                hPsyConfLong->sfbMinSnrLdData,
                psyData[ch]->sfbActive * sizeof(FIXP_DBL));

      /* sfbEnergyMSLdData ist already calculated in FDKaacEnc_CalcBandNrgMSOpt;
       * only in long case */

      /* calc sfbThrld and set Values smaller 2^-31 to 2^-33*/
      LdDataVector(psyData[ch]->sfbThreshold.Long,
                   psyOutChannel[ch]->sfbThresholdLdData,
                   psyData[ch]->sfbActive);
      for (i = 0; i < psyData[ch]->sfbActive; i++) {
        psyOutChannel[ch]->sfbThresholdLdData[i] =
            fixMax(psyOutChannel[ch]->sfbThresholdLdData[i],
                   FL2FXCONST_DBL(-0.515625f));
      }
    }
  }

  /*
      Intensity parameter intialization.
   */
  for (ch = 0; ch < channels; ch++) {
    FDKmemclear(psyOutChannel[ch]->isBook, MAX_GROUPED_SFB * sizeof(INT));
    FDKmemclear(psyOutChannel[ch]->isScale, MAX_GROUPED_SFB * sizeof(INT));
  }

  for (ch = 0; ch < channels; ch++) {
    INT win = (isShortWindow[ch] ? 1 : 0);
    if (!psyStatic[ch]->isLFE) {
      /* PNS Decision */
      FDKaacEnc_PnsDetect(
          &(psyConf[0].pnsConf), pnsData[ch],
          psyStatic[ch]->blockSwitchingControl.lastWindowSequence,
          psyData[ch]->sfbActive,
          maxSfbPerGroup[ch], /* count of Sfb which are not zero. */
          psyOutChannel[ch]->sfbThresholdLdData, psyConf[win].sfbOffset,
          psyData[ch]->mdctSpectrum, psyData[ch]->sfbMaxScaleSpec.Long,
          sfbTonality[ch], psyOutChannel[ch]->tnsInfo.order[0][0],
          tnsData[ch]->dataRaw.Long.subBlockInfo.predictionGain[HIFILT],
          tnsData[ch]->dataRaw.Long.subBlockInfo.tnsActive[HIFILT],
          psyOutChannel[ch]->sfbEnergyLdData, psyOutChannel[ch]->noiseNrg);
    } /* !isLFE */
  }   /* ch */

  /*
      stereo Processing
  */
  if (channels == 2) {
    psyOutElement->toolsInfo.msDigest = MS_NONE;
    psyOutElement->commonWindow = commonWindow;
    if (psyOutElement->commonWindow)
      maxSfbPerGroup[0] = maxSfbPerGroup[1] =
          fixMax(maxSfbPerGroup[0], maxSfbPerGroup[1]);
    if (psyStatic[0]->blockSwitchingControl.lastWindowSequence !=
        SHORT_WINDOW) {
      /* PNS preprocessing depending on ms processing: PNS not in Short Window!
       */
      FDKaacEnc_PreProcessPnsChannelPair(
          psyData[0]->sfbActive, (&psyData[0]->sfbEnergy)->Long,
          (&psyData[1]->sfbEnergy)->Long, psyOutChannel[0]->sfbEnergyLdData,
          psyOutChannel[1]->sfbEnergyLdData, psyData[0]->sfbEnergyMS.Long,
          &(psyConf[0].pnsConf), pnsData[0], pnsData[1]);

      FDKaacEnc_IntensityStereoProcessing(
          psyData[0]->sfbEnergy.Long, psyData[1]->sfbEnergy.Long,
          psyData[0]->mdctSpectrum, psyData[1]->mdctSpectrum,
          psyData[0]->sfbThreshold.Long, psyData[1]->sfbThreshold.Long,
          psyOutChannel[1]->sfbThresholdLdData,
          psyData[0]->sfbSpreadEnergy.Long, psyData[1]->sfbSpreadEnergy.Long,
          psyOutChannel[0]->sfbEnergyLdData, psyOutChannel[1]->sfbEnergyLdData,
          &psyOutElement->toolsInfo.msDigest, psyOutElement->toolsInfo.msMask,
          psyConf[0].sfbCnt, psyConf[0].sfbCnt, maxSfbPerGroup[0],
          psyConf[0].sfbOffset,
          psyConf[0].allowIS && psyOutElement->commonWindow,
          psyOutChannel[1]->isBook, psyOutChannel[1]->isScale, pnsData);

      FDKaacEnc_MsStereoProcessing(
          psyData, psyOutChannel, psyOutChannel[1]->isBook,
          &psyOutElement->toolsInfo.msDigest, psyOutElement->toolsInfo.msMask,
          psyConf[0].allowMS, psyData[0]->sfbActive, psyData[0]->sfbActive,
          maxSfbPerGroup[0], psyOutChannel[0]->sfbOffsets);

      /* PNS postprocessing */
      FDKaacEnc_PostProcessPnsChannelPair(
          psyData[0]->sfbActive, &(psyConf[0].pnsConf), pnsData[0], pnsData[1],
          psyOutElement->toolsInfo.msMask, &psyOutElement->toolsInfo.msDigest);

    } else {
      FDKaacEnc_IntensityStereoProcessing(
          psyData[0]->sfbEnergy.Long, psyData[1]->sfbEnergy.Long,
          psyData[0]->mdctSpectrum, psyData[1]->mdctSpectrum,
          psyData[0]->sfbThreshold.Long, psyData[1]->sfbThreshold.Long,
          psyOutChannel[1]->sfbThresholdLdData,
          psyData[0]->sfbSpreadEnergy.Long, psyData[1]->sfbSpreadEnergy.Long,
          psyOutChannel[0]->sfbEnergyLdData, psyOutChannel[1]->sfbEnergyLdData,
          &psyOutElement->toolsInfo.msDigest, psyOutElement->toolsInfo.msMask,
          psyStatic[0]->blockSwitchingControl.noOfGroups *
              hPsyConfShort->sfbCnt,
          psyConf[1].sfbCnt, maxSfbPerGroup[0], psyData[0]->groupedSfbOffset,
          psyConf[0].allowIS && psyOutElement->commonWindow,
          psyOutChannel[1]->isBook, psyOutChannel[1]->isScale, pnsData);

      /* it's OK to pass the ".Long" arrays here. They contain grouped short
       * data since FDKaacEnc_groupShortData() */
      FDKaacEnc_MsStereoProcessing(
          psyData, psyOutChannel, psyOutChannel[1]->isBook,
          &psyOutElement->toolsInfo.msDigest, psyOutElement->toolsInfo.msMask,
          psyConf[1].allowMS,
          psyStatic[0]->blockSwitchingControl.noOfGroups *
              hPsyConfShort->sfbCnt,
          hPsyConfShort->sfbCnt, maxSfbPerGroup[0],
          psyOutChannel[0]->sfbOffsets);
    }
  } /* (channels == 2) */

  /*
    PNS Coding
  */
  for (ch = 0; ch < channels; ch++) {
    if (psyStatic[ch]->isLFE) {
      /* no PNS coding */
      for (sfb = 0; sfb < psyData[ch]->sfbActive; sfb++) {
        psyOutChannel[ch]->noiseNrg[sfb] = NO_NOISE_PNS;
      }
    } else {
      FDKaacEnc_CodePnsChannel(
          psyData[ch]->sfbActive, &(hThisPsyConf[ch]->pnsConf),
          pnsData[ch]->pnsFlag, psyData[ch]->sfbEnergyLdData.Long,
          psyOutChannel[ch]->noiseNrg, /* this is the energy that will be
                                          written to the bitstream */
          psyOutChannel[ch]->sfbThresholdLdData);
    }
  }

  /*
      build output
  */
  for (ch = 0; ch < channels; ch++) {
    INT mask;
    int grp;
    psyOutChannel[ch]->maxSfbPerGroup = maxSfbPerGroup[ch];
    psyOutChannel[ch]->mdctScale = psyData[ch]->mdctScale;
    if (isShortWindow[ch] == 0) {
      psyOutChannel[ch]->sfbCnt = hPsyConfLong->sfbActive;
      psyOutChannel[ch]->sfbPerGroup = hPsyConfLong->sfbActive;
      psyOutChannel[ch]->lastWindowSequence =
          psyStatic[ch]->blockSwitchingControl.lastWindowSequence;
      psyOutChannel[ch]->windowShape =
          psyStatic[ch]->blockSwitchingControl.windowShape;
    } else {
      INT sfbCnt = psyStatic[ch]->blockSwitchingControl.noOfGroups *
                   hPsyConfShort->sfbCnt;

      psyOutChannel[ch]->sfbCnt = sfbCnt;
      psyOutChannel[ch]->sfbPerGroup = hPsyConfShort->sfbCnt;
      psyOutChannel[ch]->lastWindowSequence = SHORT_WINDOW;
      psyOutChannel[ch]->windowShape = SINE_WINDOW;
    }
    /* generate grouping mask */
    mask = 0;
    for (grp = 0; grp < psyStatic[ch]->blockSwitchingControl.noOfGroups;
         grp++) {
      int j;
      mask <<= 1;
      for (j = 1; j < psyStatic[ch]->blockSwitchingControl.groupLen[grp]; j++) {
        mask = (mask << 1) | 1;
      }
    }
    psyOutChannel[ch]->groupingMask = mask;

    /* build interface */
    FDKmemcpy(psyOutChannel[ch]->groupLen,
              psyStatic[ch]->blockSwitchingControl.groupLen,
              MAX_NO_OF_GROUPS * sizeof(INT));
    FDKmemcpy(psyOutChannel[ch]->sfbEnergy, (&psyData[ch]->sfbEnergy)->Long,
              MAX_GROUPED_SFB * sizeof(FIXP_DBL));
    FDKmemcpy(psyOutChannel[ch]->sfbSpreadEnergy,
              (&psyData[ch]->sfbSpreadEnergy)->Long,
              MAX_GROUPED_SFB * sizeof(FIXP_DBL));
    //        FDKmemcpy(psyOutChannel[ch]->mdctSpectrum,
    //        psyData[ch]->mdctSpectrum, (1024)*sizeof(FIXP_DBL));
  }

  return AAC_ENC_OK;
}

void FDKaacEnc_PsyClose(PSY_INTERNAL **phPsyInternal, PSY_OUT **phPsyOut) {
  int n, i;

  if (phPsyInternal != NULL) {
    PSY_INTERNAL *hPsyInternal = *phPsyInternal;

    if (hPsyInternal) {
      for (i = 0; i < (8); i++) {
        if (hPsyInternal->pStaticChannels[i]) {
          if (hPsyInternal->pStaticChannels[i]->psyInputBuffer)
            FreeRam_aacEnc_PsyInputBuffer(
                &hPsyInternal->pStaticChannels[i]
                     ->psyInputBuffer); /* AUDIO INPUT BUFFER */

          FreeRam_aacEnc_PsyStatic(
              &hPsyInternal->pStaticChannels[i]); /* PSY_STATIC */
        }
      }

      for (i = 0; i < ((8)); i++) {
        if (hPsyInternal->psyElement[i])
          FreeRam_aacEnc_PsyElement(
              &hPsyInternal->psyElement[i]); /* PSY_ELEMENT */
      }

      FreeRam_aacEnc_PsyInternal(phPsyInternal);
    }
  }

  if (phPsyOut != NULL) {
    for (n = 0; n < (1); n++) {
      if (phPsyOut[n]) {
        for (i = 0; i < (8); i++) {
          if (phPsyOut[n]->pPsyOutChannels[i])
            FreeRam_aacEnc_PsyOutChannel(
                &phPsyOut[n]->pPsyOutChannels[i]); /* PSY_OUT_CHANNEL */
        }

        for (i = 0; i < ((8)); i++) {
          if (phPsyOut[n]->psyOutElement[i])
            FreeRam_aacEnc_PsyOutElements(
                &phPsyOut[n]->psyOutElement[i]); /* PSY_OUT_ELEMENTS */
        }

        FreeRam_aacEnc_PsyOut(&phPsyOut[n]);
      }
    }
  }
}
