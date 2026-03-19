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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  SBR main definitions $Revision: 92790 $
*/
#ifndef SBR_DEF_H
#define SBR_DEF_H

#include "common_fix.h"

#define noError 0
#define HANDLE_ERROR_INFO INT
#define ERROR(a, b) 1

/* #define SBR_ENV_STATISTICS_BITRATE */
#undef SBR_ENV_STATISTICS_BITRATE

/* #define SBR_ENV_STATISTICS */
#undef SBR_ENV_STATISTICS

/* #define SBR_PAYLOAD_MONITOR */
#undef SBR_PAYLOAD_MONITOR

#define SWAP(a, b) tempr = a, a = b, b = tempr
#define TRUE 1
#define FALSE 0

/* Constants */
#define EPS 1e-12
#define LOG2 0.69314718056f /* natural logarithm of 2 */
#define ILOG2 1.442695041f  /* 1/LOG2 */
#define RELAXATION_FLOAT (1e-6f)
#define RELAXATION (FL2FXCONST_DBL(RELAXATION_FLOAT))
#define RELAXATION_FRACT \
  (FL2FXCONST_DBL(0.524288f)) /* 0.524288f is fractional part of RELAXATION */
#define RELAXATION_SHIFT (19)
#define RELAXATION_LD64                                 \
  (FL2FXCONST_DBL(0.31143075889f)) /* (ld64(RELAXATION) \
                                    */

/************  Definitions ***************/
#define SBR_COMP_MODE_DELTA 0
#define SBR_COMP_MODE_CTS 1
#define SBR_MAX_ENERGY_VALUES 5
#define SBR_GLOBAL_TONALITY_VALUES 2

#define MAX_NUM_CHANNELS 2

#define MAX_NOISE_ENVELOPES 2
#define MAX_NUM_NOISE_COEFFS 5
#define MAX_NUM_NOISE_VALUES (MAX_NUM_NOISE_COEFFS * MAX_NOISE_ENVELOPES)

#define MAX_NUM_ENVELOPE_VALUES (MAX_ENVELOPES * MAX_FREQ_COEFFS)
#define MAX_ENVELOPES 5
#define MAX_FREQ_COEFFS 48

#define MAX_FREQ_COEFFS_FS44100 35
#define MAX_FREQ_COEFFS_FS48000 32

#define NO_OF_ESTIMATES_LC 4
#define NO_OF_ESTIMATES_LD 3
#define MAX_NO_OF_ESTIMATES 4

#define NOISE_FLOOR_OFFSET 6
#define NOISE_FLOOR_OFFSET_64 (FL2FXCONST_DBL(0.09375f))

#define LOW_RES 0
#define HIGH_RES 1

#define LO 0
#define HI 1

#define LENGTH_SBR_FRAME_INFO 35 /* 19 */

#define SBR_NSFB_LOW_RES 9   /*  8 */
#define SBR_NSFB_HIGH_RES 18 /* 16 */

#define SBR_XPOS_CTRL_DEFAULT 2

#define SBR_FREQ_SCALE_DEFAULT 2
#define SBR_ALTER_SCALE_DEFAULT 1
#define SBR_NOISE_BANDS_DEFAULT 2

#define SBR_LIMITER_BANDS_DEFAULT 2
#define SBR_LIMITER_GAINS_DEFAULT 2
#define SBR_LIMITER_GAINS_INFINITE 3
#define SBR_INTERPOL_FREQ_DEFAULT 1
#define SBR_SMOOTHING_LENGTH_DEFAULT 0

/* sbr_header */
#define SI_SBR_AMP_RES_BITS 1
#define SI_SBR_COUPLING_BITS 1
#define SI_SBR_START_FREQ_BITS 4
#define SI_SBR_STOP_FREQ_BITS 4
#define SI_SBR_XOVER_BAND_BITS 3
#define SI_SBR_RESERVED_BITS 2
#define SI_SBR_DATA_EXTRA_BITS 1
#define SI_SBR_HEADER_EXTRA_1_BITS 1
#define SI_SBR_HEADER_EXTRA_2_BITS 1

/* sbr_header extra 1 */
#define SI_SBR_FREQ_SCALE_BITS 2
#define SI_SBR_ALTER_SCALE_BITS 1
#define SI_SBR_NOISE_BANDS_BITS 2

/* sbr_header extra 2 */
#define SI_SBR_LIMITER_BANDS_BITS 2
#define SI_SBR_LIMITER_GAINS_BITS 2
#define SI_SBR_INTERPOL_FREQ_BITS 1
#define SI_SBR_SMOOTHING_LENGTH_BITS 1

/* sbr_grid */
#define SBR_CLA_BITS 2    /*!< size of bs_frame_class */
#define SBR_CLA_BITS_LD 1 /*!< size of bs_frame_class */
#define SBR_ENV_BITS 2    /*!< size of bs_num_env_raw */
#define SBR_ABS_BITS 2    /*!< size of bs_abs_bord_raw for HE-AAC */
#define SBR_NUM_BITS 2    /*!< size of bs_num_rel */
#define SBR_REL_BITS 2    /*!< size of bs_rel_bord_raw */
#define SBR_RES_BITS 1    /*!< size of bs_freq_res_flag */
#define SBR_DIR_BITS 1    /*!< size of bs_df_flag */

/* sbr_data */
#define SI_SBR_INVF_MODE_BITS 2

#define SI_SBR_START_ENV_BITS_AMP_RES_3_0 6
#define SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0 5
#define SI_SBR_START_NOISE_BITS_AMP_RES_3_0 5
#define SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0 5

#define SI_SBR_START_ENV_BITS_AMP_RES_1_5 7
#define SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5 6

#define SI_SBR_EXTENDED_DATA_BITS 1
#define SI_SBR_EXTENSION_SIZE_BITS 4
#define SI_SBR_EXTENSION_ESC_COUNT_BITS 8
#define SI_SBR_EXTENSION_ID_BITS 2

#define SBR_EXTENDED_DATA_MAX_CNT (15 + 255)

#define EXTENSION_ID_PS_CODING 2

/* Envelope coding constants */
#define FREQ 0
#define TIME 1

/* qmf data scaling */
#define QMF_SCALE_OFFSET 7

/* huffman tables */
#define CODE_BOOK_SCF_LAV00 60
#define CODE_BOOK_SCF_LAV01 31
#define CODE_BOOK_SCF_LAV10 60
#define CODE_BOOK_SCF_LAV11 31
#define CODE_BOOK_SCF_LAV_BALANCE11 12
#define CODE_BOOK_SCF_LAV_BALANCE10 24

typedef enum { SBR_AMP_RES_1_5 = 0, SBR_AMP_RES_3_0 } AMP_RES;

typedef enum {
  XPOS_MDCT,
  XPOS_MDCT_CROSS,
  XPOS_LC,
  XPOS_RESERVED,
  XPOS_SWITCHED /* not a real choice but used here to control behaviour */
} XPOS_MODE;

typedef enum {
  INVF_OFF = 0,
  INVF_LOW_LEVEL,
  INVF_MID_LEVEL,
  INVF_HIGH_LEVEL,
  INVF_SWITCHED /* not a real choice but used here to control behaviour */
} INVF_MODE;

#endif
