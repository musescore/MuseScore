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

   Author(s):   M. Neusinger

   Description: Compressor for AAC Metadata Generator

*******************************************************************************/

#include "metadata_compressor.h"
#include "channel_map.h"

#define LOG2 0.69314718056f /* natural logarithm of 2 */
#define ILOG2 1.442695041f  /* 1/LOG2 */
#define FIXP_ILOG2_DIV2 (FL2FXCONST_DBL(ILOG2 / 2))

/*----------------- defines ----------------------*/

#define MAX_DRC_CHANNELS (8)       /*!< Max number of audio input channels. */
#define DOWNMIX_SHIFT (3)          /*!< Max 8 channel. */
#define WEIGHTING_FILTER_SHIFT (2) /*!< Scaling used in weighting filter. */

#define METADATA_INT_BITS 10
#define METADATA_LINT_BITS 20
#define METADATA_INT_SCALE (INT64(1) << (METADATA_INT_BITS))
#define METADATA_FRACT_BITS (DFRACT_BITS - 1 - METADATA_INT_BITS)
#define METADATA_FRACT_SCALE (INT64(1) << (METADATA_FRACT_BITS))

/**
 *  Enum for channel assignment.
 */
enum { L = 0, R = 1, C = 2, LFE = 3, LS = 4, RS = 5, S = 6, LS2 = 7, RS2 = 8 };

/*--------------- structure definitions --------------------*/

/**
 *  Structure holds weighting filter filter states.
 */
struct WEIGHTING_STATES {
  FIXP_DBL x1;
  FIXP_DBL x2;
  FIXP_DBL y1;
  FIXP_DBL y2;
};

/**
 *  Dynamic Range Control compressor structure.
 */
struct DRC_COMP {
  FIXP_DBL maxBoostThr[2]; /*!< Max boost threshold. */
  FIXP_DBL boostThr[2];    /*!< Boost threshold. */
  FIXP_DBL earlyCutThr[2]; /*!< Early cut threshold. */
  FIXP_DBL cutThr[2];      /*!< Cut threshold. */
  FIXP_DBL maxCutThr[2];   /*!< Max cut threshold. */

  FIXP_DBL boostFac[2]; /*!< Precalculated factor for boost compression. */
  FIXP_DBL
  earlyCutFac[2];     /*!< Precalculated factor for early cut compression. */
  FIXP_DBL cutFac[2]; /*!< Precalculated factor for cut compression. */

  FIXP_DBL maxBoost[2];    /*!< Maximum boost. */
  FIXP_DBL maxCut[2];      /*!< Maximum cut. */
  FIXP_DBL maxEarlyCut[2]; /*!< Maximum early cut. */

  FIXP_DBL fastAttack[2]; /*!< Fast attack coefficient. */
  FIXP_DBL fastDecay[2];  /*!< Fast release coefficient. */
  FIXP_DBL slowAttack[2]; /*!< Slow attack coefficient. */
  FIXP_DBL slowDecay[2];  /*!< Slow release coefficient. */
  UINT holdOff[2];        /*!< Hold time in blocks. */

  FIXP_DBL attackThr[2]; /*!< Slow/fast attack threshold. */
  FIXP_DBL decayThr[2];  /*!< Slow/fast release threshold. */

  DRC_PROFILE profile[2];  /*!< DRC profile. */
  INT blockLength;         /*!< Block length in samples. */
  UINT sampleRate;         /*!< Sample rate. */
  CHANNEL_MODE chanConfig; /*!< Channel configuration. */

  UCHAR useWeighting; /*!< Use weighting filter. */

  UINT channels;     /*!< Number of channels. */
  UINT fullChannels; /*!< Number of full range channels. */
  INT channelIdx[9]; /*!< Offsets of interleaved channel samples (L, R, C, LFE,
                        Ls, Rs, S, Ls2, Rs2). */

  FIXP_DBL smoothLevel[2]; /*!< level smoothing states */
  FIXP_DBL smoothGain[2];  /*!< gain smoothing states */
  UINT holdCnt[2];         /*!< hold counter */

  FIXP_DBL limGain[2];  /*!< limiter gain */
  FIXP_DBL limDecay;    /*!< limiter decay (linear) */
  FIXP_DBL prevPeak[2]; /*!< max peak of previous block (stereo/mono)*/

  WEIGHTING_STATES
  filter[MAX_DRC_CHANNELS]; /*!< array holds weighting filter states */
};

/*---------------- constants -----------------------*/

/**
 *  Profile tables.
 */
static const FIXP_DBL tabMaxBoostThr[] = {
    (FIXP_DBL)(-(43 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(53 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(55 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(65 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(50 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(40 << METADATA_FRACT_BITS))};
static const FIXP_DBL tabBoostThr[] = {
    (FIXP_DBL)(-(31 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(41 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(31 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(41 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(31 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(31 << METADATA_FRACT_BITS))};
static const FIXP_DBL tabEarlyCutThr[] = {
    (FIXP_DBL)(-(26 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(21 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(26 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(21 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(26 << METADATA_FRACT_BITS)),
    (FIXP_DBL)(-(20 << METADATA_FRACT_BITS))};
static const FIXP_DBL tabCutThr[] = {(FIXP_DBL)(-(16 << METADATA_FRACT_BITS)),
                                     (FIXP_DBL)(-(11 << METADATA_FRACT_BITS)),
                                     (FIXP_DBL)(-(16 << METADATA_FRACT_BITS)),
                                     (FIXP_DBL)(-(21 << METADATA_FRACT_BITS)),
                                     (FIXP_DBL)(-(16 << METADATA_FRACT_BITS)),
                                     (FIXP_DBL)(-(10 << METADATA_FRACT_BITS))};
static const FIXP_DBL tabMaxCutThr[] = {
    (FIXP_DBL)(4 << METADATA_FRACT_BITS), (FIXP_DBL)(9 << METADATA_FRACT_BITS),
    (FIXP_DBL)(4 << METADATA_FRACT_BITS), (FIXP_DBL)(9 << METADATA_FRACT_BITS),
    (FIXP_DBL)(4 << METADATA_FRACT_BITS), (FIXP_DBL)(4 << METADATA_FRACT_BITS)};
static const FIXP_DBL tabBoostRatio[] = {
    FL2FXCONST_DBL(((1.f / 2.f) - 1.f)), FL2FXCONST_DBL(((1.f / 2.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 2.f) - 1.f)), FL2FXCONST_DBL(((1.f / 2.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 5.f) - 1.f)), FL2FXCONST_DBL(((1.f / 5.f) - 1.f))};
static const FIXP_DBL tabEarlyCutRatio[] = {
    FL2FXCONST_DBL(((1.f / 2.f) - 1.f)), FL2FXCONST_DBL(((1.f / 2.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 2.f) - 1.f)), FL2FXCONST_DBL(((1.f / 1.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 2.f) - 1.f)), FL2FXCONST_DBL(((1.f / 2.f) - 1.f))};
static const FIXP_DBL tabCutRatio[] = {
    FL2FXCONST_DBL(((1.f / 20.f) - 1.f)), FL2FXCONST_DBL(((1.f / 20.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 20.f) - 1.f)), FL2FXCONST_DBL(((1.f / 2.f) - 1.f)),
    FL2FXCONST_DBL(((1.f / 20.f) - 1.f)), FL2FXCONST_DBL(((1.f / 20.f) - 1.f))};
static const FIXP_DBL tabMaxBoost[] = {(FIXP_DBL)(6 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(6 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(12 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(12 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(15 << METADATA_FRACT_BITS)};
static const FIXP_DBL tabMaxCut[] = {(FIXP_DBL)(24 << METADATA_FRACT_BITS),
                                     (FIXP_DBL)(24 << METADATA_FRACT_BITS),
                                     (FIXP_DBL)(24 << METADATA_FRACT_BITS),
                                     (FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                     (FIXP_DBL)(24 << METADATA_FRACT_BITS),
                                     (FIXP_DBL)(24 << METADATA_FRACT_BITS)};
static const FIXP_DBL tabFastAttack[] = {
    FL2FXCONST_DBL((10.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((10.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((10.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((10.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((10.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((0.f / 1000.f) / METADATA_INT_SCALE)};
static const FIXP_DBL tabFastDecay[] = {
    FL2FXCONST_DBL((1000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((1000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((1000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((1000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((200.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((0.f / 1000.f) / METADATA_INT_SCALE)};
static const FIXP_DBL tabSlowAttack[] = {
    FL2FXCONST_DBL((100.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((100.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((100.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((100.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((100.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((0.f / 1000.f) / METADATA_INT_SCALE)};
static const FIXP_DBL tabSlowDecay[] = {
    FL2FXCONST_DBL((3000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((3000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((10000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((3000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((1000.f / 1000.f) / METADATA_INT_SCALE),
    FL2FXCONST_DBL((0.f / 1000.f) / METADATA_INT_SCALE)};

static const INT tabHoldOff[] = {10, 10, 10, 10, 10, 0};

static const FIXP_DBL tabAttackThr[] = {(FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                        (FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                        (FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                        (FIXP_DBL)(15 << METADATA_FRACT_BITS),
                                        (FIXP_DBL)(10 << METADATA_FRACT_BITS),
                                        (FIXP_DBL)(0 << METADATA_FRACT_BITS)};
static const FIXP_DBL tabDecayThr[] = {(FIXP_DBL)(20 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(20 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(20 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(20 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(10 << METADATA_FRACT_BITS),
                                       (FIXP_DBL)(0 << METADATA_FRACT_BITS)};

/**
 *  Weighting filter coefficients (biquad bandpass).
 */
static const FIXP_DBL b0 = FL2FXCONST_DBL(0.53050662f); /* b1 = 0, b2 = -b0 */
static const FIXP_DBL a1 = FL2FXCONST_DBL(-0.95237983f),
                      a2 = FL2FXCONST_DBL(-0.02248836f); /* a0 = 1 */

/*------------- function definitions ----------------*/

/**
 * \brief  Calculate scaling factor for denoted processing block.
 *
 * \param blockLength   Length of processing block.
 *
 * \return    shiftFactor
 */
static UINT getShiftFactor(const UINT length) {
  UINT ldN;
  for (ldN = 1; (((UINT)1) << ldN) < length; ldN++)
    ;

  return ldN;
}

/**
 * \brief  Sum up fixpoint values with best possible accuracy.
 *
 * \param value1        First input value.
 * \param q1            Scaling factor of first input value.
 * \param pValue2       Pointer to second input value, will be modified on
 * return.
 * \param pQ2           Pointer to second scaling factor, will be modified on
 * return.
 *
 * \return    void
 */
static void fixpAdd(const FIXP_DBL value1, const int q1,
                    FIXP_DBL* const pValue2, int* const pQ2) {
  const int headroom1 = fNormz(fixp_abs(value1)) - 1;
  const int headroom2 = fNormz(fixp_abs(*pValue2)) - 1;
  int resultScale = fixMax(q1 - headroom1, (*pQ2) - headroom2);

  if ((value1 != FL2FXCONST_DBL(0.f)) && (*pValue2 != FL2FXCONST_DBL(0.f))) {
    resultScale++;
  }

  *pValue2 = scaleValue(value1, q1 - resultScale) +
             scaleValue(*pValue2, (*pQ2) - resultScale);
  *pQ2 = (*pValue2 != (FIXP_DBL)0) ? resultScale : DFRACT_BITS - 1;
}

/**
 * \brief  Function for converting time constant to filter coefficient.
 *
 * \param t             Time constant.
 * \param sampleRate    Sampling rate in Hz.
 * \param blockLength   Length of processing block in samples per channel.
 *
 * \return    result = 1.0 - exp(-1.0/((t) * (f)))
 */
static FIXP_DBL tc2Coeff(const FIXP_DBL t, const INT sampleRate,
                         const INT blockLength) {
  FIXP_DBL sampleRateFract;
  FIXP_DBL blockLengthFract;
  FIXP_DBL f, product;
  FIXP_DBL exponent, result;
  INT e_res;

  /* f = sampleRate/blockLength */
  sampleRateFract =
      (FIXP_DBL)(sampleRate << (DFRACT_BITS - 1 - METADATA_LINT_BITS));
  blockLengthFract =
      (FIXP_DBL)(blockLength << (DFRACT_BITS - 1 - METADATA_LINT_BITS));
  f = fDivNorm(sampleRateFract, blockLengthFract, &e_res);
  f = scaleValue(f, e_res - METADATA_INT_BITS); /* convert to METADATA_FRACT */

  /* product = t*f */
  product = fMultNorm(t, f, &e_res);
  product = scaleValue(
      product, e_res + METADATA_INT_BITS); /* convert to METADATA_FRACT */

  /* exponent = (-1.0/((t) * (f))) */
  exponent = fDivNorm(METADATA_FRACT_SCALE, product, &e_res);
  exponent = scaleValue(
      exponent, e_res - METADATA_INT_BITS); /* convert to METADATA_FRACT */

  /* exponent * ld(e) */
  exponent = fMult(exponent, FIXP_ILOG2_DIV2) << 1; /* e^(x) = 2^(x*ld(e)) */

  /* exp(-1.0/((t) * (f))) */
  result = f2Pow(-exponent, DFRACT_BITS - 1 - METADATA_FRACT_BITS, &e_res);

  /* result = 1.0 - exp(-1.0/((t) * (f))) */
  result = (FIXP_DBL)MAXVAL_DBL - scaleValue(result, e_res);

  return result;
}

static void findPeakLevels(HDRC_COMP drcComp, const INT_PCM* const inSamples,
                           const FIXP_DBL clev, const FIXP_DBL slev,
                           const FIXP_DBL ext_leva, const FIXP_DBL ext_levb,
                           const FIXP_DBL lfe_lev, const FIXP_DBL dmxGain5,
                           const FIXP_DBL dmxGain2, FIXP_DBL peak[2]) {
  int i, c;
  FIXP_DBL tmp = FL2FXCONST_DBL(0.f);
  INT_PCM maxSample = 0;

  /* find peak level */
  peak[0] = peak[1] = FL2FXCONST_DBL(0.f);
  for (i = 0; i < drcComp->blockLength; i++) {
    const INT_PCM* pSamples = &inSamples[i * drcComp->channels];

    /* single channels */
    for (c = 0; c < (int)drcComp->channels; c++) {
      maxSample = fMax(maxSample, (INT_PCM)fAbs(pSamples[c]));
    }
  }
  peak[0] = fixMax(peak[0], FX_PCM2FX_DBL(maxSample) >> DOWNMIX_SHIFT);

  /* 7.1/6.1 to 5.1 downmixes */
  if (drcComp->fullChannels > 5) {
    for (i = 0; i < drcComp->blockLength; i++) {
      const INT_PCM* pSamples = &inSamples[i * drcComp->channels];

      /* channel 1 (L, Ls,...) */
      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_6_1:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
              (DOWNMIX_SHIFT - 1); /* Ls */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[6]]) >>
              (DOWNMIX_SHIFT - 1); /* Cs */
          break;
        case MODE_7_1_BACK:
        case MODE_7_1_REAR_SURROUND:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
              (DOWNMIX_SHIFT - 1); /* Ls */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lrs / Lss */
          break;
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lc */
          break;
        case MODE_7_1_TOP_FRONT:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
              (DOWNMIX_SHIFT - 1); /* L */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lvh */
          break;
        default:
          break;
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));

      /* channel 2 (R, Rs,...) */
      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_6_1:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
              (DOWNMIX_SHIFT - 1); /* Rs */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[6]]) >>
              (DOWNMIX_SHIFT - 1); /* Cs */
          break;
        case MODE_7_1_BACK:
        case MODE_7_1_REAR_SURROUND:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
              (DOWNMIX_SHIFT - 1); /* Rs */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rrs / Rss */
          break;
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rc */
          break;
        case MODE_7_1_TOP_FRONT:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
              (DOWNMIX_SHIFT - 1); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rvh */
          break;
        default:
          break;
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));

      /* channel 3 (C) */
      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                  DOWNMIX_SHIFT); /* C */
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lc */
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rc */
          break;
        default:
          break;
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));

    } /* for (blocklength) */

    /* take downmix gain into accout */
    peak[0] = fMult(dmxGain5, peak[0])
              << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
  }

  /* 7.1 / 5.1 to stereo downmixes */
  if (drcComp->fullChannels > 2) {
    /* Lt/Rt downmix */
    for (i = 0; i < drcComp->blockLength; i++) {
      const INT_PCM* pSamples = &inSamples[i * drcComp->channels];

      /* Lt */
      tmp = FL2FXCONST_DBL(0.f);
      if (drcComp->channelIdx[LS] >= 0)
        tmp -= fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[LS]]) >>
               (DOWNMIX_SHIFT - 1); /* Ls */
      if (drcComp->channelIdx[LS2] >= 0)
        tmp -= fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[LS2]]) >>
               (DOWNMIX_SHIFT - 1); /* Ls2 */
      if (drcComp->channelIdx[RS] >= 0)
        tmp -= fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[RS]]) >>
               (DOWNMIX_SHIFT - 1); /* Rs */
      if (drcComp->channelIdx[RS2] >= 0)
        tmp -= fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[RS2]]) >>
               (DOWNMIX_SHIFT - 1); /* Rs2 */
      if ((drcComp->channelIdx[LS] >= 0) && (drcComp->channelIdx[LS2] >= 0))
        tmp = fMult(FL2FXCONST_DBL(0.707f), tmp); /* 7.1ch */
      if (drcComp->channelIdx[S] >= 0)
        tmp -= fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[S]]) >>
               (DOWNMIX_SHIFT - 1); /* S */
      if (drcComp->channelIdx[C] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[C]]) >>
               (DOWNMIX_SHIFT - 1); /* C */
      tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[L]]) >>
              DOWNMIX_SHIFT); /* L */

      /* apply scaling of downmix gains */
      /* only for positive values only, as legacy decoders might not know this
       * parameter */
      if (dmxGain2 > FL2FXCONST_DBL(0.f)) {
        if (drcComp->fullChannels > 5) {
          tmp = fMult(dmxGain5, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
        }
        tmp = fMult(dmxGain2, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));

      /* Rt */
      tmp = FL2FXCONST_DBL(0.f);
      if (drcComp->channelIdx[LS] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[LS]]) >>
               (DOWNMIX_SHIFT - 1); /* Ls */
      if (drcComp->channelIdx[LS2] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[LS2]]) >>
               (DOWNMIX_SHIFT - 1); /* Ls2 */
      if (drcComp->channelIdx[RS] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[RS]]) >>
               (DOWNMIX_SHIFT - 1); /* Rs */
      if (drcComp->channelIdx[RS2] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[RS2]]) >>
               (DOWNMIX_SHIFT - 1); /* Rs2 */
      if ((drcComp->channelIdx[RS] >= 0) && (drcComp->channelIdx[RS2] >= 0))
        tmp = fMult(FL2FXCONST_DBL(0.707f), tmp); /* 7.1ch */
      if (drcComp->channelIdx[S] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[S]]) >>
               (DOWNMIX_SHIFT - 1); /* S */
      if (drcComp->channelIdx[C] >= 0)
        tmp += fMultDiv2(FL2FXCONST_DBL(0.707f),
                         (FIXP_PCM)pSamples[drcComp->channelIdx[C]]) >>
               (DOWNMIX_SHIFT - 1); /* C */
      tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[R]]) >>
              DOWNMIX_SHIFT); /* R */

      /* apply scaling of downmix gains */
      /* only for positive values only, as legacy decoders might not know this
       * parameter */
      if (dmxGain2 > FL2FXCONST_DBL(0.f)) {
        if (drcComp->fullChannels > 5) {
          tmp = fMult(dmxGain5, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
        }
        tmp = fMult(dmxGain2, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));
    }

    /* Lo/Ro downmix */
    for (i = 0; i < drcComp->blockLength; i++) {
      const INT_PCM* pSamples = &inSamples[i * drcComp->channels];

      /* Lo */
      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_6_1:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[6]]) >>
                 (DOWNMIX_SHIFT - 1); /* Cs */
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_BACK:
        case MODE_7_1_REAR_SURROUND:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
                 (DOWNMIX_SHIFT - 1); /* Lrs / Lss*/
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lc */
          tmp += fMultDiv2(fMult(ext_leva, clev),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
                 (DOWNMIX_SHIFT - 1); /* Lc - second path*/
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_TOP_FRONT:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
              (DOWNMIX_SHIFT - 1); /* L */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lvh */
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        default:
          if (drcComp->channelIdx[LS] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[LS]]) >>
                (DOWNMIX_SHIFT - 1); /* Ls */
          if (drcComp->channelIdx[LS2] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[LS2]]) >>
                (DOWNMIX_SHIFT - 1); /* Ls2 */
          if ((drcComp->channelIdx[LS] >= 0) && (drcComp->channelIdx[LS2] >= 0))
            tmp = fMult(FL2FXCONST_DBL(0.707f), tmp); /* 7.1ch */
          if (drcComp->channelIdx[S] >= 0)
            tmp +=
                fMultDiv2(slev,
                          fMult(FL2FXCONST_DBL(0.7f),
                                (FIXP_PCM)pSamples[drcComp->channelIdx[S]])) >>
                (DOWNMIX_SHIFT - 1); /* S */
          if (drcComp->channelIdx[C] >= 0)
            tmp +=
                fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[C]]) >>
                (DOWNMIX_SHIFT - 1); /* C */
          if (drcComp->channelIdx[3] >= 0)
            tmp += fMultDiv2(lfe_lev,
                             (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                   (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[L]]) >>
                  DOWNMIX_SHIFT); /* L */
          break;
      }

      /* apply scaling of downmix gains */
      /* only for positive values only, as legacy decoders might not know this
       * parameter */
      if (dmxGain2 > FL2FXCONST_DBL(0.f)) {
        if (drcComp->fullChannels > 5) {
          tmp = fMult(dmxGain5, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
        }
        tmp = fMult(dmxGain2, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));

      /* Ro */
      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_6_1:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[6]]) >>
                 (DOWNMIX_SHIFT - 1); /* Cs */
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_BACK:
        case MODE_7_1_REAR_SURROUND:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rrs / Rss*/
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rc */
          tmp += fMultDiv2(fMult(ext_leva, clev),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rc - second path*/
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_TOP_FRONT:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
              (DOWNMIX_SHIFT - 1); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rvh */
          tmp += fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp +=
              fMultDiv2(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
              (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        default:
          if (drcComp->channelIdx[RS] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[RS]]) >>
                (DOWNMIX_SHIFT - 1); /* Rs */
          if (drcComp->channelIdx[RS2] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[RS2]]) >>
                (DOWNMIX_SHIFT - 1); /* Rs2 */
          if ((drcComp->channelIdx[RS] >= 0) && (drcComp->channelIdx[RS2] >= 0))
            tmp = fMult(FL2FXCONST_DBL(0.707f), tmp); /* 7.1ch */
          if (drcComp->channelIdx[S] >= 0)
            tmp +=
                fMultDiv2(slev,
                          fMult(FL2FXCONST_DBL(0.7f),
                                (FIXP_PCM)pSamples[drcComp->channelIdx[S]])) >>
                (DOWNMIX_SHIFT - 1); /* S */
          if (drcComp->channelIdx[C] >= 0)
            tmp +=
                fMultDiv2(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[C]]) >>
                (DOWNMIX_SHIFT - 1); /* C */
          if (drcComp->channelIdx[3] >= 0)
            tmp += fMultDiv2(lfe_lev,
                             (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                   (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[R]]) >>
                  DOWNMIX_SHIFT); /* R */
      }

      /* apply scaling of downmix gains */
      /* only for positive values only, as legacy decoders might not know this
       * parameter */
      if (dmxGain2 > FL2FXCONST_DBL(0.f)) {
        if (drcComp->fullChannels > 5) {
          tmp = fMult(dmxGain5, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
        }
        tmp = fMult(dmxGain2, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
      }
      peak[0] = fixMax(peak[0], fixp_abs(tmp));
    }
  }

  peak[1] = fixMax(peak[0], peak[1]);

  /* Mono Downmix - for comp_val only */
  if (drcComp->fullChannels > 1) {
    for (i = 0; i < drcComp->blockLength; i++) {
      const INT_PCM* pSamples = &inSamples[i * drcComp->channels];

      tmp = FL2FXCONST_DBL(0.f);
      switch (drcComp->chanConfig) {
        case MODE_6_1:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMult(fMult(slev, ext_levb),
                       (FIXP_PCM)pSamples[drcComp->channelIdx[6]]) >>
                 (DOWNMIX_SHIFT - 1); /* Cs */
          tmp += fMult(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp += fMult(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                 (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_BACK:
        case MODE_7_1_REAR_SURROUND:
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(fMult(slev, ext_leva),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
                 (DOWNMIX_SHIFT - 1); /* Lrs / Lss*/
          tmp += fMultDiv2(fMult(slev, ext_levb),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rrs / Rss*/
          tmp += fMult(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp += fMult(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                 (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_1_2_2_2_1:
        case MODE_7_1_FRONT_CENTER:
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
                  DOWNMIX_SHIFT); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lc */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rc */
          tmp += fMultDiv2(fMult(ext_leva, clev),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
                 (DOWNMIX_SHIFT - 1); /* Lc - second path*/
          tmp += fMultDiv2(fMult(ext_leva, clev),
                           (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rc - second path*/
          tmp += fMult(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMult(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                 (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        case MODE_7_1_TOP_FRONT:
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[0]]) >>
              (DOWNMIX_SHIFT - 1); /* L */
          tmp +=
              fMultDiv2(ext_leva, (FIXP_PCM)pSamples[drcComp->channelIdx[1]]) >>
              (DOWNMIX_SHIFT - 1); /* R */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[7]]) >>
              (DOWNMIX_SHIFT - 1); /* Lvh */
          tmp +=
              fMultDiv2(ext_levb, (FIXP_PCM)pSamples[drcComp->channelIdx[8]]) >>
              (DOWNMIX_SHIFT - 1); /* Rvh */
          tmp += fMult(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[2]]) >>
                 (DOWNMIX_SHIFT - 1); /* C */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[4]]) >>
                 (DOWNMIX_SHIFT - 1); /* Ls */
          tmp += fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[5]]) >>
                 (DOWNMIX_SHIFT - 1); /* Rs */
          tmp += fMult(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                 (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          break;
        default:
          if (drcComp->channelIdx[LS] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[LS]]) >>
                (DOWNMIX_SHIFT - 1); /* Ls */
          if (drcComp->channelIdx[LS2] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[LS2]]) >>
                (DOWNMIX_SHIFT - 1); /* Ls2 */
          if (drcComp->channelIdx[RS] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[RS]]) >>
                (DOWNMIX_SHIFT - 1); /* Rs */
          if (drcComp->channelIdx[RS2] >= 0)
            tmp +=
                fMultDiv2(slev, (FIXP_PCM)pSamples[drcComp->channelIdx[RS2]]) >>
                (DOWNMIX_SHIFT - 1); /* Rs2 */
          if ((drcComp->channelIdx[LS] >= 0) && (drcComp->channelIdx[LS2] >= 0))
            tmp = fMult(FL2FXCONST_DBL(0.707f), tmp); /* 7.1ch */
          /*if ((drcComp->channelIdx[RS] >= 0) && (drcComp->channelIdx[RS2] >= 0)) tmp *=0.707f;*/ /* 7.1ch */
          if (drcComp->channelIdx[S] >= 0)
            tmp +=
                fMultDiv2(slev,
                          fMult(FL2FXCONST_DBL(0.7f),
                                (FIXP_PCM)pSamples[drcComp->channelIdx[S]])) >>
                (DOWNMIX_SHIFT - 1); /* S */
          if (drcComp->channelIdx[C] >= 0)
            tmp += fMult(clev, (FIXP_PCM)pSamples[drcComp->channelIdx[C]]) >>
                   (DOWNMIX_SHIFT - 1); /* C (2*clev) */
          if (drcComp->channelIdx[3] >= 0)
            tmp += fMult(lfe_lev, (FIXP_PCM)pSamples[drcComp->channelIdx[3]]) >>
                   (DOWNMIX_SHIFT - 1 - LFE_LEV_SCALE); /* LFE */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[L]]) >>
                  DOWNMIX_SHIFT); /* L */
          tmp += (FX_PCM2FX_DBL((FIXP_PCM)pSamples[drcComp->channelIdx[R]]) >>
                  DOWNMIX_SHIFT); /* R */
      }

      /* apply scaling of downmix gains */
      /* only for positive values only, as legacy decoders might not know this
       * parameter */
      if (dmxGain2 > FL2FXCONST_DBL(0.f)) {
        if (drcComp->fullChannels > 5) {
          tmp = fMult(dmxGain5, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
        }
        tmp = fMult(dmxGain2, tmp) << (DFRACT_BITS - 1 - METADATA_FRACT_BITS);
      }
      peak[1] = fixMax(peak[1], fixp_abs(tmp));
    }
  }
}

INT FDK_DRC_Generator_Open(HDRC_COMP* phDrcComp) {
  INT err = 0;
  HDRC_COMP hDcComp = NULL;

  if (phDrcComp == NULL) {
    err = -1;
    goto bail;
  }

  /* allocate memory */
  hDcComp = (HDRC_COMP)FDKcalloc(1, sizeof(DRC_COMP));

  if (hDcComp == NULL) {
    err = -1;
    goto bail;
  }

  FDKmemclear(hDcComp, sizeof(DRC_COMP));

  /* Return drc compressor instance */
  *phDrcComp = hDcComp;
  return err;
bail:
  FDK_DRC_Generator_Close(&hDcComp);
  return err;
}

INT FDK_DRC_Generator_Close(HDRC_COMP* phDrcComp) {
  if (phDrcComp == NULL) {
    return -1;
  }
  if (*phDrcComp != NULL) {
    FDKfree(*phDrcComp);
    *phDrcComp = NULL;
  }
  return 0;
}

INT FDK_DRC_Generator_Initialize(HDRC_COMP drcComp,
                                 const DRC_PROFILE profileLine,
                                 const DRC_PROFILE profileRF,
                                 const INT blockLength, const UINT sampleRate,
                                 const CHANNEL_MODE channelMode,
                                 const CHANNEL_ORDER channelOrder,
                                 const UCHAR useWeighting) {
  int i;
  CHANNEL_MAPPING channelMapping;

  drcComp->limDecay =
      FL2FXCONST_DBL(((0.006f / 256) * blockLength) / METADATA_INT_SCALE);

  /* Save parameters. */
  drcComp->blockLength = blockLength;
  drcComp->sampleRate = sampleRate;
  drcComp->chanConfig = channelMode;
  drcComp->useWeighting = useWeighting;

  if (FDK_DRC_Generator_setDrcProfile(drcComp, profileLine, profileRF) !=
      0) { /* expects initialized blockLength and sampleRate */
    return (-1);
  }

  /* Set number of channels and channel offsets. */
  if (FDKaacEnc_InitChannelMapping(channelMode, channelOrder,
                                   &channelMapping) != AAC_ENC_OK) {
    return (-2);
  }

  for (i = 0; i < 9; i++) drcComp->channelIdx[i] = -1;

  switch (channelMode) {
    case MODE_1: /* mono */
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0];
      break;
    case MODE_2: /* stereo */
      drcComp->channelIdx[L] = channelMapping.elInfo[0].ChannelIndex[0];
      drcComp->channelIdx[R] = channelMapping.elInfo[0].ChannelIndex[1];
      break;
    case MODE_1_2: /* 3ch */
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0];
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1];
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0];
      break;
    case MODE_1_2_1: /* 4ch */
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0];
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1];
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0];
      drcComp->channelIdx[S] = channelMapping.elInfo[2].ChannelIndex[0];
      break;
    case MODE_1_2_2: /* 5ch */
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0];
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1];
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0];
      drcComp->channelIdx[LS] = channelMapping.elInfo[2].ChannelIndex[0];
      drcComp->channelIdx[RS] = channelMapping.elInfo[2].ChannelIndex[1];
      break;
    case MODE_1_2_2_1: /* 5.1 ch */
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0];
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1];
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0];
      drcComp->channelIdx[LFE] = channelMapping.elInfo[3].ChannelIndex[0];
      drcComp->channelIdx[LS] = channelMapping.elInfo[2].ChannelIndex[0];
      drcComp->channelIdx[RS] = channelMapping.elInfo[2].ChannelIndex[1];
      break;
    case MODE_1_2_2_2_1: /* 7.1 ch */
    case MODE_7_1_FRONT_CENTER:
      drcComp->channelIdx[L] = channelMapping.elInfo[2].ChannelIndex[0]; /* l */
      drcComp->channelIdx[R] = channelMapping.elInfo[2].ChannelIndex[1]; /* r */
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0]; /* c */
      drcComp->channelIdx[LFE] =
          channelMapping.elInfo[4].ChannelIndex[0]; /* lfe */
      drcComp->channelIdx[LS] =
          channelMapping.elInfo[3].ChannelIndex[0]; /* ls */
      drcComp->channelIdx[RS] =
          channelMapping.elInfo[3].ChannelIndex[1]; /* rs */
      drcComp->channelIdx[LS2] =
          channelMapping.elInfo[1].ChannelIndex[0]; /* lc */
      drcComp->channelIdx[RS2] =
          channelMapping.elInfo[1].ChannelIndex[1]; /* rc */
      break;
    case MODE_7_1_BACK:
    case MODE_7_1_REAR_SURROUND:
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0]; /* l */
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1]; /* r */
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0]; /* c */
      drcComp->channelIdx[LFE] =
          channelMapping.elInfo[4].ChannelIndex[0]; /* lfe */
      drcComp->channelIdx[LS] =
          channelMapping.elInfo[3].ChannelIndex[0]; /* lrear */
      drcComp->channelIdx[RS] =
          channelMapping.elInfo[3].ChannelIndex[1]; /* rrear */
      drcComp->channelIdx[LS2] =
          channelMapping.elInfo[2].ChannelIndex[0]; /* ls */
      drcComp->channelIdx[RS2] =
          channelMapping.elInfo[2].ChannelIndex[1]; /* rs */
      break;
    case MODE_6_1:
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0]; /* l */
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1]; /* r */
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0]; /* c */
      drcComp->channelIdx[LFE] =
          channelMapping.elInfo[4].ChannelIndex[0]; /* lfe */
      drcComp->channelIdx[LS] =
          channelMapping.elInfo[2].ChannelIndex[0]; /* ls */
      drcComp->channelIdx[RS] =
          channelMapping.elInfo[2].ChannelIndex[1]; /* rs */
      drcComp->channelIdx[S] = channelMapping.elInfo[3].ChannelIndex[0]; /* s */
      break;
    case MODE_7_1_TOP_FRONT:
      drcComp->channelIdx[L] = channelMapping.elInfo[1].ChannelIndex[0]; /* l */
      drcComp->channelIdx[R] = channelMapping.elInfo[1].ChannelIndex[1]; /* r */
      drcComp->channelIdx[C] = channelMapping.elInfo[0].ChannelIndex[0]; /* c */
      drcComp->channelIdx[LFE] =
          channelMapping.elInfo[3].ChannelIndex[0]; /* lfe */
      drcComp->channelIdx[LS] =
          channelMapping.elInfo[2].ChannelIndex[0]; /* ls */
      drcComp->channelIdx[RS] =
          channelMapping.elInfo[2].ChannelIndex[1]; /* rs */
      drcComp->channelIdx[LS2] =
          channelMapping.elInfo[4].ChannelIndex[0]; /* lvh2 */
      drcComp->channelIdx[RS2] =
          channelMapping.elInfo[4].ChannelIndex[1]; /* rvh2 */
      break;
    default:
      return (-1);
  }

  drcComp->fullChannels = channelMapping.nChannelsEff;
  drcComp->channels = channelMapping.nChannels;

  /* Init states. */
  drcComp->smoothLevel[0] = drcComp->smoothLevel[1] =
      (FIXP_DBL)(-(135 << METADATA_FRACT_BITS));

  FDKmemclear(drcComp->smoothGain, sizeof(drcComp->smoothGain));
  FDKmemclear(drcComp->holdCnt, sizeof(drcComp->holdCnt));
  FDKmemclear(drcComp->limGain, sizeof(drcComp->limGain));
  FDKmemclear(drcComp->prevPeak, sizeof(drcComp->prevPeak));
  FDKmemclear(drcComp->filter, sizeof(drcComp->filter));

  return (0);
}

INT FDK_DRC_Generator_setDrcProfile(HDRC_COMP drcComp,
                                    const DRC_PROFILE profileLine,
                                    const DRC_PROFILE profileRF) {
  int profileIdx, i;

  drcComp->profile[0] = profileLine;
  drcComp->profile[1] = profileRF;

  for (i = 0; i < 2; i++) {
    /* get profile index */
    switch (drcComp->profile[i]) {
      case DRC_NONE:
      case DRC_NOT_PRESENT:
      case DRC_FILMSTANDARD:
        profileIdx = 0;
        break;
      case DRC_FILMLIGHT:
        profileIdx = 1;
        break;
      case DRC_MUSICSTANDARD:
        profileIdx = 2;
        break;
      case DRC_MUSICLIGHT:
        profileIdx = 3;
        break;
      case DRC_SPEECH:
        profileIdx = 4;
        break;
      case DRC_DELAY_TEST:
        profileIdx = 5;
        break;
      default:
        return (-1);
    }

    /* get parameters for selected profile */
    if (profileIdx >= 0) {
      drcComp->maxBoostThr[i] = tabMaxBoostThr[profileIdx];
      drcComp->boostThr[i] = tabBoostThr[profileIdx];
      drcComp->earlyCutThr[i] = tabEarlyCutThr[profileIdx];
      drcComp->cutThr[i] = tabCutThr[profileIdx];
      drcComp->maxCutThr[i] = tabMaxCutThr[profileIdx];

      drcComp->boostFac[i] = tabBoostRatio[profileIdx];
      drcComp->earlyCutFac[i] = tabEarlyCutRatio[profileIdx];
      drcComp->cutFac[i] = tabCutRatio[profileIdx];

      drcComp->maxBoost[i] = tabMaxBoost[profileIdx];
      drcComp->maxCut[i] = tabMaxCut[profileIdx];
      drcComp->maxEarlyCut[i] =
          -fMult((drcComp->cutThr[i] - drcComp->earlyCutThr[i]),
                 drcComp->earlyCutFac[i]); /* no scaling after mult needed,
                                              earlyCutFac is in FIXP_DBL */

      drcComp->fastAttack[i] = tc2Coeff(
          tabFastAttack[profileIdx], drcComp->sampleRate, drcComp->blockLength);
      drcComp->fastDecay[i] = tc2Coeff(
          tabFastDecay[profileIdx], drcComp->sampleRate, drcComp->blockLength);
      drcComp->slowAttack[i] = tc2Coeff(
          tabSlowAttack[profileIdx], drcComp->sampleRate, drcComp->blockLength);
      drcComp->slowDecay[i] = tc2Coeff(
          tabSlowDecay[profileIdx], drcComp->sampleRate, drcComp->blockLength);
      drcComp->holdOff[i] = tabHoldOff[profileIdx] * 256 / drcComp->blockLength;

      drcComp->attackThr[i] = tabAttackThr[profileIdx];
      drcComp->decayThr[i] = tabDecayThr[profileIdx];
    }

    drcComp->smoothGain[i] = FL2FXCONST_DBL(0.f);
  }
  return (0);
}

INT FDK_DRC_Generator_Calc(HDRC_COMP drcComp, const INT_PCM* const inSamples,
                           const UINT inSamplesBufSize, const INT dialnorm,
                           const INT drc_TargetRefLevel,
                           const INT comp_TargetRefLevel, const FIXP_DBL clev,
                           const FIXP_DBL slev, const FIXP_DBL ext_leva,
                           const FIXP_DBL ext_levb, const FIXP_DBL lfe_lev,
                           const INT dmxGain5, const INT dmxGain2,
                           INT* const pDynrng, INT* const pCompr) {
  int i, c;
  FIXP_DBL peak[2];

  /**************************************************************************
   * compressor
   **************************************************************************/
  if ((drcComp->profile[0] != DRC_NONE) || (drcComp->profile[1] != DRC_NONE)) {
    /* Calc loudness level */
    FIXP_DBL level_b = FL2FXCONST_DBL(0.f);
    int level_e = DFRACT_BITS - 1;

    /* Increase energy time resolution with shorter processing blocks. 16 is an
     * empiric value. */
    const int granuleLength = fixMin(16, drcComp->blockLength);

    if (drcComp->useWeighting) {
      FIXP_DBL x1, x2, y, y1, y2;
      /* sum of filter coefficients about 2.5 -> squared value is 6.25
         WEIGHTING_FILTER_SHIFT is 2 -> scaling about 16, therefore reduce
         granuleShift by 1.
       */
      const int granuleShift = getShiftFactor(granuleLength) - 1;

      for (c = 0; c < (int)drcComp->channels; c++) {
        const INT_PCM* pSamples = inSamples + c * inSamplesBufSize;

        if (c == drcComp->channelIdx[LFE]) {
          continue; /* skip LFE */
        }

        /* get filter states */
        x1 = drcComp->filter[c].x1;
        x2 = drcComp->filter[c].x2;
        y1 = drcComp->filter[c].y1;
        y2 = drcComp->filter[c].y2;

        i = 0;

        do {
          int offset = i;
          FIXP_DBL accu = FL2FXCONST_DBL(0.f);

          for (i = offset;
               i < fixMin(offset + granuleLength, drcComp->blockLength); i++) {
            /* apply weighting filter */
            FIXP_DBL x =
                FX_PCM2FX_DBL((FIXP_PCM)pSamples[i]) >> WEIGHTING_FILTER_SHIFT;

            /* y = b0 * (x - x2) - a1 * y1 - a2 * y2; */
            y = fMult(b0, x - x2) - fMult(a1, y1) - fMult(a2, y2);

            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;

            accu += fPow2Div2(y) >> (granuleShift - 1); /* partial energy */
          }                                             /* i */

          fixpAdd(accu, granuleShift + 2 * WEIGHTING_FILTER_SHIFT, &level_b,
                  &level_e); /* sup up partial energies */

        } while (i < drcComp->blockLength);

        /* save filter states */
        drcComp->filter[c].x1 = x1;
        drcComp->filter[c].x2 = x2;
        drcComp->filter[c].y1 = y1;
        drcComp->filter[c].y2 = y2;
      } /* c */
    }   /* weighting */
    else {
      const int granuleShift = getShiftFactor(granuleLength);

      for (c = 0; c < (int)drcComp->channels; c++) {
        const INT_PCM* pSamples = inSamples + c * inSamplesBufSize;

        if ((int)c == drcComp->channelIdx[LFE]) {
          continue; /* skip LFE */
        }

        i = 0;

        do {
          int offset = i;
          FIXP_DBL accu = FL2FXCONST_DBL(0.f);

          for (i = offset;
               i < fixMin(offset + granuleLength, drcComp->blockLength); i++) {
            /* partial energy */
            accu += fPow2Div2((FIXP_PCM)pSamples[i]) >> (granuleShift - 1);
          } /* i */

          fixpAdd(accu, granuleShift, &level_b,
                  &level_e); /* sup up partial energies */

        } while (i < drcComp->blockLength);
      }
    } /* weighting */

    /*
     * Convert to dBFS, apply dialnorm
     */
    /* level scaling */

    /* descaled level in ld64 representation */
    FIXP_DBL ldLevel =
        CalcLdData(level_b) +
        (FIXP_DBL)((level_e - 12) << (DFRACT_BITS - 1 - LD_DATA_SHIFT)) -
        CalcLdData((FIXP_DBL)(drcComp->blockLength << (DFRACT_BITS - 1 - 12)));

    /* if (level < 1e-10) level = 1e-10f; */
    ldLevel =
        fMax(ldLevel, FL2FXCONST_DBL(-0.51905126482615036685473741085772f));

    /* level = 10 * log(level)/log(10) + 3;
     *       = 10*log(2)/log(10) * ld(level) + 3;
     *       = 10 * 0.30102999566398119521373889472449 * ld(level) + 3
     *       = 10 * (0.30102999566398119521373889472449 * ld(level) + 0.3)
     *       = 10 * (0.30102999566398119521373889472449 * ld64(level) + 0.3/64)
     * * 64
     *
     *    additional scaling with METADATA_FRACT_BITS:
     *       = 10 * (0.30102999566398119521373889472449 * ld64(level) + 0.3/64)
     * * 64 * 2^(METADATA_FRACT_BITS) = 10 * (0.30102999566398119521373889472449
     * * ld64(level) + 0.3/64) * 2^(METADATA_FRACT_BITS+LD_DATA_SHIFT) =
     * 10*2^(METADATA_FRACT_BITS+LD_DATA_SHIFT) * (
     * 0.30102999566398119521373889472449 * ld64(level) + 0.3/64 )
     * */
    FIXP_DBL level = fMult(
        (FIXP_DBL)(10 << (METADATA_FRACT_BITS + LD_DATA_SHIFT)),
        fMult(FL2FXCONST_DBL(0.30102999566398119521373889472449f), ldLevel) +
            (FIXP_DBL)(FL2FXCONST_DBL(0.3f) >> LD_DATA_SHIFT));

    /* level -= dialnorm + 31 */ /* this is fixed to Dolby-ReferenceLevel as
                                    compressor profiles are defined relative to
                                    this */
    level -= ((FIXP_DBL)(dialnorm << (METADATA_FRACT_BITS - 16)) +
              (FIXP_DBL)(31 << METADATA_FRACT_BITS));

    for (i = 0; i < 2; i++) {
      if (drcComp->profile[i] == DRC_NONE) {
        /* no compression */
        drcComp->smoothGain[i] = FL2FXCONST_DBL(0.f);
      } else {
        FIXP_DBL gain, alpha, lvl2smthlvl;

        /* calc static gain */
        if (level <= drcComp->maxBoostThr[i]) {
          /* max boost */
          gain = drcComp->maxBoost[i];
        } else if (level < drcComp->boostThr[i]) {
          /* boost range */
          gain = fMult((level - drcComp->boostThr[i]), drcComp->boostFac[i]);
        } else if (level <= drcComp->earlyCutThr[i]) {
          /* null band */
          gain = FL2FXCONST_DBL(0.f);
        } else if (level <= drcComp->cutThr[i]) {
          /* early cut range */
          gain =
              fMult((level - drcComp->earlyCutThr[i]), drcComp->earlyCutFac[i]);
        } else if (level < drcComp->maxCutThr[i]) {
          /* cut range */
          gain = fMult((level - drcComp->cutThr[i]), drcComp->cutFac[i]) -
                 drcComp->maxEarlyCut[i];
        } else {
          /* max cut */
          gain = -drcComp->maxCut[i];
        }

        /* choose time constant */
        lvl2smthlvl = level - drcComp->smoothLevel[i];
        if (gain < drcComp->smoothGain[i]) {
          /* attack */
          if (lvl2smthlvl > drcComp->attackThr[i]) {
            /* fast attack */
            alpha = drcComp->fastAttack[i];
          } else {
            /* slow attack */
            alpha = drcComp->slowAttack[i];
          }
        } else {
          /* release */
          if (lvl2smthlvl < -drcComp->decayThr[i]) {
            /* fast release */
            alpha = drcComp->fastDecay[i];
          } else {
            /* slow release */
            alpha = drcComp->slowDecay[i];
          }
        }

        /* smooth gain & level */
        if ((gain < drcComp->smoothGain[i]) ||
            (drcComp->holdCnt[i] ==
             0)) { /* hold gain unless we have an attack or hold
                      period is over */
          FIXP_DBL accu;

          /* drcComp->smoothLevel[i] = (1-alpha) * drcComp->smoothLevel[i] +
           * alpha * level; */
          accu = fMult(((FIXP_DBL)MAXVAL_DBL - alpha), drcComp->smoothLevel[i]);
          accu += fMult(alpha, level);
          drcComp->smoothLevel[i] = accu;

          /* drcComp->smoothGain[i]  = (1-alpha) * drcComp->smoothGain[i] +
           * alpha * gain; */
          accu = fMult(((FIXP_DBL)MAXVAL_DBL - alpha), drcComp->smoothGain[i]);
          accu += fMult(alpha, gain);
          drcComp->smoothGain[i] = accu;
        }

        /* hold counter */
        if (drcComp->holdCnt[i]) {
          drcComp->holdCnt[i]--;
        }
        if (gain < drcComp->smoothGain[i]) {
          drcComp->holdCnt[i] = drcComp->holdOff[i];
        }
      } /* profile != DRC_NONE */
    }   /* for i=1..2 */
  } else {
    /* no compression */
    drcComp->smoothGain[0] = FL2FXCONST_DBL(0.f);
    drcComp->smoothGain[1] = FL2FXCONST_DBL(0.f);
  }

  /**************************************************************************
   * limiter
   **************************************************************************/

  findPeakLevels(drcComp, inSamples, clev, slev, ext_leva, ext_levb, lfe_lev,
                 (FIXP_DBL)((LONG)(dmxGain5) << (METADATA_FRACT_BITS - 16)),
                 (FIXP_DBL)((LONG)(dmxGain2) << (METADATA_FRACT_BITS - 16)),
                 peak);

  for (i = 0; i < 2; i++) {
    FIXP_DBL tmp = drcComp->prevPeak[i];
    drcComp->prevPeak[i] = peak[i];
    peak[i] = fixMax(peak[i], tmp);

    /*
     * Convert to dBFS, apply dialnorm
     */
    /* descaled peak in ld64 representation */
    FIXP_DBL ld_peak =
        CalcLdData(peak[i]) +
        (FIXP_DBL)((LONG)DOWNMIX_SHIFT << (DFRACT_BITS - 1 - LD_DATA_SHIFT));

    /* if (peak < 1e-6) level = 1e-6f; */
    ld_peak =
        fMax(ld_peak, FL2FXCONST_DBL(-0.31143075889569022011284244651463f));

    /* peak[i] = 20 * log(peak[i])/log(10) + 0.2f +
     * (drcComp->smoothGain[i]*2^METADATA_FRACT_BITS) peak[i] = 20 *
     * log(2)/log(10) * ld(peak[i]) + 0.2f +
     * (drcComp->smoothGain[i]*2^METADATA_FRACT_BITS) peak[i] = 10 *
     * 2*0.30102999566398119521373889472449 * ld(peak[i]) + 0.2f +
     * (drcComp->smoothGain[i]*2^METADATA_FRACT_BITS)
     *
     *    additional scaling with METADATA_FRACT_BITS:
     * peak[i] = (10 * 2*0.30102999566398119521373889472449 * ld64(peak[i]) * 64
     * + 0.2f +
     * (drcComp->smoothGain[i]*2^METADATA_FRACT_BITS))*2^(-METADATA_FRACT_BITS)
     * peak[i] = 10*2^(METADATA_FRACT_BITS+LD_DATA_SHIFT) *
     * 2*0.30102999566398119521373889472449 * ld64(peak[i])
     *         + 0.2f*2^(-METADATA_FRACT_BITS) + drcComp->smoothGain[i]
     */
    peak[i] = fMult(
        (FIXP_DBL)(10 << (METADATA_FRACT_BITS + LD_DATA_SHIFT)),
        fMult(FL2FX_DBL(2 * 0.30102999566398119521373889472449f), ld_peak));
    peak[i] +=
        (FL2FX_DBL(0.5f) >> METADATA_INT_BITS); /* add a little bit headroom */
    peak[i] += drcComp->smoothGain[i];
  }

  /* peak -= dialnorm + 31; */ /* this is Dolby style only */
  peak[0] -= (FIXP_DBL)((dialnorm - drc_TargetRefLevel)
                        << (METADATA_FRACT_BITS -
                            16)); /* peak[0] -= dialnorm - drc_TargetRefLevel */

  /* peak += 11; */
  /* this is Dolby style only */ /* RF mode output is 11dB higher */
  /*peak += comp_TargetRefLevel - drc_TargetRefLevel;*/
  peak[1] -=
      (FIXP_DBL)((dialnorm - comp_TargetRefLevel)
                 << (METADATA_FRACT_BITS -
                     16)); /* peak[1] -= dialnorm - comp_TargetRefLevel */

  /* limiter gain */
  drcComp->limGain[0] += drcComp->limDecay; /* linear limiter release */
  drcComp->limGain[0] = fixMin(drcComp->limGain[0], -peak[0]);

  drcComp->limGain[1] += 2 * drcComp->limDecay; /* linear limiter release */
  drcComp->limGain[1] = fixMin(drcComp->limGain[1], -peak[1]);

  /*************************************************************************/

  /* apply limiting, return DRC gains*/
  {
    FIXP_DBL tmp;

    tmp = drcComp->smoothGain[0];
    if (drcComp->limGain[0] < FL2FXCONST_DBL(0.f)) {
      tmp += drcComp->limGain[0];
    }
    *pDynrng = (LONG)scaleValue(tmp, -(METADATA_FRACT_BITS - 16));

    tmp = drcComp->smoothGain[1];
    if (drcComp->limGain[1] < FL2FXCONST_DBL(0.f)) {
      tmp += drcComp->limGain[1];
    }
    *pCompr = (LONG)scaleValue(tmp, -(METADATA_FRACT_BITS - 16));
  }

  return 0;
}

DRC_PROFILE FDK_DRC_Generator_getDrcProfile(const HDRC_COMP drcComp) {
  return drcComp->profile[0];
}

DRC_PROFILE FDK_DRC_Generator_getCompProfile(const HDRC_COMP drcComp) {
  return drcComp->profile[1];
}
