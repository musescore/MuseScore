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

/**************************** AAC encoder library ******************************

   Author(s):   M. Schug / A. Groeschel

   Description: bandwidth expert

*******************************************************************************/

#include "channel_map.h"
#include "bandwidth.h"
#include "aacEnc_ram.h"

typedef struct {
  INT chanBitRate;
  INT bandWidthMono;
  INT bandWidth2AndMoreChan;

} BANDWIDTH_TAB;

static const BANDWIDTH_TAB bandWidthTable[] = {
    {0, 3700, 5000},       {12000, 5000, 6400},   {20000, 6900, 9640},
    {28000, 9600, 13050},  {40000, 12060, 14260}, {56000, 13950, 15500},
    {72000, 14200, 16120}, {96000, 17000, 17000}, {576001, 17000, 17000}};

static const BANDWIDTH_TAB bandWidthTable_LD_22050[] = {
    {8000, 2000, 2400},    {12000, 2500, 2700},   {16000, 3300, 3100},
    {24000, 6250, 7200},   {32000, 9200, 10500},  {40000, 16000, 16000},
    {48000, 16000, 16000}, {282241, 16000, 16000}};

static const BANDWIDTH_TAB bandWidthTable_LD_24000[] = {
    {8000, 2000, 2000},    {12000, 2000, 2300},   {16000, 2200, 2500},
    {24000, 5650, 7200},   {32000, 11600, 12000}, {40000, 12000, 16000},
    {48000, 16000, 16000}, {64000, 16000, 16000}, {307201, 16000, 16000}};

static const BANDWIDTH_TAB bandWidthTable_LD_32000[] = {
    {8000, 2000, 2000},    {12000, 2000, 2000},   {24000, 4250, 7200},
    {32000, 8400, 9000},   {40000, 9400, 11300},  {48000, 11900, 14700},
    {64000, 14800, 16000}, {76000, 16000, 16000}, {409601, 16000, 16000}};

static const BANDWIDTH_TAB bandWidthTable_LD_44100[] = {
    {8000, 2000, 2000},     {24000, 2000, 2000},   {32000, 4400, 5700},
    {40000, 7400, 8800},    {48000, 9000, 10700},  {56000, 11000, 12900},
    {64000, 14400, 15500},  {80000, 16000, 16200}, {96000, 16500, 16000},
    {128000, 16000, 16000}, {564481, 16000, 16000}};

static const BANDWIDTH_TAB bandWidthTable_LD_48000[] = {
    {8000, 2000, 2000},     {24000, 2000, 2000},   {32000, 4400, 5700},
    {40000, 7400, 8800},    {48000, 9000, 10700},  {56000, 11000, 12800},
    {64000, 14300, 15400},  {80000, 16000, 16200}, {96000, 16500, 16000},
    {128000, 16000, 16000}, {614401, 16000, 16000}};

typedef struct {
  AACENC_BITRATE_MODE bitrateMode;
  int bandWidthMono;
  int bandWidth2AndMoreChan;
} BANDWIDTH_TAB_VBR;

static const BANDWIDTH_TAB_VBR bandWidthTableVBR[] = {
    {AACENC_BR_MODE_CBR, 0, 0},
    {AACENC_BR_MODE_VBR_1, 13000, 13000},
    {AACENC_BR_MODE_VBR_2, 13000, 13000},
    {AACENC_BR_MODE_VBR_3, 15750, 15750},
    {AACENC_BR_MODE_VBR_4, 16500, 16500},
    {AACENC_BR_MODE_VBR_5, 19293, 19293},
    {AACENC_BR_MODE_SFR, 0, 0},
    {AACENC_BR_MODE_FF, 0, 0}

};

static INT GetBandwidthEntry(const INT frameLength, const INT sampleRate,
                             const INT chanBitRate, const INT entryNo) {
  INT bandwidth = -1;
  const BANDWIDTH_TAB *pBwTab = NULL;
  INT bwTabSize = 0;

  switch (frameLength) {
    case 960:
    case 1024:
      pBwTab = bandWidthTable;
      bwTabSize = sizeof(bandWidthTable) / sizeof(BANDWIDTH_TAB);
      break;
    case 120:
    case 128:
    case 240:
    case 256:
    case 480:
    case 512:
      switch (sampleRate) {
        case 8000:
        case 11025:
        case 12000:
        case 16000:
        case 22050:
          pBwTab = bandWidthTable_LD_22050;
          bwTabSize = sizeof(bandWidthTable_LD_22050) / sizeof(BANDWIDTH_TAB);
          break;
        case 24000:
          pBwTab = bandWidthTable_LD_24000;
          bwTabSize = sizeof(bandWidthTable_LD_24000) / sizeof(BANDWIDTH_TAB);
          break;
        case 32000:
          pBwTab = bandWidthTable_LD_32000;
          bwTabSize = sizeof(bandWidthTable_LD_32000) / sizeof(BANDWIDTH_TAB);
          break;
        case 44100:
          pBwTab = bandWidthTable_LD_44100;
          bwTabSize = sizeof(bandWidthTable_LD_44100) / sizeof(BANDWIDTH_TAB);
          break;
        case 48000:
        case 64000:
        case 88200:
        case 96000:
          pBwTab = bandWidthTable_LD_48000;
          bwTabSize = sizeof(bandWidthTable_LD_48000) / sizeof(BANDWIDTH_TAB);
          break;
      }
      break;
    default:
      pBwTab = NULL;
      bwTabSize = 0;
  }

  if (pBwTab != NULL) {
    int i;
    for (i = 0; i < bwTabSize - 1; i++) {
      if (chanBitRate >= pBwTab[i].chanBitRate &&
          chanBitRate < pBwTab[i + 1].chanBitRate) {
        switch (frameLength) {
          case 960:
          case 1024:
            bandwidth = (entryNo == 0) ? pBwTab[i].bandWidthMono
                                       : pBwTab[i].bandWidth2AndMoreChan;
            break;
          case 120:
          case 128:
          case 240:
          case 256:
          case 480:
          case 512: {
            INT q_res = 0;
            INT startBw = (entryNo == 0) ? pBwTab[i].bandWidthMono
                                         : pBwTab[i].bandWidth2AndMoreChan;
            INT endBw = (entryNo == 0) ? pBwTab[i + 1].bandWidthMono
                                       : pBwTab[i + 1].bandWidth2AndMoreChan;
            INT startBr = pBwTab[i].chanBitRate;
            INT endBr = pBwTab[i + 1].chanBitRate;

            FIXP_DBL bwFac_fix =
                fDivNorm(chanBitRate - startBr, endBr - startBr, &q_res);
            bandwidth =
                (INT)scaleValue(fMult(bwFac_fix, (FIXP_DBL)(endBw - startBw)),
                                q_res) +
                startBw;
          } break;
          default:
            bandwidth = -1;
        }
        break;
      } /* within bitrate range */
    }
  } /* pBwTab!=NULL */

  return bandwidth;
}

AAC_ENCODER_ERROR FDKaacEnc_DetermineBandWidth(
    const INT proposedBandWidth, const INT bitrate,
    const AACENC_BITRATE_MODE bitrateMode, const INT sampleRate,
    const INT frameLength, const CHANNEL_MAPPING *const cm,
    const CHANNEL_MODE encoderMode, INT *const bandWidth) {
  AAC_ENCODER_ERROR ErrorStatus = AAC_ENC_OK;
  INT chanBitRate = bitrate / cm->nChannelsEff;

  switch (bitrateMode) {
    case AACENC_BR_MODE_VBR_1:
    case AACENC_BR_MODE_VBR_2:
    case AACENC_BR_MODE_VBR_3:
    case AACENC_BR_MODE_VBR_4:
    case AACENC_BR_MODE_VBR_5:
      if (proposedBandWidth != 0) {
        /* use given bw */
        *bandWidth = proposedBandWidth;
      } else {
        /* take bw from table */
        switch (encoderMode) {
          case MODE_1:
            *bandWidth = bandWidthTableVBR[bitrateMode].bandWidthMono;
            break;
          case MODE_2:
          case MODE_1_2:
          case MODE_1_2_1:
          case MODE_1_2_2:
          case MODE_1_2_2_1:
          case MODE_6_1:
          case MODE_1_2_2_2_1:
          case MODE_7_1_REAR_SURROUND:
          case MODE_7_1_FRONT_CENTER:
          case MODE_7_1_BACK:
          case MODE_7_1_TOP_FRONT:
            *bandWidth = bandWidthTableVBR[bitrateMode].bandWidth2AndMoreChan;
            break;
          default:
            return AAC_ENC_UNSUPPORTED_CHANNELCONFIG;
        }
      }
      break;
    case AACENC_BR_MODE_CBR:
    case AACENC_BR_MODE_SFR:
    case AACENC_BR_MODE_FF:

      /* bandwidth limiting */
      if (proposedBandWidth != 0) {
        *bandWidth = fMin(proposedBandWidth, fMin(20000, sampleRate >> 1));
      } else { /* search reasonable bandwidth */

        int entryNo = 0;

        switch (encoderMode) {
          case MODE_1:   /* mono      */
            entryNo = 0; /* use mono bandwidth settings */
            break;

          case MODE_2:       /* stereo    */
          case MODE_1_2:     /* sce + cpe */
          case MODE_1_2_1:   /* sce + cpe + sce */
          case MODE_1_2_2:   /* sce + cpe + cpe */
          case MODE_1_2_2_1: /* (5.1) sce + cpe + cpe + lfe */
          case MODE_6_1:
          case MODE_1_2_2_2_1:
          case MODE_7_1_REAR_SURROUND:
          case MODE_7_1_FRONT_CENTER:
          case MODE_7_1_BACK:
          case MODE_7_1_TOP_FRONT:
            entryNo = 1; /* use stereo bandwidth settings */
            break;

          default:
            return AAC_ENC_UNSUPPORTED_CHANNELCONFIG;
        }

        *bandWidth =
            GetBandwidthEntry(frameLength, sampleRate, chanBitRate, entryNo);

        if (*bandWidth == -1) {
          switch (frameLength) {
            case 120:
            case 128:
            case 240:
            case 256:
              *bandWidth = 16000;
              break;
            default:
              ErrorStatus = AAC_ENC_INVALID_CHANNEL_BITRATE;
          }
        }
      }
      break;
    default:
      *bandWidth = 0;
      return AAC_ENC_UNSUPPORTED_BITRATE_MODE;
  }

  *bandWidth = fMin(*bandWidth, sampleRate / 2);

  return ErrorStatus;
}
