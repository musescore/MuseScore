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

/************************* MPEG-D DRC decoder library **************************

   Author(s):

   Description:

*******************************************************************************/

#include "drcDec_types.h"
#include "drcDec_gainDecoder.h"
#include "drcGainDec_process.h"

#define E_TGAINSTEP 12

static DRC_ERROR _prepareLnbIndex(ACTIVE_DRC* pActiveDrc,
                                  const int channelOffset,
                                  const int drcChannelOffset,
                                  const int numChannelsProcessed,
                                  const int lnbPointer) {
  int g, c;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = pActiveDrc->pInst;

  /* channelOffset: start index of physical channels
     numChannelsProcessed: number of processed channels, physical channels and
     DRC channels channelOffset + drcChannelOffset: start index of DRC channels,
        i.e. the channel order referenced in pInst.sequenceIndex */

  /* sanity checks */
  if ((channelOffset + numChannelsProcessed) > 8) return DE_NOT_OK;

  if ((channelOffset + drcChannelOffset + numChannelsProcessed) > 8)
    return DE_NOT_OK;

  if ((channelOffset + drcChannelOffset) < 0) return DE_NOT_OK;

  /* prepare lnbIndexForChannel, a map of indices from each channel to its
   * corresponding linearNodeBuffer instance */
  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    if (pInst->drcSetId > 0) {
      int drcChannel = c + drcChannelOffset;
      /* fallback for configuration with more physical channels than DRC
         channels: reuse DRC gain of first channel. This is necessary for HE-AAC
         mono with stereo output */
      if (drcChannel >= pInst->drcChannelCount) drcChannel = 0;
      g = pActiveDrc->channelGroupForChannel[drcChannel];
      if ((g >= 0) && !pActiveDrc->channelGroupIsParametricDrc[g]) {
        pActiveDrc->lnbIndexForChannel[c][lnbPointer] =
            pActiveDrc->activeDrcOffset + pActiveDrc->gainElementForGroup[g];
      }
    }
  }

  return DE_OK;
}

static DRC_ERROR _interpolateDrcGain(
    const GAIN_INTERPOLATION_TYPE gainInterpolationType,
    const SHORT timePrev,  /* time0 */
    const SHORT tGainStep, /* time1 - time0 */
    const SHORT start, const SHORT stop, const SHORT stepsize,
    const FIXP_DBL gainLeft, const FIXP_DBL gainRight, const FIXP_DBL slopeLeft,
    const FIXP_DBL slopeRight, FIXP_DBL* buffer) {
  int n, n_buf;
  int start_modulo, start_offset;

  if (tGainStep < 0) {
    return DE_NOT_OK;
  }
  if (tGainStep == 0) {
    return DE_OK;
  }

  /* get start index offset and buffer index for downsampled interpolation */
  /* start_modulo = (start+timePrev)%stepsize; */ /* stepsize is a power of 2 */
  start_modulo = (start + timePrev) & (stepsize - 1);
  start_offset = (start_modulo ? stepsize - start_modulo : 0);
  /* n_buf = (start + timePrev + start_offset)/stepsize; */
  n_buf = (start + timePrev + start_offset) >> (15 - fixnormz_S(stepsize));

  { /* gainInterpolationType == GIT_LINEAR */
    LONG a;
    /* runs = ceil((stop - start - start_offset)/stepsize). This works for
     * stepsize = 2^N only. */
    INT runs = (INT)(stop - start - start_offset + stepsize - 1) >>
               (30 - CountLeadingBits(stepsize));
    INT n_min = fMin(
        fMin(CntLeadingZeros(gainRight), CntLeadingZeros(gainLeft)) - 1, 8);
    a = (LONG)((gainRight << n_min) - (gainLeft << n_min)) / tGainStep;
    LONG a_step = a * stepsize;
    n = start + start_offset;
    a = a * n + (LONG)(gainLeft << n_min);
    buffer += n_buf;
#if defined(FUNCTION_interpolateDrcGain_func1)
    interpolateDrcGain_func1(buffer, a, a_step, n_min, runs);
#else
    a -= a_step;
    n_min = 8 - n_min;
    for (int i = 0; i < runs; i++) {
      a += a_step;
      buffer[i] = fMultDiv2(buffer[i], (FIXP_DBL)a) << n_min;
    }
#endif /* defined(FUNCTION_interpolateDrcGain_func1) */
  }
  return DE_OK;
}

static DRC_ERROR _processNodeSegments(
    const int frameSize, const GAIN_INTERPOLATION_TYPE gainInterpolationType,
    const int nNodes, const NODE_LIN* pNodeLin, const int offset,
    const SHORT stepsize,
    const NODE_LIN nodePrevious, /* the last node of the previous frame */
    const FIXP_DBL channelGain, FIXP_DBL* buffer) {
  DRC_ERROR err = DE_OK;
  SHORT timePrev, duration, start, stop, time;
  int n;
  FIXP_DBL gainLin = FL2FXCONST_DBL(1.0f / (float)(1 << 7)), gainLinPrev;
  FIXP_DBL slopeLin = (FIXP_DBL)0, slopeLinPrev = (FIXP_DBL)0;

  timePrev = nodePrevious.time + offset;
  gainLinPrev = nodePrevious.gainLin;
  for (n = 0; n < nNodes; n++) {
    time = pNodeLin[n].time + offset;
    duration = time - timePrev;
    gainLin = pNodeLin[n].gainLin;
    if (channelGain != FL2FXCONST_DBL(1.0f / (float)(1 << 8)))
      gainLin =
          SATURATE_LEFT_SHIFT(fMultDiv2(gainLin, channelGain), 9, DFRACT_BITS);

    if ((timePrev >= (frameSize - 1)) ||
        (time < 0)) { /* This segment (between previous and current node) lies
                         outside of this audio frame */
      timePrev = time;
      gainLinPrev = gainLin;
      slopeLinPrev = slopeLin;
      continue;
    }

    /* start and stop are the boundaries of the region of this segment that lie
       within this audio frame. Their values are relative to the beginning of
       this segment. stop is the first sample that isn't processed any more. */
    start = fMax(-timePrev, 1);
    stop = fMin(time, (SHORT)(frameSize - 1)) - timePrev + 1;

    err = _interpolateDrcGain(gainInterpolationType, timePrev, duration, start,
                              stop, stepsize, gainLinPrev, gainLin,
                              slopeLinPrev, slopeLin, buffer);
    if (err) return err;

    timePrev = time;
    gainLinPrev = gainLin;
  }
  return err;
}

/* process DRC on time-domain signal */
DRC_ERROR
processDrcTime(HANDLE_DRC_GAIN_DECODER hGainDec, const int activeDrcIndex,
               const int delaySamples, const int channelOffset,
               const int drcChannelOffset, const int numChannelsProcessed,
               const int timeDataChannelOffset, FIXP_DBL* deinterleavedAudio) {
  DRC_ERROR err = DE_OK;
  int c, b, i;
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcIndex]);
  DRC_GAIN_BUFFERS* pDrcGainBuffers = &(hGainDec->drcGainBuffers);
  int lnbPointer = pDrcGainBuffers->lnbPointer, lnbIx;
  LINEAR_NODE_BUFFER* pLinearNodeBuffer = pDrcGainBuffers->linearNodeBuffer;
  LINEAR_NODE_BUFFER* pDummyLnb = &(pDrcGainBuffers->dummyLnb);
  int offset = 0;

  if (hGainDec->delayMode == DM_REGULAR_DELAY) {
    offset = hGainDec->frameSize;
  }

  if ((delaySamples + offset) >
      (NUM_LNB_FRAMES - 2) *
          hGainDec->frameSize) /* if delaySamples is too big, NUM_LNB_FRAMES
                                  should be increased */
    return DE_NOT_OK;

  err = _prepareLnbIndex(pActiveDrc, channelOffset, drcChannelOffset,
                         numChannelsProcessed, lnbPointer);
  if (err) return err;

  deinterleavedAudio +=
      channelOffset * timeDataChannelOffset; /* apply channelOffset */

  /* signal processing loop */
  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    if (activeDrcIndex == hGainDec->channelGainActiveDrcIndex)
      pDrcGainBuffers->channelGain[c][lnbPointer] = hGainDec->channelGain[c];

    b = 0;
    {
      LINEAR_NODE_BUFFER *pLnb, *pLnbPrevious;
      NODE_LIN nodePrevious;
      int lnbPointerDiff;
      FIXP_DBL channelGain;
      /* get pointer to oldest linearNodes */
      lnbIx = lnbPointer + 1 - NUM_LNB_FRAMES;
      while (lnbIx < 0) lnbIx += NUM_LNB_FRAMES;

      if (activeDrcIndex == hGainDec->channelGainActiveDrcIndex)
        channelGain = pDrcGainBuffers->channelGain[c][lnbIx];
      else
        channelGain = FL2FXCONST_DBL(1.0f / (float)(1 << 8));

      /* Loop over all node buffers in linearNodeBuffer.
         All nodes which are not relevant for the current frame are sorted out
         inside _processNodeSegments. */
      for (i = 0; i < NUM_LNB_FRAMES - 1; i++) {
        /* Prepare previous node */
        if (pActiveDrc->lnbIndexForChannel[c][lnbIx] >= 0)
          pLnbPrevious = &(
              pLinearNodeBuffer[pActiveDrc->lnbIndexForChannel[c][lnbIx] + b]);
        else
          pLnbPrevious = pDummyLnb;
        nodePrevious =
            pLnbPrevious->linearNode[lnbIx][pLnbPrevious->nNodes[lnbIx] - 1];
        nodePrevious.time -= hGainDec->frameSize;
        if (channelGain != FL2FXCONST_DBL(1.0f / (float)(1 << 8)))
          nodePrevious.gainLin = SATURATE_LEFT_SHIFT(
              fMultDiv2(nodePrevious.gainLin,
                        pDrcGainBuffers->channelGain[c][lnbIx]),
              9, DFRACT_BITS);

        /* Prepare current linearNodeBuffer instance */
        lnbIx++;
        if (lnbIx >= NUM_LNB_FRAMES) lnbIx = 0;

        /* if lnbIndexForChannel changes over time, use the old indices for
         * smooth transitions */
        if (pActiveDrc->lnbIndexForChannel[c][lnbIx] >= 0)
          pLnb = &(
              pLinearNodeBuffer[pActiveDrc->lnbIndexForChannel[c][lnbIx] + b]);
        else /* lnbIndexForChannel = -1 means "no DRC processing", due to
                drcInstructionsIndex < 0, drcSetId < 0 or channel group < 0 */
          pLnb = pDummyLnb;

        if (activeDrcIndex == hGainDec->channelGainActiveDrcIndex)
          channelGain = pDrcGainBuffers->channelGain[c][lnbIx];

        /* number of frames of offset with respect to lnbPointer */
        lnbPointerDiff = i - (NUM_LNB_FRAMES - 2);

        err = _processNodeSegments(
            hGainDec->frameSize, pLnb->gainInterpolationType,
            pLnb->nNodes[lnbIx], pLnb->linearNode[lnbIx],
            lnbPointerDiff * hGainDec->frameSize + delaySamples + offset, 1,
            nodePrevious, channelGain, deinterleavedAudio);
        if (err) return err;
      }
      deinterleavedAudio += timeDataChannelOffset; /* proceed to next channel */
    }
  }
  return DE_OK;
}

/* process DRC on subband-domain signal */
DRC_ERROR
processDrcSubband(HANDLE_DRC_GAIN_DECODER hGainDec, const int activeDrcIndex,
                  const int delaySamples, const int channelOffset,
                  const int drcChannelOffset, const int numChannelsProcessed,
                  const int processSingleTimeslot,
                  FIXP_DBL* deinterleavedAudioReal[],
                  FIXP_DBL* deinterleavedAudioImag[]) {
  DRC_ERROR err = DE_OK;
  int b, c, g, m, m_start, m_stop, s, i;
  FIXP_DBL gainSb;
  DRC_INSTRUCTIONS_UNI_DRC* pInst = hGainDec->activeDrc[activeDrcIndex].pInst;
  DRC_GAIN_BUFFERS* pDrcGainBuffers = &(hGainDec->drcGainBuffers);
  ACTIVE_DRC* pActiveDrc = &(hGainDec->activeDrc[activeDrcIndex]);
  int activeDrcOffset = pActiveDrc->activeDrcOffset;
  int lnbPointer = pDrcGainBuffers->lnbPointer, lnbIx;
  LINEAR_NODE_BUFFER* pLinearNodeBuffer = pDrcGainBuffers->linearNodeBuffer;
  FIXP_DBL(*subbandGains)[4 * 1024 / 256] = hGainDec->subbandGains;
  FIXP_DBL* dummySubbandGains = hGainDec->dummySubbandGains;
  SUBBAND_DOMAIN_MODE subbandDomainMode = hGainDec->subbandDomainSupported;
  int signalIndex = 0;
  int frameSizeSb = 0;
  int nDecoderSubbands;
  SHORT L = 0; /* L: downsampling factor */
  int offset = 0;
  FIXP_DBL *audioReal = NULL, *audioImag = NULL;

  if (hGainDec->delayMode == DM_REGULAR_DELAY) {
    offset = hGainDec->frameSize;
  }

  if ((delaySamples + offset) >
      (NUM_LNB_FRAMES - 2) *
          hGainDec->frameSize) /* if delaySamples is too big, NUM_LNB_FRAMES
                                  should be increased */
    return DE_NOT_OK;

  switch (subbandDomainMode) {
#if ((1024 / 256) >= (4096 / SUBBAND_DOWNSAMPLING_FACTOR_QMF64))
    case SDM_QMF64:
      nDecoderSubbands = SUBBAND_NUM_BANDS_QMF64;
      L = SUBBAND_DOWNSAMPLING_FACTOR_QMF64;
      /* analysisDelay = SUBBAND_ANALYSIS_DELAY_QMF64; */
      break;
    case SDM_QMF71:
      nDecoderSubbands = SUBBAND_NUM_BANDS_QMF71;
      L = SUBBAND_DOWNSAMPLING_FACTOR_QMF71;
      /* analysisDelay = SUBBAND_ANALYSIS_DELAY_QMF71; */
      break;
#else
    case SDM_QMF64:
    case SDM_QMF71:
      /* QMF domain processing is not supported. */
      return DE_NOT_OK;
#endif
    case SDM_STFT256:
      nDecoderSubbands = SUBBAND_NUM_BANDS_STFT256;
      L = SUBBAND_DOWNSAMPLING_FACTOR_STFT256;
      /* analysisDelay = SUBBAND_ANALYSIS_DELAY_STFT256; */
      break;
    default:
      return DE_NOT_OK;
  }

  /* frameSizeSb = hGainDec->frameSize/L; */ /* L is a power of 2 */
  frameSizeSb =
      hGainDec->frameSize >> (15 - fixnormz_S(L)); /* timeslots per frame */

  if ((processSingleTimeslot < 0) || (processSingleTimeslot >= frameSizeSb)) {
    m_start = 0;
    m_stop = frameSizeSb;
  } else {
    m_start = processSingleTimeslot;
    m_stop = m_start + 1;
  }

  err = _prepareLnbIndex(pActiveDrc, channelOffset, drcChannelOffset,
                         numChannelsProcessed, lnbPointer);
  if (err) return err;

  if (!pActiveDrc->subbandGainsReady) /* only for the first time per frame that
                                         processDrcSubband is called */
  {
    /* write subbandGains */
    for (g = 0; g < pInst->nDrcChannelGroups; g++) {
      b = 0;
      {
        LINEAR_NODE_BUFFER* pLnb =
            &(pLinearNodeBuffer[activeDrcOffset +
                                pActiveDrc->gainElementForGroup[g] + b]);
        NODE_LIN nodePrevious;
        int lnbPointerDiff;

        for (m = 0; m < frameSizeSb; m++) {
          subbandGains[activeDrcOffset + g][b * frameSizeSb + m] =
              FL2FXCONST_DBL(1.0f / (float)(1 << 7));
        }

        lnbIx = lnbPointer - (NUM_LNB_FRAMES - 1);
        while (lnbIx < 0) lnbIx += NUM_LNB_FRAMES;

        /* Loop over all node buffers in linearNodeBuffer.
           All nodes which are not relevant for the current frame are sorted out
           inside _processNodeSegments. */
        for (i = 0; i < NUM_LNB_FRAMES - 1; i++) {
          /* Prepare previous node */
          nodePrevious = pLnb->linearNode[lnbIx][pLnb->nNodes[lnbIx] - 1];
          nodePrevious.time -= hGainDec->frameSize;

          lnbIx++;
          if (lnbIx >= NUM_LNB_FRAMES) lnbIx = 0;

          /* number of frames of offset with respect to lnbPointer */
          lnbPointerDiff = i - (NUM_LNB_FRAMES - 2);

          err = _processNodeSegments(
              hGainDec->frameSize, pLnb->gainInterpolationType,
              pLnb->nNodes[lnbIx], pLnb->linearNode[lnbIx],
              lnbPointerDiff * hGainDec->frameSize + delaySamples + offset -
                  (L - 1) / 2,
              L, nodePrevious, FL2FXCONST_DBL(1.0f / (float)(1 << 8)),
              &(subbandGains[activeDrcOffset + g][b * frameSizeSb]));
          if (err) return err;
        }
      }
    }
    pActiveDrc->subbandGainsReady = 1;
  }

  for (c = channelOffset; c < channelOffset + numChannelsProcessed; c++) {
    FIXP_DBL* thisSubbandGainsBuffer;
    if (pInst->drcSetId > 0)
      g = pActiveDrc->channelGroupForChannel[c + drcChannelOffset];
    else
      g = -1;

    audioReal = deinterleavedAudioReal[signalIndex];
    if (subbandDomainMode != SDM_STFT256) {
      audioImag = deinterleavedAudioImag[signalIndex];
    }

    if ((g >= 0) && !pActiveDrc->channelGroupIsParametricDrc[g]) {
      thisSubbandGainsBuffer = subbandGains[activeDrcOffset + g];
    } else {
      thisSubbandGainsBuffer = dummySubbandGains;
    }

    for (m = m_start; m < m_stop; m++) {
      INT n_min = 8;
      { /* single-band DRC */
        gainSb = thisSubbandGainsBuffer[m];
        if (activeDrcIndex == hGainDec->channelGainActiveDrcIndex)
          gainSb = SATURATE_LEFT_SHIFT(
              fMultDiv2(gainSb, hGainDec->channelGain[c]), 9, DFRACT_BITS);
        /* normalize gainSb for keeping signal precision */
        n_min = fMin(CntLeadingZeros(gainSb) - 1, n_min);
        gainSb <<= n_min;
        n_min = 8 - n_min;
        if (subbandDomainMode ==
            SDM_STFT256) { /* For STFT filterbank, real and imaginary parts are
                              interleaved. */
          for (s = 0; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
          }
        } else {
          for (s = 0; s < nDecoderSubbands; s++) {
            *audioReal = fMultDiv2(*audioReal, gainSb) << n_min;
            audioReal++;
            *audioImag = fMultDiv2(*audioImag, gainSb) << n_min;
            audioImag++;
          }
        }
      }
    }
    signalIndex++;
  }
  return DE_OK;
}
