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

/*********************** MPEG surround decoder library *************************

   Author(s):

   Description: SAC Processing

*******************************************************************************/

/*!
  \file
  \brief  Polyphase Filterbank
*/

#ifndef SAC_PROCESS_H
#define SAC_PROCESS_H

#include "sac_dec.h"

void SpatialDecApplyPhase(spatialDec *self, FIXP_SGL alpha,
                          int lastSlotOfParamSet);

/**
 * \brief  Apply QMF Analysis Filterbank.
 *
 * Calculates qmf data on downmix input time data.
 * Delaylines will be applied if necessaray.
 *
 * \param self                A spatial decoder handle.
 * \param inData              Downmix channel time data as input.
 * \param ts                  Signals time slot offset for input buffer.
 * \param qmfReal             Downmix channel qmf output data.
 * \param qmfImag             Downmix channel qmf output data.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecQMFAnalysis(spatialDec *self, const PCM_MPS *inData,
                                   const INT ts, const INT bypassMode,
                                   FIXP_DBL **qmfReal, FIXP_DBL **qmfImag,
                                   const int numInputChannels);

/**
 * \brief  Feed spatial decoder with external qmf data.
 *
 * \param self                A spatial decoder handle.
 * \param qmfInDataReal       External qmf downmix data as input.
 * \param qmfInDataImag       External qmf downmix data as input.
 * \param ts                  Signals time slot in input buffer to process.
 * \param qmfReal             Downmix channel qmf output data.
 * \param qmfImag             Downmix channel qmf output data.
 * \param numInputChannels    Number of input channels. Might differ from
 * self->numInputChannels.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecFeedQMF(spatialDec *self, FIXP_DBL **qmfInDataReal,
                               FIXP_DBL **qmfInDataImag, const INT ts,
                               const INT bypassMode, FIXP_DBL **qmfReal,
                               FIXP_DBL **qmfImag, const INT numInputChannels);

/**
 * \brief  Apply Hybrdid Analysis Filterbank.
 *
 * Calculates hybrid data on downmix input data.
 * Residual hybrid signals will also be calculated on current slot if available.
 *
 * \param self                A spatial decoder handle.
 * \param qmfInputReal        Downmix channel qmf data as input.
 * \param qmfInputImag        Downmix channel qmf data as input.
 * \param hybOutputReal       Downmix channel hybrid output data.
 * \param hybOutputImag       Downmix channel hybrid output data.
 * \param ts                  Signals time slot in spatial frame to process.
 * \param numInputChannels    Number of input channels. Might differ from
 * self->numInputChannels.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecHybridAnalysis(spatialDec *self, FIXP_DBL **qmfInputReal,
                                      FIXP_DBL **qmfInputImag,
                                      FIXP_DBL **hybOutputReal,
                                      FIXP_DBL **hybOutputImag, const INT ts,
                                      const INT numInputChannels);

/**
 * \brief  Create X data.
 *
 * Returns a pointer list over Xchannels pointing to downmix input channels
 * and to residual channels when provided.
 *
 * \param self                A spatial decoder handle.
 * \param hybInputReal        Downmix channel hybrid data as input.
 * \param hybInputImag        Downmix channel hybrid data as input.
 * \param pxReal              Pointer to hybrid and residual data as output.
 * \param pxImag              Pointer to hybrid and residual data as output.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecCreateX(spatialDec *self, FIXP_DBL **hybInputReal,
                               FIXP_DBL **hybInputImag, FIXP_DBL **pxReal,
                               FIXP_DBL **pxImag);

/**
 * \brief  MPS212 combined version of apply M1 parameters and create wet signal
 *
 * \param self                A spatial decoder handle.
 * \param xReal               Downmix and residual X data as input.
 * \param xImag               Downmix and residual X data as input.
 * \param vReal               output data: [0] direct signal (V); [1] wet signal
 * (W).
 * \param vImag               output data: [0] direct signal (V); [1] wet signal
 * (W).
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecApplyM1_CreateW_Mode212(
    spatialDec *self, const SPATIAL_BS_FRAME *frame, FIXP_DBL **xReal,
    FIXP_DBL **xImag, FIXP_DBL **vReal, FIXP_DBL **vImag);

/**
 * \brief  Apply M2 parameters.
 *
 * \param self                A spatial decoder handle.
 * \param ps                  Signals parameter band from where M2 parameter to
 * use.
 * \param alpha               Smoothing factor between current and previous
 * parameter band. Rangeability between 0.f and 1.f.
 * \param wReal               Wet input data.
 * \param wImag               Wet input data.
 * \param hybOutputRealDry    Dry output data.
 * \param hybOutputImagDry    Dry output data.
 * \param hybOutputRealWet    Wet output data.
 * \param hybOutputImagWet    Wet output data.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecApplyM2(spatialDec *self, INT ps, const FIXP_SGL alpha,
                               FIXP_DBL **wReal, FIXP_DBL **wImag,
                               FIXP_DBL **hybOutputRealDry,
                               FIXP_DBL **hybOutputImagDry,
                               FIXP_DBL **hybOutputRealWet,
                               FIXP_DBL **hybOutputImagWet);

/**
 * \brief  Apply M2 parameter for 212 mode with residualCoding and phaseCoding.
 *
 * \param self                [i] A spatial decoder handle.
 * \param ps                  [i] Signals parameter band from where M2 parameter
 * to use.
 * \param alpha               [i] Smoothing factor between current and previous
 * parameter band. Rangeability between 0.f and 1.f.
 * \param wReal               [i] Wet input data.
 * \param wImag               [i] Wet input data.
 * \param hybOutputRealDry    [o] Dry output data.
 * \param hybOutputImagDry    [o] Dry output data.
 *
 * \return error
 */
SACDEC_ERROR SpatialDecApplyM2_Mode212_ResidualsPlusPhaseCoding(
    spatialDec *self, INT ps, const FIXP_SGL alpha, FIXP_DBL **wReal,
    FIXP_DBL **wImag, FIXP_DBL **hybOutputRealDry, FIXP_DBL **hybOutputImagDry);

/**
 * \brief  Apply M2 parameter for 212 mode, upmix from mono to stereo.
 *
 * \param self                [i] A spatial decoder handle.
 * \param ps                  [i] Signals parameter band from where M2 parameter
 * to use.
 * \param alpha               [i] Smoothing factor between current and previous
 * parameter band. Rangeability between 0.f and 1.f.
 * \param wReal               [i] Wet input data.
 * \param wImag               [i] Wet input data.
 * \param hybOutputRealDry    [o] Dry output data.
 * \param hybOutputImagDry    [o] Dry output data.
 *
 * \return error
 */
SACDEC_ERROR SpatialDecApplyM2_Mode212(spatialDec *self, INT ps,
                                       const FIXP_SGL alpha, FIXP_DBL **wReal,
                                       FIXP_DBL **wImag,
                                       FIXP_DBL **hybOutputRealDry,
                                       FIXP_DBL **hybOutputImagDry);

/**
 * \brief  Convert Hybrid input to output audio data.
 *
 * \param hSpaceSynthesisQmf  A spatial decoder handle.
 * \param ts                  Signals time slot in spatial frame to process.
 * \param hybOutputReal       Hybrid data as input.
 * \param hybOutputImag       Hybrid data as input.
 * \param timeOut             audio output data.
 *
 * \return  Error status.
 */
SACDEC_ERROR SpatialDecSynthesis(spatialDec *self, const INT ts,
                                 FIXP_DBL **hybOutputReal,
                                 FIXP_DBL **hybOutputImag, PCM_MPS *timeOut,
                                 const INT numInputChannels,
                                 const FDK_channelMapDescr *const mapDescr);

void SpatialDecBufferMatrices(spatialDec *self);

FIXP_DBL getChGain(spatialDec *self, UINT ch, INT *scale);

#endif
