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

/**************************** AAC encoder library ******************************

   Author(s):   Tobias Chalupka

   Description: FDKaacLdEnc_MdctTransform480:
                The module FDKaacLdEnc_MdctTransform will perform the MDCT.
                The MDCT supports the sine window and
                the zero padded window. The algorithm of the MDCT
                can be divided in  Windowing, PreModulation, Fft and
                PostModulation.

*******************************************************************************/

#include "transform.h"
#include "dct.h"
#include "psy_const.h"
#include "aacEnc_rom.h"
#include "FDK_tools_rom.h"

#if defined(__arm__)
#endif

INT FDKaacEnc_Transform_Real(const INT_PCM *pTimeData,
                             FIXP_DBL *RESTRICT mdctData, const INT blockType,
                             const INT windowShape, INT *prevWindowShape,
                             H_MDCT mdctPers, const INT frameLength,
                             INT *pMdctData_e, INT filterType) {
  const INT_PCM *RESTRICT timeData;

  UINT numSpec;
  UINT numMdctLines;
  UINT offset;
  int fr; /* fr: right window slope length */
  SHORT mdctData_e[8];

  timeData = pTimeData;

  if (blockType == SHORT_WINDOW) {
    numSpec = 8;
    numMdctLines = frameLength >> 3;
  } else {
    numSpec = 1;
    numMdctLines = frameLength;
  }

  offset = (windowShape == LOL_WINDOW) ? ((frameLength * 3) >> 2) : 0;
  switch (blockType) {
    case LONG_WINDOW:
    case STOP_WINDOW:
      fr = frameLength - offset;
      break;
    case START_WINDOW: /* or StopStartSequence */
    case SHORT_WINDOW:
      fr = frameLength >> 3;
      break;
    default:
      FDK_ASSERT(0);
      return -1;
  }

  mdct_block(mdctPers, timeData, frameLength, mdctData, numSpec, numMdctLines,
             FDKgetWindowSlope(fr, windowShape), fr, mdctData_e);

  if (blockType == SHORT_WINDOW) {
    if (!(mdctData_e[0] == mdctData_e[1] && mdctData_e[1] == mdctData_e[2] &&
          mdctData_e[2] == mdctData_e[3] && mdctData_e[3] == mdctData_e[4] &&
          mdctData_e[4] == mdctData_e[5] && mdctData_e[5] == mdctData_e[6] &&
          mdctData_e[6] == mdctData_e[7])) {
      return -1;
    }
  }
  *prevWindowShape = windowShape;
  *pMdctData_e = mdctData_e[0];

  return 0;
}

INT FDKaacEnc_Transform_Real_Eld(const INT_PCM *pTimeData,
                                 FIXP_DBL *RESTRICT mdctData,
                                 const INT blockType, const INT windowShape,
                                 INT *prevWindowShape, const INT frameLength,
                                 INT *mdctData_e, INT filterType,
                                 FIXP_DBL *RESTRICT overlapAddBuffer) {
  const INT_PCM *RESTRICT timeData;

  INT i;

  /* tl: transform length
     fl: left window slope length
     nl: left window slope offset
     fr: right window slope length
     nr: right window slope offset */
  const FIXP_WTB *pWindowELD = NULL;
  int N = frameLength;
  int L = frameLength;

  timeData = pTimeData;

  if (blockType != LONG_WINDOW) {
    return -1;
  }

  /*
   * MDCT scale:
   * + 1: fMultDiv2() in windowing.
   * + 1: Because of factor 1/2 in Princen-Bradley compliant windowed TDAC.
   */
  *mdctData_e = 1 + 1;

  switch (frameLength) {
    case 512:
      pWindowELD = ELDAnalysis512;
      break;
    case 480:
      pWindowELD = ELDAnalysis480;
      break;
    case 256:
      pWindowELD = ELDAnalysis256;
      *mdctData_e += 1;
      break;
    case 240:
      pWindowELD = ELDAnalysis240;
      *mdctData_e += 1;
      break;
    case 128:
      pWindowELD = ELDAnalysis128;
      *mdctData_e += 2;
      break;
    case 120:
      pWindowELD = ELDAnalysis120;
      *mdctData_e += 2;
      break;
    default:
      FDK_ASSERT(0);
      return -1;
  }

  for (i = 0; i < N / 4; i++) {
    FIXP_DBL z0, outval;

    z0 = (fMult((FIXP_PCM)timeData[L + N * 3 / 4 - 1 - i],
                pWindowELD[N / 2 - 1 - i])
          << (WTS0 - 1)) +
         (fMult((FIXP_PCM)timeData[L + N * 3 / 4 + i], pWindowELD[N / 2 + i])
          << (WTS0 - 1));

    outval = (fMultDiv2((FIXP_PCM)timeData[L + N * 3 / 4 - 1 - i],
                        pWindowELD[N + N / 2 - 1 - i]) >>
              (-WTS1));
    outval += (fMultDiv2((FIXP_PCM)timeData[L + N * 3 / 4 + i],
                         pWindowELD[N + N / 2 + i]) >>
               (-WTS1));
    outval += (fMultDiv2(overlapAddBuffer[N / 2 + i], pWindowELD[2 * N + i]) >>
               (-WTS2 - 1));

    overlapAddBuffer[N / 2 + i] = overlapAddBuffer[i];

    overlapAddBuffer[i] = z0;
    mdctData[i] = overlapAddBuffer[N / 2 + i] +
                  (fMultDiv2(overlapAddBuffer[N + N / 2 - 1 - i],
                             pWindowELD[2 * N + N / 2 + i]) >>
                   (-WTS2 - 1));

    mdctData[N - 1 - i] = outval;
    overlapAddBuffer[N + N / 2 - 1 - i] = outval;
  }

  for (i = N / 4; i < N / 2; i++) {
    FIXP_DBL z0, outval;

    z0 = fMult((FIXP_PCM)timeData[L + N * 3 / 4 - 1 - i],
               pWindowELD[N / 2 - 1 - i])
         << (WTS0 - 1);

    outval = (fMultDiv2((FIXP_PCM)timeData[L + N * 3 / 4 - 1 - i],
                        pWindowELD[N + N / 2 - 1 - i]) >>
              (-WTS1));
    outval += (fMultDiv2(overlapAddBuffer[N / 2 + i], pWindowELD[2 * N + i]) >>
               (-WTS2 - 1));

    overlapAddBuffer[N / 2 + i] =
        overlapAddBuffer[i] +
        (fMult((FIXP_PCM)timeData[L - N / 4 + i], pWindowELD[N / 2 + i])
         << (WTS0 - 1));

    overlapAddBuffer[i] = z0;
    mdctData[i] = overlapAddBuffer[N / 2 + i] +
                  (fMultDiv2(overlapAddBuffer[N + N / 2 - 1 - i],
                             pWindowELD[2 * N + N / 2 + i]) >>
                   (-WTS2 - 1));

    mdctData[N - 1 - i] = outval;
    overlapAddBuffer[N + N / 2 - 1 - i] = outval;
  }
  dct_IV(mdctData, frameLength, mdctData_e);

  *prevWindowShape = windowShape;

  return 0;
}
