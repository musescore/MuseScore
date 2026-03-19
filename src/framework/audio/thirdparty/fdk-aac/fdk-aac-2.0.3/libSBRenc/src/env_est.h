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

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Envelope estimation structs and prototypes $Revision: 92790 $
*/
#ifndef ENV_EST_H
#define ENV_EST_H

#include "sbr_def.h"
#include "sbr_encoder.h" /* SBR econfig structs */
#include "ps_main.h"
#include "bit_sbr.h"
#include "fram_gen.h"
#include "tran_det.h"
#include "code_env.h"
#include "ton_corr.h"

typedef struct {
  FIXP_DBL *rBuffer[32];
  FIXP_DBL *iBuffer[32];

  FIXP_DBL *p_YBuffer;

  FIXP_DBL *YBuffer[32];
  int YBufferScale[2];

  UCHAR envelopeCompensation[MAX_FREQ_COEFFS];
  UCHAR pre_transient_info[2];

  int YBufferWriteOffset;
  int YBufferSzShift;
  int rBufferReadOffset;

  int no_cols;
  int no_rows;
  int start_index;

  int time_slots;
  int time_step;
} SBR_EXTRACT_ENVELOPE;
typedef SBR_EXTRACT_ENVELOPE *HANDLE_SBR_EXTRACT_ENVELOPE;

struct ENV_CHANNEL {
  FAST_TRAN_DETECTOR sbrFastTransientDetector;
  SBR_TRANSIENT_DETECTOR sbrTransientDetector;
  SBR_CODE_ENVELOPE sbrCodeEnvelope;
  SBR_CODE_ENVELOPE sbrCodeNoiseFloor;
  SBR_EXTRACT_ENVELOPE sbrExtractEnvelope;

  SBR_ENVELOPE_FRAME SbrEnvFrame;
  SBR_TON_CORR_EST TonCorr;

  struct SBR_ENV_DATA encEnvData;

  int qmfScale;
  UCHAR fLevelProtect;
};
typedef struct ENV_CHANNEL *HANDLE_ENV_CHANNEL;

/************  Function Declarations ***************/

INT FDKsbrEnc_CreateExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut,
                                       INT channel, INT chInEl,
                                       UCHAR *dynamic_RAM);

INT FDKsbrEnc_InitExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbr,
                                     int no_cols, int no_rows, int start_index,
                                     int time_slots, int time_step,
                                     int tran_off, ULONG statesInitFlag,
                                     int chInEl, UCHAR *dynamic_RAM,
                                     UINT sbrSyntaxFlags);

void FDKsbrEnc_deleteExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut);

typedef struct {
  FREQ_RES res[MAX_NUM_NOISE_VALUES];
  int maxQuantError;

} SBR_FRAME_TEMP_DATA;

typedef struct {
  const SBR_FRAME_INFO *frame_info;
  FIXP_DBL noiseFloor[MAX_NUM_NOISE_VALUES];
  SCHAR sfb_nrg_coupling
      [MAX_NUM_ENVELOPE_VALUES]; /* only used if stereomode = SWITCH_L_R_C */
  SCHAR sfb_nrg[MAX_NUM_ENVELOPE_VALUES];
  SCHAR noise_level_coupling
      [MAX_NUM_NOISE_VALUES]; /* only used if stereomode = SWITCH_L_R_C */
  SCHAR noise_level[MAX_NUM_NOISE_VALUES];
  UCHAR transient_info[3];
  UCHAR nEnvelopes;
} SBR_ENV_TEMP_DATA;

/*
 * Extract features from QMF data. Afterwards, the QMF data is not required
 * anymore.
 */
void FDKsbrEnc_extractSbrEnvelope1(HANDLE_SBR_CONFIG_DATA h_con,
                                   HANDLE_SBR_HEADER_DATA sbrHeaderData,
                                   HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
                                   HANDLE_ENV_CHANNEL h_envChan,
                                   HANDLE_COMMON_DATA cmonData,
                                   SBR_ENV_TEMP_DATA *eData,
                                   SBR_FRAME_TEMP_DATA *fData);

/*
 * Process the previously features extracted by FDKsbrEnc_extractSbrEnvelope1
 * and create/encode SBR envelopes.
 */
void FDKsbrEnc_extractSbrEnvelope2(HANDLE_SBR_CONFIG_DATA h_con,
                                   HANDLE_SBR_HEADER_DATA sbrHeaderData,
                                   HANDLE_PARAMETRIC_STEREO hParametricStereo,
                                   HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
                                   HANDLE_ENV_CHANNEL sbrEnvChannel0,
                                   HANDLE_ENV_CHANNEL sbrEnvChannel1,
                                   HANDLE_COMMON_DATA cmonData,
                                   SBR_ENV_TEMP_DATA *eData,
                                   SBR_FRAME_TEMP_DATA *fData, int clearOutput);

INT FDKsbrEnc_GetEnvEstDelay(HANDLE_SBR_EXTRACT_ENVELOPE hSbr);

#endif
