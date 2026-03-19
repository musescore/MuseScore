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

/******************* Library for basic calculation routines ********************

   Author(s):   Manuel Jander

   Description: LPC related functions

*******************************************************************************/

#include "FDK_lpc.h"

/* Internal scaling of LPC synthesis to avoid overflow of filte states.
   This depends on the LPC order, because the LPC order defines the amount
   of MAC operations. */
static SCHAR order_ld[LPC_MAX_ORDER] = {
    /* Assume that Synthesis filter output does not clip and filter
       accu does change no more than 1.0 for each iteration.
       ceil(0.5*log((1:24))/log(2)) */
    0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};

/* IIRLattice */
#ifndef FUNCTION_CLpc_SynthesisLattice_SGL
void CLpc_SynthesisLattice(FIXP_DBL *signal, const int signal_size,
                           const int signal_e, const int signal_e_out,
                           const int inc, const FIXP_SGL *coeff,
                           const int order, FIXP_DBL *state) {
  int i, j;
  FIXP_DBL *pSignal;
  int shift;

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(order > 0);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /*
    tmp = x(k) - K(M)*g(M);
    for m=M-1:-1:1
            tmp = tmp - K(m) * g(m);
            g(m+1) = g(m) + K(m) * tmp;
    endfor
    g(1) = tmp;

    y(k) = tmp;
  */

  shift = -order_ld[order - 1];

  for (i = signal_size; i != 0; i--) {
    FIXP_DBL *pState = state + order - 1;
    const FIXP_SGL *pCoeff = coeff + order - 1;
    FIXP_DBL tmp;

    tmp = scaleValue(*pSignal, shift + signal_e) -
          fMultDiv2(*pCoeff--, *pState--);
    for (j = order - 1; j != 0; j--) {
      tmp = fMultSubDiv2(tmp, pCoeff[0], pState[0]);
      pState[1] = pState[0] + (fMultDiv2(*pCoeff--, tmp) << 2);
      pState--;
    }

    *pSignal = scaleValueSaturate(tmp, -shift - signal_e_out);

    /* exponent of state[] is -1 */
    pState[1] = tmp << 1;
    pSignal += inc;
  }
}
#endif

#ifndef FUNCTION_CLpc_SynthesisLattice_DBL
void CLpc_SynthesisLattice(FIXP_DBL *signal, const int signal_size,
                           const int signal_e, const int signal_e_out,
                           const int inc, const FIXP_DBL *coeff,
                           const int order, FIXP_DBL *state) {
  int i, j;
  FIXP_DBL *pSignal;

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(order > 0);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  FDK_ASSERT(signal_size > 0);
  for (i = signal_size; i != 0; i--) {
    FIXP_DBL *pState = state + order - 1;
    const FIXP_DBL *pCoeff = coeff + order - 1;
    FIXP_DBL tmp, accu;

    accu =
        fMultSubDiv2(scaleValue(*pSignal, signal_e - 1), *pCoeff--, *pState--);
    tmp = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);

    for (j = order - 1; j != 0; j--) {
      accu = fMultSubDiv2(tmp >> 1, pCoeff[0], pState[0]);
      tmp = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);

      accu = fMultAddDiv2(pState[0] >> 1, *pCoeff--, tmp);
      pState[1] = SATURATE_LEFT_SHIFT_ALT(accu, 1, DFRACT_BITS);

      pState--;
    }

    *pSignal = scaleValue(tmp, -signal_e_out);

    /* exponent of state[] is 0 */
    pState[1] = tmp;
    pSignal += inc;
  }
}

#endif

/* LPC_SYNTHESIS_IIR version */
void CLpc_Synthesis(FIXP_DBL *signal, const int signal_size, const int signal_e,
                    const int inc, const FIXP_LPC_TNS *lpcCoeff_m,
                    const int lpcCoeff_e, const int order, FIXP_DBL *state,
                    int *pStateIndex) {
  int i, j;
  FIXP_DBL *pSignal;
  int stateIndex = *pStateIndex;

  FIXP_LPC_TNS coeff[2 * LPC_MAX_ORDER];
  FDKmemcpy(&coeff[0], lpcCoeff_m, order * sizeof(FIXP_LPC_TNS));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order * sizeof(FIXP_LPC_TNS));

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(stateIndex < order);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */

  for (i = 0; i < signal_size; i++) {
    FIXP_DBL x;
    const FIXP_LPC_TNS *pCoeff = coeff + order - stateIndex;

    x = scaleValue(*pSignal, -(lpcCoeff_e + 1));
    for (j = 0; j < order; j++) {
      x -= fMultDiv2(state[j], pCoeff[j]);
    }
    x = SATURATE_SHIFT(x, -lpcCoeff_e - 1, DFRACT_BITS);

    /* Update states */
    stateIndex = ((stateIndex - 1) < 0) ? (order - 1) : (stateIndex - 1);
    state[stateIndex] = x;

    *pSignal = scaleValue(x, signal_e);
    pSignal += inc;
  }

  *pStateIndex = stateIndex;
}
/* default version */
void CLpc_Synthesis(FIXP_DBL *signal, const int signal_size, const int signal_e,
                    const int inc, const FIXP_LPC *lpcCoeff_m,
                    const int lpcCoeff_e, const int order, FIXP_DBL *state,
                    int *pStateIndex) {
  int i, j;
  FIXP_DBL *pSignal;
  int stateIndex = *pStateIndex;

  FIXP_LPC coeff[2 * LPC_MAX_ORDER];
  FDKmemcpy(&coeff[0], lpcCoeff_m, order * sizeof(FIXP_LPC));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order * sizeof(FIXP_LPC));

  FDK_ASSERT(order <= LPC_MAX_ORDER);
  FDK_ASSERT(stateIndex < order);

  if (inc == -1)
    pSignal = &signal[signal_size - 1];
  else
    pSignal = &signal[0];

  /* y(n) = x(n) - lpc[1]*y(n-1) - ... - lpc[order]*y(n-order) */

  for (i = 0; i < signal_size; i++) {
    FIXP_DBL x;
    const FIXP_LPC *pCoeff = coeff + order - stateIndex;

    x = scaleValue(*pSignal, -(lpcCoeff_e + 1));
    for (j = 0; j < order; j++) {
      x -= fMultDiv2(state[j], pCoeff[j]);
    }
    x = SATURATE_SHIFT(x, -lpcCoeff_e - 1, DFRACT_BITS);

    /* Update states */
    stateIndex = ((stateIndex - 1) < 0) ? (order - 1) : (stateIndex - 1);
    state[stateIndex] = x;

    *pSignal = scaleValue(x, signal_e);
    pSignal += inc;
  }

  *pStateIndex = stateIndex;
}

/* FIR */
void CLpc_Analysis(FIXP_DBL *RESTRICT signal, const int signal_size,
                   const FIXP_LPC lpcCoeff_m[], const int lpcCoeff_e,
                   const int order, FIXP_DBL *RESTRICT filtState,
                   int *filtStateIndex) {
  int stateIndex;
  INT i, j, shift = lpcCoeff_e + 1; /* +1, because fMultDiv2 */
  FIXP_DBL tmp;

  if (order <= 0) {
    return;
  }
  if (filtStateIndex != NULL) {
    stateIndex = *filtStateIndex;
  } else {
    stateIndex = 0;
  }

  /* keep filter coefficients twice and save memory copy operation in
     modulo state buffer */
  FIXP_LPC coeff[2 * LPC_MAX_ORDER];
  FIXP_LPC *pCoeff;
  FDKmemcpy(&coeff[0], lpcCoeff_m, order * sizeof(FIXP_LPC));
  FDKmemcpy(&coeff[order], lpcCoeff_m, order * sizeof(FIXP_LPC));

  /*
      # Analysis filter, obtain residual.
      for k = 0:BL-1
              err(i-BL+k) = a * inputSignal(i-BL+k:-1:i-BL-M+k);
      endfor
   */

  FDK_ASSERT(shift >= 0);

  for (j = 0; j < signal_size; j++) {
    pCoeff = &coeff[(order - stateIndex)];

    tmp = signal[j] >> shift;
    for (i = 0; i < order; i++) {
      tmp = fMultAddDiv2(tmp, pCoeff[i], filtState[i]);
    }

    stateIndex =
        ((stateIndex - 1) < 0) ? (stateIndex - 1 + order) : (stateIndex - 1);
    filtState[stateIndex] = signal[j];

    signal[j] = tmp << shift;
  }

  if (filtStateIndex != NULL) {
    *filtStateIndex = stateIndex;
  }
}

/* For the LPC_SYNTHESIS_IIR version */
INT CLpc_ParcorToLpc(const FIXP_LPC_TNS reflCoeff[], FIXP_LPC_TNS LpcCoeff[],
                     INT numOfCoeff, FIXP_DBL workBuffer[]) {
  INT i, j;
  INT shiftval,
      par2LpcShiftVal = 6; /* 6 should be enough, bec. max(numOfCoeff) = 20 */
  FIXP_DBL maxVal = (FIXP_DBL)0;

  workBuffer[0] = FX_LPC_TNS2FX_DBL(reflCoeff[0]) >> par2LpcShiftVal;
  for (i = 1; i < numOfCoeff; i++) {
    for (j = 0; j < i / 2; j++) {
      FIXP_DBL tmp1, tmp2;

      tmp1 = workBuffer[j];
      tmp2 = workBuffer[i - 1 - j];
      workBuffer[j] += fMult(reflCoeff[i], tmp2);
      workBuffer[i - 1 - j] += fMult(reflCoeff[i], tmp1);
    }
    if (i & 1) {
      workBuffer[j] += fMult(reflCoeff[i], workBuffer[j]);
    }

    workBuffer[i] = FX_LPC_TNS2FX_DBL(reflCoeff[i]) >> par2LpcShiftVal;
  }

  /* calculate exponent */
  for (i = 0; i < numOfCoeff; i++) {
    maxVal = fMax(maxVal, fAbs(workBuffer[i]));
  }

  shiftval = fMin(fNorm(maxVal), par2LpcShiftVal);

  for (i = 0; i < numOfCoeff; i++) {
    LpcCoeff[i] = FX_DBL2FX_LPC_TNS(workBuffer[i] << shiftval);
  }

  return (par2LpcShiftVal - shiftval);
}
/* Default version */
INT CLpc_ParcorToLpc(const FIXP_LPC reflCoeff[], FIXP_LPC LpcCoeff[],
                     INT numOfCoeff, FIXP_DBL workBuffer[]) {
  INT i, j;
  INT shiftval,
      par2LpcShiftVal = 6; /* 6 should be enough, bec. max(numOfCoeff) = 20 */
  FIXP_DBL maxVal = (FIXP_DBL)0;

  workBuffer[0] = FX_LPC2FX_DBL(reflCoeff[0]) >> par2LpcShiftVal;
  for (i = 1; i < numOfCoeff; i++) {
    for (j = 0; j < i / 2; j++) {
      FIXP_DBL tmp1, tmp2;

      tmp1 = workBuffer[j];
      tmp2 = workBuffer[i - 1 - j];
      workBuffer[j] += fMult(reflCoeff[i], tmp2);
      workBuffer[i - 1 - j] += fMult(reflCoeff[i], tmp1);
    }
    if (i & 1) {
      workBuffer[j] += fMult(reflCoeff[i], workBuffer[j]);
    }

    workBuffer[i] = FX_LPC2FX_DBL(reflCoeff[i]) >> par2LpcShiftVal;
  }

  /* calculate exponent */
  for (i = 0; i < numOfCoeff; i++) {
    maxVal = fMax(maxVal, fAbs(workBuffer[i]));
  }

  shiftval = fMin(fNorm(maxVal), par2LpcShiftVal);

  for (i = 0; i < numOfCoeff; i++) {
    LpcCoeff[i] = FX_DBL2FX_LPC(workBuffer[i] << shiftval);
  }

  return (par2LpcShiftVal - shiftval);
}

void CLpc_AutoToParcor(FIXP_DBL acorr[], const int acorr_e,
                       FIXP_LPC reflCoeff[], const int numOfCoeff,
                       FIXP_DBL *pPredictionGain_m, INT *pPredictionGain_e) {
  INT i, j, scale = 0;
  FIXP_DBL parcorWorkBuffer[LPC_MAX_ORDER];

  FIXP_DBL *workBuffer = parcorWorkBuffer;
  FIXP_DBL autoCorr_0 = acorr[0];

  FDKmemclear(reflCoeff, numOfCoeff * sizeof(FIXP_LPC));

  if (autoCorr_0 == FL2FXCONST_DBL(0.0)) {
    if (pPredictionGain_m != NULL) {
      *pPredictionGain_m = FL2FXCONST_DBL(0.5f);
      *pPredictionGain_e = 1;
    }
    return;
  }

  FDKmemcpy(workBuffer, acorr + 1, numOfCoeff * sizeof(FIXP_DBL));
  for (i = 0; i < numOfCoeff; i++) {
    LONG sign = ((LONG)workBuffer[0] >> (DFRACT_BITS - 1));
    FIXP_DBL tmp = (FIXP_DBL)((LONG)workBuffer[0] ^ sign);

    /* Check preconditions for division function: num<=denum             */
    /* For 1st iteration acorr[0] cannot be 0, it is checked before loop */
    /* Due to exor operation with "sign", num(=tmp) is greater/equal 0   */
    if (acorr[0] < tmp) break;

    /* tmp = div(num, denum, 16) */
    tmp = (FIXP_DBL)((LONG)schur_div(tmp, acorr[0], FRACT_BITS) ^ (~sign));

    reflCoeff[i] = FX_DBL2FX_LPC(tmp);

    for (j = numOfCoeff - i - 1; j >= 0; j--) {
      FIXP_DBL accu1 = fMult(tmp, acorr[j]);
      FIXP_DBL accu2 = fMult(tmp, workBuffer[j]);
      workBuffer[j] += accu1;
      acorr[j] += accu2;
    }
    /* Check preconditions for division function: denum (=acorr[0]) > 0 */
    if (acorr[0] == (FIXP_DBL)0) break;

    workBuffer++;
  }

  if (pPredictionGain_m != NULL) {
    if (acorr[0] > (FIXP_DBL)0) {
      /* prediction gain = signal power / error (residual) power */
      *pPredictionGain_m = fDivNormSigned(autoCorr_0, acorr[0], &scale);
      *pPredictionGain_e = scale;
    } else {
      *pPredictionGain_m = (FIXP_DBL)0;
      *pPredictionGain_e = 0;
    }
  }
}
