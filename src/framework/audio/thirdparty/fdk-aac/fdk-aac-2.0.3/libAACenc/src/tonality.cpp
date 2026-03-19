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

   Author(s):   M. Werner

   Description: Convert chaos measure to the tonality index

*******************************************************************************/

#include "tonality.h"

#include "chaosmeasure.h"

#if defined(__arm__)
#endif

static const FIXP_DBL normlog =
    (FIXP_DBL)0xd977d949; /*FL2FXCONST_DBL(-0.4342944819f *
                             FDKlog(2.0)/FDKlog(2.7182818)); */

static void FDKaacEnc_CalcSfbTonality(FIXP_DBL *RESTRICT spectrum,
                                      INT *RESTRICT sfbMaxScaleSpec,
                                      FIXP_DBL *RESTRICT chaosMeasure,
                                      FIXP_SGL *RESTRICT sfbTonality,
                                      INT sfbCnt, const INT *RESTRICT sfbOffset,
                                      FIXP_DBL *RESTRICT sfbEnergyLD64);

void FDKaacEnc_CalculateFullTonality(FIXP_DBL *RESTRICT spectrum,
                                     INT *RESTRICT sfbMaxScaleSpec,
                                     FIXP_DBL *RESTRICT sfbEnergyLD64,
                                     FIXP_SGL *RESTRICT sfbTonality, INT sfbCnt,
                                     const INT *sfbOffset, INT usePns) {
  INT j;
  INT numberOfLines = sfbOffset[sfbCnt];

  if (usePns) {
    C_ALLOC_SCRATCH_START(chaosMeasurePerLine, FIXP_DBL, (1024))

    /* calculate chaos measure */
    FDKaacEnc_CalculateChaosMeasure(spectrum, numberOfLines,
                                    chaosMeasurePerLine);

    /* smooth ChaosMeasure */
    FIXP_DBL left = chaosMeasurePerLine[0];
    FIXP_DBL right;
    for (j = 1; j < (numberOfLines - 1); j += 2) {
      right = chaosMeasurePerLine[j];
      right = right - (right >> 2);
      left = right + (left >> 2);
      chaosMeasurePerLine[j] = left; /* 0.25 left + 0.75 right */

      right = chaosMeasurePerLine[j + 1];
      right = right - (right >> 2);
      left = right + (left >> 2);
      chaosMeasurePerLine[j + 1] = left;
    }
    if (j == (numberOfLines - 1)) {
      right = chaosMeasurePerLine[j];
      right = right - (right >> 2);
      left = right + (left >> 2);
      chaosMeasurePerLine[j] = left;
    }

    FDKaacEnc_CalcSfbTonality(spectrum, sfbMaxScaleSpec, chaosMeasurePerLine,
                              sfbTonality, sfbCnt, sfbOffset, sfbEnergyLD64);

    C_ALLOC_SCRATCH_END(chaosMeasurePerLine, FIXP_DBL, (1024))
  }
}

/*****************************************************************************

    functionname: CalculateTonalityIndex
    description:  computes tonality values out of unpredictability values
                  limits range and computes log()
    returns:
    input:        ptr to energies, ptr to chaos measure values,
                  number of sfb
    output:       sfb wise tonality values

*****************************************************************************/
static void FDKaacEnc_CalcSfbTonality(FIXP_DBL *RESTRICT spectrum,
                                      INT *RESTRICT sfbMaxScaleSpec,
                                      FIXP_DBL *RESTRICT chaosMeasure,
                                      FIXP_SGL *RESTRICT sfbTonality,
                                      INT sfbCnt, const INT *RESTRICT sfbOffset,
                                      FIXP_DBL *RESTRICT sfbEnergyLD64) {
  INT i;

  for (i = 0; i < sfbCnt; i++) {
    FIXP_DBL chaosMeasureSfbLD64;
    INT shiftBits =
        fixMax(0, sfbMaxScaleSpec[i] -
                      4); /* max sfbWidth = 96 ; 2^7=128 => 7/2 = 4 (spc*spc) */

    INT j;
    FIXP_DBL chaosMeasureSfb = FL2FXCONST_DBL(0.0);

    /* calc chaosMeasurePerSfb */
    for (j = (sfbOffset[i + 1] - sfbOffset[i]) - 1; j >= 0; j--) {
      FIXP_DBL tmp = (*spectrum++) << shiftBits;
      FIXP_DBL lineNrg = fMultDiv2(tmp, tmp);
      chaosMeasureSfb = fMultAddDiv2(chaosMeasureSfb, lineNrg, *chaosMeasure++);
    }

    /* calc tonalityPerSfb */
    if (chaosMeasureSfb != FL2FXCONST_DBL(0.0)) {
      /* add ld(convtone)/64 and 2/64 bec.fMultDiv2 */
      chaosMeasureSfbLD64 = CalcLdData((chaosMeasureSfb)) - sfbEnergyLD64[i];
      chaosMeasureSfbLD64 += FL2FXCONST_DBL(3.0f / 64) -
                             ((FIXP_DBL)(shiftBits) << (DFRACT_BITS - 6));

      if (chaosMeasureSfbLD64 >
          FL2FXCONST_DBL(-0.0519051)) /* > ld(0.05)+ld(2) */
      {
        if (chaosMeasureSfbLD64 <= FL2FXCONST_DBL(0.0))
          sfbTonality[i] =
              FX_DBL2FX_SGL(fMultDiv2(chaosMeasureSfbLD64, normlog) << 7);
        else
          sfbTonality[i] = FL2FXCONST_SGL(0.0);
      } else
        sfbTonality[i] = (FIXP_SGL)MAXVAL_SGL;
    } else
      sfbTonality[i] = (FIXP_SGL)MAXVAL_SGL;
  }
}
