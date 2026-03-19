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

/**************************** AAC encoder library ******************************

   Author(s):   M. Schug / A. Groeschel

   Description: fast aac coder interface library functions

*******************************************************************************/

#ifndef AACENC_H
#define AACENC_H

#include "common_fix.h"
#include "FDK_audio.h"

#include "tpenc_lib.h"

#include "sbr_encoder.h"

#define MIN_BUFSIZE_PER_EFF_CHAN 6144

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AAC-LC error codes.
 */
typedef enum {
  AAC_ENC_OK = 0x0000, /*!< All fine. */

  AAC_ENC_UNKNOWN = 0x0002, /*!< Error condition is of unknown reason, or from
                               another module.              */

  /* initialization errors */
  aac_enc_init_error_start = 0x2000,
  AAC_ENC_INVALID_HANDLE = 0x2020, /*!< The handle passed to the function call
                                      was invalid (probably NULL).        */
  AAC_ENC_INVALID_FRAME_LENGTH =
      0x2080, /*!< Invalid frame length (must be 1024 or 960). */
  AAC_ENC_INVALID_N_CHANNELS =
      0x20e0, /*!< Invalid amount of audio input channels. */
  AAC_ENC_INVALID_SFB_TABLE = 0x2140, /*!< Internal encoder error. */

  AAC_ENC_UNSUPPORTED_AOT =
      0x3000, /*!< The Audio Object Type (AOT) is not supported. */
  AAC_ENC_UNSUPPORTED_FILTERBANK =
      0x3010, /*!< Filterbank type is not supported. */
  AAC_ENC_UNSUPPORTED_BITRATE =
      0x3020, /*!< The chosen bitrate is not supported. */
  AAC_ENC_UNSUPPORTED_BITRATE_MODE =
      0x3028, /*!< Unsupported bit rate mode (CBR or VBR). */
  AAC_ENC_UNSUPPORTED_ANC_BITRATE =
      0x3040, /*!< Unsupported ancillay bitrate. */
  AAC_ENC_UNSUPPORTED_ANC_MODE = 0x3060,
  AAC_ENC_UNSUPPORTED_TRANSPORT_TYPE =
      0x3080, /*!< The bitstream format is not supported. */
  AAC_ENC_UNSUPPORTED_ER_FORMAT =
      0x30a0, /*!< The error resilience tool format is not supported. */
  AAC_ENC_UNSUPPORTED_EPCONFIG =
      0x30c0, /*!< The error protection format is not supported. */
  AAC_ENC_UNSUPPORTED_CHANNELCONFIG =
      0x30e0, /*!< The channel configuration (either number or arrangement) is
                 not supported. */
  AAC_ENC_UNSUPPORTED_SAMPLINGRATE =
      0x3100, /*!< Sample rate of audio input is not supported. */
  AAC_ENC_NO_MEMORY = 0x3120,               /*!< Could not allocate memory. */
  AAC_ENC_PE_INIT_TABLE_NOT_FOUND = 0x3140, /*!< Internal encoder error.    */

  aac_enc_init_error_end,

  /* encode errors */
  aac_enc_error_start = 0x4000,
  AAC_ENC_QUANT_ERROR = 0x4020, /*!< Too many bits used in quantization. */
  AAC_ENC_WRITTEN_BITS_ERROR =
      0x4040, /*!< Unexpected number of written bits, differs to
                   calculated number of bits.                      */
  AAC_ENC_PNS_TABLE_ERROR = 0x4060,      /*!< PNS level out of range.      */
  AAC_ENC_GLOBAL_GAIN_TOO_HIGH = 0x4080, /*!< Internal quantizer error. */
  AAC_ENC_BITRES_TOO_LOW = 0x40a0, /*!< Too few  bits in bit reservoir.       */
  AAC_ENC_BITRES_TOO_HIGH = 0x40a1, /*!< Too many bits in bit reservoir.      */
  AAC_ENC_INVALID_CHANNEL_BITRATE = 0x4100,
  AAC_ENC_INVALID_ELEMENTINFO_TYPE = 0x4120, /*!< Internal encoder error. */

  AAC_ENC_WRITE_SCAL_ERROR = 0x41e0, /*!< Error writing scalefacData. */
  AAC_ENC_WRITE_SEC_ERROR = 0x4200,  /*!< Error writing sectionData.  */
  AAC_ENC_WRITE_SPEC_ERROR = 0x4220, /*!< Error writing spectralData. */
  aac_enc_error_end

} AAC_ENCODER_ERROR;
/*-------------------------- defines --------------------------------------*/

#define ANC_DATA_BUFFERSIZE 1024 /* ancBuffer size */

#define MAX_TOTAL_EXT_PAYLOADS ((((8)) * (1)) + (2 + 2))

typedef enum {
  AACENC_BR_MODE_INVALID = -1, /*!< Invalid bitrate mode. */
  AACENC_BR_MODE_CBR = 0,      /*!< Constant bitrate mode.      */
  AACENC_BR_MODE_VBR_1 = 1, /*!< Variable bitrate mode, very low bitrate.    */
  AACENC_BR_MODE_VBR_2 = 2, /*!< Variable bitrate mode, low bitrate.    */
  AACENC_BR_MODE_VBR_3 = 3, /*!< Variable bitrate mode, medium bitrate.    */
  AACENC_BR_MODE_VBR_4 = 4, /*!< Variable bitrate mode, high bitrate.    */
  AACENC_BR_MODE_VBR_5 = 5, /*!< Variable bitrate mode, very high bitrate.    */
  AACENC_BR_MODE_FF = 6,    /*!< Fixed frame mode.       */
  AACENC_BR_MODE_SFR = 7    /*!< Superframe mode.       */

} AACENC_BITRATE_MODE;

#define AACENC_BR_MODE_IS_VBR(brMode) ((brMode >= 1) && (brMode <= 5))

typedef enum {

  CH_ORDER_MPEG =
      0, /*!< MPEG channel ordering (e. g. 5.1: C, L, R, SL, SR, LFE)       */
  CH_ORDER_WAV, /*!< WAV fileformat channel ordering (e. g. 5.1: L, R, C, LFE,
                   SL, SR) */
  CH_ORDER_WG4  /*!< WG4 fileformat channel ordering (e. g. 5.1: L, R, SL, SR, C, LFE) */

} CHANNEL_ORDER;

/*-------------------- structure definitions ------------------------------*/

struct AACENC_CONFIG {
  INT sampleRate;     /* encoder sample rate */
  INT bitRate;        /* encoder bit rate in bits/sec */
  INT ancDataBitRate; /* additional bits consumed by anc data or sbr have to be
                         consiedered while configuration */

  INT nSubFrames; /* number of frames in super frame (not ADTS/LATM subframes !)
                   */
  AUDIO_OBJECT_TYPE audioObjectType; /* Audio Object Type  */

  INT averageBits;                 /* encoder bit rate in bits/superframe */
  AACENC_BITRATE_MODE bitrateMode; /* encoder bitrate mode (CBR/VBR) */
  INT nChannels;                   /* number of channels to process */
  CHANNEL_ORDER channelOrder;      /* input Channel ordering scheme. */
  INT bandWidth;                   /* targeted audio bandwidth in Hz */
  CHANNEL_MODE channelMode;        /* encoder channel mode configuration */
  INT framelength;                 /* used frame size */

  UINT syntaxFlags; /* bitstreams syntax configuration */
  SCHAR epConfig;   /* error protection configuration */

  INT anc_Rate; /* ancillary rate, 0 (disabled), -1 (default) else desired rate
                 */
  UINT maxAncBytesPerAU;
  INT minBitsPerFrame; /* minimum number of bits in AU */
  INT maxBitsPerFrame; /* maximum number of bits in AU */

  INT audioMuxVersion; /* audio mux version in loas/latm transport format */

  UINT sbrRatio; /* sbr sampling rate ratio: dual- or single-rate */

  UCHAR useTns; /* flag: use temporal noise shaping */
  UCHAR usePns; /* flag: use perceptual noise substitution */
  UCHAR useIS;  /* flag: use intensity coding */
  UCHAR useMS;  /* flag: use ms stereo tool */

  UCHAR useRequant; /* flag: use afterburner */

  UINT downscaleFactor;
};

typedef struct {
  UCHAR *pData;              /* pointer to extension payload data */
  UINT dataSize;             /* extension payload data size in bits */
  EXT_PAYLOAD_TYPE dataType; /* extension payload data type */
  INT associatedChElement; /* number of the channel element the data is assigned
                              to */
} AACENC_EXT_PAYLOAD;

typedef struct AAC_ENC *HANDLE_AAC_ENC;

/**
 * \brief Calculate framesize in bits for given bit rate, frame length and
 * sampling rate.
 *
 * \param bitRate               Ttarget bitrate in bits per second.
 * \param frameLength           Number of audio samples in one frame.
 * \param samplingRate          Sampling rate in Hz.
 *
 * \return                      Framesize in bits per frame.
 */
INT FDKaacEnc_CalcBitsPerFrame(const INT bitRate, const INT frameLength,
                               const INT samplingRate);

/**
 * \brief Calculate bitrate in bits per second for given framesize, frame length
 * and sampling rate.
 *
 * \param bitsPerFrame          Framesize in bits per frame
 * \param frameLength           Number of audio samples in one frame.
 * \param samplingRate          Sampling rate in Hz.
 *
 * \return                      Bitrate in bits per second.
 */
INT FDKaacEnc_CalcBitrate(const INT bitsPerFrame, const INT frameLength,
                          const INT samplingRate);

/**
 * \brief Limit given bit rate to a valid value
 * \param hTpEnc transport encoder handle
 * \param aot audio object type
 * \param coreSamplingRate the sample rate to be used for the AAC encoder
 * \param frameLength the frameLength to be used for the AAC encoder
 * \param nChannels number of total channels
 * \param nChannelsEff number of effective channels
 * \param bitRate the initial bit rate value for which the closest valid bit
 * rate value is searched for
 * \param averageBits average bits per frame for fixed framing. Set to -1 if not
 * available.
 * \param optional pointer where the current bits per frame are stored into.
 * \param bitrateMode the current bit rate mode
 * \param nSubFrames number of sub frames for super framing (not transport
 * frames).
 * \return a valid bit rate value as close as possible or identical to bitRate
 */
INT FDKaacEnc_LimitBitrate(HANDLE_TRANSPORTENC hTpEnc, AUDIO_OBJECT_TYPE aot,
                           INT coreSamplingRate, INT frameLength, INT nChannels,
                           INT nChannelsEff, INT bitRate, INT averageBits,
                           INT *pAverageBitsPerFrame,
                           AACENC_BITRATE_MODE bitrateMode, INT nSubFrames);

/**
 * \brief Get current state of the bit reservoir
 * \param hAacEncoder encoder handle
 * \return bit reservoir state in bits
 */
INT FDKaacEnc_GetBitReservoirState(const HANDLE_AAC_ENC hAacEncoder);

/*-----------------------------------------------------------------------------

    functionname: FDKaacEnc_GetVBRBitrate
    description:  Get VBR bitrate from vbr quality
    input params: int vbrQuality (VBR0, VBR1, VBR2)
                  channelMode
    returns:      vbr bitrate

------------------------------------------------------------------------------*/
INT FDKaacEnc_GetVBRBitrate(AACENC_BITRATE_MODE bitrateMode,
                            CHANNEL_MODE channelMode);

/*-----------------------------------------------------------------------------

    functionname: FDKaacEnc_AdjustVBRBitrateMode
    description:  Adjust bitrate mode to given bitrate parameter
    input params: int vbrQuality (VBR0, VBR1, VBR2)
                  bitrate
                  channelMode
    returns:      vbr bitrate mode

 ------------------------------------------------------------------------------*/
AACENC_BITRATE_MODE FDKaacEnc_AdjustVBRBitrateMode(
    AACENC_BITRATE_MODE bitrateMode, INT bitrate, CHANNEL_MODE channelMode);

/*-----------------------------------------------------------------------------

     functionname: FDKaacEnc_AacInitDefaultConfig
     description:  gives reasonable default configuration
     returns:      ---

 ------------------------------------------------------------------------------*/
void FDKaacEnc_AacInitDefaultConfig(AACENC_CONFIG *config);

/*---------------------------------------------------------------------------

    functionname:FDKaacEnc_Open
    description: allocate and initialize a new encoder instance
    returns:     0 if success

  ---------------------------------------------------------------------------*/
AAC_ENCODER_ERROR FDKaacEnc_Open(
    HANDLE_AAC_ENC
        *phAacEnc, /* pointer to an encoder handle, initialized on return */
    const INT nElements, /* number of maximal elements in instance to support */
    const INT nChannels, /* number of maximal channels in instance to support */
    const INT nSubFrames); /* support superframing in instance */

AAC_ENCODER_ERROR FDKaacEnc_Initialize(
    HANDLE_AAC_ENC
        hAacEncoder, /* pointer to an encoder handle, initialized on return */
    AACENC_CONFIG *config, /* pre-initialized config struct */
    HANDLE_TRANSPORTENC hTpEnc, ULONG initFlags);

/*---------------------------------------------------------------------------

    functionname: FDKaacEnc_EncodeFrame
    description:  encode one frame
    returns:      0 if success

  ---------------------------------------------------------------------------*/

AAC_ENCODER_ERROR FDKaacEnc_EncodeFrame(
    HANDLE_AAC_ENC hAacEnc, /* encoder handle */
    HANDLE_TRANSPORTENC hTpEnc, INT_PCM *inputBuffer,
    const UINT inputBufferBufSize, INT *numOutBytes,
    AACENC_EXT_PAYLOAD extPayload[MAX_TOTAL_EXT_PAYLOADS]);

/*---------------------------------------------------------------------------

    functionname:FDKaacEnc_Close
    description: delete encoder instance
    returns:

  ---------------------------------------------------------------------------*/

void FDKaacEnc_Close(HANDLE_AAC_ENC *phAacEnc); /* encoder handle */

#ifdef __cplusplus
}
#endif

#endif /* AACENC_H */
