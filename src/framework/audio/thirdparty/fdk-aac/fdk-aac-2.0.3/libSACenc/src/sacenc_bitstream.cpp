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

   Author(s):

   Description: Encoder Library Interface
                Bitstream Writer

*******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_bitstream.h"
#include "sacenc_const.h"

#include "genericStds.h"
#include "common_fix.h"

#include "FDK_matrixCalloc.h"
#include "sacenc_nlc_enc.h"

/* Defines *******************************************************************/
#define MAX_FREQ_RES_INDEX 8
#define MAX_SAMPLING_FREQUENCY_INDEX 13
#define SAMPLING_FREQUENCY_INDEX_ESCAPE 15

/* Data Types ****************************************************************/
typedef struct {
  SCHAR cld_old[SACENC_MAX_NUM_BOXES][MAX_NUM_BINS];
  SCHAR icc_old[SACENC_MAX_NUM_BOXES][MAX_NUM_BINS];
  UCHAR quantCoarseCldPrev[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];
  UCHAR quantCoarseIccPrev[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];

} PREV_OTTDATA;

typedef struct {
  PREV_OTTDATA prevOttData;

} STATIC_SPATIALFRAME;

typedef struct BSF_INSTANCE {
  SPATIALSPECIFICCONFIG spatialSpecificConfig;
  SPATIALFRAME frame;
  STATIC_SPATIALFRAME prevFrameData;

} BSF_INSTANCE;

/* Constants *****************************************************************/
static const INT SampleRateTable[MAX_SAMPLING_FREQUENCY_INDEX] = {
    96000, 88200, 64000, 48000, 44100, 32000, 24000,
    22050, 16000, 12000, 11025, 8000,  7350};

static const UCHAR FreqResBinTable_LD[MAX_FREQ_RES_INDEX] = {0, 23, 15, 12,
                                                             9, 7,  5,  4};
static const UCHAR FreqResStrideTable_LD[] = {1, 2, 5, 23};

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
static FDK_SACENC_ERROR DuplicateLosslessData(
    const INT startBox, const INT stopBox,
    const LOSSLESSDATA *const hLosslessDataFrom, const INT setFrom,
    LOSSLESSDATA *const hLosslessDataTo, const INT setTo) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL == hLosslessDataFrom) || (NULL == hLosslessDataTo)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i;

    for (i = startBox; i < stopBox; i++) {
      hLosslessDataTo->bsXXXDataMode[i][setTo] =
          hLosslessDataFrom->bsXXXDataMode[i][setFrom];
      hLosslessDataTo->bsDataPair[i][setTo] =
          hLosslessDataFrom->bsDataPair[i][setFrom];
      hLosslessDataTo->bsQuantCoarseXXX[i][setTo] =
          hLosslessDataFrom->bsQuantCoarseXXX[i][setFrom];
      hLosslessDataTo->bsFreqResStrideXXX[i][setTo] =
          hLosslessDataFrom->bsFreqResStrideXXX[i][setFrom];
    }
  }
  return error;
}

FDK_SACENC_ERROR fdk_sacenc_duplicateParameterSet(
    const SPATIALFRAME *const hFrom, const INT setFrom, SPATIALFRAME *const hTo,
    const INT setTo) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL == hFrom) || (NULL == hTo)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int box;
    /* Only Copy Parameter Set selective stuff */

    /* OTT-Data */
    for (box = 0; box < SACENC_MAX_NUM_BOXES; box++) {
      FDKmemcpy(hTo->ottData.cld[box][setTo], hFrom->ottData.cld[box][setFrom],
                sizeof(hFrom->ottData.cld[0][0]));
      FDKmemcpy(hTo->ottData.icc[box][setTo], hFrom->ottData.icc[box][setFrom],
                sizeof(hFrom->ottData.icc[0][0]));
    }

    /* LOSSLESSDATA */
    DuplicateLosslessData(0, SACENC_MAX_NUM_BOXES, &hFrom->CLDLosslessData,
                          setFrom, &hTo->CLDLosslessData, setTo);
    DuplicateLosslessData(0, SACENC_MAX_NUM_BOXES, &hFrom->ICCLosslessData,
                          setFrom, &hTo->ICCLosslessData, setTo);

  } /* valid handle */

  return error;
}

/* set frame defaults */
static void clearFrame(SPATIALFRAME *const pFrame) {
  FDKmemclear(pFrame, sizeof(SPATIALFRAME));

  pFrame->bsIndependencyFlag = 1;
  pFrame->framingInfo.numParamSets = 1;
}

static void fine2coarse(SCHAR *const data, const DATA_TYPE dataType,
                        const INT startBand, const INT numBands) {
  int i;
  if (dataType == t_CLD) {
    for (i = startBand; i < startBand + numBands; i++) {
      data[i] /= 2;
    }
  } else {
    for (i = startBand; i < startBand + numBands; i++) {
      data[i] >>= 1;
    }
  }
}

static void coarse2fine(SCHAR *const data, const DATA_TYPE dataType,
                        const INT startBand, const INT numBands) {
  int i;

  for (i = startBand; i < startBand + numBands; i++) {
    data[i] <<= 1;
  }

  if (dataType == t_CLD) {
    for (i = startBand; i < startBand + numBands; i++) {
      if (data[i] == -14) {
        data[i] = -15;
      } else if (data[i] == 14) {
        data[i] = 15;
      }
    }
  } /* (dataType == t_CLD) */
}

static UCHAR getBsFreqResStride(const INT index) {
  const UCHAR *pFreqResStrideTable = NULL;
  int freqResStrideTableSize = 0;

  pFreqResStrideTable = FreqResStrideTable_LD;
  freqResStrideTableSize =
      sizeof(FreqResStrideTable_LD) / sizeof(*FreqResStrideTable_LD);

  return (((NULL != pFreqResStrideTable) && (index >= 0) &&
           (index < freqResStrideTableSize))
              ? pFreqResStrideTable[index]
              : 1);
}

/* write data to bitstream */
static void ecData(HANDLE_FDK_BITSTREAM bitstream,
                   SCHAR data[MAX_NUM_PARAMS][MAX_NUM_BINS],
                   SCHAR oldData[MAX_NUM_BINS],
                   UCHAR quantCoarseXXXprev[MAX_NUM_PARAMS],
                   LOSSLESSDATA *const losslessData, const DATA_TYPE dataType,
                   const INT paramIdx, const INT numParamSets,
                   const INT independencyFlag, const INT startBand,
                   const INT stopBand, const INT defaultValue) {
  int ps, pb, strOffset, pbStride, dataBands, i;
  int aStrides[MAX_NUM_BINS + 1] = {0};
  SHORT cmpIdxData[2][MAX_NUM_BINS] = {{0}};
  SHORT cmpOldData[MAX_NUM_BINS] = {0};

  /* bsXXXDataMode */
  if (independencyFlag || (losslessData->bsQuantCoarseXXX[paramIdx][0] !=
                           quantCoarseXXXprev[paramIdx])) {
    losslessData->bsXXXDataMode[paramIdx][0] = FINECOARSE;
  } else {
    losslessData->bsXXXDataMode[paramIdx][0] = KEEP;
    for (i = startBand; i < stopBand; i++) {
      if (data[0][i] != oldData[i]) {
        losslessData->bsXXXDataMode[paramIdx][0] = FINECOARSE;
        break;
      }
    }
  }

  FDKwriteBits(bitstream, losslessData->bsXXXDataMode[paramIdx][0], 2);

  for (ps = 1; ps < numParamSets; ps++) {
    if (losslessData->bsQuantCoarseXXX[paramIdx][ps] !=
        losslessData->bsQuantCoarseXXX[paramIdx][ps - 1]) {
      losslessData->bsXXXDataMode[paramIdx][ps] = FINECOARSE;
    } else {
      losslessData->bsXXXDataMode[paramIdx][ps] = KEEP;
      for (i = startBand; i < stopBand; i++) {
        if (data[ps][i] != data[ps - 1][i]) {
          losslessData->bsXXXDataMode[paramIdx][ps] = FINECOARSE;
          break;
        }
      }
    }

    FDKwriteBits(bitstream, losslessData->bsXXXDataMode[paramIdx][ps], 2);
  } /* for ps */

  /* Create data pairs if possible */
  for (ps = 0; ps < (numParamSets - 1); ps++) {
    if (losslessData->bsXXXDataMode[paramIdx][ps] == FINECOARSE) {
      /* Check if next parameter set is FINCOARSE */
      if (losslessData->bsXXXDataMode[paramIdx][ps + 1] == FINECOARSE) {
        /* We have to check if ps and ps+1 use the same bsXXXQuantMode */
        /* and also have the same stride */
        if ((losslessData->bsQuantCoarseXXX[paramIdx][ps + 1] ==
             losslessData->bsQuantCoarseXXX[paramIdx][ps]) &&
            (losslessData->bsFreqResStrideXXX[paramIdx][ps + 1] ==
             losslessData->bsFreqResStrideXXX[paramIdx][ps])) {
          losslessData->bsDataPair[paramIdx][ps] = 1;
          losslessData->bsDataPair[paramIdx][ps + 1] = 1;

          /* We have a data pair -> Jump to the ps after next ps*/
          ps++;
          continue;
        }
      }
      /* dataMode of next ps is not FINECOARSE or does not use the same
       * bsXXXQuantMode/stride */
      /* -> no dataPair possible */
      losslessData->bsDataPair[paramIdx][ps] = 0;

      /* Initialize ps after next ps to Zero (only important for the last
       * parameter set) */
      losslessData->bsDataPair[paramIdx][ps + 1] = 0;
    } else {
      /* No FINECOARSE -> no data pair possible */
      losslessData->bsDataPair[paramIdx][ps] = 0;

      /* Initialize ps after next ps to Zero (only important for the last
       * parameter set) */
      losslessData->bsDataPair[paramIdx][ps + 1] = 0;
    }
  } /* for ps */

  for (ps = 0; ps < numParamSets; ps++) {
    if (losslessData->bsXXXDataMode[paramIdx][ps] == DEFAULT) {
      /* Prepare old data */
      for (i = startBand; i < stopBand; i++) {
        oldData[i] = defaultValue;
      }
      quantCoarseXXXprev[paramIdx] = 0; /* Default data are always fine */
    }

    if (losslessData->bsXXXDataMode[paramIdx][ps] == FINECOARSE) {
      FDKwriteBits(bitstream, losslessData->bsDataPair[paramIdx][ps], 1);
      FDKwriteBits(bitstream, losslessData->bsQuantCoarseXXX[paramIdx][ps], 1);
      FDKwriteBits(bitstream, losslessData->bsFreqResStrideXXX[paramIdx][ps],
                   2);

      if (losslessData->bsQuantCoarseXXX[paramIdx][ps] !=
          quantCoarseXXXprev[paramIdx]) {
        if (quantCoarseXXXprev[paramIdx]) {
          coarse2fine(oldData, dataType, startBand, stopBand - startBand);
        } else {
          fine2coarse(oldData, dataType, startBand, stopBand - startBand);
        }
      }

      /* Handle strides */
      pbStride =
          getBsFreqResStride(losslessData->bsFreqResStrideXXX[paramIdx][ps]);
      dataBands = (stopBand - startBand - 1) / pbStride + 1;

      aStrides[0] = startBand;
      for (pb = 1; pb <= dataBands; pb++) {
        aStrides[pb] = aStrides[pb - 1] + pbStride;
      }

      strOffset = 0;
      while (aStrides[dataBands] > stopBand) {
        if (strOffset < dataBands) {
          strOffset++;
        }
        for (i = strOffset; i <= dataBands; i++) {
          aStrides[i]--;
        }
      } /* while */

      for (pb = 0; pb < dataBands; pb++) {
        cmpOldData[startBand + pb] = oldData[aStrides[pb]];
        cmpIdxData[0][startBand + pb] = data[ps][aStrides[pb]];

        if (losslessData->bsDataPair[paramIdx][ps]) {
          cmpIdxData[1][startBand + pb] = data[ps + 1][aStrides[pb]];
        }
      } /* for pb*/

      /* Finally encode */
      if (losslessData->bsDataPair[paramIdx][ps]) {
        fdk_sacenc_ecDataPairEnc(bitstream, cmpIdxData, cmpOldData, dataType, 0,
                                 startBand, dataBands,
                                 losslessData->bsQuantCoarseXXX[paramIdx][ps],
                                 independencyFlag && (ps == 0));
      } else {
        fdk_sacenc_ecDataSingleEnc(bitstream, cmpIdxData, cmpOldData, dataType,
                                   0, startBand, dataBands,
                                   losslessData->bsQuantCoarseXXX[paramIdx][ps],
                                   independencyFlag && (ps == 0));
      }

      /* Overwrite old data */
      for (i = startBand; i < stopBand; i++) {
        if (losslessData->bsDataPair[paramIdx][ps]) {
          oldData[i] = data[ps + 1][i];
        } else {
          oldData[i] = data[ps][i];
        }
      }

      quantCoarseXXXprev[paramIdx] =
          losslessData->bsQuantCoarseXXX[paramIdx][ps];

      /* Jump forward if we have encoded a data pair */
      if (losslessData->bsDataPair[paramIdx][ps]) {
        ps++;
      }

    } /* if (losslessData->bsXXXDataMode[paramIdx][ps] == FINECOARSE ) */
  }   /* for ps */
}

/****************************************************************************/
/* Bitstream formatter interface functions                                  */
/****************************************************************************/
static FDK_SACENC_ERROR getBsFreqResIndex(const INT numBands,
                                          INT *const pbsFreqResIndex) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pbsFreqResIndex) {
    error = SACENC_INVALID_HANDLE;
  } else {
    const UCHAR *pFreqResBinTable = FreqResBinTable_LD;
    int i;
    *pbsFreqResIndex = -1;

    for (i = 0; i < MAX_FREQ_RES_INDEX; i++) {
      if (numBands == pFreqResBinTable[i]) {
        *pbsFreqResIndex = i;
        break;
      }
    }
    if (*pbsFreqResIndex < 0 || *pbsFreqResIndex >= MAX_FREQ_RES_INDEX) {
      error = SACENC_INVALID_CONFIG;
    }
  }
  return error;
}

static FDK_SACENC_ERROR getSamplingFrequencyIndex(
    const INT bsSamplingFrequency, INT *const pbsSamplingFrequencyIndex) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == pbsSamplingFrequencyIndex) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i;
    *pbsSamplingFrequencyIndex = SAMPLING_FREQUENCY_INDEX_ESCAPE;

    for (i = 0; i < MAX_SAMPLING_FREQUENCY_INDEX; i++) {
      if (bsSamplingFrequency == SampleRateTable[i]) { /*spatial sampling rate*/
        *pbsSamplingFrequencyIndex = i;
        break;
      }
    }
  }
  return error;
}

/* destroy encoder instance */
FDK_SACENC_ERROR fdk_sacenc_destroySpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE *selfPtr) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((selfPtr == NULL) || (*selfPtr == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    if (*selfPtr != NULL) {
      FDK_FREE_MEMORY_1D(*selfPtr);
    }
  }
  return error;
}

/* create encoder instance */
FDK_SACENC_ERROR fdk_sacenc_createSpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE *selfPtr) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL == selfPtr) {
    error = SACENC_INVALID_HANDLE;
  } else {
    /* allocate encoder struct */
    FDK_ALLOCATE_MEMORY_1D(*selfPtr, 1, BSF_INSTANCE);
  }
  return error;

bail:
  fdk_sacenc_destroySpatialBitstreamEncoder(selfPtr);
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

/* init encoder instance */
FDK_SACENC_ERROR fdk_sacenc_initSpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE selfPtr) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (selfPtr == NULL) {
    error = SACENC_INVALID_HANDLE;
  } else {
    /* init/clear */
    clearFrame(&selfPtr->frame);

  } /* valid handle */
  return error;
}

/* get SpatialSpecificConfig struct */
SPATIALSPECIFICCONFIG *fdk_sacenc_getSpatialSpecificConfig(
    HANDLE_BSF_INSTANCE selfPtr) {
  return ((selfPtr == NULL) ? NULL : &(selfPtr->spatialSpecificConfig));
}

/* write SpatialSpecificConfig to stream */
FDK_SACENC_ERROR fdk_sacenc_writeSpatialSpecificConfig(
    SPATIALSPECIFICCONFIG *const spatialSpecificConfig,
    UCHAR *const pOutputBuffer, const INT outputBufferSize,
    INT *const pnOutputBits) {
  FDK_SACENC_ERROR error = SACENC_OK;
  INT bsSamplingFrequencyIndex = 0;
  INT bsFreqRes = 0;

  if ((spatialSpecificConfig == NULL) || (pOutputBuffer == NULL) ||
      (pnOutputBits == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    FDK_BITSTREAM bitstream;

    /* Find FreqRes */
    if (SACENC_OK != (error = getBsFreqResIndex(spatialSpecificConfig->numBands,
                                                &bsFreqRes)))
      goto bail;

    /* Find SamplingFrequencyIndex */
    if (SACENC_OK != (error = getSamplingFrequencyIndex(
                          spatialSpecificConfig->bsSamplingFrequency,
                          &bsSamplingFrequencyIndex)))
      goto bail;

    /* bind extern buffer to bitstream handle */
    FDKinitBitStream(&bitstream, pOutputBuffer, outputBufferSize, 0, BS_WRITER);

    /****************************************************************************/
    /* write to bitstream */

    FDKwriteBits(&bitstream, bsSamplingFrequencyIndex, 4);

    if (bsSamplingFrequencyIndex == 15) {
      FDKwriteBits(&bitstream, spatialSpecificConfig->bsSamplingFrequency, 24);
    }

    FDKwriteBits(&bitstream, spatialSpecificConfig->bsFrameLength, 5);

    FDKwriteBits(&bitstream, bsFreqRes, 3);
    FDKwriteBits(&bitstream, spatialSpecificConfig->bsTreeConfig, 4);
    FDKwriteBits(&bitstream, spatialSpecificConfig->bsQuantMode, 2);

    FDKwriteBits(&bitstream, 0, 1); /* bsArbitraryDownmix */

    FDKwriteBits(&bitstream, spatialSpecificConfig->bsFixedGainDMX, 3);

    FDKwriteBits(&bitstream, TEMPSHAPE_OFF, 2);
    FDKwriteBits(&bitstream, spatialSpecificConfig->bsDecorrConfig, 2);

    FDKbyteAlign(&bitstream, 0); /* byte alignment */

    /* return number of valid bits in bitstream */
    if ((*pnOutputBits = FDKgetValidBits(&bitstream)) >
        (outputBufferSize * 8)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /* terminate buffer with alignment */
    FDKbyteAlign(&bitstream, 0);

  } /* valid handle */

bail:
  return error;
}

/* get SpatialFrame struct */
SPATIALFRAME *fdk_sacenc_getSpatialFrame(HANDLE_BSF_INSTANCE selfPtr,
                                         const SPATIALFRAME_TYPE frameType) {
  int idx = -1;

  switch (frameType) {
    case READ_SPATIALFRAME:
    case WRITE_SPATIALFRAME:
      idx = 0;
      break;
    default:
      idx = -1; /* invalid configuration */
  }             /* switch frameType */

  return (((selfPtr == NULL) || (idx == -1)) ? NULL : &selfPtr->frame);
}

static FDK_SACENC_ERROR writeFramingInfo(HANDLE_FDK_BITSTREAM hBitstream,
                                         const FRAMINGINFO *const pFramingInfo,
                                         const INT frameLength) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hBitstream == NULL) || (pFramingInfo == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    FDKwriteBits(hBitstream, pFramingInfo->bsFramingType, 1);
    FDKwriteBits(hBitstream, pFramingInfo->numParamSets - 1, 1);

    if (pFramingInfo->bsFramingType) {
      int ps = 0;
      int numParamSets = pFramingInfo->numParamSets;

      {
        for (ps = 0; ps < numParamSets; ps++) {
          int bitsParamSlot = 0;
          while ((1 << bitsParamSlot) < (frameLength + 1)) bitsParamSlot++;
          if (bitsParamSlot > 0)
            FDKwriteBits(hBitstream, pFramingInfo->bsParamSlots[ps],
                         bitsParamSlot);
        }
      }
    } /* pFramingInfo->bsFramingType */
  }   /* valid handle */

  return error;
}

static FDK_SACENC_ERROR writeSmgData(HANDLE_FDK_BITSTREAM hBitstream,
                                     const SMGDATA *const pSmgData,
                                     const INT numParamSets,
                                     const INT dataBands) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hBitstream == NULL) || (pSmgData == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i, j;

    for (i = 0; i < numParamSets; i++) {
      FDKwriteBits(hBitstream, pSmgData->bsSmoothMode[i], 2);

      if (pSmgData->bsSmoothMode[i] >= 2) {
        FDKwriteBits(hBitstream, pSmgData->bsSmoothTime[i], 2);
      }
      if (pSmgData->bsSmoothMode[i] == 3) {
        const int stride = getBsFreqResStride(pSmgData->bsFreqResStride[i]);
        FDKwriteBits(hBitstream, pSmgData->bsFreqResStride[i], 2);
        for (j = 0; j < dataBands; j += stride) {
          FDKwriteBits(hBitstream, pSmgData->bsSmgData[i][j], 1);
        }
      }
    } /* for i */
  }   /* valid handle */

  return error;
}

static FDK_SACENC_ERROR writeOttData(
    HANDLE_FDK_BITSTREAM hBitstream, PREV_OTTDATA *const pPrevOttData,
    OTTDATA *const pOttData, const OTTCONFIG ottConfig[SACENC_MAX_NUM_BOXES],
    LOSSLESSDATA *const pCLDLosslessData, LOSSLESSDATA *const pICCLosslessData,
    const INT numOttBoxes, const INT numBands, const INT numParamSets,
    const INT bsIndependencyFlag) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hBitstream == NULL) || (pPrevOttData == NULL) || (pOttData == NULL) ||
      (ottConfig == NULL) || (pCLDLosslessData == NULL) ||
      (pICCLosslessData == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i;
    for (i = 0; i < numOttBoxes; i++) {
      ecData(hBitstream, pOttData->cld[i], pPrevOttData->cld_old[i],
             pPrevOttData->quantCoarseCldPrev[i], pCLDLosslessData, t_CLD, i,
             numParamSets, bsIndependencyFlag, 0, ottConfig[i].bsOttBands, 15);
    }
    {
      for (i = 0; i < numOttBoxes; i++) {
        {
          ecData(hBitstream, pOttData->icc[i], pPrevOttData->icc_old[i],
                 pPrevOttData->quantCoarseIccPrev[i], pICCLosslessData, t_ICC,
                 i, numParamSets, bsIndependencyFlag, 0, numBands, 0);
        }
      } /* for i */
    }
  } /* valid handle */

  return error;
}

/* write extension frame data to stream */
static FDK_SACENC_ERROR WriteSpatialExtensionFrame(
    HANDLE_FDK_BITSTREAM bitstream, HANDLE_BSF_INSTANCE self) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((bitstream == NULL) || (self == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    FDKbyteAlign(bitstream, 0);
  } /* valid handle */

  return error;
}

/* write frame data to stream */
FDK_SACENC_ERROR fdk_sacenc_writeSpatialFrame(UCHAR *const pOutputBuffer,
                                              const INT outputBufferSize,
                                              INT *const pnOutputBits,
                                              HANDLE_BSF_INSTANCE selfPtr) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((pOutputBuffer == NULL) || (pnOutputBits == NULL) || (selfPtr == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    SPATIALFRAME *frame = NULL;
    SPATIALSPECIFICCONFIG *config = NULL;
    FDK_BITSTREAM bitstream;

    int i, j, numParamSets, numOttBoxes;

    if ((NULL ==
         (frame = fdk_sacenc_getSpatialFrame(selfPtr, READ_SPATIALFRAME))) ||
        (NULL == (config = &(selfPtr->spatialSpecificConfig)))) {
      error = SACENC_INVALID_HANDLE;
      goto bail;
    }

    numOttBoxes = selfPtr->spatialSpecificConfig.treeDescription.numOttBoxes;

    numParamSets = frame->framingInfo.numParamSets;

    if (frame->bUseBBCues) {
      for (i = 0; i < SACENC_MAX_NUM_BOXES; i++) {
        /* If a transient was detected, force only the second ps broad band */
        if (numParamSets == 1) {
          frame->CLDLosslessData.bsFreqResStrideXXX[i][0] = 3;
          frame->ICCLosslessData.bsFreqResStrideXXX[i][0] = 3;
        } else {
          for (j = 1; j < MAX_NUM_PARAMS; j++) {
            frame->CLDLosslessData.bsFreqResStrideXXX[i][j] = 3;
            frame->ICCLosslessData.bsFreqResStrideXXX[i][j] = 3;
          }
        }
      }
    } /* frame->bUseBBCues */

    /* bind extern buffer to bitstream handle */
    FDKinitBitStream(&bitstream, pOutputBuffer, outputBufferSize, 0, BS_WRITER);

    if (SACENC_OK != (error = writeFramingInfo(
                          &bitstream, &(frame->framingInfo),
                          selfPtr->spatialSpecificConfig.bsFrameLength))) {
      goto bail;
    }

    /* write bsIndependencyFlag */
    FDKwriteBits(&bitstream, frame->bsIndependencyFlag, 1);

    /* write spatial data to bitstream */
    if (SACENC_OK !=
        (error = writeOttData(&bitstream, &selfPtr->prevFrameData.prevOttData,
                              &frame->ottData, config->ottConfig,
                              &frame->CLDLosslessData, &frame->ICCLosslessData,
                              numOttBoxes, config->numBands, numParamSets,
                              frame->bsIndependencyFlag))) {
      goto bail;
    }
    if (SACENC_OK != (error = writeSmgData(&bitstream, &frame->smgData,
                                           numParamSets, config->numBands))) {
      goto bail;
    }

    /* byte alignment */
    FDKbyteAlign(&bitstream, 0);

    /* Write SpatialExtensionFrame */
    if (SACENC_OK !=
        (error = WriteSpatialExtensionFrame(&bitstream, selfPtr))) {
      goto bail;
    }

    if (NULL ==
        (frame = fdk_sacenc_getSpatialFrame(selfPtr, WRITE_SPATIALFRAME))) {
      error = SACENC_INVALID_HANDLE;
      goto bail;
    }

    clearFrame(frame);

    /* return number of valid bits in bitstream */
    if ((*pnOutputBits = FDKgetValidBits(&bitstream)) >
        (outputBufferSize * 8)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /* terminate buffer with alignment */
    FDKbyteAlign(&bitstream, 0);

  } /* valid handle */

bail:
  return error;
}
