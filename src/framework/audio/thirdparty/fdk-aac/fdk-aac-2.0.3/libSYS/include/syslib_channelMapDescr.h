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

/** \file   syslib_channelMapDescr.h
 *  \brief  Function and structure declarations for the channel map descriptor implementation.
 */

#ifndef SYSLIB_CHANNELMAPDESCR_H
#define SYSLIB_CHANNELMAPDESCR_H

#include "machine_type.h"

/**
 * \brief  Contains information needed for a single channel map.
 */
typedef struct {
  const UCHAR*
      pChannelMap; /*!< Actual channel mapping for one single configuration. */
  UCHAR numChannels; /*!< The number of channels for the channel map which is
                        the maximum used channel index+1. */
} CHANNEL_MAP_INFO;

/**
 * \brief   This is the main data struct. It contains the mapping for all
 * channel configurations such as administration information.
 *
 * CAUTION: Do not access this structure directly from a algorithm specific
 * library. Always use one of the API access functions below!
 */
typedef struct {
  const CHANNEL_MAP_INFO* pMapInfoTab; /*!< Table of channel maps. */
  UINT mapInfoTabLen; /*!< Length of the channel map table array. */
  UINT fPassThrough;  /*!< Flag that defines whether the specified mapping shall
                         be applied  (value: 0) or the input just gets passed
                         through (MPEG mapping). */
} FDK_channelMapDescr;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Initialize a given channel map descriptor.
 *
 * \param  pMapDescr      Pointer to a channel map descriptor to be initialized.
 * \param  pMapInfoTab    Table of channel maps to initizalize the descriptor
 with.
 *                        If a NULL pointer is given a default table for
 WAV-like mapping will be used.
 * \param  mapInfoTabLen  Length of the channel map table array (pMapInfoTab).
 If a zero length is given a default table for WAV-like mapping will be used.
 * \param  fPassThrough   If the flag is set the reordering (given by
 pMapInfoTab) will be bypassed.
 */
void FDK_chMapDescr_init(FDK_channelMapDescr* const pMapDescr,
                         const CHANNEL_MAP_INFO* const pMapInfoTab,
                         const UINT mapInfoTabLen, const UINT fPassThrough);

/**
 * \brief  Change the channel reordering state of a given channel map
 * descriptor.
 *
 * \param  pMapDescr     Pointer to a (initialized) channel map descriptor.
 * \param  fPassThrough  If the flag is set the reordering (given by
 * pMapInfoTab) will be bypassed.
 * \return               Value unequal to zero if set operation was not
 * successful. And zero on success.
 */
int FDK_chMapDescr_setPassThrough(FDK_channelMapDescr* const pMapDescr,
                                  UINT fPassThrough);

/**
 * \brief  Get the mapping value for a specific channel and map index.
 *
 * \param  pMapDescr  Pointer to channel map descriptor.
 * \param  chIdx      Channel index.
 * \param  mapIdx     Mapping index (corresponding to the channel configuration
 * index).
 * \return            Mapping value.
 */
UCHAR FDK_chMapDescr_getMapValue(const FDK_channelMapDescr* const pMapDescr,
                                 const UCHAR chIdx, const UINT mapIdx);

/**
 * \brief  Evaluate whether channel map descriptor is reasonable or not.
 *
 * \param  pMapDescr Pointer to channel map descriptor.
 * \return           Value unequal to zero if descriptor is valid, otherwise
 * zero.
 */
int FDK_chMapDescr_isValid(const FDK_channelMapDescr* const pMapDescr);

/**
 * Extra variables for setting up Wg4 channel mapping.
 */
extern const CHANNEL_MAP_INFO FDK_mapInfoTabWg4[];
extern const UINT FDK_mapInfoTabLenWg4;

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYSLIB_CHANNELMAPDESCR_H) */
