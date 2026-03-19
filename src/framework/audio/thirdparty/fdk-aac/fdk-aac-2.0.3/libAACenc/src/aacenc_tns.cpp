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

   Author(s):   Alex Groeschel, Tobias Chalupka

   Description: Temporal noise shaping

*******************************************************************************/

#include "aacenc_tns.h"
#include "psy_const.h"
#include "psy_configuration.h"
#include "tns_func.h"
#include "aacEnc_rom.h"
#include "aacenc_tns.h"
#include "FDK_lpc.h"

#define FILTER_DIRECTION 0 /* 0 = up, 1 = down */

static const FIXP_DBL acfWindowLong[12 + 3 + 1] = {
    0x7fffffff, 0x7fb80000, 0x7ee00000, 0x7d780000, 0x7b800000, 0x78f80000,
    0x75e00000, 0x72380000, 0x6e000000, 0x69380000, 0x63e00000, 0x5df80000,
    0x57800000, 0x50780000, 0x48e00000, 0x40b80000};

static const FIXP_DBL acfWindowShort[4 + 3 + 1] = {
    0x7fffffff, 0x7e000000, 0x78000000, 0x6e000000,
    0x60000000, 0x4e000000, 0x38000000, 0x1e000000};

typedef struct {
  INT bitRateFrom[2];                  /* noneSbr=0, useSbr=1 */
  INT bitRateTo[2];                    /* noneSbr=0, useSbr=1 */
  TNS_PARAMETER_TABULATED paramTab[2]; /* mono=0, stereo=1 */

} TNS_INFO_TAB;

#define TNS_TIMERES_SCALE (1)
#define FL2_TIMERES_FIX(a) (FL2FXCONST_DBL(a / (float)(1 << TNS_TIMERES_SCALE)))

static const TNS_INFO_TAB tnsInfoTab[] = {
    {{16000, 13500},
     {32000, 28000},
     {{{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 12},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.2f)},
       1},
      {{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 12},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.2f)},
       1}}},
    {{32001, 28001},
     {60000, 52000},
     {{{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 10},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.0f)},
       1},
      {{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 10},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.0f)},
       1}}},
    {{60001, 52001},
     {384000, 384000},
     {{{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 8},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.0f)},
       1},
      {{1, 1},
       {1437, 1500},
       {1400, 600},
       {12, 8},
       {FILTER_DIRECTION, FILTER_DIRECTION},
       {3, 1},
       {FL2_TIMERES_FIX(0.4f), FL2_TIMERES_FIX(1.0f)},
       1}}}};

typedef struct {
  INT samplingRate;
  SCHAR maxBands[2]; /* long=0; short=1 */

} TNS_MAX_TAB_ENTRY;

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab1024[] = {
    {96000, {31, 9}},  {88200, {31, 9}},  {64000, {34, 10}}, {48000, {40, 14}},
    {44100, {42, 14}}, {32000, {51, 14}}, {24000, {46, 14}}, {22050, {46, 14}},
    {16000, {42, 14}}, {12000, {42, 14}}, {11025, {42, 14}}, {8000, {39, 14}}};

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab120[] = {
    {48000, {12, -1}}, /* 48000 */
    {44100, {12, -1}}, /* 44100 */
    {32000, {15, -1}}, /* 32000 */
    {24000, {15, -1}}, /* 24000 */
    {22050, {15, -1}}  /* 22050 */
};

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab128[] = {
    {48000, {12, -1}}, /* 48000 */
    {44100, {12, -1}}, /* 44100 */
    {32000, {15, -1}}, /* 32000 */
    {24000, {15, -1}}, /* 24000 */
    {22050, {15, -1}}  /* 22050 */
};

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab240[] = {
    {96000, {22, -1}}, /* 96000 */
    {48000, {22, -1}}, /* 48000 */
    {44100, {22, -1}}, /* 44100 */
    {32000, {21, -1}}, /* 32000 */
    {24000, {21, -1}}, /* 24000 */
    {22050, {21, -1}}  /* 22050 */
};

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab256[] = {
    {96000, {25, -1}}, /* 96000 */
    {48000, {25, -1}}, /* 48000 */
    {44100, {25, -1}}, /* 44100 */
    {32000, {24, -1}}, /* 32000 */
    {24000, {24, -1}}, /* 24000 */
    {22050, {24, -1}}  /* 22050 */
};
static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab480[] = {{48000, {31, -1}},
                                                      {44100, {32, -1}},
                                                      {32000, {37, -1}},
                                                      {24000, {30, -1}},
                                                      {22050, {30, -1}}};

static const TNS_MAX_TAB_ENTRY tnsMaxBandsTab512[] = {{48000, {31, -1}},
                                                      {44100, {32, -1}},
                                                      {32000, {37, -1}},
                                                      {24000, {31, -1}},
                                                      {22050, {31, -1}}};

static void FDKaacEnc_Parcor2Index(const FIXP_LPC *parcor, INT *RESTRICT index,
                                   const INT order, const INT bitsPerCoeff);

static void FDKaacEnc_Index2Parcor(const INT *index, FIXP_LPC *RESTRICT parcor,
                                   const INT order, const INT bitsPerCoeff);

static void FDKaacEnc_CalcGaussWindow(FIXP_DBL *win, const int winSize,
                                      const INT samplingRate,
                                      const INT transformResolution,
                                      const FIXP_DBL timeResolution,
                                      const INT timeResolution_e);

static const TNS_PARAMETER_TABULATED *FDKaacEnc_GetTnsParam(const INT bitRate,
                                                            const INT channels,
                                                            const INT sbrLd) {
  int i;
  const TNS_PARAMETER_TABULATED *tnsConfigTab = NULL;

  for (i = 0; i < (int)(sizeof(tnsInfoTab) / sizeof(TNS_INFO_TAB)); i++) {
    if ((bitRate >= tnsInfoTab[i].bitRateFrom[sbrLd ? 1 : 0]) &&
        bitRate <= tnsInfoTab[i].bitRateTo[sbrLd ? 1 : 0]) {
      tnsConfigTab = &tnsInfoTab[i].paramTab[(channels == 1) ? 0 : 1];
    }
  }

  return tnsConfigTab;
}

static INT getTnsMaxBands(const INT sampleRate, const INT granuleLength,
                          const INT isShortBlock) {
  int i;
  INT numBands = -1;
  const TNS_MAX_TAB_ENTRY *pMaxBandsTab = NULL;
  int maxBandsTabSize = 0;

  switch (granuleLength) {
    case 960:
    case 1024:
      pMaxBandsTab = tnsMaxBandsTab1024;
      maxBandsTabSize = sizeof(tnsMaxBandsTab1024) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 120:
      pMaxBandsTab = tnsMaxBandsTab120;
      maxBandsTabSize = sizeof(tnsMaxBandsTab120) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 128:
      pMaxBandsTab = tnsMaxBandsTab128;
      maxBandsTabSize = sizeof(tnsMaxBandsTab128) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 240:
      pMaxBandsTab = tnsMaxBandsTab240;
      maxBandsTabSize = sizeof(tnsMaxBandsTab240) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 256:
      pMaxBandsTab = tnsMaxBandsTab256;
      maxBandsTabSize = sizeof(tnsMaxBandsTab256) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 480:
      pMaxBandsTab = tnsMaxBandsTab480;
      maxBandsTabSize = sizeof(tnsMaxBandsTab480) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    case 512:
      pMaxBandsTab = tnsMaxBandsTab512;
      maxBandsTabSize = sizeof(tnsMaxBandsTab512) / sizeof(TNS_MAX_TAB_ENTRY);
      break;
    default:
      numBands = -1;
  }

  if (pMaxBandsTab != NULL) {
    for (i = 0; i < maxBandsTabSize; i++) {
      numBands = pMaxBandsTab[i].maxBands[(!isShortBlock) ? 0 : 1];
      if (sampleRate >= pMaxBandsTab[i].samplingRate) {
        break;
      }
    }
  }

  return numBands;
}

/***************************************************************************/
/*!
  \brief     FDKaacEnc_FreqToBandWidthRounding

  Returns index of nearest band border

  \param frequency
  \param sampling frequency
  \param total number of bands
  \param pointer to table of band borders

  \return band border
****************************************************************************/

INT FDKaacEnc_FreqToBandWidthRounding(const INT freq, const INT fs,
                                      const INT numOfBands,
                                      const INT *bandStartOffset) {
  INT lineNumber, band;

  /*  assert(freq >= 0);  */
  lineNumber = (freq * bandStartOffset[numOfBands] * 4 / fs + 1) / 2;

  /* freq > fs/2 */
  if (lineNumber >= bandStartOffset[numOfBands]) return numOfBands;

  /* find band the line number lies in */
  for (band = 0; band < numOfBands; band++) {
    if (bandStartOffset[band + 1] > lineNumber) break;
  }

  /* round to nearest band border */
  if (lineNumber - bandStartOffset[band] >
      bandStartOffset[band + 1] - lineNumber) {
    band++;
  }

  return (band);
}

/*****************************************************************************

    functionname: FDKaacEnc_InitTnsConfiguration
    description:  fill TNS_CONFIG structure with sensible content
    returns:
    input:        bitrate, samplerate, number of channels,
                  blocktype (long or short),
                  TNS Config struct (modified),
                  psy config struct,
                  tns active flag
    output:

*****************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_InitTnsConfiguration(
    INT bitRate, INT sampleRate, INT channels, INT blockType, INT granuleLength,
    INT isLowDelay, INT ldSbrPresent, TNS_CONFIG *tC, PSY_CONFIGURATION *pC,
    INT active, INT useTnsPeak) {
  int i;
  // float acfTimeRes   = (blockType == SHORT_WINDOW) ? 0.125f : 0.046875f;

  if (channels <= 0) return (AAC_ENCODER_ERROR)1;

  tC->isLowDelay = isLowDelay;

  /* initialize TNS filter flag, order, and coefficient resolution (in bits per
   * coeff) */
  tC->tnsActive = (active) ? TRUE : FALSE;
  tC->maxOrder = (blockType == SHORT_WINDOW) ? 5 : 12; /* maximum: 7, 20 */
  if (bitRate < 16000) tC->maxOrder -= 2;
  tC->coefRes = (blockType == SHORT_WINDOW) ? 3 : 4;

  /* LPC stop line: highest MDCT line to be coded, but do not go beyond
   * TNS_MAX_BANDS! */
  tC->lpcStopBand = getTnsMaxBands(sampleRate, granuleLength,
                                   (blockType == SHORT_WINDOW) ? 1 : 0);

  if (tC->lpcStopBand < 0) {
    return (AAC_ENCODER_ERROR)1;
  }

  tC->lpcStopBand = fMin(tC->lpcStopBand, pC->sfbActive);
  tC->lpcStopLine = pC->sfbOffset[tC->lpcStopBand];

  switch (granuleLength) {
    case 960:
    case 1024:
      /* TNS start line: skip lower MDCT lines to prevent artifacts due to
       * filter mismatch */
      if (blockType == SHORT_WINDOW) {
        tC->lpcStartBand[LOFILT] = 0;
      } else {
        tC->lpcStartBand[LOFILT] =
            (sampleRate < 9391) ? 2 : ((sampleRate < 18783) ? 4 : 8);
      }
      tC->lpcStartLine[LOFILT] = pC->sfbOffset[tC->lpcStartBand[LOFILT]];

      i = tC->lpcStopBand;
      while (pC->sfbOffset[i] >
             (tC->lpcStartLine[LOFILT] +
              (tC->lpcStopLine - tC->lpcStartLine[LOFILT]) / 4))
        i--;
      tC->lpcStartBand[HIFILT] = i;
      tC->lpcStartLine[HIFILT] = pC->sfbOffset[i];

      tC->confTab.threshOn[HIFILT] = 1437;
      tC->confTab.threshOn[LOFILT] = 1500;

      tC->confTab.tnsLimitOrder[HIFILT] = tC->maxOrder;
      tC->confTab.tnsLimitOrder[LOFILT] = fMax(0, tC->maxOrder - 7);

      tC->confTab.tnsFilterDirection[HIFILT] = FILTER_DIRECTION;
      tC->confTab.tnsFilterDirection[LOFILT] = FILTER_DIRECTION;

      tC->confTab.acfSplit[HIFILT] =
          -1; /* signal Merged4to2QuartersAutoCorrelation in
                 FDKaacEnc_MergedAutoCorrelation*/
      tC->confTab.acfSplit[LOFILT] =
          -1; /* signal Merged4to2QuartersAutoCorrelation in
                 FDKaacEnc_MergedAutoCorrelation */

      tC->confTab.filterEnabled[HIFILT] = 1;
      tC->confTab.filterEnabled[LOFILT] = 1;
      tC->confTab.seperateFiltersAllowed = 1;

      /* compute autocorrelation window based on maximum filter order for given
       * block type */
      /* for (i = 0; i <= tC->maxOrder + 3; i++) {
           float acfWinTemp = acfTimeRes * i;
           acfWindow[i] = FL2FXCONST_DBL(1.0f - acfWinTemp * acfWinTemp);
         }
      */
      if (blockType == SHORT_WINDOW) {
        FDKmemcpy(tC->acfWindow[HIFILT], acfWindowShort,
                  fMin((LONG)sizeof(acfWindowShort),
                       (LONG)sizeof(tC->acfWindow[HIFILT])));
        FDKmemcpy(tC->acfWindow[LOFILT], acfWindowShort,
                  fMin((LONG)sizeof(acfWindowShort),
                       (LONG)sizeof(tC->acfWindow[HIFILT])));
      } else {
        FDKmemcpy(tC->acfWindow[HIFILT], acfWindowLong,
                  fMin((LONG)sizeof(acfWindowLong),
                       (LONG)sizeof(tC->acfWindow[HIFILT])));
        FDKmemcpy(tC->acfWindow[LOFILT], acfWindowLong,
                  fMin((LONG)sizeof(acfWindowLong),
                       (LONG)sizeof(tC->acfWindow[HIFILT])));
      }
      break;
    case 480:
    case 512: {
      const TNS_PARAMETER_TABULATED *pCfg =
          FDKaacEnc_GetTnsParam(bitRate, channels, ldSbrPresent);
      if (pCfg != NULL) {
        FDKmemcpy(&(tC->confTab), pCfg, sizeof(tC->confTab));

        tC->lpcStartBand[HIFILT] = FDKaacEnc_FreqToBandWidthRounding(
            pCfg->filterStartFreq[HIFILT], sampleRate, pC->sfbCnt,
            pC->sfbOffset);
        tC->lpcStartLine[HIFILT] = pC->sfbOffset[tC->lpcStartBand[HIFILT]];
        tC->lpcStartBand[LOFILT] = FDKaacEnc_FreqToBandWidthRounding(
            pCfg->filterStartFreq[LOFILT], sampleRate, pC->sfbCnt,
            pC->sfbOffset);
        tC->lpcStartLine[LOFILT] = pC->sfbOffset[tC->lpcStartBand[LOFILT]];

        FDKaacEnc_CalcGaussWindow(
            tC->acfWindow[HIFILT], tC->maxOrder + 1, sampleRate, granuleLength,
            pCfg->tnsTimeResolution[HIFILT], TNS_TIMERES_SCALE);
        FDKaacEnc_CalcGaussWindow(
            tC->acfWindow[LOFILT], tC->maxOrder + 1, sampleRate, granuleLength,
            pCfg->tnsTimeResolution[LOFILT], TNS_TIMERES_SCALE);
      } else {
        tC->tnsActive =
            FALSE; /* no configuration available, disable tns tool */
      }
    } break;
    default:
      tC->tnsActive = FALSE; /* no configuration available, disable tns tool */
  }

  return AAC_ENC_OK;
}

/***************************************************************************/
/*!
  \brief     FDKaacEnc_ScaleUpSpectrum

  Scales up spectrum lines in a given frequency section

  \param scaled spectrum
  \param original spectrum
  \param frequency line to start scaling
  \param frequency line to enc scaling

  \return scale factor

****************************************************************************/
static inline INT FDKaacEnc_ScaleUpSpectrum(FIXP_DBL *dest, const FIXP_DBL *src,
                                            const INT startLine,
                                            const INT stopLine) {
  INT i, scale;

  FIXP_DBL maxVal = FL2FXCONST_DBL(0.f);

  /* Get highest value in given spectrum */
  for (i = startLine; i < stopLine; i++) {
    maxVal = fixMax(maxVal, fixp_abs(src[i]));
  }
  scale = CountLeadingBits(maxVal);

  /* Scale spectrum according to highest value */
  for (i = startLine; i < stopLine; i++) {
    dest[i] = src[i] << scale;
  }

  return scale;
}

/***************************************************************************/
/*!
  \brief     FDKaacEnc_CalcAutoCorrValue

  Calculate autocorellation value for one lag

  \param pointer to spectrum
  \param start line
  \param stop line
  \param lag to be calculated
  \param scaling of the lag

****************************************************************************/
static inline FIXP_DBL FDKaacEnc_CalcAutoCorrValue(const FIXP_DBL *spectrum,
                                                   const INT startLine,
                                                   const INT stopLine,
                                                   const INT lag,
                                                   const INT scale) {
  int i;
  FIXP_DBL result = FL2FXCONST_DBL(0.f);

  /* This versions allows to save memory accesses, when computing pow2 */
  /* It is of interest for ARM, XTENSA without parallel memory access  */
  if (lag == 0) {
    for (i = startLine; i < stopLine; i++) {
      result += (fPow2(spectrum[i]) >> scale);
    }
  } else {
    for (i = startLine; i < (stopLine - lag); i++) {
      result += (fMult(spectrum[i], spectrum[i + lag]) >> scale);
    }
  }

  return result;
}

/***************************************************************************/
/*!
  \brief     FDKaacEnc_AutoCorrNormFac

  Autocorrelation function for 1st and 2nd half of the spectrum

  \param pointer to spectrum
  \param pointer to autocorrelation window
  \param filter start line

****************************************************************************/
static inline FIXP_DBL FDKaacEnc_AutoCorrNormFac(const FIXP_DBL value,
                                                 const INT scale, INT *sc) {
#define HLM_MIN_NRG 0.0000000037252902984619140625f /* 2^-28 */
#define MAX_INV_NRGFAC (1.f / HLM_MIN_NRG)

  FIXP_DBL retValue;
  FIXP_DBL A, B;

  if (scale >= 0) {
    A = value;
    B = FL2FXCONST_DBL(HLM_MIN_NRG) >> fixMin(DFRACT_BITS - 1, scale);
  } else {
    A = value >> fixMin(DFRACT_BITS - 1, (-scale));
    B = FL2FXCONST_DBL(HLM_MIN_NRG);
  }

  if (A > B) {
    int shift = 0;
    FIXP_DBL tmp = invSqrtNorm2(value, &shift);

    retValue = fMult(tmp, tmp);
    *sc += (2 * shift);
  } else {
    /* MAX_INV_NRGFAC*FDKpow(2,-28) = 1/2^-28 * 2^-28 = 1.0 */
    retValue =
        /*FL2FXCONST_DBL(MAX_INV_NRGFAC*FDKpow(2,-28))*/ (FIXP_DBL)MAXVAL_DBL;
    *sc += scale + 28;
  }

  return retValue;
}

static void FDKaacEnc_MergedAutoCorrelation(
    const FIXP_DBL *spectrum, const INT isLowDelay,
    const FIXP_DBL acfWindow[MAX_NUM_OF_FILTERS][TNS_MAX_ORDER + 3 + 1],
    const INT lpcStartLine[MAX_NUM_OF_FILTERS], const INT lpcStopLine,
    const INT maxOrder, const INT acfSplit[MAX_NUM_OF_FILTERS], FIXP_DBL *_rxx1,
    FIXP_DBL *_rxx2) {
  int i, idx0, idx1, idx2, idx3, idx4, lag;
  FIXP_DBL rxx1_0, rxx2_0, rxx3_0, rxx4_0;

  /* buffer for temporal spectrum */
  C_ALLOC_SCRATCH_START(pSpectrum, FIXP_DBL, (1024))

  /* MDCT line indices separating the 1st, 2nd, 3rd, and 4th analysis quarters
   */
  if ((acfSplit[LOFILT] == -1) || (acfSplit[HIFILT] == -1)) {
    /* autocorrelation function for 1st, 2nd, 3rd, and 4th quarter of the
     * spectrum */
    idx0 = lpcStartLine[LOFILT];
    i = lpcStopLine - lpcStartLine[LOFILT];
    idx1 = idx0 + i / 4;
    idx2 = idx0 + i / 2;
    idx3 = idx0 + i * 3 / 4;
    idx4 = lpcStopLine;
  } else {
    FDK_ASSERT(acfSplit[LOFILT] == 1);
    FDK_ASSERT(acfSplit[HIFILT] == 3);
    i = (lpcStopLine - lpcStartLine[HIFILT]) / 3;
    idx0 = lpcStartLine[LOFILT];
    idx1 = lpcStartLine[HIFILT];
    idx2 = idx1 + i;
    idx3 = idx2 + i;
    idx4 = lpcStopLine;
  }

  /* copy spectrum to temporal buffer and scale up as much as possible */
  INT sc1 = FDKaacEnc_ScaleUpSpectrum(pSpectrum, spectrum, idx0, idx1);
  INT sc2 = FDKaacEnc_ScaleUpSpectrum(pSpectrum, spectrum, idx1, idx2);
  INT sc3 = FDKaacEnc_ScaleUpSpectrum(pSpectrum, spectrum, idx2, idx3);
  INT sc4 = FDKaacEnc_ScaleUpSpectrum(pSpectrum, spectrum, idx3, idx4);

  /* get scaling values for summation */
  INT nsc1, nsc2, nsc3, nsc4;
  for (nsc1 = 1; (1 << nsc1) < (idx1 - idx0); nsc1++)
    ;
  for (nsc2 = 1; (1 << nsc2) < (idx2 - idx1); nsc2++)
    ;
  for (nsc3 = 1; (1 << nsc3) < (idx3 - idx2); nsc3++)
    ;
  for (nsc4 = 1; (1 << nsc4) < (idx4 - idx3); nsc4++)
    ;

  /* compute autocorrelation value at lag zero, i. e. energy, for each quarter
   */
  rxx1_0 = FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx0, idx1, 0, nsc1);
  rxx2_0 = FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx1, idx2, 0, nsc2);
  rxx3_0 = FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx2, idx3, 0, nsc3);
  rxx4_0 = FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx3, idx4, 0, nsc4);

  /* compute energy normalization factors, i. e. 1/energy (saves some divisions)
   */
  if (rxx1_0 != FL2FXCONST_DBL(0.f)) {
    INT sc_fac1 = -1;
    FIXP_DBL fac1 =
        FDKaacEnc_AutoCorrNormFac(rxx1_0, ((-2 * sc1) + nsc1), &sc_fac1);
    _rxx1[0] = scaleValue(fMult(rxx1_0, fac1), sc_fac1);

    if (isLowDelay) {
      for (lag = 1; lag <= maxOrder; lag++) {
        /* compute energy-normalized and windowed autocorrelation values at this
         * lag */
        FIXP_DBL x1 =
            FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx0, idx1, lag, nsc1);
        _rxx1[lag] =
            fMult(scaleValue(fMult(x1, fac1), sc_fac1), acfWindow[LOFILT][lag]);
      }
    } else {
      for (lag = 1; lag <= maxOrder; lag++) {
        if ((3 * lag) <= maxOrder + 3) {
          FIXP_DBL x1 =
              FDKaacEnc_CalcAutoCorrValue(pSpectrum, idx0, idx1, lag, nsc1);
          _rxx1[lag] = fMult(scaleValue(fMult(x1, fac1), sc_fac1),
                             acfWindow[LOFILT][3 * lag]);
        }
      }
    }
  }

  /* auto corr over upper 3/4 of spectrum */
  if (!((rxx2_0 == FL2FXCONST_DBL(0.f)) && (rxx3_0 == FL2FXCONST_DBL(0.f)) &&
        (rxx4_0 == FL2FXCONST_DBL(0.f)))) {
    FIXP_DBL fac2, fac3, fac4;
    fac2 = fac3 = fac4 = FL2FXCONST_DBL(0.f);
    INT sc_fac2, sc_fac3, sc_fac4;
    sc_fac2 = sc_fac3 = sc_fac4 = 0;

    if (rxx2_0 != FL2FXCONST_DBL(0.f)) {
      fac2 = FDKaacEnc_AutoCorrNormFac(rxx2_0, ((-2 * sc2) + nsc2), &sc_fac2);
      sc_fac2 -= 2;
    }
    if (rxx3_0 != FL2FXCONST_DBL(0.f)) {
      fac3 = FDKaacEnc_AutoCorrNormFac(rxx3_0, ((-2 * sc3) + nsc3), &sc_fac3);
      sc_fac3 -= 2;
    }
    if (rxx4_0 != FL2FXCONST_DBL(0.f)) {
      fac4 = FDKaacEnc_AutoCorrNormFac(rxx4_0, ((-2 * sc4) + nsc4), &sc_fac4);
      sc_fac4 -= 2;
    }

    _rxx2[0] = scaleValue(fMult(rxx2_0, fac2), sc_fac2) +
               scaleValue(fMult(rxx3_0, fac3), sc_fac3) +
               scaleValue(fMult(rxx4_0, fac4), sc_fac4);

    for (lag = 1; lag <= maxOrder; lag++) {
      /* merge quarters 2, 3, 4 into one autocorrelation; quarter 1 stays
       * separate */
      FIXP_DBL x2 = scaleValue(fMult(FDKaacEnc_CalcAutoCorrValue(
                                         pSpectrum, idx1, idx2, lag, nsc2),
                                     fac2),
                               sc_fac2) +
                    scaleValue(fMult(FDKaacEnc_CalcAutoCorrValue(
                                         pSpectrum, idx2, idx3, lag, nsc3),
                                     fac3),
                               sc_fac3) +
                    scaleValue(fMult(FDKaacEnc_CalcAutoCorrValue(
                                         pSpectrum, idx3, idx4, lag, nsc4),
                                     fac4),
                               sc_fac4);

      _rxx2[lag] = fMult(x2, acfWindow[HIFILT][lag]);
    }
  }

  C_ALLOC_SCRATCH_END(pSpectrum, FIXP_DBL, (1024))
}

/*****************************************************************************
    functionname: FDKaacEnc_TnsDetect
    description:  do decision, if TNS shall be used or not
    returns:
    input:        tns data structure (modified),
                  tns config structure,
                  scalefactor size and table,
                  spectrum,
                  subblock num, blocktype,
                  sfb-wise energy.

*****************************************************************************/
INT FDKaacEnc_TnsDetect(TNS_DATA *tnsData, const TNS_CONFIG *tC,
                        TNS_INFO *tnsInfo, INT sfbCnt, const FIXP_DBL *spectrum,
                        INT subBlockNumber, INT blockType) {
  /* autocorrelation function for 1st, 2nd, 3rd, and 4th quarter of the
   * spectrum. */
  FIXP_DBL rxx1[TNS_MAX_ORDER + 1]; /* higher part */
  FIXP_DBL rxx2[TNS_MAX_ORDER + 1]; /* lower part */
  FIXP_LPC parcor_tmp[TNS_MAX_ORDER];

  int i;

  FDKmemclear(rxx1, sizeof(rxx1));
  FDKmemclear(rxx2, sizeof(rxx2));

  TNS_SUBBLOCK_INFO *tsbi =
      (blockType == SHORT_WINDOW)
          ? &tnsData->dataRaw.Short.subBlockInfo[subBlockNumber]
          : &tnsData->dataRaw.Long.subBlockInfo;

  tnsData->filtersMerged = FALSE;

  tsbi->tnsActive[HIFILT] = FALSE;
  tsbi->predictionGain[HIFILT] = 1000;
  tsbi->tnsActive[LOFILT] = FALSE;
  tsbi->predictionGain[LOFILT] = 1000;

  tnsInfo->numOfFilters[subBlockNumber] = 0;
  tnsInfo->coefRes[subBlockNumber] = tC->coefRes;
  for (i = 0; i < tC->maxOrder; i++) {
    tnsInfo->coef[subBlockNumber][HIFILT][i] =
        tnsInfo->coef[subBlockNumber][LOFILT][i] = 0;
  }

  tnsInfo->length[subBlockNumber][HIFILT] =
      tnsInfo->length[subBlockNumber][LOFILT] = 0;
  tnsInfo->order[subBlockNumber][HIFILT] =
      tnsInfo->order[subBlockNumber][LOFILT] = 0;

  if ((tC->tnsActive) && (tC->maxOrder > 0)) {
    int sumSqrCoef;

    FDKaacEnc_MergedAutoCorrelation(
        spectrum, tC->isLowDelay, tC->acfWindow, tC->lpcStartLine,
        tC->lpcStopLine, tC->maxOrder, tC->confTab.acfSplit, rxx1, rxx2);

    /* compute higher TNS filter coefficients in lattice form (ParCor) with
     * LeRoux-Gueguen/Schur algorithm */
    {
      FIXP_DBL predictionGain_m;
      INT predictionGain_e;

      CLpc_AutoToParcor(rxx2, 0, parcor_tmp, tC->confTab.tnsLimitOrder[HIFILT],
                        &predictionGain_m, &predictionGain_e);
      tsbi->predictionGain[HIFILT] =
          (INT)fMultNorm(predictionGain_m, predictionGain_e, 1000, 31, 31);
    }

    /* non-linear quantization of TNS lattice coefficients with given resolution
     */
    FDKaacEnc_Parcor2Index(parcor_tmp, tnsInfo->coef[subBlockNumber][HIFILT],
                           tC->confTab.tnsLimitOrder[HIFILT], tC->coefRes);

    /* reduce filter order by truncating trailing zeros, compute sum(abs(coefs))
     */
    for (i = tC->confTab.tnsLimitOrder[HIFILT] - 1; i >= 0; i--) {
      if (tnsInfo->coef[subBlockNumber][HIFILT][i] != 0) {
        break;
      }
    }

    tnsInfo->order[subBlockNumber][HIFILT] = i + 1;

    sumSqrCoef = 0;
    for (; i >= 0; i--) {
      sumSqrCoef += tnsInfo->coef[subBlockNumber][HIFILT][i] *
                    tnsInfo->coef[subBlockNumber][HIFILT][i];
    }

    tnsInfo->direction[subBlockNumber][HIFILT] =
        tC->confTab.tnsFilterDirection[HIFILT];
    tnsInfo->length[subBlockNumber][HIFILT] = sfbCnt - tC->lpcStartBand[HIFILT];

    /* disable TNS if predictionGain is less than 3dB or sumSqrCoef is too small
     */
    if ((tsbi->predictionGain[HIFILT] > tC->confTab.threshOn[HIFILT]) ||
        (sumSqrCoef > (tC->confTab.tnsLimitOrder[HIFILT] / 2 + 2))) {
      tsbi->tnsActive[HIFILT] = TRUE;
      tnsInfo->numOfFilters[subBlockNumber]++;

      /* compute second filter for lower quarter; only allowed for long windows!
       */
      if ((blockType != SHORT_WINDOW) && (tC->confTab.filterEnabled[LOFILT]) &&
          (tC->confTab.seperateFiltersAllowed)) {
        /* compute second filter for lower frequencies */

        /* compute TNS filter in lattice (ParCor) form with LeRoux-Gueguen
         * algorithm */
        INT predGain;
        {
          FIXP_DBL predictionGain_m;
          INT predictionGain_e;

          CLpc_AutoToParcor(rxx1, 0, parcor_tmp,
                            tC->confTab.tnsLimitOrder[LOFILT],
                            &predictionGain_m, &predictionGain_e);
          predGain =
              (INT)fMultNorm(predictionGain_m, predictionGain_e, 1000, 31, 31);
        }

        /* non-linear quantization of TNS lattice coefficients with given
         * resolution */
        FDKaacEnc_Parcor2Index(parcor_tmp,
                               tnsInfo->coef[subBlockNumber][LOFILT],
                               tC->confTab.tnsLimitOrder[LOFILT], tC->coefRes);

        /* reduce filter order by truncating trailing zeros, compute
         * sum(abs(coefs)) */
        for (i = tC->confTab.tnsLimitOrder[LOFILT] - 1; i >= 0; i--) {
          if (tnsInfo->coef[subBlockNumber][LOFILT][i] != 0) {
            break;
          }
        }
        tnsInfo->order[subBlockNumber][LOFILT] = i + 1;

        sumSqrCoef = 0;
        for (; i >= 0; i--) {
          sumSqrCoef += tnsInfo->coef[subBlockNumber][LOFILT][i] *
                        tnsInfo->coef[subBlockNumber][LOFILT][i];
        }

        tnsInfo->direction[subBlockNumber][LOFILT] =
            tC->confTab.tnsFilterDirection[LOFILT];
        tnsInfo->length[subBlockNumber][LOFILT] =
            tC->lpcStartBand[HIFILT] - tC->lpcStartBand[LOFILT];

        /* filter lower quarter if gain is high enough, but not if it's too high
         */
        if (((predGain > tC->confTab.threshOn[LOFILT]) &&
             (predGain < (16000 * tC->confTab.tnsLimitOrder[LOFILT]))) ||
            ((sumSqrCoef > 9) &&
             (sumSqrCoef < 22 * tC->confTab.tnsLimitOrder[LOFILT]))) {
          /* compare lower to upper filter; if they are very similar, merge them
           */
          tsbi->tnsActive[LOFILT] = TRUE;
          sumSqrCoef = 0;
          for (i = 0; i < tC->confTab.tnsLimitOrder[LOFILT]; i++) {
            sumSqrCoef += fAbs(tnsInfo->coef[subBlockNumber][HIFILT][i] -
                               tnsInfo->coef[subBlockNumber][LOFILT][i]);
          }
          if ((sumSqrCoef < 2) &&
              (tnsInfo->direction[subBlockNumber][LOFILT] ==
               tnsInfo->direction[subBlockNumber][HIFILT])) {
            tnsData->filtersMerged = TRUE;
            tnsInfo->length[subBlockNumber][HIFILT] =
                sfbCnt - tC->lpcStartBand[LOFILT];
            for (; i < tnsInfo->order[subBlockNumber][HIFILT]; i++) {
              if (fAbs(tnsInfo->coef[subBlockNumber][HIFILT][i]) > 1) {
                break;
              }
            }
            for (i--; i >= 0; i--) {
              if (tnsInfo->coef[subBlockNumber][HIFILT][i] != 0) {
                break;
              }
            }
            if (i < tnsInfo->order[subBlockNumber][HIFILT]) {
              tnsInfo->order[subBlockNumber][HIFILT] = i + 1;
            }
          } else {
            tnsInfo->numOfFilters[subBlockNumber]++;
          }
        } /* filter lower part */
        tsbi->predictionGain[LOFILT] = predGain;

      } /* second filter allowed  */
    }   /* if predictionGain > 1437 ... */
  }     /* maxOrder > 0 && tnsActive */

  return 0;
}

/***************************************************************************/
/*!
  \brief     FDKaacLdEnc_TnsSync

  synchronize TNS parameters when TNS gain difference small (relative)

  \param pointer to TNS data structure (destination)
  \param pointer to TNS data structure (source)
  \param pointer to TNS config structure
  \param number of sub-block
  \param block type

  \return void
****************************************************************************/
void FDKaacEnc_TnsSync(TNS_DATA *tnsDataDest, const TNS_DATA *tnsDataSrc,
                       TNS_INFO *tnsInfoDest, TNS_INFO *tnsInfoSrc,
                       const INT blockTypeDest, const INT blockTypeSrc,
                       const TNS_CONFIG *tC) {
  int i, w, absDiff, nWindows;
  TNS_SUBBLOCK_INFO *sbInfoDest;
  const TNS_SUBBLOCK_INFO *sbInfoSrc;

  /* if one channel contains short blocks and the other not, do not synchronize
   */
  if ((blockTypeSrc == SHORT_WINDOW && blockTypeDest != SHORT_WINDOW) ||
      (blockTypeDest == SHORT_WINDOW && blockTypeSrc != SHORT_WINDOW)) {
    return;
  }

  if (blockTypeDest != SHORT_WINDOW) {
    sbInfoDest = &tnsDataDest->dataRaw.Long.subBlockInfo;
    sbInfoSrc = &tnsDataSrc->dataRaw.Long.subBlockInfo;
    nWindows = 1;
  } else {
    sbInfoDest = &tnsDataDest->dataRaw.Short.subBlockInfo[0];
    sbInfoSrc = &tnsDataSrc->dataRaw.Short.subBlockInfo[0];
    nWindows = 8;
  }

  for (w = 0; w < nWindows; w++) {
    const TNS_SUBBLOCK_INFO *pSbInfoSrcW = sbInfoSrc + w;
    TNS_SUBBLOCK_INFO *pSbInfoDestW = sbInfoDest + w;
    INT doSync = 1, absDiffSum = 0;

    /* if TNS is active in at least one channel, check if ParCor coefficients of
     * higher filter are similar */
    if (pSbInfoDestW->tnsActive[HIFILT] || pSbInfoSrcW->tnsActive[HIFILT]) {
      for (i = 0; i < tC->maxOrder; i++) {
        absDiff = fAbs(tnsInfoDest->coef[w][HIFILT][i] -
                       tnsInfoSrc->coef[w][HIFILT][i]);
        absDiffSum += absDiff;
        /* if coefficients diverge too much between channels, do not synchronize
         */
        if ((absDiff > 1) || (absDiffSum > 2)) {
          doSync = 0;
          break;
        }
      }

      if (doSync) {
        /* if no significant difference was detected, synchronize coefficient
         * sets */
        if (pSbInfoSrcW->tnsActive[HIFILT]) {
          /* no dest filter, or more dest than source filters: use one dest
           * filter */
          if ((!pSbInfoDestW->tnsActive[HIFILT]) ||
              ((pSbInfoDestW->tnsActive[HIFILT]) &&
               (tnsInfoDest->numOfFilters[w] > tnsInfoSrc->numOfFilters[w]))) {
            pSbInfoDestW->tnsActive[HIFILT] = tnsInfoDest->numOfFilters[w] = 1;
          }
          tnsDataDest->filtersMerged = tnsDataSrc->filtersMerged;
          tnsInfoDest->order[w][HIFILT] = tnsInfoSrc->order[w][HIFILT];
          tnsInfoDest->length[w][HIFILT] = tnsInfoSrc->length[w][HIFILT];
          tnsInfoDest->direction[w][HIFILT] = tnsInfoSrc->direction[w][HIFILT];
          tnsInfoDest->coefCompress[w][HIFILT] =
              tnsInfoSrc->coefCompress[w][HIFILT];

          for (i = 0; i < tC->maxOrder; i++) {
            tnsInfoDest->coef[w][HIFILT][i] = tnsInfoSrc->coef[w][HIFILT][i];
          }
        } else
          pSbInfoDestW->tnsActive[HIFILT] = tnsInfoDest->numOfFilters[w] = 0;
      }
    }
  }
}

/***************************************************************************/
/*!
  \brief     FDKaacEnc_TnsEncode

  perform TNS encoding

  \param pointer to TNS info structure
  \param pointer to TNS data structure
  \param number of sfbs
  \param pointer to TNS config structure
  \param low-pass line
  \param pointer to spectrum
  \param number of sub-block
  \param block type

  \return ERROR STATUS
****************************************************************************/
INT FDKaacEnc_TnsEncode(TNS_INFO *tnsInfo, TNS_DATA *tnsData,
                        const INT numOfSfb, const TNS_CONFIG *tC,
                        const INT lowPassLine, FIXP_DBL *spectrum,
                        const INT subBlockNumber, const INT blockType) {
  INT i, startLine, stopLine;

  if (((blockType == SHORT_WINDOW) &&
       (!tnsData->dataRaw.Short.subBlockInfo[subBlockNumber]
             .tnsActive[HIFILT])) ||
      ((blockType != SHORT_WINDOW) &&
       (!tnsData->dataRaw.Long.subBlockInfo.tnsActive[HIFILT]))) {
    return 1;
  }

  startLine = (tnsData->filtersMerged) ? tC->lpcStartLine[LOFILT]
                                       : tC->lpcStartLine[HIFILT];
  stopLine = tC->lpcStopLine;

  for (i = 0; i < tnsInfo->numOfFilters[subBlockNumber]; i++) {
    INT lpcGainFactor;
    FIXP_LPC LpcCoeff[TNS_MAX_ORDER];
    FIXP_DBL workBuffer[TNS_MAX_ORDER];
    FIXP_LPC parcor_tmp[TNS_MAX_ORDER];

    FDKaacEnc_Index2Parcor(tnsInfo->coef[subBlockNumber][i], parcor_tmp,
                           tnsInfo->order[subBlockNumber][i], tC->coefRes);

    lpcGainFactor = CLpc_ParcorToLpc(
        parcor_tmp, LpcCoeff, tnsInfo->order[subBlockNumber][i], workBuffer);

    FDKmemclear(workBuffer, TNS_MAX_ORDER * sizeof(FIXP_DBL));
    CLpc_Analysis(&spectrum[startLine], stopLine - startLine, LpcCoeff,
                  lpcGainFactor, tnsInfo->order[subBlockNumber][i], workBuffer,
                  NULL);

    /* update for second filter */
    startLine = tC->lpcStartLine[LOFILT];
    stopLine = tC->lpcStartLine[HIFILT];
  }

  return (0);
}

static void FDKaacEnc_CalcGaussWindow(FIXP_DBL *win, const int winSize,
                                      const INT samplingRate,
                                      const INT transformResolution,
                                      const FIXP_DBL timeResolution,
                                      const INT timeResolution_e) {
#define PI_E (2)
#define PI_M FL2FXCONST_DBL(3.1416f / (float)(1 << PI_E))

#define EULER_E (2)
#define EULER_M FL2FXCONST_DBL(2.7183 / (float)(1 << EULER_E))

#define COEFF_LOOP_SCALE (4)

  INT i, e1, e2, gaussExp_e;
  FIXP_DBL gaussExp_m;

  /* calc. window exponent from time resolution:
   *
   *   gaussExp = PI * samplingRate * 0.001f * timeResolution /
   * transformResolution; gaussExp = -0.5f * gaussExp * gaussExp;
   */
  gaussExp_m = fMultNorm(
      timeResolution,
      fMult(PI_M,
            fDivNorm((FIXP_DBL)(samplingRate),
                     (FIXP_DBL)(LONG)(transformResolution * 1000.f), &e1)),
      &e2);
  gaussExp_m = -fPow2Div2(gaussExp_m);
  gaussExp_e = 2 * (e1 + e2 + timeResolution_e + PI_E);

  FDK_ASSERT(winSize < (1 << COEFF_LOOP_SCALE));

  /* calc. window coefficients
   *   win[i] = (float)exp( gaussExp * (i+0.5) * (i+0.5) );
   */
  for (i = 0; i < winSize; i++) {
    win[i] = fPow(
        EULER_M, EULER_E,
        fMult(gaussExp_m,
              fPow2((i * FL2FXCONST_DBL(1.f / (float)(1 << COEFF_LOOP_SCALE)) +
                     FL2FXCONST_DBL(.5f / (float)(1 << COEFF_LOOP_SCALE))))),
        gaussExp_e + 2 * COEFF_LOOP_SCALE, &e1);

    win[i] = scaleValueSaturate(win[i], e1);
  }
}

static INT FDKaacEnc_Search3(FIXP_LPC parcor) {
  INT i, index = 0;

  for (i = 0; i < 8; i++) {
    if (parcor > FDKaacEnc_tnsCoeff3Borders[i]) index = i;
  }
  return (index - 4);
}

static INT FDKaacEnc_Search4(FIXP_LPC parcor) {
  INT i, index = 0;

  for (i = 0; i < 16; i++) {
    if (parcor > FDKaacEnc_tnsCoeff4Borders[i]) index = i;
  }
  return (index - 8);
}

/*****************************************************************************

    functionname: FDKaacEnc_Parcor2Index

*****************************************************************************/
static void FDKaacEnc_Parcor2Index(const FIXP_LPC *parcor, INT *RESTRICT index,
                                   const INT order, const INT bitsPerCoeff) {
  INT i;
  for (i = 0; i < order; i++) {
    if (bitsPerCoeff == 3)
      index[i] = FDKaacEnc_Search3(parcor[i]);
    else
      index[i] = FDKaacEnc_Search4(parcor[i]);
  }
}

/*****************************************************************************

    functionname: FDKaacEnc_Index2Parcor
    description:  inverse quantization for reflection coefficients
    returns:      -
    input:        quantized values, ptr. to reflection coefficients,
                  no. of coefficients, resolution
    output:       reflection coefficients

*****************************************************************************/
static void FDKaacEnc_Index2Parcor(const INT *index, FIXP_LPC *RESTRICT parcor,
                                   const INT order, const INT bitsPerCoeff) {
  INT i;
  for (i = 0; i < order; i++)
    parcor[i] = bitsPerCoeff == 4 ? FDKaacEnc_tnsEncCoeff4[index[i] + 8]
                                  : FDKaacEnc_tnsEncCoeff3[index[i] + 4];
}
