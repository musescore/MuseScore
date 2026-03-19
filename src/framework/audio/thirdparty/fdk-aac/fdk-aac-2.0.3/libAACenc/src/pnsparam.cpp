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

   Author(s):   M.Lohwasser

   Description: PNS parameters depending on bitrate and bandwidth

*******************************************************************************/

#include "pnsparam.h"

#include "psy_configuration.h"

typedef struct {
  SHORT startFreq;
  /* Parameters for detection */
  FIXP_SGL refPower;
  FIXP_SGL refTonality;
  SHORT tnsGainThreshold;    /* scaled by TNS_PREDGAIN_SCALE (=1000) */
  SHORT tnsPNSGainThreshold; /* scaled by TNS_PREDGAIN_SCALE (=1000) */
  FIXP_SGL gapFillThr;
  SHORT minSfbWidth;
  USHORT detectionAlgorithmFlags;
} PNS_INFO_TAB;

typedef struct {
  ULONG brFrom;
  ULONG brTo;
  UCHAR S16000;
  UCHAR S22050;
  UCHAR S24000;
  UCHAR S32000;
  UCHAR S44100;
  UCHAR S48000;
} AUTO_PNS_TAB;

static const AUTO_PNS_TAB levelTable_mono[] = {
    {
        0,
        11999,
        0,
        1,
        1,
        1,
        1,
        1,
    },
    {
        12000,
        19999,
        0,
        1,
        1,
        1,
        1,
        1,
    },
    {
        20000,
        28999,
        0,
        2,
        1,
        1,
        1,
        1,
    },
    {
        29000,
        40999,
        0,
        4,
        4,
        4,
        2,
        2,
    },
    {
        41000,
        55999,
        0,
        9,
        9,
        7,
        7,
        7,
    },
    {
        56000,
        61999,
        0,
        0,
        0,
        0,
        9,
        9,
    },
    {
        62000,
        75999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    {
        76000,
        92999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    {
        93000,
        999999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
};

static const AUTO_PNS_TAB levelTable_stereo[] = {
    {
        0,
        11999,
        0,
        1,
        1,
        1,
        1,
        1,
    },
    {
        12000,
        19999,
        0,
        3,
        1,
        1,
        1,
        1,
    },
    {
        20000,
        28999,
        0,
        3,
        3,
        3,
        2,
        2,
    },
    {
        29000,
        40999,
        0,
        7,
        6,
        6,
        5,
        5,
    },
    {
        41000,
        55999,
        0,
        9,
        9,
        7,
        7,
        7,
    },
    {
        56000,
        79999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    {
        80000,
        99999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    {
        100000,
        999999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
};

static const PNS_INFO_TAB pnsInfoTab[] = {
    /*0   pns off */
    /*1*/ {4000, FL2FXCONST_SGL(0.04), FL2FXCONST_SGL(0.06), 1150, 1200,
           FL2FXCONST_SGL(0.02), 8,
           USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
               USE_TNS_PNS /*| JUST_LONG_WINDOW*/},
    /*2*/
    {4000, FL2FXCONST_SGL(0.04), FL2FXCONST_SGL(0.07), 1130, 1300,
     FL2FXCONST_SGL(0.05), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS /*| JUST_LONG_WINDOW*/},
    /*3*/
    {4100, FL2FXCONST_SGL(0.04), FL2FXCONST_SGL(0.07), 1100, 1400,
     FL2FXCONST_SGL(0.10), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS /*| JUST_LONG_WINDOW*/},
    /*4*/
    {4100, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.10), 1100, 1400,
     FL2FXCONST_SGL(0.15), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS /*| JUST_LONG_WINDOW*/},
    /*5*/
    {4300, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.10), 1100, 1400,
     FL2FXCONST_SGL(0.15), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /*6*/
    {5000, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.10), 1100, 1400,
     FL2FXCONST_SGL(0.25), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /*7*/
    {5500, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.12), 1100, 1400,
     FL2FXCONST_SGL(0.35), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /*8*/
    {6000, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.12), 1080, 1400,
     FL2FXCONST_SGL(0.40), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /*9*/
    {6000, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.14), 1070, 1400,
     FL2FXCONST_SGL(0.45), 8,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
};

static const AUTO_PNS_TAB levelTable_lowComplexity[] = {
    {
        0,
        27999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
    {
        28000,
        31999,
        0,
        2,
        2,
        2,
        2,
        2,
    },
    {
        32000,
        47999,
        0,
        3,
        3,
        3,
        3,
        3,
    },
    {
        48000,
        48000,
        0,
        4,
        4,
        4,
        4,
        4,
    },
    {
        48001,
        999999,
        0,
        0,
        0,
        0,
        0,
        0,
    },
};
/* conversion of old LC tuning tables to new (LD enc) structure (only entries
 * which are actually used were converted) */
static const PNS_INFO_TAB pnsInfoTab_lowComplexity[] = {
    /*0   pns off */
    /* DEFAULT parameter set */
    /*1*/ {4100, FL2FXCONST_SGL(0.03), FL2FXCONST_SGL(0.16), 1100, 1400,
           FL2FXCONST_SGL(0.5), 16,
           USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
               USE_TNS_PNS | JUST_LONG_WINDOW},
    /*2*/
    {4100, FL2FXCONST_SGL(0.05), FL2FXCONST_SGL(0.10), 1410, 1400,
     FL2FXCONST_SGL(0.5), 16,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /*3*/
    {4100, FL2FXCONST_SGL(0.05), FL2FXCONST_SGL(0.10), 1100, 1400,
     FL2FXCONST_SGL(0.5), 16,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
    /* LOWSUBST -> PNS is used less often than with DEFAULT parameter set (for
       br: 48000 - 79999) */
    /*4*/
    {4100, FL2FXCONST_SGL(0.20), FL2FXCONST_SGL(0.10), 1410, 1400,
     FL2FXCONST_SGL(0.5), 16,
     USE_POWER_DISTRIBUTION | USE_PSYCH_TONALITY | USE_TNS_GAIN_THR |
         USE_TNS_PNS | JUST_LONG_WINDOW},
};

/****************************************************************************
  function to look up used pns level
****************************************************************************/
int FDKaacEnc_lookUpPnsUse(int bitRate, int sampleRate, int numChan,
                           const int isLC) {
  int hUsePns = 0, size, i;
  const AUTO_PNS_TAB *levelTable;

  if (isLC) {
    levelTable = &levelTable_lowComplexity[0];
    size = sizeof(levelTable_lowComplexity);
  } else { /* (E)LD */
    levelTable = (numChan > 1) ? &levelTable_stereo[0] : &levelTable_mono[0];
    size = (numChan > 1) ? sizeof(levelTable_stereo) : sizeof(levelTable_mono);
  }

  for (i = 0; i < (int)(size / sizeof(AUTO_PNS_TAB)); i++) {
    if (((ULONG)bitRate >= levelTable[i].brFrom) &&
        ((ULONG)bitRate <= levelTable[i].brTo))
      break;
  }

  /* sanity check */
  if ((int)(sizeof(pnsInfoTab) / sizeof(PNS_INFO_TAB)) < i) {
    return (PNS_TABLE_ERROR);
  }

  switch (sampleRate) {
    case 16000:
      hUsePns = levelTable[i].S16000;
      break;
    case 22050:
      hUsePns = levelTable[i].S22050;
      break;
    case 24000:
      hUsePns = levelTable[i].S24000;
      break;
    case 32000:
      hUsePns = levelTable[i].S32000;
      break;
    case 44100:
      hUsePns = levelTable[i].S44100;
      break;
    case 48000:
      hUsePns = levelTable[i].S48000;
      break;
    default:
      if (isLC) {
        hUsePns = levelTable[i].S48000;
      }
      break;
  }

  return (hUsePns);
}

/*****************************************************************************

    functionname: FDKaacEnc_GetPnsParam
    description:  Gets PNS parameters depending on bitrate and bandwidth or
                    bitsPerLine
    returns:      error status
    input:        Noiseparams struct, bitrate, sampling rate,
                  number of sfb's, pointer to sfb offset
    output:       PNS parameters

*****************************************************************************/
AAC_ENCODER_ERROR FDKaacEnc_GetPnsParam(NOISEPARAMS *np, INT bitRate,
                                        INT sampleRate, INT sfbCnt,
                                        const INT *sfbOffset, INT *usePns,
                                        INT numChan, const INT isLC) {
  int i, hUsePns;
  const PNS_INFO_TAB *pnsInfo;

  if (*usePns <= 0) return AAC_ENC_OK;

  if (isLC) {
    np->detectionAlgorithmFlags = IS_LOW_COMPLEXITY;

    pnsInfo = pnsInfoTab_lowComplexity;

    /* new pns params */
    hUsePns = FDKaacEnc_lookUpPnsUse(bitRate, sampleRate, numChan, isLC);
    if (hUsePns == 0) {
      *usePns = 0;
      return AAC_ENC_OK;
    }

    if (hUsePns == PNS_TABLE_ERROR) {
      return AAC_ENC_PNS_TABLE_ERROR;
    }

    /* select correct row of tuning table */
    pnsInfo += hUsePns - 1;

  } else {
    np->detectionAlgorithmFlags = 0;
    pnsInfo = pnsInfoTab;

    /* new pns params */
    hUsePns = FDKaacEnc_lookUpPnsUse(bitRate, sampleRate, numChan, isLC);
    if (hUsePns == 0) {
      *usePns = 0;
      return AAC_ENC_OK;
    }
    if (hUsePns == PNS_TABLE_ERROR) return AAC_ENC_PNS_TABLE_ERROR;

    /* select correct row of tuning table */
    pnsInfo += hUsePns - 1;
  }

  np->startSfb = FDKaacEnc_FreqToBandWidthRounding(
      pnsInfo->startFreq, sampleRate, sfbCnt, sfbOffset);

  np->detectionAlgorithmFlags |= pnsInfo->detectionAlgorithmFlags;

  np->refPower = FX_SGL2FX_DBL(pnsInfo->refPower);
  np->refTonality = FX_SGL2FX_DBL(pnsInfo->refTonality);
  np->tnsGainThreshold = pnsInfo->tnsGainThreshold;
  np->tnsPNSGainThreshold = pnsInfo->tnsPNSGainThreshold;
  np->minSfbWidth = pnsInfo->minSfbWidth;

  np->gapFillThr =
      pnsInfo->gapFillThr; /* for LC it is always FL2FXCONST_SGL(0.5) */

  /* assuming a constant dB/Hz slope in the signal's PSD curve,
    the detection threshold needs to be corrected for the width of the band */

  for (i = 0; i < (sfbCnt - 1); i++) {
    INT qtmp, sfbWidth;
    FIXP_DBL tmp;

    sfbWidth = sfbOffset[i + 1] - sfbOffset[i];

    tmp = fPow(np->refPower, 0, sfbWidth, DFRACT_BITS - 1 - 5, &qtmp);
    np->powDistPSDcurve[i] = (FIXP_SGL)((LONG)(scaleValue(tmp, qtmp) >> 16));
  }
  np->powDistPSDcurve[sfbCnt] = np->powDistPSDcurve[sfbCnt - 1];

  return AAC_ENC_OK;
}
