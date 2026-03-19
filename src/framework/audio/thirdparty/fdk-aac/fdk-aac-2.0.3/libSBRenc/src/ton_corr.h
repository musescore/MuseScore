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
  \brief  General tonality correction detector module.
*/
#ifndef TON_CORR_H
#define TON_CORR_H

#include "sbr_encoder.h"
#include "mh_det.h"
#include "nf_est.h"
#include "invf_est.h"

#define MAX_NUM_PATCHES 6
#define SCALE_NRGVEC 4

/** parameter set for one single patch */
typedef struct {
  INT sourceStartBand; /*!< first band in lowbands where to take the samples
                          from */
  INT sourceStopBand;  /*!< first band in lowbands which is not included in the
                          patch anymore */
  INT guardStartBand;  /*!< first band in highbands to be filled with zeros in
                          order to  reduce interferences between patches */
  INT targetStartBand; /*!< first band in highbands to be filled with whitened
                          lowband signal */
  INT targetBandOffs;  /*!< difference between 'startTargetBand' and
                          'startSourceBand' */
  INT numBandsInPatch; /*!< number of consecutive bands in this one patch */
} PATCH_PARAM;

typedef struct {
  INT switchInverseFilt; /*!< Flag to enable dynamic adaption of invf. detection
                          */
  INT noQmfChannels;
  INT bufferLength;      /*!< Length of the r and i buffers. */
  INT stepSize;          /*!< Stride for the lpc estimate. */
  INT numberOfEstimates; /*!< The total number of estiamtes, available in the
                            quotaMatrix.*/
  UINT numberOfEstimatesPerFrame; /*!< The number of estimates per frame
                                     available in the quotaMatrix.*/
  INT lpcLength[2]; /*!< Segment length used for second order LPC analysis.*/
  INT nextSample;   /*!< Where to start the LPC analysis of the current frame.*/
  INT move; /*!< How many estimates to move in the quotaMatrix, when buffering.
             */
  INT frameStartIndex; /*!< The start index for the current frame in the r and i
                          buffers. */
  INT startIndexMatrix;       /*!< The start index for the current frame in the
                                 quotaMatrix. */
  INT frameStartIndexInvfEst; /*!< The start index of the inverse filtering, not
                                 the same as the others, dependent on what
                                 decoder is used (buffer opt, or no buffer opt).
                               */
  INT prevTransientFlag;  /*!< The transisent flag (from the transient detector)
                             for the previous frame. */
  INT transientNextFrame; /*!< Flag to indicate that the transient will show up
                             in the next frame. */
  INT transientPosOffset; /*!< An offset value to match the transient pos as
                             calculated by the transient detector with the
                             actual position in the frame.*/

  INT* signMatrix[MAX_NO_OF_ESTIMATES]; /*!< Matrix holding the sign of each
                                           channe, i.e. indicating in what part
                                           of a QMF channel a possible sine is.
                                         */

  FIXP_DBL* quotaMatrix[MAX_NO_OF_ESTIMATES]; /*!< Matrix holding the quota
                                                 values for all estimates, all
                                                 channels. */

  FIXP_DBL nrgVector[MAX_NO_OF_ESTIMATES]; /*!< Vector holding the averaged
                                              energies for every QMF band. */
  FIXP_DBL nrgVectorFreq[64]; /*!< Vector holding the averaged energies for
                                 every QMF channel */

  SCHAR indexVector[64]; /*!< Index vector poINTing to the correct lowband
                            channel, when indexing a highband channel, -1
                            represents a guard band */
  PATCH_PARAM
  patchParam[MAX_NUM_PATCHES]; /*!< new parameter set for patching */
  INT guard;                   /*!< number of guardbands between every patch */
  INT shiftStartSb; /*!< lowest subband of source range to be included in the
                       patches */
  INT noOfPatches;  /*!< number of patches */

  SBR_MISSING_HARMONICS_DETECTOR
  sbrMissingHarmonicsDetector; /*!< SBR_MISSING_HARMONICS_DETECTOR struct.
                                */
  SBR_NOISE_FLOOR_ESTIMATE
  sbrNoiseFloorEstimate;       /*!< SBR_NOISE_FLOOR_ESTIMATE struct. */
  SBR_INV_FILT_EST sbrInvFilt; /*!< SBR_INV_FILT_EST struct. */
} SBR_TON_CORR_EST;

typedef SBR_TON_CORR_EST* HANDLE_SBR_TON_CORR_EST;

void FDKsbrEnc_TonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    INVF_MODE* infVec, /*!< Vector where the inverse filtering levels will be
                          stored. */
    FIXP_DBL* noiseLevels, /*!< Vector where the noise levels will be stored. */
    INT* missingHarmonicFlag, /*!< Flag set to one or zero, dependent on if any
                                 strong sines are missing.*/
    UCHAR* missingHarmonicsIndex, /*!< Vector indicating where sines are
                                     missing. */
    UCHAR* envelopeCompensation,  /*!< Vector to store compensation values for
                                     the energies in. */
    const SBR_FRAME_INFO* frameInfo, /*!< Frame info struct, contains the time
                                        and frequency grid of the current
                                        frame.*/
    UCHAR* transientInfo,            /*!< Transient info.*/
    UCHAR* freqBandTable,            /*!< Frequency band tables for high-res.*/
    INT nSfb,           /*!< Number of scalefactor bands for high-res. */
    XPOS_MODE xposType, /*!< Type of transposer used in the decoder.*/
    UINT sbrSyntaxFlags);

INT FDKsbrEnc_CreateTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST
        hTonCorr, /*!< Pointer to handle to SBR_TON_CORR struct. */
    INT chan);    /*!< Channel index, needed for mem allocation */

INT FDKsbrEnc_InitTonCorrParamExtr(
    INT frameSize, /*!< Current SBR frame size. */
    HANDLE_SBR_TON_CORR_EST
        hTonCorr, /*!< Pointer to handle to SBR_TON_CORR struct. */
    HANDLE_SBR_CONFIG_DATA
        sbrCfg,           /*!< Pointer to SBR configuration parameters. */
    INT timeSlots,        /*!< Number of time-slots per frame */
    INT xposCtrl,         /*!< Different patch modes. */
    INT ana_max_level,    /*!< Maximum level of the adaptive noise. */
    INT noiseBands,       /*!< Number of noise bands per octave. */
    INT noiseFloorOffset, /*!< Noise floor offset. */
    UINT useSpeechConfig  /*!< Speech or music tuning. */
);

void FDKsbrEnc_DeleteTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr); /*!< Handle to SBR_TON_CORR struct. */

void FDKsbrEnc_CalculateTonalityQuotas(
    HANDLE_SBR_TON_CORR_EST hTonCorr, FIXP_DBL** sourceBufferReal,
    FIXP_DBL** sourceBufferImag, INT usb,
    INT qmfScale /*!< sclefactor of QMF subsamples */
);

INT FDKsbrEnc_ResetTonCorrParamExtr(
    HANDLE_SBR_TON_CORR_EST hTonCorr, /*!< Handle to SBR_TON_CORR struct. */
    INT xposctrl,                     /*!< Different patch modes. */
    INT highBandStartSb,              /*!< Start band of the SBR range. */
    UCHAR* v_k_master, /*!< Master frequency table from which all other table
                          are derived.*/
    INT numMaster,     /*!< Number of elements in the master table. */
    INT fs,            /*!< Sampling frequency (of the SBR part). */
    UCHAR**
        freqBandTable, /*!< Frequency band table for low-res and high-res. */
    INT* nSfb,         /*!< Number of frequency bands (hig-res and low-res). */
    INT noQmfChannels  /*!< Number of QMF channels. */
);
#endif
