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

/*********************** MPEG surround encoder library *************************

   Author(s):

   Description: Encoder Library Interface
                Bitstream Writer

*******************************************************************************/

#ifndef SACENC_BITSTREAM_H
#define SACENC_BITSTREAM_H

/* Includes ******************************************************************/
#include "FDK_bitstream.h"
#include "FDK_matrixCalloc.h"
#include "sacenc_lib.h"
#include "sacenc_const.h"

/* Defines *******************************************************************/
#define MAX_NUM_BINS 23
#define MAX_NUM_PARAMS 2
#define MAX_NUM_OUTPUTCHANNELS SACENC_MAX_OUTPUT_CHANNELS
#define MAX_TIME_SLOTS 32

typedef enum {
  TREE_212 = 7,
  TREE_ESCAPE = 15

} TREECONFIG;

typedef enum {
  FREQ_RES_40 = 0,
  FREQ_RES_20 = 1,
  FREQ_RES_10 = 2,
  FREQ_RES_5 = 3

} FREQ;

typedef enum {
  QUANTMODE_INVALID = -1,
  QUANTMODE_FINE = 0,
  QUANTMODE_EBQ1 = 1,
  QUANTMODE_EBQ2 = 2

} QUANTMODE;

typedef enum {
  TEMPSHAPE_OFF = 0

} TEMPSHAPECONFIG;

typedef enum {
  FIXEDGAINDMX_INVALID = -1,
  FIXEDGAINDMX_0 = 0,
  FIXEDGAINDMX_1 = 1,
  FIXEDGAINDMX_2 = 2,
  FIXEDGAINDMX_3 = 3,
  FIXEDGAINDMX_4 = 4,
  FIXEDGAINDMX_5 = 5,
  FIXEDGAINDMX_6 = 6,
  FIXEDGAINDMX_7 = 7

} FIXEDGAINDMXCONFIG;

typedef enum {
  DECORR_INVALID = -1,
  DECORR_QMFSPLIT0 = 0, /* QMF splitfreq: 3, 15, 24, 65 */
  DECORR_QMFSPLIT1 = 1, /* QMF splitfreq: 3, 50, 65, 65 */
  DECORR_QMFSPLIT2 = 2  /* QMF splitfreq: 0, 15, 65, 65 */

} DECORRCONFIG;

typedef enum {
  DEFAULT = 0,
  KEEP = 1,
  INTERPOLATE = 2,
  FINECOARSE = 3

} DATA_MODE;

typedef enum {
  READ_SPATIALFRAME = 0,
  WRITE_SPATIALFRAME = 1

} SPATIALFRAME_TYPE;

/* Data Types ****************************************************************/
typedef struct {
  INT numOttBoxes;
  INT numInChan;
  INT numOutChan;

} TREEDESCRIPTION;

typedef struct {
  INT bsOttBands;

} OTTCONFIG;

typedef struct {
  INT bsSamplingFrequency; /* for bsSamplingFrequencyIndex */
  INT bsFrameLength;
  INT numBands; /* for bsFreqRes */
  TREECONFIG bsTreeConfig;
  QUANTMODE bsQuantMode;
  FIXEDGAINDMXCONFIG bsFixedGainDMX;
  int bsEnvQuantMode;
  DECORRCONFIG bsDecorrConfig;
  TREEDESCRIPTION treeDescription;
  OTTCONFIG ottConfig[SACENC_MAX_NUM_BOXES];

} SPATIALSPECIFICCONFIG;

typedef struct {
  UCHAR bsFramingType;
  UCHAR numParamSets;
  UCHAR bsParamSlots[MAX_NUM_PARAMS];

} FRAMINGINFO;

typedef struct {
  SCHAR cld[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS][MAX_NUM_BINS];
  SCHAR icc[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS][MAX_NUM_BINS];

} OTTDATA;

typedef struct {
  UCHAR bsSmoothMode[MAX_NUM_PARAMS];
  UCHAR bsSmoothTime[MAX_NUM_PARAMS];
  UCHAR bsFreqResStride[MAX_NUM_PARAMS];
  UCHAR bsSmgData[MAX_NUM_PARAMS][MAX_NUM_BINS];

} SMGDATA;

typedef struct {
  UCHAR bsEnvShapeChannel[MAX_NUM_OUTPUTCHANNELS];
  UCHAR bsEnvShapeData[MAX_NUM_OUTPUTCHANNELS][MAX_TIME_SLOTS];

} TEMPSHAPEDATA;

typedef struct {
  UCHAR bsXXXDataMode[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];
  UCHAR bsDataPair[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];
  UCHAR bsQuantCoarseXXX[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];
  UCHAR bsFreqResStrideXXX[SACENC_MAX_NUM_BOXES][MAX_NUM_PARAMS];

} LOSSLESSDATA;

typedef struct {
  FRAMINGINFO framingInfo;
  UCHAR bsIndependencyFlag;
  OTTDATA ottData;
  SMGDATA smgData;
  TEMPSHAPEDATA tempShapeData;
  LOSSLESSDATA CLDLosslessData;
  LOSSLESSDATA ICCLosslessData;
  UCHAR bUseBBCues;

} SPATIALFRAME;

typedef struct BSF_INSTANCE *HANDLE_BSF_INSTANCE;

/* Constants *****************************************************************/

/* Function / Class Declarations *********************************************/
/* destroy encoder instance */
FDK_SACENC_ERROR fdk_sacenc_destroySpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE *selfPtr);

/* create encoder instance */
FDK_SACENC_ERROR fdk_sacenc_createSpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE *selfPtr);

FDK_SACENC_ERROR fdk_sacenc_initSpatialBitstreamEncoder(
    HANDLE_BSF_INSTANCE selfPtr);

/* get SpatialSpecificConfig struct */
SPATIALSPECIFICCONFIG *fdk_sacenc_getSpatialSpecificConfig(
    HANDLE_BSF_INSTANCE selfPtr);

/* write SpatialSpecificConfig to stream */
FDK_SACENC_ERROR fdk_sacenc_writeSpatialSpecificConfig(
    SPATIALSPECIFICCONFIG *const spatialSpecificConfig,
    UCHAR *const pOutputBuffer, const INT outputBufferSize,
    INT *const pnOutputBits);

/* get SpatialFrame struct */
SPATIALFRAME *fdk_sacenc_getSpatialFrame(HANDLE_BSF_INSTANCE selfPtr,
                                         const SPATIALFRAME_TYPE frameType);

/* write frame data to stream */
FDK_SACENC_ERROR fdk_sacenc_writeSpatialFrame(UCHAR *const pOutputBuffer,
                                              const INT outputBufferSize,
                                              INT *const pnOutputBits,
                                              HANDLE_BSF_INSTANCE selfPtr);

/* Copy/Save spatial frame data for one parameter set */
FDK_SACENC_ERROR fdk_sacenc_duplicateParameterSet(
    const SPATIALFRAME *const hFrom, const INT setFrom, SPATIALFRAME *const hTo,
    const INT setTo);

#endif /* SACENC_BITSTREAM_H */
