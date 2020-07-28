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


#ifndef _FLUID_REV_H
#define _FLUID_REV_H

#include "fluidsynth_priv.h"

typedef struct _fluid_revmodel_t fluid_revmodel_t;


/*
 * reverb
 */
fluid_revmodel_t* new_fluid_revmodel(void);
void delete_fluid_revmodel(fluid_revmodel_t* rev);

void fluid_revmodel_processmix(fluid_revmodel_t* rev, fluid_real_t *in,
			      fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_processreplace(fluid_revmodel_t* rev, fluid_real_t *in,
				  fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_reset(fluid_revmodel_t* rev);

void fluid_revmodel_setroomsize(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setdamp(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setlevel(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setwidth(fluid_revmodel_t* rev, fluid_real_t value);
void fluid_revmodel_setmode(fluid_revmodel_t* rev, fluid_real_t value);

fluid_real_t fluid_revmodel_getroomsize(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getdamp(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getlevel(fluid_revmodel_t* rev);
fluid_real_t fluid_revmodel_getwidth(fluid_revmodel_t* rev);

/*
 * reverb preset
 */
typedef struct _fluid_revmodel_presets_t {
  char* name;
  fluid_real_t roomsize;
  fluid_real_t damp;
  fluid_real_t width;
  fluid_real_t level;
} fluid_revmodel_presets_t;


#endif /* _FLUID_REV_H */
