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

#ifndef _FLUID_SEQ_QUE_H
#define _FLUID_SEQ_QUE_H

#include "fluidsynth.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "fluid_event.h"
#include "fluid_seqbind_notes.h"

void* new_fluid_seq_queue(int nbEvents);
void delete_fluid_seq_queue(void *queue);
int fluid_seq_queue_push(void *queue, const fluid_event_t *evt);
void fluid_seq_queue_remove(void *queue, fluid_seq_id_t src, fluid_seq_id_t dest, int type);
void fluid_seq_queue_process(void *que, fluid_sequencer_t *seq, unsigned int cur_ticks);
void fluid_seq_queue_invalidate_note_private(void *que, fluid_seq_id_t dest, fluid_note_id_t id);

int event_compare_for_test(const fluid_event_t* left, const fluid_event_t* right);

#ifdef __cplusplus
}
#endif

#endif /* _FLUID_SEQ_QUE_H */
