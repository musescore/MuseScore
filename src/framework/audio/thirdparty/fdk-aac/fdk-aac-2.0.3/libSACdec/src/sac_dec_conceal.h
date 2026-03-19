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

/*********************** MPEG surround decoder library *************************

   Author(s):   Christian Ertel, Christian Griebel

   Description: SAC Dec error concealment

*******************************************************************************/

#ifndef SAC_DEC_CONCEAL_H
#define SAC_DEC_CONCEAL_H

#include "sac_dec_interface.h"

/* Modules dynamic parameters: */
typedef enum {
  SAC_DEC_CONCEAL_METHOD = 0,
  SAC_DEC_CONCEAL_NUM_KEEP_FRAMES,
  SAC_DEC_CONCEAL_FADE_OUT_SLOPE_LENGTH,
  SAC_DEC_CONCEAL_FADE_IN_SLOPE_LENGTH,
  SAC_DEC_CONCEAL_NUM_RELEASE_FRAMES

} SAC_DEC_CONCEAL_PARAM;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* sac_dec_interface.h                                 */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef enum {
  SAC_DEC_CONCEAL_WITH_ZERO_VALUED_OUTPUT = 0,
  SAC_DEC_CONCEAL_BY_FADING_PARAMETERS = 1

} SpatialDecConcealmentMethod;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Default dynamic parameter values: */
#define MPEGS_CONCEAL_DEFAULT_METHOD SAC_DEC_CONCEAL_BY_FADING_PARAMETERS
#define MPEGS_CONCEAL_DEFAULT_NUM_KEEP_FRAMES (10)
#define MPEGS_CONCEAL_DEFAULT_FADE_OUT_SLOPE_LENGTH (5)
#define MPEGS_CONCEAL_DEFAULT_FADE_IN_SLOPE_LENGTH (5)
#define MPEGS_CONCEAL_DEFAULT_NUM_RELEASE_FRAMES (3)

typedef enum {
  SpatialDecConcealState_Init = 0,
  SpatialDecConcealState_Ok,
  SpatialDecConcealState_Keep,
  SpatialDecConcealState_FadeToDefault,
  SpatialDecConcealState_Default,
  SpatialDecConcealState_FadeFromDefault

} SpatialDecConcealmentState;

typedef struct {
  SpatialDecConcealmentMethod method;

  UINT numKeepFrames;
  UINT numFadeOutFrames;
  UINT numFadeInFrames;
  UINT numReleaseFrames;

} SpatialDecConcealmentParams;

typedef struct {
  SpatialDecConcealmentParams concealParams; /* User set params */
  SpatialDecConcealmentState
      concealState; /* State of internal state machine (fade-in/out etc) */

  UINT cntStateFrames; /* Counter for fade-in/out handling */
  UINT cntValidFrames; /* Counter for the number of consecutive good frames*/

} SpatialDecConcealmentInfo;

/* Module reset flags */
#define MPEGS_CONCEAL_RESET_STATE (0x01)
#define MPEGS_CONCEAL_RESET_PARAMETER (0x02)
#define MPEGS_CONCEAL_RESET_ALL (0xFF)

void SpatialDecConcealment_Init(SpatialDecConcealmentInfo *info,
                                const UINT resetFlags);

int SpatialDecConcealment_Apply(SpatialDecConcealmentInfo *info,
                                const SCHAR (*cmpIdxData)[MAX_PARAMETER_BANDS],
                                SCHAR **diffIdxData, SCHAR *idxPrev,
                                SCHAR *bsXXXDataMode, const int startBand,
                                const int stopBand, const SCHAR defaultValue,
                                const int paramType, const int numParamSets);

void SpatialDecConcealment_UpdateState(SpatialDecConcealmentInfo *info,
                                       const int frameOk);

SACDEC_ERROR SpatialDecConcealment_SetParam(SpatialDecConcealmentInfo *info,
                                            const SAC_DEC_CONCEAL_PARAM param,
                                            const INT value);

#endif /* SAC_DEC_CONCEAL_H */
