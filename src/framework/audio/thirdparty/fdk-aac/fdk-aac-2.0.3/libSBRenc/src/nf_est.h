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
  \brief  Noise floor estimation structs and prototypes $Revision: 92790 $
*/

#ifndef NF_EST_H
#define NF_EST_H

#include "sbr_encoder.h"
#include "fram_gen.h"

#define NF_SMOOTHING_LENGTH 4 /*!< Smoothing length of the noise floors. */

typedef struct {
  FIXP_DBL
  prevNoiseLevels[NF_SMOOTHING_LENGTH]
                 [MAX_NUM_NOISE_VALUES]; /*!< The previous noise levels. */
  FIXP_DBL noiseFloorOffset
      [MAX_NUM_NOISE_VALUES];   /*!< Noise floor offset, scaled with
                                   NOISE_FLOOR_OFFSET_SCALING */
  const FIXP_DBL *smoothFilter; /*!< Smoothing filter to use. */
  FIXP_DBL ana_max_level;       /*!< Max level allowed.   */
  FIXP_DBL weightFac; /*!< Weightening factor for the difference between orig
                         and sbr. */
  INT freqBandTableQmf[MAX_NUM_NOISE_VALUES +
                       1]; /*!< Frequncy band table for the noise floor bands.*/
  INT noNoiseBands;        /*!< Number of noisebands. */
  INT noiseBands;          /*!< NoiseBands switch 4 bit.*/
  INT timeSlots;           /*!< Number of timeslots in a frame. */
  INVF_MODE diffThres;     /*!< Threshold value to control the inverse filtering
                              decision */
} SBR_NOISE_FLOOR_ESTIMATE;

typedef SBR_NOISE_FLOOR_ESTIMATE *HANDLE_SBR_NOISE_FLOOR_ESTIMATE;

void FDKsbrEnc_sbrNoiseFloorEstimateQmf(
    HANDLE_SBR_NOISE_FLOOR_ESTIMATE
        h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct
                                  */
    const SBR_FRAME_INFO
        *frame_info, /*!< Time frequency grid of the current frame. */
    FIXP_DBL
        *noiseLevels, /*!< Pointer to vector to store the noise levels in.*/
    FIXP_DBL **quotaMatrixOrig, /*!< Matrix holding the quota values of the
                                   original. */
    SCHAR *indexVector,         /*!< Index vector to obtain the patched data. */
    INT missingHarmonicsFlag,   /*!< Flag indicating if a strong tonal component
                                   will be missing. */
    INT startIndex,             /*!< Start index. */
    UINT numberOfEstimatesPerFrame, /*!< The number of tonality estimates per
                                       frame. */
    INT transientFrame, /*!< A flag indicating if a transient is present. */
    INVF_MODE *pInvFiltLevels, /*!< Pointer to the vector holding the inverse
                                  filtering levels. */
    UINT sbrSyntaxFlags);

INT FDKsbrEnc_InitSbrNoiseFloorEstimate(
    HANDLE_SBR_NOISE_FLOOR_ESTIMATE
        h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct
                                  */
    INT ana_max_level,           /*!< Maximum level of the adaptive noise. */
    const UCHAR *freqBandTable,  /*!< Frequany band table. */
    INT nSfb,                    /*!< Number of frequency bands. */
    INT noiseBands,              /*!< Number of noise bands per octave. */
    INT noiseFloorOffset,        /*!< Noise floor offset. */
    INT timeSlots,               /*!< Number of time slots in a frame. */
    UINT useSpeechConfig /*!< Flag: adapt tuning parameters according to speech
                          */
);

INT FDKsbrEnc_resetSbrNoiseFloorEstimate(
    HANDLE_SBR_NOISE_FLOOR_ESTIMATE
        h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct
                                  */
    const UCHAR *freqBandTable,  /*!< Frequany band table. */
    INT nSfb); /*!< Number of bands in the frequency band table. */

void FDKsbrEnc_deleteSbrNoiseFloorEstimate(
    HANDLE_SBR_NOISE_FLOOR_ESTIMATE
        h_sbrNoiseFloorEstimate); /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct
                                   */

#endif
