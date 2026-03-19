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
                Interface to Spacial Audio Coding Encoder lib

*******************************************************************************/

/****************************************************************************
\file
Description of file contents
******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_lib.h"
#include "sacenc_const.h"
#include "genericStds.h"
#include "FDK_core.h"
#include "sacenc_tree.h"
#include "sacenc_bitstream.h"
#include "sacenc_onsetdetect.h"
#include "sacenc_framewindowing.h"
#include "sacenc_filter.h"
#include "sacenc_paramextract.h"
#include "sacenc_staticgain.h"
#include "sacenc_delay.h"
#include "sacenc_dmx_tdom_enh.h"
#include "sacenc_vectorfunctions.h"
#include "qmf.h"

/* Defines *******************************************************************/

/* Encoder library info */
#define SACENC_LIB_VL0 2
#define SACENC_LIB_VL1 0
#define SACENC_LIB_VL2 0
#define SACENC_LIB_TITLE "MPEG Surround Encoder"
#ifdef SUPPRESS_BUILD_DATE_INFO
#define SACENC_LIB_BUILD_DATE ""
#define SACENC_LIB_BUILD_TIME ""
#else
#define SACENC_LIB_BUILD_DATE __DATE__
#define SACENC_LIB_BUILD_TIME __TIME__
#endif

#define MAX_MPEGS_BYTES (1 << 14)
#define MAX_SSC_BYTES (1 << 6)

#define MAX_SPACE_TREE_CHANNELS 2
#define NUM_KEEP_WINDOWS 3

/* Data Types ****************************************************************/
typedef struct {
  MP4SPACEENC_MODE encMode;
  MP4SPACEENC_BANDS_CONFIG nParamBands;
  MP4SPACEENC_QUANTMODE quantMode;
  UCHAR bUseCoarseQuant;
  UCHAR bLdMode;
  UCHAR bTimeDomainDmx;
  UINT sampleRate;
  UINT frameTimeSlots;     /* e.g. 32 when used with HE-AAC */
  UINT independencyFactor; /* how often should we set the independency flag */
  INT timeAlignment;       /* additional delay for downmix */

} MP4SPACEENC_SETUP, *HANDLE_MP4SPACEENC_SETUP;

struct ENC_CONFIG_SETUP {
  UCHAR bEncMode_212;
  UCHAR maxHybridInStaticSlots;
  LONG maxSamplingrate;
  INT maxAnalysisLengthTimeSlots;
  INT maxHybridBands;
  INT maxQmfBands;
  INT maxChIn;
  INT maxFrameTimeSlots;
  INT maxFrameLength;
  INT maxChOut;
  INT maxChTotOut;
};

struct MP4SPACE_ENCODER {
  MP4SPACEENC_SETUP user;

  ENC_CONFIG_SETUP setup; /* describe allocated instance */

  HANDLE_FRAMEWINDOW
  hFrameWindow;      /* Windowing, only created+updated, but not used */
  INT nSamplesValid; /* Input Buffer Handling */

  /* Routing Sensible Switches/Variables */
  MP4SPACEENC_BANDS_CONFIG nParamBands;
  UCHAR useTimeDomDownmix;

  /* not Routing Sensible Switches/Varibles - must be contained in Check */
  MP4SPACEENC_MODE encMode;
  UCHAR bEncMode_212_only;

  /* not Routing Sensible Switches/Varibles + lower Classes */
  UCHAR useFrameKeep;
  UINT independencyFactor;
  UINT nSampleRate;
  UCHAR nInputChannels;
  UCHAR nOutputChannels;
  UCHAR nFrameTimeSlots; /* e.g. 32 when used with HE-AAC */
  UCHAR nQmfBands;
  UCHAR nHybridBands;
  UINT nFrameLength; /* number of output waveform samples/channel/frame */

  /* not Routing Sensible Switches/Varibles + lower Classes, secondary computed
   */
  INT nSamplesNext;
  INT nAnalysisLengthTimeSlots;
  INT nAnalysisLookaheadTimeSlots;
  INT nUpdateHybridPositionTimeSlots;
  INT *pnOutputBits;
  INT nInputDelay;
  INT nOutputBufferDelay;
  INT nSurroundAnalysisBufferDelay;
  INT nBitstreamDelayBuffer;
  INT nBitstreamBufferRead;
  INT nBitstreamBufferWrite;
  INT nDiscardOutFrames;
  INT avoid_keep;

  /* not Routing Sensible Switches/Varibles -> moved to lower Classes */
  UCHAR useCoarseQuantCld;    /* Only Used in SpaceTreeSetup */
  UCHAR useCoarseQuantIcc;    /* Only Used in SpaceTreeSetup */
  UCHAR useCoarseQuantCpc;    /* Only Used in SpaceTreeSetup */
  UCHAR useCoarseQuantArbDmx; /* ArbitraryDmx,... not available yet */
  MP4SPACEENC_QUANTMODE
  quantMode;          /* Used for quanitzation and in bitstream writer */
  INT coreCoderDelay; /* Used in delay compensation */
  INT timeAlignment;  /* Used in delay compensation */

  /* Local Processing Variables */
  INT independencyCount;
  INT independencyFlag;
  INT **ppTrCurrPos;                /* belongs somehow to Onset Detection */
  INT trPrevPos[2 * MAX_NUM_TRANS]; /* belongs somehow to Onset Detection */

  FRAMEWIN_LIST frameWinList;
  SPATIALFRAME saveFrame;

  /* Module-Handles */
  SPACE_TREE_SETUP spaceTreeSetup;
  MPEG4SPACEENC_SSCBUF sscBuf;
  FIXP_WIN *pFrameWindowAna__FDK[MAX_NUM_PARAMS];
  HANDLE_QMF_FILTER_BANK *phQmfFiltIn__FDK;
  HANDLE_DC_FILTER phDCFilterSigIn[SACENC_MAX_INPUT_CHANNELS];
  HANDLE_ONSET_DETECT phOnset[SACENC_MAX_INPUT_CHANNELS];
  HANDLE_SPACE_TREE hSpaceTree;
  HANDLE_BSF_INSTANCE hBitstreamFormatter;
  HANDLE_STATIC_GAIN_CONFIG hStaticGainConfig;
  HANDLE_STATIC_GAIN hStaticGain;
  HANDLE_DELAY hDelay;

  /* enhanced time domain downmix (for stereo input) */
  HANDLE_ENHANCED_TIME_DOMAIN_DMX hEnhancedTimeDmx;

  /* Data Buffers */
  INT_PCM **ppTimeSigIn__FDK;
  INT_PCM **ppTimeSigDelayIn__FDK;
  INT_PCM **ppTimeSigOut__FDK;
  FIXP_DPK ***pppHybridIn__FDK;
  FIXP_DPK ***pppHybridInStatic__FDK;
  FIXP_DPK ***pppProcDataIn__FDK;
  INT_PCM *pOutputDelayBuffer__FDK;

  UCHAR **ppBitstreamDelayBuffer;

  UCHAR *pParameterBand2HybridBandOffset;
  INT staticGainScale;

  INT *pEncoderInputChScale;
  INT *staticTimeDomainDmxInScale;
};

/* Constants *****************************************************************/
static const UCHAR pValidBands_Ld[8] = {4, 5, 7, 9, 12, 15, 23, 40};

static const UCHAR qmf2qmf[] = /* Bypass the HybridAnylyis/Synthesis*/
    {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
     75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
     90,  91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
     105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
     120, 121, 122, 123, 124, 125, 126, 127};

/* Function / Class Declarations *********************************************/
static FDK_SACENC_ERROR mp4SpaceEnc_create(
    HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc);

static FDK_SACENC_ERROR FillSpatialSpecificConfig(
    const HANDLE_MP4SPACE_ENCODER hEnc, SPATIALSPECIFICCONFIG *const hSsc);

static FDK_SACENC_ERROR mp4SpaceEnc_FillSpaceTreeSetup(
    const HANDLE_MP4SPACE_ENCODER hEnc,
    SPACE_TREE_SETUP *const hSpaceTreeSetup);

static FDK_SACENC_ERROR mp4SpaceEnc_InitDelayCompensation(
    HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc, const INT coreCoderDelay);

static FDK_SACENC_ERROR mp4SpaceEnc_InitDefault(
    HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc);

static DECORRCONFIG mp4SpaceEnc_GetDecorrConfig(const MP4SPACEENC_MODE encMode);

static FDK_SACENC_ERROR mp4SpaceEnc_InitNumParamBands(
    HANDLE_MP4SPACE_ENCODER hEnc, const MP4SPACEENC_BANDS_CONFIG nParamBands);

/* Function / Class Definition ***********************************************/
static UINT mp4SpaceEnc_GetNumQmfBands(const UINT nSampleRate) {
  UINT nQmfBands = 0;

  if (nSampleRate < 27713)
    nQmfBands = 32;
  else if (nSampleRate < 55426)
    nQmfBands = 64;

  return nQmfBands;
}

static UINT updateQmfFlags(const UINT flags, const INT keepStates) {
  UINT qmfFlags = flags;

  qmfFlags = (qmfFlags & (~(UINT)QMF_FLAG_LP));
  qmfFlags = (qmfFlags | QMF_FLAG_MPSLDFB);
  qmfFlags = (keepStates) ? (qmfFlags | QMF_FLAG_KEEP_STATES)
                          : (qmfFlags & (~(UINT)QMF_FLAG_KEEP_STATES));

  return qmfFlags;
}

static INT freq2HybridBand(const UINT nFrequency, const UINT nSampleRate,
                           const UINT nQmfBands) {
  /*
    nQmfSlotWidth = (nSampleRate/2) / nQmfBands;
    nQmfBand      = nFrequency / nQmfSlotWidth;
  */
  int nHybridBand = -1;
  int scale = 0;
  const FIXP_DBL temp = fDivNorm((FIXP_DBL)(2 * nFrequency * nQmfBands),
                                 (FIXP_DBL)nSampleRate, &scale);
  const int nQmfBand = scaleValue(temp, scale - (DFRACT_BITS - 1));

  if ((nQmfBand > -1) && (nQmfBand < (int)nQmfBands)) {
    nHybridBand = qmf2qmf[nQmfBand];
  }

  return nHybridBand;
}

/*
 * Examine buffer descriptor regarding choosen type.
 *
 * \param pBufDesc              Pointer to buffer descriptor
 * \param type                  Buffer type to look for.

 * \return - Buffer descriptor index.
 *         -1, if there is no entry available.
 */
static INT getBufDescIdx(const FDK_bufDescr *pBufDesc, const UINT type) {
  INT i, idx = -1;

  for (i = 0; i < (int)pBufDesc->numBufs; i++) {
    if (pBufDesc->pBufType[i] == type) {
      idx = i;
      break;
    }
  }
  return idx;
}

FDK_SACENC_ERROR FDK_sacenc_open(HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc) {
  return mp4SpaceEnc_create(phMp4SpaceEnc);
}

static FDK_SACENC_ERROR mp4SpaceEnc_create(
    HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc) {
  FDK_SACENC_ERROR error = SACENC_OK;
  HANDLE_MP4SPACE_ENCODER hEnc = NULL;
  ENC_CONFIG_SETUP setup;

  if (NULL == phMp4SpaceEnc) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int i, ch;
    FDKmemclear(&setup, sizeof(ENC_CONFIG_SETUP));

    /* Allocate Encoder Instance */
    FDK_ALLOCATE_MEMORY_1D(hEnc, 1, struct MP4SPACE_ENCODER);

    /* Clear everything, also pointers. */
    if (NULL != hEnc) {
      FDKmemclear(hEnc, sizeof(struct MP4SPACE_ENCODER));
    }

    setup.maxSamplingrate = 48000;
    setup.maxFrameTimeSlots = 16;

    setup.maxAnalysisLengthTimeSlots = 3 * setup.maxFrameTimeSlots;
    setup.maxQmfBands = mp4SpaceEnc_GetNumQmfBands(setup.maxSamplingrate);
    ;
    setup.maxHybridBands = setup.maxQmfBands;
    setup.maxFrameLength = setup.maxQmfBands * setup.maxFrameTimeSlots;

    setup.maxChIn = 2;
    setup.maxChOut = 1;
    setup.maxChTotOut = setup.maxChOut;
    setup.bEncMode_212 = 1;
    setup.maxHybridInStaticSlots = 24;

    /* Open Static Gain*/
    if (SACENC_OK !=
        (error = fdk_sacenc_staticGain_OpenConfig(&hEnc->hStaticGainConfig))) {
      goto bail;
    }

    /* enhanced time domain downmix (for stereo input) */
    if (SACENC_OK != (error = fdk_sacenc_open_enhancedTimeDomainDmx(
                          &hEnc->hEnhancedTimeDmx, setup.maxFrameLength))) {
      goto bail;
    }

    FDK_ALLOCATE_MEMORY_1D(hEnc->pParameterBand2HybridBandOffset,
                           MAX_NUM_PARAM_BANDS, UCHAR);

    /* Create Space Tree first, to get number of in-/output channels */
    if (SACENC_OK != (error = fdk_sacenc_spaceTree_Open(&hEnc->hSpaceTree))) {
      goto bail;
    }

    FDK_ALLOCATE_MEMORY_1D(hEnc->pEncoderInputChScale, setup.maxChIn, INT);
    FDK_ALLOCATE_MEMORY_1D(hEnc->staticTimeDomainDmxInScale, setup.maxChIn,
                           INT);

    FDK_ALLOCATE_MEMORY_1D(hEnc->phQmfFiltIn__FDK, setup.maxChIn,
                           HANDLE_QMF_FILTER_BANK);

    /* Allocate Analysis Filterbank Structs */
    for (ch = 0; ch < setup.maxChIn; ch++) {
      FDK_ALLOCATE_MEMORY_1D_INT(hEnc->phQmfFiltIn__FDK[ch], 1,
                                 struct QMF_FILTER_BANK, SECT_DATA_L2)
      FDK_ALLOCATE_MEMORY_1D_INT(hEnc->phQmfFiltIn__FDK[ch]->FilterStates,
                                 2 * 5 * setup.maxQmfBands, FIXP_QAS,
                                 SECT_DATA_L2)
    }

    /* Allocate Synthesis Filterbank Structs for arbitrary downmix */

    /* Allocate DC Filter Struct for normal signal input */
    for (ch = 0; ch < setup.maxChIn; ch++) {
      if (SACENC_OK !=
          (error = fdk_sacenc_createDCFilter(&hEnc->phDCFilterSigIn[ch]))) {
        goto bail;
      }
    }

    /* Open Onset Detection */
    for (ch = 0; ch < setup.maxChIn; ch++) {
      if (SACENC_OK != (error = fdk_sacenc_onsetDetect_Open(
                            &hEnc->phOnset[ch], setup.maxFrameTimeSlots))) {
        goto bail;
      }
    }

    FDK_ALLOCATE_MEMORY_2D(hEnc->ppTrCurrPos, setup.maxChIn, MAX_NUM_TRANS,
                           INT);

    /* Create Windowing */
    if (SACENC_OK !=
        (error = fdk_sacenc_frameWindow_Create(&hEnc->hFrameWindow))) {
      goto bail;
    }

    /* Open static gain */
    if (SACENC_OK != (error = fdk_sacenc_staticGain_Open(&hEnc->hStaticGain))) {
      goto bail;
    }

    /* create bitstream encoder */
    if (SACENC_OK != (error = fdk_sacenc_createSpatialBitstreamEncoder(
                          &hEnc->hBitstreamFormatter))) {
      goto bail;
    }

    FDK_ALLOCATE_MEMORY_1D(hEnc->sscBuf.pSsc, MAX_SSC_BYTES, UCHAR);

    {
      FDK_ALLOCATE_MEMORY_2D(hEnc->ppTimeSigIn__FDK, setup.maxChIn,
                             setup.maxFrameLength + MAX_DELAY_SURROUND_ANALYSIS,
                             INT_PCM);
    }
    FDK_ALLOCATE_MEMORY_2D(hEnc->ppTimeSigDelayIn__FDK, setup.maxChIn,
                           MAX_DELAY_SURROUND_ANALYSIS, INT_PCM);

    /* Create new buffers for several signals (including arbitrary downmix) */
    if (setup.bEncMode_212 == 0) {
      /* pOutputDelayBuffer__FDK buffer is not needed for SACENC_212 mode */
      FDK_ALLOCATE_MEMORY_1D(
          hEnc->pOutputDelayBuffer__FDK,
          (setup.maxFrameLength + MAX_DELAY_OUTPUT) * setup.maxChOut, INT_PCM);
    }

    /* allocate buffers */
    if (setup.bEncMode_212 == 0) {
      /* ppTimeSigOut__FDK buffer is not needed for SACENC_212 mode */
      FDK_ALLOCATE_MEMORY_2D(hEnc->ppTimeSigOut__FDK, setup.maxChTotOut,
                             setup.maxFrameLength, INT_PCM);
    }

    if (setup.bEncMode_212 == 1) {
      /* pppHybridIn__FDK buffer can be reduced by maxFrameTimeSlots/2 slots for
       * SACENC_212 mode */
      FDK_ALLOCATE_MEMORY_3D(
          hEnc->pppHybridIn__FDK, setup.maxChIn,
          setup.maxAnalysisLengthTimeSlots - (setup.maxFrameTimeSlots >> 1),
          setup.maxHybridBands, FIXP_DPK);
      FDK_ALLOCATE_MEMORY_3D(hEnc->pppHybridInStatic__FDK, setup.maxChIn,
                             setup.maxHybridInStaticSlots, setup.maxHybridBands,
                             FIXP_DPK);
    } else {
      FDK_ALLOCATE_MEMORY_3D(hEnc->pppHybridIn__FDK, setup.maxChIn,
                             setup.maxAnalysisLengthTimeSlots,
                             setup.maxHybridBands, FIXP_DPK);
    }

    if (setup.bEncMode_212 == 0) {
      /* pppProcDataIn__FDK buffer is not needed for SACENC_212 mode */
      FDK_ALLOCATE_MEMORY_3D(hEnc->pppProcDataIn__FDK, MAX_SPACE_TREE_CHANNELS,
                             setup.maxAnalysisLengthTimeSlots,
                             setup.maxHybridBands, FIXP_DPK);
    }
    for (i = 0; i < MAX_NUM_PARAMS; i++) {
      FDK_ALLOCATE_MEMORY_1D(hEnc->pFrameWindowAna__FDK[i],
                             setup.maxAnalysisLengthTimeSlots, FIXP_WIN);
    } /* for i */

    if (SACENC_OK != (error = fdk_sacenc_delay_Open(&hEnc->hDelay))) {
      goto bail;
    }

    if (setup.bEncMode_212 == 0) {
      /* ppBitstreamDelayBuffer buffer is not needed for SACENC_212 mode */
      FDK_ALLOCATE_MEMORY_2D(hEnc->ppBitstreamDelayBuffer, MAX_BITSTREAM_DELAY,
                             MAX_MPEGS_BYTES, UCHAR);
    }
    FDK_ALLOCATE_MEMORY_1D(hEnc->pnOutputBits, MAX_BITSTREAM_DELAY, INT);

    hEnc->setup = setup; /* save configuration used while encoder allocation. */
    mp4SpaceEnc_InitDefault(hEnc);

    if (NULL != phMp4SpaceEnc) {
      *phMp4SpaceEnc = hEnc; /* return encoder handle */
    }

  } /* valid handle */

  return error;

bail:
  if (NULL != hEnc) {
    hEnc->setup = setup;
    FDK_sacenc_close(&hEnc);
  }
  return ((SACENC_OK == error) ? SACENC_MEMORY_ERROR : error);
}

static FDK_SACENC_ERROR mp4SpaceEnc_InitDefault(HANDLE_MP4SPACE_ENCODER hEnc) {
  FDK_SACENC_ERROR err = SACENC_OK;

  /* Get default static gain configuration. */
  if (SACENC_OK != (err = fdk_sacenc_staticGain_InitDefaultConfig(
                        hEnc->hStaticGainConfig))) {
    goto bail;
  }

bail:
  return err;
}

static FDK_SACENC_ERROR FDK_sacenc_configure(
    HANDLE_MP4SPACE_ENCODER hEnc, const HANDLE_MP4SPACEENC_SETUP hSetup) {
  FDK_SACENC_ERROR error = SACENC_OK;

  hEnc->nSampleRate = hSetup->sampleRate;
  hEnc->encMode = hSetup->encMode;
  hEnc->nQmfBands = mp4SpaceEnc_GetNumQmfBands(hEnc->nSampleRate);

  /* Make sure that we have set time domain downmix for 212 */
  if (hSetup->encMode == SACENC_212 && hSetup->bTimeDomainDmx == 0) {
    error = SACENC_INVALID_CONFIG;
  } else {
    hEnc->useTimeDomDownmix = hSetup->bTimeDomainDmx;
  }

  hEnc->timeAlignment = hSetup->timeAlignment;
  hEnc->quantMode = hSetup->quantMode;

  hEnc->useCoarseQuantCld = hSetup->bUseCoarseQuant;
  hEnc->useCoarseQuantCpc = hSetup->bUseCoarseQuant;
  hEnc->useFrameKeep = (hSetup->bLdMode == 2);
  hEnc->useCoarseQuantIcc = 0;    /* not available */
  hEnc->useCoarseQuantArbDmx = 0; /* not available for user right now */
  hEnc->independencyFactor = hSetup->independencyFactor;
  hEnc->independencyCount = 0;
  hEnc->independencyFlag = 1;

  /* set number of Hybrid bands */
  hEnc->nHybridBands = hEnc->nQmfBands;
  hEnc->nFrameTimeSlots = hSetup->frameTimeSlots;
  mp4SpaceEnc_InitNumParamBands(hEnc, hSetup->nParamBands);

  return error;
}

FDK_SACENC_ERROR FDK_sacenc_init(HANDLE_MP4SPACE_ENCODER hEnc,
                                 const INT dmxDelay) {
  FDK_SACENC_ERROR error = SACENC_OK;

  /* Sanity Checks */
  if (NULL == hEnc) {
    error = SACENC_INVALID_HANDLE;
  } else {
    const int initStatesFlag = 1;

    int ch; /* loop counter */
    int nChInArbDmx;

    if (SACENC_OK != (error = FDK_sacenc_configure(hEnc, &hEnc->user))) {
      goto bail;
    }

    hEnc->bEncMode_212_only = hEnc->setup.bEncMode_212;

    /* Slots per Frame and Frame Length */
    if (hEnc->nFrameTimeSlots < 1) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }
    hEnc->nFrameLength = hEnc->nQmfBands * hEnc->nFrameTimeSlots;

    if (hEnc->useFrameKeep == 1) {
      hEnc->nAnalysisLengthTimeSlots = 3 * hEnc->nFrameTimeSlots;
      hEnc->nUpdateHybridPositionTimeSlots = hEnc->nFrameTimeSlots;
    } else {
      hEnc->nAnalysisLengthTimeSlots = 2 * hEnc->nFrameTimeSlots;
      hEnc->nUpdateHybridPositionTimeSlots = 0;
    }

    {
      hEnc->nAnalysisLookaheadTimeSlots =
          hEnc->nAnalysisLengthTimeSlots - 3 * hEnc->nFrameTimeSlots / 2;
    }

    /* init parameterBand2hybridBandOffset table */
    fdk_sacenc_calcParameterBand2HybridBandOffset(
        (BOX_SUBBAND_CONFIG)hEnc->nParamBands, hEnc->nHybridBands,
        hEnc->pParameterBand2HybridBandOffset);

    /* Fill Setup structure for Space Tree */
    if (SACENC_OK !=
        (error = mp4SpaceEnc_FillSpaceTreeSetup(hEnc, &hEnc->spaceTreeSetup))) {
      goto bail;
    }

    /* Init space tree configuration */
    if (SACENC_OK !=
        (error = fdk_sacenc_spaceTree_Init(
             hEnc->hSpaceTree, &hEnc->spaceTreeSetup,
             hEnc->pParameterBand2HybridBandOffset, hEnc->useFrameKeep))) {
      goto bail;
    }

    /* Get space tree description and resulting number of input/output channels
     */
    {
      SPACE_TREE_DESCRIPTION spaceTreeDescription;

      if (SACENC_OK != (error = fdk_sacenc_spaceTree_GetDescription(
                            hEnc->hSpaceTree, &spaceTreeDescription))) {
        goto bail;
      }

      hEnc->nInputChannels =
          spaceTreeDescription.nOutChannels; /* space tree description
                                                describes decoder
                                                configuration */
      hEnc->nOutputChannels =
          spaceTreeDescription.nInChannels; /* space tree description
                                               describes decoder
                                               configuration */
    }

    nChInArbDmx = 0;

    /* INITIALIZATION */
    for (ch = 0; ch < hEnc->nInputChannels; ch++) {
      /* scaling in analysis qmf filterbank (7) */
      hEnc->pEncoderInputChScale[ch] = 7;

      {
        /* additional scaling in qmf prototype filter for low delay */
        hEnc->pEncoderInputChScale[ch] += 1;
      }

      { hEnc->pEncoderInputChScale[ch] += DC_FILTER_SF; }
    } /* nInputChannels */

    /* Init analysis filterbank */
    for (ch = 0; ch < hEnc->nInputChannels; ch++) {
      hEnc->phQmfFiltIn__FDK[ch]->flags =
          updateQmfFlags(hEnc->phQmfFiltIn__FDK[ch]->flags, !initStatesFlag);

      if (0 != qmfInitAnalysisFilterBank(
                   hEnc->phQmfFiltIn__FDK[ch],
                   (FIXP_QAS *)hEnc->phQmfFiltIn__FDK[ch]->FilterStates, 1,
                   hEnc->nQmfBands, hEnc->nQmfBands, hEnc->nQmfBands,
                   hEnc->phQmfFiltIn__FDK[ch]->flags)) {
        error = SACENC_INIT_ERROR;
        goto bail;
      }
    }

    /* Initialize DC Filter. */
    {
      for (ch = 0; ch < hEnc->nInputChannels; ch++) {
        if (SACENC_OK != (error = fdk_sacenc_initDCFilter(
                              hEnc->phDCFilterSigIn[ch], hEnc->nSampleRate))) {
          goto bail;
        }
      }
    }

    /* Init onset detect. */
    {
      /* init onset detect configuration struct */
      ONSET_DETECT_CONFIG onsetDetectConfig;
      onsetDetectConfig.maxTimeSlots = hEnc->nFrameTimeSlots;
      onsetDetectConfig.lowerBoundOnsetDetection =
          freq2HybridBand(1725, hEnc->nSampleRate, hEnc->nQmfBands);
      onsetDetectConfig.upperBoundOnsetDetection = hEnc->nHybridBands;

      for (ch = 0; ch < hEnc->nInputChannels; ch++) {
        if (SACENC_OK != (error = fdk_sacenc_onsetDetect_Init(
                              hEnc->phOnset[ch], &onsetDetectConfig, 1))) {
          goto bail;
        }
      }
    }

    {
      /* init windowing */
      FRAMEWINDOW_CONFIG framewindowConfig;
      framewindowConfig.nTimeSlotsMax = hEnc->nFrameTimeSlots;
      framewindowConfig.bFrameKeep = hEnc->useFrameKeep;

      if (SACENC_OK != (error = fdk_sacenc_frameWindow_Init(
                            hEnc->hFrameWindow, &framewindowConfig))) {
        goto bail;
      }
    }

    /* Set encoder mode for static gain initialization. */
    if (SACENC_OK != (error = fdk_sacenc_staticGain_SetEncMode(
                          hEnc->hStaticGainConfig, hEnc->encMode))) {
      goto bail;
    }

    /* Init static gain. */
    if (SACENC_OK != (error = fdk_sacenc_staticGain_Init(
                          hEnc->hStaticGain, hEnc->hStaticGainConfig,
                          &(hEnc->staticGainScale)))) {
      goto bail;
    }

    for (ch = 0; ch < hEnc->nInputChannels; ch++) {
      hEnc->pEncoderInputChScale[ch] += hEnc->staticGainScale;
    }

    /* enhanced downmix for stereo input*/
    if (hEnc->useTimeDomDownmix != 0) {
      if (SACENC_OK != (error = fdk_sacenc_init_enhancedTimeDomainDmx(
                            hEnc->hEnhancedTimeDmx,
                            fdk_sacenc_getPreGainPtrFDK(hEnc->hStaticGain),
                            hEnc->staticGainScale,
                            fdk_sacenc_getPostGainFDK(hEnc->hStaticGain),
                            hEnc->staticGainScale, hEnc->nFrameLength))) {
        goto bail;
      }
    }

    /* Create config structure for bitstream formatter including arbitrary
     * downmix residual */
    if (SACENC_OK != (error = fdk_sacenc_initSpatialBitstreamEncoder(
                          hEnc->hBitstreamFormatter))) {
      goto bail;
    }

    if (SACENC_OK != (error = FillSpatialSpecificConfig(
                          hEnc, fdk_sacenc_getSpatialSpecificConfig(
                                    hEnc->hBitstreamFormatter)))) {
      goto bail;
    }

    if (SACENC_OK !=
        (error = fdk_sacenc_writeSpatialSpecificConfig(
             fdk_sacenc_getSpatialSpecificConfig(hEnc->hBitstreamFormatter),
             hEnc->sscBuf.pSsc, MAX_SSC_BYTES, &hEnc->sscBuf.nSscSizeBits))) {
      goto bail;
    }

    /* init delay compensation with dmx core coder delay; if no core coder is
     * used, many other buffers are initialized nevertheless */
    if (SACENC_OK !=
        (error = mp4SpaceEnc_InitDelayCompensation(hEnc, dmxDelay))) {
      goto bail;
    }

    /* How much input do we need? */
    hEnc->nSamplesNext =
        hEnc->nFrameLength * (hEnc->nInputChannels + nChInArbDmx);
    hEnc->nSamplesValid = 0;
  } /* valid handle */

bail:
  return error;
}

static INT getAnalysisLengthTimeSlots(FIXP_WIN *pFrameWindowAna,
                                      INT nTimeSlots) {
  int i;
  for (i = nTimeSlots - 1; i >= 0; i--) {
    if (pFrameWindowAna[i] != (FIXP_WIN)0) {
      break;
    }
  }
  nTimeSlots = i + 1;
  return nTimeSlots;
}

static INT getAnalysisStartTimeSlot(FIXP_WIN *pFrameWindowAna, INT nTimeSlots) {
  int startTimeSlot = 0;
  int i;
  for (i = 0; i < nTimeSlots; i++) {
    if (pFrameWindowAna[i] != (FIXP_WIN)0) {
      break;
    }
  }
  startTimeSlot = i;
  return startTimeSlot;
}

static FDK_SACENC_ERROR __FeedDeinterPreScale(
    HANDLE_MP4SPACE_ENCODER hEnc, INT_PCM const *const pSamples,
    INT_PCM *const pOutputSamples, INT const nSamples,
    UINT const isInputInterleaved, UINT const inputBufferSizePerChannel,
    UINT *const pnSamplesFed) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((hEnc == NULL) || (pSamples == NULL) || (pnSamplesFed == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else if (nSamples == 0) {
    error = SACENC_INVALID_CONFIG; /* Flushing not implemented */
  } else {
    int ch;
    const INT nChIn = hEnc->nInputChannels;
    const INT nChInWithDmx = nChIn;
    const INT samplesToFeed =
        FDKmin(nSamples, hEnc->nSamplesNext - hEnc->nSamplesValid);
    const INT nSamplesPerChannel = samplesToFeed / nChInWithDmx;

    if ((samplesToFeed < 0) || (samplesToFeed % nChInWithDmx != 0) ||
        (samplesToFeed > nChInWithDmx * (INT)hEnc->nFrameLength)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }
    int i;

    const INT_PCM *pInput__FDK;
    const INT_PCM *pInput2__FDK;

    { /* no dmx align = default*/
      pInput__FDK = pSamples;
      pInput2__FDK = pSamples + (hEnc->nInputDelay * nChInWithDmx);
    }

    for (i = 0; i < hEnc->nInputChannels; i++) {
      hEnc->staticTimeDomainDmxInScale[i] = hEnc->staticGainScale;
    }

    /*****        N-channel-input     *****/
    for (ch = 0; ch < nChIn; ch++) {
      /* Write delayed time signal into time signal buffer */
      FDKmemcpy(&(hEnc->ppTimeSigIn__FDK[ch][0]),
                &(hEnc->ppTimeSigDelayIn__FDK[ch][0]),
                hEnc->nSurroundAnalysisBufferDelay * sizeof(INT_PCM));

      if (isInputInterleaved) {
        /* Add the new frame de-interleaved. Apply nSurroundAnalysisBufferDelay.
         */
        FDKmemcpy_flex(
            &(hEnc->ppTimeSigIn__FDK[ch][hEnc->nSurroundAnalysisBufferDelay]),
            1, pInput__FDK + ch, nChInWithDmx, hEnc->nInputDelay);
        FDKmemcpy_flex(
            &(hEnc->ppTimeSigIn__FDK[ch][hEnc->nSurroundAnalysisBufferDelay +
                                         hEnc->nInputDelay]),
            1, pInput2__FDK + ch, nChInWithDmx,
            nSamplesPerChannel - hEnc->nInputDelay);
      } else {
        /* Input is already deinterleaved, just copy */
        FDKmemcpy(
            &(hEnc->ppTimeSigIn__FDK[ch][hEnc->nSurroundAnalysisBufferDelay]),
            pInput__FDK + ch * inputBufferSizePerChannel,
            hEnc->nInputDelay * sizeof(INT_PCM));
        FDKmemcpy(
            &(hEnc->ppTimeSigIn__FDK[ch][hEnc->nSurroundAnalysisBufferDelay +
                                         hEnc->nInputDelay]),
            pInput2__FDK + ch * inputBufferSizePerChannel,
            (nSamplesPerChannel - hEnc->nInputDelay) * sizeof(INT_PCM));
      }

      /* Update time signal delay buffer */
      FDKmemcpy(&(hEnc->ppTimeSigDelayIn__FDK[ch][0]),
                &(hEnc->ppTimeSigIn__FDK[ch][hEnc->nFrameLength]),
                hEnc->nSurroundAnalysisBufferDelay * sizeof(INT_PCM));
    } /* for ch */

    /*****      No Arbitrary Downmix      *****/
    /* "Crude TD Dmx": Time DomainDownmix + NO Arbitrary Downmix, Delay Added at
     * pOutputBuffer */
    if ((hEnc->useTimeDomDownmix > 0)) {
      if ((hEnc->useTimeDomDownmix == 1) || (hEnc->nInputChannels != 2)) {
        error = SACENC_INVALID_CONFIG;
        goto bail;
      } else {
        /* enhanced time domain downmix (for stereo input) */
        if (hEnc->encMode == SACENC_212) {
          if (pOutputSamples == NULL) {
            error = SACENC_INVALID_HANDLE;
            goto bail;
          }

          fdk_sacenc_apply_enhancedTimeDomainDmx(
              hEnc->hEnhancedTimeDmx, hEnc->ppTimeSigIn__FDK, pOutputSamples,
              hEnc->nSurroundAnalysisBufferDelay);
        } else {
          if (&hEnc->ppTimeSigOut__FDK[0][0] == NULL) {
            error = SACENC_INVALID_HANDLE;
            goto bail;
          }

          fdk_sacenc_apply_enhancedTimeDomainDmx(
              hEnc->hEnhancedTimeDmx, hEnc->ppTimeSigIn__FDK,
              &hEnc->ppTimeSigOut__FDK[0][0],
              hEnc->nSurroundAnalysisBufferDelay);
        }
      }
    }

    /* update number of samples still to process */
    hEnc->nSamplesValid += samplesToFeed;

    /*return number of fed samples */
    *pnSamplesFed = samplesToFeed;
  }
bail:
  return error;
}

FDK_SACENC_ERROR FDK_sacenc_encode(const HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                   const FDK_bufDescr *inBufDesc,
                                   const FDK_bufDescr *outBufDesc,
                                   const SACENC_InArgs *inargs,
                                   SACENC_OutArgs *outargs) {
  FDK_SACENC_ERROR error = SACENC_OK;

  const INT_PCM *pInputSamples =
      (const INT_PCM *)inBufDesc->ppBase[getBufDescIdx(
          inBufDesc, (FDK_BUF_TYPE_INPUT | FDK_BUF_TYPE_PCM_DATA))];

  INT_PCM *const pOutputSamples = (INT_PCM *)outBufDesc->ppBase[getBufDescIdx(
      outBufDesc, (FDK_BUF_TYPE_OUTPUT | FDK_BUF_TYPE_PCM_DATA))];

  const int nOutputSamplesBufferSize =
      outBufDesc->pBufSize[getBufDescIdx(
          outBufDesc, (FDK_BUF_TYPE_OUTPUT | FDK_BUF_TYPE_PCM_DATA))] /
      outBufDesc->pEleSize[getBufDescIdx(
          outBufDesc, (FDK_BUF_TYPE_OUTPUT | FDK_BUF_TYPE_PCM_DATA))];

  if ((hMp4SpaceEnc == NULL) || (pInputSamples == NULL)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    int nOutputSamples;
    int i, ch, ps, winCnt, ts, slot;
    INT currTransPos = -1;
    SPATIALFRAME *pFrameData = NULL;

    /* Improve Code Readability */
    const int nChIn = hMp4SpaceEnc->nInputChannels;
    const int nChInWithDmx = nChIn;
    const int nChOut = hMp4SpaceEnc->nOutputChannels;
    const int nSamplesPerChannel = inargs->nInputSamples / nChInWithDmx;
    const int nOutputSamplesMax = nSamplesPerChannel * nChOut;
    const int nFrameTimeSlots = hMp4SpaceEnc->nFrameTimeSlots;

    INT encoderInputChScale[SACENC_MAX_INPUT_CHANNELS];
    INT nFrameTimeSlotsReduction = 0;

    if (hMp4SpaceEnc->encMode == SACENC_212) {
      nFrameTimeSlotsReduction = hMp4SpaceEnc->nFrameTimeSlots >> 1;
    }

    for (i = 0; i < nChIn; i++)
      encoderInputChScale[i] = hMp4SpaceEnc->pEncoderInputChScale[i];

    /* Sanity Check */
    if ((0 != inargs->nInputSamples % nChInWithDmx)) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /*
     * Get Frame Data Handle.
     */

    /* get bitstream handle (for storage of cld's, icc's and so on)
     * get spatialframe 2 frames in the future; NOTE: this is necessary to
     * synchronise spatial data and audio data */
    if (NULL == (pFrameData = fdk_sacenc_getSpatialFrame(
                     hMp4SpaceEnc->hBitstreamFormatter, WRITE_SPATIALFRAME))) {
      error = SACENC_INVALID_HANDLE;
      goto bail;
    }

    /* Independent Frames Counters*/
    if (hMp4SpaceEnc->nDiscardOutFrames >
        0) { /* Independent Frames if they should be discarded, Reset Counter*/
      hMp4SpaceEnc->independencyCount =
          0; /* Reset the counter, first valid frame is an independent one*/
      hMp4SpaceEnc->independencyFlag = 1;
    } else { /*hMp4SpaceEnc->nDiscardOutFrames == 0*/
      hMp4SpaceEnc->independencyFlag =
          (hMp4SpaceEnc->independencyCount == 0) ? 1 : 0;
      if (hMp4SpaceEnc->independencyFactor > 0) {
        hMp4SpaceEnc->independencyCount++;
        hMp4SpaceEnc->independencyCount =
            hMp4SpaceEnc->independencyCount %
            ((int)hMp4SpaceEnc->independencyFactor);
      } else { /* independencyFactor == 0 */
        hMp4SpaceEnc->independencyCount = -1;
      }
    }

    /*
     * Time signal preprocessing:
     * - Feed input buffer
     * - Prescale time signal
     * - Apply DC filter on input signal
     */

    /* Feed, Deinterleave, Pre-Scale the input time signals */
    if (SACENC_OK !=
        (error = __FeedDeinterPreScale(
             hMp4SpaceEnc, pInputSamples, pOutputSamples, inargs->nInputSamples,
             inargs->isInputInterleaved, inargs->inputBufferSizePerChannel,
             &outargs->nSamplesConsumed))) {
      goto bail;
    }

    if (hMp4SpaceEnc->nSamplesNext != hMp4SpaceEnc->nSamplesValid) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    if (hMp4SpaceEnc->encMode == SACENC_212 &&
        hMp4SpaceEnc->bEncMode_212_only) {
      for (ch = 0; ch < nChIn; ch++) {
        for (slot = 0; slot < nFrameTimeSlots; slot++) {
          setCplxVec(
              hMp4SpaceEnc->pppHybridIn__FDK
                  [ch][hMp4SpaceEnc->nUpdateHybridPositionTimeSlots +
                       nFrameTimeSlots - nFrameTimeSlotsReduction + slot],
              (FIXP_DBL)0, hMp4SpaceEnc->nHybridBands);
        }
      }
    }

    /*
     * Time / Frequency:
     * - T/F audio input channels
     * - T/F arbitrary downmix input channels
     */
    for (ch = 0; ch < nChIn; ch++) {
      C_AALLOC_SCRATCH_START(pQmfInReal, FIXP_DBL, MAX_QMF_BANDS)
      C_AALLOC_SCRATCH_START(pQmfInImag, FIXP_DBL, MAX_QMF_BANDS)
      FIXP_GAIN *pPreGain =
          fdk_sacenc_getPreGainPtrFDK(hMp4SpaceEnc->hStaticGain);

      for (ts = 0; ts < nFrameTimeSlots; ts++) {
        FIXP_DBL *pSpecReal;
        FIXP_DBL *pSpecImag;

        INT_PCM *pTimeIn =
            &hMp4SpaceEnc->ppTimeSigIn__FDK[ch][(ts * hMp4SpaceEnc->nQmfBands)];

        {
          /* Apply DC filter on input channels */
          if (SACENC_OK != (error = fdk_sacenc_applyDCFilter(
                                hMp4SpaceEnc->phDCFilterSigIn[ch], pTimeIn,
                                pTimeIn, hMp4SpaceEnc->nQmfBands))) {
            goto bail;
          }
        }

        /* QMF filterbank */
        C_ALLOC_SCRATCH_START(pWorkBuffer, FIXP_DBL, (MAX_QMF_BANDS << 1));

        qmfAnalysisFilteringSlot(hMp4SpaceEnc->phQmfFiltIn__FDK[ch], pQmfInReal,
                                 pQmfInImag, pTimeIn, 1, pWorkBuffer);

        C_ALLOC_SCRATCH_END(pWorkBuffer, FIXP_DBL, (MAX_QMF_BANDS << 1));

        pSpecReal = pQmfInReal;
        pSpecImag = pQmfInImag;

        /* Apply pre-scale after filterbank */
        if (MAXVAL_GAIN != pPreGain[ch]) {
          for (i = 0; i < hMp4SpaceEnc->nHybridBands; i++) {
            hMp4SpaceEnc
                ->pppHybridIn__FDK[ch]
                                  [hMp4SpaceEnc->nAnalysisLookaheadTimeSlots +
                                   ts][i]
                .v.re = fMult(pSpecReal[i], pPreGain[ch]);
            hMp4SpaceEnc
                ->pppHybridIn__FDK[ch]
                                  [hMp4SpaceEnc->nAnalysisLookaheadTimeSlots +
                                   ts][i]
                .v.im = fMult(pSpecImag[i], pPreGain[ch]);
          }
        } else {
          for (i = 0; i < hMp4SpaceEnc->nHybridBands; i++) {
            hMp4SpaceEnc
                ->pppHybridIn__FDK[ch]
                                  [hMp4SpaceEnc->nAnalysisLookaheadTimeSlots +
                                   ts][i]
                .v.re = pSpecReal[i];
            hMp4SpaceEnc
                ->pppHybridIn__FDK[ch]
                                  [hMp4SpaceEnc->nAnalysisLookaheadTimeSlots +
                                   ts][i]
                .v.im = pSpecImag[i];
          }
        }
      } /* ts */
      C_AALLOC_SCRATCH_END(pQmfInImag, FIXP_DBL, MAX_QMF_BANDS)
      C_AALLOC_SCRATCH_END(pQmfInReal, FIXP_DBL, MAX_QMF_BANDS)

      if (SACENC_OK != error) {
        goto bail;
      }
    } /* ch */

    if (hMp4SpaceEnc->encMode == SACENC_212 &&
        hMp4SpaceEnc->bEncMode_212_only) {
      for (ch = 0; ch < nChIn; ch++) {
        for (slot = 0;
             slot < (int)(hMp4SpaceEnc->nUpdateHybridPositionTimeSlots +
                          nFrameTimeSlots - nFrameTimeSlotsReduction);
             slot++) {
          copyCplxVec(hMp4SpaceEnc->pppHybridIn__FDK[ch][slot],
                      hMp4SpaceEnc->pppHybridInStatic__FDK[ch][slot],
                      hMp4SpaceEnc->nHybridBands);
        }
      }
      for (ch = 0; ch < nChIn; ch++) {
        for (slot = 0;
             slot < (int)(hMp4SpaceEnc->nUpdateHybridPositionTimeSlots +
                          nFrameTimeSlots - nFrameTimeSlotsReduction);
             slot++) {
          copyCplxVec(
              hMp4SpaceEnc->pppHybridInStatic__FDK[ch][slot],
              hMp4SpaceEnc->pppHybridIn__FDK[ch][nFrameTimeSlots + slot],
              hMp4SpaceEnc->nHybridBands);
        }
      }
    }

    /*
     * Onset Detection:
     * - detection of transients
     * - build framing
     */
    for (ch = 0; ch < nChIn; ch++) {
      if (ch != 3) { /* !LFE */
        if (SACENC_OK !=
            (error = fdk_sacenc_onsetDetect_Apply(
                 hMp4SpaceEnc->phOnset[ch], nFrameTimeSlots,
                 hMp4SpaceEnc->nHybridBands,
                 &hMp4SpaceEnc->pppHybridIn__FDK
                      [ch][hMp4SpaceEnc->nAnalysisLookaheadTimeSlots],
                 encoderInputChScale[ch],
                 hMp4SpaceEnc->trPrevPos[1], /* contains previous Transient */
                 hMp4SpaceEnc->ppTrCurrPos[ch]))) {
          goto bail;
        }

        if ((1) && (hMp4SpaceEnc->useFrameKeep == 0)) {
          hMp4SpaceEnc->ppTrCurrPos[ch][0] = -1;
        }

        /* Find first Transient Position */
        if ((hMp4SpaceEnc->ppTrCurrPos[ch][0] >= 0) &&
            ((currTransPos < 0) ||
             (hMp4SpaceEnc->ppTrCurrPos[ch][0] < currTransPos))) {
          currTransPos = hMp4SpaceEnc->ppTrCurrPos[ch][0];
        }
      } /* !LFE */
    }   /* ch */

    if (hMp4SpaceEnc->useFrameKeep == 1) {
      if ((currTransPos != -1) || (hMp4SpaceEnc->independencyFlag == 1)) {
        hMp4SpaceEnc->avoid_keep = NUM_KEEP_WINDOWS;
        currTransPos = -1;
      }
    }

    /* Save previous Transient Position */
    hMp4SpaceEnc->trPrevPos[0] =
        FDKmax(-1, hMp4SpaceEnc->trPrevPos[1] - (INT)nFrameTimeSlots);
    hMp4SpaceEnc->trPrevPos[1] = currTransPos;

    /* Update Onset Detection Energy Buffer */
    for (ch = 0; ch < nChIn; ch++) {
      if (SACENC_OK != (error = fdk_sacenc_onsetDetect_Update(
                            hMp4SpaceEnc->phOnset[ch], nFrameTimeSlots))) {
        goto bail;
      }
    }

    /* Framing */
    if (SACENC_OK !=
        (error = fdk_sacenc_frameWindow_GetWindow(
             hMp4SpaceEnc->hFrameWindow, hMp4SpaceEnc->trPrevPos,
             nFrameTimeSlots, &pFrameData->framingInfo,
             hMp4SpaceEnc->pFrameWindowAna__FDK, &hMp4SpaceEnc->frameWinList,
             hMp4SpaceEnc->avoid_keep))) {
      goto bail;
    }

    /*
     * MPS Processing:
     */
    for (ps = 0, winCnt = 0; ps < hMp4SpaceEnc->frameWinList.n; ++ps) {
      /* Analysis Windowing */
      if (hMp4SpaceEnc->frameWinList.dat[ps].hold == FW_HOLD) {
        /* ************************************** */
        /* ONLY COPY AND HOLD PREVIOUS PARAMETERS */
        if (SACENC_OK != (error = fdk_sacenc_duplicateParameterSet(
                              &hMp4SpaceEnc->saveFrame, 0, pFrameData, ps))) {
          goto bail;
        }

      } else { /* !FW_HOLD */
        /* ************************************** */
        /* NEW WINDOW */

        INT nAnalysisLengthTimeSlots, analysisStartTimeSlot;

        nAnalysisLengthTimeSlots = getAnalysisLengthTimeSlots(
            hMp4SpaceEnc->pFrameWindowAna__FDK[winCnt],
            hMp4SpaceEnc->nAnalysisLengthTimeSlots);

        analysisStartTimeSlot =
            getAnalysisStartTimeSlot(hMp4SpaceEnc->pFrameWindowAna__FDK[winCnt],
                                     hMp4SpaceEnc->nAnalysisLengthTimeSlots);

        /* perform main signal analysis windowing in
         * fdk_sacenc_spaceTree_Apply() */
        FIXP_WIN *pFrameWindowAna__FDK =
            hMp4SpaceEnc->pFrameWindowAna__FDK[winCnt];
        FIXP_DPK ***pppHybridIn__FDK = hMp4SpaceEnc->pppHybridIn__FDK;
        FIXP_DPK ***pppProcDataIn__FDK = hMp4SpaceEnc->pppProcDataIn__FDK;

        if (hMp4SpaceEnc->encMode == SACENC_212 &&
            hMp4SpaceEnc->bEncMode_212_only) {
          pppProcDataIn__FDK = pppHybridIn__FDK;
        }

        if (SACENC_OK !=
            (error = fdk_sacenc_spaceTree_Apply(
                 hMp4SpaceEnc->hSpaceTree, ps, nChIn, nAnalysisLengthTimeSlots,
                 analysisStartTimeSlot, hMp4SpaceEnc->nHybridBands,
                 pFrameWindowAna__FDK, pppHybridIn__FDK,
                 pppProcDataIn__FDK, /* multi-channel input */
                 pFrameData, hMp4SpaceEnc->avoid_keep, encoderInputChScale))) {
          goto bail;
        }

        /* Save spatial frame for potential hold parameter set */
        if (SACENC_OK != (error = fdk_sacenc_duplicateParameterSet(
                              pFrameData, ps, &hMp4SpaceEnc->saveFrame, 0))) {
          goto bail;
        }

        ++winCnt;
      }
      if (hMp4SpaceEnc->avoid_keep > 0) {
        hMp4SpaceEnc->avoid_keep--;
      }
    } /* Loop over Parameter Sets */
    /* ---- End of Processing Loop ---- */

    /*
     * Update hybridInReal/Imag buffer and do the same for arbDmx
     * this means to move the hybrid data of the current frame to the beginning
     * of the 2*nFrameLength-long buffer
     */
    if (!(hMp4SpaceEnc->encMode == SACENC_212 &&
          hMp4SpaceEnc->bEncMode_212_only)) {
      for (ch = 0; ch < nChIn; ch++) { /* for automatic downmix */
        for (slot = 0;
             slot < (int)(hMp4SpaceEnc->nUpdateHybridPositionTimeSlots +
                          nFrameTimeSlots - nFrameTimeSlotsReduction);
             slot++) {
          copyCplxVec(
              hMp4SpaceEnc->pppHybridIn__FDK[ch][slot],
              hMp4SpaceEnc->pppHybridIn__FDK[ch][nFrameTimeSlots + slot],
              hMp4SpaceEnc->nHybridBands);
        }
        for (slot = 0; slot < nFrameTimeSlots; slot++) {
          setCplxVec(
              hMp4SpaceEnc->pppHybridIn__FDK
                  [ch][hMp4SpaceEnc->nUpdateHybridPositionTimeSlots +
                       nFrameTimeSlots - nFrameTimeSlotsReduction + slot],
              (FIXP_DBL)0, hMp4SpaceEnc->nHybridBands);
        }
      }
    }
    /*
     * Spatial Tonality:
     */
    {
      /* Smooth config off. */
      FDKmemclear(&pFrameData->smgData, sizeof(pFrameData->smgData));
    }

    /*
     * Create bitstream
     * - control independecy flag
     * - write spatial frame
     * - return bitstream
     */
    UCHAR *pBitstreamDelayBuffer;

    if (hMp4SpaceEnc->encMode == SACENC_212) {
      /* no bitstream delay buffer for SACENC_212 mode, write bitstream directly
       * into the sacOutBuffer buffer which is provided by the core routine */
      pBitstreamDelayBuffer = (UCHAR *)outBufDesc->ppBase[1];
    } else {
      /* bitstream delay is handled in ppBitstreamDelayBuffer buffer */
      pBitstreamDelayBuffer =
          hMp4SpaceEnc
              ->ppBitstreamDelayBuffer[hMp4SpaceEnc->nBitstreamBufferWrite];
    }
    if (pBitstreamDelayBuffer == NULL) {
      error = SACENC_INVALID_HANDLE;
      goto bail;
    }

    pFrameData->bsIndependencyFlag = hMp4SpaceEnc->independencyFlag;

    if (SACENC_OK !=
        (error = fdk_sacenc_writeSpatialFrame(
             pBitstreamDelayBuffer, MAX_MPEGS_BYTES,
             &hMp4SpaceEnc->pnOutputBits[hMp4SpaceEnc->nBitstreamBufferWrite],
             hMp4SpaceEnc->hBitstreamFormatter))) {
      goto bail;
    }

    /* return bitstream info */
    if ((hMp4SpaceEnc->nDiscardOutFrames == 0) &&
        (getBufDescIdx(outBufDesc,
                       (FDK_BUF_TYPE_OUTPUT | FDK_BUF_TYPE_BS_DATA)) != -1)) {
      const INT idx = getBufDescIdx(
          outBufDesc, (FDK_BUF_TYPE_OUTPUT | FDK_BUF_TYPE_BS_DATA));
      const INT outBits =
          hMp4SpaceEnc->pnOutputBits[hMp4SpaceEnc->nBitstreamBufferRead];

      if (((outBits + 7) / 8) >
          (INT)(outBufDesc->pBufSize[idx] / outBufDesc->pEleSize[idx])) {
        outargs->nOutputBits = 0;
        error = SACENC_ENCODE_ERROR;
        goto bail;
      }

      /* return bitstream buffer, copy delayed bitstream for all configurations
       * except for the SACENC_212 mode */
      if (hMp4SpaceEnc->encMode != SACENC_212) {
        FDKmemcpy(
            outBufDesc->ppBase[idx],
            hMp4SpaceEnc
                ->ppBitstreamDelayBuffer[hMp4SpaceEnc->nBitstreamBufferRead],
            (outBits + 7) / 8);
      }

      /* return number of valid bits */
      outargs->nOutputBits = outBits;
    } else { /* No spatial data should be returned if the current frame is to be
                discarded. */
      outargs->nOutputBits = 0;
    }

    /* update pointers */
    hMp4SpaceEnc->nBitstreamBufferRead =
        (hMp4SpaceEnc->nBitstreamBufferRead + 1) %
        hMp4SpaceEnc->nBitstreamDelayBuffer;
    hMp4SpaceEnc->nBitstreamBufferWrite =
        (hMp4SpaceEnc->nBitstreamBufferWrite + 1) %
        hMp4SpaceEnc->nBitstreamDelayBuffer;

    /* Set Output Parameters */
    nOutputSamples =
        (hMp4SpaceEnc->nDiscardOutFrames == 0)
            ? (nOutputSamplesMax)
            : 0; /* don't output samples in case frames to be discarded */
    if (nOutputSamples > nOutputSamplesBufferSize) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }
    outargs->nOutputSamples = nOutputSamples;

    { /* !bQmfOutput */

      if (hMp4SpaceEnc->encMode != SACENC_212) {
        /* delay output samples and interleave them */
        /* note: in case of arbitrary downmix this will always be processed,
         * because nOutputSamples != 0, even if bDMXAlign is switched on */
        /* always run copy-func, so nOutputSamplesMax instead of nOutputSamples
         */
        for (ch = 0; ch < nChOut; ch++) {
          FDKmemcpy_flex(
              &hMp4SpaceEnc->pOutputDelayBuffer__FDK
                   [ch + (hMp4SpaceEnc->nOutputBufferDelay) * nChOut],
              nChOut, hMp4SpaceEnc->ppTimeSigOut__FDK[ch], 1,
              nOutputSamplesMax / nChOut);
        }

        /* write delayed data in output pcm stream */
        /* always calculate, limiter must have a lookahead!!! */
        FDKmemcpy(pOutputSamples, hMp4SpaceEnc->pOutputDelayBuffer__FDK,
                  nOutputSamplesMax * sizeof(INT_PCM));

        /* update delay buffer (move back end to the beginning of the buffer) */
        FDKmemmove(
            hMp4SpaceEnc->pOutputDelayBuffer__FDK,
            &hMp4SpaceEnc->pOutputDelayBuffer__FDK[nOutputSamplesMax],
            nChOut * (hMp4SpaceEnc->nOutputBufferDelay) * sizeof(INT_PCM));
      }

      if (hMp4SpaceEnc->useTimeDomDownmix <= 0) {
        if (SACENC_OK != (error = fdk_sacenc_staticPostGain_ApplyFDK(
                              hMp4SpaceEnc->hStaticGain, pOutputSamples,
                              nOutputSamplesMax, 0))) {
          goto bail;
        }
      }

    } /* !bQmfOutput */

    if (hMp4SpaceEnc->nDiscardOutFrames > 0) {
      hMp4SpaceEnc->nDiscardOutFrames--;
    }

    /* Invalidate Input Buffer */
    hMp4SpaceEnc->nSamplesValid = 0;

  } /* valid handle */
bail:
  return error;
}

FDK_SACENC_ERROR FDK_sacenc_close(HANDLE_MP4SPACE_ENCODER *phMp4SpaceEnc) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if (NULL != phMp4SpaceEnc) {
    if (NULL != *phMp4SpaceEnc) {
      int ch, i;
      HANDLE_MP4SPACE_ENCODER const hEnc = *phMp4SpaceEnc;

      if (hEnc->pParameterBand2HybridBandOffset != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->pParameterBand2HybridBandOffset);
      }
      /* Free Analysis Filterbank Structs */
      if (hEnc->pEncoderInputChScale != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->pEncoderInputChScale);
      }
      if (hEnc->staticTimeDomainDmxInScale != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->staticTimeDomainDmxInScale);
      }
      if (hEnc->phQmfFiltIn__FDK != NULL) {
        for (ch = 0; ch < hEnc->setup.maxChIn; ch++) {
          if (hEnc->phQmfFiltIn__FDK[ch] != NULL) {
            if (hEnc->phQmfFiltIn__FDK[ch]->FilterStates != NULL) {
              FDK_FREE_MEMORY_1D(hEnc->phQmfFiltIn__FDK[ch]->FilterStates);
            }
            FDK_FREE_MEMORY_1D(hEnc->phQmfFiltIn__FDK[ch]);
          }
        }
        FDK_FREE_MEMORY_1D(hEnc->phQmfFiltIn__FDK);
      }
      for (ch = 0; ch < hEnc->setup.maxChIn; ch++) {
        if (NULL != hEnc->phDCFilterSigIn[ch]) {
          fdk_sacenc_destroyDCFilter(&hEnc->phDCFilterSigIn[ch]);
        }
      }
      /* Close Onset Detection */
      for (ch = 0; ch < hEnc->setup.maxChIn; ch++) {
        if (NULL != hEnc->phOnset[ch]) {
          fdk_sacenc_onsetDetect_Close(&hEnc->phOnset[ch]);
        }
      }
      if (hEnc->ppTrCurrPos) {
        FDK_FREE_MEMORY_2D(hEnc->ppTrCurrPos);
      }
      if (hEnc->hFrameWindow) {
        fdk_sacenc_frameWindow_Destroy(&hEnc->hFrameWindow);
      }
      /* Close Space Tree */
      if (NULL != hEnc->hSpaceTree) {
        fdk_sacenc_spaceTree_Close(&hEnc->hSpaceTree);
      }
      if (NULL != hEnc->hEnhancedTimeDmx) {
        fdk_sacenc_close_enhancedTimeDomainDmx(&hEnc->hEnhancedTimeDmx);
      }
      /* Close Static Gain */
      if (NULL != hEnc->hStaticGain) {
        fdk_sacenc_staticGain_Close(&hEnc->hStaticGain);
      }
      if (NULL != hEnc->hStaticGainConfig) {
        fdk_sacenc_staticGain_CloseConfig(&hEnc->hStaticGainConfig);
      }
      /* Close Delay*/
      if (NULL != hEnc->hDelay) {
        fdk_sacenc_delay_Close(&hEnc->hDelay);
      }
      /* Delete Bitstream Stuff */
      if (NULL != hEnc->hBitstreamFormatter) {
        fdk_sacenc_destroySpatialBitstreamEncoder(&(hEnc->hBitstreamFormatter));
      }
      if (hEnc->pppHybridIn__FDK != NULL) {
        if (hEnc->setup.bEncMode_212 == 1) {
          FDK_FREE_MEMORY_3D(hEnc->pppHybridIn__FDK);
          FDK_FREE_MEMORY_3D(hEnc->pppHybridInStatic__FDK);
        } else {
          FDK_FREE_MEMORY_3D(hEnc->pppHybridIn__FDK);
        }
      }
      if (hEnc->pppProcDataIn__FDK != NULL) {
        FDK_FREE_MEMORY_3D(hEnc->pppProcDataIn__FDK);
      }
      if (hEnc->pOutputDelayBuffer__FDK != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->pOutputDelayBuffer__FDK);
      }
      if (hEnc->ppTimeSigIn__FDK != NULL) {
        { FDK_FREE_MEMORY_2D(hEnc->ppTimeSigIn__FDK); }
      }
      if (hEnc->ppTimeSigDelayIn__FDK != NULL) {
        FDK_FREE_MEMORY_2D(hEnc->ppTimeSigDelayIn__FDK);
      }
      if (hEnc->ppTimeSigOut__FDK != NULL) {
        FDK_FREE_MEMORY_2D(hEnc->ppTimeSigOut__FDK);
      }
      for (i = 0; i < MAX_NUM_PARAMS; i++) {
        if (hEnc->pFrameWindowAna__FDK[i] != NULL) {
          FDK_FREE_MEMORY_1D(hEnc->pFrameWindowAna__FDK[i]);
        }
      }
      if (hEnc->pnOutputBits != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->pnOutputBits);
      }
      if (hEnc->ppBitstreamDelayBuffer != NULL) {
        FDK_FREE_MEMORY_2D(hEnc->ppBitstreamDelayBuffer);
      }
      if (hEnc->sscBuf.pSsc != NULL) {
        FDK_FREE_MEMORY_1D(hEnc->sscBuf.pSsc);
      }
      FDK_FREE_MEMORY_1D(*phMp4SpaceEnc);
    }
  }

  return error;
}

/*-----------------------------------------------------------------------------
  functionname: mp4SpaceEnc_InitDelayCompensation()
  description:  initialzes delay compensation
  returns:      noError on success, an apropriate error code else
  -----------------------------------------------------------------------------*/
static FDK_SACENC_ERROR mp4SpaceEnc_InitDelayCompensation(
    HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc, const INT coreCoderDelay) {
  FDK_SACENC_ERROR error = SACENC_OK;

  /* Sanity Check */
  if (hMp4SpaceEnc == NULL) {
    error = SACENC_INVALID_HANDLE;
  } else {
    hMp4SpaceEnc->coreCoderDelay = coreCoderDelay;

    if (SACENC_OK != (error = fdk_sacenc_delay_Init(
                          hMp4SpaceEnc->hDelay, hMp4SpaceEnc->nQmfBands,
                          hMp4SpaceEnc->nFrameLength, coreCoderDelay,
                          hMp4SpaceEnc->timeAlignment))) {
      goto bail;
    }

    fdk_sacenc_delay_SetDmxAlign(hMp4SpaceEnc->hDelay, 0);
    fdk_sacenc_delay_SetTimeDomDmx(
        hMp4SpaceEnc->hDelay, (hMp4SpaceEnc->useTimeDomDownmix >= 1) ? 1 : 0);
    fdk_sacenc_delay_SetMinimizeDelay(hMp4SpaceEnc->hDelay, 1);

    if (SACENC_OK != (error = fdk_sacenc_delay_SubCalulateBufferDelays(
                          hMp4SpaceEnc->hDelay))) {
      goto bail;
    }

    /* init output delay compensation */
    hMp4SpaceEnc->nBitstreamDelayBuffer =
        fdk_sacenc_delay_GetBitstreamFrameBufferSize(hMp4SpaceEnc->hDelay);
    hMp4SpaceEnc->nOutputBufferDelay =
        fdk_sacenc_delay_GetOutputAudioBufferDelay(hMp4SpaceEnc->hDelay);
    hMp4SpaceEnc->nSurroundAnalysisBufferDelay =
        fdk_sacenc_delay_GetSurroundAnalysisBufferDelay(hMp4SpaceEnc->hDelay);
    hMp4SpaceEnc->nBitstreamBufferRead = 0;
    hMp4SpaceEnc->nBitstreamBufferWrite =
        hMp4SpaceEnc->nBitstreamDelayBuffer - 1;

    if (hMp4SpaceEnc->encMode == SACENC_212) {
      /* mode 212 expects no bitstream delay */
      if (hMp4SpaceEnc->nBitstreamBufferWrite !=
          hMp4SpaceEnc->nBitstreamBufferRead) {
        error = SACENC_PARAM_ERROR;
        goto bail;
      }

      /* mode 212 expects no output buffer delay */
      if (hMp4SpaceEnc->nOutputBufferDelay != 0) {
        error = SACENC_PARAM_ERROR;
        goto bail;
      }
    }

    /*** Input delay to obtain a net encoder delay that is a multiple
    of the used framelength to ensure synchronization of framing
    in artistic down-mix with the corresponding spatial data.      ***/
    hMp4SpaceEnc->nDiscardOutFrames =
        fdk_sacenc_delay_GetDiscardOutFrames(hMp4SpaceEnc->hDelay);
    hMp4SpaceEnc->nInputDelay =
        fdk_sacenc_delay_GetDmxAlignBufferDelay(hMp4SpaceEnc->hDelay);

    /* reset independency Flag counter */
    hMp4SpaceEnc->independencyCount = 0;
    hMp4SpaceEnc->independencyFlag = 1;

    int i;

    /* write some parameters to bitstream */
    for (i = 0; i < hMp4SpaceEnc->nBitstreamDelayBuffer - 1; i++) {
      SPATIALFRAME *pFrameData = NULL;

      if (NULL == (pFrameData = fdk_sacenc_getSpatialFrame(
                       hMp4SpaceEnc->hBitstreamFormatter, READ_SPATIALFRAME))) {
        error = SACENC_INVALID_HANDLE;
        goto bail;
      }

      pFrameData->bsIndependencyFlag = 1;
      pFrameData->framingInfo.numParamSets = 1;
      pFrameData->framingInfo.bsFramingType = 0;

      fdk_sacenc_writeSpatialFrame(
          hMp4SpaceEnc->ppBitstreamDelayBuffer[i], MAX_MPEGS_BYTES,
          &hMp4SpaceEnc->pnOutputBits[i], hMp4SpaceEnc->hBitstreamFormatter);
    }

    if ((hMp4SpaceEnc->nInputDelay > MAX_DELAY_INPUT) ||
        (hMp4SpaceEnc->nOutputBufferDelay > MAX_DELAY_OUTPUT) ||
        (hMp4SpaceEnc->nSurroundAnalysisBufferDelay >
         MAX_DELAY_SURROUND_ANALYSIS) ||
        (hMp4SpaceEnc->nBitstreamDelayBuffer > MAX_BITSTREAM_DELAY)) {
      error = SACENC_INIT_ERROR;
      goto bail;
    }
  }

bail:

  return error;
}

static QUANTMODE __mapQuantMode(const MP4SPACEENC_QUANTMODE quantMode) {
  QUANTMODE bsQuantMode = QUANTMODE_INVALID;

  switch (quantMode) {
    case SACENC_QUANTMODE_FINE:
      bsQuantMode = QUANTMODE_FINE;
      break;
    case SACENC_QUANTMODE_EBQ1:
      bsQuantMode = QUANTMODE_EBQ1;
      break;
    case SACENC_QUANTMODE_EBQ2:
      bsQuantMode = QUANTMODE_EBQ2;
      break;
    case SACENC_QUANTMODE_RSVD3:
    case SACENC_QUANTMODE_INVALID:
    default:
      bsQuantMode = QUANTMODE_INVALID;
  } /* switch hEnc->quantMode */

  return bsQuantMode;
}

static FDK_SACENC_ERROR FillSpatialSpecificConfig(
    const HANDLE_MP4SPACE_ENCODER hEnc, SPATIALSPECIFICCONFIG *const hSsc) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL == hEnc) || (NULL == hSsc)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    SPACE_TREE_DESCRIPTION spaceTreeDescription;
    int i;

    /* Get tree description */
    if (SACENC_OK != (error = fdk_sacenc_spaceTree_GetDescription(
                          hEnc->hSpaceTree, &spaceTreeDescription))) {
      goto bail;
    }

    /* Fill SSC */
    FDKmemclear(hSsc, sizeof(SPATIALSPECIFICCONFIG)); /* reset */

    hSsc->numBands = hEnc->spaceTreeSetup.nParamBands; /* for bsFreqRes */

    /* Fill tree configuration */
    hSsc->treeDescription.numOttBoxes = spaceTreeDescription.nOttBoxes;
    hSsc->treeDescription.numInChan = spaceTreeDescription.nInChannels;
    hSsc->treeDescription.numOutChan = spaceTreeDescription.nOutChannels;

    for (i = 0; i < SACENC_MAX_NUM_BOXES; i++) {
      hSsc->ottConfig[i].bsOttBands = hSsc->numBands;
    }

    switch (hEnc->encMode) {
      case SACENC_212:
        hSsc->bsTreeConfig = TREE_212;
        break;
      case SACENC_INVALID_MODE:
      default:
        error = SACENC_INVALID_CONFIG;
        goto bail;
    }

    hSsc->bsSamplingFrequency =
        hEnc->nSampleRate; /* for bsSamplingFrequencyIndex */
    hSsc->bsFrameLength = hEnc->nFrameTimeSlots - 1;

    /* map decorr type */
    if (DECORR_INVALID ==
        (hSsc->bsDecorrConfig = mp4SpaceEnc_GetDecorrConfig(hEnc->encMode))) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /* map quantMode */
    if (QUANTMODE_INVALID ==
        (hSsc->bsQuantMode = __mapQuantMode(hEnc->quantMode))) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    /* Configure Gains*/
    hSsc->bsFixedGainDMX = fdk_sacenc_staticGain_GetDmxGain(hEnc->hStaticGain);
    hSsc->bsEnvQuantMode = 0;

  } /* valid handle */

bail:
  return error;
}

static FDK_SACENC_ERROR mp4SpaceEnc_FillSpaceTreeSetup(
    const HANDLE_MP4SPACE_ENCODER hEnc,
    SPACE_TREE_SETUP *const hSpaceTreeSetup) {
  FDK_SACENC_ERROR error = SACENC_OK;

  /* Sanity Check */
  if (NULL == hEnc || NULL == hSpaceTreeSetup) {
    error = SACENC_INVALID_HANDLE;
  } else {
    QUANTMODE tmpQuantmode = QUANTMODE_INVALID;

    /* map quantMode */
    if (QUANTMODE_INVALID == (tmpQuantmode = __mapQuantMode(hEnc->quantMode))) {
      error = SACENC_INVALID_CONFIG;
      goto bail;
    }

    hSpaceTreeSetup->nParamBands = hEnc->nParamBands;
    hSpaceTreeSetup->bUseCoarseQuantTtoCld = hEnc->useCoarseQuantCld;
    hSpaceTreeSetup->bUseCoarseQuantTtoIcc = hEnc->useCoarseQuantIcc;
    hSpaceTreeSetup->quantMode = tmpQuantmode;
    hSpaceTreeSetup->nHybridBandsMax = hEnc->nHybridBands;

    switch (hEnc->encMode) {
      case SACENC_212:
        hSpaceTreeSetup->mode = SPACETREE_212;
        hSpaceTreeSetup->nChannelsInMax = 2;
        break;
      case SACENC_INVALID_MODE:
      default:
        error = SACENC_INVALID_CONFIG;
        goto bail;
    } /* switch hEnc->encMode */

  } /* valid handle */
bail:
  return error;
}

FDK_SACENC_ERROR FDK_sacenc_getInfo(const HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                    MP4SPACEENC_INFO *const pInfo) {
  FDK_SACENC_ERROR error = SACENC_OK;

  if ((NULL == hMp4SpaceEnc) || (NULL == pInfo)) {
    error = SACENC_INVALID_HANDLE;
  } else {
    pInfo->nSampleRate = hMp4SpaceEnc->nSampleRate;
    pInfo->nSamplesFrame = hMp4SpaceEnc->nFrameLength;
    pInfo->nTotalInputChannels = hMp4SpaceEnc->nInputChannels;
    pInfo->nDmxDelay = fdk_sacenc_delay_GetInfoDmxDelay(hMp4SpaceEnc->hDelay);
    pInfo->nCodecDelay =
        fdk_sacenc_delay_GetInfoCodecDelay(hMp4SpaceEnc->hDelay);
    pInfo->nDecoderDelay =
        fdk_sacenc_delay_GetInfoDecoderDelay(hMp4SpaceEnc->hDelay);
    pInfo->nPayloadDelay =
        fdk_sacenc_delay_GetBitstreamFrameBufferSize(hMp4SpaceEnc->hDelay) - 1;
    pInfo->nDiscardOutFrames = hMp4SpaceEnc->nDiscardOutFrames;

    pInfo->pSscBuf = &hMp4SpaceEnc->sscBuf;
  }
  return error;
}

FDK_SACENC_ERROR FDK_sacenc_setParam(HANDLE_MP4SPACE_ENCODER hMp4SpaceEnc,
                                     const SPACEENC_PARAM param,
                                     const UINT value) {
  FDK_SACENC_ERROR error = SACENC_OK;

  /* check encoder handle */
  if (hMp4SpaceEnc == NULL) {
    error = SACENC_INVALID_HANDLE;
    goto bail;
  }

  /* apply param value */
  switch (param) {
    case SACENC_LOWDELAY:
      if (!((value == 0) || (value == 1) || (value == 2))) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.bLdMode = value;
      break;

    case SACENC_ENC_MODE:
      switch ((MP4SPACEENC_MODE)value) {
        case SACENC_212:
          hMp4SpaceEnc->user.encMode = (MP4SPACEENC_MODE)value;
          break;
        default:
          error = SACENC_INVALID_CONFIG;
      }
      break;

    case SACENC_SAMPLERATE:
      if (((int)value < 0) ||
          ((int)value > hMp4SpaceEnc->setup.maxSamplingrate)) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.sampleRate = value;
      break;

    case SACENC_FRAME_TIME_SLOTS:
      if (((int)value < 0) ||
          ((int)value > hMp4SpaceEnc->setup.maxFrameTimeSlots)) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.frameTimeSlots = value;
      break;

    case SACENC_PARAM_BANDS:
      switch ((MP4SPACEENC_BANDS_CONFIG)value) {
        case SACENC_BANDS_4:
        case SACENC_BANDS_5:
        case SACENC_BANDS_7:
        case SACENC_BANDS_9:
        case SACENC_BANDS_12:
        case SACENC_BANDS_15:
        case SACENC_BANDS_23:
          hMp4SpaceEnc->user.nParamBands = (MP4SPACEENC_BANDS_CONFIG)value;
          break;
        default:
          error = SACENC_INVALID_CONFIG;
      }
      break;

    case SACENC_TIME_DOM_DMX:
      if (!((value == 0) || (value == 2))) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.bTimeDomainDmx = value;
      break;

    case SACENC_DMX_GAIN:
      if (!((value == 0) || (value == 1) || (value == 2) || (value == 3) ||
            (value == 4) || (value == 5) || (value == 6) || (value == 7))) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      error = fdk_sacenc_staticGain_SetDmxGain(hMp4SpaceEnc->hStaticGainConfig,
                                               (MP4SPACEENC_DMX_GAIN)value);
      break;

    case SACENC_COARSE_QUANT:
      if (!((value == 0) || (value == 1))) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.bUseCoarseQuant = value;
      break;

    case SACENC_QUANT_MODE:
      switch ((MP4SPACEENC_QUANTMODE)value) {
        case SACENC_QUANTMODE_FINE:
        case SACENC_QUANTMODE_EBQ1:
        case SACENC_QUANTMODE_EBQ2:
          hMp4SpaceEnc->user.quantMode = (MP4SPACEENC_QUANTMODE)value;
          break;
        default:
          error = SACENC_INVALID_CONFIG;
      }
      break;

    case SACENC_TIME_ALIGNMENT:
      if ((INT)value < -32768 || (INT)value > 32767) {
        error = SACENC_INVALID_CONFIG;
        break;
      }
      hMp4SpaceEnc->user.timeAlignment = value;
      break;

    case SACENC_INDEPENDENCY_COUNT:
      hMp4SpaceEnc->independencyCount = value;
      break;

    case SACENC_INDEPENDENCY_FACTOR:
      hMp4SpaceEnc->user.independencyFactor = value;
      break;

    default:
      error = SACENC_UNSUPPORTED_PARAMETER;
      break;
  } /* switch(param) */
bail:
  return error;
}

FDK_SACENC_ERROR FDK_sacenc_getLibInfo(LIB_INFO *info) {
  int i = 0;

  if (info == NULL) {
    return SACENC_INVALID_HANDLE;
  }

  FDK_toolsGetLibInfo(info);

  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) {
    return SACENC_INIT_ERROR;
  }

  info[i].module_id = FDK_MPSENC;
  info[i].build_date = SACENC_LIB_BUILD_DATE;
  info[i].build_time = SACENC_LIB_BUILD_TIME;
  info[i].title = SACENC_LIB_TITLE;
  info[i].version = LIB_VERSION(SACENC_LIB_VL0, SACENC_LIB_VL1, SACENC_LIB_VL2);
  LIB_VERSION_STRING(&info[i]);

  /* Capability flags */
  info[i].flags = 0;
  /* End of flags */

  return SACENC_OK;
}

static DECORRCONFIG mp4SpaceEnc_GetDecorrConfig(
    const MP4SPACEENC_MODE encMode) {
  DECORRCONFIG decorrConfig = DECORR_INVALID;

  /* set decorrConfig dependent on tree mode */
  switch (encMode) {
    case SACENC_212:
      decorrConfig = DECORR_QMFSPLIT0;
      break;
    case SACENC_INVALID_MODE:
    default:
      decorrConfig = DECORR_INVALID;
  }
  return decorrConfig;
}

static FDK_SACENC_ERROR mp4SpaceEnc_InitNumParamBands(
    HANDLE_MP4SPACE_ENCODER hEnc, const MP4SPACEENC_BANDS_CONFIG nParamBands) {
  FDK_SACENC_ERROR error = SACENC_OK;

  /* Set/Check nParamBands */
  int k = 0;
  const int n = sizeof(pValidBands_Ld) / sizeof(UCHAR);
  const UCHAR *pBands = pValidBands_Ld;

  while (k < n && pBands[k] != (UCHAR)nParamBands) ++k;
  if (k == n) {
    hEnc->nParamBands = SACENC_BANDS_INVALID;
  } else {
    hEnc->nParamBands = nParamBands;
  }
  return error;
}
