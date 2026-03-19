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

/*********************** MPEG surround encoder library *************************

   Author(s):   M. Multrus

   Description: Parameter Extraction

*******************************************************************************/

#ifndef SACENC_PARAMEXTRACT_H
#define SACENC_PARAMEXTRACT_H

/* Includes ******************************************************************/
#include "common_fix.h"
#include "sacenc_lib.h"
#include "sacenc_const.h"
#include "sacenc_bitstream.h"

/* Defines *******************************************************************/
#define MAX_CLD_QUANT_FINE (31)
#define MAX_CLD_QUANT_COARSE (15)
#define OFFSET_CLD_QUANT_COARSE (7)
#define OFFSET_CLD_QUANT_FINE (15)

#define MAX_ICC_QUANT_COARSE (4)
#define MAX_ICC_QUANT_FINE (8)
#define OFFSET_ICC_QUANT_COARSE (0)
#define OFFSET_ICC_QUANT_FINE (0)

#define MAX_NUM_PARAM_BANDS (28)

#define NUM_MAPPED_HYBRID_BANDS (16)

/* Data Types ****************************************************************/
typedef struct T_TTO_BOX *HANDLE_TTO_BOX;

typedef enum {
  BOX_SUBBANDS_INVALID = 0,
  BOX_SUBBANDS_4 = 4,
  BOX_SUBBANDS_5 = 5,
  BOX_SUBBANDS_7 = 7,
  BOX_SUBBANDS_9 = 9,
  BOX_SUBBANDS_12 = 12,
  BOX_SUBBANDS_15 = 15,
  BOX_SUBBANDS_23 = 23

} BOX_SUBBAND_CONFIG;

typedef enum {
  BOX_QUANTMODE_INVALID = -1,
  BOX_QUANTMODE_FINE = 0,
  BOX_QUANTMODE_EBQ1 = 1,
  BOX_QUANTMODE_EBQ2 = 2,
  BOX_QUANTMODE_RESERVED3 = 3,
  BOX_QUANTMODE_RESERVED4 = 4,
  BOX_QUANTMODE_RESERVED5 = 5,
  BOX_QUANTMODE_RESERVED6 = 6,
  BOX_QUANTMODE_RESERVED7 = 7

} BOX_QUANTMODE;

typedef struct T_TTO_BOX_CONFIG {
  UCHAR bUseCoarseQuantCld;
  UCHAR bUseCoarseQuantIcc;
  UCHAR bUseCoherenceIccOnly;

  BOX_SUBBAND_CONFIG subbandConfig;
  BOX_QUANTMODE boxQuantMode;

  UCHAR nHybridBandsMax;

  UCHAR bFrameKeep;

} TTO_BOX_CONFIG;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/
FDK_SACENC_ERROR fdk_sacenc_createTtoBox(HANDLE_TTO_BOX *hTtoBox);

FDK_SACENC_ERROR fdk_sacenc_initTtoBox(HANDLE_TTO_BOX hTtoBox,
                                       const TTO_BOX_CONFIG *const ttoBoxConfig,
                                       UCHAR *pParameterBand2HybridBandOffset);

FDK_SACENC_ERROR fdk_sacenc_destroyTtoBox(HANDLE_TTO_BOX *hTtoBox);

FDK_SACENC_ERROR fdk_sacenc_applyTtoBox(
    HANDLE_TTO_BOX hTtoBox, const INT nTimeSlots, const INT startTimeSlot,
    const INT nHybridBands, const FIXP_DPK *const *const ppHybridData1__FDK,
    const FIXP_DPK *const *const ppHybridData2__FDK, SCHAR *const pIccIdx,
    UCHAR *const pbIccQuantCoarse, SCHAR *const pCldIdx,
    UCHAR *const pbCldQuantCoarse, const INT bUseBBCues, INT *scaleCh0,
    INT *scaleCh1);

INT fdk_sacenc_subband2ParamBand(const BOX_SUBBAND_CONFIG boxSubbandConfig,
                                 const INT nSubband);

const INT *fdk_sacenc_getSubbandImagSign();

void fdk_sacenc_calcParameterBand2HybridBandOffset(
    const BOX_SUBBAND_CONFIG subbandConfig, const INT nHybridBands,
    UCHAR *pParameterBand2HybridBandOffset);

/* Function / Class Definition ***********************************************/
static inline UCHAR fdk_sacenc_getCldQuantOffset(const INT bUseCoarseQuant) {
  return ((bUseCoarseQuant) ? OFFSET_CLD_QUANT_COARSE : OFFSET_CLD_QUANT_FINE);
}
static inline UCHAR fdk_sacenc_getIccQuantOffset(const INT bUseCoarseQuant) {
  return ((bUseCoarseQuant) ? OFFSET_ICC_QUANT_COARSE : OFFSET_ICC_QUANT_FINE);
}

static inline UCHAR fdk_sacenc_getNumberCldQuantLevels(
    const INT bUseCoarseQuant) {
  return ((bUseCoarseQuant) ? MAX_CLD_QUANT_COARSE : MAX_CLD_QUANT_FINE);
}
static inline UCHAR fdk_sacenc_getNumberIccQuantLevels(
    const INT bUseCoarseQuant) {
  return ((bUseCoarseQuant) ? MAX_ICC_QUANT_COARSE : MAX_ICC_QUANT_FINE);
}

#endif /* SACENC_PARAMEXTRACT_H */
