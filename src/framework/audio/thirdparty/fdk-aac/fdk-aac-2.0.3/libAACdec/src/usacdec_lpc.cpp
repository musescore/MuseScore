/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2019 Fraunhofer-Gesellschaft zur Förderung der angewandten
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

/**************************** AAC decoder library ******************************

   Author(s):   Matthias Hildenbrand, Manuel Jander

   Description: USAC LPC/AVQ decode

*******************************************************************************/

#include "usacdec_lpc.h"

#include "usacdec_rom.h"
#include "FDK_trigFcts.h"

#define NQ_MAX 36

/*
 * Helper functions.
 */

/**
 * \brief Read unary code.
 * \param hBs bitstream handle as data source.
 * \return decoded value.
 */
static int get_vlclbf(HANDLE_FDK_BITSTREAM hBs) {
  int result = 0;

  while (FDKreadBits(hBs, 1) && result <= NQ_MAX) {
    result++;
  }
  return result;
}

/**
 * \brief Read bit count limited unary code.
 * \param hBs bitstream handle as data source
 * \param n max amount of bits to be read.
 * \return decoded value.
 */
static int get_vlclbf_n(HANDLE_FDK_BITSTREAM hBs, int n) {
  int result = 0;

  while (FDKreadBits(hBs, 1)) {
    result++;
    n--;
    if (n <= 0) {
      break;
    }
  }

  return result;
}

/*
 * Algebraic Vector Quantizer
 */

/* ZF_SCALE must be greater than (number of FIXP_ZF)/2
   because the loss of precision caused by fPow2Div2 in RE8_PPV() */
//#define ZF_SCALE ((NQ_MAX-3)>>1)
#define ZF_SCALE ((DFRACT_BITS / 2))
#define FIXP_ZF FIXP_DBL
#define INT2ZF(x, s) (FIXP_ZF)((x) << (ZF_SCALE - (s)))
#define ZF2INT(x) (INT)((x) >> ZF_SCALE)

/* 1.0 in ZF format format */
#define ONEZF ((FIXP_ZF)INT2ZF(1, 0))

/* static */
void nearest_neighbor_2D8(FIXP_ZF x[8], int y[8]) {
  FIXP_ZF s, em, e[8];
  int i, j, sum;

  /* round x into 2Z^8 i.e. compute y=(y1,...,y8) such that yi = 2[xi/2]
     where [.] is the nearest integer operator
     in the mean time, compute sum = y1+...+y8
  */
  sum = 0;
  for (i = 0; i < 8; i++) {
    FIXP_ZF tmp;
    /* round to ..., -2, 0, 2, ... ([-1..1[ --> 0) */
    if (x[i] < (FIXP_ZF)0) {
      tmp = ONEZF - x[i];
      y[i] = -2 * ((ZF2INT(tmp)) >> 1);
    } else {
      tmp = ONEZF + x[i];
      y[i] = 2 * ((ZF2INT(tmp)) >> 1);
    }
    sum += y[i];
  }
  /* check if y1+...+y8 is a multiple of 4
     if not, y is not round xj in the wrong way where j is defined by
        j = arg max_i | xi -yi|
     (this is called the Wagner rule)
  */
  if (sum % 4) {
    /* find j = arg max_i | xi -yi| */
    em = (FIXP_SGL)0;
    j = 0;
    for (i = 0; i < 8; i++) {
      /* compute ei = xi-yi */
      e[i] = x[i] - INT2ZF(y[i], 0);
    }
    for (i = 0; i < 8; i++) {
      /* compute |ei| = | xi-yi | */
      if (e[i] < (FIXP_ZF)0) {
        s = -e[i];
      } else {
        s = e[i];
      }
      /* check if |ei| is maximal, if so, set j=i */
      if (em < s) {
        em = s;
        j = i;
      }
    }
    /* round xj in the "wrong way" */
    if (e[j] < (FIXP_ZF)0) {
      y[j] -= 2;
    } else {
      y[j] += 2;
    }
  }
}

/*--------------------------------------------------------------
  RE8_PPV(x,y)
  NEAREST NEIGHBOR SEARCH IN INFINITE LATTICE RE8
  the algorithm is based on the definition of RE8 as
      RE8 = (2D8) U (2D8+[1,1,1,1,1,1,1,1])
  it applies the coset decoding of Sloane and Conway
  (i) x: point in R^8 in 32-ZF_SCALE.ZF_SCALE format
  (o) y: point in RE8 (8-dimensional integer vector)
  --------------------------------------------------------------
*/
/* static */
void RE8_PPV(FIXP_ZF x[], SHORT y[], int r) {
  int i, y0[8], y1[8];
  FIXP_ZF x1[8], tmp;
  INT64 e;

  /* find the nearest neighbor y0 of x in 2D8 */
  nearest_neighbor_2D8(x, y0);
  /* find the nearest neighbor y1 of x in 2D8+(1,...,1) (by coset decoding) */
  for (i = 0; i < 8; i++) {
    x1[i] = x[i] - ONEZF;
  }
  nearest_neighbor_2D8(x1, y1);
  for (i = 0; i < 8; i++) {
    y1[i] += 1;
  }

  /* compute e0=||x-y0||^2 and e1=||x-y1||^2 */
  e = 0;
  for (i = 0; i < 8; i++) {
    tmp = x[i] - INT2ZF(y0[i], 0);
    e += (INT64)fPow2Div2(
        tmp << r); /* shift left to ensure that no fract part bits get lost. */
    tmp = x[i] - INT2ZF(y1[i], 0);
    e -= (INT64)fPow2Div2(tmp << r);
  }
  /* select best candidate y0 or y1 to minimize distortion */
  if (e < 0) {
    for (i = 0; i < 8; i++) {
      y[i] = y0[i];
    }
  } else {
    for (i = 0; i < 8; i++) {
      y[i] = y1[i];
    }
  }
}

/* table look-up of unsigned value: find i where index >= table[i]
   Note: range must be >= 2, index must be >= table[0] */
static int table_lookup(const USHORT *table, unsigned int index, int range) {
  int i;

  for (i = 4; i < range; i += 4) {
    if (index < table[i]) {
      break;
    }
  }
  if (i > range) {
    i = range;
  }

  if (index < table[i - 2]) {
    i -= 2;
  }
  if (index < table[i - 1]) {
    i--;
  }
  i--;

  return (i); /* index >= table[i] */
}

/*--------------------------------------------------------------------------
  re8_decode_rank_of_permutation(rank, xs, x)
  DECODING OF THE RANK OF THE PERMUTATION OF xs
  (i) rank: index (rank) of a permutation
  (i) xs:   signed leader in RE8 (8-dimensional integer vector)
  (o) x:    point in RE8 (8-dimensional integer vector)
  --------------------------------------------------------------------------
 */
static void re8_decode_rank_of_permutation(int rank, int *xs, SHORT x[8]) {
  INT a[8], w[8], B, fac, fac_B, target;
  int i, j;

  /* --- pre-processing based on the signed leader xs ---
     - compute the alphabet a=[a[0] ... a[q-1]] of x (q elements)
       such that a[0]!=...!=a[q-1]
       it is assumed that xs is sorted in the form of a signed leader
       which can be summarized in 2 requirements:
          a) |xs[0]| >= |xs[1]| >= |xs[2]| >= ... >= |xs[7]|
          b) if |xs[i]|=|xs[i-1]|, xs[i]>=xs[i+1]
       where |.| indicates the absolute value operator
     - compute q (the number of symbols in the alphabet)
     - compute w[0..q-1] where w[j] counts the number of occurences of
       the symbol a[j] in xs
     - compute B = prod_j=0..q-1 (w[j]!) where .! is the factorial */
  /* xs[i], xs[i-1] and ptr_w/a*/
  j = 0;
  w[j] = 1;
  a[j] = xs[0];
  B = 1;
  for (i = 1; i < 8; i++) {
    if (xs[i] != xs[i - 1]) {
      j++;
      w[j] = 1;
      a[j] = xs[i];
    } else {
      w[j]++;
      B *= w[j];
    }
  }

  /* --- actual rank decoding ---
     the rank of x (where x is a permutation of xs) is based on
     Schalkwijk's formula
     it is given by rank=sum_{k=0..7} (A_k * fac_k/B_k)
     the decoding of this rank is sequential and reconstructs x[0..7]
     element by element from x[0] to x[7]
     [the tricky part is the inference of A_k for each k...]
   */

  if (w[0] == 8) {
    for (i = 0; i < 8; i++) {
      x[i] = a[0]; /* avoid fac of 40320 */
    }
  } else {
    target = rank * B;
    fac_B = 1;
    /* decode x element by element */
    for (i = 0; i < 8; i++) {
      fac = fac_B * fdk_dec_tab_factorial[i]; /* fac = 1..5040 */
      j = -1;
      do {
        target -= w[++j] * fac;
      } while (target >= 0); /* max of 30 tests / SV */
      x[i] = a[j];
      /* update rank, denominator B (B_k) and counter w[j] */
      target += w[j] * fac; /* target = fac_B*B*rank */
      fac_B *= w[j];
      w[j]--;
    }
  }
}

/*--------------------------------------------------------------------------
  re8_decode_base_index(n, I, y)
  DECODING OF AN INDEX IN Qn (n=0,2,3 or 4)
  (i) n: codebook number (*n is an integer defined in {0,2,3,4})
  (i) I: index of c (pointer to unsigned 16-bit word)
  (o) y: point in RE8 (8-dimensional integer vector)
  note: the index I is defined as a 32-bit word, but only
  16 bits are required (long can be replaced by unsigned integer)
  --------------------------------------------------------------------------
 */
static void re8_decode_base_index(int *n, UINT index, SHORT y[8]) {
  int i, im, t, sign_code, ka, ks, rank, leader[8];

  if (*n < 2) {
    for (i = 0; i < 8; i++) {
      y[i] = 0;
    }
  } else {
    // index = (unsigned int)*I;
    /* search for the identifier ka of the absolute leader (table-lookup)
       Q2 is a subset of Q3 - the two cases are considered in the same branch
     */
    switch (*n) {
      case 2:
      case 3:
        i = table_lookup(fdk_dec_I3, index, NB_LDQ3);
        ka = fdk_dec_A3[i];
        break;
      case 4:
        i = table_lookup(fdk_dec_I4, index, NB_LDQ4);
        ka = fdk_dec_A4[i];
        break;
      default:
        FDK_ASSERT(0);
        return;
    }
    /* reconstruct the absolute leader */
    for (i = 0; i < 8; i++) {
      leader[i] = fdk_dec_Da[ka][i];
    }
    /* search for the identifier ks of the signed leader (table look-up)
       (this search is focused based on the identifier ka of the absolute
        leader)*/
    t = fdk_dec_Ia[ka];
    im = fdk_dec_Ns[ka];
    ks = table_lookup(fdk_dec_Is + t, index, im);

    /* reconstruct the signed leader from its sign code */
    sign_code = 2 * fdk_dec_Ds[t + ks];
    for (i = 7; i >= 0; i--) {
      leader[i] *= (1 - (sign_code & 2));
      sign_code >>= 1;
    }

    /* compute and decode the rank of the permutation */
    rank = index - fdk_dec_Is[t + ks]; /* rank = index - cardinality offset */

    re8_decode_rank_of_permutation(rank, leader, y);
  }
  return;
}

/* re8_y2k(y,m,k)
   VORONOI INDEXING (INDEX DECODING) k -> y
   (i) k: Voronoi index k[0..7]
   (i) m: Voronoi modulo (m = 2^r = 1<<r, where r is integer >=2)
   (i) r: Voronoi order  (m = 2^r = 1<<r, where r is integer >=2)
   (o) y: 8-dimensional point y[0..7] in RE8
 */
static void re8_k2y(int *k, int r, SHORT *y) {
  int i, tmp, sum;
  SHORT v[8];
  FIXP_ZF zf[8];

  FDK_ASSERT(r <= ZF_SCALE);

  /* compute y = k M and z=(y-a)/m, where
     M = [4        ]
         [2 2      ]
         [|   \    ]
         [2     2  ]
         [1 1 _ 1 1]
     a=(2,0,...,0)
     m = 1<<r
  */
  for (i = 0; i < 8; i++) {
    y[i] = k[7];
  }
  zf[7] = INT2ZF(y[7], r);
  sum = 0;
  for (i = 6; i >= 1; i--) {
    tmp = 2 * k[i];
    sum += tmp;
    y[i] += tmp;
    zf[i] = INT2ZF(y[i], r);
  }
  y[0] += (4 * k[0] + sum);
  zf[0] = INT2ZF(y[0] - 2, r);
  /* find nearest neighbor v of z in infinite RE8 */
  RE8_PPV(zf, v, r);
  /* compute y -= m v */
  for (i = 0; i < 8; i++) {
    y[i] -= (SHORT)(v[i] << r);
  }
}

/*--------------------------------------------------------------------------
  RE8_dec(n, I, k, y)
  MULTI-RATE INDEXING OF A POINT y in THE LATTICE RE8 (INDEX DECODING)
  (i) n: codebook number (*n is an integer defined in {0,2,3,4,..,n_max}). n_max
  = 36 (i) I: index of c (pointer to unsigned 16-bit word) (i) k: index of v
  (8-dimensional vector of binary indices) = Voronoi index (o) y: point in RE8
  (8-dimensional integer vector) note: the index I is defined as a 32-bit word,
  but only 16 bits are required (long can be replaced by unsigned integer)

  return 0 on success, -1 on error.
  --------------------------------------------------------------------------
 */
static int RE8_dec(int n, int I, int *k, FIXP_DBL *y) {
  SHORT v[8];
  SHORT _y[8];
  UINT r;
  int i;

  /* Check bound of codebook qn */
  if (n > NQ_MAX) {
    return -1;
  }

  /* decode the sub-indices I and kv[] according to the codebook number n:
     if n=0,2,3,4, decode I (no Voronoi extension)
     if n>4, Voronoi extension is used, decode I and kv[] */
  if (n <= 4) {
    re8_decode_base_index(&n, I, _y);
    for (i = 0; i < 8; i++) {
      y[i] = (LONG)_y[i];
    }
  } else {
    /* compute the Voronoi modulo m = 2^r where r is extension order */
    r = ((n - 3) >> 1);

    while (n > 4) {
      n -= 2;
    }
    /* decode base codebook index I into c (c is an element of Q3 or Q4)
       [here c is stored in y to save memory] */
    re8_decode_base_index(&n, I, _y);
    /* decode Voronoi index k[] into v */
    re8_k2y(k, r, v);
    /* reconstruct y as y = m c + v (with m=2^r, r integer >=1) */
    for (i = 0; i < 8; i++) {
      y[i] = (LONG)((_y[i] << r) + v[i]);
    }
  }
  return 0;
}

/**************************/
/* start LPC decode stuff */
/**************************/
//#define M         16
#define FREQ_MAX 6400.0f
#define FREQ_DIV 400.0f
#define LSF_GAP 50.0f

/**
 * \brief calculate inverse weighting factor and add non-weighted residual
 *        LSF vector to first stage LSF approximation
 * \param lsfq first stage LSF approximation values.
 * \param xq weighted residual LSF vector
 * \param nk_mode code book number coding mode.
 */
static void lsf_weight_2st(FIXP_LPC *lsfq, FIXP_DBL *xq, int nk_mode) {
  FIXP_LPC d[M_LP_FILTER_ORDER + 1];
  FIXP_SGL factor;
  LONG w; /* inverse weight factor */
  int i;

  /* compute lsf distance */
  d[0] = lsfq[0];
  d[M_LP_FILTER_ORDER] =
      FL2FXCONST_LPC(FREQ_MAX / (1 << LSF_SCALE)) - lsfq[M_LP_FILTER_ORDER - 1];
  for (i = 1; i < M_LP_FILTER_ORDER; i++) {
    d[i] = lsfq[i] - lsfq[i - 1];
  }

  switch (nk_mode) {
    case 0:
      factor = FL2FXCONST_SGL(2.0f * 60.0f / FREQ_DIV);
      break; /* abs */
    case 1:
      factor = FL2FXCONST_SGL(2.0f * 65.0f / FREQ_DIV);
      break; /* mid */
    case 2:
      factor = FL2FXCONST_SGL(2.0f * 64.0f / FREQ_DIV);
      break; /* rel1 */
    default:
      factor = FL2FXCONST_SGL(2.0f * 63.0f / FREQ_DIV);
      break; /* rel2 */
  }
  /* add non-weighted residual LSF vector to LSF1st */
  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    w = (LONG)fMultDiv2(factor, sqrtFixp(fMult(d[i], d[i + 1])));
    lsfq[i] = fAddSaturate(lsfq[i],
                           FX_DBL2FX_LPC((FIXP_DBL)((INT64)w * (LONG)xq[i])));
  }

  return;
}

/**
 * \brief decode nqn amount of code book numbers. These values determine the
 * amount of following bits for nqn AVQ RE8 vectors.
 * \param nk_mode quantization mode.
 * \param nqn amount code book number to read.
 * \param qn pointer to output buffer to hold decoded code book numbers qn.
 */
static void decode_qn(HANDLE_FDK_BITSTREAM hBs, int nk_mode, int nqn,
                      int qn[]) {
  int n;

  if (nk_mode == 1) { /* nk mode 1 */
    /* Unary code for mid LPC1/LPC3 */
    /* Q0=0, Q2=10, Q3=110, ... */
    for (n = 0; n < nqn; n++) {
      qn[n] = get_vlclbf(hBs);
      if (qn[n] > 0) {
        qn[n]++;
      }
    }
  } else { /* nk_mode 0, 3 and 2 */
    /* 2 bits to specify Q2,Q3,Q4,ext */
    for (n = 0; n < nqn; n++) {
      qn[n] = 2 + FDKreadBits(hBs, 2);
    }
    if (nk_mode == 2) {
      /* Unary code for rel LPC1/LPC3 */
      /* Q0 = 0, Q5=10, Q6=110, ... */
      for (n = 0; n < nqn; n++) {
        if (qn[n] > 4) {
          qn[n] = get_vlclbf(hBs);
          if (qn[n] > 0) qn[n] += 4;
        }
      }
    } else { /* nk_mode == (0 and 3) */
      /* Unary code for abs and rel LPC0/LPC2 */
      /* Q5 = 0, Q6=10, Q0=110, Q7=1110, ... */
      for (n = 0; n < nqn; n++) {
        if (qn[n] > 4) {
          qn[n] = get_vlclbf(hBs);
          switch (qn[n]) {
            case 0:
              qn[n] = 5;
              break;
            case 1:
              qn[n] = 6;
              break;
            case 2:
              qn[n] = 0;
              break;
            default:
              qn[n] += 4;
              break;
          }
        }
      }
    }
  }
}

/**
 * \brief reorder LSF coefficients to minimum distance.
 * \param lsf pointer to buffer containing LSF coefficients and where reordered
 * LSF coefficients will be stored into, scaled by LSF_SCALE.
 * \param min_dist min distance scaled by LSF_SCALE
 * \param n number of LSF/LSP coefficients.
 */
static void reorder_lsf(FIXP_LPC *lsf, FIXP_LPC min_dist, int n) {
  FIXP_LPC lsf_min;
  int i;

  lsf_min = min_dist;
  for (i = 0; i < n; i++) {
    if (lsf[i] < lsf_min) {
      lsf[i] = lsf_min;
    }
    lsf_min = fAddSaturate(lsf[i], min_dist);
  }

  /* reverse */
  lsf_min = FL2FXCONST_LPC(FREQ_MAX / (1 << LSF_SCALE)) - min_dist;
  for (i = n - 1; i >= 0; i--) {
    if (lsf[i] > lsf_min) {
      lsf[i] = lsf_min;
    }

    lsf_min = lsf[i] - min_dist;
  }
}

/**
 * \brief First stage approximation
 * \param hBs bitstream handle as data source
 * \param lsfq pointer to output buffer to hold LPC coefficients scaled by
 * LSF_SCALE.
 */
static void vlpc_1st_dec(
    HANDLE_FDK_BITSTREAM hBs, /* input:  codebook index                  */
    FIXP_LPC *lsfq            /* i/o:    i:prediction   o:quantized lsf  */
) {
  const FIXP_LPC *p_dico;
  int i, index;

  index = FDKreadBits(hBs, 8);
  p_dico = &fdk_dec_dico_lsf_abs_8b[index * M_LP_FILTER_ORDER];
  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    lsfq[i] = p_dico[i];
  }
}

/**
 * \brief Do first stage approximation weighting and multiply with AVQ
 * refinement.
 * \param hBs bitstream handle data ssource.
 * \param lsfq buffer holding 1st stage approx, 2nd stage approx is added to
 * this values.
 * \param nk_mode quantization mode.
 * \return 0 on success, -1 on error.
 */
static int vlpc_2st_dec(
    HANDLE_FDK_BITSTREAM hBs,
    FIXP_LPC *lsfq, /* i/o:    i:1st stage   o:1st+2nd stage   */
    int nk_mode     /* input:  0=abs, >0=rel                   */
) {
  int err;
  FIXP_DBL xq[M_LP_FILTER_ORDER]; /* weighted residual LSF vector */

  /* Decode AVQ refinement */
  { err = CLpc_DecodeAVQ(hBs, xq, nk_mode, 2, 8); }
  if (err != 0) {
    return -1;
  }

  /* add non-weighted residual LSF vector to LSF1st */
  lsf_weight_2st(lsfq, xq, nk_mode);

  /* reorder */
  reorder_lsf(lsfq, FL2FXCONST_LPC(LSF_GAP / (1 << LSF_SCALE)),
              M_LP_FILTER_ORDER);

  return 0;
}

/*
 * Externally visible functions
 */

int CLpc_DecodeAVQ(HANDLE_FDK_BITSTREAM hBs, FIXP_DBL *pOutput, int nk_mode,
                   int no_qn, int length) {
  int i, l;

  for (i = 0; i < length; i += 8 * no_qn) {
    int qn[2], nk, n, I;
    int kv[8] = {0};

    decode_qn(hBs, nk_mode, no_qn, qn);

    for (l = 0; l < no_qn; l++) {
      if (qn[l] == 0) {
        FDKmemclear(&pOutput[i + l * 8], 8 * sizeof(FIXP_DBL));
      }

      /* Voronoi extension order ( nk ) */
      nk = 0;
      n = qn[l];
      if (qn[l] > 4) {
        nk = (qn[l] - 3) >> 1;
        n = qn[l] - nk * 2;
      }

      /* Base codebook index, in reverse bit group order (!) */
      I = FDKreadBits(hBs, 4 * n);

      if (nk > 0) {
        int j;

        for (j = 0; j < 8; j++) {
          kv[j] = FDKreadBits(hBs, nk);
        }
      }

      if (RE8_dec(qn[l], I, kv, &pOutput[i + l * 8]) != 0) {
        return -1;
      }
    }
  }
  return 0;
}

int CLpc_Read(HANDLE_FDK_BITSTREAM hBs, FIXP_LPC lsp[][M_LP_FILTER_ORDER],
              FIXP_LPC lpc4_lsf[M_LP_FILTER_ORDER],
              FIXP_LPC lsf_adaptive_mean_cand[M_LP_FILTER_ORDER],
              FIXP_SGL pStability[], UCHAR *mod, int first_lpd_flag,
              int last_lpc_lost, int last_frame_ok) {
  int i, k, err;
  int mode_lpc_bin = 0; /* mode_lpc bitstream representation */
  int lpc_present[5] = {0, 0, 0, 0, 0};
  int lpc0_available = 1;
  int s = 0;
  int l = 3;
  const int nbDiv = NB_DIV;

  lpc_present[4 >> s] = 1; /* LPC4 */

  /* Decode LPC filters in the following order: LPC 4,0,2,1,3 */

  /*** Decode LPC4 ***/
  vlpc_1st_dec(hBs, lsp[4 >> s]);
  err = vlpc_2st_dec(hBs, lsp[4 >> s], 0); /* nk_mode = 0 */
  if (err != 0) {
    return err;
  }

  /*** Decode LPC0 and LPC2 ***/
  k = 0;
  if (!first_lpd_flag) {
    lpc_present[0] = 1;
    lpc0_available = !last_lpc_lost;
    /* old LPC4 is new LPC0 */
    for (i = 0; i < M_LP_FILTER_ORDER; i++) {
      lsp[0][i] = lpc4_lsf[i];
    }
    /* skip LPC0 and continue with LPC2 */
    k = 2;
  }

  for (; k < l; k += 2) {
    int nk_mode = 0;

    if ((k == 2) && (mod[0] == 3)) {
      break; /* skip LPC2 */
    }

    lpc_present[k >> s] = 1;

    mode_lpc_bin = FDKreadBit(hBs);

    if (mode_lpc_bin == 0) {
      /* LPC0/LPC2: Abs */
      vlpc_1st_dec(hBs, lsp[k >> s]);
    } else {
      /* LPC0/LPC2: RelR */
      for (i = 0; i < M_LP_FILTER_ORDER; i++) {
        lsp[k >> s][i] = lsp[4 >> s][i];
      }
      nk_mode = 3;
    }

    err = vlpc_2st_dec(hBs, lsp[k >> s], nk_mode);
    if (err != 0) {
      return err;
    }
  }

  /*** Decode LPC1 ***/
  if (mod[0] < 2) { /* else: skip LPC1 */
    lpc_present[1] = 1;
    mode_lpc_bin = get_vlclbf_n(hBs, 2);

    switch (mode_lpc_bin) {
      case 1:
        /* LPC1: abs */
        vlpc_1st_dec(hBs, lsp[1]);
        err = vlpc_2st_dec(hBs, lsp[1], 0);
        if (err != 0) {
          return err;
        }
        break;
      case 2:
        /* LPC1: mid0 (no second stage AVQ quantizer in this case) */
        if (lpc0_available) { /* LPC0/lsf[0] might be zero some times */
          for (i = 0; i < M_LP_FILTER_ORDER; i++) {
            lsp[1][i] = (lsp[0][i] >> 1) + (lsp[2][i] >> 1);
          }
        } else {
          for (i = 0; i < M_LP_FILTER_ORDER; i++) {
            lsp[1][i] = lsp[2][i];
          }
        }
        break;
      case 0:
        /* LPC1: RelR */
        for (i = 0; i < M_LP_FILTER_ORDER; i++) {
          lsp[1][i] = lsp[2][i];
        }
        err = vlpc_2st_dec(hBs, lsp[1], 2 << s);
        if (err != 0) {
          return err;
        }
        break;
    }
  }

  /*** Decode LPC3 ***/
  if ((mod[2] < 2)) { /* else: skip LPC3 */
    int nk_mode = 0;
    lpc_present[3] = 1;

    mode_lpc_bin = get_vlclbf_n(hBs, 3);

    switch (mode_lpc_bin) {
      case 1:
        /* LPC3: abs */
        vlpc_1st_dec(hBs, lsp[3]);
        break;
      case 0:
        /* LPC3: mid */
        for (i = 0; i < M_LP_FILTER_ORDER; i++) {
          lsp[3][i] = (lsp[2][i] >> 1) + (lsp[4][i] >> 1);
        }
        nk_mode = 1;
        break;
      case 2:
        /* LPC3: relL */
        for (i = 0; i < M_LP_FILTER_ORDER; i++) {
          lsp[3][i] = lsp[2][i];
        }
        nk_mode = 2;
        break;
      case 3:
        /* LPC3: relR */
        for (i = 0; i < M_LP_FILTER_ORDER; i++) {
          lsp[3][i] = lsp[4][i];
        }
        nk_mode = 2;
        break;
    }
    err = vlpc_2st_dec(hBs, lsp[3], nk_mode);
    if (err != 0) {
      return err;
    }
  }

  if (!lpc0_available && !last_frame_ok) {
    /* LPC(0) was lost. Use next available LPC(k) instead */
    for (k = 1; k < (nbDiv + 1); k++) {
      if (lpc_present[k]) {
        for (i = 0; i < M_LP_FILTER_ORDER; i++) {
#define LSF_INIT_TILT (0.25f)
          if (mod[0] > 0) {
            lsp[0][i] = FX_DBL2FX_LPC(
                fMult(lsp[k][i], FL2FXCONST_SGL(1.0f - LSF_INIT_TILT)) +
                fMult(fdk_dec_lsf_init[i], FL2FXCONST_SGL(LSF_INIT_TILT)));
          } else {
            lsp[0][i] = lsp[k][i];
          }
        }
        break;
      }
    }
  }

  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    lpc4_lsf[i] = lsp[4 >> s][i];
  }

  {
    FIXP_DBL divFac;
    int last, numLpc = 0;

    i = nbDiv;
    do {
      numLpc += lpc_present[i--];
    } while (i >= 0 && numLpc < 3);

    last = i;

    switch (numLpc) {
      case 3:
        divFac = FL2FXCONST_DBL(1.0f / 3.0f);
        break;
      case 2:
        divFac = FL2FXCONST_DBL(1.0f / 2.0f);
        break;
      default:
        divFac = FL2FXCONST_DBL(1.0f);
        break;
    }

    /* get the adaptive mean for the next (bad) frame */
    for (k = 0; k < M_LP_FILTER_ORDER; k++) {
      FIXP_DBL tmp = (FIXP_DBL)0;
      for (i = nbDiv; i > last; i--) {
        if (lpc_present[i]) {
          tmp = fMultAdd(tmp >> 1, lsp[i][k], divFac);
        }
      }
      lsf_adaptive_mean_cand[k] = FX_DBL2FX_LPC(tmp);
    }
  }

  /* calculate stability factor Theta. Needed for ACELP decoder and concealment
   */
  {
    FIXP_LPC *lsf_prev, *lsf_curr;
    k = 0;

    FDK_ASSERT(lpc_present[0] == 1 && lpc_present[4 >> s] == 1);
    lsf_prev = lsp[0];
    for (i = 1; i < (nbDiv + 1); i++) {
      if (lpc_present[i]) {
        FIXP_DBL tmp = (FIXP_DBL)0;
        int j;
        lsf_curr = lsp[i];

        /* sum = tmp * 2^(LSF_SCALE*2 + 4) */
        for (j = 0; j < M_LP_FILTER_ORDER; j++) {
          tmp += fPow2Div2((FIXP_SGL)(lsf_curr[j] - lsf_prev[j])) >> 3;
        }

        /* tmp = (float)(FL2FXCONST_DBL(1.25f) - fMult(tmp,
         * FL2FXCONST_DBL(1/400000.0f))); */
        tmp = FL2FXCONST_DBL(1.25f / (1 << LSF_SCALE)) -
              fMult(tmp, FL2FXCONST_DBL((1 << (LSF_SCALE + 4)) / 400000.0f));
        if (tmp >= FL2FXCONST_DBL(1.0f / (1 << LSF_SCALE))) {
          pStability[k] = FL2FXCONST_SGL(1.0f / 2.0f);
        } else if (tmp < FL2FXCONST_DBL(0.0f)) {
          pStability[k] = FL2FXCONST_SGL(0.0f);
        } else {
          pStability[k] = FX_DBL2FX_SGL(tmp << (LSF_SCALE - 1));
        }

        lsf_prev = lsf_curr;
        k = i;
      } else {
        /* Mark stability value as undefined. */
        pStability[i] = (FIXP_SGL)-1;
      }
    }
  }

  /* convert into LSP domain */
  for (i = 0; i < (nbDiv + 1); i++) {
    if (lpc_present[i]) {
      for (k = 0; k < M_LP_FILTER_ORDER; k++) {
        lsp[i][k] = FX_DBL2FX_LPC(
            fixp_cos(fMult(lsp[i][k],
                           FL2FXCONST_SGL((1 << LSPARG_SCALE) * M_PI / 6400.0)),
                     LSF_SCALE - LSPARG_SCALE));
      }
    }
  }

  return 0;
}

void CLpc_Conceal(FIXP_LPC lsp[][M_LP_FILTER_ORDER],
                  FIXP_LPC lpc4_lsf[M_LP_FILTER_ORDER],
                  FIXP_LPC lsf_adaptive_mean[M_LP_FILTER_ORDER],
                  const int first_lpd_flag) {
  int i, j;

#define BETA (FL2FXCONST_SGL(0.25f))
#define ONE_BETA (FL2FXCONST_SGL(0.75f))
#define BFI_FAC (FL2FXCONST_SGL(0.90f))
#define ONE_BFI_FAC (FL2FXCONST_SGL(0.10f))

  /* Frame loss concealment (could be improved) */

  if (first_lpd_flag) {
    /* Reset past LSF values */
    for (i = 0; i < M_LP_FILTER_ORDER; i++) {
      lsp[0][i] = lpc4_lsf[i] = fdk_dec_lsf_init[i];
    }
  } else {
    /* old LPC4 is new LPC0 */
    for (i = 0; i < M_LP_FILTER_ORDER; i++) {
      lsp[0][i] = lpc4_lsf[i];
    }
  }

  /* LPC1 */
  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    FIXP_LPC lsf_mean = FX_DBL2FX_LPC(fMult(BETA, fdk_dec_lsf_init[i]) +
                                      fMult(ONE_BETA, lsf_adaptive_mean[i]));

    lsp[1][i] = FX_DBL2FX_LPC(fMult(BFI_FAC, lpc4_lsf[i]) +
                              fMult(ONE_BFI_FAC, lsf_mean));
  }

  /* LPC2 - LPC4 */
  for (j = 2; j <= 4; j++) {
    for (i = 0; i < M_LP_FILTER_ORDER; i++) {
      /* lsf_mean[i] =  FX_DBL2FX_LPC(fMult((FIXP_LPC)(BETA + j *
         FL2FXCONST_LPC(0.1f)), fdk_dec_lsf_init[i])
                                    + fMult((FIXP_LPC)(ONE_BETA - j *
         FL2FXCONST_LPC(0.1f)), lsf_adaptive_mean[i])); */

      FIXP_LPC lsf_mean = FX_DBL2FX_LPC(
          fMult((FIXP_SGL)(BETA + (FIXP_SGL)(j * (INT)FL2FXCONST_SGL(0.1f))),
                (FIXP_SGL)fdk_dec_lsf_init[i]) +
          fMult(
              (FIXP_SGL)(ONE_BETA - (FIXP_SGL)(j * (INT)FL2FXCONST_SGL(0.1f))),
              lsf_adaptive_mean[i]));

      lsp[j][i] = FX_DBL2FX_LPC(fMult(BFI_FAC, lsp[j - 1][i]) +
                                fMult(ONE_BFI_FAC, lsf_mean));
    }
  }

  /* Update past values for the future */
  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    lpc4_lsf[i] = lsp[4][i];
  }

  /* convert into LSP domain */
  for (j = 0; j < 5; j++) {
    for (i = 0; i < M_LP_FILTER_ORDER; i++) {
      lsp[j][i] = FX_DBL2FX_LPC(fixp_cos(
          fMult(lsp[j][i], FL2FXCONST_SGL((1 << LSPARG_SCALE) * M_PI / 6400.0)),
          LSF_SCALE - LSPARG_SCALE));
    }
  }
}

void E_LPC_a_weight(FIXP_LPC *wA, const FIXP_LPC *A, int m) {
  FIXP_DBL f;
  int i;

  f = FL2FXCONST_DBL(0.92f);
  for (i = 0; i < m; i++) {
    wA[i] = FX_DBL2FX_LPC(fMult(A[i], f));
    f = fMult(f, FL2FXCONST_DBL(0.92f));
  }
}

void CLpd_DecodeGain(FIXP_DBL *gain, INT *gain_e, int gain_code) {
  /* gain * 2^(gain_e) = 10^(gain_code/28) */
  *gain = fLdPow(
      FL2FXCONST_DBL(3.3219280948873623478703194294894 / 4.0), /* log2(10)*/
      2,
      fMultDiv2((FIXP_DBL)gain_code << (DFRACT_BITS - 1 - 7),
                FL2FXCONST_DBL(2.0f / 28.0f)),
      7, gain_e);
}

  /**
   * \brief *   Find the polynomial F1(z) or F2(z) from the LSPs.
   * This is performed by expanding the product polynomials:
   *
   * F1(z) =   product   ( 1 - 2 LSP_i z^-1 + z^-2 )
   *         i=0,2,4,6,8
   * F2(z) =   product   ( 1 - 2 LSP_i z^-1 + z^-2 )
   *         i=1,3,5,7,9
   *
   * where LSP_i are the LSPs in the cosine domain.
   * R.A.Salami    October 1990
   * \param lsp input, line spectral freq. (cosine domain)
   * \param f output, the coefficients of F1 or F2, scaled by 8 bits
   * \param n no of coefficients (m/2)
   * \param flag 1 : F1(z) ; 2 : F2(z)
   */

#define SF_F 8

static void get_lsppol(FIXP_LPC lsp[], FIXP_DBL f[], int n, int flag) {
  FIXP_DBL b;
  FIXP_LPC *plsp;
  int i, j;

  plsp = lsp + flag - 1;
  f[0] = FL2FXCONST_DBL(1.0f / (1 << SF_F));
  b = -FX_LPC2FX_DBL(*plsp);
  f[1] = b >> (SF_F - 1);
  for (i = 2; i <= n; i++) {
    plsp += 2;
    b = -FX_LPC2FX_DBL(*plsp);
    f[i] = SATURATE_LEFT_SHIFT((fMultDiv2(b, f[i - 1]) + (f[i - 2] >> 1)), 2,
                               DFRACT_BITS);
    for (j = i - 1; j > 1; j--) {
      f[j] = SATURATE_LEFT_SHIFT(
          ((f[j] >> 2) + fMultDiv2(b, f[j - 1]) + (f[j - 2] >> 2)), 2,
          DFRACT_BITS);
    }
    f[1] = f[1] + (b >> (SF_F - 1));
  }
  return;
}

#define NC M_LP_FILTER_ORDER / 2

/**
 * \brief lsp input LSP vector
 * \brief a output LP filter coefficient vector scaled by SF_A_COEFFS.
 */
void E_LPC_f_lsp_a_conversion(FIXP_LPC *lsp, FIXP_LPC *a, INT *a_exp) {
  FIXP_DBL f1[NC + 1], f2[NC + 1];
  int i, k;

  /*-----------------------------------------------------*
   *  Find the polynomials F1(z) and F2(z)               *
   *-----------------------------------------------------*/

  get_lsppol(lsp, f1, NC, 1);
  get_lsppol(lsp, f2, NC, 2);

  /*-----------------------------------------------------*
   *  Multiply F1(z) by (1+z^-1) and F2(z) by (1-z^-1)   *
   *-----------------------------------------------------*/
  scaleValues(f1, NC + 1, -2);
  scaleValues(f2, NC + 1, -2);

  for (i = NC; i > 0; i--) {
    f1[i] += f1[i - 1];
    f2[i] -= f2[i - 1];
  }

  FIXP_DBL aDBL[M_LP_FILTER_ORDER];

  for (i = 1, k = M_LP_FILTER_ORDER - 1; i <= NC; i++, k--) {
    aDBL[i - 1] = f1[i] + f2[i];
    aDBL[k] = f1[i] - f2[i];
  }

  int headroom_a = getScalefactor(aDBL, M_LP_FILTER_ORDER);

  for (i = 0; i < M_LP_FILTER_ORDER; i++) {
    a[i] = FX_DBL2FX_LPC(aDBL[i] << headroom_a);
  }

  *a_exp = SF_F + (2 - 1) - headroom_a;
}
