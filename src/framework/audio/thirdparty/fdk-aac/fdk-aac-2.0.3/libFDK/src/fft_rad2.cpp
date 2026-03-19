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

   Description:

*******************************************************************************/

#include "fft_rad2.h"

#include "scramble.h"

#define __FFT_RAD2_CPP__

#if defined(__arm__)
#include "arm/fft_rad2_arm.cpp"

#elif defined(__GNUC__) && defined(__mips__) && defined(__mips_dsp) && !defined(__mips16)
#include "mips/fft_rad2_mips.cpp"

#endif

/*****************************************************************************

    functionname: dit_fft (analysis)
    description:  dit-tukey-algorithm
                  scrambles data at entry
                  i.e. loop is made with scrambled data
    returns:
    input:
    output:

*****************************************************************************/

#ifndef FUNCTION_dit_fft

void dit_fft(FIXP_DBL *x, const INT ldn, const FIXP_STP *trigdata,
             const INT trigDataSize) {
  const INT n = 1 << ldn;
  INT trigstep, i, ldm;

  C_ALLOC_ALIGNED_CHECK(x);

  scramble(x, n);
  /*
   * 1+2 stage radix 4
   */

  for (i = 0; i < n * 2; i += 8) {
    FIXP_DBL a00, a10, a20, a30;
    a00 = (x[i + 0] + x[i + 2]) >> 1; /* Re A + Re B */
    a10 = (x[i + 4] + x[i + 6]) >> 1; /* Re C + Re D */
    a20 = (x[i + 1] + x[i + 3]) >> 1; /* Im A + Im B */
    a30 = (x[i + 5] + x[i + 7]) >> 1; /* Im C + Im D */

    x[i + 0] = a00 + a10; /* Re A' = Re A + Re B + Re C + Re D */
    x[i + 4] = a00 - a10; /* Re C' = Re A + Re B - Re C - Re D */
    x[i + 1] = a20 + a30; /* Im A' = Im A + Im B + Im C + Im D */
    x[i + 5] = a20 - a30; /* Im C' = Im A + Im B - Im C - Im D */

    a00 = a00 - x[i + 2]; /* Re A - Re B */
    a10 = a10 - x[i + 6]; /* Re C - Re D */
    a20 = a20 - x[i + 3]; /* Im A - Im B */
    a30 = a30 - x[i + 7]; /* Im C - Im D */

    x[i + 2] = a00 + a30; /* Re B' = Re A - Re B + Im C - Im D */
    x[i + 6] = a00 - a30; /* Re D' = Re A - Re B - Im C + Im D */
    x[i + 3] = a20 - a10; /* Im B' = Im A - Im B - Re C + Re D */
    x[i + 7] = a20 + a10; /* Im D' = Im A - Im B + Re C - Re D */
  }

  for (ldm = 3; ldm <= ldn; ++ldm) {
    INT m = (1 << ldm);
    INT mh = (m >> 1);
    INT j, r;

    trigstep = ((trigDataSize << 2) >> ldm);

    FDK_ASSERT(trigstep > 0);

    /* Do first iteration with c=1.0 and s=0.0 separately to avoid loosing to
       much precision. Beware: The impact on the overal FFT precision is rather
       large. */
    { /* block 1 */

      j = 0;

      for (r = 0; r < n; r += m) {
        INT t1 = (r + j) << 1;
        INT t2 = t1 + (mh << 1);
        FIXP_DBL vr, vi, ur, ui;

        // cplxMultDiv2(&vi, &vr, x[t2+1], x[t2], (FIXP_SGL)1.0, (FIXP_SGL)0.0);
        vi = x[t2 + 1] >> 1;
        vr = x[t2] >> 1;

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui + vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui - vi;

        t1 += mh;
        t2 = t1 + (mh << 1);

        // cplxMultDiv2(&vr, &vi, x[t2+1], x[t2], (FIXP_SGL)1.0, (FIXP_SGL)0.0);
        vr = x[t2 + 1] >> 1;
        vi = x[t2] >> 1;

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui - vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui + vi;
      }

    } /* end of  block 1 */

    for (j = 1; j < mh / 4; ++j) {
      FIXP_STP cs;

      cs = trigdata[j * trigstep];

      for (r = 0; r < n; r += m) {
        INT t1 = (r + j) << 1;
        INT t2 = t1 + (mh << 1);
        FIXP_DBL vr, vi, ur, ui;

        cplxMultDiv2(&vi, &vr, x[t2 + 1], x[t2], cs);

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui + vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui - vi;

        t1 += mh;
        t2 = t1 + (mh << 1);

        cplxMultDiv2(&vr, &vi, x[t2 + 1], x[t2], cs);

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui - vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui + vi;

        /* Same as above but for t1,t2 with j>mh/4 and thus cs swapped */
        t1 = (r + mh / 2 - j) << 1;
        t2 = t1 + (mh << 1);

        cplxMultDiv2(&vi, &vr, x[t2], x[t2 + 1], cs);

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui - vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui + vi;

        t1 += mh;
        t2 = t1 + (mh << 1);

        cplxMultDiv2(&vr, &vi, x[t2], x[t2 + 1], cs);

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur - vr;
        x[t1 + 1] = ui - vi;

        x[t2] = ur + vr;
        x[t2 + 1] = ui + vi;
      }
    }

    { /* block 2 */
      j = mh / 4;

      for (r = 0; r < n; r += m) {
        INT t1 = (r + j) << 1;
        INT t2 = t1 + (mh << 1);
        FIXP_DBL vr, vi, ur, ui;

        cplxMultDiv2(&vi, &vr, x[t2 + 1], x[t2], STC(0x5a82799a),
                     STC(0x5a82799a));

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui + vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui - vi;

        t1 += mh;
        t2 = t1 + (mh << 1);

        cplxMultDiv2(&vr, &vi, x[t2 + 1], x[t2], STC(0x5a82799a),
                     STC(0x5a82799a));

        ur = x[t1] >> 1;
        ui = x[t1 + 1] >> 1;

        x[t1] = ur + vr;
        x[t1 + 1] = ui - vi;

        x[t2] = ur - vr;
        x[t2 + 1] = ui + vi;
      }
    } /* end of block 2 */
  }
}

#endif
