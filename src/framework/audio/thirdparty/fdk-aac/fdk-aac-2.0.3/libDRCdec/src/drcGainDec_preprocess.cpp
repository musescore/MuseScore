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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_preprocess.h"
#include "drcDec_tools.h"
#include "FDK_matrixCalloc.h"
#include "drcDec_rom.h"

#define SLOPE_FACTOR_DB_TO_LINEAR \
  FL2FXCONST_DBL(0.1151f * (float)(1 << 3)) /* ln(10) / 20 */

typedef struct {
  int drcSetEffect;
  DUCKING_MODIFICATION* pDMod;
  GAIN_MODIFICATION* pGMod;
  int drcCharacteristicPresent;
  CHARACTERISTIC_FORMAT characteristicFormatSource[2];
  const CUSTOM_DRC_CHAR* pCCharSource[2];
  CHARACTERISTIC_FORMAT characteristicFormatTarget[2];
  const CUSTOM_DRC_CHAR* pCCharTarget[2];
  int slopeIsNegative;
  int limiterPeakTargetPresent;
  FIXP_SGL limiterPeakTarget;
  FIXP_DBL loudnessNormalizationGainDb;
  FIXP_SGL compress;
  FIXP_SGL boost;
} NODE_MODIFICATION;

static DRC_ERROR _getCicpCharacteristic(
    const int cicpCharacteristic,
    CHARACTERISTIC_FORMAT pCharacteristicFormat[2],
    const CUSTOM_DRC_CHAR* pCCharSource[2]) {
  if ((cicpCharacteristic < 1) || (cicpCharacteristic > 11)) {
    return DE_NOT_OK;
  }

  if (cicpCharacteristic < 7) { /* sigmoid characteristic */
    pCharacteristicFormat[CS_LEFT] = CF_SIGMOID;
    pCCharSource[CS_LEFT] =
        (const CUSTOM_DRC_CHAR*)(&cicpDrcCharSigmoidLeft[cicpCharacteristic -
                                                         1]);
    pCharacteristicFormat[CS_RIGHT] = CF_SIGMOID;
    pCCharSource[CS_RIGHT] =
        (const CUSTOM_DRC_CHAR*)(&cicpDrcCharSigmoidRight[cicpCharacteristic -
                                                          1]);
  } else { /* nodes characteristic */
    pCharacteristicFormat[CS_LEFT] = CF_NODES;
    pCCharSource[CS_LEFT] =
        (const CUSTOM_DRC_CHAR*)(&cicpDrcCharNodesLeft[cicpCharacteristic - 7]);
    pCharacteristicFormat[CS_RIGHT] = CF_NODES;
    pCCharSource[CS_RIGHT] =
        (const CUSTOM_DRC_CHAR*)(&cicpDrcCharNodesRight[cicpCharacteristic -
                                                        7]);
  }
  return DE_OK;
}

static int _getSign(FIXP_SGL in) {
  if (in > (FIXP_DBL)0) return 1;
  if (in < (FIXP_DBL)0) return -1;
  return 0;
}

static DRC_ERROR _getSlopeSign(const CHARACTERISTIC_FORMAT drcCharFormat,
                               const CUSTOM_DRC_CHAR* pCChar, int* pSlopeSign) {
  if (drcCharFormat == CF_SIGMOID) {
    *pSlopeSign = (pCChar->sigmoid.flipSign ? 1 : -1);
  } else {
    int k, slopeSign = 0, tmp_slopeSign;
    for (k = 0; k < pCChar->nodes.characteristicNodeCount; k++) {
      if (pCChar->nodes.nodeLevel[k + 1] > pCChar->nodes.nodeLevel[k]) {
        tmp_slopeSign =
            _getSign(pCChar->nodes.nodeGain[k + 1] - pCChar->nodes.nodeGain[k]);
      } else {
        tmp_slopeSign = -_getSign(pCChar->nodes.nodeGain[k + 1] -
                                  pCChar->nodes.nodeGain[k]);
      }
      if ((slopeSign || tmp_slopeSign) && (slopeSign == -tmp_slopeSign))
        return DE_NOT_OK; /* DRC characteristic is not invertible */
      else
        slopeSign = tmp_slopeSign;
    }
    *pSlopeSign = slopeSign;
  }
  return DE_OK;
}

static DRC_ERROR _isSlopeNegative(const CHARACTERISTIC_FORMAT drcCharFormat[2],
                                  const CUSTOM_DRC_CHAR* pCChar[2],
                                  int* pSlopeIsNegative) {
  DRC_ERROR err = DE_OK;
  int slopeSign[2] = {0, 0};

  err = _getSlopeSign(drcCharFormat[CS_LEFT], pCChar[CS_LEFT],
                      &slopeSign[CS_LEFT]);
  if (err) return err;

  err = _getSlopeSign(drcCharFormat[CS_RIGHT], pCChar[CS_RIGHT],
                      &slopeSign[CS_RIGHT]);
  if (err) return err;

  if ((slopeSign[CS_LEFT] || slopeSign[CS_RIGHT]) &&
      (slopeSign[CS_LEFT] == -slopeSign[CS_RIGHT]))
    return DE_NOT_OK; /* DRC characteristic is not invertible */

  *pSlopeIsNegative = (slopeSign[CS_LEFT] < 0);
  return DE_OK;
}

static DRC_ERROR _prepareDrcCharacteristic(const DRC_CHARACTERISTIC* pDChar,
                                           DRC_COEFFICIENTS_UNI_DRC* pCoef,
                                           const int b,
                                           NODE_MODIFICATION* pNodeMod) {
  DRC_ERROR err = DE_OK;
  pNodeMod->drcCharacteristicPresent = pDChar->present;
  if (pNodeMod->drcCharacteristicPresent) {
    if (pDChar->isCICP == 1) {
      err = _getCicpCharacteristic(pDChar->cicpIndex,
                                   pNodeMod->characteristicFormatSource,
                                   pNodeMod->pCCharSource);
      if (err) return err;
    } else {
      pNodeMod->characteristicFormatSource[CS_LEFT] =
          (CHARACTERISTIC_FORMAT)
              pCoef->characteristicLeftFormat[pDChar->custom.left];
      pNodeMod->pCCharSource[CS_LEFT] =
          &(pCoef->customCharacteristicLeft[pDChar->custom.left]);
      pNodeMod->characteristicFormatSource[CS_RIGHT] =
          (CHARACTERISTIC_FORMAT)
              pCoef->characteristicRightFormat[pDChar->custom.right];
      pNodeMod->pCCharSource[CS_RIGHT] =
          &(pCoef->customCharacteristicRight[pDChar->custom.right]);
    }
    err = _isSlopeNegative(pNodeMod->characteristicFormatSource,
                           pNodeMod->pCCharSource, &pNodeMod->slopeIsNegative);
    if (err) return err;

    if (pNodeMod->pGMod != NULL) {
      if (pNodeMod->pGMod[b].targetCharacteristicLeftPresent) {
        pNodeMod->characteristicFormatTarget[CS_LEFT] =
            (CHARACTERISTIC_FORMAT)pCoef->characteristicLeftFormat
                [pNodeMod->pGMod[b].targetCharacteristicLeftIndex];
        pNodeMod->pCCharTarget[CS_LEFT] =
            &(pCoef->customCharacteristicLeft
                  [pNodeMod->pGMod[b].targetCharacteristicLeftIndex]);
      }
      if (pNodeMod->pGMod[b].targetCharacteristicRightPresent) {
        pNodeMod->characteristicFormatTarget[CS_RIGHT] =
            (CHARACTERISTIC_FORMAT)pCoef->characteristicRightFormat
                [pNodeMod->pGMod[b].targetCharacteristicRightIndex];
        pNodeMod->pCCharTarget[CS_RIGHT] =
            &(pCoef->customCharacteristicRight
                  [pNodeMod->pGMod[b].targetCharacteristicRightIndex]);
      }
    }
  }
  return DE_OK;
}

static DRC_ERROR _compressorIO_sigmoid_common(
    const FIXP_DBL tmp,               /* e = 7 */
    const FIXP_DBL gainDbLimit,       /* e = 6 */
    const FIXP_DBL exp,               /* e = 5 */
    const int inverse, FIXP_DBL* out) /* e = 7 */
{
  FIXP_DBL x, tmp1, tmp2, invExp, denom;
  int e_x, e_tmp1, e_tmp2, e_invExp, e_denom, e_out;

  if (exp < FL2FXCONST_DBL(1.0f / (float)(1 << 5))) {
    return DE_NOT_OK;
  }

  /* x = tmp / gainDbLimit; */
  x = fDivNormSigned(tmp, gainDbLimit, &e_x);
  e_x += 7 - 6;
  if (x < (FIXP_DBL)0) {
    return DE_NOT_OK;
  }

  /* out = tmp / pow(1.0f +/- pow(x, exp), 1.0f/exp); */
  tmp1 = fPow(x, e_x, exp, 5, &e_tmp1);
  if (inverse) tmp1 = -tmp1;
  tmp2 = fAddNorm(FL2FXCONST_DBL(1.0f / (float)(1 << 1)), 1, tmp1, e_tmp1,
                  &e_tmp2);
  invExp = fDivNorm(FL2FXCONST_DBL(1.0f / (float)(1 << 1)), exp, &e_invExp);
  e_invExp += 1 - 5;
  if (tmp2 < (FIXP_DBL)0) {
    return DE_NOT_OK;
  }
  denom = fPow(tmp2, e_tmp2, invExp, e_invExp, &e_denom);
  *out = fDivNormSigned(tmp, denom, &e_out);
  e_out += 7 - e_denom;
  *out = scaleValueSaturate(*out, e_out - 7);
  return DE_OK;
}

static DRC_ERROR _compressorIO_sigmoid(const CUSTOM_DRC_CHAR_SIGMOID* pCChar,
                                       const FIXP_DBL inLevelDb, /* e = 7 */
                                       FIXP_DBL* outGainDb)      /* e = 7 */
{
  FIXP_DBL tmp;
  FIXP_SGL exp = pCChar->exp;
  DRC_ERROR err = DE_OK;

  tmp = fMultDiv2((DRC_INPUT_LOUDNESS_TARGET >> 1) - (inLevelDb >> 1),
                  pCChar->ioRatio);
  tmp = SATURATE_LEFT_SHIFT(tmp, 2 + 1 + 1, DFRACT_BITS);
  if (exp < (FIXP_SGL)MAXVAL_SGL) {
    /* x = tmp / gainDbLimit; */
    /* *outGainDb = tmp / pow(1.0f + pow(x, exp), 1.0f/exp); */
    err = _compressorIO_sigmoid_common(tmp, FX_SGL2FX_DBL(pCChar->gain),
                                       FX_SGL2FX_DBL(exp), 0, outGainDb);
    if (err) return err;
  } else {
    *outGainDb =
        tmp; /* scaling of outGainDb (7) is equal to scaling of tmp (7) */
  }
  if (pCChar->flipSign == 1) {
    *outGainDb = -*outGainDb;
  }
  return err;
}

static DRC_ERROR _compressorIO_sigmoid_inverse(
    const CUSTOM_DRC_CHAR_SIGMOID* pCChar, const FIXP_SGL gainDb,
    FIXP_DBL* inLev) {
  DRC_ERROR err = DE_OK;
  FIXP_SGL ioRatio = pCChar->ioRatio;
  FIXP_SGL exp = pCChar->exp;
  FIXP_DBL tmp = FX_SGL2FX_DBL(gainDb), tmp_out;
  int e_out;

  if (pCChar->flipSign == 1) {
    tmp = -tmp;
  }
  if (exp < (FIXP_SGL)MAXVAL_SGL) {
    /* x = tmp / gainDbLimit; */
    /* tmp = tmp / pow(1.0f - pow(x, exp), 1.0f / exp); */
    err = _compressorIO_sigmoid_common(tmp, FX_SGL2FX_DBL(pCChar->gain),
                                       FX_SGL2FX_DBL(exp), 1, &tmp);
    if (err) return err;
  }
  if (ioRatio == (FIXP_SGL)0) {
    return DE_NOT_OK;
  }
  tmp_out = fDivNormSigned(tmp, FX_SGL2FX_DBL(ioRatio), &e_out);
  e_out += 7 - 2;
  tmp_out = fAddNorm(DRC_INPUT_LOUDNESS_TARGET, 7, -tmp_out, e_out, &e_out);
  *inLev = scaleValueSaturate(tmp_out, e_out - 7);

  return err;
}

static DRC_ERROR _compressorIO_nodes(const CUSTOM_DRC_CHAR_NODES* pCChar,
                                     const FIXP_DBL inLevelDb, /* e = 7 */
                                     FIXP_DBL* outGainDb)      /* e = 7 */
{
  int n;
  FIXP_DBL w;
  const FIXP_SGL* nodeLevel = pCChar->nodeLevel;
  const FIXP_SGL* nodeGain = pCChar->nodeGain;

  if (inLevelDb < DRC_INPUT_LOUDNESS_TARGET) {
    for (n = 0; n < pCChar->characteristicNodeCount; n++) {
      if ((inLevelDb <= FX_SGL2FX_DBL(nodeLevel[n])) &&
          (inLevelDb > FX_SGL2FX_DBL(nodeLevel[n + 1]))) {
        w = fDivNorm(inLevelDb - FX_SGL2FX_DBL(nodeLevel[n + 1]),
                     FX_SGL2FX_DBL(nodeLevel[n] - nodeLevel[n + 1]));
        *outGainDb = fMult(w, nodeGain[n]) +
                     fMult((FIXP_DBL)MAXVAL_DBL - w, nodeGain[n + 1]);
        /* *outGainDb = (w * nodeGain[n] + (1.0-w) * nodeGain[n+1]); */
        return DE_OK;
      }
    }
  } else {
    for (n = 0; n < pCChar->characteristicNodeCount; n++) {
      if ((inLevelDb >= FX_SGL2FX_DBL(nodeLevel[n])) &&
          (inLevelDb < FX_SGL2FX_DBL(nodeLevel[n + 1]))) {
        w = fDivNorm(FX_SGL2FX_DBL(nodeLevel[n + 1]) - inLevelDb,
                     FX_SGL2FX_DBL(nodeLevel[n + 1] - nodeLevel[n]));
        *outGainDb = fMult(w, nodeGain[n]) +
                     fMult((FIXP_DBL)MAXVAL_DBL - w, nodeGain[n + 1]);
        /* *outGainDb = (w * nodeGain[n] + (1.0-w) * nodeGain[n+1]); */
        return DE_OK;
      }
    }
  }
  *outGainDb = FX_SGL2FX_DBL(nodeGain[pCChar->characteristicNodeCount]);
  return DE_OK;
}

static DRC_ERROR _compressorIO_nodes_inverse(
    const CUSTOM_DRC_CHAR_NODES* pCChar, const FIXP_SGL gainDb, /* e = 7 */
    FIXP_DBL* inLev)                                            /* e = 7 */
{
  int n;
  int k;
  FIXP_DBL w;
  int gainIsNegative = 0;
  const FIXP_SGL* nodeLevel = pCChar->nodeLevel;
  const FIXP_SGL* nodeGain = pCChar->nodeGain;
  int nodeCount = pCChar->characteristicNodeCount;
  for (k = 0; k < nodeCount; k++) {
    if (pCChar->nodeGain[k + 1] < (FIXP_SGL)0) {
      gainIsNegative = 1;
    }
  }
  if (gainIsNegative == 1) {
    if (gainDb <= nodeGain[nodeCount]) {
      *inLev = FX_SGL2FX_DBL(nodeLevel[nodeCount]);
    } else {
      if (gainDb >= (FIXP_SGL)0) {
        *inLev = DRC_INPUT_LOUDNESS_TARGET;
      } else {
        for (n = 0; n < nodeCount; n++) {
          if ((gainDb <= nodeGain[n]) && (gainDb > nodeGain[n + 1])) {
            FIXP_SGL gainDelta = nodeGain[n] - nodeGain[n + 1];
            if (gainDelta == (FIXP_SGL)0) {
              *inLev = FX_SGL2FX_DBL(nodeLevel[n]);
              return DE_OK;
            }
            w = fDivNorm(gainDb - nodeGain[n + 1], gainDelta);
            *inLev = fMult(w, nodeLevel[n]) +
                     fMult((FIXP_DBL)MAXVAL_DBL - w, nodeLevel[n + 1]);
            /* *inLev = (w * nodeLevel[n] + (1.0-w) * nodeLevel[n+1]); */
            return DE_OK;
          }
        }
        *inLev = FX_SGL2FX_DBL(nodeLevel[nodeCount]);
      }
    }
  } else {
    if (gainDb >= nodeGain[nodeCount]) {
      *inLev = FX_SGL2FX_DBL(nodeLevel[nodeCount]);
    } else {
      if (gainDb <= (FIXP_SGL)0) {
        *inLev = DRC_INPUT_LOUDNESS_TARGET;
      } else {
        for (n = 0; n < nodeCount; n++) {
          if ((gainDb >= nodeGain[n]) && (gainDb < nodeGain[n + 1])) {
            FIXP_SGL gainDelta = nodeGain[n + 1] - nodeGain[n];
            if (gainDelta == (FIXP_SGL)0) {
              *inLev = FX_SGL2FX_DBL(nodeLevel[n]);
              return DE_OK;
            }
            w = fDivNorm(nodeGain[n + 1] - gainDb, gainDelta);
            *inLev = fMult(w, nodeLevel[n]) +
                     fMult((FIXP_DBL)MAXVAL_DBL - w, nodeLevel[n + 1]);
            /* *inLev = (w * nodeLevel[n] + (1.0-w) * nodeLevel[n+1]); */
            return DE_OK;
          }
        }
        *inLev = FX_SGL2FX_DBL(nodeLevel[nodeCount]);
      }
    }
  }
  return DE_OK;
}

static DRC_ERROR _mapGain(const CHARACTERISTIC_FORMAT pCCharFormatSource,
                          const CUSTOM_DRC_CHAR* pCCharSource,
                          const CHARACTERISTIC_FORMAT pCCharFormatTarget,
                          const CUSTOM_DRC_CHAR* pCCharTarget,
                          const FIXP_SGL gainInDb, /* e = 7 */
                          FIXP_DBL* gainOutDb)     /* e = 7 */
{
  FIXP_DBL inLevel = (FIXP_DBL)0;
  DRC_ERROR err = DE_OK;

  switch (pCCharFormatSource) {
    case CF_SIGMOID:
      err = _compressorIO_sigmoid_inverse(
          (const CUSTOM_DRC_CHAR_SIGMOID*)pCCharSource, gainInDb, &inLevel);
      if (err) return err;
      break;
    case CF_NODES:
      err = _compressorIO_nodes_inverse(
          (const CUSTOM_DRC_CHAR_NODES*)pCCharSource, gainInDb, &inLevel);
      if (err) return err;
      break;
    default:
      return DE_NOT_OK;
  }
  switch (pCCharFormatTarget) {
    case CF_SIGMOID:
      err = _compressorIO_sigmoid((const CUSTOM_DRC_CHAR_SIGMOID*)pCCharTarget,
                                  inLevel, gainOutDb);
      if (err) return err;
      break;
    case CF_NODES:
      err = _compressorIO_nodes((const CUSTOM_DRC_CHAR_NODES*)pCCharTarget,
                                inLevel, gainOutDb);
      if (err) return err;
      break;
    default:
      break;
  }
  return DE_OK;
}

static DRC_ERROR _toLinear(
    const NODE_MODIFICATION* nodeMod, const int drcBand,
    const FIXP_SGL gainDb,  /* in: gain value in dB, e = 7 */
    const FIXP_SGL slopeDb, /* in: slope value in dB/deltaTmin, e = 2 */
    FIXP_DBL* gainLin,      /* out: linear gain value, e = 7 */
    FIXP_DBL* slopeLin)     /* out: linear slope value, e = 7 */
{
  FIXP_DBL gainRatio_m = FL2FXCONST_DBL(1.0f / (float)(1 << 1));
  GAIN_MODIFICATION* pGMod = NULL;
  DUCKING_MODIFICATION* pDMod = nodeMod->pDMod;
  FIXP_DBL tmp_dbl, gainDb_modified, gainDb_offset, gainDb_out, gainLin_m,
      slopeLin_m;
  int gainLin_e, gainRatio_e = 1, gainDb_out_e;
  if (nodeMod->pGMod != NULL) {
    pGMod = &(nodeMod->pGMod[drcBand]);
  }
  if (((nodeMod->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) == 0) &&
      (nodeMod->drcSetEffect != EB_FADE) &&
      (nodeMod->drcSetEffect != EB_CLIPPING)) {
    DRC_ERROR err = DE_OK;
    FIXP_DBL gainDbMapped;

    if ((pGMod != NULL) && (nodeMod->drcCharacteristicPresent)) {
      if (((gainDb > (FIXP_SGL)0) && nodeMod->slopeIsNegative) ||
          ((gainDb < (FIXP_SGL)0) && !nodeMod->slopeIsNegative)) {
        /* left side */
        if (pGMod->targetCharacteristicLeftPresent == 1) {
          err = _mapGain(nodeMod->characteristicFormatSource[CS_LEFT],
                         nodeMod->pCCharSource[CS_LEFT],
                         nodeMod->characteristicFormatTarget[CS_LEFT],
                         nodeMod->pCCharTarget[CS_LEFT], gainDb, &gainDbMapped);
          if (err) return err;
          gainRatio_m = fDivNormSigned(
              gainDbMapped, FX_SGL2FX_DBL(gainDb),
              &gainRatio_e); /* target characteristic in payload */
        }
      }

      else { /* if (((gainDb < (FIXP_SGL)0) && nodeMod->slopeIsNegative) ||
                ((gainDb > (FIXP_SGL)0) && !nodeMod->slopeIsNegative)) */

        /* right side */
        if (pGMod->targetCharacteristicRightPresent == 1) {
          err =
              _mapGain(nodeMod->characteristicFormatSource[CS_RIGHT],
                       nodeMod->pCCharSource[CS_RIGHT],
                       nodeMod->characteristicFormatTarget[CS_RIGHT],
                       nodeMod->pCCharTarget[CS_RIGHT], gainDb, &gainDbMapped);
          if (err) return err;
          gainRatio_m = fDivNormSigned(
              gainDbMapped, FX_SGL2FX_DBL(gainDb),
              &gainRatio_e); /* target characteristic in payload */
        }
      }
    }
    if (gainDb < (FIXP_SGL)0) {
      gainRatio_m = fMultDiv2(gainRatio_m, nodeMod->compress);
    } else {
      gainRatio_m = fMultDiv2(gainRatio_m, nodeMod->boost);
    }
    gainRatio_e += 2;
  }
  if ((pGMod != NULL) && (pGMod->gainScalingPresent == 1)) {
    if (gainDb < (FIXP_SGL)0) {
      gainRatio_m = fMultDiv2(gainRatio_m, pGMod->attenuationScaling);
    } else {
      gainRatio_m = fMultDiv2(gainRatio_m, pGMod->amplificationScaling);
    }
    gainRatio_e += 3;
  }
  if ((pDMod != NULL) &&
      (nodeMod->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) &&
      (pDMod->duckingScalingPresent == 1)) {
    gainRatio_m = fMultDiv2(gainRatio_m, pDMod->duckingScaling);
    gainRatio_e += 3;
  }

  gainDb_modified =
      fMultDiv2(gainDb, gainRatio_m); /* resulting e: 7 + gainRatio_e + 1*/
  gainDb_offset = (FIXP_DBL)0;

  if ((pGMod != NULL) && (pGMod->gainOffsetPresent == 1)) {
    /* *gainLin *= (float)pow(2.0, (double)(pGMod->gainOffset/6.0f)); */
    gainDb_offset += FX_SGL2FX_DBL(pGMod->gainOffset) >> 4; /* resulting e: 8 */
  }
  if ((nodeMod->limiterPeakTargetPresent == 1) &&
      (nodeMod->drcSetEffect ==
       EB_CLIPPING)) { /* The only drcSetEffect is "clipping prevention" */
    /* loudnessNormalizationGainModificationDb is included in
     * loudnessNormalizationGainDb */
    /* *gainLin *= (float)pow(2.0, max(0.0, -nodeModification->limiterPeakTarget
     * - nodeModification->loudnessNormalizationGainDb)/6.0); */
    gainDb_offset += fMax(
        (FIXP_DBL)0,
        (FX_SGL2FX_DBL(-nodeMod->limiterPeakTarget) >> 3) -
            (nodeMod->loudnessNormalizationGainDb >> 1)); /* resulting e: 8 */
  }
  if (gainDb_offset != (FIXP_DBL)0) {
    gainDb_out = fAddNorm(gainDb_modified, 7 + gainRatio_e + 1, gainDb_offset,
                          8, &gainDb_out_e);
  } else {
    gainDb_out = gainDb_modified;
    gainDb_out_e = 7 + gainRatio_e + 1;
  }

  /* *gainLin = (float)pow(2.0, (double)(gainDb_modified[1] / 6.0f)); */
  gainLin_m = approxDb2lin(gainDb_out, gainDb_out_e, &gainLin_e);
  *gainLin = scaleValueSaturate(gainLin_m, gainLin_e - 7);

  /* *slopeLin = SLOPE_FACTOR_DB_TO_LINEAR * gainRatio * *gainLin * slopeDb; */
  if (slopeDb == (FIXP_SGL)0) {
    *slopeLin = (FIXP_DBL)0;
  } else {
    tmp_dbl =
        fMult(slopeDb, SLOPE_FACTOR_DB_TO_LINEAR); /* resulting e: 2 - 3 = -1 */
    tmp_dbl = fMult(tmp_dbl, gainRatio_m); /* resulting e: -1 + gainRatio_e */
    if (gainDb_offset !=
        (FIXP_DBL)0) { /* recalculate gainLin from gainDb that wasn't modified
                          by gainOffset and limiterPeakTarget */
      gainLin_m = approxDb2lin(gainDb_modified, 7 + gainRatio_e, &gainLin_e);
    }
    slopeLin_m = fMult(tmp_dbl, gainLin_m);
    *slopeLin =
        scaleValueSaturate(slopeLin_m, -1 + gainRatio_e + gainLin_e - 7);
  }

  if ((nodeMod->limiterPeakTargetPresent == 1) &&
      (nodeMod->drcSetEffect == EB_CLIPPING)) {
    if (*gainLin >= FL2FXCONST_DBL(1.0f / (float)(1 << 7))) {
      *gainLin = FL2FXCONST_DBL(1.0f / (float)(1 << 7));
      *slopeLin = (FIXP_DBL)0;
    }
  }

  return DE_OK;
}

/* prepare buffers containing linear nodes for each gain sequence */
DRC_ERROR
prepareDrcGain(HANDLE_DRC_GAIN_DECODER hGainDec,
               HANDLE_UNI_DRC_GAIN hUniDrcGain, const FIXP_SGL compress,
               const FIXP_SGL boost, const FIXP_DBL loudnessNormalizationGainDb,
               const int activeDrcIndex) {
  int b, g, gainElementIndex;
  DRC_GAIN_BUFFERS* drcGainBuffers = &(hGainDec->drcGainBuffers);
  NODE_MODIFICATION nodeMod;
  FDKmemclear(&nodeMod, sizeof(NODE_MODIFICATION));
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcIndex]);
  DRC_INSTRUCTIONS_UNI_DRC* pInst = pActiveDrc->pInst;
  if (pInst == NULL) return DE_NOT_OK;

  nodeMod.drcSetEffect = pInst->drcSetEffect;

  nodeMod.compress = compress;
  nodeMod.boost = boost;
  nodeMod.loudnessNormalizationGainDb = loudnessNormalizationGainDb;
  nodeMod.limiterPeakTargetPresent = pInst->limiterPeakTargetPresent;
  nodeMod.limiterPeakTarget = pInst->limiterPeakTarget;

  gainElementIndex = 0;
  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    int gainSetIndex = 0;
    int nDrcBands = 0;
    DRC_COEFFICIENTS_UNI_DRC* pCoef = pActiveDrc->pCoef;
    if (pCoef == NULL) return DE_NOT_OK;

    if (!pActiveDrc->channelGroupIsParametricDrc[g]) {
      gainSetIndex = pInst->gainSetIndexForChannelGroup[g];

      if (nodeMod.drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
        nodeMod.pDMod = &(pActiveDrc->duckingModificationForChannelGroup[g]);
        nodeMod.pGMod = NULL;
      } else {
        nodeMod.pGMod = pInst->gainModificationForChannelGroup[g];
        nodeMod.pDMod = NULL;
      }

      nDrcBands = pActiveDrc->bandCountForChannelGroup[g];
      for (b = 0; b < nDrcBands; b++) {
        DRC_ERROR err = DE_OK;
        GAIN_SET* pGainSet = &(pCoef->gainSet[gainSetIndex]);
        int seq = pGainSet->gainSequenceIndex[b];
        DRC_CHARACTERISTIC* pDChar = &(pGainSet->drcCharacteristic[b]);

        /* linearNodeBuffer contains a copy of the gain sequences (consisting of
           nodes) that are relevant for decoding. It also contains gain
           sequences of previous frames. */
        LINEAR_NODE_BUFFER* pLnb =
            &(drcGainBuffers->linearNodeBuffer[pActiveDrc->activeDrcOffset +
                                               gainElementIndex]);
        int i, lnbp;
        lnbp = drcGainBuffers->lnbPointer;
        pLnb->gainInterpolationType =
            (GAIN_INTERPOLATION_TYPE)pGainSet->gainInterpolationType;

        err = _prepareDrcCharacteristic(pDChar, pCoef, b, &nodeMod);
        if (err) return err;

        /* copy a node buffer and convert from dB to linear */
        pLnb->nNodes[lnbp] = fMin((int)hUniDrcGain->nNodes[seq], 16);
        for (i = 0; i < pLnb->nNodes[lnbp]; i++) {
          FIXP_DBL gainLin, slopeLin;
          err = _toLinear(&nodeMod, b, hUniDrcGain->gainNode[seq][i].gainDb,
                          (FIXP_SGL)0, &gainLin, &slopeLin);
          if (err) return err;
          pLnb->linearNode[lnbp][i].gainLin = gainLin;
          pLnb->linearNode[lnbp][i].time = hUniDrcGain->gainNode[seq][i].time;
        }
        gainElementIndex++;
      }
    } else {
      /* parametric DRC not supported */
      gainElementIndex++;
    }
  }
  return DE_OK;
}
