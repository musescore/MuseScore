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

   Description: Psychoaccoustic data

*******************************************************************************/

#ifndef PSY_DATA_H
#define PSY_DATA_H

#include "block_switch.h"
#include "mdct.h"

/* Be careful with MAX_SFB_LONG as length of the .Long arrays.
 * sfbEnergy.Long and sfbEnergyMS.Long and sfbThreshold.Long are used as a
 * temporary storage for the regrouped short energies and thresholds between
 * FDKaacEnc_groupShortData() and BuildInterface() in FDKaacEnc_psyMain(). The
 * space required for this is MAX_GROUPED_SFB ( = MAX_NO_OF_GROUPS*MAX_SFB_SHORT
 * ). However, this is not important if unions are used (which is not possible
 * with pfloat). */

typedef shouldBeUnion {
  FIXP_DBL Long[MAX_GROUPED_SFB];
  FIXP_DBL Short[TRANS_FAC][MAX_SFB_SHORT];
}
SFB_THRESHOLD;

typedef shouldBeUnion {
  FIXP_DBL Long[MAX_GROUPED_SFB];
  FIXP_DBL Short[TRANS_FAC][MAX_SFB_SHORT];
}
SFB_ENERGY;

typedef shouldBeUnion {
  FIXP_DBL Long[MAX_GROUPED_SFB];
  FIXP_DBL Short[TRANS_FAC][MAX_SFB_SHORT];
}
SFB_LD_ENERGY;

typedef shouldBeUnion {
  INT Long[MAX_GROUPED_SFB];
  INT Short[TRANS_FAC][MAX_SFB_SHORT];
}
SFB_MAX_SCALE;

typedef struct {
  INT_PCM* psyInputBuffer;
  FIXP_DBL overlapAddBuffer[3 * 512 / 2];

  mdct_t mdctPers;                               /* MDCT persistent data */
  BLOCK_SWITCHING_CONTROL blockSwitchingControl; /* block switching */
  FIXP_DBL sfbThresholdnm1[MAX_SFB];             /* FDKaacEnc_PreEchoControl */
  INT mdctScalenm1; /* scale of last block's mdct (FDKaacEnc_PreEchoControl) */
  INT calcPreEcho;
  INT isLFE;
} PSY_STATIC;

typedef struct {
  FIXP_DBL* mdctSpectrum;
  SFB_THRESHOLD sfbThreshold;    /* adapt                                  */
  SFB_ENERGY sfbEnergy;          /* sfb energies                           */
  SFB_LD_ENERGY sfbEnergyLdData; /* sfb energies in ldData format          */
  SFB_MAX_SCALE sfbMaxScaleSpec;
  SFB_ENERGY sfbEnergyMS; /* mid/side sfb energies                  */
  FIXP_DBL sfbEnergyMSLdData[MAX_GROUPED_SFB]; /* mid/side sfb energies in
                                                  ldData format */
  SFB_ENERGY sfbSpreadEnergy;
  INT mdctScale; /* exponent of data in mdctSpectrum       */
  INT groupedSfbOffset[MAX_GROUPED_SFB + 1];
  INT sfbActive;
  INT lowpassLine;
} PSY_DATA;

#endif /* PSY_DATA_H */
