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

/************************* System integration library **************************

   Author(s):   Thomas Dietzen

   Description:

*******************************************************************************/

/** \file   syslib_channelMapDescr.cpp
 *  \brief  Implementation of routines that handle the channel map descriptor.
 */

#include "syslib_channelMapDescr.h"

#define DFLT_CH_MAP_TAB_LEN \
  (15) /* Length of the default channel map info table. */

/**
 * \brief  The following arrays provide a channel map for each channel config (0
 * to 14).
 *
 * The i-th channel will be mapped to the postion a[i-1]+1
 * with i>0 and a[] is one of the following mapping arrays.
 */
static const UCHAR mapFallback[] = {0,  1,  2,  3,  4,  5,  6,  7,
                                    8,  9,  10, 11, 12, 13, 14, 15,
                                    16, 17, 18, 19, 20, 21, 22, 23};
static const UCHAR mapCfg1[] = {0, 1};
static const UCHAR mapCfg2[] = {0, 1};
static const UCHAR mapCfg3[] = {2, 0, 1};
static const UCHAR mapCfg4[] = {2, 0, 1, 3};
static const UCHAR mapCfg5[] = {2, 0, 1, 3, 4};
static const UCHAR mapCfg6[] = {2, 0, 1, 4, 5, 3};
static const UCHAR mapCfg7[] = {2, 6, 7, 0, 1, 4, 5, 3};
static const UCHAR mapCfg11[] = {2, 0, 1, 4, 5, 6, 3};
static const UCHAR mapCfg12[] = {2, 0, 1, 6, 7, 4, 5, 3};
static const UCHAR mapCfg13[] = {2,  6,  7,  0,  1,  10, 11, 4,
                                 5,  8,  3,  9,  14, 12, 13, 18,
                                 19, 15, 16, 17, 20, 21, 22, 23};
static const UCHAR mapCfg14[] = {2, 0, 1, 4, 5, 3, 6, 7};

/**
 * \brief  Default table comprising channel map information for each channel
 * config (0 to 14).
 */
static const CHANNEL_MAP_INFO mapInfoTabDflt[DFLT_CH_MAP_TAB_LEN] =
    {/* chCfg,  map,         numCh */
     /*  0 */ {mapFallback, 24},
     /*  1 */ {mapCfg1, 2},
     /*  2 */ {mapCfg2, 2},
     /*  3 */ {mapCfg3, 3},
     /*  4 */ {mapCfg4, 4},
     /*  5 */ {mapCfg5, 5},
     /*  6 */ {mapCfg6, 6},
     /*  7 */ {mapCfg7, 8},
     /*  8 */ {mapFallback, 24},
     /*  9 */ {mapFallback, 24},
     /* 10 */ {mapFallback, 24},
     /* 11 */ {mapCfg11, 7},
     /* 12 */ {mapCfg12, 8},
     /* 13 */ {mapCfg13, 24},
     /* 14 */ {mapCfg14, 8}};


static const UCHAR mapWg4Cfg1[]  = {0, 1};
static const UCHAR mapWg4Cfg2[]  = {0, 1};
static const UCHAR mapWg4Cfg3[]  = {2, 0, 1};
static const UCHAR mapWg4Cfg4[]  = {3, 0, 1, 2};
static const UCHAR mapWg4Cfg5[]  = {4, 0, 1, 2, 3};
static const UCHAR mapWg4Cfg6[]  = {4, 0, 1, 2, 3, 5};
static const UCHAR mapWg4Cfg7[]  = {6, 0, 1, 2, 3, 4, 5, 7};
static const UCHAR mapWg4Cfg14[] = {6, 0, 1, 2, 3, 4, 5, 7};

const CHANNEL_MAP_INFO FDK_mapInfoTabWg4[] =
    {/* chCfg,  map,         numCh */
     /*  0 */ {mapFallback, 24},
     /*  1 */ {mapWg4Cfg1, 2},
     /*  2 */ {mapWg4Cfg2, 2},
     /*  3 */ {mapWg4Cfg3, 3},
     /*  4 */ {mapWg4Cfg4, 4},
     /*  5 */ {mapWg4Cfg5, 5},
     /*  6 */ {mapWg4Cfg6, 6},
     /*  7 */ {mapWg4Cfg7, 8},
     /*  8 */ {mapFallback, 24},
     /*  9 */ {mapFallback, 24},
     /* 10 */ {mapFallback, 24},
     /* 11 */ {mapFallback, 24},  // Unhandled for Wg4 yet
     /* 12 */ {mapFallback, 24},  // Unhandled for Wg4 yet
     /* 13 */ {mapFallback, 24},  // Unhandled for Wg4 yet
     /* 14 */ {mapFallback, 24}}; // Unhandled for Wg4 yet

const UINT FDK_mapInfoTabLenWg4 = sizeof(FDK_mapInfoTabWg4)/sizeof(FDK_mapInfoTabWg4[0]);


/**
 * Get the mapping value for a specific channel and map index.
 */
UCHAR FDK_chMapDescr_getMapValue(const FDK_channelMapDescr* const pMapDescr,
                                 const UCHAR chIdx, const UINT mapIdx) {
  UCHAR mapValue = chIdx; /* Pass through by default. */

  FDK_ASSERT(pMapDescr != NULL);

  if ((pMapDescr->fPassThrough == 0) && (pMapDescr->pMapInfoTab != NULL) &&
      (pMapDescr->mapInfoTabLen > mapIdx)) { /* Nest sanity check to avoid
                                                possible memory access
                                                violation. */
    if (chIdx < pMapDescr->pMapInfoTab[mapIdx].numChannels) {
      mapValue = pMapDescr->pMapInfoTab[mapIdx].pChannelMap[chIdx];
    }
  }
  return mapValue;
}

/**
 * \brief  Evaluate whether single channel map is reasonable or not.
 *
 * \param  pMapInfo  Pointer to channel map.
 * \return           Value unequal to zero if map is valid, otherwise zero.
 */
static int fdk_chMapDescr_isValidMap(const CHANNEL_MAP_INFO* const pMapInfo) {
  int result = 1;
  UINT i;

  if (pMapInfo == NULL) {
    result = 0;
  } else {
    UINT numChannels = pMapInfo->numChannels;

    /* Check for all map values if they are inside the range 0 to numChannels-1
     * and unique. */
    if (numChannels < 32) { /* Optimized version for less than 32 channels.
                               Needs only one loop. */
      UINT mappedChMask = 0x0;
      for (i = 0; i < numChannels; i += 1) {
        mappedChMask |= 1 << pMapInfo->pChannelMap[i];
      }
      if (mappedChMask != (((UINT)1 << numChannels) - 1)) {
        result = 0;
      }
    } else { /* General case that can handle all number of channels but needs
                one more loop. */
      for (i = 0; (i < numChannels) && result; i += 1) {
        UINT j;
        UCHAR value0 = pMapInfo->pChannelMap[i];

        if (value0 > numChannels - 1) { /* out of range? */
          result = 0;
        }
        for (j = numChannels - 1; (j > i) && result; j -= 1) {
          if (value0 == pMapInfo->pChannelMap[j]) { /* not unique */
            result = 0;
          }
        }
      }
    }
  }

  return result;
}

/**
 * Evaluate whether channel map descriptor is reasonable or not.
 */
int FDK_chMapDescr_isValid(const FDK_channelMapDescr* const pMapDescr) {
  int result = 0;
  UINT i;

  if (pMapDescr != NULL) {
    result = 1;
    for (i = 0; (i < pMapDescr->mapInfoTabLen) && result; i += 1) {
      if (!fdk_chMapDescr_isValidMap(&pMapDescr->pMapInfoTab[i])) {
        result = 0;
      }
    }
  }
  return result;
}

/**
 * Initialize the complete channel map descriptor.
 */
void FDK_chMapDescr_init(FDK_channelMapDescr* const pMapDescr,
                         const CHANNEL_MAP_INFO* const pMapInfoTab,
                         const UINT mapInfoTabLen, const UINT fPassThrough) {
  if (pMapDescr != NULL) {
    int useDefaultTab = 1;

    pMapDescr->fPassThrough = (fPassThrough == 0) ? 0 : 1;

    if ((pMapInfoTab != NULL) && (mapInfoTabLen > 0)) {
      /* Set the valid custom mapping table. */
      pMapDescr->pMapInfoTab = pMapInfoTab;
      pMapDescr->mapInfoTabLen = mapInfoTabLen;
      /* Validate the complete descriptor. */
      useDefaultTab = (FDK_chMapDescr_isValid(pMapDescr) == 0) ? 1 : 0;
    }
    if (useDefaultTab != 0) {
      /* Set default table. */
      pMapDescr->pMapInfoTab = mapInfoTabDflt;
      pMapDescr->mapInfoTabLen = DFLT_CH_MAP_TAB_LEN;
    }
  }
}

/**
 * Set channel mapping bypass flag in a given channel map descriptor.
 */
int FDK_chMapDescr_setPassThrough(FDK_channelMapDescr* const pMapDescr,
                                  UINT fPassThrough) {
  int err = 1;

  if (pMapDescr != NULL) {
    if ((pMapDescr->pMapInfoTab != NULL) && (pMapDescr->mapInfoTabLen > 0)) {
      pMapDescr->fPassThrough = (fPassThrough == 0) ? 0 : 1;
      err = 0;
    }
  }

  return err;
}
