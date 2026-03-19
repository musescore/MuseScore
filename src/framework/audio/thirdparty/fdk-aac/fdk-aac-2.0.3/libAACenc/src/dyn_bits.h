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

#ifndef DYN_BITS_H
#define DYN_BITS_H

#include "common_fix.h"

#include "psy_const.h"
#include "aacenc_tns.h"

#define MAX_SECTIONS MAX_GROUPED_SFB
#define SECT_ESC_VAL_LONG 31
#define SECT_ESC_VAL_SHORT 7
#define CODE_BOOK_BITS 4
#define SECT_BITS_LONG 5
#define SECT_BITS_SHORT 3
#define PNS_PCM_BITS 9

typedef struct {
  INT codeBook;
  INT sfbStart;
  INT sfbCnt;
  INT sectionBits; /* huff + si ! */
} SECTION_INFO;

typedef struct {
  INT blockType;
  INT noOfGroups;
  INT sfbCnt;
  INT maxSfbPerGroup;
  INT sfbPerGroup;
  INT noOfSections;
  SECTION_INFO huffsection[MAX_SECTIONS];
  INT sideInfoBits; /* sectioning bits       */
  INT huffmanBits;  /* huffman    coded bits */
  INT scalefacBits; /* scalefac   coded bits */
  INT noiseNrgBits; /* noiseEnergy coded bits */
  INT firstScf;     /* first scf to be coded */
} SECTION_DATA;

struct BITCNTR_STATE {
  INT* bitLookUp;
  INT* mergeGainLookUp;
};

INT FDKaacEnc_BCNew(BITCNTR_STATE** phBC, UCHAR* dynamic_RAM);

void FDKaacEnc_BCClose(BITCNTR_STATE** phBC);

INT FDKaacEnc_dynBitCount(BITCNTR_STATE* const hBC,
                          const SHORT* const quantSpectrum,
                          const UINT* const maxValueInSfb,
                          const INT* const scalefac, const INT blockType,
                          const INT sfbCnt, const INT maxSfbPerGroup,
                          const INT sfbPerGroup, const INT* const sfbOffset,
                          SECTION_DATA* const RESTRICT sectionData,
                          const INT* const noiseNrg, const INT* const isBook,
                          const INT* const isScale, const UINT syntaxFlags);

#endif
