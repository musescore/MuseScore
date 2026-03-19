/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2021 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Fast FFT routines prototypes
  \author Fabian Haussel
*/

#include "hbe.h"
#include "qmf.h"
#include "env_extr.h"

#define HBE_MAX_QMF_BANDS (40)

#define HBE_MAX_OUT_SLOTS (11)

#define QMF_WIN_LEN                                                          \
  (12 + 6 - 4 - 1) /* 6 subband slots extra delay to align with HQ - 4 slots \
                      to compensate for critical sampling delay - 1 slot to  \
                      align critical sampling exactly (w additional time     \
                      domain delay)*/

#ifndef PI
#define PI 3.14159265358979323846
#endif

static const int xProducts[MAX_STRETCH_HBE - 1] = {
    1, 1, 1}; /* Cross products on(1)/off(0) for T=2,3,4. */
static const int startSubband2kL[33] = {
    0, 0, 0, 0, 0, 0, 0,  2,  2,  2,  4,  4,  4,  4,  4,  6, 6,
    6, 8, 8, 8, 8, 8, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12};

static const int pmin = 12;

static const FIXP_DBL hintReal_F[4][3] = {
    {FL2FXCONST_DBL(0.39840335f), FL2FXCONST_DBL(0.39840335f),
     FL2FXCONST_DBL(-0.39840335f)},
    {FL2FXCONST_DBL(0.39840335f), FL2FXCONST_DBL(-0.39840335f),
     FL2FXCONST_DBL(-0.39840335f)},
    {FL2FXCONST_DBL(-0.39840335f), FL2FXCONST_DBL(-0.39840335f),
     FL2FXCONST_DBL(0.39840335f)},
    {FL2FXCONST_DBL(-0.39840335f), FL2FXCONST_DBL(0.39840335f),
     FL2FXCONST_DBL(0.39840335f)}};

static const FIXP_DBL factors[4] = {
    FL2FXCONST_DBL(0.39840335f), FL2FXCONST_DBL(-0.39840335f),
    FL2FXCONST_DBL(-0.39840335f), FL2FXCONST_DBL(0.39840335f)};

#define PSCALE 32

static const FIXP_DBL p_F[128] = {FL2FXCONST_DBL(0.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(1.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(2.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(3.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(4.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(5.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(6.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(7.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(8.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(9.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(10.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(11.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(12.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(13.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(14.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(15.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(16.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(17.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(18.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(19.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(20.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(21.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(22.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(23.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(24.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(25.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(26.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(27.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(28.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(29.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(30.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(31.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(32.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(33.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(34.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(35.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(36.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(37.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(38.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(39.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(40.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(41.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(42.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(43.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(44.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(45.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(46.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(47.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(48.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(49.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(50.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(51.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(52.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(53.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(54.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(55.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(56.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(57.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(58.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(59.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(60.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(61.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(62.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(63.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(64.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(65.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(66.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(67.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(68.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(69.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(70.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(71.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(72.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(73.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(74.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(75.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(76.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(77.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(78.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(79.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(80.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(81.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(82.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(83.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(84.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(85.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(86.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(87.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(88.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(89.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(90.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(91.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(92.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(93.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(94.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(95.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(96.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(97.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(98.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(99.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(100.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(101.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(102.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(103.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(104.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(105.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(106.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(107.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(108.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(109.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(110.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(111.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(112.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(113.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(114.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(115.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(116.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(117.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(118.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(119.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(120.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(121.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(122.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(123.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(124.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(125.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(126.f / (PSCALE * 12.f)),
                                  FL2FXCONST_DBL(127.f / (PSCALE * 12.f))};

static const FIXP_DBL band_F[64] = {
    FL2FXCONST_DBL((0.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((1.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((2.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((3.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((4.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((5.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((6.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((7.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((8.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((9.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((10.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((11.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((12.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((13.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((14.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((15.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((16.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((17.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((18.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((19.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((20.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((21.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((22.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((23.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((24.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((25.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((26.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((27.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((28.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((29.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((30.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((31.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((32.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((33.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((34.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((35.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((36.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((37.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((38.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((39.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((40.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((41.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((42.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((43.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((44.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((45.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((46.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((47.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((48.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((49.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((50.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((51.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((52.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((53.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((54.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((55.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((56.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((57.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((58.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((59.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((60.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((61.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((62.f * 2.f + 1) / (PSCALE << 2)),
    FL2FXCONST_DBL((63.f * 2.f + 1) / (PSCALE << 2))};

static const FIXP_DBL tr_str[3] = {FL2FXCONST_DBL(1.f / 4.f),
                                   FL2FXCONST_DBL(2.f / 4.f),
                                   FL2FXCONST_DBL(3.f / 4.f)};

static const FIXP_DBL stretchfac[3] = {FL2FXCONST_DBL(1.f / 2.f),
                                       FL2FXCONST_DBL(1.f / 3.f),
                                       FL2FXCONST_DBL(1.f / 4.f)};

static const FIXP_DBL cos_F[64] = {
    26353028,   -79043208,   131685776,  -184244944,  236697216,  -289006912,
    341142496,  -393072608,  444773984,  -496191392,  547325824,  -598114752,
    648559104,  -698597248,  748230016,  -797411904,  846083200,  -894275136,
    941928192,  -989013760,  1035474624, -1081340672, 1126555136, -1171063296,
    1214893696, -1257992192, 1300332544, -1341889408, 1382612736, -1422503808,
    1461586944, -1499741440, 1537039104, -1573364864, 1608743808, -1643196672,
    1676617344, -1709028992, 1740450560, -1770784896, 1800089472, -1828273536,
    1855357440, -1881356288, 1906190080, -1929876608, 1952428928, -1973777664,
    1993962880, -2012922240, 2030670208, -2047216000, 2062508288, -2076559488,
    2089376128, -2100932224, 2111196800, -2120214784, 2127953792, -2134394368,
    2139565056, -2143444864, 2146026624, -2147321856};

static const FIXP_DBL twiddle[121] = {1073741824,
                                      1071442860,
                                      1064555814,
                                      1053110176,
                                      1037154959,
                                      1016758484,
                                      992008094,
                                      963009773,
                                      929887697,
                                      892783698,
                                      851856663,
                                      807281846,
                                      759250125,
                                      707967178,
                                      653652607,
                                      596538995,
                                      536870912,
                                      474903865,
                                      410903207,
                                      345142998,
                                      277904834,
                                      209476638,
                                      140151432,
                                      70226075,
                                      0,
                                      -70226075,
                                      -140151432,
                                      -209476638,
                                      -277904834,
                                      -345142998,
                                      -410903207,
                                      -474903865,
                                      -536870912,
                                      -596538995,
                                      -653652607,
                                      -707967178,
                                      -759250125,
                                      -807281846,
                                      -851856663,
                                      -892783698,
                                      -929887697,
                                      -963009773,
                                      -992008094,
                                      -1016758484,
                                      -1037154959,
                                      -1053110176,
                                      -1064555814,
                                      -1071442860,
                                      -1073741824,
                                      -1071442860,
                                      -1064555814,
                                      -1053110176,
                                      -1037154959,
                                      -1016758484,
                                      -992008094,
                                      -963009773,
                                      -929887697,
                                      -892783698,
                                      -851856663,
                                      -807281846,
                                      -759250125,
                                      -707967178,
                                      -653652607,
                                      -596538995,
                                      -536870912,
                                      -474903865,
                                      -410903207,
                                      -345142998,
                                      -277904834,
                                      -209476638,
                                      -140151432,
                                      -70226075,
                                      0,
                                      70226075,
                                      140151432,
                                      209476638,
                                      277904834,
                                      345142998,
                                      410903207,
                                      474903865,
                                      536870912,
                                      596538995,
                                      653652607,
                                      707967178,
                                      759250125,
                                      807281846,
                                      851856663,
                                      892783698,
                                      929887697,
                                      963009773,
                                      992008094,
                                      1016758484,
                                      1037154959,
                                      1053110176,
                                      1064555814,
                                      1071442860,
                                      1073741824,
                                      1071442860,
                                      1064555814,
                                      1053110176,
                                      1037154959,
                                      1016758484,
                                      992008094,
                                      963009773,
                                      929887697,
                                      892783698,
                                      851856663,
                                      807281846,
                                      759250125,
                                      707967178,
                                      653652607,
                                      596538995,
                                      536870912,
                                      474903865,
                                      410903207,
                                      345142998,
                                      277904834,
                                      209476638,
                                      140151432,
                                      70226075,
                                      0};

#if FIXP_QTW == FIXP_SGL
#define HTW(x) (x)
#else
#define HTW(x) FX_DBL2FX_QTW(FX_SGL2FX_DBL((const FIXP_SGL)x))
#endif

static const FIXP_QTW post_twiddle_cos_8[8] = {
    HTW(-1606),  HTW(4756),  HTW(-7723),  HTW(10394),
    HTW(-12665), HTW(14449), HTW(-15679), HTW(16305)};

static const FIXP_QTW post_twiddle_cos_16[16] = {
    HTW(-804),   HTW(2404),  HTW(-3981),  HTW(5520),  HTW(-7005),  HTW(8423),
    HTW(-9760),  HTW(11003), HTW(-12140), HTW(13160), HTW(-14053), HTW(14811),
    HTW(-15426), HTW(15893), HTW(-16207), HTW(16364)};

static const FIXP_QTW post_twiddle_cos_24[24] = {
    HTW(-536),   HTW(1606),  HTW(-2669),  HTW(3720),  HTW(-4756),  HTW(5771),
    HTW(-6762),  HTW(7723),  HTW(-8652),  HTW(9543),  HTW(-10394), HTW(11200),
    HTW(-11958), HTW(12665), HTW(-13318), HTW(13913), HTW(-14449), HTW(14924),
    HTW(-15334), HTW(15679), HTW(-15956), HTW(16165), HTW(-16305), HTW(16375)};

static const FIXP_QTW post_twiddle_cos_32[32] = {
    HTW(-402),   HTW(1205),  HTW(-2006),  HTW(2801),  HTW(-3590),  HTW(4370),
    HTW(-5139),  HTW(5897),  HTW(-6639),  HTW(7366),  HTW(-8076),  HTW(8765),
    HTW(-9434),  HTW(10080), HTW(-10702), HTW(11297), HTW(-11866), HTW(12406),
    HTW(-12916), HTW(13395), HTW(-13842), HTW(14256), HTW(-14635), HTW(14978),
    HTW(-15286), HTW(15557), HTW(-15791), HTW(15986), HTW(-16143), HTW(16261),
    HTW(-16340), HTW(16379)};

static const FIXP_QTW post_twiddle_cos_40[40] = {
    HTW(-322),   HTW(965),   HTW(-1606),  HTW(2245),  HTW(-2880),  HTW(3511),
    HTW(-4137),  HTW(4756),  HTW(-5368),  HTW(5971),  HTW(-6566),  HTW(7150),
    HTW(-7723),  HTW(8285),  HTW(-8833),  HTW(9368),  HTW(-9889),  HTW(10394),
    HTW(-10883), HTW(11356), HTW(-11810), HTW(12247), HTW(-12665), HTW(13063),
    HTW(-13441), HTW(13799), HTW(-14135), HTW(14449), HTW(-14741), HTW(15011),
    HTW(-15257), HTW(15480), HTW(-15679), HTW(15853), HTW(-16003), HTW(16129),
    HTW(-16229), HTW(16305), HTW(-16356), HTW(16381)};

static const FIXP_QTW post_twiddle_sin_8[8] = {
    HTW(16305), HTW(-15679), HTW(14449), HTW(-12665),
    HTW(10394), HTW(-7723),  HTW(4756),  HTW(-1606)};

static const FIXP_QTW post_twiddle_sin_16[16] = {
    HTW(16364), HTW(-16207), HTW(15893), HTW(-15426), HTW(14811), HTW(-14053),
    HTW(13160), HTW(-12140), HTW(11003), HTW(-9760),  HTW(8423),  HTW(-7005),
    HTW(5520),  HTW(-3981),  HTW(2404),  HTW(-804)};

static const FIXP_QTW post_twiddle_sin_24[24] = {
    HTW(16375), HTW(-16305), HTW(16165), HTW(-15956), HTW(15679), HTW(-15334),
    HTW(14924), HTW(-14449), HTW(13913), HTW(-13318), HTW(12665), HTW(-11958),
    HTW(11200), HTW(-10394), HTW(9543),  HTW(-8652),  HTW(7723),  HTW(-6762),
    HTW(5771),  HTW(-4756),  HTW(3720),  HTW(-2669),  HTW(1606),  HTW(-536)};

static const FIXP_QTW post_twiddle_sin_32[32] = {
    HTW(16379), HTW(-16340), HTW(16261), HTW(-16143), HTW(15986), HTW(-15791),
    HTW(15557), HTW(-15286), HTW(14978), HTW(-14635), HTW(14256), HTW(-13842),
    HTW(13395), HTW(-12916), HTW(12406), HTW(-11866), HTW(11297), HTW(-10702),
    HTW(10080), HTW(-9434),  HTW(8765),  HTW(-8076),  HTW(7366),  HTW(-6639),
    HTW(5897),  HTW(-5139),  HTW(4370),  HTW(-3590),  HTW(2801),  HTW(-2006),
    HTW(1205),  HTW(-402)};

static const FIXP_QTW post_twiddle_sin_40[40] = {
    HTW(16381), HTW(-16356), HTW(16305), HTW(-16229), HTW(16129), HTW(-16003),
    HTW(15853), HTW(-15679), HTW(15480), HTW(-15257), HTW(15011), HTW(-14741),
    HTW(14449), HTW(-14135), HTW(13799), HTW(-13441), HTW(13063), HTW(-12665),
    HTW(12247), HTW(-11810), HTW(11356), HTW(-10883), HTW(10394), HTW(-9889),
    HTW(9368),  HTW(-8833),  HTW(8285),  HTW(-7723),  HTW(7150),  HTW(-6566),
    HTW(5971),  HTW(-5368),  HTW(4756),  HTW(-4137),  HTW(3511),  HTW(-2880),
    HTW(2245),  HTW(-1606),  HTW(965),   HTW(-322)};

static const FIXP_DBL preModCos[32] = {
    -749875776, 786681536,   711263552,  -821592064,  -670937792, 854523392,
    628995648,  -885396032,  -585538240, 914135680,   540670208,  -940673088,
    -494499680, 964944384,   447137824,  -986891008,  -398698816, 1006460096,
    349299264,  -1023604544, -299058240, 1038283072,  248096752,  -1050460288,
    -196537584, 1060106816,  144504928,  -1067199488, -92124160,  1071721152,
    39521456,   -1073660992};

static const FIXP_DBL preModSin[32] = {
    768510144,   730789760,  -804379072,  -691308864, 838310208,   650162560,
    -870221760,  -607449920, 900036928,   563273856,  -927683776,  -517740896,
    953095808,   470960608,  -976211712,  -423045728, 996975808,   374111712,
    -1015338112, -324276416, 1031254400,  273659904,  -1044686336, -222384144,
    1055601472,  170572640,  -1063973632, -118350192, 1069782528,  65842640,
    -1073014208, -13176464};

/* The cube root function */
/*****************************************************************************

    functionname: invCubeRootNorm2
    description:  delivers 1/cuberoot(op) in Q1.31 format and modified exponent

*****************************************************************************/
#define CUBE_ROOT_BITS 7
#define CUBE_ROOT_VALUES (128 + 2)
#define CUBE_ROOT_BITS_MASK 0x7f
#define CUBE_ROOT_FRACT_BITS_MASK 0x007FFFFF
/* Inverse cube root table for operands running from 0.5 to 1.0 */
/* (INT) (1.0/cuberoot((op)));                    */
/* Implicit exponent is 1.                        */

LNK_SECTION_CONSTDATA
static const FIXP_DBL invCubeRootTab[CUBE_ROOT_VALUES] = {
    (0x50a28be6), (0x506d1172), (0x503823c4), (0x5003c05a), (0x4fcfe4c0),
    (0x4f9c8e92), (0x4f69bb7d), (0x4f37693b), (0x4f059594), (0x4ed43e5f),
    (0x4ea36181), (0x4e72fcea), (0x4e430e98), (0x4e139495), (0x4de48cf5),
    (0x4db5f5db), (0x4d87cd73), (0x4d5a11f2), (0x4d2cc19c), (0x4cffdabb),
    (0x4cd35ba4), (0x4ca742b7), (0x4c7b8e5c), (0x4c503d05), (0x4c254d2a),
    (0x4bfabd50), (0x4bd08c00), (0x4ba6b7cd), (0x4b7d3f53), (0x4b542134),
    (0x4b2b5c18), (0x4b02eeb1), (0x4adad7b8), (0x4ab315ea), (0x4a8ba80d),
    (0x4a648cec), (0x4a3dc35b), (0x4a174a30), (0x49f1204a), (0x49cb448d),
    (0x49a5b5e2), (0x49807339), (0x495b7b86), (0x4936cdc2), (0x491268ec),
    (0x48ee4c08), (0x48ca761f), (0x48a6e63e), (0x48839b76), (0x486094de),
    (0x483dd190), (0x481b50ad), (0x47f91156), (0x47d712b3), (0x47b553f0),
    (0x4793d43c), (0x477292c9), (0x47518ece), (0x4730c785), (0x47103c2d),
    (0x46efec06), (0x46cfd655), (0x46affa61), (0x46905777), (0x4670ece4),
    (0x4651b9f9), (0x4632be0b), (0x4613f871), (0x45f56885), (0x45d70da5),
    (0x45b8e72f), (0x459af487), (0x457d3511), (0x455fa835), (0x45424d5d),
    (0x452523f6), (0x45082b6e), (0x44eb6337), (0x44cecac5), (0x44b2618d),
    (0x44962708), (0x447a1ab1), (0x445e3c02), (0x44428a7c), (0x4427059e),
    (0x440bacec), (0x43f07fe9), (0x43d57e1c), (0x43baa70e), (0x439ffa48),
    (0x43857757), (0x436b1dc8), (0x4350ed2b), (0x4336e511), (0x431d050c),
    (0x43034cb2), (0x42e9bb98), (0x42d05156), (0x42b70d85), (0x429defc0),
    (0x4284f7a2), (0x426c24cb), (0x425376d8), (0x423aed6a), (0x42228823),
    (0x420a46a6), (0x41f22898), (0x41da2d9f), (0x41c25561), (0x41aa9f86),
    (0x41930bba), (0x417b99a5), (0x416448f5), (0x414d1956), (0x41360a76),
    (0x411f1c06), (0x41084db5), (0x40f19f35), (0x40db1039), (0x40c4a074),
    (0x40ae4f9b), (0x40981d64), (0x40820985), (0x406c13b6), (0x40563bb1),
    (0x4040812e), (0x402ae3e7), (0x40156399), (0x40000000), (0x3FEAB8D9)};
/*  n.a.  */
static const FIXP_DBL invCubeRootCorrection[3] = {0x40000000, 0x50A28BE6,
                                                  0x6597FA95};

/*****************************************************************************
 * \brief calculate 1.0/cube_root(op), op contains mantissa and exponent
 * \param op_m: (i) mantissa of operand, must not be zero (0x0000.0000) or
 * negative
 * \param op_e: (i) pointer to the exponent of the operand (must be initialized)
 * and .. (o) pointer to the exponent of the result
 * \return:     (o) mantissa of the result
 * \description:
 *  This routine calculates the cube root of the input operand, that is
 *  given with its mantissa in Q31 format (FIXP_DBL) and its exponent (INT).
 *  The resulting mantissa is returned in format Q31. The exponent (*op_e)
 *  is modified accordingly. It is not assured, that the result is fully
 * left-aligned but assumed to have not more than 2 bits headroom. There is one
 * macro to activate the use of this algorithm: FUNCTION_invCubeRootNorm2 By
 * means of activating the macro INVCUBEROOTNORM2_LINEAR_INTERPOLATE_HQ, a
 * slightly higher precision is reachable (by default, not active). For DEBUG
 * purpose only: a FDK_ASSERT macro validates, if the input mantissa is greater
 * zero.
 *
 */
static
#ifdef __arm__
    FIXP_DBL FDK_FORCEINLINE
    invCubeRootNorm2(FIXP_DBL op_m, INT* op_e)
#else
    FIXP_DBL
    invCubeRootNorm2(FIXP_DBL op_m, INT* op_e)
#endif
{
  FDK_ASSERT(op_m > FIXP_DBL(0));

  /* normalize input, calculate shift value */
  INT exponent = (INT)fNormz(op_m) - 1;
  op_m <<= exponent;

  INT index = (INT)(op_m >> (DFRACT_BITS - 1 - (CUBE_ROOT_BITS + 1))) &
              CUBE_ROOT_BITS_MASK;
  FIXP_DBL fract = (FIXP_DBL)(((INT)op_m & CUBE_ROOT_FRACT_BITS_MASK)
                              << (CUBE_ROOT_BITS + 1));
  FIXP_DBL diff = invCubeRootTab[index + 1] - invCubeRootTab[index];
  op_m = fMultAddDiv2(invCubeRootTab[index], diff << 1, fract);
#if defined(INVCUBEROOTNORM2_LINEAR_INTERPOLATE_HQ)
  /* reg1 = t[i] + (t[i+1]-t[i])*fract ... already computed ... +
   * (1-fract)fract*(t[i+2]-t[i+1])/2 */
  if (fract != (FIXP_DBL)0) {
    /* fract = fract * (1 - fract) */
    fract = fMultDiv2(fract, (FIXP_DBL)((LONG)0x80000000 - (LONG)fract)) << 1;
    diff = diff - (invCubeRootTab[index + 2] - invCubeRootTab[index + 1]);
    op_m = fMultAddDiv2(op_m, fract, diff);
  }
#endif /* INVCUBEROOTNORM2_LINEAR_INTERPOLATE_HQ */

  /* calculate the output exponent = input * exp/3 = cubicroot(m)*2^(exp/3)
   * where 2^(exp/3) = 2^k'*2 or 2^k'*2^(1/3) or 2^k'*2^(2/3) */
  exponent = exponent - *op_e + 3;
  INT shift_tmp =
      ((INT)fMultDiv2((FIXP_SGL)fAbs(exponent), (FIXP_SGL)0x5556)) >> 16;
  if (exponent < 0) {
    shift_tmp = -shift_tmp;
  }
  INT rem = exponent - 3 * shift_tmp;
  if (rem < 0) {
    rem += 3;
    shift_tmp--;
  }

  *op_e = shift_tmp;
  op_m = fMultDiv2(op_m, invCubeRootCorrection[rem]) << 2;

  return (op_m);
}

  /*****************************************************************************

      functionname: invFourthRootNorm2
      description:  delivers 1/FourthRoot(op) in Q1.31 format and modified
  exponent

  *****************************************************************************/

#define FOURTHROOT_BITS 7
#define FOURTHROOT_VALUES (128 + 2)
#define FOURTHROOT_BITS_MASK 0x7f
#define FOURTHROOT_FRACT_BITS_MASK 0x007FFFFF

LNK_SECTION_CONSTDATA
static const FIXP_DBL invFourthRootTab[FOURTHROOT_VALUES] = {
    (0x4c1bf829), (0x4bf61977), (0x4bd09843), (0x4bab72ef), (0x4b86a7eb),
    (0x4b6235ac), (0x4b3e1ab6), (0x4b1a5592), (0x4af6e4d4), (0x4ad3c718),
    (0x4ab0fb03), (0x4a8e7f42), (0x4a6c5288), (0x4a4a7393), (0x4a28e126),
    (0x4a079a0c), (0x49e69d16), (0x49c5e91f), (0x49a57d04), (0x498557ac),
    (0x49657802), (0x4945dcf9), (0x49268588), (0x490770ac), (0x48e89d6a),
    (0x48ca0ac9), (0x48abb7d6), (0x488da3a6), (0x486fcd4f), (0x485233ed),
    (0x4834d6a3), (0x4817b496), (0x47faccf0), (0x47de1ee0), (0x47c1a999),
    (0x47a56c51), (0x47896643), (0x476d96af), (0x4751fcd6), (0x473697ff),
    (0x471b6773), (0x47006a81), (0x46e5a079), (0x46cb08ae), (0x46b0a279),
    (0x46966d34), (0x467c683d), (0x466292f4), (0x4648ecbc), (0x462f74fe),
    (0x46162b20), (0x45fd0e91), (0x45e41ebe), (0x45cb5b19), (0x45b2c315),
    (0x459a562a), (0x458213cf), (0x4569fb81), (0x45520cbc), (0x453a4701),
    (0x4522a9d1), (0x450b34b0), (0x44f3e726), (0x44dcc0ba), (0x44c5c0f7),
    (0x44aee768), (0x4498339e), (0x4481a527), (0x446b3b96), (0x4454f67e),
    (0x443ed576), (0x4428d815), (0x4412fdf3), (0x43fd46ad), (0x43e7b1de),
    (0x43d23f23), (0x43bcee1e), (0x43a7be6f), (0x4392afb8), (0x437dc19d),
    (0x4368f3c5), (0x435445d6), (0x433fb779), (0x432b4856), (0x4316f81a),
    (0x4302c66f), (0x42eeb305), (0x42dabd8a), (0x42c6e5ad), (0x42b32b21),
    (0x429f8d96), (0x428c0cc2), (0x4278a859), (0x42656010), (0x4252339e),
    (0x423f22bc), (0x422c2d23), (0x4219528b), (0x420692b2), (0x41f3ed51),
    (0x41e16228), (0x41cef0f2), (0x41bc9971), (0x41aa5b62), (0x41983687),
    (0x41862aa2), (0x41743775), (0x41625cc3), (0x41509a50), (0x413eefe2),
    (0x412d5d3e), (0x411be22b), (0x410a7e70), (0x40f931d5), (0x40e7fc23),
    (0x40d6dd24), (0x40c5d4a2), (0x40b4e268), (0x40a40642), (0x40933ffc),
    (0x40828f64), (0x4071f447), (0x40616e73), (0x4050fdb9), (0x4040a1e6),
    (0x40305acc), (0x4020283c), (0x40100a08), (0x40000000), (0x3ff009f9),
};

static const FIXP_DBL invFourthRootCorrection[4] = {0x40000000, 0x4C1BF829,
                                                    0x5A82799A, 0x6BA27E65};

/* The fourth root function */
/*****************************************************************************
 * \brief calculate 1.0/fourth_root(op), op contains mantissa and exponent
 * \param op_m: (i) mantissa of operand, must not be zero (0x0000.0000) or
 * negative
 * \param op_e: (i) pointer to the exponent of the operand (must be initialized)
 * and .. (o) pointer to the exponent of the result
 * \return:     (o) mantissa of the result
 * \description:
 *  This routine calculates the cube root of the input operand, that is
 *  given with its mantissa in Q31 format (FIXP_DBL) and its exponent (INT).
 *  The resulting mantissa is returned in format Q31. The exponent (*op_e)
 *  is modified accordingly. It is not assured, that the result is fully
 * left-aligned but assumed to have not more than 2 bits headroom. There is one
 * macro to activate the use of this algorithm: FUNCTION_invFourthRootNorm2 By
 * means of activating the macro INVFOURTHROOTNORM2_LINEAR_INTERPOLATE_HQ, a
 * slightly higher precision is reachable (by default, not active). For DEBUG
 * purpose only: a FDK_ASSERT macro validates, if the input mantissa is greater
 * zero.
 *
 */

/* #define INVFOURTHROOTNORM2_LINEAR_INTERPOLATE_HQ */

static
#ifdef __arm__
    FIXP_DBL FDK_FORCEINLINE
    invFourthRootNorm2(FIXP_DBL op_m, INT* op_e)
#else
    FIXP_DBL
    invFourthRootNorm2(FIXP_DBL op_m, INT* op_e)
#endif
{
  FDK_ASSERT(op_m > FL2FXCONST_DBL(0.0));

  /* normalize input, calculate shift value */
  INT exponent = (INT)fNormz(op_m) - 1;
  op_m <<= exponent;

  INT index = (INT)(op_m >> (DFRACT_BITS - 1 - (FOURTHROOT_BITS + 1))) &
              FOURTHROOT_BITS_MASK;
  FIXP_DBL fract = (FIXP_DBL)(((INT)op_m & FOURTHROOT_FRACT_BITS_MASK)
                              << (FOURTHROOT_BITS + 1));
  FIXP_DBL diff = invFourthRootTab[index + 1] - invFourthRootTab[index];
  op_m = invFourthRootTab[index] + (fMultDiv2(diff, fract) << 1);

#if defined(INVFOURTHROOTNORM2_LINEAR_INTERPOLATE_HQ)
  /* reg1 = t[i] + (t[i+1]-t[i])*fract ... already computed ... +
   * (1-fract)fract*(t[i+2]-t[i+1])/2 */
  if (fract != (FIXP_DBL)0) {
    /* fract = fract * (1 - fract) */
    fract = fMultDiv2(fract, (FIXP_DBL)((LONG)0x80000000 - (LONG)fract)) << 1;
    diff = diff - (invFourthRootTab[index + 2] - invFourthRootTab[index + 1]);
    op_m = fMultAddDiv2(op_m, fract, diff);
  }
#endif /* INVFOURTHROOTNORM2_LINEAR_INTERPOLATE_HQ */

  exponent = exponent - *op_e + 4;
  INT rem = exponent & 0x00000003;
  INT shift_tmp = (exponent >> 2);

  *op_e = shift_tmp;
  op_m = fMultDiv2(op_m, invFourthRootCorrection[rem]) << 2;

  return (op_m);
}

/*****************************************************************************

    functionname: inv3EigthRootNorm2
    description:  delivers 1/cubert(op) normalized to .5...1 and the shift value
of the OUTPUT

*****************************************************************************/
#define THREEIGTHROOT_BITS 7
#define THREEIGTHROOT_VALUES (128 + 2)
#define THREEIGTHROOT_BITS_MASK 0x7f
#define THREEIGTHROOT_FRACT_BITS_MASK 0x007FFFFF

LNK_SECTION_CONSTDATA
static const FIXP_DBL inv3EigthRootTab[THREEIGTHROOT_VALUES] = {
    (0x45cae0f2), (0x45b981bf), (0x45a8492a), (0x45973691), (0x45864959),
    (0x457580e6), (0x4564dca4), (0x45545c00), (0x4543fe6b), (0x4533c35a),
    (0x4523aa44), (0x4513b2a4), (0x4503dbf7), (0x44f425be), (0x44e48f7b),
    (0x44d518b6), (0x44c5c0f7), (0x44b687c8), (0x44a76cb8), (0x44986f58),
    (0x44898f38), (0x447acbef), (0x446c2514), (0x445d9a3f), (0x444f2b0d),
    (0x4440d71a), (0x44329e07), (0x44247f73), (0x44167b04), (0x4408905e),
    (0x43fabf28), (0x43ed070b), (0x43df67b0), (0x43d1e0c5), (0x43c471f7),
    (0x43b71af6), (0x43a9db71), (0x439cb31c), (0x438fa1ab), (0x4382a6d2),
    (0x4375c248), (0x4368f3c5), (0x435c3b03), (0x434f97bc), (0x434309ac),
    (0x43369091), (0x432a2c28), (0x431ddc30), (0x4311a06c), (0x4305789c),
    (0x42f96483), (0x42ed63e5), (0x42e17688), (0x42d59c30), (0x42c9d4a6),
    (0x42be1fb1), (0x42b27d1a), (0x42a6ecac), (0x429b6e2f), (0x42900172),
    (0x4284a63f), (0x42795c64), (0x426e23b0), (0x4262fbf2), (0x4257e4f9),
    (0x424cde96), (0x4241e89a), (0x423702d8), (0x422c2d23), (0x4221674d),
    (0x4216b12c), (0x420c0a94), (0x4201735b), (0x41f6eb57), (0x41ec725f),
    (0x41e2084b), (0x41d7acf3), (0x41cd6030), (0x41c321db), (0x41b8f1ce),
    (0x41aecfe5), (0x41a4bbf8), (0x419ab5e6), (0x4190bd89), (0x4186d2bf),
    (0x417cf565), (0x41732558), (0x41696277), (0x415faca1), (0x415603b4),
    (0x414c6792), (0x4142d818), (0x4139552a), (0x412fdea6), (0x41267470),
    (0x411d1668), (0x4113c472), (0x410a7e70), (0x41014445), (0x40f815d4),
    (0x40eef302), (0x40e5dbb4), (0x40dccfcd), (0x40d3cf33), (0x40cad9cb),
    (0x40c1ef7b), (0x40b9102a), (0x40b03bbd), (0x40a7721c), (0x409eb32e),
    (0x4095feda), (0x408d5508), (0x4084b5a0), (0x407c208b), (0x407395b2),
    (0x406b14fd), (0x40629e56), (0x405a31a6), (0x4051ced8), (0x404975d5),
    (0x40412689), (0x4038e0dd), (0x4030a4bd), (0x40287215), (0x402048cf),
    (0x401828d7), (0x4010121a), (0x40080483), (0x40000000), (0x3ff8047d),
};

/* The last value is rounded in order to avoid any overflow due to the values
 * range of the root table */
static const FIXP_DBL inv3EigthRootCorrection[8] = {
    0x40000000, 0x45CAE0F2, 0x4C1BF829, 0x52FF6B55,
    0x5A82799A, 0x62B39509, 0x6BA27E65, 0x75606373};

/* The 3/8 root function */
/*****************************************************************************
 * \brief calculate 1.0/3Eigth_root(op) = 1.0/(x)^(3/8), op contains mantissa
 * and exponent
 * \param op_m: (i) mantissa of operand, must not be zero (0x0000.0000) or
 * negative
 * \param op_e: (i) pointer to the exponent of the operand (must be initialized)
 * and .. (o) pointer to the exponent of the result
 * \return:     (o) mantissa of the result
 * \description:
 *  This routine calculates the cube root of the input operand, that is
 *  given with its mantissa in Q31 format (FIXP_DBL) and its exponent (INT).
 *  The resulting mantissa is returned in format Q31. The exponent (*op_e)
 *  is modified accordingly. It is not assured, that the result is fully
 * left-aligned but assumed to have not more than 2 bits headroom. There is one
 * macro to activate the use of this algorithm: FUNCTION_inv3EigthRootNorm2 By
 * means of activating the macro INVTHREEIGTHROOTNORM2_LINEAR_INTERPOLATE_HQ, a
 * slightly higher precision is reachable (by default, not active). For DEBUG
 * purpose only: a FDK_ASSERT macro validates, if the input mantissa is greater
 * zero.
 *
 */

/* #define INVTHREEIGTHROOTNORM2_LINEAR_INTERPOLATE_HQ */

static
#ifdef __arm__
    FIXP_DBL FDK_FORCEINLINE
    inv3EigthRootNorm2(FIXP_DBL op_m, INT* op_e)
#else
    FIXP_DBL
    inv3EigthRootNorm2(FIXP_DBL op_m, INT* op_e)
#endif
{
  FDK_ASSERT(op_m > FL2FXCONST_DBL(0.0));

  /* normalize input, calculate shift op_mue */
  INT exponent = (INT)fNormz(op_m) - 1;
  op_m <<= exponent;

  INT index = (INT)(op_m >> (DFRACT_BITS - 1 - (THREEIGTHROOT_BITS + 1))) &
              THREEIGTHROOT_BITS_MASK;
  FIXP_DBL fract = (FIXP_DBL)(((INT)op_m & THREEIGTHROOT_FRACT_BITS_MASK)
                              << (THREEIGTHROOT_BITS + 1));
  FIXP_DBL diff = inv3EigthRootTab[index + 1] - inv3EigthRootTab[index];
  op_m = inv3EigthRootTab[index] + (fMultDiv2(diff, fract) << 1);

#if defined(INVTHREEIGTHROOTNORM2_LINEAR_INTERPOLATE_HQ)
  /* op_m = t[i] + (t[i+1]-t[i])*fract ... already computed ... +
   * (1-fract)fract*(t[i+2]-t[i+1])/2 */
  if (fract != (FIXP_DBL)0) {
    /* fract = fract * (1 - fract) */
    fract = fMultDiv2(fract, (FIXP_DBL)((LONG)0x80000000 - (LONG)fract)) << 1;
    diff = diff - (inv3EigthRootTab[index + 2] - inv3EigthRootTab[index + 1]);
    op_m = fMultAddDiv2(op_m, fract, diff);
  }
#endif /* INVTHREEIGTHROOTNORM2_LINEAR_INTERPOLATE_HQ */

  exponent = exponent - *op_e + 8;
  INT rem = exponent & 0x00000007;
  INT shift_tmp = (exponent >> 3);

  *op_e = shift_tmp * 3;
  op_m = fMultDiv2(op_m, inv3EigthRootCorrection[rem]) << 2;

  return (fMult(op_m, fMult(op_m, op_m)));
}

SBR_ERROR
QmfTransposerCreate(HANDLE_HBE_TRANSPOSER* hQmfTransposer, const int frameSize,
                    int bDisableCrossProducts, int bSbr41) {
  HANDLE_HBE_TRANSPOSER hQmfTran = NULL;

  int i;

  if (hQmfTransposer != NULL) {
    /* Memory allocation */
    /*--------------------------------------------------------------------------------------------*/
    hQmfTran =
        (HANDLE_HBE_TRANSPOSER)FDKcalloc(1, sizeof(struct hbeTransposer));
    if (hQmfTran == NULL) {
      return SBRDEC_MEM_ALLOC_FAILED;
    }

    for (i = 0; i < MAX_STRETCH_HBE - 1; i++) {
      hQmfTran->bXProducts[i] = (bDisableCrossProducts ? 0 : xProducts[i]);
    }

    hQmfTran->timeDomainWinLen = frameSize;
    if (frameSize == 768) {
      hQmfTran->noCols =
          (8 * frameSize / 3) / QMF_SYNTH_CHANNELS; /* 32 for 24:64 */
    } else {
      hQmfTran->noCols =
          (bSbr41 + 1) * 2 * frameSize /
          QMF_SYNTH_CHANNELS; /* 32 for 32:64 and 64 for 16:64 -> identical to
                                 sbrdec->no_cols */
    }

    hQmfTran->noChannels = frameSize / hQmfTran->noCols;

    hQmfTran->qmfInBufSize = QMF_WIN_LEN;
    hQmfTran->qmfOutBufSize = 2 * (hQmfTran->noCols / 2 + QMF_WIN_LEN - 1);

    hQmfTran->inBuf_F =
        (LONG*)FDKcalloc(QMF_SYNTH_CHANNELS + 20 + 1, sizeof(LONG));
    /* buffered time signal needs to be delayed by synthesis_size; max
     * synthesis_size = 20; */
    if (hQmfTran->inBuf_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }

    hQmfTran->qmfInBufReal_F =
        (FIXP_DBL**)FDKcalloc(hQmfTran->qmfInBufSize, sizeof(FIXP_DBL*));
    hQmfTran->qmfInBufImag_F =
        (FIXP_DBL**)FDKcalloc(hQmfTran->qmfInBufSize, sizeof(FIXP_DBL*));

    if (hQmfTran->qmfInBufReal_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }
    if (hQmfTran->qmfInBufImag_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }

    for (i = 0; i < hQmfTran->qmfInBufSize; i++) {
      hQmfTran->qmfInBufReal_F[i] = (FIXP_DBL*)FDKaalloc(
          QMF_SYNTH_CHANNELS * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
      hQmfTran->qmfInBufImag_F[i] = (FIXP_DBL*)FDKaalloc(
          QMF_SYNTH_CHANNELS * sizeof(FIXP_DBL), ALIGNMENT_DEFAULT);
      if (hQmfTran->qmfInBufReal_F[i] == NULL) {
        QmfTransposerClose(hQmfTran);
        return SBRDEC_MEM_ALLOC_FAILED;
      }
      if (hQmfTran->qmfInBufImag_F[i] == NULL) {
        QmfTransposerClose(hQmfTran);
        return SBRDEC_MEM_ALLOC_FAILED;
      }
    }

    hQmfTran->qmfHBEBufReal_F =
        (FIXP_DBL**)FDKcalloc(HBE_MAX_OUT_SLOTS, sizeof(FIXP_DBL*));
    hQmfTran->qmfHBEBufImag_F =
        (FIXP_DBL**)FDKcalloc(HBE_MAX_OUT_SLOTS, sizeof(FIXP_DBL*));

    if (hQmfTran->qmfHBEBufReal_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }
    if (hQmfTran->qmfHBEBufImag_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }

    for (i = 0; i < HBE_MAX_OUT_SLOTS; i++) {
      hQmfTran->qmfHBEBufReal_F[i] =
          (FIXP_DBL*)FDKcalloc(QMF_SYNTH_CHANNELS, sizeof(FIXP_DBL));
      hQmfTran->qmfHBEBufImag_F[i] =
          (FIXP_DBL*)FDKcalloc(QMF_SYNTH_CHANNELS, sizeof(FIXP_DBL));
      if (hQmfTran->qmfHBEBufReal_F[i] == NULL) {
        QmfTransposerClose(hQmfTran);
        return SBRDEC_MEM_ALLOC_FAILED;
      }
      if (hQmfTran->qmfHBEBufImag_F[i] == NULL) {
        QmfTransposerClose(hQmfTran);
        return SBRDEC_MEM_ALLOC_FAILED;
      }
    }

    hQmfTran->qmfBufferCodecTempSlot_F =
        (FIXP_DBL*)FDKcalloc(QMF_SYNTH_CHANNELS / 2, sizeof(FIXP_DBL));
    if (hQmfTran->qmfBufferCodecTempSlot_F == NULL) {
      QmfTransposerClose(hQmfTran);
      return SBRDEC_MEM_ALLOC_FAILED;
    }

    hQmfTran->bSbr41 = bSbr41;

    hQmfTran->highband_exp[0] = 0;
    hQmfTran->highband_exp[1] = 0;
    hQmfTran->target_exp[0] = 0;
    hQmfTran->target_exp[1] = 0;

    *hQmfTransposer = hQmfTran;
  }

  return SBRDEC_OK;
}

SBR_ERROR QmfTransposerReInit(HANDLE_HBE_TRANSPOSER hQmfTransposer,
                              UCHAR* FreqBandTable[2], UCHAR NSfb[2])
/* removed bSbr41 from parameterlist:
   don't know where to get this value from
   at call-side */
{
  int L, sfb, patch, stopPatch, qmfErr;

  if (hQmfTransposer != NULL) {
    const FIXP_QTW* tmp_t_cos;
    const FIXP_QTW* tmp_t_sin;

    hQmfTransposer->startBand = FreqBandTable[0][0];
    FDK_ASSERT((!hQmfTransposer->bSbr41 && hQmfTransposer->startBand <= 32) ||
               (hQmfTransposer->bSbr41 &&
                hQmfTransposer->startBand <=
                    16)); /* is checked by resetFreqBandTables() */
    hQmfTransposer->stopBand = FreqBandTable[0][NSfb[0]];

    hQmfTransposer->synthSize =
        4 * ((hQmfTransposer->startBand + 4) / 8 + 1); /* 8, 12, 16, 20 */
    hQmfTransposer->kstart = startSubband2kL[hQmfTransposer->startBand];

    /* don't know where to take this information from */
    /* hQmfTransposer->bSbr41 = bSbr41;               */

    if (hQmfTransposer->bSbr41) {
      if (hQmfTransposer->kstart + hQmfTransposer->synthSize > 16)
        hQmfTransposer->kstart = 16 - hQmfTransposer->synthSize;
    } else if (hQmfTransposer->timeDomainWinLen == 768) {
      if (hQmfTransposer->kstart + hQmfTransposer->synthSize > 24)
        hQmfTransposer->kstart = 24 - hQmfTransposer->synthSize;
    }

    hQmfTransposer->synthesisQmfPreModCos_F =
        &preModCos[hQmfTransposer->kstart];
    hQmfTransposer->synthesisQmfPreModSin_F =
        &preModSin[hQmfTransposer->kstart];

    L = 2 * hQmfTransposer->synthSize; /* 8, 16, 24, 32, 40 */
                                       /* Change analysis post twiddles */

    switch (L) {
      case 8:
        tmp_t_cos = post_twiddle_cos_8;
        tmp_t_sin = post_twiddle_sin_8;
        break;
      case 16:
        tmp_t_cos = post_twiddle_cos_16;
        tmp_t_sin = post_twiddle_sin_16;
        break;
      case 24:
        tmp_t_cos = post_twiddle_cos_24;
        tmp_t_sin = post_twiddle_sin_24;
        break;
      case 32:
        tmp_t_cos = post_twiddle_cos_32;
        tmp_t_sin = post_twiddle_sin_32;
        break;
      case 40:
        tmp_t_cos = post_twiddle_cos_40;
        tmp_t_sin = post_twiddle_sin_40;
        break;
      default:
        return SBRDEC_UNSUPPORTED_CONFIG;
    }

    qmfErr = qmfInitSynthesisFilterBank(
        &hQmfTransposer->HBESynthesisQMF, hQmfTransposer->synQmfStates,
        hQmfTransposer->noCols, 0, hQmfTransposer->synthSize,
        hQmfTransposer->synthSize, 1);
    if (qmfErr != 0) {
      return SBRDEC_UNSUPPORTED_CONFIG;
    }

    qmfErr = qmfInitAnalysisFilterBank(
        &hQmfTransposer->HBEAnalysiscQMF, hQmfTransposer->anaQmfStates,
        hQmfTransposer->noCols / 2, 0, 2 * hQmfTransposer->synthSize,
        2 * hQmfTransposer->synthSize, 0);

    if (qmfErr != 0) {
      return SBRDEC_UNSUPPORTED_CONFIG;
    }

    hQmfTransposer->HBEAnalysiscQMF.t_cos = tmp_t_cos;
    hQmfTransposer->HBEAnalysiscQMF.t_sin = tmp_t_sin;

    FDKmemset(hQmfTransposer->xOverQmf, 0,
              MAX_NUM_PATCHES * sizeof(int)); /* global */
    sfb = 0;
    if (hQmfTransposer->bSbr41) {
      stopPatch = MAX_NUM_PATCHES;
      hQmfTransposer->maxStretch = MAX_STRETCH_HBE;
    } else {
      stopPatch = MAX_STRETCH_HBE;
    }

    for (patch = 1; patch <= stopPatch; patch++) {
      while (sfb <= NSfb[0] &&
             FreqBandTable[0][sfb] <= patch * hQmfTransposer->startBand)
        sfb++;
      if (sfb <= NSfb[0]) {
        /* If the distance is larger than three QMF bands - try aligning to high
         * resolution frequency bands instead. */
        if ((patch * hQmfTransposer->startBand - FreqBandTable[0][sfb - 1]) <=
            3) {
          hQmfTransposer->xOverQmf[patch - 1] = FreqBandTable[0][sfb - 1];
        } else {
          int sfb_tmp = 0;
          while (sfb_tmp <= NSfb[1] &&
                 FreqBandTable[1][sfb_tmp] <= patch * hQmfTransposer->startBand)
            sfb_tmp++;
          hQmfTransposer->xOverQmf[patch - 1] = FreqBandTable[1][sfb_tmp - 1];
        }
      } else {
        hQmfTransposer->xOverQmf[patch - 1] = hQmfTransposer->stopBand;
        hQmfTransposer->maxStretch = fMin(patch, MAX_STRETCH_HBE);
        break;
      }
    }

    hQmfTransposer->highband_exp[0] = 0;
    hQmfTransposer->highband_exp[1] = 0;
    hQmfTransposer->target_exp[0] = 0;
    hQmfTransposer->target_exp[1] = 0;
  }

  return SBRDEC_OK;
}

void QmfTransposerClose(HANDLE_HBE_TRANSPOSER hQmfTransposer) {
  int i;

  if (hQmfTransposer != NULL) {
    if (hQmfTransposer->inBuf_F) FDKfree(hQmfTransposer->inBuf_F);

    if (hQmfTransposer->qmfInBufReal_F) {
      for (i = 0; i < hQmfTransposer->qmfInBufSize; i++) {
        FDKafree(hQmfTransposer->qmfInBufReal_F[i]);
      }
      FDKfree(hQmfTransposer->qmfInBufReal_F);
    }

    if (hQmfTransposer->qmfInBufImag_F) {
      for (i = 0; i < hQmfTransposer->qmfInBufSize; i++) {
        FDKafree(hQmfTransposer->qmfInBufImag_F[i]);
      }
      FDKfree(hQmfTransposer->qmfInBufImag_F);
    }

    if (hQmfTransposer->qmfHBEBufReal_F) {
      for (i = 0; i < HBE_MAX_OUT_SLOTS; i++) {
        FDKfree(hQmfTransposer->qmfHBEBufReal_F[i]);
      }
      FDKfree(hQmfTransposer->qmfHBEBufReal_F);
    }

    if (hQmfTransposer->qmfHBEBufImag_F) {
      for (i = 0; i < HBE_MAX_OUT_SLOTS; i++) {
        FDKfree(hQmfTransposer->qmfHBEBufImag_F[i]);
      }
      FDKfree(hQmfTransposer->qmfHBEBufImag_F);
    }

    FDKfree(hQmfTransposer->qmfBufferCodecTempSlot_F);

    FDKfree(hQmfTransposer);
  }
}

inline void scaleUp(FIXP_DBL* real_m, FIXP_DBL* imag_m, INT* _e) {
  INT reserve;
  /* shift gc_r and gc_i up if possible */
  reserve = CntLeadingZeros((INT(*real_m) ^ INT((*real_m >> 31))) |
                            (INT(*imag_m) ^ INT((*imag_m >> 31)))) -
            1;
  reserve = fMax(reserve - 1,
                 0); /* Leave one bit headroom such that (real_m^2 + imag_m^2)
                        does not overflow later if both are 0x80000000. */
  reserve = fMin(reserve, *_e);
  FDK_ASSERT(reserve >= 0);
  *real_m <<= reserve;
  *imag_m <<= reserve;
  *_e -= reserve;
}

static void calculateCenterFIXP(FIXP_DBL gammaVecReal, FIXP_DBL gammaVecImag,
                                FIXP_DBL* centerReal, FIXP_DBL* centerImag,
                                INT* exponent, int stretch, int mult) {
  scaleUp(&gammaVecReal, &gammaVecImag, exponent);
  FIXP_DBL energy = fPow2Div2(gammaVecReal) + fPow2Div2(gammaVecImag);

  if (energy != FL2FXCONST_DBL(0.f)) {
    FIXP_DBL gc_r_m, gc_i_m, factor_m = (FIXP_DBL)0;
    INT factor_e, gc_e;
    factor_e = 2 * (*exponent) + 1;

    switch (stretch) {
      case 2:
        factor_m = invFourthRootNorm2(energy, &factor_e);
        break;
      case 3:
        factor_m = invCubeRootNorm2(energy, &factor_e);
        break;
      case 4:
        factor_m = inv3EigthRootNorm2(energy, &factor_e);
        break;
    }

    gc_r_m = fMultDiv2(gammaVecReal,
                       factor_m); /* exponent = HBE_SCALE + factor_e + 1 */
    gc_i_m = fMultDiv2(gammaVecImag,
                       factor_m); /* exponent = HBE_SCALE + factor_e + 1*/
    gc_e = *exponent + factor_e + 1;

    scaleUp(&gc_r_m, &gc_i_m, &gc_e);

    switch (mult) {
      case 0:
        *centerReal = gc_r_m;
        *centerImag = gc_i_m;
        break;
      case 1:
        *centerReal = fPow2Div2(gc_r_m) - fPow2Div2(gc_i_m);
        *centerImag = fMult(gc_r_m, gc_i_m);
        gc_e = 2 * gc_e + 1;
        break;
      case 2:
        FIXP_DBL tmp_r = gc_r_m;
        FIXP_DBL tmp_i = gc_i_m;
        gc_r_m = fPow2Div2(gc_r_m) - fPow2Div2(gc_i_m);
        gc_i_m = fMult(tmp_r, gc_i_m);
        gc_e = 3 * gc_e + 1 + 1;
        cplxMultDiv2(&centerReal[0], &centerImag[0], gc_r_m, gc_i_m, tmp_r,
                     tmp_i);
        break;
    }

    scaleUp(centerReal, centerImag, &gc_e);

    FDK_ASSERT(gc_e >= 0);
    *exponent = gc_e;
  } else {
    *centerReal = energy; /* energy = 0 */
    *centerImag = energy; /* energy = 0 */
    *exponent = (INT)energy;
  }
}

static int getHBEScaleFactorFrame(const int bSbr41, const int maxStretch,
                                  const int pitchInBins) {
  if (pitchInBins >= pmin * (1 + bSbr41)) {
    /* crossproducts enabled */
    return 26;
  } else {
    return (maxStretch == 2) ? 24 : 25;
  }
}

static void addHighBandPart(FIXP_DBL g_r_m, FIXP_DBL g_i_m, INT g_e,
                            FIXP_DBL mult, FIXP_DBL gammaCenterReal_m,
                            FIXP_DBL gammaCenterImag_m, INT gammaCenter_e,
                            INT stretch, INT scale_factor_hbe,
                            FIXP_DBL* qmfHBEBufReal_F,
                            FIXP_DBL* qmfHBEBufImag_F) {
  if ((g_r_m | g_i_m) != FL2FXCONST_DBL(0.f)) {
    FIXP_DBL factor_m = (FIXP_DBL)0;
    INT factor_e;
    INT add = (stretch == 4) ? 1 : 0;
    INT shift = (stretch == 4) ? 1 : 2;

    scaleUp(&g_r_m, &g_i_m, &g_e);
    FIXP_DBL energy = fPow2AddDiv2(fPow2Div2(g_r_m), g_i_m);
    factor_e = 2 * g_e + 1;

    switch (stretch) {
      case 2:
        factor_m = invFourthRootNorm2(energy, &factor_e);
        break;
      case 3:
        factor_m = invCubeRootNorm2(energy, &factor_e);
        break;
      case 4:
        factor_m = inv3EigthRootNorm2(energy, &factor_e);
        break;
    }

    factor_m = fMult(factor_m, mult);

    FIXP_DBL tmp_r, tmp_i;
    cplxMultDiv2(&tmp_r, &tmp_i, g_r_m, g_i_m, gammaCenterReal_m,
                 gammaCenterImag_m);

    g_r_m = fMultDiv2(tmp_r, factor_m) << shift;
    g_i_m = fMultDiv2(tmp_i, factor_m) << shift;
    g_e = scale_factor_hbe - (g_e + factor_e + gammaCenter_e + add);
    g_e = fMax((INT)0, g_e);
    *qmfHBEBufReal_F += g_r_m >> g_e;
    *qmfHBEBufImag_F += g_i_m >> g_e;
  }
}

void QmfTransposerApply(HANDLE_HBE_TRANSPOSER hQmfTransposer,
                        FIXP_DBL** qmfBufferCodecReal,
                        FIXP_DBL** qmfBufferCodecImag, int nColsIn,
                        FIXP_DBL** ppQmfBufferOutReal_F,
                        FIXP_DBL** ppQmfBufferOutImag_F,
                        FIXP_DBL lpcFilterStatesReal[2 + (3 * (4))][(64)],
                        FIXP_DBL lpcFilterStatesImag[2 + (3 * (4))][(64)],
                        int pitchInBins, int scale_lb, int scale_hbe,
                        int* scale_hb, int timeStep, int firstSlotOffsset,
                        int ov_len,
                        KEEP_STATES_SYNCED_MODE keepStatesSyncedMode) {
  int i, j, stretch, band, sourceband, r, s;
  int qmfVocoderColsIn = hQmfTransposer->noCols / 2;
  int bSbr41 = hQmfTransposer->bSbr41;

  const int winLength[3] = {10, 8, 6};
  const int slotOffset = 6; /* hQmfTransposer->winLen-6; */

  int qmfOffset = 2 * hQmfTransposer->kstart;
  int scale_border = (nColsIn == 64) ? 32 : nColsIn;

  INT slot_stretch4[9] = {0, 0, 0, 0, 2, 4, 6, 8, 10};
  INT slot_stretch2[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  INT slot_stretch3[10] = {0, 0, 0, 1, 3, 4, 6, 7, 9, 10};
  INT filt_stretch3[10] = {0, 0, 0, 1, 0, 1, 0, 1, 0, 1};
  INT filt_dummy[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  INT* pSlotStretch;
  INT* pFilt;

  int offset = 0; /* where to take  QmfTransposer data */

  int signPreMod =
      (hQmfTransposer->synthesisQmfPreModCos_F[0] < FL2FXCONST_DBL(0.f)) ? 1
                                                                         : -1;

  int scale_factor_hbe =
      getHBEScaleFactorFrame(bSbr41, hQmfTransposer->maxStretch, pitchInBins);

  if (keepStatesSyncedMode != KEEP_STATES_SYNCED_OFF) {
    offset = hQmfTransposer->noCols - ov_len - LPC_ORDER;
  }

  hQmfTransposer->highband_exp[0] = hQmfTransposer->highband_exp[1];
  hQmfTransposer->target_exp[0] = hQmfTransposer->target_exp[1];

  hQmfTransposer->highband_exp[1] = scale_factor_hbe;
  hQmfTransposer->target_exp[1] =
      fixMax(hQmfTransposer->highband_exp[1], hQmfTransposer->highband_exp[0]);

  scale_factor_hbe = hQmfTransposer->target_exp[1];

  int shift_ov = hQmfTransposer->target_exp[0] - hQmfTransposer->target_exp[1];

  if (shift_ov != 0) {
    for (i = 0; i < HBE_MAX_OUT_SLOTS; i++) {
      scaleValuesSaturate(&hQmfTransposer->qmfHBEBufReal_F[i][0],
                          QMF_SYNTH_CHANNELS, shift_ov);
      scaleValuesSaturate(&hQmfTransposer->qmfHBEBufImag_F[i][0],
                          QMF_SYNTH_CHANNELS, shift_ov);
    }

    if (keepStatesSyncedMode == KEEP_STATES_SYNCED_OFF) {
      int nBands =
          fMax(0, hQmfTransposer->stopBand - hQmfTransposer->startBand);

      for (i = timeStep * firstSlotOffsset; i < ov_len; i++) {
        scaleValuesSaturate(&ppQmfBufferOutReal_F[i][hQmfTransposer->startBand],
                            nBands, shift_ov);
        scaleValuesSaturate(&ppQmfBufferOutImag_F[i][hQmfTransposer->startBand],
                            nBands, shift_ov);
      }

      /* shift lpc filterstates */
      for (i = 0; i < timeStep * firstSlotOffsset + LPC_ORDER; i++) {
        scaleValuesSaturate(&lpcFilterStatesReal[i][0], (64), shift_ov);
        scaleValuesSaturate(&lpcFilterStatesImag[i][0], (64), shift_ov);
      }
    }
  }

  FIXP_DBL twid_m_new[3][2]; /* [stretch][cos/sin] */
  INT stepsize = 1 + !bSbr41, sine_offset = 24, mod = 96;
  INT mult[3] = {1, 2, 3};

  for (s = 0; s <= MAX_STRETCH_HBE - 2; s++) {
    twid_m_new[s][0] = twiddle[(mult[s] * (stepsize * pitchInBins)) % mod];
    twid_m_new[s][1] =
        twiddle[((mult[s] * (stepsize * pitchInBins)) + sine_offset) % mod];
  }

  /* Time-stretch */
  for (j = 0; j < qmfVocoderColsIn; j++) {
    int sign = -1, k, z, addrshift, codecTemp_e;
    /* update inbuf */
    for (i = 0; i < hQmfTransposer->synthSize; i++) {
      hQmfTransposer->inBuf_F[i] =
          hQmfTransposer->inBuf_F[i + 2 * hQmfTransposer->synthSize];
    }

    /* run synthesis for two sbr slots as transposer uses
    half slots double bands representation */
    for (z = 0; z < 2; z++) {
      int scale_factor = ((nColsIn == 64) && ((2 * j + z) < scale_border))
                             ? scale_lb
                             : scale_hbe;
      codecTemp_e = scale_factor - 1; /* -2 for Div2 and cos/sin scale of 1 */

      for (k = 0; k < hQmfTransposer->synthSize; k++) {
        int ki = hQmfTransposer->kstart + k;
        hQmfTransposer->qmfBufferCodecTempSlot_F[k] =
            fMultDiv2(signPreMod * hQmfTransposer->synthesisQmfPreModCos_F[k],
                      qmfBufferCodecReal[2 * j + z][ki]);
        hQmfTransposer->qmfBufferCodecTempSlot_F[k] +=
            fMultDiv2(signPreMod * hQmfTransposer->synthesisQmfPreModSin_F[k],
                      qmfBufferCodecImag[2 * j + z][ki]);
      }

      C_AALLOC_SCRATCH_START(pWorkBuffer, FIXP_DBL, (HBE_MAX_QMF_BANDS << 1));

      qmfSynthesisFilteringSlot(
          &hQmfTransposer->HBESynthesisQMF,
          hQmfTransposer->qmfBufferCodecTempSlot_F, NULL, 0,
          -7 - hQmfTransposer->HBESynthesisQMF.filterScale - codecTemp_e + 1,
          hQmfTransposer->inBuf_F + hQmfTransposer->synthSize * (z + 1), 1,
          pWorkBuffer);

      C_AALLOC_SCRATCH_END(pWorkBuffer, FIXP_DBL, (HBE_MAX_QMF_BANDS << 1));
    }

    C_AALLOC_SCRATCH_START(pWorkBuffer, FIXP_DBL, (HBE_MAX_QMF_BANDS << 1));

    qmfAnalysisFilteringSlot(&hQmfTransposer->HBEAnalysiscQMF,
                             hQmfTransposer->qmfInBufReal_F[QMF_WIN_LEN - 1],
                             hQmfTransposer->qmfInBufImag_F[QMF_WIN_LEN - 1],
                             hQmfTransposer->inBuf_F + 1, 1, pWorkBuffer);

    C_AALLOC_SCRATCH_END(pWorkBuffer, FIXP_DBL, (HBE_MAX_QMF_BANDS << 1));

    if ((keepStatesSyncedMode == KEEP_STATES_SYNCED_NORMAL) &&
        j <= qmfVocoderColsIn - ((LPC_ORDER + ov_len + QMF_WIN_LEN - 1) >> 1)) {
      /* update in buffer */
      for (i = 0; i < QMF_WIN_LEN - 1; i++) {
        FDKmemcpy(
            hQmfTransposer->qmfInBufReal_F[i],
            hQmfTransposer->qmfInBufReal_F[i + 1],
            sizeof(FIXP_DBL) * hQmfTransposer->HBEAnalysiscQMF.no_channels);
        FDKmemcpy(
            hQmfTransposer->qmfInBufImag_F[i],
            hQmfTransposer->qmfInBufImag_F[i + 1],
            sizeof(FIXP_DBL) * hQmfTransposer->HBEAnalysiscQMF.no_channels);
      }
      continue;
    }

    for (stretch = 2; stretch <= hQmfTransposer->maxStretch; stretch++) {
      int start = slotOffset - winLength[stretch - 2] / 2;
      int stop = slotOffset + winLength[stretch - 2] / 2;

      FIXP_DBL factor = FL2FXCONST_DBL(1.f / 3.f);

      for (band = hQmfTransposer->xOverQmf[stretch - 2];
           band < hQmfTransposer->xOverQmf[stretch - 1]; band++) {
        FIXP_DBL gammaCenterReal_m[2] = {(FIXP_DBL)0, (FIXP_DBL)0},
                 gammaCenterImag_m[2] = {(FIXP_DBL)0, (FIXP_DBL)0};
        INT gammaCenter_e[2] = {0, 0};

        FIXP_DBL gammaVecReal_m[2] = {(FIXP_DBL)0, (FIXP_DBL)0},
                 gammaVecImag_m[2] = {(FIXP_DBL)0, (FIXP_DBL)0};
        INT gammaVec_e[2] = {0, 0};

        FIXP_DBL wingain = (FIXP_DBL)0;

        gammaCenter_e[0] =
            SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
        gammaCenter_e[1] =
            SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);

        /* interpolation filters for 3rd order */
        sourceband = 2 * band / stretch - qmfOffset;
        FDK_ASSERT(sourceband >= 0);

        /* maximum gammaCenter_e == 20 */
        calculateCenterFIXP(
            hQmfTransposer->qmfInBufReal_F[slotOffset][sourceband],
            hQmfTransposer->qmfInBufImag_F[slotOffset][sourceband],
            &gammaCenterReal_m[0], &gammaCenterImag_m[0], &gammaCenter_e[0],
            stretch, stretch - 2);

        if (stretch == 4) {
          r = band - 2 * (band / 2);
          sourceband += (r == 0) ? -1 : 1;
          pSlotStretch = slot_stretch4;
          factor = FL2FXCONST_DBL(2.f / 3.f);
          pFilt = filt_dummy;
        } else if (stretch == 2) {
          r = 0;
          sourceband = 2 * band / stretch - qmfOffset;
          pSlotStretch = slot_stretch2;
          factor = FL2FXCONST_DBL(1.f / 3.f);
          pFilt = filt_dummy;
        } else {
          r = 2 * band - 3 * (2 * band / 3);
          sourceband = 2 * band / stretch - qmfOffset;
          pSlotStretch = slot_stretch3;
          factor = FL2FXCONST_DBL(1.4142f / 3.0f);
          pFilt = filt_stretch3;
        }

        if (r == 2) {
          calculateCenterFIXP(
              hQmfTransposer->qmfInBufReal_F[slotOffset][sourceband + 1],
              hQmfTransposer->qmfInBufImag_F[slotOffset][sourceband + 1],
              &gammaCenterReal_m[1], &gammaCenterImag_m[1], &gammaCenter_e[1],
              stretch, stretch - 2);

          factor = FL2FXCONST_DBL(1.4142f / 6.0f);
        }

        if (r == 2) {
          for (k = start; k < stop; k++) {
            gammaVecReal_m[0] =
                hQmfTransposer->qmfInBufReal_F[pSlotStretch[k]][sourceband];
            gammaVecReal_m[1] =
                hQmfTransposer->qmfInBufReal_F[pSlotStretch[k]][sourceband + 1];
            gammaVecImag_m[0] =
                hQmfTransposer->qmfInBufImag_F[pSlotStretch[k]][sourceband];
            gammaVecImag_m[1] =
                hQmfTransposer->qmfInBufImag_F[pSlotStretch[k]][sourceband + 1];
            gammaVec_e[0] = gammaVec_e[1] =
                SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);

            if (pFilt[k] == 1) {
              FIXP_DBL tmpRealF = gammaVecReal_m[0], tmpImagF;
              gammaVecReal_m[0] =
                  (fMult(gammaVecReal_m[0], hintReal_F[sourceband % 4][1]) -
                   fMult(gammaVecImag_m[0],
                         hintReal_F[(sourceband + 3) % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVecImag_m[0] =
                  (fMult(tmpRealF, hintReal_F[(sourceband + 3) % 4][1]) +
                   fMult(gammaVecImag_m[0], hintReal_F[sourceband % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */

              tmpRealF = hQmfTransposer
                             ->qmfInBufReal_F[pSlotStretch[k] + 1][sourceband];
              tmpImagF = hQmfTransposer
                             ->qmfInBufImag_F[pSlotStretch[k] + 1][sourceband];

              gammaVecReal_m[0] +=
                  (fMult(tmpRealF, hintReal_F[sourceband % 4][1]) -
                   fMult(tmpImagF, hintReal_F[(sourceband + 1) % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVecImag_m[0] +=
                  (fMult(tmpRealF, hintReal_F[(sourceband + 1) % 4][1]) +
                   fMult(tmpImagF, hintReal_F[sourceband % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVec_e[0]++;

              tmpRealF = gammaVecReal_m[1];

              gammaVecReal_m[1] =
                  (fMult(gammaVecReal_m[1], hintReal_F[sourceband % 4][2]) -
                   fMult(gammaVecImag_m[1],
                         hintReal_F[(sourceband + 3) % 4][2])) >>
                  1;
              gammaVecImag_m[1] =
                  (fMult(tmpRealF, hintReal_F[(sourceband + 3) % 4][2]) +
                   fMult(gammaVecImag_m[1], hintReal_F[sourceband % 4][2])) >>
                  1;

              tmpRealF =
                  hQmfTransposer
                      ->qmfInBufReal_F[pSlotStretch[k] + 1][sourceband + 1];
              tmpImagF =
                  hQmfTransposer
                      ->qmfInBufImag_F[pSlotStretch[k] + 1][sourceband + 1];

              gammaVecReal_m[1] +=
                  (fMult(tmpRealF, hintReal_F[sourceband % 4][2]) -
                   fMult(tmpImagF, hintReal_F[(sourceband + 1) % 4][2])) >>
                  1;
              gammaVecImag_m[1] +=
                  (fMult(tmpRealF, hintReal_F[(sourceband + 1) % 4][2]) +
                   fMult(tmpImagF, hintReal_F[sourceband % 4][2])) >>
                  1;
              gammaVec_e[1]++;
            }

            addHighBandPart(gammaVecReal_m[1], gammaVecImag_m[1], gammaVec_e[1],
                            factor, gammaCenterReal_m[0], gammaCenterImag_m[0],
                            gammaCenter_e[0], stretch, scale_factor_hbe,
                            &hQmfTransposer->qmfHBEBufReal_F[k][band],
                            &hQmfTransposer->qmfHBEBufImag_F[k][band]);

            addHighBandPart(gammaVecReal_m[0], gammaVecImag_m[0], gammaVec_e[0],
                            factor, gammaCenterReal_m[1], gammaCenterImag_m[1],
                            gammaCenter_e[1], stretch, scale_factor_hbe,
                            &hQmfTransposer->qmfHBEBufReal_F[k][band],
                            &hQmfTransposer->qmfHBEBufImag_F[k][band]);
          }
        } else {
          for (k = start; k < stop; k++) {
            gammaVecReal_m[0] =
                hQmfTransposer->qmfInBufReal_F[pSlotStretch[k]][sourceband];
            gammaVecImag_m[0] =
                hQmfTransposer->qmfInBufImag_F[pSlotStretch[k]][sourceband];
            gammaVec_e[0] =
                SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);

            if (pFilt[k] == 1) {
              FIXP_DBL tmpRealF = gammaVecReal_m[0], tmpImagF;
              gammaVecReal_m[0] =
                  (fMult(gammaVecReal_m[0], hintReal_F[sourceband % 4][1]) -
                   fMult(gammaVecImag_m[0],
                         hintReal_F[(sourceband + 3) % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVecImag_m[0] =
                  (fMult(tmpRealF, hintReal_F[(sourceband + 3) % 4][1]) +
                   fMult(gammaVecImag_m[0], hintReal_F[sourceband % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */

              tmpRealF = hQmfTransposer
                             ->qmfInBufReal_F[pSlotStretch[k] + 1][sourceband];
              tmpImagF = hQmfTransposer
                             ->qmfInBufImag_F[pSlotStretch[k] + 1][sourceband];

              gammaVecReal_m[0] +=
                  (fMult(tmpRealF, hintReal_F[sourceband % 4][1]) -
                   fMult(tmpImagF, hintReal_F[(sourceband + 1) % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVecImag_m[0] +=
                  (fMult(tmpRealF, hintReal_F[(sourceband + 1) % 4][1]) +
                   fMult(tmpImagF, hintReal_F[sourceband % 4][1])) >>
                  1; /* sum should be <= 1 because of sin/cos multiplication */
              gammaVec_e[0]++;
            }

            addHighBandPart(gammaVecReal_m[0], gammaVecImag_m[0], gammaVec_e[0],
                            factor, gammaCenterReal_m[0], gammaCenterImag_m[0],
                            gammaCenter_e[0], stretch, scale_factor_hbe,
                            &hQmfTransposer->qmfHBEBufReal_F[k][band],
                            &hQmfTransposer->qmfHBEBufImag_F[k][band]);
          }
        }

        /* pitchInBins is given with the resolution of a 768 bins FFT and we
         * need 64 QMF units so factor 768/64 = 12 */
        if (pitchInBins >= pmin * (1 + bSbr41)) {
          int tr, ti1, ti2, mTr = 0, ts1 = 0, ts2 = 0, mVal_e = 0, temp_e = 0;
          int sqmag0_e =
              SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);

          FIXP_DBL mVal_F = FL2FXCONST_DBL(0.f), sqmag0_F, sqmag1_F, sqmag2_F,
                   temp_F, f1_F; /* all equal exponent */
          sign = -1;

          sourceband = 2 * band / stretch - qmfOffset; /* consistent with the
                                                          already computed for
                                                          stretch = 3,4. */
          FDK_ASSERT(sourceband >= 0);

          FIXP_DBL sqmag0R_F =
              hQmfTransposer->qmfInBufReal_F[slotOffset][sourceband];
          FIXP_DBL sqmag0I_F =
              hQmfTransposer->qmfInBufImag_F[slotOffset][sourceband];
          scaleUp(&sqmag0R_F, &sqmag0I_F, &sqmag0_e);

          sqmag0_F = fPow2Div2(sqmag0R_F);
          sqmag0_F += fPow2Div2(sqmag0I_F);
          sqmag0_e = 2 * sqmag0_e + 1;

          for (tr = 1; tr < stretch; tr++) {
            int sqmag1_e =
                SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
            int sqmag2_e =
                SCALE2EXP(-hQmfTransposer->HBEAnalysiscQMF.outScalefactor);

            FIXP_DBL tmp_band = band_F[band];
            FIXP_DBL tr_p =
                fMult(p_F[pitchInBins] >> bSbr41, tr_str[tr - 1]); /* scale 7 */
            f1_F =
                fMult(tmp_band - tr_p, stretchfac[stretch - 2]); /* scale 7 */
            ti1 = (INT)(f1_F >> (DFRACT_BITS - 1 - 7)) - qmfOffset;
            ti2 = (INT)(((f1_F) + ((p_F[pitchInBins] >> bSbr41) >> 2)) >>
                        (DFRACT_BITS - 1 - 7)) -
                  qmfOffset;

            if (ti1 >= 0 && ti2 < 2 * hQmfTransposer->synthSize) {
              FIXP_DBL sqmag1R_F =
                  hQmfTransposer->qmfInBufReal_F[slotOffset][ti1];
              FIXP_DBL sqmag1I_F =
                  hQmfTransposer->qmfInBufImag_F[slotOffset][ti1];
              scaleUp(&sqmag1R_F, &sqmag1I_F, &sqmag1_e);
              sqmag1_F = fPow2Div2(sqmag1R_F);
              sqmag1_F += fPow2Div2(sqmag1I_F);
              sqmag1_e = 2 * sqmag1_e + 1;

              FIXP_DBL sqmag2R_F =
                  hQmfTransposer->qmfInBufReal_F[slotOffset][ti2];
              FIXP_DBL sqmag2I_F =
                  hQmfTransposer->qmfInBufImag_F[slotOffset][ti2];
              scaleUp(&sqmag2R_F, &sqmag2I_F, &sqmag2_e);
              sqmag2_F = fPow2Div2(sqmag2R_F);
              sqmag2_F += fPow2Div2(sqmag2I_F);
              sqmag2_e = 2 * sqmag2_e + 1;

              int shift1 = fMin(fMax(sqmag1_e, sqmag2_e) - sqmag1_e, 31);
              int shift2 = fMin(fMax(sqmag1_e, sqmag2_e) - sqmag2_e, 31);

              temp_F = fMin((sqmag1_F >> shift1), (sqmag2_F >> shift2));
              temp_e = fMax(sqmag1_e, sqmag2_e);

              int shift3 = fMin(fMax(temp_e, mVal_e) - temp_e, 31);
              int shift4 = fMin(fMax(temp_e, mVal_e) - mVal_e, 31);

              if ((temp_F >> shift3) > (mVal_F >> shift4)) {
                mVal_F = temp_F;
                mVal_e = temp_e; /* equals sqmag2_e + shift2 */
                mTr = tr;
                ts1 = ti1;
                ts2 = ti2;
              }
            }
          }

          int shift1 = fMin(fMax(sqmag0_e, mVal_e) - sqmag0_e, 31);
          int shift2 = fMin(fMax(sqmag0_e, mVal_e) - mVal_e, 31);

          if ((mVal_F >> shift2) > (sqmag0_F >> shift1) && ts1 >= 0 &&
              ts2 < 2 * hQmfTransposer->synthSize) {
            INT gammaOut_e[2];
            FIXP_DBL gammaOutReal_m[2], gammaOutImag_m[2];
            FIXP_DBL tmpReal_m = (FIXP_DBL)0, tmpImag_m = (FIXP_DBL)0;

            int Tcenter, Tvec;

            Tcenter = stretch - mTr; /* default phase power parameters */
            Tvec = mTr;
            switch (stretch) /* 2 tap block creation design depends on stretch
                                order */
            {
              case 2:
                wingain =
                    FL2FXCONST_DBL(5.f / 12.f); /* sum of taps divided by two */

                if (hQmfTransposer->bXProducts[0]) {
                  gammaCenterReal_m[0] =
                      hQmfTransposer->qmfInBufReal_F[slotOffset][ts1];
                  gammaCenterImag_m[0] =
                      hQmfTransposer->qmfInBufImag_F[slotOffset][ts1];

                  for (k = 0; k < 2; k++) {
                    gammaVecReal_m[k] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset - 1 + k][ts2];
                    gammaVecImag_m[k] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset - 1 + k][ts2];
                  }

                  gammaCenter_e[0] = SCALE2EXP(
                      -hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
                  gammaVec_e[0] = gammaVec_e[1] = SCALE2EXP(
                      -hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
                }
                break;

              case 4:
                wingain =
                    FL2FXCONST_DBL(6.f / 12.f); /* sum of taps divided by two */
                if (hQmfTransposer->bXProducts[2]) {
                  if (mTr == 1) {
                    gammaCenterReal_m[0] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts1];
                    gammaCenterImag_m[0] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts1];

                    for (k = 0; k < 2; k++) {
                      gammaVecReal_m[k] =
                          hQmfTransposer
                              ->qmfInBufReal_F[slotOffset + 2 * (k - 1)][ts2];
                      gammaVecImag_m[k] =
                          hQmfTransposer
                              ->qmfInBufImag_F[slotOffset + 2 * (k - 1)][ts2];
                    }
                  } else if (mTr == 2) {
                    gammaCenterReal_m[0] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts1];
                    gammaCenterImag_m[0] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts1];

                    for (k = 0; k < 2; k++) {
                      gammaVecReal_m[k] =
                          hQmfTransposer
                              ->qmfInBufReal_F[slotOffset + (k - 1)][ts2];
                      gammaVecImag_m[k] =
                          hQmfTransposer
                              ->qmfInBufImag_F[slotOffset + (k - 1)][ts2];
                    }
                  } else /* (mTr == 3) */
                  {
                    sign = 1;
                    Tcenter = mTr; /* opposite phase power parameters as ts2 is
                                      center */
                    Tvec = stretch - mTr;

                    gammaCenterReal_m[0] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts2];
                    gammaCenterImag_m[0] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts2];

                    for (k = 0; k < 2; k++) {
                      gammaVecReal_m[k] =
                          hQmfTransposer
                              ->qmfInBufReal_F[slotOffset + 2 * (k - 1)][ts1];
                      gammaVecImag_m[k] =
                          hQmfTransposer
                              ->qmfInBufImag_F[slotOffset + 2 * (k - 1)][ts1];
                    }
                  }

                  gammaCenter_e[0] = SCALE2EXP(
                      -hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
                  gammaVec_e[0] = gammaVec_e[1] = SCALE2EXP(
                      -hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
                }
                break;

              case 3:
                wingain = FL2FXCONST_DBL(5.6568f /
                                         12.f); /* sum of taps divided by two */

                if (hQmfTransposer->bXProducts[1]) {
                  FIXP_DBL tmpReal_F, tmpImag_F;
                  if (mTr == 1) {
                    gammaCenterReal_m[0] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts1];
                    gammaCenterImag_m[0] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts1];
                    gammaVecReal_m[1] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts2];
                    gammaVecImag_m[1] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts2];

                    addrshift = -2;
                    tmpReal_F =
                        hQmfTransposer
                            ->qmfInBufReal_F[addrshift + slotOffset][ts2];
                    tmpImag_F =
                        hQmfTransposer
                            ->qmfInBufImag_F[addrshift + slotOffset][ts2];

                    gammaVecReal_m[0] =
                        (fMult(factors[ts2 % 4], tmpReal_F) -
                         fMult(factors[(ts2 + 3) % 4], tmpImag_F)) >>
                        1;
                    gammaVecImag_m[0] =
                        (fMult(factors[(ts2 + 3) % 4], tmpReal_F) +
                         fMult(factors[ts2 % 4], tmpImag_F)) >>
                        1;

                    tmpReal_F =
                        hQmfTransposer
                            ->qmfInBufReal_F[addrshift + 1 + slotOffset][ts2];
                    tmpImag_F =
                        hQmfTransposer
                            ->qmfInBufImag_F[addrshift + 1 + slotOffset][ts2];

                    gammaVecReal_m[0] +=
                        (fMult(factors[ts2 % 4], tmpReal_F) -
                         fMult(factors[(ts2 + 1) % 4], tmpImag_F)) >>
                        1;
                    gammaVecImag_m[0] +=
                        (fMult(factors[(ts2 + 1) % 4], tmpReal_F) +
                         fMult(factors[ts2 % 4], tmpImag_F)) >>
                        1;

                  } else /* (mTr == 2) */
                  {
                    sign = 1;
                    Tcenter = mTr; /* opposite phase power parameters as ts2 is
                                      center */
                    Tvec = stretch - mTr;

                    gammaCenterReal_m[0] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts2];
                    gammaCenterImag_m[0] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts2];
                    gammaVecReal_m[1] =
                        hQmfTransposer->qmfInBufReal_F[slotOffset][ts1];
                    gammaVecImag_m[1] =
                        hQmfTransposer->qmfInBufImag_F[slotOffset][ts1];

                    addrshift = -2;
                    tmpReal_F =
                        hQmfTransposer
                            ->qmfInBufReal_F[addrshift + slotOffset][ts1];
                    tmpImag_F =
                        hQmfTransposer
                            ->qmfInBufImag_F[addrshift + slotOffset][ts1];

                    gammaVecReal_m[0] =
                        (fMult(factors[ts1 % 4], tmpReal_F) -
                         fMult(factors[(ts1 + 3) % 4], tmpImag_F)) >>
                        1;
                    gammaVecImag_m[0] =
                        (fMult(factors[(ts1 + 3) % 4], tmpReal_F) +
                         fMult(factors[ts1 % 4], tmpImag_F)) >>
                        1;

                    tmpReal_F =
                        hQmfTransposer
                            ->qmfInBufReal_F[addrshift + 1 + slotOffset][ts1];
                    tmpImag_F =
                        hQmfTransposer
                            ->qmfInBufImag_F[addrshift + 1 + slotOffset][ts1];

                    gammaVecReal_m[0] +=
                        (fMult(factors[ts1 % 4], tmpReal_F) -
                         fMult(factors[(ts1 + 1) % 4], tmpImag_F)) >>
                        1;
                    gammaVecImag_m[0] +=
                        (fMult(factors[(ts1 + 1) % 4], tmpReal_F) +
                         fMult(factors[ts1 % 4], tmpImag_F)) >>
                        1;
                  }

                  gammaCenter_e[0] = gammaVec_e[1] = SCALE2EXP(
                      -hQmfTransposer->HBEAnalysiscQMF.outScalefactor);
                  gammaVec_e[0] =
                      SCALE2EXP(
                          -hQmfTransposer->HBEAnalysiscQMF.outScalefactor) +
                      1;
                }
                break;
              default:
                FDK_ASSERT(0);
                break;
            } /* stretch cases */

            /* parameter controlled phase modification parts */
            /* maximum *_e == 20 */
            calculateCenterFIXP(gammaCenterReal_m[0], gammaCenterImag_m[0],
                                &gammaCenterReal_m[0], &gammaCenterImag_m[0],
                                &gammaCenter_e[0], stretch, Tcenter - 1);
            calculateCenterFIXP(gammaVecReal_m[0], gammaVecImag_m[0],
                                &gammaVecReal_m[0], &gammaVecImag_m[0],
                                &gammaVec_e[0], stretch, Tvec - 1);
            calculateCenterFIXP(gammaVecReal_m[1], gammaVecImag_m[1],
                                &gammaVecReal_m[1], &gammaVecImag_m[1],
                                &gammaVec_e[1], stretch, Tvec - 1);

            /*    Final multiplication of prepared parts  */
            for (k = 0; k < 2; k++) {
              gammaOutReal_m[k] =
                  fMultDiv2(gammaVecReal_m[k], gammaCenterReal_m[0]) -
                  fMultDiv2(gammaVecImag_m[k], gammaCenterImag_m[0]);
              gammaOutImag_m[k] =
                  fMultDiv2(gammaVecReal_m[k], gammaCenterImag_m[0]) +
                  fMultDiv2(gammaVecImag_m[k], gammaCenterReal_m[0]);
              gammaOut_e[k] = gammaCenter_e[0] + gammaVec_e[k] + 1;
            }

            scaleUp(&gammaOutReal_m[0], &gammaOutImag_m[0], &gammaOut_e[0]);
            scaleUp(&gammaOutReal_m[1], &gammaOutImag_m[1], &gammaOut_e[1]);
            FDK_ASSERT(gammaOut_e[0] >= 0);
            FDK_ASSERT(gammaOut_e[0] < 32);

            tmpReal_m = gammaOutReal_m[0];
            tmpImag_m = gammaOutImag_m[0];

            INT modstretch4 = ((stretch == 4) && (mTr == 2));

            FIXP_DBL cos_twid = twid_m_new[stretch - 2 - modstretch4][0];
            FIXP_DBL sin_twid = sign * twid_m_new[stretch - 2 - modstretch4][1];

            gammaOutReal_m[0] =
                fMult(tmpReal_m, cos_twid) -
                fMult(tmpImag_m, sin_twid); /* sum should be <= 1 because of
                                               sin/cos multiplication */
            gammaOutImag_m[0] =
                fMult(tmpImag_m, cos_twid) +
                fMult(tmpReal_m, sin_twid); /* sum should be <= 1 because of
                                               sin/cos multiplication */

            /* wingain */
            for (k = 0; k < 2; k++) {
              gammaOutReal_m[k] = (fMult(gammaOutReal_m[k], wingain) << 1);
              gammaOutImag_m[k] = (fMult(gammaOutImag_m[k], wingain) << 1);
            }

            gammaOutReal_m[1] >>= 1;
            gammaOutImag_m[1] >>= 1;
            gammaOut_e[0] += 2;
            gammaOut_e[1] += 2;

            /* OLA including window scaling by wingain/3 */
            for (k = 0; k < 2; k++) /* need k=1 to correspond to
                                       grainModImag[slotOffset] -> out to
                                       j*2+(slotOffset-offset)  */
            {
              hQmfTransposer->qmfHBEBufReal_F[(k + slotOffset - 1)][band] +=
                  gammaOutReal_m[k] >> (scale_factor_hbe - gammaOut_e[k]);
              hQmfTransposer->qmfHBEBufImag_F[(k + slotOffset - 1)][band] +=
                  gammaOutImag_m[k] >> (scale_factor_hbe - gammaOut_e[k]);
            }
          } /* mVal > qThrQMF * qThrQMF * sqmag0 && ts1 > 0 && ts2 < 64 */
        }   /* p >= pmin */
      }     /* for band */
    }       /* for stretch */

    for (i = 0; i < QMF_WIN_LEN - 1; i++) {
      FDKmemcpy(hQmfTransposer->qmfInBufReal_F[i],
                hQmfTransposer->qmfInBufReal_F[i + 1],
                sizeof(FIXP_DBL) * hQmfTransposer->HBEAnalysiscQMF.no_channels);
      FDKmemcpy(hQmfTransposer->qmfInBufImag_F[i],
                hQmfTransposer->qmfInBufImag_F[i + 1],
                sizeof(FIXP_DBL) * hQmfTransposer->HBEAnalysiscQMF.no_channels);
    }

    if (keepStatesSyncedMode != KEEP_STATES_SYNCED_NOOUT) {
      if (2 * j >= offset) {
        /* copy first two slots of internal buffer to output */
        if (keepStatesSyncedMode == KEEP_STATES_SYNCED_OUTDIFF) {
          for (i = 0; i < 2; i++) {
            FDKmemcpy(&ppQmfBufferOutReal_F[2 * j - offset + i]
                                           [hQmfTransposer->xOverQmf[0]],
                      &hQmfTransposer
                           ->qmfHBEBufReal_F[i][hQmfTransposer->xOverQmf[0]],
                      (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                          sizeof(FIXP_DBL));
            FDKmemcpy(&ppQmfBufferOutImag_F[2 * j - offset + i]
                                           [hQmfTransposer->xOverQmf[0]],
                      &hQmfTransposer
                           ->qmfHBEBufImag_F[i][hQmfTransposer->xOverQmf[0]],
                      (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                          sizeof(FIXP_DBL));
          }
        } else {
          for (i = 0; i < 2; i++) {
            FDKmemcpy(&ppQmfBufferOutReal_F[2 * j + i + ov_len]
                                           [hQmfTransposer->xOverQmf[0]],
                      &hQmfTransposer
                           ->qmfHBEBufReal_F[i][hQmfTransposer->xOverQmf[0]],
                      (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                          sizeof(FIXP_DBL));
            FDKmemcpy(&ppQmfBufferOutImag_F[2 * j + i + ov_len]
                                           [hQmfTransposer->xOverQmf[0]],
                      &hQmfTransposer
                           ->qmfHBEBufImag_F[i][hQmfTransposer->xOverQmf[0]],
                      (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                          sizeof(FIXP_DBL));
          }
        }
      }
    }

    /* move slots up */
    for (i = 0; i < HBE_MAX_OUT_SLOTS - 2; i++) {
      FDKmemcpy(
          &hQmfTransposer->qmfHBEBufReal_F[i][hQmfTransposer->xOverQmf[0]],
          &hQmfTransposer->qmfHBEBufReal_F[i + 2][hQmfTransposer->xOverQmf[0]],
          (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
              sizeof(FIXP_DBL));
      FDKmemcpy(
          &hQmfTransposer->qmfHBEBufImag_F[i][hQmfTransposer->xOverQmf[0]],
          &hQmfTransposer->qmfHBEBufImag_F[i + 2][hQmfTransposer->xOverQmf[0]],
          (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
              sizeof(FIXP_DBL));
    }

    /* finally set last two slot to zero */
    for (i = 0; i < 2; i++) {
      FDKmemset(&hQmfTransposer->qmfHBEBufReal_F[HBE_MAX_OUT_SLOTS - 1 - i]
                                                [hQmfTransposer->xOverQmf[0]],
                0,
                (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                    sizeof(FIXP_DBL));
      FDKmemset(&hQmfTransposer->qmfHBEBufImag_F[HBE_MAX_OUT_SLOTS - 1 - i]
                                                [hQmfTransposer->xOverQmf[0]],
                0,
                (QMF_SYNTH_CHANNELS - hQmfTransposer->xOverQmf[0]) *
                    sizeof(FIXP_DBL));
    }
  } /* qmfVocoderColsIn */

  if (keepStatesSyncedMode != KEEP_STATES_SYNCED_NOOUT) {
    if (keepStatesSyncedMode == KEEP_STATES_SYNCED_OUTDIFF) {
      for (i = 0; i < ov_len + LPC_ORDER; i++) {
        for (band = hQmfTransposer->startBand; band < hQmfTransposer->stopBand;
             band++) {
          FIXP_DBL tmpR = ppQmfBufferOutReal_F[i][band];
          FIXP_DBL tmpI = ppQmfBufferOutImag_F[i][band];

          ppQmfBufferOutReal_F[i][band] =
              fMult(tmpR, cos_F[band]) -
              fMult(tmpI, (-cos_F[64 - band - 1])); /* sum should be <= 1
                                                       because of sin/cos
                                                       multiplication */
          ppQmfBufferOutImag_F[i][band] =
              fMult(tmpR, (-cos_F[64 - band - 1])) +
              fMult(tmpI, cos_F[band]); /* sum should by <= 1 because of sin/cos
                                           multiplication */
        }
      }
    } else {
      for (i = offset; i < hQmfTransposer->noCols; i++) {
        for (band = hQmfTransposer->startBand; band < hQmfTransposer->stopBand;
             band++) {
          FIXP_DBL tmpR = ppQmfBufferOutReal_F[i + ov_len][band];
          FIXP_DBL tmpI = ppQmfBufferOutImag_F[i + ov_len][band];

          ppQmfBufferOutReal_F[i + ov_len][band] =
              fMult(tmpR, cos_F[band]) -
              fMult(tmpI, (-cos_F[64 - band - 1])); /* sum should be <= 1
                                                       because of sin/cos
                                                       multiplication */
          ppQmfBufferOutImag_F[i + ov_len][band] =
              fMult(tmpR, (-cos_F[64 - band - 1])) +
              fMult(tmpI, cos_F[band]); /* sum should by <= 1 because of sin/cos
                                           multiplication */
        }
      }
    }
  }

  *scale_hb = EXP2SCALE(scale_factor_hbe);
}

int* GetxOverBandQmfTransposer(HANDLE_HBE_TRANSPOSER hQmfTransposer) {
  if (hQmfTransposer)
    return hQmfTransposer->xOverQmf;
  else
    return NULL;
}

int Get41SbrQmfTransposer(HANDLE_HBE_TRANSPOSER hQmfTransposer) {
  if (hQmfTransposer != NULL)
    return hQmfTransposer->bSbr41;
  else
    return 0;
}
