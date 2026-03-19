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

   Author(s):   Max Neuendorf

   Description: Encoder Library Interface
                Detect Onset in current frame

*******************************************************************************/

/**************************************************************************/ /**
   \file
   Description of file contents
 ******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_onsetdetect.h"
#include "genericStds.h"
#include "sacenc_vectorfunctions.h"

/* Defines *******************************************************************/
#define SPACE_ONSET_THRESHOLD (3.0)
#define SPACE_ONSET_THRESHOLD_SF (3)
#define SPACE_ONSET_THRESHOLD_SQUARE                                        \
  (FL2FXCONST_DBL((1.0 / (SPACE_ONSET_THRESHOLD * SPACE_ONSET_THRESHOLD)) * \
                  (float)(1 << SPACE_ONSET_THRESHOLD_SF)))

/* Data Types ****************************************************************/
struct ONSET_DETECT {
  INT maxTimeSlots;
  INT minTransientDistance;
  INT avgEnergyDistance;
  INT lowerBoundOnsetDetection;
  INT upperBoundOnsetDetection;
  FIXP_DBL *pEnergyHist__FDK;
  SCHAR *pEnergyHistScale;
  SCHAR avgEnergyDistanceScale;
};

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
FDK_SACENC_ERROR fdk_sacenc_onsetDetect_Open(HANDLE_ONSET_DETECT *phOnset,
                                             const UINT maxTimeSlots) {
  FDK_SACENC_ERROR error = SACENC_OK;
  HANDLE_ONSET_DETECT hOnset = NULL;

  if (NULL == phOnset) {
    error = SACENC_INVALID_HANDLE;
  } else {
    /* Memory Allocation */
    FDK_ALLOCATE_MEMORY_1D(hOnset, 1, struct ONSET_DETECT);
    FDK_ALLOCATE_MEMORY_1D(hOnset->pEnergyHist__FDK, 16 + maxTimeSlots,
                           FIXP_DBL);
    FDK_ALLOCATE_MEMORY_1D(hOnset->pEnergyHistScale, 16 + maxTimeSlots, SCHAR);

    hOnset->maxTimeSlots = maxTimeSlots;
    hOnset->minTransientDistance =
        8; /* minimum distance between detected transients */
    hOnset->avgEnergyDistance = 16; /* average energy distance */

    hOnset->avgEnergyDistanceScale = 4;
    *phOnset = hOnset;
  }
  return error;

bail:
  fdk_sacenc_onsetDetect_Close(&hOnset);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

FDK_SACENC_ERROR fdk_sacenc_onsetDetect_Init(
    HANDLE_ONSET_DETECT hOnset,
    const ONSET_DETECT_CONFIG *const pOnsetDetectConfig, const UINT initFlags) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL == hOnset) || (pOnsetDetectConfig == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    if ((pOnsetDetectConfig->maxTimeSlots > hOnset->maxTimeSlots) ||
        (pOnsetDetectConfig->upperBoundOnsetDetection <
         hOnset->lowerBoundOnsetDetection)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    hOnset->maxTimeSlots = pOnsetDetectConfig->maxTimeSlots;
    hOnset->lowerBoundOnsetDetection =
        pOnsetDetectConfig->lowerBoundOnsetDetection;
    hOnset->upperBoundOnsetDetection =
        pOnsetDetectConfig->upperBoundOnsetDetection;

    hOnset->minTransientDistance =
        8; /* minimum distance between detected transients */
    hOnset->avgEnergyDistance = 16; /* average energy distance */

    hOnset->avgEnergyDistanceScale = 4;

    /* Init / Reset */
    if (initFlags) {
      int i;
      for (i = 0; i < hOnset->avgEnergyDistance + hOnset->maxTimeSlots; i++)
        hOnset->pEnergyHistScale[i] = -(DFRACT_BITS - 3);

      FDKmemset_flex(
          hOnset->pEnergyHist__FDK,
          FL2FXCONST_DBL(SACENC_FLOAT_EPSILON * (1 << (DFRACT_BITS - 3))),
          hOnset->avgEnergyDistance + hOnset->maxTimeSlots);
    }
  }

bail:
  return error;
}

/**************************************************************************/

FDK_SACENC_ERROR fdk_sacenc_onsetDetect_Close(HANDLE_ONSET_DETECT *phOnset) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL != phOnset) && (NULL != *phOnset)) {
    if (NULL != (*phOnset)->pEnergyHist__FDK) {
      FDKfree((*phOnset)->pEnergyHist__FDK);
    }
    (*phOnset)->pEnergyHist__FDK = NULL;

    if (NULL != (*phOnset)->pEnergyHistScale) {
      FDKfree((*phOnset)->pEnergyHistScale);
    }
    (*phOnset)->pEnergyHistScale = NULL;
    FDKfree(*phOnset);
    *phOnset = NULL;
  }
  return error;
}

/**************************************************************************/

FDK_SACENC_ERROR fdk_sacenc_onsetDetect_Update(HANDLE_ONSET_DETECT hOnset,
                                               const INT timeSlots) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == hOnset) {
    error = SACENC_INVALID_HANDLE;
  } else {
    if (timeSlots > hOnset->maxTimeSlots) {
      error = SACENC_INVALID_CONFIG;
    } else {
      int i;
      /* Shift old data */
      for (i = 0; i < hOnset->avgEnergyDistance; i++) {
        hOnset->pEnergyHist__FDK[i] = hOnset->pEnergyHist__FDK[i + timeSlots];
        hOnset->pEnergyHistScale[i] = hOnset->pEnergyHistScale[i + timeSlots];
      }

      /* Clear for new data */
      FDKmemset_flex(&hOnset->pEnergyHist__FDK[hOnset->avgEnergyDistance],
                     FL2FXCONST_DBL(SACENC_FLOAT_EPSILON), timeSlots);
    }
  }
  return error;
}

/**************************************************************************/

FDK_SACENC_ERROR fdk_sacenc_onsetDetect_Apply(
    HANDLE_ONSET_DETECT hOnset, const INT nTimeSlots, const INT nHybridBands,
    FIXP_DPK *const *const ppHybridData__FDK, const INT hybridDataScale,
    const INT prevPos, INT pTransientPos[MAX_NUM_TRANS]) {
  FDK_SACENC_ERROR error = SACENC_OK;

  C_ALLOC_SCRATCH_START(envs, FIXP_DBL, (16 + MAX_TIME_SLOTS))
  FDKmemclear(envs, (16 + MAX_TIME_SLOTS) * sizeof(FIXP_DBL));

  if ((hOnset == NULL) || (pTransientPos == NULL) ||
      (ppHybridData__FDK == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i, ts, trCnt, currPos;

    if ((nTimeSlots < 0) || (nTimeSlots > hOnset->maxTimeSlots) ||
        (hOnset->lowerBoundOnsetDetection < -1) ||
        (hOnset->upperBoundOnsetDetection > nHybridBands)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    const int lowerBoundOnsetDetection = hOnset->lowerBoundOnsetDetection;
    const int upperBoundOnsetDetection = hOnset->upperBoundOnsetDetection;
    const int M = hOnset->avgEnergyDistance;

    {
      SCHAR *envScale = hOnset->pEnergyHistScale;
      FIXP_DBL *env = hOnset->pEnergyHist__FDK;
      const FIXP_DBL threshold_square = SPACE_ONSET_THRESHOLD_SQUARE;

      trCnt = 0;

      /* reset transient array */
      FDKmemset_flex(pTransientPos, -1, MAX_NUM_TRANS);

      /* minimum transient distance of minTransDist QMF samples */
      if (prevPos > 0) {
        currPos = FDKmax(nTimeSlots,
                         prevPos - nTimeSlots + hOnset->minTransientDistance);
      } else {
        currPos = nTimeSlots;
      }

      /* get energy and scalefactor for each time slot */
      int outScale;
      int inScale = 3; /* scale factor determined empirically */
      for (ts = 0; ts < nTimeSlots; ts++) {
        env[M + ts] = sumUpCplxPow2(
            &ppHybridData__FDK[ts][lowerBoundOnsetDetection + 1],
            SUM_UP_DYNAMIC_SCALE, inScale, &outScale,
            upperBoundOnsetDetection - lowerBoundOnsetDetection - 1);
        envScale[M + ts] = outScale + (hybridDataScale << 1);
      }

      /* calculate common scale for all time slots */
      SCHAR maxScale = -(DFRACT_BITS - 1);
      for (i = 0; i < (nTimeSlots + M); i++) {
        maxScale = fixMax(maxScale, envScale[i]);
      }

      /* apply common scale and store energy in temporary buffer */
      for (i = 0; i < (nTimeSlots + M); i++) {
        envs[i] = env[i] >> fixMin((maxScale - envScale[i]), (DFRACT_BITS - 1));
      }

      FIXP_DBL maxVal = FL2FXCONST_DBL(0.0f);
      for (i = 0; i < (nTimeSlots + M); i++) {
        maxVal |= fAbs(envs[i]);
      }

      int s = fixMax(0, CntLeadingZeros(maxVal) - 1);

      for (i = 0; i < (nTimeSlots + M); i++) {
        envs[i] = envs[i] << s;
      }

      int currPosPrev = currPos;
      FIXP_DBL p1, p2;
      p2 = FL2FXCONST_DBL(0.0f);
      for (; (currPos < (nTimeSlots << 1)) && (trCnt < MAX_NUM_TRANS);
           currPos++) {
        p1 = fMultDiv2(envs[currPos - nTimeSlots + M], threshold_square) >>
             (SPACE_ONSET_THRESHOLD_SF - 1);

        /* Calculate average of past M energy values */
        if (currPosPrev == (currPos - 1)) {
          /* remove last and add new element */
          p2 -= (envs[currPosPrev - nTimeSlots] >>
                 (int)hOnset->avgEnergyDistanceScale);
          p2 += (envs[currPos - nTimeSlots + M - 1] >>
                 (int)hOnset->avgEnergyDistanceScale);
        } else {
          /* calculate complete vector */
          p2 = FL2FXCONST_DBL(0.0f);
          for (ts = 0; ts < M; ts++) {
            p2 += (envs[currPos - nTimeSlots + ts] >>
                   (int)hOnset->avgEnergyDistanceScale);
          }
        }
        currPosPrev = currPos;

        {
          /* save position if transient found */
          if (p1 > p2) {
            pTransientPos[trCnt++] = currPos;
            currPos += hOnset->minTransientDistance;
          }
        }
      } /* for currPos */
    }

  } /* valid handle*/
bail:

  C_ALLOC_SCRATCH_END(envs, FIXP_DBL, (16 + MAX_TIME_SLOTS))

  return error;
}

/**************************************************************************/
