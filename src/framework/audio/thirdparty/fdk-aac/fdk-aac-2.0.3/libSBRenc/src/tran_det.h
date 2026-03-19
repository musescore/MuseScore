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
  \brief  Transient detector prototypes $Revision: 95111 $
*/
#ifndef TRAN_DET_H
#define TRAN_DET_H

#include "sbr_encoder.h"
#include "sbr_def.h"

typedef struct {
  FIXP_DBL transients[32 + (32 / 2)];
  FIXP_DBL thresholds[64];
  FIXP_DBL tran_thr;    /* Master threshold for transient signals */
  FIXP_DBL split_thr_m; /* Threshold for splitting FIXFIX-frames into 2 env */
  INT split_thr_e;      /* Scale for splitting threshold */
  FIXP_DBL prevLowBandEnergy;  /* Energy of low band */
  FIXP_DBL prevHighBandEnergy; /* Energy of high band */
  INT tran_fc;                 /* Number of lowband subbands to discard  */
  INT no_cols;
  INT no_rows;
  INT mode;

  int frameShift;
  int tran_off; /* Offset for reading energy values. */
} SBR_TRANSIENT_DETECTOR;

typedef SBR_TRANSIENT_DETECTOR *HANDLE_SBR_TRANSIENT_DETECTOR;

#define TRAN_DET_LOOKAHEAD 2
#define TRAN_DET_START_FREQ 4500 /*start frequency for transient detection*/
#define TRAN_DET_STOP_FREQ 13500 /*stop frequency for transient detection*/
#define TRAN_DET_MIN_QMFBANDS                    \
  4 /* minimum qmf bands for transient detection \
     */
#define QMF_HP_dBd_SLOPE_FIX \
  FL2FXCONST_DBL(0.00075275f) /* 0.002266f/10 * log2(10) */
#define TRAN_DET_THRSHLD FL2FXCONST_DBL(5.0f / 8.0f)
#define TRAN_DET_THRSHLD_SCALE (3)

typedef struct {
  INT transientCandidates[32 + TRAN_DET_LOOKAHEAD];
  INT nTimeSlots;
  INT lookahead;
  INT startBand;
  INT stopBand;

  FIXP_DBL dBf_m[64];
  INT dBf_e[64];

  FIXP_DBL energy_timeSlots[32 + TRAN_DET_LOOKAHEAD];
  INT energy_timeSlots_scale[32 + TRAN_DET_LOOKAHEAD];

  FIXP_DBL delta_energy[32 + TRAN_DET_LOOKAHEAD];
  INT delta_energy_scale[32 + TRAN_DET_LOOKAHEAD];

  FIXP_DBL lowpass_energy[32 + TRAN_DET_LOOKAHEAD];
  INT lowpass_energy_scale[32 + TRAN_DET_LOOKAHEAD];
} FAST_TRAN_DETECTOR;
typedef FAST_TRAN_DETECTOR *HANDLE_FAST_TRAN_DET;

INT FDKsbrEnc_InitSbrFastTransientDetector(
    HANDLE_FAST_TRAN_DET h_sbrFastTransientDetector,
    const INT time_slots_per_frame, const INT bandwidth_qmf_slot,
    const INT no_qmf_channels, const INT sbr_qmf_1st_band);

void FDKsbrEnc_fastTransientDetect(
    const HANDLE_FAST_TRAN_DET h_sbrFastTransientDetector,
    const FIXP_DBL *const *Energies, const int *const scaleEnergies,
    const INT YBufferWriteOffset, UCHAR *const tran_vector);

void FDKsbrEnc_transientDetect(
    HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector, FIXP_DBL **Energies,
    INT *scaleEnergies, UCHAR *tran_vector, int YBufferWriteOffset,
    int YBufferSzShift, int timeStep, int frameMiddleBorder);

int FDKsbrEnc_InitSbrTransientDetector(
    HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector,
    UINT sbrSyntaxFlags, /* SBR syntax flags derived from AOT. */
    INT frameSize, INT sampleFreq, sbrConfigurationPtr params, int tran_fc,
    int no_cols, int no_rows, int YBufferWriteOffset, int YBufferSzShift,
    int frameShift, int tran_off);

void FDKsbrEnc_frameSplitter(
    FIXP_DBL **Energies, INT *scaleEnergies,
    HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector, UCHAR *freqBandTable,
    UCHAR *tran_vector, int YBufferWriteOffset, int YBufferSzShift, int nSfb,
    int timeStep, int no_cols, FIXP_DBL *tonality);
#endif
