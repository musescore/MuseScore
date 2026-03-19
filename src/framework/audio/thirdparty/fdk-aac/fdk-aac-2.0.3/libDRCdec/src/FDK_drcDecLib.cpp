/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/************************* MPEG-D DRC decoder library **************************

   Author(s):   Bernhard Neugebauer

   Description: MPEG-D DRC Decoder

*******************************************************************************/

#include "drcDec_reader.h"
#include "drcDec_gainDecoder.h"
#include "FDK_drcDecLib.h"

#include "drcDec_selectionProcess.h"
#include "drcDec_tools.h"

/* Decoder library info */
#define DRCDEC_LIB_VL0 2
#define DRCDEC_LIB_VL1 1
#define DRCDEC_LIB_VL2 0
#define DRCDEC_LIB_TITLE "MPEG-D DRC Decoder Lib"
#ifdef SUPPRESS_BUILD_DATE_INFO
#define DRCDEC_LIB_BUILD_DATE ""
#define DRCDEC_LIB_BUILD_TIME ""
#else
#define DRCDEC_LIB_BUILD_DATE __DATE__
#define DRCDEC_LIB_BUILD_TIME __TIME__
#endif

typedef enum {
  DRC_DEC_NOT_INITIALIZED = 0,
  DRC_DEC_INITIALIZED,
  DRC_DEC_NEW_GAIN_PAYLOAD,
  DRC_DEC_INTERPOLATION_PREPARED
} DRC_DEC_STATUS;

struct s_drc_decoder {
  DRC_DEC_CODEC_MODE codecMode;
  DRC_DEC_FUNCTIONAL_RANGE functionalRange;
  DRC_DEC_STATUS status;

  /* handles of submodules */
  HANDLE_DRC_GAIN_DECODER hGainDec;
  HANDLE_DRC_SELECTION_PROCESS hSelectionProc;
  int selProcInputDiff;

  /* data structs */
  UNI_DRC_CONFIG uniDrcConfig;
  LOUDNESS_INFO_SET loudnessInfoSet;
  UNI_DRC_GAIN uniDrcGain;

  SEL_PROC_OUTPUT selProcOutput;
} DRC_DECODER;

static int _getGainStatus(HANDLE_UNI_DRC_GAIN hUniDrcGain) {
  return hUniDrcGain->status;
}

static int isResetNeeded(HANDLE_DRC_DECODER hDrcDec,
                         const SEL_PROC_OUTPUT oldSelProcOutput) {
  int i, resetNeeded = 0;

  if (hDrcDec->selProcOutput.numSelectedDrcSets !=
      oldSelProcOutput.numSelectedDrcSets) {
    resetNeeded = 1;
  } else {
    for (i = 0; i < hDrcDec->selProcOutput.numSelectedDrcSets; i++) {
      if (hDrcDec->selProcOutput.selectedDrcSetIds[i] !=
          oldSelProcOutput.selectedDrcSetIds[i])
        resetNeeded = 1;
      if (hDrcDec->selProcOutput.selectedDownmixIds[i] !=
          oldSelProcOutput.selectedDownmixIds[i])
        resetNeeded = 1;
    }
  }

  if (hDrcDec->selProcOutput.boost != oldSelProcOutput.boost) resetNeeded = 1;
  if (hDrcDec->selProcOutput.compress != oldSelProcOutput.compress)
    resetNeeded = 1;

  /* Note: Changes in downmix matrix are not caught, as they don't affect the
   * DRC gain decoder */

  return resetNeeded;
}

static void startSelectionProcess(HANDLE_DRC_DECODER hDrcDec) {
  int uniDrcConfigHasChanged = 0;
  SEL_PROC_OUTPUT oldSelProcOutput = hDrcDec->selProcOutput;

  if (!hDrcDec->status) return;

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    uniDrcConfigHasChanged = hDrcDec->uniDrcConfig.diff;
    if (hDrcDec->uniDrcConfig.diff || hDrcDec->loudnessInfoSet.diff ||
        hDrcDec->selProcInputDiff) {
      /* in case of an error, signal that selection process was not successful
       */
      hDrcDec->selProcOutput.numSelectedDrcSets = 0;

      drcDec_SelectionProcess_Process(
          hDrcDec->hSelectionProc, &(hDrcDec->uniDrcConfig),
          &(hDrcDec->loudnessInfoSet), &(hDrcDec->selProcOutput));

      hDrcDec->selProcInputDiff = 0;
      hDrcDec->uniDrcConfig.diff = 0;
      hDrcDec->loudnessInfoSet.diff = 0;
    }
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    if (isResetNeeded(hDrcDec, oldSelProcOutput) || uniDrcConfigHasChanged) {
      drcDec_GainDecoder_Config(hDrcDec->hGainDec, &(hDrcDec->uniDrcConfig),
                                hDrcDec->selProcOutput.numSelectedDrcSets,
                                hDrcDec->selProcOutput.selectedDrcSetIds,
                                hDrcDec->selProcOutput.selectedDownmixIds);
    }
  }
}

DRC_DEC_ERROR
FDK_drcDec_Open(HANDLE_DRC_DECODER* phDrcDec,
                const DRC_DEC_FUNCTIONAL_RANGE functionalRange) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  HANDLE_DRC_DECODER hDrcDec;

  *phDrcDec = (HANDLE_DRC_DECODER)FDKcalloc(1, sizeof(DRC_DECODER));
  if (!*phDrcDec) return DRC_DEC_OUT_OF_MEMORY;
  hDrcDec = *phDrcDec;

  hDrcDec->functionalRange = functionalRange;

  hDrcDec->status = DRC_DEC_NOT_INITIALIZED;
  hDrcDec->codecMode = DRC_DEC_CODEC_MODE_UNDEFINED;

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    sErr = drcDec_SelectionProcess_Create(&(hDrcDec->hSelectionProc));
    if (sErr) return DRC_DEC_OUT_OF_MEMORY;
    sErr = drcDec_SelectionProcess_Init(hDrcDec->hSelectionProc);
    if (sErr) return DRC_DEC_NOT_OK;
    hDrcDec->selProcInputDiff = 1;
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    dErr = drcDec_GainDecoder_Open(&(hDrcDec->hGainDec));
    if (dErr) return DRC_DEC_OUT_OF_MEMORY;
  }

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_SetCodecMode(HANDLE_DRC_DECODER hDrcDec,
                        const DRC_DEC_CODEC_MODE codecMode) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->codecMode ==
      DRC_DEC_CODEC_MODE_UNDEFINED) { /* Set codec mode, if it is set for the
                                         first time */
    hDrcDec->codecMode = codecMode;

    if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
      sErr = drcDec_SelectionProcess_SetCodecMode(
          hDrcDec->hSelectionProc, (SEL_PROC_CODEC_MODE)codecMode);
      if (sErr) return DRC_DEC_NOT_OK;
      hDrcDec->selProcInputDiff = 1;
    }

    if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
      DELAY_MODE delayMode;
      int timeDomainSupported;
      SUBBAND_DOMAIN_MODE subbandDomainSupported;

      switch (hDrcDec->codecMode) {
        case DRC_DEC_MPEG_4_AAC:
        case DRC_DEC_MPEG_D_USAC:
        case DRC_DEC_MPEG_H_3DA:
        default:
          delayMode = DM_REGULAR_DELAY;
      }

      switch (hDrcDec->codecMode) {
        case DRC_DEC_MPEG_4_AAC:
        case DRC_DEC_MPEG_D_USAC:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_OFF;
          break;
        case DRC_DEC_MPEG_H_3DA:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_STFT256;
          break;

        case DRC_DEC_TEST_TIME_DOMAIN:
          timeDomainSupported = 1;
          subbandDomainSupported = SDM_OFF;
          break;
        case DRC_DEC_TEST_QMF_DOMAIN:
          timeDomainSupported = 0;
          subbandDomainSupported = SDM_QMF64;
          break;
        case DRC_DEC_TEST_STFT_DOMAIN:
          timeDomainSupported = 0;
          subbandDomainSupported = SDM_STFT256;
          break;

        default:
          timeDomainSupported = 0;
          subbandDomainSupported = SDM_OFF;
      }

      dErr = drcDec_GainDecoder_SetCodecDependentParameters(
          hDrcDec->hGainDec, delayMode, timeDomainSupported,
          subbandDomainSupported);
      if (dErr) return DRC_DEC_NOT_OK;
    }
  }

  /* Don't allow changing codecMode if it has already been set. */
  if (hDrcDec->codecMode != codecMode) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_Init(HANDLE_DRC_DECODER hDrcDec, const int frameSize,
                const int sampleRate, const int baseChannelCount) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;

  if (hDrcDec == NULL || frameSize == 0 || sampleRate == 0 ||
      baseChannelCount == 0)
    return DRC_DEC_OK; /* return without doing anything */

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    sErr = drcDec_SelectionProcess_SetParam(
        hDrcDec->hSelectionProc, SEL_PROC_BASE_CHANNEL_COUNT,
        (FIXP_DBL)baseChannelCount, &(hDrcDec->selProcInputDiff));
    if (sErr) return DRC_DEC_NOT_OK;
    sErr = drcDec_SelectionProcess_SetParam(
        hDrcDec->hSelectionProc, SEL_PROC_SAMPLE_RATE, (FIXP_DBL)sampleRate,
        &(hDrcDec->selProcInputDiff));
    if (sErr) return DRC_DEC_NOT_OK;
  }

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    dErr = drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_FRAME_SIZE,
                                       frameSize);
    if (dErr) return DRC_DEC_NOT_OK;
    dErr = drcDec_GainDecoder_SetParam(hDrcDec->hGainDec, GAIN_DEC_SAMPLE_RATE,
                                       sampleRate);
    if (dErr) return DRC_DEC_NOT_OK;
    dErr = drcDec_GainDecoder_Init(hDrcDec->hGainDec);
    if (dErr) return DRC_DEC_NOT_OK;
  }

  hDrcDec->status = DRC_DEC_INITIALIZED;

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_Close(HANDLE_DRC_DECODER* phDrcDec) {
  HANDLE_DRC_DECODER hDrcDec;

  if (phDrcDec == NULL) {
    return DRC_DEC_OK;
  }

  hDrcDec = *phDrcDec;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    drcDec_GainDecoder_Close(&(hDrcDec->hGainDec));
  }

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    drcDec_SelectionProcess_Delete(&(hDrcDec->hSelectionProc));
  }

  FDKfree(*phDrcDec);
  *phDrcDec = NULL;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_SetParam(HANDLE_DRC_DECODER hDrcDec,
                    const DRC_DEC_USERPARAM requestType,
                    const FIXP_DBL requestValue) {
  DRC_ERROR dErr = DE_OK;
  DRCDEC_SELECTION_PROCESS_RETURN sErr = DRCDEC_SELECTION_PROCESS_NO_ERROR;
  int invalidParameter = 0;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->functionalRange & DRC_DEC_GAIN) {
    switch (requestType) {
      case DRC_DEC_SAMPLE_RATE:
        dErr = drcDec_GainDecoder_SetParam(
            hDrcDec->hGainDec, GAIN_DEC_SAMPLE_RATE, (int)requestValue);
        if (dErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_FRAME_SIZE:
        dErr = drcDec_GainDecoder_SetParam(
            hDrcDec->hGainDec, GAIN_DEC_FRAME_SIZE, (int)requestValue);
        if (dErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      default:
        invalidParameter |= DRC_DEC_GAIN;
    }
  }

  if (hDrcDec->functionalRange & DRC_DEC_SELECTION) {
    switch (requestType) {
      case DRC_DEC_BOOST:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc,
                                                SEL_PROC_BOOST, requestValue,
                                                &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_COMPRESS:
        sErr = drcDec_SelectionProcess_SetParam(hDrcDec->hSelectionProc,
                                                SEL_PROC_COMPRESS, requestValue,
                                                &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_LOUDNESS_NORMALIZATION_ON:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_LOUDNESS_NORMALIZATION_ON,
            requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_TARGET_LOUDNESS:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_TARGET_LOUDNESS, requestValue,
            &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_EFFECT_TYPE:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_EFFECT_TYPE, requestValue,
            &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_DOWNMIX_ID:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_DOWNMIX_ID, requestValue,
            &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_TARGET_CHANNEL_COUNT_REQUESTED:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_TARGET_CHANNEL_COUNT,
            requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_BASE_CHANNEL_COUNT:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_BASE_CHANNEL_COUNT, requestValue,
            &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_NOT_OK;
        break;
      case DRC_DEC_LOUDNESS_MEASUREMENT_METHOD:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_LOUDNESS_MEASUREMENT_METHOD,
            requestValue, &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      case DRC_DEC_ALBUM_MODE:
        sErr = drcDec_SelectionProcess_SetParam(
            hDrcDec->hSelectionProc, SEL_PROC_ALBUM_MODE, requestValue,
            &(hDrcDec->selProcInputDiff));
        if (sErr) return DRC_DEC_PARAM_OUT_OF_RANGE;
        break;
      default:
        invalidParameter |= DRC_DEC_SELECTION;
    }
  }

  if (invalidParameter == hDrcDec->functionalRange)
    return DRC_DEC_INVALID_PARAM;

  /* All parameters need a new start of the selection process */
  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

LONG FDK_drcDec_GetParam(HANDLE_DRC_DECODER hDrcDec,
                         const DRC_DEC_USERPARAM requestType) {
  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  switch (requestType) {
    case DRC_DEC_BOOST:
      return (LONG)hDrcDec->selProcOutput.boost;
    case DRC_DEC_COMPRESS:
      return (LONG)hDrcDec->selProcOutput.compress;
    case DRC_DEC_IS_MULTIBAND_DRC_1:
      return (LONG)bitstreamContainsMultibandDrc(&hDrcDec->uniDrcConfig, 0);
    case DRC_DEC_IS_MULTIBAND_DRC_2:
      return (LONG)bitstreamContainsMultibandDrc(&hDrcDec->uniDrcConfig, 0x7F);
    case DRC_DEC_IS_ACTIVE: {
      /* MPEG-D DRC is considered active (and overrides MPEG-4 DRC), if
       * uniDrc payload is present (loudnessInfoSet and/or uniDrcConfig)
       * at least one of DRC and Loudness Control is switched on */
      int drcOn = drcDec_SelectionProcess_GetParam(
          hDrcDec->hSelectionProc, SEL_PROC_DYNAMIC_RANGE_CONTROL_ON);
      int lnOn = drcDec_SelectionProcess_GetParam(
          hDrcDec->hSelectionProc, SEL_PROC_LOUDNESS_NORMALIZATION_ON);
      int uniDrcPayloadPresent =
          (hDrcDec->loudnessInfoSet.loudnessInfoCount > 0);
      uniDrcPayloadPresent |=
          (hDrcDec->loudnessInfoSet.loudnessInfoAlbumCount > 0);
      uniDrcPayloadPresent |=
          (hDrcDec->uniDrcConfig.drcInstructionsUniDrcCount > 0);
      uniDrcPayloadPresent |=
          (hDrcDec->uniDrcConfig.downmixInstructionsCount > 0);
      return (LONG)(uniDrcPayloadPresent && (drcOn || lnOn));
    }
    case DRC_DEC_TARGET_CHANNEL_COUNT_SELECTED:
      return (LONG)hDrcDec->selProcOutput.targetChannelCount;
    case DRC_DEC_OUTPUT_LOUDNESS:
      return (LONG)hDrcDec->selProcOutput.outputLoudness;
    default:
      return 0;
  }
}

DRC_DEC_ERROR
FDK_drcDec_SetInterfaceParameters(HANDLE_DRC_DECODER hDrcDec,
                                  HANDLE_UNI_DRC_INTERFACE hUniDrcInterface) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}

DRC_DEC_ERROR
FDK_drcDec_SetSelectionProcessMpeghParameters_simple(
    HANDLE_DRC_DECODER hDrcDec, const int groupPresetIdRequested,
    const int numGroupIdsRequested, const int* groupIdsRequested) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}

DRC_DEC_ERROR
FDK_drcDec_SetDownmixInstructions(HANDLE_DRC_DECODER hDrcDec,
                                  const int numDownmixId, const int* downmixId,
                                  const int* targetLayout,
                                  const int* targetChannelCount) {
  return DRC_DEC_UNSUPPORTED_FUNCTION;
}

void FDK_drcDec_SetSelectionProcessOutput(
    HANDLE_DRC_DECODER hDrcDec, HANDLE_SEL_PROC_OUTPUT hSelProcOutput) {}

HANDLE_SEL_PROC_OUTPUT
FDK_drcDec_GetSelectionProcessOutput(HANDLE_DRC_DECODER hDrcDec) {
  if (hDrcDec == NULL) return NULL;

  return &(hDrcDec->selProcOutput);
}

LONG /* FIXP_DBL, e = 7 */
FDK_drcDec_GetGroupLoudness(HANDLE_SEL_PROC_OUTPUT hSelProcOutput,
                            const int groupID, int* groupLoudnessAvailable) {
  return (LONG)0;
}

void FDK_drcDec_SetChannelGains(HANDLE_DRC_DECODER hDrcDec,
                                const int numChannels, const int frameSize,
                                FIXP_DBL* channelGainDb, FIXP_DBL* audioBuffer,
                                const int audioBufferChannelOffset) {
  int err;

  if (hDrcDec == NULL) return;

  err = drcDec_GainDecoder_SetLoudnessNormalizationGainDb(
      hDrcDec->hGainDec, hDrcDec->selProcOutput.loudnessNormalizationGainDb);
  if (err) return;

  drcDec_GainDecoder_SetChannelGains(hDrcDec->hGainDec, numChannels, frameSize,
                                     channelGainDb, audioBufferChannelOffset,
                                     audioBuffer);
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcConfig(HANDLE_DRC_DECODER hDrcDec,
                            HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->codecMode == DRC_DEC_MPEG_D_USAC) {
    dErr = drcDec_readUniDrcConfig(hBitstream, &(hDrcDec->uniDrcConfig));
  } else
    return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occured */
    FDKmemclear(&hDrcDec->uniDrcConfig, sizeof(hDrcDec->uniDrcConfig));
    hDrcDec->uniDrcConfig.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadDownmixInstructions_Box(HANDLE_DRC_DECODER hDrcDec,
                                       HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occurred */
    FDKmemclear(&hDrcDec->uniDrcConfig.downmixInstructions,
                sizeof(hDrcDec->uniDrcConfig.downmixInstructions));
    hDrcDec->uniDrcConfig.downmixInstructionsCount = 0;
    hDrcDec->uniDrcConfig.downmixInstructionsCountV0 = 0;
    hDrcDec->uniDrcConfig.downmixInstructionsCountV1 = 0;
    hDrcDec->uniDrcConfig.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcInstructions_Box(HANDLE_DRC_DECODER hDrcDec,
                                      HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occurred */
    FDKmemclear(&hDrcDec->uniDrcConfig.drcInstructionsUniDrc,
                sizeof(hDrcDec->uniDrcConfig.drcInstructionsUniDrc));
    hDrcDec->uniDrcConfig.drcInstructionsUniDrcCount = 0;
    hDrcDec->uniDrcConfig.drcInstructionsUniDrcCountV0 = 0;
    hDrcDec->uniDrcConfig.drcInstructionsUniDrcCountV1 = 0;
    hDrcDec->uniDrcConfig.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcCoefficients_Box(HANDLE_DRC_DECODER hDrcDec,
                                      HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occurred */
    FDKmemclear(&hDrcDec->uniDrcConfig.drcCoefficientsUniDrc,
                sizeof(hDrcDec->uniDrcConfig.drcCoefficientsUniDrc));
    hDrcDec->uniDrcConfig.drcCoefficientsUniDrcCount = 0;
    hDrcDec->uniDrcConfig.drcCoefficientsUniDrcCountV0 = 0;
    hDrcDec->uniDrcConfig.drcCoefficientsUniDrcCountV1 = 0;
    hDrcDec->uniDrcConfig.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadLoudnessInfoSet(HANDLE_DRC_DECODER hDrcDec,
                               HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  if (hDrcDec->codecMode == DRC_DEC_MPEG_D_USAC) {
    dErr = drcDec_readLoudnessInfoSet(hBitstream, &(hDrcDec->loudnessInfoSet));
  } else
    return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occurred */
    FDKmemclear(&hDrcDec->loudnessInfoSet, sizeof(hDrcDec->loudnessInfoSet));
    hDrcDec->loudnessInfoSet.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadLoudnessBox(HANDLE_DRC_DECODER hDrcDec,
                           HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;

  return DRC_DEC_NOT_OK;

  if (dErr) {
    /* clear config, if parsing error occurred */
    FDKmemclear(&hDrcDec->loudnessInfoSet, sizeof(hDrcDec->loudnessInfoSet));
    hDrcDec->loudnessInfoSet.diff = 1;
  }

  startSelectionProcess(hDrcDec);

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrcGain(HANDLE_DRC_DECODER hDrcDec,
                          HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!hDrcDec->status) {
    return DRC_DEC_OK;
  }

  dErr = drcDec_readUniDrcGain(
      hBitstream, &(hDrcDec->uniDrcConfig),
      drcDec_GainDecoder_GetFrameSize(hDrcDec->hGainDec),
      drcDec_GainDecoder_GetDeltaTminDefault(hDrcDec->hGainDec),
      &(hDrcDec->uniDrcGain));
  if (dErr) return DRC_DEC_NOT_OK;

  if (_getGainStatus(&(hDrcDec->uniDrcGain))) {
    hDrcDec->status = DRC_DEC_NEW_GAIN_PAYLOAD;
  }

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ReadUniDrc(HANDLE_DRC_DECODER hDrcDec,
                      HANDLE_FDK_BITSTREAM hBitstream) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!hDrcDec->status) return DRC_DEC_NOT_READY;

  dErr = drcDec_readUniDrc(
      hBitstream, &(hDrcDec->uniDrcConfig), &(hDrcDec->loudnessInfoSet),
      drcDec_GainDecoder_GetFrameSize(hDrcDec->hGainDec),
      drcDec_GainDecoder_GetDeltaTminDefault(hDrcDec->hGainDec),
      &(hDrcDec->uniDrcGain));

  startSelectionProcess(hDrcDec);
  if (dErr) return DRC_DEC_NOT_OK;

  if (_getGainStatus(&(hDrcDec->uniDrcGain))) {
    hDrcDec->status = DRC_DEC_NEW_GAIN_PAYLOAD;
  }

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_Preprocess(HANDLE_DRC_DECODER hDrcDec) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!hDrcDec->status) return DRC_DEC_NOT_READY;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;

  if (hDrcDec->status != DRC_DEC_NEW_GAIN_PAYLOAD) {
    /* no new gain payload was read, e.g. during concalment or flushing.
       Generate DRC gains based on the stored DRC gains of last frames */
    drcDec_GainDecoder_Conceal(hDrcDec->hGainDec, &(hDrcDec->uniDrcConfig),
                               &(hDrcDec->uniDrcGain));
  }

  dErr = drcDec_GainDecoder_Preprocess(
      hDrcDec->hGainDec, &(hDrcDec->uniDrcGain),
      hDrcDec->selProcOutput.loudnessNormalizationGainDb,
      hDrcDec->selProcOutput.boost, hDrcDec->selProcOutput.compress);
  if (dErr) return DRC_DEC_NOT_OK;
  hDrcDec->status = DRC_DEC_INTERPOLATION_PREPARED;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ProcessTime(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation,
                       const int channelOffset, const int drcChannelOffset,
                       const int numChannelsProcessed, FIXP_DBL* realBuffer,
                       const int timeDataChannelOffset) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;
  if (hDrcDec->status != DRC_DEC_INTERPOLATION_PREPARED)
    return DRC_DEC_NOT_READY;

  dErr = drcDec_GainDecoder_ProcessTimeDomain(
      hDrcDec->hGainDec, delaySamples, (GAIN_DEC_LOCATION)drcLocation,
      channelOffset, drcChannelOffset, numChannelsProcessed,
      timeDataChannelOffset, realBuffer);
  if (dErr) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ProcessFreq(HANDLE_DRC_DECODER hDrcDec, const int delaySamples,
                       const DRC_DEC_LOCATION drcLocation,
                       const int channelOffset, const int drcChannelOffset,
                       const int numChannelsProcessed,
                       const int processSingleTimeslot, FIXP_DBL** realBuffer,
                       FIXP_DBL** imagBuffer) {
  DRC_ERROR dErr = DE_OK;

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;
  if (hDrcDec->status != DRC_DEC_INTERPOLATION_PREPARED)
    return DRC_DEC_NOT_READY;

  dErr = drcDec_GainDecoder_ProcessSubbandDomain(
      hDrcDec->hGainDec, delaySamples, (GAIN_DEC_LOCATION)drcLocation,
      channelOffset, drcChannelOffset, numChannelsProcessed,
      processSingleTimeslot, realBuffer, imagBuffer);
  if (dErr) return DRC_DEC_NOT_OK;

  return DRC_DEC_OK;
}

DRC_DEC_ERROR
FDK_drcDec_ApplyDownmix(HANDLE_DRC_DECODER hDrcDec, int* reverseInChannelMap,
                        int* reverseOutChannelMap, FIXP_DBL* realBuffer,
                        int* pNChannels) {
  SEL_PROC_OUTPUT* pSelProcOutput = &(hDrcDec->selProcOutput);
  int baseChCnt = pSelProcOutput->baseChannelCount;
  int targetChCnt = pSelProcOutput->targetChannelCount;
  int frameSize, n, ic, oc;
  FIXP_DBL tmp_out[8];
  FIXP_DBL* audioChannels[8];

  if (hDrcDec == NULL) return DRC_DEC_NOT_OPENED;
  if (!(hDrcDec->functionalRange & DRC_DEC_GAIN)) return DRC_DEC_NOT_OK;

  /* only downmix is performed here, no upmix.
     Downmix is only performed if downmix coefficients are provided.
     All other cases of downmix and upmix are treated by pcmDmx library. */
  if (pSelProcOutput->downmixMatrixPresent == 0)
    return DRC_DEC_OK;                             /* no downmix */
  if (targetChCnt >= baseChCnt) return DRC_DEC_OK; /* downmix only */

  /* sanity checks */
  if (realBuffer == NULL) return DRC_DEC_NOT_OK;
  if (reverseInChannelMap == NULL) return DRC_DEC_NOT_OK;
  if (reverseOutChannelMap == NULL) return DRC_DEC_NOT_OK;
  if (baseChCnt > 8) return DRC_DEC_NOT_OK;
  if (baseChCnt != *pNChannels) return DRC_DEC_NOT_OK;
  if (targetChCnt > 8) return DRC_DEC_NOT_OK;

  frameSize = drcDec_GainDecoder_GetFrameSize(hDrcDec->hGainDec);

  for (ic = 0; ic < baseChCnt; ic++) {
    audioChannels[ic] = &(realBuffer[ic * frameSize]);
  }

  /* in-place downmix */
  for (n = 0; n < frameSize; n++) {
    for (oc = 0; oc < targetChCnt; oc++) {
      tmp_out[oc] = (FIXP_DBL)0;
      for (ic = 0; ic < baseChCnt; ic++) {
        tmp_out[oc] +=
            fMultDiv2(audioChannels[ic][n],
                      pSelProcOutput->downmixMatrix[reverseInChannelMap[ic]]
                                                   [reverseOutChannelMap[oc]])
            << 3;
      }
    }
    for (oc = 0; oc < targetChCnt; oc++) {
      if (oc >= baseChCnt) break;
      audioChannels[oc][n] = tmp_out[oc];
    }
  }

  for (oc = targetChCnt; oc < baseChCnt; oc++) {
    FDKmemset(audioChannels[oc], 0, frameSize * sizeof(FIXP_DBL));
  }

  *pNChannels = targetChCnt;

  return DRC_DEC_OK;
}

/* Get library info for this module. */
DRC_DEC_ERROR
FDK_drcDec_GetLibInfo(LIB_INFO* info) {
  int i;

  if (info == NULL) {
    return DRC_DEC_INVALID_PARAM;
  }

  /* Search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) {
    return DRC_DEC_NOT_OK;
  }

  /* Add the library info */
  info[i].module_id = FDK_UNIDRCDEC;
  info[i].version = LIB_VERSION(DRCDEC_LIB_VL0, DRCDEC_LIB_VL1, DRCDEC_LIB_VL2);
  LIB_VERSION_STRING(info + i);
  info[i].build_date = DRCDEC_LIB_BUILD_DATE;
  info[i].build_time = DRCDEC_LIB_BUILD_TIME;
  info[i].title = DRCDEC_LIB_TITLE;

  return DRC_DEC_OK;
}
