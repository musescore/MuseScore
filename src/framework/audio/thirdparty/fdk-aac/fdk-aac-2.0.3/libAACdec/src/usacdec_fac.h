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

/**************************** AAC decoder library ******************************

   Author(s):   Manuel Jander

   Description: USAC FAC

*******************************************************************************/

#ifndef USACDEC_FAC_H
#define USACDEC_FAC_H

#include "channelinfo.h"
#include "FDK_bitstream.h"

/**
 * \brief Get the address of a memory area of the spectral data memory were the
 * FAC data can be stored into.
 * \param spec SPECTRAL_PTR pointing to the current spectral data.
 * \param mod the current LPD mod array.
 * \param pState pointer to a private state variable which must be 0 for the
 * first call and not changed externally.
 * \param isFullbandLPD is 1 if fullband LPD mode is on, otherwise it is 0.
 */
FIXP_DBL *CLpd_FAC_GetMemory(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                             UCHAR mod[NB_SUBFR], int *pState);

/**
 * \brief read a fac bitstream data block.
 * \param hBs a bit stream handle, where the fac bitstream data is located.
 * \param pFac pointer to were the FAC data will be stored into.
 * \param pFacScale pointer to were the FAC data scale value will be stored
 * into.
 * \param tcx_gain value to be used as FAC gain. If zero, read fac_gain from
 * bitstream.
 * \param tcx_gain_e exponen value of tcx_gain.
 * \param frame the subframe to be considered from the current superframe.
 * Always 0 for FD case.
 * \return 0 on success, -1 on error.
 */
int CLpd_FAC_Read(HANDLE_FDK_BITSTREAM hBs, FIXP_DBL *pFac, SCHAR *pFacScale,
                  int length, int use_gain, int frame);

/**
 * \brief Apply TCX and ALFD gains to FAC data.
 * \param fac_data pointer to FAC data.
 * \param fac_length FAC length (128 or 96).
 * \param tcx_gain TCX gain
 * \param alfd_gains pointer to alfd gains.
 * \param mod mod value (1,2,3) of TCX frame where the FAC signal needs to be
 * applied.
 */
void CFac_ApplyGains(FIXP_DBL fac_data[LFAC], const INT fac_length,
                     const FIXP_DBL tcx_gain, const FIXP_DBL alfd_gains[],
                     const INT mod);

/**
 * \brief Do FAC transition from frequency domain to ACELP domain.
 */
INT CLpd_FAC_Mdct2Acelp(H_MDCT hMdct, FIXP_DBL *output, FIXP_DBL *pFac_data,
                        const int fac_data_e, FIXP_LPC *A, INT A_exp,
                        INT nrOutSamples, const INT fac_length,
                        const INT isFdFac, UCHAR prevWindowShape);

/**
 * \brief Do FAC transition from ACELP domain to frequency domain.
 * \param hMdct MDCT context.
 * \param output pointer for time domain output.
 * \param pSpec pointer to MDCT spectrum input.
 * \param spec_scale MDCT spectrum exponents.
 * \param nSpec amount of contiguos MDCT spectra.
 * \param pFac pointer to FAC MDCT domain data.
 * \param fac_scale exponent of FAC data.
 * \param fac_length length of FAC data.
 * \param nrSamples room in samples in output buffer.
 * \param tl MDCT transform length of pSpec.
 * \param wrs right MDCT window slope.
 * \param fr right MDCT window slope length.
 * \param A LP domain filter coefficients.
 * \param deemph_mem deemphasis filter state.
 * \param gain gain to be applied to FAC data before overlap add.
 * \param old_syn_mem Synthesis filter state.
 * \param isFdFac indicates fac processing from or to FD.
 * \param pFacData fac data stored for fullband LPD.
 * \param elFlags element specific parser guidance flags.
 * \param isFacForFullband indicates that fac is processed for fullband LPD.
 */
INT CLpd_FAC_Acelp2Mdct(H_MDCT hMdct, FIXP_DBL *output, FIXP_DBL *pSpec,
                        const SHORT spec_scale[], const int nSpec,
                        FIXP_DBL *pFac_data, const int fac_data_e,
                        const INT fac_length, INT nrSamples, const INT tl,
                        const FIXP_WTP *wrs, const INT fr, FIXP_LPC A[16],
                        INT A_exp, CAcelpStaticMem *acelp_mem,
                        const FIXP_DBL gain, const int last_frame_lost,
                        const int isFdFac, const UCHAR last_lpd, const int k,
                        int currAliasingSymmetry);

#endif /* USACDEC_FAC_H */
