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

/**************************** AAC encoder library ******************************

   Author(s):   M. Werner

   Description: Bitstream encoder

*******************************************************************************/

#include "bitenc.h"
#include "bit_cnt.h"
#include "dyn_bits.h"
#include "qc_data.h"
#include "interface.h"
#include "aacEnc_ram.h"

#include "tpenc_lib.h"

#include "FDK_tools_rom.h" /* needed for the bitstream syntax tables */

static const int globalGainOffset = 100;
static const int icsReservedBit = 0;
static const int noiseOffset = 90;

/*****************************************************************************

    functionname: FDKaacEnc_encodeSpectralData
    description:  encode spectral data
    returns:      the number of written bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeSpectralData(INT *sfbOffset,
                                        SECTION_DATA *sectionData,
                                        SHORT *quantSpectrum,
                                        HANDLE_FDK_BITSTREAM hBitStream) {
  INT i, sfb;
  INT dbgVal = FDKgetValidBits(hBitStream);

  for (i = 0; i < sectionData->noOfSections; i++) {
    if (sectionData->huffsection[i].codeBook != CODE_BOOK_PNS_NO) {
      /* huffencode spectral data for this huffsection */
      INT tmp = sectionData->huffsection[i].sfbStart +
                sectionData->huffsection[i].sfbCnt;
      for (sfb = sectionData->huffsection[i].sfbStart; sfb < tmp; sfb++) {
        FDKaacEnc_codeValues(quantSpectrum + sfbOffset[sfb],
                             sfbOffset[sfb + 1] - sfbOffset[sfb],
                             sectionData->huffsection[i].codeBook, hBitStream);
      }
    }
  }
  return (FDKgetValidBits(hBitStream) - dbgVal);
}

/*****************************************************************************

    functionname:FDKaacEnc_encodeGlobalGain
    description: encodes Global Gain (common scale factor)
    returns:     the number of static bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeGlobalGain(INT globalGain, INT scalefac,
                                      HANDLE_FDK_BITSTREAM hBitStream,
                                      INT mdctScale) {
  if (hBitStream != NULL) {
    FDKwriteBits(hBitStream,
                 globalGain - scalefac + globalGainOffset -
                     4 * (LOG_NORM_PCM - mdctScale),
                 8);
  }
  return (8);
}

/*****************************************************************************

    functionname:FDKaacEnc_encodeIcsInfo
    description: encodes Ics Info
    returns:     the number of static bits
    input:
    output:

*****************************************************************************/

static INT FDKaacEnc_encodeIcsInfo(INT blockType, INT windowShape,
                                   INT groupingMask, INT maxSfbPerGroup,
                                   HANDLE_FDK_BITSTREAM hBitStream,
                                   UINT syntaxFlags) {
  INT statBits;

  if (blockType == SHORT_WINDOW) {
    statBits = 8 + TRANS_FAC - 1;
  } else {
    if (syntaxFlags & AC_ELD) {
      statBits = 6;
    } else {
      statBits = (!(syntaxFlags & AC_SCALABLE)) ? 11 : 10;
    }
  }

  if (hBitStream != NULL) {
    if (!(syntaxFlags & AC_ELD)) {
      FDKwriteBits(hBitStream, icsReservedBit, 1);
      FDKwriteBits(hBitStream, blockType, 2);
      FDKwriteBits(hBitStream,
                   (windowShape == LOL_WINDOW) ? KBD_WINDOW : windowShape, 1);
    }

    switch (blockType) {
      case LONG_WINDOW:
      case START_WINDOW:
      case STOP_WINDOW:
        FDKwriteBits(hBitStream, maxSfbPerGroup, 6);

        if (!(syntaxFlags &
              (AC_SCALABLE | AC_ELD))) { /* If not scalable syntax then ... */
          /* No predictor data present */
          FDKwriteBits(hBitStream, 0, 1);
        }
        break;

      case SHORT_WINDOW:
        FDKwriteBits(hBitStream, maxSfbPerGroup, 4);

        /* Write grouping bits */
        FDKwriteBits(hBitStream, groupingMask, TRANS_FAC - 1);
        break;
    }
  }

  return (statBits);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodeSectionData
    description:  encode section data (common Huffman codebooks for adjacent
                  SFB's)
    returns:      none
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeSectionData(SECTION_DATA *sectionData,
                                       HANDLE_FDK_BITSTREAM hBitStream,
                                       UINT useVCB11) {
  if (hBitStream != NULL) {
    INT sectEscapeVal = 0, sectLenBits = 0;
    INT sectLen;
    INT i;
    INT dbgVal = FDKgetValidBits(hBitStream);
    INT sectCbBits = 4;

    switch (sectionData->blockType) {
      case LONG_WINDOW:
      case START_WINDOW:
      case STOP_WINDOW:
        sectEscapeVal = SECT_ESC_VAL_LONG;
        sectLenBits = SECT_BITS_LONG;
        break;

      case SHORT_WINDOW:
        sectEscapeVal = SECT_ESC_VAL_SHORT;
        sectLenBits = SECT_BITS_SHORT;
        break;
    }

    for (i = 0; i < sectionData->noOfSections; i++) {
      INT codeBook = sectionData->huffsection[i].codeBook;

      FDKwriteBits(hBitStream, codeBook, sectCbBits);

      {
        sectLen = sectionData->huffsection[i].sfbCnt;

        while (sectLen >= sectEscapeVal) {
          FDKwriteBits(hBitStream, sectEscapeVal, sectLenBits);
          sectLen -= sectEscapeVal;
        }
        FDKwriteBits(hBitStream, sectLen, sectLenBits);
      }
    }
    return (FDKgetValidBits(hBitStream) - dbgVal);
  }
  return (0);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodeScaleFactorData
    description:  encode DPCM coded scale factors
    returns:      none
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeScaleFactorData(UINT *maxValueInSfb,
                                           SECTION_DATA *sectionData,
                                           INT *scalefac,
                                           HANDLE_FDK_BITSTREAM hBitStream,
                                           INT *RESTRICT noiseNrg,
                                           const INT *isScale, INT globalGain) {
  if (hBitStream != NULL) {
    INT i, j, lastValScf, deltaScf;
    INT deltaPns;
    INT lastValPns = 0;
    INT noisePCMFlag = TRUE;
    INT lastValIs;

    INT dbgVal = FDKgetValidBits(hBitStream);

    lastValScf = scalefac[sectionData->firstScf];
    lastValPns = globalGain - scalefac[sectionData->firstScf] +
                 globalGainOffset - 4 * LOG_NORM_PCM - noiseOffset;
    lastValIs = 0;

    for (i = 0; i < sectionData->noOfSections; i++) {
      if (sectionData->huffsection[i].codeBook != CODE_BOOK_ZERO_NO) {
        if ((sectionData->huffsection[i].codeBook ==
             CODE_BOOK_IS_OUT_OF_PHASE_NO) ||
            (sectionData->huffsection[i].codeBook ==
             CODE_BOOK_IS_IN_PHASE_NO)) {
          INT sfbStart = sectionData->huffsection[i].sfbStart;
          INT tmp = sfbStart + sectionData->huffsection[i].sfbCnt;
          for (j = sfbStart; j < tmp; j++) {
            INT deltaIs = isScale[j] - lastValIs;
            lastValIs = isScale[j];
            if (FDKaacEnc_codeScalefactorDelta(deltaIs, hBitStream)) {
              return (1);
            }
          } /* sfb */
        } else if (sectionData->huffsection[i].codeBook == CODE_BOOK_PNS_NO) {
          INT sfbStart = sectionData->huffsection[i].sfbStart;
          INT tmp = sfbStart + sectionData->huffsection[i].sfbCnt;
          for (j = sfbStart; j < tmp; j++) {
            deltaPns = noiseNrg[j] - lastValPns;
            lastValPns = noiseNrg[j];

            if (noisePCMFlag) {
              FDKwriteBits(hBitStream, deltaPns + (1 << (PNS_PCM_BITS - 1)),
                           PNS_PCM_BITS);
              noisePCMFlag = FALSE;
            } else {
              if (FDKaacEnc_codeScalefactorDelta(deltaPns, hBitStream)) {
                return (1);
              }
            }
          } /* sfb */
        } else {
          INT tmp = sectionData->huffsection[i].sfbStart +
                    sectionData->huffsection[i].sfbCnt;
          for (j = sectionData->huffsection[i].sfbStart; j < tmp; j++) {
            /*
              check if we can repeat the last value to save bits
            */
            if (maxValueInSfb[j] == 0)
              deltaScf = 0;
            else {
              deltaScf = -(scalefac[j] - lastValScf);
              lastValScf = scalefac[j];
            }
            if (FDKaacEnc_codeScalefactorDelta(deltaScf, hBitStream)) {
              return (1);
            }
          } /* sfb */
        }   /* code scalefactor */
      }     /* sectionData->huffsection[i].codeBook != CODE_BOOK_ZERO_NO */
    }       /* section loop */

    return (FDKgetValidBits(hBitStream) - dbgVal);
  } /* if (hBitStream != NULL) */

  return (0);
}

/*****************************************************************************

    functionname:encodeMsInfo
    description: encodes MS-Stereo Info
    returns:     the number of static bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeMSInfo(INT sfbCnt, INT grpSfb, INT maxSfb,
                                  INT msDigest, INT *jsFlags,
                                  HANDLE_FDK_BITSTREAM hBitStream) {
  INT sfb, sfbOff, msBits = 0;

  if (hBitStream != NULL) {
    switch (msDigest) {
      case MS_NONE:
        FDKwriteBits(hBitStream, SI_MS_MASK_NONE, 2);
        msBits += 2;
        break;

      case MS_ALL:
        FDKwriteBits(hBitStream, SI_MS_MASK_ALL, 2);
        msBits += 2;
        break;

      case MS_SOME:
        FDKwriteBits(hBitStream, SI_MS_MASK_SOME, 2);
        msBits += 2;
        for (sfbOff = 0; sfbOff < sfbCnt; sfbOff += grpSfb) {
          for (sfb = 0; sfb < maxSfb; sfb++) {
            if (jsFlags[sfbOff + sfb] & MS_ON) {
              FDKwriteBits(hBitStream, 1, 1);
            } else {
              FDKwriteBits(hBitStream, 0, 1);
            }
            msBits += 1;
          }
        }
        break;
    }
  } else {
    msBits += 2;
    if (msDigest == MS_SOME) {
      for (sfbOff = 0; sfbOff < sfbCnt; sfbOff += grpSfb) {
        for (sfb = 0; sfb < maxSfb; sfb++) {
          msBits += 1;
        }
      }
    }
  }
  return (msBits);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodeTnsDataPresent
    description:  encode TNS data (filter order, coeffs, ..)
    returns:      the number of static bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeTnsDataPresent(TNS_INFO *tnsInfo, INT blockType,
                                          HANDLE_FDK_BITSTREAM hBitStream) {
  if ((hBitStream != NULL) && (tnsInfo != NULL)) {
    INT i, tnsPresent = 0;
    INT numOfWindows = (blockType == SHORT_WINDOW ? TRANS_FAC : 1);

    for (i = 0; i < numOfWindows; i++) {
      if (tnsInfo->numOfFilters[i] != 0) {
        tnsPresent = 1;
        break;
      }
    }

    if (tnsPresent == 0) {
      FDKwriteBits(hBitStream, 0, 1);
    } else {
      FDKwriteBits(hBitStream, 1, 1);
    }
  }
  return (1);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodeTnsData
    description:  encode TNS data (filter order, coeffs, ..)
    returns:      the number of static bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeTnsData(TNS_INFO *tnsInfo, INT blockType,
                                   HANDLE_FDK_BITSTREAM hBitStream) {
  INT tnsBits = 0;

  if (tnsInfo != NULL) {
    INT i, j, k;
    INT tnsPresent = 0;
    INT coefBits;
    INT numOfWindows = (blockType == SHORT_WINDOW ? TRANS_FAC : 1);

    for (i = 0; i < numOfWindows; i++) {
      if (tnsInfo->numOfFilters[i] != 0) {
        tnsPresent = 1;
      }
    }

    if (hBitStream != NULL) {
      if (tnsPresent == 1) { /* there is data to be written*/
        for (i = 0; i < numOfWindows; i++) {
          FDKwriteBits(hBitStream, tnsInfo->numOfFilters[i],
                       (blockType == SHORT_WINDOW ? 1 : 2));
          tnsBits += (blockType == SHORT_WINDOW ? 1 : 2);
          if (tnsInfo->numOfFilters[i]) {
            FDKwriteBits(hBitStream, (tnsInfo->coefRes[i] == 4 ? 1 : 0), 1);
            tnsBits += 1;
          }
          for (j = 0; j < tnsInfo->numOfFilters[i]; j++) {
            FDKwriteBits(hBitStream, tnsInfo->length[i][j],
                         (blockType == SHORT_WINDOW ? 4 : 6));
            tnsBits += (blockType == SHORT_WINDOW ? 4 : 6);
            FDK_ASSERT(tnsInfo->order[i][j] <= 12);
            FDKwriteBits(hBitStream, tnsInfo->order[i][j],
                         (blockType == SHORT_WINDOW ? 3 : 5));
            tnsBits += (blockType == SHORT_WINDOW ? 3 : 5);
            if (tnsInfo->order[i][j]) {
              FDKwriteBits(hBitStream, tnsInfo->direction[i][j], 1);
              tnsBits += 1; /*direction*/
              if (tnsInfo->coefRes[i] == 4) {
                coefBits = 3;
                for (k = 0; k < tnsInfo->order[i][j]; k++) {
                  if (tnsInfo->coef[i][j][k] > 3 ||
                      tnsInfo->coef[i][j][k] < -4) {
                    coefBits = 4;
                    break;
                  }
                }
              } else {
                coefBits = 2;
                for (k = 0; k < tnsInfo->order[i][j]; k++) {
                  if (tnsInfo->coef[i][j][k] > 1 ||
                      tnsInfo->coef[i][j][k] < -2) {
                    coefBits = 3;
                    break;
                  }
                }
              }
              FDKwriteBits(hBitStream, -(coefBits - tnsInfo->coefRes[i]),
                           1); /*coef_compres*/
              tnsBits += 1;    /*coef_compression */
              for (k = 0; k < tnsInfo->order[i][j]; k++) {
                static const INT rmask[] = {0, 1, 3, 7, 15};
                FDKwriteBits(hBitStream,
                             tnsInfo->coef[i][j][k] & rmask[coefBits],
                             coefBits);
                tnsBits += coefBits;
              }
            }
          }
        }
      }
    } else {
      if (tnsPresent != 0) {
        for (i = 0; i < numOfWindows; i++) {
          tnsBits += (blockType == SHORT_WINDOW ? 1 : 2);
          if (tnsInfo->numOfFilters[i]) {
            tnsBits += 1;
            for (j = 0; j < tnsInfo->numOfFilters[i]; j++) {
              tnsBits += (blockType == SHORT_WINDOW ? 4 : 6);
              tnsBits += (blockType == SHORT_WINDOW ? 3 : 5);
              if (tnsInfo->order[i][j]) {
                tnsBits += 1; /*direction*/
                tnsBits += 1; /*coef_compression */
                if (tnsInfo->coefRes[i] == 4) {
                  coefBits = 3;
                  for (k = 0; k < tnsInfo->order[i][j]; k++) {
                    if (tnsInfo->coef[i][j][k] > 3 ||
                        tnsInfo->coef[i][j][k] < -4) {
                      coefBits = 4;
                      break;
                    }
                  }
                } else {
                  coefBits = 2;
                  for (k = 0; k < tnsInfo->order[i][j]; k++) {
                    if (tnsInfo->coef[i][j][k] > 1 ||
                        tnsInfo->coef[i][j][k] < -2) {
                      coefBits = 3;
                      break;
                    }
                  }
                }
                for (k = 0; k < tnsInfo->order[i][j]; k++) {
                  tnsBits += coefBits;
                }
              }
            }
          }
        }
      }
    }
  } /* (tnsInfo!=NULL) */

  return (tnsBits);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodeGainControlData
    description:  unsupported
    returns:      none
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodeGainControlData(HANDLE_FDK_BITSTREAM hBitStream) {
  if (hBitStream != NULL) {
    FDKwriteBits(hBitStream, 0, 1);
  }
  return (1);
}

/*****************************************************************************

    functionname: FDKaacEnc_encodePulseData
    description:  not supported yet (dummy)
    returns:      none
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_encodePulseData(HANDLE_FDK_BITSTREAM hBitStream) {
  if (hBitStream != NULL) {
    FDKwriteBits(hBitStream, 0, 1);
  }
  return (1);
}

/*****************************************************************************

    functionname: FDKaacEnc_writeExtensionPayload
    description:  write extension payload to bitstream
    returns:      number of written bits
    input:
    output:

*****************************************************************************/
static INT FDKaacEnc_writeExtensionPayload(HANDLE_FDK_BITSTREAM hBitStream,
                                           EXT_PAYLOAD_TYPE extPayloadType,
                                           const UCHAR *extPayloadData,
                                           INT extPayloadBits) {
#define EXT_TYPE_BITS (4)
#define DATA_EL_VERSION_BITS (4)
#define FILL_NIBBLE_BITS (4)

  INT extBitsUsed = 0;

  if (extPayloadBits >= EXT_TYPE_BITS) {
    UCHAR fillByte = 0x00; /* for EXT_FIL and EXT_FILL_DATA */

    if (hBitStream != NULL) {
      FDKwriteBits(hBitStream, extPayloadType, EXT_TYPE_BITS);
    }
    extBitsUsed += EXT_TYPE_BITS;

    switch (extPayloadType) {
      /* case EXT_SAC_DATA: */
      case EXT_LDSAC_DATA:
        if (hBitStream != NULL) {
          FDKwriteBits(hBitStream, *extPayloadData++, 4); /* nibble */
        }
        extBitsUsed += 4;
        FDK_FALLTHROUGH;
      case EXT_DYNAMIC_RANGE:
      case EXT_SBR_DATA:
      case EXT_SBR_DATA_CRC:
        if (hBitStream != NULL) {
          int i, writeBits = extPayloadBits;
          for (i = 0; writeBits >= 8; i++) {
            FDKwriteBits(hBitStream, *extPayloadData++, 8);
            writeBits -= 8;
          }
          if (writeBits > 0) {
            FDKwriteBits(hBitStream, (*extPayloadData) >> (8 - writeBits),
                         writeBits);
          }
        }
        extBitsUsed += extPayloadBits;
        break;

      case EXT_DATA_ELEMENT: {
        INT dataElementLength = (extPayloadBits + 7) >> 3;
        INT cnt = dataElementLength;
        int loopCounter = 1;

        while (dataElementLength >= 255) {
          loopCounter++;
          dataElementLength -= 255;
        }

        if (hBitStream != NULL) {
          int i;
          FDKwriteBits(
              hBitStream, 0x00,
              DATA_EL_VERSION_BITS); /* data_element_version = ANC_DATA */

          for (i = 1; i < loopCounter; i++) {
            FDKwriteBits(hBitStream, 255, 8);
          }
          FDKwriteBits(hBitStream, dataElementLength, 8);

          for (i = 0; i < cnt; i++) {
            FDKwriteBits(hBitStream, extPayloadData[i], 8);
          }
        }
        extBitsUsed += DATA_EL_VERSION_BITS + (loopCounter * 8) + (cnt * 8);
      } break;

      case EXT_FILL_DATA:
        fillByte = 0xA5;
        FDK_FALLTHROUGH;
      case EXT_FIL:
      default:
        if (hBitStream != NULL) {
          int writeBits = extPayloadBits;
          FDKwriteBits(hBitStream, 0x00, FILL_NIBBLE_BITS);
          writeBits -=
              8; /* acount for the extension type and the fill nibble */
          while (writeBits >= 8) {
            FDKwriteBits(hBitStream, fillByte, 8);
            writeBits -= 8;
          }
        }
        extBitsUsed += FILL_NIBBLE_BITS + (extPayloadBits & ~0x7) - 8;
        break;
    }
  }

  return (extBitsUsed);
}

/*****************************************************************************

    functionname: FDKaacEnc_writeDataStreamElement
    description:  write data stream elements like ancillary data ...
    returns:      the amount of used bits
    input:
    output:

******************************************************************************/
static INT FDKaacEnc_writeDataStreamElement(HANDLE_TRANSPORTENC hTpEnc,
                                            INT elementInstanceTag,
                                            INT dataPayloadBytes,
                                            UCHAR *dataBuffer,
                                            UINT alignAnchor) {
#define DATA_BYTE_ALIGN_FLAG (0)

#define EL_INSTANCE_TAG_BITS (4)
#define DATA_BYTE_ALIGN_FLAG_BITS (1)
#define DATA_LEN_COUNT_BITS (8)
#define DATA_LEN_ESC_COUNT_BITS (8)

#define MAX_DATA_ALIGN_BITS (7)
#define MAX_DSE_DATA_BYTES (510)

  INT dseBitsUsed = 0;

  while (dataPayloadBytes > 0) {
    int esc_count = -1;
    int cnt = 0;
    INT crcReg = -1;

    dseBitsUsed += EL_ID_BITS + EL_INSTANCE_TAG_BITS +
                   DATA_BYTE_ALIGN_FLAG_BITS + DATA_LEN_COUNT_BITS;

    if (DATA_BYTE_ALIGN_FLAG) {
      dseBitsUsed += MAX_DATA_ALIGN_BITS;
    }

    cnt = fixMin(MAX_DSE_DATA_BYTES, dataPayloadBytes);
    if (cnt >= 255) {
      esc_count = cnt - 255;
      dseBitsUsed += DATA_LEN_ESC_COUNT_BITS;
    }

    dataPayloadBytes -= cnt;
    dseBitsUsed += cnt * 8;

    if (hTpEnc != NULL) {
      HANDLE_FDK_BITSTREAM hBitStream = transportEnc_GetBitstream(hTpEnc);
      int i;

      FDKwriteBits(hBitStream, ID_DSE, EL_ID_BITS);

      crcReg = transportEnc_CrcStartReg(hTpEnc, 0);

      FDKwriteBits(hBitStream, elementInstanceTag, EL_INSTANCE_TAG_BITS);
      FDKwriteBits(hBitStream, DATA_BYTE_ALIGN_FLAG, DATA_BYTE_ALIGN_FLAG_BITS);

      /* write length field(s) */
      if (esc_count >= 0) {
        FDKwriteBits(hBitStream, 255, DATA_LEN_COUNT_BITS);
        FDKwriteBits(hBitStream, esc_count, DATA_LEN_ESC_COUNT_BITS);
      } else {
        FDKwriteBits(hBitStream, cnt, DATA_LEN_COUNT_BITS);
      }

      if (DATA_BYTE_ALIGN_FLAG) {
        INT tmp = (INT)FDKgetValidBits(hBitStream);
        FDKbyteAlign(hBitStream, alignAnchor);
        /* count actual bits */
        dseBitsUsed +=
            (INT)FDKgetValidBits(hBitStream) - tmp - MAX_DATA_ALIGN_BITS;
      }

      /* write payload */
      for (i = 0; i < cnt; i++) {
        FDKwriteBits(hBitStream, dataBuffer[i], 8);
      }
      transportEnc_CrcEndReg(hTpEnc, crcReg);
    }
  }

  return (dseBitsUsed);
}

/*****************************************************************************

    functionname: FDKaacEnc_writeExtensionData
    description:  write extension payload to bitstream
    returns:      number of written bits
    input:
    output:

*****************************************************************************/
INT FDKaacEnc_writeExtensionData(HANDLE_TRANSPORTENC hTpEnc,
                                 QC_OUT_EXTENSION *pExtension,
                                 INT elInstanceTag, /* for DSE only */
                                 UINT alignAnchor,  /* for DSE only */
                                 UINT syntaxFlags, AUDIO_OBJECT_TYPE aot,
                                 SCHAR epConfig) {
#define FILL_EL_COUNT_BITS (4)
#define FILL_EL_ESC_COUNT_BITS (8)
#define MAX_FILL_DATA_BYTES (269)

  HANDLE_FDK_BITSTREAM hBitStream = NULL;
  INT payloadBits = pExtension->nPayloadBits;
  INT extBitsUsed = 0;

  if (hTpEnc != NULL) {
    hBitStream = transportEnc_GetBitstream(hTpEnc);
  }

  if (syntaxFlags & (AC_SCALABLE | AC_ER)) {
    {
      if ((syntaxFlags & AC_ELD) && ((pExtension->type == EXT_SBR_DATA) ||
                                     (pExtension->type == EXT_SBR_DATA_CRC))) {
        if (hBitStream != NULL) {
          int i, writeBits = payloadBits;
          UCHAR *extPayloadData = pExtension->pPayload;

          for (i = 0; writeBits >= 8; i++) {
            FDKwriteBits(hBitStream, extPayloadData[i], 8);
            writeBits -= 8;
          }
          if (writeBits > 0) {
            FDKwriteBits(hBitStream, extPayloadData[i] >> (8 - writeBits),
                         writeBits);
          }
        }
        extBitsUsed += payloadBits;
      } else {
        /* ER or scalable syntax -> write extension en bloc */
        extBitsUsed += FDKaacEnc_writeExtensionPayload(
            hBitStream, pExtension->type, pExtension->pPayload, payloadBits);
      }
    }
  } else {
    /* We have normal GA bitstream payload (AOT 2,5,29) so pack
       the data into a fill elements or DSEs */

    if (pExtension->type == EXT_DATA_ELEMENT) {
      extBitsUsed += FDKaacEnc_writeDataStreamElement(
          hTpEnc, elInstanceTag, pExtension->nPayloadBits >> 3,
          pExtension->pPayload, alignAnchor);
    } else {
      while (payloadBits >= (EL_ID_BITS + FILL_EL_COUNT_BITS)) {
        INT cnt, esc_count = -1, alignBits = 7;

        if ((pExtension->type == EXT_FILL_DATA) ||
            (pExtension->type == EXT_FIL)) {
          payloadBits -= EL_ID_BITS + FILL_EL_COUNT_BITS;
          if (payloadBits >= 15 * 8) {
            payloadBits -= FILL_EL_ESC_COUNT_BITS;
            esc_count = 0; /* write esc_count even if cnt becomes smaller 15 */
          }
          alignBits = 0;
        }

        cnt = fixMin(MAX_FILL_DATA_BYTES, (payloadBits + alignBits) >> 3);

        if (cnt >= 15) {
          esc_count = cnt - 15 + 1;
        }

        if (hBitStream != NULL) {
          /* write bitstream */
          FDKwriteBits(hBitStream, ID_FIL, EL_ID_BITS);
          if (esc_count >= 0) {
            FDKwriteBits(hBitStream, 15, FILL_EL_COUNT_BITS);
            FDKwriteBits(hBitStream, esc_count, FILL_EL_ESC_COUNT_BITS);
          } else {
            FDKwriteBits(hBitStream, cnt, FILL_EL_COUNT_BITS);
          }
        }

        extBitsUsed += EL_ID_BITS + FILL_EL_COUNT_BITS +
                       ((esc_count >= 0) ? FILL_EL_ESC_COUNT_BITS : 0);

        cnt = fixMin(cnt * 8, payloadBits); /* convert back to bits */
        extBitsUsed += FDKaacEnc_writeExtensionPayload(
            hBitStream, pExtension->type, pExtension->pPayload, cnt);
        payloadBits -= cnt;
      }
    }
  }

  return (extBitsUsed);
}

/*****************************************************************************

    functionname: FDKaacEnc_ByteAlignment
    description:
    returns:
    input:
    output:

*****************************************************************************/
static void FDKaacEnc_ByteAlignment(HANDLE_FDK_BITSTREAM hBitStream,
                                    int alignBits) {
  FDKwriteBits(hBitStream, 0, alignBits);
}

AAC_ENCODER_ERROR FDKaacEnc_ChannelElementWrite(
    HANDLE_TRANSPORTENC hTpEnc, ELEMENT_INFO *pElInfo,
    QC_OUT_CHANNEL *qcOutChannel[(2)], PSY_OUT_ELEMENT *psyOutElement,
    PSY_OUT_CHANNEL *psyOutChannel[(2)], UINT syntaxFlags,
    AUDIO_OBJECT_TYPE aot, SCHAR epConfig, INT *pBitDemand, UCHAR minCnt) {
  AAC_ENCODER_ERROR error = AAC_ENC_OK;
  HANDLE_FDK_BITSTREAM hBitStream = NULL;
  INT bitDemand = 0;
  const element_list_t *list;
  int i, ch, decision_bit;
  INT crcReg1 = -1, crcReg2 = -1;
  UCHAR numberOfChannels;

  if (hTpEnc != NULL) {
    /* Get bitstream handle */
    hBitStream = transportEnc_GetBitstream(hTpEnc);
  }

  if ((pElInfo->elType == ID_SCE) || (pElInfo->elType == ID_LFE)) {
    numberOfChannels = 1;
  } else {
    numberOfChannels = 2;
  }

  /* Get channel element sequence table */
  list = getBitstreamElementList(aot, epConfig, numberOfChannels, 0, 0);
  if (list == NULL) {
    error = AAC_ENC_UNSUPPORTED_AOT;
    goto bail;
  }

  if (!(syntaxFlags & (AC_SCALABLE | AC_ER))) {
    if (hBitStream != NULL) {
      FDKwriteBits(hBitStream, pElInfo->elType, EL_ID_BITS);
    }
    bitDemand += EL_ID_BITS;
  }

  /* Iterate through sequence table */
  i = 0;
  ch = 0;
  decision_bit = 0;
  do {
    /* some tmp values */
    SECTION_DATA *pChSectionData = NULL;
    INT *pChScf = NULL;
    UINT *pChMaxValueInSfb = NULL;
    TNS_INFO *pTnsInfo = NULL;
    INT chGlobalGain = 0;
    INT chBlockType = 0;
    INT chMaxSfbPerGrp = 0;
    INT chSfbPerGrp = 0;
    INT chSfbCnt = 0;
    INT chFirstScf = 0;

    if (minCnt == 0) {
      if (qcOutChannel != NULL) {
        pChSectionData = &(qcOutChannel[ch]->sectionData);
        pChScf = qcOutChannel[ch]->scf;
        chGlobalGain = qcOutChannel[ch]->globalGain;
        pChMaxValueInSfb = qcOutChannel[ch]->maxValueInSfb;
        chBlockType = pChSectionData->blockType;
        chMaxSfbPerGrp = pChSectionData->maxSfbPerGroup;
        chSfbPerGrp = pChSectionData->sfbPerGroup;
        chSfbCnt = pChSectionData->sfbCnt;
        chFirstScf = pChScf[pChSectionData->firstScf];
      } else {
        /* get values from PSY */
        chSfbCnt = psyOutChannel[ch]->sfbCnt;
        chSfbPerGrp = psyOutChannel[ch]->sfbPerGroup;
        chMaxSfbPerGrp = psyOutChannel[ch]->maxSfbPerGroup;
      }
      pTnsInfo = &psyOutChannel[ch]->tnsInfo;
    } /* minCnt==0 */

    if (qcOutChannel == NULL) {
      chBlockType = psyOutChannel[ch]->lastWindowSequence;
    }

    switch (list->id[i]) {
      case element_instance_tag:
        /* Write element instance tag */
        if (hBitStream != NULL) {
          FDKwriteBits(hBitStream, pElInfo->instanceTag, 4);
        }
        bitDemand += 4;
        break;

      case common_window:
        /* Write common window flag */
        decision_bit = psyOutElement->commonWindow;
        if (hBitStream != NULL) {
          FDKwriteBits(hBitStream, psyOutElement->commonWindow, 1);
        }
        bitDemand += 1;
        break;

      case ics_info:
        /* Write individual channel info */
        bitDemand +=
            FDKaacEnc_encodeIcsInfo(chBlockType, psyOutChannel[ch]->windowShape,
                                    psyOutChannel[ch]->groupingMask,
                                    chMaxSfbPerGrp, hBitStream, syntaxFlags);
        break;

      case ltp_data_present:
        /* Write LTP data present flag */
        if (hBitStream != NULL) {
          FDKwriteBits(hBitStream, 0, 1);
        }
        bitDemand += 1;
        break;

      case ltp_data:
        /* Predictor data not supported.
           Nothing to do here. */
        break;

      case ms:
        /* Write MS info */
        bitDemand += FDKaacEnc_encodeMSInfo(
            chSfbCnt, chSfbPerGrp, chMaxSfbPerGrp,
            (minCnt == 0) ? psyOutElement->toolsInfo.msDigest : MS_NONE,
            psyOutElement->toolsInfo.msMask, hBitStream);
        break;

      case global_gain:
        bitDemand += FDKaacEnc_encodeGlobalGain(
            chGlobalGain, chFirstScf, hBitStream, psyOutChannel[ch]->mdctScale);
        break;

      case section_data: {
        INT siBits = FDKaacEnc_encodeSectionData(
            pChSectionData, hBitStream, (syntaxFlags & AC_ER_VCB11) ? 1 : 0);
        if (hBitStream != NULL) {
          if (siBits != qcOutChannel[ch]->sectionData.sideInfoBits) {
            error = AAC_ENC_WRITE_SEC_ERROR;
          }
        }
        bitDemand += siBits;
      } break;

      case scale_factor_data: {
        INT sfDataBits = FDKaacEnc_encodeScaleFactorData(
            pChMaxValueInSfb, pChSectionData, pChScf, hBitStream,
            psyOutChannel[ch]->noiseNrg, psyOutChannel[ch]->isScale,
            chGlobalGain);
        if ((hBitStream != NULL) &&
            (sfDataBits != (qcOutChannel[ch]->sectionData.scalefacBits +
                            qcOutChannel[ch]->sectionData.noiseNrgBits))) {
          error = AAC_ENC_WRITE_SCAL_ERROR;
        }
        bitDemand += sfDataBits;
      } break;

      case esc2_rvlc:
        if (syntaxFlags & AC_ER_RVLC) {
          /* write RVLC data into bitstream (error sens. cat. 2) */
          error = AAC_ENC_UNSUPPORTED_AOT;
        }
        break;

      case pulse:
        /* Write pulse data */
        bitDemand += FDKaacEnc_encodePulseData(hBitStream);
        break;

      case tns_data_present:
        /* Write TNS data present flag */
        bitDemand +=
            FDKaacEnc_encodeTnsDataPresent(pTnsInfo, chBlockType, hBitStream);
        break;
      case tns_data:
        /* Write TNS data */
        bitDemand += FDKaacEnc_encodeTnsData(pTnsInfo, chBlockType, hBitStream);
        break;

      case gain_control_data:
        /* Nothing to do here */
        break;

      case gain_control_data_present:
        bitDemand += FDKaacEnc_encodeGainControlData(hBitStream);
        break;

      case esc1_hcr:
        if (syntaxFlags & AC_ER_HCR) {
          error = AAC_ENC_UNKNOWN;
        }
        break;

      case spectral_data:
        if (hBitStream != NULL) {
          INT spectralBits = 0;

          spectralBits = FDKaacEnc_encodeSpectralData(
              psyOutChannel[ch]->sfbOffsets, pChSectionData,
              qcOutChannel[ch]->quantSpec, hBitStream);

          if (spectralBits != qcOutChannel[ch]->sectionData.huffmanBits) {
            return AAC_ENC_WRITE_SPEC_ERROR;
          }
          bitDemand += spectralBits;
        }
        break;

        /* Non data cases */
      case adtscrc_start_reg1:
        if (hTpEnc != NULL) {
          crcReg1 = transportEnc_CrcStartReg(hTpEnc, 192);
        }
        break;
      case adtscrc_start_reg2:
        if (hTpEnc != NULL) {
          crcReg2 = transportEnc_CrcStartReg(hTpEnc, 128);
        }
        break;
      case adtscrc_end_reg1:
      case drmcrc_end_reg:
        if (hTpEnc != NULL) {
          transportEnc_CrcEndReg(hTpEnc, crcReg1);
        }
        break;
      case adtscrc_end_reg2:
        if (hTpEnc != NULL) {
          transportEnc_CrcEndReg(hTpEnc, crcReg2);
        }
        break;
      case drmcrc_start_reg:
        if (hTpEnc != NULL) {
          crcReg1 = transportEnc_CrcStartReg(hTpEnc, 0);
        }
        break;
      case next_channel:
        ch = (ch + 1) % numberOfChannels;
        break;
      case link_sequence:
        list = list->next[decision_bit];
        i = -1;
        break;

      default:
        error = AAC_ENC_UNKNOWN;
        break;
    }

    if (error != AAC_ENC_OK) {
      return error;
    }

    i++;

  } while (list->id[i] != end_of_sequence);

bail:
  if (pBitDemand != NULL) {
    *pBitDemand = bitDemand;
  }

  return error;
}

//-----------------------------------------------------------------------------------------------

AAC_ENCODER_ERROR FDKaacEnc_WriteBitstream(HANDLE_TRANSPORTENC hTpEnc,
                                           CHANNEL_MAPPING *channelMapping,
                                           QC_OUT *qcOut, PSY_OUT *psyOut,
                                           QC_STATE *qcKernel,
                                           AUDIO_OBJECT_TYPE aot,
                                           UINT syntaxFlags, SCHAR epConfig) {
  HANDLE_FDK_BITSTREAM hBs = transportEnc_GetBitstream(hTpEnc);
  AAC_ENCODER_ERROR ErrorStatus = AAC_ENC_OK;
  int i, n, doByteAlign = 1;
  INT bitMarkUp;
  INT frameBits;
  /* Get first bit of raw data block.
     In case of ADTS+PCE, AU would start at PCE.
     This is okay because PCE assures alignment. */
  UINT alignAnchor = FDKgetValidBits(hBs);

  frameBits = bitMarkUp = alignAnchor;

  /* Channel element loop */
  for (i = 0; i < channelMapping->nElements; i++) {
    ELEMENT_INFO elInfo = channelMapping->elInfo[i];
    INT elementUsedBits = 0;

    switch (elInfo.elType) {
      case ID_SCE: /* single channel */
      case ID_CPE: /* channel pair */
      case ID_LFE: /* low freq effects channel */
      {
        if (AAC_ENC_OK !=
            (ErrorStatus = FDKaacEnc_ChannelElementWrite(
                 hTpEnc, &elInfo, qcOut->qcElement[i]->qcOutChannel,
                 psyOut->psyOutElement[i],
                 psyOut->psyOutElement[i]->psyOutChannel,
                 syntaxFlags, /* syntaxFlags (ER tools ...) */
                 aot,         /* aot: AOT_AAC_LC, AOT_SBR, AOT_PS */
                 epConfig,    /* epConfig -1, 0, 1 */
                 NULL, 0))) {
          return ErrorStatus;
        }

        if (!(syntaxFlags & AC_ER)) {
          /* Write associated extension payload */
          for (n = 0; n < qcOut->qcElement[i]->nExtensions; n++) {
            FDKaacEnc_writeExtensionData(
                hTpEnc, &qcOut->qcElement[i]->extension[n], 0, alignAnchor,
                syntaxFlags, aot, epConfig);
          }
        }
      } break;

      /* In FDK, DSE signalling explicit done in elDSE. See channel_map.cpp */
      default:
        return AAC_ENC_INVALID_ELEMENTINFO_TYPE;

    } /* switch */

    if (elInfo.elType != ID_DSE) {
      elementUsedBits -= bitMarkUp;
      bitMarkUp = FDKgetValidBits(hBs);
      elementUsedBits += bitMarkUp;
      frameBits += elementUsedBits;
    }

  } /* for (i=0; i<channelMapping.nElements; i++) */

  if ((syntaxFlags & AC_ER) && !(syntaxFlags & AC_DRM)) {
    UCHAR channelElementExtensionWritten[((8))][(
        1)]; /* 0: extension not touched, 1: extension already written */

    FDKmemclear(channelElementExtensionWritten,
                sizeof(channelElementExtensionWritten));

    if (syntaxFlags & AC_ELD) {
      for (i = 0; i < channelMapping->nElements; i++) {
        for (n = 0; n < qcOut->qcElement[i]->nExtensions; n++) {
          if ((qcOut->qcElement[i]->extension[n].type == EXT_SBR_DATA) ||
              (qcOut->qcElement[i]->extension[n].type == EXT_SBR_DATA_CRC)) {
            /* Write sbr extension payload */
            FDKaacEnc_writeExtensionData(
                hTpEnc, &qcOut->qcElement[i]->extension[n], 0, alignAnchor,
                syntaxFlags, aot, epConfig);

            channelElementExtensionWritten[i][n] = 1;
          } /* SBR */
        }   /* n */
      }     /* i */
    }       /* AC_ELD */

    for (i = 0; i < channelMapping->nElements; i++) {
      for (n = 0; n < qcOut->qcElement[i]->nExtensions; n++) {
        if (channelElementExtensionWritten[i][n] == 0) {
          /* Write all ramaining extension payloads in element */
          FDKaacEnc_writeExtensionData(hTpEnc,
                                       &qcOut->qcElement[i]->extension[n], 0,
                                       alignAnchor, syntaxFlags, aot, epConfig);
        }
      } /* n */
    }   /* i */
  }     /* if AC_ER */

  /* Extend global extension payload table with fill bits */
  n = qcOut->nExtensions;

  /* Add fill data / stuffing bits */
  qcOut->extension[n].type = EXT_FILL_DATA;
  qcOut->extension[n].nPayloadBits = qcOut->totFillBits;
  qcOut->nExtensions++;

  /* Write global extension payload and fill data */
  for (n = 0; (n < qcOut->nExtensions) && (n < (2 + 2)); n++) {
    FDKaacEnc_writeExtensionData(hTpEnc, &qcOut->extension[n], 0, alignAnchor,
                                 syntaxFlags, aot, epConfig);

    /* For EXT_FIL or EXT_FILL_DATA we could do an additional sanity check here
     */
  }

  if (!(syntaxFlags & (AC_SCALABLE | AC_ER))) {
    FDKwriteBits(hBs, ID_END, EL_ID_BITS);
  }

  if (doByteAlign) {
    /* Assure byte alignment*/
    if (((FDKgetValidBits(hBs) - alignAnchor + qcOut->alignBits) & 0x7) != 0) {
      return AAC_ENC_WRITTEN_BITS_ERROR;
    }

    FDKaacEnc_ByteAlignment(hBs, qcOut->alignBits);
  }

  frameBits -= bitMarkUp;
  frameBits += FDKgetValidBits(hBs);

  transportEnc_EndAccessUnit(hTpEnc, &frameBits);

  if (frameBits != qcOut->totalBits + qcKernel->globHdrBits) {
    return AAC_ENC_WRITTEN_BITS_ERROR;
  }

  return ErrorStatus;
}
