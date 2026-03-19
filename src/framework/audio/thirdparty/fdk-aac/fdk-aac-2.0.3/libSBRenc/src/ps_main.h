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

   Author(s):   Markus Multrus

   Description: PS Wrapper, Downmix header file

*******************************************************************************/

#ifndef PS_MAIN_H
#define PS_MAIN_H

/* Includes ******************************************************************/

#include "sbr_def.h"
#include "qmf.h"
#include "ps_encode.h"
#include "FDK_bitstream.h"
#include "FDK_hybrid.h"

/* Data Types ****************************************************************/
typedef enum {
  PSENC_STEREO_BANDS_INVALID = 0,
  PSENC_STEREO_BANDS_10 = 10,
  PSENC_STEREO_BANDS_20 = 20

} PSENC_STEREO_BANDS_CONFIG;

typedef enum {
  PSENC_NENV_1 = 1,
  PSENC_NENV_2 = 2,
  PSENC_NENV_4 = 4,
  PSENC_NENV_DEFAULT = PSENC_NENV_2,
  PSENC_NENV_MAX = PSENC_NENV_4

} PSENC_NENV_CONFIG;

typedef struct {
  UINT bitrateFrom; /* inclusive */
  UINT bitrateTo;   /* exclusive */
  PSENC_STEREO_BANDS_CONFIG nStereoBands;
  PSENC_NENV_CONFIG nEnvelopes;
  LONG iidQuantErrorThreshold; /* quantization threshold to switch between
                                  coarse and fine iid quantization */

} psTuningTable_t;

/* Function / Class Declarations *********************************************/

typedef struct T_PARAMETRIC_STEREO {
  HANDLE_PS_ENCODE hPsEncode;
  PS_OUT psOut[2];

  FIXP_DBL __staticHybridData[HYBRID_READ_OFFSET][MAX_PS_CHANNELS][2]
                             [MAX_HYBRID_BANDS];
  FIXP_DBL
  *pHybridData[HYBRID_READ_OFFSET + HYBRID_FRAMESIZE][MAX_PS_CHANNELS][2];

  FIXP_DBL qmfDelayLines[2][32 >> 1][64];
  int qmfDelayScale;

  INT psDelay;
  UINT maxEnvelopes;
  UCHAR dynBandScale[PS_MAX_BANDS];
  FIXP_DBL maxBandValue[PS_MAX_BANDS];
  SCHAR dmxScale;
  INT initPS;
  INT noQmfSlots;
  INT noQmfBands;

  FIXP_DBL __staticHybAnaStatesLF[MAX_PS_CHANNELS][2 * HYBRID_FILTER_LENGTH *
                                                   HYBRID_MAX_QMF_BANDS];
  FIXP_DBL __staticHybAnaStatesHF[MAX_PS_CHANNELS][2 * HYBRID_FILTER_DELAY *
                                                   (64 - HYBRID_MAX_QMF_BANDS)];
  FDK_ANA_HYB_FILTER fdkHybAnaFilter[MAX_PS_CHANNELS];
  FDK_SYN_HYB_FILTER fdkHybSynFilter;

} PARAMETRIC_STEREO;

typedef struct T_PSENC_CONFIG {
  INT frameSize;
  INT qmfFilterMode;
  INT sbrPsDelay;
  PSENC_STEREO_BANDS_CONFIG nStereoBands;
  PSENC_NENV_CONFIG maxEnvelopes;
  FIXP_DBL iidQuantErrorThreshold;

} PSENC_CONFIG, *HANDLE_PSENC_CONFIG;

typedef struct T_PARAMETRIC_STEREO *HANDLE_PARAMETRIC_STEREO;

/**
 * \brief  Create a parametric stereo encoder instance.
 *
 * \param phParametricStereo    A pointer to a parametric stereo handle to be
 * allocated. Initialized on return.
 *
 * \return
 *          - PSENC_OK, on succes.
 *          - PSENC_INVALID_HANDLE, PSENC_MEMORY_ERROR, on failure.
 */
FDK_PSENC_ERROR PSEnc_Create(HANDLE_PARAMETRIC_STEREO *phParametricStereo);

/**
 * \brief  Initialize a parametric stereo encoder instance.
 *
 * \param hParametricStereo     Meta Data handle.
 * \param hPsEncConfig          Filled parametric stereo configuration
 * structure.
 * \param noQmfSlots            Number of slots within one audio frame.
 * \param noQmfBands            Number of QMF bands.
 * \param dynamic_RAM           Pointer to preallocated workbuffer.
 *
 * \return
 *          - PSENC_OK, on succes.
 *          - PSENC_INVALID_HANDLE, PSENC_INIT_ERROR, on failure.
 */
FDK_PSENC_ERROR PSEnc_Init(HANDLE_PARAMETRIC_STEREO hParametricStereo,
                           const HANDLE_PSENC_CONFIG hPsEncConfig,
                           INT noQmfSlots, INT noQmfBands, UCHAR *dynamic_RAM);

/**
 * \brief  Destroy parametric stereo encoder instance.
 *
 * Deallocate instance and free whole memory.
 *
 * \param phParametricStereo    Pointer to the parametric stereo handle to be
 * deallocated.
 *
 * \return
 *          - PSENC_OK, on succes.
 *          - PSENC_INVALID_HANDLE, on failure.
 */
FDK_PSENC_ERROR PSEnc_Destroy(HANDLE_PARAMETRIC_STEREO *phParametricStereo);

/**
 * \brief  Apply parametric stereo processing.
 *
 * \param hParametricStereo     Meta Data handle.
 * \param samples               Pointer to 2 channel audio input signal.
 * \param timeInStride,         Stride factor of input buffer.
 * \param hQmfAnalysis,         Pointer to QMF analysis filterbanks.
 * \param downmixedRealQmfData  Pointer to real QMF buffer to be written to.
 * \param downmixedImagQmfData  Pointer to imag QMF buffer to be written to.
 * \param downsampledOutSignal  Pointer to buffer where to write downmixed
 * timesignal.
 * \param sbrSynthQmf           Pointer to QMF synthesis filterbank.
 * \param qmfScale              Return scaling factor of the qmf data.
 * \param sendHeader            Signal whether to write header data.
 *
 * \return
 *          - PSENC_OK, on succes.
 *          - PSENC_INVALID_HANDLE, PSENC_ENCODE_ERROR, on failure.
 */
FDK_PSENC_ERROR FDKsbrEnc_PSEnc_ParametricStereoProcessing(
    HANDLE_PARAMETRIC_STEREO hParametricStereo, INT_PCM *samples[2],
    UINT timeInStride, QMF_FILTER_BANK **hQmfAnalysis,
    FIXP_DBL **RESTRICT downmixedRealQmfData,
    FIXP_DBL **RESTRICT downmixedImagQmfData, INT_PCM *downsampledOutSignal,
    HANDLE_QMF_FILTER_BANK sbrSynthQmf, SCHAR *qmfScale, const int sendHeader);

/**
 * \brief  Write parametric stereo bitstream.
 *
 * Write ps_data() element to bitstream and return number of written bits.
 * Returns number of written bits only, if hBitstream == NULL.
 *
 * \param hParametricStereo     Meta Data handle.
 * \param hBitstream            Bitstream buffer handle.
 *
 * \return
 *          - number of written bits.
 */
INT FDKsbrEnc_PSEnc_WritePSData(HANDLE_PARAMETRIC_STEREO hParametricStereo,
                                HANDLE_FDK_BITSTREAM hBitstream);

#endif /* PS_MAIN_H */
