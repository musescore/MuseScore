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

   Description: Psychoaccoustic configuration

*******************************************************************************/

#include "psy_configuration.h"
#include "adj_thr.h"
#include "aacEnc_rom.h"

#include "genericStds.h"

#include "FDK_trigFcts.h"

typedef struct {
  LONG sampleRate;
  const SFB_PARAM_LONG *paramLong;
  const SFB_PARAM_SHORT *paramShort;
} SFB_INFO_TAB;

static const SFB_INFO_TAB sfbInfoTab[] = {
    {8000, &p_FDKaacEnc_8000_long_1024, &p_FDKaacEnc_8000_short_128},
    {11025, &p_FDKaacEnc_11025_long_1024, &p_FDKaacEnc_11025_short_128},
    {12000, &p_FDKaacEnc_12000_long_1024, &p_FDKaacEnc_12000_short_128},
    {16000, &p_FDKaacEnc_16000_long_1024, &p_FDKaacEnc_16000_short_128},
    {22050, &p_FDKaacEnc_22050_long_1024, &p_FDKaacEnc_22050_short_128},
    {24000, &p_FDKaacEnc_24000_long_1024, &p_FDKaacEnc_24000_short_128},
    {32000, &p_FDKaacEnc_32000_long_1024, &p_FDKaacEnc_32000_short_128},
    {44100, &p_FDKaacEnc_44100_long_1024, &p_FDKaacEnc_44100_short_128},
    {48000, &p_FDKaacEnc_48000_long_1024, &p_FDKaacEnc_48000_short_128},
    {64000, &p_FDKaacEnc_64000_long_1024, &p_FDKaacEnc_64000_short_128},
    {88200, &p_FDKaacEnc_88200_long_1024, &p_FDKaacEnc_88200_short_128},
    {96000, &p_FDKaacEnc_96000_long_1024, &p_FDKaacEnc_96000_short_128}

};

/* 22050 and 24000 Hz */
static const SFB_PARAM_LONG p_22050_long_512 = {
    31, {4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  12, 12,
         12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32}};

/* 32000 Hz */
static const SFB_PARAM_LONG p_32000_long_512 = {
    37,
    {4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8, 8,
     12, 12, 12, 12, 16, 16, 16, 20, 24, 24, 28, 32, 32, 32, 32, 32, 32, 32}};

/* 44100 Hz */
static const SFB_PARAM_LONG p_44100_long_512 = {
    36, {4, 4, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,
         8, 8, 12, 12, 12, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32, 32, 52}};

static const SFB_INFO_TAB sfbInfoTabLD512[] = {
    {8000, &p_22050_long_512, NULL},   {11025, &p_22050_long_512, NULL},
    {12000, &p_22050_long_512, NULL},  {16000, &p_22050_long_512, NULL},
    {22050, &p_22050_long_512, NULL},  {24000, &p_22050_long_512, NULL},
    {32000, &p_32000_long_512, NULL},  {44100, &p_44100_long_512, NULL},
    {48000, &p_44100_long_512, NULL},  {64000, &p_44100_long_512, NULL},
    {88200, &p_44100_long_512, NULL},  {96000, &p_44100_long_512, NULL},
    {128000, &p_44100_long_512, NULL}, {176400, &p_44100_long_512, NULL},
    {192000, &p_44100_long_512, NULL}, {256000, &p_44100_long_512, NULL},
    {352800, &p_44100_long_512, NULL}, {384000, &p_44100_long_512, NULL},
};

/* 22050 and 24000 Hz */
static const SFB_PARAM_LONG p_22050_long_480 = {
    30, {4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  12,
         12, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32, 32, 32, 32}};

/* 32000 Hz */
static const SFB_PARAM_LONG p_32000_long_480 = {
    37, {4, 4, 4, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8, 8,
         8, 8, 8, 12, 12, 12, 16, 16, 20, 24, 32, 32, 32, 32, 32, 32, 32, 32}};

/* 44100 Hz */
static const SFB_PARAM_LONG p_44100_long_480 = {
    35, {4, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8, 8,
         8, 12, 12, 12, 12, 12, 16, 16, 24, 28, 32, 32, 32, 32, 32, 32, 48}};

static const SFB_INFO_TAB sfbInfoTabLD480[] = {
    {8000, &p_22050_long_480, NULL},   {11025, &p_22050_long_480, NULL},
    {12000, &p_22050_long_480, NULL},  {16000, &p_22050_long_480, NULL},
    {22050, &p_22050_long_480, NULL},  {24000, &p_22050_long_480, NULL},
    {32000, &p_32000_long_480, NULL},  {44100, &p_44100_long_480, NULL},
    {48000, &p_44100_long_480, NULL},  {64000, &p_44100_long_480, NULL},
    {88200, &p_44100_long_480, NULL},  {96000, &p_44100_long_480, NULL},
    {128000, &p_44100_long_480, NULL}, {176400, &p_44100_long_480, NULL},
    {192000, &p_44100_long_480, NULL}, {256000, &p_44100_long_480, NULL},
    {352800, &p_44100_long_480, NULL}, {384000, &p_44100_long_480, NULL},
};

/* Fixed point precision definitions */
#define Q_BARCVAL (25)

AAC_ENCODER_ERROR FDKaacEnc_initSfbTable(const LONG sampleRate,
                                         const INT blockType,
                                         const INT granuleLength,
                                         INT *const sfbOffset,
                                         INT *const sfbCnt) {
  INT i, specStartOffset = 0;
  INT granuleLengthWindow = granuleLength;
  const UCHAR *sfbWidth = NULL;
  const SFB_INFO_TAB *sfbInfo = NULL;
  int size;

  /*
    select table
  */
  switch (granuleLength) {
    case 1024:
    case 960:
      sfbInfo = sfbInfoTab;
      size = (INT)(sizeof(sfbInfoTab) / sizeof(SFB_INFO_TAB));
      break;
    case 512:
      sfbInfo = sfbInfoTabLD512;
      size = sizeof(sfbInfoTabLD512);
      break;
    case 480:
      sfbInfo = sfbInfoTabLD480;
      size = sizeof(sfbInfoTabLD480);
      break;
    default:
      return AAC_ENC_INVALID_FRAME_LENGTH;
  }

  for (i = 0; i < size; i++) {
    if (sfbInfo[i].sampleRate == sampleRate) {
      switch (blockType) {
        case LONG_WINDOW:
        case START_WINDOW:
        case STOP_WINDOW:
          sfbWidth = sfbInfo[i].paramLong->sfbWidth;
          *sfbCnt = sfbInfo[i].paramLong->sfbCnt;
          break;
        case SHORT_WINDOW:
          sfbWidth = sfbInfo[i].paramShort->sfbWidth;
          *sfbCnt = sfbInfo[i].paramShort->sfbCnt;
          granuleLengthWindow /= TRANS_FAC;
          break;
      }
      break;
    }
  }
  if (i == size) {
    return AAC_ENC_UNSUPPORTED_SAMPLINGRATE;
  }

  /*
    calc sfb offsets
  */
  for (i = 0; i < *sfbCnt; i++) {
    sfbOffset[i] = specStartOffset;
    specStartOffset += sfbWidth[i];
    if (specStartOffset >= granuleLengthWindow) {
      i++;
      break;
    }
  }
  *sfbCnt = fixMin(i, *sfbCnt);
  sfbOffset[*sfbCnt] = fixMin(specStartOffset, granuleLengthWindow);
  return AAC_ENC_OK;
}

/*****************************************************************************

    functionname: FDKaacEnc_BarcLineValue
    description:  Calculates barc value for one frequency line
    returns:      barc value of line
    input:        number of lines in transform, index of line to check, Fs
    output:

*****************************************************************************/
static FIXP_DBL FDKaacEnc_BarcLineValue(INT noOfLines, INT fftLine,
                                        LONG samplingFreq) {
  FIXP_DBL FOURBY3EM4 = (FIXP_DBL)0x45e7b273; /* 4.0/3 * 0.0001 in q43 */
  FIXP_DBL PZZZ76 = (FIXP_DBL)0x639d5e4a;     /* 0.00076 in q41 */
  FIXP_DBL ONE3P3 = (FIXP_DBL)0x35333333;     /* 13.3 in q26 */
  FIXP_DBL THREEP5 = (FIXP_DBL)0x1c000000;    /* 3.5 in q27 */
  FIXP_DBL INV480 = (FIXP_DBL)0x44444444;     // 1/480 in q39

  FIXP_DBL center_freq, x1, x2;
  FIXP_DBL bvalFFTLine, atan1, atan2;

  /* Theoritical maximum of center_freq (samp_freq*0.5) is 96khz * 0.5 = 48000
   */
  /* Theoritical maximum of x1 is 1.3333333e-4f * center_freq = 6.4, can keep in
   * q28  */
  /* Theoritical maximum of x2 is 0.00076f * center_freq = 36.48, can keep in
   * q25     */

  center_freq = fftLine * samplingFreq; /* q11 or q8 */

  switch (noOfLines) {
    case 1024:
      center_freq = center_freq << 2; /* q13 */
      break;
    case 128:
      center_freq = center_freq << 5; /* q13 */
      break;
    case 512:
      center_freq = (fftLine * samplingFreq) << 3;  // q13
      break;
    case 480:
      center_freq = fMult(center_freq, INV480) << 4;  // q13
      break;
    default:
      center_freq = (FIXP_DBL)0;
  }

  x1 = fMult(center_freq, FOURBY3EM4); /* q13 * q43 - (DFRACT_BITS-1) = q25 */
  x2 = fMult(center_freq, PZZZ76)
       << 2; /* q13 * q41 - (DFRACT_BITS-1) + 2 = q25 */

  atan1 = fixp_atan(x1);
  atan2 = fixp_atan(x2);

  /* q25 (q26 * q30 - (DFRACT_BITS-1)) + q25 (q27 * q30 * q30) */
  bvalFFTLine = fMult(ONE3P3, atan2) + fMult(THREEP5, fMult(atan1, atan1));
  return (bvalFFTLine);
}

/*
   do not consider energies below a certain input signal level,
   i.e. of -96dB or 1 bit at 16 bit PCM resolution,
   might need to be configurable to e.g. 24 bit PCM Input or a lower
   resolution for low bit rates
*/
static void FDKaacEnc_InitMinPCMResolution(int numPb, int *pbOffset,
                                           FIXP_DBL *sfbPCMquantThreshold) {
/* PCM_QUANT_NOISE = FDKpow(10.0f, - 20.f / 10.0f) * ABS_LOW * NORM_PCM_ENERGY *
 * FDKpow(2,PCM_QUANT_THR_SCALE) */
#define PCM_QUANT_NOISE ((FIXP_DBL)0x00547062)

  for (int i = 0; i < numPb; i++) {
    sfbPCMquantThreshold[i] = (pbOffset[i + 1] - pbOffset[i]) * PCM_QUANT_NOISE;
  }
}

static FIXP_DBL getMaskFactor(const FIXP_DBL dbVal_fix, const INT dbVal_e,
                              const FIXP_DBL ten_fix, const INT ten_e) {
  INT q_msk;
  FIXP_DBL mask_factor;

  mask_factor = fPow(ten_fix, DFRACT_BITS - 1 - ten_e, -dbVal_fix,
                     DFRACT_BITS - 1 - dbVal_e, &q_msk);
  q_msk = fixMin(DFRACT_BITS - 1, fixMax(-(DFRACT_BITS - 1), q_msk));

  if ((q_msk > 0) && (mask_factor > (FIXP_DBL)MAXVAL_DBL >> q_msk)) {
    mask_factor = (FIXP_DBL)MAXVAL_DBL;
  } else {
    mask_factor = scaleValue(mask_factor, q_msk);
  }

  return (mask_factor);
}

static void FDKaacEnc_initSpreading(INT numPb, FIXP_DBL *pbBarcValue,
                                    FIXP_DBL *pbMaskLoFactor,
                                    FIXP_DBL *pbMaskHiFactor,
                                    FIXP_DBL *pbMaskLoFactorSprEn,
                                    FIXP_DBL *pbMaskHiFactorSprEn,
                                    const LONG bitrate, const INT blockType)

{
  INT i;
  FIXP_DBL MASKLOWSPREN, MASKHIGHSPREN;

  FIXP_DBL MASKHIGH = (FIXP_DBL)0x30000000;               /* 1.5 in q29 */
  FIXP_DBL MASKLOW = (FIXP_DBL)0x60000000;                /* 3.0 in q29 */
  FIXP_DBL MASKLOWSPRENLONG = (FIXP_DBL)0x60000000;       /* 3.0 in q29 */
  FIXP_DBL MASKHIGHSPRENLONG = (FIXP_DBL)0x40000000;      /* 2.0 in q29 */
  FIXP_DBL MASKHIGHSPRENLONGLOWBR = (FIXP_DBL)0x30000000; /* 1.5 in q29 */
  FIXP_DBL MASKLOWSPRENSHORT = (FIXP_DBL)0x40000000;      /* 2.0 in q29 */
  FIXP_DBL MASKHIGHSPRENSHORT = (FIXP_DBL)0x30000000;     /* 1.5 in q29 */
  FIXP_DBL TEN = (FIXP_DBL)0x50000000;                    /* 10.0 in q27 */

  if (blockType != SHORT_WINDOW) {
    MASKLOWSPREN = MASKLOWSPRENLONG;
    MASKHIGHSPREN =
        (bitrate > 20000) ? MASKHIGHSPRENLONG : MASKHIGHSPRENLONGLOWBR;
  } else {
    MASKLOWSPREN = MASKLOWSPRENSHORT;
    MASKHIGHSPREN = MASKHIGHSPRENSHORT;
  }

  for (i = 0; i < numPb; i++) {
    if (i > 0) {
      pbMaskHiFactor[i] = getMaskFactor(
          fMult(MASKHIGH, (pbBarcValue[i] - pbBarcValue[i - 1])), 23, TEN, 27);

      pbMaskLoFactor[i - 1] = getMaskFactor(
          fMult(MASKLOW, (pbBarcValue[i] - pbBarcValue[i - 1])), 23, TEN, 27);

      pbMaskHiFactorSprEn[i] = getMaskFactor(
          fMult(MASKHIGHSPREN, (pbBarcValue[i] - pbBarcValue[i - 1])), 23, TEN,
          27);

      pbMaskLoFactorSprEn[i - 1] = getMaskFactor(
          fMult(MASKLOWSPREN, (pbBarcValue[i] - pbBarcValue[i - 1])), 23, TEN,
          27);
    } else {
      pbMaskHiFactor[i] = (FIXP_DBL)0;
      pbMaskLoFactor[numPb - 1] = (FIXP_DBL)0;
      pbMaskHiFactorSprEn[i] = (FIXP_DBL)0;
      pbMaskLoFactorSprEn[numPb - 1] = (FIXP_DBL)0;
    }
  }
}

static void FDKaacEnc_initBarcValues(INT numPb, INT *pbOffset, INT numLines,
                                     INT samplingFrequency, FIXP_DBL *pbBval) {
  INT i;
  FIXP_DBL MAX_BARC = (FIXP_DBL)0x30000000; /* 24.0 in q25 */

  for (i = 0; i < numPb; i++) {
    FIXP_DBL v1, v2, cur_bark;
    v1 = FDKaacEnc_BarcLineValue(numLines, pbOffset[i], samplingFrequency);
    v2 = FDKaacEnc_BarcLineValue(numLines, pbOffset[i + 1], samplingFrequency);
    cur_bark = (v1 >> 1) + (v2 >> 1);
    pbBval[i] = fixMin(cur_bark, MAX_BARC);
  }
}

static void FDKaacEnc_initMinSnr(const LONG bitrate, const LONG samplerate,
                                 const INT numLines, const INT *sfbOffset,
                                 const INT sfbActive, const INT blockType,
                                 FIXP_DBL *sfbMinSnrLdData) {
  INT sfb;

  /* Fix conversion variables */
  INT qbfac, qperwin, qdiv, qpeprt_const, qpeprt;
  INT qtmp, qsnr, sfbWidth;

  FIXP_DBL MAX_BARC = (FIXP_DBL)0x30000000;   /* 24.0 in q25 */
  FIXP_DBL MAX_BARCP1 = (FIXP_DBL)0x32000000; /* 25.0 in q25 */
  FIXP_DBL BITS2PEFAC = (FIXP_DBL)0x4b851eb8; /* 1.18 in q30 */
  FIXP_DBL PERS2P4 = (FIXP_DBL)0x624dd2f2;    /* 0.024 in q36 */
  FIXP_DBL ONEP5 = (FIXP_DBL)0x60000000;      /* 1.5 in q30 */
  FIXP_DBL MAX_SNR = (FIXP_DBL)0x33333333;    /* 0.8 in q30 */
  FIXP_DBL MIN_SNR = (FIXP_DBL)0x003126e9;    /* 0.003 in q30 */

  FIXP_DBL barcFactor, pePerWindow, pePart, barcWidth;
  FIXP_DBL pePart_const, tmp, snr, one_qsnr, one_point5;

  /* relative number of active barks */
  barcFactor = fDivNorm(fixMin(FDKaacEnc_BarcLineValue(
                                   numLines, sfbOffset[sfbActive], samplerate),
                               MAX_BARC),
                        MAX_BARCP1, &qbfac);

  qbfac = DFRACT_BITS - 1 - qbfac;

  pePerWindow = fDivNorm(bitrate, samplerate, &qperwin);
  qperwin = DFRACT_BITS - 1 - qperwin;
  pePerWindow = fMult(pePerWindow, BITS2PEFAC);
  qperwin = qperwin + 30 - (DFRACT_BITS - 1);
  pePerWindow = fMult(pePerWindow, PERS2P4);
  qperwin = qperwin + 36 - (DFRACT_BITS - 1);

  switch (numLines) {
    case 1024:
      qperwin = qperwin - 10;
      break;
    case 128:
      qperwin = qperwin - 7;
      break;
    case 512:
      qperwin = qperwin - 9;
      break;
    case 480:
      qperwin = qperwin - 9;
      pePerWindow = fMult(pePerWindow, FL2FXCONST_DBL(480.f / 512.f));
      break;
  }

  /* for short blocks it is assumed that more bits are available */
  if (blockType == SHORT_WINDOW) {
    pePerWindow = fMult(pePerWindow, ONEP5);
    qperwin = qperwin + 30 - (DFRACT_BITS - 1);
  }
  pePart_const = fDivNorm(pePerWindow, barcFactor, &qdiv);
  qpeprt_const = qperwin - qbfac + DFRACT_BITS - 1 - qdiv;

  for (sfb = 0; sfb < sfbActive; sfb++) {
    barcWidth =
        FDKaacEnc_BarcLineValue(numLines, sfbOffset[sfb + 1], samplerate) -
        FDKaacEnc_BarcLineValue(numLines, sfbOffset[sfb], samplerate);

    /* adapt to sfb bands */
    pePart = fMult(pePart_const, barcWidth);
    qpeprt = qpeprt_const + 25 - (DFRACT_BITS - 1);

    /* pe -> snr calculation */
    sfbWidth = (sfbOffset[sfb + 1] - sfbOffset[sfb]);
    pePart = fDivNorm(pePart, sfbWidth, &qdiv);
    qpeprt += DFRACT_BITS - 1 - qdiv;

    tmp = f2Pow(pePart, DFRACT_BITS - 1 - qpeprt, &qtmp);
    qtmp = DFRACT_BITS - 1 - qtmp;

    /* Subtract 1.5 */
    qsnr = fixMin(qtmp, 30);
    tmp = tmp >> (qtmp - qsnr);

    if ((30 + 1 - qsnr) > (DFRACT_BITS - 1))
      one_point5 = (FIXP_DBL)0;
    else
      one_point5 = (FIXP_DBL)(ONEP5 >> (30 + 1 - qsnr));

    snr = (tmp >> 1) - (one_point5);
    qsnr -= 1;

    /* max(snr, 1.0) */
    if (qsnr > 0)
      one_qsnr = (FIXP_DBL)(1 << qsnr);
    else
      one_qsnr = (FIXP_DBL)0;

    snr = fixMax(one_qsnr, snr);

    /* 1/snr */
    snr = fDivNorm(one_qsnr, snr, &qsnr);
    qsnr = DFRACT_BITS - 1 - qsnr;
    snr = (qsnr > 30) ? (snr >> (qsnr - 30)) : snr;

    /* upper limit is -1 dB */
    snr = (snr > MAX_SNR) ? MAX_SNR : snr;

    /* lower limit is -25 dB */
    snr = (snr < MIN_SNR) ? MIN_SNR : snr;
    snr = snr << 1;

    sfbMinSnrLdData[sfb] = CalcLdData(snr);
  }
}

AAC_ENCODER_ERROR FDKaacEnc_InitPsyConfiguration(INT bitrate, INT samplerate,
                                                 INT bandwidth, INT blocktype,
                                                 INT granuleLength, INT useIS,
                                                 INT useMS,
                                                 PSY_CONFIGURATION *psyConf,
                                                 FB_TYPE filterbank) {
  AAC_ENCODER_ERROR ErrorStatus;
  INT sfb;
  FIXP_DBL sfbBarcVal[MAX_SFB];
  const INT frameLengthLong = granuleLength;
  const INT frameLengthShort = granuleLength / TRANS_FAC;
  INT downscaleFactor = 1;

  switch (granuleLength) {
    case 256:
    case 240:
      downscaleFactor = 2;
      break;
    case 128:
    case 120:
      downscaleFactor = 4;
      break;
    default:
      downscaleFactor = 1;
      break;
  }

  FDKmemclear(psyConf, sizeof(PSY_CONFIGURATION));
  psyConf->granuleLength = granuleLength;
  psyConf->filterbank = filterbank;

  psyConf->allowIS = (useIS) && ((bitrate / bandwidth) < 5);
  psyConf->allowMS = useMS;

  /* init sfb table */
  ErrorStatus = FDKaacEnc_initSfbTable(samplerate * downscaleFactor, blocktype,
                                       granuleLength * downscaleFactor,
                                       psyConf->sfbOffset, &psyConf->sfbCnt);

  if (ErrorStatus != AAC_ENC_OK) return ErrorStatus;

  /* calculate barc values for each pb */
  FDKaacEnc_initBarcValues(psyConf->sfbCnt, psyConf->sfbOffset,
                           psyConf->sfbOffset[psyConf->sfbCnt], samplerate,
                           sfbBarcVal);

  FDKaacEnc_InitMinPCMResolution(psyConf->sfbCnt, psyConf->sfbOffset,
                                 psyConf->sfbPcmQuantThreshold);

  /* calculate spreading function */
  FDKaacEnc_initSpreading(psyConf->sfbCnt, sfbBarcVal,
                          psyConf->sfbMaskLowFactor, psyConf->sfbMaskHighFactor,
                          psyConf->sfbMaskLowFactorSprEn,
                          psyConf->sfbMaskHighFactorSprEn, bitrate, blocktype);

  /* init ratio */

  psyConf->maxAllowedIncreaseFactor = 2; /* integer */
  psyConf->minRemainingThresholdFactor = (FIXP_SGL)0x0148;
  /* FL2FXCONST_SGL(0.01f); */ /* fract   */

  psyConf->clipEnergy =
      (FIXP_DBL)0x773593ff; /* FL2FXCONST_DBL(1.0e9*NORM_PCM_ENERGY); */

  if (blocktype != SHORT_WINDOW) {
    psyConf->lowpassLine =
        (INT)((2 * bandwidth * frameLengthLong) / samplerate);
    psyConf->lowpassLineLFE = LFE_LOWPASS_LINE;
  } else {
    psyConf->lowpassLine =
        (INT)((2 * bandwidth * frameLengthShort) / samplerate);
    psyConf->lowpassLineLFE = 0; /* LFE only in lonf blocks */
    /* psyConf->clipEnergy /= (TRANS_FAC * TRANS_FAC); */
    psyConf->clipEnergy >>= 6;
  }

  for (sfb = 0; sfb < psyConf->sfbCnt; sfb++) {
    if (psyConf->sfbOffset[sfb] >= psyConf->lowpassLine) break;
  }
  psyConf->sfbActive = fMax(sfb, 1);

  for (sfb = 0; sfb < psyConf->sfbCnt; sfb++) {
    if (psyConf->sfbOffset[sfb] >= psyConf->lowpassLineLFE) break;
  }
  psyConf->sfbActiveLFE = sfb;
  psyConf->sfbActive = fMax(psyConf->sfbActive, psyConf->sfbActiveLFE);

  /* calculate minSnr */
  FDKaacEnc_initMinSnr(bitrate, samplerate * downscaleFactor,
                       psyConf->sfbOffset[psyConf->sfbCnt], psyConf->sfbOffset,
                       psyConf->sfbActive, blocktype, psyConf->sfbMinSnrLdData);

  return AAC_ENC_OK;
}
