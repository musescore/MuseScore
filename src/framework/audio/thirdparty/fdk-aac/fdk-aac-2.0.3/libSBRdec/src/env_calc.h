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
  \brief  Envelope calculation prototypes
*/
#ifndef ENV_CALC_H
#define ENV_CALC_H

#include "sbrdecoder.h"
#include "env_extr.h" /* for HANDLE_SBR_HEADER_DATA */

typedef struct {
  FIXP_DBL filtBuffer[MAX_FREQ_COEFFS];      /*!< previous gains (required for
                                                smoothing) */
  FIXP_DBL filtBufferNoise[MAX_FREQ_COEFFS]; /*!< previous noise levels
                                                (required for smoothing) */
  SCHAR filtBuffer_e[MAX_FREQ_COEFFS];       /*!< Exponents of previous gains */
  SCHAR filtBufferNoise_e; /*!< Common exponent of previous noise levels */

  int startUp;     /*!< flag to signal initial conditions in buffers */
  int phaseIndex;  /*!< Index for randomPase array */
  int prevTranEnv; /*!< The transient envelope of the previous frame. */

  ULONG harmFlagsPrev[ADD_HARMONICS_FLAGS_SIZE];
  /*!< Words with 16 flags each indicating where a sine was added in the
   * previous frame.*/
  UCHAR harmIndex;     /*!< Current phase of synthetic sine */
  int sbrPatchingMode; /*!< Current patching mode           */

  FIXP_SGL prevSbrNoiseFloorLevel[MAX_NOISE_COEFFS];
  UCHAR prevNNfb;
  UCHAR prevNSfb[2];
  UCHAR prevLoSubband;
  UCHAR prevHiSubband;
  UCHAR prev_ov_highSubband;
  UCHAR *prevFreqBandTable[2];
  UCHAR prevFreqBandTableLo[MAX_FREQ_COEFFS / 2 + 1];
  UCHAR prevFreqBandTableHi[MAX_FREQ_COEFFS + 1];
  UCHAR prevFreqBandTableNoise[MAX_NOISE_COEFFS + 1];
  SCHAR sinusoidal_positionPrev;
  ULONG harmFlagsPrevActive[ADD_HARMONICS_FLAGS_SIZE];
} SBR_CALCULATE_ENVELOPE;

typedef SBR_CALCULATE_ENVELOPE *HANDLE_SBR_CALCULATE_ENVELOPE;

void calculateSbrEnvelope(
    QMF_SCALE_FACTOR *sbrScaleFactor,
    HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env,
    HANDLE_SBR_HEADER_DATA hHeaderData, HANDLE_SBR_FRAME_DATA hFrameData,
    PVC_DYNAMIC_DATA *pPvcDynamicData, FIXP_DBL **analysBufferReal,
    FIXP_DBL *
        *analysBufferImag, /*!< Imag part of subband samples to be processed */
    const int useLP,
    FIXP_DBL *degreeAlias, /*!< Estimated aliasing for each QMF channel */
    const UINT flags, const int frameErrorFlag);

SBR_ERROR
createSbrEnvelopeCalc(HANDLE_SBR_CALCULATE_ENVELOPE hSbrCalculateEnvelope,
                      HANDLE_SBR_HEADER_DATA hHeaderData, const int chan,
                      const UINT flags);

int deleteSbrEnvelopeCalc(HANDLE_SBR_CALCULATE_ENVELOPE hSbrCalculateEnvelope);

void resetSbrEnvelopeCalc(HANDLE_SBR_CALCULATE_ENVELOPE hCalEnv);

SBR_ERROR
ResetLimiterBands(UCHAR *limiterBandTable, UCHAR *noLimiterBands,
                  UCHAR *freqBandTable, int noFreqBands,
                  const PATCH_PARAM *patchParam, int noPatches,
                  int limiterBands, UCHAR sbrPatchingMode,
                  int xOverQmf[MAX_NUM_PATCHES], int sbrRatio);

void rescaleSubbandSamples(FIXP_DBL **re, FIXP_DBL **im, int lowSubband,
                           int noSubbands, int start_pos, int next_pos,
                           int shift);

FIXP_DBL maxSubbandSample(FIXP_DBL **analysBufferReal_m,
                          FIXP_DBL **analysBufferImag_m, int lowSubband,
                          int highSubband, int start_pos, int stop_pos);

#endif  // ENV_CALC_H
