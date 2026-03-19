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

/**************************** AAC encoder library ******************************

   Author(s):   M. Neusinger

   Description: Compressor for AAC Metadata Generator

*******************************************************************************/

#ifndef METADATA_COMPRESSOR_H
#define METADATA_COMPRESSOR_H

#include "FDK_audio.h"
#include "common_fix.h"

#include "aacenc.h"

#define LFE_LEV_SCALE 2

/**
 *  DRC compression profiles.
 */
typedef enum DRC_PROFILE {
  DRC_NONE = 0,
  DRC_FILMSTANDARD = 1,
  DRC_FILMLIGHT = 2,
  DRC_MUSICSTANDARD = 3,
  DRC_MUSICLIGHT = 4,
  DRC_SPEECH = 5,
  DRC_DELAY_TEST = 6,
  DRC_NOT_PRESENT = -2

} DRC_PROFILE;

/**
 *  DRC Compressor handle.
 */
typedef struct DRC_COMP DRC_COMP, *HDRC_COMP;

/**
 * \brief  Open a DRC Compressor instance.
 *
 * Allocate memory for a compressor instance.
 *
 * \param phDrcComp             A pointer to a compressor handle. Initialized on
 * return.
 *
 * \return
 *          - 0, on succes.
 *          - unequal 0, on failure.
 */
INT FDK_DRC_Generator_Open(HDRC_COMP *phDrcComp);

/**
 * \brief  Close the DRC Compressor instance.
 *
 * Deallocate instance and free whole memory.
 *
 * \param phDrcComp             Pointer to the compressor handle to be
 * deallocated.
 *
 * \return
 *          - 0, on succes.
 *          - unequal 0, on failure.
 */
INT FDK_DRC_Generator_Close(HDRC_COMP *phDrcComp);

/**
 * \brief  Configure DRC Compressor.
 *
 * \param drcComp               Compressor handle.
 * \param profileLine           DRC profile for line mode.
 * \param profileRF             DRC profile for RF mode.
 * \param blockLength           Length of processing block in samples per
 * channel.
 * \param sampleRate            Sampling rate in Hz.
 * \param channelMode           Channel configuration.
 * \param channelOrder          Channel order, MPEG or WAV.
 * \param useWeighting          Use weighting filter for loudness calculation
 *
 * \return
 *          - 0, on success,
 *          - unequal 0, on failure
 */
INT FDK_DRC_Generator_Initialize(HDRC_COMP drcComp,
                                 const DRC_PROFILE profileLine,
                                 const DRC_PROFILE profileRF,
                                 const INT blockLength, const UINT sampleRate,
                                 const CHANNEL_MODE channelMode,
                                 const CHANNEL_ORDER channelOrder,
                                 const UCHAR useWeighting);

/**
 * \brief  Calculate DRC Compressor Gain.
 *
 * \param drcComp               Compressor handle.
 * \param inSamples             Pointer to interleaved input audio samples.
 * \param inSamplesBufSize      Size of inSamples for one channel.
 * \param dialnorm              Dialog Level in dB (typically -31...-1).
 * \param drc_TargetRefLevel
 * \param comp_TargetRefLevel
 * \param clev                  Downmix center mix factor (typically 0.707,
 * 0.595 or 0.5)
 * \param slev                  Downmix surround mix factor (typically 0.707,
 * 0.5, or 0)
 * \param ext_leva              Downmix gain factor A
 * \param ext_levb              Downmix gain factor B
 * \param lfe_lev               LFE gain factor
 * \param dmxGain5              Gain factor for downmix to 5 channels
 * \param dmxGain2              Gain factor for downmix to 2 channels
 * \param dynrng                Pointer to variable receiving line mode DRC gain
 * in dB
 * \param compr                 Pointer to variable receiving RF mode DRC gain
 * in dB
 *
 * \return
 *          - 0, on success,
 *          - unequal 0, on failure
 */
INT FDK_DRC_Generator_Calc(HDRC_COMP drcComp, const INT_PCM *const inSamples,
                           const UINT inSamplesBufSize, const INT dialnorm,
                           const INT drc_TargetRefLevel,
                           const INT comp_TargetRefLevel, const FIXP_DBL clev,
                           const FIXP_DBL slev, const FIXP_DBL ext_leva,
                           const FIXP_DBL ext_levb, const FIXP_DBL lfe_lev,
                           const INT dmxGain5, const INT dmxGain2,
                           INT *const dynrng, INT *const compr);

/**
 * \brief  Configure DRC Compressor Profile.
 *
 * \param drcComp               Compressor handle.
 * \param profileLine           DRC profile for line mode.
 * \param profileRF             DRC profile for RF mode.
 *
 * \return
 *          - 0, on success,
 *          - unequal 0, on failure
 */
INT FDK_DRC_Generator_setDrcProfile(HDRC_COMP drcComp,
                                    const DRC_PROFILE profileLine,
                                    const DRC_PROFILE profileRF);

/**
 * \brief  Get DRC profile for line mode.
 *
 * \param drcComp               Compressor handle.
 *
 * \return  Current Profile.
 */
DRC_PROFILE FDK_DRC_Generator_getDrcProfile(const HDRC_COMP drcComp);

/**
 * \brief  Get DRC profile for RF mode.
 *
 * \param drcComp               Compressor handle.
 *
 * \return  Current Profile.
 */
DRC_PROFILE FDK_DRC_Generator_getCompProfile(const HDRC_COMP drcComp);

#endif /* METADATA_COMPRESSOR_H */
