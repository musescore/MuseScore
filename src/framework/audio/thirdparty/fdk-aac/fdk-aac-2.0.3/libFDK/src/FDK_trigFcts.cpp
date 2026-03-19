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

   Author(s):   Haricharan Lakshman, Manuel Jander

   Description: Trigonometric functions fixed point fractional implementation.

*******************************************************************************/

#include "FDK_trigFcts.h"

#include "fixpoint_math.h"

#define IMPROVE_ATAN2_ACCURACY 1 /* 0 --> 59 dB SNR     1 --> 65 dB SNR */
#define MINSFTAB 7
#define MAXSFTAB 25

#if IMPROVE_ATAN2_ACCURACY
static const FIXP_DBL f_atan_expand_range[MAXSFTAB - (MINSFTAB - 1)] = {
    /*****************************************************************************
     *
     *  Table holds fixp_atan() output values which are outside of input range
     *  of fixp_atan() to improve SNR of fixp_atan2().
     *
     *  This Table might also be used in fixp_atan() so there a wider input
     *  range can be covered, too.
     *
     *****************************************************************************/
    FL2FXCONST_DBL(7.775862990872099e-001),
    FL2FXCONST_DBL(7.814919928673978e-001),
    FL2FXCONST_DBL(7.834450483314648e-001),
    FL2FXCONST_DBL(7.844216021392089e-001),
    FL2FXCONST_DBL(7.849098823026687e-001),
    FL2FXCONST_DBL(7.851540227918509e-001),
    FL2FXCONST_DBL(7.852760930873737e-001),
    FL2FXCONST_DBL(7.853371282415015e-001),
    FL2FXCONST_DBL(7.853676458193612e-001),
    FL2FXCONST_DBL(7.853829046083906e-001),
    FL2FXCONST_DBL(7.853905340029177e-001),
    FL2FXCONST_DBL(7.853943487001828e-001),
    FL2FXCONST_DBL(7.853962560488155e-001),
    FL2FXCONST_DBL(7.853972097231319e-001),
    FL2FXCONST_DBL(7.853976865602901e-001),
    FL2FXCONST_DBL(7.853979249788692e-001),
    FL2FXCONST_DBL(7.853980441881587e-001),
    FL2FXCONST_DBL(7.853981037928035e-001),
    FL2FXCONST_DBL(7.853981335951259e-001)
    /* pi/4 = 0.785398163397448 = pi/2/ATO_SCALE */
};
#endif

FIXP_DBL fixp_atan2(FIXP_DBL y, FIXP_DBL x) {
  FIXP_DBL q;
  FIXP_DBL at;  /* atan  out */
  FIXP_DBL at2; /* atan2 out */
  FIXP_DBL ret = FL2FXCONST_DBL(-1.0f);
  INT sf, sfo, stf;

  /* --- division */

  if (y > FL2FXCONST_DBL(0.0f)) {
    if (x > FL2FXCONST_DBL(0.0f)) {
      q = fDivNormHighPrec(y, x, &sf); /* both pos. */
    } else if (x < FL2FXCONST_DBL(0.0f)) {
      q = -fDivNormHighPrec(y, -x, &sf); /* x neg. */
    } else {                             /* (x == FL2FXCONST_DBL(0.0f)) */
      q = FL2FXCONST_DBL(+1.0f);         /* y/x = pos/zero = +Inf */
      sf = 0;
    }
  } else if (y < FL2FXCONST_DBL(0.0f)) {
    if (x > FL2FXCONST_DBL(0.0f)) {
      q = -fDivNormHighPrec(-y, x, &sf); /* y neg. */
    } else if (x < FL2FXCONST_DBL(0.0f)) {
      q = fDivNormHighPrec(-y, -x, &sf); /* both neg. */
    } else {                             /* (x == FL2FXCONST_DBL(0.0f)) */
      q = FL2FXCONST_DBL(-1.0f);         /* y/x = neg/zero = -Inf */
      sf = 0;
    }
  } else { /* (y == FL2FXCONST_DBL(0.0f)) */
    q = FL2FXCONST_DBL(0.0f);
    sf = 0;
  }
  sfo = sf;

  /* --- atan() */

  if (sfo > ATI_SF) {
  /* --- could not calc fixp_atan() here bec of input data out of range */
  /*     ==> therefore give back boundary values */

#if IMPROVE_ATAN2_ACCURACY
    if (sfo > MAXSFTAB) sfo = MAXSFTAB;
#endif

    if (q > FL2FXCONST_DBL(0.0f)) {
#if IMPROVE_ATAN2_ACCURACY
      at = +f_atan_expand_range[sfo - ATI_SF - 1];
#else
      at = FL2FXCONST_DBL(+M_PI / 2 / ATO_SCALE);
#endif
    } else if (q < FL2FXCONST_DBL(0.0f)) {
#if IMPROVE_ATAN2_ACCURACY
      at = -f_atan_expand_range[sfo - ATI_SF - 1];
#else
      at = FL2FXCONST_DBL(-M_PI / 2 / ATO_SCALE);
#endif
    } else { /* q == FL2FXCONST_DBL(0.0f) */
      at = FL2FXCONST_DBL(0.0f);
    }
  } else {
    /* --- calc of fixp_atan() is possible; input data within range */
    /*     ==> set q on fixed scale level as desired from fixp_atan() */
    stf = sfo - ATI_SF;
    if (stf > 0)
      q = q << (INT)fMin(stf, DFRACT_BITS - 1);
    else
      q = q >> (INT)fMin(-stf, DFRACT_BITS - 1);
    at = fixp_atan(q); /* ATO_SF */
  }

  // --- atan2()

  at2 = at >> (AT2O_SF - ATO_SF);  // now AT2O_SF for atan2
  if (x > FL2FXCONST_DBL(0.0f)) {
    ret = at2;
  } else if (x < FL2FXCONST_DBL(0.0f)) {
    if (y >= FL2FXCONST_DBL(0.0f)) {
      ret = at2 + FL2FXCONST_DBL(M_PI / AT2O_SCALE);
    } else {
      ret = at2 - FL2FXCONST_DBL(M_PI / AT2O_SCALE);
    }
  } else {
    // x == 0
    if (y > FL2FXCONST_DBL(0.0f)) {
      ret = FL2FXCONST_DBL(+M_PI / 2 / AT2O_SCALE);
    } else if (y < FL2FXCONST_DBL(0.0f)) {
      ret = FL2FXCONST_DBL(-M_PI / 2 / AT2O_SCALE);
    } else if (y == FL2FXCONST_DBL(0.0f)) {
      ret = FL2FXCONST_DBL(0.0f);
    }
  }
  return ret;
}

FIXP_DBL fixp_atan(FIXP_DBL x) {
  INT sign;
  FIXP_DBL result, temp;

  /* SNR of fixp_atan() = 56 dB */
  FIXP_DBL P281 = (FIXP_DBL)0x00013000;     // 0.281 in q18
  FIXP_DBL ONEP571 = (FIXP_DBL)0x6487ef00;  // 1.571 in q30

  if (x < FIXP_DBL(0)) {
    sign = 1;
    x = -x;
  } else {
    sign = 0;
  }
  FDK_ASSERT(FL2FXCONST_DBL(1.0 / 64.0) == Q(Q_ATANINP));
  /* calc of arctan */
  if (x < FL2FXCONST_DBL(1.0 / 64.0))
  /*
    Chebyshev polynomial approximation of atan(x)
    5th-order approximation: atan(x) = a1*x + a2*x^3 + a3*x^5 = x(a1 + x^2*(a2 +
    a3*x^2)); a1 = 0.9949493661166540f, a2 = 0.2870606355326520f, a3 =
    0.0780371764464410f; 7th-order approximation: atan(x) = a1*x + a2*x^3 +
    a3*x^5 + a3*x^7 = x(a1 + x^2*(a2 + x^2*(a3 + a4*x^2))); a1 =
    0.9991334482227801, a2 = -0.3205332923816640, a3 = 0.1449824901444650, a4 =
    -0.0382544649702990; 7th-order approximation in use (the most accurate
    solution)
  */
  {
    x <<= ATI_SF;
    FIXP_DBL x2 = fPow2(x);
    temp = fMultAddDiv2((FL2FXCONST_DBL(0.1449824901444650f) >> 1), x2,
                        FL2FXCONST_DBL(-0.0382544649702990));
    temp = fMultAddDiv2((FL2FXCONST_DBL(-0.3205332923816640f) >> 2), x2, temp);
    temp = fMultAddDiv2((FL2FXCONST_DBL(0.9991334482227801f) >> 3), x2, temp);
    result = fMult(x, (temp << 2));
  } else if (x < FL2FXCONST_DBL(1.28 / 64.0)) {
    FIXP_DBL delta_fix;
    FIXP_DBL PI_BY_4 = FL2FXCONST_DBL(3.1415926 / 4.0) >> 1; /* pi/4 in q30 */

    delta_fix = (x - FL2FXCONST_DBL(1.0 / 64.0)) << 5; /* q30 */
    result = PI_BY_4 + (delta_fix >> 1) - (fPow2Div2(delta_fix));
  } else {
    /* Other approximation for |x| > 1.28 */
    INT res_e;

    temp = fPow2Div2(x); /* q25 * q25 - (DFRACT_BITS-1) - 1 = q18 */
    temp = temp + P281;  /* q18 + q18 = q18 */
    result = fDivNorm(x, temp, &res_e);
    result = scaleValue(result,
                        (Q_ATANOUT - Q_ATANINP + 18 - DFRACT_BITS + 1) + res_e);
    result = ONEP571 - result; /* q30 + q30 = q30 */
  }
  if (sign) {
    result = -result;
  }

  return (result);
}

#include "FDK_tools_rom.h"

FIXP_DBL fixp_cos(FIXP_DBL x, int scale) {
  FIXP_DBL residual, error, sine, cosine;

  residual = fixp_sin_cos_residual_inline(x, scale, &sine, &cosine);
  error = fMult(sine, residual);

#ifdef SINETABLE_16BIT
  return cosine - error;
#else
  /* Undo downscaling by 1 which was done at fixp_sin_cos_residual_inline */
  return SATURATE_LEFT_SHIFT(cosine - error, 1, DFRACT_BITS);
#endif
}

FIXP_DBL fixp_sin(FIXP_DBL x, int scale) {
  FIXP_DBL residual, error, sine, cosine;

  residual = fixp_sin_cos_residual_inline(x, scale, &sine, &cosine);
  error = fMult(cosine, residual);

#ifdef SINETABLE_16BIT
  return sine + error;
#else
  return SATURATE_LEFT_SHIFT(sine + error, 1, DFRACT_BITS);
#endif
}

void fixp_cos_sin(FIXP_DBL x, int scale, FIXP_DBL *cos, FIXP_DBL *sin) {
  FIXP_DBL residual, error0, error1, sine, cosine;

  residual = fixp_sin_cos_residual_inline(x, scale, &sine, &cosine);
  error0 = fMult(sine, residual);
  error1 = fMult(cosine, residual);

#ifdef SINETABLE_16BIT
  *cos = cosine - error0;
  *sin = sine + error1;
#else
  *cos = SATURATE_LEFT_SHIFT(cosine - error0, 1, DFRACT_BITS);
  *sin = SATURATE_LEFT_SHIFT(sine + error1, 1, DFRACT_BITS);
#endif
}
