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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

/*!
  \file
  \brief  SBR bit writing routines $Revision: 93300 $
*/

#include "bit_sbr.h"

#include "code_env.h"
#include "cmondata.h"
#include "sbr.h"

#include "ps_main.h"

typedef enum { SBR_ID_SCE = 1, SBR_ID_CPE } SBR_ELEMENT_TYPE;

static INT encodeSbrData(HANDLE_SBR_ENV_DATA sbrEnvDataLeft,
                         HANDLE_SBR_ENV_DATA sbrEnvDataRight,
                         HANDLE_PARAMETRIC_STEREO hParametricStereo,
                         HANDLE_COMMON_DATA cmonData, SBR_ELEMENT_TYPE sbrElem,
                         INT coupling, UINT sbrSyntaxFlags);

static INT encodeSbrHeader(HANDLE_SBR_HEADER_DATA sbrHeaderData,
                           HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
                           HANDLE_COMMON_DATA cmonData);

static INT encodeSbrHeaderData(HANDLE_SBR_HEADER_DATA sbrHeaderData,
                               HANDLE_FDK_BITSTREAM hBitStream);

static INT encodeSbrSingleChannelElement(
    HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_FDK_BITSTREAM hBitStream,
    HANDLE_PARAMETRIC_STEREO hParametricStereo, const UINT sbrSyntaxFlags);

static INT encodeSbrChannelPairElement(
    HANDLE_SBR_ENV_DATA sbrEnvDataLeft, HANDLE_SBR_ENV_DATA sbrEnvDataRight,
    HANDLE_PARAMETRIC_STEREO hParametricStereo, HANDLE_FDK_BITSTREAM hBitStream,
    const INT coupling, const UINT sbrSyntaxFlags);

static INT encodeSbrGrid(HANDLE_SBR_ENV_DATA sbrEnvData,
                         HANDLE_FDK_BITSTREAM hBitStream);

static int encodeLowDelaySbrGrid(HANDLE_SBR_ENV_DATA sbrEnvData,
                                 HANDLE_FDK_BITSTREAM hBitStream,
                                 const int transmitFreqs,
                                 const UINT sbrSyntaxFlags);

static INT encodeSbrDtdf(HANDLE_SBR_ENV_DATA sbrEnvData,
                         HANDLE_FDK_BITSTREAM hBitStream);

static INT writeNoiseLevelData(HANDLE_SBR_ENV_DATA sbrEnvData,
                               HANDLE_FDK_BITSTREAM hBitStream, INT coupling);

static INT writeEnvelopeData(HANDLE_SBR_ENV_DATA sbrEnvData,
                             HANDLE_FDK_BITSTREAM hBitStream, INT coupling);

static INT writeSyntheticCodingData(HANDLE_SBR_ENV_DATA sbrEnvData,
                                    HANDLE_FDK_BITSTREAM hBitStream);

static INT encodeExtendedData(HANDLE_PARAMETRIC_STEREO hParametricStereo,
                              HANDLE_FDK_BITSTREAM hBitStream);

static INT getSbrExtendedDataSize(HANDLE_PARAMETRIC_STEREO hParametricStereo);

/*****************************************************************************

    functionname: FDKsbrEnc_WriteEnvSingleChannelElement
    description:  writes pure SBR single channel data element
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
INT FDKsbrEnc_WriteEnvSingleChannelElement(
    HANDLE_SBR_HEADER_DATA sbrHeaderData,
    HANDLE_PARAMETRIC_STEREO hParametricStereo,
    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData, HANDLE_SBR_ENV_DATA sbrEnvData,
    HANDLE_COMMON_DATA cmonData, UINT sbrSyntaxFlags)

{
  INT payloadBits = 0;

  cmonData->sbrHdrBits = 0;
  cmonData->sbrDataBits = 0;

  /* write pure sbr data */
  if (sbrEnvData != NULL) {
    /* write header */
    payloadBits += encodeSbrHeader(sbrHeaderData, sbrBitstreamData, cmonData);

    /* write data */
    payloadBits += encodeSbrData(sbrEnvData, NULL, hParametricStereo, cmonData,
                                 SBR_ID_SCE, 0, sbrSyntaxFlags);
  }
  return payloadBits;
}

/*****************************************************************************

    functionname: FDKsbrEnc_WriteEnvChannelPairElement
    description:  writes pure SBR channel pair data element
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
INT FDKsbrEnc_WriteEnvChannelPairElement(
    HANDLE_SBR_HEADER_DATA sbrHeaderData,
    HANDLE_PARAMETRIC_STEREO hParametricStereo,
    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
    HANDLE_SBR_ENV_DATA sbrEnvDataLeft, HANDLE_SBR_ENV_DATA sbrEnvDataRight,
    HANDLE_COMMON_DATA cmonData, UINT sbrSyntaxFlags)

{
  INT payloadBits = 0;
  cmonData->sbrHdrBits = 0;
  cmonData->sbrDataBits = 0;

  /* write pure sbr data */
  if ((sbrEnvDataLeft != NULL) && (sbrEnvDataRight != NULL)) {
    /* write header */
    payloadBits += encodeSbrHeader(sbrHeaderData, sbrBitstreamData, cmonData);

    /* write data */
    payloadBits += encodeSbrData(sbrEnvDataLeft, sbrEnvDataRight,
                                 hParametricStereo, cmonData, SBR_ID_CPE,
                                 sbrHeaderData->coupling, sbrSyntaxFlags);
  }
  return payloadBits;
}

INT FDKsbrEnc_CountSbrChannelPairElement(
    HANDLE_SBR_HEADER_DATA sbrHeaderData,
    HANDLE_PARAMETRIC_STEREO hParametricStereo,
    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
    HANDLE_SBR_ENV_DATA sbrEnvDataLeft, HANDLE_SBR_ENV_DATA sbrEnvDataRight,
    HANDLE_COMMON_DATA cmonData, UINT sbrSyntaxFlags) {
  INT payloadBits;
  INT bitPos = FDKgetValidBits(&cmonData->sbrBitbuf);

  payloadBits = FDKsbrEnc_WriteEnvChannelPairElement(
      sbrHeaderData, hParametricStereo, sbrBitstreamData, sbrEnvDataLeft,
      sbrEnvDataRight, cmonData, sbrSyntaxFlags);

  FDKpushBack(&cmonData->sbrBitbuf,
              (FDKgetValidBits(&cmonData->sbrBitbuf) - bitPos));

  return payloadBits;
}

void sbrEncoder_GetHeader(SBR_ENCODER *sbrEncoder, HANDLE_FDK_BITSTREAM hBs,
                          INT element_index, int fSendHeaders) {
  encodeSbrHeaderData(&sbrEncoder->sbrElement[element_index]->sbrHeaderData,
                      hBs);

  if (fSendHeaders == 0) {
    /* Prevent header being embedded into the SBR payload. */
    sbrEncoder->sbrElement[element_index]->sbrBitstreamData.NrSendHeaderData =
        -1;
    sbrEncoder->sbrElement[element_index]->sbrBitstreamData.HeaderActive = 0;
    sbrEncoder->sbrElement[element_index]
        ->sbrBitstreamData.CountSendHeaderData = -1;
  }
}

/*****************************************************************************

    functionname: encodeSbrHeader
    description:  encodes SBR Header information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT encodeSbrHeader(HANDLE_SBR_HEADER_DATA sbrHeaderData,
                           HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
                           HANDLE_COMMON_DATA cmonData) {
  INT payloadBits = 0;

  if (sbrBitstreamData->HeaderActive) {
    payloadBits += FDKwriteBits(&cmonData->sbrBitbuf, 1, 1);
    payloadBits += encodeSbrHeaderData(sbrHeaderData, &cmonData->sbrBitbuf);
  } else {
    payloadBits += FDKwriteBits(&cmonData->sbrBitbuf, 0, 1);
  }

  cmonData->sbrHdrBits = payloadBits;

  return payloadBits;
}

/*****************************************************************************

    functionname: encodeSbrHeaderData
    description:  writes sbr_header()
                  bs_protocol_version through bs_header_extra_2
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT encodeSbrHeaderData(HANDLE_SBR_HEADER_DATA sbrHeaderData,
                               HANDLE_FDK_BITSTREAM hBitStream)

{
  INT payloadBits = 0;
  if (sbrHeaderData != NULL) {
    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_amp_res,
                                SI_SBR_AMP_RES_BITS);
    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_start_frequency,
                                SI_SBR_START_FREQ_BITS);
    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_stop_frequency,
                                SI_SBR_STOP_FREQ_BITS);
    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_xover_band,
                                SI_SBR_XOVER_BAND_BITS);

    payloadBits += FDKwriteBits(hBitStream, 0, SI_SBR_RESERVED_BITS);

    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->header_extra_1,
                                SI_SBR_HEADER_EXTRA_1_BITS);
    payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->header_extra_2,
                                SI_SBR_HEADER_EXTRA_2_BITS);

    if (sbrHeaderData->header_extra_1) {
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->freqScale,
                                  SI_SBR_FREQ_SCALE_BITS);
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->alterScale,
                                  SI_SBR_ALTER_SCALE_BITS);
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_noise_bands,
                                  SI_SBR_NOISE_BANDS_BITS);
    } /* sbrHeaderData->header_extra_1 */

    if (sbrHeaderData->header_extra_2) {
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_limiter_bands,
                                  SI_SBR_LIMITER_BANDS_BITS);
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_limiter_gains,
                                  SI_SBR_LIMITER_GAINS_BITS);
      payloadBits += FDKwriteBits(hBitStream, sbrHeaderData->sbr_interpol_freq,
                                  SI_SBR_INTERPOL_FREQ_BITS);
      payloadBits +=
          FDKwriteBits(hBitStream, sbrHeaderData->sbr_smoothing_length,
                       SI_SBR_SMOOTHING_LENGTH_BITS);

    } /* sbrHeaderData->header_extra_2 */
  }   /* sbrHeaderData != NULL */

  return payloadBits;
}

/*****************************************************************************

    functionname: encodeSbrData
    description:  encodes sbr Data information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT encodeSbrData(HANDLE_SBR_ENV_DATA sbrEnvDataLeft,
                         HANDLE_SBR_ENV_DATA sbrEnvDataRight,
                         HANDLE_PARAMETRIC_STEREO hParametricStereo,
                         HANDLE_COMMON_DATA cmonData, SBR_ELEMENT_TYPE sbrElem,
                         INT coupling, UINT sbrSyntaxFlags) {
  INT payloadBits = 0;

  switch (sbrElem) {
    case SBR_ID_SCE:
      payloadBits +=
          encodeSbrSingleChannelElement(sbrEnvDataLeft, &cmonData->sbrBitbuf,
                                        hParametricStereo, sbrSyntaxFlags);
      break;
    case SBR_ID_CPE:
      payloadBits += encodeSbrChannelPairElement(
          sbrEnvDataLeft, sbrEnvDataRight, hParametricStereo,
          &cmonData->sbrBitbuf, coupling, sbrSyntaxFlags);
      break;
    default:
      /* we never should apply SBR to any other element type */
      FDK_ASSERT(0);
  }

  cmonData->sbrDataBits = payloadBits;

  return payloadBits;
}

#define MODE_FREQ_TANS 1
#define MODE_NO_FREQ_TRAN 0
#define LD_TRANSMISSION MODE_FREQ_TANS
static int encodeFreqs(int mode) { return ((mode & MODE_FREQ_TANS) ? 1 : 0); }

/*****************************************************************************

    functionname: encodeSbrSingleChannelElement
    description:  encodes sbr SCE information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT encodeSbrSingleChannelElement(
    HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_FDK_BITSTREAM hBitStream,
    HANDLE_PARAMETRIC_STEREO hParametricStereo, const UINT sbrSyntaxFlags) {
  INT i, payloadBits = 0;

  payloadBits += FDKwriteBits(hBitStream, 0,
                              SI_SBR_DATA_EXTRA_BITS); /* no reserved bits */

  if (sbrEnvData->ldGrid) {
    if (sbrEnvData->hSbrBSGrid->frameClass != FIXFIXonly) {
      /* encode normal SbrGrid */
      payloadBits += encodeSbrGrid(sbrEnvData, hBitStream);
    } else {
      /* use FIXFIXonly frame Grid */
      payloadBits += encodeLowDelaySbrGrid(
          sbrEnvData, hBitStream, encodeFreqs(LD_TRANSMISSION), sbrSyntaxFlags);
    }
  } else {
    if (sbrSyntaxFlags & SBR_SYNTAX_SCALABLE) {
      payloadBits += FDKwriteBits(hBitStream, 1, SI_SBR_COUPLING_BITS);
    }
    payloadBits += encodeSbrGrid(sbrEnvData, hBitStream);
  }

  payloadBits += encodeSbrDtdf(sbrEnvData, hBitStream);

  for (i = 0; i < sbrEnvData->noOfnoisebands; i++) {
    payloadBits += FDKwriteBits(hBitStream, sbrEnvData->sbr_invf_mode_vec[i],
                                SI_SBR_INVF_MODE_BITS);
  }

  payloadBits += writeEnvelopeData(sbrEnvData, hBitStream, 0);
  payloadBits += writeNoiseLevelData(sbrEnvData, hBitStream, 0);

  payloadBits += writeSyntheticCodingData(sbrEnvData, hBitStream);

  payloadBits += encodeExtendedData(hParametricStereo, hBitStream);

  return payloadBits;
}

/*****************************************************************************

    functionname: encodeSbrChannelPairElement
    description:  encodes sbr CPE information
    returns:
    input:
    output:

*****************************************************************************/
static INT encodeSbrChannelPairElement(
    HANDLE_SBR_ENV_DATA sbrEnvDataLeft, HANDLE_SBR_ENV_DATA sbrEnvDataRight,
    HANDLE_PARAMETRIC_STEREO hParametricStereo, HANDLE_FDK_BITSTREAM hBitStream,
    const INT coupling, const UINT sbrSyntaxFlags) {
  INT payloadBits = 0;
  INT i = 0;

  payloadBits += FDKwriteBits(hBitStream, 0,
                              SI_SBR_DATA_EXTRA_BITS); /* no reserved bits */

  payloadBits += FDKwriteBits(hBitStream, coupling, SI_SBR_COUPLING_BITS);

  if (coupling) {
    if (sbrEnvDataLeft->ldGrid) {
      if (sbrEnvDataLeft->hSbrBSGrid->frameClass != FIXFIXonly) {
        /* normal SbrGrid */
        payloadBits += encodeSbrGrid(sbrEnvDataLeft, hBitStream);

      } else {
        /* FIXFIXonly frame Grid */
        payloadBits +=
            encodeLowDelaySbrGrid(sbrEnvDataLeft, hBitStream,
                                  encodeFreqs(LD_TRANSMISSION), sbrSyntaxFlags);
      }
    } else
      payloadBits += encodeSbrGrid(sbrEnvDataLeft, hBitStream);

    payloadBits += encodeSbrDtdf(sbrEnvDataLeft, hBitStream);
    payloadBits += encodeSbrDtdf(sbrEnvDataRight, hBitStream);

    for (i = 0; i < sbrEnvDataLeft->noOfnoisebands; i++) {
      payloadBits +=
          FDKwriteBits(hBitStream, sbrEnvDataLeft->sbr_invf_mode_vec[i],
                       SI_SBR_INVF_MODE_BITS);
    }

    payloadBits += writeEnvelopeData(sbrEnvDataLeft, hBitStream, 1);
    payloadBits += writeNoiseLevelData(sbrEnvDataLeft, hBitStream, 1);
    payloadBits += writeEnvelopeData(sbrEnvDataRight, hBitStream, 1);
    payloadBits += writeNoiseLevelData(sbrEnvDataRight, hBitStream, 1);

    payloadBits += writeSyntheticCodingData(sbrEnvDataLeft, hBitStream);
    payloadBits += writeSyntheticCodingData(sbrEnvDataRight, hBitStream);

  } else { /* no coupling */
    FDK_ASSERT(sbrEnvDataLeft->ldGrid == sbrEnvDataRight->ldGrid);

    if (sbrEnvDataLeft->ldGrid || sbrEnvDataRight->ldGrid) {
      /* sbrEnvDataLeft (left channel) */
      if (sbrEnvDataLeft->hSbrBSGrid->frameClass != FIXFIXonly) {
        /* no FIXFIXonly Frame so we dont need encodeLowDelaySbrGrid */
        /* normal SbrGrid */
        payloadBits += encodeSbrGrid(sbrEnvDataLeft, hBitStream);

      } else {
        /* FIXFIXonly frame Grid */
        payloadBits +=
            encodeLowDelaySbrGrid(sbrEnvDataLeft, hBitStream,
                                  encodeFreqs(LD_TRANSMISSION), sbrSyntaxFlags);
      }

      /* sbrEnvDataRight (right channel) */
      if (sbrEnvDataRight->hSbrBSGrid->frameClass != FIXFIXonly) {
        /* no FIXFIXonly Frame so we dont need encodeLowDelaySbrGrid */
        /* normal SbrGrid */
        payloadBits += encodeSbrGrid(sbrEnvDataRight, hBitStream);

      } else {
        /* FIXFIXonly frame Grid */
        payloadBits +=
            encodeLowDelaySbrGrid(sbrEnvDataRight, hBitStream,
                                  encodeFreqs(LD_TRANSMISSION), sbrSyntaxFlags);
      }
    } else {
      payloadBits += encodeSbrGrid(sbrEnvDataLeft, hBitStream);
      payloadBits += encodeSbrGrid(sbrEnvDataRight, hBitStream);
    }
    payloadBits += encodeSbrDtdf(sbrEnvDataLeft, hBitStream);
    payloadBits += encodeSbrDtdf(sbrEnvDataRight, hBitStream);

    for (i = 0; i < sbrEnvDataLeft->noOfnoisebands; i++) {
      payloadBits +=
          FDKwriteBits(hBitStream, sbrEnvDataLeft->sbr_invf_mode_vec[i],
                       SI_SBR_INVF_MODE_BITS);
    }
    for (i = 0; i < sbrEnvDataRight->noOfnoisebands; i++) {
      payloadBits +=
          FDKwriteBits(hBitStream, sbrEnvDataRight->sbr_invf_mode_vec[i],
                       SI_SBR_INVF_MODE_BITS);
    }

    payloadBits += writeEnvelopeData(sbrEnvDataLeft, hBitStream, 0);
    payloadBits += writeEnvelopeData(sbrEnvDataRight, hBitStream, 0);
    payloadBits += writeNoiseLevelData(sbrEnvDataLeft, hBitStream, 0);
    payloadBits += writeNoiseLevelData(sbrEnvDataRight, hBitStream, 0);

    payloadBits += writeSyntheticCodingData(sbrEnvDataLeft, hBitStream);
    payloadBits += writeSyntheticCodingData(sbrEnvDataRight, hBitStream);

  } /* coupling */

  payloadBits += encodeExtendedData(hParametricStereo, hBitStream);

  return payloadBits;
}

static INT ceil_ln2(INT x) {
  INT tmp = -1;
  while ((1 << ++tmp) < x)
    ;
  return (tmp);
}

/*****************************************************************************

    functionname: encodeSbrGrid
    description:  if hBitStream != NULL writes bits that describes the
                  time/frequency grouping of a frame; else counts them only
    returns:      number of bits written or counted
    input:
    output:

*****************************************************************************/
static INT encodeSbrGrid(HANDLE_SBR_ENV_DATA sbrEnvData,
                         HANDLE_FDK_BITSTREAM hBitStream) {
  INT payloadBits = 0;
  INT i, temp;
  INT bufferFrameStart = sbrEnvData->hSbrBSGrid->bufferFrameStart;
  INT numberTimeSlots = sbrEnvData->hSbrBSGrid->numberTimeSlots;

  if (sbrEnvData->ldGrid)
    payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->frameClass,
                                SBR_CLA_BITS_LD);
  else
    payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->frameClass,
                                SBR_CLA_BITS);

  switch (sbrEnvData->hSbrBSGrid->frameClass) {
    case FIXFIXonly:
      FDK_ASSERT(0 /* Fatal error in encodeSbrGrid! */);
      break;
    case FIXFIX:
      temp = ceil_ln2(sbrEnvData->hSbrBSGrid->bs_num_env);
      payloadBits += FDKwriteBits(hBitStream, temp, SBR_ENV_BITS);
      if ((sbrEnvData->ldGrid) && (sbrEnvData->hSbrBSGrid->bs_num_env == 1))
        payloadBits += FDKwriteBits(hBitStream, sbrEnvData->currentAmpResFF,
                                    SI_SBR_AMP_RES_BITS);
      payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->v_f[0],
                                  SBR_RES_BITS);

      break;

    case FIXVAR:
    case VARFIX:
      if (sbrEnvData->hSbrBSGrid->frameClass == FIXVAR)
        temp = sbrEnvData->hSbrBSGrid->bs_abs_bord -
               (bufferFrameStart + numberTimeSlots);
      else
        temp = sbrEnvData->hSbrBSGrid->bs_abs_bord - bufferFrameStart;

      payloadBits += FDKwriteBits(hBitStream, temp, SBR_ABS_BITS);
      payloadBits +=
          FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->n, SBR_NUM_BITS);

      for (i = 0; i < sbrEnvData->hSbrBSGrid->n; i++) {
        temp = (sbrEnvData->hSbrBSGrid->bs_rel_bord[i] - 2) >> 1;
        payloadBits += FDKwriteBits(hBitStream, temp, SBR_REL_BITS);
      }

      temp = ceil_ln2(sbrEnvData->hSbrBSGrid->n + 2);
      payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->p, temp);

      for (i = 0; i < sbrEnvData->hSbrBSGrid->n + 1; i++) {
        payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->v_f[i],
                                    SBR_RES_BITS);
      }
      break;

    case VARVAR:
      temp = sbrEnvData->hSbrBSGrid->bs_abs_bord_0 - bufferFrameStart;
      payloadBits += FDKwriteBits(hBitStream, temp, SBR_ABS_BITS);
      temp = sbrEnvData->hSbrBSGrid->bs_abs_bord_1 -
             (bufferFrameStart + numberTimeSlots);
      payloadBits += FDKwriteBits(hBitStream, temp, SBR_ABS_BITS);

      payloadBits += FDKwriteBits(
          hBitStream, sbrEnvData->hSbrBSGrid->bs_num_rel_0, SBR_NUM_BITS);
      payloadBits += FDKwriteBits(
          hBitStream, sbrEnvData->hSbrBSGrid->bs_num_rel_1, SBR_NUM_BITS);

      for (i = 0; i < sbrEnvData->hSbrBSGrid->bs_num_rel_0; i++) {
        temp = (sbrEnvData->hSbrBSGrid->bs_rel_bord_0[i] - 2) >> 1;
        payloadBits += FDKwriteBits(hBitStream, temp, SBR_REL_BITS);
      }

      for (i = 0; i < sbrEnvData->hSbrBSGrid->bs_num_rel_1; i++) {
        temp = (sbrEnvData->hSbrBSGrid->bs_rel_bord_1[i] - 2) >> 1;
        payloadBits += FDKwriteBits(hBitStream, temp, SBR_REL_BITS);
      }

      temp = ceil_ln2(sbrEnvData->hSbrBSGrid->bs_num_rel_0 +
                      sbrEnvData->hSbrBSGrid->bs_num_rel_1 + 2);
      payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->p, temp);

      temp = sbrEnvData->hSbrBSGrid->bs_num_rel_0 +
             sbrEnvData->hSbrBSGrid->bs_num_rel_1 + 1;

      for (i = 0; i < temp; i++) {
        payloadBits += FDKwriteBits(
            hBitStream, sbrEnvData->hSbrBSGrid->v_fLR[i], SBR_RES_BITS);
      }
      break;
  }

  return payloadBits;
}

#define SBR_CLA_BITS_LD 1
/*****************************************************************************

    functionname: encodeLowDelaySbrGrid
    description:  if hBitStream != NULL writes bits that describes the
                  time/frequency grouping of a frame;
                  else counts them only
                  (this function only write the FIXFIXonly Bitstream data)
    returns:      number of bits written or counted
    input:
    output:

*****************************************************************************/
static int encodeLowDelaySbrGrid(HANDLE_SBR_ENV_DATA sbrEnvData,
                                 HANDLE_FDK_BITSTREAM hBitStream,
                                 const int transmitFreqs,
                                 const UINT sbrSyntaxFlags) {
  int payloadBits = 0;
  int i;

  /* write FIXFIXonly Grid */
  /* write frameClass [1 bit] for FIXFIXonly Grid */
  payloadBits += FDKwriteBits(hBitStream, 1, SBR_CLA_BITS_LD);

  /* absolute Borders are fix: 0,X,X,X,nTimeSlots; so we dont have to transmit
   * them */
  /* only transmit the transient position! */
  /* with this info (b1) we can reconstruct the Frame on Decoder side : */
  /* border[0] = 0; border[1] = b1; border[2]=b1+2; border[3] = nrTimeSlots */

  /* use 3 or 4bits for transient border (border) */
  if (sbrEnvData->hSbrBSGrid->numberTimeSlots == 8)
    payloadBits +=
        FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->bs_abs_bord, 3);
  else
    payloadBits +=
        FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->bs_abs_bord, 4);

  if (transmitFreqs) {
    /* write FreqRes grid */
    for (i = 0; i < sbrEnvData->hSbrBSGrid->bs_num_env; i++) {
      payloadBits += FDKwriteBits(hBitStream, sbrEnvData->hSbrBSGrid->v_f[i],
                                  SBR_RES_BITS);
    }
  }

  return payloadBits;
}

/*****************************************************************************

    functionname: encodeSbrDtdf
    description:  writes bits that describes the direction of the envelopes of a
frame returns:      number of bits written input: output:

*****************************************************************************/
static INT encodeSbrDtdf(HANDLE_SBR_ENV_DATA sbrEnvData,
                         HANDLE_FDK_BITSTREAM hBitStream) {
  INT i, payloadBits = 0, noOfNoiseEnvelopes;

  noOfNoiseEnvelopes = sbrEnvData->noOfEnvelopes > 1 ? 2 : 1;

  for (i = 0; i < sbrEnvData->noOfEnvelopes; ++i) {
    payloadBits +=
        FDKwriteBits(hBitStream, sbrEnvData->domain_vec[i], SBR_DIR_BITS);
  }
  for (i = 0; i < noOfNoiseEnvelopes; ++i) {
    payloadBits +=
        FDKwriteBits(hBitStream, sbrEnvData->domain_vec_noise[i], SBR_DIR_BITS);
  }

  return payloadBits;
}

/*****************************************************************************

    functionname: writeNoiseLevelData
    description:  writes bits corresponding to the noise-floor-level
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT writeNoiseLevelData(HANDLE_SBR_ENV_DATA sbrEnvData,
                               HANDLE_FDK_BITSTREAM hBitStream, INT coupling) {
  INT j, i, payloadBits = 0;
  INT nNoiseEnvelopes = sbrEnvData->noOfEnvelopes > 1 ? 2 : 1;

  for (i = 0; i < nNoiseEnvelopes; i++) {
    switch (sbrEnvData->domain_vec_noise[i]) {
      case FREQ:
        if (coupling && sbrEnvData->balance) {
          payloadBits += FDKwriteBits(
              hBitStream,
              sbrEnvData->sbr_noise_levels[i * sbrEnvData->noOfnoisebands],
              sbrEnvData->si_sbr_start_noise_bits_balance);
        } else {
          payloadBits += FDKwriteBits(
              hBitStream,
              sbrEnvData->sbr_noise_levels[i * sbrEnvData->noOfnoisebands],
              sbrEnvData->si_sbr_start_noise_bits);
        }

        for (j = 1 + i * sbrEnvData->noOfnoisebands;
             j < (sbrEnvData->noOfnoisebands * (1 + i)); j++) {
          if (coupling) {
            if (sbrEnvData->balance) {
              /* coupling && balance */
              payloadBits += FDKwriteBits(hBitStream,
                                          sbrEnvData->hufftableNoiseBalanceFreqC
                                              [sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV_BALANCE11],
                                          sbrEnvData->hufftableNoiseBalanceFreqL
                                              [sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV_BALANCE11]);
            } else {
              /* coupling && !balance */
              payloadBits += FDKwriteBits(
                  hBitStream,
                  sbrEnvData->hufftableNoiseLevelFreqC
                      [sbrEnvData->sbr_noise_levels[j] + CODE_BOOK_SCF_LAV11],
                  sbrEnvData->hufftableNoiseLevelFreqL
                      [sbrEnvData->sbr_noise_levels[j] + CODE_BOOK_SCF_LAV11]);
            }
          } else {
            /* !coupling */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableNoiseFreqC[sbrEnvData->sbr_noise_levels[j] +
                                          CODE_BOOK_SCF_LAV11],
                sbrEnvData
                    ->hufftableNoiseFreqL[sbrEnvData->sbr_noise_levels[j] +
                                          CODE_BOOK_SCF_LAV11]);
          }
        }
        break;

      case TIME:
        for (j = i * sbrEnvData->noOfnoisebands;
             j < (sbrEnvData->noOfnoisebands * (1 + i)); j++) {
          if (coupling) {
            if (sbrEnvData->balance) {
              /* coupling && balance */
              payloadBits += FDKwriteBits(hBitStream,
                                          sbrEnvData->hufftableNoiseBalanceTimeC
                                              [sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV_BALANCE11],
                                          sbrEnvData->hufftableNoiseBalanceTimeL
                                              [sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV_BALANCE11]);
            } else {
              /* coupling && !balance */
              payloadBits += FDKwriteBits(
                  hBitStream,
                  sbrEnvData->hufftableNoiseLevelTimeC
                      [sbrEnvData->sbr_noise_levels[j] + CODE_BOOK_SCF_LAV11],
                  sbrEnvData->hufftableNoiseLevelTimeL
                      [sbrEnvData->sbr_noise_levels[j] + CODE_BOOK_SCF_LAV11]);
            }
          } else {
            /* !coupling */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableNoiseLevelTimeC[sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV11],
                sbrEnvData
                    ->hufftableNoiseLevelTimeL[sbrEnvData->sbr_noise_levels[j] +
                                               CODE_BOOK_SCF_LAV11]);
          }
        }
        break;
    }
  }
  return payloadBits;
}

/*****************************************************************************

    functionname: writeEnvelopeData
    description:  writes bits corresponding to the envelope
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT writeEnvelopeData(HANDLE_SBR_ENV_DATA sbrEnvData,
                             HANDLE_FDK_BITSTREAM hBitStream, INT coupling) {
  INT payloadBits = 0, j, i, delta;

  for (j = 0; j < sbrEnvData->noOfEnvelopes;
       j++) { /* loop over all envelopes */
    if (sbrEnvData->domain_vec[j] == FREQ) {
      if (coupling && sbrEnvData->balance) {
        payloadBits += FDKwriteBits(hBitStream, sbrEnvData->ienvelope[j][0],
                                    sbrEnvData->si_sbr_start_env_bits_balance);
      } else {
        payloadBits += FDKwriteBits(hBitStream, sbrEnvData->ienvelope[j][0],
                                    sbrEnvData->si_sbr_start_env_bits);
      }
    }

    for (i = 1 - sbrEnvData->domain_vec[j]; i < sbrEnvData->noScfBands[j];
         i++) {
      delta = sbrEnvData->ienvelope[j][i];
      if (coupling && sbrEnvData->balance) {
        FDK_ASSERT(fixp_abs(delta) <= sbrEnvData->codeBookScfLavBalance);
      } else {
        FDK_ASSERT(fixp_abs(delta) <= sbrEnvData->codeBookScfLav);
      }
      if (coupling) {
        if (sbrEnvData->balance) {
          if (sbrEnvData->domain_vec[j]) {
            /* coupling && balance && TIME */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableBalanceTimeC[delta +
                                            sbrEnvData->codeBookScfLavBalance],
                sbrEnvData
                    ->hufftableBalanceTimeL[delta +
                                            sbrEnvData->codeBookScfLavBalance]);
          } else {
            /* coupling && balance && FREQ */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableBalanceFreqC[delta +
                                            sbrEnvData->codeBookScfLavBalance],
                sbrEnvData
                    ->hufftableBalanceFreqL[delta +
                                            sbrEnvData->codeBookScfLavBalance]);
          }
        } else {
          if (sbrEnvData->domain_vec[j]) {
            /* coupling && !balance && TIME */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableLevelTimeC[delta + sbrEnvData->codeBookScfLav],
                sbrEnvData
                    ->hufftableLevelTimeL[delta + sbrEnvData->codeBookScfLav]);
          } else {
            /* coupling && !balance && FREQ */
            payloadBits += FDKwriteBits(
                hBitStream,
                sbrEnvData
                    ->hufftableLevelFreqC[delta + sbrEnvData->codeBookScfLav],
                sbrEnvData
                    ->hufftableLevelFreqL[delta + sbrEnvData->codeBookScfLav]);
          }
        }
      } else {
        if (sbrEnvData->domain_vec[j]) {
          /* !coupling && TIME */
          payloadBits += FDKwriteBits(
              hBitStream,
              sbrEnvData->hufftableTimeC[delta + sbrEnvData->codeBookScfLav],
              sbrEnvData->hufftableTimeL[delta + sbrEnvData->codeBookScfLav]);
        } else {
          /* !coupling && FREQ */
          payloadBits += FDKwriteBits(
              hBitStream,
              sbrEnvData->hufftableFreqC[delta + sbrEnvData->codeBookScfLav],
              sbrEnvData->hufftableFreqL[delta + sbrEnvData->codeBookScfLav]);
        }
      }
    }
  }
  return payloadBits;
}

/*****************************************************************************

    functionname: encodeExtendedData
    description:  writes bits corresponding to the extended data
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT encodeExtendedData(HANDLE_PARAMETRIC_STEREO hParametricStereo,
                              HANDLE_FDK_BITSTREAM hBitStream) {
  INT extDataSize;
  INT payloadBits = 0;

  extDataSize = getSbrExtendedDataSize(hParametricStereo);

  if (extDataSize != 0) {
    INT maxExtSize = (1 << SI_SBR_EXTENSION_SIZE_BITS) - 1;
    INT writtenNoBits = 0; /* needed to byte align the extended data */

    payloadBits += FDKwriteBits(hBitStream, 1, SI_SBR_EXTENDED_DATA_BITS);
    FDK_ASSERT(extDataSize <= SBR_EXTENDED_DATA_MAX_CNT);

    if (extDataSize < maxExtSize) {
      payloadBits +=
          FDKwriteBits(hBitStream, extDataSize, SI_SBR_EXTENSION_SIZE_BITS);
    } else {
      payloadBits +=
          FDKwriteBits(hBitStream, maxExtSize, SI_SBR_EXTENSION_SIZE_BITS);
      payloadBits += FDKwriteBits(hBitStream, extDataSize - maxExtSize,
                                  SI_SBR_EXTENSION_ESC_COUNT_BITS);
    }

    /* parametric coding signalled here? */
    if (hParametricStereo) {
      writtenNoBits += FDKwriteBits(hBitStream, EXTENSION_ID_PS_CODING,
                                    SI_SBR_EXTENSION_ID_BITS);
      writtenNoBits +=
          FDKsbrEnc_PSEnc_WritePSData(hParametricStereo, hBitStream);
    }

    payloadBits += writtenNoBits;

    /* byte alignment */
    writtenNoBits = writtenNoBits % 8;
    if (writtenNoBits)
      payloadBits += FDKwriteBits(hBitStream, 0, (8 - writtenNoBits));
  } else {
    payloadBits += FDKwriteBits(hBitStream, 0, SI_SBR_EXTENDED_DATA_BITS);
  }

  return payloadBits;
}

/*****************************************************************************

    functionname: writeSyntheticCodingData
    description:  writes bits corresponding to the "synthetic-coding"-extension
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static INT writeSyntheticCodingData(HANDLE_SBR_ENV_DATA sbrEnvData,
                                    HANDLE_FDK_BITSTREAM hBitStream)

{
  INT i;
  INT payloadBits = 0;

  payloadBits += FDKwriteBits(hBitStream, sbrEnvData->addHarmonicFlag, 1);

  if (sbrEnvData->addHarmonicFlag) {
    for (i = 0; i < sbrEnvData->noHarmonics; i++) {
      payloadBits += FDKwriteBits(hBitStream, sbrEnvData->addHarmonic[i], 1);
    }
  }

  return payloadBits;
}

/*****************************************************************************

    functionname: getSbrExtendedDataSize
    description:  counts the number of bits needed for encoding the
                  extended data (including extension id)

    returns:      number of bits needed for the extended data
    input:
    output:

*****************************************************************************/
static INT getSbrExtendedDataSize(HANDLE_PARAMETRIC_STEREO hParametricStereo) {
  INT extDataBits = 0;

  /* add your new extended data counting methods here */

  /*
    no extended data
  */

  if (hParametricStereo) {
    /* PS extended data */
    extDataBits += SI_SBR_EXTENSION_ID_BITS;
    extDataBits += FDKsbrEnc_PSEnc_WritePSData(hParametricStereo, NULL);
  }

  return (extDataBits + 7) >> 3;
}
