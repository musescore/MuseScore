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

/******************* Library for basic calculation routines ********************

   Author(s):

   Description: Scaling operations

*******************************************************************************/

#ifndef SCALE_H
#define SCALE_H

#include "common_fix.h"
#include "genericStds.h"
#include "fixminmax.h"

#define SCALE_INLINE

#if defined(__arm__)
#include "arm/scale_arm.h"

#elif defined(__mips__)
#include "mips/scale_mips.h"

#endif

void scaleValues(FIXP_SGL *vector, INT len, INT scalefactor);
void scaleValues(FIXP_DBL *vector, INT len, INT scalefactor);
void scaleValues(FIXP_DBL *dst, const FIXP_DBL *src, INT len, INT scalefactor);
#if (SAMPLE_BITS == 16)
void scaleValues(FIXP_PCM *dst, const FIXP_DBL *src, INT len, INT scalefactor);
#endif
void scaleValues(FIXP_SGL *dst, const FIXP_SGL *src, INT len, INT scalefactor);
void scaleCplxValues(FIXP_DBL *r_dst, FIXP_DBL *i_dst, const FIXP_DBL *r_src,
                     const FIXP_DBL *i_src, INT len, INT scalefactor);
void scaleValuesWithFactor(FIXP_DBL *vector, FIXP_DBL factor, INT len,
                           INT scalefactor);
void scaleValuesSaturate(FIXP_DBL *vector, INT len, INT scalefactor);
void scaleValuesSaturate(FIXP_DBL *dst, const FIXP_DBL *src, INT len,
                         INT scalefactor);
void scaleValuesSaturate(FIXP_SGL *dst, const FIXP_DBL *src, INT len,
                         INT scalefactor);
void scaleValuesSaturate(FIXP_SGL *vector, INT len, INT scalefactor);
void scaleValuesSaturate(FIXP_SGL *dst, const FIXP_SGL *src, INT len,
                         INT scalefactor);
INT getScalefactorShort(const SHORT *vector, INT len);
INT getScalefactorPCM(const INT_PCM *vector, INT len, INT stride);
INT getScalefactor(const FIXP_DBL *vector, INT len);
INT getScalefactor(const FIXP_SGL *vector, INT len);

#ifndef FUNCTION_scaleValue
/*!
 *
 *  \brief Multiply input by \f$ 2^{scalefactor} \f$
 *
 *  \return Scaled input
 *
 */
#define FUNCTION_scaleValue
inline FIXP_DBL scaleValue(const FIXP_DBL value, /*!< Value */
                           INT scalefactor       /*!< Scalefactor */
) {
  if (scalefactor > 0)
    return (value << scalefactor);
  else
    return (value >> (-scalefactor));
}
inline FIXP_SGL scaleValue(const FIXP_SGL value, /*!< Value */
                           INT scalefactor       /*!< Scalefactor */
) {
  if (scalefactor > 0)
    return (value << scalefactor);
  else
    return (value >> (-scalefactor));
}
#endif

#ifndef FUNCTION_scaleValueSaturate
/*!
 *
 *  \brief Multiply input by \f$ 2^{scalefactor} \f$
 *  \param value The value to be scaled.
 *  \param the shift amount
 *  \return \f$ value * 2^scalefactor \f$
 *
 */
#define FUNCTION_scaleValueSaturate
inline FIXP_DBL scaleValueSaturate(const FIXP_DBL value,
                                   INT scalefactor /* in range -31 ... +31 */
) {
  int headroom = fixnormz_D(
      (INT)value ^ (INT)((value >> 31))); /* headroom in range 1...32 */
  if (scalefactor >= 0) {
    /* shift left: saturate in case of headroom less/equal scalefactor */
    if (headroom <= scalefactor) {
      if (value > (FIXP_DBL)0)
        return (FIXP_DBL)MAXVAL_DBL; /* 0x7FFF.FFFF */
      else
        return (FIXP_DBL)MINVAL_DBL + (FIXP_DBL)1; /* 0x8000.0001 */
    } else {
      return fMax((value << scalefactor), (FIXP_DBL)MINVAL_DBL + (FIXP_DBL)1);
    }
  } else {
    scalefactor = -scalefactor;
    /* shift right: clear in case of 32-headroom greater/equal -scalefactor */
    if ((DFRACT_BITS - headroom) <= scalefactor) {
      return (FIXP_DBL)0;
    } else {
      return fMax((value >> scalefactor), (FIXP_DBL)MINVAL_DBL + (FIXP_DBL)1);
    }
  }
}
#endif

#ifndef FUNCTION_scaleValueInPlace
/*!
 *
 *  \brief Multiply input by \f$ 2^{scalefactor} \f$ in place
 *
 *  \return void
 *
 */
#define FUNCTION_scaleValueInPlace
inline void scaleValueInPlace(FIXP_DBL *value, /*!< Value */
                              INT scalefactor  /*!< Scalefactor */
) {
  INT newscale;
  /* Note: The assignment inside the if conditional allows combining a load with
   * the compare to zero (on ARM and maybe others) */
  if ((newscale = (scalefactor)) >= 0) {
    *(value) <<= newscale;
  } else {
    *(value) >>= -newscale;
  }
}
#endif

  /*!
   *
   *  \brief  Scale input value by 2^{scale} and saturate output to 2^{dBits-1}
   *  \return scaled and saturated value
   *
   *  This macro scales src value right or left and applies saturation to
   * (2^dBits)-1 maxima output.
   */

#ifndef SATURATE_RIGHT_SHIFT
#define SATURATE_RIGHT_SHIFT(src, scale, dBits)                            \
  ((((LONG)(src) >> (scale)) > (LONG)(((1U) << ((dBits)-1)) - 1))          \
       ? (LONG)(((1U) << ((dBits)-1)) - 1)                                 \
       : (((LONG)(src) >> (scale)) < ~((LONG)(((1U) << ((dBits)-1)) - 1))) \
             ? ~((LONG)(((1U) << ((dBits)-1)) - 1))                        \
             : ((LONG)(src) >> (scale)))
#endif

#ifndef SATURATE_LEFT_SHIFT
#define SATURATE_LEFT_SHIFT(src, scale, dBits)                           \
  (((LONG)(src) > ((LONG)(((1U) << ((dBits)-1)) - 1) >> (scale)))        \
       ? (LONG)(((1U) << ((dBits)-1)) - 1)                               \
       : ((LONG)(src) < ~((LONG)(((1U) << ((dBits)-1)) - 1) >> (scale))) \
             ? ~((LONG)(((1U) << ((dBits)-1)) - 1))                      \
             : ((LONG)(src) << (scale)))
#endif

#ifndef SATURATE_SHIFT
#define SATURATE_SHIFT(src, scale, dBits)                        \
  (((scale) < 0) ? SATURATE_LEFT_SHIFT((src), -(scale), (dBits)) \
                 : SATURATE_RIGHT_SHIFT((src), (scale), (dBits)))
#endif

/*
 * Alternative shift and saturate left, saturates to -0.99999 instead of -1.0000
 * to avoid problems when inverting the sign of the result.
 */
#ifndef SATURATE_LEFT_SHIFT_ALT
#define SATURATE_LEFT_SHIFT_ALT(src, scale, dBits)                        \
  (((LONG)(src) > ((LONG)(((1U) << ((dBits)-1)) - 1) >> (scale)))         \
       ? (LONG)(((1U) << ((dBits)-1)) - 1)                                \
       : ((LONG)(src) <= ~((LONG)(((1U) << ((dBits)-1)) - 1) >> (scale))) \
             ? ~((LONG)(((1U) << ((dBits)-1)) - 2))                       \
             : ((LONG)(src) << (scale)))
#endif

#ifndef SATURATE_RIGHT_SHIFT_ALT
#define SATURATE_RIGHT_SHIFT_ALT(src, scale, dBits)                        \
  ((((LONG)(src) >> (scale)) > (LONG)(((1U) << ((dBits)-1)) - 1))          \
       ? (LONG)(((1U) << ((dBits)-1)) - 1)                                 \
       : (((LONG)(src) >> (scale)) < ~((LONG)(((1U) << ((dBits)-1)) - 2))) \
             ? ~((LONG)(((1U) << ((dBits)-1)) - 2))                        \
             : ((LONG)(src) >> (scale)))
#endif

#ifndef SATURATE_INT_PCM_RIGHT_SHIFT
#define SATURATE_INT_PCM_RIGHT_SHIFT(src, scale) \
  SATURATE_RIGHT_SHIFT(src, scale, SAMPLE_BITS)
#endif

#ifndef SATURATE_INT_PCM_LEFT_SHIFT
#define SATURATE_INT_PCM_LEFT_SHIFT(src, scale) \
  SATURATE_LEFT_SHIFT(src, scale, SAMPLE_BITS)
#endif

#endif /* #ifndef SCALE_H */
