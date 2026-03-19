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

/**************************** AAC decoder library ******************************

   Author(s):   Josef Hoepfl

   Description: Definition of constant tables

*******************************************************************************/

#ifndef AAC_ROM_H
#define AAC_ROM_H

#include "common_fix.h"
#include "FDK_audio.h"
#include "aacdec_hcr_types.h"
#include "aacdec_hcrs.h"

#define PCM_AAC LONG
#define PCM_DEC FIXP_DBL
#define MAXVAL_PCM_DEC MAXVAL_DBL
#define MINVAL_PCM_DEC MINVAL_DBL
#define FIXP_DBL2PCM_DEC(x) (x)
#define PCM_DEC2FIXP_DBL(x) (x)
#define PCM_DEC_BITS DFRACT_BITS
#define PCM_DEC2FX_PCM(x) FX_DBL2FX_PCM(x)
#define FX_PCM2PCM_DEC(x) FX_PCM2FX_DBL(x)

#define AACDEC_MAX_CH_CONF 14
#define AACDEC_CH_ELEMENTS_TAB_SIZE 7 /*!< Size of element tables */

#define AAC_NF_NO_RANDOM_VAL \
  512 /*!< Size of random number array for noise floor */

#define INV_QUANT_TABLESIZE 256

extern const FIXP_DBL InverseQuantTable[INV_QUANT_TABLESIZE + 1];
extern const FIXP_DBL MantissaTable[4][14];
extern const SCHAR ExponentTable[4][14];

#define NUM_LD_COEF_512 1536
#define NUM_LD_COEF_480 1440
/* Window table partition exponents. */
#define WTS0 (1)
#define WTS1 (0)
#define WTS2 (-2)
extern const FIXP_WTB LowDelaySynthesis512[1536];
extern const FIXP_WTB LowDelaySynthesis480[1440];
extern const FIXP_WTB LowDelaySynthesis256[768];
extern const FIXP_WTB LowDelaySynthesis240[720];
extern const FIXP_WTB LowDelaySynthesis160[480];
extern const FIXP_WTB LowDelaySynthesis128[384];
extern const FIXP_WTB LowDelaySynthesis120[360];

typedef struct {
  const SHORT *sfbOffsetLong;
  const SHORT *sfbOffsetShort;
  UCHAR numberOfSfbLong;
  UCHAR numberOfSfbShort;
} SFB_INFO;

extern const SFB_INFO sfbOffsetTables[5][16];

/* Huffman tables */
enum { HuffmanBits = 2, HuffmanEntries = (1 << HuffmanBits) };

typedef struct {
  const USHORT (*CodeBook)[HuffmanEntries];
  UCHAR Dimension;
  UCHAR numBits;
  UCHAR Offset;
} CodeBookDescription;

extern const CodeBookDescription AACcodeBookDescriptionTable[13];
extern const CodeBookDescription AACcodeBookDescriptionSCL;

extern const STATEFUNC aStateConstant2State[];

extern const SCHAR aCodebook2StartInt[];

extern const UCHAR aMinOfCbPair[];
extern const UCHAR aMaxOfCbPair[];

extern const UCHAR aMaxCwLen[];
extern const UCHAR aDimCb[];
extern const UCHAR aDimCbShift[];
extern const UCHAR aSignCb[];
extern const UCHAR aCbPriority[];

extern const UINT *aHuffTable[];
extern const SCHAR *aQuantTable[];

extern const USHORT aLargestAbsoluteValue[];

extern const UINT aHuffTreeRvlcEscape[];
extern const UINT aHuffTreeRvlCodewds[];

extern const UCHAR tns_max_bands_tbl[13][2];

extern const UCHAR tns_max_bands_tbl_480[13];
extern const UCHAR tns_max_bands_tbl_512[13];

#define FIXP_TCC FIXP_DBL

extern const FIXP_TCC FDKaacDec_tnsCoeff3[8];
extern const FIXP_TCC FDKaacDec_tnsCoeff4[16];

extern const UCHAR FDKaacDec_tnsCoeff3_gain_ld[];
extern const UCHAR FDKaacDec_tnsCoeff4_gain_ld[];

extern const USHORT AacDec_randomSign[AAC_NF_NO_RANDOM_VAL / 16];

extern const FIXP_DBL pow2_div24minus1[47];
extern const int offsetTab[2][16];

/* Channel mapping indices for time domain I/O.
   The first dimension is the channel configuration index. */
extern const UCHAR channelMappingTablePassthrough[15][8];
extern const UCHAR channelMappingTableWAV[15][8];

/* Lookup tables for elements in ER bitstream */
extern const MP4_ELEMENT_ID elementsTab[AACDEC_MAX_CH_CONF]
                                       [AACDEC_CH_ELEMENTS_TAB_SIZE];

#define SF_FNA_COEFFS \
  1 /* Compile-time prescaler for MDST-filter coefficients. */
/* SF_FNA_COEFFS > 0 should only be considered for FIXP_DBL-coefficients  */
/* (i.e. if CPLX_PRED_FILTER_16BIT is not defined).                       */
/* With FIXP_DBL loss of precision is possible for SF_FNA_COEFFS > 11.    */

#ifdef CPLX_PRED_FILTER_16BIT
#define FIXP_FILT FIXP_SGL
#define FILT(a) ((FL2FXCONST_SGL(a)) >> SF_FNA_COEFFS)
#else
#define FIXP_FILT FIXP_DBL
#define FILT(a) ((FL2FXCONST_DBL(a)) >> SF_FNA_COEFFS)
#endif

extern const FIXP_FILT mdst_filt_coef_curr[20][3]; /* MDST-filter coefficient
                                                      tables used for current
                                                      window  */
extern const FIXP_FILT mdst_filt_coef_prev[6][4];  /* MDST-filter coefficient
                                                      tables used for previous
                                                      window */

#endif /* #ifndef AAC_ROM_H */
