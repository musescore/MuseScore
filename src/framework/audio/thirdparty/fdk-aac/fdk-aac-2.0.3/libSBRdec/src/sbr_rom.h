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

/**************************** SBR decoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
\file
\brief Declaration of constant tables
*/
#ifndef SBR_ROM_H
#define SBR_ROM_H

#include "sbrdecoder.h"
#include "env_extr.h"
#include "qmf.h"

#define INV_INT_TABLE_SIZE 49
#define SBR_NF_NO_RANDOM_VAL \
  512 /*!< Size of random number array for noise floor */

/*
  Frequency scales
*/

/* if defined(SBRDEC_RATIO_16_64_ENABLE) ((4) = 4) else ((4) = 2) */
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_16[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_22[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_24[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_32[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_40[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_44[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_48[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_64[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_88[(4) / 2][16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_192[16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_176[16];
extern const UCHAR FDK_sbrDecoder_sbr_start_freq_128[16];

/*
  Low-Power-Profile Transposer
*/
#define NUM_WHFACTOR_TABLE_ENTRIES 9
extern const USHORT
    FDK_sbrDecoder_sbr_whFactorsIndex[NUM_WHFACTOR_TABLE_ENTRIES];
extern const FIXP_DBL
    FDK_sbrDecoder_sbr_whFactorsTable[NUM_WHFACTOR_TABLE_ENTRIES][6];

/*
  Envelope Adjustor
*/
extern const FIXP_SGL FDK_sbrDecoder_sbr_limGains_m[4];
extern const UCHAR FDK_sbrDecoder_sbr_limGains_e[4];
extern const FIXP_SGL FDK_sbrDecoder_sbr_limGainsPvc_m[4];
extern const UCHAR FDK_sbrDecoder_sbr_limGainsPvc_e[4];
extern const FIXP_SGL FDK_sbrDecoder_sbr_limiterBandsPerOctaveDiv4[4];
extern const FIXP_DBL FDK_sbrDecoder_sbr_limiterBandsPerOctaveDiv4_DBL[4];
extern const FIXP_SGL FDK_sbrDecoder_sbr_smoothFilter[4];
extern const FIXP_SGL FDK_sbrDecoder_sbr_randomPhase[SBR_NF_NO_RANDOM_VAL][2];

/*
  Envelope Extractor
*/
extern const int FDK_sbrDecoder_envelopeTable_8[8][5];
extern const int FDK_sbrDecoder_envelopeTable_15[15][6];
extern const int FDK_sbrDecoder_envelopeTable_16[16][6];
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info1_15;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info2_15;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info4_15;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info8_15;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info1_16;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info2_16;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info4_16;
extern const FRAME_INFO FDK_sbrDecoder_sbr_frame_info8_16;

extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvLevel10T[120][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvLevel10F[120][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvBalance10T[48][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvBalance10F[48][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvLevel11T[62][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvLevel11F[62][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvBalance11T[24][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_EnvBalance11F[24][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_NoiseLevel11T[62][2];
extern const SCHAR FDK_sbrDecoder_sbr_huffBook_NoiseBalance11T[24][2];

/*
 Parametric stereo
*/

/* FIX_BORDER can have 0, 1, 2, 4 envelops */
extern const UCHAR FDK_sbrDecoder_aFixNoEnvDecode[4];

/* IID & ICC Huffman codebooks */
extern const SCHAR aBookPsIidTimeDecode[28][2];
extern const SCHAR aBookPsIidFreqDecode[28][2];
extern const SCHAR aBookPsIccTimeDecode[14][2];
extern const SCHAR aBookPsIccFreqDecode[14][2];

/* IID-fine Huffman codebooks */

extern const SCHAR aBookPsIidFineTimeDecode[60][2];
extern const SCHAR aBookPsIidFineFreqDecode[60][2];

/* the values of the following 3 tables are shiftet right by 1 ! */
extern const FIXP_DBL ScaleFactors[NO_IID_LEVELS];
extern const FIXP_DBL ScaleFactorsFine[NO_IID_LEVELS_FINE];
extern const FIXP_DBL Alphas[NO_ICC_LEVELS];

extern const UCHAR bins2groupMap20[NO_IID_GROUPS];
extern const UCHAR FDK_sbrDecoder_aNoIidBins[3];
extern const UCHAR FDK_sbrDecoder_aNoIccBins[3];

/* Lookup tables for some arithmetic functions */

#define INV_TABLE_BITS 8
#define INV_TABLE_SIZE (1 << INV_TABLE_BITS)
extern const FIXP_SGL FDK_sbrDecoder_invTable[INV_TABLE_SIZE];

#endif  // SBR_ROM_H
