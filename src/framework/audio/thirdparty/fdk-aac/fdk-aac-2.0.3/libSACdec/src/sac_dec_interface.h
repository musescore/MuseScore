/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Description: SAC Decoder Library Interface

*******************************************************************************/

#ifndef SAC_DEC_INTERFACE_H
#define SAC_DEC_INTERFACE_H

#include "common_fix.h"
#include "FDK_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "sac_dec_errorcodes.h"
#include "sac_dec_ssc_struct.h"

/**
 * \brief  Baseline MPEG-Surround profile Level 1-5.
 */
typedef enum {
  DECODER_LEVEL_0 = 0, /*!< Level 0: dummy level; 212 only */
  DECODER_LEVEL_6 = 6  /*!< Level 6: no support */
} CFG_LEVEL;

/*
 * \brief  Number of output channels restriction.
 */
typedef enum {
  OUTPUT_CHANNELS_DEFAULT, /*!< Default configuration depending on Decoder Level
                            */
  OUTPUT_CHANNELS_2_0,     /*!< Limitation to stereo output */
  OUTPUT_CHANNELS_5_1      /*!< Limitation to 5.1 output */
} CFG_RESTRICTION;

/*
 * \brief  Supported decoder mode.
 */
typedef enum {
  EXT_HQ_ONLY = 0,  /*!< High Quality processing only */
  EXT_LP_ONLY = 1,  /*!< Low Power procesing only */
  EXT_HQ_AND_LP = 2 /*!< Support both HQ and LP processing */
} CFG_EXTENT;

/*
 * \brief  Supported binaural mode.
 */
typedef enum {
  BINAURAL_NONE = -1 /*!< No binaural procesing supported */
} CFG_BINAURAL;

/**
 * \brief  Decoder configuration structure.
 *
 * These structure contains all parameters necessary for decoder open function.
 * The configuration specifies the functional range of the decoder instance.
 */
typedef struct {
  CFG_LEVEL decoderLevel;
  CFG_EXTENT decoderMode;
  CFG_RESTRICTION maxNumOutputChannels;
  CFG_BINAURAL binauralMode;

} SPATIAL_DEC_CONFIG;

typedef enum {
  INPUTMODE_QMF = 1000,
  INPUTMODE_QMF_SBR = 1001,
  INPUTMODE_TIME = 1002
} SPATIALDEC_INPUT_MODE;

/**
 * \brief  MPEG Surround upmix type mode.
 **/
typedef enum {
  UPMIX_TYPE_BYPASS =
      -1, /*!< Bypass the downmix channels from the core decoder.    */
  UPMIX_TYPE_NORMAL = 0 /*!< Multi channel output. */

} SPATIAL_DEC_UPMIX_TYPE;

/**
 * \brief  Dynamic decoder parameters.
 */
typedef struct {
  /* Basics */
  UCHAR outputMode;
  UCHAR blindEnable;
  UCHAR bypassMode;

  /* Error concealment */
  UCHAR concealMethod;
  UINT concealNumKeepFrames;
  UINT concealFadeOutSlopeLength;
  UINT concealFadeInSlopeLength;
  UINT concealNumReleaseFrames;

} SPATIALDEC_PARAM;

/**
 * \brief Flags which control the initialization
 **/
typedef enum {
  MPEGS_INIT_NONE = 0x00000000, /*!< Indicates no initialization */

  MPEGS_INIT_CONFIG = 0x00000010, /*!< Indicates a configuration change due to
                                     SSC value changes */

  MPEGS_INIT_STATES_ANA_QMF_FILTER =
      0x00000100, /*!< Controls the initialization of the analysis qmf filter
                     states */
  MPEGS_INIT_STATES_SYN_QMF_FILTER =
      0x00000200, /*!< Controls the initialization of the synthesis qmf filter
                     states */
  MPEGS_INIT_STATES_ANA_HYB_FILTER = 0x00000400, /*!< Controls the
                                                    initialization of the
                                                    analysis hybrid filter
                                                    states */
  MPEGS_INIT_STATES_DECORRELATOR =
      0x00000800, /*!< Controls the initialization of the decorrelator states */
  MPEGS_INIT_STATES_M1M2 = 0x00002000, /*!< Controls the initialization of the
                                          history in m1 and m2 parameter
                                          calculation */
  MPEGS_INIT_STATES_GES = 0x00004000,  /*!< Controls the initialization of the
                                          history in the ges calculation */
  MPEGS_INIT_STATES_REVERB =
      0x00008000, /*!< Controls the initialization of the reverb states */
  MPEGS_INIT_STATES_PARAM =
      0x00020000, /*!< Controls the initialization of the history of all other
                     parameter */
  MPEGS_INIT_STATES_ERROR_CONCEALMENT =
      0x00080000, /*!< Controls the initialization of the error concealment
                     module state */
  MPEGS_INIT_PARAMS_ERROR_CONCEALMENT = 0x00200000 /*!< Controls the
                                                      initialization of the
                                                      whole error concealment
                                                      parameter set */

} MPEGS_INIT_CTRL_FLAGS;

#define MASK_MPEGS_INIT_ALL_STATES (0x000FFF00)
#define MASK_MPEGS_INIT_ALL_PARAMS (0x00F00000)

typedef struct spatialDec_struct spatialDec, *HANDLE_SPATIAL_DEC;

typedef struct SPATIAL_BS_FRAME_struct SPATIAL_BS_FRAME;

typedef struct {
  UINT sizePersistent;     /* persistent memory */
  UINT sizeFastPersistent; /* fast persistent memory */

} MEM_REQUIREMENTS;

#define PCM_MPS LONG
#define PCM_MPSF FIXP_DBL

#define FIXP_DBL2PCM_MPS(x) ((LONG)(x))

/* exposed functions (library interface) */

int FDK_SpatialDecCompareSpatialSpecificConfigHeader(
    SPATIAL_SPECIFIC_CONFIG *pSsc1, SPATIAL_SPECIFIC_CONFIG *pSsc2);

int FDK_SpatialDecInitDefaultSpatialSpecificConfig(
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
    AUDIO_OBJECT_TYPE coreCodec, int coreChannels, int samplingFreq,
    int nTimeSlots, int decoderLevel, int isBlind);

spatialDec *FDK_SpatialDecOpen(const SPATIAL_DEC_CONFIG *config,
                               int stereoConfigIndex);

/**
 * \brief Initialize state variables of the MPS parser
 */
void SpatialDecInitParserContext(spatialDec *self);

/**
 * \brief Initialize state of MPS decoder. This may happen after the first parse
 * operation.
 */
SACDEC_ERROR FDK_SpatialDecInit(spatialDec *self, SPATIAL_BS_FRAME *frame,
                                SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
                                int nQmfBands,
                                SPATIAL_DEC_UPMIX_TYPE const upmixType,
                                SPATIALDEC_PARAM *pUserParams,
                                UINT initFlags /* MPEGS_INIT_CTRL_FLAGS */
);

/**
 * \brief Apply decoded MPEG Surround parameters to time domain or QMF down mix
 * data.
 * \param self spatial decoder handle.
 * \param inData Pointer to time domain input down mix data if any.
 * \param qmfInDataReal Pointer array of QMF domain down mix input data (real
 * part).
 * \param qmfInDataImag Pointer array of QMF domain down mix input data
 * (imaginary part).
 * \param pcmOutBuf Pointer to a time domain buffer were the upmixed output data
 * will be stored into.
 * \param nSamples Amount of audio samples per channel of down mix input data
 * (frame length).
 * \param pControlFlags pointer to control flags field; input/output.
 * \param numInputChannels amount of down mix input channels. Might not match
 * the current tree config, useful for internal sanity checks and bypass mode.
 * \param channelMapping array containing the desired output channel ordering to
 * transform MPEG PCE style ordering to any other channel ordering. First
 * dimension is the total channel count.
 */
SACDEC_ERROR SpatialDecApplyFrame(
    spatialDec *self, SPATIAL_BS_FRAME *frame, SPATIALDEC_INPUT_MODE inputMode,
    PCM_MPS *inData,          /* Time domain input  */
    FIXP_DBL **qmfInDataReal, /* interleaved l/r */
    FIXP_DBL **qmfInDataImag, /* interleaved l/r */
    PCM_MPS *pcmOutBuf, /* MAX_OUTPUT_CHANNELS*MAX_TIME_SLOTS*NUM_QMF_BANDS] */
    UINT nSamples, UINT *pControlFlags, int numInputChannels,
    const FDK_channelMapDescr *const mapDescr);

/**
 * \brief Fill given arrays with audio channel types and indices.
 * \param self spatial decoder handle.
 * \param channelType array where corresponding channel types fr each output
 * channels are stored into.
 * \param channelIndices array where corresponding channel type indices fr each
 * output channels are stored into.
 */
void SpatialDecChannelProperties(spatialDec *self,
                                 AUDIO_CHANNEL_TYPE channelType[],
                                 UCHAR channelIndices[],
                                 const FDK_channelMapDescr *const mapDescr);

void FDK_SpatialDecClose(spatialDec *self);

#ifdef __cplusplus
}
#endif

#endif /* SAC_DEC_INTERFACE_H */
