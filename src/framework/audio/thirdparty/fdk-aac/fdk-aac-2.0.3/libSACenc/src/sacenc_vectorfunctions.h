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

   Author(s):   Josef Hoepfl

   Description: Encoder Library Interface
                vector functions

*******************************************************************************/

/*****************************************************************************
\file
This file contains vector functions
******************************************************************************/

#ifndef SACENC_VECTORFUNCTIONS_H
#define SACENC_VECTORFUNCTIONS_H

/* Includes ******************************************************************/
#include "common_fix.h"

/* Defines *******************************************************************/
#define SUM_UP_STATIC_SCALE 0
#define SUM_UP_DYNAMIC_SCALE 1

/* Data Types ****************************************************************/

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/

/**
 * \brief          Vector function : Sum up complex power
 *
 *                 Description : ret = sum( re{X[i]} * re{X[i]} + im{X[i]} *
 * im{X[i]} ),  i=0,...,n-1 ret is scaled by outScaleFactor
 *
 * \param          const FIXP_DPK x[]
 *                 Input: complex vector of the length n
 *
 * \param          int scaleMode
 *                 Input: choose static or dynamic scaling
 * (SUM_UP_DYNAMIC_SCALE/SUM_UP_STATIC_SCALE)
 *
 * \param          int inScaleFactor
 *                 Input: determine headroom bits for the complex input vector
 *
 * \param          int outScaleFactor
 *                 Output: complete scaling in energy calculation
 *
 * \return         FIXP_DBL ret
 */
FIXP_DBL sumUpCplxPow2(const FIXP_DPK *const x, const INT scaleMode,
                       const INT inScaleFactor, INT *const outScaleFactor,
                       const INT n);

/**
 * \brief          Vector function : Sum up complex power
 *
 *                 Description : ret = sum( re{X[i][j]} * re{X[i][]} +
 * im{X[i][]} * im{X[i][]} ),  i=sDim1,...,nDim1-1 i=sDim2,...,nDim2-1 ret is
 * scaled by outScaleFactor
 *
 * \param          const FIXP_DPK x[]
 *                 Input: complex vector of the length n
 *
 * \param          int scaleMode
 *                 Input: choose static or dynamic scaling
 * (SUM_UP_DYNAMIC_SCALE/SUM_UP_STATIC_SCALE)
 *
 * \param          int inScaleFactor
 *                 Input: determine headroom bits for the complex input vector
 *
 * \param          int outScaleFactor
 *                 Output: complete scaling in energy calculation
 *
 * \param          int sDim1
 *                 Input: start index for loop counter in dimension 1
 *
 * \param          int nDim1
 *                 Input: loop counter in dimension 1
 *
 * \param          int sDim2
 *                 Input: start index for loop counter in dimension 2
 *
 * \param          int nDim2
 *                 Input: loop counter in dimension 2
 *
 * \return         FIXP_DBL ret
 */
FIXP_DBL sumUpCplxPow2Dim2(const FIXP_DPK *const *const x, const INT scaleMode,
                           const INT inScaleFactor, INT *const outScaleFactor,
                           const INT sDim1, const INT nDim1, const INT sDim2,
                           const INT nDim2);

/**
 * \brief          Vector function : Z[i] = X[i],  i=0,...,n-1
 *
 *                 Description : re{Z[i]} = re{X[i]},  i=0,...,n-1
 *                               im{Z[i]} = im{X[i]},  i=0,...,n-1
 *
 *                 Copy complex vector X[] to complex vector Z[].
 *                 It is allowed to overlay X[] with Z[].
 *
 * \param          FIXP_DPK Z[]
 *                 Output: vector of the length n
 *
 * \param          const FIXP_DPK X[]
 *                 Input: vector of the length n
 *
 * \param          int n
 *                 Input: length of vector Z[] and X[]
 *
 * \return         void
 */
void copyCplxVec(FIXP_DPK *const Z, const FIXP_DPK *const X, const INT n);

/**
 * \brief          Vector function : Z[i] = a,  i=0,...,n-1
 *
 *                 Description : re{Z[i]} = a,  i=0,...,n-1
 *                               im{Z[i]} = a,  i=0,...,n-1
 *
 *                 Set real and imaginary part of the complex value Z to a.
 *
 * \param          FIPX_DPK Z[]
 *                 Output: vector of the length n
 *
 * \param          const FIXP_DBL a
 *                 Input: constant value
 *
 * \param          int n
 *                 Input: length of vector Z[]
 *
 * \return         void
 */
void setCplxVec(FIXP_DPK *const Z, const FIXP_DBL a, const INT n);

/**
 * \brief          Vector function : Calculate complex-valued result of complex
 * scalar product
 *
 *                 Description : re{Z} = sum( re{X[i]} * re{Y[i]} + im{X[i]} *
 * im{Y[i]},  i=0,...,n-1 ) im{Z} = sum( im{X[i]} * re{Y[i]} - re{X[i]} *
 * im{Y[i]},  i=0,...,n-1 )
 *
 *                 The function returns the complex-valued result of the complex
 * scalar product at the address of Z. The result is scaled by scaleZ.
 *
 * \param          FIXP_DPK *Z
 *                 Output: pointer to Z
 *
 * \param          const FIXP_DPK *const *const X
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DPK *const *const Y
 *                 Input: vector of the length n
 *
 * \param          int scaleX
 *                 Input: scalefactor of vector X[]
 *
 * \param          int scaleY
 *                 Input: scalefactor of vector Y[]
 *
 * \param          int scaleZ
 *                 Output: scalefactor of vector Z[]
 *
 * \param          int sDim1
 *                 Input: start index for loop counter in dimension 1
 *
 * \param          int nDim1
 *                 Input: loop counter in dimension 1
 *
 * \param          int sDim2
 *                 Input: start index for loop counter in dimension 2
 *
 * \param          int nDim2
 *                 Input: loop counter in dimension 2
 *
 * \return         void
 */
void cplx_cplxScalarProduct(FIXP_DPK *const Z, const FIXP_DPK *const *const X,
                            const FIXP_DPK *const *const Y, const INT scaleX,
                            const INT scaleY, INT *const scaleZ,
                            const INT sDim1, const INT nDim1, const INT sDim2,
                            const INT nDim2);

/**
 * \brief          Vector function : Calculate correlation
 *
 *                 Description : z[i] = pr12[i] / sqrt(p1[i]*p2[i]) ,
 * i=0,...,n-1
 *
 * \param          FIXP_DBL z[]
 *                 Output: vector of length n
 *
 * \param          const FIXP_DBL pr12[]
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DBL p1[]
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DBL p2[]
 *                 Input: vector of the length n
 *
 * \param          int n
 *                 Input: length of vector pr12[], p1[] and p2[]
 *
 * \return         void
 */
void FDKcalcCorrelationVec(FIXP_DBL *const z, const FIXP_DBL *const pr12,
                           const FIXP_DBL *const p1, const FIXP_DBL *const p2,
                           const INT n);

/**
 * \brief          Vector function : Calculate coherence
 *
 *                 Description : z[i] = sqrt( (p12r[i]*p12r[i] +
 * p12i[i]*p12i[i]) / (p1[i]*p2[i]) ),  i=0,...,n-1
 *
 * \param          FIXP_DBL z[]
 *                 Output: vector of length n
 *
 * \param          const FIXP_DBL p12r[]
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DBL p12i[]
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DBL p1[]
 *                 Input: vector of the length n
 *
 * \param          const FIXP_DBL p2[]
 *                 Input: vector of the length n
 *
 * \param          int scaleP12[]
 *                 Input: scalefactor of p12r and p12i
 *
 * \param          int scaleP
 *                 Input: scalefactor of p1 and p2
 *
 * \param          int n
 *                 Input: length of vector p12r[], p12i[], p1[] and p2[]
 *
 * \return         void
 */
void calcCoherenceVec(FIXP_DBL *const z, const FIXP_DBL *const p12r,
                      const FIXP_DBL *const p12i, const FIXP_DBL *const p1,
                      const FIXP_DBL *const p2, const INT scaleP12,
                      const INT scaleP, const INT n);

/**
 * \brief          Vector function : Z[j][i] = a[pb] * X[j][i] + b[pb] *
 * Y[j][i],  j=0,...,nHybridBands-1;  i=startTimeSlot,...,nTimeSlots-1;
 * pb=0,...,nParameterBands-1
 *
 *                 Description : re{Z[j][i]} = a[pb] * re{X[j][i]} + b[pb] *
 * re{Y[j][i]},  j=0,...,nHybridBands-1;  i=startTimeSlot,...,nTimeSlots-1;
 * pb=0,...,nParameterBands-1 im{Z[j][i]} = a[pb] * im{X[j][i]} + b[pb] *
 * im{Y[j][i]},  j=0,...,nHybridBands-1;
 * i=startTimeSlot,...,nTimeSlots-1;  pb=0,...,nParameterBands-1
 *
 *                 It is allowed to overlay X[] or Y[] with Z[]. The scalefactor
 * of channel 1 is updated with the common scalefactor of channel 1 and
 * channel 2.
 *
 * \param          FIXP_DPK **Z
 *                 Output: vector of the length nHybridBands*nTimeSlots
 *
 * \param          const FIXP_DBL *a
 *                 Input: vector of length nParameterBands
 *
 * \param          const FIXP_DPK **X
 *                 Input: vector of the length nHybridBands*nTimeSlots
 *
 * \param          const FIXP_DBL *b
 *                 Input: vector of length nParameterBands
 *
 * \param          const FIXP_DPK **Y
 *                 Input: vector of the length nHybridBands*nTimeSlots
 *
 * \param          int scale
 *                 Input: scale of vector a and b
 *
 * \param          int *scaleCh1
 *                 Input: scale of ch1
 *
 * \param          int scaleCh2
 *                 Input: scale of ch2
 *
 * \param          UCHAR *pParameterBand2HybridBandOffset
 *                 Input: vector of length nParameterBands
 *
 * \param          int nTimeSlots
 *                 Input: number of time slots
 *
 * \param          int startTimeSlot
 *                 Input: start time slot
 *
 * \return         void
 */
void addWeightedCplxVec(FIXP_DPK *const *const Z, const FIXP_DBL *const a,
                        const FIXP_DPK *const *const X, const FIXP_DBL *const b,
                        const FIXP_DPK *const *const Y, const INT scale,
                        INT *const scaleCh1, const INT scaleCh2,
                        const UCHAR *const pParameterBand2HybridBandOffset,
                        const INT nParameterBands, const INT nTimeSlots,
                        const INT startTimeSlot);

/**
 * \brief          Vector function : Calculate the headroom of a complex vector
 * in a parameter band grid
 *
 * \param          FIXP_DPK **x
 *                 Input: pointer to complex input vector
 *
 * \param          UCHAR *pParameterBand2HybridBandOffset
 *                 Input: pointer to hybrid band offsets
 *
 * \param          int *outScaleFactor
 *                 Input: pointer to ouput scalefactor
 *
 * \param          int startTimeSlot
 *                 Input: start time slot
 *
 * \param          int nTimeSlots
 *                 Input: number of time slot
 *
 * \param          int nParamBands
 *                 Input: number of parameter bands
 *
 * \return         void
 */
void FDKcalcPbScaleFactor(const FIXP_DPK *const *const x,
                          const UCHAR *const pParameterBand2HybridBandOffset,
                          INT *const outScaleFactor, const INT startTimeSlot,
                          const INT nTimeSlots, const INT nParamBands);

/**
 * \brief          Vector function : Calculate the common headroom of two
 * sparate vectors
 *
 * \param          FIXP_DBL *x
 *                 Input: pointer to first input vector
 *
 * \param          FIXP_DBL *y
 *                 Input: pointer to second input vector
 *
 * \param          int n
 *                 Input: number of samples
 *
 * \return         int headromm in bits
 */
INT FDKcalcScaleFactor(const FIXP_DBL *const x, const FIXP_DBL *const y,
                       const INT n);

/**
 * \brief          Vector function : Calculate the headroom of a complex vector
 *
 * \param          FIXP_DPK *x
 *                 Input: pointer to complex input vector
 *
 * \param          INT startBand
 *                 Input: start band
 *
 * \param          INT bands
 *                 Input: number of bands
 *
 * \return         int headromm in bits
 */
INT FDKcalcScaleFactorDPK(const FIXP_DPK *RESTRICT x, const INT startBand,
                          const INT bands);

/* Function / Class Definition ***********************************************/
template <class T>
inline void FDKmemcpy_flex(T *const dst, const INT dstStride,
                           const T *const src, const INT srcStride,
                           const INT nSamples) {
  int i;

  for (i = 0; i < nSamples; i++) {
    dst[i * dstStride] = src[i * srcStride];
  }
}

template <class T>
inline void FDKmemset_flex(T *const x, const T c, const INT nSamples) {
  int i;

  for (i = 0; i < nSamples; i++) {
    x[i] = c;
  }
}

#endif /* SACENC_VECTORFUNCTIONS_H */
