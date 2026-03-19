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

   Author(s):   Karsten Linzmeier

   Description: Noiseless Coding
                Huffman encoder

*******************************************************************************/

/* Includes ******************************************************************/
#include "sacenc_nlc_enc.h"

#include "genericStds.h"
#include "fixpoint_math.h"

#include "sacenc_const.h"
#include "sacenc_huff_tab.h"
#include "sacenc_paramextract.h"

/* Defines *******************************************************************/
#define PAIR_SHIFT 4
#define PAIR_MASK 0xf

#define PBC_MIN_BANDS 5

typedef enum {
  BACKWARDS = 0x0,
  FORWARDS = 0x1

} DIRECTION;

typedef enum {
  DIFF_FREQ = 0x0,
  DIFF_TIME = 0x1

} DIFF_TYPE;

typedef enum {
  HUFF_1D = 0x0,
  HUFF_2D = 0x1

} CODING_SCHEME;

typedef enum {
  FREQ_PAIR = 0x0,
  TIME_PAIR = 0x1

} PAIRING;

/* Data Types ****************************************************************/

/* Constants *****************************************************************/
static const UCHAR lavHuffVal[4] = {0, 2, 6, 7};
static const UCHAR lavHuffLen[4] = {1, 2, 3, 3};

static const UCHAR lav_step_CLD[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3};
static const UCHAR lav_step_ICC[] = {0, 0, 1, 1, 2, 2, 3, 3};

/* Function / Class Declarations *********************************************/

/* Function / Class Definition ***********************************************/
static void split_lsb(const SHORT *const in_data, SHORT offset,
                      const INT num_val, SHORT *const out_data_lsb,
                      SHORT *const out_data_msb) {
  int i;

  for (i = 0; i < num_val; i++) {
    SHORT val = in_data[i] + offset;
    if (out_data_lsb != NULL) out_data_lsb[i] = val & 0x0001;
    if (out_data_msb != NULL) out_data_msb[i] = val >> 1;
  }
}

static void apply_lsb_coding(HANDLE_FDK_BITSTREAM strm,
                             const SHORT *const in_data_lsb, const UINT num_lsb,
                             const INT num_val) {
  int i;

  for (i = 0; i < num_val; i++) {
    FDKwriteBits(strm, in_data_lsb[i], num_lsb);
  }
}

static void calc_diff_freq(const SHORT *const in_data, SHORT *const out_data,
                           const INT num_val) {
  int i;
  out_data[0] = in_data[0];

  for (i = 1; i < num_val; i++) {
    out_data[i] = in_data[i] - in_data[i - 1];
  }
}

static void calc_diff_time(const SHORT *const in_data,
                           const SHORT *const prev_data, SHORT *const out_data,
                           const INT num_val) {
  int i;
  out_data[0] = in_data[0];
  out_data[1] = prev_data[0];

  for (i = 0; i < num_val; i++) {
    out_data[i + 2] = in_data[i] - prev_data[i];
  }
}

static INT sym_check(SHORT data[2], const INT lav, SHORT *const pSym_bits) {
  UCHAR symBits = 0;
  int sum_val = data[0] + data[1];
  int diff_val = data[0] - data[1];
  int num_sbits = 0;

  if (sum_val != 0) {
    int sum_neg = (sum_val < 0) ? 1 : 0;
    if (sum_neg) {
      sum_val = -sum_val;
      diff_val = -diff_val;
    }
    symBits = (symBits << 1) | sum_neg;
    num_sbits++;
  }

  if (diff_val != 0) {
    int diff_neg = (diff_val < 0) ? 1 : 0;
    if (diff_neg) {
      diff_val = -diff_val;
    }
    symBits = (symBits << 1) | diff_neg;
    num_sbits++;
  }

  if (pSym_bits != NULL) {
    *pSym_bits = symBits;
  }

  if (sum_val % 2) {
    data[0] = lav - sum_val / 2;
    data[1] = lav - diff_val / 2;
  } else {
    data[0] = sum_val / 2;
    data[1] = diff_val / 2;
  }

  return num_sbits;
}

static INT ilog2(UINT i) {
  int l = 0;

  if (i) i--;
  while (i > 0) {
    i >>= 1;
    l++;
  }

  return l;
}

static SHORT calc_pcm_bits(const SHORT num_val, const SHORT num_levels) {
  SHORT num_complete_chunks = 0, rest_chunk_size = 0;
  SHORT max_grp_len = 0, bits_pcm = 0;
  int chunk_levels, i;

  switch (num_levels) {
    case 3:
      max_grp_len = 5;
      break;
    case 6:
      max_grp_len = 5;
      break;
    case 7:
      max_grp_len = 6;
      break;
    case 11:
      max_grp_len = 2;
      break;
    case 13:
      max_grp_len = 4;
      break;
    case 19:
      max_grp_len = 4;
      break;
    case 25:
      max_grp_len = 3;
      break;
    case 51:
      max_grp_len = 4;
      break;
    default:
      max_grp_len = 1;
  }

  num_complete_chunks = num_val / max_grp_len;
  rest_chunk_size = num_val % max_grp_len;

  chunk_levels = 1;
  for (i = 1; i <= max_grp_len; i++) {
    chunk_levels *= num_levels;
  }

  bits_pcm = (SHORT)(ilog2(chunk_levels) * num_complete_chunks);
  bits_pcm += (SHORT)(ilog2(num_levels) * rest_chunk_size);

  return bits_pcm;
}

static void apply_pcm_coding(HANDLE_FDK_BITSTREAM strm,
                             const SHORT *const in_data_1,
                             const SHORT *const in_data_2, const SHORT offset,
                             const SHORT num_val, const SHORT num_levels) {
  SHORT i = 0, j = 0, idx = 0;
  SHORT max_grp_len = 0, grp_len = 0, next_val = 0;
  int grp_val = 0, chunk_levels = 0;

  SHORT pcm_chunk_size[7] = {0};

  switch (num_levels) {
    case 3:
      max_grp_len = 5;
      break;
    case 5:
      max_grp_len = 3;
      break;
    case 6:
      max_grp_len = 5;
      break;
    case 7:
      max_grp_len = 6;
      break;
    case 9:
      max_grp_len = 5;
      break;
    case 11:
      max_grp_len = 2;
      break;
    case 13:
      max_grp_len = 4;
      break;
    case 19:
      max_grp_len = 4;
      break;
    case 25:
      max_grp_len = 3;
      break;
    case 51:
      max_grp_len = 4;
      break;
    default:
      max_grp_len = 1;
  }

  chunk_levels = 1;
  for (i = 1; i <= max_grp_len; i++) {
    chunk_levels *= num_levels;
    pcm_chunk_size[i] = ilog2(chunk_levels);
  }

  for (i = 0; i < num_val; i += max_grp_len) {
    grp_len = FDKmin(max_grp_len, num_val - i);
    grp_val = 0;
    for (j = 0; j < grp_len; j++) {
      idx = i + j;
      if (in_data_2 == NULL) {
        next_val = in_data_1[idx];
      } else if (in_data_1 == NULL) {
        next_val = in_data_2[idx];
      } else {
        next_val = ((idx % 2) ? in_data_2[idx / 2] : in_data_1[idx / 2]);
      }
      next_val += offset;
      grp_val = grp_val * num_levels + next_val;
    }

    FDKwriteBits(strm, grp_val, pcm_chunk_size[grp_len]);
  }
}

static UINT huff_enc_1D(HANDLE_FDK_BITSTREAM strm, const DATA_TYPE data_type,
                        const INT dim1, SHORT *const in_data,
                        const SHORT num_val, const SHORT p0_flag) {
  int i, offset = 0;
  UINT huffBits = 0;

  HUFF_ENTRY part0 = {0};
  const HUFF_ENTRY *pHuffTab = NULL;

  switch (data_type) {
    case t_CLD:
      pHuffTab = fdk_sacenc_huffCLDTab.h1D[dim1];
      break;
    case t_ICC:
      pHuffTab = fdk_sacenc_huffICCTab.h1D[dim1];
      break;
  }

  if (p0_flag) {
    switch (data_type) {
      case t_CLD:
        part0 = fdk_sacenc_huffPart0Tab.cld[in_data[0]];
        break;
      case t_ICC:
        part0 = fdk_sacenc_huffPart0Tab.icc[in_data[0]];
        break;
    }
    huffBits += FDKwriteBits(strm, HUFF_VALUE(part0), HUFF_LENGTH(part0));
    offset = 1;
  }

  for (i = offset; i < num_val; i++) {
    int id_sign = 0;
    int id = in_data[i];

    if (id != 0) {
      id_sign = 0;
      if (id < 0) {
        id = -id;
        id_sign = 1;
      }
    }

    huffBits +=
        FDKwriteBits(strm, HUFF_VALUE(pHuffTab[id]), HUFF_LENGTH(pHuffTab[id]));

    if (id != 0) {
      huffBits += FDKwriteBits(strm, id_sign, 1);
    }
  } /* for i */

  return huffBits;
}

static void getHuffEntry(const INT lav, const DATA_TYPE data_type, const INT i,
                         const SHORT tab_idx_2D[2], const SHORT in_data[][2],
                         HUFF_ENTRY *const pEntry, HUFF_ENTRY *const pEscape) {
  const HUFF_CLD_TAB_2D *pCLD2dTab =
      &fdk_sacenc_huffCLDTab.h2D[tab_idx_2D[0]][tab_idx_2D[1]];
  const HUFF_ICC_TAB_2D *pICC2dTab =
      &fdk_sacenc_huffICCTab.h2D[tab_idx_2D[0]][tab_idx_2D[1]];

  switch (lav) {
    case 1: {
      const LAV1_2D *pLav1 = NULL;
      switch (data_type) {
        case t_CLD:
          pLav1 = NULL;
          break;
        case t_ICC:
          pLav1 = &pICC2dTab->lav1;
          break;
      }
      if (pLav1 != NULL) {
        *pEntry = pLav1->entry[in_data[i][0]][in_data[i][1]];
        *pEscape = pLav1->escape;
      }
    } break;
    case 3: {
      const LAV3_2D *pLav3 = NULL;
      switch (data_type) {
        case t_CLD:
          pLav3 = &pCLD2dTab->lav3;
          break;
        case t_ICC:
          pLav3 = &pICC2dTab->lav3;
          break;
      }
      if (pLav3 != NULL) {
        *pEntry = pLav3->entry[in_data[i][0]][in_data[i][1]];
        *pEscape = pLav3->escape;
      }
    } break;
    case 5: {
      const LAV5_2D *pLav5 = NULL;
      switch (data_type) {
        case t_CLD:
          pLav5 = &pCLD2dTab->lav5;
          break;
        case t_ICC:
          pLav5 = &pICC2dTab->lav5;
          break;
      }
      if (pLav5 != NULL) {
        *pEntry = pLav5->entry[in_data[i][0]][in_data[i][1]];
        *pEscape = pLav5->escape;
      }
    } break;
    case 7: {
      const LAV7_2D *pLav7 = NULL;
      switch (data_type) {
        case t_CLD:
          pLav7 = &pCLD2dTab->lav7;
          break;
        case t_ICC:
          pLav7 = &pICC2dTab->lav7;
          break;
      }
      if (pLav7 != NULL) {
        *pEntry = pLav7->entry[in_data[i][0]][in_data[i][1]];
        *pEscape = pLav7->escape;
      }
    } break;
    case 9: {
      const LAV9_2D *pLav9 = NULL;
      switch (data_type) {
        case t_CLD:
          pLav9 = &pCLD2dTab->lav9;
          break;
        case t_ICC:
          pLav9 = NULL;
          break;
      }
      if (pLav9 != NULL) {
        *pEntry = pLav9->entry[in_data[i][0]][in_data[i][1]];
        *pEscape = pLav9->escape;
      }
    } break;
  }
}

static UINT huff_enc_2D(HANDLE_FDK_BITSTREAM strm, const DATA_TYPE data_type,
                        SHORT tab_idx_2D[2], SHORT lav_idx, SHORT in_data[][2],
                        SHORT num_val, SHORT stride, SHORT *p0_data[2]) {
  SHORT i = 0, lav = 0, num_sbits = 0, sym_bits = 0, escIdx = 0;
  SHORT esc_data[2][MAXBANDS] = {{0}};

  UINT huffBits = 0;

  const HUFF_ENTRY *pHuffEntry = NULL;

  switch (data_type) {
    case t_CLD:
      lav = 2 * lav_idx + 3; /* LAV */
      pHuffEntry = fdk_sacenc_huffPart0Tab.cld;
      break;
    case t_ICC:
      lav = 2 * lav_idx + 1; /* LAV */
      pHuffEntry = fdk_sacenc_huffPart0Tab.icc;
      break;
  }

  /* Partition 0 */
  if (p0_data[0] != NULL) {
    HUFF_ENTRY entry = pHuffEntry[*p0_data[0]];
    huffBits += FDKwriteBits(strm, HUFF_VALUE(entry), HUFF_LENGTH(entry));
  }
  if (p0_data[1] != NULL) {
    HUFF_ENTRY entry = pHuffEntry[*p0_data[1]];
    huffBits += FDKwriteBits(strm, HUFF_VALUE(entry), HUFF_LENGTH(entry));
  }

  for (i = 0; i < num_val; i += stride) {
    HUFF_ENTRY entry = {0};
    HUFF_ENTRY escape = {0};

    esc_data[0][escIdx] = in_data[i][0] + lav;
    esc_data[1][escIdx] = in_data[i][1] + lav;

    num_sbits = sym_check(in_data[i], lav, &sym_bits);

    getHuffEntry(lav, data_type, i, tab_idx_2D, in_data, &entry, &escape);

    huffBits += FDKwriteBits(strm, HUFF_VALUE(entry), HUFF_LENGTH(entry));

    if ((HUFF_VALUE(entry) == HUFF_VALUE(escape)) &&
        (HUFF_LENGTH(entry) == HUFF_LENGTH(escape))) {
      escIdx++;
    } else {
      huffBits += FDKwriteBits(strm, sym_bits, num_sbits);
    }
  } /* for i */

  if (escIdx > 0) {
    huffBits += calc_pcm_bits(2 * escIdx, (2 * lav + 1));
    if (strm != NULL) {
      apply_pcm_coding(strm, esc_data[0], esc_data[1], 0 /*offset*/, 2 * escIdx,
                       (2 * lav + 1));
    }
  }

  return huffBits;
}

static SCHAR get_next_lav_step(const INT lav, const DATA_TYPE data_type) {
  SCHAR lav_step = 0;

  switch (data_type) {
    case t_CLD:
      lav_step = (lav > 9) ? -1 : lav_step_CLD[lav];
      break;
    case t_ICC:
      lav_step = (lav > 7) ? -1 : lav_step_ICC[lav];
      break;
  }

  return lav_step;
}

static INT diff_type_offset(const DIFF_TYPE diff_type) {
  int offset = 0;
  switch (diff_type) {
    case DIFF_FREQ:
      offset = 0;
      break;
    case DIFF_TIME:
      offset = 2;
      break;
  }
  return offset;
}

static SHORT calc_huff_bits(SHORT *in_data_1, SHORT *in_data_2,
                            const DATA_TYPE data_type,
                            const DIFF_TYPE diff_type_1,
                            const DIFF_TYPE diff_type_2, const SHORT num_val,
                            SHORT *const lav_idx, SHORT *const cdg_scheme) {
  SHORT tab_idx_2D[2][2] = {{0}};
  SHORT tab_idx_1D[2] = {0};
  SHORT df_rest_flag[2] = {0};
  SHORT p0_flag[2] = {0};

  SHORT pair_vec[MAXBANDS][2] = {{0}};

  SHORT *p0_data_1[2] = {NULL};
  SHORT *p0_data_2[2] = {NULL};

  SHORT i = 0;
  SHORT lav_fp[2] = {0};

  SHORT bit_count_1D = 0;
  SHORT bit_count_2D_freq = 0;
  SHORT bit_count_min = 0;

  SHORT num_val_1_short = 0;
  SHORT num_val_2_short = 0;

  SHORT *in_data_1_short = NULL;
  SHORT *in_data_2_short = NULL;

  /* 1D Huffman coding */
  bit_count_1D = 1; /* HUFF_1D */

  num_val_1_short = num_val;
  num_val_2_short = num_val;

  if (in_data_1 != NULL) {
    in_data_1_short = in_data_1 + diff_type_offset(diff_type_1);
  }
  if (in_data_2 != NULL) {
    in_data_2_short = in_data_2 + diff_type_offset(diff_type_2);
  }

  p0_flag[0] = (diff_type_1 == DIFF_FREQ);
  p0_flag[1] = (diff_type_2 == DIFF_FREQ);

  tab_idx_1D[0] = (diff_type_1 == DIFF_FREQ) ? 0 : 1;
  tab_idx_1D[1] = (diff_type_2 == DIFF_FREQ) ? 0 : 1;

  if (in_data_1 != NULL) {
    bit_count_1D += huff_enc_1D(NULL, data_type, tab_idx_1D[0], in_data_1_short,
                                num_val_1_short, p0_flag[0]);
  }
  if (in_data_2 != NULL) {
    bit_count_1D += huff_enc_1D(NULL, data_type, tab_idx_1D[1], in_data_2_short,
                                num_val_2_short, p0_flag[1]);
  }

  bit_count_min = bit_count_1D;
  *cdg_scheme = HUFF_1D << PAIR_SHIFT;
  lav_idx[0] = lav_idx[1] = -1;

  /* Huffman 2D frequency pairs */
  bit_count_2D_freq = 1; /* HUFF_2D */

  num_val_1_short = num_val;
  num_val_2_short = num_val;

  if (in_data_1 != NULL) {
    in_data_1_short = in_data_1 + diff_type_offset(diff_type_1);
  }
  if (in_data_2 != NULL) {
    in_data_2_short = in_data_2 + diff_type_offset(diff_type_2);
  }

  lav_fp[0] = lav_fp[1] = 0;

  p0_data_1[0] = NULL;
  p0_data_1[1] = NULL;
  p0_data_2[0] = NULL;
  p0_data_2[1] = NULL;

  if (in_data_1 != NULL) {
    if (diff_type_1 == DIFF_FREQ) {
      p0_data_1[0] = &in_data_1[0];
      p0_data_1[1] = NULL;

      num_val_1_short -= 1;
      in_data_1_short += 1;
    }

    df_rest_flag[0] = num_val_1_short % 2;

    if (df_rest_flag[0]) num_val_1_short -= 1;

    for (i = 0; i < num_val_1_short - 1; i += 2) {
      pair_vec[i][0] = in_data_1_short[i];
      pair_vec[i][1] = in_data_1_short[i + 1];

      lav_fp[0] = FDKmax(lav_fp[0], fAbs(pair_vec[i][0]));
      lav_fp[0] = FDKmax(lav_fp[0], fAbs(pair_vec[i][1]));
    }

    tab_idx_2D[0][0] = (diff_type_1 == DIFF_TIME) ? 1 : 0;
    tab_idx_2D[0][1] = 0;

    tab_idx_1D[0] = (diff_type_1 == DIFF_FREQ) ? 0 : 1;

    lav_fp[0] = get_next_lav_step(lav_fp[0], data_type);

    if (lav_fp[0] != -1) bit_count_2D_freq += lavHuffLen[lav_fp[0]];
  }

  if (in_data_2 != NULL) {
    if (diff_type_2 == DIFF_FREQ) {
      p0_data_2[0] = NULL;
      p0_data_2[1] = &in_data_2[0];

      num_val_2_short -= 1;
      in_data_2_short += 1;
    }

    df_rest_flag[1] = num_val_2_short % 2;

    if (df_rest_flag[1]) num_val_2_short -= 1;

    for (i = 0; i < num_val_2_short - 1; i += 2) {
      pair_vec[i + 1][0] = in_data_2_short[i];
      pair_vec[i + 1][1] = in_data_2_short[i + 1];

      lav_fp[1] = FDKmax(lav_fp[1], fAbs(pair_vec[i + 1][0]));
      lav_fp[1] = FDKmax(lav_fp[1], fAbs(pair_vec[i + 1][1]));
    }

    tab_idx_2D[1][0] = (diff_type_2 == DIFF_TIME) ? 1 : 0;
    tab_idx_2D[1][1] = 0;

    tab_idx_1D[1] = (diff_type_2 == DIFF_FREQ) ? 0 : 1;

    lav_fp[1] = get_next_lav_step(lav_fp[1], data_type);

    if (lav_fp[1] != -1) bit_count_2D_freq += lavHuffLen[lav_fp[1]];
  }

  if ((lav_fp[0] != -1) && (lav_fp[1] != -1)) {
    if (in_data_1 != NULL) {
      bit_count_2D_freq +=
          huff_enc_2D(NULL, data_type, tab_idx_2D[0], lav_fp[0], pair_vec,
                      num_val_1_short, 2, p0_data_1);
    }
    if (in_data_2 != NULL) {
      bit_count_2D_freq +=
          huff_enc_2D(NULL, data_type, tab_idx_2D[1], lav_fp[1], pair_vec + 1,
                      num_val_2_short, 2, p0_data_2);
    }
    if (in_data_1 != NULL) {
      if (df_rest_flag[0])
        bit_count_2D_freq +=
            huff_enc_1D(NULL, data_type, tab_idx_1D[0],
                        in_data_1_short + num_val_1_short, 1, 0);
    }
    if (in_data_2 != NULL) {
      if (df_rest_flag[1])
        bit_count_2D_freq +=
            huff_enc_1D(NULL, data_type, tab_idx_1D[1],
                        in_data_2_short + num_val_2_short, 1, 0);
    }

    if (bit_count_2D_freq < bit_count_min) {
      bit_count_min = bit_count_2D_freq;
      *cdg_scheme = HUFF_2D << PAIR_SHIFT | FREQ_PAIR;
      lav_idx[0] = lav_fp[0];
      lav_idx[1] = lav_fp[1];
    }
  }

  return bit_count_min;
}

static void apply_huff_coding(HANDLE_FDK_BITSTREAM strm, SHORT *const in_data_1,
                              SHORT *const in_data_2, const DATA_TYPE data_type,
                              const DIFF_TYPE diff_type_1,
                              const DIFF_TYPE diff_type_2, const SHORT num_val,
                              const SHORT *const lav_idx,
                              const SHORT cdg_scheme) {
  SHORT tab_idx_2D[2][2] = {{0}};
  SHORT tab_idx_1D[2] = {0};
  SHORT df_rest_flag[2] = {0};
  SHORT p0_flag[2] = {0};

  SHORT pair_vec[MAXBANDS][2] = {{0}};

  SHORT *p0_data_1[2] = {NULL};
  SHORT *p0_data_2[2] = {NULL};

  SHORT i = 0;

  SHORT num_val_1_short = num_val;
  SHORT num_val_2_short = num_val;

  SHORT *in_data_1_short = NULL;
  SHORT *in_data_2_short = NULL;

  /* Offset */
  if (in_data_1 != NULL) {
    in_data_1_short = in_data_1 + diff_type_offset(diff_type_1);
  }
  if (in_data_2 != NULL) {
    in_data_2_short = in_data_2 + diff_type_offset(diff_type_2);
  }

  /* Signalize coding scheme */
  FDKwriteBits(strm, cdg_scheme >> PAIR_SHIFT, 1);

  switch (cdg_scheme >> PAIR_SHIFT) {
    case HUFF_1D:

      p0_flag[0] = (diff_type_1 == DIFF_FREQ);
      p0_flag[1] = (diff_type_2 == DIFF_FREQ);

      tab_idx_1D[0] = (diff_type_1 == DIFF_FREQ) ? 0 : 1;
      tab_idx_1D[1] = (diff_type_2 == DIFF_FREQ) ? 0 : 1;

      if (in_data_1 != NULL) {
        huff_enc_1D(strm, data_type, tab_idx_1D[0], in_data_1_short,
                    num_val_1_short, p0_flag[0]);
      }
      if (in_data_2 != NULL) {
        huff_enc_1D(strm, data_type, tab_idx_1D[1], in_data_2_short,
                    num_val_2_short, p0_flag[1]);
      }
      break; /* HUFF_1D */

    case HUFF_2D:

      switch (cdg_scheme & PAIR_MASK) {
        case FREQ_PAIR:

          if (in_data_1 != NULL) {
            if (diff_type_1 == DIFF_FREQ) {
              p0_data_1[0] = &in_data_1[0];
              p0_data_1[1] = NULL;

              num_val_1_short -= 1;
              in_data_1_short += 1;
            }

            df_rest_flag[0] = num_val_1_short % 2;

            if (df_rest_flag[0]) num_val_1_short -= 1;

            for (i = 0; i < num_val_1_short - 1; i += 2) {
              pair_vec[i][0] = in_data_1_short[i];
              pair_vec[i][1] = in_data_1_short[i + 1];
            }

            tab_idx_2D[0][0] = (diff_type_1 == DIFF_TIME) ? 1 : 0;
            tab_idx_2D[0][1] = 0;

            tab_idx_1D[0] = (diff_type_1 == DIFF_FREQ) ? 0 : 1;
          } /* if( in_data_1 != NULL ) */

          if (in_data_2 != NULL) {
            if (diff_type_2 == DIFF_FREQ) {
              p0_data_2[0] = NULL;
              p0_data_2[1] = &in_data_2[0];

              num_val_2_short -= 1;
              in_data_2_short += 1;
            }

            df_rest_flag[1] = num_val_2_short % 2;

            if (df_rest_flag[1]) num_val_2_short -= 1;

            for (i = 0; i < num_val_2_short - 1; i += 2) {
              pair_vec[i + 1][0] = in_data_2_short[i];
              pair_vec[i + 1][1] = in_data_2_short[i + 1];
            }

            tab_idx_2D[1][0] = (diff_type_2 == DIFF_TIME) ? 1 : 0;
            tab_idx_2D[1][1] = 0;

            tab_idx_1D[1] = (diff_type_2 == DIFF_FREQ) ? 0 : 1;
          } /* if( in_data_2 != NULL ) */

          if (in_data_1 != NULL) {
            FDKwriteBits(strm, lavHuffVal[lav_idx[0]], lavHuffLen[lav_idx[0]]);
            huff_enc_2D(strm, data_type, tab_idx_2D[0], lav_idx[0], pair_vec,
                        num_val_1_short, 2, p0_data_1);
            if (df_rest_flag[0]) {
              huff_enc_1D(strm, data_type, tab_idx_1D[0],
                          in_data_1_short + num_val_1_short, 1, 0);
            }
          }
          if (in_data_2 != NULL) {
            FDKwriteBits(strm, lavHuffVal[lav_idx[1]], lavHuffLen[lav_idx[1]]);
            huff_enc_2D(strm, data_type, tab_idx_2D[1], lav_idx[1],
                        pair_vec + 1, num_val_2_short, 2, p0_data_2);
            if (df_rest_flag[1]) {
              huff_enc_1D(strm, data_type, tab_idx_1D[1],
                          in_data_2_short + num_val_2_short, 1, 0);
            }
          }
          break; /* FREQ_PAIR */

        case TIME_PAIR:

          if ((diff_type_1 == DIFF_FREQ) || (diff_type_2 == DIFF_FREQ)) {
            p0_data_1[0] = &in_data_1[0];
            p0_data_1[1] = &in_data_2[0];

            in_data_1_short += 1;
            in_data_2_short += 1;

            num_val_1_short -= 1;
          }

          for (i = 0; i < num_val_1_short; i++) {
            pair_vec[i][0] = in_data_1_short[i];
            pair_vec[i][1] = in_data_2_short[i];
          }

          tab_idx_2D[0][0] =
              ((diff_type_1 == DIFF_TIME) || (diff_type_2 == DIFF_TIME)) ? 1
                                                                         : 0;
          tab_idx_2D[0][1] = 1;

          FDKwriteBits(strm, lavHuffVal[lav_idx[0]], lavHuffLen[lav_idx[0]]);

          huff_enc_2D(strm, data_type, tab_idx_2D[0], lav_idx[0], pair_vec,
                      num_val_1_short, 1, p0_data_1);

          break; /* TIME_PAIR */
      }          /* switch( cdg_scheme & PAIR_MASK ) */

      break; /* HUFF_2D */

    default:
      break;
  } /* switch( cdg_scheme >> PAIR_SHIFT ) */
}

INT fdk_sacenc_ecDataPairEnc(HANDLE_FDK_BITSTREAM strm,
                             SHORT aaInData[][MAXBANDS],
                             SHORT aHistory[MAXBANDS],
                             const DATA_TYPE data_type, const INT setIdx,
                             const INT startBand, const INT dataBands,
                             const INT coarse_flag,
                             const INT independency_flag) {
  SHORT reset = 0, pb = 0;
  SHORT quant_levels = 0, quant_offset = 0, num_pcm_val = 0;

  SHORT splitLsb_flag = 0;
  SHORT pcmCoding_flag = 0;

  SHORT allowDiffTimeBack_flag = !independency_flag || (setIdx > 0);

  SHORT num_lsb_bits = -1;
  SHORT num_pcm_bits = -1;

  SHORT quant_data_lsb[2][MAXBANDS];
  SHORT quant_data_msb[2][MAXBANDS];

  SHORT quant_data_hist_lsb[MAXBANDS];
  SHORT quant_data_hist_msb[MAXBANDS];

  SHORT data_diff_freq[2][MAXBANDS];
  SHORT data_diff_time[2][MAXBANDS + 2];

  SHORT *p_quant_data_msb[2];
  SHORT *p_quant_data_hist_msb = NULL;

  SHORT min_bits_all = 0;
  SHORT min_found = 0;

  SHORT min_bits_df_df = -1;
  SHORT min_bits_df_dt = -1;
  SHORT min_bits_dtbw_df = -1;
  SHORT min_bits_dt_dt = -1;

  SHORT lav_df_df[2] = {-1, -1};
  SHORT lav_df_dt[2] = {-1, -1};
  SHORT lav_dtbw_df[2] = {-1, -1};
  SHORT lav_dt_dt[2] = {-1, -1};

  SHORT coding_scheme_df_df = 0;
  SHORT coding_scheme_df_dt = 0;
  SHORT coding_scheme_dtbw_df = 0;
  SHORT coding_scheme_dt_dt = 0;

  switch (data_type) {
    case t_CLD:
      if (coarse_flag) {
        splitLsb_flag = 0;
        quant_levels = 15;
        quant_offset = 7;
      } else {
        splitLsb_flag = 0;
        quant_levels = 31;
        quant_offset = 15;
      }
      break;
    case t_ICC:
      if (coarse_flag) {
        splitLsb_flag = 0;
        quant_levels = 4;
        quant_offset = 0;
      } else {
        splitLsb_flag = 0;
        quant_levels = 8;
        quant_offset = 0;
      }
      break;
  } /* switch( data_type ) */

  /* Split off LSB */
  if (splitLsb_flag) {
    split_lsb(aaInData[setIdx] + startBand, quant_offset, dataBands,
              quant_data_lsb[0], quant_data_msb[0]);

    split_lsb(aaInData[setIdx + 1] + startBand, quant_offset, dataBands,
              quant_data_lsb[1], quant_data_msb[1]);

    p_quant_data_msb[0] = quant_data_msb[0];
    p_quant_data_msb[1] = quant_data_msb[1];

    num_lsb_bits = 2 * dataBands;
  } else if (quant_offset != 0) {
    for (pb = 0; pb < dataBands; pb++) {
      quant_data_msb[0][pb] = aaInData[setIdx][startBand + pb] + quant_offset;
      quant_data_msb[1][pb] =
          aaInData[setIdx + 1][startBand + pb] + quant_offset;
    }

    p_quant_data_msb[0] = quant_data_msb[0];
    p_quant_data_msb[1] = quant_data_msb[1];

    num_lsb_bits = 0;
  } else {
    p_quant_data_msb[0] = aaInData[setIdx] + startBand;
    p_quant_data_msb[1] = aaInData[setIdx + 1] + startBand;

    num_lsb_bits = 0;
  }

  if (allowDiffTimeBack_flag) {
    if (splitLsb_flag) {
      split_lsb(aHistory + startBand, quant_offset, dataBands,
                quant_data_hist_lsb, quant_data_hist_msb);

      p_quant_data_hist_msb = quant_data_hist_msb;
    } else if (quant_offset != 0) {
      for (pb = 0; pb < dataBands; pb++) {
        quant_data_hist_msb[pb] = aHistory[startBand + pb] + quant_offset;
      }
      p_quant_data_hist_msb = quant_data_hist_msb;
    } else {
      p_quant_data_hist_msb = aHistory + startBand;
    }
  }

  /* Calculate frequency differences */
  calc_diff_freq(p_quant_data_msb[0], data_diff_freq[0], dataBands);

  calc_diff_freq(p_quant_data_msb[1], data_diff_freq[1], dataBands);

  /* Calculate time differences */
  if (allowDiffTimeBack_flag) {
    calc_diff_time(p_quant_data_msb[0], p_quant_data_hist_msb,
                   data_diff_time[0], dataBands);
  }

  calc_diff_time(p_quant_data_msb[1], p_quant_data_msb[0], data_diff_time[1],
                 dataBands);

  /* Calculate coding scheme with minumum bit consumption */

  /**********************************************************/
  num_pcm_bits = calc_pcm_bits(2 * dataBands, quant_levels);
  num_pcm_val = 2 * dataBands;

  /**********************************************************/

  min_bits_all = num_pcm_bits;

  /**********************************************************/
  /**********************************************************/

  /**********************************************************/
  min_bits_df_df =
      calc_huff_bits(data_diff_freq[0], data_diff_freq[1], data_type, DIFF_FREQ,
                     DIFF_FREQ, dataBands, lav_df_df, &coding_scheme_df_df);

  min_bits_df_df += 2;

  min_bits_df_df += num_lsb_bits;

  if (min_bits_df_df < min_bits_all) {
    min_bits_all = min_bits_df_df;
  }
  /**********************************************************/

  /**********************************************************/
  min_bits_df_dt =
      calc_huff_bits(data_diff_freq[0], data_diff_time[1], data_type, DIFF_FREQ,
                     DIFF_TIME, dataBands, lav_df_dt, &coding_scheme_df_dt);

  min_bits_df_dt += 2;

  min_bits_df_dt += num_lsb_bits;

  if (min_bits_df_dt < min_bits_all) {
    min_bits_all = min_bits_df_dt;
  }
  /**********************************************************/

  /**********************************************************/
  /**********************************************************/

  if (allowDiffTimeBack_flag) {
    /**********************************************************/
    min_bits_dtbw_df = calc_huff_bits(
        data_diff_time[0], data_diff_freq[1], data_type, DIFF_TIME, DIFF_FREQ,
        dataBands, lav_dtbw_df, &coding_scheme_dtbw_df);

    min_bits_dtbw_df += 2;

    min_bits_dtbw_df += num_lsb_bits;

    if (min_bits_dtbw_df < min_bits_all) {
      min_bits_all = min_bits_dtbw_df;
    }
    /**********************************************************/

    /**********************************************************/
    min_bits_dt_dt = calc_huff_bits(data_diff_time[0], data_diff_time[1],
                                    data_type, DIFF_TIME, DIFF_TIME, dataBands,
                                    lav_dt_dt, &coding_scheme_dt_dt);

    min_bits_dt_dt += 2;

    min_bits_dt_dt += num_lsb_bits;

    if (min_bits_dt_dt < min_bits_all) {
      min_bits_all = min_bits_dt_dt;
    }
    /**********************************************************/

  } /* if( allowDiffTimeBack_flag ) */

  /***************************/
  /* Start actual coding now */
  /***************************/

  /* PCM or Diff/Huff Coding? */
  pcmCoding_flag = (min_bits_all == num_pcm_bits);

  FDKwriteBits(strm, pcmCoding_flag, 1);

  if (pcmCoding_flag) {
    /* Grouped PCM Coding */
    apply_pcm_coding(strm, aaInData[setIdx] + startBand,
                     aaInData[setIdx + 1] + startBand, quant_offset,
                     num_pcm_val, quant_levels);
  } else {
    /* Diff/Huff Coding */

    min_found = 0;

    /*******************************************/
    if (min_bits_all == min_bits_df_df) {
      FDKwriteBits(strm, DIFF_FREQ, 1);
      FDKwriteBits(strm, DIFF_FREQ, 1);

      apply_huff_coding(strm, data_diff_freq[0], data_diff_freq[1], data_type,
                        DIFF_FREQ, DIFF_FREQ, dataBands, lav_df_df,
                        coding_scheme_df_df);

      min_found = 1;
    }
    /*******************************************/

    /*******************************************/
    if (!min_found && (min_bits_all == min_bits_df_dt)) {
      FDKwriteBits(strm, DIFF_FREQ, 1);
      FDKwriteBits(strm, DIFF_TIME, 1);

      apply_huff_coding(strm, data_diff_freq[0], data_diff_time[1], data_type,
                        DIFF_FREQ, DIFF_TIME, dataBands, lav_df_dt,
                        coding_scheme_df_dt);

      min_found = 1;
    }
    /*******************************************/

    /*******************************************/
    /*******************************************/

    if (allowDiffTimeBack_flag) {
      /*******************************************/
      if (!min_found && (min_bits_all == min_bits_dtbw_df)) {
        FDKwriteBits(strm, DIFF_TIME, 1);
        FDKwriteBits(strm, DIFF_FREQ, 1);

        apply_huff_coding(strm, data_diff_time[0], data_diff_freq[1], data_type,
                          DIFF_TIME, DIFF_FREQ, dataBands, lav_dtbw_df,
                          coding_scheme_dtbw_df);

        min_found = 1;
      }
      /*******************************************/

      /*******************************************/
      if (!min_found && (min_bits_all == min_bits_dt_dt)) {
        FDKwriteBits(strm, DIFF_TIME, 1);
        FDKwriteBits(strm, DIFF_TIME, 1);

        apply_huff_coding(strm, data_diff_time[0], data_diff_time[1], data_type,
                          DIFF_TIME, DIFF_TIME, dataBands, lav_dt_dt,
                          coding_scheme_dt_dt);
      }
      /*******************************************/

    } /* if( allowDiffTimeBack_flag ) */

    /* LSB coding */
    if (splitLsb_flag) {
      apply_lsb_coding(strm, quant_data_lsb[0], 1, dataBands);

      apply_lsb_coding(strm, quant_data_lsb[1], 1, dataBands);
    }

  } /* Diff/Huff/LSB coding */

  return reset;
}

INT fdk_sacenc_ecDataSingleEnc(HANDLE_FDK_BITSTREAM strm,
                               SHORT aaInData[][MAXBANDS],
                               SHORT aHistory[MAXBANDS],
                               const DATA_TYPE data_type, const INT setIdx,
                               const INT startBand, const INT dataBands,
                               const INT coarse_flag,
                               const INT independency_flag) {
  SHORT reset = 0, pb = 0;
  SHORT quant_levels = 0, quant_offset = 0, num_pcm_val = 0;

  SHORT splitLsb_flag = 0;
  SHORT pcmCoding_flag = 0;

  SHORT allowDiffTimeBack_flag = !independency_flag || (setIdx > 0);

  SHORT num_lsb_bits = -1;
  SHORT num_pcm_bits = -1;

  SHORT quant_data_lsb[MAXBANDS];
  SHORT quant_data_msb[MAXBANDS];

  SHORT quant_data_hist_lsb[MAXBANDS];
  SHORT quant_data_hist_msb[MAXBANDS];

  SHORT data_diff_freq[MAXBANDS];
  SHORT data_diff_time[MAXBANDS + 2];

  SHORT *p_quant_data_msb;
  SHORT *p_quant_data_hist_msb = NULL;

  SHORT min_bits_all = 0;
  SHORT min_found = 0;

  SHORT min_bits_df = -1;
  SHORT min_bits_dt = -1;

  SHORT lav_df[2] = {-1, -1};
  SHORT lav_dt[2] = {-1, -1};

  SHORT coding_scheme_df = 0;
  SHORT coding_scheme_dt = 0;

  switch (data_type) {
    case t_CLD:
      if (coarse_flag) {
        splitLsb_flag = 0;
        quant_levels = 15;
        quant_offset = 7;
      } else {
        splitLsb_flag = 0;
        quant_levels = 31;
        quant_offset = 15;
      }
      break;
    case t_ICC:
      if (coarse_flag) {
        splitLsb_flag = 0;
        quant_levels = 4;
        quant_offset = 0;
      } else {
        splitLsb_flag = 0;
        quant_levels = 8;
        quant_offset = 0;
      }
      break;
  } /* switch( data_type ) */

  /* Split off LSB */
  if (splitLsb_flag) {
    split_lsb(aaInData[setIdx] + startBand, quant_offset, dataBands,
              quant_data_lsb, quant_data_msb);

    p_quant_data_msb = quant_data_msb;
    num_lsb_bits = dataBands;
  } else if (quant_offset != 0) {
    for (pb = 0; pb < dataBands; pb++) {
      quant_data_msb[pb] = aaInData[setIdx][startBand + pb] + quant_offset;
    }

    p_quant_data_msb = quant_data_msb;
    num_lsb_bits = 0;
  } else {
    p_quant_data_msb = aaInData[setIdx] + startBand;
    num_lsb_bits = 0;
  }

  if (allowDiffTimeBack_flag) {
    if (splitLsb_flag) {
      split_lsb(aHistory + startBand, quant_offset, dataBands,
                quant_data_hist_lsb, quant_data_hist_msb);

      p_quant_data_hist_msb = quant_data_hist_msb;
    } else if (quant_offset != 0) {
      for (pb = 0; pb < dataBands; pb++) {
        quant_data_hist_msb[pb] = aHistory[startBand + pb] + quant_offset;
      }
      p_quant_data_hist_msb = quant_data_hist_msb;
    } else {
      p_quant_data_hist_msb = aHistory + startBand;
    }
  }

  /* Calculate frequency differences */
  calc_diff_freq(p_quant_data_msb, data_diff_freq, dataBands);

  /* Calculate time differences */
  if (allowDiffTimeBack_flag) {
    calc_diff_time(p_quant_data_msb, p_quant_data_hist_msb, data_diff_time,
                   dataBands);
  }

  /* Calculate coding scheme with minumum bit consumption */

  /**********************************************************/
  num_pcm_bits = calc_pcm_bits(dataBands, quant_levels);
  num_pcm_val = dataBands;

  /**********************************************************/

  min_bits_all = num_pcm_bits;

  /**********************************************************/
  /**********************************************************/

  /**********************************************************/
  min_bits_df = calc_huff_bits(data_diff_freq, NULL, data_type, DIFF_FREQ,
                               DIFF_FREQ, dataBands, lav_df, &coding_scheme_df);

  if (allowDiffTimeBack_flag) min_bits_df += 1;

  min_bits_df += num_lsb_bits;

  if (min_bits_df < min_bits_all) {
    min_bits_all = min_bits_df;
  }
  /**********************************************************/

  /**********************************************************/
  if (allowDiffTimeBack_flag) {
    min_bits_dt =
        calc_huff_bits(data_diff_time, NULL, data_type, DIFF_TIME, DIFF_TIME,
                       dataBands, lav_dt, &coding_scheme_dt);

    min_bits_dt += 1;
    min_bits_dt += num_lsb_bits;

    if (min_bits_dt < min_bits_all) {
      min_bits_all = min_bits_dt;
    }
  } /* if( allowDiffTimeBack_flag ) */

  /***************************/
  /* Start actual coding now */
  /***************************/

  /* PCM or Diff/Huff Coding? */
  pcmCoding_flag = (min_bits_all == num_pcm_bits);

  FDKwriteBits(strm, pcmCoding_flag, 1);

  if (pcmCoding_flag) {
    /* Grouped PCM Coding */
    apply_pcm_coding(strm, aaInData[setIdx] + startBand, NULL, quant_offset,
                     num_pcm_val, quant_levels);
  } else {
    /* Diff/Huff Coding */

    min_found = 0;

    /*******************************************/
    if (min_bits_all == min_bits_df) {
      if (allowDiffTimeBack_flag) {
        FDKwriteBits(strm, DIFF_FREQ, 1);
      }

      apply_huff_coding(strm, data_diff_freq, NULL, data_type, DIFF_FREQ,
                        DIFF_FREQ, dataBands, lav_df, coding_scheme_df);

      min_found = 1;
    } /* if( min_bits_all == min_bits_df ) */
    /*******************************************/

    /*******************************************/
    if (allowDiffTimeBack_flag) {
      /*******************************************/
      if (!min_found && (min_bits_all == min_bits_dt)) {
        FDKwriteBits(strm, DIFF_TIME, 1);

        apply_huff_coding(strm, data_diff_time, NULL, data_type, DIFF_TIME,
                          DIFF_TIME, dataBands, lav_dt, coding_scheme_dt);
      }
      /*******************************************/

    } /* if( allowDiffTimeBack_flag ) */

    /* LSB coding */
    if (splitLsb_flag) {
      apply_lsb_coding(strm, quant_data_lsb, 1, dataBands);
    }

  } /* Diff/Huff/LSB coding */

  return reset;
}
