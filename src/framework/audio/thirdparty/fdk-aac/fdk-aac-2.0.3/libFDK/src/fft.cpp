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

/******************* Library for basic calculation routines ********************

   Author(s):   Josef Hoepfl, DSP Solutions

   Description: Fix point FFT

*******************************************************************************/

#include "fft_rad2.h"
#include "FDK_tools_rom.h"

#define W_PiFOURTH STC(0x5a82799a)
//#define W_PiFOURTH ((FIXP_DBL)(0x5a82799a))
#ifndef SUMDIFF_PIFOURTH
#define SUMDIFF_PIFOURTH(diff, sum, a, b) \
  {                                       \
    FIXP_DBL wa, wb;                      \
    wa = fMultDiv2(a, W_PiFOURTH);        \
    wb = fMultDiv2(b, W_PiFOURTH);        \
    diff = wb - wa;                       \
    sum = wb + wa;                        \
  }
#define SUMDIFF_PIFOURTH16(diff, sum, a, b)       \
  {                                               \
    FIXP_SGL wa, wb;                              \
    wa = FX_DBL2FX_SGL(fMultDiv2(a, W_PiFOURTH)); \
    wb = FX_DBL2FX_SGL(fMultDiv2(b, W_PiFOURTH)); \
    diff = wb - wa;                               \
    sum = wb + wa;                                \
  }
#endif

#define SCALEFACTOR2048 10
#define SCALEFACTOR1024 9
#define SCALEFACTOR512 8
#define SCALEFACTOR256 7
#define SCALEFACTOR128 6
#define SCALEFACTOR64 5
#define SCALEFACTOR32 4
#define SCALEFACTOR16 3
#define SCALEFACTOR8 2
#define SCALEFACTOR4 1
#define SCALEFACTOR2 1

#define SCALEFACTOR3 1
#define SCALEFACTOR5 1
#define SCALEFACTOR6 (SCALEFACTOR2 + SCALEFACTOR3 + 2)
#define SCALEFACTOR7 2
#define SCALEFACTOR9 2
#define SCALEFACTOR10 5
#define SCALEFACTOR12 3
#define SCALEFACTOR15 3
#define SCALEFACTOR18 (SCALEFACTOR2 + SCALEFACTOR9 + 2)
#define SCALEFACTOR20 (SCALEFACTOR4 + SCALEFACTOR5 + 2)
#define SCALEFACTOR21 (SCALEFACTOR3 + SCALEFACTOR7 + 2)
#define SCALEFACTOR24 (SCALEFACTOR2 + SCALEFACTOR12 + 2)
#define SCALEFACTOR30 (SCALEFACTOR2 + SCALEFACTOR15 + 2)
#define SCALEFACTOR40 (SCALEFACTOR5 + SCALEFACTOR8 + 2)
#define SCALEFACTOR48 (SCALEFACTOR4 + SCALEFACTOR12 + 2)
#define SCALEFACTOR60 (SCALEFACTOR4 + SCALEFACTOR15 + 2)
#define SCALEFACTOR80 (SCALEFACTOR5 + SCALEFACTOR16 + 2)
#define SCALEFACTOR96 (SCALEFACTOR3 + SCALEFACTOR32 + 2)
#define SCALEFACTOR120 (SCALEFACTOR8 + SCALEFACTOR15 + 2)
#define SCALEFACTOR160 (SCALEFACTOR10 + SCALEFACTOR16 + 2)
#define SCALEFACTOR168 (SCALEFACTOR21 + SCALEFACTOR8 + 2)
#define SCALEFACTOR192 (SCALEFACTOR12 + SCALEFACTOR16 + 2)
#define SCALEFACTOR240 (SCALEFACTOR16 + SCALEFACTOR15 + 2)
#define SCALEFACTOR320 (SCALEFACTOR10 + SCALEFACTOR32 + 2)
#define SCALEFACTOR336 (SCALEFACTOR21 + SCALEFACTOR16 + 2)
#define SCALEFACTOR384 (SCALEFACTOR12 + SCALEFACTOR32 + 2)
#define SCALEFACTOR480 (SCALEFACTOR32 + SCALEFACTOR15 + 2)

#include "fft.h"

#ifndef FUNCTION_fft2

/* Performs the FFT of length 2. Input vector unscaled, output vector scaled
 * with factor 0.5 */
static inline void fft2(FIXP_DBL *RESTRICT pDat) {
  FIXP_DBL r1, i1;
  FIXP_DBL r2, i2;

  /* real part */
  r1 = pDat[2];
  r2 = pDat[0];

  /* imaginary part */
  i1 = pDat[3];
  i2 = pDat[1];

  /* real part */
  pDat[0] = (r2 + r1) >> 1;
  pDat[2] = (r2 - r1) >> 1;

  /* imaginary part */
  pDat[1] = (i2 + i1) >> 1;
  pDat[3] = (i2 - i1) >> 1;
}
#endif /* FUNCTION_fft2 */

#define C31 (STC(0x91261468)) /* FL2FXCONST_DBL(-0.86602540) = -sqrt(3)/2  */

#ifndef FUNCTION_fft3
/* Performs the FFT of length 3 according to the algorithm after winograd. */
static inline void fft3(FIXP_DBL *RESTRICT pDat) {
  FIXP_DBL r1, r2;
  FIXP_DBL s1, s2;
  FIXP_DBL pD;

  /* real part */
  r1 = pDat[2] + pDat[4];
  r2 = fMultDiv2((pDat[2] - pDat[4]), C31);
  pD = pDat[0] >> 1;
  pDat[0] = pD + (r1 >> 1);
  r1 = pD - (r1 >> 2);

  /* imaginary part */
  s1 = pDat[3] + pDat[5];
  s2 = fMultDiv2((pDat[3] - pDat[5]), C31);
  pD = pDat[1] >> 1;
  pDat[1] = pD + (s1 >> 1);
  s1 = pD - (s1 >> 2);

  /* combination */
  pDat[2] = r1 - s2;
  pDat[4] = r1 + s2;
  pDat[3] = s1 + r2;
  pDat[5] = s1 - r2;
}
#endif /* #ifndef FUNCTION_fft3 */

#define F5C(x) STC(x)

#define C51 (F5C(0x79bc3854)) /* FL2FXCONST_DBL( 0.95105652)   */
#define C52 (F5C(0x9d839db0)) /* FL2FXCONST_DBL(-1.53884180/2) */
#define C53 (F5C(0xd18053ce)) /* FL2FXCONST_DBL(-0.36327126)   */
#define C54 (F5C(0x478dde64)) /* FL2FXCONST_DBL( 0.55901699)   */
#define C55 (F5C(0xb0000001)) /* FL2FXCONST_DBL(-1.25/2)       */

/* performs the FFT of length 5 according to the algorithm after winograd */
/* This version works with a prescale of 2 instead of 3 */
static inline void fft5(FIXP_DBL *RESTRICT pDat) {
  FIXP_DBL r1, r2, r3, r4;
  FIXP_DBL s1, s2, s3, s4;
  FIXP_DBL t;

  /* real part */
  r1 = (pDat[2] + pDat[8]) >> 1;
  r4 = (pDat[2] - pDat[8]) >> 1;
  r3 = (pDat[4] + pDat[6]) >> 1;
  r2 = (pDat[4] - pDat[6]) >> 1;
  t = fMult((r1 - r3), C54);
  r1 = r1 + r3;
  pDat[0] = (pDat[0] >> 1) + r1;
  /* Bit shift left because of the constant C55 which was scaled with the factor
     0.5 because of the representation of the values as fracts */
  r1 = pDat[0] + (fMultDiv2(r1, C55) << (2));
  r3 = r1 - t;
  r1 = r1 + t;
  t = fMult((r4 + r2), C51);
  /* Bit shift left because of the constant C55 which was scaled with the factor
     0.5 because of the representation of the values as fracts */
  r4 = t + (fMultDiv2(r4, C52) << (2));
  r2 = t + fMult(r2, C53);

  /* imaginary part */
  s1 = (pDat[3] + pDat[9]) >> 1;
  s4 = (pDat[3] - pDat[9]) >> 1;
  s3 = (pDat[5] + pDat[7]) >> 1;
  s2 = (pDat[5] - pDat[7]) >> 1;
  t = fMult((s1 - s3), C54);
  s1 = s1 + s3;
  pDat[1] = (pDat[1] >> 1) + s1;
  /* Bit shift left because of the constant C55 which was scaled with the factor
     0.5 because of the representation of the values as fracts */
  s1 = pDat[1] + (fMultDiv2(s1, C55) << (2));
  s3 = s1 - t;
  s1 = s1 + t;
  t = fMult((s4 + s2), C51);
  /* Bit shift left because of the constant C55 which was scaled with the factor
     0.5 because of the representation of the values as fracts */
  s4 = t + (fMultDiv2(s4, C52) << (2));
  s2 = t + fMult(s2, C53);

  /* combination */
  pDat[2] = r1 + s2;
  pDat[8] = r1 - s2;
  pDat[4] = r3 - s4;
  pDat[6] = r3 + s4;

  pDat[3] = s1 - r2;
  pDat[9] = s1 + r2;
  pDat[5] = s3 + r4;
  pDat[7] = s3 - r4;
}

#define F5C(x) STC(x)

#define C51 (F5C(0x79bc3854)) /* FL2FXCONST_DBL( 0.95105652)   */
#define C52 (F5C(0x9d839db0)) /* FL2FXCONST_DBL(-1.53884180/2) */
#define C53 (F5C(0xd18053ce)) /* FL2FXCONST_DBL(-0.36327126)   */
#define C54 (F5C(0x478dde64)) /* FL2FXCONST_DBL( 0.55901699)   */
#define C55 (F5C(0xb0000001)) /* FL2FXCONST_DBL(-1.25/2)       */
/**
 * \brief    Function performs a complex 10-point FFT
 *           The FFT is performed inplace. The result of the FFT
 *           is scaled by SCALEFACTOR10 bits.
 *
 *           WOPS FLC version:                    1093 cycles
 *           WOPS with 32x16 bit multiplications:  196 cycles
 *
 * \param    [i/o] re    real input / output
 * \param    [i/o] im    imag input / output
 * \param    [i  ] s     stride real and imag input / output
 *
 * \return   void
 */
static void fft10(FIXP_DBL *x)  // FIXP_DBL *re, FIXP_DBL *im, FIXP_SGL s)
{
  FIXP_DBL t;
  FIXP_DBL x0, x1, x2, x3, x4;
  FIXP_DBL r1, r2, r3, r4;
  FIXP_DBL s1, s2, s3, s4;
  FIXP_DBL y00, y01, y02, y03, y04, y05, y06, y07, y08, y09;
  FIXP_DBL y10, y11, y12, y13, y14, y15, y16, y17, y18, y19;

  const int s = 1;  // stride factor

  /* 2 fft5 stages */

  /* real part */
  x0 = (x[s * 0] >> SCALEFACTOR10);
  x1 = (x[s * 4] >> SCALEFACTOR10);
  x2 = (x[s * 8] >> SCALEFACTOR10);
  x3 = (x[s * 12] >> SCALEFACTOR10);
  x4 = (x[s * 16] >> SCALEFACTOR10);

  r1 = (x3 + x2);
  r4 = (x3 - x2);
  r3 = (x1 + x4);
  r2 = (x1 - x4);
  t = fMult((r1 - r3), C54);
  r1 = (r1 + r3);
  y00 = (x0 + r1);
  r1 = (y00 + ((fMult(r1, C55) << 1)));
  r3 = (r1 - t);
  r1 = (r1 + t);
  t = fMult((r4 + r2), C51);
  r4 = (t + (fMult(r4, C52) << 1));
  r2 = (t + fMult(r2, C53));

  /* imaginary part */
  x0 = (x[s * 0 + 1] >> SCALEFACTOR10);
  x1 = (x[s * 4 + 1] >> SCALEFACTOR10);
  x2 = (x[s * 8 + 1] >> SCALEFACTOR10);
  x3 = (x[s * 12 + 1] >> SCALEFACTOR10);
  x4 = (x[s * 16 + 1] >> SCALEFACTOR10);

  s1 = (x3 + x2);
  s4 = (x3 - x2);
  s3 = (x1 + x4);
  s2 = (x1 - x4);
  t = fMult((s1 - s3), C54);
  s1 = (s1 + s3);
  y01 = (x0 + s1);
  s1 = (y01 + (fMult(s1, C55) << 1));
  s3 = (s1 - t);
  s1 = (s1 + t);
  t = fMult((s4 + s2), C51);
  s4 = (t + (fMult(s4, C52) << 1));
  s2 = (t + fMult(s2, C53));

  /* combination */
  y04 = (r1 + s2);
  y16 = (r1 - s2);
  y08 = (r3 - s4);
  y12 = (r3 + s4);

  y05 = (s1 - r2);
  y17 = (s1 + r2);
  y09 = (s3 + r4);
  y13 = (s3 - r4);

  /* real part */
  x0 = (x[s * 10] >> SCALEFACTOR10);
  x1 = (x[s * 2] >> SCALEFACTOR10);
  x2 = (x[s * 6] >> SCALEFACTOR10);
  x3 = (x[s * 14] >> SCALEFACTOR10);
  x4 = (x[s * 18] >> SCALEFACTOR10);

  r1 = (x1 + x4);
  r4 = (x1 - x4);
  r3 = (x3 + x2);
  r2 = (x3 - x2);
  t = fMult((r1 - r3), C54);
  r1 = (r1 + r3);
  y02 = (x0 + r1);
  r1 = (y02 + ((fMult(r1, C55) << 1)));
  r3 = (r1 - t);
  r1 = (r1 + t);
  t = fMult(((r4 + r2)), C51);
  r4 = (t + (fMult(r4, C52) << 1));
  r2 = (t + fMult(r2, C53));

  /* imaginary part */
  x0 = (x[s * 10 + 1] >> SCALEFACTOR10);
  x1 = (x[s * 2 + 1] >> SCALEFACTOR10);
  x2 = (x[s * 6 + 1] >> SCALEFACTOR10);
  x3 = (x[s * 14 + 1] >> SCALEFACTOR10);
  x4 = (x[s * 18 + 1] >> SCALEFACTOR10);

  s1 = (x1 + x4);
  s4 = (x1 - x4);
  s3 = (x3 + x2);
  s2 = (x3 - x2);
  t = fMult((s1 - s3), C54);
  s1 = (s1 + s3);
  y03 = (x0 + s1);
  s1 = (y03 + (fMult(s1, C55) << 1));
  s3 = (s1 - t);
  s1 = (s1 + t);
  t = fMult((s4 + s2), C51);
  s4 = (t + (fMult(s4, C52) << 1));
  s2 = (t + fMult(s2, C53));

  /* combination */
  y06 = (r1 + s2);
  y18 = (r1 - s2);
  y10 = (r3 - s4);
  y14 = (r3 + s4);

  y07 = (s1 - r2);
  y19 = (s1 + r2);
  y11 = (s3 + r4);
  y15 = (s3 - r4);

  /* 5 fft2 stages */
  x[s * 0] = (y00 + y02);
  x[s * 0 + 1] = (y01 + y03);
  x[s * 10] = (y00 - y02);
  x[s * 10 + 1] = (y01 - y03);

  x[s * 4] = (y04 + y06);
  x[s * 4 + 1] = (y05 + y07);
  x[s * 14] = (y04 - y06);
  x[s * 14 + 1] = (y05 - y07);

  x[s * 8] = (y08 + y10);
  x[s * 8 + 1] = (y09 + y11);
  x[s * 18] = (y08 - y10);
  x[s * 18 + 1] = (y09 - y11);

  x[s * 12] = (y12 + y14);
  x[s * 12 + 1] = (y13 + y15);
  x[s * 2] = (y12 - y14);
  x[s * 2 + 1] = (y13 - y15);

  x[s * 16] = (y16 + y18);
  x[s * 16 + 1] = (y17 + y19);
  x[s * 6] = (y16 - y18);
  x[s * 6 + 1] = (y17 - y19);
}

#ifndef FUNCTION_fft12
#define FUNCTION_fft12

#undef C31
#define C31 (STC(0x91261468)) /* FL2FXCONST_DBL(-0.86602540) = -sqrt(3)/2  */

static inline void fft12(FIXP_DBL *pInput) {
  FIXP_DBL aDst[24];
  FIXP_DBL *pSrc, *pDst;
  int i;

  pSrc = pInput;
  pDst = aDst;
  FIXP_DBL r1, r2, s1, s2, pD;

  /* First 3*2 samples are shifted right by 2 before output */
  r1 = pSrc[8] + pSrc[16];
  r2 = fMultDiv2((pSrc[8] - pSrc[16]), C31);
  pD = pSrc[0] >> 1;
  pDst[0] = (pD + (r1 >> 1)) >> 1;
  r1 = pD - (r1 >> 2);

  /* imaginary part */
  s1 = pSrc[9] + pSrc[17];
  s2 = fMultDiv2((pSrc[9] - pSrc[17]), C31);
  pD = pSrc[1] >> 1;
  pDst[1] = (pD + (s1 >> 1)) >> 1;
  s1 = pD - (s1 >> 2);

  /* combination */
  pDst[2] = (r1 - s2) >> 1;
  pDst[3] = (s1 + r2) >> 1;
  pDst[4] = (r1 + s2) >> 1;
  pDst[5] = (s1 - r2) >> 1;
  pSrc += 2;
  pDst += 6;

  const FIXP_STB *pVecRe = RotVectorReal12;
  const FIXP_STB *pVecIm = RotVectorImag12;
  FIXP_DBL re, im;
  FIXP_STB vre, vim;
  for (i = 0; i < 2; i++) {
    /* sample 0,1 are shifted right by 2 before output */
    /* sample 2,3 4,5 are shifted right by 1 and complex multiplied before
     * output */

    r1 = pSrc[8] + pSrc[16];
    r2 = fMultDiv2((pSrc[8] - pSrc[16]), C31);
    pD = pSrc[0] >> 1;
    pDst[0] = (pD + (r1 >> 1)) >> 1;
    r1 = pD - (r1 >> 2);

    /* imaginary part */
    s1 = pSrc[9] + pSrc[17];
    s2 = fMultDiv2((pSrc[9] - pSrc[17]), C31);
    pD = pSrc[1] >> 1;
    pDst[1] = (pD + (s1 >> 1)) >> 1;
    s1 = pD - (s1 >> 2);

    /* combination */
    re = (r1 - s2) >> 0;
    im = (s1 + r2) >> 0;
    vre = *pVecRe++;
    vim = *pVecIm++;
    cplxMultDiv2(&pDst[3], &pDst[2], im, re, vre, vim);

    re = (r1 + s2) >> 0;
    im = (s1 - r2) >> 0;
    vre = *pVecRe++;
    vim = *pVecIm++;
    cplxMultDiv2(&pDst[5], &pDst[4], im, re, vre, vim);

    pDst += 6;
    pSrc += 2;
  }
  /* sample 0,1 are shifted right by 2 before output */
  /* sample 2,3 is shifted right by 1 and complex multiplied with (0.0,+1.0) */
  /* sample 4,5 is shifted right by 1 and complex multiplied with (-1.0,0.0) */
  r1 = pSrc[8] + pSrc[16];
  r2 = fMultDiv2((pSrc[8] - pSrc[16]), C31);
  pD = pSrc[0] >> 1;
  pDst[0] = (pD + (r1 >> 1)) >> 1;
  r1 = pD - (r1 >> 2);

  /* imaginary part */
  s1 = pSrc[9] + pSrc[17];
  s2 = fMultDiv2((pSrc[9] - pSrc[17]), C31);
  pD = pSrc[1] >> 1;
  pDst[1] = (pD + (s1 >> 1)) >> 1;
  s1 = pD - (s1 >> 2);

  /* combination */
  pDst[2] = (s1 + r2) >> 1;
  pDst[3] = (s2 - r1) >> 1;
  pDst[4] = -((r1 + s2) >> 1);
  pDst[5] = (r2 - s1) >> 1;

  /* Perform 3 times the fft of length 4. The input samples are at the address
  of aDst and the output samples are at the address of pInput. The input vector
  for the fft of length 4 is built of the interleaved samples in aDst, the
  output samples are stored consecutively at the address of pInput.
  */
  pSrc = aDst;
  pDst = pInput;
  for (i = 0; i < 3; i++) {
    /* inline FFT4 merged with incoming resorting loop */
    FIXP_DBL a00, a10, a20, a30, tmp0, tmp1;

    a00 = (pSrc[0] + pSrc[12]) >> 1; /* Re A + Re B */
    a10 = (pSrc[6] + pSrc[18]) >> 1; /* Re C + Re D */
    a20 = (pSrc[1] + pSrc[13]) >> 1; /* Im A + Im B */
    a30 = (pSrc[7] + pSrc[19]) >> 1; /* Im C + Im D */

    pDst[0] = a00 + a10; /* Re A' = Re A + Re B + Re C + Re D */
    pDst[1] = a20 + a30; /* Im A' = Im A + Im B + Im C + Im D */

    tmp0 = a00 - pSrc[12]; /* Re A - Re B */
    tmp1 = a20 - pSrc[13]; /* Im A - Im B */

    pDst[12] = a00 - a10; /* Re C' = Re A + Re B - Re C - Re D */
    pDst[13] = a20 - a30; /* Im C' = Im A + Im B - Im C - Im D */

    a10 = a10 - pSrc[18]; /* Re C - Re D */
    a30 = a30 - pSrc[19]; /* Im C - Im D */

    pDst[6] = tmp0 + a30;  /* Re B' = Re A - Re B + Im C - Im D */
    pDst[18] = tmp0 - a30; /* Re D' = Re A - Re B - Im C + Im D */
    pDst[7] = tmp1 - a10;  /* Im B' = Im A - Im B - Re C + Re D */
    pDst[19] = tmp1 + a10; /* Im D' = Im A - Im B + Re C - Re D */

    pSrc += 2;
    pDst += 2;
  }
}
#endif /* FUNCTION_fft12 */

#ifndef FUNCTION_fft15

#define N3 3
#define N5 5
#define N6 6
#define N15 15

/* Performs the FFT of length 15. It is split into FFTs of length 3 and
 * length 5. */
static inline void fft15(FIXP_DBL *pInput) {
  FIXP_DBL aDst[2 * N15];
  FIXP_DBL aDst1[2 * N15];
  int i, k, l;

  /* Sort input vector for fft's of length 3
  input3(0:2)   = [input(0) input(5) input(10)];
  input3(3:5)   = [input(3) input(8) input(13)];
  input3(6:8)   = [input(6) input(11) input(1)];
  input3(9:11)  = [input(9) input(14) input(4)];
  input3(12:14) = [input(12) input(2) input(7)]; */
  {
    const FIXP_DBL *pSrc = pInput;
    FIXP_DBL *RESTRICT pDst = aDst;
    /* Merge 3 loops into one, skip call of fft3 */
    for (i = 0, l = 0, k = 0; i < N5; i++, k += 6) {
      pDst[k + 0] = pSrc[l];
      pDst[k + 1] = pSrc[l + 1];
      l += 2 * N5;
      if (l >= (2 * N15)) l -= (2 * N15);

      pDst[k + 2] = pSrc[l];
      pDst[k + 3] = pSrc[l + 1];
      l += 2 * N5;
      if (l >= (2 * N15)) l -= (2 * N15);
      pDst[k + 4] = pSrc[l];
      pDst[k + 5] = pSrc[l + 1];
      l += (2 * N5) + (2 * N3);
      if (l >= (2 * N15)) l -= (2 * N15);

      /* fft3 merged with shift right by 2 loop */
      FIXP_DBL r1, r2, r3;
      FIXP_DBL s1, s2;
      /* real part */
      r1 = pDst[k + 2] + pDst[k + 4];
      r2 = fMult((pDst[k + 2] - pDst[k + 4]), C31);
      s1 = pDst[k + 0];
      pDst[k + 0] = (s1 + r1) >> 2;
      r1 = s1 - (r1 >> 1);

      /* imaginary part */
      s1 = pDst[k + 3] + pDst[k + 5];
      s2 = fMult((pDst[k + 3] - pDst[k + 5]), C31);
      r3 = pDst[k + 1];
      pDst[k + 1] = (r3 + s1) >> 2;
      s1 = r3 - (s1 >> 1);

      /* combination */
      pDst[k + 2] = (r1 - s2) >> 2;
      pDst[k + 4] = (r1 + s2) >> 2;
      pDst[k + 3] = (s1 + r2) >> 2;
      pDst[k + 5] = (s1 - r2) >> 2;
    }
  }
  /* Sort input vector for fft's of length 5
  input5(0:4)   = [output3(0) output3(3) output3(6) output3(9) output3(12)];
  input5(5:9)   = [output3(1) output3(4) output3(7) output3(10) output3(13)];
  input5(10:14) = [output3(2) output3(5) output3(8) output3(11) output3(14)]; */
  /* Merge 2 loops into one, brings about 10% */
  {
    const FIXP_DBL *pSrc = aDst;
    FIXP_DBL *RESTRICT pDst = aDst1;
    for (i = 0, l = 0, k = 0; i < N3; i++, k += 10) {
      l = 2 * i;
      pDst[k + 0] = pSrc[l + 0];
      pDst[k + 1] = pSrc[l + 1];
      pDst[k + 2] = pSrc[l + 0 + (2 * N3)];
      pDst[k + 3] = pSrc[l + 1 + (2 * N3)];
      pDst[k + 4] = pSrc[l + 0 + (4 * N3)];
      pDst[k + 5] = pSrc[l + 1 + (4 * N3)];
      pDst[k + 6] = pSrc[l + 0 + (6 * N3)];
      pDst[k + 7] = pSrc[l + 1 + (6 * N3)];
      pDst[k + 8] = pSrc[l + 0 + (8 * N3)];
      pDst[k + 9] = pSrc[l + 1 + (8 * N3)];
      fft5(&pDst[k]);
    }
  }
  /* Sort output vector of length 15
  output = [out5(0)  out5(6)  out5(12) out5(3)  out5(9)
            out5(10) out5(1)  out5(7)  out5(13) out5(4)
            out5(5)  out5(11) out5(2)  out5(8)  out5(14)]; */
  /* optimize clumsy loop, brings about 5% */
  {
    const FIXP_DBL *pSrc = aDst1;
    FIXP_DBL *RESTRICT pDst = pInput;
    for (i = 0, l = 0, k = 0; i < N3; i++, k += 10) {
      pDst[k + 0] = pSrc[l];
      pDst[k + 1] = pSrc[l + 1];
      l += (2 * N6);
      if (l >= (2 * N15)) l -= (2 * N15);
      pDst[k + 2] = pSrc[l];
      pDst[k + 3] = pSrc[l + 1];
      l += (2 * N6);
      if (l >= (2 * N15)) l -= (2 * N15);
      pDst[k + 4] = pSrc[l];
      pDst[k + 5] = pSrc[l + 1];
      l += (2 * N6);
      if (l >= (2 * N15)) l -= (2 * N15);
      pDst[k + 6] = pSrc[l];
      pDst[k + 7] = pSrc[l + 1];
      l += (2 * N6);
      if (l >= (2 * N15)) l -= (2 * N15);
      pDst[k + 8] = pSrc[l];
      pDst[k + 9] = pSrc[l + 1];
      l += 2; /* no modulo check needed, it cannot occur */
    }
  }
}
#endif /* FUNCTION_fft15 */

/*
 Select shift placement.
 Some processors like ARM may shift "for free" in combination with an addition
 or substraction, but others don't so either combining shift with +/- or reduce
 the total amount or shift operations is optimal
 */
#if !defined(__arm__)
#define SHIFT_A >> 1
#define SHIFT_B
#else
#define SHIFT_A
#define SHIFT_B >> 1
#endif

#ifndef FUNCTION_fft_16 /* we check, if fft_16 (FIXP_DBL *) is not yet defined \
                         */

/* This defines prevents this array to be declared twice, if 16-bit fft is
 * enabled too */
#define FUNCTION_DATA_fft_16_w16
static const FIXP_STP fft16_w16[2] = {STCP(0x7641af3d, 0x30fbc54d),
                                      STCP(0x30fbc54d, 0x7641af3d)};

LNK_SECTION_CODE_L1
inline void fft_16(FIXP_DBL *RESTRICT x) {
  FIXP_DBL vr, ur;
  FIXP_DBL vr2, ur2;
  FIXP_DBL vr3, ur3;
  FIXP_DBL vr4, ur4;
  FIXP_DBL vi, ui;
  FIXP_DBL vi2, ui2;
  FIXP_DBL vi3, ui3;

  vr = (x[0] >> 1) + (x[16] >> 1);       /* Re A + Re B */
  ur = (x[1] >> 1) + (x[17] >> 1);       /* Im A + Im B */
  vi = (x[8] SHIFT_A) + (x[24] SHIFT_A); /* Re C + Re D */
  ui = (x[9] SHIFT_A) + (x[25] SHIFT_A); /* Im C + Im D */
  x[0] = vr + (vi SHIFT_B);              /* Re A' = ReA + ReB +ReC + ReD */
  x[1] = ur + (ui SHIFT_B);              /* Im A' = sum of imag values */

  vr2 = (x[4] >> 1) + (x[20] >> 1); /* Re A + Re B */
  ur2 = (x[5] >> 1) + (x[21] >> 1); /* Im A + Im B */

  x[4] = vr - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
  x[5] = ur - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */
  vr -= x[16];              /* Re A - Re B */
  vi = (vi SHIFT_B)-x[24];  /* Re C - Re D */
  ur -= x[17];              /* Im A - Im B */
  ui = (ui SHIFT_B)-x[25];  /* Im C - Im D */

  vr3 = (x[2] >> 1) + (x[18] >> 1); /* Re A + Re B */
  ur3 = (x[3] >> 1) + (x[19] >> 1); /* Im A + Im B */

  x[2] = ui + vr; /* Re B' = Im C - Im D  + Re A - Re B */
  x[3] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */

  vr4 = (x[6] >> 1) + (x[22] >> 1); /* Re A + Re B */
  ur4 = (x[7] >> 1) + (x[23] >> 1); /* Im A + Im B */

  x[6] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
  x[7] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

  vi2 = (x[12] SHIFT_A) + (x[28] SHIFT_A); /* Re C + Re D */
  ui2 = (x[13] SHIFT_A) + (x[29] SHIFT_A); /* Im C + Im D */
  x[8] = vr2 + (vi2 SHIFT_B);              /* Re A' = ReA + ReB +ReC + ReD */
  x[9] = ur2 + (ui2 SHIFT_B);              /* Im A' = sum of imag values */
  x[12] = vr2 - (vi2 SHIFT_B);             /* Re C' = -(ReC+ReD) + (ReA+ReB) */
  x[13] = ur2 - (ui2 SHIFT_B);             /* Im C' = -Im C -Im D +Im A +Im B */
  vr2 -= x[20];                            /* Re A - Re B */
  ur2 -= x[21];                            /* Im A - Im B */
  vi2 = (vi2 SHIFT_B)-x[28];               /* Re C - Re D */
  ui2 = (ui2 SHIFT_B)-x[29];               /* Im C - Im D */

  vi = (x[10] SHIFT_A) + (x[26] SHIFT_A); /* Re C + Re D */
  ui = (x[11] SHIFT_A) + (x[27] SHIFT_A); /* Im C + Im D */

  x[10] = ui2 + vr2; /* Re B' = Im C - Im D  + Re A - Re B */
  x[11] = ur2 - vi2; /* Im B'= -Re C + Re D + Im A - Im B */

  vi3 = (x[14] SHIFT_A) + (x[30] SHIFT_A); /* Re C + Re D */
  ui3 = (x[15] SHIFT_A) + (x[31] SHIFT_A); /* Im C + Im D */

  x[14] = vr2 - ui2; /* Re D' = -Im C + Im D + Re A - Re B */
  x[15] = vi2 + ur2; /* Im D'= Re C - Re D + Im A - Im B */

  x[16] = vr3 + (vi SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
  x[17] = ur3 + (ui SHIFT_B); /* Im A' = sum of imag values */
  x[20] = vr3 - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
  x[21] = ur3 - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */
  vr3 -= x[18];               /* Re A - Re B */
  ur3 -= x[19];               /* Im A - Im B */
  vi = (vi SHIFT_B)-x[26];    /* Re C - Re D */
  ui = (ui SHIFT_B)-x[27];    /* Im C - Im D */
  x[18] = ui + vr3;           /* Re B' = Im C - Im D  + Re A - Re B */
  x[19] = ur3 - vi;           /* Im B'= -Re C + Re D + Im A - Im B */

  x[24] = vr4 + (vi3 SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
  x[28] = vr4 - (vi3 SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
  x[25] = ur4 + (ui3 SHIFT_B); /* Im A' = sum of imag values */
  x[29] = ur4 - (ui3 SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */
  vr4 -= x[22];                /* Re A - Re B */
  ur4 -= x[23];                /* Im A - Im B */

  x[22] = vr3 - ui; /* Re D' = -Im C + Im D + Re A - Re B */
  x[23] = vi + ur3; /* Im D'= Re C - Re D + Im A - Im B */

  vi3 = (vi3 SHIFT_B)-x[30]; /* Re C - Re D */
  ui3 = (ui3 SHIFT_B)-x[31]; /* Im C - Im D */
  x[26] = ui3 + vr4;         /* Re B' = Im C - Im D  + Re A - Re B */
  x[30] = vr4 - ui3;         /* Re D' = -Im C + Im D + Re A - Re B */
  x[27] = ur4 - vi3;         /* Im B'= -Re C + Re D + Im A - Im B */
  x[31] = vi3 + ur4;         /* Im D'= Re C - Re D + Im A - Im B */

  // xt1 =  0
  // xt2 =  8
  vr = x[8];
  vi = x[9];
  ur = x[0] >> 1;
  ui = x[1] >> 1;
  x[0] = ur + (vr >> 1);
  x[1] = ui + (vi >> 1);
  x[8] = ur - (vr >> 1);
  x[9] = ui - (vi >> 1);

  // xt1 =  4
  // xt2 = 12
  vr = x[13];
  vi = x[12];
  ur = x[4] >> 1;
  ui = x[5] >> 1;
  x[4] = ur + (vr >> 1);
  x[5] = ui - (vi >> 1);
  x[12] = ur - (vr >> 1);
  x[13] = ui + (vi >> 1);

  // xt1 = 16
  // xt2 = 24
  vr = x[24];
  vi = x[25];
  ur = x[16] >> 1;
  ui = x[17] >> 1;
  x[16] = ur + (vr >> 1);
  x[17] = ui + (vi >> 1);
  x[24] = ur - (vr >> 1);
  x[25] = ui - (vi >> 1);

  // xt1 = 20
  // xt2 = 28
  vr = x[29];
  vi = x[28];
  ur = x[20] >> 1;
  ui = x[21] >> 1;
  x[20] = ur + (vr >> 1);
  x[21] = ui - (vi >> 1);
  x[28] = ur - (vr >> 1);
  x[29] = ui + (vi >> 1);

  // xt1 =  2
  // xt2 = 10
  SUMDIFF_PIFOURTH(vi, vr, x[10], x[11])
  // vr = fMultDiv2((x[11] + x[10]),W_PiFOURTH);
  // vi = fMultDiv2((x[11] - x[10]),W_PiFOURTH);
  ur = x[2];
  ui = x[3];
  x[2] = (ur >> 1) + vr;
  x[3] = (ui >> 1) + vi;
  x[10] = (ur >> 1) - vr;
  x[11] = (ui >> 1) - vi;

  // xt1 =  6
  // xt2 = 14
  SUMDIFF_PIFOURTH(vr, vi, x[14], x[15])
  ur = x[6];
  ui = x[7];
  x[6] = (ur >> 1) + vr;
  x[7] = (ui >> 1) - vi;
  x[14] = (ur >> 1) - vr;
  x[15] = (ui >> 1) + vi;

  // xt1 = 18
  // xt2 = 26
  SUMDIFF_PIFOURTH(vi, vr, x[26], x[27])
  ur = x[18];
  ui = x[19];
  x[18] = (ur >> 1) + vr;
  x[19] = (ui >> 1) + vi;
  x[26] = (ur >> 1) - vr;
  x[27] = (ui >> 1) - vi;

  // xt1 = 22
  // xt2 = 30
  SUMDIFF_PIFOURTH(vr, vi, x[30], x[31])
  ur = x[22];
  ui = x[23];
  x[22] = (ur >> 1) + vr;
  x[23] = (ui >> 1) - vi;
  x[30] = (ur >> 1) - vr;
  x[31] = (ui >> 1) + vi;

  // xt1 =  0
  // xt2 = 16
  vr = x[16];
  vi = x[17];
  ur = x[0] >> 1;
  ui = x[1] >> 1;
  x[0] = ur + (vr >> 1);
  x[1] = ui + (vi >> 1);
  x[16] = ur - (vr >> 1);
  x[17] = ui - (vi >> 1);

  // xt1 =  8
  // xt2 = 24
  vi = x[24];
  vr = x[25];
  ur = x[8] >> 1;
  ui = x[9] >> 1;
  x[8] = ur + (vr >> 1);
  x[9] = ui - (vi >> 1);
  x[24] = ur - (vr >> 1);
  x[25] = ui + (vi >> 1);

  // xt1 =  2
  // xt2 = 18
  cplxMultDiv2(&vi, &vr, x[19], x[18], fft16_w16[0]);
  ur = x[2];
  ui = x[3];
  x[2] = (ur >> 1) + vr;
  x[3] = (ui >> 1) + vi;
  x[18] = (ur >> 1) - vr;
  x[19] = (ui >> 1) - vi;

  // xt1 = 10
  // xt2 = 26
  cplxMultDiv2(&vr, &vi, x[27], x[26], fft16_w16[0]);
  ur = x[10];
  ui = x[11];
  x[10] = (ur >> 1) + vr;
  x[11] = (ui >> 1) - vi;
  x[26] = (ur >> 1) - vr;
  x[27] = (ui >> 1) + vi;

  // xt1 =  4
  // xt2 = 20
  SUMDIFF_PIFOURTH(vi, vr, x[20], x[21])
  ur = x[4];
  ui = x[5];
  x[4] = (ur >> 1) + vr;
  x[5] = (ui >> 1) + vi;
  x[20] = (ur >> 1) - vr;
  x[21] = (ui >> 1) - vi;

  // xt1 = 12
  // xt2 = 28
  SUMDIFF_PIFOURTH(vr, vi, x[28], x[29])
  ur = x[12];
  ui = x[13];
  x[12] = (ur >> 1) + vr;
  x[13] = (ui >> 1) - vi;
  x[28] = (ur >> 1) - vr;
  x[29] = (ui >> 1) + vi;

  // xt1 =  6
  // xt2 = 22
  cplxMultDiv2(&vi, &vr, x[23], x[22], fft16_w16[1]);
  ur = x[6];
  ui = x[7];
  x[6] = (ur >> 1) + vr;
  x[7] = (ui >> 1) + vi;
  x[22] = (ur >> 1) - vr;
  x[23] = (ui >> 1) - vi;

  // xt1 = 14
  // xt2 = 30
  cplxMultDiv2(&vr, &vi, x[31], x[30], fft16_w16[1]);
  ur = x[14];
  ui = x[15];
  x[14] = (ur >> 1) + vr;
  x[15] = (ui >> 1) - vi;
  x[30] = (ur >> 1) - vr;
  x[31] = (ui >> 1) + vi;
}
#endif /* FUNCTION_fft_16 */

#ifndef FUNCTION_fft_32
static const FIXP_STP fft32_w32[6] = {
    STCP(0x7641af3d, 0x30fbc54d), STCP(0x30fbc54d, 0x7641af3d),
    STCP(0x7d8a5f40, 0x18f8b83c), STCP(0x6a6d98a4, 0x471cece7),
    STCP(0x471cece7, 0x6a6d98a4), STCP(0x18f8b83c, 0x7d8a5f40)};
#define W_PiFOURTH STC(0x5a82799a)

LNK_SECTION_CODE_L1
inline void fft_32(FIXP_DBL *const _x) {
  /*
   * 1+2 stage radix 4
   */

  /////////////////////////////////////////////////////////////////////////////////////////
  {
    FIXP_DBL *const x = _x;
    FIXP_DBL vi, ui;
    FIXP_DBL vi2, ui2;
    FIXP_DBL vi3, ui3;
    FIXP_DBL vr, ur;
    FIXP_DBL vr2, ur2;
    FIXP_DBL vr3, ur3;
    FIXP_DBL vr4, ur4;

    // i = 0
    vr = (x[0] + x[32]) >> 1;     /* Re A + Re B */
    ur = (x[1] + x[33]) >> 1;     /* Im A + Im B */
    vi = (x[16] + x[48]) SHIFT_A; /* Re C + Re D */
    ui = (x[17] + x[49]) SHIFT_A; /* Im C + Im D */

    x[0] = vr + (vi SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[1] = ur + (ui SHIFT_B); /* Im A' = sum of imag values */

    vr2 = (x[4] + x[36]) >> 1; /* Re A + Re B */
    ur2 = (x[5] + x[37]) >> 1; /* Im A + Im B */

    x[4] = vr - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[5] = ur - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[32];             /* Re A - Re B */
    ur -= x[33];             /* Im A - Im B */
    vi = (vi SHIFT_B)-x[48]; /* Re C - Re D */
    ui = (ui SHIFT_B)-x[49]; /* Im C - Im D */

    vr3 = (x[2] + x[34]) >> 1; /* Re A + Re B */
    ur3 = (x[3] + x[35]) >> 1; /* Im A + Im B */

    x[2] = ui + vr; /* Re B' = Im C - Im D  + Re A - Re B */
    x[3] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */

    vr4 = (x[6] + x[38]) >> 1; /* Re A + Re B */
    ur4 = (x[7] + x[39]) >> 1; /* Im A + Im B */

    x[6] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    x[7] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

    // i=16
    vi = (x[20] + x[52]) SHIFT_A; /* Re C + Re D */
    ui = (x[21] + x[53]) SHIFT_A; /* Im C + Im D */

    x[16] = vr2 + (vi SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[17] = ur2 + (ui SHIFT_B); /* Im A' = sum of imag values */
    x[20] = vr2 - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[21] = ur2 - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr2 -= x[36];            /* Re A - Re B */
    ur2 -= x[37];            /* Im A - Im B */
    vi = (vi SHIFT_B)-x[52]; /* Re C - Re D */
    ui = (ui SHIFT_B)-x[53]; /* Im C - Im D */

    vi2 = (x[18] + x[50]) SHIFT_A; /* Re C + Re D */
    ui2 = (x[19] + x[51]) SHIFT_A; /* Im C + Im D */

    x[18] = ui + vr2; /* Re B' = Im C - Im D  + Re A - Re B */
    x[19] = ur2 - vi; /* Im B'= -Re C + Re D + Im A - Im B */

    vi3 = (x[22] + x[54]) SHIFT_A; /* Re C + Re D */
    ui3 = (x[23] + x[55]) SHIFT_A; /* Im C + Im D */

    x[22] = vr2 - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    x[23] = vi + ur2; /* Im D'= Re C - Re D + Im A - Im B */

    // i = 32

    x[32] = vr3 + (vi2 SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[33] = ur3 + (ui2 SHIFT_B); /* Im A' = sum of imag values */
    x[36] = vr3 - (vi2 SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[37] = ur3 - (ui2 SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr3 -= x[34];              /* Re A - Re B */
    ur3 -= x[35];              /* Im A - Im B */
    vi2 = (vi2 SHIFT_B)-x[50]; /* Re C - Re D */
    ui2 = (ui2 SHIFT_B)-x[51]; /* Im C - Im D */

    x[34] = ui2 + vr3; /* Re B' = Im C - Im D  + Re A - Re B */
    x[35] = ur3 - vi2; /* Im B'= -Re C + Re D + Im A - Im B */

    // i=48

    x[48] = vr4 + (vi3 SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[52] = vr4 - (vi3 SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[49] = ur4 + (ui3 SHIFT_B); /* Im A' = sum of imag values */
    x[53] = ur4 - (ui3 SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr4 -= x[38]; /* Re A - Re B */
    ur4 -= x[39]; /* Im A - Im B */

    x[38] = vr3 - ui2; /* Re D' = -Im C + Im D + Re A - Re B */
    x[39] = vi2 + ur3; /* Im D'= Re C - Re D + Im A - Im B */

    vi3 = (vi3 SHIFT_B)-x[54]; /* Re C - Re D */
    ui3 = (ui3 SHIFT_B)-x[55]; /* Im C - Im D */

    x[50] = ui3 + vr4; /* Re B' = Im C - Im D  + Re A - Re B */
    x[54] = vr4 - ui3; /* Re D' = -Im C + Im D + Re A - Re B */
    x[51] = ur4 - vi3; /* Im B'= -Re C + Re D + Im A - Im B */
    x[55] = vi3 + ur4; /* Im D'= Re C - Re D + Im A - Im B */

    // i=8
    vr = (x[8] + x[40]) >> 1;     /* Re A + Re B */
    ur = (x[9] + x[41]) >> 1;     /* Im A + Im B */
    vi = (x[24] + x[56]) SHIFT_A; /* Re C + Re D */
    ui = (x[25] + x[57]) SHIFT_A; /* Im C + Im D */

    x[8] = vr + (vi SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[9] = ur + (ui SHIFT_B); /* Im A' = sum of imag values */

    vr2 = (x[12] + x[44]) >> 1; /* Re A + Re B */
    ur2 = (x[13] + x[45]) >> 1; /* Im A + Im B */

    x[12] = vr - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[13] = ur - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr -= x[40];             /* Re A - Re B */
    ur -= x[41];             /* Im A - Im B */
    vi = (vi SHIFT_B)-x[56]; /* Re C - Re D */
    ui = (ui SHIFT_B)-x[57]; /* Im C - Im D */

    vr3 = (x[10] + x[42]) >> 1; /* Re A + Re B */
    ur3 = (x[11] + x[43]) >> 1; /* Im A + Im B */

    x[10] = ui + vr; /* Re B' = Im C - Im D  + Re A - Re B */
    x[11] = ur - vi; /* Im B'= -Re C + Re D + Im A - Im B */

    vr4 = (x[14] + x[46]) >> 1; /* Re A + Re B */
    ur4 = (x[15] + x[47]) >> 1; /* Im A + Im B */

    x[14] = vr - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    x[15] = vi + ur; /* Im D'= Re C - Re D + Im A - Im B */

    // i=24
    vi = (x[28] + x[60]) SHIFT_A; /* Re C + Re D */
    ui = (x[29] + x[61]) SHIFT_A; /* Im C + Im D */

    x[24] = vr2 + (vi SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[28] = vr2 - (vi SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[25] = ur2 + (ui SHIFT_B); /* Im A' = sum of imag values */
    x[29] = ur2 - (ui SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr2 -= x[44];            /* Re A - Re B */
    ur2 -= x[45];            /* Im A - Im B */
    vi = (vi SHIFT_B)-x[60]; /* Re C - Re D */
    ui = (ui SHIFT_B)-x[61]; /* Im C - Im D */

    vi2 = (x[26] + x[58]) SHIFT_A; /* Re C + Re D */
    ui2 = (x[27] + x[59]) SHIFT_A; /* Im C + Im D */

    x[26] = ui + vr2; /* Re B' = Im C - Im D  + Re A - Re B */
    x[27] = ur2 - vi; /* Im B'= -Re C + Re D + Im A - Im B */

    vi3 = (x[30] + x[62]) SHIFT_A; /* Re C + Re D */
    ui3 = (x[31] + x[63]) SHIFT_A; /* Im C + Im D */

    x[30] = vr2 - ui; /* Re D' = -Im C + Im D + Re A - Re B */
    x[31] = vi + ur2; /* Im D'= Re C - Re D + Im A - Im B */

    // i=40

    x[40] = vr3 + (vi2 SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[44] = vr3 - (vi2 SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[41] = ur3 + (ui2 SHIFT_B); /* Im A' = sum of imag values */
    x[45] = ur3 - (ui2 SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr3 -= x[42];              /* Re A - Re B */
    ur3 -= x[43];              /* Im A - Im B */
    vi2 = (vi2 SHIFT_B)-x[58]; /* Re C - Re D */
    ui2 = (ui2 SHIFT_B)-x[59]; /* Im C - Im D */

    x[42] = ui2 + vr3; /* Re B' = Im C - Im D  + Re A - Re B */
    x[43] = ur3 - vi2; /* Im B'= -Re C + Re D + Im A - Im B */

    // i=56

    x[56] = vr4 + (vi3 SHIFT_B); /* Re A' = ReA + ReB +ReC + ReD */
    x[60] = vr4 - (vi3 SHIFT_B); /* Re C' = -(ReC+ReD) + (ReA+ReB) */
    x[57] = ur4 + (ui3 SHIFT_B); /* Im A' = sum of imag values */
    x[61] = ur4 - (ui3 SHIFT_B); /* Im C' = -Im C -Im D +Im A +Im B */

    vr4 -= x[46]; /* Re A - Re B */
    ur4 -= x[47]; /* Im A - Im B */

    x[46] = vr3 - ui2; /* Re D' = -Im C + Im D + Re A - Re B */
    x[47] = vi2 + ur3; /* Im D'= Re C - Re D + Im A - Im B */

    vi3 = (vi3 SHIFT_B)-x[62]; /* Re C - Re D */
    ui3 = (ui3 SHIFT_B)-x[63]; /* Im C - Im D */

    x[58] = ui3 + vr4; /* Re B' = Im C - Im D  + Re A - Re B */
    x[62] = vr4 - ui3; /* Re D' = -Im C + Im D + Re A - Re B */
    x[59] = ur4 - vi3; /* Im B'= -Re C + Re D + Im A - Im B */
    x[63] = vi3 + ur4; /* Im D'= Re C - Re D + Im A - Im B */
  }

  {
    FIXP_DBL *xt = _x;

    int j = 4;
    do {
      FIXP_DBL vi, ui, vr, ur;

      vr = xt[8];
      vi = xt[9];
      ur = xt[0] >> 1;
      ui = xt[1] >> 1;
      xt[0] = ur + (vr >> 1);
      xt[1] = ui + (vi >> 1);
      xt[8] = ur - (vr >> 1);
      xt[9] = ui - (vi >> 1);

      vr = xt[13];
      vi = xt[12];
      ur = xt[4] >> 1;
      ui = xt[5] >> 1;
      xt[4] = ur + (vr >> 1);
      xt[5] = ui - (vi >> 1);
      xt[12] = ur - (vr >> 1);
      xt[13] = ui + (vi >> 1);

      SUMDIFF_PIFOURTH(vi, vr, xt[10], xt[11])
      ur = xt[2];
      ui = xt[3];
      xt[2] = (ur >> 1) + vr;
      xt[3] = (ui >> 1) + vi;
      xt[10] = (ur >> 1) - vr;
      xt[11] = (ui >> 1) - vi;

      SUMDIFF_PIFOURTH(vr, vi, xt[14], xt[15])
      ur = xt[6];
      ui = xt[7];

      xt[6] = (ur >> 1) + vr;
      xt[7] = (ui >> 1) - vi;
      xt[14] = (ur >> 1) - vr;
      xt[15] = (ui >> 1) + vi;
      xt += 16;
    } while (--j != 0);
  }

  {
    FIXP_DBL *const x = _x;
    FIXP_DBL vi, ui, vr, ur;

    vr = x[16];
    vi = x[17];
    ur = x[0] >> 1;
    ui = x[1] >> 1;
    x[0] = ur + (vr >> 1);
    x[1] = ui + (vi >> 1);
    x[16] = ur - (vr >> 1);
    x[17] = ui - (vi >> 1);

    vi = x[24];
    vr = x[25];
    ur = x[8] >> 1;
    ui = x[9] >> 1;
    x[8] = ur + (vr >> 1);
    x[9] = ui - (vi >> 1);
    x[24] = ur - (vr >> 1);
    x[25] = ui + (vi >> 1);

    vr = x[48];
    vi = x[49];
    ur = x[32] >> 1;
    ui = x[33] >> 1;
    x[32] = ur + (vr >> 1);
    x[33] = ui + (vi >> 1);
    x[48] = ur - (vr >> 1);
    x[49] = ui - (vi >> 1);

    vi = x[56];
    vr = x[57];
    ur = x[40] >> 1;
    ui = x[41] >> 1;
    x[40] = ur + (vr >> 1);
    x[41] = ui - (vi >> 1);
    x[56] = ur - (vr >> 1);
    x[57] = ui + (vi >> 1);

    cplxMultDiv2(&vi, &vr, x[19], x[18], fft32_w32[0]);
    ur = x[2];
    ui = x[3];
    x[2] = (ur >> 1) + vr;
    x[3] = (ui >> 1) + vi;
    x[18] = (ur >> 1) - vr;
    x[19] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[27], x[26], fft32_w32[0]);
    ur = x[10];
    ui = x[11];
    x[10] = (ur >> 1) + vr;
    x[11] = (ui >> 1) - vi;
    x[26] = (ur >> 1) - vr;
    x[27] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[51], x[50], fft32_w32[0]);
    ur = x[34];
    ui = x[35];
    x[34] = (ur >> 1) + vr;
    x[35] = (ui >> 1) + vi;
    x[50] = (ur >> 1) - vr;
    x[51] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[59], x[58], fft32_w32[0]);
    ur = x[42];
    ui = x[43];
    x[42] = (ur >> 1) + vr;
    x[43] = (ui >> 1) - vi;
    x[58] = (ur >> 1) - vr;
    x[59] = (ui >> 1) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[20], x[21])
    ur = x[4];
    ui = x[5];
    x[4] = (ur >> 1) + vr;
    x[5] = (ui >> 1) + vi;
    x[20] = (ur >> 1) - vr;
    x[21] = (ui >> 1) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[28], x[29])
    ur = x[12];
    ui = x[13];
    x[12] = (ur >> 1) + vr;
    x[13] = (ui >> 1) - vi;
    x[28] = (ur >> 1) - vr;
    x[29] = (ui >> 1) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[52], x[53])
    ur = x[36];
    ui = x[37];
    x[36] = (ur >> 1) + vr;
    x[37] = (ui >> 1) + vi;
    x[52] = (ur >> 1) - vr;
    x[53] = (ui >> 1) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[60], x[61])
    ur = x[44];
    ui = x[45];
    x[44] = (ur >> 1) + vr;
    x[45] = (ui >> 1) - vi;
    x[60] = (ur >> 1) - vr;
    x[61] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[23], x[22], fft32_w32[1]);
    ur = x[6];
    ui = x[7];
    x[6] = (ur >> 1) + vr;
    x[7] = (ui >> 1) + vi;
    x[22] = (ur >> 1) - vr;
    x[23] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[31], x[30], fft32_w32[1]);
    ur = x[14];
    ui = x[15];
    x[14] = (ur >> 1) + vr;
    x[15] = (ui >> 1) - vi;
    x[30] = (ur >> 1) - vr;
    x[31] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[55], x[54], fft32_w32[1]);
    ur = x[38];
    ui = x[39];
    x[38] = (ur >> 1) + vr;
    x[39] = (ui >> 1) + vi;
    x[54] = (ur >> 1) - vr;
    x[55] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[63], x[62], fft32_w32[1]);
    ur = x[46];
    ui = x[47];

    x[46] = (ur >> 1) + vr;
    x[47] = (ui >> 1) - vi;
    x[62] = (ur >> 1) - vr;
    x[63] = (ui >> 1) + vi;

    vr = x[32];
    vi = x[33];
    ur = x[0] >> 1;
    ui = x[1] >> 1;
    x[0] = ur + (vr >> 1);
    x[1] = ui + (vi >> 1);
    x[32] = ur - (vr >> 1);
    x[33] = ui - (vi >> 1);

    vi = x[48];
    vr = x[49];
    ur = x[16] >> 1;
    ui = x[17] >> 1;
    x[16] = ur + (vr >> 1);
    x[17] = ui - (vi >> 1);
    x[48] = ur - (vr >> 1);
    x[49] = ui + (vi >> 1);

    cplxMultDiv2(&vi, &vr, x[35], x[34], fft32_w32[2]);
    ur = x[2];
    ui = x[3];
    x[2] = (ur >> 1) + vr;
    x[3] = (ui >> 1) + vi;
    x[34] = (ur >> 1) - vr;
    x[35] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[51], x[50], fft32_w32[2]);
    ur = x[18];
    ui = x[19];
    x[18] = (ur >> 1) + vr;
    x[19] = (ui >> 1) - vi;
    x[50] = (ur >> 1) - vr;
    x[51] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[37], x[36], fft32_w32[0]);
    ur = x[4];
    ui = x[5];
    x[4] = (ur >> 1) + vr;
    x[5] = (ui >> 1) + vi;
    x[36] = (ur >> 1) - vr;
    x[37] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[53], x[52], fft32_w32[0]);
    ur = x[20];
    ui = x[21];
    x[20] = (ur >> 1) + vr;
    x[21] = (ui >> 1) - vi;
    x[52] = (ur >> 1) - vr;
    x[53] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[39], x[38], fft32_w32[3]);
    ur = x[6];
    ui = x[7];
    x[6] = (ur >> 1) + vr;
    x[7] = (ui >> 1) + vi;
    x[38] = (ur >> 1) - vr;
    x[39] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[55], x[54], fft32_w32[3]);
    ur = x[22];
    ui = x[23];
    x[22] = (ur >> 1) + vr;
    x[23] = (ui >> 1) - vi;
    x[54] = (ur >> 1) - vr;
    x[55] = (ui >> 1) + vi;

    SUMDIFF_PIFOURTH(vi, vr, x[40], x[41])
    ur = x[8];
    ui = x[9];
    x[8] = (ur >> 1) + vr;
    x[9] = (ui >> 1) + vi;
    x[40] = (ur >> 1) - vr;
    x[41] = (ui >> 1) - vi;

    SUMDIFF_PIFOURTH(vr, vi, x[56], x[57])
    ur = x[24];
    ui = x[25];
    x[24] = (ur >> 1) + vr;
    x[25] = (ui >> 1) - vi;
    x[56] = (ur >> 1) - vr;
    x[57] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[43], x[42], fft32_w32[4]);
    ur = x[10];
    ui = x[11];

    x[10] = (ur >> 1) + vr;
    x[11] = (ui >> 1) + vi;
    x[42] = (ur >> 1) - vr;
    x[43] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[59], x[58], fft32_w32[4]);
    ur = x[26];
    ui = x[27];
    x[26] = (ur >> 1) + vr;
    x[27] = (ui >> 1) - vi;
    x[58] = (ur >> 1) - vr;
    x[59] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[45], x[44], fft32_w32[1]);
    ur = x[12];
    ui = x[13];
    x[12] = (ur >> 1) + vr;
    x[13] = (ui >> 1) + vi;
    x[44] = (ur >> 1) - vr;
    x[45] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[61], x[60], fft32_w32[1]);
    ur = x[28];
    ui = x[29];
    x[28] = (ur >> 1) + vr;
    x[29] = (ui >> 1) - vi;
    x[60] = (ur >> 1) - vr;
    x[61] = (ui >> 1) + vi;

    cplxMultDiv2(&vi, &vr, x[47], x[46], fft32_w32[5]);
    ur = x[14];
    ui = x[15];
    x[14] = (ur >> 1) + vr;
    x[15] = (ui >> 1) + vi;
    x[46] = (ur >> 1) - vr;
    x[47] = (ui >> 1) - vi;

    cplxMultDiv2(&vr, &vi, x[63], x[62], fft32_w32[5]);
    ur = x[30];
    ui = x[31];
    x[30] = (ur >> 1) + vr;
    x[31] = (ui >> 1) - vi;
    x[62] = (ur >> 1) - vr;
    x[63] = (ui >> 1) + vi;
  }
}
#endif /* #ifndef FUNCTION_fft_32 */

/**
 * \brief Apply rotation vectors to a data buffer.
 * \param cl length of each row of input data.
 * \param l total length of input data.
 * \param pVecRe real part of rotation coefficient vector.
 * \param pVecIm imaginary part of rotation coefficient vector.
 */

/*
   This defines patches each inaccurate 0x7FFF i.e. 0.9999 and uses 0x8000
   (-1.0) instead. At the end, the sign of the result is inverted
*/
#define noFFT_APPLY_ROT_VECTOR_HQ

#ifndef FUNCTION_fft_apply_rot_vector__FIXP_DBL
static inline void fft_apply_rot_vector(FIXP_DBL *RESTRICT pData, const int cl,
                                        const int l, const FIXP_STB *pVecRe,
                                        const FIXP_STB *pVecIm) {
  FIXP_DBL re, im;
  FIXP_STB vre, vim;

  int i, c;

  for (i = 0; i < cl; i++) {
    re = pData[2 * i];
    im = pData[2 * i + 1];

    pData[2 * i] = re >> 2;     /* * 0.25 */
    pData[2 * i + 1] = im >> 2; /* * 0.25 */
  }
  for (; i < l; i += cl) {
    re = pData[2 * i];
    im = pData[2 * i + 1];

    pData[2 * i] = re >> 2;     /* * 0.25 */
    pData[2 * i + 1] = im >> 2; /* * 0.25 */

    for (c = i + 1; c < i + cl; c++) {
      re = pData[2 * c] >> 1;
      im = pData[2 * c + 1] >> 1;
      vre = *pVecRe++;
      vim = *pVecIm++;

      cplxMultDiv2(&pData[2 * c + 1], &pData[2 * c], im, re, vre, vim);
    }
  }
}
#endif /* FUNCTION_fft_apply_rot_vector__FIXP_DBL */

/* select either switch case of function pointer. */
//#define FFT_TWO_STAGE_SWITCH_CASE
#ifndef FUNCTION_fftN2_func
static inline void fftN2_func(FIXP_DBL *pInput, const int length,
                              const int dim1, const int dim2,
                              void (*const fft1)(FIXP_DBL *),
                              void (*const fft2)(FIXP_DBL *),
                              const FIXP_STB *RotVectorReal,
                              const FIXP_STB *RotVectorImag, FIXP_DBL *aDst,
                              FIXP_DBL *aDst2) {
  /* The real part of the input samples are at the addresses with even indices
  and the imaginary part of the input samples are at the addresses with odd
  indices. The output samples are stored at the address of pInput
  */
  FIXP_DBL *pSrc, *pDst, *pDstOut;
  int i;

  FDK_ASSERT(length == dim1 * dim2);

  /* Perform dim2 times the fft of length dim1. The input samples are at the
  address of pSrc and the output samples are at the address of pDst. The input
  vector for the fft of length dim1 is built of the interleaved samples in pSrc,
  the output samples are stored consecutively.
  */
  pSrc = pInput;
  pDst = aDst;
  for (i = 0; i < dim2; i++) {
    for (int j = 0; j < dim1; j++) {
      pDst[2 * j] = pSrc[2 * j * dim2];
      pDst[2 * j + 1] = pSrc[2 * j * dim2 + 1];
    }

      /* fft of size dim1 */
#ifndef FFT_TWO_STAGE_SWITCH_CASE
    fft1(pDst);
#else
    switch (dim1) {
      case 2:
        fft2(pDst);
        break;
      case 3:
        fft3(pDst);
        break;
      case 4:
        fft_4(pDst);
        break;
      /* case 5: fft5(pDst); break; */
      /* case 8: fft_8(pDst); break; */
      case 12:
        fft12(pDst);
        break;
      /* case 15: fft15(pDst); break; */
      case 16:
        fft_16(pDst);
        break;
      case 32:
        fft_32(pDst);
        break;
        /*case 64: fft_64(pDst); break;*/
        /* case 128: fft_128(pDst); break; */
    }
#endif
    pSrc += 2;
    pDst = pDst + 2 * dim1;
  }

  /* Perform the modulation of the output of the fft of length dim1 */
  pSrc = aDst;
  fft_apply_rot_vector(pSrc, dim1, length, RotVectorReal, RotVectorImag);

  /* Perform dim1 times the fft of length dim2. The input samples are at the
  address of aDst and the output samples are at the address of pInput. The input
  vector for the fft of length dim2 is built of the interleaved samples in aDst,
  the output samples are stored consecutively at the address of pInput.
  */
  pSrc = aDst;
  pDst = aDst2;
  pDstOut = pInput;
  for (i = 0; i < dim1; i++) {
    for (int j = 0; j < dim2; j++) {
      pDst[2 * j] = pSrc[2 * j * dim1];
      pDst[2 * j + 1] = pSrc[2 * j * dim1 + 1];
    }

#ifndef FFT_TWO_STAGE_SWITCH_CASE
    fft2(pDst);
#else
    switch (dim2) {
      case 4:
        fft_4(pDst);
        break;
      case 9:
        fft9(pDst);
        break;
      case 12:
        fft12(pDst);
        break;
      case 15:
        fft15(pDst);
        break;
      case 16:
        fft_16(pDst);
        break;
      case 32:
        fft_32(pDst);
        break;
    }
#endif

    for (int j = 0; j < dim2; j++) {
      pDstOut[2 * j * dim1] = pDst[2 * j];
      pDstOut[2 * j * dim1 + 1] = pDst[2 * j + 1];
    }
    pSrc += 2;
    pDstOut += 2;
  }
}
#endif /* FUNCTION_fftN2_function */

#define fftN2(DATA_TYPE, pInput, length, dim1, dim2, fft_func1, fft_func2, \
              RotVectorReal, RotVectorImag)                                \
  {                                                                        \
    C_AALLOC_SCRATCH_START(aDst, DATA_TYPE, 2 * length)                    \
    C_AALLOC_SCRATCH_START(aDst2, DATA_TYPE, 2 * dim2)                     \
    fftN2_func(pInput, length, dim1, dim2, fft_func1, fft_func2,           \
               RotVectorReal, RotVectorImag, aDst, aDst2);                 \
    C_AALLOC_SCRATCH_END(aDst2, DATA_TYPE, 2 * dim2)                       \
    C_AALLOC_SCRATCH_END(aDst, DATA_TYPE, 2 * length)                      \
  }

  /*!
   *
   *  \brief  complex FFT of length 12,18,24,30,48,60,96, 192, 240, 384, 480
   *  \param  pInput contains the input signal prescaled right by 2
   *          pInput contains the output signal scaled by SCALEFACTOR<#length>
   *          The output signal does not have any fixed headroom
   *  \return void
   *
   */

#ifndef FUNCTION_fft6
static inline void fft6(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 6, 2, 3, fft2, fft3, RotVectorReal6, RotVectorImag6);
}
#endif /* #ifndef FUNCTION_fft6 */

#ifndef FUNCTION_fft12
static inline void fft12(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 12, 3, 4, fft3, fft_4, RotVectorReal12,
        RotVectorImag12); /* 16,58 */
}
#endif /* #ifndef FUNCTION_fft12 */

#ifndef FUNCTION_fft20
static inline void fft20(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 20, 4, 5, fft_4, fft5, RotVectorReal20,
        RotVectorImag20);
}
#endif /* FUNCTION_fft20 */

static inline void fft24(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 24, 2, 12, fft2, fft12, RotVectorReal24,
        RotVectorImag24); /* 16,73 */
}

static inline void fft48(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 48, 4, 12, fft_4, fft12, RotVectorReal48,
        RotVectorImag48); /* 16,32 */
}

#ifndef FUNCTION_fft60
static inline void fft60(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 60, 4, 15, fft_4, fft15, RotVectorReal60,
        RotVectorImag60); /* 15,51 */
}
#endif /* FUNCTION_fft60 */

#ifndef FUNCTION_fft80
static inline void fft80(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 80, 5, 16, fft5, fft_16, RotVectorReal80,
        RotVectorImag80); /*  */
}
#endif

#ifndef FUNCTION_fft96
static inline void fft96(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 96, 3, 32, fft3, fft_32, RotVectorReal96,
        RotVectorImag96); /* 15,47 */
}
#endif /* FUNCTION_fft96*/

#ifndef FUNCTION_fft120
static inline void fft120(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 120, 8, 15, fft_8, fft15, RotVectorReal120,
        RotVectorImag120);
}
#endif /* FUNCTION_fft120 */

#ifndef FUNCTION_fft192
static inline void fft192(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 192, 16, 12, fft_16, fft12, RotVectorReal192,
        RotVectorImag192); /* 15,50 */
}
#endif

#ifndef FUNCTION_fft240
static inline void fft240(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 240, 16, 15, fft_16, fft15, RotVectorReal240,
        RotVectorImag240); /* 15.44 */
}
#endif

#ifndef FUNCTION_fft384
static inline void fft384(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 384, 12, 32, fft12, fft_32, RotVectorReal384,
        RotVectorImag384); /* 16.02 */
}
#endif /* FUNCTION_fft384 */

#ifndef FUNCTION_fft480
static inline void fft480(FIXP_DBL *pInput) {
  fftN2(FIXP_DBL, pInput, 480, 32, 15, fft_32, fft15, RotVectorReal480,
        RotVectorImag480); /* 15.84 */
}
#endif /* FUNCTION_fft480 */

void fft(int length, FIXP_DBL *pInput, INT *pScalefactor) {
  /* Ensure, that the io-ptr is always (at least 8-byte) aligned */
  C_ALLOC_ALIGNED_CHECK(pInput);

  if (length == 32) {
    fft_32(pInput);
    *pScalefactor += SCALEFACTOR32;
  } else {
    switch (length) {
      case 16:
        fft_16(pInput);
        *pScalefactor += SCALEFACTOR16;
        break;
      case 8:
        fft_8(pInput);
        *pScalefactor += SCALEFACTOR8;
        break;
      case 2:
        fft2(pInput);
        *pScalefactor += SCALEFACTOR2;
        break;
      case 3:
        fft3(pInput);
        *pScalefactor += SCALEFACTOR3;
        break;
      case 4:
        fft_4(pInput);
        *pScalefactor += SCALEFACTOR4;
        break;
      case 5:
        fft5(pInput);
        *pScalefactor += SCALEFACTOR5;
        break;
      case 6:
        fft6(pInput);
        *pScalefactor += SCALEFACTOR6;
        break;
      case 10:
        fft10(pInput);
        *pScalefactor += SCALEFACTOR10;
        break;
      case 12:
        fft12(pInput);
        *pScalefactor += SCALEFACTOR12;
        break;
      case 15:
        fft15(pInput);
        *pScalefactor += SCALEFACTOR15;
        break;
      case 20:
        fft20(pInput);
        *pScalefactor += SCALEFACTOR20;
        break;
      case 24:
        fft24(pInput);
        *pScalefactor += SCALEFACTOR24;
        break;
      case 48:
        fft48(pInput);
        *pScalefactor += SCALEFACTOR48;
        break;
      case 60:
        fft60(pInput);
        *pScalefactor += SCALEFACTOR60;
        break;
      case 64:
        dit_fft(pInput, 6, SineTable512, 512);
        *pScalefactor += SCALEFACTOR64;
        break;
      case 80:
        fft80(pInput);
        *pScalefactor += SCALEFACTOR80;
        break;
      case 96:
        fft96(pInput);
        *pScalefactor += SCALEFACTOR96;
        break;
      case 120:
        fft120(pInput);
        *pScalefactor += SCALEFACTOR120;
        break;
      case 128:
        dit_fft(pInput, 7, SineTable512, 512);
        *pScalefactor += SCALEFACTOR128;
        break;
      case 192:
        fft192(pInput);
        *pScalefactor += SCALEFACTOR192;
        break;
      case 240:
        fft240(pInput);
        *pScalefactor += SCALEFACTOR240;
        break;
      case 256:
        dit_fft(pInput, 8, SineTable512, 512);
        *pScalefactor += SCALEFACTOR256;
        break;
      case 384:
        fft384(pInput);
        *pScalefactor += SCALEFACTOR384;
        break;
      case 480:
        fft480(pInput);
        *pScalefactor += SCALEFACTOR480;
        break;
      case 512:
        dit_fft(pInput, 9, SineTable512, 512);
        *pScalefactor += SCALEFACTOR512;
        break;
      default:
        FDK_ASSERT(0); /* FFT length not supported! */
        break;
    }
  }
}

void ifft(int length, FIXP_DBL *pInput, INT *scalefactor) {
  switch (length) {
    default:
      FDK_ASSERT(0); /* IFFT length not supported! */
      break;
  }
}
