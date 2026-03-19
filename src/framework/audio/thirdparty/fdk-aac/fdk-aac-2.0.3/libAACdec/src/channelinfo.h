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

/**************************** AAC decoder library ******************************

   Author(s):   Josef Hoepfl

   Description: individual channel stream info

*******************************************************************************/

#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include "common_fix.h"

#include "aac_rom.h"
#include "aacdecoder_lib.h"
#include "FDK_bitstream.h"
#include "overlapadd.h"

#include "mdct.h"
#include "stereo.h"
#include "pulsedata.h"
#include "aacdec_tns.h"

#include "aacdec_pns.h"

#include "aacdec_hcr_types.h"
#include "rvlc_info.h"

#include "usacdec_acelp.h"
#include "usacdec_const.h"
#include "usacdec_rom.h"

#include "ac_arith_coder.h"

#include "conceal_types.h"

#include "aacdec_drc_types.h"

#define WB_SECTION_SIZE (1024 * 2)

#define DRM_BS_BUFFER_SIZE                                                 \
  (512) /* size of the dynamic buffer which is used to reverse the bits of \
           the DRM SBR payload */

/* Output rendering mode */
typedef enum {
  AACDEC_RENDER_INVALID = 0,
  AACDEC_RENDER_IMDCT,
  AACDEC_RENDER_ELDFB,
  AACDEC_RENDER_LPD,
  AACDEC_RENDER_INTIMDCT
} AACDEC_RENDER_MODE;

enum { MAX_QUANTIZED_VALUE = 8191 };

typedef enum { FD_LONG, FD_SHORT, LPD } USAC_COREMODE;

typedef struct {
  const SHORT *ScaleFactorBands_Long;
  const SHORT *ScaleFactorBands_Short;
  UCHAR NumberOfScaleFactorBands_Long;
  UCHAR NumberOfScaleFactorBands_Short;
  UINT samplingRateIndex;
  UINT samplingRate;
} SamplingRateInfo;

typedef struct {
  UCHAR CommonWindow;
  UCHAR GlobalGain;

} CRawDataInfo;

typedef struct {
  UCHAR WindowGroupLength[8];
  UCHAR WindowGroups;
  UCHAR Valid;

  UCHAR WindowShape;         /* 0: sine window, 1: KBD, 2: low overlap */
  BLOCK_TYPE WindowSequence; /* mdct.h; 0: long, 1: start, 2: short, 3: stop */
  UCHAR MaxSfBands;
  UCHAR max_sfb_ste;
  UCHAR ScaleFactorGrouping;

  UCHAR TotalSfBands;

} CIcsInfo;

enum {
  ZERO_HCB = 0,
  ESCBOOK = 11,
  NSPECBOOKS = ESCBOOK + 1,
  BOOKSCL = NSPECBOOKS,
  NOISE_HCB = 13,
  INTENSITY_HCB2 = 14,
  INTENSITY_HCB = 15,
  LAST_HCB
};

/* This struct holds the persistent data shared by both channels of a CPE.
   It needs to be allocated for each CPE. */
typedef struct {
  CJointStereoPersistentData jointStereoPersistentData;
} CpePersistentData;

/*
 * This struct must be allocated one for every channel and must be persistent.
 */
typedef struct {
  FIXP_DBL *pOverlapBuffer;
  mdct_t IMdct;

  CArcoData *hArCo;

  INT pnsCurrentSeed;

  /* LPD memory */
  FIXP_DBL old_synth[PIT_MAX_MAX - L_SUBFR];
  INT old_T_pf[SYN_SFD];
  FIXP_DBL old_gain_pf[SYN_SFD];
  FIXP_DBL mem_bpf[L_FILT + L_SUBFR];
  UCHAR
  old_bpf_control_info; /* (1: enable, 0: disable) bpf for past superframe
                         */

  USAC_COREMODE last_core_mode; /* core mode used by the decoder in previous
                                   frame. (not signalled by the bitstream, see
                                   CAacDecoderChannelInfo::core_mode_last !! )
                                 */
  UCHAR last_lpd_mode;      /* LPD mode used by the decoder in last LPD subframe
                                (not signalled by the bitstream, see
                               CAacDecoderChannelInfo::lpd_mode_last !! ) */
  UCHAR last_last_lpd_mode; /* LPD mode used in second last LPD subframe
                                (not signalled by the bitstream) */
  UCHAR last_lpc_lost;      /* Flag indicating that the previous LPC is lost */

  FIXP_LPC
  lpc4_lsf[M_LP_FILTER_ORDER]; /* Last LPC4 coefficients in LSF domain. */
  FIXP_LPC lsf_adaptive_mean[M_LP_FILTER_ORDER]; /* Adaptive mean of LPC
                                                    coefficients in LSF domain
                                                    for concealment. */
  FIXP_LPC lp_coeff_old[2][M_LP_FILTER_ORDER];   /* Last LPC coefficients in LP
                                    domain. lp_coeff_old[0] is lpc4 (coeffs for
                                    right folding point of last tcx frame),
                                    lp_coeff_old[1] are coeffs for left folding
                                    point of last tcx frame */
  INT lp_coeff_old_exp[2];

  FIXP_SGL
  oldStability; /* LPC coeff stability value from last frame (required for
                   TCX concealment). */
  UINT numLostLpdFrames; /* Number of consecutive lost subframes. */

  /* TCX memory */
  FIXP_DBL last_tcx_gain;
  INT last_tcx_gain_e;
  FIXP_DBL last_alfd_gains[32]; /* Scaled by one bit. */
  SHORT last_tcx_pitch;
  UCHAR last_tcx_noise_factor;

  /* ACELP memory */
  CAcelpStaticMem acelp;

  ULONG nfRandomSeed; /* seed value for USAC noise filling random generator */

  CDrcChannelData drcData;
  CConcealmentInfo concealmentInfo;

  CpePersistentData *pCpeStaticData;

} CAacDecoderStaticChannelInfo;

/*
 * This union must be allocated for every element (up to 2 channels).
 */
typedef struct {
  /* Common bit stream data */
  SHORT aScaleFactor[(
      8 * 16)]; /* Spectral scale factors for each sfb in each window. */
  SHORT aSfbScale[(8 * 16)]; /* could be free after ApplyTools() */
  UCHAR
  aCodeBook[(8 * 16)]; /* section data: codebook for each window and sfb. */
  UCHAR band_is_noise[(8 * 16)];
  CTnsData TnsData;
  CRawDataInfo RawDataInfo;

  shouldBeUnion {
    struct {
      CPulseData PulseData;
      SHORT aNumLineInSec4Hcr[MAX_SFB_HCR]; /* needed once for all channels
                                               except for Drm syntax */
      UCHAR
      aCodeBooks4Hcr[MAX_SFB_HCR]; /* needed once for all channels except for
                                      Drm syntax. Same as "aCodeBook" ? */
      SHORT lenOfReorderedSpectralData;
      SCHAR lenOfLongestCodeword;
      SCHAR numberSection;
      SCHAR rvlcCurrentScaleFactorOK;
      SCHAR rvlcIntensityUsed;
    } aac;
    struct {
      UCHAR fd_noise_level_and_offset;
      UCHAR tns_active;
      UCHAR tns_on_lr;
      UCHAR tcx_noise_factor[4];
      UCHAR tcx_global_gain[4];
    } usac;
  }
  specificTo;

} CAacDecoderDynamicData;

typedef shouldBeUnion {
  UCHAR DrmBsBuffer[DRM_BS_BUFFER_SIZE];

  /* Common signal data, can be used once the bit stream data from above is not
   * used anymore. */
  FIXP_DBL mdctOutTemp[1024];

  FIXP_DBL synth_buf[(PIT_MAX_MAX + SYN_DELAY + L_FRAME_PLUS)];

  FIXP_DBL workBuffer[WB_SECTION_SIZE];
}
CWorkBufferCore1;

/* Common data referenced by all channels */
typedef struct {
  CAacDecoderDynamicData pAacDecoderDynamicData[2];

  CPnsInterChannelData pnsInterChannelData;
  INT pnsRandomSeed[(8 * 16)];

  CJointStereoData jointStereoData; /* One for one element */

  shouldBeUnion {
    struct {
      CErHcrInfo erHcrInfo;
      CErRvlcInfo erRvlcInfo;
      SHORT aRvlcScfEsc[RVLC_MAX_SFB]; /* needed once for all channels */
      SHORT aRvlcScfFwd[RVLC_MAX_SFB]; /* needed once for all channels */
      SHORT aRvlcScfBwd[RVLC_MAX_SFB]; /* needed once for all channels */
    } aac;
  }
  overlay;

} CAacDecoderCommonData;

typedef struct {
  CWorkBufferCore1 *pWorkBufferCore1;
  CCplxPredictionData *cplxPredictionData;
} CAacDecoderCommonStaticData;

/*
 * This struct must be allocated one for every channel of every element and must
 * be persistent. Among its members, the following memory areas can be
 * overwritten under the given conditions:
 *  - pSpectralCoefficient The memory pointed to can be overwritten after time
 * signal rendering.
 *  - data can be overwritten after time signal rendering.
 *  - pDynData memory pointed to can be overwritten after each
 * CChannelElement_Decode() call.
 *  - pComData->overlay memory pointed to can be overwritten after each
 * CChannelElement_Decode() call..
 */
typedef struct {
  shouldBeUnion {
    struct {
      FIXP_DBL fac_data0[LFAC];
      SCHAR fac_data_e[4];
      FIXP_DBL
      *fac_data[4]; /* Pointers to unused parts of pSpectralCoefficient */

      UCHAR core_mode; /* current core mode */
      USAC_COREMODE
      core_mode_last;      /* previous core mode, signalled in the bitstream
                              (not done by the decoder, see
                              CAacDecoderStaticChannelInfo::last_core_mode !!)*/
      UCHAR lpd_mode_last; /* previous LPD mode, signalled in the bitstream
                              (not done by the decoder, see
                              CAacDecoderStaticChannelInfo::last_core_mode !!)*/
      UCHAR mod[4];
      UCHAR bpf_control_info; /* (1: enable, 0: disable) bpf for current
                                 superframe */

      FIXP_LPC lsp_coeff[5][M_LP_FILTER_ORDER]; /* linear prediction
                                                   coefficients in LSP domain */
      FIXP_LPC
      lp_coeff[5][M_LP_FILTER_ORDER]; /* linear prediction coefficients in
                                         LP domain */
      INT lp_coeff_exp[5];
      FIXP_LPC lsf_adaptive_mean_cand
          [M_LP_FILTER_ORDER]; /* concealment: is copied to
                  CAacDecoderStaticChannelInfo->lsf_adaptive_mean once frame is
                  assumed to be correct*/
      FIXP_SGL aStability[4];  /* LPC coeff stability values required for ACELP
                                  and TCX (concealment) */

      CAcelpChannelData acelp[4];

      FIXP_DBL tcx_gain[4];
      SCHAR tcx_gain_e[4];
    } usac;

    struct {
      CPnsData PnsData; /* Not required for USAC */
    } aac;
  }
  data;

  SPECTRAL_PTR pSpectralCoefficient; /* Spectral coefficients of each window */
  SHORT specScale[8]; /* Scale shift values of each spectrum window */
  CIcsInfo icsInfo;
  INT granuleLength; /* Size of smallest spectrum piece */
  UCHAR ElementInstanceTag;

  AACDEC_RENDER_MODE renderMode; /* Output signal rendering mode */

  CAacDecoderDynamicData *
      pDynData; /* Data required for one element and discarded after decoding */
  CAacDecoderCommonData
      *pComData; /* Data required for one channel at a time during decode */
  CAacDecoderCommonStaticData *pComStaticData; /* Persistent data required for
                                                  one channel at a time during
                                                  decode */

  int currAliasingSymmetry; /* required for RSVD60 MCT */

} CAacDecoderChannelInfo;

/* channelinfo.cpp */

AAC_DECODER_ERROR getSamplingRateInfo(SamplingRateInfo *t, UINT samplesPerFrame,
                                      UINT samplingRateIndex,
                                      UINT samplingRate);

/**
 * \brief Read max SFB from bit stream and assign TotalSfBands according
 *        to the window sequence and sample rate.
 * \param hBs bit stream handle as data source
 * \param pIcsInfo IcsInfo structure to read the window sequence and store
 * MaxSfBands and TotalSfBands
 * \param pSamplingRateInfo read only
 */
AAC_DECODER_ERROR IcsReadMaxSfb(HANDLE_FDK_BITSTREAM hBs, CIcsInfo *pIcsInfo,
                                const SamplingRateInfo *pSamplingRateInfo);

AAC_DECODER_ERROR IcsRead(HANDLE_FDK_BITSTREAM bs, CIcsInfo *pIcsInfo,
                          const SamplingRateInfo *SamplingRateInfoTable,
                          const UINT flags);

/* stereo.cpp, only called from this file */

/*!
  \brief Applies MS stereo.

  The function applies MS stereo.

  \param pAacDecoderChannelInfo aac channel info.
  \param pScaleFactorBandOffsets pointer to scalefactor band offsets.
  \param pWindowGroupLength pointer to window group length array.
  \param windowGroups number of window groups.
  \param scaleFactorBandsTransmittedL number of transmitted scalefactor bands in
  left channel. \param scaleFactorBandsTransmittedR number of transmitted
  scalefactor bands in right channel. May differ from
  scaleFactorBandsTransmittedL only for USAC. \return  none
*/
void CJointStereo_ApplyMS(
    CAacDecoderChannelInfo *pAacDecoderChannelInfo[2],
    CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo[2],
    FIXP_DBL *spectrumL, FIXP_DBL *spectrumR, SHORT *SFBleftScale,
    SHORT *SFBrightScale, SHORT *specScaleL, SHORT *specScaleR,
    const SHORT *pScaleFactorBandOffsets, const UCHAR *pWindowGroupLength,
    const int windowGroups, const int max_sfb_ste_outside,
    const int scaleFactorBandsTransmittedL,
    const int scaleFactorBandsTransmittedR, FIXP_DBL *store_dmx_re_prev,
    SHORT *store_dmx_re_prev_e, const int mainband_flag);

/*!
  \brief Applies intensity stereo

  The function applies intensity stereo.

  \param pAacDecoderChannelInfo aac channel info.
  \param pScaleFactorBandOffsets pointer to scalefactor band offsets.
  \param pWindowGroupLength pointer to window group length array.
  \param windowGroups number of window groups.
  \param scaleFactorBandsTransmitted number of transmitted scalefactor bands.
  \return  none
*/
void CJointStereo_ApplyIS(CAacDecoderChannelInfo *pAacDecoderChannelInfo[2],
                          const short *pScaleFactorBandOffsets,
                          const UCHAR *pWindowGroupLength,
                          const int windowGroups,
                          const int scaleFactorBandsTransmitted);

/* aacdec_pns.cpp */
int CPns_IsPnsUsed(const CPnsData *pPnsData, const int group, const int band);

void CPns_SetCorrelation(CPnsData *pPnsData, const int group, const int band,
                         const int outofphase);

/****************** inline functions ******************/

inline UCHAR IsValid(const CIcsInfo *pIcsInfo) { return pIcsInfo->Valid; }

inline UCHAR IsLongBlock(const CIcsInfo *pIcsInfo) {
  return (pIcsInfo->WindowSequence != BLOCK_SHORT);
}

inline UCHAR GetWindowShape(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->WindowShape;
}

inline BLOCK_TYPE GetWindowSequence(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->WindowSequence;
}

inline const SHORT *GetScaleFactorBandOffsets(
    const CIcsInfo *pIcsInfo, const SamplingRateInfo *samplingRateInfo) {
  if (IsLongBlock(pIcsInfo)) {
    return samplingRateInfo->ScaleFactorBands_Long;
  } else {
    return samplingRateInfo->ScaleFactorBands_Short;
  }
}

inline UCHAR GetNumberOfScaleFactorBands(
    const CIcsInfo *pIcsInfo, const SamplingRateInfo *samplingRateInfo) {
  if (IsLongBlock(pIcsInfo)) {
    return samplingRateInfo->NumberOfScaleFactorBands_Long;
  } else {
    return samplingRateInfo->NumberOfScaleFactorBands_Short;
  }
}

inline int GetWindowsPerFrame(const CIcsInfo *pIcsInfo) {
  return (pIcsInfo->WindowSequence == BLOCK_SHORT) ? 8 : 1;
}

inline UCHAR GetWindowGroups(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->WindowGroups;
}

inline UCHAR GetWindowGroupLength(const CIcsInfo *pIcsInfo, const INT index) {
  return pIcsInfo->WindowGroupLength[index];
}

inline const UCHAR *GetWindowGroupLengthTable(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->WindowGroupLength;
}

inline UCHAR GetScaleFactorBandsTransmitted(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->MaxSfBands;
}

inline UCHAR GetScaleMaxFactorBandsTransmitted(const CIcsInfo *pIcsInfo0,
                                               const CIcsInfo *pIcsInfo1) {
  return fMax(pIcsInfo0->MaxSfBands, pIcsInfo1->MaxSfBands);
}

inline UCHAR GetScaleFactorBandsTotal(const CIcsInfo *pIcsInfo) {
  return pIcsInfo->TotalSfBands;
}

/* Note: This function applies to AAC-LC only ! */
inline UCHAR GetMaximumTnsBands(const CIcsInfo *pIcsInfo,
                                const int samplingRateIndex) {
  return tns_max_bands_tbl[samplingRateIndex][!IsLongBlock(pIcsInfo)];
}

#endif /* #ifndef CHANNELINFO_H */
