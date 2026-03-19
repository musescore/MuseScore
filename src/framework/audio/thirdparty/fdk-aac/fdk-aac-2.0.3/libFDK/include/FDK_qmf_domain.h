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

/******************* Library for basic calculation routines ********************

   Author(s):   Matthias Hildenbrand

   Description: Module to efficiently handle QMF data for multiple channels and
                to share the data between e.g. SBR and MPS

*******************************************************************************/

#ifndef FDK_QMF_DOMAIN_H
#define FDK_QMF_DOMAIN_H

#include "qmf.h"

typedef enum {
  QMF_DOMAIN_OK = 0x0, /*!< No error occurred. */
  QMF_DOMAIN_OUT_OF_MEMORY =
      0x1, /*!< QMF-Configuration demands for more memory than allocated on
              heap. */
  QMF_DOMAIN_INIT_ERROR =
      0x2, /*!< An error during filterbank-setup occurred. */
  QMF_DOMAIN_RESAMPLER_INIT_ERROR =
      0x3 /*!< An error during QMF-resampler-setup occurred. */
} QMF_DOMAIN_ERROR;

#define CMPLX_MOD (2)

#define QMF_MAX_WB_SECTIONS (5) /* maximum number of workbuffer sections */
#define QMF_WB_SECTION_SIZE (1024 * 2)

H_ALLOC_MEM_OVERLAY(QmfWorkBufferCore1, FIXP_DBL)
H_ALLOC_MEM_OVERLAY(QmfWorkBufferCore3, FIXP_DBL)
H_ALLOC_MEM_OVERLAY(QmfWorkBufferCore4, FIXP_DBL)
H_ALLOC_MEM_OVERLAY(QmfWorkBufferCore6, FIXP_DBL)
H_ALLOC_MEM_OVERLAY(QmfWorkBufferCore7, FIXP_DBL)

#define QMF_DOMAIN_MAX_ANALYSIS_QMF_BANDS (64)
#define QMF_DOMAIN_MAX_SYNTHESIS_QMF_BANDS (QMF_MAX_SYNTHESIS_BANDS)
#define QMF_DOMAIN_MAX_QMF_PROC_BANDS (64)
#define QMF_DOMAIN_MAX_TIMESLOTS (64)
#define QMF_DOMAIN_MAX_OV_TIMESLOTS (12)

#define QMF_DOMAIN_ANALYSIS_QMF_BANDS_16 (16)
#define QMF_DOMAIN_ANALYSIS_QMF_BANDS_24 (24)
#define QMF_DOMAIN_ANALYSIS_QMF_BANDS_32 (32)

#define QMF_DOMAIN_TIMESLOTS_16 (16)
#define QMF_DOMAIN_TIMESLOTS_32 (32)

#define QMF_DOMAIN_OV_TIMESLOTS_16 (3)
#define QMF_DOMAIN_OV_TIMESLOTS_32 (6)

H_ALLOC_MEM(AnaQmfStates, FIXP_DBL)
H_ALLOC_MEM(SynQmfStates, FIXP_QSS)
H_ALLOC_MEM(QmfSlotsReal, FIXP_DBL *)
H_ALLOC_MEM(QmfSlotsImag, FIXP_DBL *)
H_ALLOC_MEM(QmfOverlapBuffer, FIXP_DBL)

H_ALLOC_MEM(AnaQmfStates16, FIXP_DBL)
H_ALLOC_MEM(AnaQmfStates24, FIXP_DBL)
H_ALLOC_MEM(AnaQmfStates32, FIXP_DBL)
H_ALLOC_MEM(QmfSlotsReal16, FIXP_DBL *)
H_ALLOC_MEM(QmfSlotsReal32, FIXP_DBL *)
H_ALLOC_MEM(QmfSlotsImag16, FIXP_DBL *)
H_ALLOC_MEM(QmfSlotsImag32, FIXP_DBL *)
H_ALLOC_MEM(QmfOverlapBuffer16, FIXP_DBL)
H_ALLOC_MEM(QmfOverlapBuffer32, FIXP_DBL)

/**
 * Structure to hold the configuration data which is global whithin a QMF domain
 * instance.
 */
typedef struct {
  UCHAR qmfDomainExplicitConfig;   /*!< Flag to signal that QMF domain is set
                                      explicitly instead of SBR and MPS init
                                      routines. */
  UCHAR nInputChannels;            /*!< Number of QMF input channels. */
  UCHAR nInputChannels_requested;  /*!< Corresponding requested not yet active
                                      configuration parameter. */
  UCHAR nOutputChannels;           /*!< Number of QMF output channels. */
  UCHAR nOutputChannels_requested; /*!< Corresponding requested not yet active
                                      configuration parameter. */
  UCHAR
  parkChannel; /*!< signal to automatically allocate additional memory to
                  park a channel if only one processing channel is
                  available. */
  UCHAR parkChannel_requested;
  FIXP_DBL *
      pWorkBuffer[QMF_MAX_WB_SECTIONS]; /*!< Pointerarray to volatile memory. */
  UINT flags; /*!< Flags to be set on all QMF analysis/synthesis filter
                 instances. */
  UINT flags_requested; /*!< Corresponding requested not yet active
                           configuration parameter. */
  UCHAR nBandsAnalysis; /*!< Number of QMF analysis bands for all input
                           channels. */
  UCHAR nBandsAnalysis_requested; /*!< Corresponding requested not yet active
                                     configuration parameter. */
  USHORT nBandsSynthesis; /*!< Number of QMF synthesis bands for all output
                             channels. */
  USHORT
  nBandsSynthesis_requested; /*!< Corresponding requested not yet active
                                configuration parameter. */
  UCHAR nQmfTimeSlots; /*!< Number of QMF time slots (stored in work buffer
                          memory). */
  UCHAR nQmfTimeSlots_requested; /*!< Corresponding requested not yet active
                                    configuration parameter. */
  UCHAR
  nQmfOvTimeSlots; /*!< Number of QMF overlap/delay time slots (stored in
                      persistent memory). */
  UCHAR nQmfOvTimeSlots_requested; /*!< Corresponding requested not yet active
                                      configuration parameter. */
  UCHAR nQmfProcBands; /*!< Number of QMF bands which are processed by the
                          decoder. Typically this is equal to nBandsSynthesis
                          but it may differ if the QMF based resampler is being
                          used. */
  UCHAR nQmfProcBands_requested; /*!< Corresponding requested not yet active
                                    configuration parameter. */
  UCHAR
  nQmfProcChannels; /*!< Number of complete QMF channels which need to
                       coexist in memory at the same time. For most cases
                       this is 1 which means the work buffer can be shared
                       between audio channels. */
  UCHAR
  nQmfProcChannels_requested; /*!< Corresponding requested not yet active
                                 configuration parameter. */
} FDK_QMF_DOMAIN_GC;
typedef FDK_QMF_DOMAIN_GC *HANDLE_FDK_QMF_DOMAIN_GC;

/**
 * Structure representing one QMF input channel. This includes the QMF analysis
 * and the QMF domain data representation needed by the codec. Work buffer data
 * may be shared between channels if the codec processes all QMF channels in a
 * consecutive order.
 */
typedef struct {
  HANDLE_FDK_QMF_DOMAIN_GC
  pGlobalConf;               /*!< Pointer to global configuration structure. */
  QMF_FILTER_BANK fb;        /*!< QMF (analysis) filter bank structure. */
  QMF_SCALE_FACTOR scaling;  /*!< Structure with scaling information. */
  UCHAR workBuf_nTimeSlots;  /*!< Work buffer dimension for this channel is
                                (workBuf_nTimeSlots * workBuf_nBands *
                                CMPLX_MOD). */
  UCHAR workBuf_nBands;      /*!< Work buffer dimension for this channel is
                                (workBuf_nTimeSlots * workBuf_nBands * CMPLX_MOD). */
  USHORT workBufferOffset;   /*!< Offset within work buffer. */
  USHORT workBufferSectSize; /*!< Size of work buffer section. */
  FIXP_DBL *
      pAnaQmfStates; /*!< Pointer to QMF analysis states (persistent memory). */
  FIXP_DBL
  *pOverlapBuffer;        /*!< Pointer to QMF overlap/delay memory (persistent
                             memory). */
  FIXP_DBL **pWorkBuffer; /*!< Pointer array to available work buffers. */
  FIXP_DBL *
      *hQmfSlotsReal; /*!< Handle for QMF real data time slot pointer array. */
  FIXP_DBL **hQmfSlotsImag; /*!< Handle for QMF imaginary data time slot pointer
                               array. */
} FDK_QMF_DOMAIN_IN;
typedef FDK_QMF_DOMAIN_IN *HANDLE_FDK_QMF_DOMAIN_IN;

/**
 * Structure representing one QMF output channel.
 */
typedef struct {
  QMF_FILTER_BANK fb;      /*!< QMF (synthesis) filter bank structure. */
  FIXP_QSS *pSynQmfStates; /*!< Pointer to QMF synthesis states (persistent
                              memory). */
} FDK_QMF_DOMAIN_OUT;
typedef FDK_QMF_DOMAIN_OUT *HANDLE_FDK_QMF_DOMAIN_OUT;

/**
 * Structure representing the QMF domain for multiple channels.
 */
typedef struct {
  FDK_QMF_DOMAIN_GC globalConf; /*!< Global configuration structure. */
  FDK_QMF_DOMAIN_IN
  QmfDomainIn[((8) + (1))]; /*!< Array of QMF domain input structures */
  FDK_QMF_DOMAIN_OUT
  QmfDomainOut[((8) + (1))]; /*!< Array of QMF domain output structures */
} FDK_QMF_DOMAIN;
typedef FDK_QMF_DOMAIN *HANDLE_FDK_QMF_DOMAIN;

/**
 * \brief Check whether analysis- and synthesis-filterbank-states have been
 * initialized.
 *
 * \param qd Pointer to QMF domain structure.
 *
 * \return  1 if initialized, 0 else
 */
int FDK_QmfDomain_IsInitialized(const HANDLE_FDK_QMF_DOMAIN qd);

/**
 * \brief Initialize QMF analysis and synthesis filter banks and set up QMF data
 * representation.
 *
 * \param qd Pointer to QMF domain structure.
 * \param extra_flags Initialize filter banks with extra flags which were not
 * set in the global config flags field.
 *
 * \return  0 on success.
 */
int FDK_QmfDomain_InitFilterBank(HANDLE_FDK_QMF_DOMAIN qd, UINT extra_flags);

/**
 * \brief When QMF processing of one channel is finished copy the overlap/delay
 * part into the persistent memory to be used in the next frame.
 *
 * \param qd_ch Pointer to a QMF domain input channel.
 * \param offset
 *
 * \return  void
 */
void FDK_QmfDomain_SaveOverlap(HANDLE_FDK_QMF_DOMAIN_IN qd_ch, int offset);

/**
 * \brief Get one slot of QMF data and adapt the scaling.
 *
 * \param qd_ch Pointer to a QMF domain input channel.
 * \param ts Time slot number to be obtained.
 * \param start_band Start index of QMF bands to be obtained.
 * \param stop_band Stop index of QMF band to be obtained.
 * \param pQmfOutReal Output buffer (real QMF data).
 * \param pQmfOutImag  Output buffer (imag QMF data).
 * \param exp_out Target exponent (scaling) of data.
 *
 * \return  void
 */
void FDK_QmfDomain_GetSlot(const HANDLE_FDK_QMF_DOMAIN_IN qd_ch, const int ts,
                           const int start_band, const int stop_band,
                           FIXP_DBL *pQmfOutReal, FIXP_DBL *pQmfOutImag,
                           const int exp_out);

/**
 * \brief Direct access to the work buffer associated with a certain channel (no
 * time slot pointer array is used).
 *
 * \param qd_ch Pointer to a QMF domain input channel.
 * \param ts Time slot number to be obtained.
 * \param ppQmfReal Returns the pointer to the requested part of the work buffer
 * (real time slot).
 * \param ppQmfImag Returns the pointer to the requested part of the work buffer
 * (imag time slot).
 *
 * \return  void
 */
void FDK_QmfDomain_GetWorkBuffer(const HANDLE_FDK_QMF_DOMAIN_IN qd_ch,
                                 const int ts, FIXP_DBL **ppQmfReal,
                                 FIXP_DBL **ppQmfImag);

/**
 * \brief For the case that the work buffer associated to this channel is not
 * identical to the processing channel work buffer copy the data into the
 * processing channel.
 *
 * \param qd_ch Pointer to a QMF domain input channel.
 * \return  void
 */
void FDK_QmfDomain_WorkBuffer2ProcChannel(const HANDLE_FDK_QMF_DOMAIN_IN qd_ch);

/**
 * \brief For the case of stereoCfgIndex3 with HBE the HBE buffer is copied into
 * the processing channel work buffer and the processing channel work buffer is
 * copied into the HBE buffer.
 *
 * \param qd_ch Pointer to a QMF domain input channel.
 * \param ppQmfReal Pointer to a HBE QMF data buffer (real).
 * \param ppQmfImag Pointer to a HBE QMF data buffer (imag).
 *
 * \return  void
 */
void FDK_QmfDomain_QmfData2HBE(HANDLE_FDK_QMF_DOMAIN_IN qd_ch,
                               FIXP_DBL **ppQmfReal, FIXP_DBL **ppQmfImag);

/**
 * \brief Set all fields for requested parametervalues in global config struct
 * FDK_QMF_DOMAIN_GC to 0.
 *
 * \param hgc  Pointer to a QMF domain global config struct.
 */
void FDK_QmfDomain_ClearRequested(HANDLE_FDK_QMF_DOMAIN_GC hgc);

/**
 * \brief Check for parameter-change requests in global config and
 * (re-)configure QMF domain accordingly.
 *
 * \param  hqd  Pointer to QMF domain
 *
 * \return  errorcode
 */
QMF_DOMAIN_ERROR FDK_QmfDomain_Configure(HANDLE_FDK_QMF_DOMAIN hqd);

/**
 * \brief Free QMF workbuffer, QMF persistent memory and configuration
 * variables.
 *
 * \param  hqd  Pointer to QMF domain
 */
void FDK_QmfDomain_FreeMem(HANDLE_FDK_QMF_DOMAIN hqd);

/**
 * \brief Clear QMF overlap buffers and QMF filter bank states.
 *
 * \param  hqd  Pointer to QMF domain
 */
QMF_DOMAIN_ERROR FDK_QmfDomain_ClearPersistentMemory(HANDLE_FDK_QMF_DOMAIN hqd);

/**
 * \brief Free QMF workbuffer and QMF persistent memory.
 *
 * \param  hqd  Pointer to QMF domain
 *
 * \param  dmx_lp_mode  downmix low power mode flag
 */
void FDK_QmfDomain_Close(HANDLE_FDK_QMF_DOMAIN hqd);

#endif /* FDK_QMF_DOMAIN_H */
