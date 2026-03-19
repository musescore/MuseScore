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

#if !defined(CPLX_MUL_ARM_H)
#define CPLX_MUL_ARM_H

#if defined(__arm__) && defined(__GNUC__)

#if defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__) || \
    defined(__ARM_ARCH_8__)
#define FUNCTION_cplxMultDiv2_32x16
#define FUNCTION_cplxMultDiv2_32x16X2
#endif

#define FUNCTION_cplxMultDiv2_32x32X2
#ifdef FUNCTION_cplxMultDiv2_32x32X2
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, const FIXP_DBL b_Re,
                         const FIXP_DBL b_Im) {
  LONG tmp1, tmp2;

#ifdef __ARM_ARCH_8__
  asm("smull  %x0, %w2, %w4;       \n" /* tmp1  = a_Re * b_Re */
      "smull  %x1, %w2, %w5;       \n" /* tmp2  = a_Re * b_Im */
      "smsubl %x0, %w3, %w5, %x0;  \n" /* tmp1 -= a_Im * b_Im */
      "smaddl %x1, %w3, %w4, %x1;  \n" /* tmp2 += a_Im * b_Re */
      "asr    %x0, %x0,  #32       \n"
      "asr    %x1, %x1,  #32       \n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
#elif defined(__ARM_ARCH_6__)
  asm("smmul %0, %2, %4;\n"     /* tmp1  = a_Re * b_Re */
      "smmls %0, %3, %5, %0;\n" /* tmp1 -= a_Im * b_Im */
      "smmul %1, %2, %5;\n"     /* tmp2  = a_Re * b_Im */
      "smmla %1, %3, %4, %1;\n" /* tmp2 += a_Im * b_Re */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
#else
  LONG discard;
  asm("smull %2, %0, %7, %6;\n" /* tmp1  = -a_Im * b_Im */
      "smlal %2, %0, %3, %5;\n" /* tmp1 +=  a_Re * b_Re */
      "smull %2, %1, %3, %6;\n" /* tmp2  =  a_Re * b_Im */
      "smlal %2, %1, %4, %5;\n" /* tmp2 +=  a_Im * b_Re */
      : "=&r"(tmp1), "=&r"(tmp2), "=&r"(discard)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im), "r"(-a_Im));
#endif
  *c_Re = tmp1;
  *c_Im = tmp2;
}
#endif /* FUNCTION_cplxMultDiv2_32x32X2 */

#if defined(FUNCTION_cplxMultDiv2_32x16)
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, FIXP_SPK wpk) {
#ifdef __ARM_ARCH_8__
  FIXP_DBL b_Im = FX_SGL2FX_DBL(wpk.v.im);
  FIXP_DBL b_Re = FX_SGL2FX_DBL(wpk.v.re);
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, b_Re, b_Im);
#else
  LONG tmp1, tmp2;
  const LONG w = wpk.w;
  asm("smulwt %0, %3, %4;\n"
      "rsb %1,%0,#0;\n"
      "smlawb %0, %2, %4, %1;\n"
      "smulwt %1, %2, %4;\n"
      "smlawb %1, %3, %4, %1;\n"
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(w));
  *c_Re = tmp1;
  *c_Im = tmp2;
#endif
}
#endif /* FUNCTION_cplxMultDiv2_32x16 */

#ifdef FUNCTION_cplxMultDiv2_32x16X2
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, const FIXP_SGL b_Re,
                         const FIXP_SGL b_Im) {
#ifdef __ARM_ARCH_8__
  FIXP_DBL b_re = FX_SGL2FX_DBL(b_Re);
  FIXP_DBL b_im = FX_SGL2FX_DBL(b_Im);
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, b_re, b_im);
#else
  LONG tmp1, tmp2;

  asm("smulwb %0, %3, %5;\n" /* %7   = -a_Im * b_Im */
      "rsb %1,%0,#0;\n"
      "smlawb %0, %2, %4, %1;\n" /* tmp1 =  a_Re * b_Re - a_Im * b_Im */
      "smulwb %1, %2, %5;\n"     /* %7   =  a_Re * b_Im */
      "smlawb %1, %3, %4, %1;\n" /* tmp2 =  a_Im * b_Re + a_Re * b_Im */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));

  *c_Re = tmp1;
  *c_Im = tmp2;
#endif
}
#endif /* FUNCTION_cplxMultDiv2_32x16X2 */

#endif

#endif /* !defined(CPLX_MUL_ARM_H) */
