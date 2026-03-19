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

   Author(s):   Omer Osman

   Description: SAC/SAOC Dec Noiseless Coding

*******************************************************************************/

#include "nlc_dec.h"
#include "FDK_tools_rom.h"

/* MAX_PARAMETER_BANDS defines array length in huffdec */

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

ERROR_t sym_restoreIPD(HANDLE_FDK_BITSTREAM strm, int lav, SCHAR data[2]) {
  int sum_val = data[0] + data[1];
  int diff_val = data[0] - data[1];

  if (sum_val > lav) {
    data[0] = -sum_val + (2 * lav + 1);
    data[1] = -diff_val;
  } else {
    data[0] = sum_val;
    data[1] = diff_val;
  }

  if (data[0] - data[1] != 0) {
    ULONG sym_bit;
    sym_bit = FDKreadBits(strm, 1);
    if (sym_bit) {
      int tmp;
      tmp = data[0];
      data[0] = data[1];
      data[1] = tmp;
    }
  }

  return HUFFDEC_OK;
}

static int ilog2(unsigned int i) {
  int l = 0;

  if (i) i--;
  while (i > 0) {
    i >>= 1;
    l++;
  }

  return l;
}

static ERROR_t pcm_decode(HANDLE_FDK_BITSTREAM strm, SCHAR* out_data_1,
                          SCHAR* out_data_2, int offset, int num_val,
                          int num_levels) {
  int i = 0, j = 0, idx = 0;
  int max_grp_len = 0, next_val = 0;
  ULONG tmp;

  int pcm_chunk_size[7] = {0};

  switch (num_levels) {
    case 3:
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
    case 4:
    case 8:
    case 15:
    case 16:
    case 26:
    case 31:
      max_grp_len = 1;
      break;
    default:
      return HUFFDEC_NOTOK;
  }

  tmp = 1;
  for (i = 1; i <= max_grp_len; i++) {
    tmp *= num_levels;
    pcm_chunk_size[i] = ilog2(tmp);
  }

  for (i = 0; i < num_val; i += max_grp_len) {
    int grp_len, grp_val, data;
    grp_len = min(max_grp_len, num_val - i);
    data = FDKreadBits(strm, pcm_chunk_size[grp_len]);

    grp_val = data;

    for (j = 0; j < grp_len; j++) {
      idx = i + (grp_len - j - 1);
      next_val = grp_val % num_levels;

      if (out_data_2 == NULL) {
        out_data_1[idx] = next_val - offset;
      } else if (out_data_1 == NULL) {
        out_data_2[idx] = next_val - offset;
      } else {
        if (idx % 2) {
          out_data_2[idx / 2] = next_val - offset;
        } else {
          out_data_1[idx / 2] = next_val - offset;
        }
      }

      grp_val = (grp_val - next_val) / num_levels;
    }
  }

  return HUFFDEC_OK;
}

static ERROR_t huff_read(HANDLE_FDK_BITSTREAM strm,
                         const SHORT (*nodeTab)[MAX_ENTRIES][2],
                         int* out_data) {
  int node = 0;
  int len = 0;

  do {
    ULONG next_bit;
    next_bit = FDKreadBits(strm, 1);
    len++;
    node = (*nodeTab)[node][next_bit];
  } while (node > 0);

  *out_data = node;

  return HUFFDEC_OK;
}

static ERROR_t huff_read_2D(HANDLE_FDK_BITSTREAM strm,
                            const SHORT (*nodeTab)[MAX_ENTRIES][2],
                            SCHAR out_data[2], int* escape) {
  ERROR_t err = HUFFDEC_OK;

  int huff_2D_8bit = 0;
  int node = 0;

  if ((err = huff_read(strm, nodeTab, &node)) != HUFFDEC_OK) {
    goto bail;
  }
  *escape = (node == 0);

  if (*escape) {
    out_data[0] = 0;
    out_data[1] = 1;
  } else {
    huff_2D_8bit = -(node + 1);
    out_data[0] = huff_2D_8bit >> 4;
    out_data[1] = huff_2D_8bit & 0xf;
  }

bail:
  return err;
}

static ERROR_t sym_restore(HANDLE_FDK_BITSTREAM strm, int lav, SCHAR data[2]) {
  ULONG sym_bit = 0;

  int sum_val = data[0] + data[1];
  int diff_val = data[0] - data[1];

  if (sum_val > lav) {
    data[0] = -sum_val + (2 * lav + 1);
    data[1] = -diff_val;
  } else {
    data[0] = sum_val;
    data[1] = diff_val;
  }

  if (data[0] + data[1] != 0) {
    sym_bit = FDKreadBits(strm, 1);
    if (sym_bit) {
      data[0] = -data[0];
      data[1] = -data[1];
    }
  }

  if (data[0] - data[1] != 0) {
    sym_bit = FDKreadBits(strm, 1);
    if (sym_bit) {
      int tmp;
      tmp = data[0];
      data[0] = data[1];
      data[1] = tmp;
    }
  }

  return HUFFDEC_OK;
}

static ERROR_t huff_dec_1D(HANDLE_FDK_BITSTREAM strm, const DATA_TYPE data_type,
                           const INT dim1, SCHAR* out_data, const INT num_val,
                           const INT p0_flag)

{
  ERROR_t err = HUFFDEC_OK;
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  ULONG data = 0;
  int bitsAvail = 0;

  const SHORT(*partTab)[MAX_ENTRIES][2] = NULL;
  const SHORT(*nodeTab)[MAX_ENTRIES][2] = NULL;

  switch (data_type) {
    case t_CLD:
      partTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.cld[0][0];
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffCLDNodes.h1D[dim1]->nodeTab[0][0];
      break;
    case t_ICC:
      partTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.icc[0][0];
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffICCNodes.h1D[dim1]->nodeTab[0][0];
      break;
    case t_OLD:
      partTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.old[0][0];
      nodeTab = (HANDLE_HUFF_NODE)&huffOLDNodes.h1D[dim1]->nodeTab[0][0];
      break;
    case t_IPD:
      partTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.ipd[0][0];
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffIPDNodes.h1D[dim1].nodeTab[0][0];
      break;
    default:
      FDK_ASSERT(0);
      err = HUFFDEC_NOTOK;
      goto bail;
  }

  if (p0_flag) {
    if ((err = huff_read(strm, partTab, &node)) != HUFFDEC_OK) {
      goto bail;
    }

    out_data[0] = -(node + 1);
    offset = 1;
  }

  for (i = offset; i < num_val; i++) {
    bitsAvail = FDKgetValidBits(strm);
    if (bitsAvail < 1) {
      err = HUFFDEC_NOTOK;
      goto bail;
    }

    if ((err = huff_read(strm, nodeTab, &node)) != HUFFDEC_OK) {
      goto bail;
    }
    od = -(node + 1);

    if (data_type != t_IPD) {
      if (od != 0) {
        bitsAvail = FDKgetValidBits(strm);
        if (bitsAvail < 1) {
          err = HUFFDEC_NOTOK;
          goto bail;
        }

        data = FDKreadBits(strm, 1);
        od_sign = data;

        if (od_sign) od = -od;
      }
    }

    out_data[i] = od;
  }

bail:
  return err;
}

static ERROR_t huff_dec_2D(HANDLE_FDK_BITSTREAM strm, const DATA_TYPE data_type,
                           const INT dim1, const INT dim2, SCHAR out_data[][2],
                           const INT num_val, const INT stride,
                           SCHAR* p0_data[2]) {
  ERROR_t err = HUFFDEC_OK;
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;

  SCHAR esc_data[2][28] = {{0}};
  int escIdx[28] = {0};
  const SHORT(*nodeTab)[MAX_ENTRIES][2] = NULL;

  /* LAV */
  if ((err =
           huff_read(strm, (HANDLE_HUFF_NODE)&FDK_huffLavIdxNodes.nodeTab[0][0],
                     &node)) != HUFFDEC_OK) {
    goto bail;
  }
  data = -(node + 1);

  switch (data_type) {
    case t_CLD:
      lav = 2 * data + 3; /* 3, 5, 7, 9 */
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.cld[0][0];
      break;
    case t_ICC:
      lav = 2 * data + 1; /* 1, 3, 5, 7 */
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.icc[0][0];
      break;
    case t_OLD:
      lav = 3 * data + 3;
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.old[0][0];
      break;
    case t_IPD:
      if (data == 0)
        data = 3;
      else
        data--;
      lav = 2 * data + 1; /* 1, 3, 5, 7 */
      nodeTab = (HANDLE_HUFF_NODE)&FDK_huffPart0Nodes.ipd[0][0];
      break;
    default:
      FDK_ASSERT(0);
      err = HUFFDEC_NOTOK;
      goto bail;
  }

  /* Partition 0 */
  if (p0_data[0] != NULL) {
    if ((err = huff_read(strm, nodeTab, &node)) != HUFFDEC_OK) {
      goto bail;
    }
    *p0_data[0] = -(node + 1);
  }
  if (p0_data[1] != NULL) {
    if ((err = huff_read(strm, nodeTab, &node)) != HUFFDEC_OK) {
      goto bail;
    }
    *p0_data[1] = -(node + 1);
  }

  switch (data_type) {
    case t_CLD:
      switch (lav) {
        case 3:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffCLDNodes.h2D[dim1][dim2]->lav3[0][0];
          break;
        case 5:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffCLDNodes.h2D[dim1][dim2]->lav5[0][0];
          break;
        case 7:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffCLDNodes.h2D[dim1][dim2]->lav7[0][0];
          break;
        case 9:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffCLDNodes.h2D[dim1][dim2]->lav9[0][0];
          break;
      }
      break;
    case t_ICC:
      switch (lav) {
        case 1:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffICCNodes.h2D[dim1][dim2]->lav1[0][0];
          break;
        case 3:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffICCNodes.h2D[dim1][dim2]->lav3[0][0];
          break;
        case 5:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffICCNodes.h2D[dim1][dim2]->lav5[0][0];
          break;
        case 7:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffICCNodes.h2D[dim1][dim2]->lav7[0][0];
          break;
      }
      break;
    case t_OLD:
      switch (lav) {
        case 3:
          nodeTab = (HANDLE_HUFF_NODE)&huffOLDNodes.h2D[dim1][dim2]->lav3[0][0];
          break;
        case 6:
          nodeTab = (HANDLE_HUFF_NODE)&huffOLDNodes.h2D[dim1][dim2]->lav6[0][0];
          break;
        case 9:
          nodeTab = (HANDLE_HUFF_NODE)&huffOLDNodes.h2D[dim1][dim2]->lav9[0][0];
          break;
        case 12:
          nodeTab =
              (HANDLE_HUFF_NODE)&huffOLDNodes.h2D[dim1][dim2]->lav12[0][0];
          break;
      }
      break;
    case t_IPD:
      switch (lav) {
        case 1:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffIPDNodes.h2D[dim1][dim2].lav1[0][0];
          break;
        case 3:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffIPDNodes.h2D[dim1][dim2].lav3[0][0];
          break;
        case 5:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffIPDNodes.h2D[dim1][dim2].lav5[0][0];
          break;
        case 7:
          nodeTab =
              (HANDLE_HUFF_NODE)&FDK_huffIPDNodes.h2D[dim1][dim2].lav7[0][0];
          break;
      }
      break;
    default:
      break;
  }

  for (i = 0; i < num_val; i += stride) {
    if ((err = huff_read_2D(strm, nodeTab, out_data[i], &escape)) !=
        HUFFDEC_OK) {
      goto bail;
    }

    if (escape) {
      escIdx[escCntr++] = i;
    } else {
      if (data_type == t_IPD) {
        if ((err = sym_restoreIPD(strm, lav, out_data[i])) != HUFFDEC_OK) {
          goto bail;
        }
      } else {
        if ((err = sym_restore(strm, lav, out_data[i])) != HUFFDEC_OK) {
          goto bail;
        }
      }
    }
  } /* i */

  if (escCntr > 0) {
    if ((err = pcm_decode(strm, esc_data[0], esc_data[1], 0, 2 * escCntr,
                          (2 * lav + 1))) != HUFFDEC_OK) {
      goto bail;
    }

    for (i = 0; i < escCntr; i++) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
bail:
  return err;
}

static ERROR_t huff_decode(HANDLE_FDK_BITSTREAM strm, SCHAR* out_data_1,
                           SCHAR* out_data_2, DATA_TYPE data_type,
                           DIFF_TYPE diff_type_1, DIFF_TYPE diff_type_2,
                           int num_val, PAIRING* pairing_scheme, int ldMode) {
  ERROR_t err = HUFFDEC_OK;
  CODING_SCHEME coding_scheme = HUFF_1D;
  DIFF_TYPE diff_type;

  int i = 0;

  SCHAR pair_vec[28][2];

  SCHAR* p0_data_1[2] = {NULL, NULL};
  SCHAR* p0_data_2[2] = {NULL, NULL};

  int p0_flag[2];

  int num_val_1_int = num_val;
  int num_val_2_int = num_val;

  SCHAR* out_data_1_int = out_data_1;
  SCHAR* out_data_2_int = out_data_2;

  int df_rest_flag_1 = 0;
  int df_rest_flag_2 = 0;

  int hufYY1;
  int hufYY2;
  int hufYY;

  /* Coding scheme */
  coding_scheme = (CODING_SCHEME)FDKreadBits(strm, 1);

  if (coding_scheme == HUFF_2D) {
    if ((out_data_1 != NULL) && (out_data_2 != NULL) && (ldMode == 0)) {
      *pairing_scheme = (PAIRING)FDKreadBits(strm, 1);
    } else {
      *pairing_scheme = FREQ_PAIR;
    }
  }

  {
    hufYY1 = diff_type_1;
    hufYY2 = diff_type_2;
  }

  switch (coding_scheme) {
    case HUFF_1D:
      p0_flag[0] = (diff_type_1 == DIFF_FREQ);
      p0_flag[1] = (diff_type_2 == DIFF_FREQ);
      if (out_data_1 != NULL) {
        if ((err = huff_dec_1D(strm, data_type, hufYY1, out_data_1,
                               num_val_1_int, p0_flag[0])) != HUFFDEC_OK) {
          goto bail;
        }
      }
      if (out_data_2 != NULL) {
        if ((err = huff_dec_1D(strm, data_type, hufYY2, out_data_2,
                               num_val_2_int, p0_flag[1])) != HUFFDEC_OK) {
          goto bail;
        }
      }

      break; /* HUFF_1D */

    case HUFF_2D:

      switch (*pairing_scheme) {
        case FREQ_PAIR:

          if (out_data_1 != NULL) {
            if (diff_type_1 == DIFF_FREQ) {
              p0_data_1[0] = &out_data_1[0];
              p0_data_1[1] = NULL;

              num_val_1_int -= 1;
              out_data_1_int += 1;
            }
            df_rest_flag_1 = num_val_1_int % 2;
            if (df_rest_flag_1) num_val_1_int -= 1;
            if (num_val_1_int < 0) {
              err = HUFFDEC_NOTOK;
              goto bail;
            }
          }
          if (out_data_2 != NULL) {
            if (diff_type_2 == DIFF_FREQ) {
              p0_data_2[0] = NULL;
              p0_data_2[1] = &out_data_2[0];

              num_val_2_int -= 1;
              out_data_2_int += 1;
            }
            df_rest_flag_2 = num_val_2_int % 2;
            if (df_rest_flag_2) num_val_2_int -= 1;
            if (num_val_2_int < 0) {
              err = HUFFDEC_NOTOK;
              goto bail;
            }
          }

          if (out_data_1 != NULL) {
            if ((err = huff_dec_2D(strm, data_type, hufYY1, FREQ_PAIR, pair_vec,
                                   num_val_1_int, 2, p0_data_1)) !=
                HUFFDEC_OK) {
              goto bail;
            }
            if (df_rest_flag_1) {
              if ((err = huff_dec_1D(strm, data_type, hufYY1,
                                     out_data_1_int + num_val_1_int, 1, 0)) !=
                  HUFFDEC_OK) {
                goto bail;
              }
            }
          }
          if (out_data_2 != NULL) {
            if ((err = huff_dec_2D(strm, data_type, hufYY2, FREQ_PAIR,
                                   pair_vec + 1, num_val_2_int, 2,
                                   p0_data_2)) != HUFFDEC_OK) {
              goto bail;
            }
            if (df_rest_flag_2) {
              if ((err = huff_dec_1D(strm, data_type, hufYY2,
                                     out_data_2_int + num_val_2_int, 1, 0)) !=
                  HUFFDEC_OK) {
                goto bail;
              }
            }
          }

          if (out_data_1 != NULL) {
            for (i = 0; i < num_val_1_int - 1; i += 2) {
              out_data_1_int[i] = pair_vec[i][0];
              out_data_1_int[i + 1] = pair_vec[i][1];
            }
          }
          if (out_data_2 != NULL) {
            for (i = 0; i < num_val_2_int - 1; i += 2) {
              out_data_2_int[i] = pair_vec[i + 1][0];
              out_data_2_int[i + 1] = pair_vec[i + 1][1];
            }
          }
          break; /* FREQ_PAIR */

        case TIME_PAIR:
          if (((diff_type_1 == DIFF_FREQ) || (diff_type_2 == DIFF_FREQ))) {
            p0_data_1[0] = &out_data_1[0];
            p0_data_1[1] = &out_data_2[0];

            out_data_1_int += 1;
            out_data_2_int += 1;

            num_val_1_int -= 1;
          }

          if ((diff_type_1 == DIFF_TIME) || (diff_type_2 == DIFF_TIME)) {
            diff_type = DIFF_TIME;
          } else {
            diff_type = DIFF_FREQ;
          }
          { hufYY = diff_type; }

          if ((err = huff_dec_2D(strm, data_type, hufYY, TIME_PAIR, pair_vec,
                                 num_val_1_int, 1, p0_data_1)) != HUFFDEC_OK) {
            goto bail;
          }

          for (i = 0; i < num_val_1_int; i++) {
            out_data_1_int[i] = pair_vec[i][0];
            out_data_2_int[i] = pair_vec[i][1];
          }

          break; /* TIME_PAIR */

        default:
          break;
      }

      break; /* HUFF_2D */

    default:
      break;
  }
bail:
  return err;
}

static void diff_freq_decode(const SCHAR* const diff_data,
                             SCHAR* const out_data, const int num_val) {
  int i = 0;
  out_data[0] = diff_data[0];

  for (i = 1; i < num_val; i++) {
    out_data[i] = out_data[i - 1] + diff_data[i];
  }
}

static void diff_time_decode_backwards(const SCHAR* const prev_data,
                                       const SCHAR* const diff_data,
                                       SCHAR* const out_data,
                                       const int mixed_diff_type,
                                       const int num_val) {
  int i = 0; /* default start value*/

  if (mixed_diff_type) {
    out_data[0] = diff_data[0];
    i = 1; /* new start value */
  }
  for (; i < num_val; i++) {
    out_data[i] = prev_data[i] + diff_data[i];
  }
}

static void diff_time_decode_forwards(const SCHAR* const prev_data,
                                      const SCHAR* const diff_data,
                                      SCHAR* const out_data,
                                      const int mixed_diff_type,
                                      const int num_val) {
  int i = 0; /* default start value*/

  if (mixed_diff_type) {
    out_data[0] = diff_data[0];
    i = 1; /* new start value */
  }
  for (; i < num_val; i++) {
    out_data[i] = prev_data[i] - diff_data[i];
  }
}

static ERROR_t attach_lsb(HANDLE_FDK_BITSTREAM strm, SCHAR* in_data_msb,
                          int offset, int num_lsb, int num_val,
                          SCHAR* out_data) {
  int i = 0, lsb = 0;
  ULONG data = 0;

  for (i = 0; i < num_val; i++) {
    int msb;
    msb = in_data_msb[i];

    if (num_lsb > 0) {
      data = FDKreadBits(strm, num_lsb);
      lsb = data;

      out_data[i] = ((msb << num_lsb) | lsb) - offset;
    } else
      out_data[i] = msb - offset;
  }

  return HUFFDEC_OK; /* dummy */
}

ERROR_t EcDataPairDec(DECODER_TYPE DECODER, HANDLE_FDK_BITSTREAM strm,
                      SCHAR* aaOutData1, SCHAR* aaOutData2, SCHAR* aHistory,
                      DATA_TYPE data_type, int startBand, int dataBands,
                      int pair_flag, int coarse_flag,
                      int allowDiffTimeBack_flag)

{
  ERROR_t err = HUFFDEC_OK;

  // int allowDiffTimeBack_flag = !independency_flag || (setIdx > 0);
  int attachLsb_flag = 0;
  int pcmCoding_flag = 0;

  int mixed_time_pair = 0, numValPcm = 0;
  int quant_levels = 0, quant_offset = 0;
  ULONG data = 0;

  SCHAR aaDataPair[2][28] = {{0}};
  SCHAR aaDataDiff[2][28] = {{0}};

  SCHAR aHistoryMsb[28] = {0};

  SCHAR* pDataVec[2] = {NULL, NULL};

  DIFF_TYPE diff_type[2] = {DIFF_FREQ, DIFF_FREQ};
  PAIRING pairing = FREQ_PAIR;
  DIRECTION direction = BACKWARDS;

  switch (data_type) {
    case t_CLD:
      if (coarse_flag) {
        attachLsb_flag = 0;
        quant_levels = 15;
        quant_offset = 7;
      } else {
        attachLsb_flag = 0;
        quant_levels = 31;
        quant_offset = 15;
      }

      break;

    case t_ICC:
      if (coarse_flag) {
        attachLsb_flag = 0;
        quant_levels = 4;
        quant_offset = 0;
      } else {
        attachLsb_flag = 0;
        quant_levels = 8;
        quant_offset = 0;
      }

      break;

    case t_OLD:
      if (coarse_flag) {
        attachLsb_flag = 0;
        quant_levels = 8;
        quant_offset = 0;
      } else {
        attachLsb_flag = 0;
        quant_levels = 16;
        quant_offset = 0;
      }
      break;

    case t_NRG:
      if (coarse_flag) {
        attachLsb_flag = 0;
        quant_levels = 32;
        quant_offset = 0;
      } else {
        attachLsb_flag = 0;
        quant_levels = 64;
        quant_offset = 0;
      }
      break;

    case t_IPD:
      if (!coarse_flag) {
        attachLsb_flag = 1;
        quant_levels = 16;
        quant_offset = 0;
      } else {
        attachLsb_flag = 0;
        quant_levels = 8;
        quant_offset = 0;
      }
      break;

    default:
      return HUFFDEC_NOTOK;
  }

  data = FDKreadBits(strm, 1);
  pcmCoding_flag = data;

  if (pcmCoding_flag) {
    if (pair_flag) {
      pDataVec[0] = aaDataPair[0];
      pDataVec[1] = aaDataPair[1];
      numValPcm = 2 * dataBands;
    } else {
      pDataVec[0] = aaDataPair[0];
      pDataVec[1] = NULL;
      numValPcm = dataBands;
    }

    err = pcm_decode(strm, pDataVec[0], pDataVec[1], quant_offset, numValPcm,
                     quant_levels);
    if (err != HUFFDEC_OK) return HUFFDEC_NOTOK;

  } else { /* Differential/Huffman/LSB Coding */

    if (pair_flag) {
      pDataVec[0] = aaDataDiff[0];
      pDataVec[1] = aaDataDiff[1];
    } else {
      pDataVec[0] = aaDataDiff[0];
      pDataVec[1] = NULL;
    }

    diff_type[0] = DIFF_FREQ;
    diff_type[1] = DIFF_FREQ;

    direction = BACKWARDS;
    {
      if (pair_flag || allowDiffTimeBack_flag) {
        data = FDKreadBits(strm, 1);
        diff_type[0] = (DIFF_TYPE)data;
      }

      if (pair_flag &&
          ((diff_type[0] == DIFF_FREQ) || allowDiffTimeBack_flag)) {
        data = FDKreadBits(strm, 1);
        diff_type[1] = (DIFF_TYPE)data;
      }
    }
    /* Huffman decoding */
    err = huff_decode(strm, pDataVec[0], pDataVec[1], data_type, diff_type[0],
                      diff_type[1], dataBands, &pairing,
                      (DECODER == SAOC_DECODER));
    if (err != HUFFDEC_OK) {
      return HUFFDEC_NOTOK;
    }

    {
      /* Differential decoding */
      if ((diff_type[0] == DIFF_TIME) || (diff_type[1] == DIFF_TIME)) {
        if (DECODER == SAOC_DECODER) {
          direction = BACKWARDS;
        } else {
          if (pair_flag) {
            if ((diff_type[0] == DIFF_TIME) && !allowDiffTimeBack_flag) {
              direction = FORWARDS;
            } else if (diff_type[1] == DIFF_TIME) {
              direction = BACKWARDS;
            } else {
              data = FDKreadBits(strm, 1);
              direction = (DIRECTION)data;
            }
          } else {
            direction = BACKWARDS;
          }
        }
      }

      mixed_time_pair =
          (diff_type[0] != diff_type[1]) && (pairing == TIME_PAIR);

      if (direction == BACKWARDS) {
        if (diff_type[0] == DIFF_FREQ) {
          diff_freq_decode(aaDataDiff[0], aaDataPair[0], dataBands);
        } else {
          int i;
          for (i = 0; i < dataBands; i++) {
            aHistoryMsb[i] = aHistory[i + startBand] + quant_offset;
            if (attachLsb_flag) {
              aHistoryMsb[i] >>= 1;
            }
          }
          diff_time_decode_backwards(aHistoryMsb, aaDataDiff[0], aaDataPair[0],
                                     mixed_time_pair, dataBands);
        }
        if (diff_type[1] == DIFF_FREQ) {
          diff_freq_decode(aaDataDiff[1], aaDataPair[1], dataBands);
        } else {
          diff_time_decode_backwards(aaDataPair[0], aaDataDiff[1],
                                     aaDataPair[1], mixed_time_pair, dataBands);
        }
      } else {
        /* diff_type[1] MUST BE DIFF_FREQ */
        diff_freq_decode(aaDataDiff[1], aaDataPair[1], dataBands);

        if (diff_type[0] == DIFF_FREQ) {
          diff_freq_decode(aaDataDiff[0], aaDataPair[0], dataBands);
        } else {
          diff_time_decode_forwards(aaDataPair[1], aaDataDiff[0], aaDataPair[0],
                                    mixed_time_pair, dataBands);
        }
      }
    }

    /* LSB decoding */
    err = attach_lsb(strm, aaDataPair[0], quant_offset, attachLsb_flag ? 1 : 0,
                     dataBands, aaDataPair[0]);
    if (err != HUFFDEC_OK) goto bail;

    if (pair_flag) {
      err = attach_lsb(strm, aaDataPair[1], quant_offset,
                       attachLsb_flag ? 1 : 0, dataBands, aaDataPair[1]);
      if (err != HUFFDEC_OK) goto bail;
    }
  } /* End: Differential/Huffman/LSB Coding */

  /* Copy data to output arrays */
  FDKmemcpy(aaOutData1 + startBand, aaDataPair[0], sizeof(SCHAR) * dataBands);
  if (pair_flag) {
    FDKmemcpy(aaOutData2 + startBand, aaDataPair[1], sizeof(SCHAR) * dataBands);
  }

bail:
  return err;
}

ERROR_t huff_dec_reshape(HANDLE_FDK_BITSTREAM strm, int* out_data,
                         int num_val) {
  ERROR_t err = HUFFDEC_OK;
  int val_rcvd = 0, dummy = 0, i = 0, val = 0, len = 0;
  SCHAR rl_data[2] = {0};

  while (val_rcvd < num_val) {
    err = huff_read_2D(strm,
                       (HANDLE_HUFF_NODE)&FDK_huffReshapeNodes.nodeTab[0][0],
                       rl_data, &dummy);
    if (err != HUFFDEC_OK) goto bail;
    val = rl_data[0];
    len = rl_data[1] + 1;
    if (val_rcvd + len > num_val) {
      err = HUFFDEC_NOTOK;
      goto bail;
    }
    for (i = val_rcvd; i < val_rcvd + len; i++) {
      out_data[i] = val;
    }
    val_rcvd += len;
  }
bail:
  return err;
}
