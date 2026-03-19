/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2021 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  SBR decoder frontend
  This module provides a frontend to the SBR decoder. The function openSBR() is
  called for initialization. The function sbrDecoder_Apply() is called for each
  frame. sbr_Apply() will call the required functions to decode the raw SBR data
  (provided by env_extr.cpp), to decode the envelope data and noise floor levels
  [decodeSbrData()], and to finally apply SBR to the current frame [sbr_dec()].

  \sa sbrDecoder_Apply(), \ref documentationOverview
*/

/*!
  \page documentationOverview Overview of important information resources and
  source code documentation

  As part of this documentation you can find more extensive descriptions about
  key concepts and algorithms at the following locations:

  <h2>Programming</h2>

  \li Buffer management: sbrDecoder_Apply() and sbr_dec()
  \li Internal scale factors to maximize SNR on fixed point processors:
  #QMF_SCALE_FACTOR \li Special mantissa-exponent format: Created in
  requantizeEnvelopeData() and used in calculateSbrEnvelope()

  <h2>Algorithmic details</h2>
  \li About the SBR data format: \ref SBR_HEADER_ELEMENT and \ref
  SBR_STANDARD_ELEMENT \li Details about the bitstream decoder: env_extr.cpp \li
  Details about the QMF filterbank and the provided polyphase implementation:
  qmf_dec.cpp \li Details about the transposer: lpp_tran.cpp \li Details about
  the envelope adjuster: env_calc.cpp

*/

#include "sbrdecoder.h"

#include "FDK_bitstream.h"

#include "sbrdec_freq_sca.h"
#include "env_extr.h"
#include "sbr_dec.h"
#include "env_dec.h"
#include "FDK_crc.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "lpp_tran.h"
#include "transcendent.h"

#include "sbrdec_drc.h"

#include "psbitdec.h"

/* Decoder library info */
#define SBRDECODER_LIB_VL0 3
#define SBRDECODER_LIB_VL1 1
#define SBRDECODER_LIB_VL2 0
#define SBRDECODER_LIB_TITLE "SBR Decoder"
#ifdef SUPPRESS_BUILD_DATE_INFO
#define SBRDECODER_LIB_BUILD_DATE ""
#define SBRDECODER_LIB_BUILD_TIME ""
#else
#define SBRDECODER_LIB_BUILD_DATE __DATE__
#define SBRDECODER_LIB_BUILD_TIME __TIME__
#endif

static void setFrameErrorFlag(SBR_DECODER_ELEMENT *pSbrElement, UCHAR value) {
  if (pSbrElement != NULL) {
    switch (value) {
      case FRAME_ERROR_ALLSLOTS:
        FDKmemset(pSbrElement->frameErrorFlag, FRAME_ERROR,
                  sizeof(pSbrElement->frameErrorFlag));
        break;
      default:
        pSbrElement->frameErrorFlag[pSbrElement->useFrameSlot] = value;
    }
  }
}

static UCHAR getHeaderSlot(UCHAR currentSlot, UCHAR hdrSlotUsage[(1) + 1]) {
  UINT occupied = 0;
  int s;
  UCHAR slot = hdrSlotUsage[currentSlot];

  FDK_ASSERT((1) + 1 < 32);

  for (s = 0; s < (1) + 1; s++) {
    if ((hdrSlotUsage[s] == slot) && (s != slot)) {
      occupied = 1;
      break;
    }
  }

  if (occupied) {
    occupied = 0;

    for (s = 0; s < (1) + 1; s++) {
      occupied |= 1 << hdrSlotUsage[s];
    }
    for (s = 0; s < (1) + 1; s++) {
      if (!(occupied & 0x1)) {
        slot = s;
        break;
      }
      occupied >>= 1;
    }
  }

  return slot;
}

static void copySbrHeader(HANDLE_SBR_HEADER_DATA hDst,
                          const HANDLE_SBR_HEADER_DATA hSrc) {
  /* copy the whole header memory (including pointers) */
  FDKmemcpy(hDst, hSrc, sizeof(SBR_HEADER_DATA));

  /* update pointers */
  hDst->freqBandData.freqBandTable[0] = hDst->freqBandData.freqBandTableLo;
  hDst->freqBandData.freqBandTable[1] = hDst->freqBandData.freqBandTableHi;
}

static int compareSbrHeader(const HANDLE_SBR_HEADER_DATA hHdr1,
                            const HANDLE_SBR_HEADER_DATA hHdr2) {
  int result = 0;

  /* compare basic data */
  result |= (hHdr1->syncState != hHdr2->syncState) ? 1 : 0;
  result |= (hHdr1->status != hHdr2->status) ? 1 : 0;
  result |= (hHdr1->frameErrorFlag != hHdr2->frameErrorFlag) ? 1 : 0;
  result |= (hHdr1->numberTimeSlots != hHdr2->numberTimeSlots) ? 1 : 0;
  result |=
      (hHdr1->numberOfAnalysisBands != hHdr2->numberOfAnalysisBands) ? 1 : 0;
  result |= (hHdr1->timeStep != hHdr2->timeStep) ? 1 : 0;
  result |= (hHdr1->sbrProcSmplRate != hHdr2->sbrProcSmplRate) ? 1 : 0;

  /* compare bitstream data */
  result |=
      FDKmemcmp(&hHdr1->bs_data, &hHdr2->bs_data, sizeof(SBR_HEADER_DATA_BS));
  result |=
      FDKmemcmp(&hHdr1->bs_dflt, &hHdr2->bs_dflt, sizeof(SBR_HEADER_DATA_BS));
  result |= FDKmemcmp(&hHdr1->bs_info, &hHdr2->bs_info,
                      sizeof(SBR_HEADER_DATA_BS_INFO));

  /* compare frequency band data */
  result |= FDKmemcmp(&hHdr1->freqBandData, &hHdr2->freqBandData,
                      (8 + MAX_NUM_LIMITERS + 1) * sizeof(UCHAR));
  result |= FDKmemcmp(hHdr1->freqBandData.freqBandTableLo,
                      hHdr2->freqBandData.freqBandTableLo,
                      (MAX_FREQ_COEFFS / 2 + 1) * sizeof(UCHAR));
  result |= FDKmemcmp(hHdr1->freqBandData.freqBandTableHi,
                      hHdr2->freqBandData.freqBandTableHi,
                      (MAX_FREQ_COEFFS + 1) * sizeof(UCHAR));
  result |= FDKmemcmp(hHdr1->freqBandData.freqBandTableNoise,
                      hHdr2->freqBandData.freqBandTableNoise,
                      (MAX_NOISE_COEFFS + 1) * sizeof(UCHAR));
  result |=
      FDKmemcmp(hHdr1->freqBandData.v_k_master, hHdr2->freqBandData.v_k_master,
                (MAX_FREQ_COEFFS + 1) * sizeof(UCHAR));

  return result;
}

/*!
  \brief Reset SBR decoder.

  Reset should only be called if SBR has been sucessfully detected by
  an appropriate checkForPayload() function.

  \return Error code.
*/
static SBR_ERROR sbrDecoder_ResetElement(HANDLE_SBRDECODER self,
                                         int sampleRateIn, int sampleRateOut,
                                         int samplesPerFrame,
                                         const MP4_ELEMENT_ID elementID,
                                         const int elementIndex,
                                         const int overlap) {
  SBR_ERROR sbrError = SBRDEC_OK;
  HANDLE_SBR_HEADER_DATA hSbrHeader;
  UINT qmfFlags = 0;

  int i, synDownsampleFac;

  /* USAC: assuming theoretical case 8 kHz output sample rate with 4:1 SBR */
  const int sbr_min_sample_rate_in = IS_USAC(self->coreCodec) ? 2000 : 6400;

  /* Check in/out samplerates */
  if (sampleRateIn < sbr_min_sample_rate_in || sampleRateIn > (96000)) {
    sbrError = SBRDEC_UNSUPPORTED_CONFIG;
    goto bail;
  }

  if (sampleRateOut > (96000)) {
    sbrError = SBRDEC_UNSUPPORTED_CONFIG;
    goto bail;
  }

  /* Set QMF mode flags */
  if (self->flags & SBRDEC_LOW_POWER) qmfFlags |= QMF_FLAG_LP;

  if (self->coreCodec == AOT_ER_AAC_ELD) {
    if (self->flags & SBRDEC_LD_MPS_QMF) {
      qmfFlags |= QMF_FLAG_MPSLDFB;
    } else {
      qmfFlags |= QMF_FLAG_CLDFB;
    }
  }

  /* Set downsampling factor for synthesis filter bank */
  if (sampleRateOut == 0) {
    /* no single rate mode */
    sampleRateOut =
        sampleRateIn
        << 1; /* In case of implicit signalling, assume dual rate SBR */
  }

  if (sampleRateIn == sampleRateOut) {
    synDownsampleFac = 2;
    self->flags |= SBRDEC_DOWNSAMPLE;
  } else {
    synDownsampleFac = 1;
    self->flags &= ~SBRDEC_DOWNSAMPLE;
  }

  self->synDownsampleFac = synDownsampleFac;
  self->sampleRateOut = sampleRateOut;

  {
    for (i = 0; i < (1) + 1; i++) {
      int setDflt;
      hSbrHeader = &(self->sbrHeader[elementIndex][i]);
      setDflt = ((hSbrHeader->syncState == SBR_NOT_INITIALIZED) ||
                 (self->flags & SBRDEC_FORCE_RESET))
                    ? 1
                    : 0;

      /* init a default header such that we can at least do upsampling later */
      sbrError = initHeaderData(hSbrHeader, sampleRateIn, sampleRateOut,
                                self->downscaleFactor, samplesPerFrame,
                                self->flags, setDflt);

      /* Set synchState to UPSAMPLING in case it already is initialized */
      hSbrHeader->syncState = hSbrHeader->syncState > UPSAMPLING
                                  ? UPSAMPLING
                                  : hSbrHeader->syncState;
    }
  }

  if (sbrError != SBRDEC_OK) {
    goto bail;
  }

  if (!self->pQmfDomain->globalConf.qmfDomainExplicitConfig) {
    self->pQmfDomain->globalConf.flags_requested |= qmfFlags;
    self->pQmfDomain->globalConf.nBandsAnalysis_requested =
        self->sbrHeader[elementIndex][0].numberOfAnalysisBands;
    self->pQmfDomain->globalConf.nBandsSynthesis_requested =
        (synDownsampleFac == 1) ? 64 : 32; /* may be overwritten by MPS */
    self->pQmfDomain->globalConf.nBandsSynthesis_requested /=
        self->downscaleFactor;
    self->pQmfDomain->globalConf.nQmfTimeSlots_requested =
        self->sbrHeader[elementIndex][0].numberTimeSlots *
        self->sbrHeader[elementIndex][0].timeStep;
    self->pQmfDomain->globalConf.nQmfOvTimeSlots_requested = overlap;
    self->pQmfDomain->globalConf.nQmfProcBands_requested = 64; /* always 64 */
    self->pQmfDomain->globalConf.nQmfProcChannels_requested =
        1; /* may be overwritten by MPS */
  }

  /* Init SBR channels going to be assigned to a SBR element */
  {
    int ch;
    for (ch = 0; ch < self->pSbrElement[elementIndex]->nChannels; ch++) {
      int headerIndex =
          getHeaderSlot(self->pSbrElement[elementIndex]->useFrameSlot,
                        self->pSbrElement[elementIndex]->useHeaderSlot);

      /* and create sbrDec */
      sbrError =
          createSbrDec(self->pSbrElement[elementIndex]->pSbrChannel[ch],
                       &self->sbrHeader[elementIndex][headerIndex],
                       &self->pSbrElement[elementIndex]->transposerSettings,
                       synDownsampleFac, qmfFlags, self->flags, overlap, ch,
                       self->codecFrameSize);

      if (sbrError != SBRDEC_OK) {
        goto bail;
      }
    }
  }

  // FDKmemclear(sbr_OverlapBuffer, sizeof(sbr_OverlapBuffer));

  if (self->numSbrElements == 1) {
    switch (self->coreCodec) {
      case AOT_AAC_LC:
      case AOT_SBR:
      case AOT_PS:
      case AOT_ER_AAC_SCAL:
      case AOT_DRM_AAC:
      case AOT_DRM_SURROUND:
        if (CreatePsDec(&self->hParametricStereoDec, samplesPerFrame)) {
          sbrError = SBRDEC_CREATE_ERROR;
          goto bail;
        }
        break;
      default:
        break;
    }
  }

  /* Init frame delay slot handling */
  self->pSbrElement[elementIndex]->useFrameSlot = 0;
  for (i = 0; i < ((1) + 1); i++) {
    self->pSbrElement[elementIndex]->useHeaderSlot[i] = i;
  }

bail:

  return sbrError;
}

/*!
  \brief Assign QMF domain provided QMF channels to SBR channels.

  \return void
*/
static void sbrDecoder_AssignQmfChannels2SbrChannels(HANDLE_SBRDECODER self) {
  int ch, el, absCh_offset = 0;
  for (el = 0; el < self->numSbrElements; el++) {
    if (self->pSbrElement[el] != NULL) {
      for (ch = 0; ch < self->pSbrElement[el]->nChannels; ch++) {
        FDK_ASSERT(((absCh_offset + ch) < ((8) + (1))) &&
                   ((absCh_offset + ch) < ((8) + (1))));
        self->pSbrElement[el]->pSbrChannel[ch]->SbrDec.qmfDomainInCh =
            &self->pQmfDomain->QmfDomainIn[absCh_offset + ch];
        self->pSbrElement[el]->pSbrChannel[ch]->SbrDec.qmfDomainOutCh =
            &self->pQmfDomain->QmfDomainOut[absCh_offset + ch];
      }
      absCh_offset += self->pSbrElement[el]->nChannels;
    }
  }
}

SBR_ERROR sbrDecoder_Open(HANDLE_SBRDECODER *pSelf,
                          HANDLE_FDK_QMF_DOMAIN pQmfDomain) {
  HANDLE_SBRDECODER self = NULL;
  SBR_ERROR sbrError = SBRDEC_OK;
  int elIdx;

  if ((pSelf == NULL) || (pQmfDomain == NULL)) {
    return SBRDEC_INVALID_ARGUMENT;
  }

  /* Get memory for this instance */
  self = GetRam_SbrDecoder();
  if (self == NULL) {
    sbrError = SBRDEC_MEM_ALLOC_FAILED;
    goto bail;
  }

  self->pQmfDomain = pQmfDomain;

  /*
  Already zero because of calloc
  self->numSbrElements = 0;
  self->numSbrChannels = 0;
  self->codecFrameSize = 0;
  */

  self->numDelayFrames = (1); /* set to the max value by default */

  /* Initialize header sync state */
  for (elIdx = 0; elIdx < (8); elIdx += 1) {
    int i;
    for (i = 0; i < (1) + 1; i += 1) {
      self->sbrHeader[elIdx][i].syncState = SBR_NOT_INITIALIZED;
    }
  }

  *pSelf = self;

bail:
  return sbrError;
}

/**
 * \brief determine if the given core codec AOT can be processed or not.
 * \param coreCodec core codec audio object type.
 * \return 1 if SBR can be processed, 0 if SBR cannot be processed/applied.
 */
static int sbrDecoder_isCoreCodecValid(AUDIO_OBJECT_TYPE coreCodec) {
  switch (coreCodec) {
    case AOT_AAC_LC:
    case AOT_SBR:
    case AOT_PS:
    case AOT_ER_AAC_SCAL:
    case AOT_ER_AAC_ELD:
    case AOT_DRM_AAC:
    case AOT_DRM_SURROUND:
    case AOT_USAC:
      return 1;
    default:
      return 0;
  }
}

static void sbrDecoder_DestroyElement(HANDLE_SBRDECODER self,
                                      const int elementIndex) {
  if (self->pSbrElement[elementIndex] != NULL) {
    int ch;

    for (ch = 0; ch < SBRDEC_MAX_CH_PER_ELEMENT; ch++) {
      if (self->pSbrElement[elementIndex]->pSbrChannel[ch] != NULL) {
        deleteSbrDec(self->pSbrElement[elementIndex]->pSbrChannel[ch]);
        FreeRam_SbrDecChannel(
            &self->pSbrElement[elementIndex]->pSbrChannel[ch]);
        self->numSbrChannels -= 1;
      }
    }
    FreeRam_SbrDecElement(&self->pSbrElement[elementIndex]);
    self->numSbrElements -= 1;
  }
}

SBR_ERROR sbrDecoder_InitElement(
    HANDLE_SBRDECODER self, const int sampleRateIn, const int sampleRateOut,
    const int samplesPerFrame, const AUDIO_OBJECT_TYPE coreCodec,
    const MP4_ELEMENT_ID elementID, const int elementIndex,
    const UCHAR harmonicSBR, const UCHAR stereoConfigIndex,
    const UCHAR configMode, UCHAR *configChanged, const INT downscaleFactor) {
  SBR_ERROR sbrError = SBRDEC_OK;
  int chCnt = 0;
  int nSbrElementsStart;
  int nSbrChannelsStart;
  if (self == NULL) {
    return SBRDEC_INVALID_ARGUMENT;
  }

  nSbrElementsStart = self->numSbrElements;
  nSbrChannelsStart = self->numSbrChannels;

  /* Check core codec AOT */
  if (!sbrDecoder_isCoreCodecValid(coreCodec) || elementIndex >= (8)) {
    sbrError = SBRDEC_UNSUPPORTED_CONFIG;
    goto bail;
  }

  if (elementID != ID_SCE && elementID != ID_CPE && elementID != ID_LFE) {
    sbrError = SBRDEC_UNSUPPORTED_CONFIG;
    goto bail;
  }

  if (self->sampleRateIn == sampleRateIn &&
      self->codecFrameSize == samplesPerFrame && self->coreCodec == coreCodec &&
      self->pSbrElement[elementIndex] != NULL &&
      self->pSbrElement[elementIndex]->elementID == elementID &&
      !(self->flags & SBRDEC_FORCE_RESET) &&
      ((sampleRateOut == 0) ? 1 : (self->sampleRateOut == sampleRateOut)) &&
      ((harmonicSBR == 2) ? 1
                          : (self->harmonicSBR ==
                             harmonicSBR)) /* The value 2 signalizes that
                                              harmonicSBR shall be ignored in
                                              the config change detection */
  ) {
    /* Nothing to do */
    return SBRDEC_OK;
  } else {
    if (configMode & AC_CM_DET_CFG_CHANGE) {
      *configChanged = 1;
    }
  }

  /* reaching this point the SBR-decoder gets (re-)configured */

  /* The flags field is used for all elements! */
  self->flags &=
      (SBRDEC_FORCE_RESET | SBRDEC_FLUSH); /* Keep the global flags. They will
                                              be reset after decoding. */
  self->flags |= (downscaleFactor > 1) ? SBRDEC_ELD_DOWNSCALE : 0;
  self->flags |= (coreCodec == AOT_ER_AAC_ELD) ? SBRDEC_ELD_GRID : 0;
  self->flags |= (coreCodec == AOT_ER_AAC_SCAL) ? SBRDEC_SYNTAX_SCAL : 0;
  self->flags |=
      (coreCodec == AOT_DRM_AAC) ? SBRDEC_SYNTAX_SCAL | SBRDEC_SYNTAX_DRM : 0;
  self->flags |= (coreCodec == AOT_DRM_SURROUND)
                     ? SBRDEC_SYNTAX_SCAL | SBRDEC_SYNTAX_DRM
                     : 0;
  self->flags |= (coreCodec == AOT_USAC) ? SBRDEC_SYNTAX_USAC : 0;
  /* Robustness: Take integer division rounding into consideration. E.g. 22050
   * Hz with 4:1 SBR => 5512 Hz core sampling rate. */
  self->flags |= (sampleRateIn == sampleRateOut / 4) ? SBRDEC_QUAD_RATE : 0;
  self->flags |= (harmonicSBR == 1) ? SBRDEC_USAC_HARMONICSBR : 0;

  if (configMode & AC_CM_DET_CFG_CHANGE) {
    return SBRDEC_OK;
  }

  self->sampleRateIn = sampleRateIn;
  self->codecFrameSize = samplesPerFrame;
  self->coreCodec = coreCodec;
  self->harmonicSBR = harmonicSBR;
  self->downscaleFactor = downscaleFactor;

  /* Init SBR elements */
  {
    int elChannels, ch;

    if (self->pSbrElement[elementIndex] == NULL) {
      self->pSbrElement[elementIndex] = GetRam_SbrDecElement(elementIndex);
      if (self->pSbrElement[elementIndex] == NULL) {
        sbrError = SBRDEC_MEM_ALLOC_FAILED;
        goto bail;
      }
      self->numSbrElements++;
    } else {
      self->numSbrChannels -= self->pSbrElement[elementIndex]->nChannels;
    }

    /* Determine amount of channels for this element */
    switch (elementID) {
      case ID_NONE:
      case ID_CPE:
        elChannels = 2;
        break;
      case ID_LFE:
      case ID_SCE:
        elChannels = 1;
        break;
      default:
        elChannels = 0;
        break;
    }

    /* Handle case of Parametric Stereo */
    if (elementIndex == 0 && elementID == ID_SCE) {
      switch (coreCodec) {
        case AOT_AAC_LC:
        case AOT_SBR:
        case AOT_PS:
        case AOT_ER_AAC_SCAL:
        case AOT_DRM_AAC:
        case AOT_DRM_SURROUND:
          elChannels = 2;
          break;
        default:
          break;
      }
    }

    /* Sanity check to avoid memory leaks */
    if (elChannels < self->pSbrElement[elementIndex]->nChannels ||
        (self->numSbrChannels + elChannels) > (8) + (1)) {
      self->numSbrChannels += self->pSbrElement[elementIndex]->nChannels;
      sbrError = SBRDEC_PARSE_ERROR;
      goto bail;
    }

    /* Save element ID for sanity checks and to have a fallback for concealment.
     */
    self->pSbrElement[elementIndex]->elementID = elementID;
    self->pSbrElement[elementIndex]->nChannels = elChannels;

    for (ch = 0; ch < elChannels; ch++) {
      if (self->pSbrElement[elementIndex]->pSbrChannel[ch] == NULL) {
        self->pSbrElement[elementIndex]->pSbrChannel[ch] =
            GetRam_SbrDecChannel(chCnt);
        if (self->pSbrElement[elementIndex]->pSbrChannel[ch] == NULL) {
          sbrError = SBRDEC_MEM_ALLOC_FAILED;
          goto bail;
        }
      }
      self->numSbrChannels++;

      sbrDecoder_drcInitChannel(&self->pSbrElement[elementIndex]
                                     ->pSbrChannel[ch]
                                     ->SbrDec.sbrDrcChannel);

      chCnt++;
    }
  }

  if (!self->pQmfDomain->globalConf.qmfDomainExplicitConfig) {
    self->pQmfDomain->globalConf.nInputChannels_requested =
        self->numSbrChannels;
    self->pQmfDomain->globalConf.nOutputChannels_requested =
        fMax((INT)self->numSbrChannels,
             (INT)self->pQmfDomain->globalConf.nOutputChannels_requested);
  }

  /* Make sure each SBR channel has one QMF channel assigned even if
   * numSbrChannels or element set-up has changed. */
  sbrDecoder_AssignQmfChannels2SbrChannels(self);

  /* clear error flags for all delay slots */
  FDKmemclear(self->pSbrElement[elementIndex]->frameErrorFlag,
              ((1) + 1) * sizeof(UCHAR));

  {
    int overlap;

    if (coreCodec == AOT_ER_AAC_ELD) {
      overlap = 0;
    } else if (self->flags & SBRDEC_QUAD_RATE) {
      overlap = (3 * 4);
    } else {
      overlap = (3 * 2);
    }
    /* Initialize this instance */
    sbrError = sbrDecoder_ResetElement(self, sampleRateIn, sampleRateOut,
                                       samplesPerFrame, elementID, elementIndex,
                                       overlap);
  }

bail:
  if (sbrError != SBRDEC_OK) {
    if ((nSbrElementsStart < self->numSbrElements) ||
        (nSbrChannelsStart < self->numSbrChannels)) {
      /* Free the memory allocated for this element */
      sbrDecoder_DestroyElement(self, elementIndex);
    } else if ((elementIndex < (8)) &&
               (self->pSbrElement[elementIndex] !=
                NULL)) { /* Set error flag to trigger concealment */
      setFrameErrorFlag(self->pSbrElement[elementIndex], FRAME_ERROR);
    }
  }

  return sbrError;
}

/**
 * \brief Free config dependent SBR memory.
 * \param self SBR decoder instance handle
 */
SBR_ERROR sbrDecoder_FreeMem(HANDLE_SBRDECODER *self) {
  int i;
  int elIdx;

  if (self != NULL && *self != NULL) {
    for (i = 0; i < (8); i++) {
      sbrDecoder_DestroyElement(*self, i);
    }

    for (elIdx = 0; elIdx < (8); elIdx += 1) {
      for (i = 0; i < (1) + 1; i += 1) {
        (*self)->sbrHeader[elIdx][i].syncState = SBR_NOT_INITIALIZED;
      }
    }
  }

  return SBRDEC_OK;
}

/**
 * \brief Apply decoded SBR header for one element.
 * \param self SBR decoder instance handle
 * \param hSbrHeader SBR header handle to be processed.
 * \param hSbrChannel pointer array to the SBR element channels corresponding to
 * the SBR header.
 * \param headerStatus header status value returned from SBR header parser.
 * \param numElementChannels amount of channels for the SBR element whos header
 * is to be processed.
 */
static SBR_ERROR sbrDecoder_HeaderUpdate(HANDLE_SBRDECODER self,
                                         HANDLE_SBR_HEADER_DATA hSbrHeader,
                                         SBR_HEADER_STATUS headerStatus,
                                         HANDLE_SBR_CHANNEL hSbrChannel[],
                                         const int numElementChannels) {
  SBR_ERROR errorStatus = SBRDEC_OK;

  /*
    change of control data, reset decoder
  */
  errorStatus = resetFreqBandTables(hSbrHeader, self->flags);

  if (errorStatus == SBRDEC_OK) {
    if (hSbrHeader->syncState == UPSAMPLING && headerStatus != HEADER_RESET) {
#if (SBRDEC_MAX_HB_FADE_FRAMES > 0)
      int ch;
      for (ch = 0; ch < numElementChannels; ch += 1) {
        hSbrChannel[ch]->SbrDec.highBandFadeCnt = SBRDEC_MAX_HB_FADE_FRAMES;
      }

#endif
      /* As the default header would limit the frequency range,
         lowSubband and highSubband must be patched. */
      hSbrHeader->freqBandData.lowSubband = hSbrHeader->numberOfAnalysisBands;
      hSbrHeader->freqBandData.highSubband = hSbrHeader->numberOfAnalysisBands;
    }

    /* Trigger a reset before processing this slot */
    hSbrHeader->status |= SBRDEC_HDR_STAT_RESET;
  }

  return errorStatus;
}

INT sbrDecoder_Header(HANDLE_SBRDECODER self, HANDLE_FDK_BITSTREAM hBs,
                      const INT sampleRateIn, const INT sampleRateOut,
                      const INT samplesPerFrame,
                      const AUDIO_OBJECT_TYPE coreCodec,
                      const MP4_ELEMENT_ID elementID, const INT elementIndex,
                      const UCHAR harmonicSBR, const UCHAR stereoConfigIndex,
                      const UCHAR configMode, UCHAR *configChanged,
                      const INT downscaleFactor) {
  SBR_HEADER_STATUS headerStatus;
  HANDLE_SBR_HEADER_DATA hSbrHeader;
  SBR_ERROR sbrError = SBRDEC_OK;
  int headerIndex;
  UINT flagsSaved =
      0; /* flags should not be changed in AC_CM_DET_CFG_CHANGE - mode after
            parsing */

  if (self == NULL || elementIndex >= (8)) {
    return SBRDEC_UNSUPPORTED_CONFIG;
  }

  if (!sbrDecoder_isCoreCodecValid(coreCodec)) {
    return SBRDEC_UNSUPPORTED_CONFIG;
  }

  if (configMode & AC_CM_DET_CFG_CHANGE) {
    flagsSaved = self->flags; /* store */
  }

  sbrError = sbrDecoder_InitElement(
      self, sampleRateIn, sampleRateOut, samplesPerFrame, coreCodec, elementID,
      elementIndex, harmonicSBR, stereoConfigIndex, configMode, configChanged,
      downscaleFactor);

  if ((sbrError != SBRDEC_OK) || (elementID == ID_LFE)) {
    goto bail;
  }

  if (configMode & AC_CM_DET_CFG_CHANGE) {
    hSbrHeader = NULL;
  } else {
    headerIndex = getHeaderSlot(self->pSbrElement[elementIndex]->useFrameSlot,
                                self->pSbrElement[elementIndex]->useHeaderSlot);

    hSbrHeader = &(self->sbrHeader[elementIndex][headerIndex]);
  }

  headerStatus = sbrGetHeaderData(hSbrHeader, hBs, self->flags, 0, configMode);

  if (coreCodec == AOT_USAC) {
    if (configMode & AC_CM_DET_CFG_CHANGE) {
      self->flags = flagsSaved; /* restore */
    }
    return sbrError;
  }

  if (configMode & AC_CM_ALLOC_MEM) {
    SBR_DECODER_ELEMENT *pSbrElement;

    pSbrElement = self->pSbrElement[elementIndex];

    /* Sanity check */
    if (pSbrElement != NULL) {
      if ((elementID == ID_CPE && pSbrElement->nChannels != 2) ||
          (elementID != ID_CPE && pSbrElement->nChannels != 1)) {
        return SBRDEC_UNSUPPORTED_CONFIG;
      }
      if (headerStatus == HEADER_RESET) {
        sbrError = sbrDecoder_HeaderUpdate(self, hSbrHeader, headerStatus,
                                           pSbrElement->pSbrChannel,
                                           pSbrElement->nChannels);

        if (sbrError == SBRDEC_OK) {
          hSbrHeader->syncState = SBR_HEADER;
          hSbrHeader->status |= SBRDEC_HDR_STAT_UPDATE;
        } else {
          hSbrHeader->syncState = SBR_NOT_INITIALIZED;
          hSbrHeader->status = HEADER_ERROR;
        }
      }
    }
  }
bail:
  if (configMode & AC_CM_DET_CFG_CHANGE) {
    self->flags = flagsSaved; /* restore */
  }
  return sbrError;
}

SBR_ERROR sbrDecoder_SetParam(HANDLE_SBRDECODER self, const SBRDEC_PARAM param,
                              const INT value) {
  SBR_ERROR errorStatus = SBRDEC_OK;

  /* configure the subsystems */
  switch (param) {
    case SBR_SYSTEM_BITSTREAM_DELAY:
      if (value < 0 || value > (1)) {
        errorStatus = SBRDEC_SET_PARAM_FAIL;
        break;
      }
      if (self == NULL) {
        errorStatus = SBRDEC_NOT_INITIALIZED;
      } else {
        self->numDelayFrames = (UCHAR)value;
      }
      break;
    case SBR_QMF_MODE:
      if (self == NULL) {
        errorStatus = SBRDEC_NOT_INITIALIZED;
      } else {
        if (value == 1) {
          self->flags |= SBRDEC_LOW_POWER;
        } else {
          self->flags &= ~SBRDEC_LOW_POWER;
        }
      }
      break;
    case SBR_LD_QMF_TIME_ALIGN:
      if (self == NULL) {
        errorStatus = SBRDEC_NOT_INITIALIZED;
      } else {
        if (value == 1) {
          self->flags |= SBRDEC_LD_MPS_QMF;
        } else {
          self->flags &= ~SBRDEC_LD_MPS_QMF;
        }
      }
      break;
    case SBR_FLUSH_DATA:
      if (value != 0) {
        if (self == NULL) {
          errorStatus = SBRDEC_NOT_INITIALIZED;
        } else {
          self->flags |= SBRDEC_FLUSH;
        }
      }
      break;
    case SBR_CLEAR_HISTORY:
      if (value != 0) {
        if (self == NULL) {
          errorStatus = SBRDEC_NOT_INITIALIZED;
        } else {
          self->flags |= SBRDEC_FORCE_RESET;
        }
      }
      break;
    case SBR_BS_INTERRUPTION: {
      int elementIndex;

      if (self == NULL) {
        errorStatus = SBRDEC_NOT_INITIALIZED;
        break;
      }

      /* Loop over SBR elements */
      for (elementIndex = 0; elementIndex < self->numSbrElements;
           elementIndex++) {
        if (self->pSbrElement[elementIndex] != NULL) {
          HANDLE_SBR_HEADER_DATA hSbrHeader;
          int headerIndex =
              getHeaderSlot(self->pSbrElement[elementIndex]->useFrameSlot,
                            self->pSbrElement[elementIndex]->useHeaderSlot);

          hSbrHeader = &(self->sbrHeader[elementIndex][headerIndex]);

          /* Set sync state UPSAMPLING for the corresponding slot.
             This switches off bitstream parsing until a new header arrives. */
          if (hSbrHeader->syncState != SBR_NOT_INITIALIZED) {
            hSbrHeader->syncState = UPSAMPLING;
            hSbrHeader->status |= SBRDEC_HDR_STAT_UPDATE;
          }
        }
      }
    } break;

    case SBR_SKIP_QMF:
      if (self == NULL) {
        errorStatus = SBRDEC_NOT_INITIALIZED;
      } else {
        if (value == 1) {
          self->flags |= SBRDEC_SKIP_QMF_ANA;
        } else {
          self->flags &= ~SBRDEC_SKIP_QMF_ANA;
        }
        if (value == 2) {
          self->flags |= SBRDEC_SKIP_QMF_SYN;
        } else {
          self->flags &= ~SBRDEC_SKIP_QMF_SYN;
        }
      }
      break;
    default:
      errorStatus = SBRDEC_SET_PARAM_FAIL;
      break;
  } /* switch(param) */

  return (errorStatus);
}

static SBRDEC_DRC_CHANNEL *sbrDecoder_drcGetChannel(
    const HANDLE_SBRDECODER self, const INT channel) {
  SBRDEC_DRC_CHANNEL *pSbrDrcChannelData = NULL;
  int elementIndex, elChanIdx = 0, numCh = 0;

  for (elementIndex = 0; (elementIndex < (8)) && (numCh <= channel);
       elementIndex++) {
    SBR_DECODER_ELEMENT *pSbrElement = self->pSbrElement[elementIndex];
    int c, elChannels;

    elChanIdx = 0;
    if (pSbrElement == NULL) break;

    /* Determine amount of channels for this element */
    switch (pSbrElement->elementID) {
      case ID_CPE:
        elChannels = 2;
        break;
      case ID_LFE:
      case ID_SCE:
        elChannels = 1;
        break;
      case ID_NONE:
      default:
        elChannels = 0;
        break;
    }

    /* Limit with actual allocated element channels */
    elChannels = fMin(elChannels, pSbrElement->nChannels);

    for (c = 0; (c < elChannels) && (numCh <= channel); c++) {
      if (pSbrElement->pSbrChannel[elChanIdx] != NULL) {
        numCh++;
        elChanIdx++;
      }
    }
  }
  elementIndex -= 1;
  elChanIdx -= 1;

  if (elChanIdx < 0 || elementIndex < 0) {
    return NULL;
  }

  if (self->pSbrElement[elementIndex] != NULL) {
    if (self->pSbrElement[elementIndex]->pSbrChannel[elChanIdx] != NULL) {
      pSbrDrcChannelData = &self->pSbrElement[elementIndex]
                                ->pSbrChannel[elChanIdx]
                                ->SbrDec.sbrDrcChannel;
    }
  }

  return (pSbrDrcChannelData);
}

SBR_ERROR sbrDecoder_drcFeedChannel(HANDLE_SBRDECODER self, INT ch,
                                    UINT numBands, FIXP_DBL *pNextFact_mag,
                                    INT nextFact_exp,
                                    SHORT drcInterpolationScheme,
                                    UCHAR winSequence, USHORT *pBandTop) {
  SBRDEC_DRC_CHANNEL *pSbrDrcChannelData = NULL;
  int band, isValidData = 0;

  if (self == NULL) {
    return SBRDEC_NOT_INITIALIZED;
  }
  if (ch > (8) || pNextFact_mag == NULL) {
    return SBRDEC_SET_PARAM_FAIL;
  }

  /* Search for gain values different to 1.0f */
  for (band = 0; band < (int)numBands; band += 1) {
    if (!((pNextFact_mag[band] == FL2FXCONST_DBL(0.5)) &&
          (nextFact_exp == 1)) &&
        !((pNextFact_mag[band] == (FIXP_DBL)MAXVAL_DBL) &&
          (nextFact_exp == 0))) {
      isValidData = 1;
      break;
    }
  }

  /* Find the right SBR channel */
  pSbrDrcChannelData = sbrDecoder_drcGetChannel(self, ch);

  if (pSbrDrcChannelData != NULL) {
    if (pSbrDrcChannelData->enable ||
        isValidData) { /* Activate processing only with real and valid data */
      int i;

      pSbrDrcChannelData->enable = 1;
      pSbrDrcChannelData->numBandsNext = numBands;

      pSbrDrcChannelData->winSequenceNext = winSequence;
      pSbrDrcChannelData->drcInterpolationSchemeNext = drcInterpolationScheme;
      pSbrDrcChannelData->nextFact_exp = nextFact_exp;

      for (i = 0; i < (int)numBands; i++) {
        pSbrDrcChannelData->bandTopNext[i] = pBandTop[i];
        pSbrDrcChannelData->nextFact_mag[i] = pNextFact_mag[i];
      }
    }
  }

  return SBRDEC_OK;
}

void sbrDecoder_drcDisable(HANDLE_SBRDECODER self, INT ch) {
  SBRDEC_DRC_CHANNEL *pSbrDrcChannelData = NULL;

  if ((self == NULL) || (ch > (8)) || (self->numSbrElements == 0) ||
      (self->numSbrChannels == 0)) {
    return;
  }

  /* Find the right SBR channel */
  pSbrDrcChannelData = sbrDecoder_drcGetChannel(self, ch);

  if (pSbrDrcChannelData != NULL) {
    sbrDecoder_drcInitChannel(pSbrDrcChannelData);
  }
}

SBR_ERROR sbrDecoder_Parse(HANDLE_SBRDECODER self, HANDLE_FDK_BITSTREAM hBs,
                           UCHAR *pDrmBsBuffer, USHORT drmBsBufferSize,
                           int *count, int bsPayLen, int crcFlag,
                           MP4_ELEMENT_ID prevElement, int elementIndex,
                           UINT acFlags, UINT acElFlags[]) {
  SBR_DECODER_ELEMENT *hSbrElement = NULL;
  HANDLE_SBR_HEADER_DATA hSbrHeader = NULL;
  HANDLE_SBR_CHANNEL *pSbrChannel;

  SBR_FRAME_DATA *hFrameDataLeft = NULL;
  SBR_FRAME_DATA *hFrameDataRight = NULL;
  SBR_FRAME_DATA frameDataLeftCopy;
  SBR_FRAME_DATA frameDataRightCopy;

  SBR_ERROR errorStatus = SBRDEC_OK;
  SBR_HEADER_STATUS headerStatus = HEADER_NOT_PRESENT;

  INT startPos = FDKgetValidBits(hBs);
  FDK_CRCINFO crcInfo;
  INT crcReg = 0;
  USHORT sbrCrc = 0;
  UINT crcPoly;
  UINT crcStartValue = 0;
  UINT crcLen;

  HANDLE_FDK_BITSTREAM hBsOriginal = hBs;
  FDK_BITSTREAM bsBwd;

  const int fGlobalIndependencyFlag = acFlags & AC_INDEP;
  const int bs_pvc = acElFlags[elementIndex] & AC_EL_USAC_PVC;
  const int bs_interTes = acElFlags[elementIndex] & AC_EL_USAC_ITES;
  int stereo;
  int fDoDecodeSbrData = 1;
  int alignBits = 0;

  int lastSlot, lastHdrSlot = 0, thisHdrSlot = 0;

  if (*count <= 0) {
    setFrameErrorFlag(self->pSbrElement[elementIndex], FRAME_ERROR);
    return SBRDEC_OK;
  }

  /* SBR sanity checks */
  if (self == NULL) {
    errorStatus = SBRDEC_NOT_INITIALIZED;
    goto bail;
  }

  /* Reverse bits of DRM SBR payload */
  if ((self->flags & SBRDEC_SYNTAX_DRM) && *count > 0) {
    int dataBytes, dataBits;

    FDK_ASSERT(drmBsBufferSize >= (512));
    dataBits = *count;

    if (dataBits > ((512) * 8)) {
      /* do not flip more data than needed */
      dataBits = (512) * 8;
    }

    dataBytes = (dataBits + 7) >> 3;

    int j;

    if ((j = (int)FDKgetValidBits(hBs)) != 8) {
      FDKpushBiDirectional(hBs, (j - 8));
    }

    j = 0;
    for (; dataBytes > 0; dataBytes--) {
      int i;
      UCHAR tmpByte;
      UCHAR buffer = 0x00;

      tmpByte = (UCHAR)FDKreadBits(hBs, 8);
      for (i = 0; i < 4; i++) {
        int shift = 2 * i + 1;
        buffer |= (tmpByte & (0x08 >> i)) << shift;
        buffer |= (tmpByte & (0x10 << i)) >> shift;
      }
      pDrmBsBuffer[j++] = buffer;
      FDKpushBack(hBs, 16);
    }

    FDKinitBitStream(&bsBwd, pDrmBsBuffer, (512), dataBits, BS_READER);

    /* Use reversed data */
    hBs = &bsBwd;
    bsPayLen = *count;
  }

  /* Remember start position of  SBR element */
  startPos = FDKgetValidBits(hBs);

  /* SBR sanity checks */
  if (self->pSbrElement[elementIndex] == NULL) {
    errorStatus = SBRDEC_NOT_INITIALIZED;
    goto bail;
  }
  hSbrElement = self->pSbrElement[elementIndex];

  lastSlot = (hSbrElement->useFrameSlot > 0) ? hSbrElement->useFrameSlot - 1
                                             : self->numDelayFrames;
  lastHdrSlot = hSbrElement->useHeaderSlot[lastSlot];
  thisHdrSlot = getHeaderSlot(
      hSbrElement->useFrameSlot,
      hSbrElement->useHeaderSlot); /* Get a free header slot not used by
                                      frames not processed yet. */

  /* Assign the free slot to store a new header if there is one. */
  hSbrHeader = &self->sbrHeader[elementIndex][thisHdrSlot];

  pSbrChannel = hSbrElement->pSbrChannel;
  stereo = (hSbrElement->elementID == ID_CPE) ? 1 : 0;

  hFrameDataLeft = &self->pSbrElement[elementIndex]
                        ->pSbrChannel[0]
                        ->frameData[hSbrElement->useFrameSlot];
  if (stereo) {
    hFrameDataRight = &self->pSbrElement[elementIndex]
                           ->pSbrChannel[1]
                           ->frameData[hSbrElement->useFrameSlot];
  }

  /* store frameData; new parsed frameData possibly corrupted */
  FDKmemcpy(&frameDataLeftCopy, hFrameDataLeft, sizeof(SBR_FRAME_DATA));
  if (stereo) {
    FDKmemcpy(&frameDataRightCopy, hFrameDataRight, sizeof(SBR_FRAME_DATA));
  }

  /* reset PS flag; will be set after PS was found */
  self->flags &= ~SBRDEC_PS_DECODED;

  if (hSbrHeader->status & SBRDEC_HDR_STAT_UPDATE) {
    /* Got a new header from extern (e.g. from an ASC) */
    headerStatus = HEADER_OK;
    hSbrHeader->status &= ~SBRDEC_HDR_STAT_UPDATE;
  } else if (thisHdrSlot != lastHdrSlot) {
    /* Copy the last header into this slot otherwise the
       header compare will trigger more HEADER_RESETs than needed. */
    copySbrHeader(hSbrHeader, &self->sbrHeader[elementIndex][lastHdrSlot]);
  }

  /*
     Check if bit stream data is valid and matches the element context
  */
  if (((prevElement != ID_SCE) && (prevElement != ID_CPE)) ||
      prevElement != hSbrElement->elementID) {
    /* In case of LFE we also land here, since there is no LFE SBR element (do
     * upsampling only) */
    fDoDecodeSbrData = 0;
  }

  if (fDoDecodeSbrData) {
    if ((INT)FDKgetValidBits(hBs) <= 0) {
      fDoDecodeSbrData = 0;
    }
  }

  /*
     SBR CRC-check
  */
  if (fDoDecodeSbrData) {
    if (crcFlag) {
      switch (self->coreCodec) {
        case AOT_DRM_AAC:
        case AOT_DRM_SURROUND:
          crcPoly = 0x001d;
          crcLen = 8;
          crcStartValue = 0x000000ff;
          break;
        default:
          crcPoly = 0x0633;
          crcLen = 10;
          crcStartValue = 0x00000000;
          break;
      }
      sbrCrc = (USHORT)FDKreadBits(hBs, crcLen);
      /* Setup CRC decoder */
      FDKcrcInit(&crcInfo, crcPoly, crcStartValue, crcLen);
      /* Start CRC region */
      crcReg = FDKcrcStartReg(&crcInfo, hBs, 0);
    }
  } /* if (fDoDecodeSbrData) */

  /*
     Read in the header data and issue a reset if change occured
  */
  if (fDoDecodeSbrData) {
    int sbrHeaderPresent;

    if (self->flags & (SBRDEC_SYNTAX_RSVD50 | SBRDEC_SYNTAX_USAC)) {
      SBR_HEADER_DATA_BS_INFO newSbrInfo;
      int sbrInfoPresent;

      if (bs_interTes) {
        self->flags |= SBRDEC_USAC_ITES;
      } else {
        self->flags &= ~SBRDEC_USAC_ITES;
      }

      if (fGlobalIndependencyFlag) {
        self->flags |= SBRDEC_USAC_INDEP;
        sbrInfoPresent = 1;
        sbrHeaderPresent = 1;
      } else {
        self->flags &= ~SBRDEC_USAC_INDEP;
        sbrInfoPresent = FDKreadBit(hBs);
        if (sbrInfoPresent) {
          sbrHeaderPresent = FDKreadBit(hBs);
        } else {
          sbrHeaderPresent = 0;
        }
      }

      if (sbrInfoPresent) {
        newSbrInfo.ampResolution = FDKreadBit(hBs);
        newSbrInfo.xover_band = FDKreadBits(hBs, 4);
        newSbrInfo.sbr_preprocessing = FDKreadBit(hBs);
        if (bs_pvc) {
          newSbrInfo.pvc_mode = FDKreadBits(hBs, 2);
          /* bs_pvc_mode: 0 -> no PVC, 1 -> PVC mode 1, 2 -> PVC mode 2, 3 ->
           * reserved */
          if (newSbrInfo.pvc_mode > 2) {
            headerStatus = HEADER_ERROR;
          }
          if (stereo && newSbrInfo.pvc_mode > 0) {
            /* bs_pvc is always transmitted but pvc_mode is set to zero in case
             * of stereo SBR. The config might be wrong but we cannot tell for
             * sure. */
            newSbrInfo.pvc_mode = 0;
          }
        } else {
          newSbrInfo.pvc_mode = 0;
        }
        if (headerStatus != HEADER_ERROR) {
          if (FDKmemcmp(&hSbrHeader->bs_info, &newSbrInfo,
                        sizeof(SBR_HEADER_DATA_BS_INFO))) {
            /* in case of ampResolution and preprocessing change no full reset
             * required    */
            /* HEADER reset would trigger HBE transposer reset which breaks
             * eSbr_3_Eaa.mp4 */
            if ((hSbrHeader->bs_info.pvc_mode != newSbrInfo.pvc_mode) ||
                (hSbrHeader->bs_info.xover_band != newSbrInfo.xover_band)) {
              headerStatus = HEADER_RESET;
            } else {
              headerStatus = HEADER_OK;
            }

            hSbrHeader->bs_info = newSbrInfo;
          } else {
            headerStatus = HEADER_OK;
          }
        }
      }
      if (headerStatus == HEADER_ERROR) {
        /* Corrupt SBR info data, do not decode and switch to UPSAMPLING */
        hSbrHeader->syncState = hSbrHeader->syncState > UPSAMPLING
                                    ? UPSAMPLING
                                    : hSbrHeader->syncState;
        fDoDecodeSbrData = 0;
        sbrHeaderPresent = 0;
      }

      if (sbrHeaderPresent && fDoDecodeSbrData) {
        int useDfltHeader;

        useDfltHeader = FDKreadBit(hBs);

        if (useDfltHeader) {
          sbrHeaderPresent = 0;
          if (FDKmemcmp(&hSbrHeader->bs_data, &hSbrHeader->bs_dflt,
                        sizeof(SBR_HEADER_DATA_BS)) ||
              hSbrHeader->syncState != SBR_ACTIVE) {
            hSbrHeader->bs_data = hSbrHeader->bs_dflt;
            headerStatus = HEADER_RESET;
          }
        }
      }
    } else {
      sbrHeaderPresent = FDKreadBit(hBs);
    }

    if (sbrHeaderPresent) {
      headerStatus = sbrGetHeaderData(hSbrHeader, hBs, self->flags, 1, 0);
    }

    if (headerStatus == HEADER_RESET) {
      errorStatus = sbrDecoder_HeaderUpdate(
          self, hSbrHeader, headerStatus, pSbrChannel, hSbrElement->nChannels);

      if (errorStatus == SBRDEC_OK) {
        hSbrHeader->syncState = SBR_HEADER;
      } else {
        hSbrHeader->syncState = SBR_NOT_INITIALIZED;
        headerStatus = HEADER_ERROR;
      }
    }

    if (errorStatus != SBRDEC_OK) {
      fDoDecodeSbrData = 0;
    }
  } /* if (fDoDecodeSbrData) */

  /*
    Print debugging output only if state has changed
  */

  /* read frame data */
  if ((hSbrHeader->syncState >= SBR_HEADER) && fDoDecodeSbrData) {
    int sbrFrameOk;
    /* read the SBR element data */
    if (!stereo && (self->hParametricStereoDec != NULL)) {
      /* update slot index for PS bitstream parsing */
      self->hParametricStereoDec->bsLastSlot =
          self->hParametricStereoDec->bsReadSlot;
      self->hParametricStereoDec->bsReadSlot = hSbrElement->useFrameSlot;
    }
    sbrFrameOk = sbrGetChannelElement(
        hSbrHeader, hFrameDataLeft, (stereo) ? hFrameDataRight : NULL,
        &pSbrChannel[0]->prevFrameData,
        pSbrChannel[0]->SbrDec.PvcStaticData.pvc_mode_last, hBs,
        (stereo) ? NULL : self->hParametricStereoDec, self->flags,
        self->pSbrElement[elementIndex]->transposerSettings.overlap);

    if (!sbrFrameOk) {
      fDoDecodeSbrData = 0;
    } else {
      INT valBits;

      if (bsPayLen > 0) {
        valBits = bsPayLen - ((INT)startPos - (INT)FDKgetValidBits(hBs));
      } else {
        valBits = (INT)FDKgetValidBits(hBs);
      }

      /* sanity check of remaining bits */
      if (valBits < 0) {
        fDoDecodeSbrData = 0;
      } else {
        switch (self->coreCodec) {
          case AOT_SBR:
          case AOT_PS:
          case AOT_AAC_LC: {
            /* This sanity check is only meaningful with General Audio
             * bitstreams */
            alignBits = valBits & 0x7;

            if (valBits > alignBits) {
              fDoDecodeSbrData = 0;
            }
          } break;
          default:
            /* No sanity check available */
            break;
        }
      }
    }
  } else {
    /* The returned bit count will not be the actual payload size since we did
       not parse the frame data. Return an error so that the caller can react
       respectively. */
    errorStatus = SBRDEC_PARSE_ERROR;
  }

  if (crcFlag && (hSbrHeader->syncState >= SBR_HEADER) && fDoDecodeSbrData) {
    FDKpushFor(hBs, alignBits);
    FDKcrcEndReg(&crcInfo, hBs, crcReg); /* End CRC region */
    FDKpushBack(hBs, alignBits);
    /* Check CRC */
    if ((FDKcrcGetCRC(&crcInfo) ^ crcStartValue) != sbrCrc) {
      fDoDecodeSbrData = 0;
      if (headerStatus != HEADER_NOT_PRESENT) {
        headerStatus = HEADER_ERROR;
        hSbrHeader->syncState = SBR_NOT_INITIALIZED;
      }
    }
  }

  if (!fDoDecodeSbrData) {
    /* Set error flag for this slot to trigger concealment */
    setFrameErrorFlag(self->pSbrElement[elementIndex], FRAME_ERROR);
    /* restore old frameData for concealment */
    FDKmemcpy(hFrameDataLeft, &frameDataLeftCopy, sizeof(SBR_FRAME_DATA));
    if (stereo) {
      FDKmemcpy(hFrameDataRight, &frameDataRightCopy, sizeof(SBR_FRAME_DATA));
    }
    errorStatus = SBRDEC_PARSE_ERROR;
  } else {
    /* Everything seems to be ok so clear the error flag */
    setFrameErrorFlag(self->pSbrElement[elementIndex], FRAME_OK);
  }

  if (!stereo) {
    /* Turn coupling off explicitely to avoid access to absent right frame data
       that might occur with corrupt bitstreams. */
    hFrameDataLeft->coupling = COUPLING_OFF;
  }

bail:

  if (self != NULL) {
    if (self->flags & SBRDEC_SYNTAX_DRM) {
      hBs = hBsOriginal;
    }

    if (errorStatus != SBRDEC_NOT_INITIALIZED) {
      int useOldHdr =
          ((headerStatus == HEADER_NOT_PRESENT) ||
           (headerStatus == HEADER_ERROR) ||
           (headerStatus == HEADER_RESET && errorStatus == SBRDEC_PARSE_ERROR))
              ? 1
              : 0;

      if (!useOldHdr && (thisHdrSlot != lastHdrSlot) && (hSbrHeader != NULL)) {
        useOldHdr |=
            (compareSbrHeader(hSbrHeader,
                              &self->sbrHeader[elementIndex][lastHdrSlot]) == 0)
                ? 1
                : 0;
      }

      if (hSbrElement != NULL) {
        if (useOldHdr != 0) {
          /* Use the old header for this frame */
          hSbrElement->useHeaderSlot[hSbrElement->useFrameSlot] = lastHdrSlot;
        } else {
          /* Use the new header for this frame */
          hSbrElement->useHeaderSlot[hSbrElement->useFrameSlot] = thisHdrSlot;
        }

        /* Move frame pointer to the next slot which is up to be decoded/applied
         * next */
        hSbrElement->useFrameSlot =
            (hSbrElement->useFrameSlot + 1) % (self->numDelayFrames + 1);
      }
    }
  }

  *count -= startPos - (INT)FDKgetValidBits(hBs);

  return errorStatus;
}

/**
 * \brief Render one SBR element into time domain signal.
 * \param self SBR decoder handle
 * \param timeData pointer to output buffer
 * \param channelMapping pointer to UCHAR array where next 2 channel offsets are
 * stored.
 * \param elementIndex enumerating index of the SBR element to render.
 * \param numInChannels number of channels from core coder.
 * \param numOutChannels pointer to a location to return number of output
 * channels.
 * \param psPossible flag indicating if PS is possible or not.
 * \return SBRDEC_OK if successfull, else error code
 */
static SBR_ERROR sbrDecoder_DecodeElement(
    HANDLE_SBRDECODER self, LONG *input, LONG *timeData, const int timeDataSize,
    const FDK_channelMapDescr *const mapDescr, const int mapIdx,
    int channelIndex, const int elementIndex, const int numInChannels,
    int *numOutChannels, const int psPossible) {
  SBR_DECODER_ELEMENT *hSbrElement = self->pSbrElement[elementIndex];
  HANDLE_SBR_CHANNEL *pSbrChannel =
      self->pSbrElement[elementIndex]->pSbrChannel;
  HANDLE_SBR_HEADER_DATA hSbrHeader =
      &self->sbrHeader[elementIndex]
                      [hSbrElement->useHeaderSlot[hSbrElement->useFrameSlot]];
  HANDLE_PS_DEC h_ps_d = self->hParametricStereoDec;

  /* get memory for frame data from scratch */
  SBR_FRAME_DATA *hFrameDataLeft = NULL;
  SBR_FRAME_DATA *hFrameDataRight = NULL;

  SBR_ERROR errorStatus = SBRDEC_OK;

  INT strideOut, offset0 = 255, offset0_block = 0, offset1 = 255,
                 offset1_block = 0;
  INT codecFrameSize = self->codecFrameSize;

  int stereo = (hSbrElement->elementID == ID_CPE) ? 1 : 0;
  int numElementChannels =
      hSbrElement
          ->nChannels; /* Number of channels of the current SBR element */

  hFrameDataLeft =
      &hSbrElement->pSbrChannel[0]->frameData[hSbrElement->useFrameSlot];
  if (stereo) {
    hFrameDataRight =
        &hSbrElement->pSbrChannel[1]->frameData[hSbrElement->useFrameSlot];
  }

  if (self->flags & SBRDEC_FLUSH) {
    if (self->numFlushedFrames > self->numDelayFrames) {
      int hdrIdx;
      /* No valid SBR payload available, hence switch to upsampling (in all
       * headers) */
      for (hdrIdx = 0; hdrIdx < ((1) + 1); hdrIdx += 1) {
        if (self->sbrHeader[elementIndex][hdrIdx].syncState > UPSAMPLING) {
          self->sbrHeader[elementIndex][hdrIdx].syncState = UPSAMPLING;
        }
      }
    } else {
      /* Move frame pointer to the next slot which is up to be decoded/applied
       * next */
      hSbrElement->useFrameSlot =
          (hSbrElement->useFrameSlot + 1) % (self->numDelayFrames + 1);
      /* Update header and frame data pointer because they have already been set
       */
      hSbrHeader =
          &self->sbrHeader[elementIndex]
                          [hSbrElement
                               ->useHeaderSlot[hSbrElement->useFrameSlot]];
      hFrameDataLeft =
          &hSbrElement->pSbrChannel[0]->frameData[hSbrElement->useFrameSlot];
      if (stereo) {
        hFrameDataRight =
            &hSbrElement->pSbrChannel[1]->frameData[hSbrElement->useFrameSlot];
      }
    }
  }

  /* Update the header error flag */
  hSbrHeader->frameErrorFlag =
      hSbrElement->frameErrorFlag[hSbrElement->useFrameSlot];

  /*
     Prepare filterbank for upsampling if no valid bit stream data is available.
   */
  if (hSbrHeader->syncState == SBR_NOT_INITIALIZED) {
    errorStatus =
        initHeaderData(hSbrHeader, self->sampleRateIn, self->sampleRateOut,
                       self->downscaleFactor, codecFrameSize, self->flags,
                       1 /* SET_DEFAULT_HDR */
        );

    if (errorStatus != SBRDEC_OK) {
      return errorStatus;
    }

    hSbrHeader->syncState = UPSAMPLING;

    errorStatus = sbrDecoder_HeaderUpdate(self, hSbrHeader, HEADER_NOT_PRESENT,
                                          pSbrChannel, hSbrElement->nChannels);

    if (errorStatus != SBRDEC_OK) {
      hSbrHeader->syncState = SBR_NOT_INITIALIZED;
      return errorStatus;
    }
  }

  /* reset */
  if (hSbrHeader->status & SBRDEC_HDR_STAT_RESET) {
    int ch;
    int applySbrProc = (hSbrHeader->syncState == SBR_ACTIVE ||
                        (hSbrHeader->frameErrorFlag == 0 &&
                         hSbrHeader->syncState == SBR_HEADER));
    for (ch = 0; ch < numElementChannels; ch++) {
      SBR_ERROR errorStatusTmp = SBRDEC_OK;

      errorStatusTmp = resetSbrDec(
          &pSbrChannel[ch]->SbrDec, hSbrHeader, &pSbrChannel[ch]->prevFrameData,
          self->synDownsampleFac, self->flags, pSbrChannel[ch]->frameData);

      if (errorStatusTmp != SBRDEC_OK) {
        hSbrHeader->syncState = UPSAMPLING;
      }
    }
    if (applySbrProc) {
      hSbrHeader->status &= ~SBRDEC_HDR_STAT_RESET;
    }
  }

  /* decoding */
  if ((hSbrHeader->syncState == SBR_ACTIVE) ||
      ((hSbrHeader->syncState == SBR_HEADER) &&
       (hSbrHeader->frameErrorFlag == 0))) {
    errorStatus = SBRDEC_OK;

    decodeSbrData(hSbrHeader, hFrameDataLeft, &pSbrChannel[0]->prevFrameData,
                  (stereo) ? hFrameDataRight : NULL,
                  (stereo) ? &pSbrChannel[1]->prevFrameData : NULL);

    /* Now we have a full parameter set and can do parameter
       based concealment instead of plain upsampling. */
    hSbrHeader->syncState = SBR_ACTIVE;
  }

  if (timeDataSize <
      hSbrHeader->numberTimeSlots * hSbrHeader->timeStep *
          self->pQmfDomain->globalConf.nBandsSynthesis *
          (psPossible ? fMax(2, numInChannels) : numInChannels)) {
    return SBRDEC_OUTPUT_BUFFER_TOO_SMALL;
  }

  {
    self->flags &= ~SBRDEC_PS_DECODED;
    C_ALLOC_SCRATCH_START(pPsScratch, struct PS_DEC_COEFFICIENTS, 1)

    /* decode PS data if available */
    if (h_ps_d != NULL && psPossible && (hSbrHeader->syncState == SBR_ACTIVE)) {
      int applyPs = 1;

      /* define which frame delay line slot to process */
      h_ps_d->processSlot = hSbrElement->useFrameSlot;

      applyPs = DecodePs(h_ps_d, hSbrHeader->frameErrorFlag, pPsScratch);
      self->flags |= (applyPs) ? SBRDEC_PS_DECODED : 0;
    }

    offset0 = FDK_chMapDescr_getMapValue(mapDescr, channelIndex, mapIdx);
    offset0_block = offset0 * codecFrameSize;
    if (stereo || psPossible) {
      /* the value of offset1 only matters if the condition is true, however if
      it is not true channelIndex+1 may exceed the channel map resutling in an
      error, though the value of offset1 is actually meaningless. This is
      prevented here. */
      offset1 = FDK_chMapDescr_getMapValue(mapDescr, channelIndex + 1, mapIdx);
      offset1_block = offset1 * codecFrameSize;
    }
    /* Set strides for reading and writing */
    if (psPossible)
      strideOut = (numInChannels < 2) ? 2 : numInChannels;
    else
      strideOut = numInChannels;

    /* use same buffers for left and right channel and apply PS per timeslot */
    /* Process left channel */
    sbr_dec(&pSbrChannel[0]->SbrDec, input + offset0_block, timeData + offset0,
            (self->flags & SBRDEC_PS_DECODED) ? &pSbrChannel[1]->SbrDec : NULL,
            timeData + offset1, strideOut, hSbrHeader, hFrameDataLeft,
            &pSbrChannel[0]->prevFrameData,
            (hSbrHeader->syncState == SBR_ACTIVE), h_ps_d, self->flags,
            codecFrameSize, self->sbrInDataHeadroom);

    if (stereo) {
      /* Process right channel */
      sbr_dec(&pSbrChannel[1]->SbrDec, input + offset1_block,
              timeData + offset1, NULL, NULL, strideOut, hSbrHeader,
              hFrameDataRight, &pSbrChannel[1]->prevFrameData,
              (hSbrHeader->syncState == SBR_ACTIVE), NULL, self->flags,
              codecFrameSize, self->sbrInDataHeadroom);
    }

    C_ALLOC_SCRATCH_END(pPsScratch, struct PS_DEC_COEFFICIENTS, 1)
  }

  if (h_ps_d != NULL) {
    /* save PS status for next run */
    h_ps_d->psDecodedPrv = (self->flags & SBRDEC_PS_DECODED) ? 1 : 0;
  }

  if (psPossible && !(self->flags & SBRDEC_SKIP_QMF_SYN)) {
    FDK_ASSERT(strideOut > 1);
    if (!(self->flags & SBRDEC_PS_DECODED)) {
      /* A decoder which is able to decode PS has to produce a stereo output
       * even if no PS data is available. */
      /* So copy left channel to right channel. */
      int copyFrameSize =
          codecFrameSize * self->pQmfDomain->QmfDomainOut->fb.no_channels;
      copyFrameSize /= self->pQmfDomain->QmfDomainIn->fb.no_channels;
      LONG *ptr;
      INT i;
      FDK_ASSERT(strideOut == 2);

      ptr = timeData;
      for (i = copyFrameSize >> 1; i--;) {
        LONG tmp; /* This temporal variable is required because some compilers
                     can't do *ptr++ = *ptr++ correctly. */
        tmp = *ptr++;
        *ptr++ = tmp;
        tmp = *ptr++;
        *ptr++ = tmp;
      }
    }
    *numOutChannels = 2; /* Output minimum two channels when PS is enabled. */
  }

  return errorStatus;
}

SBR_ERROR sbrDecoder_Apply(HANDLE_SBRDECODER self, LONG *input, LONG *timeData,
                           const int timeDataSize, int *numChannels,
                           int *sampleRate,
                           const FDK_channelMapDescr *const mapDescr,
                           const int mapIdx, const int coreDecodedOk,
                           UCHAR *psDecoded, const INT inDataHeadroom,
                           INT *outDataHeadroom) {
  SBR_ERROR errorStatus = SBRDEC_OK;

  int psPossible;
  int sbrElementNum;
  int numCoreChannels;
  int numSbrChannels = 0;

  if ((self == NULL) || (timeData == NULL) || (numChannels == NULL) ||
      (sampleRate == NULL) || (psDecoded == NULL) ||
      !FDK_chMapDescr_isValid(mapDescr)) {
    return SBRDEC_INVALID_ARGUMENT;
  }

  psPossible = *psDecoded;
  numCoreChannels = *numChannels;
  if (numCoreChannels <= 0) {
    return SBRDEC_INVALID_ARGUMENT;
  }

  if (self->numSbrElements < 1) {
    /* exit immediately to avoid access violations */
    return SBRDEC_NOT_INITIALIZED;
  }

  /* Sanity check of allocated SBR elements. */
  for (sbrElementNum = 0; sbrElementNum < self->numSbrElements;
       sbrElementNum++) {
    if (self->pSbrElement[sbrElementNum] == NULL) {
      return SBRDEC_NOT_INITIALIZED;
    }
  }

  if (self->numSbrElements != 1 || self->pSbrElement[0]->elementID != ID_SCE) {
    psPossible = 0;
  }

  self->sbrInDataHeadroom = inDataHeadroom;
  *outDataHeadroom = (INT)(8);

  /* Make sure that even if no SBR data was found/parsed *psDecoded is returned
   * 1 if psPossible was 0. */
  if (psPossible == 0) {
    self->flags &= ~SBRDEC_PS_DECODED;
  }

  /* replaces channel based reset inside sbr_dec() */
  if (((self->flags & SBRDEC_LOW_POWER) ? 1 : 0) !=
      ((self->pQmfDomain->globalConf.flags & QMF_FLAG_LP) ? 1 : 0)) {
    if (self->flags & SBRDEC_LOW_POWER) {
      self->pQmfDomain->globalConf.flags |= QMF_FLAG_LP;
      self->pQmfDomain->globalConf.flags_requested |= QMF_FLAG_LP;
    } else {
      self->pQmfDomain->globalConf.flags &= ~QMF_FLAG_LP;
      self->pQmfDomain->globalConf.flags_requested &= ~QMF_FLAG_LP;
    }
    if (FDK_QmfDomain_InitFilterBank(self->pQmfDomain, QMF_FLAG_KEEP_STATES)) {
      return SBRDEC_UNSUPPORTED_CONFIG;
    }
  }
  if (self->numSbrChannels > self->pQmfDomain->globalConf.nInputChannels) {
    return SBRDEC_UNSUPPORTED_CONFIG;
  }

  if (self->flags & SBRDEC_FLUSH) {
    /* flushing is signalized, hence increment the flush frame counter */
    self->numFlushedFrames++;
  } else {
    /* no flushing is signalized, hence reset the flush frame counter */
    self->numFlushedFrames = 0;
  }

  /* Loop over SBR elements */
  for (sbrElementNum = 0; sbrElementNum < self->numSbrElements;
       sbrElementNum++) {
    int numElementChan;

    if (psPossible &&
        self->pSbrElement[sbrElementNum]->pSbrChannel[1] == NULL) {
      /* Disable PS and try decoding SBR mono. */
      psPossible = 0;
    }

    numElementChan =
        (self->pSbrElement[sbrElementNum]->elementID == ID_CPE) ? 2 : 1;

    /* If core signal is bad then force upsampling */
    if (!coreDecodedOk) {
      setFrameErrorFlag(self->pSbrElement[sbrElementNum], FRAME_ERROR_ALLSLOTS);
    }

    errorStatus = sbrDecoder_DecodeElement(
        self, input, timeData, timeDataSize, mapDescr, mapIdx, numSbrChannels,
        sbrElementNum,
        numCoreChannels, /* is correct even for USC SCI==2 case */
        &numElementChan, psPossible);

    if (errorStatus != SBRDEC_OK) {
      goto bail;
    }

    numSbrChannels += numElementChan;

    if (numSbrChannels >= numCoreChannels) {
      break;
    }
  }

  /* Update numChannels and samplerate */
  /* Do not mess with output channels in case of USAC. numSbrChannels !=
   * numChannels for stereoConfigIndex == 2 */
  if (!(self->flags & SBRDEC_SYNTAX_USAC)) {
    *numChannels = numSbrChannels;
  }
  *sampleRate = self->sampleRateOut;
  *psDecoded = (self->flags & SBRDEC_PS_DECODED) ? 1 : 0;

  /* Clear reset and flush flag because everything seems to be done
   * successfully. */
  self->flags &= ~SBRDEC_FORCE_RESET;
  self->flags &= ~SBRDEC_FLUSH;

bail:

  return errorStatus;
}

SBR_ERROR sbrDecoder_Close(HANDLE_SBRDECODER *pSelf) {
  HANDLE_SBRDECODER self = *pSelf;
  int i;

  if (self != NULL) {
    if (self->hParametricStereoDec != NULL) {
      DeletePsDec(&self->hParametricStereoDec);
    }

    for (i = 0; i < (8); i++) {
      sbrDecoder_DestroyElement(self, i);
    }

    FreeRam_SbrDecoder(pSelf);
  }

  return SBRDEC_OK;
}

INT sbrDecoder_GetLibInfo(LIB_INFO *info) {
  int i;

  if (info == NULL) {
    return -1;
  }

  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) return -1;
  info += i;

  info->module_id = FDK_SBRDEC;
  info->version =
      LIB_VERSION(SBRDECODER_LIB_VL0, SBRDECODER_LIB_VL1, SBRDECODER_LIB_VL2);
  LIB_VERSION_STRING(info);
  info->build_date = SBRDECODER_LIB_BUILD_DATE;
  info->build_time = SBRDECODER_LIB_BUILD_TIME;
  info->title = SBRDECODER_LIB_TITLE;

  /* Set flags */
  info->flags = 0 | CAPF_SBR_HQ | CAPF_SBR_LP | CAPF_SBR_PS_MPEG |
                CAPF_SBR_DRM_BS | CAPF_SBR_CONCEALMENT | CAPF_SBR_DRC |
                CAPF_SBR_ELD_DOWNSCALE | CAPF_SBR_HBEHQ;
  /* End of flags */

  return 0;
}

UINT sbrDecoder_GetDelay(const HANDLE_SBRDECODER self) {
  UINT outputDelay = 0;

  if (self != NULL) {
    UINT flags = self->flags;

    /* See chapter 1.6.7.2 of ISO/IEC 14496-3 for the GA-SBR figures below. */

    /* Are we initialized? */
    if ((self->numSbrChannels > 0) && (self->numSbrElements > 0)) {
      /* Add QMF synthesis delay */
      if ((flags & SBRDEC_ELD_GRID) && IS_LOWDELAY(self->coreCodec)) {
        /* Low delay SBR: */
        if (!(flags & SBRDEC_SKIP_QMF_SYN)) {
          outputDelay +=
              (flags & SBRDEC_DOWNSAMPLE) ? 32 : 64; /* QMF synthesis */
          if (flags & SBRDEC_LD_MPS_QMF) {
            outputDelay += 32;
          }
        }
      } else if (!IS_USAC(self->coreCodec)) {
        /* By the method of elimination this is the GA (AAC-LC, HE-AAC, ...)
         * branch: */
        outputDelay += (flags & SBRDEC_DOWNSAMPLE) ? 481 : 962;
        if (flags & SBRDEC_SKIP_QMF_SYN) {
          outputDelay -= 257; /* QMF synthesis */
        }
      }
    }
  }

  return (outputDelay);
}
