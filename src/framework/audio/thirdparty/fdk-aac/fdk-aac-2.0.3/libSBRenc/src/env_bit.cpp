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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Remaining SBR Bit Writing Routines
*/

#include "env_bit.h"
#include "cmondata.h"

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

/* ***************************** crcAdvance **********************************/
/**
 * @fn
 * @brief    updates crc data register
 * @return   none
 *
 * This function updates the crc register
 *
 */
static void crcAdvance(USHORT crcPoly, USHORT crcMask, USHORT *crc,
                       ULONG bValue, INT bBits) {
  INT i;
  USHORT flag;

  for (i = bBits - 1; i >= 0; i--) {
    flag = ((*crc) & crcMask) ? (1) : (0);
    flag ^= (bValue & (1 << i)) ? (1) : (0);

    (*crc) <<= 1;
    if (flag) (*crc) ^= crcPoly;
  }
}

/* ***************************** FDKsbrEnc_InitSbrBitstream
 * **********************************/
/**
 * @fn
 * @brief    Inittialisation of sbr bitstream, write of dummy header and CRC
 * @return   none
 *
 *
 *
 */

INT FDKsbrEnc_InitSbrBitstream(
    HANDLE_COMMON_DATA hCmonData,
    UCHAR *memoryBase, /*!< Pointer to bitstream buffer */
    INT memorySize,    /*!< Length of bitstream buffer in bytes */
    HANDLE_FDK_CRCINFO hCrcInfo, UINT sbrSyntaxFlags) /*!< SBR syntax flags */
{
  INT crcRegion = 0;

  /* reset bit buffer */
  FDKresetBitbuffer(&hCmonData->sbrBitbuf, BS_WRITER);

  FDKinitBitStream(&hCmonData->tmpWriteBitbuf, memoryBase, memorySize, 0,
                   BS_WRITER);

  if (sbrSyntaxFlags & SBR_SYNTAX_CRC) {
    if (sbrSyntaxFlags & SBR_SYNTAX_DRM_CRC) { /* Init and start CRC region */
      FDKwriteBits(&hCmonData->sbrBitbuf, 0x0, SI_SBR_DRM_CRC_BITS);
      FDKcrcInit(hCrcInfo, 0x001d, 0xFFFF, SI_SBR_DRM_CRC_BITS);
      crcRegion = FDKcrcStartReg(hCrcInfo, &hCmonData->sbrBitbuf, 0);
    } else {
      FDKwriteBits(&hCmonData->sbrBitbuf, 0x0, SI_SBR_CRC_BITS);
    }
  }

  return (crcRegion);
}

/* ************************** FDKsbrEnc_AssembleSbrBitstream
 * *******************************/
/**
 * @fn
 * @brief    Formats the SBR payload
 * @return   nothing
 *
 * Also the CRC will be calculated here.
 *
 */

void FDKsbrEnc_AssembleSbrBitstream(HANDLE_COMMON_DATA hCmonData,
                                    HANDLE_FDK_CRCINFO hCrcInfo, INT crcRegion,
                                    UINT sbrSyntaxFlags) {
  USHORT crcReg = SBR_CRCINIT;
  INT numCrcBits, i;

  /* check if SBR is present */
  if (hCmonData == NULL) return;

  hCmonData->sbrFillBits = 0; /* Fill bits are written only for GA streams */

  if (sbrSyntaxFlags & SBR_SYNTAX_DRM_CRC) {
    /*
     * Calculate and write DRM CRC
     */
    FDKcrcEndReg(hCrcInfo, &hCmonData->sbrBitbuf, crcRegion);
    FDKwriteBits(&hCmonData->tmpWriteBitbuf, FDKcrcGetCRC(hCrcInfo) ^ 0xFF,
                 SI_SBR_DRM_CRC_BITS);
  } else {
    if (!(sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY)) {
      /* Do alignment here, because its defined as part of the
       * sbr_extension_data */
      int sbrLoad = hCmonData->sbrHdrBits + hCmonData->sbrDataBits;

      if (sbrSyntaxFlags & SBR_SYNTAX_CRC) {
        sbrLoad += SI_SBR_CRC_BITS;
      }

      sbrLoad += 4; /* Do byte Align with 4 bit offset. ISO/IEC 14496-3:2005(E)
                       page 39. */

      hCmonData->sbrFillBits = (8 - (sbrLoad % 8)) % 8;

      /*
        append fill bits
      */
      FDKwriteBits(&hCmonData->sbrBitbuf, 0, hCmonData->sbrFillBits);

      FDK_ASSERT(FDKgetValidBits(&hCmonData->sbrBitbuf) % 8 == 4);
    }

    /*
      calculate crc
    */
    if (sbrSyntaxFlags & SBR_SYNTAX_CRC) {
      FDK_BITSTREAM tmpCRCBuf = hCmonData->sbrBitbuf;
      FDKresetBitbuffer(&tmpCRCBuf, BS_READER);

      numCrcBits = hCmonData->sbrHdrBits + hCmonData->sbrDataBits +
                   hCmonData->sbrFillBits;

      for (i = 0; i < numCrcBits; i++) {
        INT bit;
        bit = FDKreadBits(&tmpCRCBuf, 1);
        crcAdvance(SBR_CRC_POLY, SBR_CRC_MASK, &crcReg, bit, 1);
      }
      crcReg &= (SBR_CRC_RANGE);

      /*
       * Write CRC data.
       */
      FDKwriteBits(&hCmonData->tmpWriteBitbuf, crcReg, SI_SBR_CRC_BITS);
    }
  }

  FDKsyncCache(&hCmonData->tmpWriteBitbuf);
}
