/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2019  Tom Moebert and others.
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

#include "fluid_seqbind_notes.h"

#include <set>

/*
 * This is a hash container allows us to detect overlapping notes, by storing a bunch of unique integers,
 * that allow us to track noteOn events.
 * If an ID is part of the container, it means that we have received a noteOn on a certain channel and key.
 * Once we receive a noteOff, we remove that ID again.
 *
 * Unfortunately, I can't think of a way to detect overlapping notes by using the synth only. One might
 * think, that it's possible to use fluid_synth_get_voicelist(), in order to get a list of active voices
 * and then detect overlaps. However, this doesn't reliably work, because voices may finish, before the
 * noteOff is received. Think of short percussion samples spawned by long MIDI note durations.
 *
 * Here is an example of how it might look like. The "ticks" are equivalent to the time parameter passed
 * into fluid_event_callback_t.
 *
fluidsynth: debug: Tick 1728: Note on chan 15, key 44, ends at tick 1824
fluidsynth: debug: Tick 1825: Normal NoteOFF on chan 15, key 44
(so far, so usual)

fluidsynth: debug: Tick 1825: Note on chan 15, key 44, dur 96, ends at tick 1921

oops, the voice spawned by the previous note already finished at tick 1900, but the noteOff is yet to come

fluidsynth: debug: Tick 1920: Note on chan 15, key 44, dur 143, ends at tick 2063
(Shit, we got another noteOn before the last noteOff. If we check for playing voices now, we won't find any,
because they have all finished.)

fluidsynth: debug: Tick 1921: Normal NoteOFF on chan 15, key 44
(... which means that we cannot detect an overlap, thus we cannot remove this noteoff, thus
this noteoff will immediately kill the voice that we've just started 1 tick ago)
*/

typedef std::set<fluid_note_id_t> note_container_t;

// Compute a unique ID for a given channel-key combination. Think of it as a two-dimensional array index.
fluid_note_id_t fluid_note_compute_id(int chan, short key)
{
    return 128 * chan + key;
}

void* new_fluid_note_container()
{
    try
    {
        note_container_t* cont = new note_container_t;
        return cont;
    }
    catch(...)
    {
        return 0;
    }
}

void delete_fluid_note_container(void *cont)
{
    delete static_cast<note_container_t*>(cont);
}

// Returns true, if the ID was already included in the container before, false if it was just inserted and
// FLUID_FAILED in case of error.
int fluid_note_container_insert(void* cont, fluid_note_id_t id)
{
    try
    {
        std::pair<note_container_t::iterator, bool> res = static_cast<note_container_t*>(cont)->insert(id);
        // res.second tells us whether the element was inserted
        // by inverting it, we know whether it contained the element previously
        return !res.second;
    }
    catch(...)
    {
        return FLUID_FAILED;
    }
}

void fluid_note_container_remove(void* cont, fluid_note_id_t id)
{
    try
    {
        static_cast<note_container_t*>(cont)->erase(id);
    }
    catch(...)
    {
        // should never happen
    }
}

// Empties the entire collection, e.g. in case of a AllNotesOff event
void fluid_note_container_clear(void* cont)
{
    static_cast<note_container_t*>(cont)->clear();
}
