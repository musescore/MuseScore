/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):

   Description: low delay filterbank

*******************************************************************************/

#include "ldfiltbank.h"

#include "aac_rom.h"
#include "dct.h"
#include "FDK_tools_rom.h"
#include "mdct.h"

#define LDFB_HEADROOM 2

#if defined(__arm__)
#endif

static void multE2_DinvF_fdk(PCM_DEC *output, FIXP_DBL *x, const FIXP_WTB *fb,
                             FIXP_DBL *z, const int N) {
  int i;

  /*  scale for FIXP_DBL -> PCM_DEC conversion:       */
  const int scale = (DFRACT_BITS - PCM_OUT_BITS) - LDFB_HEADROOM + (3);

#if ((DFRACT_BITS - PCM_OUT_BITS - LDFB_HEADROOM + (3) - 1) > 0)
  FIXP_DBL rnd_val_wts0 = (FIXP_DBL)0;
  FIXP_DBL rnd_val_wts1 = (FIXP_DBL)0;
#if ((DFRACT_BITS - PCM_OUT_BITS - LDFB_HEADROOM + (3) - WTS0 - 1) > 0)
  if (-WTS0 - 1 + scale)
    rnd_val_wts0 = (FIXP_DBL)(1 << (-WTS0 - 1 + scale - 1));
#endif
  if (-WTS1 - 1 + scale)
    rnd_val_wts1 = (FIXP_DBL)(1 << (-WTS1 - 1 + scale - 1));
#endif

  for (i = 0; i < N / 4; i++) {
    FIXP_DBL z0, z2, tmp;

    z2 = x[N / 2 + i];
    z0 = fAddSaturate(z2,
                      (fMultDiv2(z[N / 2 + i], fb[2 * N + i]) >> (-WTS2 - 1)));

    z[N / 2 + i] = fAddSaturate(
        x[N / 2 - 1 - i],
        (fMultDiv2(z[N + i], fb[2 * N + N / 2 + i]) >> (-WTS2 - 1)));

    tmp = (fMultDiv2(z[N / 2 + i], fb[N + N / 2 - 1 - i]) +
           fMultDiv2(z[i], fb[N + N / 2 + i]));

#if ((DFRACT_BITS - PCM_OUT_BITS - LDFB_HEADROOM + (3) - 1) > 0)
    FDK_ASSERT((-WTS1 - 1 + scale) >= 0);
    FDK_ASSERT(tmp <= ((FIXP_DBL)0x7FFFFFFF -
                       rnd_val_wts1)); /* rounding must not cause overflow */
    output[(N * 3 / 4 - 1 - i)] = (PCM_DEC)SATURATE_RIGHT_SHIFT(
        tmp + rnd_val_wts1, -WTS1 - 1 + scale, PCM_OUT_BITS);
#else
    FDK_ASSERT((WTS1 + 1 - scale) >= 0);
    output[(N * 3 / 4 - 1 - i)] =
        (PCM_DEC)SATURATE_LEFT_SHIFT(tmp, WTS1 + 1 - scale, PCM_OUT_BITS);
#endif

    z[i] = z0;
    z[N + i] = z2;
  }

  for (i = N / 4; i < N / 2; i++) {
    FIXP_DBL z0, z2, tmp0, tmp1;

    z2 = x[N / 2 + i];
    z0 = fAddSaturate(z2,
                      (fMultDiv2(z[N / 2 + i], fb[2 * N + i]) >> (-WTS2 - 1)));

    z[N / 2 + i] = fAddSaturate(
        x[N / 2 - 1 - i],
        (fMultDiv2(z[N + i], fb[2 * N + N / 2 + i]) >> (-WTS2 - 1)));

    tmp0 = (fMultDiv2(z[N / 2 + i], fb[N / 2 - 1 - i]) +
            fMultDiv2(z[i], fb[N / 2 + i]));
    tmp1 = (fMultDiv2(z[N / 2 + i], fb[N + N / 2 - 1 - i]) +
            fMultDiv2(z[i], fb[N + N / 2 + i]));

#if ((DFRACT_BITS - PCM_OUT_BITS - LDFB_HEADROOM + (3) - 1) > 0)
    FDK_ASSERT((-WTS0 - 1 + scale) >= 0);
    FDK_ASSERT(tmp0 <= ((FIXP_DBL)0x7FFFFFFF -
                        rnd_val_wts0)); /* rounding must not cause overflow */
    FDK_ASSERT(tmp1 <= ((FIXP_DBL)0x7FFFFFFF -
                        rnd_val_wts1)); /* rounding must not cause overflow */
    output[(i - N / 4)] = (PCM_DEC)SATURATE_RIGHT_SHIFT(
        tmp0 + rnd_val_wts0, -WTS0 - 1 + scale, PCM_OUT_BITS);
    output[(N * 3 / 4 - 1 - i)] = (PCM_DEC)SATURATE_RIGHT_SHIFT(
        tmp1 + rnd_val_wts1, -WTS1 - 1 + scale, PCM_OUT_BITS);
#else
    FDK_ASSERT((WTS0 + 1 - scale) >= 0);
    output[(i - N / 4)] =
        (PCM_DEC)SATURATE_LEFT_SHIFT(tmp0, WTS0 + 1 - scale, PCM_OUT_BITS);
    output[(N * 3 / 4 - 1 - i)] =
        (PCM_DEC)SATURATE_LEFT_SHIFT(tmp1, WTS1 + 1 - scale, PCM_OUT_BITS);
#endif
    z[i] = z0;
    z[N + i] = z2;
  }

  /* Exchange quarter parts of x to bring them in the "right" order */
  for (i = 0; i < N / 4; i++) {
    FIXP_DBL tmp0 = fMultDiv2(z[i], fb[N / 2 + i]);

#if ((DFRACT_BITS - PCM_OUT_BITS - LDFB_HEADROOM + (3) - 1) > 0)
    FDK_ASSERT((-WTS0 - 1 + scale) >= 0);
    FDK_ASSERT(tmp0 <= ((FIXP_DBL)0x7FFFFFFF -
                        rnd_val_wts0)); /* rounding must not cause overflow */
    output[(N * 3 / 4 + i)] = (PCM_DEC)SATURATE_RIGHT_SHIFT(
        tmp0 + rnd_val_wts0, -WTS0 - 1 + scale, PCM_OUT_BITS);
#else
    FDK_ASSERT((WTS0 + 1 - scale) >= 0);
    output[(N * 3 / 4 + i)] =
        (PCM_DEC)SATURATE_LEFT_SHIFT(tmp0, WTS0 + 1 - scale, PCM_OUT_BITS);
#endif
  }
}

int InvMdctTransformLowDelay_fdk(FIXP_DBL *mdctData, const int mdctData_e,
                                 PCM_DEC *output, FIXP_DBL *fs_buffer,
                                 const int N) {
  const FIXP_WTB *coef;
  FIXP_DBL gain = (FIXP_DBL)0;
  int scale = mdctData_e + MDCT_OUT_HEADROOM -
              LDFB_HEADROOM; /* The LDFB_HEADROOM is compensated inside
                                multE2_DinvF_fdk() below */
  int i;

  /* Select LD window slope */
  switch (N) {
    case 256:
      coef = LowDelaySynthesis256;
      break;
    case 240:
      coef = LowDelaySynthesis240;
      break;
    case 160:
      coef = LowDelaySynthesis160;
      break;
    case 128:
      coef = LowDelaySynthesis128;
      break;
    case 120:
      coef = LowDelaySynthesis120;
      break;
    case 512:
      coef = LowDelaySynthesis512;
      break;
    case 480:
    default:
      coef = LowDelaySynthesis480;
      break;
  }

  /*
     Apply exponent and 1/N factor.
     Note: "scale" is off by one because for LD_MDCT the window length is twice
     the window length of a regular MDCT. This is corrected inside
     multE2_DinvF_fdk(). Refer to ISO/IEC 14496-3:2009 page 277,
     chapter 4.6.20.2 "Low Delay Window".
   */
  imdct_gain(&gain, &scale, N);

  dct_IV(mdctData, N, &scale);

  if (N == 256 || N == 240 || N == 160) {
    scale -= 1;
  } else if (N == 128 || N == 120) {
    scale -= 2;
  }

  if (gain != (FIXP_DBL)0) {
    for (i = 0; i < N; i++) {
      mdctData[i] = fMult(mdctData[i], gain);
    }
  }
  scaleValuesSaturate(mdctData, N, scale);

  /* Since all exponent and factors have been applied, current exponent is zero.
   */
  multE2_DinvF_fdk(output, mdctData, coef, fs_buffer, N);

  return (1);
}
