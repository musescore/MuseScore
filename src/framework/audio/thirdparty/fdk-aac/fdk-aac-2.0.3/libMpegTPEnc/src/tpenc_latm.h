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

   Author(s):

   Description:

*******************************************************************************/

#ifndef TPENC_LATM_H
#define TPENC_LATM_H

#include "tpenc_lib.h"
#include "FDK_bitstream.h"

#define DEFAULT_LATM_NR_OF_SUBFRAMES 1
#define DEFAULT_LATM_SMC_REPEAT 8

#define MAX_AAC_LAYERS 9

#define LATM_MAX_PROGRAMS 1
#define LATM_MAX_STREAM_ID 16

#define LATM_MAX_LAYERS 1 /*MAX_AAC_LAYERS*/

#define MAX_NR_OF_SUBFRAMES                         \
  2 /* set this carefully to avoid buffer overflows \
     */

typedef enum { LATMVAR_SIMPLE_SEQUENCE } LATM_VAR_MODE;

typedef struct {
  signed int frameLengthType;
  signed int frameLengthBits;
  signed int varFrameLengthTable[4];
  signed int streamID;
} LATM_LAYER_INFO;

typedef struct {
  LATM_LAYER_INFO m_linfo[LATM_MAX_PROGRAMS][LATM_MAX_LAYERS];
  CODER_CONFIG *config[LATM_MAX_PROGRAMS][LATM_MAX_LAYERS];

  LATM_VAR_MODE varMode;
  TRANSPORT_TYPE tt;

  int audioMuxLengthBytes;

  int audioMuxLengthBytesPos;
  int taraBufferFullness; /* state of the bit reservoir */
  int varStreamCnt;

  UCHAR
  latmFrameCounter;      /* Current frame number. Counts modulo muxConfigPeriod
                          */
  UCHAR muxConfigPeriod; /* Distance in frames between MuxConfig */

  UCHAR
  audioMuxVersion; /* AMV1 supports transmission of taraBufferFullness and
                      ASC lengths */
  UCHAR audioMuxVersionA; /* for future extensions */

  UCHAR noProgram;
  UCHAR noLayer[LATM_MAX_PROGRAMS];
  UCHAR fractDelayPresent;

  UCHAR allStreamsSameTimeFraming;
  UCHAR subFrameCnt;      /* Current Subframe frame */
  UCHAR noSubframes;      /* Number of subframes    */
  UINT latmSubframeStart; /* Position of current subframe start */
  UCHAR noSubframes_next;

  UCHAR otherDataLenBits; /* AudioMuxElement other data bits */
  UCHAR fillBits;         /* AudioMuxElement fill bits */
  UINT streamMuxConfigBits;

} LATM_STREAM;

typedef LATM_STREAM *HANDLE_LATM_STREAM;

/**
 * \brief Initialize LATM_STREAM Handle. Creates automatically one program with
 * one layer with the given layerConfig. The layerConfig must be persisten
 * because references to this pointer are made at any time again. Use
 * transportEnc_Latm_AddLayer() to add more programs/layers.
 *
 * \param hLatmStreamInfo HANDLE_LATM_STREAM handle
 * \param hBs Bitstream handle
 * \param layerConfig a valid CODER_CONFIG struct containing the current audio
 * configuration parameters
 * \param audioMuxVersion the LATM audioMuxVersion to be used
 * \param tt the specific TRANSPORT_TYPE to be used, either TT_MP4_LOAS,
 * TT_MP4_LATM_MCP1 or TT_MP4_LATM_MCP0 LOAS
 * \param cb callback information structure.
 *
 * \return an TRANSPORTENC_ERROR error code
 */
TRANSPORTENC_ERROR transportEnc_Latm_Init(HANDLE_LATM_STREAM hLatmStreamInfo,
                                          HANDLE_FDK_BITSTREAM hBs,
                                          CODER_CONFIG *layerConfig,
                                          UINT audioMuxVersion,
                                          TRANSPORT_TYPE tt, CSTpCallBacks *cb);

/**
 * \brief Write addional other data bits in AudioMuxElement
 *
 * \param hAss HANDLE_LATM_STREAM handle
 * \param otherDataBits number of other data bits to be written
 *
 * \return an TRANSPORTENC_ERROR error code
 */
TRANSPORTENC_ERROR transportEnc_LatmAddOtherDataBits(HANDLE_LATM_STREAM hAss,
                                                     const int otherDataBits);

/**
 * \brief Get bit demand of next LATM/LOAS header
 *
 * \param hAss HANDLE_LATM_STREAM handle
 * \param streamDataLength the length of the payload
 *
 * \return the number of bits required by the LATM/LOAS headers
 */
unsigned int transportEnc_LatmCountTotalBitDemandHeader(
    HANDLE_LATM_STREAM hAss, unsigned int streamDataLength);

/**
 * \brief Write LATM/LOAS header into given bitstream handle
 *
 * \param hLatmStreamInfo HANDLE_LATM_STREAM handle
 * \param hBitstream Bitstream handle
 * \param auBits amount of current payload bits
 * \param bufferFullness LATM buffer fullness value
 * \param cb callback information structure.
 *
 * \return an TRANSPORTENC_ERROR error code
 */
TRANSPORTENC_ERROR
transportEnc_LatmWrite(HANDLE_LATM_STREAM hAss, HANDLE_FDK_BITSTREAM hBitstream,
                       int auBits, int bufferFullness, CSTpCallBacks *cb);

/**
 * \brief Adjust bit count relative to current subframe
 *
 * \param hAss HANDLE_LATM_STREAM handle
 * \param pBits pointer to an int, where the current frame bit count is
 * contained, and where the subframe relative bit count will be returned into
 *
 * \return void
 */
void transportEnc_LatmAdjustSubframeBits(HANDLE_LATM_STREAM hAss, int *pBits);

/**
 * \brief Request an LATM frame, which may, or may not be available
 *
 * \param hAss HANDLE_LATM_STREAM handle
 * \param hBs Bitstream handle
 * \param pBytes pointer to an int, where the current frame byte count stored
 * into. A return value of zero means that currently no LATM/LOAS frame can be
 * returned. The latter is expected in case of multiple subframes being
 * used.
 *
 * \return an TRANSPORTENC_ERROR error code
 */
TRANSPORTENC_ERROR transportEnc_LatmGetFrame(HANDLE_LATM_STREAM hAss,
                                             HANDLE_FDK_BITSTREAM hBs,
                                             int *pBytes);

/**
 * \brief Write a StreamMuxConfig into the given bitstream handle
 *
 * \param hAss HANDLE_LATM_STREAM handle
 * \param hBs Bitstream handle
 * \param bufferFullness LATM buffer fullness value
 * \param cb callback information structure.
 *
 * \return void
 */
TRANSPORTENC_ERROR
CreateStreamMuxConfig(HANDLE_LATM_STREAM hAss, HANDLE_FDK_BITSTREAM hBs,
                      int bufferFullness, CSTpCallBacks *cb);

#endif /* TPENC_LATM_H */
