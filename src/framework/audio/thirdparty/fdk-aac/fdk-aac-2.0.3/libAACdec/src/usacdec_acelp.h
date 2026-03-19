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

   Author(s):   Matthias Hildenbrand

   Description: USAC ACELP frame decoder

*******************************************************************************/

#ifndef USACDEC_ACELP_H
#define USACDEC_ACELP_H

#include "common_fix.h"
#include "FDK_bitstream.h"
#include "usacdec_const.h"
#include "usacdec_rom.h"

//#define ENHANCED_TCX_TD_CONCEAL_ENABLE

/** Structure which holds the ACELP internal persistent memory */
typedef struct {
  FIXP_DBL old_exc_mem[PIT_MAX_MAX + L_INTERPOL];
  FIXP_DBL old_syn_mem[M_LP_FILTER_ORDER]; /* synthesis filter states */
  FIXP_SGL A[M_LP_FILTER_ORDER];
  INT A_exp;
  FIXP_DBL gc_threshold;
  FIXP_DBL de_emph_mem;
  FIXP_SGL past_gpit;
  FIXP_DBL past_gcode;
  USHORT old_T0;
  UCHAR old_T0_frac;
  FIXP_DBL deemph_mem_wsyn;
  FIXP_DBL wsyn_rms;
  SHORT seed_ace;
} CAcelpStaticMem;

/** Structure which holds the parameter data needed to decode one ACELP frame.
 */
typedef struct {
  UCHAR
  acelp_core_mode;   /**< mean excitation energy index for whole ACELP frame
                      */
  UCHAR mean_energy; /**< acelp core mode for whole ACELP frame */
  USHORT T0[NB_SUBFR];
  UCHAR T0_frac[NB_SUBFR];
  UCHAR ltp_filtering_flag[NB_SUBFR]; /**< controlls whether LTP postfilter is
                                         active for each ACELP subframe */
  SHORT icb_index[NB_SUBFR]
                 [8]; /**< innovative codebook index for each ACELP subframe */
  UCHAR gains[NB_SUBFR]; /**< gain index for each ACELP subframe */
} CAcelpChannelData;

/**
 * \brief Read the acelp_coding() bitstream part.
 * \param[in] hBs bitstream handle to read data from.
 * \param[out] acelpData pointer to structure to store the parsed data of one
 * ACELP frame.
 * \param[in] acelp_core_mode the ACELP core mode index.
 * \param[in] coreCoderFrameLength length of core coder frame (1024|768)
 */
INT CLpd_AcelpRead(HANDLE_FDK_BITSTREAM hBs, CAcelpChannelData *acelpData,
                   INT acelp_core_mode, INT i_offset, INT coreCoderFrameLength);
/**
 * \brief Initialization of memory before one LPD frame is decoded
 * \param[out] synth_buf synthesis buffer to be initialized, exponent = SF_SYNTH
 * \param[in] old_synth past synthesis of previous LPD frame, exponent =
 * SF_SYNTH
 * \param[out] synth_buf_fb fullband synthesis buffer to be initialized,
 * exponent = SF_SYNTH
 * \param[in] old_synth_fb past fullband synthesis of previous LPD frame,
 * exponent = SF_SYNTH
 * \param[out] pitch vector where decoded pitch lag values are stored
 * \param[in] old_T_pf past pitch lag values of previous LPD frame
 * \param[in] samplingRate sampling rate for pitch lag offset calculation
 * \param[out] i_offset pitch lag offset for the decoding of the pitch lag
 * \param[in] coreCoderFrameLength length of core coder frame (1024|768)
 */
void Acelp_PreProcessing(FIXP_DBL *synth_buf, FIXP_DBL *old_synth, INT *pitch,
                         INT *old_T_pf, FIXP_DBL *pit_gain,
                         FIXP_DBL *old_gain_pf, INT samplingRate, INT *i_offset,
                         INT coreCoderFrameLength, INT synSfd,
                         INT nbSubfrSuperfr);

/**
 * \brief Save tail of buffers for the initialization of the next LPD frame
 * \param[in] synth_buf synthesis of current LPD frame, exponent = SF_SYNTH
 * \param[out] old_synth memory where tail of fullband synth_buf is stored,
 * exponent = SF_SYNTH
 * \param[in] synth_buf_fb fullband synthesis of current LPD frame, exponent =
 * SF_SYNTH
 * \param[out] old_synth_fb memory where tail of fullband synth_buf is stored,
 * exponent = SF_SYNTH
 * \param[in] pitch decoded pitch lag values of current LPD frame
 * \param[out] old_T_pf memory where last SYN_SFD pitch lag values are stored
 */
void Acelp_PostProcessing(FIXP_DBL *synth_buf, FIXP_DBL *old_synth, INT *pitch,
                          INT *old_T_pf, INT coreCoderFrameLength, INT synSfd,
                          INT nbSubfrSuperfr);

/**
 * \brief Decode one ACELP frame (three or four ACELP subframes with 64 samples
 * each)
 * \param[in,out] acelp_mem pointer to ACELP memory structure
 * \param[in] i_offset pitch lag offset
 * \param[in] lsp_old LPC filter in LSP domain corresponding to previous frame
 * \param[in] lsp_new LPC filter in LSP domain corresponding to current frame
 * \param[in] stab_fac stability factor constrained by 0<=stab_fac<=1.0,
 * exponent = SF_STAB
 * \param[in] acelpData pointer to struct with data which is needed for decoding
 * one ACELP frame
 * \param[out] synth ACELP output signal
 * \param[out] pT four decoded pitch lag values
 * \param[in] coreCoderFrameLength length of core coder frame (1024|768)
 */
void CLpd_AcelpDecode(CAcelpStaticMem *acelp_mem, INT i_offset,
                      const FIXP_LPC lsp_old[M_LP_FILTER_ORDER],
                      const FIXP_LPC lsp_new[M_LP_FILTER_ORDER],
                      FIXP_SGL stab_fac, CAcelpChannelData *acelpData,
                      INT numLostSubframes, int lastLpcLost, int frameCnt,
                      FIXP_DBL synth[], int pT[], FIXP_DBL *pit_gain,
                      INT coreCoderFrameLength);

/**
 * \brief Reset ACELP internal memory.
 * \param[out] acelp_mem pointer to ACELP memory structure
 */
void CLpd_AcelpReset(CAcelpStaticMem *acelp_mem);

/**
 * \brief Initialize ACELP internal memory in case of FAC before ACELP decoder
 * is called
 * \param[in] synth points to end+1 of past valid synthesis signal, exponent =
 * SF_SYNTH
 * \param[in] last_lpd_mode last lpd mode
 * \param[in] last_last_lpd_mode lpd mode before last_lpd_mode
 * \param[in] A_new LP synthesis filter coeffs corresponding to last frame,
 * exponent = SF_A_COEFFS
 * \param[in] A_old LP synthesis filter coeffs corresponding to the frame before
 * last frame, exponent = SF_A_COEFFS
 * \param[in,out] acelp_mem pointer to ACELP memory structure
 * \param[in] coreCoderFrameLength length of core coder frame (1024|768)
 */
void CLpd_AcelpPrepareInternalMem(const FIXP_DBL *synth, UCHAR last_lpd_mode,
                                  UCHAR last_last_lpd_mode,
                                  const FIXP_LPC *A_new, const INT A_new_exp,
                                  const FIXP_LPC *A_old, const INT A_old_exp,
                                  CAcelpStaticMem *acelp_mem,
                                  INT coreCoderFrameLength, INT clearOldExc,
                                  UCHAR lpd_mode);

/**
 * \brief Calculate zero input response (zir) of the acelp synthesis filter
 * \param[in] A LP synthesis filter coefficients, exponent = SF_A_COEFFS
 * \param[in,out] acelp_mem pointer to ACELP memory structure
 * \param[in] length length of zir
 * \param[out] zir pointer to zir output buffer, exponent = SF_SYNTH
 */
void CLpd_Acelp_Zir(const FIXP_LPC A[], const INT A_exp,
                    CAcelpStaticMem *acelp_mem, const INT length,
                    FIXP_DBL zir[], int doDeemph);

/**
 * \brief Borrow static excitation memory from ACELP decoder
 * \param[in] acelp_mem pointer to ACELP memory structure
 * \param[in] length number of requested FIXP_DBL values
 * \return pointer to requested memory
 *
 * The caller has to take care not to overwrite valid memory areas.
 * During TCX/FAC calculations and before CLpd_AcelpPrepareInternalMem() is
 * called, the following memory size is available:
 * - 256 samples in case of ACELP -> TCX20 -> ACELP transition
 * - PIT_MAX_MAX+L_INTERPOL samples in all other cases
 */
FIXP_DBL *CLpd_ACELP_GetFreeExcMem(CAcelpStaticMem *acelp_mem, INT length);

void CLpd_TcxTDConceal(CAcelpStaticMem *acelp_mem, SHORT *pitch,
                       const FIXP_LPC lsp_old[M_LP_FILTER_ORDER],
                       const FIXP_LPC lsp_new[M_LP_FILTER_ORDER],
                       const FIXP_SGL stab_fac, INT numLostSubframes,
                       FIXP_DBL synth[], INT coreCoderFrameLength,
                       UCHAR last_tcx_noise_factor);

inline SHORT E_UTIL_random(SHORT *seed) {
  *seed = (SHORT)((((LONG)*seed * (LONG)31821) >> 1) + (LONG)13849);
  return (*seed);
}

#endif /* USACDEC_ACELP_H */
