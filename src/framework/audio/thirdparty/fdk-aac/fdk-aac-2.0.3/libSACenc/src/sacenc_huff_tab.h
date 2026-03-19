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

/*********************** MPEG surround encoder library *************************

   Author(s):   Markus Lohwasser

   Description: SAC-Encoder constant huffman tables

*******************************************************************************/

#ifndef SACENC_HUFF_TAB_H
#define SACENC_HUFF_TAB_H

/* Includes ******************************************************************/
#include "machine_type.h"

/* Defines *******************************************************************/
#define HUFF_PACK(a, b)                                    \
  {                                                        \
    ((((ULONG)a) & 0x00FFFFFF) << 8) | (((ULONG)b) & 0xFF) \
  } /*!< Pack huffman value and length information. */
#define HUFF_VALUE(a)                                                         \
  (((a.packed >> 8) & 0x00FFFFFF)) /*!< Return value from packed table entry. \
                                    */
#define HUFF_LENGTH(a) \
  ((a.packed & 0xFF)) /*!< Return length from packed table entry. */

/* Data Types ****************************************************************/
/**
 * \brief  This struct contains packed huffman entries.
 *
 * The packed entry consist of hffman value and length information.
 *
 *   |---------------------------------|
 *   |         value          | length |
 *   |---------------------------------|
 *   |<------- 31...8 ------->|< 7..0 >|
 */
typedef struct {
  ULONG packed; /*! Packed huffman entry:
                    - lower 8 bit are reservoed for length information
                    - upper 24 bit contains huffman value */
} HUFF_ENTRY;

typedef struct {
  HUFF_ENTRY entry[2][2];
  HUFF_ENTRY escape;

} LAV1_2D;

typedef struct {
  HUFF_ENTRY entry[4][4];
  HUFF_ENTRY escape;

} LAV3_2D;

typedef struct {
  HUFF_ENTRY entry[6][6];
  HUFF_ENTRY escape;

} LAV5_2D;

typedef struct {
  HUFF_ENTRY entry[7][7];
  HUFF_ENTRY escape;

} LAV6_2D;

typedef struct {
  HUFF_ENTRY entry[8][8];
  HUFF_ENTRY escape;

} LAV7_2D;

typedef struct {
  HUFF_ENTRY entry[10][10];
  HUFF_ENTRY escape;

} LAV9_2D;

typedef struct {
  HUFF_ENTRY entry[13][13];
  HUFF_ENTRY escape;

} LAV12_2D;

typedef struct {
  LAV3_2D lav3;
  LAV5_2D lav5;
  LAV7_2D lav7;
  LAV9_2D lav9;

} HUFF_CLD_TAB_2D;

typedef struct {
  LAV1_2D lav1;
  LAV3_2D lav3;
  LAV5_2D lav5;
  LAV7_2D lav7;

} HUFF_ICC_TAB_2D;

typedef struct {
  HUFF_ENTRY h1D[2][31];
  HUFF_CLD_TAB_2D h2D[2][2];

} HUFF_CLD_TABLE;

typedef struct {
  HUFF_ENTRY h1D[2][8];
  HUFF_ICC_TAB_2D h2D[2][2];

} HUFF_ICC_TABLE;

typedef struct {
  HUFF_ENTRY cld[31];
  HUFF_ENTRY icc[8];

} HUFF_PT0_TABLE;

typedef HUFF_ENTRY HUFF_RES_TABLE[5][8];

/* Constants *****************************************************************/
extern const HUFF_CLD_TABLE fdk_sacenc_huffCLDTab;
extern const HUFF_ICC_TABLE fdk_sacenc_huffICCTab;
extern const HUFF_PT0_TABLE fdk_sacenc_huffPart0Tab;

/* Function / Class Declarations *********************************************/

#endif /* SACENC_HUFF_TAB_H */
