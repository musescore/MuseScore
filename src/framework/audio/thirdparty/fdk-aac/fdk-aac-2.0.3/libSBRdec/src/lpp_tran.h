/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
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
  \brief  Low Power Profile Transposer
*/

#ifndef LPP_TRAN_H
#define LPP_TRAN_H

#include "sbrdecoder.h"
#include "hbe.h"
#include "qmf.h"

/*
  Common
*/
#define QMF_OUT_SCALE 8

/*
  Frequency scales
*/

/*
  Env-Adjust
*/
#define MAX_NOISE_ENVELOPES 2
#define MAX_NOISE_COEFFS 5
#define MAX_NUM_NOISE_VALUES (MAX_NOISE_ENVELOPES * MAX_NOISE_COEFFS)
#define MAX_NUM_LIMITERS 12

/* Set MAX_ENVELOPES to the largest value of all supported BSFORMATs
   by overriding MAX_ENVELOPES in the correct order: */
#define MAX_ENVELOPES_LEGACY 5
#define MAX_ENVELOPES_USAC 8
#define MAX_ENVELOPES MAX_ENVELOPES_USAC

#define MAX_FREQ_COEFFS_DUAL_RATE 48
#define MAX_FREQ_COEFFS_QUAD_RATE 56
#define MAX_FREQ_COEFFS MAX_FREQ_COEFFS_QUAD_RATE

#define MAX_FREQ_COEFFS_FS44100 35
#define MAX_FREQ_COEFFS_FS48000 32

#define MAX_NUM_ENVELOPE_VALUES (MAX_ENVELOPES * MAX_FREQ_COEFFS)

#define MAX_GAIN_EXP 34
/* Maximum gain will be sqrt(0.5 * 2^MAX_GAIN_EXP)
   example: 34=99dB   */
#define MAX_GAIN_CONCEAL_EXP 1
/* Maximum gain will be sqrt(0.5 * 2^MAX_GAIN_CONCEAL_EXP) in concealment case
 * (0dB) */

/*
  LPP Transposer
*/
#define LPC_ORDER 2

#define MAX_INVF_BANDS MAX_NOISE_COEFFS

#define MAX_NUM_PATCHES 6
#define SHIFT_START_SB 1 /*!< lowest subband of source range */

typedef enum {
  INVF_OFF = 0,
  INVF_LOW_LEVEL,
  INVF_MID_LEVEL,
  INVF_HIGH_LEVEL,
  INVF_SWITCHED /* not a real choice but used here to control behaviour */
} INVF_MODE;

/** parameter set for one single patch */
typedef struct {
  UCHAR sourceStartBand; /*!< first band in lowbands where to take the samples
                            from */
  UCHAR
  sourceStopBand;       /*!< first band in lowbands which is not included in the
                           patch anymore */
  UCHAR guardStartBand; /*!< first band in highbands to be filled with zeros in
                           order to reduce interferences between patches */
  UCHAR
  targetStartBand;       /*!< first band in highbands to be filled with whitened
                            lowband signal */
  UCHAR targetBandOffs;  /*!< difference between 'startTargetBand' and
                            'startSourceBand' */
  UCHAR numBandsInPatch; /*!< number of consecutive bands in this one patch */
} PATCH_PARAM;

/** whitening factors for different levels of whitening
    need to be initialized corresponding to crossover frequency */
typedef struct {
  FIXP_DBL off; /*!< bw factor for signal OFF */
  FIXP_DBL transitionLevel;
  FIXP_DBL lowLevel;  /*!< bw factor for signal LOW_LEVEL */
  FIXP_DBL midLevel;  /*!< bw factor for signal MID_LEVEL */
  FIXP_DBL highLevel; /*!< bw factor for signal HIGH_LEVEL */
} WHITENING_FACTORS;

/*! The transposer settings are calculated on a header reset and are shared by
 * both channels. */
typedef struct {
  UCHAR nCols;           /*!< number subsamples of a codec frame */
  UCHAR noOfPatches;     /*!< number of patches */
  UCHAR lbStartPatching; /*!< first band of lowbands that will be patched */
  UCHAR lbStopPatching;  /*!< first band that won't be patched anymore*/
  UCHAR bwBorders[MAX_NUM_NOISE_VALUES]; /*!< spectral bands with different
                                            inverse filtering levels */

  PATCH_PARAM
  patchParam[MAX_NUM_PATCHES + 1]; /*!< new parameter set for patching */
  WHITENING_FACTORS
  whFactors;     /*!< the pole moving factors for certain
                    whitening levels as indicated     in the bitstream
                    depending on the crossover frequency */
  UCHAR overlap; /*!< Overlap size */
} TRANSPOSER_SETTINGS;

typedef struct {
  TRANSPOSER_SETTINGS *pSettings; /*!< Common settings for both channels */
  FIXP_DBL
  bwVectorOld[MAX_NUM_PATCHES]; /*!< pole moving factors of past frame */
  FIXP_DBL lpcFilterStatesRealLegSBR[LPC_ORDER + (3 * (4))][(
      32)]; /*!< pointer array to save filter states */

  FIXP_DBL lpcFilterStatesImagLegSBR[LPC_ORDER + (3 * (4))][(
      32)]; /*!< pointer array to save filter states */

  FIXP_DBL lpcFilterStatesRealHBE[LPC_ORDER + (3 * (4))][(
      64)]; /*!< pointer array to save filter states */
  FIXP_DBL lpcFilterStatesImagHBE[LPC_ORDER + (3 * (4))][(
      64)]; /*!< pointer array to save filter states */
} SBR_LPP_TRANS;

typedef SBR_LPP_TRANS *HANDLE_SBR_LPP_TRANS;

void lppTransposer(HANDLE_SBR_LPP_TRANS hLppTrans,
                   QMF_SCALE_FACTOR *sbrScaleFactor, FIXP_DBL **qmfBufferReal,

                   FIXP_DBL *degreeAlias, FIXP_DBL **qmfBufferImag,
                   const int useLP, const int fPreWhitening,
                   const int v_k_master0, const int timeStep,
                   const int firstSlotOffset, const int lastSlotOffset,
                   const int nInvfBands, INVF_MODE *sbr_invf_mode,
                   INVF_MODE *sbr_invf_mode_prev);

void lppTransposerHBE(
    HANDLE_SBR_LPP_TRANS hLppTrans, /*!< Handle of lpp transposer  */
    HANDLE_HBE_TRANSPOSER hQmfTransposer,
    QMF_SCALE_FACTOR *sbrScaleFactor, /*!< Scaling factors */
    FIXP_DBL **qmfBufferReal, /*!< Pointer to pointer to real part of subband
                                 samples (source) */
    FIXP_DBL **qmfBufferImag, /*!< Pointer to pointer to imaginary part of
                                 subband samples (source) */
    const int timeStep,       /*!< Time step of envelope */
    const int firstSlotOffs,  /*!< Start position in time */
    const int lastSlotOffs,   /*!< Number of overlap-slots into next frame */
    const int nInvfBands,     /*!< Number of bands for inverse filtering */
    INVF_MODE *sbr_invf_mode, /*!< Current inverse filtering modes */
    INVF_MODE *sbr_invf_mode_prev /*!< Previous inverse filtering modes */
);

SBR_ERROR
createLppTransposer(HANDLE_SBR_LPP_TRANS hLppTrans,
                    TRANSPOSER_SETTINGS *pSettings, const int highBandStartSb,
                    UCHAR *v_k_master, const int numMaster, const int usb,
                    const int timeSlots, const int nCols, UCHAR *noiseBandTable,
                    const int noNoiseBands, UINT fs, const int chan,
                    const int overlap);

SBR_ERROR
resetLppTransposer(HANDLE_SBR_LPP_TRANS hLppTrans, UCHAR highBandStartSb,
                   UCHAR *v_k_master, UCHAR numMaster, UCHAR *noiseBandTable,
                   UCHAR noNoiseBands, UCHAR usb, UINT fs);

#endif /* LPP_TRAN_H */
