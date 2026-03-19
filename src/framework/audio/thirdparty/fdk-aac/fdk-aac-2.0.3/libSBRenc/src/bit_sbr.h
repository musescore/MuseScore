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
  \brief  SBR bit writing $Revision: 92790 $
*/
#ifndef BIT_SBR_H
#define BIT_SBR_H

#include "sbr_def.h"
#include "cmondata.h"
#include "fram_gen.h"

struct SBR_ENV_DATA;

struct SBR_BITSTREAM_DATA {
  INT TotalBits;
  INT PayloadBits;
  INT FillBits;
  INT HeaderActive;
  INT HeaderActiveDelay; /**< sbr payload and its header is delayed depending on
                            encoder configuration*/
  INT NrSendHeaderData;  /**< input from commandline */
  INT CountSendHeaderData; /**< modulo count. If < 0 then no counting is done
                              (no SBR headers) */
  INT rightBorderFIX;      /**< force VARFIX or FIXFIX frames */
};

typedef struct SBR_BITSTREAM_DATA *HANDLE_SBR_BITSTREAM_DATA;

struct SBR_HEADER_DATA {
  AMP_RES sbr_amp_res;
  INT sbr_start_frequency;
  INT sbr_stop_frequency;
  INT sbr_xover_band;
  INT sbr_noise_bands;
  INT sbr_data_extra;
  INT header_extra_1;
  INT header_extra_2;
  INT sbr_lc_stereo_mode;
  INT sbr_limiter_bands;
  INT sbr_limiter_gains;
  INT sbr_interpol_freq;
  INT sbr_smoothing_length;
  INT alterScale;
  INT freqScale;

  /*
    element of channelpairelement
  */
  INT coupling;
  INT prev_coupling;

  /*
    element of singlechannelelement
  */
};
typedef struct SBR_HEADER_DATA *HANDLE_SBR_HEADER_DATA;

struct SBR_ENV_DATA {
  INT sbr_xpos_ctrl;
  FREQ_RES freq_res_fixfix[2];
  UCHAR fResTransIsLow;

  INVF_MODE sbr_invf_mode;
  INVF_MODE sbr_invf_mode_vec[MAX_NUM_NOISE_VALUES];

  XPOS_MODE sbr_xpos_mode;

  INT ienvelope[MAX_ENVELOPES][MAX_FREQ_COEFFS];

  INT codeBookScfLavBalance;
  INT codeBookScfLav;
  const INT *hufftableTimeC;
  const INT *hufftableFreqC;
  const UCHAR *hufftableTimeL;
  const UCHAR *hufftableFreqL;

  const INT *hufftableLevelTimeC;
  const INT *hufftableBalanceTimeC;
  const INT *hufftableLevelFreqC;
  const INT *hufftableBalanceFreqC;
  const UCHAR *hufftableLevelTimeL;
  const UCHAR *hufftableBalanceTimeL;
  const UCHAR *hufftableLevelFreqL;
  const UCHAR *hufftableBalanceFreqL;

  const UCHAR *hufftableNoiseTimeL;
  const INT *hufftableNoiseTimeC;
  const UCHAR *hufftableNoiseFreqL;
  const INT *hufftableNoiseFreqC;

  const UCHAR *hufftableNoiseLevelTimeL;
  const INT *hufftableNoiseLevelTimeC;
  const UCHAR *hufftableNoiseBalanceTimeL;
  const INT *hufftableNoiseBalanceTimeC;
  const UCHAR *hufftableNoiseLevelFreqL;
  const INT *hufftableNoiseLevelFreqC;
  const UCHAR *hufftableNoiseBalanceFreqL;
  const INT *hufftableNoiseBalanceFreqC;

  HANDLE_SBR_GRID hSbrBSGrid;

  INT noHarmonics;
  INT addHarmonicFlag;
  UCHAR addHarmonic[MAX_FREQ_COEFFS];

  /* calculated helper vars */
  INT si_sbr_start_env_bits_balance;
  INT si_sbr_start_env_bits;
  INT si_sbr_start_noise_bits_balance;
  INT si_sbr_start_noise_bits;

  INT noOfEnvelopes;
  INT noScfBands[MAX_ENVELOPES];
  INT domain_vec[MAX_ENVELOPES];
  INT domain_vec_noise[MAX_ENVELOPES];
  SCHAR sbr_noise_levels[MAX_FREQ_COEFFS];
  INT noOfnoisebands;

  INT balance;
  AMP_RES init_sbr_amp_res;
  AMP_RES currentAmpResFF;
  FIXP_DBL
  ton_HF[SBR_GLOBAL_TONALITY_VALUES]; /* tonality is scaled by
                                         2^19/0.524288f (fract part of
                                         RELAXATION) */
  FIXP_DBL global_tonality;

  /* extended data */
  INT extended_data;
  INT extension_size;
  INT extension_id;
  UCHAR extended_data_buffer[SBR_EXTENDED_DATA_MAX_CNT];

  UCHAR ldGrid;
};
typedef struct SBR_ENV_DATA *HANDLE_SBR_ENV_DATA;

INT FDKsbrEnc_WriteEnvSingleChannelElement(
    struct SBR_HEADER_DATA *sbrHeaderData,
    struct T_PARAMETRIC_STEREO *hParametricStereo,
    struct SBR_BITSTREAM_DATA *sbrBitstreamData,
    struct SBR_ENV_DATA *sbrEnvData, struct COMMON_DATA *cmonData,
    UINT sbrSyntaxFlags);

INT FDKsbrEnc_WriteEnvChannelPairElement(
    struct SBR_HEADER_DATA *sbrHeaderData,
    struct T_PARAMETRIC_STEREO *hParametricStereo,
    struct SBR_BITSTREAM_DATA *sbrBitstreamData,
    struct SBR_ENV_DATA *sbrEnvDataLeft, struct SBR_ENV_DATA *sbrEnvDataRight,
    struct COMMON_DATA *cmonData, UINT sbrSyntaxFlags);

INT FDKsbrEnc_CountSbrChannelPairElement(
    struct SBR_HEADER_DATA *sbrHeaderData,
    struct T_PARAMETRIC_STEREO *hParametricStereo,
    struct SBR_BITSTREAM_DATA *sbrBitstreamData,
    struct SBR_ENV_DATA *sbrEnvDataLeft, struct SBR_ENV_DATA *sbrEnvDataRight,
    struct COMMON_DATA *cmonData, UINT sbrSyntaxFlags);

/* debugging and tuning functions */

/*#define SBR_ENV_STATISTICS */

/*#define SBR_PAYLOAD_MONITOR*/

#endif
