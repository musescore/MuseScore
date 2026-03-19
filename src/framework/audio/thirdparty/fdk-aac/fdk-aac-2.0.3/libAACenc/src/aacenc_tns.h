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

   Author(s):   Alex Groeschel

   Description: Temporal noise shaping

*******************************************************************************/

#ifndef AACENC_TNS_H
#define AACENC_TNS_H

#include "common_fix.h"

#include "psy_const.h"

#ifndef PI
#define PI 3.1415926535897931f
#endif

/**
 * TNS_ENABLE_MASK
 * This bitfield defines which TNS features are enabled
 *   The TNS mask is composed of 4 bits.
 *   tnsMask |= 0x1; activate TNS short blocks
 *   tnsMask |= 0x2; activate TNS for long blocks
 *   tnsMask |= 0x4; activate TNS PEAK tool for short blocks
 *   tnsMask |= 0x8; activate TNS PEAK tool for long blocks
 */
#define TNS_ENABLE_MASK 0xf

/* TNS max filter order for Low Complexity MPEG4 profile */
#define TNS_MAX_ORDER 12

#define MAX_NUM_OF_FILTERS 2

#define HIFILT 0 /* index of higher filter   */
#define LOFILT 1 /* index of lower filter    */

typedef struct { /* stuff that is tabulated dependent on bitrate etc. */
  INT filterEnabled[MAX_NUM_OF_FILTERS];
  INT threshOn[MAX_NUM_OF_FILTERS];        /* min. prediction gain for using tns
                                              TABUL*/
  INT filterStartFreq[MAX_NUM_OF_FILTERS]; /* lowest freq for lpc TABUL*/
  INT tnsLimitOrder[MAX_NUM_OF_FILTERS];   /* Limit for TNS order TABUL*/
  INT tnsFilterDirection[MAX_NUM_OF_FILTERS]; /* Filtering direction, 0=up,
                                                 1=down TABUL */
  INT acfSplit[MAX_NUM_OF_FILTERS];
  FIXP_DBL tnsTimeResolution[MAX_NUM_OF_FILTERS]; /* TNS max. time resolution
                                                     TABUL. Should be fract but
                                                     MSVC won't compile then */
  INT seperateFiltersAllowed;
} TNS_PARAMETER_TABULATED;

typedef struct { /*assigned at InitTime*/
  TNS_PARAMETER_TABULATED confTab;
  INT isLowDelay;
  INT tnsActive;
  INT maxOrder; /* max. order of tns filter */
  INT coefRes;
  FIXP_DBL acfWindow[MAX_NUM_OF_FILTERS][TNS_MAX_ORDER + 3 + 1];
  /* now some things that only probably can be done at Init time;
     could be they have to be split up for each individual (short) window or
     even filter.  */
  INT lpcStartBand[MAX_NUM_OF_FILTERS];
  INT lpcStartLine[MAX_NUM_OF_FILTERS];
  INT lpcStopBand;
  INT lpcStopLine;

} TNS_CONFIG;

typedef struct {
  INT tnsActive[MAX_NUM_OF_FILTERS];
  INT predictionGain[MAX_NUM_OF_FILTERS];
} TNS_SUBBLOCK_INFO;

typedef struct { /*changed at runTime*/
  TNS_SUBBLOCK_INFO subBlockInfo[TRANS_FAC];
  FIXP_DBL ratioMultTable[TRANS_FAC][MAX_SFB_SHORT];
} TNS_DATA_SHORT;

typedef struct { /*changed at runTime*/
  TNS_SUBBLOCK_INFO subBlockInfo;
  FIXP_DBL ratioMultTable[MAX_SFB_LONG];
} TNS_DATA_LONG;

/* can be implemented as union */
typedef shouldBeUnion {
  TNS_DATA_LONG Long;
  TNS_DATA_SHORT Short;
}
TNS_DATA_RAW;

typedef struct {
  INT numOfSubblocks;
  TNS_DATA_RAW dataRaw;
  INT tnsMaxScaleSpec;
  INT filtersMerged;
} TNS_DATA;

typedef struct {
  INT numOfFilters[TRANS_FAC];
  INT coefRes[TRANS_FAC];
  INT length[TRANS_FAC][MAX_NUM_OF_FILTERS];
  INT order[TRANS_FAC][MAX_NUM_OF_FILTERS];
  INT direction[TRANS_FAC][MAX_NUM_OF_FILTERS];
  INT coefCompress[TRANS_FAC][MAX_NUM_OF_FILTERS];
  /* for Long: length TNS_MAX_ORDER (12 for LC) is required -> 12 */
  /* for Short: length TRANS_FAC*TNS_MAX_ORDER (only 5 for short LC) is required
   * -> 8*5=40 */
  /* Currently TRANS_FAC*TNS_MAX_ORDER = 8*12 = 96 (for LC) is used (per
   * channel)! Memory could be saved here! */
  INT coef[TRANS_FAC][MAX_NUM_OF_FILTERS][TNS_MAX_ORDER];
} TNS_INFO;

INT FDKaacEnc_FreqToBandWidthRounding(const INT freq, const INT fs,
                                      const INT numOfBands,
                                      const INT *bandStartOffset);

#endif /* AACENC_TNS_H */
