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

/**************************** AAC decoder library ******************************

   Author(s):

   Description: ACELP

*******************************************************************************/

#include "usacdec_ace_d4t64.h"

#define L_SUBFR 64 /* Subframe size              */

/*
 * D_ACELP_add_pulse
 *
 * Parameters:
 *    pos         I: position of pulse
 *    nb_pulse    I: number of pulses
 *    track       I: track
 *    code        O: fixed codebook
 *
 * Function:
 *    Add pulses to fixed codebook
 *
 * Returns:
 *    void
 */
static void D_ACELP_add_pulse(SHORT pos[], SHORT nb_pulse, SHORT track,
                              FIXP_COD code[]) {
  SHORT i, k;
  for (k = 0; k < nb_pulse; k++) {
    /* i = ((pos[k] & (16-1))*NB_TRACK) + track; */
    i = ((pos[k] & (16 - 1)) << 2) + track;
    if ((pos[k] & 16) == 0) {
      code[i] = code[i] + (FIXP_COD)(512 << (COD_BITS - FRACT_BITS));
    } else {
      code[i] = code[i] - (FIXP_COD)(512 << (COD_BITS - FRACT_BITS));
    }
  }
  return;
}
/*
 * D_ACELP_decode_1p_N1
 *
 * Parameters:
 *    index    I: pulse index
 *    N        I: number of bits for position
 *    offset   I: offset
 *    pos      O: position of the pulse
 *
 * Function:
 *    Decode 1 pulse with N+1 bits
 *
 * Returns:
 *    void
 */
static void D_ACELP_decode_1p_N1(LONG index, SHORT N, SHORT offset,
                                 SHORT pos[]) {
  SHORT pos1;
  LONG i, mask;

  mask = ((1 << N) - 1);
  /*
   * Decode 1 pulse with N+1 bits
   */
  pos1 = (SHORT)((index & mask) + offset);
  i = ((index >> N) & 1);
  if (i == 1) {
    pos1 += 16;
  }
  pos[0] = pos1;
  return;
}
/*
 * D_ACELP_decode_2p_2N1
 *
 * Parameters:
 *    index    I: pulse index
 *    N        I: number of bits for position
 *    offset   I: offset
 *    pos      O: position of the pulse
 *
 * Function:
 *    Decode 2 pulses with 2*N+1 bits
 *
 * Returns:
 *    void
 */
static void D_ACELP_decode_2p_2N1(LONG index, SHORT N, SHORT offset,
                                  SHORT pos[]) {
  SHORT pos1, pos2;
  LONG mask, i;
  mask = ((1 << N) - 1);
  /*
   * Decode 2 pulses with 2*N+1 bits
   */
  pos1 = (SHORT)(((index >> N) & mask) + offset);
  i = (index >> (2 * N)) & 1;
  pos2 = (SHORT)((index & mask) + offset);
  if ((pos2 - pos1) < 0) {
    if (i == 1) {
      pos1 += 16;
    } else {
      pos2 += 16;
    }
  } else {
    if (i == 1) {
      pos1 += 16;
      pos2 += 16;
    }
  }
  pos[0] = pos1;
  pos[1] = pos2;
  return;
}
/*
 * D_ACELP_decode_3p_3N1
 *
 * Parameters:
 *    index    I: pulse index
 *    N        I: number of bits for position
 *    offset   I: offset
 *    pos      O: position of the pulse
 *
 * Function:
 *    Decode 3 pulses with 3*N+1 bits
 *
 * Returns:
 *    void
 */
static void D_ACELP_decode_3p_3N1(LONG index, SHORT N, SHORT offset,
                                  SHORT pos[]) {
  SHORT j;
  LONG mask, idx;

  /*
   * Decode 3 pulses with 3*N+1 bits
   */
  mask = ((1 << ((2 * N) - 1)) - 1);
  idx = index & mask;
  j = offset;
  if (((index >> ((2 * N) - 1)) & 1) == 1) {
    j += (1 << (N - 1));
  }
  D_ACELP_decode_2p_2N1(idx, N - 1, j, pos);
  mask = ((1 << (N + 1)) - 1);
  idx = (index >> (2 * N)) & mask;
  D_ACELP_decode_1p_N1(idx, N, offset, pos + 2);
  return;
}
/*
 * D_ACELP_decode_4p_4N1
 *
 * Parameters:
 *    index    I: pulse index
 *    N        I: number of bits for position
 *    offset   I: offset
 *    pos      O: position of the pulse
 *
 * Function:
 *    Decode 4 pulses with 4*N+1 bits
 *
 * Returns:
 *    void
 */
static void D_ACELP_decode_4p_4N1(LONG index, SHORT N, SHORT offset,
                                  SHORT pos[]) {
  SHORT j;
  LONG mask, idx;
  /*
   * Decode 4 pulses with 4*N+1 bits
   */
  mask = ((1 << ((2 * N) - 1)) - 1);
  idx = index & mask;
  j = offset;
  if (((index >> ((2 * N) - 1)) & 1) == 1) {
    j += (1 << (N - 1));
  }
  D_ACELP_decode_2p_2N1(idx, N - 1, j, pos);
  mask = ((1 << ((2 * N) + 1)) - 1);
  idx = (index >> (2 * N)) & mask;
  D_ACELP_decode_2p_2N1(idx, N, offset, pos + 2);
  return;
}
/*
 * D_ACELP_decode_4p_4N
 *
 * Parameters:
 *    index    I: pulse index
 *    N        I: number of bits for position
 *    offset   I: offset
 *    pos      O: position of the pulse
 *
 * Function:
 *    Decode 4 pulses with 4*N bits
 *
 * Returns:
 *    void
 */
static void D_ACELP_decode_4p_4N(LONG index, SHORT N, SHORT offset,
                                 SHORT pos[]) {
  SHORT j, n_1;
  /*
   * Decode 4 pulses with 4*N bits
   */
  n_1 = N - 1;
  j = offset + (1 << n_1);
  switch ((index >> ((4 * N) - 2)) & 3) {
    case 0:
      if (((index >> ((4 * n_1) + 1)) & 1) == 0) {
        D_ACELP_decode_4p_4N1(index, n_1, offset, pos);
      } else {
        D_ACELP_decode_4p_4N1(index, n_1, j, pos);
      }
      break;
    case 1:
      D_ACELP_decode_1p_N1((index >> ((3 * n_1) + 1)), n_1, offset, pos);
      D_ACELP_decode_3p_3N1(index, n_1, j, pos + 1);
      break;
    case 2:
      D_ACELP_decode_2p_2N1((index >> ((2 * n_1) + 1)), n_1, offset, pos);
      D_ACELP_decode_2p_2N1(index, n_1, j, pos + 2);
      break;
    case 3:
      D_ACELP_decode_3p_3N1((index >> (n_1 + 1)), n_1, offset, pos);
      D_ACELP_decode_1p_N1(index, n_1, j, pos + 3);
      break;
  }
  return;
}

/*
 * D_ACELP_decode_4t
 *
 * Parameters:
 *    index          I: index
 *    mode           I: speech mode
 *    code           I: (Q9) algebraic (fixed) codebook excitation
 *
 * Function:
 *    20, 36, 44, 52, 64, 72, 88 bits algebraic codebook.
 *    4 tracks x 16 positions per track = 64 samples.
 *
 *    20 bits 5+5+5+5 --> 4 pulses in a frame of 64 samples.
 *    36 bits 9+9+9+9 --> 8 pulses in a frame of 64 samples.
 *    44 bits 13+9+13+9 --> 10 pulses in a frame of 64 samples.
 *    52 bits 13+13+13+13 --> 12 pulses in a frame of 64 samples.
 *    64 bits 2+2+2+2+14+14+14+14 --> 16 pulses in a frame of 64 samples.
 *    72 bits 10+2+10+2+10+14+10+14 --> 18 pulses in a frame of 64 samples.
 *    88 bits 11+11+11+11+11+11+11+11 --> 24 pulses in a frame of 64 samples.
 *
 *    All pulses can have two (2) possible amplitudes: +1 or -1.
 *    Each pulse can sixteen (16) possible positions.
 *
 *    codevector length    64
 *    number of track      4
 *    number of position   16
 *
 * Returns:
 *    void
 */
void D_ACELP_decode_4t64(SHORT index[], int nbits, FIXP_COD code[]) {
  LONG L_index;
  SHORT k, pos[6];

  FDKmemclear(code, L_SUBFR * sizeof(FIXP_COD));

  /* decode the positions and signs of pulses and build the codeword */
  switch (nbits) {
    case 12:
      for (k = 0; k < 4; k += 2) {
        L_index = index[2 * (k / 2) + 1];
        D_ACELP_decode_1p_N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 1, 2 * (index[2 * (k / 2)]) + k / 2, code);
      }
      break;
    case 16: {
      int i = 0;
      int offset = index[i++];
      offset = (offset == 0) ? 1 : 3;
      for (k = 0; k < 4; k++) {
        if (k != offset) {
          L_index = index[i++];
          D_ACELP_decode_1p_N1(L_index, 4, 0, pos);
          D_ACELP_add_pulse(pos, 1, k, code);
        }
      }
    } break;
    case 20:
      for (k = 0; k < 4; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_1p_N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 1, k, code);
      }
      break;
    case 28:
      for (k = 0; k < 4 - 2; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_2p_2N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 2, k, code);
      }
      for (k = 2; k < 4; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_1p_N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 1, k, code);
      }
      break;
    case 36:
      for (k = 0; k < 4; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_2p_2N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 2, k, code);
      }
      break;
    case 44:
      for (k = 0; k < 4 - 2; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_3p_3N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 3, k, code);
      }
      for (k = 2; k < 4; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_2p_2N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 2, k, code);
      }
      break;
    case 52:
      for (k = 0; k < 4; k++) {
        L_index = (LONG)index[k];
        D_ACELP_decode_3p_3N1(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 3, k, code);
      }
      break;
    case 64:
      for (k = 0; k < 4; k++) {
        L_index = (((LONG)index[k] << 14) + (LONG)index[k + 4]);
        D_ACELP_decode_4p_4N(L_index, 4, 0, pos);
        D_ACELP_add_pulse(pos, 4, k, code);
      }
      break;
    default:
      FDK_ASSERT(0);
  }
  return;
}
