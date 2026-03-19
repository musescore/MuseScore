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

/******************* Library for basic calculation routines ********************

   Author(s):   M. Gayer

   Description: Fixed point specific mathematical functions

*******************************************************************************/

#include "fixpoint_math.h"

/*
 * Hardware specific implementations
 */

/*
 * Fallback implementations
 */

/*****************************************************************************
  functionname: LdDataVector
*****************************************************************************/
LNK_SECTION_CODE_L1
void LdDataVector(FIXP_DBL *srcVector, FIXP_DBL *destVector, INT n) {
  INT i;
  for (i = 0; i < n; i++) {
    destVector[i] = fLog2(srcVector[i], 0);
  }
}

#define MAX_POW2_PRECISION 8
#ifndef SINETABLE_16BIT
#define POW2_PRECISION MAX_POW2_PRECISION
#else
#define POW2_PRECISION 5
#endif

/*
  Taylor series coefficients of the function x^2. The first coefficient is
  ommited (equal to 1.0).

  pow2Coeff[i-1] = (1/i!) d^i(2^x)/dx^i, i=1..MAX_POW2_PRECISION
  To evaluate the taylor series around x = 0, the coefficients are: 1/!i *
  ln(2)^i
 */
#ifndef POW2COEFF_16BIT
RAM_ALIGN
LNK_SECTION_CONSTDATA_L1
static const FIXP_DBL pow2Coeff[MAX_POW2_PRECISION] = {
    FL2FXCONST_DBL(0.693147180559945309417232121458177),   /* ln(2)^1 /1! */
    FL2FXCONST_DBL(0.240226506959100712333551263163332),   /* ln(2)^2 /2! */
    FL2FXCONST_DBL(0.0555041086648215799531422637686218),  /* ln(2)^3 /3! */
    FL2FXCONST_DBL(0.00961812910762847716197907157365887), /* ln(2)^4 /4! */
    FL2FXCONST_DBL(0.00133335581464284434234122219879962), /* ln(2)^5 /5! */
    FL2FXCONST_DBL(1.54035303933816099544370973327423e-4), /* ln(2)^6 /6! */
    FL2FXCONST_DBL(1.52527338040598402800254390120096e-5), /* ln(2)^7 /7! */
    FL2FXCONST_DBL(1.32154867901443094884037582282884e-6)  /* ln(2)^8 /8! */
};
#else
RAM_ALIGN
LNK_SECTION_CONSTDATA_L1
static const FIXP_SGL pow2Coeff[MAX_POW2_PRECISION] = {
    FL2FXCONST_SGL(0.693147180559945309417232121458177),   /* ln(2)^1 /1! */
    FL2FXCONST_SGL(0.240226506959100712333551263163332),   /* ln(2)^2 /2! */
    FL2FXCONST_SGL(0.0555041086648215799531422637686218),  /* ln(2)^3 /3! */
    FL2FXCONST_SGL(0.00961812910762847716197907157365887), /* ln(2)^4 /4! */
    FL2FXCONST_SGL(0.00133335581464284434234122219879962), /* ln(2)^5 /5! */
    FL2FXCONST_SGL(1.54035303933816099544370973327423e-4), /* ln(2)^6 /6! */
    FL2FXCONST_SGL(1.52527338040598402800254390120096e-5), /* ln(2)^7 /7! */
    FL2FXCONST_SGL(1.32154867901443094884037582282884e-6)  /* ln(2)^8 /8! */
};
#endif

/*****************************************************************************

  functionname: CalcInvLdData
  description:  Delivers the inverse of function CalcLdData().
                Delivers 2^(op*LD_DATA_SCALING)
  input:        Input op is assumed to be fractional -1.0 < op < 1.0
  output:       For op == 0, the result is MAXVAL_DBL (almost 1.0).
                For negative input values the output should be treated as a
positive fractional value. For positive input values the output should be
treated as a positive integer value. This function does not output negative
values.

*****************************************************************************/
/* Date: 06-JULY-2012 Arthur Tritthart, IIS Fraunhofer Erlangen */
/* Version with 3 table lookup and 1 linear interpolations      */
/* Algorithm: compute power of 2, argument x is in Q7.25 format */
/*  result = 2^(x/64)                                           */
/*  We split exponent (x/64) into 5 components:                 */
/*  integer part:      represented by b31..b25  (exp)           */
/*  fractional part 1: represented by b24..b20  (lookup1)       */
/*  fractional part 2: represented by b19..b15  (lookup2)       */
/*  fractional part 3: represented by b14..b10  (lookup3)       */
/*  fractional part 4: represented by b09..b00  (frac)          */
/*  => result = (lookup1*lookup2*(lookup3+C1*frac)<<3)>>exp     */
/* Due to the fact, that all lookup values contain a factor 0.5 */
/* the result has to be shifted by 3 to the right also.         */
/* Table exp2_tab_long contains the log2 for 0 to 1.0 in steps  */
/* of 1/32, table exp2w_tab_long the log2 for 0 to 1/32 in steps*/
/* of 1/1024, table exp2x_tab_long the log2 for 0 to 1/1024 in  */
/* steps of 1/32768. Since the 2-logarithm of very very small   */
/* negative value is rather linear, we can use interpolation.   */
/* Limitations:                                                 */
/* For x <= 0, the result is fractional positive                */
/* For x > 0, the result is integer in range 1...7FFF.FFFF      */
/* For x < -31/64, we have to clear the result                  */
/* For x = 0, the result is ~1.0 (0x7FFF.FFFF)                  */
/* For x >= 31/64, the result is 0x7FFF.FFFF                    */

/* This table is used for lookup 2^x with   */
/* x in range [0...1.0[ in steps of 1/32 */
LNK_SECTION_DATA_L1
const UINT exp2_tab_long[32] = {
    0x40000000, 0x4166C34C, 0x42D561B4, 0x444C0740, 0x45CAE0F2, 0x47521CC6,
    0x48E1E9BA, 0x4A7A77D4, 0x4C1BF829, 0x4DC69CDD, 0x4F7A9930, 0x51382182,
    0x52FF6B55, 0x54D0AD5A, 0x56AC1F75, 0x5891FAC1, 0x5A82799A, 0x5C7DD7A4,
    0x5E8451D0, 0x60962665, 0x62B39509, 0x64DCDEC3, 0x6712460B, 0x69540EC9,
    0x6BA27E65, 0x6DFDDBCC, 0x70666F76, 0x72DC8374, 0x75606374, 0x77F25CCE,
    0x7A92BE8B, 0x7D41D96E
    // 0x80000000
};

/* This table is used for lookup 2^x with   */
/* x in range [0...1/32[ in steps of 1/1024 */
LNK_SECTION_DATA_L1
const UINT exp2w_tab_long[32] = {
    0x40000000, 0x400B1818, 0x4016321B, 0x40214E0C, 0x402C6BE9, 0x40378BB4,
    0x4042AD6D, 0x404DD113, 0x4058F6A8, 0x40641E2B, 0x406F479E, 0x407A7300,
    0x4085A051, 0x4090CF92, 0x409C00C4, 0x40A733E6, 0x40B268FA, 0x40BD9FFF,
    0x40C8D8F5, 0x40D413DD, 0x40DF50B8, 0x40EA8F86, 0x40F5D046, 0x410112FA,
    0x410C57A2, 0x41179E3D, 0x4122E6CD, 0x412E3152, 0x41397DCC, 0x4144CC3B,
    0x41501CA0, 0x415B6EFB,
    // 0x4166C34C,
};
/* This table is used for lookup 2^x with   */
/* x in range [0...1/1024[ in steps of 1/32768 */
LNK_SECTION_DATA_L1
const UINT exp2x_tab_long[32] = {
    0x40000000, 0x400058B9, 0x4000B173, 0x40010A2D, 0x400162E8, 0x4001BBA3,
    0x4002145F, 0x40026D1B, 0x4002C5D8, 0x40031E95, 0x40037752, 0x4003D011,
    0x400428CF, 0x4004818E, 0x4004DA4E, 0x4005330E, 0x40058BCE, 0x4005E48F,
    0x40063D51, 0x40069613, 0x4006EED5, 0x40074798, 0x4007A05B, 0x4007F91F,
    0x400851E4, 0x4008AAA8, 0x4009036E, 0x40095C33, 0x4009B4FA, 0x400A0DC0,
    0x400A6688, 0x400ABF4F,
    // 0x400B1818
};

/*****************************************************************************
    functionname: InitLdInt and CalcLdInt
    description:  Create and access table with integer LdData (0 to
LD_INT_TAB_LEN)
*****************************************************************************/
#ifndef LD_INT_TAB_LEN
#define LD_INT_TAB_LEN \
  193 /* Default tab length. Lower value should be set in fix.h */
#endif

#if (LD_INT_TAB_LEN <= 120)
LNK_SECTION_CONSTDATA_L1
static const FIXP_DBL ldIntCoeff[] = {
    (FIXP_DBL)0x80000001, (FIXP_DBL)0x00000000, (FIXP_DBL)0x02000000,
    (FIXP_DBL)0x032b8034, (FIXP_DBL)0x04000000, (FIXP_DBL)0x04a4d3c2,
    (FIXP_DBL)0x052b8034, (FIXP_DBL)0x059d5da0, (FIXP_DBL)0x06000000,
    (FIXP_DBL)0x06570069, (FIXP_DBL)0x06a4d3c2, (FIXP_DBL)0x06eb3a9f,
    (FIXP_DBL)0x072b8034, (FIXP_DBL)0x0766a009, (FIXP_DBL)0x079d5da0,
    (FIXP_DBL)0x07d053f7, (FIXP_DBL)0x08000000, (FIXP_DBL)0x082cc7ee,
    (FIXP_DBL)0x08570069, (FIXP_DBL)0x087ef05b, (FIXP_DBL)0x08a4d3c2,
    (FIXP_DBL)0x08c8ddd4, (FIXP_DBL)0x08eb3a9f, (FIXP_DBL)0x090c1050,
    (FIXP_DBL)0x092b8034, (FIXP_DBL)0x0949a785, (FIXP_DBL)0x0966a009,
    (FIXP_DBL)0x0982809d, (FIXP_DBL)0x099d5da0, (FIXP_DBL)0x09b74949,
    (FIXP_DBL)0x09d053f7, (FIXP_DBL)0x09e88c6b, (FIXP_DBL)0x0a000000,
    (FIXP_DBL)0x0a16bad3, (FIXP_DBL)0x0a2cc7ee, (FIXP_DBL)0x0a423162,
    (FIXP_DBL)0x0a570069, (FIXP_DBL)0x0a6b3d79, (FIXP_DBL)0x0a7ef05b,
    (FIXP_DBL)0x0a92203d, (FIXP_DBL)0x0aa4d3c2, (FIXP_DBL)0x0ab7110e,
    (FIXP_DBL)0x0ac8ddd4, (FIXP_DBL)0x0ada3f60, (FIXP_DBL)0x0aeb3a9f,
    (FIXP_DBL)0x0afbd42b, (FIXP_DBL)0x0b0c1050, (FIXP_DBL)0x0b1bf312,
    (FIXP_DBL)0x0b2b8034, (FIXP_DBL)0x0b3abb40, (FIXP_DBL)0x0b49a785,
    (FIXP_DBL)0x0b584822, (FIXP_DBL)0x0b66a009, (FIXP_DBL)0x0b74b1fd,
    (FIXP_DBL)0x0b82809d, (FIXP_DBL)0x0b900e61, (FIXP_DBL)0x0b9d5da0,
    (FIXP_DBL)0x0baa708f, (FIXP_DBL)0x0bb74949, (FIXP_DBL)0x0bc3e9ca,
    (FIXP_DBL)0x0bd053f7, (FIXP_DBL)0x0bdc899b, (FIXP_DBL)0x0be88c6b,
    (FIXP_DBL)0x0bf45e09, (FIXP_DBL)0x0c000000, (FIXP_DBL)0x0c0b73cb,
    (FIXP_DBL)0x0c16bad3, (FIXP_DBL)0x0c21d671, (FIXP_DBL)0x0c2cc7ee,
    (FIXP_DBL)0x0c379085, (FIXP_DBL)0x0c423162, (FIXP_DBL)0x0c4caba8,
    (FIXP_DBL)0x0c570069, (FIXP_DBL)0x0c6130af, (FIXP_DBL)0x0c6b3d79,
    (FIXP_DBL)0x0c7527b9, (FIXP_DBL)0x0c7ef05b, (FIXP_DBL)0x0c88983f,
    (FIXP_DBL)0x0c92203d, (FIXP_DBL)0x0c9b8926, (FIXP_DBL)0x0ca4d3c2,
    (FIXP_DBL)0x0cae00d2, (FIXP_DBL)0x0cb7110e, (FIXP_DBL)0x0cc0052b,
    (FIXP_DBL)0x0cc8ddd4, (FIXP_DBL)0x0cd19bb0, (FIXP_DBL)0x0cda3f60,
    (FIXP_DBL)0x0ce2c97d, (FIXP_DBL)0x0ceb3a9f, (FIXP_DBL)0x0cf39355,
    (FIXP_DBL)0x0cfbd42b, (FIXP_DBL)0x0d03fda9, (FIXP_DBL)0x0d0c1050,
    (FIXP_DBL)0x0d140ca0, (FIXP_DBL)0x0d1bf312, (FIXP_DBL)0x0d23c41d,
    (FIXP_DBL)0x0d2b8034, (FIXP_DBL)0x0d3327c7, (FIXP_DBL)0x0d3abb40,
    (FIXP_DBL)0x0d423b08, (FIXP_DBL)0x0d49a785, (FIXP_DBL)0x0d510118,
    (FIXP_DBL)0x0d584822, (FIXP_DBL)0x0d5f7cff, (FIXP_DBL)0x0d66a009,
    (FIXP_DBL)0x0d6db197, (FIXP_DBL)0x0d74b1fd, (FIXP_DBL)0x0d7ba190,
    (FIXP_DBL)0x0d82809d, (FIXP_DBL)0x0d894f75, (FIXP_DBL)0x0d900e61,
    (FIXP_DBL)0x0d96bdad, (FIXP_DBL)0x0d9d5da0, (FIXP_DBL)0x0da3ee7f,
    (FIXP_DBL)0x0daa708f, (FIXP_DBL)0x0db0e412, (FIXP_DBL)0x0db74949,
    (FIXP_DBL)0x0dbda072, (FIXP_DBL)0x0dc3e9ca, (FIXP_DBL)0x0dca258e};

#elif (LD_INT_TAB_LEN <= 193)
LNK_SECTION_CONSTDATA_L1
static const FIXP_DBL ldIntCoeff[] = {
    (FIXP_DBL)0x80000001, (FIXP_DBL)0x00000000, (FIXP_DBL)0x02000000,
    (FIXP_DBL)0x032b8034, (FIXP_DBL)0x04000000, (FIXP_DBL)0x04a4d3c2,
    (FIXP_DBL)0x052b8034, (FIXP_DBL)0x059d5da0, (FIXP_DBL)0x06000000,
    (FIXP_DBL)0x06570069, (FIXP_DBL)0x06a4d3c2, (FIXP_DBL)0x06eb3a9f,
    (FIXP_DBL)0x072b8034, (FIXP_DBL)0x0766a009, (FIXP_DBL)0x079d5da0,
    (FIXP_DBL)0x07d053f7, (FIXP_DBL)0x08000000, (FIXP_DBL)0x082cc7ee,
    (FIXP_DBL)0x08570069, (FIXP_DBL)0x087ef05b, (FIXP_DBL)0x08a4d3c2,
    (FIXP_DBL)0x08c8ddd4, (FIXP_DBL)0x08eb3a9f, (FIXP_DBL)0x090c1050,
    (FIXP_DBL)0x092b8034, (FIXP_DBL)0x0949a785, (FIXP_DBL)0x0966a009,
    (FIXP_DBL)0x0982809d, (FIXP_DBL)0x099d5da0, (FIXP_DBL)0x09b74949,
    (FIXP_DBL)0x09d053f7, (FIXP_DBL)0x09e88c6b, (FIXP_DBL)0x0a000000,
    (FIXP_DBL)0x0a16bad3, (FIXP_DBL)0x0a2cc7ee, (FIXP_DBL)0x0a423162,
    (FIXP_DBL)0x0a570069, (FIXP_DBL)0x0a6b3d79, (FIXP_DBL)0x0a7ef05b,
    (FIXP_DBL)0x0a92203d, (FIXP_DBL)0x0aa4d3c2, (FIXP_DBL)0x0ab7110e,
    (FIXP_DBL)0x0ac8ddd4, (FIXP_DBL)0x0ada3f60, (FIXP_DBL)0x0aeb3a9f,
    (FIXP_DBL)0x0afbd42b, (FIXP_DBL)0x0b0c1050, (FIXP_DBL)0x0b1bf312,
    (FIXP_DBL)0x0b2b8034, (FIXP_DBL)0x0b3abb40, (FIXP_DBL)0x0b49a785,
    (FIXP_DBL)0x0b584822, (FIXP_DBL)0x0b66a009, (FIXP_DBL)0x0b74b1fd,
    (FIXP_DBL)0x0b82809d, (FIXP_DBL)0x0b900e61, (FIXP_DBL)0x0b9d5da0,
    (FIXP_DBL)0x0baa708f, (FIXP_DBL)0x0bb74949, (FIXP_DBL)0x0bc3e9ca,
    (FIXP_DBL)0x0bd053f7, (FIXP_DBL)0x0bdc899b, (FIXP_DBL)0x0be88c6b,
    (FIXP_DBL)0x0bf45e09, (FIXP_DBL)0x0c000000, (FIXP_DBL)0x0c0b73cb,
    (FIXP_DBL)0x0c16bad3, (FIXP_DBL)0x0c21d671, (FIXP_DBL)0x0c2cc7ee,
    (FIXP_DBL)0x0c379085, (FIXP_DBL)0x0c423162, (FIXP_DBL)0x0c4caba8,
    (FIXP_DBL)0x0c570069, (FIXP_DBL)0x0c6130af, (FIXP_DBL)0x0c6b3d79,
    (FIXP_DBL)0x0c7527b9, (FIXP_DBL)0x0c7ef05b, (FIXP_DBL)0x0c88983f,
    (FIXP_DBL)0x0c92203d, (FIXP_DBL)0x0c9b8926, (FIXP_DBL)0x0ca4d3c2,
    (FIXP_DBL)0x0cae00d2, (FIXP_DBL)0x0cb7110e, (FIXP_DBL)0x0cc0052b,
    (FIXP_DBL)0x0cc8ddd4, (FIXP_DBL)0x0cd19bb0, (FIXP_DBL)0x0cda3f60,
    (FIXP_DBL)0x0ce2c97d, (FIXP_DBL)0x0ceb3a9f, (FIXP_DBL)0x0cf39355,
    (FIXP_DBL)0x0cfbd42b, (FIXP_DBL)0x0d03fda9, (FIXP_DBL)0x0d0c1050,
    (FIXP_DBL)0x0d140ca0, (FIXP_DBL)0x0d1bf312, (FIXP_DBL)0x0d23c41d,
    (FIXP_DBL)0x0d2b8034, (FIXP_DBL)0x0d3327c7, (FIXP_DBL)0x0d3abb40,
    (FIXP_DBL)0x0d423b08, (FIXP_DBL)0x0d49a785, (FIXP_DBL)0x0d510118,
    (FIXP_DBL)0x0d584822, (FIXP_DBL)0x0d5f7cff, (FIXP_DBL)0x0d66a009,
    (FIXP_DBL)0x0d6db197, (FIXP_DBL)0x0d74b1fd, (FIXP_DBL)0x0d7ba190,
    (FIXP_DBL)0x0d82809d, (FIXP_DBL)0x0d894f75, (FIXP_DBL)0x0d900e61,
    (FIXP_DBL)0x0d96bdad, (FIXP_DBL)0x0d9d5da0, (FIXP_DBL)0x0da3ee7f,
    (FIXP_DBL)0x0daa708f, (FIXP_DBL)0x0db0e412, (FIXP_DBL)0x0db74949,
    (FIXP_DBL)0x0dbda072, (FIXP_DBL)0x0dc3e9ca, (FIXP_DBL)0x0dca258e,
    (FIXP_DBL)0x0dd053f7, (FIXP_DBL)0x0dd6753e, (FIXP_DBL)0x0ddc899b,
    (FIXP_DBL)0x0de29143, (FIXP_DBL)0x0de88c6b, (FIXP_DBL)0x0dee7b47,
    (FIXP_DBL)0x0df45e09, (FIXP_DBL)0x0dfa34e1, (FIXP_DBL)0x0e000000,
    (FIXP_DBL)0x0e05bf94, (FIXP_DBL)0x0e0b73cb, (FIXP_DBL)0x0e111cd2,
    (FIXP_DBL)0x0e16bad3, (FIXP_DBL)0x0e1c4dfb, (FIXP_DBL)0x0e21d671,
    (FIXP_DBL)0x0e275460, (FIXP_DBL)0x0e2cc7ee, (FIXP_DBL)0x0e323143,
    (FIXP_DBL)0x0e379085, (FIXP_DBL)0x0e3ce5d8, (FIXP_DBL)0x0e423162,
    (FIXP_DBL)0x0e477346, (FIXP_DBL)0x0e4caba8, (FIXP_DBL)0x0e51daa8,
    (FIXP_DBL)0x0e570069, (FIXP_DBL)0x0e5c1d0b, (FIXP_DBL)0x0e6130af,
    (FIXP_DBL)0x0e663b74, (FIXP_DBL)0x0e6b3d79, (FIXP_DBL)0x0e7036db,
    (FIXP_DBL)0x0e7527b9, (FIXP_DBL)0x0e7a1030, (FIXP_DBL)0x0e7ef05b,
    (FIXP_DBL)0x0e83c857, (FIXP_DBL)0x0e88983f, (FIXP_DBL)0x0e8d602e,
    (FIXP_DBL)0x0e92203d, (FIXP_DBL)0x0e96d888, (FIXP_DBL)0x0e9b8926,
    (FIXP_DBL)0x0ea03232, (FIXP_DBL)0x0ea4d3c2, (FIXP_DBL)0x0ea96df0,
    (FIXP_DBL)0x0eae00d2, (FIXP_DBL)0x0eb28c7f, (FIXP_DBL)0x0eb7110e,
    (FIXP_DBL)0x0ebb8e96, (FIXP_DBL)0x0ec0052b, (FIXP_DBL)0x0ec474e4,
    (FIXP_DBL)0x0ec8ddd4, (FIXP_DBL)0x0ecd4012, (FIXP_DBL)0x0ed19bb0,
    (FIXP_DBL)0x0ed5f0c4, (FIXP_DBL)0x0eda3f60, (FIXP_DBL)0x0ede8797,
    (FIXP_DBL)0x0ee2c97d, (FIXP_DBL)0x0ee70525, (FIXP_DBL)0x0eeb3a9f,
    (FIXP_DBL)0x0eef69ff, (FIXP_DBL)0x0ef39355, (FIXP_DBL)0x0ef7b6b4,
    (FIXP_DBL)0x0efbd42b, (FIXP_DBL)0x0effebcd, (FIXP_DBL)0x0f03fda9,
    (FIXP_DBL)0x0f0809cf, (FIXP_DBL)0x0f0c1050, (FIXP_DBL)0x0f10113b,
    (FIXP_DBL)0x0f140ca0, (FIXP_DBL)0x0f18028d, (FIXP_DBL)0x0f1bf312,
    (FIXP_DBL)0x0f1fde3d, (FIXP_DBL)0x0f23c41d, (FIXP_DBL)0x0f27a4c0,
    (FIXP_DBL)0x0f2b8034};

#else
#error "ldInt table size too small"

#endif

LNK_SECTION_INITCODE
void InitLdInt() { /* nothing to do! Use preinitialized logarithm table */
}

#if (LD_INT_TAB_LEN != 0)

LNK_SECTION_CODE_L1
FIXP_DBL CalcLdInt(INT i) {
  /* calculates ld(op)/LD_DATA_SCALING */
  /* op is assumed to be an integer value between 1 and LD_INT_TAB_LEN */

  FDK_ASSERT((LD_INT_TAB_LEN > 0) &&
             ((FIXP_DBL)ldIntCoeff[0] ==
              (FIXP_DBL)0x80000001)); /* tab has to be initialized */

  if ((i > 0) && (i < LD_INT_TAB_LEN))
    return ldIntCoeff[i];
  else {
    return (0);
  }
}
#endif /* (LD_INT_TAB_LEN!=0)  */

#if !defined(FUNCTION_schur_div)
/*****************************************************************************

    functionname: schur_div
    description:  delivers op1/op2 with op3-bit accuracy

*****************************************************************************/

FIXP_DBL schur_div(FIXP_DBL num, FIXP_DBL denum, INT count) {
  INT L_num = (LONG)num >> 1;
  INT L_denum = (LONG)denum >> 1;
  INT div = 0;
  INT k = count;

  FDK_ASSERT(num >= (FIXP_DBL)0);
  FDK_ASSERT(denum > (FIXP_DBL)0);
  FDK_ASSERT(num <= denum);

  if (L_num != 0)
    while (--k) {
      div <<= 1;
      L_num <<= 1;
      if (L_num >= L_denum) {
        L_num -= L_denum;
        div++;
      }
    }
  return (FIXP_DBL)(div << (DFRACT_BITS - count));
}

#endif /* !defined(FUNCTION_schur_div) */

#ifndef FUNCTION_fMultNorm
FIXP_DBL fMultNorm(FIXP_DBL f1, FIXP_DBL f2, INT *result_e) {
  INT product = 0;
  INT norm_f1, norm_f2;

  if ((f1 == (FIXP_DBL)0) || (f2 == (FIXP_DBL)0)) {
    *result_e = 0;
    return (FIXP_DBL)0;
  }
  norm_f1 = CountLeadingBits(f1);
  f1 = f1 << norm_f1;
  norm_f2 = CountLeadingBits(f2);
  f2 = f2 << norm_f2;

  if ((f1 == (FIXP_DBL)MINVAL_DBL) && (f2 == (FIXP_DBL)MINVAL_DBL)) {
    product = -((FIXP_DBL)MINVAL_DBL >> 1);
    *result_e = -(norm_f1 + norm_f2 - 1);
  } else {
    product = fMult(f1, f2);
    *result_e = -(norm_f1 + norm_f2);
  }

  return (FIXP_DBL)product;
}
#endif

#ifndef FUNCTION_fDivNorm
FIXP_DBL fDivNorm(FIXP_DBL L_num, FIXP_DBL L_denum, INT *result_e) {
  FIXP_DBL div;
  INT norm_num, norm_den;

  FDK_ASSERT(L_num >= (FIXP_DBL)0);
  FDK_ASSERT(L_denum > (FIXP_DBL)0);

  if (L_num == (FIXP_DBL)0) {
    *result_e = 0;
    return ((FIXP_DBL)0);
  }

  norm_num = CountLeadingBits(L_num);
  L_num = L_num << norm_num;
  L_num = L_num >> 1;
  *result_e = -norm_num + 1;

  norm_den = CountLeadingBits(L_denum);
  L_denum = L_denum << norm_den;
  *result_e -= -norm_den;

  div = schur_div(L_num, L_denum, FRACT_BITS);

  return div;
}
#endif /* !FUNCTION_fDivNorm */

#ifndef FUNCTION_fDivNorm
FIXP_DBL fDivNorm(FIXP_DBL num, FIXP_DBL denom) {
  INT e;
  FIXP_DBL res;

  FDK_ASSERT(denom >= num);

  res = fDivNorm(num, denom, &e);

  /* Avoid overflow since we must output a value with exponent 0
     there is no other choice than saturating to almost 1.0f */
  if (res == (FIXP_DBL)(1 << (DFRACT_BITS - 2)) && e == 1) {
    res = (FIXP_DBL)MAXVAL_DBL;
  } else {
    res = scaleValue(res, e);
  }

  return res;
}
#endif /* !FUNCTION_fDivNorm */

#ifndef FUNCTION_fDivNormSigned
FIXP_DBL fDivNormSigned(FIXP_DBL num, FIXP_DBL denom) {
  INT e;
  FIXP_DBL res;
  int sign;

  if (denom == (FIXP_DBL)0) {
    return (FIXP_DBL)MAXVAL_DBL;
  }

  sign = ((num >= (FIXP_DBL)0) != (denom >= (FIXP_DBL)0));
  res = fDivNormSigned(num, denom, &e);

  /* Saturate since we must output a value with exponent 0 */
  if ((e > 0) && (fAbs(res) >= FL2FXCONST_DBL(0.5))) {
    if (sign) {
      res = (FIXP_DBL)MINVAL_DBL;
    } else {
      res = (FIXP_DBL)MAXVAL_DBL;
    }
  } else {
    res = scaleValue(res, e);
  }

  return res;
}
FIXP_DBL fDivNormSigned(FIXP_DBL L_num, FIXP_DBL L_denum, INT *result_e) {
  FIXP_DBL div;
  INT norm_num, norm_den;
  int sign;

  sign = ((L_num >= (FIXP_DBL)0) != (L_denum >= (FIXP_DBL)0));

  if (L_num == (FIXP_DBL)0) {
    *result_e = 0;
    return ((FIXP_DBL)0);
  }
  if (L_denum == (FIXP_DBL)0) {
    *result_e = 14;
    return ((FIXP_DBL)MAXVAL_DBL);
  }

  norm_num = CountLeadingBits(L_num);
  L_num = L_num << norm_num;
  L_num = L_num >> 2;
  L_num = fAbs(L_num);
  *result_e = -norm_num + 1;

  norm_den = CountLeadingBits(L_denum);
  L_denum = L_denum << norm_den;
  L_denum = L_denum >> 1;
  L_denum = fAbs(L_denum);
  *result_e -= -norm_den;

  div = schur_div(L_num, L_denum, FRACT_BITS);

  if (sign) {
    div = -div;
  }

  return div;
}
#endif /* FUNCTION_fDivNormSigned */

#ifndef FUNCTION_fDivNormHighPrec
FIXP_DBL fDivNormHighPrec(FIXP_DBL num, FIXP_DBL denom, INT *result_e) {
  FIXP_DBL div;
  INT norm_num, norm_den;

  FDK_ASSERT(num >= (FIXP_DBL)0);
  FDK_ASSERT(denom > (FIXP_DBL)0);

  if (num == (FIXP_DBL)0) {
    *result_e = 0;
    return ((FIXP_DBL)0);
  }

  norm_num = CountLeadingBits(num);
  num = num << norm_num;
  num = num >> 1;
  *result_e = -norm_num + 1;

  norm_den = CountLeadingBits(denom);
  denom = denom << norm_den;
  *result_e -= -norm_den;

  div = schur_div(num, denom, 31);
  return div;
}
#endif /* !FUNCTION_fDivNormHighPrec */

#ifndef FUNCTION_fPow
FIXP_DBL f2Pow(const FIXP_DBL exp_m, const INT exp_e, INT *result_e) {
  FIXP_DBL frac_part, result_m;
  INT int_part;

  if (exp_e > 0) {
    INT exp_bits = DFRACT_BITS - 1 - exp_e;
    int_part = exp_m >> exp_bits;
    frac_part = exp_m - (FIXP_DBL)(int_part << exp_bits);
    frac_part = frac_part << exp_e;
  } else {
    int_part = 0;
    frac_part = exp_m >> -exp_e;
  }

  /* Best accuracy is around 0, so try to get there with the fractional part. */
  if (frac_part > FL2FXCONST_DBL(0.5f)) {
    int_part = int_part + 1;
    frac_part = frac_part + FL2FXCONST_DBL(-1.0f);
  }
  if (frac_part < FL2FXCONST_DBL(-0.5f)) {
    int_part = int_part - 1;
    frac_part = -(FL2FXCONST_DBL(-1.0f) - frac_part);
  }

  /* "+ 1" compensates fMultAddDiv2() of the polynomial evaluation below. */
  *result_e = int_part + 1;

  /* Evaluate taylor polynomial which approximates 2^x */
  {
    FIXP_DBL p;

    /* result_m ~= 2^frac_part */
    p = frac_part;
    /* First taylor series coefficient a_0 = 1.0, scaled by 0.5 due to
     * fMultDiv2(). */
    result_m = FL2FXCONST_DBL(1.0f / 2.0f);
    for (INT i = 0; i < POW2_PRECISION; i++) {
      /* next taylor series term: a_i * x^i, x=0 */
      result_m = fMultAddDiv2(result_m, pow2Coeff[i], p);
      p = fMult(p, frac_part);
    }
  }
  return result_m;
}

FIXP_DBL f2Pow(const FIXP_DBL exp_m, const INT exp_e) {
  FIXP_DBL result_m;
  INT result_e;

  result_m = f2Pow(exp_m, exp_e, &result_e);
  result_e = fixMin(DFRACT_BITS - 1, fixMax(-(DFRACT_BITS - 1), result_e));

  return scaleValue(result_m, result_e);
}

FIXP_DBL fPow(FIXP_DBL base_m, INT base_e, FIXP_DBL exp_m, INT exp_e,
              INT *result_e) {
  INT ans_lg2_e, baselg2_e;
  FIXP_DBL base_lg2, ans_lg2, result;

  if (base_m <= (FIXP_DBL)0) {
    result = (FIXP_DBL)0;
    *result_e = 0;
    return result;
  }

  /* Calc log2 of base */
  base_lg2 = fLog2(base_m, base_e, &baselg2_e);

  /* Prepare exp */
  {
    INT leadingBits;

    leadingBits = CountLeadingBits(fAbs(exp_m));
    exp_m = exp_m << leadingBits;
    exp_e -= leadingBits;
  }

  /* Calc base pow exp */
  ans_lg2 = fMult(base_lg2, exp_m);
  ans_lg2_e = exp_e + baselg2_e;

  /* Calc antilog */
  result = f2Pow(ans_lg2, ans_lg2_e, result_e);

  return result;
}

FIXP_DBL fLdPow(FIXP_DBL baseLd_m, INT baseLd_e, FIXP_DBL exp_m, INT exp_e,
                INT *result_e) {
  INT ans_lg2_e;
  FIXP_DBL ans_lg2, result;

  /* Prepare exp */
  {
    INT leadingBits;

    leadingBits = CountLeadingBits(fAbs(exp_m));
    exp_m = exp_m << leadingBits;
    exp_e -= leadingBits;
  }

  /* Calc base pow exp */
  ans_lg2 = fMult(baseLd_m, exp_m);
  ans_lg2_e = exp_e + baseLd_e;

  /* Calc antilog */
  result = f2Pow(ans_lg2, ans_lg2_e, result_e);

  return result;
}

FIXP_DBL fLdPow(FIXP_DBL baseLd_m, INT baseLd_e, FIXP_DBL exp_m, INT exp_e) {
  FIXP_DBL result_m;
  int result_e;

  result_m = fLdPow(baseLd_m, baseLd_e, exp_m, exp_e, &result_e);

  return SATURATE_SHIFT(result_m, -result_e, DFRACT_BITS);
}

FIXP_DBL fPowInt(FIXP_DBL base_m, INT base_e, INT exp, INT *pResult_e) {
  FIXP_DBL result;

  if (exp != 0) {
    INT result_e = 0;

    if (base_m != (FIXP_DBL)0) {
      {
        INT leadingBits;
        leadingBits = CountLeadingBits(base_m);
        base_m <<= leadingBits;
        base_e -= leadingBits;
      }

      result = base_m;

      {
        int i;
        for (i = 1; i < fAbs(exp); i++) {
          result = fMult(result, base_m);
        }
      }

      if (exp < 0) {
        /* 1.0 / ans */
        result = fDivNorm(FL2FXCONST_DBL(0.5f), result, &result_e);
        result_e++;
      } else {
        int ansScale = CountLeadingBits(result);
        result <<= ansScale;
        result_e -= ansScale;
      }

      result_e += exp * base_e;

    } else {
      result = (FIXP_DBL)0;
    }
    *pResult_e = result_e;
  } else {
    result = FL2FXCONST_DBL(0.5f);
    *pResult_e = 1;
  }

  return result;
}
#endif /* FUNCTION_fPow */

#ifndef FUNCTION_fLog2
FIXP_DBL CalcLog2(FIXP_DBL base_m, INT base_e, INT *result_e) {
  return fLog2(base_m, base_e, result_e);
}
#endif /* FUNCTION_fLog2 */

INT fixp_floorToInt(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT floorInt = (INT)(f_inp >> ((DFRACT_BITS - 1) - sf));
  return floorInt;
}

FIXP_DBL fixp_floor(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT floorInt = fixp_floorToInt(f_inp, sf);
  FIXP_DBL f_floor = (FIXP_DBL)(floorInt << ((DFRACT_BITS - 1) - sf));
  return f_floor;
}

INT fixp_ceilToInt(FIXP_DBL f_inp, INT sf)  // sf  mantissaBits left of dot
{
  FDK_ASSERT(sf >= 0);
  INT sx = (DFRACT_BITS - 1) - sf;  // sx  mantissaBits right of dot
  INT inpINT = (INT)f_inp;

  INT mask = (0x1 << sx) - 1;
  INT ceilInt = (INT)(f_inp >> sx);

  if (inpINT & mask) {
    ceilInt++;  // increment only, if there is at least one set mantissaBit
                // right of dot [in inpINT]
  }

  return ceilInt;
}

FIXP_DBL fixp_ceil(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT sx = (DFRACT_BITS - 1) - sf;
  INT ceilInt = fixp_ceilToInt(f_inp, sf);
  ULONG mask = (ULONG)0x1 << (DFRACT_BITS - 1);  // 0x80000000
  ceilInt = ceilInt
            << sx;  // no fract warn bec. shift into saturation done on int side

  if ((f_inp > FL2FXCONST_DBL(0.0f)) && (ceilInt & mask)) {
    --ceilInt;
  }
  FIXP_DBL f_ceil = (FIXP_DBL)ceilInt;

  return f_ceil;
}

/*****************************************************************************
   fixp_truncateToInt()
     Just remove the fractional part which is located right of decimal point
     Same as which is done when a float is casted to (INT) :
     result_INTtype = (INT)b_floatTypeInput;

   returns INT
*****************************************************************************/
INT fixp_truncateToInt(FIXP_DBL f_inp, INT sf)  // sf  mantissaBits left  of dot
                                                // (without sign)  e.g. at width
                                                // 32 this would be [sign]7.
                                                // supposed sf equals 8.
{
  FDK_ASSERT(sf >= 0);
  INT sx = (DFRACT_BITS - 1) - sf;  // sx  mantissaBits right of dot
                                    // at width 32 this would be        .24
                                    // supposed sf equals 8.
  INT fbaccu = (INT)f_inp;
  INT mask = (0x1 << sx);

  if ((fbaccu < 0) && (fbaccu & (mask - 1))) {
    fbaccu = fbaccu + mask;
  }

  fbaccu = fbaccu >> sx;
  return fbaccu;
}

/*****************************************************************************
   fixp_truncate()
     Just remove the fractional part which is located right of decimal point

   returns FIXP_DBL
*****************************************************************************/
FIXP_DBL fixp_truncate(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT truncateInt = fixp_truncateToInt(f_inp, sf);
  FIXP_DBL f_truncate = (FIXP_DBL)(truncateInt << ((DFRACT_BITS - 1) - sf));
  return f_truncate;
}

/*****************************************************************************
  fixp_roundToInt()
    round [typical rounding]

    See fct roundRef() [which is the reference]
  returns INT
*****************************************************************************/
INT fixp_roundToInt(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT sx = DFRACT_BITS - 1 - sf;
  INT inp = (INT)f_inp;
  INT mask1 = (0x1 << (sx - 1));
  INT mask2 = (0x1 << (sx)) - 1;
  INT mask3 = 0x7FFFFFFF;
  INT iam = inp & mask2;
  INT rnd;

  if ((inp < 0) && !(iam == mask1))
    rnd = inp + mask1;
  else if ((inp > 0) && !(inp == mask3))
    rnd = inp + mask1;
  else
    rnd = inp;

  rnd = rnd >> sx;

  if (inp == mask3) rnd++;

  return rnd;
}

/*****************************************************************************
  fixp_round()
    round [typical rounding]

    See fct roundRef() [which is the reference]
  returns FIXP_DBL
*****************************************************************************/
FIXP_DBL fixp_round(FIXP_DBL f_inp, INT sf) {
  FDK_ASSERT(sf >= 0);
  INT sx = DFRACT_BITS - 1 - sf;
  INT r = fixp_roundToInt(f_inp, sf);
  ULONG mask = (ULONG)0x1 << (DFRACT_BITS - 1);  // 0x80000000
  r = r << sx;

  if ((f_inp > FL2FXCONST_DBL(0.0f)) && (r & mask)) {
    --r;
  }

  FIXP_DBL f_round = (FIXP_DBL)r;
  return f_round;
}
