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

   Author(s):   Josef Hoepfl

   Description: ADTS interface

*******************************************************************************/

#include "tpdec_adts.h"

#include "FDK_bitstream.h"

void adtsRead_CrcInit(
    HANDLE_ADTS pAdts) /*!< pointer to adts crc info stucture */
{
  FDKcrcInit(&pAdts->crcInfo, 0x8005, 0xFFFF, 16);
}

int adtsRead_CrcStartReg(
    HANDLE_ADTS pAdts,        /*!< pointer to adts stucture */
    HANDLE_FDK_BITSTREAM hBs, /*!< handle to current bit buffer structure */
    int mBits                 /*!< number of bits in crc region */
) {
  if (pAdts->bs.protection_absent) {
    return 0;
  }

  return (FDKcrcStartReg(&pAdts->crcInfo, hBs, mBits));
}

void adtsRead_CrcEndReg(
    HANDLE_ADTS pAdts,        /*!< pointer to adts crc info stucture */
    HANDLE_FDK_BITSTREAM hBs, /*!< handle to current bit buffer structure */
    int reg                   /*!< crc region */
) {
  if (pAdts->bs.protection_absent == 0) {
    FDKcrcEndReg(&pAdts->crcInfo, hBs, reg);
  }
}

TRANSPORTDEC_ERROR adtsRead_CrcCheck(HANDLE_ADTS pAdts) {
  TRANSPORTDEC_ERROR ErrorStatus = TRANSPORTDEC_OK;
  USHORT crc;

  if (pAdts->bs.protection_absent) return TRANSPORTDEC_OK;

  crc = FDKcrcGetCRC(&pAdts->crcInfo);
  if (crc != pAdts->crcReadValue) {
    return (TRANSPORTDEC_CRC_ERROR);
  }

  return (ErrorStatus);
}

#define Adts_Length_SyncWord 12
#define Adts_Length_Id 1
#define Adts_Length_Layer 2
#define Adts_Length_ProtectionAbsent 1
#define Adts_Length_Profile 2
#define Adts_Length_SamplingFrequencyIndex 4
#define Adts_Length_PrivateBit 1
#define Adts_Length_ChannelConfiguration 3
#define Adts_Length_OriginalCopy 1
#define Adts_Length_Home 1
#define Adts_Length_CopyrightIdentificationBit 1
#define Adts_Length_CopyrightIdentificationStart 1
#define Adts_Length_FrameLength 13
#define Adts_Length_BufferFullness 11
#define Adts_Length_NumberOfRawDataBlocksInFrame 2
#define Adts_Length_CrcCheck 16

TRANSPORTDEC_ERROR adtsRead_DecodeHeader(HANDLE_ADTS pAdts,
                                         CSAudioSpecificConfig *pAsc,
                                         HANDLE_FDK_BITSTREAM hBs,
                                         const INT ignoreBufferFullness) {
  INT crcReg;

  INT valBits;
  INT cmp_buffer_fullness;
  int i, adtsHeaderLength;

  STRUCT_ADTS_BS bs;

  CProgramConfig oldPce;
  /* Store the old PCE temporarily. Maybe we'll need it later if we
     have channelConfig=0 and no PCE in this frame. */
  FDKmemcpy(&oldPce, &pAsc->m_progrConfigElement, sizeof(CProgramConfig));

  valBits = FDKgetValidBits(hBs) + ADTS_SYNCLENGTH;

  if (valBits < ADTS_HEADERLENGTH) {
    return TRANSPORTDEC_NOT_ENOUGH_BITS;
  }

  /* adts_fixed_header */
  bs.mpeg_id = FDKreadBits(hBs, Adts_Length_Id);
  bs.layer = FDKreadBits(hBs, Adts_Length_Layer);
  bs.protection_absent = FDKreadBits(hBs, Adts_Length_ProtectionAbsent);
  bs.profile = FDKreadBits(hBs, Adts_Length_Profile);
  bs.sample_freq_index = FDKreadBits(hBs, Adts_Length_SamplingFrequencyIndex);
  bs.private_bit = FDKreadBits(hBs, Adts_Length_PrivateBit);
  bs.channel_config = FDKreadBits(hBs, Adts_Length_ChannelConfiguration);
  bs.original = FDKreadBits(hBs, Adts_Length_OriginalCopy);
  bs.home = FDKreadBits(hBs, Adts_Length_Home);

  /* adts_variable_header */
  bs.copyright_id = FDKreadBits(hBs, Adts_Length_CopyrightIdentificationBit);
  bs.copyright_start =
      FDKreadBits(hBs, Adts_Length_CopyrightIdentificationStart);
  bs.frame_length = FDKreadBits(hBs, Adts_Length_FrameLength);
  bs.adts_fullness = FDKreadBits(hBs, Adts_Length_BufferFullness);
  bs.num_raw_blocks =
      FDKreadBits(hBs, Adts_Length_NumberOfRawDataBlocksInFrame);
  bs.num_pce_bits = 0;

  adtsHeaderLength = ADTS_HEADERLENGTH;

  if (valBits < bs.frame_length * 8) {
    goto bail;
  }

  FDKcrcReset(&pAdts->crcInfo);
  if (!bs.protection_absent) {
    FDKpushBack(hBs, 56); /* complete fixed and variable header! */
    crcReg = FDKcrcStartReg(&pAdts->crcInfo, hBs, 0);
    FDKpushFor(hBs, 56);
  }

  if (!bs.protection_absent && bs.num_raw_blocks > 0) {
    if ((INT)FDKgetValidBits(hBs) < bs.num_raw_blocks * 16) {
      goto bail;
    }
    for (i = 0; i < bs.num_raw_blocks; i++) {
      pAdts->rawDataBlockDist[i] = (USHORT)FDKreadBits(hBs, 16);
      adtsHeaderLength += 16;
    }
    /* Change raw data blocks to delta values */
    pAdts->rawDataBlockDist[bs.num_raw_blocks] =
        bs.frame_length - 7 - bs.num_raw_blocks * 2 - 2;
    for (i = bs.num_raw_blocks; i > 0; i--) {
      pAdts->rawDataBlockDist[i] -= pAdts->rawDataBlockDist[i - 1];
    }
  }

  /* adts_error_check */
  if (!bs.protection_absent) {
    USHORT crc_check;

    FDKcrcEndReg(&pAdts->crcInfo, hBs, crcReg);

    if ((INT)FDKgetValidBits(hBs) < Adts_Length_CrcCheck) {
      goto bail;
    }

    crc_check = FDKreadBits(hBs, Adts_Length_CrcCheck);
    adtsHeaderLength += Adts_Length_CrcCheck;

    pAdts->crcReadValue = crc_check;
    /* Check header CRC in case of multiple raw data blocks */
    if (bs.num_raw_blocks > 0) {
      if (pAdts->crcReadValue != FDKcrcGetCRC(&pAdts->crcInfo)) {
        return TRANSPORTDEC_CRC_ERROR;
      }
      /* Reset CRC for the upcoming raw_data_block() */
      FDKcrcReset(&pAdts->crcInfo);
    }
  }

  /* check if valid header */
  if ((bs.layer != 0) ||            // we only support MPEG ADTS
      (bs.sample_freq_index >= 13)  // we only support 96kHz - 7350kHz
  ) {
    FDKpushFor(hBs, bs.frame_length * 8);  // try again one frame later
    return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  }

  /* special treatment of id-bit */
  if ((bs.mpeg_id == 0) && (pAdts->decoderCanDoMpeg4 == 0)) {
    /* MPEG-2 decoder cannot play MPEG-4 bitstreams */

    FDKpushFor(hBs, bs.frame_length * 8);  // try again one frame later
    return TRANSPORTDEC_UNSUPPORTED_FORMAT;
  }

  if (!ignoreBufferFullness) {
    cmp_buffer_fullness =
        bs.frame_length * 8 +
        bs.adts_fullness * 32 * getNumberOfEffectiveChannels(bs.channel_config);

    /* Evaluate buffer fullness */
    if (bs.adts_fullness != 0x7FF) {
      if (pAdts->BufferFullnesStartFlag) {
        if (valBits < cmp_buffer_fullness) {
          /* Condition for start of decoding is not fulfilled */

          /* The current frame will not be decoded */
          FDKpushBack(hBs, adtsHeaderLength);

          if ((cmp_buffer_fullness + adtsHeaderLength) >
              (((8192 * 4) << 3) - 7)) {
            return TRANSPORTDEC_SYNC_ERROR;
          } else {
            return TRANSPORTDEC_NOT_ENOUGH_BITS;
          }
        } else {
          pAdts->BufferFullnesStartFlag = 0;
        }
      }
    }
  }

  /* Get info from ADTS header */
  AudioSpecificConfig_Init(pAsc);
  pAsc->m_aot = (AUDIO_OBJECT_TYPE)(bs.profile + 1);
  pAsc->m_samplingFrequencyIndex = bs.sample_freq_index;
  pAsc->m_samplingFrequency = SamplingRateTable[bs.sample_freq_index];
  pAsc->m_channelConfiguration = bs.channel_config;
  pAsc->m_samplesPerFrame = 1024;

  if (bs.channel_config == 0) {
    int pceBits = 0;
    UINT alignAnchor = FDKgetValidBits(hBs);
    CProgramConfig tmpPce;

    if (FDKreadBits(hBs, 3) == ID_PCE) {
      /* Got luck! Parse the PCE */
      crcReg = adtsRead_CrcStartReg(pAdts, hBs, 0);

      CProgramConfig_Init(&tmpPce);
      CProgramConfig_Read(&tmpPce, hBs, alignAnchor);

      if (CProgramConfig_IsValid(&tmpPce)) {
        if (CProgramConfig_IsValid(&oldPce)) {
          /* Compare the new and the old PCE (tags ignored) */
          switch (CProgramConfig_Compare(&tmpPce, &oldPce)) {
            case 0: /* Nothing to do because PCE matches the old one exactly. */
            case 1: /* Channel configuration not changed. Just new metadata. */
              FDKmemcpy(&pAsc->m_progrConfigElement, &tmpPce,
                        sizeof(CProgramConfig));
              break;
            case 2:  /* The number of channels are identical but not the config
                      */
            case -1: /* The channel configuration is completely different */
            default:
              FDKmemcpy(&pAsc->m_progrConfigElement, &oldPce,
                        sizeof(CProgramConfig));
              FDKpushBack(hBs, adtsHeaderLength);
              return TRANSPORTDEC_PARSE_ERROR;
          }
        } else {
          FDKmemcpy(&pAsc->m_progrConfigElement, &tmpPce,
                    sizeof(CProgramConfig));
        }
      } else {
        if (CProgramConfig_IsValid(&oldPce)) {
          FDKmemcpy(&pAsc->m_progrConfigElement, &oldPce,
                    sizeof(CProgramConfig));
        } else {
          FDKpushBack(hBs, adtsHeaderLength);
          return TRANSPORTDEC_PARSE_ERROR;
        }
      }

      adtsRead_CrcEndReg(pAdts, hBs, crcReg);
      pceBits = (INT)alignAnchor - (INT)FDKgetValidBits(hBs);
      adtsHeaderLength += pceBits;

      if (pceBits > (INT)alignAnchor) {
        goto bail;
      }

      /* store the number of PCE bits */
      bs.num_pce_bits = pceBits;
    } else {
      /* No PCE in this frame! Push back the ID tag bits. */
      FDKpushBack(hBs, 3);

      /* Encoders do not have to write a PCE in each frame.
         So if we already have a valid PCE we have to use it. */
      if (oldPce.isValid &&
          (bs.sample_freq_index ==
           pAdts->bs.sample_freq_index) /* we could compare the complete fixed
                                           header (bytes) here! */
          && (bs.channel_config == pAdts->bs.channel_config) /* == 0 */
          &&
          (bs.mpeg_id ==
           pAdts->bs.mpeg_id)) { /* Restore previous PCE which is still valid */
        FDKmemcpy(&pAsc->m_progrConfigElement, &oldPce, sizeof(CProgramConfig));
      } else if (bs.mpeg_id == 0) {
        /* If not it seems that we have a implicit channel configuration.
           This mode is not allowed in the context of ISO/IEC 14496-3.
           Skip this frame and try the next one. */
        FDKpushFor(hBs, (bs.frame_length << 3) - adtsHeaderLength - 3);
        return TRANSPORTDEC_UNSUPPORTED_FORMAT;
      }
      /* else {
         ISO/IEC 13818-7 implicit channel mapping is allowed.
         So just open the box of chocolates to see what we got.
      } */
    }
  }

  /* Copy bit stream data struct to persistent memory now, once we passed all
   * sanity checks above. */
  FDKmemcpy(&pAdts->bs, &bs, sizeof(STRUCT_ADTS_BS));

  return TRANSPORTDEC_OK;

bail:
  FDKpushBack(hBs, adtsHeaderLength);
  return TRANSPORTDEC_NOT_ENOUGH_BITS;
}

int adtsRead_GetRawDataBlockLength(HANDLE_ADTS pAdts, INT blockNum) {
  int length;

  if (pAdts->bs.num_raw_blocks == 0) {
    length =
        (pAdts->bs.frame_length - 7)
        << 3; /* aac_frame_length subtracted by the header size (7 bytes). */
    if (pAdts->bs.protection_absent == 0)
      length -= 16; /* substract 16 bit CRC */
  } else {
    if (pAdts->bs.protection_absent) {
      length = -1; /* raw data block length is unknown */
    } else {
      if (blockNum < 0 || blockNum > 3) {
        length = -1;
      } else {
        length = (pAdts->rawDataBlockDist[blockNum] << 3) - 16;
      }
    }
  }
  if (blockNum == 0 && length > 0) {
    length -= pAdts->bs.num_pce_bits;
  }
  return length;
}
