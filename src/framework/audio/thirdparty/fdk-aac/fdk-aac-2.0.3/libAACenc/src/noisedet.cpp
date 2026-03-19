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

   Author(s):   M. Lohwasser

   Description: noisedet.c
                Routines for Noise Detection

*******************************************************************************/

#include "noisedet.h"

#include "aacenc_pns.h"
#include "pnsparam.h"

/*****************************************************************************

    functionname: FDKaacEnc_fuzzyIsSmaller
    description:  Fuzzy value calculation for "testVal is smaller than refVal"
    returns:      fuzzy value
    input:        test and ref Value,
                  low and high Lim
    output:       return fuzzy value

*****************************************************************************/
static FIXP_SGL FDKaacEnc_fuzzyIsSmaller(FIXP_DBL testVal, FIXP_DBL refVal,
                                         FIXP_DBL loLim, FIXP_DBL hiLim) {
  if (refVal <= FL2FXCONST_DBL(0.0))
    return (FL2FXCONST_SGL(0.0f));
  else if (testVal >= fMult((hiLim >> 1) + (loLim >> 1), refVal))
    return (FL2FXCONST_SGL(0.0f));
  else
    return ((FIXP_SGL)MAXVAL_SGL);
}

/*****************************************************************************

    functionname: FDKaacEnc_noiseDetect
    description:  detect tonal sfb's; two tests
                  Powerdistribution:
                    sfb splittet in four regions,
                    compare the energy in all sections
                  PsychTonality:
                    compare tonality from chaosmeasure with reftonality
    returns:
    input:        spectrum of one large mdct
                  number of sfb's
                  pointer to offset of sfb's
                  pointer to noiseFuzzyMeasure (modified)
                  noiseparams struct
                  pointer to sfb energies
                  pointer to tonality calculated in chaosmeasure
    output:       noiseFuzzy Measure

*****************************************************************************/

void FDKaacEnc_noiseDetect(FIXP_DBL *RESTRICT mdctSpectrum,
                           INT *RESTRICT sfbMaxScaleSpec, INT sfbActive,
                           const INT *RESTRICT sfbOffset,
                           FIXP_SGL *RESTRICT noiseFuzzyMeasure,
                           NOISEPARAMS *np, FIXP_SGL *RESTRICT sfbtonality)

{
  int i, k, sfb, sfbWidth;
  FIXP_SGL fuzzy, fuzzyTotal;
  FIXP_DBL refVal, testVal;

  /***** Start detection phase *****/
  /* Start noise detection for each band based on a number of checks */
  for (sfb = 0; sfb < sfbActive; sfb++) {
    fuzzyTotal = (FIXP_SGL)MAXVAL_SGL;
    sfbWidth = sfbOffset[sfb + 1] - sfbOffset[sfb];

    /* Reset output for lower bands or too small bands */
    if (sfb < np->startSfb || sfbWidth < np->minSfbWidth) {
      noiseFuzzyMeasure[sfb] = FL2FXCONST_SGL(0.0f);
      continue;
    }

    if ((np->detectionAlgorithmFlags & USE_POWER_DISTRIBUTION) &&
        (fuzzyTotal > FL2FXCONST_SGL(0.5f))) {
      FIXP_DBL fhelp1, fhelp2, fhelp3, fhelp4, maxVal, minVal;
      INT leadingBits = fixMax(
          0, (sfbMaxScaleSpec[sfb] -
              3)); /* max sfbWidth = 96/4 ; 2^5=32 => 5/2 = 3 (spc*spc) */

      /*  check power distribution in four regions */
      fhelp1 = fhelp2 = fhelp3 = fhelp4 = FL2FXCONST_DBL(0.0f);
      k = sfbWidth >> 2; /* Width of a quarter band */

      for (i = sfbOffset[sfb]; i < sfbOffset[sfb] + k; i++) {
        fhelp1 = fPow2AddDiv2(fhelp1, mdctSpectrum[i] << leadingBits);
        fhelp2 = fPow2AddDiv2(fhelp2, mdctSpectrum[i + k] << leadingBits);
        fhelp3 = fPow2AddDiv2(fhelp3, mdctSpectrum[i + 2 * k] << leadingBits);
        fhelp4 = fPow2AddDiv2(fhelp4, mdctSpectrum[i + 3 * k] << leadingBits);
      }

      /* get max into fhelp: */
      maxVal = fixMax(fhelp1, fhelp2);
      maxVal = fixMax(maxVal, fhelp3);
      maxVal = fixMax(maxVal, fhelp4);

      /* get min into fhelp1: */
      minVal = fixMin(fhelp1, fhelp2);
      minVal = fixMin(minVal, fhelp3);
      minVal = fixMin(minVal, fhelp4);

      /* Normalize min and max Val */
      leadingBits = CountLeadingBits(maxVal);
      testVal = maxVal << leadingBits;
      refVal = minVal << leadingBits;

      /* calculate fuzzy value for power distribution */
      testVal = fMultDiv2(testVal, np->powDistPSDcurve[sfb]);

      fuzzy = FDKaacEnc_fuzzyIsSmaller(
          testVal,                /* 1/2 * maxValue * PSDcurve */
          refVal,                 /* 1   * minValue            */
          FL2FXCONST_DBL(0.495),  /* 1/2 * loLim  (0.99f/2)    */
          FL2FXCONST_DBL(0.505)); /* 1/2 * hiLim  (1.01f/2)    */

      fuzzyTotal = fixMin(fuzzyTotal, fuzzy);
    }

    if ((np->detectionAlgorithmFlags & USE_PSYCH_TONALITY) &&
        (fuzzyTotal > FL2FXCONST_SGL(0.5f))) {
      /* Detection with tonality-value of psych. acoustic (here: 1 is tonal!)*/

      testVal = FX_SGL2FX_DBL(sfbtonality[sfb]) >> 1; /* 1/2 * sfbTonality */
      refVal = np->refTonality;

      fuzzy = FDKaacEnc_fuzzyIsSmaller(
          testVal, refVal, FL2FXCONST_DBL(0.45f), /* 1/2 * loLim  (0.9f/2) */
          FL2FXCONST_DBL(0.55f)); /* 1/2 * hiLim  (1.1f/2)     */

      fuzzyTotal = fixMin(fuzzyTotal, fuzzy);
    }

    /* Output of final result */
    noiseFuzzyMeasure[sfb] = fuzzyTotal;
  }
}
