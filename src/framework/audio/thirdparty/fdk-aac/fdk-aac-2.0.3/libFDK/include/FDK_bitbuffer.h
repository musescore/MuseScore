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

/******************* Library for basic calculation routines ********************

   Author(s):   M. Lohwasser

   Description: common bitbuffer read/write routines

*******************************************************************************/

#ifndef FDK_BITBUFFER_H
#define FDK_BITBUFFER_H

#include "FDK_archdef.h"
#include "machine_type.h"

/* leave 3 bits headroom so MAX_BUFSIZE can be represented in bits as well. */
#define MAX_BUFSIZE_BYTES (0x10000000)

typedef struct {
  UINT ValidBits;
  UINT ReadOffset;
  UINT WriteOffset;
  UINT BitNdx;

  UCHAR *Buffer;
  UINT bufSize;
  UINT bufBits;
} FDK_BITBUF;

typedef FDK_BITBUF *HANDLE_FDK_BITBUF;

#ifdef __cplusplus
extern "C" {
#endif

extern const UINT BitMask[32 + 1];

/**  The BitBuffer Functions are called straight from FDK_bitstream Interface.
     For Functions functional survey look there.
*/

void FDK_CreateBitBuffer(HANDLE_FDK_BITBUF *hBitBuffer, UCHAR *pBuffer,
                         UINT bufSize);

void FDK_InitBitBuffer(HANDLE_FDK_BITBUF hBitBuffer, UCHAR *pBuffer,
                       UINT bufSize, UINT validBits);

void FDK_ResetBitBuffer(HANDLE_FDK_BITBUF hBitBuffer);

void FDK_DeleteBitBuffer(HANDLE_FDK_BITBUF hBitBuffer);

INT FDK_get(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits);

INT FDK_get32(HANDLE_FDK_BITBUF hBitBuf);

void FDK_put(HANDLE_FDK_BITBUF hBitBuffer, UINT value, const UINT numberOfBits);

INT FDK_getBwd(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits);
void FDK_putBwd(HANDLE_FDK_BITBUF hBitBuffer, UINT value,
                const UINT numberOfBits);

void FDK_pushBack(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits,
                  UCHAR config);

void FDK_pushForward(HANDLE_FDK_BITBUF hBitBuffer, const UINT numberOfBits,
                     UCHAR config);

UINT FDK_getValidBits(HANDLE_FDK_BITBUF hBitBuffer);

INT FDK_getFreeBits(HANDLE_FDK_BITBUF hBitBuffer);

void FDK_Feed(HANDLE_FDK_BITBUF hBitBuffer, const UCHAR inputBuffer[],
              const UINT bufferSize, UINT *bytesValid);

void FDK_Copy(HANDLE_FDK_BITBUF hBitBufDst, HANDLE_FDK_BITBUF hBitBufSrc,
              UINT *bytesValid);

void FDK_Fetch(HANDLE_FDK_BITBUF hBitBuffer, UCHAR outBuf[], UINT *writeBytes);

#ifdef __cplusplus
}
#endif

#endif
