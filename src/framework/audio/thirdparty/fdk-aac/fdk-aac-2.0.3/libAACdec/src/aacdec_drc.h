/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** AAC decoder library ******************************

   Author(s):   Christian Griebel

   Description: Dynamic range control (DRC) decoder tool for AAC

*******************************************************************************/

#ifndef AACDEC_DRC_H
#define AACDEC_DRC_H

#include "tp_data.h" /* for program config element support */

#include "aacdec_drc_types.h"
#include "channel.h"
#include "FDK_bitstream.h"

#define AACDEC_DRC_GAIN_SCALING (11) /* Scaling of DRC gains */
#define AACDEC_DRC_GAIN_INIT_VALUE \
  (FL2FXCONST_DBL(                 \
      1.0f / (1 << AACDEC_DRC_GAIN_SCALING))) /* Init value for DRC gains */

#define AACDEC_DRC_DFLT_EXPIRY_FRAMES \
  (0) /* Default DRC data expiry time in AAC frames   */

/* #define AACDEC_DRC_IGNORE_FRAMES_WITH_MULTIPLE_CH_THREADS */ /* The name says
                                                                   it all. */
/* #define AACDEC_DRC_DEBUG */

/**
 * \brief DRC module setting parameters
 */
typedef enum {
  DRC_CUT_SCALE = 0,
  DRC_BOOST_SCALE,
  TARGET_REF_LEVEL,
  DRC_BS_DELAY,
  DRC_DATA_EXPIRY_FRAME,
  APPLY_HEAVY_COMPRESSION,
  DEFAULT_PRESENTATION_MODE,
  ENCODER_TARGET_LEVEL,
  MAX_OUTPUT_CHANNELS
} AACDEC_DRC_PARAM;

/**
 * \brief DRC module interface functions
 */
void aacDecoder_drcDisable(HANDLE_AAC_DRC self);

void aacDecoder_drcReset(HANDLE_AAC_DRC self);

void aacDecoder_drcInit(HANDLE_AAC_DRC self);

void aacDecoder_drcInitChannelData(CDrcChannelData *pDrcChannel);

AAC_DECODER_ERROR aacDecoder_drcSetParam(HANDLE_AAC_DRC self,
                                         AACDEC_DRC_PARAM param, INT value);

int aacDecoder_drcMarkPayload(HANDLE_AAC_DRC self, HANDLE_FDK_BITSTREAM hBs,
                              AACDEC_DRC_PAYLOAD_TYPE type);

int aacDecoder_drcProlog(
    HANDLE_AAC_DRC self, HANDLE_FDK_BITSTREAM hBs,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo[],
    UCHAR pceInstanceTag, UCHAR channelMapping[], int validChannels);

/**
 * \brief Apply DRC. If SBR is present, DRC data is handed over to the SBR
 * decoder.
 * \param self AAC decoder instance
 * \param pSbrDec pointer to SBR decoder instance
 * \param pAacDecoderChannelInfo AAC decoder channel instance to be processed
 * \param pDrcDat DRC channel data
 * \param extGain Pointer to a FIXP_DBL where a externally applyable gain will
 * be stored into (independently on whether it will be apply internally or not).
 *                At function call the buffer must hold the scale (0 >= scale <
 * DFRACT_BITS) to be applied on the gain value.
 * \param ch channel index
 * \param aacFrameSize AAC frame size
 * \param bSbrPresent flag indicating that SBR is present, in which case DRC is
 * handed over to the SBR instance pSbrDec
 */
void aacDecoder_drcApply(HANDLE_AAC_DRC self, void *pSbrDec,
                         CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                         CDrcChannelData *pDrcDat, FIXP_DBL *extGain, int ch,
                         int aacFrameSize, int bSbrPresent);

int aacDecoder_drcEpilog(
    HANDLE_AAC_DRC self, HANDLE_FDK_BITSTREAM hBs,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo[],
    UCHAR pceInstanceTag, UCHAR channelMapping[], int validChannels);

/**
 * \brief Get metadata information found in bitstream.
 * \param self DRC module instance handle.
 * \param pPresMode Pointer to field where the presentation mode will be written
 * to.
 * \param pProgRefLevel Pointer to field where the program reference level will
 * be written to.
 * \return Nothing.
 */
void aacDecoder_drcGetInfo(HANDLE_AAC_DRC self, SCHAR *pPresMode,
                           SCHAR *pProgRefLevel);

/**
 * \brief  Apply DRC Level Normalization.
 *
 *         This function prepares/applies the gain values for the DRC Level
 * Normalization and returns the exponent of the time data. The following two
 * cases are handled:
 *
 *         - Limiter enabled:
 *           The input data must be interleaved.
 *           One gain per sample is written to the buffer pGainPerSample.
 *           If necessary the time data is rescaled.
 *
 *         - Limiter disabled:
 *           The input data can be interleaved or deinterleaved.
 *           The gain values are applied to the time data.
 *           If necessary the time data is rescaled.
 *
 * \param hDrcInfo                     [i/o] handle to drc data structure.
 * \param samplesIn                    [i/o] pointer to time data.
 * \param pGain                        [i  ] pointer to gain to be applied to
 * the time data.
 * \param pGainPerSample               [o  ] pointer to the gain per sample to
 * be applied to the time data in the limiter.
 * \param gain_scale                   [i  ] exponent to be applied to the time
 * data.
 * \param gain_delay                   [i  ] delay[samples] with which the gains
 * in pGain shall be applied (gain_delay <= nSamples).
 * \param nSamples                     [i  ] number of samples per frame.
 * \param channels                     [i  ] number of channels.
 * \param stride                       [i  ] channel stride of time data.
 * \param limiterEnabled               [i  ] 1 if limiter is enabled, otherwise
 * 0.
 *
 * \return exponent of time data
 */
INT applyDrcLevelNormalization(HANDLE_AAC_DRC hDrcInfo, PCM_DEC *samplesIn,
                               FIXP_DBL *pGain, FIXP_DBL *pGainPerSample,
                               const INT gain_scale, const UINT gain_delay,
                               const UINT nSamples, const UINT channels,
                               const UINT stride, const UINT limiterEnabled);

#endif /* AACDEC_DRC_H */
