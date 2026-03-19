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

   Author(s):   Josef Hoepfl, Tobias Chalupka

   Description: Definition of constant tables

*******************************************************************************/

#include "aac_rom.h"

/* Prescale InverseQuantTable by 4 to save
   redundant shifts in invers quantization
 */
#define SCL_TAB(a) (a >> 4)
const FIXP_DBL InverseQuantTable[INV_QUANT_TABLESIZE + 1] = {
    SCL_TAB(0x32CBFD40), SCL_TAB(0x330FC340), SCL_TAB(0x33539FC0),
    SCL_TAB(0x33979280), SCL_TAB(0x33DB9BC0), SCL_TAB(0x341FBB80),
    SCL_TAB(0x3463F180), SCL_TAB(0x34A83DC0), SCL_TAB(0x34ECA000),
    SCL_TAB(0x35311880), SCL_TAB(0x3575A700), SCL_TAB(0x35BA4B80),
    SCL_TAB(0x35FF0600), SCL_TAB(0x3643D680), SCL_TAB(0x3688BCC0),
    SCL_TAB(0x36CDB880), SCL_TAB(0x3712CA40), SCL_TAB(0x3757F1C0),
    SCL_TAB(0x379D2F00), SCL_TAB(0x37E28180), SCL_TAB(0x3827E9C0),
    SCL_TAB(0x386D6740), SCL_TAB(0x38B2FA40), SCL_TAB(0x38F8A2C0),
    SCL_TAB(0x393E6080), SCL_TAB(0x39843380), SCL_TAB(0x39CA1BC0),
    SCL_TAB(0x3A101940), SCL_TAB(0x3A562BC0), SCL_TAB(0x3A9C5340),
    SCL_TAB(0x3AE28FC0), SCL_TAB(0x3B28E180), SCL_TAB(0x3B6F4800),
    SCL_TAB(0x3BB5C340), SCL_TAB(0x3BFC5380), SCL_TAB(0x3C42F880),
    SCL_TAB(0x3C89B200), SCL_TAB(0x3CD08080), SCL_TAB(0x3D176340),
    SCL_TAB(0x3D5E5B00), SCL_TAB(0x3DA56700), SCL_TAB(0x3DEC87C0),
    SCL_TAB(0x3E33BCC0), SCL_TAB(0x3E7B0640), SCL_TAB(0x3EC26400),
    SCL_TAB(0x3F09D640), SCL_TAB(0x3F515C80), SCL_TAB(0x3F98F740),
    SCL_TAB(0x3FE0A600), SCL_TAB(0x40286900), SCL_TAB(0x40704000),
    SCL_TAB(0x40B82B00), SCL_TAB(0x41002A00), SCL_TAB(0x41483D00),
    SCL_TAB(0x41906400), SCL_TAB(0x41D89F00), SCL_TAB(0x4220ED80),
    SCL_TAB(0x42695000), SCL_TAB(0x42B1C600), SCL_TAB(0x42FA5000),
    SCL_TAB(0x4342ED80), SCL_TAB(0x438B9E80), SCL_TAB(0x43D46380),
    SCL_TAB(0x441D3B80), SCL_TAB(0x44662780), SCL_TAB(0x44AF2680),
    SCL_TAB(0x44F83900), SCL_TAB(0x45415F00), SCL_TAB(0x458A9880),
    SCL_TAB(0x45D3E500), SCL_TAB(0x461D4500), SCL_TAB(0x4666B800),
    SCL_TAB(0x46B03E80), SCL_TAB(0x46F9D800), SCL_TAB(0x47438480),
    SCL_TAB(0x478D4400), SCL_TAB(0x47D71680), SCL_TAB(0x4820FC00),
    SCL_TAB(0x486AF500), SCL_TAB(0x48B50000), SCL_TAB(0x48FF1E80),
    SCL_TAB(0x49494F80), SCL_TAB(0x49939380), SCL_TAB(0x49DDEA80),
    SCL_TAB(0x4A285400), SCL_TAB(0x4A72D000), SCL_TAB(0x4ABD5E80),
    SCL_TAB(0x4B080000), SCL_TAB(0x4B52B400), SCL_TAB(0x4B9D7A80),
    SCL_TAB(0x4BE85380), SCL_TAB(0x4C333F00), SCL_TAB(0x4C7E3D00),
    SCL_TAB(0x4CC94D00), SCL_TAB(0x4D146F80), SCL_TAB(0x4D5FA500),
    SCL_TAB(0x4DAAEC00), SCL_TAB(0x4DF64580), SCL_TAB(0x4E41B180),
    SCL_TAB(0x4E8D2F00), SCL_TAB(0x4ED8BF80), SCL_TAB(0x4F246180),
    SCL_TAB(0x4F701600), SCL_TAB(0x4FBBDC00), SCL_TAB(0x5007B480),
    SCL_TAB(0x50539F00), SCL_TAB(0x509F9B80), SCL_TAB(0x50EBA980),
    SCL_TAB(0x5137C980), SCL_TAB(0x5183FB80), SCL_TAB(0x51D03F80),
    SCL_TAB(0x521C9500), SCL_TAB(0x5268FC80), SCL_TAB(0x52B57580),
    SCL_TAB(0x53020000), SCL_TAB(0x534E9C80), SCL_TAB(0x539B4A80),
    SCL_TAB(0x53E80A80), SCL_TAB(0x5434DB80), SCL_TAB(0x5481BE80),
    SCL_TAB(0x54CEB280), SCL_TAB(0x551BB880), SCL_TAB(0x5568CF80),
    SCL_TAB(0x55B5F800), SCL_TAB(0x56033200), SCL_TAB(0x56507D80),
    SCL_TAB(0x569DDA00), SCL_TAB(0x56EB4800), SCL_TAB(0x5738C700),
    SCL_TAB(0x57865780), SCL_TAB(0x57D3F900), SCL_TAB(0x5821AC00),
    SCL_TAB(0x586F7000), SCL_TAB(0x58BD4500), SCL_TAB(0x590B2B00),
    SCL_TAB(0x59592200), SCL_TAB(0x59A72A80), SCL_TAB(0x59F54380),
    SCL_TAB(0x5A436D80), SCL_TAB(0x5A91A900), SCL_TAB(0x5ADFF500),
    SCL_TAB(0x5B2E5180), SCL_TAB(0x5B7CBF80), SCL_TAB(0x5BCB3E00),
    SCL_TAB(0x5C19CD00), SCL_TAB(0x5C686D80), SCL_TAB(0x5CB71E00),
    SCL_TAB(0x5D05DF80), SCL_TAB(0x5D54B200), SCL_TAB(0x5DA39500),
    SCL_TAB(0x5DF28880), SCL_TAB(0x5E418C80), SCL_TAB(0x5E90A100),
    SCL_TAB(0x5EDFC680), SCL_TAB(0x5F2EFC00), SCL_TAB(0x5F7E4280),
    SCL_TAB(0x5FCD9900), SCL_TAB(0x601D0080), SCL_TAB(0x606C7800),
    SCL_TAB(0x60BC0000), SCL_TAB(0x610B9800), SCL_TAB(0x615B4100),
    SCL_TAB(0x61AAF980), SCL_TAB(0x61FAC300), SCL_TAB(0x624A9C80),
    SCL_TAB(0x629A8600), SCL_TAB(0x62EA8000), SCL_TAB(0x633A8A00),
    SCL_TAB(0x638AA480), SCL_TAB(0x63DACF00), SCL_TAB(0x642B0980),
    SCL_TAB(0x647B5400), SCL_TAB(0x64CBAE80), SCL_TAB(0x651C1900),
    SCL_TAB(0x656C9400), SCL_TAB(0x65BD1E80), SCL_TAB(0x660DB900),
    SCL_TAB(0x665E6380), SCL_TAB(0x66AF1E00), SCL_TAB(0x66FFE880),
    SCL_TAB(0x6750C280), SCL_TAB(0x67A1AC80), SCL_TAB(0x67F2A600),
    SCL_TAB(0x6843B000), SCL_TAB(0x6894C900), SCL_TAB(0x68E5F200),
    SCL_TAB(0x69372B00), SCL_TAB(0x69887380), SCL_TAB(0x69D9CB80),
    SCL_TAB(0x6A2B3300), SCL_TAB(0x6A7CAA80), SCL_TAB(0x6ACE3180),
    SCL_TAB(0x6B1FC800), SCL_TAB(0x6B716E00), SCL_TAB(0x6BC32400),
    SCL_TAB(0x6C14E900), SCL_TAB(0x6C66BD80), SCL_TAB(0x6CB8A180),
    SCL_TAB(0x6D0A9500), SCL_TAB(0x6D5C9800), SCL_TAB(0x6DAEAA00),
    SCL_TAB(0x6E00CB80), SCL_TAB(0x6E52FC80), SCL_TAB(0x6EA53D00),
    SCL_TAB(0x6EF78C80), SCL_TAB(0x6F49EB80), SCL_TAB(0x6F9C5980),
    SCL_TAB(0x6FEED700), SCL_TAB(0x70416380), SCL_TAB(0x7093FF00),
    SCL_TAB(0x70E6AA00), SCL_TAB(0x71396400), SCL_TAB(0x718C2D00),
    SCL_TAB(0x71DF0580), SCL_TAB(0x7231ED00), SCL_TAB(0x7284E300),
    SCL_TAB(0x72D7E880), SCL_TAB(0x732AFD00), SCL_TAB(0x737E2080),
    SCL_TAB(0x73D15300), SCL_TAB(0x74249480), SCL_TAB(0x7477E480),
    SCL_TAB(0x74CB4400), SCL_TAB(0x751EB200), SCL_TAB(0x75722F00),
    SCL_TAB(0x75C5BB00), SCL_TAB(0x76195580), SCL_TAB(0x766CFF00),
    SCL_TAB(0x76C0B700), SCL_TAB(0x77147E00), SCL_TAB(0x77685400),
    SCL_TAB(0x77BC3880), SCL_TAB(0x78102B80), SCL_TAB(0x78642D80),
    SCL_TAB(0x78B83E00), SCL_TAB(0x790C5D00), SCL_TAB(0x79608B00),
    SCL_TAB(0x79B4C780), SCL_TAB(0x7A091280), SCL_TAB(0x7A5D6C00),
    SCL_TAB(0x7AB1D400), SCL_TAB(0x7B064A80), SCL_TAB(0x7B5ACF80),
    SCL_TAB(0x7BAF6380), SCL_TAB(0x7C040580), SCL_TAB(0x7C58B600),
    SCL_TAB(0x7CAD7500), SCL_TAB(0x7D024200), SCL_TAB(0x7D571E00),
    SCL_TAB(0x7DAC0800), SCL_TAB(0x7E010080), SCL_TAB(0x7E560780),
    SCL_TAB(0x7EAB1C80), SCL_TAB(0x7F004000), SCL_TAB(0x7F557200),
    SCL_TAB(0x7FAAB200), SCL_TAB(0x7FFFFFFF)};

/**
 * \brief Table representing scale factor gains. Given a scale factor sf, and a
 * value pSpec[i] the gain is given by: MantissaTable[sf % 4][msb] = 2^(sf % 4)
 * / (1<<ExponentTable[sf % 4][msb] The second dimension "msb" represents the
 * upper scale factor bit count floor(log2(scalefactor >> 2)) The corresponding
 * exponents for the values in this tables are stored in ExponentTable[sf %
 * 4][msb] below.
 */
const FIXP_DBL MantissaTable[4][14] = {
    {0x40000000, 0x50A28C00, 0x6597FA80, 0x40000000, 0x50A28C00, 0x6597FA80,
     0x40000000, 0x50A28C00, 0x6597FA80, 0x40000000, 0x50A28C00, 0x6597FA80,
     0x40000000, 0x50A28C00},
    {0x4C1BF800, 0x5FE44380, 0x78D0DF80, 0x4C1BF800, 0x5FE44380, 0x78D0DF80,
     0x4C1BF800, 0x5FE44380, 0x78D0DF80, 0x4C1BF800, 0x5FE44380, 0x78D0DF80,
     0x4C1BF800, 0x5FE44380},
    {0x5A827980, 0x7208F800, 0x47D66B00, 0x5A827980, 0x7208F800, 0x47D66B00,
     0x5A827980, 0x7208F800, 0x47D66B00, 0x5A827980, 0x7208F800, 0x47D66B00,
     0x5A827980, 0x7208F800},
    {0x6BA27E80, 0x43CE3E80, 0x556E0400, 0x6BA27E80, 0x43CE3E80, 0x556E0400,
     0x6BA27E80, 0x43CE3E80, 0x556E0400, 0x6BA27E80, 0x43CE3E80, 0x556E0400,
     0x6BA27E80, 0x43CE3E80}};

const SCHAR ExponentTable[4][14] = {
    {1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18},
    {1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18},
    {1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 16, 17, 18},
    {1, 3, 4, 5, 7, 8, 9, 11, 12, 13, 15, 16, 17, 19}};

/* 41 scfbands */
static const SHORT sfb_96_1024[42] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,
    56,  64,  72,  80,  88,  96,  108, 120, 132, 144, 156, 172, 188, 212,
    240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024};
/* 12 scfbands */
static const SHORT sfb_96_128[13] = {0,  4,  8,  12, 16, 20, 24,
                                     32, 40, 48, 64, 92, 128};

/* 47 scfbands*/
static const SHORT sfb_64_1024[48] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,
    48,  52,  56,  64,  72,  80,  88,  100, 112, 124, 140, 156,
    172, 192, 216, 240, 268, 304, 344, 384, 424, 464, 504, 544,
    584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 984, 1024};

/* 12 scfbands */
static const SHORT sfb_64_128[13] = {0,  4,  8,  12, 16, 20, 24,
                                     32, 40, 48, 64, 92, 128};

/* 49 scfbands */
static const SHORT sfb_48_1024[50] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,
    64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176, 196, 216,
    240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608,
    640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 1024};
/* 14 scfbands */
static const SHORT sfb_48_128[15] = {0,  4,  8,  12, 16, 20,  28, 36,
                                     44, 56, 68, 80, 96, 112, 128};

/* 51 scfbands */
static const SHORT sfb_32_1024[52] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,
    64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176, 196, 216,
    240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608,
    640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

/* 47 scfbands */
static const SHORT sfb_24_1024[48] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,
    52,  60,  68,  76,  84,  92,  100, 108, 116, 124, 136, 148,
    160, 172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396,
    432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960, 1024};

/* 15 scfbands */
static const SHORT sfb_24_128[16] = {0,  4,  8,  12, 16, 20, 24,  28,
                                     36, 44, 52, 64, 76, 92, 108, 128};

/* 43 scfbands */
static const SHORT sfb_16_1024[44] = {
    0,   8,   16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  100, 112, 124,
    136, 148, 160, 172, 184, 196, 212, 228, 244, 260, 280, 300, 320, 344, 368,
    396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024};

/* 15 scfbands */
static const SHORT sfb_16_128[16] = {0,  4,  8,  12, 16, 20, 24,  28,
                                     32, 40, 48, 60, 72, 88, 108, 128};

/* 40 scfbands */
static const SHORT sfb_8_1024[41] = {
    0,   12,  24,  36,  48,  60,  72,  84,  96,  108, 120, 132, 144, 156,
    172, 188, 204, 220, 236, 252, 268, 288, 308, 328, 348, 372, 396, 420,
    448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024};

/* 15 scfbands */
static const SHORT sfb_8_128[16] = {0,  4,  8,  12, 16, 20, 24,  28,
                                    36, 44, 52, 60, 72, 88, 108, 128};

static const SHORT
    sfb_96_960[42] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,
                      44,  48,  52,  56,  64,  72,  80,  88,  96,  108, 120,
                      132, 144, 156, 172, 188, 212, 240, 276, 320, 384, 448,
                      512, 576, 640, 704, 768, 832, 896, 960}; /* 40 scfbands */

static const SHORT sfb_96_120[13] = {0,  4,  8,  12, 16, 20, 24,
                                     32, 40, 48, 64, 92, 120}; /* 12 scfbands */

static const SHORT sfb_64_960[47] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,
    48,  52,  56,  64,  72,  80,  88,  100, 112, 124, 140, 156,
    172, 192, 216, 240, 268, 304, 344, 384, 424, 464, 504, 544,
    584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 960}; /* 46 scfbands */

static const SHORT sfb_64_120[13] = {0,  4,  8,  12, 16, 20, 24,
                                     32, 40, 48, 64, 92, 120}; /* 12 scfbands */

static const SHORT sfb_48_960[50] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,
    64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176, 196, 216,
    240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608,
    640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960}; /* 49 scfbands */
static const SHORT sfb_48_120[15] = {
    0,  4,  8,  12, 16, 20,  28, 36,
    44, 56, 68, 80, 96, 112, 120}; /* 14 scfbands */

static const SHORT sfb_32_960[50] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,  56,
    64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176, 196, 216,
    240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608,
    640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960}; /* 49 scfbands */

static const SHORT sfb_24_960[47] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,
    52,  60,  68,  76,  84,  92,  100, 108, 116, 124, 136, 148,
    160, 172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396,
    432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960}; /* 46 scfbands */

static const SHORT sfb_24_120[16] = {
    0,  4,  8,  12, 16, 20, 24,  28,
    36, 44, 52, 64, 76, 92, 108, 120}; /* 15 scfbands */

static const SHORT sfb_16_960[43] = {0,   8,   16,  24,  32,  40,  48,  56,
                                     64,  72,  80,  88,  100, 112, 124, 136,
                                     148, 160, 172, 184, 196, 212, 228, 244,
                                     260, 280, 300, 320, 344, 368, 396, 424,
                                     456, 492, 532, 572, 616, 664, 716, 772,
                                     832, 896, 960}; /* 42 scfbands */

static const SHORT sfb_16_120[16] = {
    0,  4,  8,  12, 16, 20, 24,  28,
    32, 40, 48, 60, 72, 88, 108, 120}; /* 15 scfbands */

static const SHORT sfb_8_960[41] = {0,   12,  24,  36,  48,  60,  72,  84,  96,
                                    108, 120, 132, 144, 156, 172, 188, 204, 220,
                                    236, 252, 268, 288, 308, 328, 348, 372, 396,
                                    420, 448, 476, 508, 544, 580, 620, 664, 712,
                                    764, 820, 880, 944, 960}; /* 40 scfbands */

static const SHORT sfb_8_120[16] = {
    0,  4,  8,  12, 16, 20, 24,  28,
    36, 44, 52, 60, 72, 88, 108, 120}; /* 15 scfbands */

static const SHORT
    sfb_96_768[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,
                    40,  44,  48,  52,  56,  64,  72,  80,  88,  96,
                    108, 120, 132, 144, 156, 172, 188, 212, 240, 276,
                    320, 384, 448, 512, 576, 640, 704, 768}; /* 37 scfbands */
static const SHORT sfb_96_96[] = {0,  4,  8,  12, 16, 20, 24,
                                  32, 40, 48, 64, 92, 96}; /* 12 scfbands */

static const SHORT sfb_64_768[] =
    {
        0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,
        44,  48,  52,  56,  64,  72,  80,  88,  100, 112, 124,
        140, 156, 172, 192, 216, 240, 268, 304, 344, 384, 424,
        464, 504, 544, 584, 624, 664, 704, 744, 768}; /* 41 scfbands */

static const SHORT sfb_64_96[] = {0,  4,  8,  12, 16, 20, 24,
                                  32, 40, 48, 64, 92, 96}; /* 12 scfbands */

static const SHORT
    sfb_48_768[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,
                    56,  64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176,
                    196, 216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512,
                    544, 576, 608, 640, 672, 704, 736, 768}; /* 43 scfbands */

static const SHORT sfb_48_96[] = {0,  4,  8,  12, 16, 20, 28,
                                  36, 44, 56, 68, 80, 96}; /* 12 scfbands */

static const SHORT
    sfb_32_768[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  48,
                    56,  64,  72,  80,  88,  96,  108, 120, 132, 144, 160, 176,
                    196, 216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512,
                    544, 576, 608, 640, 672, 704, 736, 768}; /* 43 scfbands */

static const SHORT
    sfb_24_768[] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,
                    52,  60,  68,  76,  84,  92,  100, 108, 116, 124, 136, 148,
                    160, 172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396,
                    432, 468, 508, 552, 600, 652, 704, 768}; /* 43 scfbands */

static const SHORT sfb_24_96[] = {0,  4,  8,  12, 16, 20, 24, 28,
                                  36, 44, 52, 64, 76, 92, 96}; /* 14 scfbands */

static const SHORT sfb_16_768[] = {0,   8,   16,  24,  32,  40,  48,  56,  64,
                                   72,  80,  88,  100, 112, 124, 136, 148, 160,
                                   172, 184, 196, 212, 228, 244, 260, 280, 300,
                                   320, 344, 368, 396, 424, 456, 492, 532, 572,
                                   616, 664, 716, 768}; /* 39 scfbands */

static const SHORT sfb_16_96[] = {0,  4,  8,  12, 16, 20, 24, 28,
                                  32, 40, 48, 60, 72, 88, 96}; /* 14 scfbands */

static const SHORT
    sfb_8_768[] = {0,   12,  24,  36,  48,  60,  72,  84,  96,  108,
                   120, 132, 144, 156, 172, 188, 204, 220, 236, 252,
                   268, 288, 308, 328, 348, 372, 396, 420, 448, 476,
                   508, 544, 580, 620, 664, 712, 764, 768}; /* 37 scfbands */

static const SHORT sfb_8_96[] = {0,  4,  8,  12, 16, 20, 24, 28,
                                 36, 44, 52, 60, 72, 88, 96}; /* 14 scfbands */

static const SHORT sfb_48_512[37] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,
    52,  56,  60,  68,  76,  84,  92,  100, 112, 124, 136, 148, 164,
    184, 208, 236, 268, 300, 332, 364, 396, 428, 460, 512}; /* 36 scfbands */
static const SHORT
    sfb_32_512[38] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,
                      40,  44,  48,  52,  56,  64,  72,  80,  88,  96,
                      108, 120, 132, 144, 160, 176, 192, 212, 236, 260,
                      288, 320, 352, 384, 416, 448, 480, 512}; /* 37 scfbands */
static const SHORT sfb_24_512[32] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,
    44,  52,  60,  68,  80,  92,  104, 120, 140, 164, 192,
    224, 256, 288, 320, 352, 384, 416, 448, 480, 512}; /* 31 scfbands */

static const SHORT sfb_48_480[36] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,
    52,  56,  64,  72,  80,  88,  96,  108, 120, 132, 144, 156, 172,
    188, 212, 240, 272, 304, 336, 368, 400, 432, 480}; /* 35 scfbands */
static const SHORT
    sfb_32_480[38] = {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,
                      40,  44,  48,  52,  56,  60,  64,  72,  80,  88,
                      96,  104, 112, 124, 136, 148, 164, 180, 200, 224,
                      256, 288, 320, 352, 384, 416, 448, 480}; /* 37 scfbands */
static const SHORT sfb_24_480[31] =
    {0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,
     44,  52,  60,  68,  80,  92,  104, 120, 140, 164, 192,
     224, 256, 288, 320, 352, 384, 416, 448, 480}; /* 30 scfbands */

const SFB_INFO sfbOffsetTables[5][16] = {{
                                             {sfb_96_1024, sfb_96_128, 41, 12},
                                             {sfb_96_1024, sfb_96_128, 41, 12},
                                             {sfb_64_1024, sfb_64_128, 47, 12},
                                             {sfb_48_1024, sfb_48_128, 49, 14},
                                             {sfb_48_1024, sfb_48_128, 49, 14},
                                             {sfb_32_1024, sfb_48_128, 51, 14},
                                             {sfb_24_1024, sfb_24_128, 47, 15},
                                             {sfb_24_1024, sfb_24_128, 47, 15},
                                             {sfb_16_1024, sfb_16_128, 43, 15},
                                             {sfb_16_1024, sfb_16_128, 43, 15},
                                             {sfb_16_1024, sfb_16_128, 43, 15},
                                             {sfb_8_1024, sfb_8_128, 40, 15},
                                             {sfb_8_1024, sfb_8_128, 40, 15},
                                         },
                                         {
                                             {sfb_96_960, sfb_96_120, 40, 12},
                                             {sfb_96_960, sfb_96_120, 40, 12},
                                             {sfb_64_960, sfb_64_120, 46, 12},
                                             {sfb_48_960, sfb_48_120, 49, 14},
                                             {sfb_48_960, sfb_48_120, 49, 14},
                                             {sfb_32_960, sfb_48_120, 49, 14},
                                             {sfb_24_960, sfb_24_120, 46, 15},
                                             {sfb_24_960, sfb_24_120, 46, 15},
                                             {sfb_16_960, sfb_16_120, 42, 15},
                                             {sfb_16_960, sfb_16_120, 42, 15},
                                             {sfb_16_960, sfb_16_120, 42, 15},
                                             {sfb_8_960, sfb_8_120, 40, 15},
                                             {sfb_8_960, sfb_8_120, 40, 15},
                                         },
                                         {
                                             {sfb_96_768, sfb_96_96, 37, 12},
                                             {sfb_96_768, sfb_96_96, 37, 12},
                                             {sfb_64_768, sfb_64_96, 41, 12},
                                             {sfb_48_768, sfb_48_96, 43, 12},
                                             {sfb_48_768, sfb_48_96, 43, 12},
                                             {sfb_32_768, sfb_48_96, 43, 12},
                                             {sfb_24_768, sfb_24_96, 43, 14},
                                             {sfb_24_768, sfb_24_96, 43, 14},
                                             {sfb_16_768, sfb_16_96, 39, 14},
                                             {sfb_16_768, sfb_16_96, 39, 14},
                                             {sfb_16_768, sfb_16_96, 39, 14},
                                             {sfb_8_768, sfb_8_96, 37, 14},
                                             {sfb_8_768, sfb_8_96, 37, 14},
                                         },
                                         {
                                             {sfb_48_512, NULL, 36, 0},
                                             {sfb_48_512, NULL, 36, 0},
                                             {sfb_48_512, NULL, 36, 0},
                                             {sfb_48_512, NULL, 36, 0},
                                             {sfb_48_512, NULL, 36, 0},
                                             {sfb_32_512, NULL, 37, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                             {sfb_24_512, NULL, 31, 0},
                                         },
                                         {
                                             {sfb_48_480, NULL, 35, 0},
                                             {sfb_48_480, NULL, 35, 0},
                                             {sfb_48_480, NULL, 35, 0},
                                             {sfb_48_480, NULL, 35, 0},
                                             {sfb_48_480, NULL, 35, 0},
                                             {sfb_32_480, NULL, 37, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                             {sfb_24_480, NULL, 30, 0},
                                         }};

/*# don't use 1 bit hufman tables */
/*
  MPEG-2 AAC 2 BITS parallel Hufman Tables

  Bit 0:     = 1=ENDNODE, 0=INDEX
  Bit 1:     = CODEWORD LEN MOD 2
  Bit 2..9:  = VALUE/REF Tables 1..10,SCL
  Bit 2..11: = VALUE/REF Table 11
*/
const USHORT HuffmanCodeBook_1[51][4] = {
    {0x0157, 0x0157, 0x0004, 0x0018}, {0x0008, 0x000c, 0x0010, 0x0014},
    {0x015b, 0x015b, 0x0153, 0x0153}, {0x0057, 0x0057, 0x0167, 0x0167},
    {0x0257, 0x0257, 0x0117, 0x0117}, {0x0197, 0x0197, 0x0147, 0x0147},
    {0x001c, 0x0030, 0x0044, 0x0058}, {0x0020, 0x0024, 0x0028, 0x002c},
    {0x014b, 0x014b, 0x0163, 0x0163}, {0x0217, 0x0217, 0x0127, 0x0127},
    {0x0187, 0x0187, 0x0097, 0x0097}, {0x016b, 0x016b, 0x0017, 0x0017},
    {0x0034, 0x0038, 0x003c, 0x0040}, {0x0143, 0x0143, 0x0107, 0x0107},
    {0x011b, 0x011b, 0x0067, 0x0067}, {0x0193, 0x0193, 0x0297, 0x0297},
    {0x019b, 0x019b, 0x0247, 0x0247}, {0x0048, 0x004c, 0x0050, 0x0054},
    {0x01a7, 0x01a7, 0x0267, 0x0267}, {0x0113, 0x0113, 0x025b, 0x025b},
    {0x0053, 0x0053, 0x005b, 0x005b}, {0x0253, 0x0253, 0x0047, 0x0047},
    {0x005c, 0x0070, 0x0084, 0x0098}, {0x0060, 0x0064, 0x0068, 0x006c},
    {0x012b, 0x012b, 0x0123, 0x0123}, {0x018b, 0x018b, 0x00a7, 0x00a7},
    {0x0227, 0x0227, 0x0287, 0x0287}, {0x0087, 0x0087, 0x010b, 0x010b},
    {0x0074, 0x0078, 0x007c, 0x0080}, {0x021b, 0x021b, 0x0027, 0x0027},
    {0x01a3, 0x01a3, 0x0093, 0x0093}, {0x0183, 0x0183, 0x0207, 0x0207},
    {0x024b, 0x024b, 0x004b, 0x004b}, {0x0088, 0x008c, 0x0090, 0x0094},
    {0x0063, 0x0063, 0x0103, 0x0103}, {0x0007, 0x0007, 0x02a7, 0x02a7},
    {0x009b, 0x009b, 0x026b, 0x026b}, {0x0263, 0x0263, 0x01ab, 0x01ab},
    {0x009c, 0x00a0, 0x00a4, 0x00b8}, {0x0241, 0x0011, 0x0069, 0x0019},
    {0x0211, 0x0041, 0x0291, 0x0299}, {0x00a8, 0x00ac, 0x00b0, 0x00b4},
    {0x008b, 0x008b, 0x0223, 0x0223}, {0x00a3, 0x00a3, 0x020b, 0x020b},
    {0x02ab, 0x02ab, 0x0283, 0x0283}, {0x002b, 0x002b, 0x0083, 0x0083},
    {0x00bc, 0x00c0, 0x00c4, 0x00c8}, {0x0003, 0x0003, 0x022b, 0x022b},
    {0x028b, 0x028b, 0x02a3, 0x02a3}, {0x0023, 0x0023, 0x0203, 0x0203},
    {0x000b, 0x000b, 0x00ab, 0x00ab}};

const USHORT HuffmanCodeBook_2[39][4] = {
    {0x0004, 0x000c, 0x0020, 0x0034}, {0x0157, 0x0157, 0x0159, 0x0008},
    {0x0153, 0x0153, 0x0257, 0x0257}, {0x0010, 0x0014, 0x0018, 0x001c},
    {0x0117, 0x0117, 0x0057, 0x0057}, {0x0147, 0x0147, 0x0197, 0x0197},
    {0x0167, 0x0167, 0x0185, 0x0161}, {0x0125, 0x0095, 0x0065, 0x0215},
    {0x0024, 0x0028, 0x002c, 0x0030}, {0x0051, 0x0149, 0x0119, 0x0141},
    {0x0015, 0x0199, 0x0259, 0x0245}, {0x0191, 0x0265, 0x0105, 0x0251},
    {0x0045, 0x0111, 0x0169, 0x01a5}, {0x0038, 0x0044, 0x0058, 0x006c},
    {0x0295, 0x0059, 0x003c, 0x0040}, {0x0227, 0x0227, 0x021b, 0x021b},
    {0x0123, 0x0123, 0x0087, 0x0087}, {0x0048, 0x004c, 0x0050, 0x0054},
    {0x018b, 0x018b, 0x006b, 0x006b}, {0x029b, 0x029b, 0x01a3, 0x01a3},
    {0x0207, 0x0207, 0x01ab, 0x01ab}, {0x0093, 0x0093, 0x0103, 0x0103},
    {0x005c, 0x0060, 0x0064, 0x0068}, {0x0213, 0x0213, 0x010b, 0x010b},
    {0x012b, 0x012b, 0x0249, 0x0061}, {0x0181, 0x0291, 0x0241, 0x0041},
    {0x0005, 0x0099, 0x0019, 0x0025}, {0x0070, 0x0074, 0x0078, 0x0088},
    {0x02a5, 0x0261, 0x0011, 0x00a5}, {0x0049, 0x0285, 0x0269, 0x0089},
    {0x0221, 0x007c, 0x0080, 0x0084}, {0x020b, 0x020b, 0x0003, 0x0003},
    {0x00a3, 0x00a3, 0x02a3, 0x02a3}, {0x02ab, 0x02ab, 0x0083, 0x0083},
    {0x008c, 0x0090, 0x0094, 0x0098}, {0x028b, 0x028b, 0x0023, 0x0023},
    {0x0283, 0x0283, 0x002b, 0x002b}, {0x000b, 0x000b, 0x0203, 0x0203},
    {0x022b, 0x022b, 0x00ab, 0x00ab}};

const USHORT HuffmanCodeBook_3[39][4] = {
    {0x0003, 0x0003, 0x0004, 0x0008}, {0x0005, 0x0101, 0x0011, 0x0041},
    {0x000c, 0x0010, 0x0014, 0x0020}, {0x0017, 0x0017, 0x0143, 0x0143},
    {0x0051, 0x0111, 0x0045, 0x0151}, {0x0105, 0x0055, 0x0018, 0x001c},
    {0x0157, 0x0157, 0x0147, 0x0147}, {0x0117, 0x0117, 0x0009, 0x0201},
    {0x0024, 0x002c, 0x0040, 0x0054}, {0x0241, 0x0019, 0x0065, 0x0028},
    {0x0183, 0x0183, 0x0193, 0x0193}, {0x0030, 0x0034, 0x0038, 0x003c},
    {0x0027, 0x0027, 0x0253, 0x0253}, {0x005b, 0x005b, 0x0083, 0x0083},
    {0x0063, 0x0063, 0x0093, 0x0093}, {0x0023, 0x0023, 0x0213, 0x0213},
    {0x0044, 0x0048, 0x004c, 0x0050}, {0x004b, 0x004b, 0x0167, 0x0167},
    {0x0163, 0x0163, 0x0097, 0x0097}, {0x0197, 0x0197, 0x0125, 0x0085},
    {0x0185, 0x0121, 0x0159, 0x0255}, {0x0058, 0x005c, 0x0060, 0x0070},
    {0x0119, 0x0245, 0x0281, 0x0291}, {0x0069, 0x00a5, 0x0205, 0x0109},
    {0x01a1, 0x0064, 0x0068, 0x006c}, {0x002b, 0x002b, 0x01a7, 0x01a7},
    {0x0217, 0x0217, 0x014b, 0x014b}, {0x0297, 0x0297, 0x016b, 0x016b},
    {0x0074, 0x0078, 0x007c, 0x0080}, {0x00a3, 0x00a3, 0x0263, 0x0263},
    {0x0285, 0x0129, 0x0099, 0x00a9}, {0x02a1, 0x01a9, 0x0199, 0x0265},
    {0x02a5, 0x0084, 0x0088, 0x008c}, {0x0223, 0x0223, 0x008b, 0x008b},
    {0x0227, 0x0227, 0x0189, 0x0259}, {0x0219, 0x0090, 0x0094, 0x0098},
    {0x02ab, 0x02ab, 0x026b, 0x026b}, {0x029b, 0x029b, 0x024b, 0x024b},
    {0x020b, 0x020b, 0x0229, 0x0289}};

const USHORT HuffmanCodeBook_4[38][4] = {
    {0x0004, 0x0008, 0x000c, 0x0018}, {0x0155, 0x0151, 0x0115, 0x0055},
    {0x0145, 0x0005, 0x0015, 0x0001}, {0x0141, 0x0045, 0x0010, 0x0014},
    {0x0107, 0x0107, 0x0053, 0x0053}, {0x0103, 0x0103, 0x0113, 0x0113},
    {0x001c, 0x0020, 0x0034, 0x0048}, {0x0043, 0x0043, 0x0013, 0x0013},
    {0x0024, 0x0028, 0x002c, 0x0030}, {0x015b, 0x015b, 0x0197, 0x0197},
    {0x0167, 0x0167, 0x0257, 0x0257}, {0x005b, 0x005b, 0x011b, 0x011b},
    {0x0067, 0x0067, 0x014b, 0x014b}, {0x0038, 0x003c, 0x0040, 0x0044},
    {0x0193, 0x0193, 0x0251, 0x0095}, {0x0161, 0x0245, 0x0125, 0x0215},
    {0x0185, 0x0019, 0x0049, 0x0025}, {0x0109, 0x0211, 0x0061, 0x0241},
    {0x004c, 0x0050, 0x0058, 0x006c}, {0x0091, 0x0121, 0x0205, 0x0181},
    {0x0085, 0x0009, 0x0201, 0x0054}, {0x0023, 0x0023, 0x0083, 0x0083},
    {0x005c, 0x0060, 0x0064, 0x0068}, {0x01a7, 0x01a7, 0x016b, 0x016b},
    {0x019b, 0x019b, 0x0297, 0x0297}, {0x0267, 0x0267, 0x025b, 0x025b},
    {0x00a5, 0x0069, 0x0099, 0x01a1}, {0x0070, 0x0074, 0x0078, 0x0084},
    {0x0291, 0x0129, 0x0261, 0x0189}, {0x0285, 0x01a9, 0x0225, 0x0249},
    {0x0219, 0x02a5, 0x007c, 0x0080}, {0x029b, 0x029b, 0x026b, 0x026b},
    {0x00a3, 0x00a3, 0x002b, 0x002b}, {0x0088, 0x008c, 0x0090, 0x0094},
    {0x0283, 0x0283, 0x008b, 0x008b}, {0x0223, 0x0223, 0x020b, 0x020b},
    {0x02ab, 0x02ab, 0x02a3, 0x02a3}, {0x00ab, 0x00ab, 0x0229, 0x0289}};

const USHORT HuffmanCodeBook_5[41][4] = {
    {0x0113, 0x0113, 0x0004, 0x0008}, {0x010d, 0x0115, 0x0151, 0x00d1},
    {0x000c, 0x0010, 0x0014, 0x0028}, {0x00d7, 0x00d7, 0x014f, 0x014f},
    {0x00cf, 0x00cf, 0x0157, 0x0157}, {0x0018, 0x001c, 0x0020, 0x0024},
    {0x010b, 0x010b, 0x0193, 0x0193}, {0x011b, 0x011b, 0x0093, 0x0093},
    {0x00c9, 0x0159, 0x008d, 0x0195}, {0x0149, 0x00d9, 0x018d, 0x0095},
    {0x002c, 0x0030, 0x0044, 0x0058}, {0x0105, 0x011d, 0x0051, 0x01d1},
    {0x0034, 0x0038, 0x003c, 0x0040}, {0x00c7, 0x00c7, 0x01d7, 0x01d7},
    {0x015f, 0x015f, 0x004f, 0x004f}, {0x0147, 0x0147, 0x00df, 0x00df},
    {0x0057, 0x0057, 0x01cf, 0x01cf}, {0x0048, 0x004c, 0x0050, 0x0054},
    {0x018b, 0x018b, 0x019b, 0x019b}, {0x008b, 0x008b, 0x009b, 0x009b},
    {0x0085, 0x009d, 0x01c9, 0x0059}, {0x019d, 0x01d9, 0x0185, 0x0049},
    {0x005c, 0x0060, 0x0074, 0x0088}, {0x0011, 0x0101, 0x0161, 0x0121},
    {0x0064, 0x0068, 0x006c, 0x0070}, {0x00c3, 0x00c3, 0x0213, 0x0213},
    {0x00e3, 0x00e3, 0x000f, 0x000f}, {0x0217, 0x0217, 0x020f, 0x020f},
    {0x0143, 0x0143, 0x0017, 0x0017}, {0x0078, 0x007c, 0x0080, 0x0084},
    {0x005f, 0x005f, 0x0047, 0x0047}, {0x01c7, 0x01c7, 0x020b, 0x020b},
    {0x0083, 0x0083, 0x01a3, 0x01a3}, {0x001b, 0x001b, 0x021b, 0x021b},
    {0x008c, 0x0090, 0x0094, 0x0098}, {0x01df, 0x01df, 0x0183, 0x0183},
    {0x0009, 0x00a1, 0x001d, 0x0041}, {0x01c1, 0x021d, 0x0205, 0x01e1},
    {0x0061, 0x0005, 0x009c, 0x00a0}, {0x0023, 0x0023, 0x0203, 0x0203},
    {0x0223, 0x0223, 0x0003, 0x0003}};

const USHORT HuffmanCodeBook_6[40][4] = {
    {0x0004, 0x0008, 0x000c, 0x001c}, {0x0111, 0x0115, 0x00d1, 0x0151},
    {0x010d, 0x0155, 0x014d, 0x00d5}, {0x00cd, 0x0010, 0x0014, 0x0018},
    {0x00d9, 0x0159, 0x0149, 0x00c9}, {0x0109, 0x018d, 0x0119, 0x0095},
    {0x0195, 0x0091, 0x008d, 0x0191}, {0x0020, 0x0024, 0x0038, 0x004c},
    {0x0099, 0x0189, 0x0089, 0x0199}, {0x0028, 0x002c, 0x0030, 0x0034},
    {0x0147, 0x0147, 0x015f, 0x015f}, {0x00df, 0x00df, 0x01cf, 0x01cf},
    {0x00c7, 0x00c7, 0x01d7, 0x01d7}, {0x0057, 0x0057, 0x004f, 0x004f},
    {0x003c, 0x0040, 0x0044, 0x0048}, {0x011f, 0x011f, 0x0107, 0x0107},
    {0x0053, 0x0053, 0x01d3, 0x01d3}, {0x019f, 0x019f, 0x0085, 0x01c9},
    {0x01d9, 0x009d, 0x0059, 0x0049}, {0x0050, 0x005c, 0x0070, 0x0084},
    {0x0185, 0x01dd, 0x0054, 0x0058}, {0x005f, 0x005f, 0x0047, 0x0047},
    {0x01c7, 0x01c7, 0x0017, 0x0017}, {0x0060, 0x0064, 0x0068, 0x006c},
    {0x000f, 0x000f, 0x0163, 0x0163}, {0x0143, 0x0143, 0x00c3, 0x00c3},
    {0x0217, 0x0217, 0x00e3, 0x00e3}, {0x020f, 0x020f, 0x0013, 0x0013},
    {0x0074, 0x0078, 0x007c, 0x0080}, {0x0183, 0x0183, 0x0083, 0x0083},
    {0x021b, 0x021b, 0x000b, 0x000b}, {0x0103, 0x0103, 0x01a3, 0x01a3},
    {0x00a3, 0x00a3, 0x020b, 0x020b}, {0x0088, 0x008c, 0x0090, 0x0094},
    {0x0123, 0x0123, 0x001b, 0x001b}, {0x0213, 0x0213, 0x0005, 0x0205},
    {0x001d, 0x0061, 0x021d, 0x01e1}, {0x01c1, 0x0041, 0x0098, 0x009c},
    {0x0223, 0x0223, 0x0203, 0x0203}, {0x0003, 0x0003, 0x0023, 0x0023}};

const USHORT HuffmanCodeBook_7[31][4] = {
    {0x0003, 0x0003, 0x0004, 0x0008}, {0x0007, 0x0007, 0x0043, 0x0043},
    {0x0045, 0x000c, 0x0010, 0x0024}, {0x0049, 0x0085, 0x0009, 0x0081},
    {0x0014, 0x0018, 0x001c, 0x0020}, {0x004f, 0x004f, 0x00c7, 0x00c7},
    {0x008b, 0x008b, 0x000f, 0x000f}, {0x00c3, 0x00c3, 0x00c9, 0x008d},
    {0x0105, 0x0051, 0x0145, 0x0055}, {0x0028, 0x002c, 0x0040, 0x0054},
    {0x00cd, 0x0109, 0x0101, 0x0011}, {0x0030, 0x0034, 0x0038, 0x003c},
    {0x0093, 0x0093, 0x014b, 0x014b}, {0x0097, 0x0097, 0x0143, 0x0143},
    {0x005b, 0x005b, 0x0017, 0x0017}, {0x0187, 0x0187, 0x00d3, 0x00d3},
    {0x0044, 0x0048, 0x004c, 0x0050}, {0x014f, 0x014f, 0x010f, 0x010f},
    {0x00d7, 0x00d7, 0x018b, 0x018b}, {0x009b, 0x009b, 0x01c7, 0x01c7},
    {0x018d, 0x0181, 0x0019, 0x0111}, {0x0058, 0x005c, 0x0060, 0x0068},
    {0x005d, 0x0151, 0x009d, 0x0115}, {0x00d9, 0x01c9, 0x00dd, 0x0119},
    {0x0155, 0x0191, 0x01cd, 0x0064}, {0x001f, 0x001f, 0x01c3, 0x01c3},
    {0x006c, 0x0070, 0x0074, 0x0078}, {0x015b, 0x015b, 0x0197, 0x0197},
    {0x011f, 0x011f, 0x01d3, 0x01d3}, {0x01d7, 0x01d7, 0x015f, 0x015f},
    {0x019d, 0x0199, 0x01d9, 0x01dd}};

const USHORT HuffmanCodeBook_8[31][4] = {
    {0x0004, 0x0008, 0x0010, 0x0024}, {0x0047, 0x0047, 0x0049, 0x0005},
    {0x0085, 0x0041, 0x0089, 0x000c}, {0x0003, 0x0003, 0x000b, 0x000b},
    {0x0014, 0x0018, 0x001c, 0x0020}, {0x0083, 0x0083, 0x004f, 0x004f},
    {0x00c7, 0x00c7, 0x008f, 0x008f}, {0x00cb, 0x00cb, 0x00cd, 0x0051},
    {0x0105, 0x0091, 0x0109, 0x000d}, {0x0028, 0x002c, 0x0040, 0x0054},
    {0x00c1, 0x00d1, 0x010d, 0x0095}, {0x0030, 0x0034, 0x0038, 0x003c},
    {0x0057, 0x0057, 0x014b, 0x014b}, {0x0147, 0x0147, 0x00d7, 0x00d7},
    {0x014f, 0x014f, 0x0113, 0x0113}, {0x0117, 0x0117, 0x0103, 0x0103},
    {0x0044, 0x0048, 0x004c, 0x0050}, {0x0153, 0x0153, 0x0013, 0x0013},
    {0x018b, 0x018b, 0x009b, 0x009b}, {0x005b, 0x005b, 0x0187, 0x0187},
    {0x018d, 0x00d9, 0x0155, 0x0015}, {0x0058, 0x005c, 0x0060, 0x0068},
    {0x0119, 0x0141, 0x0191, 0x005d}, {0x009d, 0x01c9, 0x0159, 0x00dd},
    {0x01c5, 0x0195, 0x01cd, 0x0064}, {0x019b, 0x019b, 0x011f, 0x011f},
    {0x006c, 0x0070, 0x0074, 0x0078}, {0x001b, 0x001b, 0x01d3, 0x01d3},
    {0x0183, 0x0183, 0x015f, 0x015f}, {0x019f, 0x019f, 0x01db, 0x01db},
    {0x01d5, 0x001d, 0x01c1, 0x01dd}};

const USHORT HuffmanCodeBook_9[84][4] = {
    {0x0003, 0x0003, 0x0004, 0x0008}, {0x0007, 0x0007, 0x0043, 0x0043},
    {0x0045, 0x000c, 0x0010, 0x002c}, {0x0049, 0x0085, 0x0009, 0x0081},
    {0x0014, 0x0018, 0x001c, 0x0020}, {0x004f, 0x004f, 0x008b, 0x008b},
    {0x00c7, 0x00c7, 0x000d, 0x00c1}, {0x00c9, 0x008d, 0x0105, 0x0051},
    {0x0109, 0x0145, 0x0024, 0x0028}, {0x0093, 0x0093, 0x00cf, 0x00cf},
    {0x0103, 0x0103, 0x0013, 0x0013}, {0x0030, 0x0044, 0x0058, 0x00a4},
    {0x0034, 0x0038, 0x003c, 0x0040}, {0x0057, 0x0057, 0x014b, 0x014b},
    {0x0187, 0x0187, 0x010f, 0x010f}, {0x0097, 0x0097, 0x005b, 0x005b},
    {0x00d3, 0x00d3, 0x0141, 0x0189}, {0x0048, 0x004c, 0x0050, 0x0054},
    {0x0015, 0x01c5, 0x014d, 0x0205}, {0x0061, 0x0111, 0x00d5, 0x0099},
    {0x005d, 0x0181, 0x00a1, 0x0209}, {0x018d, 0x01c9, 0x0151, 0x0065},
    {0x005c, 0x0068, 0x007c, 0x0090}, {0x0245, 0x009d, 0x0060, 0x0064},
    {0x001b, 0x001b, 0x0117, 0x0117}, {0x00db, 0x00db, 0x00e3, 0x00e3},
    {0x006c, 0x0070, 0x0074, 0x0078}, {0x01c3, 0x01c3, 0x00a7, 0x00a7},
    {0x020f, 0x020f, 0x0193, 0x0193}, {0x01cf, 0x01cf, 0x0203, 0x0203},
    {0x006b, 0x006b, 0x011b, 0x011b}, {0x0080, 0x0084, 0x0088, 0x008c},
    {0x024b, 0x024b, 0x0157, 0x0157}, {0x0023, 0x0023, 0x001f, 0x001f},
    {0x00df, 0x00df, 0x00ab, 0x00ab}, {0x00e7, 0x00e7, 0x0123, 0x0123},
    {0x0094, 0x0098, 0x009c, 0x00a0}, {0x0287, 0x0287, 0x011f, 0x011f},
    {0x015b, 0x015b, 0x0197, 0x0197}, {0x0213, 0x0213, 0x01d3, 0x01d3},
    {0x024f, 0x024f, 0x006f, 0x006f}, {0x00a8, 0x00bc, 0x00d0, 0x00f4},
    {0x00ac, 0x00b0, 0x00b4, 0x00b8}, {0x0217, 0x0217, 0x0027, 0x0027},
    {0x0163, 0x0163, 0x00e9, 0x0289}, {0x0241, 0x00ad, 0x0125, 0x0199},
    {0x0071, 0x0251, 0x01a1, 0x02c5}, {0x00c0, 0x00c4, 0x00c8, 0x00cc},
    {0x0165, 0x0129, 0x01d5, 0x015d}, {0x02c9, 0x0305, 0x00b1, 0x00ed},
    {0x028d, 0x0255, 0x01d9, 0x01e1}, {0x012d, 0x0281, 0x019d, 0x00f1},
    {0x00d4, 0x00d8, 0x00dc, 0x00e0}, {0x0029, 0x0169, 0x0291, 0x0219},
    {0x0309, 0x01a5, 0x01e5, 0x02d1}, {0x002d, 0x0259, 0x02cd, 0x0295},
    {0x00e4, 0x00e8, 0x00ec, 0x00f0}, {0x0223, 0x0223, 0x021f, 0x021f},
    {0x0173, 0x0173, 0x030f, 0x030f}, {0x016f, 0x016f, 0x01df, 0x01df},
    {0x0133, 0x0133, 0x01af, 0x01af}, {0x00f8, 0x010c, 0x0120, 0x0134},
    {0x00fc, 0x0100, 0x0104, 0x0108}, {0x01ab, 0x01ab, 0x0313, 0x0313},
    {0x025f, 0x025f, 0x02d7, 0x02d7}, {0x02c3, 0x02c3, 0x01b3, 0x01b3},
    {0x029b, 0x029b, 0x0033, 0x0033}, {0x0110, 0x0114, 0x0118, 0x011c},
    {0x01eb, 0x01eb, 0x0317, 0x0317}, {0x029f, 0x029f, 0x0227, 0x0227},
    {0x0303, 0x0303, 0x01ef, 0x01ef}, {0x0263, 0x0263, 0x0267, 0x0267},
    {0x0124, 0x0128, 0x012c, 0x0130}, {0x022b, 0x022b, 0x02df, 0x02df},
    {0x01f3, 0x01f3, 0x02db, 0x02db}, {0x02e3, 0x02e3, 0x022f, 0x022f},
    {0x031f, 0x031f, 0x031b, 0x031b}, {0x0138, 0x013c, 0x0140, 0x0144},
    {0x02a1, 0x0269, 0x0321, 0x02a5}, {0x02e5, 0x0325, 0x02e9, 0x0271},
    {0x02a9, 0x026d, 0x0231, 0x02ad}, {0x02b1, 0x02f1, 0x0148, 0x014c},
    {0x032b, 0x032b, 0x02ef, 0x02ef}, {0x032f, 0x032f, 0x0333, 0x0333}};

const USHORT HuffmanCodeBook_10[82][4] = {
    {0x0004, 0x000c, 0x0020, 0x004c}, {0x0045, 0x0085, 0x0049, 0x0008},
    {0x008b, 0x008b, 0x0007, 0x0007}, {0x0010, 0x0014, 0x0018, 0x001c},
    {0x0043, 0x0043, 0x00c7, 0x00c7}, {0x008f, 0x008f, 0x004f, 0x004f},
    {0x00cb, 0x00cb, 0x00cf, 0x00cf}, {0x0009, 0x0081, 0x0109, 0x0091},
    {0x0024, 0x0028, 0x002c, 0x0038}, {0x0105, 0x0051, 0x0001, 0x00d1},
    {0x010d, 0x000d, 0x00c1, 0x0111}, {0x0149, 0x0095, 0x0030, 0x0034},
    {0x0147, 0x0147, 0x0057, 0x0057}, {0x00d7, 0x00d7, 0x014f, 0x014f},
    {0x003c, 0x0040, 0x0044, 0x0048}, {0x0117, 0x0117, 0x0153, 0x0153},
    {0x009b, 0x009b, 0x018b, 0x018b}, {0x00db, 0x00db, 0x0013, 0x0013},
    {0x005b, 0x005b, 0x0103, 0x0103}, {0x0050, 0x0064, 0x0078, 0x00c0},
    {0x0054, 0x0058, 0x005c, 0x0060}, {0x0187, 0x0187, 0x018f, 0x018f},
    {0x0157, 0x0157, 0x011b, 0x011b}, {0x0193, 0x0193, 0x0159, 0x009d},
    {0x01cd, 0x01c9, 0x0195, 0x00a1}, {0x0068, 0x006c, 0x0070, 0x0074},
    {0x00dd, 0x0015, 0x005d, 0x0141}, {0x0061, 0x01c5, 0x00e1, 0x011d},
    {0x01d1, 0x0209, 0x0199, 0x015d}, {0x0205, 0x020d, 0x0121, 0x0211},
    {0x007c, 0x0084, 0x0098, 0x00ac}, {0x01d5, 0x0161, 0x0215, 0x0080},
    {0x019f, 0x019f, 0x01db, 0x01db}, {0x0088, 0x008c, 0x0090, 0x0094},
    {0x00a7, 0x00a7, 0x001b, 0x001b}, {0x021b, 0x021b, 0x00e7, 0x00e7},
    {0x024f, 0x024f, 0x0067, 0x0067}, {0x024b, 0x024b, 0x0183, 0x0183},
    {0x009c, 0x00a0, 0x00a4, 0x00a8}, {0x01a3, 0x01a3, 0x0127, 0x0127},
    {0x0253, 0x0253, 0x00ab, 0x00ab}, {0x0247, 0x0247, 0x01df, 0x01df},
    {0x01e3, 0x01e3, 0x0167, 0x0167}, {0x00b0, 0x00b4, 0x00b8, 0x00bc},
    {0x021f, 0x021f, 0x00eb, 0x00eb}, {0x0257, 0x0257, 0x012b, 0x012b},
    {0x028b, 0x028b, 0x006b, 0x006b}, {0x028f, 0x028f, 0x01a7, 0x01a7},
    {0x00c4, 0x00d8, 0x00ec, 0x0100}, {0x00c8, 0x00cc, 0x00d0, 0x00d4},
    {0x025b, 0x025b, 0x0023, 0x0023}, {0x0293, 0x0293, 0x001f, 0x001f},
    {0x00af, 0x00af, 0x025d, 0x00ed}, {0x01a9, 0x0285, 0x006d, 0x01e5},
    {0x00dc, 0x00e0, 0x00e4, 0x00e8}, {0x01c1, 0x0221, 0x0169, 0x02cd},
    {0x0295, 0x0261, 0x016d, 0x0201}, {0x012d, 0x02c9, 0x029d, 0x0299},
    {0x01e9, 0x02d1, 0x02c5, 0x00b1}, {0x00f0, 0x00f4, 0x00f8, 0x00fc},
    {0x0225, 0x00f1, 0x01ad, 0x02d5}, {0x0131, 0x01ed, 0x0171, 0x030d},
    {0x02d9, 0x0025, 0x0229, 0x0029}, {0x0071, 0x0241, 0x0311, 0x0265},
    {0x0104, 0x010c, 0x0120, 0x0134}, {0x01b1, 0x0309, 0x02a1, 0x0108},
    {0x02a7, 0x02a7, 0x0307, 0x0307}, {0x0110, 0x0114, 0x0118, 0x011c},
    {0x022f, 0x022f, 0x01f3, 0x01f3}, {0x02df, 0x02df, 0x0317, 0x0317},
    {0x031b, 0x031b, 0x026b, 0x026b}, {0x02e3, 0x02e3, 0x0233, 0x0233},
    {0x0124, 0x0128, 0x012c, 0x0130}, {0x0283, 0x0283, 0x031f, 0x031f},
    {0x002f, 0x002f, 0x02ab, 0x02ab}, {0x026f, 0x026f, 0x02af, 0x02af},
    {0x02c3, 0x02c3, 0x02ef, 0x02ef}, {0x0138, 0x013c, 0x0140, 0x0144},
    {0x02e7, 0x02e7, 0x02eb, 0x02eb}, {0x0033, 0x0033, 0x0323, 0x0323},
    {0x0271, 0x0329, 0x0325, 0x032d}, {0x02f1, 0x0301, 0x02b1, 0x0331}};

const USHORT HuffmanCodeBook_11[152][4] = {
    {0x0004, 0x0010, 0x0038, 0x008c}, {0x0001, 0x0085, 0x0008, 0x000c},
    {0x0843, 0x0843, 0x0007, 0x0007}, {0x0083, 0x0083, 0x008b, 0x008b},
    {0x0014, 0x0018, 0x001c, 0x0024}, {0x0107, 0x0107, 0x010b, 0x010b},
    {0x0185, 0x008d, 0x010d, 0x0009}, {0x0189, 0x0101, 0x018d, 0x0020},
    {0x0093, 0x0093, 0x0207, 0x0207}, {0x0028, 0x002c, 0x0030, 0x0034},
    {0x0113, 0x0113, 0x020b, 0x020b}, {0x0193, 0x0193, 0x020f, 0x020f},
    {0x000f, 0x000f, 0x0183, 0x0183}, {0x0097, 0x0097, 0x0117, 0x0117},
    {0x003c, 0x0050, 0x0064, 0x0078}, {0x0040, 0x0044, 0x0048, 0x004c},
    {0x028b, 0x028b, 0x0213, 0x0213}, {0x0287, 0x0287, 0x0197, 0x0197},
    {0x028f, 0x028f, 0x0217, 0x0217}, {0x0291, 0x0119, 0x0309, 0x0099},
    {0x0054, 0x0058, 0x005c, 0x0060}, {0x0199, 0x030d, 0x0305, 0x0811},
    {0x080d, 0x02c1, 0x01c1, 0x0241}, {0x0219, 0x0341, 0x0011, 0x0311},
    {0x0201, 0x0809, 0x0295, 0x0815}, {0x0068, 0x006c, 0x0070, 0x0074},
    {0x03c1, 0x0141, 0x0441, 0x0389}, {0x011d, 0x038d, 0x0299, 0x0315},
    {0x0819, 0x0541, 0x019d, 0x009d}, {0x04c1, 0x081d, 0x0805, 0x0385},
    {0x007c, 0x0080, 0x0084, 0x0088}, {0x0391, 0x05c1, 0x021d, 0x0641},
    {0x0821, 0x00c1, 0x0319, 0x0825}, {0x0409, 0x0395, 0x0829, 0x06c1},
    {0x01a1, 0x0121, 0x040d, 0x0015}, {0x0090, 0x00c8, 0x011c, 0x0170},
    {0x0094, 0x0098, 0x00a0, 0x00b4}, {0x0741, 0x082d, 0x029d, 0x0411},
    {0x0399, 0x031d, 0x0281, 0x009c}, {0x0223, 0x0223, 0x07c3, 0x07c3},
    {0x00a4, 0x00a8, 0x00ac, 0x00b0}, {0x0833, 0x0833, 0x0407, 0x0407},
    {0x00a3, 0x00a3, 0x083b, 0x083b}, {0x0417, 0x0417, 0x0837, 0x0837},
    {0x048f, 0x048f, 0x02a3, 0x02a3}, {0x00b8, 0x00bc, 0x00c0, 0x00c4},
    {0x039f, 0x039f, 0x048b, 0x048b}, {0x0323, 0x0323, 0x0127, 0x0127},
    {0x01a7, 0x01a7, 0x083f, 0x083f}, {0x0493, 0x0493, 0x041b, 0x041b},
    {0x00cc, 0x00e0, 0x00f4, 0x0108}, {0x00d0, 0x00d4, 0x00d8, 0x00dc},
    {0x001b, 0x001b, 0x0227, 0x0227}, {0x0497, 0x0497, 0x03a3, 0x03a3},
    {0x041f, 0x041f, 0x0487, 0x0487}, {0x01ab, 0x01ab, 0x0303, 0x0303},
    {0x00e4, 0x00e8, 0x00ec, 0x00f0}, {0x012b, 0x012b, 0x00a7, 0x00a7},
    {0x02a7, 0x02a7, 0x0513, 0x0513}, {0x050b, 0x050b, 0x0327, 0x0327},
    {0x050f, 0x050f, 0x049b, 0x049b}, {0x00f8, 0x00fc, 0x0100, 0x0104},
    {0x022b, 0x022b, 0x0423, 0x0423}, {0x02ab, 0x02ab, 0x03a7, 0x03a7},
    {0x01af, 0x01af, 0x0507, 0x0507}, {0x001f, 0x001f, 0x032b, 0x032b},
    {0x010c, 0x0110, 0x0114, 0x0118}, {0x049f, 0x049f, 0x058f, 0x058f},
    {0x0517, 0x0517, 0x00ab, 0x00ab}, {0x0593, 0x0593, 0x012f, 0x012f},
    {0x0137, 0x0137, 0x051b, 0x051b}, {0x0120, 0x0134, 0x0148, 0x015c},
    {0x0124, 0x0128, 0x012c, 0x0130}, {0x01b7, 0x01b7, 0x058b, 0x058b},
    {0x0043, 0x0043, 0x0597, 0x0597}, {0x02af, 0x02af, 0x022d, 0x0425},
    {0x051d, 0x04a1, 0x0801, 0x0691}, {0x0138, 0x013c, 0x0140, 0x0144},
    {0x0381, 0x068d, 0x032d, 0x00b5}, {0x0235, 0x01b1, 0x0689, 0x02b5},
    {0x0521, 0x0599, 0x0429, 0x03a9}, {0x0139, 0x0231, 0x0585, 0x0611},
    {0x014c, 0x0150, 0x0154, 0x0158}, {0x00ad, 0x060d, 0x0685, 0x0131},
    {0x059d, 0x070d, 0x0615, 0x0695}, {0x0239, 0x0711, 0x03ad, 0x01b9},
    {0x02b1, 0x0335, 0x0331, 0x0021}, {0x0160, 0x0164, 0x0168, 0x016c},
    {0x042d, 0x0609, 0x04a5, 0x02b9}, {0x0699, 0x0529, 0x013d, 0x05a1},
    {0x0525, 0x0339, 0x04a9, 0x0715}, {0x04ad, 0x00b9, 0x0709, 0x0619},
    {0x0174, 0x0188, 0x019c, 0x01cc}, {0x0178, 0x017c, 0x0180, 0x0184},
    {0x0605, 0x0435, 0x0401, 0x03b5}, {0x061d, 0x03b1, 0x069d, 0x01bd},
    {0x00b1, 0x0719, 0x0789, 0x02bd}, {0x023d, 0x0705, 0x05a5, 0x0791},
    {0x018c, 0x0190, 0x0194, 0x0198}, {0x03b9, 0x06a1, 0x04b5, 0x0621},
    {0x0795, 0x078d, 0x05a9, 0x052d}, {0x0431, 0x033d, 0x03bd, 0x0721},
    {0x00bd, 0x071d, 0x0025, 0x0481}, {0x01a0, 0x01a4, 0x01a8, 0x01b8},
    {0x06a5, 0x0625, 0x04b1, 0x0439}, {0x06a9, 0x04b9, 0x0531, 0x0799},
    {0x079d, 0x01ac, 0x01b0, 0x01b4}, {0x0727, 0x0727, 0x043f, 0x043f},
    {0x05af, 0x05af, 0x072f, 0x072f}, {0x0787, 0x0787, 0x062b, 0x062b},
    {0x01bc, 0x01c0, 0x01c4, 0x01c8}, {0x072b, 0x072b, 0x05b7, 0x05b7},
    {0x0537, 0x0537, 0x06af, 0x06af}, {0x062f, 0x062f, 0x07a3, 0x07a3},
    {0x05bb, 0x05bb, 0x0637, 0x0637}, {0x01d0, 0x01e4, 0x01f8, 0x020c},
    {0x01d4, 0x01d8, 0x01dc, 0x01e0}, {0x06b3, 0x06b3, 0x04bf, 0x04bf},
    {0x053b, 0x053b, 0x002b, 0x002b}, {0x05b3, 0x05b3, 0x07a7, 0x07a7},
    {0x0503, 0x0503, 0x0633, 0x0633}, {0x01e8, 0x01ec, 0x01f0, 0x01f4},
    {0x002f, 0x002f, 0x0733, 0x0733}, {0x07ab, 0x07ab, 0x06b7, 0x06b7},
    {0x0683, 0x0683, 0x063b, 0x063b}, {0x053f, 0x053f, 0x05bf, 0x05bf},
    {0x01fc, 0x0200, 0x0204, 0x0208}, {0x07af, 0x07af, 0x06bb, 0x06bb},
    {0x0037, 0x0037, 0x0583, 0x0583}, {0x0737, 0x0737, 0x063f, 0x063f},
    {0x06bf, 0x06bf, 0x07b3, 0x07b3}, {0x0210, 0x0214, 0x0218, 0x021c},
    {0x003b, 0x003b, 0x073b, 0x073b}, {0x07b7, 0x07b7, 0x0033, 0x0033},
    {0x07bb, 0x07bb, 0x0701, 0x0601}, {0x073d, 0x003d, 0x0781, 0x07bd},
    {0x0118, 0x0117, 0x0100, 0x0109}, {0x05a5, 0x05a1, 0x05b7, 0x0513},
    {0x08f9, 0x08ff, 0x0821, 0x08ff}, {0x084f, 0x08ff, 0x08bc, 0x08ff},
    {0x0815, 0x08ff, 0x0837, 0x08ff}, {0x080d, 0x08ff, 0x085f, 0x08ff},
    {0x084a, 0x08ff, 0x087d, 0x08ff}, {0x08ff, 0x08ff, 0x08a8, 0x08ff},
    {0x0815, 0x08ff, 0x083f, 0x08ff}, {0x0830, 0x08ff, 0x0894, 0x08ff},
    {0x08d4, 0x08ff, 0x0825, 0x08ff}, {0x08ef, 0x08ff, 0x083f, 0x08ff},
    {0x0809, 0x08ff, 0x08fc, 0x08ff}, {0x0842, 0x08ff, 0x08b3, 0x08ff},
    {0x070d, 0x07a9, 0x060e, 0x06e2}, {0x06c7, 0x06d0, 0x04b2, 0x0407}};

const USHORT HuffmanCodeBook_SCL[65][4] = {
    {0x00f3, 0x00f3, 0x0004, 0x0008}, {0x00ef, 0x00ef, 0x00f5, 0x00e9},
    {0x00f9, 0x000c, 0x0010, 0x0014}, {0x00e7, 0x00e7, 0x00ff, 0x00ff},
    {0x00e1, 0x0101, 0x00dd, 0x0105}, {0x0018, 0x001c, 0x0020, 0x0028},
    {0x010b, 0x010b, 0x00db, 0x00db}, {0x010f, 0x010f, 0x00d5, 0x0111},
    {0x00d1, 0x0115, 0x00cd, 0x0024}, {0x011b, 0x011b, 0x00cb, 0x00cb},
    {0x002c, 0x0030, 0x0034, 0x0040}, {0x00c7, 0x00c7, 0x011f, 0x011f},
    {0x0121, 0x00c1, 0x0125, 0x00bd}, {0x0129, 0x00b9, 0x0038, 0x003c},
    {0x0133, 0x0133, 0x012f, 0x012f}, {0x0137, 0x0137, 0x013b, 0x013b},
    {0x0044, 0x0048, 0x004c, 0x0058}, {0x00b7, 0x00b7, 0x00af, 0x00af},
    {0x00b1, 0x013d, 0x00a9, 0x00a5}, {0x0141, 0x00a1, 0x0050, 0x0054},
    {0x0147, 0x0147, 0x009f, 0x009f}, {0x014b, 0x014b, 0x009b, 0x009b},
    {0x005c, 0x0060, 0x0064, 0x0070}, {0x014f, 0x014f, 0x0095, 0x008d},
    {0x0155, 0x0085, 0x0091, 0x0089}, {0x0151, 0x0081, 0x0068, 0x006c},
    {0x015f, 0x015f, 0x0167, 0x0167}, {0x007b, 0x007b, 0x007f, 0x007f},
    {0x0074, 0x0078, 0x0080, 0x00b0}, {0x0159, 0x0075, 0x0069, 0x006d},
    {0x0071, 0x0061, 0x0161, 0x007c}, {0x0067, 0x0067, 0x005b, 0x005b},
    {0x0084, 0x0088, 0x008c, 0x009c}, {0x005f, 0x005f, 0x0169, 0x0055},
    {0x004d, 0x000d, 0x0005, 0x0009}, {0x0001, 0x0090, 0x0094, 0x0098},
    {0x018b, 0x018b, 0x018f, 0x018f}, {0x0193, 0x0193, 0x0197, 0x0197},
    {0x019b, 0x019b, 0x01d7, 0x01d7}, {0x00a0, 0x00a4, 0x00a8, 0x00ac},
    {0x0187, 0x0187, 0x016f, 0x016f}, {0x0173, 0x0173, 0x0177, 0x0177},
    {0x017b, 0x017b, 0x017f, 0x017f}, {0x0183, 0x0183, 0x01a3, 0x01a3},
    {0x00b4, 0x00c8, 0x00dc, 0x00f0}, {0x00b8, 0x00bc, 0x00c0, 0x00c4},
    {0x01bf, 0x01bf, 0x01c3, 0x01c3}, {0x01c7, 0x01c7, 0x01cb, 0x01cb},
    {0x01cf, 0x01cf, 0x01d3, 0x01d3}, {0x01bb, 0x01bb, 0x01a7, 0x01a7},
    {0x00cc, 0x00d0, 0x00d4, 0x00d8}, {0x01ab, 0x01ab, 0x01af, 0x01af},
    {0x01b3, 0x01b3, 0x01b7, 0x01b7}, {0x01db, 0x01db, 0x001b, 0x001b},
    {0x0023, 0x0023, 0x0027, 0x0027}, {0x00e0, 0x00e4, 0x00e8, 0x00ec},
    {0x002b, 0x002b, 0x0017, 0x0017}, {0x019f, 0x019f, 0x01e3, 0x01e3},
    {0x01df, 0x01df, 0x0013, 0x0013}, {0x001f, 0x001f, 0x003f, 0x003f},
    {0x00f4, 0x00f8, 0x00fc, 0x0100}, {0x0043, 0x0043, 0x004b, 0x004b},
    {0x0053, 0x0053, 0x0047, 0x0047}, {0x002f, 0x002f, 0x0033, 0x0033},
    {0x003b, 0x003b, 0x0037, 0x0037}};

/* .CodeBook = HuffmanCodeBook_x, .Dimension = 4, .numBits = 2, .Offset =  0  */
const CodeBookDescription AACcodeBookDescriptionTable[13] = {
    {NULL, 0, 0, 0},
    {HuffmanCodeBook_1, 4, 2, 1},
    {HuffmanCodeBook_2, 4, 2, 1},
    {HuffmanCodeBook_3, 4, 2, 0},
    {HuffmanCodeBook_4, 4, 2, 0},
    {HuffmanCodeBook_5, 2, 4, 4},
    {HuffmanCodeBook_6, 2, 4, 4},
    {HuffmanCodeBook_7, 2, 4, 0},
    {HuffmanCodeBook_8, 2, 4, 0},
    {HuffmanCodeBook_9, 2, 4, 0},
    {HuffmanCodeBook_10, 2, 4, 0},
    {HuffmanCodeBook_11, 2, 5, 0},
    {HuffmanCodeBook_SCL, 1, 8, 60}};

const CodeBookDescription AACcodeBookDescriptionSCL = {HuffmanCodeBook_SCL, 1,
                                                       8, 60};

/* *********************************************************************************************
 */
/*  Table: HuffTree41 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 1).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 4)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 4 */
/* ---------------------------------------------------------------------------------------------
 */
/* HuffTree */
const UINT aHuffTree41[80] = {
    0x4a0001, 0x026002, 0x013003, 0x021004, 0x01c005, 0x00b006, 0x010007,
    0x019008, 0x00900e, 0x00a03a, 0x400528, 0x00c037, 0x00d03b, 0x454404,
    0x00f04c, 0x448408, 0x017011, 0x01202e, 0x42c40c, 0x034014, 0x01502c,
    0x016049, 0x410470, 0x01804e, 0x414424, 0x03201a, 0x02001b, 0x520418,
    0x02f01d, 0x02a01e, 0x01f04d, 0x41c474, 0x540420, 0x022024, 0x04a023,
    0x428510, 0x025029, 0x430508, 0x02703c, 0x028047, 0x50c434, 0x438478,
    0x04802b, 0x46443c, 0x02d03e, 0x4404b0, 0x44451c, 0x03003f, 0x03104b,
    0x52444c, 0x033039, 0x4f0450, 0x035041, 0x036046, 0x4e8458, 0x04f038,
    0x45c53c, 0x4604e0, 0x4f8468, 0x46c4d4, 0x04503d, 0x4ac47c, 0x518480,
    0x043040, 0x4844dc, 0x042044, 0x4884a8, 0x4bc48c, 0x530490, 0x4a4494,
    0x4984b8, 0x49c4c4, 0x5044b4, 0x5004c0, 0x4d04c8, 0x4f44cc, 0x4d8538,
    0x4ec4e4, 0x52c4fc, 0x514534};

/* *********************************************************************************************
 */
/*  Table: HuffTree42 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 2).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 4)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 4 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree42[80] = {
    0x026001, 0x014002, 0x009003, 0x010004, 0x01d005, 0x00600d, 0x007018,
    0x450008, 0x4e0400, 0x02e00a, 0x03900b, 0x03d00c, 0x43c404, 0x01b00e,
    0x00f04f, 0x4d8408, 0x023011, 0x01203b, 0x01a013, 0x41440c, 0x015020,
    0x016040, 0x025017, 0x500410, 0x038019, 0x540418, 0x41c444, 0x02d01c,
    0x420520, 0x01e042, 0x03701f, 0x4244cc, 0x02a021, 0x02204c, 0x478428,
    0x024031, 0x42c4dc, 0x4304e8, 0x027033, 0x4a0028, 0x50c029, 0x4344a4,
    0x02c02b, 0x470438, 0x4404c8, 0x4f8448, 0x04902f, 0x04b030, 0x44c484,
    0x524032, 0x4ec454, 0x03e034, 0x035046, 0x4c4036, 0x488458, 0x4d445c,
    0x460468, 0x04e03a, 0x51c464, 0x03c04a, 0x46c514, 0x47453c, 0x04503f,
    0x47c4ac, 0x044041, 0x510480, 0x04304d, 0x4e448c, 0x490518, 0x49449c,
    0x048047, 0x4c0498, 0x4b84a8, 0x4b0508, 0x4fc4b4, 0x4bc504, 0x5304d0,
    0x5344f0, 0x4f452c, 0x528538};

/* *********************************************************************************************
 */
/*  Table: HuffTree43 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 3).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 4)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 4 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree43[80] = {
    0x400001, 0x002004, 0x00300a, 0x46c404, 0x00b005, 0x00600d, 0x034007,
    0x037008, 0x494009, 0x4d8408, 0x42440c, 0x00c01b, 0x490410, 0x00e016,
    0x00f011, 0x010014, 0x4144fc, 0x01201d, 0x020013, 0x508418, 0x4c0015,
    0x41c440, 0x022017, 0x018026, 0x019035, 0x03801a, 0x420444, 0x01c01f,
    0x430428, 0x02101e, 0x44842c, 0x478434, 0x4b4438, 0x45443c, 0x02c023,
    0x039024, 0x02503f, 0x48844c, 0x030027, 0x02e028, 0x032029, 0x02a041,
    0x4d402b, 0x4504f0, 0x04302d, 0x4584a8, 0x02f03b, 0x46045c, 0x03103d,
    0x464046, 0x033044, 0x46853c, 0x47049c, 0x045036, 0x4744dc, 0x4a047c,
    0x500480, 0x4ac03a, 0x4b8484, 0x03c04e, 0x48c524, 0x03e040, 0x4984e8,
    0x50c4a4, 0x4b0530, 0x042047, 0x4bc04b, 0x4e44c4, 0x5184c8, 0x52c4cc,
    0x5204d0, 0x04d048, 0x04a049, 0x4e004c, 0x51c4ec, 0x4f4510, 0x5284f8,
    0x50404f, 0x514538, 0x540534};

/* *********************************************************************************************
 */
/*  Table: HuffTree44 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 4).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 4)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 4 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree44[80] = {
    0x001004, 0x020002, 0x036003, 0x490400, 0x005008, 0x010006, 0x01f007,
    0x404428, 0x00e009, 0x01100a, 0x00b018, 0x01600c, 0x03700d, 0x408015,
    0x00f03e, 0x40c424, 0x410478, 0x022012, 0x038013, 0x01e014, 0x454414,
    0x448418, 0x025017, 0x47441c, 0x030019, 0x02601a, 0x02d01b, 0x01c034,
    0x01d029, 0x4204f0, 0x4dc42c, 0x470430, 0x02103c, 0x4a0434, 0x02302a,
    0x440024, 0x4384a8, 0x43c44c, 0x02703a, 0x02802c, 0x444524, 0x4504e0,
    0x02b03d, 0x458480, 0x45c4f4, 0x04b02e, 0x04f02f, 0x460520, 0x042031,
    0x048032, 0x049033, 0x514464, 0x03504c, 0x540468, 0x47c46c, 0x4844d8,
    0x039044, 0x4884fc, 0x03b045, 0x48c53c, 0x49449c, 0x4b8498, 0x03f046,
    0x041040, 0x4c44a4, 0x50c4ac, 0x04a043, 0x5184b0, 0x4e44b4, 0x4bc4ec,
    0x04e047, 0x4c04e8, 0x4c8510, 0x4cc52c, 0x4d0530, 0x5044d4, 0x53804d,
    0x5284f8, 0x508500, 0x51c534};

/* *********************************************************************************************
 */
/*  Table: HuffTree21 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 5).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree21[80] = {
    0x450001, 0x044002, 0x042003, 0x035004, 0x026005, 0x022006, 0x013007,
    0x010008, 0x00d009, 0x01c00a, 0x01f00b, 0x01e00c, 0x4a0400, 0x01b00e,
    0x03200f, 0x47e402, 0x020011, 0x01204d, 0x40449c, 0x017014, 0x015019,
    0x01603f, 0x406458, 0x01804f, 0x448408, 0x04901a, 0x40a45a, 0x48c40c,
    0x01d031, 0x40e48e, 0x490410, 0x492412, 0x021030, 0x480414, 0x033023,
    0x02402e, 0x02503e, 0x416482, 0x02a027, 0x02802c, 0x029040, 0x418468,
    0x02b04a, 0x41a486, 0x02d048, 0x41c484, 0x04e02f, 0x41e426, 0x420434,
    0x42249e, 0x424494, 0x03d034, 0x428470, 0x039036, 0x03703b, 0x038041,
    0x42a476, 0x03a04b, 0x42c454, 0x03c047, 0x42e472, 0x430478, 0x43246e,
    0x496436, 0x488438, 0x43a466, 0x046043, 0x43c464, 0x04504c, 0x43e462,
    0x460440, 0x44245e, 0x45c444, 0x46a446, 0x44a456, 0x47444c, 0x45244e,
    0x46c47c, 0x48a47a, 0x49a498};

/* *********************************************************************************************
 */
/*  Table: HuffTree22 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 6).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree22[80] = {
    0x03c001, 0x02f002, 0x020003, 0x01c004, 0x00f005, 0x00c006, 0x016007,
    0x04d008, 0x00b009, 0x01500a, 0x400490, 0x40e402, 0x00d013, 0x00e02a,
    0x40c404, 0x019010, 0x011041, 0x038012, 0x40a406, 0x014037, 0x40849c,
    0x4a0410, 0x04a017, 0x458018, 0x412422, 0x02801a, 0x01b029, 0x480414,
    0x02401d, 0x01e02b, 0x48a01f, 0x416432, 0x02d021, 0x026022, 0x023039,
    0x418468, 0x025043, 0x48641a, 0x027040, 0x41c488, 0x41e48c, 0x42045a,
    0x47c424, 0x04c02c, 0x46e426, 0x03602e, 0x428478, 0x030033, 0x43c031,
    0x04b032, 0x42e42a, 0x03403a, 0x035048, 0x42c442, 0x470430, 0x494434,
    0x43649a, 0x45c438, 0x04403b, 0x43a454, 0x04503d, 0x03e03f, 0x43e464,
    0x440460, 0x484444, 0x049042, 0x446448, 0x44a456, 0x46644c, 0x047046,
    0x44e452, 0x450462, 0x47445e, 0x46a496, 0x49846c, 0x472476, 0x47a482,
    0x04e04f, 0x47e492, 0x48e49e};

/* *********************************************************************************************
 */
/*  Table: HuffTree23 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 7).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree23[63] = {
    0x400001, 0x002003, 0x410402, 0x004007, 0x412005, 0x01c006, 0x420404,
    0x00800b, 0x01d009, 0x00a01f, 0x406026, 0x00c012, 0x00d00f, 0x02700e,
    0x408440, 0x010022, 0x028011, 0x45440a, 0x013017, 0x029014, 0x024015,
    0x01602f, 0x43c40c, 0x02b018, 0x019033, 0x03201a, 0x43e01b, 0x47040e,
    0x422414, 0x01e025, 0x432416, 0x020021, 0x418442, 0x41a452, 0x036023,
    0x41c446, 0x46441e, 0x424430, 0x426434, 0x436428, 0x44442a, 0x02e02a,
    0x45642c, 0x03002c, 0x02d03b, 0x46642e, 0x43a438, 0x460448, 0x031037,
    0x47244a, 0x45a44c, 0x034039, 0x038035, 0x47844e, 0x462450, 0x474458,
    0x46a45c, 0x03a03c, 0x45e47a, 0x476468, 0x03d03e, 0x47c46c, 0x46e47e};

/* *********************************************************************************************
 */
/*  Table: HuffTree24 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 8).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree24[63] = {
    0x001006, 0x01d002, 0x005003, 0x424004, 0x400420, 0x414402, 0x00700a,
    0x008020, 0x00901f, 0x404432, 0x00b011, 0x00c00e, 0x00d032, 0x406446,
    0x02300f, 0x033010, 0x458408, 0x025012, 0x013016, 0x01402f, 0x015038,
    0x46840a, 0x028017, 0x01801a, 0x039019, 0x40c47a, 0x03e01b, 0x03b01c,
    0x40e47e, 0x41201e, 0x422410, 0x416434, 0x02a021, 0x02202b, 0x418444,
    0x02c024, 0x41a456, 0x02d026, 0x027034, 0x46241c, 0x029036, 0x41e45c,
    0x426031, 0x428430, 0x45242a, 0x03702e, 0x42c464, 0x03003c, 0x47442e,
    0x436442, 0x438454, 0x43a448, 0x03503a, 0x43c466, 0x43e03d, 0x44a440,
    0x44c472, 0x46044e, 0x45a450, 0x45e470, 0x46a476, 0x46c478, 0x47c46e};

/* *********************************************************************************************
 */
/*  Table: HuffTree25 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 9).        */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree25[168] = {
    0x400001, 0x002003, 0x41a402, 0x004007, 0x41c005, 0x035006, 0x434404,
    0x008010, 0x00900c, 0x04a00a, 0x42000b, 0x44e406, 0x03600d, 0x03800e,
    0x05a00f, 0x408468, 0x01101a, 0x012016, 0x039013, 0x070014, 0x46e015,
    0x40a440, 0x03b017, 0x01804d, 0x01904f, 0x4b840c, 0x01b022, 0x01c041,
    0x03f01d, 0x01e020, 0x01f05b, 0x40e4ee, 0x02107c, 0x45c410, 0x02302c,
    0x024028, 0x053025, 0x026045, 0x02707d, 0x412522, 0x047029, 0x05e02a,
    0x02b08a, 0x526414, 0x05602d, 0x02e081, 0x02f032, 0x06e030, 0x031080,
    0x416544, 0x079033, 0x034091, 0x41852c, 0x43641e, 0x04b037, 0x42246a,
    0x43c424, 0x04c03a, 0x426456, 0x03c066, 0x03d03e, 0x482428, 0x45842a,
    0x040072, 0x42c4ba, 0x050042, 0x04305c, 0x044074, 0x42e4be, 0x06a046,
    0x4dc430, 0x075048, 0x0490a3, 0x44a432, 0x450438, 0x43a452, 0x48443e,
    0x04e068, 0x45a442, 0x4d4444, 0x051088, 0x052087, 0x44648c, 0x077054,
    0x4da055, 0x50a448, 0x057060, 0x06b058, 0x05906d, 0x44c4f6, 0x46c454,
    0x45e474, 0x06905d, 0x460520, 0x05f07e, 0x462494, 0x061063, 0x07f062,
    0x464496, 0x06408b, 0x08d065, 0x542466, 0x067071, 0x4d2470, 0x4724ec,
    0x478476, 0x53a47a, 0x09b06c, 0x47c4ac, 0x4f847e, 0x06f078, 0x510480,
    0x48649e, 0x4884a0, 0x07307b, 0x49c48a, 0x4a648e, 0x098076, 0x4904c0,
    0x4924ea, 0x4c8498, 0x07a08e, 0x51249a, 0x4a24d6, 0x5064a4, 0x4f24a8,
    0x4aa4de, 0x51e4ae, 0x4b0538, 0x082092, 0x083085, 0x08f084, 0x5464b2,
    0x096086, 0x4ce4b4, 0x4d04b6, 0x089090, 0x4bc508, 0x4c253e, 0x08c0a4,
    0x5284c4, 0x4e04c6, 0x4ca4fa, 0x5144cc, 0x4f04d8, 0x4e24fc, 0x09309c,
    0x094099, 0x095097, 0x4e4516, 0x4e652e, 0x4e84fe, 0x4f450c, 0x09a09f,
    0x500502, 0x50450e, 0x09d0a0, 0x09e0a5, 0x518530, 0x51a54a, 0x0a70a1,
    0x0a20a6, 0x51c534, 0x53c524, 0x54052a, 0x548532, 0x536550, 0x54c54e};

/* *********************************************************************************************
 */
/*  Table: HuffTree26 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 10).       */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree26[168] = {
    0x006001, 0x002013, 0x00300f, 0x00400d, 0x03b005, 0x40046e, 0x037007,
    0x00800a, 0x009067, 0x402420, 0x05600b, 0x00c057, 0x434404, 0x06600e,
    0x406470, 0x03c010, 0x059011, 0x06f012, 0x49e408, 0x014019, 0x03f015,
    0x016044, 0x017042, 0x079018, 0x4b840a, 0x01a01f, 0x01b047, 0x07c01c,
    0x08701d, 0x06901e, 0x44640c, 0x020027, 0x04b021, 0x02204f, 0x023025,
    0x02406b, 0x40e4e0, 0x081026, 0x528410, 0x02802c, 0x06c029, 0x08f02a,
    0x02b078, 0x53a412, 0x05202d, 0x02e033, 0x02f031, 0x0300a2, 0x4144ce,
    0x0a6032, 0x416534, 0x09a034, 0x09f035, 0x0360a7, 0x54e418, 0x03a038,
    0x436039, 0x43841a, 0x41c41e, 0x42246a, 0x05803d, 0x03e068, 0x424484,
    0x04005b, 0x04107a, 0x42645a, 0x043093, 0x4d2428, 0x05e045, 0x046072,
    0x42a45e, 0x048060, 0x073049, 0x04a098, 0x42c4c4, 0x07504c, 0x09504d,
    0x04e09c, 0x51042e, 0x063050, 0x077051, 0x43053c, 0x053084, 0x065054,
    0x4e4055, 0x4fe432, 0x43a454, 0x43c46c, 0x43e486, 0x07005a, 0x4a0440,
    0x07105c, 0x05d07b, 0x45c442, 0x05f08a, 0x476444, 0x07f061, 0x06206a,
    0x448506, 0x06408e, 0x52644a, 0x54444c, 0x45644e, 0x452450, 0x488458,
    0x4604ec, 0x4624f6, 0x50e464, 0x08206d, 0x0a406e, 0x542466, 0x4a2468,
    0x48a472, 0x474089, 0x4d8478, 0x097074, 0x47a508, 0x08d076, 0x47c4b6,
    0x51247e, 0x4804fc, 0x4bc482, 0x48c4a4, 0x48e4d4, 0x07d07e, 0x4904da,
    0x49208b, 0x094080, 0x49450c, 0x4964e2, 0x09d083, 0x52a498, 0x085091,
    0x0a5086, 0x4cc49a, 0x08808c, 0x4ee49c, 0x4a64ba, 0x4a84c0, 0x4c24aa,
    0x4ac4f0, 0x4ae4d0, 0x4ca4b0, 0x0900a1, 0x4b24ea, 0x092099, 0x4b4516,
    0x4d64be, 0x4c650a, 0x522096, 0x4c8524, 0x4dc4f2, 0x4de4f4, 0x4e6548,
    0x09e09b, 0x5384e8, 0x5204f8, 0x4fa53e, 0x50051a, 0x0a30a0, 0x502536,
    0x514504, 0x51e518, 0x54a51c, 0x54052c, 0x52e546, 0x530532, 0x54c550};

/* *********************************************************************************************
 */
/*  Table: HuffTree27 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the decode tree for spectral data
 * (Codebook 11).       */
/*                    bit 23 and 11 not used */
/*                    bit 22 and 10 determine end value */
/*                    bit 21-12 and 9-0 (offset to next node) or (index value *
 * 2)               */
/* ---------------------------------------------------------------------------------------------
 */
/*    input:          codeword */
/* ---------------------------------------------------------------------------------------------
 */
/*    output:         index * 2 */
/* ---------------------------------------------------------------------------------------------
 */
const UINT aHuffTree27[288] = {
    0x00100d, 0x002006, 0x003004, 0x400424, 0x047005, 0x402446, 0x048007,
    0x00800a, 0x00904c, 0x44a404, 0x07400b, 0x00c0bb, 0x466406, 0x00e014,
    0x00f054, 0x04e010, 0x051011, 0x0a9012, 0x0130bc, 0x408464, 0x01501f,
    0x01601a, 0x017059, 0x0af018, 0x0ca019, 0x40a0e4, 0x01b05e, 0x01c084,
    0x0bf01d, 0x05d01e, 0x55a40c, 0x020026, 0x021066, 0x043022, 0x023062,
    0x02408d, 0x025108, 0x40e480, 0x027030, 0x02802c, 0x02906b, 0x02a0da,
    0x06502b, 0x4105c8, 0x0a402d, 0x0ec02e, 0x0dd02f, 0x532412, 0x06e031,
    0x032036, 0x03303e, 0x0fd034, 0x0fc035, 0x4145b0, 0x03703a, 0x038117,
    0x10d039, 0x5ba416, 0x10f03b, 0x03c041, 0x5fa03d, 0x41c418, 0x10403f,
    0x04011d, 0x41a5f4, 0x11c042, 0x41e61c, 0x087044, 0x0f5045, 0x0d9046,
    0x4204a2, 0x640422, 0x04904a, 0x426448, 0x04b073, 0x428468, 0x46c04d,
    0x48a42a, 0x04f077, 0x076050, 0x42c4b0, 0x0520a7, 0x096053, 0x42e4a8,
    0x05507d, 0x07a056, 0x0d4057, 0x0df058, 0x442430, 0x05a081, 0x05b09b,
    0x05c0e2, 0x5b8432, 0x4fe434, 0x05f09e, 0x0e6060, 0x0610d6, 0x57c436,
    0x0cc063, 0x112064, 0x4384a0, 0x43a5ca, 0x067089, 0x0680b7, 0x0690a2,
    0x0a106a, 0x43c59c, 0x09206c, 0x06d0ba, 0x60643e, 0x0d106f, 0x0700ee,
    0x0de071, 0x10b072, 0x44056c, 0x46a444, 0x075094, 0x48c44c, 0x44e490,
    0x095078, 0x0ab079, 0x4504ce, 0x07b097, 0x11e07c, 0x630452, 0x0ac07e,
    0x07f099, 0x080106, 0x4544b8, 0x0820b1, 0x0830e5, 0x4fc456, 0x0b3085,
    0x08609d, 0x45853e, 0x0880c2, 0x5c045a, 0x08a08f, 0x08b0ce, 0x08c0f7,
    0x58645c, 0x11108e, 0x45e5c4, 0x0c4090, 0x10a091, 0x4604e4, 0x0d0093,
    0x462608, 0x48e46e, 0x4704b2, 0x4d2472, 0x0980bd, 0x4f2474, 0x0e309a,
    0x4764aa, 0x0be09c, 0x47851a, 0x47a4de, 0x09f0b5, 0x0a00c1, 0x50047c,
    0x57847e, 0x0a30c3, 0x504482, 0x0e90a5, 0x0a6100, 0x4c8484, 0x0a811f,
    0x48662a, 0x0c70aa, 0x488494, 0x4924d0, 0x0ad0c8, 0x0ae0d8, 0x496636,
    0x10e0b0, 0x4f8498, 0x0f30b2, 0x49a4dc, 0x0f20b4, 0x53c49c, 0x0b60cb,
    0x49e57a, 0x0b80e0, 0x0b9109, 0x5e44a4, 0x5484a6, 0x4ac4ae, 0x4b44ca,
    0x4d64b6, 0x4ba5da, 0x0c60c0, 0x4bc51e, 0x4be556, 0x6204c0, 0x4c24c4,
    0x0f80c5, 0x5664c6, 0x4cc53a, 0x4d462c, 0x0f10c9, 0x4d8552, 0x4da4fa,
    0x5be4e0, 0x0cd0ff, 0x5244e2, 0x0cf0e8, 0x4e6568, 0x59a4e8, 0x0f90d2,
    0x1010d3, 0x5ac4ea, 0x0d50d7, 0x4ec634, 0x4ee560, 0x4f44f0, 0x4f6638,
    0x502522, 0x0db0dc, 0x5065a6, 0x508604, 0x60050a, 0x50c0fb, 0x63250e,
    0x1130e1, 0x5a4510, 0x5125fc, 0x516514, 0x51863e, 0x51c536, 0x0e70f4,
    0x55c520, 0x602526, 0x0eb0ea, 0x5cc528, 0x5ea52a, 0x1140ed, 0x60c52c,
    0x1020ef, 0x0f0119, 0x58e52e, 0x530622, 0x558534, 0x53861e, 0x55e540,
    0x5800f6, 0x57e542, 0x5445e6, 0x5465e8, 0x0fa115, 0x54c54a, 0x54e60e,
    0x5ae550, 0x1160fe, 0x5f0554, 0x564562, 0x56a58a, 0x56e5ee, 0x10310c,
    0x5705d0, 0x107105, 0x5725d4, 0x57463a, 0x5765b4, 0x5825bc, 0x5845e2,
    0x5885de, 0x58c592, 0x5ce590, 0x5945f6, 0x63c596, 0x11b110, 0x5d8598,
    0x5c259e, 0x5e05a0, 0x5a25c6, 0x5a860a, 0x5aa5ec, 0x5b2610, 0x11a118,
    0x6185b6, 0x5f25d2, 0x5d6616, 0x5dc5f8, 0x61a5fe, 0x612614, 0x62e624,
    0x626628};

/* get starting addresses of huffman tables into an array [convert codebook into
 * starting address] */
/* cb    tree */
const UINT *aHuffTable[MAX_CB] = {
    aHuffTree41,
    /* 0      -   */ /* use tree 1 as dummy here */
    aHuffTree41,     /* 1      1   */
    aHuffTree42,     /* 2      2   */
    aHuffTree43,     /* 3      3   */
    aHuffTree44,     /* 4      4   */
    aHuffTree21,     /* 5      5   */
    aHuffTree22,     /* 6      6   */
    aHuffTree23,     /* 7      7   */
    aHuffTree24,     /* 8      8   */
    aHuffTree25,     /* 9      9   */
    aHuffTree26,     /* 10     10  */
    aHuffTree27,     /* 11     11  */
    aHuffTree41,
    /* 12     -   */ /* use tree 1 as dummy here */
    aHuffTree41,
    /* 13     -   */ /* use tree 1 as dummy here */
    aHuffTree41,
    /* 14     -   */ /* use tree 1 as dummy here */
    aHuffTree41,
    /* 15     -   */ /* use tree 1 as dummy here */
    aHuffTree27,     /* 16     11  */
    aHuffTree27,     /* 17     11  */
    aHuffTree27,     /* 18     11  */
    aHuffTree27,     /* 19     11  */
    aHuffTree27,     /* 20     11  */
    aHuffTree27,     /* 21     11  */
    aHuffTree27,     /* 22     11  */
    aHuffTree27,     /* 23     11  */
    aHuffTree27,     /* 24     11  */
    aHuffTree27,     /* 25     11  */
    aHuffTree27,     /* 26     11  */
    aHuffTree27,     /* 27     11  */
    aHuffTree27,     /* 28     11  */
    aHuffTree27,     /* 29     11  */
    aHuffTree27,     /* 30     11  */
    aHuffTree27};    /* 31     11  */

/*---------------------------------------------------------------------------------------------
   data-description:
  The following tables contain the quantized values. Two or four of the
 quantized values are indexed by the result of the decoding in the decoding tree
 (see tables above).
 --------------------------------------------------------------------------------------------
 */

/* *********************************************************************************************
 */
/*  Table: ValTab41 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks
 * 1-2.                */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab41[324] = {
    -1, -1, -1, -1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, 0,  -1, -1, -1,
    0,  0,  -1, -1, 0,  1,  -1, -1, 1,  -1, -1, -1, 1,  0,  -1, -1, 1,  1,
    -1, 0,  -1, -1, -1, 0,  -1, 0,  -1, 0,  -1, 1,  -1, 0,  0,  -1, -1, 0,
    0,  0,  -1, 0,  0,  1,  -1, 0,  1,  -1, -1, 0,  1,  0,  -1, 0,  1,  1,
    -1, 1,  -1, -1, -1, 1,  -1, 0,  -1, 1,  -1, 1,  -1, 1,  0,  -1, -1, 1,
    0,  0,  -1, 1,  0,  1,  -1, 1,  1,  -1, -1, 1,  1,  0,  -1, 1,  1,  1,
    0,  -1, -1, -1, 0,  -1, -1, 0,  0,  -1, -1, 1,  0,  -1, 0,  -1, 0,  -1,
    0,  0,  0,  -1, 0,  1,  0,  -1, 1,  -1, 0,  -1, 1,  0,  0,  -1, 1,  1,
    0,  0,  -1, -1, 0,  0,  -1, 0,  0,  0,  -1, 1,  0,  0,  0,  -1, 0,  0,
    0,  0,  0,  0,  0,  1,  0,  0,  1,  -1, 0,  0,  1,  0,  0,  0,  1,  1,
    0,  1,  -1, -1, 0,  1,  -1, 0,  0,  1,  -1, 1,  0,  1,  0,  -1, 0,  1,
    0,  0,  0,  1,  0,  1,  0,  1,  1,  -1, 0,  1,  1,  0,  0,  1,  1,  1,
    1,  -1, -1, -1, 1,  -1, -1, 0,  1,  -1, -1, 1,  1,  -1, 0,  -1, 1,  -1,
    0,  0,  1,  -1, 0,  1,  1,  -1, 1,  -1, 1,  -1, 1,  0,  1,  -1, 1,  1,
    1,  0,  -1, -1, 1,  0,  -1, 0,  1,  0,  -1, 1,  1,  0,  0,  -1, 1,  0,
    0,  0,  1,  0,  0,  1,  1,  0,  1,  -1, 1,  0,  1,  0,  1,  0,  1,  1,
    1,  1,  -1, -1, 1,  1,  -1, 0,  1,  1,  -1, 1,  1,  1,  0,  -1, 1,  1,
    0,  0,  1,  1,  0,  1,  1,  1,  1,  -1, 1,  1,  1,  0,  1,  1,  1,  1};

/* *********************************************************************************************
 */
/*  Table: ValTab42 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks
 * 3-4.                */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab42[324] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 2, 0,
    0, 2, 0, 0, 0, 2, 1, 0, 0, 2, 2, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 2, 0, 1,
    1, 0, 0, 1, 1, 1, 0, 1, 1, 2, 0, 1, 2, 0, 0, 1, 2, 1, 0, 1, 2, 2, 0, 2, 0,
    0, 0, 2, 0, 1, 0, 2, 0, 2, 0, 2, 1, 0, 0, 2, 1, 1, 0, 2, 1, 2, 0, 2, 2, 0,
    0, 2, 2, 1, 0, 2, 2, 2, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 2, 1, 0, 1, 0, 1,
    0, 1, 1, 1, 0, 1, 2, 1, 0, 2, 0, 1, 0, 2, 1, 1, 0, 2, 2, 1, 1, 0, 0, 1, 1,
    0, 1, 1, 1, 0, 2, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 0, 1, 1, 2,
    1, 1, 1, 2, 2, 1, 2, 0, 0, 1, 2, 0, 1, 1, 2, 0, 2, 1, 2, 1, 0, 1, 2, 1, 1,
    1, 2, 1, 2, 1, 2, 2, 0, 1, 2, 2, 1, 1, 2, 2, 2, 2, 0, 0, 0, 2, 0, 0, 1, 2,
    0, 0, 2, 2, 0, 1, 0, 2, 0, 1, 1, 2, 0, 1, 2, 2, 0, 2, 0, 2, 0, 2, 1, 2, 0,
    2, 2, 2, 1, 0, 0, 2, 1, 0, 1, 2, 1, 0, 2, 2, 1, 1, 0, 2, 1, 1, 1, 2, 1, 1,
    2, 2, 1, 2, 0, 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 0, 0, 2, 2, 0, 1, 2, 2, 0, 2,
    2, 2, 1, 0, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2, 2};

/* *********************************************************************************************
 */
/*  Table: ValTab21 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks
 * 5-6.                */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab21[162] = {
    -4, -4, -4, -3, -4, -2, -4, -1, -4, 0, -4, 1, -4, 2, -4, 3, -4, 4,
    -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 3, -3, 4,
    -2, -4, -2, -3, -2, -2, -2, -1, -2, 0, -2, 1, -2, 2, -2, 3, -2, 4,
    -1, -4, -1, -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4,
    0,  -4, 0,  -3, 0,  -2, 0,  -1, 0,  0, 0,  1, 0,  2, 0,  3, 0,  4,
    1,  -4, 1,  -3, 1,  -2, 1,  -1, 1,  0, 1,  1, 1,  2, 1,  3, 1,  4,
    2,  -4, 2,  -3, 2,  -2, 2,  -1, 2,  0, 2,  1, 2,  2, 2,  3, 2,  4,
    3,  -4, 3,  -3, 3,  -2, 3,  -1, 3,  0, 3,  1, 3,  2, 3,  3, 3,  4,
    4,  -4, 4,  -3, 4,  -2, 4,  -1, 4,  0, 4,  1, 4,  2, 4,  3, 4,  4};

/* *********************************************************************************************
 */
/*  Table: ValTab22 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks
 * 7-8.                */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab22[128] = {
    0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 1, 0, 1, 1, 1, 2,
    1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 2, 0, 2, 1, 2, 2, 2, 3, 2, 4, 2, 5,
    2, 6, 2, 7, 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 5, 3, 6, 3, 7, 4, 0,
    4, 1, 4, 2, 4, 3, 4, 4, 4, 5, 4, 6, 4, 7, 5, 0, 5, 1, 5, 2, 5, 3,
    5, 4, 5, 5, 5, 6, 5, 7, 6, 0, 6, 1, 6, 2, 6, 3, 6, 4, 6, 5, 6, 6,
    6, 7, 7, 0, 7, 1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 6, 7, 7};

/* *********************************************************************************************
 */
/*  Table: ValTab23 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks
 * 9-10.               */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab23[338] = {
    0,  0,  0,  1,  0,  2,  0,  3,  0,  4,  0,  5,  0,  6,  0,  7,  0,  8,  0,
    9,  0,  10, 0,  11, 0,  12, 1,  0,  1,  1,  1,  2,  1,  3,  1,  4,  1,  5,
    1,  6,  1,  7,  1,  8,  1,  9,  1,  10, 1,  11, 1,  12, 2,  0,  2,  1,  2,
    2,  2,  3,  2,  4,  2,  5,  2,  6,  2,  7,  2,  8,  2,  9,  2,  10, 2,  11,
    2,  12, 3,  0,  3,  1,  3,  2,  3,  3,  3,  4,  3,  5,  3,  6,  3,  7,  3,
    8,  3,  9,  3,  10, 3,  11, 3,  12, 4,  0,  4,  1,  4,  2,  4,  3,  4,  4,
    4,  5,  4,  6,  4,  7,  4,  8,  4,  9,  4,  10, 4,  11, 4,  12, 5,  0,  5,
    1,  5,  2,  5,  3,  5,  4,  5,  5,  5,  6,  5,  7,  5,  8,  5,  9,  5,  10,
    5,  11, 5,  12, 6,  0,  6,  1,  6,  2,  6,  3,  6,  4,  6,  5,  6,  6,  6,
    7,  6,  8,  6,  9,  6,  10, 6,  11, 6,  12, 7,  0,  7,  1,  7,  2,  7,  3,
    7,  4,  7,  5,  7,  6,  7,  7,  7,  8,  7,  9,  7,  10, 7,  11, 7,  12, 8,
    0,  8,  1,  8,  2,  8,  3,  8,  4,  8,  5,  8,  6,  8,  7,  8,  8,  8,  9,
    8,  10, 8,  11, 8,  12, 9,  0,  9,  1,  9,  2,  9,  3,  9,  4,  9,  5,  9,
    6,  9,  7,  9,  8,  9,  9,  9,  10, 9,  11, 9,  12, 10, 0,  10, 1,  10, 2,
    10, 3,  10, 4,  10, 5,  10, 6,  10, 7,  10, 8,  10, 9,  10, 10, 10, 11, 10,
    12, 11, 0,  11, 1,  11, 2,  11, 3,  11, 4,  11, 5,  11, 6,  11, 7,  11, 8,
    11, 9,  11, 10, 11, 11, 11, 12, 12, 0,  12, 1,  12, 2,  12, 3,  12, 4,  12,
    5,  12, 6,  12, 7,  12, 8,  12, 9,  12, 10, 12, 11, 12, 12};

/* *********************************************************************************************
 */
/*  Table: ValTab24 */
/* ---------------------------------------------------------------------------------------------
 */
/*    description:    This table contains the quantized values for codebooks 11.
 */
/* ---------------------------------------------------------------------------------------------
 */
const SCHAR aValTab24[578] = {
    0,  0,  0,  1,  0,  2,  0,  3,  0,  4,  0,  5,  0,  6,  0,  7,  0,  8,  0,
    9,  0,  10, 0,  11, 0,  12, 0,  13, 0,  14, 0,  15, 0,  16, 1,  0,  1,  1,
    1,  2,  1,  3,  1,  4,  1,  5,  1,  6,  1,  7,  1,  8,  1,  9,  1,  10, 1,
    11, 1,  12, 1,  13, 1,  14, 1,  15, 1,  16, 2,  0,  2,  1,  2,  2,  2,  3,
    2,  4,  2,  5,  2,  6,  2,  7,  2,  8,  2,  9,  2,  10, 2,  11, 2,  12, 2,
    13, 2,  14, 2,  15, 2,  16, 3,  0,  3,  1,  3,  2,  3,  3,  3,  4,  3,  5,
    3,  6,  3,  7,  3,  8,  3,  9,  3,  10, 3,  11, 3,  12, 3,  13, 3,  14, 3,
    15, 3,  16, 4,  0,  4,  1,  4,  2,  4,  3,  4,  4,  4,  5,  4,  6,  4,  7,
    4,  8,  4,  9,  4,  10, 4,  11, 4,  12, 4,  13, 4,  14, 4,  15, 4,  16, 5,
    0,  5,  1,  5,  2,  5,  3,  5,  4,  5,  5,  5,  6,  5,  7,  5,  8,  5,  9,
    5,  10, 5,  11, 5,  12, 5,  13, 5,  14, 5,  15, 5,  16, 6,  0,  6,  1,  6,
    2,  6,  3,  6,  4,  6,  5,  6,  6,  6,  7,  6,  8,  6,  9,  6,  10, 6,  11,
    6,  12, 6,  13, 6,  14, 6,  15, 6,  16, 7,  0,  7,  1,  7,  2,  7,  3,  7,
    4,  7,  5,  7,  6,  7,  7,  7,  8,  7,  9,  7,  10, 7,  11, 7,  12, 7,  13,
    7,  14, 7,  15, 7,  16, 8,  0,  8,  1,  8,  2,  8,  3,  8,  4,  8,  5,  8,
    6,  8,  7,  8,  8,  8,  9,  8,  10, 8,  11, 8,  12, 8,  13, 8,  14, 8,  15,
    8,  16, 9,  0,  9,  1,  9,  2,  9,  3,  9,  4,  9,  5,  9,  6,  9,  7,  9,
    8,  9,  9,  9,  10, 9,  11, 9,  12, 9,  13, 9,  14, 9,  15, 9,  16, 10, 0,
    10, 1,  10, 2,  10, 3,  10, 4,  10, 5,  10, 6,  10, 7,  10, 8,  10, 9,  10,
    10, 10, 11, 10, 12, 10, 13, 10, 14, 10, 15, 10, 16, 11, 0,  11, 1,  11, 2,
    11, 3,  11, 4,  11, 5,  11, 6,  11, 7,  11, 8,  11, 9,  11, 10, 11, 11, 11,
    12, 11, 13, 11, 14, 11, 15, 11, 16, 12, 0,  12, 1,  12, 2,  12, 3,  12, 4,
    12, 5,  12, 6,  12, 7,  12, 8,  12, 9,  12, 10, 12, 11, 12, 12, 12, 13, 12,
    14, 12, 15, 12, 16, 13, 0,  13, 1,  13, 2,  13, 3,  13, 4,  13, 5,  13, 6,
    13, 7,  13, 8,  13, 9,  13, 10, 13, 11, 13, 12, 13, 13, 13, 14, 13, 15, 13,
    16, 14, 0,  14, 1,  14, 2,  14, 3,  14, 4,  14, 5,  14, 6,  14, 7,  14, 8,
    14, 9,  14, 10, 14, 11, 14, 12, 14, 13, 14, 14, 14, 15, 14, 16, 15, 0,  15,
    1,  15, 2,  15, 3,  15, 4,  15, 5,  15, 6,  15, 7,  15, 8,  15, 9,  15, 10,
    15, 11, 15, 12, 15, 13, 15, 14, 15, 15, 15, 16, 16, 0,  16, 1,  16, 2,  16,
    3,  16, 4,  16, 5,  16, 6,  16, 7,  16, 8,  16, 9,  16, 10, 16, 11, 16, 12,
    16, 13, 16, 14, 16, 15, 16, 16};

/* cb    quant. val table */
const SCHAR *aQuantTable[] = {
    aValTab41,
    /* 0             -        */ /* use quant. val talble 1 as dummy here */
    aValTab41,                   /* 1             1        */
    aValTab41,                   /* 2             1        */
    aValTab42,                   /* 3             2        */
    aValTab42,                   /* 4             2        */
    aValTab21,                   /* 5             3        */
    aValTab21,                   /* 6             3        */
    aValTab22,                   /* 7             4        */
    aValTab22,                   /* 8             4        */
    aValTab23,                   /* 9             5        */
    aValTab23,                   /* 10            5        */
    aValTab24,                   /* 11            6        */
    aValTab41,
    /* 12            -        */ /* use quant. val talble 1 as dummy here */
    aValTab41,
    /* 13            -        */ /* use quant. val talble 1 as dummy here */
    aValTab41,
    /* 14            -        */ /* use quant. val talble 1 as dummy here */
    aValTab41,
    /* 15            -        */ /* use quant. val talble 1 as dummy here */
    aValTab24,                   /* 16            6        */
    aValTab24,                   /* 17            6        */
    aValTab24,                   /* 18            6        */
    aValTab24,                   /* 19            6        */
    aValTab24,                   /* 20            6        */
    aValTab24,                   /* 21            6        */
    aValTab24,                   /* 22            6        */
    aValTab24,                   /* 23            6        */
    aValTab24,                   /* 24            6        */
    aValTab24,                   /* 25            6        */
    aValTab24,                   /* 26            6        */
    aValTab24,                   /* 27            6        */
    aValTab24,                   /* 28            6        */
    aValTab24,                   /* 29            6        */
    aValTab24,                   /* 30            6        */
    aValTab24};                  /* 31            6        */

/* arrays for HCR_TABLE_INFO structures */
/* maximum length of codeword in each codebook */
/* codebook:                     0,1, 2,3, 4, 5, 6, 7, 8, 9,
 * 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 */
const UCHAR aMaxCwLen[MAX_CB] = {0,  11, 9,  20, 16, 13, 11, 14, 12, 17, 14,
                                 49, 0,  0,  0,  0,  14, 17, 21, 21, 25, 25,
                                 29, 29, 29, 29, 33, 33, 33, 37, 37, 41};

/*                                                           11  13  15  17  19
 * 21  23  25  27  39  31 */
/*                            CB:  0 1 2 3 4 5 6 7 8 9 10  12  14  16  18  20
 * 22  24  26  28  30       */
const UCHAR aDimCb[MAX_CB] = {
    2, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}; /* codebook dimension -
                                                          zero cb got a
                                                          dimension of 2 */

/*                                                           11  13  15  17  19
 * 21  23  25  27  39  31 */
/*                            CB:  0 1 2 3 4 5 6 7 8 9 10  12  14  16  18  20
 * 22  24  26  28  30       */
const UCHAR aDimCbShift[MAX_CB] = {
    1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; /* codebook dimension */

/*               1 -> decode sign bits */
/*               0 -> decode no sign bits                11  13  15  17  19  21
 * 23  25  27  39  31 */
/*                        CB:  0 1 2 3 4 5 6 7 8 9 10  12  14  16  18  20  22
 * 24  26  28  30       */
const UCHAR aSignCb[MAX_CB] = {0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

/* arrays for HCR_CB_PAIRS structures */
const UCHAR aMinOfCbPair[MAX_CB_PAIRS] = {0,  1,  3,  5,  7,  9,  16, 17,
                                          18, 19, 20, 21, 22, 23, 24, 25,
                                          26, 27, 28, 29, 30, 31, 11};
const UCHAR aMaxOfCbPair[MAX_CB_PAIRS] = {0,  2,  4,  6,  8,  10, 16, 17,
                                          18, 19, 20, 21, 22, 23, 24, 25,
                                          26, 27, 28, 29, 30, 31, 11};

/* priorities of codebooks */
const UCHAR aCbPriority[MAX_CB] = {0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,
                                   22, 0,  0,  0,  0,  6,  7,  8,  9,  10, 11,
                                   12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

const SCHAR aCodebook2StartInt[] = {STOP_THIS_STATE,      /* cb  0 */
                                    BODY_ONLY,            /* cb  1 */
                                    BODY_ONLY,            /* cb  2 */
                                    BODY_SIGN__BODY,      /* cb  3 */
                                    BODY_SIGN__BODY,      /* cb  4 */
                                    BODY_ONLY,            /* cb  5 */
                                    BODY_ONLY,            /* cb  6 */
                                    BODY_SIGN__BODY,      /* cb  7 */
                                    BODY_SIGN__BODY,      /* cb  8 */
                                    BODY_SIGN__BODY,      /* cb  9 */
                                    BODY_SIGN__BODY,      /* cb 10 */
                                    BODY_SIGN_ESC__BODY,  /* cb 11 */
                                    STOP_THIS_STATE,      /* cb 12 */
                                    STOP_THIS_STATE,      /* cb 13 */
                                    STOP_THIS_STATE,      /* cb 14 */
                                    STOP_THIS_STATE,      /* cb 15 */
                                    BODY_SIGN_ESC__BODY,  /* cb 16 */
                                    BODY_SIGN_ESC__BODY,  /* cb 17 */
                                    BODY_SIGN_ESC__BODY,  /* cb 18 */
                                    BODY_SIGN_ESC__BODY,  /* cb 19 */
                                    BODY_SIGN_ESC__BODY,  /* cb 20 */
                                    BODY_SIGN_ESC__BODY,  /* cb 21 */
                                    BODY_SIGN_ESC__BODY,  /* cb 22 */
                                    BODY_SIGN_ESC__BODY,  /* cb 23 */
                                    BODY_SIGN_ESC__BODY,  /* cb 24 */
                                    BODY_SIGN_ESC__BODY,  /* cb 25 */
                                    BODY_SIGN_ESC__BODY,  /* cb 26 */
                                    BODY_SIGN_ESC__BODY,  /* cb 27 */
                                    BODY_SIGN_ESC__BODY,  /* cb 28 */
                                    BODY_SIGN_ESC__BODY,  /* cb 29 */
                                    BODY_SIGN_ESC__BODY,  /* cb 30 */
                                    BODY_SIGN_ESC__BODY}; /* cb 31 */

const STATEFUNC aStateConstant2State[] = {
    NULL,                                /*  0 = STOP_THIS_STATE           */
    Hcr_State_BODY_ONLY,                 /*  1 = BODY_ONLY                 */
    Hcr_State_BODY_SIGN__BODY,           /*  2 = BODY_SIGN__BODY           */
    Hcr_State_BODY_SIGN__SIGN,           /*  3 = BODY_SIGN__SIGN           */
    Hcr_State_BODY_SIGN_ESC__BODY,       /*  4 = BODY_SIGN_ESC__BODY       */
    Hcr_State_BODY_SIGN_ESC__SIGN,       /*  5 = BODY_SIGN_ESC__SIGN       */
    Hcr_State_BODY_SIGN_ESC__ESC_PREFIX, /*  6 = BODY_SIGN_ESC__ESC_PREFIX */
    Hcr_State_BODY_SIGN_ESC__ESC_WORD};  /*  7 = BODY_SIGN_ESC__ESC_WORD   */

/*                                     CB:  0 1 2 3 4 5 6 7 8  9 10      12
 * 14    16    18    20      22      24      26      28       30         */
const USHORT aLargestAbsoluteValue[MAX_CB] = {
    0,    1,   1,   2,   2,   4,   4,   7,   7,    12,  12,
    8191, 0,   0,   0,   0,   15,  31,  47,  63,   95,  127,
    159,  191, 223, 255, 319, 383, 511, 767, 1023, 2047}; /* lav */
/*                                     CB:                           11     13
 * 15    17    19     21      23      25      27      39       31     */

/* ------------------------------------------------------------------------------------------
   description:    The table 'HuffTreeRvlcEscape' contains the decode tree for
the rvlc escape sequences. bit 23 and 11 not used bit 22 and 10 determine end
value  -->  if set codeword is decoded bit 21-12 and 9-0 (offset to next node)
or (index value) The escape sequence is the index value.

   input:          codeword
   output:         index
------------------------------------------------------------------------------------------
*/
const UINT aHuffTreeRvlcEscape[53] = {
    0x002001, 0x400003, 0x401004, 0x402005, 0x403007, 0x404006, 0x00a405,
    0x009008, 0x00b406, 0x00c407, 0x00d408, 0x00e409, 0x40b40a, 0x40c00f,
    0x40d010, 0x40e011, 0x40f012, 0x410013, 0x411014, 0x412015, 0x016413,
    0x414415, 0x017416, 0x417018, 0x419019, 0x01a418, 0x01b41a, 0x01c023,
    0x03201d, 0x01e020, 0x43501f, 0x41b41c, 0x021022, 0x41d41e, 0x41f420,
    0x02402b, 0x025028, 0x026027, 0x421422, 0x423424, 0x02902a, 0x425426,
    0x427428, 0x02c02f, 0x02d02e, 0x42942a, 0x42b42c, 0x030031, 0x42d42e,
    0x42f430, 0x033034, 0x431432, 0x433434};

/* ------------------------------------------------------------------------------------------
   description:    The table 'HuffTreeRvlc' contains the huffman decoding tree
for the RVLC scale factors. The table contains 15 allowed, symmetric codewords
and 8 forbidden codewords, which are used for error detection.

   usage of bits:  bit 23 and 11 not used
                   bit 22 and 10 determine end value  -->  if set codeword is
decoded bit 21-12 and 9-0 (offset to next node within the table) or (index+7).
                   The decoded (index+7) is in the range from 0,1,..,22. If the
(index+7) is in the range 15,16,..,22, then a forbidden codeword is decoded.

   input:          A single bit from a RVLC scalefactor codeword
   output:         [if codeword is not completely decoded:] offset to next node
within table or [if codeword is decoded:] A dpcm value i.e. (index+7) in range
from 0,1,..,22. The differential scalefactor (DPCM value) named 'index' is
calculated by subtracting 7 from the decoded value (index+7).
------------------------------------------------------------------------------------------
*/
const UINT aHuffTreeRvlCodewds[22] = {
    0x407001, 0x002009, 0x003406, 0x004405, 0x005404, 0x006403,
    0x007400, 0x008402, 0x411401, 0x00a408, 0x00c00b, 0x00e409,
    0x01000d, 0x40f40a, 0x41400f, 0x01340b, 0x011015, 0x410012,
    0x41240c, 0x416014, 0x41540d, 0x41340e};

const FIXP_WTB LowDelaySynthesis256[768] = {
    WTC(0xdaecb88a), WTC(0xdb7a5230), WTC(0xdc093961), WTC(0xdc9977b5),
    WTC(0xdd2b11b4), WTC(0xddbe06c1), WTC(0xde525277), WTC(0xdee7f167),
    WTC(0xdf7ee2f9), WTC(0xe0173207), WTC(0xe0b0e70e), WTC(0xe14beb4d),
    WTC(0xe1e82002), WTC(0xe285693d), WTC(0xe323ba3c), WTC(0xe3c33cdb),
    WTC(0xe4640c93), WTC(0xe5060b3f), WTC(0xe5a915ce), WTC(0xe64cffc2),
    WTC(0xe6f19868), WTC(0xe796b4d7), WTC(0xe83c2ebf), WTC(0xe8e1eea1),
    WTC(0xe987f784), WTC(0xea2e8014), WTC(0xead5a2b6), WTC(0xeb7d3476),
    WTC(0xec24ebfb), WTC(0xeccc4e9a), WTC(0xed72f723), WTC(0xee18b585),
    WTC(0xeebd6902), WTC(0xef610661), WTC(0xf0037f41), WTC(0xf0a4c139),
    WTC(0xf144c5bc), WTC(0xf1e395c4), WTC(0xf2812c5d), WTC(0xf31d6db4),
    WTC(0xf3b83ed0), WTC(0xf4517963), WTC(0xf4e8d672), WTC(0xf57de495),
    WTC(0xf610395e), WTC(0xf69f7e84), WTC(0xf72b9152), WTC(0xf7b495b5),
    WTC(0xf83af453), WTC(0xf8bf7118), WTC(0xf9429a33), WTC(0xf9c4c183),
    WTC(0xfa466592), WTC(0xfac80867), WTC(0xfb49d2bd), WTC(0xfbcb7294),
    WTC(0xfc4d0e35), WTC(0xfccf7e7b), WTC(0xfd53af97), WTC(0xfddab5ca),
    WTC(0xfe629a56), WTC(0xfee5c7c9), WTC(0xff5b6311), WTC(0xffb6bd45),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbff6c845), WTC(0xbfe461ee), WTC(0xbfd1f586), WTC(0xbfbf85b2),
    WTC(0xbfad1518), WTC(0xbf9aa640), WTC(0xbf883911), WTC(0xbf75caf0),
    WTC(0xbf635c4b), WTC(0xbf50f09b), WTC(0xbf3e886d), WTC(0xbf2c2167),
    WTC(0xbf19bc2c), WTC(0xbf075c64), WTC(0xbef502db), WTC(0xbee2ad83),
    WTC(0xbed05d67), WTC(0xbebe16a7), WTC(0xbeabda46), WTC(0xbe99a62e),
    WTC(0xbe877b90), WTC(0xbe755ee8), WTC(0xbe63517d), WTC(0xbe51515b),
    WTC(0xbe3f5fc9), WTC(0xbe2d8141), WTC(0xbe1bb721), WTC(0xbe09ffa3),
    WTC(0xbdf85c17), WTC(0xbde6d0e3), WTC(0xbdd55f47), WTC(0xbdc4055d),
    WTC(0xbdb2c438), WTC(0xbda19fe0), WTC(0xbd909942), WTC(0xbd7fae2f),
    WTC(0xbd6edf8d), WTC(0xbd5e3153), WTC(0xbd4da45b), WTC(0xbd3d365a),
    WTC(0xbd2ce7eb), WTC(0xbd1cbc87), WTC(0xbd0cb466), WTC(0xbcfccc80),
    WTC(0xbced04c5), WTC(0xbcdd6022), WTC(0xbccdde49), WTC(0xbcbe7bb4),
    WTC(0xbcaf37ec), WTC(0xbca01594), WTC(0xbc91149b), WTC(0xbc823234),
    WTC(0xbc736d81), WTC(0xbc64c7a4), WTC(0xbc564085), WTC(0xbc47d6a9),
    WTC(0xbc398617), WTC(0xbc2b4915), WTC(0xbc1d2a92), WTC(0xbc0f4370),
    WTC(0xbc016785), WTC(0xbbf32f6c), WTC(0xbbe53648), WTC(0xbbd91eb5),
    WTC(0xbbd02f7c), WTC(0xbbcb58db), WTC(0xbbca5ac2), WTC(0xbbcbdea2),
    WTC(0xbbcee192), WTC(0xbbd2c7c2), WTC(0xbbd7a648), WTC(0xbbde3c5a),
    WTC(0xbbe7079f), WTC(0xbbf238bf), WTC(0xbbff8eec), WTC(0xbc0e5d22),
    WTC(0xbc1e2c8b), WTC(0xbc2ec3a4), WTC(0xbc402f19), WTC(0xbc52c1ac),
    WTC(0xbc6709aa), WTC(0xbc7dcbce), WTC(0xbc979383), WTC(0xbcb4ae30),
    WTC(0xbcd52aeb), WTC(0xbcf8db63), WTC(0xbd1f6993), WTC(0xbd485b46),
    WTC(0xbd735007), WTC(0xbda003fc), WTC(0xbdce556c), WTC(0xbdfe453b),
    WTC(0xbe2ffd4e), WTC(0xbe63ceb7), WTC(0xbe9a006f), WTC(0xbed2ce2b),
    WTC(0xbf0e7d8e), WTC(0xbf4d5cfc), WTC(0xbf8f92bd), WTC(0xbfd5160c),
    WTC(0xc01d3b27), WTC(0xc066b8f5), WTC(0xc0b0a3b8), WTC(0xc0fa7cdd),
    WTC(0xc144b0e8), WTC(0xc1908d71), WTC(0xc1ded403), WTC(0xc22fae49),
    WTC(0xc2830092), WTC(0xc2d86ca8), WTC(0xc32f4341), WTC(0xc38687e5),
    WTC(0xc3dd5c55), WTC(0xc43310e4), WTC(0xc4883eca), WTC(0xc4deba3d),
    WTC(0xc5372c82), WTC(0xc59102fb), WTC(0xc5eb416a), WTC(0xc644986f),
    WTC(0xc69cabb2), WTC(0xc6f41290), WTC(0xc74b22bf), WTC(0xc7a1e4b9),
    WTC(0xc7f83500), WTC(0xc84dc737), WTC(0xc8a24e07), WTC(0xc8f5776f),
    WTC(0xb4db4a8b), WTC(0xb3c58a32), WTC(0xb2b2acb4), WTC(0xb1a2afc0),
    WTC(0xb0959107), WTC(0xaf8b4e0a), WTC(0xae83dfef), WTC(0xad7f3ba6),
    WTC(0xac7d5a57), WTC(0xab7e397d), WTC(0xaa81d5a7), WTC(0xa9882a64),
    WTC(0xa89135bc), WTC(0xa79cf84e), WTC(0xa6ab74fc), WTC(0xa5bcb0d0),
    WTC(0xa4d0b1b8), WTC(0xa3e77e6d), WTC(0xa3011e16), WTC(0xa21d986c),
    WTC(0xa13cf921), WTC(0xa05f4fb9), WTC(0x9f84a850), WTC(0x9ead0baf),
    WTC(0x9dd8888c), WTC(0x9d073385), WTC(0x9c391db5), WTC(0x9b6e5484),
    WTC(0x9aa6e656), WTC(0x99e2e2c5), WTC(0x99225ac4), WTC(0x9865608a),
    WTC(0x97ac0564), WTC(0x96f65984), WTC(0x96446a25), WTC(0x95964196),
    WTC(0x94ebe9ec), WTC(0x94456d19), WTC(0x93a2d46a), WTC(0x93042864),
    WTC(0x92696dea), WTC(0x91d2a637), WTC(0x913fd120), WTC(0x90b0ecfe),
    WTC(0x9025f239), WTC(0x8f9ed33e), WTC(0x8f1b804b), WTC(0x8e9be72b),
    WTC(0x8e1fe984), WTC(0x8da75ce4), WTC(0x8d321511), WTC(0x8cbfe381),
    WTC(0x8c508090), WTC(0x8be38b7b), WTC(0x8b78a10a), WTC(0x8b0f5bb4),
    WTC(0x8aa740f3), WTC(0x8a3fc439), WTC(0x89d8a093), WTC(0x8971e7bf),
    WTC(0x890d0fa8), WTC(0x88acfc32), WTC(0x8855e07a), WTC(0x880d5010),
    WTC(0x87cc444c), WTC(0x87b6e84e), WTC(0x879e3713), WTC(0x87850a87),
    WTC(0x876c76ee), WTC(0x8753c55a), WTC(0x873aa70d), WTC(0x872147e7),
    WTC(0x8707bb23), WTC(0x86edf64f), WTC(0x86d3f20d), WTC(0x86b9ab68),
    WTC(0x869f21e7), WTC(0x86845744), WTC(0x8669499e), WTC(0x864df395),
    WTC(0x863254b2), WTC(0x8616715e), WTC(0x85fa4870), WTC(0x85ddd324),
    WTC(0x85c1108b), WTC(0x85a40597), WTC(0x8586b1d1), WTC(0x85690f48),
    WTC(0x854b1dfb), WTC(0x852ce3e7), WTC(0x850e61c8), WTC(0x84ef9303),
    WTC(0x84d078c2), WTC(0x84b119fa), WTC(0x849177fa), WTC(0x84718e56),
    WTC(0x84515e65), WTC(0x8430ef53), WTC(0x841042cb), WTC(0x83ef54e8),
    WTC(0x83ce27ab), WTC(0x83acc30a), WTC(0x838b2934), WTC(0x83695686),
    WTC(0x83474d47), WTC(0x832515b6), WTC(0x8302b202), WTC(0x82e01e3b),
    WTC(0x82bd5c8e), WTC(0x829a755a), WTC(0x82776abf), WTC(0x8254388a),
    WTC(0x8230e07e), WTC(0x820d6a69), WTC(0x81e9d82d), WTC(0x81c625ae),
    WTC(0x81a25455), WTC(0x817e6b1f), WTC(0x815a6b39), WTC(0x81364fec),
    WTC(0x81121a34), WTC(0x80edd0c9), WTC(0x80c97479), WTC(0x80a5000a),
    WTC(0x80807332), WTC(0x805bd2e6), WTC(0x80372460), WTC(0x80126cc4),
    WTC(0x0a8a8431), WTC(0x0ae3c329), WTC(0x0b3fdab2), WTC(0x0b9e6e44),
    WTC(0x0bff2161), WTC(0x0c619a72), WTC(0x0cc5c66d), WTC(0x0d2bd6df),
    WTC(0x0d93cf64), WTC(0x0dfd8545), WTC(0x0e69093a), WTC(0x0ed6a636),
    WTC(0x0f465aea), WTC(0x0fb7d810), WTC(0x102ade99), WTC(0x109f4245),
    WTC(0x1114c9e8), WTC(0x118b31bc), WTC(0x120282b8), WTC(0x127b0be1),
    WTC(0x12f46b9a), WTC(0x136d8dde), WTC(0x13e57912), WTC(0x145b55cb),
    WTC(0x14ce5b18), WTC(0x153dccd0), WTC(0x15a8e724), WTC(0x160edefe),
    WTC(0x166f004f), WTC(0x16c8aff4), WTC(0x171b7afc), WTC(0x17673db3),
    WTC(0x17afb798), WTC(0x17fc62a2), WTC(0x185114c1), WTC(0x18add722),
    WTC(0x1912798d), WTC(0x197eb9a2), WTC(0x19f25657), WTC(0x1a6d113e),
    WTC(0x1aeeb824), WTC(0x1b7723ab), WTC(0x1c060b77), WTC(0x1c9b09a1),
    WTC(0x1d361822), WTC(0x1dd7933f), WTC(0x1e7ff592), WTC(0x1f2fd0b5),
    WTC(0x1fe762a9), WTC(0x20a68700), WTC(0x216bbf5e), WTC(0x22344798),
    WTC(0x22feebdb), WTC(0x23cc22a6), WTC(0x249da10b), WTC(0x25762e41),
    WTC(0x2655ebc8), WTC(0x273a4e66), WTC(0x28212eaa), WTC(0x2908ff30),
    WTC(0x29f2ca00), WTC(0x2ae221c4), WTC(0x2bd9c6fc), WTC(0x2cdb967e),
    WTC(0x2de64c86), WTC(0x2ef61340), WTC(0x3006852b), WTC(0x31131a1b),
    WTC(0x321aec79), WTC(0x3320d4ed), WTC(0x342a2cb3), WTC(0x353e77f7),
    WTC(0x3660aaba), WTC(0x378ef057), WTC(0x38c42495), WTC(0x39f81cec),
    WTC(0x3b250148), WTC(0x3c47960e), WTC(0x3d60c625), WTC(0x3e75864a),
    WTC(0x3f8a9ef7), WTC(0x40a46d36), WTC(0x41c537d7), WTC(0x42ed2889),
    WTC(0x441b52d1), WTC(0x454dc37f), WTC(0x4681e9fa), WTC(0x47b4a9fe),
    WTC(0x48e39960), WTC(0x4a0d10fd), WTC(0x4b30824b), WTC(0x4c4e759e),
    WTC(0x4d680b1c), WTC(0x4e7eecaa), WTC(0x4f947d1e), WTC(0x50a9d1f4),
    WTC(0x51bfee19), WTC(0x52d7be4d), WTC(0x53f187eb), WTC(0x550cd3c8),
    WTC(0x56270706), WTC(0x573b7c75), WTC(0x58474819), WTC(0x59496ae2),
    WTC(0x5a43dfa0), WTC(0x5b3b721f), WTC(0x5c32e266), WTC(0x5d2abea1),
    WTC(0x5e22b479), WTC(0x5f199f8b), WTC(0x600d9194), WTC(0x60fbe188),
    WTC(0x61e289de), WTC(0x62c0520c), WTC(0x63974d1e), WTC(0x646cb022),
    WTC(0x6542745a), WTC(0x66172e2f), WTC(0x66e897d0), WTC(0x67b3cc52),
    WTC(0x68783ebd), WTC(0x6937b9ed), WTC(0x69f365dd), WTC(0x6aabab1a),
    WTC(0x6b60849c), WTC(0x6c11876e), WTC(0x6cbe46f0), WTC(0x6d6645bc),
    WTC(0xae238366), WTC(0xb047d99a), WTC(0xb26207ba), WTC(0xb4733ab5),
    WTC(0xb67c9f5f), WTC(0xb87f5bce), WTC(0xba7bf18d), WTC(0xbc723dd6),
    WTC(0xbe621be8), WTC(0xc04b6b3d), WTC(0xc22e00ff), WTC(0xc409a8ad),
    WTC(0xc5de425b), WTC(0xc7abc2a7), WTC(0xc9721421), WTC(0xcb31173c),
    WTC(0xcce8c08d), WTC(0xce991894), WTC(0xd04218cd), WTC(0xd1e3aba9),
    WTC(0xd37dcf0c), WTC(0xd510942f), WTC(0xd69bfa86), WTC(0xd81fefcc),
    WTC(0xd99c766d), WTC(0xdb11a570), WTC(0xdc7f806e), WTC(0xdde5f79d),
    WTC(0xdf451092), WTC(0xe09ce645), WTC(0xe1ed7fff), WTC(0xe336d16a),
    WTC(0xe478e4f4), WTC(0xe5b3dbbb), WTC(0xe6e7c1ac), WTC(0xe8148d53),
    WTC(0xe93a4835), WTC(0xea590eb0), WTC(0xeb70e4f8), WTC(0xec81b75d),
    WTC(0xed8b8b6c), WTC(0xee8e8068), WTC(0xef8aa970), WTC(0xf0800dfd),
    WTC(0xf16edc4d), WTC(0xf2576853), WTC(0xf339e058), WTC(0xf4164a51),
    WTC(0xf4ec8fb8), WTC(0xf5bc7cef), WTC(0xf685a697), WTC(0xf74771c9),
    WTC(0xf801f31b), WTC(0xf8b5eeb1), WTC(0xf963fadb), WTC(0xfa0c7427),
    WTC(0xfaaf3e0f), WTC(0xfb4bcb6d), WTC(0xfbe2276f), WTC(0xfc72f578),
    WTC(0xfcfe65d9), WTC(0xfd842e5b), WTC(0xfe03e14e), WTC(0xfe7ce07d),
    WTC(0xfeef932b), WTC(0xff5d0236), WTC(0xffc68fee), WTC(0x002dbccc),
    WTC(0x00927c78), WTC(0x00f32cbd), WTC(0x014d7209), WTC(0x019e5f51),
    WTC(0x01e550aa), WTC(0x0223fc07), WTC(0x025d2618), WTC(0x02947b12),
    WTC(0x02cc33db), WTC(0x0304fa92), WTC(0x033db713), WTC(0x0373a431),
    WTC(0x03a47d96), WTC(0x03ce9b0f), WTC(0x03f14dc9), WTC(0x040ce0bc),
    WTC(0x042245a9), WTC(0x043309a5), WTC(0x0440981a), WTC(0x044c30f1),
    WTC(0x0456bb74), WTC(0x0460c1b4), WTC(0x046a2fdd), WTC(0x0472573f),
    WTC(0x047877b6), WTC(0x047bc673), WTC(0x047b8615), WTC(0x04770be2),
    WTC(0x046e23fc), WTC(0x04611460), WTC(0x04507fbd), WTC(0x043d6170),
    WTC(0x0428ca31), WTC(0x0413d39a), WTC(0x03feb9c3), WTC(0x03e8d946),
    WTC(0x03d16667), WTC(0x03b77aba), WTC(0x039aa384), WTC(0x037ae75c),
    WTC(0x0358be80), WTC(0x03350af6), WTC(0x031064fa), WTC(0x02eb13f6),
    WTC(0x02c51a7b), WTC(0x029e38b0), WTC(0x027619ef), WTC(0x024c5c09),
    WTC(0x02210ea3), WTC(0x01f4b19e), WTC(0x01c78f79), WTC(0x0199b8ad),
    WTC(0x016b3aef), WTC(0x013c2366), WTC(0x010c7be8), WTC(0x00dc4bb5),
    WTC(0x00abab29), WTC(0x007ac3e1), WTC(0x0049c02f), WTC(0x0018ca71),
    WTC(0xd6208221), WTC(0xd54e9f5b), WTC(0xd47fa35f), WTC(0xd3b35bdb),
    WTC(0xd2e99693), WTC(0xd2222685), WTC(0xd15d5e4c), WTC(0xd09c06ff),
    WTC(0xcfde0c24), WTC(0xcf2281d2), WTC(0xce698b2a), WTC(0xcdb455e0),
    WTC(0xcd02c9b7), WTC(0xcc5381e5), WTC(0xcba580c3), WTC(0xcaf839ea),
    WTC(0xca4ad0b7), WTC(0xc99c2153), WTC(0xc8ec5a61), WTC(0xc83ce258),
    WTC(0xc78c42cd), WTC(0xc6d6213b), WTC(0xc616a6e0), WTC(0xc54a9f27),
    WTC(0xc46ee44d), WTC(0xc38058a2), WTC(0xc27bd88d), WTC(0xc15e3d07),
    WTC(0xc024a4d9), WTC(0xbecc7ca2), WTC(0xbd53f2b9), WTC(0xbbba98b4),
    WTC(0xba0ffbc3), WTC(0xb872fb24), WTC(0xb6f36547), WTC(0xb5915345),
    WTC(0xb44c3975), WTC(0xb3238942), WTC(0xb216b3fb), WTC(0xb1252b0b),
    WTC(0xb04e5fc8), WTC(0xaf91c370), WTC(0xaeeec760), WTC(0xae64dd0b),
    WTC(0xadf375ce), WTC(0xad9a02f0), WTC(0xad57f5bc), WTC(0xad2cbf87),
    WTC(0xad17d19d), WTC(0xad189d42), WTC(0xad2e93d6), WTC(0xad5926d3),
    WTC(0xad97c78a), WTC(0xade9e726), WTC(0xae4ef6fc), WTC(0xaec6688c),
    WTC(0xaf4fad2c), WTC(0xafea3605), WTC(0xb0957469), WTC(0xb150d9d2),
    WTC(0xb21bd793), WTC(0xb2f5de5d), WTC(0xb3de573d), WTC(0xb4d4de47),
    WTC(0xb5d89c80), WTC(0xb6e937c2), WTC(0xb8061354), WTC(0xb92ea07c),
    WTC(0xba62508e), WTC(0xbba094e4), WTC(0xbce8dee0), WTC(0xbe3a9fe3),
    WTC(0xbf954939), WTC(0xc0f84c11), WTC(0xc26319c2), WTC(0xc3d523c5),
    WTC(0xc54ddb77), WTC(0xc6ccb217), WTC(0xc85118f8), WTC(0xc9da8182),
    WTC(0xcb685d07), WTC(0xccfa1cc7), WTC(0xce8f3211), WTC(0xd0270e48),
    WTC(0xd1c122c7), WTC(0xd35ce0de), WTC(0xd4f9b9e1), WTC(0xd6971f23),
    WTC(0xd83481f8), WTC(0xd9d153bb), WTC(0xdb6d05c0), WTC(0xdd070956),
    WTC(0xde9ecfd2), WTC(0xe033ca91), WTC(0xe1c56ae7), WTC(0xe3532223),
    WTC(0xe4dc6199), WTC(0xe6609aa5), WTC(0xe7df3e9a), WTC(0xe957bec9),
    WTC(0xeac98c84), WTC(0xec341927), WTC(0xed96d607), WTC(0xeef13474),
    WTC(0xf042a5c5), WTC(0xf18a9b4e), WTC(0xf2c88667), WTC(0xf3fbd863),
    WTC(0xf5240296), WTC(0xf6407658), WTC(0xf750a4fe), WTC(0xf853ffda),
    WTC(0xf949f840), WTC(0xfa31ff83), WTC(0xfb0b86fb), WTC(0xfbd5fffe),
    WTC(0xfc90dbe1), WTC(0xfd3b8bf8), WTC(0xfdd58197), WTC(0xfe5e2e14),
    WTC(0xfed502c4), WTC(0xff3970fc), WTC(0xff8aea0f), WTC(0xffc8df55),
    WTC(0xfff2c233), WTC(0x000804e6), WTC(0x0008256a), WTC(0xfff25358),
};

const FIXP_WTB LowDelaySynthesis240[720] = {
    WTC(0xdaf16ba1), WTC(0xdb888d4d), WTC(0xdc212bc1), WTC(0xdcbb51d0),
    WTC(0xdd5703be), WTC(0xddf43f3b), WTC(0xde92fee5), WTC(0xdf333f92),
    WTC(0xdfd505d1), WTC(0xe078624a), WTC(0xe11d48ff), WTC(0xe1c39358),
    WTC(0xe26b2066), WTC(0xe313d8a9), WTC(0xe3bde63c), WTC(0xe4696e45),
    WTC(0xe5164d95), WTC(0xe5c4597d), WTC(0xe673596f), WTC(0xe723153c),
    WTC(0xe7d35813), WTC(0xe883fa49), WTC(0xe934e7a3), WTC(0xe9e64205),
    WTC(0xea984aa8), WTC(0xeb4ae6ef), WTC(0xebfdcbf6), WTC(0xecb0744b),
    WTC(0xed625671), WTC(0xee133361), WTC(0xeec2e1b4), WTC(0xef715334),
    WTC(0xf01e7588), WTC(0xf0ca33ec), WTC(0xf1748a4e), WTC(0xf21d83ed),
    WTC(0xf2c50dbe), WTC(0xf36b0639), WTC(0xf40f4892), WTC(0xf4b1944d),
    WTC(0xf5517289), WTC(0xf5ee573d), WTC(0xf687d70d), WTC(0xf71db368),
    WTC(0xf7b01057), WTC(0xf83f6570), WTC(0xf8cc9d0a), WTC(0xf9585a73),
    WTC(0xf9e307f8), WTC(0xfa6d44d2), WTC(0xfaf79d75), WTC(0xfb8208e8),
    WTC(0xfc0c33c8), WTC(0xfc96d00c), WTC(0xfd22f257), WTC(0xfdb1e623),
    WTC(0xfe4318a3), WTC(0xfed08c1d), WTC(0xff5091f1), WTC(0xffb4390a),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbff62b62), WTC(0xbfe28a84), WTC(0xbfcee30d), WTC(0xbfbb3832),
    WTC(0xbfa78d29), WTC(0xbf93e490), WTC(0xbf803cc6), WTC(0xbf6c9376),
    WTC(0xbf58eb89), WTC(0xbf4547f8), WTC(0xbf31a6aa), WTC(0xbf1e06a3),
    WTC(0xbf0a6bdf), WTC(0xbef6d863), WTC(0xbee349e5), WTC(0xbecfc145),
    WTC(0xbebc4362), WTC(0xbea8d111), WTC(0xbe956806), WTC(0xbe820aeb),
    WTC(0xbe6ebeb9), WTC(0xbe5b8313), WTC(0xbe485678), WTC(0xbe353cfb),
    WTC(0xbe223ab8), WTC(0xbe0f4e5f), WTC(0xbdfc77b4), WTC(0xbde9bb99),
    WTC(0xbdd71cbe), WTC(0xbdc4990d), WTC(0xbdb23175), WTC(0xbd9feab6),
    WTC(0xbd8dc58d), WTC(0xbd7bbf8c), WTC(0xbd69daec), WTC(0xbd581c07),
    WTC(0xbd46820c), WTC(0xbd350af8), WTC(0xbd23b9cc), WTC(0xbd129158),
    WTC(0xbd018ece), WTC(0xbcf0b057), WTC(0xbcdff912), WTC(0xbccf69cf),
    WTC(0xbcbefe81), WTC(0xbcaeb63a), WTC(0xbc9e9406), WTC(0xbc8e977c),
    WTC(0xbc7ebd58), WTC(0xbc6f0544), WTC(0xbc5f7077), WTC(0xbc4ffe1d),
    WTC(0xbc40ab99), WTC(0xbc317239), WTC(0xbc2252ad), WTC(0xbc136963),
    WTC(0xbc04a822), WTC(0xbbf5941e), WTC(0xbbe68dab), WTC(0xbbd97a36),
    WTC(0xbbcff44f), WTC(0xbbcb1891), WTC(0xbbca7818), WTC(0xbbcc765f),
    WTC(0xbbcffaa5), WTC(0xbbd46a0f), WTC(0xbbda4060), WTC(0xbbe257f1),
    WTC(0xbbed131c), WTC(0xbbfa7628), WTC(0xbc09cdd6), WTC(0xbc1a685b),
    WTC(0xbc2bf2fd), WTC(0xbc3e6557), WTC(0xbc521d37), WTC(0xbc67c093),
    WTC(0xbc803b98), WTC(0xbc9c3060), WTC(0xbcbbf6da), WTC(0xbcdf8c51),
    WTC(0xbd06afb5), WTC(0xbd30e36d), WTC(0xbd5d99a0), WTC(0xbd8c71a6),
    WTC(0xbdbd29c3), WTC(0xbdefb72a), WTC(0xbe243660), WTC(0xbe5b0335),
    WTC(0xbe9477fd), WTC(0xbed0de00), WTC(0xbf108881), WTC(0xbf53d585),
    WTC(0xbf9aeeba), WTC(0xbfe5bb38), WTC(0xc033318c), WTC(0xc081cdf2),
    WTC(0xc0d0a9ee), WTC(0xc11f76d5), WTC(0xc16f66c3), WTC(0xc1c1d5c1),
    WTC(0xc21726d4), WTC(0xc26f5bba), WTC(0xc2ca1036), WTC(0xc3268b0f),
    WTC(0xc3839ff3), WTC(0xc3e03d22), WTC(0xc43b93ef), WTC(0xc4968594),
    WTC(0xc4f3353c), WTC(0xc55202ab), WTC(0xc5b21fcf), WTC(0xc61224e6),
    WTC(0xc670c168), WTC(0xc6ce3f1e), WTC(0xc72b3fbb), WTC(0xc787e688),
    WTC(0xc7e41f56), WTC(0xc83f94a9), WTC(0xc899e833), WTC(0xc8f2b832),
    WTC(0xb4d1fc78), WTC(0xb3a9ec70), WTC(0xb28524ca), WTC(0xb163a2ba),
    WTC(0xb0456373), WTC(0xaf2a6327), WTC(0xae129708), WTC(0xacfdf2ca),
    WTC(0xabec7168), WTC(0xaade0f8c), WTC(0xa9d2c80a), WTC(0xa8ca9711),
    WTC(0xa7c57cdc), WTC(0xa6c37c29), WTC(0xa5c49ae6), WTC(0xa4c8e02c),
    WTC(0xa3d05422), WTC(0xa2daff83), WTC(0xa1e8ec04), WTC(0xa0fa293f),
    WTC(0xa00ec98a), WTC(0x9f26d990), WTC(0x9e4265a5), WTC(0x9d618437),
    WTC(0x9c844ce0), WTC(0x9baad0fa), WTC(0x9ad52154), WTC(0x9a03509b),
    WTC(0x993572ed), WTC(0x986b9e4d), WTC(0x97a5e7d8), WTC(0x96e46329),
    WTC(0x96271ff8), WTC(0x956e2ab8), WTC(0x94b98f9c), WTC(0x94095a97),
    WTC(0x935d969b), WTC(0x92b64cdf), WTC(0x9213809b), WTC(0x9175328a),
    WTC(0x90db6153), WTC(0x90460712), WTC(0x8fb5146f), WTC(0x8f2876f4),
    WTC(0x8ea018e5), WTC(0x8e1bd6df), WTC(0x8d9b7d9b), WTC(0x8d1ed745),
    WTC(0x8ca5a942), WTC(0x8c2f93ce), WTC(0x8bbc204c), WTC(0x8b4ad58d),
    WTC(0x8adb31e4), WTC(0x8a6c909b), WTC(0x89fe673d), WTC(0x8990a478),
    WTC(0x89244670), WTC(0x88bc84cb), WTC(0x885e0963), WTC(0x880f73f5),
    WTC(0x87cba2c0), WTC(0x87b48a6f), WTC(0x8799fb25), WTC(0x877f46f2),
    WTC(0x876519cd), WTC(0x874a9a11), WTC(0x872fae01), WTC(0x871487d2),
    WTC(0x86f9287b), WTC(0x86dd83b5), WTC(0x86c1947c), WTC(0x86a558f1),
    WTC(0x8688d2e3), WTC(0x866c015b), WTC(0x864ede0c), WTC(0x863167cc),
    WTC(0x8613a3b8), WTC(0x85f58fb2), WTC(0x85d723f0), WTC(0x85b86176),
    WTC(0x85994d82), WTC(0x8579e443), WTC(0x855a201b), WTC(0x853a05b7),
    WTC(0x85199a26), WTC(0x84f8d93f), WTC(0x84d7c100), WTC(0x84b65901),
    WTC(0x8494a4ee), WTC(0x84729fd4), WTC(0x84504a9b), WTC(0x842dadb3),
    WTC(0x840aca71), WTC(0x83e79c87), WTC(0x83c42892), WTC(0x83a07767),
    WTC(0x837c883a), WTC(0x83585835), WTC(0x8333eeca), WTC(0x830f5368),
    WTC(0x82ea82ec), WTC(0x82c57c57), WTC(0x82a048ea), WTC(0x827aed86),
    WTC(0x82556588), WTC(0x822fb255), WTC(0x8209dd1e), WTC(0x81e3e76f),
    WTC(0x81bdccac), WTC(0x81979098), WTC(0x81713ad7), WTC(0x814ac95d),
    WTC(0x812437f2), WTC(0x80fd8c4b), WTC(0x80d6cc0a), WTC(0x80aff283),
    WTC(0x8088fc64), WTC(0x8061eebe), WTC(0x803acfe9), WTC(0x8013a62d),
    WTC(0x0a8d710a), WTC(0x0aecd974), WTC(0x0b4f7449), WTC(0x0bb4d13b),
    WTC(0x0c1c7fee), WTC(0x0c86202a), WTC(0x0cf1c2e7), WTC(0x0d5f98d1),
    WTC(0x0dcf816a), WTC(0x0e4162ae), WTC(0x0eb588ad), WTC(0x0f2c1e88),
    WTC(0x0fa4d14f), WTC(0x101f4d80), WTC(0x109b5bfd), WTC(0x1118b908),
    WTC(0x11971466), WTC(0x12168249), WTC(0x129754ab), WTC(0x1318d5c0),
    WTC(0x1399b5d7), WTC(0x1418d8ec), WTC(0x14953f24), WTC(0x150dfe8a),
    WTC(0x15822faa), WTC(0x15f0dd70), WTC(0x16592014), WTC(0x16ba35b2),
    WTC(0x171384e7), WTC(0x1764cf1f), WTC(0x17b22a28), WTC(0x18047d40),
    WTC(0x185ffc05), WTC(0x18c4a075), WTC(0x19322980), WTC(0x19a84643),
    WTC(0x1a26a8f4), WTC(0x1aad099d), WTC(0x1b3b3493), WTC(0x1bd0eb32),
    WTC(0x1c6db754), WTC(0x1d115a8c), WTC(0x1dbc329b), WTC(0x1e6ecb33),
    WTC(0x1f29d42a), WTC(0x1feda2e4), WTC(0x20ba056e), WTC(0x218d02a2),
    WTC(0x22635c75), WTC(0x233c2e7f), WTC(0x24184da1), WTC(0x24fa799d),
    WTC(0x25e54e95), WTC(0x26d6edd8), WTC(0x27cc57a5), WTC(0x28c36349),
    WTC(0x29bbdfdf), WTC(0x2ab9b7c2), WTC(0x2bc09790), WTC(0x2cd2d465),
    WTC(0x2def4cb9), WTC(0x2f115dcc), WTC(0x3033a99c), WTC(0x3150fd8d),
    WTC(0x32698a94), WTC(0x338152e6), WTC(0x34a02dcd), WTC(0x35cdedd1),
    WTC(0x370aa9fa), WTC(0x38527a79), WTC(0x399c4a4f), WTC(0x3adf9c27),
    WTC(0x3c17edc7), WTC(0x3d44f05d), WTC(0x3e6c519e), WTC(0x3f93ea5b),
    WTC(0x40c0fc6d), WTC(0x41f60bdd), WTC(0x43332d15), WTC(0x4476e6ea),
    WTC(0x45beb7e1), WTC(0x47072f2b), WTC(0x484cb71a), WTC(0x498ceafd),
    WTC(0x4ac650c3), WTC(0x4bf93458), WTC(0x4d26a4c7), WTC(0x4e5090eb),
    WTC(0x4f78c2ac), WTC(0x50a0918a), WTC(0x51c93995), WTC(0x52f3d6c7),
    WTC(0x5420a9a2), WTC(0x554ef122), WTC(0x567ac4a9), WTC(0x579ebeb9),
    WTC(0x58b8569c), WTC(0x59c74ea4), WTC(0x5ad03f31), WTC(0x5bd821c8),
    WTC(0x5ce05502), WTC(0x5de8e582), WTC(0x5ef09b49), WTC(0x5ff56247),
    WTC(0x60f40d81), WTC(0x61ea1450), WTC(0x62d60a2d), WTC(0x63bad7f1),
    WTC(0x649e942e), WTC(0x65827556), WTC(0x66648178), WTC(0x67418c31),
    WTC(0x6816c1ee), WTC(0x68e5411d), WTC(0x69aeffdb), WTC(0x6a74bb0e),
    WTC(0x6b36a5ae), WTC(0x6bf44fd1), WTC(0x6cad341f), WTC(0x6d60c0ca),
    WTC(0xae35f79b), WTC(0xb07e1b0d), WTC(0xb2bad26d), WTC(0xb4ed8af7),
    WTC(0xb717b207), WTC(0xb93a8f5b), WTC(0xbb56631e), WTC(0xbd6afcab),
    WTC(0xbf7832db), WTC(0xc17dd996), WTC(0xc37bb355), WTC(0xc5718d72),
    WTC(0xc75f56cc), WTC(0xc944f93d), WTC(0xcb224f17), WTC(0xccf74805),
    WTC(0xcec3ed8c), WTC(0xd08835d9), WTC(0xd2440837), WTC(0xd3f7692d),
    WTC(0xd5a26b17), WTC(0xd74502b6), WTC(0xd8df1f82), WTC(0xda70d495),
    WTC(0xdbfa35a1), WTC(0xdd7b3498), WTC(0xdef3cba3), WTC(0xe064184e),
    WTC(0xe1cc2ab9), WTC(0xe32bf548), WTC(0xe48381cd), WTC(0xe5d2f779),
    WTC(0xe71a6218), WTC(0xe859b789), WTC(0xe9910a60), WTC(0xeac07956),
    WTC(0xebe7fb55), WTC(0xed077f41), WTC(0xee1f1f9d), WTC(0xef2efd99),
    WTC(0xf0372472), WTC(0xf137b605), WTC(0xf23112b3), WTC(0xf323808a),
    WTC(0xf40f0a86), WTC(0xf4f39883), WTC(0xf5d0eb3c), WTC(0xf6a679c6),
    WTC(0xf7739640), WTC(0xf8389849), WTC(0xf8f66b2a), WTC(0xf9ada7d6),
    WTC(0xfa5e97ed), WTC(0xfb08becd), WTC(0xfbabb380), WTC(0xfc48125f),
    WTC(0xfcde5b40), WTC(0xfd6e4aac), WTC(0xfdf76560), WTC(0xfe78f3ab),
    WTC(0xfef34cf0), WTC(0xff67b689), WTC(0xffd7e342), WTC(0x004584ef),
    WTC(0x00b00074), WTC(0x01152d47), WTC(0x0171ccea), WTC(0x01c2fdbe),
    WTC(0x0209b563), WTC(0x02489832), WTC(0x0283e2cd), WTC(0x02bf2050),
    WTC(0x02fb72fb), WTC(0x03381f29), WTC(0x0371ea7c), WTC(0x03a602f3),
    WTC(0x03d267e0), WTC(0x03f6621b), WTC(0x04125dc8), WTC(0x0427b7a5),
    WTC(0x04385062), WTC(0x0445caca), WTC(0x04518ee3), WTC(0x045c745e),
    WTC(0x0466d714), WTC(0x0470125c), WTC(0x047742d7), WTC(0x047b73cf),
    WTC(0x047bba82), WTC(0x047744aa), WTC(0x046dc536), WTC(0x045f9068),
    WTC(0x044d7632), WTC(0x0438aa92), WTC(0x04227f39), WTC(0x040c2331),
    WTC(0x03f561d8), WTC(0x03dd60b4), WTC(0x03c31064), WTC(0x03a58d7d),
    WTC(0x0384b74f), WTC(0x0360e29a), WTC(0x033b1151), WTC(0x031417f0),
    WTC(0x02ec54ca), WTC(0x02c3d2ce), WTC(0x029a458d), WTC(0x026f4411),
    WTC(0x02426093), WTC(0x0213d67f), WTC(0x01e43b01), WTC(0x01b3c7c1),
    WTC(0x01828dd8), WTC(0x01509d92), WTC(0x011e0556), WTC(0x00eace1d),
    WTC(0x00b70bb3), WTC(0x0082ed8b), WTC(0x004ea707), WTC(0x001a6b8f),
    WTC(0xd619769b), WTC(0xd539cbf6), WTC(0xd45d68b9), WTC(0xd3840fce),
    WTC(0xd2ad840b), WTC(0xd1d9a580), WTC(0xd1092160), WTC(0xd03cacdc),
    WTC(0xcf7383ba), WTC(0xceacfd46), WTC(0xcdea429a), WTC(0xcd2bf5ec),
    WTC(0xcc709dc1), WTC(0xcbb6dc91), WTC(0xcafdff71), WTC(0xca4504cd),
    WTC(0xc98a93f4), WTC(0xc8cf0eb0), WTC(0xc813ee7f), WTC(0xc75665f3),
    WTC(0xc6912d63), WTC(0xc5bff74a), WTC(0xc4dee9fa), WTC(0xc3ea39ef),
    WTC(0xc2de1c94), WTC(0xc1b6bd08), WTC(0xc07074da), WTC(0xbf081038),
    WTC(0xbd7b166b), WTC(0xbbc8afd7), WTC(0xba01d47c), WTC(0xb84b481f),
    WTC(0xb6b65953), WTC(0xb542e5d2), WTC(0xb3f04291), WTC(0xb2bdc25a),
    WTC(0xb1aab810), WTC(0xb0b676a0), WTC(0xafe050d5), WTC(0xaf27997d),
    WTC(0xae8ba390), WTC(0xae0bc202), WTC(0xada7479d), WTC(0xad5d872f),
    WTC(0xad2dd392), WTC(0xad177f97), WTC(0xad19de04), WTC(0xad3441c5),
    WTC(0xad65fddc), WTC(0xadae6514), WTC(0xae0cca18), WTC(0xae807fde),
    WTC(0xaf08d967), WTC(0xafa5296f), WTC(0xb054c2ac), WTC(0xb116f81b),
    WTC(0xb1eb1cb1), WTC(0xb2d082fe), WTC(0xb3c677e5), WTC(0xb4cc6e9c),
    WTC(0xb5e17eb4), WTC(0xb7052956), WTC(0xb836b427), WTC(0xb97571f3),
    WTC(0xbac0b594), WTC(0xbc17d1ee), WTC(0xbd7a19eb), WTC(0xbee6e071),
    WTC(0xc05d7837), WTC(0xc1dd33fe), WTC(0xc36566c2), WTC(0xc4f56377),
    WTC(0xc68c7ce5), WTC(0xc82a05db), WTC(0xc9cd5148), WTC(0xcb75b207),
    WTC(0xcd227ad9), WTC(0xced2fe95), WTC(0xd0869026), WTC(0xd23c8268),
    WTC(0xd3f42834), WTC(0xd5acd460), WTC(0xd765d9c5), WTC(0xd91e8b42),
    WTC(0xdad63bb5), WTC(0xdc8c3df2), WTC(0xde3fe4d1), WTC(0xdff08333),
    WTC(0xe19d6bf6), WTC(0xe345f1ee), WTC(0xe4e967f3), WTC(0xe68720e7),
    WTC(0xe81e6fa3), WTC(0xe9aea6fb), WTC(0xeb3719cb), WTC(0xecb71af3),
    WTC(0xee2dfd4b), WTC(0xef9b13ab), WTC(0xf0fdb0ee), WTC(0xf25527f1),
    WTC(0xf3a0cb8e), WTC(0xf4dfee9f), WTC(0xf611e3ff), WTC(0xf735fe8b),
    WTC(0xf84b911a), WTC(0xf951ee85), WTC(0xfa4869a5), WTC(0xfb2e5557),
    WTC(0xfc030477), WTC(0xfcc5c9e0), WTC(0xfd75f86b), WTC(0xfe12e2f2),
    WTC(0xfe9bdc51), WTC(0xff103761), WTC(0xff6f46fc), WTC(0xffb85dfe),
    WTC(0xffeacf42), WTC(0x0005ee03), WTC(0x0009162b), WTC(0xfff368d1),
};

const FIXP_WTB LowDelaySynthesis160[480] = {
    WTC(0xdb171130), WTC(0xdbfadfbd), WTC(0xdce2192a), WTC(0xddcccbc8),
    WTC(0xdebaeb0c), WTC(0xdfac6ebd), WTC(0xe0a17875), WTC(0xe199e10c),
    WTC(0xe29531fd), WTC(0xe3933ef6), WTC(0xe49487be), WTC(0xe598bcf5),
    WTC(0xe69f38b0), WTC(0xe7a73d45), WTC(0xe8b02e94), WTC(0xe9b9dc97),
    WTC(0xeac4e62c), WTC(0xebd111fc), WTC(0xecdd0242), WTC(0xede7178d),
    WTC(0xeeee9c24), WTC(0xeff34dba), WTC(0xf0f4eaf5), WTC(0xf1f36695),
    WTC(0xf2eeb2b2), WTC(0xf3e663cb), WTC(0xf4d9cbfe), WTC(0xf5c76b3c),
    WTC(0xf6ada4cb), WTC(0xf78bc92d), WTC(0xf862dc57), WTC(0xf93589ca),
    WTC(0xfa059b44), WTC(0xfad501fc), WTC(0xfba49819), WTC(0xfc740f58),
    WTC(0xfd465b6d), WTC(0xfe1ee06f), WTC(0xfef2581b), WTC(0xff9ec7d9),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbff143f6), WTC(0xbfd3cd5c), WTC(0xbfb64d54), WTC(0xbf98ce80),
    WTC(0xbf7b5291), WTC(0xbf5dd523), WTC(0xbf405f8e), WTC(0xbf22ee55),
    WTC(0xbf058664), WTC(0xbee82d22), WTC(0xbecae0a1), WTC(0xbeadacb5),
    WTC(0xbe908f68), WTC(0xbe73901d), WTC(0xbe56b673), WTC(0xbe3a013e),
    WTC(0xbe1d7dad), WTC(0xbe012b19), WTC(0xbde51134), WTC(0xbdc93781),
    WTC(0xbdad9c88), WTC(0xbd924bd8), WTC(0xbd774311), WTC(0xbd5c882b),
    WTC(0xbd4220fa), WTC(0xbd280a49), WTC(0xbd0e4d50), WTC(0xbcf4e46e),
    WTC(0xbcdbd1a4), WTC(0xbcc31624), WTC(0xbcaaaa02), WTC(0xbc929348),
    WTC(0xbc7acc12), WTC(0xbc63525e), WTC(0xbc4c26b1), WTC(0xbc353ea9),
    WTC(0xbc1e91e2), WTC(0xbc085aea), WTC(0xbbf1c04a), WTC(0xbbdc7521),
    WTC(0xbbce4740), WTC(0xbbca5187), WTC(0xbbcd3a7c), WTC(0xbbd33634),
    WTC(0xbbdc0a34), WTC(0xbbea218a), WTC(0xbbfe25b7), WTC(0xbc162887),
    WTC(0xbc3076c5), WTC(0xbc4d09f6), WTC(0xbc6d925c), WTC(0xbc94da6e),
    WTC(0xbcc48300), WTC(0xbcfc9763), WTC(0xbd3bd94f), WTC(0xbd808d05),
    WTC(0xbdc9a11b), WTC(0xbe16e339), WTC(0xbe691db9), WTC(0xbec179c1),
    WTC(0xbf210140), WTC(0xbf88cd62), WTC(0xbff8e8d3), WTC(0xc06e1aaa),
    WTC(0xc0e45951), WTC(0xc15b3820), WTC(0xc1d6e2ff), WTC(0xc2590e0a),
    WTC(0xc2e10f83), WTC(0xc36c5c9a), WTC(0xc3f735f3), WTC(0xc47fb2d1),
    WTC(0xc50abce5), WTC(0xc59a0a25), WTC(0xc629f21c), WTC(0xc6b6ef89),
    WTC(0xc7427180), WTC(0xc7cd1fc4), WTC(0xc856485f), WTC(0xc8dcae93),
    WTC(0xb487a986), WTC(0xb2ce0812), WTC(0xb11bc4c2), WTC(0xaf70d5cb),
    WTC(0xadcd228e), WTC(0xac3086a2), WTC(0xaa9af36b), WTC(0xa90c5935),
    WTC(0xa784b214), WTC(0xa60407e9), WTC(0xa48a7076), WTC(0xa31806f9),
    WTC(0xa1aceb03), WTC(0xa0494f63), WTC(0x9eed6840), WTC(0x9d996570),
    WTC(0x9c4d93b4), WTC(0x9b0a3114), WTC(0x99cf798d), WTC(0x989db16a),
    WTC(0x97752146), WTC(0x965609f1), WTC(0x95409b05), WTC(0x9434fda9),
    WTC(0x9333587b), WTC(0x923bc7d6), WTC(0x914e5299), WTC(0x906af345),
    WTC(0x8f9185e6), WTC(0x8ec1cbe9), WTC(0x8dfb64c9), WTC(0x8d3dab18),
    WTC(0x8c87de34), WTC(0x8bd8c494), WTC(0x8b2ecacc), WTC(0x8a882a5f),
    WTC(0x89e2ecd1), WTC(0x893f12b2), WTC(0x88a3c878), WTC(0x882141c7),
    WTC(0x87c65dcd), WTC(0x87a0c07b), WTC(0x8778b859), WTC(0x87514698),
    WTC(0x8728e900), WTC(0x87000679), WTC(0x86d68f05), WTC(0x86ac6ee7),
    WTC(0x8681a5ce), WTC(0x86562ed0), WTC(0x8629fdeb), WTC(0x85fd1ca8),
    WTC(0x85cf7b32), WTC(0x85a11a2f), WTC(0x8571fbbe), WTC(0x854213f5),
    WTC(0x8511722b), WTC(0x84e00ee3), WTC(0x84adf346), WTC(0x847b28e5),
    WTC(0x8447a9d0), WTC(0x84138a20), WTC(0x83dec5b6), WTC(0x83a9694f),
    WTC(0x83738231), WTC(0x833d0def), WTC(0x8306247b), WTC(0x82cec29b),
    WTC(0x8296f5f3), WTC(0x825ecbe4), WTC(0x82263fe8), WTC(0x81ed682f),
    WTC(0x81b4406d), WTC(0x817ad2a1), WTC(0x814127f4), WTC(0x8107392d),
    WTC(0x80cd184d), WTC(0x8092bc5f), WTC(0x80582866), WTC(0x801d714f),
    WTC(0x0aa4f846), WTC(0x0b3686fe), WTC(0x0bce899e), WTC(0x0c6b8a7e),
    WTC(0x0d0d036f), WTC(0x0db358b8), WTC(0x0e5e31dd), WTC(0x0f0e4270),
    WTC(0x0fc347c1), WTC(0x107c35cc), WTC(0x11383a63), WTC(0x11f68759),
    WTC(0x12b7b1e3), WTC(0x13799d61), WTC(0x14383db4), WTC(0x14f032c0),
    WTC(0x159e6920), WTC(0x163fb449), WTC(0x16d14820), WTC(0x17512c3f),
    WTC(0x17c5f622), WTC(0x18483f2a), WTC(0x18df3074), WTC(0x1989f589),
    WTC(0x1a4783af), WTC(0x1b16f152), WTC(0x1bf7795a), WTC(0x1ce7c950),
    WTC(0x1de817ec), WTC(0x1efa3f4a), WTC(0x201ff37a), WTC(0x2157d4e1),
    WTC(0x22995036), WTC(0x23e0d882), WTC(0x25345db2), WTC(0x269a10c3),
    WTC(0x280a0798), WTC(0x297d6cf9), WTC(0x2afa7e1b), WTC(0x2c8d210a),
    WTC(0x2e377db2), WTC(0x2feb60cc), WTC(0x31977111), WTC(0x333b1637),
    WTC(0x34ea14df), WTC(0x36ba32d4), WTC(0x38a524c8), WTC(0x3a8fa891),
    WTC(0x3c640cb3), WTC(0x3e22ab11), WTC(0x3fde7cc9), WTC(0x41a80486),
    WTC(0x438394e4), WTC(0x456c8d8f), WTC(0x4758f4da), WTC(0x493d7a7b),
    WTC(0x4b139f32), WTC(0x4cdbb199), WTC(0x4e9ab8d1), WTC(0x50569530),
    WTC(0x5213a89e), WTC(0x53d5462c), WTC(0x559a5aa7), WTC(0x5756acf5),
    WTC(0x58fcfb38), WTC(0x5a8e3e07), WTC(0x5c1a20e8), WTC(0x5da6c7da),
    WTC(0x5f3231f3), WTC(0x60b51fb6), WTC(0x62260848), WTC(0x6381f5a9),
    WTC(0x64d79ac7), WTC(0x662c51c0), WTC(0x67779c07), WTC(0x68b22184),
    WTC(0x69e0ca28), WTC(0x6b068bcf), WTC(0x6c23008e), WTC(0x6d346856),
    WTC(0xaec92693), WTC(0xb22ca2bd), WTC(0xb578d421), WTC(0xb8b27edb),
    WTC(0xbbdc38b5), WTC(0xbef598bb), WTC(0xc1fe0dcd), WTC(0xc4f4d7ff),
    WTC(0xc7d98479), WTC(0xcaabc289), WTC(0xcd6b38f6), WTC(0xd017edff),
    WTC(0xd2b1aa37), WTC(0xd538739f), WTC(0xd7ac554d), WTC(0xda0d2f5e),
    WTC(0xdc5b3f69), WTC(0xde966e30), WTC(0xe0bee2c8), WTC(0xe2d4c9cd),
    WTC(0xe4d82000), WTC(0xe6c94926), WTC(0xe8a84b14), WTC(0xea755ac3),
    WTC(0xec309bdc), WTC(0xedd9f2a8), WTC(0xef71c040), WTC(0xf0f842ad),
    WTC(0xf26e53cb), WTC(0xf3d4cd9f), WTC(0xf52b9ddf), WTC(0xf671db4d),
    WTC(0xf7a58faa), WTC(0xf8c79907), WTC(0xf9da7c73), WTC(0xfadee240),
    WTC(0xfbd360e6), WTC(0xfcb95db4), WTC(0xfd913bb1), WTC(0xfe594e2e),
    WTC(0xff10e67c), WTC(0xffbc2326), WTC(0x00608350), WTC(0x00fc8a2d),
    WTC(0x01873313), WTC(0x01f8e4fe), WTC(0x02579318), WTC(0x02b03ec3),
    WTC(0x030ab2da), WTC(0x0363ea2a), WTC(0x03b1e60d), WTC(0x03ee2920),
    WTC(0x0418405c), WTC(0x04348449), WTC(0x0448d9f7), WTC(0x0459c65f),
    WTC(0x04694a29), WTC(0x0475b506), WTC(0x047bebeb), WTC(0x0478dcd2),
    WTC(0x046aa475), WTC(0x045249a9), WTC(0x043332f0), WTC(0x0411bb77),
    WTC(0x03ef893b), WTC(0x03c9ebb8), WTC(0x039da778), WTC(0x036a1273),
    WTC(0x03316a60), WTC(0x02f6553a), WTC(0x02b98be9), WTC(0x027a2e2c),
    WTC(0x0236df64), WTC(0x01f036bf), WTC(0x01a78b41), WTC(0x015d29da),
    WTC(0x01114624), WTC(0x00c406e9), WTC(0x0075ddb1), WTC(0x00277692),
    WTC(0xd5e139e9), WTC(0xd494362e), WTC(0xd34e2bff), WTC(0xd20e56f8),
    WTC(0xd0d5a3dc), WTC(0xcfa5904f), WTC(0xce7be3cb), WTC(0xcd5b3086),
    WTC(0xcc420f0f), WTC(0xcb2c2bf6), WTC(0xca1695c3), WTC(0xc8fdf020),
    WTC(0xc7e4fc07), WTC(0xc6c374c6), WTC(0xc5895653), WTC(0xc4297218),
    WTC(0xc296f958), WTC(0xc0c51a3c), WTC(0xbea84ee5), WTC(0xbc38842f),
    WTC(0xb9915966), WTC(0xb7186ae5), WTC(0xb4eb3040), WTC(0xb3076897),
    WTC(0xb16acba3), WTC(0xb013112a), WTC(0xaefdf0a2), WTC(0xae2921f5),
    WTC(0xad925cd6), WTC(0xad3758be), WTC(0xad15cd3d), WTC(0xad2b71ca),
    WTC(0xad75fe65), WTC(0xadf32a84), WTC(0xaea0ada9), WTC(0xaf7c3fcd),
    WTC(0xb0839825), WTC(0xb1b46e9c), WTC(0xb30c7a20), WTC(0xb48970be),
    WTC(0xb62912e0), WTC(0xb7e90de7), WTC(0xb9c71d12), WTC(0xbbc0f7fd),
    WTC(0xbdd45674), WTC(0xbffef022), WTC(0xc23e7c49), WTC(0xc490b2d4),
    WTC(0xc6f34b70), WTC(0xc963fda9), WTC(0xcbe0813e), WTC(0xce668d98),
    WTC(0xd0f3da69), WTC(0xd3861f64), WTC(0xd61b141f), WTC(0xd8b07038),
    WTC(0xdb43eb5d), WTC(0xddd33d22), WTC(0xe05c1d2e), WTC(0xe2dc432c),
    WTC(0xe55166ad), WTC(0xe7b93f62), WTC(0xea1184df), WTC(0xec57eec9),
    WTC(0xee8a34c6), WTC(0xf0a60e70), WTC(0xf2a9336e), WTC(0xf4915b60),
    WTC(0xf65c3dea), WTC(0xf80792ae), WTC(0xf9911147), WTC(0xfaf67154),
    WTC(0xfc356a7e), WTC(0xfd4bb465), WTC(0xfe3706a9), WTC(0xfef518ec),
    WTC(0xff83a2cf), WTC(0xffe05bed), WTC(0x0008fd26), WTC(0xfffb4037),
};

const FIXP_WTB LowDelaySynthesis128[384] = {
    WTC(0xdb335c78), WTC(0xdc512d40), WTC(0xdd746116), WTC(0xde9cf7ae),
    WTC(0xdfcadda8), WTC(0xe0fe4196), WTC(0xe236a409), WTC(0xe373518f),
    WTC(0xe4b4e870), WTC(0xe5faf25e), WTC(0xe744190a), WTC(0xe88f06f4),
    WTC(0xe9db2796), WTC(0xeb29613f), WTC(0xec78b08e), WTC(0xedc5f65d),
    WTC(0xef0f5af4), WTC(0xf05447dd), WTC(0xf1945392), WTC(0xf2cf795a),
    WTC(0xf405123d), WTC(0xf533af3d), WTC(0xf65842dd), WTC(0xf77071ae),
    WTC(0xf87d623f), WTC(0xf983c711), WTC(0xfa8730ce), WTC(0xfb8aad4b),
    WTC(0xfc8e1dc6), WTC(0xfd96d19a), WTC(0xfea5325a), WTC(0xff8d21da),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbfed9606), WTC(0xbfc8bddf), WTC(0xbfa3dd57), WTC(0xbf7f023c),
    WTC(0xbf5a25e8), WTC(0xbf3554df), WTC(0xbf108b6b), WTC(0xbeebd7bd),
    WTC(0xbec738a6), WTC(0xbea2bf45), WTC(0xbe7e6b42), WTC(0xbe5a4fd5),
    WTC(0xbe366de8), WTC(0xbe12d91e), WTC(0xbdef9338), WTC(0xbdccaf6e),
    WTC(0xbdaa2e3f), WTC(0xbd88205e), WTC(0xbd668430), WTC(0xbd456994),
    WTC(0xbd24cdad), WTC(0xbd04bc91), WTC(0xbce52def), WTC(0xbcc62942),
    WTC(0xbca7a273), WTC(0xbc899fba), WTC(0xbc6c16ab), WTC(0xbc4f0812),
    WTC(0xbc326537), WTC(0xbc162fb3), WTC(0xbbfa5915), WTC(0xbbded462),
    WTC(0xbbcd3956), WTC(0xbbcae0f5), WTC(0xbbd0beb7), WTC(0xbbdaaf42),
    WTC(0xbbec5289), WTC(0xbc06d115), WTC(0xbc266162), WTC(0xbc494d27),
    WTC(0xbc721013), WTC(0xbca5b2d9), WTC(0xbce6a063), WTC(0xbd339d3d),
    WTC(0xbd8975b2), WTC(0xbde618b1), WTC(0xbe499de1), WTC(0xbeb60feb),
    WTC(0xbf2d830e), WTC(0xbfb1ee2e), WTC(0xc041e22f), WTC(0xc0d59618),
    WTC(0xc16a574d), WTC(0xc206ed94), WTC(0xc2ad7ad1), WTC(0xc35ae733),
    WTC(0xc4085fbf), WTC(0xc4b33a35), WTC(0xc563f6ce), WTC(0xc6181ab7),
    WTC(0xc6c86beb), WTC(0xc7768de4), WTC(0xc8231ab3), WTC(0xc8cc145a),
    WTC(0xb4500dde), WTC(0xb22a524e), WTC(0xb010144c), WTC(0xae013530),
    WTC(0xabfd7206), WTC(0xaa04a92f), WTC(0xa816c008), WTC(0xa633baab),
    WTC(0xa45bbe2b), WTC(0xa28eff5e), WTC(0xa0cdc4c6), WTC(0x9f187801),
    WTC(0x9d6f7708), WTC(0x9bd34eb0), WTC(0x9a44763a), WTC(0x98c36ace),
    WTC(0x9750b896), WTC(0x95ecdc61), WTC(0x94982f8c), WTC(0x9353005a),
    WTC(0x921d8bac), WTC(0x90f7e126), WTC(0x8fe1e828), WTC(0x8edb3ddc),
    WTC(0x8de337c8), WTC(0x8cf89cdb), WTC(0x8c19be6d), WTC(0x8b43d072),
    WTC(0x8a737663), WTC(0x89a52f4d), WTC(0x88dc38e8), WTC(0x882f6f3f),
    WTC(0x87c22c55), WTC(0x87918a21), WTC(0x87602c61), WTC(0x872dfd0a),
    WTC(0x86fae063), WTC(0x86c6d729), WTC(0x8691c4ad), WTC(0x865ba7e8),
    WTC(0x86246b66), WTC(0x85ec17ab), WTC(0x85b293e2), WTC(0x8577eaac),
    WTC(0x853c09be), WTC(0x84ff042d), WTC(0x84c0d195), WTC(0x84818c4c),
    WTC(0x84412e63), WTC(0x83ffd42c), WTC(0x83bd7bdf), WTC(0x837a471b),
    WTC(0x833636dc), WTC(0x82f16e48), WTC(0x82abed37), WTC(0x8265d6c4),
    WTC(0x821f28cf), WTC(0x81d80322), WTC(0x8190625c), WTC(0x81486134),
    WTC(0x80fff7a0), WTC(0x80b73d86), WTC(0x806e2527), WTC(0x8024c969),
    WTC(0x0ab6c2d2), WTC(0x0b6edac2), WTC(0x0c302988), WTC(0x0cf88fab),
    WTC(0x0dc87461), WTC(0x0e9f9122), WTC(0x0f7ee53f), WTC(0x1064e7bf),
    WTC(0x114fe4b7), WTC(0x123e9e4c), WTC(0x13311589), WTC(0x1420b664),
    WTC(0x15069245), WTC(0x15dc93ad), WTC(0x169caf41), WTC(0x17422e16),
    WTC(0x17d51de4), WTC(0x187e7622), WTC(0x1947aa0b), WTC(0x1a2ed3b4),
    WTC(0x1b321858), WTC(0x1c4fcc7f), WTC(0x1d8601a3), WTC(0x1ed6ec5e),
    WTC(0x20460b07), WTC(0x21cfbf59), WTC(0x23652732), WTC(0x2508e4b1),
    WTC(0x26c7b0a6), WTC(0x289505da), WTC(0x2a698dd8), WTC(0x2c5954d2),
    WTC(0x2e6dd135), WTC(0x308d838f), WTC(0x329de377), WTC(0x34b28f13),
    WTC(0x36f67988), WTC(0x395ec1e5), WTC(0x3bb7b587), WTC(0x3deb63cc),
    WTC(0x4016b320), WTC(0x42584eb2), WTC(0x44b424ca), WTC(0x471ba5b2),
    WTC(0x49791954), WTC(0x4bc02004), WTC(0x4df3b8c0), WTC(0x501f1e1e),
    WTC(0x524b93e2), WTC(0x547f0eef), WTC(0x56b23d03), WTC(0x58c99052),
    WTC(0x5abfc042), WTC(0x5caebf1a), WTC(0x5e9e618b), WTC(0x608595d7),
    WTC(0x62528e67), WTC(0x6401eb53), WTC(0x65ad0eb1), WTC(0x674f1cd2),
    WTC(0x68d8804e), WTC(0x6a4ff10e), WTC(0x6bb987a5), WTC(0x6d12e937),
    WTC(0xaf370652), WTC(0xb36badcf), WTC(0xb77ec321), WTC(0xbb77e364),
    WTC(0xbf57979e), WTC(0xc31cb589), WTC(0xc6c5e686), WTC(0xca52811d),
    WTC(0xcdc1d66b), WTC(0xd113d0f0), WTC(0xd4481c98), WTC(0xd75ee41b),
    WTC(0xda57f7bf), WTC(0xdd33a926), WTC(0xdff1e272), WTC(0xe2931227),
    WTC(0xe5174232), WTC(0xe77f0b15), WTC(0xe9ca889d), WTC(0xebfa2f7c),
    WTC(0xee0de002), WTC(0xf0063326), WTC(0xf1e3e5f1), WTC(0xf3a8d749),
    WTC(0xf5555599), WTC(0xf6e77eb2), WTC(0xf85cb5d6), WTC(0xf9b8e64d),
    WTC(0xfafe52ad), WTC(0xfc2b37b8), WTC(0xfd420467), WTC(0xfe41412c),
    WTC(0xff26ded5), WTC(0xfffa6150), WTC(0x00c374a4), WTC(0x01773884),
    WTC(0x02058de3), WTC(0x0278d689), WTC(0x02e87372), WTC(0x03593268),
    WTC(0x03ba79ab), WTC(0x03fff32b), WTC(0x042b2341), WTC(0x044690de),
    WTC(0x045bc96a), WTC(0x046e77e9), WTC(0x047a85c1), WTC(0x0479d8c4),
    WTC(0x04681aec), WTC(0x0447318f), WTC(0x041e4d13), WTC(0x03f3edad),
    WTC(0x03c4cc17), WTC(0x038b1f6f), WTC(0x03470909), WTC(0x02fdcebd),
    WTC(0x02b1cb18), WTC(0x0261730d), WTC(0x020afad0), WTC(0x01b0b9c8),
    WTC(0x0153c1a0), WTC(0x00f47426), WTC(0x00933db7), WTC(0x003140e9),
    WTC(0xd5b730bf), WTC(0xd4192c32), WTC(0xd2859479), WTC(0xd0fc3b9d),
    WTC(0xcf800352), WTC(0xce0e6ab3), WTC(0xccaaf25f), WTC(0xcb4ed003),
    WTC(0xc9f3ae44), WTC(0xc8948a56), WTC(0xc73226fd), WTC(0xc5b2676a),
    WTC(0xc3fa2a82), WTC(0xc1f05f70), WTC(0xbf7c884b), WTC(0xbc8b1e86),
    WTC(0xb93e1633), WTC(0xb63eb483), WTC(0xb3b45d14), WTC(0xb19a8ee2),
    WTC(0xafecd4aa), WTC(0xaea6b8e7), WTC(0xadc3c6bf), WTC(0xad3f88ac),
    WTC(0xad158929), WTC(0xad4152b1), WTC(0xadbe7068), WTC(0xae886c76),
    WTC(0xaf9ad1fe), WTC(0xb0f12b26), WTC(0xb2870347), WTC(0xb457d633),
    WTC(0xb65f592c), WTC(0xb898eca0), WTC(0xbb00291b), WTC(0xbd90996b),
    WTC(0xc045c861), WTC(0xc31b4022), WTC(0xc60c8bd5), WTC(0xc91535f2),
    WTC(0xcc30c94c), WTC(0xcf5ad039), WTC(0xd28ed58a), WTC(0xd5c863e5),
    WTC(0xd90305e6), WTC(0xdc3a4644), WTC(0xdf69af93), WTC(0xe28ccc93),
    WTC(0xe59f27d7), WTC(0xe89c4c15), WTC(0xeb7fc3e3), WTC(0xee4519f7),
    WTC(0xf0e7d8ed), WTC(0xf3638b73), WTC(0xf5b3bc30), WTC(0xf7d3f5d0),
    WTC(0xf9bfc2f0), WTC(0xfb72ae35), WTC(0xfce84251), WTC(0xfe1c09e5),
    WTC(0xff098f9a), WTC(0xffac5e15), WTC(0xffffffb2), WTC(0x0000159b),
};

const FIXP_WTB LowDelaySynthesis120[360] = {
    WTC(0xdb3ccdcd), WTC(0xdc6e0d69), WTC(0xdda570a7), WTC(0xdee2ef33),
    WTC(0xe02680f4), WTC(0xe1704397), WTC(0xe2bf5626), WTC(0xe4137c80),
    WTC(0xe56d30cd), WTC(0xe6cb22fe), WTC(0xe82b9f14), WTC(0xe98d8220),
    WTC(0xeaf18a17), WTC(0xec57328f), WTC(0xedbae901), WTC(0xef1a42b9),
    WTC(0xf07481f7), WTC(0xf1c932d4), WTC(0xf3183e19), WTC(0xf460b35f),
    WTC(0xf5a04ac7), WTC(0xf6d33845), WTC(0xf7f80ec0), WTC(0xf912a5cd),
    WTC(0xfa282a2e), WTC(0xfb3cd81c), WTC(0xfc51629b), WTC(0xfd69f81f),
    WTC(0xfe8abdeb), WTC(0xff86f173), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbfec5bfa), WTC(0xbfc50dd8), WTC(0xbf9db88b), WTC(0xbf76683c),
    WTC(0xbf4f1944), WTC(0xbf27d649), WTC(0xbf00a158), WTC(0xbed9848c),
    WTC(0xbeb288f1), WTC(0xbe8bb79f), WTC(0xbe651f00), WTC(0xbe3ec6f8),
    WTC(0xbe18c1f6), WTC(0xbdf315fe), WTC(0xbdcdd79d), WTC(0xbda909c1),
    WTC(0xbd84beae), WTC(0xbd60f6ab), WTC(0xbd3dc210), WTC(0xbd1b2093),
    WTC(0xbcf91ae9), WTC(0xbcd7acab), WTC(0xbcb6d5cc), WTC(0xbc969143),
    WTC(0xbc76dcf4), WTC(0xbc57b317), WTC(0xbc390c4d), WTC(0xbc1ad514),
    WTC(0xbbfd2fdb), WTC(0xbbdfa548), WTC(0xbbcce946), WTC(0xbbcb39d9),
    WTC(0xbbd214cf), WTC(0xbbddfb60), WTC(0xbbf3783e), WTC(0xbc11f8d2),
    WTC(0xbc3509f8), WTC(0xbc5ca2b0), WTC(0xbc8dc0b0), WTC(0xbccd4b2f),
    WTC(0xbd1b7107), WTC(0xbd74c6a4), WTC(0xbdd635c0), WTC(0xbe3f4cd7),
    WTC(0xbeb24836), WTC(0xbf31b5df), WTC(0xbfbfe6e2), WTC(0xc05a6dbf),
    WTC(0xc0f80700), WTC(0xc198449c), WTC(0xc242ea15), WTC(0xc2f8270d),
    WTC(0xc3b20bd2), WTC(0xc468f554), WTC(0xc522637e), WTC(0xc5e2403b),
    WTC(0xc69f9771), WTC(0xc7599dd5), WTC(0xc811f85a), WTC(0xc8c687db),
    WTC(0xb43d8b3b), WTC(0xb1f3fb3c), WTC(0xafb77be9), WTC(0xad87e063),
    WTC(0xab64dcd5), WTC(0xa94e4cc2), WTC(0xa74418f8), WTC(0xa5465837),
    WTC(0xa3554252), WTC(0xa1711f59), WTC(0x9f9a62f5), WTC(0x9dd18104),
    WTC(0x9c17167e), WTC(0x9a6bbbe9), WTC(0x98d0061c), WTC(0x97449e23),
    WTC(0x95ca1ad6), WTC(0x9460e7a1), WTC(0x93096222), WTC(0x91c3c9da),
    WTC(0x90902633), WTC(0x8f6e3c5f), WTC(0x8e5d7788), WTC(0x8d5cb789),
    WTC(0x8c6a42d5), WTC(0x8b833d6f), WTC(0x8aa3cc10), WTC(0x89c7789a),
    WTC(0x88ef9802), WTC(0x883452c3), WTC(0x87c0bb9a), WTC(0x878c8326),
    WTC(0x8757eb4c), WTC(0x87222119), WTC(0x86eb5f31), WTC(0x86b3802c),
    WTC(0x867a73ee), WTC(0x86402cf8), WTC(0x8604a439), WTC(0x85c7cd22),
    WTC(0x8589a40b), WTC(0x854a1d2f), WTC(0x850944c1), WTC(0x84c71670),
    WTC(0x8483acc8), WTC(0x843f04b6), WTC(0x83f93cd9), WTC(0x83b2576c),
    WTC(0x836a7812), WTC(0x8321a764), WTC(0x82d805e8), WTC(0x828da076),
    WTC(0x824290c3), WTC(0x81f6e6a8), WTC(0x81aab23c), WTC(0x815e05f8),
    WTC(0x8110e4d6), WTC(0x80c362df), WTC(0x8075781d), WTC(0x80273c0c),
    WTC(0x0abcb7ed), WTC(0x0b81d183), WTC(0x0c5113e3), WTC(0x0d2867d5),
    WTC(0x0e082fa5), WTC(0x0ef089ad), WTC(0x0fe1d9cd), WTC(0x10d9e5f4),
    WTC(0x11d6a2a0), WTC(0x12d814b1), WTC(0x13d98f52), WTC(0x14d221e1),
    WTC(0x15ba467a), WTC(0x168a9c18), WTC(0x173d1fef), WTC(0x17da3bff),
    WTC(0x18912cac), WTC(0x196c2a5e), WTC(0x1a68dd2e), WTC(0x1b85250c),
    WTC(0x1cbea9a1), WTC(0x1e147be7), WTC(0x1f8aa446), WTC(0x2122e73a),
    WTC(0x22cf6de3), WTC(0x248867f3), WTC(0x265d7937), WTC(0x2847c91b),
    WTC(0x2a39d98e), WTC(0x2c482abf), WTC(0x2e7ff439), WTC(0x30c31f46),
    WTC(0x32f5245d), WTC(0x3535070e), WTC(0x37adae38), WTC(0x3a3f151a),
    WTC(0x3caf861b), WTC(0x3effc08b), WTC(0x415a803b), WTC(0x43d45ca9),
    WTC(0x46631c69), WTC(0x48ed9c7e), WTC(0x4b608807), WTC(0x4dbbead3),
    WTC(0x500ca1df), WTC(0x525e3c5d), WTC(0x54b7c199), WTC(0x570df9a2),
    WTC(0x5940f661), WTC(0x5b542f13), WTC(0x5d64a202), WTC(0x5f738e39),
    WTC(0x61704027), WTC(0x6348ef5e), WTC(0x65109c5f), WTC(0x66d3e29d),
    WTC(0x687eb29a), WTC(0x6a125643), WTC(0x6b960b3a), WTC(0x6d07b283),
    WTC(0xaf5b8daa), WTC(0xb3d557ba), WTC(0xb829feb2), WTC(0xbc6199e7),
    WTC(0xc07bfbc1), WTC(0xc477a1b6), WTC(0xc8532f30), WTC(0xcc0dd698),
    WTC(0xcfa71f06), WTC(0xd31ec578), WTC(0xd674c5b9), WTC(0xd9a90516),
    WTC(0xdcbbc2b6), WTC(0xdfacf934), WTC(0xe27d19ab), WTC(0xe52c3d89),
    WTC(0xe7bb0f52), WTC(0xea29bdd0), WTC(0xec78bc4a), WTC(0xeea805e6),
    WTC(0xf0b85a68), WTC(0xf2ab266e), WTC(0xf48234a7), WTC(0xf63cb733),
    WTC(0xf7d70b3f), WTC(0xf952d4d3), WTC(0xfab4906d), WTC(0xfbfaa858),
    WTC(0xfd272437), WTC(0xfe392bea), WTC(0xff2e26f1), WTC(0x000efac0),
    WTC(0x00e36d26), WTC(0x019bd803), WTC(0x0229e357), WTC(0x02a16b25),
    WTC(0x0319ee5e), WTC(0x038ccea0), WTC(0x03e56d3d), WTC(0x041dc072),
    WTC(0x043f58cc), WTC(0x04571273), WTC(0x046ba761), WTC(0x0479caa5),
    WTC(0x047a2279), WTC(0x04673950), WTC(0x04435263), WTC(0x041750da),
    WTC(0x03e99926), WTC(0x03b4ba1f), WTC(0x03731e2b), WTC(0x0327b303),
    WTC(0x02d8301b), WTC(0x0284fb11), WTC(0x022b464a), WTC(0x01cc1b40),
    WTC(0x0169ab7d), WTC(0x01047d1e), WTC(0x009d04e0), WTC(0x003484b0),
    WTC(0xd5a9348a), WTC(0xd3f05eca), WTC(0xd24339e7), WTC(0xd0a269b7),
    WTC(0xcf0fe026), WTC(0xcd8aa14b), WTC(0xcc13963e), WTC(0xcaa19c5a),
    WTC(0xc92cd701), WTC(0xc7b5cd7b), WTC(0xc62a4fe3), WTC(0xc46742be),
    WTC(0xc24e128c), WTC(0xbfc0b758), WTC(0xbca661e3), WTC(0xb922924b),
    WTC(0xb5f87ac2), WTC(0xb35308e7), WTC(0xb12cc90f), WTC(0xaf80522c),
    WTC(0xae483b10), WTC(0xad7f1af9), WTC(0xad1f8874), WTC(0xad241a09),
    WTC(0xad8766ea), WTC(0xae4405b1), WTC(0xaf548d72), WTC(0xb0b394ad),
    WTC(0xb25bb297), WTC(0xb4476fa1), WTC(0xb6718d2c), WTC(0xb8d47781),
    WTC(0xbb6ad37b), WTC(0xbe2f3831), WTC(0xc11c3c6c), WTC(0xc42c76b1),
    WTC(0xc75a7e40), WTC(0xcaa0e9d1), WTC(0xcdfa502b), WTC(0xd1614803),
    WTC(0xd4d06850), WTC(0xd84247d3), WTC(0xdbb17d6d), WTC(0xdf189fe3),
    WTC(0xe272461e), WTC(0xe5b906e1), WTC(0xe8e7790e), WTC(0xebf83366),
    WTC(0xeee5cccc), WTC(0xf1aadc0a), WTC(0xf441f7fa), WTC(0xf6a5b772),
    WTC(0xf8d0b146), WTC(0xfabd7c3e), WTC(0xfc66af35), WTC(0xfdc6e101),
    WTC(0xfed8a875), WTC(0xff969c63), WTC(0xfffb5390), WTC(0x00017ad8),
};

const FIXP_WTB LowDelaySynthesis512[1536] = {
    /* part 0 */
    WTC(0xdac984c0), WTC(0xdb100080), WTC(0xdb56cd00), WTC(0xdb9dec40),
    WTC(0xdbe55fc0), WTC(0xdc2d2880), WTC(0xdc754780), WTC(0xdcbdbd80),
    WTC(0xdd068a80), WTC(0xdd4fae80), WTC(0xdd992940), WTC(0xdde2f9c0),
    WTC(0xde2d1fc0), WTC(0xde779a80), WTC(0xdec26a00), WTC(0xdf0d8e00),
    WTC(0xdf590680), WTC(0xdfa4d540), WTC(0xdff0fc80), WTC(0xe03d7e20),
    WTC(0xe08a5900), WTC(0xe0d78a20), WTC(0xe1250cc0), WTC(0xe172dcc0),
    WTC(0xe1c0f7a0), WTC(0xe20f59a0), WTC(0xe25dfea0), WTC(0xe2ace400),
    WTC(0xe2fc0be0), WTC(0xe34b7bc0), WTC(0xe39b3c80), WTC(0xe3eb5260),
    WTC(0xe43bbac0), WTC(0xe48c7160), WTC(0xe4dd7140), WTC(0xe52eb600),
    WTC(0xe5803c00), WTC(0xe5d1fda0), WTC(0xe623f360), WTC(0xe6761700),
    WTC(0xe6c86400), WTC(0xe71ad500), WTC(0xe76d63e0), WTC(0xe7c00ba0),
    WTC(0xe812c8e0), WTC(0xe86598e0), WTC(0xe8b878e0), WTC(0xe90b68a0),
    WTC(0xe95e6c40), WTC(0xe9b18ae0), WTC(0xea04ce80), WTC(0xea583ba0),
    WTC(0xeaabcda0), WTC(0xeaff7ee0), WTC(0xeb5348e0), WTC(0xeba722c0),
    WTC(0xebfb0060), WTC(0xec4ed240), WTC(0xeca28540), WTC(0xecf60c20),
    WTC(0xed496120), WTC(0xed9c7e80), WTC(0xedef5e40), WTC(0xee41fc00),
    WTC(0xee945600), WTC(0xeee66ac0), WTC(0xef3839a0), WTC(0xef89c0e0),
    WTC(0xefdafda0), WTC(0xf02bed60), WTC(0xf07c8e80), WTC(0xf0cce000),
    WTC(0xf11ce220), WTC(0xf16c9620), WTC(0xf1bbfe30), WTC(0xf20b19e0),
    WTC(0xf259e5a0), WTC(0xf2a85dc0), WTC(0xf2f67ed0), WTC(0xf34445b0),
    WTC(0xf391aed0), WTC(0xf3deb590), WTC(0xf42b53e0), WTC(0xf4778140),
    WTC(0xf4c33190), WTC(0xf50e5660), WTC(0xf558df30), WTC(0xf5a2be50),
    WTC(0xf5ebea10), WTC(0xf6345780), WTC(0xf67bfab0), WTC(0xf6c2cee0),
    WTC(0xf708d7b0), WTC(0xf74e19c0), WTC(0xf7929a70), WTC(0xf7d66630),
    WTC(0xf8199268), WTC(0xf85c3860), WTC(0xf89e7480), WTC(0xf8e058c0),
    WTC(0xf921ec08), WTC(0xf9633800), WTC(0xf9a44980), WTC(0xf9e53158),
    WTC(0xfa260158), WTC(0xfa66ca18), WTC(0xfaa79ac0), WTC(0xfae87920),
    WTC(0xfb295fa0), WTC(0xfb6a42b8), WTC(0xfbab1240), WTC(0xfbebd1c0),
    WTC(0xfc2c9c24), WTC(0xfc6d8d90), WTC(0xfcaec240), WTC(0xfcf05684),
    WTC(0xfd326a98), WTC(0xfd75254c), WTC(0xfdb8afd4), WTC(0xfdfccdfc),
    WTC(0xfe40d694), WTC(0xfe84161c), WTC(0xfec5cf5a), WTC(0xff04e7fc),
    WTC(0xff3fdfe3), WTC(0xff751ddf), WTC(0xffa2fb0f), WTC(0xffc87c42),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbffb6081), WTC(0xbff22f81), WTC(0xbfe8fc01), WTC(0xbfdfc781),
    WTC(0xbfd69101), WTC(0xbfcd5a01), WTC(0xbfc42201), WTC(0xbfbae981),
    WTC(0xbfb1b101), WTC(0xbfa87901), WTC(0xbf9f4181), WTC(0xbf960b01),
    WTC(0xbf8cd481), WTC(0xbf839d81), WTC(0xbf7a6681), WTC(0xbf712f01),
    WTC(0xbf67f801), WTC(0xbf5ec101), WTC(0xbf558b01), WTC(0xbf4c5681),
    WTC(0xbf432281), WTC(0xbf39ee81), WTC(0xbf30bb01), WTC(0xbf278801),
    WTC(0xbf1e5501), WTC(0xbf152381), WTC(0xbf0bf381), WTC(0xbf02c581),
    WTC(0xbef99901), WTC(0xbef06d01), WTC(0xbee74281), WTC(0xbede1901),
    WTC(0xbed4f081), WTC(0xbecbca81), WTC(0xbec2a781), WTC(0xbeb98681),
    WTC(0xbeb06881), WTC(0xbea74c81), WTC(0xbe9e3281), WTC(0xbe951a81),
    WTC(0xbe8c0501), WTC(0xbe82f301), WTC(0xbe79e481), WTC(0xbe70da01),
    WTC(0xbe67d381), WTC(0xbe5ed081), WTC(0xbe55d001), WTC(0xbe4cd381),
    WTC(0xbe43da81), WTC(0xbe3ae601), WTC(0xbe31f701), WTC(0xbe290d01),
    WTC(0xbe202801), WTC(0xbe174781), WTC(0xbe0e6c01), WTC(0xbe059481),
    WTC(0xbdfcc301), WTC(0xbdf3f701), WTC(0xbdeb3101), WTC(0xbde27201),
    WTC(0xbdd9b981), WTC(0xbdd10681), WTC(0xbdc85981), WTC(0xbdbfb281),
    WTC(0xbdb71201), WTC(0xbdae7881), WTC(0xbda5e601), WTC(0xbd9d5b81),
    WTC(0xbd94d801), WTC(0xbd8c5c01), WTC(0xbd83e681), WTC(0xbd7b7781),
    WTC(0xbd731081), WTC(0xbd6ab101), WTC(0xbd625981), WTC(0xbd5a0b01),
    WTC(0xbd51c481), WTC(0xbd498601), WTC(0xbd414f01), WTC(0xbd391f81),
    WTC(0xbd30f881), WTC(0xbd28d981), WTC(0xbd20c401), WTC(0xbd18b781),
    WTC(0xbd10b381), WTC(0xbd08b781), WTC(0xbd00c381), WTC(0xbcf8d781),
    WTC(0xbcf0f381), WTC(0xbce91801), WTC(0xbce14601), WTC(0xbcd97c81),
    WTC(0xbcd1bb81), WTC(0xbcca0301), WTC(0xbcc25181), WTC(0xbcbaa801),
    WTC(0xbcb30601), WTC(0xbcab6c01), WTC(0xbca3db01), WTC(0xbc9c5281),
    WTC(0xbc94d201), WTC(0xbc8d5901), WTC(0xbc85e801), WTC(0xbc7e7e01),
    WTC(0xbc771c01), WTC(0xbc6fc101), WTC(0xbc686e01), WTC(0xbc612301),
    WTC(0xbc59df81), WTC(0xbc52a381), WTC(0xbc4b6e81), WTC(0xbc444081),
    WTC(0xbc3d1801), WTC(0xbc35f501), WTC(0xbc2ed681), WTC(0xbc27bd81),
    WTC(0xbc20ae01), WTC(0xbc19ab01), WTC(0xbc12b801), WTC(0xbc0bcf81),
    WTC(0xbc04e381), WTC(0xbbfde481), WTC(0xbbf6c601), WTC(0xbbef9b81),
    WTC(0xbbe89901), WTC(0xbbe1f401), WTC(0xbbdbe201), WTC(0xbbd68c81),
    WTC(0xbbd21281), WTC(0xbbce9181), WTC(0xbbcc2681), WTC(0xbbcaca01),
    WTC(0xbbca5081), WTC(0xbbca8d01), WTC(0xbbcb5301), WTC(0xbbcc8201),
    WTC(0xbbce0601), WTC(0xbbcfca81), WTC(0xbbd1bd81), WTC(0xbbd3e101),
    WTC(0xbbd64d01), WTC(0xbbd91b81), WTC(0xbbdc6481), WTC(0xbbe03801),
    WTC(0xbbe49d01), WTC(0xbbe99981), WTC(0xbbef3301), WTC(0xbbf56181),
    WTC(0xbbfc0f81), WTC(0xbc032601), WTC(0xbc0a8f01), WTC(0xbc123b81),
    WTC(0xbc1a2401), WTC(0xbc224181), WTC(0xbc2a8c81), WTC(0xbc330781),
    WTC(0xbc3bbc01), WTC(0xbc44b481), WTC(0xbc4dfb81), WTC(0xbc57a301),
    WTC(0xbc61c401), WTC(0xbc6c7781), WTC(0xbc77d601), WTC(0xbc83f201),
    WTC(0xbc90d481), WTC(0xbc9e8801), WTC(0xbcad1501), WTC(0xbcbc7e01),
    WTC(0xbcccbd01), WTC(0xbcddcc81), WTC(0xbcefa601), WTC(0xbd023f01),
    WTC(0xbd158801), WTC(0xbd297181), WTC(0xbd3deb81), WTC(0xbd52eb01),
    WTC(0xbd686681), WTC(0xbd7e5581), WTC(0xbd94b001), WTC(0xbdab7181),
    WTC(0xbdc29a81), WTC(0xbdda2a01), WTC(0xbdf22181), WTC(0xbe0a8581),
    WTC(0xbe236001), WTC(0xbe3cbc01), WTC(0xbe56a381), WTC(0xbe712001),
    WTC(0xbe8c3781), WTC(0xbea7f301), WTC(0xbec45881), WTC(0xbee17201),
    WTC(0xbeff4801), WTC(0xbf1de601), WTC(0xbf3d5501), WTC(0xbf5d9a81),
    WTC(0xbf7eb581), WTC(0xbfa0a581), WTC(0xbfc36a01), WTC(0xbfe6ed01),
    WTC(0xc00b04c0), WTC(0xc02f86c0), WTC(0xc0544940), WTC(0xc0792ec0),
    WTC(0xc09e2640), WTC(0xc0c31f00), WTC(0xc0e80a00), WTC(0xc10cf480),
    WTC(0xc1320940), WTC(0xc15773c0), WTC(0xc17d5f00), WTC(0xc1a3e340),
    WTC(0xc1cb05c0), WTC(0xc1f2cbc0), WTC(0xc21b3940), WTC(0xc2444b00),
    WTC(0xc26df5c0), WTC(0xc2982d80), WTC(0xc2c2e640), WTC(0xc2ee0a00),
    WTC(0xc3197940), WTC(0xc34513c0), WTC(0xc370b9c0), WTC(0xc39c4f00),
    WTC(0xc3c7bc00), WTC(0xc3f2e940), WTC(0xc41dc140), WTC(0xc44856c0),
    WTC(0xc472e640), WTC(0xc49dad80), WTC(0xc4c8e880), WTC(0xc4f4acc0),
    WTC(0xc520e840), WTC(0xc54d8780), WTC(0xc57a76c0), WTC(0xc5a79640),
    WTC(0xc5d4bac0), WTC(0xc601b880), WTC(0xc62e6580), WTC(0xc65ab600),
    WTC(0xc686bd40), WTC(0xc6b28fc0), WTC(0xc6de41c0), WTC(0xc709de40),
    WTC(0xc7356640), WTC(0xc760da80), WTC(0xc78c3c40), WTC(0xc7b78640),
    WTC(0xc7e2afc0), WTC(0xc80dae80), WTC(0xc83878c0), WTC(0xc86304c0),
    WTC(0xc88d4900), WTC(0xc8b73b80), WTC(0xc8e0d280), WTC(0xc90a0440),
    /* part 1 */
    WTC(0xb5212e81), WTC(0xb4959501), WTC(0xb40ab501), WTC(0xb3808d81),
    WTC(0xb2f71f01), WTC(0xb26e6881), WTC(0xb1e66a01), WTC(0xb15f2381),
    WTC(0xb0d89401), WTC(0xb052bc01), WTC(0xafcd9a81), WTC(0xaf492f01),
    WTC(0xaec57801), WTC(0xae427481), WTC(0xadc02281), WTC(0xad3e8101),
    WTC(0xacbd9081), WTC(0xac3d5001), WTC(0xabbdc001), WTC(0xab3edf01),
    WTC(0xaac0ad01), WTC(0xaa432981), WTC(0xa9c65401), WTC(0xa94a2c01),
    WTC(0xa8ceb201), WTC(0xa853e501), WTC(0xa7d9c681), WTC(0xa7605601),
    WTC(0xa6e79401), WTC(0xa66f8201), WTC(0xa5f81f81), WTC(0xa5816e81),
    WTC(0xa50b6e81), WTC(0xa4962181), WTC(0xa4218801), WTC(0xa3ada281),
    WTC(0xa33a7201), WTC(0xa2c7f801), WTC(0xa2563501), WTC(0xa1e52a81),
    WTC(0xa174da81), WTC(0xa1054701), WTC(0xa0967201), WTC(0xa0285d81),
    WTC(0x9fbb0981), WTC(0x9f4e7801), WTC(0x9ee2a901), WTC(0x9e779f81),
    WTC(0x9e0d5e01), WTC(0x9da3e601), WTC(0x9d3b3b81), WTC(0x9cd35f81),
    WTC(0x9c6c5481), WTC(0x9c061b81), WTC(0x9ba0b701), WTC(0x9b3c2801),
    WTC(0x9ad87081), WTC(0x9a759301), WTC(0x9a139101), WTC(0x99b26c81),
    WTC(0x99522801), WTC(0x98f2c601), WTC(0x98944901), WTC(0x9836b201),
    WTC(0x97da0481), WTC(0x977e4181), WTC(0x97236b01), WTC(0x96c98381),
    WTC(0x96708b81), WTC(0x96188501), WTC(0x95c17081), WTC(0x956b4f81),
    WTC(0x95162381), WTC(0x94c1ee01), WTC(0x946eaf81), WTC(0x941c6901),
    WTC(0x93cb1c81), WTC(0x937acb01), WTC(0x932b7501), WTC(0x92dd1b01),
    WTC(0x928fbe01), WTC(0x92435d01), WTC(0x91f7f981), WTC(0x91ad9281),
    WTC(0x91642781), WTC(0x911bb981), WTC(0x90d44781), WTC(0x908dd101),
    WTC(0x90485401), WTC(0x9003ce81), WTC(0x8fc03f01), WTC(0x8f7da401),
    WTC(0x8f3bfb01), WTC(0x8efb4181), WTC(0x8ebb7581), WTC(0x8e7c9301),
    WTC(0x8e3e9481), WTC(0x8e017581), WTC(0x8dc53001), WTC(0x8d89be81),
    WTC(0x8d4f1b01), WTC(0x8d154081), WTC(0x8cdc2901), WTC(0x8ca3cb01),
    WTC(0x8c6c1b01), WTC(0x8c350d01), WTC(0x8bfe9401), WTC(0x8bc8a401),
    WTC(0x8b933001), WTC(0x8b5e2c81), WTC(0x8b298b81), WTC(0x8af53e81),
    WTC(0x8ac13381), WTC(0x8a8d5801), WTC(0x8a599a81), WTC(0x8a25f301),
    WTC(0x89f26101), WTC(0x89bee581), WTC(0x898b8301), WTC(0x89586901),
    WTC(0x8925f101), WTC(0x88f47901), WTC(0x88c45e81), WTC(0x88962981),
    WTC(0x886a8a81), WTC(0x88423301), WTC(0x881dd301), WTC(0x87fdd781),
    WTC(0x87d0ca81), WTC(0x87c76201), WTC(0x87bcab81), WTC(0x87b0ef01),
    WTC(0x87a48b01), WTC(0x8797dd81), WTC(0x878b4301), WTC(0x877ede01),
    WTC(0x87729701), WTC(0x87665481), WTC(0x8759fd01), WTC(0x874d8681),
    WTC(0x8740f681), WTC(0x87345381), WTC(0x8727a381), WTC(0x871ae981),
    WTC(0x870e2301), WTC(0x87014f81), WTC(0x86f46d81), WTC(0x86e77b81),
    WTC(0x86da7901), WTC(0x86cd6681), WTC(0x86c04381), WTC(0x86b30f01),
    WTC(0x86a5ca81), WTC(0x86987581), WTC(0x868b1001), WTC(0x867d9a81),
    WTC(0x86701381), WTC(0x86627b01), WTC(0x8654d001), WTC(0x86471281),
    WTC(0x86394301), WTC(0x862b6201), WTC(0x861d7081), WTC(0x860f6e01),
    WTC(0x86015981), WTC(0x85f33281), WTC(0x85e4f801), WTC(0x85d6a981),
    WTC(0x85c84801), WTC(0x85b9d481), WTC(0x85ab4f01), WTC(0x859cb781),
    WTC(0x858e0e01), WTC(0x857f5101), WTC(0x85707f81), WTC(0x85619a01),
    WTC(0x8552a181), WTC(0x85439601), WTC(0x85347901), WTC(0x85254a81),
    WTC(0x85160981), WTC(0x8506b581), WTC(0x84f74e01), WTC(0x84e7d381),
    WTC(0x84d84601), WTC(0x84c8a701), WTC(0x84b8f801), WTC(0x84a93801),
    WTC(0x84996701), WTC(0x84898481), WTC(0x84798f81), WTC(0x84698881),
    WTC(0x84597081), WTC(0x84494881), WTC(0x84391081), WTC(0x8428ca01),
    WTC(0x84187401), WTC(0x84080d81), WTC(0x83f79681), WTC(0x83e70f01),
    WTC(0x83d67881), WTC(0x83c5d381), WTC(0x83b52101), WTC(0x83a46181),
    WTC(0x83939501), WTC(0x8382ba01), WTC(0x8371d081), WTC(0x8360d901),
    WTC(0x834fd481), WTC(0x833ec381), WTC(0x832da781), WTC(0x831c8101),
    WTC(0x830b4f81), WTC(0x82fa1181), WTC(0x82e8c801), WTC(0x82d77201),
    WTC(0x82c61101), WTC(0x82b4a601), WTC(0x82a33281), WTC(0x8291b601),
    WTC(0x82803101), WTC(0x826ea201), WTC(0x825d0901), WTC(0x824b6601),
    WTC(0x8239b981), WTC(0x82280581), WTC(0x82164a81), WTC(0x82048881),
    WTC(0x81f2bf81), WTC(0x81e0ee81), WTC(0x81cf1581), WTC(0x81bd3401),
    WTC(0x81ab4b01), WTC(0x81995c01), WTC(0x81876781), WTC(0x81756d81),
    WTC(0x81636d81), WTC(0x81516701), WTC(0x813f5981), WTC(0x812d4481),
    WTC(0x811b2981), WTC(0x81090981), WTC(0x80f6e481), WTC(0x80e4bb81),
    WTC(0x80d28d81), WTC(0x80c05a01), WTC(0x80ae1f81), WTC(0x809bdf01),
    WTC(0x80899881), WTC(0x80774c81), WTC(0x8064fc81), WTC(0x8052a881),
    WTC(0x80405101), WTC(0x802df701), WTC(0x801b9b01), WTC(0x80093e01),
    WTC(0x0a74b120), WTC(0x0aa08a90), WTC(0x0acd2b80), WTC(0x0afa8860),
    WTC(0x0b289590), WTC(0x0b574790), WTC(0x0b8692d0), WTC(0x0bb66bb0),
    WTC(0x0be6c6b0), WTC(0x0c179830), WTC(0x0c48d500), WTC(0x0c7a7ad0),
    WTC(0x0cac9000), WTC(0x0cdf1b60), WTC(0x0d122390), WTC(0x0d45a8f0),
    WTC(0x0d79a5e0), WTC(0x0dae1480), WTC(0x0de2ef30), WTC(0x0e183800),
    WTC(0x0e4df8c0), WTC(0x0e843b90), WTC(0x0ebb0a20), WTC(0x0ef26430),
    WTC(0x0f2a3fc0), WTC(0x0f629280), WTC(0x0f9b5210), WTC(0x0fd47690),
    WTC(0x100dfa80), WTC(0x1047d8a0), WTC(0x10820b40), WTC(0x10bc8b80),
    WTC(0x10f75080), WTC(0x11325100), WTC(0x116d84e0), WTC(0x11a8ece0),
    WTC(0x11e49420), WTC(0x122085a0), WTC(0x125ccbc0), WTC(0x12995a40),
    WTC(0x12d60e80), WTC(0x1312c4c0), WTC(0x134f59e0), WTC(0x138bae60),
    WTC(0x13c7a740), WTC(0x140329e0), WTC(0x143e1b60), WTC(0x147862a0),
    WTC(0x14b1e840), WTC(0x14ea94c0), WTC(0x152250a0), WTC(0x15590380),
    WTC(0x158e93e0), WTC(0x15c2e820), WTC(0x15f5e6e0), WTC(0x162779a0),
    WTC(0x16578ca0), WTC(0x16860ca0), WTC(0x16b2e640), WTC(0x16de0b00),
    WTC(0x17077140), WTC(0x172f0fa0), WTC(0x1754e200), WTC(0x17796080),
    WTC(0x179d7f20), WTC(0x17c23760), WTC(0x17e87da0), WTC(0x1810cc80),
    WTC(0x183b25a0), WTC(0x18678520), WTC(0x1895e700), WTC(0x18c64540),
    WTC(0x18f89780), WTC(0x192cd560), WTC(0x1962f680), WTC(0x199af2a0),
    WTC(0x19d4c1e0), WTC(0x1a105ca0), WTC(0x1a4dbae0), WTC(0x1a8cd660),
    WTC(0x1acdaa60), WTC(0x1b103260), WTC(0x1b546940), WTC(0x1b9a4600),
    WTC(0x1be1bb80), WTC(0x1c2abc60), WTC(0x1c753b80), WTC(0x1cc13860),
    WTC(0x1d0ebe20), WTC(0x1d5dd8c0), WTC(0x1dae9480), WTC(0x1e010060),
    WTC(0x1e552f40), WTC(0x1eab33e0), WTC(0x1f032060), WTC(0x1f5cfce0),
    WTC(0x1fb8c660), WTC(0x201679c0), WTC(0x207611c0), WTC(0x20d75f00),
    WTC(0x213a0640), WTC(0x219dab80), WTC(0x2201f480), WTC(0x2266ba80),
    WTC(0x22cc0ac0), WTC(0x2331f4c0), WTC(0x23988940), WTC(0x23ffff40),
    WTC(0x2468b340), WTC(0x24d30300), WTC(0x253f4900), WTC(0x25ad8980),
    WTC(0x261d72c0), WTC(0x268eaec0), WTC(0x2700e880), WTC(0x2773db40),
    WTC(0x27e751c0), WTC(0x285b1780), WTC(0x28cefbc0), WTC(0x29431f80),
    WTC(0x29b7f680), WTC(0x2a2df780), WTC(0x2aa59880), WTC(0x2b1f3280),
    WTC(0x2b9b0140), WTC(0x2c194000), WTC(0x2c9a2540), WTC(0x2d1d8dc0),
    WTC(0x2da2fc40), WTC(0x2e29ee80), WTC(0x2eb1e340), WTC(0x2f3a4e40),
    WTC(0x2fc29980), WTC(0x304a2ec0), WTC(0x30d07cc0), WTC(0x315566c0),
    WTC(0x31d94480), WTC(0x325c72c0), WTC(0x32df51c0), WTC(0x33628c80),
    WTC(0x33e71a00), WTC(0x346df400), WTC(0x34f80dc0), WTC(0x3585c640),
    WTC(0x3616e700), WTC(0x36ab3380), WTC(0x37426ac0), WTC(0x37dbe840),
    WTC(0x3876a340), WTC(0x39118f40), WTC(0x39aba2c0), WTC(0x3a4422c0),
    WTC(0x3adaa200), WTC(0x3b6eb6c0), WTC(0x3bfffd80), WTC(0x3c8e9380),
    WTC(0x3d1b1780), WTC(0x3da62e00), WTC(0x3e307b00), WTC(0x3eba97c0),
    WTC(0x3f451280), WTC(0x3fd07940), WTC(0x405d577f), WTC(0x40ebf57f),
    WTC(0x417c59ff), WTC(0x420e897f), WTC(0x42a2857f), WTC(0x4338307f),
    WTC(0x43cf4d7f), WTC(0x44679cff), WTC(0x4500dfff), WTC(0x459ac2ff),
    WTC(0x4634e2ff), WTC(0x46ced9ff), WTC(0x4768437f), WTC(0x4800d27f),
    WTC(0x489850ff), WTC(0x492e88ff), WTC(0x49c346ff), WTC(0x4a5678ff),
    WTC(0x4ae82f7f), WTC(0x4b787c7f), WTC(0x4c07717f), WTC(0x4c95337f),
    WTC(0x4d21f77f), WTC(0x4dadf3ff), WTC(0x4e395eff), WTC(0x4ec4657f),
    WTC(0x4f4f297f), WTC(0x4fd9cd7f), WTC(0x5064737f), WTC(0x50ef3cff),
    WTC(0x517a46ff), WTC(0x5205b0ff), WTC(0x529197ff), WTC(0x531e04ff),
    WTC(0x53aaeb7f), WTC(0x54383eff), WTC(0x54c5ef7f), WTC(0x5553a8ff),
    WTC(0x55e0d57f), WTC(0x566cda7f), WTC(0x56f720ff), WTC(0x577f4aff),
    WTC(0x580534ff), WTC(0x5888bd7f), WTC(0x5909c6ff), WTC(0x598890ff),
    WTC(0x5a05b7ff), WTC(0x5a81db7f), WTC(0x5afd99ff), WTC(0x5b794a7f),
    WTC(0x5bf5007f), WTC(0x5c70cbff), WTC(0x5cecbb7f), WTC(0x5d68c47f),
    WTC(0x5de4c3ff), WTC(0x5e6094ff), WTC(0x5edc127f), WTC(0x5f56fdff),
    WTC(0x5fd1017f), WTC(0x6049c67f), WTC(0x60c0f67f), WTC(0x613650ff),
    WTC(0x61a9a9ff), WTC(0x621ad77f), WTC(0x6289b37f), WTC(0x62f67fff),
    WTC(0x6361e87f), WTC(0x63cc9bff), WTC(0x6437457f), WTC(0x64a2247f),
    WTC(0x650d0c7f), WTC(0x6577cc7f), WTC(0x65e2327f), WTC(0x664bf57f),
    WTC(0x66b4b5ff), WTC(0x671c137f), WTC(0x6781afff), WTC(0x67e579ff),
    WTC(0x6847abff), WTC(0x68a882ff), WTC(0x69083bff), WTC(0x6966fbff),
    WTC(0x69c4cfff), WTC(0x6a21c57f), WTC(0x6a7de87f), WTC(0x6ad9377f),
    WTC(0x6b33a5ff), WTC(0x6b8d257f), WTC(0x6be5a8ff), WTC(0x6c3d20ff),
    WTC(0x6c9380ff), WTC(0x6ce8ba7f), WTC(0x6d3cbfff), WTC(0x6d8f827f),
    /* part 2 */
    WTC(0xad98b481), WTC(0xaead9d01), WTC(0xafbfc381), WTC(0xb0cf4d01),
    WTC(0xb1dc5f81), WTC(0xb2e72081), WTC(0xb3efb501), WTC(0xb4f64381),
    WTC(0xb5faf101), WTC(0xb6fde401), WTC(0xb7ff4001), WTC(0xb8ff1601),
    WTC(0xb9fd6181), WTC(0xbafa1d01), WTC(0xbbf54401), WTC(0xbceed101),
    WTC(0xbde6c081), WTC(0xbedd0e81), WTC(0xbfd1b701), WTC(0xc0c4b440),
    WTC(0xc1b5ffc0), WTC(0xc2a59340), WTC(0xc3936780), WTC(0xc47f78c0),
    WTC(0xc569c600), WTC(0xc6524d40), WTC(0xc7390dc0), WTC(0xc81e04c0),
    WTC(0xc9012e00), WTC(0xc9e28540), WTC(0xcac20700), WTC(0xcb9fb1c0),
    WTC(0xcc7b8640), WTC(0xcd558600), WTC(0xce2db200), WTC(0xcf0409c0),
    WTC(0xcfd88a40), WTC(0xd0ab3080), WTC(0xd17bfa00), WTC(0xd24ae640),
    WTC(0xd317f7c0), WTC(0xd3e33080), WTC(0xd4ac9340), WTC(0xd5741f40),
    WTC(0xd639d2c0), WTC(0xd6fdab00), WTC(0xd7bfa5c0), WTC(0xd87fc300),
    WTC(0xd93e0600), WTC(0xd9fa7180), WTC(0xdab50900), WTC(0xdb6dccc0),
    WTC(0xdc24ba80), WTC(0xdcd9d000), WTC(0xdd8d0b80), WTC(0xde3e6dc0),
    WTC(0xdeedf9c0), WTC(0xdf9bb340), WTC(0xe0479e20), WTC(0xe0f1bac0),
    WTC(0xe19a07e0), WTC(0xe2408380), WTC(0xe2e52c00), WTC(0xe38802e0),
    WTC(0xe4290c00), WTC(0xe4c84c20), WTC(0xe565c760), WTC(0xe6017f20),
    WTC(0xe69b7240), WTC(0xe7339f60), WTC(0xe7ca0500), WTC(0xe85ea480),
    WTC(0xe8f18180), WTC(0xe9829fc0), WTC(0xea1202e0), WTC(0xea9fab80),
    WTC(0xeb2b9700), WTC(0xebb5c2a0), WTC(0xec3e2bc0), WTC(0xecc4d300),
    WTC(0xed49bc80), WTC(0xedccec60), WTC(0xee4e66a0), WTC(0xeece2d80),
    WTC(0xef4c41e0), WTC(0xefc8a480), WTC(0xf0435610), WTC(0xf0bc5c60),
    WTC(0xf133c230), WTC(0xf1a99270), WTC(0xf21dd7b0), WTC(0xf29097e0),
    WTC(0xf301d3d0), WTC(0xf3718c20), WTC(0xf3dfc180), WTC(0xf44c7100),
    WTC(0xf4b79480), WTC(0xf52125b0), WTC(0xf5891df0), WTC(0xf5ef6fe0),
    WTC(0xf6540730), WTC(0xf6b6cf50), WTC(0xf717b490), WTC(0xf776b9a0),
    WTC(0xf7d3f720), WTC(0xf82f86e8), WTC(0xf8898260), WTC(0xf8e1fc50),
    WTC(0xf93900f0), WTC(0xf98e9c28), WTC(0xf9e2d940), WTC(0xfa35b4a0),
    WTC(0xfa871bd8), WTC(0xfad6fbd0), WTC(0xfb254250), WTC(0xfb71f0c0),
    WTC(0xfbbd1c28), WTC(0xfc06da60), WTC(0xfc4f40a4), WTC(0xfc965500),
    WTC(0xfcdc0e5c), WTC(0xfd2062f4), WTC(0xfd6348d0), WTC(0xfda4b1b8),
    WTC(0xfde48b2c), WTC(0xfe22c280), WTC(0xfe5f462a), WTC(0xfe9a1f2e),
    WTC(0xfed3711c), WTC(0xff0b60ac), WTC(0xff4212dd), WTC(0xff77b344),
    WTC(0xffac7407), WTC(0xffe08796), WTC(0x00141e37), WTC(0x00473665),
    WTC(0x00799cd0), WTC(0x00ab1bff), WTC(0x00db7d8b), WTC(0x010a75ea),
    WTC(0x0137a46e), WTC(0x0162a77a), WTC(0x018b20ac), WTC(0x01b0fb7a),
    WTC(0x01d46d3c), WTC(0x01f5ae7c), WTC(0x0214f91c), WTC(0x0232a5cc),
    WTC(0x024f2c04), WTC(0x026b048c), WTC(0x0286a628), WTC(0x02a25808),
    WTC(0x02be31c0), WTC(0x02da48e0), WTC(0x02f6b09c), WTC(0x031345dc),
    WTC(0x032faf50), WTC(0x034b9148), WTC(0x036690e8), WTC(0x0380658c),
    WTC(0x0398d8e4), WTC(0x03afb568), WTC(0x03c4c6e0), WTC(0x03d7f770),
    WTC(0x03e94f9c), WTC(0x03f8d938), WTC(0x04069ee8), WTC(0x0412bef8),
    WTC(0x041d6b30), WTC(0x0426d638), WTC(0x042f3288), WTC(0x0436ad98),
    WTC(0x043d6fd0), WTC(0x0443a170), WTC(0x04496a40), WTC(0x044ee728),
    WTC(0x04542a40), WTC(0x04594520), WTC(0x045e4890), WTC(0x04633210),
    WTC(0x0467ebe8), WTC(0x046c5f80), WTC(0x04707630), WTC(0x047417f0),
    WTC(0x04772b58), WTC(0x047996e8), WTC(0x047b4140), WTC(0x047c12a0),
    WTC(0x047bf520), WTC(0x047ad2e0), WTC(0x04789690), WTC(0x047539c8),
    WTC(0x0470c4b8), WTC(0x046b4058), WTC(0x0464b600), WTC(0x045d3a08),
    WTC(0x0454ebc8), WTC(0x044beb00), WTC(0x04425798), WTC(0x043853b0),
    WTC(0x042e0398), WTC(0x04238bd8), WTC(0x04190f98), WTC(0x040e9670),
    WTC(0x04040c18), WTC(0x03f95b30), WTC(0x03ee6e20), WTC(0x03e32b64),
    WTC(0x03d77598), WTC(0x03cb2f24), WTC(0x03be3b18), WTC(0x03b08b18),
    WTC(0x03a21f64), WTC(0x0392f8d4), WTC(0x038318e0), WTC(0x03728e94),
    WTC(0x03617694), WTC(0x034fee18), WTC(0x033e11f4), WTC(0x032bf530),
    WTC(0x0319a114), WTC(0x03071e80), WTC(0x02f475f4), WTC(0x02e1a7c0),
    WTC(0x02ceac04), WTC(0x02bb7a84), WTC(0x02a80af0), WTC(0x029452b0),
    WTC(0x028044e0), WTC(0x026bd488), WTC(0x0256f558), WTC(0x0241a940),
    WTC(0x022c0084), WTC(0x02160c08), WTC(0x01ffdc5a), WTC(0x01e97ad2),
    WTC(0x01d2e982), WTC(0x01bc2a2a), WTC(0x01a53e8c), WTC(0x018e2860),
    WTC(0x0176e94c), WTC(0x015f82fa), WTC(0x0147f70e), WTC(0x013046c2),
    WTC(0x011872e8), WTC(0x01007c4a), WTC(0x00e863cf), WTC(0x00d02c81),
    WTC(0x00b7db94), WTC(0x009f7651), WTC(0x00870204), WTC(0x006e83f8),
    WTC(0x00560176), WTC(0x003d7fcb), WTC(0x0025043f), WTC(0x000c941f),
    WTC(0xd65574c0), WTC(0xd5ebc100), WTC(0xd582d080), WTC(0xd51a9cc0),
    WTC(0xd4b31f80), WTC(0xd44c5280), WTC(0xd3e62f80), WTC(0xd380b040),
    WTC(0xd31bce40), WTC(0xd2b78380), WTC(0xd253ca40), WTC(0xd1f0acc0),
    WTC(0xd18e4580), WTC(0xd12caf40), WTC(0xd0cc0400), WTC(0xd06c40c0),
    WTC(0xd00d4740), WTC(0xcfaef6c0), WTC(0xcf513140), WTC(0xcef3fa80),
    WTC(0xce977a40), WTC(0xce3bd980), WTC(0xcde13f40), WTC(0xcd87a880),
    WTC(0xcd2ee800), WTC(0xccd6cf00), WTC(0xcc7f2f40), WTC(0xcc27e880),
    WTC(0xcbd0ea00), WTC(0xcb7a2380), WTC(0xcb238380), WTC(0xcaccee80),
    WTC(0xca763ec0), WTC(0xca1f4d00), WTC(0xc9c7f480), WTC(0xc9703b40),
    WTC(0xc9185200), WTC(0xc8c06b00), WTC(0xc868b4c0), WTC(0xc81100c0),
    WTC(0xc7b8c280), WTC(0xc75f6a40), WTC(0xc7046900), WTC(0xc6a74340),
    WTC(0xc6479300), WTC(0xc5e4f200), WTC(0xc57efac0), WTC(0xc5154880),
    WTC(0xc4a77780), WTC(0xc4352440), WTC(0xc3bdeac0), WTC(0xc3416740),
    WTC(0xc2bf33c0), WTC(0xc236eb40), WTC(0xc1a82900), WTC(0xc11290c0),
    WTC(0xc075cf00), WTC(0xbfd19081), WTC(0xbf258401), WTC(0xbe716d81),
    WTC(0xbdb52b81), WTC(0xbcf09a81), WTC(0xbc23af81), WTC(0xbb505c01),
    WTC(0xba7a9081), WTC(0xb9a65281), WTC(0xb8d79301), WTC(0xb8104c01),
    WTC(0xb7508181), WTC(0xb6982201), WTC(0xb5e71b01), WTC(0xb53d5b01),
    WTC(0xb49ad081), WTC(0xb3ff6901), WTC(0xb36b1301), WTC(0xb2ddbd01),
    WTC(0xb2575481), WTC(0xb1d7c801), WTC(0xb15f0601), WTC(0xb0ecfc01),
    WTC(0xb0819881), WTC(0xb01cca01), WTC(0xafbe7e01), WTC(0xaf66a301),
    WTC(0xaf152701), WTC(0xaec9f881), WTC(0xae850601), WTC(0xae463c81),
    WTC(0xae0d8b01), WTC(0xaddae001), WTC(0xadae2881), WTC(0xad875381),
    WTC(0xad664f81), WTC(0xad4b0981), WTC(0xad357081), WTC(0xad257301),
    WTC(0xad1afe01), WTC(0xad160081), WTC(0xad166901), WTC(0xad1c2481),
    WTC(0xad272201), WTC(0xad374f81), WTC(0xad4c9b01), WTC(0xad66f381),
    WTC(0xad864601), WTC(0xadaa8101), WTC(0xadd39301), WTC(0xae016a01),
    WTC(0xae33f481), WTC(0xae6b2001), WTC(0xaea6db01), WTC(0xaee71381),
    WTC(0xaf2bb801), WTC(0xaf74b681), WTC(0xafc1fd01), WTC(0xb0137a01),
    WTC(0xb0691b81), WTC(0xb0c2cf81), WTC(0xb1208481), WTC(0xb1822881),
    WTC(0xb1e7a981), WTC(0xb250f601), WTC(0xb2bdfc01), WTC(0xb32eaa01),
    WTC(0xb3a2ed01), WTC(0xb41ab481), WTC(0xb495ee01), WTC(0xb5148801),
    WTC(0xb5967081), WTC(0xb61b9581), WTC(0xb6a3e581), WTC(0xb72f4e01),
    WTC(0xb7bdbe01), WTC(0xb84f2381), WTC(0xb8e36c81), WTC(0xb97a8701),
    WTC(0xba146101), WTC(0xbab0e981), WTC(0xbb500d81), WTC(0xbbf1bc81),
    WTC(0xbc95e381), WTC(0xbd3c7181), WTC(0xbde55481), WTC(0xbe907a01),
    WTC(0xbf3dd101), WTC(0xbfed4701), WTC(0xc09ecac0), WTC(0xc1524a00),
    WTC(0xc207b300), WTC(0xc2bef440), WTC(0xc377fb80), WTC(0xc432b700),
    WTC(0xc4ef1500), WTC(0xc5ad03c0), WTC(0xc66c7140), WTC(0xc72d4bc0),
    WTC(0xc7ef8180), WTC(0xc8b30080), WTC(0xc977b700), WTC(0xca3d9340),
    WTC(0xcb048340), WTC(0xcbcc7540), WTC(0xcc955740), WTC(0xcd5f17c0),
    WTC(0xce29a480), WTC(0xcef4ec00), WTC(0xcfc0dc80), WTC(0xd08d63c0),
    WTC(0xd15a7040), WTC(0xd227f000), WTC(0xd2f5d140), WTC(0xd3c40240),
    WTC(0xd4927100), WTC(0xd5610b80), WTC(0xd62fc080), WTC(0xd6fe7dc0),
    WTC(0xd7cd3140), WTC(0xd89bc980), WTC(0xd96a34c0), WTC(0xda3860c0),
    WTC(0xdb063c00), WTC(0xdbd3b480), WTC(0xdca0b880), WTC(0xdd6d3640),
    WTC(0xde391bc0), WTC(0xdf045740), WTC(0xdfced6c0), WTC(0xe09888c0),
    WTC(0xe1615b20), WTC(0xe2293c20), WTC(0xe2f01a00), WTC(0xe3b5e2c0),
    WTC(0xe47a84c0), WTC(0xe53dee00), WTC(0xe6000cc0), WTC(0xe6c0cf20),
    WTC(0xe7802360), WTC(0xe83df7a0), WTC(0xe8fa39e0), WTC(0xe9b4d880),
    WTC(0xea6dc1a0), WTC(0xeb24e360), WTC(0xebda2be0), WTC(0xec8d8960),
    WTC(0xed3eea20), WTC(0xedee3c00), WTC(0xee9b6d80), WTC(0xef466ca0),
    WTC(0xefef2780), WTC(0xf0958c50), WTC(0xf1398950), WTC(0xf1db0ca0),
    WTC(0xf27a0470), WTC(0xf3165ed0), WTC(0xf3b00a10), WTC(0xf446f440),
    WTC(0xf4db0b90), WTC(0xf56c3e30), WTC(0xf5fa7a50), WTC(0xf685ae10),
    WTC(0xf70dc7a0), WTC(0xf792b520), WTC(0xf81464c8), WTC(0xf892c4c0),
    WTC(0xf90dc330), WTC(0xf9854e40), WTC(0xf9f95418), WTC(0xfa69c2f0),
    WTC(0xfad688e8), WTC(0xfb3f9428), WTC(0xfba4d2e8), WTC(0xfc063344),
    WTC(0xfc63a370), WTC(0xfcbd1194), WTC(0xfd126bdc), WTC(0xfd63a06c),
    WTC(0xfdb09d78), WTC(0xfdf95124), WTC(0xfe3da99e), WTC(0xfe7d950e),
    WTC(0xfeb901a2), WTC(0xfeefdd80), WTC(0xff2216d7), WTC(0xff4f9bcf),
    WTC(0xff785a93), WTC(0xff9c414e), WTC(0xffbb3e2b), WTC(0xffd53f54),
    WTC(0xffea32f4), WTC(0xfffa0735), WTC(0x0004aa43), WTC(0x000a0a47),
    WTC(0x000a156c), WTC(0x0004b9de), WTC(0xfff9e5c5), WTC(0xffe9874e)};

const FIXP_WTB LowDelaySynthesis480[1440] = {
    WTC(0xdad2e6c0), WTC(0xdb1da900), WTC(0xdb68ce40), WTC(0xdbb45840),
    WTC(0xdc004940), WTC(0xdc4ca280), WTC(0xdc996500), WTC(0xdce69140),
    WTC(0xdd342780), WTC(0xdd822700), WTC(0xddd08a80), WTC(0xde1f4d00),
    WTC(0xde6e6ec0), WTC(0xdebdec40), WTC(0xdf0dba80), WTC(0xdf5dd540),
    WTC(0xdfae3cc0), WTC(0xdfff0500), WTC(0xe0505140), WTC(0xe0a22980),
    WTC(0xe0f488e0), WTC(0xe1476180), WTC(0xe19aa480), WTC(0xe1ee4d80),
    WTC(0xe2425400), WTC(0xe29689a0), WTC(0xe2eacd60), WTC(0xe33f2420),
    WTC(0xe393a300), WTC(0xe3e87f20), WTC(0xe43dcee0), WTC(0xe4938a80),
    WTC(0xe4e9b0a0), WTC(0xe5404300), WTC(0xe5973e60), WTC(0xe5ee9b80),
    WTC(0xe64649e0), WTC(0xe69e37e0), WTC(0xe6f65ec0), WTC(0xe74eb6c0),
    WTC(0xe7a73000), WTC(0xe7ffbe40), WTC(0xe8585ee0), WTC(0xe8b10740),
    WTC(0xe9099c40), WTC(0xe96214e0), WTC(0xe9ba79a0), WTC(0xea12e7c0),
    WTC(0xea6b89c0), WTC(0xeac46580), WTC(0xeb1d7260), WTC(0xeb76b620),
    WTC(0xebd036c0), WTC(0xec29e520), WTC(0xec83aa60), WTC(0xecdd5a00),
    WTC(0xed36d500), WTC(0xed901540), WTC(0xede91160), WTC(0xee41bc20),
    WTC(0xee9a0ee0), WTC(0xeef20860), WTC(0xef49a7e0), WTC(0xefa0ec00),
    WTC(0xeff7d1c0), WTC(0xf04e56b0), WTC(0xf0a476e0), WTC(0xf0fa2f60),
    WTC(0xf14f80e0), WTC(0xf1a46e10), WTC(0xf1f8fe80), WTC(0xf24d34a0),
    WTC(0xf2a10bb0), WTC(0xf2f48210), WTC(0xf3479cc0), WTC(0xf39a5be0),
    WTC(0xf3ecb8f0), WTC(0xf43eafa0), WTC(0xf4903b50), WTC(0xf4e14e80),
    WTC(0xf531d6a0), WTC(0xf581bc10), WTC(0xf5d0e9c0), WTC(0xf61f5250),
    WTC(0xf66ce6e0), WTC(0xf6b99330), WTC(0xf7054eb0), WTC(0xf7501f20),
    WTC(0xf79a0750), WTC(0xf7e30700), WTC(0xf82b2fc0), WTC(0xf872a138),
    WTC(0xf8b97f18), WTC(0xf8ffe668), WTC(0xf945e538), WTC(0xf98b8860),
    WTC(0xf9d0f380), WTC(0xfa165148), WTC(0xfa5bb8a8), WTC(0xfaa13df8),
    WTC(0xfae6fb00), WTC(0xfb2cf8c8), WTC(0xfb732a80), WTC(0xfbb97910),
    WTC(0xfbffcd10), WTC(0xfc463478), WTC(0xfc8cd3fc), WTC(0xfcd3be5c),
    WTC(0xfd1afa90), WTC(0xfd62aa84), WTC(0xfdab0288), WTC(0xfdf404b4),
    WTC(0xfe3d3006), WTC(0xfe85b20e), WTC(0xfecca4cc), WTC(0xff10d559),
    WTC(0xff50579b), WTC(0xff8a40d2), WTC(0xffb7d86e), WTC(0xffef6bbb),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0x00000000), WTC(0x00000000), WTC(0x00000000), WTC(0x00000000),
    WTC(0xbff67a01), WTC(0xbfecaa81), WTC(0xbfe2d901), WTC(0xbfd90601),
    WTC(0xbfcf3181), WTC(0xbfc55c81), WTC(0xbfbb8701), WTC(0xbfb1b101),
    WTC(0xbfa7dc01), WTC(0xbf9e0701), WTC(0xbf943301), WTC(0xbf8a5f81),
    WTC(0xbf808b81), WTC(0xbf76b701), WTC(0xbf6ce201), WTC(0xbf630d81),
    WTC(0xbf593a01), WTC(0xbf4f6801), WTC(0xbf459681), WTC(0xbf3bc601),
    WTC(0xbf31f501), WTC(0xbf282501), WTC(0xbf1e5501), WTC(0xbf148681),
    WTC(0xbf0aba01), WTC(0xbf00ef81), WTC(0xbef72681), WTC(0xbeed5f01),
    WTC(0xbee39801), WTC(0xbed9d281), WTC(0xbed00f81), WTC(0xbec64e81),
    WTC(0xbebc9181), WTC(0xbeb2d681), WTC(0xbea91f01), WTC(0xbe9f6901),
    WTC(0xbe95b581), WTC(0xbe8c0501), WTC(0xbe825801), WTC(0xbe78b001),
    WTC(0xbe6f0c01), WTC(0xbe656c01), WTC(0xbe5bd001), WTC(0xbe523781),
    WTC(0xbe48a301), WTC(0xbe3f1381), WTC(0xbe358901), WTC(0xbe2c0501),
    WTC(0xbe228681), WTC(0xbe190d81), WTC(0xbe0f9a01), WTC(0xbe062b81),
    WTC(0xbdfcc301), WTC(0xbdf36101), WTC(0xbdea0681), WTC(0xbde0b301),
    WTC(0xbdd76701), WTC(0xbdce2181), WTC(0xbdc4e301), WTC(0xbdbbab01),
    WTC(0xbdb27b01), WTC(0xbda95301), WTC(0xbda03381), WTC(0xbd971c81),
    WTC(0xbd8e0e01), WTC(0xbd850701), WTC(0xbd7c0781), WTC(0xbd731081),
    WTC(0xbd6a2201), WTC(0xbd613d81), WTC(0xbd586281), WTC(0xbd4f9101),
    WTC(0xbd46c801), WTC(0xbd3e0801), WTC(0xbd355081), WTC(0xbd2ca281),
    WTC(0xbd23ff01), WTC(0xbd1b6501), WTC(0xbd12d581), WTC(0xbd0a4f81),
    WTC(0xbd01d281), WTC(0xbcf95e81), WTC(0xbcf0f381), WTC(0xbce89281),
    WTC(0xbce03b81), WTC(0xbcd7ef01), WTC(0xbccfac01), WTC(0xbcc77181),
    WTC(0xbcbf4001), WTC(0xbcb71701), WTC(0xbcaef701), WTC(0xbca6e101),
    WTC(0xbc9ed481), WTC(0xbc96d101), WTC(0xbc8ed701), WTC(0xbc86e581),
    WTC(0xbc7efc81), WTC(0xbc771c01), WTC(0xbc6f4401), WTC(0xbc677501),
    WTC(0xbc5fae81), WTC(0xbc57f101), WTC(0xbc503b81), WTC(0xbc488e81),
    WTC(0xbc40e881), WTC(0xbc394901), WTC(0xbc31af01), WTC(0xbc2a1a81),
    WTC(0xbc228f01), WTC(0xbc1b1081), WTC(0xbc13a481), WTC(0xbc0c4581),
    WTC(0xbc04e381), WTC(0xbbfd6c01), WTC(0xbbf5d181), WTC(0xbbee2f81),
    WTC(0xbbe6c801), WTC(0xbbdfdb81), WTC(0xbbd9a781), WTC(0xbbd45881),
    WTC(0xbbd01301), WTC(0xbbccfc81), WTC(0xbbcb2281), WTC(0xbbca5d01),
    WTC(0xbbca7481), WTC(0xbbcb3201), WTC(0xbbcc6b01), WTC(0xbbce0601),
    WTC(0xbbcfea81), WTC(0xbbd20301), WTC(0xbbd45601), WTC(0xbbd70201),
    WTC(0xbbda2501), WTC(0xbbdddb01), WTC(0xbbe23281), WTC(0xbbe73201),
    WTC(0xbbece281), WTC(0xbbf34281), WTC(0xbbfa3c01), WTC(0xbc01b381),
    WTC(0xbc098d81), WTC(0xbc11b681), WTC(0xbc1a2401), WTC(0xbc22cd81),
    WTC(0xbc2bab01), WTC(0xbc34c081), WTC(0xbc3e1981), WTC(0xbc47c281),
    WTC(0xbc51cb01), WTC(0xbc5c4c81), WTC(0xbc676501), WTC(0xbc733401),
    WTC(0xbc7fd301), WTC(0xbc8d5101), WTC(0xbc9bb901), WTC(0xbcab1781),
    WTC(0xbcbb7001), WTC(0xbcccbd01), WTC(0xbcdef701), WTC(0xbcf21601),
    WTC(0xbd060c81), WTC(0xbd1ac801), WTC(0xbd303581), WTC(0xbd464281),
    WTC(0xbd5ce281), WTC(0xbd740b81), WTC(0xbd8bb281), WTC(0xbda3d081),
    WTC(0xbdbc6381), WTC(0xbdd56b81), WTC(0xbdeee981), WTC(0xbe08e181),
    WTC(0xbe236001), WTC(0xbe3e7201), WTC(0xbe5a2301), WTC(0xbe767e81),
    WTC(0xbe938c81), WTC(0xbeb15701), WTC(0xbecfe601), WTC(0xbeef4601),
    WTC(0xbf0f8301), WTC(0xbf30a901), WTC(0xbf52c101), WTC(0xbf75cc81),
    WTC(0xbf99cb01), WTC(0xbfbebb81), WTC(0xbfe48981), WTC(0xc00b04c0),
    WTC(0xc031f880), WTC(0xc0593340), WTC(0xc0809280), WTC(0xc0a802c0),
    WTC(0xc0cf6ec0), WTC(0xc0f6cc00), WTC(0xc11e3a80), WTC(0xc145f040),
    WTC(0xc16e22c0), WTC(0xc196fb00), WTC(0xc1c08680), WTC(0xc1eaca00),
    WTC(0xc215cbc0), WTC(0xc2418940), WTC(0xc26df5c0), WTC(0xc29b02c0),
    WTC(0xc2c8a140), WTC(0xc2f6b500), WTC(0xc3251740), WTC(0xc353a0c0),
    WTC(0xc3822c00), WTC(0xc3b09940), WTC(0xc3deccc0), WTC(0xc40ca800),
    WTC(0xc43a28c0), WTC(0xc4678a00), WTC(0xc4951780), WTC(0xc4c31d00),
    WTC(0xc4f1bdc0), WTC(0xc520e840), WTC(0xc5508440), WTC(0xc5807900),
    WTC(0xc5b09e80), WTC(0xc5e0bfc0), WTC(0xc610a740), WTC(0xc64029c0),
    WTC(0xc66f49c0), WTC(0xc69e2180), WTC(0xc6ccca40), WTC(0xc6fb5700),
    WTC(0xc729cc80), WTC(0xc7582b40), WTC(0xc7867480), WTC(0xc7b4a480),
    WTC(0xc7e2afc0), WTC(0xc8108a80), WTC(0xc83e28c0), WTC(0xc86b7f00),
    WTC(0xc8988100), WTC(0xc8c52340), WTC(0xc8f15980), WTC(0xc91d1840),
    WTC(0xb4d6a381), WTC(0xb4422b81), WTC(0xb3ae8601), WTC(0xb31bb301),
    WTC(0xb289b181), WTC(0xb1f88181), WTC(0xb1682281), WTC(0xb0d89401),
    WTC(0xb049d601), WTC(0xafbbe801), WTC(0xaf2ec901), WTC(0xaea27681),
    WTC(0xae16f001), WTC(0xad8c3301), WTC(0xad023f01), WTC(0xac791401),
    WTC(0xabf0b181), WTC(0xab691681), WTC(0xaae24301), WTC(0xaa5c3601),
    WTC(0xa9d6ef01), WTC(0xa9526d81), WTC(0xa8ceb201), WTC(0xa84bbb81),
    WTC(0xa7c98b01), WTC(0xa7482101), WTC(0xa6c77e01), WTC(0xa647a301),
    WTC(0xa5c89001), WTC(0xa54a4701), WTC(0xa4ccc901), WTC(0xa4501601),
    WTC(0xa3d43001), WTC(0xa3591801), WTC(0xa2dece81), WTC(0xa2655581),
    WTC(0xa1ecae01), WTC(0xa174da81), WTC(0xa0fddd81), WTC(0xa087b981),
    WTC(0xa0127081), WTC(0x9f9e0301), WTC(0x9f2a7281), WTC(0x9eb7c101),
    WTC(0x9e45f081), WTC(0x9dd50481), WTC(0x9d650081), WTC(0x9cf5e701),
    WTC(0x9c87ba81), WTC(0x9c1a7c81), WTC(0x9bae2f81), WTC(0x9b42d581),
    WTC(0x9ad87081), WTC(0x9a6f0381), WTC(0x9a069001), WTC(0x999f1981),
    WTC(0x9938a281), WTC(0x98d32d81), WTC(0x986ebd81), WTC(0x980b5501),
    WTC(0x97a8f681), WTC(0x9747a481), WTC(0x96e76101), WTC(0x96882e01),
    WTC(0x962a0c81), WTC(0x95ccff01), WTC(0x95710601), WTC(0x95162381),
    WTC(0x94bc5981), WTC(0x9463a881), WTC(0x940c1281), WTC(0x93b59901),
    WTC(0x93603d01), WTC(0x930bff81), WTC(0x92b8e101), WTC(0x9266e281),
    WTC(0x92160301), WTC(0x91c64301), WTC(0x9177a301), WTC(0x912a2201),
    WTC(0x90ddc001), WTC(0x90927b81), WTC(0x90485401), WTC(0x8fff4601),
    WTC(0x8fb74f81), WTC(0x8f706f01), WTC(0x8f2aa101), WTC(0x8ee5e301),
    WTC(0x8ea23201), WTC(0x8e5f8881), WTC(0x8e1de001), WTC(0x8ddd3201),
    WTC(0x8d9d7781), WTC(0x8d5eaa01), WTC(0x8d20c301), WTC(0x8ce3ba81),
    WTC(0x8ca78781), WTC(0x8c6c1b01), WTC(0x8c316681), WTC(0x8bf75b01),
    WTC(0x8bbde981), WTC(0x8b850281), WTC(0x8b4c9701), WTC(0x8b149701),
    WTC(0x8adcee01), WTC(0x8aa58681), WTC(0x8a6e4a01), WTC(0x8a372881),
    WTC(0x8a001f01), WTC(0x89c92f81), WTC(0x89925a81), WTC(0x895bcd01),
    WTC(0x8925f101), WTC(0x88f13801), WTC(0x88be1681), WTC(0x888d3181),
    WTC(0x885f8481), WTC(0x88353501), WTC(0x88124281), WTC(0x87e73d81),
    WTC(0x87d4ac81), WTC(0x87cb5101), WTC(0x87c05e81), WTC(0x87b42481),
    WTC(0x87a70e81), WTC(0x87998f01), WTC(0x878c1881), WTC(0x877ede01),
    WTC(0x8771c601), WTC(0x8764b101), WTC(0x87578181), WTC(0x874a2f01),
    WTC(0x873cc201), WTC(0x872f4201), WTC(0x8721b481), WTC(0x87141b01),
    WTC(0x87067281), WTC(0x86f8ba81), WTC(0x86eaf081), WTC(0x86dd1481),
    WTC(0x86cf2601), WTC(0x86c12401), WTC(0x86b30f01), WTC(0x86a4e781),
    WTC(0x8696ad01), WTC(0x86886001), WTC(0x867a0081), WTC(0x866b8d81),
    WTC(0x865d0581), WTC(0x864e6901), WTC(0x863fb701), WTC(0x8630f181),
    WTC(0x86221801), WTC(0x86132c01), WTC(0x86042c01), WTC(0x85f51681),
    WTC(0x85e5eb81), WTC(0x85d6a981), WTC(0x85c75201), WTC(0x85b7e601),
    WTC(0x85a86581), WTC(0x8598d081), WTC(0x85892681), WTC(0x85796601),
    WTC(0x85698e81), WTC(0x8559a081), WTC(0x85499d01), WTC(0x85398481),
    WTC(0x85295881), WTC(0x85191801), WTC(0x8508c181), WTC(0x84f85581),
    WTC(0x84e7d381), WTC(0x84d73c01), WTC(0x84c69101), WTC(0x84b5d301),
    WTC(0x84a50201), WTC(0x84941d81), WTC(0x84832481), WTC(0x84721701),
    WTC(0x8460f581), WTC(0x844fc081), WTC(0x843e7a81), WTC(0x842d2281),
    WTC(0x841bb981), WTC(0x840a3e81), WTC(0x83f8b001), WTC(0x83e70f01),
    WTC(0x83d55d01), WTC(0x83c39a81), WTC(0x83b1c881), WTC(0x839fe801),
    WTC(0x838df801), WTC(0x837bf801), WTC(0x8369e781), WTC(0x8357c701),
    WTC(0x83459881), WTC(0x83335c81), WTC(0x83211501), WTC(0x830ec081),
    WTC(0x82fc5f01), WTC(0x82e9ef01), WTC(0x82d77201), WTC(0x82c4e801),
    WTC(0x82b25301), WTC(0x829fb401), WTC(0x828d0b01), WTC(0x827a5801),
    WTC(0x82679901), WTC(0x8254cf01), WTC(0x8241fa01), WTC(0x822f1b01),
    WTC(0x821c3401), WTC(0x82094581), WTC(0x81f64f01), WTC(0x81e34f81),
    WTC(0x81d04681), WTC(0x81bd3401), WTC(0x81aa1981), WTC(0x8196f781),
    WTC(0x8183cf81), WTC(0x8170a181), WTC(0x815d6c01), WTC(0x814a2f81),
    WTC(0x8136ea01), WTC(0x81239d81), WTC(0x81104a01), WTC(0x80fcf181),
    WTC(0x80e99401), WTC(0x80d63101), WTC(0x80c2c781), WTC(0x80af5701),
    WTC(0x809bdf01), WTC(0x80886081), WTC(0x8074dc01), WTC(0x80615281),
    WTC(0x804dc481), WTC(0x803a3381), WTC(0x80269f81), WTC(0x80130981),
    WTC(0x0a608220), WTC(0x0a8ee7d0), WTC(0x0abe35c0), WTC(0x0aee5de0),
    WTC(0x0b1f5230), WTC(0x0b5104a0), WTC(0x0b836720), WTC(0x0bb66bb0),
    WTC(0x0bea0440), WTC(0x0c1e22c0), WTC(0x0c52ba70), WTC(0x0c87ca90),
    WTC(0x0cbd5ba0), WTC(0x0cf375e0), WTC(0x0d2a1f50), WTC(0x0d615480),
    WTC(0x0d990e40), WTC(0x0dd14500), WTC(0x0e09f730), WTC(0x0e432e90),
    WTC(0x0e7cf790), WTC(0x0eb75e50), WTC(0x0ef26430), WTC(0x0f2dfd70),
    WTC(0x0f6a1d70), WTC(0x0fa6b7e0), WTC(0x0fe3c3d0), WTC(0x10213ac0),
    WTC(0x105f1640), WTC(0x109d4f20), WTC(0x10dbdb80), WTC(0x111ab0c0),
    WTC(0x1159c360), WTC(0x11990fc0), WTC(0x11d8a060), WTC(0x121882c0),
    WTC(0x1258c480), WTC(0x12995a40), WTC(0x12da1b00), WTC(0x131adb60),
    WTC(0x135b70c0), WTC(0x139bb680), WTC(0x13db8c00), WTC(0x141ad080),
    WTC(0x14596460), WTC(0x149729e0), WTC(0x14d404e0), WTC(0x150fd8e0),
    WTC(0x154a88c0), WTC(0x1583f5e0), WTC(0x15bc0120), WTC(0x15f28ba0),
    WTC(0x162779a0), WTC(0x165ab300), WTC(0x168c2040), WTC(0x16bbaa80),
    WTC(0x16e94120), WTC(0x1714d9e0), WTC(0x173e6440), WTC(0x17660680),
    WTC(0x178ca020), WTC(0x17b36400), WTC(0x17db84e0), WTC(0x1805d920),
    WTC(0x18328400), WTC(0x18617cc0), WTC(0x1892bfa0), WTC(0x18c64540),
    WTC(0x18fc0400), WTC(0x1933f140), WTC(0x196e0320), WTC(0x19aa2fc0),
    WTC(0x19e86d80), WTC(0x1a28b2e0), WTC(0x1a6af700), WTC(0x1aaf3320),
    WTC(0x1af56180), WTC(0x1b3d7ce0), WTC(0x1b877c40), WTC(0x1bd350c0),
    WTC(0x1c20ea40), WTC(0x1c703840), WTC(0x1cc13860), WTC(0x1d13f760),
    WTC(0x1d688420), WTC(0x1dbeed40), WTC(0x1e174660), WTC(0x1e71a640),
    WTC(0x1ece2400), WTC(0x1f2cd220), WTC(0x1f8db3c0), WTC(0x1ff0c3e0),
    WTC(0x20560080), WTC(0x20bd46c0), WTC(0x21263400), WTC(0x21905740),
    WTC(0x21fb4100), WTC(0x2266ba80), WTC(0x22d2d140), WTC(0x233f9780),
    WTC(0x23ad25c0), WTC(0x241bc800), WTC(0x248bf040), WTC(0x24fe1380),
    WTC(0x25728180), WTC(0x25e90a00), WTC(0x26614080), WTC(0x26dabdc0),
    WTC(0x27552540), WTC(0x27d03200), WTC(0x284ba580), WTC(0x28c740c0),
    WTC(0x29431f80), WTC(0x29bfc9c0), WTC(0x2a3dd080), WTC(0x2abdc000),
    WTC(0x2b3ffd00), WTC(0x2bc4cd80), WTC(0x2c4c7d40), WTC(0x2cd72ec0),
    WTC(0x2d647f80), WTC(0x2df3cd80), WTC(0x2e847d80), WTC(0x2f15ea40),
    WTC(0x2fa760c0), WTC(0x30382b80), WTC(0x30c79440), WTC(0x315566c0),
    WTC(0x31e20800), WTC(0x326de7c0), WTC(0x32f98200), WTC(0x3385ba00),
    WTC(0x3413bec0), WTC(0x34a4c480), WTC(0x3539bf00), WTC(0x35d2c4c0),
    WTC(0x366f8340), WTC(0x370fb800), WTC(0x37b2cf80), WTC(0x3857a480),
    WTC(0x38fcee80), WTC(0x39a16840), WTC(0x3a4422c0), WTC(0x3ae495c0),
    WTC(0x3b824000), WTC(0x3c1cb500), WTC(0x3cb438c0), WTC(0x3d4994c0),
    WTC(0x3ddd8f40), WTC(0x3e70ec00), WTC(0x3f045e40), WTC(0x3f989080),
    WTC(0x402e32ff), WTC(0x40c5c07f), WTC(0x415f547f), WTC(0x41faf07f),
    WTC(0x4298997f), WTC(0x4338307f), WTC(0x43d96bff), WTC(0x447bffff),
    WTC(0x451f9cff), WTC(0x45c3daff), WTC(0x46683eff), WTC(0x470c4cff),
    WTC(0x47af93ff), WTC(0x4851c3ff), WTC(0x48f29d7f), WTC(0x4991de7f),
    WTC(0x4a2f5e7f), WTC(0x4acb287f), WTC(0x4b65537f), WTC(0x4bfdf37f),
    WTC(0x4c95337f), WTC(0x4d2b51ff), WTC(0x4dc091ff), WTC(0x4e5533ff),
    WTC(0x4ee96b7f), WTC(0x4f7d61ff), WTC(0x501140ff), WTC(0x50a5317f),
    WTC(0x51395a7f), WTC(0x51cddf7f), WTC(0x5262e6ff), WTC(0x52f885ff),
    WTC(0x538eb47f), WTC(0x542560ff), WTC(0x54bc7b7f), WTC(0x5553a8ff),
    WTC(0x55ea35ff), WTC(0x567f66ff), WTC(0x5712897f), WTC(0x57a33a7f),
    WTC(0x583152ff), WTC(0x58bca5ff), WTC(0x594530ff), WTC(0x59cb79ff),
    WTC(0x5a5047ff), WTC(0x5ad45eff), WTC(0x5b584e7f), WTC(0x5bdc417f),
    WTC(0x5c60487f), WTC(0x5ce476ff), WTC(0x5d68c47f), WTC(0x5ded06ff),
    WTC(0x5e7111ff), WTC(0x5ef4b5ff), WTC(0x5f77a17f), WTC(0x5ff96aff),
    WTC(0x6079a7ff), WTC(0x60f7f7ff), WTC(0x617417ff), WTC(0x61edd87f),
    WTC(0x6264ffff), WTC(0x62d9a6ff), WTC(0x634c817f), WTC(0x63be657f),
    WTC(0x6430277f), WTC(0x64a2247f), WTC(0x65142bff), WTC(0x6586027f),
    WTC(0x65f7697f), WTC(0x666801ff), WTC(0x66d756ff), WTC(0x6744f0ff),
    WTC(0x67b0787f), WTC(0x681a077f), WTC(0x6881ebff), WTC(0x68e8707f),
    WTC(0x694dceff), WTC(0x69b21e7f), WTC(0x6a156cff), WTC(0x6a77ca7f),
    WTC(0x6ad9377f), WTC(0x6b39a4ff), WTC(0x6b9901ff), WTC(0x6bf73cff),
    WTC(0x6c54457f), WTC(0x6cb00aff), WTC(0x6d0a7bff), WTC(0x6d6387ff),
    WTC(0xae2cbe01), WTC(0xaf526d01), WTC(0xb0751201), WTC(0xb194da81),
    WTC(0xb2b1f401), WTC(0xb3cc8d01), WTC(0xb4e4d201), WTC(0xb5faf101),
    WTC(0xb70f1881), WTC(0xb8217301), WTC(0xb9321181), WTC(0xba40ee01),
    WTC(0xbb4e0201), WTC(0xbc594781), WTC(0xbd62b881), WTC(0xbe6a5181),
    WTC(0xbf700d01), WTC(0xc073e4c0), WTC(0xc175d240), WTC(0xc275cc80),
    WTC(0xc373cb80), WTC(0xc46fca00), WTC(0xc569c600), WTC(0xc661bdc0),
    WTC(0xc757af80), WTC(0xc84b9840), WTC(0xc93d7300), WTC(0xca2d3a40),
    WTC(0xcb1aea40), WTC(0xcc068280), WTC(0xccf00480), WTC(0xcdd77200),
    WTC(0xcebccb40), WTC(0xcfa00d80), WTC(0xd0813540), WTC(0xd1603f00),
    WTC(0xd23d2980), WTC(0xd317f7c0), WTC(0xd3f0ac40), WTC(0xd4c74980),
    WTC(0xd59bcf80), WTC(0xd66e3b00), WTC(0xd73e8900), WTC(0xd80cb740),
    WTC(0xd8d8c7c0), WTC(0xd9a2be00), WTC(0xda6a9e40), WTC(0xdb306a40),
    WTC(0xdbf42080), WTC(0xdcb5be80), WTC(0xdd754140), WTC(0xde32a900),
    WTC(0xdeedf9c0), WTC(0xdfa737c0), WTC(0xe05e6740), WTC(0xe1138900),
    WTC(0xe1c69ac0), WTC(0xe2779a40), WTC(0xe3268680), WTC(0xe3d36260),
    WTC(0xe47e33a0), WTC(0xe526ff80), WTC(0xe5cdc960), WTC(0xe6729100),
    WTC(0xe7155460), WTC(0xe7b611c0), WTC(0xe854ca20), WTC(0xe8f18180),
    WTC(0xe98c3ca0), WTC(0xea24ffe0), WTC(0xeabbcb20), WTC(0xeb509b60),
    WTC(0xebe36d00), WTC(0xec743e00), WTC(0xed0310e0), WTC(0xed8feaa0),
    WTC(0xee1ad060), WTC(0xeea3c640), WTC(0xef2acd60), WTC(0xefafe6a0),
    WTC(0xf03312f0), WTC(0xf0b45800), WTC(0xf133c230), WTC(0xf1b15ef0),
    WTC(0xf22d3af0), WTC(0xf2a75c80), WTC(0xf31fc460), WTC(0xf39673b0),
    WTC(0xf40b6a00), WTC(0xf47ea230), WTC(0xf4f01450), WTC(0xf55fb930),
    WTC(0xf5cd84c0), WTC(0xf6396090), WTC(0xf6a333e0), WTC(0xf70ae540),
    WTC(0xf7707260), WTC(0xf7d3f720), WTC(0xf83592f0), WTC(0xf8956450),
    WTC(0xf8f38120), WTC(0xf94ff7c8), WTC(0xf9aad740), WTC(0xfa042920),
    WTC(0xfa5be110), WTC(0xfab1e778), WTC(0xfb062478), WTC(0xfb588d78),
    WTC(0xfba93530), WTC(0xfbf836c8), WTC(0xfc45ace0), WTC(0xfc91a294),
    WTC(0xfcdc0e5c), WTC(0xfd24e438), WTC(0xfd6c17dc), WTC(0xfdb19758),
    WTC(0xfdf54c3c), WTC(0xfe371ef8), WTC(0xfe7701aa), WTC(0xfeb50d62),
    WTC(0xfef1700a), WTC(0xff2c5574), WTC(0xff65ee7b), WTC(0xff9e75de),
    WTC(0xffd62863), WTC(0x000d4401), WTC(0x0043d345), WTC(0x00799cd0),
    WTC(0x00ae5f49), WTC(0x00e1d7a4), WTC(0x0113a6f2), WTC(0x0143575c),
    WTC(0x01707024), WTC(0x019a9346), WTC(0x01c1cf08), WTC(0x01e66c12),
    WTC(0x0208ac48), WTC(0x0228e868), WTC(0x0247a6c8), WTC(0x02657aa0),
    WTC(0x0282f710), WTC(0x02a07e50), WTC(0x02be31c0), WTC(0x02dc2b30),
    WTC(0x02fa7f34), WTC(0x0318fb10), WTC(0x03372fdc), WTC(0x0354ae54),
    WTC(0x03710d18), WTC(0x038bfdb4), WTC(0x03a54084), WTC(0x03bc92b8),
    WTC(0x03d1c710), WTC(0x03e4dd20), WTC(0x03f5e25c), WTC(0x0404e218),
    WTC(0x0411fc30), WTC(0x041d6b30), WTC(0x04276cd0), WTC(0x04303e00),
    WTC(0x04381528), WTC(0x043f2310), WTC(0x04459908), WTC(0x044ba430),
    WTC(0x045161f8), WTC(0x0456e6f8), WTC(0x045c49a8), WTC(0x046192f8),
    WTC(0x0466af40), WTC(0x046b8240), WTC(0x046ff0d8), WTC(0x0473de18),
    WTC(0x04772b58), WTC(0x0479b9a0), WTC(0x047b6a30), WTC(0x047c2088),
    WTC(0x047bc230), WTC(0x047a3418), WTC(0x04776098), WTC(0x04734790),
    WTC(0x046df4c0), WTC(0x04677220), WTC(0x045fd1b0), WTC(0x04573588),
    WTC(0x044dc4b8), WTC(0x0443a5b8), WTC(0x04390160), WTC(0x042e0398),
    WTC(0x0422d8c0), WTC(0x0417aa30), WTC(0x040c7ce0), WTC(0x040136e0),
    WTC(0x03f5beb0), WTC(0x03e9f8ec), WTC(0x03ddc484), WTC(0x03d0fd9c),
    WTC(0x03c37fa0), WTC(0x03b53014), WTC(0x03a60a18), WTC(0x03960f88),
    WTC(0x03854110), WTC(0x0373ad9c), WTC(0x03617694), WTC(0x034ebf9c),
    WTC(0x033bab30), WTC(0x03284ef0), WTC(0x0314b598), WTC(0x0300ea54),
    WTC(0x02ecf524), WTC(0x02d8d210), WTC(0x02c476ac), WTC(0x02afd940),
    WTC(0x029aee4c), WTC(0x0285a6f4), WTC(0x026ff398), WTC(0x0259c448),
    WTC(0x024317cc), WTC(0x022c0084), WTC(0x02149310), WTC(0x01fce334),
    WTC(0x01e4fb24), WTC(0x01ccdd0a), WTC(0x01b48b20), WTC(0x019c077e),
    WTC(0x01835432), WTC(0x016a733c), WTC(0x015166a6), WTC(0x0138302e),
    WTC(0x011ed0f6), WTC(0x010549f8), WTC(0x00eb9c25), WTC(0x00d1caa6),
    WTC(0x00b7db94), WTC(0x009dd560), WTC(0x0083be75), WTC(0x00699d41),
    WTC(0x004f782f), WTC(0x003555ab), WTC(0x001b3c21), WTC(0x000131fe),
    WTC(0xd61cfc40), WTC(0xd5acb340), WTC(0xd53d4400), WTC(0xd4cea6c0),
    WTC(0xd460d440), WTC(0xd3f3c440), WTC(0xd3876f80), WTC(0xd31bce40),
    WTC(0xd2b0d900), WTC(0xd2468980), WTC(0xd1dcef00), WTC(0xd17429c0),
    WTC(0xd10c5b80), WTC(0xd0a59b80), WTC(0xd03fd780), WTC(0xcfdae780),
    WTC(0xcf76a380), WTC(0xcf12fac0), WTC(0xceb01100), WTC(0xce4e18c0),
    WTC(0xcded4440), WTC(0xcd8d9a40), WTC(0xcd2ee800), WTC(0xccd0f440),
    WTC(0xcc738780), WTC(0xcc167d40), WTC(0xcbb9c180), WTC(0xcb5d4040),
    WTC(0xcb00e240), WTC(0xcaa48000), WTC(0xca47eac0), WTC(0xc9eaf1c0),
    WTC(0xc98d8100), WTC(0xc92fc580), WTC(0xc8d1fc80), WTC(0xc8746480),
    WTC(0xc816dc40), WTC(0xc7b8c280), WTC(0xc7596800), WTC(0xc6f81f80),
    WTC(0xc6945740), WTC(0xc62d93c0), WTC(0xc5c358c0), WTC(0xc5552b80),
    WTC(0xc4e29240), WTC(0xc46b1440), WTC(0xc3ee3840), WTC(0xc36b8500),
    WTC(0xc2e28040), WTC(0xc252ae80), WTC(0xc1bb9540), WTC(0xc11cc200),
    WTC(0xc075cf00), WTC(0xbfc65781), WTC(0xbf0df881), WTC(0xbe4c6f01),
    WTC(0xbd819401), WTC(0xbcad2d01), WTC(0xbbcfb981), WTC(0xbaeca681),
    WTC(0xba08e781), WTC(0xb9297081), WTC(0xb851e081), WTC(0xb782ed01),
    WTC(0xb6bc6a81), WTC(0xb5fe4981), WTC(0xb5487281), WTC(0xb49ad081),
    WTC(0xb3f54d81), WTC(0xb357d401), WTC(0xb2c24e01), WTC(0xb234a681),
    WTC(0xb1aec701), WTC(0xb1309b01), WTC(0xb0ba0c01), WTC(0xb04b0481),
    WTC(0xafe36f01), WTC(0xaf833601), WTC(0xaf2a4381), WTC(0xaed88201),
    WTC(0xae8ddb81), WTC(0xae4a3b81), WTC(0xae0d8b01), WTC(0xadd7b581),
    WTC(0xada8a481), WTC(0xad804281), WTC(0xad5e7a81), WTC(0xad433601),
    WTC(0xad2e6001), WTC(0xad1fe281), WTC(0xad17a801), WTC(0xad159a81),
    WTC(0xad19a501), WTC(0xad23b101), WTC(0xad33aa01), WTC(0xad497981),
    WTC(0xad650a01), WTC(0xad864601), WTC(0xadad1781), WTC(0xadd96981),
    WTC(0xae0b2601), WTC(0xae423781), WTC(0xae7e8801), WTC(0xaec00201),
    WTC(0xaf069081), WTC(0xaf521c81), WTC(0xafa29201), WTC(0xaff7da01),
    WTC(0xb051df01), WTC(0xb0b08c81), WTC(0xb113cb81), WTC(0xb17b8701),
    WTC(0xb1e7a981), WTC(0xb2581d81), WTC(0xb2cccc81), WTC(0xb345a181),
    WTC(0xb3c28701), WTC(0xb4436681), WTC(0xb4c82b81), WTC(0xb550bf81),
    WTC(0xb5dd0d01), WTC(0xb66cff01), WTC(0xb7007f01), WTC(0xb7977781),
    WTC(0xb831d381), WTC(0xb8cf7d01), WTC(0xb9705e01), WTC(0xba146101),
    WTC(0xbabb7081), WTC(0xbb657781), WTC(0xbc125f01), WTC(0xbcc21281),
    WTC(0xbd747b81), WTC(0xbe298581), WTC(0xbee11981), WTC(0xbf9b2301),
    WTC(0xc0578b80), WTC(0xc1163dc0), WTC(0xc1d72400), WTC(0xc29a28c0),
    WTC(0xc35f3640), WTC(0xc42636c0), WTC(0xc4ef1500), WTC(0xc5b9bb00),
    WTC(0xc6861340), WTC(0xc7540840), WTC(0xc8238400), WTC(0xc8f47100),
    WTC(0xc9c6b9c0), WTC(0xca9a4840), WTC(0xcb6f0780), WTC(0xcc44e140),
    WTC(0xcd1bc000), WTC(0xcdf38e00), WTC(0xcecc3600), WTC(0xcfa5a240),
    WTC(0xd07fbcc0), WTC(0xd15a7040), WTC(0xd235a6c0), WTC(0xd3114b00),
    WTC(0xd3ed4740), WTC(0xd4c98580), WTC(0xd5a5f080), WTC(0xd6827280),
    WTC(0xd75ef600), WTC(0xd83b6500), WTC(0xd917aa00), WTC(0xd9f3af80),
    WTC(0xdacf5fc0), WTC(0xdbaaa540), WTC(0xdc856a00), WTC(0xdd5f98c0),
    WTC(0xde391bc0), WTC(0xdf11dd40), WTC(0xdfe9c780), WTC(0xe0c0c540),
    WTC(0xe196c080), WTC(0xe26ba3c0), WTC(0xe33f5960), WTC(0xe411cba0),
    WTC(0xe4e2e500), WTC(0xe5b28fc0), WTC(0xe680b640), WTC(0xe74d42e0),
    WTC(0xe8181fe0), WTC(0xe8e137e0), WTC(0xe9a87500), WTC(0xea6dc1a0),
    WTC(0xeb310820), WTC(0xebf23300), WTC(0xecb12c60), WTC(0xed6ddee0),
    WTC(0xee2834a0), WTC(0xeee01800), WTC(0xef957380), WTC(0xf0483160),
    WTC(0xf0f83c00), WTC(0xf1a57db0), WTC(0xf24fe0f0), WTC(0xf2f74ff0),
    WTC(0xf39bb530), WTC(0xf43cfaf0), WTC(0xf4db0b90), WTC(0xf575d180),
    WTC(0xf60d3700), WTC(0xf6a12680), WTC(0xf7318a50), WTC(0xf7be4cc0),
    WTC(0xf8475850), WTC(0xf8cc9738), WTC(0xf94df3e0), WTC(0xf9cb58a8),
    WTC(0xfa44afe0), WTC(0xfab9e3e8), WTC(0xfb2adf20), WTC(0xfb978be8),
    WTC(0xfbffd488), WTC(0xfc63a370), WTC(0xfcc2e2f0), WTC(0xfd1d7d64),
    WTC(0xfd735d2c), WTC(0xfdc46c9c), WTC(0xfe109618), WTC(0xfe57c3f4),
    WTC(0xfe99e090), WTC(0xfed6d644), WTC(0xff0e8f6e), WTC(0xff40f667),
    WTC(0xff6df58c), WTC(0xff957738), WTC(0xffb765c5), WTC(0xffd3ab90),
    WTC(0xffea32f4), WTC(0xfffae64c), WTC(0x0005aff3), WTC(0x000a7a44),
    WTC(0x00092f9c), WTC(0x0001ba54), WTC(0xfff404ca), WTC(0xffdff957)};

/*
 * TNS_MAX_BANDS
 * entry for each sampling rate
 *  1  long window
 *  2  SHORT window
 */
const UCHAR tns_max_bands_tbl[13][2] = {
    {31, 9},  /* 96000 */
    {31, 9},  /* 88200 */
    {34, 10}, /* 64000 */
    {40, 14}, /* 48000 */
    {42, 14}, /* 44100 */
    {51, 14}, /* 32000 */
    {46, 14}, /* 24000 */
    {46, 14}, /* 22050 */
    {42, 14}, /* 16000 */
    {42, 14}, /* 12000 */
    {42, 14}, /* 11025 */
    {39, 14}, /*  8000 */
    {39, 14}, /*  7350 */
};

/* TNS_MAX_BANDS for low delay. The array index is the sampleRateIndex */
const UCHAR tns_max_bands_tbl_480[13] = {
    31, /* 96000 */
    31, /* 88200 */
    31, /* 64000 */
    31, /* 48000 */
    32, /* 44100 */
    37, /* 32000 */
    30, /* 24000 */
    30, /* 22050 */
    30, /* 16000 */
    30, /* 12000 */
    30, /* 11025 */
    30, /*  8000 */
    30  /*  7350 */
};
const UCHAR tns_max_bands_tbl_512[13] = {
    31, /* 96000 */
    31, /* 88200 */
    31, /* 64000 */
    31, /* 48000 */
    32, /* 44100 */
    37, /* 32000 */
    31, /* 24000 */
    31, /* 22050 */
    31, /* 16000 */
    31, /* 12000 */
    31, /* 11025 */
    31, /*  8000 */
    31  /*  7350 */
};

#define TCC(x) (FIXP_DBL(x))

const FIXP_TCC FDKaacDec_tnsCoeff3[8] = {
    TCC(0x81f1d1d4), TCC(0x9126146c), TCC(0xadb922c4), TCC(0xd438af1f),
    TCC(0x00000000), TCC(0x3789809b), TCC(0x64130dd4), TCC(0x7cca7016)};
const FIXP_TCC FDKaacDec_tnsCoeff4[16] = {
    TCC(0x808bc842), TCC(0x84e2e58c), TCC(0x8d6b49d1), TCC(0x99da920a),
    TCC(0xa9c45713), TCC(0xbc9ddeb9), TCC(0xd1c2d51b), TCC(0xe87ae53d),
    TCC(0x00000000), TCC(0x1a9cd9b6), TCC(0x340ff254), TCC(0x4b3c8c29),
    TCC(0x5f1f5ebb), TCC(0x6ed9ebba), TCC(0x79bc385f), TCC(0x7f4c7e5b)};

const UCHAR FDKaacDec_tnsCoeff3_gain_ld[] = {
    3, 1, 1, 1, 0, 1, 1, 3,
};
const UCHAR FDKaacDec_tnsCoeff4_gain_ld[] = {
    4, 2, 2, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 2, 4,
};

/* Lookup tables for elements in ER bitstream */
const MP4_ELEMENT_ID
    elementsTab[AACDEC_MAX_CH_CONF][AACDEC_CH_ELEMENTS_TAB_SIZE] = {
        /*  1 */ {ID_SCE, ID_EXT, ID_END, ID_NONE, ID_NONE, ID_NONE,
                  ID_NONE}, /* 1 channel  */
                            /*  2 */
        {ID_CPE, ID_EXT, ID_END, ID_NONE, ID_NONE, ID_NONE,
         ID_NONE} /* 2 channels */
#if (AACDEC_MAX_CH_CONF > 2)
        /*  3 */,
        {ID_SCE, ID_CPE, ID_EXT, ID_END, ID_NONE, ID_NONE,
         ID_NONE}, /* 3 channels */
                   /*  4 */
        {ID_SCE, ID_CPE, ID_SCE, ID_EXT, ID_END, ID_NONE,
         ID_NONE}, /* 4 channels */
                   /*  5 */
        {ID_SCE, ID_CPE, ID_CPE, ID_EXT, ID_END, ID_NONE,
         ID_NONE}, /* 5 channels */
                   /*  6 */
        {ID_SCE, ID_CPE, ID_CPE, ID_LFE, ID_EXT, ID_END,
         ID_NONE} /* 6 channels */
#endif
#if (AACDEC_MAX_CH_CONF > 6)
        /*  7 */,
        {ID_SCE, ID_CPE, ID_CPE, ID_CPE, ID_LFE, ID_EXT,
         ID_END}, /* 8 channels */
                  /*  8 */
        {ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE,
         ID_NONE}, /* reserved   */
                   /*  9 */
        {ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE,
         ID_NONE}, /* reserved   */
                   /* 10 */
        {ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE,
         ID_NONE}, /* reserved   */
                   /* 11 */
        {ID_SCE, ID_CPE, ID_CPE, ID_SCE, ID_LFE, ID_EXT,
         ID_END}, /* 7 channels */
                  /* 12 */
        {ID_SCE, ID_CPE, ID_CPE, ID_CPE, ID_LFE, ID_EXT,
         ID_END}, /* 8 channels */
                  /* 13 */
        {ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE, ID_NONE,
         ID_NONE}, /* see elementsChCfg13 */
                   /* 14 */
        {ID_SCE, ID_CPE, ID_CPE, ID_LFE, ID_CPE, ID_EXT,
         ID_END} /* 8 channels */
#endif
};

/*! Random sign bit used for concealment
 */
const USHORT AacDec_randomSign[AAC_NF_NO_RANDOM_VAL / 16] = {
    /*
       sign bits of FDK_sbrDecoder_sbr_randomPhase[] entries:
       LSB ........... MSB  ->    MSB ... LSB
    */
    /* 1001 0111 0011 1100  -> */ 0x3ce9,
    /* 0100 0111 0111 1011  -> */ 0xdee2,
    /* 0001 1100 1110 1011  -> */ 0xd738,
    /* 0001 0011 0110 1001  -> */ 0x96c8,
    /* 0101 0011 1101 0000  -> */ 0x0bca,
    /* 0001 0001 1111 0100  -> */ 0x2f88,
    /* 1110 1100 1110 1101  -> */ 0xb737,
    /* 0010 1010 1011 1001  -> */ 0x9d54,
    /* 0111 1100 0110 1010  -> */ 0x563e,
    /* 1101 0111 0010 0101  -> */ 0xa4eb,
    /* 0001 0101 1011 1100  -> */ 0x3da8,
    /* 0101 0111 1001 1011  -> */ 0xd9ea,
    /* 1101 0100 0101 0101  -> */ 0xaa2b,
    /* 1000 1001 0100 0011  -> */ 0xc291,
    /* 1100 1111 1010 1100  -> */ 0x35f3,
    /* 1100 1010 1110 0010  -> */ 0x4753,
    /* 0110 0001 1010 1000  -> */ 0x1586,
    /* 0011 0101 1111 1100  -> */ 0x3fac,
    /* 0001 0110 1010 0001  -> */ 0x8568,
    /* 0010 1101 0111 0010  -> */ 0x4eb4,
    /* 1101 1010 0100 1001  -> */ 0x925b,
    /* 1100 1001 0000 1110  -> */ 0x7093,
    /* 1000 1100 0110 1010  -> */ 0x5631,
    /* 0000 1000 0110 1101  -> */ 0xb610,
    /* 1000 0001 1111 1011  -> */ 0xdf81,
    /* 1111 0011 0100 0111  -> */ 0xe2cf,
    /* 1000 0001 0010 1010  -> */ 0x5481,
    /* 1101 0101 1100 1111  -> */ 0xf3ab,
    /* 0110 0001 0110 1000  -> */ 0x1686,
    /* 0011 0011 1100 0110  -> */ 0x63cc,
    /* 0011 0111 0101 0110  -> */ 0x6aec,
    /* 1011 0001 1010 0010  -> */ 0x458d};

/* MDST filter coefficients for current window
 * max: 0.635722 => 20 bits (unsigned) necessary for representation
 * min: = -max */
const FIXP_FILT mdst_filt_coef_curr[20][3] = {
    {FILT(0.000000f), FILT(0.000000f), FILT(0.500000f)},
    /*, FILT( 0.000000f), FILT(-0.500000f), FILT( 0.000000f), FILT( 0.000000f) }, */ /* only long / eight short  l:sine r:sine */
    {FILT(0.091497f), FILT(0.000000f), FILT(0.581427f)},
    /*, FILT( 0.000000f), FILT(-0.581427f), FILT( 0.000000f), FILT(-0.091497f) }, */ /*                          l:kbd  r:kbd  */
    {FILT(0.045748f), FILT(0.057238f), FILT(0.540714f)},
    /*, FILT( 0.000000f), FILT(-0.540714f), FILT(-0.057238f), FILT(-0.045748f) }, */ /*                          l:sine r:kbd  */
    {FILT(0.045748f), FILT(-0.057238f), FILT(0.540714f)},
    /*, FILT( 0.000000f), FILT(-0.540714f), FILT( 0.057238f), FILT(-0.045748f) }, */ /*                          l:kbd  r:sine */

    {FILT(0.102658f), FILT(0.103791f), FILT(0.567149f)},
    /*, FILT( 0.000000f), FILT(-0.567149f), FILT(-0.103791f), FILT(-0.102658f) }, */ /* long start                             */
    {FILT(0.150512f), FILT(0.047969f),
     FILT(0.608574f)}, /*, FILT( 0.000000f), FILT(-0.608574f),
                          FILT(-0.047969f), FILT(-0.150512f) }, */
    {FILT(0.104763f), FILT(0.105207f),
     FILT(0.567861f)}, /*, FILT( 0.000000f), FILT(-0.567861f),
                          FILT(-0.105207f), FILT(-0.104763f) }, */
    {FILT(0.148406f), FILT(0.046553f),
     FILT(0.607863f)}, /*, FILT( 0.000000f), FILT(-0.607863f),
                          FILT(-0.046553f), FILT(-0.148406f) }, */

    {FILT(0.102658f), FILT(-0.103791f), FILT(0.567149f)},
    /*, FILT( 0.000000f), FILT(-0.567149f), FILT( 0.103791f), FILT(-0.102658f) }, */ /* long stop                              */
    {FILT(0.150512f), FILT(-0.047969f),
     FILT(0.608574f)}, /*, FILT( 0.000000f), FILT(-0.608574f), FILT(
                          0.047969f), FILT(-0.150512f) }, */
    {FILT(0.148406f), FILT(-0.046553f),
     FILT(0.607863f)}, /*, FILT( 0.000000f), FILT(-0.607863f), FILT(
                          0.046553f), FILT(-0.148406f) }, */
    {FILT(0.104763f), FILT(-0.105207f),
     FILT(0.567861f)}, /*, FILT( 0.000000f), FILT(-0.567861f), FILT(
                          0.105207f), FILT(-0.104763f) }, */

    {FILT(0.205316f), FILT(0.000000f), FILT(0.634298f)},
    /*, FILT( 0.000000f), FILT(-0.634298f), FILT( 0.000000f), FILT(-0.205316f) }, */ /* stop start                             */
    {FILT(0.209526f), FILT(0.000000f),
     FILT(0.635722f)}, /*, FILT( 0.000000f), FILT(-0.635722f), FILT(
                          0.000000f), FILT(-0.209526f) }, */
    {FILT(0.207421f), FILT(0.001416f),
     FILT(0.635010f)}, /*, FILT( 0.000000f), FILT(-0.635010f),
                          FILT(-0.001416f), FILT(-0.207421f) }, */
    {FILT(0.207421f), FILT(-0.001416f),
     FILT(0.635010f)}, /*, FILT( 0.000000f), FILT(-0.635010f), FILT(
                          0.001416f), FILT(-0.207421f) }  */

    {FILT(0.185618f), FILT(0.000000f), FILT(0.627371f)},
    /*, FILT( 0.000000f), FILT(-0.634298f), FILT( 0.000000f), FILT(-0.205316f) }, */ /* stop start   Transform Splitting        */
    {FILT(0.204932f), FILT(0.000000f),
     FILT(0.634159f)}, /*, FILT( 0.000000f), FILT(-0.635722f), FILT(
                          0.000000f), FILT(-0.209526f) }, */
    {FILT(0.194609f), FILT(0.006202f),
     FILT(0.630536f)}, /*, FILT( 0.000000f), FILT(-0.635010f),
                          FILT(-0.001416f), FILT(-0.207421f) }, */
    {FILT(0.194609f), FILT(-0.006202f),
     FILT(0.630536f)}, /*, FILT( 0.000000f), FILT(-0.635010f), FILT(
                          0.001416f), FILT(-0.207421f) }  */
};

/* MDST filter coefficients for previous window
 * max: 0.31831 => 15 bits (unsigned) necessary for representation
 * min: 0.0 */
const FIXP_FILT mdst_filt_coef_prev[6][4] = {
    {FILT(0.000000f), FILT(0.106103f), FILT(0.250000f), FILT(0.318310f)},
    /*, FILT( 0.250000f), FILT( 0.106103f), FILT( 0.000000f) }, */ /* only long
                                                                      / long
                                                                      start /
                                                                      eight
                                                                      short
                                                                      l:sine */
    {FILT(0.059509f), FILT(0.123714f), FILT(0.186579f), FILT(0.213077f)},
    /*, FILT( 0.186579f), FILT( 0.123714f), FILT( 0.059509f) }, */ /*                                       l:kbd
                                                                    */

    {FILT(0.038498f), FILT(0.039212f), FILT(0.039645f), FILT(0.039790f)},
    /*, FILT( 0.039645f), FILT( 0.039212f), FILT( 0.038498f) }, */ /* long stop
                                                                      / stop
                                                                      start
                                                                      l:sine */
    {FILT(0.026142f), FILT(0.026413f), FILT(0.026577f), FILT(0.026631f)},
    /*, FILT( 0.026577f), FILT( 0.026413f), FILT( 0.026142f) }  */ /*                         l:kbd
                                                                    */

    {FILT(0.069608f), FILT(0.075028f), FILT(0.078423f), FILT(0.079580f)},
    /*, FILT( 0.039645f), FILT( 0.039212f), FILT( 0.038498f) }, */ /* Transform
                                                                      splitting
                                                                      l:sine */
    {FILT(0.042172f), FILT(0.043458f), FILT(0.044248f), FILT(0.044514f)},
    /*, FILT( 0.026577f), FILT( 0.026413f), FILT( 0.026142f) }  */ /*                         l:kbd
                                                                    */
};
