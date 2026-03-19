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

/**************************** SBR encoder library ******************************

   Author(s):   Tobias Chalupka

   Description: Definition of constant tables

*******************************************************************************/

/*!
  \file
  \brief  Definition of constant tables
  $Revision: 95404 $

  This module contains most of the constant data that can be stored in ROM.
*/

#include "sbrenc_rom.h"
#include "genericStds.h"

//@{
/*******************************************************************************

   Table Overview:

 o envelope level,   1.5 dB:
    1a)  v_Huff_envelopeLevelC10T[121]
    1b)  v_Huff_envelopeLevelL10T[121]
    2a)  v_Huff_envelopeLevelC10F[121]
    2b)  v_Huff_envelopeLevelL10F[121]

 o envelope balance, 1.5 dB:
    3a)  bookSbrEnvBalanceC10T[49]
    3b)  bookSbrEnvBalanceL10T[49]
    4a)  bookSbrEnvBalanceC10F[49]
    4b)  bookSbrEnvBalanceL10F[49]

 o envelope level,   3.0 dB:
    5a)  v_Huff_envelopeLevelC11T[63]
    5b)  v_Huff_envelopeLevelL11T[63]
    6a)  v_Huff_envelopeLevelC11F[63]
    6b)  v_Huff_envelopeLevelC11F[63]

 o envelope balance, 3.0 dB:
    7a)  bookSbrEnvBalanceC11T[25]
    7b)  bookSbrEnvBalanceL11T[25]
    8a)  bookSbrEnvBalanceC11F[25]
    8b)  bookSbrEnvBalanceL11F[25]

 o noise level,      3.0 dB:
    9a)  v_Huff_NoiseLevelC11T[63]
    9b)  v_Huff_NoiseLevelL11T[63]
    - ) (v_Huff_envelopeLevelC11F[63] is used for freq dir)
    - ) (v_Huff_envelopeLevelL11F[63] is used for freq dir)

 o noise balance,    3.0 dB:
   10a)  bookSbrNoiseBalanceC11T[25]
   10b)  bookSbrNoiseBalanceL11T[25]
    - ) (bookSbrEnvBalanceC11F[25] is used for freq dir)
    - ) (bookSbrEnvBalanceL11F[25] is used for freq dir)


  (1.5 dB is never used for noise)

********************************************************************************/

/*******************************************************************************/
/* table       : envelope level, 1.5 dB */
/* theor range : [-58,58], CODE_BOOK_SCF_LAV   = 58 */
/* implem range: [-60,60], CODE_BOOK_SCF_LAV10 = 60 */
/* raw stats   : envelopeLevel_00 (yes, wrong suffix in name)  KK 01-03-09 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode3C2FIX.m/envelopeLevel_00T_cF.mat/v_nChex_cF
   built by : FH 01-07-05 */

const INT v_Huff_envelopeLevelC10T[121] = {
    0x0003FFD6, 0x0003FFD7, 0x0003FFD8, 0x0003FFD9, 0x0003FFDA, 0x0003FFDB,
    0x0007FFB8, 0x0007FFB9, 0x0007FFBA, 0x0007FFBB, 0x0007FFBC, 0x0007FFBD,
    0x0007FFBE, 0x0007FFBF, 0x0007FFC0, 0x0007FFC1, 0x0007FFC2, 0x0007FFC3,
    0x0007FFC4, 0x0007FFC5, 0x0007FFC6, 0x0007FFC7, 0x0007FFC8, 0x0007FFC9,
    0x0007FFCA, 0x0007FFCB, 0x0007FFCC, 0x0007FFCD, 0x0007FFCE, 0x0007FFCF,
    0x0007FFD0, 0x0007FFD1, 0x0007FFD2, 0x0007FFD3, 0x0001FFE6, 0x0003FFD4,
    0x0000FFF0, 0x0001FFE9, 0x0003FFD5, 0x0001FFE7, 0x0000FFF1, 0x0000FFEC,
    0x0000FFED, 0x0000FFEE, 0x00007FF4, 0x00003FF9, 0x00003FF7, 0x00001FFA,
    0x00001FF9, 0x00000FFB, 0x000007FC, 0x000003FC, 0x000001FD, 0x000000FD,
    0x0000007D, 0x0000003D, 0x0000001D, 0x0000000D, 0x00000005, 0x00000001,
    0x00000000, 0x00000004, 0x0000000C, 0x0000001C, 0x0000003C, 0x0000007C,
    0x000000FC, 0x000001FC, 0x000003FD, 0x00000FFA, 0x00001FF8, 0x00003FF6,
    0x00003FF8, 0x00007FF5, 0x0000FFEF, 0x0001FFE8, 0x0000FFF2, 0x0007FFD4,
    0x0007FFD5, 0x0007FFD6, 0x0007FFD7, 0x0007FFD8, 0x0007FFD9, 0x0007FFDA,
    0x0007FFDB, 0x0007FFDC, 0x0007FFDD, 0x0007FFDE, 0x0007FFDF, 0x0007FFE0,
    0x0007FFE1, 0x0007FFE2, 0x0007FFE3, 0x0007FFE4, 0x0007FFE5, 0x0007FFE6,
    0x0007FFE7, 0x0007FFE8, 0x0007FFE9, 0x0007FFEA, 0x0007FFEB, 0x0007FFEC,
    0x0007FFED, 0x0007FFEE, 0x0007FFEF, 0x0007FFF0, 0x0007FFF1, 0x0007FFF2,
    0x0007FFF3, 0x0007FFF4, 0x0007FFF5, 0x0007FFF6, 0x0007FFF7, 0x0007FFF8,
    0x0007FFF9, 0x0007FFFA, 0x0007FFFB, 0x0007FFFC, 0x0007FFFD, 0x0007FFFE,
    0x0007FFFF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode3C2FIX.m/envelopeLevel_00T_cF.mat/v_nLhex_cF
   built by : FH 01-07-05 */

const UCHAR v_Huff_envelopeLevelL10T[121] = {
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x11, 0x12, 0x10, 0x11, 0x12, 0x11, 0x10, 0x10, 0x10, 0x10,
    0x0F, 0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07,
    0x06, 0x05, 0x04, 0x03, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0C, 0x0D, 0x0E, 0x0E, 0x0F, 0x10, 0x11, 0x10,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};

/* direction: freq
   contents : codewords
   raw table: HuffCode3C2FIX.m/envelopeLevel_00F_cF.mat/v_nChex_cF
   built by : FH 01-07-05 */

const INT v_Huff_envelopeLevelC10F[121] = {
    0x0007FFE7, 0x0007FFE8, 0x000FFFD2, 0x000FFFD3, 0x000FFFD4, 0x000FFFD5,
    0x000FFFD6, 0x000FFFD7, 0x000FFFD8, 0x0007FFDA, 0x000FFFD9, 0x000FFFDA,
    0x000FFFDB, 0x000FFFDC, 0x0007FFDB, 0x000FFFDD, 0x0007FFDC, 0x0007FFDD,
    0x000FFFDE, 0x0003FFE4, 0x000FFFDF, 0x000FFFE0, 0x000FFFE1, 0x0007FFDE,
    0x000FFFE2, 0x000FFFE3, 0x000FFFE4, 0x0007FFDF, 0x000FFFE5, 0x0007FFE0,
    0x0003FFE8, 0x0007FFE1, 0x0003FFE0, 0x0003FFE9, 0x0001FFEF, 0x0003FFE5,
    0x0001FFEC, 0x0001FFED, 0x0001FFEE, 0x0000FFF4, 0x0000FFF3, 0x0000FFF0,
    0x00007FF7, 0x00007FF6, 0x00003FFA, 0x00001FFA, 0x00001FF9, 0x00000FFA,
    0x00000FF8, 0x000007F9, 0x000003FB, 0x000001FC, 0x000001FA, 0x000000FB,
    0x0000007C, 0x0000003C, 0x0000001C, 0x0000000C, 0x00000005, 0x00000001,
    0x00000000, 0x00000004, 0x0000000D, 0x0000001D, 0x0000003D, 0x000000FA,
    0x000000FC, 0x000001FB, 0x000003FA, 0x000007F8, 0x000007FA, 0x000007FB,
    0x00000FF9, 0x00000FFB, 0x00001FF8, 0x00001FFB, 0x00003FF8, 0x00003FF9,
    0x0000FFF1, 0x0000FFF2, 0x0001FFEA, 0x0001FFEB, 0x0003FFE1, 0x0003FFE2,
    0x0003FFEA, 0x0003FFE3, 0x0003FFE6, 0x0003FFE7, 0x0003FFEB, 0x000FFFE6,
    0x0007FFE2, 0x000FFFE7, 0x000FFFE8, 0x000FFFE9, 0x000FFFEA, 0x000FFFEB,
    0x000FFFEC, 0x0007FFE3, 0x000FFFED, 0x000FFFEE, 0x000FFFEF, 0x000FFFF0,
    0x0007FFE4, 0x000FFFF1, 0x0003FFEC, 0x000FFFF2, 0x000FFFF3, 0x0007FFE5,
    0x0007FFE6, 0x000FFFF4, 0x000FFFF5, 0x000FFFF6, 0x000FFFF7, 0x000FFFF8,
    0x000FFFF9, 0x000FFFFA, 0x000FFFFB, 0x000FFFFC, 0x000FFFFD, 0x000FFFFE,
    0x000FFFFF};

/* direction: freq
   contents : codeword lengths
   raw table: HuffCode3C2FIX.m/envelopeLevel_00F_cF.mat/v_nLhex_cF
   built by : FH 01-07-05 */

const UCHAR v_Huff_envelopeLevelL10F[121] = {
    0x13, 0x13, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x13, 0x14,
    0x14, 0x14, 0x14, 0x13, 0x14, 0x13, 0x13, 0x14, 0x12, 0x14, 0x14,
    0x14, 0x13, 0x14, 0x14, 0x14, 0x13, 0x14, 0x13, 0x12, 0x13, 0x12,
    0x12, 0x11, 0x12, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10, 0x0F, 0x0F,
    0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x0B, 0x0A, 0x09, 0x09, 0x08, 0x07,
    0x06, 0x05, 0x04, 0x03, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
    0x08, 0x09, 0x0A, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E,
    0x0E, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x14, 0x13, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x13, 0x14,
    0x14, 0x14, 0x14, 0x13, 0x14, 0x12, 0x14, 0x14, 0x13, 0x13, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14};

/*******************************************************************************/
/* table       : envelope balance, 1.5 dB */
/* theor range : [-48,48], CODE_BOOK_SCF_LAV = 48 */
/* implem range: same but mapped to [-24,24], CODE_BOOK_SCF_LAV_BALANCE10 = 24
 */
/* raw stats   : envelopePan_00 (yes, wrong suffix in name)  KK 01-03-09 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode3C.m/envelopePan_00T.mat/v_nBhex
   built by : FH 01-05-15 */

const INT bookSbrEnvBalanceC10T[49] = {
    0x0000FFE4, 0x0000FFE5, 0x0000FFE6, 0x0000FFE7, 0x0000FFE8, 0x0000FFE9,
    0x0000FFEA, 0x0000FFEB, 0x0000FFEC, 0x0000FFED, 0x0000FFEE, 0x0000FFEF,
    0x0000FFF0, 0x0000FFF1, 0x0000FFF2, 0x0000FFF3, 0x0000FFF4, 0x0000FFE2,
    0x00000FFC, 0x000007FC, 0x000001FE, 0x0000007E, 0x0000001E, 0x00000006,
    0x00000000, 0x00000002, 0x0000000E, 0x0000003E, 0x000000FE, 0x000007FD,
    0x00000FFD, 0x00007FF0, 0x0000FFE3, 0x0000FFF5, 0x0000FFF6, 0x0000FFF7,
    0x0000FFF8, 0x0000FFF9, 0x0000FFFA, 0x0001FFF6, 0x0001FFF7, 0x0001FFF8,
    0x0001FFF9, 0x0001FFFA, 0x0001FFFB, 0x0001FFFC, 0x0001FFFD, 0x0001FFFE,
    0x0001FFFF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode3C.m/envelopePan_00T.mat/v_nLhex
   built by : FH 01-05-15 */

const UCHAR bookSbrEnvBalanceL10T[49] = {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0C, 0x0B,
    0x09, 0x07, 0x05, 0x03, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0B,
    0x0C, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

/* direction: freq
   contents : codewords
   raw table: HuffCode3C.m/envelopePan_00F.mat/v_nBhex
   built by : FH 01-05-15 */

const INT bookSbrEnvBalanceC10F[49] = {
    0x0003FFE2, 0x0003FFE3, 0x0003FFE4, 0x0003FFE5, 0x0003FFE6, 0x0003FFE7,
    0x0003FFE8, 0x0003FFE9, 0x0003FFEA, 0x0003FFEB, 0x0003FFEC, 0x0003FFED,
    0x0003FFEE, 0x0003FFEF, 0x0003FFF0, 0x0000FFF7, 0x0001FFF0, 0x00003FFC,
    0x000007FE, 0x000007FC, 0x000000FE, 0x0000007E, 0x0000000E, 0x00000002,
    0x00000000, 0x00000006, 0x0000001E, 0x0000003E, 0x000001FE, 0x000007FD,
    0x00000FFE, 0x00007FFA, 0x0000FFF6, 0x0003FFF1, 0x0003FFF2, 0x0003FFF3,
    0x0003FFF4, 0x0003FFF5, 0x0003FFF6, 0x0003FFF7, 0x0003FFF8, 0x0003FFF9,
    0x0003FFFA, 0x0003FFFB, 0x0003FFFC, 0x0003FFFD, 0x0003FFFE, 0x0007FFFE,
    0x0007FFFF};

/* direction: freq
   contents : codeword lengths
   raw table: HuffCode3C.m/envelopePan_00F.mat/v_nLhex
   built by : FH 01-05-15 */

const UCHAR bookSbrEnvBalanceL10F[49] = {
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x10, 0x11, 0x0E, 0x0B, 0x0B,
    0x08, 0x07, 0x04, 0x02, 0x01, 0x03, 0x05, 0x06, 0x09, 0x0B,
    0x0C, 0x0F, 0x10, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13};

/*******************************************************************************/
/* table       : envelope level, 3.0 dB */
/* theor range : [-29,29], CODE_BOOK_SCF_LAV   = 29 */
/* implem range: [-31,31], CODE_BOOK_SCF_LAV11 = 31 */
/* raw stats   : envelopeLevel_11  KK 00-02-03 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const INT v_Huff_envelopeLevelC11T[63] = {
    0x0003FFED, 0x0003FFEE, 0x0007FFDE, 0x0007FFDF, 0x0007FFE0, 0x0007FFE1,
    0x0007FFE2, 0x0007FFE3, 0x0007FFE4, 0x0007FFE5, 0x0007FFE6, 0x0007FFE7,
    0x0007FFE8, 0x0007FFE9, 0x0007FFEA, 0x0007FFEB, 0x0007FFEC, 0x0001FFF4,
    0x0000FFF7, 0x0000FFF9, 0x0000FFF8, 0x00003FFB, 0x00003FFA, 0x00003FF8,
    0x00001FFA, 0x00000FFC, 0x000007FC, 0x000000FE, 0x0000003E, 0x0000000E,
    0x00000002, 0x00000000, 0x00000006, 0x0000001E, 0x0000007E, 0x000001FE,
    0x000007FD, 0x00001FFB, 0x00003FF9, 0x00003FFC, 0x00007FFA, 0x0000FFF6,
    0x0001FFF5, 0x0003FFEC, 0x0007FFED, 0x0007FFEE, 0x0007FFEF, 0x0007FFF0,
    0x0007FFF1, 0x0007FFF2, 0x0007FFF3, 0x0007FFF4, 0x0007FFF5, 0x0007FFF6,
    0x0007FFF7, 0x0007FFF8, 0x0007FFF9, 0x0007FFFA, 0x0007FFFB, 0x0007FFFC,
    0x0007FFFD, 0x0007FFFE, 0x0007FFFF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const UCHAR v_Huff_envelopeLevelL11T[63] = {
    0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x11, 0x10, 0x10, 0x10, 0x0E,
    0x0E, 0x0E, 0x0D, 0x0C, 0x0B, 0x08, 0x06, 0x04, 0x02, 0x01, 0x03,
    0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0E, 0x0E, 0x0F, 0x10, 0x11, 0x12,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13};

/* direction: freq
   contents : codewords
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const INT v_Huff_envelopeLevelC11F[63] = {
    0x000FFFF0, 0x000FFFF1, 0x000FFFF2, 0x000FFFF3, 0x000FFFF4, 0x000FFFF5,
    0x000FFFF6, 0x0003FFF3, 0x0007FFF5, 0x0007FFEE, 0x0007FFEF, 0x0007FFF6,
    0x0003FFF4, 0x0003FFF2, 0x000FFFF7, 0x0007FFF0, 0x0001FFF5, 0x0003FFF0,
    0x0001FFF4, 0x0000FFF7, 0x0000FFF6, 0x00007FF8, 0x00003FFB, 0x00000FFD,
    0x000007FD, 0x000003FD, 0x000001FD, 0x000000FD, 0x0000003E, 0x0000000E,
    0x00000002, 0x00000000, 0x00000006, 0x0000001E, 0x000000FC, 0x000001FC,
    0x000003FC, 0x000007FC, 0x00000FFC, 0x00001FFC, 0x00003FFA, 0x00007FF9,
    0x00007FFA, 0x0000FFF8, 0x0000FFF9, 0x0001FFF6, 0x0001FFF7, 0x0003FFF5,
    0x0003FFF6, 0x0003FFF1, 0x000FFFF8, 0x0007FFF1, 0x0007FFF2, 0x0007FFF3,
    0x000FFFF9, 0x0007FFF7, 0x0007FFF4, 0x000FFFFA, 0x000FFFFB, 0x000FFFFC,
    0x000FFFFD, 0x000FFFFE, 0x000FFFFF};

/* direction: freq
   contents : codeword lengths
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const UCHAR v_Huff_envelopeLevelL11F[63] = {
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x12, 0x13, 0x13, 0x13,
    0x13, 0x12, 0x12, 0x14, 0x13, 0x11, 0x12, 0x11, 0x10, 0x10, 0x0F,
    0x0E, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01, 0x03,
    0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10,
    0x10, 0x11, 0x11, 0x12, 0x12, 0x12, 0x14, 0x13, 0x13, 0x13, 0x14,
    0x13, 0x13, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14};

/*******************************************************************************/
/* table       : envelope balance, 3.0 dB */
/* theor range : [-24,24], CODE_BOOK_SCF_LAV = 24 */
/* implem range: same but mapped to [-12,12], CODE_BOOK_SCF_LAV_BALANCE11 = 12
 */
/* raw stats   : envelopeBalance_11  KK 00-02-03 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode3C.m/envelopeBalance_11T.mat/v_nBhex
   built by : FH 01-05-15 */

const INT bookSbrEnvBalanceC11T[25] = {
    0x00001FF2, 0x00001FF3, 0x00001FF4, 0x00001FF5, 0x00001FF6,
    0x00001FF7, 0x00001FF8, 0x00000FF8, 0x000000FE, 0x0000007E,
    0x0000000E, 0x00000006, 0x00000000, 0x00000002, 0x0000001E,
    0x0000003E, 0x000001FE, 0x00001FF9, 0x00001FFA, 0x00001FFB,
    0x00001FFC, 0x00001FFD, 0x00001FFE, 0x00003FFE, 0x00003FFF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode3C.m/envelopeBalance_11T.mat/v_nLhex
   built by : FH 01-05-15 */

const UCHAR bookSbrEnvBalanceL11T[25] = {
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x08,
    0x07, 0x04, 0x03, 0x01, 0x02, 0x05, 0x06, 0x09, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0E, 0x0E};

/* direction: freq
   contents : codewords
   raw table: HuffCode3C.m/envelopeBalance_11F.mat/v_nBhex
   built by : FH 01-05-15 */

const INT bookSbrEnvBalanceC11F[25] = {
    0x00001FF7, 0x00001FF8, 0x00001FF9, 0x00001FFA, 0x00001FFB,
    0x00003FF8, 0x00003FF9, 0x000007FC, 0x000000FE, 0x0000007E,
    0x0000000E, 0x00000002, 0x00000000, 0x00000006, 0x0000001E,
    0x0000003E, 0x000001FE, 0x00000FFA, 0x00001FF6, 0x00003FFA,
    0x00003FFB, 0x00003FFC, 0x00003FFD, 0x00003FFE, 0x00003FFF};

/* direction: freq
   contents : codeword lengths
   raw table: HuffCode3C.m/envelopeBalance_11F.mat/v_nLhex
   built by : FH 01-05-15 */

const UCHAR bookSbrEnvBalanceL11F[25] = {
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0E, 0x0E, 0x0B, 0x08,
    0x07, 0x04, 0x02, 0x01, 0x03, 0x05, 0x06, 0x09, 0x0C,
    0x0D, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E};

/*******************************************************************************/
/* table       : noise level, 3.0 dB */
/* theor range : [-29,29], CODE_BOOK_SCF_LAV   = 29 */
/* implem range: [-31,31], CODE_BOOK_SCF_LAV11 = 31 */
/* raw stats   : noiseLevel_11  KK 00-02-03 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const INT v_Huff_NoiseLevelC11T[63] = {
    0x00001FCE, 0x00001FCF, 0x00001FD0, 0x00001FD1, 0x00001FD2, 0x00001FD3,
    0x00001FD4, 0x00001FD5, 0x00001FD6, 0x00001FD7, 0x00001FD8, 0x00001FD9,
    0x00001FDA, 0x00001FDB, 0x00001FDC, 0x00001FDD, 0x00001FDE, 0x00001FDF,
    0x00001FE0, 0x00001FE1, 0x00001FE2, 0x00001FE3, 0x00001FE4, 0x00001FE5,
    0x00001FE6, 0x00001FE7, 0x000007F2, 0x000000FD, 0x0000003E, 0x0000000E,
    0x00000006, 0x00000000, 0x00000002, 0x0000001E, 0x000000FC, 0x000003F8,
    0x00001FCC, 0x00001FE8, 0x00001FE9, 0x00001FEA, 0x00001FEB, 0x00001FEC,
    0x00001FCD, 0x00001FED, 0x00001FEE, 0x00001FEF, 0x00001FF0, 0x00001FF1,
    0x00001FF2, 0x00001FF3, 0x00001FF4, 0x00001FF5, 0x00001FF6, 0x00001FF7,
    0x00001FF8, 0x00001FF9, 0x00001FFA, 0x00001FFB, 0x00001FFC, 0x00001FFD,
    0x00001FFE, 0x00003FFE, 0x00003FFF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode2.m
   built by : FH 00-02-04 */

const UCHAR v_Huff_NoiseLevelL11T[63] = {
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000B, 0x00000008, 0x00000006, 0x00000004,
    0x00000003, 0x00000001, 0x00000002, 0x00000005, 0x00000008, 0x0000000A,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D, 0x0000000D,
    0x0000000D, 0x0000000E, 0x0000000E};

/*******************************************************************************/
/* table       : noise balance, 3.0 dB */
/* theor range : [-24,24], CODE_BOOK_SCF_LAV = 24 */
/* implem range: same but mapped to [-12,12], CODE_BOOK_SCF_LAV_BALANCE11 = 12
 */
/* raw stats   : noiseBalance_11  KK 00-02-03 */
/*******************************************************************************/

/* direction: time
   contents : codewords
   raw table: HuffCode3C.m/noiseBalance_11.mat/v_nBhex
   built by : FH 01-05-15 */

const INT bookSbrNoiseBalanceC11T[25] = {
    0x000000EC, 0x000000ED, 0x000000EE, 0x000000EF, 0x000000F0,
    0x000000F1, 0x000000F2, 0x000000F3, 0x000000F4, 0x000000F5,
    0x0000001C, 0x00000002, 0x00000000, 0x00000006, 0x0000003A,
    0x000000F6, 0x000000F7, 0x000000F8, 0x000000F9, 0x000000FA,
    0x000000FB, 0x000000FC, 0x000000FD, 0x000000FE, 0x000000FF};

/* direction: time
   contents : codeword lengths
   raw table: HuffCode3C.m/noiseBalance_11.mat/v_nLhex
   built by : FH 01-05-15 */

const UCHAR bookSbrNoiseBalanceL11T[25] = {
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x05, 0x02, 0x01, 0x03, 0x06, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};

/*
   tuningTable
*/
const sbrTuningTable_t sbrTuningTable[] = {
    /* Some of the low bitrates are commented out here, this is because the
       encoder could lose frames at those bitrates and throw an error
       because it has insufficient bits to encode for some test items.
    */

    /*** HE-AAC section ***/
    /*                        sf,sfsp,sf,sfsp,nnb,nfo,saml,SM,FS*/

    /*** mono ***/

    /* 8/16 kHz dual rate */
    {CODEC_AAC, 8000, 10000, 8000, 1, 7, 6, 11, 10, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 10000, 12000, 8000, 1, 11, 7, 13, 12, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 12000, 16001, 8000, 1, 14, 10, 13, 13, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 24000, 8000, 1, 14, 10, 14, 14, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 24000, 32000, 8000, 1, 14, 10, 14, 14, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 32000, 48001, 8000, 1, 14, 11, 15, 15, 2, 0, 3, SBR_MONO, 2},

    /* 11/22 kHz dual rate */
    {CODEC_AAC, 8000, 10000, 11025, 1, 5, 4, 6, 6, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 10000, 12000, 11025, 1, 8, 5, 12, 9, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 12000, 16000, 11025, 1, 12, 8, 13, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 20000, 11025, 1, 12, 8, 13, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 20000, 24001, 11025, 1, 13, 9, 13, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 24000, 32000, 11025, 1, 14, 10, 14, 9, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 32000, 48000, 11025, 1, 15, 11, 15, 10, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 48000, 64001, 11025, 1, 15, 11, 15, 10, 2, 0, 3, SBR_MONO, 1},

    /* 12/24 kHz dual rate */
    {CODEC_AAC, 8000, 10000, 12000, 1, 4, 3, 6, 6, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 10000, 12000, 12000, 1, 7, 4, 11, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 12000, 16000, 12000, 1, 11, 7, 12, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 20000, 12000, 1, 11, 7, 12, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 20000, 24001, 12000, 1, 12, 8, 12, 8, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 24000, 32000, 12000, 1, 13, 9, 13, 9, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 32000, 48000, 12000, 1, 14, 10, 14, 10, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 48000, 64001, 12000, 1, 14, 11, 15, 11, 2, 0, 3, SBR_MONO, 1},

    /* 16/32 kHz dual rate */
    {CODEC_AAC, 8000, 10000, 16000, 1, 1, 1, 0, 0, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 10000, 12000, 16000, 1, 2, 1, 6, 0, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 12000, 16000, 16000, 1, 4, 2, 6, 0, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 18000, 16000, 1, 4, 2, 8, 3, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 18000, 22000, 16000, 1, 6, 5, 11, 7, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 22000, 28000, 16000, 1, 10, 9, 12, 8, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 28000, 36000, 16000, 1, 12, 12, 13, 13, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 36000, 44000, 16000, 1, 14, 14, 13, 13, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 44000, 64001, 16000, 1, 14, 14, 13, 13, 2, 0, 3, SBR_MONO, 1},

    /* 22.05/44.1 kHz dual rate */
    /* { CODEC_AAC,   8000, 11369,  22050, 1,  1, 1, 1, 1,  1, 0, 6,
       SBR_MONO, 3 }, */
    {CODEC_AAC, 11369, 16000, 22050, 1, 3, 1, 4, 4, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 18000, 22050, 1, 3, 1, 5, 4, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 18000, 22000, 22050, 1, 4, 4, 8, 5, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 22000, 28000, 22050, 1, 7, 6, 8, 6, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 28000, 36000, 22050, 1, 10, 10, 9, 9, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 36000, 44000, 22050, 1, 11, 11, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 44000, 64001, 22050, 1, 13, 13, 12, 12, 2, 0, 3, SBR_MONO, 1},

    /* 24/48 kHz dual rate */
    /* { CODEC_AAC,   8000, 12000,  24000, 1,  1, 1, 1, 1,  1, 0, 6,
       SBR_MONO, 3 }, */
    {CODEC_AAC, 12000, 16000, 24000, 1, 3, 1, 4, 4, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 16000, 18000, 24000, 1, 3, 1, 5, 4, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AAC, 18000, 22000, 24000, 1, 4, 3, 8, 5, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 22000, 28000, 24000, 1, 7, 6, 8, 6, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AAC, 28000, 36000, 24000, 1, 10, 10, 9, 9, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 36000, 44000, 24000, 1, 11, 11, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 44000, 64001, 24000, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO, 1},

    /* 32/64 kHz dual rate */
    {CODEC_AAC, 24000, 36000, 32000, 1, 4, 4, 4, 4, 2, 0, 3, SBR_MONO, 3},
    {CODEC_AAC, 36000, 60000, 32000, 1, 7, 7, 6, 6, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 60000, 72000, 32000, 1, 9, 9, 8, 8, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 72000, 100000, 32000, 1, 11, 11, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 100000, 160001, 32000, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO, 1},

    /* 44.1/88.2 kHz dual rate */
    {CODEC_AAC, 24000, 36000, 44100, 1, 4, 4, 4, 4, 2, 0, 3, SBR_MONO, 3},
    {CODEC_AAC, 36000, 60000, 44100, 1, 7, 7, 6, 6, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 60000, 72000, 44100, 1, 9, 9, 8, 8, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 72000, 100000, 44100, 1, 11, 11, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 100000, 160001, 44100, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO, 1},

    /* 48/96 kHz dual rate */
    {CODEC_AAC, 32000, 36000, 48000, 1, 4, 4, 9, 9, 2, 0, 3, SBR_MONO, 3},
    {CODEC_AAC, 36000, 60000, 48000, 1, 7, 7, 10, 10, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AAC, 60000, 72000, 48000, 1, 9, 9, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 72000, 100000, 48000, 1, 11, 11, 11, 11, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AAC, 100000, 160001, 48000, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO, 1},

    /*** stereo ***/
    /* 08/16 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 8000, 2, 6, 6, 9, 7, 1, 0, -3, SBR_SWITCH_LRC, 3},
    {CODEC_AAC, 24000, 28000, 8000, 2, 9, 9, 11, 9, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 36000, 8000, 2, 11, 9, 11, 9, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 8000, 2, 13, 11, 13, 11, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 44000, 52000, 8000, 2, 14, 12, 13, 12, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 52000, 60000, 8000, 2, 14, 14, 13, 13, 3, 0, -3, SBR_SWITCH_LRC,
     1},
    {CODEC_AAC, 60000, 76000, 8000, 2, 14, 14, 13, 13, 3, 0, -3, SBR_LEFT_RIGHT,
     1},
    {CODEC_AAC, 76000, 128001, 8000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 11/22 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 11025, 2, 7, 5, 9, 7, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 24000, 28000, 11025, 2, 10, 8, 10, 8, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 36000, 11025, 2, 12, 8, 12, 8, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 11025, 2, 13, 9, 13, 9, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 44000, 52000, 11025, 2, 14, 11, 13, 11, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AAC, 52000, 60000, 11025, 2, 15, 15, 13, 13, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AAC, 60000, 76000, 11025, 2, 15, 15, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 76000, 128001, 11025, 2, 15, 15, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 12/24 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 12000, 2, 6, 4, 9, 7, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 24000, 28000, 12000, 2, 9, 7, 10, 8, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 36000, 12000, 2, 11, 7, 12, 8, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 12000, 2, 12, 9, 12, 9, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 44000, 52000, 12000, 2, 13, 12, 13, 12, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AAC, 52000, 60000, 12000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AAC, 60000, 76000, 12000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 76000, 128001, 12000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 16/32 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 16000, 2, 4, 2, 1, 0, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 24000, 28000, 16000, 2, 8, 7, 10, 8, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 36000, 16000, 2, 10, 9, 12, 11, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 16000, 2, 13, 13, 13, 13, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AAC, 44000, 52000, 16000, 2, 14, 14, 13, 13, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AAC, 52000, 60000, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AAC, 60000, 76000, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 76000, 128001, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 22.05/44.1 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 22050, 2, 2, 1, 1, 0, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 24000, 28000, 22050, 2, 5, 4, 6, 5, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 32000, 22050, 2, 5, 4, 8, 7, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 32000, 36000, 22050, 2, 7, 6, 8, 7, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 22050, 2, 10, 10, 9, 9, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 44000, 52000, 22050, 2, 12, 12, 9, 9, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 52000, 60000, 22050, 2, 13, 13, 10, 10, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AAC, 60000, 76000, 22050, 2, 14, 14, 12, 12, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 76000, 128001, 22050, 2, 14, 14, 12, 12, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 24/48 kHz dual rate */
    {CODEC_AAC, 16000, 24000, 24000, 2, 2, 1, 1, 0, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 24000, 28000, 24000, 2, 5, 5, 6, 6, 1, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 28000, 36000, 24000, 2, 7, 6, 8, 7, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 36000, 44000, 24000, 2, 10, 10, 9, 9, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 44000, 52000, 24000, 2, 12, 12, 9, 9, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 52000, 60000, 24000, 2, 13, 13, 10, 10, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AAC, 60000, 76000, 24000, 2, 14, 14, 12, 12, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 76000, 128001, 24000, 2, 14, 14, 12, 12, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 32/64 kHz dual rate */
    {CODEC_AAC, 32000, 60000, 32000, 2, 4, 4, 4, 4, 2, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 60000, 80000, 32000, 2, 7, 7, 6, 6, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 80000, 112000, 32000, 2, 9, 9, 8, 8, 3, 0, -3, SBR_LEFT_RIGHT,
     1},
    {CODEC_AAC, 112000, 144000, 32000, 2, 11, 11, 10, 10, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 144000, 256001, 32000, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 44.1/88.2 kHz dual rate */
    {CODEC_AAC, 32000, 60000, 44100, 2, 4, 4, 4, 4, 2, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 60000, 80000, 44100, 2, 7, 7, 6, 6, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 80000, 112000, 44100, 2, 9, 9, 8, 8, 3, 0, -3, SBR_LEFT_RIGHT,
     1},
    {CODEC_AAC, 112000, 144000, 44100, 2, 11, 11, 10, 10, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 144000, 256001, 44100, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 48/96 kHz dual rate */
    {CODEC_AAC, 36000, 60000, 48000, 2, 4, 4, 9, 9, 2, 0, -3, SBR_SWITCH_LRC,
     3},
    {CODEC_AAC, 60000, 80000, 48000, 2, 7, 7, 9, 9, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AAC, 80000, 112000, 48000, 2, 9, 9, 10, 10, 3, 0, -3, SBR_LEFT_RIGHT,
     1},
    {CODEC_AAC, 112000, 144000, 48000, 2, 11, 11, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AAC, 144000, 256001, 48000, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /** AAC LOW DELAY SECTION **/

    /* 24 kHz dual rate - 12kHz singlerate is not allowed (deactivated in
       FDKsbrEnc_IsSbrSettingAvail()) */
    {CODEC_AACLD, 8000, 32000, 12000, 1, 1, 1, 0, 0, 1, 0, 6, SBR_MONO, 3},

    /*** mono ***/
    /* 16/32 kHz dual rate */
    {CODEC_AACLD, 16000, 18000, 16000, 1, 4, 5, 9, 7, 1, 0, 6, SBR_MONO, 3},
    {CODEC_AACLD, 18000, 22000, 16000, 1, 7, 7, 12, 12, 1, 6, 9, SBR_MONO, 3},
    {CODEC_AACLD, 22000, 28000, 16000, 1, 6, 6, 9, 9, 2, 3, 6, SBR_MONO, 3},
    {CODEC_AACLD, 28000, 36000, 16000, 1, 8, 8, 12, 7, 2, 9, 12, SBR_MONO, 3},
    {CODEC_AACLD, 36000, 44000, 16000, 1, 10, 14, 12, 13, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 44000, 64001, 16000, 1, 11, 14, 13, 13, 2, 0, 3, SBR_MONO, 1},

    /* 22.05/44.1 kHz dual rate */
    {CODEC_AACLD, 18000, 22000, 22050, 1, 4, 4, 5, 5, 2, 0, 6, SBR_MONO, 3},
    {CODEC_AACLD, 22000, 28000, 22050, 1, 5, 5, 6, 6, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AACLD, 28000, 36000, 22050, 1, 7, 8, 8, 8, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AACLD, 36000, 44000, 22050, 1, 9, 9, 9, 9, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 44000, 52000, 22050, 1, 12, 11, 11, 11, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 52000, 64001, 22050, 1, 13, 11, 11, 10, 2, 0, 3, SBR_MONO, 1},

    /* 24/48 kHz dual rate */
    {CODEC_AACLD, 20000, 22000, 24000, 1, 3, 4, 8, 8, 2, 0, 6, SBR_MONO, 2},
    {CODEC_AACLD, 22000, 28000, 24000, 1, 3, 8, 8, 7, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AACLD, 28000, 36000, 24000, 1, 4, 8, 8, 7, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AACLD, 36000, 56000, 24000, 1, 8, 9, 9, 8, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 56000, 64001, 24000, 1, 13, 11, 11, 10, 2, 0, 3, SBR_MONO, 1},

    /* 32/64 kHz dual rate */
    {CODEC_AACLD, 24000, 36000, 32000, 1, 4, 4, 4, 4, 2, 0, 3, SBR_MONO, 3},
    {CODEC_AACLD, 36000, 60000, 32000, 1, 7, 7, 6, 6, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AACLD, 60000, 72000, 32000, 1, 9, 9, 8, 8, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 72000, 100000, 32000, 1, 11, 11, 10, 10, 2, 0, 3, SBR_MONO,
     1},
    {CODEC_AACLD, 100000, 160001, 32000, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO,
     1},

    /* 44/88 kHz dual rate */
    {CODEC_AACLD, 36000, 60000, 44100, 1, 8, 7, 6, 9, 2, 0, 3, SBR_MONO, 2},
    {CODEC_AACLD, 60000, 72000, 44100, 1, 9, 9, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 72000, 100000, 44100, 1, 11, 11, 11, 11, 2, 0, 3, SBR_MONO,
     1},
    {CODEC_AACLD, 100000, 160001, 44100, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO,
     1},

    /* 48/96 kHz dual rate */ /* 32 and 40kbps line tuned for dual-rate SBR
                               */
    {CODEC_AACLD, 36000, 60000, 48000, 1, 4, 7, 4, 4, 2, 0, 3, SBR_MONO, 3},
    {CODEC_AACLD, 60000, 72000, 48000, 1, 9, 9, 10, 10, 2, 0, 3, SBR_MONO, 1},
    {CODEC_AACLD, 72000, 100000, 48000, 1, 11, 11, 11, 11, 2, 0, 3, SBR_MONO,
     1},
    {CODEC_AACLD, 100000, 160001, 48000, 1, 13, 13, 11, 11, 2, 0, 3, SBR_MONO,
     1},

    /*** stereo ***/
    /* 16/32 kHz dual rate */
    {CODEC_AACLD, 32000, 36000, 16000, 2, 10, 9, 12, 11, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AACLD, 36000, 44000, 16000, 2, 13, 13, 13, 13, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AACLD, 44000, 52000, 16000, 2, 10, 9, 11, 9, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AACLD, 52000, 60000, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_SWITCH_LRC, 1},
    {CODEC_AACLD, 60000, 76000, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 76000, 128001, 16000, 2, 14, 14, 13, 13, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 22.05/44.1 kHz dual rate */
    {CODEC_AACLD, 32000, 36000, 22050, 2, 5, 4, 7, 6, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 36000, 44000, 22050, 2, 5, 8, 8, 8, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 44000, 52000, 22050, 2, 7, 10, 8, 8, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 52000, 60000, 22050, 2, 9, 11, 9, 9, 3, 0, -3, SBR_SWITCH_LRC,
     1},
    {CODEC_AACLD, 60000, 76000, 22050, 2, 10, 12, 10, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 76000, 82000, 22050, 2, 12, 12, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 82000, 128001, 22050, 2, 13, 12, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 24/48 kHz dual rate */
    {CODEC_AACLD, 32000, 36000, 24000, 2, 5, 4, 7, 6, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 36000, 44000, 24000, 2, 4, 8, 8, 8, 2, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 44000, 52000, 24000, 2, 6, 10, 8, 8, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 52000, 60000, 24000, 2, 9, 11, 9, 9, 3, 0, -3, SBR_SWITCH_LRC,
     1},
    {CODEC_AACLD, 60000, 76000, 24000, 2, 11, 12, 10, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 76000, 88000, 24000, 2, 12, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 88000, 128001, 24000, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 32/64 kHz dual rate */
    {CODEC_AACLD, 60000, 80000, 32000, 2, 7, 7, 6, 6, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 80000, 112000, 32000, 2, 9, 9, 8, 8, 3, 0, -3, SBR_LEFT_RIGHT,
     1},
    {CODEC_AACLD, 112000, 144000, 32000, 2, 11, 11, 10, 10, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 144000, 256001, 32000, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 44.1/88.2 kHz dual rate */
    {CODEC_AACLD, 60000, 80000, 44100, 2, 7, 7, 6, 6, 3, 0, -3, SBR_SWITCH_LRC,
     2},
    {CODEC_AACLD, 80000, 112000, 44100, 2, 10, 10, 8, 8, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 112000, 144000, 44100, 2, 12, 12, 10, 10, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 144000, 256001, 44100, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

    /* 48/96 kHz dual rate */
    {CODEC_AACLD, 60000, 80000, 48000, 2, 7, 7, 10, 10, 2, 0, -3,
     SBR_SWITCH_LRC, 2},
    {CODEC_AACLD, 80000, 112000, 48000, 2, 9, 9, 10, 10, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 112000, 144000, 48000, 2, 11, 11, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 144000, 176000, 48000, 2, 12, 12, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},
    {CODEC_AACLD, 176000, 256001, 48000, 2, 13, 13, 11, 11, 3, 0, -3,
     SBR_LEFT_RIGHT, 1},

};

const int sbrTuningTableSize =
    sizeof(sbrTuningTable) / sizeof(sbrTuningTable[0]);

const psTuningTable_t psTuningTable[4] = {
    {8000, 22000, PSENC_STEREO_BANDS_10, PSENC_NENV_1,
     FL2FXCONST_DBL(3.0f / 4.0f)},
    {22000, 28000, PSENC_STEREO_BANDS_20, PSENC_NENV_1,
     FL2FXCONST_DBL(2.0f / 4.0f)},
    {28000, 36000, PSENC_STEREO_BANDS_20, PSENC_NENV_2,
     FL2FXCONST_DBL(1.5f / 4.0f)},
    {36000, 160001, PSENC_STEREO_BANDS_20, PSENC_NENV_4,
     FL2FXCONST_DBL(1.1f / 4.0f)},
};

//@}
