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

   Description: Quantizing & coding

*******************************************************************************/

#ifndef QC_MAIN_H
#define QC_MAIN_H

#include "aacenc.h"
#include "qc_data.h"
#include "interface.h"
#include "psy_main.h"
#include "tpenc_lib.h"

/* Quantizing & coding stage */

AAC_ENCODER_ERROR FDKaacEnc_QCOutNew(QC_OUT **phQC, const INT nElements,
                                     const INT nChannels, const INT nSubFrames,
                                     UCHAR *dynamic_RAM);

AAC_ENCODER_ERROR FDKaacEnc_QCOutInit(QC_OUT *phQC[(1)], const INT nSubFrames,
                                      const CHANNEL_MAPPING *cm);

AAC_ENCODER_ERROR FDKaacEnc_QCNew(QC_STATE **phQC, INT nElements,
                                  UCHAR *dynamic_RAM);

AAC_ENCODER_ERROR FDKaacEnc_QCInit(QC_STATE *hQC, struct QC_INIT *init,
                                   const ULONG initFlags);

AAC_ENCODER_ERROR FDKaacEnc_QCMainPrepare(
    ELEMENT_INFO *elInfo, ATS_ELEMENT *RESTRICT adjThrStateElement,
    PSY_OUT_ELEMENT *RESTRICT psyOutElement,
    QC_OUT_ELEMENT *RESTRICT qcOutElement, /* returns error code       */
    AUDIO_OBJECT_TYPE aot, UINT syntaxFlags, SCHAR epConfig);

AAC_ENCODER_ERROR FDKaacEnc_QCMain(QC_STATE *RESTRICT hQC, PSY_OUT **psyOut,
                                   QC_OUT **qcOut, INT avgTotalBits,
                                   CHANNEL_MAPPING *cm, AUDIO_OBJECT_TYPE aot,
                                   UINT syntaxFlags, SCHAR epConfig);

AAC_ENCODER_ERROR FDKaacEnc_updateFillBits(CHANNEL_MAPPING *cm,
                                           QC_STATE *qcKernel,
                                           ELEMENT_BITS *RESTRICT elBits[((8))],
                                           QC_OUT **qcOut);

void FDKaacEnc_updateBitres(CHANNEL_MAPPING *cm, QC_STATE *qcKernel,
                            QC_OUT **qcOut);

AAC_ENCODER_ERROR FDKaacEnc_FinalizeBitConsumption(
    CHANNEL_MAPPING *cm, QC_STATE *hQC, QC_OUT *qcOut,
    QC_OUT_ELEMENT **qcElement, HANDLE_TRANSPORTENC hTpEnc,
    AUDIO_OBJECT_TYPE aot, UINT syntaxFlags, SCHAR epConfig);

AAC_ENCODER_ERROR FDKaacEnc_AdjustBitrate(QC_STATE *RESTRICT hQC,
                                          CHANNEL_MAPPING *RESTRICT cm,
                                          INT *avgTotalBits, INT bitRate,
                                          INT sampleRate, INT granuleLength);

void FDKaacEnc_QCClose(QC_STATE **phQCstate, QC_OUT **phQC);

#endif /* QC_MAIN_H */
