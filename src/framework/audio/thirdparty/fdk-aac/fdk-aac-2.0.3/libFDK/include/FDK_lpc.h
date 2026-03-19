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

#ifndef FDK_LPC_H
#define FDK_LPC_H

#include "common_fix.h"

#define LPC_MAX_ORDER 24

/*
 * Experimental solution for lattice filter substitution.
 * LPC_SYNTHESIS_IIR macro must be activated in aacdec_tns.cpp.
 * When LPC_SYNTHESIS_IIR enabled, there will be a substitution of the default
 * lpc synthesis lattice filter by an IIR synthesis filter (with a conversionof
 * the filter coefs). LPC_TNS related macros are intended to implement the data
 * types used by the CLpc_Synthesis variant which is used for this solution.
 * */

/* #define LPC_TNS_LOWER_PRECISION */

typedef FIXP_DBL FIXP_LPC_TNS;
#define FX_DBL2FX_LPC_TNS(x) (x)
#define FX_DBL2FXCONST_LPC_TNS(x) (x)
#define FX_LPC_TNS2FX_DBL(x) (x)
#define FL2FXCONST_LPC_TNS(val) FL2FXCONST_DBL(val)
#define MAXVAL_LPC_TNS MAXVAL_DBL

typedef FIXP_SGL FIXP_LPC;
#define FX_DBL2FX_LPC(x) FX_DBL2FX_SGL((FIXP_DBL)(x))
#define FX_DBL2FXCONST_LPC(x) FX_DBL2FXCONST_SGL(x)
#define FX_LPC2FX_DBL(x) FX_SGL2FX_DBL(x)
#define FL2FXCONST_LPC(val) FL2FXCONST_SGL(val)
#define MAXVAL_LPC MAXVAL_SGL

/**
 * \brief Obtain residual signal through LPC analysis.
 * \param signal pointer to buffer holding signal to be analysed. Residual is
 * returned there (in place)
 * \param signal_size the size of the input data in pData
 * \param lpcCoeff_m the LPC filter coefficient mantissas
 * \param lpcCoeff_e the LPC filter coefficient exponent
 * \param order the LPC filter order (size of coeff)
 * \param filtState Pointer to state buffer of size order
 * \param filtStateIndex pointer to state index storage
 */
void CLpc_Analysis(FIXP_DBL signal[], const int signal_size,
                   const FIXP_LPC lpcCoeff_m[], const int lpcCoeff_e,
                   const int order, FIXP_DBL *filtState, int *filtStateIndex);

/**
 * \brief Synthesize signal fom residual through LPC synthesis, using LP
 * coefficients.
 * \param signal pointer to buffer holding the residual signal. The synthesis is
 * returned there (in place)
 * \param signal_size the size of the input data in pData
 * \param inc buffer traversal increment for signal
 * \param coeff the LPC filter coefficients
 * \param coeff_e exponent of coeff
 * \param order the LPC filter order (size of coeff)
 * \param state state buffer of size LPC_MAX_ORDER
 * \param pStateIndex pointer to state index storage
 */
void CLpc_Synthesis(FIXP_DBL *signal, const int signal_size, const int signal_e,
                    const int inc, const FIXP_LPC_TNS *lpcCoeff_m,
                    const int lpcCoeff_e, const int order, FIXP_DBL *state,
                    int *pStateIndex);
void CLpc_Synthesis(FIXP_DBL *signal, const int signal_size, const int signal_e,
                    const int inc, const FIXP_LPC coeff[], const int coeff_e,
                    const int order, FIXP_DBL *filtState, int *pStateIndex);

/**
 * \brief Synthesize signal fom residual through LPC synthesis, using ParCor
 * coefficients. The algorithm assumes a filter gain of max 1.0. If the filter
 * gain is higher, this must be accounted into the values of signal_e
 * and/or signal_e_out to avoid overflows.
 * \param signal pointer to buffer holding the residual signal. The synthesis is
 * returned there (in place)
 * \param signal_size the size of the input data in pData
 * \param inc buffer traversal increment for signal
 * \param coeff the LPC filter coefficients
 * \param coeff_e exponent of coeff
 * \param order the LPC filter order (size of coeff)
 * \param state state buffer of size LPC_MAX_ORDER
 */
void CLpc_SynthesisLattice(FIXP_DBL *signal, const int signal_size,
                           const int signal_e, const int signal_e_out,
                           const int inc, const FIXP_SGL *coeff,
                           const int order, FIXP_DBL *state);

void CLpc_SynthesisLattice(FIXP_DBL *RESTRICT signal, const int signal_size,
                           const int signal_e, const int signal_e_out,
                           const int inc, const FIXP_DBL *RESTRICT coeff,
                           const int order, FIXP_DBL *RESTRICT state);

/**
 * \brief
 */
INT CLpc_ParcorToLpc(const FIXP_LPC_TNS reflCoeff[], FIXP_LPC_TNS LpcCoeff[],
                     INT numOfCoeff, FIXP_DBL workBuffer[]);
INT CLpc_ParcorToLpc(const FIXP_LPC reflCoeff[], FIXP_LPC LpcCoeff[],
                     const int numOfCoeff, FIXP_DBL workBuffer[]);

/**
 * \brief Calculate ParCor (Partial autoCorrelation, reflection) coefficients
 * from autocorrelation coefficients using the Schur algorithm (instead of
 * Levinson Durbin).
 * \param acorr order+1 autocorrelation coefficients
 * \param reflCoeff output reflection /ParCor coefficients. The first
 * coefficient which is always 1.0 is ommitted.
 * \param order number of acorr / reflCoeff coefficients.
 * \param pPredictionGain_m prediction gain mantissa
 * \param pPredictionGain_e prediction gain exponent
 */
void CLpc_AutoToParcor(FIXP_DBL acorr[], const int acorr_e,
                       FIXP_LPC reflCoeff[], const int order,
                       FIXP_DBL *pPredictionGain_m, INT *pPredictionGain_e);

#endif /* FDK_LPC_H */
