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

/*********************** MPEG surround encoder library *************************

   Author(s):   Max Neuendorf

   Description: Encoder Library Interface
                Encoder API

*******************************************************************************/

/**************************************************************************/ /**
   \file
 ******************************************************************************/

#ifndef SACENC_LIB_H
#define SACENC_LIB_H

/* Includes ******************************************************************/
#include "machine_type.h"
#include "FDK_audio.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Defines *******************************************************************/

/* Data Types ****************************************************************/

/**
 *  Space encoder error codes.
 */
typedef enum {
  SACENC_OK = 0x00000000, /*!< No error happened. All fine. */
  SACENC_INVALID_HANDLE =
      0x00000080, /*!< Handle passed to function call was invalid. */
  SACENC_MEMORY_ERROR = 0x00000800, /*!< Memory allocation failed. */
  SACENC_INIT_ERROR = 0x00008000,   /*!< General initialization error. */
  SACENC_ENCODE_ERROR =
      0x00080000, /*!< The encoding process was interrupted by an unexpected
                     error. */
  SACENC_PARAM_ERROR = 0x00800000,           /*!< Invalid runtime parameter. */
  SACENC_UNSUPPORTED_PARAMETER = 0x00800001, /*!< Parameter not available. */
  SACENC_INVALID_CONFIG = 0x00800002,        /*!< Configuration not provided. */
  SACENC_UNKNOWN_ERROR = 0x08000000          /*!< Unknown error. */

} FDK_SACENC_ERROR;

typedef enum {
  SACENC_INVALID_MODE = 0,
  SACENC_212 = 8,
  SACENC_ESCAPE = 15

} MP4SPACEENC_MODE;

typedef enum {
  SACENC_BANDS_INVALID = 0,
  SACENC_BANDS_4 = 4,
  SACENC_BANDS_5 = 5,
  SACENC_BANDS_7 = 7,
  SACENC_BANDS_9 = 9,
  SACENC_BANDS_12 = 12,
  SACENC_BANDS_15 = 15,
  SACENC_BANDS_23 = 23

} MP4SPACEENC_BANDS_CONFIG;

typedef enum {
  SACENC_QUANTMODE_INVALID = -1,
  SACENC_QUANTMODE_FINE = 0,
  SACENC_QUANTMODE_EBQ1 = 1,
  SACENC_QUANTMODE_EBQ2 = 2,
  SACENC_QUANTMODE_RSVD3 = 3

} MP4SPACEENC_QUANTMODE;

typedef enum {
  SACENC_DMXGAIN_INVALID = -1,
  SACENC_DMXGAIN_0_dB = 0,
  SACENC_DMXGAIN_1_5_dB = 1,
  SACENC_DMXGAIN_3_dB = 2,
  SACENC_DMXGAIN_4_5_dB = 3,
  SACENC_DMXGAIN_6_dB = 4,
  SACENC_DMXGAIN_7_5_dB = 5,
  SACENC_DMXGAIN_9_dB = 6,
  SACENC_DMXGAIN_12_dB = 7

} MP4SPACEENC_DMX_GAIN;

/**
 * \brief  Space Encoder setting parameters.
 *
 * Use FDK_sacenc_setParam() function to configure the internal status of the
 * following parameters.
 */
typedef enum {
  SACENC_LOWDELAY, /*!< Configure lowdelay MPEG Surround.
                        - 0: Disable Lowdelay. (default)
                        - 1: Enable Lowdelay.
                        - 2: Enable Lowdelay including keep frame. */

  SACENC_ENC_MODE, /*!< Configure encoder tree mode. See ::MP4SPACEENC_MODE for
                      available values. */

  SACENC_SAMPLERATE, /*!< Configure encoder sampling rate. */

  SACENC_FRAME_TIME_SLOTS, /*!< Configure number of slots per spatial frame. */

  SACENC_PARAM_BANDS, /*!< Configure number of parameter bands. See
                         ::MP4SPACEENC_BANDS_CONFIG for available values. */

  SACENC_TIME_DOM_DMX, /*!< Configure time domain downmix.
                            - 0: No time domain downmix. (default)
                            - 1: Static time domain downmix.
                            - 2: Enhanced time domain downmix, stereo to mono
                          only. */

  SACENC_DMX_GAIN, /*!< Configure downmix gain. See ::MP4SPACEENC_DMX_GAIN for
                      available values. */

  SACENC_COARSE_QUANT, /*!< Use coarse parameter quantization.
                            - 0: No (default)
                            - 1: Yes */

  SACENC_QUANT_MODE, /*!< Configure quanitzation mode. See
                        ::MP4SPACEENC_QUANTMODE for available values. */

  SACENC_TIME_ALIGNMENT, /*!< Configure time alignment in samples. */

  SACENC_INDEPENDENCY_COUNT, /*!< Configure the independency count. (count == 0
                                means independencyFlag == 1) */

  SACENC_INDEPENDENCY_FACTOR, /*!< How often should we set the independency flag
                               */

  SACENC_NONE /*!< ------ */

} SPACEENC_PARAM;

/**
 *  Describes Spatial Specific Config.
 */
typedef struct {
  INT nSscSizeBits; /*!< Number of valid bits in pSsc buffer. */
  UCHAR *pSsc;      /*!< SpatialSpecificConfig buffer in binary format. */

} MPEG4SPACEENC_SSCBUF;

/**
 *  Provides some info about the encoder configuration.
 */
typedef struct {
  INT nSampleRate;         /*!< Configured sampling rate.*/
  INT nSamplesFrame;       /*!< Frame length in samples. */
  INT nTotalInputChannels; /*!< Number of expected audio input channels. */
  INT nDmxDelay;           /*!< Delay of the downmixed signal. */
  INT nCodecDelay;         /*!< Delay of the whole en-/decoded signal, including
                              core-coder delay. */
  INT nDecoderDelay;       /*!< Delay added by the MP4SPACE decoder. */
  INT nPayloadDelay;       /*!< Delay of the payload. */
  INT nDiscardOutFrames; /*!< Number of dmx frames to discard for alignment with
                            bitstream. */

  MPEG4SPACEENC_SSCBUF
  *pSscBuf; /*!< Pointer to Spatial Specific Config structure. */

} MP4SPACEENC_INFO;

/**
 *  MPEG Surround encoder handle.
 */
typedef struct MP4SPACE_ENCODER *HANDLE_MP4SPACE_ENCODER;

/**
 *  Defines the input arguments for a FDK_sacenc_encode() call.
 */
typedef struct {
  INT nInputSamples; /*!< Number of valid input audio samples (multiple of input
                        channels). */
  UINT inputBufferSizePerChannel; /*!< Size of input buffer (input audio
                                     samples) per channel. */
  UINT isInputInterleaved; /*!< Indicates if input audio samples are represented
                              in blocks or interleaved:
                                - 0 : in blocks.
                                - 1 : interleaved. */

} SACENC_InArgs;

/**
 *  Defines the output arguments for a FDK_sacenc_encode() call.
 */
typedef struct {
  INT nOutputBits;    /*!< Number of valid payload bits generated during
                         FDK_sacenc_encode(). */
  INT nOutputSamples; /*!< Number of valid output audio samples generated during
                         FDK_sacenc_encode(). */
  UINT nSamplesConsumed; /*!< Number of input audio samples consumed in
                            FDK_sacenc_encode(). */

} SACENC_OutArgs;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/**
 * \brief  Opens a new instace of the MPEG Surround encoder.
 *
 * \param phMp4SpaceEnc      Pointer to the encoder handle to be deallocated.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, SACENC_MEMORY_ERROR, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_open(HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc);

/**
 * \brief  Finalizes opening process of MPEG Surround encoder.
 *
 * Shows, how many samples are needed as input
 *
 * \param hMp4SpaceEnc       A valid MPEG Surround encoder handle.
 * \param dmxDelay           Downmix delay.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, SACENC_INIT_ERROR, SACENC_INVALID_CONFIG,
 * on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_init(HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                 const INT dmxDelay);

/**
 * \brief  Close the MPEG Surround encoder instance.
 *
 * Deallocate encoder instance and free whole memory.
 *
 * \param phMp4SpaceEnc      Pointer to the encoder handle to be deallocated.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_close(HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc);

/**
 * \brief  MPEG surround parameter extraction, framwise.
 *
 * \param hMp4SpaceEnc       A valid MPEG Surround encoder handle.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_encode(const HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                   const FDK_bufDescr *inBufDesc,
                                   const FDK_bufDescr *outBufDesc,
                                   const SACENC_InArgs *inargs,
                                   SACENC_OutArgs *outargs);

/**
 * \brief  Provides information on produced bitstream.
 *
 * \param hMp4SpaceEnc       A valid MPEG Surround encoder handle.
 * \param pInfo              Pointer to an encoder info struct, filled on
 * return.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_getInfo(const HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                    MP4SPACEENC_INFO *const pInfo);

/**
 * \brief  Set one single MPEG Surround encoder parameter.
 *
 * This function allows configuration of all encoder parameters specified in
 * ::SPACEENC_PARAM. Each parameter must be set with a separate function call.
 * An internal validation of the configuration value range will be done.
 *
 * \param hMp4SpaceEnc       A valid MPEG Surround encoder handle.
 * \param param              Parameter to be set. See ::SPACEENC_PARAM.
 * \param value              Parameter value. See parameter description in
 * ::SPACEENC_PARAM.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, SACENC_UNSUPPORTED_PARAMETER,
 * SACENC_INVALID_CONFIG, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_setParam(HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                     const SPACEENC_PARAM param,
                                     const UINT value);

/**
 * \brief  Get information about MPEG Surround encoder library build.
 *
 * Fill a given LIB_INFO structure with library version information.
 *
 * \param info               Pointer to an allocated LIB_INFO struct.
 *
 * \return
 *          - SACENC_OK, on success.
 *          - SACENC_INVALID_HANDLE, SACENC_INIT_ERROR, on failure.
 */
FDK_SACENC_ERROR FDK_sacenc_getLibInfo(LIB_INFO *info);

#ifdef __cplusplus
}
#endif

#endif /* SACENC_LIB_H */
