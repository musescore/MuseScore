/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** AAC decoder library ******************************

   Author(s):   Josef Hoepfl

   Description:

*******************************************************************************/

/*!
  \page default General Overview of the AAC Decoder Implementation

  The main entry point to decode a AAC frame is CAacDecoder_DecodeFrame(). It
  handles the different transport multiplexes and bitstream formats supported by
  this implementation. It extracts the AAC_raw_data_blocks from these bitstreams
  to further process then in the actual decoding stages.

  Note: Click on a function of file in the above image to see details about the
  function. Also note, that this is just an overview of the most important
  functions and not a complete call graph.

  <h2>1 Bitstream deformatter</h2>
  The basic bit stream parser function CChannelElement_Read() is called. It uses
  other subcalls in order to parse and unpack the bitstreams. Note, that this
  includes huffmann decoding of the coded spectral data. This operation can be
  computational significant specifically at higher bitrates. Optimization is
  likely in CBlock_ReadSpectralData().

  The bitstream deformatter also includes many bitfield operations. Profiling on
  the target will determine required optimizations.

  <h2>2 Actual decoding to retain the time domain output</h2>
  The basic bitstream deformatter function CChannelElement_Decode() for CPE
  elements and SCE elements are called. Except for the stereo processing (2.1)
  which is only used for CPE elements, the function calls for CPE or SCE are
  similar, except that CPE always processes to independent channels while SCE
  only processes one channel.

  Often there is the distinction between long blocks and short blocks. However,
  computational expensive functions that ususally require optimization are being
  shared by these two groups,

  <h3>2.1 Stereo processing for CPE elements</h3>
  CChannelPairElement_Decode() first calles the joint stereo  tools in
  stereo.cpp when required.

  <h3>2.2 Scaling of spectral data</h3>
  CBlock_ScaleSpectralData().

  <h3>2.3 Apply additional coding tools</h3>
  ApplyTools() calles the PNS tools in case of MPEG-4 bitstreams, and TNS
  filtering CTns_Apply() for MPEG-2 and MPEG-4 bitstreams. The function
  TnsFilterIIR() which is called by CTns_Apply() (2.3.1) might require some
  optimization.

  <h2>3 Frequency-To-Time conversion</h3>
  The filterbank is called using CBlock_FrequencyToTime() using the MDCT module
  from the FDK Tools

*/

#include "aacdecoder.h"

#include "aac_rom.h"
#include "aac_ram.h"
#include "channel.h"
#include "FDK_audio.h"

#include "aacdec_pns.h"

#include "sbrdecoder.h"

#include "sac_dec_lib.h"

#include "aacdec_hcr.h"
#include "rvlc.h"

#include "usacdec_lpd.h"

#include "ac_arith_coder.h"

#include "tpdec_lib.h"

#include "conceal.h"

#include "FDK_crc.h"
#define PS_IS_EXPLICITLY_DISABLED(aot, flags) \
  (((aot) == AOT_DRM_AAC) && !(flags & AC_PS_PRESENT))

#define IS_STEREO_SBR(el_id, stereoConfigIndex)            \
  (((el_id) == ID_USAC_CPE && (stereoConfigIndex) == 0) || \
   ((el_id) == ID_USAC_CPE && (stereoConfigIndex) == 3))

void CAacDecoder_SyncQmfMode(HANDLE_AACDECODER self) {
  FDK_ASSERT(
      !((self->flags[0] & AC_MPS_PRESENT) && (self->flags[0] & AC_PS_PRESENT)));

  /* Assign user requested mode */
  self->qmfModeCurr = self->qmfModeUser;

  if (IS_USAC(self->streamInfo.aot)) {
    self->qmfModeCurr = MODE_HQ;
  }

  if (self->qmfModeCurr == NOT_DEFINED) {
    if ((IS_LOWDELAY(self->streamInfo.aot) &&
         (self->flags[0] & AC_MPS_PRESENT)) ||
        ((self->streamInfo.aacNumChannels == 1) &&
         ((CAN_DO_PS(self->streamInfo.aot) &&
           !(self->flags[0] & AC_MPS_PRESENT)) ||
          (IS_USAC(self->streamInfo.aot))))) {
      self->qmfModeCurr = MODE_HQ;
    } else {
      self->qmfModeCurr = MODE_LP;
    }
  }

  if (self->mpsEnableCurr) {
    if (IS_LOWDELAY(self->streamInfo.aot) &&
        (self->qmfModeCurr == MODE_LP)) { /* Overrule user requested QMF mode */
      self->qmfModeCurr = MODE_HQ;
    }
    /* Set and check if MPS decoder allows the current mode */
    switch (mpegSurroundDecoder_SetParam(
        (CMpegSurroundDecoder *)self->pMpegSurroundDecoder,
        SACDEC_PARTIALLY_COMPLEX, self->qmfModeCurr == MODE_LP)) {
      case MPS_OK:
        break;
      case MPS_INVALID_PARAMETER: { /* Only one mode supported. Find out which
                                       one: */
        LIB_INFO libInfo[FDK_MODULE_LAST];
        UINT mpsCaps;

        FDKinitLibInfo(libInfo);
        mpegSurroundDecoder_GetLibInfo(libInfo);
        mpsCaps = FDKlibInfo_getCapabilities(libInfo, FDK_MPSDEC);

        if (((mpsCaps & CAPF_MPS_LP) && (self->qmfModeCurr == MODE_LP)) ||
            ((mpsCaps & CAPF_MPS_HQ) &&
             (self->qmfModeCurr ==
              MODE_HQ))) { /* MPS decoder does support the requested mode. */
          break;
        }
      }
        FDK_FALLTHROUGH;
      default:
        if (self->qmfModeUser == NOT_DEFINED) {
          /* Revert in case mpegSurroundDecoder_SetParam() fails. */
          self->qmfModeCurr =
              (self->qmfModeCurr == MODE_LP) ? MODE_HQ : MODE_LP;
        } else {
          /* in case specific mode was requested we disable MPS and playout the
           * downmix */
          self->mpsEnableCurr = 0;
        }
    }
  }

  /* Set SBR to current QMF mode. Error does not matter. */
  sbrDecoder_SetParam(self->hSbrDecoder, SBR_QMF_MODE,
                      (self->qmfModeCurr == MODE_LP));
  self->psPossible =
      ((CAN_DO_PS(self->streamInfo.aot) &&
        !PS_IS_EXPLICITLY_DISABLED(self->streamInfo.aot, self->flags[0]) &&
        self->streamInfo.aacNumChannels == 1 &&
        !(self->flags[0] & AC_MPS_PRESENT))) &&
      self->qmfModeCurr == MODE_HQ;
  FDK_ASSERT(!((self->flags[0] & AC_MPS_PRESENT) && self->psPossible));
}

void CAacDecoder_SignalInterruption(HANDLE_AACDECODER self) {
  if (self->flags[0] & (AC_USAC | AC_RSVD50 | AC_RSV603DA)) {
    int i;

    for (i = 0; i < fMin(self->aacChannels, (8)); i++) {
      if (self->pAacDecoderStaticChannelInfo
              [i]) { /* number of active channels can be smaller */
        self->pAacDecoderStaticChannelInfo[i]->hArCo->m_numberLinesPrev = 0;
      }
    }
  }
}

/*!
  \brief Calculates the number of element channels

  \type  channel type
  \usacStereoConfigIndex  usac stereo config index

  \return  element channels
*/
static int CAacDecoder_GetELChannels(MP4_ELEMENT_ID type,
                                     UCHAR usacStereoConfigIndex) {
  int el_channels = 0;

  switch (type) {
    case ID_USAC_CPE:
      if (usacStereoConfigIndex == 1) {
        el_channels = 1;
      } else {
        el_channels = 2;
      }
      break;
    case ID_CPE:
      el_channels = 2;
      break;
    case ID_USAC_SCE:
    case ID_USAC_LFE:
    case ID_SCE:
    case ID_LFE:
      el_channels = 1;
      break;
    default:
      el_channels = 0;
      break;
  }

  return el_channels;
}

/*!
  \brief Reset ancillary data struct. Call before parsing a new frame.

  \ancData Pointer to ancillary data structure

  \return  Error code
*/
static AAC_DECODER_ERROR CAacDecoder_AncDataReset(CAncData *ancData) {
  int i;
  for (i = 0; i < 8; i++) {
    ancData->offset[i] = 0;
  }
  ancData->nrElements = 0;

  return AAC_DEC_OK;
}

/*!
  \brief Initialize ancillary buffer

  \ancData Pointer to ancillary data structure
  \buffer Pointer to (external) anc data buffer
  \size Size of the buffer pointed on by buffer in bytes

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataInit(CAncData *ancData,
                                          unsigned char *buffer, int size) {
  if (size >= 0) {
    ancData->buffer = buffer;
    ancData->bufferSize = size;

    CAacDecoder_AncDataReset(ancData);

    return AAC_DEC_OK;
  }

  return AAC_DEC_ANC_DATA_ERROR;
}

/*!
  \brief Get one ancillary data element

  \ancData Pointer to ancillary data structure
  \index Index of the anc data element to get
  \ptr Pointer to a buffer receiving a pointer to the requested anc data element
  \size Pointer to a buffer receiving the length of the requested anc data
  element in bytes

  \return  Error code
*/
AAC_DECODER_ERROR CAacDecoder_AncDataGet(CAncData *ancData, int index,
                                         unsigned char **ptr, int *size) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;

  *ptr = NULL;
  *size = 0;

  if (index >= 0 && index < 8 - 1 && index < ancData->nrElements) {
    *ptr = &ancData->buffer[ancData->offset[index]];
    *size = ancData->offset[index + 1] - ancData->offset[index];
  }

  return error;
}

/*!
  \brief Parse ancillary data

  \ancData Pointer to ancillary data structure
  \hBs Handle to FDK bitstream
  \ancBytes Length of ancillary data to read from the bitstream

  \return  Error code
*/
static AAC_DECODER_ERROR CAacDecoder_AncDataParse(CAncData *ancData,
                                                  HANDLE_FDK_BITSTREAM hBs,
                                                  const int ancBytes) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  int readBytes = 0;

  if (ancData->buffer != NULL) {
    if (ancBytes > 0) {
      /* write ancillary data to external buffer */
      int offset = ancData->offset[ancData->nrElements];

      if ((offset + ancBytes) > ancData->bufferSize) {
        error = AAC_DEC_TOO_SMALL_ANC_BUFFER;
      } else if (ancData->nrElements >= 8 - 1) {
        error = AAC_DEC_TOO_MANY_ANC_ELEMENTS;
      } else {
        int i;

        for (i = 0; i < ancBytes; i++) {
          ancData->buffer[i + offset] = FDKreadBits(hBs, 8);
          readBytes++;
        }

        ancData->nrElements++;
        ancData->offset[ancData->nrElements] =
            ancBytes + ancData->offset[ancData->nrElements - 1];
      }
    }
  }

  readBytes = ancBytes - readBytes;

  if (readBytes > 0) {
    /* skip data */
    FDKpushFor(hBs, readBytes << 3);
  }

  return error;
}

/*!
  \brief Read Stream Data Element

  \bs Bitstream Handle

  \return  Error code
*/
static AAC_DECODER_ERROR CDataStreamElement_Read(HANDLE_AACDECODER self,
                                                 HANDLE_FDK_BITSTREAM bs,
                                                 UCHAR *elementInstanceTag,
                                                 UINT alignmentAnchor) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  UINT dseBits;
  INT dataStart;
  int dataByteAlignFlag, count;

  FDK_ASSERT(self != NULL);

  int crcReg = transportDec_CrcStartReg(self->hInput, 0);

  /* Element Instance Tag */
  *elementInstanceTag = FDKreadBits(bs, 4);
  /* Data Byte Align Flag */
  dataByteAlignFlag = FDKreadBits(bs, 1);

  count = FDKreadBits(bs, 8);

  if (count == 255) {
    count += FDKreadBits(bs, 8); /* EscCount */
  }
  dseBits = count * 8;

  if (dataByteAlignFlag) {
    FDKbyteAlign(bs, alignmentAnchor);
  }

  dataStart = (INT)FDKgetValidBits(bs);

  error = CAacDecoder_AncDataParse(&self->ancData, bs, count);
  transportDec_CrcEndReg(self->hInput, crcReg);

  {
    /* Move to the beginning of the data chunk */
    FDKpushBack(bs, dataStart - (INT)FDKgetValidBits(bs));

    /* Read Anc data if available */
    aacDecoder_drcMarkPayload(self->hDrcInfo, bs, DVB_DRC_ANC_DATA);
  }

  {
    PCMDMX_ERROR dmxErr = PCMDMX_OK;

    /* Move to the beginning of the data chunk */
    FDKpushBack(bs, dataStart - (INT)FDKgetValidBits(bs));

    /* Read DMX meta-data */
    dmxErr = pcmDmx_Parse(self->hPcmUtils, bs, dseBits, 0 /* not mpeg2 */);
    if (error == AAC_DEC_OK && dmxErr != PCMDMX_OK) {
      error = AAC_DEC_UNKNOWN;
    }
  }

  /* Move to the very end of the element. */
  FDKpushBiDirectional(bs, (INT)FDKgetValidBits(bs) - dataStart + (INT)dseBits);

  return error;
}

static INT findElementInstanceTag(
    INT elementTag, MP4_ELEMENT_ID elementId,
    CAacDecoderChannelInfo **pAacDecoderChannelInfo, INT nChannels,
    MP4_ELEMENT_ID *pElementIdTab, INT nElements) {
  int el, chCnt = 0;

  for (el = 0; el < nElements; el++) {
    switch (pElementIdTab[el]) {
      case ID_CPE:
      case ID_SCE:
      case ID_LFE:
        if ((elementTag == pAacDecoderChannelInfo[chCnt]->ElementInstanceTag) &&
            (elementId == pElementIdTab[el])) {
          return 1; /* element instance tag found */
        }
        chCnt += (pElementIdTab[el] == ID_CPE) ? 2 : 1;
        break;
      default:
        break;
    }
    if (chCnt >= nChannels) break;
    if (pElementIdTab[el] == ID_END) break;
  }

  return 0; /* element instance tag not found */
}

static INT validateElementInstanceTags(
    CProgramConfig *pce, CAacDecoderChannelInfo **pAacDecoderChannelInfo,
    INT nChannels, MP4_ELEMENT_ID *pElementIdTab, INT nElements) {
  if (nChannels >= pce->NumChannels) {
    for (int el = 0; el < pce->NumFrontChannelElements; el++) {
      if (!findElementInstanceTag(pce->FrontElementTagSelect[el],
                                  pce->FrontElementIsCpe[el] ? ID_CPE : ID_SCE,
                                  pAacDecoderChannelInfo, nChannels,
                                  pElementIdTab, nElements)) {
        return 0; /* element instance tag not in raw_data_block() */
      }
    }
    for (int el = 0; el < pce->NumSideChannelElements; el++) {
      if (!findElementInstanceTag(pce->SideElementTagSelect[el],
                                  pce->SideElementIsCpe[el] ? ID_CPE : ID_SCE,
                                  pAacDecoderChannelInfo, nChannels,
                                  pElementIdTab, nElements)) {
        return 0; /* element instance tag not in raw_data_block() */
      }
    }
    for (int el = 0; el < pce->NumBackChannelElements; el++) {
      if (!findElementInstanceTag(pce->BackElementTagSelect[el],
                                  pce->BackElementIsCpe[el] ? ID_CPE : ID_SCE,
                                  pAacDecoderChannelInfo, nChannels,
                                  pElementIdTab, nElements)) {
        return 0; /* element instance tag not in raw_data_block() */
      }
    }
    for (int el = 0; el < pce->NumLfeChannelElements; el++) {
      if (!findElementInstanceTag(pce->LfeElementTagSelect[el], ID_LFE,
                                  pAacDecoderChannelInfo, nChannels,
                                  pElementIdTab, nElements)) {
        return 0; /* element instance tag not in raw_data_block() */
      }
    }
  } else {
    return 0; /* too less decoded audio channels */
  }

  return 1; /* all element instance tags found in raw_data_block() */
}

/*!
  \brief Read Program Config Element

  \bs Bitstream Handle
  \pTp Transport decoder handle for CRC handling
  \pce Pointer to PCE buffer
  \channelConfig Current channel configuration
  \alignAnchor Anchor for byte alignment

  \return  PCE status (-1: fail, 0: no new PCE, 1: PCE updated, 2: PCE updated
  need re-config).
*/
static int CProgramConfigElement_Read(HANDLE_FDK_BITSTREAM bs,
                                      HANDLE_TRANSPORTDEC pTp,
                                      CProgramConfig *pce,
                                      const UINT channelConfig,
                                      const UINT alignAnchor) {
  int pceStatus = 0;
  int crcReg;

  /* read PCE to temporal buffer first */
  C_ALLOC_SCRATCH_START(tmpPce, CProgramConfig, 1);

  CProgramConfig_Init(tmpPce);

  crcReg = transportDec_CrcStartReg(pTp, 0);

  CProgramConfig_Read(tmpPce, bs, alignAnchor);

  transportDec_CrcEndReg(pTp, crcReg);

  if (CProgramConfig_IsValid(tmpPce) && (tmpPce->Profile == 1)) {
    if (!CProgramConfig_IsValid(pce) && (channelConfig > 0)) {
      /* Create a standard channel config PCE to compare with */
      CProgramConfig_GetDefault(pce, channelConfig);
    }

    if (CProgramConfig_IsValid(pce)) {
      /* Compare the new and the old PCE (tags ignored) */
      switch (CProgramConfig_Compare(pce, tmpPce)) {
        case 1: /* Channel configuration not changed. Just new metadata. */
          FDKmemcpy(pce, tmpPce,
                    sizeof(CProgramConfig)); /* Store the complete PCE */
          pceStatus = 1; /* New PCE but no change of config */
          break;
        case 2:  /* The number of channels are identical but not the config */
        case -1: /* The channel configuration is completely different */
          pceStatus = -1; /* Not supported! */
          break;
        case 0: /* Nothing to do because PCE matches the old one exactly. */
        default:
          /* pceStatus = 0; */
          break;
      }
    }
  }

  C_ALLOC_SCRATCH_END(tmpPce, CProgramConfig, 1);

  return pceStatus;
}

/*!
  \brief Prepares crossfade for USAC DASH IPF config change

  \pTimeData             Pointer to time data
  \pTimeDataFlush        Pointer to flushed time data
  \numChannels           Number of channels
  \frameSize             Size of frame
  \interleaved           Indicates if time data is interleaved

  \return  Error code
*/
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_PrepareCrossFade(
    const PCM_DEC *pTimeData, PCM_DEC **pTimeDataFlush, const INT numChannels,
    const INT frameSize, const INT interleaved) {
  int i, ch, s1, s2;
  AAC_DECODER_ERROR ErrorStatus;

  ErrorStatus = AAC_DEC_OK;

  if (interleaved) {
    s1 = 1;
    s2 = numChannels;
  } else {
    s1 = frameSize;
    s2 = 1;
  }

  for (ch = 0; ch < numChannels; ch++) {
    const PCM_DEC *pIn = &pTimeData[ch * s1];
    for (i = 0; i < TIME_DATA_FLUSH_SIZE; i++) {
      pTimeDataFlush[ch][i] = *pIn;
      pIn += s2;
    }
  }

  return ErrorStatus;
}

/*!
  \brief Applies crossfade for USAC DASH IPF config change

  \pTimeData             Pointer to time data
  \pTimeDataFlush        Pointer to flushed time data
  \numChannels           Number of channels
  \frameSize             Size of frame
  \interleaved           Indicates if time data is interleaved

  \return  Error code
*/
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_ApplyCrossFade(
    PCM_DEC *pTimeData, PCM_DEC **pTimeDataFlush, const INT numChannels,
    const INT frameSize, const INT interleaved) {
  int i, ch, s1, s2;
  AAC_DECODER_ERROR ErrorStatus;

  ErrorStatus = AAC_DEC_OK;

  if (interleaved) {
    s1 = 1;
    s2 = numChannels;
  } else {
    s1 = frameSize;
    s2 = 1;
  }

  for (ch = 0; ch < numChannels; ch++) {
    PCM_DEC *pIn = &pTimeData[ch * s1];
    for (i = 0; i < TIME_DATA_FLUSH_SIZE; i++) {
      FIXP_SGL alpha = (FIXP_SGL)i
                       << (FRACT_BITS - 1 - TIME_DATA_FLUSH_SIZE_SF);
      FIXP_DBL time = PCM_DEC2FIXP_DBL(*pIn);
      FIXP_DBL timeFlush = PCM_DEC2FIXP_DBL(pTimeDataFlush[ch][i]);

      *pIn = FIXP_DBL2PCM_DEC(timeFlush - fMult(timeFlush, alpha) +
                              fMult(time, alpha));
      pIn += s2;
    }
  }

  return ErrorStatus;
}

/*!
  \brief Parse PreRoll Extension Payload

  \self             Handle of AAC decoder
  \numPrerollAU     Number of preRoll AUs
  \prerollAUOffset  Offset to each preRoll AU
  \prerollAULength  Length of each preRoll AU

  \return  Error code
*/
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_PreRollExtensionPayloadParse(
    HANDLE_AACDECODER self, UINT *numPrerollAU, UINT *prerollAUOffset,
    UINT *prerollAULength) {
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs;
  AAC_DECODER_ERROR ErrorStatus;

  INT auStartAnchor;
  UINT independencyFlag;
  UINT extPayloadPresentFlag;
  UINT useDefaultLengthFlag;
  UINT configLength = 0;
  UINT preRollPossible = 1;
  UINT i;
  UCHAR configChanged = 0;
  UCHAR config[TP_USAC_MAX_CONFIG_LEN] = {0};
  UCHAR
  implicitExplicitCfgDiff = 0; /* in case implicit and explicit config is
                                  equal preroll AU's should be processed
                                  after decoder reset */

  ErrorStatus = AAC_DEC_OK;

  hBs = transportDec_GetBitstream(self->hInput, 0);
  bs = *hBs;

  auStartAnchor = (INT)FDKgetValidBits(hBs);
  if (auStartAnchor <= 0) {
    ErrorStatus = AAC_DEC_NOT_ENOUGH_BITS;
    goto bail;
  }

  /* Independency flag */
  FDKreadBit(hBs);

  /* Payload present flag of extension ID_EXT_ELE_AUDIOPREROLL must be one */
  extPayloadPresentFlag = FDKreadBits(hBs, 1);
  if (!extPayloadPresentFlag) {
    preRollPossible = 0;
  }

  /* Default length flag of extension ID_EXT_ELE_AUDIOPREROLL must be zero */
  useDefaultLengthFlag = FDKreadBits(hBs, 1);
  if (useDefaultLengthFlag) {
    preRollPossible = 0;
  }

  if (preRollPossible) { /* extPayloadPresentFlag && !useDefaultLengthFlag */
    /* Read overall ext payload length, useDefaultLengthFlag must be zero.  */
    escapedValue(hBs, 8, 16, 0);

    /* Read RSVD60 Config size */
    configLength = escapedValue(hBs, 4, 4, 8);

    /* Avoid decoding pre roll frames if there was no config change and no
     * config is included in the pre roll ext payload. */
  }

  /* If pre roll not possible then exit. */
  if (preRollPossible == 0) {
    /* Sanity check: if flushing is switched on, preRollPossible must be 1 */
    if (self->flushStatus != AACDEC_FLUSH_OFF) {
      /* Mismatch of current payload and flushing status */
      self->flushStatus = AACDEC_FLUSH_OFF;
      ErrorStatus = AAC_DEC_PARSE_ERROR;
    }
    goto bail;
  }

  if (self->flags[0] & AC_USAC) {
    if (configLength > 0) {
      /* DASH IPF USAC Config Change: Read new config and compare with current
       * config. Apply reconfiguration if config's are different. */
      for (i = 0; i < configLength; i++) {
        config[i] = FDKreadBits(hBs, 8);
      }
      TRANSPORTDEC_ERROR terr;
      terr = transportDec_InBandConfig(self->hInput, config, configLength,
                                       self->buildUpStatus, &configChanged, 0,
                                       &implicitExplicitCfgDiff);
      if (terr != TRANSPORTDEC_OK) {
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }
  }

  /* For the first frame buildUpStatus is not set and no flushing is performed
   * but preroll AU's should processed. */
  /* For USAC there is no idle state. */
  if ((self->streamInfo.numChannels == 0) && !implicitExplicitCfgDiff &&
      (self->flags[0] & AC_USAC)) {
    self->buildUpStatus = AACDEC_USAC_BUILD_UP_ON;
    /* sanity check: if buildUp status on -> flushing must be off */
    if (self->flushStatus != AACDEC_FLUSH_OFF) {
      self->flushStatus = AACDEC_FLUSH_OFF;
      ErrorStatus = AAC_DEC_PARSE_ERROR;
      goto bail;
    }
  }

  if (self->flags[0] & AC_USAC) {
    /* We are interested in preroll AUs if an explicit or an implicit config
     * change is signalized in other words if the build up status is set. */
    if (self->buildUpStatus == AACDEC_USAC_BUILD_UP_ON) {
      UCHAR applyCrossfade = FDKreadBit(hBs);
      if (applyCrossfade) {
        self->applyCrossfade |= AACDEC_CROSSFADE_BITMASK_PREROLL;
      } else {
        self->applyCrossfade &= ~AACDEC_CROSSFADE_BITMASK_PREROLL;
      }
      FDKreadBit(hBs); /* reserved */
      /* Read num preroll AU's */
      *numPrerollAU = escapedValue(hBs, 2, 4, 0);
      /* check limits for USAC */
      if (*numPrerollAU > AACDEC_MAX_NUM_PREROLL_AU_USAC) {
        *numPrerollAU = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }
  }

  for (i = 0; i < *numPrerollAU; i++) {
    /* For every AU get length and offset in the bitstream */
    prerollAULength[i] = escapedValue(hBs, 16, 16, 0);
    if (prerollAULength[i] > 0) {
      prerollAUOffset[i] = auStartAnchor - (INT)FDKgetValidBits(hBs);
      independencyFlag = FDKreadBit(hBs);
      if (i == 0 && !independencyFlag) {
        *numPrerollAU = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
      FDKpushFor(hBs, prerollAULength[i] * 8 - 1);
      self->prerollAULength[i] = (prerollAULength[i] * 8) + prerollAUOffset[i];
    } else {
      *numPrerollAU = 0;
      ErrorStatus = AAC_DEC_PARSE_ERROR; /* Something is wrong */
      goto bail;
    }
  }

bail:

  *hBs = bs;

  return ErrorStatus;
}

/*!
  \brief Parse Extension Payload

  \self Handle of AAC decoder
  \count Pointer to bit counter.
  \previous_element ID of previous element (required by some extension payloads)

  \return  Error code
*/
static AAC_DECODER_ERROR CAacDecoder_ExtPayloadParse(
    HANDLE_AACDECODER self, HANDLE_FDK_BITSTREAM hBs, int *count,
    MP4_ELEMENT_ID previous_element, int elIndex, int fIsFillElement) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  EXT_PAYLOAD_TYPE extension_type;
  int bytes = (*count) >> 3;
  int crcFlag = 0;

  if (*count < 4) {
    return AAC_DEC_PARSE_ERROR;
  } else if ((INT)FDKgetValidBits(hBs) < *count) {
    return AAC_DEC_DECODE_FRAME_ERROR;
  }

  extension_type =
      (EXT_PAYLOAD_TYPE)FDKreadBits(hBs, 4); /* bs_extension_type */
  *count -= 4;

  /* For ELD, the SBR signaling is explicit and parsed in
     aacDecoder_ParseExplicitMpsAndSbr(), therefore skip SBR if implicit
     present. */
  if ((self->flags[0] & AC_ELD) && ((extension_type == EXT_SBR_DATA_CRC) ||
                                    (extension_type == EXT_SBR_DATA))) {
    extension_type = EXT_FIL; /* skip sbr data */
  }

  switch (extension_type) {
    case EXT_DYNAMIC_RANGE: {
      INT readBits =
          aacDecoder_drcMarkPayload(self->hDrcInfo, hBs, MPEG_DRC_EXT_DATA);

      if (readBits > *count) { /* Read too much. Something went wrong! */
        error = AAC_DEC_PARSE_ERROR;
      }
      *count -= readBits;
    } break;
    case EXT_LDSAC_DATA:
    case EXT_SAC_DATA:
      /* Read MPEG Surround Extension payload */
      {
        int err, mpsSampleRate, mpsFrameSize;

        if (self->flags[0] & AC_PS_PRESENT) {
          error = AAC_DEC_PARSE_ERROR;
          goto bail;
        }

        /* Handle SBR dual rate case */
        if (self->streamInfo.extSamplingRate != 0) {
          mpsSampleRate = self->streamInfo.extSamplingRate;
          mpsFrameSize = self->streamInfo.aacSamplesPerFrame *
                         (self->streamInfo.extSamplingRate /
                          self->streamInfo.aacSampleRate);
        } else {
          mpsSampleRate = self->streamInfo.aacSampleRate;
          mpsFrameSize = self->streamInfo.aacSamplesPerFrame;
        }
        /* Setting of internal MPS state; may be reset in
           CAacDecoder_SyncQmfMode if decoder is unable to decode with user
           defined qmfMode */
        if (!(self->flags[0] & (AC_USAC | AC_RSVD50 | AC_ELD))) {
          self->mpsEnableCurr = self->mpsEnableUser;
        }
        if (self->mpsEnableCurr) {
          if (!self->qmfDomain.globalConf.qmfDomainExplicitConfig) {
            /* if not done yet, allocate full MPEG Surround decoder instance */
            if (mpegSurroundDecoder_IsFullMpegSurroundDecoderInstanceAvailable(
                    (CMpegSurroundDecoder *)self->pMpegSurroundDecoder) ==
                SAC_INSTANCE_NOT_FULL_AVAILABLE) {
              if (mpegSurroundDecoder_Open(
                      (CMpegSurroundDecoder **)&self->pMpegSurroundDecoder, -1,
                      &self->qmfDomain)) {
                return AAC_DEC_OUT_OF_MEMORY;
              }
            }
          }
          err = mpegSurroundDecoder_Parse(
              (CMpegSurroundDecoder *)self->pMpegSurroundDecoder, hBs, count,
              self->streamInfo.aot, mpsSampleRate, mpsFrameSize,
              self->flags[0] & AC_INDEP);
          if (err == MPS_OK) {
            self->flags[0] |= AC_MPS_PRESENT;
          } else {
            error = AAC_DEC_PARSE_ERROR;
          }
        }
        /* Skip any trailing bytes */
        FDKpushFor(hBs, *count);
        *count = 0;
      }
      break;

    case EXT_SBR_DATA_CRC:
      crcFlag = 1;
      FDK_FALLTHROUGH;
    case EXT_SBR_DATA:
      if (IS_CHANNEL_ELEMENT(previous_element)) {
        SBR_ERROR sbrError;
        UCHAR configMode = 0;
        UCHAR configChanged = 0;

        CAacDecoder_SyncQmfMode(self);

        configMode |= AC_CM_ALLOC_MEM;

        sbrError = sbrDecoder_InitElement(
            self->hSbrDecoder, self->streamInfo.aacSampleRate,
            self->streamInfo.extSamplingRate,
            self->streamInfo.aacSamplesPerFrame, self->streamInfo.aot,
            previous_element, elIndex,
            2, /* Signalize that harmonicSBR shall be ignored in the config
                  change detection */
            0, configMode, &configChanged, self->downscaleFactor);

        if (sbrError == SBRDEC_OK) {
          sbrError = sbrDecoder_Parse(self->hSbrDecoder, hBs,
                                      self->pDrmBsBuffer, self->drmBsBufferSize,
                                      count, *count, crcFlag, previous_element,
                                      elIndex, self->flags[0], self->elFlags);
          /* Enable SBR for implicit SBR signalling but only if no severe error
           * happend. */
          if ((sbrError == SBRDEC_OK) || (sbrError == SBRDEC_PARSE_ERROR)) {
            self->sbrEnabled = 1;
          }
        } else {
          /* Do not try to apply SBR because initializing the element failed. */
          self->sbrEnabled = 0;
        }
        /* Citation from ISO/IEC 14496-3 chapter 4.5.2.1.5.2
        Fill elements containing an extension_payload() with an extension_type
        of EXT_SBR_DATA or EXT_SBR_DATA_CRC shall not contain any other
        extension_payload of any other extension_type.
        */
        if (fIsFillElement) {
          FDKpushBiDirectional(hBs, *count);
          *count = 0;
        } else {
          /* If this is not a fill element with a known length, we are screwed
           * and further parsing makes no sense. */
          if (sbrError != SBRDEC_OK) {
            self->frameOK = 0;
          }
        }
      } else {
        error = AAC_DEC_PARSE_ERROR;
      }
      break;

    case EXT_FILL_DATA: {
      int temp;

      temp = FDKreadBits(hBs, 4);
      bytes--;
      if (temp != 0) {
        error = AAC_DEC_PARSE_ERROR;
        break;
      }
      while (bytes > 0) {
        temp = FDKreadBits(hBs, 8);
        bytes--;
        if (temp != 0xa5) {
          error = AAC_DEC_PARSE_ERROR;
          break;
        }
      }
      *count = bytes << 3;
    } break;

    case EXT_DATA_ELEMENT: {
      int dataElementVersion;

      dataElementVersion = FDKreadBits(hBs, 4);
      *count -= 4;
      if (dataElementVersion == 0) /* ANC_DATA */
      {
        int temp, dataElementLength = 0;
        do {
          temp = FDKreadBits(hBs, 8);
          *count -= 8;
          dataElementLength += temp;
        } while (temp == 255);

        CAacDecoder_AncDataParse(&self->ancData, hBs, dataElementLength);
        *count -= (dataElementLength << 3);
      } else {
        /* align = 0 */
        error = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    } break;

    case EXT_DATA_LENGTH:
      if (!fIsFillElement /* Makes no sens to have an additional length in a
                             fill ...   */
          &&
          (self->flags[0] &
           AC_ER)) /* ... element because this extension payload type was ... */
      { /* ... created to circumvent the missing length in ER-Syntax. */
        int bitCnt, len = FDKreadBits(hBs, 4);
        *count -= 4;

        if (len == 15) {
          int add_len = FDKreadBits(hBs, 8);
          *count -= 8;
          len += add_len;

          if (add_len == 255) {
            len += FDKreadBits(hBs, 16);
            *count -= 16;
          }
        }
        len <<= 3;
        bitCnt = len;

        if ((EXT_PAYLOAD_TYPE)FDKreadBits(hBs, 4) == EXT_DATA_LENGTH) {
          /* Check NOTE 2: The extension_payload() included here must
                           not have extension_type == EXT_DATA_LENGTH. */
          error = AAC_DEC_PARSE_ERROR;
          goto bail;
        } else {
          /* rewind and call myself again. */
          FDKpushBack(hBs, 4);

          error = CAacDecoder_ExtPayloadParse(
              self, hBs, &bitCnt, previous_element, elIndex,
              1); /* Treat same as fill element */

          *count -= len - bitCnt;
        }
        /* Note: the fall through in case the if statement above is not taken is
         * intentional. */
        break;
      }
      FDK_FALLTHROUGH;

    case EXT_FIL:

    default:
      /* align = 4 */
      FDKpushFor(hBs, *count);
      *count = 0;
      break;
  }

bail:
  if ((error != AAC_DEC_OK) &&
      fIsFillElement) { /* Skip the remaining extension bytes */
    FDKpushBiDirectional(hBs, *count);
    *count = 0;
    /* Patch error code because decoding can go on. */
    error = AAC_DEC_OK;
    /* Be sure that parsing errors have been stored. */
  }
  return error;
}

static AAC_DECODER_ERROR aacDecoder_ParseExplicitMpsAndSbr(
    HANDLE_AACDECODER self, HANDLE_FDK_BITSTREAM bs,
    const MP4_ELEMENT_ID previous_element, const int previous_element_index,
    const int element_index, const int el_cnt[]) {
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;
  INT bitCnt = 0;

  /* get the remaining bits of this frame */
  bitCnt = transportDec_GetAuBitsRemaining(self->hInput, 0);

  if ((self->flags[0] & AC_SBR_PRESENT) &&
      (self->flags[0] & (AC_USAC | AC_RSVD50 | AC_ELD | AC_DRM))) {
    SBR_ERROR err = SBRDEC_OK;
    int chElIdx, numChElements = el_cnt[ID_SCE] + el_cnt[ID_CPE] +
                                 el_cnt[ID_LFE] + el_cnt[ID_USAC_SCE] +
                                 el_cnt[ID_USAC_CPE] + el_cnt[ID_USAC_LFE];
    INT bitCntTmp = bitCnt;

    if (self->flags[0] & AC_USAC) {
      chElIdx = numChElements - 1;
    } else {
      chElIdx = 0; /* ELD case */
    }

    for (; chElIdx < numChElements; chElIdx += 1) {
      MP4_ELEMENT_ID sbrType;
      SBR_ERROR errTmp;
      if (self->flags[0] & (AC_USAC)) {
        FDK_ASSERT((self->elements[element_index] == ID_USAC_SCE) ||
                   (self->elements[element_index] == ID_USAC_CPE));
        sbrType = IS_STEREO_SBR(self->elements[element_index],
                                self->usacStereoConfigIndex[element_index])
                      ? ID_CPE
                      : ID_SCE;
      } else
        sbrType = self->elements[chElIdx];
      errTmp = sbrDecoder_Parse(self->hSbrDecoder, bs, self->pDrmBsBuffer,
                                self->drmBsBufferSize, &bitCnt, -1,
                                self->flags[0] & AC_SBRCRC, sbrType, chElIdx,
                                self->flags[0], self->elFlags);
      if (errTmp != SBRDEC_OK) {
        err = errTmp;
        bitCntTmp = bitCnt;
        bitCnt = 0;
      }
    }
    switch (err) {
      case SBRDEC_PARSE_ERROR:
        /* Can not go on parsing because we do not
            know the length of the SBR extension data. */
        FDKpushFor(bs, bitCntTmp);
        bitCnt = 0;
        break;
      case SBRDEC_OK:
        self->sbrEnabled = 1;
        break;
      default:
        self->frameOK = 0;
        break;
    }
  }

  if ((bitCnt > 0) && (self->flags[0] & (AC_USAC | AC_RSVD50))) {
    if ((self->flags[0] & AC_MPS_PRESENT) ||
        (self->elFlags[element_index] & AC_EL_USAC_MPS212)) {
      int err;

      err = mpegSurroundDecoder_ParseNoHeader(
          (CMpegSurroundDecoder *)self->pMpegSurroundDecoder, bs, &bitCnt,
          self->flags[0] & AC_INDEP);
      if (err != MPS_OK) {
        self->frameOK = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
      }
    }
  }

  if (self->flags[0] & AC_DRM) {
    if ((bitCnt = (INT)FDKgetValidBits(bs)) != 0) {
      FDKpushBiDirectional(bs, bitCnt);
    }
  }

  if (!(self->flags[0] & (AC_USAC | AC_RSVD50 | AC_DRM))) {
    while (bitCnt > 7) {
      ErrorStatus = CAacDecoder_ExtPayloadParse(
          self, bs, &bitCnt, previous_element, previous_element_index, 0);
      if (ErrorStatus != AAC_DEC_OK) {
        self->frameOK = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        break;
      }
    }
  }
  return ErrorStatus;
}

/*  Stream Configuration and Information.

    This class holds configuration and information data for a stream to be
   decoded. It provides the calling application as well as the decoder with
   substantial information, e.g. profile, sampling rate, number of channels
   found in the bitstream etc.
*/
static void CStreamInfoInit(CStreamInfo *pStreamInfo) {
  pStreamInfo->aacSampleRate = 0;
  pStreamInfo->profile = -1;
  pStreamInfo->aot = AOT_NONE;

  pStreamInfo->channelConfig = -1;
  pStreamInfo->bitRate = 0;
  pStreamInfo->aacSamplesPerFrame = 0;

  pStreamInfo->extAot = AOT_NONE;
  pStreamInfo->extSamplingRate = 0;

  pStreamInfo->flags = 0;

  pStreamInfo->epConfig = -1; /* default: no ER */

  pStreamInfo->numChannels = 0;
  pStreamInfo->sampleRate = 0;
  pStreamInfo->frameSize = 0;

  pStreamInfo->outputDelay = 0;

  /* DRC */
  pStreamInfo->drcProgRefLev =
      -1; /* set program reference level to not indicated */
  pStreamInfo->drcPresMode = -1; /* default: presentation mode not indicated */

  pStreamInfo->outputLoudness = -1; /* default: no loudness metadata present */
}

/*!
  \brief Initialization of AacDecoderChannelInfo

  The function initializes the pointers to AacDecoderChannelInfo for each
  channel, set the start values for window shape and window sequence of
  overlap&add to zero, set the overlap buffer to zero and initializes the
  pointers to the window coefficients. \param bsFormat is the format of the AAC
  bitstream

  \return  AACDECODER instance
*/
LINKSPEC_CPP HANDLE_AACDECODER CAacDecoder_Open(
    TRANSPORT_TYPE bsFormat) /*!< bitstream format (adif,adts,loas,...). */
{
  HANDLE_AACDECODER self;

  self = GetAacDecoder();
  if (self == NULL) {
    goto bail;
  }

  FDK_QmfDomain_ClearRequested(&self->qmfDomain.globalConf);

  /* Assign channel mapping info arrays (doing so removes dependency of settings
   * header in API header). */
  self->streamInfo.pChannelIndices = self->channelIndices;
  self->streamInfo.pChannelType = self->channelType;
  self->downscaleFactor = 1;
  self->downscaleFactorInBS = 1;

  /* initialize anc data */
  CAacDecoder_AncDataInit(&self->ancData, NULL, 0);

  /* initialize stream info */
  CStreamInfoInit(&self->streamInfo);

  /* initialize progam config */
  CProgramConfig_Init(&self->pce);

  /* initialize error concealment common data */
  CConcealment_InitCommonData(&self->concealCommonData);
  self->concealMethodUser = ConcealMethodNone; /* undefined -> auto mode */

  self->hDrcInfo = GetDrcInfo();
  if (self->hDrcInfo == NULL) {
    goto bail;
  }
  /* Init common DRC structure */
  aacDecoder_drcInit(self->hDrcInfo);
  /* Set default frame delay */
  aacDecoder_drcSetParam(self->hDrcInfo, DRC_BS_DELAY,
                         CConcealment_GetDelay(&self->concealCommonData));
  self->workBufferCore1 = (FIXP_DBL *)GetWorkBufferCore1();

  self->workBufferCore2 = GetWorkBufferCore2();
  if (self->workBufferCore2 == NULL) goto bail;

  /* When RSVD60 is active use dedicated memory for core decoding */
  self->pTimeData2 = GetWorkBufferCore5();
  self->timeData2Size = GetRequiredMemWorkBufferCore5();
  if (self->pTimeData2 == NULL) {
    goto bail;
  }

  return self;

bail:
  CAacDecoder_Close(self);

  return NULL;
}

/* Revert CAacDecoder_Init() */
static void CAacDecoder_DeInit(HANDLE_AACDECODER self,
                               const int subStreamIndex) {
  int ch;
  int aacChannelOffset = 0, aacChannels = (8);
  int numElements = (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1),
      elementOffset = 0;

  if (self == NULL) return;

  {
    self->ascChannels[0] = 0;
    self->elements[0] = ID_END;
  }

  for (ch = aacChannelOffset; ch < aacChannelOffset + aacChannels; ch++) {
    if (self->pAacDecoderChannelInfo[ch] != NULL) {
      if (self->pAacDecoderChannelInfo[ch]->pComStaticData != NULL) {
        if (self->pAacDecoderChannelInfo[ch]
                ->pComStaticData->pWorkBufferCore1 != NULL) {
          if (ch == aacChannelOffset) {
            FreeWorkBufferCore1(&self->pAacDecoderChannelInfo[ch]
                                     ->pComStaticData->pWorkBufferCore1);
          }
        }
        if (self->pAacDecoderChannelInfo[ch]
                ->pComStaticData->cplxPredictionData != NULL) {
          FreeCplxPredictionData(&self->pAacDecoderChannelInfo[ch]
                                      ->pComStaticData->cplxPredictionData);
        }
        /* Avoid double free of linked pComStaticData in case of CPE by settings
         * pointer to NULL. */
        if (ch < (8) - 1) {
          if ((self->pAacDecoderChannelInfo[ch + 1] != NULL) &&
              (self->pAacDecoderChannelInfo[ch + 1]->pComStaticData ==
               self->pAacDecoderChannelInfo[ch]->pComStaticData)) {
            self->pAacDecoderChannelInfo[ch + 1]->pComStaticData = NULL;
          }
        }
        FDKfree(self->pAacDecoderChannelInfo[ch]->pComStaticData);
        self->pAacDecoderChannelInfo[ch]->pComStaticData = NULL;
      }
      if (self->pAacDecoderChannelInfo[ch]->pComData != NULL) {
        /* Avoid double free of linked pComData in case of CPE by settings
         * pointer to NULL. */
        if (ch < (8) - 1) {
          if ((self->pAacDecoderChannelInfo[ch + 1] != NULL) &&
              (self->pAacDecoderChannelInfo[ch + 1]->pComData ==
               self->pAacDecoderChannelInfo[ch]->pComData)) {
            self->pAacDecoderChannelInfo[ch + 1]->pComData = NULL;
          }
        }
        if (ch == aacChannelOffset) {
          FreeWorkBufferCore6(
              (SCHAR **)&self->pAacDecoderChannelInfo[ch]->pComData);
        } else {
          FDKafree(self->pAacDecoderChannelInfo[ch]->pComData);
        }
        self->pAacDecoderChannelInfo[ch]->pComData = NULL;
      }
    }
    if (self->pAacDecoderStaticChannelInfo[ch] != NULL) {
      if (self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer != NULL) {
        FreeOverlapBuffer(
            &self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer);
      }
      if (self->pAacDecoderStaticChannelInfo[ch]->hArCo != NULL) {
        CArco_Destroy(self->pAacDecoderStaticChannelInfo[ch]->hArCo);
      }
      FreeAacDecoderStaticChannelInfo(&self->pAacDecoderStaticChannelInfo[ch]);
    }
    if (self->pAacDecoderChannelInfo[ch] != NULL) {
      FreeAacDecoderChannelInfo(&self->pAacDecoderChannelInfo[ch]);
    }
  }

  {
    int el;
    for (el = elementOffset; el < elementOffset + numElements; el++) {
      if (self->cpeStaticData[el] != NULL) {
        FreeCpePersistentData(&self->cpeStaticData[el]);
      }
    }
  }

  FDK_Delay_Destroy(&self->usacResidualDelay);

  self->aacChannels = 0;
  self->streamInfo.aacSampleRate = 0;
  self->streamInfo.sampleRate = 0;
  /* This samplerate value is checked for configuration change, not the others
   * above. */
  self->samplingRateInfo[subStreamIndex].samplingRate = 0;
}

/*!
 * \brief CAacDecoder_AcceptFlags Accept flags and element flags
 *
 * \param self          [o]   handle to AACDECODER structure
 * \param asc           [i]   handle to ASC structure
 * \param flags         [i]   flags
 * \param elFlags       [i]   pointer to element flags
 * \param streamIndex   [i]   stream index
 * \param elementOffset [i]   element offset
 *
 * \return void
 */
static void CAacDecoder_AcceptFlags(HANDLE_AACDECODER self,
                                    const CSAudioSpecificConfig *asc,
                                    UINT flags, UINT *elFlags, int streamIndex,
                                    int elementOffset) {
  FDKmemcpy(self->elFlags, elFlags, sizeof(self->elFlags));

  self->flags[streamIndex] = flags;
}

/*!
 * \brief CAacDecoder_CtrlCFGChange Set config change parameters.
 *
 * \param self           [i]   handle to AACDECODER structure
 * \param flushStatus    [i]   flush status: on|off
 * \param flushCnt       [i]   flush frame counter
 * \param buildUpStatus  [i]   build up status: on|off
 * \param buildUpCnt     [i]   build up frame counter
 *
 * \return error
 */
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_CtrlCFGChange(HANDLE_AACDECODER self,
                                                         UCHAR flushStatus,
                                                         SCHAR flushCnt,
                                                         UCHAR buildUpStatus,
                                                         SCHAR buildUpCnt) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;

  self->flushStatus = flushStatus;
  self->flushCnt = flushCnt;
  self->buildUpStatus = buildUpStatus;
  self->buildUpCnt = buildUpCnt;

  return (err);
}

/*!
 * \brief CAacDecoder_FreeMem Free config dependent AAC memory.
 *
 * \param self       [i]   handle to AACDECODER structure
 *
 * \return error
 */
LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_FreeMem(HANDLE_AACDECODER self,
                                                   const int subStreamIndex) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;

  CAacDecoder_DeInit(self, subStreamIndex);

  return (err);
}

/* Destroy aac decoder */
LINKSPEC_CPP void CAacDecoder_Close(HANDLE_AACDECODER self) {
  if (self == NULL) return;

  CAacDecoder_DeInit(self, 0);

  {
    int ch;
    for (ch = 0; ch < (8); ch++) {
      if (self->pTimeDataFlush[ch] != NULL) {
        FreeTimeDataFlush(&self->pTimeDataFlush[ch]);
      }
    }
  }

  if (self->hDrcInfo) {
    FreeDrcInfo(&self->hDrcInfo);
  }

  if (self->workBufferCore1 != NULL) {
    FreeWorkBufferCore1((CWorkBufferCore1 **)&self->workBufferCore1);
  }

  /* Free WorkBufferCore2 */
  if (self->workBufferCore2 != NULL) {
    FreeWorkBufferCore2(&self->workBufferCore2);
  }
  if (self->pTimeData2 != NULL) {
    FreeWorkBufferCore5(&self->pTimeData2);
  }

  FDK_QmfDomain_Close(&self->qmfDomain);

  FreeAacDecoder(&self);
}

/*!
  \brief Initialization of decoder instance

  The function initializes the decoder.

  \return  error status: 0 for success, <>0 for unsupported configurations
*/
LINKSPEC_CPP AAC_DECODER_ERROR
CAacDecoder_Init(HANDLE_AACDECODER self, const CSAudioSpecificConfig *asc,
                 UCHAR configMode, UCHAR *configChanged) {
  AAC_DECODER_ERROR err = AAC_DEC_OK;
  INT ascChannels, ascChanged = 0;
  AACDEC_RENDER_MODE initRenderMode = AACDEC_RENDER_INVALID;
  SCHAR usacStereoConfigIndex = -1;
  int usacResidualDelayCompSamples = 0;
  int elementOffset, aacChannelsOffset, aacChannelsOffsetIdx;
  const int streamIndex = 0;
  INT flushChannels = 0;

  UINT flags;
  /* elFlags[(3*MAX_CHANNELS + (MAX_CHANNELS)/2 + 4 * (MAX_TRACKS) + 1]
     where MAX_CHANNELS is (8*2) and MAX_TRACKS is 1 */
  UINT elFlags[(3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)];

  UCHAR sbrEnabled = self->sbrEnabled;
  UCHAR sbrEnabledPrev = self->sbrEnabledPrev;
  UCHAR mpsEnableCurr = self->mpsEnableCurr;

  if (!self) return AAC_DEC_INVALID_HANDLE;

  UCHAR downscaleFactor = self->downscaleFactor;
  UCHAR downscaleFactorInBS = self->downscaleFactorInBS;

  self->aacOutDataHeadroom = (3);

  // set profile and check for supported aot
  // leave profile on default (=-1) for all other supported MPEG-4 aot's except
  // aot=2 (=AAC-LC)
  switch (asc->m_aot) {
    case AOT_AAC_LC:
      self->streamInfo.profile = 1;
      FDK_FALLTHROUGH;
    case AOT_ER_AAC_SCAL:
      if (asc->m_sc.m_gaSpecificConfig.m_layer > 0) {
        /* aac_scalable_extension_element() currently not supported. */
        return AAC_DEC_UNSUPPORTED_FORMAT;
      }
      FDK_FALLTHROUGH;
    case AOT_SBR:
    case AOT_PS:
    case AOT_ER_AAC_LC:
    case AOT_ER_AAC_LD:
    case AOT_DRM_AAC:
    case AOT_DRM_SURROUND:
      initRenderMode = AACDEC_RENDER_IMDCT;
      break;
    case AOT_ER_AAC_ELD:
      initRenderMode = AACDEC_RENDER_ELDFB;
      break;
    case AOT_USAC:
      initRenderMode = AACDEC_RENDER_IMDCT;
      break;
    default:
      return AAC_DEC_UNSUPPORTED_AOT;
  }

  if (CProgramConfig_IsValid(&self->pce) && (asc->m_channelConfiguration > 0)) {
    /* Compare the stored (old) PCE with a default PCE created from the (new)
       channel_config (on a temporal buffer) to find out wheter we can keep it
       (and its metadata) or not. */
    int pceCmpResult;
    C_ALLOC_SCRATCH_START(tmpPce, CProgramConfig, 1);

    CProgramConfig_GetDefault(tmpPce, asc->m_channelConfiguration);
    pceCmpResult = CProgramConfig_Compare(&self->pce, tmpPce);
    if ((pceCmpResult < 0) /* Reset if PCEs are completely different ... */
        ||
        (pceCmpResult > 1)) { /*            ... or have a different layout. */
      CProgramConfig_Init(&self->pce);
    } /* Otherwise keep the PCE (and its metadata). */
    C_ALLOC_SCRATCH_END(tmpPce, CProgramConfig, 1);
  } else {
    CProgramConfig_Init(&self->pce);
  }

  /* set channels */
  switch (asc->m_channelConfiguration) {
    case 0:
      switch (asc->m_aot) {
        case AOT_USAC:
          self->chMapIndex = 0;
          ascChannels = asc->m_sc.m_usacConfig.m_nUsacChannels;
          break;
        default:
          /* get channels from program config (ASC) */
          if (CProgramConfig_IsValid(&asc->m_progrConfigElement)) {
            ascChannels = asc->m_progrConfigElement.NumChannels;
            if (ascChannels > 0) {
              int el_tmp;
              /* valid number of channels -> copy program config element (PCE)
               * from ASC */
              FDKmemcpy(&self->pce, &asc->m_progrConfigElement,
                        sizeof(CProgramConfig));
              /* Built element table */
              el_tmp = CProgramConfig_GetElementTable(
                  &asc->m_progrConfigElement, self->elements, (((8)) + (8)),
                  &self->chMapIndex);
              for (; el_tmp < (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1);
                   el_tmp++) {
                self->elements[el_tmp] = ID_NONE;
              }
            } else {
              return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
            }
          } else {
            self->chMapIndex = 0;
            return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
          }
          break;
      }
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      ascChannels = asc->m_channelConfiguration;
      break;
    case 11:
      ascChannels = 7;
      break;
    case 7:
    case 12:
    case 14:
      ascChannels = 8;
      break;
    default:
      return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  if (asc->m_aot == AOT_USAC) {
    flushChannels = fMin(ascChannels, (8));
    INT numChannel;
    pcmDmx_GetParam(self->hPcmUtils, MIN_NUMBER_OF_OUTPUT_CHANNELS,
                    &numChannel);
    flushChannels = fMin(fMax(numChannel, flushChannels), (8));
  }

  if (IS_USAC(asc->m_aot)) {
    for (int el = 0; el < (INT)asc->m_sc.m_usacConfig.m_usacNumElements; el++) {
      /* fix number of core channels aka ascChannels for stereoConfigIndex = 1
       * cases */
      if (asc->m_sc.m_usacConfig.element[el].m_stereoConfigIndex == 1) {
        ascChannels--; /* stereoConfigIndex == 1 stereo cases do actually
                          contain only a mono core channel. */
      } else if (asc->m_sc.m_usacConfig.element[el].m_stereoConfigIndex == 2) {
        /* In this case it is necessary to follow up the DMX signal delay caused
           by HBE also with the residual signal (2nd core channel). The SBR
           overlap delay is not regarded here, this is handled by the MPS212
           implementation.
        */
        if (asc->m_sc.m_usacConfig.element[el].m_harmonicSBR) {
          usacResidualDelayCompSamples += asc->m_samplesPerFrame;
        }
        if (asc->m_sc.m_usacConfig.m_coreSbrFrameLengthIndex == 4) {
          usacResidualDelayCompSamples +=
              6 * 16; /* difference between 12 SBR
                         overlap slots from SBR and 6
                         slots delayed in MPS212 */
        }
      }
    }
  }

  aacChannelsOffset = 0;
  aacChannelsOffsetIdx = 0;
  elementOffset = 0;
  if ((ascChannels <= 0) || (ascChannels > (8)) ||
      (asc->m_channelConfiguration > AACDEC_MAX_CH_CONF)) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  /* Set syntax flags */
  flags = 0;
  { FDKmemclear(elFlags, sizeof(elFlags)); }

  if ((asc->m_channelConfiguration > 0) || IS_USAC(asc->m_aot)) {
    if (IS_USAC(asc->m_aot)) {
      /* copy pointer to usac config
        (this is preliminary since there's an ongoing discussion about storing
        the config-part of the bitstream rather than the complete decoded
        configuration) */
      self->pUsacConfig[streamIndex] = &asc->m_sc.m_usacConfig;

      /* copy list of elements */
      if (self->pUsacConfig[streamIndex]->m_usacNumElements >
          (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)) {
        goto bail;
      }

      if (self->numUsacElements[streamIndex] !=
          asc->m_sc.m_usacConfig.m_usacNumElements) {
        ascChanged = 1;
      }

      if (configMode & AC_CM_ALLOC_MEM) {
        self->numUsacElements[streamIndex] =
            asc->m_sc.m_usacConfig.m_usacNumElements;
      }

      mpsEnableCurr = 0;
      for (int _el = 0;
           _el < (int)self->pUsacConfig[streamIndex]->m_usacNumElements;
           _el++) {
        int el = _el + elementOffset;
        if (self->elements[el] !=
            self->pUsacConfig[streamIndex]->element[_el].usacElementType) {
          ascChanged = 1;
        }
        if (self->usacStereoConfigIndex[el] !=
            asc->m_sc.m_usacConfig.element[_el].m_stereoConfigIndex) {
          ascChanged = 1;
        }
        if (configMode & AC_CM_ALLOC_MEM) {
          self->elements[el] =
              self->pUsacConfig[streamIndex]->element[_el].usacElementType;
          /* for Unified Stereo Coding */
          self->usacStereoConfigIndex[el] =
              asc->m_sc.m_usacConfig.element[_el].m_stereoConfigIndex;
          if (self->elements[el] == ID_USAC_CPE) {
            mpsEnableCurr |= self->usacStereoConfigIndex[el] ? 1 : 0;
          }
        }

        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].m_noiseFilling)
                           ? AC_EL_USAC_NOISE
                           : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].m_stereoConfigIndex > 0)
                ? AC_EL_USAC_MPS212
                : 0;
        elFlags[el] |= (asc->m_sc.m_usacConfig.element[_el].m_interTes)
                           ? AC_EL_USAC_ITES
                           : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].m_pvc) ? AC_EL_USAC_PVC : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].usacElementType == ID_USAC_LFE)
                ? AC_EL_USAC_LFE
                : 0;
        elFlags[el] |=
            (asc->m_sc.m_usacConfig.element[_el].usacElementType == ID_USAC_LFE)
                ? AC_EL_LFE
                : 0;
        if ((asc->m_sc.m_usacConfig.element[_el].usacElementType ==
             ID_USAC_CPE) &&
            ((self->usacStereoConfigIndex[el] == 0))) {
          elFlags[el] |= AC_EL_USAC_CP_POSSIBLE;
        }
      }

      self->hasAudioPreRoll = 0;
      if (self->pUsacConfig[streamIndex]->m_usacNumElements) {
        self->hasAudioPreRoll = asc->m_sc.m_usacConfig.element[0]
                                    .extElement.usacExtElementHasAudioPreRoll;
      }
      if (configMode & AC_CM_ALLOC_MEM) {
        self->elements[elementOffset +
                       self->pUsacConfig[streamIndex]->m_usacNumElements] =
            ID_END;
      }
    } else {
      /* Initialize constant mappings for channel config 1-7 */
      int i;
      for (i = 0; i < AACDEC_CH_ELEMENTS_TAB_SIZE; i++) {
        self->elements[i] = elementsTab[asc->m_channelConfiguration - 1][i];
      }
      for (; i < (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1); i++) {
        self->elements[i] = ID_NONE;
      }
    }

    {
      int ch;

      for (ch = 0; ch < ascChannels; ch++) {
        self->chMapping[ch] = ch;
      }
      for (; ch < (8); ch++) {
        self->chMapping[ch] = 255;
      }
    }

    self->chMapIndex = asc->m_channelConfiguration;
  } else {
    if (CProgramConfig_IsValid(&asc->m_progrConfigElement)) {
      /* Set matrix mixdown infos if available from PCE. */
      pcmDmx_SetMatrixMixdownFromPce(
          self->hPcmUtils, asc->m_progrConfigElement.MatrixMixdownIndexPresent,
          asc->m_progrConfigElement.MatrixMixdownIndex,
          asc->m_progrConfigElement.PseudoSurroundEnable);
    }
  }

  self->streamInfo.channelConfig = asc->m_channelConfiguration;

  if (self->streamInfo.aot != asc->m_aot) {
    if (configMode & AC_CM_ALLOC_MEM) {
      self->streamInfo.aot = asc->m_aot;
    }
    ascChanged = 1;
  }

  if (asc->m_aot == AOT_ER_AAC_ELD &&
      asc->m_sc.m_eldSpecificConfig.m_downscaledSamplingFrequency != 0) {
    if (self->samplingRateInfo[0].samplingRate !=
            asc->m_sc.m_eldSpecificConfig.m_downscaledSamplingFrequency ||
        self->samplingRateInfo[0].samplingRate * self->downscaleFactor !=
            asc->m_samplingFrequency) {
      /* get downscaledSamplingFrequency from ESC and compute the downscale
       * factor */
      downscaleFactorInBS =
          asc->m_samplingFrequency /
          asc->m_sc.m_eldSpecificConfig.m_downscaledSamplingFrequency;
      if ((downscaleFactorInBS == 1 || downscaleFactorInBS == 2 ||
           (downscaleFactorInBS == 3 &&
            asc->m_sc.m_eldSpecificConfig.m_frameLengthFlag) ||
           downscaleFactorInBS == 4) &&
          ((asc->m_samplingFrequency %
            asc->m_sc.m_eldSpecificConfig.m_downscaledSamplingFrequency) ==
           0)) {
        downscaleFactor = downscaleFactorInBS;
      } else {
        downscaleFactorInBS = 1;
        downscaleFactor = 1;
      }
    }
  } else {
    downscaleFactorInBS = 1;
    downscaleFactor = 1;
  }

  if (self->downscaleFactorInBS != downscaleFactorInBS) {
    if (configMode & AC_CM_ALLOC_MEM) {
      self->downscaleFactorInBS = downscaleFactorInBS;
      self->downscaleFactor = downscaleFactor;
    }
    ascChanged = 1;
  }

  if ((INT)asc->m_samplesPerFrame % downscaleFactor != 0) {
    return AAC_DEC_UNSUPPORTED_SAMPLINGRATE; /* frameSize/dsf must be an integer
                                                number */
  }

  self->streamInfo.bitRate = 0;

  if (asc->m_aot == AOT_ER_AAC_ELD) {
    if (self->useLdQmfTimeAlign !=
        asc->m_sc.m_eldSpecificConfig.m_useLdQmfTimeAlign) {
      ascChanged = 1;
    }
    if (configMode & AC_CM_ALLOC_MEM) {
      self->useLdQmfTimeAlign =
          asc->m_sc.m_eldSpecificConfig.m_useLdQmfTimeAlign;
    }
    if (sbrEnabled != asc->m_sbrPresentFlag) {
      ascChanged = 1;
    }
  }

  self->streamInfo.extAot = asc->m_extensionAudioObjectType;
  if (self->streamInfo.extSamplingRate !=
      (INT)asc->m_extensionSamplingFrequency) {
    ascChanged = 1;
  }
  if (configMode & AC_CM_ALLOC_MEM) {
    self->streamInfo.extSamplingRate = asc->m_extensionSamplingFrequency;
  }
  flags |= (asc->m_sbrPresentFlag) ? AC_SBR_PRESENT : 0;
  flags |= (asc->m_psPresentFlag) ? AC_PS_PRESENT : 0;
  if (asc->m_sbrPresentFlag) {
    sbrEnabled = 1;
    sbrEnabledPrev = 1;
  } else {
    sbrEnabled = 0;
    sbrEnabledPrev = 0;
  }
  if (sbrEnabled && asc->m_extensionSamplingFrequency) {
    if (downscaleFactor != 1 && (downscaleFactor)&1) {
      return AAC_DEC_UNSUPPORTED_SAMPLINGRATE; /* SBR needs an even downscale
                                                  factor */
    }
    if (configMode & AC_CM_ALLOC_MEM) {
      self->streamInfo.extSamplingRate =
          self->streamInfo.extSamplingRate / self->downscaleFactor;
    }
  }
  if ((asc->m_aot == AOT_AAC_LC) && (asc->m_sbrPresentFlag == 1) &&
      (asc->m_extensionSamplingFrequency > (2 * asc->m_samplingFrequency))) {
    return AAC_DEC_UNSUPPORTED_SAMPLINGRATE; /* Core decoder supports at most a
                                                1:2 upsampling for HE-AAC and
                                                HE-AACv2 */
  }

  /* --------- vcb11 ------------ */
  flags |= (asc->m_vcb11Flag) ? AC_ER_VCB11 : 0;

  /* ---------- rvlc ------------ */
  flags |= (asc->m_rvlcFlag) ? AC_ER_RVLC : 0;

  /* ----------- hcr ------------ */
  flags |= (asc->m_hcrFlag) ? AC_ER_HCR : 0;

  if (asc->m_aot == AOT_ER_AAC_ELD) {
    mpsEnableCurr = 0;
    flags |= AC_ELD;
    flags |= (asc->m_sbrPresentFlag)
                 ? AC_SBR_PRESENT
                 : 0; /* Need to set the SBR flag for backward-compatibility
                               reasons. Even if SBR is not supported. */
    flags |= (asc->m_sc.m_eldSpecificConfig.m_sbrCrcFlag) ? AC_SBRCRC : 0;
    flags |= (asc->m_sc.m_eldSpecificConfig.m_useLdQmfTimeAlign)
                 ? AC_MPS_PRESENT
                 : 0;
    if (self->mpsApplicable) {
      mpsEnableCurr = asc->m_sc.m_eldSpecificConfig.m_useLdQmfTimeAlign;
    }
  }
  flags |= (asc->m_aot == AOT_ER_AAC_LD) ? AC_LD : 0;
  flags |= (asc->m_epConfig >= 0) ? AC_ER : 0;

  if (asc->m_aot == AOT_USAC) {
    flags |= AC_USAC;
    flags |= (asc->m_sc.m_usacConfig.element[0].m_stereoConfigIndex > 0)
                 ? AC_MPS_PRESENT
                 : 0;
  }
  if (asc->m_aot == AOT_DRM_AAC) {
    flags |= AC_DRM | AC_SBRCRC | AC_SCALABLE;
  }
  if (asc->m_aot == AOT_DRM_SURROUND) {
    flags |= AC_DRM | AC_SBRCRC | AC_SCALABLE | AC_MPS_PRESENT;
    FDK_ASSERT(!asc->m_psPresentFlag);
  }
  if ((asc->m_aot == AOT_AAC_SCAL) || (asc->m_aot == AOT_ER_AAC_SCAL)) {
    flags |= AC_SCALABLE;
  }

  if ((asc->m_epConfig >= 0) && (asc->m_channelConfiguration <= 0)) {
    /* we have to know the number of channels otherwise no decoding is possible
     */
    return AAC_DEC_UNSUPPORTED_ER_FORMAT;
  }

  self->streamInfo.epConfig = asc->m_epConfig;
  /* self->hInput->asc.m_epConfig = asc->m_epConfig; */

  if (asc->m_epConfig > 1) return AAC_DEC_UNSUPPORTED_ER_FORMAT;

  /* Check if samplerate changed. */
  if ((self->samplingRateInfo[streamIndex].samplingRate !=
       asc->m_samplingFrequency) ||
      (self->streamInfo.aacSamplesPerFrame !=
       (INT)asc->m_samplesPerFrame / downscaleFactor)) {
    AAC_DECODER_ERROR error;

    ascChanged = 1;

    if (configMode & AC_CM_ALLOC_MEM) {
      /* Update samplerate info. */
      error = getSamplingRateInfo(
          &self->samplingRateInfo[streamIndex], asc->m_samplesPerFrame,
          asc->m_samplingFrequencyIndex, asc->m_samplingFrequency);
      if (error != AAC_DEC_OK) {
        return error;
      }
      self->streamInfo.aacSampleRate =
          self->samplingRateInfo[0].samplingRate / self->downscaleFactor;
      self->streamInfo.aacSamplesPerFrame =
          asc->m_samplesPerFrame / self->downscaleFactor;
      if (self->streamInfo.aacSampleRate <= 0) {
        return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
      }
    }
  }

  /* Check if amount of channels has changed. */
  if (self->ascChannels[streamIndex] != ascChannels) {
    ascChanged = 1;
  }

  /* detect config change */
  if (configMode & AC_CM_DET_CFG_CHANGE) {
    if (ascChanged != 0) {
      *configChanged = 1;
    }

    CAacDecoder_AcceptFlags(self, asc, flags, elFlags, streamIndex,
                            elementOffset);

    return err;
  }

  /* set AC_USAC_SCFGI3 globally if any usac element uses */
  switch (asc->m_aot) {
    case AOT_USAC:
      if (sbrEnabled) {
        for (int _el = 0;
             _el < (int)self->pUsacConfig[streamIndex]->m_usacNumElements;
             _el++) {
          int el = elementOffset + _el;
          if (IS_USAC_CHANNEL_ELEMENT(self->elements[el])) {
            if (usacStereoConfigIndex < 0) {
              usacStereoConfigIndex = self->usacStereoConfigIndex[el];
            } else {
              if ((usacStereoConfigIndex != self->usacStereoConfigIndex[el]) ||
                  (self->usacStereoConfigIndex[el] > 0)) {
                goto bail;
              }
            }
          }
        }

        if (usacStereoConfigIndex < 0) {
          goto bail;
        }

        if (usacStereoConfigIndex == 3) {
          flags |= AC_USAC_SCFGI3;
        }
      }
      break;
    default:
      break;
  }

  if (*configChanged) {
    /* Set up QMF domain for AOTs with explicit signalling of SBR and or MPS.
       This is to be able to play out the first frame alway with the correct
       frame size and sampling rate even in case of concealment.
    */
    switch (asc->m_aot) {
      case AOT_USAC:
        if (sbrEnabled) {
          const UCHAR map_sbrRatio_2_nAnaBands[] = {16, 24, 32};

          FDK_ASSERT(asc->m_sc.m_usacConfig.m_sbrRatioIndex > 0);
          FDK_ASSERT(streamIndex == 0);

          self->qmfDomain.globalConf.nInputChannels_requested = ascChannels;
          self->qmfDomain.globalConf.nOutputChannels_requested =
              (usacStereoConfigIndex == 1) ? 2 : ascChannels;
          self->qmfDomain.globalConf.flags_requested = 0;
          self->qmfDomain.globalConf.nBandsAnalysis_requested =
              map_sbrRatio_2_nAnaBands[asc->m_sc.m_usacConfig.m_sbrRatioIndex -
                                       1];
          self->qmfDomain.globalConf.nBandsSynthesis_requested = 64;
          self->qmfDomain.globalConf.nQmfTimeSlots_requested =
              (asc->m_sc.m_usacConfig.m_sbrRatioIndex == 1) ? 64 : 32;
          self->qmfDomain.globalConf.nQmfOvTimeSlots_requested =
              (asc->m_sc.m_usacConfig.m_sbrRatioIndex == 1) ? 12 : 6;
          self->qmfDomain.globalConf.nQmfProcBands_requested = 64;
          self->qmfDomain.globalConf.nQmfProcChannels_requested = 1;
          self->qmfDomain.globalConf.parkChannel =
              (usacStereoConfigIndex == 3) ? 1 : 0;
          self->qmfDomain.globalConf.parkChannel_requested =
              (usacStereoConfigIndex == 3) ? 1 : 0;
          self->qmfDomain.globalConf.qmfDomainExplicitConfig = 1;
        }
        break;
      case AOT_ER_AAC_ELD:
        if (mpsEnableCurr &&
            asc->m_sc.m_eldSpecificConfig.m_useLdQmfTimeAlign) {
          SAC_INPUT_CONFIG sac_interface = (sbrEnabled && self->hSbrDecoder)
                                               ? SAC_INTERFACE_QMF
                                               : SAC_INTERFACE_TIME;
          mpegSurroundDecoder_ConfigureQmfDomain(
              (CMpegSurroundDecoder *)self->pMpegSurroundDecoder, sac_interface,
              (UINT)self->streamInfo.aacSampleRate, asc->m_aot);
          self->qmfDomain.globalConf.qmfDomainExplicitConfig = 1;
        }
        break;
      default:
        self->qmfDomain.globalConf.qmfDomainExplicitConfig =
            0; /* qmfDomain is initialized by SBR and MPS init functions if
                  required */
        break;
    }

    /* Allocate all memory structures for each channel */
    {
      int ch = aacChannelsOffset;
      for (int _ch = 0; _ch < ascChannels; _ch++) {
        if (ch >= (8)) {
          goto bail;
        }
        self->pAacDecoderChannelInfo[ch] = GetAacDecoderChannelInfo(ch);
        /* This is temporary until the DynamicData is split into two or more
           regions! The memory could be reused after completed core decoding. */
        if (self->pAacDecoderChannelInfo[ch] == NULL) {
          goto bail;
        }
        ch++;
      }

      int chIdx = aacChannelsOffsetIdx;
      ch = aacChannelsOffset;
      int _numElements;
      _numElements = (((8)) + (8));
      if (flags & (AC_RSV603DA | AC_USAC)) {
        _numElements = (int)asc->m_sc.m_usacConfig.m_usacNumElements;
      }
      for (int _el = 0; _el < _numElements; _el++) {
        int el_channels = 0;
        int el = elementOffset + _el;

        if (flags &
            (AC_ER | AC_LD | AC_ELD | AC_RSV603DA | AC_USAC | AC_RSVD50)) {
          if (ch >= ascChannels) {
            break;
          }
        }

        switch (self->elements[el]) {
          case ID_SCE:
          case ID_CPE:
          case ID_LFE:
          case ID_USAC_SCE:
          case ID_USAC_CPE:
          case ID_USAC_LFE:

            el_channels = CAacDecoder_GetELChannels(
                self->elements[el], self->usacStereoConfigIndex[el]);

            {
              self->pAacDecoderChannelInfo[ch]->pComStaticData =
                  (CAacDecoderCommonStaticData *)FDKcalloc(
                      1, sizeof(CAacDecoderCommonStaticData));
              if (self->pAacDecoderChannelInfo[ch]->pComStaticData == NULL) {
                goto bail;
              }
              if (ch == aacChannelsOffset) {
                self->pAacDecoderChannelInfo[ch]->pComData =
                    (CAacDecoderCommonData *)GetWorkBufferCore6();
                self->pAacDecoderChannelInfo[ch]
                    ->pComStaticData->pWorkBufferCore1 = GetWorkBufferCore1();
              } else {
                self->pAacDecoderChannelInfo[ch]->pComData =
                    (CAacDecoderCommonData *)FDKaalloc(
                        sizeof(CAacDecoderCommonData), ALIGNMENT_DEFAULT);
                self->pAacDecoderChannelInfo[ch]
                    ->pComStaticData->pWorkBufferCore1 =
                    self->pAacDecoderChannelInfo[aacChannelsOffset]
                        ->pComStaticData->pWorkBufferCore1;
              }
              if ((self->pAacDecoderChannelInfo[ch]->pComData == NULL) ||
                  (self->pAacDecoderChannelInfo[ch]
                       ->pComStaticData->pWorkBufferCore1 == NULL)) {
                goto bail;
              }
              self->pAacDecoderChannelInfo[ch]->pDynData =
                  &(self->pAacDecoderChannelInfo[ch]
                        ->pComData->pAacDecoderDynamicData[0]);
              self->pAacDecoderChannelInfo[ch]->pSpectralCoefficient =
                  (SPECTRAL_PTR)&self->workBufferCore2[ch * 1024];

              if (el_channels == 2) {
                if (ch >= (8) - 1) {
                  return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
                }
                self->pAacDecoderChannelInfo[ch + 1]->pComData =
                    self->pAacDecoderChannelInfo[ch]->pComData;
                self->pAacDecoderChannelInfo[ch + 1]->pComStaticData =
                    self->pAacDecoderChannelInfo[ch]->pComStaticData;
                self->pAacDecoderChannelInfo[ch + 1]
                    ->pComStaticData->pWorkBufferCore1 =
                    self->pAacDecoderChannelInfo[ch]
                        ->pComStaticData->pWorkBufferCore1;
                self->pAacDecoderChannelInfo[ch + 1]->pDynData =
                    &(self->pAacDecoderChannelInfo[ch]
                          ->pComData->pAacDecoderDynamicData[1]);
                self->pAacDecoderChannelInfo[ch + 1]->pSpectralCoefficient =
                    (SPECTRAL_PTR)&self->workBufferCore2[(ch + 1) * 1024];
              }

              ch += el_channels;
            }
            chIdx += el_channels;
            break;

          default:
            break;
        }

        if (self->elements[el] == ID_END) {
          break;
        }

        el++;
      }

      chIdx = aacChannelsOffsetIdx;
      ch = aacChannelsOffset;
      for (int _ch = 0; _ch < ascChannels; _ch++) {
        /* Allocate persistent channel memory */
        {
          self->pAacDecoderStaticChannelInfo[ch] =
              GetAacDecoderStaticChannelInfo(ch);
          if (self->pAacDecoderStaticChannelInfo[ch] == NULL) {
            goto bail;
          }
          self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer =
              GetOverlapBuffer(ch); /* This area size depends on the AOT */
          if (self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer == NULL) {
            goto bail;
          }
          if (flags & (AC_USAC | AC_RSVD50 | AC_RSV603DA /*|AC_BSAC*/)) {
            self->pAacDecoderStaticChannelInfo[ch]->hArCo = CArco_Create();
            if (self->pAacDecoderStaticChannelInfo[ch]->hArCo == NULL) {
              goto bail;
            }
          }

          if (!(flags & (AC_USAC | AC_RSV603DA))) {
            CPns_UpdateNoiseState(
                &self->pAacDecoderChannelInfo[ch]->data.aac.PnsData,
                &self->pAacDecoderStaticChannelInfo[ch]->pnsCurrentSeed,
                self->pAacDecoderChannelInfo[ch]->pComData->pnsRandomSeed);
          }
          ch++;
        }
        chIdx++;
      }

      if (flags & AC_USAC) {
        for (int _ch = 0; _ch < flushChannels; _ch++) {
          ch = aacChannelsOffset + _ch;
          if (self->pTimeDataFlush[ch] == NULL) {
            self->pTimeDataFlush[ch] = GetTimeDataFlush(ch);
            if (self->pTimeDataFlush[ch] == NULL) {
              goto bail;
            }
          }
        }
      }

      if (flags & (AC_USAC | AC_RSV603DA)) {
        int complexStereoPredPossible = 0;
        ch = aacChannelsOffset;
        chIdx = aacChannelsOffsetIdx;
        for (int _el2 = 0; _el2 < (int)asc->m_sc.m_usacConfig.m_usacNumElements;
             _el2++) {
          int el2 = elementOffset + _el2;
          int elCh = 0, ch2;

          if ((self->elements[el2] == ID_USAC_CPE) &&
              !(self->usacStereoConfigIndex[el2] == 1)) {
            elCh = 2;
          } else if (IS_CHANNEL_ELEMENT(self->elements[el2])) {
            elCh = 1;
          }

          if (elFlags[el2] & AC_EL_USAC_CP_POSSIBLE) {
            complexStereoPredPossible = 1;
            if (self->cpeStaticData[el2] == NULL) {
              self->cpeStaticData[el2] = GetCpePersistentData();
              if (self->cpeStaticData[el2] == NULL) {
                goto bail;
              }
            }
          }

          for (ch2 = 0; ch2 < elCh; ch2++) {
            /* Hook element specific cpeStaticData into channel specific
             * aacDecoderStaticChannelInfo */
            self->pAacDecoderStaticChannelInfo[ch]->pCpeStaticData =
                self->cpeStaticData[el2];
            if (self->pAacDecoderStaticChannelInfo[ch]->pCpeStaticData !=
                NULL) {
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData
                  .spectralCoeffs[ch2] =
                  self->pAacDecoderStaticChannelInfo[ch]
                      ->concealmentInfo.spectralCoefficient;
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.specScale[ch2] =
                  self->pAacDecoderStaticChannelInfo[ch]
                      ->concealmentInfo.specScale;
              self->pAacDecoderStaticChannelInfo[ch]
                  ->pCpeStaticData->jointStereoPersistentData.scratchBuffer =
                  (FIXP_DBL *)self->pTimeData2;
            }
            chIdx++;
            ch++;
          } /* for each channel in current element */
          if (complexStereoPredPossible && (elCh == 2)) {
            /* needed once for all channels */
            if (self->pAacDecoderChannelInfo[ch - 1]
                    ->pComStaticData->cplxPredictionData == NULL) {
              self->pAacDecoderChannelInfo[ch - 1]
                  ->pComStaticData->cplxPredictionData =
                  GetCplxPredictionData();
            }
            if (self->pAacDecoderChannelInfo[ch - 1]
                    ->pComStaticData->cplxPredictionData == NULL) {
              goto bail;
            }
          }
          if (elCh > 0) {
            self->pAacDecoderStaticChannelInfo[ch - elCh]->nfRandomSeed =
                (ULONG)0x3039;
            if (self->elements[el2] == ID_USAC_CPE) {
              if (asc->m_sc.m_usacConfig.element[el2].m_stereoConfigIndex !=
                  1) {
                self->pAacDecoderStaticChannelInfo[ch - elCh + 1]
                    ->nfRandomSeed = (ULONG)0x10932;
              }
            }
          }
        } /* for each element */
      }

      if (ascChannels != self->aacChannels) {
        /* Make allocated channel count persistent in decoder context. */
        self->aacChannels = aacChannelsOffset + ch;
      }
    }

    if (usacResidualDelayCompSamples) {
      INT delayErr = FDK_Delay_Create(&self->usacResidualDelay,
                                      (USHORT)usacResidualDelayCompSamples, 1);
      if (delayErr) {
        goto bail;
      }
    }

    /* Make amount of signalled channels persistent in decoder context. */
    self->ascChannels[streamIndex] = ascChannels;
    /* Init the previous channel count values. This is required to avoid a
       mismatch of memory accesses in the error concealment module and the
       allocated channel structures in this function. */
    self->aacChannelsPrev = 0;
  }

  if (self->pAacDecoderChannelInfo[0] != NULL) {
    self->pDrmBsBuffer = self->pAacDecoderChannelInfo[0]
                             ->pComStaticData->pWorkBufferCore1->DrmBsBuffer;
    self->drmBsBufferSize = DRM_BS_BUFFER_SIZE;
  }

  /* Update structures */
  if (*configChanged) {
    /* Things to be done for each channel, which do not involve allocating
       memory. Doing these things only on the channels needed for the current
       configuration (ascChannels) could lead to memory access violation later
       (error concealment). */
    int ch = 0;
    int chIdx = 0;
    for (int _ch = 0; _ch < self->ascChannels[streamIndex]; _ch++) {
      switch (self->streamInfo.aot) {
        case AOT_ER_AAC_ELD:
        case AOT_ER_AAC_LD:
          self->pAacDecoderChannelInfo[ch]->granuleLength =
              self->streamInfo.aacSamplesPerFrame;
          break;
        default:
          self->pAacDecoderChannelInfo[ch]->granuleLength =
              self->streamInfo.aacSamplesPerFrame / 8;
          break;
      }
      self->pAacDecoderChannelInfo[ch]->renderMode = initRenderMode;

      mdct_init(&self->pAacDecoderStaticChannelInfo[ch]->IMdct,
                self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer,
                OverlapBufferSize);

      self->pAacDecoderStaticChannelInfo[ch]->last_core_mode = FD_LONG;
      self->pAacDecoderStaticChannelInfo[ch]->last_lpd_mode = 255;

      self->pAacDecoderStaticChannelInfo[ch]->last_tcx_pitch = L_DIV;

      /* Reset DRC control data for this channel */
      aacDecoder_drcInitChannelData(
          &self->pAacDecoderStaticChannelInfo[ch]->drcData);

      /* Delete mixdown metadata from the past */
      pcmDmx_Reset(self->hPcmUtils, PCMDMX_RESET_BS_DATA);

      /* Reset concealment only if ASC changed. Otherwise it will be done with
         any config callback. E.g. every time the LATM SMC is present. */
      CConcealment_InitChannelData(
          &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo,
          &self->concealCommonData, initRenderMode,
          self->streamInfo.aacSamplesPerFrame);
      ch++;
      chIdx++;
    }
  }

  if (*configChanged) {
    int drcDecSampleRate, drcDecFrameSize;

    if (self->streamInfo.extSamplingRate != 0) {
      drcDecSampleRate = self->streamInfo.extSamplingRate;
      drcDecFrameSize = (self->streamInfo.aacSamplesPerFrame *
                         self->streamInfo.extSamplingRate) /
                        self->streamInfo.aacSampleRate;
    } else {
      drcDecSampleRate = self->streamInfo.aacSampleRate;
      drcDecFrameSize = self->streamInfo.aacSamplesPerFrame;
    }

    if (FDK_drcDec_Init(self->hUniDrcDecoder, drcDecFrameSize, drcDecSampleRate,
                        self->aacChannels) != 0)
      goto bail;
  }

  if (*configChanged) {
    if (asc->m_aot == AOT_USAC) {
      aacDecoder_drcDisable(self->hDrcInfo);
    }
  }

  if (asc->m_aot == AOT_USAC) {
    pcmLimiter_SetAttack(self->hLimiter, (5));
    pcmLimiter_SetThreshold(self->hLimiter, FL2FXCONST_DBL(0.89125094f));
  }

  CAacDecoder_AcceptFlags(self, asc, flags, elFlags, streamIndex,
                          elementOffset);
  self->sbrEnabled = sbrEnabled;
  self->sbrEnabledPrev = sbrEnabledPrev;
  self->mpsEnableCurr = mpsEnableCurr;

  /* Update externally visible copy of flags */
  self->streamInfo.flags = self->flags[0];

  return err;

bail:
  CAacDecoder_DeInit(self, 0);
  return AAC_DEC_OUT_OF_MEMORY;
}

LINKSPEC_CPP AAC_DECODER_ERROR CAacDecoder_DecodeFrame(
    HANDLE_AACDECODER self, const UINT flags, PCM_DEC *pTimeData,
    const INT timeDataSize, const int timeDataChannelOffset) {
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;

  CProgramConfig *pce;
  HANDLE_FDK_BITSTREAM bs = transportDec_GetBitstream(self->hInput, 0);

  MP4_ELEMENT_ID type = ID_NONE; /* Current element type */
  INT aacChannels = 0; /* Channel counter for channels found in the bitstream */
  const int streamIndex = 0; /* index of the current substream */

  INT auStartAnchor = (INT)FDKgetValidBits(
      bs); /* AU start bit buffer position for AU byte alignment */

  INT checkSampleRate = self->streamInfo.aacSampleRate;

  INT CConceal_TDFading_Applied[(8)] = {
      0}; /* Initialize status of Time Domain fading */

  if (self->aacChannels <= 0) {
    return AAC_DEC_UNSUPPORTED_CHANNELCONFIG;
  }

  /* Any supported base layer valid AU will require more than 16 bits. */
  if ((transportDec_GetAuBitsRemaining(self->hInput, 0) < 15) &&
      (flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) == 0) {
    self->frameOK = 0;
    ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
  }

  /* Reset Program Config structure */
  pce = &self->pce;
  CProgramConfig_Reset(pce);

  CAacDecoder_AncDataReset(&self->ancData);
  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) &&
      !(self->flags[0] & (AC_USAC | AC_RSV603DA))) {
    int ch;
    if (self->streamInfo.channelConfig == 0) {
      /* Init Channel/Element mapping table */
      for (ch = 0; ch < (8); ch++) {
        self->chMapping[ch] = 255;
      }
      if (!CProgramConfig_IsValid(pce)) {
        int el;
        for (el = 0; el < (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1);
             el++) {
          self->elements[el] = ID_NONE;
        }
      }
    }
  }

  if (self->downscaleFactor > 1 && (self->flags[0] & AC_ELD)) {
    self->flags[0] |= AC_ELD_DOWNSCALE;
  } else {
    self->flags[0] &= ~AC_ELD_DOWNSCALE;
  }
  /* unsupported dsf (aacSampleRate has not yet been divided by dsf) -> divide
   */
  if (self->downscaleFactorInBS > 1 &&
      (self->flags[0] & AC_ELD_DOWNSCALE) == 0) {
    checkSampleRate =
        self->streamInfo.aacSampleRate / self->downscaleFactorInBS;
  }

  /* Check sampling frequency  */
  if (self->streamInfo.aacSampleRate <= 0) {
    /* Instance maybe uninitialized! */
    return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
  }
  switch (checkSampleRate) {
    case 96000:
    case 88200:
    case 64000:
    case 16000:
    case 12000:
    case 11025:
    case 8000:
    case 7350:
    case 48000:
    case 44100:
    case 32000:
    case 24000:
    case 22050:
      break;
    default:
      if (!(self->flags[0] & (AC_USAC | AC_RSVD50 | AC_RSV603DA))) {
        return AAC_DEC_UNSUPPORTED_SAMPLINGRATE;
      }
      break;
  }

  if (flags & AACDEC_CLRHIST) {
    if (!(self->flags[0] & AC_USAC)) {
      int ch;
      /* Clear history */
      for (ch = 0; ch < self->aacChannels; ch++) {
        /* Reset concealment */
        CConcealment_InitChannelData(
            &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo,
            &self->concealCommonData,
            self->pAacDecoderChannelInfo[0]->renderMode,
            self->streamInfo.aacSamplesPerFrame);
        /* Clear overlap-add buffers to avoid clicks. */
        FDKmemclear(self->pAacDecoderStaticChannelInfo[ch]->pOverlapBuffer,
                    OverlapBufferSize * sizeof(FIXP_DBL));
      }
      if (self->streamInfo.channelConfig > 0) {
        /* Declare the possibly adopted old PCE (with outdated metadata)
         * invalid. */
        CProgramConfig_Init(pce);
      }
    }
  }

  int pceRead = 0; /* Flag indicating a PCE in the current raw_data_block() */

  INT hdaacDecoded = 0;
  MP4_ELEMENT_ID previous_element =
      ID_END; /* Last element ID (required for extension payload mapping */
  UCHAR previous_element_index = 0; /* Canonical index of last element */
  int element_count =
      0; /* Element counter for elements found in the bitstream */
  int channel_element_count = 0; /* Channel element counter */
  MP4_ELEMENT_ID
  channel_elements[(3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) +
                    1)];     /* Channel elements in bit stream order. */
  int el_cnt[ID_LAST] = {0}; /* element counter ( robustness ) */
  int element_count_prev_streams =
      0; /* Element count of all previous sub streams. */

  while ((type != ID_END) && (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) &&
         self->frameOK) {
    int el_channels;

    if (!(self->flags[0] &
          (AC_USAC | AC_RSVD50 | AC_RSV603DA | AC_ELD | AC_SCALABLE | AC_ER)))
      type = (MP4_ELEMENT_ID)FDKreadBits(bs, 3);
    else {
      if (element_count >= (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)) {
        self->frameOK = 0;
        ErrorStatus = AAC_DEC_PARSE_ERROR;
        break;
      }
      type = self->elements[element_count];
    }

    if ((self->flags[streamIndex] & (AC_USAC | AC_RSVD50) &&
         element_count == 0) ||
        (self->flags[streamIndex] & AC_RSV603DA)) {
      self->flags[streamIndex] &= ~AC_INDEP;

      if (FDKreadBit(bs)) {
        self->flags[streamIndex] |= AC_INDEP;
      }

      int ch = aacChannels;
      for (int chIdx = aacChannels; chIdx < self->ascChannels[streamIndex];
           chIdx++) {
        {
          /* Robustness check */
          if (ch >= self->aacChannels) {
            return AAC_DEC_UNKNOWN;
          }

          /* if last frame was broken and this frame is no independent frame,
           * correct decoding is impossible we need to trigger concealment */
          if ((CConcealment_GetLastFrameOk(
                   &self->pAacDecoderStaticChannelInfo[ch]->concealmentInfo,
                   1) == 0) &&
              !(self->flags[streamIndex] & AC_INDEP)) {
            self->frameOK = 0;
          }
          ch++;
        }
      }
    }

    if ((INT)FDKgetValidBits(bs) < 0) {
      self->frameOK = 0;
    }

    switch (type) {
      case ID_SCE:
      case ID_CPE:
      case ID_LFE:
      case ID_USAC_SCE:
      case ID_USAC_CPE:
      case ID_USAC_LFE:
        if (element_count >= (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)) {
          self->frameOK = 0;
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          break;
        }

        el_channels = CAacDecoder_GetELChannels(
            type, self->usacStereoConfigIndex[element_count]);

        /*
          Consistency check
         */
        {
          int totalAscChannels = 0;

          for (int i = 0; i < (1 * 1); i++) {
            totalAscChannels += self->ascChannels[i];
          }
          if ((el_cnt[type] >= (totalAscChannels >> (el_channels - 1))) ||
              (aacChannels > (totalAscChannels - el_channels))) {
            ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
            self->frameOK = 0;
            break;
          }
        }

        if (!(self->flags[streamIndex] & (AC_USAC | AC_RSVD50 | AC_RSV603DA))) {
          int ch;
          for (ch = 0; ch < el_channels; ch += 1) {
            CPns_ResetData(&self->pAacDecoderChannelInfo[aacChannels + ch]
                                ->data.aac.PnsData,
                           &self->pAacDecoderChannelInfo[aacChannels + ch]
                                ->pComData->pnsInterChannelData);
          }
        }

        if (self->frameOK) {
          ErrorStatus = CChannelElement_Read(
              bs, &self->pAacDecoderChannelInfo[aacChannels],
              &self->pAacDecoderStaticChannelInfo[aacChannels],
              self->streamInfo.aot, &self->samplingRateInfo[streamIndex],
              self->flags[streamIndex], self->elFlags[element_count],
              self->streamInfo.aacSamplesPerFrame, el_channels,
              self->streamInfo.epConfig, self->hInput);
          if (ErrorStatus != AAC_DEC_OK) {
            self->frameOK = 0;
          }
        }

        if (self->frameOK) {
          /* Lookup the element and decode it only if it belongs to the current
           * program */
          if (CProgramConfig_LookupElement(
                  pce, self->streamInfo.channelConfig,
                  self->pAacDecoderChannelInfo[aacChannels]->ElementInstanceTag,
                  aacChannels, self->chMapping, self->channelType,
                  self->channelIndices, (8), &previous_element_index,
                  self->elements, type)) {
            channel_elements[channel_element_count++] = type;
            aacChannels += el_channels;
          } else {
            self->frameOK = 0;
          }
          /* Create SBR element for SBR for upsampling for LFE elements,
             and if SBR was implicitly signaled, because the first frame(s)
             may not contain SBR payload (broken encoder, bit errors). */
          if (self->frameOK &&
              ((self->flags[streamIndex] & AC_SBR_PRESENT) ||
               (self->sbrEnabled == 1)) &&
              !(self->flags[streamIndex] &
                AC_USAC) /* Is done during explicit config set up */
          ) {
            SBR_ERROR sbrError;
            UCHAR configMode = 0;
            UCHAR configChanged = 0;
            configMode |= AC_CM_ALLOC_MEM;

            sbrError = sbrDecoder_InitElement(
                self->hSbrDecoder, self->streamInfo.aacSampleRate,
                self->streamInfo.extSamplingRate,
                self->streamInfo.aacSamplesPerFrame, self->streamInfo.aot, type,
                previous_element_index, 2, /* Signalize that harmonicSBR shall
                                              be ignored in the config change
                                              detection */
                0, configMode, &configChanged, self->downscaleFactor);
            if (sbrError != SBRDEC_OK) {
              /* Do not try to apply SBR because initializing the element
               * failed. */
              self->sbrEnabled = 0;
            }
          }
        }

        el_cnt[type]++;
        if (self->frameOK && (self->flags[streamIndex] & AC_USAC) &&
            (type == ID_USAC_CPE || type == ID_USAC_SCE)) {
          ErrorStatus = aacDecoder_ParseExplicitMpsAndSbr(
              self, bs, previous_element, previous_element_index, element_count,
              el_cnt);
          if (ErrorStatus != AAC_DEC_OK) {
            self->frameOK = 0;
          }
        }
        break;

      case ID_CCE:
        /*
          Consistency check
        */
        if (el_cnt[type] > self->ascChannels[streamIndex]) {
          ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
          self->frameOK = 0;
          break;
        }

        if (self->frameOK) {
          CAacDecoderCommonData commonData;
          CAacDecoderCommonStaticData commonStaticData;
          CWorkBufferCore1 workBufferCore1;
          commonStaticData.pWorkBufferCore1 = &workBufferCore1;
          /* memory for spectral lines temporal on scratch */
          C_AALLOC_SCRATCH_START(mdctSpec, FIXP_DBL, 1024);

          /* create dummy channel for CCE parsing on stack */
          CAacDecoderChannelInfo tmpAacDecoderChannelInfo,
              *pTmpAacDecoderChannelInfo;

          FDKmemclear(mdctSpec, 1024 * sizeof(FIXP_DBL));

          tmpAacDecoderChannelInfo.pDynData = commonData.pAacDecoderDynamicData;
          tmpAacDecoderChannelInfo.pComData = &commonData;
          tmpAacDecoderChannelInfo.pComStaticData = &commonStaticData;
          tmpAacDecoderChannelInfo.pSpectralCoefficient =
              (SPECTRAL_PTR)mdctSpec;
          /* Assume AAC-LC */
          tmpAacDecoderChannelInfo.granuleLength =
              self->streamInfo.aacSamplesPerFrame / 8;
          /* Reset PNS data. */
          CPns_ResetData(
              &tmpAacDecoderChannelInfo.data.aac.PnsData,
              &tmpAacDecoderChannelInfo.pComData->pnsInterChannelData);
          pTmpAacDecoderChannelInfo = &tmpAacDecoderChannelInfo;
          /* do CCE parsing */
          ErrorStatus = CChannelElement_Read(
              bs, &pTmpAacDecoderChannelInfo, NULL, self->streamInfo.aot,
              &self->samplingRateInfo[streamIndex], self->flags[streamIndex],
              AC_EL_GA_CCE, self->streamInfo.aacSamplesPerFrame, 1,
              self->streamInfo.epConfig, self->hInput);

          C_AALLOC_SCRATCH_END(mdctSpec, FIXP_DBL, 1024);

          if (ErrorStatus) {
            self->frameOK = 0;
          }

          if (self->frameOK) {
            /* Lookup the element and decode it only if it belongs to the
             * current program */
            if (CProgramConfig_LookupElement(
                    pce, self->streamInfo.channelConfig,
                    pTmpAacDecoderChannelInfo->ElementInstanceTag, 0,
                    self->chMapping, self->channelType, self->channelIndices,
                    (8), &previous_element_index, self->elements, type)) {
              /* decoding of CCE not supported */
            } else {
              self->frameOK = 0;
            }
          }
        }
        el_cnt[type]++;
        break;

      case ID_DSE: {
        UCHAR element_instance_tag;

        CDataStreamElement_Read(self, bs, &element_instance_tag, auStartAnchor);

        if (!CProgramConfig_LookupElement(
                pce, self->streamInfo.channelConfig, element_instance_tag, 0,
                self->chMapping, self->channelType, self->channelIndices, (8),
                &previous_element_index, self->elements, type)) {
          /* most likely an error in bitstream occured */
          // self->frameOK = 0;
        }
      } break;

      case ID_PCE: {
        int result = CProgramConfigElement_Read(bs, self->hInput, pce,
                                                self->streamInfo.channelConfig,
                                                auStartAnchor);
        if (result < 0) {
          /* Something went wrong */
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          self->frameOK = 0;
        } else if (result > 1) {
          /* Built element table */
          int elIdx = CProgramConfig_GetElementTable(
              pce, self->elements, (((8)) + (8)), &self->chMapIndex);
          /* Reset the remaining tabs */
          for (; elIdx < (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1);
               elIdx++) {
            self->elements[elIdx] = ID_NONE;
          }
          /* Make new number of channel persistent */
          self->ascChannels[streamIndex] = pce->NumChannels;
          /* If PCE is not first element conceal this frame to avoid
           * inconsistencies */
          if (element_count != 0) {
            self->frameOK = 0;
          }
        }
        pceRead = (result >= 0) ? 1 : 0;
      } break;

      case ID_FIL: {
        int bitCnt = FDKreadBits(bs, 4); /* bs_count */

        if (bitCnt == 15) {
          int esc_count = FDKreadBits(bs, 8); /* bs_esc_count */
          bitCnt = esc_count + 14;
        }

        /* Convert to bits */
        bitCnt <<= 3;

        while (bitCnt > 0) {
          ErrorStatus = CAacDecoder_ExtPayloadParse(
              self, bs, &bitCnt, previous_element, previous_element_index, 1);
          if (ErrorStatus != AAC_DEC_OK) {
            self->frameOK = 0;
            break;
          }
        }
      } break;

      case ID_EXT:
        if (element_count >= (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)) {
          self->frameOK = 0;
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          break;
        }

        ErrorStatus = aacDecoder_ParseExplicitMpsAndSbr(
            self, bs, previous_element, previous_element_index, element_count,
            el_cnt);
        break;

      case ID_USAC_EXT: {
        if ((element_count - element_count_prev_streams) >=
            TP_USAC_MAX_ELEMENTS) {
          self->frameOK = 0;
          ErrorStatus = AAC_DEC_PARSE_ERROR;
          break;
        }
        /* parse extension element payload
           q.v. rsv603daExtElement() ISO/IEC DIS 23008-3  Table 30
           or   UsacExElement() ISO/IEC FDIS 23003-3:2011(E)  Table 21
         */
        int usacExtElementPayloadLength;
        /* int usacExtElementStart, usacExtElementStop; */

        if (FDKreadBit(bs)) {   /* usacExtElementPresent */
          if (FDKreadBit(bs)) { /* usacExtElementUseDefaultLength */
            usacExtElementPayloadLength =
                self->pUsacConfig[streamIndex]
                    ->element[element_count - element_count_prev_streams]
                    .extElement.usacExtElementDefaultLength;
          } else {
            usacExtElementPayloadLength = FDKreadBits(bs, 8);
            if (usacExtElementPayloadLength == (UINT)(1 << 8) - 1) {
              UINT valueAdd = FDKreadBits(bs, 16);
              usacExtElementPayloadLength += (INT)valueAdd - 2;
            }
          }
          if (usacExtElementPayloadLength > 0) {
            int usacExtBitPos;

            if (self->pUsacConfig[streamIndex]
                    ->element[element_count - element_count_prev_streams]
                    .extElement.usacExtElementPayloadFrag) {
              /* usacExtElementStart = */ FDKreadBit(bs);
              /* usacExtElementStop = */ FDKreadBit(bs);
            } else {
              /* usacExtElementStart = 1; */
              /* usacExtElementStop = 1; */
            }

            usacExtBitPos = (INT)FDKgetValidBits(bs);

            USAC_EXT_ELEMENT_TYPE usacExtElementType =
                self->pUsacConfig[streamIndex]
                    ->element[element_count - element_count_prev_streams]
                    .extElement.usacExtElementType;

            switch (usacExtElementType) {
              case ID_EXT_ELE_UNI_DRC: /* uniDrcGain() */
                if (streamIndex == 0) {
                  int drcErr;

                  drcErr = FDK_drcDec_ReadUniDrcGain(self->hUniDrcDecoder, bs);
                  if (drcErr != 0) {
                    ErrorStatus = AAC_DEC_PARSE_ERROR;
                  }
                }
                break;

              default:
                break;
            }

            /* Skip any remaining bits of extension payload */
            usacExtBitPos = (usacExtElementPayloadLength * 8) -
                            (usacExtBitPos - (INT)FDKgetValidBits(bs));
            if (usacExtBitPos < 0) {
              self->frameOK = 0;
              ErrorStatus = AAC_DEC_PARSE_ERROR;
            }
            FDKpushBiDirectional(bs, usacExtBitPos);
          }
        }
      } break;
      case ID_END:
      case ID_USAC_END:
        break;

      default:
        ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
        self->frameOK = 0;
        break;
    }

    previous_element = type;
    element_count++;

  } /* while ( (type != ID_END) ... ) */

  if (!(self->flags[streamIndex] &
        (AC_USAC | AC_RSVD50 | AC_RSV603DA | AC_BSAC | AC_LD | AC_ELD | AC_ER |
         AC_SCALABLE)) &&
      (self->streamInfo.channelConfig == 0) && pce->isValid &&
      (ErrorStatus == AAC_DEC_OK) && self->frameOK &&
      !(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    /* Check whether all PCE listed element instance tags are present in
     * raw_data_block() */
    if (!validateElementInstanceTags(
            &self->pce, self->pAacDecoderChannelInfo, aacChannels,
            channel_elements,
            fMin(channel_element_count, (int)(sizeof(channel_elements) /
                                              sizeof(*channel_elements))))) {
      ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
      self->frameOK = 0;
    }
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    /* float decoder checks if bitsLeft is in range 0-7; only prerollAUs are
     * byteAligned with respect to the first bit */
    /* Byte alignment with respect to the first bit of the raw_data_block(). */
    if (!(self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) ||
        (self->prerollAULength[self->accessUnit]) /* indicates preroll */
    ) {
      FDKbyteAlign(bs, auStartAnchor);
    }

    /* Check if all bits of the raw_data_block() have been read. */
    if (transportDec_GetAuBitsTotal(self->hInput, 0) > 0) {
      INT unreadBits = transportDec_GetAuBitsRemaining(self->hInput, 0);
      /* for pre-roll frames pre-roll length has to be used instead of total AU
       * lenght */
      /* unreadBits regarding preroll bounds */
      if (self->prerollAULength[self->accessUnit]) {
        unreadBits = unreadBits - transportDec_GetAuBitsTotal(self->hInput, 0) +
                     (INT)self->prerollAULength[self->accessUnit];
      }
      if (((self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) &&
           ((unreadBits < 0) || (unreadBits > 7)) &&
           !(self->prerollAULength[self->accessUnit])) ||
          ((!(self->flags[streamIndex] & (AC_RSVD50 | AC_USAC)) ||
            (self->prerollAULength[self->accessUnit])) &&
           (unreadBits != 0))) {
        if ((((unreadBits < 0) || (unreadBits > 7)) && self->frameOK) &&
            ((transportDec_GetFormat(self->hInput) == TT_DRM) &&
             (self->flags[streamIndex] & AC_USAC))) {
          /* Set frame OK because of fill bits. */
          self->frameOK = 1;
        } else {
          self->frameOK = 0;
        }

        /* Do not overwrite current error */
        if (ErrorStatus == AAC_DEC_OK && self->frameOK == 0) {
          ErrorStatus = AAC_DEC_PARSE_ERROR;
        }
        /* Always put the bitbuffer at the right position after the current
         * Access Unit. */
        FDKpushBiDirectional(bs, unreadBits);
      }
    }

    /* Check the last element. The terminator (ID_END) has to be the last one
     * (even if ER syntax is used). */
    if (self->frameOK && type != ID_END) {
      /* Do not overwrite current error */
      if (ErrorStatus == AAC_DEC_OK) {
        ErrorStatus = AAC_DEC_PARSE_ERROR;
      }
      self->frameOK = 0;
    }
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && self->frameOK) {
    channel_elements[channel_element_count++] = ID_END;
  }
  element_count = 0;
  aacChannels = 0;
  type = ID_NONE;
  previous_element_index = 0;

  while (type != ID_END &&
         element_count < (3 * ((8) * 2) + (((8) * 2)) / 2 + 4 * (1) + 1)) {
    int el_channels;

    if ((flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) || !self->frameOK) {
      channel_elements[element_count] = self->elements[element_count];
      if (channel_elements[element_count] == ID_NONE) {
        channel_elements[element_count] = ID_END;
      }
    }

    if (self->flags[streamIndex] & (AC_USAC | AC_RSV603DA | AC_BSAC)) {
      type = self->elements[element_count];
    } else {
      type = channel_elements[element_count];
    }

    if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH)) && self->frameOK) {
      switch (type) {
        case ID_SCE:
        case ID_CPE:
        case ID_LFE:
        case ID_USAC_SCE:
        case ID_USAC_CPE:
        case ID_USAC_LFE:

          el_channels = CAacDecoder_GetELChannels(
              type, self->usacStereoConfigIndex[element_count]);

          if (!hdaacDecoded) {
            if (self->pAacDecoderStaticChannelInfo[aacChannels]
                    ->pCpeStaticData != NULL) {
              self->pAacDecoderStaticChannelInfo[aacChannels]
                  ->pCpeStaticData->jointStereoPersistentData.scratchBuffer =
                  (FIXP_DBL *)pTimeData;
            }
            CChannelElement_Decode(
                &self->pAacDecoderChannelInfo[aacChannels],
                &self->pAacDecoderStaticChannelInfo[aacChannels],
                &self->samplingRateInfo[streamIndex], self->flags[streamIndex],
                self->elFlags[element_count], el_channels);
          }
          aacChannels += el_channels;
          break;
        case ID_NONE:
          type = ID_END;
          break;
        default:
          break;
      }
    }
    element_count++;
  }

  /* More AAC channels than specified by the ASC not allowed. */
  if ((aacChannels == 0 || aacChannels > self->aacChannels) &&
      !(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    /* Do not overwrite current error */
    if (ErrorStatus == AAC_DEC_OK) {
      ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
    }
    self->frameOK = 0;
    aacChannels = 0;
  }

  if (!(flags & (AACDEC_CONCEAL | AACDEC_FLUSH))) {
    if (TRANSPORTDEC_OK != transportDec_CrcCheck(self->hInput)) {
      ErrorStatus = AAC_DEC_CRC_ERROR;
      self->frameOK = 0;
    }
  }

  /* Ensure that in case of concealment a proper error status is set. */
  if ((self->frameOK == 0) && (ErrorStatus == AAC_DEC_OK)) {
    ErrorStatus = AAC_DEC_DECODE_FRAME_ERROR;
  }

  if (self->frameOK && (flags & AACDEC_FLUSH)) {
    aacChannels = self->aacChannelsPrev;
    /* Because the downmix could be active, its necessary to restore the channel
     * type and indices. */
    FDKmemcpy(self->channelType, self->channelTypePrev,
              (8) * sizeof(AUDIO_CHANNEL_TYPE)); /* restore */
    FDKmemcpy(self->channelIndices, self->channelIndicesPrev,
              (8) * sizeof(UCHAR)); /* restore */
    self->sbrEnabled = self->sbrEnabledPrev;
  } else {
    /* store or restore the number of channels and the corresponding info */
    if (self->frameOK && !(flags & AACDEC_CONCEAL)) {
      self->aacChannelsPrev = aacChannels; /* store */
      FDKmemcpy(self->channelTypePrev, self->channelType,
                (8) * sizeof(AUDIO_CHANNEL_TYPE)); /* store */
      FDKmemcpy(self->channelIndicesPrev, self->channelIndices,
                (8) * sizeof(UCHAR)); /* store */
      self->sbrEnabledPrev = self->sbrEnabled;
    } else {
      if (self->aacChannels > 0) {
        if ((self->buildUpStatus == AACDEC_RSV60_BUILD_UP_ON) ||
            (self->buildUpStatus == AACDEC_RSV60_BUILD_UP_ON_IN_BAND) ||
            (self->buildUpStatus == AACDEC_USAC_BUILD_UP_ON)) {
          aacChannels = self->aacChannels;
          self->aacChannelsPrev = aacChannels; /* store */
        } else {
          aacChannels = self->aacChannelsPrev; /* restore */
        }
        FDKmemcpy(self->channelType, self->channelTypePrev,
                  (8) * sizeof(AUDIO_CHANNEL_TYPE)); /* restore */
        FDKmemcpy(self->channelIndices, self->channelIndicesPrev,
                  (8) * sizeof(UCHAR)); /* restore */
        self->sbrEnabled = self->sbrEnabledPrev;
      }
    }
  }

  /* Update number of output channels */
  self->streamInfo.aacNumChannels = aacChannels;

  /* Ensure consistency of IS_OUTPUT_VALID() macro. */
  if (aacChannels == 0) {
    ErrorStatus = AAC_DEC_UNKNOWN;
  }

  if (pceRead == 1 && CProgramConfig_IsValid(pce)) {
    /* Set matrix mixdown infos if available from PCE. */
    pcmDmx_SetMatrixMixdownFromPce(
        self->hPcmUtils, pce->MatrixMixdownIndexPresent,
        pce->MatrixMixdownIndex, pce->PseudoSurroundEnable);
    ;
  }

  /* If there is no valid data to transfrom into time domain, return. */
  if (!IS_OUTPUT_VALID(ErrorStatus)) {
    return ErrorStatus;
  }

  /* Setup the output channel mapping. The table below shows the three
   * possibilities: # | chCfg | PCE | chMapIndex
   *  ---+-------+-----+------------------
   *   1 |  > 0  |  no | chCfg
   *   2 |   0   | yes | cChCfg
   *   3 |   0   |  no | aacChannels || 0
   *  ---+-------+-----+--------+------------------
   *  Where chCfg is the channel configuration index from ASC and cChCfg is a
   * corresponding chCfg derived from a given PCE. The variable aacChannels
   * represents the number of channel found during bitstream decoding. Due to
   * the structure of the mapping table it can only be used for mapping if its
   * value is smaller than 7. Otherwise we use the fallback (0) which is a
   * simple pass-through. The possibility #3 should appear only with MPEG-2
   * (ADTS) streams. This is mode is called "implicit channel mapping".
   */
  if ((self->streamInfo.channelConfig == 0) && !pce->isValid) {
    self->chMapIndex = (aacChannels < 7) ? aacChannels : 0;
  }

  /*
    Inverse transform
  */
  {
    int c, cIdx;
    int mapped, fCopyChMap = 1;
    UCHAR drcChMap[(8)];

    if ((self->streamInfo.channelConfig == 0) && CProgramConfig_IsValid(pce)) {
      /* ISO/IEC 14496-3 says:
           If a PCE is present, the exclude_mask bits correspond to the audio
         channels in the SCE, CPE, CCE and LFE syntax elements in the order of
         their appearance in the PCE. In the case of a CPE, the first
         transmitted mask bit corresponds to the first channel in the CPE, the
         second transmitted mask bit to the second channel. In the case of a
         CCE, a mask bit is transmitted only if the coupling channel is
         specified to be an independently switched coupling channel. Thus we
         have to convert the internal channel mapping from "canonical" MPEG to
         PCE order: */
      UCHAR tmpChMap[(8)];
      if (CProgramConfig_GetPceChMap(pce, tmpChMap, (8)) == 0) {
        for (c = 0; c < aacChannels; c += 1) {
          drcChMap[c] =
              (self->chMapping[c] == 255) ? 255 : tmpChMap[self->chMapping[c]];
        }
        fCopyChMap = 0;
      }
    }
    if (fCopyChMap != 0) {
      FDKmemcpy(drcChMap, self->chMapping, (8) * sizeof(UCHAR));
    }

    /* Extract DRC control data and map it to channels (without bitstream delay)
     */
    mapped = aacDecoder_drcProlog(
        self->hDrcInfo, bs, self->pAacDecoderStaticChannelInfo,
        pce->ElementInstanceTag, drcChMap, aacChannels);
    if (mapped > 0) {
      if (!(self->flags[streamIndex] & (AC_USAC | AC_RSV603DA))) {
        /* If at least one DRC thread has been mapped to a channel there was DRC
         * data in the bitstream. */
        self->flags[streamIndex] |= AC_DRC_PRESENT;
      } else {
        ErrorStatus = AAC_DEC_UNSUPPORTED_FORMAT;
      }
    }
    if (self->flags[streamIndex] & (AC_USAC | AC_RSV603DA)) {
      aacDecoder_drcDisable(self->hDrcInfo);
    }

    /* Create a reverse mapping table */
    UCHAR Reverse_chMapping[((8) * 2)];
    for (c = 0; c < aacChannels; c++) {
      int d;
      for (d = 0; d < aacChannels - 1; d++) {
        if (self->chMapping[d] == c) {
          break;
        }
      }
      Reverse_chMapping[c] = d;
    }

    int el;
    int el_channels;
    c = 0;
    cIdx = 0;
    el_channels = 0;
    for (el = 0; el < element_count; el++) {
      int frameOk_butConceal =
          0; /* Force frame concealment during mute release active state. */
      int concealApplyReturnCode;

      if (self->flags[streamIndex] & (AC_USAC | AC_RSV603DA | AC_BSAC)) {
        type = self->elements[el];
      } else {
        type = channel_elements[el];
      }

      {
        int nElementChannels;

        nElementChannels =
            CAacDecoder_GetELChannels(type, self->usacStereoConfigIndex[el]);

        el_channels += nElementChannels;

        if (nElementChannels == 0) {
          continue;
        }
      }

      int offset;
      int elCh = 0;
      /* "c" iterates in canonical MPEG channel order */
      for (; cIdx < el_channels; c++, cIdx++, elCh++) {
        /* Robustness check */
        if (c >= aacChannels) {
          return AAC_DEC_UNKNOWN;
        }

        CAacDecoderChannelInfo *pAacDecoderChannelInfo =
            self->pAacDecoderChannelInfo[c];
        CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo =
            self->pAacDecoderStaticChannelInfo[c];

        /* Setup offset for time buffer traversal. */
        {
          pAacDecoderStaticChannelInfo =
              self->pAacDecoderStaticChannelInfo[Reverse_chMapping[c]];
          offset =
              FDK_chMapDescr_getMapValue(
                  &self->mapDescr, Reverse_chMapping[cIdx], self->chMapIndex) *
              timeDataChannelOffset;
        }

        if (flags & AACDEC_FLUSH) {
          /* Clear pAacDecoderChannelInfo->pSpectralCoefficient because with
           * AACDEC_FLUSH set it contains undefined data. */
          FDKmemclear(pAacDecoderChannelInfo->pSpectralCoefficient,
                      sizeof(FIXP_DBL) * self->streamInfo.aacSamplesPerFrame);
        }

        /* if The ics info is not valid and it will be stored and used in the
         * following concealment method, mark the frame as erroneous */
        {
          CIcsInfo *pIcsInfo = &pAacDecoderChannelInfo->icsInfo;
          CConcealmentInfo *hConcealmentInfo =
              &pAacDecoderStaticChannelInfo->concealmentInfo;
          const int mute_release_active =
              (self->frameOK && !(flags & AACDEC_CONCEAL)) &&
              ((hConcealmentInfo->concealState >= ConcealState_Mute) &&
               (hConcealmentInfo->cntValidFrames + 1 <=
                hConcealmentInfo->pConcealParams->numMuteReleaseFrames));
          const int icsIsInvalid = (GetScaleFactorBandsTransmitted(pIcsInfo) >
                                    GetScaleFactorBandsTotal(pIcsInfo));
          const int icsInfoUsedinFadeOut =
              !(pAacDecoderChannelInfo->renderMode == AACDEC_RENDER_LPD &&
                pAacDecoderStaticChannelInfo->last_lpd_mode == 0);
          if (icsInfoUsedinFadeOut && icsIsInvalid && !mute_release_active) {
            self->frameOK = 0;
          }
        }

        /*
          Conceal defective spectral data
        */
        {
          CAacDecoderChannelInfo **ppAacDecoderChannelInfo =
              &pAacDecoderChannelInfo;
          CAacDecoderStaticChannelInfo **ppAacDecoderStaticChannelInfo =
              &pAacDecoderStaticChannelInfo;
          {
            concealApplyReturnCode = CConcealment_Apply(
                &(*ppAacDecoderStaticChannelInfo)->concealmentInfo,
                *ppAacDecoderChannelInfo, *ppAacDecoderStaticChannelInfo,
                &self->samplingRateInfo[streamIndex],
                self->streamInfo.aacSamplesPerFrame,
                pAacDecoderStaticChannelInfo->last_lpd_mode,
                (self->frameOK && !(flags & AACDEC_CONCEAL)),
                self->flags[streamIndex]);
          }
        }
        if (concealApplyReturnCode == -1) {
          frameOk_butConceal = 1;
        }

        if (flags & (AACDEC_INTR)) {
          /* Reset DRC control data for this channel */
          aacDecoder_drcInitChannelData(&pAacDecoderStaticChannelInfo->drcData);
        }
        if (flags & (AACDEC_CLRHIST)) {
          if (!(self->flags[0] & AC_USAC)) {
            /* Reset DRC control data for this channel */
            aacDecoder_drcInitChannelData(
                &pAacDecoderStaticChannelInfo->drcData);
          }
        }

        /* The DRC module demands to be called with the gain field holding the
         * gain scale. */
        self->extGain[0] = (FIXP_DBL)AACDEC_DRC_GAIN_SCALING;

        /* DRC processing */
        aacDecoder_drcApply(
            self->hDrcInfo, self->hSbrDecoder, pAacDecoderChannelInfo,
            &pAacDecoderStaticChannelInfo->drcData, self->extGain, c,
            self->streamInfo.aacSamplesPerFrame, self->sbrEnabled

        );

        if (timeDataSize < timeDataChannelOffset * self->aacChannels) {
          ErrorStatus = AAC_DEC_OUTPUT_BUFFER_TOO_SMALL;
          break;
        }
        if (self->flushStatus && (self->flushCnt > 0) &&
            !(flags & AACDEC_CONCEAL)) {
          FDKmemclear(pTimeData + offset,
                      sizeof(PCM_DEC) * self->streamInfo.aacSamplesPerFrame);
        } else
          switch (pAacDecoderChannelInfo->renderMode) {
            case AACDEC_RENDER_IMDCT:

              CBlock_FrequencyToTime(
                  pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
                  pTimeData + offset, self->streamInfo.aacSamplesPerFrame,
                  (self->frameOK && !(flags & AACDEC_CONCEAL) &&
                   !frameOk_butConceal),
                  pAacDecoderChannelInfo->pComStaticData->pWorkBufferCore1
                      ->mdctOutTemp,
                  self->aacOutDataHeadroom, self->elFlags[el], elCh);

              self->extGainDelay = self->streamInfo.aacSamplesPerFrame;
              break;
            case AACDEC_RENDER_ELDFB: {
              CBlock_FrequencyToTimeLowDelay(
                  pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
                  pTimeData + offset, self->streamInfo.aacSamplesPerFrame);
              self->extGainDelay =
                  (self->streamInfo.aacSamplesPerFrame * 2 -
                   self->streamInfo.aacSamplesPerFrame / 2 - 1) /
                  2;
            } break;
            case AACDEC_RENDER_LPD:

              CLpd_RenderTimeSignal(
                  pAacDecoderStaticChannelInfo, pAacDecoderChannelInfo,
                  pTimeData + offset, self->streamInfo.aacSamplesPerFrame,
                  &self->samplingRateInfo[streamIndex],
                  (self->frameOK && !(flags & AACDEC_CONCEAL) &&
                   !frameOk_butConceal),
                  self->aacOutDataHeadroom, flags, self->flags[streamIndex]);

              self->extGainDelay = self->streamInfo.aacSamplesPerFrame;
              break;
            default:
              ErrorStatus = AAC_DEC_UNKNOWN;
              break;
          }
        /* TimeDomainFading */
        if (!CConceal_TDFading_Applied[c]) {
          CConceal_TDFading_Applied[c] = CConcealment_TDFading(
              self->streamInfo.aacSamplesPerFrame,
              &self->pAacDecoderStaticChannelInfo[c], self->aacOutDataHeadroom,
              pTimeData + offset, 0);
          if (c + 1 < (8) && c < aacChannels - 1) {
            /* update next TDNoise Seed to avoid muting in case of Parametric
             * Stereo */
            self->pAacDecoderStaticChannelInfo[c + 1]
                ->concealmentInfo.TDNoiseSeed =
                self->pAacDecoderStaticChannelInfo[c]
                    ->concealmentInfo.TDNoiseSeed;
          }
        }
      }
    }

    if (self->flags[streamIndex] & AC_USAC) {
      int bsPseudoLr = 0;
      mpegSurroundDecoder_IsPseudoLR(
          (CMpegSurroundDecoder *)self->pMpegSurroundDecoder, &bsPseudoLr);
      /* ISO/IEC 23003-3, 7.11.2.6 Modification of core decoder output (pseudo
       * LR) */
      if ((aacChannels == 2) && bsPseudoLr) {
        int i, offset2;
        const FIXP_SGL invSqrt2 = FL2FXCONST_SGL(0.707106781186547f);
        PCM_DEC *pTD = pTimeData;

        offset2 = timeDataChannelOffset;

        for (i = 0; i < self->streamInfo.aacSamplesPerFrame; i++) {
          FIXP_DBL L = PCM_DEC2FIXP_DBL(pTD[0]);
          FIXP_DBL R = PCM_DEC2FIXP_DBL(pTD[offset2]);
          L = fMult(L, invSqrt2);
          R = fMult(R, invSqrt2);
          pTD[0] = L + R;
          pTD[offset2] = L - R;
          pTD++;
        }
      }
    }

    /* Extract DRC control data and map it to channels (with bitstream delay) */
    mapped = aacDecoder_drcEpilog(
        self->hDrcInfo, bs, self->pAacDecoderStaticChannelInfo,
        pce->ElementInstanceTag, drcChMap, aacChannels);
    if (mapped > 0) {
      if (!(self->flags[streamIndex] & (AC_USAC | AC_RSV603DA))) {
        /* If at least one DRC thread has been mapped to a channel there was DRC
         * data in the bitstream. */
        self->flags[streamIndex] |= AC_DRC_PRESENT;
      } else {
        ErrorStatus = AAC_DEC_UNSUPPORTED_FORMAT;
      }
    }
    if (self->flags[streamIndex] & (AC_USAC | AC_RSV603DA)) {
      aacDecoder_drcDisable(self->hDrcInfo);
    }
  }

  /* Add additional concealment delay */
  self->streamInfo.outputDelay +=
      CConcealment_GetDelay(&self->concealCommonData) *
      self->streamInfo.aacSamplesPerFrame;

  /* Map DRC data to StreamInfo structure */
  aacDecoder_drcGetInfo(self->hDrcInfo, &self->streamInfo.drcPresMode,
                        &self->streamInfo.drcProgRefLev);

  /* Reorder channel type information tables.  */
  if (!(self->flags[0] & AC_RSV603DA)) {
    AUDIO_CHANNEL_TYPE types[(8)];
    UCHAR idx[(8)];
    int c;
    int mapValue;

    FDK_ASSERT(sizeof(self->channelType) == sizeof(types));
    FDK_ASSERT(sizeof(self->channelIndices) == sizeof(idx));

    FDKmemcpy(types, self->channelType, sizeof(types));
    FDKmemcpy(idx, self->channelIndices, sizeof(idx));

    for (c = 0; c < aacChannels; c++) {
      mapValue =
          FDK_chMapDescr_getMapValue(&self->mapDescr, c, self->chMapIndex);
      self->channelType[mapValue] = types[c];
      self->channelIndices[mapValue] = idx[c];
    }
  }

  self->blockNumber++;

  return ErrorStatus;
}

/*!
  \brief returns the streaminfo pointer

  The function hands back a pointer to the streaminfo structure

  \return pointer to the struct
*/
LINKSPEC_CPP CStreamInfo *CAacDecoder_GetStreamInfo(HANDLE_AACDECODER self) {
  if (!self) {
    return NULL;
  }
  return &self->streamInfo;
}
