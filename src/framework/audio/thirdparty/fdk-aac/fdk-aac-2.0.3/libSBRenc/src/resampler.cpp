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
  \brief  FDK resampler tool box:$Revision: 91655 $
  \author M. Werner
*/

#include "resampler.h"

#include "genericStds.h"

/**************************************************************************/
/*                   BIQUAD Filter Specifications                         */
/**************************************************************************/

#define B1 0
#define B2 1
#define A1 2
#define A2 3

#define BQC(x) FL2FXCONST_SGL(x / 2)

struct FILTER_PARAM {
  const FIXP_SGL *coeffa; /*! SOS matrix One row/section. Scaled using BQC().
                             Order of coefficients: B1,B2,A1,A2. B0=A0=1.0 */
  FIXP_DBL g;             /*! overall gain */
  int Wc;       /*! normalized passband bandwidth at input samplerate * 1000 */
  int noCoeffs; /*! number of filter coeffs */
  int delay;    /*! delay in samples at input samplerate */
};

#define BIQUAD_COEFSTEP 4

/**
 *\brief Low Pass
 Wc = 0,5, order 30, Stop Band -96dB. Wc criteria is "almost 0dB passband", not
 the usual -3db gain point. [b,a]=cheby2(30,96,0.505) [sos,g]=tf2sos(b,a)
 bandwidth 0.48
 */
static const FIXP_SGL sos48[] = {
    BQC(1.98941075681938),      BQC(0.999999996890811),
    BQC(0.863264527201963),     BQC(0.189553799960663),
    BQC(1.90733804822445),      BQC(1.00000001736189),
    BQC(0.836321575841691),     BQC(0.203505809266564),
    BQC(1.75616665495325),      BQC(0.999999946079721),
    BQC(0.784699225121588),     BQC(0.230471265506986),
    BQC(1.55727745512726),      BQC(1.00000011737815),
    BQC(0.712515423588351),     BQC(0.268752723900498),
    BQC(1.33407591943643),      BQC(0.999999795953228),
    BQC(0.625059117330989),     BQC(0.316194685288965),
    BQC(1.10689898412458),      BQC(1.00000035057114),
    BQC(0.52803514366398),      BQC(0.370517843224669),
    BQC(0.89060371078454),      BQC(0.999999343962822),
    BQC(0.426920462165257),     BQC(0.429608200207746),
    BQC(0.694438261209433),     BQC(1.0000008629792),
    BQC(0.326530699561716),     BQC(0.491714450654174),
    BQC(0.523237800935322),     BQC(1.00000101349782),
    BQC(0.230829556274851),     BQC(0.555559034843281),
    BQC(0.378631165929563),     BQC(0.99998986482665),
    BQC(0.142906422036095),     BQC(0.620338874442411),
    BQC(0.260786911308437),     BQC(1.00003261460178),
    BQC(0.0651008576256505),    BQC(0.685759923926262),
    BQC(0.168409429188098),     BQC(0.999933049695828),
    BQC(-0.000790067789975562), BQC(0.751905896602325),
    BQC(0.100724533818628),     BQC(1.00009472669872),
    BQC(-0.0533772830257041),   BQC(0.81930744384525),
    BQC(0.0561434357867363),    BQC(0.999911636304276),
    BQC(-0.0913550299236405),   BQC(0.88883625875915),
    BQC(0.0341680678662057),    BQC(1.00003667508676),
    BQC(-0.113405185536697),    BQC(0.961756638268446)};

static const FIXP_DBL g48 =
    FL2FXCONST_DBL(0.002712866530047) - (FIXP_DBL)0x8000;

static const struct FILTER_PARAM param_set48 = {
    sos48, g48, 480, 15, 4 /* LF 2 */
};

/**
 *\brief Low Pass
 Wc = 0,5, order 24, Stop Band -96dB. Wc criteria is "almost 0dB passband", not
 the usual -3db gain point. [b,a]=cheby2(24,96,0.5) [sos,g]=tf2sos(b,a)
 bandwidth 0.45
 */
static const FIXP_SGL sos45[] = {
    BQC(1.982962601444),     BQC(1.00000000007504),    BQC(0.646113303737836),
    BQC(0.10851149979981),   BQC(1.85334094281111),    BQC(0.999999999677192),
    BQC(0.612073220102006),  BQC(0.130022141698044),   BQC(1.62541051415425),
    BQC(1.00000000080398),   BQC(0.547879702855959),   BQC(0.171165825133192),
    BQC(1.34554656923247),   BQC(0.9999999980169),     BQC(0.460373914508491),
    BQC(0.228677463376354),  BQC(1.05656568503116),    BQC(1.00000000569363),
    BQC(0.357891894038287),  BQC(0.298676843912185),   BQC(0.787967587877312),
    BQC(0.999999984415017),  BQC(0.248826893211877),   BQC(0.377441803512978),
    BQC(0.555480971120497),  BQC(1.00000003583307),    BQC(0.140614263345315),
    BQC(0.461979302213679),  BQC(0.364986207070964),   BQC(0.999999932084303),
    BQC(0.0392669446074516), BQC(0.55033451180825),    BQC(0.216827267631558),
    BQC(1.00000010534682),   BQC(-0.0506232228865103), BQC(0.641691581560946),
    BQC(0.108951672277119),  BQC(0.999999871167516),   BQC(-0.125584840183225),
    BQC(0.736367748771803),  BQC(0.0387988607229035),  BQC(1.00000011205574),
    BQC(-0.182814849097974), BQC(0.835802108714964),   BQC(0.0042866175809225),
    BQC(0.999999954830813),  BQC(-0.21965740617151),   BQC(0.942623047782363)};

static const FIXP_DBL g45 =
    FL2FXCONST_DBL(0.00242743980909524) - (FIXP_DBL)0x8000;

static const struct FILTER_PARAM param_set45 = {
    sos45, g45, 450, 12, 4 /* LF 2 */
};

/*
 Created by Octave 2.1.73, Mon Oct 13 17:31:32 2008 CEST
 Wc = 0,5, order 16, Stop Band -96dB damping.
 [b,a]=cheby2(16,96,0.5)
 [sos,g]=tf2sos(b,a)
 bandwidth = 0.41
 */

static const FIXP_SGL sos41[] = {
    BQC(1.96193625292),      BQC(0.999999999999964),   BQC(0.169266178786789),
    BQC(0.0128823300475907), BQC(1.68913437662092),    BQC(1.00000000000053),
    BQC(0.124751503206552),  BQC(0.0537472273950989),  BQC(1.27274692366017),
    BQC(0.999999999995674),  BQC(0.0433108625178357),  BQC(0.131015753236317),
    BQC(0.85214175088395),   BQC(1.00000000001813),    BQC(-0.0625658152550408),
    BQC(0.237763778993806),  BQC(0.503841579939009),   BQC(0.999999999953223),
    BQC(-0.179176128722865), BQC(0.367475236424474),   BQC(0.249990711986162),
    BQC(1.00000000007952),   BQC(-0.294425165824676),  BQC(0.516594857170212),
    BQC(0.087971668680286),  BQC(0.999999999915528),   BQC(-0.398956566777928),
    BQC(0.686417767801123),  BQC(0.00965373325350294), BQC(1.00000000003744),
    BQC(-0.48579173764817),  BQC(0.884931534239068)};

static const FIXP_DBL g41 = FL2FXCONST_DBL(0.00155956951169248);

static const struct FILTER_PARAM param_set41 = {
    sos41, g41, 410, 8, 5 /* LF 3 */
};

/*
 # Created by Octave 2.1.73, Mon Oct 13 17:55:33 2008 CEST
 Wc = 0,5, order 12, Stop Band -96dB damping.
 [b,a]=cheby2(12,96,0.5);
 [sos,g]=tf2sos(b,a)
*/
static const FIXP_SGL sos35[] = {
    BQC(1.93299325235762),   BQC(0.999999999999985),  BQC(-0.140733187246596),
    BQC(0.0124139497836062), BQC(1.4890416764109),    BQC(1.00000000000011),
    BQC(-0.198215402588504), BQC(0.0746730616584138), BQC(0.918450161309795),
    BQC(0.999999999999619),  BQC(-0.30133912791941),  BQC(0.192276468839529),
    BQC(0.454877024246818),  BQC(1.00000000000086),   BQC(-0.432337328809815),
    BQC(0.356852933642815),  BQC(0.158017147118507),  BQC(0.999999999998876),
    BQC(-0.574817494249777), BQC(0.566380436970833),  BQC(0.0171834649478749),
    BQC(1.00000000000055),   BQC(-0.718581178041165), BQC(0.83367484487889)};

static const FIXP_DBL g35 = FL2FXCONST_DBL(0.00162580994125131);

static const struct FILTER_PARAM param_set35 = {sos35, g35, 350, 6, 4};

/*
 # Created by Octave 2.1.73, Mon Oct 13 18:15:38 2008 CEST
 Wc = 0,5, order 8, Stop Band -96dB damping.
 [b,a]=cheby2(8,96,0.5);
 [sos,g]=tf2sos(b,a)
*/
static const FIXP_SGL sos25[] = {
    BQC(1.85334094301225),   BQC(1.0),
    BQC(-0.702127214212663), BQC(0.132452403998767),
    BQC(1.056565682167),     BQC(0.999999999999997),
    BQC(-0.789503667880785), BQC(0.236328693569128),
    BQC(0.364986307455489),  BQC(0.999999999999996),
    BQC(-0.955191189843375), BQC(0.442966457936379),
    BQC(0.0387985751642125), BQC(1.0),
    BQC(-1.19817786088084),  BQC(0.770493895456328)};

static const FIXP_DBL g25 = FL2FXCONST_DBL(0.000945182835294559);

static const struct FILTER_PARAM param_set25 = {sos25, g25, 250, 4, 5};

/* Must be sorted in descending order */
static const struct FILTER_PARAM *const filter_paramSet[] = {
    &param_set48, &param_set45, &param_set41, &param_set35, &param_set25};

/**************************************************************************/
/*                         Resampler Functions                            */
/**************************************************************************/

/*!
  \brief   Reset downsampler instance and clear delay lines

  \return  success of operation
*/

INT FDKaacEnc_InitDownsampler(
    DOWNSAMPLER *DownSampler, /*!< pointer to downsampler instance */
    int Wc,                   /*!< normalized cutoff freq * 1000*  */
    int ratio)                /*!< downsampler ratio */

{
  UINT i;
  const struct FILTER_PARAM *currentSet = NULL;

  FDKmemclear(DownSampler->downFilter.states,
              sizeof(DownSampler->downFilter.states));
  DownSampler->downFilter.ptr = 0;

  /*
    find applicable parameter set
  */
  currentSet = filter_paramSet[0];
  for (i = 1; i < sizeof(filter_paramSet) / sizeof(struct FILTER_PARAM *);
       i++) {
    if (filter_paramSet[i]->Wc <= Wc) {
      break;
    }
    currentSet = filter_paramSet[i];
  }

  DownSampler->downFilter.coeffa = currentSet->coeffa;

  DownSampler->downFilter.gain = currentSet->g;
  FDK_ASSERT(currentSet->noCoeffs <= MAXNR_SECTIONS * 2);

  DownSampler->downFilter.noCoeffs = currentSet->noCoeffs;
  DownSampler->delay = currentSet->delay;
  DownSampler->downFilter.Wc = currentSet->Wc;

  DownSampler->ratio = ratio;
  DownSampler->pending = ratio - 1;
  return (1);
}

/*!
  \brief   faster simple folding operation
           Filter:
           H(z) = A(z)/B(z)
           with
           A(z) = a[0]*z^0 + a[1]*z^1 + a[2]*z^2 ... a[n]*z^n

  \return  filtered value
*/

static inline INT_PCM AdvanceFilter(
    LP_FILTER *downFilter, /*!< pointer to iir filter instance */
    INT_PCM *pInput,       /*!< input of filter                */
    int downRatio) {
  INT_PCM output;
  int i, n;

#define BIQUAD_SCALE 12

  FIXP_DBL y = FL2FXCONST_DBL(0.0f);
  FIXP_DBL input;

  for (n = 0; n < downRatio; n++) {
    FIXP_BQS(*states)[2] = downFilter->states;
    const FIXP_SGL *coeff = downFilter->coeffa;
    int s1, s2;

    s1 = downFilter->ptr;
    s2 = s1 ^ 1;

#if (SAMPLE_BITS == 16)
    input = ((FIXP_DBL)pInput[n]) << (DFRACT_BITS - SAMPLE_BITS - BIQUAD_SCALE);
#elif (SAMPLE_BITS == 32)
    input = pInput[n] >> BIQUAD_SCALE;
#else
#error NOT IMPLEMENTED
#endif

    FIXP_BQS state1, state2, state1b, state2b;

    state1 = states[0][s1];
    state2 = states[0][s2];

    /* Loop over sections */
    for (i = 0; i < downFilter->noCoeffs; i++) {
      FIXP_DBL state0;

      /* Load merged states (from next section) */
      state1b = states[i + 1][s1];
      state2b = states[i + 1][s2];

      state0 = input + fMult(state1, coeff[B1]) + fMult(state2, coeff[B2]);
      y = state0 - fMult(state1b, coeff[A1]) - fMult(state2b, coeff[A2]);

      /* Store new feed forward merge state */
      states[i + 1][s2] = y << 1;
      /* Store new feed backward state */
      states[i][s2] = input << 1;

      /* Feedback output to next section. */
      input = y;

      /* Transfer merged states */
      state1 = state1b;
      state2 = state2b;

      /* Step to next coef set */
      coeff += BIQUAD_COEFSTEP;
    }
    downFilter->ptr ^= 1;
  }
  /* Apply global gain */
  y = fMult(y, downFilter->gain);

  /* Apply final gain/scaling to output */
#if (SAMPLE_BITS == 16)
  output = (INT_PCM)SATURATE_RIGHT_SHIFT(
      y + (FIXP_DBL)(1 << (DFRACT_BITS - SAMPLE_BITS - BIQUAD_SCALE - 1)),
      DFRACT_BITS - SAMPLE_BITS - BIQUAD_SCALE, SAMPLE_BITS);
  // output = (INT_PCM) SATURATE_RIGHT_SHIFT(y,
  // DFRACT_BITS-SAMPLE_BITS-BIQUAD_SCALE, SAMPLE_BITS);
#else
  output = SATURATE_LEFT_SHIFT(y, BIQUAD_SCALE, SAMPLE_BITS);
#endif

  return output;
}

/*!
  \brief   FDKaacEnc_Downsample numInSamples of type INT_PCM
           Returns number of output samples in numOutSamples

  \return  success of operation
*/

INT FDKaacEnc_Downsample(
    DOWNSAMPLER *DownSampler, /*!< pointer to downsampler instance */
    INT_PCM *inSamples,       /*!< pointer to input samples */
    INT numInSamples,         /*!< number  of input samples  */
    INT_PCM *outSamples,      /*!< pointer to output samples */
    INT *numOutSamples        /*!< pointer tp number of output samples */
) {
  INT i;
  *numOutSamples = 0;

  for (i = 0; i < numInSamples; i += DownSampler->ratio) {
    *outSamples = AdvanceFilter(&(DownSampler->downFilter), &inSamples[i],
                                DownSampler->ratio);
    outSamples++;
  }
  *numOutSamples = numInSamples / DownSampler->ratio;

  return 0;
}
