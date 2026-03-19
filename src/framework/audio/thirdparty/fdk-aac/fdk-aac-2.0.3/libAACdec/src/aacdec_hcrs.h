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

/**************************** AAC decoder library ******************************

   Author(s):   Robert Weidner (DSP Solutions)

   Description: HCR Decoder: Defines of state-constants, masks and
                state-prototypes

*******************************************************************************/

#ifndef AACDEC_HCRS_H
#define AACDEC_HCRS_H

#include "FDK_bitstream.h"
#include "aacdec_hcr_types.h"
/* The four different kinds of types of states are:                     */
/* different states are defined as constants */ /* start   middle=self   next
                                                   stop */
#define STOP_THIS_STATE \
  0 /*                                                                      */
#define BODY_ONLY \
  1 /*   X          X                X                                      */
#define BODY_SIGN__BODY \
  2 /*   X          X         X      X [stop if no sign]                    */
#define BODY_SIGN__SIGN \
  3 /*              X                X [stop if sign bits decoded]          */
#define BODY_SIGN_ESC__BODY \
  4 /*   X          X         X      X [stop if no sign]                    */
#define BODY_SIGN_ESC__SIGN \
  5 /*              X         X      X [stop if no escape sequence]         */
#define BODY_SIGN_ESC__ESC_PREFIX \
  6 /*              X         X                                             */
#define BODY_SIGN_ESC__ESC_WORD \
  7 /*              X         X      X [stop if abs(second qsc) != 16]      */

/* examples: */

/* BODY_ONLY                    means only the codeword body will be decoded; no
 * sign bits will follow and no escapesequence will follow */

/* BODY_SIGN__BODY              means that the codeword consists of two parts;
 * body and sign part. The part '__BODY' after the two underscores shows */
/*                              that the bits which are currently decoded belong
 * to the '__BODY' of the codeword and not to the sign part. */

/* BODY_SIGN_ESC__ESC_PB        means that the codeword consists of three parts;
 * body, sign and (here: two) escape sequences;  */
/*                              P = Prefix = ones */
/*                              W = Escape Word */
/*                              A = first possible (of two) Escape sequeces */
/*                              B = second possible (of two) Escape sequeces */
/*                              The part after the two underscores shows that
 * the current bits which are decoded belong to the '__ESC_PB' - part of the */
/*                              codeword. That means the body and the sign bits
 * are decoded completely and the bits which are decoded now belong to */
/*                              the escape sequence [P = prefix; B=second
 * possible escape sequence] */

#define MSB_31_MASK 0x80000000 /* masks MSB (= Bit 31) in a 32 bit word */
#define DIMENSION_OF_ESCAPE_CODEBOOK 2 /* for cb >= 11 is dimension 2 */
#define ESCAPE_CODEBOOK 11

#define MASK_ESCAPE_PREFIX_UP 0x000F0000
#define LSB_ESCAPE_PREFIX_UP 16

#define MASK_ESCAPE_PREFIX_DOWN 0x0000F000
#define LSB_ESCAPE_PREFIX_DOWN 12

#define MASK_ESCAPE_WORD 0x00000FFF
#define MASK_FLAG_A 0x00200000
#define MASK_FLAG_B 0x00100000

extern void DecodeNonPCWs(HANDLE_FDK_BITSTREAM bs, H_HCR_INFO hHcr);

UINT Hcr_State_BODY_ONLY(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN__BODY(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN__SIGN(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN_ESC__BODY(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN_ESC__SIGN(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN_ESC__ESC_PREFIX(HANDLE_FDK_BITSTREAM, void*);
UINT Hcr_State_BODY_SIGN_ESC__ESC_WORD(HANDLE_FDK_BITSTREAM, void*);

#endif /* AACDEC_HCRS_H */
