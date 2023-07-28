/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


/*

  More information about micro tuning can be found at:

  https://www.midi.org/about-midi/tuning.htm
  https://www.midi.org/about-midi/tuning-scale.htm
  https://www.midi.org/about-midi/tuning_extens.htm

*/

#ifndef _FLUID_TUNING_H
#define _FLUID_TUNING_H

#include "fluidsynth_priv.h"

struct _fluid_tuning_t
{
    char *name;
    int bank;
    int prog;
    double pitch[128];  /* the pitch of every key, in cents */
    fluid_atomic_int_t refcount;         /* Tuning reference count */
};

fluid_tuning_t *new_fluid_tuning(const char *name, int bank, int prog);
void delete_fluid_tuning(fluid_tuning_t *tuning);
fluid_tuning_t *fluid_tuning_duplicate(fluid_tuning_t *tuning);
void fluid_tuning_ref(fluid_tuning_t *tuning);
int fluid_tuning_unref(fluid_tuning_t *tuning, int count);

int fluid_tuning_set_name(fluid_tuning_t *tuning, const char *name);
char *fluid_tuning_get_name(fluid_tuning_t *tuning);

#define fluid_tuning_get_bank(_t) ((_t)->bank)
#define fluid_tuning_get_prog(_t) ((_t)->prog)

void fluid_tuning_set_pitch(fluid_tuning_t *tuning, int key, double pitch);
#define fluid_tuning_get_pitch(_t, _key) ((_t)->pitch[_key])

void fluid_tuning_set_octave(fluid_tuning_t *tuning, const double *pitch_deriv);

void fluid_tuning_set_all(fluid_tuning_t *tuning, const double *pitch);
#define fluid_tuning_get_all(_t) (&(_t)->pitch[0])




#endif /* _FLUID_TUNING_H */
