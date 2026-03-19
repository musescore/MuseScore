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

/******************* Library for basic calculation routines ********************

   Author(s):   Markus Lohwasser

   Description: FDK Tools Decorrelator

*******************************************************************************/

#include "FDK_decorrelate.h"

#define PC_NUM_BANDS (8)
#define PC_NUM_HYB_BANDS (PC_NUM_BANDS - 3 + 10)

#define DUCK_ALPHA (0.8f)
#define DUCK_GAMMA (1.5f)
#define ABS_THR (1e-9f * 32768 * 32768)
#define ABS_THR_FDK ((FIXP_DBL)1)

#define DECORR_ZERO_PADDING 0

#define DECORR_FILTER_ORDER_BAND_0_MPS (20)
#define DECORR_FILTER_ORDER_BAND_1_MPS (15)
#define DECORR_FILTER_ORDER_BAND_2_MPS (6)
#define DECORR_FILTER_ORDER_BAND_3_MPS (3)

#define DECORR_FILTER_ORDER_BAND_0_USAC (10)
#define DECORR_FILTER_ORDER_BAND_1_USAC (8)
#define DECORR_FILTER_ORDER_BAND_2_USAC (3)
#define DECORR_FILTER_ORDER_BAND_3_USAC (2)

#define DECORR_FILTER_ORDER_BAND_0_LD (0)
#define DECORR_FILTER_ORDER_BAND_1_LD (DECORR_FILTER_ORDER_BAND_1_MPS)
#define DECORR_FILTER_ORDER_BAND_2_LD (DECORR_FILTER_ORDER_BAND_2_MPS)
#define DECORR_FILTER_ORDER_BAND_3_LD (DECORR_FILTER_ORDER_BAND_3_MPS)

#define MAX_DECORR_SEED_MPS \
  (5) /* 4 is worst case for 7272 mode for low power */
      /* 5 is worst case for 7271 and 7272 mode for high quality */
#define MAX_DECORR_SEED_USAC (1)
#define MAX_DECORR_SEED_LD (4)

#define DECORR_FILTER_ORDER_PS (12)
#define NUM_DECORR_CONFIGS \
  (3) /* different configs defined by bsDecorrConfig bitstream field */

/* REV_bandOffset_... tables map (hybrid) bands to the corresponding reverb
   bands. Within each reverb band the same processing is applied. Instead of QMF
   split frequencies the corresponding hybrid band offsets are stored directly
 */
static const UCHAR REV_bandOffset_MPS_HQ[NUM_DECORR_CONFIGS][(4)] = {
    {8, 21, 30, 71}, {8, 56, 71, 71}, {0, 21, 71, 71}};
/* REV_bandOffset_USAC[] are equivalent to REV_bandOffset_MPS_HQ */
static const UCHAR REV_bandOffset_PS_HQ[(4)] = {30, 42, 71, 71};
static const UCHAR REV_bandOffset_PS_LP[(4)] = {14, 42, 71, 71};
static const UCHAR REV_bandOffset_LD[NUM_DECORR_CONFIGS][(4)] = {
    {0, 14, 23, 64}, {0, 49, 64, 64}, {0, 14, 64, 64}};

/* REV_delay_... tables define the number of delay elements within each reverb
 * band */
/* REV_filterOrder_... tables define the filter order within each reverb band */
static const UCHAR REV_delay_MPS[(4)] = {8, 7, 2, 1};
static const SCHAR REV_filterOrder_MPS[(4)] = {
    DECORR_FILTER_ORDER_BAND_0_MPS, DECORR_FILTER_ORDER_BAND_1_MPS,
    DECORR_FILTER_ORDER_BAND_2_MPS, DECORR_FILTER_ORDER_BAND_3_MPS};
static const UCHAR REV_delay_PS_HQ[(4)] = {2, 14, 1, 0};
static const UCHAR REV_delay_PS_LP[(4)] = {8, 14, 1, 0};
static const SCHAR REV_filterOrder_PS[(4)] = {DECORR_FILTER_ORDER_PS, -1, -1,
                                              -1};
static const UCHAR REV_delay_USAC[(4)] = {11, 10, 5, 2};
static const SCHAR REV_filterOrder_USAC[(4)] = {
    DECORR_FILTER_ORDER_BAND_0_USAC, DECORR_FILTER_ORDER_BAND_1_USAC,
    DECORR_FILTER_ORDER_BAND_2_USAC, DECORR_FILTER_ORDER_BAND_3_USAC};

/* REV_filtType_... tables define the type of processing (filtering with
   different properties or pure delay) done in each reverb band. This is mapped
   to specialized routines. */
static const REVBAND_FILT_TYPE REV_filtType_MPS[(4)] = {
    COMMON_REAL, COMMON_REAL, COMMON_REAL, COMMON_REAL};

static const REVBAND_FILT_TYPE REV_filtType_PS[(4)] = {INDEP_CPLX_PS, DELAY,
                                                       DELAY, NOT_EXIST};

/* initialization values of ring buffer offsets for the 3 concatenated allpass
 * filters (PS type decorrelator). */
static const UCHAR stateBufferOffsetInit[(3)] = {0, 6, 14};

static const REVBAND_FILT_TYPE REV_filtType_LD[(4)] = {
    NOT_EXIST, COMMON_REAL, COMMON_REAL, COMMON_REAL};

/*** mapping of hybrid bands to processing (/parameter?) bands ***/
/* table for PS decorr running in legacy PS decoder. */
static const UCHAR kernels_20_to_71_PS[(71) + 1] = {
    0,  0,  1,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 14,
    15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19};

/*** mapping of processing (/parameter?) bands to hybrid bands ***/
/* table for PS decorr running in legacy PS decoder. */
static const UCHAR kernels_20_to_71_offset_PS[(20) + 1] = {
    0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 21, 25, 30, 42, 71};

static const UCHAR kernels_28_to_71[(71) + 1] = {
    0,  0,  1,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
    23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26,
    26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27};

static const UCHAR kernels_28_to_71_offset[(28) + 1] = {
    0,  2,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 21, 23, 25, 27, 30, 33, 37, 42, 48, 55, 71};

/* LD-MPS defined in SAOC standart (mapping qmf -> param bands)*/
static const UCHAR kernels_23_to_64[(64) + 1] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 12, 13, 13, 14,
    14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19,
    19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
};

static const UCHAR kernels_23_to_64_offset[(23) + 1] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    12, 14, 16, 18, 20, 23, 26, 30, 35, 41, 48, 64};

static inline int SpatialDecGetProcessingBand(int hybridBand,
                                              const UCHAR *tab) {
  return tab[hybridBand];
}

/* helper inline function */
static inline int SpatialDecGetQmfBand(int paramBand, const UCHAR *tab) {
  return (int)tab[paramBand];
}

#define DUCKER_MAX_NRG_SCALE (24)
#define DUCKER_HEADROOM_BITS (2)

#define FILTER_SF (2)

#ifdef ARCH_PREFER_MULT_32x32
#define FIXP_DUCK_GAIN FIXP_DBL
#define FX_DBL2FX_DUCK_GAIN
#define FL2FXCONST_DUCK FL2FXCONST_DBL
#else
#define FIXP_DUCK_GAIN FIXP_SGL
#define FX_DBL2FX_DUCK_GAIN FX_DBL2FX_SGL
#define FL2FXCONST_DUCK FL2FXCONST_SGL
#endif
#define PS_DUCK_PEAK_DECAY_FACTOR (0.765928338364649f)
#define PS_DUCK_FILTER_COEFF (0.25f)
#define DUCK_ALPHA_FDK FL2FXCONST_DUCK(DUCK_ALPHA)
#define DUCK_ONE_MINUS_ALPHA_X4_FDK FL2FXCONST_DUCK(4.0f * (1.0f - DUCK_ALPHA))
#define DUCK_GAMMA_FDK FL2FXCONST_DUCK(DUCK_GAMMA / 2)
#define PS_DUCK_PEAK_DECAY_FACTOR_FDK FL2FXCONST_DUCK(PS_DUCK_PEAK_DECAY_FACTOR)
#define PS_DUCK_FILTER_COEFF_FDK FL2FXCONST_DUCK(PS_DUCK_FILTER_COEFF)
RAM_ALIGN
const FIXP_STP DecorrPsCoeffsCplx[][4] = {
    {STCP(0x5d6940eb, 0x5783153e), STCP(0xadcd41a8, 0x0e0373ed),
     STCP(0xbad41f3e, 0x14fba045), STCP(0xc1eb6694, 0x0883227d)},
    {STCP(0x5d6940eb, 0xa87ceac2), STCP(0xadcd41a8, 0xf1fc8c13),
     STCP(0xbad41f3e, 0xeb045fbb), STCP(0xc1eb6694, 0xf77cdd83)},
    {STCP(0xaec24162, 0x62e9d75b), STCP(0xb7169316, 0x28751048),
     STCP(0xd224c0cc, 0x37e05050), STCP(0xc680864f, 0x18e88cba)},
    {STCP(0xaec24162, 0x9d1628a5), STCP(0xb7169316, 0xd78aefb8),
     STCP(0xd224c0cc, 0xc81fafb0), STCP(0xc680864f, 0xe7177346)},
    {STCP(0x98012341, 0x4aa00ed1), STCP(0xc89ca1b2, 0xc1ab6bff),
     STCP(0xf8ea394e, 0xb8106bf4), STCP(0xcf542d73, 0xd888b99b)},
    {STCP(0x43b137b3, 0x6ca2ca40), STCP(0xe0649cc4, 0xb2d69cca),
     STCP(0x22130c21, 0xc0405382), STCP(0xdbbf8fba, 0xcce3c7cc)},
    {STCP(0x28fc4d71, 0x86bd3b87), STCP(0x09ccfeb9, 0xad319baf),
     STCP(0x46e51f02, 0xf1e5ea55), STCP(0xf30d5e34, 0xc2b0e335)},
    {STCP(0xc798f756, 0x72e73c7d), STCP(0x3b6c3c1e, 0xc580dc72),
     STCP(0x2828a6ba, 0x3c1a14fb), STCP(0x14b733bb, 0xc4dcaae1)},
    {STCP(0x46dcadd3, 0x956795c7), STCP(0x52f32fae, 0xf78048cd),
     STCP(0xd7d75946, 0x3c1a14fb), STCP(0x306017cb, 0xd82c0a75)},
    {STCP(0xabe197de, 0x607a675e), STCP(0x460cef6e, 0x2d3b264e),
     STCP(0xb91ae0fe, 0xf1e5ea55), STCP(0x3e03e5e0, 0xf706590e)},
    {STCP(0xb1b4f509, 0x9abcaf5f), STCP(0xfeb0b4be, 0x535fb8ba),
     STCP(0x1ba96f8e, 0xbd37e6d8), STCP(0x30f6dbbb, 0x271a0743)},
    {STCP(0xce75b52a, 0x89f9be61), STCP(0xb26e4dda, 0x101054c5),
     STCP(0x1a475d2e, 0x3f714b19), STCP(0xf491f154, 0x3a6baf46)},
    {STCP(0xee8fdfcb, 0x813181fa), STCP(0xe11e1a00, 0xbb9a6039),
     STCP(0xc3e582f5, 0xe71ab533), STCP(0xc9eb35e2, 0x0ffd212a)},
    {STCP(0x0fd7d92f, 0x80fbf975), STCP(0x38adccbc, 0xd571bbf4),
     STCP(0x38c3aefc, 0xe87cc794), STCP(0xdafe8c3d, 0xd9b16100)},
    {STCP(0x300d9e10, 0x895cc359), STCP(0x32b9843e, 0x2b52adcc),
     STCP(0xe9ded9f4, 0x356ce0ed), STCP(0x0fdd5ca3, 0xd072932e)},
    {STCP(0x4d03b4f8, 0x99c2dec3), STCP(0xe2bc8d94, 0x3744e195),
     STCP(0xeb40ec55, 0xcde9ed22), STCP(0x2e67e231, 0xf893470b)},
    {STCP(0x64c4deb3, 0xb112790f), STCP(0xc7b32682, 0xf099172d),
     STCP(0x2ebf44cf, 0x135d014a), STCP(0x1a2bacd5, 0x23334254)},
    {STCP(0x75b5f9aa, 0xcdb81e14), STCP(0x028d9bb1, 0xc9dc45b9),
     STCP(0xd497893f, 0x11faeee9), STCP(0xee40ff71, 0x24a91b85)},
    {STCP(0x7eb1cd81, 0xedc3feec), STCP(0x31491897, 0xf765f6d8),
     STCP(0x1098dc89, 0xd7ee574e), STCP(0xda6b816d, 0x011f35cf)},
    {STCP(0x7f1cde01, 0x0f0b7727), STCP(0x118ce49d, 0x2a5ecda4),
     STCP(0x0f36ca28, 0x24badaa3), STCP(0xef2908a4, 0xe1ee3743)},
    {STCP(0x76efee25, 0x2f4e8c3a), STCP(0xdde3be2a, 0x17f92215),
     STCP(0xde9bf36c, 0xf22b4839), STCP(0x1128fc0c, 0xe5c95f5a)},
    {STCP(0x66b87d65, 0x4c5ede42), STCP(0xe43f351a, 0xe6bf22dc),
     STCP(0x1e0d3e85, 0xf38d5a9a), STCP(0x1c0f44a3, 0x02c92fe3)},
    {STCP(0x4f8f36b7, 0x6445680f), STCP(0x10867ea2, 0xe3072740),
     STCP(0xf4ef6cfa, 0x1ab67076), STCP(0x09562a8a, 0x1742bb8b)},
    {STCP(0x3304f6ec, 0x7564812a), STCP(0x1be4f1a8, 0x0894d75a),
     STCP(0xf6517f5b, 0xe8a05d98), STCP(0xf1bb0053, 0x10a78853)},
    {STCP(0x1307b2c5, 0x7e93d532), STCP(0xfe098e27, 0x18f02a58),
     STCP(0x1408d459, 0x084c6e44), STCP(0xedafe5bd, 0xfbc15b2e)},
    {STCP(0xf1c111cd, 0x7f346c97), STCP(0xeb5ca6a0, 0x02efee93),
     STCP(0xef4df9b6, 0x06ea5be4), STCP(0xfc149289, 0xf0d53ce4)},
    {STCP(0xd1710001, 0x773b6beb), STCP(0xfa1aeb8c, 0xf06655ff),
     STCP(0x05884983, 0xf2a4c7c5), STCP(0x094f13df, 0xf79c01bf)},
    {STCP(0xb446be0b, 0x6732cfca), STCP(0x0a743752, 0xf9220dfa),
     STCP(0x04263722, 0x0a046a2c), STCP(0x08ced80b, 0x0347e9c2)},
    {STCP(0x9c3b1202, 0x503018a5), STCP(0x05fcf01a, 0x05cd8529),
     STCP(0xf95263e2, 0xfd3bdb3f), STCP(0x00c68cf9, 0x0637cb7f)},
    {STCP(0x8aee2710, 0x33c187ec), STCP(0xfdd253f8, 0x038e09b9),
     STCP(0x0356ce0f, 0xfe9ded9f), STCP(0xfd6c3054, 0x01c8060a)}};

const FIXP_DECORR DecorrNumeratorReal0_USAC
    [MAX_DECORR_SEED_USAC][DECORR_FILTER_ORDER_BAND_0_USAC + 1] = {
        {DECORR(0x05bf4880), DECORR(0x08321c00), DECORR(0xe9315ee0),
         DECORR(0x07d9dd20), DECORR(0x02224994), DECORR(0x0009d200),
         DECORR(0xf8a29358), DECORR(0xf4e310d0), DECORR(0xef901fc0),
         DECORR(0xebda0460), DECORR(0x40000000)}};

const FIXP_DECORR DecorrNumeratorReal1_USAC
    [MAX_DECORR_SEED_USAC][DECORR_FILTER_ORDER_BAND_1_USAC + 1] = {
        {DECORR(0xf82f8378), DECORR(0xfef588c2), DECORR(0x02eddbd8),
         DECORR(0x041c2450), DECORR(0xf7edcd60), DECORR(0x07e29310),
         DECORR(0xfa4ece48), DECORR(0xed9f8a20), DECORR(0x40000000)}};

/* identical to MPS coeffs for reverb band 3: DecorrNumeratorReal3[0] */
const FIXP_DECORR
    DecorrNumeratorReal2_USAC[MAX_DECORR_SEED_USAC]
                             [DECORR_FILTER_ORDER_BAND_2_USAC + 1] = {
                                 {DECORR(0x0248e8a8), DECORR(0xfde95838),
                                  DECORR(0x084823c0), DECORR(0x40000000)}};

const FIXP_DECORR
    DecorrNumeratorReal3_USAC[MAX_DECORR_SEED_USAC]
                             [DECORR_FILTER_ORDER_BAND_3_USAC + 1] = {
                                 {DECORR(0xff2b020c), DECORR(0x02393830),
                                  DECORR(0x40000000)}};

/* const FIXP_DECORR DecorrNumeratorReal0_LD[MAX_DECORR_SEED_LD][] does not
 * exist */

RAM_ALIGN
const FIXP_DECORR DecorrNumeratorReal1_LD[MAX_DECORR_SEED_LD]
                                         [DECORR_FILTER_ORDER_BAND_1_LD + 1] = {
                                             {
                                                 DECORR(0xf310cb29),
                                                 DECORR(0x1932d745),
                                                 DECORR(0x0cc2d917),
                                                 DECORR(0xddde064e),
                                                 DECORR(0xf234a626),
                                                 DECORR(0x198551a6),
                                                 DECORR(0x17141b6a),
                                                 DECORR(0xf298803d),
                                                 DECORR(0xef98be92),
                                                 DECORR(0x09ea1706),
                                                 DECORR(0x28fbdff4),
                                                 DECORR(0x1a869eb9),
                                                 DECORR(0xdeefe147),
                                                 DECORR(0xcde2adda),
                                                 DECORR(0x13ddc619),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0x041d7dbf),
                                                 DECORR(0x01b7309c),
                                                 DECORR(0xfb599834),
                                                 DECORR(0x092fc5ed),
                                                 DECORR(0xf2fd7c25),
                                                 DECORR(0xdd51e2eb),
                                                 DECORR(0xf62fe72b),
                                                 DECORR(0x0b15d588),
                                                 DECORR(0xf1f091a7),
                                                 DECORR(0xed1bbbfe),
                                                 DECORR(0x03526899),
                                                 DECORR(0x180cb256),
                                                 DECORR(0xecf1433d),
                                                 DECORR(0xf626ab95),
                                                 DECORR(0x197dd27e),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0x157a786c),
                                                 DECORR(0x0028c98c),
                                                 DECORR(0xf5eff57b),
                                                 DECORR(0x11f7d04f),
                                                 DECORR(0xf390d28d),
                                                 DECORR(0x18947081),
                                                 DECORR(0xe5dc2319),
                                                 DECORR(0xf4cc0235),
                                                 DECORR(0x2394d47f),
                                                 DECORR(0xe069230e),
                                                 DECORR(0x03a1a773),
                                                 DECORR(0xfbc9b092),
                                                 DECORR(0x15a0173b),
                                                 DECORR(0x0e9ecdf0),
                                                 DECORR(0xd309b2c7),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0xe0ce703b),
                                                 DECORR(0xe508b672),
                                                 DECORR(0xef362398),
                                                 DECORR(0xffe788ef),
                                                 DECORR(0x2fda3749),
                                                 DECORR(0x4671c0c6),
                                                 DECORR(0x3c003494),
                                                 DECORR(0x2387707c),
                                                 DECORR(0xd2107d2e),
                                                 DECORR(0xb3e47e08),
                                                 DECORR(0xacd0abca),
                                                 DECORR(0xc70791df),
                                                 DECORR(0x0b586e85),
                                                 DECORR(0x2f11cda7),
                                                 DECORR(0x3a4a210b),
                                                 DECORR(0x40000000),
                                             },
};

RAM_ALIGN
const FIXP_DECORR DecorrNumeratorReal2_LD[MAX_DECORR_SEED_LD]
                                         [DECORR_FILTER_ORDER_BAND_2_LD + 1 +
                                          DECORR_ZERO_PADDING] = {
                                             {
                                                 DECORR(0xffb4a234),
                                                 DECORR(0x01ac71a2),
                                                 DECORR(0xf2bca010),
                                                 DECORR(0xfe3d7593),
                                                 DECORR(0x093e9976),
                                                 DECORR(0xf2c5f3f5),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0xe303afb8),
                                                 DECORR(0xcd70c2bb),
                                                 DECORR(0xf1e2ad7e),
                                                 DECORR(0x0c8ffbe2),
                                                 DECORR(0x21f80abf),
                                                 DECORR(0x3d08410c),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0xe26809d5),
                                                 DECORR(0x0efbcfa4),
                                                 DECORR(0x210c1a97),
                                                 DECORR(0xfe60af4e),
                                                 DECORR(0xeda01a51),
                                                 DECORR(0x00faf468),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0x1edc5d64),
                                                 DECORR(0xe5b2e35c),
                                                 DECORR(0xe94b1c45),
                                                 DECORR(0x30a6f1e1),
                                                 DECORR(0xf04e52de),
                                                 DECORR(0xe30de45a),
                                                 DECORR(0x40000000),
                                             },
};

RAM_ALIGN
const FIXP_DECORR DecorrNumeratorReal3_LD[MAX_DECORR_SEED_LD]
                                         [DECORR_FILTER_ORDER_BAND_3_LD + 1] = {
                                             {
                                                 DECORR(0x0248e8a7),
                                                 DECORR(0xfde9583b),
                                                 DECORR(0x084823bb),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0x1db22d0e),
                                                 DECORR(0xfc773992),
                                                 DECORR(0x0e819a74),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0x0fcb923a),
                                                 DECORR(0x0154b7ff),
                                                 DECORR(0xe70cb647),
                                                 DECORR(0x40000000),
                                             },
                                             {
                                                 DECORR(0xe39f559b),
                                                 DECORR(0xe06dd6ca),
                                                 DECORR(0x19f71f71),
                                                 DECORR(0x40000000),
                                             },
};

FIXP_DBL *getAddrDirectSignalMaxVal(HANDLE_DECORR_DEC self) {
  return &(self->ducker.maxValDirectData);
}

static INT DecorrFilterInit(DECORR_FILTER_INSTANCE *const self,
                            FIXP_MPS *pStateBufferCplx,
                            FIXP_DBL *pDelayBufferCplx, INT *offsetStateBuffer,
                            INT *offsetDelayBuffer, INT const decorr_seed,
                            INT const reverb_band, INT const useFractDelay,
                            INT const noSampleDelay, INT const filterOrder,
                            FDK_DECORR_TYPE const decorrType) {
  INT errorCode = 0;
  switch (decorrType) {
    case DECORR_USAC:
      if (useFractDelay) {
        return 1;
      } else {
        FDK_ASSERT(decorr_seed == 0);

        switch (reverb_band) {
          case 0:
            self->numeratorReal = DecorrNumeratorReal0_USAC[decorr_seed];
            break;
          case 1:
            self->numeratorReal = DecorrNumeratorReal1_USAC[decorr_seed];
            break;
          case 2:
            self->numeratorReal = DecorrNumeratorReal2_USAC[decorr_seed];
            break;
          case 3:
            self->numeratorReal = DecorrNumeratorReal3_USAC[decorr_seed];
            break;
        }
      }
      break;
    case DECORR_LD:
      FDK_ASSERT(decorr_seed < MAX_DECORR_SEED_LD);
      switch (reverb_band) {
        case 0:
          self->numeratorReal = NULL;
          break;
        case 1:
          self->numeratorReal = DecorrNumeratorReal1_LD[decorr_seed];
          break;
        case 2:
          self->numeratorReal = DecorrNumeratorReal2_LD[decorr_seed];
          break;
        case 3:
          self->numeratorReal = DecorrNumeratorReal3_LD[decorr_seed];
          break;
      }
      break;
    default:
      return 1;
  }

  self->stateCplx = pStateBufferCplx + (*offsetStateBuffer);
  *offsetStateBuffer += 2 * filterOrder;
  self->DelayBufferCplx = pDelayBufferCplx + (*offsetDelayBuffer);
  *offsetDelayBuffer += 2 * noSampleDelay;

  return errorCode;
}

/*******************************************************************************
*******************************************************************************/
static INT DecorrFilterInitPS(DECORR_FILTER_INSTANCE *const self,
                              FIXP_MPS *pStateBufferCplx,
                              FIXP_DBL *pDelayBufferCplx,
                              INT *offsetStateBuffer, INT *offsetDelayBuffer,
                              INT const hybridBand, INT const reverbBand,
                              INT const noSampleDelay) {
  INT errorCode = 0;

  if (reverbBand == 0) {
    self->coeffsPacked = DecorrPsCoeffsCplx[hybridBand];

    self->stateCplx = pStateBufferCplx + (*offsetStateBuffer);
    *offsetStateBuffer += 2 * DECORR_FILTER_ORDER_PS;
  }

  self->DelayBufferCplx = pDelayBufferCplx + (*offsetDelayBuffer);
  *offsetDelayBuffer += 2 * noSampleDelay;

  return errorCode;
}

LNK_SECTION_CODE_L1
static INT DecorrFilterApplyPASS(DECORR_FILTER_INSTANCE const filter[],
                                 FIXP_DBL *dataRealIn, FIXP_DBL *dataImagIn,
                                 FIXP_DBL *dataRealOut, FIXP_DBL *dataImagOut,
                                 INT start, INT stop,
                                 INT reverbBandNoSampleDelay,
                                 INT reverbBandDelayBufferIndex) {
  INT i;
  INT offset = 2 * reverbBandNoSampleDelay;
  FIXP_MPS *pDelayBuffer =
      &filter[start].DelayBufferCplx[reverbBandDelayBufferIndex];

  /* Memory for the delayline has been allocated in a consecutive order, so we
     can address from filter to filter with a constant length.
     Be aware that real and imaginary part of the delayline are stored in
     interleaved order.
  */
  if (dataImagIn == NULL) {
    for (i = start; i < stop; i++) {
      FIXP_DBL tmp;

      tmp = *pDelayBuffer;
      *pDelayBuffer = dataRealIn[i];
      dataRealOut[i] = tmp;
      pDelayBuffer += offset;
    }
  } else {
    if ((i = stop - start) != 0) {
      dataRealIn += start;
      dataImagIn += start;
      dataRealOut += start;
      dataImagOut += start;
      do {
        FIXP_DBL delay_re, delay_im, real, imag;

        real = *dataRealIn++;
        imag = *dataImagIn++;
        delay_re = pDelayBuffer[0];
        delay_im = pDelayBuffer[1];
        pDelayBuffer[0] = real;
        pDelayBuffer[1] = imag;
        *dataRealOut++ = delay_re;
        *dataImagOut++ = delay_im;
        pDelayBuffer += offset;
      } while (--i != 0);
    }
  }

  return (INT)0;
}

#ifndef FUNCTION_DecorrFilterApplyREAL
LNK_SECTION_CODE_L1
static INT DecorrFilterApplyREAL(DECORR_FILTER_INSTANCE const filter[],
                                 FIXP_DBL *dataRealIn, FIXP_DBL *dataImagIn,
                                 FIXP_DBL *dataRealOut, FIXP_DBL *dataImagOut,
                                 INT start, INT stop, INT reverbFilterOrder,
                                 INT reverbBandNoSampleDelay,
                                 INT reverbBandDelayBufferIndex) {
  INT i, j;
  FIXP_DBL xReal, xImag, yReal, yImag;

  const FIXP_DECORR *pFilter = filter[start].numeratorReal;

  INT offsetDelayBuffer = (2 * reverbBandNoSampleDelay) - 1;
  FIXP_MPS *pDelayBuffer =
      &filter[start].DelayBufferCplx[reverbBandDelayBufferIndex];

  INT offsetStates = 2 * reverbFilterOrder;
  FIXP_DBL *pStates = filter[start].stateCplx;

  /* Memory for the delayline has been allocated in a consecutive order, so we
     can address from filter to filter with a constant length. The same is valid
     for the states.
     Be aware that real and imaginary part of the delayline and the states are
     stored in interleaved order.
     All filter in a reverb band have the same filter coefficients.
     Exploit symmetry: numeratorReal[i] =
     denominatorReal[reverbFilterLength-1-i] Do not accumulate the highest
     states which are always zero.
  */
  if (reverbFilterOrder == 2) {
    FIXP_DECORR nFilt0L, nFilt0H;

    nFilt0L = pFilter[0];
    nFilt0H = pFilter[1];

    for (i = start; i < stop; i++) {
      xReal = *pDelayBuffer;
      *pDelayBuffer = dataRealIn[i];
      pDelayBuffer++;

      xImag = *pDelayBuffer;
      *pDelayBuffer = dataImagIn[i];
      pDelayBuffer += offsetDelayBuffer;

      yReal = (pStates[0] + fMultDiv2(xReal, nFilt0L)) << FILTER_SF;
      yImag = (pStates[1] + fMultDiv2(xImag, nFilt0L)) << FILTER_SF;

      dataRealOut[i] = yReal;
      dataImagOut[i] = yImag;

      pStates[0] =
          pStates[2] + fMultDiv2(xReal, nFilt0H) - fMultDiv2(yReal, nFilt0H);
      pStates[1] =
          pStates[3] + fMultDiv2(xImag, nFilt0H) - fMultDiv2(yImag, nFilt0H);
      pStates[2] = (xReal >> FILTER_SF) - fMultDiv2(yReal, nFilt0L);
      pStates[3] = (xImag >> FILTER_SF) - fMultDiv2(yImag, nFilt0L);
      pStates += offsetStates;
    }
  } else if (reverbFilterOrder == 3) {
    FIXP_DECORR nFilt0L, nFilt0H, nFilt1L;

    nFilt0L = pFilter[0];
    nFilt0H = pFilter[1];
    nFilt1L = pFilter[2];

    for (i = start; i < stop; i++) {
      xReal = *pDelayBuffer;
      *pDelayBuffer = dataRealIn[i];
      pDelayBuffer++;

      xImag = *pDelayBuffer;
      *pDelayBuffer = dataImagIn[i];
      pDelayBuffer += offsetDelayBuffer;

      yReal = (pStates[0] + fMultDiv2(xReal, nFilt0L)) << FILTER_SF;
      yImag = (pStates[1] + fMultDiv2(xImag, nFilt0L)) << FILTER_SF;

      dataRealOut[i] = yReal;
      dataImagOut[i] = yImag;

      pStates[0] =
          pStates[2] + fMultDiv2(xReal, nFilt0H) - fMultDiv2(yReal, nFilt1L);
      pStates[1] =
          pStates[3] + fMultDiv2(xImag, nFilt0H) - fMultDiv2(yImag, nFilt1L);
      pStates[2] =
          pStates[4] + fMultDiv2(xReal, nFilt1L) - fMultDiv2(yReal, nFilt0H);
      pStates[3] =
          pStates[5] + fMultDiv2(xImag, nFilt1L) - fMultDiv2(yImag, nFilt0H);
      pStates[4] = (xReal >> FILTER_SF) - fMultDiv2(yReal, nFilt0L);
      pStates[5] = (xImag >> FILTER_SF) - fMultDiv2(yImag, nFilt0L);
      pStates += offsetStates;
    }
  } else if (reverbFilterOrder == 6) {
    FIXP_DECORR nFilt0L, nFilt0H, nFilt1L, nFilt1H, nFilt2L, nFilt2H;

    nFilt0L = pFilter[0];
    nFilt0H = pFilter[1];
    nFilt1L = pFilter[2];
    nFilt1H = pFilter[3];
    nFilt2L = pFilter[4];
    nFilt2H = pFilter[5];

    for (i = start; i < stop; i++) {
      xReal = *pDelayBuffer;
      *pDelayBuffer = dataRealIn[i];
      pDelayBuffer++;

      xImag = *pDelayBuffer;
      *pDelayBuffer = dataImagIn[i];
      pDelayBuffer += offsetDelayBuffer;

      yReal = (pStates[0] + fMultDiv2(xReal, nFilt0L)) << FILTER_SF;
      yImag = (pStates[1] + fMultDiv2(xImag, nFilt0L)) << FILTER_SF;
      dataRealOut[i] = yReal;
      dataImagOut[i] = yImag;

      pStates[0] =
          pStates[2] + fMultDiv2(xReal, nFilt0H) - fMultDiv2(yReal, nFilt2H);
      pStates[1] =
          pStates[3] + fMultDiv2(xImag, nFilt0H) - fMultDiv2(yImag, nFilt2H);
      pStates[2] =
          pStates[4] + fMultDiv2(xReal, nFilt1L) - fMultDiv2(yReal, nFilt2L);
      pStates[3] =
          pStates[5] + fMultDiv2(xImag, nFilt1L) - fMultDiv2(yImag, nFilt2L);
      pStates[4] =
          pStates[6] + fMultDiv2(xReal, nFilt1H) - fMultDiv2(yReal, nFilt1H);
      pStates[5] =
          pStates[7] + fMultDiv2(xImag, nFilt1H) - fMultDiv2(yImag, nFilt1H);
      pStates[6] =
          pStates[8] + fMultDiv2(xReal, nFilt2L) - fMultDiv2(yReal, nFilt1L);
      pStates[7] =
          pStates[9] + fMultDiv2(xImag, nFilt2L) - fMultDiv2(yImag, nFilt1L);
      pStates[8] =
          pStates[10] + fMultDiv2(xReal, nFilt2H) - fMultDiv2(yReal, nFilt0H);
      pStates[9] =
          pStates[11] + fMultDiv2(xImag, nFilt2H) - fMultDiv2(yImag, nFilt0H);
      pStates[10] = (xReal >> FILTER_SF) - fMultDiv2(yReal, nFilt0L);
      pStates[11] = (xImag >> FILTER_SF) - fMultDiv2(yImag, nFilt0L);
      pStates += offsetStates;
    }
  } else {
    FIXP_DECORR nFilt0L, nFilt0H;
    for (i = start; i < stop; i++) {
      xReal = *pDelayBuffer;
      *pDelayBuffer = dataRealIn[i];
      pDelayBuffer++;

      xImag = *pDelayBuffer;
      *pDelayBuffer = dataImagIn[i];
      pDelayBuffer += offsetDelayBuffer;

      nFilt0L = pFilter[0];
      yReal = (pStates[0] + fMultDiv2(xReal, nFilt0L)) << 2;
      yImag = (pStates[1] + fMultDiv2(xImag, nFilt0L)) << 2;
      dataRealOut[i] = yReal;
      dataImagOut[i] = yImag;

      for (j = 1; j < reverbFilterOrder; j++) {
        nFilt0L = pFilter[j];
        nFilt0H = pFilter[reverbFilterOrder - j];
        pStates[2 * j - 2] = pStates[2 * j] + fMultDiv2(xReal, nFilt0L) -
                             fMultDiv2(yReal, nFilt0H);
        pStates[2 * j - 1] = pStates[2 * j + 1] + fMultDiv2(xImag, nFilt0L) -
                             fMultDiv2(yImag, nFilt0H);
      }
      nFilt0L = pFilter[j];
      nFilt0H = pFilter[reverbFilterOrder - j];
      pStates[2 * j - 2] =
          fMultDiv2(xReal, nFilt0L) - fMultDiv2(yReal, nFilt0H);
      pStates[2 * j - 1] =
          fMultDiv2(xImag, nFilt0L) - fMultDiv2(yImag, nFilt0H);

      pStates += offsetStates;
    }
  }

  return (INT)0;
}
#endif /* #ifndef FUNCTION_DecorrFilterApplyREAL */

#ifndef FUNCTION_DecorrFilterApplyCPLX_PS
LNK_SECTION_CODE_L1
static INT DecorrFilterApplyCPLX_PS(
    DECORR_FILTER_INSTANCE const filter[], FIXP_DBL *dataRealIn,
    FIXP_DBL *dataImagIn, FIXP_DBL *dataRealOut, FIXP_DBL *dataImagOut,
    INT start, INT stop, INT reverbFilterOrder, INT reverbBandNoSampleDelay,
    INT reverbBandDelayBufferIndex, UCHAR *stateBufferOffset) {
  /* r = real, j = imaginary */
  FIXP_DBL r_data_a, j_data_a, r_data_b, j_data_b, r_stage_mult, j_stage_mult;
  FIXP_STP rj_coeff;

  /* get pointer to current position in input delay buffer of filter with
   * starting-index */
  FIXP_DBL *pDelayBuffer =
      &filter[start].DelayBufferCplx[reverbBandDelayBufferIndex]; /* increases
                                                                     by 2 every
                                                                     other call
                                                                     of this
                                                                     function */
  /* determine the increment for this pointer to get to the correct position in
   * the delay buffer of the next filter */
  INT offsetDelayBuffer = (2 * reverbBandNoSampleDelay) - 1;

  /* pointer to current position in state buffer */
  FIXP_DBL *pStates = filter[start].stateCplx;
  INT pStatesIncrement = 2 * reverbFilterOrder;

  /* stateBufferOffset-pointers */
  FIXP_DBL *pStateBufferOffset0 = pStates + stateBufferOffset[0];
  FIXP_DBL *pStateBufferOffset1 = pStates + stateBufferOffset[1];
  FIXP_DBL *pStateBufferOffset2 = pStates + stateBufferOffset[2];

  /* traverse all hybrid-bands inbetween start- and stop-index */
  for (int i = start; i < stop; i++) {
    /* 1. input delay (real/imaginary values interleaved) */

    /* load delayed real input value */
    r_data_a = *pDelayBuffer;
    /* store incoming real data value to delay buffer and increment pointer */
    *pDelayBuffer++ = dataRealIn[i];

    /* load delayed imaginary input value */
    j_data_a = *pDelayBuffer;
    /* store incoming imaginary data value to delay buffer */
    *pDelayBuffer = dataImagIn[i];
    /* increase delay buffer by offset */
    pDelayBuffer += offsetDelayBuffer;

    /* 2. Phi(k)-stage */

    /* create pointer to coefficient table (real and imaginary coefficients
     * interleaved) */
    const FIXP_STP *pCoeffs = filter[i].coeffsPacked;

    /* the first two entries of the coefficient table are the
     * Phi(k)-multiplicants */
    rj_coeff = *pCoeffs++;
    /* multiply value from input delay buffer by looked-up values */
    cplxMultDiv2(&r_data_b, &j_data_b, r_data_a, j_data_a, rj_coeff);

    /* 3. process all three filter stages */

    /* stage 0 */

    /* get coefficients from lookup table */
    rj_coeff = *pCoeffs++;

    /* multiply output of last stage by coefficient */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, r_data_b, j_data_b, rj_coeff);
    r_stage_mult <<= 1;
    j_stage_mult <<= 1;

    /* read and add value from state buffer (this is the input for the next
     * stage) */
    r_data_a = r_stage_mult + pStateBufferOffset0[0];
    j_data_a = j_stage_mult + pStateBufferOffset0[1];

    /* negate r_data_a to perform multiplication with complex conjugate of
     * rj_coeff */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, -r_data_a, j_data_a, rj_coeff);

    /* add stage input to shifted result */
    r_stage_mult = r_data_b + (r_stage_mult << 1);
    j_stage_mult = j_data_b - (j_stage_mult << 1);

    /* store result to state buffer */
    pStateBufferOffset0[0] = r_stage_mult;
    pStateBufferOffset0[1] = j_stage_mult;
    pStateBufferOffset0 += pStatesIncrement;

    /* stage 1 */

    /* get coefficients from lookup table */
    rj_coeff = *pCoeffs++;

    /* multiply output of last stage by coefficient */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, r_data_a, j_data_a, rj_coeff);
    r_stage_mult <<= 1;
    j_stage_mult <<= 1;

    /* read and add value from state buffer (this is the input for the next
     * stage) */
    r_data_b = r_stage_mult + pStateBufferOffset1[0];
    j_data_b = j_stage_mult + pStateBufferOffset1[1];

    /* negate r_data_b to perform multiplication with complex conjugate of
     * rj_coeff */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, -r_data_b, j_data_b, rj_coeff);

    /* add stage input to shifted result */
    r_stage_mult = r_data_a + (r_stage_mult << 1);
    j_stage_mult = j_data_a - (j_stage_mult << 1);

    /* store result to state buffer */
    pStateBufferOffset1[0] = r_stage_mult;
    pStateBufferOffset1[1] = j_stage_mult;
    pStateBufferOffset1 += pStatesIncrement;

    /* stage 2 */

    /* get coefficients from lookup table */
    rj_coeff = *pCoeffs++;

    /* multiply output of last stage by coefficient */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, r_data_b, j_data_b, rj_coeff);
    r_stage_mult <<= 1;
    j_stage_mult <<= 1;

    /* read and add value from state buffer (this is the input for the next
     * stage) */
    r_data_a = r_stage_mult + pStateBufferOffset2[0];
    j_data_a = j_stage_mult + pStateBufferOffset2[1];

    /* negate r_data_a to perform multiplication with complex conjugate of
     * rj_coeff */
    cplxMultDiv2(&r_stage_mult, &j_stage_mult, -r_data_a, j_data_a, rj_coeff);

    /* add stage input to shifted result */
    r_stage_mult = r_data_b + (r_stage_mult << 1);
    j_stage_mult = j_data_b - (j_stage_mult << 1);

    /* store result to state buffer */
    pStateBufferOffset2[0] = r_stage_mult;
    pStateBufferOffset2[1] = j_stage_mult;
    pStateBufferOffset2 += pStatesIncrement;

    /* write filter output */
    dataRealOut[i] = r_data_a << 1;
    dataImagOut[i] = j_data_a << 1;

  } /* end of band/filter loop (outer loop) */

  /* update stateBufferOffset with respect to ring buffer boundaries */
  if (stateBufferOffset[0] == 4)
    stateBufferOffset[0] = 0;
  else
    stateBufferOffset[0] += 2;

  if (stateBufferOffset[1] == 12)
    stateBufferOffset[1] = 6;
  else
    stateBufferOffset[1] += 2;

  if (stateBufferOffset[2] == 22)
    stateBufferOffset[2] = 14;
  else
    stateBufferOffset[2] += 2;

  return (INT)0;
}

#endif /* FUNCTION_DecorrFilterApplyCPLX_PS */

/*******************************************************************************
*******************************************************************************/
static INT DuckerInit(DUCKER_INSTANCE *const self, int const hybridBands,
                      int partiallyComplex, const FDK_DUCKER_TYPE duckerType,
                      const int nParamBands, int initStatesFlag) {
  INT errorCode = 0;

  if (self) {
    switch (nParamBands) {
      case (20):
        FDK_ASSERT(hybridBands == 71);
        self->mapHybBands2ProcBands = kernels_20_to_71_PS;
        self->mapProcBands2HybBands = kernels_20_to_71_offset_PS;
        self->parameterBands = (20);
        break;
      case (28):

        self->mapHybBands2ProcBands = kernels_28_to_71;
        self->mapProcBands2HybBands = kernels_28_to_71_offset;
        self->parameterBands = (28);
        break;
      case (23):
        FDK_ASSERT(hybridBands == 64 || hybridBands == 32);
        self->mapHybBands2ProcBands = kernels_23_to_64;
        self->mapProcBands2HybBands = kernels_23_to_64_offset;
        self->parameterBands = (23);
        break;
      default:
        return 1;
    }
    self->qs_next = &self->mapProcBands2HybBands[1];

    self->maxValDirectData = FL2FXCONST_DBL(-1.0f);
    self->maxValReverbData = FL2FXCONST_DBL(-1.0f);
    self->scaleDirectNrg = 2 * DUCKER_MAX_NRG_SCALE;
    self->scaleReverbNrg = 2 * DUCKER_MAX_NRG_SCALE;
    self->scaleSmoothDirRevNrg = 2 * DUCKER_MAX_NRG_SCALE;
    self->headroomSmoothDirRevNrg = 2 * DUCKER_MAX_NRG_SCALE;
    self->hybridBands = hybridBands;
    self->partiallyComplex = partiallyComplex;

    if (initStatesFlag && (duckerType == DUCKER_PS)) {
      int pb;
      for (pb = 0; pb < self->parameterBands; pb++) {
        self->SmoothDirRevNrg[pb] = (FIXP_MPS)0;
      }
    }
  } else
    errorCode = 1;

  return errorCode;
}

  /*******************************************************************************
  *******************************************************************************/

#ifndef FUNCTION_DuckerCalcEnergy
static INT DuckerCalcEnergy(DUCKER_INSTANCE *const self,
                            FIXP_DBL const inputReal[(71)],
                            FIXP_DBL const inputImag[(71)],
                            FIXP_DBL energy[(28)], FIXP_DBL inputMaxVal,
                            SCHAR *nrgScale, int mode, /* 1:(ps) 0:(else) */
                            int startHybBand) {
  INT err = 0;
  int qs, maxHybBand;
  int maxHybridBand = self->hybridBands - 1;

  maxHybBand = maxHybridBand;

  FDKmemclear(energy, (28) * sizeof(FIXP_DBL));

  if (mode == 1) {
    int pb;
    int clz;
    FIXP_DBL maxVal = FL2FXCONST_DBL(-1.0f);

    if (maxVal == FL2FXCONST_DBL(-1.0f)) {
      clz = fMin(getScalefactor(&inputReal[startHybBand],
                                fMax(0, maxHybridBand - startHybBand + 1)),
                 getScalefactor(&inputImag[startHybBand],
                                fMax(0, maxHybBand - startHybBand + 1)));
    } else {
      clz = CntLeadingZeros(maxVal) - 1;
    }

    clz = fMin(fMax(0, clz - DUCKER_HEADROOM_BITS), DUCKER_MAX_NRG_SCALE);
    *nrgScale = (SCHAR)clz << 1;

    /* Initialize pb since it would stay uninitialized for the case startHybBand
     * > maxHybBand. */
    pb = SpatialDecGetProcessingBand(maxHybBand, self->mapHybBands2ProcBands);
    for (qs = startHybBand; qs <= maxHybBand; qs++) {
      pb = SpatialDecGetProcessingBand(qs, self->mapHybBands2ProcBands);
      energy[pb] = SATURATE_LEFT_SHIFT(
          (energy[pb] >> 1) + (fPow2Div2(inputReal[qs] << clz) >> 1) +
              (fPow2Div2(inputImag[qs] << clz) >> 1),
          1, DFRACT_BITS);
    }
    pb++;

    for (; pb <= SpatialDecGetProcessingBand(maxHybridBand,
                                             self->mapHybBands2ProcBands);
         pb++) {
      FDK_ASSERT(pb != SpatialDecGetProcessingBand(
                           qs - 1, self->mapHybBands2ProcBands));
      int qs_next;
      FIXP_DBL nrg = 0;
      qs_next = (int)self->qs_next[pb];
      for (; qs < qs_next; qs++) {
        nrg = fAddSaturate(nrg, fPow2Div2(inputReal[qs] << clz));
      }
      energy[pb] = nrg;
    }
  } else {
    int clz;
    FIXP_DBL maxVal = FL2FXCONST_DBL(-1.0f);

    maxVal = inputMaxVal;

    if (maxVal == FL2FXCONST_DBL(-1.0f)) {
      clz = fMin(getScalefactor(&inputReal[startHybBand],
                                fMax(0, maxHybridBand - startHybBand + 1)),
                 getScalefactor(&inputImag[startHybBand],
                                fMax(0, maxHybBand - startHybBand + 1)));
    } else {
      clz = CntLeadingZeros(maxVal) - 1;
    }

    clz = fMin(fMax(0, clz - DUCKER_HEADROOM_BITS), DUCKER_MAX_NRG_SCALE);
    *nrgScale = (SCHAR)clz << 1;

    for (qs = startHybBand; qs <= maxHybBand; qs++) {
      int pb = SpatialDecGetProcessingBand(qs, self->mapHybBands2ProcBands);
      energy[pb] = SATURATE_LEFT_SHIFT(
          (energy[pb] >> 1) + (fPow2Div2(inputReal[qs] << clz) >> 1) +
              (fPow2Div2(inputImag[qs] << clz) >> 1),
          1, DFRACT_BITS);
    }

    for (; qs <= maxHybridBand; qs++) {
      int pb = SpatialDecGetProcessingBand(qs, self->mapHybBands2ProcBands);
      energy[pb] = fAddSaturate(energy[pb], fPow2Div2(inputReal[qs] << clz));
    }
  }

  {
    /* Catch overflows which have been observed in erred bitstreams to avoid
     * assertion failures later. */
    int pb;
    for (pb = 0; pb < (28); pb++) {
      energy[pb] = (FIXP_DBL)((LONG)energy[pb] & (LONG)MAXVAL_DBL);
    }
  }
  return err;
}
#endif /* #ifndef FUNCTION_DuckerCalcEnergy */

LNK_SECTION_CODE_L1
static INT DuckerApply(DUCKER_INSTANCE *const self,
                       FIXP_DBL const directNrg[(28)],
                       FIXP_DBL outputReal[(71)], FIXP_DBL outputImag[(71)],
                       int startHybBand) {
  INT err = 0;
  int qs = startHybBand;
  int qs_next = 0;
  int pb = 0;
  int startParamBand = 0;
  int hybBands;
  int hybridBands = self->hybridBands;

  C_ALLOC_SCRATCH_START(reverbNrg, FIXP_DBL, (28));

  FIXP_DBL *smoothDirRevNrg = &self->SmoothDirRevNrg[0];
  FIXP_DUCK_GAIN duckGain = 0;

  int doScaleNrg = 0;
  int scaleDirectNrg = 0;
  int scaleReverbNrg = 0;
  int scaleSmoothDirRevNrg = 0;
  FIXP_DBL maxDirRevNrg = FL2FXCONST_DBL(0.0);

  hybBands = hybridBands;

  startParamBand =
      SpatialDecGetProcessingBand(startHybBand, self->mapHybBands2ProcBands);

  DuckerCalcEnergy(self, outputReal, outputImag, reverbNrg,
                   self->maxValReverbData, &(self->scaleReverbNrg), 0,
                   startHybBand);

  if ((self->scaleDirectNrg != self->scaleReverbNrg) ||
      (self->scaleDirectNrg != self->scaleSmoothDirRevNrg) ||
      (self->headroomSmoothDirRevNrg == 0)) {
    int scale;

    scale = fixMin(self->scaleDirectNrg, self->scaleSmoothDirRevNrg +
                                             self->headroomSmoothDirRevNrg - 1);
    scale = fixMin(scale, self->scaleReverbNrg);

    scaleDirectNrg = fMax(fMin(self->scaleDirectNrg - scale, (DFRACT_BITS - 1)),
                          -(DFRACT_BITS - 1));
    scaleReverbNrg = fMax(fMin(self->scaleReverbNrg - scale, (DFRACT_BITS - 1)),
                          -(DFRACT_BITS - 1));
    scaleSmoothDirRevNrg =
        fMax(fMin(self->scaleSmoothDirRevNrg - scale, (DFRACT_BITS - 1)),
             -(DFRACT_BITS - 1));

    self->scaleSmoothDirRevNrg = (SCHAR)scale;

    doScaleNrg = 1;
  }
  for (pb = startParamBand; pb < self->parameterBands; pb++) {
    FIXP_DBL tmp1;
    FIXP_DBL tmp2;
    INT s;

    /* smoothDirRevNrg[2*pb  ] = fMult(smoothDirRevNrg[2*pb  ],DUCK_ALPHA_FDK) +
       fMultDiv2(directNrg[pb],DUCK_ONE_MINUS_ALPHA_X4_FDK);
       smoothDirRevNrg[2*pb+1] = fMult(smoothDirRevNrg[2*pb+1],DUCK_ALPHA_FDK) +
       fMultDiv2(reverbNrg[pb],DUCK_ONE_MINUS_ALPHA_X4_FDK); tmp1 =
       fMult(smoothDirRevNrg[2*pb],DUCK_GAMMA_FDK); tmp2 =
       smoothDirRevNrg[2*pb+1] >> 1;
    */
    tmp1 = smoothDirRevNrg[2 * pb + 0];
    tmp2 = smoothDirRevNrg[2 * pb + 1];
    tmp1 = fMult(tmp1, DUCK_ALPHA_FDK);
    tmp2 = fMult(tmp2, DUCK_ALPHA_FDK);

    if (doScaleNrg) {
      int scaleSmoothDirRevNrg_asExponent = -scaleSmoothDirRevNrg;

      tmp1 = scaleValue(tmp1, scaleSmoothDirRevNrg_asExponent);
      tmp2 = scaleValue(tmp2, scaleSmoothDirRevNrg_asExponent);
      tmp1 = fMultAddDiv2(tmp1, scaleValue(directNrg[pb], -scaleDirectNrg),
                          DUCK_ONE_MINUS_ALPHA_X4_FDK);
      tmp2 = fMultAddDiv2(tmp2, scaleValue(reverbNrg[pb], -scaleReverbNrg),
                          DUCK_ONE_MINUS_ALPHA_X4_FDK);
    } else {
      tmp1 = fMultAddDiv2(tmp1, directNrg[pb], DUCK_ONE_MINUS_ALPHA_X4_FDK);
      tmp2 = fMultAddDiv2(tmp2, reverbNrg[pb], DUCK_ONE_MINUS_ALPHA_X4_FDK);
    }

    smoothDirRevNrg[2 * pb] = tmp1;
    smoothDirRevNrg[2 * pb + 1] = tmp2;

    maxDirRevNrg |= fAbs(tmp1);
    maxDirRevNrg |= fAbs(tmp2);

    tmp1 = fMult(tmp1, DUCK_GAMMA_FDK);
    tmp2 = tmp2 >> 1;

    qs_next = fMin((int)self->qs_next[pb], self->hybridBands);

    if (tmp2 > tmp1) { /* true for about 20% */
      /* gain smaller than 1.0 */
      tmp1 = sqrtFixp(tmp1);
      tmp2 = invSqrtNorm2(tmp2, &s);
      duckGain = FX_DBL2FX_DUCK_GAIN(fMultDiv2(tmp1, tmp2) << s);
    } else { /* true for about 80 % */
      tmp2 = smoothDirRevNrg[2 * pb] >> 1;
      tmp1 = fMult(smoothDirRevNrg[2 * pb + 1], DUCK_GAMMA_FDK);
      if (tmp2 > tmp1) { /* true for about 20% */
        if (tmp1 <= (tmp2 >> 2)) {
          /* limit gain to 2.0 */
          if (qs < hybBands) {
            for (; qs < qs_next; qs++) {
              outputReal[qs] = outputReal[qs] << 1;
              outputImag[qs] = outputImag[qs] << 1;
            }
          } else {
            for (; qs < qs_next; qs++) {
              outputReal[qs] = outputReal[qs] << 1;
            }
          }
          /* skip general gain*output section */
          continue;
        } else {
          /* gain from 1.0 to 2.0 */
          tmp2 = sqrtFixp(tmp2 >> 2);
          tmp1 = invSqrtNorm2(tmp1, &s);
          duckGain = FX_DBL2FX_DUCK_GAIN(fMult(tmp1, tmp2) << s);
        }
      } else { /* true for about 60% */
        /* gain = 1.0; output does not change; update qs index */
        qs = qs_next;
        continue;
      }
    }

    /* general gain*output section */
    if (qs < hybBands) {           /* true for about 39% */
      for (; qs < qs_next; qs++) { /* runs about 2 times */
        outputReal[qs] = fMultDiv2(outputReal[qs], duckGain) << 2;
        outputImag[qs] = fMultDiv2(outputImag[qs], duckGain) << 2;
      }
    } else {
      for (; qs < qs_next; qs++) {
        outputReal[qs] = fMultDiv2(outputReal[qs], duckGain) << 2;
      }
    }
  } /* pb */

  self->headroomSmoothDirRevNrg =
      (SCHAR)fixMax(0, CntLeadingZeros(maxDirRevNrg) - 1);

  C_ALLOC_SCRATCH_END(reverbNrg, FIXP_DBL, (28));

  return err;
}

LNK_SECTION_CODE_L1
static INT DuckerApplyPS(DUCKER_INSTANCE *const self,
                         FIXP_DBL const directNrg[(28)],
                         FIXP_DBL outputReal[(71)], FIXP_DBL outputImag[(71)],
                         int startHybBand) {
  int qs = startHybBand;
  int pb = 0;
  int startParamBand =
      SpatialDecGetProcessingBand(startHybBand, self->mapHybBands2ProcBands);
  int hybBands;

  int doScaleNrg = 0;
  int scaleDirectNrg = 0;
  int scaleSmoothDirRevNrg = 0;
  FIXP_DBL maxDirRevNrg = FL2FXCONST_DBL(0.0);

  if ((self->scaleDirectNrg != self->scaleSmoothDirRevNrg) ||
      (self->headroomSmoothDirRevNrg == 0)) {
    int scale;

    scale = fixMin(self->scaleDirectNrg, self->scaleSmoothDirRevNrg +
                                             self->headroomSmoothDirRevNrg - 2);

    scaleDirectNrg = fMax(fMin(self->scaleDirectNrg - scale, (DFRACT_BITS - 1)),
                          -(DFRACT_BITS - 1));
    scaleSmoothDirRevNrg =
        fMax(fMin(self->scaleSmoothDirRevNrg - scale, (DFRACT_BITS - 1)),
             -(DFRACT_BITS - 1));

    self->scaleSmoothDirRevNrg = (SCHAR)scale;

    doScaleNrg = 1;
  }

  hybBands = self->hybridBands;

  FDK_ASSERT((self->parameterBands == (28)) || (self->parameterBands == (20)));
  for (pb = startParamBand; pb < self->parameterBands; pb++) {
    FIXP_DBL directNrg2 = directNrg[pb];

    if (doScaleNrg) {
      directNrg2 = scaleValue(directNrg2, -scaleDirectNrg);
      self->peakDiff[pb] =
          scaleValue(self->peakDiff[pb], -scaleSmoothDirRevNrg);
      self->peakDecay[pb] =
          scaleValue(self->peakDecay[pb], -scaleSmoothDirRevNrg);
      self->SmoothDirRevNrg[pb] =
          scaleValue(self->SmoothDirRevNrg[pb], -scaleSmoothDirRevNrg);
    }
    self->peakDecay[pb] = fixMax(
        directNrg2, fMult(self->peakDecay[pb], PS_DUCK_PEAK_DECAY_FACTOR_FDK));
    self->peakDiff[pb] =
        self->peakDiff[pb] +
        fMult(PS_DUCK_FILTER_COEFF_FDK,
              (self->peakDecay[pb] - directNrg2 - self->peakDiff[pb]));
    self->SmoothDirRevNrg[pb] =
        fixMax(self->SmoothDirRevNrg[pb] +
                   fMult(PS_DUCK_FILTER_COEFF_FDK,
                         (directNrg2 - self->SmoothDirRevNrg[pb])),
               FL2FXCONST_DBL(0));

    maxDirRevNrg |= fAbs(self->peakDiff[pb]);
    maxDirRevNrg |= fAbs(self->SmoothDirRevNrg[pb]);

    if ((self->peakDiff[pb] == FL2FXCONST_DBL(0)) &&
        (self->SmoothDirRevNrg[pb] == FL2FXCONST_DBL(0))) {
      int qs_next;

      qs = fMax(qs, SpatialDecGetQmfBand(pb, self->mapProcBands2HybBands));
      qs_next = fMin((int)self->qs_next[pb], self->hybridBands);

      FIXP_DBL *pOutputReal = &outputReal[qs];
      FIXP_DBL *pOutputImag = &outputImag[qs];

      if (qs < hybBands) {
        for (; qs < qs_next; qs++) {
          *pOutputReal++ = FL2FXCONST_DBL(0);
          *pOutputImag++ = FL2FXCONST_DBL(0);
        }
      } else {
        for (; qs < qs_next; qs++) {
          *pOutputReal++ = FL2FXCONST_DBL(0);
        }
      }
    } else if (self->peakDiff[pb] != FL2FXCONST_DBL(0)) {
      FIXP_DBL multiplication =
          fMult(FL2FXCONST_DUCK(0.75f), self->peakDiff[pb]);
      if (multiplication > (self->SmoothDirRevNrg[pb] >> 1)) {
        FIXP_DBL num, denom, duckGain;
        int scale, qs_next;

        /* implement x/y as (sqrt(x)*invSqrt(y))^2 */
        num = sqrtFixp(self->SmoothDirRevNrg[pb] >> 1);
        denom = self->peakDiff[pb] +
                FL2FXCONST_DBL(ABS_THR / (32768.0f * 32768.0f * 128.0f * 1.5f));
        denom = invSqrtNorm2(denom, &scale);

        /* duck output whether duckGain != 1.f */
        qs = fMax(qs, SpatialDecGetQmfBand(pb, self->mapProcBands2HybBands));
        qs_next = fMin((int)self->qs_next[pb], self->hybridBands);

        duckGain = fMult(num, denom);
        duckGain = fPow2Div2(duckGain << scale);
        duckGain = fMultDiv2(FL2FXCONST_DUCK(2.f / 3.f), duckGain) << 3;

        FIXP_DBL *pOutputReal = &outputReal[qs];
        FIXP_DBL *pOutputImag = &outputImag[qs];

        if (qs < hybBands) {
          for (; qs < qs_next; qs++) {
            *pOutputReal = fMult(*pOutputReal, duckGain);
            pOutputReal++; /* don't move in front of "=" above, because then the
                              fract class treats it differently and provides
                              wrong argument to fMult() (seen on win32/msvc8) */
            *pOutputImag = fMult(*pOutputImag, duckGain);
            pOutputImag++;
          }
        } else {
          for (; qs < qs_next; qs++) {
            *pOutputReal = fMult(*pOutputReal, duckGain);
            pOutputReal++;
          }
        }
      }
    }
  } /* pb */

  self->headroomSmoothDirRevNrg =
      (SCHAR)fixMax(0, CntLeadingZeros(maxDirRevNrg) - 1);

  return 0;
}

INT FDKdecorrelateOpen(HANDLE_DECORR_DEC hDecorrDec, FIXP_DBL *bufferCplx,
                       const INT bufLen) {
  HANDLE_DECORR_DEC self = hDecorrDec;

  if (bufLen < (2 * ((825) + (373)))) return 1;

  /* assign all memory to stateBufferCplx. It is reassigned during
   * FDKdecorrelateInit() */
  self->stateBufferCplx = bufferCplx;
  self->L_stateBufferCplx = 0;

  self->delayBufferCplx = NULL;
  self->L_delayBufferCplx = 0;

  return 0;
}

static int distributeBuffer(HANDLE_DECORR_DEC self, const int L_stateBuf,
                            const int L_delayBuf) {
  /* factor 2 because of complex values */
  if ((2 * ((825) + (373))) < 2 * (L_stateBuf + L_delayBuf)) {
    return 1;
  }

  self->L_stateBufferCplx = 2 * L_stateBuf;
  self->delayBufferCplx = self->stateBufferCplx + 2 * L_stateBuf;
  self->L_delayBufferCplx = 2 * L_delayBuf;

  return 0;
}
INT FDKdecorrelateInit(HANDLE_DECORR_DEC hDecorrDec, const INT nrHybBands,
                       const FDK_DECORR_TYPE decorrType,
                       const FDK_DUCKER_TYPE duckerType, const INT decorrConfig,
                       const INT seed, const INT partiallyComplex,
                       const INT useFractDelay, const INT isLegacyPS,
                       const INT initStatesFlag) {
  INT errorCode = 0;
  int i, rb, i_start;
  int nParamBands = 28;

  INT offsetStateBuffer = 0;
  INT offsetDelayBuffer = 0;

  const UCHAR *REV_bandOffset;

  const SCHAR *REV_filterOrder;

  hDecorrDec->partiallyComplex = partiallyComplex;
  hDecorrDec->numbins = nrHybBands;

  switch (decorrType) {
    case DECORR_PS:
      /* ignore decorrConfig, seed */
      if (partiallyComplex) {
        hDecorrDec->REV_bandOffset = REV_bandOffset_PS_LP;
        hDecorrDec->REV_delay = REV_delay_PS_LP;
        errorCode = distributeBuffer(hDecorrDec, (168), (533));
      } else {
        hDecorrDec->REV_bandOffset = REV_bandOffset_PS_HQ;
        hDecorrDec->REV_delay = REV_delay_PS_HQ;
        errorCode = distributeBuffer(hDecorrDec, (360), (257));
      }
      hDecorrDec->REV_filterOrder = REV_filterOrder_PS;
      hDecorrDec->REV_filtType = REV_filtType_PS;

      /* Initialize ring buffer offsets for PS specific filter implementation.
       */
      for (i = 0; i < (3); i++)
        hDecorrDec->stateBufferOffset[i] = stateBufferOffsetInit[i];

      break;
    case DECORR_USAC:
      if (partiallyComplex) return 1;
      if (seed != 0) return 1;
      hDecorrDec->REV_bandOffset =
          REV_bandOffset_MPS_HQ[decorrConfig]; /* reverb band layout is
                                                  inherited from MPS standard */
      hDecorrDec->REV_filterOrder = REV_filterOrder_USAC;
      hDecorrDec->REV_delay = REV_delay_USAC;
      if (useFractDelay) {
        return 1; /* not yet supported */
      } else {
        hDecorrDec->REV_filtType = REV_filtType_MPS; /* the filter types are
                                                        inherited from MPS
                                                        standard */
      }
      /* bsDecorrConfig == 1 is worst case */
      errorCode = distributeBuffer(hDecorrDec, (509), (643));
      break;
    case DECORR_LD:
      if (partiallyComplex) return 1;
      if (useFractDelay) return 1;
      if (decorrConfig > 2) return 1;
      if (seed > (MAX_DECORR_SEED_LD - 1)) return 1;
      if (!(nrHybBands == 64 || nrHybBands == 32))
        return 1; /* actually just qmf bands and no hybrid bands */
      hDecorrDec->REV_bandOffset = REV_bandOffset_LD[decorrConfig];
      hDecorrDec->REV_filterOrder = REV_filterOrder_MPS; /* the filter orders
                                                            are inherited from
                                                            MPS standard */
      hDecorrDec->REV_delay =
          REV_delay_MPS; /* the delays in each reverb band are inherited from
                            MPS standard */
      hDecorrDec->REV_filtType = REV_filtType_LD;
      errorCode = distributeBuffer(hDecorrDec, (825), (373));
      break;
    default:
      return 1;
  }

  if (errorCode) {
    return errorCode;
  }

  if (initStatesFlag) {
    FDKmemclear(
        hDecorrDec->stateBufferCplx,
        hDecorrDec->L_stateBufferCplx * sizeof(*hDecorrDec->stateBufferCplx));
    FDKmemclear(
        hDecorrDec->delayBufferCplx,
        hDecorrDec->L_delayBufferCplx * sizeof(*hDecorrDec->delayBufferCplx));
    FDKmemclear(hDecorrDec->reverbBandDelayBufferIndex,
                sizeof(hDecorrDec->reverbBandDelayBufferIndex));
  }

  REV_bandOffset = hDecorrDec->REV_bandOffset;

  REV_filterOrder = hDecorrDec->REV_filterOrder;

  i_start = 0;
  for (rb = 0; rb < (4); rb++) {
    int i_stop;

    i_stop = REV_bandOffset[rb];

    if (i_stop <= i_start) {
      continue;
    }

    for (i = i_start; i < i_stop; i++) {
      switch (decorrType) {
        case DECORR_PS:
          errorCode = DecorrFilterInitPS(
              &hDecorrDec->Filter[i], hDecorrDec->stateBufferCplx,
              hDecorrDec->delayBufferCplx, &offsetStateBuffer,
              &offsetDelayBuffer, i, rb, hDecorrDec->REV_delay[rb]);
          break;
        default:
          errorCode = DecorrFilterInit(
              &hDecorrDec->Filter[i], hDecorrDec->stateBufferCplx,
              hDecorrDec->delayBufferCplx, &offsetStateBuffer,
              &offsetDelayBuffer, seed, rb, useFractDelay,
              hDecorrDec->REV_delay[rb], REV_filterOrder[rb], decorrType);
          break;
      }
    }

    i_start = i_stop;
  } /* loop over reverbBands */

  if (!(offsetStateBuffer <= hDecorrDec->L_stateBufferCplx) ||
      !(offsetDelayBuffer <= hDecorrDec->L_delayBufferCplx)) {
    return errorCode = 1;
  }

  if (duckerType == DUCKER_AUTOMATIC) {
    /* Choose correct ducker type according to standards: */
    switch (decorrType) {
      case DECORR_PS:
        hDecorrDec->ducker.duckerType = DUCKER_PS;
        if (isLegacyPS) {
          nParamBands = (20);
        } else {
          nParamBands = (28);
        }
        break;
      case DECORR_USAC:
        hDecorrDec->ducker.duckerType = DUCKER_MPS;
        nParamBands = (28);
        break;
      case DECORR_LD:
        hDecorrDec->ducker.duckerType = DUCKER_MPS;
        nParamBands = (23);
        break;
      default:
        return 1;
    }
  }

  errorCode = DuckerInit(
      &hDecorrDec->ducker, hDecorrDec->numbins, hDecorrDec->partiallyComplex,
      hDecorrDec->ducker.duckerType, nParamBands, initStatesFlag);

  return errorCode;
}

INT FDKdecorrelateClose(HANDLE_DECORR_DEC hDecorrDec) {
  INT err = 0;

  if (hDecorrDec == NULL) {
    return 1;
  }

  hDecorrDec->stateBufferCplx = NULL;
  hDecorrDec->L_stateBufferCplx = 0;
  hDecorrDec->delayBufferCplx = NULL;
  hDecorrDec->L_delayBufferCplx = 0;

  return err;
}

LNK_SECTION_CODE_L1
INT FDKdecorrelateApply(HANDLE_DECORR_DEC hDecorrDec, FIXP_DBL *dataRealIn,
                        FIXP_DBL *dataImagIn, FIXP_DBL *dataRealOut,
                        FIXP_DBL *dataImagOut, const INT startHybBand) {
  HANDLE_DECORR_DEC self = hDecorrDec;
  INT err = 0;
  INT rb, stop, start;

  if (self != NULL) {
    int nHybBands = 0;
    /* copy new samples */
    nHybBands = self->numbins;

    FIXP_DBL directNrg[(28)];

    DuckerCalcEnergy(
        &self->ducker, dataRealIn, dataImagIn, directNrg,
        self->ducker.maxValDirectData, &(self->ducker.scaleDirectNrg),
        (self->ducker.duckerType == DUCKER_PS) ? 1 : 0, startHybBand);

    /* complex-valued hybrid bands */
    for (stop = 0, rb = 0; rb < (4); rb++) {
      start = fMax(stop, startHybBand);
      stop = fMin(self->REV_bandOffset[rb], (UCHAR)nHybBands);

      if (start < stop) {
        switch (hDecorrDec->REV_filtType[rb]) {
          case DELAY:
            err = DecorrFilterApplyPASS(&self->Filter[0], dataRealIn,
                                        dataImagIn, dataRealOut, dataImagOut,
                                        start, stop, self->REV_delay[rb],
                                        self->reverbBandDelayBufferIndex[rb]);
            break;
          case INDEP_CPLX_PS:
            err = DecorrFilterApplyCPLX_PS(
                &self->Filter[0], dataRealIn, dataImagIn, dataRealOut,
                dataImagOut, start, stop, self->REV_filterOrder[rb],
                self->REV_delay[rb], self->reverbBandDelayBufferIndex[rb],
                self->stateBufferOffset);
            break;
          case COMMON_REAL:
            err = DecorrFilterApplyREAL(
                &self->Filter[0], dataRealIn, dataImagIn, dataRealOut,
                dataImagOut, start, stop, self->REV_filterOrder[rb],
                self->REV_delay[rb], self->reverbBandDelayBufferIndex[rb]);
            break;
          default:
            err = 1;
            break;
        }
        if (err != 0) {
          goto bail;
        }
      } /* if start < stop */
    }   /* loop over reverb bands */

    for (rb = 0; rb < (4); rb++) {
      self->reverbBandDelayBufferIndex[rb] += 2;
      if (self->reverbBandDelayBufferIndex[rb] >= 2 * self->REV_delay[rb])
        self->reverbBandDelayBufferIndex[rb] = 0;
    }

    switch (self->ducker.duckerType) {
      case DUCKER_PS:
        err = DuckerApplyPS(&self->ducker, directNrg, dataRealOut, dataImagOut,
                            startHybBand);
        if (err != 0) goto bail;
        break;
      default:
        err = DuckerApply(&self->ducker, directNrg, dataRealOut, dataImagOut,
                          startHybBand);
        if (err != 0) goto bail;
        break;
    }
  }

bail:
  return err;
}
