/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):   Matthias Neusinger

   Description: Hard limiter for clipping prevention

*******************************************************************************/

#ifndef LIMITER_H
#define LIMITER_H

#include "common_fix.h"
#include "FDK_audio.h"

#define TDL_ATTACK_DEFAULT_MS (15)  /* default attack  time in ms */
#define TDL_RELEASE_DEFAULT_MS (50) /* default release time in ms */

#ifdef __cplusplus
extern "C" {
#endif

struct TDLimiter {
  unsigned int attack;
  FIXP_DBL attackConst, releaseConst;
  unsigned int attackMs, releaseMs, maxAttackMs;
  FIXP_DBL threshold;
  unsigned int channels, maxChannels;
  UINT sampleRate, maxSampleRate;
  FIXP_DBL cor, max;
  FIXP_DBL* maxBuf;
  FIXP_DBL* delayBuf;
  unsigned int maxBufIdx, delayBufIdx;
  FIXP_DBL smoothState0;
  FIXP_DBL minGain;
  INT scaling;
};

typedef enum {
  TDLIMIT_OK = 0,
  TDLIMIT_UNKNOWN = -1,

  __error_codes_start = -100,

  TDLIMIT_INVALID_HANDLE,
  TDLIMIT_INVALID_PARAMETER,

  __error_codes_end
} TDLIMITER_ERROR;

struct TDLimiter;
typedef struct TDLimiter* TDLimiterPtr;

#define PCM_LIM LONG
#define FIXP_DBL2PCM_LIM(x) (x)
#define PCM_LIM2FIXP_DBL(x) (x)
#define PCM_LIM_BITS 32
#define FIXP_PCM_LIM FIXP_DBL

#define SAMPLE_BITS_LIM DFRACT_BITS

/******************************************************************************
 * pcmLimiter_Reset                                                            *
 * limiter: limiter handle                                                     *
 * returns: error code                                                         *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Reset(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_Destroy                                                          *
 * limiter: limiter handle                                                     *
 * returns: error code                                                         *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Destroy(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_GetDelay                                                         *
 * limiter: limiter handle                                                     *
 * returns: exact delay caused by the limiter in samples per channel           *
 ******************************************************************************/
unsigned int pcmLimiter_GetDelay(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_GetMaxGainReduction                                              *
 * limiter: limiter handle                                                     *
 * returns: maximum gain reduction in last processed block in dB               *
 ******************************************************************************/
INT pcmLimiter_GetMaxGainReduction(TDLimiterPtr limiter);

/******************************************************************************
 * pcmLimiter_SetNChannels                                                     *
 * limiter:   limiter handle                                                   *
 * nChannels: number of channels ( <= maxChannels specified on create)         *
 * returns:   error code                                                       *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetNChannels(TDLimiterPtr limiter,
                                        unsigned int nChannels);

/******************************************************************************
 * pcmLimiter_SetSampleRate                                                    *
 * limiter:    limiter handle                                                  *
 * sampleRate: sampling rate in Hz ( <= maxSampleRate specified on create)     *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetSampleRate(TDLimiterPtr limiter, UINT sampleRate);

/******************************************************************************
 * pcmLimiter_SetAttack                                                        *
 * limiter:    limiter handle                                                  *
 * attackMs:   attack time in ms ( <= maxAttackMs specified on create)         *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetAttack(TDLimiterPtr limiter,
                                     unsigned int attackMs);

/******************************************************************************
 * pcmLimiter_SetRelease                                                       *
 * limiter:    limiter handle                                                  *
 * releaseMs:  release time in ms                                              *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetRelease(TDLimiterPtr limiter,
                                      unsigned int releaseMs);

/******************************************************************************
 * pcmLimiter_GetLibInfo                                                       *
 * info:       pointer to an allocated and initialized LIB_INFO structure      *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_GetLibInfo(LIB_INFO* info);

#ifdef __cplusplus
}
#endif

/******************************************************************************
 * pcmLimiter_Create                                                           *
 * maxAttackMs:   maximum and initial attack/lookahead time in milliseconds    *
 * releaseMs:     release time in milliseconds (90% time constant)             *
 * threshold:     limiting threshold                                           *
 * maxChannels:   maximum and initial number of channels                       *
 * maxSampleRate: maximum and initial sampling rate in Hz                      *
 * returns:       limiter handle                                               *
 ******************************************************************************/
TDLimiterPtr pcmLimiter_Create(unsigned int maxAttackMs, unsigned int releaseMs,
                               FIXP_DBL threshold, unsigned int maxChannels,
                               UINT maxSampleRate);

/******************************************************************************
 * pcmLimiter_SetThreshold                                                     *
 * limiter:    limiter handle                                                  *
 * threshold:  limiter threshold                                               *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_SetThreshold(TDLimiterPtr limiter,
                                        FIXP_DBL threshold);

/******************************************************************************
 * pcmLimiter_Apply                                                            *
 * limiter:        limiter handle                                              *
 * samplesIn:      pointer to input buffer containing interleaved samples      *
 * samplesOut:     pointer to output buffer containing interleaved samples     *
 * pGainPerSample: pointer to gains for each sample                            *
 * scaling:        scaling of output samples                                   *
 * nSamples:       number of samples per channel                               *
 * returns:    error code                                                      *
 ******************************************************************************/
TDLIMITER_ERROR pcmLimiter_Apply(TDLimiterPtr limiter, PCM_LIM* samplesIn,
                                 INT_PCM* samplesOut, FIXP_DBL* pGainPerSample,
                                 const INT scaling, const UINT nSamples);

#endif /* #ifndef LIMITER_H */
