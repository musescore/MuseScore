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

   Author(s):   Josef Hoepfl

   Description: joint stereo processing

*******************************************************************************/

#ifndef STEREO_H
#define STEREO_H

#include "machine_type.h"
#include "FDK_bitstream.h"
#include "common_fix.h"

#define SFB_PER_PRED_BAND 2

#define SR_FNA_OUT                                                           \
  0 /* Additional scaling of the CJointStereo_filterAndAdd()-output to avoid \
       overflows.    */
    /* The scaling factor can be set to 0 if the coefficients are prescaled
     * appropriately. */
/* Prescaling via factor SF_FNA_COEFFS is done at compile-time but should only
 * be      */
/* utilized if the coefficients are stored as FIXP_DBL. (cp. aac_rom.cpp/.h) */

/* The NO_CPLX_PRED_BUGFIX-switch was introduced to enable decoding of
   conformance-streams in way that they are comparable to buggy
   reference-streams. This is established by storing the prediction direction
   for computation of the "downmix MDCT of previous frame". This is not standard
   compliant. Once correct reference-streams for complex-stereo-prediction are
   available this switch becomes obsolete.
*/
/*#define NO_CPLX_PRED_BUGFIX*/

enum { JointStereoMaximumGroups = 8, JointStereoMaximumBands = 64 };

typedef struct {
  UCHAR pred_dir;  // 0 = prediction from mid to side channel, 1 = vice versa
  UCHAR
  igf_pred_dir;  // 0 = prediction from mid to side channel, 1 = vice versa
  UCHAR complex_coef;    // 0 = alpha_q_im[x] is 0 for all prediction bands, 1 =
                         // alpha_q_im[x] is transmitted via bitstream
  UCHAR use_prev_frame;  // 0 = use current frame for MDST estimation, 1 = use
                         // current and previous frame

  SHORT alpha_q_re[JointStereoMaximumGroups][JointStereoMaximumBands];
  SHORT alpha_q_im[JointStereoMaximumGroups][JointStereoMaximumBands];
} CCplxPredictionData;

/* joint stereo scratch memory (valid for this frame) */
typedef struct {
  UCHAR MsMaskPresent;
  UCHAR MsUsed[JointStereoMaximumBands]; /*!< every arry element contains flags
                                            for up to 8 groups. this array is
                                            also utilized for complex stereo
                                            prediction. */
  UCHAR IGF_MsMaskPresent;

  UCHAR cplx_pred_flag; /* stereo complex prediction was signalled for this
                           frame */
  UCHAR igf_cplx_pred_flag;

  /* The following array and variable are needed for the case  when INF is
   * active */
  FIXP_DBL store_dmx_re_prev[1024];
  SHORT store_dmx_re_prev_e;

} CJointStereoData;

/* joint stereo persistent memory */
typedef struct {
  UCHAR clearSpectralCoeffs; /* indicates that the spectral coeffs must be
                                cleared because the transform splitting active
                                flag of the left and right channel was different
                              */

  FIXP_DBL *scratchBuffer; /* pointer to scratch buffer */

  FIXP_DBL
  *spectralCoeffs[2]; /* spectral coefficients of this channel utilized by
                         complex stereo prediction */
  SHORT *specScale[2];

  SHORT alpha_q_re_prev[JointStereoMaximumGroups][JointStereoMaximumBands];
  SHORT alpha_q_im_prev[JointStereoMaximumGroups][JointStereoMaximumBands];

  UCHAR winSeqPrev;
  UCHAR winShapePrev;
  UCHAR winGroupsPrev;

} CJointStereoPersistentData;

/*!
  \brief Read joint stereo data from bitstream

  The function reads joint stereo data from bitstream.

  \param bs bit stream handle data source.
  \param pJointStereoData pointer to stereo data structure to receive decoded
  data. \param windowGroups number of window groups. \param
  scaleFactorBandsTransmitted number of transmitted scalefactor bands. \param
  flags decoder flags

  \return  0 on success, -1 on error.
*/
int CJointStereo_Read(HANDLE_FDK_BITSTREAM bs,
                      CJointStereoData *pJointStereoData,
                      const int windowGroups,
                      const int scaleFactorBandsTransmitted,
                      const int max_sfb_ste_clear,
                      CJointStereoPersistentData *pJointStereoPersistentData,
                      CCplxPredictionData *cplxPredictionData,
                      int cplxPredictionActiv, int scaleFactorBandsTotal,
                      int windowSequence, const UINT flags);

#endif /* #ifndef STEREO_H */
