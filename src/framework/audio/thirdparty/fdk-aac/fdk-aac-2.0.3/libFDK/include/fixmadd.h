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

   Author(s):   M. Lohwasser, M. Gayer

   Description: fixed point intrinsics

*******************************************************************************/

#if !defined(FIXMADD_H)
#define FIXMADD_H

#include "FDK_archdef.h"
#include "machine_type.h"
#include "fixmul.h"

#if defined(__arm__)
#include "arm/fixmadd_arm.h"

#endif /* all cores */

/*************************************************************************
 *************************************************************************
    Software fallbacks for missing functions.
**************************************************************************
**************************************************************************/

/* Divide by two versions. */

#if !defined(FUNCTION_fixmadddiv2_DD)
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return (x + fMultDiv2(a, b));
}
#endif

#if !defined(FUNCTION_fixmadddiv2_SD)
inline FIXP_DBL fixmadddiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmadddiv2_DS
  return fixmadddiv2_DS(x, b, a);
#else
  return fixmadddiv2_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif

#if !defined(FUNCTION_fixmadddiv2_DS)
inline FIXP_DBL fixmadddiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmadddiv2_SD
  return fixmadddiv2_SD(x, b, a);
#else
  return fixmadddiv2_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif

#if !defined(FUNCTION_fixmadddiv2_SS)
inline FIXP_DBL fixmadddiv2_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return x + fMultDiv2(a, b);
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_DD)
inline FIXP_DBL fixmsubdiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return (x - fMultDiv2(a, b));
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_SD)
inline FIXP_DBL fixmsubdiv2_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmsubdiv2_DS
  return fixmsubdiv2_DS(x, b, a);
#else
  return fixmsubdiv2_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_DS)
inline FIXP_DBL fixmsubdiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmsubdiv2_SD
  return fixmsubdiv2_SD(x, b, a);
#else
  return fixmsubdiv2_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2_SS)
inline FIXP_DBL fixmsubdiv2_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return x - fMultDiv2(a, b);
}
#endif

#if !defined(FUNCTION_fixmadddiv2BitExact_DD)
#define FUNCTION_fixmadddiv2BitExact_DD
inline FIXP_DBL fixmadddiv2BitExact_DD(FIXP_DBL x, const FIXP_DBL a,
                                       const FIXP_DBL b) {
  return x + fMultDiv2BitExact(a, b);
}
#endif
#if !defined(FUNCTION_fixmadddiv2BitExact_SD)
#define FUNCTION_fixmadddiv2BitExact_SD
inline FIXP_DBL fixmadddiv2BitExact_SD(FIXP_DBL x, const FIXP_SGL a,
                                       const FIXP_DBL b) {
#ifdef FUNCTION_fixmadddiv2BitExact_DS
  return fixmadddiv2BitExact_DS(x, b, a);
#else
  return x + fMultDiv2BitExact(a, b);
#endif
}
#endif
#if !defined(FUNCTION_fixmadddiv2BitExact_DS)
#define FUNCTION_fixmadddiv2BitExact_DS
inline FIXP_DBL fixmadddiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a,
                                       const FIXP_SGL b) {
#ifdef FUNCTION_fixmadddiv2BitExact_SD
  return fixmadddiv2BitExact_SD(x, b, a);
#else
  return x + fMultDiv2BitExact(a, b);
#endif
}
#endif

#if !defined(FUNCTION_fixmsubdiv2BitExact_DD)
#define FUNCTION_fixmsubdiv2BitExact_DD
inline FIXP_DBL fixmsubdiv2BitExact_DD(FIXP_DBL x, const FIXP_DBL a,
                                       const FIXP_DBL b) {
  return x - fMultDiv2BitExact(a, b);
}
#endif
#if !defined(FUNCTION_fixmsubdiv2BitExact_SD)
#define FUNCTION_fixmsubdiv2BitExact_SD
inline FIXP_DBL fixmsubdiv2BitExact_SD(FIXP_DBL x, const FIXP_SGL a,
                                       const FIXP_DBL b) {
#ifdef FUNCTION_fixmsubdiv2BitExact_DS
  return fixmsubdiv2BitExact_DS(x, b, a);
#else
  return x - fMultDiv2BitExact(a, b);
#endif
}
#endif
#if !defined(FUNCTION_fixmsubdiv2BitExact_DS)
#define FUNCTION_fixmsubdiv2BitExact_DS
inline FIXP_DBL fixmsubdiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a,
                                       const FIXP_SGL b) {
#ifdef FUNCTION_fixmsubdiv2BitExact_SD
  return fixmsubdiv2BitExact_SD(x, b, a);
#else
  return x - fMultDiv2BitExact(a, b);
#endif
}
#endif

  /* Normal versions */

#if !defined(FUNCTION_fixmadd_DD)
inline FIXP_DBL fixmadd_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return fixmadddiv2_DD(x, a, b) << 1;
}
#endif
#if !defined(FUNCTION_fixmadd_SD)
inline FIXP_DBL fixmadd_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmadd_DS
  return fixmadd_DS(x, b, a);
#else
  return fixmadd_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif
#if !defined(FUNCTION_fixmadd_DS)
inline FIXP_DBL fixmadd_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmadd_SD
  return fixmadd_SD(x, b, a);
#else
  return fixmadd_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif
#if !defined(FUNCTION_fixmadd_SS)
inline FIXP_DBL fixmadd_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return (x + fMultDiv2(a, b)) << 1;
}
#endif

#if !defined(FUNCTION_fixmsub_DD)
inline FIXP_DBL fixmsub_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  return fixmsubdiv2_DD(x, a, b) << 1;
}
#endif
#if !defined(FUNCTION_fixmsub_SD)
inline FIXP_DBL fixmsub_SD(FIXP_DBL x, const FIXP_SGL a, const FIXP_DBL b) {
#ifdef FUNCTION_fixmsub_DS
  return fixmsub_DS(x, b, a);
#else
  return fixmsub_DD(x, FX_SGL2FX_DBL(a), b);
#endif
}
#endif
#if !defined(FUNCTION_fixmsub_DS)
inline FIXP_DBL fixmsub_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
#ifdef FUNCTION_fixmsub_SD
  return fixmsub_SD(x, b, a);
#else
  return fixmsub_DD(x, a, FX_SGL2FX_DBL(b));
#endif
}
#endif
#if !defined(FUNCTION_fixmsub_SS)
inline FIXP_DBL fixmsub_SS(FIXP_DBL x, const FIXP_SGL a, const FIXP_SGL b) {
  return (x - fMultDiv2(a, b)) << 1;
}
#endif

#if !defined(FUNCTION_fixpow2adddiv2_D)
#ifdef FUNCTION_fixmadddiv2_DD
#define fixpadddiv2_D(x, a) fixmadddiv2_DD(x, a, a)
#else
inline INT fixpadddiv2_D(FIXP_DBL x, const FIXP_DBL a) {
  return (x + fPow2Div2(a));
}
#endif
#endif
#if !defined(FUNCTION_fixpow2add_D)
inline INT fixpadd_D(FIXP_DBL x, const FIXP_DBL a) { return (x + fPow2(a)); }
#endif

#if !defined(FUNCTION_fixpow2adddiv2_S)
#ifdef FUNCTION_fixmadddiv2_SS
#define fixpadddiv2_S(x, a) fixmadddiv2_SS(x, a, a)
#else
inline INT fixpadddiv2_S(FIXP_DBL x, const FIXP_SGL a) {
  return (x + fPow2Div2(a));
}
#endif
#endif
#if !defined(FUNCTION_fixpow2add_S)
inline INT fixpadd_S(FIXP_DBL x, const FIXP_SGL a) { return (x + fPow2(a)); }
#endif

#endif /* FIXMADD_H */
