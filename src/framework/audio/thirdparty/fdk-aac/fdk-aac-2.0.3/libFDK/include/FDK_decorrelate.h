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

/******************* Library for basic calculation routines ********************

   Author(s):   Markus Lohwasser

   Description: FDK Tools Decorrelator

*******************************************************************************/

#ifndef FDK_DECORRELATE_H
#define FDK_DECORRELATE_H

#include "common_fix.h"

#define FIXP_MPS FIXP_DBL

#ifndef ARCH_PREFER_MULT_32x32
#define FIXP_DECORR FIXP_SGL
#define FX_DECORR2FX_DBL FX_SGL2FX_DBL
#define FX_DECORR2FX_SGL
#define FX_DBL2FX_DECORR FX_DBL2FX_SGL
#define FX_SGL2FX_DECORR
#define DECORR(a) (FX_DBL2FXCONST_SGL(a))
#define FL2FXCONST_DECORR FL2FXCONST_SGL
#else
#define FIXP_DECORR FIXP_DBL
#define FX_DECORR2FX_DBL
#define FX_DECORR2FX_SGL FX_DBL2FX_SGL
#define FX_DBL2FX_DECORR
#define FX_SGL2FX_DECORR FX_SGL2FX_DBL
#define DECORR(a) FIXP_DBL(a)
#define FL2FXCONST_DECORR FL2FXCONST_DBL
#endif

/*--------------- enums -------------------------------*/

/**
 * Decorrelator types.
 */
typedef enum {
  DECORR_MPS,  /**< Decorrelator type used by MPS LP/HQ */
  DECORR_PS,   /**< Decorrelator type used by HEAACv2 and MPS LP */
  DECORR_USAC, /**< Decorrelator type used by USAC */
  DECORR_LD    /**< Decorrelator type used by MPS Low Delay */
} FDK_DECORR_TYPE;

/**
 * Ducker types.
 */
typedef enum {
  DUCKER_AUTOMATIC, /**< FDKdecorrelateInit() chooses correct ducker type
                       depending on provided parameters. */
  DUCKER_MPS,       /**< Force ducker type to MPS. */
  DUCKER_PS         /**< Force ducker type to PS. */
} FDK_DUCKER_TYPE;

/**
 * Reverb band types.
 */
typedef enum {
  NOT_EXIST, /**< Mark reverb band as non-existing (number of bands = 0). */
  DELAY, /**< Reverb bands just contains delay elements and no allpass filters.
          */
  COMMON_REAL,  /**< Real filter coeffs, common filter coeffs within one reverb
                   band */
  COMMON_CPLX,  /**< Complex filter coeffs, common filter coeffs within one
                   reverb band */
  INDEP_CPLX,   /**< Complex filter coeffs, independent filter coeffs for each
                   hybrid band */
  INDEP_CPLX_PS /**< PS optimized implementation of general INDEP_CPLX type */
} REVBAND_FILT_TYPE;

typedef struct DECORR_DEC *HANDLE_DECORR_DEC;

typedef struct DUCKER_INSTANCE {
  int hybridBands;
  int parameterBands;
  int partiallyComplex;
  FDK_DUCKER_TYPE duckerType;

  const UCHAR *qs_next;
  const UCHAR *mapProcBands2HybBands;
  const UCHAR *mapHybBands2ProcBands;
  /* interleaved     SmoothDirectNrg[] and SmoothReverbNrg[],
     non-interleaved SmoothDirectNrg[] in case of parametric stereo */
  FIXP_MPS SmoothDirRevNrg[2 * (28)];

  /*
    parametric stereo
  */
  FIXP_MPS peakDecay[(28)];
  FIXP_MPS peakDiff[(28)];
  FIXP_DBL maxValDirectData;
  FIXP_DBL maxValReverbData;
  SCHAR scaleDirectNrg;
  SCHAR scaleReverbNrg;
  SCHAR scaleSmoothDirRevNrg;
  SCHAR headroomSmoothDirRevNrg;

} DUCKER_INSTANCE;

typedef struct DECORR_FILTER_INSTANCE {
  FIXP_MPS *stateCplx;
  FIXP_DBL *DelayBufferCplx;

  const FIXP_DECORR *numeratorReal;
  const FIXP_STP *coeffsPacked;
  const FIXP_DECORR *denominatorReal;
} DECORR_FILTER_INSTANCE;

typedef struct DECORR_DEC {
  INT L_stateBufferCplx;
  FIXP_DBL *stateBufferCplx;
  INT L_delayBufferCplx;
  FIXP_DBL *delayBufferCplx;

  const REVBAND_FILT_TYPE *REV_filtType;
  const UCHAR *REV_bandOffset;
  const UCHAR *REV_delay;
  const SCHAR *REV_filterOrder;
  INT reverbBandDelayBufferIndex[(4)];
  UCHAR stateBufferOffset[(3)];

  DECORR_FILTER_INSTANCE Filter[(71)];
  DUCKER_INSTANCE ducker;

  int numbins;
  int partiallyComplex;
} DECORR_DEC;

/**
 * \brief  Create one instance of Decorrelator.
 *
 * \param hDecorrDec          A pointer to a decorrelator instance which was
 * allocated externally.
 * \param bufferCplx          Externally allocated buffer (allocate (2*( ( 825 )
 * + ( 373 ) )) FIXP_DBL values).
 * \param bufLen              Length of bufferCplx. Must be >= (2*( ( 825 ) + (
 * 373 ) )).
 *
 * \return  0 on success.
 */
INT FDKdecorrelateOpen(HANDLE_DECORR_DEC hDecorrDec, FIXP_DBL *bufferCplx,
                       const INT bufLen);

/**
 * \brief  Initialize and configure Decorrelator instance.
 *
 * \param hDecorrDec          A Decorrelator handle.
 * \param nrHybBands          Number of (hybrid) bands.
 * \param decorrType          Decorrelator type to use.
 * \param duckerType          Ducker type to use (in general use
 * DUCKER_AUTOMATIC).
 * \param decorrConfig        Depending on decorrType values of 0,1,2 are
 * allowed.
 * \param seed                Seed of decorrelator instance. Allowed maximum
 * valued depends on decorrType.
 * \param partiallyComplex    Low power or high quality processing 0: HQ, 1: LQ
 * (only allowed for DECORR_MPS | DECORR_PS).
 * \param useFractDelay       Indicate usage of fractional delay 0: off, 1: on
 * (currently not supported).
 * \param isLegacyPS          Indicate if DECORR_PS is used for HEAACv2 (for all
 * other cases: isLegacyPS = 0). The purpose of this parameter is to select the
 * correct number of param bands for the ducker.
 * \param initStatesFlag      Indicates whether the states buffer has to be
 * cleared.
 *
 * \return  0 on success.
 */
INT FDKdecorrelateInit(HANDLE_DECORR_DEC hDecorrDec, const INT nrHybBands,
                       const FDK_DECORR_TYPE decorrType,
                       const FDK_DUCKER_TYPE duckerType, const INT decorrConfig,
                       const INT seed, const INT partiallyComplex,
                       const INT useFractDelay, const INT isLegacyPS,
                       const INT initStatesFlag);

/**
 * \brief  Apply Decorrelator on input data.
 *
 * Function applies decorrelator and ducker inplace on hybrid input data.
 * Modified hybrid data will be returned inplace.
 *
 * \param hDecorrDec          A decorrelator handle.
 * \param dataRealIn          In (hybrid) data.
 * \param dataImagIn          In (hybrid) data.
 * \param dataRealOut         Out (hybrid) data (can be same as dataRealIn for
 * in-place calculation).
 * \param dataImagOut         Out (hybrid) data (can be same as dataImagIn for
 * in-place calculation).
 * \param startHybBand        Hybrid band to start with decorrelation.
 *
 * \return  0 on success.
 */
INT FDKdecorrelateApply(HANDLE_DECORR_DEC hDecorrDec, FIXP_DBL *dataRealIn,
                        FIXP_DBL *dataImagIn, FIXP_DBL *dataRealOut,
                        FIXP_DBL *dataImagOut, const INT startHybBand);

/**
 * \brief  Destroy a Decorrelator instance.
 *
 * Deallocate whole memory of decorraltor and inside ducker.
 *
 * \param hDecorrDec  Pointer to a decoderrolator handle. Null initialized on
 * return.
 *
 * \return  0 on success.
 */
INT FDKdecorrelateClose(HANDLE_DECORR_DEC hDecorrDec);

/**
 * \brief  Get max value address of direct signal.
 *
 * Get max value address of direct signal needed for ducker energy calculation.
 *
 * \param hDecorrDec  Pointer to a decoderrolator handle.
 *
 * \return  address of max value
 */
FIXP_DBL *getAddrDirectSignalMaxVal(HANDLE_DECORR_DEC hDecorrDec);

#endif /* FDK_DECORRELATE_H */
