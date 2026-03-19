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

/*********************** MPEG surround decoder library *************************

   Author(s):

   Description: SAC Dec tables

*******************************************************************************/

#ifndef SAC_ROM_H
#define SAC_ROM_H

#include "FDK_archdef.h"
#include "sac_dec_interface.h"

#include "huff_nodes.h"
#include "sac_bitdec.h"
#include "machine_type.h"

/* Global ROM table data type: */
#define FIXP_CFG FIXP_DBL
#define FX_CFG2FX_DBL
#define FX_CFG2FX_SGL FX_DBL2FX_SGL
#define CFG(a) FIXP_DBL(a)
#define FL2FXCONST_CFG FL2FXCONST_DBL
#define FX_DBL2FX_CFG(x) ((FIXP_DBL)(x))

/* others  */
#define SCALE_INV_ICC (2)
#define G_dd_SCALE (2)

#define QCC_SCALE 1
#define M1M2_DATA FIXP_DBL
#define M1M2_CDATA FIXP_DBL
#define M1M2_CDATA2FX_DBL(a) (a)
#define FX_DBL2M1M2_CDATA(a) (a)

#define CLIP_PROTECT_GAIN_0(x) FL2FXCONST_CFG(((x) / (float)(1 << 0)))
#define CLIP_PROTECT_GAIN_1(x) FL2FXCONST_CFG(((x) / (float)(1 << 1)))
#define CLIP_PROTECT_GAIN_2(x) FL2FXCONST_CFG(((x) / (float)(1 << 2)))

#define SF_CLD_C1C2 (9)

extern const FIXP_CFG dequantCPC__FDK[];
extern const FIXP_CFG dequantICC__FDK[8];
extern const FIXP_CFG dequantCLD__FDK[31];

#define IPD_SCALE (5)
#define PI__IPD (FL2FXCONST_DBL(3.1415926535897932f / (float)(1 << IPD_SCALE)))
/* Define for PI*2 for better precision in  SpatialDecApplyPhase() */
#define PIx2__IPD \
  (FL2FXCONST_DBL(3.1415926535897932f / (float)(1 << (IPD_SCALE - 1))))

extern const FIXP_CFG dequantIPD__FDK[16];
extern const FIXP_DBL dequantIPD_CLD_ICC_splitAngle__FDK[15][31][8];

extern const FIXP_CFG H11_nc[31][8];
extern const FIXP_CFG H12_nc[31][8];

extern const FIXP_DBL dequantCLD_c1[31];

extern const FIXP_CFG BP__FDK[];
extern const FIXP_CFG BP_GF__FDK[];
extern const SCHAR row2channelSTP[][MAX_M2_INPUT];

/* sac_bitdec */
extern const INT samplingFreqTable[16];
extern const UCHAR freqResTable[];
extern const UCHAR freqResTable_LD[];
extern const UCHAR tempShapeChanTable[2][8];
extern const TREEPROPERTIES treePropertyTable[];

extern const SCHAR kernels_4_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_5_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_7_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_10_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_14_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_20_to_71[MAX_HYBRID_BANDS];
extern const SCHAR kernels_28_to_71[MAX_HYBRID_BANDS];

extern const UCHAR mapping_4_to_28[MAX_PARAMETER_BANDS];
extern const UCHAR mapping_5_to_28[MAX_PARAMETER_BANDS];
extern const UCHAR mapping_7_to_28[MAX_PARAMETER_BANDS];
extern const UCHAR mapping_10_to_28[MAX_PARAMETER_BANDS];
extern const UCHAR mapping_14_to_28[MAX_PARAMETER_BANDS];
extern const UCHAR mapping_20_to_28[MAX_PARAMETER_BANDS];
extern const SCHAR kernels_4_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_5_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_7_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_9_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_12_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_15_to_64[MAX_HYBRID_BANDS];
extern const SCHAR kernels_23_to_64[MAX_HYBRID_BANDS];

extern const UCHAR mapping_15_to_23[MAX_PARAMETER_BANDS_LD];
extern const UCHAR mapping_12_to_23[MAX_PARAMETER_BANDS_LD];
extern const UCHAR mapping_9_to_23[MAX_PARAMETER_BANDS_LD];
extern const UCHAR mapping_7_to_23[MAX_PARAMETER_BANDS_LD];
extern const UCHAR mapping_5_to_23[MAX_PARAMETER_BANDS_LD];
extern const UCHAR mapping_4_to_23[MAX_PARAMETER_BANDS_LD];

extern const FIXP_CFG clipGainTable__FDK[];
extern const UCHAR clipGainSFTable__FDK[];

extern const UCHAR pbStrideTable[];
extern const int smgTimeTable[];

extern const FIXP_CFG envShapeDataTable__FDK[5][2];
extern const SCHAR row2channelGES[][MAX_M2_INPUT];

/* sac_calcM1andM2 */
extern const SCHAR row2residual[][MAX_M2_INPUT];

void SpatialDequantGetCLDValues(int index, FIXP_DBL* cu, FIXP_DBL* cl);

void SpatialDequantGetCLD2Values(int index, FIXP_DBL* x);

/* External helper functions */
static inline int SacGetHybridSubbands(int qmfSubbands) {
  return qmfSubbands - MAX_QMF_BANDS_TO_HYBRID + 10;
}

#endif /* SAC_ROM_H */
