/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2020 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Description: Quantizing & coding

*******************************************************************************/

#include "qc_main.h"
#include "quantize.h"
#include "interface.h"
#include "adj_thr.h"
#include "sf_estim.h"
#include "bit_cnt.h"
#include "dyn_bits.h"
#include "channel_map.h"
#include "aacEnc_ram.h"

#include "genericStds.h"

#define AACENC_DZQ_BR_THR 32000 /* Dead zone quantizer bitrate threshold */

typedef struct {
  QCDATA_BR_MODE bitrateMode;
  LONG vbrQualFactor;
} TAB_VBR_QUAL_FACTOR;

static const TAB_VBR_QUAL_FACTOR tableVbrQualFactor[] = {
    {QCDATA_BR_MODE_VBR_1,
     FL2FXCONST_DBL(0.150f)}, /* Approx.  32 kbps mono   AAC-LC + SBR + PS */
    {QCDATA_BR_MODE_VBR_2,
     FL2FXCONST_DBL(0.162f)}, /* Approx.  64 kbps stereo AAC-LC + SBR      */
    {QCDATA_BR_MODE_VBR_3,
     FL2FXCONST_DBL(0.176f)}, /* Approx.  96 kbps stereo AAC-LC            */
    {QCDATA_BR_MODE_VBR_4,
     FL2FXCONST_DBL(0.120f)}, /* Approx. 128 kbps stereo AAC-LC            */
    {QCDATA_BR_MODE_VBR_5,
     FL2FXCONST_DBL(0.070f)} /* Approx. 192 kbps stereo AAC-LC            */
};

static INT isConstantBitrateMode(const QCDATA_BR_MODE bitrateMode) {
  return (((bitrateMode == QCDATA_BR_MODE_CBR) ||
           (bitrateMode == QCDATA_BR_MODE_SFR) ||
           (bitrateMode == QCDATA_BR_MODE_FF))
              ? 1
              : 0);
}

typedef enum {
  FRAME_LEN_BYTES_MODULO = 1,
  FRAME_LEN_BYTES_INT = 2
} FRAME_LEN_RESULT_MODE;

/* forward declarations */

static INT FDKaacEnc_calcMaxValueInSfb(INT sfbCnt, INT maxSfbPerGroup,
                                       INT sfbPerGroup, INT* RESTRICT sfbOffset,
                                       SHORT* RESTRICT quantSpectrum,
                                       UINT* RESTRICT maxValue);

static void FDKaacEnc_crashRecovery(INT nChannels,
                                    PSY_OUT_ELEMENT* psyOutElement,
                                    QC_OUT* qcOut, QC_OUT_ELEMENT* qcElement,
                                    INT bitsToSave, AUDIO_OBJECT_TYPE aot,
                                    UINT syntaxFlags, SCHAR epConfig);

static AAC_ENCODER_ERROR FDKaacEnc_reduceBitConsumption(
    int* iterations, const int maxIterations, int gainAdjustment,
    int* chConstraintsFulfilled, int* calculateQuant, int nChannels,
    PSY_OUT_ELEMENT* psyOutElement, QC_OUT* qcOut, QC_OUT_ELEMENT* qcOutElement,
    ELEMENT_BITS* elBits, AUDIO_OBJECT_TYPE aot, UINT syntaxFlags,
    SCHAR epConfig);

void FDKaacEnc_QCClose(QC_STATE** phQCstate, QC_OUT** phQC);

/*****************************************************************************

    functionname: FDKaacEnc_calcFrameLen
    description:
    returns:
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_calcFrameLen(INT bitRate, INT sampleRate,
                                  INT granuleLength,
                                  FRAME_LEN_RESULT_MODE mode) {
  INT result;

  result = ((granuleLength) >> 3) * (bitRate);

  switch (mode) {
    case FRAME_LEN_BYTES_MODULO:
      result %= sampleRate;
      break;
    case FRAME_LEN_BYTES_INT:
      result /= sampleRate;
      break;
  }
  return (result);
}

/*****************************************************************************

    functionname:FDKaacEnc_framePadding
    description: Calculates if padding is needed for actual frame
    returns:
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_framePadding(INT bitRate, INT sampleRate,
                                  INT granuleLength, INT* paddingRest) {
  INT paddingOn;
  INT difference;

  paddingOn = 0;

  difference = FDKaacEnc_calcFrameLen(bitRate, sampleRate, granuleLength,
                                      FRAME_LEN_BYTES_MODULO);
  *paddingRest -= difference;

  if (*paddingRest <= 0) {
    paddingOn = 1;
    *paddingRest += sampleRate;
  }

  return (paddingOn);
}

/*********************************************************************************

         functionname: FDKaacEnc_QCOutNew
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_QCOutNew(QC_OUT** phQC, const INT nElements,
                                     const INT nChannels, const INT nSubFrames,
                                     UCHAR* dynamic_RAM) {
  AAC_ENCODER_ERROR ErrorStatus;
  int n, i;
  int elInc = 0, chInc = 0;

  for (n = 0; n < nSubFrames; n++) {
    phQC[n] = GetRam_aacEnc_QCout(n);
    if (phQC[n] == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto QCOutNew_bail;
    }

    for (i = 0; i < nChannels; i++) {
      phQC[n]->pQcOutChannels[i] = GetRam_aacEnc_QCchannel(chInc, dynamic_RAM);
      if (phQC[n]->pQcOutChannels[i] == NULL) {
        ErrorStatus = AAC_ENC_NO_MEMORY;
        goto QCOutNew_bail;
      }

      chInc++;
    } /* nChannels */

    for (i = 0; i < nElements; i++) {
      phQC[n]->qcElement[i] = GetRam_aacEnc_QCelement(elInc);
      if (phQC[n]->qcElement[i] == NULL) {
        ErrorStatus = AAC_ENC_NO_MEMORY;
        goto QCOutNew_bail;
      }
      elInc++;

      /* initialize pointer to dynamic buffer which are used in adjust
       * thresholds */
      phQC[n]->qcElement[i]->dynMem_Ah_Flag = dynamic_RAM + (P_BUF_1);
      phQC[n]->qcElement[i]->dynMem_Thr_Exp =
          dynamic_RAM + (P_BUF_1) + ADJ_THR_AH_FLAG_SIZE;
      phQC[n]->qcElement[i]->dynMem_SfbNActiveLinesLdData =
          dynamic_RAM + (P_BUF_1) + ADJ_THR_AH_FLAG_SIZE + ADJ_THR_THR_EXP_SIZE;

    } /* nElements */

  } /* nSubFrames */

  return AAC_ENC_OK;

QCOutNew_bail:
  return ErrorStatus;
}

/*********************************************************************************

         functionname: FDKaacEnc_QCOutInit
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_QCOutInit(QC_OUT* phQC[(1)], const INT nSubFrames,
                                      const CHANNEL_MAPPING* cm) {
  INT n, i, ch;

  for (n = 0; n < nSubFrames; n++) {
    INT chInc = 0;
    for (i = 0; i < cm->nElements; i++) {
      for (ch = 0; ch < cm->elInfo[i].nChannelsInEl; ch++) {
        phQC[n]->qcElement[i]->qcOutChannel[ch] =
            phQC[n]->pQcOutChannels[chInc];
        chInc++;
      } /* chInEl */
    }   /* nElements */
  }     /* nSubFrames */

  return AAC_ENC_OK;
}

/*********************************************************************************

         functionname: FDKaacEnc_QCNew
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_QCNew(QC_STATE** phQC, INT nElements,
                                  UCHAR* dynamic_RAM) {
  AAC_ENCODER_ERROR ErrorStatus;
  int i;

  QC_STATE* hQC = GetRam_aacEnc_QCstate();
  *phQC = hQC;
  if (hQC == NULL) {
    ErrorStatus = AAC_ENC_NO_MEMORY;
    goto QCNew_bail;
  }

  if (FDKaacEnc_AdjThrNew(&hQC->hAdjThr, nElements)) {
    ErrorStatus = AAC_ENC_NO_MEMORY;
    goto QCNew_bail;
  }

  if (FDKaacEnc_BCNew(&(hQC->hBitCounter), dynamic_RAM)) {
    ErrorStatus = AAC_ENC_NO_MEMORY;
    goto QCNew_bail;
  }

  for (i = 0; i < nElements; i++) {
    hQC->elementBits[i] = GetRam_aacEnc_ElementBits(i);
    if (hQC->elementBits[i] == NULL) {
      ErrorStatus = AAC_ENC_NO_MEMORY;
      goto QCNew_bail;
    }
  }

  return AAC_ENC_OK;

QCNew_bail:
  FDKaacEnc_QCClose(phQC, NULL);
  return ErrorStatus;
}

/*********************************************************************************

         functionname: FDKaacEnc_QCInit
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_QCInit(QC_STATE* hQC, struct QC_INIT* init,
                                   const ULONG initFlags) {
  AAC_ENCODER_ERROR err = AAC_ENC_OK;

  int i;
  hQC->maxBitsPerFrame = init->maxBits;
  hQC->minBitsPerFrame = init->minBits;
  hQC->nElements = init->channelMapping->nElements;
  if ((initFlags != 0) || ((init->bitrateMode != QCDATA_BR_MODE_FF) &&
                           (hQC->bitResTotMax != init->bitRes))) {
    hQC->bitResTot = init->bitRes;
  }
  hQC->bitResTotMax = init->bitRes;
  hQC->maxBitFac = init->maxBitFac;
  hQC->bitrateMode = init->bitrateMode;
  hQC->invQuant = init->invQuant;
  hQC->maxIterations = init->maxIterations;

  /* 0: full bitreservoir, 1: reduced bitreservoir, 2: disabled bitreservoir */
  hQC->bitResMode = init->bitResMode;

  hQC->padding.paddingRest = init->padding.paddingRest;

  hQC->globHdrBits = init->staticBits; /* Bit overhead due to transport */

  err = FDKaacEnc_InitElementBits(
      hQC, init->channelMapping, init->bitrate,
      (init->averageBits / init->nSubFrames) - hQC->globHdrBits,
      hQC->maxBitsPerFrame / init->channelMapping->nChannelsEff);
  if (err != AAC_ENC_OK) goto bail;

  hQC->vbrQualFactor = FL2FXCONST_DBL(0.f);
  for (i = 0;
       i < (int)(sizeof(tableVbrQualFactor) / sizeof(TAB_VBR_QUAL_FACTOR));
       i++) {
    if (hQC->bitrateMode == tableVbrQualFactor[i].bitrateMode) {
      hQC->vbrQualFactor = (FIXP_DBL)tableVbrQualFactor[i].vbrQualFactor;
      break;
    }
  }

  if (init->channelMapping->nChannelsEff == 1 &&
      (init->bitrate / init->channelMapping->nChannelsEff) <
          AACENC_DZQ_BR_THR &&
      init->isLowDelay !=
          0) /* watch out here: init->bitrate is the bitrate "minus" the
                standard SBR bitrate (=2500kbps) --> for the FDK the OFFSTE
                tuning should start somewhere below 32000kbps-2500kbps ... so
                everything is fine here */
  {
    hQC->dZoneQuantEnable = 1;
  } else {
    hQC->dZoneQuantEnable = 0;
  }

  FDKaacEnc_AdjThrInit(
      hQC->hAdjThr, init->meanPe, hQC->invQuant, init->channelMapping,
      init->sampleRate, /* output sample rate */
      init->bitrate,    /* total bitrate */
      init->isLowDelay, /* if set, calc bits2PE factor
                           depending on samplerate */
      init->bitResMode  /* for a small bitreservoir, the pe
                           correction is calc'd differently */
      ,
      hQC->dZoneQuantEnable, init->bitDistributionMode, hQC->vbrQualFactor);

bail:
  return err;
}

/*********************************************************************************

         functionname: FDKaacEnc_QCMainPrepare
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_QCMainPrepare(
    ELEMENT_INFO* elInfo, ATS_ELEMENT* RESTRICT adjThrStateElement,
    PSY_OUT_ELEMENT* RESTRICT psyOutElement,
    QC_OUT_ELEMENT* RESTRICT qcOutElement, AUDIO_OBJECT_TYPE aot,
    UINT syntaxFlags, SCHAR epConfig) {
  AAC_ENCODER_ERROR ErrorStatus = AAC_ENC_OK;
  INT nChannels = elInfo->nChannelsInEl;

  PSY_OUT_CHANNEL** RESTRICT psyOutChannel =
      psyOutElement->psyOutChannel; /* may be modified in-place */

  FDKaacEnc_CalcFormFactor(qcOutElement->qcOutChannel, psyOutChannel,
                           nChannels);

  /* prepare and calculate PE without reduction */
  FDKaacEnc_peCalculation(&qcOutElement->peData, psyOutChannel,
                          qcOutElement->qcOutChannel, &psyOutElement->toolsInfo,
                          adjThrStateElement, nChannels);

  ErrorStatus = FDKaacEnc_ChannelElementWrite(
      NULL, elInfo, NULL, psyOutElement, psyOutElement->psyOutChannel,
      syntaxFlags, aot, epConfig, &qcOutElement->staticBitsUsed, 0);

  return ErrorStatus;
}

/*********************************************************************************

         functionname: FDKaacEnc_AdjustBitrate
         description:  adjusts framelength via padding on a frame to frame
basis, to achieve a bitrate that demands a non byte aligned framelength return:
errorcode

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_AdjustBitrate(
    QC_STATE* RESTRICT hQC, CHANNEL_MAPPING* RESTRICT cm, INT* avgTotalBits,
    INT bitRate,       /* total bitrate */
    INT sampleRate,    /* output sampling rate */
    INT granuleLength) /* frame length */
{
  INT paddingOn;
  INT frameLen;

  /* Do we need an extra padding byte? */
  paddingOn = FDKaacEnc_framePadding(bitRate, sampleRate, granuleLength,
                                     &hQC->padding.paddingRest);

  frameLen =
      paddingOn + FDKaacEnc_calcFrameLen(bitRate, sampleRate, granuleLength,
                                         FRAME_LEN_BYTES_INT);

  *avgTotalBits = frameLen << 3;

  return AAC_ENC_OK;
}

#define isAudioElement(elType) \
  ((elType == ID_SCE) || (elType == ID_CPE) || (elType == ID_LFE))

/*********************************************************************************

         functionname: FDKaacEnc_distributeElementDynBits
         description:  distributes all bits over all elements. The relative bit
                       distibution is described in the ELEMENT_INFO of the
                       appropriate element. The bit distribution table is
                       initialized in FDKaacEnc_InitChannelMapping().
         return:       errorcode

**********************************************************************************/
static AAC_ENCODER_ERROR FDKaacEnc_distributeElementDynBits(
    QC_STATE* hQC, QC_OUT_ELEMENT* qcElement[((8))], CHANNEL_MAPPING* cm,
    INT codeBits) {
  INT i;             /* counter variable */
  INT totalBits = 0; /* sum of bits over all elements */

  for (i = (cm->nElements - 1); i >= 0; i--) {
    if (isAudioElement(cm->elInfo[i].elType)) {
      qcElement[i]->grantedDynBits =
          fMax(0, fMultI(hQC->elementBits[i]->relativeBitsEl, codeBits));
      totalBits += qcElement[i]->grantedDynBits;
    }
  }

  /* Due to inaccuracies with the multiplication, codeBits may differ from
     totalBits. For that case, the difference must be added/substracted again
     to/from one element, i.e:
     Negative differences are substracted from the element with the most bits.
     Positive differences are added to the element with the least bits.
  */
  if (codeBits != totalBits) {
    INT elMaxBits = cm->nElements - 1; /* element with the most bits */
    INT elMinBits = cm->nElements - 1; /* element with the least bits */

    /* Search for biggest and smallest audio element */
    for (i = (cm->nElements - 1); i >= 0; i--) {
      if (isAudioElement(cm->elInfo[i].elType)) {
        if (qcElement[i]->grantedDynBits >
            qcElement[elMaxBits]->grantedDynBits) {
          elMaxBits = i;
        }
        if (qcElement[i]->grantedDynBits <
            qcElement[elMinBits]->grantedDynBits) {
          elMinBits = i;
        }
      }
    }
    /* Compensate for bit distibution difference */
    if (codeBits - totalBits > 0) {
      qcElement[elMinBits]->grantedDynBits += codeBits - totalBits;
    } else {
      qcElement[elMaxBits]->grantedDynBits += codeBits - totalBits;
    }
  }

  return AAC_ENC_OK;
}

/**
 * \brief  Verify whether minBitsPerFrame criterion can be satisfied.
 *
 * This function evaluates the bit consumption only if minBitsPerFrame parameter
 * is not 0. In hyperframing mode the difference between grantedDynBits and
 * usedDynBits of all sub frames results the number of fillbits to be written.
 * This bits can be distrubitued in superframe to reach minBitsPerFrame bit
 * consumption in single AU's. The return value denotes if enough desired fill
 * bits are available to achieve minBitsPerFrame in all frames. This check can
 * only be used within superframes.
 *
 * \param qcOut            Pointer to coding data struct.
 * \param minBitsPerFrame  Minimal number of bits to be consumed in each frame.
 * \param nSubFrames       Number of frames in superframe
 *
 * \return
 *          - 1: all fine
 *          - 0: criterion not fulfilled
 */
static int checkMinFrameBitsDemand(QC_OUT** qcOut, const INT minBitsPerFrame,
                                   const INT nSubFrames) {
  int result = 1; /* all fine*/
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************************************

         functionname: FDKaacEnc_getMinimalStaticBitdemand
         description:  calculate minmal size of static bits by reduction ,
                       to zero spectrum and deactivating tns and MS
         return:       number of static bits

**********************************************************************************/
static int FDKaacEnc_getMinimalStaticBitdemand(CHANNEL_MAPPING* cm,
                                               PSY_OUT** psyOut) {
  AUDIO_OBJECT_TYPE aot = AOT_AAC_LC;
  UINT syntaxFlags = 0;
  SCHAR epConfig = -1;
  int i, bitcount = 0;

  for (i = 0; i < cm->nElements; i++) {
    ELEMENT_INFO elInfo = cm->elInfo[i];

    if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
        (elInfo.elType == ID_LFE)) {
      INT minElBits = 0;

      FDKaacEnc_ChannelElementWrite(NULL, &elInfo, NULL,
                                    psyOut[0]->psyOutElement[i],
                                    psyOut[0]->psyOutElement[i]->psyOutChannel,
                                    syntaxFlags, aot, epConfig, &minElBits, 1);
      bitcount += minElBits;
    }
  }

  return bitcount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static AAC_ENCODER_ERROR FDKaacEnc_prepareBitDistribution(
    QC_STATE* hQC, PSY_OUT** psyOut, QC_OUT** qcOut, CHANNEL_MAPPING* cm,
    QC_OUT_ELEMENT* qcElement[(1)][((8))], INT avgTotalBits,
    INT* totalAvailableBits, INT* avgTotalDynBits) {
  int i;
  /* get maximal allowed dynamic bits */
  qcOut[0]->grantedDynBits =
      (fixMin(hQC->maxBitsPerFrame, avgTotalBits) - hQC->globHdrBits) & ~7;
  qcOut[0]->grantedDynBits -= (qcOut[0]->globalExtBits + qcOut[0]->staticBits +
                               qcOut[0]->elementExtBits);
  qcOut[0]->maxDynBits = ((hQC->maxBitsPerFrame) & ~7) -
                         (qcOut[0]->globalExtBits + qcOut[0]->staticBits +
                          qcOut[0]->elementExtBits);
  /* assure that enough bits are available */
  if ((qcOut[0]->grantedDynBits + hQC->bitResTot) < 0) {
    /* crash recovery allows to reduce static bits to a minimum */
    if ((qcOut[0]->grantedDynBits + hQC->bitResTot) <
        (FDKaacEnc_getMinimalStaticBitdemand(cm, psyOut) -
         qcOut[0]->staticBits))
      return AAC_ENC_BITRES_TOO_LOW;
  }

  /* distribute dynamic bits to each element */
  FDKaacEnc_distributeElementDynBits(hQC, qcElement[0], cm,
                                     qcOut[0]->grantedDynBits);

  *avgTotalDynBits = 0; /*frameDynBits;*/

  *totalAvailableBits = avgTotalBits;

  /* sum up corrected granted PE */
  qcOut[0]->totalGrantedPeCorr = 0;

  for (i = 0; i < cm->nElements; i++) {
    ELEMENT_INFO elInfo = cm->elInfo[i];
    int nChannels = elInfo.nChannelsInEl;

    if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
        (elInfo.elType == ID_LFE)) {
      /* for ( all sub frames ) ... */
      FDKaacEnc_DistributeBits(
          hQC->hAdjThr, hQC->hAdjThr->adjThrStateElem[i],
          psyOut[0]->psyOutElement[i]->psyOutChannel, &qcElement[0][i]->peData,
          &qcElement[0][i]->grantedPe, &qcElement[0][i]->grantedPeCorr,
          nChannels, psyOut[0]->psyOutElement[i]->commonWindow,
          qcElement[0][i]->grantedDynBits, hQC->elementBits[i]->bitResLevelEl,
          hQC->elementBits[i]->maxBitResBitsEl, hQC->maxBitFac,
          hQC->bitResMode);

      *totalAvailableBits += hQC->elementBits[i]->bitResLevelEl;
      /* get total corrected granted PE */
      qcOut[0]->totalGrantedPeCorr += qcElement[0][i]->grantedPeCorr;
    } /*  -end- if(ID_SCE || ID_CPE || ID_LFE) */

  } /* -end- element loop */

  *totalAvailableBits = fMin(hQC->maxBitsPerFrame, (*totalAvailableBits));

  return AAC_ENC_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static AAC_ENCODER_ERROR FDKaacEnc_updateUsedDynBits(
    INT* sumDynBitsConsumed, QC_OUT_ELEMENT* qcElement[((8))],
    CHANNEL_MAPPING* cm) {
  INT i;

  *sumDynBitsConsumed = 0;

  for (i = 0; i < cm->nElements; i++) {
    ELEMENT_INFO elInfo = cm->elInfo[i];

    if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
        (elInfo.elType == ID_LFE)) {
      /* sum up bits consumed */
      *sumDynBitsConsumed += qcElement[i]->dynBitsUsed;
    } /*  -end- if(ID_SCE || ID_CPE || ID_LFE) */

  } /* -end- element loop */

  return AAC_ENC_OK;
}

static INT FDKaacEnc_getTotalConsumedDynBits(QC_OUT** qcOut, INT nSubFrames) {
  INT c, totalBits = 0;

  /* sum up bit consumption for all sub frames */
  for (c = 0; c < nSubFrames; c++) {
    /* bit consumption not valid if dynamic bits
       not available in one sub frame */
    if (qcOut[c]->usedDynBits == -1) return -1;
    totalBits += qcOut[c]->usedDynBits;
  }

  return totalBits;
}

static INT FDKaacEnc_getTotalConsumedBits(QC_OUT** qcOut,
                                          QC_OUT_ELEMENT* qcElement[(1)][((8))],
                                          CHANNEL_MAPPING* cm, INT globHdrBits,
                                          INT nSubFrames) {
  int c, i;
  int totalUsedBits = 0;

  for (c = 0; c < nSubFrames; c++) {
    int dataBits = 0;
    for (i = 0; i < cm->nElements; i++) {
      if ((cm->elInfo[i].elType == ID_SCE) ||
          (cm->elInfo[i].elType == ID_CPE) ||
          (cm->elInfo[i].elType == ID_LFE)) {
        dataBits += qcElement[c][i]->dynBitsUsed +
                    qcElement[c][i]->staticBitsUsed +
                    qcElement[c][i]->extBitsUsed;
      }
    }
    dataBits += qcOut[c]->globalExtBits;

    totalUsedBits += (8 - (dataBits) % 8) % 8;
    totalUsedBits += dataBits + globHdrBits; /* header bits for every frame */
  }
  return totalUsedBits;
}

static AAC_ENCODER_ERROR FDKaacEnc_BitResRedistribution(
    QC_STATE* const hQC, const CHANNEL_MAPPING* const cm,
    const INT avgTotalBits) {
  /* check bitreservoir fill level */
  if (hQC->bitResTot < 0) {
    return AAC_ENC_BITRES_TOO_LOW;
  } else if (hQC->bitResTot > hQC->bitResTotMax) {
    return AAC_ENC_BITRES_TOO_HIGH;
  } else {
    INT i;
    INT totalBits = 0, totalBits_max = 0;

    const int totalBitreservoir =
        fMin(hQC->bitResTot, (hQC->maxBitsPerFrame - avgTotalBits));
    const int totalBitreservoirMax =
        fMin(hQC->bitResTotMax, (hQC->maxBitsPerFrame - avgTotalBits));

    for (i = (cm->nElements - 1); i >= 0; i--) {
      if ((cm->elInfo[i].elType == ID_SCE) ||
          (cm->elInfo[i].elType == ID_CPE) ||
          (cm->elInfo[i].elType == ID_LFE)) {
        hQC->elementBits[i]->bitResLevelEl =
            fMultI(hQC->elementBits[i]->relativeBitsEl, totalBitreservoir);
        totalBits += hQC->elementBits[i]->bitResLevelEl;

        hQC->elementBits[i]->maxBitResBitsEl =
            fMultI(hQC->elementBits[i]->relativeBitsEl, totalBitreservoirMax);
        totalBits_max += hQC->elementBits[i]->maxBitResBitsEl;
      }
    }
    for (i = 0; i < cm->nElements; i++) {
      if ((cm->elInfo[i].elType == ID_SCE) ||
          (cm->elInfo[i].elType == ID_CPE) ||
          (cm->elInfo[i].elType == ID_LFE)) {
        int deltaBits = fMax(totalBitreservoir - totalBits,
                             -hQC->elementBits[i]->bitResLevelEl);
        hQC->elementBits[i]->bitResLevelEl += deltaBits;
        totalBits += deltaBits;

        deltaBits = fMax(totalBitreservoirMax - totalBits_max,
                         -hQC->elementBits[i]->maxBitResBitsEl);
        hQC->elementBits[i]->maxBitResBitsEl += deltaBits;
        totalBits_max += deltaBits;
      }
    }
  }

  return AAC_ENC_OK;
}

AAC_ENCODER_ERROR FDKaacEnc_QCMain(QC_STATE* RESTRICT hQC, PSY_OUT** psyOut,
                                   QC_OUT** qcOut, INT avgTotalBits,
                                   CHANNEL_MAPPING* cm,
                                   const AUDIO_OBJECT_TYPE aot,
                                   UINT syntaxFlags, SCHAR epConfig) {
  int i, c;
  AAC_ENCODER_ERROR ErrorStatus = AAC_ENC_OK;
  INT avgTotalDynBits = 0; /* maximal allowed dynamic bits for all frames */
  INT totalAvailableBits = 0;
  INT nSubFrames = 1;
  const INT isCBRAdjustment = (isConstantBitrateMode(hQC->bitrateMode) ||
                               (hQC->bitResMode != AACENC_BR_MODE_FULL))
                                  ? 1
                                  : 0;

  /*-------------------------------------------- */
  /* redistribute total bitreservoir to elements */
  ErrorStatus = FDKaacEnc_BitResRedistribution(
      hQC, cm, (isCBRAdjustment == 0) ? hQC->maxBitsPerFrame : avgTotalBits);
  if (ErrorStatus != AAC_ENC_OK) {
    return ErrorStatus;
  }

  /*-------------------------------------------- */
  /* fastenc needs one time threshold simulation,
     in case of multiple frames, one more guess has to be calculated */

  /*-------------------------------------------- */
  /* helper pointer */
  QC_OUT_ELEMENT* qcElement[(1)][((8))];

  /* work on a copy of qcChannel and qcElement */
  for (i = 0; i < cm->nElements; i++) {
    ELEMENT_INFO elInfo = cm->elInfo[i];

    if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
        (elInfo.elType == ID_LFE)) {
      /* for ( all sub frames ) ... */
      for (c = 0; c < nSubFrames; c++) {
        { qcElement[c][i] = qcOut[c]->qcElement[i]; }
      }
    }
  }

  /*-------------------------------------------- */
  /*-------------------------------------------- */
  /* calc granted dynamic bits for sub frame and
     distribute it to each element */
  ErrorStatus = FDKaacEnc_prepareBitDistribution(
      hQC, psyOut, qcOut, cm, qcElement,
      (isCBRAdjustment == 0) ? hQC->maxBitsPerFrame : avgTotalBits,
      &totalAvailableBits, &avgTotalDynBits);

  if (ErrorStatus != AAC_ENC_OK) {
    return ErrorStatus;
  }

  /* for ( all sub frames ) ... */
  for (c = 0; c < nSubFrames; c++) {
    /* for CBR and VBR mode */
    FDKaacEnc_AdjustThresholds(hQC->hAdjThr, qcElement[c], qcOut[c],
                               psyOut[c]->psyOutElement, isCBRAdjustment, cm);

  } /* -end- sub frame counter */

  /*-------------------------------------------- */
  INT iterations[(1)][((8))];
  INT chConstraintsFulfilled[(1)][((8))][(2)];
  INT calculateQuant[(1)][((8))][(2)];
  INT constraintsFulfilled[(1)][((8))];
  /*-------------------------------------------- */

  /* for ( all sub frames ) ... */
  for (c = 0; c < nSubFrames; c++) {
    for (i = 0; i < cm->nElements; i++) {
      ELEMENT_INFO elInfo = cm->elInfo[i];
      INT ch, nChannels = elInfo.nChannelsInEl;

      if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
          (elInfo.elType == ID_LFE)) {
        /* Turn thresholds into scalefactors, optimize bit consumption and
         * verify conformance */
        FDKaacEnc_EstimateScaleFactors(
            psyOut[c]->psyOutElement[i]->psyOutChannel,
            qcElement[c][i]->qcOutChannel, hQC->invQuant, hQC->dZoneQuantEnable,
            cm->elInfo[i].nChannelsInEl);

        /*-------------------------------------------- */
        constraintsFulfilled[c][i] = 1;
        iterations[c][i] = 0;

        for (ch = 0; ch < nChannels; ch++) {
          chConstraintsFulfilled[c][i][ch] = 1;
          calculateQuant[c][i][ch] = 1;
        }

        /*-------------------------------------------- */

      } /*  -end- if(ID_SCE || ID_CPE || ID_LFE) */

    } /* -end- element loop */

    qcOut[c]->usedDynBits = -1;

  } /* -end- sub frame counter */

  INT quantizationDone = 0;
  INT sumDynBitsConsumedTotal = 0;
  INT decreaseBitConsumption = -1; /* no direction yet! */

  /*-------------------------------------------- */
  /* -start- Quantization loop ...               */
  /*-------------------------------------------- */
  do /* until max allowed bits per frame and maxDynBits!=-1*/
  {
    quantizationDone = 0;

    c = 0; /* get frame to process */

    for (i = 0; i < cm->nElements; i++) {
      ELEMENT_INFO elInfo = cm->elInfo[i];
      INT ch, nChannels = elInfo.nChannelsInEl;

      if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
          (elInfo.elType == ID_LFE)) {
        do /* until element bits < nChannels*MIN_BUFSIZE_PER_EFF_CHAN */
        {
          do /* until spectral values < MAX_QUANT */
          {
            /*-------------------------------------------- */
            if (!constraintsFulfilled[c][i]) {
              if ((ErrorStatus = FDKaacEnc_reduceBitConsumption(
                       &iterations[c][i], hQC->maxIterations,
                       (decreaseBitConsumption) ? 1 : -1,
                       chConstraintsFulfilled[c][i], calculateQuant[c][i],
                       nChannels, psyOut[c]->psyOutElement[i], qcOut[c],
                       qcElement[c][i], hQC->elementBits[i], aot, syntaxFlags,
                       epConfig)) != AAC_ENC_OK) {
                return ErrorStatus;
              }
            }

            /*-------------------------------------------- */
            /*-------------------------------------------- */
            constraintsFulfilled[c][i] = 1;

            /*-------------------------------------------- */
            /* quantize spectrum (per each channel) */
            for (ch = 0; ch < nChannels; ch++) {
              /*-------------------------------------------- */
              chConstraintsFulfilled[c][i][ch] = 1;

              /*-------------------------------------------- */

              if (calculateQuant[c][i][ch]) {
                QC_OUT_CHANNEL* qcOutCh = qcElement[c][i]->qcOutChannel[ch];
                PSY_OUT_CHANNEL* psyOutCh =
                    psyOut[c]->psyOutElement[i]->psyOutChannel[ch];

                calculateQuant[c][i][ch] =
                    0; /* calculate quantization only if necessary */

                /*-------------------------------------------- */
                FDKaacEnc_QuantizeSpectrum(
                    psyOutCh->sfbCnt, psyOutCh->maxSfbPerGroup,
                    psyOutCh->sfbPerGroup, psyOutCh->sfbOffsets,
                    qcOutCh->mdctSpectrum, qcOutCh->globalGain, qcOutCh->scf,
                    qcOutCh->quantSpec, hQC->dZoneQuantEnable);

                /*-------------------------------------------- */
                if (FDKaacEnc_calcMaxValueInSfb(
                        psyOutCh->sfbCnt, psyOutCh->maxSfbPerGroup,
                        psyOutCh->sfbPerGroup, psyOutCh->sfbOffsets,
                        qcOutCh->quantSpec,
                        qcOutCh->maxValueInSfb) > MAX_QUANT) {
                  chConstraintsFulfilled[c][i][ch] = 0;
                  constraintsFulfilled[c][i] = 0;
                  /* if quanizted value out of range; increase global gain! */
                  decreaseBitConsumption = 1;
                }

                /*-------------------------------------------- */

              } /* if calculateQuant[c][i][ch] */

            } /* channel loop */

            /*-------------------------------------------- */
            /* quantize spectrum (per each channel) */

            /*-------------------------------------------- */

          } while (!constraintsFulfilled[c][i]); /* does not regard bit
                                                    consumption */

          /*-------------------------------------------- */
          /*-------------------------------------------- */
          qcElement[c][i]->dynBitsUsed = 0; /* reset dynamic bits */

          /* quantization valid in current channel! */
          for (ch = 0; ch < nChannels; ch++) {
            QC_OUT_CHANNEL* qcOutCh = qcElement[c][i]->qcOutChannel[ch];
            PSY_OUT_CHANNEL* psyOutCh =
                psyOut[c]->psyOutElement[i]->psyOutChannel[ch];

            /* count dynamic bits */
            INT chDynBits = FDKaacEnc_dynBitCount(
                hQC->hBitCounter, qcOutCh->quantSpec, qcOutCh->maxValueInSfb,
                qcOutCh->scf, psyOutCh->lastWindowSequence, psyOutCh->sfbCnt,
                psyOutCh->maxSfbPerGroup, psyOutCh->sfbPerGroup,
                psyOutCh->sfbOffsets, &qcOutCh->sectionData, psyOutCh->noiseNrg,
                psyOutCh->isBook, psyOutCh->isScale, syntaxFlags);

            /* sum up dynamic channel bits */
            qcElement[c][i]->dynBitsUsed += chDynBits;
          }

          /* save dynBitsUsed for correction of bits2pe relation */
          if (hQC->hAdjThr->adjThrStateElem[i]->dynBitsLast == -1) {
            hQC->hAdjThr->adjThrStateElem[i]->dynBitsLast =
                qcElement[c][i]->dynBitsUsed;
          }

          /* hold total bit consumption in present element below maximum allowed
           */
          if (qcElement[c][i]->dynBitsUsed >
              ((nChannels * MIN_BUFSIZE_PER_EFF_CHAN) -
               qcElement[c][i]->staticBitsUsed -
               qcElement[c][i]->extBitsUsed)) {
            constraintsFulfilled[c][i] = 0;
          }

        } while (!constraintsFulfilled[c][i]);

      } /*  -end- if(ID_SCE || ID_CPE || ID_LFE) */

    } /* -end- element loop */

    /* update dynBits of current subFrame */
    FDKaacEnc_updateUsedDynBits(&qcOut[c]->usedDynBits, qcElement[c], cm);

    /* get total consumed bits, dyn bits in all sub frames have to be valid */
    sumDynBitsConsumedTotal =
        FDKaacEnc_getTotalConsumedDynBits(qcOut, nSubFrames);

    if (sumDynBitsConsumedTotal == -1) {
      quantizationDone = 0; /* bit consumption not valid in all sub frames */
    } else {
      int sumBitsConsumedTotal = FDKaacEnc_getTotalConsumedBits(
          qcOut, qcElement, cm, hQC->globHdrBits, nSubFrames);

      /* in all frames are valid dynamic bits */
      if (((sumBitsConsumedTotal < totalAvailableBits) ||
           sumDynBitsConsumedTotal == 0) &&
          (decreaseBitConsumption == 1) &&
          checkMinFrameBitsDemand(qcOut, hQC->minBitsPerFrame, nSubFrames)
          /*()*/) {
        quantizationDone = 1; /* exit bit adjustment */
      }
      if (sumBitsConsumedTotal > totalAvailableBits &&
          (decreaseBitConsumption == 0)) {
        quantizationDone = 0; /* reset! */
      }
    }

    /*-------------------------------------------- */

    int emergencyIterations = 1;
    int dynBitsOvershoot = 0;

    for (c = 0; c < nSubFrames; c++) {
      for (i = 0; i < cm->nElements; i++) {
        ELEMENT_INFO elInfo = cm->elInfo[i];

        if ((elInfo.elType == ID_SCE) || (elInfo.elType == ID_CPE) ||
            (elInfo.elType == ID_LFE)) {
          /* iteration limitation */
          emergencyIterations &=
              ((iterations[c][i] < hQC->maxIterations) ? 0 : 1);
        }
      }
      /* detection if used dyn bits exceeds the maximal allowed criterion */
      dynBitsOvershoot |=
          ((qcOut[c]->usedDynBits > qcOut[c]->maxDynBits) ? 1 : 0);
    }

    if (quantizationDone == 0 || dynBitsOvershoot) {
      int sumBitsConsumedTotal = FDKaacEnc_getTotalConsumedBits(
          qcOut, qcElement, cm, hQC->globHdrBits, nSubFrames);

      if ((sumDynBitsConsumedTotal >= avgTotalDynBits) ||
          (sumDynBitsConsumedTotal == 0)) {
        quantizationDone = 1;
      }
      if (emergencyIterations && (sumBitsConsumedTotal < totalAvailableBits)) {
        quantizationDone = 1;
      }
      if ((sumBitsConsumedTotal > totalAvailableBits) ||
          !checkMinFrameBitsDemand(qcOut, hQC->minBitsPerFrame, nSubFrames)) {
        quantizationDone = 0;
      }
      if ((sumBitsConsumedTotal < totalAvailableBits) &&
          checkMinFrameBitsDemand(qcOut, hQC->minBitsPerFrame, nSubFrames)) {
        decreaseBitConsumption = 0;
      } else {
        decreaseBitConsumption = 1;
      }

      if (dynBitsOvershoot) {
        quantizationDone = 0;
        decreaseBitConsumption = 1;
      }

      /* reset constraints fullfilled flags */
      FDKmemclear(constraintsFulfilled, sizeof(constraintsFulfilled));
      FDKmemclear(chConstraintsFulfilled, sizeof(chConstraintsFulfilled));

    } /* quantizationDone */

  } while (!quantizationDone);

  /*-------------------------------------------- */
  /* ... -end- Quantization loop                 */
  /*-------------------------------------------- */

  /*-------------------------------------------- */
  /*-------------------------------------------- */

  return AAC_ENC_OK;
}

static AAC_ENCODER_ERROR FDKaacEnc_reduceBitConsumption(
    int* iterations, const int maxIterations, int gainAdjustment,
    int* chConstraintsFulfilled, int* calculateQuant, int nChannels,
    PSY_OUT_ELEMENT* psyOutElement, QC_OUT* qcOut, QC_OUT_ELEMENT* qcOutElement,
    ELEMENT_BITS* elBits, AUDIO_OBJECT_TYPE aot, UINT syntaxFlags,
    SCHAR epConfig) {
  int ch;

  /** SOLVING PROBLEM **/
  if ((*iterations) < maxIterations) {
    /* increase gain (+ next iteration) */
    for (ch = 0; ch < nChannels; ch++) {
      if (!chConstraintsFulfilled[ch]) {
        qcOutElement->qcOutChannel[ch]->globalGain += gainAdjustment;
        calculateQuant[ch] = 1; /* global gain has changed, recalculate
                                   quantization in next iteration! */
      }
    }
  } else if ((*iterations) == maxIterations) {
    if (qcOutElement->dynBitsUsed == 0) {
      return AAC_ENC_QUANT_ERROR;
    } else {
      /* crash recovery */
      INT bitsToSave = 0;
      if ((bitsToSave = fixMax(
               (qcOutElement->dynBitsUsed + 8) -
                   (elBits->bitResLevelEl + qcOutElement->grantedDynBits),
               (qcOutElement->dynBitsUsed + qcOutElement->staticBitsUsed + 8) -
                   (elBits->maxBitsEl))) > 0) {
        FDKaacEnc_crashRecovery(nChannels, psyOutElement, qcOut, qcOutElement,
                                bitsToSave, aot, syntaxFlags, epConfig);
      } else {
        for (ch = 0; ch < nChannels; ch++) {
          qcOutElement->qcOutChannel[ch]->globalGain += 1;
        }
      }
      for (ch = 0; ch < nChannels; ch++) {
        calculateQuant[ch] = 1;
      }
    }
  } else {
    /* (*iterations) > maxIterations */
    return AAC_ENC_QUANT_ERROR;
  }
  (*iterations)++;

  return AAC_ENC_OK;
}

AAC_ENCODER_ERROR FDKaacEnc_updateFillBits(CHANNEL_MAPPING* cm,
                                           QC_STATE* qcKernel,
                                           ELEMENT_BITS* RESTRICT elBits[((8))],
                                           QC_OUT** qcOut) {
  switch (qcKernel->bitrateMode) {
    case QCDATA_BR_MODE_SFR:
      break;

    case QCDATA_BR_MODE_FF:
      break;
    case QCDATA_BR_MODE_VBR_1:
    case QCDATA_BR_MODE_VBR_2:
    case QCDATA_BR_MODE_VBR_3:
    case QCDATA_BR_MODE_VBR_4:
    case QCDATA_BR_MODE_VBR_5:
      qcOut[0]->totFillBits =
          (qcOut[0]->grantedDynBits - qcOut[0]->usedDynBits) &
          7; /* precalculate alignment bits */
      qcOut[0]->totalBits = qcOut[0]->staticBits + qcOut[0]->usedDynBits +
                            qcOut[0]->totFillBits + qcOut[0]->elementExtBits +
                            qcOut[0]->globalExtBits;
      qcOut[0]->totFillBits +=
          (fixMax(0, qcKernel->minBitsPerFrame - qcOut[0]->totalBits) + 7) & ~7;
      break;
    case QCDATA_BR_MODE_CBR:
    case QCDATA_BR_MODE_INVALID:
    default:
      INT bitResSpace = qcKernel->bitResTotMax - qcKernel->bitResTot;
      /* processing fill-bits */
      INT deltaBitRes = qcOut[0]->grantedDynBits - qcOut[0]->usedDynBits;
      qcOut[0]->totFillBits = fixMax(
          (deltaBitRes & 7), (deltaBitRes - (fixMax(0, bitResSpace - 7) & ~7)));
      qcOut[0]->totalBits = qcOut[0]->staticBits + qcOut[0]->usedDynBits +
                            qcOut[0]->totFillBits + qcOut[0]->elementExtBits +
                            qcOut[0]->globalExtBits;
      qcOut[0]->totFillBits +=
          (fixMax(0, qcKernel->minBitsPerFrame - qcOut[0]->totalBits) + 7) & ~7;
      break;
  } /* switch (qcKernel->bitrateMode) */

  return AAC_ENC_OK;
}

/*********************************************************************************

         functionname: FDKaacEnc_calcMaxValueInSfb
         description:
         return:

**********************************************************************************/

static INT FDKaacEnc_calcMaxValueInSfb(INT sfbCnt, INT maxSfbPerGroup,
                                       INT sfbPerGroup, INT* RESTRICT sfbOffset,
                                       SHORT* RESTRICT quantSpectrum,
                                       UINT* RESTRICT maxValue) {
  INT sfbOffs, sfb;
  INT maxValueAll = 0;

  for (sfbOffs = 0; sfbOffs < sfbCnt; sfbOffs += sfbPerGroup)
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      INT line;
      INT maxThisSfb = 0;
      for (line = sfbOffset[sfbOffs + sfb]; line < sfbOffset[sfbOffs + sfb + 1];
           line++) {
        INT tmp = fixp_abs(quantSpectrum[line]);
        maxThisSfb = fixMax(tmp, maxThisSfb);
      }

      maxValue[sfbOffs + sfb] = maxThisSfb;
      maxValueAll = fixMax(maxThisSfb, maxValueAll);
    }
  return maxValueAll;
}

/*********************************************************************************

         functionname: FDKaacEnc_updateBitres
         description:
         return:

**********************************************************************************/
void FDKaacEnc_updateBitres(CHANNEL_MAPPING* cm, QC_STATE* qcKernel,
                            QC_OUT** qcOut) {
  switch (qcKernel->bitrateMode) {
    case QCDATA_BR_MODE_VBR_1:
    case QCDATA_BR_MODE_VBR_2:
    case QCDATA_BR_MODE_VBR_3:
    case QCDATA_BR_MODE_VBR_4:
    case QCDATA_BR_MODE_VBR_5:
      /* variable bitrate */
      qcKernel->bitResTot =
          fMin(qcKernel->maxBitsPerFrame, qcKernel->bitResTotMax);
      break;
    case QCDATA_BR_MODE_CBR:
    case QCDATA_BR_MODE_SFR:
    case QCDATA_BR_MODE_INVALID:
    default:
      int c = 0;
      /* constant bitrate */
      {
        qcKernel->bitResTot += qcOut[c]->grantedDynBits -
                               (qcOut[c]->usedDynBits + qcOut[c]->totFillBits +
                                qcOut[c]->alignBits);
      }
      break;
  }
}

/*********************************************************************************

         functionname: FDKaacEnc_FinalizeBitConsumption
         description:
         return:

**********************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_FinalizeBitConsumption(
    CHANNEL_MAPPING* cm, QC_STATE* qcKernel, QC_OUT* qcOut,
    QC_OUT_ELEMENT** qcElement, HANDLE_TRANSPORTENC hTpEnc,
    AUDIO_OBJECT_TYPE aot, UINT syntaxFlags, SCHAR epConfig) {
  QC_OUT_EXTENSION fillExtPayload;
  INT totFillBits, alignBits;

  /* Get total consumed bits in AU */
  qcOut->totalBits = qcOut->staticBits + qcOut->usedDynBits +
                     qcOut->totFillBits + qcOut->elementExtBits +
                     qcOut->globalExtBits;

  if (qcKernel->bitrateMode == QCDATA_BR_MODE_CBR) {
    /* Now we can get the exact transport bit amount, and hopefully it is equal
     * to the estimated value */
    INT exactTpBits = transportEnc_GetStaticBits(hTpEnc, qcOut->totalBits);

    if (exactTpBits != qcKernel->globHdrBits) {
      INT diffFillBits = 0;

      /* How many bits can be take by bitreservoir */
      const INT bitresSpace =
          qcKernel->bitResTotMax -
          (qcKernel->bitResTot +
           (qcOut->grantedDynBits - (qcOut->usedDynBits + qcOut->totFillBits)));

      /* Number of bits which can be moved to bitreservoir. */
      const INT bitsToBitres = qcKernel->globHdrBits - exactTpBits;
      FDK_ASSERT(bitsToBitres >= 0); /* is always positive */

      /* If bitreservoir can not take all bits, move ramaining bits to fillbits
       */
      diffFillBits = fMax(0, bitsToBitres - bitresSpace);

      /* Assure previous alignment */
      diffFillBits = (diffFillBits + 7) & ~7;

      /* Move as many bits as possible to bitreservoir */
      qcKernel->bitResTot += (bitsToBitres - diffFillBits);

      /* Write remaing bits as fill bits */
      qcOut->totFillBits += diffFillBits;
      qcOut->totalBits += diffFillBits;
      qcOut->grantedDynBits += diffFillBits;

      /* Get new header bits */
      qcKernel->globHdrBits =
          transportEnc_GetStaticBits(hTpEnc, qcOut->totalBits);

      if (qcKernel->globHdrBits != exactTpBits) {
        /* In previous step, fill bits and corresponding total bits were changed
           when bitreservoir was completely filled. Now we can take the too much
           taken bits caused by header overhead from bitreservoir.
         */
        qcKernel->bitResTot -= (qcKernel->globHdrBits - exactTpBits);
      }
    }

  } /* MODE_CBR */

  /* Update exact number of consumed header bits. */
  qcKernel->globHdrBits = transportEnc_GetStaticBits(hTpEnc, qcOut->totalBits);

  /* Save total fill bits and distribut to alignment and fill bits */
  totFillBits = qcOut->totFillBits;

  /* fake a fill extension payload */
  FDKmemclear(&fillExtPayload, sizeof(QC_OUT_EXTENSION));

  fillExtPayload.type = EXT_FILL_DATA;
  fillExtPayload.nPayloadBits = totFillBits;

  /* ask bitstream encoder how many of that bits can be written in a fill
   * extension data entity */
  qcOut->totFillBits = FDKaacEnc_writeExtensionData(NULL, &fillExtPayload, 0, 0,
                                                    syntaxFlags, aot, epConfig);

  /* now distribute extra fillbits and alignbits */
  alignBits =
      7 - (qcOut->staticBits + qcOut->usedDynBits + qcOut->elementExtBits +
           qcOut->totFillBits + qcOut->globalExtBits - 1) %
              8;

  /* Maybe we could remove this */
  if (((alignBits + qcOut->totFillBits - totFillBits) == 8) &&
      (qcOut->totFillBits > 8))
    qcOut->totFillBits -= 8;

  qcOut->totalBits = qcOut->staticBits + qcOut->usedDynBits +
                     qcOut->totFillBits + alignBits + qcOut->elementExtBits +
                     qcOut->globalExtBits;

  if ((qcOut->totalBits > qcKernel->maxBitsPerFrame) ||
      (qcOut->totalBits < qcKernel->minBitsPerFrame)) {
    return AAC_ENC_QUANT_ERROR;
  }

  qcOut->alignBits = alignBits;

  return AAC_ENC_OK;
}

/*********************************************************************************

         functionname: FDKaacEnc_crashRecovery
         description:  fulfills constraints by means of brute force...
                       => bits are saved by cancelling out spectral lines!!
                          (beginning at the highest frequencies)
         return:       errorcode

**********************************************************************************/

static void FDKaacEnc_crashRecovery(INT nChannels,
                                    PSY_OUT_ELEMENT* psyOutElement,
                                    QC_OUT* qcOut, QC_OUT_ELEMENT* qcElement,
                                    INT bitsToSave, AUDIO_OBJECT_TYPE aot,
                                    UINT syntaxFlags, SCHAR epConfig) {
  INT ch;
  INT savedBits = 0;
  INT sfb, sfbGrp;
  INT bitsPerScf[(2)][MAX_GROUPED_SFB];
  INT sectionToScf[(2)][MAX_GROUPED_SFB];
  INT* sfbOffset;
  INT sect, statBitsNew;
  QC_OUT_CHANNEL** qcChannel = qcElement->qcOutChannel;
  PSY_OUT_CHANNEL** psyChannel = psyOutElement->psyOutChannel;

  /* create a table which converts frq-bins to bit-demand...    [bitsPerScf] */
  /* ...and another one which holds the corresponding sections [sectionToScf] */
  for (ch = 0; ch < nChannels; ch++) {
    sfbOffset = psyChannel[ch]->sfbOffsets;

    for (sect = 0; sect < qcChannel[ch]->sectionData.noOfSections; sect++) {
      INT codeBook = qcChannel[ch]->sectionData.huffsection[sect].codeBook;

      for (sfb = qcChannel[ch]->sectionData.huffsection[sect].sfbStart;
           sfb < qcChannel[ch]->sectionData.huffsection[sect].sfbStart +
                     qcChannel[ch]->sectionData.huffsection[sect].sfbCnt;
           sfb++) {
        bitsPerScf[ch][sfb] = 0;
        if ((codeBook != CODE_BOOK_PNS_NO) /*&&
             (sfb < (qcChannel[ch]->sectionData.noOfGroups*qcChannel[ch]->sectionData.maxSfbPerGroup))*/) {
          INT sfbStartLine = sfbOffset[sfb];
          INT noOfLines = sfbOffset[sfb + 1] - sfbStartLine;
          bitsPerScf[ch][sfb] = FDKaacEnc_countValues(
              &(qcChannel[ch]->quantSpec[sfbStartLine]), noOfLines, codeBook);
        }
        sectionToScf[ch][sfb] = sect;
      }
    }
  }

  /* LOWER [maxSfb] IN BOTH CHANNELS!! */
  /* Attention: in case of stereo: maxSfbL == maxSfbR, GroupingL == GroupingR ;
   */

  for (sfb = qcChannel[0]->sectionData.maxSfbPerGroup - 1; sfb >= 0; sfb--) {
    for (sfbGrp = 0; sfbGrp < psyChannel[0]->sfbCnt;
         sfbGrp += psyChannel[0]->sfbPerGroup) {
      for (ch = 0; ch < nChannels; ch++) {
        sect = sectionToScf[ch][sfbGrp + sfb];
        qcChannel[ch]->sectionData.huffsection[sect].sfbCnt--;
        savedBits += bitsPerScf[ch][sfbGrp + sfb];

        if (qcChannel[ch]->sectionData.huffsection[sect].sfbCnt == 0) {
          savedBits += (psyChannel[ch]->lastWindowSequence != SHORT_WINDOW)
                           ? FDKaacEnc_sideInfoTabLong[0]
                           : FDKaacEnc_sideInfoTabShort[0];
        }
      }
    }

    /* ...have enough bits been saved? */
    if (savedBits >= bitsToSave) break;

  } /* sfb loop */

  /* if not enough bits saved,
     clean whole spectrum and remove side info overhead */
  if (sfb == -1) {
    sfb = 0;
  }

  for (ch = 0; ch < nChannels; ch++) {
    qcChannel[ch]->sectionData.maxSfbPerGroup = sfb;
    psyChannel[ch]->maxSfbPerGroup = sfb;
    /* when no spectrum is coded save tools info in bitstream */
    if (sfb == 0) {
      FDKmemclear(&psyChannel[ch]->tnsInfo, sizeof(TNS_INFO));
      FDKmemclear(&psyOutElement->toolsInfo, sizeof(TOOLSINFO));
    }
  }
  /* dynamic bits will be updated in iteration loop */

  { /* if stop sfb has changed save bits in side info, e.g. MS or TNS coding */
    ELEMENT_INFO elInfo;

    FDKmemclear(&elInfo, sizeof(ELEMENT_INFO));
    elInfo.nChannelsInEl = nChannels;
    elInfo.elType = (nChannels == 2) ? ID_CPE : ID_SCE;

    FDKaacEnc_ChannelElementWrite(NULL, &elInfo, NULL, psyOutElement,
                                  psyChannel, syntaxFlags, aot, epConfig,
                                  &statBitsNew, 0);
  }

  savedBits = qcElement->staticBitsUsed - statBitsNew;

  /* update static and dynamic bits */
  qcElement->staticBitsUsed -= savedBits;
  qcElement->grantedDynBits += savedBits;

  qcOut->staticBits -= savedBits;
  qcOut->grantedDynBits += savedBits;
  qcOut->maxDynBits += savedBits;
}

void FDKaacEnc_QCClose(QC_STATE** phQCstate, QC_OUT** phQC) {
  int n, i;

  if (phQC != NULL) {
    for (n = 0; n < (1); n++) {
      if (phQC[n] != NULL) {
        QC_OUT* hQC = phQC[n];
        for (i = 0; i < (8); i++) {
        }

        for (i = 0; i < ((8)); i++) {
          if (hQC->qcElement[i]) FreeRam_aacEnc_QCelement(&hQC->qcElement[i]);
        }

        FreeRam_aacEnc_QCout(&phQC[n]);
      }
    }
  }

  if (phQCstate != NULL) {
    if (*phQCstate != NULL) {
      QC_STATE* hQCstate = *phQCstate;

      if (hQCstate->hAdjThr != NULL) FDKaacEnc_AdjThrClose(&hQCstate->hAdjThr);

      if (hQCstate->hBitCounter != NULL)
        FDKaacEnc_BCClose(&hQCstate->hBitCounter);

      for (i = 0; i < ((8)); i++) {
        if (hQCstate->elementBits[i] != NULL) {
          FreeRam_aacEnc_ElementBits(&hQCstate->elementBits[i]);
        }
      }
      FreeRam_aacEnc_QCstate(phQCstate);
    }
  }
}
