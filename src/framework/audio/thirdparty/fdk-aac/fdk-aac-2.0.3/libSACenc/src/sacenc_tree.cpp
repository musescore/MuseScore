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
                Tree Structure for Space Encoder

*******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_tree.h"
#include "genericStds.h"
#include "sacenc_const.h"
#include "sacenc_paramextract.h"
#include "sacenc_framewindowing.h"
#include "FDK_matrixCalloc.h"

/* Defines *******************************************************************/
enum { BOX_0 = 0, BOX_1 = 1 };

enum { CH_L = 0, CH_R = 1 };

enum { TTO_CH_0 = 0, TTO_CH_1 = 1 };

enum { WIN_INACTIV = 0, WIN_ACTIV = 1 };

enum { MAX_KEEP_FRAMECOUNT = 100 };

/* Data Types ****************************************************************/
struct SPACE_TREE {
  SPACETREE_MODE mode;
  SPACE_TREE_DESCRIPTION descr;
  HANDLE_TTO_BOX ttoBox[SACENC_MAX_NUM_BOXES];
  UCHAR nParamBands;
  UCHAR bUseCoarseQuantTtoIcc;
  UCHAR bUseCoarseQuantTtoCld;
  QUANTMODE quantMode;
  INT frameCount;
  UCHAR bFrameKeep;

  /* Intermediate buffers */
  UCHAR pCld_prev[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAM_BANDS];
  UCHAR pIcc_prev[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAM_BANDS];

  UCHAR nChannelsInMax;
  UCHAR nHybridBandsMax;
};

typedef struct {
  UCHAR boxId;
  UCHAR inCh1;
  UCHAR inCh2;
  UCHAR inCh3;
  UCHAR inCh4;
  UCHAR wCh1;
  UCHAR wCh2;

} TTO_DESCRIPTOR;

typedef struct {
  SPACETREE_MODE mode;
  SPACE_TREE_DESCRIPTION treeDescription;

} TREE_CONFIG;

typedef struct {
  SPACETREE_MODE mode;
  UCHAR nChannelsIn;
  UCHAR nChannelsOut;
  UCHAR nTtoBoxes;
  TTO_DESCRIPTOR tto_descriptor[1];

} TREE_SETUP;

/* Constants *****************************************************************/
static const TREE_CONFIG treeConfigTable[] = {
    {SPACETREE_INVALID_MODE, {0, 0, 0}}, {SPACETREE_212, {1, 1, 2}}};

static const TREE_SETUP treeSetupTable[] = {
    {SPACETREE_INVALID_MODE, 0, 0, 0, {{0, 0, 0, 0, 0, 0, 0}}},
    {SPACETREE_212,
     2,
     1,
     1,
     {{BOX_0, CH_L, CH_R, TTO_CH_0, TTO_CH_1, WIN_ACTIV, WIN_ACTIV}}}};

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
static FDK_SACENC_ERROR getTreeConfig(
    const SPACETREE_MODE mode, SPACE_TREE_DESCRIPTION *pTreeDescription) {
  FDK_SACENC_ERROR error = SACENC_INIT_ERROR;

  if (pTreeDescription == NULL) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i;
    for (i = 0; i < (int)(sizeof(treeConfigTable) / sizeof(TREE_CONFIG)); i++) {
      if (treeConfigTable[i].mode == mode) {
        *pTreeDescription = treeConfigTable[i].treeDescription;
        error = SACENC_OK;
        break;
      }
    }
  } /* valid handle */
  return error;
}

static const TREE_SETUP *getTreeSetup(const SPACETREE_MODE mode) {
  int i;
  const TREE_SETUP *setup = NULL;

  for (i = 0; i < (int)(sizeof(treeSetupTable) / sizeof(TREE_SETUP)); i++) {
    if (treeSetupTable[i].mode == mode) {
      setup = &treeSetupTable[i];
      break;
    }
  }
  return setup;
}

FDK_SACENC_ERROR fdk_sacenc_spaceTree_Open(HANDLE_SPACE_TREE *phSpaceTree) {
  FDK_SACENC_ERROR error = SACENC_OK;
  HANDLE_SPACE_TREE hSpaceTree = NULL;

  if (NULL == phSpaceTree) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int box;

    FDK_ALLOCATE_MEMORY_1D(hSpaceTree, 1, struct SPACE_TREE);

    for (box = 0; box < SACENC_MAX_NUM_BOXES; box++) {
      HANDLE_TTO_BOX ttoBox = NULL;
      if (SACENC_OK != (error = fdk_sacenc_createTtoBox(&ttoBox))) {
        goto bail;
      }
      if (NULL != hSpaceTree) {
        hSpaceTree->ttoBox[box] = ttoBox;
      }
    }
    *phSpaceTree = hSpaceTree;
  }
  return error;

bail:
  fdk_sacenc_spaceTree_Close(&hSpaceTree);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

FDK_SACENC_ERROR fdk_sacenc_spaceTree_Init(
    HANDLE_SPACE_TREE hST, const SPACE_TREE_SETUP *const hSetup,
    UCHAR *pParameterBand2HybridBandOffset, const INT bFrameKeep) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hST == NULL) || (hSetup == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int bTtoBoxFrontBackCombin[SACENC_MAX_NUM_BOXES] = {0};
    int box = 0;

    hST->frameCount = 0;
    hST->bFrameKeep = bFrameKeep;

    /* Init */
    hST->mode = hSetup->mode;
    hST->nParamBands = hSetup->nParamBands;
    hST->bUseCoarseQuantTtoIcc = hSetup->bUseCoarseQuantTtoIcc;
    hST->bUseCoarseQuantTtoCld = hSetup->bUseCoarseQuantTtoCld;
    hST->quantMode = hSetup->quantMode;
    hST->nChannelsInMax = hSetup->nChannelsInMax;
    hST->nHybridBandsMax = hSetup->nHybridBandsMax;

    if (SACENC_OK != (error = getTreeConfig(hST->mode, &hST->descr))) {
      goto bail;
    }

    switch (hST->mode) {
      case SPACETREE_212:
        bTtoBoxFrontBackCombin[BOX_0] = 0;
        break;
      case SPACETREE_INVALID_MODE:
      default:
        error = SACENC_INIT_ERROR;
        goto bail;
    } /* switch (hST->mode) */

    if (hST->descr.nOttBoxes > SACENC_MAX_NUM_BOXES) {
      error = SACENC_INIT_ERROR;
      goto bail;
    }

    for (box = 0; box < hST->descr.nOttBoxes; box++) {
      TTO_BOX_CONFIG boxConfig;
      boxConfig.subbandConfig = (BOX_SUBBAND_CONFIG)hST->nParamBands;
      boxConfig.bUseCoarseQuantCld = hST->bUseCoarseQuantTtoCld;
      boxConfig.bUseCoarseQuantIcc = hST->bUseCoarseQuantTtoIcc;
      boxConfig.bUseCoherenceIccOnly = bTtoBoxFrontBackCombin[box];
      boxConfig.boxQuantMode = (BOX_QUANTMODE)hST->quantMode;
      boxConfig.nHybridBandsMax = hST->nHybridBandsMax;
      boxConfig.bFrameKeep = hST->bFrameKeep;

      if (SACENC_OK !=
          (error = fdk_sacenc_initTtoBox(hST->ttoBox[box], &boxConfig,
                                         pParameterBand2HybridBandOffset))) {
        goto bail;
      }
    } /* for box */

  } /* valid handle */

bail:
  return error;
}

static void SpaceTree_FrameKeep212(const HANDLE_SPACE_TREE hST,
                                   SPATIALFRAME *const hSTOut,
                                   const INT avoid_keep) {
  int pb;

  if (avoid_keep == 0) {
    if (hST->frameCount % 2 == 0) {
      for (pb = 0; pb < hST->nParamBands; pb++) {
        hST->pIcc_prev[BOX_0][pb] = hSTOut->ottData.icc[BOX_0][0][pb];
        hSTOut->ottData.cld[BOX_0][0][pb] = hST->pCld_prev[BOX_0][pb];
      }
    } else {
      for (pb = 0; pb < hST->nParamBands; pb++) {
        hSTOut->ottData.icc[BOX_0][0][pb] = hST->pIcc_prev[BOX_0][pb];
        hST->pCld_prev[BOX_0][pb] = hSTOut->ottData.cld[BOX_0][0][pb];
      }
    }
  } else {
    for (pb = 0; pb < hST->nParamBands; pb++) {
      hST->pIcc_prev[BOX_0][pb] = hSTOut->ottData.icc[BOX_0][0][pb];
      hST->pCld_prev[BOX_0][pb] = hSTOut->ottData.cld[BOX_0][0][pb];
    }
  }
  hST->frameCount++;
  if (hST->frameCount == MAX_KEEP_FRAMECOUNT) {
    hST->frameCount = 0;
  }
}

static FDK_SACENC_ERROR SpaceTree_FrameKeep(const HANDLE_SPACE_TREE hST,
                                            SPATIALFRAME *const hSTOut,
                                            const INT avoid_keep) {
  FDK_SACENC_ERROR error = SACENC_OK;

  switch (hST->mode) {
    case SPACETREE_212:
      SpaceTree_FrameKeep212(hST, hSTOut, avoid_keep);
      break;
    case SPACETREE_INVALID_MODE:
    default:
      error = SACENC_INVALID_CONFIG;
      break;
  }
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_spaceTree_Apply(
    HANDLE_SPACE_TREE hST, const INT paramSet, const INT nChannelsIn,
    const INT nTimeSlots, const INT startTimeSlot, const INT nHybridBands,
    FIXP_WIN *pFrameWindowAna__FDK,
    FIXP_DPK *const *const *const pppHybrid__FDK,
    FIXP_DPK *const *const *const pppHybridIn__FDK, SPATIALFRAME *const hSTOut,
    const INT avoid_keep, INT *pEncoderInputChScale) {
  /** \verbatim
   =============================================================================================================================
      TREE_212
   =============================================================================================================================
                         _______
        L -- TTO_CH_0 --|       |
                        | TTO_0 |-- TTO_CH_0
        R -- TTO_CH_1 --|_______|

  \endverbatim */

  FDK_SACENC_ERROR error = SACENC_OK;
  int k;
  const TREE_SETUP *treeSetup = NULL;

  if ((hST == NULL) || (hSTOut == NULL) || (pppHybrid__FDK == NULL) ||
      (pppHybridIn__FDK == NULL)) {
    error = SACENC_INVALID_HANDLE;
    goto bail;
  }

  if ((treeSetup = getTreeSetup(hST->mode)) == NULL) {
    error = SACENC_INVALID_CONFIG;
    goto bail;
  }

  /* Sanity Checks */
  if ((nChannelsIn != treeSetup->nChannelsIn) ||
      (nChannelsIn > hST->nChannelsInMax) ||
      (nHybridBands > hST->nHybridBandsMax)) {
    error = SACENC_INVALID_CONFIG;
    goto bail;
  }

  /* Apply all TTO boxes. */
  for (k = 0; k < treeSetup->nTtoBoxes; k++) {
    const TTO_DESCRIPTOR *pTTO = &treeSetup->tto_descriptor[k];

    int i, inCh[2], outCh[2], win[2];

    inCh[0] = pTTO->inCh1;
    outCh[0] = pTTO->inCh3;
    win[0] = pTTO->wCh1;
    inCh[1] = pTTO->inCh2;
    outCh[1] = pTTO->inCh4;
    win[1] = pTTO->wCh2;

    for (i = 0; i < 2; i++) {
      if (win[i] == WIN_ACTIV) {
        fdk_sacenc_analysisWindowing(
            nTimeSlots, startTimeSlot, pFrameWindowAna__FDK,
            pppHybrid__FDK[inCh[i]], pppHybridIn__FDK[outCh[i]], nHybridBands,
            FW_LEAVE_DIM);
      }
    }

    /* Calculate output downmix within last TTO box, if no TTT box is applied.
     */
    if (SACENC_OK !=
        (error = fdk_sacenc_applyTtoBox(
             hST->ttoBox[pTTO->boxId], nTimeSlots, startTimeSlot, nHybridBands,
             pppHybridIn__FDK[pTTO->inCh3], pppHybridIn__FDK[pTTO->inCh4],
             hSTOut->ottData.icc[pTTO->boxId][paramSet],
             &(hSTOut->ICCLosslessData.bsQuantCoarseXXX[pTTO->boxId][paramSet]),
             hSTOut->ottData.cld[pTTO->boxId][paramSet],
             &(hSTOut->CLDLosslessData.bsQuantCoarseXXX[pTTO->boxId][paramSet]),
             hSTOut->bUseBBCues, &pEncoderInputChScale[inCh[0]],
             &pEncoderInputChScale[inCh[1]]))) {
      goto bail;
    }
  }

  if (hST->bFrameKeep == 1) {
    if (SACENC_OK != (error = SpaceTree_FrameKeep(hST, hSTOut, avoid_keep))) {
      goto bail;
    }
  }

bail:
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_spaceTree_Close(HANDLE_SPACE_TREE *phSpaceTree) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((phSpaceTree == NULL) || (*phSpaceTree == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int box;
    HANDLE_SPACE_TREE const hST = *phSpaceTree;

    /* for (box = 0; box < hST->descr.nOttBoxes; ++box) { */
    for (box = 0; box < SACENC_MAX_NUM_BOXES; ++box) {
      if (SACENC_OK != (error = fdk_sacenc_destroyTtoBox(&hST->ttoBox[box]))) {
        goto bail;
      }
    }

    FDKfree(*phSpaceTree);
    *phSpaceTree = NULL;
  }
bail:
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_spaceTree_GetDescription(
    const HANDLE_SPACE_TREE hSpaceTree,
    SPACE_TREE_DESCRIPTION *pSpaceTreeDescription) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hSpaceTree == NULL) || (pSpaceTreeDescription == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    *pSpaceTreeDescription = hSpaceTree->descr;
  }
  return error;
}

INT fdk_sacenc_spaceTree_Hybrid2ParamBand(const INT nParamBands,
                                          const INT nHybridBand) {
  return fdk_sacenc_subband2ParamBand((BOX_SUBBAND_CONFIG)nParamBands,
                                      nHybridBand);
}

/*****************************************************************************
******************************************************************************/
