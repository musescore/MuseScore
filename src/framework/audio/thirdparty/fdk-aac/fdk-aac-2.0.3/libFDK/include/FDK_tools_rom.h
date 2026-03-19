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

/******************* Library for basic calculation routines ********************

   Author(s):   Oliver Moser

   Description: ROM tables used by FDK tools

*******************************************************************************/

#ifndef FDK_TOOLS_ROM_H
#define FDK_TOOLS_ROM_H

#include "common_fix.h"
#include "FDK_audio.h"

/* sinetables */

/* None radix2 rotation vectors */
extern RAM_ALIGN const FIXP_STB RotVectorReal60[60];
extern RAM_ALIGN const FIXP_STB RotVectorImag60[60];
extern RAM_ALIGN const FIXP_STB RotVectorReal192[192];
extern RAM_ALIGN const FIXP_STB RotVectorImag192[192];
extern RAM_ALIGN const FIXP_STB RotVectorReal240[210];
extern RAM_ALIGN const FIXP_STB RotVectorImag240[210];
extern RAM_ALIGN const FIXP_STB RotVectorReal480[480];
extern RAM_ALIGN const FIXP_STB RotVectorImag480[480];
extern RAM_ALIGN const FIXP_STB RotVectorReal6[6];
extern RAM_ALIGN const FIXP_STB RotVectorImag6[6];
extern RAM_ALIGN const FIXP_STB RotVectorReal12[12];
extern RAM_ALIGN const FIXP_STB RotVectorImag12[12];
extern RAM_ALIGN const FIXP_STB RotVectorReal24[24];
extern RAM_ALIGN const FIXP_STB RotVectorImag24[24];
extern RAM_ALIGN const FIXP_STB RotVectorReal48[48];
extern RAM_ALIGN const FIXP_STB RotVectorImag48[48];
extern RAM_ALIGN const FIXP_STB RotVectorReal80[80];
extern RAM_ALIGN const FIXP_STB RotVectorImag80[80];
extern RAM_ALIGN const FIXP_STB RotVectorReal96[96];
extern RAM_ALIGN const FIXP_STB RotVectorImag96[96];
extern RAM_ALIGN const FIXP_STB RotVectorReal384[384];
extern RAM_ALIGN const FIXP_STB RotVectorImag384[384];
extern RAM_ALIGN const FIXP_STB RotVectorReal20[20];
extern RAM_ALIGN const FIXP_STB RotVectorImag20[20];
extern RAM_ALIGN const FIXP_STB RotVectorReal120[120];
extern RAM_ALIGN const FIXP_STB RotVectorImag120[120];

/* Regular sine tables */
extern RAM_ALIGN const FIXP_STP SineTable1024[];
extern RAM_ALIGN const FIXP_STP SineTable512[];
extern RAM_ALIGN const FIXP_STP SineTable480[];
extern RAM_ALIGN const FIXP_STP SineTable384[];
extern RAM_ALIGN const FIXP_STP SineTable80[];
#ifdef INCLUDE_SineTable10
extern RAM_ALIGN const FIXP_STP SineTable10[];
#endif

/* AAC-LC windows */
extern RAM_ALIGN const FIXP_WTP SineWindow1024[];
extern RAM_ALIGN const FIXP_WTP KBDWindow1024[];
extern RAM_ALIGN const FIXP_WTP SineWindow128[];
extern RAM_ALIGN const FIXP_WTP KBDWindow128[];

extern RAM_ALIGN const FIXP_WTP SineWindow960[];
extern RAM_ALIGN const FIXP_WTP KBDWindow960[];
extern RAM_ALIGN const FIXP_WTP SineWindow120[];
extern RAM_ALIGN const FIXP_WTP KBDWindow120[];

/* AAC-LD windows */
extern RAM_ALIGN const FIXP_WTP SineWindow512[];
#define LowOverlapWindow512 SineWindow128
extern RAM_ALIGN const FIXP_WTP SineWindow480[];
#define LowOverlapWindow480 SineWindow120

/* USAC TCX Window */
extern RAM_ALIGN const FIXP_WTP SineWindow256[256];
extern RAM_ALIGN const FIXP_WTP SineWindow192[];

/* USAC 8/3 windows */
extern RAM_ALIGN const FIXP_WTP SineWindow768[];
extern RAM_ALIGN const FIXP_WTP KBDWindow768[];
extern RAM_ALIGN const FIXP_WTP SineWindow96[];
extern RAM_ALIGN const FIXP_WTP KBDWindow96[];

/* DCT and others */
extern RAM_ALIGN const FIXP_WTP SineWindow64[];
extern RAM_ALIGN const FIXP_WTP SineWindow48[];
extern RAM_ALIGN const FIXP_WTP SineWindow32[];
extern RAM_ALIGN const FIXP_WTP SineWindow24[];
extern RAM_ALIGN const FIXP_WTP SineWindow16[];
extern RAM_ALIGN const FIXP_WTP SineWindow8[];

/**
 * \brief Helper table for window slope mapping. You should prefer the usage of
 * the function FDKgetWindowSlope(), this table is only made public for some
 * optimized access inside dct.cpp.
 */
extern const FIXP_WTP *const windowSlopes[2][4][9];

/**
 * \brief Window slope access helper. Obtain a window of given length and shape.
 * \param length Length of the window slope.
 * \param shape Shape index of the window slope. 0: sine window, 1:
 * Kaiser-Bessel. Any other value is applied a mask of 1 to, mapping it to
 * either 0 or 1.
 * \param Pointer to window slope or NULL if the requested window slope is not
 * available.
 */
const FIXP_WTP *FDKgetWindowSlope(int length, int shape);

extern const FIXP_WTP sin_twiddle_L64[];

/*
 * Filter coefficient type definition
 */

#if defined(ARCH_PREFER_MULT_16x16) || defined(ARCH_PREFER_MULT_32x16)
#define QMF_COEFF_16BIT
#endif

#define QMF_FILTER_PROTOTYPE_SIZE 640
#define QMF_NO_POLY 5

#ifdef QMF_COEFF_16BIT
#define FIXP_PFT FIXP_SGL
#define FIXP_QTW FIXP_SGL
#define FX_DBL2FX_QTW(x) FX_DBL2FX_SGL(x)
#else
#define FIXP_PFT FIXP_DBL
#define FIXP_QTW FIXP_DBL

#define FX_DBL2FX_QTW(x) (x)

#endif

#define QMF640_PFT_TABLE_SIZE (640 / 2 + QMF_NO_POLY)

/* Resampling twiddles for QMF */

/* Not resampling twiddles */
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos32[32];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin32[32];
/* Adapted analysis post-twiddles for down-sampled HQ SBR */
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos_downsamp32[32];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin_downsamp32[32];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos64[64];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin64[64];
extern RAM_ALIGN const FIXP_PFT
    qmf_pfilt640[QMF640_PFT_TABLE_SIZE + QMF_NO_POLY];
extern RAM_ALIGN const FIXP_PFT qmf_pfilt640_vector[640];

extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos40[40];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin40[40];
extern RAM_ALIGN const FIXP_PFT qmf_pfilt400[];
extern RAM_ALIGN const FIXP_PFT qmf_pfilt200[];
extern RAM_ALIGN const FIXP_PFT qmf_pfilt120[];

extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos24[24];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin24[24];
extern RAM_ALIGN const FIXP_PFT qmf_pfilt240[];

extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_cos16[16];
extern RAM_ALIGN const FIXP_QTW qmf_phaseshift_sin16[16];

#define QMF640_CLDFB_PFT_TABLE_SIZE (640)
#define QMF320_CLDFB_PFT_TABLE_SIZE (320)
#define QMF_CLDFB_PFT_SCALE 1

extern const FIXP_QTW qmf_phaseshift_cos32_cldfb_ana[32];
extern const FIXP_QTW qmf_phaseshift_cos32_cldfb_syn[32];
extern const FIXP_QTW qmf_phaseshift_sin32_cldfb[32];

extern const FIXP_QTW qmf_phaseshift_cos16_cldfb_ana[16];
extern const FIXP_QTW qmf_phaseshift_cos16_cldfb_syn[16];
extern const FIXP_QTW qmf_phaseshift_sin16_cldfb[16];

extern const FIXP_QTW qmf_phaseshift_cos8_cldfb_ana[8];
extern const FIXP_QTW qmf_phaseshift_cos8_cldfb_syn[8];
extern const FIXP_QTW qmf_phaseshift_sin8_cldfb[8];

extern const FIXP_QTW qmf_phaseshift_cos64_cldfb[64];
extern const FIXP_QTW qmf_phaseshift_sin64_cldfb[64];

extern RAM_ALIGN const FIXP_PFT qmf_cldfb_640[QMF640_CLDFB_PFT_TABLE_SIZE];
extern RAM_ALIGN const FIXP_PFT qmf_cldfb_320[QMF320_CLDFB_PFT_TABLE_SIZE];
#define QMF160_CLDFB_PFT_TABLE_SIZE (160)
extern RAM_ALIGN const FIXP_PFT qmf_cldfb_160[QMF160_CLDFB_PFT_TABLE_SIZE];
#define QMF80_CLDFB_PFT_TABLE_SIZE (80)
extern RAM_ALIGN const FIXP_PFT qmf_cldfb_80[QMF80_CLDFB_PFT_TABLE_SIZE];

#define QMF320_MPSLDFB_PFT_TABLE_SIZE (320)
#define QMF640_MPSLDFB_PFT_TABLE_SIZE (640)
#define QMF_MPSLDFB_PFT_SCALE 1

extern const FIXP_PFT qmf_mpsldfb_320[QMF320_MPSLDFB_PFT_TABLE_SIZE];
extern RAM_ALIGN const FIXP_PFT qmf_mpsldfb_640[QMF640_MPSLDFB_PFT_TABLE_SIZE];

/**
 * Audio bitstream element specific syntax flags:
 */
#define AC_EL_GA_CCE 0x00000001 /*!< GA AAC coupling channel element (CCE) */

/*
 * Raw Data Block list items.
 */
typedef enum {
  element_instance_tag,
  common_window, /* -> decision for link_sequence */
  global_gain,
  ics_info, /* ics_reserved_bit, window_sequence, window_shape, max_sfb,
               scale_factor_grouping, predictor_data_present, ltp_data_present,
               ltp_data */
  max_sfb,
  ms,                         /* ms_mask_present, ms_used */
  /*predictor_data_present,*/ /* part of ics_info */
  ltp_data_present,
  ltp_data,
  section_data,
  scale_factor_data,
  pulse, /* pulse_data_present, pulse_data  */
  tns_data_present,
  tns_data,
  gain_control_data_present,
  gain_control_data,
  esc1_hcr,
  esc2_rvlc,
  spectral_data,

  scale_factor_data_usac,
  core_mode, /* -> decision for link_sequence */
  common_tw,
  lpd_channel_stream,
  tw_data,
  noise,
  ac_spectral_data,
  fac_data,
  tns_active, /* introduced in MPEG-D usac CD */
  tns_data_present_usac,
  common_max_sfb,

  coupled_elements,   /* only for CCE parsing */
  gain_element_lists, /* only for CCE parsing */

  /* Non data list items */
  adtscrc_start_reg1,
  adtscrc_start_reg2,
  adtscrc_end_reg1,
  adtscrc_end_reg2,
  drmcrc_start_reg,
  drmcrc_end_reg,
  next_channel,
  next_channel_loop,
  link_sequence,
  end_of_sequence
} rbd_id_t;

struct element_list {
  const rbd_id_t *id;
  const struct element_list *next[2];
};

typedef struct element_list element_list_t;
/**
 * \brief get elementary stream pieces list for given parameters.
 * \param aot audio object type
 * \param epConfig the epConfig value from the current Audio Specific Config
 * \param nChannels amount of channels contained in the current element.
 * \param layer the layer of the current element.
 * \param elFlags element specific flags.
 * \return element_list_t parser guidance structure.
 */
const element_list_t *getBitstreamElementList(AUDIO_OBJECT_TYPE aot,
                                              SCHAR epConfig, UCHAR nChannels,
                                              UCHAR layer, UINT elFlags);

typedef enum {
  /* n.a. */
  FDK_FORMAT_1_0 = 1,     /* mono */
  FDK_FORMAT_2_0 = 2,     /* stereo */
  FDK_FORMAT_3_0_FC = 3,  /* 3/0.0 */
  FDK_FORMAT_3_1_0 = 4,   /* 3/1.0 */
  FDK_FORMAT_5_0 = 5,     /* 3/2.0 */
  FDK_FORMAT_5_1 = 6,     /* 5.1 */
  FDK_FORMAT_7_1_ALT = 7, /* 5/2.1 ALT */
  /* 8 n.a.*/
  FDK_FORMAT_3_0_RC = 9, /* 2/1.0 */
  FDK_FORMAT_2_2_0 = 10, /* 2/2.0 */
  FDK_FORMAT_6_1 = 11,   /* 3/3.1 */
  FDK_FORMAT_7_1 = 12,   /* 3/4.1 */
  FDK_FORMAT_22_2 = 13,  /* 22.2 */
  FDK_FORMAT_5_2_1 = 14, /* 5/2.1*/
  FDK_FORMAT_5_5_2 = 15, /* 5/5.2 */
  FDK_FORMAT_9_1 = 16,   /* 5/4.1 */
  FDK_FORMAT_6_5_1 = 17, /* 6/5.1 */
  FDK_FORMAT_6_7_1 = 18, /* 6/7.1 */
  FDK_FORMAT_5_6_1 = 19, /* 5/6.1 */
  FDK_FORMAT_7_6_1 = 20, /* 7/6.1 */
  FDK_FORMAT_IN_LISTOFCHANNELS = 21,
  FDK_FORMAT_OUT_LISTOFCHANNELS = 22,
  /* 20 formats + In & Out list of channels */
  FDK_NFORMATS = 23,
  FDK_FORMAT_FAIL = -1
} FDK_converter_formatid_t;

extern const INT format_nchan[FDK_NFORMATS + 9 - 2];

#endif
