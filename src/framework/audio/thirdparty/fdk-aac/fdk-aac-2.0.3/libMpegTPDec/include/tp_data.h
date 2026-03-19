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

/******************* MPEG transport format decoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport data tables

*******************************************************************************/

#ifndef TP_DATA_H
#define TP_DATA_H

#include "machine_type.h"
#include "FDK_audio.h"
#include "FDK_bitstream.h"

/*
 * Configuration
 */

#define TP_USAC_MAX_SPEAKERS (24)

#define TP_USAC_MAX_EXT_ELEMENTS ((24))

#define TP_USAC_MAX_ELEMENTS ((24) + TP_USAC_MAX_EXT_ELEMENTS)

#define TP_USAC_MAX_CONFIG_LEN                                         \
  512 /* next power of two of maximum of escapedValue(hBs, 4, 4, 8) in \
         AudioPreRoll() (285) */

#define TPDEC_USAC_NUM_CONFIG_CHANGE_FRAMES \
  (1) /* Number of frames for config change in USAC */

enum {
  TPDEC_FLUSH_OFF = 0,
  TPDEC_RSV60_CFG_CHANGE_ATSC_FLUSH_ON = 1,
  TPDEC_RSV60_DASH_IPF_ATSC_FLUSH_ON = 2,
  TPDEC_USAC_DASH_IPF_FLUSH_ON = 3
};

enum {
  TPDEC_BUILD_UP_OFF = 0,
  TPDEC_RSV60_BUILD_UP_ON = 1,
  TPDEC_RSV60_BUILD_UP_ON_IN_BAND = 2,
  TPDEC_USAC_BUILD_UP_ON = 3,
  TPDEC_RSV60_BUILD_UP_IDLE = 4,
  TPDEC_RSV60_BUILD_UP_IDLE_IN_BAND = 5
};

/**
 * ProgramConfig struct.
 */
/* ISO/IEC 14496-3 4.4.1.1 Table 4.2 Program config element */
#define PC_FSB_CHANNELS_MAX 16 /* Front/Side/Back channels */
#define PC_LFE_CHANNELS_MAX 4
#define PC_ASSOCDATA_MAX 8
#define PC_CCEL_MAX 16 /* CC elements */
#define PC_COMMENTLENGTH 256
#define PC_NUM_HEIGHT_LAYER 3

typedef struct {
  /* PCE bitstream elements: */
  UCHAR ElementInstanceTag;
  UCHAR Profile;
  UCHAR SamplingFrequencyIndex;
  UCHAR NumFrontChannelElements;
  UCHAR NumSideChannelElements;
  UCHAR NumBackChannelElements;
  UCHAR NumLfeChannelElements;
  UCHAR NumAssocDataElements;
  UCHAR NumValidCcElements;

  UCHAR MonoMixdownPresent;
  UCHAR MonoMixdownElementNumber;

  UCHAR StereoMixdownPresent;
  UCHAR StereoMixdownElementNumber;

  UCHAR MatrixMixdownIndexPresent;
  UCHAR MatrixMixdownIndex;
  UCHAR PseudoSurroundEnable;

  UCHAR FrontElementIsCpe[PC_FSB_CHANNELS_MAX];
  UCHAR FrontElementTagSelect[PC_FSB_CHANNELS_MAX];
  UCHAR FrontElementHeightInfo[PC_FSB_CHANNELS_MAX];

  UCHAR SideElementIsCpe[PC_FSB_CHANNELS_MAX];
  UCHAR SideElementTagSelect[PC_FSB_CHANNELS_MAX];
  UCHAR SideElementHeightInfo[PC_FSB_CHANNELS_MAX];

  UCHAR BackElementIsCpe[PC_FSB_CHANNELS_MAX];
  UCHAR BackElementTagSelect[PC_FSB_CHANNELS_MAX];
  UCHAR BackElementHeightInfo[PC_FSB_CHANNELS_MAX];

  UCHAR LfeElementTagSelect[PC_LFE_CHANNELS_MAX];

  UCHAR AssocDataElementTagSelect[PC_ASSOCDATA_MAX];

  UCHAR CcElementIsIndSw[PC_CCEL_MAX];
  UCHAR ValidCcElementTagSelect[PC_CCEL_MAX];

  UCHAR CommentFieldBytes;
  UCHAR Comment[PC_COMMENTLENGTH];

  /* Helper variables for administration: */
  UCHAR isValid; /*!< Flag showing if PCE has been read successfully. */
  UCHAR
  NumChannels; /*!< Amount of audio channels summing all channel elements
                  including LFEs */
  UCHAR NumEffectiveChannels; /*!< Amount of audio channels summing only SCEs
                                 and CPEs */
  UCHAR elCounter;

} CProgramConfig;

typedef enum {
  ASCEXT_UNKOWN = -1,
  ASCEXT_SBR = 0x2b7,
  ASCEXT_PS = 0x548,
  ASCEXT_MPS = 0x76a,
  ASCEXT_SAOC = 0x7cb,
  ASCEXT_LDMPS = 0x7cc

} TP_ASC_EXTENSION_ID;

/**
 * GaSpecificConfig struct
 */
typedef struct {
  UINT m_frameLengthFlag;
  UINT m_dependsOnCoreCoder;
  UINT m_coreCoderDelay;

  UINT m_extensionFlag;
  UINT m_extensionFlag3;

  UINT m_layer;
  UINT m_numOfSubFrame;
  UINT m_layerLength;

} CSGaSpecificConfig;

typedef enum {
  ELDEXT_TERM = 0x0,         /* Termination tag */
  ELDEXT_SAOC = 0x1,         /* SAOC config */
  ELDEXT_LDSAC = 0x2,        /* LD MPEG Surround config */
  ELDEXT_DOWNSCALEINFO = 0x3 /* ELD sample rate adaptation */
  /* reserved */
} ASC_ELD_EXT_TYPE;

typedef struct {
  UCHAR m_frameLengthFlag;

  UCHAR m_sbrPresentFlag;
  UCHAR
  m_useLdQmfTimeAlign; /* Use LD-MPS QMF in SBR to achive time alignment */
  UCHAR m_sbrSamplingRate;
  UCHAR m_sbrCrcFlag;
  UINT m_downscaledSamplingFrequency;

} CSEldSpecificConfig;

typedef struct {
  USAC_EXT_ELEMENT_TYPE usacExtElementType;
  USHORT usacExtElementConfigLength;
  USHORT usacExtElementDefaultLength;
  UCHAR usacExtElementPayloadFrag;
  UCHAR usacExtElementHasAudioPreRoll;
} CSUsacExtElementConfig;

typedef struct {
  MP4_ELEMENT_ID usacElementType;
  UCHAR m_noiseFilling;
  UCHAR m_harmonicSBR;
  UCHAR m_interTes;
  UCHAR m_pvc;
  UCHAR m_stereoConfigIndex;
  CSUsacExtElementConfig extElement;
} CSUsacElementConfig;

typedef struct {
  UCHAR m_frameLengthFlag;
  UCHAR m_coreSbrFrameLengthIndex;
  UCHAR m_sbrRatioIndex;
  UCHAR m_nUsacChannels; /* number of audio channels signaled in
                            UsacDecoderConfig() / rsv603daDecoderConfig() via
                            numElements and usacElementType */
  UCHAR m_channelConfigurationIndex;
  UINT m_usacNumElements;
  CSUsacElementConfig element[TP_USAC_MAX_ELEMENTS];

  UCHAR numAudioChannels;
  UCHAR m_usacConfigExtensionPresent;
  UCHAR elementLengthPresent;
  UCHAR UsacConfig[TP_USAC_MAX_CONFIG_LEN];
  USHORT UsacConfigBits;
} CSUsacConfig;

/**
 * Audio configuration struct, suitable for encoder and decoder configuration.
 */
typedef struct {
  /* XYZ Specific Data */
  union {
    CSGaSpecificConfig
        m_gaSpecificConfig; /**< General audio specific configuration. */
    CSEldSpecificConfig m_eldSpecificConfig; /**< ELD specific configuration. */
    CSUsacConfig m_usacConfig; /**< USAC specific configuration               */
  } m_sc;

  /* Common ASC parameters */
  CProgramConfig m_progrConfigElement; /**< Program configuration. */

  AUDIO_OBJECT_TYPE m_aot;  /**< Audio Object Type.  */
  UINT m_samplingFrequency; /**< Samplerate. */
  UINT m_samplesPerFrame;   /**< Amount of samples per frame.   */
  UINT m_directMapping; /**< Document this please !!                         */

  AUDIO_OBJECT_TYPE m_extensionAudioObjectType; /**< Audio object type */
  UINT m_extensionSamplingFrequency;            /**< Samplerate            */

  SCHAR m_channelConfiguration; /**< Channel configuration index */

  SCHAR m_epConfig;  /**< Error protection index                           */
  SCHAR m_vcb11Flag; /**< aacSectionDataResilienceFlag                     */
  SCHAR m_rvlcFlag;  /**< aacScalefactorDataResilienceFlag                 */
  SCHAR m_hcrFlag;   /**< aacSpectralDataResilienceFlag                    */

  SCHAR m_sbrPresentFlag; /**< Flag indicating the presence of SBR data in the
                             bitstream               */
  SCHAR
  m_psPresentFlag; /**< Flag indicating the presence of parametric stereo
                      data in the bitstream */
  UCHAR m_samplingFrequencyIndex;          /**< Samplerate index          */
  UCHAR m_extensionSamplingFrequencyIndex; /**< Samplerate index */
  SCHAR m_extensionChannelConfiguration;   /**< Channel configuration index   */

  UCHAR
  configMode; /**< The flag indicates if the callback shall work in memory
                 allocation mode or in config change detection mode */
  UCHAR AacConfigChanged; /**< The flag will be set if at least one aac config
                             parameter has changed that requires a memory
                             reconfiguration, otherwise it will be cleared */
  UCHAR SbrConfigChanged; /**< The flag will be set if at least one sbr config
                             parameter has changed that requires a memory
                             reconfiguration, otherwise it will be cleared */
  UCHAR SacConfigChanged; /**< The flag will be set if at least one sac config
                             parameter has changed that requires a memory
                             reconfiguration, otherwise it will be cleared */

  UCHAR
  config[TP_USAC_MAX_CONFIG_LEN]; /**< Configuration stored as bitstream */
  UINT configBits;                /**< Configuration length in bits */

} CSAudioSpecificConfig;

typedef struct {
  SCHAR flushCnt;      /**< Flush frame counter */
  UCHAR flushStatus;   /**< Flag indicates flush mode: on|off */
  SCHAR buildUpCnt;    /**< Build up frame counter */
  UCHAR buildUpStatus; /**< Flag indicates build up mode: on|off */
  UCHAR cfgChanged; /**< Flag indicates that the config changed and the decoder
                       needs to be initialized again via callback. Make sure
                       that memory is freed before initialization. */
  UCHAR contentChanged; /**< Flag indicates that the content changed i.e. a
                           right truncation occured before */
  UCHAR forceCfgChange; /**< Flag indicates if config change has to be forced
                           even if new config is the same */
} CCtrlCFGChange;

typedef INT (*cbUpdateConfig_t)(void *, const CSAudioSpecificConfig *,
                                const UCHAR configMode, UCHAR *configChanged);
typedef INT (*cbFreeMem_t)(void *, const CSAudioSpecificConfig *);
typedef INT (*cbCtrlCFGChange_t)(void *, const CCtrlCFGChange *);
typedef INT (*cbSsc_t)(void *, HANDLE_FDK_BITSTREAM,
                       const AUDIO_OBJECT_TYPE coreCodec,
                       const INT samplingRate, const INT frameSize,
                       const INT numChannels, const INT stereoConfigIndex,
                       const INT coreSbrFrameLengthIndex, const INT configBytes,
                       const UCHAR configMode, UCHAR *configChanged);

typedef INT (*cbSbr_t)(void *self, HANDLE_FDK_BITSTREAM hBs,
                       const INT sampleRateIn, const INT sampleRateOut,
                       const INT samplesPerFrame,
                       const AUDIO_OBJECT_TYPE coreCodec,
                       const MP4_ELEMENT_ID elementID, const INT elementIndex,
                       const UCHAR harmonicSbr, const UCHAR stereoConfigIndex,
                       const UCHAR configMode, UCHAR *configChanged,
                       const INT downscaleFactor);

typedef INT (*cbUsac_t)(void *self, HANDLE_FDK_BITSTREAM hBs);

typedef INT (*cbUniDrc_t)(void *self, HANDLE_FDK_BITSTREAM hBs,
                          const INT fullPayloadLength, const INT payloadType,
                          const INT subStreamIndex, const INT payloadStart,
                          const AUDIO_OBJECT_TYPE);

typedef struct {
  cbUpdateConfig_t cbUpdateConfig; /*!< Function pointer for Config change
                                      notify callback.  */
  void *cbUpdateConfigData; /*!< User data pointer for Config change notify
                               callback. */
  cbFreeMem_t cbFreeMem;    /*!< Function pointer for free memory callback.  */
  void *cbFreeMemData;      /*!< User data pointer for free memory callback. */
  cbCtrlCFGChange_t cbCtrlCFGChange; /*!< Function pointer for config change
                                        control callback. */
  void *cbCtrlCFGChangeData; /*!< User data pointer for config change control
                                callback. */
  cbSsc_t cbSsc;             /*!< Function pointer for SSC parser callback. */
  void *cbSscData;           /*!< User data pointer for SSC parser callback. */
  cbSbr_t cbSbr;   /*!< Function pointer for SBR header parser callback. */
  void *cbSbrData; /*!< User data pointer for SBR header parser callback. */
  cbUsac_t cbUsac;
  void *cbUsacData;
  cbUniDrc_t cbUniDrc; /*!< Function pointer for uniDrcConfig and
                          loudnessInfoSet parser callback. */
  void *cbUniDrcData;  /*!< User data pointer for uniDrcConfig and
                          loudnessInfoSet parser callback. */
} CSTpCallBacks;

static const UINT SamplingRateTable[] = {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025,
    8000,  7350,  0,     0,     57600, 51200, 40000, 38400, 34150, 28800, 25600,
    20000, 19200, 17075, 14400, 12800, 9600,  0,     0,     0,     0};

static inline int getSamplingRateIndex(UINT samplingRate, UINT nBits) {
  UINT sf_index;
  UINT tableSize = (1 << nBits) - 1;

  for (sf_index = 0; sf_index < tableSize; sf_index++) {
    if (SamplingRateTable[sf_index] == samplingRate) break;
  }

  if (sf_index > tableSize) {
    return tableSize - 1;
  }

  return sf_index;
}

/*
 * Get Channel count from channel configuration
 */
static inline int getNumberOfTotalChannels(int channelConfig) {
  switch (channelConfig) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return channelConfig;
    case 7:
    case 12:
    case 14:
      return 8;
    case 11:
      return 7;
    case 13:
      return 24;
    default:
      return 0;
  }
}

static inline int getNumberOfEffectiveChannels(
    const int
        channelConfig) { /* index: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 */
  const int n[] = {0, 1, 2, 3, 4, 5, 5, 7, 0, 0, 0, 6, 7, 22, 7, 0};
  return n[channelConfig];
}

#endif /* TP_DATA_H */
