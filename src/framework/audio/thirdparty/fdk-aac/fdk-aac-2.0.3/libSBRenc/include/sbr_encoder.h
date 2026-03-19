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

/**************************** SBR encoder library ******************************

   Author(s):

   Description: SBR encoder top level processing prototype

*******************************************************************************/

#ifndef SBR_ENCODER_H
#define SBR_ENCODER_H

#include "common_fix.h"
#include "FDK_audio.h"

#include "FDK_bitstream.h"

/* core coder helpers */
#define MAX_TRANS_FAC 8
#define MAX_CODEC_FRAME_RATIO 2
#define MAX_PAYLOAD_SIZE 256

typedef enum codecType {
  CODEC_AAC = 0,
  CODEC_AACLD = 1,
  CODEC_UNSPECIFIED = 99
} CODEC_TYPE;

typedef struct {
  INT bitRate;
  INT nChannels;
  INT sampleFreq;
  INT transFac;
  INT standardBitrate;
} CODEC_PARAM;

typedef enum {
  SBR_MONO,
  SBR_LEFT_RIGHT,
  SBR_COUPLING,
  SBR_SWITCH_LRC
} SBR_STEREO_MODE;

/* bitstream syntax flags */
enum {
  SBR_SYNTAX_LOW_DELAY = 0x0001,
  SBR_SYNTAX_SCALABLE = 0x0002,
  SBR_SYNTAX_CRC = 0x0004,
  SBR_SYNTAX_DRM_CRC = 0x0008,
  SBR_SYNTAX_ELD_REDUCED_DELAY = 0x0010
};

typedef enum { FREQ_RES_LOW = 0, FREQ_RES_HIGH } FREQ_RES;

typedef struct {
  CODEC_TYPE coreCoder; /*!< LC or ELD */
  UINT bitrateFrom;     /*!< inclusive */
  UINT bitrateTo;       /*!< exclusive */

  UINT sampleRate;   /*!<   */
  UCHAR numChannels; /*!<   */

  UCHAR startFreq;       /*!< bs_start_freq */
  UCHAR startFreqSpeech; /*!< bs_start_freq for speech config flag */
  UCHAR stopFreq;        /*!< bs_stop_freq */
  UCHAR stopFreqSpeech;  /*!< bs_stop_freq for speech config flag */

  UCHAR numNoiseBands;        /*!<   */
  UCHAR noiseFloorOffset;     /*!<   */
  SCHAR noiseMaxLevel;        /*!<   */
  SBR_STEREO_MODE stereoMode; /*!<   */
  UCHAR freqScale;            /*!<   */
} sbrTuningTable_t;

typedef struct sbrConfiguration {
  /*
     core coder dependent configurations
  */
  CODEC_PARAM
  codecSettings; /*!< Core coder settings. To be set from core coder. */
  INT SendHeaderDataTime; /*!< SBR header send update frequency in ms. */
  INT useWaveCoding;      /*!< Flag: usage of wavecoding tool. */
  INT crcSbr;             /*!< Flag: usage of SBR-CRC. */
  INT dynBwSupported;     /*!< Flag: support for dynamic bandwidth in this
                             combination. */
  INT parametricCoding;   /*!< Flag: usage of parametric coding tool. */
  INT downSampleFactor; /*!< Sampling rate relation between the SBR and the core
                           encoder. */
  FREQ_RES freq_res_fixfix[2]; /*!< Frequency resolution of envelopes in frame
                                  class FIXFIX, for non-split case and split
                                  case */
  UCHAR fResTransIsLow; /*!< Frequency resolution of envelopes in transient
                           frames: low (0) or variable (1) */

  /*
     core coder dependent tuning parameters
  */
  INT tran_thr;         /*!< SBR transient detector threshold (* 100). */
  INT noiseFloorOffset; /*!< Noise floor offset.      */
  UINT useSpeechConfig; /*!< Flag: adapt tuning parameters according to speech.
                         */

  /*
     core coder independent configurations
  */
  INT sbrFrameSize; /*!< SBR frame size in samples. Will be calculated from core
                       coder settings. */
  INT sbr_data_extra; /*!< Flag usage of data extra. */
  INT amp_res;        /*!< Amplitude resolution. */
  INT ana_max_level;  /*!< Noise insertion maximum level. */
  INT tran_fc;        /*!< Transient detector start frequency. */
  INT tran_det_mode;  /*!< Transient detector mode. */
  INT spread;         /*!< Flag: usage of SBR spread. */
  INT stat;           /*!< Flag: usage of static framing. */
  INT e;              /*!< Number of envelopes when static framing is chosen. */
  SBR_STEREO_MODE stereoMode; /*!< SBR stereo mode. */
  INT deltaTAcrossFrames;     /*!< Flag: allow time-delta coding. */
  FIXP_DBL dF_edge_1stEnv; /*!< Extra fraction delta-F coding is allowed to be
                              more expensive. */
  FIXP_DBL dF_edge_incr;   /*!< Increment dF_edge_1stEnv this much if dT-coding
                              was used this frame. */
  INT sbr_invf_mode;       /*!< Inverse filtering mode. */
  INT sbr_xpos_mode;       /*!< Transposer mode. */
  INT sbr_xpos_ctrl;       /*!< Transposer control. */
  INT sbr_xpos_level;      /*!< Transposer 3rd order level. */
  INT startFreq;           /*!< The start frequency table index. */
  INT stopFreq;            /*!< The stop frequency table index. */
  INT useSaPan;            /*!< Flag: usage of SAPAN stereo. */
  INT dynBwEnabled;        /*!< Flag: usage of dynamic bandwidth. */
  INT bParametricStereo;   /*!< Flag: usage of parametric stereo coding tool. */

  /*
     header_extra1 configuration
  */
  UCHAR freqScale;     /*!< Frequency grouping. */
  INT alterScale;      /*!< Scale resolution. */
  INT sbr_noise_bands; /*!< Number of noise bands. */

  /*
     header_extra2 configuration
  */
  INT sbr_limiter_bands;    /*!< Number of limiter bands. */
  INT sbr_limiter_gains;    /*!< Gain of limiter. */
  INT sbr_interpol_freq;    /*!< Flag: use interpolation in freq. direction. */
  INT sbr_smoothing_length; /*!< Flag: choose length 4 or 0 (=on, off). */
  UCHAR init_amp_res_FF;
  FIXP_DBL threshold_AmpRes_FF_m;
  SCHAR threshold_AmpRes_FF_e;
} sbrConfiguration, *sbrConfigurationPtr;

typedef struct SBR_CONFIG_DATA {
  UINT sbrSyntaxFlags; /**< SBR syntax flags derived from AOT. */
  INT nChannels;       /**< Number of channels.  */

  INT nSfb[2]; /**< Number of SBR scalefactor bands for LO_RES and HI_RES (?) */
  INT num_Master; /**< Number of elements in v_k_master. */
  INT sampleFreq; /**< SBR sampling frequency. */
  INT frameSize;
  INT xOverFreq;    /**< The SBR start frequency. */
  INT dynXOverFreq; /**< Used crossover frequency when dynamic bandwidth is
                       enabled. */

  INT noQmfBands; /**< Number of QMF frequency bands. */
  INT noQmfSlots; /**< Number of QMF slots. */

  UCHAR *freqBandTable[2]; /**< Frequency table for low and hires, only
                              MAX_FREQ_COEFFS/2 +1 coeffs actually needed for
                              lowres. */
  UCHAR
  *v_k_master; /**< Master BandTable where freqBandTable is derived from. */

  SBR_STEREO_MODE stereoMode;
  INT noEnvChannels; /**< Number of envelope channels. */

  INT useWaveCoding; /**< Flag indicates whether to use wave coding at all. */
  INT useParametricCoding; /**< Flag indicates whether to use para coding at
                              all.      */
  INT xposCtrlSwitch;    /**< Flag indicates whether to switch xpos ctrl on the
                            fly. */
  INT switchTransposers; /**< Flag indicates whether to switch xpos on the fly .
                          */
  UCHAR initAmpResFF;
  FIXP_DBL thresholdAmpResFF_m;
  SCHAR thresholdAmpResFF_e;
} SBR_CONFIG_DATA, *HANDLE_SBR_CONFIG_DATA;

typedef struct {
  MP4_ELEMENT_ID elType;
  INT bitRate;
  int instanceTag;
  UCHAR fParametricStereo;
  UCHAR fDualMono; /**< This flags allows to disable coupling in sbr channel
                      pair element */
  UCHAR nChannelsInEl;
  UCHAR ChannelIndex[2];
} SBR_ELEMENT_INFO;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SBR_ENCODER *HANDLE_SBR_ENCODER;

/**
 * \brief  Get the max required input buffer size including delay balancing
 * space for N audio channels.
 * \param noChannels  Number of audio channels.
 * \return            Max required input buffer size in bytes.
 */
INT sbrEncoder_GetInBufferSize(int noChannels);

INT sbrEncoder_Open(HANDLE_SBR_ENCODER *phSbrEncoder, INT nElements,
                    INT nChannels, INT supportPS);

/**
 * \brief                 Get closest working bitrate to specified desired
 *                        bitrate for a single SBR element.
 * \param bitRate         The desired target bit rate
 * \param numChannels     The amount of audio channels
 * \param coreSampleRate  The sample rate of the core coder
 * \param aot             The current Audio Object Type
 * \return                Closest working bit rate to bitRate value
 */
UINT sbrEncoder_LimitBitRate(UINT bitRate, UINT numChannels,
                             UINT coreSampleRate, AUDIO_OBJECT_TYPE aot);

/**
 * \brief                Check whether downsampled SBR single rate is possible
 *                       with given audio object type.
 * \param aot            The Audio object type.
 * \return               0 when downsampled SBR is not possible,
 *                       1 when downsampled SBR is possible.
 */
UINT sbrEncoder_IsSingleRatePossible(AUDIO_OBJECT_TYPE aot);

/**
 * \brief                  Initialize SBR Encoder instance.
 * \param phSbrEncoder     Pointer to a SBR Encoder instance.
 * \param elInfo           Structure that describes the element/channel
 * arrangement.
 * \param noElements       Amount of elements described in elInfo.
 * \param inputBuffer      Pointer to the encoder audio buffer
 * \param inputBufferBufSize    Buffer offset of one channel (frameSize + delay)
 * \param bandwidth        Returns the core audio encoder bandwidth (output)
 * \param bufferOffset     Returns the offset for the audio input data in order
 * to do delay balancing.
 * \param numChannels      Input: Encoder input channels. output: core encoder
 * channels.
 * \param sampleRate       Input: Encoder samplerate. output core encoder
 * samplerate.
 * \param downSampleFactor Input: Relation between SBR and core coder sampling
 * rate;
 * \param frameLength      Input: Encoder frameLength. output core encoder
 * frameLength.
 * \param aot              Input: AOT..
 * \param delay            Input: core encoder delay. Output: total delay
 * because of SBR.
 * \param transformFactor  The core encoder transform factor (blockswitching).
 * \param headerPeriod     Repetition rate of the SBR header:
 *                           - (-1) means intern configuration.
 *                           - (1-10) corresponds to header repetition rate in
 * frames.
 * \return                 0 on success, and non-zero if failed.
 */
INT sbrEncoder_Init(HANDLE_SBR_ENCODER hSbrEncoder,
                    SBR_ELEMENT_INFO elInfo[(8)], int noElements,
                    INT_PCM *inputBuffer, UINT inputBufferBufSize,
                    INT *coreBandwidth, INT *inputBufferOffset,
                    INT *numChannels, const UINT syntaxFlags, INT *sampleRate,
                    UINT *downSampleFactor, INT *frameLength,
                    AUDIO_OBJECT_TYPE aot, int *delay, int transformFactor,
                    const int headerPeriod, ULONG statesInitFlag);

/**
 * \brief             Do delay line buffers housekeeping. To be called after
 * each encoded audio frame.
 * \param hEnvEnc     SBR Encoder handle.
 * \param timeBuffer  Pointer to the encoder audio buffer.
 * \param timeBufferBufSIze buffer size for one channel
 * \return            0 on success, and non-zero if failed.
 */
INT sbrEncoder_UpdateBuffers(HANDLE_SBR_ENCODER hEnvEnc, INT_PCM *timeBuffer,
                             UINT timeBufferBufSIze);

/**
 * \brief               Close SBR encoder instance.
 * \param phEbrEncoder  Handle of SBR encoder instance to be closed.
 * \return              void
 */
void sbrEncoder_Close(HANDLE_SBR_ENCODER *phEbrEncoder);

/**
 * \brief               Encode SBR data of one complete audio frame.
 * \param hEnvEncoder   Handle of SBR encoder instance.
 * \param samples       Time samples, not interleaved.
 * \param timeInStride  Channel offset of samples buffer.
 * \param sbrDataBits   Size of SBR payload in bits.
 * \param sbrData       SBR payload.
 * \return              0 on success, and non-zero if failed.
 */
INT sbrEncoder_EncodeFrame(HANDLE_SBR_ENCODER hEnvEncoder, INT_PCM *samples,
                           UINT samplesBufSize, UINT sbrDataBits[(8)],
                           UCHAR sbrData[(8)][MAX_PAYLOAD_SIZE]);

/**
 * \brief               Write SBR headers of one SBR element.
 * \param sbrEncoder    Handle of the SBR encoder instance.
 * \param hBs           Handle of bit stream handle to write SBR header to.
 * \param element_index Index of the SBR element which header should be written.
 * \param fSendHeaders  Flag indicating that the SBR encoder should send more
 * headers in the SBR payload or not.
 * \return              void
 */
void sbrEncoder_GetHeader(HANDLE_SBR_ENCODER sbrEncoder,
                          HANDLE_FDK_BITSTREAM hBs, INT element_index,
                          int fSendHeaders);

/**
 * \brief              Request to write SBR header.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             0 on success, and non-zero if failed.
 */
INT sbrEncoder_SendHeader(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              Request if last sbr payload contains an SBR header.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             1 contains sbr header, 0 without sbr header.
 */
INT sbrEncoder_ContainsHeader(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              SBR header delay in frames.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             Delay in frames, -1 on failure.
 */
INT sbrEncoder_GetHeaderDelay(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              Bitstrem delay in SBR frames.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             Delay in frames, -1 on failure.
 */
INT sbrEncoder_GetBsDelay(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              Prepare SBR payload for SAP.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             0 on success, and non-zero if failed.
 */
INT sbrEncoder_SAPPrepare(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              SBR encoder bitrate estimation.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             Estimated bitrate.
 */
INT sbrEncoder_GetEstimateBitrate(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              Delay between input data and downsampled output data.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             Delay.
 */
INT sbrEncoder_GetInputDataDelay(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief              Delay caused by the SBR decoder.
 * \param hSbrEncoder  SBR encoder handle.
 * \return             Delay.
 */
INT sbrEncoder_GetSbrDecDelay(HANDLE_SBR_ENCODER hSbrEncoder);

/**
 * \brief       Get decoder library version info.
 * \param info  Pointer to an allocated LIB_INFO struct, where library info is
 * written to.
 * \return      0 on sucess.
 */
INT sbrEncoder_GetLibInfo(LIB_INFO *info);

void sbrPrintRAM(void);

void sbrPrintROM(void);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __SBR_MAIN_H */
