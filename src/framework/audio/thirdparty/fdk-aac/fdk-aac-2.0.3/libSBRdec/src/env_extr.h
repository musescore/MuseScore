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

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  Envelope extraction prototypes
*/

#ifndef ENV_EXTR_H
#define ENV_EXTR_H

#include "sbrdecoder.h"

#include "FDK_bitstream.h"
#include "lpp_tran.h"

#include "psdec.h"
#include "pvc_dec.h"

#define ENV_EXP_FRACT 0
/*!< Shift raw envelope data to support fractional numbers.
  Can be set to 8 instead of 0 to enhance accuracy during concealment.
  This is not required for conformance and #requantizeEnvelopeData() will
  become more expensive.
*/

#define EXP_BITS 6
/*!< Size of exponent-part of a pseudo float envelope value (should be at least
  6). The remaining bits in each word are used for the mantissa (should be at
  least 10). This format is used in the arrays iEnvelope[] and
  sbrNoiseFloorLevel[] in the FRAME_DATA struct which must fit in a certain part
  of the output buffer (See buffer management in sbr_dec.cpp). Exponents and
  mantissas could also be stored in separate arrays. Accessing the exponent or
  the mantissa would be simplified and the masks #MASK_E resp. #MASK_M would
  no longer be required.
*/

#define MASK_M                                                          \
  (((1 << (FRACT_BITS - EXP_BITS)) - 1)                                 \
   << EXP_BITS) /*!< Mask for extracting the mantissa of a pseudo float \
                   envelope value */
#define MASK_E                                                            \
  ((1 << EXP_BITS) - 1) /*!< Mask for extracting the exponent of a pseudo \
                           float envelope value */

#define SIGN_EXT \
  (((SCHAR)-1) ^ \
   MASK_E) /*!< a CHAR-constant with all bits above our sign-bit set */
#define ROUNDING                                                           \
  ((FIXP_SGL)(                                                             \
      1 << (EXP_BITS - 1))) /*!< 0.5-offset for rounding the mantissa of a \
                               pseudo-float envelope value */
#define NRG_EXP_OFFSET                                                         \
  16 /*!< Will be added to the reference energy's exponent to prevent negative \
        numbers */
#define NOISE_EXP_OFFSET                                                \
  38 /*!< Will be added to the noise level exponent to prevent negative \
        numbers */

#define ADD_HARMONICS_FLAGS_SIZE 2 /* ceil(MAX_FREQ_COEFFS/32) */

typedef enum {
  HEADER_NOT_PRESENT,
  HEADER_ERROR,
  HEADER_OK,
  HEADER_RESET
} SBR_HEADER_STATUS;

typedef enum {
  SBR_NOT_INITIALIZED = 0,
  UPSAMPLING = 1,
  SBR_HEADER = 2,
  SBR_ACTIVE = 3
} SBR_SYNC_STATE;

typedef enum { COUPLING_OFF = 0, COUPLING_LEVEL, COUPLING_BAL } COUPLING_MODE;

typedef struct {
  UCHAR nSfb[2]; /*!< Number of SBR-bands for low and high freq-resolution */
  UCHAR nNfb;    /*!< Actual number of noise bands to read from the bitstream*/
  UCHAR numMaster;      /*!< Number of SBR-bands in v_k_master */
  UCHAR lowSubband;     /*!< QMF-band where SBR frequency range starts */
  UCHAR highSubband;    /*!< QMF-band where SBR frequency range ends */
  UCHAR ov_highSubband; /*!< if headerchange applies this value holds the old
                           highband value -> highband value of overlap area;
                             required for overlap in usac when headerchange
                           occurs between XVAR and VARX frame */
  UCHAR limiterBandTable[MAX_NUM_LIMITERS + 1]; /*!< Limiter band table. */
  UCHAR noLimiterBands;                         /*!< Number of limiter bands. */
  UCHAR nInvfBands; /*!< Number of bands for inverse filtering */
  UCHAR
  *freqBandTable[2]; /*!< Pointers to freqBandTableLo and freqBandTableHi */
  UCHAR freqBandTableLo[MAX_FREQ_COEFFS / 2 + 1];
  /*!< Mapping of SBR bands to QMF bands for low frequency resolution */
  UCHAR freqBandTableHi[MAX_FREQ_COEFFS + 1];
  /*!< Mapping of SBR bands to QMF bands for high frequency resolution */
  UCHAR freqBandTableNoise[MAX_NOISE_COEFFS + 1];
  /*!< Mapping of SBR noise bands to QMF bands */
  UCHAR v_k_master[MAX_FREQ_COEFFS + 1];
  /*!< Master BandTable which freqBandTable is derived from */
} FREQ_BAND_DATA;

typedef FREQ_BAND_DATA *HANDLE_FREQ_BAND_DATA;

#define SBRDEC_ELD_GRID 1
#define SBRDEC_SYNTAX_SCAL 2
#define SBRDEC_SYNTAX_USAC 4
#define SBRDEC_SYNTAX_RSVD50 8
#define SBRDEC_USAC_INDEP \
  16 /* Flag indicating that USAC global independency flag is active. */
#define SBRDEC_LOW_POWER \
  32 /* Flag indicating that Low Power QMF mode shall be used. */
#define SBRDEC_PS_DECODED \
  64 /* Flag indicating that PS was decoded and rendered. */
#define SBRDEC_QUAD_RATE                              \
  128 /* Flag indicating that USAC SBR 4:1 is active. \
       */
#define SBRDEC_USAC_HARMONICSBR \
  256 /* Flag indicating that USAC HBE tool is active. */
#define SBRDEC_LD_MPS_QMF \
  512 /* Flag indicating that the LD-MPS QMF shall be used. */
#define SBRDEC_USAC_ITES \
  1024 /* Flag indicating that USAC inter TES tool is active. */
#define SBRDEC_SYNTAX_DRM \
  2048 /* Flag indicating that DRM30/DRM+ reverse syntax is being used. */
#define SBRDEC_ELD_DOWNSCALE \
  4096 /* Flag indicating that ELD downscaled mode decoding is used */
#define SBRDEC_DOWNSAMPLE \
  8192 /* Flag indicating that the downsampling mode is used. */
#define SBRDEC_FLUSH 16384 /* Flag is used to flush all elements in use. */
#define SBRDEC_FORCE_RESET \
  32768 /* Flag is used to force a reset of all elements in use. */
#define SBRDEC_SKIP_QMF_ANA                                               \
  (1 << 21) /* Flag indicating that the input data is provided in the QMF \
               domain. */
#define SBRDEC_SKIP_QMF_SYN                                                \
  (1 << 22) /* Flag indicating that the output data is exported in the QMF \
               domain. */

#define SBRDEC_HDR_STAT_RESET 1
#define SBRDEC_HDR_STAT_UPDATE 2

typedef struct {
  UCHAR ampResolution; /*!< Amplitude resolution of envelope values (0: 1.5dB,
                          1: 3dB) */
  UCHAR
  xover_band; /*!< Start index in #v_k_master[] used for dynamic crossover
                 frequency */
  UCHAR sbr_preprocessing; /*!< SBR prewhitening flag. */
  UCHAR pvc_mode;          /*!< Predictive vector coding mode */
} SBR_HEADER_DATA_BS_INFO;

typedef struct {
  /* Changes in these variables causes a reset of the decoder */
  UCHAR startFreq;   /*!< Index for SBR start frequency */
  UCHAR stopFreq;    /*!< Index for SBR highest frequency */
  UCHAR freqScale;   /*!< 0: linear scale,  1-3 logarithmic scales */
  UCHAR alterScale;  /*!< Flag for coarser frequency resolution */
  UCHAR noise_bands; /*!< Noise bands per octave, read from bitstream*/

  /* don't require reset */
  UCHAR limiterBands; /*!< Index for number of limiter bands per octave */
  UCHAR limiterGains; /*!< Index to select gain limit */
  UCHAR interpolFreq; /*!< Select gain calculation method (1: per QMF channel,
                         0: per SBR band) */
  UCHAR smoothingLength; /*!< Smoothing of gains over time (0: on  1: off) */

} SBR_HEADER_DATA_BS;

typedef struct {
  SBR_SYNC_STATE
  syncState; /*!< The current initialization status of the header */

  UCHAR status; /*!< Flags field used for signaling a reset right before the
                   processing starts and an update from config (e.g. ASC). */
  UCHAR
  frameErrorFlag; /*!< Frame data valid flag. CAUTION: This variable will be
                     overwritten by the flag stored in the element
                     structure. This is necessary because of the frame
                     delay. There it might happen that different slots use
                     the same header. */
  UCHAR numberTimeSlots;       /*!< AAC: 16,15 */
  UCHAR numberOfAnalysisBands; /*!< Number of QMF analysis bands */
  UCHAR timeStep;              /*!< Time resolution of SBR in QMF-slots */
  UINT
      sbrProcSmplRate; /*!< SBR processing sampling frequency (!=
                          OutputSamplingRate)        (always: CoreSamplingRate *
                          UpSamplingFactor; even in single rate mode) */

  SBR_HEADER_DATA_BS bs_data;      /*!< current SBR header. */
  SBR_HEADER_DATA_BS bs_dflt;      /*!< Default sbr header. */
  SBR_HEADER_DATA_BS_INFO bs_info; /*!< SBR info. */

  FREQ_BAND_DATA freqBandData; /*!< Pointer to struct #FREQ_BAND_DATA */
  UCHAR pvcIDprev;
} SBR_HEADER_DATA;

typedef SBR_HEADER_DATA *HANDLE_SBR_HEADER_DATA;

typedef struct {
  UCHAR frameClass;                 /*!< Select grid type */
  UCHAR nEnvelopes;                 /*!< Number of envelopes */
  UCHAR borders[MAX_ENVELOPES + 1]; /*!< Envelope borders (in SBR-timeslots,
                                       e.g. mp3PRO: 0..11) */
  UCHAR freqRes[MAX_ENVELOPES];     /*!< Frequency resolution for each envelope
                                       (0=low, 1=high) */
  SCHAR tranEnv;                    /*!< Transient envelope, -1 if none */
  UCHAR nNoiseEnvelopes;            /*!< Number of noise envelopes */
  UCHAR
  bordersNoise[MAX_NOISE_ENVELOPES + 1]; /*!< borders of noise envelopes */
  UCHAR pvcBorders[MAX_PVC_ENVELOPES + 1];
  UCHAR noisePosition;
  UCHAR varLength;
} FRAME_INFO;

typedef struct {
  FIXP_SGL sfb_nrg_prev[MAX_FREQ_COEFFS]; /*!< Previous envelope (required for
                                             differential-coded values) */
  FIXP_SGL
  prevNoiseLevel[MAX_NOISE_COEFFS]; /*!< Previous noise envelope (required
                                       for differential-coded values) */
  COUPLING_MODE coupling;           /*!< Stereo-mode of previous frame */
  INVF_MODE sbr_invf_mode[MAX_INVF_BANDS]; /*!< Previous strength of filtering
                                              in transposer */
  UCHAR ampRes;         /*!< Previous amplitude resolution (0: 1.5dB, 1: 3dB) */
  UCHAR stopPos;        /*!< Position in time where last envelope ended */
  UCHAR frameErrorFlag; /*!< Previous frame status */
  UCHAR prevSbrPitchInBins; /*!< Previous frame pitchInBins */
  FRAME_INFO prevFrameInfo;
} SBR_PREV_FRAME_DATA;

typedef SBR_PREV_FRAME_DATA *HANDLE_SBR_PREV_FRAME_DATA;

typedef struct {
  int nScaleFactors; /*!< total number of scalefactors in frame */

  FRAME_INFO frameInfo;            /*!< time grid for current frame */
  UCHAR domain_vec[MAX_ENVELOPES]; /*!< Bitfield containing direction of
                                      delta-coding for each envelope
                                      (0:frequency, 1:time) */
  UCHAR domain_vec_noise
      [MAX_NOISE_ENVELOPES]; /*!< Same as above, but for noise envelopes */

  INVF_MODE
  sbr_invf_mode[MAX_INVF_BANDS]; /*!< Strength of filtering in transposer */
  COUPLING_MODE coupling;        /*!< Stereo-mode */
  int ampResolutionCurrentFrame; /*!< Amplitude resolution of envelope values
                                    (0: 1.5dB, 1: 3dB) */

  ULONG addHarmonics[ADD_HARMONICS_FLAGS_SIZE]; /*!< Flags for synthetic sine
                                                   addition (aligned to MSB) */

  FIXP_SGL iEnvelope[MAX_NUM_ENVELOPE_VALUES];       /*!< Envelope data */
  FIXP_SGL sbrNoiseFloorLevel[MAX_NUM_NOISE_VALUES]; /*!< Noise envelope data */
  UCHAR iTESactive; /*!< One flag for each envelope to enable USAC inter-TES */
  UCHAR
  interTempShapeMode[MAX_ENVELOPES]; /*!< USAC inter-TES:
                                        bs_inter_temp_shape_mode[ch][env]
                                        value */
  UCHAR pvcID[PVC_NTIMESLOT];        /*!< One PVC ID value for each time slot */
  UCHAR ns;
  UCHAR sinusoidal_position;

  UCHAR sbrPatchingMode;
  UCHAR sbrOversamplingFlag;
  UCHAR sbrPitchInBins;
} SBR_FRAME_DATA;

typedef SBR_FRAME_DATA *HANDLE_SBR_FRAME_DATA;

/*!
\brief   Maps sampling frequencies to frequencies for which setup tables are
available

Maps arbitary sampling frequency to nearest neighbors for which setup tables
are available (e.g. 25600 -> 24000).
Used for startFreq calculation.
The mapping is defined in 14496-3 (4.6.18.2.6), fs(SBR), and table 4.82

\return  mapped sampling frequency
*/
UINT sbrdec_mapToStdSampleRate(UINT fs,
                               UINT isUsac); /*!< Output sampling frequency */

void initSbrPrevFrameData(HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                          int timeSlots);

int sbrGetChannelElement(HANDLE_SBR_HEADER_DATA hHeaderData,
                         HANDLE_SBR_FRAME_DATA hFrameDataLeft,
                         HANDLE_SBR_FRAME_DATA hFrameDataRight,
                         HANDLE_SBR_PREV_FRAME_DATA hFrameDataLeftPrev,
                         UCHAR pvc_mode_last, HANDLE_FDK_BITSTREAM hBitBuf,
                         HANDLE_PS_DEC hParametricStereoDec, const UINT flags,
                         const int overlap);

SBR_HEADER_STATUS
sbrGetHeaderData(HANDLE_SBR_HEADER_DATA headerData,
                 HANDLE_FDK_BITSTREAM hBitBuf, const UINT flags,
                 const int fIsSbrData, const UCHAR configMode);

/*!
  \brief     Initialize SBR header data

  Copy default values to the header data struct and patch some entries
  depending on the core codec.
*/
SBR_ERROR
initHeaderData(HANDLE_SBR_HEADER_DATA hHeaderData, const int sampleRateIn,
               const int sampleRateOut, const INT downscaleFactor,
               const int samplesPerFrame, const UINT flags,
               const int setDefaultHdr);
#endif

/* Convert headroom bits to exponent */
#define SCALE2EXP(s) (15 - (s))
#define EXP2SCALE(e) (15 - (e))
