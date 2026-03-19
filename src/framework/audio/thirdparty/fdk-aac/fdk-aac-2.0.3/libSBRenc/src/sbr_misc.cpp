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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Sbr miscellaneous helper functions $Revision: 36750 $
*/
#include "sbr_misc.h"

void FDKsbrEnc_Shellsort_fract(FIXP_DBL *in, INT n) {
  FIXP_DBL v;
  INT i, j;
  INT inc = 1;

  do
    inc = 3 * inc + 1;
  while (inc <= n);

  do {
    inc = inc / 3;
    for (i = inc + 1; i <= n; i++) {
      v = in[i - 1];
      j = i;
      while (in[j - inc - 1] > v) {
        in[j - 1] = in[j - inc - 1];
        j -= inc;
        if (j <= inc) break;
      }
      in[j - 1] = v;
    }
  } while (inc > 1);
}

/* Sorting routine */
void FDKsbrEnc_Shellsort_int(INT *in, INT n) {
  INT i, j, v;
  INT inc = 1;

  do
    inc = 3 * inc + 1;
  while (inc <= n);

  do {
    inc = inc / 3;
    for (i = inc + 1; i <= n; i++) {
      v = in[i - 1];
      j = i;
      while (in[j - inc - 1] > v) {
        in[j - 1] = in[j - inc - 1];
        j -= inc;
        if (j <= inc) break;
      }
      in[j - 1] = v;
    }
  } while (inc > 1);
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_AddVecLeft
 *******************************************************************************

 Description:

 Arguments:   INT* dst, INT* length_dst, INT* src, INT length_src

 Return:      none

*******************************************************************************/
void FDKsbrEnc_AddVecLeft(INT *dst, INT *length_dst, INT *src, INT length_src) {
  INT i;

  for (i = length_src - 1; i >= 0; i--)
    FDKsbrEnc_AddLeft(dst, length_dst, src[i]);
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_AddLeft
 *******************************************************************************

 Description:

 Arguments:   INT* vector, INT* length_vector, INT value

 Return:      none

*******************************************************************************/
void FDKsbrEnc_AddLeft(INT *vector, INT *length_vector, INT value) {
  INT i;

  for (i = *length_vector; i > 0; i--) vector[i] = vector[i - 1];
  vector[0] = value;
  (*length_vector)++;
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_AddRight
 *******************************************************************************

 Description:

 Arguments:   INT* vector, INT* length_vector, INT value

 Return:      none

*******************************************************************************/
void FDKsbrEnc_AddRight(INT *vector, INT *length_vector, INT value) {
  vector[*length_vector] = value;
  (*length_vector)++;
}

/*******************************************************************************
 Functionname:  FDKsbrEnc_AddVecRight
 *******************************************************************************

 Description:

 Arguments:   INT* dst, INT* length_dst, INT* src, INT length_src)

 Return:      none

*******************************************************************************/
void FDKsbrEnc_AddVecRight(INT *dst, INT *length_dst, INT *src,
                           INT length_src) {
  INT i;
  for (i = 0; i < length_src; i++) FDKsbrEnc_AddRight(dst, length_dst, src[i]);
}

/*****************************************************************************

  functionname: FDKsbrEnc_LSI_divide_scale_fract

  description:  Calculates division with best precision and scales the result.

  return:       num*scale/denom

*****************************************************************************/
FIXP_DBL FDKsbrEnc_LSI_divide_scale_fract(FIXP_DBL num, FIXP_DBL denom,
                                          FIXP_DBL scale) {
  FIXP_DBL tmp = FL2FXCONST_DBL(0.0f);
  if (num != FL2FXCONST_DBL(0.0f)) {
    INT shiftCommon;
    INT shiftNum = CountLeadingBits(num);
    INT shiftDenom = CountLeadingBits(denom);
    INT shiftScale = CountLeadingBits(scale);

    num = num << shiftNum;
    scale = scale << shiftScale;

    tmp = fMultDiv2(num, scale);

    if (denom > (tmp >> fixMin(shiftNum + shiftScale - 1, (DFRACT_BITS - 1)))) {
      denom = denom << shiftDenom;
      tmp = schur_div(tmp, denom, 15);
      shiftCommon =
          fixMin((shiftNum - shiftDenom + shiftScale - 1), (DFRACT_BITS - 1));
      if (shiftCommon < 0)
        tmp <<= -shiftCommon;
      else
        tmp >>= shiftCommon;
    } else {
      tmp = /*FL2FXCONST_DBL(1.0)*/ (FIXP_DBL)MAXVAL_DBL;
    }
  }

  return (tmp);
}
