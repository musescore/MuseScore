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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#ifndef DRCDECODER_H
#define DRCDECODER_H

/* drcDecoder.h: definitions used in all submodules */

#define MAX_ACTIVE_DRCS 3

typedef enum { DM_REGULAR_DELAY = 0, DM_LOW_DELAY = 1 } DELAY_MODE;

typedef enum {
  DE_OK = 0,
  DE_NOT_OK = -100,
  DE_PARAM_OUT_OF_RANGE,
  DE_PARAM_INVALID,
  DE_MEMORY_ERROR
} DRC_ERROR;

typedef enum { SDM_OFF, SDM_QMF64, SDM_QMF71, SDM_STFT256 } SUBBAND_DOMAIN_MODE;

#define DOWNMIX_ID_BASE_LAYOUT 0x0
#define DOWNMIX_ID_ANY_DOWNMIX 0x7F
#define DRC_SET_ID_NO_DRC 0x0
#define DRC_SET_ID_ANY_DRC 0x3F

#define LOCATION_MP4_INSTREAM_UNIDRC 0x1
#define LOCATION_MP4_DYN_RANGE_INFO 0x2
#define LOCATION_MP4_COMPRESSION_VALUE 0x3
#define LOCATION_SELECTED \
  LOCATION_MP4_INSTREAM_UNIDRC /* set to location selected by system */

#define MAX_REQUESTS_DOWNMIX_ID 15
#define MAX_REQUESTS_DRC_FEATURE 7
#define MAX_REQUESTS_DRC_EFFECT_TYPE 15

#define DEFAULT_LOUDNESS_DEVIATION_MAX 63

#define DRC_INPUT_LOUDNESS_TARGET FL2FXCONST_DBL(-31.0f / (float)(1 << 7))
#define DRC_INPUT_LOUDNESS_TARGET_SGL FL2FXCONST_SGL(-31.0f / (float)(1 << 7))

#endif
