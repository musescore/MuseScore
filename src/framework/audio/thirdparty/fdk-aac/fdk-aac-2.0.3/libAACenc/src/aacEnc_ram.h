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

   Author(s):   M. Lohwasser, M. Gayer

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Memory layout
  \author Markus Lohwasser
*/

#ifndef AACENC_RAM_H
#define AACENC_RAM_H

#include "common_fix.h"

#include "aacenc.h"
#include "psy_data.h"
#include "interface.h"
#include "psy_main.h"
#include "bitenc.h"
#include "bit_cnt.h"
#include "psy_const.h"

#define OUTPUTBUFFER_SIZE                                                 \
  (8192) /*!< Output buffer size has to be at least 6144 bits per channel \
            (768 bytes). FDK bitbuffer implementation expects buffer of   \
            size 2^n. */

/*
  Moved AAC_ENC struct definition from aac_enc.cpp into aacEnc_ram.h to get size
  and respective static memory in aacEnc_ram.cpp. aac_enc.h is the outward
  visible header file and putting the struct into would cause necessity of
  additional visible header files outside library.
*/

/* define hBitstream size: max AAC framelength is 6144 bits/channel */
/*#define BUFFER_BITSTR_SIZE ((6400*(8)/bbWordSize)    +((bbWordSize - 1) /
 * bbWordSize))*/

struct AAC_ENC {
  AACENC_CONFIG *config;

  INT ancillaryBitsPerFrame; /* ancillary bits per frame calculated from
                                ancillary rate */

  CHANNEL_MAPPING channelMapping;

  QC_STATE *qcKernel;
  QC_OUT *qcOut[(1)];

  PSY_OUT *psyOut[(1)];
  PSY_INTERNAL *psyKernel;

  /* lifetime vars */

  CHANNEL_MODE encoderMode;
  INT bandwidth90dB;
  AACENC_BITRATE_MODE bitrateMode;

  INT dontWriteAdif; /* use: write ADIF header only before 1st frame */

  FIXP_DBL *dynamic_RAM;

  INT maxChannels; /* used while allocation */
  INT maxElements;
  INT maxFrames;

  AUDIO_OBJECT_TYPE aot; /* AOT to be used while encoding.  */
};

#define maxSize(a, b) (((a) > (b)) ? (a) : (b))

#define BIT_LOOK_UP_SIZE \
  (sizeof(INT) * (MAX_SFB_LONG * (CODE_BOOK_ESC_NDX + 1)))
#define MERGE_GAIN_LOOK_UP_SIZE (sizeof(INT) * MAX_SFB_LONG)

/* Size of AhFlag buffer in function FDKaacEnc_adaptThresholdsToPe() */
#define ADJ_THR_AH_FLAG_SIZE (sizeof(UCHAR) * ((8)) * (2) * MAX_GROUPED_SFB)
/* Size of ThrExp buffer in function FDKaacEnc_adaptThresholdsToPe() */
#define ADJ_THR_THR_EXP_SIZE (sizeof(FIXP_DBL) * ((8)) * (2) * MAX_GROUPED_SFB)
/* Size of sfbNActiveLinesLdData buffer in function FDKaacEnc_correctThresh() */
#define ADJ_THR_ACT_LIN_LD_DATA_SIZE \
  (sizeof(FIXP_DBL) * ((8)) * (2) * MAX_GROUPED_SFB)
/* Total amount of dynamic buffer needed in adjust thresholds functionality */
#define ADJ_THR_SIZE \
  (ADJ_THR_AH_FLAG_SIZE + ADJ_THR_THR_EXP_SIZE + ADJ_THR_ACT_LIN_LD_DATA_SIZE)

/* Dynamic RAM - Allocation */
/*
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
 |   P_BUF_0   |               P_BUF_1               |
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
 |  QC_OUT_CH  |               PSY_DYN               |
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
 |             |       BitLookUp+MergeGainLookUp     |
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
 |             | AH_FLAG | THR_EXP | ACT_LIN_LD_DATA |
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
 |             |       Bitstream output buffer       |
 +++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#define BUF_SIZE_0 (ALIGN_SIZE(sizeof(QC_OUT_CHANNEL)) * (8))
#define BUF_SIZE_1                                                           \
  (ALIGN_SIZE(maxSize(maxSize(sizeof(PSY_DYNAMIC),                           \
                              (BIT_LOOK_UP_SIZE + MERGE_GAIN_LOOK_UP_SIZE)), \
                      ADJ_THR_SIZE)))

#define P_BUF_0 (0)
#define P_BUF_1 (P_BUF_0 + BUF_SIZE_0)

#define AAC_ENC_DYN_RAM_SIZE (BUF_SIZE_0 + BUF_SIZE_1)

H_ALLOC_MEM(AACdynamic_RAM, FIXP_DBL)
/*
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
END - Dynamic RAM - Allocation */

/*
  See further Memory Allocation details in aacEnc_ram.cpp
*/
H_ALLOC_MEM(Ram_aacEnc_AacEncoder, AAC_ENC)

H_ALLOC_MEM(Ram_aacEnc_PsyElement, PSY_ELEMENT)

H_ALLOC_MEM(Ram_aacEnc_PsyInternal, PSY_INTERNAL)
H_ALLOC_MEM(Ram_aacEnc_PsyStatic, PSY_STATIC)
H_ALLOC_MEM(Ram_aacEnc_PsyInputBuffer, INT_PCM)

PSY_DYNAMIC *GetRam_aacEnc_PsyDynamic(int n, UCHAR *dynamic_RAM);

H_ALLOC_MEM(Ram_aacEnc_PsyOutChannel, PSY_OUT_CHANNEL)

H_ALLOC_MEM(Ram_aacEnc_PsyOut, PSY_OUT)
H_ALLOC_MEM(Ram_aacEnc_PsyOutElements, PSY_OUT_ELEMENT)

H_ALLOC_MEM(Ram_aacEnc_QCstate, QC_STATE)
H_ALLOC_MEM(Ram_aacEnc_AdjustThreshold, ADJ_THR_STATE)

H_ALLOC_MEM(Ram_aacEnc_AdjThrStateElement, ATS_ELEMENT)
H_ALLOC_MEM(Ram_aacEnc_ElementBits, ELEMENT_BITS)
H_ALLOC_MEM(Ram_aacEnc_BitCntrState, BITCNTR_STATE)

INT *GetRam_aacEnc_BitLookUp(int n, UCHAR *dynamic_RAM);
INT *GetRam_aacEnc_MergeGainLookUp(int n, UCHAR *dynamic_RAM);
QC_OUT_CHANNEL *GetRam_aacEnc_QCchannel(int n, UCHAR *dynamic_RAM);

H_ALLOC_MEM(Ram_aacEnc_QCout, QC_OUT)
H_ALLOC_MEM(Ram_aacEnc_QCelement, QC_OUT_ELEMENT)

#endif /* #ifndef AACENC_RAM_H */
