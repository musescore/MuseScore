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

/* Includes ******************************************************************/
#include "sacenc_huff_tab.h"

/* Defines *******************************************************************/

/* Data Types ****************************************************************/

/* Constants *****************************************************************/
const HUFF_CLD_TABLE fdk_sacenc_huffCLDTab = {
    {/* h1D[2][31] */
     {HUFF_PACK(0x00000000, 1),  HUFF_PACK(0x00000002, 2),
      HUFF_PACK(0x00000006, 3),  HUFF_PACK(0x0000000e, 4),
      HUFF_PACK(0x0000001e, 5),  HUFF_PACK(0x0000003e, 6),
      HUFF_PACK(0x0000007e, 7),  HUFF_PACK(0x000000fe, 8),
      HUFF_PACK(0x000001fe, 9),  HUFF_PACK(0x000003fe, 10),
      HUFF_PACK(0x000007fe, 11), HUFF_PACK(0x00000ffe, 12),
      HUFF_PACK(0x00001ffe, 13), HUFF_PACK(0x00007ffe, 15),
      HUFF_PACK(0x00007ffc, 15), HUFF_PACK(0x0000fffe, 16),
      HUFF_PACK(0x0000fffa, 16), HUFF_PACK(0x0001fffe, 17),
      HUFF_PACK(0x0001fff6, 17), HUFF_PACK(0x0003fffe, 18),
      HUFF_PACK(0x0003ffff, 18), HUFF_PACK(0x0007ffde, 19),
      HUFF_PACK(0x0003ffee, 18), HUFF_PACK(0x000fffbe, 20),
      HUFF_PACK(0x001fff7e, 21), HUFF_PACK(0x00fffbfc, 24),
      HUFF_PACK(0x00fffbfd, 24), HUFF_PACK(0x00fffbfe, 24),
      HUFF_PACK(0x00fffbff, 24), HUFF_PACK(0x007ffdfc, 23),
      HUFF_PACK(0x007ffdfd, 23)},
     {HUFF_PACK(0x00000000, 1),  HUFF_PACK(0x00000002, 2),
      HUFF_PACK(0x00000006, 3),  HUFF_PACK(0x0000000e, 4),
      HUFF_PACK(0x0000001e, 5),  HUFF_PACK(0x0000003e, 6),
      HUFF_PACK(0x0000007e, 7),  HUFF_PACK(0x000001fe, 9),
      HUFF_PACK(0x000001fc, 9),  HUFF_PACK(0x000003fe, 10),
      HUFF_PACK(0x000003fa, 10), HUFF_PACK(0x000007fe, 11),
      HUFF_PACK(0x000007f6, 11), HUFF_PACK(0x00000ffe, 12),
      HUFF_PACK(0x00000fee, 12), HUFF_PACK(0x00001ffe, 13),
      HUFF_PACK(0x00001fde, 13), HUFF_PACK(0x00003ffe, 14),
      HUFF_PACK(0x00003fbe, 14), HUFF_PACK(0x00003fbf, 14),
      HUFF_PACK(0x00007ffe, 15), HUFF_PACK(0x0000fffe, 16),
      HUFF_PACK(0x0001fffe, 17), HUFF_PACK(0x0007fffe, 19),
      HUFF_PACK(0x0007fffc, 19), HUFF_PACK(0x000ffffa, 20),
      HUFF_PACK(0x001ffffc, 21), HUFF_PACK(0x001ffffd, 21),
      HUFF_PACK(0x001ffffe, 21), HUFF_PACK(0x001fffff, 21),
      HUFF_PACK(0x000ffffb, 20)}},
    {  /* HUFF_CLD_TAB_2D */
     { /* HUFF_CLD_TAB_2D[0][] */
      {/* HUFF_CLD_TAB_2D[0][0] */
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x00000002, 3),
             HUFF_PACK(0x00000004, 5), HUFF_PACK(0x0000003e, 8)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000007, 4),
             HUFF_PACK(0x0000000e, 6), HUFF_PACK(0x000000fe, 10)},
            {HUFF_PACK(0x0000007e, 9), HUFF_PACK(0x0000001e, 7),
             HUFF_PACK(0x0000000c, 6), HUFF_PACK(0x00000005, 5)},
            {HUFF_PACK(0x000000ff, 10), HUFF_PACK(0x0000000d, 6),
             HUFF_PACK(0x00000000, 3), HUFF_PACK(0x00000003, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000002, 3), HUFF_PACK(0x00000003, 3),
             HUFF_PACK(0x00000010, 5), HUFF_PACK(0x0000007c, 7),
             HUFF_PACK(0x000000d6, 8), HUFF_PACK(0x000003ee, 10)},
            {HUFF_PACK(0x0000000a, 4), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x00000016, 5), HUFF_PACK(0x00000034, 6),
             HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x00001f7e, 13)},
            {HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x00000036, 6),
             HUFF_PACK(0x00000026, 6), HUFF_PACK(0x00000046, 7),
             HUFF_PACK(0x0000011e, 9), HUFF_PACK(0x000001f6, 9)},
            {HUFF_PACK(0x0000011f, 9), HUFF_PACK(0x000000d7, 8),
             HUFF_PACK(0x0000008e, 8), HUFF_PACK(0x000000ff, 8),
             HUFF_PACK(0x0000006a, 7), HUFF_PACK(0x0000004e, 7)},
            {HUFF_PACK(0x00000fbe, 12), HUFF_PACK(0x000007de, 11),
             HUFF_PACK(0x0000004f, 7), HUFF_PACK(0x00000037, 6),
             HUFF_PACK(0x00000017, 5), HUFF_PACK(0x0000001e, 5)},
            {HUFF_PACK(0x00001f7f, 13), HUFF_PACK(0x000000fa, 8),
             HUFF_PACK(0x00000022, 6), HUFF_PACK(0x00000012, 5),
             HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x0000000a, 4),
             HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x0000007c, 7),
             HUFF_PACK(0x000000be, 8), HUFF_PACK(0x0000017a, 9),
             HUFF_PACK(0x000000ee, 9), HUFF_PACK(0x000007b6, 11)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x00000016, 5), HUFF_PACK(0x00000026, 6),
             HUFF_PACK(0x0000003e, 7), HUFF_PACK(0x0000002e, 7),
             HUFF_PACK(0x000001ec, 9), HUFF_PACK(0x000047ce, 15)},
            {HUFF_PACK(0x00000016, 6), HUFF_PACK(0x0000003c, 6),
             HUFF_PACK(0x00000022, 6), HUFF_PACK(0x0000004e, 7),
             HUFF_PACK(0x0000003f, 7), HUFF_PACK(0x0000005e, 8),
             HUFF_PACK(0x000008fa, 12), HUFF_PACK(0x000008fb, 12)},
            {HUFF_PACK(0x0000005f, 8), HUFF_PACK(0x000000fa, 8),
             HUFF_PACK(0x000000bf, 8), HUFF_PACK(0x0000003a, 7),
             HUFF_PACK(0x000001f6, 9), HUFF_PACK(0x000001de, 10),
             HUFF_PACK(0x000003da, 10), HUFF_PACK(0x000007b7, 11)},
            {HUFF_PACK(0x000001df, 10), HUFF_PACK(0x000003ee, 10),
             HUFF_PACK(0x0000017b, 9), HUFF_PACK(0x000003ef, 10),
             HUFF_PACK(0x000001ee, 9), HUFF_PACK(0x0000008e, 8),
             HUFF_PACK(0x000001ef, 9), HUFF_PACK(0x000001fe, 9)},
            {HUFF_PACK(0x000008f8, 12), HUFF_PACK(0x0000047e, 11),
             HUFF_PACK(0x0000047f, 11), HUFF_PACK(0x00000076, 8),
             HUFF_PACK(0x0000003c, 7), HUFF_PACK(0x00000046, 7),
             HUFF_PACK(0x0000007a, 7), HUFF_PACK(0x0000007e, 7)},
            {HUFF_PACK(0x000023e6, 14), HUFF_PACK(0x000011f2, 13),
             HUFF_PACK(0x000001ff, 9), HUFF_PACK(0x0000003d, 7),
             HUFF_PACK(0x0000004f, 7), HUFF_PACK(0x0000002e, 6),
             HUFF_PACK(0x00000012, 5), HUFF_PACK(0x00000004, 4)},
            {HUFF_PACK(0x000047cf, 15), HUFF_PACK(0x0000011e, 9),
             HUFF_PACK(0x000000bc, 8), HUFF_PACK(0x000000fe, 8),
             HUFF_PACK(0x0000001c, 6), HUFF_PACK(0x00000010, 5),
             HUFF_PACK(0x0000000d, 4), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV9_2D */
           {{HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000007, 4),
             HUFF_PACK(0x00000006, 5), HUFF_PACK(0x0000007e, 7),
             HUFF_PACK(0x0000000a, 7), HUFF_PACK(0x0000001e, 8),
             HUFF_PACK(0x0000008a, 9), HUFF_PACK(0x0000004e, 10),
             HUFF_PACK(0x00000276, 10), HUFF_PACK(0x000002e2, 11)},
            {HUFF_PACK(0x00000000, 4), HUFF_PACK(0x0000000a, 4),
             HUFF_PACK(0x00000016, 5), HUFF_PACK(0x00000026, 6),
             HUFF_PACK(0x00000076, 7), HUFF_PACK(0x000000f2, 8),
             HUFF_PACK(0x00000012, 8), HUFF_PACK(0x0000005e, 8),
             HUFF_PACK(0x0000008b, 9), HUFF_PACK(0x00002e76, 15)},
            {HUFF_PACK(0x00000012, 6), HUFF_PACK(0x00000007, 5),
             HUFF_PACK(0x00000038, 6), HUFF_PACK(0x0000007c, 7),
             HUFF_PACK(0x00000008, 7), HUFF_PACK(0x00000046, 8),
             HUFF_PACK(0x000000f6, 8), HUFF_PACK(0x000001ca, 9),
             HUFF_PACK(0x0000173a, 14), HUFF_PACK(0x00001738, 14)},
            {HUFF_PACK(0x0000009e, 8), HUFF_PACK(0x0000004a, 7),
             HUFF_PACK(0x00000026, 7), HUFF_PACK(0x0000000c, 7),
             HUFF_PACK(0x0000004e, 8), HUFF_PACK(0x000000f7, 8),
             HUFF_PACK(0x0000013a, 9), HUFF_PACK(0x0000009e, 11),
             HUFF_PACK(0x000009fe, 12), HUFF_PACK(0x0000013e, 12)},
            {HUFF_PACK(0x00000026, 9), HUFF_PACK(0x0000001a, 8),
             HUFF_PACK(0x000001e6, 9), HUFF_PACK(0x000001e2, 9),
             HUFF_PACK(0x000000ee, 8), HUFF_PACK(0x000001ce, 9),
             HUFF_PACK(0x00000277, 10), HUFF_PACK(0x000003ce, 10),
             HUFF_PACK(0x000002e6, 11), HUFF_PACK(0x000004fc, 11)},
            {HUFF_PACK(0x000002e3, 11), HUFF_PACK(0x00000170, 10),
             HUFF_PACK(0x00000172, 10), HUFF_PACK(0x000000ba, 9),
             HUFF_PACK(0x0000003e, 9), HUFF_PACK(0x000001e3, 9),
             HUFF_PACK(0x0000001b, 8), HUFF_PACK(0x0000003f, 9),
             HUFF_PACK(0x0000009e, 9), HUFF_PACK(0x0000009f, 9)},
            {HUFF_PACK(0x00000b9e, 13), HUFF_PACK(0x000009ff, 12),
             HUFF_PACK(0x000004fd, 11), HUFF_PACK(0x000004fe, 11),
             HUFF_PACK(0x000001cf, 9), HUFF_PACK(0x000000ef, 8),
             HUFF_PACK(0x00000044, 8), HUFF_PACK(0x0000005f, 8),
             HUFF_PACK(0x000000e4, 8), HUFF_PACK(0x000000f0, 8)},
            {HUFF_PACK(0x00002e72, 15), HUFF_PACK(0x0000013f, 12),
             HUFF_PACK(0x00000b9f, 13), HUFF_PACK(0x0000013e, 9),
             HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x00000047, 8),
             HUFF_PACK(0x0000000e, 7), HUFF_PACK(0x0000007d, 7),
             HUFF_PACK(0x00000010, 6), HUFF_PACK(0x00000024, 6)},
            {HUFF_PACK(0x00002e77, 15), HUFF_PACK(0x00005ce6, 16),
             HUFF_PACK(0x000000bb, 9), HUFF_PACK(0x000000e6, 8),
             HUFF_PACK(0x00000016, 8), HUFF_PACK(0x000000ff, 8),
             HUFF_PACK(0x0000007a, 7), HUFF_PACK(0x0000003a, 6),
             HUFF_PACK(0x00000017, 5), HUFF_PACK(0x00000002, 4)},
            {HUFF_PACK(0x00005ce7, 16), HUFF_PACK(0x000003cf, 10),
             HUFF_PACK(0x00000017, 8), HUFF_PACK(0x000001cb, 9),
             HUFF_PACK(0x0000009c, 8), HUFF_PACK(0x0000004b, 7),
             HUFF_PACK(0x00000016, 6), HUFF_PACK(0x0000000a, 5),
             HUFF_PACK(0x00000008, 4), HUFF_PACK(0x00000006, 3)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       }},
      {/* HUFF_CLD_TAB_2D[0][1] */
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x0000003e, 6),
             HUFF_PACK(0x0000076e, 11), HUFF_PACK(0x00000ede, 12)},
            {HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000003f, 6),
             HUFF_PACK(0x000003b6, 10), HUFF_PACK(0x0000003a, 6)},
            {HUFF_PACK(0x0000001c, 5), HUFF_PACK(0x000000ee, 8),
             HUFF_PACK(0x000001da, 9), HUFF_PACK(0x0000001e, 5)},
            {HUFF_PACK(0x000000ef, 8), HUFF_PACK(0x00000edf, 12),
             HUFF_PACK(0x000000ec, 8), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000001c, 5),
             HUFF_PACK(0x0000007e, 8), HUFF_PACK(0x00000efc, 12),
             HUFF_PACK(0x0000effe, 16), HUFF_PACK(0x0001dffe, 17)},
            {HUFF_PACK(0x00000004, 3), HUFF_PACK(0x0000000a, 4),
             HUFF_PACK(0x0000003e, 7), HUFF_PACK(0x00000efe, 12),
             HUFF_PACK(0x000077fe, 15), HUFF_PACK(0x00000076, 7)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000016, 5),
             HUFF_PACK(0x000000be, 8), HUFF_PACK(0x00000efd, 12),
             HUFF_PACK(0x000000ee, 8), HUFF_PACK(0x0000000e, 5)},
            {HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x0000002e, 6),
             HUFF_PACK(0x000001de, 9), HUFF_PACK(0x000003be, 10),
             HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x0000001e, 5)},
            {HUFF_PACK(0x0000007f, 7), HUFF_PACK(0x0000005e, 7),
             HUFF_PACK(0x00003bfe, 14), HUFF_PACK(0x000000fe, 9),
             HUFF_PACK(0x0000001e, 6), HUFF_PACK(0x00000002, 3)},
            {HUFF_PACK(0x000000bf, 8), HUFF_PACK(0x0001dfff, 17),
             HUFF_PACK(0x00001dfe, 13), HUFF_PACK(0x000000ff, 9),
             HUFF_PACK(0x0000003a, 6), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000002, 3), HUFF_PACK(0x0000001c, 5),
             HUFF_PACK(0x000000bc, 8), HUFF_PACK(0x000005fc, 11),
             HUFF_PACK(0x00005ffe, 15), HUFF_PACK(0x0002ffde, 18),
             HUFF_PACK(0x000bff7e, 20), HUFF_PACK(0x0017feff, 21)},
            {HUFF_PACK(0x00000004, 3), HUFF_PACK(0x0000000a, 4),
             HUFF_PACK(0x0000000e, 7), HUFF_PACK(0x000002fa, 10),
             HUFF_PACK(0x000001fe, 13), HUFF_PACK(0x0000bff2, 16),
             HUFF_PACK(0x0005ffbe, 19), HUFF_PACK(0x000000ee, 8)},
            {HUFF_PACK(0x00000002, 4), HUFF_PACK(0x00000016, 5),
             HUFF_PACK(0x000000f6, 8), HUFF_PACK(0x000005fe, 11),
             HUFF_PACK(0x000001ff, 13), HUFF_PACK(0x0000bff6, 16),
             HUFF_PACK(0x000001de, 9), HUFF_PACK(0x0000007e, 7)},
            {HUFF_PACK(0x00000000, 5), HUFF_PACK(0x0000003c, 6),
             HUFF_PACK(0x0000000e, 8), HUFF_PACK(0x0000003e, 10),
             HUFF_PACK(0x00002ffe, 14), HUFF_PACK(0x000002fb, 10),
             HUFF_PACK(0x000000f7, 8), HUFF_PACK(0x0000002e, 6)},
            {HUFF_PACK(0x00000006, 6), HUFF_PACK(0x0000007a, 7),
             HUFF_PACK(0x0000000a, 8), HUFF_PACK(0x0000007e, 11),
             HUFF_PACK(0x000000fe, 12), HUFF_PACK(0x00000016, 9),
             HUFF_PACK(0x00000006, 7), HUFF_PACK(0x00000002, 5)},
            {HUFF_PACK(0x0000000f, 7), HUFF_PACK(0x00000076, 7),
             HUFF_PACK(0x00000017, 9), HUFF_PACK(0x00005ff8, 15),
             HUFF_PACK(0x00000bfe, 12), HUFF_PACK(0x0000001e, 9),
             HUFF_PACK(0x0000007f, 7), HUFF_PACK(0x00000003, 4)},
            {HUFF_PACK(0x00000004, 7), HUFF_PACK(0x000000bd, 8),
             HUFF_PACK(0x0000bff3, 16), HUFF_PACK(0x00005fff, 15),
             HUFF_PACK(0x00000bfa, 12), HUFF_PACK(0x0000017c, 9),
             HUFF_PACK(0x0000003a, 6), HUFF_PACK(0x00000003, 3)},
            {HUFF_PACK(0x0000017e, 9), HUFF_PACK(0x0017fefe, 21),
             HUFF_PACK(0x00017fee, 17), HUFF_PACK(0x00005ffa, 15),
             HUFF_PACK(0x00000bfb, 12), HUFF_PACK(0x000001df, 9),
             HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x00000006, 3)}},
           HUFF_PACK(0x0017feff, 21) /* escape */
       },
       {
           /* LAV9_2D */
           {{HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x00000014, 5),
             HUFF_PACK(0x0000008e, 8), HUFF_PACK(0x000004fe, 11),
             HUFF_PACK(0x000023fe, 14), HUFF_PACK(0x00008ffe, 16),
             HUFF_PACK(0x0005ffbc, 19), HUFF_PACK(0x0017fef7, 21),
             HUFF_PACK(0x0017fef7, 21), HUFF_PACK(0x0017fef7, 21)},
            {HUFF_PACK(0x00000002, 3), HUFF_PACK(0x00000002, 4),
             HUFF_PACK(0x00000044, 7), HUFF_PACK(0x0000027e, 10),
             HUFF_PACK(0x000017fc, 13), HUFF_PACK(0x0000bff6, 16),
             HUFF_PACK(0x0005ffbe, 19), HUFF_PACK(0x00011ff8, 17),
             HUFF_PACK(0x000bff7a, 20), HUFF_PACK(0x000000bc, 8)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000016, 5),
             HUFF_PACK(0x0000001a, 7), HUFF_PACK(0x000000fe, 10),
             HUFF_PACK(0x000011f6, 13), HUFF_PACK(0x0000bffe, 16),
             HUFF_PACK(0x00011ff9, 17), HUFF_PACK(0x0017fef6, 21),
             HUFF_PACK(0x0000011e, 9), HUFF_PACK(0x00000056, 7)},
            {HUFF_PACK(0x00000010, 5), HUFF_PACK(0x0000003e, 6),
             HUFF_PACK(0x0000009e, 8), HUFF_PACK(0x000007fe, 11),
             HUFF_PACK(0x000011f7, 13), HUFF_PACK(0x00005ff8, 15),
             HUFF_PACK(0x00017fee, 17), HUFF_PACK(0x000007ff, 11),
             HUFF_PACK(0x000000ae, 8), HUFF_PACK(0x0000001e, 7)},
            {HUFF_PACK(0x00000026, 6), HUFF_PACK(0x0000000e, 6),
             HUFF_PACK(0x000001ee, 9), HUFF_PACK(0x0000047e, 11),
             HUFF_PACK(0x00000bfc, 12), HUFF_PACK(0x0000bfff, 16),
             HUFF_PACK(0x000008fa, 12), HUFF_PACK(0x0000006e, 9),
             HUFF_PACK(0x000001ef, 9), HUFF_PACK(0x0000007e, 7)},
            {HUFF_PACK(0x0000007a, 7), HUFF_PACK(0x0000004e, 7),
             HUFF_PACK(0x0000007e, 9), HUFF_PACK(0x000000de, 10),
             HUFF_PACK(0x000011fe, 13), HUFF_PACK(0x00002ffe, 14),
             HUFF_PACK(0x000004ff, 11), HUFF_PACK(0x000000ff, 10),
             HUFF_PACK(0x000000bd, 8), HUFF_PACK(0x0000002e, 6)},
            {HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x000000af, 8),
             HUFF_PACK(0x000001ec, 9), HUFF_PACK(0x000001be, 11),
             HUFF_PACK(0x00011ffe, 17), HUFF_PACK(0x00002ffa, 14),
             HUFF_PACK(0x000008fe, 12), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x00000046, 7), HUFF_PACK(0x00000012, 5)},
            {HUFF_PACK(0x0000003e, 8), HUFF_PACK(0x00000045, 7),
             HUFF_PACK(0x000002fe, 10), HUFF_PACK(0x000bff7e, 20),
             HUFF_PACK(0x00005ff9, 15), HUFF_PACK(0x00005ffa, 15),
             HUFF_PACK(0x00000bfd, 12), HUFF_PACK(0x0000013e, 9),
             HUFF_PACK(0x0000000c, 6), HUFF_PACK(0x00000007, 4)},
            {HUFF_PACK(0x000000be, 8), HUFF_PACK(0x00000036, 8),
             HUFF_PACK(0x000bff7f, 20), HUFF_PACK(0x00023ffe, 18),
             HUFF_PACK(0x00011ffa, 17), HUFF_PACK(0x00005ffe, 15),
             HUFF_PACK(0x000001bf, 11), HUFF_PACK(0x000001ed, 9),
             HUFF_PACK(0x0000002a, 6), HUFF_PACK(0x00000000, 3)},
            {HUFF_PACK(0x0000017e, 9), HUFF_PACK(0x0017fef7, 21),
             HUFF_PACK(0x00047ffe, 19), HUFF_PACK(0x00047fff, 19),
             HUFF_PACK(0x00011ffb, 17), HUFF_PACK(0x00002ffb, 14),
             HUFF_PACK(0x0000047c, 11), HUFF_PACK(0x000001fe, 9),
             HUFF_PACK(0x0000003c, 6), HUFF_PACK(0x00000006, 3)}},
           HUFF_PACK(0x0017fef7, 21) /* escape */
       }}},
     { /* HUFF_CLD_TAB_2D[1][] */
      {/* HUFF_CLD_TAB_2D[1][0] */
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x0000001e, 5),
             HUFF_PACK(0x000003be, 10), HUFF_PACK(0x00000efe, 12)},
            {HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000001c, 5),
             HUFF_PACK(0x000001de, 9), HUFF_PACK(0x000000ea, 8)},
            {HUFF_PACK(0x00000074, 7), HUFF_PACK(0x000000ee, 8),
             HUFF_PACK(0x000000eb, 8), HUFF_PACK(0x0000001f, 5)},
            {HUFF_PACK(0x0000077e, 11), HUFF_PACK(0x00000eff, 12),
             HUFF_PACK(0x00000076, 7), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x00000006, 4),
             HUFF_PACK(0x00000024, 7), HUFF_PACK(0x0000025e, 11),
             HUFF_PACK(0x00003cfe, 14), HUFF_PACK(0x000079fe, 15)},
            {HUFF_PACK(0x00000006, 3), HUFF_PACK(0x00000007, 4),
             HUFF_PACK(0x00000078, 7), HUFF_PACK(0x000003ce, 10),
             HUFF_PACK(0x00001e7e, 13), HUFF_PACK(0x000000be, 9)},
            {HUFF_PACK(0x00000008, 5), HUFF_PACK(0x0000003e, 6),
             HUFF_PACK(0x00000026, 7), HUFF_PACK(0x0000012e, 10),
             HUFF_PACK(0x000000bf, 9), HUFF_PACK(0x0000002e, 7)},
            {HUFF_PACK(0x00000027, 7), HUFF_PACK(0x0000007a, 7),
             HUFF_PACK(0x000001e4, 9), HUFF_PACK(0x00000096, 9),
             HUFF_PACK(0x0000007b, 7), HUFF_PACK(0x0000003f, 6)},
            {HUFF_PACK(0x000001e6, 9), HUFF_PACK(0x000001e5, 9),
             HUFF_PACK(0x00000f3e, 12), HUFF_PACK(0x0000005e, 8),
             HUFF_PACK(0x00000016, 6), HUFF_PACK(0x0000000e, 4)},
            {HUFF_PACK(0x0000079e, 11), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x0000025f, 11), HUFF_PACK(0x0000004a, 8),
             HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x00000006, 4),
             HUFF_PACK(0x000000de, 8), HUFF_PACK(0x0000069e, 11),
             HUFF_PACK(0x000034fe, 14), HUFF_PACK(0x0001a7fe, 17),
             HUFF_PACK(0x00069ff6, 19), HUFF_PACK(0x00069ff7, 19)},
            {HUFF_PACK(0x00000002, 3), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x0000006a, 7), HUFF_PACK(0x0000034e, 10),
             HUFF_PACK(0x00001fde, 13), HUFF_PACK(0x000069fe, 15),
             HUFF_PACK(0x0001a7fc, 17), HUFF_PACK(0x00000372, 10)},
            {HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x0000003c, 6),
             HUFF_PACK(0x000000df, 8), HUFF_PACK(0x000001ee, 10),
             HUFF_PACK(0x00000dde, 12), HUFF_PACK(0x000069fa, 15),
             HUFF_PACK(0x00000373, 10), HUFF_PACK(0x0000007a, 8)},
            {HUFF_PACK(0x0000003e, 7), HUFF_PACK(0x00000068, 7),
             HUFF_PACK(0x000001ba, 9), HUFF_PACK(0x000003f6, 10),
             HUFF_PACK(0x00000d3e, 12), HUFF_PACK(0x0000034c, 10),
             HUFF_PACK(0x000001fa, 9), HUFF_PACK(0x000000d2, 8)},
            {HUFF_PACK(0x0000007e, 8), HUFF_PACK(0x0000007f, 8),
             HUFF_PACK(0x000001f8, 9), HUFF_PACK(0x000006ee, 11),
             HUFF_PACK(0x000003de, 11), HUFF_PACK(0x000001b8, 9),
             HUFF_PACK(0x000001fc, 9), HUFF_PACK(0x0000006b, 7)},
            {HUFF_PACK(0x000000f6, 9), HUFF_PACK(0x000001fe, 9),
             HUFF_PACK(0x0000034d, 10), HUFF_PACK(0x00003fbe, 14),
             HUFF_PACK(0x000007f6, 11), HUFF_PACK(0x000003fa, 10),
             HUFF_PACK(0x0000003c, 7), HUFF_PACK(0x0000003d, 6)},
            {HUFF_PACK(0x000003f7, 10), HUFF_PACK(0x00000376, 10),
             HUFF_PACK(0x0001a7ff, 17), HUFF_PACK(0x00003fbf, 14),
             HUFF_PACK(0x00000ddf, 12), HUFF_PACK(0x000001f9, 9),
             HUFF_PACK(0x00000036, 6), HUFF_PACK(0x0000000e, 4)},
            {HUFF_PACK(0x000003df, 11), HUFF_PACK(0x00034ffa, 18),
             HUFF_PACK(0x000069fb, 15), HUFF_PACK(0x000034fc, 14),
             HUFF_PACK(0x00000fee, 12), HUFF_PACK(0x000001ff, 9),
             HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV9_2D */
           {{HUFF_PACK(0x00000006, 3), HUFF_PACK(0x00000004, 4),
             HUFF_PACK(0x00000012, 7), HUFF_PACK(0x000007fe, 11),
             HUFF_PACK(0x00001f7e, 13), HUFF_PACK(0x0000fbfe, 16),
             HUFF_PACK(0x0001f7fe, 17), HUFF_PACK(0x000b7dfe, 21),
             HUFF_PACK(0x000b7dff, 21), HUFF_PACK(0x000b7dff, 21)},
            {HUFF_PACK(0x00000000, 3), HUFF_PACK(0x00000006, 4),
             HUFF_PACK(0x0000007c, 7), HUFF_PACK(0x00000046, 9),
             HUFF_PACK(0x000007d0, 12), HUFF_PACK(0x00001f4e, 14),
             HUFF_PACK(0x0000b7fe, 17), HUFF_PACK(0x00005bee, 16),
             HUFF_PACK(0x00016fbe, 18), HUFF_PACK(0x000003ee, 10)},
            {HUFF_PACK(0x00000006, 5), HUFF_PACK(0x0000000a, 5),
             HUFF_PACK(0x0000002e, 7), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x000007d2, 12), HUFF_PACK(0x00001f4f, 14),
             HUFF_PACK(0x00002dfe, 15), HUFF_PACK(0x0000b7de, 17),
             HUFF_PACK(0x000001fe, 10), HUFF_PACK(0x0000002e, 8)},
            {HUFF_PACK(0x0000007a, 7), HUFF_PACK(0x0000007e, 7),
             HUFF_PACK(0x0000007a, 8), HUFF_PACK(0x000001fa, 10),
             HUFF_PACK(0x000007fe, 12), HUFF_PACK(0x00001f7c, 13),
             HUFF_PACK(0x000016fa, 14), HUFF_PACK(0x0000009e, 10),
             HUFF_PACK(0x00000020, 8), HUFF_PACK(0x00000021, 8)},
            {HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x00000016, 7),
             HUFF_PACK(0x000000fe, 9), HUFF_PACK(0x0000016e, 10),
             HUFF_PACK(0x0000009f, 10), HUFF_PACK(0x00000b7c, 13),
             HUFF_PACK(0x000003de, 11), HUFF_PACK(0x000000b6, 9),
             HUFF_PACK(0x000000be, 9), HUFF_PACK(0x0000007c, 8)},
            {HUFF_PACK(0x0000005a, 8), HUFF_PACK(0x00000078, 8),
             HUFF_PACK(0x00000047, 9), HUFF_PACK(0x00000044, 9),
             HUFF_PACK(0x000007ff, 12), HUFF_PACK(0x000007d1, 12),
             HUFF_PACK(0x000001f6, 10), HUFF_PACK(0x000001f7, 10),
             HUFF_PACK(0x0000002f, 8), HUFF_PACK(0x0000002c, 7)},
            {HUFF_PACK(0x000000fc, 9), HUFF_PACK(0x000001f6, 9),
             HUFF_PACK(0x000000f6, 9), HUFF_PACK(0x000007ff, 11),
             HUFF_PACK(0x000016fe, 14), HUFF_PACK(0x000002de, 11),
             HUFF_PACK(0x000003ea, 11), HUFF_PACK(0x000000bf, 9),
             HUFF_PACK(0x000000fa, 8), HUFF_PACK(0x0000000a, 6)},
            {HUFF_PACK(0x0000004e, 9), HUFF_PACK(0x00000026, 8),
             HUFF_PACK(0x000001ee, 10), HUFF_PACK(0x00005bfe, 16),
             HUFF_PACK(0x00003efe, 14), HUFF_PACK(0x00000b7e, 13),
             HUFF_PACK(0x000003eb, 11), HUFF_PACK(0x000001fe, 9),
             HUFF_PACK(0x0000007b, 7), HUFF_PACK(0x00000007, 5)},
            {HUFF_PACK(0x000001fb, 10), HUFF_PACK(0x00000045, 9),
             HUFF_PACK(0x00016ffe, 18), HUFF_PACK(0x0001f7ff, 17),
             HUFF_PACK(0x00002df6, 15), HUFF_PACK(0x00001f7d, 13),
             HUFF_PACK(0x000003fe, 11), HUFF_PACK(0x0000005e, 8),
             HUFF_PACK(0x0000003c, 6), HUFF_PACK(0x0000000e, 4)},
            {HUFF_PACK(0x000003df, 11), HUFF_PACK(0x0005befe, 20),
             HUFF_PACK(0x0002df7e, 19), HUFF_PACK(0x00016fff, 18),
             HUFF_PACK(0x00007dfe, 15), HUFF_PACK(0x00000fa6, 13),
             HUFF_PACK(0x000007de, 11), HUFF_PACK(0x00000079, 8),
             HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x000b7dff, 21) /* escape */
       }},
      {/* HUFF_CLD_TAB_2D[1][1] */
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x000000fa, 8), HUFF_PACK(0x000007de, 11)},
            {HUFF_PACK(0x0000000c, 4), HUFF_PACK(0x0000001e, 5),
             HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x000001f6, 9)},
            {HUFF_PACK(0x000000ff, 8), HUFF_PACK(0x0000007c, 7),
             HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x0000001a, 5)},
            {HUFF_PACK(0x000007df, 11), HUFF_PACK(0x000003ee, 10),
             HUFF_PACK(0x0000001b, 5), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x0000007c, 7), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x00000fbe, 12), HUFF_PACK(0x00003efe, 14)},
            {HUFF_PACK(0x00000000, 3), HUFF_PACK(0x00000001, 3),
             HUFF_PACK(0x0000003c, 6), HUFF_PACK(0x0000005e, 8),
             HUFF_PACK(0x000007de, 11), HUFF_PACK(0x000007be, 11)},
            {HUFF_PACK(0x0000001e, 6), HUFF_PACK(0x0000000a, 5),
             HUFF_PACK(0x0000001f, 6), HUFF_PACK(0x0000005f, 8),
             HUFF_PACK(0x000001ee, 9), HUFF_PACK(0x000001f6, 9)},
            {HUFF_PACK(0x000001fe, 9), HUFF_PACK(0x000000fe, 8),
             HUFF_PACK(0x000000f6, 8), HUFF_PACK(0x000000fa, 8),
             HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x00000016, 6)},
            {HUFF_PACK(0x000007bf, 11), HUFF_PACK(0x000003de, 10),
             HUFF_PACK(0x000003ee, 10), HUFF_PACK(0x0000007a, 7),
             HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x00000006, 4)},
            {HUFF_PACK(0x00003eff, 14), HUFF_PACK(0x00001f7e, 13),
             HUFF_PACK(0x000003ff, 10), HUFF_PACK(0x0000002e, 7),
             HUFF_PACK(0x00000004, 4), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000002, 3), HUFF_PACK(0x0000000a, 4),
             HUFF_PACK(0x0000001a, 6), HUFF_PACK(0x000001be, 9),
             HUFF_PACK(0x000006e6, 11), HUFF_PACK(0x0000067a, 12),
             HUFF_PACK(0x00000cf2, 13), HUFF_PACK(0x000033de, 15)},
            {HUFF_PACK(0x0000000c, 4), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x000000de, 8),
             HUFF_PACK(0x00000372, 10), HUFF_PACK(0x000003d6, 11),
             HUFF_PACK(0x00000678, 12), HUFF_PACK(0x00000cf6, 13)},
            {HUFF_PACK(0x00000036, 6), HUFF_PACK(0x00000012, 5),
             HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x0000003c, 7),
             HUFF_PACK(0x000001b8, 9), HUFF_PACK(0x000003d4, 11),
             HUFF_PACK(0x0000033e, 11), HUFF_PACK(0x0000033f, 11)},
            {HUFF_PACK(0x0000007e, 8), HUFF_PACK(0x0000006a, 7),
             HUFF_PACK(0x0000004e, 7), HUFF_PACK(0x0000007e, 7),
             HUFF_PACK(0x000001ba, 9), HUFF_PACK(0x000000ce, 9),
             HUFF_PACK(0x000000f6, 9), HUFF_PACK(0x000001ee, 10)},
            {HUFF_PACK(0x000001ef, 10), HUFF_PACK(0x0000013e, 9),
             HUFF_PACK(0x0000007f, 8), HUFF_PACK(0x00000066, 8),
             HUFF_PACK(0x000000d6, 8), HUFF_PACK(0x0000003e, 7),
             HUFF_PACK(0x000000d7, 8), HUFF_PACK(0x0000009e, 8)},
            {HUFF_PACK(0x000007ae, 12), HUFF_PACK(0x000001e8, 10),
             HUFF_PACK(0x000001e9, 10), HUFF_PACK(0x0000027e, 10),
             HUFF_PACK(0x00000032, 7), HUFF_PACK(0x00000018, 6),
             HUFF_PACK(0x00000026, 6), HUFF_PACK(0x00000034, 6)},
            {HUFF_PACK(0x00000cf3, 13), HUFF_PACK(0x000007aa, 12),
             HUFF_PACK(0x000007ab, 12), HUFF_PACK(0x0000027f, 10),
             HUFF_PACK(0x000001bf, 9), HUFF_PACK(0x0000001b, 6),
             HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000000b, 4)},
            {HUFF_PACK(0x000033df, 15), HUFF_PACK(0x000019ee, 14),
             HUFF_PACK(0x000007af, 12), HUFF_PACK(0x000006e7, 11),
             HUFF_PACK(0x000001bb, 9), HUFF_PACK(0x0000007f, 7),
             HUFF_PACK(0x00000008, 4), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV9_2D */
           {{HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x00000008, 4),
             HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x000001fe, 9),
             HUFF_PACK(0x000001ba, 10), HUFF_PACK(0x00000dbe, 12),
             HUFF_PACK(0x00000d7e, 13), HUFF_PACK(0x00001af6, 14),
             HUFF_PACK(0x00007fec, 15), HUFF_PACK(0x0001ffb6, 17)},
            {HUFF_PACK(0x0000000a, 4), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x0000000c, 5), HUFF_PACK(0x00000036, 7),
             HUFF_PACK(0x000000de, 9), HUFF_PACK(0x000005fe, 11),
             HUFF_PACK(0x000006be, 12), HUFF_PACK(0x00001b7e, 13),
             HUFF_PACK(0x00007fee, 15), HUFF_PACK(0x00006dfe, 15)},
            {HUFF_PACK(0x0000001e, 6), HUFF_PACK(0x0000000e, 5),
             HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x0000006a, 7),
             HUFF_PACK(0x000001ae, 9), HUFF_PACK(0x000006fe, 11),
             HUFF_PACK(0x00000376, 11), HUFF_PACK(0x00000dfe, 13),
             HUFF_PACK(0x00000dff, 13), HUFF_PACK(0x00000d7f, 13)},
            {HUFF_PACK(0x000000b6, 8), HUFF_PACK(0x0000005e, 7),
             HUFF_PACK(0x0000007c, 7), HUFF_PACK(0x0000006e, 7),
             HUFF_PACK(0x0000006a, 8), HUFF_PACK(0x0000016a, 9),
             HUFF_PACK(0x00000ffe, 12), HUFF_PACK(0x00000dfe, 12),
             HUFF_PACK(0x00000ffc, 12), HUFF_PACK(0x00001bfe, 13)},
            {HUFF_PACK(0x0000035e, 10), HUFF_PACK(0x000001b6, 9),
             HUFF_PACK(0x0000005e, 8), HUFF_PACK(0x000000b4, 8),
             HUFF_PACK(0x0000006c, 7), HUFF_PACK(0x0000017e, 9),
             HUFF_PACK(0x0000036e, 10), HUFF_PACK(0x000003ee, 10),
             HUFF_PACK(0x0000037e, 11), HUFF_PACK(0x00000377, 11)},
            {HUFF_PACK(0x00000fff, 12), HUFF_PACK(0x000001ae, 10),
             HUFF_PACK(0x000001be, 10), HUFF_PACK(0x000001f6, 9),
             HUFF_PACK(0x000001be, 9), HUFF_PACK(0x000000da, 8),
             HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x0000016b, 9),
             HUFF_PACK(0x000000d6, 9), HUFF_PACK(0x0000037e, 10)},
            {HUFF_PACK(0x000017fe, 13), HUFF_PACK(0x00000bfe, 12),
             HUFF_PACK(0x000007de, 11), HUFF_PACK(0x000006de, 11),
             HUFF_PACK(0x000001b8, 10), HUFF_PACK(0x000000d6, 8),
             HUFF_PACK(0x0000002e, 7), HUFF_PACK(0x00000034, 7),
             HUFF_PACK(0x000000de, 8), HUFF_PACK(0x000000be, 8)},
            {HUFF_PACK(0x00007fef, 15), HUFF_PACK(0x000006bc, 12),
             HUFF_PACK(0x00001bff, 13), HUFF_PACK(0x00001ffa, 13),
             HUFF_PACK(0x000001b9, 10), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x000000fa, 8), HUFF_PACK(0x0000002e, 6),
             HUFF_PACK(0x00000034, 6), HUFF_PACK(0x0000001f, 6)},
            {HUFF_PACK(0x00006dff, 15), HUFF_PACK(0x00001af7, 14),
             HUFF_PACK(0x000036fe, 14), HUFF_PACK(0x000006fe, 12),
             HUFF_PACK(0x00000fbe, 12), HUFF_PACK(0x0000035f, 10),
             HUFF_PACK(0x000000b7, 8), HUFF_PACK(0x0000002c, 6),
             HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x00000009, 4)},
            {HUFF_PACK(0x0001ffb7, 17), HUFF_PACK(0x0000ffda, 16),
             HUFF_PACK(0x00000d7a, 13), HUFF_PACK(0x000017ff, 13),
             HUFF_PACK(0x00000fbf, 12), HUFF_PACK(0x000002fe, 10),
             HUFF_PACK(0x0000005f, 8), HUFF_PACK(0x00000016, 6),
             HUFF_PACK(0x00000004, 4), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       }}}}};

const HUFF_ICC_TABLE fdk_sacenc_huffICCTab = {
    {/* h1D[2][8] */
     {HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000002, 2),
      HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000000e, 4),
      HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000003e, 6),
      HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x0000007f, 7)},
     {HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000002, 2),
      HUFF_PACK(0x00000006, 3), HUFF_PACK(0x0000000e, 4),
      HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000003e, 6),
      HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x0000007f, 7)}},
    {  /* HUFF_ICC_TAB_2D */
     { /* HUFF_ICC_TAB_2D[0][] */
      {/* HUFF_ICC_TAB_2D[0][0] */
       {
           /* LAV1_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000006, 3)},
            {HUFF_PACK(0x00000007, 3), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x00000000, 2),
             HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x0000007e, 8)},
            {HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x00000004, 4),
             HUFF_PACK(0x00000016, 6), HUFF_PACK(0x000003fe, 11)},
            {HUFF_PACK(0x000001fe, 10), HUFF_PACK(0x000000fe, 9),
             HUFF_PACK(0x0000003e, 7), HUFF_PACK(0x0000001e, 6)},
            {HUFF_PACK(0x000003ff, 11), HUFF_PACK(0x00000017, 6),
             HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000003, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x00000002, 3),
             HUFF_PACK(0x0000000c, 5), HUFF_PACK(0x0000006a, 7),
             HUFF_PACK(0x000000dc, 8), HUFF_PACK(0x000006ee, 11)},
            {HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x0000000d, 5), HUFF_PACK(0x0000001e, 6),
             HUFF_PACK(0x000001ae, 9), HUFF_PACK(0x0000ddff, 16)},
            {HUFF_PACK(0x000000de, 8), HUFF_PACK(0x0000007e, 7),
             HUFF_PACK(0x0000001f, 6), HUFF_PACK(0x000001be, 9),
             HUFF_PACK(0x00006efe, 15), HUFF_PACK(0x0000ddfe, 16)},
            {HUFF_PACK(0x0000377e, 14), HUFF_PACK(0x00001bbe, 13),
             HUFF_PACK(0x00000dde, 12), HUFF_PACK(0x000001bf, 9),
             HUFF_PACK(0x000000d6, 8), HUFF_PACK(0x00000376, 10)},
            {HUFF_PACK(0x0000ddff, 16), HUFF_PACK(0x0000ddff, 16),
             HUFF_PACK(0x000001ba, 9), HUFF_PACK(0x00000034, 6),
             HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x0000000e, 5)},
            {HUFF_PACK(0x0000ddff, 16), HUFF_PACK(0x000001af, 9),
             HUFF_PACK(0x0000007f, 7), HUFF_PACK(0x00000036, 6),
             HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x0000ddff, 16) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x0000002e, 6), HUFF_PACK(0x00000044, 7),
             HUFF_PACK(0x00000086, 8), HUFF_PACK(0x0000069e, 11),
             HUFF_PACK(0x0000043e, 11), HUFF_PACK(0x0000087a, 12)},
            {HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x0000002a, 6), HUFF_PACK(0x00000046, 7),
             HUFF_PACK(0x0000015e, 9), HUFF_PACK(0x00000047, 7),
             HUFF_PACK(0x0000034a, 10), HUFF_PACK(0x0000087b, 12)},
            {HUFF_PACK(0x000000d6, 8), HUFF_PACK(0x00000026, 6),
             HUFF_PACK(0x0000002f, 6), HUFF_PACK(0x000000d7, 8),
             HUFF_PACK(0x0000006a, 7), HUFF_PACK(0x0000034e, 10),
             HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12)},
            {HUFF_PACK(0x000002be, 10), HUFF_PACK(0x000001a6, 9),
             HUFF_PACK(0x000001be, 9), HUFF_PACK(0x00000012, 5),
             HUFF_PACK(0x000001bf, 9), HUFF_PACK(0x0000087b, 12),
             HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12)},
            {HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12),
             HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12),
             HUFF_PACK(0x00000036, 6), HUFF_PACK(0x000000d0, 8),
             HUFF_PACK(0x0000043c, 11), HUFF_PACK(0x0000043f, 11)},
            {HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12),
             HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000034b, 10),
             HUFF_PACK(0x00000027, 6), HUFF_PACK(0x00000020, 6),
             HUFF_PACK(0x00000042, 7), HUFF_PACK(0x000000d1, 8)},
            {HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000087b, 12),
             HUFF_PACK(0x000002bf, 10), HUFF_PACK(0x000000de, 8),
             HUFF_PACK(0x000000ae, 8), HUFF_PACK(0x00000056, 7),
             HUFF_PACK(0x00000016, 5), HUFF_PACK(0x00000014, 5)},
            {HUFF_PACK(0x0000087b, 12), HUFF_PACK(0x0000069f, 11),
             HUFF_PACK(0x000001a4, 9), HUFF_PACK(0x0000010e, 9),
             HUFF_PACK(0x00000045, 7), HUFF_PACK(0x0000006e, 7),
             HUFF_PACK(0x0000001f, 5), HUFF_PACK(0x00000001, 2)}},
           HUFF_PACK(0x0000087b, 12) /* escape */
       }},
      {/* HUFF_ICC_TAB_2D[0][1] */
       {
           /* LAV1_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000006, 3)},
            {HUFF_PACK(0x00000007, 3), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x00000004, 4),
             HUFF_PACK(0x0000017e, 10), HUFF_PACK(0x000002fe, 11)},
            {HUFF_PACK(0x00000000, 2), HUFF_PACK(0x0000000e, 5),
             HUFF_PACK(0x000000be, 9), HUFF_PACK(0x00000016, 6)},
            {HUFF_PACK(0x0000000f, 5), HUFF_PACK(0x00000014, 6),
             HUFF_PACK(0x0000005e, 8), HUFF_PACK(0x00000006, 4)},
            {HUFF_PACK(0x0000002e, 7), HUFF_PACK(0x000002ff, 11),
             HUFF_PACK(0x00000015, 6), HUFF_PACK(0x00000003, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x0000001e, 5),
             HUFF_PACK(0x000003fc, 10), HUFF_PACK(0x0000fffa, 16),
             HUFF_PACK(0x000fff9e, 20), HUFF_PACK(0x000fff9f, 20)},
            {HUFF_PACK(0x00000006, 3), HUFF_PACK(0x00000004, 4),
             HUFF_PACK(0x000000be, 9), HUFF_PACK(0x00007ffe, 15),
             HUFF_PACK(0x0007ffce, 19), HUFF_PACK(0x000000fe, 8)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x0000001e, 6),
             HUFF_PACK(0x000003fd, 10), HUFF_PACK(0x0000fffb, 16),
             HUFF_PACK(0x00000ffe, 12), HUFF_PACK(0x0000003e, 6)},
            {HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x0000007e, 7),
             HUFF_PACK(0x00001ffe, 13), HUFF_PACK(0x00007fff, 15),
             HUFF_PACK(0x0000005e, 8), HUFF_PACK(0x0000000e, 5)},
            {HUFF_PACK(0x0000001f, 6), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x0001fff2, 17), HUFF_PACK(0x00000ffc, 12),
             HUFF_PACK(0x0000002e, 7), HUFF_PACK(0x0000000e, 4)},
            {HUFF_PACK(0x000000bf, 9), HUFF_PACK(0x0003ffe6, 18),
             HUFF_PACK(0x0000fff8, 16), HUFF_PACK(0x00000ffd, 12),
             HUFF_PACK(0x00000016, 6), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x0000001e, 6),
             HUFF_PACK(0x00000ffe, 12), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000fffe, 16), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000ffff, 16)},
            {HUFF_PACK(0x00000006, 3), HUFF_PACK(0x00000008, 5),
             HUFF_PACK(0x000007fe, 11), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000005a, 8)},
            {HUFF_PACK(0x00000006, 4), HUFF_PACK(0x0000007a, 7),
             HUFF_PACK(0x00000164, 10), HUFF_PACK(0x00007ffa, 15),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x00001fee, 13), HUFF_PACK(0x0000003c, 6)},
            {HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x000000fe, 8),
             HUFF_PACK(0x000002ce, 11), HUFF_PACK(0x000002cf, 11),
             HUFF_PACK(0x00007ffb, 15), HUFF_PACK(0x00001fec, 13),
             HUFF_PACK(0x000000b0, 9), HUFF_PACK(0x0000002e, 7)},
            {HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x00000165, 10), HUFF_PACK(0x00007ffc, 15),
             HUFF_PACK(0x00001fef, 13), HUFF_PACK(0x000007fa, 11),
             HUFF_PACK(0x000007f8, 11), HUFF_PACK(0x0000001f, 6)},
            {HUFF_PACK(0x0000002f, 7), HUFF_PACK(0x000000f6, 8),
             HUFF_PACK(0x00001fed, 13), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x00007ffd, 15), HUFF_PACK(0x00000ff2, 12),
             HUFF_PACK(0x000000b1, 9), HUFF_PACK(0x0000000a, 5)},
            {HUFF_PACK(0x00000009, 5), HUFF_PACK(0x00000166, 10),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x00007ffe, 15), HUFF_PACK(0x00003ffc, 14),
             HUFF_PACK(0x0000005b, 8), HUFF_PACK(0x0000000e, 4)},
            {HUFF_PACK(0x0000007e, 7), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x0000ffff, 16),
             HUFF_PACK(0x0000ffff, 16), HUFF_PACK(0x00000ff3, 12),
             HUFF_PACK(0x000000f7, 8), HUFF_PACK(0x00000000, 2)}},
           HUFF_PACK(0x0000ffff, 16) /* escape */
       }}},
     { /* HUFF_ICC_TAB_2D[1][] */
      {/* HUFF_ICC_TAB_2D[1][0] */
       {
           /* LAV1_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000006, 3)},
            {HUFF_PACK(0x00000007, 3), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x0000037e, 10), HUFF_PACK(0x00000dfe, 12)},
            {HUFF_PACK(0x0000000f, 4), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x000001ba, 9), HUFF_PACK(0x000001bb, 9)},
            {HUFF_PACK(0x000000de, 8), HUFF_PACK(0x000000dc, 8),
             HUFF_PACK(0x000001be, 9), HUFF_PACK(0x0000001a, 5)},
            {HUFF_PACK(0x000006fe, 11), HUFF_PACK(0x00000dff, 12),
             HUFF_PACK(0x00000036, 6), HUFF_PACK(0x00000000, 1)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x000001b6, 9), HUFF_PACK(0x00001b7c, 13),
             HUFF_PACK(0x0000dbfe, 16), HUFF_PACK(0x00036fff, 18)},
            {HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x0000001e, 5),
             HUFF_PACK(0x000001be, 9), HUFF_PACK(0x00000dfe, 12),
             HUFF_PACK(0x00036ffe, 18), HUFF_PACK(0x0000036e, 10)},
            {HUFF_PACK(0x0000006e, 7), HUFF_PACK(0x000000fe, 8),
             HUFF_PACK(0x000000d8, 8), HUFF_PACK(0x000036fe, 14),
             HUFF_PACK(0x000006de, 11), HUFF_PACK(0x000000de, 8)},
            {HUFF_PACK(0x000001fa, 9), HUFF_PACK(0x000000da, 8),
             HUFF_PACK(0x00000dff, 12), HUFF_PACK(0x00001b7e, 13),
             HUFF_PACK(0x000000d9, 8), HUFF_PACK(0x000000ff, 8)},
            {HUFF_PACK(0x000003f6, 10), HUFF_PACK(0x000006fe, 11),
             HUFF_PACK(0x00006dfe, 15), HUFF_PACK(0x0000037e, 10),
             HUFF_PACK(0x000000fc, 8), HUFF_PACK(0x0000001a, 5)},
            {HUFF_PACK(0x000007ee, 11), HUFF_PACK(0x0001b7fe, 17),
             HUFF_PACK(0x00001b7d, 13), HUFF_PACK(0x000007ef, 11),
             HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00036fff, 18) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x0000000c, 4),
             HUFF_PACK(0x000007ee, 11), HUFF_PACK(0x00001e7e, 13),
             HUFF_PACK(0x00003cfe, 14), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x000079ff, 15)},
            {HUFF_PACK(0x0000000e, 4), HUFF_PACK(0x0000001a, 5),
             HUFF_PACK(0x000001e6, 9), HUFF_PACK(0x00001fbe, 13),
             HUFF_PACK(0x000079fe, 15), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x000006fc, 11)},
            {HUFF_PACK(0x0000006c, 7), HUFF_PACK(0x000000f6, 8),
             HUFF_PACK(0x000001ba, 9), HUFF_PACK(0x00000dfc, 12),
             HUFF_PACK(0x00000dfd, 12), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x00000f3e, 12), HUFF_PACK(0x000001bb, 9)},
            {HUFF_PACK(0x000000dc, 8), HUFF_PACK(0x000001fe, 9),
             HUFF_PACK(0x0000036e, 10), HUFF_PACK(0x000003fe, 10),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x00000fde, 12),
             HUFF_PACK(0x000001ee, 9), HUFF_PACK(0x000000f2, 8)},
            {HUFF_PACK(0x000001fa, 9), HUFF_PACK(0x000003f6, 10),
             HUFF_PACK(0x000001be, 9), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x00001fbf, 13), HUFF_PACK(0x000003ce, 10),
             HUFF_PACK(0x000003ff, 10), HUFF_PACK(0x000000de, 8)},
            {HUFF_PACK(0x00000078, 7), HUFF_PACK(0x000000da, 8),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000006fd, 11), HUFF_PACK(0x0000036c, 10),
             HUFF_PACK(0x000001ef, 9), HUFF_PACK(0x000000fe, 8)},
            {HUFF_PACK(0x0000036f, 10), HUFF_PACK(0x00000dfe, 12),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x0000036d, 10),
             HUFF_PACK(0x000000fc, 8), HUFF_PACK(0x0000003e, 6)},
            {HUFF_PACK(0x00000dff, 12), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x000079ff, 15),
             HUFF_PACK(0x000079ff, 15), HUFF_PACK(0x0000079e, 11),
             HUFF_PACK(0x0000007a, 7), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x000079ff, 15) /* escape */
       }},
      {/* HUFF_ICC_TAB_2D[1][1] */
       {
           /* LAV1_2D */
           {{HUFF_PACK(0x00000000, 1), HUFF_PACK(0x00000006, 3)},
            {HUFF_PACK(0x00000007, 3), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV3_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x000000fc, 8), HUFF_PACK(0x00000fde, 12)},
            {HUFF_PACK(0x0000000c, 4), HUFF_PACK(0x0000000d, 4),
             HUFF_PACK(0x000001fe, 9), HUFF_PACK(0x000007ee, 11)},
            {HUFF_PACK(0x000001fa, 9), HUFF_PACK(0x000001ff, 9),
             HUFF_PACK(0x000000fe, 8), HUFF_PACK(0x0000003e, 6)},
            {HUFF_PACK(0x00000fdf, 12), HUFF_PACK(0x000003f6, 10),
             HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x00000000, 1)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV5_2D */
           {{HUFF_PACK(0x00000000, 2), HUFF_PACK(0x0000000e, 4),
             HUFF_PACK(0x0000003a, 7), HUFF_PACK(0x00000676, 11),
             HUFF_PACK(0x000019fe, 13), HUFF_PACK(0x0000cebe, 16)},
            {HUFF_PACK(0x0000000f, 4), HUFF_PACK(0x00000002, 3),
             HUFF_PACK(0x0000001e, 6), HUFF_PACK(0x000000fe, 9),
             HUFF_PACK(0x000019d6, 13), HUFF_PACK(0x0000675e, 15)},
            {HUFF_PACK(0x0000003e, 7), HUFF_PACK(0x00000032, 6),
             HUFF_PACK(0x00000018, 5), HUFF_PACK(0x0000033e, 10),
             HUFF_PACK(0x00000cfe, 12), HUFF_PACK(0x00000677, 11)},
            {HUFF_PACK(0x00000674, 11), HUFF_PACK(0x0000019c, 9),
             HUFF_PACK(0x000000ff, 9), HUFF_PACK(0x0000003b, 7),
             HUFF_PACK(0x0000001c, 6), HUFF_PACK(0x0000007e, 8)},
            {HUFF_PACK(0x000033fe, 14), HUFF_PACK(0x000033ff, 14),
             HUFF_PACK(0x00000cea, 12), HUFF_PACK(0x00000066, 7),
             HUFF_PACK(0x0000001a, 5), HUFF_PACK(0x00000006, 4)},
            {HUFF_PACK(0x0000cebf, 16), HUFF_PACK(0x000033ae, 14),
             HUFF_PACK(0x0000067e, 11), HUFF_PACK(0x0000019e, 9),
             HUFF_PACK(0x0000001b, 5), HUFF_PACK(0x00000002, 2)}},
           HUFF_PACK(0x00000000, 0) /* escape */
       },
       {
           /* LAV7_2D */
           {{HUFF_PACK(0x00000002, 2), HUFF_PACK(0x00000002, 4),
             HUFF_PACK(0x000000fe, 9), HUFF_PACK(0x000007be, 12),
             HUFF_PACK(0x00000ffc, 13), HUFF_PACK(0x00000ffd, 13),
             HUFF_PACK(0x00001efe, 15), HUFF_PACK(0x00003dfe, 16)},
            {HUFF_PACK(0x00000004, 4), HUFF_PACK(0x00000000, 3),
             HUFF_PACK(0x0000003c, 7), HUFF_PACK(0x000000f6, 10),
             HUFF_PACK(0x000001da, 11), HUFF_PACK(0x000003fe, 12),
             HUFF_PACK(0x00003dfe, 15), HUFF_PACK(0x00003dff, 16)},
            {HUFF_PACK(0x0000003c, 8), HUFF_PACK(0x0000003e, 7),
             HUFF_PACK(0x0000000a, 5), HUFF_PACK(0x0000003a, 8),
             HUFF_PACK(0x000003de, 11), HUFF_PACK(0x000007be, 13),
             HUFF_PACK(0x00000f7e, 14), HUFF_PACK(0x00001efe, 14)},
            {HUFF_PACK(0x000001de, 11), HUFF_PACK(0x000000ec, 10),
             HUFF_PACK(0x0000007e, 9), HUFF_PACK(0x0000000c, 5),
             HUFF_PACK(0x000001ee, 10), HUFF_PACK(0x00000f7e, 13),
             HUFF_PACK(0x000007fc, 12), HUFF_PACK(0x00003dff, 15)},
            {HUFF_PACK(0x00007ffe, 16), HUFF_PACK(0x000003be, 12),
             HUFF_PACK(0x000000fe, 10), HUFF_PACK(0x000001fe, 10),
             HUFF_PACK(0x0000001a, 6), HUFF_PACK(0x0000001c, 7),
             HUFF_PACK(0x000007fd, 12), HUFF_PACK(0x00000ffe, 13)},
            {HUFF_PACK(0x00003dff, 16), HUFF_PACK(0x000003bf, 12),
             HUFF_PACK(0x00001ffe, 14), HUFF_PACK(0x000003ff, 12),
             HUFF_PACK(0x0000003e, 8), HUFF_PACK(0x0000001b, 6),
             HUFF_PACK(0x0000007e, 8), HUFF_PACK(0x000000f6, 9)},
            {HUFF_PACK(0x00007fff, 16), HUFF_PACK(0x00003dff, 16),
             HUFF_PACK(0x00003ffe, 15), HUFF_PACK(0x000001db, 11),
             HUFF_PACK(0x000000ee, 10), HUFF_PACK(0x0000007a, 8),
             HUFF_PACK(0x0000000e, 5), HUFF_PACK(0x0000000b, 5)},
            {HUFF_PACK(0x00003dff, 16), HUFF_PACK(0x00003dff, 16),
             HUFF_PACK(0x000003de, 12), HUFF_PACK(0x000001fe, 11),
             HUFF_PACK(0x000001ee, 11), HUFF_PACK(0x0000007a, 9),
             HUFF_PACK(0x00000006, 5), HUFF_PACK(0x00000003, 2)}},
           HUFF_PACK(0x00003dff, 16) /* escape */
       }}}}};

const HUFF_PT0_TABLE fdk_sacenc_huffPart0Tab = {
    {/* CLD */
     HUFF_PACK(0x00000052, 8), HUFF_PACK(0x000000ae, 9),
     HUFF_PACK(0x000000af, 9), HUFF_PACK(0x00000028, 7),
     HUFF_PACK(0x0000006e, 7), HUFF_PACK(0x00000036, 6),
     HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000000e, 4),
     HUFF_PACK(0x0000000c, 4), HUFF_PACK(0x0000000a, 4),
     HUFF_PACK(0x00000002, 4), HUFF_PACK(0x00000016, 5),
     HUFF_PACK(0x00000012, 5), HUFF_PACK(0x00000017, 5),
     HUFF_PACK(0x00000000, 4), HUFF_PACK(0x00000004, 4),
     HUFF_PACK(0x00000006, 4), HUFF_PACK(0x00000008, 4),
     HUFF_PACK(0x00000007, 4), HUFF_PACK(0x00000003, 4),
     HUFF_PACK(0x00000001, 4), HUFF_PACK(0x0000001a, 5),
     HUFF_PACK(0x00000013, 5), HUFF_PACK(0x0000003e, 6),
     HUFF_PACK(0x00000016, 6), HUFF_PACK(0x00000017, 6),
     HUFF_PACK(0x0000006f, 7), HUFF_PACK(0x0000002a, 7),
     HUFF_PACK(0x00000056, 8), HUFF_PACK(0x00000053, 8),
     HUFF_PACK(0x0000003f, 6)},
    {/* ICC */
     HUFF_PACK(0x0000001e, 5), HUFF_PACK(0x0000000e, 4),
     HUFF_PACK(0x00000006, 3), HUFF_PACK(0x00000000, 2),
     HUFF_PACK(0x00000002, 2), HUFF_PACK(0x00000001, 2),
     HUFF_PACK(0x0000003e, 6), HUFF_PACK(0x0000003f, 6)}};

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
