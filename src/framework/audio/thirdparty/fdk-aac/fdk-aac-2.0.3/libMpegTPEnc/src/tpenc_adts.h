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

/******************* MPEG transport format encoder library *********************

   Author(s):   Alex Groeschel

   Description: ADTS Transport writer

*******************************************************************************/

#ifndef TPENC_ADTS_H
#define TPENC_ADTS_H

#include "tp_data.h"

#include "FDK_crc.h"

typedef struct {
  INT sample_freq;
  CHANNEL_MODE channel_mode;
  UCHAR decoderCanDoMpeg4;
  UCHAR mpeg_id;
  UCHAR layer;
  UCHAR protection_absent;
  UCHAR profile;
  UCHAR sample_freq_index;
  UCHAR private_bit;
  UCHAR original;
  UCHAR home;
  UCHAR copyright_id;
  UCHAR copyright_start;
  USHORT frame_length;
  UCHAR num_raw_blocks;
  UCHAR BufferFullnesStartFlag;
  UCHAR channel_config_zero;
  int headerBits;       /*!< Header bit demand for the current raw data block */
  int currentBlock;     /*!< Index of current raw data block */
  int subFrameStartBit; /*!< Bit position where the current raw data block
                           begins */
  FDK_CRCINFO crcInfo;
} STRUCT_ADTS;

typedef STRUCT_ADTS *HANDLE_ADTS;

/**
 * \brief Initialize ADTS data structure
 *
 * \param hAdts ADTS data handle
 * \param config a valid CODER_CONFIG struct from where the required
 *        information for the ADTS header is extrated from
 *
 * \return 0 in case of success.
 */
INT adtsWrite_Init(HANDLE_ADTS hAdts, CODER_CONFIG *config);

/**
 * \brief Get the total bit overhead caused by ADTS
 *
 * \hAdts handle to ADTS data
 *
 * \return Amount of additional bits required for the current raw data block
 */
int adtsWrite_GetHeaderBits(HANDLE_ADTS hAdts);

/**
 * \brief Write an ADTS header into the given bitstream. May not write a header
 *        in case of multiple raw data blocks.
 *
 * \param hAdts ADTS data handle
 * \param hBitStream bitstream handle into which the ADTS may be written into
 * \param buffer_fullness the buffer fullness value for the ADTS header
 * \param the current raw data block length
 *
 * \return 0 in case of success.
 */
INT adtsWrite_EncodeHeader(HANDLE_ADTS hAdts, HANDLE_FDK_BITSTREAM hBitStream,
                           int bufferFullness, int frame_length);
/**
 * \brief Finish a ADTS raw data block
 *
 * \param hAdts ADTS data handle
 * \param hBs bitstream handle into which the ADTS may be written into
 * \param pBits a pointer to a integer holding the current bitstream buffer bit
 * count, which is corrected to the current raw data block boundary.
 *
 */
void adtsWrite_EndRawDataBlock(HANDLE_ADTS hAdts, HANDLE_FDK_BITSTREAM hBs,
                               int *bits);

/**
 * \brief Start CRC region with a maximum number of bits
 *        If mBits is positive zero padding will be used for CRC calculation, if
 * there are less than mBits bits available. If mBits is negative no zero
 * padding is done. If mBits is zero the memory for the buffer is
 * allocated dynamically, the number of bits is not limited.
 *
 * \param pAdts ADTS data handle
 * \param hBs bitstream handle of which the CRC region ends
 * \param mBits limit of number of bits to be considered for the requested CRC
 * region
 *
 * \return ID for the created region, -1 in case of an error
 */
int adtsWrite_CrcStartReg(HANDLE_ADTS pAdts, HANDLE_FDK_BITSTREAM hBs,
                          int mBits);

/**
 * \brief Ends CRC region identified by reg
 *
 * \param pAdts ADTS data handle
 * \param hBs bitstream handle of which the CRC region ends
 * \param reg a CRC region ID returned previously by adtsWrite_CrcStartReg()
 */
void adtsWrite_CrcEndReg(HANDLE_ADTS pAdts, HANDLE_FDK_BITSTREAM hBs, int reg);

#endif /* TPENC_ADTS_H */
