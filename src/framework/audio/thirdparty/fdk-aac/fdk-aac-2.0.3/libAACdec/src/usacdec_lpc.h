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

   Author(s):   Matthias Hildenbrand, Manuel Jander

   Description: USAC LPC/AVQ decode

*******************************************************************************/

#ifndef USACDEC_LPC_H
#define USACDEC_LPC_H

#include "channelinfo.h"
#include "common_fix.h"
#include "FDK_bitstream.h"
#include "usacdec_rom.h"

#define LSPARG_SCALE 10

/**
 * \brief AVQ (refinement) decode
 * \param hBs bitstream handle
 * \param lsfq buffer for AVQ decode output.
 * \param nk_mode quantization mode.
 * \param nqn amount of split/interleaved RE8 vectors.
 * \param total amount of individual data values to decode.
 * \return 0 on success, -1 on error.
 */
int CLpc_DecodeAVQ(HANDLE_FDK_BITSTREAM hBs, FIXP_DBL *lsfq, int nk_mode,
                   int nqn, int length);

/**
 * \brief Read and decode LPC coeficient sets. First stage approximation + AVQ
 * decode.
 * \param[in]  hBs bitstream handle to read data from.
 * \param[out] lsp buffer into which the decoded LSP coefficients will be stored
 * into.
 * \param[in,out] lpc4_lsf buffer into which the decoded LCP4 LSF coefficients
 * will be stored into (persistent).
 * \param[out] lsf_adaptive_mean_cand lsf adaptive mean vector needed for
 * concealment.
 * \param[out] pStability array with stability values for the ACELP decoder (and
 * concealment).
 * \param[in]  mod array which defines modes (ACELP, TCX20|40|80) are used in
 * the current superframe.
 * \param[in]  first_lpd_flag indicates the presence of LPC0
 * \param[in]  last_lpc_lost indicate that LPC4 of previous frame was lost.
 * \param[in]  last_frame_ok indicate that the last frame was ok.
 * \return 0 on success, -1 on error.
 */
int CLpc_Read(HANDLE_FDK_BITSTREAM hBs, FIXP_LPC lsp[][M_LP_FILTER_ORDER],
              FIXP_LPC lpc4_lsf[M_LP_FILTER_ORDER],
              FIXP_LPC lsf_adaptive_mean_cand[M_LP_FILTER_ORDER],
              FIXP_SGL pStability[], UCHAR *mod, int first_lpd_flag,
              int last_lpc_lost, int last_frame_ok);

/**
 * \brief Generate LPC coefficient sets in case frame loss.
 * \param lsp buffer into which the decoded LSP coefficients will be stored
 * into.
 * \param lpc4_lsf buffer into which the decoded LCP4 LSF coefficients will be
 * stored into (persistent).
 * \param isf_adaptive_mean
 * \param first_lpd_flag indicates the previous LSF4 coefficients lpc4_lsf[] are
 * not valid.
 */
void CLpc_Conceal(FIXP_LPC lsp[][M_LP_FILTER_ORDER],
                  FIXP_LPC lpc4_lsf[M_LP_FILTER_ORDER],
                  FIXP_LPC isf_adaptive_mean[M_LP_FILTER_ORDER],
                  const int first_lpd_flag);

/**
 * \brief apply absolute weighting
 * \param A weighted LPC coefficient vector output. The first coeffcient is
 * implicitly 1.0
 * \param A LPC coefficient vector. The first coeffcient is implicitly 1.0
 * \param m length of vector A
 */
/* static */
void E_LPC_a_weight(FIXP_LPC *wA, const FIXP_LPC *A, const int m);

/**
 * \brief decode TCX/FAC gain. In case of TCX the lg/sqrt(rms) part
 *        must still be applied to obtain the gain value.
 * \param gain (o) pointer were the gain mantissa is stored into.
 * \param gain_e (o) pointer were the gain exponent is stored into.
 * \param gain_code (i) the 7 bit binary word from the bitstream
 *                      representing the gain.
 */
void CLpd_DecodeGain(FIXP_DBL *gain, INT *gain_e, int gain_code);

/**
 * \brief convert LSP coefficients into LP domain.
 */
void E_LPC_f_lsp_a_conversion(FIXP_LPC *lsp, FIXP_LPC *a, INT *a_exp);

#endif /* USACDEC_LPC_H */
