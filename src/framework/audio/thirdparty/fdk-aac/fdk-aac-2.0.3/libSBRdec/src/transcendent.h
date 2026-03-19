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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  FDK Fixed Point Arithmetic Library Interface
*/

#ifndef TRANSCENDENT_H
#define TRANSCENDENT_H

#include "sbrdecoder.h"
#include "sbr_rom.h"

/************************************************************************/
/*!
  \brief   Get number of octaves between frequencies a and b

  The Result is scaled with 1/8.
  The valid range for a and b is 1 to LOG_DUALIS_TABLE_SIZE.

  \return   ld(a/b) / 8
*/
/************************************************************************/
static inline FIXP_SGL FDK_getNumOctavesDiv8(INT a, /*!< lower band */
                                             INT b) /*!< upper band */
{
  return ((SHORT)((LONG)(CalcLdInt(b) - CalcLdInt(a)) >> (FRACT_BITS - 3)));
}

/************************************************************************/
/*!
  \brief   Add two values given by mantissa and exponent.

  Mantissas are in fract format with values between 0 and 1. <br>
  The base for exponents is 2.  Example:  \f$  a = a\_m * 2^{a\_e}  \f$<br>
*/
/************************************************************************/
inline void FDK_add_MantExp(FIXP_SGL a_m, /*!< Mantissa of 1st operand a */
                            SCHAR a_e,    /*!< Exponent of 1st operand a */
                            FIXP_SGL b_m, /*!< Mantissa of 2nd operand b */
                            SCHAR b_e,    /*!< Exponent of 2nd operand b */
                            FIXP_SGL *ptrSum_m, /*!< Mantissa of result */
                            SCHAR *ptrSum_e)    /*!< Exponent of result */
{
  FIXP_DBL accu;
  int shift;
  int shiftAbs;

  FIXP_DBL shiftedMantissa;
  FIXP_DBL otherMantissa;

  /* Equalize exponents of the summands.
     For the smaller summand, the exponent is adapted and
     for compensation, the mantissa is shifted right. */

  shift = (int)(a_e - b_e);

  shiftAbs = (shift > 0) ? shift : -shift;
  shiftAbs = (shiftAbs < DFRACT_BITS - 1) ? shiftAbs : DFRACT_BITS - 1;
  shiftedMantissa = (shift > 0) ? (FX_SGL2FX_DBL(b_m) >> shiftAbs)
                                : (FX_SGL2FX_DBL(a_m) >> shiftAbs);
  otherMantissa = (shift > 0) ? FX_SGL2FX_DBL(a_m) : FX_SGL2FX_DBL(b_m);
  *ptrSum_e = (shift > 0) ? a_e : b_e;

  accu = (shiftedMantissa >> 1) + (otherMantissa >> 1);
  /* shift by 1 bit to avoid overflow */

  if ((accu >= (FL2FXCONST_DBL(0.5f) - (FIXP_DBL)1)) ||
      (accu <= FL2FXCONST_DBL(-0.5f)))
    *ptrSum_e += 1;
  else
    accu = (shiftedMantissa + otherMantissa);

  *ptrSum_m = FX_DBL2FX_SGL(accu);
}

inline void FDK_add_MantExp(FIXP_DBL a,       /*!< Mantissa of 1st operand a */
                            SCHAR a_e,        /*!< Exponent of 1st operand a */
                            FIXP_DBL b,       /*!< Mantissa of 2nd operand b */
                            SCHAR b_e,        /*!< Exponent of 2nd operand b */
                            FIXP_DBL *ptrSum, /*!< Mantissa of result */
                            SCHAR *ptrSum_e)  /*!< Exponent of result */
{
  FIXP_DBL accu;
  int shift;
  int shiftAbs;

  FIXP_DBL shiftedMantissa;
  FIXP_DBL otherMantissa;

  /* Equalize exponents of the summands.
     For the smaller summand, the exponent is adapted and
     for compensation, the mantissa is shifted right. */

  shift = (int)(a_e - b_e);

  shiftAbs = (shift > 0) ? shift : -shift;
  shiftAbs = (shiftAbs < DFRACT_BITS - 1) ? shiftAbs : DFRACT_BITS - 1;
  shiftedMantissa = (shift > 0) ? (b >> shiftAbs) : (a >> shiftAbs);
  otherMantissa = (shift > 0) ? a : b;
  *ptrSum_e = (shift > 0) ? a_e : b_e;

  accu = (shiftedMantissa >> 1) + (otherMantissa >> 1);
  /* shift by 1 bit to avoid overflow */

  if ((accu >= (FL2FXCONST_DBL(0.5f) - (FIXP_DBL)1)) ||
      (accu <= FL2FXCONST_DBL(-0.5f)))
    *ptrSum_e += 1;
  else
    accu = (shiftedMantissa + otherMantissa);

  *ptrSum = accu;
}

/************************************************************************/
/*!
  \brief   Divide two values given by mantissa and exponent.

  Mantissas are in fract format with values between 0 and 1. <br>
  The base for exponents is 2.  Example:  \f$  a = a\_m * 2^{a\_e}  \f$<br>

  For performance reasons, the division is based on a table lookup
  which limits accuracy.
*/
/************************************************************************/
static inline void FDK_divide_MantExp(
    FIXP_SGL a_m,          /*!< Mantissa of dividend a */
    SCHAR a_e,             /*!< Exponent of dividend a */
    FIXP_SGL b_m,          /*!< Mantissa of divisor b */
    SCHAR b_e,             /*!< Exponent of divisor b */
    FIXP_SGL *ptrResult_m, /*!< Mantissa of quotient a/b */
    SCHAR *ptrResult_e)    /*!< Exponent of quotient a/b */

{
  int preShift, postShift, index, shift;
  FIXP_DBL ratio_m;
  FIXP_SGL bInv_m = FL2FXCONST_SGL(0.0f);

  preShift = CntLeadingZeros(FX_SGL2FX_DBL(b_m));

  /*
    Shift b into the range from 0..INV_TABLE_SIZE-1,

    E.g. 10 bits must be skipped for INV_TABLE_BITS 8:
    - leave 8 bits as index for table
    - skip sign bit,
    - skip first bit of mantissa, because this is always the same (>0.5)

    We are dealing with energies, so we need not care
    about negative numbers
  */

  /*
    The first interval has half width so the lowest bit of the index is
    needed for a doubled resolution.
  */
  shift = (FRACT_BITS - 2 - INV_TABLE_BITS - preShift);

  index = (shift < 0) ? (LONG)b_m << (-shift) : (LONG)b_m >> shift;

  /* The index has INV_TABLE_BITS +1 valid bits here. Clear the other bits. */
  index &= (1 << (INV_TABLE_BITS + 1)) - 1;

  /* Remove offset of half an interval */
  index--;

  /* Now the lowest bit is shifted out */
  index = index >> 1;

  /* Fetch inversed mantissa from table: */
  bInv_m = (index < 0) ? bInv_m : FDK_sbrDecoder_invTable[index];

  /* Multiply a with the inverse of b: */
  ratio_m = (index < 0) ? FX_SGL2FX_DBL(a_m >> 1) : fMultDiv2(bInv_m, a_m);

  postShift = CntLeadingZeros(ratio_m) - 1;

  *ptrResult_m = FX_DBL2FX_SGL(ratio_m << postShift);
  *ptrResult_e = a_e - b_e + 1 + preShift - postShift;
}

static inline void FDK_divide_MantExp(
    FIXP_DBL a_m,          /*!< Mantissa of dividend a */
    SCHAR a_e,             /*!< Exponent of dividend a */
    FIXP_DBL b_m,          /*!< Mantissa of divisor b */
    SCHAR b_e,             /*!< Exponent of divisor b */
    FIXP_DBL *ptrResult_m, /*!< Mantissa of quotient a/b */
    SCHAR *ptrResult_e)    /*!< Exponent of quotient a/b */

{
  int preShift, postShift, index, shift;
  FIXP_DBL ratio_m;
  FIXP_SGL bInv_m = FL2FXCONST_SGL(0.0f);

  preShift = CntLeadingZeros(b_m);

  /*
    Shift b into the range from 0..INV_TABLE_SIZE-1,

    E.g. 10 bits must be skipped for INV_TABLE_BITS 8:
    - leave 8 bits as index for table
    - skip sign bit,
    - skip first bit of mantissa, because this is always the same (>0.5)

    We are dealing with energies, so we need not care
    about negative numbers
  */

  /*
    The first interval has half width so the lowest bit of the index is
    needed for a doubled resolution.
  */
  shift = (DFRACT_BITS - 2 - INV_TABLE_BITS - preShift);

  index = (shift < 0) ? (LONG)b_m << (-shift) : (LONG)b_m >> shift;

  /* The index has INV_TABLE_BITS +1 valid bits here. Clear the other bits. */
  index &= (1 << (INV_TABLE_BITS + 1)) - 1;

  /* Remove offset of half an interval */
  index--;

  /* Now the lowest bit is shifted out */
  index = index >> 1;

  /* Fetch inversed mantissa from table: */
  bInv_m = (index < 0) ? bInv_m : FDK_sbrDecoder_invTable[index];

  /* Multiply a with the inverse of b: */
  ratio_m = (index < 0) ? (a_m >> 1) : fMultDiv2(bInv_m, a_m);

  postShift = CntLeadingZeros(ratio_m) - 1;

  *ptrResult_m = ratio_m << postShift;
  *ptrResult_e = a_e - b_e + 1 + preShift - postShift;
}

/*!
  \brief   Calculate the squareroot of a number given by mantissa and exponent

  Mantissa is in fract format with values between 0 and 1. <br>
  The base for the exponent is 2.  Example:  \f$  a = a\_m * 2^{a\_e}  \f$<br>
  The operand is addressed via pointers and will be overwritten with the result.

  For performance reasons, the square root is based on a table lookup
  which limits accuracy.
*/
static inline void FDK_sqrt_MantExp(
    FIXP_DBL *mantissa, /*!< Pointer to mantissa */
    SCHAR *exponent, const SCHAR *destScale) {
  FIXP_DBL input_m = *mantissa;
  int input_e = (int)*exponent;
  FIXP_DBL result = FL2FXCONST_DBL(0.0f);
  int result_e = -FRACT_BITS;

  /* Call lookup square root, which does internally normalization. */
  result = sqrtFixp_lookup(input_m, &input_e);
  result_e = input_e;

  /* Write result */
  if (exponent == destScale) {
    *mantissa = result;
    *exponent = result_e;
  } else {
    int shift = result_e - *destScale;
    *mantissa = (shift >= 0) ? result << (INT)fixMin(DFRACT_BITS - 1, shift)
                             : result >> (INT)fixMin(DFRACT_BITS - 1, -shift);
    *exponent = *destScale;
  }
}

#endif
