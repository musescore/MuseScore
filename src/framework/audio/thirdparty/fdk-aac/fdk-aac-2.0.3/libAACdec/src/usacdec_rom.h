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

   Author(s):   M. Jander

   Description: re8.h

*******************************************************************************/

#ifndef USACDEC_ROM_H
#define USACDEC_ROM_H

#include "common_fix.h"
#include "FDK_lpc.h"

#include "usacdec_const.h"

/* RE8 lattice quantiser constants */
#define NB_SPHERE 32
#define NB_LEADER 37
#define NB_LDSIGN 226
#define NB_LDQ3 9
#define NB_LDQ4 28

#define LSF_SCALE 13

/* RE8 lattice quantiser tables */
extern const UINT fdk_dec_tab_factorial[8];
extern const UCHAR fdk_dec_Ia[NB_LEADER];
extern const UCHAR fdk_dec_Ds[NB_LDSIGN];
extern const USHORT fdk_dec_Is[NB_LDSIGN];
extern const UCHAR fdk_dec_Ns[], fdk_dec_A3[], fdk_dec_A4[];
extern const UCHAR fdk_dec_Da[][8];
extern const USHORT fdk_dec_I3[], fdk_dec_I4[];

/* temp float tables for LPC decoding */
extern const FIXP_LPC fdk_dec_lsf_init[16];
extern const FIXP_LPC fdk_dec_dico_lsf_abs_8b[16 * 256];

/* ACELP tables */
#define SF_QUA_GAIN7B 4
extern const FIXP_SGL fdk_t_qua_gain7b[128 * 2];
extern const FIXP_SGL lsp_interpol_factor[2][NB_SUBFR];

/* For bass post filter */
#define L_FILT 12 /* Delay of up-sampling filter                */

extern const FIXP_SGL fdk_dec_filt_lp[1 + L_FILT];

extern const FIXP_WTB FacWindowSynth128[128];
extern const FIXP_WTB FacWindowZir128[128];
extern const FIXP_WTB FacWindowSynth64[64];
extern const FIXP_WTB FacWindowZir64[64];
extern const FIXP_WTB FacWindowSynth32[32];
extern const FIXP_WTB FacWindowZir32[32];
extern const FIXP_WTB FacWindowSynth96[96];
extern const FIXP_WTB FacWindowZir96[96];
extern const FIXP_WTB FacWindowSynth48[48];
extern const FIXP_WTB FacWindowZir48[48];

#endif /* USACDEC_ROM_H */
