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
  \brief  missing harmonics detection header file $Revision: 92790 $
*/

#ifndef MH_DET_H
#define MH_DET_H

#include "sbr_encoder.h"
#include "fram_gen.h"

typedef struct {
  FIXP_DBL thresHoldDiff;      /*!< threshold for tonality difference */
  FIXP_DBL thresHoldDiffGuide; /*!< threshold for tonality difference for the
                                  guide */
  FIXP_DBL thresHoldTone;      /*!< threshold for tonality for a sine */
  FIXP_DBL invThresHoldTone;
  FIXP_DBL thresHoldToneGuide; /*!< threshold for tonality for a sine for the
                                  guide */
  FIXP_DBL sfmThresSbr;    /*!< tonality flatness measure threshold for the SBR
                              signal.*/
  FIXP_DBL sfmThresOrig;   /*!< tonality flatness measure threshold for the
                              original signal.*/
  FIXP_DBL decayGuideOrig; /*!< decay value of the tonality value of the guide
                              for the tone. */
  FIXP_DBL decayGuideDiff; /*!< decay value of the tonality value of the guide
                              for the tonality difference. */
  FIXP_DBL derivThresMaxLD64;   /*!< threshold for detecting LP character in a
                                   signal. */
  FIXP_DBL derivThresBelowLD64; /*!< threshold for detecting LP character in a
                                   signal. */
  FIXP_DBL derivThresAboveLD64; /*!< threshold for detecting LP character in a
                                   signal. */
} THRES_HOLDS;

typedef struct {
  INT deltaTime; /*!< maximum allowed transient distance (from frame border in
                    number of qmf subband sample) for a frame to be considered a
                    transient frame.*/
  THRES_HOLDS thresHolds; /*!< the thresholds used for detection. */
  INT maxComp; /*!< maximum alllowed compensation factor for the envelope data.
                */
} DETECTOR_PARAMETERS_MH;

typedef struct {
  FIXP_DBL *guideVectorDiff;
  FIXP_DBL *guideVectorOrig;
  UCHAR *guideVectorDetected;
} GUIDE_VECTORS;

typedef struct {
  INT qmfNoChannels;
  INT nSfb;
  INT sampleFreq;
  INT previousTransientFlag;
  INT previousTransientFrame;
  INT previousTransientPos;

  INT noVecPerFrame;
  INT transientPosOffset;

  INT move;
  INT totNoEst;
  INT noEstPerFrame;
  INT timeSlots;

  UCHAR *guideScfb;
  UCHAR *prevEnvelopeCompensation;
  UCHAR *detectionVectors[MAX_NO_OF_ESTIMATES];
  FIXP_DBL tonalityDiff[MAX_NO_OF_ESTIMATES / 2][MAX_FREQ_COEFFS];
  FIXP_DBL sfmOrig[MAX_NO_OF_ESTIMATES / 2][MAX_FREQ_COEFFS];
  FIXP_DBL sfmSbr[MAX_NO_OF_ESTIMATES / 2][MAX_FREQ_COEFFS];
  const DETECTOR_PARAMETERS_MH *mhParams;
  GUIDE_VECTORS guideVectors[MAX_NO_OF_ESTIMATES];
} SBR_MISSING_HARMONICS_DETECTOR;

typedef SBR_MISSING_HARMONICS_DETECTOR *HANDLE_SBR_MISSING_HARMONICS_DETECTOR;

void FDKsbrEnc_SbrMissingHarmonicsDetectorQmf(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
    FIXP_DBL **pQuotaBuffer, INT **pSignBuffer, SCHAR *indexVector,
    const SBR_FRAME_INFO *pFrameInfo, const UCHAR *pTranInfo,
    INT *pAddHarmonicsFlag, UCHAR *pAddHarmonicsScaleFactorBands,
    const UCHAR *freqBandTable, INT nSfb, UCHAR *envelopeCompensation,
    FIXP_DBL *pNrgVector);

INT FDKsbrEnc_CreateSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet, INT chan);

INT FDKsbrEnc_InitSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
    INT sampleFreq, INT frameSize, INT nSfb, INT qmfNoChannels, INT totNoEst,
    INT move, INT noEstPerFrame, UINT sbrSyntaxFlags);

void FDKsbrEnc_DeleteSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector);

INT FDKsbrEnc_ResetSbrMissingHarmonicsDetector(
    HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMissingHarmonicsDetector,
    INT nSfb);

#endif
