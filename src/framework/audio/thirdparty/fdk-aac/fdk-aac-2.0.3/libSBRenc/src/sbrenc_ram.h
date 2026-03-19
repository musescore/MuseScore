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
$Revision: 92790 $
*/
#ifndef SBRENC_RAM_H
#define SBRENC_RAM_H

#include "sbr_def.h"
#include "env_est.h"
#include "sbr_encoder.h"
#include "sbr.h"

#include "ps_main.h"
#include "ps_encode.h"

#define ENV_TRANSIENTS_BYTE ((sizeof(FIXP_DBL) * (MAX_NUM_CHANNELS * 3 * 32)))

#define ENV_R_BUFF_BYTE ((sizeof(FIXP_DBL) * ((32) * MAX_HYBRID_BANDS)))
#define ENV_I_BUFF_BYTE ((sizeof(FIXP_DBL) * ((32) * MAX_HYBRID_BANDS)))
#define Y_BUF_CH_BYTE \
  ((2 * sizeof(FIXP_DBL) * (((32) - (32 / 2)) * MAX_HYBRID_BANDS)))

#define ENV_R_BUF_PS_BYTE ((sizeof(FIXP_DBL) * 32 * 64 / 2))
#define ENV_I_BUF_PS_BYTE ((sizeof(FIXP_DBL) * 32 * 64 / 2))

#define TON_BUF_CH_BYTE \
  ((sizeof(FIXP_DBL) * (MAX_NO_OF_ESTIMATES * MAX_FREQ_COEFFS)))

#define Y_2_BUF_BYTE (Y_BUF_CH_BYTE)

/* Workbuffer RAM - Allocation */
/*
 ++++++++++++++++++++++++++++++++++++++++++++++++++++
 |        OFFSET_QMF       |        OFFSET_NRG      |
 ++++++++++++++++++++++++++++++++++++++++++++++++++++
  ------------------------- -------------------------
 |                         |         0.5 *          |
 |     sbr_envRBuffer      | sbr_envYBuffer_size    |
 |     sbr_envIBuffer      |                        |
  ------------------------- -------------------------

*/
#define BUF_NRG_SIZE ((MAX_NUM_CHANNELS * Y_2_BUF_BYTE))
#define BUF_QMF_SIZE (ENV_R_BUFF_BYTE + ENV_I_BUFF_BYTE)

/* Size of the shareable memory region than can be reused */
#define SBR_ENC_DYN_RAM_SIZE (BUF_QMF_SIZE + BUF_NRG_SIZE)

#define OFFSET_QMF (0)
#define OFFSET_NRG (OFFSET_QMF + BUF_QMF_SIZE)

/*
 *****************************************************************************************************
 */

H_ALLOC_MEM(Ram_SbrDynamic_RAM, FIXP_DBL)

H_ALLOC_MEM(Ram_SbrEncoder, SBR_ENCODER)
H_ALLOC_MEM(Ram_SbrChannel, SBR_CHANNEL)
H_ALLOC_MEM(Ram_SbrElement, SBR_ELEMENT)

H_ALLOC_MEM(Ram_Sbr_quotaMatrix, FIXP_DBL)
H_ALLOC_MEM(Ram_Sbr_signMatrix, INT)

H_ALLOC_MEM(Ram_Sbr_QmfStatesAnalysis, FIXP_QAS)

H_ALLOC_MEM(Ram_Sbr_freqBandTableLO, UCHAR)
H_ALLOC_MEM(Ram_Sbr_freqBandTableHI, UCHAR)
H_ALLOC_MEM(Ram_Sbr_v_k_master, UCHAR)

H_ALLOC_MEM(Ram_Sbr_detectionVectors, UCHAR)
H_ALLOC_MEM(Ram_Sbr_prevEnvelopeCompensation, UCHAR)
H_ALLOC_MEM(Ram_Sbr_guideScfb, UCHAR)
H_ALLOC_MEM(Ram_Sbr_guideVectorDetected, UCHAR)

/* Dynamic Memory Allocation */

H_ALLOC_MEM(Ram_Sbr_envYBuffer, FIXP_DBL)
FIXP_DBL* GetRam_Sbr_envYBuffer(int n, UCHAR* dynamic_RAM);
FIXP_DBL* GetRam_Sbr_envRBuffer(int n, UCHAR* dynamic_RAM);
FIXP_DBL* GetRam_Sbr_envIBuffer(int n, UCHAR* dynamic_RAM);

H_ALLOC_MEM(Ram_Sbr_guideVectorDiff, FIXP_DBL)
H_ALLOC_MEM(Ram_Sbr_guideVectorOrig, FIXP_DBL)

H_ALLOC_MEM(Ram_PsQmfStatesSynthesis, FIXP_DBL)

H_ALLOC_MEM(Ram_PsEncode, PS_ENCODE)

FIXP_DBL* FDKsbrEnc_SliceRam_PsRqmf(FIXP_DBL* rQmfData, UCHAR* dynamic_RAM,
                                    int n, int i, int qmfSlots);
FIXP_DBL* FDKsbrEnc_SliceRam_PsIqmf(FIXP_DBL* iQmfData, UCHAR* dynamic_RAM,
                                    int n, int i, int qmfSlots);

H_ALLOC_MEM(Ram_ParamStereo, PARAMETRIC_STEREO)
#endif
