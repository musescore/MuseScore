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


#ifndef _FLUID_CHORUS_H
#define _FLUID_CHORUS_H

#include "fluidsynth_priv.h"


typedef struct _fluid_chorus_t fluid_chorus_t;

/*
 * chorus
 */
fluid_chorus_t* new_fluid_chorus(fluid_real_t sample_rate);
void delete_fluid_chorus(fluid_chorus_t* chorus);
void fluid_chorus_processmix(fluid_chorus_t* chorus, fluid_real_t *in,
			    fluid_real_t *left_out, fluid_real_t *right_out);
void fluid_chorus_processreplace(fluid_chorus_t* chorus, fluid_real_t *in,
				fluid_real_t *left_out, fluid_real_t *right_out);

int fluid_chorus_init(fluid_chorus_t* chorus);
void fluid_chorus_reset(fluid_chorus_t* chorus);

void fluid_chorus_set_nr(fluid_chorus_t* chorus, int nr);
void fluid_chorus_set_level(fluid_chorus_t* chorus, fluid_real_t level);
void fluid_chorus_set_speed_Hz(fluid_chorus_t* chorus, fluid_real_t speed_Hz);
void fluid_chorus_set_depth_ms(fluid_chorus_t* chorus, fluid_real_t depth_ms);
void fluid_chorus_set_type(fluid_chorus_t* chorus, int type);
int fluid_chorus_update(fluid_chorus_t* chorus);
int fluid_chorus_get_nr(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_level(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_speed_Hz(fluid_chorus_t* chorus);
fluid_real_t fluid_chorus_get_depth_ms(fluid_chorus_t* chorus);
int fluid_chorus_get_type(fluid_chorus_t* chorus);


#endif /* _FLUID_CHORUS_H */
