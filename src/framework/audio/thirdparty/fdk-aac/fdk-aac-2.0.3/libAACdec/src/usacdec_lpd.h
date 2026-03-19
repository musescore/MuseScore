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

/**************************** AAC decoder library ******************************

   Author(s):   Manuel Jander

   Description: USAC Linear Prediction Domain coding

*******************************************************************************/

#ifndef USACDEC_LPD_H
#define USACDEC_LPD_H

#include "channelinfo.h"

#define OPTIMIZE_AVG_PERFORMANCE

/**
 * \brief read a lpd_channel_stream.
 * \param hBs a bit stream handle, where the lpd_channel_stream is located.
 * \param pAacDecoderChannelInfo the channel context structure for storing read
 * data.
 * \param flags bit stream syntax flags.
 * \return AAC_DECODER_ERROR error code.
 */
AAC_DECODER_ERROR CLpdChannelStream_Read(
    HANDLE_FDK_BITSTREAM hBs, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
    const SamplingRateInfo *pSamplingRateInfo, UINT flags);

/**
 * \brief decode one lpd_channel_stream and render the audio output.
 * \param pAacDecoderChannelInfo struct holding the channel information to be
 * rendered.
 * \param pAacDecoderStaticChannelInfo struct holding the persistent channel
 * information to be rendered.
 * \param pSamplingRateInfo holds the sampling rate information
 * \param elFlags holds the internal decoder flags
 */
void CLpdChannelStream_Decode(
    CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo, UINT flags);

/**
 * \brief generate time domain output signal for LPD channel streams
 * \param pAacDecoderStaticChannelInfo
 * \param pAacDecoderChannelInfo
 * \param pTimeData pointer to output buffer
 * \param samplesPerFrame amount of output samples
 * \param pSamplingRateInfo holds the sampling rate information
 * \param aacOutDataHeadroom headroom of the output time signal to prevent
 * clipping
 */
AAC_DECODER_ERROR CLpd_RenderTimeSignal(
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
    CAacDecoderChannelInfo *pAacDecoderChannelInfo, PCM_DEC *pTimeData,
    INT samplesPerFrame, SamplingRateInfo *pSamplingRateInfo, UINT frameOk,
    const INT aacOutDataHeadroom, UINT flags, UINT strmFlags);

static inline INT CLpd_FAC_getLength(int fNotShortBlock, int fac_length_long) {
  if (fNotShortBlock) {
    return (fac_length_long);
  } else {
    return fac_length_long / 2;
  }
}

void filtLP(const FIXP_DBL *syn, PCM_DEC *syn_out, FIXP_DBL *noise,
            const FIXP_SGL *filt, const INT aacOutDataHeadroom, INT stop,
            int len);

/**
 * \brief perform a low-frequency pitch enhancement on time domain signal
 * \param[in] syn pointer to time domain input signal
 * \param[in] synFB pointer to time domain input signal
 * \param[in] upsampling factor
 * \param[in] T_sf array with past decoded pitch period values for each subframe
 * \param[in] non_zero_gain_flags indicates whether pitch gains of past
 * subframes are zero or not, msb -> [1 BPF_DELAY subfr][7 SYN_DELAY subfr][16
 * new subfr] <- lsb
 * \param[in] l_frame length of filtering, must be multiple of L_SUBFR
 * \param[in] l_next length of allowed look ahead on syn[i], i < l_frame+l_next
 * \param[out] synth_out pointer to time domain output signal
 * \param[in] headroom of the output time signal to prevent clipping
 * \param[in,out] mem_bpf pointer to filter memory (L_FILT+L_SUBFR)
 */

void bass_pf_1sf_delay(FIXP_DBL syn[], const INT T_sf[], FIXP_DBL *pit_gain,
                       const int frame_length, const INT l_frame,
                       const INT l_next, PCM_DEC *synth_out,
                       const INT aacOutDataHeadroom, FIXP_DBL mem_bpf[]);

/**
 * \brief random sign generator for FD and TCX noise filling
 * \param[in,out] seed pointer to random seed
 * \return if return value is zero use positive sign
 * \Note: This code is also implemented as a copy in block.cpp, grep for
 * "UsacRandomSign"
 */
FDK_INLINE
int UsacRandomSign(ULONG *seed) {
  *seed = (ULONG)((UINT64)(*seed) * 69069 + 5);

  return (int)((*seed) & 0x10000);
}

void CFdp_Reset(CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo);

#endif /* USACDEC_LPD_H */
