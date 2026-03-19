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

/**************************** AAC decoder library ******************************

   Author(s):   Matthias Hildenbrand

   Description: USAC ACELP LTP filter

*******************************************************************************/

#include "usacdec_ace_ltp.h"

#include "genericStds.h"
#include "common_fix.h"

#define UP_SAMP 4
#define L_INTERPOL2 16
#define L_SUBFR 64

#define A2 FL2FX_SGL(2 * 0.18f)
#define B FL2FX_SGL(0.64f)

static const LONG Pred_lt4_inter4_2[UP_SAMP][L_INTERPOL2] = {
    {(LONG)0x0000FFFC, (LONG)0x0008FFFC, (LONG)0xFFEB004C, (LONG)0xFF50014A,
     (LONG)0xFDD90351, (LONG)0xFB2A06CD, (LONG)0xF6920D46, (LONG)0xEBB42B35,
     (LONG)0x6D9EEF39, (LONG)0x0618FE0F, (LONG)0xFFE00131, (LONG)0xFE5501C5,
     (LONG)0xFE5E015D, (LONG)0xFEF700B6, (LONG)0xFF920037, (LONG)0xFFEC0003},
    {(LONG)0x0002FFF2, (LONG)0x0026FFBD, (LONG)0x005DFF98, (LONG)0x0055FFEF,
     (LONG)0xFF89015F, (LONG)0xFD3A04E5, (LONG)0xF7D90DAA, (LONG)0xE67A50EE,
     (LONG)0x50EEE67A, (LONG)0x0DAAF7D9, (LONG)0x04E5FD3A, (LONG)0x015FFF89,
     (LONG)0xFFEF0055, (LONG)0xFF98005D, (LONG)0xFFBD0026, (LONG)0xFFF20002},
    {(LONG)0x0003FFEC, (LONG)0x0037FF92, (LONG)0x00B6FEF7, (LONG)0x015DFE5E,
     (LONG)0x01C5FE55, (LONG)0x0131FFE0, (LONG)0xFE0F0618, (LONG)0xEF396D9E,
     (LONG)0x2B35EBB4, (LONG)0x0D46F692, (LONG)0x06CDFB2A, (LONG)0x0351FDD9,
     (LONG)0x014AFF50, (LONG)0x004CFFEB, (LONG)0xFFFC0008, (LONG)0xFFFC0000},
    {(LONG)0x0002FFF2, (LONG)0x002BFF9E, (LONG)0x00B9FECE, (LONG)0x01CFFD75,
     (LONG)0x035EFBC1, (LONG)0x0521FA0C, (LONG)0x06AAF8C9, (LONG)0x07907852,
     (LONG)0x0790F8C9, (LONG)0x06AAFA0C, (LONG)0x0521FBC1, (LONG)0x035EFD75,
     (LONG)0x01CFFECE, (LONG)0x00B9FF9E, (LONG)0x002BFFF2, (LONG)0x00020000}};

void Pred_lt4(FIXP_DBL exc[], /* in/out: excitation buffer              */
              int T0,         /* input : integer pitch lag              */
              int frac        /* input : fraction of lag in range 0..3  */
) {
  int j;
  FIXP_DBL *x;
  const LONG *interpol;
  FIXP_DBL L_sumb, L_sumt;

  x = &exc[-T0 - L_INTERPOL2 + 1];

  /* remap frac and x:
           0 -> 3   x (unchanged)
           1 -> 0   x--
           2 -> 1   x--
           3 -> 2   x--
  */

  if (--frac < 0)
    frac += UP_SAMP;
  else
    x--;

  j = L_SUBFR + 1;
  do {
    LONG filt;
    FIXP_DBL x0, x1;
    FIXP_DBL *xi = x++;
    interpol = Pred_lt4_inter4_2[frac];
    int i = 3;

    filt = *interpol++;
    x0 = *xi++;
    x1 = *xi++;
    L_sumt = fMultDiv2(x0, (FIXP_SGL)((SHORT)(filt >> 16)));
    L_sumb = fMultDiv2(x1, (FIXP_SGL)((SHORT)filt));
    do {
      filt = *interpol++;
      x0 = *xi++;
      x1 = *xi++;
      L_sumt = fMultAddDiv2(L_sumt, x0, (FIXP_SGL)((SHORT)(filt >> 16)));
      L_sumb = fMultAddDiv2(L_sumb, x1, (FIXP_SGL)((SHORT)filt));

      filt = *interpol++;
      x0 = *xi++;
      x1 = *xi++;
      L_sumt = fMultAddDiv2(L_sumt, x0, (FIXP_SGL)((SHORT)(filt >> 16)));
      L_sumb = fMultAddDiv2(L_sumb, x1, (FIXP_SGL)((SHORT)filt));

      filt = *interpol++;
      x0 = *xi++;
      x1 = *xi++;
      L_sumt = fMultAddDiv2(L_sumt, x0, (FIXP_SGL)((SHORT)(filt >> 16)));
      L_sumb = fMultAddDiv2(L_sumb, x1, (FIXP_SGL)((SHORT)filt));

      filt = *interpol++;
      x0 = *xi++;
      x1 = *xi++;
      L_sumt = fMultAddDiv2(L_sumt, x0, (FIXP_SGL)((SHORT)(filt >> 16)));
      L_sumb = fMultAddDiv2(L_sumb, x1, (FIXP_SGL)((SHORT)filt));

      filt = *interpol++;
      x0 = *xi++;
      x1 = *xi++;
      L_sumt = fMultAddDiv2(L_sumt, x0, (FIXP_SGL)((SHORT)(filt >> 16)));
      L_sumb = fMultAddDiv2(L_sumb, x1, (FIXP_SGL)((SHORT)filt));
    } while (--i != 0);

    L_sumb <<= 1;
    L_sumb = fAddSaturate(L_sumt << 1, L_sumb);
    *exc++ = L_sumb;
  } while (--j != 0);
  return;
}

void Pred_lt4_postfilter(FIXP_DBL exc[] /* in/out: excitation buffer */
) {
  /*
  exc[i]   = A*exc[i-1] + B*exc[i] + A*exc[i+1]
  exc[i+1] =              A*exc[i] + B*exc[i+1] + A*exc[i+2] ; i = 0:2:62
  */
  int i;
  FIXP_DBL sum0, sum1, a_exc0, a_exc1;
  a_exc0 = fMultDiv2(A2, exc[-1]);
  a_exc1 = fMultDiv2(A2, exc[0]);

  /* ARM926: 22 cycles/iteration */
  for (i = 0; i < L_SUBFR; i += 2) {
    sum0 = a_exc0 + fMult(B, exc[i]);
    sum1 = a_exc1 + fMult(B, exc[i + 1]);
    a_exc0 = fMultDiv2(A2, exc[i + 1]);
    a_exc1 = fMultDiv2(A2, exc[i + 2]);
    exc[i] = sum0 + a_exc0;
    exc[i + 1] = sum1 + a_exc1;
  }
  return;
}
