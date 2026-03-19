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

/**************************** AAC decoder library ******************************

   Author(s):   Manuel Jander

   Description:

*******************************************************************************/

#ifndef AACDECODER_LIB_H
#define AACDECODER_LIB_H

/**
 * \file   aacdecoder_lib.h
 * \brief  FDK AAC decoder library interface header file.
 *

\page INTRO Introduction


\section SCOPE Scope

This document describes the high-level application interface and usage of the
ISO/MPEG-2/4 AAC Decoder library developed by the Fraunhofer Institute for
Integrated Circuits (IIS). Depending on the library configuration, decoding of
AAC-LC (Low-Complexity), HE-AAC (High-Efficiency AAC v1 and v2), AAC-LD
(Low-Delay) and AAC-ELD (Enhanced Low-Delay) is implemented.

All references to SBR (Spectral Band Replication) are only applicable to HE-AAC
and AAC-ELD configurations of the FDK library. All references to PS (Parametric
Stereo) are only applicable to HE-AAC v2 decoder configuration of the library.

\section DecoderBasics Decoder Basics

This document can only give a rough overview about the ISO/MPEG-2, ISO/MPEG-4
AAC audio and MPEG-D USAC coding standards. To understand all details referenced
in this document, you are encouraged to read the following documents.

- ISO/IEC 13818-7 (MPEG-2 AAC) Standard, defines the syntax of MPEG-2 AAC audio
bitstreams.
- ISO/IEC 14496-3 (MPEG-4 AAC, subpart 1 and 4) Standard, defines the syntax of
MPEG-4 AAC audio bitstreams.
- ISO/IEC 23003-3 (MPEG-D USAC), defines MPEG-D USAC unified speech and audio
codec.
- Lutzky, Schuller, Gayer, Kr&auml;mer, Wabnik, "A guideline to audio codec
delay", 116th AES Convention, May 8, 2004

In short, MPEG Advanced Audio Coding is based on a time-to-frequency mapping of
the signal. The signal is partitioned into overlapping time portions and
transformed into frequency domain. The spectral components are then quantized
and coded using a highly efficient coding scheme.\n Encoded MPEG-2 and MPEG-4
AAC audio bitstreams are composed of frames. Contrary to MPEG-1/2 Layer-3 (mp3),
the length of individual frames is not restricted to a fixed number of bytes,
but can take any length between 1 and 768 bytes.

In addition to the above mentioned frequency domain coding mode, MPEG-D USAC
also employs a time domain Algebraic Code-Excited Linear Prediction (ACELP)
speech coder core. This operating mode is selected by the encoder in order to
achieve the optimum audio quality for different content type. Several
enhancements allow achieving higher quality at lower bit rates compared to
MPEG-4 HE-AAC.


\page LIBUSE Library Usage


\section InterfaceDescritpion API Description

All API header files are located in the folder /include of the release package.
The contents of each file is described in detail in this document. All header
files are provided for usage in specific C/C++ programs. The main AAC decoder
library API functions are located in aacdecoder_lib.h header file.


\section Calling_Sequence Calling Sequence

The following sequence is necessary for proper decoding of ISO/MPEG-2/4 AAC,
HE-AAC v2, or MPEG-D USAC bitstreams. In the following description, input stream
read and output write function details are left out, since they may be
implemented in a variety of configurations depending on the user's specific
requirements.


-# Call aacDecoder_Open() to open and retrieve a handle to a new AAC decoder
instance. \code aacDecoderInfo = aacDecoder_Open(transportType, nrOfLayers);
\endcode
-# If out-of-band config data (Audio Specific Config (ASC) or Stream Mux Config
(SMC)) is available, call aacDecoder_ConfigRaw() to pass this data to the
decoder before beginning the decoding process. If this data is not available in
advance, the decoder will configure itself while decoding, during the
aacDecoder_DecodeFrame() function call.
-# Begin decoding loop.
\code
do {
\endcode
-# Read data from bitstream file or stream buffer in to the driver program
working memory (a client-supplied input buffer "inBuffer" in framework). This
buffer will be used to load AAC bitstream data to the decoder.  Only when all
data in this buffer has been processed will the decoder signal an empty buffer.
-# Call aacDecoder_Fill() to fill the decoder's internal bitstream input buffer
with the client-supplied bitstream input buffer. Note, if the data loaded in to
the internal buffer is not sufficient to decode a frame,
aacDecoder_DecodeFrame() will return ::AAC_DEC_NOT_ENOUGH_BITS until a
sufficient amount of data is loaded in to the internal buffer. For streaming
formats (ADTS, LOAS), it is acceptable to load more than one frame to the
decoder. However, for packed based formats, only one frame may be loaded to the
decoder per aacDecoder_DecodeFrame() call. For least amount of communication
delay, fill and decode should be performed on a frame by frame basis. \code
    ErrorStatus = aacDecoder_Fill(aacDecoderInfo, inBuffer, bytesRead,
bytesValid); \endcode
-# Call aacDecoder_DecodeFrame(). This function decodes one frame and writes
decoded PCM audio data to a client-supplied buffer. It is the client's
responsibility to allocate a buffer which is large enough to hold the decoded
output data. \code ErrorStatus = aacDecoder_DecodeFrame(aacDecoderInfo,
TimeData, OUT_BUF_SIZE, flags); \endcode If the bitstream configuration (number
of channels, sample rate, frame size) is not known a priori, you may call
aacDecoder_GetStreamInfo() to retrieve a structure that contains this
information. You may use this data to initialize an audio output device. \code
    p_si = aacDecoder_GetStreamInfo(aacDecoderInfo);
\endcode
-# Repeat steps 5 to 7 until no data is available to decode any more, or in case
of error. \code } while (bytesRead[0] > 0 || doFlush || doBsFlush ||
forceContinue); \endcode
-# Call aacDecoder_Close() to de-allocate all AAC decoder and transport layer
structures. \code aacDecoder_Close(aacDecoderInfo); \endcode

\image latex decode.png "Decode calling sequence" width=11cm

\image latex change_source.png "Change data source sequence" width=5cm

\image latex conceal.png "Error concealment sequence" width=14cm

\subsection Error_Concealment_Sequence Error Concealment Sequence

There are different strategies to handle bit stream errors. Depending on the
system properties the product designer might choose to take different actions in
case a bit error occurs. In many cases the decoder might be able to do
reasonable error concealment without the need of any additional actions from the
system. But in some cases its not even possible to know how many decoded PCM
output samples are required to fill the gap due to the data error, then the
software surrounding the decoder must deal with the situation. The most simple
way would be to just stop audio playback and resume once enough bit stream data
and/or buffered output samples are available. More sophisticated designs might
also be able to deal with sender/receiver clock drifts or data drop outs by
using a closed loop control of FIFO fulness levels. The chosen strategy depends
on the final product requirements.

The error concealment sequence diagram illustrates the general execution paths
for error handling.

The macro IS_OUTPUT_VALID(err) can be used to identify if the audio output
buffer contains valid audio either from error free bit stream data or successful
error concealment. In case the result is false, the decoder output buffer does
not contain meaningful audio samples and should not be passed to any output as
it is. Most likely in case that a continuous audio output PCM stream is
required, the output buffer must be filled with audio data from the calling
framework. This might be e.g. an appropriate number of samples all zero.

If error code ::AAC_DEC_TRANSPORT_SYNC_ERROR is returned by the decoder, under
some particular conditions it is possible to estimate lost frames due to the bit
stream error. In that case the bit stream is required to have a constant
bitrate, and compatible transport type. Audio samples for the lost frames can be
obtained by calling aacDecoder_DecodeFrame() with flag ::AACDEC_CONCEAL set
n-times where n is the count of lost frames. Please note that the decoder has to
have encountered valid configuration data at least once to be able to generate
concealed data, because at the minimum the sampling rate, frame size and amount
of audio channels needs to be known.

If it is not possible to get an estimation of lost frames then a constant
fullness of the audio output buffer can be achieved by implementing different
FIFO control techniques e.g. just stop taking of samples from the buffer to
avoid underflow or stop filling new data to the buffer to avoid overflow. But
this techniques are out of scope of this document.

For a detailed description of a specific error code please refer also to
::AAC_DECODER_ERROR.

\section BufferSystem Buffer System

There are three main buffers in an AAC decoder application. One external input
buffer to hold bitstream data from file I/O or elsewhere, one decoder-internal
input buffer, and one to hold the decoded output PCM sample data. In resource
limited applications, the output buffer may be reused as an external input
buffer prior to the subsequence aacDecoder_Fill() function call.

To feed the data to the decoder-internal input buffer, use the
function aacDecoder_Fill(). This function returns important information
regarding the number of bytes in the external input buffer that have not yet
been copied into the internal input buffer (variable bytesValid). Once the
external buffer has been fully copied, it can be completely re-filled again. In
case you wish to refill the buffer while there are unprocessed bytes (bytesValid
is unequal 0), you should preserve the unconsumed data. However, we recommend to
refill the buffer only when bytesValid returns 0.

The bytesValid parameter is an input and output parameter to the FDK decoder. As
an input, it signals how many valid bytes are available in the external buffer.
After consumption of the external buffer using aacDecoder_Fill() function, the
bytesValid parameter indicates if any of the bytes in the external buffer were
not consumed.

\image latex dec_buffer.png "Life cycle of the external input buffer" width=9cm

\page OutputFormat Decoder audio output

\section OutputFormatObtaining Obtaining channel mapping information

The decoded audio output format is indicated by a set of variables of the
CStreamInfo structure. While the struct members sampleRate, frameSize and
numChannels might be self explanatory, pChannelType and pChannelIndices require
some further explanation.

These two arrays indicate the configuration of channel data within the output
buffer. Both arrays have CStreamInfo::numChannels number of cells. Each cell of
pChannelType indicates the channel type, which is described in the enum
::AUDIO_CHANNEL_TYPE (defined in FDK_audio.h). The cells of pChannelIndices
indicate the sub index among the channels starting with 0 among channels of the
same audio channel type.

The indexing scheme is structured as defined in MPEG-2/4 Standards. Indices
start from the front direction (a center channel if available, will always be
index 0) and increment, starting with the left side, pairwise (e.g. L, R) and
from front to back (Front L, Front R, Surround L, Surround R). For detailed
explanation, please refer to ISO/IEC 13818-7:2005(E), chapter 8.5.3.2.

In case a Program Config is included in the audio configuration, the channel
mapping described within it will be adopted.

The examples below explain these aspects in detail.

\section OutputFormatChange Changing the audio output format

For MPEG-4 audio the channel order can be changed at runtime through the
parameter
::AAC_PCM_OUTPUT_CHANNEL_MAPPING. See the description of those
parameters and the decoder library function aacDecoder_SetParam() for more
detail.

\section OutputFormatExample Channel mapping examples

The following examples illustrate the location of individual audio samples in
the audio buffer that is passed to aacDecoder_DecodeFrame() and the expected
data in the CStreamInfo structure which can be obtained by calling
aacDecoder_GetStreamInfo().

\subsection ExamplesStereo Stereo

In case of ::AAC_PCM_OUTPUT_CHANNEL_MAPPING set to 1,
a AAC-LC bit stream which has channelConfiguration = 2 in its audio specific
config would lead to the following values in CStreamInfo:

CStreamInfo::numChannels = 2

CStreamInfo::pChannelType = { ::ACT_FRONT, ::ACT_FRONT }

CStreamInfo::pChannelIndices = { 0, 1 }

The output buffer will be formatted as follows:

\verbatim
  <left sample 0>  <left sample 1>  <left sample 2>  ... <left sample N>
  <right sample 0> <right sample 1> <right sample 2> ... <right sample N>
\endverbatim

Where N equals to CStreamInfo::frameSize .

\subsection ExamplesSurround Surround 5.1

In case of ::AAC_PCM_OUTPUT_CHANNEL_MAPPING set to 1,
a AAC-LC bit stream which has channelConfiguration = 6 in its audio specific
config, would lead to the following values in CStreamInfo:

CStreamInfo::numChannels = 6

CStreamInfo::pChannelType = { ::ACT_FRONT, ::ACT_FRONT, ::ACT_FRONT, ::ACT_LFE,
::ACT_BACK, ::ACT_BACK }

CStreamInfo::pChannelIndices = { 1, 2, 0, 0, 0, 1 }

Since ::AAC_PCM_OUTPUT_CHANNEL_MAPPING is 1, WAV file channel ordering will be
used. For a 5.1 channel scheme, thus the channels would be: front left, front
right, center, LFE, surround left, surround right. Thus the third channel is the
center channel, receiving the index 0. The other front channels are front left,
front right being placed as first and second channels with indices 1 and 2
correspondingly. There is only one LFE, placed as the fourth channel and index
0. Finally both surround channels get the type definition ACT_BACK, and the
indices 0 and 1.

The output buffer will be formatted as follows:

\verbatim
<front left sample 0> <front right sample 0>
<center sample 0> <LFE sample 0>
<surround left sample 0> <surround right sample 0>

<front left sample 1> <front right sample 1>
<center sample 1> <LFE sample 1>
<surround left sample 1> <surround right sample 1>

...

<front left sample N> <front right sample N>
<center sample N> <LFE sample N>
<surround left sample N> <surround right sample N>
\endverbatim

Where N equals to CStreamInfo::frameSize .

\subsection ExamplesArib ARIB coding mode 2/1

In case of ::AAC_PCM_OUTPUT_CHANNEL_MAPPING set to 1,
in case of a ARIB bit stream using coding mode 2/1 as described in ARIB STD-B32
Part 2 Version 2.1-E1, page 61, would lead to the following values in
CStreamInfo:

CStreamInfo::numChannels = 3

CStreamInfo::pChannelType = { ::ACT_FRONT, ::ACT_FRONT, ::ACT_BACK }

CStreamInfo::pChannelIndices = { 0, 1, 0 }

The audio channels will be placed as follows in the audio output buffer:

\verbatim
<front left sample 0> <front right sample 0>  <mid surround sample 0>

<front left sample 1> <front right sample 1> <mid surround sample 1>

...

<front left sample N> <front right sample N> <mid surround sample N>

Where N equals to CStreamInfo::frameSize .

\endverbatim

*/

#include "machine_type.h"
#include "FDK_audio.h"

#define AACDECODER_LIB_VL0 3
#define AACDECODER_LIB_VL1 2
#define AACDECODER_LIB_VL2 0

#include "genericStds.h"
/**
 * \brief  AAC decoder error codes.
 */
typedef enum {
  AAC_DEC_OK =
      0x0000, /*!< No error occurred. Output buffer is valid and error free. */
  AAC_DEC_OUT_OF_MEMORY =
      0x0002, /*!< Heap returned NULL pointer. Output buffer is invalid. */
  AAC_DEC_UNKNOWN =
      0x0005, /*!< Error condition is of unknown reason, or from a another
                 module. Output buffer is invalid. */

  /* Synchronization errors. Output buffer is invalid. */
  aac_dec_sync_error_start = 0x1000,
  AAC_DEC_TRANSPORT_SYNC_ERROR = 0x1001, /*!< The transport decoder had
                                            synchronization problems. Do not
                                            exit decoding. Just feed new
                                              bitstream data. */
  AAC_DEC_NOT_ENOUGH_BITS = 0x1002, /*!< The input buffer ran out of bits. */
  aac_dec_sync_error_end = 0x1FFF,

  /* Initialization errors. Output buffer is invalid. */
  aac_dec_init_error_start = 0x2000,
  AAC_DEC_INVALID_HANDLE =
      0x2001, /*!< The handle passed to the function call was invalid (NULL). */
  AAC_DEC_UNSUPPORTED_AOT =
      0x2002, /*!< The AOT found in the configuration is not supported. */
  AAC_DEC_UNSUPPORTED_FORMAT =
      0x2003, /*!< The bitstream format is not supported.  */
  AAC_DEC_UNSUPPORTED_ER_FORMAT =
      0x2004, /*!< The error resilience tool format is not supported. */
  AAC_DEC_UNSUPPORTED_EPCONFIG =
      0x2005, /*!< The error protection format is not supported. */
  AAC_DEC_UNSUPPORTED_MULTILAYER =
      0x2006, /*!< More than one layer for AAC scalable is not supported. */
  AAC_DEC_UNSUPPORTED_CHANNELCONFIG =
      0x2007, /*!< The channel configuration (either number or arrangement) is
                 not supported. */
  AAC_DEC_UNSUPPORTED_SAMPLINGRATE = 0x2008, /*!< The sample rate specified in
                                                the configuration is not
                                                supported. */
  AAC_DEC_INVALID_SBR_CONFIG =
      0x2009, /*!< The SBR configuration is not supported. */
  AAC_DEC_SET_PARAM_FAIL = 0x200A,  /*!< The parameter could not be set. Either
                                       the value was out of range or the
                                       parameter does  not exist. */
  AAC_DEC_NEED_TO_RESTART = 0x200B, /*!< The decoder needs to be restarted,
                                       since the required configuration change
                                       cannot be performed. */
  AAC_DEC_OUTPUT_BUFFER_TOO_SMALL =
      0x200C, /*!< The provided output buffer is too small. */
  aac_dec_init_error_end = 0x2FFF,

  /* Decode errors. Output buffer is valid but concealed. */
  aac_dec_decode_error_start = 0x4000,
  AAC_DEC_TRANSPORT_ERROR =
      0x4001, /*!< The transport decoder encountered an unexpected error. */
  AAC_DEC_PARSE_ERROR = 0x4002, /*!< Error while parsing the bitstream. Most
                                   probably it is corrupted, or the system
                                   crashed. */
  AAC_DEC_UNSUPPORTED_EXTENSION_PAYLOAD =
      0x4003, /*!< Error while parsing the extension payload of the bitstream.
                 The extension payload type found is not supported. */
  AAC_DEC_DECODE_FRAME_ERROR = 0x4004, /*!< The parsed bitstream value is out of
                                          range. Most probably the bitstream is
                                          corrupt, or the system crashed. */
  AAC_DEC_CRC_ERROR = 0x4005,          /*!< The embedded CRC did not match. */
  AAC_DEC_INVALID_CODE_BOOK = 0x4006,  /*!< An invalid codebook was signaled.
                                          Most probably the bitstream is corrupt,
                                          or the system  crashed. */
  AAC_DEC_UNSUPPORTED_PREDICTION =
      0x4007, /*!< Predictor found, but not supported in the AAC Low Complexity
                 profile. Most probably the bitstream is corrupt, or has a wrong
                 format. */
  AAC_DEC_UNSUPPORTED_CCE = 0x4008, /*!< A CCE element was found which is not
                                       supported. Most probably the bitstream is
                                       corrupt, or has a wrong format. */
  AAC_DEC_UNSUPPORTED_LFE = 0x4009, /*!< A LFE element was found which is not
                                       supported. Most probably the bitstream is
                                       corrupt, or has a wrong format. */
  AAC_DEC_UNSUPPORTED_GAIN_CONTROL_DATA =
      0x400A, /*!< Gain control data found but not supported. Most probably the
                 bitstream is corrupt, or has a wrong format. */
  AAC_DEC_UNSUPPORTED_SBA =
      0x400B, /*!< SBA found, but currently not supported in the BSAC profile.
               */
  AAC_DEC_TNS_READ_ERROR = 0x400C, /*!< Error while reading TNS data. Most
                                      probably the bitstream is corrupt or the
                                      system crashed. */
  AAC_DEC_RVLC_ERROR =
      0x400D, /*!< Error while decoding error resilient data. */
  aac_dec_decode_error_end = 0x4FFF,
  /* Ancillary data errors. Output buffer is valid. */
  aac_dec_anc_data_error_start = 0x8000,
  AAC_DEC_ANC_DATA_ERROR =
      0x8001, /*!< Non severe error concerning the ancillary data handling. */
  AAC_DEC_TOO_SMALL_ANC_BUFFER = 0x8002,  /*!< The registered ancillary data
                                             buffer is too small to receive the
                                             parsed data. */
  AAC_DEC_TOO_MANY_ANC_ELEMENTS = 0x8003, /*!< More than the allowed number of
                                             ancillary data elements should be
                                             written to buffer. */
  aac_dec_anc_data_error_end = 0x8FFF

} AAC_DECODER_ERROR;

/** Macro to identify initialization errors. Output buffer is invalid. */
#define IS_INIT_ERROR(err)                                                    \
  ((((err) >= aac_dec_init_error_start) && ((err) <= aac_dec_init_error_end)) \
       ? 1                                                                    \
       : 0)
/** Macro to identify decode errors. Output buffer is valid but concealed. */
#define IS_DECODE_ERROR(err)                 \
  ((((err) >= aac_dec_decode_error_start) && \
    ((err) <= aac_dec_decode_error_end))     \
       ? 1                                   \
       : 0)
/**
 * Macro to identify if the audio output buffer contains valid samples after
 * calling aacDecoder_DecodeFrame(). Output buffer is valid but can be
 * concealed.
 */
#define IS_OUTPUT_VALID(err) (((err) == AAC_DEC_OK) || IS_DECODE_ERROR(err))

/*! \enum  AAC_MD_PROFILE
 *  \brief The available metadata profiles which are mostly related to downmixing. The values define the arguments
 *         for the use with parameter ::AAC_METADATA_PROFILE.
 */
typedef enum {
  AAC_MD_PROFILE_MPEG_STANDARD =
      0, /*!< The standard profile creates a mixdown signal based on the
            advanced downmix metadata (from a DSE). The equations and default
            values are defined in ISO/IEC 14496:3 Ammendment 4. Any other
            (legacy) downmix metadata will be ignored. No other parameter will
            be modified.         */
  AAC_MD_PROFILE_MPEG_LEGACY =
      1, /*!< This profile behaves identical to the standard profile if advanced
              downmix metadata (from a DSE) is available. If not, the
            matrix_mixdown information embedded in the program configuration
            element (PCE) will be applied. If neither is the case, the module
            creates a mixdown using the default coefficients as defined in
            ISO/IEC 14496:3 AMD 4. The profile can be used to support legacy
            digital TV (e.g. DVB) streams.           */
  AAC_MD_PROFILE_MPEG_LEGACY_PRIO =
      2, /*!< Similar to the ::AAC_MD_PROFILE_MPEG_LEGACY profile but if both
            the advanced (ISO/IEC 14496:3 AMD 4) and the legacy (PCE) MPEG
            downmix metadata are available the latter will be applied.
          */
  AAC_MD_PROFILE_ARIB_JAPAN =
      3 /*!< Downmix creation as described in ABNT NBR 15602-2. But if advanced
             downmix metadata (ISO/IEC 14496:3 AMD 4) is available it will be
             preferred because of the higher resolutions. In addition the
           metadata expiry time will be set to the value defined in the ARIB
           standard (see ::AAC_METADATA_EXPIRY_TIME).
         */
} AAC_MD_PROFILE;

/*! \enum  AAC_DRC_DEFAULT_PRESENTATION_MODE_OPTIONS
 *  \brief Options for handling of DRC parameters, if presentation mode is not indicated in bitstream
 */
typedef enum {
  AAC_DRC_PARAMETER_HANDLING_DISABLED = -1, /*!< DRC parameter handling
                                               disabled, all parameters are
                                               applied as requested. */
  AAC_DRC_PARAMETER_HANDLING_ENABLED =
      0, /*!< Apply changes to requested DRC parameters to prevent clipping. */
  AAC_DRC_PRESENTATION_MODE_1_DEFAULT =
      1, /*!< Use DRC presentation mode 1 as default (e.g. for Nordig) */
  AAC_DRC_PRESENTATION_MODE_2_DEFAULT =
      2 /*!< Use DRC presentation mode 2 as default (e.g. for DTG DBook) */
} AAC_DRC_DEFAULT_PRESENTATION_MODE_OPTIONS;

/**
 * \brief AAC decoder setting parameters
 */
typedef enum {
  AAC_PCM_DUAL_CHANNEL_OUTPUT_MODE =
      0x0002, /*!< Defines how the decoder processes two channel signals: \n
                   0: Leave both signals as they are (default). \n
                   1: Create a dual mono output signal from channel 1. \n
                   2: Create a dual mono output signal from channel 2. \n
                   3: Create a dual mono output signal by mixing both channels
                 (L' = R' = 0.5*Ch1 + 0.5*Ch2). */
  AAC_PCM_OUTPUT_CHANNEL_MAPPING =
      0x0003, /*!< Output buffer channel ordering. 0: MPEG PCE style order, 1:
                 WAV file channel order (default). */
  AAC_PCM_LIMITER_ENABLE =
      0x0004,                           /*!< Enable signal level limiting. \n
                                             -1: Auto-config. Enable limiter for all
                                           non-lowdelay configurations by default. \n
                                              0: Disable limiter in general. \n
                                              1: Enable limiter always.
                                             It is recommended to call the decoder
                                           with a AACDEC_CLRHIST flag to reset all
                                           states when      the limiter switch is changed
                                           explicitly. */
  AAC_PCM_LIMITER_ATTACK_TIME = 0x0005, /*!< Signal level limiting attack time
                                           in ms. Default configuration is 15
                                           ms. Adjustable range from 1 ms to 15
                                           ms. */
  AAC_PCM_LIMITER_RELEAS_TIME = 0x0006, /*!< Signal level limiting release time
                                           in ms. Default configuration is 50
                                           ms. Adjustable time must be larger
                                           than 0 ms. */
  AAC_PCM_MIN_OUTPUT_CHANNELS =
      0x0011, /*!< Minimum number of PCM output channels. If higher than the
                 number of encoded audio channels, a simple channel extension is
                 applied (see note 4 for exceptions). \n -1, 0: Disable channel
                 extension feature. The decoder output contains the same number
                 of channels as the encoded bitstream. \n 1:    This value is
                 currently needed only together with the mix-down feature. See
                          ::AAC_PCM_MAX_OUTPUT_CHANNELS and note 2 below. \n
                    2:    Encoded mono signals will be duplicated to achieve a
                 2/0/0.0 channel output configuration. \n 6:    The decoder
                 tries to reorder encoded signals with less than six channels to
                 achieve a 3/0/2.1 channel output signal. Missing channels will
                 be filled with a zero signal. If reordering is not possible the
                 empty channels will simply be appended. Only available if
                 instance is configured to support multichannel output. \n 8:
                 The decoder tries to reorder encoded signals with less than
                 eight channels to achieve a 3/0/4.1 channel output signal.
                 Missing channels will be filled with a zero signal. If
                 reordering is not possible the empty channels will simply be
                          appended. Only available if instance is configured to
                 support multichannel output.\n NOTE: \n
                     1. The channel signaling (CStreamInfo::pChannelType and
                 CStreamInfo::pChannelIndices) will not be modified. Added empty
                 channels will be signaled with channel type
                        AUDIO_CHANNEL_TYPE::ACT_NONE. \n
                     2. If the parameter value is greater than that of
                 ::AAC_PCM_MAX_OUTPUT_CHANNELS both will be set to the same
                 value. \n
                     3. This parameter will be ignored if the number of encoded
                 audio channels is greater than 8. */
  AAC_PCM_MAX_OUTPUT_CHANNELS =
      0x0012, /*!< Maximum number of PCM output channels. If lower than the
                 number of encoded audio channels, downmixing is applied
                 accordingly (see note 5 for exceptions). If dedicated metadata
                 is available in the stream it will be used to achieve better
                 mixing results. \n -1, 0: Disable downmixing feature. The
                 decoder output contains the same number of channels as the
                 encoded bitstream. \n 1:    All encoded audio configurations
                 with more than one channel will be mixed down to one mono
                 output signal. \n 2:    The decoder performs a stereo mix-down
                 if the number encoded audio channels is greater than two. \n 6:
                 If the number of encoded audio channels is greater than six the
                 decoder performs a mix-down to meet the target output
                 configuration of 3/0/2.1 channels. Only available if instance
                 is configured to support multichannel output. \n 8:    This
                 value is currently needed only together with the channel
                 extension feature. See ::AAC_PCM_MIN_OUTPUT_CHANNELS and note 2
                 below. Only available if instance is configured to support
                 multichannel output. \n NOTE: \n
                     1. Down-mixing of any seven or eight channel configuration
                 not defined in ISO/IEC 14496-3 PDAM 4 is not supported by this
                 software version. \n
                     2. If the parameter value is greater than zero but smaller
                 than ::AAC_PCM_MIN_OUTPUT_CHANNELS both will be set to same
                 value. \n
                     3. This parameter will be ignored if the number of encoded
                 audio channels is greater than 8. */
  AAC_METADATA_PROFILE =
      0x0020, /*!< See ::AAC_MD_PROFILE for all available values. */
  AAC_METADATA_EXPIRY_TIME = 0x0021, /*!< Defines the time in ms after which all
                                        the bitstream associated meta-data (DRC,
                                        downmix coefficients, ...) will be reset
                                        to default if no update has been
                                        received. Negative values disable the
                                        feature. */

  AAC_CONCEAL_METHOD = 0x0100, /*!< Error concealment: Processing method. \n
                                    0: Spectral muting. \n
                                    1: Noise substitution (see ::CONCEAL_NOISE).
                                  \n 2: Energy interpolation (adds additional
                                  signal delay of one frame, see
                                  ::CONCEAL_INTER. only some AOTs are
                                  supported). \n */
  AAC_DRC_BOOST_FACTOR =
      0x0200, /*!< MPEG-4 / MPEG-D Dynamic Range Control (DRC): Scaling factor
                 for boosting gain values. Defines how the boosting DRC factors
                 (conveyed in the bitstream) will be applied to the decoded
                 signal. The valid values range from 0 (don't apply boost
                 factors) to 127 (fully apply boost factors). Default value is 0
                 for MPEG-4 DRC and 127 for MPEG-D DRC. */
  AAC_DRC_ATTENUATION_FACTOR = 0x0201, /*!< MPEG-4 / MPEG-D DRC: Scaling factor
                                          for attenuating gain values. Same as
                                            ::AAC_DRC_BOOST_FACTOR but for
                                          attenuating DRC factors. */
  AAC_DRC_REFERENCE_LEVEL =
      0x0202, /*!< MPEG-4 / MPEG-D DRC: Target reference level / decoder target
                 loudness.\n Defines the level below full-scale (quantized in
                 steps of 0.25dB) to which the output audio signal will be
                 normalized to by the DRC module.\n The parameter controls
                 loudness normalization for both MPEG-4 DRC and MPEG-D DRC. The
                 valid values range from 40 (-10 dBFS) to 127 (-31.75 dBFS).\n
                   Example values:\n
                   124 (-31 dBFS) for audio/video receivers (AVR) or other
                 devices allowing audio playback with high dynamic range,\n 96
                 (-24 dBFS) for TV sets or equivalent devices (default),\n 64
                 (-16 dBFS) for mobile devices where the dynamic range of audio
                 playback is restricted.\n Any value smaller than 0 switches off
                 loudness normalization and MPEG-4 DRC. */
  AAC_DRC_HEAVY_COMPRESSION =
      0x0203, /*!< MPEG-4 DRC: En-/Disable DVB specific heavy compression (aka
                 RF mode). If set to 1, the decoder will apply the compression
                 values from the DVB specific ancillary data field. At the same
                 time the MPEG-4 Dynamic Range Control tool will be disabled. By
                   default, heavy compression is disabled. */
  AAC_DRC_DEFAULT_PRESENTATION_MODE =
      0x0204, /*!< MPEG-4 DRC: Default presentation mode (DRC parameter
                 handling). \n Defines the handling of the DRC parameters boost
                 factor, attenuation factor and heavy compression, if no
                 presentation mode is indicated in the bitstream.\n For options,
                 see ::AAC_DRC_DEFAULT_PRESENTATION_MODE_OPTIONS.\n Default:
                 ::AAC_DRC_PARAMETER_HANDLING_DISABLED */
  AAC_DRC_ENC_TARGET_LEVEL =
      0x0205, /*!< MPEG-4 DRC: Encoder target level for light (i.e. not heavy)
                 compression.\n If known, this declares the target reference
                 level that was assumed at the encoder for calculation of
                 limiting gains. The valid values range from 0 (full-scale) to
                 127 (31.75 dB below full-scale). This parameter is used only
                 with ::AAC_DRC_PARAMETER_HANDLING_ENABLED and ignored
                 otherwise.\n Default: 127 (worst-case assumption).\n */
  AAC_UNIDRC_SET_EFFECT = 0x0206, /*!< MPEG-D DRC: Request a DRC effect type for
                                     selection of a DRC set.\n Supported indices
                                     are:\n -1: DRC off. Completely disables
                                     MPEG-D DRC.\n 0: None (default). Disables
                                     MPEG-D DRC, but automatically enables DRC
                                     if necessary to prevent clipping.\n 1: Late
                                     night\n 2: Noisy environment\n 3: Limited
                                     playback range\n 4: Low playback level\n 5:
                                     Dialog enhancement\n 6: General
                                     compression. Used for generally enabling
                                     MPEG-D DRC without particular request.\n */
  AAC_UNIDRC_ALBUM_MODE =
      0x0207, /*!<  MPEG-D DRC: Enable album mode. 0: Disabled (default), 1:
                 Enabled.\n Disabled album mode leads to application of gain
                 sequences for fading in and out, if provided in the
                 bitstream.\n Enabled album mode makes use of dedicated album
                 loudness information, if provided in the bitstream.\n */
  AAC_QMF_LOWPOWER =
      0x0300, /*!< Quadrature Mirror Filter (QMF) Bank processing mode. \n
                   -1: Use internal default. \n
                    0: Use complex QMF data mode. \n
                    1: Use real (low power) QMF data mode. \n */
  AAC_TPDEC_CLEAR_BUFFER =
      0x0603 /*!< Clear internal bit stream buffer of transport layers. The
                decoder will start decoding at new data passed after this event
                and any previous data is discarded. */

} AACDEC_PARAM;

/**
 * \brief This structure gives information about the currently decoded audio
 * data. All fields are read-only.
 */
typedef struct {
  /* These five members are the only really relevant ones for the user. */
  INT sampleRate; /*!< The sample rate in Hz of the decoded PCM audio signal. */
  INT frameSize;  /*!< The frame size of the decoded PCM audio signal. \n
                       Typically this is: \n
                       1024 or 960 for AAC-LC \n
                       2048 or 1920 for HE-AAC (v2) \n
                       512 or 480 for AAC-LD and AAC-ELD \n
                       768, 1024, 2048 or 4096 for USAC  */
  INT numChannels; /*!< The number of output audio channels before the rendering
                      module, i.e. the original channel configuration. */
  AUDIO_CHANNEL_TYPE
  *pChannelType; /*!< Audio channel type of each output audio channel. */
  UCHAR *pChannelIndices; /*!< Audio channel index for each output audio
                             channel. See ISO/IEC 13818-7:2005(E), 8.5.3.2
                             Explicit channel mapping using a
                             program_config_element() */
  /* Decoder internal members. */
  INT aacSampleRate; /*!< Sampling rate in Hz without SBR (from configuration
                        info) divided by a (ELD) downscale factor if present. */
  INT profile; /*!< MPEG-2 profile (from file header) (-1: not applicable (e. g.
                  MPEG-4)).               */
  AUDIO_OBJECT_TYPE
  aot; /*!< Audio Object Type (from ASC): is set to the appropriate value
          for MPEG-2 bitstreams (e. g. 2 for AAC-LC). */
  INT channelConfig; /*!< Channel configuration (0: PCE defined, 1: mono, 2:
                        stereo, ...                       */
  INT bitRate;       /*!< Instantaneous bit rate.                   */
  INT aacSamplesPerFrame;   /*!< Samples per frame for the AAC core (from ASC)
                               divided by a (ELD) downscale factor if present. \n
                                 Typically this is (with a downscale factor of 1):
                               \n   1024 or 960 for AAC-LC \n   512 or 480 for
                               AAC-LD   and AAC-ELD         */
  INT aacNumChannels;       /*!< The number of audio channels after AAC core
                               processing (before PS or MPS processing).       CAUTION: This
                               are not the final number of output channels! */
  AUDIO_OBJECT_TYPE extAot; /*!< Extension Audio Object Type (from ASC)   */
  INT extSamplingRate; /*!< Extension sampling rate in Hz (from ASC) divided by
                          a (ELD) downscale factor if present. */

  UINT outputDelay; /*!< The number of samples the output is additionally
                       delayed by.the decoder. */
  UINT flags; /*!< Copy of internal flags. Only to be written by the decoder,
                 and only to be read externally. */

  SCHAR epConfig; /*!< epConfig level (from ASC): only level 0 supported, -1
                     means no ER (e. g. AOT=2, MPEG-2 AAC, etc.)  */
  /* Statistics */
  INT numLostAccessUnits; /*!< This integer will reflect the estimated amount of
                             lost access units in case aacDecoder_DecodeFrame()
                               returns AAC_DEC_TRANSPORT_SYNC_ERROR. It will be
                             < 0 if the estimation failed. */

  INT64 numTotalBytes; /*!< This is the number of total bytes that have passed
                          through the decoder. */
  INT64
  numBadBytes; /*!< This is the number of total bytes that were considered
                  with errors from numTotalBytes. */
  INT64
  numTotalAccessUnits;     /*!< This is the number of total access units that
                              have passed through the decoder. */
  INT64 numBadAccessUnits; /*!< This is the number of total access units that
                              were considered with errors from numTotalBytes. */

  /* Metadata */
  SCHAR drcProgRefLev; /*!< DRC program reference level. Defines the reference
                          level below full-scale. It is quantized in steps of
                          0.25dB. The valid values range from 0 (0 dBFS) to 127
                          (-31.75 dBFS). It is used to reflect the average
                          loudness of the audio in LKFS according to ITU-R BS
                          1770. If no level has been found in the bitstream the
                          value is -1. */
  SCHAR
  drcPresMode;        /*!< DRC presentation mode. According to ETSI TS 101 154,
                         this field indicates whether   light (MPEG-4 Dynamic Range
                         Control tool) or heavy compression (DVB heavy
                         compression)   dynamic range control shall take priority
                         on the outputs.   For details, see ETSI TS 101 154, table
                         C.33. Possible values are: \n   -1: No corresponding
                         metadata found in the bitstream \n   0: DRC presentation
                         mode not indicated \n   1: DRC presentation mode 1 \n   2:
                         DRC presentation mode 2 \n   3: Reserved */
  INT outputLoudness; /*!< Audio output loudness in steps of -0.25 dB. Range: 0
                         (0 dBFS) to 231 (-57.75 dBFS).\n  A value of -1
                         indicates that no loudness metadata is present.\n  If
                         loudness normalization is active, the value corresponds
                         to the target loudness value set with
                         ::AAC_DRC_REFERENCE_LEVEL.\n  If loudness normalization
                         is not active, the output loudness value corresponds to
                         the loudness metadata given in the bitstream.\n
                           Loudness metadata can originate from MPEG-4 DRC or
                         MPEG-D DRC. */

} CStreamInfo;

typedef struct AAC_DECODER_INSTANCE
    *HANDLE_AACDECODER; /*!< Pointer to a AAC decoder instance. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize ancillary data buffer.
 *
 * \param self    AAC decoder handle.
 * \param buffer  Pointer to (external) ancillary data buffer.
 * \param size    Size of the buffer pointed to by buffer.
 * \return        Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_AncDataInit(HANDLE_AACDECODER self,
                                                    UCHAR *buffer, int size);

/**
 * \brief Get one ancillary data element.
 *
 * \param self   AAC decoder handle.
 * \param index  Index of the ancillary data element to get.
 * \param ptr    Pointer to a buffer receiving a pointer to the requested
 * ancillary data element.
 * \param size   Pointer to a buffer receiving the length of the requested
 * ancillary data element.
 * \return       Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_AncDataGet(HANDLE_AACDECODER self,
                                                   int index, UCHAR **ptr,
                                                   int *size);

/**
 * \brief Set one single decoder parameter.
 *
 * \param self   AAC decoder handle.
 * \param param  Parameter to be set.
 * \param value  Parameter value.
 * \return       Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_SetParam(const HANDLE_AACDECODER self,
                                                 const AACDEC_PARAM param,
                                                 const INT value);

/**
 * \brief              Get free bytes inside decoder internal buffer.
 * \param self         Handle of AAC decoder instance.
 * \param pFreeBytes   Pointer to variable receiving amount of free bytes inside
 * decoder internal buffer.
 * \return             Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR
aacDecoder_GetFreeBytes(const HANDLE_AACDECODER self, UINT *pFreeBytes);

/**
 * \brief               Open an AAC decoder instance.
 * \param transportFmt  The transport type to be used.
 * \param nrOfLayers    Number of transport layers.
 * \return              AAC decoder handle.
 */
LINKSPEC_H HANDLE_AACDECODER aacDecoder_Open(TRANSPORT_TYPE transportFmt,
                                             UINT nrOfLayers);

/**
 * \brief Explicitly configure the decoder by passing a raw AudioSpecificConfig
 * (ASC) or a StreamMuxConfig (SMC), contained in a binary buffer. This is
 * required for MPEG-4 and Raw Packets file format bitstreams as well as for
 * LATM bitstreams with no in-band SMC. If the transport format is LATM with or
 * without LOAS, configuration is assumed to be an SMC, for all other file
 * formats an ASC.
 *
 * \param self    AAC decoder handle.
 * \param conf    Pointer to an unsigned char buffer containing the binary
 * configuration buffer (either ASC or SMC).
 * \param length  Length of the configuration buffer in bytes.
 * \return        Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_ConfigRaw(HANDLE_AACDECODER self,
                                                  UCHAR *conf[],
                                                  const UINT length[]);

/**
 * \brief Submit raw ISO base media file format boxes to decoder for parsing
 * (only some box types are recognized).
 *
 * \param self    AAC decoder handle.
 * \param buffer  Pointer to an unsigned char buffer containing the binary box
 * data (including size and type, can be a sequence of multiple boxes).
 * \param length  Length of the data in bytes.
 * \return        Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_RawISOBMFFData(HANDLE_AACDECODER self,
                                                       UCHAR *buffer,
                                                       UINT length);

/**
 * \brief Fill AAC decoder's internal input buffer with bitstream data from the
 * external input buffer. The function only copies such data as long as the
 * decoder-internal input buffer is not full. So it grabs whatever it can from
 * pBuffer and returns information (bytesValid) so that at a subsequent call of
 * %aacDecoder_Fill(), the right position in pBuffer can be determined to grab
 * the next data.
 *
 * \param self        AAC decoder handle.
 * \param pBuffer     Pointer to external input buffer.
 * \param bufferSize  Size of external input buffer. This argument is required
 * because decoder-internally we need the information to calculate the offset to
 * pBuffer, where the next available data is, which is then
 * fed into the decoder-internal buffer (as much as
 * possible). Our example framework implementation fills the
 * buffer at pBuffer again, once it contains no available valid bytes anymore
 * (meaning bytesValid equal 0).
 * \param bytesValid  Number of bitstream bytes in the external bitstream buffer
 * that have not yet been copied into the decoder's internal bitstream buffer by
 * calling this function. The value is updated according to
 * the amount of newly copied bytes.
 * \return            Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_Fill(HANDLE_AACDECODER self,
                                             UCHAR *pBuffer[],
                                             const UINT bufferSize[],
                                             UINT *bytesValid);

/** Flag for aacDecoder_DecodeFrame(): Trigger the built-in error concealment
 * module to generate a substitute signal for one lost frame. New input data
 * will not be considered.
 */
#define AACDEC_CONCEAL 1
/** Flag for aacDecoder_DecodeFrame(): Flush all filterbanks to get all delayed
 * audio without having new input data. Thus new input data will not be
 * considered.
 */
#define AACDEC_FLUSH 2
/** Flag for aacDecoder_DecodeFrame(): Signal an input bit stream data
 * discontinuity. Resync any internals as necessary.
 */
#define AACDEC_INTR 4
/** Flag for aacDecoder_DecodeFrame(): Clear all signal delay lines and history
 * buffers. CAUTION: This can cause discontinuities in the output signal.
 */
#define AACDEC_CLRHIST 8

/**
 * \brief               Decode one audio frame
 *
 * \param self          AAC decoder handle.
 * \param pTimeData     Pointer to external output buffer where the decoded PCM
 * samples will be stored into.
 * \param timeDataSize  Size of external output buffer in PCM samples.
 * \param flags         Bit field with flags for the decoder: \n
 *                      (flags & AACDEC_CONCEAL) == 1: Do concealment. \n
 *                      (flags & AACDEC_FLUSH) == 2: Discard input data. Flush
 * filter banks (output delayed audio). \n (flags & AACDEC_INTR) == 4: Input
 * data is discontinuous. Resynchronize any internals as
 * necessary. \n (flags & AACDEC_CLRHIST) == 8: Clear all signal delay lines and
 * history buffers.
 * \return              Error code.
 */
LINKSPEC_H AAC_DECODER_ERROR aacDecoder_DecodeFrame(HANDLE_AACDECODER self,
                                                    INT_PCM *pTimeData,
                                                    const INT timeDataSize,
                                                    const UINT flags);

/**
 * \brief       De-allocate all resources of an AAC decoder instance.
 *
 * \param self  AAC decoder handle.
 * \return      void.
 */
LINKSPEC_H void aacDecoder_Close(HANDLE_AACDECODER self);

/**
 * \brief       Get CStreamInfo handle from decoder.
 *
 * \param self  AAC decoder handle.
 * \return      Reference to requested CStreamInfo.
 */
LINKSPEC_H CStreamInfo *aacDecoder_GetStreamInfo(HANDLE_AACDECODER self);

/**
 * \brief       Get decoder library info.
 *
 * \param info  Pointer to an allocated LIB_INFO structure.
 * \return      0 on success.
 */
LINKSPEC_H INT aacDecoder_GetLibInfo(LIB_INFO *info);

#ifdef __cplusplus
}
#endif

#endif /* AACDECODER_LIB_H */
