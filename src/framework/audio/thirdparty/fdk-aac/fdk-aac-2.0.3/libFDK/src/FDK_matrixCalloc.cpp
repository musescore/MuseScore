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

#include "FDK_matrixCalloc.h"

#include "genericStds.h"

void *fdkCallocMatrix1D_aligned(UINT dim1, UINT size) {
  return FDKaalloc(dim1 * size, ALIGNMENT_DEFAULT);
}

void *fdkCallocMatrix1D_int(UINT dim, UINT size, MEMORY_SECTION s) {
  return FDKcalloc_L(dim, size, s);
}

void *fdkCallocMatrix1D_int_aligned(UINT dim, UINT size, MEMORY_SECTION s) {
  return FDKaalloc_L(dim * size, ALIGNMENT_DEFAULT, s);
}

void fdkFreeMatrix1D(void *p) {
  if (p != NULL) {
    FDKfree_L(p);
  }
}

void fdkFreeMatrix1D_aligned(void *p) {
  if (p != NULL) {
    FDKafree_L(p);
  }
}

void *fdkCallocMatrix1D(UINT dim1, UINT size) { return FDKcalloc(dim1, size); }

/* 2D */
void **fdkCallocMatrix2D(UINT dim1, UINT dim2, UINT size) {
  void **p1;
  UINT i;
  char *p2;
  if (!dim1 || !dim2) return NULL;
  if ((p1 = (void **)fdkCallocMatrix1D(dim1, sizeof(void *))) == NULL) {
    goto bail;
  }
  if ((p2 = (char *)fdkCallocMatrix1D(dim1 * dim2, size)) == NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    p2 += dim2 * size;
  }
bail:
  return p1;
}

void **fdkCallocMatrix2D_aligned(UINT dim1, UINT dim2, UINT size) {
  void **p1;
  UINT i;
  char *p2;
  if (!dim1 || !dim2) return NULL;
  if ((p1 = (void **)fdkCallocMatrix1D(dim1, sizeof(void *))) == NULL) {
    goto bail;
  }
  if ((p2 = (char *)fdkCallocMatrix1D_aligned(dim1 * dim2, size)) == NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    p2 += dim2 * size;
  }
bail:
  return p1;
}

void fdkFreeMatrix2D(void **p) {
  if (!p) return;
  fdkFreeMatrix1D(p[0]);
  fdkFreeMatrix1D(p);
}

void fdkFreeMatrix2D_aligned(void **p) {
  if (!p) return;
  fdkFreeMatrix1D_aligned(p[0]);
  fdkFreeMatrix1D(p);
}

void **fdkCallocMatrix2D_int(UINT dim1, UINT dim2, UINT size,
                             MEMORY_SECTION s) {
  void **p1;
  UINT i;
  char *p2;

  if (!dim1 || !dim2) return NULL;
  if ((p1 = (void **)fdkCallocMatrix1D_int(dim1, sizeof(void *), s)) == NULL) {
    goto bail;
  }
  if ((p2 = (char *)fdkCallocMatrix1D_int(dim1 * dim2, size, s)) == NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    p2 += dim2 * size;
  }
bail:
  return p1;
}

void **fdkCallocMatrix2D_int_aligned(UINT dim1, UINT dim2, UINT size,
                                     MEMORY_SECTION s) {
  void **p1;
  UINT i;
  char *p2;

  if (!dim1 || !dim2) return NULL;
  if ((p1 = (void **)fdkCallocMatrix1D_int(dim1, sizeof(void *), s)) == NULL) {
    goto bail;
  }
  if ((p2 = (char *)fdkCallocMatrix1D_int_aligned(dim1 * dim2, size, s)) ==
      NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    p2 += dim2 * size;
  }
bail:
  return p1;
}

/* 3D */
void ***fdkCallocMatrix3D(UINT dim1, UINT dim2, UINT dim3, UINT size) {
  void ***p1;
  UINT i, j;
  void **p2;
  char *p3;

  if (!dim1 || !dim2 || !dim3) return NULL;
  if ((p1 = (void ***)fdkCallocMatrix1D(dim1, sizeof(void **))) == NULL) {
    goto bail;
  }
  if ((p2 = (void **)fdkCallocMatrix1D(dim1 * dim2, sizeof(void *))) == NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  p1[0] = p2;
  if ((p3 = (char *)fdkCallocMatrix1D(dim1 * dim2 * dim3, size)) == NULL) {
    fdkFreeMatrix1D(p1);
    fdkFreeMatrix1D(p2);
    p1 = NULL;
    p2 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    for (j = 0; j < dim2; j++) {
      p2[j] = p3;
      p3 += dim3 * size;
    }
    p2 += dim2;
  }
bail:
  return p1;
}

void fdkFreeMatrix3D(void ***p) {
  if (!p) return;
  if (p[0] != NULL) fdkFreeMatrix1D(p[0][0]);
  fdkFreeMatrix1D(p[0]);
  fdkFreeMatrix1D(p);
}

void ***fdkCallocMatrix3D_int(UINT dim1, UINT dim2, UINT dim3, UINT size,
                              MEMORY_SECTION s) {
  void ***p1;
  UINT i, j;
  void **p2;
  char *p3;

  if (!dim1 || !dim2 || !dim3) return NULL;
  if ((p1 = (void ***)fdkCallocMatrix1D_int(dim1, sizeof(void **), s)) ==
      NULL) {
    goto bail;
  }
  if ((p2 = (void **)fdkCallocMatrix1D_int(dim1 * dim2, sizeof(void *), s)) ==
      NULL) {
    fdkFreeMatrix1D(p1);
    p1 = NULL;
    goto bail;
  }
  p1[0] = p2;
  if ((p3 = (char *)fdkCallocMatrix1D_int(dim1 * dim2 * dim3, size, s)) ==
      NULL) {
    fdkFreeMatrix1D(p1);
    fdkFreeMatrix1D(p2);
    p1 = NULL;
    p2 = NULL;
    goto bail;
  }
  for (i = 0; i < dim1; i++) {
    p1[i] = p2;
    for (j = 0; j < dim2; j++) {
      p2[j] = p3;
      p3 += dim3 * size;
    }
    p2 += dim2;
  }
bail:
  return p1;
}
