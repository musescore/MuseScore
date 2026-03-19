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

   Author(s):   M. Neuendorf, N. Rettelbach, M. Multrus

   Description: PS parameter extraction, encoding

*******************************************************************************/

/*!
  \file
  \brief  PS parameter extraction, encoding functions $Revision: 96441 $
*/

#include "ps_main.h"
#include "ps_encode.h"
#include "qmf.h"
#include "sbr_misc.h"
#include "sbrenc_ram.h"

#include "genericStds.h"

inline void FDKsbrEnc_addFIXP_DBL(const FIXP_DBL *X, const FIXP_DBL *Y,
                                  FIXP_DBL *Z, INT n) {
  for (INT i = 0; i < n; i++) Z[i] = (X[i] >> 1) + (Y[i] >> 1);
}

#define LOG10_2_10 3.01029995664f /* 10.0f*log10(2.f) */

static const INT
    iidGroupBordersLoRes[QMF_GROUPS_LO_RES + SUBQMF_GROUPS_LO_RES + 1] = {
        0,  1,  2,  3,  4,  5, /* 6 subqmf subbands - 0th qmf subband */
        6,  7,                 /* 2 subqmf subbands - 1st qmf subband */
        8,  9,                 /* 2 subqmf subbands - 2nd qmf subband */
        10, 11, 12, 13, 14, 15, 16, 18, 21, 25, 30, 42, 71};

static const UCHAR
    iidGroupWidthLdLoRes[QMF_GROUPS_LO_RES + SUBQMF_GROUPS_LO_RES] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 4, 5};

static const INT subband2parameter20[QMF_GROUPS_LO_RES + SUBQMF_GROUPS_LO_RES] =
    {1, 0, 0,  1,  2,  3, /* 6 subqmf subbands - 0th qmf subband */
     4, 5,                /* 2 subqmf subbands - 1st qmf subband */
     6, 7,                /* 2 subqmf subbands - 2nd qmf subband */
     8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

typedef enum {
  MAX_TIME_DIFF_FRAMES = 20,
  MAX_PS_NOHEADER_CNT = 10,
  MAX_NOENV_CNT = 10,
  DO_NOT_USE_THIS_MODE = 0x7FFFFF
} __PS_CONSTANTS;

static const FIXP_DBL iidQuant_fx[15] = {
    (FIXP_DBL)0xce000000, (FIXP_DBL)0xdc000000, (FIXP_DBL)0xe4000000,
    (FIXP_DBL)0xec000000, (FIXP_DBL)0xf2000000, (FIXP_DBL)0xf8000000,
    (FIXP_DBL)0xfc000000, (FIXP_DBL)0x00000000, (FIXP_DBL)0x04000000,
    (FIXP_DBL)0x08000000, (FIXP_DBL)0x0e000000, (FIXP_DBL)0x14000000,
    (FIXP_DBL)0x1c000000, (FIXP_DBL)0x24000000, (FIXP_DBL)0x32000000};

static const FIXP_DBL iidQuantFine_fx[31] = {
    (FIXP_DBL)0x9c000001, (FIXP_DBL)0xa6000001, (FIXP_DBL)0xb0000001,
    (FIXP_DBL)0xba000001, (FIXP_DBL)0xc4000000, (FIXP_DBL)0xce000000,
    (FIXP_DBL)0xd4000000, (FIXP_DBL)0xda000000, (FIXP_DBL)0xe0000000,
    (FIXP_DBL)0xe6000000, (FIXP_DBL)0xec000000, (FIXP_DBL)0xf0000000,
    (FIXP_DBL)0xf4000000, (FIXP_DBL)0xf8000000, (FIXP_DBL)0xfc000000,
    (FIXP_DBL)0x00000000, (FIXP_DBL)0x04000000, (FIXP_DBL)0x08000000,
    (FIXP_DBL)0x0c000000, (FIXP_DBL)0x10000000, (FIXP_DBL)0x14000000,
    (FIXP_DBL)0x1a000000, (FIXP_DBL)0x20000000, (FIXP_DBL)0x26000000,
    (FIXP_DBL)0x2c000000, (FIXP_DBL)0x32000000, (FIXP_DBL)0x3c000000,
    (FIXP_DBL)0x45ffffff, (FIXP_DBL)0x4fffffff, (FIXP_DBL)0x59ffffff,
    (FIXP_DBL)0x63ffffff};

static const FIXP_DBL iccQuant[8] = {
    (FIXP_DBL)0x7fffffff, (FIXP_DBL)0x77ef9d7f, (FIXP_DBL)0x6babc97f,
    (FIXP_DBL)0x4ceaf27f, (FIXP_DBL)0x2f0ed3c0, (FIXP_DBL)0x00000000,
    (FIXP_DBL)0xb49ba601, (FIXP_DBL)0x80000000};

static FDK_PSENC_ERROR InitPSData(HANDLE_PS_DATA hPsData) {
  FDK_PSENC_ERROR error = PSENC_OK;

  if (hPsData == NULL) {
    error = PSENC_INVALID_HANDLE;
  } else {
    int i, env;
    FDKmemclear(hPsData, sizeof(PS_DATA));

    for (i = 0; i < PS_MAX_BANDS; i++) {
      hPsData->iidIdxLast[i] = 0;
      hPsData->iccIdxLast[i] = 0;
    }

    hPsData->iidEnable = hPsData->iidEnableLast = 0;
    hPsData->iccEnable = hPsData->iccEnableLast = 0;
    hPsData->iidQuantMode = hPsData->iidQuantModeLast = PS_IID_RES_COARSE;
    hPsData->iccQuantMode = hPsData->iccQuantModeLast = PS_ICC_ROT_A;

    for (env = 0; env < PS_MAX_ENVELOPES; env++) {
      hPsData->iccDiffMode[env] = PS_DELTA_FREQ;
      hPsData->iccDiffMode[env] = PS_DELTA_FREQ;

      for (i = 0; i < PS_MAX_BANDS; i++) {
        hPsData->iidIdx[env][i] = 0;
        hPsData->iccIdx[env][i] = 0;
      }
    }

    hPsData->nEnvelopesLast = 0;

    hPsData->headerCnt = MAX_PS_NOHEADER_CNT;
    hPsData->iidTimeCnt = MAX_TIME_DIFF_FRAMES;
    hPsData->iccTimeCnt = MAX_TIME_DIFF_FRAMES;
    hPsData->noEnvCnt = MAX_NOENV_CNT;
  }

  return error;
}

static FIXP_DBL quantizeCoef(const FIXP_DBL *RESTRICT input, const INT nBands,
                             const FIXP_DBL *RESTRICT quantTable,
                             const INT idxOffset, const INT nQuantSteps,
                             INT *RESTRICT quantOut) {
  INT idx, band;
  FIXP_DBL quantErr = FL2FXCONST_DBL(0.f);

  for (band = 0; band < nBands; band++) {
    for (idx = 0; idx < nQuantSteps - 1; idx++) {
      if (fixp_abs((input[band] >> 1) - (quantTable[idx + 1] >> 1)) >
          fixp_abs((input[band] >> 1) - (quantTable[idx] >> 1))) {
        break;
      }
    }
    quantErr += (fixp_abs(input[band] - quantTable[idx]) >>
                 PS_QUANT_SCALE); /* don't scale before subtraction; diff
                                     smaller (64-25)/64 */
    quantOut[band] = idx - idxOffset;
  }

  return quantErr;
}

static INT getICCMode(const INT nBands, const INT rotType) {
  INT mode = 0;

  switch (nBands) {
    case PS_BANDS_COARSE:
      mode = PS_RES_COARSE;
      break;
    case PS_BANDS_MID:
      mode = PS_RES_MID;
      break;
    default:
      mode = 0;
  }
  if (rotType == PS_ICC_ROT_B) {
    mode += 3;
  }

  return mode;
}

static INT getIIDMode(const INT nBands, const INT iidRes) {
  INT mode = 0;

  switch (nBands) {
    case PS_BANDS_COARSE:
      mode = PS_RES_COARSE;
      break;
    case PS_BANDS_MID:
      mode = PS_RES_MID;
      break;
    default:
      mode = 0;
      break;
  }

  if (iidRes == PS_IID_RES_FINE) {
    mode += 3;
  }

  return mode;
}

static INT envelopeReducible(FIXP_DBL iid[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                             FIXP_DBL icc[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                             INT psBands, INT nEnvelopes) {
#define THRESH_SCALE 7

  INT reducible = 1; /* true */
  INT e = 0, b = 0;
  FIXP_DBL dIid = FL2FXCONST_DBL(0.f);
  FIXP_DBL dIcc = FL2FXCONST_DBL(0.f);

  FIXP_DBL iidErrThreshold, iccErrThreshold;
  FIXP_DBL iidMeanError, iccMeanError;

  /* square values to prevent sqrt,
     multiply bands to prevent division; bands shifted DFRACT_BITS instead
     (DFRACT_BITS-1) because fMultDiv2 used*/
  iidErrThreshold =
      fMultDiv2(FL2FXCONST_DBL(6.5f * 6.5f / (IID_SCALE_FT * IID_SCALE_FT)),
                (FIXP_DBL)(psBands << ((DFRACT_BITS)-THRESH_SCALE)));
  iccErrThreshold =
      fMultDiv2(FL2FXCONST_DBL(0.75f * 0.75f),
                (FIXP_DBL)(psBands << ((DFRACT_BITS)-THRESH_SCALE)));

  if (nEnvelopes <= 1) {
    reducible = 0;
  } else {
    /* mean error criterion */
    for (e = 0; (e < nEnvelopes / 2) && (reducible != 0); e++) {
      iidMeanError = iccMeanError = FL2FXCONST_DBL(0.f);
      for (b = 0; b < psBands; b++) {
        dIid = (iid[2 * e][b] >> 1) -
               (iid[2 * e + 1][b] >> 1); /* scale 1 bit; squared -> 2 bit */
        dIcc = (icc[2 * e][b] >> 1) - (icc[2 * e + 1][b] >> 1);
        iidMeanError += fPow2Div2(dIid) >> (5 - 1); /* + (bands=20) scale = 5 */
        iccMeanError += fPow2Div2(dIcc) >> (5 - 1);
      } /* --> scaling = 7 bit = THRESH_SCALE !! */

      /* instead sqrt values are squared!
         instead of division, multiply threshold with psBands
         scaling necessary!! */

      /* quit as soon as threshold is reached */
      if ((iidMeanError > (iidErrThreshold)) ||
          (iccMeanError > (iccErrThreshold))) {
        reducible = 0;
      }
    }
  } /* nEnvelopes != 1 */

  return reducible;
}

static void processIidData(PS_DATA *psData,
                           FIXP_DBL iid[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                           const INT psBands, const INT nEnvelopes,
                           const FIXP_DBL quantErrorThreshold) {
  INT iidIdxFine[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  INT iidIdxCoarse[PS_MAX_ENVELOPES][PS_MAX_BANDS];

  FIXP_DBL errIID = FL2FXCONST_DBL(0.f);
  FIXP_DBL errIIDFine = FL2FXCONST_DBL(0.f);
  INT bitsIidFreq = 0;
  INT bitsIidTime = 0;
  INT bitsFineTot = 0;
  INT bitsCoarseTot = 0;
  INT error = 0;
  INT env, band;
  INT diffMode[PS_MAX_ENVELOPES], diffModeFine[PS_MAX_ENVELOPES];
  INT loudnDiff = 0;
  INT iidTransmit = 0;

  /* Quantize IID coefficients */
  for (env = 0; env < nEnvelopes; env++) {
    errIID +=
        quantizeCoef(iid[env], psBands, iidQuant_fx, 7, 15, iidIdxCoarse[env]);
    errIIDFine += quantizeCoef(iid[env], psBands, iidQuantFine_fx, 15, 31,
                               iidIdxFine[env]);
  }

  /* normalize error to number of envelopes, ps bands
     errIID /= psBands*nEnvelopes;
     errIIDFine /= psBands*nEnvelopes; */

  /* Check if IID coefficients should be used in this frame */
  psData->iidEnable = 0;
  for (env = 0; env < nEnvelopes; env++) {
    for (band = 0; band < psBands; band++) {
      loudnDiff += fixp_abs(iidIdxCoarse[env][band]);
      iidTransmit++;
    }
  }

  if (loudnDiff >
      fMultI(FL2FXCONST_DBL(0.7f), iidTransmit)) { /* 0.7f empiric value */
    psData->iidEnable = 1;
  }

  /* if iid not active -> RESET data */
  if (psData->iidEnable == 0) {
    psData->iidTimeCnt = MAX_TIME_DIFF_FRAMES;
    for (env = 0; env < nEnvelopes; env++) {
      psData->iidDiffMode[env] = PS_DELTA_FREQ;
      FDKmemclear(psData->iidIdx[env], sizeof(INT) * psBands);
    }
    return;
  }

  /* count COARSE quantization bits for first envelope*/
  bitsIidFreq = FDKsbrEnc_EncodeIid(NULL, iidIdxCoarse[0], NULL, psBands,
                                    PS_IID_RES_COARSE, PS_DELTA_FREQ, &error);

  if ((psData->iidTimeCnt >= MAX_TIME_DIFF_FRAMES) ||
      (psData->iidQuantModeLast == PS_IID_RES_FINE)) {
    bitsIidTime = DO_NOT_USE_THIS_MODE;
  } else {
    bitsIidTime =
        FDKsbrEnc_EncodeIid(NULL, iidIdxCoarse[0], psData->iidIdxLast, psBands,
                            PS_IID_RES_COARSE, PS_DELTA_TIME, &error);
  }

  /* decision DELTA_FREQ vs DELTA_TIME */
  if (bitsIidTime > bitsIidFreq) {
    diffMode[0] = PS_DELTA_FREQ;
    bitsCoarseTot = bitsIidFreq;
  } else {
    diffMode[0] = PS_DELTA_TIME;
    bitsCoarseTot = bitsIidTime;
  }

  /* count COARSE quantization bits for following envelopes*/
  for (env = 1; env < nEnvelopes; env++) {
    bitsIidFreq = FDKsbrEnc_EncodeIid(NULL, iidIdxCoarse[env], NULL, psBands,
                                      PS_IID_RES_COARSE, PS_DELTA_FREQ, &error);
    bitsIidTime =
        FDKsbrEnc_EncodeIid(NULL, iidIdxCoarse[env], iidIdxCoarse[env - 1],
                            psBands, PS_IID_RES_COARSE, PS_DELTA_TIME, &error);

    /* decision DELTA_FREQ vs DELTA_TIME */
    if (bitsIidTime > bitsIidFreq) {
      diffMode[env] = PS_DELTA_FREQ;
      bitsCoarseTot += bitsIidFreq;
    } else {
      diffMode[env] = PS_DELTA_TIME;
      bitsCoarseTot += bitsIidTime;
    }
  }

  /* count FINE quantization bits for first envelope*/
  bitsIidFreq = FDKsbrEnc_EncodeIid(NULL, iidIdxFine[0], NULL, psBands,
                                    PS_IID_RES_FINE, PS_DELTA_FREQ, &error);

  if ((psData->iidTimeCnt >= MAX_TIME_DIFF_FRAMES) ||
      (psData->iidQuantModeLast == PS_IID_RES_COARSE)) {
    bitsIidTime = DO_NOT_USE_THIS_MODE;
  } else {
    bitsIidTime =
        FDKsbrEnc_EncodeIid(NULL, iidIdxFine[0], psData->iidIdxLast, psBands,
                            PS_IID_RES_FINE, PS_DELTA_TIME, &error);
  }

  /* decision DELTA_FREQ vs DELTA_TIME */
  if (bitsIidTime > bitsIidFreq) {
    diffModeFine[0] = PS_DELTA_FREQ;
    bitsFineTot = bitsIidFreq;
  } else {
    diffModeFine[0] = PS_DELTA_TIME;
    bitsFineTot = bitsIidTime;
  }

  /* count FINE quantization bits for following envelopes*/
  for (env = 1; env < nEnvelopes; env++) {
    bitsIidFreq = FDKsbrEnc_EncodeIid(NULL, iidIdxFine[env], NULL, psBands,
                                      PS_IID_RES_FINE, PS_DELTA_FREQ, &error);
    bitsIidTime =
        FDKsbrEnc_EncodeIid(NULL, iidIdxFine[env], iidIdxFine[env - 1], psBands,
                            PS_IID_RES_FINE, PS_DELTA_TIME, &error);

    /* decision DELTA_FREQ vs DELTA_TIME */
    if (bitsIidTime > bitsIidFreq) {
      diffModeFine[env] = PS_DELTA_FREQ;
      bitsFineTot += bitsIidFreq;
    } else {
      diffModeFine[env] = PS_DELTA_TIME;
      bitsFineTot += bitsIidTime;
    }
  }

  if (bitsFineTot == bitsCoarseTot) {
    /* if same number of bits is needed, use the quantization with lower error
     */
    if (errIIDFine < errIID) {
      bitsCoarseTot = DO_NOT_USE_THIS_MODE;
    } else {
      bitsFineTot = DO_NOT_USE_THIS_MODE;
    }
  } else {
    /* const FIXP_DBL minThreshold =
     * FL2FXCONST_DBL(0.2f/(IID_SCALE_FT*PS_QUANT_SCALE_FT)*(psBands*nEnvelopes));
     */
    const FIXP_DBL minThreshold =
        (FIXP_DBL)((LONG)0x00019999 * (psBands * nEnvelopes));

    /* decision RES_FINE vs RES_COARSE                 */
    /* test if errIIDFine*quantErrorThreshold < errIID */
    /* shiftVal 2 comes from scaling of quantErrorThreshold */
    if (fixMax(((errIIDFine >> 1) + (minThreshold >> 1)) >> 1,
               fMult(quantErrorThreshold, errIIDFine)) < (errIID >> 2)) {
      bitsCoarseTot = DO_NOT_USE_THIS_MODE;
    } else if (fixMax(((errIID >> 1) + (minThreshold >> 1)) >> 1,
                      fMult(quantErrorThreshold, errIID)) < (errIIDFine >> 2)) {
      bitsFineTot = DO_NOT_USE_THIS_MODE;
    }
  }

  /* decision RES_FINE vs RES_COARSE */
  if (bitsFineTot < bitsCoarseTot) {
    psData->iidQuantMode = PS_IID_RES_FINE;
    for (env = 0; env < nEnvelopes; env++) {
      psData->iidDiffMode[env] = diffModeFine[env];
      FDKmemcpy(psData->iidIdx[env], iidIdxFine[env], psBands * sizeof(INT));
    }
  } else {
    psData->iidQuantMode = PS_IID_RES_COARSE;
    for (env = 0; env < nEnvelopes; env++) {
      psData->iidDiffMode[env] = diffMode[env];
      FDKmemcpy(psData->iidIdx[env], iidIdxCoarse[env], psBands * sizeof(INT));
    }
  }

  /* Count DELTA_TIME encoding streaks */
  for (env = 0; env < nEnvelopes; env++) {
    if (psData->iidDiffMode[env] == PS_DELTA_TIME)
      psData->iidTimeCnt++;
    else
      psData->iidTimeCnt = 0;
  }
}

static INT similarIid(PS_DATA *psData, const INT psBands,
                      const INT nEnvelopes) {
  const INT diffThr = (psData->iidQuantMode == PS_IID_RES_COARSE) ? 2 : 3;
  const INT sumDiffThr = diffThr * psBands / 4;
  INT similar = 0;
  INT diff = 0;
  INT sumDiff = 0;
  INT env = 0;
  INT b = 0;
  if ((nEnvelopes == psData->nEnvelopesLast) && (nEnvelopes == 1)) {
    similar = 1;
    for (env = 0; env < nEnvelopes; env++) {
      sumDiff = 0;
      b = 0;
      do {
        diff = fixp_abs(psData->iidIdx[env][b] - psData->iidIdxLast[b]);
        sumDiff += diff;
        if ((diff > diffThr) /* more than x quantization steps in any band */
            || (sumDiff > sumDiffThr)) { /* more than x quantisations steps
                                            overall difference */
          similar = 0;
        }
        b++;
      } while ((b < psBands) && (similar > 0));
    }
  } /* nEnvelopes==1  */

  return similar;
}

static INT similarIcc(PS_DATA *psData, const INT psBands,
                      const INT nEnvelopes) {
  const INT diffThr = 2;
  const INT sumDiffThr = diffThr * psBands / 4;
  INT similar = 0;
  INT diff = 0;
  INT sumDiff = 0;
  INT env = 0;
  INT b = 0;
  if ((nEnvelopes == psData->nEnvelopesLast) && (nEnvelopes == 1)) {
    similar = 1;
    for (env = 0; env < nEnvelopes; env++) {
      sumDiff = 0;
      b = 0;
      do {
        diff = fixp_abs(psData->iccIdx[env][b] - psData->iccIdxLast[b]);
        sumDiff += diff;
        if ((diff > diffThr) /* more than x quantisation step in any band */
            || (sumDiff > sumDiffThr)) { /* more than x quantisations steps
                                            overall difference */
          similar = 0;
        }
        b++;
      } while ((b < psBands) && (similar > 0));
    }
  } /* nEnvelopes==1  */

  return similar;
}

static void processIccData(
    PS_DATA *psData,
    FIXP_DBL icc[PS_MAX_ENVELOPES][PS_MAX_BANDS], /* const input values:
                                                     unable to declare as
                                                     const, since it does
                                                     not poINT to const
                                                     memory */
    const INT psBands, const INT nEnvelopes) {
  FIXP_DBL errICC = FL2FXCONST_DBL(0.f);
  INT env, band;
  INT bitsIccFreq, bitsIccTime;
  INT error = 0;
  INT inCoherence = 0, iccTransmit = 0;
  INT *iccIdxLast;

  iccIdxLast = psData->iccIdxLast;

  /* Quantize ICC coefficients */
  for (env = 0; env < nEnvelopes; env++) {
    errICC +=
        quantizeCoef(icc[env], psBands, iccQuant, 0, 8, psData->iccIdx[env]);
  }

  /* Check if ICC coefficients should be used */
  psData->iccEnable = 0;
  for (env = 0; env < nEnvelopes; env++) {
    for (band = 0; band < psBands; band++) {
      inCoherence += psData->iccIdx[env][band];
      iccTransmit++;
    }
  }
  if (inCoherence >
      fMultI(FL2FXCONST_DBL(0.5f), iccTransmit)) { /* 0.5f empiric value */
    psData->iccEnable = 1;
  }

  if (psData->iccEnable == 0) {
    psData->iccTimeCnt = MAX_TIME_DIFF_FRAMES;
    for (env = 0; env < nEnvelopes; env++) {
      psData->iccDiffMode[env] = PS_DELTA_FREQ;
      FDKmemclear(psData->iccIdx[env], sizeof(INT) * psBands);
    }
    return;
  }

  for (env = 0; env < nEnvelopes; env++) {
    bitsIccFreq = FDKsbrEnc_EncodeIcc(NULL, psData->iccIdx[env], NULL, psBands,
                                      PS_DELTA_FREQ, &error);

    if (psData->iccTimeCnt < MAX_TIME_DIFF_FRAMES) {
      bitsIccTime = FDKsbrEnc_EncodeIcc(NULL, psData->iccIdx[env], iccIdxLast,
                                        psBands, PS_DELTA_TIME, &error);
    } else {
      bitsIccTime = DO_NOT_USE_THIS_MODE;
    }

    if (bitsIccFreq > bitsIccTime) {
      psData->iccDiffMode[env] = PS_DELTA_TIME;
      psData->iccTimeCnt++;
    } else {
      psData->iccDiffMode[env] = PS_DELTA_FREQ;
      psData->iccTimeCnt = 0;
    }
    iccIdxLast = psData->iccIdx[env];
  }
}

static void calculateIID(FIXP_DBL ldPwrL[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL ldPwrR[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL iid[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         INT nEnvelopes, INT psBands) {
  INT i = 0;
  INT env = 0;
  for (env = 0; env < nEnvelopes; env++) {
    for (i = 0; i < psBands; i++) {
      /* iid[env][i] = 10.0f*(float)log10(pwrL[env][i]/pwrR[env][i]);
       */
      FIXP_DBL IID = fMultDiv2(FL2FXCONST_DBL(LOG10_2_10 / IID_SCALE_FT),
                               (ldPwrL[env][i] - ldPwrR[env][i]));

      IID = fixMin(IID, (FIXP_DBL)(MAXVAL_DBL >> (LD_DATA_SHIFT + 1)));
      IID = fixMax(IID, (FIXP_DBL)(MINVAL_DBL >> (LD_DATA_SHIFT + 1)));
      iid[env][i] = IID << (LD_DATA_SHIFT + 1);
    }
  }
}

static void calculateICC(FIXP_DBL pwrL[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL pwrR[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL pwrCr[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL pwrCi[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         FIXP_DBL icc[PS_MAX_ENVELOPES][PS_MAX_BANDS],
                         INT nEnvelopes, INT psBands) {
  INT i = 0;
  INT env = 0;
  INT border = psBands;

  switch (psBands) {
    case PS_BANDS_COARSE:
      border = 5;
      break;
    case PS_BANDS_MID:
      border = 11;
      break;
    default:
      break;
  }

  for (env = 0; env < nEnvelopes; env++) {
    for (i = 0; i < border; i++) {
      /* icc[env][i] = min( pwrCr[env][i] / (float) sqrt(pwrL[env][i] *
       * pwrR[env][i]) , 1.f);
       */
      int scale;
      FIXP_DBL invNrg = invSqrtNorm2(
          fMax(fMult(pwrL[env][i], pwrR[env][i]), (FIXP_DBL)1), &scale);
      icc[env][i] =
          SATURATE_LEFT_SHIFT(fMult(pwrCr[env][i], invNrg), scale, DFRACT_BITS);
    }

    for (; i < psBands; i++) {
      int denom_e;
      FIXP_DBL denom_m = fMultNorm(pwrL[env][i], pwrR[env][i], &denom_e);

      if (denom_m == (FIXP_DBL)0) {
        icc[env][i] = (FIXP_DBL)MAXVAL_DBL;
      } else {
        int num_e, result_e;
        FIXP_DBL num_m, result_m;

        num_e = CountLeadingBits(
            fixMax(fixp_abs(pwrCr[env][i]), fixp_abs(pwrCi[env][i])));
        num_m = fPow2Div2((pwrCr[env][i] << num_e)) +
                fPow2Div2((pwrCi[env][i] << num_e));

        result_m = fDivNorm(num_m, denom_m, &result_e);
        result_e += (-2 * num_e + 1) - denom_e;
        icc[env][i] = scaleValueSaturate(sqrtFixp(result_m >> (result_e & 1)),
                                         (result_e + (result_e & 1)) >> 1);
      }
    }
  }
}

void FDKsbrEnc_initPsBandNrgScale(HANDLE_PS_ENCODE hPsEncode) {
  INT group, bin;
  INT nIidGroups = hPsEncode->nQmfIidGroups + hPsEncode->nSubQmfIidGroups;

  FDKmemclear(hPsEncode->psBandNrgScale, PS_MAX_BANDS * sizeof(SCHAR));

  for (group = 0; group < nIidGroups; group++) {
    /* Translate group to bin */
    bin = hPsEncode->subband2parameterIndex[group];

    /* Translate from 20 bins to 10 bins */
    if (hPsEncode->psEncMode == PS_BANDS_COARSE) {
      bin = bin >> 1;
    }

    hPsEncode->psBandNrgScale[bin] =
        (hPsEncode->psBandNrgScale[bin] == 0)
            ? (hPsEncode->iidGroupWidthLd[group] + 5)
            : (fixMax(hPsEncode->iidGroupWidthLd[group],
                      hPsEncode->psBandNrgScale[bin]) +
               1);
  }
}

FDK_PSENC_ERROR FDKsbrEnc_CreatePSEncode(HANDLE_PS_ENCODE *phPsEncode) {
  FDK_PSENC_ERROR error = PSENC_OK;

  if (phPsEncode == NULL) {
    error = PSENC_INVALID_HANDLE;
  } else {
    HANDLE_PS_ENCODE hPsEncode = NULL;
    if (NULL == (hPsEncode = GetRam_PsEncode())) {
      error = PSENC_MEMORY_ERROR;
      goto bail;
    }
    FDKmemclear(hPsEncode, sizeof(PS_ENCODE));
    *phPsEncode = hPsEncode; /* return allocated handle */
  }
bail:
  return error;
}

FDK_PSENC_ERROR FDKsbrEnc_InitPSEncode(HANDLE_PS_ENCODE hPsEncode,
                                       const PS_BANDS psEncMode,
                                       const FIXP_DBL iidQuantErrorThreshold) {
  FDK_PSENC_ERROR error = PSENC_OK;

  if (NULL == hPsEncode) {
    error = PSENC_INVALID_HANDLE;
  } else {
    if (PSENC_OK != (InitPSData(&hPsEncode->psData))) {
      goto bail;
    }

    switch (psEncMode) {
      case PS_BANDS_COARSE:
      case PS_BANDS_MID:
        hPsEncode->nQmfIidGroups = QMF_GROUPS_LO_RES;
        hPsEncode->nSubQmfIidGroups = SUBQMF_GROUPS_LO_RES;
        FDKmemcpy(hPsEncode->iidGroupBorders, iidGroupBordersLoRes,
                  (hPsEncode->nQmfIidGroups + hPsEncode->nSubQmfIidGroups + 1) *
                      sizeof(INT));
        FDKmemcpy(hPsEncode->subband2parameterIndex, subband2parameter20,
                  (hPsEncode->nQmfIidGroups + hPsEncode->nSubQmfIidGroups) *
                      sizeof(INT));
        FDKmemcpy(hPsEncode->iidGroupWidthLd, iidGroupWidthLdLoRes,
                  (hPsEncode->nQmfIidGroups + hPsEncode->nSubQmfIidGroups) *
                      sizeof(UCHAR));
        break;
      default:
        error = PSENC_INIT_ERROR;
        goto bail;
    }

    hPsEncode->psEncMode = psEncMode;
    hPsEncode->iidQuantErrorThreshold = iidQuantErrorThreshold;
    FDKsbrEnc_initPsBandNrgScale(hPsEncode);
  }
bail:
  return error;
}

FDK_PSENC_ERROR FDKsbrEnc_DestroyPSEncode(HANDLE_PS_ENCODE *phPsEncode) {
  FDK_PSENC_ERROR error = PSENC_OK;

  if (NULL != phPsEncode) {
    FreeRam_PsEncode(phPsEncode);
  }

  return error;
}

typedef struct {
  FIXP_DBL pwrL[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL pwrR[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL ldPwrL[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL ldPwrR[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL pwrCr[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL pwrCi[PS_MAX_ENVELOPES][PS_MAX_BANDS];

} PS_PWR_DATA;

FDK_PSENC_ERROR FDKsbrEnc_PSEncode(
    HANDLE_PS_ENCODE hPsEncode, HANDLE_PS_OUT hPsOut, UCHAR *dynBandScale,
    UINT maxEnvelopes,
    FIXP_DBL *hybridData[HYBRID_FRAMESIZE][MAX_PS_CHANNELS][2],
    const INT frameSize, const INT sendHeader) {
  FDK_PSENC_ERROR error = PSENC_OK;

  HANDLE_PS_DATA hPsData = &hPsEncode->psData;
  FIXP_DBL iid[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  FIXP_DBL icc[PS_MAX_ENVELOPES][PS_MAX_BANDS];
  int envBorder[PS_MAX_ENVELOPES + 1];

  int group, bin, col, subband, band;
  int i = 0;

  int env = 0;
  int psBands = (int)hPsEncode->psEncMode;
  int nIidGroups = hPsEncode->nQmfIidGroups + hPsEncode->nSubQmfIidGroups;
  int nEnvelopes = fixMin(maxEnvelopes, (UINT)PS_MAX_ENVELOPES);

  C_ALLOC_SCRATCH_START(pwrData, PS_PWR_DATA, 1)

  for (env = 0; env < nEnvelopes + 1; env++) {
    envBorder[env] = fMultI(GetInvInt(nEnvelopes), frameSize * env);
  }

  for (env = 0; env < nEnvelopes; env++) {
    /* clear energy array */
    for (band = 0; band < psBands; band++) {
      pwrData->pwrL[env][band] = pwrData->pwrR[env][band] =
          pwrData->pwrCr[env][band] = pwrData->pwrCi[env][band] = FIXP_DBL(1);
    }

    /**** calculate energies and correlation ****/

    /* start with hybrid data */
    for (group = 0; group < nIidGroups; group++) {
      /* Translate group to bin */
      bin = hPsEncode->subband2parameterIndex[group];

      /* Translate from 20 bins to 10 bins */
      if (hPsEncode->psEncMode == PS_BANDS_COARSE) {
        bin >>= 1;
      }

      /* determine group border */
      int bScale = hPsEncode->psBandNrgScale[bin];

      FIXP_DBL pwrL_env_bin = pwrData->pwrL[env][bin];
      FIXP_DBL pwrR_env_bin = pwrData->pwrR[env][bin];
      FIXP_DBL pwrCr_env_bin = pwrData->pwrCr[env][bin];
      FIXP_DBL pwrCi_env_bin = pwrData->pwrCi[env][bin];

      int scale = (int)dynBandScale[bin];
      for (col = envBorder[env]; col < envBorder[env + 1]; col++) {
        for (subband = hPsEncode->iidGroupBorders[group];
             subband < hPsEncode->iidGroupBorders[group + 1]; subband++) {
          FIXP_DBL l_real = (hybridData[col][0][0][subband]) << scale;
          FIXP_DBL l_imag = (hybridData[col][0][1][subband]) << scale;
          FIXP_DBL r_real = (hybridData[col][1][0][subband]) << scale;
          FIXP_DBL r_imag = (hybridData[col][1][1][subband]) << scale;

          pwrL_env_bin += (fPow2Div2(l_real) + fPow2Div2(l_imag)) >> bScale;
          pwrR_env_bin += (fPow2Div2(r_real) + fPow2Div2(r_imag)) >> bScale;
          pwrCr_env_bin +=
              (fMultDiv2(l_real, r_real) + fMultDiv2(l_imag, r_imag)) >> bScale;
          pwrCi_env_bin +=
              (fMultDiv2(r_real, l_imag) - fMultDiv2(l_real, r_imag)) >> bScale;
        }
      }
      /* assure, nrg's of left and right channel are not negative; necessary on
       * 16 bit multiply units */
      pwrData->pwrL[env][bin] = fixMax((FIXP_DBL)0, pwrL_env_bin);
      pwrData->pwrR[env][bin] = fixMax((FIXP_DBL)0, pwrR_env_bin);

      pwrData->pwrCr[env][bin] = pwrCr_env_bin;
      pwrData->pwrCi[env][bin] = pwrCi_env_bin;

    } /* nIidGroups */

    /* calc logarithmic energy */
    LdDataVector(pwrData->pwrL[env], pwrData->ldPwrL[env], psBands);
    LdDataVector(pwrData->pwrR[env], pwrData->ldPwrR[env], psBands);

  } /* nEnvelopes */

  /* calculate iid and icc */
  calculateIID(pwrData->ldPwrL, pwrData->ldPwrR, iid, nEnvelopes, psBands);
  calculateICC(pwrData->pwrL, pwrData->pwrR, pwrData->pwrCr, pwrData->pwrCi,
               icc, nEnvelopes, psBands);

  /*** Envelope Reduction ***/
  while (envelopeReducible(iid, icc, psBands, nEnvelopes)) {
    int e = 0;
    /* sum energies of two neighboring envelopes */
    nEnvelopes >>= 1;
    for (e = 0; e < nEnvelopes; e++) {
      FDKsbrEnc_addFIXP_DBL(pwrData->pwrL[2 * e], pwrData->pwrL[2 * e + 1],
                            pwrData->pwrL[e], psBands);
      FDKsbrEnc_addFIXP_DBL(pwrData->pwrR[2 * e], pwrData->pwrR[2 * e + 1],
                            pwrData->pwrR[e], psBands);
      FDKsbrEnc_addFIXP_DBL(pwrData->pwrCr[2 * e], pwrData->pwrCr[2 * e + 1],
                            pwrData->pwrCr[e], psBands);
      FDKsbrEnc_addFIXP_DBL(pwrData->pwrCi[2 * e], pwrData->pwrCi[2 * e + 1],
                            pwrData->pwrCi[e], psBands);

      /* calc logarithmic energy */
      LdDataVector(pwrData->pwrL[e], pwrData->ldPwrL[e], psBands);
      LdDataVector(pwrData->pwrR[e], pwrData->ldPwrR[e], psBands);

      /* reduce number of envelopes and adjust borders */
      envBorder[e] = envBorder[2 * e];
    }
    envBorder[nEnvelopes] = envBorder[2 * nEnvelopes];

    /* re-calculate iid and icc */
    calculateIID(pwrData->ldPwrL, pwrData->ldPwrR, iid, nEnvelopes, psBands);
    calculateICC(pwrData->pwrL, pwrData->pwrR, pwrData->pwrCr, pwrData->pwrCi,
                 icc, nEnvelopes, psBands);
  }

  /*  */
  if (sendHeader) {
    hPsData->headerCnt = MAX_PS_NOHEADER_CNT;
    hPsData->iidTimeCnt = MAX_TIME_DIFF_FRAMES;
    hPsData->iccTimeCnt = MAX_TIME_DIFF_FRAMES;
    hPsData->noEnvCnt = MAX_NOENV_CNT;
  }

  /*** Parameter processing, quantisation etc ***/
  processIidData(hPsData, iid, psBands, nEnvelopes,
                 hPsEncode->iidQuantErrorThreshold);
  processIccData(hPsData, icc, psBands, nEnvelopes);

  /*** Initialize output struct ***/

  /* PS Header on/off ? */
  if ((hPsData->headerCnt < MAX_PS_NOHEADER_CNT) &&
      ((hPsData->iidQuantMode == hPsData->iidQuantModeLast) &&
       (hPsData->iccQuantMode == hPsData->iccQuantModeLast)) &&
      ((hPsData->iidEnable == hPsData->iidEnableLast) &&
       (hPsData->iccEnable == hPsData->iccEnableLast))) {
    hPsOut->enablePSHeader = 0;
  } else {
    hPsOut->enablePSHeader = 1;
    hPsData->headerCnt = 0;
  }

  /* nEnvelopes = 0 ? */
  if ((hPsData->noEnvCnt < MAX_NOENV_CNT) &&
      (similarIid(hPsData, psBands, nEnvelopes)) &&
      (similarIcc(hPsData, psBands, nEnvelopes))) {
    hPsOut->nEnvelopes = nEnvelopes = 0;
    hPsData->noEnvCnt++;
  } else {
    hPsData->noEnvCnt = 0;
  }

  if (nEnvelopes > 0) {
    hPsOut->enableIID = hPsData->iidEnable;
    hPsOut->iidMode = getIIDMode(psBands, hPsData->iidQuantMode);

    hPsOut->enableICC = hPsData->iccEnable;
    hPsOut->iccMode = getICCMode(psBands, hPsData->iccQuantMode);

    hPsOut->enableIpdOpd = 0;
    hPsOut->frameClass = 0;
    hPsOut->nEnvelopes = nEnvelopes;

    for (env = 0; env < nEnvelopes; env++) {
      hPsOut->frameBorder[env] = envBorder[env + 1];
      hPsOut->deltaIID[env] = (PS_DELTA)hPsData->iidDiffMode[env];
      hPsOut->deltaICC[env] = (PS_DELTA)hPsData->iccDiffMode[env];
      for (band = 0; band < psBands; band++) {
        hPsOut->iid[env][band] = hPsData->iidIdx[env][band];
        hPsOut->icc[env][band] = hPsData->iccIdx[env][band];
      }
    }

    /* IPD OPD not supported right now */
    FDKmemclear(hPsOut->ipd,
                PS_MAX_ENVELOPES * PS_MAX_BANDS * sizeof(PS_DELTA));
    for (env = 0; env < PS_MAX_ENVELOPES; env++) {
      hPsOut->deltaIPD[env] = PS_DELTA_FREQ;
      hPsOut->deltaOPD[env] = PS_DELTA_FREQ;
    }

    FDKmemclear(hPsOut->ipdLast, PS_MAX_BANDS * sizeof(INT));
    FDKmemclear(hPsOut->opdLast, PS_MAX_BANDS * sizeof(INT));

    for (band = 0; band < PS_MAX_BANDS; band++) {
      hPsOut->iidLast[band] = hPsData->iidIdxLast[band];
      hPsOut->iccLast[band] = hPsData->iccIdxLast[band];
    }

    /* save iids and iccs for differential time coding in the next frame */
    hPsData->nEnvelopesLast = nEnvelopes;
    hPsData->iidEnableLast = hPsData->iidEnable;
    hPsData->iccEnableLast = hPsData->iccEnable;
    hPsData->iidQuantModeLast = hPsData->iidQuantMode;
    hPsData->iccQuantModeLast = hPsData->iccQuantMode;
    for (i = 0; i < psBands; i++) {
      hPsData->iidIdxLast[i] = hPsData->iidIdx[nEnvelopes - 1][i];
      hPsData->iccIdxLast[i] = hPsData->iccIdx[nEnvelopes - 1][i];
    }
  } /* Envelope > 0 */

  C_ALLOC_SCRATCH_END(pwrData, PS_PWR_DATA, 1)

  return error;
}
