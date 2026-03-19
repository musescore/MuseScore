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

/*********************** MPEG surround decoder library *************************

   Author(s):

   Description: SAC Dec bitstream decoder

*******************************************************************************/

/*!
  \file
  \brief  Spatial Audio bitstream decoder
*/

#ifndef SAC_BITDEC_H
#define SAC_BITDEC_H

#include "sac_dec.h"

typedef struct {
  SCHAR numInputChannels;
  SCHAR numOutputChannels;
  SCHAR numOttBoxes;
  SCHAR numTttBoxes;
  SCHAR ottModeLfe[MAX_NUM_OTT];
} TREEPROPERTIES;

enum { TREE_212 = 7, TREE_DUMMY = 255 };

enum { QUANT_FINE = 0, QUANT_EBQ1 = 1, QUANT_EBQ2 = 2 };

SACDEC_ERROR SpatialDecParseSpecificConfigHeader(
    HANDLE_FDK_BITSTREAM bitstream,
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
    AUDIO_OBJECT_TYPE coreCodec, SPATIAL_DEC_UPMIX_TYPE upmixType);

SACDEC_ERROR SpatialDecParseMps212Config(
    HANDLE_FDK_BITSTREAM bitstream,
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig, int samplingRate,
    AUDIO_OBJECT_TYPE coreCodec, INT stereoConfigIndex,
    INT coreSbrFrameLengthIndex);

SACDEC_ERROR SpatialDecParseSpecificConfig(
    HANDLE_FDK_BITSTREAM bitstream,
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig, int sacHeaderLen,
    AUDIO_OBJECT_TYPE coreCodec);

int SpatialDecDefaultSpecificConfig(
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
    AUDIO_OBJECT_TYPE coreCodec, int samplingFreq, int nTimeSlots,
    int sacDecoderLevel, int isBlind, int coreChannels);

SACDEC_ERROR SpatialDecCreateBsFrame(SPATIAL_BS_FRAME *bsFrame,
                                     BS_LL_STATE *llState);

void SpatialDecCloseBsFrame(SPATIAL_BS_FRAME *bsFrame);

SACDEC_ERROR SpatialDecParseFrameData(
    spatialDec *self, SPATIAL_BS_FRAME *frame, HANDLE_FDK_BITSTREAM bitstream,
    const SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig, UPMIXTYPE upmixType,
    int fGlobalIndependencyFlag);

SACDEC_ERROR SpatialDecDecodeFrame(spatialDec *self, SPATIAL_BS_FRAME *frame);

SACDEC_ERROR SpatialDecDecodeHeader(
    spatialDec *self, SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig);

#endif
