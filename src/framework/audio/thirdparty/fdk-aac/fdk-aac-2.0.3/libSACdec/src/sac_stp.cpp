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

   Description: SAC Dec subband processing

*******************************************************************************/

#include "sac_stp.h"
#include "sac_calcM1andM2.h"
#include "sac_bitdec.h"
#include "FDK_matrixCalloc.h"
#include "sac_rom.h"

#define SF_FREQ_DOMAIN_HEADROOM (2 * (1))

#define BP_GF_START 6
#define BP_GF_SIZE 25
#define HP_SIZE 9
#define STP_UPDATE_ENERGY_RATE 32

#define SF_WET 5
#define SF_DRY \
  3 /* SF_DRY == 2 would produce good conformance test results as well */
#define SF_DRY_NRG                                                           \
  (4 - 1) /* 8.495f = sum(BP_GF__FDK[i])                                     \
             i=0,..,(sizeof(BP_GF__FDK)/sizeof(FIXP_CFG)-1) => energy        \
             calculation needs 4 bits headroom, headroom can be reduced by 1 \
             bit due to fPow2Div2() usage */
#define SF_WET_NRG                                                           \
  (4 - 1) /* 8.495f = sum(BP_GF__FDK[i])                                     \
             i=0,..,(sizeof(BP_GF__FDK)/sizeof(FIXP_CFG)-1) => energy        \
             calculation needs 4 bits headroom, headroom can be reduced by 1 \
             bit due to fPow2Div2() usage */
#define SF_PRODUCT_BP_GF 13
#define SF_PRODUCT_BP_GF_GF 26
#define SF_SCALE 2

#define SF_SCALE_LD64 FL2FXCONST_DBL(0.03125)      /* LD64((1<<SF_SCALE))*/
#define STP_LPF_COEFF1__FDK FL2FXCONST_DBL(0.950f) /* 0.95 */
#define ONE_MINUS_STP_LPF_COEFF1__FDK FL2FXCONST_DBL(0.05f) /* 1.0 - 0.95 */
#define STP_LPF_COEFF2__FDK FL2FXCONST_DBL(0.450f)          /* 0.45 */
#define ONE_MINUS_STP_LPF_COEFF2__FDK \
  FL2FXCONST_DBL(1.0f - 0.450f) /* 1.0 - 0.45 */
#define STP_SCALE_LIMIT__FDK \
  FL2FXCONST_DBL(2.82f / (float)(1 << SF_SCALE)) /* scaled by SF_SCALE */
#define ONE_DIV_STP_SCALE_LIMIT__FDK                                          \
  FL2FXCONST_DBL(1.0f / 2.82f / (float)(1 << SF_SCALE)) /* scaled by SF_SCALE \
                                                         */
#define ABS_THR__FDK       \
  FL2FXCONST_DBL(ABS_THR / \
                 ((float)(1 << (22 + 22 - 26)))) /* scaled by 18 bits */
#define ABS_THR2__FDK                      \
  FL2FXCONST_DBL(ABS_THR * 32.0f * 32.0f / \
                 ((float)(1 << (22 + 22 - 26)))) /* scaled by 10 bits */
#define STP_SCALE_LIMIT_HI \
  FL2FXCONST_DBL(3.02222222222 / (1 << SF_SCALE)) /* see 4. below */
#define STP_SCALE_LIMIT_LO \
  FL2FXCONST_DBL(0.28289992119 / (1 << SF_SCALE)) /* see 4. below */
#define STP_SCALE_LIMIT_HI_LD64                 \
  FL2FXCONST_DBL(0.04986280452) /* see 4. below \
                                 */
#define STP_SCALE_LIMIT_LO_LD64                 \
  FL2FXCONST_DBL(0.05692613500) /* see 4. below \
                                 */

/*  Scale factor calculation for the diffuse signal needs adapted thresholds
    for STP_SCALE_LIMIT and 1/STP_SCALE_LIMIT:

    1. scale = sqrt(DryNrg/WetNrg)

    2. Damping of scale factor
       scale2 = 0.1 + 0.9 * scale

    3. Limiting of scale factor
          STP_SCALE_LIMIT           >=        scale2        >= 1/STP_SCALE_LIMIT
       => STP_SCALE_LIMIT           >=  (0.1 + 0.9 * scale) >= 1/STP_SCALE_LIMIT
       => (STP_SCALE_LIMIT-0.1)/0.9 >=        scale         >=
   (1/STP_SCALE_LIMIT-0.1)/0.9

    3. Limiting of scale factor before sqrt calculation
       ((STP_SCALE_LIMIT-0.1)/0.9)^2 >= (scale^2) >=
   ((1/STP_SCALE_LIMIT-0.1)/0.9)^2 (STP_SCALE_LIMIT_HI)^2        >= (scale^2) >=
   (STP_SCALE_LIMIT_LO)^2

    4. Thresholds for limiting of scale factor
       STP_SCALE_LIMIT_HI      = ((2.82-0.1)/0.9)
       STP_SCALE_LIMIT_LO      = (((1.0/2.82)-0.1)/0.9)
       STP_SCALE_LIMIT_HI_LD64 = LD64(STP_SCALE_LIMIT_HI*STP_SCALE_LIMIT_HI)
       STP_SCALE_LIMIT_LO_LD64 = LD64(STP_SCALE_LIMIT_LO*STP_SCALE_LIMIT_LO)
*/

#define CALC_WET_SCALE(dryIdx, wetIdx)                                         \
  if ((DryEnerLD64[dryIdx] - STP_SCALE_LIMIT_HI_LD64) > WetEnerLD64[wetIdx]) { \
    scale[wetIdx] = STP_SCALE_LIMIT_HI;                                        \
  } else if (DryEnerLD64[dryIdx] <                                             \
             (WetEnerLD64[wetIdx] - STP_SCALE_LIMIT_LO_LD64)) {                \
    scale[wetIdx] = STP_SCALE_LIMIT_LO;                                        \
  } else {                                                                     \
    tmp = ((DryEnerLD64[dryIdx] - WetEnerLD64[wetIdx]) >> 1) - SF_SCALE_LD64;  \
    scale[wetIdx] = CalcInvLdData(tmp);                                        \
  }

struct STP_DEC {
  FIXP_DBL runDryEner[MAX_INPUT_CHANNELS];
  FIXP_DBL runWetEner[MAX_OUTPUT_CHANNELS];
  FIXP_DBL oldDryEnerLD64[MAX_INPUT_CHANNELS];
  FIXP_DBL oldWetEnerLD64[MAX_OUTPUT_CHANNELS];
  FIXP_DBL prev_tp_scale[MAX_OUTPUT_CHANNELS];
  const FIXP_CFG *BP;
  const FIXP_CFG *BP_GF;
  int update_old_ener;
};

inline void combineSignalCplx(FIXP_DBL *hybOutputRealDry,
                              FIXP_DBL *hybOutputImagDry,
                              FIXP_DBL *hybOutputRealWet,
                              FIXP_DBL *hybOutputImagWet, int bands) {
  int n;

  for (n = bands - 1; n >= 0; n--) {
    *hybOutputRealDry = fAddSaturate(*hybOutputRealDry, *hybOutputRealWet);
    *hybOutputImagDry = fAddSaturate(*hybOutputImagDry, *hybOutputImagWet);
    hybOutputRealDry++, hybOutputRealWet++;
    hybOutputImagDry++, hybOutputImagWet++;
  }
}

inline void combineSignalCplxScale1(FIXP_DBL *hybOutputRealDry,
                                    FIXP_DBL *hybOutputImagDry,
                                    FIXP_DBL *hybOutputRealWet,
                                    FIXP_DBL *hybOutputImagWet,
                                    const FIXP_CFG *pBP, FIXP_DBL scaleX,
                                    int bands) {
  int n;
  FIXP_DBL scaleY;
  for (n = bands - 1; n >= 0; n--) {
    scaleY = fMult(scaleX, *pBP);
    *hybOutputRealDry = SATURATE_LEFT_SHIFT(
        (*hybOutputRealDry >> SF_SCALE) + fMult(*hybOutputRealWet, scaleY),
        SF_SCALE, DFRACT_BITS);
    *hybOutputImagDry = SATURATE_LEFT_SHIFT(
        (*hybOutputImagDry >> SF_SCALE) + fMult(*hybOutputImagWet, scaleY),
        SF_SCALE, DFRACT_BITS);
    hybOutputRealDry++, hybOutputRealWet++;
    hybOutputImagDry++, hybOutputImagWet++;
    pBP++;
  }
}

inline void combineSignalCplxScale2(FIXP_DBL *hybOutputRealDry,
                                    FIXP_DBL *hybOutputImagDry,
                                    FIXP_DBL *hybOutputRealWet,
                                    FIXP_DBL *hybOutputImagWet, FIXP_DBL scaleX,
                                    int bands) {
  int n;

  for (n = bands - 1; n >= 0; n--) {
    *hybOutputRealDry = SATURATE_LEFT_SHIFT(
        (*hybOutputRealDry >> SF_SCALE) + fMult(*hybOutputRealWet, scaleX),
        SF_SCALE, DFRACT_BITS);
    *hybOutputImagDry = SATURATE_LEFT_SHIFT(
        (*hybOutputImagDry >> SF_SCALE) + fMult(*hybOutputImagWet, scaleX),
        SF_SCALE, DFRACT_BITS);
    hybOutputRealDry++, hybOutputRealWet++;
    hybOutputImagDry++, hybOutputImagWet++;
  }
}

/*******************************************************************************
 Functionname: subbandTPCreate
 ******************************************************************************/
SACDEC_ERROR subbandTPCreate(HANDLE_STP_DEC *hStpDec) {
  HANDLE_STP_DEC self = NULL;
  FDK_ALLOCATE_MEMORY_1D(self, 1, struct STP_DEC)
  if (hStpDec != NULL) {
    *hStpDec = self;
  }

  return MPS_OK;
bail:
  return MPS_OUTOFMEMORY;
}

SACDEC_ERROR subbandTPInit(HANDLE_STP_DEC self) {
  SACDEC_ERROR err = MPS_OK;
  int ch;

  for (ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
    self->prev_tp_scale[ch] = FL2FXCONST_DBL(1.0f / (1 << SF_SCALE));
    self->oldWetEnerLD64[ch] = FL2FXCONST_DBL(0.0);
  }
  for (ch = 0; ch < MAX_INPUT_CHANNELS; ch++) {
    self->oldDryEnerLD64[ch] = FL2FXCONST_DBL(0.0);
  }

  self->BP = BP__FDK;
  self->BP_GF = BP_GF__FDK;

  self->update_old_ener = 0;

  return err;
}

/*******************************************************************************
 Functionname: subbandTPDestroy
 ******************************************************************************/
void subbandTPDestroy(HANDLE_STP_DEC *hStpDec) {
  if (hStpDec != NULL) {
    FDK_FREE_MEMORY_1D(*hStpDec);
  }
}

/*******************************************************************************
 Functionname: subbandTPApply
 ******************************************************************************/
SACDEC_ERROR subbandTPApply(spatialDec *self, const SPATIAL_BS_FRAME *frame) {
  FIXP_DBL *qmfOutputRealDry[MAX_OUTPUT_CHANNELS];
  FIXP_DBL *qmfOutputImagDry[MAX_OUTPUT_CHANNELS];
  FIXP_DBL *qmfOutputRealWet[MAX_OUTPUT_CHANNELS];
  FIXP_DBL *qmfOutputImagWet[MAX_OUTPUT_CHANNELS];

  FIXP_DBL DryEner[MAX_INPUT_CHANNELS];
  FIXP_DBL scale[MAX_OUTPUT_CHANNELS];

  FIXP_DBL DryEnerLD64[MAX_INPUT_CHANNELS];
  FIXP_DBL WetEnerLD64[MAX_OUTPUT_CHANNELS];

  FIXP_DBL DryEner0 = FL2FXCONST_DBL(0.0f);
  FIXP_DBL WetEnerX, damp, tmp;
  FIXP_DBL dmxReal0, dmxImag0;
  int skipChannels[MAX_OUTPUT_CHANNELS];
  int n, ch, cplxBands, cplxHybBands;
  int dry_scale_dmx, wet_scale_dmx;
  int i_LF, i_RF;
  HANDLE_STP_DEC hStpDec;
  const FIXP_CFG *pBP;

  int nrgScale = (2 * self->clipProtectGainSF__FDK);

  hStpDec = self->hStpDec;

  /* set scalefactor and loop counter */
  FDK_ASSERT(SF_DRY >= 1);
  {
    cplxBands = BP_GF_SIZE;
    cplxHybBands = self->hybridBands;
    if (self->treeConfig == TREE_212) {
      dry_scale_dmx = 2; /* 2 bits to compensate fMultDiv2() and fPow2Div2()
                            used in energy calculation */
    } else {
      dry_scale_dmx = (2 * SF_DRY) - 2;
    }
    wet_scale_dmx = 2;
  }

  /* setup pointer for forming the direct downmix signal */
  for (ch = 0; ch < self->numOutputChannels; ch++) {
    qmfOutputRealDry[ch] = &self->hybOutputRealDry__FDK[ch][7];
    qmfOutputRealWet[ch] = &self->hybOutputRealWet__FDK[ch][7];
    qmfOutputImagDry[ch] = &self->hybOutputImagDry__FDK[ch][7];
    qmfOutputImagWet[ch] = &self->hybOutputImagWet__FDK[ch][7];
  }

  /* clear skipping flag for all output channels */
  FDKmemset(skipChannels, 0, self->numOutputChannels * sizeof(int));

  /* set scale values to zero */
  FDKmemset(scale, 0, self->numOutputChannels * sizeof(FIXP_DBL));

  /* update normalisation energy with latest smoothed energy */
  if (hStpDec->update_old_ener == STP_UPDATE_ENERGY_RATE) {
    hStpDec->update_old_ener = 1;
    for (ch = 0; ch < self->numInputChannels; ch++) {
      hStpDec->oldDryEnerLD64[ch] =
          CalcLdData(fAddSaturate(hStpDec->runDryEner[ch], ABS_THR__FDK));
    }
    for (ch = 0; ch < self->numOutputChannels; ch++) {
      if (self->treeConfig == TREE_212)
        hStpDec->oldWetEnerLD64[ch] =
            CalcLdData(fAddSaturate(hStpDec->runWetEner[ch], ABS_THR__FDK));
      else
        hStpDec->oldWetEnerLD64[ch] =
            CalcLdData(fAddSaturate(hStpDec->runWetEner[ch], ABS_THR2__FDK));
    }
  } else {
    hStpDec->update_old_ener++;
  }

  /* get channel configuration */
  switch (self->treeConfig) {
    case TREE_212:
      i_LF = 0;
      i_RF = 1;
      break;
    default:
      return MPS_WRONG_TREECONFIG;
  }

  /* form the 'direct' downmix signal */
  pBP = hStpDec->BP_GF - BP_GF_START;
  switch (self->treeConfig) {
    case TREE_212:
      INT sMin, sNorm, sReal, sImag;

      sReal = fMin(getScalefactor(&qmfOutputRealDry[i_LF][BP_GF_START],
                                  cplxBands - BP_GF_START),
                   getScalefactor(&qmfOutputRealDry[i_RF][BP_GF_START],
                                  cplxBands - BP_GF_START));
      sImag = fMin(getScalefactor(&qmfOutputImagDry[i_LF][BP_GF_START],
                                  cplxBands - BP_GF_START),
                   getScalefactor(&qmfOutputImagDry[i_RF][BP_GF_START],
                                  cplxBands - BP_GF_START));
      sMin = fMin(sReal, sImag) - 1;

      for (n = BP_GF_START; n < cplxBands; n++) {
        dmxReal0 = scaleValue(qmfOutputRealDry[i_LF][n], sMin) +
                   scaleValue(qmfOutputRealDry[i_RF][n], sMin);
        dmxImag0 = scaleValue(qmfOutputImagDry[i_LF][n], sMin) +
                   scaleValue(qmfOutputImagDry[i_RF][n], sMin);

        DryEner0 += (fMultDiv2(fPow2Div2(dmxReal0), pBP[n]) +
                     fMultDiv2(fPow2Div2(dmxImag0), pBP[n])) >>
                    SF_DRY_NRG;
      }

      sNorm = SF_FREQ_DOMAIN_HEADROOM + SF_DRY_NRG + dry_scale_dmx -
              (2 * sMin) + nrgScale;
      DryEner0 = scaleValueSaturate(
          DryEner0, fMax(fMin(sNorm, DFRACT_BITS - 1), -(DFRACT_BITS - 1)));
      break;
    default:;
  }
  DryEner[0] = DryEner0;

  /* normalise the 'direct' signals */
  for (ch = 0; ch < self->numInputChannels; ch++) {
    if (self->treeConfig != TREE_212) DryEner[ch] = DryEner[ch] << nrgScale;
    hStpDec->runDryEner[ch] =
        fMult(STP_LPF_COEFF1__FDK, hStpDec->runDryEner[ch]) +
        fMult(ONE_MINUS_STP_LPF_COEFF1__FDK, DryEner[ch]);
    if (DryEner[ch] != FL2FXCONST_DBL(0.0f)) {
      DryEnerLD64[ch] =
          fixMax((CalcLdData(DryEner[ch]) - hStpDec->oldDryEnerLD64[ch]),
                 FL2FXCONST_DBL(-0.484375f));
    } else {
      DryEnerLD64[ch] = FL2FXCONST_DBL(-0.484375f);
    }
  }
  for (; ch < MAX_INPUT_CHANNELS; ch++) {
    DryEnerLD64[ch] = FL2FXCONST_DBL(-0.484375f);
  }

  /* normalise the 'diffuse' signals */
  pBP = hStpDec->BP_GF - BP_GF_START;
  for (ch = 0; ch < self->numOutputChannels; ch++) {
    if (skipChannels[ch]) {
      continue;
    }

    WetEnerX = FL2FXCONST_DBL(0.0f);

    if (self->treeConfig == TREE_212) {
      INT sMin, sNorm;

      sMin = fMin(getScalefactor(&qmfOutputRealWet[ch][BP_GF_START],
                                 cplxBands - BP_GF_START),
                  getScalefactor(&qmfOutputImagWet[ch][BP_GF_START],
                                 cplxBands - BP_GF_START));

      for (n = BP_GF_START; n < cplxBands; n++) {
        WetEnerX +=
            (fMultDiv2(fPow2Div2(scaleValue(qmfOutputRealWet[ch][n], sMin)),
                       pBP[n]) +
             fMultDiv2(fPow2Div2(scaleValue(qmfOutputImagWet[ch][n], sMin)),
                       pBP[n])) >>
            SF_WET_NRG;
      }
      sNorm = SF_FREQ_DOMAIN_HEADROOM + SF_WET_NRG + wet_scale_dmx -
              (2 * sMin) + nrgScale;
      WetEnerX = scaleValueSaturate(
          WetEnerX, fMax(fMin(sNorm, DFRACT_BITS - 1), -(DFRACT_BITS - 1)));
    } else
      FDK_ASSERT(self->treeConfig == TREE_212);

    hStpDec->runWetEner[ch] =
        fMult(STP_LPF_COEFF1__FDK, hStpDec->runWetEner[ch]) +
        fMult(ONE_MINUS_STP_LPF_COEFF1__FDK, WetEnerX);

    if (WetEnerX == FL2FXCONST_DBL(0.0f)) {
      WetEnerLD64[ch] = FL2FXCONST_DBL(-0.484375f);
    } else {
      WetEnerLD64[ch] =
          fixMax((CalcLdData(WetEnerX) - hStpDec->oldWetEnerLD64[ch]),
                 FL2FXCONST_DBL(-0.484375f));
    }
  }

  /* compute scale factor for the 'diffuse' signals */
  switch (self->treeConfig) {
    case TREE_212:
      if (DryEner[0] != FL2FXCONST_DBL(0.0f)) {
        CALC_WET_SCALE(0, i_LF);
        CALC_WET_SCALE(0, i_RF);
      }
      break;
    default:;
  }

  damp = FL2FXCONST_DBL(0.1f / (1 << SF_SCALE));
  for (ch = 0; ch < self->numOutputChannels; ch++) {
    /* damp the scaling factor */
    scale[ch] = damp + fMult(FL2FXCONST_DBL(0.9f), scale[ch]);

    /* limiting the scale factor */
    if (scale[ch] > STP_SCALE_LIMIT__FDK) {
      scale[ch] = STP_SCALE_LIMIT__FDK;
    }
    if (scale[ch] < ONE_DIV_STP_SCALE_LIMIT__FDK) {
      scale[ch] = ONE_DIV_STP_SCALE_LIMIT__FDK;
    }

    /* low pass filter the scaling factor */
    scale[ch] =
        fMult(STP_LPF_COEFF2__FDK, scale[ch]) +
        fMult(ONE_MINUS_STP_LPF_COEFF2__FDK, hStpDec->prev_tp_scale[ch]);
    hStpDec->prev_tp_scale[ch] = scale[ch];
  }

  /* combine 'direct' and scaled 'diffuse' signal */
  FDK_ASSERT((HP_SIZE - 3 + 10 - 1) == PC_NUM_HYB_BANDS);
  const SCHAR *channlIndex = row2channelSTP[self->treeConfig];

  for (ch = 0; ch < self->numOutputChannels; ch++) {
    int no_scaling;

    no_scaling = !frame->tempShapeEnableChannelSTP[channlIndex[ch]];
    if (no_scaling) {
      combineSignalCplx(
          &self->hybOutputRealDry__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputImagDry__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputRealWet__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputImagWet__FDK[ch][self->tp_hybBandBorder],
          cplxHybBands - self->tp_hybBandBorder);

    } else {
      FIXP_DBL scaleX;
      scaleX = scale[ch];
      pBP = hStpDec->BP - self->tp_hybBandBorder;
      /* Band[HP_SIZE-3+10-1] needs not to be processed in
         combineSignalCplxScale1(), because pB[HP_SIZE-3+10-1] would be 1.0 */
      combineSignalCplxScale1(
          &self->hybOutputRealDry__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputImagDry__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputRealWet__FDK[ch][self->tp_hybBandBorder],
          &self->hybOutputImagWet__FDK[ch][self->tp_hybBandBorder],
          &pBP[self->tp_hybBandBorder], scaleX,
          (HP_SIZE - 3 + 10 - 1) - self->tp_hybBandBorder);

      {
        combineSignalCplxScale2(
            &self->hybOutputRealDry__FDK[ch][HP_SIZE - 3 + 10 - 1],
            &self->hybOutputImagDry__FDK[ch][HP_SIZE - 3 + 10 - 1],
            &self->hybOutputRealWet__FDK[ch][HP_SIZE - 3 + 10 - 1],
            &self->hybOutputImagWet__FDK[ch][HP_SIZE - 3 + 10 - 1], scaleX,
            cplxHybBands - (HP_SIZE - 3 + 10 - 1));
      }
    }
  }

  return (SACDEC_ERROR)MPS_OK;
  ;
}
