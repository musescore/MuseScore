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

#ifndef _FLUIDSYNTH_MOD_H
#define _FLUIDSYNTH_MOD_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Modulator-related definitions */

  /* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD           64

  /*
   *  fluid_mod_t
   */
struct _fluid_mod_t
{
  unsigned char dest;
  unsigned char src1;
  unsigned char flags1;
  unsigned char src2;
  unsigned char flags2;
  double amount;
  /* The 'next' field allows to link modulators into a list.  It is
   * not used in fluid_voice.c, there each voice allocates memory for a
   * fixed number of modulators.  Since there may be a huge number of
   * different zones, this is more efficient.
   */
  fluid_mod_t * next;
};

/* Flags telling the polarity of a modulator.  Compare with SF2.01
   section 8.2. Note: The numbers of the bits are different!  (for
   example: in the flags of a SF modulator, the polarity bit is bit
   nr. 9) */
enum fluid_mod_flags
{
  FLUID_MOD_POSITIVE = 0,
  FLUID_MOD_NEGATIVE = 1,
  FLUID_MOD_UNIPOLAR = 0,
  FLUID_MOD_BIPOLAR = 2,
  FLUID_MOD_LINEAR = 0,
  FLUID_MOD_CONCAVE = 4,
  FLUID_MOD_CONVEX = 8,
  FLUID_MOD_SWITCH = 12,
  FLUID_MOD_GC = 0,
  FLUID_MOD_CC = 16
};

/* Flags telling the source of a modulator.  This corresponds to
 * SF2.01 section 8.2.1 */
enum fluid_mod_src
{
  FLUID_MOD_NONE = 0,
  FLUID_MOD_VELOCITY = 2,
  FLUID_MOD_KEY = 3,
  FLUID_MOD_KEYPRESSURE = 10,
  FLUID_MOD_CHANNELPRESSURE = 13,
  FLUID_MOD_PITCHWHEEL = 14,
  FLUID_MOD_PITCHWHEELSENS = 16
};

/* Allocates memory for a new modulator */
FLUIDSYNTH_API fluid_mod_t * fluid_mod_new(void);

/* Frees the modulator */
FLUIDSYNTH_API void fluid_mod_delete(fluid_mod_t * mod);


FLUIDSYNTH_API void fluid_mod_set_source1(fluid_mod_t* mod, int src, int flags); 
FLUIDSYNTH_API void fluid_mod_set_source2(fluid_mod_t* mod, int src, int flags); 
FLUIDSYNTH_API void fluid_mod_set_dest(fluid_mod_t* mod, int dst); 
FLUIDSYNTH_API void fluid_mod_set_amount(fluid_mod_t* mod, double amount); 

FLUIDSYNTH_API int fluid_mod_get_source1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_source2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_dest(fluid_mod_t* mod);
FLUIDSYNTH_API double fluid_mod_get_amount(fluid_mod_t* mod);


/* Determines, if two modulators are 'identical' (all parameters
   except the amount match) */
FLUIDSYNTH_API int fluid_mod_test_identity(fluid_mod_t * mod1, fluid_mod_t * mod2);


#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_MOD_H */

