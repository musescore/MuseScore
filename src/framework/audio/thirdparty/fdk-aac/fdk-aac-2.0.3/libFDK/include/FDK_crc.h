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

/******************* Library for basic calculation routines ********************

   Author(s):

   Description: CRC calculation

*******************************************************************************/

#ifndef FDK_CRC_H
#define FDK_CRC_H

#include "FDK_bitstream.h"

#define MAX_CRC_REGS                                                           \
  3 /*!< Maximal number of overlapping crc region in ADTS channel pair element \
       is two. Select three independent regions preventively. */

/**
 *  This structure describes single crc region used for crc calculation.
 */
typedef struct {
  UCHAR isActive;
  INT maxBits;
  INT bitBufCntBits;
  INT validBits;

} CCrcRegData;

/**
 *  CRC info structure.
 */
typedef struct {
  CCrcRegData crcRegData[MAX_CRC_REGS]; /*!< Multiple crc region description. */
  const USHORT*
      pCrcLookup; /*!< Pointer to lookup table filled in FDK_crcInit(). */

  USHORT crcPoly;    /*!< CRC generator polynom. */
  USHORT crcMask;    /*!< CRC mask. */
  USHORT startValue; /*!< CRC start value. */
  UCHAR crcLen;      /*!< CRC length. */

  UINT regStart; /*!< Start region marker for synchronization. */
  UINT regStop;  /*!< Stop region marker for synchronization. */

  USHORT crcValue; /*!< Crc value to be calculated. */

} FDK_CRCINFO;

/**
 *  CRC info handle.
 */
typedef FDK_CRCINFO* HANDLE_FDK_CRCINFO;

/**
 * \brief  Initialize CRC structure.
 *
 * The function initializes existing crc info structure with denoted
 * configuration.
 *
 * \param hCrcInfo              Pointer to an outlying allocated crc info
 * structure.
 * \param crcPoly               Configure crc polynom.
 * \param crcStartValue         Configure crc start value.
 * \param crcLen                Configure crc length.
 *
 * \return  none
 */
void FDKcrcInit(HANDLE_FDK_CRCINFO hCrcInfo, const UINT crcPoly,
                const UINT crcStartValue, const UINT crcLen);

/**
 * \brief  Reset CRC info structure.
 *
 * This function clears all intern states of the crc structure.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 *
 * \return  none
 */
void FDKcrcReset(HANDLE_FDK_CRCINFO hCrcInfo);

/**
 * \brief  Start CRC region with maximum number of bits.
 *
 * This function marks position in bitstream to be used as start point for crc
 * calculation. Bitstream range for crc calculation can be limited or kept
 * dynamic depending on mBits parameter. The crc region has to be terminated
 * with FDKcrcEndReg() in each case.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 * \param hBs                   Pointer to current bit buffer structure.
 * \param mBits                 Number of bits in crc region to be calculated.
 *                              - mBits > 0: Zero padding will be used for CRC
 * calculation, if there are less than mBits bits available.
 *                              - mBits < 0: No zero padding is done.
 *                              - mBits = 0: The number of bits used in crc
 * calculation is dynamically, depending on bitstream position between
 * FDKcrcStartReg() and FDKcrcEndReg()
 * call.
 *
 * \return  ID for the created region, -1 in case of an error
 */
INT FDKcrcStartReg(HANDLE_FDK_CRCINFO hCrcInfo, const HANDLE_FDK_BITSTREAM hBs,
                   const INT mBits);

/**
 * \brief  Ends CRC region.
 *
 * This function terminates crc region specified with FDKcrcStartReg(). The
 * number of bits in crc region depends on mBits parameter of FDKcrcStartReg().
 * This function calculates and updates crc in info structure.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 * \param hBs                   Pointer to current bit buffer structure.
 * \param reg                   Crc region ID created in FDKcrcStartReg().
 *
 * \return  0 on success
 */
INT FDKcrcEndReg(HANDLE_FDK_CRCINFO hCrcInfo, const HANDLE_FDK_BITSTREAM hBs,
                 const INT reg);

/**
 * \brief  This function returns crc value from info struct.
 *
 * \param hCrcInfo              Pointer to crc info stucture.
 *
 * \return  CRC value masked with crc length.
 */
USHORT FDKcrcGetCRC(const HANDLE_FDK_CRCINFO hCrcInfo);

#endif /* FDK_CRC_H */
