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

   Description: Perceptual entropie module

*******************************************************************************/

#include "line_pe.h"
#include "sf_estim.h"
#include "bit_cnt.h"

#include "genericStds.h"

static const FIXP_DBL C1LdData =
    FL2FXCONST_DBL(3.0 / LD_DATA_SCALING); /* C1 = 3.0 = log(8.0)/log(2) */
static const FIXP_DBL C2LdData = FL2FXCONST_DBL(
    1.3219281 / LD_DATA_SCALING); /* C2 = 1.3219281 = log(2.5)/log(2) */
static const FIXP_DBL C3LdData = FL2FXCONST_DBL(0.5593573); /* 1-C2/C1 */

/* constants that do not change during successive pe calculations */
void FDKaacEnc_prepareSfbPe(PE_CHANNEL_DATA *RESTRICT const peChanData,
                            const FIXP_DBL *RESTRICT const sfbEnergyLdData,
                            const FIXP_DBL *RESTRICT const sfbThresholdLdData,
                            const FIXP_DBL *RESTRICT const sfbFormFactorLdData,
                            const INT *RESTRICT const sfbOffset,
                            const INT sfbCnt, const INT sfbPerGroup,
                            const INT maxSfbPerGroup) {
  INT sfbGrp, sfb;
  INT sfbWidth;
  FIXP_DBL avgFormFactorLdData;
  const FIXP_DBL formFacScaling =
      FL2FXCONST_DBL((float)FORM_FAC_SHIFT / LD_DATA_SCALING);

  for (sfbGrp = 0; sfbGrp < sfbCnt; sfbGrp += sfbPerGroup) {
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      if ((FIXP_DBL)sfbEnergyLdData[sfbGrp + sfb] >
          (FIXP_DBL)sfbThresholdLdData[sfbGrp + sfb]) {
        sfbWidth = sfbOffset[sfbGrp + sfb + 1] - sfbOffset[sfbGrp + sfb];
        /* estimate number of active lines */
        avgFormFactorLdData = ((-sfbEnergyLdData[sfbGrp + sfb] >> 1) +
                               (CalcLdInt(sfbWidth) >> 1)) >>
                              1;
        peChanData->sfbNLines[sfbGrp + sfb] = (INT)CalcInvLdData(
            (sfbFormFactorLdData[sfbGrp + sfb] + formFacScaling) +
            avgFormFactorLdData);
        /* Make sure sfbNLines is never greater than sfbWidth due to
         * unaccuracies (e.g. sfbEnergyLdData[sfbGrp+sfb] = 0x80000000) */
        peChanData->sfbNLines[sfbGrp + sfb] =
            fMin(sfbWidth, peChanData->sfbNLines[sfbGrp + sfb]);
      } else {
        peChanData->sfbNLines[sfbGrp + sfb] = 0;
      }
    }
  }
}

/*
   formula for one sfb:
   pe = n * ld(en/thr),                if ld(en/thr) >= C1
   pe = n * (C2 + C3 * ld(en/thr)),    if ld(en/thr) <  C1
   n: estimated number of lines in sfb,
   ld(x) = log(x)/log(2)

   constPart is sfbPe without the threshold part n*ld(thr) or n*C3*ld(thr)
*/
void FDKaacEnc_calcSfbPe(PE_CHANNEL_DATA *RESTRICT const peChanData,
                         const FIXP_DBL *RESTRICT const sfbEnergyLdData,
                         const FIXP_DBL *RESTRICT const sfbThresholdLdData,
                         const INT sfbCnt, const INT sfbPerGroup,
                         const INT maxSfbPerGroup,
                         const INT *RESTRICT const isBook,
                         const INT *RESTRICT const isScale) {
  INT sfbGrp, sfb, thisSfb;
  INT nLines;
  FIXP_DBL logDataRatio;
  FIXP_DBL scaleLd = (FIXP_DBL)0;
  INT lastValIs = 0;

  FIXP_DBL pe = 0;
  FIXP_DBL constPart = 0;
  FIXP_DBL nActiveLines = 0;

  FIXP_DBL tmpPe, tmpConstPart, tmpNActiveLines;

  for (sfbGrp = 0; sfbGrp < sfbCnt; sfbGrp += sfbPerGroup) {
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      tmpPe = (FIXP_DBL)0;
      tmpConstPart = (FIXP_DBL)0;
      tmpNActiveLines = (FIXP_DBL)0;

      thisSfb = sfbGrp + sfb;

      if (sfbEnergyLdData[thisSfb] > sfbThresholdLdData[thisSfb]) {
        logDataRatio = sfbEnergyLdData[thisSfb] - sfbThresholdLdData[thisSfb];
        nLines = peChanData->sfbNLines[thisSfb];

        FIXP_DBL factor = nLines << (LD_DATA_SHIFT + PE_CONSTPART_SHIFT + 1);
        if (logDataRatio >= C1LdData) {
          /* scale sfbPe and sfbConstPart with PE_CONSTPART_SHIFT */
          tmpPe = fMultDiv2(logDataRatio, factor);
          tmpConstPart = fMultDiv2(sfbEnergyLdData[thisSfb] + scaleLd, factor);
        } else {
          /* scale sfbPe and sfbConstPart with PE_CONSTPART_SHIFT */
          tmpPe = fMultDiv2(
              ((FIXP_DBL)C2LdData + fMult(C3LdData, logDataRatio)), factor);
          tmpConstPart =
              fMultDiv2(((FIXP_DBL)C2LdData +
                         fMult(C3LdData, sfbEnergyLdData[thisSfb] + scaleLd)),
                        factor);

          nLines = fMultI(C3LdData, nLines);
        }
        tmpNActiveLines = (FIXP_DBL)nLines;
      } else if (isBook[thisSfb]) {
        /* provide for cost of scale factor for Intensity */
        INT delta = isScale[thisSfb] - lastValIs;
        lastValIs = isScale[thisSfb];
        peChanData->sfbPe[thisSfb] = FDKaacEnc_bitCountScalefactorDelta(delta)
                                     << PE_CONSTPART_SHIFT;
        peChanData->sfbConstPart[thisSfb] = 0;
        peChanData->sfbNActiveLines[thisSfb] = 0;
      }
      peChanData->sfbPe[thisSfb] = tmpPe;
      peChanData->sfbConstPart[thisSfb] = tmpConstPart;
      peChanData->sfbNActiveLines[thisSfb] = tmpNActiveLines;

      /* sum up peChanData values */
      pe += tmpPe;
      constPart += tmpConstPart;
      nActiveLines += tmpNActiveLines;
    }
  }

  /* correct scaled pe and constPart values */
  peChanData->pe = pe >> PE_CONSTPART_SHIFT;
  peChanData->constPart = constPart >> PE_CONSTPART_SHIFT;

  peChanData->nActiveLines = nActiveLines;
}
