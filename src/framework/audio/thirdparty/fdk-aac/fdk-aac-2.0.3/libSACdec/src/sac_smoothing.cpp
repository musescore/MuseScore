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

   Author(s):

   Description: SAC Dec parameter smoothing

*******************************************************************************/

#include "sac_dec.h"
#include "sac_bitdec.h"
#include "sac_smoothing.h"
#include "sac_rom.h"

/*******************************************************************************
 Functionname: calcFilterCoeff
 *******************************************************************************

 Description:

 Arguments:

 Input:

 Output:


*******************************************************************************/
static FIXP_DBL calcFilterCoeff__FDK(spatialDec *self, int ps,
                                     const SPATIAL_BS_FRAME *frame) {
  int dSlots;
  FIXP_DBL delta;

  dSlots = frame->paramSlot[ps] - self->smoothState->prevParamSlot;

  if (dSlots <= 0) {
    dSlots += self->timeSlots;
  }

  delta = fDivNorm(dSlots, self->smgTime[ps]);

  return delta;
}

/*******************************************************************************
 Functionname: getSmoothOnOff
 *******************************************************************************

 Description:

 Arguments:

 Input:

 Output:


*******************************************************************************/
static int getSmoothOnOff(spatialDec *self, int ps, int pb) {
  int smoothBand = 0;

  smoothBand = self->smgData[ps][pb];

  return smoothBand;
}

void SpatialDecSmoothM1andM2(spatialDec *self, const SPATIAL_BS_FRAME *frame,
                             int ps) {
  FIXP_DBL delta__FDK;
  FIXP_DBL one_minus_delta__FDK;

  int pb, row, col;
  int residualBands = 0;

  if (self->residualCoding) {
    int i;
    int boxes = self->numOttBoxes;
    for (i = 0; i < boxes; i++) {
      if (self->residualBands[i] > residualBands) {
        residualBands = self->residualBands[i];
      }
    }
  }

  delta__FDK = calcFilterCoeff__FDK(self, ps, frame);
  if (delta__FDK == /*FL2FXCONST_DBL(1.0f)*/ (FIXP_DBL)MAXVAL_DBL)
    one_minus_delta__FDK = FL2FXCONST_DBL(0.0f);
  else if (delta__FDK == FL2FXCONST_DBL(0.0f))
    one_minus_delta__FDK = /*FL2FXCONST_DBL(1.0f)*/ (FIXP_DBL)MAXVAL_DBL;
  else
    one_minus_delta__FDK = (FL2FXCONST_DBL(0.5f) - (delta__FDK >> 1)) << 1;

  for (pb = 0; pb < self->numParameterBands; pb++) {
    int smoothBand;

    smoothBand = getSmoothOnOff(self, ps, pb);

    if (smoothBand && (pb >= residualBands)) {
      for (row = 0; row < self->numM2rows; row++) {
        for (col = 0; col < self->numVChannels; col++) {
          self->M2Real__FDK[row][col][pb] =
              ((fMultDiv2(delta__FDK, self->M2Real__FDK[row][col][pb]) +
                fMultDiv2(one_minus_delta__FDK,
                          self->M2RealPrev__FDK[row][col][pb]))
               << 1);
          if (0 || (self->phaseCoding == 3)) {
            self->M2Imag__FDK[row][col][pb] =
                ((fMultDiv2(delta__FDK, self->M2Imag__FDK[row][col][pb]) +
                  fMultDiv2(one_minus_delta__FDK,
                            self->M2ImagPrev__FDK[row][col][pb]))
                 << 1);
          }
        }
      }
    }
  }
  self->smoothState->prevParamSlot = frame->paramSlot[ps];
}

/* init states */
void initParameterSmoothing(spatialDec *self) {
  self->smoothState->prevParamSlot = 0;
}

void SpatialDecSmoothOPD(spatialDec *self, const SPATIAL_BS_FRAME *frame,
                         int ps) {
  int pb;
  int dSlots;
  FIXP_DBL delta__FDK;
  FIXP_DBL one_minus_delta__FDK;
  FIXP_DBL *phaseLeftSmooth__FDK = self->smoothState->opdLeftState__FDK;
  FIXP_DBL *phaseRightSmooth__FDK = self->smoothState->opdRightState__FDK;
  int quantCoarse;

  quantCoarse = frame->IPDLosslessData[0].bsQuantCoarseXXX[ps];

  if (frame->OpdSmoothingMode == 0) {
    FDKmemcpy(phaseLeftSmooth__FDK, self->PhaseLeft__FDK,
              self->numParameterBands * sizeof(FIXP_DBL));
    FDKmemcpy(phaseRightSmooth__FDK, self->PhaseRight__FDK,
              self->numParameterBands * sizeof(FIXP_DBL));
  } else {
    if (ps == 0) {
      dSlots = frame->paramSlot[ps] + 1;
    } else {
      dSlots = frame->paramSlot[ps] - frame->paramSlot[ps - 1];
    }

    delta__FDK = (FIXP_DBL)((INT)(FL2FXCONST_DBL(0.0078125f)) * dSlots);

    if (delta__FDK == (FIXP_DBL)MAXVAL_DBL /*FL2FXCONST_DBL(1.0f)*/)
      one_minus_delta__FDK = FL2FXCONST_DBL(0.0f);
    else if (delta__FDK == FL2FXCONST_DBL(0.0f))
      one_minus_delta__FDK = (FIXP_DBL)MAXVAL_DBL /*FL2FXCONST_DBL(1.0f)*/;
    else
      one_minus_delta__FDK = (FL2FXCONST_DBL(0.5f) - (delta__FDK >> 1)) << 1;

    for (pb = 0; pb < self->numParameterBands; pb++) {
      FIXP_DBL tmpL, tmpR, tmp;

      tmpL = self->PhaseLeft__FDK[pb];
      tmpR = self->PhaseRight__FDK[pb];

      while (tmpL > phaseLeftSmooth__FDK[pb] + PI__IPD) tmpL -= PI__IPD << 1;
      while (tmpL < phaseLeftSmooth__FDK[pb] - PI__IPD) tmpL += PI__IPD << 1;
      while (tmpR > phaseRightSmooth__FDK[pb] + PI__IPD) tmpR -= PI__IPD << 1;
      while (tmpR < phaseRightSmooth__FDK[pb] - PI__IPD) tmpR += PI__IPD << 1;

      phaseLeftSmooth__FDK[pb] =
          fMult(delta__FDK, tmpL) +
          fMult(one_minus_delta__FDK, phaseLeftSmooth__FDK[pb]);
      phaseRightSmooth__FDK[pb] =
          fMult(delta__FDK, tmpR) +
          fMult(one_minus_delta__FDK, phaseRightSmooth__FDK[pb]);

      tmp = (((tmpL >> 1) - (tmpR >> 1)) - ((phaseLeftSmooth__FDK[pb] >> 1) -
                                            (phaseRightSmooth__FDK[pb] >> 1)))
            << 1;
      while (tmp > PI__IPD) tmp -= PI__IPD << 1;
      while (tmp < -PI__IPD) tmp += PI__IPD << 1;
      if (fixp_abs(tmp) > fMult((quantCoarse ? FL2FXCONST_DBL(50.f / 180.f)
                                             : FL2FXCONST_DBL(25.f / 180.f)),
                                PI__IPD)) {
        phaseLeftSmooth__FDK[pb] = tmpL;
        phaseRightSmooth__FDK[pb] = tmpR;
      }

      while (phaseLeftSmooth__FDK[pb] > PI__IPD << 1)
        phaseLeftSmooth__FDK[pb] -= PI__IPD << 1;
      while (phaseLeftSmooth__FDK[pb] < (FIXP_DBL)0)
        phaseLeftSmooth__FDK[pb] += PI__IPD << 1;
      while (phaseRightSmooth__FDK[pb] > PI__IPD << 1)
        phaseRightSmooth__FDK[pb] -= PI__IPD << 1;
      while (phaseRightSmooth__FDK[pb] < (FIXP_DBL)0)
        phaseRightSmooth__FDK[pb] += PI__IPD << 1;

      self->PhaseLeft__FDK[pb] = phaseLeftSmooth__FDK[pb];
      self->PhaseRight__FDK[pb] = phaseRightSmooth__FDK[pb];
    }
  }
  return;
}
