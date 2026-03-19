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

/************************* MPEG-D DRC decoder library **************************

   Author(s):   Andreas Hoelzer

   Description: DRC Set Selection

*******************************************************************************/

#ifndef DRCDEC_SELECTIONPROCESS_H
#define DRCDEC_SELECTIONPROCESS_H

#include "drcDec_types.h"
#include "drcDecoder.h"

/* DRC set selection according to section 6.2 of ISO/IEC 23003-4 (MPEG-D DRC) */
/* including ISO/IEC 23003-4/AMD1 (Amendment 1) */

typedef struct s_drcdec_selection_process* HANDLE_DRC_SELECTION_PROCESS;

#define UNDEFINED_LOUDNESS_VALUE (FIXP_DBL)(MAXVAL_DBL - 1)

typedef enum {
  DRCDEC_SELECTION_PROCESS_NO_ERROR = 0,

  DRCDEC_SELECTION_PROCESS_WARNING = -1000,

  DRCDEC_SELECTION_PROCESS_NOT_OK = -2000,
  DRCDEC_SELECTION_PROCESS_OUTOFMEMORY,
  DRCDEC_SELECTION_PROCESS_INVALID_HANDLE,
  DRCDEC_SELECTION_PROCESS_NOT_SUPPORTED,
  DRCDEC_SELECTION_PROCESS_INVALID_PARAM,
  DRCDEC_SELECTION_PROCESS_PARAM_OUT_OF_RANGE

} DRCDEC_SELECTION_PROCESS_RETURN;

typedef enum {
  SEL_PROC_TEST_TIME_DOMAIN = -100,
  SEL_PROC_TEST_QMF_DOMAIN,
  SEL_PROC_TEST_STFT_DOMAIN,

  SEL_PROC_CODEC_MODE_UNDEFINED = -1,
  SEL_PROC_MPEG_4_AAC,
  SEL_PROC_MPEG_D_USAC,
  SEL_PROC_MPEG_H_3DA
} SEL_PROC_CODEC_MODE;

typedef enum {
  /* set and get user param */
  SEL_PROC_LOUDNESS_NORMALIZATION_ON,
  /* get only user param */
  SEL_PROC_DYNAMIC_RANGE_CONTROL_ON,
  /* set only user params */
  SEL_PROC_TARGET_LOUDNESS,
  SEL_PROC_EFFECT_TYPE,
  SEL_PROC_EFFECT_TYPE_FALLBACK_CODE,
  SEL_PROC_LOUDNESS_MEASUREMENT_METHOD,
  SEL_PROC_ALBUM_MODE,
  SEL_PROC_DOWNMIX_ID,
  SEL_PROC_TARGET_LAYOUT,
  SEL_PROC_TARGET_CHANNEL_COUNT,
  SEL_PROC_BASE_CHANNEL_COUNT,
  SEL_PROC_SAMPLE_RATE,
  SEL_PROC_BOOST,
  SEL_PROC_COMPRESS
} SEL_PROC_USER_PARAM;

typedef struct s_selection_process_output {
  FIXP_DBL outputPeakLevelDb;           /* e = 7 */
  FIXP_DBL loudnessNormalizationGainDb; /* e = 7 */
  FIXP_DBL outputLoudness;              /* e = 7 */

  UCHAR numSelectedDrcSets;
  SCHAR selectedDrcSetIds[MAX_ACTIVE_DRCS];
  UCHAR selectedDownmixIds[MAX_ACTIVE_DRCS];

  UCHAR activeDownmixId;
  UCHAR baseChannelCount;
  UCHAR targetChannelCount;
  SCHAR targetLayout;
  UCHAR downmixMatrixPresent;
  FIXP_DBL downmixMatrix[8][8]; /* e = 2 */

  FIXP_SGL boost;    /* e = 1 */
  FIXP_SGL compress; /* e = 1 */

  FIXP_DBL mixingLevel; /* e = 7 */

} SEL_PROC_OUTPUT, *HANDLE_SEL_PROC_OUTPUT;

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Create(HANDLE_DRC_SELECTION_PROCESS* phInstance);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Delete(HANDLE_DRC_SELECTION_PROCESS* phInstance);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Init(HANDLE_DRC_SELECTION_PROCESS hInstance);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetCodecMode(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                     const SEL_PROC_CODEC_MODE codecMode);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetParam(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                 const SEL_PROC_USER_PARAM requestType,
                                 FIXP_DBL requestValue, int* pDiff);

FIXP_DBL
drcDec_SelectionProcess_GetParam(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                 const SEL_PROC_USER_PARAM requestType);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_SetMpeghParams(
    HANDLE_DRC_SELECTION_PROCESS hInstance, const int numGroupIdsRequested,
    const int* groupIdRequested, const int numGroupPresetIdsRequested,
    const int* groupPresetIdRequested,
    const int* numMembersGroupPresetIdsRequested,
    const int groupPresetIdRequestedPreference, int* pDiff);

DRCDEC_SELECTION_PROCESS_RETURN
drcDec_SelectionProcess_Process(HANDLE_DRC_SELECTION_PROCESS hInstance,
                                HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                HANDLE_SEL_PROC_OUTPUT hSelProcOutput);

#endif
