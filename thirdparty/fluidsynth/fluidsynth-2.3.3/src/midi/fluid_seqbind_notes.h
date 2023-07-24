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

#ifndef _FLUID_SEQBIND_NOTE_H
#define _FLUID_SEQBIND_NOTE_H

#include "fluidsynth.h"
#include "fluid_event.h"

#ifdef __cplusplus
extern "C" {
#endif

fluid_note_id_t fluid_note_compute_id(int chan, short key);
void* new_fluid_note_container(void);
void delete_fluid_note_container(void *cont);
int fluid_note_container_insert(void* cont, fluid_note_id_t id);
void fluid_note_container_remove(void* cont, fluid_note_id_t id);
void fluid_note_container_clear(void* cont);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_SEQBIND_NOTE_H */
