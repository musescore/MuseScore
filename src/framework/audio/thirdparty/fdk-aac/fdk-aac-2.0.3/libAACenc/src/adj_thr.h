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

/**************************** AAC encoder library ******************************

   Author(s):   M. Werner

   Description: Threshold compensation

*******************************************************************************/

#ifndef ADJ_THR_H
#define ADJ_THR_H

#include "common_fix.h"
#include "adj_thr_data.h"
#include "qc_data.h"
#include "line_pe.h"
#include "interface.h"

/*****************************************************************************
    functionname: FDKaacEnc_peCalculation
    description:
*****************************************************************************/
void FDKaacEnc_peCalculation(PE_DATA *const peData,
                             const PSY_OUT_CHANNEL *const psyOutChannel[(2)],
                             QC_OUT_CHANNEL *const qcOutChannel[(2)],
                             const struct TOOLSINFO *const toolsInfo,
                             ATS_ELEMENT *const adjThrStateElement,
                             const INT nChannels);

/*****************************************************************************
functionname: FDKaacEnc_AdjThrNew
description:  allocate ADJ_THR_STATE
*****************************************************************************/
INT FDKaacEnc_AdjThrNew(ADJ_THR_STATE **phAdjThr, INT nElements);

/*****************************************************************************
functionname: FDKaacEnc_AdjThrInit
description:  initialize ADJ_THR_STATE
*****************************************************************************/
void FDKaacEnc_AdjThrInit(
    ADJ_THR_STATE *const hAdjThr, const INT meanPe, const INT invQuant,
    const CHANNEL_MAPPING *const channelMapping, const INT sampleRate,
    const INT totalBitrate, const INT isLowDelay,
    const AACENC_BITRES_MODE bitResMode, const INT dZoneQuantEnable,
    const INT bitDistributionMode, const FIXP_DBL vbrQualFactor);

/*****************************************************************************
functionname: FDKaacEnc_DistributeBits
description:
*****************************************************************************/
void FDKaacEnc_DistributeBits(
    ADJ_THR_STATE *adjThrState, ATS_ELEMENT *AdjThrStateElement,
    PSY_OUT_CHANNEL *psyOutChannel[(2)], PE_DATA *peData, INT *grantedPe,
    INT *grantedPeCorr, const INT nChannels, const INT commonWindow,
    const INT avgBits, const INT bitresBits, const INT maxBitresBits,
    const FIXP_DBL maxBitFac, const AACENC_BITRES_MODE bitResMode);

/*****************************************************************************
functionname: FDKaacEnc_AdjustThresholds
description:  adjust thresholds
*****************************************************************************/
void FDKaacEnc_AdjustThresholds(
    ADJ_THR_STATE *const hAdjThr, QC_OUT_ELEMENT *const qcElement[((8))],
    QC_OUT *const qcOut, const PSY_OUT_ELEMENT *const psyOutElement[((8))],
    const INT CBRbitrateMode, const CHANNEL_MAPPING *const cm);

/*****************************************************************************
functionname: FDKaacEnc_AdjThrClose
description:
*****************************************************************************/
void FDKaacEnc_AdjThrClose(ADJ_THR_STATE **hAdjThr);

#endif
