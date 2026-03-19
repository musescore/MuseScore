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

   Author(s):

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(FIXMUL_ARM_H)
#define FIXMUL_ARM_H

#if defined(__arm__)

#if defined(__GNUC__) && defined(__arm__)
/* ARM with GNU compiler */

#define FUNCTION_fixmuldiv2_DD

#define FUNCTION_fixmuldiv2BitExact_DD
#ifdef FUNCTION_fixmuldiv2BitExact_DD
#define fixmuldiv2BitExact_DD(a, b) fixmuldiv2_DD(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DD */

#define FUNCTION_fixmulBitExact_DD
#ifdef FUNCTION_fixmulBitExact_DD
#define fixmulBitExact_DD(a, b) (fixmuldiv2BitExact_DD(a, b) << 1)
#endif /* #ifdef FUNCTION_fixmulBitExact_DD */

#define FUNCTION_fixmuldiv2BitExact_DS
#ifdef FUNCTION_fixmuldiv2BitExact_DS
#define fixmuldiv2BitExact_DS(a, b) fixmuldiv2_DS(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DS */

#define FUNCTION_fixmulBitExact_DS
#ifdef FUNCTION_fixmulBitExact_DS
#define fixmulBitExact_DS(a, b) fixmul_DS(a, b)
#endif /* #ifdef FUNCTION_fixmulBitExact_DS */

#ifdef FUNCTION_fixmuldiv2_DD
inline INT fixmuldiv2_DD(const INT a, const INT b) {
  INT result;
#if defined(__ARM_ARCH_8__)
  INT64 result64;
  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #32;     "
      : "=r"(result64)
      : "r"(a), "r"(b));
  result = (INT)result64;
#elif defined(__ARM_ARCH_6__) || defined(__TARGET_ARCH_7E_M)
  __asm__("smmul %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
#else
  INT discard;
  __asm__("smull %0, %1, %2, %3"
          : "=&r"(discard), "=r"(result)
          : "r"(a), "r"(b));
#endif
  return result;
}
#endif /* #ifdef FUNCTION_fixmuldiv2_DD */

#if defined(__ARM_ARCH_8__)
#define FUNCTION_fixmuldiv2_SD
#ifdef FUNCTION_fixmuldiv2_SD
inline INT fixmuldiv2_SD(const SHORT a, const INT b) {
  return fixmuldiv2_DD((INT)(a << 16), b);
}
#endif /* #ifdef FUNCTION_fixmuldiv2_SD */
#elif defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__)
#define FUNCTION_fixmuldiv2_SD
#ifdef FUNCTION_fixmuldiv2_SD
inline INT fixmuldiv2_SD(const SHORT a, const INT b) {
  INT result;
  __asm__("smulwb %0, %1, %2" : "=r"(result) : "r"(b), "r"(a));
  return result;
}
#endif /* #ifdef FUNCTION_fixmuldiv2_SD */
#endif

#define FUNCTION_fixmul_DD
#ifdef FUNCTION_fixmul_DD
#if defined(__ARM_ARCH_8__)
inline INT fixmul_DD(const INT a, const INT b) {
  INT64 result64;

  __asm__(
      "smull %x0, %w1, %w2;\n"
      "asr %x0, %x0, #31;     "
      : "=r"(result64)
      : "r"(a), "r"(b));
  return (INT)result64;
}
#else
inline INT fixmul_DD(const INT a, const INT b) {
  return (fixmuldiv2_DD(a, b) << 1);
}
#endif /* __ARM_ARCH_8__ */
#endif /* #ifdef FUNCTION_fixmul_DD */

#endif /* defined(__GNUC__) && defined(__arm__) */

#endif /* __arm__ */

#endif /* !defined(FIXMUL_ARM_H) */
