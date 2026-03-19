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

/**************************** AAC decoder library ******************************

   Author(s):   Robert Weidner (DSP Solutions)

   Description: HCR Decoder: Common defines and structures; defines for
                switching error-generator, -detector, and -concealment;

*******************************************************************************/

#ifndef AACDEC_HCR_TYPES_H
#define AACDEC_HCR_TYPES_H

#include "FDK_bitstream.h"
#include "overlapadd.h"

/* ------------------------------------------------ */
/* ------------------------------------------------ */

#define LINES_PER_UNIT 4

/* ------------------------------------------------ */
/* ------------------------------------------------ */
/* ----------- basic HCR configuration ------------ */

#define MAX_SFB_HCR                                                          \
  (((1024 / 8) / LINES_PER_UNIT) * 8) /* (8 * 16) is not enough because sfbs \
                                         are split in units for blocktype    \
                                         short */
#define NUMBER_OF_UNIT_GROUPS (LINES_PER_UNIT * 8)
#define LINES_PER_UNIT_GROUP (1024 / NUMBER_OF_UNIT_GROUPS) /* 15 16 30 32 */

/* ------------------------------------------------ */
/* ------------------------------------------------ */
/* ------------------------------------------------ */

#define FROM_LEFT_TO_RIGHT 0
#define FROM_RIGHT_TO_LEFT 1

#define MAX_CB_PAIRS 23
#define MAX_HCR_SETS 14

#define ESCAPE_VALUE 16
#define POSITION_OF_FLAG_A 21
#define POSITION_OF_FLAG_B 20

#define MAX_CB 32 /* last used CB is cb #31 when VCB11 is used */

#define MAX_CB_CHECK                                                         \
  32 /* support for VCB11 available -- is more general, could therefore used \
        in both cases */

#define NUMBER_OF_BIT_IN_WORD 32

/* log */
#define THIRTYTWO_LOG_DIV_TWO_LOG 5
#define EIGHT_LOG_DIV_TWO_LOG 3
#define FOUR_LOG_DIV_TWO_LOG 2

/* borders */
#define CPE_TOP_LENGTH 12288
#define SCE_TOP_LENGTH 6144
#define LEN_OF_LONGEST_CW_TOP_LENGTH 49

/* qsc's of high level */
#define Q_VALUE_INVALID \
  8192 /* mark a invalid line with this value (to be concealed later on) */
#define HCR_DIRAC 500 /* a line of high level */

/* masks */
#define MASK_LEFT 0xFFF000
#define MASK_RIGHT 0xFFF
#define CLR_BIT_10 0x3FF
#define TEST_BIT_10 0x400

#define LEFT_OFFSET 12

/* when set HCR is replaced by a dummy-module which just fills the outputbuffer
 * with a dirac sequence */
/* use this if HCR is suspected to write in other modules -- if error is stell
 * there, HCR is innocent */

/* ------------------------------ */
/* -    insert HCR errors       - */
/* ------------------------------ */

/* modify input lengths -- high protected */
#define ERROR_LORSD 0 /* offset: error if different from zero */
#define ERROR_LOLC 0  /* offset: error if different from zero */

/* segments are earlier empty as expected when decoding PCWs */
#define ERROR_PCW_BODY                                                   \
  0 /* set a positive values to trigger the error (make segments earlyer \
       appear to be empty) */
#define ERROR_PCW_BODY_SIGN                                              \
  0 /* set a positive values to trigger the error (make segments earlyer \
       appear to be empty) */
#define ERROR_PCW_BODY_SIGN_ESC                                          \
  0 /* set a positive values to trigger the error (make segments earlyer \
       appear to be empty) */

/* pretend there are too many bits decoded (enlarge length of codeword) at PCWs
 * -- use a positive value */
#define ERROR_PCW_BODY_ONLY_TOO_LONG \
  0 /* set a positive values to trigger the error */
#define ERROR_PCW_BODY_SIGN_TOO_LONG \
  0 /* set a positive values to trigger the error */
#define ERROR_PCW_BODY_SIGN_ESC_TOO_LONG \
  0 /* set a positive values to trigger the error */

/* modify HCR bitstream block */

#define MODULO_DIVISOR_HCR 30

/* ------------------------------ */
/* -    detect HCR errors       - */
/* ------------------------------ */
/* check input data */

/* during decoding */

/* all the segments are checked -- therefore -- if this check passes, its a kind
   of evidence that the decoded PCWs and non-PCWs are fine */

/* if a codeword is decoded there exists a border for the number of bits, which
   are allowed to read for this codeword. This border is the minimum of the
   length of the longest codeword (for the currently used codebook) and the
   separately transmitted 'lengthOfLongestCodeword' in this frame and channel.
   The number of decoded bits is counted (for PCWs only -- there it makes really
   sense in my opinion). If this number exceeds the border (derived as minimum
   -- see above), a error is detected. */

/* -----------------------------------------------------------------------------------------------------
   This error check could be set to zero because due to a test within
   RVLC-Escape-huffman-Decoder a too long codeword could not be detected -- it
   seems that for RVLC-Escape-Codeword the coderoom is used to 100%. Therefore I
   assume that the coderoom is used to 100% also for the codebooks 1..11 used at
   HCR Therefore this test is deactivated pending further notice
   -----------------------------------------------------------------------------------------------------
 */

/* test if the number of remaining bits in a segment is _below_ zero. If there
   are no errors the lowest allowed value for remainingBitsInSegment is zero.
   This check also could be set to zero (save runtime) */

/* other */
/* when set to '1', avoid setting the LAV-Flag in errorLog due to a
   previous-line-marking (at PCW decoder). A little more runtime is needed then
   when writing values out into output-buffer. */

/* ------------------------------ */
/* -    conceal HCR errors      - */
/* ------------------------------ */

#define HCR_ERROR_CONCEALMENT \
  1 /* if set to '1', HCR _mutes_ the erred quantized spectral coefficients */

// ------------------------------------------------------------------------------------------------------------------
//                                         errorLog: A word of 32 bits used for
//                                         logging possible errors within HCR
//                                                   in case of distorted
//                                                   bitstreams. Table of all
//                                                   known errors:
// ------------------------------------------------------------------------------------------------------------------------
// bit  fatal  location    meaning
// ----+-----+-----------+--------------------------------------
#define SEGMENT_OVERRIDE_ERR_PCW_BODY \
  0x80000000  //  31   no    PCW-Dec     During PCW decoding it is checked after
              //  every PCW if there are too many bits decoded (immediate
              //  check).
#define SEGMENT_OVERRIDE_ERR_PCW_BODY_SIGN \
  0x40000000  //  30   no    PCW-Dec     During PCW decoding it is checked after
              //  every PCW if there are too many bits decoded (immediate
              //  check).
#define SEGMENT_OVERRIDE_ERR_PCW_BODY_SIGN_ESC \
  0x20000000  //  29   no    PCW-Dec     During PCW decoding it is checked after
              //  every PCW if there are too many bits decoded (immediate
              //  check).
#define EXTENDED_SORTED_COUNTER_OVERFLOW \
  0x10000000  //  28   yes   Init-Dec    Error during extending sideinfo
              //  (neither a PCW nor a nonPCW was decoded so far)
              // 0x08000000 //  27                     reserved
              // 0x04000000 //  26                     reserved
              // 0x02000000 //  25                     reserved
              // 0x01000000 //  24                     reserved
              // 0x00800000 //  23                     reserved
              // 0x00400000 //  22                     reserved
              // 0x00200000 //  21                     reserved
              // 0x00100000 //  20                     reserved

/* special errors */
#define TOO_MANY_PCW_BODY_BITS_DECODED \
  0x00080000  //  19   yes   PCW-Dec     During PCW-body-decoding too many bits
              //  have been read from bitstream -- advice: skip non-PCW decoding
#define TOO_MANY_PCW_BODY_SIGN_BITS_DECODED \
  0x00040000  //  18   yes   PCW-Dec     During PCW-body-sign-decoding too many
              //  bits have been read from bitstream -- advice: skip non-PCW
              //  decoding
#define TOO_MANY_PCW_BODY_SIGN_ESC_BITS_DECODED \
  0x00020000  //  17   yes   PCW-Dec     During PCW-body-sign-esc-decoding too
              //  many bits have been read from bitstream -- advice: skip
              //  non-PCW decoding

// 0x00010000 //  16                     reserved
#define STATE_ERROR_BODY_ONLY \
  0x00008000  //  15   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN__BODY \
  0x00004000  //  14   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN__SIGN \
  0x00002000  //  13   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN_ESC__BODY \
  0x00001000  //  12   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN_ESC__SIGN \
  0x00000800  //  11   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN_ESC__ESC_PREFIX \
  0x00000400  //  10   no    NonPCW-Dec  State machine returned with error
#define STATE_ERROR_BODY_SIGN_ESC__ESC_WORD \
  0x00000200  //   9   no    NonPCW-Dec  State machine returned with error
#define HCR_SI_LENGTHS_FAILURE \
  0x00000100  //   8   yes   Init-Dec    LengthOfLongestCodeword must not be
              //   less than lenghtOfReorderedSpectralData
#define NUM_SECT_OUT_OF_RANGE_SHORT_BLOCK \
  0x00000080  //   7   yes   Init-Dec    The number of sections is not within
              //   the allowed range (short block)
#define NUM_SECT_OUT_OF_RANGE_LONG_BLOCK \
  0x00000040  //   6   yes   Init-Dec    The number of sections is not within
              //   the allowed range (long block)
#define LINE_IN_SECT_OUT_OF_RANGE_SHORT_BLOCK \
  0x00000020  //   5   yes   Init-Dec    The number of lines per section is not
              //   within the allowed range (short block)
#define CB_OUT_OF_RANGE_SHORT_BLOCK \
  0x00000010  //   4   yes   Init-Dec    The codebook is not within the allowed
              //   range (short block)
#define LINE_IN_SECT_OUT_OF_RANGE_LONG_BLOCK \
  0x00000008  //   3   yes   Init-Dec    The number of lines per section is not
              //   within the allowed range (long block)
#define CB_OUT_OF_RANGE_LONG_BLOCK \
  0x00000004  //   2   yes   Init-Dec    The codebook is not within the allowed
              //   range (long block)
#define LAV_VIOLATION \
  0x00000002  //   1   no    Final       The absolute value of at least one
              //   decoded line was too high for the according codebook.
#define BIT_IN_SEGMENTATION_ERROR \
  0x00000001  //   0   no    Final       After PCW and non-PWC-decoding at least
              //   one segment is not zero (global check).

/*----------*/
#define HCR_FATAL_PCW_ERROR_MASK 0x100E01FC

typedef enum { PCW_BODY, PCW_BODY_SIGN, PCW_BODY_SIGN_ESC } PCW_TYPE;

/* interface Decoder <---> HCR */
typedef struct {
  UINT errorLog;
  SPECTRAL_PTR pQuantizedSpectralCoefficientsBase;
  int quantizedSpectralCoefficientsIdx;
  SHORT lengthOfReorderedSpectralData;
  SHORT numSection;
  SHORT *pNumLineInSect;
  INT bitstreamAnchor;
  SCHAR lengthOfLongestCodeword;
  UCHAR *pCodebook;
} HCR_INPUT_OUTPUT;

typedef struct {
  const UCHAR *pMinOfCbPair;
  const UCHAR *pMaxOfCbPair;
} HCR_CB_PAIRS;

typedef struct {
  const USHORT *pLargestAbsVal;
  const UCHAR *pMaxCwLength;
  const UCHAR *pCbDimension;
  const UCHAR *pCbDimShift;
  const UCHAR *pCbSign;
  const UCHAR *pCbPriority;
} HCR_TABLE_INFO;

typedef struct {
  UINT numSegment;
  UINT pSegmentBitfield[((1024 >> 1) / NUMBER_OF_BIT_IN_WORD + 1)];
  UINT pCodewordBitfield[((1024 >> 1) / NUMBER_OF_BIT_IN_WORD + 1)];
  UINT segmentOffset;
  INT pLeftStartOfSegment[1024 >> 1];
  INT pRightStartOfSegment[1024 >> 1];
  SCHAR pRemainingBitsInSegment[1024 >> 1];
  UCHAR readDirection;
  UCHAR numWordForBitfield;
  USHORT pNumBitValidInLastWord;
} HCR_SEGMENT_INFO;

typedef struct {
  UINT numCodeword;
  UINT numSortedSection;
  USHORT pNumCodewordInSection[MAX_SFB_HCR];
  USHORT pNumSortedCodewordInSection[MAX_SFB_HCR];
  USHORT pNumExtendedSortedCodewordInSection[MAX_SFB_HCR + MAX_HCR_SETS];
  int numExtendedSortedCodewordInSectionIdx;
  USHORT pNumExtendedSortedSectionsInSets[MAX_HCR_SETS];
  int numExtendedSortedSectionsInSetsIdx;
  USHORT pReorderOffset[MAX_SFB_HCR];
  UCHAR pSortedCodebook[MAX_SFB_HCR];

  UCHAR pExtendedSortedCodebook[MAX_SFB_HCR + MAX_HCR_SETS];
  int extendedSortedCodebookIdx;
  UCHAR pMaxLenOfCbInExtSrtSec[MAX_SFB_HCR + MAX_HCR_SETS];
  int maxLenOfCbInExtSrtSecIdx;
  UCHAR pCodebookSwitch[MAX_SFB_HCR];
} HCR_SECTION_INFO;

typedef UINT (*STATEFUNC)(HANDLE_FDK_BITSTREAM, void *);

typedef struct {
  /* worst-case and 1024/4 non-PCWs exist in worst-case */
  FIXP_DBL
  *pResultBase; /* Base address for spectral data output target buffer */
  UINT iNode[1024 >> 2]; /* Helper indices for code books */
  USHORT
  iResultPointer[1024 >> 2]; /* Helper indices for accessing pResultBase */
  UINT pEscapeSequenceInfo[1024 >> 2];
  UINT codewordOffset;
  STATEFUNC pState;
  UCHAR pCodebook[1024 >> 2];
  UCHAR pCntSign[1024 >> 2];
  /* this array holds the states coded as integer values within the range
   * [0,1,..,7] */
  SCHAR pSta[1024 >> 2];
} HCR_NON_PCW_SIDEINFO;

typedef struct {
  HCR_INPUT_OUTPUT decInOut;
  HCR_SEGMENT_INFO segmentInfo;
  HCR_SECTION_INFO sectionInfo;
  HCR_NON_PCW_SIDEINFO nonPcwSideinfo;
} CErHcrInfo;

typedef CErHcrInfo *H_HCR_INFO;

#endif /* AACDEC_HCR_TYPES_H */
