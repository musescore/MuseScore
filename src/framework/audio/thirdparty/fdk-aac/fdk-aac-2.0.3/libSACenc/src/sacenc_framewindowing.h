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

   Author(s):   Max Neuendorf

   Description: Encoder Library Interface
                Get windows for framing

*******************************************************************************/

#ifndef SACENC_FRAMEWINDOWING_H
#define SACENC_FRAMEWINDOWING_H

/**************************************************************************/ /**
   \file
   Description of file contents
 ******************************************************************************/

/* Includes ******************************************************************/
#include "genericStds.h"
#include "common_fix.h"
#include "sacenc_lib.h"
#include "sacenc_bitstream.h"

/* Defines *******************************************************************/
#define FIXP_WIN FIXP_DBL
#define FX_DBL2FX_WIN(x) (x)
#define DALDATATYPE_WIN DALDATATYPE_DFRACT

typedef enum {
  FW_INTP = 0,
  FW_HOLD = 1

} FW_SLOTTYPE;

typedef enum {
  FW_LEAVE_DIM = 0,
  FW_CHANGE_DIM = 1

} FW_DIMENSION;

/* Data Types ****************************************************************/
typedef struct T_FRAMEWINDOW *HANDLE_FRAMEWINDOW;

typedef struct T_FRAMEWINDOW_CONFIG {
  INT nTimeSlotsMax;
  INT bFrameKeep;

} FRAMEWINDOW_CONFIG;

typedef struct {
  INT slot;
  FW_SLOTTYPE hold;

} FRAMEWIN_DATA;

typedef struct {
  FRAMEWIN_DATA dat[MAX_NUM_PARAMS];
  INT n;

} FRAMEWIN_LIST;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/
FDK_SACENC_ERROR fdk_sacenc_frameWindow_Create(
    HANDLE_FRAMEWINDOW *phFrameWindow);

FDK_SACENC_ERROR fdk_sacenc_frameWindow_Init(
    HANDLE_FRAMEWINDOW hFrameWindow,
    const FRAMEWINDOW_CONFIG *const pFrameWindowConfig);

FDK_SACENC_ERROR fdk_sacenc_frameWindow_Destroy(
    HANDLE_FRAMEWINDOW *phFrameWindow);

FDK_SACENC_ERROR fdk_sacenc_frameWindow_GetWindow(
    HANDLE_FRAMEWINDOW hFrameWindow, INT tr_pos[MAX_NUM_PARAMS],
    const INT timeSlots, FRAMINGINFO *const pFramingInfo,
    FIXP_WIN *pWindowAna__FDK[MAX_NUM_PARAMS],
    FRAMEWIN_LIST *const pFrameWinList, const INT avoid_keep);

FDK_SACENC_ERROR fdk_sacenc_analysisWindowing(
    const INT nTimeSlots, const INT startTimeSlot,
    FIXP_WIN *pFrameWindowAna__FDK, const FIXP_DPK *const *const ppDataIn__FDK,
    FIXP_DPK *const *const ppDataOut__FDK, const INT nHybridBands,
    const INT dim);

#endif /* SACENC_FRAMEWINDOWING_H */
