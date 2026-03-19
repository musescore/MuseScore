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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Sbr decoder
*/
#ifndef SBR_DEC_H
#define SBR_DEC_H

#include "sbrdecoder.h"

#include "lpp_tran.h"
#include "qmf.h"
#include "env_calc.h"
#include "FDK_audio.h"

#include "sbrdec_drc.h"

#include "pvc_dec.h"

#include "hbe.h"

enum SBRDEC_QMF_SKIP {
  qmfSkipNothing = 0,
  qmfSkipAnalysis = 1 << 0,
  qmfSkipSynthesis = 1 << 1
};

typedef struct {
  SBR_CALCULATE_ENVELOPE SbrCalculateEnvelope;
  SBR_LPP_TRANS LppTrans;
  PVC_STATIC_DATA PvcStaticData;

  /* do scale handling in sbr an not in qmf */
  SHORT scale_ov;
  SHORT scale_lb;
  SHORT scale_hbe;

  SHORT prev_frame_lSbr;
  SHORT prev_frame_hbeSbr;

  int codecFrameSize;

  HANDLE_HBE_TRANSPOSER hHBE;

  HANDLE_FDK_QMF_DOMAIN_IN qmfDomainInCh;
  HANDLE_FDK_QMF_DOMAIN_OUT qmfDomainOutCh;

  SBRDEC_DRC_CHANNEL sbrDrcChannel;

#if (SBRDEC_MAX_HB_FADE_FRAMES > 0)
  INT highBandFadeCnt; /* counter for fading in high-band signal smoothly */

#endif
  FIXP_DBL **tmp_memory; /* shared memory between hbeLightTimeDelayBuffer and
                            hQmfHBESlotsReal */

  FIXP_DBL **hQmfHBESlotsReal;
  FIXP_DBL **hQmfHBESlotsImag;

  FIXP_DBL **codecQMFBufferReal;
  FIXP_DBL **codecQMFBufferImag;
  UCHAR savedStates;
  int applySbrProc_old;
} SBR_DEC;

typedef SBR_DEC *HANDLE_SBR_DEC;

typedef struct {
  SBR_FRAME_DATA frameData[(1) + 1];
  SBR_PREV_FRAME_DATA prevFrameData;
  SBR_DEC SbrDec;
} SBR_CHANNEL;

typedef SBR_CHANNEL *HANDLE_SBR_CHANNEL;

void sbr_dec(
    HANDLE_SBR_DEC hSbrDec,             /*!< handle to Decoder channel */
    LONG *timeIn,                       /*!< pointer to input time signal */
    LONG *timeOut,                      /*!< pointer to output time signal */
    HANDLE_SBR_DEC hSbrDecRight,        /*!< handle to Decoder channel right */
    LONG *timeOutRight,                 /*!< pointer to output time signal */
    INT strideOut,                      /*!< Time data traversal strideOut */
    HANDLE_SBR_HEADER_DATA hHeaderData, /*!< Static control data */
    HANDLE_SBR_FRAME_DATA hFrameData,   /*!< Control data of current frame */
    HANDLE_SBR_PREV_FRAME_DATA
        hPrevFrameData,        /*!< Some control data of last frame */
    const int applyProcessing, /*!< Flag for SBR operation */
    HANDLE_PS_DEC h_ps_d, const UINT flags, const int codecFrameSize,
    const INT sbrInDataHeadroom);

SBR_ERROR
createSbrDec(SBR_CHANNEL *hSbrChannel, HANDLE_SBR_HEADER_DATA hHeaderData,
             TRANSPOSER_SETTINGS *pSettings, const int downsampleFac,
             const UINT qmfFlags, const UINT flags, const int overlap, int chan,
             int codecFrameSize);

int deleteSbrDec(SBR_CHANNEL *hSbrChannel);

SBR_ERROR
resetSbrDec(HANDLE_SBR_DEC hSbrDec, HANDLE_SBR_HEADER_DATA hHeaderData,
            HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData, const int downsampleFac,
            const UINT flags, HANDLE_SBR_FRAME_DATA hFrameData);

#endif
