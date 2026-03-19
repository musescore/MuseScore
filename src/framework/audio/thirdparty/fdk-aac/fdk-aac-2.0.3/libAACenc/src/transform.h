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

   Author(s):   M. Werner

   Description: MDCT Transform

*******************************************************************************/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "mdct.h"
#include "common_fix.h"

#define WTS0 1
#define WTS1 0
#define WTS2 -2

/**
 * \brief: Performe MDCT transform of time domain data.
 * \param timeData pointer to time domain input signal.
 * \param mdctData pointer to store frequency domain output data.
 * \param blockType index indicating the type of block. Either
 *        LONG_WINDOW, START_WINDOW, SHORT_WINDOW or STOP_WINDOW.
 * \param windowShape index indicating the window slope type to be used.
 *        Values allowed are either SINE_WINDOW or KBD_WINDOW.
 * \param previndowShape index indicating the window slope type used
 *        in the last frame.
 *        Values allowed are either SINE_WINDOW or KBD_WINDOW.
 * \param frameLength length of the block. Either 1024 or 960.
 * \param mdctData_e pointer to an INT where the exponent of the frequency
 *        domain output data is stored into.
 * \param filterType xxx
 * \return 0 in case of success, non-zero in case of error (inconsistent
 * parameters).
 */
INT FDKaacEnc_Transform_Real(const INT_PCM* pTimeData,
                             FIXP_DBL* RESTRICT mdctData, const INT blockType,
                             const INT windowShape, INT* prevWindowShape,
                             H_MDCT mdctPers, const INT frameLength,
                             INT* pMdctData_e, INT filterType);

/**
 * \brief: Performe ELD filterbnank transform of time domain data.
 * \param timeData pointer to time domain input signal.
 * \param mdctData pointer to store frequency domain output data.
 * \param blockType index indicating the type of block. Either
 *        LONG_WINDOW, START_WINDOW, SHORT_WINDOW or STOP_WINDOW.
 * \param windowShape index indicating the window slope type to be used.
 *        Values allowed are either SINE_WINDOW or KBD_WINDOW.
 * \param previndowShape index indicating the window slope type used
 *        in the last frame.
 *        Values allowed are either SINE_WINDOW or KBD_WINDOW.
 * \param frameLength length of the block. Either 1024 or 960.
 * \param mdctData_e pointer to an INT where the exponent of the frequency
 *        domain output data is stored into.
 * \param filterType xxx
 * \param overlapAddBuffer overlap add buffer for overlap of ELD filterbank
 * \return 0 in case of success, non-zero in case of error (inconsistent
 * parameters).
 */
INT FDKaacEnc_Transform_Real_Eld(const INT_PCM* pTimeData,
                                 FIXP_DBL* RESTRICT mdctData,
                                 const INT blockType, const INT windowShape,
                                 INT* prevWindowShape, const INT frameLength,
                                 INT* mdctData_e, INT filterType,
                                 FIXP_DBL* RESTRICT overlapAddBuffer);

#endif /* #!defined (TRANSFORM_H) */
