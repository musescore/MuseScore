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

/**************************** AAC encoder library ******************************

   Author(s):   A. Horndasch (code originally from lwr) / Josef Hoepfl (FDK)

   Description: intensity stereo processing

*******************************************************************************/

#include "intensity.h"

#include "interface.h"
#include "psy_configuration.h"
#include "psy_const.h"
#include "qc_main.h"
#include "bit_cnt.h"

/* only set an IS seed it left/right channel correlation is above IS_CORR_THRESH
 */
#define IS_CORR_THRESH FL2FXCONST_DBL(0.95f)

/* when expanding the IS region to more SFBs only accept an error that is
 * not more than IS_TOTAL_ERROR_THRESH overall and
 * not more than IS_LOCAL_ERROR_THRESH for the current SFB */
#define IS_TOTAL_ERROR_THRESH FL2FXCONST_DBL(0.04f)
#define IS_LOCAL_ERROR_THRESH FL2FXCONST_DBL(0.01f)

/* the maximum allowed change of the intensity direction (unit: IS scale) -
 * scaled with factor 0.25 - */
#define IS_DIRECTION_DEVIATION_THRESH_SF 2
#define IS_DIRECTION_DEVIATION_THRESH \
  FL2FXCONST_DBL(2.0f / (1 << IS_DIRECTION_DEVIATION_THRESH_SF))

/* IS regions need to have a minimal percentage of the overall loudness, e.g.
 * 0.06 == 6% */
#define IS_REGION_MIN_LOUDNESS FL2FXCONST_DBL(0.1f)

/* only perform IS if IS_MIN_SFBS neighboring SFBs can be processed */
#define IS_MIN_SFBS 6

/* only do IS if
 * if IS_LEFT_RIGHT_RATIO_THRESH < sfbEnergyLeft[sfb]/sfbEnergyRight[sfb] < 1 /
 * IS_LEFT_RIGHT_RATIO_THRESH
 * -> no IS if the panning angle is not far from the middle, MS will do */
/* this is equivalent to a scale of +/-1.02914634566 */
#define IS_LEFT_RIGHT_RATIO_THRESH FL2FXCONST_DBL(0.7f)

/* scalefactor of realScale */
#define REAL_SCALE_SF 1

/* scalefactor overallLoudness */
#define OVERALL_LOUDNESS_SF 6

/* scalefactor for sum over max samples per goup */
#define MAX_SFB_PER_GROUP_SF 6

/* scalefactor for sum of mdct spectrum */
#define MDCT_SPEC_SF 6

typedef struct {
  FIXP_DBL corr_thresh; /*!< Only set an IS seed it left/right channel
                           correlation is above corr_thresh */

  FIXP_DBL total_error_thresh; /*!< When expanding the IS region to more SFBs
                                  only accept an error that is not more than
                                  'total_error_thresh' overall. */

  FIXP_DBL local_error_thresh; /*!< When expanding the IS region to more SFBs
                                  only accept an error that is not more than
                                  'local_error_thresh' for the current SFB. */

  FIXP_DBL direction_deviation_thresh; /*!< The maximum allowed change of the
                                          intensity direction (unit: IS scale)
                                        */

  FIXP_DBL is_region_min_loudness; /*!< IS regions need to have a minimal
                                      percentage of the overall loudness, e.g.
                                      0.06 == 6% */

  INT min_is_sfbs; /*!< Only perform IS if 'min_is_sfbs' neighboring SFBs can be
                      processed */

  FIXP_DBL left_right_ratio_threshold; /*!< No IS if the panning angle is not
                                          far from the middle, MS will do */

} INTENSITY_PARAMETERS;

/*****************************************************************************

    functionname: calcSfbMaxScale

    description:  Calc max value in scalefactor band

    input:        *mdctSpectrum
                   l1
                   l2

    output:       none

    returns:      scalefactor

*****************************************************************************/
static INT calcSfbMaxScale(const FIXP_DBL *mdctSpectrum, const INT l1,
                           const INT l2) {
  INT i;
  INT sfbMaxScale;
  FIXP_DBL maxSpc;

  maxSpc = FL2FXCONST_DBL(0.0);
  for (i = l1; i < l2; i++) {
    FIXP_DBL tmp = fixp_abs((FIXP_DBL)mdctSpectrum[i]);
    maxSpc = fixMax(maxSpc, tmp);
  }
  sfbMaxScale = (maxSpc == FL2FXCONST_DBL(0.0)) ? (DFRACT_BITS - 2)
                                                : CntLeadingZeros(maxSpc) - 1;

  return sfbMaxScale;
}

/*****************************************************************************

    functionname: FDKaacEnc_initIsParams

    description:  Initialization of intensity parameters

    input:        isParams

    output:       isParams

    returns:      none

*****************************************************************************/
static void FDKaacEnc_initIsParams(INTENSITY_PARAMETERS *isParams) {
  isParams->corr_thresh = IS_CORR_THRESH;
  isParams->total_error_thresh = IS_TOTAL_ERROR_THRESH;
  isParams->local_error_thresh = IS_LOCAL_ERROR_THRESH;
  isParams->direction_deviation_thresh = IS_DIRECTION_DEVIATION_THRESH;
  isParams->is_region_min_loudness = IS_REGION_MIN_LOUDNESS;
  isParams->min_is_sfbs = IS_MIN_SFBS;
  isParams->left_right_ratio_threshold = IS_LEFT_RIGHT_RATIO_THRESH;
}

/*****************************************************************************

    functionname: FDKaacEnc_prepareIntensityDecision

    description:  Prepares intensity decision

    input:        sfbEnergyLeft
                  sfbEnergyRight
                  sfbEnergyLdDataLeft
                  sfbEnergyLdDataRight
                  mdctSpectrumLeft
                  sfbEnergyLdDataRight
                  isParams

    output:       hrrErr            scale: none
                  isMask            scale: none
                  realScale         scale: LD_DATA_SHIFT + REAL_SCALE_SF
                  normSfbLoudness   scale: none

    returns:      none

*****************************************************************************/
static void FDKaacEnc_prepareIntensityDecision(
    const FIXP_DBL *sfbEnergyLeft, const FIXP_DBL *sfbEnergyRight,
    const FIXP_DBL *sfbEnergyLdDataLeft, const FIXP_DBL *sfbEnergyLdDataRight,
    const FIXP_DBL *mdctSpectrumLeft, const FIXP_DBL *mdctSpectrumRight,
    const INTENSITY_PARAMETERS *isParams, FIXP_DBL *hrrErr, INT *isMask,
    FIXP_DBL *realScale, FIXP_DBL *normSfbLoudness, const INT sfbCnt,
    const INT sfbPerGroup, const INT maxSfbPerGroup, const INT *sfbOffset) {
  INT j, sfb, sfboffs;
  INT grpCounter;

  /* temporary variables to compute loudness */
  FIXP_DBL overallLoudness[MAX_NO_OF_GROUPS];

  /* temporary variables to compute correlation */
  FIXP_DBL channelCorr[MAX_GROUPED_SFB];
  FIXP_DBL ml, mr;
  FIXP_DBL prod_lr;
  FIXP_DBL square_l, square_r;
  FIXP_DBL tmp_l, tmp_r;
  FIXP_DBL inv_n;

  FDKmemclear(channelCorr, MAX_GROUPED_SFB * sizeof(FIXP_DBL));
  FDKmemclear(normSfbLoudness, MAX_GROUPED_SFB * sizeof(FIXP_DBL));
  FDKmemclear(overallLoudness, MAX_NO_OF_GROUPS * sizeof(FIXP_DBL));
  FDKmemclear(realScale, MAX_GROUPED_SFB * sizeof(FIXP_DBL));

  for (grpCounter = 0, sfboffs = 0; sfboffs < sfbCnt;
       sfboffs += sfbPerGroup, grpCounter++) {
    overallLoudness[grpCounter] = FL2FXCONST_DBL(0.0f);
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      INT sL, sR, s;
      FIXP_DBL isValue = sfbEnergyLdDataLeft[sfb + sfboffs] -
                         sfbEnergyLdDataRight[sfb + sfboffs];

      /* delimitate intensity scale value to representable range */
      realScale[sfb + sfboffs] = fixMin(
          FL2FXCONST_DBL(60.f / (1 << (REAL_SCALE_SF + LD_DATA_SHIFT))),
          fixMax(FL2FXCONST_DBL(-60.f / (1 << (REAL_SCALE_SF + LD_DATA_SHIFT))),
                 isValue));

      sL = fixMax(0, (CntLeadingZeros(sfbEnergyLeft[sfb + sfboffs]) - 1));
      sR = fixMax(0, (CntLeadingZeros(sfbEnergyRight[sfb + sfboffs]) - 1));
      s = (fixMin(sL, sR) >> 2) << 2;
      normSfbLoudness[sfb + sfboffs] =
          sqrtFixp(sqrtFixp(((sfbEnergyLeft[sfb + sfboffs] << s) >> 1) +
                            ((sfbEnergyRight[sfb + sfboffs] << s) >> 1))) >>
          (s >> 2);

      overallLoudness[grpCounter] +=
          normSfbLoudness[sfb + sfboffs] >> OVERALL_LOUDNESS_SF;
      /* don't do intensity if
       * - panning angle is too close to the middle or
       * - one channel is non-existent or
       * - if it is dual mono */
      if ((sfbEnergyLeft[sfb + sfboffs] >=
           fMult(isParams->left_right_ratio_threshold,
                 sfbEnergyRight[sfb + sfboffs])) &&
          (fMult(isParams->left_right_ratio_threshold,
                 sfbEnergyLeft[sfb + sfboffs]) <=
           sfbEnergyRight[sfb + sfboffs])) {
        /* this will prevent post processing from considering this SFB for
         * merging */
        hrrErr[sfb + sfboffs] = FL2FXCONST_DBL(1.0 / 8.0);
      }
    }
  }

  for (grpCounter = 0, sfboffs = 0; sfboffs < sfbCnt;
       sfboffs += sfbPerGroup, grpCounter++) {
    INT invOverallLoudnessSF;
    FIXP_DBL invOverallLoudness;

    if (overallLoudness[grpCounter] == FL2FXCONST_DBL(0.0)) {
      invOverallLoudness = FL2FXCONST_DBL(0.0);
      invOverallLoudnessSF = 0;
    } else {
      invOverallLoudness =
          fDivNorm((FIXP_DBL)MAXVAL_DBL, overallLoudness[grpCounter],
                   &invOverallLoudnessSF);
      invOverallLoudnessSF =
          invOverallLoudnessSF - OVERALL_LOUDNESS_SF +
          1; /* +1: compensate fMultDiv2() in subsequent loop */
    }
    invOverallLoudnessSF = fixMin(
        fixMax(invOverallLoudnessSF, -(DFRACT_BITS - 1)), DFRACT_BITS - 1);

    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      FIXP_DBL tmp;

      tmp = fMultDiv2((normSfbLoudness[sfb + sfboffs] >> OVERALL_LOUDNESS_SF)
                          << OVERALL_LOUDNESS_SF,
                      invOverallLoudness);

      normSfbLoudness[sfb + sfboffs] = scaleValue(tmp, invOverallLoudnessSF);

      channelCorr[sfb + sfboffs] = FL2FXCONST_DBL(0.0f);

      /* max width of scalefactorband is 96; width's are always even */
      /* inv_n is scaled with factor 2 to compensate fMultDiv2() in subsequent
       * loops */
      inv_n = GetInvInt(
          (sfbOffset[sfb + sfboffs + 1] - sfbOffset[sfb + sfboffs]) >> 1);

      if (inv_n > FL2FXCONST_DBL(0.0f)) {
        INT s, sL, sR;

        /* correlation := Pearson's product-moment coefficient */
        /* compute correlation between channels and check if it is over
         * threshold */
        ml = FL2FXCONST_DBL(0.0f);
        mr = FL2FXCONST_DBL(0.0f);
        prod_lr = FL2FXCONST_DBL(0.0f);
        square_l = FL2FXCONST_DBL(0.0f);
        square_r = FL2FXCONST_DBL(0.0f);

        sL = calcSfbMaxScale(mdctSpectrumLeft, sfbOffset[sfb + sfboffs],
                             sfbOffset[sfb + sfboffs + 1]);
        sR = calcSfbMaxScale(mdctSpectrumRight, sfbOffset[sfb + sfboffs],
                             sfbOffset[sfb + sfboffs + 1]);
        s = fixMin(sL, sR);

        for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
             j++) {
          ml += fMultDiv2((mdctSpectrumLeft[j] << s),
                          inv_n);  // scaled with mdctScale - s + inv_n
          mr += fMultDiv2((mdctSpectrumRight[j] << s),
                          inv_n);  // scaled with mdctScale - s + inv_n
        }
        ml = fMultDiv2(ml, inv_n);  // scaled with mdctScale - s + inv_n
        mr = fMultDiv2(mr, inv_n);  // scaled with mdctScale - s + inv_n

        for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
             j++) {
          tmp_l = fMultDiv2((mdctSpectrumLeft[j] << s), inv_n) -
                  ml;  // scaled with mdctScale - s + inv_n
          tmp_r = fMultDiv2((mdctSpectrumRight[j] << s), inv_n) -
                  mr;  // scaled with mdctScale - s + inv_n

          prod_lr += fMultDiv2(
              tmp_l, tmp_r);  // scaled with 2*(mdctScale - s + inv_n) + 1
          square_l +=
              fPow2Div2(tmp_l);  // scaled with 2*(mdctScale - s + inv_n) + 1
          square_r +=
              fPow2Div2(tmp_r);  // scaled with 2*(mdctScale - s + inv_n) + 1
        }
        prod_lr = prod_lr << 1;    // scaled with 2*(mdctScale - s + inv_n)
        square_l = square_l << 1;  // scaled with 2*(mdctScale - s + inv_n)
        square_r = square_r << 1;  // scaled with 2*(mdctScale - s + inv_n)

        if (square_l > FL2FXCONST_DBL(0.0f) &&
            square_r > FL2FXCONST_DBL(0.0f)) {
          INT channelCorrSF = 0;

          /* local scaling of square_l and square_r is compensated after sqrt
           * calculation */
          sL = fixMax(0, (CntLeadingZeros(square_l) - 1));
          sR = fixMax(0, (CntLeadingZeros(square_r) - 1));
          s = ((sL + sR) >> 1) << 1;
          sL = fixMin(sL, s);
          sR = s - sL;
          tmp = fMult(square_l << sL, square_r << sR);
          tmp = sqrtFixp(tmp);

          FDK_ASSERT(tmp > FL2FXCONST_DBL(0.0f));

          /* numerator and denominator have the same scaling */
          if (prod_lr < FL2FXCONST_DBL(0.0f)) {
            channelCorr[sfb + sfboffs] =
                -(fDivNorm(-prod_lr, tmp, &channelCorrSF));

          } else {
            channelCorr[sfb + sfboffs] =
                (fDivNorm(prod_lr, tmp, &channelCorrSF));
          }
          channelCorrSF = fixMin(
              fixMax((channelCorrSF + ((sL + sR) >> 1)), -(DFRACT_BITS - 1)),
              DFRACT_BITS - 1);

          if (channelCorrSF < 0) {
            channelCorr[sfb + sfboffs] =
                channelCorr[sfb + sfboffs] >> (-channelCorrSF);
          } else {
            /* avoid overflows due to limited computational accuracy */
            if (fAbs(channelCorr[sfb + sfboffs]) >
                (((FIXP_DBL)MAXVAL_DBL) >> channelCorrSF)) {
              if (channelCorr[sfb + sfboffs] < FL2FXCONST_DBL(0.0f))
                channelCorr[sfb + sfboffs] = -(FIXP_DBL)MAXVAL_DBL;
              else
                channelCorr[sfb + sfboffs] = (FIXP_DBL)MAXVAL_DBL;
            } else {
              channelCorr[sfb + sfboffs] = channelCorr[sfb + sfboffs]
                                           << channelCorrSF;
            }
          }
        }
      }

      /* for post processing: hrrErr is the error in terms of (too little)
       * correlation weighted with the loudness of the SFB; SFBs with small
       * hrrErr can be merged */
      if (hrrErr[sfb + sfboffs] == FL2FXCONST_DBL(1.0 / 8.0)) {
        continue;
      }

      hrrErr[sfb + sfboffs] =
          fMultDiv2((FL2FXCONST_DBL(0.25f) - (channelCorr[sfb + sfboffs] >> 2)),
                    normSfbLoudness[sfb + sfboffs]);

      /* set IS mask/vector to 1, if correlation is high enough */
      if (fAbs(channelCorr[sfb + sfboffs]) >= isParams->corr_thresh) {
        isMask[sfb + sfboffs] = 1;
      }
    }
  }
}

/*****************************************************************************

    functionname: FDKaacEnc_finalizeIntensityDecision

    description:  Finalizes intensity decision

    input:        isParams          scale: none
                  hrrErr            scale: none
                  realIsScale       scale: LD_DATA_SHIFT + REAL_SCALE_SF
                  normSfbLoudness   scale: none

    output:       isMask            scale: none

    returns:      none

*****************************************************************************/
static void FDKaacEnc_finalizeIntensityDecision(
    const FIXP_DBL *hrrErr, INT *isMask, const FIXP_DBL *realIsScale,
    const FIXP_DBL *normSfbLoudness, const INTENSITY_PARAMETERS *isParams,
    const INT sfbCnt, const INT sfbPerGroup, const INT maxSfbPerGroup) {
  INT sfb, sfboffs, j;
  FIXP_DBL isScaleLast = FL2FXCONST_DBL(0.0f);
  INT isStartValueFound = 0;

  for (sfboffs = 0; sfboffs < sfbCnt; sfboffs += sfbPerGroup) {
    INT startIsSfb = 0;
    INT inIsBlock = 0;
    INT currentIsSfbCount = 0;
    FIXP_DBL overallHrrError = FL2FXCONST_DBL(0.0f);
    FIXP_DBL isRegionLoudness = FL2FXCONST_DBL(0.0f);

    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      if (isMask[sfboffs + sfb] == 1) {
        if (currentIsSfbCount == 0) {
          startIsSfb = sfboffs + sfb;
        }
        if (isStartValueFound == 0) {
          isScaleLast = realIsScale[sfboffs + sfb];
          isStartValueFound = 1;
        }
        inIsBlock = 1;
        currentIsSfbCount++;
        overallHrrError += hrrErr[sfboffs + sfb] >> (MAX_SFB_PER_GROUP_SF - 3);
        isRegionLoudness +=
            normSfbLoudness[sfboffs + sfb] >> MAX_SFB_PER_GROUP_SF;
      } else {
        /* based on correlation, IS should not be used
         * -> use it anyway, if overall error is below threshold
         *    and if local error does not exceed threshold
         * otherwise: check if there are enough IS SFBs
         */
        if (inIsBlock) {
          overallHrrError +=
              hrrErr[sfboffs + sfb] >> (MAX_SFB_PER_GROUP_SF - 3);
          isRegionLoudness +=
              normSfbLoudness[sfboffs + sfb] >> MAX_SFB_PER_GROUP_SF;

          if ((hrrErr[sfboffs + sfb] < (isParams->local_error_thresh >> 3)) &&
              (overallHrrError <
               (isParams->total_error_thresh >> MAX_SFB_PER_GROUP_SF))) {
            currentIsSfbCount++;
            /* overwrite correlation based decision */
            isMask[sfboffs + sfb] = 1;
          } else {
            inIsBlock = 0;
          }
        }
      }
      /* check for large direction deviation */
      if (inIsBlock) {
        if (fAbs(isScaleLast - realIsScale[sfboffs + sfb]) <
            (isParams->direction_deviation_thresh >>
             (REAL_SCALE_SF + LD_DATA_SHIFT -
              IS_DIRECTION_DEVIATION_THRESH_SF))) {
          isScaleLast = realIsScale[sfboffs + sfb];
        } else {
          isMask[sfboffs + sfb] = 0;
          inIsBlock = 0;
          currentIsSfbCount--;
        }
      }

      if (currentIsSfbCount > 0 && (!inIsBlock || sfb == maxSfbPerGroup - 1)) {
        /* not enough SFBs -> do not use IS */
        if (currentIsSfbCount < isParams->min_is_sfbs ||
            (isRegionLoudness<isParams->is_region_min_loudness>>
             MAX_SFB_PER_GROUP_SF)) {
          for (j = startIsSfb; j <= sfboffs + sfb; j++) {
            isMask[j] = 0;
          }
          isScaleLast = FL2FXCONST_DBL(0.0f);
          isStartValueFound = 0;
          for (j = 0; j < startIsSfb; j++) {
            if (isMask[j] != 0) {
              isScaleLast = realIsScale[j];
              isStartValueFound = 1;
            }
          }
        }
        currentIsSfbCount = 0;
        overallHrrError = FL2FXCONST_DBL(0.0f);
        isRegionLoudness = FL2FXCONST_DBL(0.0f);
      }
    }
  }
}

/*****************************************************************************

    functionname: FDKaacEnc_IntensityStereoProcessing

    description:  Intensity stereo processing tool

    input:        sfbEnergyLeft
                  sfbEnergyRight
                  mdctSpectrumLeft
                  mdctSpectrumRight
                  sfbThresholdLeft
                  sfbThresholdRight
                  sfbSpreadEnLeft
                  sfbSpreadEnRight
                  sfbEnergyLdDataLeft
                  sfbEnergyLdDataRight

    output:       isBook
                  isScale
                  pnsData->pnsFlag
                  msDigest                 zeroed from start to sfbCnt
                  msMask                   zeroed from start to sfbCnt
                  mdctSpectrumRight        zeroed where isBook!=0
                  sfbEnergyRight           zeroed where isBook!=0
                  sfbSpreadEnRight       zeroed where isBook!=0
                  sfbThresholdRight        zeroed where isBook!=0
                  sfbEnergyLdDataRight     FL2FXCONST_DBL(-1.0) where isBook!=0
                  sfbThresholdLdDataRight  FL2FXCONST_DBL(-0.515625f) where
isBook!=0

    returns:      none

*****************************************************************************/
void FDKaacEnc_IntensityStereoProcessing(
    FIXP_DBL *sfbEnergyLeft, FIXP_DBL *sfbEnergyRight,
    FIXP_DBL *mdctSpectrumLeft, FIXP_DBL *mdctSpectrumRight,
    FIXP_DBL *sfbThresholdLeft, FIXP_DBL *sfbThresholdRight,
    FIXP_DBL *sfbThresholdLdDataRight, FIXP_DBL *sfbSpreadEnLeft,
    FIXP_DBL *sfbSpreadEnRight, FIXP_DBL *sfbEnergyLdDataLeft,
    FIXP_DBL *sfbEnergyLdDataRight, INT *msDigest, INT *msMask,
    const INT sfbCnt, const INT sfbPerGroup, const INT maxSfbPerGroup,
    const INT *sfbOffset, const INT allowIS, INT *isBook, INT *isScale,
    PNS_DATA *RESTRICT pnsData[2]) {
  INT sfb, sfboffs, j;
  FIXP_DBL scale;
  FIXP_DBL lr;
  FIXP_DBL hrrErr[MAX_GROUPED_SFB];
  FIXP_DBL normSfbLoudness[MAX_GROUPED_SFB];
  FIXP_DBL realIsScale[MAX_GROUPED_SFB];
  INTENSITY_PARAMETERS isParams;
  INT isMask[MAX_GROUPED_SFB];

  FDKmemclear((void *)isBook, sfbCnt * sizeof(INT));
  FDKmemclear((void *)isMask, sfbCnt * sizeof(INT));
  FDKmemclear((void *)realIsScale, sfbCnt * sizeof(FIXP_DBL));
  FDKmemclear((void *)isScale, sfbCnt * sizeof(INT));
  FDKmemclear((void *)hrrErr, sfbCnt * sizeof(FIXP_DBL));

  if (!allowIS) return;

  FDKaacEnc_initIsParams(&isParams);

  /* compute / set the following values per SFB:
   * - left/right ratio between channels
   * - normalized loudness
   *   + loudness == average of energy in channels to 0.25
   *   + normalization: division by sum of all SFB loudnesses
   * - isMask (is set to 0 if channels are the same or one is 0)
   */
  FDKaacEnc_prepareIntensityDecision(
      sfbEnergyLeft, sfbEnergyRight, sfbEnergyLdDataLeft, sfbEnergyLdDataRight,
      mdctSpectrumLeft, mdctSpectrumRight, &isParams, hrrErr, isMask,
      realIsScale, normSfbLoudness, sfbCnt, sfbPerGroup, maxSfbPerGroup,
      sfbOffset);

  FDKaacEnc_finalizeIntensityDecision(hrrErr, isMask, realIsScale,
                                      normSfbLoudness, &isParams, sfbCnt,
                                      sfbPerGroup, maxSfbPerGroup);

  for (sfb = 0; sfb < sfbCnt; sfb += sfbPerGroup) {
    for (sfboffs = 0; sfboffs < maxSfbPerGroup; sfboffs++) {
      INT sL, sR;
      FIXP_DBL inv_n;
      INT mdct_spec_sf = MDCT_SPEC_SF;

      msMask[sfb + sfboffs] = 0;
      if (isMask[sfb + sfboffs] == 0) {
        continue;
      }

      if ((sfbEnergyLeft[sfb + sfboffs] < sfbThresholdLeft[sfb + sfboffs]) &&
          (fMult(FL2FXCONST_DBL(1.0f / 1.5f), sfbEnergyRight[sfb + sfboffs]) >
           sfbThresholdRight[sfb + sfboffs])) {
        continue;
      }
      /* NEW: if there is a big-enough IS region, switch off PNS */
      if (pnsData[0]) {
        if (pnsData[0]->pnsFlag[sfb + sfboffs]) {
          pnsData[0]->pnsFlag[sfb + sfboffs] = 0;
        }
        if (pnsData[1]->pnsFlag[sfb + sfboffs]) {
          pnsData[1]->pnsFlag[sfb + sfboffs] = 0;
        }
      }

      if (sfbOffset[sfb + sfboffs + 1] - sfbOffset[sfb + sfboffs] >
          1 << mdct_spec_sf) {
        mdct_spec_sf++; /* This is for rare cases where the number of bins in a
                           scale factor band is > 64 */
      }

      inv_n = GetInvInt(
          (sfbOffset[sfb + sfboffs + 1] - sfbOffset[sfb + sfboffs]) >>
          1);  // scaled with 2 to compensate fMultDiv2() in subsequent loop
      sL = calcSfbMaxScale(mdctSpectrumLeft, sfbOffset[sfb + sfboffs],
                           sfbOffset[sfb + sfboffs + 1]);
      sR = calcSfbMaxScale(mdctSpectrumRight, sfbOffset[sfb + sfboffs],
                           sfbOffset[sfb + sfboffs + 1]);

      lr = FL2FXCONST_DBL(0.0f);
      for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1]; j++)
        lr += fMultDiv2(
            fMultDiv2(mdctSpectrumLeft[j] << sL, mdctSpectrumRight[j] << sR),
            inv_n);
      lr = lr << 1;

      if (lr < FL2FXCONST_DBL(0.0f)) {
        /* This means OUT OF phase intensity stereo, cf. standard */
        INT s0, s1, s2;
        FIXP_DBL tmp, d, ed = FL2FXCONST_DBL(0.0f);

        s0 = fixMin(sL, sR);
        for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
             j++) {
          d = ((mdctSpectrumLeft[j] << s0) >> 1) -
              ((mdctSpectrumRight[j] << s0) >> 1);
          ed += fMultDiv2(d, d) >> (mdct_spec_sf - 1);
        }
        msMask[sfb + sfboffs] = 1;
        tmp = fDivNorm(sfbEnergyLeft[sfb + sfboffs], ed, &s1);
        s2 = (s1) + (2 * s0) - 2 - mdct_spec_sf;
        if (s2 & 1) {
          tmp = tmp >> 1;
          s2 = s2 + 1;
        }
        s2 = (s2 >> 1) + 1;  // +1 compensate fMultDiv2() in subsequent loop
        s2 = fixMin(fixMax(s2, -(DFRACT_BITS - 1)), (DFRACT_BITS - 1));
        scale = sqrtFixp(tmp);
        if (s2 < 0) {
          s2 = -s2;
          for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
               j++) {
            mdctSpectrumLeft[j] = (fMultDiv2(mdctSpectrumLeft[j], scale) -
                                   fMultDiv2(mdctSpectrumRight[j], scale)) >>
                                  s2;
            mdctSpectrumRight[j] = FL2FXCONST_DBL(0.0f);
          }
        } else {
          for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
               j++) {
            mdctSpectrumLeft[j] = (fMultDiv2(mdctSpectrumLeft[j], scale) -
                                   fMultDiv2(mdctSpectrumRight[j], scale))
                                  << s2;
            mdctSpectrumRight[j] = FL2FXCONST_DBL(0.0f);
          }
        }
      } else {
        /* This means IN phase intensity stereo, cf. standard */
        INT s0, s1, s2;
        FIXP_DBL tmp, s, es = FL2FXCONST_DBL(0.0f);

        s0 = fixMin(sL, sR);
        for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
             j++) {
          s = ((mdctSpectrumLeft[j] << s0) >> 1) +
              ((mdctSpectrumRight[j] << s0) >> 1);
          es += fMultDiv2(s, s) >>
                (mdct_spec_sf -
                 1);  // scaled 2*(mdctScale - s0 + 1) + mdct_spec_sf
        }
        msMask[sfb + sfboffs] = 0;
        tmp = fDivNorm(sfbEnergyLeft[sfb + sfboffs], es, &s1);
        s2 = (s1) + (2 * s0) - 2 - mdct_spec_sf;
        if (s2 & 1) {
          tmp = tmp >> 1;
          s2 = s2 + 1;
        }
        s2 = (s2 >> 1) + 1;  // +1 compensate fMultDiv2() in subsequent loop
        s2 = fixMin(fixMax(s2, -(DFRACT_BITS - 1)), (DFRACT_BITS - 1));
        scale = sqrtFixp(tmp);
        if (s2 < 0) {
          s2 = -s2;
          for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
               j++) {
            mdctSpectrumLeft[j] = (fMultDiv2(mdctSpectrumLeft[j], scale) +
                                   fMultDiv2(mdctSpectrumRight[j], scale)) >>
                                  s2;
            mdctSpectrumRight[j] = FL2FXCONST_DBL(0.0f);
          }
        } else {
          for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
               j++) {
            mdctSpectrumLeft[j] = (fMultDiv2(mdctSpectrumLeft[j], scale) +
                                   fMultDiv2(mdctSpectrumRight[j], scale))
                                  << s2;
            mdctSpectrumRight[j] = FL2FXCONST_DBL(0.0f);
          }
        }
      }

      isBook[sfb + sfboffs] = CODE_BOOK_IS_IN_PHASE_NO;

      if (realIsScale[sfb + sfboffs] < FL2FXCONST_DBL(0.0f)) {
        isScale[sfb + sfboffs] =
            (INT)(((realIsScale[sfb + sfboffs] >> 1) -
                   FL2FXCONST_DBL(
                       0.5f / (1 << (REAL_SCALE_SF + LD_DATA_SHIFT + 1)))) >>
                  (DFRACT_BITS - 1 - REAL_SCALE_SF - LD_DATA_SHIFT - 1)) +
            1;
      } else {
        isScale[sfb + sfboffs] =
            (INT)(((realIsScale[sfb + sfboffs] >> 1) +
                   FL2FXCONST_DBL(
                       0.5f / (1 << (REAL_SCALE_SF + LD_DATA_SHIFT + 1)))) >>
                  (DFRACT_BITS - 1 - REAL_SCALE_SF - LD_DATA_SHIFT - 1));
      }

      sfbEnergyRight[sfb + sfboffs] = FL2FXCONST_DBL(0.0f);
      sfbEnergyLdDataRight[sfb + sfboffs] = FL2FXCONST_DBL(-1.0f);
      sfbThresholdRight[sfb + sfboffs] = FL2FXCONST_DBL(0.0f);
      sfbThresholdLdDataRight[sfb + sfboffs] = FL2FXCONST_DBL(-0.515625f);
      sfbSpreadEnRight[sfb + sfboffs] = FL2FXCONST_DBL(0.0f);

      *msDigest = MS_SOME;
    }
  }
}
