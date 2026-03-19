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

/**************************** SBR encoder library ******************************

   Author(s):   Andreas Ehret, Tobias Chalupka

   Description: SBR encoder top level processing.

*******************************************************************************/

#include "sbr_encoder.h"

#include "sbrenc_ram.h"
#include "sbrenc_rom.h"
#include "sbrenc_freq_sca.h"
#include "env_bit.h"
#include "cmondata.h"
#include "sbr_misc.h"
#include "sbr.h"
#include "qmf.h"

#include "ps_main.h"

#define SBRENCODER_LIB_VL0 4
#define SBRENCODER_LIB_VL1 0
#define SBRENCODER_LIB_VL2 0

/***************************************************************************/
/*
 * SBR Delay balancing definitions.
 */

/*
      input buffer (1ch)

      |------------ 1537   -------------|-----|---------- 2048 -------------|
           (core2sbr delay     )          ds     (read, core and ds area)
*/

#define SFB(dwnsmp) \
  (32 << (dwnsmp -  \
          1)) /* SBR Frequency bands: 64 for dual-rate, 32 for single-rate */
#define STS(fl)                                                              \
  (((fl) == 1024) ? 32                                                       \
                  : 30) /* SBR Time Slots: 32 for core frame length 1024, 30 \
                           for core frame length 960 */

#define DELAY_QMF_ANA(dwnsmp) \
  ((320 << ((dwnsmp)-1)) - (32 << ((dwnsmp)-1))) /* Full bandwidth */
#define DELAY_HYB_ANA (10 * 64) /* + 0.5 */      /*  */
#define DELAY_HYB_SYN (6 * 64 - 32)              /*  */
#define DELAY_QMF_POSTPROC(dwnsmp) \
  (32 * (dwnsmp))                               /* QMF postprocessing delay */
#define DELAY_DEC_QMF(dwnsmp) (6 * SFB(dwnsmp)) /* Decoder QMF overlap */
#define DELAY_QMF_SYN(dwnsmp) \
  (1 << (dwnsmp -             \
         1)) /* QMF_NO_POLY/2=2.5, rounded down to 2, half for single-rate */
#define DELAY_QMF_DS (32) /* QMF synthesis for downsampled time signal */

/* Delay in QMF paths */
#define DELAY_SBR(fl, dwnsmp) \
  (DELAY_QMF_ANA(dwnsmp) + (SFB(dwnsmp) * STS(fl) - 1) + DELAY_QMF_SYN(dwnsmp))
#define DELAY_PS(fl, dwnsmp)                                       \
  (DELAY_QMF_ANA(dwnsmp) + DELAY_HYB_ANA + DELAY_DEC_QMF(dwnsmp) + \
   (SFB(dwnsmp) * STS(fl) - 1) + DELAY_HYB_SYN + DELAY_QMF_SYN(dwnsmp))
#define DELAY_ELDSBR(fl, dwnsmp) \
  ((((fl) / 2) * (dwnsmp)) - 1 + DELAY_QMF_POSTPROC(dwnsmp))
#define DELAY_ELDv2SBR(fl, dwnsmp)                                        \
  ((((fl) / 2) * (dwnsmp)) - 1 + 80 * (dwnsmp)) /* 80 is the delay caused \
                                                   by the sum of the CLD  \
                                                   analysis and the MPSLD \
                                                   synthesis filterbank */

/* Delay in core path (core and downsampler not taken into account) */
#define DELAY_COREPATH_SBR(fl, dwnsmp) \
  ((DELAY_QMF_ANA(dwnsmp) + DELAY_DEC_QMF(dwnsmp) + DELAY_QMF_SYN(dwnsmp)))
#define DELAY_COREPATH_ELDSBR(fl, dwnsmp) ((DELAY_QMF_POSTPROC(dwnsmp)))
#define DELAY_COREPATH_ELDv2SBR(fl, dwnsmp) (128 * (dwnsmp)) /* 4 slots */
#define DELAY_COREPATH_PS(fl, dwnsmp)                                        \
  ((DELAY_QMF_ANA(dwnsmp) + DELAY_QMF_DS +                                   \
    /*(DELAY_AAC(fl)*2) + */ DELAY_QMF_ANA(dwnsmp) + DELAY_DEC_QMF(dwnsmp) + \
    DELAY_HYB_SYN + DELAY_QMF_SYN(dwnsmp))) /* 2048 - 463*2 */

/* Delay differences between SBR- and downsampled path for SBR and SBR+PS */
#define DELAY_AAC2SBR(fl, dwnsmp) \
  ((DELAY_COREPATH_SBR(fl, dwnsmp)) - DELAY_SBR((fl), (dwnsmp)))
#define DELAY_ELD2SBR(fl, dwnsmp) \
  ((DELAY_COREPATH_ELDSBR(fl, dwnsmp)) - DELAY_ELDSBR(fl, dwnsmp))
#define DELAY_AAC2PS(fl, dwnsmp) \
  ((DELAY_COREPATH_PS(fl, dwnsmp)) - DELAY_PS(fl, dwnsmp)) /* 2048 - 463*2 */

/* Assumption: The sample delay resulting of of DELAY_AAC2PS is always smaller
 * than the sample delay implied by DELAY_AAC2SBR */
#define MAX_DS_FILTER_DELAY \
  (5) /* the additional max downsampler filter delay (source fs) */
#define MAX_SAMPLE_DELAY                                                 \
  (DELAY_AAC2SBR(1024, 2) + MAX_DS_FILTER_DELAY) /* maximum delay: frame \
                                                    length of 1024 and   \
                                                    dual-rate sbr */

/***************************************************************************/

/*************** Delay parameters for sbrEncoder_Init_delay() **************/
typedef struct {
  int dsDelay;        /* the delay of the (time-domain) downsampler itself */
  int delay;          /* overall delay / samples  */
  int sbrDecDelay;    /* SBR decoder's delay */
  int corePathOffset; /* core path offset / samples; added by
                         sbrEncoder_Init_delay() */
  int sbrPathOffset;  /* SBR path offset / samples; added by
                         sbrEncoder_Init_delay() */
  int bitstrDelay; /* bitstream delay / frames; added by sbrEncoder_Init_delay()
                    */
  int delayInput2Core; /* delay of the input to the core / samples */
} DELAY_PARAM;
/***************************************************************************/

#define INVALID_TABLE_IDX -1

/***************************************************************************/
/*!

  \brief  Selects the SBR tuning settings to use dependent on number of
          channels, bitrate, sample rate and core coder

  \return Index to the appropriate table

****************************************************************************/
#define DISTANCE_CEIL_VALUE 5000000
static INT getSbrTuningTableIndex(
    UINT bitrate,     /*! the total bitrate in bits/sec */
    UINT numChannels, /*! the number of channels for the core coder */
    UINT sampleRate,  /*! the sampling rate of the core coder */
    AUDIO_OBJECT_TYPE core, UINT *pBitRateClosest) {
  int i, bitRateClosestLowerIndex = -1, bitRateClosestUpperIndex = -1,
         found = 0;
  UINT bitRateClosestUpper = 0, bitRateClosestLower = DISTANCE_CEIL_VALUE;

#define isForThisCore(i)                                                     \
  ((sbrTuningTable[i].coreCoder == CODEC_AACLD && core == AOT_ER_AAC_ELD) || \
   (sbrTuningTable[i].coreCoder == CODEC_AAC && core != AOT_ER_AAC_ELD))

  for (i = 0; i < sbrTuningTableSize; i++) {
    if (isForThisCore(i)) /* tuning table is for this core codec */
    {
      if (numChannels == sbrTuningTable[i].numChannels &&
          sampleRate == sbrTuningTable[i].sampleRate) {
        found = 1;
        if ((bitrate >= sbrTuningTable[i].bitrateFrom) &&
            (bitrate < sbrTuningTable[i].bitrateTo)) {
          return i;
        } else {
          if (sbrTuningTable[i].bitrateFrom > bitrate) {
            if (sbrTuningTable[i].bitrateFrom < bitRateClosestLower) {
              bitRateClosestLower = sbrTuningTable[i].bitrateFrom;
              bitRateClosestLowerIndex = i;
            }
          }
          if (sbrTuningTable[i].bitrateTo <= bitrate) {
            if (sbrTuningTable[i].bitrateTo > bitRateClosestUpper) {
              bitRateClosestUpper = sbrTuningTable[i].bitrateTo - 1;
              bitRateClosestUpperIndex = i;
            }
          }
        }
      }
    }
  }

  if (bitRateClosestUpperIndex >= 0) {
    return bitRateClosestUpperIndex;
  }

  if (pBitRateClosest != NULL) {
    /* If there was at least one matching tuning entry pick the least distance
     * bit rate */
    if (found) {
      int distanceUpper = DISTANCE_CEIL_VALUE,
          distanceLower = DISTANCE_CEIL_VALUE;
      if (bitRateClosestLowerIndex >= 0) {
        distanceLower =
            sbrTuningTable[bitRateClosestLowerIndex].bitrateFrom - bitrate;
      }
      if (bitRateClosestUpperIndex >= 0) {
        distanceUpper =
            bitrate - sbrTuningTable[bitRateClosestUpperIndex].bitrateTo;
      }
      if (distanceUpper < distanceLower) {
        *pBitRateClosest = bitRateClosestUpper;
      } else {
        *pBitRateClosest = bitRateClosestLower;
      }
    } else {
      *pBitRateClosest = 0;
    }
  }

  return INVALID_TABLE_IDX;
}

/***************************************************************************/
/*!

  \brief  Selects the PS tuning settings to use dependent on bitrate
  and core coder

  \return Index to the appropriate table

****************************************************************************/
static INT getPsTuningTableIndex(UINT bitrate, UINT *pBitRateClosest) {
  INT i, paramSets = sizeof(psTuningTable) / sizeof(psTuningTable[0]);
  int bitRateClosestLowerIndex = -1, bitRateClosestUpperIndex = -1;
  UINT bitRateClosestUpper = 0, bitRateClosestLower = DISTANCE_CEIL_VALUE;

  for (i = 0; i < paramSets; i++) {
    if ((bitrate >= psTuningTable[i].bitrateFrom) &&
        (bitrate < psTuningTable[i].bitrateTo)) {
      return i;
    } else {
      if (psTuningTable[i].bitrateFrom > bitrate) {
        if (psTuningTable[i].bitrateFrom < bitRateClosestLower) {
          bitRateClosestLower = psTuningTable[i].bitrateFrom;
          bitRateClosestLowerIndex = i;
        }
      }
      if (psTuningTable[i].bitrateTo <= bitrate) {
        if (psTuningTable[i].bitrateTo > bitRateClosestUpper) {
          bitRateClosestUpper = psTuningTable[i].bitrateTo - 1;
          bitRateClosestUpperIndex = i;
        }
      }
    }
  }

  if (bitRateClosestUpperIndex >= 0) {
    return bitRateClosestUpperIndex;
  }

  if (pBitRateClosest != NULL) {
    int distanceUpper = DISTANCE_CEIL_VALUE,
        distanceLower = DISTANCE_CEIL_VALUE;
    if (bitRateClosestLowerIndex >= 0) {
      distanceLower =
          sbrTuningTable[bitRateClosestLowerIndex].bitrateFrom - bitrate;
    }
    if (bitRateClosestUpperIndex >= 0) {
      distanceUpper =
          bitrate - sbrTuningTable[bitRateClosestUpperIndex].bitrateTo;
    }
    if (distanceUpper < distanceLower) {
      *pBitRateClosest = bitRateClosestUpper;
    } else {
      *pBitRateClosest = bitRateClosestLower;
    }
  }

  return INVALID_TABLE_IDX;
}

/***************************************************************************/
/*!

  \brief  In case of downsampled SBR we may need to lower the stop freq
          of a tuning setting to fit into the lower half of the
          spectrum ( which is sampleRate/4 )

  \return the adapted stop frequency index (-1 -> error)

  \ingroup SbrEncCfg

****************************************************************************/
static INT FDKsbrEnc_GetDownsampledStopFreq(const INT sampleRateCore,
                                            const INT startFreq, INT stopFreq,
                                            const INT downSampleFactor) {
  INT maxStopFreqRaw = sampleRateCore / 2;
  INT startBand, stopBand;
  HANDLE_ERROR_INFO err;

  while (stopFreq > 0 && FDKsbrEnc_getSbrStopFreqRAW(stopFreq, sampleRateCore) >
                             maxStopFreqRaw) {
    stopFreq--;
  }

  if (FDKsbrEnc_getSbrStopFreqRAW(stopFreq, sampleRateCore) > maxStopFreqRaw)
    return -1;

  err = FDKsbrEnc_FindStartAndStopBand(
      sampleRateCore << (downSampleFactor - 1), sampleRateCore,
      32 << (downSampleFactor - 1), startFreq, stopFreq, &startBand, &stopBand);
  if (err) return -1;

  return stopFreq;
}

/***************************************************************************/
/*!

  \brief  tells us, if for the given coreCoder, bitrate, number of channels
          and input sampling rate an SBR setting is available. If yes, it
          tells us also the core sampling rate we would need to run with

  \return a flag indicating success: yes (1) or no (0)

****************************************************************************/
static UINT FDKsbrEnc_IsSbrSettingAvail(
    UINT bitrate,           /*! the total bitrate in bits/sec */
    UINT vbrMode,           /*! the vbr paramter, 0 means constant bitrate */
    UINT numOutputChannels, /*! the number of channels for the core coder */
    UINT sampleRateInput,   /*! the input sample rate [in Hz] */
    UINT sampleRateCore,    /*! the core's sampling rate */
    AUDIO_OBJECT_TYPE core) {
  INT idx = INVALID_TABLE_IDX;

  if (sampleRateInput < 16000) return 0;

  if (bitrate == 0) {
    /* map vbr quality to bitrate */
    if (vbrMode < 30)
      bitrate = 24000;
    else if (vbrMode < 40)
      bitrate = 28000;
    else if (vbrMode < 60)
      bitrate = 32000;
    else if (vbrMode < 75)
      bitrate = 40000;
    else
      bitrate = 48000;
    bitrate *= numOutputChannels;
  }

  idx = getSbrTuningTableIndex(bitrate, numOutputChannels, sampleRateCore, core,
                               NULL);

  return (idx == INVALID_TABLE_IDX ? 0 : 1);
}

/***************************************************************************/
/*!

  \brief  Adjusts the SBR settings according to the chosen core coder
          settings which are accessible via config->codecSettings

  \return A flag indicating success: yes (1) or no (0)

****************************************************************************/
static UINT FDKsbrEnc_AdjustSbrSettings(
    const sbrConfigurationPtr config, /*! output, modified */
    UINT bitRate,                     /*! the total bitrate in bits/sec */
    UINT numChannels,                 /*! the core coder number of channels */
    UINT sampleRateCore,              /*! the core coder sampling rate in Hz */
    UINT sampleRateSbr,               /*! the sbr coder sampling rate in Hz */
    UINT transFac,                    /*! the short block to long block ratio */
    UINT standardBitrate, /*! the standard bitrate per channel in bits/sec */
    UINT vbrMode, /*! the vbr paramter, 0 poor quality .. 100 high quality*/
    UINT useSpeechConfig,   /*!< adapt tuning parameters for speech ? */
    UINT lcsMode,           /*! the low complexity stereo mode */
    UINT bParametricStereo, /*!< use parametric stereo */
    AUDIO_OBJECT_TYPE core) /* Core audio codec object type */
{
  INT idx = INVALID_TABLE_IDX;
  /* set the core codec settings */
  config->codecSettings.bitRate = bitRate;
  config->codecSettings.nChannels = numChannels;
  config->codecSettings.sampleFreq = sampleRateCore;
  config->codecSettings.transFac = transFac;
  config->codecSettings.standardBitrate = standardBitrate;

  if (bitRate < 28000) {
    config->threshold_AmpRes_FF_m = (FIXP_DBL)MAXVAL_DBL;
    config->threshold_AmpRes_FF_e = 7;
  } else if (bitRate >= 28000 && bitRate <= 48000) {
    /* The float threshold is 75
       0.524288f is fractional part of RELAXATION, the quotaMatrix and therefore
       tonality are scaled by this 2/3 is because the original implementation
       divides the tonality values by 3, here it's divided by 2 128 compensates
       the necessary shiftfactor of 7 */
    config->threshold_AmpRes_FF_m =
        FL2FXCONST_DBL(75.0f * 0.524288f / (2.0f / 3.0f) / 128.0f);
    config->threshold_AmpRes_FF_e = 7;
  } else if (bitRate > 48000) {
    config->threshold_AmpRes_FF_m = FL2FXCONST_DBL(0);
    config->threshold_AmpRes_FF_e = 0;
  }

  if (bitRate == 0) {
    /* map vbr quality to bitrate */
    if (vbrMode < 30)
      bitRate = 24000;
    else if (vbrMode < 40)
      bitRate = 28000;
    else if (vbrMode < 60)
      bitRate = 32000;
    else if (vbrMode < 75)
      bitRate = 40000;
    else
      bitRate = 48000;
    bitRate *= numChannels;
    /* fix to enable mono vbrMode<40 @ 44.1 of 48kHz */
    if (numChannels == 1) {
      if (sampleRateSbr == 44100 || sampleRateSbr == 48000) {
        if (vbrMode < 40) bitRate = 32000;
      }
    }
  }

  idx =
      getSbrTuningTableIndex(bitRate, numChannels, sampleRateCore, core, NULL);

  if (idx != INVALID_TABLE_IDX) {
    config->startFreq = sbrTuningTable[idx].startFreq;
    config->stopFreq = sbrTuningTable[idx].stopFreq;
    if (useSpeechConfig) {
      config->startFreq = sbrTuningTable[idx].startFreqSpeech;
      config->stopFreq = sbrTuningTable[idx].stopFreqSpeech;
    }

    /* Adapt stop frequency in case of downsampled SBR - only 32 bands then */
    if (1 == config->downSampleFactor) {
      INT dsStopFreq = FDKsbrEnc_GetDownsampledStopFreq(
          sampleRateCore, config->startFreq, config->stopFreq,
          config->downSampleFactor);
      if (dsStopFreq < 0) {
        return 0;
      }

      config->stopFreq = dsStopFreq;
    }

    config->sbr_noise_bands = sbrTuningTable[idx].numNoiseBands;
    if (core == AOT_ER_AAC_ELD) config->init_amp_res_FF = SBR_AMP_RES_1_5;
    config->noiseFloorOffset = sbrTuningTable[idx].noiseFloorOffset;

    config->ana_max_level = sbrTuningTable[idx].noiseMaxLevel;
    config->stereoMode = sbrTuningTable[idx].stereoMode;
    config->freqScale = sbrTuningTable[idx].freqScale;

    if (numChannels == 1) {
      /* stereo case */
      switch (core) {
        case AOT_AAC_LC:
          if (bitRate <= (useSpeechConfig ? 24000U : 20000U)) {
            config->freq_res_fixfix[0] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for
                                                          non-split frames */
            config->freq_res_fixfix[1] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for split
                                                          frames */
          }
          break;
        case AOT_ER_AAC_ELD:
          if (bitRate < 36000)
            config->freq_res_fixfix[1] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for split
                                                          frames */
          if (bitRate < 26000) {
            config->freq_res_fixfix[0] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for
                                                          non-split frames */
            config->fResTransIsLow =
                1; /* for transient frames, set low frequency resolution */
          }
          break;
        default:
          break;
      }
    } else {
      /* stereo case */
      switch (core) {
        case AOT_AAC_LC:
          if (bitRate <= 28000) {
            config->freq_res_fixfix[0] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for
                                                          non-split frames */
            config->freq_res_fixfix[1] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for split
                                                          frames */
          }
          break;
        case AOT_ER_AAC_ELD:
          if (bitRate < 72000) {
            config->freq_res_fixfix[1] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for split
                                                          frames */
          }
          if (bitRate < 52000) {
            config->freq_res_fixfix[0] = FREQ_RES_LOW; /* set low frequency
                                                          resolution for
                                                          non-split frames */
            config->fResTransIsLow =
                1; /* for transient frames, set low frequency resolution */
          }
          break;
        default:
          break;
      }
      if (bitRate <= 28000) {
        /*
          additionally restrict frequency resolution in FIXFIX frames
          to further reduce SBR payload size */
        config->freq_res_fixfix[0] = FREQ_RES_LOW;
        config->freq_res_fixfix[1] = FREQ_RES_LOW;
      }
    }

    /* adjust usage of parametric coding dependent on bitrate and speech config
     * flag */
    if (useSpeechConfig) config->parametricCoding = 0;

    if (core == AOT_ER_AAC_ELD) {
      if (bitRate < 28000) config->init_amp_res_FF = SBR_AMP_RES_3_0;
      config->SendHeaderDataTime = -1;
    }

    if (numChannels == 1) {
      if (bitRate < 16000) {
        config->parametricCoding = 0;
      }
    } else {
      if (bitRate < 20000) {
        config->parametricCoding = 0;
      }
    }

    config->useSpeechConfig = useSpeechConfig;

    /* PS settings */
    config->bParametricStereo = bParametricStereo;

    return 1;
  } else {
    return 0;
  }
}

/*****************************************************************************

 functionname: FDKsbrEnc_InitializeSbrDefaults
 description:  initializes the SBR configuration
 returns:      error status
 input:        - core codec type,
               - factor of SBR to core frame length,
               - core frame length
 output:       initialized SBR configuration

*****************************************************************************/
static UINT FDKsbrEnc_InitializeSbrDefaults(sbrConfigurationPtr config,
                                            INT downSampleFactor,
                                            UINT codecGranuleLen,
                                            const INT isLowDelay) {
  if ((downSampleFactor < 1 || downSampleFactor > 2) ||
      (codecGranuleLen * downSampleFactor > 64 * 32))
    return (0); /* error */

  config->SendHeaderDataTime = 1000;
  config->useWaveCoding = 0;
  config->crcSbr = 0;
  config->dynBwSupported = 1;
  if (isLowDelay)
    config->tran_thr = 6000;
  else
    config->tran_thr = 13000;

  config->parametricCoding = 1;

  config->sbrFrameSize = codecGranuleLen * downSampleFactor;
  config->downSampleFactor = downSampleFactor;

  /* sbr default parameters */
  config->sbr_data_extra = 0;
  config->amp_res = SBR_AMP_RES_3_0;
  config->tran_fc = 0;
  config->tran_det_mode = 1;
  config->spread = 1;
  config->stat = 0;
  config->e = 1;
  config->deltaTAcrossFrames = 1;
  config->dF_edge_1stEnv = FL2FXCONST_DBL(0.3f);
  config->dF_edge_incr = FL2FXCONST_DBL(0.3f);

  config->sbr_invf_mode = INVF_SWITCHED;
  config->sbr_xpos_mode = XPOS_LC;
  config->sbr_xpos_ctrl = SBR_XPOS_CTRL_DEFAULT;
  config->sbr_xpos_level = 0;
  config->useSaPan = 0;
  config->dynBwEnabled = 0;

  /* the following parameters are overwritten by the
     FDKsbrEnc_AdjustSbrSettings() function since they are included in the
     tuning table */
  config->stereoMode = SBR_SWITCH_LRC;
  config->ana_max_level = 6;
  config->noiseFloorOffset = 0;
  config->startFreq = 5; /*  5.9 respectively  6.0 kHz at fs = 44.1/48 kHz */
  config->stopFreq = 9;  /* 16.2 respectively 16.8 kHz at fs = 44.1/48 kHz */
  config->freq_res_fixfix[0] = FREQ_RES_HIGH; /* non-split case */
  config->freq_res_fixfix[1] = FREQ_RES_HIGH; /* split case */
  config->fResTransIsLow = 0; /* for transient frames, set variable frequency
                                 resolution according to freqResTable */

  /* header_extra_1 */
  config->freqScale = SBR_FREQ_SCALE_DEFAULT;
  config->alterScale = SBR_ALTER_SCALE_DEFAULT;
  config->sbr_noise_bands = SBR_NOISE_BANDS_DEFAULT;

  /* header_extra_2 */
  config->sbr_limiter_bands = SBR_LIMITER_BANDS_DEFAULT;
  config->sbr_limiter_gains = SBR_LIMITER_GAINS_DEFAULT;
  config->sbr_interpol_freq = SBR_INTERPOL_FREQ_DEFAULT;
  config->sbr_smoothing_length = SBR_SMOOTHING_LENGTH_DEFAULT;

  return 1;
}

/*****************************************************************************

 functionname: DeleteEnvChannel
 description:  frees memory of one SBR channel
 returns:      -
 input:        handle of channel
 output:       released handle

*****************************************************************************/
static void deleteEnvChannel(HANDLE_ENV_CHANNEL hEnvCut) {
  if (hEnvCut) {
    FDKsbrEnc_DeleteTonCorrParamExtr(&hEnvCut->TonCorr);

    FDKsbrEnc_deleteExtractSbrEnvelope(&hEnvCut->sbrExtractEnvelope);
  }
}

/*****************************************************************************

 functionname: sbrEncoder_ChannelClose
 description:  close the channel coding handle
 returns:
 input:        phSbrChannel
 output:

*****************************************************************************/
static void sbrEncoder_ChannelClose(HANDLE_SBR_CHANNEL hSbrChannel) {
  if (hSbrChannel != NULL) {
    deleteEnvChannel(&hSbrChannel->hEnvChannel);
  }
}

/*****************************************************************************

 functionname: sbrEncoder_ElementClose
 description:  close the channel coding handle
 returns:
 input:        phSbrChannel
 output:

*****************************************************************************/
static void sbrEncoder_ElementClose(HANDLE_SBR_ELEMENT *phSbrElement) {
  HANDLE_SBR_ELEMENT hSbrElement = *phSbrElement;

  if (hSbrElement != NULL) {
    if (hSbrElement->sbrConfigData.v_k_master)
      FreeRam_Sbr_v_k_master(&hSbrElement->sbrConfigData.v_k_master);
    if (hSbrElement->sbrConfigData.freqBandTable[LO])
      FreeRam_Sbr_freqBandTableLO(
          &hSbrElement->sbrConfigData.freqBandTable[LO]);
    if (hSbrElement->sbrConfigData.freqBandTable[HI])
      FreeRam_Sbr_freqBandTableHI(
          &hSbrElement->sbrConfigData.freqBandTable[HI]);

    FreeRam_SbrElement(phSbrElement);
  }
  return;
}

void sbrEncoder_Close(HANDLE_SBR_ENCODER *phSbrEncoder) {
  HANDLE_SBR_ENCODER hSbrEncoder = *phSbrEncoder;

  if (hSbrEncoder != NULL) {
    int el, ch;

    for (el = 0; el < (8); el++) {
      if (hSbrEncoder->sbrElement[el] != NULL) {
        sbrEncoder_ElementClose(&hSbrEncoder->sbrElement[el]);
      }
    }

    /* Close sbr Channels */
    for (ch = 0; ch < (8); ch++) {
      if (hSbrEncoder->pSbrChannel[ch]) {
        sbrEncoder_ChannelClose(hSbrEncoder->pSbrChannel[ch]);
        FreeRam_SbrChannel(&hSbrEncoder->pSbrChannel[ch]);
      }

      if (hSbrEncoder->QmfAnalysis[ch].FilterStates)
        FreeRam_Sbr_QmfStatesAnalysis(
            (FIXP_QAS **)&hSbrEncoder->QmfAnalysis[ch].FilterStates);
    }

    if (hSbrEncoder->hParametricStereo)
      PSEnc_Destroy(&hSbrEncoder->hParametricStereo);
    if (hSbrEncoder->qmfSynthesisPS.FilterStates)
      FreeRam_PsQmfStatesSynthesis(
          (FIXP_DBL **)&hSbrEncoder->qmfSynthesisPS.FilterStates);

    /* Release Overlay */
    if (hSbrEncoder->pSBRdynamic_RAM)
      FreeRam_SbrDynamic_RAM((FIXP_DBL **)&hSbrEncoder->pSBRdynamic_RAM);

    FreeRam_SbrEncoder(phSbrEncoder);
  }
}

/*****************************************************************************

 functionname: updateFreqBandTable
 description:  updates vk_master
 returns:      -
 input:        config handle
 output:       error info

*****************************************************************************/
static INT updateFreqBandTable(HANDLE_SBR_CONFIG_DATA sbrConfigData,
                               HANDLE_SBR_HEADER_DATA sbrHeaderData,
                               const INT downSampleFactor) {
  INT k0, k2;

  if (FDKsbrEnc_FindStartAndStopBand(
          sbrConfigData->sampleFreq,
          sbrConfigData->sampleFreq >> (downSampleFactor - 1),
          sbrConfigData->noQmfBands, sbrHeaderData->sbr_start_frequency,
          sbrHeaderData->sbr_stop_frequency, &k0, &k2))
    return (1);

  if (FDKsbrEnc_UpdateFreqScale(
          sbrConfigData->v_k_master, &sbrConfigData->num_Master, k0, k2,
          sbrHeaderData->freqScale, sbrHeaderData->alterScale))
    return (1);

  sbrHeaderData->sbr_xover_band = 0;

  if (FDKsbrEnc_UpdateHiRes(sbrConfigData->freqBandTable[HI],
                            &sbrConfigData->nSfb[HI], sbrConfigData->v_k_master,
                            sbrConfigData->num_Master,
                            &sbrHeaderData->sbr_xover_band))
    return (1);

  FDKsbrEnc_UpdateLoRes(
      sbrConfigData->freqBandTable[LO], &sbrConfigData->nSfb[LO],
      sbrConfigData->freqBandTable[HI], sbrConfigData->nSfb[HI]);

  sbrConfigData->xOverFreq =
      (sbrConfigData->freqBandTable[LOW_RES][0] * sbrConfigData->sampleFreq /
           sbrConfigData->noQmfBands +
       1) >>
      1;

  return (0);
}

/*****************************************************************************

 functionname: resetEnvChannel
 description:  resets parameters and allocates memory
 returns:      error status
 input:
 output:       hEnv

*****************************************************************************/
static INT resetEnvChannel(HANDLE_SBR_CONFIG_DATA sbrConfigData,
                           HANDLE_SBR_HEADER_DATA sbrHeaderData,
                           HANDLE_ENV_CHANNEL hEnv) {
  /* note !!! hEnv->encEnvData.noOfnoisebands will be updated later in function
   * FDKsbrEnc_extractSbrEnvelope !!!*/
  hEnv->TonCorr.sbrNoiseFloorEstimate.noiseBands =
      sbrHeaderData->sbr_noise_bands;

  if (FDKsbrEnc_ResetTonCorrParamExtr(
          &hEnv->TonCorr, sbrConfigData->xposCtrlSwitch,
          sbrConfigData->freqBandTable[HI][0], sbrConfigData->v_k_master,
          sbrConfigData->num_Master, sbrConfigData->sampleFreq,
          sbrConfigData->freqBandTable, sbrConfigData->nSfb,
          sbrConfigData->noQmfBands))
    return (1);

  hEnv->sbrCodeNoiseFloor.nSfb[LO] =
      hEnv->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;
  hEnv->sbrCodeNoiseFloor.nSfb[HI] =
      hEnv->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;

  hEnv->sbrCodeEnvelope.nSfb[LO] = sbrConfigData->nSfb[LO];
  hEnv->sbrCodeEnvelope.nSfb[HI] = sbrConfigData->nSfb[HI];

  hEnv->encEnvData.noHarmonics = sbrConfigData->nSfb[HI];

  hEnv->sbrCodeEnvelope.upDate = 0;
  hEnv->sbrCodeNoiseFloor.upDate = 0;

  return (0);
}

/* ****************************** FDKsbrEnc_SbrGetXOverFreq
 * ******************************/
/**
 * @fn
 * @brief       calculates the closest possible crossover frequency
 * @return      the crossover frequency SBR accepts
 *
 */
static INT FDKsbrEnc_SbrGetXOverFreq(
    HANDLE_SBR_ELEMENT hEnv, /*!< handle to SBR encoder instance */
    INT xoverFreq) /*!< from core coder suggested crossover frequency */
{
  INT band;
  INT lastDiff, newDiff;
  INT cutoffSb;

  UCHAR *RESTRICT pVKMaster = hEnv->sbrConfigData.v_k_master;

  /* Check if there is a matching cutoff frequency in the master table */
  cutoffSb = (4 * xoverFreq * hEnv->sbrConfigData.noQmfBands /
                  hEnv->sbrConfigData.sampleFreq +
              1) >>
             1;
  lastDiff = cutoffSb;
  for (band = 0; band < hEnv->sbrConfigData.num_Master; band++) {
    newDiff = fixp_abs((INT)pVKMaster[band] - cutoffSb);

    if (newDiff >= lastDiff) {
      band--;
      break;
    }

    lastDiff = newDiff;
  }

  return ((pVKMaster[band] * hEnv->sbrConfigData.sampleFreq /
               hEnv->sbrConfigData.noQmfBands +
           1) >>
          1);
}

/*****************************************************************************

 functionname: FDKsbrEnc_EnvEncodeFrame
 description: performs the sbr envelope calculation for one element
 returns:
 input:
 output:

*****************************************************************************/
INT FDKsbrEnc_EnvEncodeFrame(
    HANDLE_SBR_ENCODER hEnvEncoder, int iElement,
    INT_PCM *samples,    /*!< time samples, always deinterleaved */
    UINT samplesBufSize, /*!< time buffer channel stride */
    UINT *sbrDataBits,   /*!< Size of SBR payload  */
    UCHAR *sbrData,      /*!< SBR payload  */
    int clearOutput      /*!< Do not consider any input signal */
) {
  HANDLE_SBR_ELEMENT hSbrElement = NULL;
  FDK_CRCINFO crcInfo;
  INT crcReg;
  INT ch;
  INT band;
  INT cutoffSb;
  INT newXOver;

  if (hEnvEncoder == NULL) return -1;

  hSbrElement = hEnvEncoder->sbrElement[iElement];

  if (hSbrElement == NULL) return -1;

  /* header bitstream handling */
  HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData = &hSbrElement->sbrBitstreamData;

  INT psHeaderActive = 0;
  sbrBitstreamData->HeaderActive = 0;

  /* Anticipate PS header because of internal PS bitstream delay in order to be
   * in sync with SBR header. */
  if (sbrBitstreamData->CountSendHeaderData ==
      (sbrBitstreamData->NrSendHeaderData - 1)) {
    psHeaderActive = 1;
  }

  /* Signal SBR header to be written into bitstream */
  if (sbrBitstreamData->CountSendHeaderData == 0) {
    sbrBitstreamData->HeaderActive = 1;
  }

  /* Increment header interval counter */
  if (sbrBitstreamData->NrSendHeaderData == 0) {
    sbrBitstreamData->CountSendHeaderData = 1;
  } else {
    if (sbrBitstreamData->CountSendHeaderData >= 0) {
      sbrBitstreamData->CountSendHeaderData++;
      sbrBitstreamData->CountSendHeaderData %=
          sbrBitstreamData->NrSendHeaderData;
    }
  }

  if (hSbrElement->CmonData.dynBwEnabled) {
    INT i;
    for (i = 4; i > 0; i--)
      hSbrElement->dynXOverFreqDelay[i] = hSbrElement->dynXOverFreqDelay[i - 1];

    hSbrElement->dynXOverFreqDelay[0] = hSbrElement->CmonData.dynXOverFreqEnc;
    if (hSbrElement->dynXOverFreqDelay[1] > hSbrElement->dynXOverFreqDelay[2])
      newXOver = hSbrElement->dynXOverFreqDelay[2];
    else
      newXOver = hSbrElement->dynXOverFreqDelay[1];

    /* has the crossover frequency changed? */
    if (hSbrElement->sbrConfigData.dynXOverFreq != newXOver) {
      /* get corresponding master band */
      cutoffSb = ((4 * newXOver * hSbrElement->sbrConfigData.noQmfBands /
                   hSbrElement->sbrConfigData.sampleFreq) +
                  1) >>
                 1;

      for (band = 0; band < hSbrElement->sbrConfigData.num_Master; band++) {
        if (cutoffSb == hSbrElement->sbrConfigData.v_k_master[band]) break;
      }
      FDK_ASSERT(band < hSbrElement->sbrConfigData.num_Master);

      hSbrElement->sbrConfigData.dynXOverFreq = newXOver;
      hSbrElement->sbrHeaderData.sbr_xover_band = band;
      hSbrElement->sbrBitstreamData.HeaderActive = 1;
      psHeaderActive = 1; /* ps header is one frame delayed */

      /*
        update vk_master table
      */
      if (updateFreqBandTable(&hSbrElement->sbrConfigData,
                              &hSbrElement->sbrHeaderData,
                              hEnvEncoder->downSampleFactor))
        return (1);

      /* reset SBR channels */
      INT nEnvCh = hSbrElement->sbrConfigData.nChannels;
      for (ch = 0; ch < nEnvCh; ch++) {
        if (resetEnvChannel(&hSbrElement->sbrConfigData,
                            &hSbrElement->sbrHeaderData,
                            &hSbrElement->sbrChannel[ch]->hEnvChannel))
          return (1);
      }
    }
  }

  /*
    allocate space for dummy header and crc
  */
  crcReg = FDKsbrEnc_InitSbrBitstream(
      &hSbrElement->CmonData,
      hSbrElement->payloadDelayLine[hEnvEncoder->nBitstrDelay],
      MAX_PAYLOAD_SIZE * sizeof(UCHAR), &crcInfo,
      hSbrElement->sbrConfigData.sbrSyntaxFlags);

  /* Temporal Envelope Data */
  SBR_FRAME_TEMP_DATA _fData;
  SBR_FRAME_TEMP_DATA *fData = &_fData;
  SBR_ENV_TEMP_DATA eData[MAX_NUM_CHANNELS];

  /* Init Temporal Envelope Data */
  {
    int i;

    FDKmemclear(&eData[0], sizeof(SBR_ENV_TEMP_DATA));
    FDKmemclear(&eData[1], sizeof(SBR_ENV_TEMP_DATA));
    FDKmemclear(fData, sizeof(SBR_FRAME_TEMP_DATA));

    for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) fData->res[i] = FREQ_RES_HIGH;
  }

  if (!clearOutput) {
    /*
     * Transform audio data into QMF domain
     */
    for (ch = 0; ch < hSbrElement->sbrConfigData.nChannels; ch++) {
      HANDLE_ENV_CHANNEL h_envChan = &hSbrElement->sbrChannel[ch]->hEnvChannel;
      HANDLE_SBR_EXTRACT_ENVELOPE sbrExtrEnv = &h_envChan->sbrExtractEnvelope;

      if (hSbrElement->elInfo.fParametricStereo == 0) {
        QMF_SCALE_FACTOR tmpScale;
        FIXP_DBL **pQmfReal, **pQmfImag;
        C_AALLOC_SCRATCH_START(qmfWorkBuffer, FIXP_DBL, 64 * 2)

        /* Obtain pointers to QMF buffers. */
        pQmfReal = sbrExtrEnv->rBuffer;
        pQmfImag = sbrExtrEnv->iBuffer;

        qmfAnalysisFiltering(
            hSbrElement->hQmfAnalysis[ch], pQmfReal, pQmfImag, &tmpScale,
            samples + hSbrElement->elInfo.ChannelIndex[ch] * samplesBufSize, 0,
            1, qmfWorkBuffer);

        h_envChan->qmfScale = tmpScale.lb_scale + 7;

        C_AALLOC_SCRATCH_END(qmfWorkBuffer, FIXP_DBL, 64 * 2)

      } /* fParametricStereo == 0 */

      /*
        Parametric Stereo processing
      */
      if (hSbrElement->elInfo.fParametricStereo) {
        INT error = noError;

        /* Limit Parametric Stereo to one instance */
        FDK_ASSERT(ch == 0);

        if (error == noError) {
          /* parametric stereo processing:
             - input:
               o left and right time domain samples
             - processing:
               o stereo qmf analysis
               o stereo hybrid analysis
               o ps parameter extraction
               o downmix + hybrid synthesis
             - output:
               o downmixed qmf data is written to sbrExtrEnv->rBuffer and
             sbrExtrEnv->iBuffer
          */
          SCHAR qmfScale;
          INT_PCM *pSamples[2] = {
              samples + hSbrElement->elInfo.ChannelIndex[0] * samplesBufSize,
              samples + hSbrElement->elInfo.ChannelIndex[1] * samplesBufSize};
          error = FDKsbrEnc_PSEnc_ParametricStereoProcessing(
              hEnvEncoder->hParametricStereo, pSamples, samplesBufSize,
              hSbrElement->hQmfAnalysis, sbrExtrEnv->rBuffer,
              sbrExtrEnv->iBuffer,
              samples + hSbrElement->elInfo.ChannelIndex[ch] * samplesBufSize,
              &hEnvEncoder->qmfSynthesisPS, &qmfScale, psHeaderActive);
          h_envChan->qmfScale = (int)qmfScale;
        }

      } /* if (hEnvEncoder->hParametricStereo) */

      /*

         Extract Envelope relevant things from QMF data

      */
      FDKsbrEnc_extractSbrEnvelope1(&hSbrElement->sbrConfigData,
                                    &hSbrElement->sbrHeaderData,
                                    &hSbrElement->sbrBitstreamData, h_envChan,
                                    &hSbrElement->CmonData, &eData[ch], fData);

    } /* hEnvEncoder->sbrConfigData.nChannels */
  }

  /*
     Process Envelope relevant things and calculate envelope data and write
     payload
  */
  FDKsbrEnc_extractSbrEnvelope2(
      &hSbrElement->sbrConfigData, &hSbrElement->sbrHeaderData,
      (hSbrElement->elInfo.fParametricStereo) ? hEnvEncoder->hParametricStereo
                                              : NULL,
      &hSbrElement->sbrBitstreamData, &hSbrElement->sbrChannel[0]->hEnvChannel,
      (hSbrElement->sbrConfigData.stereoMode != SBR_MONO)
          ? &hSbrElement->sbrChannel[1]->hEnvChannel
          : NULL,
      &hSbrElement->CmonData, eData, fData, clearOutput);

  hSbrElement->sbrBitstreamData.rightBorderFIX = 0;

  /*
    format payload, calculate crc
  */
  FDKsbrEnc_AssembleSbrBitstream(&hSbrElement->CmonData, &crcInfo, crcReg,
                                 hSbrElement->sbrConfigData.sbrSyntaxFlags);

  /*
    save new payload, set to zero length if greater than MAX_PAYLOAD_SIZE
  */
  hSbrElement->payloadDelayLineSize[hEnvEncoder->nBitstrDelay] =
      FDKgetValidBits(&hSbrElement->CmonData.sbrBitbuf);

  if (hSbrElement->payloadDelayLineSize[hEnvEncoder->nBitstrDelay] >
      (MAX_PAYLOAD_SIZE << 3))
    hSbrElement->payloadDelayLineSize[hEnvEncoder->nBitstrDelay] = 0;

  /* While filling the Delay lines, sbrData is NULL */
  if (sbrData) {
    *sbrDataBits = hSbrElement->payloadDelayLineSize[0];
    FDKmemcpy(sbrData, hSbrElement->payloadDelayLine[0],
              (hSbrElement->payloadDelayLineSize[0] + 7) >> 3);
  }

  /* delay header active flag */
  if (hSbrElement->sbrBitstreamData.HeaderActive == 1) {
    hSbrElement->sbrBitstreamData.HeaderActiveDelay =
        1 + hEnvEncoder->nBitstrDelay;
  } else {
    if (hSbrElement->sbrBitstreamData.HeaderActiveDelay > 0) {
      hSbrElement->sbrBitstreamData.HeaderActiveDelay--;
    }
  }

  return (0);
}

/*****************************************************************************

 functionname: FDKsbrEnc_Downsample
 description: performs downsampling and delay compensation of the core path
 returns:
 input:
 output:

*****************************************************************************/
INT FDKsbrEnc_Downsample(
    HANDLE_SBR_ENCODER hSbrEncoder,
    INT_PCM *samples,    /*!< time samples, always deinterleaved */
    UINT samplesBufSize, /*!< time buffer size per channel */
    UINT numChannels,    /*!< number of channels */
    UINT *sbrDataBits,   /*!< Size of SBR payload  */
    UCHAR *sbrData,      /*!< SBR payload  */
    int clearOutput      /*!< Do not consider any input signal */
) {
  HANDLE_SBR_ELEMENT hSbrElement = NULL;
  INT nOutSamples;
  int el;
  if (hSbrEncoder->downSampleFactor > 1) {
    /* Do downsampling */

    /* Loop over elements (LFE is handled later) */
    for (el = 0; el < hSbrEncoder->noElements; el++) {
      hSbrElement = hSbrEncoder->sbrElement[el];
      if (hSbrEncoder->sbrElement[el] != NULL) {
        if (hSbrEncoder->downsamplingMethod == SBRENC_DS_TIME) {
          int ch;
          int nChannels = hSbrElement->sbrConfigData.nChannels;

          for (ch = 0; ch < nChannels; ch++) {
            FDKaacEnc_Downsample(
                &hSbrElement->sbrChannel[ch]->downSampler,
                samples +
                    hSbrElement->elInfo.ChannelIndex[ch] * samplesBufSize +
                    hSbrEncoder->bufferOffset / numChannels,
                hSbrElement->sbrConfigData.frameSize,
                samples + hSbrElement->elInfo.ChannelIndex[ch] * samplesBufSize,
                &nOutSamples);
          }
        }
      }
    }

    /* Handle LFE (if existing) */
    if (hSbrEncoder->lfeChIdx != -1) { /* lfe downsampler */
      FDKaacEnc_Downsample(&hSbrEncoder->lfeDownSampler,
                           samples + hSbrEncoder->lfeChIdx * samplesBufSize +
                               hSbrEncoder->bufferOffset / numChannels,
                           hSbrEncoder->frameSize,
                           samples + hSbrEncoder->lfeChIdx * samplesBufSize,
                           &nOutSamples);
    }
  } else {
    /* No downsampling. Still, some buffer shifting for correct delay */
    int samples2Copy = hSbrEncoder->frameSize;
    if (hSbrEncoder->bufferOffset / (int)numChannels < samples2Copy) {
      for (int c = 0; c < (int)numChannels; c++) {
        /* Do memmove while taking care of overlapping memory areas. (memcpy
           does not necessarily take care) Distinguish between oeverlapping and
           non overlapping version due to reasons of complexity. */
        FDKmemmove(samples + c * samplesBufSize,
                   samples + c * samplesBufSize +
                       hSbrEncoder->bufferOffset / numChannels,
                   samples2Copy * sizeof(INT_PCM));
      }
    } else {
      for (int c = 0; c < (int)numChannels; c++) {
        /* Simple memcpy since the memory areas are not overlapping */
        FDKmemcpy(samples + c * samplesBufSize,
                  samples + c * samplesBufSize +
                      hSbrEncoder->bufferOffset / numChannels,
                  samples2Copy * sizeof(INT_PCM));
      }
    }
  }

  return 0;
}

/*****************************************************************************

 functionname: createEnvChannel
 description:  initializes parameters and allocates memory
 returns:      error status
 input:
 output:       hEnv

*****************************************************************************/

static INT createEnvChannel(HANDLE_ENV_CHANNEL hEnv, INT channel,
                            UCHAR *dynamic_RAM) {
  FDKmemclear(hEnv, sizeof(struct ENV_CHANNEL));

  if (FDKsbrEnc_CreateTonCorrParamExtr(&hEnv->TonCorr, channel)) {
    return (1);
  }

  if (FDKsbrEnc_CreateExtractSbrEnvelope(&hEnv->sbrExtractEnvelope, channel,
                                         /*chan*/ 0, dynamic_RAM)) {
    return (1);
  }

  return 0;
}

/*****************************************************************************

 functionname: initEnvChannel
 description:  initializes parameters
 returns:      error status
 input:
 output:

*****************************************************************************/
static INT initEnvChannel(HANDLE_SBR_CONFIG_DATA sbrConfigData,
                          HANDLE_SBR_HEADER_DATA sbrHeaderData,
                          HANDLE_ENV_CHANNEL hEnv, sbrConfigurationPtr params,
                          ULONG statesInitFlag, INT chanInEl,
                          UCHAR *dynamic_RAM) {
  int frameShift, tran_off = 0;
  INT e;
  INT tran_fc;
  INT timeSlots, timeStep, startIndex;
  INT noiseBands[2] = {3, 3};

  e = 1 << params->e;

  FDK_ASSERT(params->e >= 0);

  hEnv->encEnvData.freq_res_fixfix[0] = params->freq_res_fixfix[0];
  hEnv->encEnvData.freq_res_fixfix[1] = params->freq_res_fixfix[1];
  hEnv->encEnvData.fResTransIsLow = params->fResTransIsLow;

  hEnv->fLevelProtect = 0;

  hEnv->encEnvData.ldGrid =
      (sbrConfigData->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) ? 1 : 0;

  hEnv->encEnvData.sbr_xpos_mode = (XPOS_MODE)params->sbr_xpos_mode;

  if (hEnv->encEnvData.sbr_xpos_mode == XPOS_SWITCHED) {
    /*
       no other type than XPOS_MDCT or XPOS_SPEECH allowed,
       but enable switching
    */
    sbrConfigData->switchTransposers = TRUE;
    hEnv->encEnvData.sbr_xpos_mode = XPOS_MDCT;
  } else {
    sbrConfigData->switchTransposers = FALSE;
  }

  hEnv->encEnvData.sbr_xpos_ctrl = params->sbr_xpos_ctrl;

  /* extended data */
  if (params->parametricCoding) {
    hEnv->encEnvData.extended_data = 1;
  } else {
    hEnv->encEnvData.extended_data = 0;
  }

  hEnv->encEnvData.extension_size = 0;

  startIndex = QMF_FILTER_PROTOTYPE_SIZE - sbrConfigData->noQmfBands;

  switch (params->sbrFrameSize) {
    case 2304:
      timeSlots = 18;
      break;
    case 2048:
    case 1024:
    case 512:
      timeSlots = 16;
      break;
    case 1920:
    case 960:
    case 480:
      timeSlots = 15;
      break;
    case 1152:
      timeSlots = 9;
      break;
    default:
      return (1); /* Illegal frame size */
  }

  timeStep = sbrConfigData->noQmfSlots / timeSlots;

  if (FDKsbrEnc_InitTonCorrParamExtr(
          params->sbrFrameSize, &hEnv->TonCorr, sbrConfigData, timeSlots,
          params->sbr_xpos_ctrl, params->ana_max_level,
          sbrHeaderData->sbr_noise_bands, params->noiseFloorOffset,
          params->useSpeechConfig))
    return (1);

  hEnv->encEnvData.noOfnoisebands =
      hEnv->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;

  noiseBands[0] = hEnv->encEnvData.noOfnoisebands;
  noiseBands[1] = hEnv->encEnvData.noOfnoisebands;

  hEnv->encEnvData.sbr_invf_mode = (INVF_MODE)params->sbr_invf_mode;

  if (hEnv->encEnvData.sbr_invf_mode == INVF_SWITCHED) {
    hEnv->encEnvData.sbr_invf_mode = INVF_MID_LEVEL;
    hEnv->TonCorr.switchInverseFilt = TRUE;
  } else {
    hEnv->TonCorr.switchInverseFilt = FALSE;
  }

  tran_fc = params->tran_fc;

  if (tran_fc == 0) {
    tran_fc = fixMin(
        5000, FDKsbrEnc_getSbrStartFreqRAW(sbrHeaderData->sbr_start_frequency,
                                           params->codecSettings.sampleFreq));
  }

  tran_fc =
      (tran_fc * 4 * sbrConfigData->noQmfBands / sbrConfigData->sampleFreq +
       1) >>
      1;

  if (sbrConfigData->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    frameShift = LD_PRETRAN_OFF;
    tran_off = LD_PRETRAN_OFF + FRAME_MIDDLE_SLOT_512LD * timeStep;
  } else {
    frameShift = 0;
    switch (timeSlots) {
      /* The factor of 2 is by definition. */
      case NUMBER_TIME_SLOTS_2048:
        tran_off = 8 + FRAME_MIDDLE_SLOT_2048 * timeStep;
        break;
      case NUMBER_TIME_SLOTS_1920:
        tran_off = 7 + FRAME_MIDDLE_SLOT_1920 * timeStep;
        break;
      default:
        return 1;
    }
  }
  if (FDKsbrEnc_InitExtractSbrEnvelope(
          &hEnv->sbrExtractEnvelope, sbrConfigData->noQmfSlots,
          sbrConfigData->noQmfBands, startIndex, timeSlots, timeStep, tran_off,
          statesInitFlag, chanInEl, dynamic_RAM, sbrConfigData->sbrSyntaxFlags))
    return (1);

  if (FDKsbrEnc_InitSbrCodeEnvelope(&hEnv->sbrCodeEnvelope, sbrConfigData->nSfb,
                                    params->deltaTAcrossFrames,
                                    params->dF_edge_1stEnv,
                                    params->dF_edge_incr))
    return (1);

  if (FDKsbrEnc_InitSbrCodeEnvelope(&hEnv->sbrCodeNoiseFloor, noiseBands,
                                    params->deltaTAcrossFrames, 0, 0))
    return (1);

  if (FDKsbrEnc_InitSbrHuffmanTables(&hEnv->encEnvData, &hEnv->sbrCodeEnvelope,
                                     &hEnv->sbrCodeNoiseFloor,
                                     sbrHeaderData->sbr_amp_res))
    return (1);

  FDKsbrEnc_initFrameInfoGenerator(
      &hEnv->SbrEnvFrame, params->spread, e, params->stat, timeSlots,
      hEnv->encEnvData.freq_res_fixfix, hEnv->encEnvData.fResTransIsLow,
      hEnv->encEnvData.ldGrid);

  if (sbrConfigData->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY)

  {
    INT bandwidth_qmf_slot =
        (sbrConfigData->sampleFreq >> 1) / (sbrConfigData->noQmfBands);
    if (FDKsbrEnc_InitSbrFastTransientDetector(
            &hEnv->sbrFastTransientDetector, sbrConfigData->noQmfSlots,
            bandwidth_qmf_slot, sbrConfigData->noQmfBands,
            sbrConfigData->freqBandTable[0][0]))
      return (1);
  }

  /* The transient detector has to be initialized also if the fast transient
     detector was active, because the values from the transient detector
     structure are used. */
  if (FDKsbrEnc_InitSbrTransientDetector(
          &hEnv->sbrTransientDetector, sbrConfigData->sbrSyntaxFlags,
          sbrConfigData->frameSize, sbrConfigData->sampleFreq, params, tran_fc,
          sbrConfigData->noQmfSlots, sbrConfigData->noQmfBands,
          hEnv->sbrExtractEnvelope.YBufferWriteOffset,
          hEnv->sbrExtractEnvelope.YBufferSzShift, frameShift, tran_off))
    return (1);

  sbrConfigData->xposCtrlSwitch = params->sbr_xpos_ctrl;

  hEnv->encEnvData.noHarmonics = sbrConfigData->nSfb[HI];
  hEnv->encEnvData.addHarmonicFlag = 0;

  return (0);
}

INT sbrEncoder_Open(HANDLE_SBR_ENCODER *phSbrEncoder, INT nElements,
                    INT nChannels, INT supportPS) {
  INT i;
  INT errorStatus = 1;
  HANDLE_SBR_ENCODER hSbrEncoder = NULL;

  if (phSbrEncoder == NULL) {
    goto bail;
  }

  hSbrEncoder = GetRam_SbrEncoder();
  if (hSbrEncoder == NULL) {
    goto bail;
  }
  FDKmemclear(hSbrEncoder, sizeof(SBR_ENCODER));

  if (NULL ==
      (hSbrEncoder->pSBRdynamic_RAM = (UCHAR *)GetRam_SbrDynamic_RAM())) {
    goto bail;
  }
  hSbrEncoder->dynamicRam = hSbrEncoder->pSBRdynamic_RAM;

  /* Create SBR elements */
  for (i = 0; i < nElements; i++) {
    hSbrEncoder->sbrElement[i] = GetRam_SbrElement(i);
    if (hSbrEncoder->sbrElement[i] == NULL) {
      goto bail;
    }
    FDKmemclear(hSbrEncoder->sbrElement[i], sizeof(SBR_ELEMENT));
    hSbrEncoder->sbrElement[i]->sbrConfigData.freqBandTable[LO] =
        GetRam_Sbr_freqBandTableLO(i);
    hSbrEncoder->sbrElement[i]->sbrConfigData.freqBandTable[HI] =
        GetRam_Sbr_freqBandTableHI(i);
    hSbrEncoder->sbrElement[i]->sbrConfigData.v_k_master =
        GetRam_Sbr_v_k_master(i);
    if ((hSbrEncoder->sbrElement[i]->sbrConfigData.freqBandTable[LO] == NULL) ||
        (hSbrEncoder->sbrElement[i]->sbrConfigData.freqBandTable[HI] == NULL) ||
        (hSbrEncoder->sbrElement[i]->sbrConfigData.v_k_master == NULL)) {
      goto bail;
    }
  }

  /* Create SBR channels */
  for (i = 0; i < nChannels; i++) {
    hSbrEncoder->pSbrChannel[i] = GetRam_SbrChannel(i);
    if (hSbrEncoder->pSbrChannel[i] == NULL) {
      goto bail;
    }

    if (createEnvChannel(&hSbrEncoder->pSbrChannel[i]->hEnvChannel, i,
                         hSbrEncoder->dynamicRam)) {
      goto bail;
    }
  }

  /* Create QMF States */
  for (i = 0; i < fixMax(nChannels, (supportPS) ? 2 : 0); i++) {
    hSbrEncoder->QmfAnalysis[i].FilterStates = GetRam_Sbr_QmfStatesAnalysis(i);
    if (hSbrEncoder->QmfAnalysis[i].FilterStates == NULL) {
      goto bail;
    }
  }

  /* Create Parametric Stereo handle */
  if (supportPS) {
    if (PSEnc_Create(&hSbrEncoder->hParametricStereo)) {
      goto bail;
    }

    hSbrEncoder->qmfSynthesisPS.FilterStates = GetRam_PsQmfStatesSynthesis();
    if (hSbrEncoder->qmfSynthesisPS.FilterStates == NULL) {
      goto bail;
    }
  } /* supportPS */

  *phSbrEncoder = hSbrEncoder;

  errorStatus = 0;
  return errorStatus;

bail:
  /* Close SBR encoder instance */
  sbrEncoder_Close(&hSbrEncoder);
  return errorStatus;
}

static INT FDKsbrEnc_Reallocate(HANDLE_SBR_ENCODER hSbrEncoder,
                                SBR_ELEMENT_INFO elInfo[(8)],
                                const INT noElements) {
  INT totalCh = 0;
  INT totalQmf = 0;
  INT coreEl;
  INT el = -1;

  hSbrEncoder->lfeChIdx = -1; /* default value, until lfe found */

  for (coreEl = 0; coreEl < noElements; coreEl++) {
    /* SBR only handles SCE and CPE's */
    if (elInfo[coreEl].elType == ID_SCE || elInfo[coreEl].elType == ID_CPE) {
      el++;
    } else {
      if (elInfo[coreEl].elType == ID_LFE) {
        hSbrEncoder->lfeChIdx = elInfo[coreEl].ChannelIndex[0];
      }
      continue;
    }

    SBR_ELEMENT_INFO *pelInfo = &elInfo[coreEl];
    HANDLE_SBR_ELEMENT hSbrElement = hSbrEncoder->sbrElement[el];

    int ch;
    for (ch = 0; ch < pelInfo->nChannelsInEl; ch++) {
      hSbrElement->sbrChannel[ch] = hSbrEncoder->pSbrChannel[totalCh];
      totalCh++;
    }
    /* analysis QMF */
    for (ch = 0;
         ch < ((pelInfo->fParametricStereo) ? 2 : pelInfo->nChannelsInEl);
         ch++) {
      hSbrElement->elInfo.ChannelIndex[ch] = pelInfo->ChannelIndex[ch];
      hSbrElement->hQmfAnalysis[ch] = &hSbrEncoder->QmfAnalysis[totalQmf++];
    }

    /* Copy Element info */
    hSbrElement->elInfo.elType = pelInfo->elType;
    hSbrElement->elInfo.instanceTag = pelInfo->instanceTag;
    hSbrElement->elInfo.nChannelsInEl = pelInfo->nChannelsInEl;
    hSbrElement->elInfo.fParametricStereo = pelInfo->fParametricStereo;
    hSbrElement->elInfo.fDualMono = pelInfo->fDualMono;
  } /* coreEl */

  return 0;
}

/*****************************************************************************

 functionname: FDKsbrEnc_bsBufInit
 description:  initializes bitstream buffer
 returns:      initialized bitstream buffer in env encoder
 input:
 output:       hEnv

*****************************************************************************/
static INT FDKsbrEnc_bsBufInit(HANDLE_SBR_ELEMENT hSbrElement,
                               int nBitstrDelay) {
  UCHAR *bitstreamBuffer;

  /* initialize the bitstream buffer */
  bitstreamBuffer = hSbrElement->payloadDelayLine[nBitstrDelay];
  FDKinitBitStream(&hSbrElement->CmonData.sbrBitbuf, bitstreamBuffer,
                   MAX_PAYLOAD_SIZE * sizeof(UCHAR), 0, BS_WRITER);

  return (0);
}

/*****************************************************************************

 functionname: FDKsbrEnc_EnvInit
 description:  initializes parameters
 returns:      error status
 input:
 output:       hEnv

*****************************************************************************/
static INT FDKsbrEnc_EnvInit(HANDLE_SBR_ELEMENT hSbrElement,
                             sbrConfigurationPtr params, INT *coreBandWith,
                             AUDIO_OBJECT_TYPE aot, int nElement,
                             const int headerPeriod, ULONG statesInitFlag,
                             const SBRENC_DS_TYPE downsamplingMethod,
                             UCHAR *dynamic_RAM) {
  int ch, i;

  if ((params->codecSettings.nChannels < 1) ||
      (params->codecSettings.nChannels > MAX_NUM_CHANNELS)) {
    return (1);
  }

  /* init and set syntax flags */
  hSbrElement->sbrConfigData.sbrSyntaxFlags = 0;

  switch (aot) {
    case AOT_ER_AAC_ELD:
      hSbrElement->sbrConfigData.sbrSyntaxFlags |= SBR_SYNTAX_LOW_DELAY;
      break;
    default:
      break;
  }
  if (params->crcSbr) {
    hSbrElement->sbrConfigData.sbrSyntaxFlags |= SBR_SYNTAX_CRC;
  }

  hSbrElement->sbrConfigData.noQmfBands = 64 >> (2 - params->downSampleFactor);
  switch (hSbrElement->sbrConfigData.noQmfBands) {
    case 64:
      hSbrElement->sbrConfigData.noQmfSlots = params->sbrFrameSize >> 6;
      break;
    case 32:
      hSbrElement->sbrConfigData.noQmfSlots = params->sbrFrameSize >> 5;
      break;
    default:
      hSbrElement->sbrConfigData.noQmfSlots = params->sbrFrameSize >> 6;
      return (2);
  }

  /*
    now initialize sbrConfigData, sbrHeaderData and sbrBitstreamData,
  */
  hSbrElement->sbrConfigData.nChannels = params->codecSettings.nChannels;

  if (params->codecSettings.nChannels == 2) {
    if ((hSbrElement->elInfo.elType == ID_CPE) &&
        ((hSbrElement->elInfo.fDualMono == 1))) {
      hSbrElement->sbrConfigData.stereoMode = SBR_LEFT_RIGHT;
    } else {
      hSbrElement->sbrConfigData.stereoMode = params->stereoMode;
    }
  } else {
    hSbrElement->sbrConfigData.stereoMode = SBR_MONO;
  }

  hSbrElement->sbrConfigData.frameSize = params->sbrFrameSize;

  hSbrElement->sbrConfigData.sampleFreq =
      params->downSampleFactor * params->codecSettings.sampleFreq;

  hSbrElement->sbrBitstreamData.CountSendHeaderData = 0;
  if (params->SendHeaderDataTime > 0) {
    if (headerPeriod == -1) {
      hSbrElement->sbrBitstreamData.NrSendHeaderData = (INT)(
          params->SendHeaderDataTime * hSbrElement->sbrConfigData.sampleFreq /
          (1000 * hSbrElement->sbrConfigData.frameSize));
      hSbrElement->sbrBitstreamData.NrSendHeaderData =
          fixMax(hSbrElement->sbrBitstreamData.NrSendHeaderData, 1);
    } else {
      /* assure header period at least once per second */
      hSbrElement->sbrBitstreamData.NrSendHeaderData = fixMin(
          fixMax(headerPeriod, 1), (hSbrElement->sbrConfigData.sampleFreq /
                                    hSbrElement->sbrConfigData.frameSize));
    }
  } else {
    hSbrElement->sbrBitstreamData.NrSendHeaderData = 0;
  }

  hSbrElement->sbrHeaderData.sbr_data_extra = params->sbr_data_extra;
  hSbrElement->sbrBitstreamData.HeaderActive = 0;
  hSbrElement->sbrBitstreamData.rightBorderFIX = 0;
  hSbrElement->sbrHeaderData.sbr_start_frequency = params->startFreq;
  hSbrElement->sbrHeaderData.sbr_stop_frequency = params->stopFreq;
  hSbrElement->sbrHeaderData.sbr_xover_band = 0;
  hSbrElement->sbrHeaderData.sbr_lc_stereo_mode = 0;

  /* data_extra */
  if (params->sbr_xpos_ctrl != SBR_XPOS_CTRL_DEFAULT)
    hSbrElement->sbrHeaderData.sbr_data_extra = 1;

  hSbrElement->sbrHeaderData.sbr_amp_res = (AMP_RES)params->amp_res;
  hSbrElement->sbrConfigData.initAmpResFF = params->init_amp_res_FF;

  /* header_extra_1 */
  hSbrElement->sbrHeaderData.freqScale = params->freqScale;
  hSbrElement->sbrHeaderData.alterScale = params->alterScale;
  hSbrElement->sbrHeaderData.sbr_noise_bands = params->sbr_noise_bands;
  hSbrElement->sbrHeaderData.header_extra_1 = 0;

  if ((params->freqScale != SBR_FREQ_SCALE_DEFAULT) ||
      (params->alterScale != SBR_ALTER_SCALE_DEFAULT) ||
      (params->sbr_noise_bands != SBR_NOISE_BANDS_DEFAULT)) {
    hSbrElement->sbrHeaderData.header_extra_1 = 1;
  }

  /* header_extra_2 */
  hSbrElement->sbrHeaderData.sbr_limiter_bands = params->sbr_limiter_bands;
  hSbrElement->sbrHeaderData.sbr_limiter_gains = params->sbr_limiter_gains;

  if ((hSbrElement->sbrConfigData.sampleFreq > 48000) &&
      (hSbrElement->sbrHeaderData.sbr_start_frequency >= 9)) {
    hSbrElement->sbrHeaderData.sbr_limiter_gains = SBR_LIMITER_GAINS_INFINITE;
  }

  hSbrElement->sbrHeaderData.sbr_interpol_freq = params->sbr_interpol_freq;
  hSbrElement->sbrHeaderData.sbr_smoothing_length =
      params->sbr_smoothing_length;
  hSbrElement->sbrHeaderData.header_extra_2 = 0;

  if ((params->sbr_limiter_bands != SBR_LIMITER_BANDS_DEFAULT) ||
      (params->sbr_limiter_gains != SBR_LIMITER_GAINS_DEFAULT) ||
      (params->sbr_interpol_freq != SBR_INTERPOL_FREQ_DEFAULT) ||
      (params->sbr_smoothing_length != SBR_SMOOTHING_LENGTH_DEFAULT)) {
    hSbrElement->sbrHeaderData.header_extra_2 = 1;
  }

  /* other switches */
  hSbrElement->sbrConfigData.useWaveCoding = params->useWaveCoding;
  hSbrElement->sbrConfigData.useParametricCoding = params->parametricCoding;
  hSbrElement->sbrConfigData.thresholdAmpResFF_m =
      params->threshold_AmpRes_FF_m;
  hSbrElement->sbrConfigData.thresholdAmpResFF_e =
      params->threshold_AmpRes_FF_e;

  /* init freq band table */
  if (updateFreqBandTable(&hSbrElement->sbrConfigData,
                          &hSbrElement->sbrHeaderData,
                          params->downSampleFactor)) {
    return (1);
  }

  /* now create envelope ext and QMF for each available channel */
  for (ch = 0; ch < hSbrElement->sbrConfigData.nChannels; ch++) {
    if (initEnvChannel(&hSbrElement->sbrConfigData, &hSbrElement->sbrHeaderData,
                       &hSbrElement->sbrChannel[ch]->hEnvChannel, params,
                       statesInitFlag, ch, dynamic_RAM)) {
      return (1);
    }

  } /* nChannels */

  /* reset and intialize analysis qmf */
  for (ch = 0; ch < ((hSbrElement->elInfo.fParametricStereo)
                         ? 2
                         : hSbrElement->sbrConfigData.nChannels);
       ch++) {
    int err;
    UINT qmfFlags =
        (hSbrElement->sbrConfigData.sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY)
            ? QMF_FLAG_CLDFB
            : 0;
    if (statesInitFlag)
      qmfFlags &= ~QMF_FLAG_KEEP_STATES;
    else
      qmfFlags |= QMF_FLAG_KEEP_STATES;

    err = qmfInitAnalysisFilterBank(
        hSbrElement->hQmfAnalysis[ch],
        (FIXP_QAS *)hSbrElement->hQmfAnalysis[ch]->FilterStates,
        hSbrElement->sbrConfigData.noQmfSlots,
        hSbrElement->sbrConfigData.noQmfBands,
        hSbrElement->sbrConfigData.noQmfBands,
        hSbrElement->sbrConfigData.noQmfBands, qmfFlags);
    if (0 != err) {
      return err;
    }
  }

  /*  */
  hSbrElement->CmonData.xOverFreq = hSbrElement->sbrConfigData.xOverFreq;
  hSbrElement->CmonData.dynBwEnabled =
      (params->dynBwSupported && params->dynBwEnabled);
  hSbrElement->CmonData.dynXOverFreqEnc =
      FDKsbrEnc_SbrGetXOverFreq(hSbrElement, hSbrElement->CmonData.xOverFreq);
  for (i = 0; i < 5; i++)
    hSbrElement->dynXOverFreqDelay[i] = hSbrElement->CmonData.dynXOverFreqEnc;
  hSbrElement->CmonData.sbrNumChannels = hSbrElement->sbrConfigData.nChannels;
  hSbrElement->sbrConfigData.dynXOverFreq = hSbrElement->CmonData.xOverFreq;

  /* Update Bandwith to be passed to the core encoder */
  *coreBandWith = hSbrElement->CmonData.xOverFreq;

  return (0);
}

INT sbrEncoder_GetInBufferSize(int noChannels) {
  INT temp;

  temp = (2048);
  temp += 1024 + MAX_SAMPLE_DELAY;
  temp *= noChannels;
  temp *= sizeof(INT_PCM);
  return temp;
}

/*
 * Encode Dummy SBR payload frames to fill the delay lines.
 */
static INT FDKsbrEnc_DelayCompensation(HANDLE_SBR_ENCODER hEnvEnc,
                                       INT_PCM *timeBuffer,
                                       UINT timeBufferBufSize) {
  int n, el;

  for (n = hEnvEnc->nBitstrDelay; n > 0; n--) {
    for (el = 0; el < hEnvEnc->noElements; el++) {
      if (FDKsbrEnc_EnvEncodeFrame(
              hEnvEnc, el,
              timeBuffer + hEnvEnc->downsampledOffset / hEnvEnc->nChannels,
              timeBufferBufSize, NULL, NULL, 1))
        return -1;
    }
    sbrEncoder_UpdateBuffers(hEnvEnc, timeBuffer, timeBufferBufSize);
  }
  return 0;
}

UINT sbrEncoder_LimitBitRate(UINT bitRate, UINT numChannels,
                             UINT coreSampleRate, AUDIO_OBJECT_TYPE aot) {
  UINT newBitRate = bitRate;
  INT index;

  FDK_ASSERT(numChannels > 0 && numChannels <= 2);
  if (aot == AOT_PS) {
    if (numChannels == 1) {
      index = getPsTuningTableIndex(bitRate, &newBitRate);
      if (index == INVALID_TABLE_IDX) {
        bitRate = newBitRate;
      }
    } else {
      return 0;
    }
  }
  index = getSbrTuningTableIndex(bitRate, numChannels, coreSampleRate, aot,
                                 &newBitRate);
  if (index != INVALID_TABLE_IDX) {
    newBitRate = bitRate;
  }

  return newBitRate;
}

UINT sbrEncoder_IsSingleRatePossible(AUDIO_OBJECT_TYPE aot) {
  UINT isPossible = (AOT_PS == aot) ? 0 : 1;
  return isPossible;
}

/*****************************************************************************/
/*                                                                           */
/*functionname: sbrEncoder_Init_delay                                        */
/*description:  Determine Delay balancing and new encoder delay              */
/*                                                                           */
/*returns:      - error status                                               */
/*input:        - frame length of the core (i.e. e.g. AAC)                   */
/*              - number of channels                                         */
/*              - downsample factor (1 for downsampled, 2 for dual-rate SBR) */
/*              - low delay presence                                         */
/*              - ps presence                                                */
/*              - downsampling method: QMF-, time domain or no downsampling  */
/*              - various delay values (see DELAY_PARAM struct description)  */
/*                                                                           */
/*Example: Delay balancing for a HE-AACv1 encoder (time-domain downsampling) */
/*========================================================================== */
/*                                                                           */
/*    +--------+            +--------+ +--------+ +--------+ +--------+      */
/*    |core    |            |ds 2:1  | |AAC     | |QMF     | |QMF     |      */
/*  +-+path    +------------+        +-+core    +-+analysis+-+overlap +-+    */
/*  | |offset  |            |        | |        | |32 bands| |        | |    */
/*  | +--------+            +--------+ +--------+ +--------+ +--------+ |    */
/*  |                              core path                    +-------++   */
/*  |                                                           |QMF     |   */
/*->+                                                           +synth.  +-> */
/*  |                                                           |64 bands|   */
/*  |                                                           +-------++   */
/*  | +--------+ +--------+ +--------+ +--------+                       |    */
/*  | |SBR path| |QMF     | |subband | |bs delay|                       |    */
/*  +-+offset  +-+analysis+-+sample  +-+(full   +-----------------------+    */
/*    |        | |64 bands| |buffer  | | frames)|                            */
/*    +--------+ +--------+ +--------+ +--------+                            */
/*                                 SBR path                                  */
/*                                                                           */
/*****************************************************************************/
static INT sbrEncoder_Init_delay(
    const int coreFrameLength,               /* input */
    const int numChannels,                   /* input */
    const int downSampleFactor,              /* input */
    const int lowDelay,                      /* input */
    const int usePs,                         /* input */
    const int is212,                         /* input */
    const SBRENC_DS_TYPE downsamplingMethod, /* input */
    DELAY_PARAM *hDelayParam                 /* input/output */
) {
  int delayCorePath = 0;   /* delay in core path */
  int delaySbrPath = 0;    /* delay difference in QMF aka SBR path */
  int delayInput2Core = 0; /* delay from the input to the core */
  int delaySbrDec = 0;     /* delay of the decoder's SBR module */

  int delayCore = hDelayParam->delay; /* delay of the core */

  /* Added delay by the SBR delay initialization */
  int corePathOffset = 0; /* core path */
  int sbrPathOffset = 0;  /* sbr path */
  int bitstreamDelay = 0; /* sbr path, framewise */

  int flCore = coreFrameLength; /* core frame length */

  int returnValue = 0; /* return value - 0 means: no error */

  /* 1) Calculate actual delay  for core and SBR path */
  if (is212) {
    delayCorePath = DELAY_COREPATH_ELDv2SBR(flCore, downSampleFactor);
    delaySbrPath = DELAY_ELDv2SBR(flCore, downSampleFactor);
    delaySbrDec = ((flCore) / 2) * (downSampleFactor);
  } else if (lowDelay) {
    delayCorePath = DELAY_COREPATH_ELDSBR(flCore, downSampleFactor);
    delaySbrPath = DELAY_ELDSBR(flCore, downSampleFactor);
    delaySbrDec = DELAY_QMF_POSTPROC(downSampleFactor);
  } else if (usePs) {
    delayCorePath = DELAY_COREPATH_PS(flCore, downSampleFactor);
    delaySbrPath = DELAY_PS(flCore, downSampleFactor);
    delaySbrDec = DELAY_COREPATH_SBR(flCore, downSampleFactor);
  } else {
    delayCorePath = DELAY_COREPATH_SBR(flCore, downSampleFactor);
    delaySbrPath = DELAY_SBR(flCore, downSampleFactor);
    delaySbrDec = DELAY_COREPATH_SBR(flCore, downSampleFactor);
  }
  delayCorePath += delayCore * downSampleFactor;
  delayCorePath +=
      (downsamplingMethod == SBRENC_DS_TIME) ? hDelayParam->dsDelay : 0;

  /* 2) Manage coupling of paths */
  if (downsamplingMethod == SBRENC_DS_QMF && delayCorePath > delaySbrPath) {
    /* In case of QMF downsampling, both paths are coupled, i.e. the SBR path
       offset would be added to both the SBR path and to the core path
       as well, thus making it impossible to achieve delay balancing.
       To overcome that problem, a framewise delay is added to the SBR path
       first, until the overall delay of the core path is shorter than
       the delay of the SBR path. When this is achieved, the missing delay
       difference can be added as downsampled offset to the core path.
    */
    while (delayCorePath > delaySbrPath) {
      /* Add one frame delay to SBR path */
      delaySbrPath += flCore * downSampleFactor;
      bitstreamDelay += 1;
    }
  }

  /* 3) Calculate necessary additional delay to balance the paths */
  if (delayCorePath > delaySbrPath) {
    /* Delay QMF input */
    while (delayCorePath > delaySbrPath + (int)flCore * (int)downSampleFactor) {
      /* Do bitstream frame-wise delay balancing if there are
         more than SBR framelength samples delay difference */
      delaySbrPath += flCore * downSampleFactor;
      bitstreamDelay += 1;
    }
    /* Multiply input offset by input channels */
    corePathOffset = 0;
    sbrPathOffset = (delayCorePath - delaySbrPath) * numChannels;
  } else {
    /* Delay AAC data */
    /* Multiply downsampled offset by AAC core channels. Divide by 2 because of
       half samplerate of downsampled data. */
    corePathOffset = ((delaySbrPath - delayCorePath) * numChannels) >>
                     (downSampleFactor - 1);
    sbrPathOffset = 0;
  }

  /* 4) Calculate delay from input to core */
  if (usePs) {
    delayInput2Core =
        (DELAY_QMF_ANA(downSampleFactor) + DELAY_QMF_DS + DELAY_HYB_SYN) +
        (downSampleFactor * corePathOffset) + 1;
  } else if (downsamplingMethod == SBRENC_DS_TIME) {
    delayInput2Core = corePathOffset + hDelayParam->dsDelay;
  } else {
    delayInput2Core = corePathOffset;
  }

  /* 6) Set output parameters */
  hDelayParam->delay = FDKmax(delayCorePath, delaySbrPath); /* overall delay */
  hDelayParam->sbrDecDelay = delaySbrDec;         /* SBR decoder delay */
  hDelayParam->delayInput2Core = delayInput2Core; /* delay input - core */
  hDelayParam->bitstrDelay = bitstreamDelay;    /* bitstream delay, in frames */
  hDelayParam->corePathOffset = corePathOffset; /* offset added to core path */
  hDelayParam->sbrPathOffset = sbrPathOffset;   /* offset added to SBR path */

  return returnValue;
}

/*****************************************************************************

 functionname: sbrEncoder_Init
 description:  initializes the SBR encoder
 returns:      error status

*****************************************************************************/
INT sbrEncoder_Init(HANDLE_SBR_ENCODER hSbrEncoder,
                    SBR_ELEMENT_INFO elInfo[(8)], int noElements,
                    INT_PCM *inputBuffer, UINT inputBufferBufSize,
                    INT *coreBandwidth, INT *inputBufferOffset,
                    INT *numChannels, const UINT syntaxFlags,
                    INT *coreSampleRate, UINT *downSampleFactor,
                    INT *frameLength, AUDIO_OBJECT_TYPE aot, int *delay,
                    int transformFactor, const int headerPeriod,
                    ULONG statesInitFlag) {
  HANDLE_ERROR_INFO errorInfo = noError;
  sbrConfiguration sbrConfig[(8)];
  INT error = 0;
  INT lowestBandwidth;
  /* Save input parameters */
  INT inputSampleRate = *coreSampleRate;
  int coreFrameLength = *frameLength;
  int inputBandWidth = *coreBandwidth;
  int inputChannels = *numChannels;

  SBRENC_DS_TYPE downsamplingMethod = SBRENC_DS_NONE;
  int highestSbrStartFreq, highestSbrStopFreq;
  int lowDelay = 0;
  int usePs = 0;
  int is212 = 0;

  DELAY_PARAM delayParam;

  /* check whether SBR setting is available for the current encoder
   * configuration (bitrate, samplerate) */
  if (!sbrEncoder_IsSingleRatePossible(aot)) {
    *downSampleFactor = 2;
  }

  if (aot == AOT_PS) {
    usePs = 1;
  }
  if (aot == AOT_ER_AAC_ELD) {
    lowDelay = 1;
  } else if (aot == AOT_ER_AAC_LD) {
    error = 1;
    goto bail;
  }

  /* Parametric Stereo */
  if (usePs) {
    if (*numChannels == 2 && noElements == 1) {
      /* Override Element type in case of Parametric stereo */
      elInfo[0].elType = ID_SCE;
      elInfo[0].fParametricStereo = 1;
      elInfo[0].nChannelsInEl = 1;
      /* core encoder gets downmixed mono signal */
      *numChannels = 1;
    } else {
      error = 1;
      goto bail;
    }
  } /* usePs */

  /* set the core's sample rate */
  switch (*downSampleFactor) {
    case 1:
      *coreSampleRate = inputSampleRate;
      downsamplingMethod = SBRENC_DS_NONE;
      break;
    case 2:
      *coreSampleRate = inputSampleRate >> 1;
      downsamplingMethod = usePs ? SBRENC_DS_QMF : SBRENC_DS_TIME;
      break;
    default:
      *coreSampleRate = inputSampleRate >> 1;
      return 0; /* return error */
  }

  /* check whether SBR setting is available for the current encoder
   * configuration (bitrate, coreSampleRate) */
  {
    int el, coreEl;

    /* Check if every element config is feasible */
    for (coreEl = 0; coreEl < noElements; coreEl++) {
      /* SBR only handles SCE and CPE's */
      if (elInfo[coreEl].elType != ID_SCE && elInfo[coreEl].elType != ID_CPE) {
        continue;
      }
      /* check if desired configuration is available */
      if (!FDKsbrEnc_IsSbrSettingAvail(elInfo[coreEl].bitRate, 0,
                                       elInfo[coreEl].nChannelsInEl,
                                       inputSampleRate, *coreSampleRate, aot)) {
        error = 1;
        goto bail;
      }
    }

    hSbrEncoder->nChannels = *numChannels;
    hSbrEncoder->frameSize = coreFrameLength * *downSampleFactor;
    hSbrEncoder->downsamplingMethod = downsamplingMethod;
    hSbrEncoder->downSampleFactor = *downSampleFactor;
    hSbrEncoder->estimateBitrate = 0;
    hSbrEncoder->inputDataDelay = 0;
    is212 = ((aot == AOT_ER_AAC_ELD) && (syntaxFlags & AC_LD_MPS)) ? 1 : 0;

    /* Open SBR elements */
    el = -1;
    highestSbrStartFreq = highestSbrStopFreq = 0;
    lowestBandwidth = 99999;

    /* Loop through each core encoder element and get a matching SBR element
     * config */
    for (coreEl = 0; coreEl < noElements; coreEl++) {
      /* SBR only handles SCE and CPE's */
      if (elInfo[coreEl].elType == ID_SCE || elInfo[coreEl].elType == ID_CPE) {
        el++;
      } else {
        continue;
      }

      /* Set parametric Stereo Flag. */
      if (usePs) {
        elInfo[coreEl].fParametricStereo = 1;
      } else {
        elInfo[coreEl].fParametricStereo = 0;
      }

      /*
       * Init sbrConfig structure
       */
      if (!FDKsbrEnc_InitializeSbrDefaults(&sbrConfig[el], *downSampleFactor,
                                           coreFrameLength, IS_LOWDELAY(aot))) {
        error = 1;
        goto bail;
      }

      /*
       * Modify sbrConfig structure according to Element parameters
       */
      if (!FDKsbrEnc_AdjustSbrSettings(
              &sbrConfig[el], elInfo[coreEl].bitRate,
              elInfo[coreEl].nChannelsInEl, *coreSampleRate, inputSampleRate,
              transformFactor, 24000, 0, 0, /* useSpeechConfig */
              0,                            /* lcsMode */
              usePs,                        /* bParametricStereo */
              aot)) {
        error = 1;
        goto bail;
      }

      /* Find common frequency border for all SBR elements */
      highestSbrStartFreq =
          fixMax(highestSbrStartFreq, sbrConfig[el].startFreq);
      highestSbrStopFreq = fixMax(highestSbrStopFreq, sbrConfig[el].stopFreq);

    } /* first element loop */

    /* Set element count (can be less than core encoder element count) */
    hSbrEncoder->noElements = el + 1;

    FDKsbrEnc_Reallocate(hSbrEncoder, elInfo, noElements);

    for (el = 0; el < hSbrEncoder->noElements; el++) {
      int bandwidth = *coreBandwidth;

      /* Use lowest common bandwidth */
      sbrConfig[el].startFreq = highestSbrStartFreq;
      sbrConfig[el].stopFreq = highestSbrStopFreq;

      /* initialize SBR element, and get core bandwidth */
      error = FDKsbrEnc_EnvInit(hSbrEncoder->sbrElement[el], &sbrConfig[el],
                                &bandwidth, aot, el, headerPeriod,
                                statesInitFlag, hSbrEncoder->downsamplingMethod,
                                hSbrEncoder->dynamicRam);

      if (error != 0) {
        error = 2;
        goto bail;
      }

      /* Get lowest core encoder bandwidth to be returned later. */
      lowestBandwidth = fixMin(lowestBandwidth, bandwidth);

    } /* second element loop */

    /* Initialize a downsampler for each channel in each SBR element */
    if (hSbrEncoder->downsamplingMethod == SBRENC_DS_TIME) {
      for (el = 0; el < hSbrEncoder->noElements; el++) {
        HANDLE_SBR_ELEMENT hSbrEl = hSbrEncoder->sbrElement[el];
        INT Wc, ch;

        Wc = 500; /* Cutoff frequency with full bandwidth */

        for (ch = 0; ch < hSbrEl->elInfo.nChannelsInEl; ch++) {
          FDKaacEnc_InitDownsampler(&hSbrEl->sbrChannel[ch]->downSampler, Wc,
                                    *downSampleFactor);
          FDK_ASSERT(hSbrEl->sbrChannel[ch]->downSampler.delay <=
                     MAX_DS_FILTER_DELAY);
        }
      } /* third element loop */

      /* lfe */
      FDKaacEnc_InitDownsampler(&hSbrEncoder->lfeDownSampler, 0,
                                *downSampleFactor);
    }

    /* Get delay information */
    delayParam.dsDelay =
        hSbrEncoder->sbrElement[0]->sbrChannel[0]->downSampler.delay;
    delayParam.delay = *delay;

    error = sbrEncoder_Init_delay(coreFrameLength, *numChannels,
                                  *downSampleFactor, lowDelay, usePs, is212,
                                  downsamplingMethod, &delayParam);

    if (error != 0) {
      error = 3;
      goto bail;
    }

    hSbrEncoder->nBitstrDelay = delayParam.bitstrDelay;
    hSbrEncoder->sbrDecDelay = delayParam.sbrDecDelay;
    hSbrEncoder->inputDataDelay = delayParam.delayInput2Core;

    /* Assign core encoder Bandwidth */
    *coreBandwidth = lowestBandwidth;

    /* Estimate sbr bitrate, 2.5 kBit/s per sbr channel */
    hSbrEncoder->estimateBitrate += 2500 * (*numChannels);

    /* Initialize bitstream buffer for each element */
    for (el = 0; el < hSbrEncoder->noElements; el++) {
      FDKsbrEnc_bsBufInit(hSbrEncoder->sbrElement[el], delayParam.bitstrDelay);
    }

    /* initialize parametric stereo */
    if (usePs) {
      PSENC_CONFIG psEncConfig;
      FDK_ASSERT(hSbrEncoder->noElements == 1);
      INT psTuningTableIdx = getPsTuningTableIndex(elInfo[0].bitRate, NULL);

      psEncConfig.frameSize = coreFrameLength;  // sbrConfig.sbrFrameSize;
      psEncConfig.qmfFilterMode = 0;
      psEncConfig.sbrPsDelay = 0;

      /* tuning parameters */
      if (psTuningTableIdx != INVALID_TABLE_IDX) {
        psEncConfig.nStereoBands = psTuningTable[psTuningTableIdx].nStereoBands;
        psEncConfig.maxEnvelopes = psTuningTable[psTuningTableIdx].nEnvelopes;
        psEncConfig.iidQuantErrorThreshold =
            (FIXP_DBL)psTuningTable[psTuningTableIdx].iidQuantErrorThreshold;

        /* calculation is not quite linear, increased number of envelopes causes
         * more bits */
        /* assume avg. 50 bits per frame for 10 stereo bands / 1 envelope
         * configuration */
        hSbrEncoder->estimateBitrate +=
            ((((*coreSampleRate) * 5 * psEncConfig.nStereoBands *
               psEncConfig.maxEnvelopes) /
              hSbrEncoder->frameSize));

      } else {
        error = ERROR(CDI, "Invalid ps tuning table index.");
        goto bail;
      }

      qmfInitSynthesisFilterBank(
          &hSbrEncoder->qmfSynthesisPS,
          (FIXP_DBL *)hSbrEncoder->qmfSynthesisPS.FilterStates,
          hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfSlots,
          hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfBands >> 1,
          hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfBands >> 1,
          hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfBands >> 1,
          (statesInitFlag) ? 0 : QMF_FLAG_KEEP_STATES);

      if (errorInfo == noError) {
        /* update delay */
        psEncConfig.sbrPsDelay =
            FDKsbrEnc_GetEnvEstDelay(&hSbrEncoder->sbrElement[0]
                                          ->sbrChannel[0]
                                          ->hEnvChannel.sbrExtractEnvelope);

        errorInfo =
            PSEnc_Init(hSbrEncoder->hParametricStereo, &psEncConfig,
                       hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfSlots,
                       hSbrEncoder->sbrElement[0]->sbrConfigData.noQmfBands,
                       hSbrEncoder->dynamicRam);
      }
    }

    hSbrEncoder->downsampledOffset = delayParam.corePathOffset;
    hSbrEncoder->bufferOffset = delayParam.sbrPathOffset;
    *delay = delayParam.delay;

    { hSbrEncoder->downmixSize = coreFrameLength * (*numChannels); }

    /* Delay Compensation: fill bitstream delay buffer with zero input signal */
    if (hSbrEncoder->nBitstrDelay > 0) {
      error = FDKsbrEnc_DelayCompensation(hSbrEncoder, inputBuffer,
                                          inputBufferBufSize);
      if (error != 0) goto bail;
    }

    /* Set Output frame length */
    *frameLength = coreFrameLength * *downSampleFactor;
    /* Input buffer offset */
    *inputBufferOffset =
        fixMax(delayParam.sbrPathOffset, delayParam.corePathOffset);
  }

  return error;

bail:
  /* Restore input settings */
  *coreSampleRate = inputSampleRate;
  *frameLength = coreFrameLength;
  *numChannels = inputChannels;
  *coreBandwidth = inputBandWidth;

  return error;
}

INT sbrEncoder_EncodeFrame(HANDLE_SBR_ENCODER hSbrEncoder, INT_PCM *samples,
                           UINT samplesBufSize, UINT sbrDataBits[(8)],
                           UCHAR sbrData[(8)][MAX_PAYLOAD_SIZE]) {
  INT error;
  int el;

  for (el = 0; el < hSbrEncoder->noElements; el++) {
    if (hSbrEncoder->sbrElement[el] != NULL) {
      error = FDKsbrEnc_EnvEncodeFrame(
          hSbrEncoder, el,
          samples + hSbrEncoder->downsampledOffset / hSbrEncoder->nChannels,
          samplesBufSize, &sbrDataBits[el], sbrData[el], 0);
      if (error) return error;
    }
  }

  error = FDKsbrEnc_Downsample(
      hSbrEncoder,
      samples + hSbrEncoder->downsampledOffset / hSbrEncoder->nChannels,
      samplesBufSize, hSbrEncoder->nChannels, &sbrDataBits[el], sbrData[el], 0);
  if (error) return error;

  return 0;
}

INT sbrEncoder_UpdateBuffers(HANDLE_SBR_ENCODER hSbrEncoder,
                             INT_PCM *timeBuffer, UINT timeBufferBufSize) {
  if (hSbrEncoder->downsampledOffset > 0) {
    int c;
    int nd = hSbrEncoder->downmixSize / hSbrEncoder->nChannels;

    for (c = 0; c < hSbrEncoder->nChannels; c++) {
      /* Move delayed downsampled data */
      FDKmemcpy(timeBuffer + timeBufferBufSize * c,
                timeBuffer + timeBufferBufSize * c + nd,
                sizeof(INT_PCM) *
                    (hSbrEncoder->downsampledOffset / hSbrEncoder->nChannels));
    }
  } else {
    int c;

    for (c = 0; c < hSbrEncoder->nChannels; c++) {
      /* Move delayed input data */
      FDKmemcpy(
          timeBuffer + timeBufferBufSize * c,
          timeBuffer + timeBufferBufSize * c + hSbrEncoder->frameSize,
          sizeof(INT_PCM) * hSbrEncoder->bufferOffset / hSbrEncoder->nChannels);
    }
  }
  if (hSbrEncoder->nBitstrDelay > 0) {
    int el;

    for (el = 0; el < hSbrEncoder->noElements; el++) {
      FDKmemmove(
          hSbrEncoder->sbrElement[el]->payloadDelayLine[0],
          hSbrEncoder->sbrElement[el]->payloadDelayLine[1],
          sizeof(UCHAR) * (hSbrEncoder->nBitstrDelay * MAX_PAYLOAD_SIZE));

      FDKmemmove(&hSbrEncoder->sbrElement[el]->payloadDelayLineSize[0],
                 &hSbrEncoder->sbrElement[el]->payloadDelayLineSize[1],
                 sizeof(UINT) * (hSbrEncoder->nBitstrDelay));
    }
  }
  return 0;
}

INT sbrEncoder_SendHeader(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT error = -1;
  if (hSbrEncoder) {
    int el;
    for (el = 0; el < hSbrEncoder->noElements; el++) {
      if ((hSbrEncoder->noElements == 1) &&
          (hSbrEncoder->sbrElement[0]->elInfo.fParametricStereo == 1)) {
        hSbrEncoder->sbrElement[el]->sbrBitstreamData.CountSendHeaderData =
            hSbrEncoder->sbrElement[el]->sbrBitstreamData.NrSendHeaderData - 1;
      } else {
        hSbrEncoder->sbrElement[el]->sbrBitstreamData.CountSendHeaderData = 0;
      }
    }
    error = 0;
  }
  return error;
}

INT sbrEncoder_ContainsHeader(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT sbrHeader = 1;
  if (hSbrEncoder) {
    int el;
    for (el = 0; el < hSbrEncoder->noElements; el++) {
      sbrHeader &=
          (hSbrEncoder->sbrElement[el]->sbrBitstreamData.HeaderActiveDelay == 1)
              ? 1
              : 0;
    }
  }
  return sbrHeader;
}

INT sbrEncoder_GetHeaderDelay(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT delay = -1;

  if (hSbrEncoder) {
    if ((hSbrEncoder->noElements == 1) &&
        (hSbrEncoder->sbrElement[0]->elInfo.fParametricStereo == 1)) {
      delay = hSbrEncoder->nBitstrDelay + 1;
    } else {
      delay = hSbrEncoder->nBitstrDelay;
    }
  }
  return delay;
}
INT sbrEncoder_GetBsDelay(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT delay = -1;

  if (hSbrEncoder) {
    delay = hSbrEncoder->nBitstrDelay;
  }
  return delay;
}

INT sbrEncoder_SAPPrepare(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT error = -1;
  if (hSbrEncoder) {
    int el;
    for (el = 0; el < hSbrEncoder->noElements; el++) {
      hSbrEncoder->sbrElement[el]->sbrBitstreamData.rightBorderFIX = 1;
    }
    error = 0;
  }
  return error;
}

INT sbrEncoder_GetEstimateBitrate(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT estimateBitrate = 0;

  if (hSbrEncoder) {
    estimateBitrate += hSbrEncoder->estimateBitrate;
  }

  return estimateBitrate;
}

INT sbrEncoder_GetInputDataDelay(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT delay = -1;

  if (hSbrEncoder) {
    delay = hSbrEncoder->inputDataDelay;
  }
  return delay;
}

INT sbrEncoder_GetSbrDecDelay(HANDLE_SBR_ENCODER hSbrEncoder) {
  INT delay = -1;

  if (hSbrEncoder) {
    delay = hSbrEncoder->sbrDecDelay;
  }
  return delay;
}

INT sbrEncoder_GetLibInfo(LIB_INFO *info) {
  int i;

  if (info == NULL) {
    return -1;
  }
  /* search for next free tab */
  for (i = 0; i < FDK_MODULE_LAST; i++) {
    if (info[i].module_id == FDK_NONE) break;
  }
  if (i == FDK_MODULE_LAST) {
    return -1;
  }
  info += i;

  info->module_id = FDK_SBRENC;
  info->version =
      LIB_VERSION(SBRENCODER_LIB_VL0, SBRENCODER_LIB_VL1, SBRENCODER_LIB_VL2);
  LIB_VERSION_STRING(info);
#ifdef SUPPRESS_BUILD_DATE_INFO
  info->build_date = "";
  info->build_time = "";
#else
  info->build_date = __DATE__;
  info->build_time = __TIME__;
#endif
  info->title = "SBR Encoder";

  /* Set flags */
  info->flags = 0 | CAPF_SBR_HQ | CAPF_SBR_PS_MPEG;
  /* End of flags */

  return 0;
}
