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

   Author(s):   M. Werner

   Description: Bitstream encoder

*******************************************************************************/

#ifndef BITENC_H
#define BITENC_H

#include "qc_data.h"
#include "aacenc_tns.h"
#include "channel_map.h"
#include "interface.h" /* obsolete, when PSY_OUT is thrown out of the WritBS-call! */
#include "FDK_audio.h"
#include "aacenc.h"

#include "tpenc_lib.h"

typedef enum {
  MAX_ENCODER_CHANNELS = 9,
  MAX_BLOCK_TYPES = 4,
  MAX_AAC_LAYERS = 9,
  MAX_LAYERS = MAX_AAC_LAYERS, /* only one core layer if present */
  FIRST_LAY = 1                /* default layer number for AAC nonscalable */
} _MAX_CONST;

#define BUFFER_MX_HUFFCB_SIZE \
  (32 * sizeof(INT)) /* our FDK_bitbuffer needs size of power 2 */

#define EL_ID_BITS (3)

/**
 * \brief Arbitrary order bitstream writer. This function can either assemble a
 * bit stream and write into the bit buffer of hTpEnc or calculate the number of
 * static bits (signal independent) TpEnc handle must be NULL in this
 * case. Or also Calculate the minimum possible number of static bits
 * which by disabling all tools e.g. MS, TNS and sbfCnt=0. The minCnt
 * parameter has to be 1 in this latter case.
 * \param hTpEnc Transport encoder handle. If NULL, the number of static bits
 * will be returned into *pBitDemand.
 * \param pElInfo
 * \param qcOutChannel
 * \param hReorderInfo
 * \param psyOutElement
 * \param psyOutChannel
 * \param syntaxFlags Bit stream syntax flags as defined in FDK_audio.h (Audio
 * Codec flags).
 * \param aot
 * \param epConfig
 * \param pBitDemand Pointer to an int where the amount of bits is returned
 * into. The returned value depends on if hTpEnc is NULL and minCnt.
 * \param minCnt If non-zero the value returned into *pBitDemand is the absolute
 * minimum required amount of static bits in order to write a valid bit stream.
 * \return AAC_ENCODER_ERROR error code
 */
AAC_ENCODER_ERROR FDKaacEnc_ChannelElementWrite(
    HANDLE_TRANSPORTENC hTpEnc, ELEMENT_INFO *pElInfo,
    QC_OUT_CHANNEL *qcOutChannel[(2)], PSY_OUT_ELEMENT *psyOutElement,
    PSY_OUT_CHANNEL *psyOutChannel[(2)], UINT syntaxFlags,
    AUDIO_OBJECT_TYPE aot, SCHAR epConfig, INT *pBitDemand, UCHAR minCnt);
/**
 * \brief Write bit stream or account static bits
 * \param hTpEnc transport encoder handle. If NULL, the function will
 *        not write any bit stream data but only count the amount
 *        of static (signal independent) bits
 * \param channelMapping Channel mapping info
 * \param qcOut
 * \param psyOut
 * \param qcKernel
 * \param hBSE
 * \param aot Audio Object Type being encoded
 * \param syntaxFlags Flags indicating format specific detail
 * \param epConfig Error protection config
 */
AAC_ENCODER_ERROR FDKaacEnc_WriteBitstream(HANDLE_TRANSPORTENC hTpEnc,
                                           CHANNEL_MAPPING *channelMapping,
                                           QC_OUT *qcOut, PSY_OUT *psyOut,
                                           QC_STATE *qcKernel,
                                           AUDIO_OBJECT_TYPE aot,
                                           UINT syntaxFlags, SCHAR epConfig);

INT FDKaacEnc_writeExtensionData(HANDLE_TRANSPORTENC hTpEnc,
                                 QC_OUT_EXTENSION *pExtension,
                                 INT elInstanceTag, UINT alignAnchor,
                                 UINT syntaxFlags, AUDIO_OBJECT_TYPE aot,
                                 SCHAR epConfig);

#endif /* BITENC_H */
