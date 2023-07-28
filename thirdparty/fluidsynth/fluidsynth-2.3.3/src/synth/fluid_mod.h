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

#ifndef _FLUID_MOD_H
#define _FLUID_MOD_H

#include "fluidsynth_priv.h"
#include "fluid_conv.h"

/*
 * Modulator structure.  See SoundFont 2.04 PDF section 8.2.
 */
struct _fluid_mod_t
{
    unsigned char dest;           /**< Destination generator to control */
    unsigned char src1;           /**< Source controller 1 */
    unsigned char flags1;         /**< Source controller 1 flags */
    unsigned char src2;           /**< Source controller 2 */
    unsigned char flags2;         /**< Source controller 2 flags */
    double amount;                /**< Multiplier amount */
    /* The 'next' field allows to link modulators into a list.  It is
     * not used in fluid_voice.c, there each voice allocates memory for a
     * fixed number of modulators.  Since there may be a huge number of
     * different zones, this is more efficient.
     */
    fluid_mod_t *next;
};

fluid_real_t fluid_mod_get_value(fluid_mod_t *mod, fluid_voice_t *voice);
int fluid_mod_check_sources(const fluid_mod_t *mod, char *name);

#ifdef DEBUG
void fluid_dump_modulator(fluid_mod_t *mod);
#endif


#endif /* _FLUID_MOD_H */
