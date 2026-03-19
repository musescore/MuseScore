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
                Get windows for framing

*******************************************************************************/

/**************************************************************************/ /**
   \file
   Description of file contents
 ******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_framewindowing.h"
#include "sacenc_vectorfunctions.h"

/* Defines *******************************************************************/

/* Data Types ****************************************************************/
typedef struct T_FRAMEWINDOW {
  INT nTimeSlotsMax;
  INT bFrameKeep;
  INT startSlope;
  INT stopSlope;
  INT startRect;
  INT stopRect;

  INT taperAnaLen;
  INT taperSynLen;
  FIXP_WIN pTaperAna__FDK[MAX_TIME_SLOTS];
  FIXP_WIN pTaperSyn__FDK[MAX_TIME_SLOTS];

} FRAMEWINDOW;

typedef enum {
  FIX_INVALID = -1,
  FIX_RECT_SMOOTH = 0,
  FIX_SMOOTH_RECT = 1,
  FIX_LARGE_SMOOTH = 2,
  FIX_RECT_TRIANG = 3

} FIX_TYPE;

typedef enum {
  VAR_INVALID = -1,
  VAR_HOLD = 0,
  VAR_ISOLATE = 1

} VAR_TYPE;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
static void calcTaperWin(FIXP_WIN *pTaperWin, INT timeSlots) {
  FIXP_DBL x;
  int i, scale;

  for (i = 0; i < timeSlots; i++) {
    x = fDivNormHighPrec((FIXP_DBL)i, (FIXP_DBL)timeSlots, &scale);

    if (scale < 0) {
      pTaperWin[i] = FX_DBL2FX_WIN(x >> (-scale));
    } else {
      pTaperWin[i] = FX_DBL2FX_WIN(x << (scale));
    }
  }
  pTaperWin[timeSlots] = FX_DBL2FX_WIN((FIXP_DBL)MAXVAL_DBL);
}

FDK_SACENC_ERROR fdk_sacenc_frameWindow_Create(
    HANDLE_FRAMEWINDOW *phFrameWindow) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == phFrameWindow) {
    error = SACENC_INVALID_HANDLE;
  } else {
    /* Memory Allocation */
    FDK_ALLOCATE_MEMORY_1D(*phFrameWindow, 1, FRAMEWINDOW);
  }
  return error;

bail:
  fdk_sacenc_frameWindow_Destroy(phFrameWindow);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

FDK_SACENC_ERROR fdk_sacenc_frameWindow_Init(
    HANDLE_FRAMEWINDOW hFrameWindow,
    const FRAMEWINDOW_CONFIG *const pFrameWindowConfig) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hFrameWindow == NULL) || (pFrameWindowConfig == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else if (pFrameWindowConfig->nTimeSlotsMax < 0) {
    error = SACENC_INIT_ERROR;
  } else {
    int ts;
    hFrameWindow->bFrameKeep = pFrameWindowConfig->bFrameKeep;
    hFrameWindow->nTimeSlotsMax = pFrameWindowConfig->nTimeSlotsMax;

    FIXP_WIN winMaxVal = FX_DBL2FX_WIN((FIXP_DBL)MAXVAL_DBL);
    int timeSlots = pFrameWindowConfig->nTimeSlotsMax;
    {
      hFrameWindow->startSlope = 0;
      hFrameWindow->stopSlope = ((3 * timeSlots) >> 1) - 1;
      hFrameWindow->startRect = timeSlots >> 1;
      hFrameWindow->stopRect = timeSlots;
      calcTaperWin(hFrameWindow->pTaperSyn__FDK, timeSlots >> 1);
      hFrameWindow->taperSynLen = timeSlots >> 1;
    }

    /* Calculate Taper for non-rect. ana. windows */
    hFrameWindow->taperAnaLen =
        hFrameWindow->startRect - hFrameWindow->startSlope;
    for (ts = 0; ts < hFrameWindow->taperAnaLen; ts++) {
      { hFrameWindow->pTaperAna__FDK[ts] = winMaxVal; }
    }
  }

  return error;
}

FDK_SACENC_ERROR fdk_sacenc_frameWindow_Destroy(
    HANDLE_FRAMEWINDOW *phFrameWindow) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL != phFrameWindow) && (NULL != *phFrameWindow)) {
    FDKfree(*phFrameWindow);
    *phFrameWindow = NULL;
  }
  return error;
}

static FDK_SACENC_ERROR FrameWinList_Reset(FRAMEWIN_LIST *const pFrameWinList) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pFrameWinList) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int k = 0;
    for (k = 0; k < MAX_NUM_PARAMS; k++) {
      pFrameWinList->dat[k].slot = -1;
      pFrameWinList->dat[k].hold = FW_INTP;
    }
    pFrameWinList->n = 0;
  }
  return error;
}

static FDK_SACENC_ERROR FrameWindowList_Add(FRAMEWIN_LIST *const pFrameWinList,
                                            const INT slot,
                                            const FW_SLOTTYPE hold) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pFrameWinList) {
    error = SACENC_INVALID_HANDLE;
  } else {
    if (pFrameWinList->n >= MAX_NUM_PARAMS) { /* Place left in List ?*/
      error = SACENC_PARAM_ERROR;
    } else if (pFrameWinList->n > 0 &&
               pFrameWinList->dat[pFrameWinList->n - 1].slot - slot > 0) {
      error = SACENC_PARAM_ERROR;
    } else {
      pFrameWinList->dat[pFrameWinList->n].slot = slot;
      pFrameWinList->dat[pFrameWinList->n].hold = hold;
      pFrameWinList->n++;
    }
  }
  return error;
}

static FDK_SACENC_ERROR FrameWindowList_Remove(
    FRAMEWIN_LIST *const pFrameWinList, const INT idx) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pFrameWinList) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int k = 0;
    if (idx < 0 || idx >= MAX_NUM_PARAMS) {
      error = SACENC_PARAM_ERROR;
    } else if (pFrameWinList->n > 0) {
      if (idx == MAX_NUM_PARAMS - 1) {
        pFrameWinList->dat[idx].slot = -1;
        pFrameWinList->dat[idx].hold = FW_INTP;
      } else {
        for (k = idx; k < MAX_NUM_PARAMS - 1; k++) {
          pFrameWinList->dat[k] = pFrameWinList->dat[k + 1];
        }
      }
      pFrameWinList->n--;
    }
  }
  return error;
}

static FDK_SACENC_ERROR FrameWindowList_Limit(
    FRAMEWIN_LIST *const pFrameWinList, const INT ll /*lower limit*/,
    const INT ul /*upper limit*/
) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pFrameWinList) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int k = 0;
    for (k = 0; k < pFrameWinList->n; k++) {
      if (pFrameWinList->dat[k].slot < ll || pFrameWinList->dat[k].slot > ul) {
        FrameWindowList_Remove(pFrameWinList, k);
        --k;
      }
    }
  }
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_frameWindow_GetWindow(
    HANDLE_FRAMEWINDOW hFrameWindow, INT tr_pos[MAX_NUM_PARAMS],
    const INT timeSlots, FRAMINGINFO *const pFramingInfo,
    FIXP_WIN *pWindowAna__FDK[MAX_NUM_PARAMS],
    FRAMEWIN_LIST *const pFrameWinList, const INT avoid_keep) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hFrameWindow == NULL) || (tr_pos == NULL) || (pFramingInfo == NULL) ||
      (pFrameWinList == NULL) || (pWindowAna__FDK == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    const VAR_TYPE varType = VAR_HOLD;
    const int tranL = 4;
    int winCnt = 0;
    int w, ps;

    int startSlope = hFrameWindow->startSlope;
    int stopSlope = hFrameWindow->stopSlope;
    int startRect = hFrameWindow->startRect;
    int stopRect = hFrameWindow->stopRect;
    int taperAnaLen = hFrameWindow->taperAnaLen;

    FIXP_WIN winMaxVal = FX_DBL2FX_WIN((FIXP_DBL)MAXVAL_DBL);
    FIXP_WIN applyRightWindowGain__FDK[MAX_NUM_PARAMS];
    FIXP_WIN *pTaperAna__FDK = hFrameWindow->pTaperAna__FDK;

    /* sanity check */
    for (ps = 0; ps < MAX_NUM_PARAMS; ps++) {
      if (pWindowAna__FDK[ps] == NULL) {
        error = SACENC_INVALID_HANDLE;
        goto bail;
      }
    }

    if ((timeSlots > hFrameWindow->nTimeSlotsMax) || (timeSlots < 0)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /* Reset */
    if (SACENC_OK != (error = FrameWinList_Reset(pFrameWinList))) goto bail;

    FDKmemclear(applyRightWindowGain__FDK, sizeof(applyRightWindowGain__FDK));

    if (tr_pos[0] > -1) { /* Transients in first (left) half? */
      int p_l = tr_pos[0];
      winCnt = 0;

      /* Create Parameter Positions */
      switch (varType) {
        case VAR_HOLD:
          if (SACENC_OK !=
              (error = FrameWindowList_Add(pFrameWinList, p_l - 1, FW_HOLD)))
            goto bail;
          if (SACENC_OK !=
              (error = FrameWindowList_Add(pFrameWinList, p_l, FW_INTP)))
            goto bail;
          break;
        case VAR_ISOLATE:
          if (SACENC_OK !=
              (error = FrameWindowList_Add(pFrameWinList, p_l - 1, FW_HOLD)))
            goto bail;
          if (SACENC_OK !=
              (error = FrameWindowList_Add(pFrameWinList, p_l, FW_INTP)))
            goto bail;
          if (SACENC_OK != (error = FrameWindowList_Add(pFrameWinList,
                                                        p_l + tranL, FW_HOLD)))
            goto bail;
          if (SACENC_OK != (error = FrameWindowList_Add(
                                pFrameWinList, p_l + tranL + 1, FW_INTP)))
            goto bail;
          break;
        default:
          error = SACENC_INVALID_CONFIG;
          break;
      }

      /* Outside of frame? => Kick Out */
      if (SACENC_OK !=
          (error = FrameWindowList_Limit(pFrameWinList, 0, timeSlots - 1)))
        goto bail;

      /* Add timeSlots as temporary border for window creation */
      if (SACENC_OK !=
          (error = FrameWindowList_Add(pFrameWinList, timeSlots - 1, FW_HOLD)))
        goto bail;

      /* Create Windows */
      for (ps = 0; ps < pFrameWinList->n - 1; ps++) {
        if (FW_HOLD != pFrameWinList->dat[ps].hold) {
          int const start = pFrameWinList->dat[ps].slot;
          int const stop = pFrameWinList->dat[ps + 1].slot;

          /* Analysis Window */
          FDKmemset_flex(pWindowAna__FDK[winCnt], FX_DBL2FX_WIN((FIXP_DBL)0),
                         start);
          FDKmemset_flex(&pWindowAna__FDK[winCnt][start], winMaxVal,
                         stop - start + 1);
          FDKmemset_flex(&pWindowAna__FDK[winCnt][stop + 1],
                         FX_DBL2FX_WIN((FIXP_DBL)0), timeSlots - stop - 1);

          applyRightWindowGain__FDK[winCnt] =
              pWindowAna__FDK[winCnt][timeSlots - 1];
          winCnt++;
        }
      } /* ps */

      /* Pop temporary frame border */
      if (SACENC_OK !=
          (error = FrameWindowList_Remove(pFrameWinList, pFrameWinList->n - 1)))
        goto bail;
    } else { /* No transient in left half of ana. window */
      winCnt = 0;

      /* Add paramter set at end of frame */
      if (SACENC_OK !=
          (error = FrameWindowList_Add(pFrameWinList, timeSlots - 1, FW_INTP)))
        goto bail;
      /* Analysis Window */
      FDKmemset_flex(pWindowAna__FDK[winCnt], FX_DBL2FX_WIN((FIXP_DBL)0),
                     startSlope);
      FDKmemcpy_flex(&pWindowAna__FDK[winCnt][startSlope], 1, pTaperAna__FDK, 1,
                     taperAnaLen);
      FDKmemset_flex(&pWindowAna__FDK[winCnt][startRect], winMaxVal,
                     timeSlots - startRect);

      applyRightWindowGain__FDK[winCnt] = winMaxVal;
      winCnt++;
    } /* if (tr_pos[0] > -1) */

    for (w = 0; w < winCnt; w++) {
      if (applyRightWindowGain__FDK[w] > (FIXP_WIN)0) {
        if (tr_pos[1] > -1) { /* Transients in second (right) half? */
          int p_r = tr_pos[1];

          /* Analysis Window */
          FDKmemset_flex(&pWindowAna__FDK[w][timeSlots], winMaxVal,
                         p_r - timeSlots);
          FDKmemset_flex(&pWindowAna__FDK[w][p_r], FX_DBL2FX_WIN((FIXP_DBL)0),
                         2 * timeSlots - p_r);

        } else { /* No transient in right half of ana. window */
          /* Analysis Window */
          FDKmemset_flex(&pWindowAna__FDK[w][timeSlots], winMaxVal,
                         stopRect - timeSlots + 1);
          FDKmemcpy_flex(&pWindowAna__FDK[w][stopRect], 1,
                         &pTaperAna__FDK[taperAnaLen - 1], -1, taperAnaLen);
          FDKmemset_flex(&pWindowAna__FDK[w][stopSlope + 1],
                         FX_DBL2FX_WIN((FIXP_DBL)0),
                         2 * timeSlots - stopSlope - 1);

        } /* if (tr_pos[1] > -1) */

        /* Weight */
        if (applyRightWindowGain__FDK[w] < winMaxVal) {
          int ts;
          for (ts = 0; ts < timeSlots; ts++) {
            pWindowAna__FDK[w][timeSlots + ts] =
                FX_DBL2FX_WIN(fMult(pWindowAna__FDK[w][timeSlots + ts],
                                    applyRightWindowGain__FDK[w]));
          }
        }
      } /* if (applyRightWindowGain[w] > 0.0f) */
      else {
        /* All Zero */
        FDKmemset_flex(&pWindowAna__FDK[w][timeSlots],
                       FX_DBL2FX_WIN((FIXP_DBL)0), timeSlots);
      }
    } /* loop over windows */

    if (hFrameWindow->bFrameKeep == 1) {
      FDKmemcpy_flex(&pWindowAna__FDK[0][2 * timeSlots], 1,
                     &pWindowAna__FDK[0][timeSlots], 1, timeSlots);
      FDKmemcpy_flex(&pWindowAna__FDK[0][timeSlots], 1, pWindowAna__FDK[0], 1,
                     timeSlots);

      if (avoid_keep != 0) {
        FDKmemset_flex(pWindowAna__FDK[0], FX_DBL2FX_WIN((FIXP_DBL)0),
                       timeSlots);
      } else {
        FDKmemset_flex(pWindowAna__FDK[0], winMaxVal, timeSlots);
      }
    } /* if (hFrameWindow->bFrameKeep==1) */

    /* Feed Info to Bitstream Formatter */
    pFramingInfo->numParamSets = pFrameWinList->n;
    pFramingInfo->bsFramingType = 1; /* variable framing */
    for (ps = 0; ps < pFramingInfo->numParamSets; ps++) {
      pFramingInfo->bsParamSlots[ps] = pFrameWinList->dat[ps].slot;
    }

    /* if there is just one param set at last slot,
       use fixed framing to save some bits */
    if ((pFramingInfo->numParamSets == 1) &&
        (pFramingInfo->bsParamSlots[0] == timeSlots - 1)) {
      pFramingInfo->bsFramingType = 0;
    }

  } /* valid handle */

bail:

  return error;
}

FDK_SACENC_ERROR fdk_sacenc_analysisWindowing(
    const INT nTimeSlots, const INT startTimeSlot,
    FIXP_WIN *pFrameWindowAna__FDK, const FIXP_DPK *const *const ppDataIn__FDK,
    FIXP_DPK *const *const ppDataOut__FDK, const INT nHybridBands,
    const INT dim) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((pFrameWindowAna__FDK == NULL) || (ppDataIn__FDK == NULL) ||
      (ppDataOut__FDK == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i, ts;
    FIXP_WIN maxVal = FX_DBL2FX_WIN((FIXP_DBL)MAXVAL_DBL);

    if (dim == FW_CHANGE_DIM) {
      for (ts = startTimeSlot; ts < nTimeSlots; ts++) {
        FIXP_WIN win = pFrameWindowAna__FDK[ts];
        if (win == maxVal) {
          for (i = 0; i < nHybridBands; i++) {
            ppDataOut__FDK[i][ts].v.re = ppDataIn__FDK[ts][i].v.re;
            ppDataOut__FDK[i][ts].v.im = ppDataIn__FDK[ts][i].v.im;
          }
        } else {
          for (i = 0; i < nHybridBands; i++) {
            ppDataOut__FDK[i][ts].v.re = fMult(win, ppDataIn__FDK[ts][i].v.re);
            ppDataOut__FDK[i][ts].v.im = fMult(win, ppDataIn__FDK[ts][i].v.im);
          }
        }
      } /* ts */
    } else {
      for (ts = startTimeSlot; ts < nTimeSlots; ts++) {
        FIXP_WIN win = pFrameWindowAna__FDK[ts];
        if (win == maxVal) {
          for (i = 0; i < nHybridBands; i++) {
            ppDataOut__FDK[ts][i].v.re = ppDataIn__FDK[ts][i].v.re;
            ppDataOut__FDK[ts][i].v.im = ppDataIn__FDK[ts][i].v.im;
          }
        } else {
          for (i = 0; i < nHybridBands; i++) {
            ppDataOut__FDK[ts][i].v.re = fMult(win, ppDataIn__FDK[ts][i].v.re);
            ppDataOut__FDK[ts][i].v.im = fMult(win, ppDataIn__FDK[ts][i].v.im);
          }
        }
      } /* ts */
    }
  }

  return error;
}
