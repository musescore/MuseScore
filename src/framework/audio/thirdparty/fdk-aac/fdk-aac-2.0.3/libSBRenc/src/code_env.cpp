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

#include "code_env.h"
#include "sbrenc_rom.h"

/*****************************************************************************

 functionname: FDKsbrEnc_InitSbrHuffmanTables
 description:  initializes Huffman Tables dependent on chosen amp_res
 returns:      error handle
 input:
 output:

*****************************************************************************/
INT FDKsbrEnc_InitSbrHuffmanTables(HANDLE_SBR_ENV_DATA sbrEnvData,
                                   HANDLE_SBR_CODE_ENVELOPE henv,
                                   HANDLE_SBR_CODE_ENVELOPE hnoise,
                                   AMP_RES amp_res) {
  if ((!henv) || (!hnoise) || (!sbrEnvData)) return (1); /* not init. */

  sbrEnvData->init_sbr_amp_res = amp_res;

  switch (amp_res) {
    case SBR_AMP_RES_3_0:
      /*envelope data*/

      /*Level/Pan - coding */
      sbrEnvData->hufftableLevelTimeC = v_Huff_envelopeLevelC11T;
      sbrEnvData->hufftableLevelTimeL = v_Huff_envelopeLevelL11T;
      sbrEnvData->hufftableBalanceTimeC = bookSbrEnvBalanceC11T;
      sbrEnvData->hufftableBalanceTimeL = bookSbrEnvBalanceL11T;

      sbrEnvData->hufftableLevelFreqC = v_Huff_envelopeLevelC11F;
      sbrEnvData->hufftableLevelFreqL = v_Huff_envelopeLevelL11F;
      sbrEnvData->hufftableBalanceFreqC = bookSbrEnvBalanceC11F;
      sbrEnvData->hufftableBalanceFreqL = bookSbrEnvBalanceL11F;

      /*Right/Left - coding */
      sbrEnvData->hufftableTimeC = v_Huff_envelopeLevelC11T;
      sbrEnvData->hufftableTimeL = v_Huff_envelopeLevelL11T;
      sbrEnvData->hufftableFreqC = v_Huff_envelopeLevelC11F;
      sbrEnvData->hufftableFreqL = v_Huff_envelopeLevelL11F;

      sbrEnvData->codeBookScfLavBalance = CODE_BOOK_SCF_LAV_BALANCE11;
      sbrEnvData->codeBookScfLav = CODE_BOOK_SCF_LAV11;

      sbrEnvData->si_sbr_start_env_bits = SI_SBR_START_ENV_BITS_AMP_RES_3_0;
      sbrEnvData->si_sbr_start_env_bits_balance =
          SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0;
      break;

    case SBR_AMP_RES_1_5:
      /*envelope data*/

      /*Level/Pan - coding */
      sbrEnvData->hufftableLevelTimeC = v_Huff_envelopeLevelC10T;
      sbrEnvData->hufftableLevelTimeL = v_Huff_envelopeLevelL10T;
      sbrEnvData->hufftableBalanceTimeC = bookSbrEnvBalanceC10T;
      sbrEnvData->hufftableBalanceTimeL = bookSbrEnvBalanceL10T;

      sbrEnvData->hufftableLevelFreqC = v_Huff_envelopeLevelC10F;
      sbrEnvData->hufftableLevelFreqL = v_Huff_envelopeLevelL10F;
      sbrEnvData->hufftableBalanceFreqC = bookSbrEnvBalanceC10F;
      sbrEnvData->hufftableBalanceFreqL = bookSbrEnvBalanceL10F;

      /*Right/Left - coding */
      sbrEnvData->hufftableTimeC = v_Huff_envelopeLevelC10T;
      sbrEnvData->hufftableTimeL = v_Huff_envelopeLevelL10T;
      sbrEnvData->hufftableFreqC = v_Huff_envelopeLevelC10F;
      sbrEnvData->hufftableFreqL = v_Huff_envelopeLevelL10F;

      sbrEnvData->codeBookScfLavBalance = CODE_BOOK_SCF_LAV_BALANCE10;
      sbrEnvData->codeBookScfLav = CODE_BOOK_SCF_LAV10;

      sbrEnvData->si_sbr_start_env_bits = SI_SBR_START_ENV_BITS_AMP_RES_1_5;
      sbrEnvData->si_sbr_start_env_bits_balance =
          SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5;
      break;

    default:
      return (1); /* undefined amp_res mode */
  }

  /* these are common to both amp_res values */
  /*Noise data*/

  /*Level/Pan - coding */
  sbrEnvData->hufftableNoiseLevelTimeC = v_Huff_NoiseLevelC11T;
  sbrEnvData->hufftableNoiseLevelTimeL = v_Huff_NoiseLevelL11T;
  sbrEnvData->hufftableNoiseBalanceTimeC = bookSbrNoiseBalanceC11T;
  sbrEnvData->hufftableNoiseBalanceTimeL = bookSbrNoiseBalanceL11T;

  sbrEnvData->hufftableNoiseLevelFreqC = v_Huff_envelopeLevelC11F;
  sbrEnvData->hufftableNoiseLevelFreqL = v_Huff_envelopeLevelL11F;
  sbrEnvData->hufftableNoiseBalanceFreqC = bookSbrEnvBalanceC11F;
  sbrEnvData->hufftableNoiseBalanceFreqL = bookSbrEnvBalanceL11F;

  /*Right/Left - coding */
  sbrEnvData->hufftableNoiseTimeC = v_Huff_NoiseLevelC11T;
  sbrEnvData->hufftableNoiseTimeL = v_Huff_NoiseLevelL11T;
  sbrEnvData->hufftableNoiseFreqC = v_Huff_envelopeLevelC11F;
  sbrEnvData->hufftableNoiseFreqL = v_Huff_envelopeLevelL11F;

  sbrEnvData->si_sbr_start_noise_bits = SI_SBR_START_NOISE_BITS_AMP_RES_3_0;
  sbrEnvData->si_sbr_start_noise_bits_balance =
      SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0;

  /* init envelope tables and codebooks */
  henv->codeBookScfLavBalanceTime = sbrEnvData->codeBookScfLavBalance;
  henv->codeBookScfLavBalanceFreq = sbrEnvData->codeBookScfLavBalance;
  henv->codeBookScfLavLevelTime = sbrEnvData->codeBookScfLav;
  henv->codeBookScfLavLevelFreq = sbrEnvData->codeBookScfLav;
  henv->codeBookScfLavTime = sbrEnvData->codeBookScfLav;
  henv->codeBookScfLavFreq = sbrEnvData->codeBookScfLav;

  henv->hufftableLevelTimeL = sbrEnvData->hufftableLevelTimeL;
  henv->hufftableBalanceTimeL = sbrEnvData->hufftableBalanceTimeL;
  henv->hufftableTimeL = sbrEnvData->hufftableTimeL;
  henv->hufftableLevelFreqL = sbrEnvData->hufftableLevelFreqL;
  henv->hufftableBalanceFreqL = sbrEnvData->hufftableBalanceFreqL;
  henv->hufftableFreqL = sbrEnvData->hufftableFreqL;

  henv->codeBookScfLavFreq = sbrEnvData->codeBookScfLav;
  henv->codeBookScfLavTime = sbrEnvData->codeBookScfLav;

  henv->start_bits = sbrEnvData->si_sbr_start_env_bits;
  henv->start_bits_balance = sbrEnvData->si_sbr_start_env_bits_balance;

  /* init noise tables and codebooks */

  hnoise->codeBookScfLavBalanceTime = CODE_BOOK_SCF_LAV_BALANCE11;
  hnoise->codeBookScfLavBalanceFreq = CODE_BOOK_SCF_LAV_BALANCE11;
  hnoise->codeBookScfLavLevelTime = CODE_BOOK_SCF_LAV11;
  hnoise->codeBookScfLavLevelFreq = CODE_BOOK_SCF_LAV11;
  hnoise->codeBookScfLavTime = CODE_BOOK_SCF_LAV11;
  hnoise->codeBookScfLavFreq = CODE_BOOK_SCF_LAV11;

  hnoise->hufftableLevelTimeL = sbrEnvData->hufftableNoiseLevelTimeL;
  hnoise->hufftableBalanceTimeL = sbrEnvData->hufftableNoiseBalanceTimeL;
  hnoise->hufftableTimeL = sbrEnvData->hufftableNoiseTimeL;
  hnoise->hufftableLevelFreqL = sbrEnvData->hufftableNoiseLevelFreqL;
  hnoise->hufftableBalanceFreqL = sbrEnvData->hufftableNoiseBalanceFreqL;
  hnoise->hufftableFreqL = sbrEnvData->hufftableNoiseFreqL;

  hnoise->start_bits = sbrEnvData->si_sbr_start_noise_bits;
  hnoise->start_bits_balance = sbrEnvData->si_sbr_start_noise_bits_balance;

  /* No delta coding in time from the previous frame due to 1.5dB FIx-FIX rule
   */
  henv->upDate = 0;
  hnoise->upDate = 0;
  return (0);
}

/*******************************************************************************
 Functionname:  indexLow2High
 *******************************************************************************

 Description:   Nice small patch-functions in order to cope with non-factor-2
                ratios between high-res and low-res

 Arguments:     INT offset, INT index, FREQ_RES res

 Return:        INT

*******************************************************************************/
static INT indexLow2High(INT offset, INT index, FREQ_RES res) {
  if (res == FREQ_RES_LOW) {
    if (offset >= 0) {
      if (index < offset)
        return (index);
      else
        return (2 * index - offset);
    } else {
      offset = -offset;
      if (index < offset)
        return (2 * index + index);
      else
        return (2 * index + offset);
    }
  } else
    return (index);
}

/*******************************************************************************
 Functionname:  mapLowResEnergyVal
 *******************************************************************************

 Description:

 Arguments:     INT currVal,INT* prevData, INT offset, INT index, FREQ_RES res

 Return:        none

*******************************************************************************/
static void mapLowResEnergyVal(SCHAR currVal, SCHAR *prevData, INT offset,
                               INT index, FREQ_RES res) {
  if (res == FREQ_RES_LOW) {
    if (offset >= 0) {
      if (index < offset)
        prevData[index] = currVal;
      else {
        prevData[2 * index - offset] = currVal;
        prevData[2 * index + 1 - offset] = currVal;
      }
    } else {
      offset = -offset;
      if (index < offset) {
        prevData[3 * index] = currVal;
        prevData[3 * index + 1] = currVal;
        prevData[3 * index + 2] = currVal;
      } else {
        prevData[2 * index + offset] = currVal;
        prevData[2 * index + 1 + offset] = currVal;
      }
    }
  } else
    prevData[index] = currVal;
}

/*******************************************************************************
 Functionname:  computeBits
 *******************************************************************************

 Description:

 Arguments:     INT delta,
                INT codeBookScfLavLevel,
                INT codeBookScfLavBalance,
                const UCHAR * hufftableLevel,
                const UCHAR * hufftableBalance, INT coupling, INT channel)

 Return:        INT

*******************************************************************************/
static INT computeBits(SCHAR *delta, INT codeBookScfLavLevel,
                       INT codeBookScfLavBalance, const UCHAR *hufftableLevel,
                       const UCHAR *hufftableBalance, INT coupling,
                       INT channel) {
  INT index;
  INT delta_bits = 0;

  if (coupling) {
    if (channel == 1) {
      if (*delta < 0)
        index = fixMax(*delta, -codeBookScfLavBalance);
      else
        index = fixMin(*delta, codeBookScfLavBalance);

      if (index != *delta) {
        *delta = index;
        return (10000);
      }

      delta_bits = hufftableBalance[index + codeBookScfLavBalance];
    } else {
      if (*delta < 0)
        index = fixMax(*delta, -codeBookScfLavLevel);
      else
        index = fixMin(*delta, codeBookScfLavLevel);

      if (index != *delta) {
        *delta = index;
        return (10000);
      }
      delta_bits = hufftableLevel[index + codeBookScfLavLevel];
    }
  } else {
    if (*delta < 0)
      index = fixMax(*delta, -codeBookScfLavLevel);
    else
      index = fixMin(*delta, codeBookScfLavLevel);

    if (index != *delta) {
      *delta = index;
      return (10000);
    }
    delta_bits = hufftableLevel[index + codeBookScfLavLevel];
  }

  return (delta_bits);
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_codeEnvelope
 *******************************************************************************

 Description:

 Arguments:     INT *sfb_nrg,
                const FREQ_RES *freq_res,
                SBR_CODE_ENVELOPE * h_sbrCodeEnvelope,
                INT *directionVec, INT scalable, INT nEnvelopes, INT channel,
                INT headerActive)

 Return:        none
                h_sbrCodeEnvelope->sfb_nrg_prev is modified !
                sfb_nrg is modified
                h_sbrCodeEnvelope->update is modfied !
                *directionVec is modified

*******************************************************************************/
void FDKsbrEnc_codeEnvelope(SCHAR *sfb_nrg, const FREQ_RES *freq_res,
                            SBR_CODE_ENVELOPE *h_sbrCodeEnvelope,
                            INT *directionVec, INT coupling, INT nEnvelopes,
                            INT channel, INT headerActive) {
  INT i, no_of_bands, band;
  FIXP_DBL tmp1, tmp2, tmp3, dF_edge_1stEnv;
  SCHAR *ptr_nrg;

  INT codeBookScfLavLevelTime;
  INT codeBookScfLavLevelFreq;
  INT codeBookScfLavBalanceTime;
  INT codeBookScfLavBalanceFreq;
  const UCHAR *hufftableLevelTimeL;
  const UCHAR *hufftableBalanceTimeL;
  const UCHAR *hufftableLevelFreqL;
  const UCHAR *hufftableBalanceFreqL;

  INT offset = h_sbrCodeEnvelope->offset;
  INT envDataTableCompFactor;

  INT delta_F_bits = 0, delta_T_bits = 0;
  INT use_dT;

  SCHAR delta_F[MAX_FREQ_COEFFS];
  SCHAR delta_T[MAX_FREQ_COEFFS];
  SCHAR last_nrg, curr_nrg;

  tmp1 = FL2FXCONST_DBL(0.5f) >> (DFRACT_BITS - 16 - 1);
  tmp2 = h_sbrCodeEnvelope->dF_edge_1stEnv >> (DFRACT_BITS - 16);
  tmp3 = (FIXP_DBL)fMult(h_sbrCodeEnvelope->dF_edge_incr,
                         ((FIXP_DBL)h_sbrCodeEnvelope->dF_edge_incr_fac) << 15);

  dF_edge_1stEnv = tmp1 + tmp2 + tmp3;

  if (coupling) {
    codeBookScfLavLevelTime = h_sbrCodeEnvelope->codeBookScfLavLevelTime;
    codeBookScfLavLevelFreq = h_sbrCodeEnvelope->codeBookScfLavLevelFreq;
    codeBookScfLavBalanceTime = h_sbrCodeEnvelope->codeBookScfLavBalanceTime;
    codeBookScfLavBalanceFreq = h_sbrCodeEnvelope->codeBookScfLavBalanceFreq;
    hufftableLevelTimeL = h_sbrCodeEnvelope->hufftableLevelTimeL;
    hufftableBalanceTimeL = h_sbrCodeEnvelope->hufftableBalanceTimeL;
    hufftableLevelFreqL = h_sbrCodeEnvelope->hufftableLevelFreqL;
    hufftableBalanceFreqL = h_sbrCodeEnvelope->hufftableBalanceFreqL;
  } else {
    codeBookScfLavLevelTime = h_sbrCodeEnvelope->codeBookScfLavTime;
    codeBookScfLavLevelFreq = h_sbrCodeEnvelope->codeBookScfLavFreq;
    codeBookScfLavBalanceTime = h_sbrCodeEnvelope->codeBookScfLavTime;
    codeBookScfLavBalanceFreq = h_sbrCodeEnvelope->codeBookScfLavFreq;
    hufftableLevelTimeL = h_sbrCodeEnvelope->hufftableTimeL;
    hufftableBalanceTimeL = h_sbrCodeEnvelope->hufftableTimeL;
    hufftableLevelFreqL = h_sbrCodeEnvelope->hufftableFreqL;
    hufftableBalanceFreqL = h_sbrCodeEnvelope->hufftableFreqL;
  }

  if (coupling == 1 && channel == 1)
    envDataTableCompFactor =
        1; /*should be one when the new huffman-tables are ready*/
  else
    envDataTableCompFactor = 0;

  if (h_sbrCodeEnvelope->deltaTAcrossFrames == 0) h_sbrCodeEnvelope->upDate = 0;

  /* no delta coding in time in case of a header */
  if (headerActive) h_sbrCodeEnvelope->upDate = 0;

  for (i = 0; i < nEnvelopes; i++) {
    if (freq_res[i] == FREQ_RES_HIGH)
      no_of_bands = h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH];
    else
      no_of_bands = h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW];

    ptr_nrg = sfb_nrg;
    curr_nrg = *ptr_nrg;

    delta_F[0] = curr_nrg >> envDataTableCompFactor;

    if (coupling && channel == 1)
      delta_F_bits = h_sbrCodeEnvelope->start_bits_balance;
    else
      delta_F_bits = h_sbrCodeEnvelope->start_bits;

    if (h_sbrCodeEnvelope->upDate != 0) {
      delta_T[0] = (curr_nrg - h_sbrCodeEnvelope->sfb_nrg_prev[0]) >>
                   envDataTableCompFactor;

      delta_T_bits = computeBits(&delta_T[0], codeBookScfLavLevelTime,
                                 codeBookScfLavBalanceTime, hufftableLevelTimeL,
                                 hufftableBalanceTimeL, coupling, channel);
    }

    mapLowResEnergyVal(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev, offset, 0,
                       freq_res[i]);

    /* ensure that nrg difference is not higher than codeBookScfLavXXXFreq */
    if (coupling && channel == 1) {
      for (band = no_of_bands - 1; band > 0; band--) {
        if (ptr_nrg[band] - ptr_nrg[band - 1] > codeBookScfLavBalanceFreq) {
          ptr_nrg[band - 1] = ptr_nrg[band] - codeBookScfLavBalanceFreq;
        }
      }
      for (band = 1; band < no_of_bands; band++) {
        if (ptr_nrg[band - 1] - ptr_nrg[band] > codeBookScfLavBalanceFreq) {
          ptr_nrg[band] = ptr_nrg[band - 1] - codeBookScfLavBalanceFreq;
        }
      }
    } else {
      for (band = no_of_bands - 1; band > 0; band--) {
        if (ptr_nrg[band] - ptr_nrg[band - 1] > codeBookScfLavLevelFreq) {
          ptr_nrg[band - 1] = ptr_nrg[band] - codeBookScfLavLevelFreq;
        }
      }
      for (band = 1; band < no_of_bands; band++) {
        if (ptr_nrg[band - 1] - ptr_nrg[band] > codeBookScfLavLevelFreq) {
          ptr_nrg[band] = ptr_nrg[band - 1] - codeBookScfLavLevelFreq;
        }
      }
    }

    /* Coding loop*/
    for (band = 1; band < no_of_bands; band++) {
      last_nrg = (*ptr_nrg);
      ptr_nrg++;
      curr_nrg = (*ptr_nrg);

      delta_F[band] = (curr_nrg - last_nrg) >> envDataTableCompFactor;

      delta_F_bits += computeBits(
          &delta_F[band], codeBookScfLavLevelFreq, codeBookScfLavBalanceFreq,
          hufftableLevelFreqL, hufftableBalanceFreqL, coupling, channel);

      if (h_sbrCodeEnvelope->upDate != 0) {
        delta_T[band] =
            curr_nrg -
            h_sbrCodeEnvelope
                ->sfb_nrg_prev[indexLow2High(offset, band, freq_res[i])];
        delta_T[band] = delta_T[band] >> envDataTableCompFactor;
      }

      mapLowResEnergyVal(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev, offset,
                         band, freq_res[i]);

      if (h_sbrCodeEnvelope->upDate != 0) {
        delta_T_bits += computeBits(
            &delta_T[band], codeBookScfLavLevelTime, codeBookScfLavBalanceTime,
            hufftableLevelTimeL, hufftableBalanceTimeL, coupling, channel);
      }
    }

    /* Replace sfb_nrg with deltacoded samples and set flag */
    if (i == 0) {
      INT tmp_bits;
      tmp_bits = (((delta_T_bits * dF_edge_1stEnv) >> (DFRACT_BITS - 18)) +
                  (FIXP_DBL)1) >>
                 1;
      use_dT = (h_sbrCodeEnvelope->upDate != 0 && (delta_F_bits > tmp_bits));
    } else
      use_dT = (delta_T_bits < delta_F_bits && h_sbrCodeEnvelope->upDate != 0);

    if (use_dT) {
      directionVec[i] = TIME;
      FDKmemcpy(sfb_nrg, delta_T, no_of_bands * sizeof(SCHAR));
    } else {
      h_sbrCodeEnvelope->upDate = 0;
      directionVec[i] = FREQ;
      FDKmemcpy(sfb_nrg, delta_F, no_of_bands * sizeof(SCHAR));
    }
    sfb_nrg += no_of_bands;
    h_sbrCodeEnvelope->upDate = 1;
  }
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_InitSbrCodeEnvelope
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
INT FDKsbrEnc_InitSbrCodeEnvelope(HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope,
                                  INT *nSfb, INT deltaTAcrossFrames,
                                  FIXP_DBL dF_edge_1stEnv,
                                  FIXP_DBL dF_edge_incr) {
  FDKmemclear(h_sbrCodeEnvelope, sizeof(SBR_CODE_ENVELOPE));

  h_sbrCodeEnvelope->deltaTAcrossFrames = deltaTAcrossFrames;
  h_sbrCodeEnvelope->dF_edge_1stEnv = dF_edge_1stEnv;
  h_sbrCodeEnvelope->dF_edge_incr = dF_edge_incr;
  h_sbrCodeEnvelope->dF_edge_incr_fac = 0;
  h_sbrCodeEnvelope->upDate = 0;
  h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW] = nSfb[FREQ_RES_LOW];
  h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH] = nSfb[FREQ_RES_HIGH];
  h_sbrCodeEnvelope->offset = 2 * h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW] -
                              h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH];

  return (0);
}
