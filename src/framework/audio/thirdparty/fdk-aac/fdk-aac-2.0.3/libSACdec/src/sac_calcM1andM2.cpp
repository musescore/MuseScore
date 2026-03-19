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

/*********************** MPEG surround decoder library *************************

   Author(s):

   Description: SAC Dec M1 and M2 calculation

*******************************************************************************/

#include "sac_calcM1andM2.h"
#include "sac_bitdec.h"
#include "sac_process.h"
#include "sac_rom.h"
#include "sac_smoothing.h"
#include "FDK_trigFcts.h"

/* assorted definitions and constants */

#define ABS_THR2 1.0e-9
#define SQRT2_FDK \
  ((FIXP_DBL)FL2FXCONST_DBL(0.70710678118f)) /* FDKsqrt(2.0) scaled by 0.5 */

static void param2UMX_PS__FDK(spatialDec* self,
                              FIXP_DBL H11[MAX_PARAMETER_BANDS],
                              FIXP_DBL H12[MAX_PARAMETER_BANDS],
                              FIXP_DBL H21[MAX_PARAMETER_BANDS],
                              FIXP_DBL H22[MAX_PARAMETER_BANDS],
                              FIXP_DBL c_l[MAX_PARAMETER_BANDS],
                              FIXP_DBL c_r[MAX_PARAMETER_BANDS], int ottBoxIndx,
                              int parameterSetIndx, int resBands);

static void param2UMX_PS_Core__FDK(
    const SCHAR cld[MAX_PARAMETER_BANDS], const SCHAR icc[MAX_PARAMETER_BANDS],
    const int numOttBands, const int resBands,
    FIXP_DBL H11[MAX_PARAMETER_BANDS], FIXP_DBL H12[MAX_PARAMETER_BANDS],
    FIXP_DBL H21[MAX_PARAMETER_BANDS], FIXP_DBL H22[MAX_PARAMETER_BANDS],
    FIXP_DBL c_l[MAX_PARAMETER_BANDS], FIXP_DBL c_r[MAX_PARAMETER_BANDS]);

static void param2UMX_PS_IPD_OPD__FDK(
    spatialDec* self, const SPATIAL_BS_FRAME* frame,
    FIXP_DBL H11re[MAX_PARAMETER_BANDS], FIXP_DBL H12re[MAX_PARAMETER_BANDS],
    FIXP_DBL H21re[MAX_PARAMETER_BANDS], FIXP_DBL H22re[MAX_PARAMETER_BANDS],
    FIXP_DBL c_l[MAX_PARAMETER_BANDS], FIXP_DBL c_r[MAX_PARAMETER_BANDS],
    int ottBoxIndx, int parameterSetIndx, int residualBands);

static void param2UMX_Prediction__FDK(
    spatialDec* self, FIXP_DBL H11re[MAX_PARAMETER_BANDS],
    FIXP_DBL H11im[MAX_PARAMETER_BANDS], FIXP_DBL H12re[MAX_PARAMETER_BANDS],
    FIXP_DBL H12im[MAX_PARAMETER_BANDS], FIXP_DBL H21re[MAX_PARAMETER_BANDS],
    FIXP_DBL H21im[MAX_PARAMETER_BANDS], FIXP_DBL H22re[MAX_PARAMETER_BANDS],
    FIXP_DBL H22im[MAX_PARAMETER_BANDS], int ottBoxIndx, int parameterSetIndx,
    int resBands);

/* static void SpatialDecCalculateM0(spatialDec* self,int ps); */
static SACDEC_ERROR SpatialDecCalculateM1andM2_212(
    spatialDec* self, int ps, const SPATIAL_BS_FRAME* frame);

/*******************************************************************************
 Functionname: SpatialDecGetResidualIndex
 *******************************************************************************

 Description:

 Arguments:

 Input:

 Output:

*******************************************************************************/
int SpatialDecGetResidualIndex(spatialDec* self, int row) {
  return row2residual[self->treeConfig][row];
}

/*******************************************************************************
 Functionname: UpdateAlpha
 *******************************************************************************

 Description:

 Arguments:

 Input:

 Output:

*******************************************************************************/
static void updateAlpha(spatialDec* self) {
  int nChIn = self->numInputChannels;
  int ch;

  for (ch = 0; ch < nChIn; ch++) {
    FIXP_DBL alpha = /* FL2FXCONST_DBL(1.0f) */ (FIXP_DBL)MAXVAL_DBL;

    self->arbdmxAlphaPrev__FDK[ch] = self->arbdmxAlpha__FDK[ch];

    self->arbdmxAlpha__FDK[ch] = alpha;
  }
}

/*******************************************************************************
 Functionname: SpatialDecCalculateM1andM2
 *******************************************************************************
 Description:
 Arguments:
*******************************************************************************/
SACDEC_ERROR SpatialDecCalculateM1andM2(spatialDec* self, int ps,
                                        const SPATIAL_BS_FRAME* frame) {
  SACDEC_ERROR err = MPS_OK;

  if ((self->arbitraryDownmix != 0) && (ps == 0)) {
    updateAlpha(self);
  }

  self->pActivM2ParamBands = NULL;

  switch (self->upmixType) {
    case UPMIXTYPE_BYPASS:
    case UPMIXTYPE_NORMAL:
      switch (self->treeConfig) {
        case TREE_212:
          err = SpatialDecCalculateM1andM2_212(self, ps, frame);
          break;
        default:
          err = MPS_WRONG_TREECONFIG;
      };
      break;

    default:
      err = MPS_WRONG_TREECONFIG;
  }

  if (err != MPS_OK) {
    goto bail;
  }

bail:
  return err;
}

/*******************************************************************************
 Functionname: SpatialDecCalculateM1andM2_212
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
static SACDEC_ERROR SpatialDecCalculateM1andM2_212(
    spatialDec* self, int ps, const SPATIAL_BS_FRAME* frame) {
  SACDEC_ERROR err = MPS_OK;
  int pb;

  FIXP_DBL H11re[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};
  FIXP_DBL H12re[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};
  FIXP_DBL H21re[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};
  FIXP_DBL H22re[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};
  FIXP_DBL H11im[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};
  FIXP_DBL H21im[MAX_PARAMETER_BANDS] = {FL2FXCONST_DBL(0.0f)};

  INT phaseCoding = self->phaseCoding;

  switch (phaseCoding) {
    case 1:
      /* phase coding: yes; residuals: no */
      param2UMX_PS_IPD_OPD__FDK(self, frame, H11re, H12re, H21re, H22re, NULL,
                                NULL, 0, ps, self->residualBands[0]);
      break;
    case 3:
      /* phase coding: yes; residuals: yes */
      param2UMX_Prediction__FDK(self, H11re, H11im, H12re, NULL, H21re, H21im,
                                H22re, NULL, 0, ps, self->residualBands[0]);
      break;
    default:
      if (self->residualCoding) {
        /* phase coding: no; residuals: yes */
        param2UMX_Prediction__FDK(self, H11re, NULL, H12re, NULL, H21re, NULL,
                                  H22re, NULL, 0, ps, self->residualBands[0]);
      } else {
        /* phase coding: no; residuals: no */
        param2UMX_PS__FDK(self, H11re, H12re, H21re, H22re, NULL, NULL, 0, ps,
                          0);
      }
      break;
  }

  for (pb = 0; pb < self->numParameterBands; pb++) {
    self->M2Real__FDK[0][0][pb] = (H11re[pb]);
    self->M2Real__FDK[0][1][pb] = (H12re[pb]);

    self->M2Real__FDK[1][0][pb] = (H21re[pb]);
    self->M2Real__FDK[1][1][pb] = (H22re[pb]);
  }
  if (phaseCoding == 3) {
    for (pb = 0; pb < self->numParameterBands; pb++) {
      self->M2Imag__FDK[0][0][pb] = (H11im[pb]);
      self->M2Imag__FDK[1][0][pb] = (H21im[pb]);
      self->M2Imag__FDK[0][1][pb] = (FIXP_DBL)0;  // H12im[pb];
      self->M2Imag__FDK[1][1][pb] = (FIXP_DBL)0;  // H22im[pb];
    }
  }

  if (self->phaseCoding == 1) {
    SpatialDecSmoothOPD(
        self, frame,
        ps); /* INPUT: PhaseLeft, PhaseRight, (opdLeftState, opdRightState) */
  }

  return err;
}

/*******************************************************************************
 Functionname: param2UMX_PS_Core
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
static void param2UMX_PS_Core__FDK(
    const SCHAR cld[MAX_PARAMETER_BANDS], const SCHAR icc[MAX_PARAMETER_BANDS],
    const int numOttBands, const int resBands,
    FIXP_DBL H11[MAX_PARAMETER_BANDS], FIXP_DBL H12[MAX_PARAMETER_BANDS],
    FIXP_DBL H21[MAX_PARAMETER_BANDS], FIXP_DBL H22[MAX_PARAMETER_BANDS],
    FIXP_DBL c_l[MAX_PARAMETER_BANDS], FIXP_DBL c_r[MAX_PARAMETER_BANDS]) {
  int band;

  if ((c_l != NULL) && (c_r != NULL)) {
    for (band = 0; band < numOttBands; band++) {
      SpatialDequantGetCLDValues(cld[band], &c_l[band], &c_r[band]);
    }
  }

  band = 0;
  FDK_ASSERT(resBands == 0);
  for (; band < numOttBands; band++) {
    /* compute mixing variables: */
    const int idx1 = cld[band];
    const int idx2 = icc[band];
    H11[band] = FX_CFG2FX_DBL(H11_nc[idx1][idx2]);
    H21[band] = FX_CFG2FX_DBL(H11_nc[30 - idx1][idx2]);
    H12[band] = FX_CFG2FX_DBL(H12_nc[idx1][idx2]);
    H22[band] = FX_CFG2FX_DBL(-H12_nc[30 - idx1][idx2]);
  }
}

/*******************************************************************************
 Functionname: param2UMX_PS
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
static void param2UMX_PS__FDK(spatialDec* self,
                              FIXP_DBL H11[MAX_PARAMETER_BANDS],
                              FIXP_DBL H12[MAX_PARAMETER_BANDS],
                              FIXP_DBL H21[MAX_PARAMETER_BANDS],
                              FIXP_DBL H22[MAX_PARAMETER_BANDS],
                              FIXP_DBL c_l[MAX_PARAMETER_BANDS],
                              FIXP_DBL c_r[MAX_PARAMETER_BANDS], int ottBoxIndx,
                              int parameterSetIndx, int residualBands) {
  int band;
  param2UMX_PS_Core__FDK(self->ottCLD__FDK[ottBoxIndx][parameterSetIndx],
                         self->ottICC__FDK[ottBoxIndx][parameterSetIndx],
                         self->numOttBands[ottBoxIndx], residualBands, H11, H12,
                         H21, H22, c_l, c_r);

  for (band = self->numOttBands[ottBoxIndx]; band < self->numParameterBands;
       band++) {
    H11[band] = H21[band] = H12[band] = H22[band] = FL2FXCONST_DBL(0.f);
  }
}

#define N_CLD (31)
#define N_IPD (16)

static const FIXP_DBL sinIpd_tab[N_IPD] = {
    FIXP_DBL(0x00000000), FIXP_DBL(0x30fbc54e), FIXP_DBL(0x5a827999),
    FIXP_DBL(0x7641af3d), FIXP_DBL(0x7fffffff), FIXP_DBL(0x7641af3d),
    FIXP_DBL(0x5a82799a), FIXP_DBL(0x30fbc54d), FIXP_DBL(0xffffffff),
    FIXP_DBL(0xcf043ab3), FIXP_DBL(0xa57d8666), FIXP_DBL(0x89be50c3),
    FIXP_DBL(0x80000000), FIXP_DBL(0x89be50c3), FIXP_DBL(0xa57d8666),
    FIXP_DBL(0xcf043ab2),
};

/* cosIpd[i] = sinIpd[(i+4)&15] */
#define SIN_IPD(a) (sinIpd_tab[(a)])
#define COS_IPD(a) (sinIpd_tab[((a) + 4) & 15])  //(cosIpd_tab[(a)])

static const FIXP_SGL sqrt_one_minus_ICC2[8] = {
    FL2FXCONST_SGL(0.0f),
    FL2FXCONST_SGL(0.349329357483736f),
    FL2FXCONST_SGL(0.540755219669676f),
    FL2FXCONST_SGL(0.799309172723546f),
    FL2FXCONST_SGL(0.929968187843004f),
    FX_DBL2FXCONST_SGL(MAXVAL_DBL),
    FL2FXCONST_SGL(0.80813303360276f),
    FL2FXCONST_SGL(0.141067359796659f),
};

/* exponent of sqrt(CLD) */
static const SCHAR sqrt_CLD_e[N_CLD] = {
    -24, -7, -6, -5, -4, -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 1,
    1,   1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  7, 8, 25};

static const FIXP_DBL sqrt_CLD_m[N_CLD] = {
    FL2FXCONST_DBL(0.530542153566195f),
    FL2FXCONST_DBL(0.719796896243647f),
    FL2FXCONST_DBL(0.64f),
    FL2FXCONST_DBL(0.569049411212455f),
    FL2FXCONST_DBL(0.505964425626941f),
    FL2FXCONST_DBL(0.899746120304559f),
    FL2FXCONST_DBL(0.635462587779425f),
    FL2FXCONST_DBL(0.897614763441571f),
    FL2FXCONST_DBL(0.633957276984445f),
    FL2FXCONST_DBL(0.895488455427336f),
    FL2FXCONST_DBL(0.632455532033676f),
    FL2FXCONST_DBL(0.796214341106995f),
    FL2FXCONST_DBL(0.501187233627272f),
    FL2FXCONST_DBL(0.630957344480193f),
    FL2FXCONST_DBL(0.794328234724281f),
    FL2FXCONST_DBL(0.5f),
    FL2FXCONST_DBL(0.629462705897084f),
    FL2FXCONST_DBL(0.792446596230557f),
    FL2FXCONST_DBL(0.99763115748444f),
    FL2FXCONST_DBL(0.627971607877395f),
    FL2FXCONST_DBL(0.790569415042095f),
    FL2FXCONST_DBL(0.558354490188704f),
    FL2FXCONST_DBL(0.788696680600242f),
    FL2FXCONST_DBL(0.557031836333591f),
    FL2FXCONST_DBL(0.786828382371355f),
    FL2FXCONST_DBL(0.555712315637163f),
    FL2FXCONST_DBL(0.988211768802619f),
    FL2FXCONST_DBL(0.87865832060992f),
    FL2FXCONST_DBL(0.78125f),
    FL2FXCONST_DBL(0.694640394546454f),
    FL2FXCONST_DBL(0.942432183077448f),
};

static const FIXP_DBL CLD_m[N_CLD] = {
    FL2FXCONST_DBL(0.281474976710656f),
    FL2FXCONST_DBL(0.518107571841987f),
    FL2FXCONST_DBL(0.4096f),
    FL2FXCONST_DBL(0.323817232401242f),
    FL2FXCONST_DBL(0.256f),
    FL2FXCONST_DBL(0.809543081003105f),
    FL2FXCONST_DBL(0.403812700467324f),
    FL2FXCONST_DBL(0.805712263548267f),
    FL2FXCONST_DBL(0.401901829041533f),
    FL2FXCONST_DBL(0.801899573803636f),
    FL2FXCONST_DBL(0.4f),
    FL2FXCONST_DBL(0.633957276984445f),
    FL2FXCONST_DBL(0.251188643150958f),
    FL2FXCONST_DBL(0.398107170553497f),
    FL2FXCONST_DBL(0.630957344480193f),
    FL2FXCONST_DBL(0.25f),
    FL2FXCONST_DBL(0.396223298115278f),
    FL2FXCONST_DBL(0.627971607877395f),
    FL2FXCONST_DBL(0.995267926383743f),
    FL2FXCONST_DBL(0.394348340300121f),
    FL2FXCONST_DBL(0.625f),
    FL2FXCONST_DBL(0.311759736713887f),
    FL2FXCONST_DBL(0.62204245398984f),
    FL2FXCONST_DBL(0.310284466689172f),
    FL2FXCONST_DBL(0.619098903305123f),
    FL2FXCONST_DBL(0.308816177750818f),
    FL2FXCONST_DBL(0.9765625f),
    FL2FXCONST_DBL(0.772040444377046f),
    FL2FXCONST_DBL(0.6103515625f),
    FL2FXCONST_DBL(0.482525277735654f),
    FL2FXCONST_DBL(0.888178419700125),
};

static void calculateOpd(spatialDec* self, INT ottBoxIndx, INT parameterSetIndx,
                         FIXP_DBL opd[MAX_PARAMETER_BANDS]) {
  INT band;

  for (band = 0; band < self->numOttBandsIPD; band++) {
    INT idxCld = self->ottCLD__FDK[ottBoxIndx][parameterSetIndx][band];
    INT idxIpd = self->ottIPD__FDK[ottBoxIndx][parameterSetIndx][band];
    INT idxIcc = self->ottICC__FDK[ottBoxIndx][parameterSetIndx][band];
    FIXP_DBL cld, ipd;

    ipd = FX_CFG2FX_DBL(dequantIPD__FDK[idxIpd]);

    SpatialDequantGetCLD2Values(idxCld, &cld);

    /* ipd(idxIpd==8) == PI */
    if (((cld == FL2FXCONST_DBL(0.0f)) && (idxIpd == 8)) || (idxIpd == 0)) {
      opd[2 * band] = FL2FXCONST_DBL(0.0f);
    } else {
      FDK_ASSERT(idxIpd > 0);
      opd[2 * band] =
          dequantIPD_CLD_ICC_splitAngle__FDK[idxIpd - 1][idxCld][idxIcc];
    }
    opd[2 * band + 1] = opd[2 * band] - ipd;
  }
}

/* wrap phase in rad to the range of 0 <= x < 2*pi */
static FIXP_DBL wrapPhase(FIXP_DBL phase) {
  while (phase < (FIXP_DBL)0) phase += PIx2__IPD;
  while (phase >= PIx2__IPD) phase -= PIx2__IPD;
  FDK_ASSERT((phase >= (FIXP_DBL)0) && (phase < PIx2__IPD));

  return phase;
}

/*******************************************************************************
 Functionname: param2UMX_PS_IPD
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
static void param2UMX_PS_IPD_OPD__FDK(
    spatialDec* self, const SPATIAL_BS_FRAME* frame,
    FIXP_DBL H11[MAX_PARAMETER_BANDS], FIXP_DBL H12[MAX_PARAMETER_BANDS],
    FIXP_DBL H21[MAX_PARAMETER_BANDS], FIXP_DBL H22[MAX_PARAMETER_BANDS],
    FIXP_DBL c_l[MAX_PARAMETER_BANDS], FIXP_DBL c_r[MAX_PARAMETER_BANDS],
    int ottBoxIndx, int parameterSetIndx, int residualBands) {
  INT band;
  FIXP_DBL opd[2 * MAX_PARAMETER_BANDS];
  INT numOttBands = self->numOttBands[ottBoxIndx];
  INT numIpdBands;

  numIpdBands = frame->phaseMode ? self->numOttBandsIPD : 0;

  FDK_ASSERT(self->residualCoding == 0);

  param2UMX_PS_Core__FDK(self->ottCLD__FDK[ottBoxIndx][parameterSetIndx],
                         self->ottICC__FDK[ottBoxIndx][parameterSetIndx],
                         self->numOttBands[ottBoxIndx], residualBands, H11, H12,
                         H21, H22, c_l, c_r);

  for (band = self->numOttBands[ottBoxIndx]; band < self->numParameterBands;
       band++) {
    H11[band] = H21[band] = H12[band] = H22[band] = FL2FXCONST_DBL(0.f);
  }

  if (frame->phaseMode) {
    calculateOpd(self, ottBoxIndx, parameterSetIndx, opd);

    for (band = 0; band < numIpdBands; band++) {
      self->PhaseLeft__FDK[band] = wrapPhase(opd[2 * band]);
      self->PhaseRight__FDK[band] = wrapPhase(opd[2 * band + 1]);
    }
  }

  for (band = numIpdBands; band < numOttBands; band++) {
    self->PhaseLeft__FDK[band] = FL2FXCONST_DBL(0.0f);
    self->PhaseRight__FDK[band] = FL2FXCONST_DBL(0.0f);
  }
}

FDK_INLINE void param2UMX_Prediction_Core__FDK(
    FIXP_DBL* H11re, FIXP_DBL* H11im, FIXP_DBL* H12re, FIXP_DBL* H12im,
    FIXP_DBL* H21re, FIXP_DBL* H21im, FIXP_DBL* H22re, FIXP_DBL* H22im,
    int cldIdx, int iccIdx, int ipdIdx, int band, int numOttBandsIPD,
    int resBands) {
#define MAX_WEIGHT (1.2f)
  FDK_ASSERT((H12im == NULL) && (H22im == NULL)); /* always == 0 */

  if ((band < numOttBandsIPD) && (cldIdx == 15) && (iccIdx == 0) &&
      (ipdIdx == 8)) {
    const FIXP_DBL gain =
        FL2FXCONST_DBL(0.5f / MAX_WEIGHT) >> SCALE_PARAM_M2_212_PRED;

    *H11re = gain;
    if (band < resBands) {
      *H21re = gain;
      *H12re = gain;
      *H22re = -gain;
    } else {
      *H21re = -gain;
      *H12re = (FIXP_DBL)0;
      *H22re = (FIXP_DBL)0;
    }
    if ((H11im != NULL) &&
        (H21im != NULL) /*&& (H12im!=NULL) && (H22im!=NULL)*/) {
      *H11im = (FIXP_DBL)0;
      *H21im = (FIXP_DBL)0;
      /* *H12im = (FIXP_DBL)0; */
      /* *H22im = (FIXP_DBL)0; */
    }
  } else {
    const FIXP_DBL one_m = (FIXP_DBL)MAXVAL_DBL;
    const int one_e = 0;
    /* iidLin = sqrt(cld); */
    FIXP_DBL iidLin_m = sqrt_CLD_m[cldIdx];
    int iidLin_e = sqrt_CLD_e[cldIdx];
    /* iidLin2 = cld; */
    FIXP_DBL iidLin2_m = CLD_m[cldIdx];
    int iidLin2_e = sqrt_CLD_e[cldIdx] << 1;
    /* iidLin21 = iidLin2 + 1.0f; */
    int iidLin21_e;
    FIXP_DBL iidLin21_m =
        fAddNorm(iidLin2_m, iidLin2_e, one_m, one_e, &iidLin21_e);
    /* iidIcc2 = iidLin * icc * 2.0f; */
    FIXP_CFG icc = dequantICC__FDK[iccIdx];
    int iidIcc2_e = iidLin_e + 1;
    FIXP_DBL iidIcc2_m = fMult(iidLin_m, icc);
    FIXP_DBL temp_m, sqrt_temp_m, inv_temp_m, weight_m;
    int temp_e, sqrt_temp_e, inv_temp_e, weight_e, scale;
    FIXP_DBL cosIpd, sinIpd;

    cosIpd = COS_IPD((band < numOttBandsIPD) ? ipdIdx : 0);
    sinIpd = SIN_IPD((band < numOttBandsIPD) ? ipdIdx : 0);

    /* temp    = iidLin21 + iidIcc2 * cosIpd; */
    temp_m = fAddNorm(iidLin21_m, iidLin21_e, fMult(iidIcc2_m, cosIpd),
                      iidIcc2_e, &temp_e);

    /* calculate 1/temp needed later */
    inv_temp_e = temp_e;
    inv_temp_m = invFixp(temp_m, &inv_temp_e);

    /* 1/weight = sqrt(temp) * 1/sqrt(iidLin21) */
    if (temp_e & 1) {
      sqrt_temp_m = temp_m >> 1;
      sqrt_temp_e = (temp_e + 1) >> 1;
    } else {
      sqrt_temp_m = temp_m;
      sqrt_temp_e = temp_e >> 1;
    }
    sqrt_temp_m = sqrtFixp(sqrt_temp_m);
    if (iidLin21_e & 1) {
      iidLin21_e += 1;
      iidLin21_m >>= 1;
    }
    /* weight_[m,e] is actually 1/weight in the next few lines */
    weight_m = invSqrtNorm2(iidLin21_m, &weight_e);
    weight_e -= iidLin21_e >> 1;
    weight_m = fMult(sqrt_temp_m, weight_m);
    weight_e += sqrt_temp_e;
    scale = fNorm(weight_m);
    weight_m = scaleValue(weight_m, scale);
    weight_e -= scale;
    /* weight = 0.5 * max(1/weight, 1/maxWeight) */
    if ((weight_e < 0) ||
        ((weight_e == 0) && (weight_m < FL2FXCONST_DBL(1.f / MAX_WEIGHT)))) {
      weight_m = FL2FXCONST_DBL(1.f / MAX_WEIGHT);
      weight_e = 0;
    }
    weight_e -= 1;

    {
      FIXP_DBL alphaRe_m, alphaIm_m, accu_m;
      int alphaRe_e, alphaIm_e, accu_e;
      /* alphaRe = (1.0f - iidLin2) / temp; */
      alphaRe_m = fAddNorm(one_m, one_e, -iidLin2_m, iidLin2_e, &alphaRe_e);
      alphaRe_m = fMult(alphaRe_m, inv_temp_m);
      alphaRe_e += inv_temp_e;

      /* H11re = weight - alphaRe * weight; */
      /* H21re = weight + alphaRe * weight; */
      accu_m = fMult(alphaRe_m, weight_m);
      accu_e = alphaRe_e + weight_e;
      {
        int accu2_e;
        FIXP_DBL accu2_m;
        accu2_m = fAddNorm(weight_m, weight_e, -accu_m, accu_e, &accu2_e);
        *H11re = scaleValue(accu2_m, accu2_e - SCALE_PARAM_M2_212_PRED);
        accu2_m = fAddNorm(weight_m, weight_e, accu_m, accu_e, &accu2_e);
        *H21re = scaleValue(accu2_m, accu2_e - SCALE_PARAM_M2_212_PRED);
      }

      if ((H11im != NULL) &&
          (H21im != NULL) /*&& (H12im != NULL) && (H22im != NULL)*/) {
        /* alphaIm = -iidIcc2 * sinIpd / temp; */
        alphaIm_m = fMult(-iidIcc2_m, sinIpd);
        alphaIm_m = fMult(alphaIm_m, inv_temp_m);
        alphaIm_e = iidIcc2_e + inv_temp_e;
        /* H11im = -alphaIm * weight; */
        /* H21im =  alphaIm * weight; */
        accu_m = fMult(alphaIm_m, weight_m);
        accu_e = alphaIm_e + weight_e;
        accu_m = scaleValue(accu_m, accu_e - SCALE_PARAM_M2_212_PRED);
        *H11im = -accu_m;
        *H21im = accu_m;

        /* *H12im = (FIXP_DBL)0; */
        /* *H22im = (FIXP_DBL)0; */
      }
    }
    if (band < resBands) {
      FIXP_DBL weight =
          scaleValue(weight_m, weight_e - SCALE_PARAM_M2_212_PRED);
      *H12re = weight;
      *H22re = -weight;
    } else {
      /* beta = 2.0f * iidLin * (float) sqrt(1.0f - icc * icc) * weight / temp;
       */
      FIXP_DBL beta_m;
      int beta_e;
      beta_m = FX_SGL2FX_DBL(sqrt_one_minus_ICC2[iccIdx]);
      beta_e = 1; /* multipication with 2.0f */
      beta_m = fMult(beta_m, weight_m);
      beta_e += weight_e;
      beta_m = fMult(beta_m, iidLin_m);
      beta_e += iidLin_e;
      beta_m = fMult(beta_m, inv_temp_m);
      beta_e += inv_temp_e;

      beta_m = scaleValue(beta_m, beta_e - SCALE_PARAM_M2_212_PRED);
      *H12re = beta_m;
      *H22re = -beta_m;
    }
  }
}

static void param2UMX_Prediction__FDK(spatialDec* self, FIXP_DBL* H11re,
                                      FIXP_DBL* H11im, FIXP_DBL* H12re,
                                      FIXP_DBL* H12im, FIXP_DBL* H21re,
                                      FIXP_DBL* H21im, FIXP_DBL* H22re,
                                      FIXP_DBL* H22im, int ottBoxIndx,
                                      int parameterSetIndx, int resBands) {
  int band;
  FDK_ASSERT((H12im == NULL) && (H22im == NULL)); /* always == 0 */

  for (band = 0; band < self->numParameterBands; band++) {
    int cldIdx = self->ottCLD__FDK[ottBoxIndx][parameterSetIndx][band];
    int iccIdx = self->ottICC__FDK[ottBoxIndx][parameterSetIndx][band];
    int ipdIdx = self->ottIPD__FDK[ottBoxIndx][parameterSetIndx][band];

    param2UMX_Prediction_Core__FDK(
        &H11re[band], (H11im ? &H11im[band] : NULL), &H12re[band], NULL,
        &H21re[band], (H21im ? &H21im[band] : NULL), &H22re[band], NULL, cldIdx,
        iccIdx, ipdIdx, band, self->numOttBandsIPD, resBands);
  }
}

/*******************************************************************************
 Functionname:  initM1andM2
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/

SACDEC_ERROR initM1andM2(spatialDec* self, int initStatesFlag,
                         int configChanged) {
  SACDEC_ERROR err = MPS_OK;

  self->bOverwriteM1M2prev = (configChanged && !initStatesFlag) ? 1 : 0;

  { self->numM2rows = self->numOutputChannels; }

  if (initStatesFlag) {
    int i, j, k;

    for (i = 0; i < self->numM2rows; i++) {
      for (j = 0; j < self->numVChannels; j++) {
        for (k = 0; k < MAX_PARAMETER_BANDS; k++) {
          self->M2Real__FDK[i][j][k] = FL2FXCONST_DBL(0);
          self->M2RealPrev__FDK[i][j][k] = FL2FXCONST_DBL(0);
        }
      }
    }
  }

  return err;
}
