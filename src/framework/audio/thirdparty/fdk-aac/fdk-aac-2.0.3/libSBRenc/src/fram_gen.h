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
  \brief  Framing generator prototypes and structs $Revision: 92790 $
*/
#ifndef FRAM_GEN_H
#define FRAM_GEN_H

#include "sbr_def.h" /* for MAX_ENVELOPES and MAX_NOISE_ENVELOPES in struct FRAME_INFO and CODEC_TYPE */
#include "sbr_encoder.h" /* for FREQ_RES */

#define MAX_ENVELOPES_VARVAR \
  MAX_ENVELOPES /*!< worst case number of envelopes in a VARVAR frame */
#define MAX_ENVELOPES_FIXVAR_VARFIX \
  4 /*!< worst case number of envelopes in VARFIX and FIXVAR frames */
#define MAX_NUM_REL \
  3 /*!< maximum number of relative borders in any VAR frame */

/* SBR frame class definitions */
typedef enum {
  FIXFIX =
      0,  /*!< bs_frame_class: leading and trailing frame borders are fixed */
  FIXVAR, /*!< bs_frame_class: leading frame border is fixed, trailing frame
             border is variable */
  VARFIX, /*!< bs_frame_class: leading frame border is variable, trailing frame
             border is fixed */
  VARVAR /*!< bs_frame_class: leading and trailing frame borders are variable */
  ,
  FIXFIXonly /*!< bs_frame_class: leading border fixed (0), trailing border
                fixed (nrTimeSlots) and encased borders are dynamically derived
                from the tranPos */
} FRAME_CLASS;

/* helper constants */
#define DC 4711     /*!< helper constant: don't care */
#define EMPTY (-99) /*!< helper constant: empty */

/* system constants: AAC+SBR, DRM Frame-Length */
#define FRAME_MIDDLE_SLOT_1920 4
#define NUMBER_TIME_SLOTS_1920 15

#define LD_PRETRAN_OFF 3
#define FRAME_MIDDLE_SLOT_512LD 4
#define NUMBER_TIME_SLOTS_512LD 8
#define TRANSIENT_OFFSET_LD 0

/*
system constants: AAC+SBR or aacPRO (hybrid format), Standard Frame-Length,
Multi-Rate
---------------------------------------------------------------------------
Number of slots (numberTimeSlots): 16  (NUMBER_TIME_SLOTS_2048)
Detector-offset (frameMiddleSlot):  4
Overlap                          :  3
Buffer-offset                    :  8  (BUFFER_FRAME_START_2048 = 0)


                        |<------------tranPos---------->|
                |c|d|e|f|0|1|2|3|4|5|6|7|8|9|a|b|c|d|e|f|
        FixFix  |                               |
        FixVar  |                               :<- ->:
        VarFix  :<- ->:                         |
        VarVar  :<- ->:                         :<- ->:
                0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3
................................................................................

|-|-|-|-|-|-|-|-B-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-B-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|

frame-generator:0                               16              24 32
analysis-buffer:8                               24              32 40
*/
#define FRAME_MIDDLE_SLOT_2048 4
#define NUMBER_TIME_SLOTS_2048 16

/*
system constants: mp3PRO, Multi-Rate & Single-Rate
--------------------------------------------------
Number of slots (numberTimeSlots):  9    (NUMBER_TIME_SLOTS_1152)
Detector-offset (frameMiddleSlot):  4    (FRAME_MIDDLE_SLOT_1152)
Overlap                          :  3
Buffer-offset                    :  4.5  (BUFFER_FRAME_START_1152 = 0)


                         |<----tranPos---->|
                 |5|6|7|8|0|1|2|3|4|5|6|7|8|
         FixFix  |                 |
         FixVar  |                 :<- ->:
         VarFix  :<- ->:           |
         VarVar  :<- ->:           :<- ->:
                 0 1 2 3 4 5 6 7 8 0 1 2 3
        .............................................

        -|-|-|-|-B-|-|-|-|-|-|-|-|-B-|-|-|-|-|-|-|-|-|

frame-generator: 0                 9       13        18
analysis-buffer: 4.5               13.5              22.5
*/
#define FRAME_MIDDLE_SLOT_1152 4
#define NUMBER_TIME_SLOTS_1152 9

/* system constants: Layer2+SBR */
#define FRAME_MIDDLE_SLOT_2304 8
#define NUMBER_TIME_SLOTS_2304 18

/*!
  \struct SBR_GRID
  \brief  sbr_grid() signals to be converted to bitstream elements

  The variables hold the signals (e.g. lengths and numbers) in "clear text"
*/

typedef struct {
  /* system constants */
  INT bufferFrameStart; /*!< frame generator vs analysis buffer time alignment
                           (currently set to 0, offset added elsewhere) */
  INT numberTimeSlots;  /*!< number of SBR timeslots per frame */

  /* will be adjusted for every frame */
  FRAME_CLASS frameClass; /*!< SBR frame class  */
  INT bs_num_env;         /*!< bs_num_env, number of envelopes for FIXFIX */
  INT bs_abs_bord; /*!< bs_abs_bord, absolute border for VARFIX and FIXVAR */
  INT n;           /*!< number of relative borders for VARFIX and FIXVAR   */
  INT p;           /*!< pointer-to-transient-border  */
  INT bs_rel_bord[MAX_NUM_REL]; /*!< bs_rel_bord, relative borders for all VAR
                                 */
  INT v_f[MAX_ENVELOPES_FIXVAR_VARFIX]; /*!< envelope frequency resolutions for
                                           FIXVAR and VARFIX  */

  INT bs_abs_bord_0; /*!< bs_abs_bord_0, leading absolute border for VARVAR */
  INT bs_abs_bord_1; /*!< bs_abs_bord_1, trailing absolute border for VARVAR */
  INT bs_num_rel_0;  /*!< bs_num_rel_0, number of relative borders associated
                        with leading absolute border for VARVAR */
  INT bs_num_rel_1;  /*!< bs_num_rel_1, number of relative borders associated
                        with trailing absolute border for VARVAR */
  INT bs_rel_bord_0[MAX_NUM_REL];  /*!< bs_rel_bord_0, relative borders
                                      associated with  leading absolute border
                                      for  VARVAR */
  INT bs_rel_bord_1[MAX_NUM_REL];  /*!< bs_rel_bord_1, relative borders
                                      associated with trailing absolute border
                                      for VARVAR */
  INT v_fLR[MAX_ENVELOPES_VARVAR]; /*!< envelope frequency resolutions for
                                      VARVAR  */

} SBR_GRID;
typedef SBR_GRID *HANDLE_SBR_GRID;

/*!
  \struct SBR_FRAME_INFO
  \brief  time/frequency grid description for one frame
*/
typedef struct {
  INT nEnvelopes;                  /*!< number of envelopes */
  INT borders[MAX_ENVELOPES + 1];  /*!< envelope borders in SBR timeslots */
  FREQ_RES freqRes[MAX_ENVELOPES]; /*!< frequency resolution of each envelope */
  INT shortEnv; /*!< number of an envelope to be shortened (starting at 1) or 0
                   for no shortened envelope */
  INT nNoiseEnvelopes; /*!< number of noise floors */
  INT bordersNoise[MAX_NOISE_ENVELOPES +
                   1]; /*!< noise floor borders in SBR timeslots */
} SBR_FRAME_INFO;
/* WARNING: When rearranging the elements of this struct keep in mind that the
 * static initializations in the corresponding C-file have to be rearranged as
 * well! snd 2002/01/23
 */
typedef SBR_FRAME_INFO *HANDLE_SBR_FRAME_INFO;

/*!
  \struct SBR_ENVELOPE_FRAME
  \brief  frame generator main struct

  Contains tuning parameters, time/frequency grid description, sbr_grid()
  bitstream elements, and generator internal signals
*/
typedef struct {
  /* system constants */
  INT frameMiddleSlot; /*!< transient detector offset in SBR timeslots */

  /* basic tuning parameters */
  INT staticFraming; /*!< 1: run static framing in time, i.e. exclusive use of
                        bs_frame_class = FIXFIX */
  INT numEnvStatic;  /*!< number of envelopes per frame for static framing */
  FREQ_RES
  freq_res_fixfix[2]; /*!< envelope frequency resolution to use for
                         bs_frame_class = FIXFIX; single env and split */
  UCHAR
  fResTransIsLow; /*!< frequency resolution for transient frames - always
                     low (0) or according to table (1) */

  /* expert tuning parameters */
  const int *v_tuningSegm; /*!< segment lengths to use around transient */
  const int *v_tuningFreq; /*!< frequency resolutions to use around transient */
  INT dmin;                /*!< minimum length of dependent segments */
  INT dmax;                /*!< maximum length of dependent segments */
  INT allowSpread; /*!< 1: allow isolated transient to influence grid of 3
                      consecutive frames */

  /* internally used signals */
  FRAME_CLASS frameClassOld; /*!< frame class used for previous frame */
  INT spreadFlag; /*!< 1: use VARVAR instead of VARFIX to follow up old
                     transient */

  INT v_bord[2 * MAX_ENVELOPES_VARVAR + 1]; /*!< borders for current frame and
                                               preliminary borders for next
                                               frame (fixed borders excluded) */
  INT length_v_bord; /*!< helper variable: length of v_bord */
  INT v_freq[2 * MAX_ENVELOPES_VARVAR + 1]; /*!< frequency resolutions for
                                               current frame and preliminary
                                               resolutions for next frame */
  INT length_v_freq; /*!< helper variable: length of v_freq */

  INT v_bordFollow[MAX_ENVELOPES_VARVAR]; /*!< preliminary borders for current
                                             frame (calculated during previous
                                             frame) */
  INT length_v_bordFollow; /*!< helper variable: length of v_bordFollow */
  INT i_tranFollow; /*!< points to transient border in v_bordFollow (may be
                       negative, see keepForFollowUp()) */
  INT i_fillFollow; /*!< points to first fill border in v_bordFollow */
  INT v_freqFollow[MAX_ENVELOPES_VARVAR]; /*!< preliminary frequency resolutions
                                             for current frame (calculated
                                             during previous frame) */
  INT length_v_freqFollow; /*!< helper variable: length of v_freqFollow */

  /* externally needed signals */
  SBR_GRID
  SbrGrid; /*!< sbr_grid() signals to be converted to bitstream elements */
  SBR_FRAME_INFO
  SbrFrameInfo; /*!< time/frequency grid description for one frame */
} SBR_ENVELOPE_FRAME;
typedef SBR_ENVELOPE_FRAME *HANDLE_SBR_ENVELOPE_FRAME;

void FDKsbrEnc_initFrameInfoGenerator(HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                                      INT allowSpread, INT numEnvStatic,
                                      INT staticFraming, INT timeSlots,
                                      const FREQ_RES *freq_res_fixfix,
                                      UCHAR fResTransIsLow, INT ldGrid);

HANDLE_SBR_FRAME_INFO
FDKsbrEnc_frameInfoGenerator(HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                             UCHAR *v_transient_info, const INT rightBorderFIX,
                             UCHAR *v_transient_info_pre, int ldGrid,
                             const int *v_tuning);

#endif
