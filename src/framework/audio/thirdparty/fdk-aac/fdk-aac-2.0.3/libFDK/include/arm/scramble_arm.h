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

   Description: bitreversal of input data

*******************************************************************************/

#if !defined(SCRAMBLE_ARM_H)
#define SCRAMBLE_ARM_H

#if defined(FUNCTION_scramble)
#if defined(__GNUC__)

#define FUNCTION_scramble

#if defined(__ARM_ARCH_5TE__)
#define USE_LDRD_STRD /* LDRD requires 8 byte data alignment. */
#endif

inline void scramble(FIXP_DBL x[], INT n) {
  FDK_ASSERT(!(((INT)x) & (ALIGNMENT_DEFAULT - 1)));
  asm("mov     r2, #1;\n"     /* r2(m) = 1;           */
      "sub     r3, %1, #1;\n" /* r3 = n-1;            */
      "mov     r4, #0;\n"     /* r4(j) = 0;           */

      "scramble_m_loop%=:\n" /* {                    */
      "mov     r5, %1;\n"    /*  r5(k) = 1;          */

      "scramble_k_loop%=:\n"         /*  {                   */
      "mov     r5, r5, lsr #1;\n"    /*   k >>= 1;           */
      "eor     r4, r4, r5;\n"        /*   j ^=k;             */
      "ands    r10, r4, r5;\n"       /*   r10 = r4 & r5;      */
      "beq     scramble_k_loop%=;\n" /*  } while (r10 == 0);  */

      "cmp     r4, r2;\n" /*   if (r4 < r2) break;        */
      "bcc     scramble_m_loop_end%=;\n"

#ifdef USE_LDRD_STRD
      "mov     r5, r2, lsl #3;\n" /* m(r5) = r2*4*2               */
      "ldrd    r10, [%0, r5];\n"  /* r10 = x[r5], x7 = x[r5+1]     */
      "mov     r6, r4, lsl #3;\n" /* j(r6) = r4*4*2              */
      "ldrd    r8, [%0, r6];\n"   /* r8 = x[r6], r9 = x[r6+1];  */
      "strd    r10, [%0, r6];\n"  /* x[r6,r6+1] = r10,r11;        */
      "strd    r8, [%0, r5];\n"   /* x[r5,r5+1] = r8,r9;          */
#else
      "mov      r5, r2, lsl #3;\n" /* m(r5) = r2*4*2               */
      "ldr      r10, [%0, r5];\n"
      "mov      r6, r4, lsl #3;\n" /* j(r6) = r4*4*2              */
      "ldr      r11, [%0, r6];\n"

      "str      r10, [%0, r6];\n"
      "str      r11, [%0, r5];\n"

      "add      r5, r5, #4;"
      "ldr      r10, [%0, r5];\n"
      "add      r6, r6, #4;"
      "ldr      r11, [%0, r6];\n"
      "str      r10, [%0, r6];\n"
      "str      r11, [%0, r5];\n"
#endif
      "scramble_m_loop_end%=:\n"
      "add     r2, r2, #1;\n" /* r2++;                        */
      "cmp     r2, r3;\n"
      "bcc     scramble_m_loop%=;\n" /* } while (r2(m) < r3(n-1));   */
      :
      : "r"(x), "r"(n)
#ifdef USE_LDRD_STRD
      : "r2", "r3", "r4", "r5", "r10", "r11", "r8", "r9", "r6");
#else
      : "r2", "r3", "r4", "r5", "r10", "r11", "r6");
#endif
}
#else
/* Force C implementation if no assembler version available. */
#undef FUNCTION_scramble
#endif /* Toolchain selection. */

#endif /* defined(FUNCTION_scramble) */
#endif /* !defined(SCRAMBLE_ARM_H) */
