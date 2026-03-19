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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_tools.h"
#include "fixpoint_math.h"
#include "drcDecoder.h"

int getDeltaTmin(const int sampleRate) {
  /* half_ms = round (0.0005 * sampleRate); */
  int half_ms = (sampleRate + 1000) / 2000;
  int deltaTmin = 1;
  if (sampleRate < 1000) {
    return DE_NOT_OK;
  }
  while (deltaTmin <= half_ms) {
    deltaTmin = deltaTmin << 1;
  }
  return deltaTmin;
}

DRC_COEFFICIENTS_UNI_DRC* selectDrcCoefficients(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, const int location) {
  int n;
  int c = -1;
  for (n = 0; n < hUniDrcConfig->drcCoefficientsUniDrcCount; n++) {
    if (hUniDrcConfig->drcCoefficientsUniDrc[n].drcLocation == location) {
      c = n;
    }
  }
  if (c >= 0) {
    return &(hUniDrcConfig->drcCoefficientsUniDrc[c]);
  }
  return NULL; /* possible during bitstream parsing */
}

DRC_INSTRUCTIONS_UNI_DRC* selectDrcInstructions(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, const int drcSetId) {
  int i;
  for (i = 0; i < hUniDrcConfig->drcInstructionsCountInclVirtual; i++) {
    if (hUniDrcConfig->drcInstructionsUniDrc[i].drcSetId == drcSetId) {
      return &(hUniDrcConfig->drcInstructionsUniDrc[i]);
    }
  }
  return NULL;
}

DOWNMIX_INSTRUCTIONS* selectDownmixInstructions(
    HANDLE_UNI_DRC_CONFIG hUniDrcConfig, const int downmixId) {
  int i;
  for (i = 0; i < hUniDrcConfig->downmixInstructionsCount; i++) {
    if (hUniDrcConfig->downmixInstructions[i].downmixId == downmixId) {
      return &(hUniDrcConfig->downmixInstructions[i]);
    }
  }
  return NULL;
}

DRC_ERROR
deriveDrcChannelGroups(
    const int drcSetEffect,                                    /* in */
    const int channelCount,                                    /* in */
    const SCHAR* gainSetIndex,                                 /* in */
    const DUCKING_MODIFICATION* duckingModificationForChannel, /* in */
    UCHAR* nDrcChannelGroups,                                  /* out */
    SCHAR* uniqueIndex,     /* out (gainSetIndexForChannelGroup) */
    SCHAR* groupForChannel, /* out */
    DUCKING_MODIFICATION* duckingModificationForChannelGroup) /* out */
{
  int duckingSequence = -1;
  int c, n, g, match, idx;
  FIXP_SGL factor;
  FIXP_SGL uniqueScaling[8];

  for (g = 0; g < 8; g++) {
    uniqueIndex[g] = -10;
    uniqueScaling[g] = FIXP_SGL(-1.0f);
  }

  g = 0;

  if (drcSetEffect & EB_DUCK_OTHER) {
    for (c = 0; c < channelCount; c++) {
      match = 0;
      if (c >= 8) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      factor = duckingModificationForChannel[c].duckingScaling;
      if (idx < 0) {
        for (n = 0; n < g; n++) {
          if (uniqueScaling[n] == factor) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 8) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          uniqueScaling[g] = factor;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        if ((duckingSequence > 0) && (duckingSequence != idx)) {
          return DE_NOT_OK;
        }
        duckingSequence = idx;
        groupForChannel[c] = -1;
      }
    }
    if (duckingSequence == -1) {
      return DE_NOT_OK;
    }
  } else if (drcSetEffect & EB_DUCK_SELF) {
    for (c = 0; c < channelCount; c++) {
      match = 0;
      if (c >= 8) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      factor = duckingModificationForChannel[c].duckingScaling;
      if (idx >= 0) {
        for (n = 0; n < g; n++) {
          if ((uniqueIndex[n] == idx) && (uniqueScaling[n] == factor)) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 8) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          uniqueScaling[g] = factor;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        groupForChannel[c] = -1;
      }
    }
  } else { /* no ducking */
    for (c = 0; c < channelCount; c++) {
      if (c >= 8) return DE_MEMORY_ERROR;
      idx = gainSetIndex[c];
      match = 0;
      if (idx >= 0) {
        for (n = 0; n < g; n++) {
          if (uniqueIndex[n] == idx) {
            match = 1;
            groupForChannel[c] = n;
            break;
          }
        }
        if (match == 0) {
          if (g >= 8) return DE_MEMORY_ERROR;
          uniqueIndex[g] = idx;
          groupForChannel[c] = g;
          g++;
        }
      } else {
        groupForChannel[c] = -1;
      }
    }
  }
  *nDrcChannelGroups = g;

  if (drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    for (g = 0; g < *nDrcChannelGroups; g++) {
      if (drcSetEffect & EB_DUCK_OTHER) {
        uniqueIndex[g] = duckingSequence;
      }
      duckingModificationForChannelGroup[g].duckingScaling = uniqueScaling[g];
      if (uniqueScaling[g] != FL2FXCONST_SGL(1.0f / (float)(1 << 2))) {
        duckingModificationForChannelGroup[g].duckingScalingPresent = 1;
      } else {
        duckingModificationForChannelGroup[g].duckingScalingPresent = 0;
      }
    }
  }

  return DE_OK;
}

FIXP_DBL
dB2lin(const FIXP_DBL dB_m, const int dB_e, int* pLin_e) {
  /* get linear value from dB.
     return lin_val = 10^(dB_val/20) = 2^(log2(10)/20*dB_val)
     with dB_val = dB_m *2^dB_e and lin_val = lin_m * 2^lin_e */
  FIXP_DBL lin_m =
      f2Pow(fMult(dB_m, FL2FXCONST_DBL(0.1660964f * (float)(1 << 2))), dB_e - 2,
            pLin_e);

  return lin_m;
}

FIXP_DBL
lin2dB(const FIXP_DBL lin_m, const int lin_e, int* pDb_e) {
  /* get dB value from linear value.
     return dB_val = 20*log10(lin_val)
     with dB_val = dB_m *2^dB_e and lin_val = lin_m * 2^lin_e */
  FIXP_DBL dB_m;

  if (lin_m == (FIXP_DBL)0) { /* return very small value representing -inf */
    dB_m = (FIXP_DBL)MINVAL_DBL;
    *pDb_e = DFRACT_BITS - 1;
  } else {
    /* 20*log10(lin_val) = 20/log2(10)*log2(lin_val) */
    dB_m = fMultDiv2(FL2FXCONST_DBL(6.02059991f / (float)(1 << 3)),
                     fLog2(lin_m, lin_e, pDb_e));
    *pDb_e += 3 + 1;
  }

  return dB_m;
}

FIXP_DBL
approxDb2lin(const FIXP_DBL dB_m, const int dB_e, int* pLin_e) {
  /* get linear value from approximate dB.
     return lin_val = 2^(dB_val/6)
     with dB_val = dB_m *2^dB_e and lin_val = lin_m * 2^lin_e */
  FIXP_DBL lin_m =
      f2Pow(fMult(dB_m, FL2FXCONST_DBL(0.1666667f * (float)(1 << 2))), dB_e - 2,
            pLin_e);

  return lin_m;
}

int bitstreamContainsMultibandDrc(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                  const int downmixId) {
  int i, g, d, seq;
  DRC_INSTRUCTIONS_UNI_DRC* pInst;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = NULL;
  int isMultiband = 0;

  pCoef = selectDrcCoefficients(hUniDrcConfig, LOCATION_SELECTED);
  if (pCoef == NULL) return 0;

  for (i = 0; i < hUniDrcConfig->drcInstructionsUniDrcCount; i++) {
    pInst = &(hUniDrcConfig->drcInstructionsUniDrc[i]);
    for (d = 0; d < pInst->downmixIdCount; d++) {
      if (downmixId == pInst->downmixId[d]) {
        for (g = 0; g < pInst->nDrcChannelGroups; g++) {
          seq = pInst->gainSetIndexForChannelGroup[g];
          if (pCoef->gainSet[seq].bandCount > 1) {
            isMultiband = 1;
          }
        }
      }
    }
  }

  return isMultiband;
}

FIXP_DBL getDownmixOffset(DOWNMIX_INSTRUCTIONS* pDown, int baseChannelCount) {
  FIXP_DBL downmixOffset = FL2FXCONST_DBL(1.0f / (1 << 1)); /* e = 1 */
  if ((pDown->bsDownmixOffset == 1) || (pDown->bsDownmixOffset == 2)) {
    int e_a, e_downmixOffset;
    FIXP_DBL a, q;
    if (baseChannelCount <= pDown->targetChannelCount) return downmixOffset;

    q = fDivNorm((FIXP_DBL)pDown->targetChannelCount,
                 (FIXP_DBL)baseChannelCount); /* e = 0 */
    a = lin2dB(q, 0, &e_a);
    if (pDown->bsDownmixOffset == 2) {
      e_a += 1; /* a *= 2 */
    }
    /* a = 0.5 * round (a) */
    a = fixp_round(a, e_a) >> 1;
    downmixOffset = dB2lin(a, e_a, &e_downmixOffset);
    downmixOffset = scaleValue(downmixOffset, e_downmixOffset - 1);
  }
  return downmixOffset;
}
