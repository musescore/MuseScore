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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief Memory layout
  $Revision: 92864 $

  This module declares all static and dynamic memory spaces
*/
#include "sbrenc_ram.h"

#include "sbr.h"
#include "genericStds.h"

C_AALLOC_MEM(Ram_SbrDynamic_RAM, FIXP_DBL,
             ((SBR_ENC_DYN_RAM_SIZE) / sizeof(FIXP_DBL)))

/*!
  \name StaticSbrData

  Static memory areas, must not be overwritten in other sections of the encoder
*/
/* @{ */

/*! static sbr encoder instance for one encoder (2 channels)
  all major static and dynamic memory areas are located
  in module sbr_ram and sbr rom
*/
C_ALLOC_MEM(Ram_SbrEncoder, SBR_ENCODER, 1)
C_ALLOC_MEM2(Ram_SbrChannel, SBR_CHANNEL, 1, (8))
C_ALLOC_MEM2(Ram_SbrElement, SBR_ELEMENT, 1, (8))

/*! Filter states for QMF-analysis. <br>
  Dimension: #MAXNRSBRCHANNELS * #SBR_QMF_FILTER_LENGTH
*/
C_AALLOC_MEM2_L(Ram_Sbr_QmfStatesAnalysis, FIXP_QAS, 640, (8), SECT_DATA_L1)

/*! Matrix holding the quota values for all estimates, all channels
  Dimension #MAXNRSBRCHANNELS * +#SBR_QMF_CHANNELS* #MAX_NO_OF_ESTIMATES
*/
C_ALLOC_MEM2_L(Ram_Sbr_quotaMatrix, FIXP_DBL, (MAX_NO_OF_ESTIMATES * 64), (8),
               SECT_DATA_L1)

/*! Matrix holding the sign values for all estimates, all channels
  Dimension #MAXNRSBRCHANNELS * +#SBR_QMF_CHANNELS* #MAX_NO_OF_ESTIMATES
*/
C_ALLOC_MEM2(Ram_Sbr_signMatrix, INT, (MAX_NO_OF_ESTIMATES * 64), (8))

/*! Frequency band table (low res) <br>
  Dimension #MAX_FREQ_COEFFS/2+1
*/
C_ALLOC_MEM2(Ram_Sbr_freqBandTableLO, UCHAR, (MAX_FREQ_COEFFS / 2 + 1), (8))

/*! Frequency band table (high res) <br>
  Dimension #MAX_FREQ_COEFFS +1
*/
C_ALLOC_MEM2(Ram_Sbr_freqBandTableHI, UCHAR, (MAX_FREQ_COEFFS + 1), (8))

/*! vk matser table <br>
  Dimension #MAX_FREQ_COEFFS +1
*/
C_ALLOC_MEM2(Ram_Sbr_v_k_master, UCHAR, (MAX_FREQ_COEFFS + 1), (8))

/*
  Missing harmonics detection
*/

/*! sbr_detectionVectors <br>
  Dimension #MAX_NUM_CHANNELS*#MAX_NO_OF_ESTIMATES*#MAX_FREQ_COEFFS]
*/
C_ALLOC_MEM2(Ram_Sbr_detectionVectors, UCHAR,
             (MAX_NO_OF_ESTIMATES * MAX_FREQ_COEFFS), (8))

/*! sbr_prevCompVec[ <br>
  Dimension #MAX_NUM_CHANNELS*#MAX_FREQ_COEFFS]
*/
C_ALLOC_MEM2(Ram_Sbr_prevEnvelopeCompensation, UCHAR, MAX_FREQ_COEFFS, (8))
/*! sbr_guideScfb[ <br>
  Dimension #MAX_NUM_CHANNELS*#MAX_FREQ_COEFFS]
*/
C_ALLOC_MEM2(Ram_Sbr_guideScfb, UCHAR, MAX_FREQ_COEFFS, (8))

/*! sbr_guideVectorDetected <br>
  Dimension #MAX_NUM_CHANNELS*#MAX_NO_OF_ESTIMATES*#MAX_FREQ_COEFFS]
*/
C_ALLOC_MEM2(Ram_Sbr_guideVectorDetected, UCHAR,
             (MAX_NO_OF_ESTIMATES * MAX_FREQ_COEFFS), (8))
C_ALLOC_MEM2(Ram_Sbr_guideVectorDiff, FIXP_DBL,
             (MAX_NO_OF_ESTIMATES * MAX_FREQ_COEFFS), (8))
C_ALLOC_MEM2(Ram_Sbr_guideVectorOrig, FIXP_DBL,
             (MAX_NO_OF_ESTIMATES * MAX_FREQ_COEFFS), (8))

/*
  Static Parametric Stereo memory
*/
C_AALLOC_MEM_L(Ram_PsQmfStatesSynthesis, FIXP_DBL, 640 / 2, SECT_DATA_L1)

C_ALLOC_MEM_L(Ram_PsEncode, PS_ENCODE, 1, SECT_DATA_L1)
C_ALLOC_MEM(Ram_ParamStereo, PARAMETRIC_STEREO, 1)

/* @} */

/*!
  \name DynamicSbrData

  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the core encoder.
*/
/* @{ */

/*! Energy buffer for envelope extraction <br>
  Dimension #MAXNRSBRCHANNELS * +#SBR_QMF_SLOTS *  #SBR_QMF_CHANNELS
*/
C_ALLOC_MEM2(Ram_Sbr_envYBuffer, FIXP_DBL, (32 / 2 * 64), (8))

FIXP_DBL* GetRam_Sbr_envYBuffer(int n, UCHAR* dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + OFFSET_NRG + (n*Y_2_BUF_BYTE)) is sufficiently aligned, so
   * the cast is safe */
  return reinterpret_cast<FIXP_DBL*>(
      reinterpret_cast<void*>(dynamic_RAM + OFFSET_NRG + (n * Y_2_BUF_BYTE)));
}

/*
 * QMF data
 */
/* The SBR encoder uses a single channel overlapping buffer set (always n=0),
 * but PS does not. */
FIXP_DBL* GetRam_Sbr_envRBuffer(int n, UCHAR* dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + OFFSET_QMF + (n*(ENV_R_BUFF_BYTE+ENV_I_BUFF_BYTE))) is
   * sufficiently aligned, so the cast is safe */
  return reinterpret_cast<FIXP_DBL*>(reinterpret_cast<void*>(
      dynamic_RAM + OFFSET_QMF + (n * (ENV_R_BUFF_BYTE + ENV_I_BUFF_BYTE))));
}
FIXP_DBL* GetRam_Sbr_envIBuffer(int n, UCHAR* dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + OFFSET_QMF + (ENV_R_BUFF_BYTE) +
   * (n*(ENV_R_BUFF_BYTE+ENV_I_BUFF_BYTE))) is sufficiently aligned, so the cast
   * is safe */
  return reinterpret_cast<FIXP_DBL*>(
      reinterpret_cast<void*>(dynamic_RAM + OFFSET_QMF + (ENV_R_BUFF_BYTE) +
                              (n * (ENV_R_BUFF_BYTE + ENV_I_BUFF_BYTE))));
}

/* @} */
