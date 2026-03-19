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

#include "invf_est.h"
#include "sbr_misc.h"

#include "genericStds.h"

#define MAX_NUM_REGIONS 10
#define SCALE_FAC_QUO 512.0f
#define SCALE_FAC_NRG 256.0f

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

static const FIXP_DBL quantStepsSbr[4] = {
    0x00400000, 0x02800000, 0x03800000,
    0x04c00000}; /* table scaled with SCALE_FAC_QUO */
static const FIXP_DBL quantStepsOrig[4] = {
    0x00000000, 0x00c00000, 0x01c00000,
    0x02800000}; /* table scaled with SCALE_FAC_QUO */
static const FIXP_DBL nrgBorders[4] = {
    0x0c800000, 0x0f000000, 0x11800000,
    0x14000000}; /* table scaled with SCALE_FAC_NRG */

static const DETECTOR_PARAMETERS detectorParamsAAC = {
    quantStepsSbr,
    quantStepsOrig,
    nrgBorders,
    4, /* Number of borders SBR. */
    4, /* Number of borders orig. */
    4, /* Number of borders Nrg. */
    {
        /* Region space. */
        {INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /* regionSbr */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF} /*    |      */
    }, /*------------------------ regionOrig ---------------------------------*/
    {
        /* Region space transient. */
        {INVF_LOW_LEVEL, INVF_LOW_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_LOW_LEVEL, INVF_LOW_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /* regionSbr */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF} /*    |      */
    }, /*------------------------ regionOrig ---------------------------------*/
    {-4, -3, -2, -1,
     0} /* Reduction factor of the inverse filtering for low energies.*/
};

static const FIXP_DBL hysteresis =
    0x00400000; /* Delta value for hysteresis. scaled with SCALE_FAC_QUO */

/*
 * AAC+SBR PARAMETERS for Speech
 *********************************/
static const DETECTOR_PARAMETERS detectorParamsAACSpeech = {
    quantStepsSbr,
    quantStepsOrig,
    nrgBorders,
    4, /* Number of borders SBR. */
    4, /* Number of borders orig. */
    4, /* Number of borders Nrg. */
    {
        /* Region space. */
        {INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /* regionSbr */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF} /*    |      */
    }, /*------------------------ regionOrig ---------------------------------*/
    {
        /* Region space transient. */
        {INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_LOW_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /* regionSbr */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF}, /*    |      */
        {INVF_HIGH_LEVEL, INVF_HIGH_LEVEL, INVF_MID_LEVEL, INVF_OFF,
         INVF_OFF} /*    |      */
    }, /*------------------------ regionOrig ---------------------------------*/
    {-4, -3, -2, -1,
     0} /* Reduction factor of the inverse filtering for low energies.*/
};

/*
 * Smoothing filters.
 ************************/
typedef const FIXP_DBL FIR_FILTER[5];

static const FIR_FILTER fir_0 = {0x7fffffff, 0x00000000, 0x00000000, 0x00000000,
                                 0x00000000};
static const FIR_FILTER fir_1 = {0x2aaaaa80, 0x555554ff, 0x00000000, 0x00000000,
                                 0x00000000};
static const FIR_FILTER fir_2 = {0x10000000, 0x30000000, 0x40000000, 0x00000000,
                                 0x00000000};
static const FIR_FILTER fir_3 = {0x077f80e8, 0x199999a0, 0x2bb3b240, 0x33333340,
                                 0x00000000};
static const FIR_FILTER fir_4 = {0x04130598, 0x0ebdb000, 0x1becfa60, 0x2697a4c0,
                                 0x2aaaaa80};

static const FIR_FILTER *const fir_table[5] = {&fir_0, &fir_1, &fir_2, &fir_3,
                                               &fir_4};

/**************************************************************************/
/*!
  \brief     Calculates the values used for the detector.


  \return    none

*/
/**************************************************************************/
static void calculateDetectorValues(
    FIXP_DBL **quotaMatrixOrig, /*!< Matrix holding the tonality values of the
                                   original. */
    SCHAR *indexVector,         /*!< Index vector to obtain the patched data. */
    FIXP_DBL *nrgVector,        /*!< Energy vector. */
    DETECTOR_VALUES *detectorValues, /*!< pointer to DETECTOR_VALUES struct. */
    INT startChannel,                /*!< Start channel. */
    INT stopChannel,                 /*!< Stop channel. */
    INT startIndex,                  /*!< Start index. */
    INT stopIndex,                   /*!< Stop index. */
    INT numberOfStrongest /*!< The number of sorted tonal components to be
                             considered. */
) {
  INT i, temp, j;

  const FIXP_DBL *filter = *fir_table[INVF_SMOOTHING_LENGTH];
  FIXP_DBL origQuotaMeanStrongest, sbrQuotaMeanStrongest;
  FIXP_DBL origQuota, sbrQuota;
  FIXP_DBL invIndex, invChannel, invTemp;
  FIXP_DBL quotaVecOrig[64], quotaVecSbr[64];

  FDKmemclear(quotaVecOrig, 64 * sizeof(FIXP_DBL));
  FDKmemclear(quotaVecSbr, 64 * sizeof(FIXP_DBL));

  invIndex = GetInvInt(stopIndex - startIndex);
  invChannel = GetInvInt(stopChannel - startChannel);

  /*
   Calculate the mean value, over the current time segment, for the original,
   the HFR and the difference, over all channels in the current frequency range.
   NOTE: the averaging is done on the values quota/(1 - quota + RELAXATION).
   */

  /* The original, the sbr signal and the total energy */
  detectorValues->avgNrg = FL2FXCONST_DBL(0.0f);
  for (j = startIndex; j < stopIndex; j++) {
    for (i = startChannel; i < stopChannel; i++) {
      quotaVecOrig[i] += fMult(quotaMatrixOrig[j][i], invIndex);

      if (indexVector[i] != -1)
        quotaVecSbr[i] += fMult(quotaMatrixOrig[j][indexVector[i]], invIndex);
    }
    detectorValues->avgNrg += fMult(nrgVector[j], invIndex);
  }

  /*
   Calculate the mean value, over the current frequency range, for the original,
   the HFR and the difference. Also calculate the same mean values for the three
   vectors, but only includeing the x strongest copmponents.
   */

  origQuota = FL2FXCONST_DBL(0.0f);
  sbrQuota = FL2FXCONST_DBL(0.0f);
  for (i = startChannel; i < stopChannel; i++) {
    origQuota += fMultDiv2(quotaVecOrig[i], invChannel);
    sbrQuota += fMultDiv2(quotaVecSbr[i], invChannel);
  }

  /*
   Calculate the mean value for the x strongest components
  */
  FDKsbrEnc_Shellsort_fract(quotaVecOrig + startChannel,
                            stopChannel - startChannel);
  FDKsbrEnc_Shellsort_fract(quotaVecSbr + startChannel,
                            stopChannel - startChannel);

  origQuotaMeanStrongest = FL2FXCONST_DBL(0.0f);
  sbrQuotaMeanStrongest = FL2FXCONST_DBL(0.0f);

  temp = min(stopChannel - startChannel, numberOfStrongest);
  invTemp = GetInvInt(temp);

  for (i = 0; i < temp; i++) {
    origQuotaMeanStrongest +=
        fMultDiv2(quotaVecOrig[i + stopChannel - temp], invTemp);
    sbrQuotaMeanStrongest +=
        fMultDiv2(quotaVecSbr[i + stopChannel - temp], invTemp);
  }

  /*
   The value for the strongest component
  */
  detectorValues->origQuotaMax = quotaVecOrig[stopChannel - 1];
  detectorValues->sbrQuotaMax = quotaVecSbr[stopChannel - 1];

  /*
   Buffer values
  */
  FDKmemmove(detectorValues->origQuotaMean, detectorValues->origQuotaMean + 1,
             INVF_SMOOTHING_LENGTH * sizeof(FIXP_DBL));
  FDKmemmove(detectorValues->sbrQuotaMean, detectorValues->sbrQuotaMean + 1,
             INVF_SMOOTHING_LENGTH * sizeof(FIXP_DBL));
  FDKmemmove(detectorValues->origQuotaMeanStrongest,
             detectorValues->origQuotaMeanStrongest + 1,
             INVF_SMOOTHING_LENGTH * sizeof(FIXP_DBL));
  FDKmemmove(detectorValues->sbrQuotaMeanStrongest,
             detectorValues->sbrQuotaMeanStrongest + 1,
             INVF_SMOOTHING_LENGTH * sizeof(FIXP_DBL));

  detectorValues->origQuotaMean[INVF_SMOOTHING_LENGTH] = origQuota << 1;
  detectorValues->sbrQuotaMean[INVF_SMOOTHING_LENGTH] = sbrQuota << 1;
  detectorValues->origQuotaMeanStrongest[INVF_SMOOTHING_LENGTH] =
      origQuotaMeanStrongest << 1;
  detectorValues->sbrQuotaMeanStrongest[INVF_SMOOTHING_LENGTH] =
      sbrQuotaMeanStrongest << 1;

  /*
   Filter values
  */
  detectorValues->origQuotaMeanFilt = FL2FXCONST_DBL(0.0f);
  detectorValues->sbrQuotaMeanFilt = FL2FXCONST_DBL(0.0f);
  detectorValues->origQuotaMeanStrongestFilt = FL2FXCONST_DBL(0.0f);
  detectorValues->sbrQuotaMeanStrongestFilt = FL2FXCONST_DBL(0.0f);

  for (i = 0; i < INVF_SMOOTHING_LENGTH + 1; i++) {
    detectorValues->origQuotaMeanFilt +=
        fMult(detectorValues->origQuotaMean[i], filter[i]);
    detectorValues->sbrQuotaMeanFilt +=
        fMult(detectorValues->sbrQuotaMean[i], filter[i]);
    detectorValues->origQuotaMeanStrongestFilt +=
        fMult(detectorValues->origQuotaMeanStrongest[i], filter[i]);
    detectorValues->sbrQuotaMeanStrongestFilt +=
        fMult(detectorValues->sbrQuotaMeanStrongest[i], filter[i]);
  }
}

/**************************************************************************/
/*!
  \brief     Returns the region in which the input value belongs.



  \return    region.

*/
/**************************************************************************/
static INT findRegion(
    FIXP_DBL currVal,        /*!< The current value. */
    const FIXP_DBL *borders, /*!< The border of the regions. */
    const INT numBorders     /*!< The number of borders. */
) {
  INT i;

  if (currVal < borders[0]) {
    return 0;
  }

  for (i = 1; i < numBorders; i++) {
    if (currVal >= borders[i - 1] && currVal < borders[i]) {
      return i;
    }
  }

  if (currVal >= borders[numBorders - 1]) {
    return numBorders;
  }

  return 0; /* We never get here, it's just to avoid compiler warnings.*/
}

/**************************************************************************/
/*!
  \brief     Makes a clever decision based on the quota vector.


  \return     decision on which invf mode to use

*/
/**************************************************************************/
static INVF_MODE decisionAlgorithm(
    const DETECTOR_PARAMETERS
        *detectorParams, /*!< Struct with the detector parameters. */
    DETECTOR_VALUES *detectorValues, /*!< Struct with the detector values. */
    INT transientFlag,  /*!< Flag indicating if there is a transient present.*/
    INT *prevRegionSbr, /*!< The previous region in which the Sbr value was. */
    INT *prevRegionOrig /*!< The previous region in which the Orig value was. */
) {
  INT invFiltLevel, regionSbr, regionOrig, regionNrg;

  /*
   Current thresholds.
   */
  const INT numRegionsSbr = detectorParams->numRegionsSbr;
  const INT numRegionsOrig = detectorParams->numRegionsOrig;
  const INT numRegionsNrg = detectorParams->numRegionsNrg;

  FIXP_DBL quantStepsSbrTmp[MAX_NUM_REGIONS];
  FIXP_DBL quantStepsOrigTmp[MAX_NUM_REGIONS];

  /*
   Current detector values.
   */
  FIXP_DBL origQuotaMeanFilt;
  FIXP_DBL sbrQuotaMeanFilt;
  FIXP_DBL nrg;

  /* 0.375 = 3.0 / 8.0; 0.31143075889 = log2(RELAXATION)/64.0; 0.625 =
   * log(16)/64.0; 0.6875 = 44/64.0 */
  origQuotaMeanFilt =
      (fMultDiv2(FL2FXCONST_DBL(2.f * 0.375f),
                 (FIXP_DBL)(CalcLdData(max(detectorValues->origQuotaMeanFilt,
                                           (FIXP_DBL)1)) +
                            FL2FXCONST_DBL(0.31143075889f))))
      << 0; /* scaled by 1/2^9 */
  sbrQuotaMeanFilt =
      (fMultDiv2(FL2FXCONST_DBL(2.f * 0.375f),
                 (FIXP_DBL)(CalcLdData(max(detectorValues->sbrQuotaMeanFilt,
                                           (FIXP_DBL)1)) +
                            FL2FXCONST_DBL(0.31143075889f))))
      << 0; /* scaled by 1/2^9 */
  /* If energy is zero then we will get different results for different word
   * lengths. */
  nrg =
      (fMultDiv2(FL2FXCONST_DBL(2.f * 0.375f),
                 (FIXP_DBL)(CalcLdData(detectorValues->avgNrg + (FIXP_DBL)1) +
                            FL2FXCONST_DBL(0.0625f) + FL2FXCONST_DBL(0.6875f))))
      << 0; /* scaled by 1/2^8; 2^44 -> qmf energy scale */

  FDKmemcpy(quantStepsSbrTmp, detectorParams->quantStepsSbr,
            numRegionsSbr * sizeof(FIXP_DBL));
  FDKmemcpy(quantStepsOrigTmp, detectorParams->quantStepsOrig,
            numRegionsOrig * sizeof(FIXP_DBL));

  if (*prevRegionSbr < numRegionsSbr)
    quantStepsSbrTmp[*prevRegionSbr] =
        detectorParams->quantStepsSbr[*prevRegionSbr] + hysteresis;
  if (*prevRegionSbr > 0)
    quantStepsSbrTmp[*prevRegionSbr - 1] =
        detectorParams->quantStepsSbr[*prevRegionSbr - 1] - hysteresis;

  if (*prevRegionOrig < numRegionsOrig)
    quantStepsOrigTmp[*prevRegionOrig] =
        detectorParams->quantStepsOrig[*prevRegionOrig] + hysteresis;
  if (*prevRegionOrig > 0)
    quantStepsOrigTmp[*prevRegionOrig - 1] =
        detectorParams->quantStepsOrig[*prevRegionOrig - 1] - hysteresis;

  regionSbr = findRegion(sbrQuotaMeanFilt, quantStepsSbrTmp, numRegionsSbr);
  regionOrig = findRegion(origQuotaMeanFilt, quantStepsOrigTmp, numRegionsOrig);
  regionNrg = findRegion(nrg, detectorParams->nrgBorders, numRegionsNrg);

  *prevRegionSbr = regionSbr;
  *prevRegionOrig = regionOrig;

  /* Use different settings if a transient is present*/
  invFiltLevel =
      (transientFlag == 1)
          ? detectorParams->regionSpaceTransient[regionSbr][regionOrig]
          : detectorParams->regionSpace[regionSbr][regionOrig];

  /* Compensate for low energy.*/
  invFiltLevel =
      max(invFiltLevel + detectorParams->EnergyCompFactor[regionNrg], 0);

  return (INVF_MODE)(invFiltLevel);
}

/**************************************************************************/
/*!
  \brief     Estiamtion of the inverse filtering level required
             in the decoder.

   A second order LPC is calculated for every filterbank channel, using
   the covariance method. THe ratio between the energy of the predicted
   signal and the energy of the non-predictable signal is calcualted.

  \return    none.

*/
/**************************************************************************/
void FDKsbrEnc_qmfInverseFilteringDetector(
    HANDLE_SBR_INV_FILT_EST
        hInvFilt,           /*!< Handle to the SBR_INV_FILT_EST struct. */
    FIXP_DBL **quotaMatrix, /*!< The matrix holding the tonality values of the
                               original. */
    FIXP_DBL *nrgVector,    /*!< The energy vector. */
    SCHAR *indexVector,     /*!< Index vector to obtain the patched data. */
    INT startIndex,         /*!< Start index. */
    INT stopIndex,          /*!< Stop index. */
    INT transientFlag, /*!< Flag indicating if a transient is present or not.*/
    INVF_MODE *infVec  /*!< Vector holding the inverse filtering levels. */
) {
  INT band;

  /*
   * Do the inverse filtering level estimation.
   *****************************************************/
  for (band = 0; band < hInvFilt->noDetectorBands; band++) {
    INT startChannel = hInvFilt->freqBandTableInvFilt[band];
    INT stopChannel = hInvFilt->freqBandTableInvFilt[band + 1];

    calculateDetectorValues(quotaMatrix, indexVector, nrgVector,
                            &hInvFilt->detectorValues[band], startChannel,
                            stopChannel, startIndex, stopIndex,
                            hInvFilt->numberOfStrongest);

    infVec[band] = decisionAlgorithm(
        hInvFilt->detectorParams, &hInvFilt->detectorValues[band],
        transientFlag, &hInvFilt->prevRegionSbr[band],
        &hInvFilt->prevRegionOrig[band]);
  }
}

/**************************************************************************/
/*!
  \brief     Initialize an instance of the inverse filtering level estimator.


  \return   errorCode, noError if successful.

*/
/**************************************************************************/
INT FDKsbrEnc_initInvFiltDetector(
    HANDLE_SBR_INV_FILT_EST
        hInvFilt, /*!< Pointer to a handle to the SBR_INV_FILT_EST struct. */
    INT *freqBandTableDetector, /*!< Frequency band table for the inverse
                                   filtering. */
    INT numDetectorBands,       /*!< Number of inverse filtering bands. */
    UINT
        useSpeechConfig /*!< Flag: adapt tuning parameters according to speech*/
) {
  INT i;

  FDKmemclear(hInvFilt, sizeof(SBR_INV_FILT_EST));

  hInvFilt->detectorParams =
      (useSpeechConfig) ? &detectorParamsAACSpeech : &detectorParamsAAC;

  hInvFilt->noDetectorBandsMax = numDetectorBands;

  /*
     Memory initialisation
  */
  for (i = 0; i < hInvFilt->noDetectorBandsMax; i++) {
    FDKmemclear(&hInvFilt->detectorValues[i], sizeof(DETECTOR_VALUES));
    hInvFilt->prevInvfMode[i] = INVF_OFF;
    hInvFilt->prevRegionOrig[i] = 0;
    hInvFilt->prevRegionSbr[i] = 0;
  }

  /*
  Reset the inverse fltering detector.
  */
  FDKsbrEnc_resetInvFiltDetector(hInvFilt, freqBandTableDetector,
                                 hInvFilt->noDetectorBandsMax);

  return (0);
}

/**************************************************************************/
/*!
  \brief     resets sbr inverse filtering structure.



  \return   errorCode, noError if successful.

*/
/**************************************************************************/
INT FDKsbrEnc_resetInvFiltDetector(
    HANDLE_SBR_INV_FILT_EST
        hInvFilt,               /*!< Handle to the SBR_INV_FILT_EST struct. */
    INT *freqBandTableDetector, /*!< Frequency band table for the inverse
                                   filtering. */
    INT numDetectorBands)       /*!< Number of inverse filtering bands. */
{
  hInvFilt->numberOfStrongest = 1;
  FDKmemcpy(hInvFilt->freqBandTableInvFilt, freqBandTableDetector,
            (numDetectorBands + 1) * sizeof(INT));
  hInvFilt->noDetectorBands = numDetectorBands;

  return (0);
}
