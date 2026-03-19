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

/**************************** AAC decoder library ******************************

   Author(s):   Robert Weidner (DSP Solutions)

   Description: HCR Decoder: HCR initialization, preprocess HCR sideinfo,
                decode priority codewords (PCWs)

*******************************************************************************/

#include "aacdec_hcr.h"

#include "aacdec_hcr_types.h"
#include "aacdec_hcr_bit.h"
#include "aacdec_hcrs.h"
#include "aac_ram.h"
#include "aac_rom.h"
#include "channel.h"
#include "block.h"

#include "aacdecoder.h" /* for ID_CPE, ID_SCE ... */
#include "FDK_bitstream.h"

extern int mlFileChCurr;

static void errDetectorInHcrSideinfoShrt(SCHAR cb, SHORT numLine,
                                         UINT *errorWord);

static void errDetectorInHcrLengths(SCHAR lengthOfLongestCodeword,
                                    SHORT lengthOfReorderedSpectralData,
                                    UINT *errorWord);

static void HcrCalcNumCodeword(H_HCR_INFO pHcr);
static void HcrSortCodebookAndNumCodewordInSection(H_HCR_INFO pHcr);
static void HcrPrepareSegmentationGrid(H_HCR_INFO pHcr);
static void HcrExtendedSectionInfo(H_HCR_INFO pHcr);

static void DeriveNumberOfExtendedSortedSectionsInSets(
    UINT numSegment, USHORT *pNumExtendedSortedCodewordInSection,
    int numExtendedSortedCodewordInSectionIdx,
    USHORT *pNumExtendedSortedSectionsInSets,
    int numExtendedSortedSectionsInSetsIdx);

static INT DecodeEscapeSequence(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                                INT quantSpecCoef, INT *pLeftStartOfSegment,
                                SCHAR *pRemainingBitsInSegment,
                                int *pNumDecodedBits, UINT *errorWord);

static int DecodePCW_Sign(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                          UINT codebookDim, const SCHAR *pQuantVal,
                          FIXP_DBL *pQuantSpecCoef, int *quantSpecCoefIdx,
                          INT *pLeftStartOfSegment,
                          SCHAR *pRemainingBitsInSegment, int *pNumDecodedBits);

static const SCHAR *DecodePCW_Body(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                                   const UINT *pCurrentTree,
                                   const SCHAR *pQuantValBase,
                                   INT *pLeftStartOfSegment,
                                   SCHAR *pRemainingBitsInSegment,
                                   int *pNumDecodedBits);

static void DecodePCWs(HANDLE_FDK_BITSTREAM bs, H_HCR_INFO pHcr);

static void HcrReorderQuantizedSpectralCoefficients(
    H_HCR_INFO pHcr, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    const SamplingRateInfo *pSamplingRateInfo);

static UCHAR errDetectPcwSegmentation(SCHAR remainingBitsInSegment,
                                      H_HCR_INFO pHcr, PCW_TYPE kind,
                                      FIXP_DBL *qsc_base_of_cw,
                                      UCHAR dimension);

static void errDetectWithinSegmentationFinal(H_HCR_INFO pHcr);

/*---------------------------------------------------------------------------------------------
     description:   Check if codebook and numSect are within allowed range
(short only)
--------------------------------------------------------------------------------------------
*/
static void errDetectorInHcrSideinfoShrt(SCHAR cb, SHORT numLine,
                                         UINT *errorWord) {
  if (cb < ZERO_HCB || cb >= MAX_CB_CHECK || cb == BOOKSCL) {
    *errorWord |= CB_OUT_OF_RANGE_SHORT_BLOCK;
  }
  if (numLine < 0 || numLine > 1024) {
    *errorWord |= LINE_IN_SECT_OUT_OF_RANGE_SHORT_BLOCK;
  }
}

/*---------------------------------------------------------------------------------------------
     description:   Check both HCR lengths
--------------------------------------------------------------------------------------------
*/
static void errDetectorInHcrLengths(SCHAR lengthOfLongestCodeword,
                                    SHORT lengthOfReorderedSpectralData,
                                    UINT *errorWord) {
  if (lengthOfReorderedSpectralData < lengthOfLongestCodeword) {
    *errorWord |= HCR_SI_LENGTHS_FAILURE;
  }
}

/*---------------------------------------------------------------------------------------------
     description:   Decode (and adapt if necessary) the two HCR sideinfo
components: 'reordered_spectral_data_length' and 'longest_codeword_length'
--------------------------------------------------------------------------------------------
*/

void CHcr_Read(HANDLE_FDK_BITSTREAM bs,
               CAacDecoderChannelInfo *pAacDecoderChannelInfo,
               const MP4_ELEMENT_ID globalHcrType) {
  SHORT lengOfReorderedSpectralData;
  SCHAR lengOfLongestCodeword;

  pAacDecoderChannelInfo->pDynData->specificTo.aac.lenOfReorderedSpectralData =
      0;
  pAacDecoderChannelInfo->pDynData->specificTo.aac.lenOfLongestCodeword = 0;

  /* ------- SI-Value No 1 ------- */
  lengOfReorderedSpectralData = FDKreadBits(bs, 14) + ERROR_LORSD;
  if (globalHcrType == ID_CPE) {
    if ((lengOfReorderedSpectralData >= 0) &&
        (lengOfReorderedSpectralData <= CPE_TOP_LENGTH)) {
      pAacDecoderChannelInfo->pDynData->specificTo.aac
          .lenOfReorderedSpectralData =
          lengOfReorderedSpectralData; /* the decoded value is within range */
    } else {
      if (lengOfReorderedSpectralData > CPE_TOP_LENGTH) {
        pAacDecoderChannelInfo->pDynData->specificTo.aac
            .lenOfReorderedSpectralData =
            CPE_TOP_LENGTH; /* use valid maximum */
      }
    }
  } else if (globalHcrType == ID_SCE || globalHcrType == ID_LFE ||
             globalHcrType == ID_CCE) {
    if ((lengOfReorderedSpectralData >= 0) &&
        (lengOfReorderedSpectralData <= SCE_TOP_LENGTH)) {
      pAacDecoderChannelInfo->pDynData->specificTo.aac
          .lenOfReorderedSpectralData =
          lengOfReorderedSpectralData; /* the decoded value is within range */
    } else {
      if (lengOfReorderedSpectralData > SCE_TOP_LENGTH) {
        pAacDecoderChannelInfo->pDynData->specificTo.aac
            .lenOfReorderedSpectralData =
            SCE_TOP_LENGTH; /* use valid maximum */
      }
    }
  }

  /* ------- SI-Value No 2 ------- */
  lengOfLongestCodeword = FDKreadBits(bs, 6) + ERROR_LOLC;
  if ((lengOfLongestCodeword >= 0) &&
      (lengOfLongestCodeword <= LEN_OF_LONGEST_CW_TOP_LENGTH)) {
    pAacDecoderChannelInfo->pDynData->specificTo.aac.lenOfLongestCodeword =
        lengOfLongestCodeword; /* the decoded value is within range */
  } else {
    if (lengOfLongestCodeword > LEN_OF_LONGEST_CW_TOP_LENGTH) {
      pAacDecoderChannelInfo->pDynData->specificTo.aac.lenOfLongestCodeword =
          LEN_OF_LONGEST_CW_TOP_LENGTH; /* use valid maximum */
    }
  }
}

/*---------------------------------------------------------------------------------------------
     description:   Set up HCR - must be called before every call to
HcrDecoder(). For short block a sorting algorithm is applied to get the SI in
the order that HCR could assemble the qsc's as if it is a long block.
-----------------------------------------------------------------------------------------------
        return:     error log
--------------------------------------------------------------------------------------------
*/

UINT HcrInit(H_HCR_INFO pHcr, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
             const SamplingRateInfo *pSamplingRateInfo,
             HANDLE_FDK_BITSTREAM bs) {
  CIcsInfo *pIcsInfo = &pAacDecoderChannelInfo->icsInfo;
  SHORT *pNumLinesInSec;
  UCHAR *pCodeBk;
  SHORT numSection;
  SCHAR cb;
  int numLine;
  int i;

  pHcr->decInOut.lengthOfReorderedSpectralData =
      pAacDecoderChannelInfo->pDynData->specificTo.aac
          .lenOfReorderedSpectralData;
  pHcr->decInOut.lengthOfLongestCodeword =
      pAacDecoderChannelInfo->pDynData->specificTo.aac.lenOfLongestCodeword;
  pHcr->decInOut.pQuantizedSpectralCoefficientsBase =
      pAacDecoderChannelInfo->pSpectralCoefficient;
  pHcr->decInOut.quantizedSpectralCoefficientsIdx = 0;
  pHcr->decInOut.pCodebook =
      pAacDecoderChannelInfo->pDynData->specificTo.aac.aCodeBooks4Hcr;
  pHcr->decInOut.pNumLineInSect =
      pAacDecoderChannelInfo->pDynData->specificTo.aac.aNumLineInSec4Hcr;
  pHcr->decInOut.numSection =
      pAacDecoderChannelInfo->pDynData->specificTo.aac.numberSection;
  pHcr->decInOut.errorLog = 0;
  pHcr->nonPcwSideinfo.pResultBase =
      SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);

  FDKsyncCache(bs);
  pHcr->decInOut.bitstreamAnchor = (INT)FDKgetValidBits(bs);

  if (!IsLongBlock(&pAacDecoderChannelInfo->icsInfo)) /* short block */
  {
    SHORT band;
    SHORT maxBand;
    SCHAR group;
    SCHAR winGroupLen;
    SCHAR window;
    SCHAR numUnitInBand;
    SCHAR cntUnitInBand;
    SCHAR groupWin;
    SCHAR cb_prev;

    UCHAR *pCodeBook;
    const SHORT *BandOffsets;
    SCHAR numOfGroups;

    pCodeBook = pAacDecoderChannelInfo->pDynData->aCodeBook; /* in */
    pNumLinesInSec = pHcr->decInOut.pNumLineInSect;          /* out */
    pCodeBk = pHcr->decInOut.pCodebook;                      /* out */
    BandOffsets =
        GetScaleFactorBandOffsets(pIcsInfo, pSamplingRateInfo); /* aux */
    numOfGroups = GetWindowGroups(pIcsInfo);

    numLine = 0;
    numSection = 0;
    cb = pCodeBook[0];
    cb_prev = pCodeBook[0];

    /* convert HCR-sideinfo into a unitwise manner: When the cb changes, a new
     * section starts */

    *pCodeBk++ = cb_prev;

    maxBand = GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);
    for (band = 0; band < maxBand;
         band++) { /* from low to high sfbs i.e. from low to high frequencies */
      numUnitInBand =
          ((BandOffsets[band + 1] - BandOffsets[band]) >>
           FOUR_LOG_DIV_TWO_LOG); /* get the number of units in current sfb */
      for (cntUnitInBand = numUnitInBand; cntUnitInBand != 0;
           cntUnitInBand--) { /* for every unit in the band */
        for (window = 0, group = 0; group < numOfGroups; group++) {
          winGroupLen = (SCHAR)GetWindowGroupLength(
              &pAacDecoderChannelInfo->icsInfo, group);
          for (groupWin = winGroupLen; groupWin != 0; groupWin--, window++) {
            cb = pCodeBook[group * 16 + band];
            if (cb != cb_prev) {
              errDetectorInHcrSideinfoShrt(cb, numLine,
                                           &pHcr->decInOut.errorLog);
              if (pHcr->decInOut.errorLog != 0) {
                return (pHcr->decInOut.errorLog);
              }
              *pCodeBk++ = cb;
              *pNumLinesInSec++ = numLine;
              numSection++;

              cb_prev = cb;
              numLine = LINES_PER_UNIT;
            } else {
              numLine += LINES_PER_UNIT;
            }
          }
        }
      }
    }

    numSection++;

    errDetectorInHcrSideinfoShrt(cb, numLine, &pHcr->decInOut.errorLog);
    if (numSection <= 0 || numSection > 1024 / 2) {
      pHcr->decInOut.errorLog |= NUM_SECT_OUT_OF_RANGE_SHORT_BLOCK;
    }
    errDetectorInHcrLengths(pHcr->decInOut.lengthOfLongestCodeword,
                            pHcr->decInOut.lengthOfReorderedSpectralData,
                            &pHcr->decInOut.errorLog);
    if (pHcr->decInOut.errorLog != 0) {
      return (pHcr->decInOut.errorLog);
    }

    *pCodeBk = cb;
    *pNumLinesInSec = numLine;
    pHcr->decInOut.numSection = numSection;

  } else /* end short block prepare SI */
  {      /* long block */
    errDetectorInHcrLengths(pHcr->decInOut.lengthOfLongestCodeword,
                            pHcr->decInOut.lengthOfReorderedSpectralData,
                            &pHcr->decInOut.errorLog);
    numSection = pHcr->decInOut.numSection;
    pNumLinesInSec = pHcr->decInOut.pNumLineInSect;
    pCodeBk = pHcr->decInOut.pCodebook;
    if (numSection <= 0 || numSection > 64) {
      pHcr->decInOut.errorLog |= NUM_SECT_OUT_OF_RANGE_LONG_BLOCK;
      numSection = 0;
    }

    for (i = numSection; i != 0; i--) {
      cb = *pCodeBk++;

      if (cb < ZERO_HCB || cb >= MAX_CB_CHECK || cb == BOOKSCL) {
        pHcr->decInOut.errorLog |= CB_OUT_OF_RANGE_LONG_BLOCK;
      }

      numLine = *pNumLinesInSec++;
      /* FDK_ASSERT(numLine > 0); */

      if ((numLine <= 0) || (numLine > 1024)) {
        pHcr->decInOut.errorLog |= LINE_IN_SECT_OUT_OF_RANGE_LONG_BLOCK;
      }
    }
    if (pHcr->decInOut.errorLog != 0) {
      return (pHcr->decInOut.errorLog);
    }
  }

  pCodeBk = pHcr->decInOut.pCodebook;
  for (i = 0; i < numSection; i++) {
    if ((*pCodeBk == NOISE_HCB) || (*pCodeBk == INTENSITY_HCB2) ||
        (*pCodeBk == INTENSITY_HCB)) {
      *pCodeBk = 0;
    }
    pCodeBk++;
  }

  /* HCR-sideinfo-input is complete and seems to be valid */

  return (pHcr->decInOut.errorLog);
}

/*---------------------------------------------------------------------------------------------
     description:   This function decodes the codewords of the spectral
coefficients from the bitstream according to the HCR algorithm and stores the
quantized spectral coefficients in correct order in the output buffer.
--------------------------------------------------------------------------------------------
*/

UINT HcrDecoder(H_HCR_INFO pHcr, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                const SamplingRateInfo *pSamplingRateInfo,
                HANDLE_FDK_BITSTREAM bs) {
  int pTmp1, pTmp2, pTmp3, pTmp4;
  int pTmp5;

  INT bitCntOffst;
  INT saveBitCnt = (INT)FDKgetValidBits(bs); /* save bitstream position */

  HcrCalcNumCodeword(pHcr);

  HcrSortCodebookAndNumCodewordInSection(pHcr);

  HcrPrepareSegmentationGrid(pHcr);

  HcrExtendedSectionInfo(pHcr);

  if ((pHcr->decInOut.errorLog & HCR_FATAL_PCW_ERROR_MASK) != 0) {
    return (pHcr->decInOut.errorLog); /* sideinfo is massively corrupt, return
                                         from HCR without having decoded
                                         anything */
  }

  DeriveNumberOfExtendedSortedSectionsInSets(
      pHcr->segmentInfo.numSegment,
      pHcr->sectionInfo.pNumExtendedSortedCodewordInSection,
      pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx,
      pHcr->sectionInfo.pNumExtendedSortedSectionsInSets,
      pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx);

  /* store */
  pTmp1 = pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx;
  pTmp2 = pHcr->sectionInfo.extendedSortedCodebookIdx;
  pTmp3 = pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx;
  pTmp4 = pHcr->decInOut.quantizedSpectralCoefficientsIdx;
  pTmp5 = pHcr->sectionInfo.maxLenOfCbInExtSrtSecIdx;

  /* ------- decode meaningful PCWs ------ */
  DecodePCWs(bs, pHcr);

  if ((pHcr->decInOut.errorLog & HCR_FATAL_PCW_ERROR_MASK) == 0) {
    /* ------ decode the non-PCWs -------- */
    DecodeNonPCWs(bs, pHcr);
  }

  errDetectWithinSegmentationFinal(pHcr);

  /* restore */
  pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx = pTmp1;
  pHcr->sectionInfo.extendedSortedCodebookIdx = pTmp2;
  pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx = pTmp3;
  pHcr->decInOut.quantizedSpectralCoefficientsIdx = pTmp4;
  pHcr->sectionInfo.maxLenOfCbInExtSrtSecIdx = pTmp5;

  HcrReorderQuantizedSpectralCoefficients(pHcr, pAacDecoderChannelInfo,
                                          pSamplingRateInfo);

  /* restore bitstream position */
  bitCntOffst = (INT)FDKgetValidBits(bs) - saveBitCnt;
  if (bitCntOffst) {
    FDKpushBiDirectional(bs, bitCntOffst);
  }

  return (pHcr->decInOut.errorLog);
}

/*---------------------------------------------------------------------------------------------
     description:   This function reorders the quantized spectral coefficients
sectionwise for long- and short-blocks and compares to the LAV (Largest Absolute
Value of the current codebook) -- a counter is incremented if there is an error
                    detected.
                    Additional for short-blocks a unit-based-deinterleaving is
applied. Moreover (for short blocks) the scaling is derived (compare plain
huffman decoder).
--------------------------------------------------------------------------------------------
*/

static void HcrReorderQuantizedSpectralCoefficients(
    H_HCR_INFO pHcr, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    const SamplingRateInfo *pSamplingRateInfo) {
  INT qsc;
  UINT abs_qsc;
  UINT i, j;
  USHORT numSpectralValuesInSection;
  FIXP_DBL *pTeVa;
  USHORT lavErrorCnt = 0;

  UINT numSection = pHcr->decInOut.numSection;
  SPECTRAL_PTR pQuantizedSpectralCoefficientsBase =
      pHcr->decInOut.pQuantizedSpectralCoefficientsBase;
  FIXP_DBL *pQuantizedSpectralCoefficients =
      SPEC_LONG(pHcr->decInOut.pQuantizedSpectralCoefficientsBase);
  const UCHAR *pCbDimShift = aDimCbShift;
  const USHORT *pLargestAbsVal = aLargestAbsoluteValue;
  UCHAR *pSortedCodebook = pHcr->sectionInfo.pSortedCodebook;
  USHORT *pNumSortedCodewordInSection =
      pHcr->sectionInfo.pNumSortedCodewordInSection;
  USHORT *pReorderOffset = pHcr->sectionInfo.pReorderOffset;
  FIXP_DBL pTempValues[1024];
  FIXP_DBL *pBak = pTempValues;

  FDKmemclear(pTempValues, 1024 * sizeof(FIXP_DBL));

  /* long and short: check if decoded huffman-values (quantized spectral
   * coefficients) are within range */
  for (i = numSection; i != 0; i--) {
    numSpectralValuesInSection = *pNumSortedCodewordInSection++
                                 << pCbDimShift[*pSortedCodebook];
    pTeVa = &pTempValues[*pReorderOffset++];
    for (j = numSpectralValuesInSection; j != 0; j--) {
      qsc = *pQuantizedSpectralCoefficients++;
      abs_qsc = fAbs(qsc);
      if (abs_qsc <= pLargestAbsVal[*pSortedCodebook]) {
        *pTeVa++ = (FIXP_DBL)qsc; /* the qsc value is within range */
      } else {                    /* line is too high .. */
        if (abs_qsc ==
            Q_VALUE_INVALID) { /* .. because of previous marking --> dont set
                                  LAV flag (would be confusing), just copy out
                                  the already marked value */
          *pTeVa++ = (FIXP_DBL)qsc;
        } else { /* .. because a too high value was decoded for this cb --> set
                    LAV flag */
          *pTeVa++ = (FIXP_DBL)Q_VALUE_INVALID;
          lavErrorCnt += 1;
        }
      }
    }
    pSortedCodebook++;
  }

  if (!IsLongBlock(&pAacDecoderChannelInfo->icsInfo)) {
    FIXP_DBL *pOut;
    FIXP_DBL locMax;
    FIXP_DBL tmp;
    SCHAR groupoffset;
    SCHAR group;
    SCHAR band;
    SCHAR groupwin;
    SCHAR window;
    SCHAR numWinGroup;
    SHORT interm;
    SCHAR numSfbTransm;
    SCHAR winGroupLen;
    SHORT index;
    INT msb;
    INT lsb;

    SHORT *pScaleFacHcr = pAacDecoderChannelInfo->pDynData->aScaleFactor;
    SHORT *pSfbSclHcr = pAacDecoderChannelInfo->pDynData->aSfbScale;
    const SHORT *BandOffsets = GetScaleFactorBandOffsets(
        &pAacDecoderChannelInfo->icsInfo, pSamplingRateInfo);

    pBak = pTempValues;
    /* deinterleave unitwise for short blocks */
    for (window = 0; window < (8); window++) {
      pOut = SPEC(pQuantizedSpectralCoefficientsBase, window,
                  pAacDecoderChannelInfo->granuleLength);
      for (i = 0; i < (LINES_PER_UNIT_GROUP); i++) {
        pTeVa = pBak + (window << FOUR_LOG_DIV_TWO_LOG) +
                i * 32; /* distance of lines between unit groups has to be
                           constant for every framelength (32)!  */
        for (j = (LINES_PER_UNIT); j != 0; j--) {
          *pOut++ = *pTeVa++;
        }
      }
    }

    /* short blocks only */
    /* derive global scaling-value for every sfb and every window (as it is done
     * in plain-huffman-decoder at short blocks) */
    groupoffset = 0;

    numWinGroup = GetWindowGroups(&pAacDecoderChannelInfo->icsInfo);
    numSfbTransm =
        GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo->icsInfo);

    for (group = 0; group < numWinGroup; group++) {
      winGroupLen =
          GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, group);
      for (band = 0; band < numSfbTransm; band++) {
        interm = group * 16 + band;
        msb = pScaleFacHcr[interm] >> 2;
        lsb = pScaleFacHcr[interm] & 3;
        for (groupwin = 0; groupwin < winGroupLen; groupwin++) {
          window = groupoffset + groupwin;
          pBak = SPEC(pQuantizedSpectralCoefficientsBase, window,
                      pAacDecoderChannelInfo->granuleLength);
          locMax = FL2FXCONST_DBL(0.0f);
          for (index = BandOffsets[band]; index < BandOffsets[band + 1];
               index += LINES_PER_UNIT) {
            pTeVa = &pBak[index];
            for (i = LINES_PER_UNIT; i != 0; i--) {
              tmp = (*pTeVa < FL2FXCONST_DBL(0.0f)) ? -*pTeVa++ : *pTeVa++;
              locMax = fixMax(tmp, locMax);
            }
          }
          if (fixp_abs(locMax) > (FIXP_DBL)MAX_QUANTIZED_VALUE) {
            locMax = (FIXP_DBL)MAX_QUANTIZED_VALUE;
          }
          pSfbSclHcr[window * 16 + band] =
              msb - GetScaleFromValue(
                        locMax, lsb); /* save global scale maxima in this sfb */
        }
      }
      groupoffset +=
          GetWindowGroupLength(&pAacDecoderChannelInfo->icsInfo, group);
    }
  } else {
    /* copy straight for long-blocks */
    pQuantizedSpectralCoefficients =
        SPEC_LONG(pQuantizedSpectralCoefficientsBase);
    for (i = 1024; i != 0; i--) {
      *pQuantizedSpectralCoefficients++ = *pBak++;
    }
  }

  if (lavErrorCnt != 0) {
    pHcr->decInOut.errorLog |= LAV_VIOLATION;
  }
}

/*---------------------------------------------------------------------------------------------
     description:   This function calculates the number of codewords
                    for each section (numCodewordInSection) and the number of
codewords for all sections (numCodeword). For zero and intensity codebooks a
entry is also done in the variable numCodewordInSection. It is assumed that the
codebook is a two tuples codebook. This is needed later for the calculation of
the base addresses for the reordering of the quantize spectral coefficients at
the end of the hcr tool. The variable numCodeword contain the number of
codewords which are really in the bitstream. Zero or intensity codebooks does
not increase the variable numCodewords.
-----------------------------------------------------------------------------------------------
        return:   -
--------------------------------------------------------------------------------------------
*/

static void HcrCalcNumCodeword(H_HCR_INFO pHcr) {
  int hcrSection;
  UINT numCodeword;

  UINT numSection = pHcr->decInOut.numSection;
  UCHAR *pCodebook = pHcr->decInOut.pCodebook;
  SHORT *pNumLineInSection = pHcr->decInOut.pNumLineInSect;
  const UCHAR *pCbDimShift = aDimCbShift;

  USHORT *pNumCodewordInSection = pHcr->sectionInfo.pNumCodewordInSection;

  numCodeword = 0;
  for (hcrSection = numSection; hcrSection != 0; hcrSection--) {
    *pNumCodewordInSection = *pNumLineInSection++ >> pCbDimShift[*pCodebook];
    if (*pCodebook != 0) {
      numCodeword += *pNumCodewordInSection;
    }
    pNumCodewordInSection++;
    pCodebook++;
  }
  pHcr->sectionInfo.numCodeword = numCodeword;
}

/*---------------------------------------------------------------------------------------------
     description:   This function calculates the number
                    of sorted codebooks and sorts the codebooks and the
numCodewordInSection according to the priority.
--------------------------------------------------------------------------------------------
*/

static void HcrSortCodebookAndNumCodewordInSection(H_HCR_INFO pHcr) {
  UINT i, j, k;
  UCHAR temp;
  UINT counter;
  UINT startOffset;
  UINT numZeroSection;
  UCHAR *pDest;
  UINT numSectionDec;

  UINT numSection = pHcr->decInOut.numSection;
  UCHAR *pCodebook = pHcr->decInOut.pCodebook;
  UCHAR *pSortedCodebook = pHcr->sectionInfo.pSortedCodebook;
  USHORT *pNumCodewordInSection = pHcr->sectionInfo.pNumCodewordInSection;
  USHORT *pNumSortedCodewordInSection =
      pHcr->sectionInfo.pNumSortedCodewordInSection;
  UCHAR *pCodebookSwitch = pHcr->sectionInfo.pCodebookSwitch;
  USHORT *pReorderOffset = pHcr->sectionInfo.pReorderOffset;
  const UCHAR *pCbPriority = aCbPriority;
  const UCHAR *pMinOfCbPair = aMinOfCbPair;
  const UCHAR *pMaxOfCbPair = aMaxOfCbPair;
  const UCHAR *pCbDimShift = aDimCbShift;

  UINT searchStart = 0;

  /* calculate *pNumSortedSection and store the priorities in array
   * pSortedCdebook */
  pDest = pSortedCodebook;
  numZeroSection = 0;
  for (i = numSection; i != 0; i--) {
    if (pCbPriority[*pCodebook] == 0) {
      numZeroSection += 1;
    }
    *pDest++ = pCbPriority[*pCodebook++];
  }
  pHcr->sectionInfo.numSortedSection =
      numSection - numZeroSection; /* numSortedSection contains no zero or
                                      intensity section */
  pCodebook = pHcr->decInOut.pCodebook;

  /* sort priorities of the codebooks in array pSortedCdebook[] */
  numSectionDec = numSection - 1;
  if (numSectionDec > 0) {
    counter = numSectionDec;
    for (j = numSectionDec; j != 0; j--) {
      for (i = 0; i < counter; i++) {
        /* swap priorities */
        if (pSortedCodebook[i + 1] > pSortedCodebook[i]) {
          temp = pSortedCodebook[i];
          pSortedCodebook[i] = pSortedCodebook[i + 1];
          pSortedCodebook[i + 1] = temp;
        }
      }
      counter -= 1;
    }
  }

  /* clear codebookSwitch array */
  for (i = numSection; i != 0; i--) {
    *pCodebookSwitch++ = 0;
  }
  pCodebookSwitch = pHcr->sectionInfo.pCodebookSwitch;

  /* sort sectionCodebooks and numCodwordsInSection and calculate
   * pReorderOffst[j] */
  for (j = 0; j < numSection; j++) {
    for (i = searchStart; i < numSection; i++) {
      if (pCodebookSwitch[i] == 0 &&
          (pMinOfCbPair[pSortedCodebook[j]] == pCodebook[i] ||
           pMaxOfCbPair[pSortedCodebook[j]] == pCodebook[i])) {
        pCodebookSwitch[i] = 1;
        pSortedCodebook[j] = pCodebook[i]; /* sort codebook */
        pNumSortedCodewordInSection[j] =
            pNumCodewordInSection[i]; /* sort NumCodewordInSection */

        startOffset = 0;
        for (k = 0; k < i; k++) { /* make entry in pReorderOffst */
          startOffset += pNumCodewordInSection[k] << pCbDimShift[pCodebook[k]];
        }
        pReorderOffset[j] =
            startOffset; /* offset for reordering the codewords */

        if (i == searchStart) {
          k = i;
          while (pCodebookSwitch[k++] == 1) searchStart++;
        }
        break;
      }
    }
  }
}

/*---------------------------------------------------------------------------------------------
     description:   This function calculates the segmentation, which includes
numSegment, leftStartOfSegment, rightStartOfSegment and remainingBitsInSegment.
                    The segmentation could be visualized a as kind of
'overlay-grid' for the bitstream-block holding the HCR-encoded
quantized-spectral-coefficients.
--------------------------------------------------------------------------------------------
*/

static void HcrPrepareSegmentationGrid(H_HCR_INFO pHcr) {
  USHORT i, j;
  USHORT numSegment = 0;
  INT segmentStart = 0;
  UCHAR segmentWidth;
  UCHAR lastSegmentWidth;
  UCHAR sortedCodebook;
  UCHAR endFlag = 0;
  INT intermediateResult;

  SCHAR lengthOfLongestCodeword = pHcr->decInOut.lengthOfLongestCodeword;
  SHORT lengthOfReorderedSpectralData =
      pHcr->decInOut.lengthOfReorderedSpectralData;
  UINT numSortedSection = pHcr->sectionInfo.numSortedSection;
  UCHAR *pSortedCodebook = pHcr->sectionInfo.pSortedCodebook;
  USHORT *pNumSortedCodewordInSection =
      pHcr->sectionInfo.pNumSortedCodewordInSection;
  INT *pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  INT *pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  SCHAR *pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  const UCHAR *pMaxCwLength = aMaxCwLen;

  for (i = numSortedSection; i != 0; i--) {
    sortedCodebook = *pSortedCodebook++;
    segmentWidth =
        fMin((INT)pMaxCwLength[sortedCodebook], (INT)lengthOfLongestCodeword);

    for (j = *pNumSortedCodewordInSection; j != 0; j--) {
      /* width allows a new segment */
      intermediateResult = segmentStart;
      if ((segmentStart + segmentWidth) <= lengthOfReorderedSpectralData) {
        /* store segment start, segment length and increment the number of
         * segments */
        *pLeftStartOfSegment++ = intermediateResult;
        *pRightStartOfSegment++ = intermediateResult + segmentWidth - 1;
        *pRemainingBitsInSegment++ = segmentWidth;
        segmentStart += segmentWidth;
        numSegment += 1;
      }
      /* width does not allow a new segment */
      else {
        /* correct the last segment length */
        pLeftStartOfSegment--;
        pRightStartOfSegment--;
        pRemainingBitsInSegment--;
        segmentStart = *pLeftStartOfSegment;

        lastSegmentWidth = lengthOfReorderedSpectralData - segmentStart;
        *pRemainingBitsInSegment = lastSegmentWidth;
        *pRightStartOfSegment = segmentStart + lastSegmentWidth - 1;
        endFlag = 1;
        break;
      }
    }
    pNumSortedCodewordInSection++;
    if (endFlag != 0) {
      break;
    }
  }
  pHcr->segmentInfo.numSegment = numSegment;
}

/*---------------------------------------------------------------------------------------------
     description:   This function adapts the sorted section boundaries to the
boundaries of segmentation. If the section lengths does not fit completely into
the current segment, the section is spitted into two so called 'extended
                    sections'. The extended-section-info
(pNumExtendedSortedCodewordInSectin and pExtendedSortedCodebook) is updated in
this case.

--------------------------------------------------------------------------------------------
*/

static void HcrExtendedSectionInfo(H_HCR_INFO pHcr) {
  UINT srtSecCnt = 0; /* counter for sorted sections */
  UINT xSrtScCnt = 0; /* counter for extended sorted sections */
  UINT remainNumCwInSortSec;
  UINT inSegmentRemainNumCW;

  UINT numSortedSection = pHcr->sectionInfo.numSortedSection;
  UCHAR *pSortedCodebook = pHcr->sectionInfo.pSortedCodebook;
  USHORT *pNumSortedCodewordInSection =
      pHcr->sectionInfo.pNumSortedCodewordInSection;
  UCHAR *pExtendedSortedCoBo = pHcr->sectionInfo.pExtendedSortedCodebook;
  USHORT *pNumExtSortCwInSect =
      pHcr->sectionInfo.pNumExtendedSortedCodewordInSection;
  UINT numSegment = pHcr->segmentInfo.numSegment;
  UCHAR *pMaxLenOfCbInExtSrtSec = pHcr->sectionInfo.pMaxLenOfCbInExtSrtSec;
  SCHAR lengthOfLongestCodeword = pHcr->decInOut.lengthOfLongestCodeword;
  const UCHAR *pMaxCwLength = aMaxCwLen;

  remainNumCwInSortSec = pNumSortedCodewordInSection[srtSecCnt];
  inSegmentRemainNumCW = numSegment;

  while (srtSecCnt < numSortedSection) {
    if (inSegmentRemainNumCW < remainNumCwInSortSec) {
      pNumExtSortCwInSect[xSrtScCnt] = inSegmentRemainNumCW;
      pExtendedSortedCoBo[xSrtScCnt] = pSortedCodebook[srtSecCnt];

      remainNumCwInSortSec -= inSegmentRemainNumCW;
      inSegmentRemainNumCW = numSegment;
      /* data of a sorted section was not integrated in extended sorted section
       */
    } else if (inSegmentRemainNumCW == remainNumCwInSortSec) {
      pNumExtSortCwInSect[xSrtScCnt] = inSegmentRemainNumCW;
      pExtendedSortedCoBo[xSrtScCnt] = pSortedCodebook[srtSecCnt];

      srtSecCnt++;
      remainNumCwInSortSec = pNumSortedCodewordInSection[srtSecCnt];
      inSegmentRemainNumCW = numSegment;
      /* data of a sorted section was integrated in extended sorted section */
    } else { /* inSegmentRemainNumCW > remainNumCwInSortSec */
      pNumExtSortCwInSect[xSrtScCnt] = remainNumCwInSortSec;
      pExtendedSortedCoBo[xSrtScCnt] = pSortedCodebook[srtSecCnt];

      inSegmentRemainNumCW -= remainNumCwInSortSec;
      srtSecCnt++;
      remainNumCwInSortSec = pNumSortedCodewordInSection[srtSecCnt];
      /* data of a sorted section was integrated in extended sorted section */
    }
    pMaxLenOfCbInExtSrtSec[xSrtScCnt] =
        fMin((INT)pMaxCwLength[pExtendedSortedCoBo[xSrtScCnt]],
             (INT)lengthOfLongestCodeword);

    xSrtScCnt += 1;

    if (xSrtScCnt >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
      pHcr->decInOut.errorLog |= EXTENDED_SORTED_COUNTER_OVERFLOW;
      return;
    }
  }
  pNumExtSortCwInSect[xSrtScCnt] = 0;
}

/*---------------------------------------------------------------------------------------------
     description:   This function calculates the number of extended sorted
sections which belong to the sets. Each set from set 0 (one and only set for the
PCWs) till to the last set gets a entry in the array to which
                    'pNumExtendedSortedSectinsInSets' points to.

                    Calculation: The entrys in
pNumExtendedSortedCodewordInSectin are added untill the value numSegment is
reached. Then the sum_variable is cleared and the calculation starts from the
beginning. As much extended sorted Sections are summed up to reach the value
numSegment, as much is the current entry in *pNumExtendedSortedCodewordInSectin.
--------------------------------------------------------------------------------------------
*/
static void DeriveNumberOfExtendedSortedSectionsInSets(
    UINT numSegment, USHORT *pNumExtendedSortedCodewordInSection,
    int numExtendedSortedCodewordInSectionIdx,
    USHORT *pNumExtendedSortedSectionsInSets,
    int numExtendedSortedSectionsInSetsIdx) {
  USHORT counter = 0;
  UINT cwSum = 0;
  USHORT *pNumExSortCwInSec = pNumExtendedSortedCodewordInSection;
  USHORT *pNumExSortSecInSets = pNumExtendedSortedSectionsInSets;

  while (pNumExSortCwInSec[numExtendedSortedCodewordInSectionIdx] != 0) {
    cwSum += pNumExSortCwInSec[numExtendedSortedCodewordInSectionIdx];
    numExtendedSortedCodewordInSectionIdx++;
    if (numExtendedSortedCodewordInSectionIdx >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
      return;
    }
    if (cwSum > numSegment) {
      return;
    }
    counter++;
    if (counter > 1024 / 4) {
      return;
    }
    if (cwSum == numSegment) {
      pNumExSortSecInSets[numExtendedSortedSectionsInSetsIdx] = counter;
      numExtendedSortedSectionsInSetsIdx++;
      if (numExtendedSortedSectionsInSetsIdx >= MAX_HCR_SETS) {
        return;
      }
      counter = 0;
      cwSum = 0;
    }
  }
  pNumExSortSecInSets[numExtendedSortedSectionsInSetsIdx] =
      counter; /* save last entry for the last - probably shorter - set */
}

/*---------------------------------------------------------------------------------------------
     description:   This function decodes all priority codewords (PCWs) in a
spectrum (within set 0). The calculation of the PCWs is managed in two loops.
The loopcounter of the outer loop is set to the first value pointer
                    pNumExtendedSortedSectionsInSets points to. This value
represents the number of extended sorted sections within set 0. The loopcounter
of the inner loop is set to the first value pointer
                    pNumExtendedSortedCodewordInSectin points to. The value
represents the number of extended sorted codewords in sections (the original
sections have been splitted to go along with the borders of the sets). Each time
the number of the extended sorted codewords in sections are de- coded, the
pointer 'pNumExtendedSortedCodewordInSectin' is incremented by one.
--------------------------------------------------------------------------------------------
*/
static void DecodePCWs(HANDLE_FDK_BITSTREAM bs, H_HCR_INFO pHcr) {
  UINT i;
  USHORT extSortSec;
  USHORT curExtSortCwInSec;
  UCHAR codebook;
  UCHAR dimension;
  const UINT *pCurrentTree;
  const SCHAR *pQuantValBase;
  const SCHAR *pQuantVal;

  USHORT *pNumExtendedSortedCodewordInSection =
      pHcr->sectionInfo.pNumExtendedSortedCodewordInSection;
  int numExtendedSortedCodewordInSectionIdx =
      pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx;
  UCHAR *pExtendedSortedCodebook = pHcr->sectionInfo.pExtendedSortedCodebook;
  int extendedSortedCodebookIdx = pHcr->sectionInfo.extendedSortedCodebookIdx;
  USHORT *pNumExtendedSortedSectionsInSets =
      pHcr->sectionInfo.pNumExtendedSortedSectionsInSets;
  int numExtendedSortedSectionsInSetsIdx =
      pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx;
  FIXP_DBL *pQuantizedSpectralCoefficients =
      SPEC_LONG(pHcr->decInOut.pQuantizedSpectralCoefficientsBase);
  int quantizedSpectralCoefficientsIdx =
      pHcr->decInOut.quantizedSpectralCoefficientsIdx;
  INT *pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  SCHAR *pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  UCHAR *pMaxLenOfCbInExtSrtSec = pHcr->sectionInfo.pMaxLenOfCbInExtSrtSec;
  int maxLenOfCbInExtSrtSecIdx = pHcr->sectionInfo.maxLenOfCbInExtSrtSecIdx;
  UCHAR maxAllowedCwLen;
  int numDecodedBits;
  const UCHAR *pCbDimension = aDimCb;
  const UCHAR *pCbSign = aSignCb;

  /* clear result array */
  FDKmemclear(pQuantizedSpectralCoefficients + quantizedSpectralCoefficientsIdx,
              1024 * sizeof(FIXP_DBL));

  /* decode all PCWs in the extended sorted section(s) belonging to set 0 */
  for (extSortSec =
           pNumExtendedSortedSectionsInSets[numExtendedSortedSectionsInSetsIdx];
       extSortSec != 0; extSortSec--) {
    codebook =
        pExtendedSortedCodebook[extendedSortedCodebookIdx]; /* get codebook for
                                                               this extended
                                                               sorted section
                                                               and increment ptr
                                                               to cb of next
                                                               ext. sort sec */
    extendedSortedCodebookIdx++;
    if (extendedSortedCodebookIdx >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
      return;
    }
    dimension = pCbDimension[codebook]; /* get dimension of codebook of this
                                           extended sort. sec. */
    pCurrentTree =
        aHuffTable[codebook]; /* convert codebook to pointer to QSCs */
    pQuantValBase =
        aQuantTable[codebook]; /* convert codebook to index to table of QSCs */
    maxAllowedCwLen = pMaxLenOfCbInExtSrtSec[maxLenOfCbInExtSrtSecIdx];
    maxLenOfCbInExtSrtSecIdx++;
    if (maxLenOfCbInExtSrtSecIdx >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
      return;
    }

    /* switch for decoding with different codebooks: */
    if (pCbSign[codebook] ==
        0) { /* no sign bits follow after the codeword-body */
      /* PCW_BodyONLY */
      /*==============*/

      for (curExtSortCwInSec = pNumExtendedSortedCodewordInSection
               [numExtendedSortedCodewordInSectionIdx];
           curExtSortCwInSec != 0; curExtSortCwInSec--) {
        numDecodedBits = 0;

        /* decode PCW_BODY */
        pQuantVal = DecodePCW_Body(
            bs, pHcr->decInOut.bitstreamAnchor, pCurrentTree, pQuantValBase,
            pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits);

        /* result is written out here because NO sign bits follow the body */
        for (i = dimension; i != 0; i--) {
          pQuantizedSpectralCoefficients[quantizedSpectralCoefficientsIdx] =
              (FIXP_DBL)*pQuantVal++; /* write quant. spec. coef. into
                                         spectrum; sign is already valid */
          quantizedSpectralCoefficientsIdx++;
          if (quantizedSpectralCoefficientsIdx >= 1024) {
            return;
          }
        }

        /* one more PCW should be decoded */

        if (maxAllowedCwLen < (numDecodedBits + ERROR_PCW_BODY_ONLY_TOO_LONG)) {
          pHcr->decInOut.errorLog |= TOO_MANY_PCW_BODY_BITS_DECODED;
        }

        if (1 == errDetectPcwSegmentation(
                     *pRemainingBitsInSegment - ERROR_PCW_BODY, pHcr, PCW_BODY,
                     pQuantizedSpectralCoefficients +
                         quantizedSpectralCoefficientsIdx - dimension,
                     dimension)) {
          return;
        }
        pLeftStartOfSegment++; /* update pointer for decoding the next PCW */
        pRemainingBitsInSegment++; /* update pointer for decoding the next PCW
                                    */
      }
    } else if ((codebook < 11) && (pCbSign[codebook] ==
                                   1)) { /* possibly there follow 1,2,3 or 4
                                            sign bits after the codeword-body */
      /* PCW_Body and PCW_Sign */
      /*=======================*/

      for (curExtSortCwInSec = pNumExtendedSortedCodewordInSection
               [numExtendedSortedCodewordInSectionIdx];
           curExtSortCwInSec != 0; curExtSortCwInSec--) {
        int err;
        numDecodedBits = 0;

        pQuantVal = DecodePCW_Body(
            bs, pHcr->decInOut.bitstreamAnchor, pCurrentTree, pQuantValBase,
            pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits);

        err = DecodePCW_Sign(
            bs, pHcr->decInOut.bitstreamAnchor, dimension, pQuantVal,
            pQuantizedSpectralCoefficients, &quantizedSpectralCoefficientsIdx,
            pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits);
        if (err != 0) {
          return;
        }
        /* one more PCW should be decoded */

        if (maxAllowedCwLen < (numDecodedBits + ERROR_PCW_BODY_SIGN_TOO_LONG)) {
          pHcr->decInOut.errorLog |= TOO_MANY_PCW_BODY_SIGN_BITS_DECODED;
        }

        if (1 == errDetectPcwSegmentation(
                     *pRemainingBitsInSegment - ERROR_PCW_BODY_SIGN, pHcr,
                     PCW_BODY_SIGN,
                     pQuantizedSpectralCoefficients +
                         quantizedSpectralCoefficientsIdx - dimension,
                     dimension)) {
          return;
        }
        pLeftStartOfSegment++;
        pRemainingBitsInSegment++;
      }
    } else if ((pCbSign[codebook] == 1) &&
               (codebook >= 11)) { /* possibly there follow some sign bits and
                                      maybe one or two escape sequences after
                                      the cw-body */
      /* PCW_Body, PCW_Sign and maybe PCW_Escape */
      /*=========================================*/

      for (curExtSortCwInSec = pNumExtendedSortedCodewordInSection
               [numExtendedSortedCodewordInSectionIdx];
           curExtSortCwInSec != 0; curExtSortCwInSec--) {
        int err;
        numDecodedBits = 0;

        /* decode PCW_BODY */
        pQuantVal = DecodePCW_Body(
            bs, pHcr->decInOut.bitstreamAnchor, pCurrentTree, pQuantValBase,
            pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits);

        err = DecodePCW_Sign(
            bs, pHcr->decInOut.bitstreamAnchor, dimension, pQuantVal,
            pQuantizedSpectralCoefficients, &quantizedSpectralCoefficientsIdx,
            pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits);
        if (err != 0) {
          return;
        }

        /* decode PCW_ESCAPE if present */
        quantizedSpectralCoefficientsIdx -= DIMENSION_OF_ESCAPE_CODEBOOK;

        if (fixp_abs(pQuantizedSpectralCoefficients
                         [quantizedSpectralCoefficientsIdx]) ==
            (FIXP_DBL)ESCAPE_VALUE) {
          pQuantizedSpectralCoefficients[quantizedSpectralCoefficientsIdx] =
              (FIXP_DBL)DecodeEscapeSequence(
                  bs, pHcr->decInOut.bitstreamAnchor,
                  pQuantizedSpectralCoefficients
                      [quantizedSpectralCoefficientsIdx],
                  pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits,
                  &pHcr->decInOut.errorLog);
        }
        quantizedSpectralCoefficientsIdx++;
        if (quantizedSpectralCoefficientsIdx >= 1024) {
          return;
        }

        if (fixp_abs(pQuantizedSpectralCoefficients
                         [quantizedSpectralCoefficientsIdx]) ==
            (FIXP_DBL)ESCAPE_VALUE) {
          pQuantizedSpectralCoefficients[quantizedSpectralCoefficientsIdx] =
              (FIXP_DBL)DecodeEscapeSequence(
                  bs, pHcr->decInOut.bitstreamAnchor,
                  pQuantizedSpectralCoefficients
                      [quantizedSpectralCoefficientsIdx],
                  pLeftStartOfSegment, pRemainingBitsInSegment, &numDecodedBits,
                  &pHcr->decInOut.errorLog);
        }
        quantizedSpectralCoefficientsIdx++;
        if (quantizedSpectralCoefficientsIdx >= 1024) {
          return;
        }

        /* one more PCW should be decoded */

        if (maxAllowedCwLen <
            (numDecodedBits + ERROR_PCW_BODY_SIGN_ESC_TOO_LONG)) {
          pHcr->decInOut.errorLog |= TOO_MANY_PCW_BODY_SIGN_ESC_BITS_DECODED;
        }

        if (1 == errDetectPcwSegmentation(
                     *pRemainingBitsInSegment - ERROR_PCW_BODY_SIGN_ESC, pHcr,
                     PCW_BODY_SIGN_ESC,
                     pQuantizedSpectralCoefficients +
                         quantizedSpectralCoefficientsIdx -
                         DIMENSION_OF_ESCAPE_CODEBOOK,
                     DIMENSION_OF_ESCAPE_CODEBOOK)) {
          return;
        }
        pLeftStartOfSegment++;
        pRemainingBitsInSegment++;
      }
    }

    /* all PCWs belonging to this extended section should be decoded */
    numExtendedSortedCodewordInSectionIdx++;
    if (numExtendedSortedCodewordInSectionIdx >= MAX_SFB_HCR + MAX_HCR_SETS) {
      return;
    }
  }
  /* all PCWs should be decoded */

  numExtendedSortedSectionsInSetsIdx++;
  if (numExtendedSortedSectionsInSetsIdx >= MAX_HCR_SETS) {
    return;
  }

  /* Write back indexes into structure */
  pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx =
      numExtendedSortedCodewordInSectionIdx;
  pHcr->sectionInfo.extendedSortedCodebookIdx = extendedSortedCodebookIdx;
  pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx =
      numExtendedSortedSectionsInSetsIdx;
  pHcr->decInOut.quantizedSpectralCoefficientsIdx =
      quantizedSpectralCoefficientsIdx;
  pHcr->sectionInfo.maxLenOfCbInExtSrtSecIdx = maxLenOfCbInExtSrtSecIdx;
}

/*---------------------------------------------------------------------------------------------
     description:   This function checks immediately after every decoded PCW,
whether out of the current segment too many bits have been read or not. If an
error occurrs, probably the sideinfo or the HCR-bitstream block holding the
huffman encoded quantized spectral coefficients is distorted. In this case the
two or four quantized spectral coefficients belonging to the current codeword
                    are marked (for being detected by concealment later).
--------------------------------------------------------------------------------------------
*/
static UCHAR errDetectPcwSegmentation(SCHAR remainingBitsInSegment,
                                      H_HCR_INFO pHcr, PCW_TYPE kind,
                                      FIXP_DBL *qsc_base_of_cw,
                                      UCHAR dimension) {
  SCHAR i;
  if (remainingBitsInSegment < 0) {
    /* log the error */
    switch (kind) {
      case PCW_BODY:
        pHcr->decInOut.errorLog |= SEGMENT_OVERRIDE_ERR_PCW_BODY;
        break;
      case PCW_BODY_SIGN:
        pHcr->decInOut.errorLog |= SEGMENT_OVERRIDE_ERR_PCW_BODY_SIGN;
        break;
      case PCW_BODY_SIGN_ESC:
        pHcr->decInOut.errorLog |= SEGMENT_OVERRIDE_ERR_PCW_BODY_SIGN_ESC;
        break;
    }
    /* mark the erred lines */
    for (i = dimension; i != 0; i--) {
      *qsc_base_of_cw++ = (FIXP_DBL)Q_VALUE_INVALID;
    }
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------------------------
     description:   This function checks if all segments are empty after
decoding. There are _no lines markded_ as invalid because it could not be traced
back where from the remaining bits are.
--------------------------------------------------------------------------------------------
*/
static void errDetectWithinSegmentationFinal(H_HCR_INFO pHcr) {
  UCHAR segmentationErrorFlag = 0;
  USHORT i;
  SCHAR *pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  UINT numSegment = pHcr->segmentInfo.numSegment;

  for (i = numSegment; i != 0; i--) {
    if (*pRemainingBitsInSegment++ != 0) {
      segmentationErrorFlag = 1;
    }
  }
  if (segmentationErrorFlag == 1) {
    pHcr->decInOut.errorLog |= BIT_IN_SEGMENTATION_ERROR;
  }
}

/*---------------------------------------------------------------------------------------------
     description:   This function walks one step within the decoding tree. Which
branch is taken depends on the decoded carryBit input parameter.
--------------------------------------------------------------------------------------------
*/
void CarryBitToBranchValue(UCHAR carryBit, UINT treeNode, UINT *branchValue,
                           UINT *branchNode) {
  if (carryBit == 0) {
    *branchNode =
        (treeNode & MASK_LEFT) >> LEFT_OFFSET; /* MASK_LEFT:  00FFF000 */
  } else {
    *branchNode = treeNode & MASK_RIGHT; /* MASK_RIGHT: 00000FFF */
  }

  *branchValue = *branchNode & CLR_BIT_10; /* clear bit 10 (if set) */
}

/*---------------------------------------------------------------------------------------------
     description:   Decodes the body of a priority codeword (PCW)
-----------------------------------------------------------------------------------------------
        return:   - return value is pointer to first of two or four quantized
spectral coefficients
--------------------------------------------------------------------------------------------
*/
static const SCHAR *DecodePCW_Body(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                                   const UINT *pCurrentTree,
                                   const SCHAR *pQuantValBase,
                                   INT *pLeftStartOfSegment,
                                   SCHAR *pRemainingBitsInSegment,
                                   int *pNumDecodedBits) {
  UCHAR carryBit;
  UINT branchNode;
  UINT treeNode;
  UINT branchValue;
  const SCHAR *pQuantVal;

  /* decode PCW_BODY */
  treeNode = *pCurrentTree; /* get first node of current tree belonging to
                               current codebook */

  /* decode whole PCW-codeword-body */
  while (1) {
    carryBit = HcrGetABitFromBitstream(bs, bsAnchor, pLeftStartOfSegment,
                                       pLeftStartOfSegment, /* dummy */
                                       FROM_LEFT_TO_RIGHT);
    *pRemainingBitsInSegment -= 1;
    *pNumDecodedBits += 1;

    CarryBitToBranchValue(carryBit, treeNode, &branchValue, &branchNode);

    if ((branchNode & TEST_BIT_10) ==
        TEST_BIT_10) { /* test bit 10 ; if set --> codeword-body is complete */
      break; /* end of branch in tree reached  i.e. a whole PCW-Body is decoded
              */
    } else {
      treeNode = *(
          pCurrentTree +
          branchValue); /* update treeNode for further step in decoding tree */
    }
  }

  pQuantVal =
      pQuantValBase + branchValue; /* update pointer to valid first of 2 or 4
                                      quantized values */

  return pQuantVal;
}

/*---------------------------------------------------------------------------------------------
     description:   This function decodes one escape sequence. In case of a
escape codebook and in case of the absolute value of the quantized spectral
value == 16, a escapeSequence is decoded in two steps:
                      1. escape prefix
                      2. escape word
--------------------------------------------------------------------------------------------
*/

static INT DecodeEscapeSequence(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                                INT quantSpecCoef, INT *pLeftStartOfSegment,
                                SCHAR *pRemainingBitsInSegment,
                                int *pNumDecodedBits, UINT *errorWord) {
  UINT i;
  INT sign;
  UINT escapeOnesCounter = 0;
  UINT carryBit;
  INT escape_word = 0;

  /* decode escape prefix */
  while (1) {
    carryBit = HcrGetABitFromBitstream(bs, bsAnchor, pLeftStartOfSegment,
                                       pLeftStartOfSegment, /* dummy */
                                       FROM_LEFT_TO_RIGHT);
    *pRemainingBitsInSegment -= 1;
    *pNumDecodedBits += 1;
    if (*pRemainingBitsInSegment < 0) {
      return Q_VALUE_INVALID;
    }

    if (carryBit != 0) {
      escapeOnesCounter += 1;
    } else {
      escapeOnesCounter += 4;
      break;
    }
  }

  /* decode escape word */
  for (i = escapeOnesCounter; i != 0; i--) {
    carryBit = HcrGetABitFromBitstream(bs, bsAnchor, pLeftStartOfSegment,
                                       pLeftStartOfSegment, /* dummy */
                                       FROM_LEFT_TO_RIGHT);
    *pRemainingBitsInSegment -= 1;
    *pNumDecodedBits += 1;
    if (*pRemainingBitsInSegment < 0) {
      return Q_VALUE_INVALID;
    }

    escape_word <<= 1;
    escape_word = escape_word | carryBit;
  }

  sign = (quantSpecCoef >= 0) ? 1 : -1;

  if (escapeOnesCounter < 13) {
    quantSpecCoef = sign * (((INT)1 << escapeOnesCounter) + escape_word);
  } else {
    *errorWord |= TOO_MANY_PCW_BODY_SIGN_ESC_BITS_DECODED;
    quantSpecCoef = Q_VALUE_INVALID;
  }
  return quantSpecCoef;
}

/*---------------------------------------------------------------------------------------------
     description:   Decodes the Signbits of a priority codeword (PCW) and writes
out the resulting quantized spectral values into unsorted sections
-----------------------------------------------------------------------------------------------
        output:   - two or four lines at position in corresponding section
(which are not located at the desired position, i.e. they must be reordered in
the last of eight function of HCR)
-----------------------------------------------------------------------------------------------
        return:   - updated pQuantSpecCoef pointer (to next empty storage for a
line)
--------------------------------------------------------------------------------------------
*/
static int DecodePCW_Sign(HANDLE_FDK_BITSTREAM bs, const INT bsAnchor,
                          UINT codebookDim, const SCHAR *pQuantVal,
                          FIXP_DBL *pQuantSpecCoef, int *quantSpecCoefIdx,
                          INT *pLeftStartOfSegment,
                          SCHAR *pRemainingBitsInSegment,
                          int *pNumDecodedBits) {
  UINT i;
  UINT carryBit;
  INT quantSpecCoef;

  for (i = codebookDim; i != 0; i--) {
    quantSpecCoef = *pQuantVal++;
    if (quantSpecCoef != 0) {
      carryBit = HcrGetABitFromBitstream(bs, bsAnchor, pLeftStartOfSegment,
                                         pLeftStartOfSegment, /* dummy */
                                         FROM_LEFT_TO_RIGHT);
      *pRemainingBitsInSegment -= 1;
      *pNumDecodedBits += 1;
      if (*pRemainingBitsInSegment < 0 || *pNumDecodedBits >= (1024 >> 1)) {
        return -1;
      }

      /* adapt sign of values according to the decoded sign bit */
      if (carryBit != 0) {
        pQuantSpecCoef[*quantSpecCoefIdx] = -(FIXP_DBL)quantSpecCoef;
      } else {
        pQuantSpecCoef[*quantSpecCoefIdx] = (FIXP_DBL)quantSpecCoef;
      }
    } else {
      pQuantSpecCoef[*quantSpecCoefIdx] = FL2FXCONST_DBL(0.0f);
    }
    *quantSpecCoefIdx += 1;
    if (*quantSpecCoefIdx >= 1024) {
      return -1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------------------------
     description:   Mutes spectral lines which have been marked as erroneous
(Q_VALUE_INVALID)
--------------------------------------------------------------------------------------------
*/
void HcrMuteErroneousLines(H_HCR_INFO hHcr) {
  int c;
  FIXP_DBL *RESTRICT pLong =
      SPEC_LONG(hHcr->decInOut.pQuantizedSpectralCoefficientsBase);

  /* if there is a line with value Q_VALUE_INVALID mute it */
  for (c = 0; c < 1024; c++) {
    if (pLong[c] == (FIXP_DBL)Q_VALUE_INVALID) {
      pLong[c] = FL2FXCONST_DBL(0.0f); /* muting */
    }
  }
}
