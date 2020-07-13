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


#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

#include "fluidsynth_priv.h"

typedef struct _fluid_gen_info_t {
	char num;		/* Generator number */
	char init;		/* Does the generator need to be initialized (cfr. fluid_voice_init()) */
	char nrpn_scale;	/* The scale to convert from NRPN (cfr. fluid_gen_map_nrpn()) */
	float min;		/* The minimum value */
	float max;		/* The maximum value */
	float def;		/* The default value (cfr. fluid_gen_set_default_values()) */
} fluid_gen_info_t;

#define fluid_gen_set_mod(_gen, _val)  { (_gen)->mod = (double) (_val); }
#define fluid_gen_set_nrpn(_gen, _val) { (_gen)->nrpn = (double) (_val); }

fluid_real_t fluid_gen_scale(int gen, float value);
fluid_real_t fluid_gen_scale_nrpn(int gen, int nrpn);
int fluid_gen_init(fluid_gen_t* gen, fluid_channel_t* channel);


#endif /* _FLUID_GEN_H */
