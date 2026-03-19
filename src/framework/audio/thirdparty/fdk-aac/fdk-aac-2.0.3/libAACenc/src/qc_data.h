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

   Description: Quantizing & coding data

*******************************************************************************/

#ifndef QC_DATA_H
#define QC_DATA_H

#include "aacenc.h"
#include "psy_const.h"
#include "dyn_bits.h"
#include "adj_thr_data.h"
#include "line_pe.h"
#include "FDK_audio.h"
#include "interface.h"

typedef enum {
  QCDATA_BR_MODE_INVALID = -1,
  QCDATA_BR_MODE_CBR = 0,   /* Constant bit rate, given average bitrate */
  QCDATA_BR_MODE_VBR_1 = 1, /* Variable bit rate, very low */
  QCDATA_BR_MODE_VBR_2 = 2, /* Variable bit rate, low */
  QCDATA_BR_MODE_VBR_3 = 3, /* Variable bit rate, medium */
  QCDATA_BR_MODE_VBR_4 = 4, /* Variable bit rate, high */
  QCDATA_BR_MODE_VBR_5 = 5, /* Variable bit rate, very high */
  QCDATA_BR_MODE_FF = 6,    /* Fixed frame mode. */
  QCDATA_BR_MODE_SFR = 7    /* Superframe mode. */

} QCDATA_BR_MODE;

typedef struct {
  MP4_ELEMENT_ID elType;
  INT instanceTag;
  INT nChannelsInEl;
  INT ChannelIndex[2];
  FIXP_DBL relativeBits;
} ELEMENT_INFO;

typedef struct {
  CHANNEL_MODE encMode;
  INT nChannels;
  INT nChannelsEff;
  INT nElements;
  ELEMENT_INFO elInfo[((8))];
} CHANNEL_MAPPING;

typedef struct {
  INT paddingRest;
} PADDING;

/* Quantizing & coding stage */

struct QC_INIT {
  CHANNEL_MAPPING *channelMapping;
  INT sceCpe;      /* not used yet                         */
  INT maxBits;     /* maximum number of bits in reservoir  */
  INT averageBits; /* average number of bits we should use */
  INT bitRes;
  INT sampleRate; /* output sample rate                   */
  INT isLowDelay; /* if set, calc bits2PE factor depending on samplerate */
  INT staticBits; /* Bits per frame consumed by transport layers. */
  QCDATA_BR_MODE bitrateMode;
  INT meanPe;
  INT chBitrate; /* Bitrate/channel */
  INT invQuant;
  INT maxIterations; /* Maximum number of allowed iterations before
                        FDKaacEnc_crashRecovery() is applied. */
  FIXP_DBL maxBitFac;
  INT bitrate;
  INT nSubFrames;                /* helper variable */
  INT minBits;                   /* minimal number of bits in one frame*/
  AACENC_BITRES_MODE bitResMode; /* 0: full bitreservoir, 1: reduced
                                    bitreservoir, 2: disabled bitreservoir */
  INT bitDistributionMode; /* Configure element-wise execution or execution over
                              all elements for the pe-dependent
                              threshold-adaption */

  PADDING padding;
};

typedef struct {
  FIXP_DBL mdctSpectrum[(1024)];

  SHORT quantSpec[(1024)];

  UINT maxValueInSfb[MAX_GROUPED_SFB];
  INT scf[MAX_GROUPED_SFB];
  INT globalGain;
  SECTION_DATA sectionData;

  FIXP_DBL sfbFormFactorLdData[MAX_GROUPED_SFB];

  FIXP_DBL sfbThresholdLdData[MAX_GROUPED_SFB];
  FIXP_DBL sfbMinSnrLdData[MAX_GROUPED_SFB];
  FIXP_DBL sfbEnergyLdData[MAX_GROUPED_SFB];
  FIXP_DBL sfbEnergy[MAX_GROUPED_SFB];
  FIXP_DBL sfbWeightedEnergyLdData[MAX_GROUPED_SFB];

  FIXP_DBL sfbEnFacLd[MAX_GROUPED_SFB];

  FIXP_DBL sfbSpreadEnergy[MAX_GROUPED_SFB];

} QC_OUT_CHANNEL;

typedef struct {
  EXT_PAYLOAD_TYPE type; /* type of the extension payload */
  INT nPayloadBits;      /* size of the payload */
  UCHAR *pPayload;       /* pointer to payload */

} QC_OUT_EXTENSION;

typedef struct {
  INT staticBitsUsed; /* for verification purposes */
  INT dynBitsUsed;    /* for verification purposes */

  INT extBitsUsed; /* bit consumption of extended fill elements */
  INT nExtensions; /* number of extension payloads for this element */
  QC_OUT_EXTENSION extension[(1)]; /* reffering extension payload */

  INT grantedDynBits;

  INT grantedPe;
  INT grantedPeCorr;

  PE_DATA peData;

  QC_OUT_CHANNEL *qcOutChannel[(2)];

  UCHAR
  *dynMem_Ah_Flag; /* pointer to dynamic buffer used by AhFlag in function
                      FDKaacEnc_adaptThresholdsToPe() */
  UCHAR
  *dynMem_Thr_Exp; /* pointer to dynamic buffer used by ThrExp in function
                      FDKaacEnc_adaptThresholdsToPe() */
  UCHAR *dynMem_SfbNActiveLinesLdData; /* pointer to dynamic buffer used by
                                          sfbNActiveLinesLdData in function
                                          FDKaacEnc_correctThresh() */

} QC_OUT_ELEMENT;

typedef struct {
  QC_OUT_ELEMENT *qcElement[((8))];
  QC_OUT_CHANNEL *pQcOutChannels[(8)];
  QC_OUT_EXTENSION extension[(2 + 2)]; /* global extension payload */
  INT nExtensions;    /* number of extension payloads for this AU */
  INT maxDynBits;     /* maximal allowed dynamic bits in frame */
  INT grantedDynBits; /* granted dynamic bits in frame */
  INT totFillBits;    /* fill bits */
  INT elementExtBits; /* element associated extension payload bits, e.g. sbr,
                         drc ... */
  INT globalExtBits;  /* frame/au associated extension payload bits (anc data
                         ...) */
  INT staticBits;     /* aac side info bits */

  INT totalNoRedPe;
  INT totalGrantedPeCorr;

  INT usedDynBits; /* number of dynamic bits in use */
  INT alignBits;   /* AU alignment bits */
  INT totalBits;   /* sum of static, dyn, sbr, fill, align and dse bits */

} QC_OUT;

typedef struct {
  INT chBitrateEl;         /* channel bitrate in element
                              (totalbitrate*el_relativeBits/el_channels) */
  INT maxBitsEl;           /* used in crash recovery */
  INT bitResLevelEl;       /* update bitreservoir level in each call of
                              FDKaacEnc_QCMain */
  INT maxBitResBitsEl;     /* nEffChannels*6144 - averageBitsInFrame */
  FIXP_DBL relativeBitsEl; /* Bits relative to total Bits*/
} ELEMENT_BITS;

typedef struct {
  /* this is basically struct QC_INIT */

  INT globHdrBits;
  INT maxBitsPerFrame; /* maximal allowed bits per frame, 6144*nChannelsEff */
  INT minBitsPerFrame; /* minimal allowd bits per fram, superframing - DRM */
  INT nElements;
  QCDATA_BR_MODE bitrateMode;
  AACENC_BITRES_MODE bitResMode; /* 0: full bitreservoir, 1: reduced
                                    bitreservoir, 2: disabled bitreservoir */
  INT bitResTot;
  INT bitResTotMax;
  INT maxIterations; /* Maximum number of allowed iterations before
                        FDKaacEnc_crashRecovery() is applied. */
  INT invQuant;

  FIXP_DBL vbrQualFactor;
  FIXP_DBL maxBitFac;

  PADDING padding;

  ELEMENT_BITS *elementBits[((8))];
  BITCNTR_STATE *hBitCounter;
  ADJ_THR_STATE *hAdjThr;

  INT dZoneQuantEnable; /* enable dead zone quantizer */

} QC_STATE;

#endif /* QC_DATA_H */
