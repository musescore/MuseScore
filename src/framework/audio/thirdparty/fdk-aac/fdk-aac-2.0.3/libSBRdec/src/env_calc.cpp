/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2021 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Envelope calculation

  The envelope adjustor compares the energies present in the transposed
  highband to the reference energies conveyed with the bitstream.
  The highband is amplified (sometimes) or attenuated (mostly) to the
  desired level.

  The spectral shape of the reference energies can be changed several times per
  frame if necessary. Each set of energy values corresponding to a certain range
  in time will be called an <em>envelope</em> here.
  The bitstream supports several frequency scales and two resolutions. Normally,
  one or more QMF-subbands are grouped to one SBR-band. An envelope contains
  reference energies for each SBR-band.
  In addition to the energy envelopes, noise envelopes are transmitted that
  define the ratio of energy which is generated by adding noise instead of
  transposing the lowband. The noise envelopes are given in a coarser time
  and frequency resolution.
  If a signal contains strong tonal components, synthetic sines can be
  generated in individual SBR bands.

  An overlap buffer of 6 QMF-timeslots is used to allow a more
  flexible alignment of the envelopes in time that is not restricted to the
  core codec's frame borders.
  Therefore the envelope adjustor has access to the spectral data of the
  current frame as well as the last 6 QMF-timeslots of the previous frame.
  However, in average only the data of 1 frame is being processed as
  the adjustor is called once per frame.

  Depending on the frequency range set in the bitstream, only QMF-subbands
  between <em>lowSubband</em> and <em>highSubband</em> are adjusted.

  Scaling of spectral data to maximize SNR (see #QMF_SCALE_FACTOR) as well as a
  special Mantissa-Exponent format ( see  calculateSbrEnvelope() ) are being
  used. The main entry point for this modules is calculateSbrEnvelope().

  \sa sbr_scale.h, #QMF_SCALE_FACTOR, calculateSbrEnvelope(), \ref
  documentationOverview
*/

#include "env_calc.h"

#include "sbrdec_freq_sca.h"
#include "env_extr.h"
#include "transcendent.h"
#include "sbr_ram.h"
#include "sbr_rom.h"

#include "genericStds.h" /* need FDKpow() for debug outputs */

#define MAX_SFB_NRG_HEADROOM (1)
#define MAX_VAL_NRG_HEADROOM ((((FIXP_DBL)MAXVAL_DBL) >> MAX_SFB_NRG_HEADROOM))

typedef struct {
  FIXP_DBL nrgRef[MAX_FREQ_COEFFS];
  FIXP_DBL nrgEst[MAX_FREQ_COEFFS];
  FIXP_DBL nrgGain[MAX_FREQ_COEFFS];
  FIXP_DBL noiseLevel[MAX_FREQ_COEFFS];
  FIXP_DBL nrgSine[MAX_FREQ_COEFFS];

  SCHAR nrgRef_e[MAX_FREQ_COEFFS];
  SCHAR nrgEst_e[MAX_FREQ_COEFFS];
  SCHAR nrgGain_e[MAX_FREQ_COEFFS];
  SCHAR noiseLevel_e[MAX_FREQ_COEFFS];
  SCHAR nrgSine_e[MAX_FREQ_COEFFS];
  /* yet another exponent [0]: for ts < no_cols; [1]: for ts >= no_cols */
  SCHAR exponent[2];
} ENV_CALC_NRGS;

static void equalizeFiltBufferExp(FIXP_DBL *filtBuffer, SCHAR *filtBuffer_e,
                                  FIXP_DBL *NrgGain, SCHAR *NrgGain_e,
                                  int subbands);

static void calcNrgPerSubband(FIXP_DBL **analysBufferReal,
                              FIXP_DBL **analysBufferImag, int lowSubband,
                              int highSubband, int start_pos, int next_pos,
                              SCHAR frameExp, FIXP_DBL *nrgEst,
                              SCHAR *nrgEst_e);

static void calcNrgPerSfb(FIXP_DBL **analysBufferReal,
                          FIXP_DBL **analysBufferImag, int nSfb,
                          UCHAR *freqBandTable, int start_pos, int next_pos,
                          SCHAR input_e, FIXP_DBL *nrg_est, SCHAR *nrg_est_e);

static void calcSubbandGain(FIXP_DBL nrgRef, SCHAR nrgRef_e,
                            ENV_CALC_NRGS *nrgs, int c, FIXP_DBL tmpNoise,
                            SCHAR tmpNoise_e, UCHAR sinePresentFlag,
                            UCHAR sineMapped, int noNoiseFlag);

static void calcAvgGain(ENV_CALC_NRGS *nrgs, int lowSubband, int highSubband,
                        FIXP_DBL *sumRef_m, SCHAR *sumRef_e,
                        FIXP_DBL *ptrAvgGain_m, SCHAR *ptrAvgGain_e);

static void adjustTimeSlot_EldGrid(FIXP_DBL *ptrReal, ENV_CALC_NRGS *nrgs,
                                   UCHAR *ptrHarmIndex, int lowSubbands,
                                   int noSubbands, int scale_change,
                                   int noNoiseFlag, int *ptrPhaseIndex,
                                   int scale_diff_low);

static void adjustTimeSlotLC(FIXP_DBL *ptrReal, ENV_CALC_NRGS *nrgs,
                             UCHAR *ptrHarmIndex, int lowSubbands,
                             int noSubbands, int scale_change, int noNoiseFlag,
                             int *ptrPhaseIndex);

/**
 * \brief Variant of adjustTimeSlotHQ() which only regards gain and noise but no
 * additional harmonics
 */
static void adjustTimeSlotHQ_GainAndNoise(
    FIXP_DBL *ptrReal, FIXP_DBL *ptrImag,
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env, ENV_CALC_NRGS *nrgs,
    int lowSubbands, int noSubbands, int scale_change, FIXP_SGL smooth_ratio,
    int noNoiseFlag, int filtBufferNoiseShift);
/**
 * \brief Variant of adjustTimeSlotHQ() which only adds the additional harmonics
 */
static void adjustTimeSlotHQ_AddHarmonics(
    FIXP_DBL *ptrReal, FIXP_DBL *ptrImag,
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env, ENV_CALC_NRGS *nrgs,
    int lowSubbands, int noSubbands, int scale_change);

static void adjustTimeSlotHQ(FIXP_DBL *ptrReal, FIXP_DBL *ptrImag,
                             HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env,
                             ENV_CALC_NRGS *nrgs, int lowSubbands,
                             int noSubbands, int scale_change,
                             FIXP_SGL smooth_ratio, int noNoiseFlag,
                             int filtBufferNoiseShift);

/*!
  \brief     Map sine flags from bitstream to QMF bands

  The bitstream carries only 1 sine flag per band (Sfb) and frame.
  This function maps every sine flag from the bitstream to a specific QMF
  subband and to a specific envelope where the sine shall start. The result is
  stored in the vector sineMapped which contains one entry per QMF subband. The
  value of an entry specifies the envelope where a sine shall start. A value of
  32 indicates that no sine is present in the subband. The missing harmonics
  flags from the previous frame (harmFlagsPrev) determine if a sine starts at
  the beginning of the frame or at the transient position. Additionally, the
  flags in harmFlagsPrev are being updated by this function for the next frame.
*/
static void mapSineFlags(
    UCHAR *freqBandTable, /*!< Band borders (there's only 1 flag per band) */
    int nSfb,             /*!< Number of bands in the table */
    ULONG *addHarmonics,  /*!< Packed addHarmonics of current frame (aligned to
                             the MSB) */
    ULONG *harmFlagsPrev, /*!< Packed addHarmonics of previous frame (aligned to
                             the LSB) */
    ULONG *harmFlagsPrevActive, /*!< Packed sineMapped of previous frame
                                   (aligned to the LSB) */
    int tranEnv,                /*!< Transient position */
    SCHAR *sineMapped) /*!< Resulting vector of sine start positions for each
                          QMF band */

{
  int i;
  int bitcount = 31;
  ULONG harmFlagsQmfBands[ADD_HARMONICS_FLAGS_SIZE] = {0};
  ULONG *curFlags = addHarmonics;

  /*
    Format of addHarmonics (aligned to MSB):

      Up to MAX_FREQ_COEFFS sfb bands can be flagged for a sign.
      first word  = flags for lowest 32 sfb bands in use
      second word = flags for higest 32 sfb bands (if present)

    Format of harmFlagsPrev (aligned to LSB):

      Index is absolute (not relative to lsb) so it is correct even if lsb
    changes first word  = flags for lowest 32 qmf bands (0...31) second word =
    flags for next higher 32 qmf bands (32...63)

  */

  /* Reset the output vector first */
  FDKmemset(sineMapped, 32,
            MAX_FREQ_COEFFS * sizeof(SCHAR)); /* 32 means 'no sine' */
  FDKmemclear(harmFlagsPrevActive, ADD_HARMONICS_FLAGS_SIZE * sizeof(ULONG));
  for (i = 0; i < nSfb; i++) {
    ULONG maskSfb =
        1 << bitcount; /* mask to extract addHarmonics flag of current Sfb */

    if (*curFlags & maskSfb) {          /* There is a sine in this band */
      const int lsb = freqBandTable[0]; /* start of sbr range */
      /* qmf band to which sine should be added */
      const int qmfBand = (freqBandTable[i] + freqBandTable[i + 1]) >> 1;
      const int qmfBandDiv32 = qmfBand >> 5;
      const int maskQmfBand =
          1 << (qmfBand &
                31); /* mask to extract harmonic flag from prevFlags */

      /* mapping of sfb with sine to a certain qmf band -> for harmFlagsPrev */
      harmFlagsQmfBands[qmfBandDiv32] |= maskQmfBand;

      /*
        If there was a sine in the last frame, let it continue from the first
        envelope on else start at the transient position. Indexing of sineMapped
        starts relative to lsb.
      */
      sineMapped[qmfBand - lsb] =
          (harmFlagsPrev[qmfBandDiv32] & maskQmfBand) ? 0 : tranEnv;
      if (sineMapped[qmfBand - lsb] < PVC_NTIMESLOT) {
        harmFlagsPrevActive[qmfBandDiv32] |= maskQmfBand;
      }
    }

    if (bitcount-- == 0) {
      bitcount = 31;
      curFlags++;
    }
  }
  FDKmemcpy(harmFlagsPrev, harmFlagsQmfBands,
            sizeof(ULONG) * ADD_HARMONICS_FLAGS_SIZE);
}

/*!
  \brief     Restore sineMapped of previous frame

  For PVC it might happen that the PVC framing (always 0) is out of sync with
  the SBR framing. The adding of additional harmonics is done based on the SBR
  framing. If the SBR framing is trailing the PVC framing the sine mapping of
  the previous SBR frame needs to be used for the overlapping time slots.
*/
/*static*/ void mapSineFlagsPvc(
    UCHAR *freqBandTable,       /*!< Band borders (there's only 1 flag per
                                   band) */
    int nSfb,                   /*!< Number of bands in the table */
    ULONG *harmFlagsPrev,       /*!< Packed addHarmonics of previous frame
                                   (aligned to the MSB) */
    ULONG *harmFlagsPrevActive, /*!< Packed sineMapped of previous
                                   frame (aligned to the LSB) */
    SCHAR *sineMapped,          /*!< Resulting vector of sine start positions
                                   for each QMF band */
    int sinusoidalPos,          /*!< sinusoidal position */
    SCHAR *sinusoidalPosPrev,   /*!< sinusoidal position of previous
                                   frame */
    int trailingSbrFrame)       /*!< indication if the SBR framing is
                                   trailing the PVC framing */
{
  /* Reset the output vector first */
  FDKmemset(sineMapped, 32, MAX_FREQ_COEFFS); /* 32 means 'no sine' */

  if (trailingSbrFrame) {
    /* restore sineMapped[] of previous frame */
    int i;
    const int lsb = freqBandTable[0];
    const int usb = freqBandTable[nSfb];
    for (i = lsb; i < usb; i++) {
      const int qmfBandDiv32 = i >> 5;
      const int maskQmfBand =
          1 << (i & 31); /* mask to extract harmonic flag from prevFlags */

      /* Two cases need to be distinguished ... */
      if (harmFlagsPrevActive[qmfBandDiv32] & maskQmfBand) {
        /* the sine mapping already started last PVC frame -> seamlessly
         * continue */
        sineMapped[i - lsb] = 0;
      } else if (harmFlagsPrev[qmfBandDiv32] & maskQmfBand) {
        /* sinusoidalPos of prev PVC frame was >= PVC_NTIMESLOT -> sine starts
         * in this frame */
        sineMapped[i - lsb] =
            *sinusoidalPosPrev - PVC_NTIMESLOT; /* we are 16 sbr time slots
                                                   ahead of last frame now */
      }
    }
  }
  *sinusoidalPosPrev = sinusoidalPos;
}

/*!
  \brief     Reduce gain-adjustment induced aliasing for real valued filterbank.
*/
/*static*/ void aliasingReduction(
    FIXP_DBL *degreeAlias, /*!< estimated aliasing for each QMF
                              channel */
    ENV_CALC_NRGS *nrgs,
    UCHAR *useAliasReduction, /*!< synthetic sine energy for each
                                 subband, used as flag */
    int noSubbands)           /*!< number of QMF channels to process */
{
  FIXP_DBL *nrgGain = nrgs->nrgGain; /*!< subband gains to be modified */
  SCHAR *nrgGain_e =
      nrgs->nrgGain_e; /*!< subband gains to be modified (exponents) */
  FIXP_DBL *nrgEst = nrgs->nrgEst; /*!< subband energy before amplification */
  SCHAR *nrgEst_e =
      nrgs->nrgEst_e; /*!< subband energy before amplification (exponents) */
  int grouping = 0, index = 0, noGroups, k;
  int groupVector[MAX_FREQ_COEFFS];

  /* Calculate grouping*/
  for (k = 0; k < noSubbands - 1; k++) {
    if ((degreeAlias[k + 1] != FL2FXCONST_DBL(0.0f)) && useAliasReduction[k]) {
      if (grouping == 0) {
        groupVector[index++] = k;
        grouping = 1;
      } else {
        if (groupVector[index - 1] + 3 == k) {
          groupVector[index++] = k + 1;
          grouping = 0;
        }
      }
    } else {
      if (grouping) {
        if (useAliasReduction[k])
          groupVector[index++] = k + 1;
        else
          groupVector[index++] = k;
        grouping = 0;
      }
    }
  }

  if (grouping) {
    groupVector[index++] = noSubbands;
  }
  noGroups = index >> 1;

  /*Calculate new gain*/
  for (int group = 0; group < noGroups; group++) {
    FIXP_DBL nrgOrig = FL2FXCONST_DBL(
        0.0f); /* Original signal energy in current group of bands */
    SCHAR nrgOrig_e = 0;
    FIXP_DBL nrgAmp = FL2FXCONST_DBL(
        0.0f); /* Amplified signal energy in group (using current gains) */
    SCHAR nrgAmp_e = 0;
    FIXP_DBL nrgMod = FL2FXCONST_DBL(
        0.0f); /* Signal energy in group when applying modified gains */
    SCHAR nrgMod_e = 0;
    FIXP_DBL groupGain; /* Total energy gain in group */
    SCHAR groupGain_e;
    FIXP_DBL compensation; /* Compensation factor for the energy change when
                              applying modified gains */
    SCHAR compensation_e;

    int startGroup = groupVector[2 * group];
    int stopGroup = groupVector[2 * group + 1];

    /* Calculate total energy in group before and after amplification with
     * current gains: */
    for (k = startGroup; k < stopGroup; k++) {
      /* Get original band energy */
      FIXP_DBL tmp = nrgEst[k];
      SCHAR tmp_e = nrgEst_e[k];

      FDK_add_MantExp(tmp, tmp_e, nrgOrig, nrgOrig_e, &nrgOrig, &nrgOrig_e);

      /* Multiply band energy with current gain */
      tmp = fMult(tmp, nrgGain[k]);
      tmp_e = tmp_e + nrgGain_e[k];

      FDK_add_MantExp(tmp, tmp_e, nrgAmp, nrgAmp_e, &nrgAmp, &nrgAmp_e);
    }

    /* Calculate total energy gain in group */
    FDK_divide_MantExp(nrgAmp, nrgAmp_e, nrgOrig, nrgOrig_e, &groupGain,
                       &groupGain_e);

    for (k = startGroup; k < stopGroup; k++) {
      FIXP_DBL tmp;
      SCHAR tmp_e;

      FIXP_DBL alpha = degreeAlias[k];
      if (k < noSubbands - 1) {
        if (degreeAlias[k + 1] > alpha) alpha = degreeAlias[k + 1];
      }

      /* Modify gain depending on the degree of aliasing */
      FDK_add_MantExp(
          fMult(alpha, groupGain), groupGain_e,
          fMult(/*FL2FXCONST_DBL(1.0f)*/ (FIXP_DBL)MAXVAL_DBL - alpha,
                nrgGain[k]),
          nrgGain_e[k], &nrgGain[k], &nrgGain_e[k]);

      /* Apply modified gain to original energy */
      tmp = fMult(nrgGain[k], nrgEst[k]);
      tmp_e = nrgGain_e[k] + nrgEst_e[k];

      /* Accumulate energy with modified gains applied */
      FDK_add_MantExp(tmp, tmp_e, nrgMod, nrgMod_e, &nrgMod, &nrgMod_e);
    }

    /* Calculate compensation factor to retain the energy of the amplified
     * signal */
    FDK_divide_MantExp(nrgAmp, nrgAmp_e, nrgMod, nrgMod_e, &compensation,
                       &compensation_e);

    /* Apply compensation factor to all gains of the group */
    for (k = startGroup; k < stopGroup; k++) {
      nrgGain[k] = fMult(nrgGain[k], compensation);
      nrgGain_e[k] = nrgGain_e[k] + compensation_e;
    }
  }
}

#define INTER_TES_SF_CHANGE 4

typedef struct {
  FIXP_DBL subsample_power_low[(((1024) / (32) * (4) / 2) + (3 * (4)))];
  FIXP_DBL subsample_power_high[(((1024) / (32) * (4) / 2) + (3 * (4)))];
  FIXP_DBL gain[(((1024) / (32) * (4) / 2) + (3 * (4)))];
  SCHAR subsample_power_low_sf[(((1024) / (32) * (4) / 2) + (3 * (4)))];
  SCHAR subsample_power_high_sf[(((1024) / (32) * (4) / 2) + (3 * (4)))];
} ITES_TEMP;

static void apply_inter_tes(FIXP_DBL **qmfReal, FIXP_DBL **qmfImag,
                            const QMF_SCALE_FACTOR *sbrScaleFactor,
                            const SCHAR exp[2], const int RATE,
                            const int startPos, const int stopPos,
                            const int lowSubband, const int nbSubband,
                            const UCHAR gamma_idx) {
  int highSubband = lowSubband + nbSubband;
  FIXP_DBL *subsample_power_high, *subsample_power_low;
  SCHAR *subsample_power_high_sf, *subsample_power_low_sf;
  FIXP_DBL total_power_high = (FIXP_DBL)0;
  FIXP_DBL total_power_low = (FIXP_DBL)0;
  FIXP_DBL *gain;
  int gain_sf[(((1024) / (32) * (4) / 2) + (3 * (4)))];

  /* gamma[gamma_idx] = {0.0f, 1.0f, 2.0f, 4.0f} */
  int gamma_sf =
      (int)gamma_idx - 1; /* perhaps +1 to save one bit? (0.99999f vs 1.f) */

  int nbSubsample = stopPos - startPos;
  int i, j;

  C_ALLOC_SCRATCH_START(pTmp, ITES_TEMP, 1);
  subsample_power_high = pTmp->subsample_power_high;
  subsample_power_low = pTmp->subsample_power_low;
  subsample_power_high_sf = pTmp->subsample_power_high_sf;
  subsample_power_low_sf = pTmp->subsample_power_low_sf;
  gain = pTmp->gain;

  if (gamma_idx > 0) {
    int preShift2 = 32 - fNormz((FIXP_DBL)nbSubsample);
    int total_power_low_sf = 1 - DFRACT_BITS;
    int total_power_high_sf = 1 - DFRACT_BITS;

    for (i = 0; i < nbSubsample; ++i) {
      FIXP_DBL bufferReal[(((1024) / (32) * (4) / 2) + (3 * (4)))];
      FIXP_DBL bufferImag[(((1024) / (32) * (4) / 2) + (3 * (4)))];
      FIXP_DBL maxVal = (FIXP_DBL)0;

      int ts = startPos + i;

      int low_sf = (ts < 3 * RATE) ? sbrScaleFactor->ov_lb_scale
                                   : sbrScaleFactor->lb_scale;
      low_sf = 15 - low_sf;

      for (j = 0; j < lowSubband; ++j) {
        bufferImag[j] = qmfImag[startPos + i][j];
        maxVal |= (FIXP_DBL)((LONG)(bufferImag[j]) ^
                             ((LONG)bufferImag[j] >> (DFRACT_BITS - 1)));
        bufferReal[j] = qmfReal[startPos + i][j];
        maxVal |= (FIXP_DBL)((LONG)(bufferReal[j]) ^
                             ((LONG)bufferReal[j] >> (DFRACT_BITS - 1)));
      }

      subsample_power_low[i] = (FIXP_DBL)0;
      subsample_power_low_sf[i] = 0;

      if (maxVal != FL2FXCONST_DBL(0.f)) {
        /* multiply first, then shift for safe summation */
        int preShift = 1 - CntLeadingZeros(maxVal);
        int postShift = 32 - fNormz((FIXP_DBL)lowSubband);

        /* reduce preShift because otherwise we risk to square -1.f */
        if (preShift != 0) preShift++;

        subsample_power_low_sf[i] += (low_sf + preShift) * 2 + postShift + 1;

        scaleValues(bufferReal, lowSubband, -preShift);
        scaleValues(bufferImag, lowSubband, -preShift);
        for (j = 0; j < lowSubband; ++j) {
          FIXP_DBL addme;
          addme = fPow2Div2(bufferReal[j]);
          subsample_power_low[i] += addme >> postShift;
          addme = fPow2Div2(bufferImag[j]);
          subsample_power_low[i] += addme >> postShift;
        }
      }

      /* now get high */

      maxVal = (FIXP_DBL)0;

      int high_sf = exp[(ts < 16 * RATE) ? 0 : 1];

      for (j = lowSubband; j < highSubband; ++j) {
        bufferImag[j] = qmfImag[startPos + i][j];
        maxVal |= (FIXP_DBL)((LONG)(bufferImag[j]) ^
                             ((LONG)bufferImag[j] >> (DFRACT_BITS - 1)));
        bufferReal[j] = qmfReal[startPos + i][j];
        maxVal |= (FIXP_DBL)((LONG)(bufferReal[j]) ^
                             ((LONG)bufferReal[j] >> (DFRACT_BITS - 1)));
      }

      subsample_power_high[i] = (FIXP_DBL)0;
      subsample_power_high_sf[i] = 0;

      if (maxVal != FL2FXCONST_DBL(0.f)) {
        int preShift = 1 - CntLeadingZeros(maxVal);
        /* reduce preShift because otherwise we risk to square -1.f */
        if (preShift != 0) preShift++;

        int postShift = 32 - fNormz((FIXP_DBL)(highSubband - lowSubband));
        subsample_power_high_sf[i] += (high_sf + preShift) * 2 + postShift + 1;

        scaleValues(&bufferReal[lowSubband], highSubband - lowSubband,
                    -preShift);
        scaleValues(&bufferImag[lowSubband], highSubband - lowSubband,
                    -preShift);
        for (j = lowSubband; j < highSubband; j++) {
          subsample_power_high[i] += fPow2Div2(bufferReal[j]) >> postShift;
          subsample_power_high[i] += fPow2Div2(bufferImag[j]) >> postShift;
        }
      }

      /* sum all together */
      FIXP_DBL new_summand = subsample_power_low[i];
      int new_summand_sf = subsample_power_low_sf[i];

      /* make sure the current sum, and the new summand have the same SF */
      if (new_summand_sf > total_power_low_sf) {
        int diff = fMin(DFRACT_BITS - 1, new_summand_sf - total_power_low_sf);
        total_power_low >>= diff;
        total_power_low_sf = new_summand_sf;
      } else if (new_summand_sf < total_power_low_sf) {
        new_summand >>=
            fMin(DFRACT_BITS - 1, total_power_low_sf - new_summand_sf);
      }

      total_power_low += (new_summand >> preShift2);

      new_summand = subsample_power_high[i];
      new_summand_sf = subsample_power_high_sf[i];
      if (new_summand_sf > total_power_high_sf) {
        total_power_high >>=
            fMin(DFRACT_BITS - 1, new_summand_sf - total_power_high_sf);
        total_power_high_sf = new_summand_sf;
      } else if (new_summand_sf < total_power_high_sf) {
        new_summand >>=
            fMin(DFRACT_BITS - 1, total_power_high_sf - new_summand_sf);
      }

      total_power_high += (new_summand >> preShift2);
    }

    total_power_low_sf += preShift2;
    total_power_high_sf += preShift2;

    /* gain[i] = e_LOW[i] */
    for (i = 0; i < nbSubsample; ++i) {
      int sf2;
      FIXP_DBL mult =
          fMultNorm(subsample_power_low[i], (FIXP_DBL)nbSubsample, &sf2);
      int mult_sf = subsample_power_low_sf[i] + DFRACT_BITS - 1 + sf2;

      if (total_power_low != FIXP_DBL(0)) {
        gain[i] = fDivNorm(mult, total_power_low, &sf2);
        gain_sf[i] = mult_sf - total_power_low_sf + sf2;
        gain[i] = sqrtFixp_lookup(gain[i], &gain_sf[i]);
        if (gain_sf[i] < 0) {
          gain[i] >>= fMin(DFRACT_BITS - 1, -gain_sf[i]);
          gain_sf[i] = 0;
        }
      } else {
        if (mult == FIXP_DBL(0)) {
          gain[i] = FIXP_DBL(0);
          gain_sf[i] = 0;
        } else {
          gain[i] = (FIXP_DBL)MAXVAL_DBL;
          gain_sf[i] = 0;
        }
      }
    }

    FIXP_DBL total_power_high_after = (FIXP_DBL)0;
    int total_power_high_after_sf = 1 - DFRACT_BITS;

    /* gain[i] = g_inter[i] */
    for (i = 0; i < nbSubsample; ++i) {
      /* calculate: gain[i] = 1.0f + gamma * (gain[i] - 1.0f); */
      FIXP_DBL one = (FIXP_DBL)MAXVAL_DBL >>
                     gain_sf[i]; /* to substract this from gain[i] */

      /* gamma is actually always 1 according to the table, so skip the
       * fMultDiv2 */
      FIXP_DBL mult = (gain[i] - one) >> 1;
      int mult_sf = gain_sf[i] + gamma_sf;

      one = FL2FXCONST_DBL(0.5f) >> mult_sf;
      gain[i] = one + mult;
      gain_sf[i] += gamma_sf + 1; /* +1 because of fMultDiv2() */

      /* set gain to at least 0.2f */
      /* limit and calculate gain[i]^2 too */
      FIXP_DBL gain_pow2;
      int gain_pow2_sf;

      if (fIsLessThan(gain[i], gain_sf[i], FL2FXCONST_DBL(0.2f), 0)) {
        gain[i] = FL2FXCONST_DBL(0.8f);
        gain_sf[i] = -2;
        gain_pow2 = FL2FXCONST_DBL(0.64f);
        gain_pow2_sf = -4;
      } else {
        /* this upscaling seems quite important */
        int r = CountLeadingBits(gain[i]);
        gain[i] <<= r;
        gain_sf[i] -= r;

        gain_pow2 = fPow2(gain[i]);
        gain_pow2_sf = gain_sf[i] << 1;
      }

      int room;
      subsample_power_high[i] =
          fMultNorm(subsample_power_high[i], gain_pow2, &room);
      subsample_power_high_sf[i] =
          subsample_power_high_sf[i] + gain_pow2_sf + room;

      int new_summand_sf = subsample_power_high_sf[i]; /* + gain_pow2_sf; */
      if (new_summand_sf > total_power_high_after_sf) {
        total_power_high_after >>=
            fMin(DFRACT_BITS - 1, new_summand_sf - total_power_high_after_sf);
        total_power_high_after_sf = new_summand_sf;
      } else if (new_summand_sf < total_power_high_after_sf) {
        subsample_power_high[i] >>=
            fMin(DFRACT_BITS - 1, total_power_high_after_sf - new_summand_sf);
      }
      total_power_high_after += subsample_power_high[i] >> preShift2;
    }

    total_power_high_after_sf += preShift2;

    int sf2 = 0;
    FIXP_DBL gain_adj_2 = FL2FX_DBL(0.5f);
    int gain_adj_2_sf = 1;

    if ((total_power_high != (FIXP_DBL)0) &&
        (total_power_high_after != (FIXP_DBL)0)) {
      gain_adj_2 = fDivNorm(total_power_high, total_power_high_after, &sf2);
      gain_adj_2_sf = total_power_high_sf - total_power_high_after_sf + sf2;
    }

    FIXP_DBL gain_adj = sqrtFixp_lookup(gain_adj_2, &gain_adj_2_sf);
    int gain_adj_sf = gain_adj_2_sf;

    for (i = 0; i < nbSubsample; ++i) {
      int gain_e = fMax(
          fMin(gain_sf[i] + gain_adj_sf - INTER_TES_SF_CHANGE, DFRACT_BITS - 1),
          -(DFRACT_BITS - 1));
      FIXP_DBL gain_final = fMult(gain[i], gain_adj);
      gain_final = scaleValueSaturate(gain_final, gain_e);

      for (j = lowSubband; j < highSubband; j++) {
        qmfReal[startPos + i][j] = fMult(qmfReal[startPos + i][j], gain_final);
        qmfImag[startPos + i][j] = fMult(qmfImag[startPos + i][j], gain_final);
      }
    }
  } else { /* gamma_idx == 0 */
    /* Inter-TES is not active. Still perform the scale change to have a
     * consistent scaling for all envelopes of this frame. */
    for (i = 0; i < nbSubsample; ++i) {
      for (j = lowSubband; j < highSubband; j++) {
        qmfReal[startPos + i][j] >>= INTER_TES_SF_CHANGE;
        qmfImag[startPos + i][j] >>= INTER_TES_SF_CHANGE;
      }
    }
  }
  C_ALLOC_SCRATCH_END(pTmp, ITES_TEMP, 1);
}

/*!
  \brief  Apply spectral envelope to subband samples

  This function is called from sbr_dec.cpp in each frame.

  To enhance accuracy and due to the usage of tables for squareroots and
  inverse, some calculations are performed with the operands being split
  into mantissa and exponent. The variable names in the source code carry
  the suffixes <em>_m</em> and  <em>_e</em> respectively. The control data
  in #hFrameData containts envelope data which is represented by this format but
  stored in single words. (See requantizeEnvelopeData() for details). This data
  is unpacked within calculateSbrEnvelope() to follow the described suffix
  convention.

  The actual value (comparable to the corresponding float-variable in the
  research-implementation) of a mantissa/exponent-pair can be calculated as

  \f$ value = value\_m * 2^{value\_e} \f$

  All energies and noise levels decoded from the bitstream suit for an
  original signal magnitude of \f$\pm 32768 \f$ rather than \f$ \pm 1\f$.
  Therefore, the scale factor <em>hb_scale</em> passed into this function will
  be converted to an 'input exponent' (#input_e), which fits the internal
  representation.

  Before the actual processing, an exponent #adj_e for resulting adjusted
  samples is derived from the maximum reference energy.

  Then, for each envelope, the following steps are performed:

  \li Calculate energy in the signal to be adjusted. Depending on the the value
  of #interpolFreq (interpolation mode), this is either done seperately for each
  QMF-subband or for each SBR-band. The resulting energies are stored in
  #nrgEst_m[#MAX_FREQ_COEFFS] (mantissas) and #nrgEst_e[#MAX_FREQ_COEFFS]
  (exponents). \li Calculate gain and noise level for each subband:<br> \f$ gain
  = \sqrt{ \frac{nrgRef}{nrgEst} \cdot (1 - noiseRatio) } \hspace{2cm} noise =
  \sqrt{ nrgRef \cdot noiseRatio } \f$<br> where <em>noiseRatio</em> and
  <em>nrgRef</em> are extracted from the bitstream and <em>nrgEst</em> is the
  subband energy before adjustment. The resulting gains are stored in
  #nrgGain_m[#MAX_FREQ_COEFFS] (mantissas) and #nrgGain_e[#MAX_FREQ_COEFFS]
  (exponents), the noise levels are stored in #noiseLevel_m[#MAX_FREQ_COEFFS]
  and #noiseLevel_e[#MAX_FREQ_COEFFS] (exponents). The sine levels are stored in
  #nrgSine_m[#MAX_FREQ_COEFFS] and #nrgSine_e[#MAX_FREQ_COEFFS]. \li Noise
  limiting: The gain for each subband is limited both absolutely and relatively
  compared to the total gain over all subbands. \li Boost gain: Calculate and
  apply boost factor for each limiter band in order to compensate for the energy
  loss imposed by the limiting. \li Apply gains and add noise: The gains and
  noise levels are applied to all timeslots of the current envelope. A short
  FIR-filter (length 4 QMF-timeslots) can be used to smooth the sudden change at
  the envelope borders. Each complex subband sample of the current timeslot is
  multiplied by the smoothed gain, then random noise with the calculated level
  is added.

  \note
  To reduce the stack size, some of the local arrays could be located within
  the time output buffer. Of the 512 samples temporarily available there,
  about half the size is already used by #SBR_FRAME_DATA. A pointer to the
  remaining free memory could be supplied by an additional argument to
  calculateSbrEnvelope() in sbr_dec:

  \par
  \code
    calculateSbrEnvelope (&hSbrDec->sbrScaleFactor,
                          &hSbrDec->SbrCalculateEnvelope,
                          hHeaderData,
                          hFrameData,
                          QmfBufferReal,
                          QmfBufferImag,
                          timeOutPtr + sizeof(SBR_FRAME_DATA)/sizeof(Float) +
  1); \endcode

  \par
  Within calculateSbrEnvelope(), some pointers could be defined instead of the
  arrays #nrgRef_m, #nrgRef_e, #nrgEst_m, #nrgEst_e, #noiseLevel_m:

  \par
  \code
    fract*        nrgRef_m = timeOutPtr;
    SCHAR*        nrgRef_e = nrgRef_m + MAX_FREQ_COEFFS;
    fract*        nrgEst_m = nrgRef_e + MAX_FREQ_COEFFS;
    SCHAR*        nrgEst_e = nrgEst_m + MAX_FREQ_COEFFS;
    fract*        noiseLevel_m = nrgEst_e + MAX_FREQ_COEFFS;
  \endcode

  <br>
*/
void calculateSbrEnvelope(
    QMF_SCALE_FACTOR *sbrScaleFactor, /*!< Scaling factors */
    HANDLE_SBR_CALCULATE_ENVELOPE
        h_sbr_cal_env, /*!< Handle to struct filled by the create-function */
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA hFrameData,   /*!< Control data of current frame */
    PVC_DYNAMIC_DATA *pPvcDynamicData,
    FIXP_DBL *
        *analysBufferReal, /*!< Real part of subband samples to be processed */
    FIXP_DBL *
        *analysBufferImag, /*!< Imag part of subband samples to be processed */
    const int useLP,
    FIXP_DBL *degreeAlias, /*!< Estimated aliasing for each QMF channel */
    const UINT flags, const int frameErrorFlag) {
  int c, i, i_stop, j, envNoise = 0;
  UCHAR *borders = hFrameData->frameInfo.borders;
  UCHAR *bordersPvc = hFrameData->frameInfo.pvcBorders;
  int pvc_mode = pPvcDynamicData->pvc_mode;
  int first_start =
      ((pvc_mode > 0) ? bordersPvc[0] : borders[0]) * hHeaderData->timeStep;
  FIXP_SGL *noiseLevels = hFrameData->sbrNoiseFloorLevel;
  HANDLE_FREQ_BAND_DATA hFreq = &hHeaderData->freqBandData;
  UCHAR **pFreqBandTable = hFreq->freqBandTable;
  UCHAR *pFreqBandTableNoise = hFreq->freqBandTableNoise;

  int lowSubband = hFreq->lowSubband;
  int highSubband = hFreq->highSubband;
  int noSubbands = highSubband - lowSubband;

  /* old high subband before headerchange
     we asume no headerchange here        */
  int ov_highSubband = hFreq->highSubband;

  int noNoiseBands = hFreq->nNfb;
  UCHAR *noSubFrameBands = hFreq->nSfb;
  int no_cols = hHeaderData->numberTimeSlots * hHeaderData->timeStep;

  SCHAR sineMapped[MAX_FREQ_COEFFS];
  SCHAR ov_adj_e = SCALE2EXP(sbrScaleFactor->ov_hb_scale);
  SCHAR adj_e = 0;
  SCHAR output_e;
  SCHAR final_e = 0;
  /* inter-TES is active in one or more envelopes of the current SBR frame */
  const int iTES_enable = hFrameData->iTESactive;
  const int iTES_scale_change = (iTES_enable) ? INTER_TES_SF_CHANGE : 0;
  SCHAR maxGainLimit_e = (frameErrorFlag) ? MAX_GAIN_CONCEAL_EXP : MAX_GAIN_EXP;

  UCHAR smooth_length = 0;

  FIXP_SGL *pIenv = hFrameData->iEnvelope;

  C_ALLOC_SCRATCH_START(useAliasReduction, UCHAR, 64)

  /* if values differ we had a headerchange; if old highband is bigger then new
     one we need to patch overlap-highband-scaling for this frame (see use of
     ov_highSubband) as overlap contains higher frequency components which would
     get lost */
  if (hFreq->highSubband < hFreq->ov_highSubband) {
    ov_highSubband = hFreq->ov_highSubband;
  }

  if (pvc_mode > 0) {
    if (hFrameData->frameInfo.bordersNoise[0] > bordersPvc[0]) {
      /* noise envelope of previous frame is trailing into current PVC frame */
      envNoise = -1;
      noiseLevels = h_sbr_cal_env->prevSbrNoiseFloorLevel;
      noNoiseBands = h_sbr_cal_env->prevNNfb;
      noSubFrameBands = h_sbr_cal_env->prevNSfb;
      lowSubband = h_sbr_cal_env->prevLoSubband;
      highSubband = h_sbr_cal_env->prevHiSubband;

      noSubbands = highSubband - lowSubband;
      ov_highSubband = highSubband;
      if (highSubband < h_sbr_cal_env->prev_ov_highSubband) {
        ov_highSubband = h_sbr_cal_env->prev_ov_highSubband;
      }

      pFreqBandTable[0] = h_sbr_cal_env->prevFreqBandTableLo;
      pFreqBandTable[1] = h_sbr_cal_env->prevFreqBandTableHi;
      pFreqBandTableNoise = h_sbr_cal_env->prevFreqBandTableNoise;
    }

    mapSineFlagsPvc(pFreqBandTable[1], noSubFrameBands[1],
                    h_sbr_cal_env->harmFlagsPrev,
                    h_sbr_cal_env->harmFlagsPrevActive, sineMapped,
                    hFrameData->sinusoidal_position,
                    &h_sbr_cal_env->sinusoidal_positionPrev,
                    (borders[0] > bordersPvc[0]) ? 1 : 0);
  } else {
    /*
      Extract sine flags for all QMF bands
    */
    mapSineFlags(pFreqBandTable[1], noSubFrameBands[1],
                 hFrameData->addHarmonics, h_sbr_cal_env->harmFlagsPrev,
                 h_sbr_cal_env->harmFlagsPrevActive,
                 hFrameData->frameInfo.tranEnv, sineMapped);
  }

  /*
    Scan for maximum in bufferd noise levels.
    This is needed in case that we had strong noise in the previous frame
    which is smoothed into the current frame.
    The resulting exponent is used as start value for the maximum search
    in reference energies
  */
  if (!useLP)
    adj_e = h_sbr_cal_env->filtBufferNoise_e -
            getScalefactor(h_sbr_cal_env->filtBufferNoise, noSubbands) +
            (INT)MAX_SFB_NRG_HEADROOM;

  /*
    Scan for maximum reference energy to be able
    to select appropriate values for adj_e and final_e.
  */
  if (pvc_mode > 0) {
    INT maxSfbNrg_e = pPvcDynamicData->predEsg_expMax;

    /* Energy -> magnitude (sqrt halfens exponent) */
    maxSfbNrg_e =
        (maxSfbNrg_e + 1) >> 1; /* +1 to go safe (round to next higher int) */

    /* Some safety margin is needed for 2 reasons:
       - The signal energy is not equally spread over all subband samples in
         a specific sfb of an envelope (Nrg could be too high by a factor of
         envWidth * sfbWidth)
       - Smoothing can smear high gains of the previous envelope into the
       current
    */
    maxSfbNrg_e += (6 + MAX_SFB_NRG_HEADROOM);

    adj_e = maxSfbNrg_e;
    // final_e should not exist for PVC fixfix framing
  } else {
    for (i = 0; i < hFrameData->frameInfo.nEnvelopes; i++) {
      INT maxSfbNrg_e =
          -FRACT_BITS + NRG_EXP_OFFSET; /* start value for maximum search */

      /* Fetch frequency resolution for current envelope: */
      for (j = noSubFrameBands[hFrameData->frameInfo.freqRes[i]]; j != 0; j--) {
        maxSfbNrg_e = fixMax(maxSfbNrg_e, (INT)((LONG)(*pIenv++) & MASK_E));
      }
      maxSfbNrg_e -= NRG_EXP_OFFSET;

      /* Energy -> magnitude (sqrt halfens exponent) */
      maxSfbNrg_e =
          (maxSfbNrg_e + 1) >> 1; /* +1 to go safe (round to next higher int) */

      /* Some safety margin is needed for 2 reasons:
         - The signal energy is not equally spread over all subband samples in
           a specific sfb of an envelope (Nrg could be too high by a factor of
           envWidth * sfbWidth)
         - Smoothing can smear high gains of the previous envelope into the
         current
      */
      maxSfbNrg_e += (6 + MAX_SFB_NRG_HEADROOM);

      if (borders[i] < hHeaderData->numberTimeSlots)
        /* This envelope affects timeslots that belong to the output frame */
        adj_e = fMax(maxSfbNrg_e, adj_e);

      if (borders[i + 1] > hHeaderData->numberTimeSlots)
        /* This envelope affects timeslots after the output frame */
        final_e = fMax(maxSfbNrg_e, final_e);
    }
  }
  /*
    Calculate adjustment factors and apply them for every envelope.
  */
  pIenv = hFrameData->iEnvelope;

  if (pvc_mode > 0) {
    /* iterate over SBR time slots starting with bordersPvc[i] */
    i = bordersPvc[0]; /* usually 0; can be >0 if switching from legacy SBR to
                          PVC */
    i_stop = PVC_NTIMESLOT;
    FDK_ASSERT(bordersPvc[hFrameData->frameInfo.nEnvelopes] == PVC_NTIMESLOT);
  } else {
    /* iterate over SBR envelopes starting with 0 */
    i = 0;
    i_stop = hFrameData->frameInfo.nEnvelopes;
  }
  for (; i < i_stop; i++) {
    int k, noNoiseFlag;
    SCHAR noise_e, input_e = SCALE2EXP(sbrScaleFactor->hb_scale);
    C_ALLOC_SCRATCH_START(pNrgs, ENV_CALC_NRGS, 1);

    /*
      Helper variables.
    */
    int start_pos, stop_pos, freq_res;
    if (pvc_mode > 0) {
      start_pos =
          hHeaderData->timeStep *
          i; /* Start-position in time (subband sample) for current envelope. */
      stop_pos = hHeaderData->timeStep * (i + 1); /* Stop-position in time
                                                     (subband sample) for
                                                     current envelope. */
      freq_res =
          hFrameData->frameInfo
              .freqRes[0]; /* Frequency resolution for current envelope. */
      FDK_ASSERT(
          freq_res ==
          hFrameData->frameInfo.freqRes[hFrameData->frameInfo.nEnvelopes - 1]);
    } else {
      start_pos = hHeaderData->timeStep *
                  borders[i]; /* Start-position in time (subband sample) for
                                 current envelope. */
      stop_pos = hHeaderData->timeStep *
                 borders[i + 1]; /* Stop-position in time (subband sample) for
                                    current envelope. */
      freq_res =
          hFrameData->frameInfo
              .freqRes[i]; /* Frequency resolution for current envelope. */
    }

    /* Always fully initialize the temporary energy table. This prevents
       negative energies and extreme gain factors in cases where the number of
       limiter bands exceeds the number of subbands. The latter can be caused by
       undetected bit errors and is tested by some streams from the
       certification set. */
    FDKmemclear(pNrgs, sizeof(ENV_CALC_NRGS));

    if (pvc_mode > 0) {
      /* get predicted energy values from PVC module */
      expandPredEsg(pPvcDynamicData, i, (int)MAX_FREQ_COEFFS, pNrgs->nrgRef,
                    pNrgs->nrgRef_e);

      if (i == borders[0]) {
        mapSineFlags(pFreqBandTable[1], noSubFrameBands[1],
                     hFrameData->addHarmonics, h_sbr_cal_env->harmFlagsPrev,
                     h_sbr_cal_env->harmFlagsPrevActive,
                     hFrameData->sinusoidal_position, sineMapped);
      }

      if (i >= hFrameData->frameInfo.bordersNoise[envNoise + 1]) {
        if (envNoise >= 0) {
          noiseLevels += noNoiseBands; /* The noise floor data is stored in a
                                          row [noiseFloor1 noiseFloor2...].*/
        } else {
          /* leave trailing noise envelope of past frame */
          noNoiseBands = hFreq->nNfb;
          noSubFrameBands = hFreq->nSfb;
          noiseLevels = hFrameData->sbrNoiseFloorLevel;

          lowSubband = hFreq->lowSubband;
          highSubband = hFreq->highSubband;

          noSubbands = highSubband - lowSubband;
          ov_highSubband = highSubband;
          if (highSubband < hFreq->ov_highSubband) {
            ov_highSubband = hFreq->ov_highSubband;
          }

          pFreqBandTable[0] = hFreq->freqBandTableLo;
          pFreqBandTable[1] = hFreq->freqBandTableHi;
          pFreqBandTableNoise = hFreq->freqBandTableNoise;
        }
        envNoise++;
      }
    } else {
      /* If the start-pos of the current envelope equals the stop pos of the
         current noise envelope, increase the pointer (i.e. choose the next
         noise-floor).*/
      if (borders[i] == hFrameData->frameInfo.bordersNoise[envNoise + 1]) {
        noiseLevels += noNoiseBands; /* The noise floor data is stored in a row
                                        [noiseFloor1 noiseFloor2...].*/
        envNoise++;
      }
    }
    if (i == hFrameData->frameInfo.tranEnv ||
        i == h_sbr_cal_env->prevTranEnv) /* attack */
    {
      noNoiseFlag = 1;
      if (!useLP) smooth_length = 0; /* No smoothing on attacks! */
    } else {
      noNoiseFlag = 0;
      if (!useLP)
        smooth_length = (1 - hHeaderData->bs_data.smoothingLength)
                        << 2; /* can become either 0 or 4 */
    }

    /*
      Energy estimation in transposed highband.
    */
    if (hHeaderData->bs_data.interpolFreq)
      calcNrgPerSubband(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                        lowSubband, highSubband, start_pos, stop_pos, input_e,
                        pNrgs->nrgEst, pNrgs->nrgEst_e);
    else
      calcNrgPerSfb(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                    noSubFrameBands[freq_res], pFreqBandTable[freq_res],
                    start_pos, stop_pos, input_e, pNrgs->nrgEst,
                    pNrgs->nrgEst_e);

    /*
      Calculate subband gains
    */
    {
      UCHAR *table = pFreqBandTable[freq_res];
      UCHAR *pUiNoise =
          &pFreqBandTableNoise[1]; /*! Upper limit of the current noise floor
                                      band. */

      FIXP_SGL *pNoiseLevels = noiseLevels;

      FIXP_DBL tmpNoise =
          FX_SGL2FX_DBL((FIXP_SGL)((LONG)(*pNoiseLevels) & MASK_M));
      SCHAR tmpNoise_e =
          (UCHAR)((LONG)(*pNoiseLevels++) & MASK_E) - NOISE_EXP_OFFSET;

      int cc = 0;
      c = 0;
      if (pvc_mode > 0) {
        for (j = 0; j < noSubFrameBands[freq_res]; j++) {
          UCHAR sinePresentFlag = 0;
          int li = table[j];
          int ui = table[j + 1];

          for (k = li; k < ui; k++) {
            sinePresentFlag |= (i >= sineMapped[cc]);
            cc++;
          }

          for (k = li; k < ui; k++) {
            FIXP_DBL refNrg = pNrgs->nrgRef[k - lowSubband];
            SCHAR refNrg_e = pNrgs->nrgRef_e[k - lowSubband];

            if (k >= *pUiNoise) {
              tmpNoise =
                  FX_SGL2FX_DBL((FIXP_SGL)((LONG)(*pNoiseLevels) & MASK_M));
              tmpNoise_e =
                  (SCHAR)((LONG)(*pNoiseLevels++) & MASK_E) - NOISE_EXP_OFFSET;

              pUiNoise++;
            }

            FDK_ASSERT(k >= lowSubband);

            if (useLP) useAliasReduction[k - lowSubband] = !sinePresentFlag;

            pNrgs->nrgSine[c] = FL2FXCONST_DBL(0.0f);
            pNrgs->nrgSine_e[c] = 0;

            calcSubbandGain(refNrg, refNrg_e, pNrgs, c, tmpNoise, tmpNoise_e,
                            sinePresentFlag, i >= sineMapped[c], noNoiseFlag);

            c++;
          }
        }
      } else {
        for (j = 0; j < noSubFrameBands[freq_res]; j++) {
          FIXP_DBL refNrg = FX_SGL2FX_DBL((FIXP_SGL)((LONG)(*pIenv) & MASK_M));
          SCHAR refNrg_e = (SCHAR)((LONG)(*pIenv) & MASK_E) - NRG_EXP_OFFSET;

          UCHAR sinePresentFlag = 0;
          int li = table[j];
          int ui = table[j + 1];

          for (k = li; k < ui; k++) {
            sinePresentFlag |= (i >= sineMapped[cc]);
            cc++;
          }

          for (k = li; k < ui; k++) {
            if (k >= *pUiNoise) {
              tmpNoise =
                  FX_SGL2FX_DBL((FIXP_SGL)((LONG)(*pNoiseLevels) & MASK_M));
              tmpNoise_e =
                  (SCHAR)((LONG)(*pNoiseLevels++) & MASK_E) - NOISE_EXP_OFFSET;

              pUiNoise++;
            }

            FDK_ASSERT(k >= lowSubband);

            if (useLP) useAliasReduction[k - lowSubband] = !sinePresentFlag;

            pNrgs->nrgSine[c] = FL2FXCONST_DBL(0.0f);
            pNrgs->nrgSine_e[c] = 0;

            calcSubbandGain(refNrg, refNrg_e, pNrgs, c, tmpNoise, tmpNoise_e,
                            sinePresentFlag, i >= sineMapped[c], noNoiseFlag);

            pNrgs->nrgRef[c] = refNrg;
            pNrgs->nrgRef_e[c] = refNrg_e;

            c++;
          }
          pIenv++;
        }
      }
    }

    /*
      Noise limiting
    */

    for (c = 0; c < hFreq->noLimiterBands; c++) {
      FIXP_DBL sumRef, boostGain, maxGain;
      FIXP_DBL accu = FL2FXCONST_DBL(0.0f);
      SCHAR sumRef_e, boostGain_e, maxGain_e, accu_e = 0;
      int maxGainLimGainSum_e = 0;

      calcAvgGain(pNrgs, hFreq->limiterBandTable[c],
                  hFreq->limiterBandTable[c + 1], &sumRef, &sumRef_e, &maxGain,
                  &maxGain_e);

      /* Multiply maxGain with limiterGain: */
      maxGain = fMult(
          maxGain,
          FDK_sbrDecoder_sbr_limGains_m[hHeaderData->bs_data.limiterGains]);
      /* maxGain_e +=
       * FDK_sbrDecoder_sbr_limGains_e[hHeaderData->bs_data.limiterGains]; */
      /* The addition of maxGain_e and FDK_sbrDecoder_sbr_limGains_e[3] might
         yield values greater than 127 which doesn't fit into an SCHAR! In these
         rare situations limit maxGain_e to 127.
      */
      maxGainLimGainSum_e =
          maxGain_e +
          FDK_sbrDecoder_sbr_limGains_e[hHeaderData->bs_data.limiterGains];
      maxGain_e =
          (maxGainLimGainSum_e > 127) ? (SCHAR)127 : (SCHAR)maxGainLimGainSum_e;

      /* Scale mantissa of MaxGain into range between 0.5 and 1: */
      if (maxGain == FL2FXCONST_DBL(0.0f))
        maxGain_e = -FRACT_BITS;
      else {
        SCHAR charTemp = CountLeadingBits(maxGain);
        maxGain_e -= charTemp;
        maxGain <<= (int)charTemp;
      }

      if (maxGain_e >= maxGainLimit_e) { /* upper limit (e.g. 96 dB) */
        maxGain = FL2FXCONST_DBL(0.5f);
        maxGain_e = maxGainLimit_e;
      }

      /* Every subband gain is compared to the scaled "average gain"
         and limited if necessary: */
      for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c + 1];
           k++) {
        if ((pNrgs->nrgGain_e[k] > maxGain_e) ||
            (pNrgs->nrgGain_e[k] == maxGain_e && pNrgs->nrgGain[k] > maxGain)) {
          FIXP_DBL noiseAmp;
          SCHAR noiseAmp_e;

          FDK_divide_MantExp(maxGain, maxGain_e, pNrgs->nrgGain[k],
                             pNrgs->nrgGain_e[k], &noiseAmp, &noiseAmp_e);
          pNrgs->noiseLevel[k] = fMult(pNrgs->noiseLevel[k], noiseAmp);
          pNrgs->noiseLevel_e[k] += noiseAmp_e;
          pNrgs->nrgGain[k] = maxGain;
          pNrgs->nrgGain_e[k] = maxGain_e;
        }
      }

      /* -- Boost gain
        Calculate and apply boost factor for each limiter band:
        1. Check how much energy would be present when using the limited gain
        2. Calculate boost factor by comparison with reference energy
        3. Apply boost factor to compensate for the energy loss due to limiting
      */
      for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c + 1];
           k++) {
        /* 1.a  Add energy of adjusted signal (using preliminary gain) */
        FIXP_DBL tmp = fMult(pNrgs->nrgGain[k], pNrgs->nrgEst[k]);
        SCHAR tmp_e = pNrgs->nrgGain_e[k] + pNrgs->nrgEst_e[k];
        FDK_add_MantExp(tmp, tmp_e, accu, accu_e, &accu, &accu_e);

        /* 1.b  Add sine energy (if present) */
        if (pNrgs->nrgSine[k] != FL2FXCONST_DBL(0.0f)) {
          FDK_add_MantExp(pNrgs->nrgSine[k], pNrgs->nrgSine_e[k], accu, accu_e,
                          &accu, &accu_e);
        } else {
          /* 1.c  Add noise energy (if present) */
          if (noNoiseFlag == 0) {
            FDK_add_MantExp(pNrgs->noiseLevel[k], pNrgs->noiseLevel_e[k], accu,
                            accu_e, &accu, &accu_e);
          }
        }
      }

      /* 2.a  Calculate ratio of wanted energy and accumulated energy */
      if (accu == (FIXP_DBL)0) { /* If divisor is 0, limit quotient to +4 dB */
        boostGain = FL2FXCONST_DBL(0.6279716f);
        boostGain_e = 2;
      } else {
        INT div_e;
        boostGain = fDivNorm(sumRef, accu, &div_e);
        boostGain_e = sumRef_e - accu_e + div_e;
      }

      /* 2.b Result too high? --> Limit the boost factor to +4 dB */
      if ((boostGain_e > 3) ||
          (boostGain_e == 2 && boostGain > FL2FXCONST_DBL(0.6279716f)) ||
          (boostGain_e == 3 && boostGain > FL2FXCONST_DBL(0.3139858f))) {
        boostGain = FL2FXCONST_DBL(0.6279716f);
        boostGain_e = 2;
      }
      /* 3.  Multiply all signal components with the boost factor */
      for (k = hFreq->limiterBandTable[c]; k < hFreq->limiterBandTable[c + 1];
           k++) {
        pNrgs->nrgGain[k] = fMultDiv2(pNrgs->nrgGain[k], boostGain);
        pNrgs->nrgGain_e[k] = pNrgs->nrgGain_e[k] + boostGain_e + 1;

        pNrgs->nrgSine[k] = fMultDiv2(pNrgs->nrgSine[k], boostGain);
        pNrgs->nrgSine_e[k] = pNrgs->nrgSine_e[k] + boostGain_e + 1;

        pNrgs->noiseLevel[k] = fMultDiv2(pNrgs->noiseLevel[k], boostGain);
        pNrgs->noiseLevel_e[k] = pNrgs->noiseLevel_e[k] + boostGain_e + 1;
      }
    }
    /* End of noise limiting */

    if (useLP)
      aliasingReduction(degreeAlias + lowSubband, pNrgs, useAliasReduction,
                        noSubbands);

    /* For the timeslots within the range for the output frame,
       use the same scale for the noise levels.
       Drawback: If the envelope exceeds the frame border, the noise levels
                 will have to be rescaled later to fit final_e of
                 the gain-values.
    */
    noise_e = (start_pos < no_cols) ? adj_e : final_e;

    if (start_pos >= no_cols) {
      int diff = h_sbr_cal_env->filtBufferNoise_e - noise_e;
      if (diff > 0) {
        int s = getScalefactor(h_sbr_cal_env->filtBufferNoise, noSubbands);
        if (diff > s) {
          final_e += diff - s;
          noise_e = final_e;
        }
      }
    }

    /*
      Convert energies to amplitude levels
    */
    for (k = 0; k < noSubbands; k++) {
      FDK_sqrt_MantExp(&pNrgs->nrgSine[k], &pNrgs->nrgSine_e[k], &noise_e);
      FDK_sqrt_MantExp(&pNrgs->nrgGain[k], &pNrgs->nrgGain_e[k],
                       &pNrgs->nrgGain_e[k]);
      FDK_sqrt_MantExp(&pNrgs->noiseLevel[k], &pNrgs->noiseLevel_e[k],
                       &noise_e);
    }

    /*
      Apply calculated gains and adaptive noise
    */

    /* assembleHfSignals() */
    {
      int scale_change, sc_change;
      FIXP_SGL smooth_ratio;
      int filtBufferNoiseShift = 0;

      /* Initialize smoothing buffers with the first valid values */
      if (h_sbr_cal_env->startUp) {
        if (!useLP) {
          h_sbr_cal_env->filtBufferNoise_e = noise_e;

          FDKmemcpy(h_sbr_cal_env->filtBuffer_e, pNrgs->nrgGain_e,
                    noSubbands * sizeof(SCHAR));
          FDKmemcpy(h_sbr_cal_env->filtBufferNoise, pNrgs->noiseLevel,
                    noSubbands * sizeof(FIXP_DBL));
          FDKmemcpy(h_sbr_cal_env->filtBuffer, pNrgs->nrgGain,
                    noSubbands * sizeof(FIXP_DBL));
        }
        h_sbr_cal_env->startUp = 0;
      }

      if (!useLP) {
        equalizeFiltBufferExp(h_sbr_cal_env->filtBuffer,   /* buffered */
                              h_sbr_cal_env->filtBuffer_e, /* buffered */
                              pNrgs->nrgGain,              /* current  */
                              pNrgs->nrgGain_e,            /* current  */
                              noSubbands);

        /* Adapt exponent of buffered noise levels to the current exponent
           so they can easily be smoothed */
        if ((h_sbr_cal_env->filtBufferNoise_e - noise_e) >= 0) {
          int shift = fixMin(DFRACT_BITS - 1,
                             (int)(h_sbr_cal_env->filtBufferNoise_e - noise_e));
          for (k = 0; k < noSubbands; k++)
            h_sbr_cal_env->filtBufferNoise[k] <<= shift;
        } else {
          int shift =
              fixMin(DFRACT_BITS - 1,
                     -(int)(h_sbr_cal_env->filtBufferNoise_e - noise_e));
          for (k = 0; k < noSubbands; k++)
            h_sbr_cal_env->filtBufferNoise[k] >>= shift;
        }

        h_sbr_cal_env->filtBufferNoise_e = noise_e;
      }

      /* find best scaling! */
      scale_change = -(DFRACT_BITS - 1);
      for (k = 0; k < noSubbands; k++) {
        scale_change = fixMax(scale_change, (int)pNrgs->nrgGain_e[k]);
      }
      sc_change = (start_pos < no_cols) ? adj_e - input_e : final_e - input_e;

      if ((scale_change - sc_change + 1) < 0)
        scale_change -= (scale_change - sc_change + 1);

      scale_change = (scale_change - sc_change) + 1;

      for (k = 0; k < noSubbands; k++) {
        int sc = scale_change - pNrgs->nrgGain_e[k] + (sc_change - 1);
        pNrgs->nrgGain[k] >>= fixMin(sc, DFRACT_BITS - 1);
        pNrgs->nrgGain_e[k] += sc;
      }

      if (!useLP) {
        for (k = 0; k < noSubbands; k++) {
          int sc =
              scale_change - h_sbr_cal_env->filtBuffer_e[k] + (sc_change - 1);
          h_sbr_cal_env->filtBuffer[k] >>= fixMin(sc, DFRACT_BITS - 1);
        }
      }

      for (j = start_pos; j < stop_pos; j++) {
        /* This timeslot is located within the first part of the processing
           buffer and will be fed into the QMF-synthesis for the current frame.
               adj_e - input_e
           This timeslot will not yet be fed into the QMF so we do not care
           about the adj_e.
               sc_change = final_e - input_e
        */
        if ((j == no_cols) && (start_pos < no_cols)) {
          int shift = (int)(noise_e - final_e);
          if (!useLP)
            filtBufferNoiseShift = shift; /* shifting of
                                             h_sbr_cal_env->filtBufferNoise[k]
                                             will be applied in function
                                             adjustTimeSlotHQ() */
          if (shift >= 0) {
            shift = fixMin(DFRACT_BITS - 1, shift);
            for (k = 0; k < noSubbands; k++) {
              pNrgs->nrgSine[k] <<= shift;
              pNrgs->noiseLevel[k] <<= shift;
              /*
              if (!useLP)
                h_sbr_cal_env->filtBufferNoise[k]  <<= shift;
              */
            }
          } else {
            shift = fixMin(DFRACT_BITS - 1, -shift);
            for (k = 0; k < noSubbands; k++) {
              pNrgs->nrgSine[k] >>= shift;
              pNrgs->noiseLevel[k] >>= shift;
              /*
              if (!useLP)
                h_sbr_cal_env->filtBufferNoise[k]  >>= shift;
              */
            }
          }

          /* update noise scaling */
          noise_e = final_e;
          if (!useLP)
            h_sbr_cal_env->filtBufferNoise_e =
                noise_e; /* scaling value unused! */

          /* update gain buffer*/
          sc_change -= (final_e - input_e);

          if (sc_change < 0) {
            for (k = 0; k < noSubbands; k++) {
              pNrgs->nrgGain[k] >>= -sc_change;
              pNrgs->nrgGain_e[k] += -sc_change;
            }
            if (!useLP) {
              for (k = 0; k < noSubbands; k++) {
                h_sbr_cal_env->filtBuffer[k] >>= -sc_change;
              }
            }
          } else {
            scale_change += sc_change;
          }

        } /* if */

        if (!useLP) {
          /* Prevent the smoothing filter from running on constant levels */
          if (j - start_pos < smooth_length)
            smooth_ratio = FDK_sbrDecoder_sbr_smoothFilter[j - start_pos];
          else
            smooth_ratio = FL2FXCONST_SGL(0.0f);

          if (iTES_enable) {
            /* adjustTimeSlotHQ() without adding of additional harmonics */
            adjustTimeSlotHQ_GainAndNoise(
                &analysBufferReal[j][lowSubband],
                &analysBufferImag[j][lowSubband], h_sbr_cal_env, pNrgs,
                lowSubband, noSubbands, fMin(scale_change, DFRACT_BITS - 1),
                smooth_ratio, noNoiseFlag, filtBufferNoiseShift);
          } else {
            adjustTimeSlotHQ(&analysBufferReal[j][lowSubband],
                             &analysBufferImag[j][lowSubband], h_sbr_cal_env,
                             pNrgs, lowSubband, noSubbands,
                             fMin(scale_change, DFRACT_BITS - 1), smooth_ratio,
                             noNoiseFlag, filtBufferNoiseShift);
          }
        } else {
          FDK_ASSERT(!iTES_enable); /* not supported */
          if (flags & SBRDEC_ELD_GRID) {
            /* FDKmemset(analysBufferReal[j], 0, 64 * sizeof(FIXP_DBL)); */
            adjustTimeSlot_EldGrid(
                &analysBufferReal[j][lowSubband], pNrgs,
                &h_sbr_cal_env->harmIndex, lowSubband, noSubbands,
                fMin(scale_change, DFRACT_BITS - 1), noNoiseFlag,
                &h_sbr_cal_env->phaseIndex,
                fMax(EXP2SCALE(adj_e) - sbrScaleFactor->lb_scale,
                     -(DFRACT_BITS - 1)));
          } else {
            adjustTimeSlotLC(&analysBufferReal[j][lowSubband], pNrgs,
                             &h_sbr_cal_env->harmIndex, lowSubband, noSubbands,
                             fMin(scale_change, DFRACT_BITS - 1), noNoiseFlag,
                             &h_sbr_cal_env->phaseIndex);
          }
        }
        /* In case the envelope spans accross the no_cols border both exponents
         * are needed. */
        /* nrgGain_e[0...(noSubbands-1)] are equalized by
         * equalizeFiltBufferExp() */
        pNrgs->exponent[(j < no_cols) ? 0 : 1] =
            (SCHAR)((15 - sbrScaleFactor->hb_scale) + pNrgs->nrgGain_e[0] + 1 -
                    scale_change);
      } /* for */

      if (iTES_enable) {
        apply_inter_tes(
            analysBufferReal, /* pABufR, */
            analysBufferImag, /* pABufI, */
            sbrScaleFactor, pNrgs->exponent, hHeaderData->timeStep, start_pos,
            stop_pos, lowSubband, noSubbands,
            hFrameData
                ->interTempShapeMode[i] /* frameData->interTempShapeMode[env] */
        );

        /* add additional harmonics */
        for (j = start_pos; j < stop_pos; j++) {
          /* match exponent of additional harmonics to scale change of QMF data
           * caused by apply_inter_tes() */
          scale_change = 0;

          if ((start_pos <= no_cols) && (stop_pos > no_cols)) {
            /* Scaling of analysBuffers was potentially changed within this
               envelope. The pNrgs->nrgSine_e match the second part of the
               envelope. For (j<=no_cols) the exponent of the sine energies has
               to be adapted. */
            scale_change = pNrgs->exponent[1] - pNrgs->exponent[0];
          }

          adjustTimeSlotHQ_AddHarmonics(
              &analysBufferReal[j][lowSubband],
              &analysBufferImag[j][lowSubband], h_sbr_cal_env, pNrgs,
              lowSubband, noSubbands,
              -iTES_scale_change + ((j < no_cols) ? scale_change : 0));
        }
      }

      if (!useLP) {
        /* Update time-smoothing-buffers for gains and noise levels
           The gains and the noise values of the current envelope are copied
           into the buffer. This has to be done at the end of each envelope as
           the values are required for a smooth transition to the next envelope.
         */
        FDKmemcpy(h_sbr_cal_env->filtBuffer, pNrgs->nrgGain,
                  noSubbands * sizeof(FIXP_DBL));
        FDKmemcpy(h_sbr_cal_env->filtBuffer_e, pNrgs->nrgGain_e,
                  noSubbands * sizeof(SCHAR));
        FDKmemcpy(h_sbr_cal_env->filtBufferNoise, pNrgs->noiseLevel,
                  noSubbands * sizeof(FIXP_DBL));
      }
    }
    C_ALLOC_SCRATCH_END(pNrgs, ENV_CALC_NRGS, 1);
  }

  /* adapt adj_e to the scale change caused by apply_inter_tes() */
  adj_e += iTES_scale_change;

  /* Rescale output samples */
  {
    FIXP_DBL maxVal;
    int ov_reserve, reserve;

    /* Determine headroom in old adjusted samples */
    maxVal =
        maxSubbandSample(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                         lowSubband, ov_highSubband, 0, first_start);

    ov_reserve = fNorm(maxVal);

    /* Determine headroom in new adjusted samples */
    maxVal =
        maxSubbandSample(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                         lowSubband, highSubband, first_start, no_cols);

    reserve = fNorm(maxVal);

    /* Determine common output exponent */
    output_e = fMax(ov_adj_e - ov_reserve, adj_e - reserve);

    /* Rescale old samples */
    rescaleSubbandSamples(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                          lowSubband, ov_highSubband, 0, first_start,
                          ov_adj_e - output_e);

    /* Rescale new samples */
    rescaleSubbandSamples(analysBufferReal, (useLP) ? NULL : analysBufferImag,
                          lowSubband, highSubband, first_start, no_cols,
                          adj_e - output_e);
  }

  /* Update hb_scale */
  sbrScaleFactor->hb_scale = EXP2SCALE(output_e);

  /* Save the current final exponent for the next frame: */
  /* adapt final_e to the scale change caused by apply_inter_tes() */
  sbrScaleFactor->ov_hb_scale = EXP2SCALE(final_e + iTES_scale_change);

  /* We need to remember to the next frame that the transient
     will occur in the first envelope (if tranEnv == nEnvelopes). */
  if (hFrameData->frameInfo.tranEnv == hFrameData->frameInfo.nEnvelopes)
    h_sbr_cal_env->prevTranEnv = 0;
  else
    h_sbr_cal_env->prevTranEnv = -1;

  if (pvc_mode > 0) {
    /* Not more than just the last noise envelope reaches into the next PVC
       frame! This should be true because bs_noise_position is <= 15 */
    FDK_ASSERT(hFrameData->frameInfo
                   .bordersNoise[hFrameData->frameInfo.nNoiseEnvelopes - 1] <
               PVC_NTIMESLOT);
    if (hFrameData->frameInfo
            .bordersNoise[hFrameData->frameInfo.nNoiseEnvelopes] >
        PVC_NTIMESLOT) {
      FDK_ASSERT(noiseLevels ==
                 (hFrameData->sbrNoiseFloorLevel +
                  (hFrameData->frameInfo.nNoiseEnvelopes - 1) * noNoiseBands));
      h_sbr_cal_env->prevNNfb = noNoiseBands;

      h_sbr_cal_env->prevNSfb[0] = noSubFrameBands[0];
      h_sbr_cal_env->prevNSfb[1] = noSubFrameBands[1];

      h_sbr_cal_env->prevLoSubband = lowSubband;
      h_sbr_cal_env->prevHiSubband = highSubband;
      h_sbr_cal_env->prev_ov_highSubband = ov_highSubband;

      FDKmemcpy(h_sbr_cal_env->prevFreqBandTableLo, pFreqBandTable[0],
                noSubFrameBands[0] + 1);
      FDKmemcpy(h_sbr_cal_env->prevFreqBandTableHi, pFreqBandTable[1],
                noSubFrameBands[1] + 1);
      FDKmemcpy(h_sbr_cal_env->prevFreqBandTableNoise,
                hFreq->freqBandTableNoise, sizeof(hFreq->freqBandTableNoise));

      FDKmemcpy(h_sbr_cal_env->prevSbrNoiseFloorLevel, noiseLevels,
                MAX_NOISE_COEFFS * sizeof(FIXP_SGL));
    }
  }

  C_ALLOC_SCRATCH_END(useAliasReduction, UCHAR, 64)
}

/*!
  \brief   Create envelope instance

  Must be called once for each channel before calculateSbrEnvelope() can be
  used.

  \return  errorCode, 0 if successful
*/
SBR_ERROR
createSbrEnvelopeCalc(
    HANDLE_SBR_CALCULATE_ENVELOPE hs, /*!< pointer to envelope instance */
    HANDLE_SBR_HEADER_DATA
        hHeaderData, /*!< static SBR control data, initialized with defaults */
    const int chan,  /*!< Channel for which to assign buffers */
    const UINT flags) {
  SBR_ERROR err = SBRDEC_OK;
  int i;

  /* Clear previous missing harmonics flags */
  for (i = 0; i < ADD_HARMONICS_FLAGS_SIZE; i++) {
    hs->harmFlagsPrev[i] = 0;
    hs->harmFlagsPrevActive[i] = 0;
  }
  hs->harmIndex = 0;

  FDKmemclear(hs->prevSbrNoiseFloorLevel, sizeof(hs->prevSbrNoiseFloorLevel));
  hs->prevNNfb = 0;
  FDKmemclear(hs->prevFreqBandTableNoise, sizeof(hs->prevFreqBandTableNoise));
  hs->sinusoidal_positionPrev = 0;

  /*
    Setup pointers for time smoothing.
    The buffer itself will be initialized later triggered by the startUp-flag.
  */
  hs->prevTranEnv = -1;

  /* initialization */
  resetSbrEnvelopeCalc(hs);

  if (chan == 0) { /* do this only once */
    err = resetFreqBandTables(hHeaderData, flags);
  }

  return err;
}

/*!
  \brief   Create envelope instance

  Must be called once for each channel before calculateSbrEnvelope() can be
  used.

  \return  errorCode, 0 if successful
*/
int deleteSbrEnvelopeCalc(HANDLE_SBR_CALCULATE_ENVELOPE hs) { return 0; }

/*!
  \brief   Reset envelope instance

  This function must be called for each channel on a change of configuration.
  Note that resetFreqBandTables should also be called in this case.

  \return  errorCode, 0 if successful
*/
void resetSbrEnvelopeCalc(
    HANDLE_SBR_CALCULATE_ENVELOPE hCalEnv) /*!< pointer to envelope instance */
{
  hCalEnv->phaseIndex = 0;

  /* Noise exponent needs to be reset because the output exponent for the next
   * frame depends on it */
  hCalEnv->filtBufferNoise_e = 0;

  hCalEnv->startUp = 1;
}

/*!
  \brief  Equalize exponents of the buffered gain values and the new ones

  After equalization of exponents, the FIR-filter addition for smoothing
  can be performed.
  This function is called once for each envelope before adjusting.
*/
static void equalizeFiltBufferExp(
    FIXP_DBL *filtBuffer, /*!< bufferd gains */
    SCHAR *filtBuffer_e,  /*!< exponents of bufferd gains */
    FIXP_DBL *nrgGain,    /*!< gains for current envelope */
    SCHAR *nrgGain_e,     /*!< exponents of gains for current envelope */
    int subbands)         /*!< Number of QMF subbands */
{
  int band;
  int diff;

  for (band = 0; band < subbands; band++) {
    diff = (int)(nrgGain_e[band] - filtBuffer_e[band]);
    if (diff > 0) {
      filtBuffer[band] >>=
          fMin(diff, DFRACT_BITS - 1); /* Compensate for the scale change by
                                          shifting the mantissa. */
      filtBuffer_e[band] += diff; /* New gain is bigger, use its exponent */
    } else if (diff < 0) {
      /* The buffered gains seem to be larger, but maybe there
         are some unused bits left in the mantissa */

      int reserve = CntLeadingZeros(fixp_abs(filtBuffer[band])) - 1;

      if ((-diff) <= reserve) {
        /* There is enough space in the buffered mantissa so
           that we can take the new exponent as common.
        */
        filtBuffer[band] <<= (-diff);
        filtBuffer_e[band] += diff; /* becomes equal to *ptrNewExp */
      } else {
        filtBuffer[band] <<=
            reserve; /* Shift the mantissa as far as possible: */
        filtBuffer_e[band] -= reserve; /* Compensate in the exponent: */

        /* For the remaining difference, change the new gain value */
        diff = -(reserve + diff);
        nrgGain[band] >>= fMin(diff, DFRACT_BITS - 1);
        nrgGain_e[band] += diff;
      }
    }
  }
}

/*!
  \brief  Shift left the mantissas of all subband samples
          in the giventime and frequency range by the specified number of bits.

  This function is used to rescale the audio data in the overlap buffer
  which has already been envelope adjusted with the last frame.
*/
void rescaleSubbandSamples(
    FIXP_DBL **re,   /*!< Real part of input and output subband samples */
    FIXP_DBL **im,   /*!< Imaginary part of input and output subband samples */
    int lowSubband,  /*!< Begin of frequency range to process */
    int highSubband, /*!< End of frequency range to process */
    int start_pos,   /*!< Begin of time rage (QMF-timeslot) */
    int next_pos,    /*!< End of time rage (QMF-timeslot) */
    int shift)       /*!< number of bits to shift */
{
  int width = highSubband - lowSubband;

  if ((width > 0) && (shift != 0)) {
    if (im != NULL) {
      for (int l = start_pos; l < next_pos; l++) {
        scaleValues(&re[l][lowSubband], width, shift);
        scaleValues(&im[l][lowSubband], width, shift);
      }
    } else {
      for (int l = start_pos; l < next_pos; l++) {
        scaleValues(&re[l][lowSubband], width, shift);
      }
    }
  }
}

static inline FIXP_DBL FDK_get_maxval_real(FIXP_DBL maxVal, FIXP_DBL *reTmp,
                                           INT width) {
  maxVal = (FIXP_DBL)0;
  while (width-- != 0) {
    FIXP_DBL tmp = *(reTmp++);
    maxVal |= (FIXP_DBL)((LONG)(tmp) ^ ((LONG)tmp >> (DFRACT_BITS - 1)));
  }

  return maxVal;
}

/*!
  \brief   Determine headroom for shifting

  Determine by how much the spectrum can be shifted left
  for better accuracy in later processing.

  \return  Number of free bits in the biggest spectral value
*/

FIXP_DBL maxSubbandSample(
    FIXP_DBL **re,   /*!< Real part of input and output subband samples */
    FIXP_DBL **im,   /*!< Real part of input and output subband samples */
    int lowSubband,  /*!< Begin of frequency range to process */
    int highSubband, /*!< Number of QMF bands to process */
    int start_pos,   /*!< Begin of time rage (QMF-timeslot) */
    int next_pos     /*!< End of time rage (QMF-timeslot) */
) {
  FIXP_DBL maxVal = FL2FX_DBL(0.0f);
  unsigned int width = highSubband - lowSubband;

  FDK_ASSERT(width <= (64));

  if (width > 0) {
    if (im != NULL) {
      for (int l = start_pos; l < next_pos; l++) {
        int k = width;
        FIXP_DBL *reTmp = &re[l][lowSubband];
        FIXP_DBL *imTmp = &im[l][lowSubband];
        do {
          FIXP_DBL tmp1 = *(reTmp++);
          FIXP_DBL tmp2 = *(imTmp++);
          maxVal |=
              (FIXP_DBL)((LONG)(tmp1) ^ ((LONG)tmp1 >> (DFRACT_BITS - 1)));
          maxVal |=
              (FIXP_DBL)((LONG)(tmp2) ^ ((LONG)tmp2 >> (DFRACT_BITS - 1)));
        } while (--k != 0);
      }
    } else {
      for (int l = start_pos; l < next_pos; l++) {
        maxVal |= FDK_get_maxval_real(maxVal, &re[l][lowSubband], width);
      }
    }
  }

  if (maxVal > (FIXP_DBL)0) {
    /* For negative input values, maxVal is too small by 1. Add 1 only when
     * necessary: if maxVal is a power of 2 */
    FIXP_DBL lowerPow2 =
        (FIXP_DBL)(1 << (DFRACT_BITS - 1 - CntLeadingZeros(maxVal)));
    if (maxVal == lowerPow2) maxVal += (FIXP_DBL)1;
  }

  return (maxVal);
}

/* #define SHIFT_BEFORE_SQUARE (3) */ /* (7/2) */
/* Avoid assertion failures triggerd by overflows which occured in robustness
   tests. Setting the SHIFT_BEFORE_SQUARE to 4 has negligible effect on (USAC)
   conformance results. */
#define SHIFT_BEFORE_SQUARE (4) /* ((8 - 0) / 2) */

/*!<
  If the accumulator does not provide enough overflow bits or
  does not provide a high dynamic range, the below energy calculation
  requires an additional shift operation for each sample.
  On the other hand, doing the shift allows using a single-precision
  multiplication for the square (at least 16bit x 16bit).
  For even values of OVRFLW_BITS (0, 2, 4, 6), saturated arithmetic
  is required for the energy accumulation.
  Theoretically, the sample-squares can sum up to a value of 76,
  requiring 7 overflow bits. However since such situations are *very*
  rare, accu can be limited to 64.
  In case native saturated arithmetic is not available, overflows
  can be prevented by replacing the above #define by
    #define SHIFT_BEFORE_SQUARE ((8 - OVRFLW_BITS) / 2)
  which will result in slightly reduced accuracy.
*/

/*!
  \brief  Estimates the mean energy of each filter-bank channel for the
          duration of the current envelope

  This function is used when interpolFreq is true.
*/
static void calcNrgPerSubband(
    FIXP_DBL **analysBufferReal, /*!< Real part of subband samples */
    FIXP_DBL **analysBufferImag, /*!< Imaginary part of subband samples */
    int lowSubband,              /*!< Begin of the SBR frequency range */
    int highSubband,             /*!< High end of the SBR frequency range */
    int start_pos,               /*!< First QMF-slot of current envelope */
    int next_pos,                /*!< Last QMF-slot of current envelope + 1 */
    SCHAR frameExp,              /*!< Common exponent for all input samples */
    FIXP_DBL *nrgEst,            /*!< resulting Energy (0..1) */
    SCHAR *nrgEst_e)             /*!< Exponent of resulting Energy */
{
  FIXP_SGL invWidth;
  SCHAR preShift;
  SCHAR shift;
  FIXP_DBL sum;
  int k;

  /* Divide by width of envelope later: */
  invWidth = FX_DBL2FX_SGL(GetInvInt(next_pos - start_pos));
  /* The common exponent needs to be doubled because all mantissas are squared:
   */
  frameExp = frameExp << 1;

  for (k = lowSubband; k < highSubband; k++) {
    FIXP_DBL bufferReal[(((1024) / (32) * (4) / 2) + (3 * (4)))];
    FIXP_DBL bufferImag[(((1024) / (32) * (4) / 2) + (3 * (4)))];
    FIXP_DBL maxVal;

    if (analysBufferImag != NULL) {
      int l;
      maxVal = FL2FX_DBL(0.0f);
      for (l = start_pos; l < next_pos; l++) {
        bufferImag[l] = analysBufferImag[l][k];
        maxVal |= (FIXP_DBL)((LONG)(bufferImag[l]) ^
                             ((LONG)bufferImag[l] >> (DFRACT_BITS - 1)));
        bufferReal[l] = analysBufferReal[l][k];
        maxVal |= (FIXP_DBL)((LONG)(bufferReal[l]) ^
                             ((LONG)bufferReal[l] >> (DFRACT_BITS - 1)));
      }
    } else {
      int l;
      maxVal = FL2FX_DBL(0.0f);
      for (l = start_pos; l < next_pos; l++) {
        bufferReal[l] = analysBufferReal[l][k];
        maxVal |= (FIXP_DBL)((LONG)(bufferReal[l]) ^
                             ((LONG)bufferReal[l] >> (DFRACT_BITS - 1)));
      }
    }

    if (maxVal != FL2FXCONST_DBL(0.f)) {
      /* If the accu does not provide enough overflow bits, we cannot
         shift the samples up to the limit.
         Instead, keep up to 3 free bits in each sample, i.e. up to
         6 bits after calculation of square.
         Please note the comment on saturated arithmetic above!
      */
      FIXP_DBL accu;
      preShift = CntLeadingZeros(maxVal) - 1;
      preShift -= SHIFT_BEFORE_SQUARE;

      /* Limit preShift to a maximum value to prevent accumulator overflow in
         exceptional situations where the signal in the analysis-buffer is very
         small (small maxVal).
      */
      preShift = fMin(preShift, (SCHAR)25);

      accu = FL2FXCONST_DBL(0.0f);
      if (preShift >= 0) {
        int l;
        if (analysBufferImag != NULL) {
          for (l = start_pos; l < next_pos; l++) {
            FIXP_DBL temp1 = bufferReal[l] << (int)preShift;
            FIXP_DBL temp2 = bufferImag[l] << (int)preShift;
            accu = fPow2AddDiv2(accu, temp1);
            accu = fPow2AddDiv2(accu, temp2);
          }
        } else {
          for (l = start_pos; l < next_pos; l++) {
            FIXP_DBL temp = bufferReal[l] << (int)preShift;
            accu = fPow2AddDiv2(accu, temp);
          }
        }
      } else { /* if negative shift value */
        int l;
        int negpreShift = -preShift;
        if (analysBufferImag != NULL) {
          for (l = start_pos; l < next_pos; l++) {
            FIXP_DBL temp1 = bufferReal[l] >> (int)negpreShift;
            FIXP_DBL temp2 = bufferImag[l] >> (int)negpreShift;
            accu = fPow2AddDiv2(accu, temp1);
            accu = fPow2AddDiv2(accu, temp2);
          }
        } else {
          for (l = start_pos; l < next_pos; l++) {
            FIXP_DBL temp = bufferReal[l] >> (int)negpreShift;
            accu = fPow2AddDiv2(accu, temp);
          }
        }
      }
      accu <<= 1;

      /* Convert double precision to Mantissa/Exponent: */
      shift = fNorm(accu);
      sum = accu << (int)shift;

      /* Divide by width of envelope and apply frame scale: */
      *nrgEst++ = fMult(sum, invWidth);
      shift += 2 * preShift;
      if (analysBufferImag != NULL)
        *nrgEst_e++ = frameExp - shift;
      else
        *nrgEst_e++ = frameExp - shift + 1; /* +1 due to missing imag. part */
    }                                       /* maxVal!=0 */
    else {
      /* Prevent a zero-mantissa-number from being misinterpreted
         due to its exponent. */
      *nrgEst++ = FL2FXCONST_DBL(0.0f);
      *nrgEst_e++ = 0;
    }
  }
}

/*!
  \brief   Estimates the mean energy of each Scale factor band for the
           duration of the current envelope.

  This function is used when interpolFreq is false.
*/
static void calcNrgPerSfb(
    FIXP_DBL **analysBufferReal, /*!< Real part of subband samples */
    FIXP_DBL **analysBufferImag, /*!< Imaginary part of subband samples */
    int nSfb,                    /*!< Number of scale factor bands */
    UCHAR *freqBandTable,        /*!< First Subband for each Sfb */
    int start_pos,               /*!< First QMF-slot of current envelope */
    int next_pos,                /*!< Last QMF-slot of current envelope + 1 */
    SCHAR input_e,               /*!< Common exponent for all input samples */
    FIXP_DBL *nrgEst,            /*!< resulting Energy (0..1) */
    SCHAR *nrgEst_e)             /*!< Exponent of resulting Energy */
{
  FIXP_SGL invWidth;
  FIXP_DBL temp;
  SCHAR preShift;
  SCHAR shift, sum_e;
  FIXP_DBL sum;

  int j, k, l, li, ui;
  FIXP_DBL sumAll, sumLine; /* Single precision would be sufficient,
                             but overflow bits are required for accumulation */

  /* Divide by width of envelope later: */
  invWidth = FX_DBL2FX_SGL(GetInvInt(next_pos - start_pos));
  /* The common exponent needs to be doubled because all mantissas are squared:
   */
  input_e = input_e << 1;

  for (j = 0; j < nSfb; j++) {
    li = freqBandTable[j];
    ui = freqBandTable[j + 1];

    FIXP_DBL maxVal = maxSubbandSample(analysBufferReal, analysBufferImag, li,
                                       ui, start_pos, next_pos);

    if (maxVal != FL2FXCONST_DBL(0.f)) {
      preShift = CntLeadingZeros(maxVal) - 1;

      /* If the accu does not provide enough overflow bits, we cannot
         shift the samples up to the limit.
         Instead, keep up to 3 free bits in each sample, i.e. up to
         6 bits after calculation of square.
         Please note the comment on saturated arithmetic above!
      */
      preShift -= SHIFT_BEFORE_SQUARE;

      sumAll = FL2FXCONST_DBL(0.0f);

      for (k = li; k < ui; k++) {
        sumLine = FL2FXCONST_DBL(0.0f);

        if (analysBufferImag != NULL) {
          if (preShift >= 0) {
            for (l = start_pos; l < next_pos; l++) {
              temp = analysBufferReal[l][k] << (int)preShift;
              sumLine += fPow2Div2(temp);
              temp = analysBufferImag[l][k] << (int)preShift;
              sumLine += fPow2Div2(temp);
            }
          } else {
            for (l = start_pos; l < next_pos; l++) {
              temp = analysBufferReal[l][k] >> -(int)preShift;
              sumLine += fPow2Div2(temp);
              temp = analysBufferImag[l][k] >> -(int)preShift;
              sumLine += fPow2Div2(temp);
            }
          }
        } else {
          if (preShift >= 0) {
            for (l = start_pos; l < next_pos; l++) {
              temp = analysBufferReal[l][k] << (int)preShift;
              sumLine += fPow2Div2(temp);
            }
          } else {
            for (l = start_pos; l < next_pos; l++) {
              temp = analysBufferReal[l][k] >> -(int)preShift;
              sumLine += fPow2Div2(temp);
            }
          }
        }

        /* The number of QMF-channels per SBR bands may be up to 15.
           Shift right to avoid overflows in sum over all channels. */
        sumLine = sumLine >> (4 - 1);
        sumAll += sumLine;
      }

      /* Convert double precision to Mantissa/Exponent: */
      shift = fNorm(sumAll);
      sum = sumAll << (int)shift;

      /* Divide by width of envelope: */
      sum = fMult(sum, invWidth);

      /* Divide by width of Sfb: */
      sum = fMult(sum, FX_DBL2FX_SGL(GetInvInt(ui - li)));

      /* Set all Subband energies in the Sfb to the average energy: */
      if (analysBufferImag != NULL)
        sum_e = input_e + 4 - shift; /* -4 to compensate right-shift */
      else
        sum_e = input_e + 4 + 1 -
                shift; /* -4 to compensate right-shift; +1 due to missing
                          imag. part */

      sum_e -= 2 * preShift;
    } /* maxVal!=0 */
    else {
      /* Prevent a zero-mantissa-number from being misinterpreted
         due to its exponent. */
      sum = FL2FXCONST_DBL(0.0f);
      sum_e = 0;
    }

    for (k = li; k < ui; k++) {
      *nrgEst++ = sum;
      *nrgEst_e++ = sum_e;
    }
  }
}

/*!
  \brief  Calculate gain, noise, and additional sine level for one subband.

  The resulting energy gain is given by mantissa and exponent.
*/
static void calcSubbandGain(
    FIXP_DBL nrgRef, /*!< Reference Energy according to envelope data */
    SCHAR
        nrgRef_e, /*!< Reference Energy according to envelope data (exponent) */
    ENV_CALC_NRGS *nrgs, int i, FIXP_DBL tmpNoise, /*!< Relative noise level */
    SCHAR tmpNoise_e,      /*!< Relative noise level (exponent) */
    UCHAR sinePresentFlag, /*!< Indicates if sine is present on band */
    UCHAR sineMapped,      /*!< Indicates if sine must be added */
    int noNoiseFlag)       /*!< Flag to suppress noise addition */
{
  FIXP_DBL nrgEst = nrgs->nrgEst[i]; /*!< Energy in transposed signal */
  SCHAR nrgEst_e =
      nrgs->nrgEst_e[i]; /*!< Energy in transposed signal (exponent) */
  FIXP_DBL *ptrNrgGain = &nrgs->nrgGain[i]; /*!< Resulting energy gain */
  SCHAR *ptrNrgGain_e =
      &nrgs->nrgGain_e[i]; /*!< Resulting energy gain (exponent) */
  FIXP_DBL *ptrNoiseLevel =
      &nrgs->noiseLevel[i]; /*!< Resulting absolute noise energy */
  SCHAR *ptrNoiseLevel_e =
      &nrgs->noiseLevel_e[i]; /*!< Resulting absolute noise energy (exponent) */
  FIXP_DBL *ptrNrgSine = &nrgs->nrgSine[i]; /*!< Additional sine energy */
  SCHAR *ptrNrgSine_e =
      &nrgs->nrgSine_e[i]; /*!< Additional sine energy (exponent) */

  FIXP_DBL a, b, c;
  SCHAR a_e, b_e, c_e;

  /*
     This addition of 1 prevents divisions by zero in the reference code.
     For very small energies in nrgEst, it prevents the gains from becoming
     very high which could cause some trouble due to the smoothing.
  */
  b_e = (int)(nrgEst_e - 1);
  if (b_e >= 0) {
    nrgEst = (FL2FXCONST_DBL(0.5f) >> (INT)fixMin(b_e + 1, DFRACT_BITS - 1)) +
             (nrgEst >> 1);
    nrgEst_e += 1; /* shift by 1 bit to avoid overflow */

  } else {
    nrgEst = (nrgEst >> (INT)(fixMin(-b_e + 1, DFRACT_BITS - 1))) +
             (FL2FXCONST_DBL(0.5f) >> 1);
    nrgEst_e = 2; /* shift by 1 bit to avoid overflow */
  }

  /*  A = NrgRef * TmpNoise */
  a = fMult(nrgRef, tmpNoise);
  a_e = nrgRef_e + tmpNoise_e;

  /*  B = 1 + TmpNoise */
  b_e = (int)(tmpNoise_e - 1);
  if (b_e >= 0) {
    b = (FL2FXCONST_DBL(0.5f) >> (INT)fixMin(b_e + 1, DFRACT_BITS - 1)) +
        (tmpNoise >> 1);
    b_e = tmpNoise_e + 1; /* shift by 1 bit to avoid overflow */
  } else {
    b = (tmpNoise >> (INT)(fixMin(-b_e + 1, DFRACT_BITS - 1))) +
        (FL2FXCONST_DBL(0.5f) >> 1);
    b_e = 2; /* shift by 1 bit to avoid overflow */
  }

  /*  noiseLevel = A / B = (NrgRef * TmpNoise) / (1 + TmpNoise) */
  FDK_divide_MantExp(a, a_e, b, b_e, ptrNoiseLevel, ptrNoiseLevel_e);

  if (sinePresentFlag) {
    /*  C = (1 + TmpNoise) * NrgEst */
    c = fMult(b, nrgEst);
    c_e = b_e + nrgEst_e;

    /*  gain = A / C = (NrgRef * TmpNoise) / (1 + TmpNoise) * NrgEst */
    FDK_divide_MantExp(a, a_e, c, c_e, ptrNrgGain, ptrNrgGain_e);

    if (sineMapped) {
      /*  sineLevel = nrgRef/ (1 + TmpNoise) */
      FDK_divide_MantExp(nrgRef, nrgRef_e, b, b_e, ptrNrgSine, ptrNrgSine_e);
    }
  } else {
    if (noNoiseFlag) {
      /*  B = NrgEst */
      b = nrgEst;
      b_e = nrgEst_e;
    } else {
      /*  B = NrgEst * (1 + TmpNoise) */
      b = fMult(b, nrgEst);
      b_e = b_e + nrgEst_e;
    }

    /*  gain = nrgRef / B */
    INT result_exp = 0;
    *ptrNrgGain = fDivNorm(nrgRef, b, &result_exp);
    *ptrNrgGain_e = (SCHAR)result_exp + (nrgRef_e - b_e);

    /* There could be a one bit diffs. This is important to compensate,
       because later in the code values are compared by exponent only. */
    int headroom = CountLeadingBits(*ptrNrgGain);
    *ptrNrgGain <<= headroom;
    *ptrNrgGain_e -= headroom;
  }
}

/*!
  \brief  Calculate "average gain" for the specified subband range.

  This is rather a gain of the average magnitude than the average
  of gains!
  The result is used as a relative limit for all gains within the
  current "limiter band" (a certain frequency range).
*/
static void calcAvgGain(
    ENV_CALC_NRGS *nrgs, int lowSubband, /*!< Begin of the limiter band */
    int highSubband,                     /*!< High end of the limiter band */
    FIXP_DBL *ptrSumRef, SCHAR *ptrSumRef_e,
    FIXP_DBL *ptrAvgGain, /*!< Resulting overall gain (mantissa) */
    SCHAR *ptrAvgGain_e)  /*!< Resulting overall gain (exponent) */
{
  FIXP_DBL *nrgRef =
      nrgs->nrgRef; /*!< Reference Energy according to envelope data */
  SCHAR *nrgRef_e =
      nrgs->nrgRef_e; /*!< Reference Energy according to envelope data
                         (exponent) */
  FIXP_DBL *nrgEst = nrgs->nrgEst; /*!< Energy in transposed signal */
  SCHAR *nrgEst_e =
      nrgs->nrgEst_e; /*!< Energy in transposed signal (exponent) */

  FIXP_DBL sumRef = 1;
  FIXP_DBL sumEst = 1;
  SCHAR sumRef_e = -FRACT_BITS;
  SCHAR sumEst_e = -FRACT_BITS;
  int k;

  for (k = lowSubband; k < highSubband; k++) {
    /* Add nrgRef[k] to sumRef: */
    FDK_add_MantExp(sumRef, sumRef_e, nrgRef[k], nrgRef_e[k], &sumRef,
                    &sumRef_e);

    /* Add nrgEst[k] to sumEst: */
    FDK_add_MantExp(sumEst, sumEst_e, nrgEst[k], nrgEst_e[k], &sumEst,
                    &sumEst_e);
  }

  FDK_divide_MantExp(sumRef, sumRef_e, sumEst, sumEst_e, ptrAvgGain,
                     ptrAvgGain_e);

  *ptrSumRef = sumRef;
  *ptrSumRef_e = sumRef_e;
}

static void adjustTimeSlot_EldGrid(
    FIXP_DBL *RESTRICT
        ptrReal, /*!< Subband samples to be adjusted, real part */
    ENV_CALC_NRGS *nrgs, UCHAR *ptrHarmIndex, /*!< Harmonic index */
    int lowSubband, /*!< Lowest QMF-channel in the currently used SBR range. */
    int noSubbands, /*!< Number of QMF subbands */
    int scale_change,   /*!< Number of bits to shift adjusted samples */
    int noNoiseFlag,    /*!< Flag to suppress noise addition */
    int *ptrPhaseIndex, /*!< Start index to random number array */
    int scale_diff_low) /*!<  */

{
  int k;
  FIXP_DBL signalReal, sbNoise;
  int tone_count = 0;

  FIXP_DBL *pGain = nrgs->nrgGain; /*!< Gains of current envelope */
  FIXP_DBL *RESTRICT pNoiseLevel =
      nrgs->noiseLevel; /*!< Noise levels of current envelope */
  FIXP_DBL *RESTRICT pSineLevel = nrgs->nrgSine; /*!< Sine levels */

  int phaseIndex = *ptrPhaseIndex;
  UCHAR harmIndex = *ptrHarmIndex;

  static const INT harmonicPhase[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

  static const FIXP_DBL harmonicPhaseX[4][2] = {
      {FL2FXCONST_DBL(2.0 * 1.245183154539139e-001),
       FL2FXCONST_DBL(2.0 * 1.245183154539139e-001)},
      {FL2FXCONST_DBL(2.0 * -1.123767859325028e-001),
       FL2FXCONST_DBL(2.0 * 1.123767859325028e-001)},
      {FL2FXCONST_DBL(2.0 * -1.245183154539139e-001),
       FL2FXCONST_DBL(2.0 * -1.245183154539139e-001)},
      {FL2FXCONST_DBL(2.0 * 1.123767859325028e-001),
       FL2FXCONST_DBL(2.0 * -1.123767859325028e-001)}};

  const FIXP_DBL *p_harmonicPhaseX = &harmonicPhaseX[harmIndex][0];
  const INT *p_harmonicPhase = &harmonicPhase[harmIndex][0];

  const FIXP_DBL max_val = MAX_VAL_NRG_HEADROOM >> scale_change;
  const FIXP_DBL min_val = -max_val;

  *(ptrReal - 1) = fAddSaturate(
      *(ptrReal - 1),
      SATURATE_SHIFT(fMultDiv2(p_harmonicPhaseX[lowSubband & 1], pSineLevel[0]),
                     scale_diff_low, DFRACT_BITS));
  FIXP_DBL pSineLevel_prev = (FIXP_DBL)0;

  int idx_k = lowSubband & 1;

  for (k = 0; k < noSubbands; k++) {
    FIXP_DBL sineLevel_curr = *pSineLevel++;
    phaseIndex = (phaseIndex + 1) & (SBR_NF_NO_RANDOM_VAL - 1);

    signalReal = fMax(fMin(fMultDiv2(*ptrReal, *pGain++), max_val), min_val)
                 << scale_change;
    sbNoise = *pNoiseLevel++;
    if (((INT)sineLevel_curr | noNoiseFlag) == 0) {
      signalReal +=
          fMult(FDK_sbrDecoder_sbr_randomPhase[phaseIndex][0], sbNoise);
    }
    signalReal += sineLevel_curr * p_harmonicPhase[0];
    signalReal =
        fMultAddDiv2(signalReal, pSineLevel_prev, p_harmonicPhaseX[idx_k]);
    pSineLevel_prev = sineLevel_curr;
    idx_k = !idx_k;
    if (k < noSubbands - 1) {
      signalReal =
          fMultAddDiv2(signalReal, pSineLevel[0], p_harmonicPhaseX[idx_k]);
    } else /* (k == noSubbands - 1)  */
    {
      if (k + lowSubband + 1 < 63) {
        *(ptrReal + 1) += fMultDiv2(pSineLevel_prev, p_harmonicPhaseX[idx_k]);
      }
    }
    *ptrReal++ = signalReal;

    if (pSineLevel_prev != FL2FXCONST_DBL(0.0f)) {
      if (++tone_count == 16) {
        k++;
        break;
      }
    }
  }
  /* Run again, if previous loop got breaked with tone_count = 16 */
  for (; k < noSubbands; k++) {
    FIXP_DBL sineLevel_curr = *pSineLevel++;
    phaseIndex = (phaseIndex + 1) & (SBR_NF_NO_RANDOM_VAL - 1);

    signalReal = fMax(fMin(fMultDiv2(*ptrReal, *pGain++), max_val), min_val)
                 << scale_change;
    sbNoise = *pNoiseLevel++;
    if (((INT)sineLevel_curr | noNoiseFlag) == 0) {
      signalReal +=
          fMult(FDK_sbrDecoder_sbr_randomPhase[phaseIndex][0], sbNoise);
    }
    signalReal += sineLevel_curr * p_harmonicPhase[0];
    *ptrReal++ = signalReal;
  }

  *ptrHarmIndex = (harmIndex + 1) & 3;
  *ptrPhaseIndex = phaseIndex & (SBR_NF_NO_RANDOM_VAL - 1);
}

/*!
  \brief   Amplify one timeslot of the signal with the calculated gains
           and add the noisefloor.
*/

static void adjustTimeSlotLC(
    FIXP_DBL *ptrReal, /*!< Subband samples to be adjusted, real part */
    ENV_CALC_NRGS *nrgs, UCHAR *ptrHarmIndex, /*!< Harmonic index */
    int lowSubband, /*!< Lowest QMF-channel in the currently used SBR range. */
    int noSubbands, /*!< Number of QMF subbands */
    int scale_change,   /*!< Number of bits to shift adjusted samples */
    int noNoiseFlag,    /*!< Flag to suppress noise addition */
    int *ptrPhaseIndex) /*!< Start index to random number array */
{
  FIXP_DBL *pGain = nrgs->nrgGain; /*!< Gains of current envelope */
  FIXP_DBL *pNoiseLevel =
      nrgs->noiseLevel;                 /*!< Noise levels of current envelope */
  FIXP_DBL *pSineLevel = nrgs->nrgSine; /*!< Sine levels */

  int k;
  int index = *ptrPhaseIndex;
  UCHAR harmIndex = *ptrHarmIndex;
  UCHAR freqInvFlag = (lowSubband & 1);
  FIXP_DBL signalReal, sineLevel, sineLevelNext, sineLevelPrev;
  int tone_count = 0;
  int sineSign = 1;
  const FIXP_DBL max_val = MAX_VAL_NRG_HEADROOM >> scale_change;
  const FIXP_DBL min_val = -max_val;

#define C1 ((FIXP_SGL)FL2FXCONST_SGL(2.f * 0.00815f))
#define C1_CLDFB ((FIXP_SGL)FL2FXCONST_SGL(2.f * 0.16773f))

  /*
    First pass for k=0 pulled out of the loop:
  */

  index = (index + 1) & (SBR_NF_NO_RANDOM_VAL - 1);

  /*
    The next multiplication constitutes the actual envelope adjustment
    of the signal and should be carried out with full accuracy
    (supplying #FRACT_BITS valid bits).
  */
  signalReal = fMax(fMin(fMultDiv2(*ptrReal, *pGain++), max_val), min_val)
               << scale_change;
  sineLevel = *pSineLevel++;
  sineLevelNext = (noSubbands > 1) ? pSineLevel[0] : FL2FXCONST_DBL(0.0f);

  if (sineLevel != FL2FXCONST_DBL(0.0f))
    tone_count++;
  else if (!noNoiseFlag)
    /* Add noisefloor to the amplified signal */
    signalReal +=
        fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], pNoiseLevel[0]);

  {
    if (!(harmIndex & 0x1)) {
      /* harmIndex 0,2 */
      signalReal += (harmIndex & 0x2) ? -sineLevel : sineLevel;
      *ptrReal++ = signalReal;
    } else {
      /* harmIndex 1,3 in combination with freqInvFlag */
      int shift = (int)(scale_change + 1);
      shift = (shift >= 0) ? fixMin(DFRACT_BITS - 1, shift)
                           : fixMax(-(DFRACT_BITS - 1), shift);

      FIXP_DBL tmp1 = (shift >= 0) ? (fMultDiv2(C1, sineLevel) >> shift)
                                   : (fMultDiv2(C1, sineLevel) << (-shift));
      FIXP_DBL tmp2 = fMultDiv2(C1, sineLevelNext);

      /* save switch and compare operations and reduce to XOR statement */
      if (((harmIndex >> 1) & 0x1) ^ freqInvFlag) {
        *(ptrReal - 1) = fAddSaturate(*(ptrReal - 1), tmp1);
        signalReal -= tmp2;
      } else {
        *(ptrReal - 1) = fAddSaturate(*(ptrReal - 1), -tmp1);
        signalReal += tmp2;
      }
      *ptrReal++ = signalReal;
      freqInvFlag = !freqInvFlag;
    }
  }

  pNoiseLevel++;

  if (noSubbands > 2) {
    if (!(harmIndex & 0x1)) {
      /* harmIndex 0,2 */
      if (!harmIndex) {
        sineSign = 0;
      }

      for (k = noSubbands - 2; k != 0; k--) {
        FIXP_DBL sinelevel = *pSineLevel++;
        index++;
        if (((signalReal = (sineSign ? -sinelevel : sinelevel)) ==
             FL2FXCONST_DBL(0.0f)) &&
            !noNoiseFlag) {
          /* Add noisefloor to the amplified signal */
          index &= (SBR_NF_NO_RANDOM_VAL - 1);
          signalReal +=
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], pNoiseLevel[0]);
        }

        /* The next multiplication constitutes the actual envelope adjustment of
         * the signal. */
        signalReal +=
            fMax(fMin(fMultDiv2(*ptrReal, *pGain++), max_val), min_val)
            << scale_change;

        pNoiseLevel++;
        *ptrReal++ = signalReal;
      } /* for ... */
    } else {
      /* harmIndex 1,3 in combination with freqInvFlag */
      if (harmIndex == 1) freqInvFlag = !freqInvFlag;

      for (k = noSubbands - 2; k != 0; k--) {
        index++;
        /* The next multiplication constitutes the actual envelope adjustment of
         * the signal. */
        signalReal = fMax(fMin(fMultDiv2(*ptrReal, *pGain++), max_val), min_val)
                     << scale_change;

        if (*pSineLevel++ != FL2FXCONST_DBL(0.0f))
          tone_count++;
        else if (!noNoiseFlag) {
          /* Add noisefloor to the amplified signal */
          index &= (SBR_NF_NO_RANDOM_VAL - 1);
          signalReal +=
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], pNoiseLevel[0]);
        }

        pNoiseLevel++;

        if (tone_count <= 16) {
          FIXP_DBL addSine = fMultDiv2((pSineLevel[-2] - pSineLevel[0]), C1);
          signalReal += (freqInvFlag) ? (-addSine) : (addSine);
        }

        *ptrReal++ = signalReal;
        freqInvFlag = !freqInvFlag;
      } /* for ... */
    }
  }

  if (noSubbands > -1) {
    index++;
    /* The next multiplication constitutes the actual envelope adjustment of the
     * signal. */
    signalReal = fMax(fMin(fMultDiv2(*ptrReal, *pGain), max_val), min_val)
                 << scale_change;
    sineLevelPrev = fMultDiv2(pSineLevel[-1], FL2FX_SGL(0.0163f));
    sineLevel = pSineLevel[0];

    if (pSineLevel[0] != FL2FXCONST_DBL(0.0f))
      tone_count++;
    else if (!noNoiseFlag) {
      /* Add noisefloor to the amplified signal */
      index &= (SBR_NF_NO_RANDOM_VAL - 1);
      signalReal = signalReal + fMult(FDK_sbrDecoder_sbr_randomPhase[index][0],
                                      pNoiseLevel[0]);
    }

    if (!(harmIndex & 0x1)) {
      /* harmIndex 0,2 */
      *ptrReal = signalReal + ((sineSign) ? -sineLevel : sineLevel);
    } else {
      /* harmIndex 1,3 in combination with freqInvFlag */
      if (tone_count <= 16) {
        if (freqInvFlag) {
          *ptrReal++ = signalReal - sineLevelPrev;
          if (noSubbands + lowSubband < 63)
            *ptrReal = *ptrReal + fMultDiv2(C1, sineLevel);
        } else {
          *ptrReal++ = signalReal + sineLevelPrev;
          if (noSubbands + lowSubband < 63)
            *ptrReal = *ptrReal - fMultDiv2(C1, sineLevel);
        }
      } else
        *ptrReal = signalReal;
    }
  }
  *ptrHarmIndex = (harmIndex + 1) & 3;
  *ptrPhaseIndex = index & (SBR_NF_NO_RANDOM_VAL - 1);
}

static void adjustTimeSlotHQ_GainAndNoise(
    FIXP_DBL *RESTRICT
        ptrReal, /*!< Subband samples to be adjusted, real part */
    FIXP_DBL *RESTRICT
        ptrImag, /*!< Subband samples to be adjusted, imag part */
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env, ENV_CALC_NRGS *nrgs,
    int lowSubband, /*!< Lowest QMF-channel in the currently used SBR range. */
    int noSubbands, /*!< Number of QMF subbands */
    int scale_change,         /*!< Number of bits to shift adjusted samples */
    FIXP_SGL smooth_ratio,    /*!< Impact of last envelope */
    int noNoiseFlag,          /*!< Start index to random number array */
    int filtBufferNoiseShift) /*!< Shift factor of filtBufferNoise */
{
  FIXP_DBL *RESTRICT gain = nrgs->nrgGain; /*!< Gains of current envelope */
  FIXP_DBL *RESTRICT noiseLevel =
      nrgs->noiseLevel; /*!< Noise levels of current envelope */
  FIXP_DBL *RESTRICT pSineLevel = nrgs->nrgSine; /*!< Sine levels */

  FIXP_DBL *RESTRICT filtBuffer =
      h_sbr_cal_env->filtBuffer; /*!< Gains of last envelope */
  FIXP_DBL *RESTRICT filtBufferNoise =
      h_sbr_cal_env->filtBufferNoise; /*!< Noise levels of last envelope */
  int *RESTRICT ptrPhaseIndex =
      &h_sbr_cal_env->phaseIndex; /*!< Start index to random number array */

  int k;
  FIXP_DBL signalReal, signalImag;
  FIXP_DBL noiseReal, noiseImag;
  FIXP_DBL smoothedGain, smoothedNoise;
  FIXP_SGL direct_ratio =
      /*FL2FXCONST_SGL(1.0f) */ (FIXP_SGL)MAXVAL_SGL - smooth_ratio;
  int index = *ptrPhaseIndex;
  int shift;
  FIXP_DBL max_val_noise = 0, min_val_noise = 0;
  const FIXP_DBL max_val = MAX_VAL_NRG_HEADROOM >> scale_change;
  const FIXP_DBL min_val = -max_val;

  *ptrPhaseIndex = (index + noSubbands) & (SBR_NF_NO_RANDOM_VAL - 1);

  filtBufferNoiseShift +=
      1; /* due to later use of fMultDiv2 instead of fMult */
  if (filtBufferNoiseShift < 0) {
    shift = fixMin(DFRACT_BITS - 1, -filtBufferNoiseShift);
  } else {
    shift = fixMin(DFRACT_BITS - 1, filtBufferNoiseShift);
    max_val_noise = MAX_VAL_NRG_HEADROOM >> shift;
    min_val_noise = -max_val_noise;
  }

  if (smooth_ratio > FL2FXCONST_SGL(0.0f)) {
    for (k = 0; k < noSubbands; k++) {
      /*
        Smoothing: The old envelope has been bufferd and a certain ratio
        of the old gains and noise levels is used.
      */
      smoothedGain =
          fMult(smooth_ratio, filtBuffer[k]) + fMult(direct_ratio, gain[k]);

      if (filtBufferNoiseShift < 0) {
        smoothedNoise = (fMultDiv2(smooth_ratio, filtBufferNoise[k]) >> shift) +
                        fMult(direct_ratio, noiseLevel[k]);
      } else {
        smoothedNoise = fMultDiv2(smooth_ratio, filtBufferNoise[k]);
        smoothedNoise =
            (fMax(fMin(smoothedNoise, max_val_noise), min_val_noise) << shift) +
            fMult(direct_ratio, noiseLevel[k]);
      }

      smoothedNoise = fMax(fMin(smoothedNoise, (FIXP_DBL)(MAXVAL_DBL / 2)),
                           (FIXP_DBL)(MINVAL_DBL / 2));

      /*
        The next 2 multiplications constitute the actual envelope adjustment
        of the signal and should be carried out with full accuracy
        (supplying #DFRACT_BITS valid bits).
      */
      signalReal =
          fMax(fMin(fMultDiv2(*ptrReal, smoothedGain), max_val), min_val)
          << scale_change;
      signalImag =
          fMax(fMin(fMultDiv2(*ptrImag, smoothedGain), max_val), min_val)
          << scale_change;

      index++;

      if ((pSineLevel[k] != FL2FXCONST_DBL(0.0f)) || noNoiseFlag) {
        /* Just the amplified signal is saved */
        *ptrReal++ = signalReal;
        *ptrImag++ = signalImag;
      } else {
        /* Add noisefloor to the amplified signal */
        index &= (SBR_NF_NO_RANDOM_VAL - 1);
        noiseReal =
            fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], smoothedNoise);
        noiseImag =
            fMult(FDK_sbrDecoder_sbr_randomPhase[index][1], smoothedNoise);
        *ptrReal++ = (signalReal + noiseReal);
        *ptrImag++ = (signalImag + noiseImag);
      }
    }
  } else {
    for (k = 0; k < noSubbands; k++) {
      smoothedGain = gain[k];
      signalReal =
          fMax(fMin(fMultDiv2(*ptrReal, smoothedGain), max_val), min_val)
          << scale_change;
      signalImag =
          fMax(fMin(fMultDiv2(*ptrImag, smoothedGain), max_val), min_val)
          << scale_change;

      index++;

      if ((pSineLevel[k] == FL2FXCONST_DBL(0.0f)) && (noNoiseFlag == 0)) {
        /* Add noisefloor to the amplified signal */
        smoothedNoise = noiseLevel[k];
        index &= (SBR_NF_NO_RANDOM_VAL - 1);
        noiseReal =
            fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], smoothedNoise);
        noiseImag =
            fMult(FDK_sbrDecoder_sbr_randomPhase[index][1], smoothedNoise);

        signalReal += noiseReal;
        signalImag += noiseImag;
      }
      *ptrReal++ = signalReal;
      *ptrImag++ = signalImag;
    }
  }
}

static void adjustTimeSlotHQ_AddHarmonics(
    FIXP_DBL *RESTRICT
        ptrReal, /*!< Subband samples to be adjusted, real part */
    FIXP_DBL *RESTRICT
        ptrImag, /*!< Subband samples to be adjusted, imag part */
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env, ENV_CALC_NRGS *nrgs,
    int lowSubband,  /*!< Lowest QMF-channel in the currently used SBR range. */
    int noSubbands,  /*!< Number of QMF subbands */
    int scale_change /*!< Scale mismatch between QMF input and sineLevel
                        exponent. */
) {
  FIXP_DBL *RESTRICT pSineLevel = nrgs->nrgSine; /*!< Sine levels */
  UCHAR *RESTRICT ptrHarmIndex =
      &h_sbr_cal_env->harmIndex; /*!< Harmonic index */

  int k;
  FIXP_DBL signalReal, signalImag;
  UCHAR harmIndex = *ptrHarmIndex;
  int freqInvFlag = (lowSubband & 1);
  FIXP_DBL sineLevel;

  *ptrHarmIndex = (harmIndex + 1) & 3;

  for (k = 0; k < noSubbands; k++) {
    sineLevel = pSineLevel[k];
    freqInvFlag ^= 1;
    if (sineLevel != FL2FXCONST_DBL(0.f)) {
      signalReal = ptrReal[k];
      signalImag = ptrImag[k];
      sineLevel = scaleValue(sineLevel, scale_change);
      if (harmIndex & 2) {
        /* case 2,3 */
        sineLevel = -sineLevel;
      }
      if (!(harmIndex & 1)) {
        /* case 0,2: */
        ptrReal[k] = signalReal + sineLevel;
      } else {
        /* case 1,3 */
        if (!freqInvFlag) sineLevel = -sineLevel;
        ptrImag[k] = signalImag + sineLevel;
      }
    }
  }
}

static void adjustTimeSlotHQ(
    FIXP_DBL *RESTRICT
        ptrReal, /*!< Subband samples to be adjusted, real part */
    FIXP_DBL *RESTRICT
        ptrImag, /*!< Subband samples to be adjusted, imag part */
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env, ENV_CALC_NRGS *nrgs,
    int lowSubband, /*!< Lowest QMF-channel in the currently used SBR range. */
    int noSubbands, /*!< Number of QMF subbands */
    int scale_change,         /*!< Number of bits to shift adjusted samples */
    FIXP_SGL smooth_ratio,    /*!< Impact of last envelope */
    int noNoiseFlag,          /*!< Start index to random number array */
    int filtBufferNoiseShift) /*!< Shift factor of filtBufferNoise */
{
  FIXP_DBL *RESTRICT gain = nrgs->nrgGain; /*!< Gains of current envelope */
  FIXP_DBL *RESTRICT noiseLevel =
      nrgs->noiseLevel; /*!< Noise levels of current envelope */
  FIXP_DBL *RESTRICT pSineLevel = nrgs->nrgSine; /*!< Sine levels */

  FIXP_DBL *RESTRICT filtBuffer =
      h_sbr_cal_env->filtBuffer; /*!< Gains of last envelope */
  FIXP_DBL *RESTRICT filtBufferNoise =
      h_sbr_cal_env->filtBufferNoise; /*!< Noise levels of last envelope */
  UCHAR *RESTRICT ptrHarmIndex =
      &h_sbr_cal_env->harmIndex; /*!< Harmonic index */
  int *RESTRICT ptrPhaseIndex =
      &h_sbr_cal_env->phaseIndex; /*!< Start index to random number array */

  int k;
  FIXP_DBL signalReal, signalImag;
  FIXP_DBL noiseReal, noiseImag;
  FIXP_DBL smoothedGain, smoothedNoise;
  FIXP_SGL direct_ratio =
      /*FL2FXCONST_SGL(1.0f) */ (FIXP_SGL)MAXVAL_SGL - smooth_ratio;
  int index = *ptrPhaseIndex;
  UCHAR harmIndex = *ptrHarmIndex;
  int freqInvFlag = (lowSubband & 1);
  FIXP_DBL sineLevel;
  int shift;
  FIXP_DBL max_val_noise = 0, min_val_noise = 0;
  const FIXP_DBL max_val = MAX_VAL_NRG_HEADROOM >> scale_change;
  const FIXP_DBL min_val = -max_val;

  *ptrPhaseIndex = (index + noSubbands) & (SBR_NF_NO_RANDOM_VAL - 1);
  *ptrHarmIndex = (harmIndex + 1) & 3;

  /*
    Possible optimization:
    smooth_ratio and harmIndex stay constant during the loop.
    It might be faster to include a separate loop in each path.

    the check for smooth_ratio is now outside the loop and the workload
    of the whole function decreased by about 20 %
  */

  filtBufferNoiseShift +=
      1; /* due to later use of fMultDiv2 instead of fMult */
  if (filtBufferNoiseShift < 0) {
    shift = fixMin(DFRACT_BITS - 1, -filtBufferNoiseShift);
  } else {
    shift = fixMin(DFRACT_BITS - 1, filtBufferNoiseShift);
    max_val_noise = MAX_VAL_NRG_HEADROOM >> shift;
    min_val_noise = -max_val_noise;
  }

  if (smooth_ratio > FL2FXCONST_SGL(0.0f)) {
    for (k = 0; k < noSubbands; k++) {
      /*
        Smoothing: The old envelope has been bufferd and a certain ratio
        of the old gains and noise levels is used.
      */

      smoothedGain =
          fMult(smooth_ratio, filtBuffer[k]) + fMult(direct_ratio, gain[k]);

      if (filtBufferNoiseShift < 0) {
        smoothedNoise = (fMultDiv2(smooth_ratio, filtBufferNoise[k]) >> shift) +
                        fMult(direct_ratio, noiseLevel[k]);
      } else {
        smoothedNoise = fMultDiv2(smooth_ratio, filtBufferNoise[k]);
        smoothedNoise =
            (fMax(fMin(smoothedNoise, max_val_noise), min_val_noise) << shift) +
            fMult(direct_ratio, noiseLevel[k]);
      }

      smoothedNoise = fMax(fMin(smoothedNoise, (FIXP_DBL)(MAXVAL_DBL / 2)),
                           (FIXP_DBL)(MINVAL_DBL / 2));

      /*
        The next 2 multiplications constitute the actual envelope adjustment
        of the signal and should be carried out with full accuracy
        (supplying #DFRACT_BITS valid bits).
      */
      signalReal =
          fMax(fMin(fMultDiv2(*ptrReal, smoothedGain), max_val), min_val)
          << scale_change;
      signalImag =
          fMax(fMin(fMultDiv2(*ptrImag, smoothedGain), max_val), min_val)
          << scale_change;

      index++;

      if (pSineLevel[k] != FL2FXCONST_DBL(0.0f)) {
        sineLevel = pSineLevel[k];

        switch (harmIndex) {
          case 0:
            *ptrReal++ = (signalReal + sineLevel);
            *ptrImag++ = (signalImag);
            break;
          case 2:
            *ptrReal++ = (signalReal - sineLevel);
            *ptrImag++ = (signalImag);
            break;
          case 1:
            *ptrReal++ = (signalReal);
            if (freqInvFlag)
              *ptrImag++ = (signalImag - sineLevel);
            else
              *ptrImag++ = (signalImag + sineLevel);
            break;
          case 3:
            *ptrReal++ = signalReal;
            if (freqInvFlag)
              *ptrImag++ = (signalImag + sineLevel);
            else
              *ptrImag++ = (signalImag - sineLevel);
            break;
        }
      } else {
        if (noNoiseFlag) {
          /* Just the amplified signal is saved */
          *ptrReal++ = (signalReal);
          *ptrImag++ = (signalImag);
        } else {
          /* Add noisefloor to the amplified signal */
          index &= (SBR_NF_NO_RANDOM_VAL - 1);
          noiseReal =
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], smoothedNoise);
          noiseImag =
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][1], smoothedNoise);
          *ptrReal++ = (signalReal + noiseReal);
          *ptrImag++ = (signalImag + noiseImag);
        }
      }
      freqInvFlag ^= 1;
    }

  } else {
    for (k = 0; k < noSubbands; k++) {
      smoothedGain = gain[k];
      signalReal =
          fMax(fMin(fMultDiv2(*ptrReal, smoothedGain), max_val), min_val)
          << scale_change;
      signalImag =
          fMax(fMin(fMultDiv2(*ptrImag, smoothedGain), max_val), min_val)
          << scale_change;

      index++;

      if ((sineLevel = pSineLevel[k]) != FL2FXCONST_DBL(0.0f)) {
        switch (harmIndex) {
          case 0:
            signalReal += sineLevel;
            break;
          case 1:
            if (freqInvFlag)
              signalImag -= sineLevel;
            else
              signalImag += sineLevel;
            break;
          case 2:
            signalReal -= sineLevel;
            break;
          case 3:
            if (freqInvFlag)
              signalImag += sineLevel;
            else
              signalImag -= sineLevel;
            break;
        }
      } else {
        if (noNoiseFlag == 0) {
          /* Add noisefloor to the amplified signal */
          smoothedNoise = noiseLevel[k];
          index &= (SBR_NF_NO_RANDOM_VAL - 1);
          noiseReal =
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][0], smoothedNoise);
          noiseImag =
              fMult(FDK_sbrDecoder_sbr_randomPhase[index][1], smoothedNoise);

          signalReal += noiseReal;
          signalImag += noiseImag;
        }
      }
      *ptrReal++ = signalReal;
      *ptrImag++ = signalImag;

      freqInvFlag ^= 1;
    }
  }
}

/*!
  \brief   Reset limiter bands.

  Build frequency band table for the gain limiter dependent on
  the previously generated transposer patch areas.

  \return  SBRDEC_OK if ok,  SBRDEC_UNSUPPORTED_CONFIG on error
*/
SBR_ERROR
ResetLimiterBands(
    UCHAR *limiterBandTable, /*!< Resulting band borders in QMF channels */
    UCHAR *noLimiterBands,   /*!< Resulting number of limiter band */
    UCHAR *freqBandTable,    /*!< Table with possible band borders */
    int noFreqBands,         /*!< Number of bands in freqBandTable */
    const PATCH_PARAM *patchParam, /*!< Transposer patch parameters */
    int noPatches,                 /*!< Number of transposer patches */
    int limiterBands, /*!< Selected 'band density' from bitstream */
    UCHAR sbrPatchingMode, int xOverQmf[MAX_NUM_PATCHES], int b41Sbr) {
  int i, k, isPatchBorder[2], loLimIndex, hiLimIndex, tempNoLim, nBands;
  UCHAR workLimiterBandTable[MAX_FREQ_COEFFS / 2 + MAX_NUM_PATCHES + 1];
  int patchBorders[MAX_NUM_PATCHES + 1];
  int kx, k2;

  int lowSubband = freqBandTable[0];
  int highSubband = freqBandTable[noFreqBands];

  /* 1 limiter band. */
  if (limiterBands == 0) {
    limiterBandTable[0] = 0;
    limiterBandTable[1] = highSubband - lowSubband;
    nBands = 1;
  } else {
    if (!sbrPatchingMode && xOverQmf != NULL) {
      noPatches = 0;

      if (b41Sbr == 1) {
        for (i = 1; i < MAX_NUM_PATCHES_HBE; i++)
          if (xOverQmf[i] != 0) noPatches++;
      } else {
        for (i = 1; i < MAX_STRETCH_HBE; i++)
          if (xOverQmf[i] != 0) noPatches++;
      }
      for (i = 0; i < noPatches; i++) {
        patchBorders[i] = xOverQmf[i] - lowSubband;
      }
    } else {
      for (i = 0; i < noPatches; i++) {
        patchBorders[i] = patchParam[i].guardStartBand - lowSubband;
      }
    }
    patchBorders[i] = highSubband - lowSubband;

    /* 1.2, 2, or 3 limiter bands/octave plus bandborders at patchborders. */
    for (k = 0; k <= noFreqBands; k++) {
      workLimiterBandTable[k] = freqBandTable[k] - lowSubband;
    }
    for (k = 1; k < noPatches; k++) {
      workLimiterBandTable[noFreqBands + k] = patchBorders[k];
    }

    tempNoLim = nBands = noFreqBands + noPatches - 1;
    shellsort(workLimiterBandTable, tempNoLim + 1);

    loLimIndex = 0;
    hiLimIndex = 1;

    while (hiLimIndex <= tempNoLim) {
      FIXP_DBL div_m, oct_m, temp;
      INT div_e = 0, oct_e = 0, temp_e = 0;

      k2 = workLimiterBandTable[hiLimIndex] + lowSubband;
      kx = workLimiterBandTable[loLimIndex] + lowSubband;

      div_m = fDivNorm(k2, kx, &div_e);

      /* calculate number of octaves */
      oct_m = fLog2(div_m, div_e, &oct_e);

      /* multiply with limiterbands per octave    */
      /* values 1, 1.2, 2, 3 -> scale factor of 2 */
      temp = fMultNorm(
          oct_m, FDK_sbrDecoder_sbr_limiterBandsPerOctaveDiv4_DBL[limiterBands],
          &temp_e);

      /* overall scale factor of temp ist addition of scalefactors from log2
         calculation, limiter bands scalefactor (2) and limiter bands
         multiplication */
      temp_e += oct_e + 2;

      /*    div can be a maximum of 64 (k2 = 64 and kx = 1)
         -> oct can be a maximum of 6
         -> temp can be a maximum of 18 (as limiterBandsPerOctoave is a maximum
         factor of 3)
         -> we need a scale factor of 5 for comparisson
      */
      if (temp >> (5 - temp_e) < FL2FXCONST_DBL(0.49f) >> 5) {
        if (workLimiterBandTable[hiLimIndex] ==
            workLimiterBandTable[loLimIndex]) {
          workLimiterBandTable[hiLimIndex] = highSubband;
          nBands--;
          hiLimIndex++;
          continue;
        }
        isPatchBorder[0] = isPatchBorder[1] = 0;
        for (k = 0; k <= noPatches; k++) {
          if (workLimiterBandTable[hiLimIndex] == patchBorders[k]) {
            isPatchBorder[1] = 1;
            break;
          }
        }
        if (!isPatchBorder[1]) {
          workLimiterBandTable[hiLimIndex] = highSubband;
          nBands--;
          hiLimIndex++;
          continue;
        }
        for (k = 0; k <= noPatches; k++) {
          if (workLimiterBandTable[loLimIndex] == patchBorders[k]) {
            isPatchBorder[0] = 1;
            break;
          }
        }
        if (!isPatchBorder[0]) {
          workLimiterBandTable[loLimIndex] = highSubband;
          nBands--;
        }
      }
      loLimIndex = hiLimIndex;
      hiLimIndex++;
    }
    shellsort(workLimiterBandTable, tempNoLim + 1);

    /* Test if algorithm exceeded maximum allowed limiterbands */
    if (nBands > MAX_NUM_LIMITERS || nBands <= 0) {
      return SBRDEC_UNSUPPORTED_CONFIG;
    }

    /* Restrict maximum value of limiter band table */
    if (workLimiterBandTable[tempNoLim] > highSubband) {
      return SBRDEC_UNSUPPORTED_CONFIG;
    }

    /* Copy limiterbands from working buffer into final destination */
    for (k = 0; k <= nBands; k++) {
      limiterBandTable[k] = workLimiterBandTable[k];
    }
  }
  *noLimiterBands = nBands;

  return SBRDEC_OK;
}
