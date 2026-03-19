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

#ifndef HBE_H
#define HBE_H

#include "sbrdecoder.h"

#ifndef QMF_SYNTH_CHANNELS
#define QMF_SYNTH_CHANNELS (64)
#endif

#define HBE_QMF_FILTER_STATE_ANA_SIZE (400)
#define HBE_QMF_FILTER_STATE_SYN_SIZE (200)

#ifndef MAX_NUM_PATCHES_HBE
#define MAX_NUM_PATCHES_HBE (6)
#endif
#define MAX_STRETCH_HBE (4)

typedef enum {
  KEEP_STATES_SYNCED_OFF = 0,    /*!< normal QMF transposer behaviour */
  KEEP_STATES_SYNCED_NORMAL = 1, /*!< QMF transposer called for syncing of
                                    states the last 8/14 slots are calculated in
                                    case next frame is HBE */
  KEEP_STATES_SYNCED_OUTDIFF =
      2, /*!< QMF transposer behaviour as in normal case, but the calculated
              slots are directly written to overlap area of buffer; only used in
              resetSbrDec function */
  KEEP_STATES_SYNCED_NOOUT =
      3 /*!< QMF transposer is called for syncing of states only, not output
             is generated at all; only used in resetSbrDec function */
} KEEP_STATES_SYNCED_MODE;

struct hbeTransposer {
  FIXP_DBL anaQmfStates[HBE_QMF_FILTER_STATE_ANA_SIZE];
  FIXP_QSS synQmfStates[HBE_QMF_FILTER_STATE_SYN_SIZE];

  int xOverQmf[MAX_NUM_PATCHES_HBE];

  int maxStretch;
  int timeDomainWinLen;
  int qmfInBufSize;
  int qmfOutBufSize;
  int noCols;
  int noChannels;
  int startBand;
  int stopBand;
  int bSbr41;

  LONG *inBuf_F;
  FIXP_DBL **qmfInBufReal_F;
  FIXP_DBL **qmfInBufImag_F;

  FIXP_DBL *qmfBufferCodecTempSlot_F;

  QMF_FILTER_BANK HBEAnalysiscQMF;
  QMF_FILTER_BANK HBESynthesisQMF;

  FIXP_DBL const *synthesisQmfPreModCos_F;
  FIXP_DBL const *synthesisQmfPreModSin_F;

  FIXP_DBL **qmfHBEBufReal_F;
  FIXP_DBL **qmfHBEBufImag_F;

  int bXProducts[MAX_STRETCH_HBE];

  int kstart;
  int synthSize;

  int highband_exp[2];
  int target_exp[2];
};

typedef struct hbeTransposer *HANDLE_HBE_TRANSPOSER;

SBR_ERROR QmfTransposerCreate(HANDLE_HBE_TRANSPOSER *hQmfTransposer,
                              const int frameSize, int bDisableCrossProducts,
                              int bSbr41);

SBR_ERROR QmfTransposerReInit(HANDLE_HBE_TRANSPOSER hQmfTransposer,
                              UCHAR *FreqBandTable[2], UCHAR NSfb[2]);

void QmfTransposerClose(HANDLE_HBE_TRANSPOSER hQmfTransposer);

void QmfTransposerApply(HANDLE_HBE_TRANSPOSER hQmfTransposer,
                        FIXP_DBL **qmfBufferCodecReal,
                        FIXP_DBL **qmfBufferCodecImag, int nColsIn,
                        FIXP_DBL **ppQmfBufferOutReal_F,
                        FIXP_DBL **ppQmfBufferOutImag_F,
                        FIXP_DBL lpcFilterStatesReal[2 + (3 * (4))][(64)],
                        FIXP_DBL lpcFilterStatesImag[2 + (3 * (4))][(64)],
                        int pitchInBins, int scale_lb, int scale_hbe,
                        int *scale_hb, int timeStep, int firstSlotOffsset,
                        int ov_len,
                        KEEP_STATES_SYNCED_MODE keepStatesSyncedMode);

int *GetxOverBandQmfTransposer(HANDLE_HBE_TRANSPOSER hQmfTransposer);

int Get41SbrQmfTransposer(HANDLE_HBE_TRANSPOSER hQmfTransposer);
#endif /* HBE_H */
