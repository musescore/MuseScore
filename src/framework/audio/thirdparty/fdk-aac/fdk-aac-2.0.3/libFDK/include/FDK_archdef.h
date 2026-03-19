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

   Description:

*******************************************************************************/

#ifndef FDK_ARCHDEF_H
#define FDK_ARCHDEF_H

/* Unify some few toolchain specific defines to avoid having large "or" macro
 * contraptions all over the source code. */

/* Use single macro (the GCC built in macro) for architecture identification
 * independent of the particular toolchain */
#if defined(__i386__) || defined(__i486__) || defined(__i586__) ||  \
    defined(__i686__) || (defined(_MSC_VER) && defined(_M_IX86)) || \
    (defined(_MSC_VER) && defined(_M_X64)) || defined(__x86_64__)
#define __x86__
#endif

#if defined(_M_ARM) && !defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
#define __arm__
#endif

#if defined(_ARCH_PPC) && !defined(__powerpc__)
#define __powerpc__ 1
#endif

#if (__TARGET_ARCH_ARM == 5) || defined(__TARGET_FEATURE_DSPMUL) || \
    (_M_ARM == 5) || defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_7EM__)
/* Define __ARM_ARCH_5TE__ if armv5te features are supported  */
#define __ARM_ARCH_5TE__
#endif

#if (__TARGET_ARCH_ARM == 6) || defined(__ARM_ARCH_6J__) || \
    defined(__ARM_ARCH_6ZK__)
/* Define __ARM_ARCH_6__ if the armv6 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#endif

#if defined(__TARGET_ARCH_7_R) || defined(__ARM_ARCH_7R__)
/* Define __ARM_ARCH_7_A__ if the armv7 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#define __ARM_ARCH_7_R__
#endif

#if defined(__TARGET_ARCH_7_A) || defined(__ARM_ARCH_7A__) || \
    ((__ARM_ARCH == 8) && (__ARM_32BIT_STATE == 1))
/* Define __ARM_ARCH_7_A__ if the armv7 intructions are being supported. */
#define __ARM_ARCH_5TE__
#define __ARM_ARCH_6__
#define __ARM_ARCH_7_A__
#endif

#if defined(__TARGET_ARCH_7_M) || defined(__ARM_ARCH_7_M__)
/* Define __ARM_ARCH_7M__ if the ARMv7-M instructions are being supported, e.g.
 * Cortex-M3. */
#define __ARM_ARCH_7M__
#endif

#if defined(__TARGET_ARCH_7E_M) || defined(__ARM_ARCH_7E_M__)
/* Define __ARM_ARCH_7EM__ if the ARMv7-ME instructions are being supported,
 * e.g. Cortex-M4. */
#define __ARM_ARCH_7EM__
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define __ARM_ARCH_8__
#endif

#ifdef _M_ARM
#include "armintr.h"
#endif

/* Define preferred Multiplication type */

#if defined(__mips__)
#define ARCH_PREFER_MULT_16x16
#undef SINETABLE_16BIT
#undef POW2COEFF_16BIT
#undef LDCOEFF_16BIT
#undef WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_8__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_5TE__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_7M__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && defined(__ARM_ARCH_7EM__)
#define ARCH_PREFER_MULT_32x32
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__arm__) && !defined(__ARM_ARCH_5TE__)
#define ARCH_PREFER_MULT_16x16
#undef SINETABLE_16BIT
#undef WINDOWTABLE_16BIT
#undef POW2COEFF_16BIT
#undef LDCOEFF_16BIT

#elif defined(__x86__)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define WINDOWTABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT

#elif defined(__riscv)
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#elif defined(__powerpc__)
#define ARCH_PREFER_MULT_32x32

#elif defined(__s390x__)
#define ARCH_PREFER_MULT_32x32
#define ARCH_PREFER_MULT_32x16
#define SINETABLE_16BIT
#define POW2COEFF_16BIT
#define LDCOEFF_16BIT
#define WINDOWTABLE_16BIT

#else
#warning >>>> Please set architecture characterization defines for your platform (FDK_HIGH_PERFORMANCE)! <<<<

#endif /* Architecture switches */

#ifdef SINETABLE_16BIT
#define FIXP_STB FIXP_SGL /* STB sinus Tab used in transformation */
#define FIXP_STP FIXP_SPK
#define STC(a) (FX_DBL2FXCONST_SGL(a))
#else
#define FIXP_STB FIXP_DBL
#define FIXP_STP FIXP_DPK
#define STC(a) ((FIXP_DBL)(LONG)(a))
#endif /* defined(SINETABLE_16BIT) */

#define STCP(cos, sin)     \
  {                        \
    { STC(cos), STC(sin) } \
  }

#ifdef WINDOWTABLE_16BIT
#define FIXP_WTB FIXP_SGL /* single FIXP_SGL values */
#define FX_DBL2FX_WTB(x) FX_DBL2FX_SGL(x)
#define FIXP_WTP FIXP_SPK /* packed FIXP_SGL values */
#define WTC(a) FX_DBL2FXCONST_SGL(a)
#else /* SINETABLE_16BIT */
#define FIXP_WTB FIXP_DBL
#define FX_DBL2FX_WTB(x) (x)
#define FIXP_WTP FIXP_DPK
#define WTC(a) (FIXP_DBL)(a)
#endif /* SINETABLE_16BIT */

#define WTCP(a, b)     \
  {                    \
    { WTC(a), WTC(b) } \
  }

#endif /* FDK_ARCHDEF_H */
