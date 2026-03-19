/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2021 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):   Daniel Homm

   Description:

*******************************************************************************/

#ifndef TPDEC_LATM_H
#define TPDEC_LATM_H

#include "tpdec_lib.h"

#include "FDK_bitstream.h"

#define MIN_LATM_HEADERLENGTH 9
#define MIN_LOAS_HEADERLENGTH MIN_LATM_HEADERLENGTH + 24 /* both in bits */
#define MIN_TP_BUF_SIZE_LOAS (8194)

enum {
  LATM_MAX_PROG = 1,
  LATM_MAX_LAYER = 1,
  LATM_MAX_VAR_CHUNKS = 16,
  LATM_MAX_ID = 16
};

typedef struct {
  UINT m_frameLengthType;
  UINT m_bufferFullness;
  UINT m_streamID;
  UINT m_frameLengthInBits;
} LATM_LAYER_INFO;

typedef struct {
  LATM_LAYER_INFO m_linfo[LATM_MAX_PROG][LATM_MAX_LAYER];
  UINT m_taraBufferFullness;
  UINT m_otherDataLength;
  UINT m_audioMuxLengthBytes; /* Length of LOAS payload */

  UCHAR m_useSameStreamMux;
  UCHAR m_AudioMuxVersion;
  UCHAR m_AudioMuxVersionA;
  UCHAR m_allStreamsSameTimeFraming;
  UCHAR m_noSubFrames;
  UCHAR m_numProgram;
  UCHAR m_numLayer[LATM_MAX_PROG];

  UCHAR m_otherDataPresent;
  UCHAR m_crcCheckPresent;

  SCHAR BufferFullnessAchieved;
  UCHAR
  usacExplicitCfgChanged;      /* explicit config in case of USAC and LOAS/LATM
                                  must be compared to IPF cfg */
  UCHAR applyAsc;              /* apply ASC immediate without flushing */
  UCHAR newCfgHasAudioPreRoll; /* the new (dummy parsed) config has an
                                  AudioPreRoll */
} CLatmDemux;

TRANSPORTDEC_ERROR CLatmDemux_Read(HANDLE_FDK_BITSTREAM bs,
                                   CLatmDemux *pLatmDemux, TRANSPORT_TYPE tt,
                                   CSTpCallBacks *pTpDecCallbacks,
                                   CSAudioSpecificConfig *pAsc,
                                   int *pfConfigFound,
                                   const INT ignoreBufferFullness);

/**
 * \brief Read StreamMuxConfig
 * \param bs bit stream handle as data source
 * \param pLatmDemux pointer to CLatmDemux struct of current LATM context
 * \param pTpDecCallbacks Call back structure for configuration callbacks
 * \param pAsc pointer to a ASC for configuration storage
 * \param pfConfigFound pointer to a flag which is set to 1 if a configuration
 * was found and processed successfully
 * \param configMode Config modes: memory allocation mode or config change
 * detection mode
 * \param configChanged Indicates a config change
 * \return error code
 */
TRANSPORTDEC_ERROR CLatmDemux_ReadStreamMuxConfig(
    HANDLE_FDK_BITSTREAM bs, CLatmDemux *pLatmDemux,
    CSTpCallBacks *pTpDecCallbacks, CSAudioSpecificConfig *pAsc,
    int *pfConfigFound, UCHAR configMode, UCHAR configChanged);

TRANSPORTDEC_ERROR CLatmDemux_ReadPayloadLengthInfo(HANDLE_FDK_BITSTREAM bs,
                                                    CLatmDemux *pLatmDemux);

UINT CLatmDemux_GetFrameLengthInBits(CLatmDemux *pLatmDemux, const UINT prog,
                                     const UINT layer);
UINT CLatmDemux_GetOtherDataPresentFlag(CLatmDemux *pLatmDemux);
UINT CLatmDemux_GetOtherDataLength(CLatmDemux *pLatmDemux);
UINT CLatmDemux_GetNrOfSubFrames(CLatmDemux *pLatmDemux);
UINT CLatmDemux_GetNrOfLayers(CLatmDemux *pLatmDemux, const UINT program);

#endif /* TPDEC_LATM_H */
