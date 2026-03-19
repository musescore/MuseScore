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

/**************************** SBR decoder library ******************************

   Author(s):   Matthias Hildenbrand

   Description: Decode Predictive Vector Coding Data

*******************************************************************************/

#ifndef PVC_DEC_H
#define PVC_DEC_H

#include "common_fix.h"

#define PVC_DIVMODE_BITS 3
#define PVC_REUSEPVCID_BITS 1
#define PVC_PVCID_BITS 7
#define PVC_GRIDINFO_BITS 1

#define MAX_PVC_ENVELOPES 2
#define PVC_NTIMESLOT 16
#define PVC_NBLOW 3 /* max. number of grouped QMF subbands below SBR range */

#define PVC_NBHIGH_MODE1 8
#define PVC_NBHIGH_MODE2 6
#define PVC_NBHIGH_MAX (PVC_NBHIGH_MODE1)
#define PVC_NS_MAX 16

/** Data for each PVC instance which needs to be persistent accross SBR frames
 */
typedef struct {
  UCHAR kx_last;        /**< Xover frequency of last frame */
  UCHAR pvc_mode_last;  /**< PVC mode of last frame */
  UCHAR Esg_slot_index; /**< Ring buffer index to current Esg time slot */
  UCHAR pvcBorder0;     /**< Start SBR time slot of PVC frame */
  FIXP_DBL Esg[PVC_NS_MAX][PVC_NBLOW]; /**< Esg(ksg,t) of current and 15
                                          previous time slots (ring buffer) in
                                          logarithmical domain */
} PVC_STATIC_DATA;

/** Data for each PVC instance which is valid during one SBR frame */
typedef struct {
  UCHAR pvc_mode;   /**< PVC mode 1 or 2, 0 means legacy SBR */
  UCHAR pvcBorder0; /**< Start SBR time slot of PVC frame */
  UCHAR kx;         /**< Index of the first QMF subband in the SBR range */
  UCHAR RATE;       /**< Number of QMF subband samples per time slot (2 or 4) */
  UCHAR ns; /**< Number of time slots for time-domain smoothing of Esg(ksg,t) */
  const UCHAR
      *pPvcID; /**< Pointer to prediction coefficient matrix index table */
  UCHAR pastEsgSlotsAvail;   /**< Number of past Esg(ksg,t) which are available
                                for smoothing filter */
  const FIXP_SGL *pSCcoeffs; /**< Pointer to smoothing window table */
  SCHAR
  sg_offset_low[PVC_NBLOW + 1]; /**< Offset table for PVC grouping of SBR
                                   subbands below SBR range */
  SCHAR sg_offset_high_kx[PVC_NBHIGH_MAX + 1]; /**< Offset table for PVC
                                                  grouping of SBR subbands in
                                                  SBR range (relativ to kx) */
  UCHAR nbHigh; /**< Number of grouped QMF subbands in the SBR range */
  const SCHAR *pScalingCoef; /**< Pointer to scaling coeff table */
  const UCHAR *pPVCTab1;     /**< PVC mode 1 table */
  const UCHAR *pPVCTab2;     /**< PVC mode 2 table */
  const UCHAR *pPVCTab1_dp;  /**< Mapping of pvcID to PVC mode 1 table */
  FIXP_DBL predEsg[PVC_NTIMESLOT]
                  [PVC_NBHIGH_MAX]; /**< Predicted Energy in linear domain */
  int predEsg_exp[PVC_NTIMESLOT];   /**< Exponent of predicted Energy in linear
                                       domain */
  int predEsg_expMax;               /**< Maximum of predEsg_exp[] */
} PVC_DYNAMIC_DATA;

/**
 * \brief Initialize PVC data structures for current frame (call if pvcMode =
 * 0,1,2)
 * \param[in] pPvcStaticData Pointer to PVC persistent data
 * \param[out] pPvcDynamicData Pointer to PVC dynamic data
 * \param[in] pvcMode PVC mode 1 or 2, 0 means legacy SBR
 * \param[in] ns Number of time slots for time-domain smoothing of Esg(ksg,t)
 * \param[in] RATE Number of QMF subband samples per time slot (2 or 4)
 * \param[in] kx Index of the first QMF subband in the SBR range
 * \param[in] pvcBorder0 Start SBR time slot of PVC frame
 * \param[in] pPvcID Pointer to array of PvcIDs read from bitstream
 */
int pvcInitFrame(PVC_STATIC_DATA *pPvcStaticData,
                 PVC_DYNAMIC_DATA *pPvcDynamicData, const UCHAR pvcMode,
                 const UCHAR ns, const int RATE, const int kx,
                 const int pvcBorder0, const UCHAR *pPvcID);

/**
 * \brief Wrapper function for pvcDecodeTimeSlot() to decode PVC data of one
 * frame (call if pvcMode = 1,2)
 * \param[in,out] pPvcStaticData Pointer to PVC persistent data
 * \param[in,out] pPvcDynamicData Pointer to PVC dynamic data
 * \param[in] qmfBufferReal Pointer to array with real QMF subbands
 * \param[in] qmfBufferImag Pointer to array with imag QMF subbands
 * \param[in] overlap Number of QMF overlap slots
 * \param[in] qmfExponentOverlap Exponent of qmfBuffer (low part) of overlap
 * slots
 * \param[in] qmfExponentCurrent Exponent of qmfBuffer (low part)
 */
void pvcDecodeFrame(PVC_STATIC_DATA *pPvcStaticData,
                    PVC_DYNAMIC_DATA *pPvcDynamicData, FIXP_DBL **qmfBufferReal,
                    FIXP_DBL **qmfBufferImag, const int overlap,
                    const int qmfExponentOverlap, const int qmfExponentCurrent);

/**
 * \brief Decode PVC data for one SBR time slot (call if pvcMode = 1,2)
 * \param[in,out] pPvcStaticData Pointer to PVC persistent data
 * \param[in,out] pPvcDynamicData Pointer to PVC dynamic data
 * \param[in] qmfBufferReal Pointer to array with real QMF subbands
 * \param[in] qmfBufferImag Pointer to array with imag QMF subbands
 * \param[in] qmfExponent Exponent of qmfBuffer of current time slot
 * \param[in] pvcBorder0 Start SBR time slot of PVC frame
 * \param[in] timeSlotNumber Number of current SBR time slot (0..15)
 * \param[out] predictedEsgSlot Predicted Energy of current time slot
 * \param[out] predictedEsg_exp Exponent of predicted Energy of current time
 * slot
 */
void pvcDecodeTimeSlot(PVC_STATIC_DATA *pPvcStaticData,
                       PVC_DYNAMIC_DATA *pPvcDynamicData,
                       FIXP_DBL **qmfSlotReal, FIXP_DBL **qmfSlotImag,
                       const int qmfExponent, const int pvcBorder0,
                       const int timeSlotNumber, FIXP_DBL predictedEsgSlot[],
                       int *predictedEsg_exp);

/**
 * \brief Finish the current PVC frame (call if pvcMode = 0,1,2)
 * \param[in,out] pPvcStaticData Pointer to PVC persistent data
 * \param[in,out] pPvcDynamicData Pointer to PVC dynamic data
 */
void pvcEndFrame(PVC_STATIC_DATA *pPvcStaticData,
                 PVC_DYNAMIC_DATA *pPvcDynamicData);

/**
 * \brief Expand predicted PVC grouped energies to full QMF subband resolution
 * \param[in] pPvcDynamicData Pointer to PVC dynamic data
 * \param[in] timeSlot Number of current SBR time slot (0..15)
 * \param[in] lengthOutputVector Lenght of output vector
 * \param[out] pOutput Output array for predicted energies
 * \param[out] pOutput_exp Exponent of predicted energies
 */
void expandPredEsg(const PVC_DYNAMIC_DATA *pPvcDynamicData, const int timeSlot,
                   const int lengthOutputVector, FIXP_DBL *pOutput,
                   SCHAR *pOutput_exp);

#endif /* PVC_DEC_H*/
