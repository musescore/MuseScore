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
  \brief  frequency scale $Revision: 95225 $
*/

#include "sbrenc_freq_sca.h"
#include "sbr_misc.h"

#include "genericStds.h"

/*  StartFreq */
static INT getStartFreq(INT fsCore, const INT start_freq);

/* StopFreq */
static INT getStopFreq(INT fsCore, const INT stop_freq);

static INT numberOfBands(INT b_p_o, INT start, INT stop, FIXP_DBL warp_factor);
static void CalcBands(INT *diff, INT start, INT stop, INT num_bands);
static INT modifyBands(INT max_band, INT *diff, INT length);
static void cumSum(INT start_value, INT *diff, INT length, UCHAR *start_adress);

/*******************************************************************************
 Functionname:  FDKsbrEnc_getSbrStartFreqRAW
 *******************************************************************************
 Description:

 Arguments:

 Return:
 *******************************************************************************/

INT FDKsbrEnc_getSbrStartFreqRAW(INT startFreq, INT fsCore) {
  INT result;

  if (startFreq < 0 || startFreq > 15) {
    return -1;
  }
  /* Update startFreq struct */
  result = getStartFreq(fsCore, startFreq);

  result =
      (result * (fsCore >> 5) + 1) >> 1; /* (result*fsSBR/QMFbands+1)>>1; */

  return (result);

} /* End FDKsbrEnc_getSbrStartFreqRAW */

/*******************************************************************************
 Functionname:  getSbrStopFreq
 *******************************************************************************
 Description:

 Arguments:

 Return:
 *******************************************************************************/
INT FDKsbrEnc_getSbrStopFreqRAW(INT stopFreq, INT fsCore) {
  INT result;

  if (stopFreq < 0 || stopFreq > 13) return -1;

  /* Uppdate stopFreq struct */
  result = getStopFreq(fsCore, stopFreq);
  result =
      (result * (fsCore >> 5) + 1) >> 1; /* (result*fsSBR/QMFbands+1)>>1; */

  return (result);
} /* End getSbrStopFreq */

/*******************************************************************************
 Functionname:  getStartFreq
 *******************************************************************************
 Description:

 Arguments:  fsCore - core sampling rate


 Return:
 *******************************************************************************/
static INT getStartFreq(INT fsCore, const INT start_freq) {
  INT k0_min;

  switch (fsCore) {
    case 8000:
      k0_min = 24; /* (3000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 11025:
      k0_min = 17; /* (3000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 12000:
      k0_min = 16; /* (3000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 16000:
      k0_min = 16; /* (4000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 22050:
      k0_min = 12; /* (4000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 24000:
      k0_min = 11; /* (4000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 32000:
      k0_min = 10; /* (5000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 44100:
      k0_min = 7; /* (5000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 48000:
      k0_min = 7; /* (5000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    case 96000:
      k0_min = 3; /* (5000 * nQmfChannels / fsSBR ) + 0.5 */
      break;
    default:
      k0_min = 11; /* illegal fs */
  }

  switch (fsCore) {
    case 8000: {
      INT v_offset[] = {-8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7};
      return (k0_min + v_offset[start_freq]);
    }
    case 11025: {
      INT v_offset[] = {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13};
      return (k0_min + v_offset[start_freq]);
    }
    case 12000: {
      INT v_offset[] = {-5, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16};
      return (k0_min + v_offset[start_freq]);
    }
    case 16000: {
      INT v_offset[] = {-6, -4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16};
      return (k0_min + v_offset[start_freq]);
    }
    case 22050:
    case 24000:
    case 32000: {
      INT v_offset[] = {-4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20};
      return (k0_min + v_offset[start_freq]);
    }
    case 44100:
    case 48000:
    case 96000: {
      INT v_offset[] = {-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24};
      return (k0_min + v_offset[start_freq]);
    }
    default: {
      INT v_offset[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24, 28, 33};
      return (k0_min + v_offset[start_freq]);
    }
  }
} /* End getStartFreq */

/*******************************************************************************
 Functionname:  getStopFreq
 *******************************************************************************
 Description:

 Arguments:

 Return:
 *******************************************************************************/
static INT getStopFreq(INT fsCore, const INT stop_freq) {
  INT result, i;
  INT k1_min;
  INT v_dstop[13];

  INT *v_stop_freq = NULL;
  INT v_stop_freq_16[14] = {48, 49, 50, 51, 52, 54, 55,
                            56, 57, 59, 60, 61, 63, 64};
  INT v_stop_freq_22[14] = {35, 37, 38, 40, 42, 44, 46,
                            48, 51, 53, 56, 58, 61, 64};
  INT v_stop_freq_24[14] = {32, 34, 36, 38, 40, 42, 44,
                            46, 49, 52, 55, 58, 61, 64};
  INT v_stop_freq_32[14] = {32, 34, 36, 38, 40, 42, 44,
                            46, 49, 52, 55, 58, 61, 64};
  INT v_stop_freq_44[14] = {23, 25, 27, 29, 32, 34, 37,
                            40, 43, 47, 51, 55, 59, 64};
  INT v_stop_freq_48[14] = {21, 23, 25, 27, 30, 32, 35,
                            38, 42, 45, 49, 54, 59, 64};
  INT v_stop_freq_64[14] = {20, 22, 24, 26, 29, 31, 34,
                            37, 41, 45, 49, 54, 59, 64};
  INT v_stop_freq_88[14] = {15, 17, 19, 21, 23, 26, 29,
                            33, 37, 41, 46, 51, 57, 64};
  INT v_stop_freq_96[14] = {13, 15, 17, 19, 21, 24, 27,
                            31, 35, 39, 44, 50, 57, 64};
  INT v_stop_freq_192[14] = {7,  8,  10, 12, 14, 16, 19,
                             23, 27, 32, 38, 46, 54, 64};

  switch (fsCore) {
    case 8000:
      k1_min = 48;
      v_stop_freq = v_stop_freq_16;
      break;
    case 11025:
      k1_min = 35;
      v_stop_freq = v_stop_freq_22;
      break;
    case 12000:
      k1_min = 32;
      v_stop_freq = v_stop_freq_24;
      break;
    case 16000:
      k1_min = 32;
      v_stop_freq = v_stop_freq_32;
      break;
    case 22050:
      k1_min = 23;
      v_stop_freq = v_stop_freq_44;
      break;
    case 24000:
      k1_min = 21;
      v_stop_freq = v_stop_freq_48;
      break;
    case 32000:
      k1_min = 20;
      v_stop_freq = v_stop_freq_64;
      break;
    case 44100:
      k1_min = 15;
      v_stop_freq = v_stop_freq_88;
      break;
    case 48000:
      k1_min = 13;
      v_stop_freq = v_stop_freq_96;
      break;
    case 96000:
      k1_min = 7;
      v_stop_freq = v_stop_freq_192;
      break;
    default:
      k1_min = 21; /* illegal fs  */
  }

  /* Ensure increasing bandwidth */
  for (i = 0; i <= 12; i++) {
    v_dstop[i] = v_stop_freq[i + 1] - v_stop_freq[i];
  }

  FDKsbrEnc_Shellsort_int(v_dstop, 13); /* Sort bandwidth changes */

  result = k1_min;
  for (i = 0; i < stop_freq; i++) {
    result = result + v_dstop[i];
  }

  return (result);

} /* End getStopFreq */

/*******************************************************************************
 Functionname:  FDKsbrEnc_FindStartAndStopBand
 *******************************************************************************
 Description:

 Arguments:     srSbr            SBR sampling freqency
                srCore           AAC core sampling freqency
                noChannels       Number of QMF channels
                startFreq        SBR start frequency in QMF bands
                stopFreq         SBR start frequency in QMF bands

               *k0               Output parameter
               *k2               Output parameter

 Return:       Error code (0 is OK)
 *******************************************************************************/
INT FDKsbrEnc_FindStartAndStopBand(const INT srSbr, const INT srCore,
                                   const INT noChannels, const INT startFreq,
                                   const INT stopFreq, INT *k0, INT *k2) {
  /* Update startFreq struct */
  *k0 = getStartFreq(srCore, startFreq);

  /* Test if start freq is outside corecoder range */
  if (srSbr * noChannels < *k0 * srCore) {
    return (
        1); /* raise the cross-over frequency and/or lower the number
               of target bands per octave (or lower the sampling frequency) */
  }

  /*Update stopFreq struct */
  if (stopFreq < 14) {
    *k2 = getStopFreq(srCore, stopFreq);
  } else if (stopFreq == 14) {
    *k2 = 2 * *k0;
  } else {
    *k2 = 3 * *k0;
  }

  /* limit to Nyqvist */
  if (*k2 > noChannels) {
    *k2 = noChannels;
  }

  /* Test for invalid  k0 k2 combinations */
  if ((srCore == 22050) && ((*k2 - *k0) > MAX_FREQ_COEFFS_FS44100))
    return (1); /* Number of bands exceeds valid range of MAX_FREQ_COEFFS for
                   fs=44.1kHz */

  if ((srCore >= 24000) && ((*k2 - *k0) > MAX_FREQ_COEFFS_FS48000))
    return (1); /* Number of bands exceeds valid range of MAX_FREQ_COEFFS for
                   fs>=48kHz */

  if ((*k2 - *k0) > MAX_FREQ_COEFFS)
    return (1); /*Number of bands exceeds valid range of MAX_FREQ_COEFFS */

  if ((*k2 - *k0) < 0) return (1); /* Number of bands is negative */

  return (0);
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_UpdateFreqScale
 *******************************************************************************
 Description:

 Arguments:

 Return:
 *******************************************************************************/
INT FDKsbrEnc_UpdateFreqScale(UCHAR *v_k_master, INT *h_num_bands, const INT k0,
                              const INT k2, const INT freqScale,
                              const INT alterScale)

{
  INT b_p_o = 0; /* bands_per_octave */
  FIXP_DBL warp = FL2FXCONST_DBL(0.0f);
  INT dk = 0;

  /* Internal variables */
  INT k1 = 0, i;
  INT num_bands0;
  INT num_bands1;
  INT diff_tot[MAX_OCTAVE + MAX_SECOND_REGION];
  INT *diff0 = diff_tot;
  INT *diff1 = diff_tot + MAX_OCTAVE;
  INT k2_achived;
  INT k2_diff;
  INT incr = 0;

  /* Init */
  if (freqScale == 1) b_p_o = 12;
  if (freqScale == 2) b_p_o = 10;
  if (freqScale == 3) b_p_o = 8;

  if (freqScale > 0) /*Bark*/
  {
    if (alterScale == 0)
      warp = FL2FXCONST_DBL(0.5f); /* 1.0/(1.0*2.0) */
    else
      warp = FL2FXCONST_DBL(1.0f / 2.6f); /* 1.0/(1.3*2.0); */

    if (4 * k2 >= 9 * k0) /*two or more regions (how many times the basis band
                             is copied)*/
    {
      k1 = 2 * k0;

      num_bands0 = numberOfBands(b_p_o, k0, k1, FL2FXCONST_DBL(0.5f));
      num_bands1 = numberOfBands(b_p_o, k1, k2, warp);

      CalcBands(diff0, k0, k1, num_bands0);       /*CalcBands1 => diff0 */
      FDKsbrEnc_Shellsort_int(diff0, num_bands0); /*SortBands sort diff0 */

      if (diff0[0] == 0) /* too wide FB bands for target tuning */
      {
        return (1); /* raise the cross-over frequency and/or lower the number
                       of target bands per octave (or lower the sampling
                       frequency */
      }

      cumSum(k0, diff0, num_bands0, v_k_master); /* cumsum */

      CalcBands(diff1, k1, k2, num_bands1);       /* CalcBands2 => diff1 */
      FDKsbrEnc_Shellsort_int(diff1, num_bands1); /* SortBands sort diff1 */
      if (diff0[num_bands0 - 1] > diff1[0])       /* max(1) > min(2) */
      {
        if (modifyBands(diff0[num_bands0 - 1], diff1, num_bands1)) return (1);
      }

      /* Add 2'nd region */
      cumSum(k1, diff1, num_bands1, &v_k_master[num_bands0]);
      *h_num_bands = num_bands0 + num_bands1; /* Output nr of bands */

    } else /* one region */
    {
      k1 = k2;

      num_bands0 = numberOfBands(b_p_o, k0, k1, FL2FXCONST_DBL(0.5f));
      CalcBands(diff0, k0, k1, num_bands0);       /* CalcBands1 => diff0 */
      FDKsbrEnc_Shellsort_int(diff0, num_bands0); /* SortBands sort diff0 */

      if (diff0[0] == 0) /* too wide FB bands for target tuning */
      {
        return (1); /* raise the cross-over frequency and/or lower the number
                       of target bands per octave (or lower the sampling
                       frequency */
      }

      cumSum(k0, diff0, num_bands0, v_k_master); /* cumsum */
      *h_num_bands = num_bands0;                 /* Output nr of bands */
    }
  } else /* Linear mode */
  {
    if (alterScale == 0) {
      dk = 1;
      num_bands0 = 2 * ((k2 - k0) / 2); /* FLOOR to get to few number of bands*/
    } else {
      dk = 2;
      num_bands0 =
          2 * (((k2 - k0) / dk + 1) / 2); /* ROUND to get closest fit */
    }

    k2_achived = k0 + num_bands0 * dk;
    k2_diff = k2 - k2_achived;

    for (i = 0; i < num_bands0; i++) diff_tot[i] = dk;

    /* If linear scale wasn't achived */
    /* and we got wide SBR are */
    if (k2_diff < 0) {
      incr = 1;
      i = 0;
    }

    /* If linear scale wasn't achived */
    /* and we got small SBR are */
    if (k2_diff > 0) {
      incr = -1;
      i = num_bands0 - 1;
    }

    /* Adjust diff vector to get sepc. SBR range */
    while (k2_diff != 0) {
      diff_tot[i] = diff_tot[i] - incr;
      i = i + incr;
      k2_diff = k2_diff + incr;
    }

    cumSum(k0, diff_tot, num_bands0, v_k_master); /* cumsum */
    *h_num_bands = num_bands0;                    /* Output nr of bands */
  }

  if (*h_num_bands < 1) return (1); /*To small sbr area */

  return (0);
} /* End FDKsbrEnc_UpdateFreqScale */

static INT numberOfBands(INT b_p_o, INT start, INT stop, FIXP_DBL warp_factor) {
  INT result = 0;
  /* result = 2* (INT) ( (double)b_p_o *
   * (double)(FDKlog((double)stop/(double)start)/FDKlog((double)2)) *
   * (double)FX_DBL2FL(warp_factor) + 0.5); */
  result = ((b_p_o * fMult((CalcLdInt(stop) - CalcLdInt(start)), warp_factor) +
             (FL2FX_DBL(0.5f) >> LD_DATA_SHIFT)) >>
            ((DFRACT_BITS - 1) - LD_DATA_SHIFT))
           << 1; /* do not optimize anymore (rounding!!) */

  return (result);
}

static void CalcBands(INT *diff, INT start, INT stop, INT num_bands) {
  INT i, qb, qe, qtmp;
  INT previous;
  INT current;
  FIXP_DBL base, exp, tmp;

  previous = start;
  for (i = 1; i <= num_bands; i++) {
    base = fDivNorm((FIXP_DBL)stop, (FIXP_DBL)start, &qb);
    exp = fDivNorm((FIXP_DBL)i, (FIXP_DBL)num_bands, &qe);
    tmp = fPow(base, qb, exp, qe, &qtmp);
    tmp = fMult(tmp, (FIXP_DBL)(start << 24));
    current = (INT)scaleValue(tmp, qtmp - 23);
    current = (current + 1) >> 1; /* rounding*/
    diff[i - 1] = current - previous;
    previous = current;
  }

} /* End CalcBands */

static void cumSum(INT start_value, INT *diff, INT length,
                   UCHAR *start_adress) {
  INT i;
  start_adress[0] = start_value;
  for (i = 1; i <= length; i++)
    start_adress[i] = start_adress[i - 1] + diff[i - 1];
} /* End cumSum */

static INT modifyBands(INT max_band_previous, INT *diff, INT length) {
  INT change = max_band_previous - diff[0];

  /* Limit the change so that the last band cannot get narrower than the first
   * one */
  if (change > (diff[length - 1] - diff[0]) / 2)
    change = (diff[length - 1] - diff[0]) / 2;

  diff[0] += change;
  diff[length - 1] -= change;
  FDKsbrEnc_Shellsort_int(diff, length);

  return (0);
} /* End modifyBands */

/*******************************************************************************
 Functionname:  FDKsbrEnc_UpdateHiRes
 *******************************************************************************
 Description:


 Arguments:

 Return:
 *******************************************************************************/
INT FDKsbrEnc_UpdateHiRes(UCHAR *h_hires, INT *num_hires, UCHAR *v_k_master,
                          INT num_master, INT *xover_band) {
  INT i;
  INT max1, max2;

  if ((v_k_master[*xover_band] >
       32) || /* v_k_master[*xover_band] > noQMFChannels(dualRate)/divider */
      (*xover_band > num_master)) {
    /* xover_band error, too big for this startFreq. Will be clipped */

    /* Calculate maximum value for xover_band */
    max1 = 0;
    max2 = num_master;
    while ((v_k_master[max1 + 1] < 32) && /* noQMFChannels(dualRate)/divider */
           ((max1 + 1) < max2)) {
      max1++;
    }

    *xover_band = max1;
  }

  *num_hires = num_master - *xover_band;
  for (i = *xover_band; i <= num_master; i++) {
    h_hires[i - *xover_band] = v_k_master[i];
  }

  return (0);
} /* End FDKsbrEnc_UpdateHiRes */

/*******************************************************************************
 Functionname:  FDKsbrEnc_UpdateLoRes
 *******************************************************************************
 Description:

 Arguments:

 Return:
 *******************************************************************************/
void FDKsbrEnc_UpdateLoRes(UCHAR *h_lores, INT *num_lores, UCHAR *h_hires,
                           INT num_hires) {
  INT i;

  if (num_hires % 2 == 0) /* if even number of hires bands */
  {
    *num_lores = num_hires / 2;
    /* Use every second lores=hires[0,2,4...] */
    for (i = 0; i <= *num_lores; i++) h_lores[i] = h_hires[i * 2];

  } else /* odd number of hires which means xover is odd */
  {
    *num_lores = (num_hires + 1) / 2;

    /* Use lores=hires[0,1,3,5 ...] */
    h_lores[0] = h_hires[0];
    for (i = 1; i <= *num_lores; i++) {
      h_lores[i] = h_hires[i * 2 - 1];
    }
  }

} /* End FDKsbrEnc_UpdateLoRes */
