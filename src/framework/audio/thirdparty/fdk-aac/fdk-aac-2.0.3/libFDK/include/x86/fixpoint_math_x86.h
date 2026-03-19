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

   Description: Fixed point specific mathematical functions for x86

*******************************************************************************/

#if !defined(FIXPOINT_MATH_X86_H)
#define FIXPOINT_MATH_X86_H

#define FUNCTION_sqrtFixp

#include <math.h>

#ifdef FUNCTION_sqrtFixp
static inline FIXP_DBL sqrtFixp(const FIXP_DBL op) {
  FIXP_DBL result;
  /* result =
   * (FIXP_DBL)(INT)(sqrt((double)(INT)op)*46340.950011841578559133736114903);
   */
  result = (FIXP_DBL)(INT)(sqrt((float)(INT)op) * 46340.9492f);
  FDK_ASSERT(result >= (FIXP_DBL)0);
  return result;
}
#endif /* FUNCTION_sqrtFixp */

#include <math.h>

#define FUNCTION_invSqrtNorm2
/**
 * \brief calculate 1.0/sqrt(op)
 * \param op_m mantissa of input value.
 * \param result_e pointer to return the exponent of the result
 * \return mantissa of the result
 */
#ifdef FUNCTION_invSqrtNorm2
inline FIXP_DBL invSqrtNorm2(FIXP_DBL op_m, INT *result_e) {
  float result;
  if (op_m == (FIXP_DBL)0) {
    *result_e = 16;
    return ((LONG)0x7fffffff);
  }
  result = (float)(1.0 / sqrt(0.5f * (float)(INT)op_m));
  result = (float)ldexp(frexpf(result, result_e), DFRACT_BITS - 1);
  *result_e += 15;

  FDK_ASSERT(result >= 0);
  return (FIXP_DBL)(INT)result;
}
#endif /* FUNCTION_invSqrtNorm2 */

#define FUNCTION_invFixp
/**
 * \brief calculate 1.0/op
 * \param op mantissa of the input value.
 * \return mantissa of the result with implizit exponent of 31
 */
#ifdef FUNCTION_invFixp
inline FIXP_DBL invFixp(FIXP_DBL op) {
  float result;
  INT result_e;
  if ((op == (FIXP_DBL)0) || (op == (FIXP_DBL)1)) {
    return ((LONG)0x7fffffff);
  }
  result = (float)(1.0 / (float)(INT)op);
  result = frexpf(result, &result_e);
  result = ldexpf(result, 31 + result_e);

  return (FIXP_DBL)(INT)result;
}

/**
 * \brief calculate 1.0/(op_m * 2^op_e)
 * \param op_m mantissa of the input value.
 * \param op_e pointer into were the exponent of the input value is stored, and
 * the result will be stored into.
 * \return mantissa of the result
 */
inline FIXP_DBL invFixp(FIXP_DBL op_m, int *op_e) {
  float result;
  INT result_e;
  if ((op_m == (FIXP_DBL)0x00000000) || (op_m == (FIXP_DBL)0x00000001)) {
    *op_e = 31 - *op_e;
    return ((LONG)0x7fffffff);
  }
  result = (float)(1.0 / (float)(INT)op_m);
  result = ldexpf(frexpf(result, &result_e), DFRACT_BITS - 1);
  *op_e = result_e - *op_e + 31;
  return (FIXP_DBL)(INT)result;
}
#endif /* FUNCTION_invFixp */

#define FUNCTION_schur_div
/**
 * \brief Divide two FIXP_DBL values with given precision.
 * \param num dividend
 * \param denum divisor
 * \param count amount of significant bits of the result (starting to the MSB)
 * \return num/divisor
 */
#ifdef FUNCTION_schur_div
inline FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, INT count) {
  (void)count;
  /* same asserts than for fallback implementation */
  FDK_ASSERT(num >= (FIXP_DBL)0);
  FDK_ASSERT(denum > (FIXP_DBL)0);
  FDK_ASSERT(num <= denum);

  return (num == denum) ? (FIXP_DBL)MAXVAL_DBL
                        : (FIXP_DBL)(INT)(((INT64)(INT)num << 31) / (INT)denum);
}
#endif /* FUNCTION_schur_div */
#endif /* !defined(FIXPOINT_MATH_X86_H) */
