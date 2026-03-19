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

   Author(s):

   Description: matrix memory allocation

*******************************************************************************/

#ifndef FDK_MATRIXCALLOC_H
#define FDK_MATRIXCALLOC_H

#include "machine_type.h"
#include "genericStds.h"

/* It is recommended to use FDK_ALLOCATE_MEMORY_1D instead of fdkCallocMatrix1D
 */
void* fdkCallocMatrix1D(UINT dim1, UINT size);
void* fdkCallocMatrix1D_aligned(UINT dim1, UINT size);
/* It is recommended to use FDK_ALLOCATE_MEMORY_1D_INT instead of
 * fdkCallocMatrix1D_int */
void* fdkCallocMatrix1D_int(UINT dim1, UINT size, MEMORY_SECTION s);
void* fdkCallocMatrix1D_int_aligned(UINT dim1, UINT size, MEMORY_SECTION s);
/* It is recommended to use FDK_FREE_MEMORY_1D instead of fdkFreeMatrix1D */
void fdkFreeMatrix1D(void* p);
void fdkFreeMatrix1D_aligned(void* p);

/* It is recommended to use FDK_ALLOCATE_MEMORY_2D instead of fdkCallocMatrix2D
 */
void** fdkCallocMatrix2D(UINT dim1, UINT dim2, UINT size);
void** fdkCallocMatrix2D_aligned(UINT dim1, UINT dim2, UINT size);
/* It is recommended to use FDK_ALLOCATE_MEMORY_2D_INT instead of
 * fdkCallocMatrix2D_int */
void** fdkCallocMatrix2D_int(UINT dim1, UINT dim2, UINT size, MEMORY_SECTION s);
/* It is recommended to use FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED instead of
 * fdkCallocMatrix2D_int_aligned */
void** fdkCallocMatrix2D_int_aligned(UINT dim1, UINT dim2, UINT size,
                                     MEMORY_SECTION s);
/* It is recommended to use FDK_FREE_MEMORY_2D instead of fdkFreeMatrix2D */
void fdkFreeMatrix2D(void** p);
/* It is recommended to use FDK_FREE_MEMORY_2D_ALIGNED instead of
 * fdkFreeMatrix2D_aligned */
void fdkFreeMatrix2D_aligned(void** p);

/* It is recommended to use FDK_ALLOCATE_MEMORY_3D instead of fdkCallocMatrix3D
 */
void*** fdkCallocMatrix3D(UINT dim1, UINT dim2, UINT dim3, UINT size);
/* It is recommended to use FDK_ALLOCATE_MEMORY_3D_INT instead of
 * fdkCallocMatrix3D_int */
void*** fdkCallocMatrix3D_int(UINT dim1, UINT dim2, UINT dim3, UINT size,
                              MEMORY_SECTION s);
/* It is recommended to use FDK_FREE_MEMORY_3D instead of fdkFreeMatrix3D */
void fdkFreeMatrix3D(void*** p);

#define FDK_ALLOCATE_MEMORY_1D(a, dim1, type)                           \
  if (((a) = (type*)fdkCallocMatrix1D((dim1), sizeof(type))) == NULL) { \
    goto bail;                                                          \
  }

#define FDK_ALLOCATE_MEMORY_1D_ALIGNED(a, dim1, type)                   \
  if (((a) = (type*)fdkCallocMatrix1D_aligned((dim1), sizeof(type))) == \
      NULL) {                                                           \
    goto bail;                                                          \
  }

#define FDK_ALLOCATE_MEMORY_1D_P(a, dim1, type, ptype)                  \
  if (((a) = (ptype)fdkCallocMatrix1D((dim1), sizeof(type))) == NULL) { \
    goto bail;                                                          \
  }

#define FDK_ALLOCATE_MEMORY_1D_INT(a, dim1, type, s)                     \
  if (((a) = (type*)fdkCallocMatrix1D_int((dim1), sizeof(type), (s))) == \
      NULL) {                                                            \
    goto bail;                                                           \
  }

#define FDK_FREE_MEMORY_1D(a)    \
  do {                           \
    fdkFreeMatrix1D((void*)(a)); \
    (a) = NULL;                  \
  } while (0)

#define FDK_FREE_MEMORY_1D_ALIGNED(a)    \
  do {                                   \
    fdkFreeMatrix1D_aligned((void*)(a)); \
    (a) = NULL;                          \
  } while (0)

#define FDK_ALLOCATE_MEMORY_2D(a, dim1, dim2, type)                      \
  if (((a) = (type**)fdkCallocMatrix2D((dim1), (dim2), sizeof(type))) == \
      NULL) {                                                            \
    goto bail;                                                           \
  }

#define FDK_ALLOCATE_MEMORY_2D_INT(a, dim1, dim2, type, s)               \
  if (((a) = (type**)fdkCallocMatrix2D_int((dim1), (dim2), sizeof(type), \
                                           (s))) == NULL) {              \
    goto bail;                                                           \
  }

#define FDK_ALLOCATE_MEMORY_2D_INT_ALIGNED(a, dim1, dim2, type, s) \
  if (((a) = (type**)fdkCallocMatrix2D_int_aligned(                \
           (dim1), (dim2), sizeof(type), (s))) == NULL) {          \
    goto bail;                                                     \
  }

#define FDK_FREE_MEMORY_2D(a)     \
  do {                            \
    fdkFreeMatrix2D((void**)(a)); \
    (a) = NULL;                   \
  } while (0)

#define FDK_FREE_MEMORY_2D_ALIGNED(a)     \
  do {                                    \
    fdkFreeMatrix2D_aligned((void**)(a)); \
    (a) = NULL;                           \
  } while (0)

#define FDK_ALLOCATE_MEMORY_3D(a, dim1, dim2, dim3, type)         \
  if (((a) = (type***)fdkCallocMatrix3D((dim1), (dim2), (dim3),   \
                                        sizeof(type))) == NULL) { \
    goto bail;                                                    \
  }

#define FDK_ALLOCATE_MEMORY_3D_INT(a, dim1, dim2, dim3, type, s)           \
  if (((a) = (type***)fdkCallocMatrix3D_int((dim1), (dim2), (dim3),        \
                                            sizeof(type), (s))) == NULL) { \
    goto bail;                                                             \
  }

#define FDK_FREE_MEMORY_3D(a)      \
  do {                             \
    fdkFreeMatrix3D((void***)(a)); \
    (a) = NULL;                    \
  } while (0)

#endif /* FDK_MATRIXCALLOC_H */
