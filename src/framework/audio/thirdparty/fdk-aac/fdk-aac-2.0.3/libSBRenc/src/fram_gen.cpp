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

#include "fram_gen.h"
#include "sbr_misc.h"

#include "genericStds.h"

static const SBR_FRAME_INFO frameInfo1_2048 = {1, {0, 16}, {FREQ_RES_HIGH},
                                               0, 1,       {0, 16}};

static const SBR_FRAME_INFO frameInfo2_2048 = {
    2, {0, 8, 16}, {FREQ_RES_HIGH, FREQ_RES_HIGH}, 0, 2, {0, 8, 16}};

static const SBR_FRAME_INFO frameInfo4_2048 = {
    4,
    {0, 4, 8, 12, 16},
    {FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH},
    0,
    2,
    {0, 8, 16}};

static const SBR_FRAME_INFO frameInfo1_2304 = {1, {0, 18}, {FREQ_RES_HIGH},
                                               0, 1,       {0, 18}};

static const SBR_FRAME_INFO frameInfo2_2304 = {
    2, {0, 9, 18}, {FREQ_RES_HIGH, FREQ_RES_HIGH}, 0, 2, {0, 9, 18}};

static const SBR_FRAME_INFO frameInfo4_2304 = {
    4,
    {0, 5, 9, 14, 18},
    {FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH},
    0,
    2,
    {0, 9, 18}};

static const SBR_FRAME_INFO frameInfo1_1920 = {1, {0, 15}, {FREQ_RES_HIGH},
                                               0, 1,       {0, 15}};

static const SBR_FRAME_INFO frameInfo2_1920 = {
    2, {0, 8, 15}, {FREQ_RES_HIGH, FREQ_RES_HIGH}, 0, 2, {0, 8, 15}};

static const SBR_FRAME_INFO frameInfo4_1920 = {
    4,
    {0, 4, 8, 12, 15},
    {FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH},
    0,
    2,
    {0, 8, 15}};

static const SBR_FRAME_INFO frameInfo1_1152 = {1, {0, 9}, {FREQ_RES_HIGH},
                                               0, 1,      {0, 9}};

static const SBR_FRAME_INFO frameInfo2_1152 = {
    2, {0, 5, 9}, {FREQ_RES_HIGH, FREQ_RES_HIGH}, 0, 2, {0, 5, 9}};

static const SBR_FRAME_INFO frameInfo4_1152 = {
    4,
    {0, 2, 5, 7, 9},
    {FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH},
    0,
    2,
    {0, 5, 9}};

/* AACLD frame info */
static const SBR_FRAME_INFO frameInfo1_512LD = {1, {0, 8}, {FREQ_RES_HIGH},
                                                0, 1,      {0, 8}};

static const SBR_FRAME_INFO frameInfo2_512LD = {
    2, {0, 4, 8}, {FREQ_RES_HIGH, FREQ_RES_HIGH}, 0, 2, {0, 4, 8}};

static const SBR_FRAME_INFO frameInfo4_512LD = {
    4,
    {0, 2, 4, 6, 8},
    {FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH},
    0,
    2,
    {0, 4, 8}};

static int calcFillLengthMax(
    int tranPos,        /*!< input : transient position (ref: tran det) */
    int numberTimeSlots /*!< input : number of timeslots */
);

static void fillFrameTran(
    const int *v_tuningSegm, /*!< tuning: desired segment lengths */
    const int *v_tuningFreq, /*!< tuning: desired frequency resolutions */
    int tran,                /*!< input : position of transient */
    int *v_bord,             /*!< memNew: borders */
    int *length_v_bord,      /*!< memNew: # borders */
    int *v_freq,             /*!< memNew: frequency resolutions */
    int *length_v_freq,      /*!< memNew: # frequency resolutions */
    int *bmin,               /*!< hlpNew: first mandatory border */
    int *bmax                /*!< hlpNew: last  mandatory border */
);

static void fillFramePre(INT dmax, INT *v_bord, INT *length_v_bord, INT *v_freq,
                         INT *length_v_freq, INT bmin, INT rest);

static void fillFramePost(INT *parts, INT *d, INT dmax, INT *v_bord,
                          INT *length_v_bord, INT *v_freq, INT *length_v_freq,
                          INT bmax, INT bufferFrameStart, INT numberTimeSlots,
                          INT fmax);

static void fillFrameInter(INT *nL, const int *v_tuningSegm, INT *v_bord,
                           INT *length_v_bord, INT bmin, INT *v_freq,
                           INT *length_v_freq, INT *v_bordFollow,
                           INT *length_v_bordFollow, INT *v_freqFollow,
                           INT *length_v_freqFollow, INT i_fillFollow, INT dmin,
                           INT dmax, INT numberTimeSlots);

static void calcFrameClass(FRAME_CLASS *frameClass, FRAME_CLASS *frameClassOld,
                           INT tranFlag, INT *spreadFlag);

static void specialCase(INT *spreadFlag, INT allowSpread, INT *v_bord,
                        INT *length_v_bord, INT *v_freq, INT *length_v_freq,
                        INT *parts, INT d);

static void calcCmonBorder(INT *i_cmon, INT *i_tran, INT *v_bord,
                           INT *length_v_bord, INT tran, INT bufferFrameStart,
                           INT numberTimeSlots);

static void keepForFollowUp(INT *v_bordFollow, INT *length_v_bordFollow,
                            INT *v_freqFollow, INT *length_v_freqFollow,
                            INT *i_tranFollow, INT *i_fillFollow, INT *v_bord,
                            INT *length_v_bord, INT *v_freq, INT i_cmon,
                            INT i_tran, INT parts, INT numberTimeSlots);

static void calcCtrlSignal(HANDLE_SBR_GRID hSbrGrid, FRAME_CLASS frameClass,
                           INT *v_bord, INT length_v_bord, INT *v_freq,
                           INT length_v_freq, INT i_cmon, INT i_tran,
                           INT spreadFlag, INT nL);

static void ctrlSignal2FrameInfo(HANDLE_SBR_GRID hSbrGrid,
                                 HANDLE_SBR_FRAME_INFO hFrameInfo,
                                 FREQ_RES *freq_res_fixfix);

/* table for 8 time slot index */
static const int envelopeTable_8[8][5] = {
    /* transientIndex  nEnv, tranIdx, shortEnv, border1, border2, ... */
    /* borders from left to right side; -1 = not in use */
    /*[|T-|------]*/ {2, 0, 0, 1, -1},
    /*[|-T-|-----]*/ {2, 0, 0, 2, -1},
    /*[--|T-|----]*/ {3, 1, 1, 2, 4},
    /*[---|T-|---]*/ {3, 1, 1, 3, 5},
    /*[----|T-|--]*/ {3, 1, 1, 4, 6},
    /*[-----|T--|]*/ {2, 1, 1, 5, -1},
    /*[------|T-|]*/ {2, 1, 1, 6, -1},
    /*[-------|T|]*/ {2, 1, 1, 7, -1},
};

/* table for 16 time slot index */
static const int envelopeTable_16[16][6] = {
    /* transientIndex  nEnv, tranIdx, shortEnv, border1, border2, ... */
    /* length from left to right side; -1 = not in use */
    /*[|T---|------------|]*/ {2, 0, 0, 4, -1, -1},
    /*[|-T---|-----------|]*/ {2, 0, 0, 5, -1, -1},
    /*[|--|T---|----------]*/ {3, 1, 1, 2, 6, -1},
    /*[|---|T---|---------]*/ {3, 1, 1, 3, 7, -1},
    /*[|----|T---|--------]*/ {3, 1, 1, 4, 8, -1},
    /*[|-----|T---|-------]*/ {3, 1, 1, 5, 9, -1},
    /*[|------|T---|------]*/ {3, 1, 1, 6, 10, -1},
    /*[|-------|T---|-----]*/ {3, 1, 1, 7, 11, -1},
    /*[|--------|T---|----]*/ {3, 1, 1, 8, 12, -1},
    /*[|---------|T---|---]*/ {3, 1, 1, 9, 13, -1},
    /*[|----------|T---|--]*/ {3, 1, 1, 10, 14, -1},
    /*[|-----------|T----|]*/ {2, 1, 1, 11, -1, -1},
    /*[|------------|T---|]*/ {2, 1, 1, 12, -1, -1},
    /*[|-------------|T--|]*/ {2, 1, 1, 13, -1, -1},
    /*[|--------------|T-|]*/ {2, 1, 1, 14, -1, -1},
    /*[|---------------|T|]*/ {2, 1, 1, 15, -1, -1},
};

/* table for 15 time slot index */
static const int envelopeTable_15[15][6] = {
    /* transientIndex  nEnv, tranIdx, shortEnv, border1, border2, ... */
    /* length from left to right side; -1 = not in use */
    /*[|T---|------------]*/ {2, 0, 0, 4, -1, -1},
    /*[|-T---|-----------]*/ {2, 0, 0, 5, -1, -1},
    /*[|--|T---|---------]*/ {3, 1, 1, 2, 6, -1},
    /*[|---|T---|--------]*/ {3, 1, 1, 3, 7, -1},
    /*[|----|T---|-------]*/ {3, 1, 1, 4, 8, -1},
    /*[|-----|T---|------]*/ {3, 1, 1, 5, 9, -1},
    /*[|------|T---|-----]*/ {3, 1, 1, 6, 10, -1},
    /*[|-------|T---|----]*/ {3, 1, 1, 7, 11, -1},
    /*[|--------|T---|---]*/ {3, 1, 1, 8, 12, -1},
    /*[|---------|T---|--]*/ {3, 1, 1, 9, 13, -1},
    /*[|----------|T----|]*/ {2, 1, 1, 10, -1, -1},
    /*[|-----------|T---|]*/ {2, 1, 1, 11, -1, -1},
    /*[|------------|T--|]*/ {2, 1, 1, 12, -1, -1},
    /*[|-------------|T-|]*/ {2, 1, 1, 13, -1, -1},
    /*[|--------------|T|]*/ {2, 1, 1, 14, -1, -1},
};

static const int minFrameTranDistance = 4;

static const FREQ_RES freqRes_table_8[] = {
    FREQ_RES_LOW,  FREQ_RES_LOW,  FREQ_RES_LOW,  FREQ_RES_LOW, FREQ_RES_LOW,
    FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH, FREQ_RES_HIGH};

static const FREQ_RES freqRes_table_16[16] = {
    /* size of envelope */
    /* 0-4 */ FREQ_RES_LOW,
    FREQ_RES_LOW,
    FREQ_RES_LOW,
    FREQ_RES_LOW,
    FREQ_RES_LOW,
    /* 5-9 */ FREQ_RES_LOW,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    /* 10-16 */ FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH,
    FREQ_RES_HIGH};

static void generateFixFixOnly(HANDLE_SBR_FRAME_INFO hSbrFrameInfo,
                               HANDLE_SBR_GRID hSbrGrid, int tranPosInternal,
                               int numberTimeSlots, UCHAR fResTransIsLow);

/*!
  Functionname: FDKsbrEnc_frameInfoGenerator

  Description:  produces the FRAME_INFO struct for the current frame

  Arguments:    hSbrEnvFrame          - pointer to sbr envelope handle
                v_pre_transient_info  - pointer to transient info vector
                v_transient_info      - pointer to previous transient info
vector v_tuning              - pointer to tuning vector

 Return:      frame_info        - pointer to SBR_FRAME_INFO struct

*******************************************************************************/
HANDLE_SBR_FRAME_INFO
FDKsbrEnc_frameInfoGenerator(HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                             UCHAR *v_transient_info, const INT rightBorderFIX,
                             UCHAR *v_transient_info_pre, int ldGrid,
                             const int *v_tuning) {
  INT numEnv, tranPosInternal = 0, bmin = 0, bmax = 0, parts, d, i_cmon = 0,
              i_tran = 0, nL;
  INT fmax = 0;

  INT *v_bord = hSbrEnvFrame->v_bord;
  INT *v_freq = hSbrEnvFrame->v_freq;
  INT *v_bordFollow = hSbrEnvFrame->v_bordFollow;
  INT *v_freqFollow = hSbrEnvFrame->v_freqFollow;

  INT *length_v_bordFollow = &hSbrEnvFrame->length_v_bordFollow;
  INT *length_v_freqFollow = &hSbrEnvFrame->length_v_freqFollow;
  INT *length_v_bord = &hSbrEnvFrame->length_v_bord;
  INT *length_v_freq = &hSbrEnvFrame->length_v_freq;
  INT *spreadFlag = &hSbrEnvFrame->spreadFlag;
  INT *i_tranFollow = &hSbrEnvFrame->i_tranFollow;
  INT *i_fillFollow = &hSbrEnvFrame->i_fillFollow;
  FRAME_CLASS *frameClassOld = &hSbrEnvFrame->frameClassOld;
  FRAME_CLASS frameClass = FIXFIX;

  INT allowSpread = hSbrEnvFrame->allowSpread;
  INT numEnvStatic = hSbrEnvFrame->numEnvStatic;
  INT staticFraming = hSbrEnvFrame->staticFraming;
  INT dmin = hSbrEnvFrame->dmin;
  INT dmax = hSbrEnvFrame->dmax;

  INT bufferFrameStart = hSbrEnvFrame->SbrGrid.bufferFrameStart;
  INT numberTimeSlots = hSbrEnvFrame->SbrGrid.numberTimeSlots;
  INT frameMiddleSlot = hSbrEnvFrame->frameMiddleSlot;

  INT tranPos = v_transient_info[0];
  INT tranFlag = v_transient_info[1];

  const int *v_tuningSegm = v_tuning;
  const int *v_tuningFreq = v_tuning + 3;

  hSbrEnvFrame->v_tuningSegm = v_tuningSegm;

  if (ldGrid) {
    /* in case there was a transient at the very end of the previous frame,
     * start with a transient envelope */
    if (!tranFlag && v_transient_info_pre[1] &&
        (numberTimeSlots - v_transient_info_pre[0] < minFrameTranDistance)) {
      tranFlag = 1;
      tranPos = 0;
    }
  }

  /*
   * Synopsis:
   *
   * The frame generator creates the time-/frequency-grid for one SBR frame.
   * Input signals are provided by the transient detector and the frame
   * splitter (transientDetectNew() & FrameSplitter() in tran_det.c).  The
   * framing is controlled by adjusting tuning parameters stored in
   * FRAME_GEN_TUNING.  The parameter values are dependent on frame lengths
   * and bitrates, and may in the future be signal dependent.
   *
   * The envelope borders are stored for frame generator internal use in
   * aBorders.  The contents of aBorders represent positions along the time
   * axis given in the figures in fram_gen.h (the "frame-generator" rows).
   * The unit is "time slot".  The figures in fram_gen.h also define the
   * detection ranges for the transient detector.  For every border in
   * aBorders, there is a corresponding entry in aFreqRes, which defines the
   * frequency resolution of the envelope following (delimited by) the
   * border.
   *
   * When no transients are present, FIXFIX class frames are used.  The
   * frame splitter decides whether to use one or two envelopes in the
   * FIXFIX frame.  "Sparse transients" (separated by a few frames without
   * transients) are handeled by [FIXVAR, VARFIX] pairs or (depending on
   * tuning and transient position relative the nominal frame boundaries)
   * by [FIXVAR, VARVAR, VARFIX] triples.  "Tight transients" (in
   * consecutive frames) are handeled by [..., VARVAR, VARVAR, ...]
   * sequences.
   *
   * The generator assumes that transients are "sparse", and designs
   * borders for [FIXVAR, VARFIX] pairs right away, where the first frame
   * corresponds to the present frame.  At the next call of the generator
   * it is known whether the transient actually is "sparse" or not.  If
   * 'yes', the already calculated VARFIX borders are used.  If 'no', new
   * borders, meeting the requirements of the "tight" transient, are
   * calculated.
   *
   * The generator produces two outputs:  A "clear-text bitstream" stored in
   * SBR_GRID, and a straight-forward representation of the grid stored in
   * SBR_FRAME_INFO.  The former is subsequently converted to the actual
   * bitstream sbr_grid() (encodeSbrGrid() in bit_sbr.c).  The latter is
   * used by other encoder functions, such as the envelope estimator
   * (calculateSbrEnvelope() in env_est.c) and the noise floor and missing
   * harmonics detector (TonCorrParamExtr() in nf_est.c).
   */

  if (staticFraming) {
    /*--------------------------------------------------------------------------
      Ignore transient detector
    ---------------------------------------------------------------------------*/

    frameClass = FIXFIX;
    numEnv = numEnvStatic;   /* {1,2,4,8} */
    *frameClassOld = FIXFIX; /* for change to dyn */
    hSbrEnvFrame->SbrGrid.bs_num_env = numEnv;
    hSbrEnvFrame->SbrGrid.frameClass = frameClass;
  } else {
    /*--------------------------------------------------------------------------
      Calculate frame class to use
    ---------------------------------------------------------------------------*/
    if (rightBorderFIX) {
      tranFlag = 0;
      *spreadFlag = 0;
    }
    calcFrameClass(&frameClass, frameClassOld, tranFlag, spreadFlag);

    /* patch for new frame class FIXFIXonly for AAC LD */
    if (tranFlag && ldGrid) {
      frameClass = FIXFIXonly;
      *frameClassOld = FIXFIX;
    }

    /*
     * every transient is processed below by inserting
     *
     * - one border at the onset of the transient
     * - one or more "decay borders" (after the onset of the transient)
     * - optionally one "attack border" (before the onset of the transient)
     *
     * those borders are referred to as "mandatory borders" and are
     * defined by the 'segmentLength' array in FRAME_GEN_TUNING
     *
     * the frequency resolutions of the corresponding envelopes are
     * defined by the 'segmentRes' array in FRAME_GEN_TUNING
     */

    /*--------------------------------------------------------------------------
      Design frame (or follow-up old design)
    ---------------------------------------------------------------------------*/
    if (tranFlag) {
      /* Always for FixVar, often but not always for VarVar */

      /*--------------------------------------------------------------------------
        Design part of T/F-grid around the new transient
      ---------------------------------------------------------------------------*/

      tranPosInternal =
          frameMiddleSlot + tranPos + bufferFrameStart; /* FH 00-06-26 */
      /*
        add mandatory borders around transient
      */

      fillFrameTran(v_tuningSegm, v_tuningFreq, tranPosInternal, v_bord,
                    length_v_bord, v_freq, length_v_freq, &bmin, &bmax);

      /* make sure we stay within the maximum SBR frame overlap */
      fmax = calcFillLengthMax(tranPos, numberTimeSlots);
    }

    switch (frameClass) {
      case FIXFIXonly:
        FDK_ASSERT(ldGrid);
        tranPosInternal = tranPos;
        generateFixFixOnly(&(hSbrEnvFrame->SbrFrameInfo),
                           &(hSbrEnvFrame->SbrGrid), tranPosInternal,
                           numberTimeSlots, hSbrEnvFrame->fResTransIsLow);

        return &(hSbrEnvFrame->SbrFrameInfo);

      case FIXVAR:

        /*--------------------------------------------------------------------------
           Design remaining parts of T/F-grid (assuming next frame is VarFix)
        ---------------------------------------------------------------------------*/

        /*--------------------------------------------------------------------------
          Fill region before new transient:
        ---------------------------------------------------------------------------*/
        fillFramePre(dmax, v_bord, length_v_bord, v_freq, length_v_freq, bmin,
                     bmin - bufferFrameStart); /* FH 00-06-26 */

        /*--------------------------------------------------------------------------
          Fill region after new transient:
        ---------------------------------------------------------------------------*/
        fillFramePost(&parts, &d, dmax, v_bord, length_v_bord, v_freq,
                      length_v_freq, bmax, bufferFrameStart, numberTimeSlots,
                      fmax);

        /*--------------------------------------------------------------------------
          Take care of special case:
        ---------------------------------------------------------------------------*/
        if (parts == 1 && d < dmin) /* no fill, short last envelope */
          specialCase(spreadFlag, allowSpread, v_bord, length_v_bord, v_freq,
                      length_v_freq, &parts, d);

        /*--------------------------------------------------------------------------
          Calculate common border (split-point)
        ---------------------------------------------------------------------------*/
        calcCmonBorder(&i_cmon, &i_tran, v_bord, length_v_bord, tranPosInternal,
                       bufferFrameStart, numberTimeSlots); /* FH 00-06-26 */

        /*--------------------------------------------------------------------------
          Extract data for proper follow-up in next frame
        ---------------------------------------------------------------------------*/
        keepForFollowUp(v_bordFollow, length_v_bordFollow, v_freqFollow,
                        length_v_freqFollow, i_tranFollow, i_fillFollow, v_bord,
                        length_v_bord, v_freq, i_cmon, i_tran, parts,
                        numberTimeSlots); /* FH 00-06-26 */

        /*--------------------------------------------------------------------------
          Calculate control signal
        ---------------------------------------------------------------------------*/
        calcCtrlSignal(&hSbrEnvFrame->SbrGrid, frameClass, v_bord,
                       *length_v_bord, v_freq, *length_v_freq, i_cmon, i_tran,
                       *spreadFlag, DC);
        break;
      case VARFIX:
        /*--------------------------------------------------------------------------
          Follow-up old transient - calculate control signal
        ---------------------------------------------------------------------------*/
        calcCtrlSignal(&hSbrEnvFrame->SbrGrid, frameClass, v_bordFollow,
                       *length_v_bordFollow, v_freqFollow, *length_v_freqFollow,
                       DC, *i_tranFollow, *spreadFlag, DC);
        break;
      case VARVAR:
        if (*spreadFlag) { /* spread across three frames */
          /*--------------------------------------------------------------------------
            Follow-up old transient - calculate control signal
          ---------------------------------------------------------------------------*/
          calcCtrlSignal(&hSbrEnvFrame->SbrGrid, frameClass, v_bordFollow,
                         *length_v_bordFollow, v_freqFollow,
                         *length_v_freqFollow, DC, *i_tranFollow, *spreadFlag,
                         DC);

          *spreadFlag = 0;

          /*--------------------------------------------------------------------------
            Extract data for proper follow-up in next frame
          ---------------------------------------------------------------------------*/
          v_bordFollow[0] = hSbrEnvFrame->SbrGrid.bs_abs_bord_1 -
                            numberTimeSlots; /* FH 00-06-26 */
          v_freqFollow[0] = 1;
          *length_v_bordFollow = 1;
          *length_v_freqFollow = 1;

          *i_tranFollow = -DC;
          *i_fillFollow = -DC;
        } else {
          /*--------------------------------------------------------------------------
            Design remaining parts of T/F-grid (assuming next frame is VarFix)
            adapt or fill region before new transient:
          ---------------------------------------------------------------------------*/
          fillFrameInter(&nL, v_tuningSegm, v_bord, length_v_bord, bmin, v_freq,
                         length_v_freq, v_bordFollow, length_v_bordFollow,
                         v_freqFollow, length_v_freqFollow, *i_fillFollow, dmin,
                         dmax, numberTimeSlots);

          /*--------------------------------------------------------------------------
            Fill after transient:
          ---------------------------------------------------------------------------*/
          fillFramePost(&parts, &d, dmax, v_bord, length_v_bord, v_freq,
                        length_v_freq, bmax, bufferFrameStart, numberTimeSlots,
                        fmax);

          /*--------------------------------------------------------------------------
            Take care of special case:
          ---------------------------------------------------------------------------*/
          if (parts == 1 && d < dmin) /*% no fill, short last envelope */
            specialCase(spreadFlag, allowSpread, v_bord, length_v_bord, v_freq,
                        length_v_freq, &parts, d);

          /*--------------------------------------------------------------------------
            Calculate common border (split-point)
          ---------------------------------------------------------------------------*/
          calcCmonBorder(&i_cmon, &i_tran, v_bord, length_v_bord,
                         tranPosInternal, bufferFrameStart, numberTimeSlots);

          /*--------------------------------------------------------------------------
            Extract data for proper follow-up in next frame
          ---------------------------------------------------------------------------*/
          keepForFollowUp(v_bordFollow, length_v_bordFollow, v_freqFollow,
                          length_v_freqFollow, i_tranFollow, i_fillFollow,
                          v_bord, length_v_bord, v_freq, i_cmon, i_tran, parts,
                          numberTimeSlots);

          /*--------------------------------------------------------------------------
            Calculate control signal
          ---------------------------------------------------------------------------*/
          calcCtrlSignal(&hSbrEnvFrame->SbrGrid, frameClass, v_bord,
                         *length_v_bord, v_freq, *length_v_freq, i_cmon, i_tran,
                         0, nL);
        }
        break;
      case FIXFIX:
        if (tranPos == 0)
          numEnv = 1;
        else
          numEnv = 2;

        hSbrEnvFrame->SbrGrid.bs_num_env = numEnv;
        hSbrEnvFrame->SbrGrid.frameClass = frameClass;

        break;
      default:
        FDK_ASSERT(0);
    }
  }

  /*-------------------------------------------------------------------------
    Convert control signal to frame info struct
  ---------------------------------------------------------------------------*/
  ctrlSignal2FrameInfo(&hSbrEnvFrame->SbrGrid, &hSbrEnvFrame->SbrFrameInfo,
                       hSbrEnvFrame->freq_res_fixfix);

  return &hSbrEnvFrame->SbrFrameInfo;
}

/***************************************************************************/
/*!
  \brief    Gnerates frame info for FIXFIXonly frame class used for low delay
 version

  \return   nothing
 ****************************************************************************/
static void generateFixFixOnly(HANDLE_SBR_FRAME_INFO hSbrFrameInfo,
                               HANDLE_SBR_GRID hSbrGrid, int tranPosInternal,
                               int numberTimeSlots, UCHAR fResTransIsLow) {
  int nEnv, i, k = 0, tranIdx;
  const int *pTable = NULL;
  const FREQ_RES *freqResTable = NULL;

  switch (numberTimeSlots) {
    case 8: {
      pTable = envelopeTable_8[tranPosInternal];
    }
      freqResTable = freqRes_table_8;
      break;
    case 15:
      pTable = envelopeTable_15[tranPosInternal];
      freqResTable = freqRes_table_16;
      break;
    case 16:
      pTable = envelopeTable_16[tranPosInternal];
      freqResTable = freqRes_table_16;
      break;
  }

  /* look number of envolpes in table */
  nEnv = pTable[0];
  /* look up envolpe distribution in table */
  for (i = 1; i < nEnv; i++) hSbrFrameInfo->borders[i] = pTable[i + 2];

  /* open and close frame border */
  hSbrFrameInfo->borders[0] = 0;
  hSbrFrameInfo->borders[nEnv] = numberTimeSlots;

  /* adjust segment-frequency-resolution according to the segment-length */
  for (i = 0; i < nEnv; i++) {
    k = hSbrFrameInfo->borders[i + 1] - hSbrFrameInfo->borders[i];
    if (!fResTransIsLow)
      hSbrFrameInfo->freqRes[i] = freqResTable[k];
    else
      hSbrFrameInfo->freqRes[i] = FREQ_RES_LOW;

    hSbrGrid->v_f[i] = hSbrFrameInfo->freqRes[i];
  }

  hSbrFrameInfo->nEnvelopes = nEnv;
  hSbrFrameInfo->shortEnv = pTable[2];
  /* transient idx */
  tranIdx = pTable[1];

  /* add noise floors */
  hSbrFrameInfo->bordersNoise[0] = 0;
  hSbrFrameInfo->bordersNoise[1] =
      hSbrFrameInfo->borders[tranIdx ? tranIdx : 1];
  hSbrFrameInfo->bordersNoise[2] = numberTimeSlots;
  hSbrFrameInfo->nNoiseEnvelopes = 2;

  hSbrGrid->frameClass = FIXFIXonly;
  hSbrGrid->bs_abs_bord = tranPosInternal;
  hSbrGrid->bs_num_env = nEnv;
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_initFrameInfoGenerator
 *******************************************************************************

 Description:

 Arguments:   hSbrEnvFrame  - pointer to sbr envelope handle
              allowSpread   - commandline parameter
              numEnvStatic  - commandline parameter
              staticFraming - commandline parameter

 Return:      none

*******************************************************************************/
void FDKsbrEnc_initFrameInfoGenerator(HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                                      INT allowSpread, INT numEnvStatic,
                                      INT staticFraming, INT timeSlots,
                                      const FREQ_RES *freq_res_fixfix,
                                      UCHAR fResTransIsLow,
                                      INT ldGrid) { /* FH 00-06-26 */

  FDKmemclear(hSbrEnvFrame, sizeof(SBR_ENVELOPE_FRAME));

  /* Initialisation */
  hSbrEnvFrame->frameClassOld = FIXFIX;
  hSbrEnvFrame->spreadFlag = 0;

  hSbrEnvFrame->allowSpread = allowSpread;
  hSbrEnvFrame->numEnvStatic = numEnvStatic;
  hSbrEnvFrame->staticFraming = staticFraming;
  hSbrEnvFrame->freq_res_fixfix[0] = freq_res_fixfix[0];
  hSbrEnvFrame->freq_res_fixfix[1] = freq_res_fixfix[1];
  hSbrEnvFrame->fResTransIsLow = fResTransIsLow;

  hSbrEnvFrame->length_v_bord = 0;
  hSbrEnvFrame->length_v_bordFollow = 0;

  hSbrEnvFrame->length_v_freq = 0;
  hSbrEnvFrame->length_v_freqFollow = 0;

  hSbrEnvFrame->i_tranFollow = 0;
  hSbrEnvFrame->i_fillFollow = 0;

  hSbrEnvFrame->SbrGrid.numberTimeSlots = timeSlots;

  if (ldGrid) {
    /*case CODEC_AACLD:*/
    hSbrEnvFrame->dmin = 2;
    hSbrEnvFrame->dmax = 16;
    hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_512LD;
    hSbrEnvFrame->SbrGrid.bufferFrameStart = 0;
  } else
    switch (timeSlots) {
      case NUMBER_TIME_SLOTS_1920:
        hSbrEnvFrame->dmin = 4;
        hSbrEnvFrame->dmax = 12;
        hSbrEnvFrame->SbrGrid.bufferFrameStart = 0;
        hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_1920;
        break;
      case NUMBER_TIME_SLOTS_2048:
        hSbrEnvFrame->dmin = 4;
        hSbrEnvFrame->dmax = 12;
        hSbrEnvFrame->SbrGrid.bufferFrameStart = 0;
        hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_2048;
        break;
      case NUMBER_TIME_SLOTS_1152:
        hSbrEnvFrame->dmin = 2;
        hSbrEnvFrame->dmax = 8;
        hSbrEnvFrame->SbrGrid.bufferFrameStart = 0;
        hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_1152;
        break;
      case NUMBER_TIME_SLOTS_2304:
        hSbrEnvFrame->dmin = 4;
        hSbrEnvFrame->dmax = 15;
        hSbrEnvFrame->SbrGrid.bufferFrameStart = 0;
        hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_2304;
        break;
      default:
        FDK_ASSERT(0);
    }
}

/*******************************************************************************
 Functionname:  fillFrameTran
 *******************************************************************************

 Description:  Add mandatory borders, as described by the tuning vector
               and the current transient position

 Arguments:
      modified:
              v_bord        - int pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - int pointer to v_freq vector
              length_v_freq - length of v_freq vector
              bmin          - int pointer to bmin (call by reference)
              bmax          - int pointer to bmax (call by reference)
      not modified:
              tran          - position of transient
              v_tuningSegm  - int pointer to v_tuningSegm vector
              v_tuningFreq  - int pointer to v_tuningFreq vector

 Return:      none

*******************************************************************************/
static void fillFrameTran(
    const int *v_tuningSegm, /*!< tuning: desired segment lengths */
    const int *v_tuningFreq, /*!< tuning: desired frequency resolutions */
    int tran,                /*!< input : position of transient */
    int *v_bord,             /*!< memNew: borders */
    int *length_v_bord,      /*!< memNew: # borders */
    int *v_freq,             /*!< memNew: frequency resolutions */
    int *length_v_freq,      /*!< memNew: # frequency resolutions */
    int *bmin,               /*!< hlpNew: first mandatory border */
    int *bmax                /*!< hlpNew: last  mandatory border */
) {
  int bord, i;

  *length_v_bord = 0;
  *length_v_freq = 0;

  /* add attack env leading border (optional) */
  if (v_tuningSegm[0]) {
    /* v_bord = [(Ba)] start of attack env */
    FDKsbrEnc_AddRight(v_bord, length_v_bord, (tran - v_tuningSegm[0]));

    /* v_freq = [(Fa)] res of attack env */
    FDKsbrEnc_AddRight(v_freq, length_v_freq, v_tuningFreq[0]);
  }

  /* add attack env trailing border/first decay env leading border */
  bord = tran;
  FDKsbrEnc_AddRight(v_bord, length_v_bord, tran); /* v_bord = [(Ba),Bd1] */

  /* add first decay env trailing border/2:nd decay env leading border */
  if (v_tuningSegm[1]) {
    bord += v_tuningSegm[1];

    /* v_bord = [(Ba),Bd1,Bd2] */
    FDKsbrEnc_AddRight(v_bord, length_v_bord, bord);

    /* v_freq = [(Fa),Fd1] */
    FDKsbrEnc_AddRight(v_freq, length_v_freq, v_tuningFreq[1]);
  }

  /* add 2:nd decay env trailing border (optional) */
  if (v_tuningSegm[2] != 0) {
    bord += v_tuningSegm[2];

    /* v_bord = [(Ba),Bd1, Bd2,(Bd3)] */
    FDKsbrEnc_AddRight(v_bord, length_v_bord, bord);

    /* v_freq = [(Fa),Fd1,(Fd2)] */
    FDKsbrEnc_AddRight(v_freq, length_v_freq, v_tuningFreq[2]);
  }

  /*  v_freq = [(Fa),Fd1,(Fd2),1] */
  FDKsbrEnc_AddRight(v_freq, length_v_freq, 1);

  /*  calc min and max values of mandatory borders */
  *bmin = v_bord[0];
  for (i = 0; i < *length_v_bord; i++)
    if (v_bord[i] < *bmin) *bmin = v_bord[i];

  *bmax = v_bord[0];
  for (i = 0; i < *length_v_bord; i++)
    if (v_bord[i] > *bmax) *bmax = v_bord[i];
}

/*******************************************************************************
 Functionname:  fillFramePre
 *******************************************************************************

 Description: Add borders before mandatory borders, if needed

 Arguments:
       modified:
              v_bord        - int pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - int pointer to v_freq vector
              length_v_freq - length of v_freq vector
       not modified:
              dmax          - int value
              bmin          - int value
              rest          - int value

 Return:      none

*******************************************************************************/
static void fillFramePre(INT dmax, INT *v_bord, INT *length_v_bord, INT *v_freq,
                         INT *length_v_freq, INT bmin, INT rest) {
  /*
    input state:
    v_bord = [(Ba),Bd1, Bd2 ,(Bd3)]
    v_freq = [(Fa),Fd1,(Fd2),1 ]
  */

  INT parts, d, j, S, s = 0, segm, bord;

  /*
    start with one envelope
  */

  parts = 1;
  d = rest;

  /*
    calc # of additional envelopes and corresponding lengths
  */

  while (d > dmax) {
    parts++;

    segm = rest / parts;
    S = (segm - 2) >> 1;
    s = fixMin(8, 2 * S + 2);
    d = rest - (parts - 1) * s;
  }

  /*
    add borders before mandatory borders
  */

  bord = bmin;

  for (j = 0; j <= parts - 2; j++) {
    bord = bord - s;

    /* v_bord = [...,(Bf),(Ba),Bd1, Bd2 ,(Bd3)] */
    FDKsbrEnc_AddLeft(v_bord, length_v_bord, bord);

    /* v_freq = [...,(1 ),(Fa),Fd1,(Fd2), 1   ] */
    FDKsbrEnc_AddLeft(v_freq, length_v_freq, 1);
  }
}

/***************************************************************************/
/*!
  \brief Overlap control

  Calculate max length of trailing fill segments, such that we always get a
  border within the frame overlap region

  \return void

****************************************************************************/
static int calcFillLengthMax(
    int tranPos,        /*!< input : transient position (ref: tran det) */
    int numberTimeSlots /*!< input : number of timeslots */
) {
  int fmax;

  /*
    calculate transient position within envelope buffer
  */
  switch (numberTimeSlots) {
    case NUMBER_TIME_SLOTS_2048:
      if (tranPos < 4)
        fmax = 6;
      else if (tranPos == 4 || tranPos == 5)
        fmax = 4;
      else
        fmax = 8;
      break;

    case NUMBER_TIME_SLOTS_1920:
      if (tranPos < 4)
        fmax = 5;
      else if (tranPos == 4 || tranPos == 5)
        fmax = 3;
      else
        fmax = 7;
      break;

    default:
      fmax = 8;
      break;
  }

  return fmax;
}

/*******************************************************************************
 Functionname:  fillFramePost
 *******************************************************************************

 Description: -Add borders after mandatory borders, if needed
               Make a preliminary design of next frame,
               assuming no transient is present there

 Arguments:
       modified:
              parts         - int pointer to parts (call by reference)
              d             - int pointer to d (call by reference)
              v_bord        - int pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - int pointer to v_freq vector
              length_v_freq - length of v_freq vector
        not modified:
              bmax          - int value
              dmax          - int value

 Return:      none

*******************************************************************************/
static void fillFramePost(INT *parts, INT *d, INT dmax, INT *v_bord,
                          INT *length_v_bord, INT *v_freq, INT *length_v_freq,
                          INT bmax, INT bufferFrameStart, INT numberTimeSlots,
                          INT fmax) {
  INT j, rest, segm, S, s = 0, bord;

  /*
    input state:
    v_bord = [...,(Bf),(Ba),Bd1, Bd2 ,(Bd3)]
    v_freq = [...,(1 ),(Fa),Fd1,(Fd2),1    ]
  */

  rest = bufferFrameStart + 2 * numberTimeSlots - bmax;
  *d = rest;

  if (*d > 0) {
    *parts = 1; /* start with one envelope */

    /* calc # of additional envelopes and corresponding lengths */

    while (*d > dmax) {
      *parts = *parts + 1;

      segm = rest / (*parts);
      S = (segm - 2) >> 1;
      s = fixMin(fmax, 2 * S + 2);
      *d = rest - (*parts - 1) * s;
    }

    /* add borders after mandatory borders */

    bord = bmax;
    for (j = 0; j <= *parts - 2; j++) {
      bord += s;

      /* v_bord =  [...,(Bf),(Ba),Bd1, Bd2 ,(Bd3),(Bf)] */
      FDKsbrEnc_AddRight(v_bord, length_v_bord, bord);

      /* v_freq =  [...,(1 ),(Fa),Fd1,(Fd2), 1   , 1! ,1] */
      FDKsbrEnc_AddRight(v_freq, length_v_freq, 1);
    }
  } else {
    *parts = 1;

    /* remove last element from v_bord and v_freq */

    *length_v_bord = *length_v_bord - 1;
    *length_v_freq = *length_v_freq - 1;
  }
}

/*******************************************************************************
 Functionname:  fillFrameInter
 *******************************************************************************

 Description:

 Arguments:   nL                  -
              v_tuningSegm        -
              v_bord              -
              length_v_bord       -
              bmin                -
              v_freq              -
              length_v_freq       -
              v_bordFollow        -
              length_v_bordFollow -
              v_freqFollow        -
              length_v_freqFollow -
              i_fillFollow        -
              dmin                -
              dmax                -

 Return:      none

*******************************************************************************/
static void fillFrameInter(INT *nL, const int *v_tuningSegm, INT *v_bord,
                           INT *length_v_bord, INT bmin, INT *v_freq,
                           INT *length_v_freq, INT *v_bordFollow,
                           INT *length_v_bordFollow, INT *v_freqFollow,
                           INT *length_v_freqFollow, INT i_fillFollow, INT dmin,
                           INT dmax, INT numberTimeSlots) {
  INT middle, b_new, numBordFollow, bordMaxFollow, i;

  if (numberTimeSlots != NUMBER_TIME_SLOTS_1152) {
    /* % remove fill borders: */
    if (i_fillFollow >= 1) {
      *length_v_bordFollow = i_fillFollow;
      *length_v_freqFollow = i_fillFollow;
    }

    numBordFollow = *length_v_bordFollow;
    bordMaxFollow = v_bordFollow[numBordFollow - 1];

    /* remove even more borders if needed */
    middle = bmin - bordMaxFollow;
    while (middle < 0) {
      numBordFollow--;
      bordMaxFollow = v_bordFollow[numBordFollow - 1];
      middle = bmin - bordMaxFollow;
    }

    *length_v_bordFollow = numBordFollow;
    *length_v_freqFollow = numBordFollow;
    *nL = numBordFollow - 1;

    b_new = *length_v_bord;

    if (middle <= dmax) {
      if (middle >= dmin) { /* concatenate */
        FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                             *length_v_bordFollow);
        FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                             *length_v_freqFollow);
      }

      else {
        if (v_tuningSegm[0] != 0) { /* remove one new border and concatenate */
          *length_v_bord = b_new - 1;
          FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                               *length_v_bordFollow);

          *length_v_freq = b_new - 1;
          FDKsbrEnc_AddVecLeft(v_freq + 1, length_v_freq, v_freqFollow,
                               *length_v_freqFollow);
        } else {
          if (*length_v_bordFollow >
              1) { /* remove one old border and concatenate */
            FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                                 *length_v_bordFollow - 1);
            FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                                 *length_v_bordFollow - 1);

            *nL = *nL - 1;
          } else { /* remove new "transient" border and concatenate */

            for (i = 0; i < *length_v_bord - 1; i++) v_bord[i] = v_bord[i + 1];

            for (i = 0; i < *length_v_freq - 1; i++) v_freq[i] = v_freq[i + 1];

            *length_v_bord = b_new - 1;
            *length_v_freq = b_new - 1;

            FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                                 *length_v_bordFollow);
            FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                                 *length_v_freqFollow);
          }
        }
      }
    } else { /* middle > dmax */

      fillFramePre(dmax, v_bord, length_v_bord, v_freq, length_v_freq, bmin,
                   middle);
      FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                           *length_v_bordFollow);
      FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                           *length_v_freqFollow);
    }

  } else { /* numberTimeSlots==NUMBER_TIME_SLOTS_1152 */

    INT l, m;

    /*------------------------------------------------------------------------
      remove fill borders
      ------------------------------------------------------------------------*/
    if (i_fillFollow >= 1) {
      *length_v_bordFollow = i_fillFollow;
      *length_v_freqFollow = i_fillFollow;
    }

    numBordFollow = *length_v_bordFollow;
    bordMaxFollow = v_bordFollow[numBordFollow - 1];

    /*------------------------------------------------------------------------
      remove more borders if necessary to eliminate overlap
      ------------------------------------------------------------------------*/

    /* check for overlap */
    middle = bmin - bordMaxFollow;

    /* intervals:
       i)             middle <  0     : overlap, must remove borders
       ii)       0 <= middle <  dmin  : no overlap but too tight, must remove
       borders iii)   dmin <= middle <= dmax  : ok, just concatenate iv)    dmax
       <= middle          : too wide, must add borders
     */

    /* first remove old non-fill-borders... */
    while (middle < 0) {
      /* ...but don't remove all of them */
      if (numBordFollow == 1) break;

      numBordFollow--;
      bordMaxFollow = v_bordFollow[numBordFollow - 1];
      middle = bmin - bordMaxFollow;
    }

    /* if this isn't enough, remove new non-fill borders */
    if (middle < 0) {
      for (l = 0, m = 0; l < *length_v_bord; l++) {
        if (v_bord[l] > bordMaxFollow) {
          v_bord[m] = v_bord[l];
          v_freq[m] = v_freq[l];
          m++;
        }
      }

      *length_v_bord = l;
      *length_v_freq = l;

      bmin = v_bord[0];
    }

    /*------------------------------------------------------------------------
      update modified follow-up data
      ------------------------------------------------------------------------*/

    *length_v_bordFollow = numBordFollow;
    *length_v_freqFollow = numBordFollow;

    /* left relative borders correspond to follow-up */
    *nL = numBordFollow - 1;

    /*------------------------------------------------------------------------
      take care of intervals ii through iv
      ------------------------------------------------------------------------*/

    /* now middle should be >= 0 */
    middle = bmin - bordMaxFollow;

    if (middle <= dmin) /* (ii) */
    {
      b_new = *length_v_bord;

      if (v_tuningSegm[0] != 0) {
        /* remove new "luxury" border and concatenate */
        *length_v_bord = b_new - 1;
        FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                             *length_v_bordFollow);

        *length_v_freq = b_new - 1;
        FDKsbrEnc_AddVecLeft(v_freq + 1, length_v_freq, v_freqFollow,
                             *length_v_freqFollow);

      } else if (*length_v_bordFollow > 1) {
        /* remove old border and concatenate */
        FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                             *length_v_bordFollow - 1);
        FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                             *length_v_bordFollow - 1);

        *nL = *nL - 1;
      } else {
        /* remove new border and concatenate */
        for (i = 0; i < *length_v_bord - 1; i++) v_bord[i] = v_bord[i + 1];

        for (i = 0; i < *length_v_freq - 1; i++) v_freq[i] = v_freq[i + 1];

        *length_v_bord = b_new - 1;
        *length_v_freq = b_new - 1;

        FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                             *length_v_bordFollow);
        FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                             *length_v_freqFollow);
      }
    } else if ((middle >= dmin) && (middle <= dmax)) /* (iii) */
    {
      /* concatenate */
      FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                           *length_v_bordFollow);
      FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                           *length_v_freqFollow);

    } else /* (iv) */
    {
      fillFramePre(dmax, v_bord, length_v_bord, v_freq, length_v_freq, bmin,
                   middle);
      FDKsbrEnc_AddVecLeft(v_bord, length_v_bord, v_bordFollow,
                           *length_v_bordFollow);
      FDKsbrEnc_AddVecLeft(v_freq, length_v_freq, v_freqFollow,
                           *length_v_freqFollow);
    }
  }
}

/*******************************************************************************
 Functionname:  calcFrameClass
 *******************************************************************************

 Description:

 Arguments:  INT* frameClass, INT* frameClassOld, INT tranFlag, INT* spreadFlag)

 Return:      none

*******************************************************************************/
static void calcFrameClass(FRAME_CLASS *frameClass, FRAME_CLASS *frameClassOld,
                           INT tranFlag, INT *spreadFlag) {
  switch (*frameClassOld) {
    case FIXFIXonly:
    case FIXFIX:
      if (tranFlag)
        *frameClass = FIXVAR;
      else
        *frameClass = FIXFIX;
      break;
    case FIXVAR:
      if (tranFlag) {
        *frameClass = VARVAR;
        *spreadFlag = 0;
      } else {
        if (*spreadFlag)
          *frameClass = VARVAR;
        else
          *frameClass = VARFIX;
      }
      break;
    case VARFIX:
      if (tranFlag)
        *frameClass = FIXVAR;
      else
        *frameClass = FIXFIX;
      break;
    case VARVAR:
      if (tranFlag) {
        *frameClass = VARVAR;
        *spreadFlag = 0;
      } else {
        if (*spreadFlag)
          *frameClass = VARVAR;
        else
          *frameClass = VARFIX;
      }
      break;
  };

  *frameClassOld = *frameClass;
}

/*******************************************************************************
 Functionname:  specialCase
 *******************************************************************************

 Description:

 Arguments:   spreadFlag
              allowSpread
              v_bord
              length_v_bord
              v_freq
              length_v_freq
              parts
              d

 Return:      none

*******************************************************************************/
static void specialCase(INT *spreadFlag, INT allowSpread, INT *v_bord,
                        INT *length_v_bord, INT *v_freq, INT *length_v_freq,
                        INT *parts, INT d) {
  INT L;

  L = *length_v_bord;

  if (allowSpread) { /* add one "step 8" */
    *spreadFlag = 1;
    FDKsbrEnc_AddRight(v_bord, length_v_bord, v_bord[L - 1] + 8);
    FDKsbrEnc_AddRight(v_freq, length_v_freq, 1);
    (*parts)++;
  } else {
    if (d == 1) { /*  stretch one slot */
      *length_v_bord = L - 1;
      *length_v_freq = L - 1;
    } else {
      if ((v_bord[L - 1] - v_bord[L - 2]) > 2) { /* compress one quant step */
        v_bord[L - 1] = v_bord[L - 1] - 2;
        v_freq[*length_v_freq - 1] = 0; /* use low res for short segment */
      }
    }
  }
}

/*******************************************************************************
 Functionname:  calcCmonBorder
 *******************************************************************************

 Description:

 Arguments:   i_cmon
              i_tran
              v_bord
              length_v_bord
              tran

 Return:      none

*******************************************************************************/
static void calcCmonBorder(INT *i_cmon, INT *i_tran, INT *v_bord,
                           INT *length_v_bord, INT tran, INT bufferFrameStart,
                           INT numberTimeSlots) { /* FH 00-06-26 */
  INT i;

  for (i = 0; i < *length_v_bord; i++)
    if (v_bord[i] >= bufferFrameStart + numberTimeSlots) { /* FH 00-06-26 */
      *i_cmon = i;
      break;
    }

  /* keep track of transient: */
  for (i = 0; i < *length_v_bord; i++)
    if (v_bord[i] >= tran) {
      *i_tran = i;
      break;
    } else
      *i_tran = EMPTY;
}

/*******************************************************************************
 Functionname:  keepForFollowUp
 *******************************************************************************

 Description:

 Arguments:   v_bordFollow
              length_v_bordFollow
              v_freqFollow
              length_v_freqFollow
              i_tranFollow
              i_fillFollow
              v_bord
              length_v_bord
              v_freq
              i_cmon
              i_tran
              parts)

 Return:      none

*******************************************************************************/
static void keepForFollowUp(INT *v_bordFollow, INT *length_v_bordFollow,
                            INT *v_freqFollow, INT *length_v_freqFollow,
                            INT *i_tranFollow, INT *i_fillFollow, INT *v_bord,
                            INT *length_v_bord, INT *v_freq, INT i_cmon,
                            INT i_tran, INT parts,
                            INT numberTimeSlots) { /* FH 00-06-26 */
  INT L, i, j;

  L = *length_v_bord;

  (*length_v_bordFollow) = 0;
  (*length_v_freqFollow) = 0;

  for (j = 0, i = i_cmon; i < L; i++, j++) {
    v_bordFollow[j] = v_bord[i] - numberTimeSlots; /* FH 00-06-26 */
    v_freqFollow[j] = v_freq[i];
    (*length_v_bordFollow)++;
    (*length_v_freqFollow)++;
  }
  if (i_tran != EMPTY)
    *i_tranFollow = i_tran - i_cmon;
  else
    *i_tranFollow = EMPTY;
  *i_fillFollow = L - (parts - 1) - i_cmon;
}

/*******************************************************************************
 Functionname:  calcCtrlSignal
 *******************************************************************************

 Description:

 Arguments:   hSbrGrid
              frameClass
              v_bord
              length_v_bord
              v_freq
              length_v_freq
              i_cmon
              i_tran
              spreadFlag
              nL

 Return:      none

*******************************************************************************/
static void calcCtrlSignal(HANDLE_SBR_GRID hSbrGrid, FRAME_CLASS frameClass,
                           INT *v_bord, INT length_v_bord, INT *v_freq,
                           INT length_v_freq, INT i_cmon, INT i_tran,
                           INT spreadFlag, INT nL) {
  INT i, r, a, n, p, b, aL, aR, ntot, nmax, nR;

  INT *v_f = hSbrGrid->v_f;
  INT *v_fLR = hSbrGrid->v_fLR;
  INT *v_r = hSbrGrid->bs_rel_bord;
  INT *v_rL = hSbrGrid->bs_rel_bord_0;
  INT *v_rR = hSbrGrid->bs_rel_bord_1;

  INT length_v_r = 0;
  INT length_v_rR = 0;
  INT length_v_rL = 0;

  switch (frameClass) {
    case FIXVAR:
      /* absolute border: */

      a = v_bord[i_cmon];

      /* relative borders: */
      length_v_r = 0;
      i = i_cmon;

      while (i >= 1) {
        r = v_bord[i] - v_bord[i - 1];
        FDKsbrEnc_AddRight(v_r, &length_v_r, r);
        i--;
      }

      /*  number of relative borders: */
      n = length_v_r;

      /* freq res: */
      for (i = 0; i < i_cmon; i++) v_f[i] = v_freq[i_cmon - 1 - i];
      v_f[i_cmon] = 1;

      /* pointer: */
      p = (i_cmon >= i_tran && i_tran != EMPTY) ? (i_cmon - i_tran + 1) : (0);

      hSbrGrid->frameClass = frameClass;
      hSbrGrid->bs_abs_bord = a;
      hSbrGrid->n = n;
      hSbrGrid->p = p;

      break;
    case VARFIX:
      /* absolute border: */
      a = v_bord[0];

      /* relative borders: */
      length_v_r = 0;

      for (i = 1; i < length_v_bord; i++) {
        r = v_bord[i] - v_bord[i - 1];
        FDKsbrEnc_AddRight(v_r, &length_v_r, r);
      }

      /* number of relative borders: */
      n = length_v_r;

      /* freq res: */
      FDKmemcpy(v_f, v_freq, length_v_freq * sizeof(INT));

      /* pointer: */
      p = (i_tran >= 0 && i_tran != EMPTY) ? (i_tran + 1) : (0);

      hSbrGrid->frameClass = frameClass;
      hSbrGrid->bs_abs_bord = a;
      hSbrGrid->n = n;
      hSbrGrid->p = p;

      break;
    case VARVAR:
      if (spreadFlag) {
        /* absolute borders: */
        b = length_v_bord;

        aL = v_bord[0];
        aR = v_bord[b - 1];

        /* number of relative borders:    */
        ntot = b - 2;

        nmax = 2; /* n: {0,1,2} */
        if (ntot > nmax) {
          nL = nmax;
          nR = ntot - nmax;
        } else {
          nL = ntot;
          nR = 0;
        }

        /* relative borders: */
        length_v_rL = 0;
        for (i = 1; i <= nL; i++) {
          r = v_bord[i] - v_bord[i - 1];
          FDKsbrEnc_AddRight(v_rL, &length_v_rL, r);
        }

        length_v_rR = 0;
        i = b - 1;
        while (i >= b - nR) {
          r = v_bord[i] - v_bord[i - 1];
          FDKsbrEnc_AddRight(v_rR, &length_v_rR, r);
          i--;
        }

        /* pointer (only one due to constraint in frame info): */
        p = (i_tran > 0 && i_tran != EMPTY) ? (b - i_tran) : (0);

        /* freq res: */

        for (i = 0; i < b - 1; i++) v_fLR[i] = v_freq[i];
      } else {
        length_v_bord = i_cmon + 1;

        /* absolute borders: */
        b = length_v_bord;

        aL = v_bord[0];
        aR = v_bord[b - 1];

        /* number of relative borders:   */
        ntot = b - 2;
        nR = ntot - nL;

        /* relative borders: */
        length_v_rL = 0;
        for (i = 1; i <= nL; i++) {
          r = v_bord[i] - v_bord[i - 1];
          FDKsbrEnc_AddRight(v_rL, &length_v_rL, r);
        }

        length_v_rR = 0;
        i = b - 1;
        while (i >= b - nR) {
          r = v_bord[i] - v_bord[i - 1];
          FDKsbrEnc_AddRight(v_rR, &length_v_rR, r);
          i--;
        }

        /* pointer (only one due to constraint in frame info): */
        p = (i_cmon >= i_tran && i_tran != EMPTY) ? (i_cmon - i_tran + 1) : (0);

        /* freq res: */
        for (i = 0; i < b - 1; i++) v_fLR[i] = v_freq[i];
      }

      hSbrGrid->frameClass = frameClass;
      hSbrGrid->bs_abs_bord_0 = aL;
      hSbrGrid->bs_abs_bord_1 = aR;
      hSbrGrid->bs_num_rel_0 = nL;
      hSbrGrid->bs_num_rel_1 = nR;
      hSbrGrid->p = p;

      break;

    default:
      /* do nothing */
      break;
  }
}

/*******************************************************************************
 Functionname:  createDefFrameInfo
 *******************************************************************************

 Description: Copies the default (static) frameInfo structs to the frameInfo
              passed by reference; only used for FIXFIX frames

 Arguments:   hFrameInfo             - HANLDE_SBR_FRAME_INFO
              nEnv                   - INT
              nTimeSlots             - INT

 Return:      none; hSbrFrameInfo contains a copy of the default frameInfo

 Written:     Andreas Schneider
 Revised:
*******************************************************************************/
static void createDefFrameInfo(HANDLE_SBR_FRAME_INFO hSbrFrameInfo, INT nEnv,
                               INT nTimeSlots) {
  switch (nEnv) {
    case 1:
      switch (nTimeSlots) {
        case NUMBER_TIME_SLOTS_1920:
          FDKmemcpy(hSbrFrameInfo, &frameInfo1_1920, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2048:
          FDKmemcpy(hSbrFrameInfo, &frameInfo1_2048, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_1152:
          FDKmemcpy(hSbrFrameInfo, &frameInfo1_1152, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2304:
          FDKmemcpy(hSbrFrameInfo, &frameInfo1_2304, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_512LD:
          FDKmemcpy(hSbrFrameInfo, &frameInfo1_512LD, sizeof(SBR_FRAME_INFO));
          break;
        default:
          FDK_ASSERT(0);
      }
      break;
    case 2:
      switch (nTimeSlots) {
        case NUMBER_TIME_SLOTS_1920:
          FDKmemcpy(hSbrFrameInfo, &frameInfo2_1920, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2048:
          FDKmemcpy(hSbrFrameInfo, &frameInfo2_2048, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_1152:
          FDKmemcpy(hSbrFrameInfo, &frameInfo2_1152, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2304:
          FDKmemcpy(hSbrFrameInfo, &frameInfo2_2304, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_512LD:
          FDKmemcpy(hSbrFrameInfo, &frameInfo2_512LD, sizeof(SBR_FRAME_INFO));
          break;
        default:
          FDK_ASSERT(0);
      }
      break;
    case 4:
      switch (nTimeSlots) {
        case NUMBER_TIME_SLOTS_1920:
          FDKmemcpy(hSbrFrameInfo, &frameInfo4_1920, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2048:
          FDKmemcpy(hSbrFrameInfo, &frameInfo4_2048, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_1152:
          FDKmemcpy(hSbrFrameInfo, &frameInfo4_1152, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_2304:
          FDKmemcpy(hSbrFrameInfo, &frameInfo4_2304, sizeof(SBR_FRAME_INFO));
          break;
        case NUMBER_TIME_SLOTS_512LD:
          FDKmemcpy(hSbrFrameInfo, &frameInfo4_512LD, sizeof(SBR_FRAME_INFO));
          break;
        default:
          FDK_ASSERT(0);
      }
      break;
    default:
      FDK_ASSERT(0);
  }
}

/*******************************************************************************
 Functionname:  ctrlSignal2FrameInfo
 *******************************************************************************

 Description: Convert "clear-text" sbr_grid() to "frame info" used by the
              envelope and noise floor estimators.
              This is basically (except for "low level" calculations) the
              bitstream decoder defined in the MPEG-4 standard, sub clause
              4.6.18.3.3, Time / Frequency Grid.  See inline comments for
              explanation of the shorten and noise border algorithms.

 Arguments:   hSbrGrid - source
              hSbrFrameInfo - destination
              freq_res_fixfix - frequency resolution for FIXFIX frames

 Return:      void; hSbrFrameInfo contains the updated FRAME_INFO struct

*******************************************************************************/
static void ctrlSignal2FrameInfo(
    HANDLE_SBR_GRID hSbrGrid,            /* input : the grid handle       */
    HANDLE_SBR_FRAME_INFO hSbrFrameInfo, /* output: the frame info handle */
    FREQ_RES
        *freq_res_fixfix /* in/out: frequency resolution for FIXFIX frames */
) {
  INT frameSplit = 0;
  INT nEnv = 0, border = 0, i, k, p /*?*/;
  INT *v_r = hSbrGrid->bs_rel_bord;
  INT *v_f = hSbrGrid->v_f;

  FRAME_CLASS frameClass = hSbrGrid->frameClass;
  INT bufferFrameStart = hSbrGrid->bufferFrameStart;
  INT numberTimeSlots = hSbrGrid->numberTimeSlots;

  switch (frameClass) {
    case FIXFIX:
      createDefFrameInfo(hSbrFrameInfo, hSbrGrid->bs_num_env, numberTimeSlots);

      frameSplit = (hSbrFrameInfo->nEnvelopes > 1);
      for (i = 0; i < hSbrFrameInfo->nEnvelopes; i++) {
        hSbrGrid->v_f[i] = hSbrFrameInfo->freqRes[i] =
            freq_res_fixfix[frameSplit];
      }
      break;

    case FIXVAR:
    case VARFIX:
      nEnv = hSbrGrid->n + 1; /* read n [SBR_NUM_BITS bits] */ /*? snd*/
      FDK_ASSERT(nEnv <= MAX_ENVELOPES_FIXVAR_VARFIX);

      hSbrFrameInfo->nEnvelopes = nEnv;

      border = hSbrGrid->bs_abs_bord; /* read the absolute border */

      if (nEnv == 1)
        hSbrFrameInfo->nNoiseEnvelopes = 1;
      else
        hSbrFrameInfo->nNoiseEnvelopes = 2;

      break;

    default:
      /* do nothing */
      break;
  }

  switch (frameClass) {
    case FIXVAR:
      hSbrFrameInfo->borders[0] =
          bufferFrameStart; /* start-position of 1st envelope */

      hSbrFrameInfo->borders[nEnv] = border;

      for (k = 0, i = nEnv - 1; k < nEnv - 1; k++, i--) {
        border -= v_r[k];
        hSbrFrameInfo->borders[i] = border;
      }

      /* make either envelope nr. nEnv + 1 - p short; or don't shorten if p == 0
       */
      p = hSbrGrid->p;
      if (p == 0) {
        hSbrFrameInfo->shortEnv = 0;
      } else {
        hSbrFrameInfo->shortEnv = nEnv + 1 - p;
      }

      for (k = 0, i = nEnv - 1; k < nEnv; k++, i--) {
        hSbrFrameInfo->freqRes[i] = (FREQ_RES)v_f[k];
      }

      /* if either there is no short envelope or the last envelope is short...
       */
      if (p == 0 || p == 1) {
        hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];
      } else {
        hSbrFrameInfo->bordersNoise[1] =
            hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];
      }

      break;

    case VARFIX:
      /* in this case 'border' indicates the start of the 1st envelope */
      hSbrFrameInfo->borders[0] = border;

      for (k = 0; k < nEnv - 1; k++) {
        border += v_r[k];
        hSbrFrameInfo->borders[k + 1] = border;
      }

      hSbrFrameInfo->borders[nEnv] = bufferFrameStart + numberTimeSlots;

      p = hSbrGrid->p;
      if (p == 0 || p == 1) {
        hSbrFrameInfo->shortEnv = 0;
      } else {
        hSbrFrameInfo->shortEnv = p - 1;
      }

      for (k = 0; k < nEnv; k++) {
        hSbrFrameInfo->freqRes[k] = (FREQ_RES)v_f[k];
      }

      switch (p) {
        case 0:
          hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[1];
          break;
        case 1:
          hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];
          break;
        default:
          hSbrFrameInfo->bordersNoise[1] =
              hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];
          break;
      }
      break;

    case VARVAR:
      nEnv = hSbrGrid->bs_num_rel_0 + hSbrGrid->bs_num_rel_1 + 1;
      FDK_ASSERT(nEnv <= MAX_ENVELOPES_VARVAR); /* just to be sure */
      hSbrFrameInfo->nEnvelopes = nEnv;

      hSbrFrameInfo->borders[0] = border = hSbrGrid->bs_abs_bord_0;

      for (k = 0, i = 1; k < hSbrGrid->bs_num_rel_0; k++, i++) {
        border += hSbrGrid->bs_rel_bord_0[k];
        hSbrFrameInfo->borders[i] = border;
      }

      border = hSbrGrid->bs_abs_bord_1;
      hSbrFrameInfo->borders[nEnv] = border;

      for (k = 0, i = nEnv - 1; k < hSbrGrid->bs_num_rel_1; k++, i--) {
        border -= hSbrGrid->bs_rel_bord_1[k];
        hSbrFrameInfo->borders[i] = border;
      }

      p = hSbrGrid->p;
      if (p == 0) {
        hSbrFrameInfo->shortEnv = 0;
      } else {
        hSbrFrameInfo->shortEnv = nEnv + 1 - p;
      }

      for (k = 0; k < nEnv; k++) {
        hSbrFrameInfo->freqRes[k] = (FREQ_RES)hSbrGrid->v_fLR[k];
      }

      if (nEnv == 1) {
        hSbrFrameInfo->nNoiseEnvelopes = 1;
        hSbrFrameInfo->bordersNoise[0] = hSbrGrid->bs_abs_bord_0;
        hSbrFrameInfo->bordersNoise[1] = hSbrGrid->bs_abs_bord_1;
      } else {
        hSbrFrameInfo->nNoiseEnvelopes = 2;
        hSbrFrameInfo->bordersNoise[0] = hSbrGrid->bs_abs_bord_0;

        if (p == 0 || p == 1) {
          hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];
        } else {
          hSbrFrameInfo->bordersNoise[1] =
              hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];
        }
        hSbrFrameInfo->bordersNoise[2] = hSbrGrid->bs_abs_bord_1;
      }
      break;

    default:
      /* do nothing */
      break;
  }

  if (frameClass == VARFIX || frameClass == FIXVAR) {
    hSbrFrameInfo->bordersNoise[0] = hSbrFrameInfo->borders[0];
    if (nEnv == 1) {
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv];
    } else {
      hSbrFrameInfo->bordersNoise[2] = hSbrFrameInfo->borders[nEnv];
    }
  }
}
