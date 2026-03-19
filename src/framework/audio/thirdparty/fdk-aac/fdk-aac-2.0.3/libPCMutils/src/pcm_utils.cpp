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

/**************************** PCM utility library ******************************

   Author(s):   Arthur Tritthart, Alfonso Pino Garcia

   Description: Functions that perform (de)interleaving combined with format
change

*******************************************************************************/

#include "pcm_utils.h"

/* library version */
#include "version.h"

void FDK_interleave(const FIXP_DBL *RESTRICT pIn, LONG *RESTRICT pOut,
                    const UINT channels, const UINT frameSize,
                    const UINT length) {
  for (UINT sample = 0; sample < length; sample++) {
    const FIXP_DBL *In = &pIn[sample];
    for (UINT ch = 0; ch < channels; ch++) {
      *pOut++ = (LONG)In[0];
      In += frameSize;
    }
  }
}

void FDK_interleave(const FIXP_DBL *RESTRICT pIn, SHORT *RESTRICT pOut,
                    const UINT channels, const UINT frameSize,
                    const UINT length) {
  for (UINT sample = 0; sample < length; sample++) {
    const FIXP_DBL *In = &pIn[sample];
    for (UINT ch = 0; ch < channels; ch++) {
      *pOut++ = (SHORT)FX_DBL2FX_SGL(In[0]);
      In += frameSize;
    }
  }
}

void FDK_interleave(const FIXP_SGL *RESTRICT pIn, SHORT *RESTRICT pOut,
                    const UINT channels, const UINT frameSize,
                    const UINT length) {
  for (UINT sample = 0; sample < length; sample++) {
    const FIXP_SGL *In = &pIn[sample];
    for (UINT ch = 0; ch < channels; ch++) {
      *pOut++ = (SHORT)In[0];
      In += frameSize;
    }
  }
}

void FDK_deinterleave(const LONG *RESTRICT pIn, SHORT *RESTRICT _pOut,
                      const UINT channels, const UINT frameSize,
                      const UINT length) {
  for (UINT ch = 0; ch < channels; ch++) {
    SHORT *pOut = _pOut + length * ch;
    const LONG *In = &pIn[ch];
    for (UINT sample = 0; sample < frameSize; sample++) {
      *pOut++ = (SHORT)(In[0] >> 16);
      In += channels;
    }
  }
}

void FDK_deinterleave(const LONG *RESTRICT pIn, LONG *RESTRICT _pOut,
                      const UINT channels, const UINT frameSize,
                      const UINT length) {
  for (UINT ch = 0; ch < channels; ch++) {
    LONG *pOut = _pOut + length * ch;
    const LONG *In = &pIn[ch];
    for (UINT sample = 0; sample < frameSize; sample++) {
      *pOut++ = In[0];
      In += channels;
    }
  }
}

void FDK_deinterleave(const SHORT *RESTRICT pIn, SHORT *RESTRICT _pOut,
                      const UINT channels, const UINT frameSize,
                      const UINT length) {
  for (UINT ch = 0; ch < channels; ch++) {
    SHORT *pOut = _pOut + length * ch;
    const SHORT *In = &pIn[ch];
    for (UINT sample = 0; sample < frameSize; sample++) {
      *pOut++ = In[0];
      In += channels;
    }
  }
}

void FDK_deinterleave(const SHORT *RESTRICT pIn, LONG *RESTRICT _pOut,
                      const UINT channels, const UINT frameSize,
                      const UINT length) {
  for (UINT ch = 0; ch < channels; ch++) {
    LONG *pOut = _pOut + length * ch;
    const SHORT *In = &pIn[ch];
    for (UINT sample = 0; sample < frameSize; sample++) {
      *pOut++ = (LONG)In[0] << 16;
      In += channels;
    }
  }
}
