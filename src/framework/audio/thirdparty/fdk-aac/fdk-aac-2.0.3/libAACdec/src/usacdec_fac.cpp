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

   Description: USAC FAC

*******************************************************************************/

#include "usacdec_fac.h"

#include "usacdec_const.h"
#include "usacdec_lpc.h"
#include "usacdec_acelp.h"
#include "usacdec_rom.h"
#include "dct.h"
#include "FDK_tools_rom.h"
#include "mdct.h"

#define SPEC_FAC(ptr, i, gl) ((ptr) + ((i) * (gl)))

FIXP_DBL *CLpd_FAC_GetMemory(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                             UCHAR mod[NB_DIV], int *pState) {
  FIXP_DBL *ptr;
  int i;
  int k = 0;
  int max_windows = 8;

  FDK_ASSERT(*pState >= 0 && *pState < max_windows);

  /* Look for free space to store FAC data. 2 FAC data blocks fit into each TCX
   * spectral data block. */
  for (i = *pState; i < max_windows; i++) {
    if (mod[i >> 1] == 0) {
      break;
    }
  }

  *pState = i + 1;

  if (i == max_windows) {
    ptr = pAacDecoderChannelInfo->data.usac.fac_data0;
  } else {
    FDK_ASSERT(mod[(i >> 1)] == 0);
    ptr = SPEC_FAC(pAacDecoderChannelInfo->pSpectralCoefficient, i,
                   pAacDecoderChannelInfo->granuleLength << k);
  }

  return ptr;
}

int CLpd_FAC_Read(HANDLE_FDK_BITSTREAM hBs, FIXP_DBL *pFac, SCHAR *pFacScale,
                  int length, int use_gain, int frame) {
  FIXP_DBL fac_gain;
  int fac_gain_e = 0;

  if (use_gain) {
    CLpd_DecodeGain(&fac_gain, &fac_gain_e, FDKreadBits(hBs, 7));
  }

  if (CLpc_DecodeAVQ(hBs, pFac, 1, 1, length) != 0) {
    return -1;
  }

  {
    int scale;

    scale = getScalefactor(pFac, length);
    scaleValues(pFac, length, scale);
    pFacScale[frame] = DFRACT_BITS - 1 - scale;
  }

  if (use_gain) {
    int i;

    pFacScale[frame] += fac_gain_e;

    for (i = 0; i < length; i++) {
      pFac[i] = fMult(pFac[i], fac_gain);
    }
  }
  return 0;
}

/**
 * \brief Apply synthesis filter with zero input to x. The overall filter gain
 * is 1.0.
 * \param a LPC filter coefficients.
 * \param length length of the input/output data vector x.
 * \param x input/output vector, where the synthesis filter is applied in place.
 */
static void Syn_filt_zero(const FIXP_LPC a[], const INT a_exp, INT length,
                          FIXP_DBL x[]) {
  int i, j;
  FIXP_DBL L_tmp;

  for (i = 0; i < length; i++) {
    L_tmp = (FIXP_DBL)0;

    for (j = 0; j < fMin(i, M_LP_FILTER_ORDER); j++) {
      L_tmp -= fMultDiv2(a[j], x[i - (j + 1)]) >> (LP_FILTER_SCALE - 1);
    }

    L_tmp = scaleValue(L_tmp, a_exp + LP_FILTER_SCALE);
    x[i] = fAddSaturate(x[i], L_tmp);
  }
}

/* Table is also correct for coreCoderFrameLength = 768. Factor 3/4 is canceled
   out: gainFac = 0.5 * sqrt(fac_length/lFrame)
*/
static const FIXP_DBL gainFac[4] = {0x40000000, 0x2d413ccd, 0x20000000,
                                    0x16a09e66};

void CFac_ApplyGains(FIXP_DBL fac_data[LFAC], const INT fac_length,
                     const FIXP_DBL tcx_gain, const FIXP_DBL alfd_gains[],
                     const INT mod) {
  FIXP_DBL facFactor;
  int i;

  FDK_ASSERT((fac_length == 128) || (fac_length == 96));

  /* 2) Apply gain factor to FAC data */
  facFactor = fMult(gainFac[mod], tcx_gain);
  for (i = 0; i < fac_length; i++) {
    fac_data[i] = fMult(fac_data[i], facFactor);
  }

  /* 3) Apply spectrum deshaping using alfd_gains */
  for (i = 0; i < fac_length / 4; i++) {
    int k;

    k = i >> (3 - mod);
    fac_data[i] = fMult(fac_data[i], alfd_gains[k])
                  << 1; /* alfd_gains is scaled by one bit. */
  }
}

static void CFac_CalcFacSignal(FIXP_DBL *pOut, FIXP_DBL *pFac,
                               const int fac_scale, const int fac_length,
                               const FIXP_LPC A[M_LP_FILTER_ORDER],
                               const INT A_exp, const int fAddZir,
                               const int isFdFac) {
  FIXP_LPC wA[M_LP_FILTER_ORDER];
  FIXP_DBL tf_gain = (FIXP_DBL)0;
  int wlength;
  int scale = fac_scale;

  /* obtain tranform gain. */
  imdct_gain(&tf_gain, &scale, isFdFac ? 0 : fac_length);

  /* 4) Compute inverse DCT-IV of FAC data. Output scale of DCT IV is 16 bits.
   */
  dct_IV(pFac, fac_length, &scale);
  /* dct_IV scale = log2(fac_length). "- 7" is a factor of 2/128 */
  if (tf_gain != (FIXP_DBL)0) { /* non-radix 2 transform gain */
    int i;

    for (i = 0; i < fac_length; i++) {
      pFac[i] = fMult(tf_gain, pFac[i]);
    }
  }
  scaleValuesSaturate(pOut, pFac, fac_length,
                      scale); /* Avoid overflow issues and saturate. */

  E_LPC_a_weight(wA, A, M_LP_FILTER_ORDER);

  /* We need the output of the IIR filter to be longer than "fac_length".
  For this reason we run it with zero input appended to the end of the input
  sequence, i.e. we generate its ZIR and extend the output signal.*/
  FDKmemclear(pOut + fac_length, fac_length * sizeof(FIXP_DBL));
  wlength = 2 * fac_length;

  /* 5) Apply weighted synthesis filter to FAC data, including optional Zir (5.
   * item 4). */
  Syn_filt_zero(wA, A_exp, wlength, pOut);
}

INT CLpd_FAC_Mdct2Acelp(H_MDCT hMdct, FIXP_DBL *output, FIXP_DBL *pFac,
                        const int fac_scale, FIXP_LPC *A, INT A_exp,
                        INT nrOutSamples, const INT fac_length,
                        const INT isFdFac, UCHAR prevWindowShape) {
  FIXP_DBL *pOvl;
  FIXP_DBL *pOut0;
  const FIXP_WTP *pWindow;
  int i, fl, nrSamples = 0;

  FDK_ASSERT(fac_length <= 1024 / (4 * 2));

  fl = fac_length * 2;

  pWindow = FDKgetWindowSlope(fl, prevWindowShape);

  /* Adapt window slope length in case of frame loss. */
  if (hMdct->prev_fr != fl) {
    int nl = 0;
    imdct_adapt_parameters(hMdct, &fl, &nl, fac_length, pWindow, nrOutSamples);
    FDK_ASSERT(nl == 0);
  }

  if (nrSamples < nrOutSamples) {
    pOut0 = output;
    nrSamples += hMdct->ov_offset;
    /* Purge buffered output. */
    FDKmemcpy(pOut0, hMdct->overlap.time, hMdct->ov_offset * sizeof(pOut0[0]));
    hMdct->ov_offset = 0;
  }

  pOvl = hMdct->overlap.freq + hMdct->ov_size - 1;

  if (nrSamples >= nrOutSamples) {
    pOut0 = hMdct->overlap.time + hMdct->ov_offset;
    hMdct->ov_offset += hMdct->prev_nr + fl / 2;
  } else {
    pOut0 = output + nrSamples;
    nrSamples += hMdct->prev_nr + fl / 2;
  }
  if (hMdct->prevPrevAliasSymmetry == 0) {
    for (i = 0; i < hMdct->prev_nr; i++) {
      FIXP_DBL x = -(*pOvl--);
      *pOut0 = IMDCT_SCALE_DBL(x);
      pOut0++;
    }
  } else {
    for (i = 0; i < hMdct->prev_nr; i++) {
      FIXP_DBL x = (*pOvl--);
      *pOut0 = IMDCT_SCALE_DBL(x);
      pOut0++;
    }
  }
  hMdct->prev_nr = 0;

  {
    if (pFac != NULL) {
      /* Note: The FAC gain might have been applied directly after bit stream
       * parse in this case. */
      CFac_CalcFacSignal(pOut0, pFac, fac_scale, fac_length, A, A_exp, 0,
                         isFdFac);
    } else {
      /* Clear buffer because of the overlap and ADD! */
      FDKmemclear(pOut0, fac_length * sizeof(FIXP_DBL));
    }
  }

  i = 0;

  if (hMdct->prevPrevAliasSymmetry == 0) {
    for (; i < fl / 2; i++) {
      FIXP_DBL x0;

      /* Overlap Add */
      x0 = -fMult(*pOvl--, pWindow[i].v.re);

      *pOut0 = fAddSaturate(*pOut0, IMDCT_SCALE_DBL(x0));
      pOut0++;
    }
  } else {
    for (; i < fl / 2; i++) {
      FIXP_DBL x0;

      /* Overlap Add */
      x0 = fMult(*pOvl--, pWindow[i].v.re);

      *pOut0 = fAddSaturate(*pOut0, IMDCT_SCALE_DBL(x0));
      pOut0++;
    }
  }
  if (hMdct->pFacZir !=
      0) { /* this should only happen for ACELP -> TCX20 -> ACELP transition */
    FIXP_DBL *pOut = pOut0 - fl / 2; /* fl/2 == fac_length */
    for (i = 0; i < fl / 2; i++) {
      pOut[i] = fAddSaturate(pOut[i], IMDCT_SCALE_DBL(hMdct->pFacZir[i]));
    }
    hMdct->pFacZir = NULL;
  }

  hMdct->prev_fr = 0;
  hMdct->prev_nr = 0;
  hMdct->prev_tl = 0;
  hMdct->prevPrevAliasSymmetry = hMdct->prevAliasSymmetry;

  return nrSamples;
}

INT CLpd_FAC_Acelp2Mdct(H_MDCT hMdct, FIXP_DBL *output, FIXP_DBL *_pSpec,
                        const SHORT spec_scale[], const int nSpec,
                        FIXP_DBL *pFac, const int fac_scale,
                        const INT fac_length, INT noOutSamples, const INT tl,
                        const FIXP_WTP *wrs, const INT fr, FIXP_LPC A[16],
                        INT A_exp, CAcelpStaticMem *acelp_mem,
                        const FIXP_DBL gain, const int last_frame_lost,
                        const int isFdFac, const UCHAR last_lpd_mode,
                        const int k, int currAliasingSymmetry) {
  FIXP_DBL *pCurr, *pOvl, *pSpec;
  const FIXP_WTP *pWindow;
  const FIXP_WTB *FacWindowZir_conceal;
  UCHAR doFacZirConceal = 0;
  int doDeemph = 1;
  const FIXP_WTB *FacWindowZir, *FacWindowSynth;
  FIXP_DBL *pOut0 = output, *pOut1;
  int w, i, fl, nl, nr, f_len, nrSamples = 0, s = 0, scale, total_gain_e;
  FIXP_DBL *pF, *pFAC_and_FAC_ZIR = NULL;
  FIXP_DBL total_gain = gain;

  FDK_ASSERT(fac_length <= 1024 / (4 * 2));
  switch (fac_length) {
    /* coreCoderFrameLength = 1024 */
    case 128:
      pWindow = SineWindow256;
      FacWindowZir = FacWindowZir128;
      FacWindowSynth = FacWindowSynth128;
      break;
    case 64:
      pWindow = SineWindow128;
      FacWindowZir = FacWindowZir64;
      FacWindowSynth = FacWindowSynth64;
      break;
    case 32:
      pWindow = SineWindow64;
      FacWindowZir = FacWindowZir32;
      FacWindowSynth = FacWindowSynth32;
      break;
    /* coreCoderFrameLength = 768 */
    case 96:
      pWindow = SineWindow192;
      FacWindowZir = FacWindowZir96;
      FacWindowSynth = FacWindowSynth96;
      break;
    case 48:
      pWindow = SineWindow96;
      FacWindowZir = FacWindowZir48;
      FacWindowSynth = FacWindowSynth48;
      break;
    default:
      FDK_ASSERT(0);
      return 0;
  }

  FacWindowZir_conceal = FacWindowSynth;
  /* Derive NR and NL */
  fl = fac_length * 2;
  nl = (tl - fl) >> 1;
  nr = (tl - fr) >> 1;

  if (noOutSamples > nrSamples) {
    /* Purge buffered output. */
    FDKmemcpy(pOut0, hMdct->overlap.time, hMdct->ov_offset * sizeof(pOut0[0]));
    nrSamples = hMdct->ov_offset;
    hMdct->ov_offset = 0;
  }

  if (nrSamples >= noOutSamples) {
    pOut1 = hMdct->overlap.time + hMdct->ov_offset;
    if (hMdct->ov_offset < fac_length) {
      pOut0 = output + nrSamples;
    } else {
      pOut0 = pOut1;
    }
    hMdct->ov_offset += fac_length + nl;
  } else {
    pOut1 = output + nrSamples;
    pOut0 = output + nrSamples;
  }

  {
    pFAC_and_FAC_ZIR = CLpd_ACELP_GetFreeExcMem(acelp_mem, 2 * fac_length);
    {
      const FIXP_DBL *pTmp1, *pTmp2;

      doFacZirConceal |= ((last_frame_lost != 0) && (k == 0));
      doDeemph &= (last_lpd_mode != 4);
      if (doFacZirConceal) {
        /* ACELP contribution in concealment case:
           Use ZIR with a modified ZIR window to preserve some more energy.
           Dont use FAC, which contains wrong information for concealed frame
           Dont use last ACELP samples, but double ZIR, instead (afterwards) */
        FDKmemclear(pFAC_and_FAC_ZIR, 2 * fac_length * sizeof(FIXP_DBL));
        FacWindowSynth = (FIXP_WTB *)pFAC_and_FAC_ZIR;
        FacWindowZir = FacWindowZir_conceal;
      } else {
        CFac_CalcFacSignal(pFAC_and_FAC_ZIR, pFac, fac_scale + s, fac_length, A,
                           A_exp, 1, isFdFac);
      }
      /* 6) Get windowed past ACELP samples and ACELP ZIR signal */

      /*
       * Get ACELP ZIR (pFac[]) and ACELP past samples (pOut0[]) and add them
       * to the FAC synth signal contribution on pOut1[].
       */
      {
        {
          CLpd_Acelp_Zir(A, A_exp, acelp_mem, fac_length, pFac, doDeemph);

          pTmp1 = pOut0;
          pTmp2 = pFac;
        }

        for (i = 0, w = 0; i < fac_length; i++) {
          FIXP_DBL x;
          /* Div2 is compensated by table scaling */
          x = fMultDiv2(pTmp2[i], FacWindowZir[w]);
          x += fMultDiv2(pTmp1[-i - 1], FacWindowSynth[w]);
          pOut1[i] = fAddSaturate(x, pFAC_and_FAC_ZIR[i]);
          w++;
        }
      }

      if (doFacZirConceal) {
        /* ZIR is the only ACELP contribution, so double it */
        scaleValues(pOut1, fac_length, 1);
      }
    }
  }

  if (nrSamples < noOutSamples) {
    nrSamples += fac_length + nl;
  }

  /* Obtain transform gain */
  total_gain = gain;
  total_gain_e = 0;
  imdct_gain(&total_gain, &total_gain_e, tl);

  /* IMDCT overlap add */
  scale = total_gain_e;
  pSpec = _pSpec;

  /* Note:when comming from an LPD frame (TCX/ACELP) the previous alisaing
   * symmetry must always be 0 */
  if (currAliasingSymmetry == 0) {
    dct_IV(pSpec, tl, &scale);
  } else {
    FIXP_DBL _tmp[1024 + ALIGNMENT_DEFAULT / sizeof(FIXP_DBL)];
    FIXP_DBL *tmp = (FIXP_DBL *)ALIGN_PTR(_tmp);
    C_ALLOC_ALIGNED_REGISTER(tmp, sizeof(_tmp));
    dst_III(pSpec, tmp, tl, &scale);
    C_ALLOC_ALIGNED_UNREGISTER(tmp);
  }

  /* Optional scaling of time domain - no yet windowed - of current spectrum */
  if (total_gain != (FIXP_DBL)0) {
    for (i = 0; i < tl; i++) {
      pSpec[i] = fMult(pSpec[i], total_gain);
    }
  }
  int loc_scale = fixmin_I(spec_scale[0] + scale, (INT)DFRACT_BITS - 1);
  scaleValuesSaturate(pSpec, tl, loc_scale);

  pOut1 += fl / 2 - 1;
  pCurr = pSpec + tl - fl / 2;

  for (i = 0; i < fl / 2; i++) {
    FIXP_DBL x1;

    /* FAC signal is already on pOut1, because of that the += operator. */
    x1 = fMult(*pCurr++, pWindow[i].v.re);
    FDK_ASSERT((pOut1 >= hMdct->overlap.time &&
                pOut1 < hMdct->overlap.time + hMdct->ov_size) ||
               (pOut1 >= output && pOut1 < output + 1024));
    *pOut1 = fAddSaturate(*pOut1, IMDCT_SCALE_DBL(-x1));
    pOut1--;
  }

  /* NL output samples TL/2+FL/2..TL. - current[FL/2..0] */
  pOut1 += (fl / 2) + 1;

  pFAC_and_FAC_ZIR += fac_length; /* set pointer to beginning of FAC ZIR */

  if (nl == 0) {
    /* save pointer to write FAC ZIR data later */
    hMdct->pFacZir = pFAC_and_FAC_ZIR;
  } else {
    FDK_ASSERT(nl >= fac_length);
    /* FAC ZIR will be added now ... */
    hMdct->pFacZir = NULL;
  }

  pF = pFAC_and_FAC_ZIR;
  f_len = fac_length;

  pCurr = pSpec + tl - fl / 2 - 1;
  for (i = 0; i < nl; i++) {
    FIXP_DBL x = -(*pCurr--);
    /* 5) (item 4) Synthesis filter Zir component, FAC ZIR (another one). */
    if (i < f_len) {
      x = fAddSaturate(x, *pF++);
    }

    FDK_ASSERT((pOut1 >= hMdct->overlap.time &&
                pOut1 < hMdct->overlap.time + hMdct->ov_size) ||
               (pOut1 >= output && pOut1 < output + 1024));
    *pOut1 = IMDCT_SCALE_DBL(x);
    pOut1++;
  }

  hMdct->prev_nr = nr;
  hMdct->prev_fr = fr;
  hMdct->prev_wrs = wrs;
  hMdct->prev_tl = tl;
  hMdct->prevPrevAliasSymmetry = hMdct->prevAliasSymmetry;
  hMdct->prevAliasSymmetry = currAliasingSymmetry;
  fl = fr;
  nl = nr;

  pOvl = pSpec + tl / 2 - 1;
  pOut0 = pOut1;

  for (w = 1; w < nSpec; w++) /* for ACELP -> FD short */
  {
    const FIXP_WTP *pWindow_prev;

    /* Setup window pointers */
    pWindow_prev = hMdct->prev_wrs;

    /* Current spectrum */
    pSpec = _pSpec + w * tl;

    scale = total_gain_e;

    /* For the second, third, etc. short frames the alisaing symmetry is equal,
     * either (0,0) or (1,1) */
    if (currAliasingSymmetry == 0) {
      /* DCT IV of current spectrum */
      dct_IV(pSpec, tl, &scale);
    } else {
      dst_IV(pSpec, tl, &scale);
    }

    /* Optional scaling of time domain - no yet windowed - of current spectrum
     */
    /* and de-scale current spectrum signal (time domain, no yet windowed) */
    if (total_gain != (FIXP_DBL)0) {
      for (i = 0; i < tl; i++) {
        pSpec[i] = fMult(pSpec[i], total_gain);
      }
    }
    loc_scale = fixmin_I(spec_scale[w] + scale, (INT)DFRACT_BITS - 1);
    scaleValuesSaturate(pSpec, tl, loc_scale);

    if (noOutSamples <= nrSamples) {
      /* Divert output first half to overlap buffer if we already got enough
       * output samples. */
      pOut0 = hMdct->overlap.time + hMdct->ov_offset;
      hMdct->ov_offset += hMdct->prev_nr + fl / 2;
    } else {
      /* Account output samples */
      nrSamples += hMdct->prev_nr + fl / 2;
    }

    /* NR output samples 0 .. NR. -overlap[TL/2..TL/2-NR] */
    for (i = 0; i < hMdct->prev_nr; i++) {
      FIXP_DBL x = -(*pOvl--);
      *pOut0 = IMDCT_SCALE_DBL(x);
      pOut0++;
    }

    if (noOutSamples <= nrSamples) {
      /* Divert output second half to overlap buffer if we already got enough
       * output samples. */
      pOut1 = hMdct->overlap.time + hMdct->ov_offset + fl / 2 - 1;
      hMdct->ov_offset += fl / 2 + nl;
    } else {
      pOut1 = pOut0 + (fl - 1);
      nrSamples += fl / 2 + nl;
    }

    /* output samples before window crossing point NR .. TL/2.
     * -overlap[TL/2-NR..TL/2-NR-FL/2] + current[NR..TL/2] */
    /* output samples after window crossing point TL/2 .. TL/2+FL/2.
     * -overlap[0..FL/2] - current[TL/2..FL/2] */
    pCurr = pSpec + tl - fl / 2;
    if (currAliasingSymmetry == 0) {
      for (i = 0; i < fl / 2; i++) {
        FIXP_DBL x0, x1;

        cplxMultDiv2(&x1, &x0, *pCurr++, -*pOvl--, pWindow_prev[i]);
        *pOut0 = IMDCT_SCALE_DBL_LSH1(x0);
        *pOut1 = IMDCT_SCALE_DBL_LSH1(-x1);
        pOut0++;
        pOut1--;
      }
    } else {
      if (hMdct->prevPrevAliasSymmetry == 0) {
        /* Jump DST II -> DST IV for the second window */
        for (i = 0; i < fl / 2; i++) {
          FIXP_DBL x0, x1;

          cplxMultDiv2(&x1, &x0, *pCurr++, -*pOvl--, pWindow_prev[i]);
          *pOut0 = IMDCT_SCALE_DBL_LSH1(x0);
          *pOut1 = IMDCT_SCALE_DBL_LSH1(x1);
          pOut0++;
          pOut1--;
        }
      } else {
        /* Jump DST IV -> DST IV from the second window on */
        for (i = 0; i < fl / 2; i++) {
          FIXP_DBL x0, x1;

          cplxMultDiv2(&x1, &x0, *pCurr++, *pOvl--, pWindow_prev[i]);
          *pOut0 = IMDCT_SCALE_DBL_LSH1(x0);
          *pOut1 = IMDCT_SCALE_DBL_LSH1(x1);
          pOut0++;
          pOut1--;
        }
      }
    }

    if (hMdct->pFacZir != 0) {
      /* add FAC ZIR of previous ACELP -> mdct transition */
      FIXP_DBL *pOut = pOut0 - fl / 2;
      FDK_ASSERT(fl / 2 <= 128);
      for (i = 0; i < fl / 2; i++) {
        pOut[i] = fAddSaturate(pOut[i], IMDCT_SCALE_DBL(hMdct->pFacZir[i]));
      }
      hMdct->pFacZir = NULL;
    }
    pOut0 += (fl / 2);

    /* NL output samples TL/2+FL/2..TL. - current[FL/2..0] */
    pOut1 += (fl / 2) + 1;
    pCurr = pSpec + tl - fl / 2 - 1;
    for (i = 0; i < nl; i++) {
      FIXP_DBL x = -(*pCurr--);
      *pOut1 = IMDCT_SCALE_DBL(x);
      pOut1++;
    }

    /* Set overlap source pointer for next window pOvl = pSpec + tl/2 - 1; */
    pOvl = pSpec + tl / 2 - 1;

    /* Previous window values. */
    hMdct->prev_nr = nr;
    hMdct->prev_fr = fr;
    hMdct->prev_tl = tl;
    hMdct->prev_wrs = pWindow_prev;
    hMdct->prevPrevAliasSymmetry = hMdct->prevAliasSymmetry;
    hMdct->prevAliasSymmetry = currAliasingSymmetry;
  }

  /* Save overlap */

  pOvl = hMdct->overlap.freq + hMdct->ov_size - tl / 2;
  FDK_ASSERT(pOvl >= hMdct->overlap.time + hMdct->ov_offset);
  FDK_ASSERT(tl / 2 <= hMdct->ov_size);
  for (i = 0; i < tl / 2; i++) {
    pOvl[i] = _pSpec[i + (w - 1) * tl];
  }

  return nrSamples;
}
