/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2020 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

   Author(s):   Markus Lohwasser

   Description: FDK Tools Hybrid Filterbank

*******************************************************************************/

#include "FDK_hybrid.h"

#include "fft.h"

/*--------------- defines -----------------------------*/
#define FFT_IDX_R(a) (2 * a)
#define FFT_IDX_I(a) (2 * a + 1)

#define HYB_COEF8_0 (0.00746082949812f)
#define HYB_COEF8_1 (0.02270420949825f)
#define HYB_COEF8_2 (0.04546865930473f)
#define HYB_COEF8_3 (0.07266113929591f)
#define HYB_COEF8_4 (0.09885108575264f)
#define HYB_COEF8_5 (0.11793710567217f)
#define HYB_COEF8_6 (0.12500000000000f)
#define HYB_COEF8_7 (HYB_COEF8_5)
#define HYB_COEF8_8 (HYB_COEF8_4)
#define HYB_COEF8_9 (HYB_COEF8_3)
#define HYB_COEF8_10 (HYB_COEF8_2)
#define HYB_COEF8_11 (HYB_COEF8_1)
#define HYB_COEF8_12 (HYB_COEF8_0)

/*--------------- structure definitions ---------------*/

#if defined(ARCH_PREFER_MULT_32x16)
#define FIXP_HTB FIXP_SGL              /* SGL data type. */
#define FIXP_HTP FIXP_SPK              /* Packed SGL data type. */
#define HTC(a) (FX_DBL2FXCONST_SGL(a)) /* Cast to SGL */
#define FL2FXCONST_HTB FL2FXCONST_SGL
#else
#define FIXP_HTB FIXP_DBL            /* SGL data type. */
#define FIXP_HTP FIXP_DPK            /* Packed DBL data type. */
#define HTC(a) ((FIXP_DBL)(LONG)(a)) /* Cast to DBL */
#define FL2FXCONST_HTB FL2FXCONST_DBL
#endif

#define HTCP(real, imag)     \
  {                          \
    { HTC(real), HTC(imag) } \
  } /* How to arrange the packed values. */

struct FDK_HYBRID_SETUP {
  UCHAR nrQmfBands;     /*!< Number of QMF bands to be converted to hybrid. */
  UCHAR nHybBands[3];   /*!< Number of Hybrid bands generated by nrQmfBands. */
  UCHAR synHybScale[3]; /*!< Headroom needed in hybrid synthesis filterbank. */
  SCHAR kHybrid[3];     /*!< Filter configuration of each QMF band. */
  UCHAR protoLen;       /*!< Prototype filter length. */
  UCHAR filterDelay;    /*!< Delay caused by hybrid filter. */
  const INT
      *pReadIdxTable; /*!< Helper table to access input data ringbuffer. */
};

/*--------------- constants ---------------------------*/
static const INT ringbuffIdxTab[2 * 13] = {0, 1,  2,  3,  4, 5,  6,  7, 8,
                                           9, 10, 11, 12, 0, 1,  2,  3, 4,
                                           5, 6,  7,  8,  9, 10, 11, 12};

static const FDK_HYBRID_SETUP setup_3_16 = {
    3, {8, 4, 4}, {4, 3, 3}, {8, 4, 4}, 13, (13 - 1) / 2, ringbuffIdxTab};
static const FDK_HYBRID_SETUP setup_3_12 = {
    3, {8, 2, 2}, {4, 2, 2}, {8, 2, 2}, 13, (13 - 1) / 2, ringbuffIdxTab};
static const FDK_HYBRID_SETUP setup_3_10 = {
    3, {6, 2, 2}, {3, 2, 2}, {-8, -2, 2}, 13, (13 - 1) / 2, ringbuffIdxTab};

static const FIXP_HTP HybFilterCoef8[] = {
    HTCP(0x10000000, 0x00000000), HTCP(0x0df26407, 0xfa391882),
    HTCP(0xff532109, 0x00acdef7), HTCP(0x08f26d36, 0xf70d92ca),
    HTCP(0xfee34b5f, 0x02af570f), HTCP(0x038f276e, 0xf7684793),
    HTCP(0x00000000, 0x05d1eac2), HTCP(0x00000000, 0x05d1eac2),
    HTCP(0x038f276e, 0x0897b86d), HTCP(0xfee34b5f, 0xfd50a8f1),
    HTCP(0x08f26d36, 0x08f26d36), HTCP(0xff532109, 0xff532109),
    HTCP(0x0df26407, 0x05c6e77e)};

static const FIXP_HTB HybFilterCoef2[3] = {FL2FXCONST_HTB(0.01899487526049f),
                                           FL2FXCONST_HTB(-0.07293139167538f),
                                           FL2FXCONST_HTB(0.30596630545168f)};

static const FIXP_HTB HybFilterCoef4[13] = {FL2FXCONST_HTB(-0.00305151927305f),
                                            FL2FXCONST_HTB(-0.00794862316203f),
                                            FL2FXCONST_HTB(0.0f),
                                            FL2FXCONST_HTB(0.04318924038756f),
                                            FL2FXCONST_HTB(0.12542448210445f),
                                            FL2FXCONST_HTB(0.21227807049160f),
                                            FL2FXCONST_HTB(0.25f),
                                            FL2FXCONST_HTB(0.21227807049160f),
                                            FL2FXCONST_HTB(0.12542448210445f),
                                            FL2FXCONST_HTB(0.04318924038756f),
                                            FL2FXCONST_HTB(0.0f),
                                            FL2FXCONST_HTB(-0.00794862316203f),
                                            FL2FXCONST_HTB(-0.00305151927305f)};

/*--------------- function declarations ---------------*/
static INT kChannelFiltering(const FIXP_DBL *const pQmfReal,
                             const FIXP_DBL *const pQmfImag,
                             const INT *const pReadIdx,
                             FIXP_DBL *const mHybridReal,
                             FIXP_DBL *const mHybridImag,
                             const SCHAR hybridConfig);

/*--------------- function definitions ----------------*/

INT FDKhybridAnalysisOpen(HANDLE_FDK_ANA_HYB_FILTER hAnalysisHybFilter,
                          FIXP_DBL *const pLFmemory, const UINT LFmemorySize,
                          FIXP_DBL *const pHFmemory, const UINT HFmemorySize) {
  INT err = 0;

  /* Save pointer to extern memory. */
  hAnalysisHybFilter->pLFmemory = pLFmemory;
  hAnalysisHybFilter->LFmemorySize = LFmemorySize;

  hAnalysisHybFilter->pHFmemory = pHFmemory;
  hAnalysisHybFilter->HFmemorySize = HFmemorySize;

  return err;
}

INT FDKhybridAnalysisInit(HANDLE_FDK_ANA_HYB_FILTER hAnalysisHybFilter,
                          const FDK_HYBRID_MODE mode, const INT qmfBands,
                          const INT cplxBands, const INT initStatesFlag) {
  int k;
  INT err = 0;
  FIXP_DBL *pMem = NULL;
  HANDLE_FDK_HYBRID_SETUP setup = NULL;

  switch (mode) {
    case THREE_TO_TEN:
      setup = &setup_3_10;
      break;
    case THREE_TO_TWELVE:
      setup = &setup_3_12;
      break;
    case THREE_TO_SIXTEEN:
      setup = &setup_3_16;
      break;
    default:
      err = -1;
      goto bail;
  }

  /* Initialize handle. */
  hAnalysisHybFilter->pSetup = setup;
  if (initStatesFlag) {
    hAnalysisHybFilter->bufferLFpos = setup->protoLen - 1;
    hAnalysisHybFilter->bufferHFpos = 0;
  }
  hAnalysisHybFilter->nrBands = qmfBands;
  hAnalysisHybFilter->cplxBands = cplxBands;
  hAnalysisHybFilter->hfMode = 0;

  /* Check available memory. */
  if (((2 * setup->nrQmfBands * setup->protoLen * sizeof(FIXP_DBL)) >
       hAnalysisHybFilter->LFmemorySize)) {
    err = -2;
    goto bail;
  }
  if (hAnalysisHybFilter->HFmemorySize != 0) {
    if (((setup->filterDelay *
          ((qmfBands - setup->nrQmfBands) + (cplxBands - setup->nrQmfBands)) *
          sizeof(FIXP_DBL)) > hAnalysisHybFilter->HFmemorySize)) {
      err = -3;
      goto bail;
    }
  }

  /* Distribute LF memory. */
  pMem = hAnalysisHybFilter->pLFmemory;
  for (k = 0; k < setup->nrQmfBands; k++) {
    hAnalysisHybFilter->bufferLFReal[k] = pMem;
    pMem += setup->protoLen;
    hAnalysisHybFilter->bufferLFImag[k] = pMem;
    pMem += setup->protoLen;
  }

  /* Distribute HF memory. */
  if (hAnalysisHybFilter->HFmemorySize != 0) {
    pMem = hAnalysisHybFilter->pHFmemory;
    for (k = 0; k < setup->filterDelay; k++) {
      hAnalysisHybFilter->bufferHFReal[k] = pMem;
      pMem += (qmfBands - setup->nrQmfBands);
      hAnalysisHybFilter->bufferHFImag[k] = pMem;
      pMem += (cplxBands - setup->nrQmfBands);
    }
  }

  if (initStatesFlag) {
    /* Clear LF buffer */
    for (k = 0; k < setup->nrQmfBands; k++) {
      FDKmemclear(hAnalysisHybFilter->bufferLFReal[k],
                  setup->protoLen * sizeof(FIXP_DBL));
      FDKmemclear(hAnalysisHybFilter->bufferLFImag[k],
                  setup->protoLen * sizeof(FIXP_DBL));
    }

    if (hAnalysisHybFilter->HFmemorySize != 0) {
      if (qmfBands > setup->nrQmfBands) {
        /* Clear HF buffer */
        for (k = 0; k < setup->filterDelay; k++) {
          FDKmemclear(hAnalysisHybFilter->bufferHFReal[k],
                      (qmfBands - setup->nrQmfBands) * sizeof(FIXP_DBL));
          FDKmemclear(hAnalysisHybFilter->bufferHFImag[k],
                      (cplxBands - setup->nrQmfBands) * sizeof(FIXP_DBL));
        }
      }
    }
  }

bail:
  return err;
}

INT FDKhybridAnalysisScaleStates(HANDLE_FDK_ANA_HYB_FILTER hAnalysisHybFilter,
                                 const INT scalingValue) {
  INT err = 0;

  if (hAnalysisHybFilter == NULL) {
    err = 1; /* invalid handle */
  } else {
    int k;
    HANDLE_FDK_HYBRID_SETUP setup = hAnalysisHybFilter->pSetup;

    /* Scale LF buffer */
    for (k = 0; k < setup->nrQmfBands; k++) {
      scaleValues(hAnalysisHybFilter->bufferLFReal[k], setup->protoLen,
                  scalingValue);
      scaleValues(hAnalysisHybFilter->bufferLFImag[k], setup->protoLen,
                  scalingValue);
    }
    if (hAnalysisHybFilter->nrBands > setup->nrQmfBands) {
      /* Scale HF buffer */
      for (k = 0; k < setup->filterDelay; k++) {
        scaleValues(hAnalysisHybFilter->bufferHFReal[k],
                    (hAnalysisHybFilter->nrBands - setup->nrQmfBands),
                    scalingValue);
        scaleValues(hAnalysisHybFilter->bufferHFImag[k],
                    (hAnalysisHybFilter->cplxBands - setup->nrQmfBands),
                    scalingValue);
      }
    }
  }
  return err;
}

INT FDKhybridAnalysisApply(HANDLE_FDK_ANA_HYB_FILTER hAnalysisHybFilter,
                           const FIXP_DBL *const pQmfReal,
                           const FIXP_DBL *const pQmfImag,
                           FIXP_DBL *const pHybridReal,
                           FIXP_DBL *const pHybridImag) {
  int k, hybOffset = 0;
  INT err = 0;
  const int nrQmfBandsLF =
      hAnalysisHybFilter->pSetup
          ->nrQmfBands; /* number of QMF bands to be converted to hybrid */

  const int writIndex = hAnalysisHybFilter->bufferLFpos;
  int readIndex = hAnalysisHybFilter->bufferLFpos;

  if (++readIndex >= hAnalysisHybFilter->pSetup->protoLen) readIndex = 0;
  const INT *pBufferLFreadIdx =
      &hAnalysisHybFilter->pSetup->pReadIdxTable[readIndex];

  /*
   * LF buffer.
   */
  for (k = 0; k < nrQmfBandsLF; k++) {
    /* New input sample. */
    hAnalysisHybFilter->bufferLFReal[k][writIndex] = pQmfReal[k];
    hAnalysisHybFilter->bufferLFImag[k][writIndex] = pQmfImag[k];

    /* Perform hybrid filtering. */
    err |=
        kChannelFiltering(hAnalysisHybFilter->bufferLFReal[k],
                          hAnalysisHybFilter->bufferLFImag[k], pBufferLFreadIdx,
                          pHybridReal + hybOffset, pHybridImag + hybOffset,
                          hAnalysisHybFilter->pSetup->kHybrid[k]);

    hybOffset += hAnalysisHybFilter->pSetup->nHybBands[k];
  }

  hAnalysisHybFilter->bufferLFpos =
      readIndex; /* Index where to write next input sample. */

  if (hAnalysisHybFilter->nrBands > nrQmfBandsLF) {
    /*
     * HF buffer.
     */
    if (hAnalysisHybFilter->hfMode != 0) {
      /* HF delay compensation was applied outside. */
      FDKmemcpy(
          pHybridReal + hybOffset, &pQmfReal[nrQmfBandsLF],
          (hAnalysisHybFilter->nrBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
      FDKmemcpy(
          pHybridImag + hybOffset, &pQmfImag[nrQmfBandsLF],
          (hAnalysisHybFilter->cplxBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
    } else {
      FDK_ASSERT(hAnalysisHybFilter->HFmemorySize != 0);
      /* HF delay compensation, filterlength/2. */
      FDKmemcpy(
          pHybridReal + hybOffset,
          hAnalysisHybFilter->bufferHFReal[hAnalysisHybFilter->bufferHFpos],
          (hAnalysisHybFilter->nrBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
      FDKmemcpy(
          pHybridImag + hybOffset,
          hAnalysisHybFilter->bufferHFImag[hAnalysisHybFilter->bufferHFpos],
          (hAnalysisHybFilter->cplxBands - nrQmfBandsLF) * sizeof(FIXP_DBL));

      FDKmemcpy(
          hAnalysisHybFilter->bufferHFReal[hAnalysisHybFilter->bufferHFpos],
          &pQmfReal[nrQmfBandsLF],
          (hAnalysisHybFilter->nrBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
      FDKmemcpy(
          hAnalysisHybFilter->bufferHFImag[hAnalysisHybFilter->bufferHFpos],
          &pQmfImag[nrQmfBandsLF],
          (hAnalysisHybFilter->cplxBands - nrQmfBandsLF) * sizeof(FIXP_DBL));

      if (++hAnalysisHybFilter->bufferHFpos >=
          hAnalysisHybFilter->pSetup->filterDelay)
        hAnalysisHybFilter->bufferHFpos = 0;
    }
  } /* process HF part*/

  return err;
}

INT FDKhybridAnalysisClose(HANDLE_FDK_ANA_HYB_FILTER hAnalysisHybFilter) {
  INT err = 0;

  if (hAnalysisHybFilter != NULL) {
    hAnalysisHybFilter->pLFmemory = NULL;
    hAnalysisHybFilter->pHFmemory = NULL;
    hAnalysisHybFilter->LFmemorySize = 0;
    hAnalysisHybFilter->HFmemorySize = 0;
  }

  return err;
}

INT FDKhybridSynthesisInit(HANDLE_FDK_SYN_HYB_FILTER hSynthesisHybFilter,
                           const FDK_HYBRID_MODE mode, const INT qmfBands,
                           const INT cplxBands) {
  INT err = 0;
  HANDLE_FDK_HYBRID_SETUP setup = NULL;

  switch (mode) {
    case THREE_TO_TEN:
      setup = &setup_3_10;
      break;
    case THREE_TO_TWELVE:
      setup = &setup_3_12;
      break;
    case THREE_TO_SIXTEEN:
      setup = &setup_3_16;
      break;
    default:
      err = -1;
      goto bail;
  }

  hSynthesisHybFilter->pSetup = setup;
  hSynthesisHybFilter->nrBands = qmfBands;
  hSynthesisHybFilter->cplxBands = cplxBands;

bail:
  return err;
}

void FDKhybridSynthesisApply(HANDLE_FDK_SYN_HYB_FILTER hSynthesisHybFilter,
                             const FIXP_DBL *const pHybridReal,
                             const FIXP_DBL *const pHybridImag,
                             FIXP_DBL *const pQmfReal,
                             FIXP_DBL *const pQmfImag) {
  int k, n, hybOffset = 0;
  const INT nrQmfBandsLF = hSynthesisHybFilter->pSetup->nrQmfBands;

  /*
   * LF buffer.
   */
  for (k = 0; k < nrQmfBandsLF; k++) {
    const int nHybBands = hSynthesisHybFilter->pSetup->nHybBands[k];
    const int scale = hSynthesisHybFilter->pSetup->synHybScale[k];

    FIXP_DBL accu1 = FL2FXCONST_DBL(0.f);
    FIXP_DBL accu2 = FL2FXCONST_DBL(0.f);

    /* Perform hybrid filtering. */
    for (n = 0; n < nHybBands; n++) {
      accu1 += pHybridReal[hybOffset + n] >> scale;
      accu2 += pHybridImag[hybOffset + n] >> scale;
    }
    pQmfReal[k] = SATURATE_LEFT_SHIFT(accu1, scale, DFRACT_BITS);
    pQmfImag[k] = SATURATE_LEFT_SHIFT(accu2, scale, DFRACT_BITS);

    hybOffset += nHybBands;
  }

  if (hSynthesisHybFilter->nrBands > nrQmfBandsLF) {
    /*
     * HF buffer.
     */
    FDKmemcpy(&pQmfReal[nrQmfBandsLF], &pHybridReal[hybOffset],
              (hSynthesisHybFilter->nrBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
    FDKmemcpy(
        &pQmfImag[nrQmfBandsLF], &pHybridImag[hybOffset],
        (hSynthesisHybFilter->cplxBands - nrQmfBandsLF) * sizeof(FIXP_DBL));
  }

  return;
}

static void dualChannelFiltering(const FIXP_DBL *const pQmfReal,
                                 const FIXP_DBL *const pQmfImag,
                                 const INT *const pReadIdx,
                                 FIXP_DBL *const mHybridReal,
                                 FIXP_DBL *const mHybridImag,
                                 const INT invert) {
  FIXP_DBL r1, r6;
  FIXP_DBL i1, i6;

  const FIXP_HTB f0 = HybFilterCoef2[0]; /* corresponds to p1 and p11 */
  const FIXP_HTB f1 = HybFilterCoef2[1]; /* corresponds to p3 and p9  */
  const FIXP_HTB f2 = HybFilterCoef2[2]; /* corresponds to p5 and p7  */

  /* symmetric filter coefficients */
  r1 = fMultDiv2(f0, pQmfReal[pReadIdx[1]]) +
       fMultDiv2(f0, pQmfReal[pReadIdx[11]]);
  i1 = fMultDiv2(f0, pQmfImag[pReadIdx[1]]) +
       fMultDiv2(f0, pQmfImag[pReadIdx[11]]);
  r1 += fMultDiv2(f1, pQmfReal[pReadIdx[3]]) +
        fMultDiv2(f1, pQmfReal[pReadIdx[9]]);
  i1 += fMultDiv2(f1, pQmfImag[pReadIdx[3]]) +
        fMultDiv2(f1, pQmfImag[pReadIdx[9]]);
  r1 += fMultDiv2(f2, pQmfReal[pReadIdx[5]]) +
        fMultDiv2(f2, pQmfReal[pReadIdx[7]]);
  i1 += fMultDiv2(f2, pQmfImag[pReadIdx[5]]) +
        fMultDiv2(f2, pQmfImag[pReadIdx[7]]);

  r6 = pQmfReal[pReadIdx[6]] >> 2;
  i6 = pQmfImag[pReadIdx[6]] >> 2;

  FDK_ASSERT((invert == 0) || (invert == 1));
  mHybridReal[0 + invert] = SATURATE_LEFT_SHIFT((r6 + r1), 1, DFRACT_BITS);
  mHybridImag[0 + invert] = SATURATE_LEFT_SHIFT((i6 + i1), 1, DFRACT_BITS);

  mHybridReal[1 - invert] = SATURATE_LEFT_SHIFT((r6 - r1), 1, DFRACT_BITS);
  mHybridImag[1 - invert] = SATURATE_LEFT_SHIFT((i6 - i1), 1, DFRACT_BITS);
}

static void fourChannelFiltering(const FIXP_DBL *const pQmfReal,
                                 const FIXP_DBL *const pQmfImag,
                                 const INT *const pReadIdx,
                                 FIXP_DBL *const mHybridReal,
                                 FIXP_DBL *const mHybridImag,
                                 const INT invert) {
  const FIXP_HTB *p = HybFilterCoef4;

  FIXP_DBL fft[8];

  static const FIXP_DBL cr[13] = {
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(-1.f), FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(0.70710678118655f),
      FL2FXCONST_DBL(1.f),  FL2FXCONST_DBL(0.70710678118655f),
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(-1.f), FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(0.f)};
  static const FIXP_DBL ci[13] = {
      FL2FXCONST_DBL(-1.f), FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(0.70710678118655f),
      FL2FXCONST_DBL(1.f),  FL2FXCONST_DBL(0.70710678118655f),
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(-1.f), FL2FXCONST_DBL(-0.70710678118655f),
      FL2FXCONST_DBL(0.f),  FL2FXCONST_DBL(0.70710678118655f),
      FL2FXCONST_DBL(1.f)};

  /* FIR filter. */
  /* pre twiddeling with pre-twiddling coefficients c[n]  */
  /* multiplication with filter coefficients p[n]         */
  /* hint: (a + ib)*(c + id) = (a*c - b*d) + i(a*d + b*c) */
  /* write to fft coefficient n'                          */
  fft[FFT_IDX_R(0)] =
      (fMult(p[10], (fMultSub(fMultDiv2(cr[2], pQmfReal[pReadIdx[2]]), ci[2],
                              pQmfImag[pReadIdx[2]]))) +
       fMult(p[6], (fMultSub(fMultDiv2(cr[6], pQmfReal[pReadIdx[6]]), ci[6],
                             pQmfImag[pReadIdx[6]]))) +
       fMult(p[2], (fMultSub(fMultDiv2(cr[10], pQmfReal[pReadIdx[10]]), ci[10],
                             pQmfImag[pReadIdx[10]]))));
  fft[FFT_IDX_I(0)] =
      (fMult(p[10], (fMultAdd(fMultDiv2(ci[2], pQmfReal[pReadIdx[2]]), cr[2],
                              pQmfImag[pReadIdx[2]]))) +
       fMult(p[6], (fMultAdd(fMultDiv2(ci[6], pQmfReal[pReadIdx[6]]), cr[6],
                             pQmfImag[pReadIdx[6]]))) +
       fMult(p[2], (fMultAdd(fMultDiv2(ci[10], pQmfReal[pReadIdx[10]]), cr[10],
                             pQmfImag[pReadIdx[10]]))));

  /* twiddle dee dum */
  fft[FFT_IDX_R(1)] =
      (fMult(p[9], (fMultSub(fMultDiv2(cr[3], pQmfReal[pReadIdx[3]]), ci[3],
                             pQmfImag[pReadIdx[3]]))) +
       fMult(p[5], (fMultSub(fMultDiv2(cr[7], pQmfReal[pReadIdx[7]]), ci[7],
                             pQmfImag[pReadIdx[7]]))) +
       fMult(p[1], (fMultSub(fMultDiv2(cr[11], pQmfReal[pReadIdx[11]]), ci[11],
                             pQmfImag[pReadIdx[11]]))));
  fft[FFT_IDX_I(1)] =
      (fMult(p[9], (fMultAdd(fMultDiv2(ci[3], pQmfReal[pReadIdx[3]]), cr[3],
                             pQmfImag[pReadIdx[3]]))) +
       fMult(p[5], (fMultAdd(fMultDiv2(ci[7], pQmfReal[pReadIdx[7]]), cr[7],
                             pQmfImag[pReadIdx[7]]))) +
       fMult(p[1], (fMultAdd(fMultDiv2(ci[11], pQmfReal[pReadIdx[11]]), cr[11],
                             pQmfImag[pReadIdx[11]]))));

  /* twiddle dee dee */
  fft[FFT_IDX_R(2)] =
      (fMult(p[12], (fMultSub(fMultDiv2(cr[0], pQmfReal[pReadIdx[0]]), ci[0],
                              pQmfImag[pReadIdx[0]]))) +
       fMult(p[8], (fMultSub(fMultDiv2(cr[4], pQmfReal[pReadIdx[4]]), ci[4],
                             pQmfImag[pReadIdx[4]]))) +
       fMult(p[4], (fMultSub(fMultDiv2(cr[8], pQmfReal[pReadIdx[8]]), ci[8],
                             pQmfImag[pReadIdx[8]]))) +
       fMult(p[0], (fMultSub(fMultDiv2(cr[12], pQmfReal[pReadIdx[12]]), ci[12],
                             pQmfImag[pReadIdx[12]]))));
  fft[FFT_IDX_I(2)] =
      (fMult(p[12], (fMultAdd(fMultDiv2(ci[0], pQmfReal[pReadIdx[0]]), cr[0],
                              pQmfImag[pReadIdx[0]]))) +
       fMult(p[8], (fMultAdd(fMultDiv2(ci[4], pQmfReal[pReadIdx[4]]), cr[4],
                             pQmfImag[pReadIdx[4]]))) +
       fMult(p[4], (fMultAdd(fMultDiv2(ci[8], pQmfReal[pReadIdx[8]]), cr[8],
                             pQmfImag[pReadIdx[8]]))) +
       fMult(p[0], (fMultAdd(fMultDiv2(ci[12], pQmfReal[pReadIdx[12]]), cr[12],
                             pQmfImag[pReadIdx[12]]))));

  fft[FFT_IDX_R(3)] =
      (fMult(p[11], (fMultSub(fMultDiv2(cr[1], pQmfReal[pReadIdx[1]]), ci[1],
                              pQmfImag[pReadIdx[1]]))) +
       fMult(p[7], (fMultSub(fMultDiv2(cr[5], pQmfReal[pReadIdx[5]]), ci[5],
                             pQmfImag[pReadIdx[5]]))) +
       fMult(p[3], (fMultSub(fMultDiv2(cr[9], pQmfReal[pReadIdx[9]]), ci[9],
                             pQmfImag[pReadIdx[9]]))));
  fft[FFT_IDX_I(3)] =
      (fMult(p[11], (fMultAdd(fMultDiv2(ci[1], pQmfReal[pReadIdx[1]]), cr[1],
                              pQmfImag[pReadIdx[1]]))) +
       fMult(p[7], (fMultAdd(fMultDiv2(ci[5], pQmfReal[pReadIdx[5]]), cr[5],
                             pQmfImag[pReadIdx[5]]))) +
       fMult(p[3], (fMultAdd(fMultDiv2(ci[9], pQmfReal[pReadIdx[9]]), cr[9],
                             pQmfImag[pReadIdx[9]]))));

  /* fft modulation                                                    */
  /* here: fast manual fft modulation for a fft of length M=4          */
  /* fft_4{x[n]} = x[0]*exp(-i*2*pi/4*m*0) + x[1]*exp(-i*2*pi/4*m*1) +
  x[2]*exp(-i*2*pi/4*m*2) + x[3]*exp(-i*2*pi/4*m*3)   */

  /*
  fft bin m=0:
  X[0, n] = x[0] +   x[1] + x[2] +   x[3]
  */
  mHybridReal[0] = fft[FFT_IDX_R(0)] + fft[FFT_IDX_R(1)] + fft[FFT_IDX_R(2)] +
                   fft[FFT_IDX_R(3)];
  mHybridImag[0] = fft[FFT_IDX_I(0)] + fft[FFT_IDX_I(1)] + fft[FFT_IDX_I(2)] +
                   fft[FFT_IDX_I(3)];

  /*
  fft bin m=1:
  X[1, n] = x[0] - i*x[1] - x[2] + i*x[3]
  */
  mHybridReal[1] = fft[FFT_IDX_R(0)] + fft[FFT_IDX_I(1)] - fft[FFT_IDX_R(2)] -
                   fft[FFT_IDX_I(3)];
  mHybridImag[1] = fft[FFT_IDX_I(0)] - fft[FFT_IDX_R(1)] - fft[FFT_IDX_I(2)] +
                   fft[FFT_IDX_R(3)];

  /*
  fft bin m=2:
  X[2, n] = x[0] -   x[1] + x[2] -   x[3]
  */
  mHybridReal[2] = fft[FFT_IDX_R(0)] - fft[FFT_IDX_R(1)] + fft[FFT_IDX_R(2)] -
                   fft[FFT_IDX_R(3)];
  mHybridImag[2] = fft[FFT_IDX_I(0)] - fft[FFT_IDX_I(1)] + fft[FFT_IDX_I(2)] -
                   fft[FFT_IDX_I(3)];

  /*
  fft bin m=3:
  X[3, n] = x[0] + j*x[1] - x[2] - j*x[3]
  */
  mHybridReal[3] = fft[FFT_IDX_R(0)] - fft[FFT_IDX_I(1)] - fft[FFT_IDX_R(2)] +
                   fft[FFT_IDX_I(3)];
  mHybridImag[3] = fft[FFT_IDX_I(0)] + fft[FFT_IDX_R(1)] - fft[FFT_IDX_I(2)] -
                   fft[FFT_IDX_R(3)];
}

static void eightChannelFiltering(const FIXP_DBL *const pQmfReal,
                                  const FIXP_DBL *const pQmfImag,
                                  const INT *const pReadIdx,
                                  FIXP_DBL *const mHybridReal,
                                  FIXP_DBL *const mHybridImag,
                                  const INT invert) {
  const FIXP_HTP *p = HybFilterCoef8;
  INT k, sc;

  FIXP_DBL mfft[16 + ALIGNMENT_DEFAULT];
  FIXP_DBL *pfft = (FIXP_DBL *)ALIGN_PTR(mfft);

  FIXP_DBL accu1, accu2, accu3, accu4;

  /* pre twiddeling */
  pfft[FFT_IDX_R(0)] =
      pQmfReal[pReadIdx[6]] >>
      (3 + 1); /* fMultDiv2(p[0].v.re, pQmfReal[pReadIdx[6]]); */
  pfft[FFT_IDX_I(0)] =
      pQmfImag[pReadIdx[6]] >>
      (3 + 1); /* fMultDiv2(p[0].v.re, pQmfImag[pReadIdx[6]]); */

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[7]], pQmfImag[pReadIdx[7]],
               p[1]);
  pfft[FFT_IDX_R(1)] = accu1;
  pfft[FFT_IDX_I(1)] = accu2;

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[0]], pQmfImag[pReadIdx[0]],
               p[2]);
  cplxMultDiv2(&accu3, &accu4, pQmfReal[pReadIdx[8]], pQmfImag[pReadIdx[8]],
               p[3]);
  pfft[FFT_IDX_R(2)] = accu1 + accu3;
  pfft[FFT_IDX_I(2)] = accu2 + accu4;

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[1]], pQmfImag[pReadIdx[1]],
               p[4]);
  cplxMultDiv2(&accu3, &accu4, pQmfReal[pReadIdx[9]], pQmfImag[pReadIdx[9]],
               p[5]);
  pfft[FFT_IDX_R(3)] = accu1 + accu3;
  pfft[FFT_IDX_I(3)] = accu2 + accu4;

  pfft[FFT_IDX_R(4)] = fMultDiv2(pQmfImag[pReadIdx[10]], p[7].v.im) -
                       fMultDiv2(pQmfImag[pReadIdx[2]], p[6].v.im);
  pfft[FFT_IDX_I(4)] = fMultDiv2(pQmfReal[pReadIdx[2]], p[6].v.im) -
                       fMultDiv2(pQmfReal[pReadIdx[10]], p[7].v.im);

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[3]], pQmfImag[pReadIdx[3]],
               p[8]);
  cplxMultDiv2(&accu3, &accu4, pQmfReal[pReadIdx[11]], pQmfImag[pReadIdx[11]],
               p[9]);
  pfft[FFT_IDX_R(5)] = accu1 + accu3;
  pfft[FFT_IDX_I(5)] = accu2 + accu4;

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[4]], pQmfImag[pReadIdx[4]],
               p[10]);
  cplxMultDiv2(&accu3, &accu4, pQmfReal[pReadIdx[12]], pQmfImag[pReadIdx[12]],
               p[11]);
  pfft[FFT_IDX_R(6)] = accu1 + accu3;
  pfft[FFT_IDX_I(6)] = accu2 + accu4;

  cplxMultDiv2(&accu1, &accu2, pQmfReal[pReadIdx[5]], pQmfImag[pReadIdx[5]],
               p[12]);
  pfft[FFT_IDX_R(7)] = accu1;
  pfft[FFT_IDX_I(7)] = accu2;

  /* fft modulation */
  fft_8(pfft);
  sc = 1 + 2;

  if (invert) {
    mHybridReal[0] = pfft[FFT_IDX_R(7)] << sc;
    mHybridImag[0] = pfft[FFT_IDX_I(7)] << sc;
    mHybridReal[1] = pfft[FFT_IDX_R(0)] << sc;
    mHybridImag[1] = pfft[FFT_IDX_I(0)] << sc;

    mHybridReal[2] = pfft[FFT_IDX_R(6)] << sc;
    mHybridImag[2] = pfft[FFT_IDX_I(6)] << sc;
    mHybridReal[3] = pfft[FFT_IDX_R(1)] << sc;
    mHybridImag[3] = pfft[FFT_IDX_I(1)] << sc;

    mHybridReal[4] = SATURATE_LEFT_SHIFT(
        (pfft[FFT_IDX_R(2)] + pfft[FFT_IDX_R(5)]), sc, DFRACT_BITS);
    mHybridImag[4] = SATURATE_LEFT_SHIFT(
        (pfft[FFT_IDX_I(2)] + pfft[FFT_IDX_I(5)]), sc, DFRACT_BITS);

    mHybridReal[5] = SATURATE_LEFT_SHIFT(
        (pfft[FFT_IDX_R(3)] + pfft[FFT_IDX_R(4)]), sc, DFRACT_BITS);
    mHybridImag[5] = SATURATE_LEFT_SHIFT(
        (pfft[FFT_IDX_I(3)] + pfft[FFT_IDX_I(4)]), sc, DFRACT_BITS);
  } else {
    for (k = 0; k < 8; k++) {
      mHybridReal[k] = pfft[FFT_IDX_R(k)] << sc;
      mHybridImag[k] = pfft[FFT_IDX_I(k)] << sc;
    }
  }
}

static INT kChannelFiltering(const FIXP_DBL *const pQmfReal,
                             const FIXP_DBL *const pQmfImag,
                             const INT *const pReadIdx,
                             FIXP_DBL *const mHybridReal,
                             FIXP_DBL *const mHybridImag,
                             const SCHAR hybridConfig) {
  INT err = 0;

  switch (hybridConfig) {
    case 2:
    case -2:
      dualChannelFiltering(pQmfReal, pQmfImag, pReadIdx, mHybridReal,
                           mHybridImag, (hybridConfig < 0) ? 1 : 0);
      break;
    case 4:
    case -4:
      fourChannelFiltering(pQmfReal, pQmfImag, pReadIdx, mHybridReal,
                           mHybridImag, (hybridConfig < 0) ? 1 : 0);
      break;
    case 8:
    case -8:
      eightChannelFiltering(pQmfReal, pQmfImag, pReadIdx, mHybridReal,
                            mHybridImag, (hybridConfig < 0) ? 1 : 0);
      break;
    default:
      err = -1;
  }

  return err;
}
