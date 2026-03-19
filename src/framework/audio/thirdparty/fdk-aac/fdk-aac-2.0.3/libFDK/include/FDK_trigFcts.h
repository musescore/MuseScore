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

#if !defined(FDK_TRIGFCTS_H)
#define FDK_TRIGFCTS_H

#include "common_fix.h"

#include "FDK_tools_rom.h"

/* Fixed point precision definitions */
#define Q(format) ((FIXP_DBL)(((LONG)1) << (format)))

#ifndef M_PI
#define M_PI (3.14159265358979323846f)
#endif

/*!
 * Inverse tangent function.
 */

/* --- fixp_atan() ----    */
#define Q_ATANINP (25)  // Input in q25, Output in q30
#define Q_ATANOUT (30)
#define ATI_SF ((DFRACT_BITS - 1) - Q_ATANINP) /* 6  */
#define ATI_SCALE ((float)(1 << ATI_SF))
#define ATO_SF ((DFRACT_BITS - 1) - Q_ATANOUT) /* 1   ] -pi/2 .. pi/2 [ */
#define ATO_SCALE ((float)(1 << ATO_SF))
/* --- fixp_atan2() ---    */
#define Q_ATAN2OUT (29)
#define AT2O_SF ((DFRACT_BITS - 1) - Q_ATAN2OUT) /* 2   ] -pi   .. pi   ] */
#define AT2O_SCALE ((float)(1 << AT2O_SF))
// --------------------

FIXP_DBL fixp_atan(FIXP_DBL x);
FIXP_DBL fixp_atan2(FIXP_DBL y, FIXP_DBL x);

FIXP_DBL fixp_cos(FIXP_DBL x, int scale);
FIXP_DBL fixp_sin(FIXP_DBL x, int scale);

#define FIXP_COS_SIN

#include "FDK_tools_rom.h"

#define SINETAB SineTable512
#define LD 9

#ifndef FUNCTION_inline_fixp_cos_sin

#define FUNCTION_inline_fixp_cos_sin

/*
 * Calculates coarse lookup index and sign for sine.
 * Returns delta x residual.
 */
static inline FIXP_DBL fixp_sin_cos_residual_inline(FIXP_DBL x, int scale,
                                                    FIXP_DBL *sine,
                                                    FIXP_DBL *cosine) {
  FIXP_DBL residual;
  int s;
  int shift = (31 - scale - LD - 1);
  int ssign = 1;
  int csign = 1;

  residual = fMult(x, FL2FXCONST_DBL(1.0 / M_PI));
  s = ((LONG)residual) >> shift;

  residual &= ((1 << shift) - 1);
  residual = fMult(residual, FL2FXCONST_DBL(M_PI / 4.0)) << 2;
  residual <<= scale;

  /* Sine sign symmetry */
  if (s & ((1 << LD) << 1)) {
    ssign = -ssign;
  }
  /* Cosine sign symmetry */
  if ((s + (1 << LD)) & ((1 << LD) << 1)) {
    csign = -csign;
  }

  s = fAbs(s);

  s &= (((1 << LD) << 1) - 1); /* Modulo PI */

  if (s > (1 << LD)) {
    s = ((1 << LD) << 1) - s;
  }

  {
    LONG sl, cl;
    /* Because of packed table */
    if (s > (1 << (LD - 1))) {
      FIXP_STP tmp;
      /* Cosine/Sine simetry for angles greater than PI/4 */
      s = (1 << LD) - s;
      tmp = SINETAB[s];
      sl = (LONG)tmp.v.re;
      cl = (LONG)tmp.v.im;
    } else {
      FIXP_STP tmp;
      tmp = SINETAB[s];
      sl = (LONG)tmp.v.im;
      cl = (LONG)tmp.v.re;
    }

#ifdef SINETABLE_16BIT
    *sine = (FIXP_DBL)((sl * ssign) << (DFRACT_BITS - FRACT_BITS));
    *cosine = (FIXP_DBL)((cl * csign) << (DFRACT_BITS - FRACT_BITS));
#else
    /* scale down by 1 for overflow prevention. This is undone at the calling
     * function. */
    *sine = (FIXP_DBL)(sl * ssign) >> 1;
    *cosine = (FIXP_DBL)(cl * csign) >> 1;
#endif
  }

  return residual;
}

/**
 * \brief Calculate cosine and sine value each of 2 angles different angle
 * values.
 * \param x1 first angle value
 * \param x2 second angle value
 * \param scale exponent of x1 and x2
 * \param out pointer to 4 FIXP_DBL locations, were the values cos(x1), sin(x1),
 * cos(x2), sin(x2) will be stored into.
 */
static inline void inline_fixp_cos_sin(FIXP_DBL x1, FIXP_DBL x2,
                                       const int scale, FIXP_DBL *out) {
  FIXP_DBL residual, error0, error1, sine, cosine;
  residual = fixp_sin_cos_residual_inline(x1, scale, &sine, &cosine);
  error0 = fMultDiv2(sine, residual);
  error1 = fMultDiv2(cosine, residual);

#ifdef SINETABLE_16BIT
  *out++ = cosine - (error0 << 1);
  *out++ = sine + (error1 << 1);
#else
  /* Undo downscaling by 1 which was done at fixp_sin_cos_residual_inline */
  *out++ = SATURATE_LEFT_SHIFT(cosine - (error0 << 1), 1, DFRACT_BITS);
  *out++ = SATURATE_LEFT_SHIFT(sine + (error1 << 1), 1, DFRACT_BITS);
#endif

  residual = fixp_sin_cos_residual_inline(x2, scale, &sine, &cosine);
  error0 = fMultDiv2(sine, residual);
  error1 = fMultDiv2(cosine, residual);

#ifdef SINETABLE_16BIT
  *out++ = cosine - (error0 << 1);
  *out++ = sine + (error1 << 1);
#else
  *out++ = SATURATE_LEFT_SHIFT(cosine - (error0 << 1), 1, DFRACT_BITS);
  *out++ = SATURATE_LEFT_SHIFT(sine + (error1 << 1), 1, DFRACT_BITS);
#endif
}
#endif

#endif /* !defined(FDK_TRIGFCTS_H) */
