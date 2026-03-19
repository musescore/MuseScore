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

   Author(s):   V. Bacigalupo

   Description: Metadata Encoder library interface functions

*******************************************************************************/

#ifndef METADATA_MAIN_H
#define METADATA_MAIN_H

/* Includes ******************************************************************/
#include "aacenc_lib.h"
#include "aacenc.h"

/* Defines *******************************************************************/

/* Data Types ****************************************************************/

typedef enum {
  METADATA_OK = 0x0000, /*!< No error happened. All fine. */
  METADATA_INVALID_HANDLE =
      0x0020, /*!< Handle passed to function call was invalid. */
  METADATA_MEMORY_ERROR = 0x0021, /*!< Memory allocation failed. */
  METADATA_INIT_ERROR = 0x0040,   /*!< General initialization error. */
  METADATA_ENCODE_ERROR =
      0x0060 /*!< The encoding process was interrupted by an unexpected error.
              */

} FDK_METADATA_ERROR;

/**
 *  Meta Data handle.
 */
typedef struct FDK_METADATA_ENCODER *HANDLE_FDK_METADATA_ENCODER;

/**
 * \brief  Open a Meta Data instance.
 *
 * \param phMetadataEnc         A pointer to a Meta Data handle to be allocated.
 * Initialized on return.
 * \param maxChannels           Maximum number of supported audio channels.
 *
 * \return
 *          - METADATA_OK, on succes.
 *          - METADATA_INVALID_HANDLE, METADATA_MEMORY_ERROR, on failure.
 */
FDK_METADATA_ERROR FDK_MetadataEnc_Open(
    HANDLE_FDK_METADATA_ENCODER *phMetadataEnc, const UINT maxChannels);

/**
 * \brief  Initialize a Meta Data instance.
 *
 * \param hMetadataEnc          Meta Data handle.
 * \param resetStates           Indication for full reset of all states.
 * \param metadataMode          Configures meta data output format (0,1,2,3).
 * \param audioDelay            Delay cause by the audio encoder.
 * \param frameLength           Number of samples to be processes within one
 * frame.
 * \param sampleRate            Sampling rat in Hz of audio input signal.
 * \param nChannels             Number of audio input channels.
 * \param channelMode           Channel configuration which is used by the
 * encoder.
 * \param channelOrder          Channel order of the input data. (WAV, MPEG)
 *
 * \return
 *          - METADATA_OK, on succes.
 *          - METADATA_INVALID_HANDLE, METADATA_INIT_ERROR, on failure.
 */
FDK_METADATA_ERROR FDK_MetadataEnc_Init(
    HANDLE_FDK_METADATA_ENCODER hMetadataEnc, const INT resetStates,
    const INT metadataMode, const INT audioDelay, const UINT frameLength,
    const UINT sampleRate, const UINT nChannels, const CHANNEL_MODE channelMode,
    const CHANNEL_ORDER channelOrder);

/**
 * \brief  Calculate Meta Data processing.
 *
 * This function treats all step necessary for meta data processing.
 * - Receive new meta data and make usable.
 * - Calculate DRC compressor and extract meta data info.
 * - Make meta data available for extern use.
 * - Apply audio data and meta data delay compensation.
 *
 * \param hMetadataEnc          Meta Data handle.
 * \param pAudioSamples         Pointer to audio input data. Existing function
 * overwrites audio data with delayed audio samples.
 * \param nAudioSamples         Number of input audio samples to be prcessed.
 * \param pMetadata             Pointer to Metat Data input.
 * \param ppMetaDataExtPayload  Pointer to extension payload array. Filled on
 * return.
 * \param nMetaDataExtensions   Pointer to variable to describe number of
 * available extension payloads. Filled on return.
 * \param matrix_mixdown_idx    Pointer to variable for matrix mixdown
 * coefficient. Filled on return.
 *
 * \return
 *          - METADATA_OK, on succes.
 *          - METADATA_INVALID_HANDLE, METADATA_ENCODE_ERROR, on failure.
 */
FDK_METADATA_ERROR FDK_MetadataEnc_Process(
    HANDLE_FDK_METADATA_ENCODER hMetadataEnc, INT_PCM *const pAudioSamples,
    const UINT audioSamplesBufSize, const INT nAudioSamples,
    const AACENC_MetaData *const pMetadata,
    AACENC_EXT_PAYLOAD **ppMetaDataExtPayload, UINT *nMetaDataExtensions,
    INT *matrix_mixdown_idx);

/**
 * \brief  Close the Meta Data instance.
 *
 * Deallocate instance and free whole memory.
 *
 * \param phMetaData            Pointer to the Meta Data handle to be
 * deallocated.
 *
 * \return
 *          - METADATA_OK, on succes.
 *          - METADATA_INVALID_HANDLE, on failure.
 */
FDK_METADATA_ERROR FDK_MetadataEnc_Close(
    HANDLE_FDK_METADATA_ENCODER *phMetaData);

/**
 * \brief  Get Meta Data Encoder delay.
 *
 * \param hMetadataEnc          Meta Data Encoder handle.
 *
 * \return  Delay caused by Meta Data module.
 */
INT FDK_MetadataEnc_GetDelay(HANDLE_FDK_METADATA_ENCODER hMetadataEnc);

#endif /* METADATA_MAIN_H */
