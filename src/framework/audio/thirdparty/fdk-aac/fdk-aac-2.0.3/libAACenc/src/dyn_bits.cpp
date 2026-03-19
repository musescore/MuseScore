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

   Author(s):   M.Werner

   Description: Noiseless coder module

*******************************************************************************/

#include "dyn_bits.h"
#include "bit_cnt.h"
#include "psy_const.h"
#include "aacenc_pns.h"
#include "aacEnc_ram.h"
#include "aacEnc_rom.h"

typedef INT (*lookUpTable)[CODE_BOOK_ESC_NDX + 1];

static INT FDKaacEnc_getSideInfoBits(const SECTION_INFO* const huffsection,
                                     const SHORT* const sideInfoTab,
                                     const INT useHCR) {
  INT sideInfoBits;

  if (useHCR &&
      ((huffsection->codeBook == 11) || (huffsection->codeBook >= 16))) {
    sideInfoBits = 5;
  } else {
    sideInfoBits = sideInfoTab[huffsection->sfbCnt];
  }

  return (sideInfoBits);
}

/* count bits using all possible tables */
static void FDKaacEnc_buildBitLookUp(
    const SHORT* const quantSpectrum, const INT maxSfb,
    const INT* const sfbOffset, const UINT* const sfbMax,
    INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
    SECTION_INFO* const huffsection) {
  INT i, sfbWidth;

  for (i = 0; i < maxSfb; i++) {
    huffsection[i].sfbCnt = 1;
    huffsection[i].sfbStart = i;
    huffsection[i].sectionBits = INVALID_BITCOUNT;
    huffsection[i].codeBook = -1;
    sfbWidth = sfbOffset[i + 1] - sfbOffset[i];
    FDKaacEnc_bitCount(quantSpectrum + sfbOffset[i], sfbWidth, sfbMax[i],
                       bitLookUp[i]);
  }
}

/* essential helper functions */
static inline INT FDKaacEnc_findBestBook(const INT* const bc, INT* const book,
                                         const INT useVCB11) {
  INT minBits = INVALID_BITCOUNT, j;

  int end = CODE_BOOK_ESC_NDX;

  for (j = 0; j <= end; j++) {
    if (bc[j] < minBits) {
      minBits = bc[j];
      *book = j;
    }
  }
  return (minBits);
}

static inline INT FDKaacEnc_findMinMergeBits(const INT* const bc1,
                                             const INT* const bc2,
                                             const INT useVCB11) {
  INT minBits = INVALID_BITCOUNT, j;

  DWORD_ALIGNED(bc1);
  DWORD_ALIGNED(bc2);

  for (j = 0; j <= CODE_BOOK_ESC_NDX; j++) {
    minBits = fixMin(minBits, bc1[j] + bc2[j]);
  }
  return (minBits);
}

static inline void FDKaacEnc_mergeBitLookUp(INT* const RESTRICT bc1,
                                            const INT* const RESTRICT bc2) {
  int j;

  for (j = 0; j <= CODE_BOOK_ESC_NDX; j++) {
    FDK_ASSERT(INVALID_BITCOUNT == 0x1FFFFFFF);
    bc1[j] = fixMin(bc1[j] + bc2[j], INVALID_BITCOUNT);
  }
}

static inline INT FDKaacEnc_findMaxMerge(const INT* const mergeGainLookUp,
                                         const SECTION_INFO* const huffsection,
                                         const INT maxSfb, INT* const maxNdx) {
  INT i, maxMergeGain = 0;
  int lastMaxNdx = 0;

  for (i = 0; i + huffsection[i].sfbCnt < maxSfb; i += huffsection[i].sfbCnt) {
    if (mergeGainLookUp[i] > maxMergeGain) {
      maxMergeGain = mergeGainLookUp[i];
      lastMaxNdx = i;
    }
  }
  *maxNdx = lastMaxNdx;
  return (maxMergeGain);
}

static inline INT FDKaacEnc_CalcMergeGain(
    const SECTION_INFO* const huffsection,
    const INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
    const SHORT* const sideInfoTab, const INT ndx1, const INT ndx2,
    const INT useVCB11) {
  INT MergeGain, MergeBits, SplitBits;

  MergeBits =
      sideInfoTab[huffsection[ndx1].sfbCnt + huffsection[ndx2].sfbCnt] +
      FDKaacEnc_findMinMergeBits(bitLookUp[ndx1], bitLookUp[ndx2], useVCB11);
  SplitBits =
      huffsection[ndx1].sectionBits +
      huffsection[ndx2].sectionBits; /* Bit amount for splitted huffsections */
  MergeGain = SplitBits - MergeBits;

  if ((huffsection[ndx1].codeBook == CODE_BOOK_PNS_NO) ||
      (huffsection[ndx2].codeBook == CODE_BOOK_PNS_NO) ||
      (huffsection[ndx1].codeBook == CODE_BOOK_IS_OUT_OF_PHASE_NO) ||
      (huffsection[ndx2].codeBook == CODE_BOOK_IS_OUT_OF_PHASE_NO) ||
      (huffsection[ndx1].codeBook == CODE_BOOK_IS_IN_PHASE_NO) ||
      (huffsection[ndx2].codeBook == CODE_BOOK_IS_IN_PHASE_NO)) {
    MergeGain = -1;
  }

  return (MergeGain);
}

/* sectioning Stage 0:find minimum codbooks */
static void FDKaacEnc_gmStage0(
    SECTION_INFO* const RESTRICT huffsection,
    const INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1], const INT maxSfb,
    const INT* const noiseNrg, const INT* const isBook) {
  INT i;

  for (i = 0; i < maxSfb; i++) {
    /* Side-Info bits will be calculated in Stage 1! */
    if (huffsection[i].sectionBits == INVALID_BITCOUNT) {
      /* intensity and pns codebooks are already allocated in bitcount.c */
      if (noiseNrg[i] != NO_NOISE_PNS) {
        huffsection[i].codeBook = CODE_BOOK_PNS_NO;
        huffsection[i].sectionBits = 0;
      } else if (isBook[i]) {
        huffsection[i].codeBook = isBook[i];
        huffsection[i].sectionBits = 0;
      } else {
        huffsection[i].sectionBits =
            FDKaacEnc_findBestBook(bitLookUp[i], &(huffsection[i].codeBook),
                                   0); /* useVCB11 must be 0!!! */
      }
    }
  }
}

/*
   sectioning Stage 1:merge all connected regions with the same code book and
   calculate side info
 */
static void FDKaacEnc_gmStage1(
    SECTION_INFO* const RESTRICT huffsection,
    INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1], const INT maxSfb,
    const SHORT* const sideInfoTab, const INT useVCB11) {
  INT mergeStart = 0, mergeEnd;

  do {
    for (mergeEnd = mergeStart + 1; mergeEnd < maxSfb; mergeEnd++) {
      if (huffsection[mergeStart].codeBook != huffsection[mergeEnd].codeBook)
        break;

      /* we can merge. update tables, side info bits will be updated outside of
       * this loop */
      huffsection[mergeStart].sfbCnt++;
      huffsection[mergeStart].sectionBits += huffsection[mergeEnd].sectionBits;

      /* update bit look up for all code books */
      FDKaacEnc_mergeBitLookUp(bitLookUp[mergeStart], bitLookUp[mergeEnd]);
    }

    /* add side info info bits */
    huffsection[mergeStart].sectionBits += FDKaacEnc_getSideInfoBits(
        &huffsection[mergeStart], sideInfoTab, useVCB11);
    huffsection[mergeEnd - 1].sfbStart =
        huffsection[mergeStart].sfbStart; /* speed up prev search */

    mergeStart = mergeEnd;

  } while (mergeStart < maxSfb);
}

/*
   sectioning Stage 2:greedy merge algorithm, merge connected sections with
   maximum bit gain until no more gain is possible
 */
static inline void FDKaacEnc_gmStage2(
    SECTION_INFO* const RESTRICT huffsection,
    INT* const RESTRICT mergeGainLookUp,
    INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1], const INT maxSfb,
    const SHORT* const sideInfoTab, const INT useVCB11) {
  INT i;

  for (i = 0; i + huffsection[i].sfbCnt < maxSfb; i += huffsection[i].sfbCnt) {
    mergeGainLookUp[i] =
        FDKaacEnc_CalcMergeGain(huffsection, bitLookUp, sideInfoTab, i,
                                i + huffsection[i].sfbCnt, useVCB11);
  }

  while (TRUE) {
    INT maxMergeGain, maxNdx, maxNdxNext, maxNdxLast;

    maxMergeGain =
        FDKaacEnc_findMaxMerge(mergeGainLookUp, huffsection, maxSfb, &maxNdx);

    /* exit while loop if no more gain is possible */
    if (maxMergeGain <= 0) break;

    maxNdxNext = maxNdx + huffsection[maxNdx].sfbCnt;

    /* merge sections with maximum bit gain */
    huffsection[maxNdx].sfbCnt += huffsection[maxNdxNext].sfbCnt;
    huffsection[maxNdx].sectionBits +=
        huffsection[maxNdxNext].sectionBits - maxMergeGain;

    /* update bit look up table for merged huffsection  */
    FDKaacEnc_mergeBitLookUp(bitLookUp[maxNdx], bitLookUp[maxNdxNext]);

    /* update mergeLookUpTable */
    if (maxNdx != 0) {
      maxNdxLast = huffsection[maxNdx - 1].sfbStart;
      mergeGainLookUp[maxNdxLast] = FDKaacEnc_CalcMergeGain(
          huffsection, bitLookUp, sideInfoTab, maxNdxLast, maxNdx, useVCB11);
    }
    maxNdxNext = maxNdx + huffsection[maxNdx].sfbCnt;

    huffsection[maxNdxNext - 1].sfbStart = huffsection[maxNdx].sfbStart;

    if (maxNdxNext < maxSfb)
      mergeGainLookUp[maxNdx] = FDKaacEnc_CalcMergeGain(
          huffsection, bitLookUp, sideInfoTab, maxNdx, maxNdxNext, useVCB11);
  }
}

/* count bits used by the noiseless coder */
static void FDKaacEnc_noiselessCounter(
    SECTION_DATA* const RESTRICT sectionData, INT mergeGainLookUp[MAX_SFB_LONG],
    INT bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
    const SHORT* const quantSpectrum, const UINT* const maxValueInSfb,
    const INT* const sfbOffset, const INT blockType, const INT* const noiseNrg,
    const INT* const isBook, const INT useVCB11) {
  INT grpNdx, i;
  const SHORT* sideInfoTab = NULL;
  SECTION_INFO* huffsection;

  /* use appropriate side info table */
  switch (blockType) {
    case LONG_WINDOW:
    case START_WINDOW:
    case STOP_WINDOW:
    default:
      sideInfoTab = FDKaacEnc_sideInfoTabLong;
      break;
    case SHORT_WINDOW:
      sideInfoTab = FDKaacEnc_sideInfoTabShort;
      break;
  }

  sectionData->noOfSections = 0;
  sectionData->huffmanBits = 0;
  sectionData->sideInfoBits = 0;

  if (sectionData->maxSfbPerGroup == 0) return;

  /* loop trough groups */
  for (grpNdx = 0; grpNdx < sectionData->sfbCnt;
       grpNdx += sectionData->sfbPerGroup) {
    huffsection = sectionData->huffsection + sectionData->noOfSections;

    /* count bits in this group */
    FDKaacEnc_buildBitLookUp(quantSpectrum, sectionData->maxSfbPerGroup,
                             sfbOffset + grpNdx, maxValueInSfb + grpNdx,
                             bitLookUp, huffsection);

    /* 0.Stage :Find minimum Codebooks */
    FDKaacEnc_gmStage0(huffsection, bitLookUp, sectionData->maxSfbPerGroup,
                       noiseNrg + grpNdx, isBook + grpNdx);

    /* 1.Stage :Merge all connected regions with the same code book */
    FDKaacEnc_gmStage1(huffsection, bitLookUp, sectionData->maxSfbPerGroup,
                       sideInfoTab, useVCB11);

    /*
       2.Stage
       greedy merge algorithm, merge connected huffsections with maximum bit
       gain until no more gain is possible
     */

    FDKaacEnc_gmStage2(huffsection, mergeGainLookUp, bitLookUp,
                       sectionData->maxSfbPerGroup, sideInfoTab, useVCB11);

    /*
       compress output, calculate total huff and side bits
       since we did not update the actual codebook in stage 2
       to save time, we must set it here for later use in bitenc
     */

    for (i = 0; i < sectionData->maxSfbPerGroup; i += huffsection[i].sfbCnt) {
      if ((huffsection[i].codeBook == CODE_BOOK_PNS_NO) ||
          (huffsection[i].codeBook == CODE_BOOK_IS_OUT_OF_PHASE_NO) ||
          (huffsection[i].codeBook == CODE_BOOK_IS_IN_PHASE_NO)) {
        huffsection[i].sectionBits = 0;
      } else {
        /* the sections in the sectionData are now marked with the optimal code
         * book */

        FDKaacEnc_findBestBook(bitLookUp[i], &(huffsection[i].codeBook),
                               useVCB11);

        sectionData->huffmanBits +=
            huffsection[i].sectionBits -
            FDKaacEnc_getSideInfoBits(&huffsection[i], sideInfoTab, useVCB11);
      }

      huffsection[i].sfbStart += grpNdx;

      /* sum up side info bits (section data bits) */
      sectionData->sideInfoBits +=
          FDKaacEnc_getSideInfoBits(&huffsection[i], sideInfoTab, useVCB11);
      sectionData->huffsection[sectionData->noOfSections++] = huffsection[i];
    }
  }
}

/*******************************************************************************

     functionname: FDKaacEnc_scfCount
     returns     : ---
     description : count bits used by scalefactors.

                   not in all cases if maxValueInSfb[] == 0 we set deltaScf
                   to zero. only if the difference of the last and future
                   scalefacGain is not greater then CODE_BOOK_SCF_LAV (60).

     example:
                  ^
     scalefacGain |
                  |
                  |       last 75
                  |          |
                  |          |
                  |          |
                  |          |      current 50
                  |          |          |
                  |          |          |
                  |          |          |
                  |          |          |
                  |          |          |      future 5
                  |          |          |          |
                  --- ... ---------------------------- ... --------->
                                                                sfb


                  if maxValueInSfb[] of current is zero because of a
                  notfallstrategie, we do not save bits and transmit a
                  deltaScf of 25. otherwise the deltaScf between the last
                  scalfacGain (75) and the future scalefacGain (5) is 70.

********************************************************************************/
static void FDKaacEnc_scfCount(const INT* const scalefacGain,
                               const UINT* const maxValueInSfb,
                               SECTION_DATA* const RESTRICT sectionData,
                               const INT* const isScale) {
  INT i, j, k, m, n;

  INT lastValScf = 0;
  INT deltaScf = 0;
  INT found = 0;
  INT scfSkipCounter = 0;
  INT lastValIs = 0;

  sectionData->scalefacBits = 0;

  if (scalefacGain == NULL) return;

  sectionData->firstScf = 0;

  for (i = 0; i < sectionData->noOfSections; i++) {
    if (sectionData->huffsection[i].codeBook != CODE_BOOK_ZERO_NO) {
      sectionData->firstScf = sectionData->huffsection[i].sfbStart;
      lastValScf = scalefacGain[sectionData->firstScf];
      break;
    }
  }

  for (i = 0; i < sectionData->noOfSections; i++) {
    if ((sectionData->huffsection[i].codeBook ==
         CODE_BOOK_IS_OUT_OF_PHASE_NO) ||
        (sectionData->huffsection[i].codeBook == CODE_BOOK_IS_IN_PHASE_NO)) {
      for (j = sectionData->huffsection[i].sfbStart;
           j < sectionData->huffsection[i].sfbStart +
                   sectionData->huffsection[i].sfbCnt;
           j++) {
        INT deltaIs = isScale[j] - lastValIs;
        lastValIs = isScale[j];
        sectionData->scalefacBits +=
            FDKaacEnc_bitCountScalefactorDelta(deltaIs);
      }
    } /* Intensity */
    else if ((sectionData->huffsection[i].codeBook != CODE_BOOK_ZERO_NO) &&
             (sectionData->huffsection[i].codeBook != CODE_BOOK_PNS_NO)) {
      INT tmp = sectionData->huffsection[i].sfbStart +
                sectionData->huffsection[i].sfbCnt;
      for (j = sectionData->huffsection[i].sfbStart; j < tmp; j++) {
        /* check if we can repeat the last value to save bits */
        if (maxValueInSfb[j] == 0) {
          found = 0;
          /* are scalefactors skipped? */
          if (scfSkipCounter == 0) {
            /* end of section */
            if (j == (tmp - 1))
              found = 0; /* search in other sections for maxValueInSfb != 0 */
            else {
              /* search in this section for the next maxValueInSfb[] != 0 */
              for (k = (j + 1); k < tmp; k++) {
                if (maxValueInSfb[k] != 0) {
                  found = 1;
                  if ((fixp_abs(scalefacGain[k] - lastValScf)) <=
                      CODE_BOOK_SCF_LAV)
                    deltaScf = 0; /* save bits */
                  else {
                    /* do not save bits */
                    deltaScf = lastValScf - scalefacGain[j];
                    lastValScf = scalefacGain[j];
                    scfSkipCounter = 0;
                  }
                  break;
                }
                /* count scalefactor skip */
                scfSkipCounter++;
              }
            }

            /* search for the next maxValueInSfb[] != 0 in all other sections */
            for (m = (i + 1); (m < sectionData->noOfSections) && (found == 0);
                 m++) {
              if ((sectionData->huffsection[m].codeBook != CODE_BOOK_ZERO_NO) &&
                  (sectionData->huffsection[m].codeBook != CODE_BOOK_PNS_NO)) {
                INT end = sectionData->huffsection[m].sfbStart +
                          sectionData->huffsection[m].sfbCnt;
                for (n = sectionData->huffsection[m].sfbStart; n < end; n++) {
                  if (maxValueInSfb[n] != 0) {
                    found = 1;
                    if (fixp_abs(scalefacGain[n] - lastValScf) <=
                        CODE_BOOK_SCF_LAV)
                      deltaScf = 0; /* save bits */
                    else {
                      /* do not save bits */
                      deltaScf = lastValScf - scalefacGain[j];
                      lastValScf = scalefacGain[j];
                      scfSkipCounter = 0;
                    }
                    break;
                  }
                  /* count scalefactor skip */
                  scfSkipCounter++;
                }
              }
            }
            /* no maxValueInSfb[] != 0 found */
            if (found == 0) {
              deltaScf = 0;
              scfSkipCounter = 0;
            }
          } else {
            /* consider skipped scalefactors */
            deltaScf = 0;
            scfSkipCounter--;
          }
        } else {
          deltaScf = lastValScf - scalefacGain[j];
          lastValScf = scalefacGain[j];
        }
        sectionData->scalefacBits +=
            FDKaacEnc_bitCountScalefactorDelta(deltaScf);
      }
    }
  } /* for (i=0; i<sectionData->noOfSections; i++) */
}

/* count bits used by pns */
static void FDKaacEnc_noiseCount(SECTION_DATA* const RESTRICT sectionData,
                                 const INT* const noiseNrg) {
  INT noisePCMFlag = TRUE;
  INT lastValPns = 0, deltaPns;
  int i, j;

  sectionData->noiseNrgBits = 0;

  for (i = 0; i < sectionData->noOfSections; i++) {
    if (sectionData->huffsection[i].codeBook == CODE_BOOK_PNS_NO) {
      int sfbStart = sectionData->huffsection[i].sfbStart;
      int sfbEnd = sfbStart + sectionData->huffsection[i].sfbCnt;
      for (j = sfbStart; j < sfbEnd; j++) {
        if (noisePCMFlag) {
          sectionData->noiseNrgBits += PNS_PCM_BITS;
          lastValPns = noiseNrg[j];
          noisePCMFlag = FALSE;
        } else {
          deltaPns = noiseNrg[j] - lastValPns;
          lastValPns = noiseNrg[j];
          sectionData->noiseNrgBits +=
              FDKaacEnc_bitCountScalefactorDelta(deltaPns);
        }
      }
    }
  }
}

INT FDKaacEnc_dynBitCount(BITCNTR_STATE* const hBC,
                          const SHORT* const quantSpectrum,
                          const UINT* const maxValueInSfb,
                          const INT* const scalefac, const INT blockType,
                          const INT sfbCnt, const INT maxSfbPerGroup,
                          const INT sfbPerGroup, const INT* const sfbOffset,
                          SECTION_DATA* const RESTRICT sectionData,
                          const INT* const noiseNrg, const INT* const isBook,
                          const INT* const isScale, const UINT syntaxFlags) {
  sectionData->blockType = blockType;
  sectionData->sfbCnt = sfbCnt;
  sectionData->sfbPerGroup = sfbPerGroup;
  sectionData->noOfGroups = sfbCnt / sfbPerGroup;
  sectionData->maxSfbPerGroup = maxSfbPerGroup;

  FDKaacEnc_noiselessCounter(sectionData, hBC->mergeGainLookUp,
                             (lookUpTable)hBC->bitLookUp, quantSpectrum,
                             maxValueInSfb, sfbOffset, blockType, noiseNrg,
                             isBook, (syntaxFlags & AC_ER_VCB11) ? 1 : 0);

  FDKaacEnc_scfCount(scalefac, maxValueInSfb, sectionData, isScale);

  FDKaacEnc_noiseCount(sectionData, noiseNrg);

  return (sectionData->huffmanBits + sectionData->sideInfoBits +
          sectionData->scalefacBits + sectionData->noiseNrgBits);
}

INT FDKaacEnc_BCNew(BITCNTR_STATE** phBC, UCHAR* dynamic_RAM) {
  BITCNTR_STATE* hBC = GetRam_aacEnc_BitCntrState();

  if (hBC) {
    *phBC = hBC;
    hBC->bitLookUp = GetRam_aacEnc_BitLookUp(0, dynamic_RAM);
    hBC->mergeGainLookUp = GetRam_aacEnc_MergeGainLookUp(0, dynamic_RAM);
    if (hBC->bitLookUp == 0 || hBC->mergeGainLookUp == 0) {
      return 1;
    }
  }
  return (hBC == 0) ? 1 : 0;
}

void FDKaacEnc_BCClose(BITCNTR_STATE** phBC) {
  if (*phBC != NULL) {
    FreeRam_aacEnc_BitCntrState(phBC);
  }
}
