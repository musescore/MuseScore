/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2020 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):   M. Lohwasser

   Description: auto-correlation functions

*******************************************************************************/

#include "autocorr2nd.h"

/*!
 *
 * \brief Calculate second order autocorrelation using 2 accumulators
 *
 */
#if !defined(FUNCTION_autoCorr2nd_real)
INT autoCorr2nd_real(
    ACORR_COEFS *ac,          /*!< Pointer to autocorrelation coeffs */
    const FIXP_DBL *reBuffer, /*!< Pointer to to real part of input samples */
    const int len             /*!< Number input samples */
) {
  int j, autoCorrScaling, mScale;

  FIXP_DBL accu1, accu2, accu3, accu4, accu5;

  const FIXP_DBL *pReBuf;

  const FIXP_DBL *realBuf = reBuffer;

  const int len_scale = fMax(DFRACT_BITS - fNormz((FIXP_DBL)(len / 2)), 1);
  /*
    r11r,r22r
    r01r,r12r
    r02r
  */
  pReBuf = realBuf - 2;
  accu5 =
      ((fMultDiv2(pReBuf[0], pReBuf[2]) + fMultDiv2(pReBuf[1], pReBuf[3])) >>
       len_scale);
  pReBuf++;

  /* len must be even */
  accu1 = fPow2Div2(pReBuf[0]) >> len_scale;
  accu3 = fMultDiv2(pReBuf[0], pReBuf[1]) >> len_scale;
  pReBuf++;

  for (j = (len - 2) >> 1; j != 0; j--, pReBuf += 2) {
    accu1 += ((fPow2Div2(pReBuf[0]) + fPow2Div2(pReBuf[1])) >> len_scale);

    accu3 +=
        ((fMultDiv2(pReBuf[0], pReBuf[1]) + fMultDiv2(pReBuf[1], pReBuf[2])) >>
         len_scale);

    accu5 +=
        ((fMultDiv2(pReBuf[0], pReBuf[2]) + fMultDiv2(pReBuf[1], pReBuf[3])) >>
         len_scale);
  }

  accu2 = (fPow2Div2(realBuf[-2]) >> len_scale);
  accu2 += accu1;

  accu1 += (fPow2Div2(realBuf[len - 2]) >> len_scale);

  accu4 = (fMultDiv2(realBuf[-1], realBuf[-2]) >> len_scale);
  accu4 += accu3;

  accu3 += (fMultDiv2(realBuf[len - 1], realBuf[len - 2]) >> len_scale);

  mScale = CntLeadingZeros(
               (accu1 | accu2 | fAbs(accu3) | fAbs(accu4) | fAbs(accu5))) -
           1;
  autoCorrScaling = mScale - 1 - len_scale; /* -1 because of fMultDiv2*/

  /* Scale to common scale factor */
  ac->r11r = accu1 << mScale;
  ac->r22r = accu2 << mScale;
  ac->r01r = accu3 << mScale;
  ac->r12r = accu4 << mScale;
  ac->r02r = accu5 << mScale;

  ac->det = (fMultDiv2(ac->r11r, ac->r22r) - fMultDiv2(ac->r12r, ac->r12r));
  mScale = CountLeadingBits(fAbs(ac->det));

  ac->det <<= mScale;
  ac->det_scale = mScale - 1;

  return autoCorrScaling;
}
#endif

#if !defined(FUNCTION_autoCorr2nd_cplx)
INT autoCorr2nd_cplx(
    ACORR_COEFS *ac,          /*!< Pointer to autocorrelation coeffs */
    const FIXP_DBL *reBuffer, /*!< Pointer to real part of input samples */
    const FIXP_DBL *imBuffer, /*!< Pointer to imag part of input samples */
    const int len /*!< Number of input samples (should be smaller than 128) */
) {
  int j, autoCorrScaling, mScale;

  FIXP_DBL accu0, accu1, accu2, accu3, accu4, accu5, accu6, accu7, accu8;

  const FIXP_DBL *pReBuf, *pImBuf;

  const FIXP_DBL *realBuf = reBuffer;
  const FIXP_DBL *imagBuf = imBuffer;

  const int len_scale = fMax(DFRACT_BITS - fNormz((FIXP_DBL)len), 1);
  /*
    r00r,
    r11r,r22r
    r01r,r12r
    r01i,r12i
    r02r,r02i
  */
  accu1 = accu3 = accu5 = accu7 = accu8 = FL2FXCONST_DBL(0.0f);

  pReBuf = realBuf - 2, pImBuf = imagBuf - 2;
  accu7 +=
      ((fMultDiv2(pReBuf[2], pReBuf[0]) + fMultDiv2(pImBuf[2], pImBuf[0])) >>
       len_scale);
  accu8 +=
      ((fMultDiv2(pImBuf[2], pReBuf[0]) - fMultDiv2(pReBuf[2], pImBuf[0])) >>
       len_scale);

  pReBuf = realBuf - 1, pImBuf = imagBuf - 1;
  for (j = (len - 1); j != 0; j--, pReBuf++, pImBuf++) {
    accu1 += ((fPow2Div2(pReBuf[0]) + fPow2Div2(pImBuf[0])) >> len_scale);
    accu3 +=
        ((fMultDiv2(pReBuf[0], pReBuf[1]) + fMultDiv2(pImBuf[0], pImBuf[1])) >>
         len_scale);
    accu5 +=
        ((fMultDiv2(pImBuf[1], pReBuf[0]) - fMultDiv2(pReBuf[1], pImBuf[0])) >>
         len_scale);
    accu7 +=
        ((fMultDiv2(pReBuf[2], pReBuf[0]) + fMultDiv2(pImBuf[2], pImBuf[0])) >>
         len_scale);
    accu8 +=
        ((fMultDiv2(pImBuf[2], pReBuf[0]) - fMultDiv2(pReBuf[2], pImBuf[0])) >>
         len_scale);
  }

  accu2 = ((fPow2Div2(realBuf[-2]) + fPow2Div2(imagBuf[-2])) >> len_scale);
  accu2 += accu1;

  accu1 += ((fPow2Div2(realBuf[len - 2]) + fPow2Div2(imagBuf[len - 2])) >>
            len_scale);
  accu0 = ((fPow2Div2(realBuf[len - 1]) + fPow2Div2(imagBuf[len - 1])) >>
           len_scale) -
          ((fPow2Div2(realBuf[-1]) + fPow2Div2(imagBuf[-1])) >> len_scale);
  accu0 += accu1;

  accu4 = ((fMultDiv2(realBuf[-1], realBuf[-2]) +
            fMultDiv2(imagBuf[-1], imagBuf[-2])) >>
           len_scale);
  accu4 += accu3;

  accu3 += ((fMultDiv2(realBuf[len - 1], realBuf[len - 2]) +
             fMultDiv2(imagBuf[len - 1], imagBuf[len - 2])) >>
            len_scale);

  accu6 = ((fMultDiv2(imagBuf[-1], realBuf[-2]) -
            fMultDiv2(realBuf[-1], imagBuf[-2])) >>
           len_scale);
  accu6 += accu5;

  accu5 += ((fMultDiv2(imagBuf[len - 1], realBuf[len - 2]) -
             fMultDiv2(realBuf[len - 1], imagBuf[len - 2])) >>
            len_scale);

  mScale =
      CntLeadingZeros((accu0 | accu1 | accu2 | fAbs(accu3) | fAbs(accu4) |
                       fAbs(accu5) | fAbs(accu6) | fAbs(accu7) | fAbs(accu8))) -
      1;
  autoCorrScaling = mScale - 1 - len_scale; /* -1 because of fMultDiv2*/

  /* Scale to common scale factor */
  ac->r00r = (FIXP_DBL)accu0 << mScale;
  ac->r11r = (FIXP_DBL)accu1 << mScale;
  ac->r22r = (FIXP_DBL)accu2 << mScale;
  ac->r01r = (FIXP_DBL)accu3 << mScale;
  ac->r12r = (FIXP_DBL)accu4 << mScale;
  ac->r01i = (FIXP_DBL)accu5 << mScale;
  ac->r12i = (FIXP_DBL)accu6 << mScale;
  ac->r02r = (FIXP_DBL)accu7 << mScale;
  ac->r02i = (FIXP_DBL)accu8 << mScale;

  ac->det =
      (fMultDiv2(ac->r11r, ac->r22r) >> 1) -
      ((fMultDiv2(ac->r12r, ac->r12r) + fMultDiv2(ac->r12i, ac->r12i)) >> 1);
  mScale = CntLeadingZeros(fAbs(ac->det)) - 1;

  ac->det <<= mScale;
  ac->det_scale = mScale - 2;

  return autoCorrScaling;
}

#endif /* FUNCTION_autoCorr2nd_cplx */
