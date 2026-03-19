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

/*********************** MPEG surround decoder library *************************

   Author(s):   Christian Ertel, Christian Griebel

   Description: SAC Dec error concealment

*******************************************************************************/

#include "sac_dec_conceal.h"

void SpatialDecConcealment_Init(SpatialDecConcealmentInfo *info,
                                const UINT resetFlags) {
  FDK_ASSERT(info != NULL);

  if (resetFlags & MPEGS_CONCEAL_RESET_STATE) {
    info->concealState = SpatialDecConcealState_Init;
    /* Frame counters will be initialized implicitely in function
     * SpatialDecConcealment_UpdateState(). */
  }

  if (resetFlags & MPEGS_CONCEAL_RESET_PARAMETER) {
    /* Set default params */
    info->concealParams.method = MPEGS_CONCEAL_DEFAULT_METHOD;
    info->concealParams.numKeepFrames = MPEGS_CONCEAL_DEFAULT_NUM_KEEP_FRAMES;
    info->concealParams.numFadeOutFrames =
        MPEGS_CONCEAL_DEFAULT_FADE_OUT_SLOPE_LENGTH;
    info->concealParams.numFadeInFrames =
        MPEGS_CONCEAL_DEFAULT_FADE_IN_SLOPE_LENGTH;
    info->concealParams.numReleaseFrames =
        MPEGS_CONCEAL_DEFAULT_NUM_RELEASE_FRAMES;
  }

  return;
}

int SpatialDecConcealment_Apply(
    SpatialDecConcealmentInfo *info,
    const SCHAR (*cmpIdxData)[MAX_PARAMETER_BANDS], SCHAR **diffIdxData,
    SCHAR *
        idxPrev, /* char
                    idxPrev[SPATIALDEC_MAX_NUM_OTT][SPATIALDEC_MAX_PARAMETER_BANDS],
                  */
    SCHAR *bsXXXDataMode, const int startBand, const int stopBand,
    const SCHAR defaultValue, const int paramType, const int numParamSets) {
  int appliedProcessing = 0;
  int band, dataMode = -1;

  FDK_ASSERT(info != NULL);
  FDK_ASSERT(cmpIdxData != NULL);
  FDK_ASSERT(idxPrev != NULL);
  FDK_ASSERT(bsXXXDataMode != NULL);

  /* Processing depends only on the internal state */
  switch (info->concealState) {
    case SpatialDecConcealState_Init:
      dataMode = 0; /* default */
      break;

    case SpatialDecConcealState_Ok:
      /* Nothing to do */
      break;

    case SpatialDecConcealState_Keep:
      dataMode = 1; /* keep */
      break;

    case SpatialDecConcealState_FadeToDefault: {
      /* Start simple fade out */
      FIXP_DBL fac = fDivNorm(info->cntStateFrames + 1,
                              info->concealParams.numFadeOutFrames + 1);

      for (band = startBand; band < stopBand; band += 1) {
        /*            idxPrev = fac * defaultValue + (1-fac) * idxPrev; */
        idxPrev[band] =
            fMultI(fac, defaultValue - idxPrev[band]) + idxPrev[band];
      }
      dataMode = 1; /* keep */
      appliedProcessing = 1;
    } break;

    case SpatialDecConcealState_Default:
      for (band = startBand; band < stopBand; band += 1) {
        idxPrev[band] = defaultValue;
      }
      dataMode = 1; /* keep */
      appliedProcessing = 1;
      break;

    case SpatialDecConcealState_FadeFromDefault: {
      FIXP_DBL fac = fDivNorm(info->cntValidFrames + 1,
                              info->concealParams.numFadeInFrames + 1);

      for (band = startBand; band < stopBand; band += 1) {
        /*            idxPrev = fac * cmpIdxData + (1-fac) * defaultValue; */
        idxPrev[band] =
            fMultI(fac, cmpIdxData[numParamSets - 1][band] - defaultValue) +
            defaultValue;
      }
      dataMode = 1; /* keep */
      appliedProcessing = 1;
    } break;

    default:
      FDK_ASSERT(0); /* All valid states shall be handled above. */
      break;
  }

  if (dataMode >= 0) {
    int i;
    for (i = 0; i < numParamSets; i += 1) {
      bsXXXDataMode[i] = dataMode;
      if (diffIdxData != NULL) {
        for (band = startBand; band < stopBand; band += 1) {
          diffIdxData[i][band] = 0;
        }
      }
    }
  }

  return appliedProcessing;
}

void SpatialDecConcealment_UpdateState(SpatialDecConcealmentInfo *info,
                                       const int frameOk) {
  FDK_ASSERT(info != NULL);

  if (frameOk) {
    info->cntValidFrames += 1;
  } else {
    info->cntValidFrames = 0;
  }

  switch (info->concealState) {
    case SpatialDecConcealState_Init:
      if (frameOk) {
        /* NEXT STATE: Ok */
        info->concealState = SpatialDecConcealState_Ok;
        info->cntStateFrames = 0;
      }
      break;

    case SpatialDecConcealState_Ok:
      if (!frameOk) {
        /* NEXT STATE: Keep */
        info->concealState = SpatialDecConcealState_Keep;
        info->cntStateFrames = 0;
      }
      break;

    case SpatialDecConcealState_Keep:
      info->cntStateFrames += 1;
      if (frameOk) {
        /* NEXT STATE: Ok */
        info->concealState = SpatialDecConcealState_Ok;
      } else {
        if (info->cntStateFrames >= info->concealParams.numKeepFrames) {
          if (info->concealParams.numFadeOutFrames == 0) {
            /* NEXT STATE: Default */
            info->concealState = SpatialDecConcealState_Default;
          } else {
            /* NEXT STATE: Fade to default */
            info->concealState = SpatialDecConcealState_FadeToDefault;
            info->cntStateFrames = 0;
          }
        }
      }
      break;

    case SpatialDecConcealState_FadeToDefault:
      info->cntStateFrames += 1;
      if (info->cntValidFrames > 0) {
        /* NEXT STATE: Fade in from default */
        info->concealState = SpatialDecConcealState_FadeFromDefault;
        info->cntStateFrames = 0;
      } else {
        if (info->cntStateFrames >= info->concealParams.numFadeOutFrames) {
          /* NEXT STATE: Default */
          info->concealState = SpatialDecConcealState_Default;
        }
      }
      break;

    case SpatialDecConcealState_Default:
      if (info->cntValidFrames > 0) {
        if (info->concealParams.numFadeInFrames == 0) {
          /* NEXT STATE: Ok */
          info->concealState = SpatialDecConcealState_Ok;
        } else {
          /* NEXT STATE: Fade in from default */
          info->concealState = SpatialDecConcealState_FadeFromDefault;
          info->cntValidFrames = 0;
        }
      }
      break;

    case SpatialDecConcealState_FadeFromDefault:
      info->cntValidFrames += 1;
      if (frameOk) {
        if (info->cntValidFrames >= info->concealParams.numFadeInFrames) {
          /* NEXT STATE: Ok */
          info->concealState = SpatialDecConcealState_Ok;
        }
      } else {
        /* NEXT STATE: Fade to default */
        info->concealState = SpatialDecConcealState_FadeToDefault;
        info->cntStateFrames = 0;
      }
      break;

    default:
      FDK_ASSERT(0); /* All valid states should be handled above! */
      break;
  }
}

SACDEC_ERROR SpatialDecConcealment_SetParam(SpatialDecConcealmentInfo *self,
                                            const SAC_DEC_CONCEAL_PARAM param,
                                            const INT value) {
  SACDEC_ERROR err = MPS_OK;

  switch (param) {
    case SAC_DEC_CONCEAL_METHOD:
      switch ((SpatialDecConcealmentMethod)value) {
        case SAC_DEC_CONCEAL_WITH_ZERO_VALUED_OUTPUT:
        case SAC_DEC_CONCEAL_BY_FADING_PARAMETERS:
          break;
        default:
          err = MPS_INVALID_PARAMETER;
          goto bail;
      }
      if (self != NULL) {
        /* store parameter value */
        self->concealParams.method = (SpatialDecConcealmentMethod)value;
      } else {
        err = MPS_INVALID_HANDLE;
        goto bail;
      }
      break;
    case SAC_DEC_CONCEAL_NUM_KEEP_FRAMES:
      if (value < 0) {
        err = MPS_INVALID_PARAMETER;
        goto bail;
      }
      if (self != NULL) {
        /* store parameter value */
        self->concealParams.numKeepFrames = (UINT)value;
      } else {
        err = MPS_INVALID_HANDLE;
        goto bail;
      }
      break;
    case SAC_DEC_CONCEAL_FADE_OUT_SLOPE_LENGTH:
      if (value < 0) {
        err = MPS_INVALID_PARAMETER;
        goto bail;
      }
      if (self != NULL) {
        /* store parameter value */
        self->concealParams.numFadeOutFrames = (UINT)value;
      } else {
        err = MPS_INVALID_HANDLE;
        goto bail;
      }
      break;
    case SAC_DEC_CONCEAL_FADE_IN_SLOPE_LENGTH:
      if (value < 0) {
        err = MPS_INVALID_PARAMETER;
        goto bail;
      }
      if (self != NULL) {
        /* store parameter value */
        self->concealParams.numFadeInFrames = (UINT)value;
      } else {
        err = MPS_INVALID_HANDLE;
        goto bail;
      }
      break;
    case SAC_DEC_CONCEAL_NUM_RELEASE_FRAMES:
      if (value < 0) {
        err = MPS_INVALID_PARAMETER;
        goto bail;
      }
      if (self != NULL) {
        /* store parameter value */
        self->concealParams.numReleaseFrames = (UINT)value;
      } else {
        err = MPS_INVALID_HANDLE;
        goto bail;
      }
      break;
    default:
      err = MPS_INVALID_PARAMETER;
      goto bail;
  }

bail:
  return err;
}
