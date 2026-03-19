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

   Description: interface - spatial specific config struct

*******************************************************************************/

#ifndef SAC_DEC_SSC_STRUCT_H
#define SAC_DEC_SSC_STRUCT_H

#include "FDK_audio.h"

#define MAX_NUM_QMF_BANDS (128)
#define MAX_TIME_SLOTS 64
#define MAX_INPUT_CHANNELS 1
#define MAX_OUTPUT_CHANNELS                                             \
  2 /* CAUTION: This does NOT restrict the number of                    \
                            output channels exclusively! In addition it \
       affects the max number of bitstream and residual channels! */
#define MAX_NUM_OTT (5)
#define MAX_NUM_TTT (0)
#define MAX_NUM_EXT_TYPES (8)
#define MAX_PARAMETER_BANDS (28)
#define MAX_PARAMETER_BANDS_LD (23)

#define MAX_NUM_XCHANNELS (6)

#define MAX_ARBITRARY_TREE_LEVELS (0)

typedef enum {
  /* CAUTION: Do not change enum values! */
  SPATIALDEC_FREQ_RES_40 = 40,
  SPATIALDEC_FREQ_RES_28 = 28,
  SPATIALDEC_FREQ_RES_23 = 23,
  SPATIALDEC_FREQ_RES_20 = 20,
  SPATIALDEC_FREQ_RES_15 = 15,
  SPATIALDEC_FREQ_RES_14 = 14,
  SPATIALDEC_FREQ_RES_12 = 12,
  SPATIALDEC_FREQ_RES_10 = 10,
  SPATIALDEC_FREQ_RES_9 = 9,
  SPATIALDEC_FREQ_RES_7 = 7,
  SPATIALDEC_FREQ_RES_5 = 5,
  SPATIALDEC_FREQ_RES_4 = 4

} SPATIALDEC_FREQ_RES;

typedef enum {

  SPATIALDEC_QUANT_FINE_DEF = 0,
  SPATIALDEC_QUANT_EDQ1 = 1,
  SPATIALDEC_QUANT_EDQ2 = 2,
  SPATIALDEC_QUANT_RSVD3 = 3,
  SPATIALDEC_QUANT_RSVD4 = 4,
  SPATIALDEC_QUANT_RSVD5 = 5,
  SPATIALDEC_QUANT_RSVD6 = 6,
  SPATIALDEC_QUANT_RSVD7 = 7

} SPATIALDEC_QUANT_MODE;

typedef enum { SPATIALDEC_MODE_RSVD7 = 7 } SPATIALDEC_TREE_CONFIG;

typedef enum {

  SPATIALDEC_GAIN_MODE0 = 0,
  SPATIALDEC_GAIN_RSVD1 = 1,
  SPATIALDEC_GAIN_RSVD2 = 2,
  SPATIALDEC_GAIN_RSVD3 = 3,
  SPATIALDEC_GAIN_RSVD4 = 4,
  SPATIALDEC_GAIN_RSVD5 = 5,
  SPATIALDEC_GAIN_RSVD6 = 6,
  SPATIALDEC_GAIN_RSVD7 = 7,
  SPATIALDEC_GAIN_RSVD8 = 8,
  SPATIALDEC_GAIN_RSVD9 = 9,
  SPATIALDEC_GAIN_RSVD10 = 10,
  SPATIALDEC_GAIN_RSVD11 = 11,
  SPATIALDEC_GAIN_RSVD12 = 12,
  SPATIALDEC_GAIN_RSVD13 = 13,
  SPATIALDEC_GAIN_RSVD14 = 14,
  SPATIALDEC_GAIN_RSVD15 = 15

} SPATIALDEC_FIXED_GAINS;

typedef enum {

  SPATIALDEC_TS_TPNOWHITE = 0,
  SPATIALDEC_TS_TPWHITE = 1,
  SPATIALDEC_TS_TES = 2,
  SPATIALDEC_TS_NOTS = 3,
  SPATIALDEC_TS_RSVD4 = 4,
  SPATIALDEC_TS_RSVD5 = 5,
  SPATIALDEC_TS_RSVD6 = 6,
  SPATIALDEC_TS_RSVD7 = 7,
  SPATIALDEC_TS_RSVD8 = 8,
  SPATIALDEC_TS_RSVD9 = 9,
  SPATIALDEC_TS_RSVD10 = 10,
  SPATIALDEC_TS_RSVD11 = 11,
  SPATIALDEC_TS_RSVD12 = 12,
  SPATIALDEC_TS_RSVD13 = 13,
  SPATIALDEC_TS_RSVD14 = 14,
  SPATIALDEC_TS_RSVD15 = 15

} SPATIALDEC_TS_CONF;

typedef enum {

  SPATIALDEC_DECORR_MODE0 = 0,
  SPATIALDEC_DECORR_MODE1 = 1,
  SPATIALDEC_DECORR_MODE2 = 2,
  SPATIALDEC_DECORR_RSVD3 = 3,
  SPATIALDEC_DECORR_RSVD4 = 4,
  SPATIALDEC_DECORR_RSVD5 = 5,
  SPATIALDEC_DECORR_RSVD6 = 6,
  SPATIALDEC_DECORR_RSVD7 = 7,
  SPATIALDEC_DECORR_RSVD8 = 8,
  SPATIALDEC_DECORR_RSVD9 = 9,
  SPATIALDEC_DECORR_RSVD10 = 10,
  SPATIALDEC_DECORR_RSVD11 = 11,
  SPATIALDEC_DECORR_RSVD12 = 12,
  SPATIALDEC_DECORR_RSVD13 = 13,
  SPATIALDEC_DECORR_RSVD14 = 14,
  SPATIALDEC_DECORR_RSVD15 = 15

} SPATIALDEC_DECORR_CONF;

typedef struct T_SPATIALDEC_OTT_CONF {
  int nOttBands;

} SPATIALDEC_OTT_CONF;

typedef struct T_SPATIALDEC_RESIDUAL_CONF {
  int bResidualPresent;
  int nResidualBands;

} SPATIALDEC_RESIDUAL_CONF;

typedef struct T_SPATIAL_SPECIFIC_CONFIG {
  UINT syntaxFlags;
  int samplingFreq;
  int nTimeSlots;
  SPATIALDEC_FREQ_RES freqRes;
  SPATIALDEC_TREE_CONFIG treeConfig;
  SPATIALDEC_QUANT_MODE quantMode;
  int bArbitraryDownmix;

  int bResidualCoding;
  SPATIALDEC_FIXED_GAINS bsFixedGainDMX;

  SPATIALDEC_TS_CONF tempShapeConfig;
  SPATIALDEC_DECORR_CONF decorrConfig;

  int nInputChannels;  /* derived from  treeConfig */
  int nOutputChannels; /* derived from  treeConfig */

  /* ott config */
  int nOttBoxes;                              /* derived from  treeConfig */
  SPATIALDEC_OTT_CONF OttConfig[MAX_NUM_OTT]; /* dimension nOttBoxes */

  /* ttt config */
  int nTttBoxes; /* derived from  treeConfig */

  /* residual config */
  SPATIALDEC_RESIDUAL_CONF
  ResidualConfig[MAX_NUM_OTT +
                 MAX_NUM_TTT]; /* dimension (nOttBoxes + nTttBoxes) */

  int sacExtCnt;
  int sacExtType[MAX_NUM_EXT_TYPES];
  int envQuantMode;

  AUDIO_OBJECT_TYPE coreCodec;

  UCHAR stereoConfigIndex;
  UCHAR coreSbrFrameLengthIndex; /* Table 70 in ISO/IEC FDIS 23003-3:2011 */
  UCHAR bsHighRateMode;
  UCHAR bsDecorrType;
  UCHAR bsPseudoLr;
  UCHAR bsPhaseCoding;
  UCHAR bsOttBandsPhasePresent;
  int bsOttBandsPhase;

  SCHAR ottCLDdefault[MAX_NUM_OTT];
  UCHAR numOttBandsIPD;
  UCHAR bitstreamOttBands[MAX_NUM_OTT];
  UCHAR numOttBands[MAX_NUM_OTT];

} SPATIAL_SPECIFIC_CONFIG;

#endif
