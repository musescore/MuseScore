/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2022 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/******************* MPEG transport format decoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport decoder

*******************************************************************************/

#include "tpdec_lib.h"

/* library version */
#include "tp_version.h"

#include "tp_data.h"

#include "tpdec_adts.h"

#include "tpdec_adif.h"

#include "tpdec_latm.h"

#include "tpdec_drm.h"

#include "FDK_crc.h"

#define MODULE_NAME "transportDec"

typedef union {
  STRUCT_ADTS adts;

  CAdifHeader adif;

  CLatmDemux latm;

  STRUCT_DRM drm;

} transportdec_parser_t;

#define MHAS_CONFIG_PRESENT 0x001
#define MHAS_UI_PRESENT 0x002

struct TRANSPORTDEC {
  TRANSPORT_TYPE transportFmt; /*!< MPEG4 transportDec type. */

  CSTpCallBacks callbacks; /*!< Struct holding callback and its data */

  FDK_BITSTREAM bitStream[1]; /* Bitstream reader */
  UCHAR *bsBuffer;            /* Internal bitstreamd data buffer */

  transportdec_parser_t parser; /* Format specific parser structs. */

  CSAudioSpecificConfig asc[(1 * 1) + 1]; /* Audio specific config from the last
                                             config found. One additional
                                             CSAudioSpecificConfig is used
                                             temporarily for parsing. */
  CCtrlCFGChange ctrlCFGChange[(1 * 1)];  /* Controls config change */

  UINT globalFramePos;      /* Global transport frame reference bit position. */
  UINT accessUnitAnchor[1]; /* Current access unit start bit position. */
  INT auLength[1];          /* Length of current access unit. */
  INT numberOfRawDataBlocks; /* Current number of raw data blocks contained
                                remaining from the current transport frame. */
  UINT avgBitRate; /* Average bit rate used for frame loss estimation. */
  UINT lastValidBufferFullness; /* Last valid buffer fullness value for frame
                                   loss estimation */
  INT remainder; /* Reminder in division during lost access unit estimation. */
  INT missingAccessUnits; /* Estimated missing access units. */
  UINT burstPeriod;       /* Data burst period in mili seconds. */
  UINT holdOffFrames;     /* Amount of frames that were already hold off due to
                             buffer fullness condition not being met. */
  UINT flags;             /* Flags. */
  INT targetLayout;       /* CICP target layout. */
  UINT *pLoudnessInfoSetPosition; /* Reference and start position (bits) and
                                     length (bytes) of loudnessInfoSet within
                                     rsv603daConfig.  */
};

/* Flag bitmasks for "flags" member of struct TRANSPORTDEC */
#define TPDEC_SYNCOK 1
#define TPDEC_MINIMIZE_DELAY 2
#define TPDEC_IGNORE_BUFFERFULLNESS 4
#define TPDEC_EARLY_CONFIG 8
#define TPDEC_LOST_FRAMES_PENDING 16
#define TPDEC_CONFIG_FOUND 32
#define TPDEC_USE_ELEM_SKIPPING 64

/* force config/content change */
#define TPDEC_FORCE_CONFIG_CHANGE 1
#define TPDEC_FORCE_CONTENT_CHANGE 2

/* skip packet */
#define TPDEC_SKIP_PACKET 1

C_ALLOC_MEM(Ram_TransportDecoder, struct TRANSPORTDEC, 1)
C_ALLOC_MEM(Ram_TransportDecoderBuffer, UCHAR, (8192 * 4))

HANDLE_TRANSPORTDEC transportDec_Open(const TRANSPORT_TYPE transportFmt,
                                      const UINT flags, const UINT nrOfLayers) {
  HANDLE_TRANSPORTDEC hInput;

  hInput = GetRam_TransportDecoder(0);
  if (hInput == NULL) {
    return NULL;
  }

  /* Init transportDec struct. */
  hInput->transportFmt = transportFmt;

  switch (transportFmt) {
    case TT_MP4_ADIF:
      break;

    case TT_MP4_ADTS:
      if (flags & TP_FLAG_MPEG4)
        hInput->parser.adts.decoderCanDoMpeg4 = 1;
      else
        hInput->parser.adts.decoderCanDoMpeg4 = 0;
      adtsRead_CrcInit(&hInput->parser.adts);
      hInput->parser.adts.BufferFullnesStartFlag = 1;
      hInput->numberOfRawDataBlocks = 0;
      break;

    case TT_DRM:
      drmRead_CrcInit(&hInput->parser.drm);
      break;

    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      hInput->parser.latm.usacExplicitCfgChanged = 0;
      hInput->parser.latm.applyAsc = 1;
      break;
    case TT_MP4_LOAS:
      hInput->parser.latm.usacExplicitCfgChanged = 0;
      hInput->parser.latm.applyAsc = 1;
      break;
    case TT_MP4_RAW:
      break;

    default:
      FreeRam_TransportDecoder(&hInput);
      hInput = NULL;
      break;
  }

  if (hInput != NULL) {
    /* Create bitstream */
    {
      hInput->bsBuffer = GetRam_TransportDecoderBuffer(0);
      if (hInput->bsBuffer == NULL) {
        transportDec_Close(&hInput);
        return NULL;
      }
      if (nrOfLayers > 1) {
        transportDec_Close(&hInput);
        return NULL;
      }
      for (UINT i = 0; i < nrOfLayers; i++) {
        FDKinitBitStream(&hInput->bitStream[i], hInput->bsBuffer, (8192 * 4), 0,
                         BS_READER);
      }
    }
    hInput->burstPeriod = 0;
  }

  return hInput;
}

TRANSPORTDEC_ERROR transportDec_OutOfBandConfig(HANDLE_TRANSPORTDEC hTp,
                                                UCHAR *conf, const UINT length,
                                                UINT layer) {
  int i;

  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;

  int fConfigFound = 0;

  UCHAR configChanged = 0;
  UCHAR configMode = AC_CM_DET_CFG_CHANGE;

  UCHAR tmpConf[1024] = {0};
  if (length > 1024) {
    return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  }
  FDKmemcpy(tmpConf, conf, length);
  FDKinitBitStream(hBs, tmpConf, 1024, length << 3, BS_READER);

  for (i = 0; i < 2; i++) {
    if (i > 0) {
      FDKpushBack(hBs, (INT)length * 8 - (INT)FDKgetValidBits(hBs));
      configMode = AC_CM_ALLOC_MEM;
    }

    /* config transport decoder */
    switch (hTp->transportFmt) {
      case TT_MP4_LATM_MCP0:
      case TT_MP4_LATM_MCP1:
      case TT_MP4_LOAS: {
        if (layer != 0) {
          return TRANSPORTDEC_INVALID_PARAMETER;
        }
        CLatmDemux *pLatmDemux = &hTp->parser.latm;
        err = CLatmDemux_ReadStreamMuxConfig(hBs, pLatmDemux, &hTp->callbacks,
                                             hTp->asc, &fConfigFound,
                                             configMode, configChanged);
        if (err != TRANSPORTDEC_OK) {
          return err;
        }
      } break;
      default:
        fConfigFound = 1;
        err = AudioSpecificConfig_Parse(&hTp->asc[(1 * 1)], hBs, 1,
                                        &hTp->callbacks, configMode,
                                        configChanged, AOT_NULL_OBJECT);
        if (err == TRANSPORTDEC_OK) {
          int errC;

          hTp->asc[layer] = hTp->asc[(1 * 1)];
          errC = hTp->callbacks.cbUpdateConfig(
              hTp->callbacks.cbUpdateConfigData, &hTp->asc[layer],
              hTp->asc[layer].configMode, &hTp->asc[layer].AacConfigChanged);
          if (errC != 0) {
            err = TRANSPORTDEC_PARSE_ERROR;
          }
        }
        break;
      case TT_DRM:
        fConfigFound = 1;
        err = DrmRawSdcAudioConfig_Parse(&hTp->asc[layer], hBs, &hTp->callbacks,
                                         configMode, configChanged);
        if (err == TRANSPORTDEC_OK) {
          int errC;

          errC = hTp->callbacks.cbUpdateConfig(
              hTp->callbacks.cbUpdateConfigData, &hTp->asc[layer],
              hTp->asc[layer].configMode, &hTp->asc[layer].AacConfigChanged);
          if (errC != 0) {
            err = TRANSPORTDEC_PARSE_ERROR;
          }
        }
        break;
    }

    if (err == TRANSPORTDEC_OK) {
      if ((i == 0) && (hTp->asc[layer].AacConfigChanged ||
                       hTp->asc[layer].SbrConfigChanged ||
                       hTp->asc[layer].SacConfigChanged)) {
        int errC;

        configChanged = 1;
        errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                        &hTp->asc[layer]);
        if (errC != 0) {
          err = TRANSPORTDEC_PARSE_ERROR;
        }
      }
    }

    /* if an error is detected terminate config parsing to avoid that an invalid
     * config is accepted in the second pass */
    if (err != TRANSPORTDEC_OK) {
      break;
    }
  }

  if (err == TRANSPORTDEC_OK && fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  return err;
}

TRANSPORTDEC_ERROR transportDec_InBandConfig(HANDLE_TRANSPORTDEC hTp,
                                             UCHAR *newConfig,
                                             const UINT newConfigLength,
                                             const UCHAR buildUpStatus,
                                             UCHAR *configChanged, UINT layer,
                                             UCHAR *implicitExplicitCfgDiff) {
  int errC;
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  int fConfigFound = 0;
  UCHAR configMode = AC_CM_ALLOC_MEM;
  *implicitExplicitCfgDiff = 0;

  FDK_ASSERT(hTp->asc->m_aot == AOT_USAC);

  FDKinitBitStream(hBs, newConfig, TP_USAC_MAX_CONFIG_LEN, newConfigLength << 3,
                   BS_READER);

  if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_FLUSH_OFF) &&
      (hTp->ctrlCFGChange[layer].buildUpStatus !=
       TPDEC_RSV60_BUILD_UP_IDLE_IN_BAND)) {
    if (hTp->asc->m_aot == AOT_USAC) {
      if ((UINT)(hTp->asc->m_sc.m_usacConfig.UsacConfigBits + 7) >> 3 ==
          newConfigLength) {
        if (0 == FDKmemcmp(newConfig, hTp->asc->m_sc.m_usacConfig.UsacConfig,
                           newConfigLength)) {
          if (hTp->parser.latm.usacExplicitCfgChanged) { /* configChange from
                                                            LOAS/LATM parser */
            hTp->parser.latm.usacExplicitCfgChanged = 0;
            hTp->ctrlCFGChange[layer].flushCnt = 0;
            hTp->ctrlCFGChange[layer].flushStatus =
                TPDEC_USAC_DASH_IPF_FLUSH_ON;
            hTp->ctrlCFGChange[layer].buildUpCnt = 0;
            hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
          } else {
            *configChanged = 0;
            return err;
          }
        } else {
          *implicitExplicitCfgDiff = 1;
        }
      } else {
        *implicitExplicitCfgDiff = 1;
      }
      /* ISO/IEC 23003-3:2012/FDAM 3:2016(E) Annex F.2: explicit and implicit
       * config shall be identical. */
      if (*implicitExplicitCfgDiff) {
        switch (hTp->transportFmt) {
          case TT_MP4_LATM_MCP0:
          case TT_MP4_LATM_MCP1:
          case TT_MP4_LOAS:
            /* reset decoder to initial state to achieve definite behavior after
             * error in config */
            hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                     &hTp->asc[layer]);
            hTp->parser.latm.usacExplicitCfgChanged = 0;
            hTp->parser.latm.applyAsc = 1;
            err = TRANSPORTDEC_PARSE_ERROR;
            goto bail;
          default:
            break;
        }
      }
    }
  }

  {
    if ((hTp->ctrlCFGChange[layer].flushStatus == TPDEC_FLUSH_OFF) &&
        (hTp->ctrlCFGChange[layer].buildUpStatus !=
         TPDEC_RSV60_BUILD_UP_IDLE_IN_BAND)) {
      hTp->ctrlCFGChange[layer].flushCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
      if (hTp->asc->m_aot == AOT_USAC) {
        hTp->ctrlCFGChange[layer].flushStatus = TPDEC_USAC_DASH_IPF_FLUSH_ON;
      }
    }

    if ((hTp->ctrlCFGChange[layer].flushStatus ==
         TPDEC_RSV60_DASH_IPF_ATSC_FLUSH_ON) ||
        (hTp->ctrlCFGChange[layer].flushStatus ==
         TPDEC_USAC_DASH_IPF_FLUSH_ON)) {
      SCHAR counter = 0;
      if (hTp->asc->m_aot == AOT_USAC) {
        counter = TPDEC_USAC_NUM_CONFIG_CHANGE_FRAMES;
      }
      if (hTp->ctrlCFGChange[layer].flushCnt >= counter) {
        hTp->ctrlCFGChange[layer].flushCnt = 0;
        hTp->ctrlCFGChange[layer].flushStatus = TPDEC_FLUSH_OFF;
        hTp->ctrlCFGChange[layer].forceCfgChange = 0;
        if (hTp->asc->m_aot == AOT_USAC) {
          hTp->ctrlCFGChange[layer].buildUpCnt =
              TPDEC_USAC_NUM_CONFIG_CHANGE_FRAMES - 1;
          hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_USAC_BUILD_UP_ON;
        }
      }

      /* Activate flush mode. After that continue with build up mode in core */
      if (hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                         &hTp->ctrlCFGChange[layer]) != 0) {
        err = TRANSPORTDEC_PARSE_ERROR;
      }

      if ((hTp->ctrlCFGChange[layer].flushStatus ==
           TPDEC_RSV60_DASH_IPF_ATSC_FLUSH_ON) ||
          (hTp->ctrlCFGChange[layer].flushStatus ==
           TPDEC_USAC_DASH_IPF_FLUSH_ON)) {
        hTp->ctrlCFGChange[layer].flushCnt++;
        return err;
      }
    }

    if (hTp->asc->m_aot == AOT_USAC) {
      fConfigFound = 1;

      if (err == TRANSPORTDEC_OK) {
        *configChanged = 0;
        configMode = AC_CM_DET_CFG_CHANGE;

        for (int i = 0; i < 2; i++) {
          if (i > 0) {
            FDKpushBack(hBs,
                        (INT)newConfigLength * 8 - (INT)FDKgetValidBits(hBs));
            configMode = AC_CM_ALLOC_MEM;
          }
          /* config transport decoder */
          err = AudioSpecificConfig_Parse(
              &hTp->asc[(1 * 1)], hBs, 0, &hTp->callbacks, configMode,
              *configChanged, hTp->asc[layer].m_aot);
          if (err == TRANSPORTDEC_OK) {
            hTp->asc[layer] = hTp->asc[(1 * 1)];
            errC = hTp->callbacks.cbUpdateConfig(
                hTp->callbacks.cbUpdateConfigData, &hTp->asc[layer],
                hTp->asc[layer].configMode, &hTp->asc[layer].AacConfigChanged);
            if (errC != 0) {
              err = TRANSPORTDEC_PARSE_ERROR;
            }
          }

          if (err == TRANSPORTDEC_OK) {
            if ((i == 0) && (hTp->asc[layer].AacConfigChanged ||
                             hTp->asc[layer].SbrConfigChanged ||
                             hTp->asc[layer].SacConfigChanged)) {
              *configChanged = 1;
              errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                              &hTp->asc[layer]);
              if (errC != 0) {
                err = TRANSPORTDEC_PARSE_ERROR;
              }
            }
          }

          /* if an error is detected terminate config parsing to avoid that an
           * invalid config is accepted in the second pass */
          if (err != TRANSPORTDEC_OK) {
            break;
          }
        }
      }
    }

  bail:
    /* save new config */
    if (err == TRANSPORTDEC_OK) {
      if (hTp->asc->m_aot == AOT_USAC) {
        hTp->asc->m_sc.m_usacConfig.UsacConfigBits = newConfigLength << 3;
        FDKmemcpy(hTp->asc->m_sc.m_usacConfig.UsacConfig, newConfig,
                  newConfigLength);
        /* in case of USAC reset transportDecoder variables here because
         * otherwise without IPF they are not reset */
        hTp->ctrlCFGChange[layer].flushCnt = 0;
        hTp->ctrlCFGChange[layer].flushStatus = TPDEC_FLUSH_OFF;
        hTp->ctrlCFGChange[layer].buildUpCnt = 0;
        hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
      }
    } else {
      hTp->numberOfRawDataBlocks = 0;

      /* If parsing error while config found, clear ctrlCFGChange-struct */
      hTp->ctrlCFGChange[layer].flushCnt = 0;
      hTp->ctrlCFGChange[layer].flushStatus = TPDEC_FLUSH_OFF;
      hTp->ctrlCFGChange[layer].buildUpCnt = 0;
      hTp->ctrlCFGChange[layer].buildUpStatus = TPDEC_BUILD_UP_OFF;
      hTp->ctrlCFGChange[layer].cfgChanged = 0;
      hTp->ctrlCFGChange[layer].contentChanged = 0;
      hTp->ctrlCFGChange[layer].forceCfgChange = 0;

      hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                     &hTp->ctrlCFGChange[layer]);
    }
  }

  if (err == TRANSPORTDEC_OK && fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  return err;
}

int transportDec_RegisterAscCallback(HANDLE_TRANSPORTDEC hTpDec,
                                     const cbUpdateConfig_t cbUpdateConfig,
                                     void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbUpdateConfig = cbUpdateConfig;
  hTpDec->callbacks.cbUpdateConfigData = user_data;
  return 0;
}

int transportDec_RegisterFreeMemCallback(HANDLE_TRANSPORTDEC hTpDec,
                                         const cbFreeMem_t cbFreeMem,
                                         void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbFreeMem = cbFreeMem;
  hTpDec->callbacks.cbFreeMemData = user_data;
  return 0;
}

int transportDec_RegisterCtrlCFGChangeCallback(
    HANDLE_TRANSPORTDEC hTpDec, const cbCtrlCFGChange_t cbCtrlCFGChange,
    void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbCtrlCFGChange = cbCtrlCFGChange;
  hTpDec->callbacks.cbCtrlCFGChangeData = user_data;
  return 0;
}

int transportDec_RegisterSscCallback(HANDLE_TRANSPORTDEC hTpDec,
                                     const cbSsc_t cbSsc, void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbSsc = cbSsc;
  hTpDec->callbacks.cbSscData = user_data;
  return 0;
}

int transportDec_RegisterSbrCallback(HANDLE_TRANSPORTDEC hTpDec,
                                     const cbSbr_t cbSbr, void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbSbr = cbSbr;
  hTpDec->callbacks.cbSbrData = user_data;
  return 0;
}

int transportDec_RegisterUsacCallback(HANDLE_TRANSPORTDEC hTpDec,
                                      const cbUsac_t cbUsac, void *user_data) {
  if (hTpDec == NULL) {
    return -1;
  }
  hTpDec->callbacks.cbUsac = cbUsac;
  hTpDec->callbacks.cbUsacData = user_data;
  return 0;
}

int transportDec_RegisterUniDrcConfigCallback(HANDLE_TRANSPORTDEC hTpDec,
                                              const cbUniDrc_t cbUniDrc,
                                              void *user_data,
                                              UINT *pLoudnessInfoSetPosition) {
  if (hTpDec == NULL) {
    return -1;
  }

  hTpDec->callbacks.cbUniDrc = cbUniDrc;
  hTpDec->callbacks.cbUniDrcData = user_data;

  hTpDec->pLoudnessInfoSetPosition = pLoudnessInfoSetPosition;
  return 0;
}

TRANSPORTDEC_ERROR transportDec_FillData(const HANDLE_TRANSPORTDEC hTp,
                                         UCHAR *pBuffer, const UINT bufferSize,
                                         UINT *pBytesValid, const INT layer) {
  HANDLE_FDK_BITSTREAM hBs;

  if ((hTp == NULL) || (layer >= 1)) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  /* set bitbuffer shortcut */
  hBs = &hTp->bitStream[layer];

  if (TT_IS_PACKET(hTp->transportFmt)) {
    if (hTp->numberOfRawDataBlocks == 0) {
      FDKresetBitbuffer(hBs);
      FDKfeedBuffer(hBs, pBuffer, bufferSize, pBytesValid);
      if (*pBytesValid != 0) {
        return TRANSPORTDEC_TOO_MANY_BITS;
      }
    }
  } else {
    /* ... else feed bitbuffer with new stream data (append). */

    if (*pBytesValid == 0) {
      /* nothing to do */
      return TRANSPORTDEC_OK;
    } else {
      const int bytesValid = *pBytesValid;
      FDKfeedBuffer(hBs, pBuffer, bufferSize, pBytesValid);

      if (hTp->numberOfRawDataBlocks > 0) {
        hTp->globalFramePos += (bytesValid - *pBytesValid) * 8;
        hTp->accessUnitAnchor[layer] = FDKgetValidBits(hBs);
      }
    }
  }

  return TRANSPORTDEC_OK;
}

HANDLE_FDK_BITSTREAM transportDec_GetBitstream(const HANDLE_TRANSPORTDEC hTp,
                                               const UINT layer) {
  return &hTp->bitStream[layer];
}

TRANSPORT_TYPE transportDec_GetFormat(const HANDLE_TRANSPORTDEC hTp) {
  return hTp->transportFmt;
}

INT transportDec_GetBufferFullness(const HANDLE_TRANSPORTDEC hTp) {
  INT bufferFullness = -1;

  switch (hTp->transportFmt) {
    case TT_MP4_ADTS:
      if (hTp->parser.adts.bs.adts_fullness != 0x7ff) {
        bufferFullness = hTp->parser.adts.bs.frame_length * 8 +
                         hTp->parser.adts.bs.adts_fullness * 32 *
                             getNumberOfEffectiveChannels(
                                 hTp->parser.adts.bs.channel_config);
      }
      break;
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      if (hTp->parser.latm.m_linfo[0][0].m_bufferFullness != 0xff) {
        bufferFullness = hTp->parser.latm.m_linfo[0][0].m_bufferFullness;
      }
      break;
    default:
      break;
  }

  return bufferFullness;
}

/**
 * \brief adjust bit stream position and the end of an access unit.
 * \param hTp transport decoder handle.
 * \return error code.
 */
static TRANSPORTDEC_ERROR transportDec_AdjustEndOfAccessUnit(
    HANDLE_TRANSPORTDEC hTp) {
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[0];
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  switch (hTp->transportFmt) {
    case TT_MP4_ADIF:
      /* Do byte align at the end of raw_data_block() because UsacFrame() is not
       * byte aligned. */
      FDKbyteAlign(hBs, hTp->accessUnitAnchor[0]);
      break;
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      if (hTp->numberOfRawDataBlocks == 0) {
        /* Do byte align at the end of AudioMuxElement. */
        FDKbyteAlign(hBs, hTp->globalFramePos);

        /* Check global frame length */
        if (hTp->transportFmt == TT_MP4_LOAS &&
            hTp->parser.latm.m_audioMuxLengthBytes > 0) {
          int loasOffset;

          loasOffset = ((INT)hTp->parser.latm.m_audioMuxLengthBytes * 8 +
                        (INT)FDKgetValidBits(hBs)) -
                       (INT)hTp->globalFramePos;
          if (loasOffset != 0) {
            FDKpushBiDirectional(hBs, loasOffset);
            /* For ELD and other payloads there is an unknown amount of padding,
               so ignore unread bits, but throw an error only if too many bits
               where read. */
            if (loasOffset < 0) {
              err = TRANSPORTDEC_PARSE_ERROR;
            }
          }
        }
      }
      break;

    case TT_MP4_ADTS:
      if (hTp->parser.adts.bs.protection_absent == 0) {
        int offset;

        /* Calculate offset to end of AU */
        offset = hTp->parser.adts
                     .rawDataBlockDist[hTp->parser.adts.bs.num_raw_blocks -
                                       hTp->numberOfRawDataBlocks]
                 << 3;
        /* CAUTION: The PCE (if available) is declared to be a part of the
         * header! */
        offset -= (INT)hTp->accessUnitAnchor[0] - (INT)FDKgetValidBits(hBs) +
                  16 + hTp->parser.adts.bs.num_pce_bits;
        FDKpushBiDirectional(hBs, offset);
      }
      if (hTp->parser.adts.bs.num_raw_blocks > 0 &&
          hTp->parser.adts.bs.protection_absent == 0) {
        /* Note this CRC read currently happens twice because of
         * transportDec_CrcCheck() */
        hTp->parser.adts.crcReadValue = FDKreadBits(hBs, 16);
      }
      if (hTp->numberOfRawDataBlocks == 0) {
        /* Check global frame length */
        if (hTp->parser.adts.bs.protection_absent == 0) {
          int offset;

          offset = (hTp->parser.adts.bs.frame_length * 8 - ADTS_SYNCLENGTH +
                    (INT)FDKgetValidBits(hBs)) -
                   (INT)hTp->globalFramePos;
          if (offset != 0) {
            FDKpushBiDirectional(hBs, offset);
          }
        }
      }
      break;

    default:
      break;
  }

  return err;
}

/**
 * \brief Determine additional buffer fullness contraint due to burst data
 * reception. The parameter TPDEC_PARAM_BURSTPERIOD must have been set as a
 * precondition.
 * \param hTp transport decoder handle.
 * \param bufferFullness the buffer fullness value of the first frame to be
 * decoded.
 * \param bitsAvail the amount of available bits at the end of the first frame
 * to be decoded.
 * \return error code
 */
static TRANSPORTDEC_ERROR additionalHoldOffNeeded(HANDLE_TRANSPORTDEC hTp,
                                                  INT bufferFullness,
                                                  INT bitsAvail) {
  INT checkLengthBits, avgBitsPerFrame;
  INT maxAU; /* maximum number of frames per Master Frame */
  INT samplesPerFrame = hTp->asc->m_samplesPerFrame;
  INT samplingFrequency = (INT)hTp->asc->m_samplingFrequency;

  if ((hTp->avgBitRate == 0) || (hTp->burstPeriod == 0)) {
    return TRANSPORTDEC_OK;
  }
  if ((samplesPerFrame == 0) || (samplingFrequency == 0)) {
    return TRANSPORTDEC_NOT_ENOUGH_BITS;
  }

  /* One Master Frame is sent every hTp->burstPeriod ms */
  maxAU = hTp->burstPeriod * samplingFrequency + (samplesPerFrame * 1000 - 1);
  maxAU = maxAU / (samplesPerFrame * 1000);
  /* Subtract number of frames which were already held off. */
  maxAU -= hTp->holdOffFrames;

  avgBitsPerFrame = hTp->avgBitRate * samplesPerFrame + (samplingFrequency - 1);
  avgBitsPerFrame = avgBitsPerFrame / samplingFrequency;

  /* Consider worst case of bufferFullness quantization. */
  switch (hTp->transportFmt) {
    case TT_MP4_ADIF:
    case TT_MP4_ADTS:
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      bufferFullness += 31;
      break;
    default: /* added to avoid compiler warning */
      break; /* added to avoid compiler warning */
  }

  checkLengthBits = bufferFullness + (maxAU - 1) * avgBitsPerFrame;

  /* Check if buffer is big enough to fullfill buffer fullness condition */
  if ((checkLengthBits /*+headerBits*/) > (((8192 * 4) << 3) - 7)) {
    return TRANSPORTDEC_SYNC_ERROR;
  }

  if (bitsAvail < checkLengthBits) {
    return TRANSPORTDEC_NOT_ENOUGH_BITS;
  } else {
    return TRANSPORTDEC_OK;
  }
}

static TRANSPORTDEC_ERROR transportDec_readHeader(
    HANDLE_TRANSPORTDEC hTp, HANDLE_FDK_BITSTREAM hBs, int syncLength,
    int ignoreBufferFullness, int *pRawDataBlockLength,
    int *pfTraverseMoreFrames, int *pSyncLayerFrameBits, int *pfConfigFound,
    int *pHeaderBits) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  int rawDataBlockLength = *pRawDataBlockLength;
  int fTraverseMoreFrames =
      (pfTraverseMoreFrames != NULL) ? *pfTraverseMoreFrames : 0;
  int syncLayerFrameBits =
      (pSyncLayerFrameBits != NULL) ? *pSyncLayerFrameBits : 0;
  int fConfigFound = (pfConfigFound != NULL) ? *pfConfigFound : 0;
  int startPos;

  startPos = (INT)FDKgetValidBits(hBs);

  switch (hTp->transportFmt) {
    case TT_MP4_ADTS:
      if (hTp->numberOfRawDataBlocks <= 0) {
        int i, errC;

        hTp->globalFramePos = FDKgetValidBits(hBs);

        UCHAR configChanged = 0;
        UCHAR configMode = AC_CM_DET_CFG_CHANGE;

        for (i = 0; i < 2; i++) {
          if (i > 0) {
            FDKpushBack(hBs,
                        (INT)hTp->globalFramePos - (INT)FDKgetValidBits(hBs));
            configMode = AC_CM_ALLOC_MEM;
          }

          /* Parse ADTS header */
          err = adtsRead_DecodeHeader(&hTp->parser.adts, &hTp->asc[0], hBs,
                                      ignoreBufferFullness);
          if (err != TRANSPORTDEC_OK) {
            if (err != TRANSPORTDEC_NOT_ENOUGH_BITS) {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
          } else {
            errC = hTp->callbacks.cbUpdateConfig(
                hTp->callbacks.cbUpdateConfigData, &hTp->asc[0], configMode,
                &configChanged);
            if (errC != 0) {
              if (errC == TRANSPORTDEC_NEED_TO_RESTART) {
                err = TRANSPORTDEC_NEED_TO_RESTART;
                goto bail;
              } else {
                err = TRANSPORTDEC_SYNC_ERROR;
              }
            } else {
              fConfigFound = 1;
              hTp->numberOfRawDataBlocks =
                  hTp->parser.adts.bs.num_raw_blocks + 1;
            }
          }

          if (err == TRANSPORTDEC_OK) {
            if ((i == 0) && configChanged) {
              errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                              &hTp->asc[0]);
              if (errC != 0) {
                err = TRANSPORTDEC_PARSE_ERROR;
              }
            }
          }
          /* if an error is detected terminate config parsing to avoid that an
           * invalid config is accepted in the second pass */
          if (err != TRANSPORTDEC_OK) {
            break;
          }
        }
      } else {
        /* Reset CRC because the next bits are the beginning of a
         * raw_data_block() */
        FDKcrcReset(&hTp->parser.adts.crcInfo);
        hTp->parser.adts.bs.num_pce_bits = 0;
      }
      if (err == TRANSPORTDEC_OK) {
        hTp->numberOfRawDataBlocks--;
        rawDataBlockLength = adtsRead_GetRawDataBlockLength(
            &hTp->parser.adts,
            (hTp->parser.adts.bs.num_raw_blocks - hTp->numberOfRawDataBlocks));
        if (rawDataBlockLength <= 0) {
          /* No further frame traversal possible. */
          fTraverseMoreFrames = 0;
        }
        syncLayerFrameBits = (hTp->parser.adts.bs.frame_length << 3) -
                             (startPos - (INT)FDKgetValidBits(hBs)) -
                             syncLength;
        if (syncLayerFrameBits <= 0) {
          err = TRANSPORTDEC_SYNC_ERROR;
        }
      } else {
        hTp->numberOfRawDataBlocks = 0;
      }
      break;
    case TT_MP4_LOAS:
      if (hTp->numberOfRawDataBlocks <= 0) {
        syncLayerFrameBits = (INT)FDKreadBits(hBs, 13);
        hTp->parser.latm.m_audioMuxLengthBytes = syncLayerFrameBits;
        syncLayerFrameBits <<= 3;
      }
      FDK_FALLTHROUGH;
    case TT_MP4_LATM_MCP1:
    case TT_MP4_LATM_MCP0:
      if (hTp->numberOfRawDataBlocks <= 0) {
        hTp->globalFramePos = FDKgetValidBits(hBs);

        err = CLatmDemux_Read(hBs, &hTp->parser.latm, hTp->transportFmt,
                              &hTp->callbacks, hTp->asc, &fConfigFound,
                              ignoreBufferFullness);

        if (err != TRANSPORTDEC_OK) {
          if ((err != TRANSPORTDEC_NOT_ENOUGH_BITS) &&
              !TPDEC_IS_FATAL_ERROR(err)) {
            err = TRANSPORTDEC_SYNC_ERROR;
          }
        } else {
          hTp->numberOfRawDataBlocks =
              CLatmDemux_GetNrOfSubFrames(&hTp->parser.latm);
          if (hTp->transportFmt == TT_MP4_LOAS) {
            syncLayerFrameBits -= startPos - (INT)FDKgetValidBits(hBs) - (13);
            if (syncLayerFrameBits <= 0) {
              err = TRANSPORTDEC_SYNC_ERROR;
            }
          }
        }
      } else {
        err = CLatmDemux_ReadPayloadLengthInfo(hBs, &hTp->parser.latm);
        if (err != TRANSPORTDEC_OK) {
          err = TRANSPORTDEC_SYNC_ERROR;
        }
      }
      if (err == TRANSPORTDEC_OK) {
        int layer;
        rawDataBlockLength = 0;
        for (layer = 0;
             layer < (int)CLatmDemux_GetNrOfLayers(&hTp->parser.latm, 0);
             layer += 1) {
          rawDataBlockLength +=
              CLatmDemux_GetFrameLengthInBits(&hTp->parser.latm, 0, layer);
        }
        hTp->numberOfRawDataBlocks--;
      } else {
        hTp->numberOfRawDataBlocks = 0;
      }
      break;
    default: { syncLayerFrameBits = 0; } break;
  }

bail:

  *pRawDataBlockLength = rawDataBlockLength;

  if (pHeaderBits != NULL) {
    *pHeaderBits += startPos - (INT)FDKgetValidBits(hBs);
  }

  for (int i = 0; i < (1 * 1); i++) {
    /* If parsing error while config found, clear ctrlCFGChange-struct */
    if (hTp->ctrlCFGChange[i].cfgChanged && err != TRANSPORTDEC_OK) {
      hTp->numberOfRawDataBlocks = 0;
      hTp->ctrlCFGChange[i].flushCnt = 0;
      hTp->ctrlCFGChange[i].flushStatus = TPDEC_FLUSH_OFF;
      hTp->ctrlCFGChange[i].buildUpCnt = 0;
      hTp->ctrlCFGChange[i].buildUpStatus = TPDEC_BUILD_UP_OFF;
      hTp->ctrlCFGChange[i].cfgChanged = 0;
      hTp->ctrlCFGChange[i].contentChanged = 0;
      hTp->ctrlCFGChange[i].forceCfgChange = 0;

      hTp->callbacks.cbCtrlCFGChange(hTp->callbacks.cbCtrlCFGChangeData,
                                     &hTp->ctrlCFGChange[i]);
    }
  }

  if (pfConfigFound != NULL) {
    *pfConfigFound = fConfigFound;
  }

  if (pfTraverseMoreFrames != NULL) {
    *pfTraverseMoreFrames = fTraverseMoreFrames;
  }
  if (pSyncLayerFrameBits != NULL) {
    *pSyncLayerFrameBits = syncLayerFrameBits;
  }

  return err;
}

/* How many bits to advance for synchronization search. */
#define TPDEC_SYNCSKIP 8

static TRANSPORTDEC_ERROR synchronization(HANDLE_TRANSPORTDEC hTp,
                                          INT *pHeaderBits) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK, errFirstFrame = TRANSPORTDEC_OK;
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[0];

  INT syncLayerFrameBits = 0; /* Length of sync layer frame (i.e. LOAS) */
  INT rawDataBlockLength = 0, rawDataBlockLengthPrevious;
  INT totalBits;
  INT headerBits = 0, headerBitsFirstFrame = 0, headerBitsPrevious;
  INT numFramesTraversed = 0, fTraverseMoreFrames,
      fConfigFound = (hTp->flags & TPDEC_CONFIG_FOUND), startPosFirstFrame = -1;
  INT numRawDataBlocksFirstFrame = 0, numRawDataBlocksPrevious,
      globalFramePosFirstFrame = 0, rawDataBlockLengthFirstFrame = 0;
  INT ignoreBufferFullness =
      hTp->flags &
      (TPDEC_LOST_FRAMES_PENDING | TPDEC_IGNORE_BUFFERFULLNESS | TPDEC_SYNCOK);
  UINT endTpFrameBitsPrevious = 0;

  /* Synch parameters */
  INT syncLength; /* Length of sync word in bits */
  UINT syncWord;  /* Sync word to be found */
  UINT syncMask;  /* Mask for sync word (for adding one bit, so comprising one
                     bit less) */
  C_ALLOC_SCRATCH_START(contextFirstFrame, transportdec_parser_t, 1);

  totalBits = (INT)FDKgetValidBits(hBs);

  if (totalBits <= 0) {
    err = TRANSPORTDEC_NOT_ENOUGH_BITS;
    goto bail;
  }

  fTraverseMoreFrames =
      (hTp->flags & (TPDEC_MINIMIZE_DELAY | TPDEC_EARLY_CONFIG)) &&
      !(hTp->flags & TPDEC_SYNCOK);

  /* Set transport specific sync parameters */
  switch (hTp->transportFmt) {
    case TT_MP4_ADTS:
      syncWord = ADTS_SYNCWORD;
      syncLength = ADTS_SYNCLENGTH;
      break;
    case TT_MP4_LOAS:
      syncWord = 0x2B7;
      syncLength = 11;
      break;
    default:
      syncWord = 0;
      syncLength = 0;
      break;
  }

  syncMask = (1 << syncLength) - 1;

  do {
    INT bitsAvail = 0;   /* Bits available in bitstream buffer    */
    INT checkLengthBits; /* Helper to check remaining bits and buffer boundaries
                          */
    UINT synch;          /* Current sync word read from bitstream */

    headerBitsPrevious = headerBits;

    bitsAvail = (INT)FDKgetValidBits(hBs);

    if (hTp->numberOfRawDataBlocks == 0) {
      /* search synchword */

      FDK_ASSERT((bitsAvail % TPDEC_SYNCSKIP) == 0);

      if ((bitsAvail - syncLength) < TPDEC_SYNCSKIP) {
        err = TRANSPORTDEC_NOT_ENOUGH_BITS;
        headerBits = 0;
      } else {
        synch = FDKreadBits(hBs, syncLength);

        if (!(hTp->flags & TPDEC_SYNCOK)) {
          for (; (bitsAvail - syncLength) >= TPDEC_SYNCSKIP;
               bitsAvail -= TPDEC_SYNCSKIP) {
            if (synch == syncWord) {
              break;
            }
            synch = ((synch << TPDEC_SYNCSKIP) & syncMask) |
                    FDKreadBits(hBs, TPDEC_SYNCSKIP);
          }
        }
        if (synch != syncWord) {
          /* No correct syncword found. */
          err = TRANSPORTDEC_SYNC_ERROR;
        } else {
          err = TRANSPORTDEC_OK;
        }
        headerBits = syncLength;
      }
    } else {
      headerBits = 0;
    }

    /* Save previous raw data block data */
    rawDataBlockLengthPrevious = rawDataBlockLength;
    numRawDataBlocksPrevious = hTp->numberOfRawDataBlocks;

    /* Parse transport header (raw data block granularity) */

    if (err == TRANSPORTDEC_OK) {
      err = transportDec_readHeader(hTp, hBs, syncLength, ignoreBufferFullness,
                                    &rawDataBlockLength, &fTraverseMoreFrames,
                                    &syncLayerFrameBits, &fConfigFound,
                                    &headerBits);
      if (headerBits > bitsAvail) {
        err = (headerBits < (INT)hBs->hBitBuf.bufBits)
                  ? TRANSPORTDEC_NOT_ENOUGH_BITS
                  : TRANSPORTDEC_SYNC_ERROR;
      }
      if (TPDEC_IS_FATAL_ERROR(err)) {
        /* Rewind - TPDEC_SYNCSKIP, in order to look for a synch one bit ahead
         * next time. Ensure that the bit amount lands at a multiple of
         * TPDEC_SYNCSKIP. */
        FDKpushBiDirectional(
            hBs, -headerBits + TPDEC_SYNCSKIP + (bitsAvail % TPDEC_SYNCSKIP));

        goto bail;
      }
    }

    bitsAvail -= headerBits;

    checkLengthBits = syncLayerFrameBits;

    /* Check if the whole frame would fit the bitstream buffer */
    if (err == TRANSPORTDEC_OK) {
      if ((checkLengthBits + headerBits) > (((8192 * 4) << 3) - 7)) {
        /* We assume that the size of the transport bit buffer has been
           chosen to meet all system requirements, thus this condition
           is considered a synchronisation error. */
        err = TRANSPORTDEC_SYNC_ERROR;
      } else {
        if (bitsAvail < checkLengthBits) {
          err = TRANSPORTDEC_NOT_ENOUGH_BITS;
        }
      }
    }

    if (err == TRANSPORTDEC_NOT_ENOUGH_BITS) {
      break;
    }

    if (err == TRANSPORTDEC_SYNC_ERROR) {
      int bits;

      /* Enforce re-sync of transport headers. */
      hTp->numberOfRawDataBlocks = 0;

      /* Ensure that the bit amount lands at a multiple of TPDEC_SYNCSKIP */
      bits = (bitsAvail + headerBits) % TPDEC_SYNCSKIP;
      /* Rewind - TPDEC_SYNCSKIP, in order to look for a synch one bit ahead
       * next time. */
      FDKpushBiDirectional(hBs, -(headerBits - TPDEC_SYNCSKIP) + bits);
      headerBits = 0;
    }

    /* Frame traversal */
    if (fTraverseMoreFrames) {
      /* Save parser context for early config discovery "rewind all frames" */
      if ((hTp->flags & TPDEC_EARLY_CONFIG) &&
          !(hTp->flags & TPDEC_MINIMIZE_DELAY)) {
        /* ignore buffer fullness if just traversing additional frames for ECD
         */
        ignoreBufferFullness = 1;

        /* Save context in order to return later */
        if (err == TRANSPORTDEC_OK && startPosFirstFrame == -1) {
          startPosFirstFrame = FDKgetValidBits(hBs);
          numRawDataBlocksFirstFrame = hTp->numberOfRawDataBlocks;
          globalFramePosFirstFrame = hTp->globalFramePos;
          rawDataBlockLengthFirstFrame = rawDataBlockLength;
          headerBitsFirstFrame = headerBits;
          errFirstFrame = err;
          FDKmemcpy(contextFirstFrame, &hTp->parser,
                    sizeof(transportdec_parser_t));
        }

        /* Break when config was found or it is not possible anymore to find a
         * config */
        if (startPosFirstFrame != -1 &&
            (fConfigFound || err != TRANSPORTDEC_OK)) {
          /* In case of ECD and sync error, do not rewind anywhere. */
          if (err == TRANSPORTDEC_SYNC_ERROR) {
            startPosFirstFrame = -1;
            fConfigFound = 0;
            numFramesTraversed = 0;
          }
          break;
        }
      }

      if (err == TRANSPORTDEC_OK) {
        FDKpushFor(hBs, rawDataBlockLength);
        numFramesTraversed++;
        endTpFrameBitsPrevious = (INT)FDKgetValidBits(hBs);
        /* Ignore error here itentionally. */
        transportDec_AdjustEndOfAccessUnit(hTp);
        endTpFrameBitsPrevious -= FDKgetValidBits(hBs);
      }
    }
  } while (fTraverseMoreFrames ||
           (err == TRANSPORTDEC_SYNC_ERROR && !(hTp->flags & TPDEC_SYNCOK)));

  /* Restore context in case of ECD frame traversal */
  if (startPosFirstFrame != -1 && (fConfigFound || err != TRANSPORTDEC_OK)) {
    FDKpushBiDirectional(hBs, FDKgetValidBits(hBs) - startPosFirstFrame);
    FDKmemcpy(&hTp->parser, contextFirstFrame, sizeof(transportdec_parser_t));
    hTp->numberOfRawDataBlocks = numRawDataBlocksFirstFrame;
    hTp->globalFramePos = globalFramePosFirstFrame;
    rawDataBlockLength = rawDataBlockLengthFirstFrame;
    headerBits = headerBitsFirstFrame;
    err = errFirstFrame;
    numFramesTraversed = 0;
  }

  /* Additional burst data mode buffer fullness check. */
  if (!(hTp->flags & (TPDEC_LOST_FRAMES_PENDING | TPDEC_IGNORE_BUFFERFULLNESS |
                      TPDEC_SYNCOK)) &&
      err == TRANSPORTDEC_OK) {
    err =
        additionalHoldOffNeeded(hTp, transportDec_GetBufferFullness(hTp),
                                (INT)FDKgetValidBits(hBs) - syncLayerFrameBits);
    if (err == TRANSPORTDEC_NOT_ENOUGH_BITS) {
      hTp->holdOffFrames++;
    }
  }

  /* Rewind for retry because of not enough bits */
  if (err == TRANSPORTDEC_NOT_ENOUGH_BITS) {
    FDKpushBack(hBs, headerBits);
    hTp->numberOfRawDataBlocks = numRawDataBlocksPrevious;
    headerBits = 0;
    rawDataBlockLength = rawDataBlockLengthPrevious;
  } else {
    /* reset hold off frame counter */
    hTp->holdOffFrames = 0;
  }

  /* Return to last good frame in case of frame traversal but not ECD. */
  if (numFramesTraversed > 0) {
    FDKpushBack(hBs, rawDataBlockLengthPrevious + endTpFrameBitsPrevious);
    if (err != TRANSPORTDEC_OK) {
      hTp->numberOfRawDataBlocks = numRawDataBlocksPrevious;
      headerBits = headerBitsPrevious;
      rawDataBlockLength = rawDataBlockLengthPrevious;
    }
    err = TRANSPORTDEC_OK;
  }

bail:
  hTp->auLength[0] = rawDataBlockLength;

  /* Detect pointless TRANSPORTDEC_NOT_ENOUGH_BITS error case, where the bit
     buffer is already full, or no new burst packet fits. Recover by advancing
     the bit buffer. */
  if ((totalBits > 0) && (TRANSPORTDEC_NOT_ENOUGH_BITS == err) &&
      (FDKgetValidBits(hBs) >=
       (((8192 * 4) * 8 - ((hTp->avgBitRate * hTp->burstPeriod) / 1000)) -
        7))) {
    FDKpushFor(hBs, TPDEC_SYNCSKIP);
    err = TRANSPORTDEC_SYNC_ERROR;
  }

  if (err == TRANSPORTDEC_OK) {
    hTp->flags |= TPDEC_SYNCOK;
  }

  if (fConfigFound) {
    hTp->flags |= TPDEC_CONFIG_FOUND;
  }

  if (pHeaderBits != NULL) {
    *pHeaderBits = headerBits;
  }

  if (err == TRANSPORTDEC_SYNC_ERROR) {
    hTp->flags &= ~TPDEC_SYNCOK;
  }

  C_ALLOC_SCRATCH_END(contextFirstFrame, transportdec_parser_t, 1);

  return err;
}

/**
 * \brief Synchronize to stream and estimate the amount of missing access units
 * due to a current synchronization error in case of constant average bit rate.
 */
static TRANSPORTDEC_ERROR transportDec_readStream(HANDLE_TRANSPORTDEC hTp,
                                                  const UINT layer) {
  TRANSPORTDEC_ERROR error = TRANSPORTDEC_OK;
  HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[layer];

  INT headerBits;
  INT bitDistance, bfDelta;

  /* Obtain distance to next synch word */
  bitDistance = (INT)FDKgetValidBits(hBs);
  error = synchronization(hTp, &headerBits);
  bitDistance -= (INT)FDKgetValidBits(hBs);

  FDK_ASSERT(bitDistance >= 0);

  INT nAU = -1;

  if (error == TRANSPORTDEC_SYNC_ERROR ||
      (hTp->flags & TPDEC_LOST_FRAMES_PENDING)) {
    /* Check if estimating lost access units is feasible. */
    if (hTp->avgBitRate > 0 && hTp->asc[0].m_samplesPerFrame > 0 &&
        hTp->asc[0].m_samplingFrequency > 0) {
      if (error == TRANSPORTDEC_OK) {
        int aj;

        aj = transportDec_GetBufferFullness(hTp);
        if (aj > 0) {
          bfDelta = aj;
        } else {
          bfDelta = 0;
        }
        /* sync was ok: last of a series of bad access units. */
        hTp->flags &= ~TPDEC_LOST_FRAMES_PENDING;
        /* Add up bitDistance until end of the current frame. Later we substract
           this frame from the grand total, since this current successfully
           synchronized frame should not be skipped of course; but it must be
           accounted into the bufferfulness math. */
        bitDistance += hTp->auLength[0];
      } else {
        if (!(hTp->flags & TPDEC_LOST_FRAMES_PENDING)) {
          /* sync not ok: one of many bad access units. */
          hTp->flags |= TPDEC_LOST_FRAMES_PENDING;
          bfDelta = -(INT)hTp->lastValidBufferFullness;
        } else {
          bfDelta = 0;
        }
      }

      {
        int num, denom;

        /* Obtain estimate of number of lost frames */
        num = (INT)hTp->asc[0].m_samplingFrequency * (bfDelta + bitDistance) +
              hTp->remainder;
        denom = hTp->avgBitRate * hTp->asc[0].m_samplesPerFrame;
        if (num > 0) {
          nAU = num / denom;
          hTp->remainder = num % denom;
        } else {
          hTp->remainder = num;
        }

        if (error == TRANSPORTDEC_OK) {
          /* Final adjustment of remainder, taken -1 into account because
             current frame should not be skipped, thus substract -1 or do
             nothing instead of +1-1 accordingly. */
          if ((denom - hTp->remainder) >= hTp->remainder) {
            nAU--;
          }

          if (nAU < 0) {
            /* There was one frame too much concealed, so unfortunately we will
             * have to skip one good frame. */
            transportDec_EndAccessUnit(hTp);
            error = synchronization(hTp, &headerBits);
            nAU = -1;
          }
          hTp->remainder = 0;
          /* Enforce last missed frames to be concealed. */
          if (nAU > 0) {
            FDKpushBack(hBs, headerBits);
          }
        }
      }
    }
  }

  /* Be sure that lost frames are handled correctly. This is necessary due to
     some sync error sequences where later it turns out that there is not enough
     data, but the bits upto the sync word are discarded, thus causing a value
     of nAU > 0 */
  if (nAU > 0) {
    error = TRANSPORTDEC_SYNC_ERROR;
  }

  hTp->missingAccessUnits = nAU;

  return error;
}

/* returns error code */
TRANSPORTDEC_ERROR transportDec_ReadAccessUnit(const HANDLE_TRANSPORTDEC hTp,
                                               const UINT layer) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;
  HANDLE_FDK_BITSTREAM hBs;

  if (!hTp) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  hBs = &hTp->bitStream[layer];

  if ((INT)FDKgetValidBits(hBs) <= 0) {
    /* This is only relevant for RAW and ADIF cases.
     * For streaming formats err will get overwritten. */
    err = TRANSPORTDEC_NOT_ENOUGH_BITS;
    hTp->numberOfRawDataBlocks = 0;
  }

  switch (hTp->transportFmt) {
    case TT_MP4_ADIF:
      /* Read header if not already done */
      if (!(hTp->flags & TPDEC_CONFIG_FOUND)) {
        int i;
        CProgramConfig *pce;
        INT bsStart = FDKgetValidBits(hBs);
        UCHAR configChanged = 0;
        UCHAR configMode = AC_CM_DET_CFG_CHANGE;

        for (i = 0; i < 2; i++) {
          if (i > 0) {
            FDKpushBack(hBs, bsStart - (INT)FDKgetValidBits(hBs));
            configMode = AC_CM_ALLOC_MEM;
          }

          AudioSpecificConfig_Init(&hTp->asc[0]);
          pce = &hTp->asc[0].m_progrConfigElement;
          err = adifRead_DecodeHeader(&hTp->parser.adif, pce, hBs);
          if (err) goto bail;

          /* Map adif header to ASC */
          hTp->asc[0].m_aot = (AUDIO_OBJECT_TYPE)(pce->Profile + 1);
          hTp->asc[0].m_samplingFrequencyIndex = pce->SamplingFrequencyIndex;
          hTp->asc[0].m_samplingFrequency =
              SamplingRateTable[pce->SamplingFrequencyIndex];
          hTp->asc[0].m_channelConfiguration = 0;
          hTp->asc[0].m_samplesPerFrame = 1024;
          hTp->avgBitRate = hTp->parser.adif.BitRate;

          /* Call callback to decoder. */
          {
            int errC;

            errC = hTp->callbacks.cbUpdateConfig(
                hTp->callbacks.cbUpdateConfigData, &hTp->asc[0], configMode,
                &configChanged);
            if (errC == 0) {
              hTp->flags |= TPDEC_CONFIG_FOUND;
            } else {
              err = TRANSPORTDEC_PARSE_ERROR;
              goto bail;
            }
          }

          if (err == TRANSPORTDEC_OK) {
            if ((i == 0) && configChanged) {
              int errC;
              errC = hTp->callbacks.cbFreeMem(hTp->callbacks.cbFreeMemData,
                                              &hTp->asc[0]);
              if (errC != 0) {
                err = TRANSPORTDEC_PARSE_ERROR;
              }
            }
          }
        }
      }
      hTp->auLength[layer] = -1; /* Access Unit data length is unknown. */
      break;

    case TT_MP4_RAW:
    case TT_DRM:
      /* One Access Unit was filled into buffer.
         So get the length out of the buffer. */
      hTp->auLength[layer] = FDKgetValidBits(hBs);
      hTp->flags |= TPDEC_SYNCOK;
      break;

    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      if (err == TRANSPORTDEC_OK) {
        int fConfigFound = hTp->flags & TPDEC_CONFIG_FOUND;
        err = transportDec_readHeader(hTp, hBs, 0, 1, &hTp->auLength[layer],
                                      NULL, NULL, &fConfigFound, NULL);
        if (fConfigFound) {
          hTp->flags |= TPDEC_CONFIG_FOUND;
        }
      }
      break;

    case TT_MP4_ADTS:
    case TT_MP4_LOAS:
      err = transportDec_readStream(hTp, layer);
      break;

    default:
      err = TRANSPORTDEC_UNSUPPORTED_FORMAT;
      break;
  }

  if (err == TRANSPORTDEC_OK) {
    hTp->accessUnitAnchor[layer] = FDKgetValidBits(hBs);
  } else {
    hTp->accessUnitAnchor[layer] = 0;
  }

bail:
  return err;
}

TRANSPORTDEC_ERROR transportDec_GetAsc(const HANDLE_TRANSPORTDEC hTp,
                                       const UINT layer,
                                       CSAudioSpecificConfig *asc) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  if (hTp != NULL) {
    *asc = hTp->asc[layer];
    err = TRANSPORTDEC_OK;
  } else {
    err = TRANSPORTDEC_INVALID_PARAMETER;
  }
  return err;
}

INT transportDec_GetAuBitsRemaining(const HANDLE_TRANSPORTDEC hTp,
                                    const UINT layer) {
  INT bits;

  if (hTp->accessUnitAnchor[layer] > 0 && hTp->auLength[layer] > 0) {
    bits = (INT)FDKgetValidBits(&hTp->bitStream[layer]);
    if (bits >= 0) {
      bits = hTp->auLength[layer] - ((INT)hTp->accessUnitAnchor[layer] - bits);
    }
  } else {
    bits = FDKgetValidBits(&hTp->bitStream[layer]);
  }

  return bits;
}

INT transportDec_GetAuBitsTotal(const HANDLE_TRANSPORTDEC hTp,
                                const UINT layer) {
  return hTp->auLength[layer];
}

TRANSPORTDEC_ERROR transportDec_GetMissingAccessUnitCount(
    INT *pNAccessUnits, HANDLE_TRANSPORTDEC hTp) {
  *pNAccessUnits = hTp->missingAccessUnits;

  return TRANSPORTDEC_OK;
}

/* Inform the transportDec layer that reading of access unit has finished. */
TRANSPORTDEC_ERROR transportDec_EndAccessUnit(HANDLE_TRANSPORTDEC hTp) {
  TRANSPORTDEC_ERROR err = TRANSPORTDEC_OK;

  switch (hTp->transportFmt) {
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1: {
      HANDLE_FDK_BITSTREAM hBs = &hTp->bitStream[0];
      if (hTp->numberOfRawDataBlocks == 0) {
        /* Read other data if available. */
        if (CLatmDemux_GetOtherDataPresentFlag(&hTp->parser.latm)) {
          int otherDataLen = CLatmDemux_GetOtherDataLength(&hTp->parser.latm);

          if ((INT)FDKgetValidBits(hBs) >= otherDataLen) {
            FDKpushFor(hBs, otherDataLen);
          } else {
            /* Do byte align at the end of AudioMuxElement. */
            if (hTp->numberOfRawDataBlocks == 0) {
              FDKbyteAlign(hBs, hTp->globalFramePos);
            }
            return TRANSPORTDEC_NOT_ENOUGH_BITS;
          }
        }
      } else {
        /* If bit buffer has not more bits but hTp->numberOfRawDataBlocks > 0
           then too many bits were read and obviously no more RawDataBlocks can
           be read. Set numberOfRawDataBlocks to zero to attempt a new sync
           attempt. */
        if ((INT)FDKgetValidBits(hBs) <= 0) {
          hTp->numberOfRawDataBlocks = 0;
        }
      }
    } break;
    default:
      break;
  }

  err = transportDec_AdjustEndOfAccessUnit(hTp);

  switch (hTp->transportFmt) {
    default:
      break;
  }

  return err;
}

TRANSPORTDEC_ERROR transportDec_SetParam(const HANDLE_TRANSPORTDEC hTp,
                                         const TPDEC_PARAM param,
                                         const INT value) {
  TRANSPORTDEC_ERROR error = TRANSPORTDEC_OK;

  if (hTp == NULL) {
    return TRANSPORTDEC_INVALID_PARAMETER;
  }

  switch (param) {
    case TPDEC_PARAM_MINIMIZE_DELAY:
      if (value) {
        hTp->flags |= TPDEC_MINIMIZE_DELAY;
      } else {
        hTp->flags &= ~TPDEC_MINIMIZE_DELAY;
      }
      break;
    case TPDEC_PARAM_EARLY_CONFIG:
      if (value) {
        hTp->flags |= TPDEC_EARLY_CONFIG;
      } else {
        hTp->flags &= ~TPDEC_EARLY_CONFIG;
      }
      break;
    case TPDEC_PARAM_IGNORE_BUFFERFULLNESS:
      if (value) {
        hTp->flags |= TPDEC_IGNORE_BUFFERFULLNESS;
      } else {
        hTp->flags &= ~TPDEC_IGNORE_BUFFERFULLNESS;
      }
      break;
    case TPDEC_PARAM_SET_BITRATE:
      hTp->avgBitRate = value;
      break;
    case TPDEC_PARAM_BURST_PERIOD:
      hTp->burstPeriod = value;
      break;
    case TPDEC_PARAM_RESET: {
      int i;

      for (i = 0; i < (1 * 1); i++) {
        FDKresetBitbuffer(&hTp->bitStream[i]);
        hTp->auLength[i] = 0;
        hTp->accessUnitAnchor[i] = 0;
      }
      hTp->flags &= ~(TPDEC_SYNCOK | TPDEC_LOST_FRAMES_PENDING);
      if (hTp->transportFmt != TT_MP4_ADIF) {
        hTp->flags &= ~TPDEC_CONFIG_FOUND;
      }
      hTp->remainder = 0;
      hTp->avgBitRate = 0;
      hTp->missingAccessUnits = 0;
      hTp->numberOfRawDataBlocks = 0;
      hTp->globalFramePos = 0;
      hTp->holdOffFrames = 0;
    } break;
    case TPDEC_PARAM_TARGETLAYOUT:
      hTp->targetLayout = value;
      break;
    case TPDEC_PARAM_FORCE_CONFIG_CHANGE:
      hTp->ctrlCFGChange[value].forceCfgChange = TPDEC_FORCE_CONFIG_CHANGE;
      break;
    case TPDEC_PARAM_USE_ELEM_SKIPPING:
      if (value) {
        hTp->flags |= TPDEC_USE_ELEM_SKIPPING;
      } else {
        hTp->flags &= ~TPDEC_USE_ELEM_SKIPPING;
      }
      break;
  }

  return error;
}

UINT transportDec_GetNrOfSubFrames(HANDLE_TRANSPORTDEC hTp) {
  UINT nSubFrames = 0;

  if (hTp == NULL) return 0;

  if (hTp->transportFmt == TT_MP4_LATM_MCP1 ||
      hTp->transportFmt == TT_MP4_LATM_MCP0 || hTp->transportFmt == TT_MP4_LOAS)
    nSubFrames = CLatmDemux_GetNrOfSubFrames(&hTp->parser.latm);
  else if (hTp->transportFmt == TT_MP4_ADTS)
    nSubFrames = hTp->parser.adts.bs.num_raw_blocks;

  return nSubFrames;
}

void transportDec_Close(HANDLE_TRANSPORTDEC *phTp) {
  if (phTp != NULL) {
    if (*phTp != NULL) {
      FreeRam_TransportDecoderBuffer(&(*phTp)->bsBuffer);
      FreeRam_TransportDecoder(phTp);
    }
  }
}

TRANSPORTDEC_ERROR transportDec_GetLibInfo(LIB_INFO *info) {
  int i;

  if (info == NULL) {
    return TRANSPORTDEC_UNKOWN_ERROR;
  }

  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) return TRANSPORTDEC_UNKOWN_ERROR;
  info += i;

  info->module_id = FDK_TPDEC;
#ifdef SUPPRESS_BUILD_DATE_INFO
  info->build_date = "";
  info->build_time = "";
#else
  info->build_date = __DATE__;
  info->build_time = __TIME__;
#endif
  info->title = TP_LIB_TITLE;
  info->version = LIB_VERSION(TP_LIB_VL0, TP_LIB_VL1, TP_LIB_VL2);
  LIB_VERSION_STRING(info);
  info->flags = 0 | CAPF_ADIF | CAPF_ADTS | CAPF_LATM | CAPF_LOAS |
                CAPF_RAWPACKETS | CAPF_DRM;

  return TRANSPORTDEC_OK; /* FDKERR_NOERROR; */
}

int transportDec_CrcStartReg(HANDLE_TRANSPORTDEC pTp, INT mBits) {
  switch (pTp->transportFmt) {
    case TT_MP4_ADTS:
      return adtsRead_CrcStartReg(&pTp->parser.adts, &pTp->bitStream[0], mBits);
    case TT_DRM:
      return drmRead_CrcStartReg(&pTp->parser.drm, &pTp->bitStream[0], mBits);
    default:
      return -1;
  }
}

void transportDec_CrcEndReg(HANDLE_TRANSPORTDEC pTp, INT reg) {
  switch (pTp->transportFmt) {
    case TT_MP4_ADTS:
      adtsRead_CrcEndReg(&pTp->parser.adts, &pTp->bitStream[0], reg);
      break;
    case TT_DRM:
      drmRead_CrcEndReg(&pTp->parser.drm, &pTp->bitStream[0], reg);
      break;
    default:
      break;
  }
}

TRANSPORTDEC_ERROR transportDec_CrcCheck(HANDLE_TRANSPORTDEC pTp) {
  switch (pTp->transportFmt) {
    case TT_MP4_ADTS:
      if ((pTp->parser.adts.bs.num_raw_blocks > 0) &&
          (pTp->parser.adts.bs.protection_absent == 0)) {
        transportDec_AdjustEndOfAccessUnit(pTp);
      }
      return adtsRead_CrcCheck(&pTp->parser.adts);
    case TT_DRM:
      return drmRead_CrcCheck(&pTp->parser.drm);
    default:
      return TRANSPORTDEC_OK;
  }
}

TRANSPORTDEC_ERROR transportDec_DrmRawSdcAudioConfig_Check(UCHAR *conf,
                                                           const UINT length) {
  CSAudioSpecificConfig asc;
  FDK_BITSTREAM bs;
  HANDLE_FDK_BITSTREAM hBs = &bs;

  FDKinitBitStream(hBs, conf, BUFSIZE_DUMMY_VALUE, length << 3, BS_READER);

  TRANSPORTDEC_ERROR err =
      DrmRawSdcAudioConfig_Parse(&asc, hBs, NULL, (UCHAR)AC_CM_ALLOC_MEM, 0);

  return err;
}
