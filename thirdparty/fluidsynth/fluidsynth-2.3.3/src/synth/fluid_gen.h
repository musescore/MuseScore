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


#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

#include "fluidsynth_priv.h"

typedef struct _fluid_gen_info_t
{
    char num;		/* Generator number */
    char *name;
    char init;		/* Does the generator need to be initialized (not used) */
    char nrpn_scale;	/* The scale to convert from NRPN (cfr. fluid_gen_map_nrpn()) */
    float min;		/* The minimum value */
    float max;		/* The maximum value */
    float def;		/* The default value (cfr. fluid_gen_init()) */
} fluid_gen_info_t;

/*
 * SoundFont generator structure.
 */
typedef struct _fluid_gen_t
{
    unsigned char flags; /**< Is the generator set or not (#fluid_gen_flags) */
    double val;          /**< The nominal value */
    double mod;          /**< Change by modulators */
    double nrpn;         /**< Change by NRPN messages */
} fluid_gen_t;

/*
 * Enum value for 'flags' field of #fluid_gen_t (not really flags).
 */
enum fluid_gen_flags
{
    GEN_UNUSED,		/**< Generator value is not set */
    GEN_SET,		/**< Generator value is set */
};

#define fluid_gen_set_mod(_gen, _val)  { (_gen)->mod = (double) (_val); }
#define fluid_gen_set_nrpn(_gen, _val) { (_gen)->nrpn = (double) (_val); }

fluid_real_t fluid_gen_scale(int gen, float value);
fluid_real_t fluid_gen_scale_nrpn(int gen, int nrpn);
void fluid_gen_init(fluid_gen_t *gen, fluid_channel_t *channel);
const char *fluid_gen_name(int gen);


#endif /* _FLUID_GEN_H */
