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

/**************************** SBR decoder library ******************************

   Author(s):   Oliver Moser, Manuel Jander, Matthias Hildenbrand

   Description: QMF frequency pre-whitening for SBR.
                In the documentation the terms "scale factor" and "exponent"
                mean the same. Variables containing such information have
                the suffix "_sf".

*******************************************************************************/

#include "HFgen_preFlat.h"

#define POLY_ORDER 3
#define MAXLOWBANDS 32
#define LOG10FAC 0.752574989159953f     /* == 10/log2(10) * 2^-2 */
#define LOG10FAC_INV 0.664385618977472f /* == log2(10)/20 * 2^2  */

#define FIXP_CHB FIXP_SGL /* STB sinus Tab used in transformation */
#define CHC(a) (FX_DBL2FXCONST_SGL(a))
#define FX_CHB2FX_DBL(a) FX_SGL2FX_DBL(a)

typedef struct backsubst_data {
  FIXP_CHB Lnorm1d[3]; /*!< Normalized L matrix */
  SCHAR Lnorm1d_sf[3];
  FIXP_CHB Lnormii
      [3]; /*!< The diagonal data points [i][i] of the normalized L matrix */
  SCHAR Lnormii_sf[3];
  FIXP_CHB Bmul0
      [4]; /*!< To normalize L*x=b, Bmul0 is what we need to multiply b with. */
  SCHAR Bmul0_sf[4];
  FIXP_CHB LnormInv1d[6]; /*!< Normalized inverted L matrix (L') */
  SCHAR LnormInv1d_sf[6];
  FIXP_CHB
  Bmul1[4]; /*!< To normalize L'*x=b, Bmul1 is what we need to multiply b
               with. */
  SCHAR Bmul1_sf[4];
} backsubst_data;

/* for each element n do, f(n) = trunc(log2(n))+1  */
const UCHAR getLog2[32] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                           5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

/** \def  BSD_IDX_OFFSET
 *
 *  bsd[] begins at index 0 with data for numBands=5. The correct bsd[] is
 *  indexed like bsd[numBands-BSD_IDX_OFFSET].
 */
#define BSD_IDX_OFFSET 5

#define N_NUMBANDS               \
  MAXLOWBANDS - BSD_IDX_OFFSET + \
      1 /*!< Number of backsubst_data elements in bsd */

const backsubst_data bsd[N_NUMBANDS] = {
    {
        /* numBands=5 */
        {CHC(0x66c85a52), CHC(0x4278e587), CHC(0x697dcaff)},
        {-1, 0, 0},
        {CHC(0x66a61789), CHC(0x5253b8e3), CHC(0x5addad81)},
        {3, 4, 1},
        {CHC(0x7525ee90), CHC(0x6e2a1210), CHC(0x6523bb40), CHC(0x59822ead)},
        {-6, -4, -2, 0},
        {CHC(0x609e4cad), CHC(0x59c7e312), CHC(0x681eecac), CHC(0x440ea893),
         CHC(0x4a214bb3), CHC(0x53c345a1)},
        {1, 0, -1, -1, -3, -5},
        {CHC(0x7525ee90), CHC(0x58587936), CHC(0x410d0b38), CHC(0x7f1519d6)},
        {-6, -1, 2, 0},
    },
    {
        /* numBands=6 */
        {CHC(0x68943285), CHC(0x4841d2c3), CHC(0x6a6214c7)},
        {-1, 0, 0},
        {CHC(0x63c5923e), CHC(0x4e906e18), CHC(0x6285af8a)},
        {3, 4, 1},
        {CHC(0x7263940b), CHC(0x424a69a5), CHC(0x4ae8383a), CHC(0x517b7730)},
        {-7, -4, -2, 0},
        {CHC(0x518aee5f), CHC(0x4823a096), CHC(0x43764a39), CHC(0x6e6faf23),
         CHC(0x61bba44f), CHC(0x59d8b132)},
        {1, 0, -1, -2, -4, -6},
        {CHC(0x7263940b), CHC(0x6757bff2), CHC(0x5bf40fe0), CHC(0x7d6f4292)},
        {-7, -2, 1, 0},
    },
    {
        /* numBands=7 */
        {CHC(0x699b4c3c), CHC(0x4b8b702f), CHC(0x6ae51a4f)},
        {-1, 0, 0},
        {CHC(0x623a7f49), CHC(0x4ccc91fc), CHC(0x68f048dd)},
        {3, 4, 1},
        {CHC(0x7e6ebe18), CHC(0x5701daf2), CHC(0x74a8198b), CHC(0x4b399aa1)},
        {-8, -5, -3, 0},
        {CHC(0x464a64a6), CHC(0x78e42633), CHC(0x5ee174ba), CHC(0x5d0008c8),
         CHC(0x455cff0f), CHC(0x6b9100e7)},
        {1, -1, -2, -2, -4, -7},
        {CHC(0x7e6ebe18), CHC(0x42c52efe), CHC(0x45fe401f), CHC(0x7b5808ef)},
        {-8, -2, 1, 0},
    },
    {
        /* numBands=8 */
        {CHC(0x6a3fd9b4), CHC(0x4d99823f), CHC(0x6b372a94)},
        {-1, 0, 0},
        {CHC(0x614c6ef7), CHC(0x4bd06699), CHC(0x6e59cfca)},
        {3, 4, 1},
        {CHC(0x4c389cc5), CHC(0x79686681), CHC(0x5e2544c2), CHC(0x46305b43)},
        {-8, -6, -3, 0},
        {CHC(0x7b4ca7c6), CHC(0x68270ac5), CHC(0x467c644c), CHC(0x505c1b0f),
         CHC(0x67a14778), CHC(0x45801767)},
        {0, -1, -2, -2, -5, -7},
        {CHC(0x4c389cc5), CHC(0x5c499ceb), CHC(0x6f863c9f), CHC(0x79059bfc)},
        {-8, -3, 0, 0},
    },
    {
        /* numBands=9 */
        {CHC(0x6aad9988), CHC(0x4ef8ac18), CHC(0x6b6df116)},
        {-1, 0, 0},
        {CHC(0x60b159b0), CHC(0x4b33f772), CHC(0x72f5573d)},
        {3, 4, 1},
        {CHC(0x6206cb18), CHC(0x58a7d8dc), CHC(0x4e0b2d0b), CHC(0x4207ad84)},
        {-9, -6, -3, 0},
        {CHC(0x6dadadae), CHC(0x5b8b2cfc), CHC(0x6cf61db2), CHC(0x46c3c90b),
         CHC(0x506314ea), CHC(0x5f034acd)},
        {0, -1, -3, -2, -5, -8},
        {CHC(0x6206cb18), CHC(0x42f8b8de), CHC(0x5bb4776f), CHC(0x769acc79)},
        {-9, -3, 0, 0},
    },
    {
        /* numBands=10 */
        {CHC(0x6afa7252), CHC(0x4feed3ed), CHC(0x6b94504d)},
        {-1, 0, 0},
        {CHC(0x60467899), CHC(0x4acbafba), CHC(0x76eb327f)},
        {3, 4, 1},
        {CHC(0x42415b15), CHC(0x431080da), CHC(0x420f1c32), CHC(0x7d0c1aeb)},
        {-9, -6, -3, -1},
        {CHC(0x62b2c7a4), CHC(0x51b040a6), CHC(0x56caddb4), CHC(0x7e74a2c8),
         CHC(0x4030adf5), CHC(0x43d1dc4f)},
        {0, -1, -3, -3, -5, -8},
        {CHC(0x42415b15), CHC(0x64e299b3), CHC(0x4d33b5e8), CHC(0x742cee5f)},
        {-9, -4, 0, 0},
    },
    {
        /* numBands=11 */
        {CHC(0x6b3258bb), CHC(0x50a21233), CHC(0x6bb03c19)},
        {-1, 0, 0},
        {CHC(0x5ff997c6), CHC(0x4a82706e), CHC(0x7a5aae36)},
        {3, 4, 1},
        {CHC(0x5d2fb4fb), CHC(0x685bddd8), CHC(0x71b5e983), CHC(0x7708c90b)},
        {-10, -7, -4, -1},
        {CHC(0x59aceea2), CHC(0x49c428a0), CHC(0x46ca5527), CHC(0x724be884),
         CHC(0x68e586da), CHC(0x643485b6)},
        {0, -1, -3, -3, -6, -9},
        {CHC(0x5d2fb4fb), CHC(0x4e3fad1a), CHC(0x42310ba2), CHC(0x71c8b3ce)},
        {-10, -4, 0, 0},
    },
    {
        /* numBands=12 */
        {CHC(0x6b5c4726), CHC(0x5128a4a8), CHC(0x6bc52ee1)},
        {-1, 0, 0},
        {CHC(0x5fc06618), CHC(0x4a4ce559), CHC(0x7d5c16e9)},
        {3, 4, 1},
        {CHC(0x43af8342), CHC(0x531533d3), CHC(0x633660a6), CHC(0x71ce6052)},
        {-10, -7, -4, -1},
        {CHC(0x522373d7), CHC(0x434150cb), CHC(0x75b58afc), CHC(0x68474f2d),
         CHC(0x575348a5), CHC(0x4c20973f)},
        {0, -1, -4, -3, -6, -9},
        {CHC(0x43af8342), CHC(0x7c4d3d11), CHC(0x732e13db), CHC(0x6f756ac4)},
        {-10, -5, -1, 0},
    },
    {
        /* numBands=13 */
        {CHC(0x6b7c8953), CHC(0x51903fcd), CHC(0x6bd54d2e)},
        {-1, 0, 0},
        {CHC(0x5f94abf0), CHC(0x4a2480fa), CHC(0x40013553)},
        {3, 4, 2},
        {CHC(0x6501236e), CHC(0x436b9c4e), CHC(0x578d7881), CHC(0x6d34f92e)},
        {-11, -7, -4, -1},
        {CHC(0x4bc0e2b2), CHC(0x7b9d12ac), CHC(0x636c1c1b), CHC(0x5fe15c2b),
         CHC(0x49d54879), CHC(0x7662cfa5)},
        {0, -2, -4, -3, -6, -10},
        {CHC(0x6501236e), CHC(0x64b059fe), CHC(0x656d8359), CHC(0x6d370900)},
        {-11, -5, -1, 0},
    },
    {
        /* numBands=14 */
        {CHC(0x6b95e276), CHC(0x51e1b637), CHC(0x6be1f7ed)},
        {-1, 0, 0},
        {CHC(0x5f727a1c), CHC(0x4a053e9c), CHC(0x412e528c)},
        {3, 4, 2},
        {CHC(0x4d178bd4), CHC(0x6f33b4e8), CHC(0x4e028f7f), CHC(0x691ee104)},
        {-11, -8, -4, -1},
        {CHC(0x46473d3f), CHC(0x725bd0a6), CHC(0x55199885), CHC(0x58bcc56b),
         CHC(0x7e7e6288), CHC(0x5ddef6eb)},
        {0, -2, -4, -3, -7, -10},
        {CHC(0x4d178bd4), CHC(0x52ebd467), CHC(0x5a395a6e), CHC(0x6b0f724f)},
        {-11, -5, -1, 0},
    },
    {
        /* numBands=15 */
        {CHC(0x6baa2a22), CHC(0x5222eb91), CHC(0x6bec1a86)},
        {-1, 0, 0},
        {CHC(0x5f57393b), CHC(0x49ec8934), CHC(0x423b5b58)},
        {3, 4, 2},
        {CHC(0x77fd2486), CHC(0x5cfbdf2c), CHC(0x46153bd1), CHC(0x65757ed9)},
        {-12, -8, -4, -1},
        {CHC(0x41888ee6), CHC(0x6a661db3), CHC(0x49abc8c8), CHC(0x52965848),
         CHC(0x6d9301b7), CHC(0x4bb04721)},
        {0, -2, -4, -3, -7, -10},
        {CHC(0x77fd2486), CHC(0x45424c68), CHC(0x50f33cc6), CHC(0x68ff43f0)},
        {-12, -5, -1, 0},
    },
    {
        /* numBands=16 */
        {CHC(0x6bbaa499), CHC(0x5257ed94), CHC(0x6bf456e4)},
        {-1, 0, 0},
        {CHC(0x5f412594), CHC(0x49d8a766), CHC(0x432d1dbd)},
        {3, 4, 2},
        {CHC(0x5ef5cfde), CHC(0x4eafcd2d), CHC(0x7ed36893), CHC(0x62274b45)},
        {-12, -8, -5, -1},
        {CHC(0x7ac438f5), CHC(0x637aab21), CHC(0x4067617a), CHC(0x4d3c6ec7),
         CHC(0x5fd6e0dd), CHC(0x7bd5f024)},
        {-1, -2, -4, -3, -7, -11},
        {CHC(0x5ef5cfde), CHC(0x751d0d4f), CHC(0x492b3c41), CHC(0x67065409)},
        {-12, -6, -1, 0},
    },
    {
        /* numBands=17 */
        {CHC(0x6bc836c9), CHC(0x5283997e), CHC(0x6bfb1f5e)},
        {-1, 0, 0},
        {CHC(0x5f2f02b6), CHC(0x49c868e9), CHC(0x44078151)},
        {3, 4, 2},
        {CHC(0x4c43b65a), CHC(0x4349dcf6), CHC(0x73799e2d), CHC(0x5f267274)},
        {-12, -8, -5, -1},
        {CHC(0x73726394), CHC(0x5d68511a), CHC(0x7191bbcc), CHC(0x48898c70),
         CHC(0x548956e1), CHC(0x66981ce8)},
        {-1, -2, -5, -3, -7, -11},
        {CHC(0x4c43b65a), CHC(0x64131116), CHC(0x429028e2), CHC(0x65240211)},
        {-12, -6, -1, 0},
    },
    {
        /* numBands=18 */
        {CHC(0x6bd3860d), CHC(0x52a80156), CHC(0x6c00c68d)},
        {-1, 0, 0},
        {CHC(0x5f1fed86), CHC(0x49baf636), CHC(0x44cdb9dc)},
        {3, 4, 2},
        {CHC(0x7c189389), CHC(0x742666d8), CHC(0x69b8c776), CHC(0x5c67e27d)},
        {-13, -9, -5, -1},
        {CHC(0x6cf1ea76), CHC(0x58095703), CHC(0x64e351a9), CHC(0x4460da90),
         CHC(0x4b1f8083), CHC(0x55f2d3e1)},
        {-1, -2, -5, -3, -7, -11},
        {CHC(0x7c189389), CHC(0x5651792a), CHC(0x79cb9b3d), CHC(0x635769c0)},
        {-13, -6, -2, 0},
    },
    {
        /* numBands=19 */
        {CHC(0x6bdd0c40), CHC(0x52c6abf6), CHC(0x6c058950)},
        {-1, 0, 0},
        {CHC(0x5f133f88), CHC(0x49afb305), CHC(0x45826d73)},
        {3, 4, 2},
        {CHC(0x6621a164), CHC(0x6512528e), CHC(0x61449fc8), CHC(0x59e2a0c0)},
        {-13, -9, -5, -1},
        {CHC(0x6721cadb), CHC(0x53404cd4), CHC(0x5a389e91), CHC(0x40abcbd2),
         CHC(0x43332f01), CHC(0x48b82e46)},
        {-1, -2, -5, -3, -7, -11},
        {CHC(0x6621a164), CHC(0x4b12cc28), CHC(0x6ffd4df8), CHC(0x619f835e)},
        {-13, -6, -2, 0},
    },
    {
        /* numBands=20 */
        {CHC(0x6be524c5), CHC(0x52e0beb3), CHC(0x6c099552)},
        {-1, 0, 0},
        {CHC(0x5f087c68), CHC(0x49a62bb5), CHC(0x4627d175)},
        {3, 4, 2},
        {CHC(0x54ec6afe), CHC(0x58991a42), CHC(0x59e23e8c), CHC(0x578f4ef4)},
        {-13, -9, -5, -1},
        {CHC(0x61e78f6f), CHC(0x4ef5e1e9), CHC(0x5129c3b8), CHC(0x7ab0f7b2),
         CHC(0x78efb076), CHC(0x7c2567ea)},
        {-1, -2, -5, -4, -8, -12},
        {CHC(0x54ec6afe), CHC(0x41c7812c), CHC(0x676f6f8d), CHC(0x5ffb383f)},
        {-13, -6, -2, 0},
    },
    {
        /* numBands=21 */
        {CHC(0x6bec1542), CHC(0x52f71929), CHC(0x6c0d0d5e)},
        {-1, 0, 0},
        {CHC(0x5eff45c5), CHC(0x499e092d), CHC(0x46bfc0c9)},
        {3, 4, 2},
        {CHC(0x47457a78), CHC(0x4e2d99b3), CHC(0x53637ea5), CHC(0x5567d0e9)},
        {-13, -9, -5, -1},
        {CHC(0x5d2dc61b), CHC(0x4b1760c8), CHC(0x4967cf39), CHC(0x74b113d8),
         CHC(0x6d6676b6), CHC(0x6ad114e9)},
        {-1, -2, -5, -4, -8, -12},
        {CHC(0x47457a78), CHC(0x740accaa), CHC(0x5feb6609), CHC(0x5e696f95)},
        {-13, -7, -2, 0},
    },
    {
        /* numBands=22 */
        {CHC(0x6bf21387), CHC(0x530a683c), CHC(0x6c100c59)},
        {-1, 0, 0},
        {CHC(0x5ef752ea), CHC(0x499708c6), CHC(0x474bcd1b)},
        {3, 4, 2},
        {CHC(0x78a21ab7), CHC(0x45658aec), CHC(0x4da3c4fe), CHC(0x5367094b)},
        {-14, -9, -5, -1},
        {CHC(0x58e2df6a), CHC(0x4795990e), CHC(0x42b5e0f7), CHC(0x6f408c64),
         CHC(0x6370bebf), CHC(0x5c91ca85)},
        {-1, -2, -5, -4, -8, -12},
        {CHC(0x78a21ab7), CHC(0x66f951d6), CHC(0x594605bb), CHC(0x5ce91657)},
        {-14, -7, -2, 0},
    },
    {
        /* numBands=23 */
        {CHC(0x6bf749b2), CHC(0x531b3348), CHC(0x6c12a750)},
        {-1, 0, 0},
        {CHC(0x5ef06b17), CHC(0x4990f6c9), CHC(0x47cd4c5b)},
        {3, 4, 2},
        {CHC(0x66dede36), CHC(0x7bdf90a9), CHC(0x4885b2b9), CHC(0x5188a6b7)},
        {-14, -10, -5, -1},
        {CHC(0x54f85812), CHC(0x446414ae), CHC(0x79c8d519), CHC(0x6a4c2f31),
         CHC(0x5ac8325f), CHC(0x50bf9200)},
        {-1, -2, -6, -4, -8, -12},
        {CHC(0x66dede36), CHC(0x5be0d90e), CHC(0x535cc453), CHC(0x5b7923f0)},
        {-14, -7, -2, 0},
    },
    {
        /* numBands=24 */
        {CHC(0x6bfbd91d), CHC(0x5329e580), CHC(0x6c14eeed)},
        {-1, 0, 0},
        {CHC(0x5eea6179), CHC(0x498baa90), CHC(0x4845635d)},
        {3, 4, 2},
        {CHC(0x58559b7e), CHC(0x6f1b231f), CHC(0x43f1789b), CHC(0x4fc8fcb8)},
        {-14, -10, -5, -1},
        {CHC(0x51621775), CHC(0x417881a3), CHC(0x6f9ba9b6), CHC(0x65c412b2),
         CHC(0x53352c61), CHC(0x46db9caf)},
        {-1, -2, -6, -4, -8, -12},
        {CHC(0x58559b7e), CHC(0x52636003), CHC(0x4e13b316), CHC(0x5a189cdf)},
        {-14, -7, -2, 0},
    },
    {
        /* numBands=25 */
        {CHC(0x6bffdc73), CHC(0x5336d4af), CHC(0x6c16f084)},
        {-1, 0, 0},
        {CHC(0x5ee51249), CHC(0x498703cc), CHC(0x48b50e4f)},
        {3, 4, 2},
        {CHC(0x4c5616cf), CHC(0x641b9fad), CHC(0x7fa735e0), CHC(0x4e24e57a)},
        {-14, -10, -6, -1},
        {CHC(0x4e15f47a), CHC(0x7d9481d6), CHC(0x66a82f8a), CHC(0x619ae971),
         CHC(0x4c8b2f5f), CHC(0x7d09ec11)},
        {-1, -3, -6, -4, -8, -13},
        {CHC(0x4c5616cf), CHC(0x4a3770fb), CHC(0x495402de), CHC(0x58c693fa)},
        {-14, -7, -2, 0},
    },
    {
        /* numBands=26 */
        {CHC(0x6c036943), CHC(0x53424625), CHC(0x6c18b6dc)},
        {-1, 0, 0},
        {CHC(0x5ee060aa), CHC(0x4982e88a), CHC(0x491d277f)},
        {3, 4, 2},
        {CHC(0x425ada5b), CHC(0x5a9368ac), CHC(0x78380a42), CHC(0x4c99aa05)},
        {-14, -10, -6, -1},
        {CHC(0x4b0b569c), CHC(0x78a420da), CHC(0x5ebdf203), CHC(0x5dc57e63),
         CHC(0x46a650ff), CHC(0x6ee13fb8)},
        {-1, -3, -6, -4, -8, -13},
        {CHC(0x425ada5b), CHC(0x4323073c), CHC(0x450ae92b), CHC(0x57822ad5)},
        {-14, -7, -2, 0},
    },
    {
        /* numBands=27 */
        {CHC(0x6c06911a), CHC(0x534c7261), CHC(0x6c1a4aba)},
        {-1, 0, 0},
        {CHC(0x5edc3524), CHC(0x497f43c0), CHC(0x497e6cd8)},
        {3, 4, 2},
        {CHC(0x73fb550e), CHC(0x5244894f), CHC(0x717aad78), CHC(0x4b24ef6c)},
        {-15, -10, -6, -1},
        {CHC(0x483aebe4), CHC(0x74139116), CHC(0x57b58037), CHC(0x5a3a4f3c),
         CHC(0x416950fe), CHC(0x62c7f4f2)},
        {-1, -3, -6, -4, -8, -13},
        {CHC(0x73fb550e), CHC(0x79efb994), CHC(0x4128cab7), CHC(0x564a919a)},
        {-15, -8, -2, 0},
    },
    {
        /* numBands=28 */
        {CHC(0x6c096264), CHC(0x535587cd), CHC(0x6c1bb355)},
        {-1, 0, 0},
        {CHC(0x5ed87c76), CHC(0x497c0439), CHC(0x49d98452)},
        {3, 4, 2},
        {CHC(0x65dec5bf), CHC(0x4afd1ba3), CHC(0x6b58b4b3), CHC(0x49c4a7b0)},
        {-15, -10, -6, -1},
        {CHC(0x459e6eb1), CHC(0x6fd850b7), CHC(0x516e7be9), CHC(0x56f13d05),
         CHC(0x79785594), CHC(0x58617de7)},
        {-1, -3, -6, -4, -9, -13},
        {CHC(0x65dec5bf), CHC(0x6f2168aa), CHC(0x7b41310f), CHC(0x551f0692)},
        {-15, -8, -3, 0},
    },
    {
        /* numBands=29 */
        {CHC(0x6c0be913), CHC(0x535dacd5), CHC(0x6c1cf6a3)},
        {-1, 0, 0},
        {CHC(0x5ed526b4), CHC(0x49791bc5), CHC(0x4a2eff99)},
        {3, 4, 2},
        {CHC(0x59e44afe), CHC(0x44949ada), CHC(0x65bf36f5), CHC(0x487705a0)},
        {-15, -10, -6, -1},
        {CHC(0x43307779), CHC(0x6be959c4), CHC(0x4bce2122), CHC(0x53e34d89),
         CHC(0x7115ff82), CHC(0x4f6421a1)},
        {-1, -3, -6, -4, -9, -13},
        {CHC(0x59e44afe), CHC(0x659eab7d), CHC(0x74cea459), CHC(0x53fed574)},
        {-15, -8, -3, 0},
    },
    {
        /* numBands=30 */
        {CHC(0x6c0e2f17), CHC(0x53650181), CHC(0x6c1e199d)},
        {-1, 0, 0},
        {CHC(0x5ed2269f), CHC(0x49767e9e), CHC(0x4a7f5f0b)},
        {3, 4, 2},
        {CHC(0x4faa4ae6), CHC(0x7dd3bf11), CHC(0x609e2732), CHC(0x473a72e9)},
        {-15, -11, -6, -1},
        {CHC(0x40ec57c6), CHC(0x683ee147), CHC(0x46be261d), CHC(0x510a7983),
         CHC(0x698a84cb), CHC(0x4794a927)},
        {-1, -3, -6, -4, -9, -13},
        {CHC(0x4faa4ae6), CHC(0x5d3615ad), CHC(0x6ee74773), CHC(0x52e956a1)},
        {-15, -8, -3, 0},
    },
    {
        /* numBands=31 */
        {CHC(0x6c103cc9), CHC(0x536ba0ac), CHC(0x6c1f2070)},
        {-1, 0, 0},
        {CHC(0x5ecf711e), CHC(0x497422ea), CHC(0x4acb1438)},
        {3, 4, 2},
        {CHC(0x46e322ad), CHC(0x73c32f3c), CHC(0x5be7d172), CHC(0x460d8800)},
        {-15, -11, -6, -1},
        {CHC(0x7d9bf8ad), CHC(0x64d22351), CHC(0x422bdc81), CHC(0x4e6184aa),
         CHC(0x62ba2375), CHC(0x40c325de)},
        {-2, -3, -6, -4, -9, -13},
        {CHC(0x46e322ad), CHC(0x55bef2a3), CHC(0x697b3135), CHC(0x51ddee4d)},
        {-15, -8, -3, 0},
    },
    {
        // numBands=32
        {CHC(0x6c121933), CHC(0x5371a104), CHC(0x6c200ea0)},
        {-1, 0, 0},
        {CHC(0x5eccfcd3), CHC(0x49720060), CHC(0x4b1283f0)},
        {3, 4, 2},
        {CHC(0x7ea12a52), CHC(0x6aca3303), CHC(0x579072bf), CHC(0x44ef056e)},
        {-16, -11, -6, -1},
        {CHC(0x79a3a9ab), CHC(0x619d38fc), CHC(0x7c0f0734), CHC(0x4be3dd5d),
         CHC(0x5c8d7163), CHC(0x7591065f)},
        {-2, -3, -7, -4, -9, -14},
        {CHC(0x7ea12a52), CHC(0x4f1782a6), CHC(0x647cbcb2), CHC(0x50dc0bb1)},
        {-16, -8, -3, 0},
    },
};

/** \def  SUM_SAFETY
 *
 *  SUM_SAFTEY defines the bits needed to right-shift every summand in
 *  order to be overflow-safe. In the two backsubst functions we sum up 4
 *  values. Since one of which is definitely not MAXVAL_DBL (the L[x][y]),
 *  we spare just 2 safety bits instead of 3.
 */
#define SUM_SAFETY 2

/**
 * \brief  Solves L*x=b via backsubstitution according to the following
 * structure:
 *
 *  x[0] =  b[0];
 *  x[1] = (b[1]                               - x[0]) / L[1][1];
 *  x[2] = (b[2] - x[1]*L[2][1]                - x[0]) / L[2][2];
 *  x[3] = (b[3] - x[2]*L[3][2] - x[1]*L[3][1] - x[0]) / L[3][3];
 *
 * \param[in]  numBands  SBR crossover band index
 * \param[in]  b         the b in L*x=b (one-dimensional)
 * \param[out] x         output polynomial coefficients (mantissa)
 * \param[out] x_sf      exponents of x[]
 */
static void backsubst_fw(const int numBands, const FIXP_DBL *const b,
                         FIXP_DBL *RESTRICT x, int *RESTRICT x_sf) {
  int i, k;
  int m; /* the trip counter that indexes incrementally through Lnorm1d[] */

  const FIXP_CHB *RESTRICT pLnorm1d = bsd[numBands - BSD_IDX_OFFSET].Lnorm1d;
  const SCHAR *RESTRICT pLnorm1d_sf = bsd[numBands - BSD_IDX_OFFSET].Lnorm1d_sf;
  const FIXP_CHB *RESTRICT pLnormii = bsd[numBands - BSD_IDX_OFFSET].Lnormii;
  const SCHAR *RESTRICT pLnormii_sf = bsd[numBands - BSD_IDX_OFFSET].Lnormii_sf;

  x[0] = b[0];

  for (i = 1, m = 0; i <= POLY_ORDER; ++i) {
    FIXP_DBL sum = b[i] >> SUM_SAFETY;
    int sum_sf = x_sf[i];
    for (k = i - 1; k > 0; --k, ++m) {
      int e;
      FIXP_DBL mult = fMultNorm(FX_CHB2FX_DBL(pLnorm1d[m]), x[k], &e);
      int mult_sf = pLnorm1d_sf[m] + x_sf[k] + e;

      /* check if the new summand mult has a different sf than the sum currently
       * has */
      int diff = mult_sf - sum_sf;

      if (diff > 0) {
        /* yes, and it requires the sum to be adjusted (scaled down) */
        sum >>= diff;
        sum_sf = mult_sf;
      } else if (diff < 0) {
        /* yes, but here mult needs to be scaled down */
        mult >>= -diff;
      }
      sum -= (mult >> SUM_SAFETY);
    }

    /* - x[0] */
    if (x_sf[0] > sum_sf) {
      sum >>= (x_sf[0] - sum_sf);
      sum_sf = x_sf[0];
    }
    sum -= (x[0] >> (sum_sf - x_sf[0] + SUM_SAFETY));

    /* instead of the division /L[i][i], we multiply by the inverse */
    int e;
    x[i] = fMultNorm(sum, FX_CHB2FX_DBL(pLnormii[i - 1]), &e);
    x_sf[i] = sum_sf + pLnormii_sf[i - 1] + e + SUM_SAFETY;
  }
}

/**
 * \brief Solves L*x=b via backsubstitution according to the following
 * structure:
 *
 *  x[3] = b[3];
 *  x[2] = b[2] - L[2][3]*x[3];
 *  x[1] = b[1] - L[1][2]*x[2] - L[1][3]*x[3];
 *  x[0] = b[0] - L[0][1]*x[1] - L[0][2]*x[2] - L[0][3]*x[3];
 *
 * \param[in]  numBands  SBR crossover band index
 * \param[in]  b         the b in L*x=b (one-dimensional)
 * \param[out] x         solution vector
 * \param[out] x_sf      exponents of x[]
 */
static void backsubst_bw(const int numBands, const FIXP_DBL *const b,
                         FIXP_DBL *RESTRICT x, int *RESTRICT x_sf) {
  int i, k;
  int m; /* the trip counter that indexes incrementally through LnormInv1d[] */

  const FIXP_CHB *RESTRICT pLnormInv1d =
      bsd[numBands - BSD_IDX_OFFSET].LnormInv1d;
  const SCHAR *RESTRICT pLnormInv1d_sf =
      bsd[numBands - BSD_IDX_OFFSET].LnormInv1d_sf;

  x[POLY_ORDER] = b[POLY_ORDER];

  for (i = POLY_ORDER - 1, m = 0; i >= 0; i--) {
    FIXP_DBL sum = b[i] >> SUM_SAFETY;
    int sum_sf = x_sf[i]; /* sum's sf but disregarding SUM_SAFETY (added at the
                             iteration's end) */

    for (k = i + 1; k <= POLY_ORDER; ++k, ++m) {
      int e;
      FIXP_DBL mult = fMultNorm(FX_CHB2FX_DBL(pLnormInv1d[m]), x[k], &e);
      int mult_sf = pLnormInv1d_sf[m] + x_sf[k] + e;

      /* check if the new summand mult has a different sf than sum currently has
       */
      int diff = mult_sf - sum_sf;

      if (diff > 0) {
        /* yes, and it requires the sum v to be adjusted (scaled down) */
        sum >>= diff;
        sum_sf = mult_sf;
      } else if (diff < 0) {
        /* yes, but here mult needs to be scaled down */
        mult >>= -diff;
      }

      /* mult has now the same sf than what it is about to be added to. */
      /* scale mult down additionally so that building the sum is overflow-safe.
       */
      sum -= (mult >> SUM_SAFETY);
    }

    x_sf[i] = sum_sf + SUM_SAFETY;
    x[i] = sum;
  }
}

/**
 * \brief  Solves a system of linear equations (L*x=b) with the Cholesky
 * algorithm.
 *
 * \param[in]     numBands  SBR crossover band index
 * \param[in,out] b         input: vector b, output: solution vector p.
 * \param[in,out] b_sf      input: exponent of b; output: exponent of solution
 * p.
 */
static void choleskySolve(const int numBands, FIXP_DBL *RESTRICT b,
                          int *RESTRICT b_sf) {
  int i, e;

  const FIXP_CHB *RESTRICT pBmul0 = bsd[numBands - BSD_IDX_OFFSET].Bmul0;
  const SCHAR *RESTRICT pBmul0_sf = bsd[numBands - BSD_IDX_OFFSET].Bmul0_sf;
  const FIXP_CHB *RESTRICT pBmul1 = bsd[numBands - BSD_IDX_OFFSET].Bmul1;
  const SCHAR *RESTRICT pBmul1_sf = bsd[numBands - BSD_IDX_OFFSET].Bmul1_sf;

  /* normalize b */
  FIXP_DBL bnormed[POLY_ORDER + 1];
  for (i = 0; i <= POLY_ORDER; ++i) {
    bnormed[i] = fMultNorm(b[i], FX_CHB2FX_DBL(pBmul0[i]), &e);
    b_sf[i] += pBmul0_sf[i] + e;
  }

  backsubst_fw(numBands, bnormed, b, b_sf);

  /* normalize b again */
  for (i = 0; i <= POLY_ORDER; ++i) {
    bnormed[i] = fMultNorm(b[i], FX_CHB2FX_DBL(pBmul1[i]), &e);
    b_sf[i] += pBmul1_sf[i] + e;
  }

  backsubst_bw(numBands, bnormed, b, b_sf);
}

/**
 * \brief  Find polynomial approximation of vector y with implicit abscisas
 * x=0,1,2,3..n-1
 *
 *  The problem (V^T * V * p = V^T * y) is solved with Cholesky.
 *  V is the Vandermode Matrix constructed with x = 0...n-1;
 *  A = V^T * V; b = V^T * y;
 *
 * \param[in]  numBands  SBR crossover band index (BSD_IDX_OFFSET <= numBands <=
 * MAXLOWBANDS)
 * \param[in]  y         input vector (mantissa)
 * \param[in]  y_sf      exponents of y[]
 * \param[out] p         output polynomial coefficients (mantissa)
 * \param[out] p_sf      exponents of p[]
 */
static void polyfit(const int numBands, const FIXP_DBL *const y, const int y_sf,
                    FIXP_DBL *RESTRICT p, int *RESTRICT p_sf) {
  int i, k;
  LONG v[POLY_ORDER + 1];
  int sum_saftey = getLog2[numBands - 1];

  FDK_ASSERT((numBands >= BSD_IDX_OFFSET) && (numBands <= MAXLOWBANDS));

  /* construct vector b[] temporarily stored in array p[] */
  FDKmemclear(p, (POLY_ORDER + 1) * sizeof(FIXP_DBL));

  /* p[] are the sums over n values and each p[i] has its own sf */
  for (i = 0; i <= POLY_ORDER; ++i) p_sf[i] = 1 - DFRACT_BITS;

  for (k = 0; k < numBands; k++) {
    v[0] = (LONG)1;
    for (i = 1; i <= POLY_ORDER; i++) {
      v[i] = k * v[i - 1];
    }

    for (i = 0; i <= POLY_ORDER; i++) {
      if (v[POLY_ORDER - i] != 0 && y[k] != FIXP_DBL(0)) {
        int e;
        FIXP_DBL mult = fMultNorm((FIXP_DBL)v[POLY_ORDER - i], y[k], &e);
        int sf = DFRACT_BITS - 1 + y_sf + e;

        /* check if the new summand has a different sf than the sum p[i]
         * currently has */
        int diff = sf - p_sf[i];

        if (diff > 0) {
          /* yes, and it requires the sum p[i] to be adjusted (scaled down) */
          p[i] >>= fMin(DFRACT_BITS - 1, diff);
          p_sf[i] = sf;
        } else if (diff < 0) {
          /* yes, but here mult needs to be scaled down */
          mult >>= -diff;
        }

        /* mult has now the same sf than what it is about to be added to.
           scale mult down additionally so that building the sum is
           overflow-safe. */
        p[i] += mult >> sum_saftey;
      }
    }
  }

  p_sf[0] += sum_saftey;
  p_sf[1] += sum_saftey;
  p_sf[2] += sum_saftey;
  p_sf[3] += sum_saftey;

  choleskySolve(numBands, p, p_sf);
}

/**
 * \brief  Calculates the output of a POLY_ORDER-degree polynomial function
 *         with Horner scheme:
 *
 *         y(x) = p3 + p2*x + p1*x^2 + p0*x^3
 *              = p3 + x*(p2 + x*(p1 + x*p0))
 *
 *         The for loop iterates through the mult/add parts in y(x) as above,
 *         during which regular upscaling ensures a stable exponent of the
 *         result.
 *
 * \param[in]  p       coefficients as in y(x)
 * \param[in]  p_sf    exponents of p[]
 * \param[in]  x_int   non-fractional integer representation of x as in y(x)
 * \param[out] out_sf  exponent of return value
 *
 * \return             result y(x)
 */
static FIXP_DBL polyval(const FIXP_DBL *const p, const int *const p_sf,
                        const int x_int, int *out_sf) {
  FDK_ASSERT(x_int <= 31); /* otherwise getLog2[] needs more elements */

  int k, x_sf;
  int result_sf;   /* working space to compute return value *out_sf */
  FIXP_DBL x;      /* fractional value of x_int */
  FIXP_DBL result; /* return value */

  /* if x == 0, then y(x) is just p3 */
  if (x_int != 0) {
    x_sf = getLog2[x_int];
    x = (FIXP_DBL)x_int << (DFRACT_BITS - 1 - x_sf);
  } else {
    *out_sf = p_sf[3];
    return p[3];
  }

  result = p[0];
  result_sf = p_sf[0];

  for (k = 1; k <= POLY_ORDER; ++k) {
    FIXP_DBL mult = fMult(x, result);
    int mult_sf = x_sf + result_sf;

    int room = CountLeadingBits(mult);
    mult <<= room;
    mult_sf -= room;

    FIXP_DBL pp = p[k];
    int pp_sf = p_sf[k];

    /* equalize the shift factors of pp and mult so that we can sum them up */
    int diff = pp_sf - mult_sf;

    if (diff > 0) {
      diff = fMin(diff, DFRACT_BITS - 1);
      mult >>= diff;
    } else if (diff < 0) {
      diff = fMax(diff, 1 - DFRACT_BITS);
      pp >>= -diff;
    }

    /* downshift by 1 to ensure safe summation */
    mult >>= 1;
    mult_sf++;
    pp >>= 1;
    pp_sf++;

    result_sf = fMax(pp_sf, mult_sf);

    result = mult + pp;
    /* rarely, mult and pp happen to be almost equal except their sign,
    and then upon summation, result becomes so small, that it is within
    the inaccuracy range of a few bits, and then the relative error
    produced by this function may become HUGE */
  }

  *out_sf = result_sf;
  return result;
}

void sbrDecoder_calculateGainVec(FIXP_DBL **sourceBufferReal,
                                 FIXP_DBL **sourceBufferImag,
                                 int sourceBuf_e_overlap,
                                 int sourceBuf_e_current, int overlap,
                                 FIXP_DBL *RESTRICT GainVec, int *GainVec_exp,
                                 int numBands, const int startSample,
                                 const int stopSample) {
  FIXP_DBL p[POLY_ORDER + 1];
  FIXP_DBL meanNrg;
  FIXP_DBL LowEnv[MAXLOWBANDS];
  FIXP_DBL invNumBands = GetInvInt(numBands);
  FIXP_DBL invNumSlots = GetInvInt(stopSample - startSample);
  int i, loBand, exp, scale_nrg, scale_nrg_ov;
  int sum_scale = 5, sum_scale_ov = 3;

  if (overlap > 8) {
    FDK_ASSERT(overlap <= 16);
    sum_scale_ov += 1;
    sum_scale += 1;
  }

  /* exponents of energy values */
  sourceBuf_e_overlap = sourceBuf_e_overlap * 2 + sum_scale_ov;
  sourceBuf_e_current = sourceBuf_e_current * 2 + sum_scale;
  exp = fMax(sourceBuf_e_overlap, sourceBuf_e_current);
  scale_nrg = sourceBuf_e_current - exp;
  scale_nrg_ov = sourceBuf_e_overlap - exp;

  meanNrg = (FIXP_DBL)0;
  /* Calculate the spectral envelope in dB over the current copy-up frame. */
  for (loBand = 0; loBand < numBands; loBand++) {
    FIXP_DBL nrg_ov, nrg;
    INT reserve = 0, exp_new;
    FIXP_DBL maxVal = FL2FX_DBL(0.0f);

    for (i = startSample; i < stopSample; i++) {
      maxVal |=
          (FIXP_DBL)((LONG)(sourceBufferReal[i][loBand]) ^
                     ((LONG)sourceBufferReal[i][loBand] >> (DFRACT_BITS - 1)));
      maxVal |=
          (FIXP_DBL)((LONG)(sourceBufferImag[i][loBand]) ^
                     ((LONG)sourceBufferImag[i][loBand] >> (DFRACT_BITS - 1)));
    }

    if (maxVal != FL2FX_DBL(0.0f)) {
      reserve = CntLeadingZeros(maxVal) - 2;
    }

    nrg_ov = nrg = (FIXP_DBL)0;
    if (scale_nrg_ov > -31) {
      for (i = startSample; i < overlap; i++) {
        nrg_ov +=
            (fPow2Div2(scaleValue(sourceBufferReal[i][loBand], reserve)) +
             fPow2Div2(scaleValue(sourceBufferImag[i][loBand], reserve))) >>
            sum_scale_ov;
      }
    } else {
      scale_nrg_ov = 0;
    }
    if (scale_nrg > -31) {
      for (i = overlap; i < stopSample; i++) {
        nrg += (fPow2Div2(scaleValue(sourceBufferReal[i][loBand], reserve)) +
                fPow2Div2(scaleValue(sourceBufferImag[i][loBand], reserve))) >>
               sum_scale;
      }
    } else {
      scale_nrg = 0;
    }

    nrg = (scaleValue(nrg_ov, scale_nrg_ov) >> 1) +
          (scaleValue(nrg, scale_nrg) >> 1);
    nrg = fMult(nrg, invNumSlots);

    exp_new =
        exp - (2 * reserve) +
        2; /* +1 for addition directly above, +1 for fPow2Div2 in loops above */

    /* LowEnv = 10*log10(nrg) = log2(nrg) * 10/log2(10) */
    /* exponent of logarithmic energy is 8 */
    if (nrg > (FIXP_DBL)0) {
      int exp_log2;
      nrg = CalcLog2(nrg, exp_new, &exp_log2);
      nrg = scaleValue(nrg, exp_log2 - 6);
      nrg = fMult(FL2FXCONST_SGL(LOG10FAC), nrg);
    } else {
      nrg = (FIXP_DBL)0;
    }
    LowEnv[loBand] = nrg;
    meanNrg += fMult(nrg, invNumBands);
  }
  exp = 6 + 2; /* exponent of LowEnv: +2 is exponent of LOG10FAC */

  /* subtract mean before polynomial approximation to reduce dynamic of p[] */
  for (loBand = 0; loBand < numBands; loBand++) {
    LowEnv[loBand] = meanNrg - LowEnv[loBand];
  }

  /* For numBands < BSD_IDX_OFFSET (== POLY_ORDER+2) we dont get an
     overdetermined equation system. The calculated polynomial will exactly fit
     the input data and evaluating the polynomial will lead to the same vector
     than the original input vector: lowEnvSlope[] == lowEnv[]
  */
  if (numBands > POLY_ORDER + 1) {
    /* Find polynomial approximation of LowEnv */
    int p_sf[POLY_ORDER + 1];

    polyfit(numBands, LowEnv, exp, p, p_sf);

    for (i = 0; i < numBands; i++) {
      int sf;

      /* lowBandEnvSlope[i] = tmp; */
      FIXP_DBL tmp = polyval(p, p_sf, i, &sf);

      /* GainVec = 10^((mean(y)-y)/20) = 2^( (mean(y)-y) * log2(10)/20 ) */
      tmp = fMult(tmp, FL2FXCONST_SGL(LOG10FAC_INV));
      GainVec[i] = f2Pow(tmp, sf - 2,
                         &GainVec_exp[i]); /* -2 is exponent of LOG10FAC_INV */
    }
  } else { /* numBands <= POLY_ORDER+1 */
    for (i = 0; i < numBands; i++) {
      int sf = exp; /* exponent of LowEnv[] */

      /* lowBandEnvSlope[i] = LowEnv[i]; */
      FIXP_DBL tmp = LowEnv[i];

      /* GainVec = 10^((mean(y)-y)/20) = 2^( (mean(y)-y) * log2(10)/20 ) */
      tmp = fMult(tmp, FL2FXCONST_SGL(LOG10FAC_INV));
      GainVec[i] = f2Pow(tmp, sf - 2,
                         &GainVec_exp[i]); /* -2 is exponent of LOG10FAC_INV */
    }
  }
}
