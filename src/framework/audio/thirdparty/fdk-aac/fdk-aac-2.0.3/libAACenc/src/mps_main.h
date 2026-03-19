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

/**************************** AAC encoder library ******************************

   Author(s):   Markus Lohwasser

   Description: Mpeg Surround library interface functions

*******************************************************************************/

#ifndef MPS_MAIN_H
#define MPS_MAIN_H

/* Includes ******************************************************************/
#include "aacenc.h"
#include "FDK_audio.h"
#include "machine_type.h"

/* Defines *******************************************************************/
typedef enum {
  MPS_ENCODER_OK = 0x0000, /*!< No error happened. All fine. */
  MPS_ENCODER_INVALID_HANDLE =
      0x0020, /*!< Handle passed to function call was invalid. */
  MPS_ENCODER_MEMORY_ERROR = 0x0021, /*!< Memory allocation failed. */
  MPS_ENCODER_INIT_ERROR = 0x0040,   /*!< General initialization error. */
  MPS_ENCODER_ENCODE_ERROR =
      0x0060 /*!< The encoding process was interrupted by an unexpected error.
              */

} MPS_ENCODER_ERROR;

/* Data Types ****************************************************************/

/**
 *  MPEG Surround Encoder interface handle.
 */
typedef struct MPS_ENCODER MPS_ENCODER, *HANDLE_MPS_ENCODER;

/* Function / Class Declarations *********************************************/

/**
 * \brief  Open a Mpeg Surround Encoder instance.
 *
 * \phMpsEnc                    A pointer to a MPS handle to be allocated.
 * Initialized on return.
 *
 * \return
 *          - MPS_ENCODER_OK, on succes.
 *          - MPS_ENCODER_INVALID_HANDLE, MPS_ENCODER_MEMORY_ERROR, on failure.
 */
MPS_ENCODER_ERROR FDK_MpegsEnc_Open(HANDLE_MPS_ENCODER *phMpsEnc);

/**
 * \brief  Close the Mpeg Surround Encoder instance.
 *
 * Deallocate instance and free whole memory.
 *
 * \param phMpsEnc              Pointer to the MPS handle to be deallocated.
 *
 * \return
 *          - MPS_ENCODER_OK, on succes.
 *          - MPS_ENCODER_INVALID_HANDLE, on failure.
 */
MPS_ENCODER_ERROR FDK_MpegsEnc_Close(HANDLE_MPS_ENCODER *phMpsEnc);

/**
 * \brief  Initialize a Mpeg Surround Encoder instance.
 *
 * \param hMpsEnc                   MPS Encoder handle.
 * \param audioObjectType           Audio object type.
 * \param samplingrate              Sampling rate in Hz of audio input signal.
 * \param bitrate                   Encder target bitrate.
 * \param sbrRatio                  SBR sampling rate ratio.
 * \param framelength               Number of samples to be processes within one
 * frame.
 * \param inputBufferSizePerChannel Size of input buffer per channel.
 * \param coreCoderDelay            Core coder delay.
 *
 * \return
 *          - MPS_ENCODER_OK, on succes.
 *          - MPS_ENCODER_INVALID_HANDLE, MPS_ENCODER_ENCODE_ERROR, on failure.
 */
MPS_ENCODER_ERROR FDK_MpegsEnc_Init(HANDLE_MPS_ENCODER hMpsEnc,
                                    const AUDIO_OBJECT_TYPE audioObjectType,
                                    const UINT samplingrate, const UINT bitrate,
                                    const UINT sbrRatio, const UINT framelength,
                                    const UINT inputBufferSizePerChannel,
                                    const UINT coreCoderDelay);

/**
 * \brief  Calculate Mpeg Surround processing.
 *
 * This fuction applies the MPS processing. The MPS side info will be written to
 * extension payload. The input audio data will be overwritten by the calculated
 * downmix.
 *
 * \param hMpsEnc               MPS Encoder handle.
 * \param pAudioSamples         Pointer to audio input/output data.
 * \param nAudioSamples         Number of input audio samples to be prcessed.
 * \param pMpsExtPayload        Pointer to extension payload to be filled on
 * return.
 *
 * \return
 *          - MPS_ENCODER_OK, on succes.
 *          - MPS_ENCODER_INVALID_HANDLE, MPS_ENCODER_ENCODE_ERROR, on failure.
 */
MPS_ENCODER_ERROR FDK_MpegsEnc_Process(HANDLE_MPS_ENCODER hMpsEnc,
                                       INT_PCM *const pAudioSamples,
                                       const INT nAudioSamples,
                                       AACENC_EXT_PAYLOAD *pMpsExtPayload);

/**
 * \brief  Write Spatial Specific Config.
 *
 * This function can be called via call back from the transport library to write
 * the Spatial Specific Config to given bitstream buffer.
 *
 * \param hMpsEnc               MPS Encoder handle.
 * \param hBs                   Bitstream buffer handle.
 *
 * \return                      Number of written bits.
 */
INT FDK_MpegsEnc_WriteSpatialSpecificConfig(HANDLE_MPS_ENCODER hMpsEnc,
                                            HANDLE_FDK_BITSTREAM hBs);

/**
 * \brief  Get closest valid bitrate supported by given config.
 *
 * \param audioObjectType       Audio object type.
 * \param channelMode           Encoder channel mode.
 * \param samplingrate          Sampling rate in Hz of audio input signal.
 * \param sbrRatio              SBR sampling rate ratio.
 * \param bitrate               The desired target bitrate.
 *
 * \return                      Closest valid bitrate to given bitrate..
 */
INT FDK_MpegsEnc_GetClosestBitRate(const AUDIO_OBJECT_TYPE audioObjectType,
                                   const CHANNEL_MODE channelMode,
                                   const UINT samplingrate, const UINT sbrRatio,
                                   const UINT bitrate);

/**
 * \brief  Get codec delay.
 *
 * This function returns delay of the whole en-/decoded signal, including
 * corecoder delay.
 *
 * \param hMpsEnc               MPS Encoder handle.
 *
 * \return                      Codec delay in samples.
 */
INT FDK_MpegsEnc_GetDelay(HANDLE_MPS_ENCODER hMpsEnc);

/**
 * \brief  Get Mpeg Surround Decoder delay.
 *
 * This function returns delay of the Mpeg Surround decoder.
 *
 * \param hMpsEnc               MPS Encoder handle.
 *
 * \return                      Mpeg Surround Decoder delay in samples.
 */
INT FDK_MpegsEnc_GetDecDelay(HANDLE_MPS_ENCODER hMpsEnc);

/**
 * \brief  Get information about encoder library build.
 *
 * Fill a given LIB_INFO structure with library version information.
 *
 * \param info  Pointer to an allocated LIB_INFO struct.
 *
 * \return
 *          - MPS_ENCODER_OK, on succes.
 *          - MPS_ENCODER_INVALID_HANDLE, MPS_ENCODER_INIT_ERROR, on failure.
 */
MPS_ENCODER_ERROR FDK_MpegsEnc_GetLibInfo(LIB_INFO *info);

#endif /* MPS_MAIN_H */
