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

/******************* MPEG transport format encoder library *********************

   Author(s):

   Description:

*******************************************************************************/

#include "tp_data.h"

#include "tpenc_lib.h"
#include "tpenc_asc.h"
#include "FDK_bitstream.h"
#include "genericStds.h"

#include "FDK_crc.h"

#define PCE_HEIGHT_EXT_SYNC (0xAC)
#define HEIGHT_NORMAL 0
#define HEIGHT_TOP 1
#define HEIGHT_BOTTOM 2
#define MAX_FRONT_ELEMENTS 8
#define MAX_SIDE_ELEMENTS 3
#define MAX_BACK_ELEMENTS 4

/**
 *  Describe additional PCE height information for front, side and back channel
 * elements.
 */
typedef struct {
  UCHAR
  num_front_height_channel_elements[2];      /*!< Number of front channel
                                                elements in top [0] and bottom
                                                [1] plane. */
  UCHAR num_side_height_channel_elements[2]; /*!< Number of side channel
                                                elements in top [0] and bottom
                                                [1] plane. */
  UCHAR num_back_height_channel_elements[2]; /*!< Number of back channel
                                                elements in top [0] and bottom
                                                [1] plane. */
} PCE_HEIGHT_NUM;

/**
 *  Describe a PCE based on placed channel elements and element type sequence.
 */
typedef struct {
  UCHAR num_front_channel_elements; /*!< Number of front channel elements. */
  UCHAR num_side_channel_elements;  /*!< Number of side channel elements. */
  UCHAR num_back_channel_elements;  /*!< Number of back channel elements. */
  UCHAR num_lfe_channel_elements;   /*!< Number of lfe channel elements. */
  const MP4_ELEMENT_ID
      *pEl_type; /*!< List contains sequence describing the elements
                      in present channel mode. (MPEG order) */
  const PCE_HEIGHT_NUM *pHeight_num;
} PCE_CONFIGURATION;

/**
 *  Map an incoming channel mode to a existing PCE configuration entry.
 */
typedef struct {
  CHANNEL_MODE channel_mode; /*!< Present channel mode. */
  PCE_CONFIGURATION
  pce_configuration; /*!< Program config element description. */

} CHANNEL_CONFIGURATION;

/**
 * The following arrays provide the IDs of the consecutive elements for each
 * mode.
 */
static const MP4_ELEMENT_ID elType_1[] = {ID_SCE};
static const MP4_ELEMENT_ID elType_2[] = {ID_CPE};
static const MP4_ELEMENT_ID elType_1_2[] = {ID_SCE, ID_CPE};
static const MP4_ELEMENT_ID elType_1_2_1[] = {ID_SCE, ID_CPE, ID_SCE};
static const MP4_ELEMENT_ID elType_1_2_2[] = {ID_SCE, ID_CPE, ID_CPE};
static const MP4_ELEMENT_ID elType_1_2_2_1[] = {ID_SCE, ID_CPE, ID_CPE, ID_LFE};
static const MP4_ELEMENT_ID elType_1_2_2_2_1[] = {ID_SCE, ID_CPE, ID_CPE,
                                                  ID_CPE, ID_LFE};
static const MP4_ELEMENT_ID elType_6_1[] = {ID_SCE, ID_CPE, ID_CPE, ID_SCE,
                                            ID_LFE};
static const MP4_ELEMENT_ID elType_7_1_back[] = {ID_SCE, ID_CPE, ID_CPE, ID_CPE,
                                                 ID_LFE};
static const MP4_ELEMENT_ID elType_7_1_top_front[] = {ID_SCE, ID_CPE, ID_CPE,
                                                      ID_LFE, ID_CPE};
static const MP4_ELEMENT_ID elType_7_1_rear_surround[] = {
    ID_SCE, ID_CPE, ID_CPE, ID_CPE, ID_LFE};
static const MP4_ELEMENT_ID elType_7_1_front_center[] = {ID_SCE, ID_CPE, ID_CPE,
                                                         ID_CPE, ID_LFE};

/**
 * The following arrays provide information on how many front, side and back
 * elements are assigned to the top or bottom plane for each mode that comprises
 * height information.
 */
static const PCE_HEIGHT_NUM heightNum_7_1_top_front = {{1, 0}, {0, 0}, {0, 0}};

/**
 * \brief Table contains all supported channel modes and according PCE
 configuration description.
 *
 * The mode identifier is followed by the number of front, side, back, and LFE
 elements.
 * These are followed by a pointer to the IDs of the consecutive elements
 (ID_SCE, ID_CPE, ID_LFE).
 *
 * For some modes (MODE_7_1_TOP_FRONT and MODE_22_2) additional height
 information is transmitted.
 * In this case the additional pointer provides information on how many front,
 side and back elements
 * are assigned to the top or bottom plane.The elements are arranged in the
 following order: normal height (front, side, back, LFE), top height (front,
 side, back), bottom height (front, side, back).
 *
 *
 * E.g. MODE_7_1_TOP_FRONT means:
 *                          - 3 elements are front channel elements.
 *                          - 0 elements are side channel elements.
 *                          - 1 element is back channel element.
 *                          - 1 element is an LFE channel element.
 *                          - the element order is ID_SCE, ID_CPE, ID_CPE,
 ID_LFE, ID_CPE.
 *                          - 1 of the front elements is in the top plane.
 *
 * This leads to the following mapping for the cconsecutive elements in the
 MODE_7_1_TOP_FRONT bitstream:
 *                          - ID_SCE -> normal height front,
                            - ID_CPE -> normal height front,
                            - ID_CPE -> normal height back,
                            - ID_LFE -> normal height LFE,
                            - ID_CPE -> top height front.
 */
static const CHANNEL_CONFIGURATION pceConfigTab[] = {
    {MODE_1,
     {1, 0, 0, 0, elType_1,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_2,
     {1, 0, 0, 0, elType_2,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_1_2,
     {2, 0, 0, 0, elType_1_2,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_1_2_1,
     {2, 0, 1, 0, elType_1_2_1,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_1_2_2,
     {2, 0, 1, 0, elType_1_2_2,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_1_2_2_1,
     {2, 0, 1, 1, elType_1_2_2_1,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_1_2_2_2_1,
     {3, 0, 1, 1, elType_1_2_2_2_1,
      NULL}}, /* don't transmit height information in this mode */

    {MODE_6_1,
     {2, 0, 2, 1, elType_6_1,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_7_1_BACK,
     {2, 0, 2, 1, elType_7_1_back,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_7_1_TOP_FRONT,
     {3, 0, 1, 1, elType_7_1_top_front, &heightNum_7_1_top_front}},

    {MODE_7_1_REAR_SURROUND,
     {2, 0, 2, 1, elType_7_1_rear_surround,
      NULL}}, /* don't transmit height information in this mode */
    {MODE_7_1_FRONT_CENTER,
     {3, 0, 1, 1, elType_7_1_front_center,
      NULL}} /* don't transmit height information in this mode */
};

/**
 * \brief  Get program config element description for existing channel mode.
 *
 * \param channel_mode          Current channel mode.
 *
 * \return
 *          - Pointer to PCE_CONFIGURATION entry, on success.
 *          - NULL, on failure.
 */
static const PCE_CONFIGURATION *getPceEntry(const CHANNEL_MODE channel_mode) {
  UINT i;
  const PCE_CONFIGURATION *pce_config = NULL;

  for (i = 0; i < (sizeof(pceConfigTab) / sizeof(CHANNEL_CONFIGURATION)); i++) {
    if (pceConfigTab[i].channel_mode == channel_mode) {
      pce_config = &pceConfigTab[i].pce_configuration;
      break;
    }
  }

  return pce_config;
}

int getChannelConfig(const CHANNEL_MODE channel_mode,
                     const UCHAR channel_config_zero) {
  INT chan_config = 0;

  if (channel_config_zero != 0) {
    chan_config = 0;
  } else {
    switch (channel_mode) {
      case MODE_1:
        chan_config = 1;
        break;
      case MODE_2:
        chan_config = 2;
        break;
      case MODE_1_2:
        chan_config = 3;
        break;
      case MODE_1_2_1:
        chan_config = 4;
        break;
      case MODE_1_2_2:
        chan_config = 5;
        break;
      case MODE_1_2_2_1:
        chan_config = 6;
        break;
      case MODE_1_2_2_2_1:
        chan_config = 7;
        break;
      case MODE_6_1:
        chan_config = 11;
        break;
      case MODE_7_1_BACK:
        chan_config = 12;
        break;
      case MODE_7_1_TOP_FRONT:
        chan_config = 14;
        break;
      default:
        chan_config = 0;
    }
  }

  return chan_config;
}

CHANNEL_MODE transportEnc_GetChannelMode(int noChannels) {
  CHANNEL_MODE chMode;

  if (noChannels <= 8 && noChannels > 0)
    chMode = (CHANNEL_MODE)(
        (noChannels == 8) ? 7
                          : noChannels); /* see : iso/mpeg4 v1 audio subpart1*/
  else
    chMode = MODE_UNKNOWN;

  return chMode;
}

int transportEnc_writePCE(HANDLE_FDK_BITSTREAM hBs, CHANNEL_MODE channelMode,
                          INT sampleRate, int instanceTagPCE, int profile,
                          int matrixMixdownA, int pseudoSurroundEnable,
                          UINT alignAnchor) {
  int sampleRateIndex, i;
  const PCE_CONFIGURATION *config = NULL;
  const MP4_ELEMENT_ID *pEl_list = NULL;
  UCHAR cpeCnt = 0, sceCnt = 0, lfeCnt = 0, frntCnt = 0, sdCnt = 0, bckCnt = 0,
        isCpe = 0, tag = 0, normalFrontEnd = 0, normalSideEnd = 0,
        normalBackEnd = 0, topFrontEnd = 0, topSideEnd = 0, topBackEnd = 0,
        bottomFrontEnd = 0, bottomSideEnd = 0;
#ifdef FDK_ASSERT_ENABLE
  UCHAR bottomBackEnd = 0;
#endif
  enum elementDepth { FRONT, SIDE, BACK } elDepth;

  sampleRateIndex = getSamplingRateIndex(sampleRate, 4);
  if (sampleRateIndex == 15) {
    return -1;
  }

  if ((config = getPceEntry(channelMode)) == NULL) {
    return -1;
  }

  FDK_ASSERT(config->num_front_channel_elements <= MAX_FRONT_ELEMENTS);
  FDK_ASSERT(config->num_side_channel_elements <= MAX_SIDE_ELEMENTS);
  FDK_ASSERT(config->num_back_channel_elements <= MAX_BACK_ELEMENTS);

  UCHAR frontIsCpe[MAX_FRONT_ELEMENTS] = {0},
        frontTag[MAX_FRONT_ELEMENTS] = {0}, sideIsCpe[MAX_SIDE_ELEMENTS] = {0},
        sideTag[MAX_SIDE_ELEMENTS] = {0}, backIsCpe[MAX_BACK_ELEMENTS] = {0},
        backTag[MAX_BACK_ELEMENTS] = {0};

  /* Write general information */

  FDKwriteBits(hBs, instanceTagPCE, 4);  /* Element instance tag */
  FDKwriteBits(hBs, profile, 2);         /* Object type */
  FDKwriteBits(hBs, sampleRateIndex, 4); /* Sample rate index*/

  FDKwriteBits(hBs, config->num_front_channel_elements,
               4); /* Front channel Elements */
  FDKwriteBits(hBs, config->num_side_channel_elements,
               4); /* No Side Channel Elements */
  FDKwriteBits(hBs, config->num_back_channel_elements,
               4); /* No Back channel Elements */
  FDKwriteBits(hBs, config->num_lfe_channel_elements,
               2); /* No Lfe channel elements */

  FDKwriteBits(hBs, 0, 3); /* No assoc data elements */
  FDKwriteBits(hBs, 0, 4); /* No valid cc elements */
  FDKwriteBits(hBs, 0, 1); /* Mono mixdown present */
  FDKwriteBits(hBs, 0, 1); /* Stereo mixdown present */

  if (matrixMixdownA != 0 &&
      ((channelMode == MODE_1_2_2) || (channelMode == MODE_1_2_2_1))) {
    FDKwriteBits(hBs, 1, 1); /* Matrix mixdown present */
    FDKwriteBits(hBs, (matrixMixdownA - 1) & 0x3, 2); /* matrix_mixdown_idx */
    FDKwriteBits(hBs, (pseudoSurroundEnable) ? 1 : 0,
                 1); /* pseudo_surround_enable */
  } else {
    FDKwriteBits(hBs, 0, 1); /* Matrix mixdown not present */
  }

  if (config->pHeight_num != NULL) {
    /* we have up to three different height levels, and in each height level we
     * may have front, side and back channels. We need to know where each
     * section ends to correctly count the tags */
    normalFrontEnd = config->num_front_channel_elements -
                     config->pHeight_num->num_front_height_channel_elements[0] -
                     config->pHeight_num->num_front_height_channel_elements[1];
    normalSideEnd = normalFrontEnd + config->num_side_channel_elements -
                    config->pHeight_num->num_side_height_channel_elements[0] -
                    config->pHeight_num->num_side_height_channel_elements[1];
    normalBackEnd = normalSideEnd + config->num_back_channel_elements -
                    config->pHeight_num->num_back_height_channel_elements[0] -
                    config->pHeight_num->num_back_height_channel_elements[1];

    topFrontEnd =
        normalBackEnd + config->num_lfe_channel_elements +
        config->pHeight_num->num_front_height_channel_elements[0]; /* only
                                                                      normal
                                                                      height
                                                                      LFEs
                                                                      assumed */
    topSideEnd =
        topFrontEnd + config->pHeight_num->num_side_height_channel_elements[0];
    topBackEnd =
        topSideEnd + config->pHeight_num->num_back_height_channel_elements[0];

    bottomFrontEnd =
        topBackEnd + config->pHeight_num->num_front_height_channel_elements[1];
    bottomSideEnd = bottomFrontEnd +
                    config->pHeight_num->num_side_height_channel_elements[1];
#ifdef FDK_ASSERT_ENABLE
    bottomBackEnd = bottomSideEnd +
                    config->pHeight_num->num_back_height_channel_elements[1];
#endif

  } else {
    /* we have only one height level, so we don't care about top or bottom */
    normalFrontEnd = config->num_front_channel_elements;
    normalSideEnd = normalFrontEnd + config->num_side_channel_elements;
    normalBackEnd = normalSideEnd + config->num_back_channel_elements;
  }

  /* assign cpe and tag information to either front, side or back channels */

  pEl_list = config->pEl_type;

  for (i = 0; i < config->num_front_channel_elements +
                      config->num_side_channel_elements +
                      config->num_back_channel_elements +
                      config->num_lfe_channel_elements;
       i++) {
    if (*pEl_list == ID_LFE) {
      pEl_list++;
      continue;
    }
    isCpe = (*pEl_list++ == ID_CPE) ? 1 : 0;
    tag = (isCpe) ? cpeCnt++ : sceCnt++;

    if (i < normalFrontEnd)
      elDepth = FRONT;
    else if (i < normalSideEnd)
      elDepth = SIDE;
    else if (i < normalBackEnd)
      elDepth = BACK;
    else if (i < topFrontEnd)
      elDepth = FRONT;
    else if (i < topSideEnd)
      elDepth = SIDE;
    else if (i < topBackEnd)
      elDepth = BACK;
    else if (i < bottomFrontEnd)
      elDepth = FRONT;
    else if (i < bottomSideEnd)
      elDepth = SIDE;
    else {
      elDepth = BACK;
      FDK_ASSERT(i < bottomBackEnd); /* won't fail if implementation of pce
                                        configuration table is correct */
    }

    switch (elDepth) {
      case FRONT:
        FDK_ASSERT(frntCnt < config->num_front_channel_elements);
        frontIsCpe[frntCnt] = isCpe;
        frontTag[frntCnt++] = tag;
        break;
      case SIDE:
        FDK_ASSERT(sdCnt < config->num_side_channel_elements);
        sideIsCpe[sdCnt] = isCpe;
        sideTag[sdCnt++] = tag;
        break;
      case BACK:
        FDK_ASSERT(bckCnt < config->num_back_channel_elements);
        backIsCpe[bckCnt] = isCpe;
        backTag[bckCnt++] = tag;
        break;
    }
  }

  /* Write front channel isCpe and tags */
  for (i = 0; i < config->num_front_channel_elements; i++) {
    FDKwriteBits(hBs, frontIsCpe[i], 1);
    FDKwriteBits(hBs, frontTag[i], 4);
  }
  /* Write side channel isCpe and tags */
  for (i = 0; i < config->num_side_channel_elements; i++) {
    FDKwriteBits(hBs, sideIsCpe[i], 1);
    FDKwriteBits(hBs, sideTag[i], 4);
  }
  /* Write back channel isCpe and tags */
  for (i = 0; i < config->num_back_channel_elements; i++) {
    FDKwriteBits(hBs, backIsCpe[i], 1);
    FDKwriteBits(hBs, backTag[i], 4);
  }
  /* Write LFE information */
  for (i = 0; i < config->num_lfe_channel_elements; i++) {
    FDKwriteBits(hBs, lfeCnt++, 4); /* LFE channel Instance Tag. */
  }

  /* - num_valid_cc_elements always 0.
     - num_assoc_data_elements always 0. */

  /* Byte alignment: relative to alignAnchor
       ADTS: align with respect to the first bit of the raw_data_block()
       ADIF: align with respect to the first bit of the header
       LATM: align with respect to the first bit of the ASC */
  FDKbyteAlign(hBs, alignAnchor); /* Alignment */

  /* Write comment information */

  if (config->pHeight_num != NULL) {
    /* embed height information in comment field */

    INT commentBytes =
        1 /* PCE_HEIGHT_EXT_SYNC */
        + ((((config->num_front_channel_elements +
              config->num_side_channel_elements +
              config->num_back_channel_elements)
             << 1) +
            7) >>
           3) /* 2 bit height info per element, round up to full bytes */
        + 1;  /* CRC */

    FDKwriteBits(hBs, commentBytes, 8); /* comment size. */

    FDK_CRCINFO crcInfo; /* CRC state info */
    INT crcReg;

    FDKcrcInit(&crcInfo, 0x07, 0xFF, 8);
    crcReg = FDKcrcStartReg(&crcInfo, hBs, 0);

    FDKwriteBits(hBs, PCE_HEIGHT_EXT_SYNC, 8); /* indicate height extension */

    /* front channel height information */
    for (i = 0;
         i < config->num_front_channel_elements -
                 config->pHeight_num->num_front_height_channel_elements[0] -
                 config->pHeight_num->num_front_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_NORMAL, 2);
    for (i = 0; i < config->pHeight_num->num_front_height_channel_elements[0];
         i++)
      FDKwriteBits(hBs, HEIGHT_TOP, 2);
    for (i = 0; i < config->pHeight_num->num_front_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_BOTTOM, 2);

    /* side channel height information */
    for (i = 0;
         i < config->num_side_channel_elements -
                 config->pHeight_num->num_side_height_channel_elements[0] -
                 config->pHeight_num->num_side_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_NORMAL, 2);
    for (i = 0; i < config->pHeight_num->num_side_height_channel_elements[0];
         i++)
      FDKwriteBits(hBs, HEIGHT_TOP, 2);
    for (i = 0; i < config->pHeight_num->num_side_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_BOTTOM, 2);

    /* back channel height information */
    for (i = 0;
         i < config->num_back_channel_elements -
                 config->pHeight_num->num_back_height_channel_elements[0] -
                 config->pHeight_num->num_back_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_NORMAL, 2);
    for (i = 0; i < config->pHeight_num->num_back_height_channel_elements[0];
         i++)
      FDKwriteBits(hBs, HEIGHT_TOP, 2);
    for (i = 0; i < config->pHeight_num->num_back_height_channel_elements[1];
         i++)
      FDKwriteBits(hBs, HEIGHT_BOTTOM, 2);

    FDKbyteAlign(hBs, alignAnchor); /* Alignment */

    FDKcrcEndReg(&crcInfo, hBs, crcReg);
    FDKwriteBits(hBs, FDKcrcGetCRC(&crcInfo), 8);

  } else {
    FDKwriteBits(hBs, 0,
                 8); /* Do no write any comment or height information. */
  }

  return 0;
}

int transportEnc_GetPCEBits(CHANNEL_MODE channelMode, int matrixMixdownA,
                            int bits) {
  const PCE_CONFIGURATION *config = NULL;

  if ((config = getPceEntry(channelMode)) == NULL) {
    return -1; /* unsupported channelmapping */
  }

  bits +=
      4 + 2 + 4; /* Element instance tag  + Object type + Sample rate index */
  bits += 4 + 4 + 4 + 2; /* No (front + side + back + lfe channel) elements */
  bits += 3 + 4;         /* No (assoc data + valid cc) elements */
  bits += 1 + 1 + 1;     /* Mono + Stereo + Matrix mixdown present */

  if (matrixMixdownA != 0 &&
      ((channelMode == MODE_1_2_2) || (channelMode == MODE_1_2_2_1))) {
    bits += 3; /* matrix_mixdown_idx + pseudo_surround_enable */
  }

  bits += (1 + 4) * (INT)config->num_front_channel_elements;
  bits += (1 + 4) * (INT)config->num_side_channel_elements;
  bits += (1 + 4) * (INT)config->num_back_channel_elements;
  bits += (4) * (INT)config->num_lfe_channel_elements;

  /* - num_valid_cc_elements always 0.
     - num_assoc_data_elements always 0. */

  if ((bits % 8) != 0) {
    bits += (8 - (bits % 8)); /* Alignment */
  }

  bits += 8; /* Comment field  bytes */

  if (config->pHeight_num != NULL) {
    /* Comment field (height extension) */

    bits +=
        8 /* PCE_HEIGHT_EXT_SYNC */
        +
        ((config->num_front_channel_elements +
          config->num_side_channel_elements + config->num_back_channel_elements)
         << 1) /* 2 bit height info per element */
        + 8;   /* CRC */

    if ((bits % 8) != 0) {
      bits += (8 - (bits % 8)); /* Alignment */
    }
  }

  return bits;
}

static void writeAot(HANDLE_FDK_BITSTREAM hBitstreamBuffer,
                     AUDIO_OBJECT_TYPE aot) {
  int tmp = (int)aot;

  if (tmp > 31) {
    FDKwriteBits(hBitstreamBuffer, AOT_ESCAPE, 5);
    FDKwriteBits(hBitstreamBuffer, tmp - 32, 6); /* AudioObjectType */
  } else {
    FDKwriteBits(hBitstreamBuffer, tmp, 5);
  }
}

static void writeSampleRate(HANDLE_FDK_BITSTREAM hBs, int sampleRate,
                            int nBits) {
  int srIdx = getSamplingRateIndex(sampleRate, nBits);

  FDKwriteBits(hBs, srIdx, nBits);
  if (srIdx == (1 << nBits) - 1) {
    FDKwriteBits(hBs, sampleRate, 24);
  }
}

static int transportEnc_writeGASpecificConfig(HANDLE_FDK_BITSTREAM asc,
                                              CODER_CONFIG *config, int extFlg,
                                              UINT alignAnchor) {
  int aot = config->aot;
  int samplesPerFrame = config->samplesPerFrame;

  /* start of GASpecificConfig according to ISO/IEC 14496-3 Subpart 4, 4.4.1 */
  FDKwriteBits(asc,
               ((samplesPerFrame == 960 || samplesPerFrame == 480) ? 1 : 0),
               1); /* frameLengthFlag: 1 for a 960/480 (I)MDCT, 0 for a 1024/512
                      (I)MDCT*/
  FDKwriteBits(asc, 0,
               1); /* dependsOnCoreCoder: Sampling Rate Coder Specific, see in
                      ISO/IEC 14496-3 Subpart 4, 4.4.1 */
  FDKwriteBits(asc, extFlg,
               1); /* Extension Flag: Shall be 1 for aot = 17,19,20,21,22,23 */

  /* Write PCE if channel config is not 1-7 */
  if (getChannelConfig(config->channelMode, config->channelConfigZero) == 0) {
    transportEnc_writePCE(asc, config->channelMode, config->samplingRate, 0, 1,
                          config->matrixMixdownA,
                          (config->flags & CC_PSEUDO_SURROUND) ? 1 : 0,
                          alignAnchor);
  }
  if ((aot == AOT_AAC_SCAL) || (aot == AOT_ER_AAC_SCAL)) {
    FDKwriteBits(asc, 0, 3); /* layerNr */
  }
  if (extFlg) {
    if (aot == AOT_ER_BSAC) {
      FDKwriteBits(asc, config->BSACnumOfSubFrame, 5); /* numOfSubFrame */
      FDKwriteBits(asc, config->BSAClayerLength, 11);  /* layer_length */
    }
    if ((aot == AOT_ER_AAC_LC) || (aot == AOT_ER_AAC_LTP) ||
        (aot == AOT_ER_AAC_SCAL) || (aot == AOT_ER_AAC_LD)) {
      FDKwriteBits(asc, (config->flags & CC_VCB11) ? 1 : 0,
                   1); /* aacSectionDataResillienceFlag */
      FDKwriteBits(asc, (config->flags & CC_RVLC) ? 1 : 0,
                   1); /* aacScaleFactorDataResillienceFlag */
      FDKwriteBits(asc, (config->flags & CC_HCR) ? 1 : 0,
                   1); /* aacSpectralDataResillienceFlag */
    }
    FDKwriteBits(asc, 0, 1); /* extensionFlag3: reserved. Shall be '0' */
  }
  return 0;
}

static int transportEnc_writeELDSpecificConfig(HANDLE_FDK_BITSTREAM hBs,
                                               CODER_CONFIG *config,
                                               int epConfig,
                                               CSTpCallBacks *cb) {
  UINT frameLengthFlag = 0;
  switch (config->samplesPerFrame) {
    case 512:
    case 256:
    case 128:
    case 64:
      frameLengthFlag = 0;
      break;
    case 480:
    case 240:
    case 160:
    case 120:
    case 60:
      frameLengthFlag = 1;
      break;
  }

  FDKwriteBits(hBs, frameLengthFlag, 1);

  FDKwriteBits(hBs, (config->flags & CC_VCB11) ? 1 : 0, 1);
  FDKwriteBits(hBs, (config->flags & CC_RVLC) ? 1 : 0, 1);
  FDKwriteBits(hBs, (config->flags & CC_HCR) ? 1 : 0, 1);

  FDKwriteBits(hBs, (config->flags & CC_SBR) ? 1 : 0, 1); /* SBR header flag */
  if ((config->flags & CC_SBR)) {
    FDKwriteBits(hBs, (config->samplingRate == config->extSamplingRate) ? 0 : 1,
                 1); /* Samplerate Flag */
    FDKwriteBits(hBs, (config->flags & CC_SBRCRC) ? 1 : 0, 1); /* SBR CRC flag*/

    if (cb->cbSbr != NULL) {
      const PCE_CONFIGURATION *pPce;
      int e, sbrElementIndex = 0;

      pPce = getPceEntry(config->channelMode);

      for (e = 0; e < pPce->num_front_channel_elements +
                          pPce->num_side_channel_elements +
                          pPce->num_back_channel_elements +
                          pPce->num_lfe_channel_elements;
           e++) {
        if ((pPce->pEl_type[e] == ID_SCE) || (pPce->pEl_type[e] == ID_CPE)) {
          cb->cbSbr(cb->cbSbrData, hBs, 0, 0, 0, config->aot, pPce->pEl_type[e],
                    sbrElementIndex, 0, 0, 0, NULL, 1);
          sbrElementIndex++;
        }
      }
    }
  }

  if ((config->flags & CC_SAC) && (cb->cbSsc != NULL)) {
    FDKwriteBits(hBs, ELDEXT_LDSAC, 4);

    const INT eldExtLen =
        (cb->cbSsc(cb->cbSscData, NULL, config->aot, config->extSamplingRate, 0,
                   0, 0, 0, 0, 0, NULL) +
         7) >>
        3;
    INT cnt = eldExtLen;

    if (cnt < 0xF) {
      FDKwriteBits(hBs, cnt, 4);
    } else {
      FDKwriteBits(hBs, 0xF, 4);
      cnt -= 0xF;

      if (cnt < 0xFF) {
        FDKwriteBits(hBs, cnt, 8);
      } else {
        FDKwriteBits(hBs, 0xFF, 8);
        cnt -= 0xFF;

        FDK_ASSERT(cnt <= 0xFFFF);
        FDKwriteBits(hBs, cnt, 16);
      }
    }

    cb->cbSsc(cb->cbSscData, hBs, config->aot, config->extSamplingRate, 0, 0, 0,
              0, 0, 0, NULL);
  }

  if (config->downscaleSamplingRate != 0 &&
      config->downscaleSamplingRate != config->extSamplingRate) {
    /* downscale active */

    /* eldExtLenDsc: Number of bytes for the ELD downscale extension (srIdx
       needs 1 byte
       + downscaleSamplingRate needs additional 3 bytes) */
    int eldExtLenDsc = 1;
    int downscaleSamplingRate = config->downscaleSamplingRate;
    FDKwriteBits(hBs, ELDEXT_DOWNSCALEINFO, 4); /* ELDEXT_DOWNSCALEINFO */

    if ((downscaleSamplingRate != 96000) && (downscaleSamplingRate != 88200) &&
        (downscaleSamplingRate != 64000) && (downscaleSamplingRate != 48000) &&
        (downscaleSamplingRate != 44100) && (downscaleSamplingRate != 32000) &&
        (downscaleSamplingRate != 24000) && (downscaleSamplingRate != 22050) &&
        (downscaleSamplingRate != 16000) && (downscaleSamplingRate != 12000) &&
        (downscaleSamplingRate != 11025) && (downscaleSamplingRate != 8000) &&
        (downscaleSamplingRate != 7350)) {
      eldExtLenDsc = 4; /* length extends to 4 if downscaleSamplingRate's value
                           is not one of the listed values */
    }

    FDKwriteBits(hBs, eldExtLenDsc, 4);
    writeSampleRate(hBs, downscaleSamplingRate, 4);
    FDKwriteBits(hBs, 0x0, 4); /* fill_nibble */
  }

  FDKwriteBits(hBs, ELDEXT_TERM, 4); /* ELDEXT_TERM */

  return 0;
}

static int transportEnc_writeUsacSpecificConfig(HANDLE_FDK_BITSTREAM hBs,
                                                int extFlag, CODER_CONFIG *cc,
                                                CSTpCallBacks *cb) {
  FDK_BITSTREAM usacConf;
  int usacConfigBits = cc->rawConfigBits;

  if ((usacConfigBits <= 0) ||
      ((usacConfigBits + 7) / 8 > (int)sizeof(cc->rawConfig))) {
    return TRANSPORTENC_UNSUPPORTED_FORMAT;
  }
  FDKinitBitStream(&usacConf, cc->rawConfig, BUFSIZE_DUMMY_VALUE,
                   usacConfigBits, BS_READER);

  for (; usacConfigBits > 0; usacConfigBits--) {
    UINT tmp = FDKreadBit(&usacConf);
    FDKwriteBits(hBs, tmp, 1);
  }
  FDKsyncCache(hBs);

  return TRANSPORTENC_OK;
}

int transportEnc_writeASC(HANDLE_FDK_BITSTREAM asc, CODER_CONFIG *config,
                          CSTpCallBacks *cb) {
  UINT extFlag = 0;
  int err;
  int epConfig = 0;

  /* Required for the PCE. */
  UINT alignAnchor = FDKgetValidBits(asc);

  /* Extension Flag: Shall be 1 for aot = 17,19,20,21,22,23,39 */
  switch (config->aot) {
    case AOT_ER_AAC_LC:
    case AOT_ER_AAC_LTP:
    case AOT_ER_AAC_SCAL:
    case AOT_ER_TWIN_VQ:
    case AOT_ER_BSAC:
    case AOT_ER_AAC_LD:
    case AOT_ER_AAC_ELD:
    case AOT_USAC:
      extFlag = 1;
      break;
    default:
      break;
  }

  if (config->sbrSignaling == SIG_EXPLICIT_HIERARCHICAL && config->sbrPresent)
    writeAot(asc, config->extAOT);
  else
    writeAot(asc, config->aot);

  /* In case of USAC it is the output not the core sampling rate */
  writeSampleRate(asc, config->samplingRate, 4);

  /* Try to guess a reasonable channel mode if not given */
  if (config->channelMode == MODE_INVALID) {
    config->channelMode = transportEnc_GetChannelMode(config->noChannels);
    if (config->channelMode == MODE_INVALID) return -1;
  }

  FDKwriteBits(
      asc, getChannelConfig(config->channelMode, config->channelConfigZero), 4);

  if (config->sbrSignaling == SIG_EXPLICIT_HIERARCHICAL && config->sbrPresent) {
    writeSampleRate(asc, config->extSamplingRate, 4);
    writeAot(asc, config->aot);
  }

  switch (config->aot) {
    case AOT_AAC_MAIN:
    case AOT_AAC_LC:
    case AOT_AAC_SSR:
    case AOT_AAC_LTP:
    case AOT_AAC_SCAL:
    case AOT_TWIN_VQ:
    case AOT_ER_AAC_LC:
    case AOT_ER_AAC_LTP:
    case AOT_ER_AAC_SCAL:
    case AOT_ER_TWIN_VQ:
    case AOT_ER_BSAC:
    case AOT_ER_AAC_LD:
      err =
          transportEnc_writeGASpecificConfig(asc, config, extFlag, alignAnchor);
      if (err) return err;
      break;
    case AOT_ER_AAC_ELD:
      err = transportEnc_writeELDSpecificConfig(asc, config, epConfig, cb);
      if (err) return err;
      break;
    case AOT_USAC:
      err = transportEnc_writeUsacSpecificConfig(asc, extFlag, config, cb);
      if (err) {
        return err;
      }
      break;
    default:
      return -1;
  }

  switch (config->aot) {
    case AOT_ER_AAC_LC:
    case AOT_ER_AAC_LTP:
    case AOT_ER_AAC_SCAL:
    case AOT_ER_TWIN_VQ:
    case AOT_ER_BSAC:
    case AOT_ER_AAC_LD:
    case AOT_ER_CELP:
    case AOT_ER_HVXC:
    case AOT_ER_HILN:
    case AOT_ER_PARA:
    case AOT_ER_AAC_ELD:
      FDKwriteBits(asc, 0, 2); /* epconfig 0 */
      break;
    default:
      break;
  }

  /* backward compatible explicit signaling of extension AOT */
  if (config->sbrSignaling == SIG_EXPLICIT_BW_COMPATIBLE) {
    TP_ASC_EXTENSION_ID ascExtId = ASCEXT_UNKOWN;

    if (config->sbrPresent) {
      ascExtId = ASCEXT_SBR;
      FDKwriteBits(asc, ascExtId, 11);
      writeAot(asc, config->extAOT);
      FDKwriteBits(asc, 1, 1); /* sbrPresentFlag=1 */
      writeSampleRate(asc, config->extSamplingRate, 4);
      if (config->psPresent) {
        ascExtId = ASCEXT_PS;
        FDKwriteBits(asc, ascExtId, 11);
        FDKwriteBits(asc, 1, 1); /* psPresentFlag=1 */
      }
    }
  }

  /* Make sure all bits are sync'ed */
  FDKsyncCache(asc);

  return 0;
}
