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

/**************************** AAC decoder library ******************************

   Author(s):   Robert Weidner (DSP Solutions)

   Description: HCR Decoder: Prepare decoding of non-PCWs, segmentation- and
                bitfield-handling, HCR-Statemachine

*******************************************************************************/

#include "aacdec_hcrs.h"

#include "aacdec_hcr.h"

#include "aacdec_hcr_bit.h"
#include "aac_rom.h"
#include "aac_ram.h"

static UINT InitSegmentBitfield(UINT *pNumSegment,
                                SCHAR *pRemainingBitsInSegment,
                                UINT *pSegmentBitfield,
                                UCHAR *pNumWordForBitfield,
                                USHORT *pNumBitValidInLastWord);

static void InitNonPCWSideInformationForCurrentSet(H_HCR_INFO pHcr);

static INT ModuloValue(INT input, INT bufferlength);

static void ClearBitFromBitfield(STATEFUNC *ptrState, UINT offset,
                                 UINT *pBitfield);

/*---------------------------------------------------------------------------------------------
     description: This function decodes all non-priority codewords (non-PCWs) by
using a state-machine.
--------------------------------------------------------------------------------------------
*/
void DecodeNonPCWs(HANDLE_FDK_BITSTREAM bs, H_HCR_INFO pHcr) {
  UINT numValidSegment;
  INT segmentOffset;
  INT codewordOffsetBase;
  INT codewordOffset;
  UINT trial;

  UINT *pNumSegment;
  SCHAR *pRemainingBitsInSegment;
  UINT *pSegmentBitfield;
  UCHAR *pNumWordForBitfield;
  USHORT *pNumBitValidInLastWord;
  UINT *pCodewordBitfield;
  INT bitfieldWord;
  INT bitInWord;
  UINT tempWord;
  UINT interMediateWord;
  INT tempBit;
  INT carry;

  UINT numCodeword;
  UCHAR numSet;
  UCHAR currentSet;
  UINT codewordInSet;
  UINT remainingCodewordsInSet;
  SCHAR *pSta;
  UINT ret;

  pNumSegment = &(pHcr->segmentInfo.numSegment);
  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pNumWordForBitfield = &(pHcr->segmentInfo.numWordForBitfield);
  pNumBitValidInLastWord = &(pHcr->segmentInfo.pNumBitValidInLastWord);
  pSta = pHcr->nonPcwSideinfo.pSta;

  numValidSegment = InitSegmentBitfield(pNumSegment, pRemainingBitsInSegment,
                                        pSegmentBitfield, pNumWordForBitfield,
                                        pNumBitValidInLastWord);

  if (numValidSegment != 0) {
    numCodeword = pHcr->sectionInfo.numCodeword;
    numSet = ((numCodeword - 1) / *pNumSegment) + 1;

    pHcr->segmentInfo.readDirection = FROM_RIGHT_TO_LEFT;

    /* Process sets subsequently */
    numSet = fMin(numSet, (UCHAR)MAX_HCR_SETS);
    for (currentSet = 1; currentSet < numSet; currentSet++) {

      /* step 1 */
      numCodeword -=
          *pNumSegment; /* number of remaining non PCWs [for all sets] */
      if (numCodeword < *pNumSegment) {
        codewordInSet = numCodeword; /* for last set */
      } else {
        codewordInSet = *pNumSegment; /* for all sets except last set */
      }

      /* step 2 */
      /* prepare array 'CodewordBitfield'; as much ones are written from left in
       * all words, as much decodedCodewordInSetCounter nonPCWs exist in this
       * set */
      tempWord = 0xFFFFFFFF;
      pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;

      for (bitfieldWord = *pNumWordForBitfield; bitfieldWord != 0;
           bitfieldWord--) { /* loop over all used words */
        if (codewordInSet > NUMBER_OF_BIT_IN_WORD) { /* more codewords than
                                                        number of bits => fill
                                                        ones */
          /* fill a whole word with ones */
          *pCodewordBitfield++ = tempWord;
          codewordInSet -= NUMBER_OF_BIT_IN_WORD; /* subtract number of bits */
        } else {
          /* prepare last tempWord */
          for (remainingCodewordsInSet = codewordInSet;
               remainingCodewordsInSet < NUMBER_OF_BIT_IN_WORD;
               remainingCodewordsInSet++) {
            tempWord =
                tempWord &
                ~(1
                  << (NUMBER_OF_BIT_IN_WORD - 1 -
                      remainingCodewordsInSet)); /* set a zero at bit number
                                                    (NUMBER_OF_BIT_IN_WORD-1-i)
                                                    in tempWord */
          }
          *pCodewordBitfield++ = tempWord;
          tempWord = 0x00000000;
        }
      }
      pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;

      /* step 3 */
      /* build non-PCW sideinfo for each non-PCW of the current set */
      InitNonPCWSideInformationForCurrentSet(pHcr);

      /* step 4 */
      /* decode all non-PCWs belonging to this set */

      /* loop over trials */
      codewordOffsetBase = 0;
      for (trial = *pNumSegment; trial > 0; trial--) {
        /* loop over number of words in bitfields */
        segmentOffset = 0; /* start at zero in every segment */
        pHcr->segmentInfo.segmentOffset =
            segmentOffset; /* store in structure for states */
        codewordOffset = codewordOffsetBase;
        pHcr->nonPcwSideinfo.codewordOffset =
            codewordOffset; /* store in structure for states */

        for (bitfieldWord = 0; bitfieldWord < *pNumWordForBitfield;
             bitfieldWord++) {
          /* derive tempWord with bitwise and */
          tempWord =
              pSegmentBitfield[bitfieldWord] & pCodewordBitfield[bitfieldWord];

          /* if tempWord is not zero, decode something */
          if (tempWord != 0) {
            /* loop over all bits in tempWord; start state machine if & is true
             */
            for (bitInWord = NUMBER_OF_BIT_IN_WORD; bitInWord > 0;
                 bitInWord--) {
              interMediateWord = ((UINT)1 << (bitInWord - 1));
              if ((tempWord & interMediateWord) == interMediateWord) {
                /* get state and start state machine */
                pHcr->nonPcwSideinfo.pState =
                    aStateConstant2State[pSta[codewordOffset]];

                while (pHcr->nonPcwSideinfo.pState) {
                  ret = ((STATEFUNC)pHcr->nonPcwSideinfo.pState)(bs, pHcr);
                  if (ret != 0) {
                    return;
                  }
                }
              }

              /* update both offsets */
              segmentOffset += 1; /* add NUMBER_OF_BIT_IN_WORD times one */
              pHcr->segmentInfo.segmentOffset = segmentOffset;
              codewordOffset += 1; /* add NUMBER_OF_BIT_IN_WORD times one */
              codewordOffset =
                  ModuloValue(codewordOffset,
                              *pNumSegment); /* index of the current codeword
                                                lies within modulo range */
              pHcr->nonPcwSideinfo.codewordOffset = codewordOffset;
            }
          } else {
            segmentOffset +=
                NUMBER_OF_BIT_IN_WORD; /* add NUMBER_OF_BIT_IN_WORD at once */
            pHcr->segmentInfo.segmentOffset = segmentOffset;
            codewordOffset +=
                NUMBER_OF_BIT_IN_WORD; /* add NUMBER_OF_BIT_IN_WORD at once */
            codewordOffset = ModuloValue(
                codewordOffset,
                *pNumSegment); /* index of the current codeword lies within
                                  modulo range */
            pHcr->nonPcwSideinfo.codewordOffset = codewordOffset;
          }
        } /* end of bitfield word loop */

        /* decrement codeword - pointer */
        codewordOffsetBase -= 1;
        codewordOffsetBase =
            ModuloValue(codewordOffsetBase, *pNumSegment); /* index of the
                                                              current codeword
                                                              base lies within
                                                              modulo range */

        /* rotate numSegment bits in codewordBitfield */
        /* rotation of *numSegment bits in bitfield of codewords
         * (circle-rotation) */
        /* get last valid bit */
        tempBit = pCodewordBitfield[*pNumWordForBitfield - 1] &
                  (1 << (NUMBER_OF_BIT_IN_WORD - *pNumBitValidInLastWord));
        tempBit = tempBit >> (NUMBER_OF_BIT_IN_WORD - *pNumBitValidInLastWord);

        /* write zero into place where tempBit was fetched from */
        pCodewordBitfield[*pNumWordForBitfield - 1] =
            pCodewordBitfield[*pNumWordForBitfield - 1] &
            ~(1 << (NUMBER_OF_BIT_IN_WORD - *pNumBitValidInLastWord));

        /* rotate last valid word */
        pCodewordBitfield[*pNumWordForBitfield - 1] =
            pCodewordBitfield[*pNumWordForBitfield - 1] >> 1;

        /* transfare carry bit 0 from current word into bitposition 31 from next
         * word and rotate current word */
        for (bitfieldWord = *pNumWordForBitfield - 2; bitfieldWord > -1;
             bitfieldWord--) {
          /* get carry (=bit at position 0) from current word */
          carry = pCodewordBitfield[bitfieldWord] & 1;

          /* put the carry bit at position 31 into word right from current word
           */
          pCodewordBitfield[bitfieldWord + 1] =
              pCodewordBitfield[bitfieldWord + 1] |
              (carry << (NUMBER_OF_BIT_IN_WORD - 1));

          /* shift current word */
          pCodewordBitfield[bitfieldWord] =
              pCodewordBitfield[bitfieldWord] >> 1;
        }

        /* put tempBit into free bit-position 31 from first word */
        pCodewordBitfield[0] =
            pCodewordBitfield[0] | (tempBit << (NUMBER_OF_BIT_IN_WORD - 1));

      } /* end of trial loop */

      /* toggle read direction */
      pHcr->segmentInfo.readDirection =
          ToggleReadDirection(pHcr->segmentInfo.readDirection);
    }
    /* end of set loop */

    /* all non-PCWs of this spectrum are decoded */
  }

  /* all PCWs and all non PCWs are decoded. They are unbacksorted in output
   * buffer. Here is the Interface with comparing QSCs to asm decoding */
}

/*---------------------------------------------------------------------------------------------
     description:   This function prepares the bitfield used for the
                    segments. The list is set up once to be used in all
following sets. If a segment is decoded empty, the according bit from the
Bitfield is removed.
-----------------------------------------------------------------------------------------------
        return:     numValidSegment = the number of valid segments
--------------------------------------------------------------------------------------------
*/
static UINT InitSegmentBitfield(UINT *pNumSegment,
                                SCHAR *pRemainingBitsInSegment,
                                UINT *pSegmentBitfield,
                                UCHAR *pNumWordForBitfield,
                                USHORT *pNumBitValidInLastWord) {
  SHORT i;
  USHORT r;
  UCHAR bitfieldWord;
  UINT tempWord;
  USHORT numValidSegment;

  *pNumWordForBitfield =
      (*pNumSegment == 0)
          ? 0
          : ((*pNumSegment - 1) >> THIRTYTWO_LOG_DIV_TWO_LOG) + 1;

  /* loop over all words, which are completely used or only partial */
  /* bit in pSegmentBitfield is zero if segment is empty; bit in
   * pSegmentBitfield is one if segment is not empty */
  numValidSegment = 0;
  *pNumBitValidInLastWord = *pNumSegment;

  /* loop over words */
  for (bitfieldWord = 0; bitfieldWord < *pNumWordForBitfield - 1;
       bitfieldWord++) {
    tempWord = 0xFFFFFFFF; /* set ones */
    r = bitfieldWord << THIRTYTWO_LOG_DIV_TWO_LOG;
    for (i = 0; i < NUMBER_OF_BIT_IN_WORD; i++) {
      if (pRemainingBitsInSegment[r + i] == 0) {
        tempWord = tempWord & ~(1 << (NUMBER_OF_BIT_IN_WORD - 1 -
                                      i)); /* set a zero at bit number
                                              (NUMBER_OF_BIT_IN_WORD-1-i) in
                                              tempWord */
      } else {
        numValidSegment += 1; /* count segments which are not empty */
      }
    }
    pSegmentBitfield[bitfieldWord] = tempWord;        /* store result */
    *pNumBitValidInLastWord -= NUMBER_OF_BIT_IN_WORD; /* calculate number of
                                                         zeros on LSB side in
                                                         the last word */
  }

  /* calculate last word: prepare special tempWord */
  tempWord = 0xFFFFFFFF;
  for (i = 0; i < (NUMBER_OF_BIT_IN_WORD - *pNumBitValidInLastWord); i++) {
    tempWord = tempWord & ~(1 << i); /* clear bit i in tempWord */
  }

  /* calculate last word */
  r = bitfieldWord << THIRTYTWO_LOG_DIV_TWO_LOG;
  for (i = 0; i < *pNumBitValidInLastWord; i++) {
    if (pRemainingBitsInSegment[r + i] == 0) {
      tempWord = tempWord & ~(1 << (NUMBER_OF_BIT_IN_WORD - 1 -
                                    i)); /* set a zero at bit number
                                            (NUMBER_OF_BIT_IN_WORD-1-i) in
                                            tempWord */
    } else {
      numValidSegment += 1; /* count segments which are not empty */
    }
  }
  pSegmentBitfield[bitfieldWord] = tempWord; /* store result */

  return numValidSegment;
}

/*---------------------------------------------------------------------------------------------
  description:  This function sets up sideinfo for the non-PCW decoder (for the
current set).
---------------------------------------------------------------------------------------------*/
static void InitNonPCWSideInformationForCurrentSet(H_HCR_INFO pHcr) {
  USHORT i, k;
  UCHAR codebookDim;
  UINT startNode;

  UCHAR *pCodebook = pHcr->nonPcwSideinfo.pCodebook;
  UINT *iNode = pHcr->nonPcwSideinfo.iNode;
  UCHAR *pCntSign = pHcr->nonPcwSideinfo.pCntSign;
  USHORT *iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  UINT *pEscapeSequenceInfo = pHcr->nonPcwSideinfo.pEscapeSequenceInfo;
  SCHAR *pSta = pHcr->nonPcwSideinfo.pSta;
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
  int quantizedSpectralCoefficientsIdx =
      pHcr->decInOut.quantizedSpectralCoefficientsIdx;
  const UCHAR *pCbDimension = aDimCb;
  int iterationCounter = 0;

  /* loop over number of extended sorted sections in the current set so all
   * codewords sideinfo variables within this set can be prepared for decoding
   */
  for (i = pNumExtendedSortedSectionsInSets[numExtendedSortedSectionsInSetsIdx];
       i != 0; i--) {
    codebookDim =
        pCbDimension[pExtendedSortedCodebook[extendedSortedCodebookIdx]];
    startNode = *aHuffTable[pExtendedSortedCodebook[extendedSortedCodebookIdx]];

    for (k = pNumExtendedSortedCodewordInSection
             [numExtendedSortedCodewordInSectionIdx];
         k != 0; k--) {
      iterationCounter++;
      if (iterationCounter > (1024 >> 2)) {
        return;
      }
      *pSta++ = aCodebook2StartInt
          [pExtendedSortedCodebook[extendedSortedCodebookIdx]];
      *pCodebook++ = pExtendedSortedCodebook[extendedSortedCodebookIdx];
      *iNode++ = startNode;
      *pCntSign++ = 0;
      *iResultPointer++ = quantizedSpectralCoefficientsIdx;
      *pEscapeSequenceInfo++ = 0;
      quantizedSpectralCoefficientsIdx +=
          codebookDim; /* update pointer by codebookDim --> point to next
                          starting value for writing out */
      if (quantizedSpectralCoefficientsIdx >= 1024) {
        return;
      }
    }
    numExtendedSortedCodewordInSectionIdx++; /* inc ptr for next ext sort sec in
                                                current set */
    extendedSortedCodebookIdx++; /* inc ptr for next ext sort sec in current set
                                  */
    if (numExtendedSortedCodewordInSectionIdx >= (MAX_SFB_HCR + MAX_HCR_SETS) ||
        extendedSortedCodebookIdx >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
      return;
    }
  }
  numExtendedSortedSectionsInSetsIdx++; /* inc ptr for next set of non-PCWs */
  if (numExtendedSortedCodewordInSectionIdx >= (MAX_SFB_HCR + MAX_HCR_SETS)) {
    return;
  }

  /* Write back indexes */
  pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx =
      numExtendedSortedCodewordInSectionIdx;
  pHcr->sectionInfo.extendedSortedCodebookIdx = extendedSortedCodebookIdx;
  pHcr->sectionInfo.numExtendedSortedSectionsInSetsIdx =
      numExtendedSortedSectionsInSetsIdx;
  pHcr->sectionInfo.numExtendedSortedCodewordInSectionIdx =
      numExtendedSortedCodewordInSectionIdx;
  pHcr->decInOut.quantizedSpectralCoefficientsIdx =
      quantizedSpectralCoefficientsIdx;
}

/*---------------------------------------------------------------------------------------------
     description: This function returns the input value if the value is in the
                  range of bufferlength. If <input> is smaller, one bufferlength
is added, if <input> is bigger one bufferlength is subtracted.
-----------------------------------------------------------------------------------------------
        return:   modulo result
--------------------------------------------------------------------------------------------
*/
static INT ModuloValue(INT input, INT bufferlength) {
  if (input > (bufferlength - 1)) {
    return (input - bufferlength);
  }
  if (input < 0) {
    return (input + bufferlength);
  }
  return input;
}

/*---------------------------------------------------------------------------------------------
     description: This function clears a bit from current bitfield and
                  switches off the statemachine.

                  A bit is cleared in two cases:
                  a) a codeword is decoded, then a bit is cleared in codeword
bitfield b) a segment is decoded empty, then a bit is cleared in segment
bitfield
--------------------------------------------------------------------------------------------
*/
static void ClearBitFromBitfield(STATEFUNC *ptrState, UINT offset,
                                 UINT *pBitfield) {
  UINT numBitfieldWord;
  UINT numBitfieldBit;

  /* get both values needed for clearing the bit */
  numBitfieldWord = offset >> THIRTYTWO_LOG_DIV_TWO_LOG; /* int   = wordNr */
  numBitfieldBit = offset - (numBitfieldWord
                             << THIRTYTWO_LOG_DIV_TWO_LOG); /* fract = bitNr  */

  /* clear a bit in bitfield */
  pBitfield[numBitfieldWord] =
      pBitfield[numBitfieldWord] &
      ~(1 << (NUMBER_OF_BIT_IN_WORD - 1 - numBitfieldBit));

  /* switch off state machine because codeword is decoded and/or because segment
   * is empty */
  *ptrState = NULL;
}

/* =========================================================================================
                              the states of the statemachine
   =========================================================================================
 */

/*---------------------------------------------------------------------------------------------
     description:  Decodes the body of a codeword. This State is used for
codebooks 1,2,5 and 6. No sign bits are decoded, because the table of the
quantized spectral values has got a valid sign at the quantized spectral lines.
-----------------------------------------------------------------------------------------------
        output:   Two or four quantizes spectral values written at position
where pResultPointr points to
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_ONLY(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;
  FIXP_DBL *pResultBase;
  UINT *iNode;
  USHORT *iResultPointer;
  UINT codewordOffset;
  UINT branchNode;
  UINT branchValue;
  UINT iQSC;
  UINT treeNode;
  UCHAR carryBit;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  SCHAR *pRemainingBitsInSegment;
  UCHAR readDirection;
  UCHAR *pCodebook;
  UCHAR dimCntr;
  const UINT *pCurrentTree;
  const UCHAR *pCbDimension;
  const SCHAR *pQuantVal;
  const SCHAR *pQuantValBase;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  pCodebook = pHcr->nonPcwSideinfo.pCodebook;
  iNode = pHcr->nonPcwSideinfo.iNode;
  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;

  pCbDimension = aDimCb;

  treeNode = iNode[codewordOffset];
  pCurrentTree = aHuffTable[pCodebook[codewordOffset]];

  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    CarryBitToBranchValue(carryBit, /* make a step in decoding tree */
                          treeNode, &branchValue, &branchNode);

    /* if end of branch reached write out lines and count bits needed for sign,
     * otherwise store node in codeword sideinfo */
    if ((branchNode & TEST_BIT_10) ==
        TEST_BIT_10) { /* test bit 10 ; ==> body is complete */
      pQuantValBase = aQuantTable[pCodebook[codewordOffset]]; /* get base
                                                                 address of
                                                                 quantized
                                                                 values
                                                                 belonging to
                                                                 current
                                                                 codebook */
      pQuantVal = pQuantValBase + branchValue; /* set pointer to first valid
                                                  line [of 2 or 4 quantized
                                                  values] */

      iQSC = iResultPointer[codewordOffset]; /* get position of first line for
                                                writing out result */

      for (dimCntr = pCbDimension[pCodebook[codewordOffset]]; dimCntr != 0;
           dimCntr--) {
        pResultBase[iQSC++] =
            (FIXP_DBL)*pQuantVal++; /* write out 2 or 4 lines into
                                       spectrum; no Sign bits
                                       available in this state */
      }

      ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                           pCodewordBitfield); /* clear a bit in bitfield and
                                                  switch off statemachine */
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */
      break; /* end of branch in tree reached  i.e. a whole nonPCW-Body is
                decoded */
    } else { /* body is not decoded completely: */
      treeNode = *(
          pCurrentTree +
          branchValue); /* update treeNode for further step in decoding tree */
    }
  }
  iNode[codewordOffset] = treeNode; /* store updated treeNode because maybe
                                       decoding of codeword body not finished
                                       yet */

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_ONLY;
      return BODY_ONLY;
    }
  }

  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: Decodes the codeword body, writes out result and counts the
number of quantized spectral values, which are different form zero. For those
values sign bits are needed.

                  If sign bit counter cntSign is different from zero, switch to
next state to decode sign Bits there. If sign bit counter cntSign is zero, no
sign bits are needed and codeword is decoded.
-----------------------------------------------------------------------------------------------
        output:   Two or four written quantizes spectral values written at
position where pResultPointr points to. The signs of those lines may be wrong.
If the signs [on just one signle sign] is wrong, the next state will correct it.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN__BODY(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;

  UCHAR *pCodebook;
  UINT *iNode;
  UCHAR *pCntSign;
  FIXP_DBL *pResultBase;
  USHORT *iResultPointer;
  UINT codewordOffset;

  UINT iQSC;
  UINT cntSign;
  UCHAR dimCntr;
  UCHAR carryBit;
  SCHAR *pSta;
  UINT treeNode;
  UINT branchValue;
  UINT branchNode;
  const UCHAR *pCbDimension;
  const UINT *pCurrentTree;
  const SCHAR *pQuantValBase;
  const SCHAR *pQuantVal;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  pCodebook = pHcr->nonPcwSideinfo.pCodebook;
  iNode = pHcr->nonPcwSideinfo.iNode;
  pCntSign = pHcr->nonPcwSideinfo.pCntSign;
  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;
  pSta = pHcr->nonPcwSideinfo.pSta;

  pCbDimension = aDimCb;

  treeNode = iNode[codewordOffset];
  pCurrentTree = aHuffTable[pCodebook[codewordOffset]];

  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    CarryBitToBranchValue(carryBit, /* make a step in decoding tree */
                          treeNode, &branchValue, &branchNode);

    /* if end of branch reached write out lines and count bits needed for sign,
     * otherwise store node in codeword sideinfo */
    if ((branchNode & TEST_BIT_10) ==
        TEST_BIT_10) { /* test bit 10 ; if set body complete */
      /* body completely decoded; branchValue is valid, set pQuantVal to first
       * (of two or four) quantized spectral coefficients */
      pQuantValBase = aQuantTable[pCodebook[codewordOffset]]; /* get base
                                                                 address of
                                                                 quantized
                                                                 values
                                                                 belonging to
                                                                 current
                                                                 codebook */
      pQuantVal = pQuantValBase + branchValue; /* set pointer to first valid
                                                  line [of 2 or 4 quantized
                                                  values] */

      iQSC = iResultPointer[codewordOffset]; /* get position of first line for
                                                writing result */

      /* codeword decoding result is written out here: Write out 2 or 4
       * quantized spectral values with probably */
      /* wrong sign and count number of values which are different from zero for
       * sign bit decoding [which happens in next state] */
      cntSign = 0;
      for (dimCntr = pCbDimension[pCodebook[codewordOffset]]; dimCntr != 0;
           dimCntr--) {
        pResultBase[iQSC++] =
            (FIXP_DBL)*pQuantVal; /* write quant. spec. coef. into spectrum */
        if (*pQuantVal++ != 0) {
          cntSign += 1;
        }
      }

      if (cntSign == 0) {
        ClearBitFromBitfield(
            &(pHcr->nonPcwSideinfo.pState), segmentOffset,
            pCodewordBitfield); /* clear a bit in bitfield and switch off
                                   statemachine */
      } else {
        pCntSign[codewordOffset] = cntSign;     /* write sign count result into
                                                   codewordsideinfo of current
                                                   codeword */
        pSta[codewordOffset] = BODY_SIGN__SIGN; /* change state */
        pHcr->nonPcwSideinfo.pState =
            aStateConstant2State[pSta[codewordOffset]]; /* get state from
                                                           separate array of
                                                           cw-sideinfo */
      }
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */
      break; /* end of branch in tree reached  i.e. a whole nonPCW-Body is
                decoded */
    } else { /* body is not decoded completely: */
      treeNode = *(
          pCurrentTree +
          branchValue); /* update treeNode for further step in decoding tree */
    }
  }
  iNode[codewordOffset] = treeNode; /* store updated treeNode because maybe
                                       decoding of codeword body not finished
                                       yet */

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN__BODY;
      return BODY_SIGN__BODY;
    }
  }

  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: This state decodes the sign bits belonging to a codeword. The
state is called as often in different "trials" until pCntSgn[codewordOffset] is
zero.
-----------------------------------------------------------------------------------------------
        output:   The two or four quantizes spectral values (written in previous
state) have now the correct sign.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN__SIGN(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;

  UCHAR *pCntSign;
  FIXP_DBL *pResultBase;
  USHORT *iResultPointer;
  UINT codewordOffset;

  UCHAR carryBit;
  UINT iQSC;
  UCHAR cntSign;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  /*pCodebook               = */
  pCntSign = pHcr->nonPcwSideinfo.pCntSign;
  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;

  iQSC = iResultPointer[codewordOffset];
  cntSign = pCntSign[codewordOffset];

  /* loop for sign bit decoding */
  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);
    cntSign -=
        1; /* decrement sign counter because one sign bit has been read */

    /* search for a line (which was decoded in previous state) which is not
     * zero. [This value will get a sign] */
    while (pResultBase[iQSC] == (FIXP_DBL)0) {
      if (++iQSC >= 1024) { /* points to current value different from zero */
        return BODY_SIGN__SIGN;
      }
    }

    /* put sign together with line; if carryBit is zero, the sign is ok already;
     * no write operation necessary in this case */
    if (carryBit != 0) {
      pResultBase[iQSC] = -pResultBase[iQSC]; /* carryBit = 1 --> minus */
    }

    iQSC++; /* update pointer to next (maybe valid) value */

    if (cntSign == 0) { /* if (cntSign==0)  ==>  set state CODEWORD_DECODED */
      ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                           pCodewordBitfield); /* clear a bit in bitfield and
                                                  switch off statemachine */
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */
      break; /* whole nonPCW-Body and according sign bits are decoded */
    }
  }
  pCntSign[codewordOffset] = cntSign;
  iResultPointer[codewordOffset] = iQSC; /* store updated pResultPointer */

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN__SIGN;
      return BODY_SIGN__SIGN;
    }
  }

  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: Decodes the codeword body in case of codebook is 11. Writes
out resulting two or four lines [with probably wrong sign] and counts the number
of lines, which are different form zero. This information is needed in next
                  state where sign bits will be decoded, if necessary.
                  If sign bit counter cntSign is zero, no sign bits are needed
and codeword is decoded completely.
-----------------------------------------------------------------------------------------------
        output:   Two lines (quantizes spectral coefficients) which are probably
wrong. The sign may be wrong and if one or two values is/are 16, the following
states will decode the escape sequence to correct the values which are wirtten
here.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN_ESC__BODY(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;

  UINT *iNode;
  UCHAR *pCntSign;
  FIXP_DBL *pResultBase;
  USHORT *iResultPointer;
  UINT codewordOffset;

  UCHAR carryBit;
  UINT iQSC;
  UINT cntSign;
  UINT dimCntr;
  UINT treeNode;
  SCHAR *pSta;
  UINT branchNode;
  UINT branchValue;
  const UINT *pCurrentTree;
  const SCHAR *pQuantValBase;
  const SCHAR *pQuantVal;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  iNode = pHcr->nonPcwSideinfo.iNode;
  pCntSign = pHcr->nonPcwSideinfo.pCntSign;
  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;
  pSta = pHcr->nonPcwSideinfo.pSta;

  treeNode = iNode[codewordOffset];
  pCurrentTree = aHuffTable[ESCAPE_CODEBOOK];

  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    /* make a step in tree */
    CarryBitToBranchValue(carryBit, treeNode, &branchValue, &branchNode);

    /* if end of branch reached write out lines and count bits needed for sign,
     * otherwise store node in codeword sideinfo */
    if ((branchNode & TEST_BIT_10) ==
        TEST_BIT_10) { /* test bit 10 ; if set body complete */

      /* body completely decoded; branchValue is valid */
      /* set pQuantVol to first (of two or four) quantized spectral coefficients
       */
      pQuantValBase = aQuantTable[ESCAPE_CODEBOOK]; /* get base address of
                                                       quantized values
                                                       belonging to current
                                                       codebook */
      pQuantVal = pQuantValBase + branchValue; /* set pointer to first valid
                                                  line [of 2 or 4 quantized
                                                  values] */

      /* make backup from original resultPointer in node storage for state
       * BODY_SIGN_ESC__SIGN */
      iNode[codewordOffset] = iResultPointer[codewordOffset];

      /* get position of first line for writing result */
      iQSC = iResultPointer[codewordOffset];

      /* codeword decoding result is written out here: Write out 2 or 4
       * quantized spectral values with probably */
      /* wrong sign and count number of values which are different from zero for
       * sign bit decoding [which happens in next state] */
      cntSign = 0;

      for (dimCntr = DIMENSION_OF_ESCAPE_CODEBOOK; dimCntr != 0; dimCntr--) {
        pResultBase[iQSC++] =
            (FIXP_DBL)*pQuantVal; /* write quant. spec. coef. into spectrum */
        if (*pQuantVal++ != 0) {
          cntSign += 1;
        }
      }

      if (cntSign == 0) {
        ClearBitFromBitfield(
            &(pHcr->nonPcwSideinfo.pState), segmentOffset,
            pCodewordBitfield); /* clear a bit in bitfield and switch off
                                   statemachine */
        /* codeword decoded */
      } else {
        /* write sign count result into codewordsideinfo of current codeword */
        pCntSign[codewordOffset] = cntSign;
        pSta[codewordOffset] = BODY_SIGN_ESC__SIGN; /* change state */
        pHcr->nonPcwSideinfo.pState =
            aStateConstant2State[pSta[codewordOffset]]; /* get state from
                                                           separate array of
                                                           cw-sideinfo */
      }
      pRemainingBitsInSegment[segmentOffset] -= 1; /* the last reinitialzation
                                                      of for loop counter (see
                                                      above) is done here */
      break; /* end of branch in tree reached  i.e. a whole nonPCW-Body is
                decoded */
    } else { /* body is not decoded completely: */
      /* update treeNode for further step in decoding tree and store updated
       * treeNode because maybe no more bits left in segment */
      treeNode = *(pCurrentTree + branchValue);
      iNode[codewordOffset] = treeNode;
    }
  }

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN_ESC__BODY;
      return BODY_SIGN_ESC__BODY;
    }
  }

  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: This state decodes the sign bits, if a codeword of codebook 11
needs some. A flag named 'flagB' in codeword sideinfo is set, if the second line
of quantized spectral values is 16. The 'flagB' is used in case of decoding of a
escape sequence is necessary as far as the second line is concerned.

                  If only the first line needs an escape sequence, the flagB is
cleared. If only the second line needs an escape sequence, the flagB is not
used.

                  For storing sideinfo in case of escape sequence decoding one
single word can be used for both escape sequences because they are decoded not
at the same time:


                  bit 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5
4  3  2  1  0
                      ===== == == =========== ===========
=================================== ^      ^  ^         ^            ^
^ |      |  |         |            |                    | res. flagA  flagB
escapePrefixUp  escapePrefixDown  escapeWord

-----------------------------------------------------------------------------------------------
        output:   Two lines with correct sign. If one or two values is/are 16,
the lines are not valid, otherwise they are.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN_ESC__SIGN(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;

  UINT *iNode;
  UCHAR *pCntSign;
  FIXP_DBL *pResultBase;
  USHORT *iResultPointer;
  UINT *pEscapeSequenceInfo;
  UINT codewordOffset;

  UINT iQSC;
  UCHAR cntSign;
  UINT flagA;
  UINT flagB;
  UINT flags;
  UCHAR carryBit;
  SCHAR *pSta;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  iNode = pHcr->nonPcwSideinfo.iNode;
  pCntSign = pHcr->nonPcwSideinfo.pCntSign;
  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  pEscapeSequenceInfo = pHcr->nonPcwSideinfo.pEscapeSequenceInfo;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;
  pSta = pHcr->nonPcwSideinfo.pSta;

  iQSC = iResultPointer[codewordOffset];
  cntSign = pCntSign[codewordOffset];

  /* loop for sign bit decoding */
  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    /* decrement sign counter because one sign bit has been read */
    cntSign -= 1;
    pCntSign[codewordOffset] = cntSign;

    /* get a quantized spectral value (which was decoded in previous state)
     * which is not zero. [This value will get a sign] */
    while (pResultBase[iQSC] == (FIXP_DBL)0) {
      if (++iQSC >= 1024) {
        return BODY_SIGN_ESC__SIGN;
      }
    }
    iResultPointer[codewordOffset] = iQSC;

    /* put negative sign together with quantized spectral value; if carryBit is
     * zero, the sign is ok already; no write operation necessary in this case
     */
    if (carryBit != 0) {
      pResultBase[iQSC] = -pResultBase[iQSC]; /* carryBit = 1 --> minus */
    }
    iQSC++; /* update index to next (maybe valid) value */
    iResultPointer[codewordOffset] = iQSC;

    if (cntSign == 0) {
      /* all sign bits are decoded now */
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */

      /* check decoded values if codeword is decoded: Check if one or two escape
       * sequences 16 follow */

      /* step 0 */
      /* restore pointer to first decoded quantized value [ = original
       * pResultPointr] from index iNode prepared in State_BODY_SIGN_ESC__BODY
       */
      iQSC = iNode[codewordOffset];

      /* step 1 */
      /* test first value if escape sequence follows */
      flagA = 0; /* for first possible escape sequence */
      if (fixp_abs(pResultBase[iQSC++]) == (FIXP_DBL)ESCAPE_VALUE) {
        flagA = 1;
      }

      /* step 2 */
      /* test second value if escape sequence follows */
      flagB = 0; /* for second possible escape sequence */
      if (fixp_abs(pResultBase[iQSC]) == (FIXP_DBL)ESCAPE_VALUE) {
        flagB = 1;
      }

      /* step 3 */
      /* evaluate flag result and go on if necessary */
      if (!flagA && !flagB) {
        ClearBitFromBitfield(
            &(pHcr->nonPcwSideinfo.pState), segmentOffset,
            pCodewordBitfield); /* clear a bit in bitfield and switch off
                                   statemachine */
      } else {
        /* at least one of two lines is 16 */
        /* store both flags at correct positions in non PCW codeword sideinfo
         * pEscapeSequenceInfo[codewordOffset] */
        flags = flagA << POSITION_OF_FLAG_A;
        flags |= (flagB << POSITION_OF_FLAG_B);
        pEscapeSequenceInfo[codewordOffset] = flags;

        /* set next state */
        pSta[codewordOffset] = BODY_SIGN_ESC__ESC_PREFIX;
        pHcr->nonPcwSideinfo.pState =
            aStateConstant2State[pSta[codewordOffset]]; /* get state from
                                                           separate array of
                                                           cw-sideinfo */

        /* set result pointer to the first line of the two decoded lines */
        iResultPointer[codewordOffset] = iNode[codewordOffset];

        if (!flagA && flagB) {
          /* update pResultPointr ==> state Stat_BODY_SIGN_ESC__ESC_WORD writes
           * to correct position. Second value is the one and only escape value
           */
          iQSC = iResultPointer[codewordOffset];
          iQSC++;
          iResultPointer[codewordOffset] = iQSC;
        }

      }      /* at least one of two lines is 16 */
      break; /* nonPCW-Body at cb 11 and according sign bits are decoded */

    } /* if ( cntSign == 0 ) */
  }   /* loop over remaining Bits in segment */

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN_ESC__SIGN;
      return BODY_SIGN_ESC__SIGN;
    }
  }
  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: Decode escape prefix of first or second escape sequence. The
escape prefix consists of ones. The following zero is also decoded here.
-----------------------------------------------------------------------------------------------
        output:   If the single separator-zero which follows the
escape-prefix-ones is not yet decoded: The value 'escapePrefixUp' in word
pEscapeSequenceInfo[codewordOffset] is updated.

                  If the single separator-zero which follows the
escape-prefix-ones is decoded: Two updated values 'escapePrefixUp' and
'escapePrefixDown' in word pEscapeSequenceInfo[codewordOffset]. This State is
finished. Switch to next state.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN_ESC__ESC_PREFIX(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT segmentOffset;
  UINT *pEscapeSequenceInfo;
  UINT codewordOffset;
  UCHAR carryBit;
  UINT escapePrefixUp;
  SCHAR *pSta;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;
  pEscapeSequenceInfo = pHcr->nonPcwSideinfo.pEscapeSequenceInfo;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;
  pSta = pHcr->nonPcwSideinfo.pSta;

  escapePrefixUp =
      (pEscapeSequenceInfo[codewordOffset] & MASK_ESCAPE_PREFIX_UP) >>
      LSB_ESCAPE_PREFIX_UP;

  /* decode escape prefix */
  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    /* count ones and store sum in escapePrefixUp */
    if (carryBit == 1) {
      escapePrefixUp += 1; /* update conter for ones */
      if (escapePrefixUp > 8) {
        pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN_ESC__ESC_PREFIX;
        return BODY_SIGN_ESC__ESC_PREFIX;
      }

      /* store updated counter in sideinfo of current codeword */
      pEscapeSequenceInfo[codewordOffset] &=
          ~MASK_ESCAPE_PREFIX_UP;              /* delete old escapePrefixUp */
      escapePrefixUp <<= LSB_ESCAPE_PREFIX_UP; /* shift to correct position */
      pEscapeSequenceInfo[codewordOffset] |=
          escapePrefixUp;                      /* insert new escapePrefixUp */
      escapePrefixUp >>= LSB_ESCAPE_PREFIX_UP; /* shift back down */
    } else {                                   /* separator [zero] reached */
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */
      escapePrefixUp +=
          4; /* if escape_separator '0' appears, add 4 and ==> break */

      /* store escapePrefixUp in pEscapeSequenceInfo[codewordOffset] at bit
       * position escapePrefixUp */
      pEscapeSequenceInfo[codewordOffset] &=
          ~MASK_ESCAPE_PREFIX_UP;              /* delete old escapePrefixUp */
      escapePrefixUp <<= LSB_ESCAPE_PREFIX_UP; /* shift to correct position */
      pEscapeSequenceInfo[codewordOffset] |=
          escapePrefixUp;                      /* insert new escapePrefixUp */
      escapePrefixUp >>= LSB_ESCAPE_PREFIX_UP; /* shift back down */

      /* store escapePrefixUp in pEscapeSequenceInfo[codewordOffset] at bit
       * position escapePrefixDown */
      pEscapeSequenceInfo[codewordOffset] &=
          ~MASK_ESCAPE_PREFIX_DOWN; /* delete old escapePrefixDown */
      escapePrefixUp <<= LSB_ESCAPE_PREFIX_DOWN; /* shift to correct position */
      pEscapeSequenceInfo[codewordOffset] |=
          escapePrefixUp; /* insert new escapePrefixDown */

      pSta[codewordOffset] = BODY_SIGN_ESC__ESC_WORD; /* set next state */
      pHcr->nonPcwSideinfo.pState =
          aStateConstant2State[pSta[codewordOffset]]; /* get state from separate
                                                         array of cw-sideinfo */
      break;
    }
  }

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN_ESC__ESC_PREFIX;
      return BODY_SIGN_ESC__ESC_PREFIX;
    }
  }

  return STOP_THIS_STATE;
}

/*---------------------------------------------------------------------------------------------
     description: Decode escapeWord of escape sequence. If the escape sequence
is decoded completely, assemble quantized-spectral-escape-coefficient and
replace the previous decoded 16 by the new value. Test flagB. If flagB is set,
the second escape sequence must be decoded. If flagB is not set, the codeword is
decoded and the state machine is switched off.
-----------------------------------------------------------------------------------------------
        output:   Two lines with valid sign. At least one of both lines has got
the correct value.
-----------------------------------------------------------------------------------------------
        return:   0
--------------------------------------------------------------------------------------------
*/
UINT Hcr_State_BODY_SIGN_ESC__ESC_WORD(HANDLE_FDK_BITSTREAM bs, void *ptr) {
  H_HCR_INFO pHcr = (H_HCR_INFO)ptr;
  SCHAR *pRemainingBitsInSegment;
  INT *pLeftStartOfSegment;
  INT *pRightStartOfSegment;
  UCHAR readDirection;
  UINT *pSegmentBitfield;
  UINT *pCodewordBitfield;
  UINT segmentOffset;

  FIXP_DBL *pResultBase;
  USHORT *iResultPointer;
  UINT *pEscapeSequenceInfo;
  UINT codewordOffset;

  UINT escapeWord;
  UINT escapePrefixDown;
  UINT escapePrefixUp;
  UCHAR carryBit;
  UINT iQSC;
  INT sign;
  UINT flagA;
  UINT flagB;
  SCHAR *pSta;

  pRemainingBitsInSegment = pHcr->segmentInfo.pRemainingBitsInSegment;
  pLeftStartOfSegment = pHcr->segmentInfo.pLeftStartOfSegment;
  pRightStartOfSegment = pHcr->segmentInfo.pRightStartOfSegment;
  readDirection = pHcr->segmentInfo.readDirection;
  pSegmentBitfield = pHcr->segmentInfo.pSegmentBitfield;
  pCodewordBitfield = pHcr->segmentInfo.pCodewordBitfield;
  segmentOffset = pHcr->segmentInfo.segmentOffset;

  pResultBase = pHcr->nonPcwSideinfo.pResultBase;
  iResultPointer = pHcr->nonPcwSideinfo.iResultPointer;
  pEscapeSequenceInfo = pHcr->nonPcwSideinfo.pEscapeSequenceInfo;
  codewordOffset = pHcr->nonPcwSideinfo.codewordOffset;
  pSta = pHcr->nonPcwSideinfo.pSta;

  escapeWord = pEscapeSequenceInfo[codewordOffset] & MASK_ESCAPE_WORD;
  escapePrefixDown =
      (pEscapeSequenceInfo[codewordOffset] & MASK_ESCAPE_PREFIX_DOWN) >>
      LSB_ESCAPE_PREFIX_DOWN;

  /* decode escape word */
  for (; pRemainingBitsInSegment[segmentOffset] > 0;
       pRemainingBitsInSegment[segmentOffset] -= 1) {
    carryBit = HcrGetABitFromBitstream(
        bs, pHcr->decInOut.bitstreamAnchor, &pLeftStartOfSegment[segmentOffset],
        &pRightStartOfSegment[segmentOffset], readDirection);

    /* build escape word */
    escapeWord <<=
        1; /* left shift previous decoded part of escapeWord by on bit */
    escapeWord = escapeWord | carryBit; /* assemble escape word by bitwise or */

    /* decrement counter for length of escape word because one more bit was
     * decoded */
    escapePrefixDown -= 1;

    /* store updated escapePrefixDown */
    pEscapeSequenceInfo[codewordOffset] &=
        ~MASK_ESCAPE_PREFIX_DOWN; /* delete old escapePrefixDown */
    escapePrefixDown <<= LSB_ESCAPE_PREFIX_DOWN; /* shift to correct position */
    pEscapeSequenceInfo[codewordOffset] |=
        escapePrefixDown; /* insert new escapePrefixDown */
    escapePrefixDown >>= LSB_ESCAPE_PREFIX_DOWN; /* shift back */

    /* store updated escapeWord */
    pEscapeSequenceInfo[codewordOffset] &=
        ~MASK_ESCAPE_WORD; /* delete old escapeWord */
    pEscapeSequenceInfo[codewordOffset] |=
        escapeWord; /* insert new escapeWord */

    if (escapePrefixDown == 0) {
      pRemainingBitsInSegment[segmentOffset] -= 1; /* last reinitialzation of
                                                      for loop counter (see
                                                      above) is done here */

      /* escape sequence decoded. Assemble escape-line and replace original line
       */

      /* step 0 */
      /* derive sign */
      iQSC = iResultPointer[codewordOffset];
      sign = (pResultBase[iQSC] >= (FIXP_DBL)0)
                 ? 1
                 : -1; /* get sign of escape value 16 */

      /* step 1 */
      /* get escapePrefixUp */
      escapePrefixUp =
          (pEscapeSequenceInfo[codewordOffset] & MASK_ESCAPE_PREFIX_UP) >>
          LSB_ESCAPE_PREFIX_UP;

      /* step 2 */
      /* calculate escape value */
      pResultBase[iQSC] =
          (FIXP_DBL)(sign * (((INT)1 << escapePrefixUp) + (INT)escapeWord));

      /* get both flags from sideinfo (flags are not shifted to the
       * lsb-position) */
      flagA = pEscapeSequenceInfo[codewordOffset] & MASK_FLAG_A;
      flagB = pEscapeSequenceInfo[codewordOffset] & MASK_FLAG_B;

      /* step 3 */
      /* clear the whole escape sideinfo word */
      pEscapeSequenceInfo[codewordOffset] = 0;

      /* change state in dependence of flag flagB */
      if (flagA != 0) {
        /* first escape sequence decoded; previous decoded 16 has been replaced
         * by valid line */

        /* clear flagA in sideinfo word because this escape sequence has already
         * beed decoded */
        pEscapeSequenceInfo[codewordOffset] &= ~MASK_FLAG_A;

        if (flagB == 0) {
          ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                               pCodewordBitfield); /* clear a bit in bitfield
                                                      and switch off
                                                      statemachine */
        } else {
          /* updated pointer to next and last 16 */
          iQSC++;
          iResultPointer[codewordOffset] = iQSC;

          /* change state */
          pSta[codewordOffset] = BODY_SIGN_ESC__ESC_PREFIX;
          pHcr->nonPcwSideinfo.pState =
              aStateConstant2State[pSta[codewordOffset]]; /* get state from
                                                             separate array of
                                                             cw-sideinfo */
        }
      } else {
        ClearBitFromBitfield(
            &(pHcr->nonPcwSideinfo.pState), segmentOffset,
            pCodewordBitfield); /* clear a bit in bitfield and switch off
                                   statemachine */
      }
      break;
    }
  }

  if (pRemainingBitsInSegment[segmentOffset] <= 0) {
    ClearBitFromBitfield(&(pHcr->nonPcwSideinfo.pState), segmentOffset,
                         pSegmentBitfield); /* clear a bit in bitfield and
                                               switch off statemachine */

    if (pRemainingBitsInSegment[segmentOffset] < 0) {
      pHcr->decInOut.errorLog |= STATE_ERROR_BODY_SIGN_ESC__ESC_WORD;
      return BODY_SIGN_ESC__ESC_WORD;
    }
  }

  return STOP_THIS_STATE;
}
