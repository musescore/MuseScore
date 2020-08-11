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

#ifndef _FLUIDSYNTH_MIDI_H
#define _FLUIDSYNTH_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file midi.h
 * @brief Functions for MIDI events, drivers and MIDI file playback.
 */

FLUIDSYNTH_API fluid_midi_event_t *new_fluid_midi_event(void);
FLUIDSYNTH_API void delete_fluid_midi_event(fluid_midi_event_t *event);

FLUIDSYNTH_API int fluid_midi_event_set_type(fluid_midi_event_t *evt, int type);
FLUIDSYNTH_API int fluid_midi_event_get_type(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_channel(fluid_midi_event_t *evt, int chan);
FLUIDSYNTH_API int fluid_midi_event_get_channel(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_get_key(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_key(fluid_midi_event_t *evt, int key);
FLUIDSYNTH_API int fluid_midi_event_get_velocity(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_velocity(fluid_midi_event_t *evt, int vel);
FLUIDSYNTH_API int fluid_midi_event_get_control(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_control(fluid_midi_event_t *evt, int ctrl);
FLUIDSYNTH_API int fluid_midi_event_get_value(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_value(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_program(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_program(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_pitch(fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_pitch(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_set_sysex(fluid_midi_event_t *evt, void *data,
        int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_set_text(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_get_text(fluid_midi_event_t *evt,
        void **data, int *size);
FLUIDSYNTH_API int fluid_midi_event_set_lyrics(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_get_lyrics(fluid_midi_event_t *evt,
        void **data, int *size);

/**
 * MIDI router rule type.
 * @since 1.1.0
 */
typedef enum
{
    FLUID_MIDI_ROUTER_RULE_NOTE,                  /**< MIDI note rule */
    FLUID_MIDI_ROUTER_RULE_CC,                    /**< MIDI controller rule */
    FLUID_MIDI_ROUTER_RULE_PROG_CHANGE,           /**< MIDI program change rule */
    FLUID_MIDI_ROUTER_RULE_PITCH_BEND,            /**< MIDI pitch bend rule */
    FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE,      /**< MIDI channel pressure rule */
    FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE,          /**< MIDI key pressure rule */
#ifndef __DOXYGEN__
    FLUID_MIDI_ROUTER_RULE_COUNT                  /**< @internal Total count of rule types @warning This symbol is not part of the public API and ABI stability guarantee and may change at any time!*/
#endif
} fluid_midi_router_rule_type;

/**
 * Generic callback function for MIDI events.
 * @param data User defined data pointer
 * @param event The MIDI event
 * @return Should return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * Will be used between
 * - MIDI driver and MIDI router
 * - MIDI router and synth
 * to communicate events.
 * In the not-so-far future...
 */
typedef int (*handle_midi_event_func_t)(void *data, fluid_midi_event_t *event);

FLUIDSYNTH_API fluid_midi_router_t *new_fluid_midi_router(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
FLUIDSYNTH_API void delete_fluid_midi_router(fluid_midi_router_t *handler);
FLUIDSYNTH_API int fluid_midi_router_set_default_rules(fluid_midi_router_t *router);
FLUIDSYNTH_API int fluid_midi_router_clear_rules(fluid_midi_router_t *router);
FLUIDSYNTH_API int fluid_midi_router_add_rule(fluid_midi_router_t *router,
        fluid_midi_router_rule_t *rule, int type);
FLUIDSYNTH_API fluid_midi_router_rule_t *new_fluid_midi_router_rule(void);
FLUIDSYNTH_API void delete_fluid_midi_router_rule(fluid_midi_router_rule_t *rule);
FLUIDSYNTH_API void fluid_midi_router_rule_set_chan(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API void fluid_midi_router_rule_set_param1(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API void fluid_midi_router_rule_set_param2(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API int fluid_midi_router_handle_midi_event(void *data, fluid_midi_event_t *event);
FLUIDSYNTH_API int fluid_midi_dump_prerouter(void *data, fluid_midi_event_t *event);
FLUIDSYNTH_API int fluid_midi_dump_postrouter(void *data, fluid_midi_event_t *event);


FLUIDSYNTH_API
fluid_midi_driver_t *new_fluid_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);

FLUIDSYNTH_API void delete_fluid_midi_driver(fluid_midi_driver_t *driver);


/**
 * MIDI player status enum.
 * @since 1.1.0
 */
enum fluid_player_status
{
    FLUID_PLAYER_READY,           /**< Player is ready */
    FLUID_PLAYER_PLAYING,         /**< Player is currently playing */
    FLUID_PLAYER_DONE             /**< Player is finished playing */
};

FLUIDSYNTH_API fluid_player_t *new_fluid_player(fluid_synth_t *synth);
FLUIDSYNTH_API void delete_fluid_player(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_add(fluid_player_t *player, const char *midifile);
FLUIDSYNTH_API int fluid_player_add_mem(fluid_player_t *player, const void *buffer, size_t len);
FLUIDSYNTH_API int fluid_player_play(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_stop(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_join(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_set_loop(fluid_player_t *player, int loop);
FLUIDSYNTH_API int fluid_player_set_midi_tempo(fluid_player_t *player, int tempo);
FLUIDSYNTH_API int fluid_player_set_bpm(fluid_player_t *player, int bpm);
FLUIDSYNTH_API int fluid_player_set_playback_callback(fluid_player_t *player, handle_midi_event_func_t handler, void *handler_data);

FLUIDSYNTH_API int fluid_player_get_status(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_current_tick(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_total_ticks(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_bpm(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_midi_tempo(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_seek(fluid_player_t *player, int ticks);

///

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_MIDI_H */
