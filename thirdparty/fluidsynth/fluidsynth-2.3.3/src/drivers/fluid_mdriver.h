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

#ifndef _FLUID_MDRIVER_H
#define _FLUID_MDRIVER_H

#include "fluid_sys.h"

/*
 * fluid_midi_driver_t
 */

typedef struct _fluid_mdriver_definition_t fluid_mdriver_definition_t;

struct _fluid_midi_driver_t
{
    const fluid_mdriver_definition_t *define;
    handle_midi_event_func_t handler;
    void *data;
};

void fluid_midi_driver_settings(fluid_settings_t *settings);

/* ALSA */
#if ALSA_SUPPORT
fluid_midi_driver_t *new_fluid_alsa_rawmidi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_alsa_rawmidi_driver(fluid_midi_driver_t *p);
void fluid_alsa_rawmidi_driver_settings(fluid_settings_t *settings);

fluid_midi_driver_t *new_fluid_alsa_seq_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_alsa_seq_driver(fluid_midi_driver_t *p);
void fluid_alsa_seq_driver_settings(fluid_settings_t *settings);
#endif

/* JACK */
#if JACK_SUPPORT
void fluid_jack_midi_driver_settings(fluid_settings_t *settings);
fluid_midi_driver_t *new_fluid_jack_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *data);
void delete_fluid_jack_midi_driver(fluid_midi_driver_t *p);
#endif

/* OSS */
#if OSS_SUPPORT
fluid_midi_driver_t *new_fluid_oss_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_oss_midi_driver(fluid_midi_driver_t *p);
void fluid_oss_midi_driver_settings(fluid_settings_t *settings);
#endif

/* Windows MIDI service */
#if WINMIDI_SUPPORT
fluid_midi_driver_t *new_fluid_winmidi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_winmidi_driver(fluid_midi_driver_t *p);
void fluid_winmidi_midi_driver_settings(fluid_settings_t *settings);
#endif

/* definitions for the MidiShare driver */
#if MIDISHARE_SUPPORT
fluid_midi_driver_t *new_fluid_midishare_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_midishare_midi_driver(fluid_midi_driver_t *p);
#endif

/* definitions for the CoreMidi driver */
#if COREMIDI_SUPPORT
fluid_midi_driver_t *new_fluid_coremidi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
void delete_fluid_coremidi_driver(fluid_midi_driver_t *p);
void fluid_coremidi_driver_settings(fluid_settings_t *settings);
#endif

#endif  /* _FLUID_AUDRIVER_H */
