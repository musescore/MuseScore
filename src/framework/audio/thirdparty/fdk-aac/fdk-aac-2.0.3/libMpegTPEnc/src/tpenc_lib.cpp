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

/******************* MPEG transport format encoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport encode

*******************************************************************************/

#include "tpenc_lib.h"

/* library info */
#include "tp_version.h"

#define MODULE_NAME "transportEnc"

#include "tpenc_asc.h"

#include "tpenc_adts.h"

#include "tpenc_adif.h"

#include "tpenc_latm.h"

typedef struct {
  int curSubFrame;
  int nSubFrames;
  int prevBits;
} RAWPACKETS_INFO;

struct TRANSPORTENC {
  CODER_CONFIG config;
  TRANSPORT_TYPE transportFmt; /*!< MPEG4 transport type. */

  FDK_BITSTREAM bitStream;
  UCHAR *bsBuffer;
  INT bsBufferSize;

  INT pceFrameCounter; /*!< Indicates frame period when PCE must be written in
                          raw_data_block. -1 means not to write a PCE in
                          raw_dat_block. */
  union {
    STRUCT_ADTS adts;

    ADIF_INFO adif;

    LATM_STREAM latm;

    RAWPACKETS_INFO raw;

  } writer;

  CSTpCallBacks callbacks;
};

typedef struct _TRANSPORTENC_STRUCT TRANSPORTENC_STRUCT;

/*
 * MEMORY Declaration
 */

C_ALLOC_MEM(Ram_TransportEncoder, struct TRANSPORTENC, 1)

TRANSPORTENC_ERROR transportEnc_Open(HANDLE_TRANSPORTENC *phTpEnc) {
  HANDLE_TRANSPORTENC hTpEnc;

  if (phTpEnc == NULL) {
    return TRANSPORTENC_INVALID_PARAMETER;
  }

  hTpEnc = GetRam_TransportEncoder(0);

  if (hTpEnc == NULL) {
    return TRANSPORTENC_NO_MEM;
  }

  *phTpEnc = hTpEnc;
  return TRANSPORTENC_OK;
}

/**
 * \brief  Get frame period of PCE in raw_data_block.
 *
 * - Write PCE only if necessary. PCE can be part of the ASC if chConfig==0
 * whererfore no additonal PCE will be written in raw_data_block.
 * - A matrixMixdown coefficient can only be written if chConfig is 5.0 or 5.1.
 * - The PCE repetition rate in raw_data_block can be controlled via
 * headerPeriod parameter.
 *
 * \param channelMode           Encoder Channel Mode.
 * \param channelConfigZero     No standard channel configuration.
 * \param transportFmt          Format of the transport to be written.
 * \param headerPeriod          Chosen PCE frame repetition rate.
 * \param matrixMixdownA        Indicates if a valid Matrix Mixdown coefficient
 * is available.
 *
 * \return  PCE frame repetition rate. -1 means no PCE present in
 * raw_data_block.
 */
static INT getPceRepetitionRate(const CHANNEL_MODE channelMode,
                                const int channelConfigZero,
                                const TRANSPORT_TYPE transportFmt,
                                const int headerPeriod,
                                const int matrixMixdownA) {
  INT pceFrameCounter = -1; /* variable to be returned */

  if (headerPeriod > 0) {
    switch (getChannelConfig(channelMode, channelConfigZero)) {
      case 0:
        switch (transportFmt) {
          case TT_MP4_ADTS:
          case TT_MP4_LATM_MCP0:
          case TT_MP4_RAW:
            pceFrameCounter = headerPeriod;
            break;
          case TT_MP4_ADIF: /* ADIF header comprises PCE */
            if ((channelMode == MODE_1_2_2) || (channelMode == MODE_1_2_2_1)) {
              pceFrameCounter = headerPeriod; /* repeating pce only meaningful
                                                 for potential matrix mixdown */
              break;
            }
            FDK_FALLTHROUGH;
          case TT_MP4_LOAS:      /* PCE in ASC if chChonfig==0 */
          case TT_MP4_LATM_MCP1: /* PCE in ASC if chChonfig==0 */
          default:
            pceFrameCounter = -1; /* no PCE in raw_data_block */
        }
        break;
      case 5: /* MODE_1_2_2 */
      case 6: /* MODE_1_2_2_1 */
        /* matrixMixdownCoefficient can only be written if 5.0 and 5.1 config
         * present. */
        if (matrixMixdownA != 0) {
          switch (transportFmt) {
            case TT_MP4_ADIF: /* ADIF header comprises PCE */
            case TT_MP4_ADTS:
            case TT_MP4_LOAS:      /* no PCE in ASC because chConfig!=0 */
            case TT_MP4_LATM_MCP1: /* no PCE in ASC because chConfig!=0 */
            case TT_MP4_LATM_MCP0:
            case TT_MP4_RAW:
              pceFrameCounter = headerPeriod;
              break;
            default:
              pceFrameCounter = -1; /* no PCE in raw_data_block */
          }                         /* switch transportFmt */
        }                           /* if matrixMixdownA!=0 */
        break;
      default:
        pceFrameCounter = -1; /* no PCE in raw_data_block */
    }                         /* switch getChannelConfig() */
  }                           /* if headerPeriod>0  */
  else {
    pceFrameCounter = -1; /* no PCE in raw_data_block */
  }

  return pceFrameCounter;
}

TRANSPORTENC_ERROR transportEnc_Init(HANDLE_TRANSPORTENC hTpEnc,
                                     UCHAR *bsBuffer, INT bsBufferSize,
                                     TRANSPORT_TYPE transportFmt,
                                     CODER_CONFIG *cconfig, UINT flags) {
  /* Copy configuration structure */
  FDKmemcpy(&hTpEnc->config, cconfig, sizeof(CODER_CONFIG));

  /* Init transportEnc struct. */
  hTpEnc->transportFmt = transportFmt;

  hTpEnc->bsBuffer = bsBuffer;
  hTpEnc->bsBufferSize = bsBufferSize;

  FDKinitBitStream(&hTpEnc->bitStream, hTpEnc->bsBuffer, hTpEnc->bsBufferSize,
                   0, BS_WRITER);

  switch (transportFmt) {
    case TT_MP4_ADIF:
      /* Sanity checks */
      if ((hTpEnc->config.aot != AOT_AAC_LC) ||
          (hTpEnc->config.samplesPerFrame != 1024)) {
        return TRANSPORTENC_INVALID_PARAMETER;
      }
      hTpEnc->writer.adif.headerWritten = 0;
      hTpEnc->writer.adif.samplingRate = hTpEnc->config.samplingRate;
      hTpEnc->writer.adif.bitRate = hTpEnc->config.bitRate;
      hTpEnc->writer.adif.profile = ((int)hTpEnc->config.aot) - 1;
      hTpEnc->writer.adif.cm = hTpEnc->config.channelMode;
      hTpEnc->writer.adif.bVariableRate = 0;
      hTpEnc->writer.adif.instanceTag = 0;
      hTpEnc->writer.adif.matrixMixdownA = hTpEnc->config.matrixMixdownA;
      hTpEnc->writer.adif.pseudoSurroundEnable =
          (hTpEnc->config.flags & CC_PSEUDO_SURROUND) ? 1 : 0;
      break;

    case TT_MP4_ADTS:
      /* Sanity checks */
      if ((hTpEnc->config.aot != AOT_AAC_LC) ||
          (hTpEnc->config.samplesPerFrame != 1024)) {
        return TRANSPORTENC_INVALID_PARAMETER;
      }
      if (adtsWrite_Init(&hTpEnc->writer.adts, &hTpEnc->config) != 0) {
        return TRANSPORTENC_INVALID_PARAMETER;
      }
      break;

    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1: {
      TRANSPORTENC_ERROR error;

      error = transportEnc_Latm_Init(&hTpEnc->writer.latm, &hTpEnc->bitStream,
                                     &hTpEnc->config, flags & TP_FLAG_LATM_AMV,
                                     transportFmt, &hTpEnc->callbacks);
      if (error != TRANSPORTENC_OK) {
        return error;
      }
    } break;

    case TT_MP4_RAW:
      hTpEnc->writer.raw.curSubFrame = 0;
      hTpEnc->writer.raw.nSubFrames = hTpEnc->config.nSubFrames;
      break;

    default:
      return TRANSPORTENC_INVALID_PARAMETER;
  }

  /* pceFrameCounter indicates if PCE must be written in raw_data_block. */
  hTpEnc->pceFrameCounter = getPceRepetitionRate(
      hTpEnc->config.channelMode, hTpEnc->config.channelConfigZero,
      transportFmt, hTpEnc->config.headerPeriod, hTpEnc->config.matrixMixdownA);

  return TRANSPORTENC_OK;
}

TRANSPORTENC_ERROR transportEnc_AddOtherDataBits(HANDLE_TRANSPORTENC hTpEnc,
                                                 const int nBits) {
  TRANSPORTENC_ERROR tpErr = TRANSPORTENC_OK;

  switch (hTpEnc->transportFmt) {
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
    case TT_MP4_LOAS:
      tpErr = transportEnc_LatmAddOtherDataBits(&hTpEnc->writer.latm, nBits);
      break;
    case TT_MP4_ADTS:
    case TT_MP4_ADIF:
    case TT_MP4_RAW:
    default:
      tpErr = TRANSPORTENC_UNKOWN_ERROR;
  }

  return tpErr;
}

HANDLE_FDK_BITSTREAM transportEnc_GetBitstream(HANDLE_TRANSPORTENC hTp) {
  return &hTp->bitStream;
}

int transportEnc_RegisterSbrCallback(HANDLE_TRANSPORTENC hTpEnc,
                                     const cbSbr_t cbSbr, void *user_data) {
  if (hTpEnc == NULL) {
    return -1;
  }
  hTpEnc->callbacks.cbSbr = cbSbr;
  hTpEnc->callbacks.cbSbrData = user_data;
  return 0;
}
int transportEnc_RegisterUsacCallback(HANDLE_TRANSPORTENC hTpEnc,
                                      const cbUsac_t cbUsac, void *user_data) {
  if (hTpEnc == NULL) {
    return -1;
  }
  hTpEnc->callbacks.cbUsac = cbUsac;
  hTpEnc->callbacks.cbUsacData = user_data;
  return 0;
}

int transportEnc_RegisterSscCallback(HANDLE_TRANSPORTENC hTpEnc,
                                     const cbSsc_t cbSsc, void *user_data) {
  if (hTpEnc == NULL) {
    return -1;
  }
  hTpEnc->callbacks.cbSsc = cbSsc;
  hTpEnc->callbacks.cbSscData = user_data;
  return 0;
}

TRANSPORTENC_ERROR transportEnc_WriteAccessUnit(HANDLE_TRANSPORTENC hTp,
                                                INT frameUsedBits,
                                                int bufferFullness, int ncc) {
  TRANSPORTENC_ERROR err = TRANSPORTENC_OK;

  if (!hTp) {
    return TRANSPORTENC_INVALID_PARAMETER;
  }
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream;

  /* In case of writing PCE in raw_data_block frameUsedBits must be adapted. */
  if (hTp->pceFrameCounter >= hTp->config.headerPeriod) {
    frameUsedBits += transportEnc_GetPCEBits(
        hTp->config.channelMode, hTp->config.matrixMixdownA,
        3); /* Consider 3 bits ID signalling in alignment */
  }

  switch (hTp->transportFmt) {
    case TT_MP4_ADIF:
      FDKinitBitStream(&hTp->bitStream, hTp->bsBuffer, hTp->bsBufferSize, 0,
                       BS_WRITER);
      if (0 != adifWrite_EncodeHeader(&hTp->writer.adif, hBs, bufferFullness)) {
        err = TRANSPORTENC_INVALID_CONFIG;
      }
      break;
    case TT_MP4_ADTS:
      bufferFullness /= ncc; /* Number of Considered Channels */
      bufferFullness /= 32;
      bufferFullness = FDKmin(0x7FF, bufferFullness); /* Signal variable rate */
      adtsWrite_EncodeHeader(&hTp->writer.adts, &hTp->bitStream, bufferFullness,
                             frameUsedBits);
      break;
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      bufferFullness /= ncc; /* Number of Considered Channels */
      bufferFullness /= 32;
      bufferFullness = FDKmin(0xFF, bufferFullness); /* Signal variable rate */
      transportEnc_LatmWrite(&hTp->writer.latm, hBs, frameUsedBits,
                             bufferFullness, &hTp->callbacks);
      break;
    case TT_MP4_RAW:
      if (hTp->writer.raw.curSubFrame >= hTp->writer.raw.nSubFrames) {
        hTp->writer.raw.curSubFrame = 0;
        FDKinitBitStream(&hTp->bitStream, hTp->bsBuffer, hTp->bsBufferSize, 0,
                         BS_WRITER);
      }
      hTp->writer.raw.prevBits = FDKgetValidBits(hBs);
      break;
    default:
      err = TRANSPORTENC_UNSUPPORTED_FORMAT;
      break;
  }

  /* Write PCE in raw_data_block if required */
  if (hTp->pceFrameCounter >= hTp->config.headerPeriod) {
    INT crcIndex = 0;
    /* Align inside PCE with repsect to the first bit of the raw_data_block() */
    UINT alignAnchor = FDKgetValidBits(&hTp->bitStream);

    /* Write PCE element ID bits */
    FDKwriteBits(&hTp->bitStream, ID_PCE, 3);

    if ((hTp->transportFmt == TT_MP4_ADTS) &&
        !hTp->writer.adts.protection_absent) {
      crcIndex = adtsWrite_CrcStartReg(&hTp->writer.adts, &hTp->bitStream, 0);
    }

    /* Write PCE as first raw_data_block element */
    transportEnc_writePCE(
        &hTp->bitStream, hTp->config.channelMode, hTp->config.samplingRate, 0,
        1, hTp->config.matrixMixdownA,
        (hTp->config.flags & CC_PSEUDO_SURROUND) ? 1 : 0, alignAnchor);

    if ((hTp->transportFmt == TT_MP4_ADTS) &&
        !hTp->writer.adts.protection_absent) {
      adtsWrite_CrcEndReg(&hTp->writer.adts, &hTp->bitStream, crcIndex);
    }
    hTp->pceFrameCounter = 0; /* reset pce frame counter */
  }

  if (hTp->pceFrameCounter != -1) {
    hTp->pceFrameCounter++; /* Update pceFrameCounter only if PCE writing is
                               active. */
  }

  return err;
}

TRANSPORTENC_ERROR transportEnc_EndAccessUnit(HANDLE_TRANSPORTENC hTp,
                                              int *bits) {
  switch (hTp->transportFmt) {
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
    case TT_MP4_LOAS:
      transportEnc_LatmAdjustSubframeBits(&hTp->writer.latm, bits);
      break;
    case TT_MP4_ADTS:
      adtsWrite_EndRawDataBlock(&hTp->writer.adts, &hTp->bitStream, bits);
      break;
    case TT_MP4_ADIF:
      /* Substract ADIF header from AU bits, not to be considered. */
      *bits -= adifWrite_GetHeaderBits(&hTp->writer.adif);
      hTp->writer.adif.headerWritten = 1;
      break;
    case TT_MP4_RAW:
      *bits -= hTp->writer.raw.prevBits;
      break;
    default:
      break;
  }

  return TRANSPORTENC_OK;
}

TRANSPORTENC_ERROR transportEnc_GetFrame(HANDLE_TRANSPORTENC hTpEnc,
                                         int *nbytes) {
  TRANSPORTENC_ERROR tpErr = TRANSPORTENC_OK;
  HANDLE_FDK_BITSTREAM hBs = &hTpEnc->bitStream;

  switch (hTpEnc->transportFmt) {
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
    case TT_MP4_LOAS:
      *nbytes = hTpEnc->bsBufferSize;
      tpErr = transportEnc_LatmGetFrame(&hTpEnc->writer.latm, hBs, nbytes);
      break;
    case TT_MP4_ADTS:
      if (hTpEnc->writer.adts.currentBlock >=
          hTpEnc->writer.adts.num_raw_blocks + 1) {
        *nbytes = (FDKgetValidBits(hBs) + 7) >> 3;
        hTpEnc->writer.adts.currentBlock = 0;
      } else {
        *nbytes = 0;
      }
      break;
    case TT_MP4_ADIF:
      FDK_ASSERT((INT)FDKgetValidBits(hBs) >= 0);
      *nbytes = (FDKgetValidBits(hBs) + 7) >> 3;
      break;
    case TT_MP4_RAW:
      FDKsyncCache(hBs);
      hTpEnc->writer.raw.curSubFrame++;
      *nbytes = ((FDKgetValidBits(hBs) - hTpEnc->writer.raw.prevBits) + 7) >> 3;
      break;
    default:
      break;
  }

  return tpErr;
}

INT transportEnc_GetStaticBits(HANDLE_TRANSPORTENC hTp, int auBits) {
  INT nbits = 0, nPceBits = 0;

  /* Write PCE within raw_data_block in transport lib. */
  if (hTp->pceFrameCounter >= hTp->config.headerPeriod) {
    nPceBits = transportEnc_GetPCEBits(
        hTp->config.channelMode, hTp->config.matrixMixdownA,
        3);             /* Consider 3 bits ID signalling in alignment */
    auBits += nPceBits; /* Adapt required raw_data_block bit consumtpion for AU
                           length information e.g. in LATM/LOAS configuration.
                         */
  }

  switch (hTp->transportFmt) {
    case TT_MP4_ADIF:
    case TT_MP4_RAW:
      nbits = 0; /* Do not consider the ADIF header into the total bitrate */
      break;
    case TT_MP4_ADTS:
      nbits = adtsWrite_GetHeaderBits(&hTp->writer.adts);
      break;
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      nbits =
          transportEnc_LatmCountTotalBitDemandHeader(&hTp->writer.latm, auBits);
      break;
    default:
      nbits = 0;
      break;
  }

  /* PCE is written in the transport library therefore the bit consumption is
   * part of the transport static bits. */
  nbits += nPceBits;

  return nbits;
}

void transportEnc_Close(HANDLE_TRANSPORTENC *phTp) {
  if (phTp != NULL) {
    if (*phTp != NULL) {
      FreeRam_TransportEncoder(phTp);
    }
  }
}

int transportEnc_CrcStartReg(HANDLE_TRANSPORTENC hTpEnc, int mBits) {
  int crcReg = 0;

  switch (hTpEnc->transportFmt) {
    case TT_MP4_ADTS:
      crcReg = adtsWrite_CrcStartReg(&hTpEnc->writer.adts, &hTpEnc->bitStream,
                                     mBits);
      break;
    default:
      break;
  }

  return crcReg;
}

void transportEnc_CrcEndReg(HANDLE_TRANSPORTENC hTpEnc, int reg) {
  switch (hTpEnc->transportFmt) {
    case TT_MP4_ADTS:
      adtsWrite_CrcEndReg(&hTpEnc->writer.adts, &hTpEnc->bitStream, reg);
      break;
    default:
      break;
  }
}

TRANSPORTENC_ERROR transportEnc_GetConf(HANDLE_TRANSPORTENC hTpEnc,
                                        CODER_CONFIG *cc,
                                        FDK_BITSTREAM *dataBuffer,
                                        UINT *confType) {
  TRANSPORTENC_ERROR tpErr = TRANSPORTENC_OK;
  HANDLE_LATM_STREAM hLatmConfig = &hTpEnc->writer.latm;

  *confType = 0; /* set confType variable to default */

  /* write StreamMuxConfig or AudioSpecificConfig depending on format used */
  switch (hTpEnc->transportFmt) {
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
    case TT_MP4_LOAS:
      tpErr =
          CreateStreamMuxConfig(hLatmConfig, dataBuffer, 0, &hTpEnc->callbacks);
      *confType = 1; /* config is SMC */
      break;
    default:
      if (transportEnc_writeASC(dataBuffer, cc, &hTpEnc->callbacks) != 0) {
        tpErr = TRANSPORTENC_UNKOWN_ERROR;
      }
  }

  return tpErr;
}

TRANSPORTENC_ERROR transportEnc_GetLibInfo(LIB_INFO *info) {
  int i;

  if (info == NULL) {
    return TRANSPORTENC_INVALID_PARAMETER;
  }
  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) {
    return TRANSPORTENC_UNKOWN_ERROR;
  }
  info += i;

  info->module_id = FDK_TPENC;
  info->version = LIB_VERSION(TP_LIB_VL0, TP_LIB_VL1, TP_LIB_VL2);
  LIB_VERSION_STRING(info);
#ifdef SUPPRESS_BUILD_DATE_INFO
  info->build_date = "";
  info->build_time = "";
#else
  info->build_date = __DATE__;
  info->build_time = __TIME__;
#endif
  info->title = TP_LIB_TITLE;

  /* Set flags */
  info->flags =
      0 | CAPF_ADIF | CAPF_ADTS | CAPF_LATM | CAPF_LOAS | CAPF_RAWPACKETS;

  return TRANSPORTENC_OK;
}
