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

/**************************** SBR encoder library ******************************

   Author(s):

   Description:

*******************************************************************************/

#include "env_est.h"
#include "tran_det.h"

#include "qmf.h"

#include "fram_gen.h"
#include "bit_sbr.h"
#include "cmondata.h"
#include "sbrenc_ram.h"

#include "genericStds.h"

#define QUANT_ERROR_THRES 200
#define Y_NRG_SCALE 5 /* noCols = 32 -> shift(5) */
#define MAX_NRG_SLOTS_LD 16

static const UCHAR panTable[2][10] = {{0, 2, 4, 6, 8, 12, 16, 20, 24},
                                      {0, 2, 4, 8, 12, 0, 0, 0, 0}};
static const UCHAR maxIndex[2] = {9, 5};

/******************************************************************************
 Functionname:  FDKsbrEnc_GetTonality
******************************************************************************/
/***************************************************************************/
/*!

  \brief      Calculates complete energy per band from the energy values
              of the QMF subsamples.

  \brief      quotaMatrix - calculated in FDKsbrEnc_CalculateTonalityQuotas()
  \brief      noEstPerFrame - number of estimations per frame
  \brief      startIndex - start index for the quota matrix
  \brief      Energies - energy matrix
  \brief      startBand - start band
  \brief      stopBand - number of QMF bands
  \brief      numberCols - number of QMF subsamples

  \return     mean tonality of the 5 bands with the highest energy
              scaled by 2^(RELAXATION_SHIFT+2)*RELAXATION_FRACT

****************************************************************************/
static FIXP_DBL FDKsbrEnc_GetTonality(const FIXP_DBL *const *quotaMatrix,
                                      const INT noEstPerFrame,
                                      const INT startIndex,
                                      const FIXP_DBL *const *Energies,
                                      const UCHAR startBand, const INT stopBand,
                                      const INT numberCols) {
  UCHAR b, e, k;
  INT no_enMaxBand[SBR_MAX_ENERGY_VALUES] = {-1, -1, -1, -1, -1};
  FIXP_DBL energyMax[SBR_MAX_ENERGY_VALUES] = {
      FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f),
      FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f)};
  FIXP_DBL energyMaxMin = MAXVAL_DBL; /* min. energy in energyMax array */
  UCHAR posEnergyMaxMin = 0; /* min. energy in energyMax array position */
  FIXP_DBL tonalityBand[SBR_MAX_ENERGY_VALUES] = {
      FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f),
      FL2FXCONST_DBL(0.0f), FL2FXCONST_DBL(0.0f)};
  FIXP_DBL globalTonality = FL2FXCONST_DBL(0.0f);
  FIXP_DBL energyBand[64];
  INT maxNEnergyValues; /* max. number of max. energy values */

  /*** Sum up energies for each band ***/
  FDK_ASSERT(numberCols == 15 || numberCols == 16);
  /* numberCols is always 15 or 16 for ELD. In case of 16 bands, the
      energyBands are initialized with the [15]th column.
      The rest of the column energies are added in the next step.   */
  if (numberCols == 15) {
    for (b = startBand; b < stopBand; b++) {
      energyBand[b] = FL2FXCONST_DBL(0.0f);
    }
  } else {
    for (b = startBand; b < stopBand; b++) {
      energyBand[b] = Energies[15][b] >> 4;
    }
  }

  for (k = 0; k < 15; k++) {
    for (b = startBand; b < stopBand; b++) {
      energyBand[b] += Energies[k][b] >> 4;
    }
  }

  /*** Determine 5 highest band-energies ***/
  maxNEnergyValues = fMin(SBR_MAX_ENERGY_VALUES, stopBand - startBand);

  /* Get min. value in energyMax array */
  energyMaxMin = energyMax[0] = energyBand[startBand];
  no_enMaxBand[0] = startBand;
  posEnergyMaxMin = 0;
  for (k = 1; k < maxNEnergyValues; k++) {
    energyMax[k] = energyBand[startBand + k];
    no_enMaxBand[k] = startBand + k;
    if (energyMaxMin > energyMax[k]) {
      energyMaxMin = energyMax[k];
      posEnergyMaxMin = k;
    }
  }

  for (b = startBand + maxNEnergyValues; b < stopBand; b++) {
    if (energyBand[b] > energyMaxMin) {
      energyMax[posEnergyMaxMin] = energyBand[b];
      no_enMaxBand[posEnergyMaxMin] = b;

      /* Again, get min. value in energyMax array */
      energyMaxMin = energyMax[0];
      posEnergyMaxMin = 0;
      for (k = 1; k < maxNEnergyValues; k++) {
        if (energyMaxMin > energyMax[k]) {
          energyMaxMin = energyMax[k];
          posEnergyMaxMin = k;
        }
      }
    }
  }
  /*** End determine 5 highest band-energies ***/

  /* Get tonality values for 5 highest energies */
  for (e = 0; e < maxNEnergyValues; e++) {
    tonalityBand[e] = FL2FXCONST_DBL(0.0f);
    for (k = 0; k < noEstPerFrame; k++) {
      tonalityBand[e] += quotaMatrix[startIndex + k][no_enMaxBand[e]] >> 1;
    }
    globalTonality +=
        tonalityBand[e] >> 2; /* headroom of 2+1 (max. 5 additions) */
  }

  return globalTonality;
}

/***************************************************************************/
/*!

  \brief      Calculates energy form real and imaginary part of
              the QMF subsamples

  \return     none

****************************************************************************/
LNK_SECTION_CODE_L1
static void FDKsbrEnc_getEnergyFromCplxQmfData(
    FIXP_DBL **RESTRICT energyValues, /*!< the result of the operation */
    FIXP_DBL **RESTRICT realValues, /*!< the real part of the QMF subsamples */
    FIXP_DBL **RESTRICT
        imagValues,   /*!< the imaginary part of the QMF subsamples */
    INT numberBands,  /*!< number of QMF bands */
    INT numberCols,   /*!< number of QMF subsamples */
    INT *qmfScale,    /*!< sclefactor of QMF subsamples */
    INT *energyScale) /*!< scalefactor of energies */
{
  int j, k;
  int scale;
  FIXP_DBL max_val = FL2FXCONST_DBL(0.0f);

  /* Get Scratch buffer */
  C_ALLOC_SCRATCH_START(tmpNrg, FIXP_DBL, 32 * 64 / 2)

  /* Get max possible scaling of QMF data */
  scale = DFRACT_BITS;
  for (k = 0; k < numberCols; k++) {
    scale = fixMin(scale, fixMin(getScalefactor(realValues[k], numberBands),
                                 getScalefactor(imagValues[k], numberBands)));
  }

  /* Tweak scaling stability for zero signal to non-zero signal transitions */
  if (scale >= DFRACT_BITS - 1) {
    scale = (FRACT_BITS - 1 - *qmfScale);
  }
  /* prevent scaling of QMF values to -1.f */
  scale = fixMax(0, scale - 1);

  /* Update QMF scale */
  *qmfScale += scale;

  /*
     Calculate energy of each time slot pair, max energy
     and shift QMF values as far as possible to the left.
   */
  {
    FIXP_DBL *nrgValues = tmpNrg;
    for (k = 0; k < numberCols; k += 2) {
      /* Load band vector addresses of 2 consecutive timeslots */
      FIXP_DBL *RESTRICT r0 = realValues[k];
      FIXP_DBL *RESTRICT i0 = imagValues[k];
      FIXP_DBL *RESTRICT r1 = realValues[k + 1];
      FIXP_DBL *RESTRICT i1 = imagValues[k + 1];
      for (j = 0; j < numberBands; j++) {
        FIXP_DBL energy;
        FIXP_DBL tr0, tr1, ti0, ti1;

        /* Read QMF values of 2 timeslots */
        tr0 = r0[j];
        tr1 = r1[j];
        ti0 = i0[j];
        ti1 = i1[j];

        /* Scale QMF Values and Calc Energy average of both timeslots */
        tr0 <<= scale;
        ti0 <<= scale;
        energy = fPow2AddDiv2(fPow2Div2(tr0), ti0) >> 1;

        tr1 <<= scale;
        ti1 <<= scale;
        energy += fPow2AddDiv2(fPow2Div2(tr1), ti1) >> 1;

        /* Write timeslot pair energy to scratch */
        *nrgValues++ = energy;
        max_val = fixMax(max_val, energy);

        /* Write back scaled QMF values */
        r0[j] = tr0;
        r1[j] = tr1;
        i0[j] = ti0;
        i1[j] = ti1;
      }
    }
  }
  /* energyScale: scalefactor energies of current frame */
  *energyScale =
      2 * (*qmfScale) -
      1; /* if qmfScale > 0: nr of right shifts otherwise nr of left shifts */

  /* Scale timeslot pair energies and write to output buffer */
  scale = CountLeadingBits(max_val);
  {
    FIXP_DBL *nrgValues = tmpNrg;
    for (k = 0; k<numberCols>> 1; k++) {
      scaleValues(energyValues[k], nrgValues, numberBands, scale);
      nrgValues += numberBands;
    }
    *energyScale += scale;
  }

  /* Free Scratch buffer */
  C_ALLOC_SCRATCH_END(tmpNrg, FIXP_DBL, 32 * 64 / 2)
}

LNK_SECTION_CODE_L1
static void FDKsbrEnc_getEnergyFromCplxQmfDataFull(
    FIXP_DBL **RESTRICT energyValues, /*!< the result of the operation */
    FIXP_DBL **RESTRICT realValues, /*!< the real part of the QMF subsamples */
    FIXP_DBL **RESTRICT
        imagValues,   /*!< the imaginary part of the QMF subsamples */
    int numberBands,  /*!< number of QMF bands */
    int numberCols,   /*!< number of QMF subsamples */
    int *qmfScale,    /*!< scalefactor of QMF subsamples */
    int *energyScale) /*!< scalefactor of energies */
{
  int j, k;
  int scale;
  FIXP_DBL max_val = FL2FXCONST_DBL(0.0f);

  /* Get Scratch buffer */
  C_ALLOC_SCRATCH_START(tmpNrg, FIXP_DBL, MAX_NRG_SLOTS_LD * 64)

  FDK_ASSERT(numberCols <= MAX_NRG_SLOTS_LD);
  FDK_ASSERT(numberBands <= 64);

  /* Get max possible scaling of QMF data */
  scale = DFRACT_BITS;
  for (k = 0; k < numberCols; k++) {
    scale = fixMin(scale, fixMin(getScalefactor(realValues[k], numberBands),
                                 getScalefactor(imagValues[k], numberBands)));
  }

  /* Tweak scaling stability for zero signal to non-zero signal transitions */
  if (scale >= DFRACT_BITS - 1) {
    scale = (FRACT_BITS - 1 - *qmfScale);
  }
  /* prevent scaling of QFM values to -1.f */
  scale = fixMax(0, scale - 1);

  /* Update QMF scale */
  *qmfScale += scale;

  /*
     Calculate energy of each time slot pair, max energy
     and shift QMF values as far as possible to the left.
   */
  {
    FIXP_DBL *nrgValues = tmpNrg;
    for (k = 0; k < numberCols; k++) {
      /* Load band vector addresses of 1 timeslot */
      FIXP_DBL *RESTRICT r0 = realValues[k];
      FIXP_DBL *RESTRICT i0 = imagValues[k];
      for (j = 0; j < numberBands; j++) {
        FIXP_DBL energy;
        FIXP_DBL tr0, ti0;

        /* Read QMF values of 1 timeslot */
        tr0 = r0[j];
        ti0 = i0[j];

        /* Scale QMF Values and Calc Energy */
        tr0 <<= scale;
        ti0 <<= scale;
        energy = fPow2AddDiv2(fPow2Div2(tr0), ti0);
        *nrgValues++ = energy;

        max_val = fixMax(max_val, energy);

        /* Write back scaled QMF values */
        r0[j] = tr0;
        i0[j] = ti0;
      }
    }
  }
  /* energyScale: scalefactor energies of current frame */
  *energyScale =
      2 * (*qmfScale) -
      1; /* if qmfScale > 0: nr of right shifts otherwise nr of left shifts */

  /* Scale timeslot pair energies and write to output buffer */
  scale = CountLeadingBits(max_val);
  {
    FIXP_DBL *nrgValues = tmpNrg;
    for (k = 0; k < numberCols; k++) {
      scaleValues(energyValues[k], nrgValues, numberBands, scale);
      nrgValues += numberBands;
    }
    *energyScale += scale;
  }

  /* Free Scratch buffer */
  C_ALLOC_SCRATCH_END(tmpNrg, FIXP_DBL, MAX_NRG_SLOTS_LD * 64)
}

/***************************************************************************/
/*!

  \brief  Quantisation of the panorama value (balance)

  \return the quantized pan value

****************************************************************************/
static INT mapPanorama(INT nrgVal,     /*! integer value of the energy */
                       INT ampRes,     /*! amplitude resolution [1.5/3dB] */
                       INT *quantError /*! quantization error of energy val*/
) {
  int i;
  INT min_val, val;
  UCHAR panIndex;
  INT sign;

  sign = nrgVal > 0 ? 1 : -1;

  nrgVal *= sign;

  min_val = FDK_INT_MAX;
  panIndex = 0;
  for (i = 0; i < maxIndex[ampRes]; i++) {
    val = fixp_abs((nrgVal - (INT)panTable[ampRes][i]));

    if (val < min_val) {
      min_val = val;
      panIndex = i;
    }
  }

  *quantError = min_val;

  return panTable[ampRes][maxIndex[ampRes] - 1] +
         sign * panTable[ampRes][panIndex];
}

/***************************************************************************/
/*!

  \brief  Quantisation of the noise floor levels

  \return void

****************************************************************************/
static void sbrNoiseFloorLevelsQuantisation(
    SCHAR *RESTRICT iNoiseLevels, /*! quantized noise levels */
    FIXP_DBL *RESTRICT
        NoiseLevels, /*! the noise levels. Exponent = LD_DATA_SHIFT  */
    INT coupling     /*! the coupling flag */
) {
  INT i;
  INT tmp, dummy;

  /* Quantisation, similar to sfb quant... */
  for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
    /* tmp = NoiseLevels[i] > (PFLOAT)30.0f ? 30: (INT) (NoiseLevels[i] +
     * (PFLOAT)0.5); */
    /* 30>>LD_DATA_SHIFT = 0.46875 */
    if ((FIXP_DBL)NoiseLevels[i] > FL2FXCONST_DBL(0.46875f)) {
      tmp = 30;
    } else {
      /* tmp = (INT)((FIXP_DBL)NoiseLevels[i] + (FL2FXCONST_DBL(0.5f)>>(*/
      /* FRACT_BITS+ */                                 /* 6-1)));*/
      /* tmp = tmp >> (DFRACT_BITS-1-LD_DATA_SHIFT); */ /* conversion to integer
                                                           happens here */
      /* rounding is done by shifting one bit less than necessary to the right,
       * adding '1' and then shifting the final bit */
      tmp = ((((INT)NoiseLevels[i]) >>
              (DFRACT_BITS - 1 - LD_DATA_SHIFT))); /* conversion to integer */
      if (tmp != 0) tmp += 1;
    }

    if (coupling) {
      tmp = tmp < -30 ? -30 : tmp;
      tmp = mapPanorama(tmp, 1, &dummy);
    }
    iNoiseLevels[i] = tmp;
  }
}

/***************************************************************************/
/*!

  \brief  Calculation of noise floor for coupling

  \return void

****************************************************************************/
static void coupleNoiseFloor(
    FIXP_DBL *RESTRICT noise_level_left, /*! noise level left  (modified)*/
    FIXP_DBL *RESTRICT noise_level_right /*! noise level right (modified)*/
) {
  FIXP_DBL cmpValLeft, cmpValRight;
  INT i;
  FIXP_DBL temp1, temp2;

  for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
    /* Calculation of the power function using ld64:
       z  = x^y;
       z' = CalcLd64(z) = y*CalcLd64(x)/64;
       z  = CalcInvLd64(z');
    */
    cmpValLeft = NOISE_FLOOR_OFFSET_64 - noise_level_left[i];
    cmpValRight = NOISE_FLOOR_OFFSET_64 - noise_level_right[i];

    if (cmpValRight < FL2FXCONST_DBL(0.0f)) {
      temp1 = CalcInvLdData(NOISE_FLOOR_OFFSET_64 - noise_level_right[i]);
    } else {
      temp1 = CalcInvLdData(NOISE_FLOOR_OFFSET_64 - noise_level_right[i]);
      temp1 = temp1 << (DFRACT_BITS - 1 - LD_DATA_SHIFT -
                        1); /* INT to fract conversion of result, if input of
                               CalcInvLdData is positiv */
    }

    if (cmpValLeft < FL2FXCONST_DBL(0.0f)) {
      temp2 = CalcInvLdData(NOISE_FLOOR_OFFSET_64 - noise_level_left[i]);
    } else {
      temp2 = CalcInvLdData(NOISE_FLOOR_OFFSET_64 - noise_level_left[i]);
      temp2 = temp2 << (DFRACT_BITS - 1 - LD_DATA_SHIFT -
                        1); /* INT to fract conversion of result, if input of
                               CalcInvLdData is positiv */
    }

    if ((cmpValLeft < FL2FXCONST_DBL(0.0f)) &&
        (cmpValRight < FL2FXCONST_DBL(0.0f))) {
      noise_level_left[i] =
          NOISE_FLOOR_OFFSET_64 -
          (CalcLdData(
              ((temp1 >> 1) +
               (temp2 >> 1)))); /* no scaling needed! both values are dfract */
      noise_level_right[i] = CalcLdData(temp2) - CalcLdData(temp1);
    }

    if ((cmpValLeft >= FL2FXCONST_DBL(0.0f)) &&
        (cmpValRight >= FL2FXCONST_DBL(0.0f))) {
      noise_level_left[i] = NOISE_FLOOR_OFFSET_64 -
                            (CalcLdData(((temp1 >> 1) + (temp2 >> 1))) +
                             FL2FXCONST_DBL(0.109375f)); /* scaled with 7/64 */
      noise_level_right[i] = CalcLdData(temp2) - CalcLdData(temp1);
    }

    if ((cmpValLeft >= FL2FXCONST_DBL(0.0f)) &&
        (cmpValRight < FL2FXCONST_DBL(0.0f))) {
      noise_level_left[i] = NOISE_FLOOR_OFFSET_64 -
                            (CalcLdData(((temp1 >> (7 + 1)) + (temp2 >> 1))) +
                             FL2FXCONST_DBL(0.109375f)); /* scaled with 7/64 */
      noise_level_right[i] =
          (CalcLdData(temp2) + FL2FXCONST_DBL(0.109375f)) - CalcLdData(temp1);
    }

    if ((cmpValLeft < FL2FXCONST_DBL(0.0f)) &&
        (cmpValRight >= FL2FXCONST_DBL(0.0f))) {
      noise_level_left[i] = NOISE_FLOOR_OFFSET_64 -
                            (CalcLdData(((temp1 >> 1) + (temp2 >> (7 + 1)))) +
                             FL2FXCONST_DBL(0.109375f)); /* scaled with 7/64 */
      noise_level_right[i] = CalcLdData(temp2) -
                             (CalcLdData(temp1) +
                              FL2FXCONST_DBL(0.109375f)); /* scaled with 7/64 */
    }
  }
}

/***************************************************************************/
/*!

  \brief  Calculation of energy starting in lower band (li) up to upper band
(ui) over slots (start_pos) to (stop_pos)

  \return void

****************************************************************************/

static FIXP_DBL getEnvSfbEnergy(
    INT li,             /*! lower band */
    INT ui,             /*! upper band */
    INT start_pos,      /*! start slot */
    INT stop_pos,       /*! stop slot */
    INT border_pos,     /*! slots scaling border */
    FIXP_DBL **YBuffer, /*! sfb energy buffer */
    INT YBufferSzShift, /*! Energy buffer index scale */
    INT scaleNrg0,      /*! scaling of lower slots */
    INT scaleNrg1)      /*! scaling of upper slots */
{
  /* use dynamic scaling for outer energy loop;
     energies are critical and every bit is important */
  int sc0, sc1, k, l;

  FIXP_DBL nrgSum, nrg1, nrg2, accu1, accu2;
  INT dynScale, dynScale1, dynScale2;
  if (ui - li == 0)
    dynScale = DFRACT_BITS - 1;
  else
    dynScale = CalcLdInt(ui - li) >> (DFRACT_BITS - 1 - LD_DATA_SHIFT);

  sc0 = fixMin(scaleNrg0, Y_NRG_SCALE);
  sc1 = fixMin(scaleNrg1, Y_NRG_SCALE);
  /* dynScale{1,2} is set such that the right shift below is positive */
  dynScale1 = fixMin((scaleNrg0 - sc0), dynScale);
  dynScale2 = fixMin((scaleNrg1 - sc1), dynScale);
  nrgSum = accu1 = accu2 = (FIXP_DBL)0;

  for (k = li; k < ui; k++) {
    nrg1 = nrg2 = (FIXP_DBL)0;
    for (l = start_pos; l < border_pos; l++) {
      nrg1 += YBuffer[l >> YBufferSzShift][k] >> sc0;
    }
    for (; l < stop_pos; l++) {
      nrg2 += YBuffer[l >> YBufferSzShift][k] >> sc1;
    }
    accu1 = fAddSaturate(accu1, (nrg1 >> dynScale1));
    accu2 = fAddSaturate(accu2, (nrg2 >> dynScale2));
  }
  /* This shift factor is always positive. See comment above. */
  nrgSum +=
      (accu1 >> fixMin((scaleNrg0 - sc0 - dynScale1), (DFRACT_BITS - 1))) +
      (accu2 >> fixMin((scaleNrg1 - sc1 - dynScale2), (DFRACT_BITS - 1)));

  return nrgSum;
}

/***************************************************************************/
/*!

  \brief  Energy compensation in missing harmonic mode

  \return void

****************************************************************************/
static FIXP_DBL mhLoweringEnergy(FIXP_DBL nrg, INT M) {
  /*
     Compensating for the fact that we in the decoder map the "average energy to
     every QMF band, and use this when we calculate the boost-factor. Since the
     mapped energy isn't the average energy but the maximum energy in case of
     missing harmonic creation, we will in the boost function calculate that too
     much limiting has been applied and hence we will boost the signal although
     it isn't called for. Hence we need to compensate for this by lowering the
     transmitted energy values for the sines so they will get the correct level
     after the boost is applied.
  */
  if (M > 2) {
    INT tmpScale;
    tmpScale = CountLeadingBits(nrg);
    nrg <<= tmpScale;
    nrg = fMult(nrg, FL2FXCONST_DBL(0.398107267f)); /* The maximum boost
                                                       is 1.584893, so the
                                                       maximum attenuation
                                                       should be
                                                       square(1/1.584893) =
                                                       0.398107267 */
    nrg >>= tmpScale;
  } else {
    if (M > 1) {
      nrg >>= 1;
    }
  }

  return nrg;
}

/***************************************************************************/
/*!

  \brief  Energy compensation in none missing harmonic mode

  \return void

****************************************************************************/
static FIXP_DBL nmhLoweringEnergy(FIXP_DBL nrg, const FIXP_DBL nrgSum,
                                  const INT nrgSum_scale, const INT M) {
  if (nrg > FL2FXCONST_DBL(0)) {
    int sc = 0;
    /* gain = nrgSum / (nrg*(M+1)) */
    FIXP_DBL gain = fMult(fDivNorm(nrgSum, nrg, &sc), GetInvInt(M + 1));
    sc += nrgSum_scale;

    /* reduce nrg if gain smaller 1.f */
    if (!((sc >= 0) && (gain > ((FIXP_DBL)MAXVAL_DBL >> sc)))) {
      nrg = fMult(scaleValue(gain, sc), nrg);
    }
  }
  return nrg;
}

/***************************************************************************/
/*!

  \brief  calculates the envelope values from the energies, depending on
          framing and stereo mode

  \return void

****************************************************************************/
static void calculateSbrEnvelope(
    FIXP_DBL **RESTRICT YBufferLeft,  /*! energy buffer left */
    FIXP_DBL **RESTRICT YBufferRight, /*! energy buffer right */
    int *RESTRICT YBufferScaleLeft,   /*! scale energy buffer left */
    int *RESTRICT YBufferScaleRight,  /*! scale energy buffer right */
    const SBR_FRAME_INFO *frame_info, /*! frame info vector */
    SCHAR *RESTRICT sfb_nrgLeft,      /*! sfb energy buffer left */
    SCHAR *RESTRICT sfb_nrgRight,     /*! sfb energy buffer right */
    HANDLE_SBR_CONFIG_DATA h_con,     /*! handle to config data   */
    HANDLE_ENV_CHANNEL h_sbr,         /*! envelope channel handle */
    SBR_STEREO_MODE stereoMode,       /*! stereo coding mode */
    INT *maxQuantError, /*! maximum quantization error, for panorama. */
    int YBufferSzShift) /*! Energy buffer index scale */

{
  int env, j, m = 0;
  INT no_of_bands, start_pos, stop_pos, li, ui;
  FREQ_RES freq_res;

  INT ca = 2 - h_sbr->encEnvData.init_sbr_amp_res;
  INT oneBitLess = 0;
  if (ca == 2)
    oneBitLess =
        1; /* LD_DATA_SHIFT => ld64 scaling; one bit less for rounding */

  INT quantError;
  INT nEnvelopes = frame_info->nEnvelopes;
  INT short_env = frame_info->shortEnv - 1;
  INT timeStep = h_sbr->sbrExtractEnvelope.time_step;
  INT commonScale, scaleLeft0, scaleLeft1;
  INT scaleRight0 = 0, scaleRight1 = 0;

  commonScale = fixMin(YBufferScaleLeft[0], YBufferScaleLeft[1]);

  if (stereoMode == SBR_COUPLING) {
    commonScale = fixMin(commonScale, YBufferScaleRight[0]);
    commonScale = fixMin(commonScale, YBufferScaleRight[1]);
  }

  commonScale = commonScale - 7;

  scaleLeft0 = YBufferScaleLeft[0] - commonScale;
  scaleLeft1 = YBufferScaleLeft[1] - commonScale;
  FDK_ASSERT((scaleLeft0 >= 0) && (scaleLeft1 >= 0));

  if (stereoMode == SBR_COUPLING) {
    scaleRight0 = YBufferScaleRight[0] - commonScale;
    scaleRight1 = YBufferScaleRight[1] - commonScale;
    FDK_ASSERT((scaleRight0 >= 0) && (scaleRight1 >= 0));
    *maxQuantError = 0;
  }

  for (env = 0; env < nEnvelopes; env++) {
    FIXP_DBL pNrgLeft[32];
    FIXP_DBL pNrgRight[32];
    int envNrg_scale;
    FIXP_DBL envNrgLeft = FL2FXCONST_DBL(0.0f);
    FIXP_DBL envNrgRight = FL2FXCONST_DBL(0.0f);
    int missingHarmonic[32];
    int count[32];

    start_pos = timeStep * frame_info->borders[env];
    stop_pos = timeStep * frame_info->borders[env + 1];
    freq_res = frame_info->freqRes[env];
    no_of_bands = h_con->nSfb[freq_res];
    envNrg_scale = DFRACT_BITS - fNormz((FIXP_DBL)no_of_bands);
    if (env == short_env) {
      j = fMax(2, timeStep); /* consider at least 2 QMF slots less for short
                                envelopes (envelopes just before transients) */
      if ((stop_pos - start_pos - j) > 0) {
        stop_pos = stop_pos - j;
      }
    }
    for (j = 0; j < no_of_bands; j++) {
      FIXP_DBL nrgLeft = FL2FXCONST_DBL(0.0f);
      FIXP_DBL nrgRight = FL2FXCONST_DBL(0.0f);

      li = h_con->freqBandTable[freq_res][j];
      ui = h_con->freqBandTable[freq_res][j + 1];

      if (freq_res == FREQ_RES_HIGH) {
        if (j == 0 && ui - li > 1) {
          li++;
        }
      } else {
        if (j == 0 && ui - li > 2) {
          li++;
        }
      }

      /*
        Find out whether a sine will be missing in the scale-factor
        band that we're currently processing.
      */
      missingHarmonic[j] = 0;

      if (h_sbr->encEnvData.addHarmonicFlag) {
        if (freq_res == FREQ_RES_HIGH) {
          if (h_sbr->encEnvData
                  .addHarmonic[j]) { /*A missing sine in the current band*/
            missingHarmonic[j] = 1;
          }
        } else {
          INT i;
          INT startBandHigh = 0;
          INT stopBandHigh = 0;

          while (h_con->freqBandTable[FREQ_RES_HIGH][startBandHigh] <
                 h_con->freqBandTable[FREQ_RES_LOW][j])
            startBandHigh++;
          while (h_con->freqBandTable[FREQ_RES_HIGH][stopBandHigh] <
                 h_con->freqBandTable[FREQ_RES_LOW][j + 1])
            stopBandHigh++;

          for (i = startBandHigh; i < stopBandHigh; i++) {
            if (h_sbr->encEnvData.addHarmonic[i]) {
              missingHarmonic[j] = 1;
            }
          }
        }
      }

      /*
        If a sine is missing in a scalefactorband, with more than one qmf
        channel use the nrg from the channel with the largest nrg rather than
        the mean. Compensate for the boost calculation in the decdoder.
      */
      int border_pos =
          fixMin(stop_pos, h_sbr->sbrExtractEnvelope.YBufferWriteOffset
                               << YBufferSzShift);

      if (missingHarmonic[j]) {
        int k;
        count[j] = stop_pos - start_pos;
        nrgLeft = FL2FXCONST_DBL(0.0f);

        for (k = li; k < ui; k++) {
          FIXP_DBL tmpNrg;
          tmpNrg = getEnvSfbEnergy(k, k + 1, start_pos, stop_pos, border_pos,
                                   YBufferLeft, YBufferSzShift, scaleLeft0,
                                   scaleLeft1);

          nrgLeft = fixMax(nrgLeft, tmpNrg);
        }

        /* Energy lowering compensation */
        nrgLeft = mhLoweringEnergy(nrgLeft, ui - li);

        if (stereoMode == SBR_COUPLING) {
          nrgRight = FL2FXCONST_DBL(0.0f);

          for (k = li; k < ui; k++) {
            FIXP_DBL tmpNrg;
            tmpNrg = getEnvSfbEnergy(k, k + 1, start_pos, stop_pos, border_pos,
                                     YBufferRight, YBufferSzShift, scaleRight0,
                                     scaleRight1);

            nrgRight = fixMax(nrgRight, tmpNrg);
          }

          /* Energy lowering compensation */
          nrgRight = mhLoweringEnergy(nrgRight, ui - li);
        }
      } /* end missingHarmonic */
      else {
        count[j] = (stop_pos - start_pos) * (ui - li);

        nrgLeft = getEnvSfbEnergy(li, ui, start_pos, stop_pos, border_pos,
                                  YBufferLeft, YBufferSzShift, scaleLeft0,
                                  scaleLeft1);

        if (stereoMode == SBR_COUPLING) {
          nrgRight = getEnvSfbEnergy(li, ui, start_pos, stop_pos, border_pos,
                                     YBufferRight, YBufferSzShift, scaleRight0,
                                     scaleRight1);
        }
      } /* !missingHarmonic */

      /* save energies */
      pNrgLeft[j] = nrgLeft;
      pNrgRight[j] = nrgRight;
      envNrgLeft += (nrgLeft >> envNrg_scale);
      envNrgRight += (nrgRight >> envNrg_scale);
    } /* j */

    for (j = 0; j < no_of_bands; j++) {
      FIXP_DBL nrgLeft2 = FL2FXCONST_DBL(0.0f);
      FIXP_DBL nrgLeft = pNrgLeft[j];
      FIXP_DBL nrgRight = pNrgRight[j];

      /* None missing harmonic Energy lowering compensation */
      if (!missingHarmonic[j] && h_sbr->fLevelProtect) {
        /* in case of missing energy in base band,
           reduce reference energy to prevent overflows in decoder output */
        nrgLeft =
            nmhLoweringEnergy(nrgLeft, envNrgLeft, envNrg_scale, no_of_bands);
        if (stereoMode == SBR_COUPLING) {
          nrgRight = nmhLoweringEnergy(nrgRight, envNrgRight, envNrg_scale,
                                       no_of_bands);
        }
      }

      if (stereoMode == SBR_COUPLING) {
        /* calc operation later with log */
        nrgLeft2 = nrgLeft;
        nrgLeft = (nrgRight + nrgLeft) >> 1;
      }

      /* nrgLeft = f20_log2(nrgLeft / (PFLOAT)(count * 64))+(PFLOAT)44; */
      /* If nrgLeft == 0 then the Log calculations below do fail. */
      if (nrgLeft > FL2FXCONST_DBL(0.0f)) {
        FIXP_DBL tmp0, tmp1, tmp2, tmp3;
        INT tmpScale;

        tmpScale = CountLeadingBits(nrgLeft);
        nrgLeft = nrgLeft << tmpScale;

        tmp0 = CalcLdData(nrgLeft); /* scaled by 1/64 */
        tmp1 = ((FIXP_DBL)(commonScale + tmpScale))
               << (DFRACT_BITS - 1 - LD_DATA_SHIFT - 1); /* scaled by 1/64 */
        tmp2 = ((FIXP_DBL)(count[j] * 64)) << (DFRACT_BITS - 1 - 14 - 1);
        tmp2 = CalcLdData(tmp2); /* scaled by 1/64 */
        tmp3 = FL2FXCONST_DBL(0.6875f - 0.21875f - 0.015625f) >>
               1; /* scaled by 1/64 */

        nrgLeft = ((tmp0 - tmp2) >> 1) + (tmp3 - tmp1);
      } else {
        nrgLeft = FL2FXCONST_DBL(-1.0f);
      }

      /* ld64 to integer conversion */
      nrgLeft = fixMin(fixMax(nrgLeft, FL2FXCONST_DBL(0.0f)),
                       (FL2FXCONST_DBL(0.5f) >> oneBitLess));
      nrgLeft = (FIXP_DBL)(LONG)nrgLeft >>
                (DFRACT_BITS - 1 - LD_DATA_SHIFT - 1 - oneBitLess - 1);
      sfb_nrgLeft[m] = ((INT)nrgLeft + 1) >> 1; /* rounding */

      if (stereoMode == SBR_COUPLING) {
        FIXP_DBL scaleFract;
        int sc0, sc1;

        nrgLeft2 = fixMax((FIXP_DBL)0x1, nrgLeft2);
        nrgRight = fixMax((FIXP_DBL)0x1, nrgRight);

        sc0 = CountLeadingBits(nrgLeft2);
        sc1 = CountLeadingBits(nrgRight);

        scaleFract =
            ((FIXP_DBL)(sc0 - sc1))
            << (DFRACT_BITS - 1 -
                LD_DATA_SHIFT); /* scale value in ld64 representation */
        nrgRight = CalcLdData(nrgLeft2 << sc0) - CalcLdData(nrgRight << sc1) -
                   scaleFract;

        /* ld64 to integer conversion */
        nrgRight = (FIXP_DBL)(LONG)(nrgRight) >>
                   (DFRACT_BITS - 1 - LD_DATA_SHIFT - 1 - oneBitLess);
        nrgRight = (nrgRight + (FIXP_DBL)1) >> 1; /* rounding */

        sfb_nrgRight[m] = mapPanorama(
            nrgRight, h_sbr->encEnvData.init_sbr_amp_res, &quantError);

        *maxQuantError = fixMax(quantError, *maxQuantError);
      }

      m++;
    } /* j */

    /* Do energy compensation for sines that are present in two
        QMF-bands in the original, but will only occur in one band in
        the decoder due to the synthetic sine coding.*/
    if (h_con->useParametricCoding) {
      m -= no_of_bands;
      for (j = 0; j < no_of_bands; j++) {
        if (freq_res == FREQ_RES_HIGH &&
            h_sbr->sbrExtractEnvelope.envelopeCompensation[j]) {
          sfb_nrgLeft[m] -=
              (ca *
               fixp_abs(
                   (INT)h_sbr->sbrExtractEnvelope.envelopeCompensation[j]));
        }
        sfb_nrgLeft[m] = fixMax(0, sfb_nrgLeft[m]);
        m++;
      }
    } /* useParametricCoding */

  } /* env loop */
}

/***************************************************************************/
/*!

  \brief  calculates the noise floor and the envelope values from the
          energies, depending on framing and stereo mode

  FDKsbrEnc_extractSbrEnvelope is the main function for encoding and writing the
  envelope and the noise floor. The function includes the following processes:

  -Analysis subband filtering.
  -Encoding SA and pan parameters (if enabled).
  -Transient detection.

****************************************************************************/

LNK_SECTION_CODE_L1
void FDKsbrEnc_extractSbrEnvelope1(
    HANDLE_SBR_CONFIG_DATA h_con, /*! handle to config data   */
    HANDLE_SBR_HEADER_DATA sbrHeaderData,
    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData, HANDLE_ENV_CHANNEL hEnvChan,
    HANDLE_COMMON_DATA hCmonData, SBR_ENV_TEMP_DATA *eData,
    SBR_FRAME_TEMP_DATA *fData) {
  HANDLE_SBR_EXTRACT_ENVELOPE sbrExtrEnv = &hEnvChan->sbrExtractEnvelope;

  if (sbrExtrEnv->YBufferSzShift == 0)
    FDKsbrEnc_getEnergyFromCplxQmfDataFull(
        &sbrExtrEnv->YBuffer[sbrExtrEnv->YBufferWriteOffset],
        sbrExtrEnv->rBuffer + sbrExtrEnv->rBufferReadOffset,
        sbrExtrEnv->iBuffer + sbrExtrEnv->rBufferReadOffset, h_con->noQmfBands,
        sbrExtrEnv->no_cols, &hEnvChan->qmfScale, &sbrExtrEnv->YBufferScale[1]);
  else
    FDKsbrEnc_getEnergyFromCplxQmfData(
        &sbrExtrEnv->YBuffer[sbrExtrEnv->YBufferWriteOffset],
        sbrExtrEnv->rBuffer + sbrExtrEnv->rBufferReadOffset,
        sbrExtrEnv->iBuffer + sbrExtrEnv->rBufferReadOffset, h_con->noQmfBands,
        sbrExtrEnv->no_cols, &hEnvChan->qmfScale, &sbrExtrEnv->YBufferScale[1]);

  /* Energie values =
   * sbrExtrEnv->YBuffer[sbrExtrEnv->YBufferWriteOffset][x].floatVal *
   * (1<<2*7-sbrExtrEnv->YBufferScale[1]) */

  /*
    Precalculation of Tonality Quotas  COEFF Transform OK
  */
  FDKsbrEnc_CalculateTonalityQuotas(
      &hEnvChan->TonCorr, sbrExtrEnv->rBuffer, sbrExtrEnv->iBuffer,
      h_con->freqBandTable[HI][h_con->nSfb[HI]], hEnvChan->qmfScale);

  if (h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    FIXP_DBL tonality = FDKsbrEnc_GetTonality(
        hEnvChan->TonCorr.quotaMatrix,
        hEnvChan->TonCorr.numberOfEstimatesPerFrame,
        hEnvChan->TonCorr.startIndexMatrix,
        sbrExtrEnv->YBuffer + sbrExtrEnv->YBufferWriteOffset,
        h_con->freqBandTable[HI][0] + 1, h_con->noQmfBands,
        sbrExtrEnv->no_cols);

    hEnvChan->encEnvData.ton_HF[1] = hEnvChan->encEnvData.ton_HF[0];
    hEnvChan->encEnvData.ton_HF[0] = tonality;

    /* tonality is scaled by 2^19/0.524288f (fract part of RELAXATION) */
    hEnvChan->encEnvData.global_tonality =
        (hEnvChan->encEnvData.ton_HF[0] >> 1) +
        (hEnvChan->encEnvData.ton_HF[1] >> 1);
  }

  /*
    Transient detection COEFF Transform OK
  */

  if (h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    FDKsbrEnc_fastTransientDetect(&hEnvChan->sbrFastTransientDetector,
                                  sbrExtrEnv->YBuffer, sbrExtrEnv->YBufferScale,
                                  sbrExtrEnv->YBufferWriteOffset,
                                  eData->transient_info);

  } else {
    FDKsbrEnc_transientDetect(
        &hEnvChan->sbrTransientDetector, sbrExtrEnv->YBuffer,
        sbrExtrEnv->YBufferScale, eData->transient_info,
        sbrExtrEnv->YBufferWriteOffset, sbrExtrEnv->YBufferSzShift,
        sbrExtrEnv->time_step, hEnvChan->SbrEnvFrame.frameMiddleSlot);
  }

  /*
    Generate flags for 2 env in a FIXFIX-frame.
    Remove this function to get always 1 env per FIXFIX-frame.
  */

  /*
    frame Splitter COEFF Transform OK
  */
  FDKsbrEnc_frameSplitter(
      sbrExtrEnv->YBuffer, sbrExtrEnv->YBufferScale,
      &hEnvChan->sbrTransientDetector, h_con->freqBandTable[1],
      eData->transient_info, sbrExtrEnv->YBufferWriteOffset,
      sbrExtrEnv->YBufferSzShift, h_con->nSfb[1], sbrExtrEnv->time_step,
      sbrExtrEnv->no_cols, &hEnvChan->encEnvData.global_tonality);
}

/***************************************************************************/
/*!

  \brief  calculates the noise floor and the envelope values from the
          energies, depending on framing and stereo mode

  FDKsbrEnc_extractSbrEnvelope is the main function for encoding and writing the
  envelope and the noise floor. The function includes the following processes:

  -Determine time/frequency division of current granule.
  -Sending transient info to bitstream.
  -Set amp_res to 1.5 dB if the current frame contains only one envelope.
  -Lock dynamic bandwidth frequency change if the next envelope not starts on a
  frame boundary.
  -MDCT transposer (needed to detect where harmonics will be missing).
  -Spectrum Estimation (used for pulse train and missing harmonics detection).
  -Pulse train detection.
  -Inverse Filtering detection.
  -Waveform Coding.
  -Missing Harmonics detection.
  -Extract envelope of current frame.
  -Noise floor estimation.
  -Noise floor quantisation and coding.
  -Encode envelope of current frame.
  -Send the encoded data to the bitstream.
  -Write to bitstream.

****************************************************************************/

LNK_SECTION_CODE_L1
void FDKsbrEnc_extractSbrEnvelope2(
    HANDLE_SBR_CONFIG_DATA h_con, /*! handle to config data   */
    HANDLE_SBR_HEADER_DATA sbrHeaderData,
    HANDLE_PARAMETRIC_STEREO hParametricStereo,
    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData, HANDLE_ENV_CHANNEL h_envChan0,
    HANDLE_ENV_CHANNEL h_envChan1, HANDLE_COMMON_DATA hCmonData,
    SBR_ENV_TEMP_DATA *eData, SBR_FRAME_TEMP_DATA *fData, int clearOutput) {
  HANDLE_ENV_CHANNEL h_envChan[MAX_NUM_CHANNELS] = {h_envChan0, h_envChan1};
  int ch, i, j, c, YSzShift = h_envChan[0]->sbrExtractEnvelope.YBufferSzShift;

  SBR_STEREO_MODE stereoMode = h_con->stereoMode;
  int nChannels = h_con->nChannels;
  const int *v_tuning;
  static const int v_tuningHEAAC[6] = {0, 2, 4, 0, 0, 0};

  static const int v_tuningELD[6] = {0, 2, 3, 0, 0, 0};

  if (h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY)
    v_tuning = v_tuningELD;
  else
    v_tuning = v_tuningHEAAC;

  /*
    Select stereo mode.
  */
  if (stereoMode == SBR_COUPLING) {
    if (eData[0].transient_info[1] && eData[1].transient_info[1]) {
      eData[0].transient_info[0] =
          fixMin(eData[1].transient_info[0], eData[0].transient_info[0]);
      eData[1].transient_info[0] = eData[0].transient_info[0];
    } else {
      if (eData[0].transient_info[1] && !eData[1].transient_info[1]) {
        eData[1].transient_info[0] = eData[0].transient_info[0];
      } else {
        if (!eData[0].transient_info[1] && eData[1].transient_info[1])
          eData[0].transient_info[0] = eData[1].transient_info[0];
        else {
          eData[0].transient_info[0] =
              fixMax(eData[1].transient_info[0], eData[0].transient_info[0]);
          eData[1].transient_info[0] = eData[0].transient_info[0];
        }
      }
    }
  }

  /*
    Determine time/frequency division of current granule
  */
  eData[0].frame_info = FDKsbrEnc_frameInfoGenerator(
      &h_envChan[0]->SbrEnvFrame, eData[0].transient_info,
      sbrBitstreamData->rightBorderFIX,
      h_envChan[0]->sbrExtractEnvelope.pre_transient_info,
      h_envChan[0]->encEnvData.ldGrid, v_tuning);

  h_envChan[0]->encEnvData.hSbrBSGrid = &h_envChan[0]->SbrEnvFrame.SbrGrid;

  /* AAC LD patch for transient prediction */
  if (h_envChan[0]->encEnvData.ldGrid && eData[0].transient_info[2]) {
    /* if next frame will start with transient, set shortEnv to
     * numEnvelopes(shortend Envelope = shortEnv-1)*/
    h_envChan[0]->SbrEnvFrame.SbrFrameInfo.shortEnv =
        h_envChan[0]->SbrEnvFrame.SbrFrameInfo.nEnvelopes;
  }

  switch (stereoMode) {
    case SBR_LEFT_RIGHT:
    case SBR_SWITCH_LRC:
      eData[1].frame_info = FDKsbrEnc_frameInfoGenerator(
          &h_envChan[1]->SbrEnvFrame, eData[1].transient_info,
          sbrBitstreamData->rightBorderFIX,
          h_envChan[1]->sbrExtractEnvelope.pre_transient_info,
          h_envChan[1]->encEnvData.ldGrid, v_tuning);

      h_envChan[1]->encEnvData.hSbrBSGrid = &h_envChan[1]->SbrEnvFrame.SbrGrid;

      if (h_envChan[1]->encEnvData.ldGrid && eData[1].transient_info[2]) {
        /* if next frame will start with transient, set shortEnv to
         * numEnvelopes(shortend Envelope = shortEnv-1)*/
        h_envChan[1]->SbrEnvFrame.SbrFrameInfo.shortEnv =
            h_envChan[1]->SbrEnvFrame.SbrFrameInfo.nEnvelopes;
      }

      /* compare left and right frame_infos */
      if (eData[0].frame_info->nEnvelopes != eData[1].frame_info->nEnvelopes) {
        stereoMode = SBR_LEFT_RIGHT;
      } else {
        for (i = 0; i < eData[0].frame_info->nEnvelopes + 1; i++) {
          if (eData[0].frame_info->borders[i] !=
              eData[1].frame_info->borders[i]) {
            stereoMode = SBR_LEFT_RIGHT;
            break;
          }
        }
        for (i = 0; i < eData[0].frame_info->nEnvelopes; i++) {
          if (eData[0].frame_info->freqRes[i] !=
              eData[1].frame_info->freqRes[i]) {
            stereoMode = SBR_LEFT_RIGHT;
            break;
          }
        }
        if (eData[0].frame_info->shortEnv != eData[1].frame_info->shortEnv) {
          stereoMode = SBR_LEFT_RIGHT;
        }
      }
      break;
    case SBR_COUPLING:
      eData[1].frame_info = eData[0].frame_info;
      h_envChan[1]->encEnvData.hSbrBSGrid = &h_envChan[0]->SbrEnvFrame.SbrGrid;
      break;
    case SBR_MONO:
      /* nothing to do */
      break;
    default:
      FDK_ASSERT(0);
  }

  for (ch = 0; ch < nChannels; ch++) {
    HANDLE_ENV_CHANNEL hEnvChan = h_envChan[ch];
    HANDLE_SBR_EXTRACT_ENVELOPE sbrExtrEnv = &hEnvChan->sbrExtractEnvelope;
    SBR_ENV_TEMP_DATA *ed = &eData[ch];

    /*
       Send transient info to bitstream and store for next call
    */
    sbrExtrEnv->pre_transient_info[0] = ed->transient_info[0]; /* tran_pos */
    sbrExtrEnv->pre_transient_info[1] = ed->transient_info[1]; /* tran_flag */
    hEnvChan->encEnvData.noOfEnvelopes = ed->nEnvelopes =
        ed->frame_info->nEnvelopes; /* number of envelopes of current frame */
    hEnvChan->encEnvData.currentAmpResFF = (AMP_RES)h_con->initAmpResFF;

    /*
      Check if the current frame is divided into one envelope only. If so, set
      the amplitude resolution to 1.5 dB, otherwise may set back to chosen value
    */
    if ((hEnvChan->encEnvData.hSbrBSGrid->frameClass == FIXFIX) &&
        (ed->nEnvelopes == 1)) {
      AMP_RES currentAmpResFF = SBR_AMP_RES_1_5;
      if (h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
        /* Note: global_tonality_float_value ==
           ((float)hEnvChan->encEnvData.global_tonality/((INT64)(1)<<(31-(19+2)))/0.524288*(2.0/3.0)));
                 threshold_float_value ==
           ((float)h_con->thresholdAmpResFF_m/((INT64)(1)<<(31-(h_con->thresholdAmpResFF_e)))/0.524288*(2.0/3.0)));
         */
        /* decision of SBR_AMP_RES */
        if (fIsLessThan(/* global_tonality > threshold ? */
                        h_con->thresholdAmpResFF_m, h_con->thresholdAmpResFF_e,
                        hEnvChan->encEnvData.global_tonality,
                        RELAXATION_SHIFT + 2)) {
          hEnvChan->encEnvData.currentAmpResFF = SBR_AMP_RES_1_5;
        } else {
          hEnvChan->encEnvData.currentAmpResFF = SBR_AMP_RES_3_0;
        }
        currentAmpResFF = hEnvChan->encEnvData.currentAmpResFF;
      }

      if (currentAmpResFF != hEnvChan->encEnvData.init_sbr_amp_res) {
        FDKsbrEnc_InitSbrHuffmanTables(
            &hEnvChan->encEnvData, &hEnvChan->sbrCodeEnvelope,
            &hEnvChan->sbrCodeNoiseFloor, currentAmpResFF);
      }
    } else {
      if (sbrHeaderData->sbr_amp_res != hEnvChan->encEnvData.init_sbr_amp_res) {
        FDKsbrEnc_InitSbrHuffmanTables(
            &hEnvChan->encEnvData, &hEnvChan->sbrCodeEnvelope,
            &hEnvChan->sbrCodeNoiseFloor, sbrHeaderData->sbr_amp_res);
      }
    }

    if (!clearOutput) {
      /*
        Tonality correction parameter extraction (inverse filtering level, noise
        floor additional sines).
      */
      FDKsbrEnc_TonCorrParamExtr(
          &hEnvChan->TonCorr, hEnvChan->encEnvData.sbr_invf_mode_vec,
          ed->noiseFloor, &hEnvChan->encEnvData.addHarmonicFlag,
          hEnvChan->encEnvData.addHarmonic, sbrExtrEnv->envelopeCompensation,
          ed->frame_info, ed->transient_info, h_con->freqBandTable[HI],
          h_con->nSfb[HI], hEnvChan->encEnvData.sbr_xpos_mode,
          h_con->sbrSyntaxFlags);
    }

    /* Low energy in low band fix */
    if (hEnvChan->sbrTransientDetector.prevLowBandEnergy <
            hEnvChan->sbrTransientDetector.prevHighBandEnergy &&
        hEnvChan->sbrTransientDetector.prevHighBandEnergy > FL2FX_DBL(0.03)
        /* The fix needs the non-fast transient detector running.
           It sets prevLowBandEnergy and prevHighBandEnergy.      */
        && !(h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY)) {
      hEnvChan->fLevelProtect = 1;

      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        hEnvChan->encEnvData.sbr_invf_mode_vec[i] = INVF_HIGH_LEVEL;
    } else {
      hEnvChan->fLevelProtect = 0;
    }

    hEnvChan->encEnvData.sbr_invf_mode =
        hEnvChan->encEnvData.sbr_invf_mode_vec[0];

    hEnvChan->encEnvData.noOfnoisebands =
        hEnvChan->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;

  } /* ch */

  /*
     Save number of scf bands per envelope
   */
  for (ch = 0; ch < nChannels; ch++) {
    for (i = 0; i < eData[ch].nEnvelopes; i++) {
      h_envChan[ch]->encEnvData.noScfBands[i] =
          (eData[ch].frame_info->freqRes[i] == FREQ_RES_HIGH
               ? h_con->nSfb[FREQ_RES_HIGH]
               : h_con->nSfb[FREQ_RES_LOW]);
    }
  }

  if (h_con->sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY &&
      stereoMode == SBR_SWITCH_LRC &&
      h_envChan[0]->encEnvData.currentAmpResFF !=
          h_envChan[1]->encEnvData.currentAmpResFF) {
    stereoMode = SBR_LEFT_RIGHT;
  }

  /*
    Extract envelope of current frame.
  */
  switch (stereoMode) {
    case SBR_MONO:
      calculateSbrEnvelope(h_envChan[0]->sbrExtractEnvelope.YBuffer, NULL,
                           h_envChan[0]->sbrExtractEnvelope.YBufferScale, NULL,
                           eData[0].frame_info, eData[0].sfb_nrg, NULL, h_con,
                           h_envChan[0], SBR_MONO, NULL, YSzShift);
      break;
    case SBR_LEFT_RIGHT:
      calculateSbrEnvelope(h_envChan[0]->sbrExtractEnvelope.YBuffer, NULL,
                           h_envChan[0]->sbrExtractEnvelope.YBufferScale, NULL,
                           eData[0].frame_info, eData[0].sfb_nrg, NULL, h_con,
                           h_envChan[0], SBR_MONO, NULL, YSzShift);
      calculateSbrEnvelope(h_envChan[1]->sbrExtractEnvelope.YBuffer, NULL,
                           h_envChan[1]->sbrExtractEnvelope.YBufferScale, NULL,
                           eData[1].frame_info, eData[1].sfb_nrg, NULL, h_con,
                           h_envChan[1], SBR_MONO, NULL, YSzShift);
      break;
    case SBR_COUPLING:
      calculateSbrEnvelope(h_envChan[0]->sbrExtractEnvelope.YBuffer,
                           h_envChan[1]->sbrExtractEnvelope.YBuffer,
                           h_envChan[0]->sbrExtractEnvelope.YBufferScale,
                           h_envChan[1]->sbrExtractEnvelope.YBufferScale,
                           eData[0].frame_info, eData[0].sfb_nrg,
                           eData[1].sfb_nrg, h_con, h_envChan[0], SBR_COUPLING,
                           &fData->maxQuantError, YSzShift);
      break;
    case SBR_SWITCH_LRC:
      calculateSbrEnvelope(h_envChan[0]->sbrExtractEnvelope.YBuffer, NULL,
                           h_envChan[0]->sbrExtractEnvelope.YBufferScale, NULL,
                           eData[0].frame_info, eData[0].sfb_nrg, NULL, h_con,
                           h_envChan[0], SBR_MONO, NULL, YSzShift);
      calculateSbrEnvelope(h_envChan[1]->sbrExtractEnvelope.YBuffer, NULL,
                           h_envChan[1]->sbrExtractEnvelope.YBufferScale, NULL,
                           eData[1].frame_info, eData[1].sfb_nrg, NULL, h_con,
                           h_envChan[1], SBR_MONO, NULL, YSzShift);
      calculateSbrEnvelope(h_envChan[0]->sbrExtractEnvelope.YBuffer,
                           h_envChan[1]->sbrExtractEnvelope.YBuffer,
                           h_envChan[0]->sbrExtractEnvelope.YBufferScale,
                           h_envChan[1]->sbrExtractEnvelope.YBufferScale,
                           eData[0].frame_info, eData[0].sfb_nrg_coupling,
                           eData[1].sfb_nrg_coupling, h_con, h_envChan[0],
                           SBR_COUPLING, &fData->maxQuantError, YSzShift);
      break;
  }

  /*
    Noise floor quantisation and coding.
  */

  switch (stereoMode) {
    case SBR_MONO:
      sbrNoiseFloorLevelsQuantisation(eData[0].noise_level, eData[0].noiseFloor,
                                      0);

      FDKsbrEnc_codeEnvelope(eData[0].noise_level, fData->res,
                             &h_envChan[0]->sbrCodeNoiseFloor,
                             h_envChan[0]->encEnvData.domain_vec_noise, 0,
                             (eData[0].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      break;
    case SBR_LEFT_RIGHT:
      sbrNoiseFloorLevelsQuantisation(eData[0].noise_level, eData[0].noiseFloor,
                                      0);

      FDKsbrEnc_codeEnvelope(eData[0].noise_level, fData->res,
                             &h_envChan[0]->sbrCodeNoiseFloor,
                             h_envChan[0]->encEnvData.domain_vec_noise, 0,
                             (eData[0].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      sbrNoiseFloorLevelsQuantisation(eData[1].noise_level, eData[1].noiseFloor,
                                      0);

      FDKsbrEnc_codeEnvelope(eData[1].noise_level, fData->res,
                             &h_envChan[1]->sbrCodeNoiseFloor,
                             h_envChan[1]->encEnvData.domain_vec_noise, 0,
                             (eData[1].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      break;

    case SBR_COUPLING:
      coupleNoiseFloor(eData[0].noiseFloor, eData[1].noiseFloor);

      sbrNoiseFloorLevelsQuantisation(eData[0].noise_level, eData[0].noiseFloor,
                                      0);

      FDKsbrEnc_codeEnvelope(eData[0].noise_level, fData->res,
                             &h_envChan[0]->sbrCodeNoiseFloor,
                             h_envChan[0]->encEnvData.domain_vec_noise, 1,
                             (eData[0].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      sbrNoiseFloorLevelsQuantisation(eData[1].noise_level, eData[1].noiseFloor,
                                      1);

      FDKsbrEnc_codeEnvelope(eData[1].noise_level, fData->res,
                             &h_envChan[1]->sbrCodeNoiseFloor,
                             h_envChan[1]->encEnvData.domain_vec_noise, 1,
                             (eData[1].frame_info->nEnvelopes > 1 ? 2 : 1), 1,
                             sbrBitstreamData->HeaderActive);

      break;
    case SBR_SWITCH_LRC:
      sbrNoiseFloorLevelsQuantisation(eData[0].noise_level, eData[0].noiseFloor,
                                      0);
      sbrNoiseFloorLevelsQuantisation(eData[1].noise_level, eData[1].noiseFloor,
                                      0);
      coupleNoiseFloor(eData[0].noiseFloor, eData[1].noiseFloor);
      sbrNoiseFloorLevelsQuantisation(eData[0].noise_level_coupling,
                                      eData[0].noiseFloor, 0);
      sbrNoiseFloorLevelsQuantisation(eData[1].noise_level_coupling,
                                      eData[1].noiseFloor, 1);
      break;
  }

  /*
    Encode envelope of current frame.
  */
  switch (stereoMode) {
    case SBR_MONO:
      sbrHeaderData->coupling = 0;
      h_envChan[0]->encEnvData.balance = 0;
      FDKsbrEnc_codeEnvelope(
          eData[0].sfb_nrg, eData[0].frame_info->freqRes,
          &h_envChan[0]->sbrCodeEnvelope, h_envChan[0]->encEnvData.domain_vec,
          sbrHeaderData->coupling, eData[0].frame_info->nEnvelopes, 0,
          sbrBitstreamData->HeaderActive);
      break;
    case SBR_LEFT_RIGHT:
      sbrHeaderData->coupling = 0;

      h_envChan[0]->encEnvData.balance = 0;
      h_envChan[1]->encEnvData.balance = 0;

      FDKsbrEnc_codeEnvelope(
          eData[0].sfb_nrg, eData[0].frame_info->freqRes,
          &h_envChan[0]->sbrCodeEnvelope, h_envChan[0]->encEnvData.domain_vec,
          sbrHeaderData->coupling, eData[0].frame_info->nEnvelopes, 0,
          sbrBitstreamData->HeaderActive);
      FDKsbrEnc_codeEnvelope(
          eData[1].sfb_nrg, eData[1].frame_info->freqRes,
          &h_envChan[1]->sbrCodeEnvelope, h_envChan[1]->encEnvData.domain_vec,
          sbrHeaderData->coupling, eData[1].frame_info->nEnvelopes, 0,
          sbrBitstreamData->HeaderActive);
      break;
    case SBR_COUPLING:
      sbrHeaderData->coupling = 1;
      h_envChan[0]->encEnvData.balance = 0;
      h_envChan[1]->encEnvData.balance = 1;

      FDKsbrEnc_codeEnvelope(
          eData[0].sfb_nrg, eData[0].frame_info->freqRes,
          &h_envChan[0]->sbrCodeEnvelope, h_envChan[0]->encEnvData.domain_vec,
          sbrHeaderData->coupling, eData[0].frame_info->nEnvelopes, 0,
          sbrBitstreamData->HeaderActive);
      FDKsbrEnc_codeEnvelope(
          eData[1].sfb_nrg, eData[1].frame_info->freqRes,
          &h_envChan[1]->sbrCodeEnvelope, h_envChan[1]->encEnvData.domain_vec,
          sbrHeaderData->coupling, eData[1].frame_info->nEnvelopes, 1,
          sbrBitstreamData->HeaderActive);
      break;
    case SBR_SWITCH_LRC: {
      INT payloadbitsLR;
      INT payloadbitsCOUPLING;

      SCHAR sfbNrgPrevTemp[MAX_NUM_CHANNELS][MAX_FREQ_COEFFS];
      SCHAR noisePrevTemp[MAX_NUM_CHANNELS][MAX_NUM_NOISE_COEFFS];
      INT upDateNrgTemp[MAX_NUM_CHANNELS];
      INT upDateNoiseTemp[MAX_NUM_CHANNELS];
      INT domainVecTemp[MAX_NUM_CHANNELS][MAX_ENVELOPES];
      INT domainVecNoiseTemp[MAX_NUM_CHANNELS][MAX_ENVELOPES];

      INT tempFlagRight = 0;
      INT tempFlagLeft = 0;

      /*
         Store previous values, in order to be able to "undo" what is being
         done.
      */

      for (ch = 0; ch < nChannels; ch++) {
        FDKmemcpy(sfbNrgPrevTemp[ch],
                  h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev,
                  MAX_FREQ_COEFFS * sizeof(SCHAR));

        FDKmemcpy(noisePrevTemp[ch],
                  h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev,
                  MAX_NUM_NOISE_COEFFS * sizeof(SCHAR));

        upDateNrgTemp[ch] = h_envChan[ch]->sbrCodeEnvelope.upDate;
        upDateNoiseTemp[ch] = h_envChan[ch]->sbrCodeNoiseFloor.upDate;

        /*
          forbid time coding in the first envelope in case of a different
          previous stereomode
        */
        if (sbrHeaderData->prev_coupling) {
          h_envChan[ch]->sbrCodeEnvelope.upDate = 0;
          h_envChan[ch]->sbrCodeNoiseFloor.upDate = 0;
        }
      } /* ch */

      /*
         Code ordinary Left/Right stereo
      */
      FDKsbrEnc_codeEnvelope(eData[0].sfb_nrg, eData[0].frame_info->freqRes,
                             &h_envChan[0]->sbrCodeEnvelope,
                             h_envChan[0]->encEnvData.domain_vec, 0,
                             eData[0].frame_info->nEnvelopes, 0,
                             sbrBitstreamData->HeaderActive);
      FDKsbrEnc_codeEnvelope(eData[1].sfb_nrg, eData[1].frame_info->freqRes,
                             &h_envChan[1]->sbrCodeEnvelope,
                             h_envChan[1]->encEnvData.domain_vec, 0,
                             eData[1].frame_info->nEnvelopes, 0,
                             sbrBitstreamData->HeaderActive);

      c = 0;
      for (i = 0; i < eData[0].nEnvelopes; i++) {
        for (j = 0; j < h_envChan[0]->encEnvData.noScfBands[i]; j++) {
          h_envChan[0]->encEnvData.ienvelope[i][j] = eData[0].sfb_nrg[c];
          h_envChan[1]->encEnvData.ienvelope[i][j] = eData[1].sfb_nrg[c];
          c++;
        }
      }

      FDKsbrEnc_codeEnvelope(eData[0].noise_level, fData->res,
                             &h_envChan[0]->sbrCodeNoiseFloor,
                             h_envChan[0]->encEnvData.domain_vec_noise, 0,
                             (eData[0].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        h_envChan[0]->encEnvData.sbr_noise_levels[i] = eData[0].noise_level[i];

      FDKsbrEnc_codeEnvelope(eData[1].noise_level, fData->res,
                             &h_envChan[1]->sbrCodeNoiseFloor,
                             h_envChan[1]->encEnvData.domain_vec_noise, 0,
                             (eData[1].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        h_envChan[1]->encEnvData.sbr_noise_levels[i] = eData[1].noise_level[i];

      sbrHeaderData->coupling = 0;
      h_envChan[0]->encEnvData.balance = 0;
      h_envChan[1]->encEnvData.balance = 0;

      payloadbitsLR = FDKsbrEnc_CountSbrChannelPairElement(
          sbrHeaderData, hParametricStereo, sbrBitstreamData,
          &h_envChan[0]->encEnvData, &h_envChan[1]->encEnvData, hCmonData,
          h_con->sbrSyntaxFlags);

      /*
        swap saved stored with current values
      */
      for (ch = 0; ch < nChannels; ch++) {
        INT itmp;
        for (i = 0; i < MAX_FREQ_COEFFS; i++) {
          /*
            swap sfb energies
          */
          itmp = h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev[i];
          h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev[i] =
              sfbNrgPrevTemp[ch][i];
          sfbNrgPrevTemp[ch][i] = itmp;
        }
        for (i = 0; i < MAX_NUM_NOISE_COEFFS; i++) {
          /*
            swap noise energies
          */
          itmp = h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev[i];
          h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev[i] =
              noisePrevTemp[ch][i];
          noisePrevTemp[ch][i] = itmp;
        }
        /* swap update flags */
        itmp = h_envChan[ch]->sbrCodeEnvelope.upDate;
        h_envChan[ch]->sbrCodeEnvelope.upDate = upDateNrgTemp[ch];
        upDateNrgTemp[ch] = itmp;

        itmp = h_envChan[ch]->sbrCodeNoiseFloor.upDate;
        h_envChan[ch]->sbrCodeNoiseFloor.upDate = upDateNoiseTemp[ch];
        upDateNoiseTemp[ch] = itmp;

        /*
            save domain vecs
        */
        FDKmemcpy(domainVecTemp[ch], h_envChan[ch]->encEnvData.domain_vec,
                  sizeof(INT) * MAX_ENVELOPES);
        FDKmemcpy(domainVecNoiseTemp[ch],
                  h_envChan[ch]->encEnvData.domain_vec_noise,
                  sizeof(INT) * MAX_ENVELOPES);

        /*
          forbid time coding in the first envelope in case of a different
          previous stereomode
        */

        if (!sbrHeaderData->prev_coupling) {
          h_envChan[ch]->sbrCodeEnvelope.upDate = 0;
          h_envChan[ch]->sbrCodeNoiseFloor.upDate = 0;
        }
      } /* ch */

      /*
         Coupling
       */

      FDKsbrEnc_codeEnvelope(
          eData[0].sfb_nrg_coupling, eData[0].frame_info->freqRes,
          &h_envChan[0]->sbrCodeEnvelope, h_envChan[0]->encEnvData.domain_vec,
          1, eData[0].frame_info->nEnvelopes, 0,
          sbrBitstreamData->HeaderActive);

      FDKsbrEnc_codeEnvelope(
          eData[1].sfb_nrg_coupling, eData[1].frame_info->freqRes,
          &h_envChan[1]->sbrCodeEnvelope, h_envChan[1]->encEnvData.domain_vec,
          1, eData[1].frame_info->nEnvelopes, 1,
          sbrBitstreamData->HeaderActive);

      c = 0;
      for (i = 0; i < eData[0].nEnvelopes; i++) {
        for (j = 0; j < h_envChan[0]->encEnvData.noScfBands[i]; j++) {
          h_envChan[0]->encEnvData.ienvelope[i][j] =
              eData[0].sfb_nrg_coupling[c];
          h_envChan[1]->encEnvData.ienvelope[i][j] =
              eData[1].sfb_nrg_coupling[c];
          c++;
        }
      }

      FDKsbrEnc_codeEnvelope(eData[0].noise_level_coupling, fData->res,
                             &h_envChan[0]->sbrCodeNoiseFloor,
                             h_envChan[0]->encEnvData.domain_vec_noise, 1,
                             (eData[0].frame_info->nEnvelopes > 1 ? 2 : 1), 0,
                             sbrBitstreamData->HeaderActive);

      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        h_envChan[0]->encEnvData.sbr_noise_levels[i] =
            eData[0].noise_level_coupling[i];

      FDKsbrEnc_codeEnvelope(eData[1].noise_level_coupling, fData->res,
                             &h_envChan[1]->sbrCodeNoiseFloor,
                             h_envChan[1]->encEnvData.domain_vec_noise, 1,
                             (eData[1].frame_info->nEnvelopes > 1 ? 2 : 1), 1,
                             sbrBitstreamData->HeaderActive);

      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        h_envChan[1]->encEnvData.sbr_noise_levels[i] =
            eData[1].noise_level_coupling[i];

      sbrHeaderData->coupling = 1;

      h_envChan[0]->encEnvData.balance = 0;
      h_envChan[1]->encEnvData.balance = 1;

      tempFlagLeft = h_envChan[0]->encEnvData.addHarmonicFlag;
      tempFlagRight = h_envChan[1]->encEnvData.addHarmonicFlag;

      payloadbitsCOUPLING = FDKsbrEnc_CountSbrChannelPairElement(
          sbrHeaderData, hParametricStereo, sbrBitstreamData,
          &h_envChan[0]->encEnvData, &h_envChan[1]->encEnvData, hCmonData,
          h_con->sbrSyntaxFlags);

      h_envChan[0]->encEnvData.addHarmonicFlag = tempFlagLeft;
      h_envChan[1]->encEnvData.addHarmonicFlag = tempFlagRight;

      if (payloadbitsCOUPLING < payloadbitsLR) {
        /*
          copy coded coupling envelope and noise data to l/r
        */
        for (ch = 0; ch < nChannels; ch++) {
          SBR_ENV_TEMP_DATA *ed = &eData[ch];
          FDKmemcpy(ed->sfb_nrg, ed->sfb_nrg_coupling,
                    MAX_NUM_ENVELOPE_VALUES * sizeof(SCHAR));
          FDKmemcpy(ed->noise_level, ed->noise_level_coupling,
                    MAX_NUM_NOISE_VALUES * sizeof(SCHAR));
        }

        sbrHeaderData->coupling = 1;
        h_envChan[0]->encEnvData.balance = 0;
        h_envChan[1]->encEnvData.balance = 1;
      } else {
        /*
          restore saved l/r items
        */
        for (ch = 0; ch < nChannels; ch++) {
          FDKmemcpy(h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev,
                    sfbNrgPrevTemp[ch], MAX_FREQ_COEFFS * sizeof(SCHAR));

          h_envChan[ch]->sbrCodeEnvelope.upDate = upDateNrgTemp[ch];

          FDKmemcpy(h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev,
                    noisePrevTemp[ch], MAX_NUM_NOISE_COEFFS * sizeof(SCHAR));

          FDKmemcpy(h_envChan[ch]->encEnvData.domain_vec, domainVecTemp[ch],
                    sizeof(INT) * MAX_ENVELOPES);
          FDKmemcpy(h_envChan[ch]->encEnvData.domain_vec_noise,
                    domainVecNoiseTemp[ch], sizeof(INT) * MAX_ENVELOPES);

          h_envChan[ch]->sbrCodeNoiseFloor.upDate = upDateNoiseTemp[ch];
        }

        sbrHeaderData->coupling = 0;
        h_envChan[0]->encEnvData.balance = 0;
        h_envChan[1]->encEnvData.balance = 0;
      }
    } break;
  } /* switch */

  /* tell the envelope encoders how long it has been, since we last sent
     a frame starting with a dF-coded envelope */
  if (stereoMode == SBR_MONO) {
    if (h_envChan[0]->encEnvData.domain_vec[0] == TIME)
      h_envChan[0]->sbrCodeEnvelope.dF_edge_incr_fac++;
    else
      h_envChan[0]->sbrCodeEnvelope.dF_edge_incr_fac = 0;
  } else {
    if (h_envChan[0]->encEnvData.domain_vec[0] == TIME ||
        h_envChan[1]->encEnvData.domain_vec[0] == TIME) {
      h_envChan[0]->sbrCodeEnvelope.dF_edge_incr_fac++;
      h_envChan[1]->sbrCodeEnvelope.dF_edge_incr_fac++;
    } else {
      h_envChan[0]->sbrCodeEnvelope.dF_edge_incr_fac = 0;
      h_envChan[1]->sbrCodeEnvelope.dF_edge_incr_fac = 0;
    }
  }

  /*
    Send the encoded data to the bitstream
  */
  for (ch = 0; ch < nChannels; ch++) {
    SBR_ENV_TEMP_DATA *ed = &eData[ch];
    c = 0;
    for (i = 0; i < ed->nEnvelopes; i++) {
      for (j = 0; j < h_envChan[ch]->encEnvData.noScfBands[i]; j++) {
        h_envChan[ch]->encEnvData.ienvelope[i][j] = ed->sfb_nrg[c];

        c++;
      }
    }
    for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
      h_envChan[ch]->encEnvData.sbr_noise_levels[i] = ed->noise_level[i];
    }
  } /* ch */

  /*
    Write bitstream
  */
  if (nChannels == 2) {
    FDKsbrEnc_WriteEnvChannelPairElement(
        sbrHeaderData, hParametricStereo, sbrBitstreamData,
        &h_envChan[0]->encEnvData, &h_envChan[1]->encEnvData, hCmonData,
        h_con->sbrSyntaxFlags);
  } else {
    FDKsbrEnc_WriteEnvSingleChannelElement(
        sbrHeaderData, hParametricStereo, sbrBitstreamData,
        &h_envChan[0]->encEnvData, hCmonData, h_con->sbrSyntaxFlags);
  }

  /*
   * Update buffers.
   */
  for (ch = 0; ch < nChannels; ch++) {
    int YBufferLength = h_envChan[ch]->sbrExtractEnvelope.no_cols >>
                        h_envChan[ch]->sbrExtractEnvelope.YBufferSzShift;
    for (i = 0; i < h_envChan[ch]->sbrExtractEnvelope.YBufferWriteOffset; i++) {
      FDKmemcpy(h_envChan[ch]->sbrExtractEnvelope.YBuffer[i],
                h_envChan[ch]->sbrExtractEnvelope.YBuffer[i + YBufferLength],
                sizeof(FIXP_DBL) * 64);
    }
    h_envChan[ch]->sbrExtractEnvelope.YBufferScale[0] =
        h_envChan[ch]->sbrExtractEnvelope.YBufferScale[1];
  }

  sbrHeaderData->prev_coupling = sbrHeaderData->coupling;
}

/***************************************************************************/
/*!

  \brief  creates an envelope extractor handle

  \return error status

****************************************************************************/
INT FDKsbrEnc_CreateExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut,
                                       INT channel, INT chInEl,
                                       UCHAR *dynamic_RAM) {
  INT i;
  FIXP_DBL *rBuffer, *iBuffer;
  INT n;
  FIXP_DBL *YBufferDyn;

  FDKmemclear(hSbrCut, sizeof(SBR_EXTRACT_ENVELOPE));

  if (NULL == (hSbrCut->p_YBuffer = GetRam_Sbr_envYBuffer(channel))) {
    goto bail;
  }

  for (i = 0; i < (32 >> 1); i++) {
    hSbrCut->YBuffer[i] = hSbrCut->p_YBuffer + (i * 64);
  }
  YBufferDyn = GetRam_Sbr_envYBuffer(chInEl, dynamic_RAM);
  for (n = 0; i < 32; i++, n++) {
    hSbrCut->YBuffer[i] = YBufferDyn + (n * 64);
  }

  rBuffer = GetRam_Sbr_envRBuffer(0, dynamic_RAM);
  iBuffer = GetRam_Sbr_envIBuffer(0, dynamic_RAM);

  for (i = 0; i < 32; i++) {
    hSbrCut->rBuffer[i] = rBuffer + (i * 64);
    hSbrCut->iBuffer[i] = iBuffer + (i * 64);
  }

  return 0;

bail:
  FDKsbrEnc_deleteExtractSbrEnvelope(hSbrCut);

  return -1;
}

/***************************************************************************/
/*!

  \brief  Initialize an envelope extractor instance.

  \return error status

****************************************************************************/
INT FDKsbrEnc_InitExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut,
                                     int no_cols, int no_rows, int start_index,
                                     int time_slots, int time_step,
                                     int tran_off, ULONG statesInitFlag,
                                     int chInEl, UCHAR *dynamic_RAM,
                                     UINT sbrSyntaxFlags) {
  int YBufferLength, rBufferLength;
  int i;

  if (sbrSyntaxFlags & SBR_SYNTAX_LOW_DELAY) {
    int off = TRANSIENT_OFFSET_LD;
    hSbrCut->YBufferWriteOffset = (no_cols >> 1) + off * time_step;
  } else {
    hSbrCut->YBufferWriteOffset = tran_off * time_step;
  }
  hSbrCut->rBufferReadOffset = 0;

  YBufferLength = hSbrCut->YBufferWriteOffset + no_cols;
  rBufferLength = no_cols;

  hSbrCut->pre_transient_info[0] = 0;
  hSbrCut->pre_transient_info[1] = 0;

  hSbrCut->no_cols = no_cols;
  hSbrCut->no_rows = no_rows;
  hSbrCut->start_index = start_index;

  hSbrCut->time_slots = time_slots;
  hSbrCut->time_step = time_step;

  FDK_ASSERT(no_rows <= 64);

  /* Use half the Energy values if time step is 2 or greater */
  if (time_step >= 2)
    hSbrCut->YBufferSzShift = 1;
  else
    hSbrCut->YBufferSzShift = 0;

  YBufferLength >>= hSbrCut->YBufferSzShift;
  hSbrCut->YBufferWriteOffset >>= hSbrCut->YBufferSzShift;

  FDK_ASSERT(YBufferLength <= 32);

  FIXP_DBL *YBufferDyn = GetRam_Sbr_envYBuffer(chInEl, dynamic_RAM);
  INT n = 0;
  for (i = (32 >> 1); i < 32; i++, n++) {
    hSbrCut->YBuffer[i] = YBufferDyn + (n * 64);
  }

  if (statesInitFlag) {
    for (i = 0; i < YBufferLength; i++) {
      FDKmemclear(hSbrCut->YBuffer[i], 64 * sizeof(FIXP_DBL));
    }
  }

  for (i = 0; i < rBufferLength; i++) {
    FDKmemclear(hSbrCut->rBuffer[i], 64 * sizeof(FIXP_DBL));
    FDKmemclear(hSbrCut->iBuffer[i], 64 * sizeof(FIXP_DBL));
  }

  FDKmemclear(hSbrCut->envelopeCompensation, sizeof(UCHAR) * MAX_FREQ_COEFFS);

  if (statesInitFlag) {
    hSbrCut->YBufferScale[0] = hSbrCut->YBufferScale[1] = FRACT_BITS - 1;
  }

  return (0);
}

/***************************************************************************/
/*!

  \brief  deinitializes an envelope extractor handle

  \return void

****************************************************************************/

void FDKsbrEnc_deleteExtractSbrEnvelope(HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut) {
  if (hSbrCut) {
    FreeRam_Sbr_envYBuffer(&hSbrCut->p_YBuffer);
  }
}

INT FDKsbrEnc_GetEnvEstDelay(HANDLE_SBR_EXTRACT_ENVELOPE hSbr) {
  return hSbr->no_rows *
         ((hSbr->YBufferWriteOffset) *
              2 /* mult 2 because nrg's are grouped half */
          - hSbr->rBufferReadOffset); /* in reference hold half spec and calc
                                         nrg's on overlapped spec */
}
