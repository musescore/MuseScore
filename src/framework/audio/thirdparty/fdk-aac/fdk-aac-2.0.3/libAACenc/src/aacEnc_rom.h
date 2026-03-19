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

/**************************** AAC encoder library ******************************

   Author(s):   M. Lohwasser, M. Gayer

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Memory layout
  \author Markus Lohwasser
*/

#ifndef AACENC_ROM_H
#define AACENC_ROM_H

#include "common_fix.h"

#include "psy_const.h"
#include "psy_configuration.h"
#include "FDK_tools_rom.h"
#include "FDK_lpc.h"

/*
  Huffman Tables
*/
extern const ULONG FDKaacEnc_huff_ltab1_2[3][3][3][3];
extern const ULONG FDKaacEnc_huff_ltab3_4[3][3][3][3];
extern const ULONG FDKaacEnc_huff_ltab5_6[9][9];
extern const ULONG FDKaacEnc_huff_ltab7_8[8][8];
extern const ULONG FDKaacEnc_huff_ltab9_10[13][13];
extern const UCHAR FDKaacEnc_huff_ltab11[17][17];
extern const UCHAR FDKaacEnc_huff_ltabscf[121];
extern const USHORT FDKaacEnc_huff_ctab1[3][3][3][3];
extern const USHORT FDKaacEnc_huff_ctab2[3][3][3][3];
extern const USHORT FDKaacEnc_huff_ctab3[3][3][3][3];
extern const USHORT FDKaacEnc_huff_ctab4[3][3][3][3];
extern const USHORT FDKaacEnc_huff_ctab5[9][9];
extern const USHORT FDKaacEnc_huff_ctab6[9][9];
extern const USHORT FDKaacEnc_huff_ctab7[8][8];
extern const USHORT FDKaacEnc_huff_ctab8[8][8];
extern const USHORT FDKaacEnc_huff_ctab9[13][13];
extern const USHORT FDKaacEnc_huff_ctab10[13][13];
extern const USHORT FDKaacEnc_huff_ctab11[21][17];
extern const ULONG FDKaacEnc_huff_ctabscf[121];

/*
  quantizer
*/
#define MANT_DIGITS 9
#define MANT_SIZE (1 << MANT_DIGITS)

#if defined(ARCH_PREFER_MULT_32x16)
#define FIXP_QTD FIXP_SGL
#define QTC FX_DBL2FXCONST_SGL
#else
#define FIXP_QTD FIXP_DBL
#define QTC
#endif

extern const FIXP_QTD FDKaacEnc_mTab_3_4[MANT_SIZE];
extern const FIXP_QTD FDKaacEnc_quantTableQ[4];
extern const FIXP_QTD FDKaacEnc_quantTableE[4];

extern const FIXP_DBL FDKaacEnc_mTab_4_3Elc[512];
extern const FIXP_DBL FDKaacEnc_specExpMantTableCombElc[4][14];
extern const UCHAR FDKaacEnc_specExpTableComb[4][14];

/*
  table to count used number of bits
*/
extern const SHORT FDKaacEnc_sideInfoTabLong[];
extern const SHORT FDKaacEnc_sideInfoTabShort[];

/*
  Psy Configuration constants
*/
extern const SFB_PARAM_LONG p_FDKaacEnc_8000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_8000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_11025_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_11025_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_12000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_12000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_16000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_16000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_22050_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_22050_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_24000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_24000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_32000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_32000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_44100_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_44100_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_48000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_48000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_64000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_64000_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_88200_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_88200_short_128;
extern const SFB_PARAM_LONG p_FDKaacEnc_96000_long_1024;
extern const SFB_PARAM_SHORT p_FDKaacEnc_96000_short_128;

/*
  TNS filter coefficients
*/
extern const FIXP_LPC FDKaacEnc_tnsEncCoeff3[8];
extern const FIXP_LPC FDKaacEnc_tnsCoeff3Borders[8];
extern const FIXP_LPC FDKaacEnc_tnsEncCoeff4[16];
extern const FIXP_LPC FDKaacEnc_tnsCoeff4Borders[16];

#define WTC0 WTC
#define WTC1 WTC
#define WTC2 WTC

extern const FIXP_WTB ELDAnalysis512[1536];
extern const FIXP_WTB ELDAnalysis480[1440];
extern const FIXP_WTB ELDAnalysis256[768];
extern const FIXP_WTB ELDAnalysis240[720];
extern const FIXP_WTB ELDAnalysis128[384];
extern const FIXP_WTB ELDAnalysis120[360];

#endif /* #ifndef AACENC_ROM_H */
