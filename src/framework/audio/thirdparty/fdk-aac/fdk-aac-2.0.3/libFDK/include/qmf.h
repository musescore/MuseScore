/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file   qmf.h
  \brief  Complex qmf analysis/synthesis
  \author Markus Werner

*/

#ifndef QMF_H
#define QMF_H

#include "common_fix.h"
#include "FDK_tools_rom.h"
#include "dct.h"

#define FIXP_QAS FIXP_PCM
#define QAS_BITS SAMPLE_BITS
#define INT_PCM_QMFIN INT_PCM

#define FIXP_QSS FIXP_DBL
#define QSS_BITS DFRACT_BITS

/* Flags for QMF intialization */
/* Low Power mode flag */
#define QMF_FLAG_LP 1
/* Filter is not symmetric. This flag is set internally in the QMF
 * initialization as required. */
/* DO NOT PASS THIS FLAG TO qmfInitAnalysisFilterBank or
 * qmfInitSynthesisFilterBank */
#define QMF_FLAG_NONSYMMETRIC 2
/* Complex Low Delay Filter Bank (or std symmetric filter bank) */
#define QMF_FLAG_CLDFB 4
/* Flag indicating that the states should be kept. */
#define QMF_FLAG_KEEP_STATES 8
/* Complex Low Delay Filter Bank used in MPEG Surround Encoder */
#define QMF_FLAG_MPSLDFB 16
/* Complex Low Delay Filter Bank used in MPEG Surround Encoder allows a
 * optimized calculation of the modulation in qmfForwardModulationHQ() */
#define QMF_FLAG_MPSLDFB_OPTIMIZE_MODULATION 32
/* Flag to indicate HE-AAC down-sampled SBR mode (decoder) -> adapt analysis
 * post twiddling */
#define QMF_FLAG_DOWNSAMPLED 64

#define QMF_MAX_SYNTHESIS_BANDS (64)

/*!
 * \brief Algorithmic scaling in sbrForwardModulation()
 *
 * The scaling in sbrForwardModulation() is caused by:
 *
 *   \li 1 R_SHIFT in sbrForwardModulation()
 *   \li 5/6 R_SHIFT in dct3() if using 32/64 Bands
 *   \li 1 omitted gain of 2.0 in qmfForwardModulation()
 */
#define ALGORITHMIC_SCALING_IN_ANALYSIS_FILTERBANK 7

/*!
 * \brief Algorithmic scaling in cplxSynthesisQmfFiltering()
 *
 * The scaling in cplxSynthesisQmfFiltering() is caused by:
 *
 *   \li  5/6 R_SHIFT in dct2() if using 32/64 Bands
 *   \li  1 omitted gain of 2.0 in qmfInverseModulation()
 *   \li -6 division by 64 in synthesis filterbank
 *   \li x bits external influence
 */
#define ALGORITHMIC_SCALING_IN_SYNTHESIS_FILTERBANK 1

typedef struct {
  int lb_scale;    /*!< Scale of low band area                   */
  int ov_lb_scale; /*!< Scale of adjusted overlap low band area  */
  int hb_scale;    /*!< Scale of high band area                  */
  int ov_hb_scale; /*!< Scale of adjusted overlap high band area */
} QMF_SCALE_FACTOR;

struct QMF_FILTER_BANK {
  const FIXP_PFT *p_filter; /*!< Pointer to filter coefficients */

  void *FilterStates;    /*!< Pointer to buffer of filter states
                              FIXP_PCM in analyse and
                              FIXP_DBL in synthesis filter */
  int FilterSize;        /*!< Size of prototype filter. */
  const FIXP_QTW *t_cos; /*!< Modulation tables. */
  const FIXP_QTW *t_sin;
  int filterScale; /*!< filter scale */

  int no_channels; /*!< Total number of channels (subbands) */
  int no_col;      /*!< Number of time slots       */
  int lsb;         /*!< Top of low subbands */
  int usb;         /*!< Top of high subbands */

  int synScalefactor; /*!< Scale factor of synthesis qmf (syn only) */
  int outScalefactor; /*!< Scale factor of output data (syn only) */
  FIXP_DBL outGain_m; /*!< Mantissa of gain output data (syn only) (init with
                         0x80000000 to ignore) */
  int outGain_e;      /*!< Exponent of gain output data (syn only) */

  UINT flags;     /*!< flags */
  UCHAR p_stride; /*!< Stride Factor of polyphase filters */
};

typedef struct QMF_FILTER_BANK *HANDLE_QMF_FILTER_BANK;

int qmfInitAnalysisFilterBank(
    HANDLE_QMF_FILTER_BANK h_Qmf, /*!< QMF Handle */
    FIXP_QAS *pFilterStates,      /*!< Pointer to filter state buffer */
    int noCols,                   /*!< Number of time slots  */
    int lsb,                      /*!< Number of lower bands */
    int usb,                      /*!< Number of upper bands */
    int no_channels,              /*!< Number of critically sampled bands */
    int flags);                   /*!< Flags */
#if SAMPLE_BITS == 16

int qmfInitAnalysisFilterBank(
    HANDLE_QMF_FILTER_BANK h_Qmf, /*!< QMF Handle */
    FIXP_DBL *pFilterStates,      /*!< Pointer to filter state buffer */
    int noCols,                   /*!< Number of time slots  */
    int lsb,                      /*!< Number of lower bands */
    int usb,                      /*!< Number of upper bands */
    int no_channels,              /*!< Number of critically sampled bands */
    int flags);                   /*!< Flags */
#endif

void qmfAnalysisFiltering(
    HANDLE_QMF_FILTER_BANK anaQmf, /*!< Handle of Qmf Analysis Bank   */
    FIXP_DBL **qmfReal,            /*!< Pointer to real subband slots */
    FIXP_DBL **qmfImag,            /*!< Pointer to imag subband slots */
    QMF_SCALE_FACTOR *scaleFactor, /*!< Scale factors of QMF data     */
    const INT_PCM *timeIn,         /*!< Time signal */
    const int timeIn_e,            /*!< Exponent of audio data        */
    const int stride,              /*!< Stride factor of audio data   */
    FIXP_DBL *pWorkBuffer          /*!< pointer to temporal working buffer */
);
#if SAMPLE_BITS == 16

void qmfAnalysisFiltering(
    HANDLE_QMF_FILTER_BANK anaQmf, /*!< Handle of Qmf Analysis Bank   */
    FIXP_DBL **qmfReal,            /*!< Pointer to real subband slots */
    FIXP_DBL **qmfImag,            /*!< Pointer to imag subband slots */
    QMF_SCALE_FACTOR *scaleFactor, /*!< Scale factors of QMF data     */
    const LONG *timeIn,            /*!< Time signal */
    const int timeIn_e,            /*!< Exponent of audio data        */
    const int stride,              /*!< Stride factor of audio data   */
    FIXP_DBL *pWorkBuffer          /*!< pointer to temporary working buffer */
);
#endif

void qmfAnalysisFilteringSlot(
    HANDLE_QMF_FILTER_BANK anaQmf, /*!< Handle of Qmf Synthesis Bank  */
    FIXP_DBL *qmfReal,             /*!< Low and High band, real */
    FIXP_DBL *qmfImag,             /*!< Low and High band, imag */
    const INT_PCM *timeIn,         /*!< Pointer to input */
    const int stride,              /*!< stride factor of input */
    FIXP_DBL *pWorkBuffer          /*!< pointer to temporal working buffer */
);
#if SAMPLE_BITS == 16

void qmfAnalysisFilteringSlot(
    HANDLE_QMF_FILTER_BANK anaQmf, /*!< Handle of Qmf Synthesis Bank  */
    FIXP_DBL *qmfReal,             /*!< Low and High band, real */
    FIXP_DBL *qmfImag,             /*!< Low and High band, imag */
    const LONG *timeIn,            /*!< Pointer to input */
    const int stride,              /*!< stride factor of input */
    FIXP_DBL *pWorkBuffer          /*!< pointer to temporary working buffer */
);
#endif

int qmfInitSynthesisFilterBank(
    HANDLE_QMF_FILTER_BANK h_Qmf, /*!< QMF Handle */
    FIXP_QSS *pFilterStates,      /*!< Pointer to filter state buffer */
    int noCols,                   /*!< Number of time slots  */
    int lsb,                      /*!< Number of lower bands */
    int usb,                      /*!< Number of upper bands */
    int no_channels,              /*!< Number of critically sampled bands */
    int flags);                   /*!< Flags */

void qmfSynthesisFiltering(
    HANDLE_QMF_FILTER_BANK synQmf,       /*!< Handle of Qmf Synthesis Bank  */
    FIXP_DBL **QmfBufferReal,            /*!< Pointer to real subband slots */
    FIXP_DBL **QmfBufferImag,            /*!< Pointer to imag subband slots */
    const QMF_SCALE_FACTOR *scaleFactor, /*!< Scale factors of QMF data     */
    const int ov_len,                    /*!< Length of band overlap        */
    INT_PCM *timeOut,                    /*!< Time signal */
    const INT stride,                    /*!< Stride factor of audio data   */
    FIXP_DBL *pWorkBuffer /*!< pointer to temporary working buffer. It must be
                             aligned */
);
#if SAMPLE_BITS == 16

void qmfSynthesisFiltering(
    HANDLE_QMF_FILTER_BANK synQmf,       /*!< Handle of Qmf Synthesis Bank  */
    FIXP_DBL **QmfBufferReal,            /*!< Pointer to real subband slots */
    FIXP_DBL **QmfBufferImag,            /*!< Pointer to imag subband slots */
    const QMF_SCALE_FACTOR *scaleFactor, /*!< Scale factors of QMF data     */
    const int ov_len,                    /*!< Length of band overlap        */
    LONG *timeOut,                       /*!< Time signal */
    const int timeOut_e,                 /*!< Target exponent for timeOut  */
    FIXP_DBL *pWorkBuffer /*!< pointer to temporary working buffer */
);
#endif

void qmfSynthesisFilteringSlot(HANDLE_QMF_FILTER_BANK synQmf,
                               const FIXP_DBL *realSlot,
                               const FIXP_DBL *imagSlot,
                               const int scaleFactorLowBand,
                               const int scaleFactorHighBand, INT_PCM *timeOut,
                               const int timeOut_e, FIXP_DBL *pWorkBuffer);
#if SAMPLE_BITS == 16

void qmfSynthesisFilteringSlot(HANDLE_QMF_FILTER_BANK synQmf,
                               const FIXP_DBL *realSlot,
                               const FIXP_DBL *imagSlot,
                               const int scaleFactorLowBand,
                               const int scaleFactorHighBand, LONG *timeOut,
                               const int timeOut_e, FIXP_DBL *pWorkBuffer);
#endif

void qmfChangeOutScalefactor(
    HANDLE_QMF_FILTER_BANK synQmf, /*!< Handle of Qmf Synthesis Bank */
    int outScalefactor             /*!< New scaling factor for output data */
);

int qmfGetOutScalefactor(
    HANDLE_QMF_FILTER_BANK synQmf /*!< Handle of Qmf Synthesis Bank */
);

void qmfChangeOutGain(
    HANDLE_QMF_FILTER_BANK synQmf, /*!< Handle of Qmf Synthesis Bank */
    FIXP_DBL outputGain,           /*!< New gain for output data (mantissa) */
    int outputGainScale            /*!< New gain for output data (exponent) */
);

#endif /*ifndef QMF_H       */
