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
#include "drcDec_tools.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_init.h"

static DRC_ERROR _generateDrcInstructionsDerivedData(
    HANDLE_DRC_GAIN_DECODER hGainDec, HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
    DRC_INSTRUCTIONS_UNI_DRC* pInst, DRC_COEFFICIENTS_UNI_DRC* pCoef,
    ACTIVE_DRC* pActiveDrc) {
  DRC_ERROR err = DE_OK;
  int g;
  int gainElementCount = 0;
  UCHAR nDrcChannelGroups = 0;
  SCHAR gainSetIndexForChannelGroup[8];

  err = deriveDrcChannelGroups(
      pInst->drcSetEffect, pInst->drcChannelCount, pInst->gainSetIndex,
      pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)
          ? pInst->duckingModificationForChannel
          : NULL,
      &nDrcChannelGroups, gainSetIndexForChannelGroup,
      pActiveDrc->channelGroupForChannel,
      pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)
          ? pActiveDrc->duckingModificationForChannelGroup
          : NULL);
  if (err) return (err);

  /* sanity check */
  if (nDrcChannelGroups != pInst->nDrcChannelGroups) return DE_NOT_OK;
  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    if (gainSetIndexForChannelGroup[g] != pInst->gainSetIndexForChannelGroup[g])
      return DE_NOT_OK;
  }

  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    int seq = pInst->gainSetIndexForChannelGroup[g];
    if (seq != -1 && (hUniDrcConfig->drcCoefficientsUniDrcCount == 0 ||
                      seq >= pCoef->gainSetCount)) {
      pActiveDrc->channelGroupIsParametricDrc[g] = 1;
    } else {
      pActiveDrc->channelGroupIsParametricDrc[g] = 0;
      if (seq >= pCoef->gainSetCount) {
        return DE_NOT_OK;
      }
    }
  }

  /* gainElementCount */
  if (pInst->drcSetEffect & (EB_DUCK_OTHER | EB_DUCK_SELF)) {
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      pActiveDrc->bandCountForChannelGroup[g] = 1;
    }
    pActiveDrc->gainElementCount =
        pInst->nDrcChannelGroups; /* one gain element per channel group */
  } else {
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      if (pActiveDrc->channelGroupIsParametricDrc[g]) {
        gainElementCount++;
        pActiveDrc->bandCountForChannelGroup[g] = 1;
      } else {
        int seq, bandCount;
        seq = pInst->gainSetIndexForChannelGroup[g];
        bandCount = pCoef->gainSet[seq].bandCount;
        pActiveDrc->bandCountForChannelGroup[g] = bandCount;
        gainElementCount += bandCount;
      }
    }
    pActiveDrc->gainElementCount = gainElementCount;
  }

  /* prepare gainElementForGroup (cumulated sum of bandCountForChannelGroup) */
  pActiveDrc->gainElementForGroup[0] = 0;
  for (g = 1; g < pInst->nDrcChannelGroups; g++) {
    pActiveDrc->gainElementForGroup[g] =
        pActiveDrc->gainElementForGroup[g - 1] +
        pActiveDrc->bandCountForChannelGroup[g - 1]; /* index of first gain
                                                        sequence in channel
                                                        group */
  }

  return DE_OK;
}

DRC_ERROR
initGainDec(HANDLE_DRC_GAIN_DECODER hGainDec) {
  int i, j, k;

  /* sanity check */
  if (hGainDec->deltaTminDefault > hGainDec->frameSize) return DE_NOT_OK;

  for (i = 0; i < MAX_ACTIVE_DRCS; i++) {
    for (j = 0; j < 8; j++) {
      /* use startup node at the beginning */
      hGainDec->activeDrc[i].lnbIndexForChannel[j][0] = 0;
      for (k = 1; k < NUM_LNB_FRAMES; k++) {
        hGainDec->activeDrc[i].lnbIndexForChannel[j][k] = -1;
      }
    }
  }

  for (j = 0; j < 8; j++) {
    hGainDec->channelGain[j] = FL2FXCONST_DBL(1.0f / (float)(1 << 8));
  }

  for (i = 0; i < 4 * 1024 / 256; i++) {
    hGainDec->dummySubbandGains[i] = FL2FXCONST_DBL(1.0f / (float)(1 << 7));
  }

  hGainDec->status = 0; /* startup */

  return DE_OK;
}

void initDrcGainBuffers(const int frameSize, DRC_GAIN_BUFFERS* drcGainBuffers) {
  int i, c, j;
  /* prepare 12 instances of node buffers */
  for (i = 0; i < 12; i++) {
    for (j = 0; j < NUM_LNB_FRAMES; j++) {
      drcGainBuffers->linearNodeBuffer[i].nNodes[j] = 1;
      drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].gainLin =
          FL2FXCONST_DBL(1.0f / (float)(1 << 7));
      if (j == 0) {
        drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].time =
            0; /* initialize last node with startup node */
      } else {
        drcGainBuffers->linearNodeBuffer[i].linearNode[j][0].time =
            frameSize - 1;
      }
    }
  }

  /* prepare dummyLnb, a linearNodeBuffer containing a constant gain of 0 dB,
   * for the "no DRC processing" case */
  drcGainBuffers->dummyLnb.gainInterpolationType = GIT_LINEAR;
  for (i = 0; i < NUM_LNB_FRAMES; i++) {
    drcGainBuffers->dummyLnb.nNodes[i] = 1;
    drcGainBuffers->dummyLnb.linearNode[i][0].gainLin =
        FL2FXCONST_DBL(1.0f / (float)(1 << 7));
    drcGainBuffers->dummyLnb.linearNode[i][0].time = frameSize - 1;
  }

  /* prepare channelGain delay line */
  for (c = 0; c < 8; c++) {
    for (i = 0; i < NUM_LNB_FRAMES; i++) {
      drcGainBuffers->channelGain[c][i] =
          FL2FXCONST_DBL(1.0f / (float)(1 << 8));
    }
  }

  drcGainBuffers->lnbPointer = 0;
}

DRC_ERROR
initActiveDrc(HANDLE_DRC_GAIN_DECODER hGainDec,
              HANDLE_UNI_DRC_CONFIG hUniDrcConfig, const int drcSetIdSelected,
              const int downmixIdSelected) {
  int g, isMultiband = 0;
  DRC_ERROR err = DE_OK;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = NULL;
  DRC_COEFFICIENTS_UNI_DRC* pCoef = NULL;

  pInst = selectDrcInstructions(hUniDrcConfig, drcSetIdSelected);
  if (pInst == NULL) {
    return DE_NOT_OK;
  }

  if (pInst->drcSetId >= 0) {
    pCoef = selectDrcCoefficients(hUniDrcConfig, pInst->drcLocation);
    if (pCoef == NULL) {
      return DE_NOT_OK;
    }

    if (pCoef->drcFrameSizePresent) {
      if (pCoef->drcFrameSize != hGainDec->frameSize) {
        return DE_NOT_OK;
      }
    }

    err = _generateDrcInstructionsDerivedData(
        hGainDec, hUniDrcConfig, pInst, pCoef,
        &(hGainDec->activeDrc[hGainDec->nActiveDrcs]));
    if (err) return err;
  }

  hGainDec->activeDrc[hGainDec->nActiveDrcs].pInst = pInst;
  hGainDec->activeDrc[hGainDec->nActiveDrcs].pCoef = pCoef;

  for (g = 0; g < pInst->nDrcChannelGroups; g++) {
    if (hGainDec->activeDrc[hGainDec->nActiveDrcs].bandCountForChannelGroup[g] >
        1) {
      if (hGainDec->multiBandActiveDrcIndex != -1) {
        return DE_NOT_OK;
      }
      isMultiband = 1;
    }
  }

  if (isMultiband) {
    /* Keep activeDrc index of multiband DRC set */
    hGainDec->multiBandActiveDrcIndex = hGainDec->nActiveDrcs;
  }

  if ((hGainDec->channelGainActiveDrcIndex == -1) &&
      (downmixIdSelected == DOWNMIX_ID_BASE_LAYOUT) &&
      (hUniDrcConfig->drcInstructionsUniDrcCount >
       0)) { /* use this activeDrc to apply channelGains */
    hGainDec->channelGainActiveDrcIndex = hGainDec->nActiveDrcs;
  }

  hGainDec->nActiveDrcs++;
  if (hGainDec->nActiveDrcs > MAX_ACTIVE_DRCS) return DE_NOT_OK;

  return DE_OK;
}

DRC_ERROR
initActiveDrcOffset(HANDLE_DRC_GAIN_DECODER hGainDec) {
  int a, accGainElementCount;

  accGainElementCount = 0;
  for (a = 0; a < hGainDec->nActiveDrcs; a++) {
    hGainDec->activeDrc[a].activeDrcOffset = accGainElementCount;
    accGainElementCount += hGainDec->activeDrc[a].gainElementCount;
    if (accGainElementCount > 12) {
      hGainDec->nActiveDrcs = a;
      return DE_NOT_OK;
    }
  }

  return DE_OK;
}
