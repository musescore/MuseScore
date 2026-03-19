/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):   Christian Griebel

   Description: Dynamic range control (DRC) global data types

*******************************************************************************/

#ifndef AACDEC_DRC_TYPES_H
#define AACDEC_DRC_TYPES_H

#include "common_fix.h"

#define MAX_DRC_THREADS \
  ((8) + 1) /* Heavy compression value is handled just like MPEG DRC data */
#define MAX_DRC_BANDS (16) /* 2^LEN_DRC_BAND_INCR (LEN_DRC_BAND_INCR = 4) */

/**
 * \brief DRC module global data types
 */
typedef enum {
  UNKNOWN_PAYLOAD = 0,
  MPEG_DRC_EXT_DATA = 1,
  DVB_DRC_ANC_DATA = 2

} AACDEC_DRC_PAYLOAD_TYPE;

/**
 * \brief Options for parameter handling / presentation mode
 */
typedef enum {
  DISABLED_PARAMETER_HANDLING = -1, /*!< DRC parameter handling disabled, all
                                       parameters are applied as requested. */
  ENABLED_PARAMETER_HANDLING =
      0, /*!< Apply changes to requested DRC parameters to prevent clipping */
  DRC_PRESENTATION_MODE_1 = 1, /*!< DRC Presentation mode 1*/
  DRC_PRESENTATION_MODE_2 = 2  /*!< DRC Presentation mode 2*/

} AACDEC_DRC_PARAMETER_HANDLING;

typedef struct {
  UINT expiryCount;
  UINT numBands;
  USHORT bandTop[MAX_DRC_BANDS];
  SHORT drcInterpolationScheme;
  UCHAR drcValue[MAX_DRC_BANDS];
  SCHAR drcDataType;

} CDrcChannelData;

typedef struct {
  UINT excludedChnsMask;
  SCHAR progRefLevel;
  SCHAR presMode; /* Presentation mode: 0 (not indicated), 1, 2, and 3
                     (reserved). */
  SCHAR pceInstanceTag;

  CDrcChannelData channelData;

} CDrcPayload;

typedef struct {
  /* DRC parameters: Latest user requests */
  FIXP_DBL usrCut;
  FIXP_DBL usrBoost;
  UCHAR usrApplyHeavyCompression;

  /* DRC parameters: Currently used, possibly changed by
   * aacDecoder_drcParameterHandling */
  FIXP_DBL cut;         /* attenuation scale factor */
  FIXP_DBL boost;       /* boost scale factor  */
  SCHAR targetRefLevel; /* target reference level for loudness normalization */
  UCHAR applyHeavyCompression; /* heavy compression (DVB) flag */

  UINT expiryFrame;
  UCHAR bsDelayEnable;

  AACDEC_DRC_PARAMETER_HANDLING defaultPresentationMode;
  UCHAR encoderTargetLevel;

} CDrcParams;

typedef struct {
  CDrcParams
      params; /* Module parameters that can be set by user (via SetParam API
                 function) */

  UCHAR enable;      /* Switch that controls dynamic range processing */
  UCHAR digitalNorm; /* Switch to en-/disable reference level normalization in
                        digital domain */

  UCHAR update; /* Flag indicating the change of a user or bitstream parameter
                   which affects aacDecoder_drcParameterHandling */
  INT numOutChannels;     /* Number of output channels */
  INT prevAacNumChannels; /* Previous number of channels of aac bitstream, used
                             for update flag */

  USHORT numPayloads; /* The number of DRC data payload elements found within
                         frame */
  USHORT
  numThreads;         /* The number of DRC data threads extracted from the found
                         payload elements */
  SCHAR progRefLevel; /* Program reference level for all channels */
  UCHAR progRefLevelPresent; /* Program reference level found in bitstream */

  UINT prlExpiryCount; /* Counter that can be used to monitor the life time of
                          the program reference level. */

  SCHAR presMode; /* Presentation mode as defined in ETSI TS 101 154 */
  UCHAR dvbAncDataAvailable; /* Flag that indicates whether DVB ancillary data
                                is present or not */
  UINT dvbAncDataPosition;   /* Used to store the DVB ancillary data payload
                                position in the bitstream (only one per frame) */
  UINT drcPayloadPosition[MAX_DRC_THREADS]; /* Used to store the DRC payload
                                               positions in the bitstream */
  UCHAR applyExtGain; /* Flag is 1 if extGain has to be applied, otherwise 0. */

  FIXP_DBL additionalGainPrev; /* Gain of previous frame to be applied to the
                                  time data */
  FIXP_DBL additionalGainFilterState;  /* Filter state for the gain smoothing */
  FIXP_DBL additionalGainFilterState1; /* Filter state for the gain smoothing */

} CDrcInfo;

typedef CDrcInfo *HANDLE_AAC_DRC;

#endif /* AACDEC_DRC_TYPES_H */
