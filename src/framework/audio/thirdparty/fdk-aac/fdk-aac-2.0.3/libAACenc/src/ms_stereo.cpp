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

   Author(s):   M.Werner

   Description: MS stereo processing

*******************************************************************************/

#include "ms_stereo.h"

#include "psy_const.h"

/* static const float scaleMinThres = 1.0f; */ /* 0.75f for 3db boost */

void FDKaacEnc_MsStereoProcessing(PSY_DATA *RESTRICT psyData[(2)],
                                  PSY_OUT_CHANNEL *psyOutChannel[2],
                                  const INT *isBook, INT *msDigest, /* output */
                                  INT *msMask,                      /* output */
                                  const INT allowMS, const INT sfbCnt,
                                  const INT sfbPerGroup,
                                  const INT maxSfbPerGroup,
                                  const INT *sfbOffset) {
  FIXP_DBL *sfbEnergyLeft =
      psyData[0]->sfbEnergy.Long; /* modified where msMask==1 */
  FIXP_DBL *sfbEnergyRight =
      psyData[1]->sfbEnergy.Long; /* modified where msMask==1 */
  const FIXP_DBL *sfbEnergyMid = psyData[0]->sfbEnergyMS.Long;
  const FIXP_DBL *sfbEnergySide = psyData[1]->sfbEnergyMS.Long;
  FIXP_DBL *sfbThresholdLeft =
      psyData[0]->sfbThreshold.Long; /* modified where msMask==1 */
  FIXP_DBL *sfbThresholdRight =
      psyData[1]->sfbThreshold.Long; /* modified where msMask==1 */

  FIXP_DBL *sfbSpreadEnLeft = psyData[0]->sfbSpreadEnergy.Long;
  FIXP_DBL *sfbSpreadEnRight = psyData[1]->sfbSpreadEnergy.Long;

  FIXP_DBL *sfbEnergyLeftLdData =
      psyOutChannel[0]->sfbEnergyLdData; /* modified where msMask==1 */
  FIXP_DBL *sfbEnergyRightLdData =
      psyOutChannel[1]->sfbEnergyLdData; /* modified where msMask==1 */
  FIXP_DBL *sfbEnergyMidLdData = psyData[0]->sfbEnergyMSLdData;
  FIXP_DBL *sfbEnergySideLdData = psyData[1]->sfbEnergyMSLdData;
  FIXP_DBL *sfbThresholdLeftLdData =
      psyOutChannel[0]->sfbThresholdLdData; /* modified where msMask==1 */
  FIXP_DBL *sfbThresholdRightLdData =
      psyOutChannel[1]->sfbThresholdLdData; /* modified where msMask==1 */

  FIXP_DBL *mdctSpectrumLeft =
      psyData[0]->mdctSpectrum; /* modified where msMask==1 */
  FIXP_DBL *mdctSpectrumRight =
      psyData[1]->mdctSpectrum; /* modified where msMask==1 */

  INT sfb, sfboffs, j; /* loop counters         */
  FIXP_DBL pnlrLdData, pnmsLdData;
  FIXP_DBL minThresholdLdData;
  FIXP_DBL minThreshold;
  INT useMS;

  INT msMaskTrueSomewhere = 0; /* to determine msDigest */
  INT numMsMaskFalse =
      0; /* number of non-intensity bands where L/R coding is used */

  for (sfb = 0; sfb < sfbCnt; sfb += sfbPerGroup) {
    for (sfboffs = 0; sfboffs < maxSfbPerGroup; sfboffs++) {
      if ((isBook == NULL) ? 1 : (isBook[sfb + sfboffs] == 0)) {
        FIXP_DBL tmp;

        /*
                  minThreshold=min(sfbThresholdLeft[sfb+sfboffs],
           sfbThresholdRight[sfb+sfboffs])*scaleMinThres; pnlr =
           (sfbThresholdLeft[sfb+sfboffs]/
                         max(sfbEnergyLeft[sfb+sfboffs],sfbThresholdLeft[sfb+sfboffs]))*
                         (sfbThresholdRight[sfb+sfboffs]/
                         max(sfbEnergyRight[sfb+sfboffs],sfbThresholdRight[sfb+sfboffs]));
                  pnms =
           (minThreshold/max(sfbEnergyMid[sfb+sfboffs],minThreshold))*
                         (minThreshold/max(sfbEnergySide[sfb+sfboffs],minThreshold));
                  useMS = (pnms > pnlr);
        */

        /* we assume that scaleMinThres == 1.0f and we can drop it */
        minThresholdLdData = fixMin(sfbThresholdLeftLdData[sfb + sfboffs],
                                    sfbThresholdRightLdData[sfb + sfboffs]);

        /* pnlrLdData = sfbThresholdLeftLdData[sfb+sfboffs] -
                      max(sfbEnergyLeftLdData[sfb+sfboffs],
           sfbThresholdLeftLdData[sfb+sfboffs]) +
                      sfbThresholdRightLdData[sfb+sfboffs] -
                      max(sfbEnergyRightLdData[sfb+sfboffs],
           sfbThresholdRightLdData[sfb+sfboffs]); */
        tmp = fixMax(sfbEnergyLeftLdData[sfb + sfboffs],
                     sfbThresholdLeftLdData[sfb + sfboffs]);
        pnlrLdData = (sfbThresholdLeftLdData[sfb + sfboffs] >> 1) - (tmp >> 1);
        pnlrLdData = pnlrLdData + (sfbThresholdRightLdData[sfb + sfboffs] >> 1);
        tmp = fixMax(sfbEnergyRightLdData[sfb + sfboffs],
                     sfbThresholdRightLdData[sfb + sfboffs]);
        pnlrLdData = pnlrLdData - (tmp >> 1);

        /* pnmsLdData = minThresholdLdData -
           max(sfbEnergyMidLdData[sfb+sfboffs], minThresholdLdData) +
                      minThresholdLdData - max(sfbEnergySideLdData[sfb+sfboffs],
           minThresholdLdData); */
        tmp = fixMax(sfbEnergyMidLdData[sfb + sfboffs], minThresholdLdData);
        pnmsLdData = minThresholdLdData - (tmp >> 1);
        tmp = fixMax(sfbEnergySideLdData[sfb + sfboffs], minThresholdLdData);
        pnmsLdData = pnmsLdData - (tmp >> 1);
        useMS = ((allowMS != 0) && (pnmsLdData > pnlrLdData)) ? 1 : 0;

        if (useMS) {
          msMask[sfb + sfboffs] = 1;
          msMaskTrueSomewhere = 1;
          for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
               j++) {
            FIXP_DBL specL, specR;
            specL = mdctSpectrumLeft[j] >> 1;
            specR = mdctSpectrumRight[j] >> 1;
            mdctSpectrumLeft[j] = specL + specR;
            mdctSpectrumRight[j] = specL - specR;
          }
          minThreshold = fixMin(sfbThresholdLeft[sfb + sfboffs],
                                sfbThresholdRight[sfb + sfboffs]);
          sfbThresholdLeft[sfb + sfboffs] = sfbThresholdRight[sfb + sfboffs] =
              minThreshold;
          sfbThresholdLeftLdData[sfb + sfboffs] =
              sfbThresholdRightLdData[sfb + sfboffs] = minThresholdLdData;
          sfbEnergyLeft[sfb + sfboffs] = sfbEnergyMid[sfb + sfboffs];
          sfbEnergyRight[sfb + sfboffs] = sfbEnergySide[sfb + sfboffs];
          sfbEnergyLeftLdData[sfb + sfboffs] =
              sfbEnergyMidLdData[sfb + sfboffs];
          sfbEnergyRightLdData[sfb + sfboffs] =
              sfbEnergySideLdData[sfb + sfboffs];

          sfbSpreadEnLeft[sfb + sfboffs] = sfbSpreadEnRight[sfb + sfboffs] =
              fixMin(sfbSpreadEnLeft[sfb + sfboffs],
                     sfbSpreadEnRight[sfb + sfboffs]) >>
              1;

        } else {
          msMask[sfb + sfboffs] = 0;
          numMsMaskFalse++;
        } /* useMS */
      }   /* isBook */
      else {
        /* keep mDigest from IS module */
        if (msMask[sfb + sfboffs]) {
          msMaskTrueSomewhere = 1;
        }
        /* prohibit MS_MASK_ALL in combination with IS */
        numMsMaskFalse = 9;
      } /* isBook */
    }   /* sfboffs */
  }     /* sfb */

  if (msMaskTrueSomewhere == 1) {
    if ((numMsMaskFalse == 0) ||
        ((numMsMaskFalse < maxSfbPerGroup) && (numMsMaskFalse < 9))) {
      *msDigest = SI_MS_MASK_ALL;
      /* loop through M/S bands; if msMask==0, set it to 1 and apply M/S */
      for (sfb = 0; sfb < sfbCnt; sfb += sfbPerGroup) {
        for (sfboffs = 0; sfboffs < maxSfbPerGroup; sfboffs++) {
          if (((isBook == NULL) ? 1 : (isBook[sfb + sfboffs] == 0)) &&
              (msMask[sfb + sfboffs] == 0)) {
            msMask[sfb + sfboffs] = 1;
            /* apply M/S coding */
            for (j = sfbOffset[sfb + sfboffs]; j < sfbOffset[sfb + sfboffs + 1];
                 j++) {
              FIXP_DBL specL, specR;
              specL = mdctSpectrumLeft[j] >> 1;
              specR = mdctSpectrumRight[j] >> 1;
              mdctSpectrumLeft[j] = specL + specR;
              mdctSpectrumRight[j] = specL - specR;
            }
            minThreshold = fixMin(sfbThresholdLeft[sfb + sfboffs],
                                  sfbThresholdRight[sfb + sfboffs]);
            sfbThresholdLeft[sfb + sfboffs] = sfbThresholdRight[sfb + sfboffs] =
                minThreshold;
            minThresholdLdData = fixMin(sfbThresholdLeftLdData[sfb + sfboffs],
                                        sfbThresholdRightLdData[sfb + sfboffs]);
            sfbThresholdLeftLdData[sfb + sfboffs] =
                sfbThresholdRightLdData[sfb + sfboffs] = minThresholdLdData;
            sfbEnergyLeft[sfb + sfboffs] = sfbEnergyMid[sfb + sfboffs];
            sfbEnergyRight[sfb + sfboffs] = sfbEnergySide[sfb + sfboffs];
            sfbEnergyLeftLdData[sfb + sfboffs] =
                sfbEnergyMidLdData[sfb + sfboffs];
            sfbEnergyRightLdData[sfb + sfboffs] =
                sfbEnergySideLdData[sfb + sfboffs];

            sfbSpreadEnLeft[sfb + sfboffs] = sfbSpreadEnRight[sfb + sfboffs] =
                fixMin(sfbSpreadEnLeft[sfb + sfboffs],
                       sfbSpreadEnRight[sfb + sfboffs]) >>
                1;
          }
        }
      }
    } else {
      *msDigest = SI_MS_MASK_SOME;
    }
  } else {
    *msDigest = SI_MS_MASK_NONE;
  }
}
