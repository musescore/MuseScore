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

   Author(s):   Manuel Jander

   Description: USAC related constants

*******************************************************************************/

#ifndef USACDEC_CONST_H
#define USACDEC_CONST_H

/* scale factors */
#define SF_CODE 6       /* exponent of code[], fixed codebook vector */
#define SF_GAIN_C 16    /* exponent of gain code and smoothed gain code */
#define SF_EXC 16       /* exponent of exc[] and exc2[], excitation buffer */
#define SF_GAIN_P 1     /* exponent of gain_pit */
#define SF_PFAC 0       /* exponent of period/voicing factor */
#define SF_SYNTH SF_EXC /* exponent of synthesis buffer */
#define SF_A_COEFFS 3   /* exponent of LP domain synthesis filter coefficient */
#define SF_STAB 1       /* exponent of stability factor */

/* definitions which are independent of coreCoderFrameLength */
#define M_LP_FILTER_ORDER 16 /* LP filter order */
#define LP_FILTER_SCALE 4    /* LP filter scale */

#define PIT_MIN_12k8 34    /* Minimum pitch lag with resolution 1/4 */
#define PIT_MAX_12k8 231   /* Maximum pitch lag for fs=12.8kHz */
#define FSCALE_DENOM 12800 /* Frequency scale denominator */
#define FAC_FSCALE_MIN \
  6000 /* Minimum allowed frequency scale for acelp decoder */

#if !defined(LPD_MAX_CORE_SR)
#define LPD_MAX_CORE_SR 24000 /* Default value from ref soft */
#endif
#define FAC_FSCALE_MAX \
  LPD_MAX_CORE_SR /* Maximum allowed frequency scale for acelp decoder */

/* Maximum pitch lag (= 411 for fs_max = 24000) */
#define PIT_MAX_TMP                                                            \
  (PIT_MAX_12k8 +                                                              \
   (6 *                                                                        \
    ((((FAC_FSCALE_MAX * PIT_MIN_12k8) + (FSCALE_DENOM / 2)) / FSCALE_DENOM) - \
     PIT_MIN_12k8)))
#if (PIT_MAX_TMP < \
     256) /* cannot be smaller because of tcx time domain concealment */
#define PIT_MAX_MAX 256
#else
#define PIT_MAX_MAX PIT_MAX_TMP
#endif

#define NB_DIV 4   /* number of division (20ms) per 80ms frame */
#define L_SUBFR 64 /* subframe size (5ms) */
#define BPF_SFD 1  /* bass postfilter delay (subframe) */
#define BPF_DELAY (BPF_SFD * L_SUBFR) /* bass postfilter delay (samples) */

#define L_FILT 12  /* Delay of up-sampling filter (bass post-filter) */
#define L_EXTRA 96 /* for bass post-filter */
#define L_INTERPOL \
  (16 + 1) /* Length of filter for interpolation (acelp decoder) */

/* definitions for coreCoderFrameLength = 1024 */
#define L_FRAME_PLUS_1024 1024 /* length of one 80ms superframe */
#define L_DIV_1024 \
  (L_FRAME_PLUS_1024 / NB_DIV) /* length of one acelp or tcx20 frame */
#define NB_SUBFR_1024 \
  (L_DIV_1024 / L_SUBFR) /* number of 5ms subframe per division */
#define NB_SUBFR_SUPERFR_1024 \
  (L_FRAME_PLUS_1024 / L_SUBFR) /* number of 5ms subframe per 80ms frame */
#define AAC_SFD_1024 (NB_SUBFR_SUPERFR_1024 / 2) /* AAC delay (subframe) */
#define AAC_DELAY_1024 (AAC_SFD_1024 * L_SUBFR)  /* AAC delay (samples) */
#define SYN_SFD_1024 (AAC_SFD_1024 - BPF_SFD) /* synthesis delay (subframe) */
#define SYN_DELAY_1024                                          \
  (SYN_SFD_1024 * L_SUBFR)         /* synthesis delay (samples) \
                                    */
#define LFAC_1024 (L_DIV_1024 / 2) /* FAC frame length */
#define LFAC_SHORT_1024 \
  (L_DIV_1024 / 4)        /* for transitions EIGHT_SHORT FD->LPD and vv. */
#define FDNS_NPTS_1024 64 /* FD noise shaping resolution (64=100Hz/point) */

/* definitions for coreCoderFrameLength = 768 */
#define L_FRAME_PLUS_768 768
#define L_DIV_768 \
  (L_FRAME_PLUS_768 / NB_DIV) /* length of one acelp or tcx20 frame */
#define NB_SUBFR_768 \
  (L_DIV_768 / L_SUBFR) /* number of 5ms subframe per division */
#define NB_SUBFR_SUPERFR_768 \
  (L_FRAME_PLUS_768 / L_SUBFR) /* number of 5ms subframe per 80ms frame */
#define AAC_SFD_768 (NB_SUBFR_SUPERFR_768 / 2) /* AAC delay (subframe) */
#define AAC_DELAY_768 (AAC_SFD_768 * L_SUBFR)  /* AAC delay (samples) */
#define SYN_SFD_768 (AAC_SFD_768 - BPF_SFD)    /* synthesis delay (subframe) */
#define SYN_DELAY_768 (SYN_SFD_768 * L_SUBFR)  /* synthesis delay (samples) */
#define LFAC_768 (L_DIV_768 / 2)               /* FAC frame length */
#define LFAC_SHORT_768 \
  (L_DIV_768 / 4) /* for transitions EIGHT_SHORT FD->LPD and vv. */

/* maximum (used for memory allocation) */
#define L_FRAME_PLUS L_FRAME_PLUS_1024
#define L_DIV L_DIV_1024
#define NB_SUBFR NB_SUBFR_1024
#define NB_SUBFR_SUPERFR NB_SUBFR_SUPERFR_1024
#define AAC_SFD AAC_SFD_1024
#define AAC_DELAY AAC_DELAY_1024
#define SYN_SFD SYN_SFD_1024
#define SYN_DELAY SYN_DELAY_1024
#define LFAC LFAC_1024
#define LFAC_SHORT LFAC_SHORT_1024
#define FDNS_NPTS FDNS_NPTS_1024

#endif /* USACDEC_CONST_H */
