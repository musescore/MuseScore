/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#ifndef _FLUID_PHASE_H
#define _FLUID_PHASE_H

#include "fluid_config.h"

/*
 *  phase
 */

#define FLUID_INTERP_BITS        8
#define FLUID_INTERP_BITS_MASK   0xff000000
#define FLUID_INTERP_BITS_SHIFT  24
#define FLUID_INTERP_MAX         256

#define FLUID_FRACT_MAX ((double)4294967296.0)

/* fluid_phase_t
* Purpose:
* Playing pointer for voice playback
*
* When a sample is played back at a different pitch, the playing pointer in the
* source sample will not advance exactly one sample per output sample.
* This playing pointer is implemented using fluid_phase_t.
* It is a 64 bit number. The higher 32 bits contain the 'index' (number of
* the current sample), the lower 32 bits the fractional part.
*/
typedef unsigned long long fluid_phase_t;

/* Purpose:
 * Set a to b.
 * a: fluid_phase_t
 * b: fluid_phase_t
 */
#define fluid_phase_set(a,b) a=b;

#define fluid_phase_set_int(a, b)    ((a) = ((unsigned long long)(b)) << 32)

/* Purpose:
 * Sets the phase a to a phase increment given in b.
 * For example, assume b is 0.9. After setting a to it, adding a to
 * the playing pointer will advance it by 0.9 samples. */
#define fluid_phase_set_float(a, b) \
  (a) = (((unsigned long long)(b)) << 32) \
  | (uint32) (((double)(b) - (int)(b)) * (double)FLUID_FRACT_MAX)

/* create a fluid_phase_t from an index and a fraction value */
#define fluid_phase_from_index_fract(index, fract) \
  ((((unsigned long long)(index)) << 32) + (fract))

/* Purpose:
 * Return the index and the fractional part, respectively. */
#define fluid_phase_index(_x) \
  ((unsigned int)((_x) >> 32))
#define fluid_phase_fract(_x) \
  ((uint32)((_x) & 0xFFFFFFFF))

/* Get the phase index with fractional rounding */
#define fluid_phase_index_round(_x) \
  ((unsigned int)(((_x) + 0x80000000) >> 32))


/* Purpose:
 * Takes the fractional part of the argument phase and
 * calculates the corresponding position in the interpolation table.
 * The fractional position of the playing pointer is calculated with a quite high
 * resolution (32 bits). It would be unpractical to keep a set of interpolation
 * coefficients for each possible fractional part...
 */
#define fluid_phase_fract_to_tablerow(_x) \
  ((unsigned int)(fluid_phase_fract(_x) & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT)

#define fluid_phase_double(_x) \
  ((double)(fluid_phase_index(_x)) + ((double)fluid_phase_fract(_x) / FLUID_FRACT_MAX))

/* Purpose:
 * Advance a by a step of b (both are fluid_phase_t).
 */
#define fluid_phase_incr(a, b)  a += b

/* Purpose:
 * Subtract b from a (both are fluid_phase_t).
 */
#define fluid_phase_decr(a, b)  a -= b

/* Purpose:
 * Subtract b samples from a.
 */
#define fluid_phase_sub_int(a, b)  ((a) -= (unsigned long long)(b) << 32)

/* Purpose:
 * Creates the expression a.index++. */
#define fluid_phase_index_plusplus(a)  (((a) += 0x100000000LL)

#endif  /* _FLUID_PHASE_H */
