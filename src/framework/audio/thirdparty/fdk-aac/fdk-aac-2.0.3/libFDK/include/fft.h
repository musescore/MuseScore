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

   Author(s):   Josef Hoepfl, DSP Solutions

   Description: Fix point FFT

*******************************************************************************/

#ifndef FFT_H
#define FFT_H

#include "common_fix.h"

/**
 * \brief Perform an inplace complex valued FFT of length 2^n
 *
 * \param length Length of the FFT to be calculated.
 * \param pInput Input/Output data buffer. The input data must have at least 1
 * bit scale headroom. The values are interleaved, real/imag pairs.
 * \param scalefactor Pointer to an INT, which contains the current scale of the
 * input data, which is updated according to the FFT scale.
 */
void fft(int length, FIXP_DBL *pInput, INT *scalefactor);

/**
 * \brief Perform an inplace complex valued IFFT of length 2^n
 *
 * \param length Length of the FFT to be calculated.
 * \param pInput Input/Output data buffer. The input data must have at least 1
 * bit scale headroom. The values are interleaved, real/imag pairs.
 * \param scalefactor Pointer to an INT, which contains the current scale of the
 * input data, which is updated according to the IFFT scale.
 */
void ifft(int length, FIXP_DBL *pInput, INT *scalefactor);

/*
 * Frequently used and fixed short length FFTs.
 */

#ifndef FUNCTION_fft_4
/**
 * \brief Perform an inplace complex valued FFT of length 4
 *
 * \param pInput Input/Output data buffer. The input data must have at least 1
 * bit scale headroom. The values are interleaved, real/imag pairs.
 */
LNK_SECTION_CODE_L1
static inline void fft_4(FIXP_DBL *x) {
  FIXP_DBL a00, a10, a20, a30, tmp0, tmp1;

  a00 = (x[0] + x[4]) >> 1; /* Re A + Re B */
  a10 = (x[2] + x[6]) >> 1; /* Re C + Re D */
  a20 = (x[1] + x[5]) >> 1; /* Im A + Im B */
  a30 = (x[3] + x[7]) >> 1; /* Im C + Im D */

  x[0] = a00 + a10; /* Re A' = Re A + Re B + Re C + Re D */
  x[1] = a20 + a30; /* Im A' = Im A + Im B + Im C + Im D */

  tmp0 = a00 - x[4]; /* Re A - Re B */
  tmp1 = a20 - x[5]; /* Im A - Im B */

  x[4] = a00 - a10; /* Re C' = Re A + Re B - Re C - Re D */
  x[5] = a20 - a30; /* Im C' = Im A + Im B - Im C - Im D */

  a10 = a10 - x[6]; /* Re C - Re D */
  a30 = a30 - x[7]; /* Im C - Im D */

  x[2] = tmp0 + a30; /* Re B' = Re A - Re B + Im C - Im D */
  x[6] = tmp0 - a30; /* Re D' = Re A - Re B - Im C + Im D */
  x[3] = tmp1 - a10; /* Im B' = Im A - Im B - Re C + Re D */
  x[7] = tmp1 + a10; /* Im D' = Im A - Im B + Re C - Re D */
}
#endif /* FUNCTION_fft_4 */

#ifndef FUNCTION_fft_8
LNK_SECTION_CODE_L1
static inline void fft_8(FIXP_DBL *x) {
  FIXP_SPK w_PiFOURTH = {{FIXP_SGL(0x5A82), FIXP_SGL(0x5A82)}};

  FIXP_DBL a00, a10, a20, a30;
  FIXP_DBL y[16];

  a00 = (x[0] + x[8]) >> 1;
  a10 = x[4] + x[12];
  a20 = (x[1] + x[9]) >> 1;
  a30 = x[5] + x[13];

  y[0] = a00 + (a10 >> 1);
  y[4] = a00 - (a10 >> 1);
  y[1] = a20 + (a30 >> 1);
  y[5] = a20 - (a30 >> 1);

  a00 = a00 - x[8];
  a10 = (a10 >> 1) - x[12];
  a20 = a20 - x[9];
  a30 = (a30 >> 1) - x[13];

  y[2] = a00 + a30;
  y[6] = a00 - a30;
  y[3] = a20 - a10;
  y[7] = a20 + a10;

  a00 = (x[2] + x[10]) >> 1;
  a10 = x[6] + x[14];
  a20 = (x[3] + x[11]) >> 1;
  a30 = x[7] + x[15];

  y[8] = a00 + (a10 >> 1);
  y[12] = a00 - (a10 >> 1);
  y[9] = a20 + (a30 >> 1);
  y[13] = a20 - (a30 >> 1);

  a00 = a00 - x[10];
  a10 = (a10 >> 1) - x[14];
  a20 = a20 - x[11];
  a30 = (a30 >> 1) - x[15];

  y[10] = a00 + a30;
  y[14] = a00 - a30;
  y[11] = a20 - a10;
  y[15] = a20 + a10;

  FIXP_DBL vr, vi, ur, ui;

  ur = y[0] >> 1;
  ui = y[1] >> 1;
  vr = y[8];
  vi = y[9];
  x[0] = ur + (vr >> 1);
  x[1] = ui + (vi >> 1);
  x[8] = ur - (vr >> 1);
  x[9] = ui - (vi >> 1);

  ur = y[4] >> 1;
  ui = y[5] >> 1;
  vi = y[12];
  vr = y[13];
  x[4] = ur + (vr >> 1);
  x[5] = ui - (vi >> 1);
  x[12] = ur - (vr >> 1);
  x[13] = ui + (vi >> 1);

  ur = y[10];
  ui = y[11];

  cplxMultDiv2(&vi, &vr, ui, ur, w_PiFOURTH);

  ur = y[2];
  ui = y[3];
  x[2] = (ur >> 1) + vr;
  x[3] = (ui >> 1) + vi;
  x[10] = (ur >> 1) - vr;
  x[11] = (ui >> 1) - vi;

  ur = y[14];
  ui = y[15];

  cplxMultDiv2(&vr, &vi, ui, ur, w_PiFOURTH);

  ur = y[6];
  ui = y[7];
  x[6] = (ur >> 1) + vr;
  x[7] = (ui >> 1) - vi;
  x[14] = (ur >> 1) - vr;
  x[15] = (ui >> 1) + vi;
}
#endif /* FUNCTION_fft_8 */

#endif
