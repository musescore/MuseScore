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

/******************* MPEG transport format decoder library *********************

   Author(s):   Josef Hoepfl

   Description: ADTS interface

*******************************************************************************/

#ifndef TPDEC_ADTS_H
#define TPDEC_ADTS_H

#include "tpdec_lib.h"

#define ADTS_SYNCWORD (0xfff)
#define ADTS_SYNCLENGTH (12)         /* in bits */
#define ADTS_HEADERLENGTH (56)       /* minimum header size in bits */
#define ADTS_FIXED_HEADERLENGTH (28) /* in bits */
#define ADTS_VARIABLE_HEADERLENGTH (ADTS_HEADERLENGTH - ADTS_FIXED_HEADERLENGTH)

#ifdef CHECK_TWO_SYNCS
#define ADTS_MIN_TP_BUF_SIZE (8191 + 2)
#else
#define ADTS_MIN_TP_BUF_SIZE (8191)
#endif

#include "FDK_crc.h"

typedef struct {
  /* ADTS header fields */
  UCHAR mpeg_id;
  UCHAR layer;
  UCHAR protection_absent;
  UCHAR profile;
  UCHAR sample_freq_index;
  UCHAR private_bit;
  UCHAR channel_config;
  UCHAR original;
  UCHAR home;
  UCHAR copyright_id;
  UCHAR copyright_start;
  USHORT frame_length;
  USHORT adts_fullness;
  UCHAR num_raw_blocks;
  UCHAR num_pce_bits;
} STRUCT_ADTS_BS;

struct STRUCT_ADTS {
  STRUCT_ADTS_BS bs;

  UCHAR decoderCanDoMpeg4;
  UCHAR BufferFullnesStartFlag;

  FDK_CRCINFO crcInfo;        /* CRC state info */
  USHORT crcReadValue;        /* CRC value read from bitstream data */
  USHORT rawDataBlockDist[4]; /* distance between each raw data block. Not the
                                 same as found in the bitstream */
};

typedef struct STRUCT_ADTS *HANDLE_ADTS;

/*!
  \brief Initialize ADTS CRC

  The function initialzes the crc buffer and the crc lookup table.

  \return  none
*/
void adtsRead_CrcInit(HANDLE_ADTS pAdts);

/**
 * \brief Starts CRC region with a maximum number of bits
 *        If mBits is positive zero padding will be used for CRC calculation, if
 * there are less than mBits bits available. If mBits is negative no zero
 * padding is done. If mBits is zero the memory for the buffer is
 * allocated dynamically, the number of bits is not limited.
 *
 * \param pAdts ADTS data handle
 * \param hBs bitstream handle, on which the CRC region referes to
 * \param mBits max number of bits in crc region to be considered
 *
 * \return  ID for the created region, -1 in case of an error
 */
int adtsRead_CrcStartReg(HANDLE_ADTS pAdts, HANDLE_FDK_BITSTREAM hBs,
                         int mBits);

/**
 * \brief Ends CRC region identified by reg
 *
 * \param pAdts ADTS data handle
 * \param hBs bitstream handle, on which the CRC region referes to
 * \param reg CRC regions ID returned by adtsRead_CrcStartReg()
 *
 * \return  none
 */
void adtsRead_CrcEndReg(HANDLE_ADTS pAdts, HANDLE_FDK_BITSTREAM hBs, int reg);

/**
 * \brief Check CRC
 *
 * Checks if the currently calculated CRC matches the CRC field read from the
 * bitstream Deletes all CRC regions.
 *
 * \param pAdts ADTS data handle
 *
 * \return Returns 0 if they are identical otherwise 1
 */
TRANSPORTDEC_ERROR adtsRead_CrcCheck(HANDLE_ADTS pAdts);

/**
 * \brief Check if we have a valid ADTS frame at the current bitbuffer position
 *
 * This function assumes enough bits in buffer for the current frame.
 * It reads out the header bits to prepare the bitbuffer for the decode loop.
 * In case the header bits show an invalid bitstream/frame, the whole frame is
 * skipped.
 *
 * \param pAdts ADTS data handle which is filled with parsed ADTS header data
 * \param bs handle of bitstream from whom the ADTS header is read
 *
 * \return  error status
 */
TRANSPORTDEC_ERROR adtsRead_DecodeHeader(HANDLE_ADTS pAdts,
                                         CSAudioSpecificConfig *pAsc,
                                         HANDLE_FDK_BITSTREAM bs,
                                         const INT ignoreBufferFullness);

/**
 * \brief Get the raw data block length of the given block number.
 *
 * \param pAdts ADTS data handle
 * \param blockNum current raw data block index
 * \param pLength pointer to an INT where the length of the given raw data block
 * is stored into the returned value might be -1, in which case the raw data
 * block length is unknown.
 *
 * \return  error status
 */
int adtsRead_GetRawDataBlockLength(HANDLE_ADTS pAdts, INT blockNum);

#endif /* TPDEC_ADTS_H */
