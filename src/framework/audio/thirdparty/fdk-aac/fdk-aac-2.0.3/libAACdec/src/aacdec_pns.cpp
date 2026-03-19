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

   Author(s):   Josef Hoepfl

   Description: perceptual noise substitution tool

*******************************************************************************/

#include "aacdec_pns.h"

#include "aac_ram.h"
#include "aac_rom.h"
#include "channelinfo.h"
#include "block.h"
#include "FDK_bitstream.h"

#include "genericStds.h"

#define NOISE_OFFSET 90 /* cf. ISO 14496-3 p. 175 */

/*!
  \brief Reset InterChannel and PNS data

  The function resets the InterChannel and PNS data
*/
void CPns_ResetData(CPnsData *pPnsData,
                    CPnsInterChannelData *pPnsInterChannelData) {
  FDK_ASSERT(pPnsData != NULL);
  FDK_ASSERT(pPnsInterChannelData != NULL);
  /* Assign pointer always, since pPnsData is not persistent data */
  pPnsData->pPnsInterChannelData = pPnsInterChannelData;
  pPnsData->PnsActive = 0;
  pPnsData->CurrentEnergy = 0;

  FDKmemclear(pPnsData->pnsUsed, (8 * 16) * sizeof(UCHAR));
  FDKmemclear(pPnsInterChannelData->correlated, (8 * 16) * sizeof(UCHAR));
}

/*!
  \brief Update PNS noise generator state.

  The function sets the seed for PNS noise generation.
  It can be used to link two or more channels in terms of PNS.
*/
void CPns_UpdateNoiseState(CPnsData *pPnsData, INT *currentSeed,
                           INT *randomSeed) {
  /* use pointer because seed has to be
     same, left and right channel ! */
  pPnsData->currentSeed = currentSeed;
  pPnsData->randomSeed = randomSeed;
}

/*!
  \brief Indicates if PNS is used

  The function returns a value indicating whether PNS is used or not
  acordding to the noise energy

  \return  PNS used
*/
int CPns_IsPnsUsed(const CPnsData *pPnsData, const int group, const int band) {
  unsigned pns_band = group * 16 + band;

  return pPnsData->pnsUsed[pns_band] & (UCHAR)1;
}

/*!
  \brief Set correlation

  The function activates the noise correlation between the channel pair
*/
void CPns_SetCorrelation(CPnsData *pPnsData, const int group, const int band,
                         const int outofphase) {
  CPnsInterChannelData *pInterChannelData = pPnsData->pPnsInterChannelData;
  unsigned pns_band = group * 16 + band;

  pInterChannelData->correlated[pns_band] = (outofphase) ? 3 : 1;
}

/*!
  \brief Indicates if correlation is used

  The function indicates if the noise correlation between the channel pair
  is activated

  \return  PNS is correlated
*/
static int CPns_IsCorrelated(const CPnsData *pPnsData, const int group,
                             const int band) {
  CPnsInterChannelData *pInterChannelData = pPnsData->pPnsInterChannelData;
  unsigned pns_band = group * 16 + band;

  return (pInterChannelData->correlated[pns_band] & 0x01) ? 1 : 0;
}

/*!
  \brief Indicates if correlated out of phase mode is used.

  The function indicates if the noise correlation between the channel pair
  is activated in out-of-phase mode.

  \return  PNS is out-of-phase
*/
static int CPns_IsOutOfPhase(const CPnsData *pPnsData, const int group,
                             const int band) {
  CPnsInterChannelData *pInterChannelData = pPnsData->pPnsInterChannelData;
  unsigned pns_band = group * 16 + band;

  return (pInterChannelData->correlated[pns_band] & 0x02) ? 1 : 0;
}

/*!
  \brief Read PNS information

  The function reads the PNS information from the bitstream
*/
void CPns_Read(CPnsData *pPnsData, HANDLE_FDK_BITSTREAM bs,
               const CodeBookDescription *hcb, SHORT *pScaleFactor,
               UCHAR global_gain, int band, int group /* = 0 */) {
  int delta;
  UINT pns_band = group * 16 + band;

  if (pPnsData->PnsActive) {
    /* Next PNS band case */
    delta = CBlock_DecodeHuffmanWord(bs, hcb) - 60;
  } else {
    /* First PNS band case */
    int noiseStartValue = FDKreadBits(bs, 9);

    delta = noiseStartValue - 256;
    pPnsData->PnsActive = 1;
    pPnsData->CurrentEnergy = global_gain - NOISE_OFFSET;
  }

  pPnsData->CurrentEnergy += delta;
  pScaleFactor[pns_band] = pPnsData->CurrentEnergy;

  pPnsData->pnsUsed[pns_band] = 1;
}

/**
 * \brief Generate a vector of noise of given length. The noise values are
 *        scaled in order to yield a noise energy of 1.0
 * \param spec pointer to were the noise values will be written to.
 * \param size amount of noise values to be generated.
 * \param pRandomState pointer to the state of the random generator being used.
 * \return exponent of generated noise vector.
 */
static int GenerateRandomVector(FIXP_DBL *RESTRICT spec, int size,
                                int *pRandomState) {
  int i, invNrg_e = 0, nrg_e = 0;
  FIXP_DBL invNrg_m, nrg_m = FL2FXCONST_DBL(0.0f);
  FIXP_DBL *RESTRICT ptr = spec;
  int randomState = *pRandomState;

#define GEN_NOISE_NRG_SCALE 7

  /* Generate noise and calculate energy. */
  for (i = 0; i < size; i++) {
    randomState =
        (((INT64)1664525 * randomState) + (INT64)1013904223) & 0xFFFFFFFF;
    nrg_m = fPow2AddDiv2(nrg_m, (FIXP_DBL)randomState >> GEN_NOISE_NRG_SCALE);
    *ptr++ = (FIXP_DBL)randomState;
  }
  nrg_e = GEN_NOISE_NRG_SCALE * 2 + 1;

  /* weight noise with = 1 / sqrt_nrg; */
  invNrg_m = invSqrtNorm2(nrg_m << 1, &invNrg_e);
  invNrg_e += -((nrg_e - 1) >> 1);

  for (i = size; i--;) {
    spec[i] = fMult(spec[i], invNrg_m);
  }

  /* Store random state */
  *pRandomState = randomState;

  return invNrg_e;
}

static void ScaleBand(FIXP_DBL *RESTRICT spec, int size, int scaleFactor,
                      int specScale, int noise_e, int out_of_phase) {
  int i, shift, sfExponent;
  FIXP_DBL sfMatissa;

  /* Get gain from scale factor value = 2^(scaleFactor * 0.25) */
  sfMatissa = MantissaTable[scaleFactor & 0x03][0];
  /* sfExponent = (scaleFactor >> 2) + ExponentTable[scaleFactor & 0x03][0]; */
  /* Note:  ExponentTable[scaleFactor & 0x03][0] is always 1. */
  sfExponent = (scaleFactor >> 2) + 1;

  if (out_of_phase != 0) {
    sfMatissa = -sfMatissa;
  }

  /* +1 because of fMultDiv2 below. */
  shift = sfExponent - specScale + 1 + noise_e;

  /* Apply gain to noise values */
  if (shift >= 0) {
    shift = fixMin(shift, DFRACT_BITS - 1);
    for (i = size; i-- != 0;) {
      spec[i] = fMultDiv2(spec[i], sfMatissa) << shift;
    }
  } else {
    shift = fixMin(-shift, DFRACT_BITS - 1);
    for (i = size; i-- != 0;) {
      spec[i] = fMultDiv2(spec[i], sfMatissa) >> shift;
    }
  }
}

/*!
  \brief Apply PNS

  The function applies PNS (i.e. it generates noise) on the bands
  flagged as noisy bands

*/
void CPns_Apply(const CPnsData *pPnsData, const CIcsInfo *pIcsInfo,
                SPECTRAL_PTR pSpectrum, const SHORT *pSpecScale,
                const SHORT *pScaleFactor,
                const SamplingRateInfo *pSamplingRateInfo,
                const INT granuleLength, const int channel) {
  if (pPnsData->PnsActive) {
    const short *BandOffsets =
        GetScaleFactorBandOffsets(pIcsInfo, pSamplingRateInfo);

    int ScaleFactorBandsTransmitted = GetScaleFactorBandsTransmitted(pIcsInfo);

    for (int window = 0, group = 0; group < GetWindowGroups(pIcsInfo);
         group++) {
      for (int groupwin = 0; groupwin < GetWindowGroupLength(pIcsInfo, group);
           groupwin++, window++) {
        FIXP_DBL *spectrum = SPEC(pSpectrum, window, granuleLength);

        for (int band = 0; band < ScaleFactorBandsTransmitted; band++) {
          if (CPns_IsPnsUsed(pPnsData, group, band)) {
            UINT pns_band = window * 16 + band;

            int bandWidth = BandOffsets[band + 1] - BandOffsets[band];
            int noise_e;

            FDK_ASSERT(bandWidth >= 0);

            if (channel > 0 && CPns_IsCorrelated(pPnsData, group, band)) {
              noise_e =
                  GenerateRandomVector(spectrum + BandOffsets[band], bandWidth,
                                       &pPnsData->randomSeed[pns_band]);
            } else {
              pPnsData->randomSeed[pns_band] = *pPnsData->currentSeed;

              noise_e = GenerateRandomVector(spectrum + BandOffsets[band],
                                             bandWidth, pPnsData->currentSeed);
            }

            int outOfPhase = CPns_IsOutOfPhase(pPnsData, group, band);

            ScaleBand(spectrum + BandOffsets[band], bandWidth,
                      pScaleFactor[group * 16 + band], pSpecScale[window],
                      noise_e, outOfPhase);
          }
        }
      }
    }
  }
}
