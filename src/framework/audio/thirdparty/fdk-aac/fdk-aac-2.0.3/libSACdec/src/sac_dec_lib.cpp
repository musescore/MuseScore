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

   Description: SAC Decoder Library Interface

*******************************************************************************/

#include "sac_dec_lib.h"
#include "sac_dec_interface.h"
#include "sac_dec.h"
#include "sac_bitdec.h"
#include "FDK_matrixCalloc.h"

#define MPS_DATA_BUFFER_SIZE (2048)

/**
 * \brief MPEG Surround data indication.
 **/
typedef enum {
  MPEGS_ANCTYPE_FRAME = 0, /*!< MPEG Surround frame, see ISO/IEC 23003-1 */
  MPEGS_ANCTYPE_HEADER_AND_FRAME = 1, /*!< MPEG Surround header and MPEG
                                         Surround frame, see ISO/IEC 23003-1 */
  MPEGS_ANCTYPE_RESERVED_1 = 2,       /*!< reserved, see ISO/IEC 23003-1 */
  MPEGS_ANCTYPE_RESERVED_2 = 3        /*!< reserved, see ISO/IEC 23003-1*/
} MPEGS_ANCTYPE;

/**
 * \brief MPEG Surround data segment indication.
 **/
typedef enum {
  MPEGS_CONTINUE = 0, /*!< Indicates if data segment continues a data block. */
  MPEGS_STOP = 1,     /*!< Indicates if data segment ends a data block. */
  MPEGS_START = 2,    /*!< Indicates if data segment begins a data block. */
  MPEGS_START_STOP =
      3 /*!< Indicates if data segment begins and ends a data block. */
} MPEGS_ANCSTARTSTOP;

/**
 * \brief MPEG Surround synchronizaiton state.
 *
 *  CAUTION: Changing the enumeration values can break the sync mechanism
 *because it is based on comparing the state values.
 **/
typedef enum {
  MPEGS_SYNC_LOST =
      0, /*!< Indicates lost sync because of current discontinuity. */
  MPEGS_SYNC_FOUND = 1,   /*!< Parsed a valid header and (re)intialization was
                             successfully completed. */
  MPEGS_SYNC_COMPLETE = 2 /*!< In sync and continuous. Found an independent
                             frame in addition to MPEGS_SYNC_FOUND.
                               Precondition: MPEGS_SYNC_FOUND. */
} MPEGS_SYNCSTATE;

/**
 * \brief MPEG Surround operation mode.
 **/
typedef enum {
  MPEGS_OPMODE_EMM = 0,           /*!< Mode: Enhanced Matrix Mode (Blind) */
  MPEGS_OPMODE_MPS_PAYLOAD = 1,   /*!< Mode: Normal, Stereo or Binaural */
  MPEGS_OPMODE_NO_MPS_PAYLOAD = 2 /*!< Mode: no MPEG Surround payload */
} MPEGS_OPMODE;

/**
 * \brief MPEG Surround init flags.
 **/
typedef enum {
  MPEGS_INIT_OK = 0x00000000, /*!< indicate correct initialization */
  MPEGS_INIT_ENFORCE_REINIT =
      0x00000001, /*!< indicate complete initialization */

  MPEGS_INIT_CHANGE_OUTPUT_MODE =
      0x00000010, /*!< indicate change of the output mode */
  MPEGS_INIT_CHANGE_PARTIALLY_COMPLEX =
      0x00000020, /*!< indicate change of low power/high quality */
  MPEGS_INIT_CHANGE_TIME_FREQ_INTERFACE =
      0x00000040, /*!< indicate change of qmf/time interface */
  MPEGS_INIT_CHANGE_HEADER = 0x00000080, /*!< indicate change of header */

  MPEGS_INIT_ERROR_PAYLOAD =
      0x00000100, /*!< indicate payload/ancType/ancStartStop error */

  MPEGS_INIT_BS_INTERRUPTION =
      0x00001000, /*!< indicate bitstream interruption  */
  MPEGS_INIT_CLEAR_HISTORY =
      0x00002000, /*!< indicate that all states shall be cleared */

  /* Re-initialization of submodules */

  MPEGS_INIT_CHANGE_CONCEAL_PARAMS = 0x00100000, /*!< indicate a change of at
                                                    least one error concealment
                                                    param */

  /* No re-initialization needed, currently not used */
  MPEGS_INIT_CHANGE_BYPASS_MODE =
      0x01000000, /*!< indicate change of bypass mode */

  /* Re-initialization needed, currently not used */
  MPEGS_INIT_ERROR_ANC_TYPE = 0x10000000, /*!< indicate ancType error*/
  MPEGS_INIT_ERROR_ANC_STARTSTOP =
      0x20000000 /*!< indicate ancStartStop error */
} MPEGS_INIT_FLAGS;

struct MpegSurroundDecoder {
  HANDLE_FDK_QMF_DOMAIN pQmfDomain;
  UCHAR mpsData[MPS_DATA_BUFFER_SIZE]; /* Buffer for MPS payload accross more
                                          than one segment */
  INT mpsDataBits;                     /* Amount of bits in mpsData */
  /* MPEG Surround decoder */
  SPATIAL_SPECIFIC_CONFIG spatialSpecificConfig[1]; /* SSC delay line which is
                                                       used during decoding */
  spatialDec *pSpatialDec;
  SPATIAL_SPECIFIC_CONFIG
  spatialSpecificConfigBackup; /* SSC used while parsing */

  /* Creation parameter */
  UCHAR mpegSurroundDecoderLevel;
  /* Run-time parameter */
  UCHAR mpegSurroundSscIsGlobalCfg; /* Flag telling that the SSC
                                       (::spatialSpecificConfig) is a
                                       out-of-band configuration. */
  UCHAR mpegSurroundUseTimeInterface;

  SPATIAL_BS_FRAME
  bsFrames[1];         /* Bitstream Structs that contain data read from the
                          SpatialFrame() bitstream element */
  BS_LL_STATE llState; /* Bit stream parser state memory */
  UCHAR bsFrameParse;  /* Current parse frame context index */
  UCHAR bsFrameDecode; /* Current decode/apply frame context index */
  UCHAR bsFrameDelay;  /* Amount of frames delay between parsing and processing.
                          Required i.e. for interpolation error concealment. */

  /* User prameters */
  SPATIALDEC_PARAM mpegSurroundUserParams;

  /* Internal flags */
  SPATIAL_DEC_UPMIX_TYPE upmixType;
  int initFlags[1];
  MPEGS_ANCSTARTSTOP ancStartStopPrev;
  MPEGS_SYNCSTATE fOnSync[1];

  /* Inital decoder configuration */
  SPATIAL_DEC_CONFIG decConfig;
};

SACDEC_ERROR
static sscCheckOutOfBand(const SPATIAL_SPECIFIC_CONFIG *pSsc,
                         const INT coreCodec, const INT sampleRate,
                         const INT frameSize);

static SACDEC_ERROR sscParseCheck(const SPATIAL_SPECIFIC_CONFIG *pSsc);

/**
 * \brief Get the number of QMF bands from the sampling frequency (in Hz)
 **/
static int mpegSurroundDecoder_GetNrOfQmfBands(
    const SPATIAL_SPECIFIC_CONFIG *pSsc, UINT sampleRate) {
  UINT samplingFrequency = sampleRate;
  int qmfBands = 64;

  if (pSsc != NULL) {
    switch (pSsc->coreCodec) {
      case AOT_USAC:
        if ((pSsc->stereoConfigIndex == 3)) {
          static const UCHAR mapIdx2QmfBands[3] = {24, 32, 16};
          FDK_ASSERT((pSsc->coreSbrFrameLengthIndex >= 2) &&
                     (pSsc->coreSbrFrameLengthIndex <= 4));
          qmfBands = mapIdx2QmfBands[pSsc->coreSbrFrameLengthIndex - 2];
        }
        return qmfBands;
      default:
        samplingFrequency = pSsc->samplingFreq;
        break;
    }
  }

  /* number of QMF bands depend on sampling frequency, see FDIS 23003-1:2006
   * Chapter 6.3.3 */
  if (samplingFrequency < 27713) {
    qmfBands = 32;
  }
  if (samplingFrequency > 55426) {
    qmfBands = 128;
  }

  return qmfBands;
}

/**
 * \brief Analyse init flags
 **/
static int mpegSurroundDecoder_CalcInitFlags(SPATIAL_SPECIFIC_CONFIG *pSsc1,
                                             SPATIAL_SPECIFIC_CONFIG *pSsc2,
                                             int upmixTypeFlag,
                                             int binauralQualityFlag,
                                             int partiallyComplexFlag,
                                             int *ctrlFlags) {
  /* Analyse core coder */
  if (pSsc1->coreCodec != pSsc2->coreCodec) {
    *ctrlFlags |= MASK_MPEGS_INIT_ALL_STATES;
    *ctrlFlags |= MASK_MPEGS_INIT_ALL_PARAMS;
  } else {
    /* Analyse elements for initialization of space analysis qmf filterbank */
    if ((partiallyComplexFlag) || (pSsc1->treeConfig != pSsc2->treeConfig) ||
        (pSsc1->samplingFreq != pSsc2->samplingFreq)) {
      *ctrlFlags |= MPEGS_INIT_STATES_ANA_QMF_FILTER;
      *ctrlFlags |= MPEGS_INIT_STATES_ANA_HYB_FILTER;
    }

    /* Analyse elements for initialization of space synthesis qmf filterbank */
    if ((upmixTypeFlag) || (partiallyComplexFlag) ||
        (pSsc1->treeConfig != pSsc2->treeConfig) ||
        (pSsc1->samplingFreq != pSsc2->samplingFreq) ||
        (pSsc1->bsFixedGainDMX != pSsc2->bsFixedGainDMX)) {
      *ctrlFlags |= MPEGS_INIT_STATES_SYN_QMF_FILTER;
    }

    /* Analyse elements for initialization of decorrelator */
    if ((upmixTypeFlag) || (partiallyComplexFlag) ||
        (pSsc1->treeConfig != pSsc2->treeConfig) ||
        (pSsc1->samplingFreq != pSsc2->samplingFreq) ||
        (pSsc1->decorrConfig != pSsc2->decorrConfig)) {
      *ctrlFlags |= MPEGS_INIT_STATES_DECORRELATOR;
    }

    /* Analyse elements for initialization of m1 and m2 calculation */
    if ((upmixTypeFlag) || (binauralQualityFlag) ||
        (pSsc1->treeConfig != pSsc2->treeConfig) ||
        (pSsc1->samplingFreq != pSsc2->samplingFreq))

    {
      *ctrlFlags |= MPEGS_INIT_STATES_M1M2;
    }

    /* Analyse elements for initialization of GES */
    if ((upmixTypeFlag) || (pSsc1->treeConfig != pSsc2->treeConfig) ||
        (pSsc1->tempShapeConfig != pSsc2->tempShapeConfig)) {
      *ctrlFlags |= MPEGS_INIT_STATES_GES;
    }

    /* Analyse elements for initialization of FDreverb */
    if ((upmixTypeFlag) || (binauralQualityFlag) || (partiallyComplexFlag) ||
        (pSsc1->samplingFreq != pSsc2->samplingFreq) ||
        (pSsc1->nTimeSlots != pSsc2->nTimeSlots)) {
      *ctrlFlags |= MPEGS_INIT_STATES_REVERB;
    }

    /* Reset previous frame data whenever the config changes */
    if (*ctrlFlags & MPEGS_INIT_CONFIG) {
      *ctrlFlags |= MPEGS_INIT_STATES_PARAM;
    }
  }

  return MPS_OK;
}

/**
 * \brief Reset MPEG Surround status info
 **/
static void updateMpegSurroundDecoderStatus(
    CMpegSurroundDecoder *pMpegSurroundDecoder, int initFlags,
    MPEGS_SYNCSTATE fOnSync, MPEGS_ANCSTARTSTOP ancStartStopPrev) {
  pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
      initFlags;
  if ((pMpegSurroundDecoder->mpegSurroundSscIsGlobalCfg != 0) &&
      (pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] >=
       MPEGS_SYNC_FOUND) &&
      (fOnSync < MPEGS_SYNC_FOUND)) {
    pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
        MPEGS_SYNC_FOUND;
  } else {
    pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
        fOnSync;
  }
  pMpegSurroundDecoder->ancStartStopPrev = ancStartStopPrev;
}

static SACDEC_ERROR mpegSurroundDecoder_Create(
    CMpegSurroundDecoder **pMpegSurroundDecoder, int stereoConfigIndex,
    HANDLE_FDK_QMF_DOMAIN pQmfDomain);

SAC_INSTANCE_AVAIL
mpegSurroundDecoder_IsFullMpegSurroundDecoderInstanceAvailable(
    CMpegSurroundDecoder *pMpegSurroundDecoder) {
  SAC_INSTANCE_AVAIL instanceAvailable = SAC_INSTANCE_NOT_FULL_AVAILABLE;

  if (pMpegSurroundDecoder->pSpatialDec != NULL) {
    instanceAvailable = SAC_INSTANCE_FULL_AVAILABLE;
  }

  return instanceAvailable;
}

SACDEC_ERROR mpegSurroundDecoder_Open(
    CMpegSurroundDecoder **pMpegSurroundDecoder, int stereoConfigIndex,
    HANDLE_FDK_QMF_DOMAIN pQmfDomain) {
  SACDEC_ERROR error;

  error = mpegSurroundDecoder_Create(pMpegSurroundDecoder, stereoConfigIndex,
                                     pQmfDomain);

  return error;
}

/**
 * \brief  Renamed function from getUpmixType to check_UParam_Build_DecConfig.
 *         This function checks if user params, decoder config and SSC are valid
 *         and if the decoder build can handle all this settings.
 *         The upmix type may be modified by this function.
 *         It is called in initMpegSurroundDecoder() after the ssc parse check,
 *         to have all checks in one place and to ensure these checks are always
 *         performed if config changes (inband and out-of-band).
 *
 * \param pUserParams  User data handle.
 * \param pDecConfig   decoder config handle.
 * \param pSsc         spatial specific config handle.
 * \param pUpmixType   upmix type which is set by this function
 *
 * \return  MPS_OK on sucess, and else on failure.
 */
static SACDEC_ERROR check_UParam_Build_DecConfig(
    SPATIALDEC_PARAM const *pUserParams, SPATIAL_DEC_CONFIG const *pDecConfig,
    const SPATIAL_SPECIFIC_CONFIG *pSsc, SPATIAL_DEC_UPMIX_TYPE *pUpmixType) {
  int dmxChannels, outChannels, maxNumOutChannels;

  FDK_ASSERT(pUserParams != NULL);
  FDK_ASSERT(pUpmixType != NULL);

  /* checks if implementation can handle the Ssc */

  switch (pSsc->treeConfig) {
    case SPATIALDEC_MODE_RSVD7: /* 212 */
      dmxChannels = 1;
      outChannels = 2;
      break;
    default:
      return MPS_UNSUPPORTED_CONFIG;
  }

  /* ------------------------------------------- */

  /* Analyse pDecConfig params */
  switch (pDecConfig->binauralMode) {
    case BINAURAL_NONE:
      break;
    default:
      return MPS_UNSUPPORTED_CONFIG;
  }

  switch (pDecConfig->decoderMode) {
    case EXT_HQ_ONLY:
      break;
    default:
      return MPS_UNSUPPORTED_CONFIG;
  }

  switch (pDecConfig->maxNumOutputChannels) {
    case OUTPUT_CHANNELS_DEFAULT:
      /* No special restrictions -> Get the level restriction: */
      switch (pDecConfig->decoderLevel) {
        case DECODER_LEVEL_0:
          maxNumOutChannels = 2;
          break;
        default:
          return MPS_UNSUPPORTED_CONFIG;
      }
      break;
    case OUTPUT_CHANNELS_2_0:
      maxNumOutChannels = 2;
      break;
    default:
      return MPS_UNSUPPORTED_CONFIG;
  }
  /* ------------------------- */

  /* check if we can handle user params */
  if (pUserParams->blindEnable == 1) {
    return MPS_UNSUPPORTED_CONFIG;
  }
  {
    switch ((SAC_DEC_OUTPUT_MODE)pUserParams->outputMode) {
      case SACDEC_OUT_MODE_NORMAL:
        if (maxNumOutChannels >= outChannels) {
          *pUpmixType = UPMIX_TYPE_NORMAL;
        } else {
          { *pUpmixType = UPMIX_TYPE_BYPASS; }
        }
        break;
      case SACDEC_OUT_MODE_STEREO:
        if (dmxChannels == 1) {
          if (outChannels == 2) {
            *pUpmixType = UPMIX_TYPE_NORMAL;
          }
        } else {
          *pUpmixType = UPMIX_TYPE_BYPASS;
        }
        break;
      case SACDEC_OUT_MODE_6CHANNEL:
        if (outChannels > 6) {
          { *pUpmixType = UPMIX_TYPE_BYPASS; }
        } else {
          *pUpmixType = UPMIX_TYPE_NORMAL;
        }
        break;
      default:
        return MPS_UNSUPPORTED_CONFIG;
    }
  }

  return MPS_OK;
}

/**
 * \brief Init MPEG Surround decoder.
 **/
static SACDEC_ERROR initMpegSurroundDecoder(
    CMpegSurroundDecoder *pMpegSurroundDecoder) {
  SACDEC_ERROR err;
  int initFlags = MPEGS_INIT_NONE, initFlagsDec;
  int upmixTypeCurr = pMpegSurroundDecoder->upmixType;

  FDK_ASSERT(pMpegSurroundDecoder != NULL);

  SPATIAL_SPECIFIC_CONFIG *const pSSCinput =
      &pMpegSurroundDecoder->spatialSpecificConfigBackup;
  SPATIAL_SPECIFIC_CONFIG *const pSSCtarget =
      &pMpegSurroundDecoder
           ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameDecode];
  initFlagsDec =
      pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode];

  if (pSSCinput->coreCodec != AOT_USAC) {
    /* here we check if we have a valid Ssc */
    err = sscParseCheck(pSSCinput);
    if (err != MPS_OK) goto bail;
  }

  /* here we check if Ssc matches build; also check UParams and DecConfig */
  /* if desired upmixType is changes                                      */
  err = check_UParam_Build_DecConfig(
      &pMpegSurroundDecoder->mpegSurroundUserParams,
      &pMpegSurroundDecoder->decConfig, pSSCinput,
      &pMpegSurroundDecoder->upmixType);
  if (err != MPS_OK) goto bail;

  /* init config */
  if (initFlagsDec & MPEGS_INIT_CHANGE_HEADER) {
    initFlags |= MPEGS_INIT_CONFIG;
  }
  /* init all states */
  if (initFlagsDec & MPEGS_INIT_CLEAR_HISTORY) {
    initFlags |= MASK_MPEGS_INIT_ALL_STATES;
  }
  if (initFlagsDec & MPEGS_INIT_CHANGE_CONCEAL_PARAMS) {
    initFlags |= MPEGS_INIT_PARAMS_ERROR_CONCEALMENT;
  }

  if (initFlagsDec & MPEGS_INIT_ENFORCE_REINIT) {
    /* init all states */
    initFlags |= MASK_MPEGS_INIT_ALL_STATES;
    initFlags |= MASK_MPEGS_INIT_ALL_PARAMS;
  } else {
    /* analyse states which have to be initialized */
    mpegSurroundDecoder_CalcInitFlags(
        pSSCtarget, pSSCinput,
        (upmixTypeCurr !=
         pMpegSurroundDecoder->upmixType), /* upmixType changed */
        0, (initFlagsDec & MPEGS_INIT_CHANGE_PARTIALLY_COMPLEX) ? 1 : 0,
        &initFlags);
  }

  {
    int nrOfQmfBands;
    FDKmemcpy(pSSCtarget, pSSCinput, sizeof(SPATIAL_SPECIFIC_CONFIG));

    nrOfQmfBands = mpegSurroundDecoder_GetNrOfQmfBands(
        pSSCtarget, pSSCtarget->samplingFreq);
    err = FDK_SpatialDecInit(
        pMpegSurroundDecoder->pSpatialDec,
        &pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameDecode],
        pSSCtarget, nrOfQmfBands, pMpegSurroundDecoder->upmixType,
        &pMpegSurroundDecoder->mpegSurroundUserParams, initFlags);

    if (err != MPS_OK) goto bail;

    /* Signal that we got a header and can go on decoding */
    if (err == MPS_OK) {
      initFlagsDec = MPEGS_INIT_OK;
      {
        pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
            MPEGS_SYNC_FOUND;
      }
    }
  }

bail:
  pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] =
      initFlagsDec;
  return err;
}

/**
 * \brief Init MPEG Surround decoder.
 **/
SACDEC_ERROR mpegSurroundDecoder_Init(
    CMpegSurroundDecoder *pMpegSurroundDecoder) {
  SACDEC_ERROR err = MPS_OK;

  if (pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode]) {
    err = initMpegSurroundDecoder(pMpegSurroundDecoder);
  }
  return err;
}

/**
 * \brief Open MPEG Surround decoder.
 **/
static SACDEC_ERROR mpegSurroundDecoder_Create(
    CMpegSurroundDecoder **pMpegSurroundDecoder, int stereoConfigIndex,
    HANDLE_FDK_QMF_DOMAIN pQmfDomain) {
  SACDEC_ERROR err = MPS_OK;
  CMpegSurroundDecoder *sacDec = NULL;
  spatialDec *self = NULL;

  /* decoderLevel  decoderMode  maxNumOutputChannels  binauralMode */
  static const SPATIAL_DEC_CONFIG decConfig = {
      (CFG_LEVEL)(0), EXT_HQ_ONLY, OUTPUT_CHANNELS_DEFAULT, BINAURAL_NONE};

  if (*pMpegSurroundDecoder == NULL) {
    FDK_ALLOCATE_MEMORY_1D(*pMpegSurroundDecoder, 1, CMpegSurroundDecoder)

    for (int i = 0; i < 1; i++) {
      err = SpatialDecCreateBsFrame(&(*pMpegSurroundDecoder)->bsFrames[i],
                                    &(*pMpegSurroundDecoder)->llState);
      if (err != MPS_OK) {
        sacDec = *pMpegSurroundDecoder;
        goto bail;
      }
    }
    (*pMpegSurroundDecoder)->pQmfDomain = pQmfDomain;

    (*pMpegSurroundDecoder)->bsFrameDelay = 1;
    (*pMpegSurroundDecoder)->bsFrameParse = 0;
    (*pMpegSurroundDecoder)->bsFrameDecode = 0;

    return err;
  } else {
    sacDec = *pMpegSurroundDecoder;
  }

  if (sacDec->pSpatialDec == NULL) {
    if ((self = FDK_SpatialDecOpen(&decConfig, stereoConfigIndex)) == NULL) {
      err = MPS_OUTOFMEMORY;
      goto bail;
    }
  } else {
    self = sacDec->pSpatialDec;
  }

  self->pQmfDomain = sacDec->pQmfDomain;

  sacDec->pSpatialDec = self;

  /* default parameter set */
  sacDec->mpegSurroundUserParams.outputMode = SACDEC_OUT_MODE_NORMAL;
  sacDec->mpegSurroundUserParams.blindEnable = 0;
  sacDec->mpegSurroundUserParams.bypassMode = 0;
  sacDec->mpegSurroundUserParams.concealMethod = 1;
  sacDec->mpegSurroundUserParams.concealNumKeepFrames = 10;
  sacDec->mpegSurroundUserParams.concealFadeOutSlopeLength = 5;
  sacDec->mpegSurroundUserParams.concealFadeInSlopeLength = 5;
  sacDec->mpegSurroundUserParams.concealNumReleaseFrames = 3;
  sacDec->mpegSurroundSscIsGlobalCfg = 0;
  sacDec->mpegSurroundUseTimeInterface = 1;
  sacDec->mpegSurroundDecoderLevel = decConfig.decoderLevel;

  sacDec->upmixType = UPMIX_TYPE_NORMAL;

  /* signalize spatial decoder re-initalization */
  updateMpegSurroundDecoderStatus(sacDec, MPEGS_INIT_ENFORCE_REINIT,
                                  MPEGS_SYNC_LOST, MPEGS_STOP);

  /* return decoder instance */
  *pMpegSurroundDecoder = sacDec;
  sacDec->decConfig = decConfig;

  SpatialDecInitParserContext(sacDec->pSpatialDec);

  return err;

bail:
  if (sacDec != NULL) {
    mpegSurroundDecoder_Close(sacDec);
  }
  *pMpegSurroundDecoder = NULL;
  if (err == MPS_OK) {
    return MPS_OUTOFMEMORY;
  } else {
    return err;
  }
}

/**
 * \brief Config MPEG Surround decoder.
 **/
SACDEC_ERROR mpegSurroundDecoder_Config(
    CMpegSurroundDecoder *pMpegSurroundDecoder, HANDLE_FDK_BITSTREAM hBs,
    AUDIO_OBJECT_TYPE coreCodec, INT samplingRate, INT frameSize,
    INT numChannels, INT stereoConfigIndex, INT coreSbrFrameLengthIndex,
    INT configBytes, const UCHAR configMode, UCHAR *configChanged) {
  SACDEC_ERROR err = MPS_OK;
  INT nInputChannels;
  SPATIAL_SPECIFIC_CONFIG spatialSpecificConfig;
  SPATIAL_SPECIFIC_CONFIG *pSsc =
      &pMpegSurroundDecoder->spatialSpecificConfigBackup;

  switch (coreCodec) {
    case AOT_DRM_USAC:
    case AOT_USAC:
      if (configMode == AC_CM_DET_CFG_CHANGE) {
        /* In config detection mode write spatial specific config parameters
         * into temporarily allocated structure */
        err = SpatialDecParseMps212Config(
            hBs, &spatialSpecificConfig, samplingRate, coreCodec,
            stereoConfigIndex, coreSbrFrameLengthIndex);
        nInputChannels = spatialSpecificConfig.nInputChannels;
        pSsc = &spatialSpecificConfig;
      } else {
        err = SpatialDecParseMps212Config(
            hBs, &pMpegSurroundDecoder->spatialSpecificConfigBackup,
            samplingRate, coreCodec, stereoConfigIndex,
            coreSbrFrameLengthIndex);
        nInputChannels =
            pMpegSurroundDecoder->spatialSpecificConfigBackup.nInputChannels;
      }
      if ((err == MPS_OK) && (numChannels != nInputChannels)) {
        err = MPS_PARSE_ERROR;
      }
      break;
    case AOT_ER_AAC_ELD:
    case AOT_ER_AAC_LD:
      if (configMode == AC_CM_DET_CFG_CHANGE) {
        /* In config detection mode write spatial specific config parameters
         * into temporarily allocated structure */
        err = SpatialDecParseSpecificConfig(hBs, &spatialSpecificConfig,
                                            configBytes, coreCodec);
        nInputChannels = spatialSpecificConfig.nInputChannels;
        pSsc = &spatialSpecificConfig;
      } else {
        err = SpatialDecParseSpecificConfig(
            hBs, &pMpegSurroundDecoder->spatialSpecificConfigBackup,
            configBytes, coreCodec);
        nInputChannels =
            pMpegSurroundDecoder->spatialSpecificConfigBackup.nInputChannels;
      }
      /* check number of channels for channel_configuration > 0  */
      if ((err == MPS_OK) && (numChannels > 0) &&
          (numChannels != nInputChannels)) {
        err = MPS_PARSE_ERROR;
      }
      break;
    default:
      err = MPS_UNSUPPORTED_FORMAT;
      break;
  }

  if (err != MPS_OK) {
    goto bail;
  }

  err = sscCheckOutOfBand(pSsc, coreCodec, samplingRate, frameSize);

  if (err != MPS_OK) {
    goto bail;
  }

  if (configMode & AC_CM_DET_CFG_CHANGE) {
    return err;
  }

  if (configMode & AC_CM_ALLOC_MEM) {
    if (*configChanged) {
      err = mpegSurroundDecoder_Open(&pMpegSurroundDecoder, stereoConfigIndex,
                                     NULL);
      if (err) {
        return err;
      }
    }
  }

  {
    SPATIAL_SPECIFIC_CONFIG *sscParse =
        &pMpegSurroundDecoder
             ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameParse];

    if (FDK_SpatialDecCompareSpatialSpecificConfigHeader(
            &pMpegSurroundDecoder->spatialSpecificConfigBackup, sscParse)) {
      pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameParse] |=
          MPEGS_INIT_CHANGE_HEADER;
      /* Error resilience code */
      if (pMpegSurroundDecoder->pSpatialDec == NULL) {
        err = MPS_NOTOK;
        goto bail;
      }
      SpatialDecInitParserContext(pMpegSurroundDecoder->pSpatialDec);
      pMpegSurroundDecoder->pSpatialDec->pConfigCurrent =
          &pMpegSurroundDecoder
               ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameDecode];
    }
  }

  if (err == MPS_OK) {
    /* We got a valid out-of-band configuration so label it accordingly. */
    pMpegSurroundDecoder->mpegSurroundSscIsGlobalCfg = 1;
  }

bail:
  return err;
}

/**
 * \brief Determine MPEG Surround operation mode.
 **/
static MPEGS_OPMODE mpegSurroundOperationMode(
    CMpegSurroundDecoder *pMpegSurroundDecoder, int mpsDataBits) {
  MPEGS_OPMODE mode;

  {
    if ((mpsDataBits > 0) &&
        (pMpegSurroundDecoder->mpegSurroundUserParams.blindEnable == 0)) {
      mode = MPEGS_OPMODE_MPS_PAYLOAD; /* Mode: Normal, Stereo or Binaural */
    } else {
      mode = MPEGS_OPMODE_NO_MPS_PAYLOAD; /* Mode: No MPEG Surround Payload */
      updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                      MPEGS_INIT_ERROR_PAYLOAD, MPEGS_SYNC_LOST,
                                      MPEGS_STOP);
    }
  }

  return (mode);
}

/**
 * \brief  Check ssc for parse errors.
 *         This one is called in initMpegSurroundDecoder()
 *         to ensure checking of inband and out-of-band mps configs.
 *         Only parse errors checked here! Check for valid config is done
 *         in check_UParam_Build_DecConfig()!
 *
 * \param pSsc         spatial specific config handle.
 *
 * \return  MPS_OK on sucess, and else on parse error.
 */
static SACDEC_ERROR sscParseCheck(const SPATIAL_SPECIFIC_CONFIG *pSsc) {
  if (pSsc->samplingFreq > 96000) return MPS_PARSE_ERROR;
  if (pSsc->samplingFreq < 8000) return MPS_PARSE_ERROR;

  if ((pSsc->treeConfig < 0) || (pSsc->treeConfig > 7)) {
    return MPS_PARSE_ERROR;
  }

  if ((pSsc->quantMode < 0) || (pSsc->quantMode > 2)) {
    return MPS_PARSE_ERROR;
  }

  /* now we are sure there were no parsing errors */

  return MPS_OK;
}

/**
 * \brief  Check number of time slots
 *
 * Basically the mps frame length must be a multiple of the core coder frame
 * length. The below table shows all valid configurations in detail. See ISO/IEC
 * 23003-1: "Table 4A - Allowed values for bsFrameLength in the Baseline MPEG
 * Surround Profile"
 *
 * Downmix Coder        Downmix Code      Allowed values for bsFrameLength
 * Allowed frame sizes for normal, downsampled and upsampled MPS Framelength
 *                      (QMF Samples)
 *
 * AAC 1024                  16           15, 31, 47, 63 1024  2048  3072  4096
 * downsampled MPS           32           31, 63 1024  2048 upsampled MPS
 * 8            7, 15, 23, 31, 39, 47, 55, 63, 71    1024  2048  3072  4096
 * 5120  6144  7168  8192  9216
 *
 * AAC 960                   15           14, 29, 44, 59 960  1920  2880  3840
 * downsampled MPS           30           29, 59 960  1920 upsampled MPS
 * 7,5           14, 29, 44, 59                        1920  3840  5760  7680
 *
 * HE-AAC 1024/2048          32           31, 63 2048  4096 downsampled MPS
 * 64           63                                    2048 upsampled MPS
 * 16           15, 31, 47, 63                        2048  4096  6144  8192
 *
 * HE-AAC 960/1920           30           29, 59 1920  3840 downsampled MPS
 * 60           59                                    1920 upsampled MPS
 * 15           14, 29, 44, 59                        1920  3840  5760  7680
 *
 * BSAC                      16           15, 31, 47, 63 1024  2048  3072  4096
 * downsampled MPS           32           31, 63 1024  2048 upsampled MPS
 * 8            7, 15, 23, 31, 39, 47, 55, 63, 71    1024  2048  3072  4096
 * 5120  6144  7168  8192  9216
 *
 * BSAC with SBR             32           31, 63 2048  4096 downsampled MPS
 * 64           63                                    2048 upsampled MPS
 * 16           15, 31, 47, 63                        2048  4096  6144  8192
 *
 * AAC LD 512                 8            7, 15, 23, 31, 39, 47, 55, 63, 71
 * 512  1024  1536  2048  2560  3072  3584  4096  4608 downsampled MPS
 * 16           15, 31, 47, 63                         512  1024  1536  2048
 *
 * AAC ELD 512                8            7, 15, 23, 31, 39, 47, 55, 63, 71
 * 512  1024  1536  2048  2560  3072  3584  4096  4608 downsampled MPS
 * 16           15, 31, 47, 63                         512  1024  1536  2048
 *
 * AAC ELD with SBR 512/1024 16           15, 31, 47, 63 1024  2048  3072  4096
 * downsampled MPS           32           31, 63 1024  2048 upsampled MPS
 * 8            7, 15, 23, 31, 39, 47, 55, 63, 71    1024  2048  3072  4096
 * 5120  6144  7168  8192  9216
 *
 * MPEG1/2 Layer II          18           17, 35, 53, 71 1152  2304  3456  4608
 * downsampled MPS           36           35, 71 1152  2304
 *
 * MPEG1/2 Layer III         18           17, 35, 53, 71 1152  2304  3456  4608
 * downsampled MPS           36           35, 71 1152  2304
 *
 * \param frameLength
 * \param qmfBands
 * \param timeSlots
 *
 * \return  error code
 */
SACDEC_ERROR checkTimeSlots(int frameLength, int qmfBands, int timeSlots) {
  int len;
  int maxFrameLength;

  if (qmfBands == 64) {
    /* normal MPEG Surround */
    switch (frameLength) {
      case 960:
      case 1920:
        maxFrameLength = 3840;
        break;
      case 1024:
      case 2048:
        maxFrameLength = 4096;
        break;
      case 512:
      case 1152:
        maxFrameLength = 4608;
        break;
      default:
        return MPS_PARSE_ERROR;
    }
  } else if (qmfBands == 32) {
    /* downsampled MPEG Surround */
    switch (frameLength) {
      case 960:
      case 1920:
        maxFrameLength = 1920;
        break;
      case 512:
      case 1024:
      case 2048:
        maxFrameLength = 2048;
        break;
      case 1152:
        maxFrameLength = 2304;
        break;
      default:
        return MPS_PARSE_ERROR;
    }
  } else if (qmfBands == 128) {
    /* upsampled MPEG Surround */
    switch (frameLength) {
      case 1920:
        maxFrameLength = 7680;
        break;
      case 1024:
        maxFrameLength = 9216;
        break;
      case 2048:
        maxFrameLength = 8192;
        break;
      case 512:
      case 960:
      case 1152:
      /* no break, no support for upsampled MPEG Surround */
      default:
        return MPS_PARSE_ERROR;
    }
  } else {
    return MPS_PARSE_ERROR;
  }

  len = frameLength;

  while (len <= maxFrameLength) {
    if (len == timeSlots * qmfBands) {
      return MPS_OK;
    }
    len += frameLength;
  }
  return MPS_PARSE_ERROR;
}

/**
 * \brief  Check ssc for consistency (e.g. bit errors could cause trouble)
 *         First of currently two ssc-checks.
 *         This (old) one is called in mpegSurroundDecoder_Apply()
 *         only if inband mps config is contained in stream.
 *
 *         New ssc check is split in two functions sscParseCheck() and
 * check_UParam_Build_DecConfig(). sscParseCheck() checks only for correct
 * parsing. check_UParam_Build_DecConfig() is used to check if we have a
 * valid config. Both are called in initMpegSurroundDecoder() to ensure
 * checking of inband and out-of-band mps configs.
 *
 *         If this function can be integrated into the new functions.
 *         We can remove this one.
 *
 * \param pSsc         spatial specific config handle.
 * \param frameLength
 * \param sampleRate
 *
 * \return  MPS_OK on sucess, and else on failure.
 */
static SACDEC_ERROR sscCheckInBand(SPATIAL_SPECIFIC_CONFIG *pSsc,
                                   int frameLength, int sampleRate) {
  SACDEC_ERROR err = MPS_OK;
  int qmfBands;

  FDK_ASSERT(pSsc != NULL);

  /* check ssc for parse errors */
  if (sscParseCheck(pSsc) != MPS_OK) {
    err = MPS_PARSE_ERROR;
  }

  /* core fs and mps fs must match */
  if (pSsc->samplingFreq != sampleRate) {
    err = MPS_PARSE_ERROR /* MPEGSDEC_SSC_PARSE_ERROR */;
  }

  qmfBands = mpegSurroundDecoder_GetNrOfQmfBands(pSsc, pSsc->samplingFreq);

  if (checkTimeSlots(frameLength, qmfBands, pSsc->nTimeSlots) != MPS_OK) {
    err = MPS_PARSE_ERROR;
  }

  return err;
}

SACDEC_ERROR
mpegSurroundDecoder_ConfigureQmfDomain(
    CMpegSurroundDecoder *pMpegSurroundDecoder,
    SAC_INPUT_CONFIG sac_dec_interface, UINT coreSamplingRate,
    AUDIO_OBJECT_TYPE coreCodec) {
  SACDEC_ERROR err = MPS_OK;
  FDK_QMF_DOMAIN_GC *pGC = NULL;

  if (pMpegSurroundDecoder == NULL) {
    return MPS_INVALID_HANDLE;
  }

  FDK_ASSERT(pMpegSurroundDecoder->pSpatialDec);

  pGC = &pMpegSurroundDecoder->pQmfDomain->globalConf;
  if (pMpegSurroundDecoder->mpegSurroundSscIsGlobalCfg) {
    SPATIAL_SPECIFIC_CONFIG *pSSC =
        &pMpegSurroundDecoder->spatialSpecificConfigBackup;
    if (sac_dec_interface == SAC_INTERFACE_TIME) {
      /* For SAC_INTERFACE_QMF these parameters are set by SBR. */
      pGC->nBandsAnalysis_requested = mpegSurroundDecoder_GetNrOfQmfBands(
          pSSC, coreSamplingRate); /* coreSamplingRate == outputSamplingRate for
                                      SAC_INTERFACE_TIME */
      pGC->nBandsSynthesis_requested = pGC->nBandsAnalysis_requested;
      pGC->nInputChannels_requested =
          fMax((UINT)pSSC->nInputChannels, (UINT)pGC->nInputChannels_requested);
    }
    pGC->nOutputChannels_requested =
        fMax((UINT)pSSC->nOutputChannels, (UINT)pGC->nOutputChannels_requested);
  } else {
    if (sac_dec_interface == SAC_INTERFACE_TIME) {
      /* For SAC_INTERFACE_QMF these parameters are set by SBR. */
      pGC->nBandsAnalysis_requested = mpegSurroundDecoder_GetNrOfQmfBands(
          NULL, coreSamplingRate); /* coreSamplingRate == outputSamplingRate for
                                      SAC_INTERFACE_TIME */
      pGC->nBandsSynthesis_requested = pGC->nBandsAnalysis_requested;
      pGC->nInputChannels_requested =
          pMpegSurroundDecoder->pSpatialDec->createParams.maxNumInputChannels;
    }
    pGC->nOutputChannels_requested =
        pMpegSurroundDecoder->pSpatialDec->createParams.maxNumOutputChannels;
  }
  pGC->nQmfProcBands_requested = 64;
  pGC->nQmfProcChannels_requested =
      fMin((INT)pGC->nInputChannels_requested,
           pMpegSurroundDecoder->pSpatialDec->createParams.maxNumInputChannels);

  if (coreCodec == AOT_ER_AAC_ELD) {
    pGC->flags_requested |= QMF_FLAG_MPSLDFB;
    pGC->flags_requested &= ~QMF_FLAG_CLDFB;
  }

  return err;
}

/**
 * \brief  Check out-of-band config
 *
 * \param pSsc         spatial specific config handle.
 * \param coreCodec    core codec.
 * \param sampleRate   sampling frequency.
 *
 * \return  errorStatus
 */
SACDEC_ERROR
sscCheckOutOfBand(const SPATIAL_SPECIFIC_CONFIG *pSsc, const INT coreCodec,
                  const INT sampleRate, const INT frameSize) {
  FDK_ASSERT(pSsc != NULL);
  int qmfBands = 0;

  /* check ssc for parse errors */
  if (sscParseCheck(pSsc) != MPS_OK) {
    return MPS_PARSE_ERROR;
  }

  switch (coreCodec) {
    case AOT_USAC:
    case AOT_DRM_USAC:
      /* ISO/IEC 23003-1:2007(E), Chapter 6.3.3, Support for lower and higher
       * sampling frequencies */
      if (pSsc->samplingFreq >= 55426) {
        return MPS_PARSE_ERROR;
      }
      break;
    case AOT_ER_AAC_LD:
    case AOT_ER_AAC_ELD:
      /* core fs and mps fs must match */
      if (pSsc->samplingFreq != sampleRate) {
        return MPS_PARSE_ERROR;
      }

      /* ISO/IEC 14496-3:2009 FDAM 3: Chapter 1.5.2.3, Levels for the Low Delay
       * AAC v2 profile */
      if (pSsc->samplingFreq > 48000) {
        return MPS_PARSE_ERROR;
      }

      qmfBands = mpegSurroundDecoder_GetNrOfQmfBands(pSsc, pSsc->samplingFreq);
      switch (frameSize) {
        case 480:
          if (!((qmfBands == 32) && (pSsc->nTimeSlots == 15))) {
            return MPS_PARSE_ERROR;
          }
          break;
        case 960:
          if (!((qmfBands == 64) && (pSsc->nTimeSlots == 15))) {
            return MPS_PARSE_ERROR;
          }
          break;
        case 512:
          if (!(((qmfBands == 32) && (pSsc->nTimeSlots == 16)) ||
                ((qmfBands == 64) && (pSsc->nTimeSlots == 8)))) {
            return MPS_PARSE_ERROR;
          }
          break;
        case 1024:
          if (!((qmfBands == 64) && (pSsc->nTimeSlots == 16))) {
            return MPS_PARSE_ERROR;
          }
          break;
        default:
          return MPS_PARSE_ERROR;
      }
      break;
    default:
      return MPS_PARSE_ERROR;
      break;
  }

  return MPS_OK;
}

/**
 * \brief Decode MPEG Surround frame.
 **/
int mpegSurroundDecoder_ParseNoHeader(
    CMpegSurroundDecoder *pMpegSurroundDecoder, HANDLE_FDK_BITSTREAM hBs,
    int *pMpsDataBits, int fGlobalIndependencyFlag) {
  SACDEC_ERROR err = MPS_OK;
  SPATIAL_SPECIFIC_CONFIG *sscParse;
  int bitsAvail, numSacBits;

  if (pMpegSurroundDecoder == NULL || hBs == NULL) {
    return MPS_INVALID_HANDLE;
  }

  sscParse = &pMpegSurroundDecoder
                  ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameParse];

  bitsAvail = FDKgetValidBits(hBs);

  /* First spatial specific config is parsed into spatialSpecificConfigBackup,
   * second spatialSpecificConfigBackup is copied into
   * spatialSpecificConfig[bsFrameDecode] */
  if (pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameParse]) {
    FDKmemcpy(sscParse, &pMpegSurroundDecoder->spatialSpecificConfigBackup,
              sizeof(SPATIAL_SPECIFIC_CONFIG));
    pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameParse] =
        MPEGS_SYNC_FOUND;
  }

  if (bitsAvail <= 0) {
    err = MPS_PARSE_ERROR;
  } else {
    err = SpatialDecParseFrameData(
        pMpegSurroundDecoder->pSpatialDec,
        &pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameParse],
        hBs, sscParse, (UPMIXTYPE)pMpegSurroundDecoder->upmixType,
        fGlobalIndependencyFlag);
    if (err == MPS_OK) {
      pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameParse]
          .newBsData = 1;
    }
  }

  numSacBits = bitsAvail - (INT)FDKgetValidBits(hBs);

  if (numSacBits > bitsAvail) {
    pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameParse]
        .newBsData = 0;
    err = MPS_PARSE_ERROR;
  }

  *pMpsDataBits -= numSacBits;

  return err;
}

/**
 * \brief Check, if ancType is valid.
 **/
static int isValidAncType(CMpegSurroundDecoder *pMpegSurroundDecoder,
                          int ancType) {
  int ret = 1;

  if ((ancType != MPEGS_ANCTYPE_HEADER_AND_FRAME) &&
      (ancType != MPEGS_ANCTYPE_FRAME)) {
    ret = 0;
  }

  if (ret == 0) {
    updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                    MPEGS_INIT_ERROR_PAYLOAD, MPEGS_SYNC_LOST,
                                    MPEGS_STOP);
  }

  return (ret);
}

/**
 * \brief Check, if ancStartStop is valid.
 **/
static int isValidAncStartStop(CMpegSurroundDecoder *pMpegSurroundDecoder,
                               int ancStartStop) {
  int ret = 1;

  switch (ancStartStop) {
    case MPEGS_START:
      /* Sequence start - start and continue - start not allowed */
      if ((pMpegSurroundDecoder->ancStartStopPrev == MPEGS_START) ||
          (pMpegSurroundDecoder->ancStartStopPrev == MPEGS_CONTINUE)) {
        ret = 0;
      }
      break;

    case MPEGS_STOP:
      /* MPS payload of the previous frame must be valid if current type is stop
         Sequence startstop - stop and stop - stop not allowed
         Sequence startstop - continue and stop - continue are allowed */
      if ((pMpegSurroundDecoder->ancStartStopPrev == MPEGS_STOP) ||
          (pMpegSurroundDecoder->ancStartStopPrev == MPEGS_START_STOP)) {
        ret = 0;
      }
      break;

    case MPEGS_CONTINUE:
    case MPEGS_START_STOP:
      /* No error detection possible for this states */
      break;
  }

  if (ret == 0) {
    updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                    MPEGS_INIT_ERROR_PAYLOAD, MPEGS_SYNC_LOST,
                                    MPEGS_STOP);
  } else {
    pMpegSurroundDecoder->ancStartStopPrev = (MPEGS_ANCSTARTSTOP)ancStartStop;
  }

  return (ret);
}

int mpegSurroundDecoder_Parse(CMpegSurroundDecoder *pMpegSurroundDecoder,
                              HANDLE_FDK_BITSTREAM hBs, int *pMpsDataBits,
                              AUDIO_OBJECT_TYPE coreCodec, int sampleRate,
                              int frameSize, int fGlobalIndependencyFlag) {
  SACDEC_ERROR err = MPS_OK;
  SPATIAL_SPECIFIC_CONFIG *sscParse;
  SPATIAL_BS_FRAME *bsFrame;
  HANDLE_FDK_BITSTREAM hMpsBsData = NULL;
  FDK_BITSTREAM mpsBsData;
  int mpsDataBits = *pMpsDataBits;
  int mpsBsBits;
  MPEGS_ANCTYPE ancType;
  MPEGS_ANCSTARTSTOP ancStartStop;

  if (pMpegSurroundDecoder == NULL) {
    return MPS_INVALID_HANDLE;
  }

  FDK_ASSERT(pMpegSurroundDecoder->pSpatialDec);

  mpsBsBits = (INT)FDKgetValidBits(hBs);

  sscParse = &pMpegSurroundDecoder
                  ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameParse];
  bsFrame = &pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameParse];

  /*
     Find operation mode of mpeg surround decoder:
     - MPEGS_OPMODE_EMM:            Mode: Enhanced Matrix Mode (Blind)
     - MPEGS_OPMODE_MPS_PAYLOAD:    Mode: Normal, Stereo or Binaural
     - MPEGS_OPMODE_NO_MPS_PAYLOAD: Mode: No MpegSurround Payload
  */
  {
    /* Parse ancType and ancStartStop */
    ancType = (MPEGS_ANCTYPE)FDKreadBits(hBs, 2);
    ancStartStop = (MPEGS_ANCSTARTSTOP)FDKreadBits(hBs, 2);
    mpsDataBits -= 4;

    /* Set valid anc type flag, if ancType signals a payload with either header
     * and frame or frame */
    if (isValidAncType(pMpegSurroundDecoder, ancType)) {
      /* Set valid anc startstop flag, if transmitted sequence is not illegal */
      if (isValidAncStartStop(pMpegSurroundDecoder, ancStartStop)) {
        switch (ancStartStop) {
          case MPEGS_START:
            /* Assuming that core coder frame size (AAC) is smaller than MPS
               coder frame size. Save audio data for next frame. */
            if (mpsDataBits > MPS_DATA_BUFFER_SIZE * 8) {
              err = MPS_NOTOK;
              goto bail;
            }
            for (int i = 0; i < mpsDataBits / 8; i++) {
              pMpegSurroundDecoder->mpsData[i] = FDKreadBits(hBs, 8);
            }
            pMpegSurroundDecoder->mpsDataBits = mpsDataBits;
            break;

          case MPEGS_CONTINUE:
          case MPEGS_STOP:
            /* Assuming that core coder frame size (AAC) is smaller than MPS
               coder frame size. Save audio data for next frame. */
            if ((pMpegSurroundDecoder->mpsDataBits + mpsDataBits) >
                MPS_DATA_BUFFER_SIZE * 8) {
              err = MPS_NOTOK;
              goto bail;
            }
            for (int i = 0; i < mpsDataBits / 8; i++) {
              pMpegSurroundDecoder
                  ->mpsData[(pMpegSurroundDecoder->mpsDataBits / 8) + i] =
                  FDKreadBits(hBs, 8);
            }
            pMpegSurroundDecoder->mpsDataBits += mpsDataBits;
            FDKinitBitStream(&mpsBsData, pMpegSurroundDecoder->mpsData,
                             MAX_BUFSIZE_BYTES,
                             pMpegSurroundDecoder->mpsDataBits, BS_READER);
            hMpsBsData = &mpsBsData;
            break;

          case MPEGS_START_STOP:
            pMpegSurroundDecoder->mpsDataBits = mpsDataBits;
            hMpsBsData = hBs;
            break;

          default:
            FDK_ASSERT(0);
        }

        if ((ancStartStop == MPEGS_STOP) ||
            (ancStartStop == MPEGS_START_STOP)) {
          switch (ancType) {
            case MPEGS_ANCTYPE_HEADER_AND_FRAME: {
              int parseResult, bitsRead;
              SPATIAL_SPECIFIC_CONFIG spatialSpecificConfigTmp =
                  pMpegSurroundDecoder->spatialSpecificConfigBackup;

              /* Parse spatial specific config */
              bitsRead = (INT)FDKgetValidBits(hMpsBsData);

              err = SpatialDecParseSpecificConfigHeader(
                  hMpsBsData,
                  &pMpegSurroundDecoder->spatialSpecificConfigBackup, coreCodec,
                  pMpegSurroundDecoder->upmixType);

              bitsRead = (bitsRead - (INT)FDKgetValidBits(hMpsBsData));
              parseResult = ((err == MPS_OK) ? bitsRead : -bitsRead);

              if (parseResult < 0) {
                parseResult = -parseResult;
                err = MPS_PARSE_ERROR;
              } else if (err == MPS_OK) {
                /* Check SSC for consistency (e.g. bit errors could cause
                 * trouble) */
                err = sscCheckInBand(
                    &pMpegSurroundDecoder->spatialSpecificConfigBackup,
                    frameSize, sampleRate);
              }
              if (err != MPS_OK) {
                pMpegSurroundDecoder->spatialSpecificConfigBackup =
                    spatialSpecificConfigTmp;
                break;
              }

              pMpegSurroundDecoder->mpsDataBits -= parseResult;

              /* Initiate re-initialization, if header has changed */
              if (FDK_SpatialDecCompareSpatialSpecificConfigHeader(
                      &pMpegSurroundDecoder->spatialSpecificConfigBackup,
                      sscParse) == MPS_UNEQUAL_SSC) {
                pMpegSurroundDecoder
                    ->initFlags[pMpegSurroundDecoder->bsFrameParse] |=
                    MPEGS_INIT_CHANGE_HEADER;
                SpatialDecInitParserContext(pMpegSurroundDecoder->pSpatialDec);
                /* We found a valid in-band configuration. Therefore any
                 * previous config is invalid now. */
                pMpegSurroundDecoder->mpegSurroundSscIsGlobalCfg = 0;
              }
            }
              FDK_FALLTHROUGH;
            case MPEGS_ANCTYPE_FRAME:

              if (pMpegSurroundDecoder
                      ->initFlags[pMpegSurroundDecoder->bsFrameParse] &
                  MPEGS_INIT_ERROR_PAYLOAD) {
                err = MPS_PARSE_ERROR;
                break;
              }

              /* First spatial specific config is parsed into
               * spatialSpecificConfigBackup, second spatialSpecificConfigBackup
               * is copied into spatialSpecificConfig[bsFrameDecode] */
              if (pMpegSurroundDecoder
                      ->initFlags[pMpegSurroundDecoder->bsFrameParse]) {
                FDKmemcpy(sscParse,
                          &pMpegSurroundDecoder->spatialSpecificConfigBackup,
                          sizeof(SPATIAL_SPECIFIC_CONFIG));
                pMpegSurroundDecoder
                    ->fOnSync[pMpegSurroundDecoder->bsFrameParse] =
                    MPEGS_SYNC_FOUND;
              }

              if (pMpegSurroundDecoder
                      ->fOnSync[pMpegSurroundDecoder->bsFrameParse] >=
                  MPEGS_SYNC_FOUND) {
                int nbits = 0, bitsAvail;

                if (err != MPS_OK) {
                  break;
                }

                bitsAvail = FDKgetValidBits(hMpsBsData);

                if (bitsAvail <= 0) {
                  err = MPS_PARSE_ERROR;
                } else {
                  err = SpatialDecParseFrameData(
                      pMpegSurroundDecoder->pSpatialDec, bsFrame, hMpsBsData,
                      sscParse, (UPMIXTYPE)pMpegSurroundDecoder->upmixType,
                      fGlobalIndependencyFlag);
                  if (err == MPS_OK) {
                    bsFrame->newBsData = 1;
                  }
                }

                nbits = bitsAvail - (INT)FDKgetValidBits(hMpsBsData);

                if ((nbits > bitsAvail) ||
                    (nbits > pMpegSurroundDecoder->mpsDataBits) ||
                    (pMpegSurroundDecoder->mpsDataBits > nbits + 7 &&
                     !IS_LOWDELAY(coreCodec))) {
                  bsFrame->newBsData = 0;
                  err = MPS_PARSE_ERROR;
                  break;
                }
                pMpegSurroundDecoder->mpsDataBits -= nbits;
              }
              break;

            default: /* added to avoid compiler warning */
              err = MPS_NOTOK;
              break; /* added to avoid compiler warning */
          }          /* switch (ancType) */

          if (err == MPS_OK) {
            pMpegSurroundDecoder->ancStartStopPrev = ancStartStop;
          } else {
            updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                            MPEGS_INIT_ERROR_PAYLOAD,
                                            MPEGS_SYNC_LOST, MPEGS_STOP);
            pMpegSurroundDecoder->mpsDataBits = 0;
          }
        } /* (ancStartStop == MPEGS_STOP) || (ancStartStop == MPEGS_START_STOP)
           */
      }   /* validAncStartStop */
    }     /* validAncType */
  }

bail:

  *pMpsDataBits -= (mpsBsBits - (INT)FDKgetValidBits(hBs));

  return err;
}

int mpegSurroundDecoder_Apply(CMpegSurroundDecoder *pMpegSurroundDecoder,
                              PCM_MPS *input, PCM_MPS *pTimeData,
                              const int timeDataSize, int timeDataFrameSize,
                              int *nChannels, int *frameSize, int sampleRate,
                              AUDIO_OBJECT_TYPE coreCodec,
                              AUDIO_CHANNEL_TYPE channelType[],
                              UCHAR channelIndices[],
                              const FDK_channelMapDescr *const mapDescr,
                              const INT inDataHeadroom, INT *outDataHeadroom) {
  SACDEC_ERROR err = MPS_OK;
  PCM_MPS *pTimeOut = pTimeData;
  PCM_MPS *TDinput = NULL;
  UINT initControlFlags = 0, controlFlags = 0;
  int timeDataRequiredSize = 0;
  int newData;

  if (pMpegSurroundDecoder == NULL) {
    return MPS_INVALID_HANDLE;
  }

  FDK_ASSERT(pMpegSurroundDecoder->pSpatialDec);

  if (!FDK_chMapDescr_isValid(mapDescr)) {
    return MPS_INVALID_HANDLE;
  }

  if ((*nChannels <= 0) || (*nChannels > 2)) {
    return MPS_NOTOK;
  }

  pMpegSurroundDecoder->pSpatialDec->sacInDataHeadroom = inDataHeadroom;
  *outDataHeadroom = (INT)(8);

  pMpegSurroundDecoder->pSpatialDec->pConfigCurrent =
      &pMpegSurroundDecoder
           ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameDecode];
  newData = pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameParse]
                .newBsData;

  switch (mpegSurroundOperationMode(pMpegSurroundDecoder, 1000)) {
    case MPEGS_OPMODE_MPS_PAYLOAD:
      if (pMpegSurroundDecoder
              ->initFlags[pMpegSurroundDecoder->bsFrameDecode]) {
        err = initMpegSurroundDecoder(pMpegSurroundDecoder);
      }

      if (err == MPS_OK) {
        if ((pMpegSurroundDecoder
                 ->fOnSync[pMpegSurroundDecoder->bsFrameDecode] !=
             MPEGS_SYNC_COMPLETE) &&
            (pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameDecode]
                 .bsIndependencyFlag == 1)) {
          /* We got a valid header and independently decodeable frame data.
              -> Go to the next sync level and start processing. */
          pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
              MPEGS_SYNC_COMPLETE;
        }
      } else {
        /* We got a valid config header but found an error while parsing the
           bitstream. Wait for the next independent frame and apply error
           conealment in the meantime. */
        pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
            MPEGS_SYNC_FOUND;
        controlFlags |= MPEGS_CONCEAL;
        err = MPS_OK;
      }
      /*
         Concealment:
         - Bitstream is available, no sync found during bitstream processing
         - Bitstream is available, sync lost due to corrupted bitstream
         - Bitstream is available, sync found but no independent frame
      */
      if (pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] !=
          MPEGS_SYNC_COMPLETE) {
        controlFlags |= MPEGS_CONCEAL;
      }
      break;

    case MPEGS_OPMODE_NO_MPS_PAYLOAD:
      /* Concealment: No bitstream is available */
      controlFlags |= MPEGS_CONCEAL;
      break;

    default:
      err = MPS_NOTOK;
  }

  if (err != MPS_OK) {
    goto bail;
  }

  /*
   * Force BypassMode if choosen by user
   */
  if (pMpegSurroundDecoder->mpegSurroundUserParams.bypassMode) {
    controlFlags |= MPEGS_BYPASSMODE;
  }

  if (pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode]) {
    int startWithDfltCfg = 0;
    /*
     * Init with a default configuration if we came here and are still not
     * initialized.
     */
    if (pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] &
        MPEGS_INIT_ENFORCE_REINIT) {
      /* Get default spatial specific config */
      if (FDK_SpatialDecInitDefaultSpatialSpecificConfig(
              &pMpegSurroundDecoder->spatialSpecificConfigBackup, coreCodec,
              *nChannels, sampleRate,
              *frameSize /
                  mpegSurroundDecoder_GetNrOfQmfBands(NULL, sampleRate),
              pMpegSurroundDecoder->mpegSurroundDecoderLevel,
              pMpegSurroundDecoder->mpegSurroundUserParams.blindEnable)) {
        err = MPS_NOTOK;
        goto bail;
      }

      /* Initiate re-initialization, if header has changed */
      if (FDK_SpatialDecCompareSpatialSpecificConfigHeader(
              &pMpegSurroundDecoder->spatialSpecificConfigBackup,
              &pMpegSurroundDecoder->spatialSpecificConfig
                   [pMpegSurroundDecoder->bsFrameDecode]) == MPS_UNEQUAL_SSC) {
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_HEADER;
        SpatialDecInitParserContext(pMpegSurroundDecoder->pSpatialDec);
      }

      startWithDfltCfg = 1;
    }

    /* First spatial specific config is parsed into spatialSpecificConfigBackup,
     * second spatialSpecificConfigBackup is copied into spatialSpecificConfig
     */
    err = initMpegSurroundDecoder(pMpegSurroundDecoder);

    if (startWithDfltCfg) {
      /* initialized with default config, but no sync found */
      /* maybe use updateMpegSurroundDecoderStatus later on */
      pMpegSurroundDecoder->fOnSync[pMpegSurroundDecoder->bsFrameDecode] =
          MPEGS_SYNC_LOST;
    }

    /* Since we do not have state MPEGS_SYNC_COMPLETE apply concealment */
    controlFlags |= MPEGS_CONCEAL;

    if (err != MPS_OK) {
      goto bail;
    }
  }

  /*
   * Process MPEG Surround Audio
   */
  initControlFlags = controlFlags;

  /* Check that provided output buffer is large enough. */
  if (pMpegSurroundDecoder->pQmfDomain->globalConf.nBandsAnalysis == 0) {
    err = MPS_UNSUPPORTED_FORMAT;
    goto bail;
  }
  timeDataRequiredSize =
      (timeDataFrameSize *
       pMpegSurroundDecoder->pSpatialDec->numOutputChannelsAT *
       pMpegSurroundDecoder->pQmfDomain->globalConf.nBandsSynthesis) /
      pMpegSurroundDecoder->pQmfDomain->globalConf.nBandsAnalysis;
  if (timeDataSize < timeDataRequiredSize) {
    err = MPS_OUTPUT_BUFFER_TOO_SMALL;
    goto bail;
  }

  if ((pMpegSurroundDecoder->pSpatialDec->pConfigCurrent->syntaxFlags &
       SACDEC_SYNTAX_USAC) &&
      (pMpegSurroundDecoder->pSpatialDec->stereoConfigIndex > 1)) {
    FDK_ASSERT(timeDataRequiredSize >= timeDataFrameSize * *nChannels);
    /* Place samples comprising QMF time slots spaced at QMF output Band raster
     * to allow slot wise processing */
    int timeDataFrameSizeOut =
        (timeDataFrameSize *
         pMpegSurroundDecoder->pQmfDomain->globalConf.nBandsSynthesis) /
        pMpegSurroundDecoder->pQmfDomain->globalConf.nBandsAnalysis;
    TDinput = pTimeData + timeDataFrameSizeOut - timeDataFrameSize;
    for (int i = *nChannels - 1; i >= 0; i--) {
      FDKmemmove(pTimeData + (i + 1) * timeDataFrameSizeOut - timeDataFrameSize,
                 pTimeData + timeDataFrameSize * i,
                 sizeof(PCM_MPS) * timeDataFrameSize);
      FDKmemclear(pTimeData + i * timeDataFrameSizeOut,
                  sizeof(PCM_MPS) * (timeDataFrameSizeOut - timeDataFrameSize));
    }
  } else {
    if (pMpegSurroundDecoder->mpegSurroundUseTimeInterface) {
      FDKmemcpy(input, pTimeData,
                sizeof(PCM_MPS) * (*nChannels) * (*frameSize));
      TDinput = input;
    }
  }

  /*
   * Process MPEG Surround Audio
   */
  err = SpatialDecApplyFrame(
      pMpegSurroundDecoder->pSpatialDec,
      &pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameDecode],
      pMpegSurroundDecoder->mpegSurroundUseTimeInterface ? INPUTMODE_TIME
                                                         : INPUTMODE_QMF_SBR,
      TDinput, NULL, NULL, pTimeOut, *frameSize, &controlFlags, *nChannels,
      mapDescr);
  *nChannels = pMpegSurroundDecoder->pSpatialDec->numOutputChannelsAT;

  if (err !=
      MPS_OK) { /* A fatal error occured. Go back to start and try again: */
    updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                    MPEGS_INIT_ENFORCE_REINIT, MPEGS_SYNC_LOST,
                                    MPEGS_STOP);
    *frameSize =
        0; /* Declare that framework can not use the data in pTimeOut. */
  } else {
    if (((controlFlags & MPEGS_CONCEAL) &&
         !(initControlFlags & MPEGS_CONCEAL)) ||
        (pMpegSurroundDecoder->pSpatialDec->errInt !=
         MPS_OK)) { /* Account for errors that occured in
                       SpatialDecApplyFrame(): */
      updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                      MPEGS_INIT_ERROR_PAYLOAD, MPEGS_SYNC_LOST,
                                      MPEGS_STOP);
    }
  }

  if ((err == MPS_OK) && !(controlFlags & MPEGS_BYPASSMODE) &&
      !(pMpegSurroundDecoder->upmixType == UPMIX_TYPE_BYPASS)) {
    SpatialDecChannelProperties(pMpegSurroundDecoder->pSpatialDec, channelType,
                                channelIndices, mapDescr);
  }

bail:

  if (newData) {
    /* numParameterSetsPrev shall only be read in the decode process, because of
       that we can update this state variable here */
    pMpegSurroundDecoder->pSpatialDec->numParameterSetsPrev =
        pMpegSurroundDecoder->bsFrames[pMpegSurroundDecoder->bsFrameDecode]
            .numParameterSets;
  }

  return (err);
}

/**
 * \brief Free config dependent MPEG Surround memory.
 **/
SACDEC_ERROR mpegSurroundDecoder_FreeMem(
    CMpegSurroundDecoder *pMpegSurroundDecoder) {
  SACDEC_ERROR err = MPS_OK;

  if (pMpegSurroundDecoder != NULL) {
    FDK_SpatialDecClose(pMpegSurroundDecoder->pSpatialDec);
    pMpegSurroundDecoder->pSpatialDec = NULL;
  }

  return err;
}

/**
 * \brief Close MPEG Surround decoder.
 **/
void mpegSurroundDecoder_Close(CMpegSurroundDecoder *pMpegSurroundDecoder) {
  if (pMpegSurroundDecoder != NULL) {
    FDK_SpatialDecClose(pMpegSurroundDecoder->pSpatialDec);
    pMpegSurroundDecoder->pSpatialDec = NULL;

    for (int i = 0; i < 1; i++) {
      SpatialDecCloseBsFrame(&pMpegSurroundDecoder->bsFrames[i]);
    }

    FDK_FREE_MEMORY_1D(pMpegSurroundDecoder);
  }
}

#define SACDEC_VL0 2
#define SACDEC_VL1 1
#define SACDEC_VL2 0

int mpegSurroundDecoder_GetLibInfo(LIB_INFO *info) {
  int i;

  if (info == NULL) {
    return -1;
  }

  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) return -1;

  info += i;

  info->module_id = FDK_MPSDEC;
#ifdef SUPPRESS_BUILD_DATE_INFO
  info->build_date = "";
  info->build_time = "";
#else
  info->build_date = __DATE__;
  info->build_time = __TIME__;
#endif
  info->title = "MPEG Surround Decoder";
  info->version = LIB_VERSION(SACDEC_VL0, SACDEC_VL1, SACDEC_VL2);
  LIB_VERSION_STRING(info);
  info->flags = 0 | CAPF_MPS_LD | CAPF_MPS_USAC | CAPF_MPS_HQ |
                CAPF_MPS_1CH_IN | CAPF_MPS_2CH_OUT; /* end flags */

  return 0;
}

SACDEC_ERROR mpegSurroundDecoder_SetParam(
    CMpegSurroundDecoder *pMpegSurroundDecoder, const SACDEC_PARAM param,
    const INT value) {
  SACDEC_ERROR err = MPS_OK;
  SPATIALDEC_PARAM *pUserParams = NULL;

  /* check decoder handle */
  if (pMpegSurroundDecoder != NULL) {
    /* init local shortcuts */
    pUserParams = &pMpegSurroundDecoder->mpegSurroundUserParams;
  } else {
    err = MPS_INVALID_HANDLE;
    /* check the parameter values before exiting. */
  }

  /* apply param value */
  switch (param) {
    case SACDEC_OUTPUT_MODE:
      switch ((SAC_DEC_OUTPUT_MODE)value) {
        case SACDEC_OUT_MODE_NORMAL:
        case SACDEC_OUT_MODE_STEREO:
          break;
        default:
          err = MPS_INVALID_PARAMETER;
      }
      if (err == MPS_OK) {
        if (0) {
          err = MPS_INVALID_PARAMETER;
        } else if (pUserParams->outputMode != (UCHAR)value) {
          pUserParams->outputMode = (UCHAR)value;
          pMpegSurroundDecoder
              ->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
              MPEGS_INIT_CHANGE_OUTPUT_MODE;
        }
      }
      break;

    case SACDEC_INTERFACE:
      if (value < 0 || value > 1) {
        err = MPS_INVALID_PARAMETER;
      }
      if (err != MPS_OK) {
        goto bail;
      }
      if (pMpegSurroundDecoder->mpegSurroundUseTimeInterface != (UCHAR)value) {
        pMpegSurroundDecoder->mpegSurroundUseTimeInterface = (UCHAR)value;
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_TIME_FREQ_INTERFACE;
      }
      break;

    case SACDEC_BS_INTERRUPTION:
      if ((err == MPS_OK) && (value != 0)) {
        updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                        MPEGS_INIT_BS_INTERRUPTION,
                                        MPEGS_SYNC_LOST, MPEGS_STOP);
      }
      break;

    case SACDEC_CLEAR_HISTORY:
      if ((err == MPS_OK) && (value != 0)) {
        /* Just reset the states and go on. */
        updateMpegSurroundDecoderStatus(pMpegSurroundDecoder,
                                        MPEGS_INIT_CLEAR_HISTORY,
                                        MPEGS_SYNC_LOST, MPEGS_STOP);
      }
      break;

    case SACDEC_CONCEAL_NUM_KEEP_FRAMES:
      if (value < 0) { /* Check valid value range */
        err = MPS_INVALID_PARAMETER;
      }
      if (err != MPS_OK) {
        goto bail;
      }
      if (pUserParams->concealNumKeepFrames != (UINT)value) {
        pUserParams->concealNumKeepFrames = (UINT)value;
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_CONCEAL_PARAMS;
      }
      break;

    case SACDEC_CONCEAL_FADE_OUT_SLOPE_LENGTH:
      if (value < 0) { /* Check valid value range */
        err = MPS_INVALID_PARAMETER;
      }
      if (err != MPS_OK) {
        goto bail;
      }
      if (pUserParams->concealFadeOutSlopeLength != (UINT)value) {
        pUserParams->concealFadeOutSlopeLength = (UINT)value;
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_CONCEAL_PARAMS;
      }
      break;

    case SACDEC_CONCEAL_FADE_IN_SLOPE_LENGTH:
      if (value < 0) { /* Check valid value range */
        err = MPS_INVALID_PARAMETER;
      }
      if (err != MPS_OK) {
        goto bail;
      }
      if (pUserParams->concealFadeInSlopeLength != (UINT)value) {
        pUserParams->concealFadeInSlopeLength = (UINT)value;
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_CONCEAL_PARAMS;
      }
      break;

    case SACDEC_CONCEAL_NUM_RELEASE_FRAMES:
      if (value < 0) { /* Check valid value range */
        err = MPS_INVALID_PARAMETER;
      }
      if (err != MPS_OK) {
        goto bail;
      }
      if (pUserParams->concealNumReleaseFrames != (UINT)value) {
        pUserParams->concealNumReleaseFrames = (UINT)value;
        pMpegSurroundDecoder->initFlags[pMpegSurroundDecoder->bsFrameDecode] |=
            MPEGS_INIT_CHANGE_CONCEAL_PARAMS;
      }
      break;

    default:
      err = MPS_INVALID_PARAMETER;
      break;
  } /* switch(param) */

bail:
  return err;
}

SACDEC_ERROR mpegSurroundDecoder_IsPseudoLR(
    CMpegSurroundDecoder *pMpegSurroundDecoder, int *bsPseudoLr) {
  if (pMpegSurroundDecoder != NULL) {
    const SPATIAL_SPECIFIC_CONFIG *sscDecode =
        &pMpegSurroundDecoder
             ->spatialSpecificConfig[pMpegSurroundDecoder->bsFrameDecode];
    *bsPseudoLr = (int)sscDecode->bsPseudoLr;
    return MPS_OK;
  } else
    return MPS_INVALID_HANDLE;
}

/**
 * \brief Get the signal delay caused by the MPEG Surround decoder module.
 **/
UINT mpegSurroundDecoder_GetDelay(const CMpegSurroundDecoder *self) {
  INT outputDelay = 0;

  if (self != NULL) {
    const SPATIAL_SPECIFIC_CONFIG *sscDecode =
        &self->spatialSpecificConfig[self->bsFrameDecode];
    AUDIO_OBJECT_TYPE coreCodec = sscDecode->coreCodec;

    /* See chapter 4.5 (delay and synchronization) of ISO/IEC FDIS 23003-1 and
       chapter 5.4.3 of ISO/IEC FDIS 23003-2 for details on the following
       figures. */

    if (coreCodec > AOT_NULL_OBJECT) {
      if (IS_LOWDELAY(coreCodec)) {
        /* All low delay variants (ER-AAC-(E)LD): */
        outputDelay += 256;
      } else if (!IS_USAC(coreCodec)) {
        /* By the method of elimination this is the GA (AAC-LC, HE-AAC, ...)
         * branch: */
        outputDelay += 320 + 257; /* cos to exp delay + QMF synthesis */
        if (self->mpegSurroundUseTimeInterface) {
          outputDelay += 320 + 384; /* QMF and hybrid analysis */
        }
      }
    }
  }

  return (outputDelay);
}
