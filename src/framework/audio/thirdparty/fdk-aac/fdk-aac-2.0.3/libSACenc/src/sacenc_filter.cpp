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

/*********************** MPEG surround encoder library *************************

   Author(s):   M. Multrus

   Description: Encoder Library
                Filter functions

*******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_filter.h"

/* Defines *******************************************************************/

/* Data Types ****************************************************************/
typedef struct T_DC_FILTER {
  FIXP_DBL c__FDK;
  FIXP_DBL state__FDK;

} DC_FILTER;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
FDK_SACENC_ERROR fdk_sacenc_createDCFilter(HANDLE_DC_FILTER *hDCFilter) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == hDCFilter) {
    error = SACENC_INVALID_HANDLE;
  } else {
    FDK_ALLOCATE_MEMORY_1D(*hDCFilter, 1, DC_FILTER);
  }
  return error;

bail:
  fdk_sacenc_destroyDCFilter(hDCFilter);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

FDK_SACENC_ERROR fdk_sacenc_initDCFilter(HANDLE_DC_FILTER hDCFilter,
                                         const UINT sampleRate) {
  FDK_SACENC_ERROR error = SACENC_OK;

  FIXP_DBL expC;
  int s;

  /* Conversion for use of CalcInvLdData: e^x = 2^(x*log10(e)/log10(2) =
     CalcInvLdData(x*log10(e)/log10(2)/64.0) 1.44269504089 = log10(e)/log10(2)
     0.5           = scale constant value with 1 Bits
  */
  expC = fDivNormHighPrec((FIXP_DBL)20, (FIXP_DBL)sampleRate, &s);
  expC = fMultDiv2(FL2FXCONST_DBL(-1.44269504089 * 0.5), expC) >>
         (LD_DATA_SHIFT - 1 - 1);

  if (s < 0)
    expC = expC >> (-s);
  else
    expC = expC << (s);

  expC = CalcInvLdData(expC);

  hDCFilter->c__FDK = expC;
  hDCFilter->state__FDK = FL2FXCONST_DBL(0.0f);

  return error;
}

FDK_SACENC_ERROR fdk_sacenc_destroyDCFilter(HANDLE_DC_FILTER *hDCFilter) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hDCFilter != NULL) && (*hDCFilter != NULL)) {
    FDKfree(*hDCFilter);

    *hDCFilter = NULL;
  }

  return error;
}

FDK_SACENC_ERROR fdk_sacenc_applyDCFilter(HANDLE_DC_FILTER hDCFilter,
                                          const INT_PCM *const signalIn,
                                          INT_PCM *const signalOut,
                                          const INT signalLength) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hDCFilter == NULL) || (signalIn == NULL) || (signalOut == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    const INT_PCM *const x = signalIn;
    INT_PCM *const y = signalOut;
    const FIXP_DBL c = hDCFilter->c__FDK;
    FIXP_DBL *const state = &hDCFilter->state__FDK;
    int i;
    FIXP_DBL x0, x1, y1;

    x1 = x0 = FX_PCM2FX_DBL(x[0]) >> DC_FILTER_SF;
    y1 = x0 + (*state);

    for (i = 1; i < signalLength; i++) {
      x0 = FX_PCM2FX_DBL(x[i]) >> DC_FILTER_SF;
      y[i - 1] = FX_DBL2FX_PCM(y1);
      y1 = x0 - x1 + fMult(c, y1);
      x1 = x0;
    }

    *state = fMult(c, y1) - x1;
    y[i - 1] = FX_DBL2FX_PCM(y1);
  }

  return error;
}
