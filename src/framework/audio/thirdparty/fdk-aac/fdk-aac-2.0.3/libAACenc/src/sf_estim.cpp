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

   Author(s):   M. Werner

   Description: Scale factor estimation

*******************************************************************************/

#include "sf_estim.h"
#include "aacEnc_rom.h"
#include "quantize.h"
#include "bit_cnt.h"

#ifdef __arm__
#endif

#define UPCOUNT_LIMIT 1
#define AS_PE_FAC_SHIFT 7
#define DIST_FAC_SHIFT 3
#define AS_PE_FAC_FLOAT (float)(1 << AS_PE_FAC_SHIFT)
static const INT MAX_SCF_DELTA = 60;

static const FIXP_DBL PE_C1 = FL2FXCONST_DBL(
    3.0f / AS_PE_FAC_FLOAT); /* (log(8.0)/log(2)) >> AS_PE_FAC_SHIFT */
static const FIXP_DBL PE_C2 = FL2FXCONST_DBL(
    1.3219281f / AS_PE_FAC_FLOAT); /* (log(2.5)/log(2)) >> AS_PE_FAC_SHIFT */
static const FIXP_DBL PE_C3 = FL2FXCONST_DBL(0.5593573f); /* 1-C2/C1 */

/*
  Function; FDKaacEnc_FDKaacEnc_CalcFormFactorChannel

  Description: Calculates the formfactor

  sf: scale factor of the mdct spectrum
  sfbFormFactorLdData is scaled with the factor 1/(((2^sf)^0.5) *
  (2^FORM_FAC_SHIFT))
*/
static void FDKaacEnc_FDKaacEnc_CalcFormFactorChannel(
    FIXP_DBL *RESTRICT sfbFormFactorLdData,
    PSY_OUT_CHANNEL *RESTRICT psyOutChan) {
  INT j, sfb, sfbGrp;
  FIXP_DBL formFactor;

  int tmp0 = psyOutChan->sfbCnt;
  int tmp1 = psyOutChan->maxSfbPerGroup;
  int step = psyOutChan->sfbPerGroup;
  for (sfbGrp = 0; sfbGrp < tmp0; sfbGrp += step) {
    for (sfb = 0; sfb < tmp1; sfb++) {
      formFactor = FL2FXCONST_DBL(0.0f);
      /* calc sum of sqrt(spec) */
      for (j = psyOutChan->sfbOffsets[sfbGrp + sfb];
           j < psyOutChan->sfbOffsets[sfbGrp + sfb + 1]; j++) {
        formFactor +=
            sqrtFixp(fixp_abs(psyOutChan->mdctSpectrum[j])) >> FORM_FAC_SHIFT;
      }
      sfbFormFactorLdData[sfbGrp + sfb] = CalcLdData(formFactor);
    }
    /* set sfbFormFactor for sfbs with zero spec to zero. Just for debugging. */
    for (; sfb < psyOutChan->sfbPerGroup; sfb++) {
      sfbFormFactorLdData[sfbGrp + sfb] = FL2FXCONST_DBL(-1.0f);
    }
  }
}

/*
  Function: FDKaacEnc_CalcFormFactor

  Description: Calls FDKaacEnc_FDKaacEnc_CalcFormFactorChannel() for each
  channel
*/

void FDKaacEnc_CalcFormFactor(QC_OUT_CHANNEL *qcOutChannel[(2)],
                              PSY_OUT_CHANNEL *psyOutChannel[(2)],
                              const INT nChannels) {
  INT j;
  for (j = 0; j < nChannels; j++) {
    FDKaacEnc_FDKaacEnc_CalcFormFactorChannel(
        qcOutChannel[j]->sfbFormFactorLdData, psyOutChannel[j]);
  }
}

/*
  Function: FDKaacEnc_calcSfbRelevantLines

  Description: Calculates sfbNRelevantLines

  sfbNRelevantLines is scaled with the factor 1/((2^FORM_FAC_SHIFT) * 2.0)
*/
static void FDKaacEnc_calcSfbRelevantLines(
    const FIXP_DBL *const sfbFormFactorLdData,
    const FIXP_DBL *const sfbEnergyLdData,
    const FIXP_DBL *const sfbThresholdLdData, const INT *const sfbOffsets,
    const INT sfbCnt, const INT sfbPerGroup, const INT maxSfbPerGroup,
    FIXP_DBL *sfbNRelevantLines) {
  INT sfbOffs, sfb;
  FIXP_DBL sfbWidthLdData;
  FIXP_DBL asPeFacLdData =
      FL2FXCONST_DBL(0.109375); /* AS_PE_FAC_SHIFT*ld64(2) */
  FIXP_DBL accu;

  /* sfbNRelevantLines[i] = 2^( (sfbFormFactorLdData[i] - 0.25 *
   * (sfbEnergyLdData[i] - ld64(sfbWidth[i]/(2^7)) - AS_PE_FAC_SHIFT*ld64(2)) *
   * 64); */

  FDKmemclear(sfbNRelevantLines, sfbCnt * sizeof(FIXP_DBL));

  for (sfbOffs = 0; sfbOffs < sfbCnt; sfbOffs += sfbPerGroup) {
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      /* calc sum of sqrt(spec) */
      if ((FIXP_DBL)sfbEnergyLdData[sfbOffs + sfb] >
          (FIXP_DBL)sfbThresholdLdData[sfbOffs + sfb]) {
        INT sfbWidth =
            sfbOffsets[sfbOffs + sfb + 1] - sfbOffsets[sfbOffs + sfb];

        /* avgFormFactorLdData =
         * sqrtFixp(sqrtFixp(sfbEnergyLdData[sfbOffs+sfb]/sfbWidth)); */
        /* sfbNRelevantLines[sfbOffs+sfb] = sfbFormFactor[sfbOffs+sfb] /
         * avgFormFactorLdData; */
        sfbWidthLdData =
            (FIXP_DBL)(sfbWidth << (DFRACT_BITS - 1 - AS_PE_FAC_SHIFT));
        sfbWidthLdData = CalcLdData(sfbWidthLdData);

        accu = sfbEnergyLdData[sfbOffs + sfb] - sfbWidthLdData - asPeFacLdData;
        accu = sfbFormFactorLdData[sfbOffs + sfb] - (accu >> 2);

        sfbNRelevantLines[sfbOffs + sfb] = CalcInvLdData(accu) >> 1;
      }
    }
  }
}

/*
  Function: FDKaacEnc_countSingleScfBits

  Description:

  scfBitsFract is scaled by 1/(2^(2*AS_PE_FAC_SHIFT))
*/
static FIXP_DBL FDKaacEnc_countSingleScfBits(INT scf, INT scfLeft,
                                             INT scfRight) {
  FIXP_DBL scfBitsFract;

  scfBitsFract = (FIXP_DBL)(FDKaacEnc_bitCountScalefactorDelta(scfLeft - scf) +
                            FDKaacEnc_bitCountScalefactorDelta(scf - scfRight));

  scfBitsFract = scfBitsFract << (DFRACT_BITS - 1 - (2 * AS_PE_FAC_SHIFT));

  return scfBitsFract; /* output scaled by 1/(2^(2*AS_PE_FAC)) */
}

/*
  Function: FDKaacEnc_calcSingleSpecPe

  specPe is scaled by 1/(2^(2*AS_PE_FAC_SHIFT))
*/
static FIXP_DBL FDKaacEnc_calcSingleSpecPe(INT scf, FIXP_DBL sfbConstPePart,
                                           FIXP_DBL nLines) {
  FIXP_DBL specPe = FL2FXCONST_DBL(0.0f);
  FIXP_DBL ldRatio;
  FIXP_DBL scfFract;

  scfFract = (FIXP_DBL)(scf << (DFRACT_BITS - 1 - AS_PE_FAC_SHIFT));

  ldRatio = sfbConstPePart - fMult(FL2FXCONST_DBL(0.375f), scfFract);

  if (ldRatio >= PE_C1) {
    specPe = fMult(FL2FXCONST_DBL(0.7f), fMult(nLines, ldRatio));
  } else {
    specPe = fMult(FL2FXCONST_DBL(0.7f),
                   fMult(nLines, (PE_C2 + fMult(PE_C3, ldRatio))));
  }

  return specPe; /* output scaled by 1/(2^(2*AS_PE_FAC)) */
}

/*
  Function: FDKaacEnc_countScfBitsDiff

  scfBitsDiff is scaled by 1/(2^(2*AS_PE_FAC_SHIFT))
*/
static FIXP_DBL FDKaacEnc_countScfBitsDiff(INT *scfOld, INT *scfNew, INT sfbCnt,
                                           INT startSfb, INT stopSfb) {
  FIXP_DBL scfBitsFract;
  INT scfBitsDiff = 0;
  INT sfb = 0, sfbLast;
  INT sfbPrev, sfbNext;

  /* search for first relevant sfb */
  sfbLast = startSfb;
  while ((sfbLast < stopSfb) && (scfOld[sfbLast] == FDK_INT_MIN)) sfbLast++;
  /* search for previous relevant sfb and count diff */
  sfbPrev = startSfb - 1;
  while ((sfbPrev >= 0) && (scfOld[sfbPrev] == FDK_INT_MIN)) sfbPrev--;
  if (sfbPrev >= 0)
    scfBitsDiff +=
        FDKaacEnc_bitCountScalefactorDelta(scfNew[sfbPrev] - scfNew[sfbLast]) -
        FDKaacEnc_bitCountScalefactorDelta(scfOld[sfbPrev] - scfOld[sfbLast]);
  /* now loop through all sfbs and count diffs of relevant sfbs */
  for (sfb = sfbLast + 1; sfb < stopSfb; sfb++) {
    if (scfOld[sfb] != FDK_INT_MIN) {
      scfBitsDiff +=
          FDKaacEnc_bitCountScalefactorDelta(scfNew[sfbLast] - scfNew[sfb]) -
          FDKaacEnc_bitCountScalefactorDelta(scfOld[sfbLast] - scfOld[sfb]);
      sfbLast = sfb;
    }
  }
  /* search for next relevant sfb and count diff */
  sfbNext = stopSfb;
  while ((sfbNext < sfbCnt) && (scfOld[sfbNext] == FDK_INT_MIN)) sfbNext++;
  if (sfbNext < sfbCnt)
    scfBitsDiff +=
        FDKaacEnc_bitCountScalefactorDelta(scfNew[sfbLast] - scfNew[sfbNext]) -
        FDKaacEnc_bitCountScalefactorDelta(scfOld[sfbLast] - scfOld[sfbNext]);

  scfBitsFract =
      (FIXP_DBL)(scfBitsDiff << (DFRACT_BITS - 1 - (2 * AS_PE_FAC_SHIFT)));

  return scfBitsFract;
}

/*
  Function: FDKaacEnc_calcSpecPeDiff

  specPeDiff is scaled by 1/(2^(2*AS_PE_FAC_SHIFT))
*/
static FIXP_DBL FDKaacEnc_calcSpecPeDiff(
    PSY_OUT_CHANNEL *psyOutChan, QC_OUT_CHANNEL *qcOutChannel, INT *scfOld,
    INT *scfNew, FIXP_DBL *sfbConstPePart, FIXP_DBL *sfbFormFactorLdData,
    FIXP_DBL *sfbNRelevantLines, INT startSfb, INT stopSfb) {
  FIXP_DBL specPeDiff = FL2FXCONST_DBL(0.0f);
  FIXP_DBL scfFract = FL2FXCONST_DBL(0.0f);
  INT sfb;

  /* loop through all sfbs and count pe difference */
  for (sfb = startSfb; sfb < stopSfb; sfb++) {
    if (scfOld[sfb] != FDK_INT_MIN) {
      FIXP_DBL ldRatioOld, ldRatioNew, pOld, pNew;

      /* sfbConstPePart[sfb] = (float)log(psyOutChan->sfbEnergy[sfb] * 6.75f /
       * sfbFormFactor[sfb]) * LOG2_1; */
      /* 0.02152255861f = log(6.75)/log(2)/AS_PE_FAC_FLOAT; LOG2_1 is 1.0 for
       * log2 */
      /* 0.09375f = log(64.0)/log(2.0)/64.0 = scale of sfbFormFactorLdData */
      if (sfbConstPePart[sfb] == (FIXP_DBL)FDK_INT_MIN)
        sfbConstPePart[sfb] =
            ((psyOutChan->sfbEnergyLdData[sfb] - sfbFormFactorLdData[sfb] -
              FL2FXCONST_DBL(0.09375f)) >>
             1) +
            FL2FXCONST_DBL(0.02152255861f);

      scfFract = (FIXP_DBL)(scfOld[sfb] << (DFRACT_BITS - 1 - AS_PE_FAC_SHIFT));
      ldRatioOld =
          sfbConstPePart[sfb] - fMult(FL2FXCONST_DBL(0.375f), scfFract);

      scfFract = (FIXP_DBL)(scfNew[sfb] << (DFRACT_BITS - 1 - AS_PE_FAC_SHIFT));
      ldRatioNew =
          sfbConstPePart[sfb] - fMult(FL2FXCONST_DBL(0.375f), scfFract);

      if (ldRatioOld >= PE_C1)
        pOld = ldRatioOld;
      else
        pOld = PE_C2 + fMult(PE_C3, ldRatioOld);

      if (ldRatioNew >= PE_C1)
        pNew = ldRatioNew;
      else
        pNew = PE_C2 + fMult(PE_C3, ldRatioNew);

      specPeDiff += fMult(FL2FXCONST_DBL(0.7f),
                          fMult(sfbNRelevantLines[sfb], (pNew - pOld)));
    }
  }

  return specPeDiff;
}

/*
  Function: FDKaacEnc_improveScf

  Description: Calculate the distortion by quantization and inverse quantization
  of the spectrum with various scalefactors. The scalefactor which provides the
  best results will be used.
*/
static INT FDKaacEnc_improveScf(const FIXP_DBL *spec, SHORT *quantSpec,
                                SHORT *quantSpecTmp, INT sfbWidth,
                                FIXP_DBL threshLdData, INT scf, INT minScf,
                                FIXP_DBL *distLdData, INT *minScfCalculated,
                                INT dZoneQuantEnable) {
  FIXP_DBL sfbDistLdData;
  INT scfBest = scf;
  INT k;
  FIXP_DBL distFactorLdData = FL2FXCONST_DBL(-0.0050301265); /* ld64(1/1.25) */

  /* calc real distortion */
  sfbDistLdData =
      FDKaacEnc_calcSfbDist(spec, quantSpec, sfbWidth, scf, dZoneQuantEnable);
  *minScfCalculated = scf;
  /* nmr > 1.25 -> try to improve nmr */
  if (sfbDistLdData > (threshLdData - distFactorLdData)) {
    INT scfEstimated = scf;
    FIXP_DBL sfbDistBestLdData = sfbDistLdData;
    INT cnt;
    /* improve by bigger scf ? */
    cnt = 0;

    while ((sfbDistLdData > (threshLdData - distFactorLdData)) &&
           (cnt++ < UPCOUNT_LIMIT)) {
      scf++;
      sfbDistLdData = FDKaacEnc_calcSfbDist(spec, quantSpecTmp, sfbWidth, scf,
                                            dZoneQuantEnable);

      if (sfbDistLdData < sfbDistBestLdData) {
        scfBest = scf;
        sfbDistBestLdData = sfbDistLdData;
        for (k = 0; k < sfbWidth; k++) quantSpec[k] = quantSpecTmp[k];
      }
    }
    /* improve by smaller scf ? */
    cnt = 0;
    scf = scfEstimated;
    sfbDistLdData = sfbDistBestLdData;
    while ((sfbDistLdData > (threshLdData - distFactorLdData)) && (cnt++ < 1) &&
           (scf > minScf)) {
      scf--;
      sfbDistLdData = FDKaacEnc_calcSfbDist(spec, quantSpecTmp, sfbWidth, scf,
                                            dZoneQuantEnable);

      if (sfbDistLdData < sfbDistBestLdData) {
        scfBest = scf;
        sfbDistBestLdData = sfbDistLdData;
        for (k = 0; k < sfbWidth; k++) quantSpec[k] = quantSpecTmp[k];
      }
      *minScfCalculated = scf;
    }
    *distLdData = sfbDistBestLdData;
  } else { /* nmr <= 1.25 -> try to find bigger scf to use less bits */
    FIXP_DBL sfbDistBestLdData = sfbDistLdData;
    FIXP_DBL sfbDistAllowedLdData =
        fixMin(sfbDistLdData - distFactorLdData, threshLdData);
    int cnt;
    for (cnt = 0; cnt < UPCOUNT_LIMIT; cnt++) {
      scf++;
      sfbDistLdData = FDKaacEnc_calcSfbDist(spec, quantSpecTmp, sfbWidth, scf,
                                            dZoneQuantEnable);

      if (sfbDistLdData < sfbDistAllowedLdData) {
        *minScfCalculated = scfBest + 1;
        scfBest = scf;
        sfbDistBestLdData = sfbDistLdData;
        for (k = 0; k < sfbWidth; k++) quantSpec[k] = quantSpecTmp[k];
      }
    }
    *distLdData = sfbDistBestLdData;
  }

  /* return best scalefactor */
  return scfBest;
}

/*
  Function: FDKaacEnc_assimilateSingleScf

*/
static void FDKaacEnc_assimilateSingleScf(
    const PSY_OUT_CHANNEL *psyOutChan, const QC_OUT_CHANNEL *qcOutChannel,
    SHORT *quantSpec, SHORT *quantSpecTmp, INT dZoneQuantEnable, INT *scf,
    const INT *minScf, FIXP_DBL *sfbDist, FIXP_DBL *sfbConstPePart,
    const FIXP_DBL *sfbFormFactorLdData, const FIXP_DBL *sfbNRelevantLines,
    INT *minScfCalculated, INT restartOnSuccess) {
  INT sfbLast, sfbAct, sfbNext;
  INT scfAct, *scfLast, *scfNext, scfMin, scfMax;
  INT sfbWidth, sfbOffs;
  FIXP_DBL enLdData;
  FIXP_DBL sfbPeOld, sfbPeNew;
  FIXP_DBL sfbDistNew;
  INT i, k;
  INT success = 0;
  FIXP_DBL deltaPe = FL2FXCONST_DBL(0.0f);
  FIXP_DBL deltaPeNew, deltaPeTmp;
  INT prevScfLast[MAX_GROUPED_SFB], prevScfNext[MAX_GROUPED_SFB];
  FIXP_DBL deltaPeLast[MAX_GROUPED_SFB];
  INT updateMinScfCalculated;

  for (i = 0; i < psyOutChan->sfbCnt; i++) {
    prevScfLast[i] = FDK_INT_MAX;
    prevScfNext[i] = FDK_INT_MAX;
    deltaPeLast[i] = (FIXP_DBL)FDK_INT_MAX;
  }

  sfbLast = -1;
  sfbAct = -1;
  sfbNext = -1;
  scfLast = 0;
  scfNext = 0;
  scfMin = FDK_INT_MAX;
  scfMax = FDK_INT_MAX;
  do {
    /* search for new relevant sfb */
    sfbNext++;
    while ((sfbNext < psyOutChan->sfbCnt) && (scf[sfbNext] == FDK_INT_MIN))
      sfbNext++;
    if ((sfbLast >= 0) && (sfbAct >= 0) && (sfbNext < psyOutChan->sfbCnt)) {
      /* relevant scfs to the left and to the right */
      scfAct = scf[sfbAct];
      scfLast = scf + sfbLast;
      scfNext = scf + sfbNext;
      scfMin = fixMin(*scfLast, *scfNext);
      scfMax = fixMax(*scfLast, *scfNext);
    } else if ((sfbLast == -1) && (sfbAct >= 0) &&
               (sfbNext < psyOutChan->sfbCnt)) {
      /* first relevant scf */
      scfAct = scf[sfbAct];
      scfLast = &scfAct;
      scfNext = scf + sfbNext;
      scfMin = *scfNext;
      scfMax = *scfNext;
    } else if ((sfbLast >= 0) && (sfbAct >= 0) &&
               (sfbNext == psyOutChan->sfbCnt)) {
      /* last relevant scf */
      scfAct = scf[sfbAct];
      scfLast = scf + sfbLast;
      scfNext = &scfAct;
      scfMin = *scfLast;
      scfMax = *scfLast;
    }
    if (sfbAct >= 0) scfMin = fixMax(scfMin, minScf[sfbAct]);

    if ((sfbAct >= 0) && (sfbLast >= 0 || sfbNext < psyOutChan->sfbCnt) &&
        (scfAct > scfMin) && (scfAct <= scfMin + MAX_SCF_DELTA) &&
        (scfAct >= scfMax - MAX_SCF_DELTA) &&
        (scfAct <=
         fixMin(scfMin, fixMin(*scfLast, *scfNext)) + MAX_SCF_DELTA) &&
        (*scfLast != prevScfLast[sfbAct] || *scfNext != prevScfNext[sfbAct] ||
         deltaPe < deltaPeLast[sfbAct])) {
      /* bigger than neighbouring scf found, try to use smaller scf */
      success = 0;

      sfbWidth =
          psyOutChan->sfbOffsets[sfbAct + 1] - psyOutChan->sfbOffsets[sfbAct];
      sfbOffs = psyOutChan->sfbOffsets[sfbAct];

      /* estimate required bits for actual scf */
      enLdData = qcOutChannel->sfbEnergyLdData[sfbAct];

      /* sfbConstPePart[sfbAct] = (float)log(6.75f*en/sfbFormFactor[sfbAct]) *
       * LOG2_1; */
      /* 0.02152255861f = log(6.75)/log(2)/AS_PE_FAC_FLOAT; LOG2_1 is 1.0 for
       * log2 */
      /* 0.09375f = log(64.0)/log(2.0)/64.0 = scale of sfbFormFactorLdData */
      if (sfbConstPePart[sfbAct] == (FIXP_DBL)FDK_INT_MIN) {
        sfbConstPePart[sfbAct] = ((enLdData - sfbFormFactorLdData[sfbAct] -
                                   FL2FXCONST_DBL(0.09375f)) >>
                                  1) +
                                 FL2FXCONST_DBL(0.02152255861f);
      }

      sfbPeOld = FDKaacEnc_calcSingleSpecPe(scfAct, sfbConstPePart[sfbAct],
                                            sfbNRelevantLines[sfbAct]) +
                 FDKaacEnc_countSingleScfBits(scfAct, *scfLast, *scfNext);

      deltaPeNew = deltaPe;
      updateMinScfCalculated = 1;

      do {
        /* estimate required bits for smaller scf */
        scfAct--;
        /* check only if the same check was not done before */
        if (scfAct < minScfCalculated[sfbAct] &&
            scfAct >= scfMax - MAX_SCF_DELTA) {
          /* estimate required bits for new scf */
          sfbPeNew = FDKaacEnc_calcSingleSpecPe(scfAct, sfbConstPePart[sfbAct],
                                                sfbNRelevantLines[sfbAct]) +
                     FDKaacEnc_countSingleScfBits(scfAct, *scfLast, *scfNext);

          /* use new scf if no increase in pe and
             quantization error is smaller */
          deltaPeTmp = deltaPe + sfbPeNew - sfbPeOld;
          /* 0.0006103515625f = 10.0f/(2^(2*AS_PE_FAC_SHIFT)) */
          if (deltaPeTmp < FL2FXCONST_DBL(0.0006103515625f)) {
            /* distortion of new scf */
            sfbDistNew = FDKaacEnc_calcSfbDist(
                qcOutChannel->mdctSpectrum + sfbOffs, quantSpecTmp + sfbOffs,
                sfbWidth, scfAct, dZoneQuantEnable);

            if (sfbDistNew < sfbDist[sfbAct]) {
              /* success, replace scf by new one */
              scf[sfbAct] = scfAct;
              sfbDist[sfbAct] = sfbDistNew;

              for (k = 0; k < sfbWidth; k++)
                quantSpec[sfbOffs + k] = quantSpecTmp[sfbOffs + k];

              deltaPeNew = deltaPeTmp;
              success = 1;
            }
            /* mark as already checked */
            if (updateMinScfCalculated) minScfCalculated[sfbAct] = scfAct;
          } else {
            /* from this scf value on not all new values have been checked */
            updateMinScfCalculated = 0;
          }
        }
      } while (scfAct > scfMin);

      deltaPe = deltaPeNew;

      /* save parameters to avoid multiple computations of the same sfb */
      prevScfLast[sfbAct] = *scfLast;
      prevScfNext[sfbAct] = *scfNext;
      deltaPeLast[sfbAct] = deltaPe;
    }

    if (success && restartOnSuccess) {
      /* start again at first sfb */
      sfbLast = -1;
      sfbAct = -1;
      sfbNext = -1;
      scfLast = 0;
      scfNext = 0;
      scfMin = FDK_INT_MAX;
      scfMax = FDK_INT_MAX;
      success = 0;
    } else {
      /* shift sfbs for next band */
      sfbLast = sfbAct;
      sfbAct = sfbNext;
    }
  } while (sfbNext < psyOutChan->sfbCnt);
}

/*
  Function: FDKaacEnc_assimilateMultipleScf

*/
static void FDKaacEnc_assimilateMultipleScf(
    PSY_OUT_CHANNEL *psyOutChan, QC_OUT_CHANNEL *qcOutChannel, SHORT *quantSpec,
    SHORT *quantSpecTmp, INT dZoneQuantEnable, INT *scf, const INT *minScf,
    FIXP_DBL *sfbDist, FIXP_DBL *sfbConstPePart, FIXP_DBL *sfbFormFactorLdData,
    FIXP_DBL *sfbNRelevantLines) {
  INT sfb, startSfb, stopSfb;
  INT scfTmp[MAX_GROUPED_SFB], scfMin, scfMax, scfAct;
  INT possibleRegionFound;
  INT sfbWidth, sfbOffs, i, k;
  FIXP_DBL sfbDistNew[MAX_GROUPED_SFB], distOldSum, distNewSum;
  INT deltaScfBits;
  FIXP_DBL deltaSpecPe;
  FIXP_DBL deltaPe = FL2FXCONST_DBL(0.0f);
  FIXP_DBL deltaPeNew;
  INT sfbCnt = psyOutChan->sfbCnt;

  /* calc min and max scalfactors */
  scfMin = FDK_INT_MAX;
  scfMax = FDK_INT_MIN;
  for (sfb = 0; sfb < sfbCnt; sfb++) {
    if (scf[sfb] != FDK_INT_MIN) {
      scfMin = fixMin(scfMin, scf[sfb]);
      scfMax = fixMax(scfMax, scf[sfb]);
    }
  }

  if (scfMax != FDK_INT_MIN && scfMax <= scfMin + MAX_SCF_DELTA) {
    scfAct = scfMax;

    do {
      /* try smaller scf */
      scfAct--;
      for (i = 0; i < MAX_GROUPED_SFB; i++) scfTmp[i] = scf[i];
      stopSfb = 0;
      do {
        /* search for region where all scfs are bigger than scfAct */
        sfb = stopSfb;
        while (sfb < sfbCnt && (scf[sfb] == FDK_INT_MIN || scf[sfb] <= scfAct))
          sfb++;
        startSfb = sfb;
        sfb++;
        while (sfb < sfbCnt && (scf[sfb] == FDK_INT_MIN || scf[sfb] > scfAct))
          sfb++;
        stopSfb = sfb;

        /* check if in all sfb of a valid region scfAct >= minScf[sfb] */
        possibleRegionFound = 0;
        if (startSfb < sfbCnt) {
          possibleRegionFound = 1;
          for (sfb = startSfb; sfb < stopSfb; sfb++) {
            if (scf[sfb] != FDK_INT_MIN)
              if (scfAct < minScf[sfb]) {
                possibleRegionFound = 0;
                break;
              }
          }
        }

        if (possibleRegionFound) { /* region found */

          /* replace scfs in region by scfAct */
          for (sfb = startSfb; sfb < stopSfb; sfb++) {
            if (scfTmp[sfb] != FDK_INT_MIN) scfTmp[sfb] = scfAct;
          }

          /* estimate change in bit demand for new scfs */
          deltaScfBits = FDKaacEnc_countScfBitsDiff(scf, scfTmp, sfbCnt,
                                                    startSfb, stopSfb);

          deltaSpecPe = FDKaacEnc_calcSpecPeDiff(
              psyOutChan, qcOutChannel, scf, scfTmp, sfbConstPePart,
              sfbFormFactorLdData, sfbNRelevantLines, startSfb, stopSfb);

          deltaPeNew = deltaPe + (FIXP_DBL)deltaScfBits + deltaSpecPe;

          /* new bit demand small enough ? */
          /* 0.0006103515625f = 10.0f/(2^(2*AS_PE_FAC_SHIFT)) */
          if (deltaPeNew < FL2FXCONST_DBL(0.0006103515625f)) {
            /* quantize and calc sum of new distortion */
            distOldSum = distNewSum = FL2FXCONST_DBL(0.0f);
            for (sfb = startSfb; sfb < stopSfb; sfb++) {
              if (scfTmp[sfb] != FDK_INT_MIN) {
                distOldSum += CalcInvLdData(sfbDist[sfb]) >> DIST_FAC_SHIFT;

                sfbWidth = psyOutChan->sfbOffsets[sfb + 1] -
                           psyOutChan->sfbOffsets[sfb];
                sfbOffs = psyOutChan->sfbOffsets[sfb];

                sfbDistNew[sfb] = FDKaacEnc_calcSfbDist(
                    qcOutChannel->mdctSpectrum + sfbOffs,
                    quantSpecTmp + sfbOffs, sfbWidth, scfAct, dZoneQuantEnable);

                if (sfbDistNew[sfb] > qcOutChannel->sfbThresholdLdData[sfb]) {
                  /* no improvement, skip further dist. calculations */
                  distNewSum = distOldSum << 1;
                  break;
                }
                distNewSum += CalcInvLdData(sfbDistNew[sfb]) >> DIST_FAC_SHIFT;
              }
            }
            /* distortion smaller ? -> use new scalefactors */
            if (distNewSum < distOldSum) {
              deltaPe = deltaPeNew;
              for (sfb = startSfb; sfb < stopSfb; sfb++) {
                if (scf[sfb] != FDK_INT_MIN) {
                  sfbWidth = psyOutChan->sfbOffsets[sfb + 1] -
                             psyOutChan->sfbOffsets[sfb];
                  sfbOffs = psyOutChan->sfbOffsets[sfb];
                  scf[sfb] = scfAct;
                  sfbDist[sfb] = sfbDistNew[sfb];

                  for (k = 0; k < sfbWidth; k++)
                    quantSpec[sfbOffs + k] = quantSpecTmp[sfbOffs + k];
                }
              }
            }
          }
        }

      } while (stopSfb <= sfbCnt);

    } while (scfAct > scfMin);
  }
}

/*
  Function: FDKaacEnc_FDKaacEnc_assimilateMultipleScf2

*/
static void FDKaacEnc_FDKaacEnc_assimilateMultipleScf2(
    PSY_OUT_CHANNEL *psyOutChan, QC_OUT_CHANNEL *qcOutChannel, SHORT *quantSpec,
    SHORT *quantSpecTmp, INT dZoneQuantEnable, INT *scf, const INT *minScf,
    FIXP_DBL *sfbDist, FIXP_DBL *sfbConstPePart, FIXP_DBL *sfbFormFactorLdData,
    FIXP_DBL *sfbNRelevantLines) {
  INT sfb, startSfb, stopSfb;
  INT scfTmp[MAX_GROUPED_SFB], scfAct, scfNew;
  INT scfPrev, scfNext, scfPrevNextMin, scfPrevNextMax, scfLo, scfHi;
  INT scfMin, scfMax;
  INT *sfbOffs = psyOutChan->sfbOffsets;
  FIXP_DBL sfbDistNew[MAX_GROUPED_SFB], sfbDistMax[MAX_GROUPED_SFB];
  FIXP_DBL distOldSum, distNewSum;
  INT deltaScfBits;
  FIXP_DBL deltaSpecPe;
  FIXP_DBL deltaPe = FL2FXCONST_DBL(0.0f);
  FIXP_DBL deltaPeNew = FL2FXCONST_DBL(0.0f);
  INT sfbCnt = psyOutChan->sfbCnt;
  INT bSuccess, bCheckScf;
  INT i, k;

  /* calc min and max scalfactors */
  scfMin = FDK_INT_MAX;
  scfMax = FDK_INT_MIN;
  for (sfb = 0; sfb < sfbCnt; sfb++) {
    if (scf[sfb] != FDK_INT_MIN) {
      scfMin = fixMin(scfMin, scf[sfb]);
      scfMax = fixMax(scfMax, scf[sfb]);
    }
  }

  stopSfb = 0;
  scfAct = FDK_INT_MIN;
  do {
    /* search for region with same scf values scfAct */
    scfPrev = scfAct;

    sfb = stopSfb;
    while (sfb < sfbCnt && (scf[sfb] == FDK_INT_MIN)) sfb++;
    startSfb = sfb;
    scfAct = scf[startSfb];
    sfb++;
    while (sfb < sfbCnt &&
           ((scf[sfb] == FDK_INT_MIN) || (scf[sfb] == scf[startSfb])))
      sfb++;
    stopSfb = sfb;

    if (stopSfb < sfbCnt)
      scfNext = scf[stopSfb];
    else
      scfNext = scfAct;

    if (scfPrev == FDK_INT_MIN) scfPrev = scfAct;

    scfPrevNextMax = fixMax(scfPrev, scfNext);
    scfPrevNextMin = fixMin(scfPrev, scfNext);

    /* try to reduce bits by checking scf values in the range
       scf[startSfb]...scfHi */
    scfHi = fixMax(scfPrevNextMax, scfAct);
    /* try to find a better solution by reducing the scf difference to
       the nearest possible lower scf */
    if (scfPrevNextMax >= scfAct)
      scfLo = fixMin(scfAct, scfPrevNextMin);
    else
      scfLo = scfPrevNextMax;

    if (startSfb < sfbCnt &&
        scfHi - scfLo <= MAX_SCF_DELTA) { /* region found */
      /* 1. try to save bits by coarser quantization */
      if (scfHi > scf[startSfb]) {
        /* calculate the allowed distortion */
        for (sfb = startSfb; sfb < stopSfb; sfb++) {
          if (scf[sfb] != FDK_INT_MIN) {
            /* sfbDistMax[sfb] =
             * (float)pow(qcOutChannel->sfbThreshold[sfb]*sfbDist[sfb]*sfbDist[sfb],1.0f/3.0f);
             */
            /* sfbDistMax[sfb] =
             * fixMax(sfbDistMax[sfb],qcOutChannel->sfbEnergy[sfb]*FL2FXCONST_DBL(1.e-3f));
             */
            /* -0.15571537944 = ld64(1.e-3f)*/
            sfbDistMax[sfb] = fMult(FL2FXCONST_DBL(1.0f / 3.0f),
                                    qcOutChannel->sfbThresholdLdData[sfb]) +
                              fMult(FL2FXCONST_DBL(1.0f / 3.0f), sfbDist[sfb]) +
                              fMult(FL2FXCONST_DBL(1.0f / 3.0f), sfbDist[sfb]);
            sfbDistMax[sfb] =
                fixMax(sfbDistMax[sfb], qcOutChannel->sfbEnergyLdData[sfb] -
                                            FL2FXCONST_DBL(0.15571537944));
            sfbDistMax[sfb] =
                fixMin(sfbDistMax[sfb], qcOutChannel->sfbThresholdLdData[sfb]);
          }
        }

        /* loop over all possible scf values for this region */
        bCheckScf = 1;
        for (scfNew = scf[startSfb] + 1; scfNew <= scfHi; scfNew++) {
          for (k = 0; k < MAX_GROUPED_SFB; k++) scfTmp[k] = scf[k];

          /* replace scfs in region by scfNew */
          for (sfb = startSfb; sfb < stopSfb; sfb++) {
            if (scfTmp[sfb] != FDK_INT_MIN) scfTmp[sfb] = scfNew;
          }

          /* estimate change in bit demand for new scfs */
          deltaScfBits = FDKaacEnc_countScfBitsDiff(scf, scfTmp, sfbCnt,
                                                    startSfb, stopSfb);

          deltaSpecPe = FDKaacEnc_calcSpecPeDiff(
              psyOutChan, qcOutChannel, scf, scfTmp, sfbConstPePart,
              sfbFormFactorLdData, sfbNRelevantLines, startSfb, stopSfb);

          deltaPeNew = deltaPe + (FIXP_DBL)deltaScfBits + deltaSpecPe;

          /* new bit demand small enough ? */
          if (deltaPeNew < FL2FXCONST_DBL(0.0f)) {
            bSuccess = 1;

            /* quantize and calc sum of new distortion */
            for (sfb = startSfb; sfb < stopSfb; sfb++) {
              if (scfTmp[sfb] != FDK_INT_MIN) {
                sfbDistNew[sfb] = FDKaacEnc_calcSfbDist(
                    qcOutChannel->mdctSpectrum + sfbOffs[sfb],
                    quantSpecTmp + sfbOffs[sfb],
                    sfbOffs[sfb + 1] - sfbOffs[sfb], scfNew, dZoneQuantEnable);

                if (sfbDistNew[sfb] > sfbDistMax[sfb]) {
                  /* no improvement, skip further dist. calculations */
                  bSuccess = 0;
                  if (sfbDistNew[sfb] == qcOutChannel->sfbEnergyLdData[sfb]) {
                    /* if whole sfb is already quantized to 0, further
                       checks with even coarser quant. are useless*/
                    bCheckScf = 0;
                  }
                  break;
                }
              }
            }
            if (bCheckScf == 0) /* further calculations useless ? */
              break;
            /* distortion small enough ? -> use new scalefactors */
            if (bSuccess) {
              deltaPe = deltaPeNew;
              for (sfb = startSfb; sfb < stopSfb; sfb++) {
                if (scf[sfb] != FDK_INT_MIN) {
                  scf[sfb] = scfNew;
                  sfbDist[sfb] = sfbDistNew[sfb];

                  for (k = 0; k < sfbOffs[sfb + 1] - sfbOffs[sfb]; k++)
                    quantSpec[sfbOffs[sfb] + k] =
                        quantSpecTmp[sfbOffs[sfb] + k];
                }
              }
            }
          }
        }
      }

      /* 2. only if coarser quantization was not successful, try to find
         a better solution by finer quantization and reducing bits for
         scalefactor coding */
      if (scfAct == scf[startSfb] && scfLo < scfAct &&
          scfMax - scfMin <= MAX_SCF_DELTA) {
        int bminScfViolation = 0;

        for (k = 0; k < MAX_GROUPED_SFB; k++) scfTmp[k] = scf[k];

        scfNew = scfLo;

        /* replace scfs in region by scfNew and
           check if in all sfb scfNew >= minScf[sfb] */
        for (sfb = startSfb; sfb < stopSfb; sfb++) {
          if (scfTmp[sfb] != FDK_INT_MIN) {
            scfTmp[sfb] = scfNew;
            if (scfNew < minScf[sfb]) bminScfViolation = 1;
          }
        }

        if (!bminScfViolation) {
          /* estimate change in bit demand for new scfs */
          deltaScfBits = FDKaacEnc_countScfBitsDiff(scf, scfTmp, sfbCnt,
                                                    startSfb, stopSfb);

          deltaSpecPe = FDKaacEnc_calcSpecPeDiff(
              psyOutChan, qcOutChannel, scf, scfTmp, sfbConstPePart,
              sfbFormFactorLdData, sfbNRelevantLines, startSfb, stopSfb);

          deltaPeNew = deltaPe + (FIXP_DBL)deltaScfBits + deltaSpecPe;
        }

        /* new bit demand small enough ? */
        if (!bminScfViolation && deltaPeNew < FL2FXCONST_DBL(0.0f)) {
          /* quantize and calc sum of new distortion */
          distOldSum = distNewSum = FL2FXCONST_DBL(0.0f);
          for (sfb = startSfb; sfb < stopSfb; sfb++) {
            if (scfTmp[sfb] != FDK_INT_MIN) {
              distOldSum += CalcInvLdData(sfbDist[sfb]) >> DIST_FAC_SHIFT;

              sfbDistNew[sfb] = FDKaacEnc_calcSfbDist(
                  qcOutChannel->mdctSpectrum + sfbOffs[sfb],
                  quantSpecTmp + sfbOffs[sfb], sfbOffs[sfb + 1] - sfbOffs[sfb],
                  scfNew, dZoneQuantEnable);

              if (sfbDistNew[sfb] > qcOutChannel->sfbThresholdLdData[sfb]) {
                /* no improvement, skip further dist. calculations */
                distNewSum = distOldSum << 1;
                break;
              }
              distNewSum += CalcInvLdData(sfbDistNew[sfb]) >> DIST_FAC_SHIFT;
            }
          }
          /* distortion smaller ? -> use new scalefactors */
          if (distNewSum < fMult(FL2FXCONST_DBL(0.8f), distOldSum)) {
            deltaPe = deltaPeNew;
            for (sfb = startSfb; sfb < stopSfb; sfb++) {
              if (scf[sfb] != FDK_INT_MIN) {
                scf[sfb] = scfNew;
                sfbDist[sfb] = sfbDistNew[sfb];

                for (k = 0; k < sfbOffs[sfb + 1] - sfbOffs[sfb]; k++)
                  quantSpec[sfbOffs[sfb] + k] = quantSpecTmp[sfbOffs[sfb] + k];
              }
            }
          }
        }
      }

      /* 3. try to find a better solution (save bits) by only reducing the
         scalefactor without new quantization */
      if (scfMax - scfMin <=
          MAX_SCF_DELTA - 3) { /* 3 bec. scf is reduced 3 times,
                                  see for loop below */

        for (k = 0; k < sfbCnt; k++) scfTmp[k] = scf[k];

        for (i = 0; i < 3; i++) {
          scfNew = scfTmp[startSfb] - 1;
          /* replace scfs in region by scfNew */
          for (sfb = startSfb; sfb < stopSfb; sfb++) {
            if (scfTmp[sfb] != FDK_INT_MIN) scfTmp[sfb] = scfNew;
          }
          /* estimate change in bit demand for new scfs */
          deltaScfBits = FDKaacEnc_countScfBitsDiff(scf, scfTmp, sfbCnt,
                                                    startSfb, stopSfb);
          deltaPeNew = deltaPe + (FIXP_DBL)deltaScfBits;
          /* new bit demand small enough ? */
          if (deltaPeNew <= FL2FXCONST_DBL(0.0f)) {
            bSuccess = 1;
            distOldSum = distNewSum = FL2FXCONST_DBL(0.0f);
            for (sfb = startSfb; sfb < stopSfb; sfb++) {
              if (scfTmp[sfb] != FDK_INT_MIN) {
                FIXP_DBL sfbEnQ;
                /* calc the energy and distortion of the quantized spectrum for
                   a smaller scf */
                FDKaacEnc_calcSfbQuantEnergyAndDist(
                    qcOutChannel->mdctSpectrum + sfbOffs[sfb],
                    quantSpec + sfbOffs[sfb], sfbOffs[sfb + 1] - sfbOffs[sfb],
                    scfNew, &sfbEnQ, &sfbDistNew[sfb]);

                distOldSum += CalcInvLdData(sfbDist[sfb]) >> DIST_FAC_SHIFT;
                distNewSum += CalcInvLdData(sfbDistNew[sfb]) >> DIST_FAC_SHIFT;

                /*  0.00259488556167 = ld64(1.122f) */
                /* -0.00778722686652 = ld64(0.7079f) */
                if ((sfbDistNew[sfb] >
                     (sfbDist[sfb] + FL2FXCONST_DBL(0.00259488556167f))) ||
                    (sfbEnQ < (qcOutChannel->sfbEnergyLdData[sfb] -
                               FL2FXCONST_DBL(0.00778722686652f)))) {
                  bSuccess = 0;
                  break;
                }
              }
            }
            /* distortion smaller ? -> use new scalefactors */
            if (distNewSum < distOldSum && bSuccess) {
              deltaPe = deltaPeNew;
              for (sfb = startSfb; sfb < stopSfb; sfb++) {
                if (scf[sfb] != FDK_INT_MIN) {
                  scf[sfb] = scfNew;
                  sfbDist[sfb] = sfbDistNew[sfb];
                }
              }
            }
          }
        }
      }
    }
  } while (stopSfb <= sfbCnt);
}

static void FDKaacEnc_EstimateScaleFactorsChannel(
    QC_OUT_CHANNEL *qcOutChannel, PSY_OUT_CHANNEL *psyOutChannel,
    INT *RESTRICT scf, INT *RESTRICT globalGain,
    FIXP_DBL *RESTRICT sfbFormFactorLdData, const INT invQuant,
    SHORT *RESTRICT quantSpec, const INT dZoneQuantEnable) {
  INT i, j, sfb, sfbOffs;
  INT scfInt;
  INT maxSf;
  INT minSf;
  FIXP_DBL threshLdData;
  FIXP_DBL energyLdData;
  FIXP_DBL energyPartLdData;
  FIXP_DBL thresholdPartLdData;
  FIXP_DBL scfFract;
  FIXP_DBL maxSpec;
  INT minScfCalculated[MAX_GROUPED_SFB];
  FIXP_DBL sfbDistLdData[MAX_GROUPED_SFB];
  C_ALLOC_SCRATCH_START(quantSpecTmp, SHORT, (1024))
  INT minSfMaxQuant[MAX_GROUPED_SFB];

  FIXP_DBL threshConstLdData =
      FL2FXCONST_DBL(0.04304511722f); /* log10(6.75)/log10(2.0)/64.0 */
  FIXP_DBL convConst = FL2FXCONST_DBL(0.30102999566f); /* log10(2.0) */
  FIXP_DBL c1Const =
      FL2FXCONST_DBL(-0.27083183594f); /* C1 = -69.33295 => C1/2^8 */

  if (invQuant > 0) {
    FDKmemclear(quantSpec, (1024) * sizeof(SHORT));
  }

  /* scfs without energy or with thresh>energy are marked with FDK_INT_MIN */
  for (i = 0; i < psyOutChannel->sfbCnt; i++) {
    scf[i] = FDK_INT_MIN;
  }

  for (i = 0; i < MAX_GROUPED_SFB; i++) {
    minSfMaxQuant[i] = FDK_INT_MIN;
  }

  for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
       sfbOffs += psyOutChannel->sfbPerGroup) {
    for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
      threshLdData = qcOutChannel->sfbThresholdLdData[sfbOffs + sfb];
      energyLdData = qcOutChannel->sfbEnergyLdData[sfbOffs + sfb];

      sfbDistLdData[sfbOffs + sfb] = energyLdData;

      if (energyLdData > threshLdData) {
        FIXP_DBL tmp;

        /* energyPart = (float)log10(sfbFormFactor[sfbOffs+sfb]); */
        /* 0.09375f = log(64.0)/log(2.0)/64.0 = scale of sfbFormFactorLdData */
        energyPartLdData =
            sfbFormFactorLdData[sfbOffs + sfb] + FL2FXCONST_DBL(0.09375f);

        /* influence of allowed distortion */
        /* thresholdPart = (float)log10(6.75*thresh+FLT_MIN); */
        thresholdPartLdData = threshConstLdData + threshLdData;

        /* scf calc */
        /* scfFloat = 8.8585f * (thresholdPart - energyPart); */
        scfFract = thresholdPartLdData - energyPartLdData;
        /* conversion from log2 to log10 */
        scfFract = fMult(convConst, scfFract);
        /* (8.8585f * scfFract)/8 = 8/8 * scfFract + 0.8585 * scfFract/8 */
        scfFract = scfFract + fMult(FL2FXCONST_DBL(0.8585f), scfFract >> 3);

        /* integer scalefactor */
        /* scfInt = (int)floor(scfFloat); */
        scfInt =
            (INT)(scfFract >>
                  ((DFRACT_BITS - 1) - 3 -
                   LD_DATA_SHIFT)); /* 3 bits => scfFract/8.0; 6 bits => ld64 */

        /* maximum of spectrum */
        maxSpec = FL2FXCONST_DBL(0.0f);

        /* Unroll by 4, allow dual memory access */
        DWORD_ALIGNED(qcOutChannel->mdctSpectrum);
        for (j = psyOutChannel->sfbOffsets[sfbOffs + sfb];
             j < psyOutChannel->sfbOffsets[sfbOffs + sfb + 1]; j += 4) {
          maxSpec = fMax(maxSpec,
                         fMax(fMax(fAbs(qcOutChannel->mdctSpectrum[j + 0]),
                                   fAbs(qcOutChannel->mdctSpectrum[j + 1])),
                              fMax(fAbs(qcOutChannel->mdctSpectrum[j + 2]),
                                   fAbs(qcOutChannel->mdctSpectrum[j + 3]))));
        }
        /* lower scf limit to avoid quantized values bigger than MAX_QUANT */
        /* C1 = -69.33295f, C2 = 5.77078f = 4/log(2) */
        /* minSfMaxQuant[sfbOffs+sfb] = (int)ceil(C1 + C2*log(maxSpec)); */
        /* C1/2^8 + 4/log(2.0)*log(maxSpec)/2^8  => C1/2^8 +
         * log(maxSpec)/log(2.0)*4/2^8 => C1/2^8 + log(maxSpec)/log(2.0)/64.0 */

        // minSfMaxQuant[sfbOffs+sfb] = ((INT) ((c1Const + CalcLdData(maxSpec))
        // >> ((DFRACT_BITS-1)-8))) + 1;
        tmp = CalcLdData(maxSpec);
        if (c1Const > FL2FXCONST_DBL(-1.f) - tmp) {
          minSfMaxQuant[sfbOffs + sfb] =
              ((INT)((c1Const + tmp) >> ((DFRACT_BITS - 1) - 8))) + 1;
        } else {
          minSfMaxQuant[sfbOffs + sfb] =
              ((INT)(FL2FXCONST_DBL(-1.f) >> ((DFRACT_BITS - 1) - 8))) + 1;
        }

        scfInt = fixMax(scfInt, minSfMaxQuant[sfbOffs + sfb]);

        /* find better scalefactor with analysis by synthesis */
        if (invQuant > 0) {
          scfInt = FDKaacEnc_improveScf(
              qcOutChannel->mdctSpectrum +
                  psyOutChannel->sfbOffsets[sfbOffs + sfb],
              quantSpec + psyOutChannel->sfbOffsets[sfbOffs + sfb],
              quantSpecTmp + psyOutChannel->sfbOffsets[sfbOffs + sfb],
              psyOutChannel->sfbOffsets[sfbOffs + sfb + 1] -
                  psyOutChannel->sfbOffsets[sfbOffs + sfb],
              threshLdData, scfInt, minSfMaxQuant[sfbOffs + sfb],
              &sfbDistLdData[sfbOffs + sfb], &minScfCalculated[sfbOffs + sfb],
              dZoneQuantEnable);
        }
        scf[sfbOffs + sfb] = scfInt;
      }
    }
  }

  if (invQuant > 0) {
    /* try to decrease scf differences */
    FIXP_DBL sfbConstPePart[MAX_GROUPED_SFB];
    FIXP_DBL sfbNRelevantLines[MAX_GROUPED_SFB];

    for (i = 0; i < psyOutChannel->sfbCnt; i++)
      sfbConstPePart[i] = (FIXP_DBL)FDK_INT_MIN;

    FDKaacEnc_calcSfbRelevantLines(
        sfbFormFactorLdData, qcOutChannel->sfbEnergyLdData,
        qcOutChannel->sfbThresholdLdData, psyOutChannel->sfbOffsets,
        psyOutChannel->sfbCnt, psyOutChannel->sfbPerGroup,
        psyOutChannel->maxSfbPerGroup, sfbNRelevantLines);

    FDKaacEnc_assimilateSingleScf(
        psyOutChannel, qcOutChannel, quantSpec, quantSpecTmp, dZoneQuantEnable,
        scf, minSfMaxQuant, sfbDistLdData, sfbConstPePart, sfbFormFactorLdData,
        sfbNRelevantLines, minScfCalculated, 1);

    if (invQuant > 1) {
      FDKaacEnc_assimilateMultipleScf(
          psyOutChannel, qcOutChannel, quantSpec, quantSpecTmp,
          dZoneQuantEnable, scf, minSfMaxQuant, sfbDistLdData, sfbConstPePart,
          sfbFormFactorLdData, sfbNRelevantLines);

      FDKaacEnc_FDKaacEnc_assimilateMultipleScf2(
          psyOutChannel, qcOutChannel, quantSpec, quantSpecTmp,
          dZoneQuantEnable, scf, minSfMaxQuant, sfbDistLdData, sfbConstPePart,
          sfbFormFactorLdData, sfbNRelevantLines);
    }
  }

  /* get min scalefac */
  minSf = FDK_INT_MAX;
  for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
       sfbOffs += psyOutChannel->sfbPerGroup) {
    for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
      if (scf[sfbOffs + sfb] != FDK_INT_MIN)
        minSf = fixMin(minSf, scf[sfbOffs + sfb]);
    }
  }

  /* limit scf delta */
  for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
       sfbOffs += psyOutChannel->sfbPerGroup) {
    for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
      if ((scf[sfbOffs + sfb] != FDK_INT_MIN) &&
          (minSf + MAX_SCF_DELTA) < scf[sfbOffs + sfb]) {
        scf[sfbOffs + sfb] = minSf + MAX_SCF_DELTA;
        if (invQuant > 0) { /* changed bands need to be quantized again */
          sfbDistLdData[sfbOffs + sfb] = FDKaacEnc_calcSfbDist(
              qcOutChannel->mdctSpectrum +
                  psyOutChannel->sfbOffsets[sfbOffs + sfb],
              quantSpec + psyOutChannel->sfbOffsets[sfbOffs + sfb],
              psyOutChannel->sfbOffsets[sfbOffs + sfb + 1] -
                  psyOutChannel->sfbOffsets[sfbOffs + sfb],
              scf[sfbOffs + sfb], dZoneQuantEnable);
        }
      }
    }
  }

  /* get max scalefac for global gain */
  maxSf = FDK_INT_MIN;
  for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
       sfbOffs += psyOutChannel->sfbPerGroup) {
    for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
      maxSf = fixMax(maxSf, scf[sfbOffs + sfb]);
    }
  }

  /* calc loop scalefactors, if spec is not all zero (i.e. maxSf == -99) */
  if (maxSf > FDK_INT_MIN) {
    *globalGain = maxSf;
    for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
         sfbOffs += psyOutChannel->sfbPerGroup) {
      for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
        if (scf[sfbOffs + sfb] == FDK_INT_MIN) {
          scf[sfbOffs + sfb] = 0;
          /* set band explicitely to zero */
          for (j = psyOutChannel->sfbOffsets[sfbOffs + sfb];
               j < psyOutChannel->sfbOffsets[sfbOffs + sfb + 1]; j++) {
            qcOutChannel->mdctSpectrum[j] = FL2FXCONST_DBL(0.0f);
          }
        } else {
          scf[sfbOffs + sfb] = maxSf - scf[sfbOffs + sfb];
        }
      }
    }
  } else {
    *globalGain = 0;
    /* set spectrum explicitely to zero */
    for (sfbOffs = 0; sfbOffs < psyOutChannel->sfbCnt;
         sfbOffs += psyOutChannel->sfbPerGroup) {
      for (sfb = 0; sfb < psyOutChannel->maxSfbPerGroup; sfb++) {
        scf[sfbOffs + sfb] = 0;
        /* set band explicitely to zero */
        for (j = psyOutChannel->sfbOffsets[sfbOffs + sfb];
             j < psyOutChannel->sfbOffsets[sfbOffs + sfb + 1]; j++) {
          qcOutChannel->mdctSpectrum[j] = FL2FXCONST_DBL(0.0f);
        }
      }
    }
  }

  /* free quantSpecTmp from scratch */
  C_ALLOC_SCRATCH_END(quantSpecTmp, SHORT, (1024))
}

void FDKaacEnc_EstimateScaleFactors(PSY_OUT_CHANNEL *psyOutChannel[],
                                    QC_OUT_CHANNEL *qcOutChannel[],
                                    const INT invQuant,
                                    const INT dZoneQuantEnable,
                                    const INT nChannels) {
  int ch;

  for (ch = 0; ch < nChannels; ch++) {
    FDKaacEnc_EstimateScaleFactorsChannel(
        qcOutChannel[ch], psyOutChannel[ch], qcOutChannel[ch]->scf,
        &qcOutChannel[ch]->globalGain, qcOutChannel[ch]->sfbFormFactorLdData,
        invQuant, qcOutChannel[ch]->quantSpec, dZoneQuantEnable);
  }
}
