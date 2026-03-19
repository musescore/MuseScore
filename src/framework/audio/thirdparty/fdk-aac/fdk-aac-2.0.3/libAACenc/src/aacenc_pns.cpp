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

   Author(s):   M. Lohwasser

   Description: pns.c

*******************************************************************************/

#include "aacenc_pns.h"

#include "psy_data.h"
#include "pnsparam.h"
#include "noisedet.h"
#include "bit_cnt.h"
#include "interface.h"

/* minCorrelationEnergy = (1.0e-10f)^2 ~ 2^-67 = 2^-47 * 2^-20 */
static const FIXP_DBL minCorrelationEnergy =
    FL2FXCONST_DBL(0.0); /* FL2FXCONST_DBL((float)FDKpow(2.0,-47)); */
/* noiseCorrelationThresh = 0.6^2 */
static const FIXP_DBL noiseCorrelationThresh = FL2FXCONST_DBL(0.36);

static void FDKaacEnc_FDKaacEnc_noiseDetection(
    PNS_CONFIG *pnsConf, PNS_DATA *pnsData, const INT sfbActive,
    const INT *sfbOffset, INT tnsOrder, INT tnsPredictionGain, INT tnsActive,
    FIXP_DBL *mdctSpectrum, INT *sfbMaxScaleSpec, FIXP_SGL *sfbtonality);

static void FDKaacEnc_CalcNoiseNrgs(const INT sfbActive, INT *pnsFlag,
                                    FIXP_DBL *sfbEnergyLdData, INT *noiseNrg);

/*****************************************************************************

    functionname: initPnsConfiguration
    description:  fill pnsConf with pns parameters
    returns:      error status
    input:        PNS Config struct (modified)
                  bitrate, samplerate, usePns,
                  number of sfb's, pointer to sfb offset
    output:       error code

*****************************************************************************/

AAC_ENCODER_ERROR FDKaacEnc_InitPnsConfiguration(
    PNS_CONFIG *pnsConf, INT bitRate, INT sampleRate, INT usePns, INT sfbCnt,
    const INT *sfbOffset, const INT numChan, const INT isLC) {
  AAC_ENCODER_ERROR ErrorStatus;

  /* init noise detection */
  ErrorStatus = FDKaacEnc_GetPnsParam(&pnsConf->np, bitRate, sampleRate, sfbCnt,
                                      sfbOffset, &usePns, numChan, isLC);
  if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

  pnsConf->minCorrelationEnergy = minCorrelationEnergy;
  pnsConf->noiseCorrelationThresh = noiseCorrelationThresh;

  pnsConf->usePns = usePns;

  return AAC_ENC_OK;
}

/*****************************************************************************

    functionname: FDKaacEnc_PnsDetect
    description:  do decision, if PNS shall used or not
    returns:
    input:        pns config structure
                  pns data structure (modified),
                  lastWindowSequence (long or short blocks)
                  sfbActive
                  pointer to Sfb Energy, Threshold, Offset
                  pointer to mdct Spectrum
                  length of each group
                  pointer to tonality calculated in chaosmeasure
                  tns order and prediction gain
                  calculated noiseNrg at active PNS
    output:       pnsFlag in pns data structure

*****************************************************************************/
void FDKaacEnc_PnsDetect(PNS_CONFIG *pnsConf, PNS_DATA *pnsData,
                         const INT lastWindowSequence, const INT sfbActive,
                         const INT maxSfbPerGroup, FIXP_DBL *sfbThresholdLdData,
                         const INT *sfbOffset, FIXP_DBL *mdctSpectrum,
                         INT *sfbMaxScaleSpec, FIXP_SGL *sfbtonality,
                         INT tnsOrder, INT tnsPredictionGain, INT tnsActive,
                         FIXP_DBL *sfbEnergyLdData, INT *noiseNrg)

{
  int sfb;
  int startNoiseSfb;

  /* Reset pns info. */
  FDKmemclear(pnsData->pnsFlag, sizeof(pnsData->pnsFlag));
  for (sfb = 0; sfb < MAX_GROUPED_SFB; sfb++) {
    noiseNrg[sfb] = NO_NOISE_PNS;
  }

  /* Disable PNS and skip detection in certain cases. */
  if (pnsConf->usePns == 0) {
    return;
  } else {
    /* AAC - LC core encoder */
    if ((pnsConf->np.detectionAlgorithmFlags & IS_LOW_COMPLEXITY) &&
        (lastWindowSequence == SHORT_WINDOW)) {
      return;
    }
    /* AAC - (E)LD core encoder */
    if (!(pnsConf->np.detectionAlgorithmFlags & IS_LOW_COMPLEXITY) &&
        (pnsConf->np.detectionAlgorithmFlags & JUST_LONG_WINDOW) &&
        (lastWindowSequence != LONG_WINDOW)) {
      return;
    }
  }

  /*
    call noise detection
  */
  FDKaacEnc_FDKaacEnc_noiseDetection(
      pnsConf, pnsData, sfbActive, sfbOffset, tnsOrder, tnsPredictionGain,
      tnsActive, mdctSpectrum, sfbMaxScaleSpec, sfbtonality);

  /* set startNoiseSfb (long) */
  startNoiseSfb = pnsConf->np.startSfb;

  /* Set noise substitution status */
  for (sfb = 0; sfb < sfbActive; sfb++) {
    /* No PNS below startNoiseSfb */
    if (sfb < startNoiseSfb) {
      pnsData->pnsFlag[sfb] = 0;
      continue;
    }

    /*
      do noise substitution if
      fuzzy measure is high enough
      sfb freq > minimum sfb freq
      signal in coder band is not masked
    */

    if ((pnsData->noiseFuzzyMeasure[sfb] > FL2FXCONST_SGL(0.5)) &&
        ((sfbThresholdLdData[sfb] +
          FL2FXCONST_DBL(0.5849625f /
                         64.0f)) /* thr * 1.5 = thrLd +ld(1.5)/64 */
         < sfbEnergyLdData[sfb])) {
      /*
        mark in psyout flag array that we will code
        this band with PNS
      */
      pnsData->pnsFlag[sfb] = 1; /* PNS_ON */
    } else {
      pnsData->pnsFlag[sfb] = 0; /* PNS_OFF */
    }

    /* no PNS if LTP is active */
  }

  /* avoid PNS holes */
  if ((pnsData->noiseFuzzyMeasure[0] > FL2FXCONST_SGL(0.5f)) &&
      (pnsData->pnsFlag[1])) {
    pnsData->pnsFlag[0] = 1;
  }

  for (sfb = 1; sfb < maxSfbPerGroup - 1; sfb++) {
    if ((pnsData->noiseFuzzyMeasure[sfb] > pnsConf->np.gapFillThr) &&
        (pnsData->pnsFlag[sfb - 1]) && (pnsData->pnsFlag[sfb + 1])) {
      pnsData->pnsFlag[sfb] = 1;
    }
  }

  if (maxSfbPerGroup > 0) {
    /* avoid PNS hole */
    if ((pnsData->noiseFuzzyMeasure[maxSfbPerGroup - 1] >
         pnsConf->np.gapFillThr) &&
        (pnsData->pnsFlag[maxSfbPerGroup - 2])) {
      pnsData->pnsFlag[maxSfbPerGroup - 1] = 1;
    }
    /* avoid single PNS band */
    if (pnsData->pnsFlag[maxSfbPerGroup - 2] == 0) {
      pnsData->pnsFlag[maxSfbPerGroup - 1] = 0;
    }
  }

  /* avoid single PNS bands */
  if (pnsData->pnsFlag[1] == 0) {
    pnsData->pnsFlag[0] = 0;
  }

  for (sfb = 1; sfb < maxSfbPerGroup - 1; sfb++) {
    if ((pnsData->pnsFlag[sfb - 1] == 0) && (pnsData->pnsFlag[sfb + 1] == 0)) {
      pnsData->pnsFlag[sfb] = 0;
    }
  }

  /*
    calculate noiseNrg's
  */
  FDKaacEnc_CalcNoiseNrgs(sfbActive, pnsData->pnsFlag, sfbEnergyLdData,
                          noiseNrg);
}

/*****************************************************************************

    functionname:FDKaacEnc_FDKaacEnc_noiseDetection
    description: wrapper for noisedet.c
    returns:
    input:       pns config structure
                 pns data structure (modified),
                 sfbActive
                 tns order and prediction gain
                 pointer to mdct Spectrumand Sfb Energy
                 pointer to Sfb tonality
    output:      noiseFuzzyMeasure in structure pnsData
                 flags tonal / nontonal

*****************************************************************************/
static void FDKaacEnc_FDKaacEnc_noiseDetection(
    PNS_CONFIG *pnsConf, PNS_DATA *pnsData, const INT sfbActive,
    const INT *sfbOffset, int tnsOrder, INT tnsPredictionGain, INT tnsActive,
    FIXP_DBL *mdctSpectrum, INT *sfbMaxScaleSpec, FIXP_SGL *sfbtonality) {
  INT condition = TRUE;
  if (!(pnsConf->np.detectionAlgorithmFlags & IS_LOW_COMPLEXITY)) {
    condition = (tnsOrder > 3);
  }
  /*
  no PNS if heavy TNS activity
  clear pnsData->noiseFuzzyMeasure
  */
  if ((pnsConf->np.detectionAlgorithmFlags & USE_TNS_GAIN_THR) &&
      (tnsPredictionGain >= pnsConf->np.tnsGainThreshold) && condition &&
      !((pnsConf->np.detectionAlgorithmFlags & USE_TNS_PNS) &&
        (tnsPredictionGain >= pnsConf->np.tnsPNSGainThreshold) &&
        (tnsActive))) {
    /* clear all noiseFuzzyMeasure */
    FDKmemclear(pnsData->noiseFuzzyMeasure, sfbActive * sizeof(FIXP_SGL));
  } else {
    /*
    call noise detection, output in pnsData->noiseFuzzyMeasure,
    use real mdct spectral data
    */
    FDKaacEnc_noiseDetect(mdctSpectrum, sfbMaxScaleSpec, sfbActive, sfbOffset,
                          pnsData->noiseFuzzyMeasure, &pnsConf->np,
                          sfbtonality);
  }
}

/*****************************************************************************

    functionname:FDKaacEnc_CalcNoiseNrgs
    description: Calculate the NoiseNrg's
    returns:
    input:       sfbActive
                 if pnsFlag calculate NoiseNrg
                 pointer to sfbEnergy and groupLen
                 pointer to noiseNrg (modified)
    output:      noiseNrg's in pnsFlaged sfb's

*****************************************************************************/

static void FDKaacEnc_CalcNoiseNrgs(const INT sfbActive, INT *RESTRICT pnsFlag,
                                    FIXP_DBL *RESTRICT sfbEnergyLdData,
                                    INT *RESTRICT noiseNrg) {
  int sfb;
  INT tmp = (-LOG_NORM_PCM) << 2;

  for (sfb = 0; sfb < sfbActive; sfb++) {
    if (pnsFlag[sfb]) {
      INT nrg = (-sfbEnergyLdData[sfb] + FL2FXCONST_DBL(0.5f / 64.0f)) >>
                (DFRACT_BITS - 1 - 7);
      noiseNrg[sfb] = tmp - nrg;
    }
  }
}

/*****************************************************************************

    functionname:FDKaacEnc_CodePnsChannel
    description: Execute pns decission
    returns:
    input:       sfbActive
                 pns config structure
                 use PNS if pnsFlag
                 pointer to Sfb Energy, noiseNrg, Threshold
    output:      set sfbThreshold high to code pe with 0,
                 noiseNrg marks flag for pns coding

*****************************************************************************/

void FDKaacEnc_CodePnsChannel(const INT sfbActive, PNS_CONFIG *pnsConf,
                              INT *RESTRICT pnsFlag,
                              FIXP_DBL *RESTRICT sfbEnergyLdData,
                              INT *RESTRICT noiseNrg,
                              FIXP_DBL *RESTRICT sfbThresholdLdData) {
  INT sfb;
  INT lastiNoiseEnergy = 0;
  INT firstPNSband = 1; /* TRUE for first PNS-coded band */

  /* no PNS */
  if (!pnsConf->usePns) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      /* no PNS coding */
      noiseNrg[sfb] = NO_NOISE_PNS;
    }
    return;
  }

  /* code PNS */
  for (sfb = 0; sfb < sfbActive; sfb++) {
    if (pnsFlag[sfb]) {
      /* high sfbThreshold causes pe = 0 */
      if (noiseNrg[sfb] != NO_NOISE_PNS)
        sfbThresholdLdData[sfb] =
            sfbEnergyLdData[sfb] + FL2FXCONST_DBL(1.0f / LD_DATA_SCALING);

      /* set noiseNrg in valid region */
      if (!firstPNSband) {
        INT deltaiNoiseEnergy = noiseNrg[sfb] - lastiNoiseEnergy;

        if (deltaiNoiseEnergy > CODE_BOOK_PNS_LAV)
          noiseNrg[sfb] -= deltaiNoiseEnergy - CODE_BOOK_PNS_LAV;
        else if (deltaiNoiseEnergy < -CODE_BOOK_PNS_LAV)
          noiseNrg[sfb] -= deltaiNoiseEnergy + CODE_BOOK_PNS_LAV;
      } else {
        firstPNSband = 0;
      }
      lastiNoiseEnergy = noiseNrg[sfb];
    } else {
      /* no PNS coding */
      noiseNrg[sfb] = NO_NOISE_PNS;
    }
  }
}

/*****************************************************************************

    functionname:FDKaacEnc_PreProcessPnsChannelPair
    description: Calculate the correlation of noise in a channel pair

    returns:
    input:       sfbActive
                 pointer to sfb energies left, right and mid channel
                 pns config structure
                 pns data structure left and right (modified)

    output:      noiseEnergyCorrelation in pns data structure

*****************************************************************************/

void FDKaacEnc_PreProcessPnsChannelPair(
    const INT sfbActive, FIXP_DBL *RESTRICT sfbEnergyLeft,
    FIXP_DBL *RESTRICT sfbEnergyRight, FIXP_DBL *RESTRICT sfbEnergyLeftLD,
    FIXP_DBL *RESTRICT sfbEnergyRightLD, FIXP_DBL *RESTRICT sfbEnergyMid,
    PNS_CONFIG *RESTRICT pnsConf, PNS_DATA *pnsDataLeft,
    PNS_DATA *pnsDataRight) {
  INT sfb;
  FIXP_DBL ccf;

  if (!pnsConf->usePns) return;

  FIXP_DBL *RESTRICT pNoiseEnergyCorrelationL =
      pnsDataLeft->noiseEnergyCorrelation;
  FIXP_DBL *RESTRICT pNoiseEnergyCorrelationR =
      pnsDataRight->noiseEnergyCorrelation;

  for (sfb = 0; sfb < sfbActive; sfb++) {
    FIXP_DBL quot = (sfbEnergyLeftLD[sfb] >> 1) + (sfbEnergyRightLD[sfb] >> 1);

    if (quot < FL2FXCONST_DBL(-32.0f / (float)LD_DATA_SCALING))
      ccf = FL2FXCONST_DBL(0.0f);
    else {
      FIXP_DBL accu =
          sfbEnergyMid[sfb] -
          (((sfbEnergyLeft[sfb] >> 1) + (sfbEnergyRight[sfb] >> 1)) >> 1);
      INT sign = (accu < FL2FXCONST_DBL(0.0f)) ? 1 : 0;
      accu = fixp_abs(accu);

      ccf = CalcLdData(accu) +
            FL2FXCONST_DBL((float)1.0f / (float)LD_DATA_SCALING) -
            quot; /* ld(accu*2) = ld(accu) + 1 */
      ccf = (ccf >= FL2FXCONST_DBL(0.0))
                ? ((FIXP_DBL)MAXVAL_DBL)
                : (sign) ? -CalcInvLdData(ccf) : CalcInvLdData(ccf);
    }

    pNoiseEnergyCorrelationL[sfb] = ccf;
    pNoiseEnergyCorrelationR[sfb] = ccf;
  }
}

/*****************************************************************************

    functionname:FDKaacEnc_PostProcessPnsChannelPair
    description: if PNS used at left and right channel,
                 use msMask to flag correlation
    returns:
    input:       sfbActive
                 pns config structure
                 pns data structure left and right (modified)
                 pointer to msMask, flags correlation by pns coding (modified)
                 Digest of MS coding
    output:      pnsFlag in pns data structure,
                 msFlag in msMask (flags correlation)

*****************************************************************************/

void FDKaacEnc_PostProcessPnsChannelPair(const INT sfbActive,
                                         PNS_CONFIG *pnsConf,
                                         PNS_DATA *pnsDataLeft,
                                         PNS_DATA *pnsDataRight,
                                         INT *RESTRICT msMask, INT *msDigest) {
  INT sfb;

  if (!pnsConf->usePns) return;

  for (sfb = 0; sfb < sfbActive; sfb++) {
    /*
      MS post processing
    */
    if (msMask[sfb]) {
      if ((pnsDataLeft->pnsFlag[sfb]) && (pnsDataRight->pnsFlag[sfb])) {
        /* AAC only: Standard */
        /* do this to avoid ms flags in layers that should not have it */
        if (pnsDataLeft->noiseEnergyCorrelation[sfb] <=
            pnsConf->noiseCorrelationThresh) {
          msMask[sfb] = 0;
          *msDigest = MS_SOME;
        }
      } else {
        /*
          No PNS coding
        */
        pnsDataLeft->pnsFlag[sfb] = 0;
        pnsDataRight->pnsFlag[sfb] = 0;
      }
    }

    /*
      Use MS flag to signal noise correlation if
      pns is active in both channels
    */
    if ((pnsDataLeft->pnsFlag[sfb]) && (pnsDataRight->pnsFlag[sfb])) {
      if (pnsDataLeft->noiseEnergyCorrelation[sfb] >
          pnsConf->noiseCorrelationThresh) {
        msMask[sfb] = 1;
        *msDigest = MS_SOME;
      }
    }
  }
}
