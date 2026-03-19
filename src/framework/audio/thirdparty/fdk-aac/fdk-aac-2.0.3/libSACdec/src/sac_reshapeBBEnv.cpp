/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2021 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Description: SAC Dec guided envelope shaping

*******************************************************************************/

#include "sac_reshapeBBEnv.h"

#include "sac_dec.h"
#include "sac_bitdec.h"
#include "sac_calcM1andM2.h"
#include "sac_reshapeBBEnv.h"
#include "sac_rom.h"

#define INP_DRY_WET 0
#define INP_DMX 1

#define SF_SHAPE 1
#define SF_DIV32 6
#define SF_FACTOR_SLOT 5

#define START_BB_ENV 0 /* 10 */
#define END_BB_ENV 9   /* 18 */

#define SF_ALPHA1 8
#define SF_BETA1 4

void initBBEnv(spatialDec *self, int initStatesFlag) {
  INT ch, k;

  for (ch = 0; ch < self->numOutputChannels; ch++) {
    k = row2channelGES[self->treeConfig][ch];
    self->row2channelDmxGES[ch] = k;
    if (k == -1) continue;

    switch (self->treeConfig) {
      case TREE_212:
        self->row2channelDmxGES[ch] = 0;
        break;
      default:;
    }
  }

  if (initStatesFlag) {
    for (k = 0; k < 2 * MAX_OUTPUT_CHANNELS + MAX_INPUT_CHANNELS; k++) {
      self->reshapeBBEnvState->normNrgPrev__FDK[k] =
          FL2FXCONST_DBL(0.5f); /* 32768.f*32768.f */
      self->reshapeBBEnvState->normNrgPrevSF[k] = DFRACT_BITS - 1;
      self->reshapeBBEnvState->partNrgPrevSF[k] = 0;
      self->reshapeBBEnvState->partNrgPrev2SF[k] = 0;
      self->reshapeBBEnvState->frameNrgPrevSF[k] = 0;
    }
  }

  self->reshapeBBEnvState->alpha__FDK =
      FL2FXCONST_DBL(0.99637845575f); /* FDKexp(-64 / (0.4f  * 44100)) */
  self->reshapeBBEnvState->beta__FDK =
      FL2FXCONST_DBL(0.96436909488f); /* FDKexp(-64 / (0.04f * 44100)) */
}

static inline void getSlotNrgHQ(FIXP_DBL *RESTRICT pReal,
                                FIXP_DBL *RESTRICT pImag,
                                FIXP_DBL *RESTRICT slotNrg, INT maxValSF,
                                INT hybBands) {
  INT qs;
  FIXP_DBL nrg;

  /* qs = 12, 13, 14 */
  slotNrg[0] = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  slotNrg[1] = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  slotNrg[2] = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  /* qs = 15 */
  slotNrg[3] = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  /* qs = 16, 17 */
  nrg = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
         (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  slotNrg[4] =
      nrg + ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
             (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  /* qs = 18, 19, 20 */
  nrg = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
         (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  nrg += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
          (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  slotNrg[5] =
      nrg + ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
             (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  /* qs = 21, 22 */
  nrg = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
         (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  slotNrg[6] =
      nrg + ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
             (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
  /* qs = 23, 24 */
  if (hybBands > 23) {
    slotNrg[6] += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                   (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    slotNrg[6] += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
                   (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    /* qs = 25, 26, 29, 28, 29 */
    nrg = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
           (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    nrg += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
            (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    nrg += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
            (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    nrg += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
            (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    slotNrg[7] =
        nrg + ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
               (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    /* qs = 30 ... min(41,hybBands-1) */
    nrg = ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
           (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    for (qs = 31; qs < hybBands; qs++) {
      nrg += ((fPow2Div2((*pReal++) << maxValSF) >> (SF_FACTOR_SLOT - 1)) +
              (fPow2Div2((*pImag++) << maxValSF) >> (SF_FACTOR_SLOT - 1)));
    }
    slotNrg[8] = nrg;
  } else {
    slotNrg[7] = (FIXP_DBL)0;
    slotNrg[8] = (FIXP_DBL)0;
  }
}

static inline void combineDryWet(FIXP_DBL *RESTRICT pReal,
                                 FIXP_DBL *RESTRICT pImag,
                                 FIXP_DBL *RESTRICT pHybOutputRealDry,
                                 FIXP_DBL *RESTRICT pHybOutputImagDry,
                                 FIXP_DBL *RESTRICT pHybOutputRealWet,
                                 FIXP_DBL *RESTRICT pHybOutputImagWet,
                                 INT cplxBands, INT hybBands) {
  INT qs;

  for (qs = 12; qs < cplxBands; qs++) {
    pReal[qs] = (pHybOutputRealDry[qs] >> 1) + (pHybOutputRealWet[qs] >> 1);
    pImag[qs] = (pHybOutputImagDry[qs] >> 1) + (pHybOutputImagWet[qs] >> 1);
  }
  for (; qs < hybBands; qs++) {
    pReal[qs] = (pHybOutputRealDry[qs] >> 1) + (pHybOutputRealWet[qs] >> 1);
  }
}

static inline void slotAmp(
    FIXP_DBL *RESTRICT slotAmp_dry, INT *RESTRICT slotAmp_dry_e,
    FIXP_DBL *RESTRICT slotAmp_wet, INT *RESTRICT slotAmp_wet_e,
    FIXP_DBL *RESTRICT pHybOutputRealDry, FIXP_DBL *RESTRICT pHybOutputImagDry,
    FIXP_DBL *RESTRICT pHybOutputRealWet, FIXP_DBL *RESTRICT pHybOutputImagWet,
    INT cplxBands, INT hybBands) {
  INT qs, s1, s2, headroom_dry, headroom_wet;
  FIXP_DBL dry, wet;

  /* headroom can be reduced by 1 bit due to use of fPow2Div2 */
  s1 = DFRACT_BITS - 1 - CntLeadingZeros(hybBands + cplxBands);
  headroom_dry = fMin(getScalefactor(pHybOutputRealDry, hybBands),
                      getScalefactor(pHybOutputImagDry, cplxBands));
  headroom_wet = fMin(getScalefactor(pHybOutputRealWet, hybBands),
                      getScalefactor(pHybOutputImagWet, cplxBands));

  dry = wet = FL2FXCONST_DBL(0.0f);
  for (qs = 0; qs < cplxBands; qs++) {
    /* sum up dry part */
    dry += (fPow2Div2(pHybOutputRealDry[qs] << headroom_dry) >> s1);
    dry += (fPow2Div2(pHybOutputImagDry[qs] << headroom_dry) >> s1);
    /* sum up wet part */
    wet += (fPow2Div2(pHybOutputRealWet[qs] << headroom_wet) >> s1);
    wet += (fPow2Div2(pHybOutputImagWet[qs] << headroom_wet) >> s1);
  }
  for (; qs < hybBands; qs++) {
    dry += (fPow2Div2(pHybOutputRealDry[qs] << headroom_dry) >> s1);
    wet += (fPow2Div2(pHybOutputRealWet[qs] << headroom_wet) >> s1);
  }

  /* consider fPow2Div2() */
  s1 += 1;

  /* normalize dry part, ensure that exponent is even */
  s2 = fixMax(0, CntLeadingZeros(dry) - 1);
  *slotAmp_dry = dry << s2;
  *slotAmp_dry_e = s1 - s2 - 2 * headroom_dry;
  if (*slotAmp_dry_e & 1) {
    *slotAmp_dry = *slotAmp_dry >> 1;
    *slotAmp_dry_e += 1;
  }

  /* normalize wet part, ensure that exponent is even */
  s2 = fixMax(0, CntLeadingZeros(wet) - 1);
  *slotAmp_wet = wet << s2;
  *slotAmp_wet_e = s1 - s2 - 2 * headroom_wet;
  if (*slotAmp_wet_e & 1) {
    *slotAmp_wet = *slotAmp_wet >> 1;
    *slotAmp_wet_e += 1;
  }
}

#if defined(__aarch64__)
__attribute__((noinline))
#endif
static void
shapeBBEnv(FIXP_DBL *pHybOutputRealDry, FIXP_DBL *pHybOutputImagDry,
           FIXP_DBL dryFac, INT scale, INT cplxBands, INT hybBands) {
  INT qs;

  if (scale == 0) {
    for (qs = 0; qs < cplxBands; qs++) {
      pHybOutputRealDry[qs] = fMultDiv2(pHybOutputRealDry[qs], dryFac);
      pHybOutputImagDry[qs] = fMultDiv2(pHybOutputImagDry[qs], dryFac);
    }
    for (; qs < hybBands; qs++) {
      pHybOutputRealDry[qs] = fMultDiv2(pHybOutputRealDry[qs], dryFac);
    }
  } else {
    for (qs = 0; qs < cplxBands; qs++) {
      pHybOutputRealDry[qs] = SATURATE_LEFT_SHIFT(
          fMultDiv2(pHybOutputRealDry[qs], dryFac), scale, DFRACT_BITS);
      pHybOutputImagDry[qs] = SATURATE_LEFT_SHIFT(
          fMultDiv2(pHybOutputImagDry[qs], dryFac), scale, DFRACT_BITS);
    }
    for (; qs < hybBands; qs++) {
      pHybOutputRealDry[qs] = SATURATE_LEFT_SHIFT(
          fMultDiv2(pHybOutputRealDry[qs], dryFac), scale, DFRACT_BITS);
    }
  }
}

static void extractBBEnv(spatialDec *self, INT inp, INT start, INT channels,
                         FIXP_DBL *pEnv, const SPATIAL_BS_FRAME *frame) {
  INT ch, pb, prevChOffs;
  INT clz, scale, scale_min, envSF;
  INT scaleCur, scalePrev, commonScale;
  INT slotNrgSF, partNrgSF, frameNrgSF;
  INT *pPartNrgPrevSF, *pFrameNrgPrevSF;
  INT *pNormNrgPrevSF, *pPartNrgPrev2SF;

  FIXP_DBL maxVal, env, frameNrg, normNrg;
  FIXP_DBL *pReal, *pImag;
  FIXP_DBL *partNrg, *partNrgPrev;

  C_ALLOC_SCRATCH_START(pScratchBuffer, FIXP_DBL,
                        (2 * 42 + MAX_PARAMETER_BANDS));
  C_ALLOC_SCRATCH_START(resPb, FIXP_DBL, (END_BB_ENV - START_BB_ENV));
  C_ALLOC_SCRATCH_START(resPbSF, INT, (END_BB_ENV - START_BB_ENV));

  FIXP_DBL *slotNrg = pScratchBuffer + (2 * 42);

  RESHAPE_BBENV_STATE *pBBEnvState = self->reshapeBBEnvState;

  FIXP_DBL alpha = pBBEnvState->alpha__FDK;
  /*FIXP_DBL  alpha1 = (FL2FXCONST_DBL(1.0f) - alpha) << SF_ALPHA1;*/
  FIXP_DBL alpha1 = ((FIXP_DBL)MAXVAL_DBL - alpha) << SF_ALPHA1;
  FIXP_DBL beta = pBBEnvState->beta__FDK;
  /*FIXP_DBL  beta1  = (FL2FXCONST_DBL(1.0f) - beta) << SF_BETA1;*/
  FIXP_DBL beta1 = ((FIXP_DBL)MAXVAL_DBL - beta) << SF_BETA1;

  INT shapeActiv = 1;
  INT hybBands = fixMin(42, self->hybridBands);
  INT staticScale = self->staticDecScale + (1);
  INT cplxBands;
  cplxBands = fixMin(42, self->hybridBands);

  for (ch = start; ch < channels; ch++) {
    if (inp == INP_DRY_WET) {
      INT ch2 = row2channelGES[self->treeConfig][ch];
      if (ch2 == -1) {
        continue;
      } else {
        if (frame->tempShapeEnableChannelGES[ch2]) {
          shapeActiv = 1;
        } else {
          shapeActiv = 0;
        }
      }
      prevChOffs = ch;
      pReal = pScratchBuffer;
      pImag = pScratchBuffer + 42;
      combineDryWet(pReal, pImag, self->hybOutputRealDry__FDK[ch],
                    self->hybOutputImagDry__FDK[ch],
                    self->hybOutputRealWet__FDK[ch],
                    self->hybOutputImagWet__FDK[ch], cplxBands, hybBands);
      clz = fMin(getScalefactor(&pReal[12], fMax(0, hybBands - 12)),
                 getScalefactor(&pImag[12], fMax(0, cplxBands - 12)));
    } else {
      prevChOffs = ch + self->numOutputChannels;
      pReal = self->hybInputReal__FDK[ch];
      pImag = self->hybInputImag__FDK[ch];
      clz = fMin(getScalefactor(&pReal[12], fMax(0, hybBands - 12)),
                 getScalefactor(&pImag[12], fMax(0, cplxBands - 12)));
    }

    partNrg = partNrgPrev = pBBEnvState->partNrgPrev__FDK[prevChOffs];
    pPartNrgPrevSF = &pBBEnvState->partNrgPrevSF[prevChOffs];
    pFrameNrgPrevSF = &pBBEnvState->frameNrgPrevSF[prevChOffs];
    pNormNrgPrevSF = &pBBEnvState->normNrgPrevSF[prevChOffs];
    pPartNrgPrev2SF = &pBBEnvState->partNrgPrev2SF[prevChOffs];

    /* calculate slot energy */
    {
      getSlotNrgHQ(&pReal[12], &pImag[12], slotNrg, clz,
                   fixMin(42, self->hybridBands)); /* scale slotNrg:
                                                      2*(staticScale-clz) +
                                                      SF_FACTOR_SLOT */
    }

    slotNrgSF = 2 * (staticScale - clz + ((inp == INP_DRY_WET) ? 1 : 0)) +
                SF_FACTOR_SLOT;
    frameNrgSF = 2 * (staticScale - clz + ((inp == INP_DRY_WET) ? 1 : 0)) +
                 SF_FACTOR_SLOT;

    partNrgSF = fixMax(slotNrgSF - SF_ALPHA1 + 1,
                       pPartNrgPrevSF[0] - pPartNrgPrev2SF[0] + 1);
    scalePrev = fixMax(fixMin(partNrgSF - pPartNrgPrevSF[0], DFRACT_BITS - 1),
                       -(DFRACT_BITS - 1));
    scaleCur =
        fixMax(fixMin(partNrgSF - slotNrgSF + SF_ALPHA1, DFRACT_BITS - 1),
               -(DFRACT_BITS - 1));

    maxVal = FL2FXCONST_DBL(0.0f);
    frameNrg = FL2FXCONST_DBL(0.0f);
    if ((scaleCur < 0) && (scalePrev < 0)) {
      scaleCur = -scaleCur;
      scalePrev = -scalePrev;
      for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
        partNrg[pb] = ((fMultDiv2(alpha1, slotNrg[pb]) << scaleCur) +
                       (fMultDiv2(alpha, partNrgPrev[pb]) << scalePrev))
                      << 1;
        maxVal |= partNrg[pb];
        frameNrg += slotNrg[pb] >> 3;
      }
    } else if ((scaleCur >= 0) && (scalePrev >= 0)) {
      for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
        partNrg[pb] = ((fMultDiv2(alpha1, slotNrg[pb]) >> scaleCur) +
                       (fMultDiv2(alpha, partNrgPrev[pb]) >> scalePrev))
                      << 1;
        maxVal |= partNrg[pb];
        frameNrg += slotNrg[pb] >> 3;
      }
    } else if ((scaleCur < 0) && (scalePrev >= 0)) {
      scaleCur = -scaleCur;
      for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
        partNrg[pb] = ((fMultDiv2(alpha1, slotNrg[pb]) << scaleCur) +
                       (fMultDiv2(alpha, partNrgPrev[pb]) >> scalePrev))
                      << 1;
        maxVal |= partNrg[pb];
        frameNrg += slotNrg[pb] >> 3;
      }
    } else { /* if ( (scaleCur >= 0) && (scalePrev < 0) ) */
      scalePrev = -scalePrev;
      for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
        partNrg[pb] = ((fMultDiv2(alpha1, slotNrg[pb]) >> scaleCur) +
                       (fMultDiv2(alpha, partNrgPrev[pb]) << scalePrev))
                      << 1;
        maxVal |= partNrg[pb];
        frameNrg += slotNrg[pb] >> 3;
      }
    }

    /* frameNrg /= (END_BB_ENV - START_BB_ENV); 0.88888888888f =
     * (1/(END_BB_ENV-START_BB_ENV)<<3; shift with 3 is compensated in loop
     * above */
    frameNrg = fMult(frameNrg, FL2FXCONST_DBL(0.88888888888f));

    /* store scalefactor and headroom for part nrg prev */
    pPartNrgPrevSF[0] = partNrgSF;
    pPartNrgPrev2SF[0] = fixMax(0, CntLeadingZeros(maxVal) - 1);

    commonScale = fixMax(frameNrgSF - SF_ALPHA1 + 1, pFrameNrgPrevSF[0] + 1);
    scalePrev = fixMin(commonScale - pFrameNrgPrevSF[0], DFRACT_BITS - 1);
    scaleCur = fixMin(commonScale - frameNrgSF + SF_ALPHA1, DFRACT_BITS - 1);
    frameNrgSF = commonScale;

    frameNrg = ((fMultDiv2(alpha1, frameNrg) >> scaleCur) +
                (fMultDiv2(alpha, pBBEnvState->frameNrgPrev__FDK[prevChOffs]) >>
                 scalePrev))
               << 1;

    clz = fixMax(0, CntLeadingZeros(frameNrg) - 1);
    pBBEnvState->frameNrgPrev__FDK[prevChOffs] = frameNrg << clz;
    pFrameNrgPrevSF[0] = frameNrgSF - clz;

    env = FL2FXCONST_DBL(0.0f);
    scale = clz + partNrgSF - frameNrgSF;
    scale_min = DFRACT_BITS - 1;
    for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
      if ((partNrg[pb] | slotNrg[pb]) != FL2FXCONST_DBL(0.0f)) {
        INT s;
        INT sc = 0;
        INT sn = fixMax(0, CntLeadingZeros(slotNrg[pb]) - 1);
        FIXP_DBL inv_sqrt = invSqrtNorm2(partNrg[pb], &sc);
        FIXP_DBL res = fMult(slotNrg[pb] << sn, fPow2(inv_sqrt));

        s = fixMax(0, CntLeadingZeros(res) - 1);
        res = res << s;

        sc = scale - (2 * sc - sn - s);
        scale_min = fixMin(scale_min, sc);

        resPb[pb] = res;
        resPbSF[pb] = sc;
      } else {
        resPb[pb] = (FIXP_DBL)0;
        resPbSF[pb] = 0;
      }
    }

    scale_min = 4 - scale_min;

    for (pb = START_BB_ENV; pb < END_BB_ENV; pb++) {
      INT sc = fixMax(fixMin(resPbSF[pb] + scale_min, DFRACT_BITS - 1),
                      -(DFRACT_BITS - 1));

      if (sc < 0) {
        env += resPb[pb] << (-sc);
      } else {
        env += resPb[pb] >> (sc);
      }
    }

    env = fMultDiv2(env, pBBEnvState->frameNrgPrev__FDK[prevChOffs]);
    envSF = slotNrgSF + scale_min + 1;

    commonScale = fixMax(envSF - SF_BETA1 + 1, pNormNrgPrevSF[0] + 1);
    scalePrev = fixMin(commonScale - pNormNrgPrevSF[0], DFRACT_BITS - 1);
    scaleCur = fixMin(commonScale - envSF + SF_BETA1, DFRACT_BITS - 1);

    normNrg = ((fMultDiv2(beta1, env) >> scaleCur) +
               (fMultDiv2(beta, pBBEnvState->normNrgPrev__FDK[prevChOffs]) >>
                scalePrev))
              << 1;

    clz = fixMax(0, CntLeadingZeros(normNrg) - 1);
    pBBEnvState->normNrgPrev__FDK[prevChOffs] = normNrg << clz;
    pNormNrgPrevSF[0] = commonScale - clz;

    if (shapeActiv) {
      if ((env | normNrg) != FL2FXCONST_DBL(0.0f)) {
        INT sc, se, sn;
        se = fixMax(0, CntLeadingZeros(env) - 1);
        sc = commonScale + SF_DIV32 - envSF + se;
        env = fMult(sqrtFixp((env << se) >> (sc & 0x1)),
                    invSqrtNorm2(normNrg, &sn));

        sc = fixMin((sc >> 1) - sn, DFRACT_BITS - 1);
        if (sc < 0) {
          env <<= (-sc);
        } else {
          env >>= (sc);
        }
      }
      /* env is scaled by SF_DIV32/2 bits */
    }
    pEnv[ch] = env;
  }

  C_ALLOC_SCRATCH_END(resPbSF, INT, (END_BB_ENV - START_BB_ENV));
  C_ALLOC_SCRATCH_END(resPb, FIXP_DBL, (END_BB_ENV - START_BB_ENV));
  C_ALLOC_SCRATCH_END(pScratchBuffer, FIXP_DBL, (2 * 42 + MAX_PARAMETER_BANDS));
}

void SpatialDecReshapeBBEnv(spatialDec *self, const SPATIAL_BS_FRAME *frame,
                            INT ts) {
  INT ch, scale;
  INT dryFacSF, slotAmpSF;
  INT slotAmp_dry_e, slotAmp_wet_e;
  FIXP_DBL tmp, dryFac, envShape;
  FIXP_DBL slotAmp_dry, slotAmp_wet, slotAmp_ratio;
  FIXP_DBL envDry[MAX_OUTPUT_CHANNELS], envDmx[2];

  INT cplxBands;
  INT hybBands = self->hybridBands - 6;

  cplxBands = self->hybridBands - 6;

  /* extract downmix envelope(s) */
  switch (self->treeConfig) {
    default:
      extractBBEnv(self, INP_DMX, 0, fMin(self->numInputChannels, 2), envDmx,
                   frame);
  }

  /* extract dry and wet envelopes */
  extractBBEnv(self, INP_DRY_WET, 0, self->numOutputChannels, envDry, frame);

  for (ch = 0; ch < self->numOutputChannels; ch++) {
    INT ch2;

    ch2 = row2channelGES[self->treeConfig][ch];

    if (ch2 == -1) continue;

    if (frame->tempShapeEnableChannelGES[ch2]) {
      INT sc;

      /* reshape dry and wet signals according to transmitted envelope */

      /* De-quantize GES data */
      FDK_ASSERT((frame->bsEnvShapeData[ch2][ts] >= 0) &&
                 (frame->bsEnvShapeData[ch2][ts] <= 4));
      FDK_ASSERT((self->envQuantMode == 0) || (self->envQuantMode == 1));
      envShape =
          FX_CFG2FX_DBL(envShapeDataTable__FDK[frame->bsEnvShapeData[ch2][ts]]
                                              [self->envQuantMode]);

      /* get downmix channel */
      ch2 = self->row2channelDmxGES[ch];

      /* multiply ratio with dmx envelope; tmp is scaled by SF_DIV32/2+SF_SHAPE
       * bits */
      if (ch2 == 2) {
        tmp = fMultDiv2(envShape, envDmx[0]) + fMultDiv2(envShape, envDmx[1]);
      } else {
        tmp = fMult(envShape, envDmx[ch2]);
      }

      /* weighting factors */
      dryFacSF = slotAmpSF = 0;
      dryFac = slotAmp_ratio = FL2FXCONST_DBL(0.0f);

      /* dryFac will be scaled by dryFacSF bits */
      if (envDry[ch] != FL2FXCONST_DBL(0.0f)) {
        envDry[ch] = invSqrtNorm2(envDry[ch], &dryFacSF);
        dryFac = fMultDiv2(tmp, fPow2Div2(envDry[ch])) << 2;
        dryFacSF = SF_SHAPE + 2 * dryFacSF;
      }

      slotAmp_dry_e = slotAmp_wet_e = 0;

      /* calculate slotAmp_dry and slotAmp_wet */
      slotAmp(&slotAmp_dry, &slotAmp_dry_e, &slotAmp_wet, &slotAmp_wet_e,
              &self->hybOutputRealDry__FDK[ch][6],
              &self->hybOutputImagDry__FDK[ch][6],
              &self->hybOutputRealWet__FDK[ch][6],
              &self->hybOutputImagWet__FDK[ch][6], cplxBands, hybBands);

      /* exponents must be even due to subsequent square root calculation */
      FDK_ASSERT(((slotAmp_dry_e & 1) == 0) && ((slotAmp_wet_e & 1) == 0));

      /* slotAmp_ratio will be scaled by slotAmpSF bits */
      if (slotAmp_dry != FL2FXCONST_DBL(0.0f)) {
        slotAmp_wet = sqrtFixp(slotAmp_wet);
        slotAmp_dry = invSqrtNorm2(slotAmp_dry, &slotAmpSF);

        slotAmp_ratio = fMult(slotAmp_wet, slotAmp_dry);
        slotAmpSF = slotAmpSF + (slotAmp_wet_e >> 1) - (slotAmp_dry_e >> 1);
      }

      /* calculate common scale factor */
      scale =
          fixMax(3, fixMax(dryFacSF, slotAmpSF)); /* scale is at least with 3
                                                     bits to avoid overflows
                                                     when calculating dryFac  */
      dryFac = dryFac >> fixMin(scale - dryFacSF, DFRACT_BITS - 1);
      slotAmp_ratio =
          slotAmp_ratio >> fixMin(scale - slotAmpSF, DFRACT_BITS - 1);

      /* limit dryFac */
      dryFac = fixMax(
          FL2FXCONST_DBL(0.25f) >> (INT)fixMin(2 * scale, DFRACT_BITS - 1),
          fMult(dryFac, slotAmp_ratio) -
              (slotAmp_ratio >> fixMin(scale, DFRACT_BITS - 1)) +
              (dryFac >> fixMin(scale, DFRACT_BITS - 1)));
      dryFac = fixMin(
          FL2FXCONST_DBL(0.50f) >> (INT)fixMin(2 * scale - 3, DFRACT_BITS - 1),
          dryFac); /* reduce shift bits by 3, because upper
                      limit 4.0 is scaled with 3 bits */
      scale = 2 * scale + 1;

      /* improve precision for dryFac */
      sc = fixMax(0, CntLeadingZeros(dryFac) - 1);
      dryFac = dryFac << (INT)fixMin(scale, sc);
      scale = scale - fixMin(scale, sc);

      /* shaping */
      shapeBBEnv(&self->hybOutputRealDry__FDK[ch][6],
                 &self->hybOutputImagDry__FDK[ch][6], dryFac,
                 fixMin(scale, DFRACT_BITS - 1), cplxBands, hybBands);
    }
  }
}
