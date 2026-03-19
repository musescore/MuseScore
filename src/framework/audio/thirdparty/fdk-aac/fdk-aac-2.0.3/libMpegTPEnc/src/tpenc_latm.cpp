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

   Author(s):

   Description:

*******************************************************************************/

#include "tpenc_latm.h"

#include "genericStds.h"

static const short celpFrameLengthTable[64] = {
    154, 170, 186, 147, 156, 165, 114, 120, 186, 126, 132, 138, 142,
    146, 154, 166, 174, 182, 190, 198, 206, 210, 214, 110, 114, 118,
    120, 122, 218, 230, 242, 254, 266, 278, 286, 294, 318, 342, 358,
    374, 390, 406, 422, 136, 142, 148, 154, 160, 166, 170, 174, 186,
    198, 206, 214, 222, 230, 238, 216, 160, 280, 338, 0,   0};

/*******
 write value to transport stream
 first two bits define the size of the value itself
 then the value itself, with a size of 0-3 bytes
*******/
static UINT transportEnc_LatmWriteValue(HANDLE_FDK_BITSTREAM hBs, int value) {
  UCHAR valueBytes = 4;
  unsigned int bitsWritten = 0;
  int i;

  if (value < (1 << 8)) {
    valueBytes = 1;
  } else if (value < (1 << 16)) {
    valueBytes = 2;
  } else if (value < (1 << 24)) {
    valueBytes = 3;
  } else {
    valueBytes = 4;
  }

  FDKwriteBits(hBs, valueBytes - 1, 2); /* size of value in Bytes */
  for (i = 0; i < valueBytes; i++) {
    /* write most significant Byte first */
    FDKwriteBits(hBs, (UCHAR)(value >> ((valueBytes - 1 - i) << 3)), 8);
  }

  bitsWritten = (valueBytes << 3) + 2;

  return bitsWritten;
}

static UINT transportEnc_LatmCountFixBitDemandHeader(HANDLE_LATM_STREAM hAss) {
  int bitDemand = 0;
  int insertSetupData = 0;

  /* only if start of new latm frame */
  if (hAss->subFrameCnt == 0) {
    /* AudioSyncStream */

    if (hAss->tt == TT_MP4_LOAS) {
      bitDemand += 11; /* syncword */
      bitDemand += 13; /* audioMuxLengthBytes */
    }

    /* AudioMuxElement*/

    /* AudioMuxElement::Stream Mux Config */
    if (hAss->muxConfigPeriod > 0) {
      insertSetupData = (hAss->latmFrameCounter == 0);
    } else {
      insertSetupData = 0;
    }

    if (hAss->tt != TT_MP4_LATM_MCP0) {
      /* AudioMuxElement::useSameStreamMux Flag */
      bitDemand += 1;

      if (insertSetupData) {
        bitDemand += hAss->streamMuxConfigBits;
      }
    }

    /* AudioMuxElement::otherDataBits */
    bitDemand += hAss->otherDataLenBits;

    /* AudioMuxElement::ByteAlign */
    if (bitDemand % 8) {
      hAss->fillBits = 8 - (bitDemand % 8);
      bitDemand += hAss->fillBits;
    } else {
      hAss->fillBits = 0;
    }
  }

  return bitDemand;
}

static UINT transportEnc_LatmCountVarBitDemandHeader(
    HANDLE_LATM_STREAM hAss, unsigned int streamDataLength) {
  int bitDemand = 0;
  int prog, layer;

  /* Payload Length Info*/
  if (hAss->allStreamsSameTimeFraming) {
    for (prog = 0; prog < hAss->noProgram; prog++) {
      for (layer = 0; layer < LATM_MAX_LAYERS; layer++) {
        LATM_LAYER_INFO *p_linfo = &(hAss->m_linfo[prog][layer]);

        if (p_linfo->streamID >= 0) {
          switch (p_linfo->frameLengthType) {
            case 0:
              if (streamDataLength > 0) {
                streamDataLength -= bitDemand;
                while (streamDataLength >= (255 << 3)) {
                  bitDemand += 8;
                  streamDataLength -= (255 << 3);
                }
                bitDemand += 8;
              }
              break;

            case 1:
            case 4:
            case 6:
              bitDemand += 2;
              break;

            default:
              return 0;
          }
        }
      }
    }
  } else {
    /* there are many possibilities to use this mechanism.  */
    switch (hAss->varMode) {
      case LATMVAR_SIMPLE_SEQUENCE: {
        /* Use the sequence generated by the encoder */
        // int streamCntPosition = transportEnc_SetWritePointer(
        // hAss->hAssemble, 0 );  int streamCntPosition = FDKgetValidBits(
        // hAss->hAssemble );
        bitDemand += 4;

        hAss->varStreamCnt = 0;
        for (prog = 0; prog < hAss->noProgram; prog++) {
          for (layer = 0; layer < LATM_MAX_LAYERS; layer++) {
            LATM_LAYER_INFO *p_linfo = &(hAss->m_linfo[prog][layer]);

            if (p_linfo->streamID >= 0) {
              bitDemand += 4; /* streamID */
              switch (p_linfo->frameLengthType) {
                case 0:
                  streamDataLength -= bitDemand;
                  while (streamDataLength >= (255 << 3)) {
                    bitDemand += 8;
                    streamDataLength -= (255 << 3);
                  }

                  bitDemand += 8;
                  break;
                  /*bitDemand += 1; endFlag
                  break;*/

                case 1:
                case 4:
                case 6:

                  break;

                default:
                  return 0;
              }
              hAss->varStreamCnt++;
            }
          }
        }
        bitDemand += 4;
        // transportEnc_UpdateBitstreamField( hAss->hAssemble,
        // streamCntPosition, hAss->varStreamCnt-1, 4 );  UINT pos =
        // streamCntPosition-FDKgetValidBits(hAss->hAssemble);  FDKpushBack(
        // hAss->hAssemble,  pos);  FDKwriteBits( hAss->hAssemble,
        // hAss->varStreamCnt-1, 4);  FDKpushFor( hAss->hAssemble, pos-4);
      } break;

      default:
        return 0;
    }
  }

  return bitDemand;
}

TRANSPORTENC_ERROR
CreateStreamMuxConfig(HANDLE_LATM_STREAM hAss, HANDLE_FDK_BITSTREAM hBs,
                      int bufferFullness, CSTpCallBacks *cb) {
  INT streamIDcnt, tmp;
  int layer, prog;

  USHORT coreFrameOffset = 0;

  hAss->taraBufferFullness = 0xFF;
  hAss->audioMuxVersionA = 0; /* for future extensions */
  hAss->streamMuxConfigBits = 0;

  FDKwriteBits(hBs, hAss->audioMuxVersion, 1); /* audioMuxVersion */
  hAss->streamMuxConfigBits += 1;

  if (hAss->audioMuxVersion == 1) {
    FDKwriteBits(hBs, hAss->audioMuxVersionA, 1); /* audioMuxVersionA */
    hAss->streamMuxConfigBits += 1;
  }

  if (hAss->audioMuxVersionA == 0) {
    if (hAss->audioMuxVersion == 1) {
      hAss->streamMuxConfigBits += transportEnc_LatmWriteValue(
          hBs, hAss->taraBufferFullness); /* taraBufferFullness */
    }
    FDKwriteBits(hBs, hAss->allStreamsSameTimeFraming ? 1 : 0,
                 1);                             /* allStreamsSameTimeFraming */
    FDKwriteBits(hBs, hAss->noSubframes - 1, 6); /* Number of Subframes */
    FDKwriteBits(hBs, hAss->noProgram - 1, 4);   /* Number of Programs */

    hAss->streamMuxConfigBits += 11;

    streamIDcnt = 0;
    for (prog = 0; prog < hAss->noProgram; prog++) {
      int transLayer = 0;

      FDKwriteBits(hBs, hAss->noLayer[prog] - 1, 3);
      hAss->streamMuxConfigBits += 3;

      for (layer = 0; layer < LATM_MAX_LAYERS; layer++) {
        LATM_LAYER_INFO *p_linfo = &(hAss->m_linfo[prog][layer]);
        CODER_CONFIG *p_lci = hAss->config[prog][layer];

        p_linfo->streamID = -1;

        if (hAss->config[prog][layer] != NULL) {
          int useSameConfig = 0;

          if (transLayer > 0) {
            FDKwriteBits(hBs, useSameConfig ? 1 : 0, 1);
            hAss->streamMuxConfigBits += 1;
          }
          if ((useSameConfig == 0) || (transLayer == 0)) {
            const UINT alignAnchor = FDKgetValidBits(hBs);

            if (0 !=
                (transportEnc_writeASC(hBs, hAss->config[prog][layer], cb))) {
              return TRANSPORTENC_UNKOWN_ERROR;
            }

            if (hAss->audioMuxVersion == 1) {
              UINT ascLen = transportEnc_LatmWriteValue(hBs, 0);
              FDKbyteAlign(hBs, alignAnchor);
              ascLen = FDKgetValidBits(hBs) - alignAnchor - ascLen;
              FDKpushBack(hBs, FDKgetValidBits(hBs) - alignAnchor);

              transportEnc_LatmWriteValue(hBs, ascLen);

              if (0 !=
                  (transportEnc_writeASC(hBs, hAss->config[prog][layer], cb))) {
                return TRANSPORTENC_UNKOWN_ERROR;
              }

              FDKbyteAlign(hBs, alignAnchor); /* asc length fillbits */
            }

            hAss->streamMuxConfigBits +=
                FDKgetValidBits(hBs) -
                alignAnchor; /* add asc length to smc summary */
          }
          transLayer++;

          if (!hAss->allStreamsSameTimeFraming) {
            if (streamIDcnt >= LATM_MAX_STREAM_ID)
              return TRANSPORTENC_INVALID_CONFIG;
          }
          p_linfo->streamID = streamIDcnt++;

          switch (p_lci->aot) {
            case AOT_AAC_MAIN:
            case AOT_AAC_LC:
            case AOT_AAC_SSR:
            case AOT_AAC_LTP:
            case AOT_AAC_SCAL:
            case AOT_ER_AAC_LD:
            case AOT_ER_AAC_ELD:
            case AOT_USAC:
              p_linfo->frameLengthType = 0;

              FDKwriteBits(hBs, p_linfo->frameLengthType,
                           3);                      /* frameLengthType */
              FDKwriteBits(hBs, bufferFullness, 8); /* bufferFullness */
              hAss->streamMuxConfigBits += 11;

              if (!hAss->allStreamsSameTimeFraming) {
                CODER_CONFIG *p_lci_prev = hAss->config[prog][layer - 1];
                if (((p_lci->aot == AOT_AAC_SCAL) ||
                     (p_lci->aot == AOT_ER_AAC_SCAL)) &&
                    ((p_lci_prev->aot == AOT_CELP) ||
                     (p_lci_prev->aot == AOT_ER_CELP))) {
                  FDKwriteBits(hBs, coreFrameOffset, 6); /* coreFrameOffset */
                  hAss->streamMuxConfigBits += 6;
                }
              }
              break;

            case AOT_TWIN_VQ:
              p_linfo->frameLengthType = 1;
              tmp = ((p_lci->bitsFrame + 7) >> 3) -
                    20; /* transmission frame length in bytes */
              if ((tmp < 0)) {
                return TRANSPORTENC_INVALID_TRANSMISSION_FRAME_LENGTH;
              }
              FDKwriteBits(hBs, p_linfo->frameLengthType,
                           3); /* frameLengthType */
              FDKwriteBits(hBs, tmp, 9);
              hAss->streamMuxConfigBits += 12;

              p_linfo->frameLengthBits = (tmp + 20) << 3;
              break;

            case AOT_CELP:
              p_linfo->frameLengthType = 4;
              FDKwriteBits(hBs, p_linfo->frameLengthType,
                           3); /* frameLengthType */
              hAss->streamMuxConfigBits += 3;
              {
                int i;
                for (i = 0; i < 62; i++) {
                  if (celpFrameLengthTable[i] == p_lci->bitsFrame) break;
                }
                if (i >= 62) {
                  return TRANSPORTENC_INVALID_CELP_FRAME_LENGTH;
                }

                FDKwriteBits(hBs, i, 6); /* CELPframeLengthTabelIndex */
                hAss->streamMuxConfigBits += 6;
              }
              p_linfo->frameLengthBits = p_lci->bitsFrame;
              break;

            case AOT_HVXC:
              p_linfo->frameLengthType = 6;
              FDKwriteBits(hBs, p_linfo->frameLengthType,
                           3); /* frameLengthType */
              hAss->streamMuxConfigBits += 3;
              {
                int i;

                if (p_lci->bitsFrame == 40) {
                  i = 0;
                } else if (p_lci->bitsFrame == 80) {
                  i = 1;
                } else {
                  return TRANSPORTENC_INVALID_FRAME_BITS;
                }
                FDKwriteBits(hBs, i, 1); /* HVXCframeLengthTableIndex */
                hAss->streamMuxConfigBits += 1;
              }
              p_linfo->frameLengthBits = p_lci->bitsFrame;
              break;

            case AOT_NULL_OBJECT:
            default:
              return TRANSPORTENC_INVALID_AOT;
          }
        }
      }
    }

    FDKwriteBits(hBs, (hAss->otherDataLenBits > 0) ? 1 : 0,
                 1); /* otherDataPresent */
    hAss->streamMuxConfigBits += 1;

    if (hAss->otherDataLenBits > 0) {
      FDKwriteBits(hBs, 0, 1);
      FDKwriteBits(hBs, hAss->otherDataLenBits, 8);
      hAss->streamMuxConfigBits += 9;
    }

    FDKwriteBits(hBs, 0, 1); /* crcCheckPresent=0 */
    hAss->streamMuxConfigBits += 1;

  } else { /* if ( audioMuxVersionA == 0 ) */

    /* for future extensions */
  }

  return TRANSPORTENC_OK;
}

static TRANSPORTENC_ERROR WriteAuPayloadLengthInfo(
    HANDLE_FDK_BITSTREAM hBitStream, int AuLengthBits) {
  int restBytes;

  if (AuLengthBits % 8) return TRANSPORTENC_INVALID_AU_LENGTH;

  while (AuLengthBits >= 255 * 8) {
    FDKwriteBits(hBitStream, 255, 8); /* 255 shows incomplete AU */
    AuLengthBits -= (255 * 8);
  }

  restBytes = (AuLengthBits) >> 3;
  FDKwriteBits(hBitStream, restBytes, 8);

  return TRANSPORTENC_OK;
}

static TRANSPORTENC_ERROR transportEnc_LatmSetNrOfSubframes(
    HANDLE_LATM_STREAM hAss, INT noSubframes_next) /* nr of access units /
                                                      payloads within a latm
                                                      frame */
{
  /* sanity chk */
  if (noSubframes_next < 1 || noSubframes_next > MAX_NR_OF_SUBFRAMES) {
    return TRANSPORTENC_LATM_INVALID_NR_OF_SUBFRAMES;
  }

  hAss->noSubframes_next = noSubframes_next;

  /* if at start then we can take over the value immediately, otherwise we have
   * to wait for the next SMC */
  if ((hAss->subFrameCnt == 0) && (hAss->latmFrameCounter == 0)) {
    hAss->noSubframes = noSubframes_next;
  }

  return TRANSPORTENC_OK;
}

static int allStreamsSameTimeFraming(HANDLE_LATM_STREAM hAss, UCHAR noProgram,
                                     UCHAR noLayer[] /* return */) {
  int prog, layer;

  signed int lastNoSamples = -1;
  signed int minFrameSamples = FDK_INT_MAX;
  signed int maxFrameSamples = 0;

  signed int highestSamplingRate = -1;

  for (prog = 0; prog < noProgram; prog++) {
    noLayer[prog] = 0;

    for (layer = 0; layer < LATM_MAX_LAYERS; layer++) {
      if (hAss->config[prog][layer] != NULL) {
        INT hsfSamplesFrame;

        noLayer[prog]++;

        if (highestSamplingRate < 0)
          highestSamplingRate = hAss->config[prog][layer]->samplingRate;

        hsfSamplesFrame = hAss->config[prog][layer]->samplesPerFrame *
                          highestSamplingRate /
                          hAss->config[prog][layer]->samplingRate;

        if (hsfSamplesFrame <= minFrameSamples)
          minFrameSamples = hsfSamplesFrame;
        if (hsfSamplesFrame >= maxFrameSamples)
          maxFrameSamples = hsfSamplesFrame;

        if (lastNoSamples == -1) {
          lastNoSamples = hsfSamplesFrame;
        } else {
          if (hsfSamplesFrame != lastNoSamples) {
            return 0;
          }
        }
      }
    }
  }

  return 1;
}

/**
 * Initialize LATM/LOAS Stream and add layer 0 at program 0.
 */
static TRANSPORTENC_ERROR transportEnc_InitLatmStream(
    HANDLE_LATM_STREAM hAss, int fractDelayPresent,
    signed int
        muxConfigPeriod, /* insert setup data every muxConfigPeriod frames */
    UINT audioMuxVersion, TRANSPORT_TYPE tt) {
  TRANSPORTENC_ERROR ErrorStatus = TRANSPORTENC_OK;

  if (hAss == NULL) return TRANSPORTENC_INVALID_PARAMETER;

  hAss->tt = tt;

  hAss->noProgram = 1;

  hAss->audioMuxVersion = audioMuxVersion;

  /* Fill noLayer array using hAss->config */
  hAss->allStreamsSameTimeFraming =
      allStreamsSameTimeFraming(hAss, hAss->noProgram, hAss->noLayer);
  /* Only allStreamsSameTimeFraming==1 is supported */
  FDK_ASSERT(hAss->allStreamsSameTimeFraming);

  hAss->fractDelayPresent = fractDelayPresent;
  hAss->otherDataLenBits = 0;

  hAss->varMode = LATMVAR_SIMPLE_SEQUENCE;

  /* initialize counters */
  hAss->subFrameCnt = 0;
  hAss->noSubframes = DEFAULT_LATM_NR_OF_SUBFRAMES;
  hAss->noSubframes_next = DEFAULT_LATM_NR_OF_SUBFRAMES;

  /* sync layer related */
  hAss->audioMuxLengthBytes = 0;

  hAss->latmFrameCounter = 0;
  hAss->muxConfigPeriod = muxConfigPeriod;

  return ErrorStatus;
}

/**
 *
 */
UINT transportEnc_LatmCountTotalBitDemandHeader(HANDLE_LATM_STREAM hAss,
                                                unsigned int streamDataLength) {
  UINT bitDemand = 0;

  switch (hAss->tt) {
    case TT_MP4_LOAS:
    case TT_MP4_LATM_MCP0:
    case TT_MP4_LATM_MCP1:
      if (hAss->subFrameCnt == 0) {
        bitDemand = transportEnc_LatmCountFixBitDemandHeader(hAss);
      }
      bitDemand += transportEnc_LatmCountVarBitDemandHeader(
          hAss, streamDataLength /*- bitDemand*/);
      break;
    default:
      break;
  }

  return bitDemand;
}

static TRANSPORTENC_ERROR AdvanceAudioMuxElement(HANDLE_LATM_STREAM hAss,
                                                 HANDLE_FDK_BITSTREAM hBs,
                                                 int auBits, int bufferFullness,
                                                 CSTpCallBacks *cb) {
  TRANSPORTENC_ERROR ErrorStatus = TRANSPORTENC_OK;
  int insertMuxSetup;

  /* Insert setup data to assemble Buffer */
  if (hAss->subFrameCnt == 0) {
    if (hAss->muxConfigPeriod > 0) {
      insertMuxSetup = (hAss->latmFrameCounter == 0);
    } else {
      insertMuxSetup = 0;
    }

    if (hAss->tt != TT_MP4_LATM_MCP0) {
      if (insertMuxSetup) {
        FDKwriteBits(hBs, 0, 1); /* useSameStreamMux useNewStreamMux */
        if (TRANSPORTENC_OK != (ErrorStatus = CreateStreamMuxConfig(
                                    hAss, hBs, bufferFullness, cb))) {
          return ErrorStatus;
        }
      } else {
        FDKwriteBits(hBs, 1, 1); /* useSameStreamMux */
      }
    }
  }

  /* PayloadLengthInfo */
  {
    int prog, layer;

    for (prog = 0; prog < hAss->noProgram; prog++) {
      for (layer = 0; layer < hAss->noLayer[prog]; layer++) {
        ErrorStatus = WriteAuPayloadLengthInfo(hBs, auBits);
        if (ErrorStatus != TRANSPORTENC_OK) return ErrorStatus;
      }
    }
  }
  /* At this point comes the access unit. */

  return TRANSPORTENC_OK;
}

TRANSPORTENC_ERROR
transportEnc_LatmWrite(HANDLE_LATM_STREAM hAss, HANDLE_FDK_BITSTREAM hBs,
                       int auBits, int bufferFullness, CSTpCallBacks *cb) {
  TRANSPORTENC_ERROR ErrorStatus;

  if (hAss->subFrameCnt == 0) {
    /* Start new frame */
    FDKresetBitbuffer(hBs, BS_WRITER);
  }

  hAss->latmSubframeStart = FDKgetValidBits(hBs);

  /* Insert syncword and syncword distance
     - only if loas
     - we must update the syncword distance (=audiomuxlengthbytes) later
   */
  if (hAss->tt == TT_MP4_LOAS && hAss->subFrameCnt == 0) {
    /* Start new LOAS frame */
    FDKwriteBits(hBs, 0x2B7, 11);
    hAss->audioMuxLengthBytes = 0;
    hAss->audioMuxLengthBytesPos =
        FDKgetValidBits(hBs); /* store read pointer position */
    FDKwriteBits(hBs, hAss->audioMuxLengthBytes, 13);
  }

  ErrorStatus = AdvanceAudioMuxElement(hAss, hBs, auBits, bufferFullness, cb);

  if (ErrorStatus != TRANSPORTENC_OK) return ErrorStatus;

  return ErrorStatus;
}

void transportEnc_LatmAdjustSubframeBits(HANDLE_LATM_STREAM hAss, int *bits) {
  /* Substract bits from possible previous subframe */
  *bits -= hAss->latmSubframeStart;
  /* Add fill bits */
  if (hAss->subFrameCnt == 0) {
    *bits += hAss->otherDataLenBits;
    *bits += hAss->fillBits;
  }
}

TRANSPORTENC_ERROR transportEnc_LatmGetFrame(HANDLE_LATM_STREAM hAss,
                                             HANDLE_FDK_BITSTREAM hBs,
                                             int *pBytes) {
  TRANSPORTENC_ERROR ErrorStatus = TRANSPORTENC_OK;

  hAss->subFrameCnt++;
  if (hAss->subFrameCnt >= hAss->noSubframes) {
    /* Add LOAS frame length if required. */
    if (hAss->tt == TT_MP4_LOAS) {
      FDK_BITSTREAM tmpBuf;

      /* Determine frame length info */
      hAss->audioMuxLengthBytes =
          ((FDKgetValidBits(hBs) + hAss->otherDataLenBits + 7) >> 3) -
          3; /* 3=Syncword + length */

      /* Check frame length info */
      if (hAss->audioMuxLengthBytes >= (1 << 13)) {
        ErrorStatus = TRANSPORTENC_INVALID_AU_LENGTH;
        goto bail;
      }

      /* Write length info into assembler buffer */
      FDKinitBitStream(&tmpBuf, hBs->hBitBuf.Buffer, hBs->hBitBuf.bufSize, 0,
                       BS_WRITER);
      FDKpushFor(&tmpBuf, hAss->audioMuxLengthBytesPos);
      FDKwriteBits(&tmpBuf, hAss->audioMuxLengthBytes, 13);
      FDKsyncCache(&tmpBuf);
    }

    /* Write AudioMuxElement other data bits */
    FDKwriteBits(hBs, 0, hAss->otherDataLenBits);

    /* Write AudioMuxElement byte alignment fill bits */
    FDKwriteBits(hBs, 0, hAss->fillBits);

    FDK_ASSERT((FDKgetValidBits(hBs) % 8) == 0);

    hAss->subFrameCnt = 0;

    FDKsyncCache(hBs);
    *pBytes = (FDKgetValidBits(hBs) + 7) >> 3;

    if (hAss->muxConfigPeriod > 0) {
      hAss->latmFrameCounter++;

      if (hAss->latmFrameCounter >= hAss->muxConfigPeriod) {
        hAss->latmFrameCounter = 0;
        hAss->noSubframes = hAss->noSubframes_next;
      }
    }
  } else {
    /* No data this time */
    *pBytes = 0;
  }

bail:
  return ErrorStatus;
}

/**
 * Init LATM/LOAS
 */
TRANSPORTENC_ERROR transportEnc_Latm_Init(HANDLE_LATM_STREAM hAss,
                                          HANDLE_FDK_BITSTREAM hBs,
                                          CODER_CONFIG *layerConfig,
                                          UINT audioMuxVersion,
                                          TRANSPORT_TYPE tt,
                                          CSTpCallBacks *cb) {
  TRANSPORTENC_ERROR ErrorStatus;
  int fractDelayPresent = 0;
  int prog, layer;

  int setupDataDistanceFrames = layerConfig->headerPeriod;

  FDK_ASSERT(setupDataDistanceFrames >= 0);

  for (prog = 0; prog < LATM_MAX_PROGRAMS; prog++) {
    for (layer = 0; layer < LATM_MAX_LAYERS; layer++) {
      hAss->config[prog][layer] = NULL;
      hAss->m_linfo[prog][layer].streamID = -1;
    }
  }

  hAss->config[0][0] = layerConfig;
  hAss->m_linfo[0][0].streamID = 0;

  ErrorStatus = transportEnc_InitLatmStream(hAss, fractDelayPresent,
                                            setupDataDistanceFrames,
                                            (audioMuxVersion) ? 1 : 0, tt);
  if (ErrorStatus != TRANSPORTENC_OK) goto bail;

  ErrorStatus =
      transportEnc_LatmSetNrOfSubframes(hAss, layerConfig->nSubFrames);
  if (ErrorStatus != TRANSPORTENC_OK) goto bail;

  /* Get the size of the StreamMuxConfig somehow */
  if (TRANSPORTENC_OK !=
      (ErrorStatus = AdvanceAudioMuxElement(hAss, hBs, 0, 0, cb))) {
    goto bail;
  }

  // CreateStreamMuxConfig(hAss, hBs, 0);

bail:
  return ErrorStatus;
}

TRANSPORTENC_ERROR transportEnc_LatmAddOtherDataBits(HANDLE_LATM_STREAM hAss,
                                                     const int otherDataBits) {
  TRANSPORTENC_ERROR ErrorStatus = TRANSPORTENC_OK;

  if ((hAss->otherDataLenBits != 0) || (otherDataBits % 8 != 0)) {
    /* This implementation allows to add other data bits only once.
       To keep existing alignment only whole bytes are allowed. */
    ErrorStatus = TRANSPORTENC_UNKOWN_ERROR;
  } else {
    /* Ensure correct addional bits in payload. */
    if (hAss->tt == TT_MP4_LATM_MCP0) {
      hAss->otherDataLenBits = otherDataBits;
    } else {
      hAss->otherDataLenBits = otherDataBits - 9;
      hAss->streamMuxConfigBits += 9;
    }
  }

  return ErrorStatus;
}
