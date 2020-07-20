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

#ifndef _FLUIDSYNTH_TYPES_H
#define _FLUIDSYNTH_TYPES_H



#ifdef __cplusplus
extern "C" {
#endif


/*

  Forward declarations

*/
typedef struct _fluid_hashtable_t fluid_settings_t;
typedef struct _fluid_synth_t fluid_synth_t;
typedef struct _fluid_voice_t fluid_voice_t;
typedef struct _fluid_sfloader_t fluid_sfloader_t;
typedef struct _fluid_sfont_t fluid_sfont_t;
typedef struct _fluid_preset_t fluid_preset_t;
typedef struct _fluid_sample_t fluid_sample_t;
typedef struct _fluid_mod_t fluid_mod_t;
typedef struct _fluid_audio_driver_t fluid_audio_driver_t;
typedef struct _fluid_player_t fluid_player_t;
typedef struct _fluid_midi_event_t fluid_midi_event_t;
typedef struct _fluid_midi_driver_t fluid_midi_driver_t;
typedef struct _fluid_midi_router_t fluid_midi_router_t;
typedef struct _fluid_midi_router_rule_t fluid_midi_router_rule_t;
typedef struct _fluid_hashtable_t fluid_cmd_handler_t;
typedef struct _fluid_shell_t fluid_shell_t;
typedef struct _fluid_server_t fluid_server_t;
typedef struct _fluid_event_t fluid_event_t;
typedef struct _fluid_sequencer_t fluid_sequencer_t;
typedef struct _fluid_ramsfont_t fluid_ramsfont_t;
typedef struct _fluid_rampreset_t fluid_rampreset_t;

typedef int fluid_istream_t;
typedef int fluid_ostream_t;


#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_TYPES_H */
