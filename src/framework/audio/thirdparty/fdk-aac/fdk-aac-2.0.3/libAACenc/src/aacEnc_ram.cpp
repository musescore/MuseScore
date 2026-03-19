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

#include "aacEnc_ram.h"

C_AALLOC_MEM(AACdynamic_RAM, FIXP_DBL, AAC_ENC_DYN_RAM_SIZE / sizeof(FIXP_DBL))

/*
  Static memory areas, must not be overwritten in other sections of the decoder
  !
*/

/*
 The structure AacEncoder contains all Encoder structures.
*/

C_ALLOC_MEM(Ram_aacEnc_AacEncoder, struct AAC_ENC, 1)

/*
   The structure PSY_INTERNAl contains all psych configuration and data pointer.
   * PsyStatic holds last and current Psych data.
   * PsyInputBuffer contains time input. Signal is needed at the beginning of
   Psych. Memory can be reused after signal is in time domain.
   * PsyData contains spectral, nrg and threshold information. Necessary data
   are copied into PsyOut, so memory is available after leaving psych.
   * TnsData, ChaosMeasure, PnsData are temporarily necessary, e.g. use memory
   from PsyInputBuffer.
*/

C_ALLOC_MEM2(Ram_aacEnc_PsyElement, PSY_ELEMENT, 1, ((8)))

C_ALLOC_MEM(Ram_aacEnc_PsyInternal, PSY_INTERNAL, 1)
C_ALLOC_MEM2(Ram_aacEnc_PsyStatic, PSY_STATIC, 1, (8))

C_ALLOC_MEM2(Ram_aacEnc_PsyInputBuffer, INT_PCM, MAX_INPUT_BUFFER_SIZE, (8))

PSY_DYNAMIC *GetRam_aacEnc_PsyDynamic(int n, UCHAR *dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + P_BUF_1 + n*sizeof(PSY_DYNAMIC)) is sufficiently aligned, so
   * the cast is safe */
  return reinterpret_cast<PSY_DYNAMIC *>(reinterpret_cast<void *>(
      dynamic_RAM + P_BUF_1 + n * sizeof(PSY_DYNAMIC)));
}

/*
   The structure PSY_OUT holds all psychoaccoustic data needed
   in quantization module
*/
C_ALLOC_MEM2(Ram_aacEnc_PsyOut, PSY_OUT, 1, (1))

C_ALLOC_MEM2(Ram_aacEnc_PsyOutElements, PSY_OUT_ELEMENT, 1, (1) * ((8)))
C_ALLOC_MEM2(Ram_aacEnc_PsyOutChannel, PSY_OUT_CHANNEL, 1, (1) * (8))

/*
   The structure QC_STATE contains preinitialized settings and quantizer
   structures.
   * AdjustThreshold structure contains element-wise settings.
   * ElementBits contains elemnt-wise bit consumption settings.
   * When CRC is active, lookup table is necessary for fast crc calculation.
   * Bitcounter contains buffer to find optimal codebooks and minimal bit
   consumption. Values are temporarily, so dynamic memory can be used.
*/

C_ALLOC_MEM(Ram_aacEnc_QCstate, QC_STATE, 1)
C_ALLOC_MEM(Ram_aacEnc_AdjustThreshold, ADJ_THR_STATE, 1)

C_ALLOC_MEM2(Ram_aacEnc_AdjThrStateElement, ATS_ELEMENT, 1, ((8)))
C_ALLOC_MEM2(Ram_aacEnc_ElementBits, ELEMENT_BITS, 1, ((8)))
C_ALLOC_MEM(Ram_aacEnc_BitCntrState, struct BITCNTR_STATE, 1)

INT *GetRam_aacEnc_BitLookUp(int n, UCHAR *dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + P_BUF_1) is sufficiently aligned, so the cast is safe */
  return reinterpret_cast<INT *>(
      reinterpret_cast<void *>(dynamic_RAM + P_BUF_1));
}
INT *GetRam_aacEnc_MergeGainLookUp(int n, UCHAR *dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + P_BUF_1 + sizeof(INT)*(MAX_SFB_LONG*(CODE_BOOK_ESC_NDX+1)))
   * is sufficiently aligned, so the cast is safe */
  return reinterpret_cast<INT *>(reinterpret_cast<void *>(
      dynamic_RAM + P_BUF_1 +
      sizeof(INT) * (MAX_SFB_LONG * (CODE_BOOK_ESC_NDX + 1))));
}

/*
   The structure QC_OUT contains settings and structures holding all necessary
   information needed in bitstreamwriter.
*/

C_ALLOC_MEM2(Ram_aacEnc_QCout, QC_OUT, 1, (1))
C_ALLOC_MEM2(Ram_aacEnc_QCelement, QC_OUT_ELEMENT, 1, (1) * ((8)))
QC_OUT_CHANNEL *GetRam_aacEnc_QCchannel(int n, UCHAR *dynamic_RAM) {
  FDK_ASSERT(dynamic_RAM != 0);
  /* The reinterpret_cast is used to suppress a compiler warning. We know that
   * (dynamic_RAM + P_BUF_0 + n*sizeof(QC_OUT_CHANNEL)) is sufficiently aligned,
   * so the cast is safe */
  return reinterpret_cast<QC_OUT_CHANNEL *>(reinterpret_cast<void *>(
      dynamic_RAM + P_BUF_0 + n * ALIGN_SIZE(sizeof(QC_OUT_CHANNEL))));
}
