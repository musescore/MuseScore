/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2020 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):

   Description: SAC Decoder Library

*******************************************************************************/

#include "sac_dec_errorcodes.h"
#include "sac_dec.h"

#include "sac_process.h"
#include "sac_bitdec.h"
#include "sac_smoothing.h"
#include "sac_calcM1andM2.h"
#include "sac_reshapeBBEnv.h"
#include "sac_stp.h"
#include "sac_rom.h"

#include "FDK_decorrelate.h"

#include "FDK_trigFcts.h"
#include "FDK_matrixCalloc.h"

/* static int pbStrideTable[] = {1, 2, 5, 28}; see sac_rom.cpp */

enum {
  APPLY_M2_NONE = 0,    /* init value */
  APPLY_M2 = 1,         /* apply m2 fallback implementation */
  APPLY_M2_MODE212 = 2, /* apply m2 for 212 mode */
  APPLY_M2_MODE212_Res_PhaseCoding =
      3 /* apply m2 for 212 mode with residuals and phase coding */
};

/******************************************************************************************/
/* function: FDK_SpatialDecInitDefaultSpatialSpecificConfig */
/* output:   struct of type SPATIAL_SPECIFIC_CONFIG */
/* input:    core coder audio object type */
/* input:    nr of core channels */
/* input:    sampling rate */
/* input:    nr of time slots */
/* input:    decoder level */
/* input:    flag indicating upmix type blind */
/*                                                                                        */
/* returns:  error code */
/******************************************************************************************/
int FDK_SpatialDecInitDefaultSpatialSpecificConfig(
    SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
    AUDIO_OBJECT_TYPE coreCodec, int coreChannels, int samplingFreq,
    int nTimeSlots, int decoderLevel, int isBlind) {
  return SpatialDecDefaultSpecificConfig(pSpatialSpecificConfig, coreCodec,
                                         samplingFreq, nTimeSlots, decoderLevel,
                                         isBlind, coreChannels);
}

/******************************************************************************************/
/* function: FDK_SpatialDecCompareSpatialSpecificConfigHeader */
/* input:    2 pointers to a ssc */
/*                                                                                        */
/* output:   - */
/* returns:  error code (0 = equal, <>0 unequal) */
/******************************************************************************************/
int FDK_SpatialDecCompareSpatialSpecificConfigHeader(
    SPATIAL_SPECIFIC_CONFIG *pSsc1, SPATIAL_SPECIFIC_CONFIG *pSsc2) {
  int result = MPS_OK;

  /* we assume: every bit must be equal */
  if (FDKmemcmp(pSsc1, pSsc2, sizeof(SPATIAL_SPECIFIC_CONFIG)) != 0) {
    result = MPS_UNEQUAL_SSC;
  }
  return result;
}

/*******************************************************************************
 Functionname: SpatialDecClearFrameData
 *******************************************************************************

 Description: Clear/Fake frame data to avoid misconfiguration and allow proper
              error concealment.
 Arguments:
 Input:       self (frame data)
 Output:      No return value.

*******************************************************************************/
static void SpatialDecClearFrameData(
    spatialDec *self, /* Shall be removed */
    SPATIAL_BS_FRAME *bsFrame, const SACDEC_CREATION_PARAMS *const setup) {
  int i;

  FDK_ASSERT(self != NULL);
  FDK_ASSERT(bsFrame != NULL);
  FDK_ASSERT(setup != NULL);

  /* do not apply shaping tools (GES or STP) */
  for (i = 0; i < setup->maxNumOutputChannels;
       i += 1) { /* MAX_OUTPUT_CHANNELS */
    bsFrame->tempShapeEnableChannelSTP[i] = 0;
    bsFrame->tempShapeEnableChannelGES[i] = 0;
  }

  bsFrame->TsdData->bsTsdEnable = 0;

  /* use only 1 parameter set at the end of the frame */
  bsFrame->numParameterSets = 1;
  bsFrame->paramSlot[0] = self->timeSlots - 1;

  /* parameter smoothing tool set to off */
  bsFrame->bsSmoothMode[0] = 0;
  initParameterSmoothing(self);

  /* reset residual data */
  {
    int resQmfBands, resTimeSlots = (1);

    resQmfBands = setup->maxNumQmfBands;

    for (i = 0; i < setup->bProcResidual
                    ? fMin(setup->maxNumResChannels,
                           setup->maxNumOttBoxes + setup->maxNumInputChannels)
                    : 0;
         i += 1) {
      for (int j = 0; j < resTimeSlots; j += 1) {
        for (int k = 0; k < resQmfBands; k += 1) {
          self->qmfResidualReal__FDK[i][j][k] = FL2FXCONST_DBL(0.0f);
          self->qmfResidualImag__FDK[i][j][k] = FL2FXCONST_DBL(0.0f);
        }
      }
    }
  }

  return;
}

/*******************************************************************************
 Functionname: FDK_SpatialDecOpen
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
spatialDec *FDK_SpatialDecOpen(const SPATIAL_DEC_CONFIG *config,
                               int stereoConfigIndex) {
  int i;
  int lfSize, hfSize;
  spatialDec *self = NULL;
  SACDEC_CREATION_PARAMS setup;

  switch (config->decoderLevel) {
    case DECODER_LEVEL_0: /* 212 maxNumOutputChannels== 2 */
      setup.maxNumInputChannels = 1;
      setup.maxNumOutputChannels = 2;
      setup.maxNumQmfBands = 64;
      setup.maxNumXChannels = 2;
      setup.maxNumVChannels = 2;
      setup.maxNumDecorChannels = 1;
      setup.bProcResidual = 1;
      setup.maxNumResidualChannels = 0;
      setup.maxNumOttBoxes = 1;
      setup.maxNumParams = setup.maxNumInputChannels + setup.maxNumOttBoxes;
      break;
    default:
      return NULL;
  }

  setup.maxNumResChannels = 1;

  {
    switch (config->maxNumOutputChannels) {
      case OUTPUT_CHANNELS_2_0:
        setup.maxNumOutputChannels = fMin(setup.maxNumOutputChannels, 2);
        break;
      case OUTPUT_CHANNELS_DEFAULT:
      default:
        break;
    }
  }

  setup.maxNumHybridBands = SacGetHybridSubbands(setup.maxNumQmfBands);

  switch (config->decoderMode) {
    case EXT_HQ_ONLY:
      setup.maxNumCmplxQmfBands = setup.maxNumQmfBands;
      setup.maxNumCmplxHybBands = setup.maxNumHybridBands;
      break;
    default:
      setup.maxNumCmplxQmfBands = fixMax(PC_NUM_BANDS, setup.maxNumQmfBands);
      setup.maxNumCmplxHybBands =
          fixMax(PC_NUM_HYB_BANDS, setup.maxNumHybridBands);
      break;
  } /* switch config->decoderMode */

  FDK_ALLOCATE_MEMORY_1D_INT(self, 1, spatialDec, SECT_DATA_L2)

  self->createParams = setup;

  FDK_ALLOCATE_MEMORY_1D(self->param2hyb, MAX_PARAMETER_BANDS + 1, int)

  FDK_ALLOCATE_MEMORY_1D(self->numOttBands, setup.maxNumOttBoxes, int)

  /* allocate arrays */

  FDK_ALLOCATE_MEMORY_1D(self->smgTime, MAX_PARAMETER_SETS, int)
  FDK_ALLOCATE_MEMORY_2D(self->smgData, MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS,
                         UCHAR)

  FDK_ALLOCATE_MEMORY_3D(self->ottCLD__FDK, setup.maxNumOttBoxes,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_3D(self->ottICC__FDK, setup.maxNumOttBoxes,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_3D(self->ottIPD__FDK, setup.maxNumOttBoxes,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)

  /* Last parameters from prev frame */
  FDK_ALLOCATE_MEMORY_2D(self->ottCLDidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_2D(self->ottICCidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_3D(self->ottICCdiffidx, setup.maxNumOttBoxes,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_2D(self->ottIPDidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_2D(self->arbdmxGainIdxPrev, setup.maxNumInputChannels,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_2D(self->cmpOttCLDidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_2D(self->cmpOttICCidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_3D(self->outIdxData, setup.maxNumOttBoxes,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)

  FDK_ALLOCATE_MEMORY_3D(self->arbdmxGain__FDK, setup.maxNumInputChannels,
                         MAX_PARAMETER_SETS, MAX_PARAMETER_BANDS, SCHAR)
  FDK_ALLOCATE_MEMORY_1D(self->arbdmxAlpha__FDK, setup.maxNumInputChannels,
                         FIXP_DBL)
  FDK_ALLOCATE_MEMORY_1D(self->arbdmxAlphaPrev__FDK, setup.maxNumInputChannels,
                         FIXP_DBL)
  FDK_ALLOCATE_MEMORY_2D(self->cmpArbdmxGainIdxPrev, setup.maxNumInputChannels,
                         MAX_PARAMETER_BANDS, SCHAR)

  FDK_ALLOCATE_MEMORY_2D(self->cmpOttIPDidxPrev, setup.maxNumOttBoxes,
                         MAX_PARAMETER_BANDS, SCHAR)

  FDK_ALLOCATE_MEMORY_3D_INT(self->M2Real__FDK, setup.maxNumOutputChannels,
                             setup.maxNumVChannels, MAX_PARAMETER_BANDS,
                             FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_3D(self->M2Imag__FDK, setup.maxNumOutputChannels,
                         setup.maxNumVChannels, MAX_PARAMETER_BANDS, FIXP_DBL)

  FDK_ALLOCATE_MEMORY_3D_INT(self->M2RealPrev__FDK, setup.maxNumOutputChannels,
                             setup.maxNumVChannels, MAX_PARAMETER_BANDS,
                             FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_3D(self->M2ImagPrev__FDK, setup.maxNumOutputChannels,
                         setup.maxNumVChannels, MAX_PARAMETER_BANDS, FIXP_DBL)

  FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED(
      self->qmfInputReal__FDK, setup.maxNumInputChannels, setup.maxNumQmfBands,
      FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED(
      self->qmfInputImag__FDK, setup.maxNumInputChannels,
      setup.maxNumCmplxQmfBands, FIXP_DBL, SECT_DATA_L2)

  FDK_ALLOCATE_MEMORY_2D_INT(self->hybInputReal__FDK, setup.maxNumInputChannels,
                             setup.maxNumHybridBands, FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_2D_INT(self->hybInputImag__FDK, setup.maxNumInputChannels,
                             setup.maxNumCmplxHybBands, FIXP_DBL, SECT_DATA_L2)

  if (setup.bProcResidual) {
    FDK_ALLOCATE_MEMORY_1D(self->qmfResidualReal__FDK, setup.maxNumResChannels,
                           FIXP_DBL **)
    FDK_ALLOCATE_MEMORY_1D(self->qmfResidualImag__FDK, setup.maxNumResChannels,
                           FIXP_DBL **)

    FDK_ALLOCATE_MEMORY_1D(self->hybResidualReal__FDK, setup.maxNumResChannels,
                           FIXP_DBL *)
    FDK_ALLOCATE_MEMORY_1D(self->hybResidualImag__FDK, setup.maxNumResChannels,
                           FIXP_DBL *)

    for (i = 0; i < setup.maxNumResChannels; i++) {
      int resQmfBands = (config->decoderMode == EXT_LP_ONLY)
                            ? PC_NUM_BANDS
                            : setup.maxNumQmfBands;
      int resHybBands = (config->decoderMode == EXT_LP_ONLY)
                            ? PC_NUM_HYB_BANDS
                            : setup.maxNumHybridBands;
      /* Alignment is needed for USAC residuals because QMF analysis directly
       * writes to this buffer. */
      FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED(self->qmfResidualReal__FDK[i], (1),
                                         resQmfBands, FIXP_DBL, SECT_DATA_L1)
      FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED(self->qmfResidualImag__FDK[i], (1),
                                         resQmfBands, FIXP_DBL, SECT_DATA_L1)

      FDK_ALLOCATE_MEMORY_1D(self->hybResidualReal__FDK[i],
                             setup.maxNumHybridBands, FIXP_DBL)
      FDK_ALLOCATE_MEMORY_1D(self->hybResidualImag__FDK[i], resHybBands,
                             FIXP_DBL)
    }
  } /* if (setup.bProcResidual) */

  FDK_ALLOCATE_MEMORY_2D_INT(self->wReal__FDK, setup.maxNumVChannels,
                             setup.maxNumHybridBands, FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_2D_INT(self->wImag__FDK, setup.maxNumVChannels,
                             setup.maxNumCmplxHybBands, FIXP_DBL, SECT_DATA_L2)

  FDK_ALLOCATE_MEMORY_2D_INT(self->hybOutputRealDry__FDK,
                             setup.maxNumOutputChannels,
                             setup.maxNumHybridBands, FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_2D_INT(self->hybOutputImagDry__FDK,
                             setup.maxNumOutputChannels,
                             setup.maxNumCmplxHybBands, FIXP_DBL, SECT_DATA_L2)

  FDK_ALLOCATE_MEMORY_2D_INT(self->hybOutputRealWet__FDK,
                             setup.maxNumOutputChannels,
                             setup.maxNumHybridBands, FIXP_DBL, SECT_DATA_L2)
  FDK_ALLOCATE_MEMORY_2D_INT(self->hybOutputImagWet__FDK,
                             setup.maxNumOutputChannels,
                             setup.maxNumCmplxHybBands, FIXP_DBL, SECT_DATA_L2)

  FDK_ALLOCATE_MEMORY_1D(self->hybridSynthesis, setup.maxNumOutputChannels,
                         FDK_SYN_HYB_FILTER)

  FDK_ALLOCATE_MEMORY_1D(
      self->hybridAnalysis,
      setup.bProcResidual ? setup.maxNumInputChannels + setup.maxNumResChannels
                          : setup.maxNumInputChannels,
      FDK_ANA_HYB_FILTER)

  lfSize = 2 * BUFFER_LEN_LF * MAX_QMF_BANDS_TO_HYBRID;
  {
    hfSize =
        BUFFER_LEN_HF * ((setup.maxNumQmfBands - MAX_QMF_BANDS_TO_HYBRID) +
                         (setup.maxNumCmplxQmfBands - MAX_QMF_BANDS_TO_HYBRID));
  }

  FDK_ALLOCATE_MEMORY_2D_INT(self->pHybridAnaStatesLFdmx,
                             setup.maxNumInputChannels, lfSize, FIXP_DBL,
                             SECT_DATA_L2) {
    FDK_ALLOCATE_MEMORY_2D(self->pHybridAnaStatesHFdmx,
                           setup.maxNumInputChannels, hfSize, FIXP_DBL)
  }

  for (i = 0; i < setup.maxNumInputChannels; i++) {
    FIXP_DBL *pHybridAnaStatesHFdmx;

    pHybridAnaStatesHFdmx = self->pHybridAnaStatesHFdmx[i];

    FDKhybridAnalysisOpen(&self->hybridAnalysis[i],
                          self->pHybridAnaStatesLFdmx[i],
                          lfSize * sizeof(FIXP_DBL), pHybridAnaStatesHFdmx,
                          hfSize * sizeof(FIXP_DBL));
  }
  if (setup.bProcResidual) {
    lfSize = 2 * BUFFER_LEN_LF * MAX_QMF_BANDS_TO_HYBRID;
    hfSize = BUFFER_LEN_HF *
             ((((config->decoderMode == EXT_LP_ONLY) ? PC_NUM_BANDS
                                                     : setup.maxNumQmfBands) -
               MAX_QMF_BANDS_TO_HYBRID) +
              (setup.maxNumCmplxQmfBands - MAX_QMF_BANDS_TO_HYBRID));

    FDK_ALLOCATE_MEMORY_2D_INT(self->pHybridAnaStatesLFres,
                               setup.maxNumResChannels, lfSize, FIXP_DBL,
                               SECT_DATA_L2)
    FDK_ALLOCATE_MEMORY_2D(self->pHybridAnaStatesHFres, setup.maxNumResChannels,
                           hfSize, FIXP_DBL)

    for (i = setup.maxNumInputChannels;
         i < (setup.maxNumInputChannels + setup.maxNumResChannels); i++) {
      FDKhybridAnalysisOpen(
          &self->hybridAnalysis[i],
          self->pHybridAnaStatesLFres[i - setup.maxNumInputChannels],
          lfSize * sizeof(FIXP_DBL),
          self->pHybridAnaStatesHFres[i - setup.maxNumInputChannels],
          hfSize * sizeof(FIXP_DBL));
    }
  }

  FDK_ALLOCATE_MEMORY_1D(self->smoothState, 1, SMOOTHING_STATE)
  FDK_ALLOCATE_MEMORY_1D(self->reshapeBBEnvState, 1, RESHAPE_BBENV_STATE)

  FDK_ALLOCATE_MEMORY_1D(self->apDecor, setup.maxNumDecorChannels, DECORR_DEC)
  FDK_ALLOCATE_MEMORY_2D_INT(self->pDecorBufferCplx, setup.maxNumDecorChannels,
                             (2 * ((825) + (373))), FIXP_DBL, SECT_DATA_L2)

  for (i = 0; i < setup.maxNumDecorChannels; i++) {
    if (FDKdecorrelateOpen(&self->apDecor[i], self->pDecorBufferCplx[i],
                           (2 * ((825) + (373))))) {
      goto bail;
    }
  }

  if (subbandTPCreate(&self->hStpDec) != MPS_OK) {
    goto bail;
  }

  /* save general decoder configuration */
  self->decoderLevel = config->decoderLevel;
  self->decoderMode = config->decoderMode;
  self->binauralMode = config->binauralMode;

  /* preinitialize configuration */
  self->partiallyComplex = (config->decoderMode != EXT_HQ_ONLY) ? 1 : 0;

  /* Set to default state */
  SpatialDecConcealment_Init(&self->concealInfo, MPEGS_CONCEAL_RESET_ALL);

  /* Everything is fine so return the handle */
  return self;

bail:
  /* Collector for all errors.
     Deallocate all memory and return a invalid handle. */
  FDK_SpatialDecClose(self);

  return NULL;
}

/*******************************************************************************
 Functionname: isValidConfig
 *******************************************************************************

 Description: Validate if configuration is supported in present instance

 Arguments:

 Return: 1: all okay
         0: configuration not supported
*******************************************************************************/
static int isValidConfig(spatialDec const *const self,
                         const SPATIAL_DEC_UPMIX_TYPE upmixType,
                         SPATIALDEC_PARAM const *const pUserParams,
                         const AUDIO_OBJECT_TYPE coreAot) {
  UPMIXTYPE nUpmixType;

  FDK_ASSERT(self != NULL);
  FDK_ASSERT(pUserParams != NULL);

  nUpmixType = (UPMIXTYPE)upmixType;

  switch (nUpmixType) {
    case UPMIXTYPE_BYPASS: /* UPMIX_TYPE_BYPASS */
      break;
    case UPMIXTYPE_NORMAL: /* UPMIX_TYPE_NORMAL */
      break;
    default:
      return 0; /* unsupported upmixType */
  }

  return 1; /* upmixType supported */
}

static SACDEC_ERROR CheckLevelTreeUpmixType(
    const SACDEC_CREATION_PARAMS *const pCreateParams,
    const SPATIAL_SPECIFIC_CONFIG *const pSsc, const int decoderLevel,
    const UPMIXTYPE upmixType) {
  SACDEC_ERROR err = MPS_OK;
  int nOutputChannels, treeConfig;

  FDK_ASSERT(pCreateParams != NULL);
  FDK_ASSERT(pSsc != NULL);

  treeConfig = pSsc->treeConfig;

  switch (decoderLevel) {
    case 0: {
      if (treeConfig != SPATIALDEC_MODE_RSVD7) {
        err = MPS_INVALID_TREECONFIG;
        goto bail;
      }
      break;
    }
    default:
      err = MPS_INVALID_PARAMETER /* MPS_UNIMPLEMENTED */;
      goto bail;
  }

  switch (upmixType) {
    case UPMIXTYPE_BYPASS:
      nOutputChannels = pSsc->nInputChannels;
      break;
    default:
      nOutputChannels = pSsc->nOutputChannels;
      break;
  }

  /* Is sufficient memory allocated. */
  if ((pSsc->nInputChannels > pCreateParams->maxNumInputChannels) ||
      (nOutputChannels > pCreateParams->maxNumOutputChannels) ||
      (pSsc->nOttBoxes > pCreateParams->maxNumOttBoxes)) {
    err = MPS_INVALID_PARAMETER;
  }

bail:
  return err;
}

void SpatialDecInitParserContext(spatialDec *self) {
  int i, j;

  for (i = 0; i < self->createParams.maxNumOttBoxes; i += 1) {
    for (j = 0; j < MAX_PARAMETER_BANDS; j++) {
      self->ottCLDidxPrev[i][j] = 0;
      self->ottICCidxPrev[i][j] = 0;
      self->cmpOttCLDidxPrev[i][j] = 0;
      self->cmpOttICCidxPrev[i][j] = 0;
    }
  }
  for (i = 0; i < self->createParams.maxNumInputChannels; i++) {
    for (j = 0; j < MAX_PARAMETER_BANDS; j++) {
      self->arbdmxGainIdxPrev[i][j] = 0;
      self->cmpArbdmxGainIdxPrev[i][j] = 0;
    }
  }
}

/*******************************************************************************
 Functionname: FDK_SpatialDecInit
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/

SACDEC_ERROR FDK_SpatialDecInit(spatialDec *self, SPATIAL_BS_FRAME *frame,
                                SPATIAL_SPECIFIC_CONFIG *pSpatialSpecificConfig,
                                int nQmfBands,
                                SPATIAL_DEC_UPMIX_TYPE const upmixType,
                                SPATIALDEC_PARAM *pUserParams, UINT initFlags) {
  SACDEC_ERROR err = MPS_OK;
  int nCh, i, j, k;
  int maxQmfBands;
  int bypassMode = 0;

  self->useFDreverb = 0;

  /* check configuration parameter */
  if (!isValidConfig(self, upmixType, pUserParams,
                     pSpatialSpecificConfig->coreCodec)) {
    return MPS_INVALID_PARAMETER;
  }

  /* check tree configuration */
  err = CheckLevelTreeUpmixType(&self->createParams, pSpatialSpecificConfig,
                                self->decoderLevel, (UPMIXTYPE)upmixType);
  if (err != MPS_OK) {
    goto bail;
  }

  /* Store and update instance after all checks passed successfully: */
  self->upmixType = (UPMIXTYPE)upmixType;

  if (initFlags & MPEGS_INIT_PARAMS_ERROR_CONCEALMENT) { /* At least one error
                                                            concealment
                                                            parameter changed */
    err = SpatialDecConcealment_SetParam(
        &self->concealInfo, SAC_DEC_CONCEAL_METHOD, pUserParams->concealMethod);
    if (err != MPS_OK) {
      goto bail;
    }
    err = SpatialDecConcealment_SetParam(&self->concealInfo,
                                         SAC_DEC_CONCEAL_NUM_KEEP_FRAMES,
                                         pUserParams->concealNumKeepFrames);
    if (err != MPS_OK) {
      goto bail;
    }
    err = SpatialDecConcealment_SetParam(
        &self->concealInfo, SAC_DEC_CONCEAL_FADE_OUT_SLOPE_LENGTH,
        pUserParams->concealFadeOutSlopeLength);
    if (err != MPS_OK) {
      goto bail;
    }
    err = SpatialDecConcealment_SetParam(&self->concealInfo,
                                         SAC_DEC_CONCEAL_FADE_IN_SLOPE_LENGTH,
                                         pUserParams->concealFadeInSlopeLength);
    if (err != MPS_OK) {
      goto bail;
    }
    err = SpatialDecConcealment_SetParam(&self->concealInfo,
                                         SAC_DEC_CONCEAL_NUM_RELEASE_FRAMES,
                                         pUserParams->concealNumReleaseFrames);
    if (err != MPS_OK) {
      goto bail;
    }
  }

  if (initFlags &
      MPEGS_INIT_STATES_ERROR_CONCEALMENT) { /* Set to default state */
    SpatialDecConcealment_Init(&self->concealInfo, MPEGS_CONCEAL_RESET_STATE);
  }

  /* determine bypass mode */
  bypassMode |= pUserParams->bypassMode;
  bypassMode |= ((self->upmixType == UPMIXTYPE_BYPASS) ? 1 : 0);

  /* static decoder scale depends on number of qmf bands */
  switch (nQmfBands) {
    case 16:
    case 24:
    case 32:
      self->staticDecScale = 21;
      break;
    case 64:
      self->staticDecScale = 22;
      break;
    default:
      return MPS_INVALID_PARAMETER;
  }

  self->numParameterSetsPrev = 1;

  self->qmfBands = nQmfBands;
  /* self->hybridBands will be updated in SpatialDecDecodeHeader() below. */

  self->bShareDelayWithSBR = 0;

  err = SpatialDecDecodeHeader(self, pSpatialSpecificConfig);
  if (err != MPS_OK) {
    goto bail;
  }

  self->stereoConfigIndex = pSpatialSpecificConfig->stereoConfigIndex;

  if (initFlags & MPEGS_INIT_STATES_ANA_QMF_FILTER) {
    self->qmfInputDelayBufPos = 0;
    self->pc_filterdelay = 1; /* Division by 0 not possible */
  }

  maxQmfBands = self->qmfBands;

  /* init residual decoder */

  /* init tonality smoothing */
  if (initFlags & MPEGS_INIT_STATES_PARAM) {
    initParameterSmoothing(self);
  }

  /* init GES */
  initBBEnv(self, (initFlags & MPEGS_INIT_STATES_GES) ? 1 : 0);

  /* Clip protection is applied only for normal processing. */
  if (!isTwoChMode(self->upmixType) && !bypassMode) {
    self->staticDecScale += self->clipProtectGainSF__FDK;
  }

  {
    UINT flags = 0;
    INT initStatesFlag = (initFlags & MPEGS_INIT_STATES_ANA_QMF_FILTER) ? 1 : 0;
    INT useLdFilter =
        (self->pConfigCurrent->syntaxFlags & SACDEC_SYNTAX_LD) ? 1 : 0;

    flags = self->pQmfDomain->globalConf.flags_requested;
    flags &= (~(UINT)QMF_FLAG_LP);

    if (initStatesFlag)
      flags &= ~QMF_FLAG_KEEP_STATES;
    else
      flags |= QMF_FLAG_KEEP_STATES;

    if (useLdFilter)
      flags |= QMF_FLAG_MPSLDFB;
    else
      flags &= ~QMF_FLAG_MPSLDFB;

    self->pQmfDomain->globalConf.flags_requested = flags;
    FDK_QmfDomain_Configure(self->pQmfDomain);

    /* output scaling */
    for (nCh = 0; nCh < self->numOutputChannelsAT; nCh++) {
      int outputScale = 0, outputGain_e = 0, scale = -(8) + (1);
      FIXP_DBL outputGain_m = getChGain(self, nCh, &outputGain_e);

      if (!isTwoChMode(self->upmixType) && !bypassMode) {
        outputScale +=
            self->clipProtectGainSF__FDK; /* consider clip protection scaling at
                                             synthesis qmf */
      }

      scale += outputScale;

      qmfChangeOutScalefactor(&self->pQmfDomain->QmfDomainOut[nCh].fb, scale);
      qmfChangeOutGain(&self->pQmfDomain->QmfDomainOut[nCh].fb, outputGain_m,
                       outputGain_e);
    }
  }

  for (nCh = 0; nCh < self->numOutputChannelsAT; nCh++) {
    FDKhybridSynthesisInit(&self->hybridSynthesis[nCh], THREE_TO_TEN,
                           self->qmfBands, maxQmfBands);
  }

  /* for input, residual channels and arbitrary down-mix residual channels */
  for (nCh = 0; nCh < self->createParams.maxNumInputChannels; nCh++) {
    FDKhybridAnalysisInit(
        &self->hybridAnalysis[nCh], THREE_TO_TEN, self->qmfBands, maxQmfBands,
        (initFlags & MPEGS_INIT_STATES_ANA_HYB_FILTER) ? 1 : 0);
  }
  for (; nCh < (self->createParams.bProcResidual
                    ? (self->createParams.maxNumInputChannels +
                       self->createParams.maxNumResChannels)
                    : self->createParams.maxNumInputChannels);
       nCh++) {
    FDKhybridAnalysisInit(&self->hybridAnalysis[nCh], THREE_TO_TEN, maxQmfBands,
                          maxQmfBands, 0);
  }

  {
    for (k = 0; k < self->numDecorSignals; k++) {
      int errCode, idec;
      FDK_DECORR_TYPE decorrType = DECORR_PS;
      decorrType = DECORR_LD;
      if (self->pConfigCurrent->syntaxFlags &
          (SACDEC_SYNTAX_USAC | SACDEC_SYNTAX_RSVD50)) {
        decorrType =
            ((self->treeConfig == TREE_212) && (self->decorrType == DECORR_PS))
                ? DECORR_PS
                : DECORR_USAC;
      }
      {
        idec = k;
        if (self->pConfigCurrent->syntaxFlags & SACDEC_SYNTAX_LD) {
          if (self->treeConfig == TREE_212 && k == 0) {
            idec = 2;
          }
        }
      }
      errCode = FDKdecorrelateInit(
          &self->apDecor[k], self->hybridBands, decorrType, DUCKER_AUTOMATIC,
          self->decorrConfig, idec, 0, /* self->partiallyComplex */
          0, 0,                        /* isLegacyPS */
          (initFlags & MPEGS_INIT_STATES_DECORRELATOR) ? 1 : 0);
      if (errCode) return MPS_NOTOK;
    }
  } /* !self->partiallyComplex */

  err = initM1andM2(self, (initFlags & MPEGS_INIT_STATES_M1M2) ? 1 : 0,
                    (initFlags & MPEGS_INIT_CONFIG) ? 1 : 0);
  if (err != MPS_OK) return err;

  /* Initialization of previous frame data */
  if (initFlags & MPEGS_INIT_STATES_PARAM) {
    for (i = 0; i < self->createParams.maxNumOttBoxes; i += 1) {
      /* reset icc diff data */
      for (k = 0; k < MAX_PARAMETER_SETS; k += 1) {
        for (j = 0; j < MAX_PARAMETER_BANDS; j += 1) {
          self->ottICCdiffidx[i][k][j] = 0;
        }
      }
    }
    /* Parameter Smoothing */
    /* robustness: init with one of the values of smgTimeTable[] = {64, 128,
       256, 512} to avoid division by zero in calcFilterCoeff__FDK() */
    self->smoothState->prevSmgTime = smgTimeTable[2]; /* == 256 */
    FDKmemclear(self->smoothState->prevSmgData,
                MAX_PARAMETER_BANDS * sizeof(UCHAR));
    FDKmemclear(self->smoothState->opdLeftState__FDK,
                MAX_PARAMETER_BANDS * sizeof(FIXP_DBL));
    FDKmemclear(self->smoothState->opdRightState__FDK,
                MAX_PARAMETER_BANDS * sizeof(FIXP_DBL));
  }

  self->prevTimeSlot = -1;
  self->curTimeSlot =
      MAX_TIME_SLOTS + 1; /* Initialize with a invalid value to trigger
                             concealment if first frame has no valid data. */
  self->curPs = 0;

  subbandTPInit(self->hStpDec);

bail:
  return err;
}

void SpatialDecChannelProperties(spatialDec *self,
                                 AUDIO_CHANNEL_TYPE channelType[],
                                 UCHAR channelIndices[],
                                 const FDK_channelMapDescr *const mapDescr) {
  if ((self == NULL) || (channelType == NULL) || (channelIndices == NULL) ||
      (mapDescr == NULL)) {
    return; /* no extern buffer to be filled */
  }

  if (self->numOutputChannelsAT !=
      treePropertyTable[self->treeConfig].numOutputChannels) {
    int ch;
    /* Declare all channels to be front channels: */
    for (ch = 0; ch < self->numOutputChannelsAT; ch += 1) {
      channelType[ch] = ACT_FRONT;
      channelIndices[ch] = ch;
    }
  } else {
    /* ISO/IEC FDIS 23003-1:2006(E), page 46, Table 40 bsTreeConfig */
    switch (self->treeConfig) {
      case TREE_212:
        channelType[0] = ACT_FRONT;
        channelIndices[0] = 0;
        channelType[1] = ACT_FRONT;
        channelIndices[1] = 1;
        break;
      default:;
    }
  }
}

/*******************************************************************************
 Functionname: FDK_SpatialDecClose
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/

void FDK_SpatialDecClose(spatialDec *self) {
  if (self) {
    int k;

    if (self->apDecor != NULL) {
      for (k = 0; k < self->createParams.maxNumDecorChannels; k++) {
        FDKdecorrelateClose(&(self->apDecor[k]));
      }
      FDK_FREE_MEMORY_1D(self->apDecor);
    }
    if (self->pDecorBufferCplx != NULL) {
      FDK_FREE_MEMORY_2D(self->pDecorBufferCplx);
    }

    subbandTPDestroy(&self->hStpDec);

    FDK_FREE_MEMORY_1D(self->reshapeBBEnvState);
    FDK_FREE_MEMORY_1D(self->smoothState);

    FDK_FREE_MEMORY_2D(self->pHybridAnaStatesLFdmx);
    FDK_FREE_MEMORY_2D(self->pHybridAnaStatesHFdmx);
    FDK_FREE_MEMORY_2D(self->pHybridAnaStatesLFres);
    FDK_FREE_MEMORY_2D(self->pHybridAnaStatesHFres);
    FDK_FREE_MEMORY_1D(self->hybridAnalysis);

    FDK_FREE_MEMORY_1D(self->hybridSynthesis);

    /* The time buffer is passed to the decoder from outside to avoid copying
     * (zero copy). */
    /* FDK_FREE_MEMORY_2D(self->timeOut__FDK); */

    FDK_FREE_MEMORY_2D(self->hybOutputImagWet__FDK);
    FDK_FREE_MEMORY_2D(self->hybOutputRealWet__FDK);

    FDK_FREE_MEMORY_2D(self->hybOutputImagDry__FDK);
    FDK_FREE_MEMORY_2D(self->hybOutputRealDry__FDK);

    FDK_FREE_MEMORY_2D(self->wImag__FDK);
    FDK_FREE_MEMORY_2D(self->wReal__FDK);

    if (self->createParams.bProcResidual) {
      int i;

      for (i = 0; i < self->createParams.maxNumResChannels; i++) {
        if (self->hybResidualImag__FDK != NULL)
          FDK_FREE_MEMORY_1D(self->hybResidualImag__FDK[i]);
        if (self->hybResidualReal__FDK != NULL)
          FDK_FREE_MEMORY_1D(self->hybResidualReal__FDK[i]);
        if (self->qmfResidualImag__FDK != NULL)
          FDK_FREE_MEMORY_2D_ALIGNED(self->qmfResidualImag__FDK[i]);
        if (self->qmfResidualReal__FDK != NULL)
          FDK_FREE_MEMORY_2D_ALIGNED(self->qmfResidualReal__FDK[i]);
      }

      FDK_FREE_MEMORY_1D(self->hybResidualImag__FDK);
      FDK_FREE_MEMORY_1D(self->hybResidualReal__FDK);

      FDK_FREE_MEMORY_1D(self->qmfResidualImag__FDK);
      FDK_FREE_MEMORY_1D(self->qmfResidualReal__FDK);

    } /* self->createParams.bProcResidual */

    FDK_FREE_MEMORY_2D(self->hybInputImag__FDK);
    FDK_FREE_MEMORY_2D(self->hybInputReal__FDK);

    FDK_FREE_MEMORY_2D_ALIGNED(self->qmfInputImag__FDK);
    FDK_FREE_MEMORY_2D_ALIGNED(self->qmfInputReal__FDK);

    FDK_FREE_MEMORY_3D(self->M2ImagPrev__FDK);

    FDK_FREE_MEMORY_3D(self->M2RealPrev__FDK);

    FDK_FREE_MEMORY_3D(self->M2Imag__FDK);

    FDK_FREE_MEMORY_3D(self->M2Real__FDK);

    FDK_FREE_MEMORY_1D(self->arbdmxAlphaPrev__FDK);
    FDK_FREE_MEMORY_1D(self->arbdmxAlpha__FDK);

    FDK_FREE_MEMORY_3D(self->arbdmxGain__FDK);

    FDK_FREE_MEMORY_3D(self->ottIPD__FDK);
    FDK_FREE_MEMORY_3D(self->ottICC__FDK);
    FDK_FREE_MEMORY_3D(self->ottCLD__FDK);

    /* Last parameters from prev frame */
    FDK_FREE_MEMORY_2D(self->ottCLDidxPrev);
    FDK_FREE_MEMORY_2D(self->ottICCidxPrev);
    FDK_FREE_MEMORY_3D(self->ottICCdiffidx);
    FDK_FREE_MEMORY_2D(self->ottIPDidxPrev);
    FDK_FREE_MEMORY_2D(self->arbdmxGainIdxPrev);

    FDK_FREE_MEMORY_2D(self->cmpOttCLDidxPrev);
    FDK_FREE_MEMORY_2D(self->cmpOttICCidxPrev);
    FDK_FREE_MEMORY_3D(self->outIdxData);
    FDK_FREE_MEMORY_2D(self->cmpOttIPDidxPrev);
    FDK_FREE_MEMORY_2D(self->cmpArbdmxGainIdxPrev);

    FDK_FREE_MEMORY_2D(self->smgData);
    FDK_FREE_MEMORY_1D(self->smgTime);

    FDK_FREE_MEMORY_1D(self->numOttBands);

    FDK_FREE_MEMORY_1D(self->param2hyb);

    FDK_FREE_MEMORY_1D(self);
  }

  return;
}

/**
 * \brief Apply Surround bypass buffer copies
 * \param self spatialDec handle
 * \param hybInputReal
 * \param hybInputImag
 * \param hybOutputReal
 * \param hybOutputImag
 * \param numInputChannels amount if input channels available in hybInputReal
 * and hybInputImag, which may differ from self->numInputChannels.
 */
static void SpatialDecApplyBypass(spatialDec *self, FIXP_DBL **hybInputReal,
                                  FIXP_DBL **hybInputImag,
                                  FIXP_DBL **hybOutputReal,
                                  FIXP_DBL **hybOutputImag,
                                  const int numInputChannels) {
  int complexHybBands;

  complexHybBands = self->hybridBands;

  {
    int ch;
    int rf = -1, lf = -1, cf = -1; /* Right Front, Left Front, Center Front */

    /* Determine output channel indices according to tree config */
    switch (self->treeConfig) {
      case TREE_212: /* 212  */
        lf = 0;
        rf = 1;
        break;
      default:;
    }

    /* Note: numInputChannels might not match the tree config ! */
    switch (numInputChannels) {
      case 1:
        if (cf > 0) {
          FDKmemcpy(hybOutputReal[cf], hybInputReal[0],
                    self->hybridBands * sizeof(FIXP_DBL));
          FDKmemcpy(hybOutputImag[cf], hybInputImag[0],
                    complexHybBands * sizeof(FIXP_DBL));
        } else {
          FDKmemcpy(hybOutputReal[lf], hybInputReal[0],
                    self->hybridBands * sizeof(FIXP_DBL));
          FDKmemcpy(hybOutputReal[rf], hybInputReal[0],
                    self->hybridBands * sizeof(FIXP_DBL));
          FDKmemcpy(hybOutputImag[lf], hybInputImag[0],
                    complexHybBands * sizeof(FIXP_DBL));
          FDKmemcpy(hybOutputImag[rf], hybInputImag[0],
                    complexHybBands * sizeof(FIXP_DBL));
        }
        break;
      case 2:
        FDK_ASSERT(lf != -1);
        FDK_ASSERT(rf != -1);
        FDKmemcpy(hybOutputReal[lf], hybInputReal[0],
                  self->hybridBands * sizeof(FIXP_DBL));
        FDKmemcpy(hybOutputReal[rf], hybInputReal[1],
                  self->hybridBands * sizeof(FIXP_DBL));
        FDKmemcpy(hybOutputImag[lf], hybInputImag[0],
                  complexHybBands * sizeof(FIXP_DBL));
        FDKmemcpy(hybOutputImag[rf], hybInputImag[1],
                  complexHybBands * sizeof(FIXP_DBL));
        break;
    }
    for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
      if (ch == lf || ch == rf || ch == cf) {
        continue; /* Skip bypassed channels */
      }
      FDKmemclear(hybOutputReal[ch], self->hybridBands * sizeof(FIXP_DBL));
      FDKmemclear(hybOutputImag[ch], complexHybBands * sizeof(FIXP_DBL));
    }
  }
}

/**
 * \brief Set internal error and reset error status
 *
 * \param self         spatialDec handle.
 * \param bypassMode   pointer to bypassMode.
 * \param err          error status.
 *
 * \return  error status.
 */
static SACDEC_ERROR SpatialDecSetInternalError(spatialDec *self,
                                               int *bypassMode,
                                               SACDEC_ERROR err) {
  *bypassMode = 1;

  if (self->errInt == MPS_OK) {
    /* store internal error before it gets overwritten */
    self->errInt = err;
  }

  return MPS_OK;
}

/*******************************************************************************
 Functionname: SpatialDecApplyParameterSets
 *******************************************************************************

 Description:

 Arguments:

 Return:

*******************************************************************************/
static SACDEC_ERROR SpatialDecApplyParameterSets(
    spatialDec *self, const SPATIAL_BS_FRAME *frame, SPATIALDEC_INPUT_MODE mode,
    PCM_MPS *inData,          /* Time domain input  */
    FIXP_DBL **qmfInDataReal, /* QMF domain data l/r */
    FIXP_DBL **qmfInDataImag, /* QMF domain data l/r */
    UINT nSamples, UINT controlFlags, int numInputChannels,
    const FDK_channelMapDescr *const mapDescr) {
  SACDEC_ERROR err = MPS_OK;

  FIXP_SGL alpha = FL2FXCONST_SGL(0.0);

  int ts;
  int ch;
  int hyb;

  int prevSlot = self->prevTimeSlot;
  int ps = self->curPs;
  int ts_io = 0; /* i/o dependent slot */
  int bypassMode = (controlFlags & MPEGS_BYPASSMODE) ? 1 : 0;

  /* Bypass can be triggered by the upmixType, too. */
  bypassMode |= ((self->upmixType == UPMIXTYPE_BYPASS) ? 1 : 0);

  /*
   * Decode available slots
   */
  for (ts = self->curTimeSlot;
       ts <= fixMin(self->curTimeSlot + (int)nSamples / self->qmfBands - 1,
                    self->timeSlots - 1);
       ts++, ts_io++) {
    int currSlot = frame->paramSlot[ps];

    err = (currSlot < ts) ? MPS_WRONG_PARAMETERSETS : MPS_OK;
    if (err != MPS_OK) {
      err = SpatialDecSetInternalError(self, &bypassMode, err);
    }

    /*
     * Get new parameter set
     */
    if (ts == prevSlot + 1) {
      if (bypassMode == 0) {
        err = SpatialDecCalculateM1andM2(
            self, ps, frame); /* input: ottCLD, ottICC, ... */
                              /* output: M1param(Real/Imag), M2(Real/Imag) */
        if (err != MPS_OK) {
          err = SpatialDecSetInternalError(self, &bypassMode, err);
        }
      }

      if ((ps == 0) && (self->bOverwriteM1M2prev != 0)) {
        /* copy matrix entries of M1/M2 of the first parameter set to the
           previous matrices (of the last frame). This avoids the interpolation
           of incompatible values. E.g. for residual bands the coefficients are
           calculated differently compared to non-residual bands.
         */
        SpatialDecBufferMatrices(self); /* input: M(1/2)param(Real/Imag) */
                                        /* output: M(1/2)param(Real/Imag)Prev */
        self->bOverwriteM1M2prev = 0;
      }

      if (bypassMode == 0) {
        SpatialDecSmoothM1andM2(
            self, frame,
            ps); /* input: M1param(Real/Imag)(Prev), M2(Real/Imag)(Prev) */
      }          /* output: M1param(Real/Imag), M2(Real/Imag) */
    }

    if (bypassMode == 0) {
      alpha = FX_DBL2FX_SGL(fDivNorm(ts - prevSlot, currSlot - prevSlot));
    }

    switch (mode) {
      case INPUTMODE_QMF_SBR:
        if (self->pConfigCurrent->syntaxFlags & SACDEC_SYNTAX_LD)
          self->bShareDelayWithSBR = 0; /* We got no hybrid delay */
        else
          self->bShareDelayWithSBR = 1;
        SpatialDecFeedQMF(
            self, qmfInDataReal, qmfInDataImag, ts_io, bypassMode,
            self->qmfInputReal__FDK, self->qmfInputImag__FDK,
            (bypassMode) ? numInputChannels : self->numInputChannels);
        break;
      case INPUTMODE_TIME:
        self->bShareDelayWithSBR = 0;
        SpatialDecQMFAnalysis(
            self, inData, ts_io, bypassMode, self->qmfInputReal__FDK,
            self->qmfInputImag__FDK,
            (bypassMode) ? numInputChannels : self->numInputChannels);
        break;
      default:
        break;
    }

    if ((self->pConfigCurrent->syntaxFlags & SACDEC_SYNTAX_USAC) &&
        self->residualCoding) {
      int offset;
      ch = 1;

      offset = self->pQmfDomain->globalConf.nBandsSynthesis *
               self->pQmfDomain->globalConf.nQmfTimeSlots;

      {
        const PCM_MPS *inSamples =
            &inData[ts * self->pQmfDomain->globalConf.nBandsAnalysis];

        CalculateSpaceAnalysisQmf(
            &self->pQmfDomain->QmfDomainIn[ch].fb, inSamples + (ch * offset),
            self->qmfResidualReal__FDK[0][0], self->qmfResidualImag__FDK[0][0]);

        if (!isTwoChMode(self->upmixType) && !bypassMode) {
          int i;
          FIXP_DBL *RESTRICT self_qmfResidualReal__FDK_0_0 =
              &self->qmfResidualReal__FDK[0][0][0];
          FIXP_DBL *RESTRICT self_qmfResidualImag__FDK_0_0 =
              &self->qmfResidualImag__FDK[0][0][0];

          if ((self->pQmfDomain->globalConf.nBandsAnalysis == 24) &&
              !(self->stereoConfigIndex == 3)) {
            for (i = 0; i < self->qmfBands; i++) {
              self_qmfResidualReal__FDK_0_0[i] =
                  fMult(scaleValueSaturate(self_qmfResidualReal__FDK_0_0[i],
                                           1 + self->sacInDataHeadroom - (1)),
                        self->clipProtectGain__FDK);
              self_qmfResidualImag__FDK_0_0[i] =
                  fMult(scaleValueSaturate(self_qmfResidualImag__FDK_0_0[i],
                                           1 + self->sacInDataHeadroom - (1)),
                        self->clipProtectGain__FDK);
            }
          } else {
            for (i = 0; i < self->qmfBands; i++) {
              self_qmfResidualReal__FDK_0_0[i] =
                  fMult(scaleValueSaturate(self_qmfResidualReal__FDK_0_0[i],
                                           self->sacInDataHeadroom - (1)),
                        self->clipProtectGain__FDK);
              self_qmfResidualImag__FDK_0_0[i] =
                  fMult(scaleValueSaturate(self_qmfResidualImag__FDK_0_0[i],
                                           self->sacInDataHeadroom - (1)),
                        self->clipProtectGain__FDK);
            }
          }
        }
      }
    }

    SpatialDecHybridAnalysis(
        self, /* input: qmfInput(Real/Imag), qmfResidual(Real/Imag) */
        self->qmfInputReal__FDK, self->qmfInputImag__FDK,
        self->hybInputReal__FDK, self->hybInputImag__FDK, ts, numInputChannels);

    if (bypassMode) {
      SpatialDecApplyBypass(
          self, self->hybInputReal__FDK, /* input: hybInput(Real/Imag) */
          self->hybInputImag__FDK,
          self->hybOutputRealDry__FDK, /* output: hybOutput(Real/Imag)Dry */
          self->hybOutputImagDry__FDK, numInputChannels);
    } else /* !bypassMode */
    {
      FIXP_DBL *pxReal[MAX_NUM_XCHANNELS] = {NULL};
      FIXP_DBL *pxImag[MAX_NUM_XCHANNELS] = {NULL};

      SpatialDecCreateX(self,
                        self->hybInputReal__FDK, /* input: hybInput(Real/Imag),
                                                    hybResidual(Real/Imag) */
                        self->hybInputImag__FDK, pxReal, pxImag);

      {
        SpatialDecApplyM1_CreateW_Mode212(
            self, frame, pxReal, pxImag,
            self->wReal__FDK, /* output: w(Real/Imag) */
            self->wImag__FDK);
      }
      if (err != MPS_OK) goto bail;

      int applyM2Config = APPLY_M2_NONE;

      applyM2Config = APPLY_M2;
      if ((self->pConfigCurrent->syntaxFlags &
           (SACDEC_SYNTAX_USAC | SACDEC_SYNTAX_RSVD50)) &&
          (self->tempShapeConfig != 1) && (self->tempShapeConfig != 2)) {
        if (self->phaseCoding == 3)
          applyM2Config = APPLY_M2_MODE212_Res_PhaseCoding;
        else
          applyM2Config = APPLY_M2_MODE212;
      }

      switch (applyM2Config) {
        case APPLY_M2_MODE212: {
          err = SpatialDecApplyM2_Mode212(
              self, ps, alpha, self->wReal__FDK, self->wImag__FDK,
              self->hybOutputRealDry__FDK, self->hybOutputImagDry__FDK);
        } break;
        case APPLY_M2_MODE212_Res_PhaseCoding:
          err = SpatialDecApplyM2_Mode212_ResidualsPlusPhaseCoding(
              self, ps, alpha, self->wReal__FDK, self->wImag__FDK,
              self->hybOutputRealDry__FDK, self->hybOutputImagDry__FDK);
          break;
        case APPLY_M2:
          err = SpatialDecApplyM2(
              self, ps, alpha, self->wReal__FDK, self->wImag__FDK,
              self->hybOutputRealDry__FDK, self->hybOutputImagDry__FDK,
              self->hybOutputRealWet__FDK, self->hybOutputImagWet__FDK);
          break;
        default:
          err = MPS_APPLY_M2_ERROR;
          goto bail;
      }

      if (err != MPS_OK) goto bail;

      if ((self->tempShapeConfig == 2) && (!isTwoChMode(self->upmixType))) {
        SpatialDecReshapeBBEnv(self, frame,
                               ts); /* input: reshapeBBEnvState,
                                       hybOutput(Real/Imag)(Dry/Wet),
                                       hybInput(Real/Imag) */
      }                             /* output: hybOutput(Real/Imag)Dry */

      /* Merge parts of the dry and wet QMF buffers. */
      if ((self->tempShapeConfig == 1) && (!isTwoChMode(self->upmixType))) {
        for (ch = 0; ch < self->numOutputChannels; ch++) {
          for (hyb = 0; hyb < self->tp_hybBandBorder; hyb++) {
            self->hybOutputRealDry__FDK[ch][hyb] =
                fAddSaturate(self->hybOutputRealDry__FDK[ch][hyb],
                             self->hybOutputRealWet__FDK[ch][hyb]);
            self->hybOutputImagDry__FDK[ch][hyb] =
                fAddSaturate(self->hybOutputImagDry__FDK[ch][hyb],
                             self->hybOutputImagWet__FDK[ch][hyb]);
          } /* loop hyb */
        }   /* loop ch */
        err = subbandTPApply(
            self, frame); /* input: hStpDec, hybOutput(Real/Imag)Dry/Wet */
                          /* output: hStpDec, hybOutput(Real/Imag)Dry */
        if (err != MPS_OK) goto bail;
      } /* (self->tempShapeConfig == 1) */
      else {
        /* The wet signal is added to the dry signal in applyM2 if GES and STP
         * are disabled */
        if ((self->tempShapeConfig == 1) || (self->tempShapeConfig == 2)) {
          int nHybBands;
          nHybBands = self->hybridBands;

          for (ch = 0; ch < self->numOutputChannels; ch++) {
            FIXP_DBL *RESTRICT pRealDry = self->hybOutputRealDry__FDK[ch];
            FIXP_DBL *RESTRICT pImagDry = self->hybOutputImagDry__FDK[ch];
            FIXP_DBL *RESTRICT pRealWet = self->hybOutputRealWet__FDK[ch];
            FIXP_DBL *RESTRICT pImagWet = self->hybOutputImagWet__FDK[ch];
            for (hyb = 0; hyb < nHybBands; hyb++) {
              pRealDry[hyb] = fAddSaturate(pRealDry[hyb], pRealWet[hyb]);
              pImagDry[hyb] = fAddSaturate(pImagDry[hyb], pImagWet[hyb]);
            } /* loop hyb */
            for (; hyb < self->hybridBands; hyb++) {
              pRealDry[hyb] = fAddSaturate(pRealDry[hyb], pRealWet[hyb]);
            } /* loop hyb */
          }   /* loop ch */
        } /* ( self->tempShapeConfig == 1 ) || ( self->tempShapeConfig == 2 ) */
      }   /* !self->tempShapeConfig == 1 */
    }     /*  !bypassMode */

    if ((self->phaseCoding == 1) && (bypassMode == 0)) {
      /* only if bsPhaseCoding == 1 and bsResidualCoding == 0 */

      SpatialDecApplyPhase(
          self, alpha, (ts == currSlot) /* signal the last slot of the set */
      );
    }

    /*
     * Synthesis Filtering
     */

    err = SpatialDecSynthesis(
        self, ts_io,
        self->hybOutputRealDry__FDK, /* input: hybOutput(Real/Imag)Dry */
        self->hybOutputImagDry__FDK, self->timeOut__FDK, /* output: timeOut */
        numInputChannels, mapDescr);

    if (err != MPS_OK) goto bail;

    /*
     * Update parameter buffer
     */
    if (ts == currSlot) {
      SpatialDecBufferMatrices(self); /* input: M(1/2)param(Real/Imag) */
                                      /* output: M(1/2)param(Real/Imag)Prev */

      prevSlot = currSlot;
      ps++;
    } /* if (ts==currSlot) */

  } /* ts loop */

  /*
   * Save parameter states
   */
  self->prevTimeSlot = prevSlot;
  self->curTimeSlot = ts;
  self->curPs = ps;

bail:

  return err;
}

SACDEC_ERROR SpatialDecApplyFrame(
    spatialDec *self,
    SPATIAL_BS_FRAME *frame, /* parsed frame data to be applied */
    SPATIALDEC_INPUT_MODE inputMode, PCM_MPS *inData, /* Time domain input  */
    FIXP_DBL **qmfInDataReal,                         /* QMF domain data l/r */
    FIXP_DBL **qmfInDataImag,                         /* QMF domain data l/r */
    PCM_MPS *pcmOutBuf, /* MAX_OUTPUT_CHANNELS*MAX_TIME_SLOTS*NUM_QMF_BANDS] */
    UINT nSamples, UINT *pControlFlags, int numInputChannels,
    const FDK_channelMapDescr *const mapDescr) {
  SACDEC_ERROR err = MPS_OK;

  int fDecAndMapFrameData;
  int controlFlags;

  FDK_ASSERT(self != NULL);
  FDK_ASSERT(pControlFlags != NULL);
  FDK_ASSERT(pcmOutBuf != NULL);
  FDK_ASSERT(self->sacInDataHeadroom >= (1));

  self->errInt = err; /* Init internal error */

  controlFlags = *pControlFlags;

  if ((self->pConfigCurrent->syntaxFlags & SACDEC_SYNTAX_USAC) &&
      (self->stereoConfigIndex > 1)) {
    numInputChannels =
        1; /* Do not count residual channel as input channel. It is handled
              seperately. */
  }

  /* Check if input amount of channels is consistent */
  if (numInputChannels != self->numInputChannels) {
    controlFlags |= MPEGS_CONCEAL;
    if (numInputChannels > self->createParams.maxNumInputChannels) {
      return MPS_INVALID_PARAMETER;
    }
  }

  self->timeOut__FDK = pcmOutBuf;

  /* Determine local function control flags */
  fDecAndMapFrameData = frame->newBsData;

  if (((fDecAndMapFrameData ==
        0) /* assures that conceal flag will not be set for blind mode */
       && (self->curTimeSlot + (int)nSamples / self->qmfBands >
           self->timeSlots)) ||
      (frame->numParameterSets ==
       0)) { /* New input samples but missing side info */
    fDecAndMapFrameData = 1;
    controlFlags |= MPEGS_CONCEAL;
  }

  if ((fDecAndMapFrameData == 0) &&
      (frame->paramSlot[fMax(0, frame->numParameterSets - 1)] !=
           (self->timeSlots - 1) ||
       self->curTimeSlot >
           frame->paramSlot[self->curPs])) { /* Detected faulty parameter slot
                                                data. */
    fDecAndMapFrameData = 1;
    controlFlags |= MPEGS_CONCEAL;
  }

  /* Update concealment state machine */
  SpatialDecConcealment_UpdateState(
      &self->concealInfo,
      (controlFlags & MPEGS_CONCEAL)
          ? 0
          : 1); /* convert from conceal flag to frame ok flag */

  if (fDecAndMapFrameData) {
    /* Reset spatial framing control vars */
    frame->newBsData = 0;
    self->prevTimeSlot = -1;
    self->curTimeSlot = 0;
    self->curPs = 0;

    if (controlFlags & MPEGS_CONCEAL) {
      /* Reset frame data to avoid misconfiguration. */
      SpatialDecClearFrameData(self, frame, &self->createParams);
    }

    {
      err = SpatialDecDecodeFrame(self, frame); /* input: ... */
      /* output: decodeAndMapFrameDATA */
    }

    if (err != MPS_OK) {
      /* Rescue strategy is to apply bypass mode in order
         to keep at least the downmix channels continuous. */
      controlFlags |= MPEGS_CONCEAL;
      if (self->errInt == MPS_OK) {
        /* store internal error befor it gets overwritten */
        self->errInt = err;
      }
    }
  }

  err = SpatialDecApplyParameterSets(
      self, frame, inputMode, inData, qmfInDataReal, qmfInDataImag, nSamples,
      controlFlags | ((err == MPS_OK) ? 0 : MPEGS_BYPASSMODE), numInputChannels,
      mapDescr);
  if (err != MPS_OK) {
    goto bail;
  }

bail:

  *pControlFlags = controlFlags;

  return err;
}
