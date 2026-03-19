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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#ifndef DRCDEC_TYPES_H
#define DRCDEC_TYPES_H

#include "common_fix.h"

/* Data structures corresponding to static and dynamic DRC/Loudness payload
   as defined in section 7 of MPEG-D DRC standard, ISO/IEC 23003-4 */

/**************/
/* uniDrcGain */
/**************/

typedef struct {
  FIXP_SGL gainDb; /* e = 7 */
  SHORT time;
} GAIN_NODE;

/* uniDrcGainExtension() (Table 56) */
typedef struct {
  UCHAR uniDrcGainExtType[8];
  ULONG extBitSize[8 - 1];
} UNI_DRC_GAIN_EXTENSION;

/* uniDrcGain() (Table 55) */
typedef struct {
  UCHAR nNodes[12]; /* unsaturated value, i.e. as provided in bitstream */
  GAIN_NODE gainNode[12][16];

  UCHAR uniDrcGainExtPresent;
  UNI_DRC_GAIN_EXTENSION uniDrcGainExtension;

  /* derived data */
  UCHAR status;
} UNI_DRC_GAIN, *HANDLE_UNI_DRC_GAIN;

/****************/
/* uniDrcConfig */
/****************/

typedef enum {
  EB_NIGHT = 0x0001,
  EB_NOISY = 0x0002,
  EB_LIMITED = 0x0004,
  EB_LOWLEVEL = 0x0008,
  EB_DIALOG = 0x0010,
  EB_GENERAL_COMPR = 0x0020,
  EB_EXPAND = 0x0040,
  EB_ARTISTIC = 0x0080,
  EB_CLIPPING = 0x0100,
  EB_FADE = 0x0200,
  EB_DUCK_OTHER = 0x0400,
  EB_DUCK_SELF = 0x0800
} EFFECT_BIT;

typedef enum {
  GCP_REGULAR = 0,
  GCP_FADING = 1,
  GCP_CLIPPING_DUCKING = 2,
  GCP_CONSTANT = 3
} GAIN_CODING_PROFILE;

typedef enum { GIT_SPLINE = 0, GIT_LINEAR = 1 } GAIN_INTERPOLATION_TYPE;

typedef enum { CS_LEFT = 0, CS_RIGHT = 1 } CHARACTERISTIC_SIDE;

typedef enum { CF_SIGMOID = 0, CF_NODES = 1 } CHARACTERISTIC_FORMAT;

typedef enum {
  GF_QMF32 = 0x1,
  GF_QMFHYBRID39 = 0x2,
  GF_QMF64 = 0x3,
  GF_QMFHYBRID71 = 0x4,
  GF_QMF128 = 0x5,
  GF_QMFHYBRID135 = 0x6,
  GF_UNIFORM = 0x7
} EQ_SUBBAND_GAIN_FORMAT;

typedef struct {
  UCHAR duckingScalingPresent;
  FIXP_SGL duckingScaling; /* e = 2 */
} DUCKING_MODIFICATION;

typedef struct {
  UCHAR targetCharacteristicLeftPresent;
  UCHAR targetCharacteristicLeftIndex;
  UCHAR targetCharacteristicRightPresent;
  UCHAR targetCharacteristicRightIndex;
  UCHAR gainScalingPresent;
  FIXP_SGL attenuationScaling;   /* e = 2 */
  FIXP_SGL amplificationScaling; /* e = 2 */
  UCHAR gainOffsetPresent;
  FIXP_SGL gainOffset; /* e = 4 */
} GAIN_MODIFICATION;

typedef union {
  UCHAR crossoverFreqIndex;
  USHORT startSubBandIndex;
} BAND_BORDER;

typedef struct {
  UCHAR left;
  UCHAR right;
} CUSTOM_INDEX;

typedef struct {
  UCHAR present;
  UCHAR isCICP;
  union {
    UCHAR cicpIndex;
    CUSTOM_INDEX custom;
  };
} DRC_CHARACTERISTIC;

typedef struct {
  UCHAR gainCodingProfile;
  UCHAR gainInterpolationType;
  UCHAR fullFrame;
  UCHAR timeAlignment;
  UCHAR timeDeltaMinPresent;
  USHORT timeDeltaMin;
  UCHAR bandCount;
  UCHAR drcBandType;
  UCHAR gainSequenceIndex[4];
  DRC_CHARACTERISTIC drcCharacteristic[4];
  BAND_BORDER bandBorder[4];
} GAIN_SET;

typedef struct {
  FIXP_SGL gain;    /* e = 6 */
  FIXP_SGL ioRatio; /* e = 2 */
  FIXP_SGL exp;     /* e = 5 */
  UCHAR flipSign;
} CUSTOM_DRC_CHAR_SIGMOID;

typedef struct {
  UCHAR characteristicNodeCount;
  FIXP_SGL nodeLevel[4 + 1]; /* e = 7 */
  FIXP_SGL nodeGain[4 + 1];  /* e = 7 */
} CUSTOM_DRC_CHAR_NODES;

typedef shouldBeUnion {
  CUSTOM_DRC_CHAR_SIGMOID sigmoid;
  CUSTOM_DRC_CHAR_NODES nodes;
}
CUSTOM_DRC_CHAR;

/* drcCoefficientsUniDrc() (Table 67) */
typedef struct {
  UCHAR drcLocation;
  UCHAR drcFrameSizePresent;
  USHORT drcFrameSize;
  UCHAR characteristicLeftCount;
  UCHAR characteristicLeftFormat[16];
  CUSTOM_DRC_CHAR customCharacteristicLeft[16];
  UCHAR characteristicRightCount;
  UCHAR characteristicRightFormat[16];
  CUSTOM_DRC_CHAR customCharacteristicRight[16];
  UCHAR
  gainSequenceCount;  /* unsaturated value, i.e. as provided in bitstream */
  UCHAR gainSetCount; /* saturated to 12 */
  GAIN_SET gainSet[12];
  /* derived data */
  UCHAR gainSetIndexForGainSequence[12];
} DRC_COEFFICIENTS_UNI_DRC;

/* drcInstructionsUniDrc() (Table 72) */
typedef struct {
  SCHAR drcSetId;
  UCHAR drcSetComplexityLevel;
  UCHAR drcLocation;
  UCHAR drcApplyToDownmix;
  UCHAR downmixIdCount;
  UCHAR downmixId[8];
  USHORT drcSetEffect;
  UCHAR limiterPeakTargetPresent;
  FIXP_SGL limiterPeakTarget; /* e = 5 */
  UCHAR drcSetTargetLoudnessPresent;
  SCHAR drcSetTargetLoudnessValueUpper;
  SCHAR drcSetTargetLoudnessValueLower;
  UCHAR dependsOnDrcSetPresent;
  union {
    SCHAR dependsOnDrcSet;
    UCHAR noIndependentUse;
  };
  UCHAR requiresEq;
  shouldBeUnion {
    GAIN_MODIFICATION gainModificationForChannelGroup[8][4];
    DUCKING_MODIFICATION duckingModificationForChannel[8];
  };
  SCHAR gainSetIndex[8];

  /* derived data */
  UCHAR drcChannelCount;
  UCHAR nDrcChannelGroups;
  SCHAR gainSetIndexForChannelGroup[8];
} DRC_INSTRUCTIONS_UNI_DRC;

/* channelLayout() (Table 62) */
typedef struct {
  UCHAR baseChannelCount;
  UCHAR layoutSignalingPresent;
  UCHAR definedLayout;
  UCHAR speakerPosition[8];
} CHANNEL_LAYOUT;

/* downmixInstructions() (Table 63) */
typedef struct {
  UCHAR downmixId;
  UCHAR targetChannelCount;
  UCHAR targetLayout;
  UCHAR downmixCoefficientsPresent;
  UCHAR bsDownmixOffset;
  FIXP_DBL downmixCoefficient[8 * 8]; /* e = 2 */
} DOWNMIX_INSTRUCTIONS;

typedef struct {
  UCHAR uniDrcConfigExtType[8];
  ULONG extBitSize[8 - 1];
} UNI_DRC_CONFIG_EXTENSION;

/* uniDrcConfig() (Table 57) */
typedef struct {
  UCHAR sampleRatePresent;
  ULONG sampleRate;
  UCHAR downmixInstructionsCountV0;
  UCHAR downmixInstructionsCountV1;
  UCHAR downmixInstructionsCount; /* saturated to 6 */
  UCHAR drcCoefficientsUniDrcCountV0;
  UCHAR drcCoefficientsUniDrcCountV1;
  UCHAR drcCoefficientsUniDrcCount; /* saturated to 2 */
  UCHAR drcInstructionsUniDrcCountV0;
  UCHAR drcInstructionsUniDrcCountV1;
  UCHAR drcInstructionsUniDrcCount; /* saturated to (12 + 1 + 6) */
  CHANNEL_LAYOUT channelLayout;
  DOWNMIX_INSTRUCTIONS downmixInstructions[6];
  DRC_COEFFICIENTS_UNI_DRC drcCoefficientsUniDrc[2];
  DRC_INSTRUCTIONS_UNI_DRC drcInstructionsUniDrc[(12 + 1 + 6)];
  UCHAR uniDrcConfigExtPresent;
  UNI_DRC_CONFIG_EXTENSION uniDrcConfigExt;

  /* derived data */
  UCHAR drcInstructionsCountInclVirtual;
  UCHAR diff;
} UNI_DRC_CONFIG, *HANDLE_UNI_DRC_CONFIG;

/*******************/
/* loudnessInfoSet */
/*******************/

typedef enum {
  MD_UNKNOWN_OTHER = 0,
  MD_PROGRAM_LOUDNESS = 1,
  MD_ANCHOR_LOUDNESS = 2,
  MD_MAX_OF_LOUDNESS_RANGE = 3,
  MD_MOMENTARY_LOUDNESS_MAX = 4,
  MD_SHORT_TERM_LOUDNESS_MAX = 5,
  MD_LOUDNESS_RANGE = 6,
  MD_MIXING_LEVEL = 7,
  MD_ROOM_TYPE = 8,
  MD_SHORT_TERM_LOUDNESS = 9
} METHOD_DEFINITION;

typedef enum {
  MS_UNKNOWN_OTHER = 0,
  MS_EBU_R_128 = 1,
  MS_BS_1770_4 = 2,
  MS_BS_1770_4_PRE_PROCESSING = 3,
  MS_USER = 4,
  MS_EXPERT_PANEL = 5,
  MS_BS_1771_1 = 6,
  MS_RESERVED_A = 7,
  MS_RESERVED_B = 8,
  MS_RESERVED_C = 9,
  MS_RESERVED_D = 10,
  MS_RESERVED_E = 11
} MEASUREMENT_SYSTEM;

typedef enum {
  R_UKNOWN = 0,
  R_UNVERIFIED = 1,
  R_CEILING = 2,
  R_ACCURATE = 3
} RELIABILITY;

typedef struct {
  UCHAR methodDefinition;
  FIXP_DBL methodValue; /* e = 7 for all methodDefinitions */
  UCHAR measurementSystem;
  UCHAR reliability;
} LOUDNESS_MEASUREMENT;

/* loudnessInfo() (Table 59) */
typedef struct {
  SCHAR drcSetId;
  UCHAR eqSetId;
  UCHAR downmixId;
  UCHAR samplePeakLevelPresent;
  FIXP_DBL samplePeakLevel; /* e = 7 */
  UCHAR truePeakLevelPresent;
  FIXP_DBL truePeakLevel; /* e = 7 */
  UCHAR truePeakLevelMeasurementSystem;
  UCHAR truePeakLevelReliability;
  UCHAR measurementCount; /* saturated to 8 */
  LOUDNESS_MEASUREMENT loudnessMeasurement[8];
} LOUDNESS_INFO;

/* loudnessInfoSetExtension() (Table 61) */
typedef struct {
  UCHAR loudnessInfoSetExtType[8];
  ULONG extBitSize[8 - 1];
} LOUDNESS_INFO_SET_EXTENSION;

/* loudnessInfoSet() (Table 58) */
typedef struct {
  UCHAR loudnessInfoAlbumCountV0;
  UCHAR loudnessInfoAlbumCountV1;
  UCHAR loudnessInfoAlbumCount; /* saturated to 12 */
  UCHAR loudnessInfoCountV0;
  UCHAR loudnessInfoCountV1;
  UCHAR loudnessInfoCount; /* saturated to 12 */
  LOUDNESS_INFO loudnessInfoAlbum[12];
  LOUDNESS_INFO loudnessInfo[12];
  UCHAR loudnessInfoSetExtPresent;
  LOUDNESS_INFO_SET_EXTENSION loudnessInfoSetExt;
  /* derived data */
  UCHAR diff;
} LOUDNESS_INFO_SET, *HANDLE_LOUDNESS_INFO_SET;

#endif
