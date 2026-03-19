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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  envelope decoding
  This module provides envelope decoding and error concealment algorithms. The
  main entry point is decodeSbrData().

  \sa decodeSbrData(),\ref documentationOverview
*/

#include "env_dec.h"

#include "env_extr.h"
#include "transcendent.h"

#include "genericStds.h"

static void decodeEnvelope(HANDLE_SBR_HEADER_DATA hHeaderData,
                           HANDLE_SBR_FRAME_DATA h_sbr_data,
                           HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                           HANDLE_SBR_PREV_FRAME_DATA h_prev_data_otherChannel);
static void sbr_envelope_unmapping(HANDLE_SBR_HEADER_DATA hHeaderData,
                                   HANDLE_SBR_FRAME_DATA h_data_left,
                                   HANDLE_SBR_FRAME_DATA h_data_right);
static void requantizeEnvelopeData(HANDLE_SBR_FRAME_DATA h_sbr_data,
                                   int ampResolution);
static void deltaToLinearPcmEnvelopeDecoding(
    HANDLE_SBR_HEADER_DATA hHeaderData, HANDLE_SBR_FRAME_DATA h_sbr_data,
    HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static void decodeNoiseFloorlevels(HANDLE_SBR_HEADER_DATA hHeaderData,
                                   HANDLE_SBR_FRAME_DATA h_sbr_data,
                                   HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static void timeCompensateFirstEnvelope(HANDLE_SBR_HEADER_DATA hHeaderData,
                                        HANDLE_SBR_FRAME_DATA h_sbr_data,
                                        HANDLE_SBR_PREV_FRAME_DATA h_prev_data);
static int checkEnvelopeData(HANDLE_SBR_HEADER_DATA hHeaderData,
                             HANDLE_SBR_FRAME_DATA h_sbr_data,
                             HANDLE_SBR_PREV_FRAME_DATA h_prev_data);

#define SBR_ENERGY_PAN_OFFSET (12 << ENV_EXP_FRACT)
#define SBR_MAX_ENERGY (35 << ENV_EXP_FRACT)

#define DECAY (1 << ENV_EXP_FRACT)

#if ENV_EXP_FRACT
#define DECAY_COUPLING \
  (1 << (ENV_EXP_FRACT - 1)) /*!< corresponds to a value of 0.5 */
#else
#define DECAY_COUPLING \
  1 /*!< If the energy data is not shifted, use 1 instead of 0.5 */
#endif

/*!
  \brief  Convert table index
*/
static int indexLow2High(int offset, /*!< mapping factor */
                         int index,  /*!< index to scalefactor band */
                         int res)    /*!< frequency resolution */
{
  if (res == 0) {
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

/*!
  \brief  Update previous envelope value for delta-coding

  The current envelope values needs to be stored for delta-coding
  in the next frame.  The stored envelope is always represented with
  the high frequency resolution.  If the current envelope uses the
  low frequency resolution, the energy value will be mapped to the
  corresponding high-res bands.
*/
static void mapLowResEnergyVal(
    FIXP_SGL currVal,   /*!< current energy value */
    FIXP_SGL *prevData, /*!< pointer to previous data vector */
    int offset,         /*!< mapping factor */
    int index,          /*!< index to scalefactor band */
    int res)            /*!< frequeny resolution */
{
  if (res == 0) {
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

/*!
  \brief    Convert raw envelope and noisefloor data to energy levels

  This function is being called by sbrDecoder_ParseElement() and provides two
  important algorithms:

  First the function decodes envelopes and noise floor levels as described in
  requantizeEnvelopeData() and sbr_envelope_unmapping(). The function also
  implements concealment algorithms in case there are errors within the sbr
  data. For both operations fractional arithmetic is used. Therefore you might
  encounter different output values on your target system compared to the
  reference implementation.
*/
void decodeSbrData(
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA
        h_data_left, /*!< pointer to left channel frame data */
    HANDLE_SBR_PREV_FRAME_DATA
        h_prev_data_left, /*!< pointer to left channel previous frame data */
    HANDLE_SBR_FRAME_DATA
        h_data_right, /*!< pointer to right channel frame data */
    HANDLE_SBR_PREV_FRAME_DATA
        h_prev_data_right) /*!< pointer to right channel previous frame data */
{
  FIXP_SGL tempSfbNrgPrev[MAX_FREQ_COEFFS];
  int errLeft;

  /* Save previous energy values to be able to reuse them later for concealment.
   */
  FDKmemcpy(tempSfbNrgPrev, h_prev_data_left->sfb_nrg_prev,
            MAX_FREQ_COEFFS * sizeof(FIXP_SGL));

  if (hHeaderData->frameErrorFlag || hHeaderData->bs_info.pvc_mode == 0) {
    decodeEnvelope(hHeaderData, h_data_left, h_prev_data_left,
                   h_prev_data_right);
  } else {
    FDK_ASSERT(h_data_right == NULL);
  }
  decodeNoiseFloorlevels(hHeaderData, h_data_left, h_prev_data_left);

  if (h_data_right != NULL) {
    errLeft = hHeaderData->frameErrorFlag;
    decodeEnvelope(hHeaderData, h_data_right, h_prev_data_right,
                   h_prev_data_left);
    decodeNoiseFloorlevels(hHeaderData, h_data_right, h_prev_data_right);

    if (!errLeft && hHeaderData->frameErrorFlag) {
      /* If an error occurs in the right channel where the left channel seemed
         ok, we apply concealment also on the left channel. This ensures that
         the coupling modes of both channels match and that we have the same
         number of envelopes in coupling mode. However, as the left channel has
         already been processed before, the resulting energy levels are not the
         same as if the left channel had been concealed during the first call of
         decodeEnvelope().
      */
      /* Restore previous energy values for concealment, because the values have
         been overwritten by the first call of decodeEnvelope(). */
      FDKmemcpy(h_prev_data_left->sfb_nrg_prev, tempSfbNrgPrev,
                MAX_FREQ_COEFFS * sizeof(FIXP_SGL));
      /* Do concealment */
      decodeEnvelope(hHeaderData, h_data_left, h_prev_data_left,
                     h_prev_data_right);
    }

    if (h_data_left->coupling) {
      sbr_envelope_unmapping(hHeaderData, h_data_left, h_data_right);
    }
  }

  /* Display the data for debugging: */
}

/*!
  \brief   Convert from coupled channels to independent L/R data
*/
static void sbr_envelope_unmapping(
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_data_left,  /*!< pointer to left channel */
    HANDLE_SBR_FRAME_DATA h_data_right) /*!< pointer to right channel */
{
  int i;
  FIXP_SGL tempL_m, tempR_m, tempRplus1_m, newL_m, newR_m;
  SCHAR tempL_e, tempR_e, tempRplus1_e, newL_e, newR_e;

  /* 1. Unmap (already dequantized) coupled envelope energies */

  for (i = 0; i < h_data_left->nScaleFactors; i++) {
    tempR_m = (FIXP_SGL)((LONG)h_data_right->iEnvelope[i] & MASK_M);
    tempR_e = (SCHAR)((LONG)h_data_right->iEnvelope[i] & MASK_E);

    tempR_e -= (18 + NRG_EXP_OFFSET); /* -18 = ld(UNMAPPING_SCALE /
                                         h_data_right->nChannels) */
    tempL_m = (FIXP_SGL)((LONG)h_data_left->iEnvelope[i] & MASK_M);
    tempL_e = (SCHAR)((LONG)h_data_left->iEnvelope[i] & MASK_E);

    tempL_e -= NRG_EXP_OFFSET;

    /* Calculate tempRight+1 */
    FDK_add_MantExp(tempR_m, tempR_e, FL2FXCONST_SGL(0.5f), 1, /* 1.0 */
                    &tempRplus1_m, &tempRplus1_e);

    FDK_divide_MantExp(tempL_m, tempL_e + 1, /*  2 * tempLeft */
                       tempRplus1_m, tempRplus1_e, &newR_m, &newR_e);

    if (newR_m >= ((FIXP_SGL)MAXVAL_SGL - ROUNDING)) {
      newR_m >>= 1;
      newR_e += 1;
    }

    newL_m = FX_DBL2FX_SGL(fMult(tempR_m, newR_m));
    newL_e = tempR_e + newR_e;

    h_data_right->iEnvelope[i] =
        ((FIXP_SGL)((SHORT)(FIXP_SGL)(newR_m + ROUNDING) & MASK_M)) +
        (FIXP_SGL)((SHORT)(FIXP_SGL)(newR_e + NRG_EXP_OFFSET) & MASK_E);
    h_data_left->iEnvelope[i] =
        ((FIXP_SGL)((SHORT)(FIXP_SGL)(newL_m + ROUNDING) & MASK_M)) +
        (FIXP_SGL)((SHORT)(FIXP_SGL)(newL_e + NRG_EXP_OFFSET) & MASK_E);
  }

  /* 2. Dequantize and unmap coupled noise floor levels */

  for (i = 0; i < hHeaderData->freqBandData.nNfb *
                      h_data_left->frameInfo.nNoiseEnvelopes;
       i++) {
    tempL_e = (SCHAR)(6 - (LONG)h_data_left->sbrNoiseFloorLevel[i]);
    tempR_e = (SCHAR)((LONG)h_data_right->sbrNoiseFloorLevel[i] -
                      12) /*SBR_ENERGY_PAN_OFFSET*/;

    /* Calculate tempR+1 */
    FDK_add_MantExp(FL2FXCONST_SGL(0.5f), 1 + tempR_e, /* tempR */
                    FL2FXCONST_SGL(0.5f), 1,           /*  1.0  */
                    &tempRplus1_m, &tempRplus1_e);

    /* Calculate 2*tempLeft/(tempR+1) */
    FDK_divide_MantExp(FL2FXCONST_SGL(0.5f), tempL_e + 2, /*  2 * tempLeft */
                       tempRplus1_m, tempRplus1_e, &newR_m, &newR_e);

    /* if (newR_m >= ((FIXP_SGL)MAXVAL_SGL - ROUNDING)) {
      newR_m >>= 1;
      newR_e += 1;
    } */

    /* L = tempR * R */
    newL_m = newR_m;
    newL_e = newR_e + tempR_e;
    h_data_right->sbrNoiseFloorLevel[i] =
        ((FIXP_SGL)((SHORT)(FIXP_SGL)(newR_m + ROUNDING) & MASK_M)) +
        (FIXP_SGL)((SHORT)(FIXP_SGL)(newR_e + NOISE_EXP_OFFSET) & MASK_E);
    h_data_left->sbrNoiseFloorLevel[i] =
        ((FIXP_SGL)((SHORT)(FIXP_SGL)(newL_m + ROUNDING) & MASK_M)) +
        (FIXP_SGL)((SHORT)(FIXP_SGL)(newL_e + NOISE_EXP_OFFSET) & MASK_E);
  }
}

/*!
  \brief    Simple alternative to the real SBR concealment

  If the real frameInfo is not available due to a frame loss, a replacement will
  be constructed with 1 envelope spanning the whole frame (FIX-FIX).
  The delta-coded energies are set to negative values, resulting in a fade-down.
  In case of coupling, the balance-channel will move towards the center.
*/
static void leanSbrConcealment(
    HANDLE_SBR_HEADER_DATA hHeaderData,    /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,      /*!< pointer to current data */
    HANDLE_SBR_PREV_FRAME_DATA h_prev_data /*!< pointer to data of last frame */
) {
  FIXP_SGL target; /* targeted level for sfb_nrg_prev during fade-down */
  FIXP_SGL step;   /* speed of fade */
  int i;

  int currentStartPos =
      fMax(0, h_prev_data->stopPos - hHeaderData->numberTimeSlots);
  int currentStopPos = hHeaderData->numberTimeSlots;

  /* Use some settings of the previous frame */
  h_sbr_data->ampResolutionCurrentFrame = h_prev_data->ampRes;
  h_sbr_data->coupling = h_prev_data->coupling;
  for (i = 0; i < MAX_INVF_BANDS; i++)
    h_sbr_data->sbr_invf_mode[i] = h_prev_data->sbr_invf_mode[i];

  /* Generate concealing control data */

  h_sbr_data->frameInfo.nEnvelopes = 1;
  h_sbr_data->frameInfo.borders[0] = currentStartPos;
  h_sbr_data->frameInfo.borders[1] = currentStopPos;
  h_sbr_data->frameInfo.freqRes[0] = 1;
  h_sbr_data->frameInfo.tranEnv = -1; /* no transient */
  h_sbr_data->frameInfo.nNoiseEnvelopes = 1;
  h_sbr_data->frameInfo.bordersNoise[0] = currentStartPos;
  h_sbr_data->frameInfo.bordersNoise[1] = currentStopPos;

  h_sbr_data->nScaleFactors = hHeaderData->freqBandData.nSfb[1];

  /* Generate fake envelope data */

  h_sbr_data->domain_vec[0] = 1;

  if (h_sbr_data->coupling == COUPLING_BAL) {
    target = (FIXP_SGL)SBR_ENERGY_PAN_OFFSET;
    step = (FIXP_SGL)DECAY_COUPLING;
  } else {
    target = FL2FXCONST_SGL(0.0f);
    step = (FIXP_SGL)DECAY;
  }
  if (hHeaderData->bs_info.ampResolution == 0) {
    target <<= 1;
    step <<= 1;
  }

  for (i = 0; i < h_sbr_data->nScaleFactors; i++) {
    if (h_prev_data->sfb_nrg_prev[i] > target)
      h_sbr_data->iEnvelope[i] = -step;
    else
      h_sbr_data->iEnvelope[i] = step;
  }

  /* Noisefloor levels are always cleared ... */

  h_sbr_data->domain_vec_noise[0] = 1;
  FDKmemclear(h_sbr_data->sbrNoiseFloorLevel,
              sizeof(h_sbr_data->sbrNoiseFloorLevel));

  /* ... and so are the sines */
  FDKmemclear(h_sbr_data->addHarmonics,
              sizeof(ULONG) * ADD_HARMONICS_FLAGS_SIZE);
}

/*!
  \brief   Build reference energies and noise levels from bitstream elements
*/
static void decodeEnvelope(
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,   /*!< pointer to current data */
    HANDLE_SBR_PREV_FRAME_DATA
        h_prev_data, /*!< pointer to data of last frame */
    HANDLE_SBR_PREV_FRAME_DATA
        otherChannel /*!< other channel's last frame data */
) {
  int i;
  int fFrameError = hHeaderData->frameErrorFlag;
  FIXP_SGL tempSfbNrgPrev[MAX_FREQ_COEFFS];

  if (!fFrameError) {
    /*
      To avoid distortions after bad frames, set the error flag if delta coding
      in time occurs. However, SBR can take a little longer to come up again.
    */
    if (h_prev_data->frameErrorFlag) {
      if (h_sbr_data->domain_vec[0] != 0) {
        fFrameError = 1;
      }
    } else {
      /* Check that the previous stop position and the current start position
         match. (Could be done in checkFrameInfo(), but the previous frame data
         is not available there) */
      if (h_sbr_data->frameInfo.borders[0] !=
          h_prev_data->stopPos - hHeaderData->numberTimeSlots) {
        /* Both the previous as well as the current frame are flagged to be ok,
         * but they do not match! */
        if (h_sbr_data->domain_vec[0] == 1) {
          /* Prefer concealment over delta-time coding between the mismatching
           * frames */
          fFrameError = 1;
        } else {
          /* Close the gap in time by triggering timeCompensateFirstEnvelope()
           */
          fFrameError = 1;
        }
      }
    }
  }

  if (fFrameError) /* Error is detected */
  {
    leanSbrConcealment(hHeaderData, h_sbr_data, h_prev_data);

    /* decode the envelope data to linear PCM */
    deltaToLinearPcmEnvelopeDecoding(hHeaderData, h_sbr_data, h_prev_data);
  } else /*Do a temporary dummy decoding and check that the envelope values are
            within limits */
  {
    if (h_prev_data->frameErrorFlag) {
      timeCompensateFirstEnvelope(hHeaderData, h_sbr_data, h_prev_data);
      if (h_sbr_data->coupling != h_prev_data->coupling) {
        /*
          Coupling mode has changed during concealment.
           The stored energy levels need to be converted.
         */
        for (i = 0; i < hHeaderData->freqBandData.nSfb[1]; i++) {
          /* Former Level-Channel will be used for both channels */
          if (h_prev_data->coupling == COUPLING_BAL) {
            h_prev_data->sfb_nrg_prev[i] =
                (otherChannel != NULL) ? otherChannel->sfb_nrg_prev[i]
                                       : (FIXP_SGL)SBR_ENERGY_PAN_OFFSET;
          }
          /* Former L/R will be combined as the new Level-Channel */
          else if (h_sbr_data->coupling == COUPLING_LEVEL &&
                   otherChannel != NULL) {
            h_prev_data->sfb_nrg_prev[i] = (h_prev_data->sfb_nrg_prev[i] +
                                            otherChannel->sfb_nrg_prev[i]) >>
                                           1;
          } else if (h_sbr_data->coupling == COUPLING_BAL) {
            h_prev_data->sfb_nrg_prev[i] = (FIXP_SGL)SBR_ENERGY_PAN_OFFSET;
          }
        }
      }
    }
    FDKmemcpy(tempSfbNrgPrev, h_prev_data->sfb_nrg_prev,
              MAX_FREQ_COEFFS * sizeof(FIXP_SGL));

    deltaToLinearPcmEnvelopeDecoding(hHeaderData, h_sbr_data, h_prev_data);

    fFrameError = checkEnvelopeData(hHeaderData, h_sbr_data, h_prev_data);

    if (fFrameError) {
      hHeaderData->frameErrorFlag = 1;
      FDKmemcpy(h_prev_data->sfb_nrg_prev, tempSfbNrgPrev,
                MAX_FREQ_COEFFS * sizeof(FIXP_SGL));
      decodeEnvelope(hHeaderData, h_sbr_data, h_prev_data, otherChannel);
      return;
    }
  }

  requantizeEnvelopeData(h_sbr_data, h_sbr_data->ampResolutionCurrentFrame);

  hHeaderData->frameErrorFlag = fFrameError;
}

/*!
  \brief   Verify that envelope energies are within the allowed range
  \return  0 if all is fine, 1 if an envelope value was too high
*/
static int checkEnvelopeData(
    HANDLE_SBR_HEADER_DATA hHeaderData,    /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,      /*!< pointer to current data */
    HANDLE_SBR_PREV_FRAME_DATA h_prev_data /*!< pointer to data of last frame */
) {
  FIXP_SGL *iEnvelope = h_sbr_data->iEnvelope;
  FIXP_SGL *sfb_nrg_prev = h_prev_data->sfb_nrg_prev;
  int i = 0, errorFlag = 0;
  FIXP_SGL sbr_max_energy = (h_sbr_data->ampResolutionCurrentFrame == 1)
                                ? SBR_MAX_ENERGY
                                : (SBR_MAX_ENERGY << 1);

  /*
    Range check for current energies
  */
  for (i = 0; i < h_sbr_data->nScaleFactors; i++) {
    if (iEnvelope[i] > sbr_max_energy) {
      errorFlag = 1;
    }
    if (iEnvelope[i] < FL2FXCONST_SGL(0.0f)) {
      errorFlag = 1;
      /* iEnvelope[i] = FL2FXCONST_SGL(0.0f); */
    }
  }

  /*
    Range check for previous energies
  */
  for (i = 0; i < hHeaderData->freqBandData.nSfb[1]; i++) {
    sfb_nrg_prev[i] = fixMax(sfb_nrg_prev[i], FL2FXCONST_SGL(0.0f));
    sfb_nrg_prev[i] = fixMin(sfb_nrg_prev[i], sbr_max_energy);
  }

  return (errorFlag);
}

/*!
  \brief   Verify that the noise levels are within the allowed range

  The function is equivalent to checkEnvelopeData().
  When the noise-levels are being decoded, it is already too late for
  concealment. Therefore the noise levels are simply limited here.
*/
static void limitNoiseLevels(
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data)   /*!< pointer to current data */
{
  int i;
  int nNfb = hHeaderData->freqBandData.nNfb;

/*
  Set range limits. The exact values depend on the coupling mode.
  However this limitation is primarily intended to avoid unlimited
  accumulation of the delta-coded noise levels.
*/
#define lowerLimit \
  ((FIXP_SGL)0) /* lowerLimit actually refers to the _highest_ noise energy */
#define upperLimit \
  ((FIXP_SGL)35) /* upperLimit actually refers to the _lowest_ noise energy */

  /*
    Range check for current noise levels
  */
  for (i = 0; i < h_sbr_data->frameInfo.nNoiseEnvelopes * nNfb; i++) {
    h_sbr_data->sbrNoiseFloorLevel[i] =
        fixMin(h_sbr_data->sbrNoiseFloorLevel[i], upperLimit);
    h_sbr_data->sbrNoiseFloorLevel[i] =
        fixMax(h_sbr_data->sbrNoiseFloorLevel[i], lowerLimit);
  }
}

/*!
  \brief   Compensate for the wrong timing that might occur after a frame error.
*/
static void timeCompensateFirstEnvelope(
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,   /*!< pointer to actual data */
    HANDLE_SBR_PREV_FRAME_DATA
        h_prev_data) /*!< pointer to data of last frame */
{
  int i, nScalefactors;
  FRAME_INFO *pFrameInfo = &h_sbr_data->frameInfo;
  UCHAR *nSfb = hHeaderData->freqBandData.nSfb;
  int estimatedStartPos =
      fMax(0, h_prev_data->stopPos - hHeaderData->numberTimeSlots);
  int refLen, newLen, shift;
  FIXP_SGL deltaExp;

  /* Original length of first envelope according to bitstream */
  refLen = pFrameInfo->borders[1] - pFrameInfo->borders[0];
  /* Corrected length of first envelope (concealing can make the first envelope
   * longer) */
  newLen = pFrameInfo->borders[1] - estimatedStartPos;

  if (newLen <= 0) {
    /* An envelope length of <= 0 would not work, so we don't use it.
       May occur if the previous frame was flagged bad due to a mismatch
       of the old and new frame infos. */
    newLen = refLen;
    estimatedStartPos = pFrameInfo->borders[0];
  }

  deltaExp = FDK_getNumOctavesDiv8(newLen, refLen);

  /* Shift by -3 to rescale ld-table, ampRes-1 to enable coarser steps */
  shift = (FRACT_BITS - 1 - ENV_EXP_FRACT - 1 +
           h_sbr_data->ampResolutionCurrentFrame - 3);
  deltaExp = deltaExp >> shift;
  pFrameInfo->borders[0] = estimatedStartPos;
  pFrameInfo->bordersNoise[0] = estimatedStartPos;

  if (h_sbr_data->coupling != COUPLING_BAL) {
    nScalefactors = (pFrameInfo->freqRes[0]) ? nSfb[1] : nSfb[0];

    for (i = 0; i < nScalefactors; i++)
      h_sbr_data->iEnvelope[i] = h_sbr_data->iEnvelope[i] + deltaExp;
  }
}

/*!
  \brief   Convert each envelope value from logarithmic to linear domain

  Energy levels are transmitted in powers of 2, i.e. only the exponent
  is extracted from the bitstream.
  Therefore, normally only integer exponents can occur. However during
  fading (in case of a corrupt bitstream), a fractional part can also
  occur. The data in the array iEnvelope is shifted left by ENV_EXP_FRACT
  compared to an integer representation so that numbers smaller than 1
  can be represented.

  This function calculates a mantissa corresponding to the fractional
  part of the exponent for each reference energy. The array iEnvelope
  is converted in place to save memory. Input and output data must
  be interpreted differently, as shown in the below figure:

  \image html  EnvelopeData.png

  The data is then used in calculateSbrEnvelope().
*/
static void requantizeEnvelopeData(HANDLE_SBR_FRAME_DATA h_sbr_data,
                                   int ampResolution) {
  int i;
  FIXP_SGL mantissa;
  int ampShift = 1 - ampResolution;
  int exponent;

  /* In case that ENV_EXP_FRACT is changed to something else but 0 or 8,
     the initialization of this array has to be adapted!
  */
#if ENV_EXP_FRACT
  static const FIXP_SGL pow2[ENV_EXP_FRACT] = {
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 1))), /* 0.7071 */
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 2))), /* 0.5946 */
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 3))),
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 4))),
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 5))),
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 6))),
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 7))),
      FL2FXCONST_SGL(0.5f * pow(2.0f, pow(0.5f, 8))) /* 0.5013 */
  };

  int bit, mask;
#endif

  for (i = 0; i < h_sbr_data->nScaleFactors; i++) {
    exponent = (LONG)h_sbr_data->iEnvelope[i];

#if ENV_EXP_FRACT

    exponent = exponent >> ampShift;
    mantissa = 0.5f;

    /* Amplify mantissa according to the fractional part of the
       exponent (result will be between 0.500000 and 0.999999)
    */
    mask = 1; /* begin with lowest bit of exponent */

    for (bit = ENV_EXP_FRACT - 1; bit >= 0; bit--) {
      if (exponent & mask) {
        /* The current bit of the exponent is set,
           multiply mantissa with the corresponding factor: */
        mantissa = (FIXP_SGL)((mantissa * pow2[bit]) << 1);
      }
      /* Advance to next bit */
      mask = mask << 1;
    }

    /* Make integer part of exponent right aligned */
    exponent = exponent >> ENV_EXP_FRACT;

#else
    /* In case of the high amplitude resolution, 1 bit of the exponent gets lost
       by the shift. This will be compensated by a mantissa of 0.5*sqrt(2)
       instead of 0.5 if that bit is 1. */
    mantissa = (exponent & ampShift) ? FL2FXCONST_SGL(0.707106781186548f)
                                     : FL2FXCONST_SGL(0.5f);
    exponent = exponent >> ampShift;
#endif

    /*
      Mantissa was set to 0.5 (instead of 1.0, therefore increase exponent by
      1). Multiply by L=nChannels=64 by increasing exponent by another 6.
      => Increase exponent by 7
    */
    exponent += 7 + NRG_EXP_OFFSET;

    /* Combine mantissa and exponent and write back the result */
    h_sbr_data->iEnvelope[i] =
        ((FIXP_SGL)((SHORT)(FIXP_SGL)mantissa & MASK_M)) +
        (FIXP_SGL)((SHORT)(FIXP_SGL)exponent & MASK_E);
  }
}

/*!
  \brief   Build new reference energies from old ones and delta coded data
*/
static void deltaToLinearPcmEnvelopeDecoding(
    HANDLE_SBR_HEADER_DATA hHeaderData,     /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,       /*!< pointer to current data */
    HANDLE_SBR_PREV_FRAME_DATA h_prev_data) /*!< pointer to previous data */
{
  int i, domain, no_of_bands, band, freqRes;

  FIXP_SGL *sfb_nrg_prev = h_prev_data->sfb_nrg_prev;
  FIXP_SGL *ptr_nrg = h_sbr_data->iEnvelope;

  int offset =
      2 * hHeaderData->freqBandData.nSfb[0] - hHeaderData->freqBandData.nSfb[1];

  for (i = 0; i < h_sbr_data->frameInfo.nEnvelopes; i++) {
    domain = h_sbr_data->domain_vec[i];
    freqRes = h_sbr_data->frameInfo.freqRes[i];

    FDK_ASSERT(freqRes >= 0 && freqRes <= 1);

    no_of_bands = hHeaderData->freqBandData.nSfb[freqRes];

    FDK_ASSERT(no_of_bands < (64));

    if (domain == 0) {
      mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, 0, freqRes);
      ptr_nrg++;
      for (band = 1; band < no_of_bands; band++) {
        *ptr_nrg = *ptr_nrg + *(ptr_nrg - 1);
        mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, band, freqRes);
        ptr_nrg++;
      }
    } else {
      for (band = 0; band < no_of_bands; band++) {
        *ptr_nrg =
            *ptr_nrg + sfb_nrg_prev[indexLow2High(offset, band, freqRes)];
        mapLowResEnergyVal(*ptr_nrg, sfb_nrg_prev, offset, band, freqRes);
        ptr_nrg++;
      }
    }
  }
}

/*!
  \brief   Build new noise levels from old ones and delta coded data
*/
static void decodeNoiseFloorlevels(
    HANDLE_SBR_HEADER_DATA hHeaderData,     /*!< Static control data */
    HANDLE_SBR_FRAME_DATA h_sbr_data,       /*!< pointer to current data */
    HANDLE_SBR_PREV_FRAME_DATA h_prev_data) /*!< pointer to previous data */
{
  int i;
  int nNfb = hHeaderData->freqBandData.nNfb;
  int nNoiseFloorEnvelopes = h_sbr_data->frameInfo.nNoiseEnvelopes;

  /* Decode first noise envelope */

  if (h_sbr_data->domain_vec_noise[0] == 0) {
    FIXP_SGL noiseLevel = h_sbr_data->sbrNoiseFloorLevel[0];
    for (i = 1; i < nNfb; i++) {
      noiseLevel += h_sbr_data->sbrNoiseFloorLevel[i];
      h_sbr_data->sbrNoiseFloorLevel[i] = noiseLevel;
    }
  } else {
    for (i = 0; i < nNfb; i++) {
      h_sbr_data->sbrNoiseFloorLevel[i] += h_prev_data->prevNoiseLevel[i];
    }
  }

  /* If present, decode the second noise envelope
     Note:  nNoiseFloorEnvelopes can only be 1 or 2 */

  if (nNoiseFloorEnvelopes > 1) {
    if (h_sbr_data->domain_vec_noise[1] == 0) {
      FIXP_SGL noiseLevel = h_sbr_data->sbrNoiseFloorLevel[nNfb];
      for (i = nNfb + 1; i < 2 * nNfb; i++) {
        noiseLevel += h_sbr_data->sbrNoiseFloorLevel[i];
        h_sbr_data->sbrNoiseFloorLevel[i] = noiseLevel;
      }
    } else {
      for (i = 0; i < nNfb; i++) {
        h_sbr_data->sbrNoiseFloorLevel[i + nNfb] +=
            h_sbr_data->sbrNoiseFloorLevel[i];
      }
    }
  }

  limitNoiseLevels(hHeaderData, h_sbr_data);

  /* Update prevNoiseLevel with the last noise envelope */
  for (i = 0; i < nNfb; i++)
    h_prev_data->prevNoiseLevel[i] =
        h_sbr_data->sbrNoiseFloorLevel[i + nNfb * (nNoiseFloorEnvelopes - 1)];

  /* Requantize the noise floor levels in COUPLING_OFF-mode */
  if (!h_sbr_data->coupling) {
    int nf_e;

    for (i = 0; i < nNoiseFloorEnvelopes * nNfb; i++) {
      nf_e = 6 - (LONG)h_sbr_data->sbrNoiseFloorLevel[i] + 1 + NOISE_EXP_OFFSET;
      /* +1 to compensate for a mantissa of 0.5 instead of 1.0 */

      h_sbr_data->sbrNoiseFloorLevel[i] =
          (FIXP_SGL)(((LONG)FL2FXCONST_SGL(0.5f)) + /* mantissa */
                     (nf_e & MASK_E));              /* exponent */
    }
  }
}
