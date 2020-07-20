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


#ifndef _FLUID_RAMSFONT_H
#define _FLUID_RAMSFONT_H


#include "fluidlite.h"
#include "fluidsynth_priv.h"

#include "fluid_defsfont.h"


#ifdef __cplusplus
extern "C" {
#endif



/*

  Public interface

 */

int fluid_ramsfont_sfont_delete(fluid_sfont_t* sfont);
char* fluid_ramsfont_sfont_get_name(fluid_sfont_t* sfont);
fluid_preset_t* fluid_ramsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);
void fluid_ramsfont_sfont_iteration_start(fluid_sfont_t* sfont);
int fluid_ramsfont_sfont_iteration_next(fluid_sfont_t* sfont, fluid_preset_t* preset);


int fluid_rampreset_preset_delete(fluid_preset_t* preset);
char* fluid_rampreset_preset_get_name(fluid_preset_t* preset);
int fluid_rampreset_preset_get_banknum(fluid_preset_t* preset);
int fluid_rampreset_preset_get_num(fluid_preset_t* preset);
int fluid_rampreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);


/*
 * fluid_ramsfont_t
 */
struct _fluid_ramsfont_t
{
  char name[21];                        /* the name of the soundfont */
  fluid_list_t* sample;    /* the samples in this soundfont */
  fluid_rampreset_t* preset;    /* the presets of this soundfont */

  fluid_preset_t iter_preset;        /* preset interface used in the iteration */
  fluid_rampreset_t* iter_cur;       /* the current preset in the iteration */
};

/* interface */
fluid_ramsfont_t* new_fluid_ramsfont(void);
int delete_fluid_ramsfont(fluid_ramsfont_t* sfont);
char* fluid_ramsfont_get_name(fluid_ramsfont_t* sfont);
fluid_rampreset_t* fluid_ramsfont_get_preset(fluid_ramsfont_t* sfont, unsigned int bank, unsigned int prenum);
void fluid_ramsfont_iteration_start(fluid_ramsfont_t* sfont);
int fluid_ramsfont_iteration_next(fluid_ramsfont_t* sfont, fluid_preset_t* preset);
/* specific */



/*
 * fluid_preset_t
 */
struct _fluid_rampreset_t
{
  fluid_rampreset_t* next;
  fluid_ramsfont_t* sfont;                  /* the soundfont this preset belongs to */
  char name[21];                        /* the name of the preset */
  unsigned int bank;                    /* the bank number */
  unsigned int num;                     /* the preset number */
  fluid_preset_zone_t* global_zone;        /* the global zone of the preset */
  fluid_preset_zone_t* zone;               /* the chained list of preset zones */
  fluid_list_t *presetvoices;									/* chained list of used voices */
};

/* interface */
fluid_rampreset_t* new_fluid_rampreset(fluid_ramsfont_t* sfont);
int delete_fluid_rampreset(fluid_rampreset_t* preset);
fluid_rampreset_t* fluid_rampreset_next(fluid_rampreset_t* preset);
char* fluid_rampreset_get_name(fluid_rampreset_t* preset);
int fluid_rampreset_get_banknum(fluid_rampreset_t* preset);
int fluid_rampreset_get_num(fluid_rampreset_t* preset);
int fluid_rampreset_noteon(fluid_rampreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);





#ifdef __cplusplus
}
#endif

#endif  /* _FLUID_SFONT_H */
