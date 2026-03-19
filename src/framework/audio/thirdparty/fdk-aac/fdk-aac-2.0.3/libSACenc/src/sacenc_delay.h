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

/*********************** MPEG surround encoder library *************************

   Author(s):   Christian Goettlinger

   Description: Encoder Library Interface
                delay management of the encoder

*******************************************************************************/

/**************************************************************************/ /**
   \file
 ******************************************************************************/
#ifndef SACENC_DELAY_H
#define SACENC_DELAY_H

/* Includes ******************************************************************/
#include "sacenc_lib.h"
#include "machine_type.h"
#include "FDK_matrixCalloc.h"

/* Defines *******************************************************************/
#define MAX_DELAY_INPUT 1024
#define MAX_DELAY_OUTPUT 4096
/* bumped from 0 to 5. this should be equal or larger to the dualrate sbr
 * resampler filter length */
#define MAX_DELAY_SURROUND_ANALYSIS 5
#define MAX_BITSTREAM_DELAY 1

/* Data Types ****************************************************************/
typedef struct DELAY *HANDLE_DELAY;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/
FDK_SACENC_ERROR fdk_sacenc_delay_Open(HANDLE_DELAY *phDelay);

FDK_SACENC_ERROR fdk_sacenc_delay_Close(HANDLE_DELAY *phDelay);

FDK_SACENC_ERROR fdk_sacenc_delay_Init(HANDLE_DELAY hDelay, const INT nQmfLen,
                                       const INT nFrameLen,
                                       const INT nCoreCoderDelay,
                                       const INT nSacStreamMuxDelay);

FDK_SACENC_ERROR fdk_sacenc_delay_SubCalulateBufferDelays(HANDLE_DELAY hDel);

/* Set Expert Config Parameters */
FDK_SACENC_ERROR fdk_sacenc_delay_SetDmxAlign(HANDLE_DELAY hDelay,
                                              const INT bDmxAlignIn);

FDK_SACENC_ERROR fdk_sacenc_delay_SetTimeDomDmx(HANDLE_DELAY hDelay,
                                                const INT bTimeDomDmxIn);

FDK_SACENC_ERROR fdk_sacenc_delay_SetSacTimeAlignmentDynamicOut(
    HANDLE_DELAY hDelay, const INT bSacTimeAlignmentDynamicOutIn);

FDK_SACENC_ERROR fdk_sacenc_delay_SetNSacTimeAlignment(
    HANDLE_DELAY hDelay, const INT nSacTimeAlignmentIn);

FDK_SACENC_ERROR fdk_sacenc_delay_SetMinimizeDelay(HANDLE_DELAY hDelay,
                                                   const INT bMinimizeDelay);

/* Get Internal Variables */
INT fdk_sacenc_delay_GetOutputAudioBufferDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetSurroundAnalysisBufferDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetArbDmxAnalysisBufferDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetBitstreamFrameBufferSize(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetDmxAlignBufferDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetDiscardOutFrames(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetInfoDmxDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetInfoCodecDelay(HANDLE_DELAY hDelay);

INT fdk_sacenc_delay_GetInfoDecoderDelay(HANDLE_DELAY hDelay);

#endif /* SACENC_DELAY_H */
