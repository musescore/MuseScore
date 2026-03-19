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

#include "usacdec_lpd.h"

#include "usacdec_rom.h"
#include "usacdec_fac.h"
#include "usacdec_lpc.h"
#include "FDK_tools_rom.h"
#include "fft.h"
#include "mdct.h"
#include "usacdec_acelp.h"
#include "overlapadd.h"

#include "conceal.h"

#include "block.h"

#define SF_PITCH_TRACK 6
#define SF_GAIN 3
#define MIN_VAL FL2FXCONST_DBL(0.0f)
#define MAX_VAL (FIXP_DBL) MAXVAL_DBL

#include "ac_arith_coder.h"

void filtLP(const FIXP_DBL *syn, PCM_DEC *syn_out, FIXP_DBL *noise,
            const FIXP_SGL *filt, const INT aacOutDataHeadroom, INT stop,
            int len) {
  INT i, j;
  FIXP_DBL tmp;

  FDK_ASSERT((aacOutDataHeadroom - 1) >= -(MDCT_OUTPUT_SCALE));

  for (i = 0; i < stop; i++) {
    tmp = fMultDiv2(noise[i], filt[0]);  // Filt in Q-1.16
    for (j = 1; j <= len; j++) {
      tmp += fMult((noise[i - j] >> 1) + (noise[i + j] >> 1), filt[j]);
    }
    syn_out[i] = (PCM_DEC)(
        IMDCT_SCALE((syn[i] >> 1) - (tmp >> 1), aacOutDataHeadroom - 1));
  }
}

void bass_pf_1sf_delay(
    FIXP_DBL *syn,   /* (i) : 12.8kHz synthesis to postfilter              */
    const INT *T_sf, /* (i) : Pitch period for all subframes (T_sf[16])    */
    FIXP_DBL *pit_gain,
    const int frame_length, /* (i) : frame length (should be 768|1024) */
    const INT l_frame,
    const INT l_next,   /* (i) : look ahead for symmetric filtering           */
    PCM_DEC *synth_out, /* (o) : filtered synthesis (with delay of 1 subfr)   */
    const INT aacOutDataHeadroom, /* (i) : headroom of the output time signal to
                                     prevent clipping */
    FIXP_DBL mem_bpf[]) /* i/o : memory state [L_FILT+L_SUBFR]                */
{
  INT i, sf, i_subfr, T, T2, lg;

  FIXP_DBL tmp, ener, corr, gain;
  FIXP_DBL *noise, *noise_in;
  FIXP_DBL
  noise_buf[L_FILT + (2 * L_SUBFR)];  // L_FILT = 12, L_SUBFR = 64 => 140
  const FIXP_DBL *x, *y;

  {
    noise = noise_buf + L_FILT;  // L_FILT = 12 delay of upsampling filter
    noise_in = noise_buf + L_FILT + L_SUBFR;
    /* Input scaling of the BPF memory */
    scaleValues(mem_bpf, (L_FILT + L_SUBFR), 1);
  }

  int gain_exp = 17;

  sf = 0;
  for (i_subfr = 0; i_subfr < l_frame; i_subfr += L_SUBFR, sf++) {
    T = T_sf[sf];
    gain = pit_gain[sf];

    /* Gain is in Q17.14 */
    /* If gain > 1 set to 1 */
    if (gain > (FIXP_DBL)(1 << 14)) gain = (FIXP_DBL)(1 << 14);

    /* If gain < 0 set to 0 */
    if (gain < (FIXP_DBL)0) gain = (FIXP_DBL)0;

    if (gain > (FIXP_DBL)0) {
      /* pitch tracker: test pitch/2 to avoid continuous pitch doubling */
      /* Note: pitch is limited to PIT_MIN (34 = 376Hz) at the encoder  */
      T2 = T >> 1;
      x = &syn[i_subfr - L_EXTRA];
      y = &syn[i_subfr - T2 - L_EXTRA];

      ener = (FIXP_DBL)0;
      corr = (FIXP_DBL)0;
      tmp = (FIXP_DBL)0;

      int headroom_x = getScalefactor(x, L_SUBFR + L_EXTRA);
      int headroom_y = getScalefactor(y, L_SUBFR + L_EXTRA);

      int width_shift = 7;

      for (i = 0; i < (L_SUBFR + L_EXTRA); i++) {
        ener += fPow2Div2((x[i] << headroom_x)) >> width_shift;
        corr += fMultDiv2((x[i] << headroom_x), (y[i] << headroom_y)) >>
                width_shift;
        tmp += fPow2Div2((y[i] << headroom_y)) >> width_shift;
      }

      int exp_ener = ((17 - headroom_x) << 1) + width_shift + 1;
      int exp_corr = (17 - headroom_x) + (17 - headroom_y) + width_shift + 1;
      int exp_tmp = ((17 - headroom_y) << 1) + width_shift + 1;

      /* Add 0.01 to "ener". Adjust exponents */
      FIXP_DBL point_zero_one = (FIXP_DBL)0x51eb851f; /* In Q-6.37 */
      int diff;
      ener = fAddNorm(ener, exp_ener, point_zero_one, -6, &exp_ener);
      corr = fAddNorm(corr, exp_corr, point_zero_one, -6, &exp_corr);
      tmp = fAddNorm(tmp, exp_tmp, point_zero_one, -6, &exp_tmp);

      /* use T2 if normalized correlation > 0.95 */
      INT s1, s2;
      s1 = CntLeadingZeros(ener) - 1;
      s2 = CntLeadingZeros(tmp) - 1;

      FIXP_DBL ener_by_tmp = fMultDiv2(ener << s1, tmp << s2);
      int ener_by_tmp_exp = (exp_ener - s1) + (exp_tmp - s2) + 1;

      if (ener_by_tmp_exp & 1) {
        ener_by_tmp <<= 1;
        ener_by_tmp_exp -= 1;
      }

      int temp_exp = 0;

      FIXP_DBL temp1 = invSqrtNorm2(ener_by_tmp, &temp_exp);

      int temp1_exp = temp_exp - (ener_by_tmp_exp >> 1);

      FIXP_DBL tmp_result = fMult(corr, temp1);

      int tmp_result_exp = exp_corr + temp1_exp;

      diff = tmp_result_exp - 0;
      FIXP_DBL point95 = FL2FXCONST_DBL(0.95f);
      if (diff >= 0) {
        diff = fMin(diff, 31);
        point95 = FL2FXCONST_DBL(0.95f) >> diff;
      } else {
        diff = fMax(diff, -31);
        tmp_result >>= (-diff);
      }

      if (tmp_result > point95) T = T2;

      /* prevent that noise calculation below reaches into not defined signal
         parts at the end of the synth_buf or in other words restrict the below
         used index (i+i_subfr+T) < l_frame + l_next
      */
      lg = l_frame + l_next - T - i_subfr;

      if (lg > L_SUBFR)
        lg = L_SUBFR;
      else if (lg < 0)
        lg = 0;

      /* limit gain to avoid problem on burst */
      if (lg > 0) {
        FIXP_DBL tmp1;

        /* max(lg) = 64 => scale with 6 bits minus 1 (fPow2Div2) */

        s1 = getScalefactor(&syn[i_subfr], lg);
        s2 = getScalefactor(&syn[i_subfr + T], lg);
        INT s = fixMin(s1, s2);

        tmp = (FIXP_DBL)0;
        ener = (FIXP_DBL)0;
        for (i = 0; i < lg; i++) {
          tmp += fPow2Div2(syn[i + i_subfr] << s1) >> (SF_PITCH_TRACK);
          ener += fPow2Div2(syn[i + i_subfr + T] << s2) >> (SF_PITCH_TRACK);
        }
        tmp = tmp >> fMin(DFRACT_BITS - 1, (2 * (s1 - s)));
        ener = ener >> fMin(DFRACT_BITS - 1, (2 * (s2 - s)));

        /* error robustness: for the specific case syn[...] == -1.0f for all 64
           samples ener or tmp might overflow and become negative. For all sane
           cases we have enough headroom.
        */
        if (ener <= (FIXP_DBL)0) {
          ener = (FIXP_DBL)1;
        }
        if (tmp <= (FIXP_DBL)0) {
          tmp = (FIXP_DBL)1;
        }
        FDK_ASSERT(ener > (FIXP_DBL)0);

        /* tmp = sqrt(tmp/ener) */
        int result_e = 0;
        tmp1 = fDivNorm(tmp, ener, &result_e);
        if (result_e & 1) {
          tmp1 >>= 1;
          result_e += 1;
        }
        tmp = sqrtFixp(tmp1);
        result_e >>= 1;

        gain_exp = 17;

        diff = result_e - gain_exp;

        FIXP_DBL gain1 = gain;

        if (diff >= 0) {
          diff = fMin(diff, 31);
          gain1 >>= diff;
        } else {
          result_e += (-diff);
          diff = fMax(diff, -31);
          tmp >>= (-diff);
        }

        if (tmp < gain1) {
          gain = tmp;
          gain_exp = result_e;
        }
      }

      /* calculate noise based on voiced pitch */
      /* fMultDiv2() replaces weighting of gain with 0.5 */
      diff = gain_exp - 17;
      if (diff >= 0) {
        gain <<= diff;
      } else {
        gain >>= (-diff);
      }

      s1 = CntLeadingZeros(gain) - 1;
      s1 -= 16; /* Leading bits for SGL */

      FIXP_SGL gainSGL = FX_DBL2FX_SGL(gain << 16);

      gainSGL = gainSGL << s1;

      {
        for (i = 0; i < lg; i++) {
          /* scaled with SF_SYNTH + gain_sf + 1; composition of scalefactor 2:
           * one additional shift of syn values + fMult => fMultDiv2 */
          noise_in[i] =
              scaleValue(fMultDiv2(gainSGL, (syn[i + i_subfr] >> 1) -
                                                (syn[i + i_subfr - T] >> 2) -
                                                (syn[i + i_subfr + T] >> 2)),
                         2 - s1);
        }

        for (i = lg; i < L_SUBFR; i++) {
          /* scaled with SF_SYNTH + gain_sf + 1; composition of scalefactor 2:
           * one additional shift of syn values + fMult => fMultDiv2 */
          noise_in[i] =
              scaleValue(fMultDiv2(gainSGL, (syn[i + i_subfr] >> 1) -
                                                (syn[i + i_subfr - T] >> 1)),
                         2 - s1);
        }
      }
    } else {
      FDKmemset(noise_in, (FIXP_DBL)0, L_SUBFR * sizeof(FIXP_DBL));
    }

    {
      FDKmemcpy(noise_buf, mem_bpf, (L_FILT + L_SUBFR) * sizeof(FIXP_DBL));

      FDKmemcpy(mem_bpf, noise_buf + L_SUBFR,
                (L_FILT + L_SUBFR) * sizeof(FIXP_DBL));
    }

    /* substract from voiced speech low-pass filtered noise */
    /* filter coefficients are scaled with factor SF_FILT_LP (1) */

    {
      filtLP(&syn[i_subfr - L_SUBFR], &synth_out[i_subfr], noise,
             fdk_dec_filt_lp, aacOutDataHeadroom, L_SUBFR, L_FILT);
    }
  }

  {

  }

  // To be determined (info from Ben)
  {
    /* Output scaling of the BPF memory */
    scaleValues(mem_bpf, (L_FILT + L_SUBFR), -1);
    /* Copy the rest of the signal (after the fac) */
    scaleValuesSaturate(
        (PCM_DEC *)&synth_out[l_frame], (FIXP_DBL *)&syn[l_frame - L_SUBFR],
        (frame_length - l_frame), MDCT_OUT_HEADROOM - aacOutDataHeadroom);
  }

  return;
}

/*
 * Frequency Domain Noise Shaping
 */

/**
 * \brief Adaptive Low Frequencies Deemphasis of spectral coefficients.
 *
 * Ensure quantization of low frequencies in case where the
 * signal dynamic is higher than the LPC noise shaping.
 * This is the inverse operation of adap_low_freq_emph().
 * Output gain of all blocks.
 *
 * \param x pointer to the spectral coefficients, requires 1 bit headroom.
 * \param lg length of x.
 * \param bUseNewAlfe if set, apply ALFD for fullband lpd.
 * \param gainLpc1 pointer to gain based on old input LPC coefficients.
 * \param gainLpc2 pointer to gain based on new input LPC coefficients.
 * \param alfd_gains pointer to output gains.
 * \param s current scale shift factor of x.
 */
#define ALFDPOW2_SCALE 3
/*static*/
void CLpd_AdaptLowFreqDeemph(FIXP_DBL x[], int lg, FIXP_DBL alfd_gains[],
                             INT s) {
  {
    int i, j, k, i_max;
    FIXP_DBL max, fac;
    /* Note: This stack array saves temporary accumulation results to be used in
     * a second run */
    /*       The size should be limited to (1024/4)/8=32 */
    FIXP_DBL tmp_pow2[32];

    s = s * 2 + ALFDPOW2_SCALE;
    s = fMin(31, s);

    k = 8;
    i_max = lg / 4; /* ALFD range = 1600Hz (lg = 6400Hz) */

    /* find spectral peak */
    max = FL2FX_DBL(0.01f) >> s;
    for (i = 0; i < i_max; i += k) {
      FIXP_DBL tmp;

      tmp = FIXP_DBL(0);
      FIXP_DBL *pX = &x[i];

      j = 8;
      do {
        FIXP_DBL x0 = *pX++;
        FIXP_DBL x1 = *pX++;
        x0 = fPow2Div2(x0);
        x1 = fPow2Div2(x1);
        tmp = tmp + (x0 >> (ALFDPOW2_SCALE - 1));
        tmp = tmp + (x1 >> (ALFDPOW2_SCALE - 1));
      } while ((j = j - 2) != 0);
      tmp = fMax(tmp, (FL2FX_DBL(0.01f) >> s));
      tmp_pow2[i >> 3] = tmp;
      if (tmp > max) {
        max = tmp;
      }
    }

    /* deemphasis of all blocks below the peak */
    fac = FL2FX_DBL(0.1f) >> 1;
    for (i = 0; i < i_max; i += k) {
      FIXP_DBL tmp;
      INT shifti;

      tmp = tmp_pow2[i >> 3];

      /* tmp = (float)sqrt(tmp/max); */

      /* value of tmp is between 8/2*max^2 and max^2 / 2. */
      /* required shift factor of division can grow up to 27
         (grows exponentially for values toward zero)
         thus using normalized division to assure valid result. */
      {
        INT sd;

        if (tmp != (FIXP_DBL)0) {
          tmp = fDivNorm(max, tmp, &sd);
          if (sd & 1) {
            sd++;
            tmp >>= 1;
          }
        } else {
          tmp = (FIXP_DBL)MAXVAL_DBL;
          sd = 0;
        }
        tmp = invSqrtNorm2(tmp, &shifti);
        tmp = scaleValue(tmp, shifti - 1 - (sd / 2));
      }
      if (tmp > fac) {
        fac = tmp;
      }
      FIXP_DBL *pX = &x[i];

      j = 8;
      do {
        FIXP_DBL x0 = pX[0];
        FIXP_DBL x1 = pX[1];
        x0 = fMultDiv2(x0, fac);
        x1 = fMultDiv2(x1, fac);
        x0 = x0 << 2;
        x1 = x1 << 2;
        *pX++ = x0;
        *pX++ = x1;

      } while ((j = j - 2) != 0);
      /* Store gains for FAC */
      *alfd_gains++ = fac;
    }
  }
}

/**
 * \brief Interpolated Noise Shaping for mdct coefficients.
 * This algorithm shapes temporally the spectral noise between
 * the two spectral noise represention (FDNS_NPTS of resolution).
 * The noise is shaped monotonically between the two points
 * using a curved shape to favor the lower gain in mid-frame.
 * ODFT and amplitud calculation are applied to the 2 LPC coefficients first.
 *
 * \param r pointer to spectrum data.
 * \param rms RMS of output spectrum.
 * \param lg length of r.
 * \param A1 pointer to old input LPC coefficients of length M_LP_FILTER_ORDER
 * scaled by SF_A_COEFFS.
 * \param A2 pointer to new input LPC coefficients of length M_LP_FILTER_ORDER
 * scaled by SF_A_COEFFS.
 * \param bLpc2Mdct flags control lpc2mdct conversion and noise shaping.
 * \param gainLpc1 pointer to gain based on old input LPC coefficients.
 * \param gainLpc2 pointer to gain based on new input LPC coefficients.
 * \param gLpc_e pointer to exponent of gainLpc1 and gainLpc2.
 */
/* static */
#define NSHAPE_SCALE (4)

#define LPC2MDCT_CALC (1)
#define LPC2MDCT_GAIN_LOAD (2)
#define LPC2MDCT_GAIN_SAVE (4)
#define LPC2MDCT_APPLY_NSHAPE (8)

void lpc2mdctAndNoiseShaping(FIXP_DBL *r, SHORT *pScale, const INT lg,
                             const INT fdns_npts, const FIXP_LPC *A1,
                             const INT A1_exp, const FIXP_LPC *A2,
                             const INT A2_exp) {
  FIXP_DBL *tmp2 = NULL;
  FIXP_DBL rr_minus_one;
  int i, k, s, step;

  C_AALLOC_SCRATCH_START(tmp1, FIXP_DBL, FDNS_NPTS * 8)

  {
    tmp2 = tmp1 + fdns_npts * 4;

    /* lpc2mdct() */

    /* ODFT. E_LPC_a_weight() for A1 and A2 vectors is included into the loop
     * below. */
    FIXP_DBL f = FL2FXCONST_DBL(0.92f);

    const FIXP_STP *SinTab;
    int k_step;
    /* needed values: sin(phi), cos(phi); phi = i*PI/(2*fdns_npts), i = 0 ...
     * M_LP_FILTER_ORDER */
    switch (fdns_npts) {
      case 64:
        SinTab = SineTable512;
        k_step = (512 / 64);
        FDK_ASSERT(512 >= 64);
        break;
      case 48:
        SinTab = SineTable384;
        k_step = 384 / 48;
        FDK_ASSERT(384 >= 48);
        break;
      default:
        FDK_ASSERT(0);
        return;
    }

    for (i = 0, k = k_step; i < M_LP_FILTER_ORDER; i++, k += k_step) {
      FIXP_STP cs = SinTab[k];
      FIXP_DBL wA1, wA2;

      wA1 = fMult(A1[i], f);
      wA2 = fMult(A2[i], f);

      /* r[i] = A[i]*cos() */
      tmp1[2 + i * 2] = fMult(wA1, cs.v.re);
      tmp2[2 + i * 2] = fMult(wA2, cs.v.re);
      /* i[i] = A[i]*sin() */
      tmp1[3 + i * 2] = -fMult(wA1, cs.v.im);
      tmp2[3 + i * 2] = -fMult(wA2, cs.v.im);

      f = fMult(f, FL2FXCONST_DBL(0.92f));
    }

    /* Guarantee at least 2 bits of headroom for the FFT */
    /* "3" stands for 1.0 with 2 bits of headroom; (A1_exp + 2) guarantess 2
     * bits of headroom if A1_exp > 1 */
    int A1_exp_fix = fMax(3, A1_exp + 2);
    int A2_exp_fix = fMax(3, A2_exp + 2);

    /* Set 1.0 in the proper format */
    tmp1[0] = (FIXP_DBL)(INT)((ULONG)0x80000000 >> A1_exp_fix);
    tmp2[0] = (FIXP_DBL)(INT)((ULONG)0x80000000 >> A2_exp_fix);

    tmp1[1] = tmp2[1] = (FIXP_DBL)0;

    /* Clear the resto of the array */
    FDKmemclear(
        tmp1 + 2 * (M_LP_FILTER_ORDER + 1),
        2 * (fdns_npts * 2 - (M_LP_FILTER_ORDER + 1)) * sizeof(FIXP_DBL));
    FDKmemclear(
        tmp2 + 2 * (M_LP_FILTER_ORDER + 1),
        2 * (fdns_npts * 2 - (M_LP_FILTER_ORDER + 1)) * sizeof(FIXP_DBL));

    /* Guarantee 2 bits of headroom for FFT */
    scaleValues(&tmp1[2], (2 * M_LP_FILTER_ORDER), (A1_exp - A1_exp_fix));
    scaleValues(&tmp2[2], (2 * M_LP_FILTER_ORDER), (A2_exp - A2_exp_fix));

    INT s2;
    s = A1_exp_fix;
    s2 = A2_exp_fix;

    fft(2 * fdns_npts, tmp1, &s);
    fft(2 * fdns_npts, tmp2, &s2);

    /* Adjust the exponents of both fft outputs if necessary*/
    if (s > s2) {
      scaleValues(tmp2, 2 * fdns_npts, s2 - s);
      s2 = s;
    } else if (s < s2) {
      scaleValues(tmp1, 2 * fdns_npts, s - s2);
      s = s2;
    }

    FDK_ASSERT(s == s2);
  }

  /* Get amplitude and apply gains */
  step = lg / fdns_npts;
  rr_minus_one = (FIXP_DBL)0;

  for (k = 0; k < fdns_npts; k++) {
    FIXP_DBL g1, g2, inv_g1_g2, a, b;
    INT inv_g1_g2_e;
    int g_e, shift;

    {
      FIXP_DBL real, imag;
      int si1, si2, sInput;

      real = tmp1[k * 2];
      imag = tmp1[k * 2 + 1];
      sInput = fMax(fMin(fNorm(real), fNorm(imag)) - 1, 0);
      real <<= sInput;
      imag <<= sInput;
      /* g1_e = si1 - 2*s/2 */
      g1 = invSqrtNorm2(fPow2(real) + fPow2(imag), &si1);
      si1 += sInput;

      real = tmp2[k * 2];
      imag = tmp2[k * 2 + 1];
      sInput = fMax(fMin(fNorm(real), fNorm(imag)) - 1, 0);
      real <<= sInput;
      imag <<= sInput;
      /* g2_e = si2 - 2*s/2 */
      g2 = invSqrtNorm2(fPow2(real) + fPow2(imag), &si2);
      si2 += sInput;

      /* Pick a common scale factor for g1 and g2 */
      if (si1 > si2) {
        g2 >>= si1 - si2;
        g_e = si1 - s;
      } else {
        g1 >>= si2 - si1;
        g_e = si2 - s;
      }
    }

    /* end of lpc2mdct() */

    FDK_ASSERT(g1 >= (FIXP_DBL)0);
    FDK_ASSERT(g2 >= (FIXP_DBL)0);

    /* mdct_IntNoiseShaping() */
    {
      /* inv_g1_g2 * 2^inv_g1_g2_e = 1/(g1+g2) */
      inv_g1_g2 = (g1 >> 1) + (g2 >> 1);
      if (inv_g1_g2 != (FIXP_DBL)0) {
        inv_g1_g2 = fDivNorm(FL2FXCONST_DBL(0.5f), inv_g1_g2, &inv_g1_g2_e);
        inv_g1_g2_e = inv_g1_g2_e - g_e;
      } else {
        inv_g1_g2 = (FIXP_DBL)MAXVAL_DBL;
        inv_g1_g2_e = 0;
      }

      if (g_e < 0) {
        /* a_e = g_e + inv_g1_g2_e + 1 */
        a = scaleValue(fMult(fMult(g1, g2), inv_g1_g2), g_e);
        /* b_e = g_e + inv_g1_g2_e */
        b = fMult(g2 - g1, inv_g1_g2);
        shift = g_e + inv_g1_g2_e + 1 - NSHAPE_SCALE;
      } else {
        /* a_e = (g_e+g_e) + inv_g1_g2_e + 1 */
        a = fMult(fMult(g1, g2), inv_g1_g2);
        /* b_e = (g_e+g_e) + inv_g1_g2_e */
        b = scaleValue(fMult(g2 - g1, inv_g1_g2), -g_e);
        shift = (g_e + g_e) + inv_g1_g2_e + 1 - NSHAPE_SCALE;
      }

      for (i = k * step; i < (k + 1) * step; i++) {
        FIXP_DBL tmp;

        /* rr[i] = 2*a*r[i] + b*rr[i-1] */
        tmp = fMult(a, r[i]);
        tmp += scaleValue(fMultDiv2(b, rr_minus_one), NSHAPE_SCALE);
        tmp = scaleValueSaturate(tmp, shift);
        rr_minus_one = tmp;
        r[i] = tmp;
      }
    }
  }

  /* end of mdct_IntNoiseShaping() */
  { *pScale += NSHAPE_SCALE; }

  C_AALLOC_SCRATCH_END(tmp1, FIXP_DBL, FDNS_NPTS * 8)
}

/**
 * \brief Calculates the energy.
 * \param r pointer to spectrum.
 * \param rs scale factor of spectrum r.
 * \param lg frame length in audio samples.
 * \param rms_e pointer to exponent of energy value.
 * \return mantissa of energy value.
 */
static FIXP_DBL calcEnergy(const FIXP_DBL *r, const SHORT rs, const INT lg,
                           INT *rms_e) {
  int headroom = getScalefactor(r, lg);

  FIXP_DBL rms_m = 0;

  /* Calculate number of growth bits due to addition */
  INT shift = (INT)(fNormz((FIXP_DBL)lg));
  shift = 31 - shift;

  /* Generate 1e-2 in Q-6.37 */
  const FIXP_DBL value0_01 = 0x51eb851e;
  const INT value0_01_exp = -6;

  /* Find the exponent of the resulting energy value */
  *rms_e = ((rs - headroom) << 1) + shift + 1;

  INT delta = *rms_e - value0_01_exp;
  if (delta > 0) {
    /* Limit shift_to 31*/
    delta = fMin(31, delta);
    rms_m = value0_01 >> delta;
  } else {
    rms_m = value0_01;
    *rms_e = value0_01_exp;
    shift = shift - delta;
    /* Limit shift_to 31*/
    shift = fMin(31, shift);
  }

  for (int i = 0; i < lg; i++) {
    rms_m += fPow2Div2(r[i] << headroom) >> shift;
  }

  return rms_m;
}

/**
 * \brief TCX gain calculation.
 * \param pAacDecoderChannelInfo channel context data.
 * \param r output spectrum.
 * \param rms_e pointer to mantissa of energy value.
 * \param rms_e pointer to exponent of energy value.
 * \param frame the frame index of the LPD super frame.
 * \param lg the frame length in audio samples.
 * \param gain_m pointer to mantissa of TCX gain.
 * \param gain_e pointer to exponent of TCX gain.
 * \param elFlags element specific parser guidance flags.
 * \param lg_fb the fullband frame length in audio samples.
 * \param IGF_bgn the IGF start index.
 */
static void calcTCXGain(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                        FIXP_DBL *r, FIXP_DBL rms_m, INT rms_e, const INT frame,
                        const INT lg) {
  if ((rms_m != (FIXP_DBL)0)) {
    FIXP_DBL tcx_gain_m;
    INT tcx_gain_e;

    CLpd_DecodeGain(&tcx_gain_m, &tcx_gain_e,
                    pAacDecoderChannelInfo->pDynData->specificTo.usac
                        .tcx_global_gain[frame]);

    /* rms * 2^rms_e = lg/sqrt(sum(spec^2)) */
    if (rms_e & 1) {
      rms_m >>= 1;
      rms_e++;
    }

    {
      FIXP_DBL fx_lg;
      INT fx_lg_e, s;
      INT inv_e;

      /* lg = fx_lg * 2^fx_lg_e */
      s = fNorm((FIXP_DBL)lg);
      fx_lg = (FIXP_DBL)lg << s;
      fx_lg_e = DFRACT_BITS - 1 - s;
      /* 1/sqrt(rms) */
      rms_m = invSqrtNorm2(rms_m, &inv_e);
      rms_m = fMult(rms_m, fx_lg);
      rms_e = inv_e - (rms_e >> 1) + fx_lg_e;
    }

    {
      int s = fNorm(tcx_gain_m);
      tcx_gain_m = tcx_gain_m << s;
      tcx_gain_e -= s;
    }

    tcx_gain_m = fMultDiv2(tcx_gain_m, rms_m);
    tcx_gain_e = tcx_gain_e + rms_e;

    /* global_gain * 2^(global_gain_e+rms_e) = (10^(global_gain/28)) * rms *
     * 2^rms_e */
    {
      { tcx_gain_e += 1; }
    }

    pAacDecoderChannelInfo->data.usac.tcx_gain[frame] = tcx_gain_m;
    pAacDecoderChannelInfo->data.usac.tcx_gain_e[frame] = tcx_gain_e;

    pAacDecoderChannelInfo->specScale[frame] += tcx_gain_e;
  }
}

/**
 * \brief FDNS decoding.
 * \param pAacDecoderChannelInfo channel context data.
 * \param pAacDecoderStaticChannelInfo channel context static data.
 * \param r output spectrum.
 * \param lg the frame length in audio samples.
 * \param frame the frame index of the LPD super frame.
 * \param pScale pointer to current scale shift factor of r[].
 * \param A1 old input LPC coefficients of length M_LP_FILTER_ORDER.
 * \param A2 new input LPC coefficients of length M_LP_FILTER_ORDER.
 * \param pAlfdGains pointer for ALFD gains output scaled by 1.
 * \param fdns_npts number of lines (FDNS_NPTS).
 * \param inf_mask pointer to noise mask.
 * \param IGF_win_mode IGF window mode (LONG, SHORT, TCX10, TCX20).
 * \param frameType (IGF_FRAME_DIVISION_AAC_OR_TCX_LONG or
 * IGF_FRAME_DIVISION_TCX_SHORT_1).
 * \param elFlags element specific parser guidance flags.
 * \param lg_fb the fullband frame length in audio samples.
 * \param IGF_bgn the IGF start index.
 * \param rms_m mantisse of energy.
 * \param rms_e exponent of energy.
 */
/* static */
void CLpd_FdnsDecode(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                     CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
                     FIXP_DBL r[], const INT lg, const INT frame, SHORT *pScale,
                     const FIXP_LPC A1[M_LP_FILTER_ORDER], const INT A1_exp,
                     const FIXP_LPC A2[M_LP_FILTER_ORDER], const INT A2_exp,
                     FIXP_DBL pAlfdGains[LFAC / 4], const INT fdns_npts) {
  /* Weight LPC coefficients using Rm values */
  CLpd_AdaptLowFreqDeemph(r, lg, pAlfdGains, *pScale);

  FIXP_DBL rms_m = (FIXP_DBL)0;
  INT rms_e = 0;
  {
    /* Calculate Energy */
    rms_m = calcEnergy(r, *pScale, lg, &rms_e);
  }

  calcTCXGain(pAacDecoderChannelInfo, r, rms_m, rms_e, frame, lg);

  /* Apply ODFT and Noise Shaping. LP coefficient (A1, A2) weighting is done
   * inside on the fly. */

  lpc2mdctAndNoiseShaping(r, pScale, lg, fdns_npts, A1, A1_exp, A2, A2_exp);
}

/**
 * find pitch for TCX20 (time domain) concealment.
 */
static int find_mpitch(FIXP_DBL xri[], int lg) {
  FIXP_DBL max, pitch;
  INT pitch_e;
  int i, n;

  max = (FIXP_DBL)0;
  n = 2;

  /* find maximum below 400Hz */
  for (i = 2; i < (lg >> 4); i += 2) {
    FIXP_DBL tmp = fPow2Div2(xri[i]) + fPow2Div2(xri[i + 1]);
    if (tmp > max) {
      max = tmp;
      n = i;
    }
  }

  // pitch = ((float)lg<<1)/(float)n;
  pitch = fDivNorm((FIXP_DBL)lg << 1, (FIXP_DBL)n, &pitch_e);
  pitch >>= fixMax(0, DFRACT_BITS - 1 - pitch_e - 16);

  /* find pitch multiple under 20ms */
  if (pitch >= (FIXP_DBL)((256 << 16) - 1)) { /*231.0f*/
    n = 256;
  } else {
    FIXP_DBL mpitch = pitch;
    while (mpitch < (FIXP_DBL)(255 << 16)) {
      mpitch += pitch;
    }
    n = (int)(mpitch - pitch) >> 16;
  }

  return (n);
}

/**
 * number of spectral coefficients / time domain samples using frame mode as
 * index.
 */
static const int lg_table_ccfl[2][4] = {
    {256, 256, 512, 1024}, /* coreCoderFrameLength = 1024 */
    {192, 192, 384, 768}   /* coreCoderFrameLength = 768  */
};

/**
 * \brief Decode and render one MDCT-TCX frame.
 * \param pAacDecoderChannelInfo channel context data.
 * \param lg the frame length in audio samples.
 * \param frame the frame index of the LPD super frame.
 */
static void CLpd_TcxDecode(
    CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo, UINT flags,
    int mod, int last_mod, int frame, int frameOk) {
  FIXP_DBL *pAlfd_gains = pAacDecoderStaticChannelInfo->last_alfd_gains;
  ULONG *pSeed = &pAacDecoderStaticChannelInfo->nfRandomSeed;
  int lg = (pAacDecoderChannelInfo->granuleLength == 128)
               ? lg_table_ccfl[0][mod + 0]
               : lg_table_ccfl[1][mod + 0];
  int next_frame = frame + (1 << (mod - 1));
  int isFullBandLpd = 0;

  /* Obtain r[] vector by combining the quant[] and noise[] vectors */
  {
    FIXP_DBL noise_level;
    FIXP_DBL *coeffs =
        SPEC_TCX(pAacDecoderChannelInfo->pSpectralCoefficient, frame,
                 pAacDecoderChannelInfo->granuleLength, isFullBandLpd);
    int scale = pAacDecoderChannelInfo->specScale[frame];
    int i, nfBgn, nfEnd;
    UCHAR tcx_noise_factor = pAacDecoderChannelInfo->pDynData->specificTo.usac
                                 .tcx_noise_factor[frame];

    /* find pitch for bfi case */
    pAacDecoderStaticChannelInfo->last_tcx_pitch = find_mpitch(coeffs, lg);

    if (frameOk) {
      /* store for concealment */
      pAacDecoderStaticChannelInfo->last_tcx_noise_factor = tcx_noise_factor;
    } else {
      /* restore last frames value */
      tcx_noise_factor = pAacDecoderStaticChannelInfo->last_tcx_noise_factor;
    }

    noise_level =
        (FIXP_DBL)((LONG)FL2FXCONST_DBL(0.0625f) * (8 - tcx_noise_factor));
    noise_level = scaleValue(noise_level, -scale);

    const FIXP_DBL neg_noise_level = -noise_level;

    {
      nfBgn = lg / 6;
      nfEnd = lg;
    }

    for (i = nfBgn; i < nfEnd - 7; i += 8) {
      LONG tmp;

      /* Fill all 8 consecutive zero coeffs with noise */
      tmp = coeffs[i + 0] | coeffs[i + 1] | coeffs[i + 2] | coeffs[i + 3] |
            coeffs[i + 4] | coeffs[i + 5] | coeffs[i + 6] | coeffs[i + 7];

      if (tmp == 0) {
        for (int k = i; k < i + 8; k++) {
          UsacRandomSign(pSeed) ? (coeffs[k] = neg_noise_level)
                                : (coeffs[k] = noise_level);
        }
      }
    }
    if ((nfEnd - i) >
        0) { /* noise filling for last "band" with less than 8 bins */
      LONG tmp = (LONG)coeffs[i];
      int k;

      FDK_ASSERT((nfEnd - i) < 8);
      for (k = 1; k < (nfEnd - i); k++) {
        tmp |= (LONG)coeffs[i + k];
      }
      if (tmp == 0) {
        for (k = i; k < nfEnd; k++) {
          UsacRandomSign(pSeed) ? (coeffs[k] = neg_noise_level)
                                : (coeffs[k] = noise_level);
        }
      }
    }
  }

  {
    /* Convert LPC to LP domain */
    if (last_mod == 0) {
      /* Note: The case where last_mod == 255 is handled by other means
       * in CLpdChannelStream_Read() */
      E_LPC_f_lsp_a_conversion(
          pAacDecoderChannelInfo->data.usac.lsp_coeff[frame],
          pAacDecoderChannelInfo->data.usac.lp_coeff[frame],
          &pAacDecoderChannelInfo->data.usac.lp_coeff_exp[frame]);
    }

    E_LPC_f_lsp_a_conversion(
        pAacDecoderChannelInfo->data.usac.lsp_coeff[next_frame],
        pAacDecoderChannelInfo->data.usac.lp_coeff[next_frame],
        &pAacDecoderChannelInfo->data.usac.lp_coeff_exp[next_frame]);

    /* FDNS decoding */
    CLpd_FdnsDecode(
        pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo,
        SPEC_TCX(pAacDecoderChannelInfo->pSpectralCoefficient, frame,
                 pAacDecoderChannelInfo->granuleLength, isFullBandLpd),
        lg, frame, pAacDecoderChannelInfo->specScale + frame,
        pAacDecoderChannelInfo->data.usac.lp_coeff[frame],
        pAacDecoderChannelInfo->data.usac.lp_coeff_exp[frame],
        pAacDecoderChannelInfo->data.usac.lp_coeff[next_frame],
        pAacDecoderChannelInfo->data.usac.lp_coeff_exp[next_frame], pAlfd_gains,
        pAacDecoderChannelInfo->granuleLength / 2 /* == FDNS_NPTS(ccfl) */
    );
  }
}

/**
 * \brief Read the tcx_coding bitstream part
 * \param hBs bitstream handle to read from.
 * \param pAacDecoderChannelInfo channel context info to store data into.
 * \param lg the frame length in audio samples.
 * \param first_tcx_flag flag indicating that this is the first TCX frame.
 * \param frame the frame index of the LPD super frame.
 */
static AAC_DECODER_ERROR CLpd_TCX_Read(
    HANDLE_FDK_BITSTREAM hBs, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo, int lg,
    int first_tcx_flag, int frame, UINT flags) {
  AAC_DECODER_ERROR errorAAC = AAC_DEC_OK;
  ARITH_CODING_ERROR error = ARITH_CODER_OK;
  FIXP_DBL *pSpec;
  int arith_reset_flag = 0;
  int isFullBandLpd = 0;

  pSpec = SPEC_TCX(pAacDecoderChannelInfo->pSpectralCoefficient, frame,
                   pAacDecoderChannelInfo->granuleLength, isFullBandLpd);

  /* TCX noise level */
  {
    pAacDecoderChannelInfo->pDynData->specificTo.usac.tcx_noise_factor[frame] =
        FDKreadBits(hBs, 3);
  }
  /* TCX global gain */
  pAacDecoderChannelInfo->pDynData->specificTo.usac.tcx_global_gain[frame] =
      FDKreadBits(hBs, 7);

  /* Arithmetic coded residual/spectrum */
  if (first_tcx_flag) {
    if (flags & AC_INDEP) {
      arith_reset_flag = 1;
    } else {
      arith_reset_flag = FDKreadBits(hBs, 1);
    }
  }

  /* CArco_DecodeArithData() output scale of "pSpec" is DFRACT_BITS-1 */
  error = CArco_DecodeArithData(pAacDecoderStaticChannelInfo->hArCo, hBs, pSpec,
                                lg, lg, arith_reset_flag);

  /* Rescale residual/spectrum */
  {
    int scale = getScalefactor(pSpec, lg) - 2; /* Leave 2 bits headroom */

    /* Exponent of CArco_DecodeArithData() output is DFRACT_BITS; integer
     * values. */
    scaleValues(pSpec, lg, scale);
    scale = DFRACT_BITS - 1 - scale;

    pAacDecoderChannelInfo->specScale[frame] = scale;
  }

  if (error == ARITH_CODER_ERROR) errorAAC = AAC_DEC_UNKNOWN;

  return errorAAC;
}

/**
 * \brief translate lpd_mode into the mod[] array which describes the mode of
 * each each LPD frame
 * \param mod[] the array that will be filled with the mode indexes of the
 * inidividual frames.
 * \param lpd_mode the lpd_mode field read from the lpd_channel_stream
 */
static AAC_DECODER_ERROR CLpd_ReadAndMapLpdModeToModArray(
    UCHAR mod[4], HANDLE_FDK_BITSTREAM hBs, UINT elFlags) {
  int lpd_mode;

  {
    lpd_mode = FDKreadBits(hBs, 5);

    if (lpd_mode > 25 || lpd_mode < 0) {
      return AAC_DEC_PARSE_ERROR;
    }

    switch (lpd_mode) {
      case 25:
        /* 1 80MS frame */
        mod[0] = mod[1] = mod[2] = mod[3] = 3;
        break;
      case 24:
        /* 2 40MS frames */
        mod[0] = mod[1] = mod[2] = mod[3] = 2;
        break;
      default:
        switch (lpd_mode >> 2) {
          case 4:
            /* lpd_mode 19 - 16  => 1 40MS and 2 20MS frames */
            mod[0] = mod[1] = 2;
            mod[2] = (lpd_mode & 1) ? 1 : 0;
            mod[3] = (lpd_mode & 2) ? 1 : 0;
            break;
          case 5:
            /* lpd_mode 23 - 20 => 2 20MS and 1 40MS frames */
            mod[2] = mod[3] = 2;
            mod[0] = (lpd_mode & 1) ? 1 : 0;
            mod[1] = (lpd_mode & 2) ? 1 : 0;
            break;
          default:
            /* lpd_mode < 16 => 4 20MS frames */
            mod[0] = (lpd_mode & 1) ? 1 : 0;
            mod[1] = (lpd_mode & 2) ? 1 : 0;
            mod[2] = (lpd_mode & 4) ? 1 : 0;
            mod[3] = (lpd_mode & 8) ? 1 : 0;
            break;
        }
        break;
    }
  }
  return AAC_DEC_OK;
}

static void CLpd_Reset(
    CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
    int keep_past_signal) {
  int i;

  /* Reset TCX / ACELP common memory */
  if (!keep_past_signal) {
    FDKmemclear(pAacDecoderStaticChannelInfo->old_synth,
                sizeof(pAacDecoderStaticChannelInfo->old_synth));
  }

  /* Initialize the LSFs */
  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    pAacDecoderStaticChannelInfo->lpc4_lsf[i] = fdk_dec_lsf_init[i];
  }

  /* Reset memory needed by bass post-filter */
  FDKmemclear(pAacDecoderStaticChannelInfo->mem_bpf,
              sizeof(pAacDecoderStaticChannelInfo->mem_bpf));

  pAacDecoderStaticChannelInfo->old_bpf_control_info = 0;
  for (i = 0; i < SYN_SFD; i++) {
    pAacDecoderStaticChannelInfo->old_T_pf[i] = 64;
    pAacDecoderStaticChannelInfo->old_gain_pf[i] = (FIXP_DBL)0;
  }

  /* Reset ACELP memory */
  CLpd_AcelpReset(&pAacDecoderStaticChannelInfo->acelp);

  pAacDecoderStaticChannelInfo->last_lpc_lost = 0;      /* prev_lpc_lost */
  pAacDecoderStaticChannelInfo->last_tcx_pitch = L_DIV; /* pitch_tcx     */
  pAacDecoderStaticChannelInfo->numLostLpdFrames = 0;   /* nbLostCmpt    */
}

/*
 * Externally visible functions
 */

AAC_DECODER_ERROR CLpdChannelStream_Read(
    HANDLE_FDK_BITSTREAM hBs, CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
    const SamplingRateInfo *pSamplingRateInfo, UINT flags) {
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  int first_tcx_flag;
  int k, nbDiv, fFacDataPresent, first_lpd_flag, acelp_core_mode,
      facGetMemState = 0;
  UCHAR *mod = pAacDecoderChannelInfo->data.usac.mod;
  int lpd_mode_last, prev_frame_was_lpd;
  USAC_COREMODE core_mode_last;
  const int lg_table_offset = 0;
  const int *lg_table = (pAacDecoderChannelInfo->granuleLength == 128)
                            ? &lg_table_ccfl[0][lg_table_offset]
                            : &lg_table_ccfl[1][lg_table_offset];
  int last_lpc_lost = pAacDecoderStaticChannelInfo->last_lpc_lost;

  int last_frame_ok = CConcealment_GetLastFrameOk(
      &pAacDecoderStaticChannelInfo->concealmentInfo, 1);

  INT i_offset;
  UINT samplingRate;

  samplingRate = pSamplingRateInfo->samplingRate;

  i_offset =
      (INT)(samplingRate * PIT_MIN_12k8 + (FSCALE_DENOM / 2)) / FSCALE_DENOM -
      (INT)PIT_MIN_12k8;

  if ((samplingRate < FAC_FSCALE_MIN) || (samplingRate > FAC_FSCALE_MAX)) {
    error = AAC_DEC_PARSE_ERROR;
    goto bail;
  }

  acelp_core_mode = FDKreadBits(hBs, 3);

  /* lpd_mode */
  error = CLpd_ReadAndMapLpdModeToModArray(mod, hBs, 0);
  if (error != AAC_DEC_OK) {
    goto bail;
  }

  /* bpf_control_info */
  pAacDecoderChannelInfo->data.usac.bpf_control_info = FDKreadBit(hBs);

  /* last_core_mode */
  prev_frame_was_lpd = FDKreadBit(hBs);
  /* fac_data_present */
  fFacDataPresent = FDKreadBit(hBs);

  /* Set valid values from
   * pAacDecoderStaticChannelInfo->{last_core_mode,last_lpd_mode} */
  pAacDecoderChannelInfo->data.usac.core_mode_last =
      pAacDecoderStaticChannelInfo->last_core_mode;
  lpd_mode_last = pAacDecoderChannelInfo->data.usac.lpd_mode_last =
      pAacDecoderStaticChannelInfo->last_lpd_mode;

  if (prev_frame_was_lpd == 0) {
    /* Last frame was FD */
    pAacDecoderChannelInfo->data.usac.core_mode_last = FD_LONG;
    pAacDecoderChannelInfo->data.usac.lpd_mode_last = 255;
  } else {
    /* Last frame was LPD */
    pAacDecoderChannelInfo->data.usac.core_mode_last = LPD;
    if (((mod[0] == 0) && fFacDataPresent) ||
        ((mod[0] != 0) && !fFacDataPresent)) {
      /* Currend mod is ACELP, fac data present -> TCX, current mod TCX, no fac
       * data -> TCX */
      if (lpd_mode_last == 0) {
        /* Bit stream interruption detected. Assume last TCX mode as TCX20. */
        pAacDecoderChannelInfo->data.usac.lpd_mode_last = 1;
      }
      /* Else assume that remembered TCX mode is correct. */
    } else {
      pAacDecoderChannelInfo->data.usac.lpd_mode_last = 0;
    }
  }

  first_lpd_flag = (pAacDecoderChannelInfo->data.usac.core_mode_last !=
                    LPD); /* Depends on bitstream configuration */
  first_tcx_flag = 1;

  if (pAacDecoderStaticChannelInfo->last_core_mode !=
      LPD) { /* ATTENTION: Reset depends on what we rendered before! */
    CLpd_Reset(pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo, 0);

    if (!last_frame_ok) {
      /* If last rendered frame was not LPD and first lpd flag is not set, this
       * must be an error - set last_lpc_lost flag */
      last_lpc_lost |= (first_lpd_flag) ? 0 : 1;
    }
  }

  core_mode_last = pAacDecoderChannelInfo->data.usac.core_mode_last;
  lpd_mode_last = pAacDecoderChannelInfo->data.usac.lpd_mode_last;

  nbDiv = NB_DIV;

  /* k is the frame index. If a frame is of size 40MS or 80MS,
     this frame index is incremented 2 or 4 instead of 1 respectively. */

  k = 0;
  while (k < nbDiv) {
    /* Reset FAC data pointers in order to avoid applying old random FAC data.
     */
    pAacDecoderChannelInfo->data.usac.fac_data[k] = NULL;

    if ((k == 0 && core_mode_last == LPD && fFacDataPresent) ||
        (lpd_mode_last == 0 && mod[k] > 0) ||
        ((lpd_mode_last != 255) && lpd_mode_last > 0 && mod[k] == 0)) {
      int err;

      /* Assign FAC memory */
      pAacDecoderChannelInfo->data.usac.fac_data[k] =
          CLpd_FAC_GetMemory(pAacDecoderChannelInfo, mod, &facGetMemState);

      /* FAC for (ACELP -> TCX) or (TCX -> ACELP) */
      err = CLpd_FAC_Read(
          hBs, pAacDecoderChannelInfo->data.usac.fac_data[k],
          pAacDecoderChannelInfo->data.usac.fac_data_e,
          pAacDecoderChannelInfo->granuleLength, /* == fac_length */
          0, k);
      if (err != 0) {
        error = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }

    if (mod[k] == 0) /* acelp-mode */
    {
      int err;
      err = CLpd_AcelpRead(
          hBs, &pAacDecoderChannelInfo->data.usac.acelp[k], acelp_core_mode,
          pAacDecoderChannelInfo->granuleLength * 8 /* coreCoderFrameLength */,
          i_offset);
      if (err != 0) {
        error = AAC_DEC_PARSE_ERROR;
        goto bail;
      }

      lpd_mode_last = 0;
      k++;
    } else /* mode != 0  =>  TCX */
    {
      error = CLpd_TCX_Read(hBs, pAacDecoderChannelInfo,
                            pAacDecoderStaticChannelInfo, lg_table[mod[k]],
                            first_tcx_flag, k, flags);

      lpd_mode_last = mod[k];
      first_tcx_flag = 0;
      k += 1 << (mod[k] - 1);
    }
    if (error != AAC_DEC_OK) {
      error = AAC_DEC_PARSE_ERROR;
      goto bail;
    }
  }

  {
    int err;

    /* Read LPC coefficients */
    err = CLpc_Read(
        hBs, pAacDecoderChannelInfo->data.usac.lsp_coeff,
        pAacDecoderStaticChannelInfo->lpc4_lsf,
        pAacDecoderChannelInfo->data.usac.lsf_adaptive_mean_cand,
        pAacDecoderChannelInfo->data.usac.aStability, mod, first_lpd_flag,
        /* if last lpc4 is available from concealment do not extrapolate lpc0
           from lpc2 */
        (mod[0] & 0x3) ? 0
                       : (last_lpc_lost &&
                          pAacDecoderStaticChannelInfo->last_core_mode != LPD),
        last_frame_ok);
    if (err != 0) {
      error = AAC_DEC_PARSE_ERROR;
      goto bail;
    }
  }

  /* adjust old lsp[] following to a bad frame (to avoid overshoot) (ref:
   * dec_LPD.c) */
  if (last_lpc_lost && !last_frame_ok) {
    int k_next;
    k = 0;
    while (k < nbDiv) {
      int i;
      k_next = k + (((mod[k] & 0x3) == 0) ? 1 : (1 << (mod[k] - 1)));
      FIXP_LPC *lsp_old = pAacDecoderChannelInfo->data.usac.lsp_coeff[k];
      FIXP_LPC *lsp_new = pAacDecoderChannelInfo->data.usac.lsp_coeff[k_next];

      for (i = 0; i < M_LP_FILTER_ORDER; i++) {
        if (lsp_new[i] < lsp_old[i]) {
          lsp_old[i] = lsp_new[i];
        }
      }
      k = k_next;
    }
  }

  if (!CConcealment_GetLastFrameOk(
          &pAacDecoderStaticChannelInfo->concealmentInfo, 1)) {
    E_LPC_f_lsp_a_conversion(
        pAacDecoderChannelInfo->data.usac.lsp_coeff[0],
        pAacDecoderChannelInfo->data.usac.lp_coeff[0],
        &pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0]);
  } else if (pAacDecoderStaticChannelInfo->last_lpd_mode != 0) {
    if (pAacDecoderStaticChannelInfo->last_lpd_mode == 255) {
      /* We need it for TCX decoding or ACELP excitation update */
      E_LPC_f_lsp_a_conversion(
          pAacDecoderChannelInfo->data.usac.lsp_coeff[0],
          pAacDecoderChannelInfo->data.usac.lp_coeff[0],
          &pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0]);
    } else { /* last_lpd_mode was TCX */
      /* Copy old LPC4 LP domain coefficients to LPC0 LP domain buffer (to avoid
       * converting LSP coefficients again). */
      FDKmemcpy(pAacDecoderChannelInfo->data.usac.lp_coeff[0],
                pAacDecoderStaticChannelInfo->lp_coeff_old[0],
                M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
      pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0] =
          pAacDecoderStaticChannelInfo->lp_coeff_old_exp[0];
    }
  } /* case last_lpd_mode was ACELP is handled by CLpd_TcxDecode() */

  if (fFacDataPresent && (core_mode_last != LPD)) {
    int prev_frame_was_short;

    prev_frame_was_short = FDKreadBit(hBs);

    if (prev_frame_was_short) {
      core_mode_last = pAacDecoderChannelInfo->data.usac.core_mode_last =
          FD_SHORT;
      pAacDecoderChannelInfo->data.usac.lpd_mode_last = 255;

      if ((pAacDecoderStaticChannelInfo->last_core_mode != FD_SHORT) &&
          CConcealment_GetLastFrameOk(
              &pAacDecoderStaticChannelInfo->concealmentInfo, 1)) {
        /* USAC Conformance document:
           short_fac_flag   shall be encoded with a value of 1 if the
           window_sequence of the previous frame was 2 (EIGHT_SHORT_SEQUENCE).
                            Otherwise short_fac_flag shall be encoded with a
           value of 0. */
        error = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }

    /* Assign memory */
    pAacDecoderChannelInfo->data.usac.fac_data[0] =
        CLpd_FAC_GetMemory(pAacDecoderChannelInfo, mod, &facGetMemState);

    {
      int err;

      /* FAC for FD -> ACELP */
      err = CLpd_FAC_Read(
          hBs, pAacDecoderChannelInfo->data.usac.fac_data[0],
          pAacDecoderChannelInfo->data.usac.fac_data_e,
          CLpd_FAC_getLength(core_mode_last != FD_SHORT,
                             pAacDecoderChannelInfo->granuleLength),
          1, 0);
      if (err != 0) {
        error = AAC_DEC_PARSE_ERROR;
        goto bail;
      }
    }
  }

bail:
  if (error == AAC_DEC_OK) {
    /* check consitency of last core/lpd mode values */
    if ((pAacDecoderChannelInfo->data.usac.core_mode_last !=
         pAacDecoderStaticChannelInfo->last_core_mode) &&
        (pAacDecoderStaticChannelInfo->last_lpc_lost == 0)) {
      /* Something got wrong! */
      /* error = AAC_DEC_PARSE_ERROR; */ /* Throwing errors does not help */
    } else if ((pAacDecoderChannelInfo->data.usac.core_mode_last == LPD) &&
               (pAacDecoderChannelInfo->data.usac.lpd_mode_last !=
                pAacDecoderStaticChannelInfo->last_lpd_mode) &&
               (pAacDecoderStaticChannelInfo->last_lpc_lost == 0)) {
      /* Something got wrong! */
      /* error = AAC_DEC_PARSE_ERROR; */ /* Throwing errors does not help */
    }
  }

  return error;
}

void CLpdChannelStream_Decode(
    CAacDecoderChannelInfo *pAacDecoderChannelInfo,
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo, UINT flags) {
  UCHAR *mod = pAacDecoderChannelInfo->data.usac.mod;
  int k;
  UCHAR last_lpd_mode;
  int nbDiv = NB_DIV;

  /* k is the frame index. If a frame is of size 40MS or 80MS,
     this frame index is incremented 2 or 4 instead of 1 respectively. */
  k = 0;
  last_lpd_mode =
      pAacDecoderChannelInfo->data.usac
          .lpd_mode_last; /* could be different to what has been rendered */
  while (k < nbDiv) {
    if (mod[k] == 0) {
      /* ACELP */

      /* If FAC (fac_data[k] != NULL), and previous frame was TCX, apply (TCX)
       * gains to FAC data */
      if (last_lpd_mode > 0 && last_lpd_mode != 255 &&
          pAacDecoderChannelInfo->data.usac.fac_data[k]) {
        CFac_ApplyGains(pAacDecoderChannelInfo->data.usac.fac_data[k],
                        pAacDecoderChannelInfo->granuleLength,
                        pAacDecoderStaticChannelInfo->last_tcx_gain,
                        pAacDecoderStaticChannelInfo->last_alfd_gains,
                        (last_lpd_mode < 4) ? last_lpd_mode : 3);

        pAacDecoderChannelInfo->data.usac.fac_data_e[k] +=
            pAacDecoderStaticChannelInfo->last_tcx_gain_e;
      }
    } else {
      /* TCX */
      CLpd_TcxDecode(pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo,
                     flags, mod[k], last_lpd_mode, k, 1 /* frameOk == 1 */
      );

      /* Store TCX gain scale for next possible FAC transition. */
      pAacDecoderStaticChannelInfo->last_tcx_gain =
          pAacDecoderChannelInfo->data.usac.tcx_gain[k];
      pAacDecoderStaticChannelInfo->last_tcx_gain_e =
          pAacDecoderChannelInfo->data.usac.tcx_gain_e[k];

      /* If FAC (fac_data[k] != NULL), apply gains */
      if (last_lpd_mode == 0 && pAacDecoderChannelInfo->data.usac.fac_data[k]) {
        CFac_ApplyGains(
            pAacDecoderChannelInfo->data.usac.fac_data[k],
            pAacDecoderChannelInfo->granuleLength /* == fac_length */,
            pAacDecoderChannelInfo->data.usac.tcx_gain[k],
            pAacDecoderStaticChannelInfo->last_alfd_gains, mod[k]);

        pAacDecoderChannelInfo->data.usac.fac_data_e[k] +=
            pAacDecoderChannelInfo->data.usac.tcx_gain_e[k];
      }
    }

    /* remember previous mode */
    last_lpd_mode = mod[k];

    /* Increase k to next frame */
    k += (mod[k] == 0) ? 1 : (1 << (mod[k] - 1));
  }
}

AAC_DECODER_ERROR CLpd_RenderTimeSignal(
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
    CAacDecoderChannelInfo *pAacDecoderChannelInfo, PCM_DEC *pTimeData,
    INT lFrame, SamplingRateInfo *pSamplingRateInfo, UINT frameOk,
    const INT aacOutDataHeadroom, UINT flags, UINT strmFlags) {
  UCHAR *mod = pAacDecoderChannelInfo->data.usac.mod;
  AAC_DECODER_ERROR error = AAC_DEC_OK;
  int k, i_offset;
  int last_k;
  int nrSamples = 0;
  int facFB = 1;
  int nbDiv = NB_DIV;
  int lDiv = lFrame / nbDiv; /* length of division (acelp or tcx20 frame)*/
  int lFac = lDiv / 2;
  int nbSubfr =
      lFrame / (nbDiv * L_SUBFR); /* number of subframes per division */
  int nbSubfrSuperfr = nbDiv * nbSubfr;
  int synSfd = (nbSubfrSuperfr / 2) - BPF_SFD;
  int SynDelay = synSfd * L_SUBFR;
  int aacDelay = lFrame / 2;

  /*
   In respect to the reference software, the synth pointer here is lagging by
   aacDelay ( == SYN_DELAY + BPF_DELAY ) samples. The corresponding old
   synthesis samples are handled by the IMDCT overlap.
   */

  FIXP_DBL *synth_buf =
      pAacDecoderChannelInfo->pComStaticData->pWorkBufferCore1->synth_buf;
  FIXP_DBL *synth = synth_buf + PIT_MAX_MAX - BPF_DELAY;
  UCHAR last_lpd_mode, last_last_lpd_mode, last_lpc_lost, last_frame_lost;

  INT pitch[NB_SUBFR_SUPERFR + SYN_SFD];
  FIXP_DBL pit_gain[NB_SUBFR_SUPERFR + SYN_SFD];

  const int *lg_table;
  int lg_table_offset = 0;

  UINT samplingRate = pSamplingRateInfo->samplingRate;

  FDKmemclear(pitch, (NB_SUBFR_SUPERFR + SYN_SFD) * sizeof(INT));

  if (flags & AACDEC_FLUSH) {
    CLpd_Reset(pAacDecoderChannelInfo, pAacDecoderStaticChannelInfo,
               flags & AACDEC_FLUSH);
    frameOk = 0;
  }

  switch (lFrame) {
    case 1024:
      lg_table = &lg_table_ccfl[0][lg_table_offset];
      break;
    case 768:
      lg_table = &lg_table_ccfl[1][lg_table_offset];
      break;
    default:
      FDK_ASSERT(0);
      return AAC_DEC_UNKNOWN;
  }

  last_frame_lost = !CConcealment_GetLastFrameOk(
      &pAacDecoderStaticChannelInfo->concealmentInfo, 0);

  /* Maintain LPD mode from previous frame */
  if ((pAacDecoderStaticChannelInfo->last_core_mode == FD_LONG) ||
      (pAacDecoderStaticChannelInfo->last_core_mode == FD_SHORT)) {
    pAacDecoderStaticChannelInfo->last_lpd_mode = 255;
  }

  if (!frameOk) {
    FIXP_DBL old_tcx_gain;
    FIXP_SGL old_stab;
    SCHAR old_tcx_gain_e;
    int nLostSf;

    last_lpd_mode = pAacDecoderStaticChannelInfo->last_lpd_mode;
    old_tcx_gain = pAacDecoderStaticChannelInfo->last_tcx_gain;
    old_tcx_gain_e = pAacDecoderStaticChannelInfo->last_tcx_gain_e;
    old_stab = pAacDecoderStaticChannelInfo->oldStability;
    nLostSf = pAacDecoderStaticChannelInfo->numLostLpdFrames;

    /* patch the last LPD mode */
    pAacDecoderChannelInfo->data.usac.lpd_mode_last = last_lpd_mode;

    /* Do mode extrapolation and repeat the previous mode:
       if previous mode = ACELP        -> ACELP
       if previous mode = TCX-20/40    -> TCX-20
       if previous mode = TCX-80       -> TCX-80
       notes:
       - ACELP is not allowed after TCX (no pitch information to reuse)
       - TCX-40 is not allowed in the mode repetition to keep the logic simple
     */
    switch (last_lpd_mode) {
      case 0:
        mod[0] = mod[1] = mod[2] = mod[3] = 0; /* -> ACELP concealment */
        break;
      case 3:
        mod[0] = mod[1] = mod[2] = mod[3] = 3; /* -> TCX FD concealment */
        break;
      case 2:
        mod[0] = mod[1] = mod[2] = mod[3] = 2; /* -> TCX FD concealment */
        break;
      case 1:
      default:
        mod[0] = mod[1] = mod[2] = mod[3] = 4; /* -> TCX TD concealment */
        break;
    }

    /* LPC extrapolation */
    CLpc_Conceal(pAacDecoderChannelInfo->data.usac.lsp_coeff,
                 pAacDecoderStaticChannelInfo->lpc4_lsf,
                 pAacDecoderStaticChannelInfo->lsf_adaptive_mean,
                 /*(pAacDecoderStaticChannelInfo->numLostLpdFrames == 0) ||*/
                 (last_lpd_mode == 255));

    if ((last_lpd_mode > 0) && (last_lpd_mode < 255)) {
      /* Copy old LPC4 LP domain coefficients to LPC0 LP domain buffer (to avoid
       * converting LSP coefficients again). */
      FDKmemcpy(pAacDecoderChannelInfo->data.usac.lp_coeff[0],
                pAacDecoderStaticChannelInfo->lp_coeff_old[0],
                M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
      pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0] =
          pAacDecoderStaticChannelInfo->lp_coeff_old_exp[0];
    } /* case last_lpd_mode was ACELP is handled by CLpd_TcxDecode() */
    /* case last_lpd_mode was Time domain TCX concealment is handled after this
     * "if (!frameOk)"-block */

    /* k is the frame index. If a frame is of size 40MS or 80MS,
       this frame index is incremented 2 or 4 instead of 1 respectively. */
    k = 0;
    while (k < nbDiv) {
      pAacDecoderChannelInfo->data.usac.tcx_gain[k] = old_tcx_gain;
      pAacDecoderChannelInfo->data.usac.tcx_gain_e[k] = old_tcx_gain_e;

      /* restore stability value from last frame */
      pAacDecoderChannelInfo->data.usac.aStability[k] = old_stab;

      /* Increase k to next frame */
      k += ((mod[k] & 0x3) == 0) ? 1 : (1 << ((mod[k] & 0x3) - 1));

      nLostSf++;
    }
  } else {
    if ((pAacDecoderStaticChannelInfo->last_lpd_mode == 4) && (mod[0] > 0)) {
      /* Copy old LPC4 LP domain coefficients to LPC0 LP domain buffer (to avoid
       * converting LSP coefficients again). */
      FDKmemcpy(pAacDecoderChannelInfo->data.usac.lp_coeff[0],
                pAacDecoderStaticChannelInfo->lp_coeff_old[0],
                M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
      pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0] =
          pAacDecoderStaticChannelInfo->lp_coeff_old_exp[0];
    }
  }

  Acelp_PreProcessing(synth_buf, pAacDecoderStaticChannelInfo->old_synth, pitch,
                      pAacDecoderStaticChannelInfo->old_T_pf, pit_gain,
                      pAacDecoderStaticChannelInfo->old_gain_pf, samplingRate,
                      &i_offset, lFrame, synSfd, nbSubfrSuperfr);

  /* k is the frame index. If a frame is of size 40MS or 80MS,
     this frame index is incremented 2 or 4 instead of 1 respectively. */
  k = 0;
  last_k = -1; /* mark invalid */
  last_lpd_mode = pAacDecoderStaticChannelInfo->last_lpd_mode;
  last_last_lpd_mode = pAacDecoderStaticChannelInfo->last_last_lpd_mode;
  last_lpc_lost = pAacDecoderStaticChannelInfo->last_lpc_lost | last_frame_lost;

  /* This buffer must be avalable for the case of FD->ACELP transition. The
  beginning of the buffer is used after the BPF to overwrite the output signal.
  Only the FAC area must be affected by the BPF */

  while (k < nbDiv) {
    if (frameOk == 0) {
      pAacDecoderStaticChannelInfo->numLostLpdFrames++;
    } else {
      last_frame_lost |=
          (pAacDecoderStaticChannelInfo->numLostLpdFrames > 0) ? 1 : 0;
      pAacDecoderStaticChannelInfo->numLostLpdFrames = 0;
    }
    if (mod[k] == 0 || mod[k] == 4) {
      /* ACELP or TCX time domain concealment */
      FIXP_DBL *acelp_out;

      /* FAC management */
      if ((last_lpd_mode != 0) && (last_lpd_mode != 4)) /* TCX TD concealment */
      {
        FIXP_DBL *pFacData = NULL;

        if (frameOk && !last_frame_lost) {
          pFacData = pAacDecoderChannelInfo->data.usac.fac_data[k];
        }

        nrSamples += CLpd_FAC_Mdct2Acelp(
            &pAacDecoderStaticChannelInfo->IMdct, synth + nrSamples, pFacData,
            pAacDecoderChannelInfo->data.usac.fac_data_e[k],
            pAacDecoderChannelInfo->data.usac.lp_coeff[k],
            pAacDecoderChannelInfo->data.usac.lp_coeff_exp[k],
            lFrame - nrSamples,
            CLpd_FAC_getLength(
                (pAacDecoderStaticChannelInfo->last_core_mode != FD_SHORT) ||
                    (k > 0),
                lFac),
            (pAacDecoderStaticChannelInfo->last_core_mode != LPD) && (k == 0),
            0);

        FDKmemcpy(
            synth + nrSamples, pAacDecoderStaticChannelInfo->IMdct.overlap.time,
            pAacDecoderStaticChannelInfo->IMdct.ov_offset * sizeof(FIXP_DBL));
        {
          FIXP_LPC *lp_prev =
              pAacDecoderChannelInfo->data.usac
                  .lp_coeff[0]; /* init value does not real matter */
          INT lp_prev_exp = pAacDecoderChannelInfo->data.usac.lp_coeff_exp[0];

          if (last_lpd_mode != 255) { /* last mode was tcx */
            last_k = k - (1 << (last_lpd_mode - 1));
            if (last_k < 0) {
              lp_prev = pAacDecoderStaticChannelInfo->lp_coeff_old[1];
              lp_prev_exp = pAacDecoderStaticChannelInfo->lp_coeff_old_exp[1];
            } else {
              lp_prev = pAacDecoderChannelInfo->data.usac.lp_coeff[last_k];
              lp_prev_exp =
                  pAacDecoderChannelInfo->data.usac.lp_coeff_exp[last_k];
            }
          }

          CLpd_AcelpPrepareInternalMem(
              synth + aacDelay + k * lDiv, last_lpd_mode,
              (last_last_lpd_mode == 4) ? 0 : last_last_lpd_mode,
              pAacDecoderChannelInfo->data.usac.lp_coeff[k],
              pAacDecoderChannelInfo->data.usac.lp_coeff_exp[k], lp_prev,
              lp_prev_exp, &pAacDecoderStaticChannelInfo->acelp, lFrame,
              (last_frame_lost && k < 2), mod[k]);
        }
      } else {
        if (k == 0 && pAacDecoderStaticChannelInfo->IMdct.ov_offset !=
                          lFrame / facFB / 2) {
          pAacDecoderStaticChannelInfo->IMdct.ov_offset = lFrame / facFB / 2;
        }
        nrSamples += imdct_drain(&pAacDecoderStaticChannelInfo->IMdct,
                                 synth + nrSamples, lFrame / facFB - nrSamples);
      }

      if (nrSamples >= lFrame / facFB) {
        /* Write ACELP time domain samples into IMDCT overlap buffer at
         * pAacDecoderStaticChannelInfo->IMdct.overlap.time +
         * pAacDecoderStaticChannelInfo->IMdct.ov_offset
         */
        acelp_out = pAacDecoderStaticChannelInfo->IMdct.overlap.time +
                    pAacDecoderStaticChannelInfo->IMdct.ov_offset;

        /* Account ACELP time domain output samples to overlap buffer */
        pAacDecoderStaticChannelInfo->IMdct.ov_offset += lDiv;
      } else {
        /* Write ACELP time domain samples into output buffer at pTimeData +
         * nrSamples */
        acelp_out = synth + nrSamples;

        /* Account ACELP time domain output samples to output buffer */
        nrSamples += lDiv;
      }

      if (mod[k] == 4) {
        pAacDecoderStaticChannelInfo->acelp.wsyn_rms = scaleValue(
            pAacDecoderChannelInfo->data.usac.tcx_gain[k],
            fixMin(0,
                   pAacDecoderChannelInfo->data.usac.tcx_gain_e[k] - SF_EXC));
        CLpd_TcxTDConceal(&pAacDecoderStaticChannelInfo->acelp,
                          &pAacDecoderStaticChannelInfo->last_tcx_pitch,
                          pAacDecoderChannelInfo->data.usac.lsp_coeff[k],
                          pAacDecoderChannelInfo->data.usac.lsp_coeff[k + 1],
                          pAacDecoderChannelInfo->data.usac.aStability[k],
                          pAacDecoderStaticChannelInfo->numLostLpdFrames,
                          acelp_out, lFrame,
                          pAacDecoderStaticChannelInfo->last_tcx_noise_factor);

      } else {
        FDK_ASSERT(pAacDecoderChannelInfo->data.usac.aStability[k] >=
                   (FIXP_SGL)0);
        CLpd_AcelpDecode(&pAacDecoderStaticChannelInfo->acelp, i_offset,
                         pAacDecoderChannelInfo->data.usac.lsp_coeff[k],
                         pAacDecoderChannelInfo->data.usac.lsp_coeff[k + 1],
                         pAacDecoderChannelInfo->data.usac.aStability[k],
                         &pAacDecoderChannelInfo->data.usac.acelp[k],
                         pAacDecoderStaticChannelInfo->numLostLpdFrames,
                         last_lpc_lost, k, acelp_out,
                         &pitch[(k * nbSubfr) + synSfd],
                         &pit_gain[(k * nbSubfr) + synSfd], lFrame);
      }

      if (mod[k] != 4) {
        if (last_lpd_mode != 0 &&
            pAacDecoderChannelInfo->data.usac
                .bpf_control_info) { /* FD/TCX -> ACELP transition */
          /* bass post-filter past FAC area (past two (one for FD short)
           * subframes) */
          int currentSf = synSfd + k * nbSubfr;

          if ((k > 0) || (pAacDecoderStaticChannelInfo->last_core_mode !=
                          FD_SHORT)) { /* TCX or FD long -> ACELP */
            pitch[currentSf - 2] = pitch[currentSf - 1] = pitch[currentSf];
            pit_gain[currentSf - 2] = pit_gain[currentSf - 1] =
                pit_gain[currentSf];
          } else { /* FD short -> ACELP */
            pitch[currentSf - 1] = pitch[currentSf];
            pit_gain[currentSf - 1] = pit_gain[currentSf];
          }
        }
      }
    } else { /* TCX */
      int lg = lg_table[mod[k]];
      int isFullBandLpd = 0;

      /* FAC management */
      if ((last_lpd_mode == 0) || (last_lpd_mode == 4)) /* TCX TD concealment */
      {
        C_AALLOC_SCRATCH_START(fac_buf, FIXP_DBL, 1024 / 8);

        /* pAacDecoderChannelInfo->data.usac.fac_data[k] == NULL means no FAC
         * data available. */
        if (last_frame_lost == 1 ||
            pAacDecoderChannelInfo->data.usac.fac_data[k] == NULL) {
          FDKmemclear(fac_buf, 1024 / 8 * sizeof(FIXP_DBL));
          pAacDecoderChannelInfo->data.usac.fac_data[k] = fac_buf;
          pAacDecoderChannelInfo->data.usac.fac_data_e[k] = 0;
        }

        nrSamples += CLpd_FAC_Acelp2Mdct(
            &pAacDecoderStaticChannelInfo->IMdct, synth + nrSamples,
            SPEC_TCX(pAacDecoderChannelInfo->pSpectralCoefficient, k,
                     pAacDecoderChannelInfo->granuleLength, isFullBandLpd),
            pAacDecoderChannelInfo->specScale + k, 1,
            pAacDecoderChannelInfo->data.usac.fac_data[k],
            pAacDecoderChannelInfo->data.usac.fac_data_e[k],
            pAacDecoderChannelInfo->granuleLength /* == fac_length */,
            lFrame - nrSamples, lg,
            FDKgetWindowSlope(lDiv,
                              GetWindowShape(&pAacDecoderChannelInfo->icsInfo)),
            lDiv, pAacDecoderChannelInfo->data.usac.lp_coeff[k],
            pAacDecoderChannelInfo->data.usac.lp_coeff_exp[k],
            &pAacDecoderStaticChannelInfo->acelp,
            pAacDecoderChannelInfo->data.usac.tcx_gain[k],
            (last_frame_lost || !frameOk), 0 /* is not FD FAC */
            ,
            last_lpd_mode, k,
            pAacDecoderChannelInfo
                ->currAliasingSymmetry /* Note: The current aliasing
                                          symmetry for a TCX (i.e. LPD)
                                          frame must always be 0 */
        );

        pitch[(k * nbSubfr) + synSfd + 1] = pitch[(k * nbSubfr) + synSfd] =
            pitch[(k * nbSubfr) + synSfd - 1];
        pit_gain[(k * nbSubfr) + synSfd + 1] =
            pit_gain[(k * nbSubfr) + synSfd] =
                pit_gain[(k * nbSubfr) + synSfd - 1];

        C_AALLOC_SCRATCH_END(fac_buf, FIXP_DBL, 1024 / 8);
      } else {
        int tl = lg;
        int fl = lDiv;
        int fr = lDiv;

        nrSamples += imlt_block(
            &pAacDecoderStaticChannelInfo->IMdct, synth + nrSamples,
            SPEC_TCX(pAacDecoderChannelInfo->pSpectralCoefficient, k,
                     pAacDecoderChannelInfo->granuleLength, isFullBandLpd),
            pAacDecoderChannelInfo->specScale + k, 1, lFrame - nrSamples, tl,
            FDKgetWindowSlope(fl,
                              GetWindowShape(&pAacDecoderChannelInfo->icsInfo)),
            fl,
            FDKgetWindowSlope(fr,
                              GetWindowShape(&pAacDecoderChannelInfo->icsInfo)),
            fr, pAacDecoderChannelInfo->data.usac.tcx_gain[k],
            pAacDecoderChannelInfo->currAliasingSymmetry
                ? MLT_FLAG_CURR_ALIAS_SYMMETRY
                : 0);
      }
    }
    /* remember previous mode */
    last_last_lpd_mode = last_lpd_mode;
    last_lpd_mode = mod[k];
    last_lpc_lost = (frameOk == 0) ? 1 : 0;

    /* Increase k to next frame */
    last_k = k;
    k += ((mod[k] & 0x3) == 0) ? 1 : (1 << (mod[k] - 1));
  }

  if (frameOk) {
    /* assume data was ok => store for concealment */
    FDK_ASSERT(pAacDecoderChannelInfo->data.usac.aStability[last_k] >=
               (FIXP_SGL)0);
    pAacDecoderStaticChannelInfo->oldStability =
        pAacDecoderChannelInfo->data.usac.aStability[last_k];
    FDKmemcpy(pAacDecoderStaticChannelInfo->lsf_adaptive_mean,
              pAacDecoderChannelInfo->data.usac.lsf_adaptive_mean_cand,
              M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
  }

  /* store past lp coeffs for next superframe (they are only valid and needed if
   * last_lpd_mode was tcx) */
  if (last_lpd_mode > 0) {
    FDKmemcpy(pAacDecoderStaticChannelInfo->lp_coeff_old[0],
              pAacDecoderChannelInfo->data.usac.lp_coeff[nbDiv],
              M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
    pAacDecoderStaticChannelInfo->lp_coeff_old_exp[0] =
        pAacDecoderChannelInfo->data.usac.lp_coeff_exp[nbDiv];
    FDKmemcpy(pAacDecoderStaticChannelInfo->lp_coeff_old[1],
              pAacDecoderChannelInfo->data.usac.lp_coeff[last_k],
              M_LP_FILTER_ORDER * sizeof(FIXP_LPC));
    pAacDecoderStaticChannelInfo->lp_coeff_old_exp[1] =
        pAacDecoderChannelInfo->data.usac.lp_coeff_exp[last_k];
  }

  FDK_ASSERT(nrSamples == lFrame);

  /* check whether usage of bass postfilter was de-activated in the bitstream;
   if yes, set pitch gain to 0 */
  if (!(pAacDecoderChannelInfo->data.usac.bpf_control_info)) {
    if (mod[0] != 0 && (pAacDecoderStaticChannelInfo->old_bpf_control_info)) {
      for (int i = 2; i < nbSubfrSuperfr; i++)
        pit_gain[synSfd + i] = (FIXP_DBL)0;
    } else {
      for (int i = 0; i < nbSubfrSuperfr; i++)
        pit_gain[synSfd + i] = (FIXP_DBL)0;
    }
  }

  /* for bass postfilter */
  for (int n = 0; n < synSfd; n++) {
    pAacDecoderStaticChannelInfo->old_T_pf[n] = pitch[nbSubfrSuperfr + n];
    pAacDecoderStaticChannelInfo->old_gain_pf[n] = pit_gain[nbSubfrSuperfr + n];
  }

  pAacDecoderStaticChannelInfo->old_bpf_control_info =
      pAacDecoderChannelInfo->data.usac.bpf_control_info;

  {
    INT lookahead = -BPF_DELAY;
    int copySamp = (mod[nbDiv - 1] == 0) ? (aacDelay) : (aacDelay - lFac);

    /* Copy enough time domain samples from MDCT to synthesis buffer as needed
     * by the bass postfilter */

    lookahead += imdct_copy_ov_and_nr(&pAacDecoderStaticChannelInfo->IMdct,
                                      synth + nrSamples, copySamp);

    FDK_ASSERT(lookahead == copySamp - BPF_DELAY);

    FIXP_DBL *p2_synth = synth + BPF_DELAY;

    /* recalculate pitch gain to allow postfilering on FAC area */
    for (int i = 0; i < nbSubfrSuperfr; i++) {
      int T = pitch[i];
      FIXP_DBL gain = pit_gain[i];

      if (gain > (FIXP_DBL)0) {
        gain = get_gain(&p2_synth[i * L_SUBFR], &p2_synth[(i * L_SUBFR) - T],
                        L_SUBFR);
        pit_gain[i] = gain;
      }
    }

    {
      bass_pf_1sf_delay(p2_synth, pitch, pit_gain, lFrame, lFrame / facFB,
                        mod[nbDiv - 1] ? (SynDelay - (lDiv / 2)) : SynDelay,
                        pTimeData, aacOutDataHeadroom,
                        pAacDecoderStaticChannelInfo->mem_bpf);
    }
  }

  Acelp_PostProcessing(synth_buf, pAacDecoderStaticChannelInfo->old_synth,
                       pitch, pAacDecoderStaticChannelInfo->old_T_pf, lFrame,
                       synSfd, nbSubfrSuperfr);

  /* Store last mode for next super frame */
  { pAacDecoderStaticChannelInfo->last_core_mode = LPD; }
  pAacDecoderStaticChannelInfo->last_lpd_mode = last_lpd_mode;
  pAacDecoderStaticChannelInfo->last_last_lpd_mode = last_last_lpd_mode;
  pAacDecoderStaticChannelInfo->last_lpc_lost = last_lpc_lost;

  return error;
}
