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

#include "ton_corr.h"

#include "sbrenc_ram.h"
#include "sbr_misc.h"
#include "genericStds.h"
#include "autocorr2nd.h"

#define BAND_V_SIZE 32
#define NUM_V_COMBINE \
  8 /* Must be a divisor of 64 and fulfill the ASSERTs below */

/**************************************************************************/
/*!
  \brief Calculates the tonal to noise ration for different frequency bands
   and time segments.

   The ratio between the predicted energy (tonal energy A) and the total
   energy (A + B) is calculated. This is converted to the ratio between
   the predicted energy (tonal energy A) and the non-predictable energy
   (noise energy B). Hence the quota-matrix contains A/B = q/(1-q).

   The samples in nrgVector are scaled by 1.0/16.0
   The samples in pNrgVectorFreq  are scaled by 1.0/2.0
   The samples in quotaMatrix are scaled by RELAXATION

  \return none.

*/
/**************************************************************************/

void FDKsbrEnc_CalculateTonalityQuotas(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    FIXP_DBL **RESTRICT
        sourceBufferReal, /*!< The real part of the QMF-matrix.  */
    FIXP_DBL **RESTRICT
        sourceBufferImag, /*!< The imaginary part of the QMF-matrix. */
    INT usb,     /*!< upper side band, highest + 1 QMF band in the SBR range. */
    INT qmfScale /*!< sclefactor of QMF subsamples */
) {
  INT i, k, r, r2, timeIndex, autoCorrScaling;

  INT startIndexMatrix = hTonCorr->startIndexMatrix;
  INT totNoEst = hTonCorr->numberOfEstimates;
  INT noEstPerFrame = hTonCorr->numberOfEstimatesPerFrame;
  INT move = hTonCorr->move;
  INT noQmfChannels = hTonCorr->noQmfChannels; /* Number of Bands */
  INT buffLen = hTonCorr->bufferLength;        /* Number of Slots */
  INT stepSize = hTonCorr->stepSize;
  INT *pBlockLength = hTonCorr->lpcLength;
  INT **RESTRICT signMatrix = hTonCorr->signMatrix;
  FIXP_DBL *RESTRICT nrgVector = hTonCorr->nrgVector;
  FIXP_DBL **RESTRICT quotaMatrix = hTonCorr->quotaMatrix;
  FIXP_DBL *RESTRICT pNrgVectorFreq = hTonCorr->nrgVectorFreq;

  FIXP_DBL *realBuf;
  FIXP_DBL *imagBuf;

  FIXP_DBL alphar[2], alphai[2], fac;

  C_ALLOC_SCRATCH_START(ac, ACORR_COEFS, 1)
  C_ALLOC_SCRATCH_START(realBufRef, FIXP_DBL, 2 * BAND_V_SIZE * NUM_V_COMBINE)
  realBuf = realBufRef;
  imagBuf = realBuf + BAND_V_SIZE * NUM_V_COMBINE;

  FDK_ASSERT(buffLen <= BAND_V_SIZE);
  FDK_ASSERT(sizeof(FIXP_DBL) * NUM_V_COMBINE * BAND_V_SIZE * 2 <
             (1024 * sizeof(FIXP_DBL) - sizeof(ACORR_COEFS)));

  /*
   * Buffering of the quotaMatrix and the quotaMatrixTransp.
   *********************************************************/
  for (i = 0; i < move; i++) {
    FDKmemcpy(quotaMatrix[i], quotaMatrix[i + noEstPerFrame],
              noQmfChannels * sizeof(FIXP_DBL));
    FDKmemcpy(signMatrix[i], signMatrix[i + noEstPerFrame],
              noQmfChannels * sizeof(INT));
  }

  FDKmemmove(nrgVector, nrgVector + noEstPerFrame, move * sizeof(FIXP_DBL));
  FDKmemclear(nrgVector + startIndexMatrix,
              (totNoEst - startIndexMatrix) * sizeof(FIXP_DBL));
  FDKmemclear(pNrgVectorFreq, noQmfChannels * sizeof(FIXP_DBL));

  /*
   * Calculate the quotas for the current time steps.
   **************************************************/

  for (r = 0; r < usb; r++) {
    int blockLength;

    k = hTonCorr->nextSample; /* startSample */
    timeIndex = startIndexMatrix;
    /* Copy as many as possible Band across all Slots at once */
    if (realBuf != realBufRef) {
      realBuf -= BAND_V_SIZE;
      imagBuf -= BAND_V_SIZE;
    } else {
      realBuf += BAND_V_SIZE * (NUM_V_COMBINE - 1);
      imagBuf += BAND_V_SIZE * (NUM_V_COMBINE - 1);

      for (i = 0; i < buffLen; i++) {
        int v;
        FIXP_DBL *ptr;
        ptr = realBuf + i;
        for (v = 0; v < NUM_V_COMBINE; v++) {
          ptr[0] = sourceBufferReal[i][r + v];
          ptr[0 + BAND_V_SIZE * NUM_V_COMBINE] = sourceBufferImag[i][r + v];
          ptr -= BAND_V_SIZE;
        }
      }
    }

    blockLength = pBlockLength[0];

    while (k <= buffLen - blockLength) {
      autoCorrScaling = fixMin(
          getScalefactor(&realBuf[k - LPC_ORDER], LPC_ORDER + blockLength),
          getScalefactor(&imagBuf[k - LPC_ORDER], LPC_ORDER + blockLength));
      autoCorrScaling = fixMax(0, autoCorrScaling - 1);

      scaleValues(&realBuf[k - LPC_ORDER], LPC_ORDER + blockLength,
                  autoCorrScaling);
      scaleValues(&imagBuf[k - LPC_ORDER], LPC_ORDER + blockLength,
                  autoCorrScaling);

      autoCorrScaling <<= 1; /* consider qmf buffer scaling twice */
      autoCorrScaling +=
          autoCorr2nd_cplx(ac, realBuf + k, imagBuf + k, blockLength);

      if (ac->det == FL2FXCONST_DBL(0.0f)) {
        alphar[1] = alphai[1] = FL2FXCONST_DBL(0.0f);

        alphar[0] = (ac->r01r) >> 2;
        alphai[0] = (ac->r01i) >> 2;

        fac = fMultDiv2(ac->r00r, ac->r11r) >> 1;
      } else {
        alphar[1] = (fMultDiv2(ac->r01r, ac->r12r) >> 1) -
                    (fMultDiv2(ac->r01i, ac->r12i) >> 1) -
                    (fMultDiv2(ac->r02r, ac->r11r) >> 1);
        alphai[1] = (fMultDiv2(ac->r01i, ac->r12r) >> 1) +
                    (fMultDiv2(ac->r01r, ac->r12i) >> 1) -
                    (fMultDiv2(ac->r02i, ac->r11r) >> 1);

        alphar[0] = (fMultDiv2(ac->r01r, ac->det) >> (ac->det_scale + 1)) +
                    fMult(alphar[1], ac->r12r) + fMult(alphai[1], ac->r12i);
        alphai[0] = (fMultDiv2(ac->r01i, ac->det) >> (ac->det_scale + 1)) +
                    fMult(alphai[1], ac->r12r) - fMult(alphar[1], ac->r12i);

        fac = fMultDiv2(ac->r00r, fMult(ac->det, ac->r11r)) >>
              (ac->det_scale + 1);
      }

      if (fac == FL2FXCONST_DBL(0.0f)) {
        quotaMatrix[timeIndex][r] = FL2FXCONST_DBL(0.0f);
        signMatrix[timeIndex][r] = 0;
      } else {
        /* quotaMatrix is scaled with the factor RELAXATION
           parse RELAXATION in fractional part and shift factor: 1/(1/0.524288 *
           2^RELAXATION_SHIFT) */
        FIXP_DBL tmp, num, denom;
        INT numShift, denomShift, commonShift;
        INT sign;

        num = fMultDiv2(alphar[0], ac->r01r) + fMultDiv2(alphai[0], ac->r01i) -
              fMultDiv2(alphar[1], fMult(ac->r02r, ac->r11r)) -
              fMultDiv2(alphai[1], fMult(ac->r02i, ac->r11r));
        num = fixp_abs(num);

        denom = (fac >> 1) +
                (fMultDiv2(fac, RELAXATION_FRACT) >> RELAXATION_SHIFT) - num;
        denom = fixp_abs(denom);

        num = fMult(num, RELAXATION_FRACT);

        numShift = CountLeadingBits(num) - 2;
        num = scaleValue(num, numShift);

        denomShift = CountLeadingBits(denom);
        denom = (FIXP_DBL)denom << denomShift;

        if ((num > FL2FXCONST_DBL(0.0f)) && (denom != FL2FXCONST_DBL(0.0f))) {
          commonShift =
              fixMin(numShift - denomShift + RELAXATION_SHIFT, DFRACT_BITS - 1);
          if (commonShift < 0) {
            commonShift = -commonShift;
            tmp = schur_div(num, denom, 16);
            commonShift = fixMin(commonShift, CountLeadingBits(tmp));
            quotaMatrix[timeIndex][r] = tmp << commonShift;
          } else {
            quotaMatrix[timeIndex][r] =
                schur_div(num, denom, 16) >> commonShift;
          }
        } else {
          quotaMatrix[timeIndex][r] = FL2FXCONST_DBL(0.0f);
        }

        if (ac->r11r != FL2FXCONST_DBL(0.0f)) {
          if (((ac->r01r >= FL2FXCONST_DBL(0.0f)) &&
               (ac->r11r >= FL2FXCONST_DBL(0.0f))) ||
              ((ac->r01r < FL2FXCONST_DBL(0.0f)) &&
               (ac->r11r < FL2FXCONST_DBL(0.0f)))) {
            sign = 1;
          } else {
            sign = -1;
          }
        } else {
          sign = 1;
        }

        if (sign < 0) {
          r2 = r; /* (INT) pow(-1, band); */
        } else {
          r2 = r + 1; /* (INT) pow(-1, band+1); */
        }
        signMatrix[timeIndex][r] = 1 - 2 * (r2 & 0x1);
      }

      nrgVector[timeIndex] +=
          ((ac->r00r) >>
           fixMin(DFRACT_BITS - 1,
                  (2 * qmfScale + autoCorrScaling + SCALE_NRGVEC)));
      /* pNrgVectorFreq[r] finally has to be divided by noEstPerFrame, replaced
       * division by shifting with one */
      pNrgVectorFreq[r] =
          pNrgVectorFreq[r] +
          ((ac->r00r) >>
           fixMin(DFRACT_BITS - 1,
                  (2 * qmfScale + autoCorrScaling + SCALE_NRGVEC)));

      blockLength = pBlockLength[1];
      k += stepSize;
      timeIndex++;
    }
  }

  C_ALLOC_SCRATCH_END(realBufRef, FIXP_DBL, 2 * BAND_V_SIZE * NUM_V_COMBINE)
  C_ALLOC_SCRATCH_END(ac, ACORR_COEFS, 1)
}

/**************************************************************************/
/*!
  \brief Extracts the parameters required in the decoder to obtain the
  correct tonal to noise ratio after SBR.

  Estimates the tonal to noise ratio of the original signal (using LPC).
  Predicts the tonal to noise ration of the SBR signal (in the decoder) by
  patching the tonal to noise ratio values similar to the patching of the
  lowband in the decoder. Given the tonal to noise ratio of the original
  and the SBR signal, it estimates the required amount of inverse filtering,
  additional noise as well as any additional sines.

  \return none.

*/
/**************************************************************************/
void FDKsbrEnc_TonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    INVF_MODE *infVec, /*!< Vector where the inverse filtering levels will be
                          stored. */
    FIXP_DBL *noiseLevels, /*!< Vector where the noise levels will be stored. */
    INT *missingHarmonicFlag, /*!< Flag set to one or zero, dependent on if any
                                 strong sines are missing.*/
    UCHAR *missingHarmonicsIndex, /*!< Vector indicating where sines are
                                     missing. */
    UCHAR *envelopeCompensation,  /*!< Vector to store compensation values for
                                     the energies in. */
    const SBR_FRAME_INFO *frameInfo, /*!< Frame info struct, contains the time
                                        and frequency grid of the current
                                        frame.*/
    UCHAR *transientInfo,            /*!< Transient info.*/
    UCHAR *freqBandTable,            /*!< Frequency band tables for high-res.*/
    INT nSfb,           /*!< Number of scalefactor bands for high-res. */
    XPOS_MODE xposType, /*!< Type of transposer used in the decoder.*/
    UINT sbrSyntaxFlags) {
  INT band;
  INT transientFlag = transientInfo[1]; /*!< Flag indicating if a transient is
                                           present in the current frame. */
  INT transientPos = transientInfo[0];  /*!< Position of the transient.*/
  INT transientFrame, transientFrameInvfEst;
  INVF_MODE *infVecPtr;

  /* Determine if this is a frame where a transient starts...

  The detection of noise-floor, missing harmonics and invf_est, is not in sync
  for the non-buf-opt decoder such as AAC. Hence we need to keep track on the
  transient in the present frame as well as in the next.
  */
  transientFrame = 0;
  if (hTonCorr->transientNextFrame) { /* The transient was detected in the
                                         previous frame, but is actually */
    transientFrame = 1;
    hTonCorr->transientNextFrame = 0;

    if (transientFlag) {
      if (transientPos + hTonCorr->transientPosOffset >=
          frameInfo->borders[frameInfo->nEnvelopes]) {
        hTonCorr->transientNextFrame = 1;
      }
    }
  } else {
    if (transientFlag) {
      if (transientPos + hTonCorr->transientPosOffset <
          frameInfo->borders[frameInfo->nEnvelopes]) {
        transientFrame = 1;
        hTonCorr->transientNextFrame = 0;
      } else {
        hTonCorr->transientNextFrame = 1;
      }
    }
  }
  transientFrameInvfEst = transientFrame;

  /*
    Estimate the required invese filtereing level.
  */
  if (hTonCorr->switchInverseFilt)
    FDKsbrEnc_qmfInverseFilteringDetector(
        &hTonCorr->sbrInvFilt, hTonCorr->quotaMatrix, hTonCorr->nrgVector,
        hTonCorr->indexVector, hTonCorr->frameStartIndexInvfEst,
        hTonCorr->numberOfEstimatesPerFrame + hTonCorr->frameStartIndexInvfEst,
        transientFrameInvfEst, infVec);

  /*
      Detect what tones will be missing.
   */
  if (xposType == XPOS_LC) {
    FDKsbrEnc_SbrMissingHarmonicsDetectorQmf(
        &hTonCorr->sbrMissingHarmonicsDetector, hTonCorr->quotaMatrix,
        hTonCorr->signMatrix, hTonCorr->indexVector, frameInfo, transientInfo,
        missingHarmonicFlag, missingHarmonicsIndex, freqBandTable, nSfb,
        envelopeCompensation, hTonCorr->nrgVectorFreq);
  } else {
    *missingHarmonicFlag = 0;
    FDKmemclear(missingHarmonicsIndex, nSfb * sizeof(UCHAR));
  }

  /*
    Noise floor estimation
  */

  infVecPtr = hTonCorr->sbrInvFilt.prevInvfMode;

  FDKsbrEnc_sbrNoiseFloorEstimateQmf(
      &hTonCorr->sbrNoiseFloorEstimate, frameInfo, noiseLevels,
      hTonCorr->quotaMatrix, hTonCorr->indexVector, *missingHarmonicFlag,
      hTonCorr->frameStartIndex, hTonCorr->numberOfEstimatesPerFrame,
      transientFrame, infVecPtr, sbrSyntaxFlags);

  /* Store the invfVec data for the next frame...*/
  for (band = 0; band < hTonCorr->sbrInvFilt.noDetectorBands; band++) {
    hTonCorr->sbrInvFilt.prevInvfMode[band] = infVec[band];
  }
}

/**************************************************************************/
/*!
  \brief     Searches for the closest match in the frequency master table.



  \return   closest entry.

*/
/**************************************************************************/
static INT findClosestEntry(INT goalSb, UCHAR *v_k_master, INT numMaster,
                            INT direction) {
  INT index;

  if (goalSb <= v_k_master[0]) return v_k_master[0];

  if (goalSb >= v_k_master[numMaster]) return v_k_master[numMaster];

  if (direction) {
    index = 0;
    while (v_k_master[index] < goalSb) {
      index++;
    }
  } else {
    index = numMaster;
    while (v_k_master[index] > goalSb) {
      index--;
    }
  }

  return v_k_master[index];
}

/**************************************************************************/
/*!
  \brief     resets the patch



  \return   errorCode, noError if successful.

*/
/**************************************************************************/
static INT resetPatch(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    INT xposctrl,                     /*!< Different patch modes. */
    INT highBandStartSb,              /*!< Start band of the SBR range. */
    UCHAR *v_k_master, /*!< Master frequency table from which all other table
                          are derived.*/
    INT numMaster,     /*!< Number of elements in the master table. */
    INT fs,            /*!< Sampling frequency. */
    INT noChannels)    /*!< Number of QMF-channels. */
{
  INT patch, k, i;
  INT targetStopBand;

  PATCH_PARAM *patchParam = hTonCorr->patchParam;

  INT sbGuard = hTonCorr->guard;
  INT sourceStartBand;
  INT patchDistance;
  INT numBandsInPatch;

  INT lsb =
      v_k_master[0]; /* Lowest subband related to the synthesis filterbank */
  INT usb = v_k_master[numMaster]; /* Stop subband related to the synthesis
                                      filterbank */
  INT xoverOffset =
      highBandStartSb -
      v_k_master[0]; /* Calculate distance in subbands between k0 and kx */

  INT goalSb;

  /*
   * Initialize the patching parameter
   */

  if (xposctrl == 1) {
    lsb += xoverOffset;
    xoverOffset = 0;
  }

  goalSb = (INT)((2 * noChannels * 16000 + (fs >> 1)) / fs); /* 16 kHz band */
  goalSb = findClosestEntry(goalSb, v_k_master, numMaster,
                            1); /* Adapt region to master-table */

  /* First patch */
  sourceStartBand = hTonCorr->shiftStartSb + xoverOffset;
  targetStopBand = lsb + xoverOffset;

  /* even (odd) numbered channel must be patched to even (odd) numbered channel
   */
  patch = 0;
  while (targetStopBand < usb) {
    /* To many patches */
    if (patch >= MAX_NUM_PATCHES) return (1); /*Number of patches to high */

    patchParam[patch].guardStartBand = targetStopBand;
    targetStopBand += sbGuard;
    patchParam[patch].targetStartBand = targetStopBand;

    numBandsInPatch =
        goalSb - targetStopBand; /* get the desired range of the patch */

    if (numBandsInPatch >= lsb - sourceStartBand) {
      /* desired number bands are not available -> patch whole source range */
      patchDistance =
          targetStopBand - sourceStartBand; /* get the targetOffset */
      patchDistance =
          patchDistance & ~1; /* rounding off odd numbers and make all even */
      numBandsInPatch = lsb - (targetStopBand - patchDistance);
      numBandsInPatch = findClosestEntry(targetStopBand + numBandsInPatch,
                                         v_k_master, numMaster, 0) -
                        targetStopBand; /* Adapt region to master-table */
    }

    /* desired number bands are available -> get the minimal even patching
     * distance */
    patchDistance =
        numBandsInPatch + targetStopBand - lsb; /* get minimal distance */
    patchDistance = (patchDistance + 1) &
                    ~1; /* rounding up odd numbers and make all even */

    if (numBandsInPatch <= 0) {
      patch--;
    } else {
      patchParam[patch].sourceStartBand = targetStopBand - patchDistance;
      patchParam[patch].targetBandOffs = patchDistance;
      patchParam[patch].numBandsInPatch = numBandsInPatch;
      patchParam[patch].sourceStopBand =
          patchParam[patch].sourceStartBand + numBandsInPatch;

      targetStopBand += patchParam[patch].numBandsInPatch;
    }

    /* All patches but first */
    sourceStartBand = hTonCorr->shiftStartSb;

    /* Check if we are close to goalSb */
    if (fixp_abs(targetStopBand - goalSb) < 3) {
      goalSb = usb;
    }

    patch++;
  }

  patch--;

  /* if highest patch contains less than three subband: skip it */
  if (patchParam[patch].numBandsInPatch < 3 && patch > 0) {
    patch--;
  }

  hTonCorr->noOfPatches = patch + 1;

  /* Assign the index-vector, so we know where to look for the high-band.
     -1 represents a guard-band. */
  for (k = 0; k < hTonCorr->patchParam[0].guardStartBand; k++)
    hTonCorr->indexVector[k] = k;

  for (i = 0; i < hTonCorr->noOfPatches; i++) {
    INT sourceStart = hTonCorr->patchParam[i].sourceStartBand;
    INT targetStart = hTonCorr->patchParam[i].targetStartBand;
    INT numberOfBands = hTonCorr->patchParam[i].numBandsInPatch;
    INT startGuardBand = hTonCorr->patchParam[i].guardStartBand;

    for (k = 0; k < (targetStart - startGuardBand); k++)
      hTonCorr->indexVector[startGuardBand + k] = -1;

    for (k = 0; k < numberOfBands; k++)
      hTonCorr->indexVector[targetStart + k] = sourceStart + k;
  }

  return (0);
}

/**************************************************************************/
/*!
  \brief     Creates an instance of the tonality correction parameter module.

  The module includes modules for inverse filtering level estimation,
  missing harmonics detection and noise floor level estimation.

  \return   errorCode, noError if successful.
*/
/**************************************************************************/
INT FDKsbrEnc_CreateTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST
        hTonCorr, /*!< Pointer to handle to SBR_TON_CORR struct. */
    INT chan)     /*!< Channel index, needed for mem allocation */
{
  INT i;
  FIXP_DBL *quotaMatrix = GetRam_Sbr_quotaMatrix(chan);
  INT *signMatrix = GetRam_Sbr_signMatrix(chan);

  if ((NULL == quotaMatrix) || (NULL == signMatrix)) {
    goto bail;
  }

  FDKmemclear(hTonCorr, sizeof(SBR_TON_CORR_EST));

  for (i = 0; i < MAX_NO_OF_ESTIMATES; i++) {
    hTonCorr->quotaMatrix[i] = quotaMatrix + (i * 64);
    hTonCorr->signMatrix[i] = signMatrix + (i * 64);
  }

  if (0 != FDKsbrEnc_CreateSbrMissingHarmonicsDetector(
               &hTonCorr->sbrMissingHarmonicsDetector, chan)) {
    goto bail;
  }

  return 0;

bail:
  hTonCorr->quotaMatrix[0] = quotaMatrix;
  hTonCorr->signMatrix[0] = signMatrix;

  FDKsbrEnc_DeleteTonCorrParamExtr(hTonCorr);

  return -1;
}

/**************************************************************************/
/*!
  \brief     Initialize an instance of the tonality correction parameter module.

  The module includes modules for inverse filtering level estimation,
  missing harmonics detection and noise floor level estimation.

  \return   errorCode, noError if successful.
*/
/**************************************************************************/
INT FDKsbrEnc_InitTonCorrParamExtr(
    INT frameSize, /*!< Current SBR frame size. */
    HANDLE_SBR_TON_CORR_EST
        hTonCorr, /*!< Pointer to handle to SBR_TON_CORR struct. */
    HANDLE_SBR_CONFIG_DATA
        sbrCfg,           /*!< Pointer to SBR configuration parameters. */
    INT timeSlots,        /*!< Number of time-slots per frame */
    INT xposCtrl,         /*!< Different patch modes. */
    INT ana_max_level,    /*!< Maximum level of the adaptive noise. */
    INT noiseBands,       /*!< Number of noise bands per octave. */
    INT noiseFloorOffset, /*!< Noise floor offset. */
    UINT useSpeechConfig) /*!< Speech or music tuning. */
{
  INT nCols = sbrCfg->noQmfSlots;
  INT fs = sbrCfg->sampleFreq;
  INT noQmfChannels = sbrCfg->noQmfBands;

  INT highBandStartSb = sbrCfg->freqBandTable[LOW_RES][0];
  UCHAR *v_k_master = sbrCfg->v_k_master;
  INT numMaster = sbrCfg->num_Master;

  UCHAR **freqBandTable = sbrCfg->freqBandTable;
  INT *nSfb = sbrCfg->nSfb;

  INT i;

  /*
  Reset the patching and allocate memory for the quota matrix.
  Assuming parameters for the LPC analysis.
  */
  if (sbrCfg->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    switch (timeSlots) {
      case NUMBER_TIME_SLOTS_1920:
        hTonCorr->lpcLength[0] = 8 - LPC_ORDER;
        hTonCorr->lpcLength[1] = 7 - LPC_ORDER;
        hTonCorr->numberOfEstimates = NO_OF_ESTIMATES_LD;
        hTonCorr->numberOfEstimatesPerFrame = 2; /* sbrCfg->noQmfSlots / 7 */
        hTonCorr->frameStartIndexInvfEst = 0;
        hTonCorr->transientPosOffset = FRAME_MIDDLE_SLOT_512LD;
        break;
      case NUMBER_TIME_SLOTS_2048:
        hTonCorr->lpcLength[0] = 8 - LPC_ORDER;
        hTonCorr->lpcLength[1] = 8 - LPC_ORDER;
        hTonCorr->numberOfEstimates = NO_OF_ESTIMATES_LD;
        hTonCorr->numberOfEstimatesPerFrame = 2; /* sbrCfg->noQmfSlots / 8 */
        hTonCorr->frameStartIndexInvfEst = 0;
        hTonCorr->transientPosOffset = FRAME_MIDDLE_SLOT_512LD;
        break;
    }
  } else
    switch (timeSlots) {
      case NUMBER_TIME_SLOTS_2048:
        hTonCorr->lpcLength[0] = 16 - LPC_ORDER; /* blockLength[0] */
        hTonCorr->lpcLength[1] = 16 - LPC_ORDER; /* blockLength[0] */
        hTonCorr->numberOfEstimates = NO_OF_ESTIMATES_LC;
        hTonCorr->numberOfEstimatesPerFrame = sbrCfg->noQmfSlots / 16;
        hTonCorr->frameStartIndexInvfEst = 0;
        hTonCorr->transientPosOffset = FRAME_MIDDLE_SLOT_2048;
        break;
      case NUMBER_TIME_SLOTS_1920:
        hTonCorr->lpcLength[0] = 15 - LPC_ORDER; /* blockLength[0] */
        hTonCorr->lpcLength[1] = 15 - LPC_ORDER; /* blockLength[0] */
        hTonCorr->numberOfEstimates = NO_OF_ESTIMATES_LC;
        hTonCorr->numberOfEstimatesPerFrame = sbrCfg->noQmfSlots / 15;
        hTonCorr->frameStartIndexInvfEst = 0;
        hTonCorr->transientPosOffset = FRAME_MIDDLE_SLOT_1920;
        break;
      default:
        return -1;
    }

  hTonCorr->bufferLength = nCols;
  hTonCorr->stepSize =
      hTonCorr->lpcLength[0] + LPC_ORDER; /* stepSize[0] implicitly 0. */

  hTonCorr->nextSample = LPC_ORDER; /* firstSample */
  hTonCorr->move = hTonCorr->numberOfEstimates -
                   hTonCorr->numberOfEstimatesPerFrame; /* Number of estimates
                                                           to move when
                                                           buffering.*/
  if (hTonCorr->move < 0) {
    return -1;
  }
  hTonCorr->startIndexMatrix =
      hTonCorr->numberOfEstimates -
      hTonCorr->numberOfEstimatesPerFrame; /* Where to store the latest
                                              estimations in the tonality
                                              Matrix.*/
  hTonCorr->frameStartIndex = 0; /* Where in the tonality matrix the current
                                    frame (to be sent to the decoder) starts. */
  hTonCorr->prevTransientFlag = 0;
  hTonCorr->transientNextFrame = 0;

  hTonCorr->noQmfChannels = noQmfChannels;

  for (i = 0; i < hTonCorr->numberOfEstimates; i++) {
    FDKmemclear(hTonCorr->quotaMatrix[i], sizeof(FIXP_DBL) * noQmfChannels);
    FDKmemclear(hTonCorr->signMatrix[i], sizeof(INT) * noQmfChannels);
  }

  /* Reset the patch.*/
  hTonCorr->guard = 0;
  hTonCorr->shiftStartSb = 1;

  if (resetPatch(hTonCorr, xposCtrl, highBandStartSb, v_k_master, numMaster, fs,
                 noQmfChannels))
    return (1);

  if (FDKsbrEnc_InitSbrNoiseFloorEstimate(
          &hTonCorr->sbrNoiseFloorEstimate, ana_max_level, freqBandTable[LO],
          nSfb[LO], noiseBands, noiseFloorOffset, timeSlots, useSpeechConfig))
    return (1);

  if (FDKsbrEnc_initInvFiltDetector(
          &hTonCorr->sbrInvFilt,
          hTonCorr->sbrNoiseFloorEstimate.freqBandTableQmf,
          hTonCorr->sbrNoiseFloorEstimate.noNoiseBands, useSpeechConfig))
    return (1);

  if (FDKsbrEnc_InitSbrMissingHarmonicsDetector(
          &hTonCorr->sbrMissingHarmonicsDetector, fs, frameSize, nSfb[HI],
          noQmfChannels, hTonCorr->numberOfEstimates, hTonCorr->move,
          hTonCorr->numberOfEstimatesPerFrame, sbrCfg->sbrSyntaxFlags))
    return (1);

  return (0);
}

/**************************************************************************/
/*!
  \brief     resets tonality correction parameter module.



  \return   errorCode, noError if successful.

*/
/**************************************************************************/
INT FDKsbrEnc_ResetTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    INT xposctrl,                     /*!< Different patch modes. */
    INT highBandStartSb,              /*!< Start band of the SBR range. */
    UCHAR *v_k_master, /*!< Master frequency table from which all other table
                          are derived.*/
    INT numMaster,     /*!< Number of elements in the master table. */
    INT fs,            /*!< Sampling frequency (of the SBR part). */
    UCHAR *
        *freqBandTable, /*!< Frequency band table for low-res and high-res. */
    INT *nSfb,          /*!< Number of frequency bands (hig-res and low-res). */
    INT noQmfChannels   /*!< Number of QMF channels. */
) {
  /* Reset the patch.*/
  hTonCorr->guard = 0;
  hTonCorr->shiftStartSb = 1;

  if (resetPatch(hTonCorr, xposctrl, highBandStartSb, v_k_master, numMaster, fs,
                 noQmfChannels))
    return (1);

  /* Reset the noise floor estimate.*/
  if (FDKsbrEnc_resetSbrNoiseFloorEstimate(&hTonCorr->sbrNoiseFloorEstimate,
                                           freqBandTable[LO], nSfb[LO]))
    return (1);

  /*
  Reset the inveerse filtereing detector.
  */
  if (FDKsbrEnc_resetInvFiltDetector(
          &hTonCorr->sbrInvFilt,
          hTonCorr->sbrNoiseFloorEstimate.freqBandTableQmf,
          hTonCorr->sbrNoiseFloorEstimate.noNoiseBands))
    return (1);
  /* Reset the missing harmonics detector. */
  if (FDKsbrEnc_ResetSbrMissingHarmonicsDetector(
          &hTonCorr->sbrMissingHarmonicsDetector, nSfb[HI]))
    return (1);

  return (0);
}

/**************************************************************************/
/*!
  \brief  Deletes the tonality correction paramtere module.



  \return   none

*/
/**************************************************************************/
void FDKsbrEnc_DeleteTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr) /*!< Handle to SBR_TON_CORR struct. */
{
  if (hTonCorr) {
    FreeRam_Sbr_quotaMatrix(hTonCorr->quotaMatrix);

    FreeRam_Sbr_signMatrix(hTonCorr->signMatrix);

    FDKsbrEnc_DeleteSbrMissingHarmonicsDetector(
        &hTonCorr->sbrMissingHarmonicsDetector);
  }
}
