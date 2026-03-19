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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

#include "mh_det.h"

#include "sbrenc_ram.h"
#include "sbr_misc.h"

#include "genericStds.h"

#define SFM_SHIFT 2 /* Attention: SFM_SCALE depends on SFM_SHIFT */
#define SFM_SCALE (MAXVAL_DBL >> SFM_SHIFT) /* 1.0 >> SFM_SHIFT */

/*!< Detector Parameters for AAC core codec. */
static const DETECTOR_PARAMETERS_MH paramsAac = {
    9, /*!< deltaTime */
    {
        FL2FXCONST_DBL(20.0f * RELAXATION_FLOAT), /*!< thresHoldDiff */
        FL2FXCONST_DBL(1.26f * RELAXATION_FLOAT), /*!< thresHoldDiffGuide */
        FL2FXCONST_DBL(15.0f * RELAXATION_FLOAT), /*!< thresHoldTone */
        FL2FXCONST_DBL((1.0f / 15.0f) *
                       RELAXATION_FLOAT),         /*!< invThresHoldTone */
        FL2FXCONST_DBL(1.26f * RELAXATION_FLOAT), /*!< thresHoldToneGuide */
        FL2FXCONST_DBL(0.3f) >> SFM_SHIFT,        /*!< sfmThresSbr */
        FL2FXCONST_DBL(0.1f) >> SFM_SHIFT,        /*!< sfmThresOrig */
        FL2FXCONST_DBL(0.3f),                     /*!< decayGuideOrig */
        FL2FXCONST_DBL(0.5f),                     /*!< decayGuideDiff */
        FL2FXCONST_DBL(-0.000112993269),
        /* LD64(FL2FXCONST_DBL(0.995f)) */ /*!< derivThresMaxLD64 */
        FL2FXCONST_DBL(-0.000112993269),
        /* LD64(FL2FXCONST_DBL(0.995f)) */ /*!< derivThresBelowLD64 */
        FL2FXCONST_DBL(
            -0.005030126483f) /* LD64(FL2FXCONST_DBL(0.8f)) */ /*!<
                                                                  derivThresAboveLD64
                                                                */
    },
    50 /*!< maxComp */
};

/*!< Detector Parameters for AAC LD core codec. */
static const DETECTOR_PARAMETERS_MH paramsAacLd = {
    16, /*!< Delta time. */
    {
        FL2FXCONST_DBL(25.0f * RELAXATION_FLOAT), /*!< thresHoldDiff */
        FL2FXCONST_DBL(1.26f * RELAXATION_FLOAT), /*!< tresHoldDiffGuide */
        FL2FXCONST_DBL(15.0f * RELAXATION_FLOAT), /*!< thresHoldTone */
        FL2FXCONST_DBL((1.0f / 15.0f) *
                       RELAXATION_FLOAT),         /*!< invThresHoldTone */
        FL2FXCONST_DBL(1.26f * RELAXATION_FLOAT), /*!< thresHoldToneGuide */
        FL2FXCONST_DBL(0.3f) >> SFM_SHIFT,        /*!< sfmThresSbr */
        FL2FXCONST_DBL(0.1f) >> SFM_SHIFT,        /*!< sfmThresOrig */
        FL2FXCONST_DBL(0.3f),                     /*!< decayGuideOrig */
        FL2FXCONST_DBL(0.2f),                     /*!< decayGuideDiff */
        FL2FXCONST_DBL(-0.000112993269),
        /* LD64(FL2FXCONST_DBL(0.995f)) */ /*!< derivThresMaxLD64 */
        FL2FXCONST_DBL(-0.000112993269),
        /* LD64(FL2FXCONST_DBL(0.995f)) */ /*!< derivThresBelowLD64 */
        FL2FXCONST_DBL(
            -0.005030126483f) /* LD64(FL2FXCONST_DBL(0.8f)) */ /*!<
                                                                  derivThresAboveLD64
                                                                */
    },
    50 /*!< maxComp */
};

/**************************************************************************/
/*!
  \brief     Calculates the difference in tonality between original and SBR
             for a given time and frequency region.

             The values for pDiffMapped2Scfb are scaled by RELAXATION

  \return    none.

*/
/**************************************************************************/
static void diff(FIXP_DBL *RESTRICT pTonalityOrig, FIXP_DBL *pDiffMapped2Scfb,
                 const UCHAR *RESTRICT pFreqBandTable, INT nScfb,
                 SCHAR *indexVector) {
  UCHAR i, ll, lu, k;
  FIXP_DBL maxValOrig, maxValSbr, tmp;
  INT scale;

  for (i = 0; i < nScfb; i++) {
    ll = pFreqBandTable[i];
    lu = pFreqBandTable[i + 1];

    maxValOrig = FL2FXCONST_DBL(0.0f);
    maxValSbr = FL2FXCONST_DBL(0.0f);

    for (k = ll; k < lu; k++) {
      maxValOrig = fixMax(maxValOrig, pTonalityOrig[k]);
      maxValSbr = fixMax(maxValSbr, pTonalityOrig[indexVector[k]]);
    }

    if ((maxValSbr >= RELAXATION)) {
      tmp = fDivNorm(maxValOrig, maxValSbr, &scale);
      pDiffMapped2Scfb[i] =
          scaleValue(fMult(tmp, RELAXATION_FRACT),
                     fixMax(-(DFRACT_BITS - 1), (scale - RELAXATION_SHIFT)));
    } else {
      pDiffMapped2Scfb[i] = maxValOrig;
    }
  }
}

/**************************************************************************/
/*!
  \brief     Calculates a flatness measure of the tonality measures.

  Calculation of the power function and using scalefactor for basis:
    Using log2:
    z  = (2^k * x)^y;
    z' = CalcLd(z) = y*CalcLd(x) + y*k;
    z  = CalcInvLd(z');

    Using ld64:
    z  = (2^k * x)^y;
    z' = CalcLd64(z) = y*CalcLd64(x)/64 + y*k/64;
    z  = CalcInvLd64(z');

  The values pSfmOrigVec and pSfmSbrVec are scaled by the factor 1/4.0

  \return    none.

*/
/**************************************************************************/
static void calculateFlatnessMeasure(FIXP_DBL *pQuotaBuffer, SCHAR *indexVector,
                                     FIXP_DBL *pSfmOrigVec,
                                     FIXP_DBL *pSfmSbrVec,
                                     const UCHAR *pFreqBandTable, INT nSfb) {
  INT i, j;
  FIXP_DBL invBands, tmp1, tmp2;
  INT shiftFac0, shiftFacSum0;
  INT shiftFac1, shiftFacSum1;
  FIXP_DBL accu;

  for (i = 0; i < nSfb; i++) {
    INT ll = pFreqBandTable[i];
    INT lu = pFreqBandTable[i + 1];
    pSfmOrigVec[i] = (FIXP_DBL)(MAXVAL_DBL >> 2);
    pSfmSbrVec[i] = (FIXP_DBL)(MAXVAL_DBL >> 2);

    if (lu - ll > 1) {
      FIXP_DBL amOrig, amTransp, gmOrig, gmTransp, sfmOrig, sfmTransp;
      invBands = GetInvInt(lu - ll);
      shiftFacSum0 = 0;
      shiftFacSum1 = 0;
      amOrig = amTransp = FL2FXCONST_DBL(0.0f);
      gmOrig = gmTransp = (FIXP_DBL)MAXVAL_DBL;

      for (j = ll; j < lu; j++) {
        sfmOrig = pQuotaBuffer[j];
        sfmTransp = pQuotaBuffer[indexVector[j]];

        amOrig += fMult(sfmOrig, invBands);
        amTransp += fMult(sfmTransp, invBands);

        shiftFac0 = CountLeadingBits(sfmOrig);
        shiftFac1 = CountLeadingBits(sfmTransp);

        gmOrig = fMult(gmOrig, sfmOrig << shiftFac0);
        gmTransp = fMult(gmTransp, sfmTransp << shiftFac1);

        shiftFacSum0 += shiftFac0;
        shiftFacSum1 += shiftFac1;
      }

      if (gmOrig > FL2FXCONST_DBL(0.0f)) {
        tmp1 = CalcLdData(gmOrig);    /* CalcLd64(x)/64 */
        tmp1 = fMult(invBands, tmp1); /* y*CalcLd64(x)/64 */

        /* y*k/64 */
        accu = (FIXP_DBL)-shiftFacSum0 << (DFRACT_BITS - 1 - 8);
        tmp2 = fMultDiv2(invBands, accu) << (2 + 1);

        tmp2 = tmp1 + tmp2;           /* y*CalcLd64(x)/64 + y*k/64 */
        gmOrig = CalcInvLdData(tmp2); /* CalcInvLd64(z'); */
      } else {
        gmOrig = FL2FXCONST_DBL(0.0f);
      }

      if (gmTransp > FL2FXCONST_DBL(0.0f)) {
        tmp1 = CalcLdData(gmTransp);  /* CalcLd64(x)/64 */
        tmp1 = fMult(invBands, tmp1); /* y*CalcLd64(x)/64 */

        /* y*k/64 */
        accu = (FIXP_DBL)-shiftFacSum1 << (DFRACT_BITS - 1 - 8);
        tmp2 = fMultDiv2(invBands, accu) << (2 + 1);

        tmp2 = tmp1 + tmp2;             /* y*CalcLd64(x)/64 + y*k/64 */
        gmTransp = CalcInvLdData(tmp2); /* CalcInvLd64(z'); */
      } else {
        gmTransp = FL2FXCONST_DBL(0.0f);
      }
      if (amOrig != FL2FXCONST_DBL(0.0f))
        pSfmOrigVec[i] =
            FDKsbrEnc_LSI_divide_scale_fract(gmOrig, amOrig, SFM_SCALE);

      if (amTransp != FL2FXCONST_DBL(0.0f))
        pSfmSbrVec[i] =
            FDKsbrEnc_LSI_divide_scale_fract(gmTransp, amTransp, SFM_SCALE);
    }
  }
}

/**************************************************************************/
/*!
  \brief     Calculates the input to the missing harmonics detection.


  \return    none.

*/
/**************************************************************************/
static void calculateDetectorInput(
    FIXP_DBL **RESTRICT pQuotaBuffer, /*!< Pointer to tonality matrix. */
    SCHAR *RESTRICT indexVector, FIXP_DBL **RESTRICT tonalityDiff,
    FIXP_DBL **RESTRICT pSfmOrig, FIXP_DBL **RESTRICT pSfmSbr,
    const UCHAR *freqBandTable, INT nSfb, INT noEstPerFrame, INT move) {
  INT est;

  /*
  New estimate.
  */
  for (est = 0; est < noEstPerFrame; est++) {
    diff(pQuotaBuffer[est + move], tonalityDiff[est + move], freqBandTable,
         nSfb, indexVector);

    calculateFlatnessMeasure(pQuotaBuffer[est + move], indexVector,
                             pSfmOrig[est + move], pSfmSbr[est + move],
                             freqBandTable, nSfb);
  }
}

/**************************************************************************/
/*!
  \brief     Checks that the detection is not due to a LP filter

  This function determines if a newly detected missing harmonics is not
  in fact just a low-pass filtere input signal. If so, the detection is
  removed.

  \return    none.

*/
/**************************************************************************/
static void removeLowPassDetection(UCHAR *RESTRICT pAddHarmSfb,
                                   UCHAR **RESTRICT pDetectionVectors,
                                   INT start, INT stop, INT nSfb,
                                   const UCHAR *RESTRICT pFreqBandTable,
                                   FIXP_DBL *RESTRICT pNrgVector,
                                   THRES_HOLDS mhThresh)

{
  INT i, est;
  INT maxDerivPos = pFreqBandTable[nSfb];
  INT numBands = pFreqBandTable[nSfb];
  FIXP_DBL nrgLow, nrgHigh;
  FIXP_DBL nrgLD64, nrgLowLD64, nrgHighLD64, nrgDiffLD64;
  FIXP_DBL valLD64, maxValLD64, maxValAboveLD64;
  INT bLPsignal = 0;

  maxValLD64 = FL2FXCONST_DBL(-1.0f);
  for (i = numBands - 1 - 2; i > pFreqBandTable[0]; i--) {
    nrgLow = pNrgVector[i];
    nrgHigh = pNrgVector[i + 2];

    if (nrgLow != FL2FXCONST_DBL(0.0f) && nrgLow > nrgHigh) {
      nrgLowLD64 = CalcLdData(nrgLow >> 1);
      nrgDiffLD64 = CalcLdData((nrgLow >> 1) - (nrgHigh >> 1));
      valLD64 = nrgDiffLD64 - nrgLowLD64;
      if (valLD64 > maxValLD64) {
        maxDerivPos = i;
        maxValLD64 = valLD64;
      }
      if (maxValLD64 > mhThresh.derivThresMaxLD64) {
        break;
      }
    }
  }

  /* Find the largest "gradient" above. (should be relatively flat, hence we
     expect a low value if the signal is LP.*/
  maxValAboveLD64 = FL2FXCONST_DBL(-1.0f);
  for (i = numBands - 1 - 2; i > maxDerivPos + 2; i--) {
    nrgLow = pNrgVector[i];
    nrgHigh = pNrgVector[i + 2];

    if (nrgLow != FL2FXCONST_DBL(0.0f) && nrgLow > nrgHigh) {
      nrgLowLD64 = CalcLdData(nrgLow >> 1);
      nrgDiffLD64 = CalcLdData((nrgLow >> 1) - (nrgHigh >> 1));
      valLD64 = nrgDiffLD64 - nrgLowLD64;
      if (valLD64 > maxValAboveLD64) {
        maxValAboveLD64 = valLD64;
      }
    } else {
      if (nrgHigh != FL2FXCONST_DBL(0.0f) && nrgHigh > nrgLow) {
        nrgHighLD64 = CalcLdData(nrgHigh >> 1);
        nrgDiffLD64 = CalcLdData((nrgHigh >> 1) - (nrgLow >> 1));
        valLD64 = nrgDiffLD64 - nrgHighLD64;
        if (valLD64 > maxValAboveLD64) {
          maxValAboveLD64 = valLD64;
        }
      }
    }
  }

  if (maxValLD64 > mhThresh.derivThresMaxLD64 &&
      maxValAboveLD64 < mhThresh.derivThresAboveLD64) {
    bLPsignal = 1;

    for (i = maxDerivPos - 1; i > maxDerivPos - 5 && i >= 0; i--) {
      if (pNrgVector[i] != FL2FXCONST_DBL(0.0f) &&
          pNrgVector[i] > pNrgVector[maxDerivPos + 2]) {
        nrgDiffLD64 = CalcLdData((pNrgVector[i] >> 1) -
                                 (pNrgVector[maxDerivPos + 2] >> 1));
        nrgLD64 = CalcLdData(pNrgVector[i] >> 1);
        valLD64 = nrgDiffLD64 - nrgLD64;
        if (valLD64 < mhThresh.derivThresBelowLD64) {
          bLPsignal = 0;
          break;
        }
      } else {
        bLPsignal = 0;
        break;
      }
    }
  }

  if (bLPsignal) {
    for (i = 0; i < nSfb; i++) {
      if (maxDerivPos >= pFreqBandTable[i] &&
          maxDerivPos < pFreqBandTable[i + 1])
        break;
    }

    if (pAddHarmSfb[i]) {
      pAddHarmSfb[i] = 0;
      for (est = start; est < stop; est++) {
        pDetectionVectors[est][i] = 0;
      }
    }
  }
}

/**************************************************************************/
/*!
  \brief     Checks if it is allowed to detect a missing tone, that wasn't
             detected previously.


  \return    newDetectionAllowed flag.

*/
/**************************************************************************/
static INT isDetectionOfNewToneAllowed(
    const SBR_FRAME_INFO *pFrameInfo, INT *pDetectionStartPos,
    INT noEstPerFrame, INT prevTransientFrame, INT prevTransientPos,
    INT prevTransientFlag, INT transientPosOffset, INT transientFlag,
    INT transientPos, INT deltaTime,
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector) {
  INT transientFrame, newDetectionAllowed;

  /* Determine if this is a frame where a transient starts...
   * If the transient flag was set the previous frame but not the
   * transient frame flag, the transient frame flag is set in the current frame.
   *****************************************************************************/
  transientFrame = 0;
  if (transientFlag) {
    if (transientPos + transientPosOffset <
        pFrameInfo->borders[pFrameInfo->nEnvelopes]) {
      transientFrame = 1;
      if (noEstPerFrame > 1) {
        if (transientPos + transientPosOffset >
            h_sbrMissingHarmonicsDetector->timeSlots >> 1) {
          *pDetectionStartPos = noEstPerFrame;
        } else {
          *pDetectionStartPos = noEstPerFrame >> 1;
        }

      } else {
        *pDetectionStartPos = noEstPerFrame;
      }
    }
  } else {
    if (prevTransientFlag && !prevTransientFrame) {
      transientFrame = 1;
      *pDetectionStartPos = 0;
    }
  }

  /*
   * Determine if detection of new missing harmonics are allowed.
   * If the frame contains a transient it's ok. If the previous
   * frame contained a transient it needs to be sufficiently close
   * to the start of the current frame.
   ****************************************************************/
  newDetectionAllowed = 0;
  if (transientFrame) {
    newDetectionAllowed = 1;
  } else {
    if (prevTransientFrame &&
        fixp_abs(pFrameInfo->borders[0] -
                 (prevTransientPos + transientPosOffset -
                  h_sbrMissingHarmonicsDetector->timeSlots)) < deltaTime) {
      newDetectionAllowed = 1;
      *pDetectionStartPos = 0;
    }
  }

  h_sbrMissingHarmonicsDetector->previousTransientFlag = transientFlag;
  h_sbrMissingHarmonicsDetector->previousTransientFrame = transientFrame;
  h_sbrMissingHarmonicsDetector->previousTransientPos = transientPos;

  return (newDetectionAllowed);
}

/**************************************************************************/
/*!
  \brief     Cleans up the detection after a transient.


  \return    none.

*/
/**************************************************************************/
static void transientCleanUp(FIXP_DBL **quotaBuffer, INT nSfb,
                             UCHAR **detectionVectors, UCHAR *pAddHarmSfb,
                             UCHAR *pPrevAddHarmSfb, INT **signBuffer,
                             const UCHAR *pFreqBandTable, INT start, INT stop,
                             INT newDetectionAllowed, FIXP_DBL *pNrgVector,
                             THRES_HOLDS mhThresh) {
  INT i, j, est;

  for (est = start; est < stop; est++) {
    for (i = 0; i < nSfb; i++) {
      pAddHarmSfb[i] = pAddHarmSfb[i] || detectionVectors[est][i];
    }
  }

  if (newDetectionAllowed == 1) {
    /*
     * Check for duplication of sines located
     * on the border of two scf-bands.
     *************************************************/
    for (i = 0; i < nSfb - 1; i++) {
      /* detection in adjacent channels.*/
      if (pAddHarmSfb[i] && pAddHarmSfb[i + 1]) {
        FIXP_DBL maxVal1, maxVal2;
        INT maxPos1, maxPos2, maxPosTime1, maxPosTime2;

        INT li = pFreqBandTable[i];
        INT ui = pFreqBandTable[i + 1];

        /* Find maximum tonality in the the two scf bands.*/
        maxPosTime1 = start;
        maxPos1 = li;
        maxVal1 = quotaBuffer[start][li];
        for (est = start; est < stop; est++) {
          for (j = li; j < ui; j++) {
            if (quotaBuffer[est][j] > maxVal1) {
              maxVal1 = quotaBuffer[est][j];
              maxPos1 = j;
              maxPosTime1 = est;
            }
          }
        }

        li = pFreqBandTable[i + 1];
        ui = pFreqBandTable[i + 2];

        /* Find maximum tonality in the the two scf bands.*/
        maxPosTime2 = start;
        maxPos2 = li;
        maxVal2 = quotaBuffer[start][li];
        for (est = start; est < stop; est++) {
          for (j = li; j < ui; j++) {
            if (quotaBuffer[est][j] > maxVal2) {
              maxVal2 = quotaBuffer[est][j];
              maxPos2 = j;
              maxPosTime2 = est;
            }
          }
        }

        /* If the maximum values are in adjacent QMF-channels, we need to remove
           the lowest of the two.*/
        if (maxPos2 - maxPos1 < 2) {
          if (pPrevAddHarmSfb[i] == 1 && pPrevAddHarmSfb[i + 1] == 0) {
            /* Keep the lower, remove the upper.*/
            pAddHarmSfb[i + 1] = 0;
            for (est = start; est < stop; est++) {
              detectionVectors[est][i + 1] = 0;
            }
          } else {
            if (pPrevAddHarmSfb[i] == 0 && pPrevAddHarmSfb[i + 1] == 1) {
              /* Keep the upper, remove the lower.*/
              pAddHarmSfb[i] = 0;
              for (est = start; est < stop; est++) {
                detectionVectors[est][i] = 0;
              }
            } else {
              /* If the maximum values are in adjacent QMF-channels, and if the
                 signs indicate that it is the same sine, we need to remove the
                 lowest of the two.*/
              if (maxVal1 > maxVal2) {
                if (signBuffer[maxPosTime1][maxPos2] < 0 &&
                    signBuffer[maxPosTime1][maxPos1] > 0) {
                  /* Keep the lower, remove the upper.*/
                  pAddHarmSfb[i + 1] = 0;
                  for (est = start; est < stop; est++) {
                    detectionVectors[est][i + 1] = 0;
                  }
                }
              } else {
                if (signBuffer[maxPosTime2][maxPos2] < 0 &&
                    signBuffer[maxPosTime2][maxPos1] > 0) {
                  /* Keep the upper, remove the lower.*/
                  pAddHarmSfb[i] = 0;
                  for (est = start; est < stop; est++) {
                    detectionVectors[est][i] = 0;
                  }
                }
              }
            }
          }
        }
      }
    }

    /* Make sure that the detection is not the cut-off of a low pass filter. */
    removeLowPassDetection(pAddHarmSfb, detectionVectors, start, stop, nSfb,
                           pFreqBandTable, pNrgVector, mhThresh);
  } else {
    /*
     * If a missing harmonic wasn't missing the previous frame
     * the transient-flag needs to be set in order to be allowed to detect it.
     *************************************************************************/
    for (i = 0; i < nSfb; i++) {
      if (pAddHarmSfb[i] - pPrevAddHarmSfb[i] > 0) pAddHarmSfb[i] = 0;
    }
  }
}

/*****************************************************************************/
/*!
  \brief     Detection for one tonality estimate.

  This is the actual missing harmonics detection, using information from the
  previous detection.

  If a missing harmonic was detected (in a previous frame) due to too high
  tonality differences, but there was not enough tonality difference in the
  current frame, the detection algorithm still continues to trace the strongest
  tone in the scalefactor band (assuming that this is the tone that is going to
  be replaced in the decoder). This is done to avoid abrupt endings of sines
  fading out (e.g. in the glockenspiel).

  The function also tries to estimate where one sine is going to be replaced
  with multiple sines (due to the patching). This is done by comparing the
  tonality flatness measure of the original and the SBR signal.

  The function also tries to estimate (for the scalefactor bands only
  containing one qmf subband) when a strong tone in the original will be
  replaced by a strong tone in the adjacent QMF subband.

  \return    none.

*/
/**************************************************************************/
static void detection(FIXP_DBL *quotaBuffer, FIXP_DBL *pDiffVecScfb, INT nSfb,
                      UCHAR *pHarmVec, const UCHAR *pFreqBandTable,
                      FIXP_DBL *sfmOrig, FIXP_DBL *sfmSbr,
                      GUIDE_VECTORS guideVectors, GUIDE_VECTORS newGuideVectors,
                      THRES_HOLDS mhThresh) {
  INT i, j, ll, lu;
  FIXP_DBL thresTemp, thresOrig;

  /*
   * Do detection on the difference vector, i.e. the difference between
   * the original and the transposed.
   *********************************************************************/
  for (i = 0; i < nSfb; i++) {
    thresTemp = (guideVectors.guideVectorDiff[i] != FL2FXCONST_DBL(0.0f))
                    ? fMax(fMult(mhThresh.decayGuideDiff,
                                 guideVectors.guideVectorDiff[i]),
                           mhThresh.thresHoldDiffGuide)
                    : mhThresh.thresHoldDiff;

    thresTemp = fMin(thresTemp, mhThresh.thresHoldDiff);

    if (pDiffVecScfb[i] > thresTemp) {
      pHarmVec[i] = 1;
      newGuideVectors.guideVectorDiff[i] = pDiffVecScfb[i];
    } else {
      /* If the guide wasn't zero, but the current level is to low,
         start tracking the decay on the tone in the original rather
         than the difference.*/
      if (guideVectors.guideVectorDiff[i] != FL2FXCONST_DBL(0.0f)) {
        guideVectors.guideVectorOrig[i] = mhThresh.thresHoldToneGuide;
      }
    }
  }

  /*
   * Trace tones in the original signal that at one point
   * have been detected because they will be replaced by
   * multiple tones in the sbr signal.
   ****************************************************/

  for (i = 0; i < nSfb; i++) {
    ll = pFreqBandTable[i];
    lu = pFreqBandTable[i + 1];

    thresOrig =
        fixMax(fMult(guideVectors.guideVectorOrig[i], mhThresh.decayGuideOrig),
               mhThresh.thresHoldToneGuide);
    thresOrig = fixMin(thresOrig, mhThresh.thresHoldTone);

    if (guideVectors.guideVectorOrig[i] != FL2FXCONST_DBL(0.0f)) {
      for (j = ll; j < lu; j++) {
        if (quotaBuffer[j] > thresOrig) {
          pHarmVec[i] = 1;
          newGuideVectors.guideVectorOrig[i] = quotaBuffer[j];
        }
      }
    }
  }

  /*
   * Check for multiple sines in the transposed signal,
   * where there is only one in the original.
   ****************************************************/
  thresOrig = mhThresh.thresHoldTone;

  for (i = 0; i < nSfb; i++) {
    ll = pFreqBandTable[i];
    lu = pFreqBandTable[i + 1];

    if (pHarmVec[i] == 0) {
      if (lu - ll > 1) {
        for (j = ll; j < lu; j++) {
          if (quotaBuffer[j] > thresOrig &&
              (sfmSbr[i] > mhThresh.sfmThresSbr &&
               sfmOrig[i] < mhThresh.sfmThresOrig)) {
            pHarmVec[i] = 1;
            newGuideVectors.guideVectorOrig[i] = quotaBuffer[j];
          }
        }
      } else {
        if (i < nSfb - 1) {
          ll = pFreqBandTable[i];

          if (i > 0) {
            if (quotaBuffer[ll] > mhThresh.thresHoldTone &&
                (pDiffVecScfb[i + 1] < mhThresh.invThresHoldTone ||
                 pDiffVecScfb[i - 1] < mhThresh.invThresHoldTone)) {
              pHarmVec[i] = 1;
              newGuideVectors.guideVectorOrig[i] = quotaBuffer[ll];
            }
          } else {
            if (quotaBuffer[ll] > mhThresh.thresHoldTone &&
                pDiffVecScfb[i + 1] < mhThresh.invThresHoldTone) {
              pHarmVec[i] = 1;
              newGuideVectors.guideVectorOrig[i] = quotaBuffer[ll];
            }
          }
        }
      }
    }
  }
}

/**************************************************************************/
/*!
  \brief     Do detection for every tonality estimate, using forward prediction.


  \return    none.

*/
/**************************************************************************/
static void detectionWithPrediction(
    FIXP_DBL **quotaBuffer, FIXP_DBL **pDiffVecScfb, INT **signBuffer, INT nSfb,
    const UCHAR *pFreqBandTable, FIXP_DBL **sfmOrig, FIXP_DBL **sfmSbr,
    UCHAR **detectionVectors, UCHAR *pPrevAddHarmSfb,
    GUIDE_VECTORS *guideVectors, INT noEstPerFrame, INT detectionStart,
    INT totNoEst, INT newDetectionAllowed, INT *pAddHarmFlag,
    UCHAR *pAddHarmSfb, FIXP_DBL *pNrgVector,
    const DETECTOR_PARAMETERS_MH *mhParams) {
  INT est = 0, i;
  INT start;

  FDKmemclear(pAddHarmSfb, nSfb * sizeof(UCHAR));

  if (newDetectionAllowed) {
    /* Since we don't want to use the transient region for detection (since the
       tonality values tend to be a bit unreliable for this region) the
       guide-values are copied to the current starting point. */
    if (totNoEst > 1) {
      start = detectionStart + 1;

      if (start != 0) {
        FDKmemcpy(guideVectors[start].guideVectorDiff,
                  guideVectors[0].guideVectorDiff, nSfb * sizeof(FIXP_DBL));
        FDKmemcpy(guideVectors[start].guideVectorOrig,
                  guideVectors[0].guideVectorOrig, nSfb * sizeof(FIXP_DBL));
        FDKmemclear(guideVectors[start - 1].guideVectorDetected,
                    nSfb * sizeof(UCHAR));
      }
    } else {
      start = 0;
    }
  } else {
    start = 0;
  }

  for (est = start; est < totNoEst; est++) {
    /*
     * Do detection on the current frame using
     * guide-info from the previous.
     *******************************************/
    if (est > 0) {
      FDKmemcpy(guideVectors[est].guideVectorDetected,
                detectionVectors[est - 1], nSfb * sizeof(UCHAR));
    }

    FDKmemclear(detectionVectors[est], nSfb * sizeof(UCHAR));

    if (est < totNoEst - 1) {
      FDKmemclear(guideVectors[est + 1].guideVectorDiff,
                  nSfb * sizeof(FIXP_DBL));
      FDKmemclear(guideVectors[est + 1].guideVectorOrig,
                  nSfb * sizeof(FIXP_DBL));
      FDKmemclear(guideVectors[est + 1].guideVectorDetected,
                  nSfb * sizeof(UCHAR));

      detection(quotaBuffer[est], pDiffVecScfb[est], nSfb,
                detectionVectors[est], pFreqBandTable, sfmOrig[est],
                sfmSbr[est], guideVectors[est], guideVectors[est + 1],
                mhParams->thresHolds);
    } else {
      FDKmemclear(guideVectors[est].guideVectorDiff, nSfb * sizeof(FIXP_DBL));
      FDKmemclear(guideVectors[est].guideVectorOrig, nSfb * sizeof(FIXP_DBL));
      FDKmemclear(guideVectors[est].guideVectorDetected, nSfb * sizeof(UCHAR));

      detection(quotaBuffer[est], pDiffVecScfb[est], nSfb,
                detectionVectors[est], pFreqBandTable, sfmOrig[est],
                sfmSbr[est], guideVectors[est], guideVectors[est],
                mhParams->thresHolds);
    }
  }

  /* Clean up the detection.*/
  transientCleanUp(quotaBuffer, nSfb, detectionVectors, pAddHarmSfb,
                   pPrevAddHarmSfb, signBuffer, pFreqBandTable, start, totNoEst,
                   newDetectionAllowed, pNrgVector, mhParams->thresHolds);

  /* Set flag... */
  *pAddHarmFlag = 0;
  for (i = 0; i < nSfb; i++) {
    if (pAddHarmSfb[i]) {
      *pAddHarmFlag = 1;
      break;
    }
  }

  FDKmemcpy(pPrevAddHarmSfb, pAddHarmSfb, nSfb * sizeof(UCHAR));
  FDKmemcpy(guideVectors[0].guideVectorDetected, pAddHarmSfb,
            nSfb * sizeof(INT));

  for (i = 0; i < nSfb; i++) {
    guideVectors[0].guideVectorDiff[i] = FL2FXCONST_DBL(0.0f);
    guideVectors[0].guideVectorOrig[i] = FL2FXCONST_DBL(0.0f);

    if (pAddHarmSfb[i] == 1) {
      /* If we had a detection use the guide-value in the next frame from the
      last estimate were the detection was done.*/
      for (est = start; est < totNoEst; est++) {
        if (guideVectors[est].guideVectorDiff[i] != FL2FXCONST_DBL(0.0f)) {
          guideVectors[0].guideVectorDiff[i] =
              guideVectors[est].guideVectorDiff[i];
        }
        if (guideVectors[est].guideVectorOrig[i] != FL2FXCONST_DBL(0.0f)) {
          guideVectors[0].guideVectorOrig[i] =
              guideVectors[est].guideVectorOrig[i];
        }
      }
    }
  }
}

/**************************************************************************/
/*!
  \brief     Calculates a compensation vector for the energy data.

  This function calculates a compensation vector for the energy data (i.e.
  envelope data) that is calculated elsewhere. This is since, one sine on
  the border of two scalefactor bands, will be replace by one sine in the
  middle of either scalefactor band. However, since the sine that is replaced
  will influence the energy estimate in both scalefactor bands (in the envelops
  calculation function) a compensation value is required in order to avoid
  noise substitution in the decoder next to the synthetic sine.

  \return    none.

*/
/**************************************************************************/
static void calculateCompVector(UCHAR *pAddHarmSfb, FIXP_DBL **pTonalityMatrix,
                                INT **pSignMatrix, UCHAR *pEnvComp, INT nSfb,
                                const UCHAR *freqBandTable, INT totNoEst,
                                INT maxComp, UCHAR *pPrevEnvComp,
                                INT newDetectionAllowed) {
  INT scfBand, est, l, ll, lu, maxPosF, maxPosT;
  FIXP_DBL maxVal;
  INT compValue;
  FIXP_DBL tmp;

  FDKmemclear(pEnvComp, nSfb * sizeof(UCHAR));

  for (scfBand = 0; scfBand < nSfb; scfBand++) {
    if (pAddHarmSfb[scfBand]) { /* A missing sine was detected */
      ll = freqBandTable[scfBand];
      lu = freqBandTable[scfBand + 1];

      maxPosF = 0; /* First find the maximum*/
      maxPosT = 0;
      maxVal = FL2FXCONST_DBL(0.0f);

      for (est = 0; est < totNoEst; est++) {
        for (l = ll; l < lu; l++) {
          if (pTonalityMatrix[est][l] > maxVal) {
            maxVal = pTonalityMatrix[est][l];
            maxPosF = l;
            maxPosT = est;
          }
        }
      }

      /*
       * If the maximum tonality is at the lower border of the
       * scalefactor band, we check the sign of the adjacent channels
       * to see if this sine is shared by the lower channel. If so, the
       * energy of the single sine will be present in two scalefactor bands
       * in the SBR data, which will cause problems in the decoder, when we
       * add a sine to just one of the channels.
       *********************************************************************/
      if (maxPosF == ll && scfBand) {
        if (!pAddHarmSfb[scfBand - 1]) { /* No detection below*/
          if (pSignMatrix[maxPosT][maxPosF - 1] > 0 &&
              pSignMatrix[maxPosT][maxPosF] < 0) {
            /* The comp value is calulated as the tonallity value, i.e we want
               to reduce the envelope data for this channel with as much as the
               tonality that is spread from the channel above. (ld64(RELAXATION)
               = 0.31143075889) */
            tmp = fixp_abs(
                (FIXP_DBL)CalcLdData(pTonalityMatrix[maxPosT][maxPosF - 1]) +
                RELAXATION_LD64);
            tmp = (tmp >> (DFRACT_BITS - 1 - LD_DATA_SHIFT - 1)) +
                  (FIXP_DBL)1; /* shift one bit less for rounding */
            compValue = ((INT)(LONG)tmp) >> 1;

            /* limit the comp-value*/
            if (compValue > maxComp) compValue = maxComp;

            pEnvComp[scfBand - 1] = compValue;
          }
        }
      }

      /*
       * Same as above, but for the upper end of the scalefactor-band.
       ***************************************************************/
      if (maxPosF == lu - 1 && scfBand + 1 < nSfb) { /* Upper border*/
        if (!pAddHarmSfb[scfBand + 1]) {
          if (pSignMatrix[maxPosT][maxPosF] > 0 &&
              pSignMatrix[maxPosT][maxPosF + 1] < 0) {
            tmp = fixp_abs(
                (FIXP_DBL)CalcLdData(pTonalityMatrix[maxPosT][maxPosF + 1]) +
                RELAXATION_LD64);
            tmp = (tmp >> (DFRACT_BITS - 1 - LD_DATA_SHIFT - 1)) +
                  (FIXP_DBL)1; /* shift one bit less for rounding */
            compValue = ((INT)(LONG)tmp) >> 1;

            if (compValue > maxComp) compValue = maxComp;

            pEnvComp[scfBand + 1] = compValue;
          }
        }
      }
    }
  }

  if (newDetectionAllowed == 0) {
    for (scfBand = 0; scfBand < nSfb; scfBand++) {
      if (pEnvComp[scfBand] != 0 && pPrevEnvComp[scfBand] == 0)
        pEnvComp[scfBand] = 0;
    }
  }

  /* remember the value for the next frame.*/
  FDKmemcpy(pPrevEnvComp, pEnvComp, nSfb * sizeof(UCHAR));
}

/**************************************************************************/
/*!
  \brief     Detects where strong tonal components will be missing after
             HFR in the decoder.


  \return    none.

*/
/**************************************************************************/
void FDKsbrEnc_SbrMissingHarmonicsDetectorQmf(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMHDet, FIXP_DBL **pQuotaBuffer,
    INT **pSignBuffer, SCHAR *indexVector, const SBR_FRAME_INFO *pFrameInfo,
    const UCHAR *pTranInfo, INT *pAddHarmonicsFlag,
    UCHAR *pAddHarmonicsScaleFactorBands, const UCHAR *freqBandTable, INT nSfb,
    UCHAR *envelopeCompensation, FIXP_DBL *pNrgVector) {
  INT transientFlag = pTranInfo[1];
  INT transientPos = pTranInfo[0];
  INT newDetectionAllowed;
  INT transientDetStart = 0;

  UCHAR **detectionVectors = h_sbrMHDet->detectionVectors;
  INT move = h_sbrMHDet->move;
  INT noEstPerFrame = h_sbrMHDet->noEstPerFrame;
  INT totNoEst = h_sbrMHDet->totNoEst;
  INT prevTransientFlag = h_sbrMHDet->previousTransientFlag;
  INT prevTransientFrame = h_sbrMHDet->previousTransientFrame;
  INT transientPosOffset = h_sbrMHDet->transientPosOffset;
  INT prevTransientPos = h_sbrMHDet->previousTransientPos;
  GUIDE_VECTORS *guideVectors = h_sbrMHDet->guideVectors;
  INT deltaTime = h_sbrMHDet->mhParams->deltaTime;
  INT maxComp = h_sbrMHDet->mhParams->maxComp;

  int est;

  /*
  Buffer values.
  */
  FDK_ASSERT(move <= (MAX_NO_OF_ESTIMATES >> 1));
  FDK_ASSERT(noEstPerFrame <= (MAX_NO_OF_ESTIMATES >> 1));

  FIXP_DBL *sfmSbr[MAX_NO_OF_ESTIMATES];
  FIXP_DBL *sfmOrig[MAX_NO_OF_ESTIMATES];
  FIXP_DBL *tonalityDiff[MAX_NO_OF_ESTIMATES];

  for (est = 0; est < MAX_NO_OF_ESTIMATES / 2; est++) {
    sfmSbr[est] = h_sbrMHDet->sfmSbr[est];
    sfmOrig[est] = h_sbrMHDet->sfmOrig[est];
    tonalityDiff[est] = h_sbrMHDet->tonalityDiff[est];
  }

  C_ALLOC_SCRATCH_START(_scratch, FIXP_DBL,
                        3 * MAX_NO_OF_ESTIMATES / 2 * MAX_FREQ_COEFFS)
  FIXP_DBL *scratch = _scratch;
  for (; est < MAX_NO_OF_ESTIMATES; est++) {
    sfmSbr[est] = scratch;
    scratch += MAX_FREQ_COEFFS;
    sfmOrig[est] = scratch;
    scratch += MAX_FREQ_COEFFS;
    tonalityDiff[est] = scratch;
    scratch += MAX_FREQ_COEFFS;
  }

  /* Determine if we're allowed to detect "missing harmonics" that wasn't
     detected before. In order to be allowed to do new detection, there must be
     a transient in the current frame, or a transient in the previous frame
     sufficiently close to the current frame. */
  newDetectionAllowed = isDetectionOfNewToneAllowed(
      pFrameInfo, &transientDetStart, noEstPerFrame, prevTransientFrame,
      prevTransientPos, prevTransientFlag, transientPosOffset, transientFlag,
      transientPos, deltaTime, h_sbrMHDet);

  /* Calulate the variables that will be used subsequently for the actual
   * detection */
  calculateDetectorInput(pQuotaBuffer, indexVector, tonalityDiff, sfmOrig,
                         sfmSbr, freqBandTable, nSfb, noEstPerFrame, move);

  /* Do the actual detection using information from previous detections */
  detectionWithPrediction(pQuotaBuffer, tonalityDiff, pSignBuffer, nSfb,
                          freqBandTable, sfmOrig, sfmSbr, detectionVectors,
                          h_sbrMHDet->guideScfb, guideVectors, noEstPerFrame,
                          transientDetStart, totNoEst, newDetectionAllowed,
                          pAddHarmonicsFlag, pAddHarmonicsScaleFactorBands,
                          pNrgVector, h_sbrMHDet->mhParams);

  /* Calculate the comp vector, so that the energy can be
     compensated for a sine between two QMF-bands. */
  calculateCompVector(pAddHarmonicsScaleFactorBands, pQuotaBuffer, pSignBuffer,
                      envelopeCompensation, nSfb, freqBandTable, totNoEst,
                      maxComp, h_sbrMHDet->prevEnvelopeCompensation,
                      newDetectionAllowed);

  for (est = 0; est < move; est++) {
    FDKmemcpy(tonalityDiff[est], tonalityDiff[est + noEstPerFrame],
              sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemcpy(sfmOrig[est], sfmOrig[est + noEstPerFrame],
              sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemcpy(sfmSbr[est], sfmSbr[est + noEstPerFrame],
              sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
  }
  C_ALLOC_SCRATCH_END(_scratch, FIXP_DBL,
                      3 * MAX_NO_OF_ESTIMATES / 2 * MAX_FREQ_COEFFS)
}

/**************************************************************************/
/*!
  \brief     Initialize an instance of the missing harmonics detector.


  \return    errorCode, noError if OK.

*/
/**************************************************************************/
INT FDKsbrEnc_CreateSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet, INT chan) {
  HANDLE_SBR_MISSING_HARMONICS_DETECTOR hs = hSbrMHDet;
  INT i;

  UCHAR *detectionVectors = GetRam_Sbr_detectionVectors(chan);
  UCHAR *guideVectorDetected = GetRam_Sbr_guideVectorDetected(chan);
  FIXP_DBL *guideVectorDiff = GetRam_Sbr_guideVectorDiff(chan);
  FIXP_DBL *guideVectorOrig = GetRam_Sbr_guideVectorOrig(chan);

  FDKmemclear(hs, sizeof(SBR_MISSING_HARMONICS_DETECTOR));

  hs->prevEnvelopeCompensation = GetRam_Sbr_prevEnvelopeCompensation(chan);
  hs->guideScfb = GetRam_Sbr_guideScfb(chan);

  if ((NULL == detectionVectors) || (NULL == guideVectorDetected) ||
      (NULL == guideVectorDiff) || (NULL == guideVectorOrig) ||
      (NULL == hs->prevEnvelopeCompensation) || (NULL == hs->guideScfb)) {
    goto bail;
  }

  for (i = 0; i < MAX_NO_OF_ESTIMATES; i++) {
    hs->guideVectors[i].guideVectorDiff =
        guideVectorDiff + (i * MAX_FREQ_COEFFS);
    hs->guideVectors[i].guideVectorOrig =
        guideVectorOrig + (i * MAX_FREQ_COEFFS);
    hs->detectionVectors[i] = detectionVectors + (i * MAX_FREQ_COEFFS);
    hs->guideVectors[i].guideVectorDetected =
        guideVectorDetected + (i * MAX_FREQ_COEFFS);
  }

  return 0;

bail:
  hs->guideVectors[0].guideVectorDiff = guideVectorDiff;
  hs->guideVectors[0].guideVectorOrig = guideVectorOrig;
  hs->detectionVectors[0] = detectionVectors;
  hs->guideVectors[0].guideVectorDetected = guideVectorDetected;

  FDKsbrEnc_DeleteSbrMissingHarmonicsDetector(hs);
  return -1;
}

/**************************************************************************/
/*!
  \brief     Initialize an instance of the missing harmonics detector.


  \return    errorCode, noError if OK.

*/
/**************************************************************************/
INT FDKsbrEnc_InitSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet, INT sampleFreq,
    INT frameSize, INT nSfb, INT qmfNoChannels, INT totNoEst, INT move,
    INT noEstPerFrame, UINT sbrSyntaxFlags) {
  HANDLE_SBR_MISSING_HARMONICS_DETECTOR hs = hSbrMHDet;
  int i;

  FDK_ASSERT(totNoEst <= MAX_NO_OF_ESTIMATES);

  if (sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    switch (frameSize) {
      case 1024:
      case 512:
        hs->transientPosOffset = FRAME_MIDDLE_SLOT_512LD;
        hs->timeSlots = 16;
        break;
      case 960:
      case 480:
        hs->transientPosOffset = FRAME_MIDDLE_SLOT_512LD;
        hs->timeSlots = 15;
        break;
      default:
        return -1;
    }
  } else {
    switch (frameSize) {
      case 2048:
      case 1024:
        hs->transientPosOffset = FRAME_MIDDLE_SLOT_2048;
        hs->timeSlots = NUMBER_TIME_SLOTS_2048;
        break;
      case 1920:
      case 960:
        hs->transientPosOffset = FRAME_MIDDLE_SLOT_1920;
        hs->timeSlots = NUMBER_TIME_SLOTS_1920;
        break;
      default:
        return -1;
    }
  }

  if (sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    hs->mhParams = &paramsAacLd;
  } else
    hs->mhParams = &paramsAac;

  hs->qmfNoChannels = qmfNoChannels;
  hs->sampleFreq = sampleFreq;
  hs->nSfb = nSfb;

  hs->totNoEst = totNoEst;
  hs->move = move;
  hs->noEstPerFrame = noEstPerFrame;

  for (i = 0; i < totNoEst; i++) {
    FDKmemclear(hs->guideVectors[i].guideVectorDiff,
                sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemclear(hs->guideVectors[i].guideVectorOrig,
                sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemclear(hs->detectionVectors[i], sizeof(UCHAR) * MAX_FREQ_COEFFS);
    FDKmemclear(hs->guideVectors[i].guideVectorDetected,
                sizeof(UCHAR) * MAX_FREQ_COEFFS);
  }

  // for(i=0; i<totNoEst/2; i++) {
  for (i = 0; i < MAX_NO_OF_ESTIMATES / 2; i++) {
    FDKmemclear(hs->tonalityDiff[i], sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemclear(hs->sfmOrig[i], sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
    FDKmemclear(hs->sfmSbr[i], sizeof(FIXP_DBL) * MAX_FREQ_COEFFS);
  }

  FDKmemclear(hs->prevEnvelopeCompensation, sizeof(UCHAR) * MAX_FREQ_COEFFS);
  FDKmemclear(hs->guideScfb, sizeof(UCHAR) * MAX_FREQ_COEFFS);

  hs->previousTransientFlag = 0;
  hs->previousTransientFrame = 0;
  hs->previousTransientPos = 0;

  return (0);
}

/**************************************************************************/
/*!
  \brief     Deletes an instance of the missing harmonics detector.


  \return    none.

*/
/**************************************************************************/
void FDKsbrEnc_DeleteSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet) {
  if (hSbrMHDet) {
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hs = hSbrMHDet;

    FreeRam_Sbr_detectionVectors(&hs->detectionVectors[0]);
    FreeRam_Sbr_guideVectorDetected(&hs->guideVectors[0].guideVectorDetected);
    FreeRam_Sbr_guideVectorDiff(&hs->guideVectors[0].guideVectorDiff);
    FreeRam_Sbr_guideVectorOrig(&hs->guideVectors[0].guideVectorOrig);
    FreeRam_Sbr_prevEnvelopeCompensation(&hs->prevEnvelopeCompensation);
    FreeRam_Sbr_guideScfb(&hs->guideScfb);
  }
}

/**************************************************************************/
/*!
  \brief     Resets an instance of the missing harmonics detector.


  \return    error code, noError if OK.

*/
/**************************************************************************/
INT FDKsbrEnc_ResetSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMissingHarmonicsDetector,
    INT nSfb) {
  int i;
  FIXP_DBL tempGuide[MAX_FREQ_COEFFS];
  UCHAR tempGuideInt[MAX_FREQ_COEFFS];
  INT nSfbPrev;

  nSfbPrev = hSbrMissingHarmonicsDetector->nSfb;
  hSbrMissingHarmonicsDetector->nSfb = nSfb;

  FDKmemcpy(tempGuideInt, hSbrMissingHarmonicsDetector->guideScfb,
            nSfbPrev * sizeof(UCHAR));

  if (nSfb > nSfbPrev) {
    for (i = 0; i < (nSfb - nSfbPrev); i++) {
      hSbrMissingHarmonicsDetector->guideScfb[i] = 0;
    }

    for (i = 0; i < nSfbPrev; i++) {
      hSbrMissingHarmonicsDetector->guideScfb[i + (nSfb - nSfbPrev)] =
          tempGuideInt[i];
    }
  } else {
    for (i = 0; i < nSfb; i++) {
      hSbrMissingHarmonicsDetector->guideScfb[i] =
          tempGuideInt[i + (nSfbPrev - nSfb)];
    }
  }

  FDKmemcpy(tempGuide,
            hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff,
            nSfbPrev * sizeof(FIXP_DBL));

  if (nSfb > nSfbPrev) {
    for (i = 0; i < (nSfb - nSfbPrev); i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff[i] =
          FL2FXCONST_DBL(0.0f);
    }

    for (i = 0; i < nSfbPrev; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0]
          .guideVectorDiff[i + (nSfb - nSfbPrev)] = tempGuide[i];
    }
  } else {
    for (i = 0; i < nSfb; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff[i] =
          tempGuide[i + (nSfbPrev - nSfb)];
    }
  }

  FDKmemcpy(tempGuide,
            hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig,
            nSfbPrev * sizeof(FIXP_DBL));

  if (nSfb > nSfbPrev) {
    for (i = 0; i < (nSfb - nSfbPrev); i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig[i] =
          FL2FXCONST_DBL(0.0f);
    }

    for (i = 0; i < nSfbPrev; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0]
          .guideVectorOrig[i + (nSfb - nSfbPrev)] = tempGuide[i];
    }
  } else {
    for (i = 0; i < nSfb; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig[i] =
          tempGuide[i + (nSfbPrev - nSfb)];
    }
  }

  FDKmemcpy(tempGuideInt,
            hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected,
            nSfbPrev * sizeof(UCHAR));

  if (nSfb > nSfbPrev) {
    for (i = 0; i < (nSfb - nSfbPrev); i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected[i] = 0;
    }

    for (i = 0; i < nSfbPrev; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0]
          .guideVectorDetected[i + (nSfb - nSfbPrev)] = tempGuideInt[i];
    }
  } else {
    for (i = 0; i < nSfb; i++) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected[i] =
          tempGuideInt[i + (nSfbPrev - nSfb)];
    }
  }

  FDKmemcpy(tempGuideInt,
            hSbrMissingHarmonicsDetector->prevEnvelopeCompensation,
            nSfbPrev * sizeof(UCHAR));

  if (nSfb > nSfbPrev) {
    for (i = 0; i < (nSfb - nSfbPrev); i++) {
      hSbrMissingHarmonicsDetector->prevEnvelopeCompensation[i] = 0;
    }

    for (i = 0; i < nSfbPrev; i++) {
      hSbrMissingHarmonicsDetector
          ->prevEnvelopeCompensation[i + (nSfb - nSfbPrev)] = tempGuideInt[i];
    }
  } else {
    for (i = 0; i < nSfb; i++) {
      hSbrMissingHarmonicsDetector->prevEnvelopeCompensation[i] =
          tempGuideInt[i + (nSfbPrev - nSfb)];
    }
  }

  return 0;
}
