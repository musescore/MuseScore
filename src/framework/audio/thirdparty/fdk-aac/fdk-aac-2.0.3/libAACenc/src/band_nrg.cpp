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

   Description: Band/Line energy calculations

*******************************************************************************/

#include "band_nrg.h"

/*****************************************************************************
  functionname: FDKaacEnc_CalcSfbMaxScaleSpec
  description:
  input:
  output:
*****************************************************************************/
void FDKaacEnc_CalcSfbMaxScaleSpec(const FIXP_DBL *RESTRICT mdctSpectrum,
                                   const INT *RESTRICT bandOffset,
                                   INT *RESTRICT sfbMaxScaleSpec,
                                   const INT numBands) {
  INT i, j;
  FIXP_DBL maxSpc, tmp;

  for (i = 0; i < numBands; i++) {
    maxSpc = (FIXP_DBL)0;

    DWORD_ALIGNED(mdctSpectrum);

    for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
      tmp = fixp_abs(mdctSpectrum[j]);
      maxSpc = fixMax(maxSpc, tmp);
    }
    j = CntLeadingZeros(maxSpc) - 1;
    sfbMaxScaleSpec[i] = fixMin((DFRACT_BITS - 2), j);
    /* CountLeadingBits() is not necessary here since test value is always > 0
     */
  }
}

/*****************************************************************************
  functionname: FDKaacEnc_CheckBandEnergyOptim
  description:
  input:
  output:
*****************************************************************************/
FIXP_DBL
FDKaacEnc_CheckBandEnergyOptim(const FIXP_DBL *const RESTRICT mdctSpectrum,
                               const INT *const RESTRICT sfbMaxScaleSpec,
                               const INT *const RESTRICT bandOffset,
                               const INT numBands,
                               FIXP_DBL *RESTRICT bandEnergy,
                               FIXP_DBL *RESTRICT bandEnergyLdData,
                               const INT minSpecShift) {
  INT i, j, scale, nr = 0;
  FIXP_DBL maxNrgLd = FL2FXCONST_DBL(-1.0f);
  FIXP_DBL maxNrg = 0;
  FIXP_DBL spec;

  for (i = 0; i < numBands; i++) {
    scale = fixMax(0, sfbMaxScaleSpec[i] - 4);
    FIXP_DBL tmp = 0;

    DWORD_ALIGNED(mdctSpectrum);

    for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
      spec = mdctSpectrum[j] << scale;
      tmp = fPow2AddDiv2(tmp, spec);
    }
    bandEnergy[i] = tmp << 1;

    /* calculate ld of bandNrg, subtract scaling */
    bandEnergyLdData[i] = CalcLdData(bandEnergy[i]);
    if (bandEnergyLdData[i] != FL2FXCONST_DBL(-1.0f)) {
      bandEnergyLdData[i] -= scale * FL2FXCONST_DBL(2.0 / 64);
    }
    /* find index of maxNrg */
    if (bandEnergyLdData[i] > maxNrgLd) {
      maxNrgLd = bandEnergyLdData[i];
      nr = i;
    }
  }

  /* return unscaled maxNrg*/
  scale = fixMax(0, sfbMaxScaleSpec[nr] - 4);
  scale = fixMax(2 * (minSpecShift - scale), -(DFRACT_BITS - 1));

  maxNrg = scaleValue(bandEnergy[nr], scale);

  return maxNrg;
}

/*****************************************************************************
  functionname: FDKaacEnc_CalcBandEnergyOptimLong
  description:
  input:
  output:
*****************************************************************************/
INT FDKaacEnc_CalcBandEnergyOptimLong(const FIXP_DBL *RESTRICT mdctSpectrum,
                                      INT *RESTRICT sfbMaxScaleSpec,
                                      const INT *RESTRICT bandOffset,
                                      const INT numBands,
                                      FIXP_DBL *RESTRICT bandEnergy,
                                      FIXP_DBL *RESTRICT bandEnergyLdData) {
  INT i, j, shiftBits = 0;
  FIXP_DBL maxNrgLd = FL2FXCONST_DBL(0.0f);

  FIXP_DBL spec;

  for (i = 0; i < numBands; i++) {
    INT leadingBits = sfbMaxScaleSpec[i] -
                      4; /* max sfbWidth = 96 ; 2^7=128 => 7/2 = 4 (spc*spc) */
    FIXP_DBL tmp = FL2FXCONST_DBL(0.0);
    /* don't use scaleValue() here, it increases workload quite sufficiently...
     */
    if (leadingBits >= 0) {
      for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
        spec = mdctSpectrum[j] << leadingBits;
        tmp = fPow2AddDiv2(tmp, spec);
      }
    } else {
      INT shift = -leadingBits;
      for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
        spec = mdctSpectrum[j] >> shift;
        tmp = fPow2AddDiv2(tmp, spec);
      }
    }
    bandEnergy[i] = tmp << 1;
  }

  /* calculate ld of bandNrg, subtract scaling */
  LdDataVector(bandEnergy, bandEnergyLdData, numBands);
  for (i = numBands; i-- != 0;) {
    FIXP_DBL scaleDiff = (sfbMaxScaleSpec[i] - 4) * FL2FXCONST_DBL(2.0 / 64);

    bandEnergyLdData[i] = (bandEnergyLdData[i] >=
                           ((FL2FXCONST_DBL(-1.f) >> 1) + (scaleDiff >> 1)))
                              ? bandEnergyLdData[i] - scaleDiff
                              : FL2FXCONST_DBL(-1.f);
    /* find maxNrgLd */
    maxNrgLd = fixMax(maxNrgLd, bandEnergyLdData[i]);
  }

  if (maxNrgLd <= (FIXP_DBL)0) {
    for (i = numBands; i-- != 0;) {
      INT scale = fixMin((sfbMaxScaleSpec[i] - 4) << 1, (DFRACT_BITS - 1));
      bandEnergy[i] = scaleValue(bandEnergy[i], -scale);
    }
    return 0;
  } else { /* scale down NRGs */
    while (maxNrgLd > FL2FXCONST_DBL(0.0f)) {
      maxNrgLd -= FL2FXCONST_DBL(2.0 / 64);
      shiftBits++;
    }
    for (i = numBands; i-- != 0;) {
      INT scale = fixMin(((sfbMaxScaleSpec[i] - 4) + shiftBits) << 1,
                         (DFRACT_BITS - 1));
      bandEnergyLdData[i] -= shiftBits * FL2FXCONST_DBL(2.0 / 64);
      bandEnergy[i] = scaleValue(bandEnergy[i], -scale);
    }
    return shiftBits;
  }
}

/*****************************************************************************
  functionname: FDKaacEnc_CalcBandEnergyOptimShort
  description:
  input:
  output:
*****************************************************************************/
void FDKaacEnc_CalcBandEnergyOptimShort(const FIXP_DBL *RESTRICT mdctSpectrum,
                                        INT *RESTRICT sfbMaxScaleSpec,
                                        const INT *RESTRICT bandOffset,
                                        const INT numBands,
                                        FIXP_DBL *RESTRICT bandEnergy) {
  INT i, j;

  for (i = 0; i < numBands; i++) {
    int leadingBits = sfbMaxScaleSpec[i] -
                      3; /* max sfbWidth = 36 ; 2^6=64 => 6/2 = 3 (spc*spc) */
    FIXP_DBL tmp = FL2FXCONST_DBL(0.0);
    for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
      FIXP_DBL spec = scaleValue(mdctSpectrum[j], leadingBits);
      tmp = fPow2AddDiv2(tmp, spec);
    }
    bandEnergy[i] = tmp;
  }

  for (i = 0; i < numBands; i++) {
    INT scale = (2 * (sfbMaxScaleSpec[i] - 3)) -
                1; /* max sfbWidth = 36 ; 2^6=64 => 6/2 = 3 (spc*spc) */
    scale = fixMax(fixMin(scale, (DFRACT_BITS - 1)), -(DFRACT_BITS - 1));
    bandEnergy[i] = scaleValueSaturate(bandEnergy[i], -scale);
  }
}

/*****************************************************************************
  functionname: FDKaacEnc_CalcBandNrgMSOpt
  description:
  input:
  output:
*****************************************************************************/
void FDKaacEnc_CalcBandNrgMSOpt(
    const FIXP_DBL *RESTRICT mdctSpectrumLeft,
    const FIXP_DBL *RESTRICT mdctSpectrumRight,
    INT *RESTRICT sfbMaxScaleSpecLeft, INT *RESTRICT sfbMaxScaleSpecRight,
    const INT *RESTRICT bandOffset, const INT numBands,
    FIXP_DBL *RESTRICT bandEnergyMid, FIXP_DBL *RESTRICT bandEnergySide,
    INT calcLdData, FIXP_DBL *RESTRICT bandEnergyMidLdData,
    FIXP_DBL *RESTRICT bandEnergySideLdData) {
  INT i, j, minScale;
  FIXP_DBL NrgMid, NrgSide, specm, specs;

  for (i = 0; i < numBands; i++) {
    NrgMid = NrgSide = FL2FXCONST_DBL(0.0);
    minScale = fixMin(sfbMaxScaleSpecLeft[i], sfbMaxScaleSpecRight[i]) - 4;
    minScale = fixMax(0, minScale);

    if (minScale > 0) {
      for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
        FIXP_DBL specL = mdctSpectrumLeft[j] << (minScale - 1);
        FIXP_DBL specR = mdctSpectrumRight[j] << (minScale - 1);
        specm = specL + specR;
        specs = specL - specR;
        NrgMid = fPow2AddDiv2(NrgMid, specm);
        NrgSide = fPow2AddDiv2(NrgSide, specs);
      }
    } else {
      for (j = bandOffset[i]; j < bandOffset[i + 1]; j++) {
        FIXP_DBL specL = mdctSpectrumLeft[j] >> 1;
        FIXP_DBL specR = mdctSpectrumRight[j] >> 1;
        specm = specL + specR;
        specs = specL - specR;
        NrgMid = fPow2AddDiv2(NrgMid, specm);
        NrgSide = fPow2AddDiv2(NrgSide, specs);
      }
    }
    bandEnergyMid[i] = fMin(NrgMid, (FIXP_DBL)MAXVAL_DBL >> 1) << 1;
    bandEnergySide[i] = fMin(NrgSide, (FIXP_DBL)MAXVAL_DBL >> 1) << 1;
  }

  if (calcLdData) {
    LdDataVector(bandEnergyMid, bandEnergyMidLdData, numBands);
    LdDataVector(bandEnergySide, bandEnergySideLdData, numBands);
  }

  for (i = 0; i < numBands; i++) {
    minScale = fixMin(sfbMaxScaleSpecLeft[i], sfbMaxScaleSpecRight[i]);
    INT scale = fixMax(0, 2 * (minScale - 4));

    if (calcLdData) {
      /* using the minimal scaling of left and right channel can cause very
      small energies; check ldNrg before subtract scaling multiplication:
      fract*INT we don't need fMult */

      int minus = scale * FL2FXCONST_DBL(1.0 / 64);

      if (bandEnergyMidLdData[i] != FL2FXCONST_DBL(-1.0f))
        bandEnergyMidLdData[i] -= minus;

      if (bandEnergySideLdData[i] != FL2FXCONST_DBL(-1.0f))
        bandEnergySideLdData[i] -= minus;
    }
    scale = fixMin(scale, (DFRACT_BITS - 1));
    bandEnergyMid[i] >>= scale;
    bandEnergySide[i] >>= scale;
  }
}
