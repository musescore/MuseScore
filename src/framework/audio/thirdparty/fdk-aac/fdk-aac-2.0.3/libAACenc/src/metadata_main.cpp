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

/**************************** AAC encoder library ******************************

   Author(s):   V. Bacigalupo

   Description: Metadata Encoder library interface functions

*******************************************************************************/

#include "metadata_main.h"
#include "metadata_compressor.h"
#include "FDK_bitstream.h"
#include "FDK_audio.h"
#include "genericStds.h"

/*----------------- defines ----------------------*/
#define MAX_DRC_BANDS (1 << 4)
#define MAX_DRC_FRAMELEN (2 * 1024)
#define MAX_DELAY_FRAMES (3)

/*--------------- structure definitions --------------------*/

typedef struct AAC_METADATA {
  /* MPEG: Dynamic Range Control */
  struct {
    UCHAR prog_ref_level_present;
    SCHAR prog_ref_level;

    UCHAR dyn_rng_sgn[MAX_DRC_BANDS];
    UCHAR dyn_rng_ctl[MAX_DRC_BANDS];

    UCHAR drc_bands_present;
    UCHAR drc_band_incr;
    UCHAR drc_band_top[MAX_DRC_BANDS];
    UCHAR drc_interpolation_scheme;
    AACENC_METADATA_DRC_PROFILE drc_profile;
    INT drc_TargetRefLevel; /* used for Limiter */

    /* excluded channels */
    UCHAR excluded_chns_present;
    UCHAR exclude_mask[2]; /* MAX_NUMBER_CHANNELS/8 */
  } mpegDrc;

  /* ETSI: addtl ancillary data */
  struct {
    /* Heavy Compression */
    UCHAR compression_on;    /* flag, if compression value should be written */
    UCHAR compression_value; /* compression value */
    AACENC_METADATA_DRC_PROFILE comp_profile;
    INT comp_TargetRefLevel; /* used for Limiter */
    INT timecode_coarse_status;
    INT timecode_fine_status;

    UCHAR extAncDataStatus;

    struct {
      UCHAR ext_downmix_lvl_status;
      UCHAR ext_downmix_gain_status;
      UCHAR ext_lfe_downmix_status;
      UCHAR
      ext_dmix_a_idx; /* extended downmix level (0..7, according to table)
                       */
      UCHAR
      ext_dmix_b_idx; /* extended downmix level (0..7, according to table)
                       */
      UCHAR dmx_gain_5_sgn;
      UCHAR dmx_gain_5_idx;
      UCHAR dmx_gain_2_sgn;
      UCHAR dmx_gain_2_idx;
      UCHAR ext_dmix_lfe_idx; /* extended downmix level for lfe (0..15,
                                 according to table) */

    } extAncData;

  } etsiAncData;

  SCHAR centerMixLevel; /* center downmix level (0...7, according to table) */
  SCHAR
  surroundMixLevel; /* surround downmix level (0...7, according to table) */
  UCHAR WritePCEMixDwnIdx; /* flag */
  UCHAR DmxLvl_On;         /* flag */

  UCHAR dolbySurroundMode;
  UCHAR drcPresentationMode;

  UCHAR
  metadataMode; /* indicate meta data mode in current frame (delay line) */

} AAC_METADATA;

typedef struct FDK_METADATA_ENCODER {
  INT metadataMode;
  HDRC_COMP hDrcComp;
  AACENC_MetaData submittedMetaData;

  INT nAudioDataDelay; /* Additional delay to round up to next frame border (in
                          samples) */
  INT nMetaDataDelay;  /* Meta data delay (in frames) */
  INT nChannels;
  CHANNEL_MODE channelMode;

  INT_PCM* pAudioDelayBuffer;

  AAC_METADATA metaDataBuffer[MAX_DELAY_FRAMES];
  INT metaDataDelayIdx;

  UCHAR drcInfoPayload[12];
  UCHAR drcDsePayload[8];

  INT matrix_mixdown_idx;

  AACENC_EXT_PAYLOAD exPayload[2];
  INT nExtensions;

  UINT maxChannels; /* Maximum number of audio channels to be supported. */

  INT finalizeMetaData;   /* Delay switch off by one frame and write default
                             configuration to   finalize the metadata setup. */
  INT initializeMetaData; /* Fill up delay line with first meta data info. This
                             is required to have meta data already in first
                             frame. */
} FDK_METADATA_ENCODER;

/*---------------- constants -----------------------*/
static const AACENC_MetaData defaultMetaDataSetup = {
    AACENC_METADATA_DRC_NONE,        /* drc_profile */
    AACENC_METADATA_DRC_NOT_PRESENT, /* comp_profile */
    -(31 << 16),                     /* drc_TargetRefLevel */
    -(23 << 16),                     /* comp_TargetRefLevel */
    0,                               /* prog_ref_level_present */
    -(23 << 16),                     /* prog_ref_level */
    0,                               /* PCE_mixdown_idx_present */
    0,                               /* ETSI_DmxLvl_present */
    0,                               /* centerMixLevel */
    0,                               /* surroundMixLevel */
    0,                               /* dolbySurroundMode */
    0,                               /* drcPresentationMode */
    {0, 0, 0, 0, 0, 0, 0, 0, 0}      /* ExtMetaData */
};

static const FIXP_DBL dmxTable[8] = {
    ((FIXP_DBL)MAXVAL_DBL), FL2FXCONST_DBL(0.841f), FL2FXCONST_DBL(0.707f),
    FL2FXCONST_DBL(0.596f), FL2FXCONST_DBL(0.500f), FL2FXCONST_DBL(0.422f),
    FL2FXCONST_DBL(0.355f), FL2FXCONST_DBL(0.000f)};

#define FL2DMXLFE(a) FL2FXCONST_DBL((a) / (1 << LFE_LEV_SCALE))
static const FIXP_DBL dmxLfeTable[16] = {
    FL2DMXLFE(3.162f), FL2DMXLFE(2.000f), FL2DMXLFE(1.679f), FL2DMXLFE(1.413f),
    FL2DMXLFE(1.189f), FL2DMXLFE(1.000f), FL2DMXLFE(0.841f), FL2DMXLFE(0.707f),
    FL2DMXLFE(0.596f), FL2DMXLFE(0.500f), FL2DMXLFE(0.316f), FL2DMXLFE(0.178f),
    FL2DMXLFE(0.100f), FL2DMXLFE(0.032f), FL2DMXLFE(0.010f), FL2DMXLFE(0.000f)};

static const UCHAR surmix2matrix_mixdown_idx[8] = {0, 0, 0, 1, 1, 2, 2, 3};

/*--------------- function declarations --------------------*/
static FDK_METADATA_ERROR WriteMetadataPayload(
    const HANDLE_FDK_METADATA_ENCODER hMetaData,
    const AAC_METADATA* const pMetadata);

static INT WriteDynamicRangeInfoPayload(const AAC_METADATA* const pMetadata,
                                        UCHAR* const pExtensionPayload);

static INT WriteEtsiAncillaryDataPayload(const AAC_METADATA* const pMetadata,
                                         UCHAR* const pExtensionPayload);

static FDK_METADATA_ERROR CompensateAudioDelay(
    HANDLE_FDK_METADATA_ENCODER hMetaDataEnc, INT_PCM* const pAudioSamples,
    const UINT audioSamplesBufSize, const INT nAudioSamples);

static FDK_METADATA_ERROR LoadSubmittedMetadata(
    const AACENC_MetaData* const hMetadata, const INT nChannels,
    const INT metadataMode, AAC_METADATA* const pAacMetaData);

static FDK_METADATA_ERROR ProcessCompressor(AAC_METADATA* pMetadata,
                                            HDRC_COMP hDrcComp,
                                            const INT_PCM* const pSamples,
                                            const UINT samplesBufSize,
                                            const INT nSamples);

/*------------- function definitions ----------------*/

static DRC_PROFILE convertProfile(AACENC_METADATA_DRC_PROFILE aacProfile) {
  DRC_PROFILE drcProfile = DRC_NONE;

  switch (aacProfile) {
    case AACENC_METADATA_DRC_NONE:
      drcProfile = DRC_NONE;
      break;
    case AACENC_METADATA_DRC_FILMSTANDARD:
      drcProfile = DRC_FILMSTANDARD;
      break;
    case AACENC_METADATA_DRC_FILMLIGHT:
      drcProfile = DRC_FILMLIGHT;
      break;
    case AACENC_METADATA_DRC_MUSICSTANDARD:
      drcProfile = DRC_MUSICSTANDARD;
      break;
    case AACENC_METADATA_DRC_MUSICLIGHT:
      drcProfile = DRC_MUSICLIGHT;
      break;
    case AACENC_METADATA_DRC_SPEECH:
      drcProfile = DRC_SPEECH;
      break;
    case AACENC_METADATA_DRC_NOT_PRESENT:
      drcProfile = DRC_NOT_PRESENT;
      break;
    default:
      drcProfile = DRC_NONE;
      break;
  }
  return drcProfile;
}

/* convert dialog normalization to program reference level */
/* NOTE: this only is correct, if the decoder target level is set to -31dB for
 * line mode / -20dB for RF mode */
static UCHAR dialnorm2progreflvl(const INT d) {
  return ((UCHAR)fMax(0, fMin((-d + (1 << 13)) >> 14, 127)));
}

/* convert program reference level to dialog normalization */
static INT progreflvl2dialnorm(const UCHAR p) {
  return -((INT)(p << (16 - 2)));
}

/* encode downmix levels to Downmixing_levels_MPEG4 */
static SCHAR encodeDmxLvls(const SCHAR cmixlev, const SCHAR surmixlev) {
  SCHAR dmxLvls = 0;
  dmxLvls |= 0x80 | (cmixlev << 4); /* center_mix_level_on */
  dmxLvls |= 0x08 | surmixlev;      /* surround_mix_level_on */

  return dmxLvls;
}

/* encode AAC DRC gain (ISO/IEC 14496-3:2005  4.5.2.7) */
static void encodeDynrng(INT gain, UCHAR* const dyn_rng_ctl,
                         UCHAR* const dyn_rng_sgn) {
  if (gain < 0) {
    *dyn_rng_sgn = 1;
    gain = -gain;
  } else {
    *dyn_rng_sgn = 0;
  }
  gain = fMin(gain, (127 << 14));

  *dyn_rng_ctl = (UCHAR)((gain + (1 << 13)) >> 14);
}

/* decode AAC DRC gain (ISO/IEC 14496-3:2005  4.5.2.7) */
static INT decodeDynrng(const UCHAR dyn_rng_ctl, const UCHAR dyn_rng_sgn) {
  INT tmp = ((INT)dyn_rng_ctl << (16 - 2));
  if (dyn_rng_sgn) tmp = -tmp;

  return tmp;
}

/* encode AAC compression value (ETSI TS 101 154 page 99) */
static UCHAR encodeCompr(INT gain) {
  UCHAR x, y;
  INT tmp;

  /* tmp = (int)((48.164f - gain) / 6.0206f * 15 + 0.5f); */
  tmp = ((3156476 - gain) * 15 + 197283) / 394566;

  if (tmp >= 240) {
    return 0xFF;
  } else if (tmp < 0) {
    return 0;
  } else {
    x = tmp / 15;
    y = tmp % 15;
  }

  return (x << 4) | y;
}

/* decode AAC compression value (ETSI TS 101 154 page 99) */
static INT decodeCompr(const UCHAR compr) {
  INT gain;
  SCHAR x = compr >> 4;     /* 4 MSB of compr */
  UCHAR y = (compr & 0x0F); /* 4 LSB of compr */

  /* gain = (INT)((48.164f - 6.0206f * x - 0.4014f * y) ); */
  gain = (INT)(
      scaleValue((FIXP_DBL)(((LONG)FL2FXCONST_DBL(6.0206f / 128.f) * (8 - x) -
                             (LONG)FL2FXCONST_DBL(0.4014f / 128.f) * y)),
                 -(DFRACT_BITS - 1 - 7 - 16)));

  return gain;
}

FDK_METADATA_ERROR FDK_MetadataEnc_Open(HANDLE_FDK_METADATA_ENCODER* phMetaData,
                                        const UINT maxChannels) {
  FDK_METADATA_ERROR err = METADATA_OK;
  HANDLE_FDK_METADATA_ENCODER hMetaData = NULL;

  if (phMetaData == NULL) {
    err = METADATA_INVALID_HANDLE;
    goto bail;
  }

  /* allocate memory */
  if (NULL == (hMetaData = (HANDLE_FDK_METADATA_ENCODER)FDKcalloc(
                   1, sizeof(FDK_METADATA_ENCODER)))) {
    err = METADATA_MEMORY_ERROR;
    goto bail;
  }
  FDKmemclear(hMetaData, sizeof(FDK_METADATA_ENCODER));

  if (NULL == (hMetaData->pAudioDelayBuffer = (INT_PCM*)FDKcalloc(
                   maxChannels * MAX_DRC_FRAMELEN, sizeof(INT_PCM)))) {
    err = METADATA_MEMORY_ERROR;
    goto bail;
  }
  FDKmemclear(hMetaData->pAudioDelayBuffer,
              maxChannels * MAX_DRC_FRAMELEN * sizeof(INT_PCM));
  hMetaData->maxChannels = maxChannels;

  /* Allocate DRC Compressor. */
  if (FDK_DRC_Generator_Open(&hMetaData->hDrcComp) != 0) {
    err = METADATA_MEMORY_ERROR;
    goto bail;
  }
  hMetaData->channelMode = MODE_UNKNOWN;

  /* Return metadata instance */
  *phMetaData = hMetaData;

  return err;

bail:
  FDK_MetadataEnc_Close(&hMetaData);
  return err;
}

FDK_METADATA_ERROR FDK_MetadataEnc_Close(
    HANDLE_FDK_METADATA_ENCODER* phMetaData) {
  FDK_METADATA_ERROR err = METADATA_OK;

  if (phMetaData == NULL) {
    err = METADATA_INVALID_HANDLE;
    goto bail;
  }

  if (*phMetaData != NULL) {
    FDK_DRC_Generator_Close(&(*phMetaData)->hDrcComp);
    FDKfree((*phMetaData)->pAudioDelayBuffer);
    FDKfree(*phMetaData);
    *phMetaData = NULL;
  }
bail:
  return err;
}

FDK_METADATA_ERROR FDK_MetadataEnc_Init(
    HANDLE_FDK_METADATA_ENCODER hMetaData, const INT resetStates,
    const INT metadataMode, const INT audioDelay, const UINT frameLength,
    const UINT sampleRate, const UINT nChannels, const CHANNEL_MODE channelMode,
    const CHANNEL_ORDER channelOrder) {
  FDK_METADATA_ERROR err = METADATA_OK;
  int i, nFrames, delay;

  if (hMetaData == NULL) {
    err = METADATA_INVALID_HANDLE;
    goto bail;
  }

  /* Determine values for delay compensation. */
  for (nFrames = 0, delay = audioDelay - (INT)frameLength; delay > 0;
       delay -= (INT)frameLength, nFrames++)
    ;

  if ((nChannels > (8)) || (nChannels > hMetaData->maxChannels) ||
      ((-delay) > MAX_DRC_FRAMELEN) || nFrames >= MAX_DELAY_FRAMES) {
    err = METADATA_INIT_ERROR;
    goto bail;
  }

  /* Initialize with default setup. */
  FDKmemcpy(&hMetaData->submittedMetaData, &defaultMetaDataSetup,
            sizeof(AACENC_MetaData));

  hMetaData->finalizeMetaData =
      0; /* finalize meta data only while on/off switching, else disabled */
  hMetaData->initializeMetaData =
      0; /* fill up meta data delay line only at a reset otherwise disabled */

  /* Reset delay lines. */
  if (resetStates || (hMetaData->nAudioDataDelay != -delay) ||
      (hMetaData->channelMode != channelMode)) {
    if (resetStates || (hMetaData->channelMode == MODE_UNKNOWN)) {
      /* clear delay buffer */
      FDKmemclear(hMetaData->pAudioDelayBuffer,
                  hMetaData->maxChannels * MAX_DRC_FRAMELEN * sizeof(INT_PCM));
    } else {
      /* if possible, keep static audio channels for seamless channel
       * reconfiguration */
      FDK_channelMapDescr mapDescrPrev, mapDescr;
      int c, src[2] = {-1, -1}, dst[2] = {-1, -1};

      if (channelOrder == CH_ORDER_WG4) {
        FDK_chMapDescr_init(&mapDescrPrev, FDK_mapInfoTabWg4,
                            FDK_mapInfoTabLenWg4, 0);
        FDK_chMapDescr_init(&mapDescr, FDK_mapInfoTabWg4,
                            FDK_mapInfoTabLenWg4, 0);
      } else {
        FDK_chMapDescr_init(&mapDescrPrev, NULL, 0,
                            (channelOrder == CH_ORDER_MPEG) ? 1 : 0);
        FDK_chMapDescr_init(&mapDescr, NULL, 0,
                            (channelOrder == CH_ORDER_MPEG) ? 1 : 0);
      }

      switch (channelMode) {
        case MODE_1:
          if ((INT)nChannels != 2) {
            /* preserve center channel */
            src[0] = FDK_chMapDescr_getMapValue(&mapDescrPrev, 0,
                                                hMetaData->channelMode);
            dst[0] = FDK_chMapDescr_getMapValue(&mapDescr, 0, channelMode);
          }
          break;
        case MODE_2:
        case MODE_1_2:
        case MODE_1_2_1:
        case MODE_1_2_2:
        case MODE_1_2_2_1:
          if (hMetaData->nChannels >= 2) {
            /* preserve left/right channel */
            src[0] = FDK_chMapDescr_getMapValue(
                &mapDescrPrev, ((hMetaData->channelMode == 2) ? 0 : 1),
                hMetaData->channelMode);
            src[1] = FDK_chMapDescr_getMapValue(
                &mapDescrPrev, ((hMetaData->channelMode == 2) ? 1 : 2),
                hMetaData->channelMode);
            dst[0] = FDK_chMapDescr_getMapValue(
                &mapDescr, ((channelMode == 2) ? 0 : 1), channelMode);
            dst[1] = FDK_chMapDescr_getMapValue(
                &mapDescr, ((channelMode == 2) ? 1 : 2), channelMode);
          }
          break;
        default:;
      }
      C_ALLOC_SCRATCH_START(scratch_audioDelayBuffer, INT_PCM, (8));
      FDKmemclear(scratch_audioDelayBuffer, (8) * sizeof(INT_PCM));

      i = (hMetaData->nChannels > (INT)nChannels)
              ? 0
              : hMetaData->nAudioDataDelay - 1;
      do {
        for (c = 0; c < 2; c++) {
          if (src[c] != -1 && dst[c] != -1) {
            scratch_audioDelayBuffer[dst[c]] =
                hMetaData->pAudioDelayBuffer[i * hMetaData->nChannels + src[c]];
          }
        }
        FDKmemcpy(&hMetaData->pAudioDelayBuffer[i * nChannels],
                  scratch_audioDelayBuffer, nChannels * sizeof(INT_PCM));
        i += (hMetaData->nChannels > (INT)nChannels) ? 1 : -1;
      } while ((i < hMetaData->nAudioDataDelay) && (i >= 0));

      C_ALLOC_SCRATCH_END(scratch_audioDelayBuffer, INT_PCM, (8));
    }
    FDKmemclear(hMetaData->metaDataBuffer, sizeof(hMetaData->metaDataBuffer));
    hMetaData->metaDataDelayIdx = 0;
    hMetaData->initializeMetaData =
        1; /* fill up delay line with first meta data info */
  } else {
    /* Enable meta data. */
    if ((hMetaData->metadataMode == 0) && (metadataMode != 0)) {
      /* disable meta data in all delay lines */
      for (i = 0;
           i < (int)(sizeof(hMetaData->metaDataBuffer) / sizeof(AAC_METADATA));
           i++) {
        LoadSubmittedMetadata(&hMetaData->submittedMetaData, nChannels, 0,
                              &hMetaData->metaDataBuffer[i]);
      }
    }

    /* Disable meta data.*/
    if ((hMetaData->metadataMode != 0) && (metadataMode == 0)) {
      hMetaData->finalizeMetaData = hMetaData->metadataMode;
    }
  }

  /* Initialize delay. */
  hMetaData->nAudioDataDelay = -delay;
  hMetaData->nMetaDataDelay = nFrames;
  hMetaData->nChannels = nChannels;
  hMetaData->channelMode = channelMode;
  hMetaData->metadataMode = metadataMode;

  /* Initialize compressor. */
  if ((metadataMode == 1) || (metadataMode == 2)) {
    if (FDK_DRC_Generator_Initialize(hMetaData->hDrcComp, DRC_NONE, DRC_NONE,
                                     frameLength, sampleRate, channelMode,
                                     channelOrder, 1) != 0) {
      err = METADATA_INIT_ERROR;
    }
  }
bail:
  return err;
}

static FDK_METADATA_ERROR ProcessCompressor(AAC_METADATA* pMetadata,
                                            HDRC_COMP hDrcComp,
                                            const INT_PCM* const pSamples,
                                            const UINT samplesBufSize,
                                            const INT nSamples) {
  FDK_METADATA_ERROR err = METADATA_OK;

  INT dynrng, compr;
  INT dmxGain5, dmxGain2;
  DRC_PROFILE profileDrc;
  DRC_PROFILE profileComp;

  if ((pMetadata == NULL) || (hDrcComp == NULL)) {
    err = METADATA_INVALID_HANDLE;
    return err;
  }

  profileDrc = convertProfile(pMetadata->mpegDrc.drc_profile);
  profileComp = convertProfile(pMetadata->etsiAncData.comp_profile);

  /* first, check if profile is same as last frame
   * otherwise, update setup */
  if ((profileDrc != FDK_DRC_Generator_getDrcProfile(hDrcComp)) ||
      (profileComp != FDK_DRC_Generator_getCompProfile(hDrcComp))) {
    FDK_DRC_Generator_setDrcProfile(hDrcComp, profileDrc, profileComp);
  }

  /* Sanity check */
  if (profileComp == DRC_NONE) {
    pMetadata->etsiAncData.compression_value = 0x80; /* to ensure no external
                                                        values will be written
                                                        if not configured */
  }

  /* in case of embedding external values, copy this now (limiter may overwrite
   * them) */
  dynrng = decodeDynrng(pMetadata->mpegDrc.dyn_rng_ctl[0],
                        pMetadata->mpegDrc.dyn_rng_sgn[0]);
  compr = decodeCompr(pMetadata->etsiAncData.compression_value);

  dmxGain5 = decodeDynrng(pMetadata->etsiAncData.extAncData.dmx_gain_5_idx,
                          pMetadata->etsiAncData.extAncData.dmx_gain_5_sgn);
  dmxGain2 = decodeDynrng(pMetadata->etsiAncData.extAncData.dmx_gain_2_idx,
                          pMetadata->etsiAncData.extAncData.dmx_gain_2_sgn);

  /* Call compressor */
  if (FDK_DRC_Generator_Calc(
          hDrcComp, pSamples, samplesBufSize,
          progreflvl2dialnorm(pMetadata->mpegDrc.prog_ref_level),
          pMetadata->mpegDrc.drc_TargetRefLevel,
          pMetadata->etsiAncData.comp_TargetRefLevel,
          dmxTable[pMetadata->centerMixLevel],
          dmxTable[pMetadata->surroundMixLevel],
          dmxTable[pMetadata->etsiAncData.extAncData.ext_dmix_a_idx],
          dmxTable[pMetadata->etsiAncData.extAncData.ext_dmix_b_idx],
          pMetadata->etsiAncData.extAncData.ext_lfe_downmix_status
              ? dmxLfeTable[pMetadata->etsiAncData.extAncData.ext_dmix_lfe_idx]
              : (FIXP_DBL)0,
          dmxGain5, dmxGain2, &dynrng, &compr) != 0) {
    err = METADATA_ENCODE_ERROR;
    goto bail;
  }

  /* Write DRC values */
  pMetadata->mpegDrc.drc_band_incr = 0;
  encodeDynrng(dynrng, pMetadata->mpegDrc.dyn_rng_ctl,
               pMetadata->mpegDrc.dyn_rng_sgn);
  pMetadata->etsiAncData.compression_value = encodeCompr(compr);

bail:
  return err;
}

FDK_METADATA_ERROR FDK_MetadataEnc_Process(
    HANDLE_FDK_METADATA_ENCODER hMetaDataEnc, INT_PCM* const pAudioSamples,
    const UINT audioSamplesBufSize, const INT nAudioSamples,
    const AACENC_MetaData* const pMetadata,
    AACENC_EXT_PAYLOAD** ppMetaDataExtPayload, UINT* nMetaDataExtensions,
    INT* matrix_mixdown_idx) {
  FDK_METADATA_ERROR err = METADATA_OK;
  int metaDataDelayWriteIdx, metaDataDelayReadIdx, metadataMode;

  /* Where to write new meta data info */
  metaDataDelayWriteIdx = hMetaDataEnc->metaDataDelayIdx;

  /* How to write the data */
  metadataMode = hMetaDataEnc->metadataMode;

  /* Compensate meta data delay. */
  hMetaDataEnc->metaDataDelayIdx++;
  if (hMetaDataEnc->metaDataDelayIdx > hMetaDataEnc->nMetaDataDelay)
    hMetaDataEnc->metaDataDelayIdx = 0;

  /* Where to read pending meta data info from. */
  metaDataDelayReadIdx = hMetaDataEnc->metaDataDelayIdx;

  /* Submit new data if available. */
  if (pMetadata != NULL) {
    FDKmemcpy(&hMetaDataEnc->submittedMetaData, pMetadata,
              sizeof(AACENC_MetaData));
  }

  /* Write one additional frame with default configuration of meta data. Ensure
   * defined behaviour on decoder side. */
  if ((hMetaDataEnc->finalizeMetaData != 0) &&
      (hMetaDataEnc->metadataMode == 0)) {
    FDKmemcpy(&hMetaDataEnc->submittedMetaData, &defaultMetaDataSetup,
              sizeof(AACENC_MetaData));
    metadataMode = hMetaDataEnc->finalizeMetaData;
    hMetaDataEnc->finalizeMetaData = 0;
  }

  /* Get last submitted data. */
  if ((err = LoadSubmittedMetadata(
           &hMetaDataEnc->submittedMetaData, hMetaDataEnc->nChannels,
           metadataMode,
           &hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx])) !=
      METADATA_OK) {
    goto bail;
  }

  /* Calculate compressor if necessary and updata meta data info */
  if ((hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx].metadataMode == 1) ||
      (hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx].metadataMode == 2)) {
    if ((err = ProcessCompressor(
             &hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx],
             hMetaDataEnc->hDrcComp, pAudioSamples, audioSamplesBufSize,
             nAudioSamples)) != METADATA_OK) {
      /* Get last submitted data again. */
      LoadSubmittedMetadata(
          &hMetaDataEnc->submittedMetaData, hMetaDataEnc->nChannels,
          metadataMode, &hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx]);
    }
  }

  /* Fill up delay line with initial meta data info.*/
  if ((hMetaDataEnc->initializeMetaData != 0) &&
      (hMetaDataEnc->metadataMode != 0)) {
    int i;
    for (i = 0;
         i < (int)(sizeof(hMetaDataEnc->metaDataBuffer) / sizeof(AAC_METADATA));
         i++) {
      if (i != metaDataDelayWriteIdx) {
        FDKmemcpy(&hMetaDataEnc->metaDataBuffer[i],
                  &hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx],
                  sizeof(hMetaDataEnc->metaDataBuffer[metaDataDelayWriteIdx]));
      }
    }
    hMetaDataEnc->initializeMetaData = 0;
  }

  /* Convert Meta Data side info to bitstream data. */
  FDK_ASSERT(metaDataDelayReadIdx < MAX_DELAY_FRAMES);
  if ((err = WriteMetadataPayload(
           hMetaDataEnc,
           &hMetaDataEnc->metaDataBuffer[metaDataDelayReadIdx])) !=
      METADATA_OK) {
    goto bail;
  }

  /* Assign meta data to output */
  *ppMetaDataExtPayload = hMetaDataEnc->exPayload;
  *nMetaDataExtensions = hMetaDataEnc->nExtensions;
  *matrix_mixdown_idx = hMetaDataEnc->matrix_mixdown_idx;

bail:
  /* Compensate audio delay, reset err status. */
  err = CompensateAudioDelay(hMetaDataEnc, pAudioSamples, audioSamplesBufSize,
                             nAudioSamples / hMetaDataEnc->nChannels);

  return err;
}

static FDK_METADATA_ERROR CompensateAudioDelay(
    HANDLE_FDK_METADATA_ENCODER hMetaDataEnc, INT_PCM* const pAudioSamples,
    const UINT audioSamplesBufSize, const INT nAudioSamples) {
  FDK_METADATA_ERROR err = METADATA_OK;

  if (hMetaDataEnc->nAudioDataDelay) {
    C_ALLOC_SCRATCH_START(scratch_audioDelayBuffer, INT_PCM, 1024);

    for (int c = 0; c < hMetaDataEnc->nChannels; c++) {
      int M = 1024;
      INT_PCM* pAudioSamples2 = pAudioSamples + c * audioSamplesBufSize;
      int delayIdx = hMetaDataEnc->nAudioDataDelay;

      do {
        M = fMin(M, delayIdx);
        delayIdx -= M;

        FDKmemcpy(&scratch_audioDelayBuffer[0],
                  &pAudioSamples2[(nAudioSamples - M)], sizeof(INT_PCM) * M);
        FDKmemmove(&pAudioSamples2[M], &pAudioSamples2[0],
                   sizeof(INT_PCM) * (nAudioSamples - M));
        FDKmemcpy(
            &pAudioSamples2[0],
            &hMetaDataEnc->pAudioDelayBuffer[delayIdx +
                                             c * hMetaDataEnc->nAudioDataDelay],
            sizeof(INT_PCM) * M);
        FDKmemcpy(
            &hMetaDataEnc->pAudioDelayBuffer[delayIdx +
                                             c * hMetaDataEnc->nAudioDataDelay],
            &scratch_audioDelayBuffer[0], sizeof(INT_PCM) * M);

      } while (delayIdx > 0);
    }

    C_ALLOC_SCRATCH_END(scratch_audioDelayBuffer, INT_PCM, 1024);
  }

  return err;
}

/*-----------------------------------------------------------------------------

  functionname: WriteMetadataPayload
  description:  fills anc data and extension payload
  returns:      Error status

 ------------------------------------------------------------------------------*/
static FDK_METADATA_ERROR WriteMetadataPayload(
    const HANDLE_FDK_METADATA_ENCODER hMetaData,
    const AAC_METADATA* const pMetadata) {
  FDK_METADATA_ERROR err = METADATA_OK;

  if ((hMetaData == NULL) || (pMetadata == NULL)) {
    err = METADATA_INVALID_HANDLE;
    goto bail;
  }

  hMetaData->nExtensions = 0;
  hMetaData->matrix_mixdown_idx = -1;

  if (pMetadata->metadataMode != 0) {
    /* AAC-DRC */
    if ((pMetadata->metadataMode == 1) || (pMetadata->metadataMode == 2)) {
      hMetaData->exPayload[hMetaData->nExtensions].pData =
          hMetaData->drcInfoPayload;
      hMetaData->exPayload[hMetaData->nExtensions].dataType = EXT_DYNAMIC_RANGE;
      hMetaData->exPayload[hMetaData->nExtensions].associatedChElement = -1;

      hMetaData->exPayload[hMetaData->nExtensions].dataSize =
          WriteDynamicRangeInfoPayload(
              pMetadata, hMetaData->exPayload[hMetaData->nExtensions].pData);

      hMetaData->nExtensions++;
    } /* pMetadata->metadataMode==1 || pMetadata->metadataMode==2 */

    /* Matrix Mixdown Coefficient in PCE */
    if (pMetadata->WritePCEMixDwnIdx) {
      hMetaData->matrix_mixdown_idx =
          surmix2matrix_mixdown_idx[pMetadata->surroundMixLevel];
    }

    /* ETSI TS 101 154 (DVB) - MPEG4 ancillary_data() */
    if ((pMetadata->metadataMode == 2) ||
        (pMetadata->metadataMode == 3)) /* MP4_METADATA_MPEG_ETSI */
    {
      hMetaData->exPayload[hMetaData->nExtensions].pData =
          hMetaData->drcDsePayload;
      hMetaData->exPayload[hMetaData->nExtensions].dataType = EXT_DATA_ELEMENT;
      hMetaData->exPayload[hMetaData->nExtensions].associatedChElement = -1;

      hMetaData->exPayload[hMetaData->nExtensions].dataSize =
          WriteEtsiAncillaryDataPayload(
              pMetadata, hMetaData->exPayload[hMetaData->nExtensions].pData);

      hMetaData->nExtensions++;
    } /* metadataMode==2 || metadataMode==3 */

  } /* metadataMode != 0 */

bail:
  return err;
}

static INT WriteDynamicRangeInfoPayload(const AAC_METADATA* const pMetadata,
                                        UCHAR* const pExtensionPayload) {
  const INT pce_tag_present = 0; /* yet fixed setting! */
  const INT prog_ref_lev_res_bits = 0;
  INT i, drc_num_bands = 1;

  FDK_BITSTREAM bsWriter;
  FDKinitBitStream(&bsWriter, pExtensionPayload, 16, 0, BS_WRITER);

  /* dynamic_range_info() */
  FDKwriteBits(&bsWriter, pce_tag_present, 1); /* pce_tag_present */
  if (pce_tag_present) {
    FDKwriteBits(&bsWriter, 0x0, 4); /* pce_instance_tag */
    FDKwriteBits(&bsWriter, 0x0, 4); /* drc_tag_reserved_bits */
  }

  /* Exclude channels */
  FDKwriteBits(&bsWriter, (pMetadata->mpegDrc.excluded_chns_present) ? 1 : 0,
               1); /* excluded_chns_present*/

  /* Multiband DRC */
  FDKwriteBits(&bsWriter, (pMetadata->mpegDrc.drc_bands_present) ? 1 : 0,
               1); /* drc_bands_present */
  if (pMetadata->mpegDrc.drc_bands_present) {
    FDKwriteBits(&bsWriter, pMetadata->mpegDrc.drc_band_incr,
                 4); /* drc_band_incr */
    FDKwriteBits(&bsWriter, pMetadata->mpegDrc.drc_interpolation_scheme,
                 4); /* drc_interpolation_scheme */
    drc_num_bands += pMetadata->mpegDrc.drc_band_incr;
    for (i = 0; i < drc_num_bands; i++) {
      FDKwriteBits(&bsWriter, pMetadata->mpegDrc.drc_band_top[i],
                   8); /* drc_band_top */
    }
  }

  /* Program Reference Level */
  FDKwriteBits(&bsWriter, pMetadata->mpegDrc.prog_ref_level_present,
               1); /* prog_ref_level_present */
  if (pMetadata->mpegDrc.prog_ref_level_present) {
    FDKwriteBits(&bsWriter, pMetadata->mpegDrc.prog_ref_level,
                 7); /* prog_ref_level */
    FDKwriteBits(&bsWriter, prog_ref_lev_res_bits,
                 1); /* prog_ref_level_reserved_bits */
  }

  /* DRC Values */
  for (i = 0; i < drc_num_bands; i++) {
    FDKwriteBits(&bsWriter, (pMetadata->mpegDrc.dyn_rng_sgn[i]) ? 1 : 0,
                 1); /* dyn_rng_sgn[ */
    FDKwriteBits(&bsWriter, pMetadata->mpegDrc.dyn_rng_ctl[i],
                 7); /* dyn_rng_ctl */
  }

  /* return number of valid bits in extension payload. */
  return FDKgetValidBits(&bsWriter);
}

static INT WriteEtsiAncillaryDataPayload(const AAC_METADATA* const pMetadata,
                                         UCHAR* const pExtensionPayload) {
  FDK_BITSTREAM bsWriter;
  FDKinitBitStream(&bsWriter, pExtensionPayload, 16, 0, BS_WRITER);

  /* ancillary_data_sync */
  FDKwriteBits(&bsWriter, 0xBC, 8);

  /* bs_info */
  FDKwriteBits(&bsWriter, 0x3, 2); /* mpeg_audio_type */
  FDKwriteBits(&bsWriter, pMetadata->dolbySurroundMode,
               2); /* dolby_surround_mode */
  FDKwriteBits(&bsWriter, pMetadata->drcPresentationMode,
               2);                 /* DRC presentation mode */
  FDKwriteBits(&bsWriter, 0x0, 1); /* stereo_downmix_mode */
  FDKwriteBits(&bsWriter, 0x0, 1); /* reserved */

  /* ancillary_data_status */
  FDKwriteBits(&bsWriter, 0, 3); /* 3 bit Reserved, set to "0" */
  FDKwriteBits(&bsWriter, (pMetadata->DmxLvl_On) ? 1 : 0,
               1); /* downmixing_levels_MPEG4_status */
  FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncDataStatus,
               1); /* ext_anc_data_status */
  FDKwriteBits(&bsWriter, (pMetadata->etsiAncData.compression_on) ? 1 : 0,
               1); /* audio_coding_mode_and_compression status */
  FDKwriteBits(&bsWriter,
               (pMetadata->etsiAncData.timecode_coarse_status) ? 1 : 0,
               1); /* coarse_grain_timecode_status */
  FDKwriteBits(&bsWriter, (pMetadata->etsiAncData.timecode_fine_status) ? 1 : 0,
               1); /* fine_grain_timecode_status */

  /* downmixing_levels_MPEG4_status */
  if (pMetadata->DmxLvl_On) {
    FDKwriteBits(
        &bsWriter,
        encodeDmxLvls(pMetadata->centerMixLevel, pMetadata->surroundMixLevel),
        8);
  }

  /* audio_coding_mode_and_compression_status */
  if (pMetadata->etsiAncData.compression_on) {
    FDKwriteBits(&bsWriter, 0x01, 8); /* audio coding mode */
    FDKwriteBits(&bsWriter, pMetadata->etsiAncData.compression_value,
                 8); /* compression value */
  }

  /* grain-timecode coarse/fine */
  if (pMetadata->etsiAncData.timecode_coarse_status) {
    FDKwriteBits(&bsWriter, 0x0, 16); /* not yet supported */
  }

  if (pMetadata->etsiAncData.timecode_fine_status) {
    FDKwriteBits(&bsWriter, 0x0, 16); /* not yet supported */
  }

  /* extended ancillary data structure */
  if (pMetadata->etsiAncData.extAncDataStatus) {
    /* ext_ancillary_data_status */
    FDKwriteBits(&bsWriter, 0, 1); /* Reserved, set to "0" */
    FDKwriteBits(&bsWriter,
                 pMetadata->etsiAncData.extAncData.ext_downmix_lvl_status,
                 1); /* ext_downmixing_levels_status */
    FDKwriteBits(&bsWriter,
                 pMetadata->etsiAncData.extAncData.ext_downmix_gain_status,
                 1); /* ext_downmixing_global_gains_status */
    FDKwriteBits(&bsWriter,
                 pMetadata->etsiAncData.extAncData.ext_lfe_downmix_status,
                 1);               /* ext_downmixing_lfe_level_status" */
    FDKwriteBits(&bsWriter, 0, 4); /* Reserved, set to "0" */

    /* ext_downmixing_levels */
    if (pMetadata->etsiAncData.extAncData.ext_downmix_lvl_status) {
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.ext_dmix_a_idx,
                   3); /* dmix_a_idx */
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.ext_dmix_b_idx,
                   3);               /* dmix_b_idx */
      FDKwriteBits(&bsWriter, 0, 2); /* Reserved, set to "0" */
    }

    /* ext_downmixing_gains */
    if (pMetadata->etsiAncData.extAncData.ext_downmix_gain_status) {
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.dmx_gain_5_sgn,
                   1); /* dmx_gain_5_sign */
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.dmx_gain_5_idx,
                   6);               /* dmx_gain_5_idx */
      FDKwriteBits(&bsWriter, 0, 1); /* Reserved, set to "0" */
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.dmx_gain_2_sgn,
                   1); /* dmx_gain_2_sign */
      FDKwriteBits(&bsWriter, pMetadata->etsiAncData.extAncData.dmx_gain_2_idx,
                   6);               /* dmx_gain_2_idx */
      FDKwriteBits(&bsWriter, 0, 1); /* Reserved, set to "0" */
    }

    if (pMetadata->etsiAncData.extAncData.ext_lfe_downmix_status) {
      FDKwriteBits(&bsWriter,
                   pMetadata->etsiAncData.extAncData.ext_dmix_lfe_idx,
                   4);               /* dmix_lfe_idx */
      FDKwriteBits(&bsWriter, 0, 4); /* Reserved, set to "0" */
    }
  }

  return FDKgetValidBits(&bsWriter);
}

static FDK_METADATA_ERROR LoadSubmittedMetadata(
    const AACENC_MetaData* const hMetadata, const INT nChannels,
    const INT metadataMode, AAC_METADATA* const pAacMetaData) {
  FDK_METADATA_ERROR err = METADATA_OK;

  if (pAacMetaData == NULL) {
    err = METADATA_INVALID_HANDLE;
  } else {
    /* init struct */
    FDKmemclear(pAacMetaData, sizeof(AAC_METADATA));

    if (hMetadata != NULL) {
      /* convert data */
      pAacMetaData->mpegDrc.drc_profile = hMetadata->drc_profile;
      pAacMetaData->etsiAncData.comp_profile = hMetadata->comp_profile;
      pAacMetaData->mpegDrc.drc_TargetRefLevel = hMetadata->drc_TargetRefLevel;
      pAacMetaData->etsiAncData.comp_TargetRefLevel =
          hMetadata->comp_TargetRefLevel;
      pAacMetaData->mpegDrc.prog_ref_level_present =
          hMetadata->prog_ref_level_present;
      pAacMetaData->mpegDrc.prog_ref_level =
          dialnorm2progreflvl(hMetadata->prog_ref_level);

      pAacMetaData->centerMixLevel = hMetadata->centerMixLevel;
      pAacMetaData->surroundMixLevel = hMetadata->surroundMixLevel;
      pAacMetaData->WritePCEMixDwnIdx = hMetadata->PCE_mixdown_idx_present;
      pAacMetaData->DmxLvl_On = hMetadata->ETSI_DmxLvl_present;

      pAacMetaData->etsiAncData.compression_on =
          (hMetadata->comp_profile == AACENC_METADATA_DRC_NOT_PRESENT ? 0 : 1);

      if (pAacMetaData->mpegDrc.drc_profile ==
          AACENC_METADATA_DRC_NOT_PRESENT) {
        pAacMetaData->mpegDrc.drc_profile =
            AACENC_METADATA_DRC_NONE; /* MPEG DRC gains are
                                         always present in BS
                                         syntax */
        /* we should give a warning, but ErrorHandler does not support this */
      }

      if (nChannels == 2) {
        pAacMetaData->dolbySurroundMode =
            hMetadata->dolbySurroundMode; /* dolby_surround_mode */
      } else {
        pAacMetaData->dolbySurroundMode = 0;
      }

      pAacMetaData->drcPresentationMode = hMetadata->drcPresentationMode;
      /* override external values if DVB DRC presentation mode is given */
      if (pAacMetaData->drcPresentationMode == 1) {
        pAacMetaData->mpegDrc.drc_TargetRefLevel =
            fMax(-(31 << 16), pAacMetaData->mpegDrc.drc_TargetRefLevel);
        pAacMetaData->etsiAncData.comp_TargetRefLevel = fMax(
            -(20 << 16),
            pAacMetaData->etsiAncData.comp_TargetRefLevel); /* implies -23dB */
      }
      if (pAacMetaData->drcPresentationMode == 2) {
        pAacMetaData->mpegDrc.drc_TargetRefLevel =
            fMax(-(23 << 16), pAacMetaData->mpegDrc.drc_TargetRefLevel);
        pAacMetaData->etsiAncData.comp_TargetRefLevel =
            fMax(-(23 << 16), pAacMetaData->etsiAncData.comp_TargetRefLevel);
      }
      if (pAacMetaData->etsiAncData.comp_profile ==
          AACENC_METADATA_DRC_NOT_PRESENT) {
        /* DVB defines to revert to Light DRC if heavy is not present */
        if (pAacMetaData->drcPresentationMode != 0) {
          /* we exclude the "not indicated" mode as this requires the user to
           * define desired levels anyway */
          pAacMetaData->mpegDrc.drc_TargetRefLevel =
              fMax(pAacMetaData->etsiAncData.comp_TargetRefLevel,
                   pAacMetaData->mpegDrc.drc_TargetRefLevel);
        }
      }

      pAacMetaData->etsiAncData.timecode_coarse_status =
          0; /* not yet supported - attention: Update
                GetEstMetadataBytesPerFrame() if enable this! */
      pAacMetaData->etsiAncData.timecode_fine_status =
          0; /* not yet supported - attention: Update
                GetEstMetadataBytesPerFrame() if enable this! */

      /* extended ancillary data */
      pAacMetaData->etsiAncData.extAncDataStatus =
          ((hMetadata->ExtMetaData.extAncDataEnable == 1) ? 1 : 0);

      if (pAacMetaData->etsiAncData.extAncDataStatus) {
        pAacMetaData->etsiAncData.extAncData.ext_downmix_lvl_status =
            (hMetadata->ExtMetaData.extDownmixLevelEnable ? 1 : 0);
        pAacMetaData->etsiAncData.extAncData.ext_downmix_gain_status =
            (hMetadata->ExtMetaData.dmxGainEnable ? 1 : 0);
        pAacMetaData->etsiAncData.extAncData.ext_lfe_downmix_status =
            (hMetadata->ExtMetaData.lfeDmxEnable ? 1 : 0);

        pAacMetaData->etsiAncData.extAncData.ext_dmix_a_idx =
            hMetadata->ExtMetaData.extDownmixLevel_A;
        pAacMetaData->etsiAncData.extAncData.ext_dmix_b_idx =
            hMetadata->ExtMetaData.extDownmixLevel_B;

        if (pAacMetaData->etsiAncData.extAncData.ext_downmix_gain_status) {
          encodeDynrng(hMetadata->ExtMetaData.dmxGain5,
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_idx),
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_sgn));
          encodeDynrng(hMetadata->ExtMetaData.dmxGain2,
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_idx),
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_sgn));
        } else {
          encodeDynrng(1 << 16,
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_idx),
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_sgn));
          encodeDynrng(1 << 16,
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_idx),
                       &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_sgn));
        }

        if (pAacMetaData->etsiAncData.extAncData.ext_lfe_downmix_status) {
          pAacMetaData->etsiAncData.extAncData.ext_dmix_lfe_idx =
              hMetadata->ExtMetaData.lfeDmxLevel;
        } else {
          pAacMetaData->etsiAncData.extAncData.ext_dmix_lfe_idx =
              15; /* -inf dB */
        }
      } else {
        pAacMetaData->etsiAncData.extAncData.ext_downmix_lvl_status = 0;
        pAacMetaData->etsiAncData.extAncData.ext_downmix_gain_status = 0;
        pAacMetaData->etsiAncData.extAncData.ext_lfe_downmix_status = 0;

        pAacMetaData->etsiAncData.extAncData.ext_dmix_a_idx = 7; /* -inf dB */
        pAacMetaData->etsiAncData.extAncData.ext_dmix_b_idx = 7; /* -inf dB */

        encodeDynrng(1 << 16,
                     &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_idx),
                     &(pAacMetaData->etsiAncData.extAncData.dmx_gain_5_sgn));
        encodeDynrng(1 << 16,
                     &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_idx),
                     &(pAacMetaData->etsiAncData.extAncData.dmx_gain_2_sgn));

        pAacMetaData->etsiAncData.extAncData.ext_dmix_lfe_idx =
            15; /* -inf dB */
      }

      pAacMetaData->metadataMode = metadataMode;
    } else {
      pAacMetaData->metadataMode = 0; /* there is no configuration available */
    }
  }

  return err;
}

INT FDK_MetadataEnc_GetDelay(HANDLE_FDK_METADATA_ENCODER hMetadataEnc) {
  INT delay = 0;

  if (hMetadataEnc != NULL) {
    delay = hMetadataEnc->nAudioDataDelay;
  }

  return delay;
}
