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

/******************* MPEG transport format encoder library *********************

   Author(s):

   Description: ADIF Transport Headers writing

*******************************************************************************/

#include "tpenc_adif.h"

#include "tpenc_lib.h"
#include "tpenc_asc.h"

int adifWrite_EncodeHeader(ADIF_INFO *adif, HANDLE_FDK_BITSTREAM hBs,
                           INT adif_buffer_fullness) {
  /* ADIF/PCE/ADTS definitions */
  const char adifId[5] = "ADIF";
  const int copyRightIdPresent = 0;
  const int originalCopy = 0;
  const int home = 0;
  int err = 0;

  int i;

  INT totalBitRate = adif->bitRate;

  if (adif->headerWritten) return 0;

  /* Align inside PCE with respect to the first bit of the header */
  UINT alignAnchor = FDKgetValidBits(hBs);

  /* Signal variable bitrate if buffer fullnes exceeds 20 bit */
  adif->bVariableRate = (adif_buffer_fullness >= (INT)(0x1 << 20)) ? 1 : 0;

  FDKwriteBits(hBs, adifId[0], 8);
  FDKwriteBits(hBs, adifId[1], 8);
  FDKwriteBits(hBs, adifId[2], 8);
  FDKwriteBits(hBs, adifId[3], 8);

  FDKwriteBits(hBs, copyRightIdPresent ? 1 : 0, 1);

  if (copyRightIdPresent) {
    for (i = 0; i < 72; i++) {
      FDKwriteBits(hBs, 0, 1);
    }
  }
  FDKwriteBits(hBs, originalCopy ? 1 : 0, 1);
  FDKwriteBits(hBs, home ? 1 : 0, 1);
  FDKwriteBits(hBs, adif->bVariableRate ? 1 : 0, 1);
  FDKwriteBits(hBs, totalBitRate, 23);

  /* we write only one PCE at the moment */
  FDKwriteBits(hBs, 0, 4);

  if (!adif->bVariableRate) {
    FDKwriteBits(hBs, adif_buffer_fullness, 20);
  }
  /* Write PCE */
  transportEnc_writePCE(hBs, adif->cm, adif->samplingRate, adif->instanceTag,
                        adif->profile, adif->matrixMixdownA,
                        (adif->pseudoSurroundEnable) ? 1 : 0, alignAnchor);

  return err;
}

int adifWrite_GetHeaderBits(ADIF_INFO *adif) {
  /* ADIF definitions */
  const int copyRightIdPresent = 0;

  if (adif->headerWritten) return 0;

  int bits = 0;

  bits += 8 * 4; /* ADIF ID */

  bits += 1; /* Copyright present */

  if (copyRightIdPresent) bits += 72; /* Copyright ID */

  bits += 26;

  bits += 4; /* Number of PCE's */

  if (!adif->bVariableRate) {
    bits += 20;
  }

  /* write PCE */
  bits = transportEnc_GetPCEBits(adif->cm, adif->matrixMixdownA, bits);

  return bits;
}
