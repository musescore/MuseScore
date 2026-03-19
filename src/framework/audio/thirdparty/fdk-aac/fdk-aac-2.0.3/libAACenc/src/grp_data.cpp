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

   Description: Short block grouping

*******************************************************************************/

#include "psy_const.h"
#include "interface.h"

/*
 * this routine does not work in-place
 */

/*
 * Don't use fAddSaturate2() because it looses one bit accuracy which is
 * usefull for quality.
 */
static inline FIXP_DBL nrgAddSaturate(const FIXP_DBL a, const FIXP_DBL b) {
  return ((a >= (FIXP_DBL)MAXVAL_DBL - b) ? (FIXP_DBL)MAXVAL_DBL : (a + b));
}

void FDKaacEnc_groupShortData(FIXP_DBL *mdctSpectrum,      /* in-out      */
                              SFB_THRESHOLD *sfbThreshold, /* in-out */
                              SFB_ENERGY *sfbEnergy,       /* in-out       */
                              SFB_ENERGY *sfbEnergyMS,     /* in-out     */
                              SFB_ENERGY *sfbSpreadEnergy, const INT sfbCnt,
                              const INT sfbActive, const INT *sfbOffset,
                              const FIXP_DBL *sfbMinSnrLdData,
                              INT *groupedSfbOffset, /* out */
                              INT *maxSfbPerGroup,   /* out */
                              FIXP_DBL *groupedSfbMinSnrLdData,
                              const INT noOfGroups, const INT *groupLen,
                              const INT granuleLength) {
  INT i, j;
  INT line;   /* counts through lines              */
  INT sfb;    /* counts through scalefactor bands  */
  INT grp;    /* counts through groups             */
  INT wnd;    /* counts through windows in a group */
  INT offset; /* needed in sfbOffset grouping      */
  INT highestSfb;
  INT granuleLength_short = granuleLength / TRANS_FAC;

  C_ALLOC_SCRATCH_START(tmpSpectrum, FIXP_DBL, (1024))

  /* for short blocks: regroup spectrum and */
  /* group energies and thresholds according to grouping */

  /* calculate maxSfbPerGroup */
  highestSfb = 0;
  for (wnd = 0; wnd < TRANS_FAC; wnd++) {
    for (sfb = sfbActive - 1; sfb >= highestSfb; sfb--) {
      for (line = sfbOffset[sfb + 1] - 1; line >= sfbOffset[sfb]; line--) {
        if (mdctSpectrum[wnd * granuleLength_short + line] !=
            FL2FXCONST_SPC(0.0))
          break; /* this band is not completely zero */
      }
      if (line >= sfbOffset[sfb]) break; /* this band was not completely zero */
    }
    highestSfb = fixMax(highestSfb, sfb);
  }
  highestSfb = highestSfb > 0 ? highestSfb : 0;
  *maxSfbPerGroup = highestSfb + 1;

  /* calculate groupedSfbOffset */
  i = 0;
  offset = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive + 1; sfb++) {
      groupedSfbOffset[i++] = offset + sfbOffset[sfb] * groupLen[grp];
    }
    i += sfbCnt - sfb;
    offset += groupLen[grp] * granuleLength_short;
  }
  groupedSfbOffset[i++] = granuleLength;

  /* calculate groupedSfbMinSnr */
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      groupedSfbMinSnrLdData[i++] = sfbMinSnrLdData[sfb];
    }
    i += sfbCnt - sfb;
  }

  /* sum up sfbThresholds */
  wnd = 0;
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      FIXP_DBL thresh = sfbThreshold->Short[wnd][sfb];
      for (j = 1; j < groupLen[grp]; j++) {
        thresh = nrgAddSaturate(thresh, sfbThreshold->Short[wnd + j][sfb]);
      }
      sfbThreshold->Long[i++] = thresh;
    }
    i += sfbCnt - sfb;
    wnd += groupLen[grp];
  }

  /* sum up sfbEnergies left/right */
  wnd = 0;
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      FIXP_DBL energy = sfbEnergy->Short[wnd][sfb];
      for (j = 1; j < groupLen[grp]; j++) {
        energy = nrgAddSaturate(energy, sfbEnergy->Short[wnd + j][sfb]);
      }
      sfbEnergy->Long[i++] = energy;
    }
    i += sfbCnt - sfb;
    wnd += groupLen[grp];
  }

  /* sum up sfbEnergies mid/side */
  wnd = 0;
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      FIXP_DBL energy = sfbEnergyMS->Short[wnd][sfb];
      for (j = 1; j < groupLen[grp]; j++) {
        energy = nrgAddSaturate(energy, sfbEnergyMS->Short[wnd + j][sfb]);
      }
      sfbEnergyMS->Long[i++] = energy;
    }
    i += sfbCnt - sfb;
    wnd += groupLen[grp];
  }

  /* sum up sfbSpreadEnergies */
  wnd = 0;
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      FIXP_DBL energy = sfbSpreadEnergy->Short[wnd][sfb];
      for (j = 1; j < groupLen[grp]; j++) {
        energy = nrgAddSaturate(energy, sfbSpreadEnergy->Short[wnd + j][sfb]);
      }
      sfbSpreadEnergy->Long[i++] = energy;
    }
    i += sfbCnt - sfb;
    wnd += groupLen[grp];
  }

  /* re-group spectrum */
  wnd = 0;
  i = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbActive; sfb++) {
      int width = sfbOffset[sfb + 1] - sfbOffset[sfb];
      FIXP_DBL *pMdctSpectrum =
          &mdctSpectrum[sfbOffset[sfb]] + wnd * granuleLength_short;
      for (j = 0; j < groupLen[grp]; j++) {
        FIXP_DBL *pTmp = pMdctSpectrum;
        for (line = width; line > 0; line--) {
          tmpSpectrum[i++] = *pTmp++;
        }
        pMdctSpectrum += granuleLength_short;
      }
    }
    i += (groupLen[grp] * (sfbOffset[sfbCnt] - sfbOffset[sfb]));
    wnd += groupLen[grp];
  }

  FDKmemcpy(mdctSpectrum, tmpSpectrum, granuleLength * sizeof(FIXP_DBL));

  C_ALLOC_SCRATCH_END(tmpSpectrum, FIXP_DBL, (1024))
}
