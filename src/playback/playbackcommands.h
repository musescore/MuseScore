/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "rcommand/commandtypes.h"

namespace mu::playback {
inline static const muse::rcommand::Command PLAY_TOGGLE_COMMAND("command://playback/play-toggle");
inline static const muse::rcommand::Command PLAY_COMMAND("command://playback/play");
inline static const muse::rcommand::Command PLAY_SELECTION_COMMAND("command://playback/play-selection");
inline static const muse::rcommand::Command PAUSE_COMMAND("command://playback/pause");
inline static const muse::rcommand::Command PAUSE_AND_SELECT_COMMAND("command://playback/pause-and-select");
inline static const muse::rcommand::Command STOP_COMMAND("command://playback/stop");
inline static const muse::rcommand::Command REWIND_COMMAND("command://playback/rewind");
inline static const muse::rcommand::Command LOOP_TOGGLE_COMMAND("command://playback/loop-toggle");
inline static const muse::rcommand::Command LOOP_IN_COMMAND("command://playback/loop-in");
inline static const muse::rcommand::Command LOOP_OUT_COMMAND("command://playback/loop-out");
inline static const muse::rcommand::Command METRONOME_TOGGLE_COMMAND("command://playback/metronome-toggle");
inline static const muse::rcommand::Command OPEN_PLAYBACK_SETUP_COMMAND("command://playback/open-playback-setup");
inline static const muse::rcommand::Command MIDI_TOGGLE_COMMAND("command://playback/midi-toggle");
inline static const muse::rcommand::Command MIDI_INPUT_WRITTEN_PITCH_COMMAND("command://playback/midi-input-written-pitch");
inline static const muse::rcommand::Command MIDI_INPUT_SOUNDING_PITCH_COMMAND("command://playback/midi-input-sounding-pitch");
inline static const muse::rcommand::Command REPEATS_TOGGLE_COMMAND("command://playback/repeats-toggle");
inline static const muse::rcommand::Command CHORDSYMBOLS_TOGGLE_COMMAND("command://playback/chordsymbols-toggle");
inline static const muse::rcommand::Command HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND("command://playback/hear-playback-when-editing-toggle");
inline static const muse::rcommand::Command PAN_TOGGLE_COMMAND("command://playback/pan-toggle");
inline static const muse::rcommand::Command COUNTIN_TOGGLE_COMMAND("command://playback/countin-toggle");
inline static const muse::rcommand::Command CLEAR_ONLINESOUNDS_CACHE_COMMAND("command://playback/clear-onlinesounds-cache");
inline static const muse::rcommand::Command PROCESS_ONLINESOUNDS_COMMAND("command://playback/process-onlinesounds");
inline static const muse::rcommand::Command RELOAD_PLAYBACK_CACHE_COMMAND("command://playback/reload-playback-cache");

inline static const muse::rcommand::Command MIDI_RECORD_TOGGLE_COMMAND("command://playback/midi-record-toggle");
inline static const muse::rcommand::Command MIDI_RECORD_REQUANTIZE_COMMAND("command://playback/midi-record-requantize");
inline static const muse::rcommand::Command MIDI_RECORD_QUANTIZATION_4_COMMAND("command://playback/midi-record-quantization-4");
inline static const muse::rcommand::Command MIDI_RECORD_QUANTIZATION_8_COMMAND("command://playback/midi-record-quantization-8");
inline static const muse::rcommand::Command MIDI_RECORD_QUANTIZATION_16_COMMAND("command://playback/midi-record-quantization-16");
inline static const muse::rcommand::Command MIDI_RECORD_QUANTIZATION_32_COMMAND("command://playback/midi-record-quantization-32");
inline static const muse::rcommand::Command MIDI_RECORD_TUPLETS_TOGGLE_COMMAND("command://playback/midi-record-tuplets-toggle");
inline static const muse::rcommand::Command MIDI_RECORD_LEGATO_TOGGLE_COMMAND("command://playback/midi-record-legato-toggle");
inline static const muse::rcommand::Command MIDI_RECORD_COUNTIN_0_COMMAND("command://playback/midi-record-countin-0");
inline static const muse::rcommand::Command MIDI_RECORD_COUNTIN_1_COMMAND("command://playback/midi-record-countin-1");
inline static const muse::rcommand::Command MIDI_RECORD_COUNTIN_2_COMMAND("command://playback/midi-record-countin-2");
inline static const muse::rcommand::Command MIDI_RECORD_ONE_STAFF_COMMAND("command://playback/midi-record-one-staff");
inline static const muse::rcommand::Command MIDI_RECORD_TWO_STAVES_COMMAND("command://playback/midi-record-two-staves");
inline static const muse::rcommand::Command MIDI_RECORD_SPLIT_POINT_COMMAND("command://playback/midi-record-split-point");
inline static const muse::rcommand::Command MIDI_RECORD_LATENCY_COMMAND("command://playback/midi-record-latency");

inline static const muse::rcommand::Command TOGGLE_MIXER_SECTION_COMMAND("command://playback/toggle-mixer-section"); // with params
inline static const muse::rcommand::Command TOGGLE_AUX_SEND_COMMAND("command://playback/toggle-aux-send");
inline static const muse::rcommand::Command TOGGLE_AUX_CHANNEL_COMMAND("command://playback/toggle-aux-channel");
}
