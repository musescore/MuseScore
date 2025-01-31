/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MUSE_MUSESAMPLER_APITYPES_H
#define MUSE_MUSESAMPLER_APITYPES_H

#include <stdint.h>
#include <memory>

#include "log.h"

typedef void* MuseSamplerLib;

typedef enum ms_Result
{
    ms_Result_OK = 0,
    ms_Result_Error = -1,
    ms_Result_TimeoutErr = -2
} ms_Result;

// Functions to return the major + minor version of the core code. Note that this is independent of the UI version.
typedef const char*(* ms_get_version_string)();
typedef int (* ms_get_version_major)();
typedef int (* ms_get_version_minor)();
typedef int (* ms_get_version_revision)();
typedef int (* ms_get_version_build_number)();

/*\\\ TYPES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

typedef void* ms_MuseSampler;
typedef void* ms_PresetList;
typedef int ms_PresetChange; // added in v0.6
typedef void* ms_InstrumentList;
typedef void* ms_InstrumentInfo;
typedef void* ms_Track;

typedef struct ms_OutputBuffer
{
    float** _channels;
    int _num_data_pts;
    int _num_channels; // For sanity checking! Shouldn't change from init!
} ms_OutputBuffer;

// Added in v0.4
typedef struct ms_DynamicsEvent_2
{
    long long _location_us;
    double _value; // 0.0 - 1.0
} ms_DynamicsEvent_2;

// Added in v0.4
typedef struct ms_PedalEvent_2
{
    long long _location_us;
    double _value; // 0.0 - 1.0
} ms_PedalEvent_2;

enum ms_NoteArticulation : uint64_t
{
    ms_NoteArticulation_None = 0,
    ms_NoteArticulation_Staccato = 1LL << 0,
    ms_NoteArticulation_Staccatissimo = 1LL << 1,
    ms_NoteArticulation_Accent = 1LL << 2,
    ms_NoteArticulation_Tenuto = 1LL << 3,
    ms_NoteArticulation_Marcato = 1LL << 4,
    ms_NoteArticulation_Harmonics = 1LL << 5,
    ms_NoteArticulation_Mute = 1LL << 6,
    ms_NoteArticulation_Trill = 1LL << 7,
    ms_NoteArticulation_MordentSemi = 1LL << 8,
    ms_NoteArticulation_MordentWhole = 1LL << 9,
    ms_NoteArticulation_MordentInvertedSemi = 1LL << 10,
    ms_NoteArticulation_MordentInvertedWhole = 1LL << 11,
    ms_NoteArticulation_TurnSemiWhole = 1LL << 12,
    ms_NoteArticulation_TurnSemiSemi = 1LL << 13,
    ms_NoteArticulation_TurnWholeWhole = 1LL << 14,
    ms_NoteArticulation_TurnWholeSemi = 1LL << 15,
    ms_NoteArticulation_TurnInvertedSemiWhole = 1LL << 16,
    ms_NoteArticulation_TurnInvertedSemiSemi = 1LL << 17,
    ms_NoteArticulation_TurnInvertedWholeWhole = 1LL << 18,
    ms_NoteArticulation_TurnInvertedWholeSemi = 1LL << 19,
    ms_NoteArticulation_ArpeggioUp = 1LL << 20,
    ms_NoteArticulation_ArpeggioDown = 1LL << 21,
    ms_NoteArticulation_Tremolo1 = 1LL << 22,
    ms_NoteArticulation_Tremolo2 = 1LL << 23,
    ms_NoteArticulation_Tremolo3 = 1LL << 24,
    ms_NoteArticulation_Scoop = 1LL << 25,
    ms_NoteArticulation_Plop = 1LL << 26,
    ms_NoteArticulation_Doit = 1LL << 27,
    ms_NoteArticulation_Fall = 1LL << 28,
    ms_NoteArticulation_Appoggiatura = 1LL << 29, // Duration is ignored
    ms_NoteArticulation_Acciaccatura = 1LL << 30, // Duration is ignored

    ms_NoteArticulation_Open = 1LL << 31,

    ms_NoteArticulation_Portamento = 1LL << 36,
    ms_NoteArticulation_Pizzicato = 1LL << 37,
    ms_NoteArticulation_Glissando = 1LL << 39,
    ms_NoteArticulation_Pedal = 1LL << 40,
    ms_NoteArticulation_Slur = 1LL << 41,
    ms_NoteArticulation_SnapPizzicato = 1LL << 42,
    ms_NoteArticulation_ColLegno = 1LL << 43,
    ms_NoteArticulation_SulTasto = 1LL << 44,
    ms_NoteArticulation_SulPonticello = 1LL << 45,
    ms_NoteArticulation_HammerOn = 1LL << 46,
    ms_NoteArticulation_PullOff = 1LL << 47,
    ms_NoteArticulation_PalmMute = 1LL << 48,
    ms_NoteArticulation_PinchHarmonic = 1LL << 49,
    ms_NoteArticulation_BuzzTremolo = 1LL << 50,

    ms_NoteArticulation_LaissezVibrer = 1LL << 59,
};

enum ms_NoteHead : int16_t
{
    ms_NoteHead_Normal,
    ms_NoteHead_XNote,
    ms_NoteHead_LargeX,
    ms_NoteHead_OrnateXNote,
    ms_NoteHead_CircleXNote,
    ms_NoteHead_CircleDot,
    ms_NoteHead_Ghost,
    ms_NoteHead_Circle,
    ms_NoteHead_Diamond,
    ms_NoteHead_Triangle,
    ms_NoteHead_TriangleUp,
    ms_NoteHead_TriangleDown,
    ms_NoteHead_TriangleRight,
    ms_NoteHead_TriangleRoundDown,
    ms_NoteHead_FlatTop,
    ms_NoteHead_Square,
    ms_NoteHead_Slash,
    ms_NoteHead_SlashRightFilled,
    ms_NoteHead_SlashLeftFilled,
    ms_NoteHead_Plus,
};

// Added in v0.6
typedef struct ms_NoteEvent_4
{
    int _voice; // 0-3
    long long _location_us;
    long long _duration_us;
    int _pitch;      // MIDI pitch
    double _tempo;   // BPM
    int _offset_cents; // pitch offset in cents: -50 would be a quarter tone flat
    ms_NoteArticulation _articulation;
    ms_NoteHead _notehead;
} ms_NoteEvent_4;

// Added in v0.6
typedef struct ms_AuditionStartNoteEvent_3
{
    int _pitch; // MIDI pitch
    int _offset_cents;
    ms_NoteArticulation _articulation;
    ms_NoteHead _notehead;
    double _dynamics;
    const char* _active_presets; // presets that are selected for this note. list is separated by "|"
    const char* _active_text_articulation; // text articulation that is active for this note. Can be empty
} ms_AuditionStartNoteEvent_3;

// Added in v0.100 -> changed from ms_AuditionStartNoteEvent3 by adding "_active_syllable"
typedef struct ms_AuditionStartNoteEvent_4
{
    int _pitch;      // MIDI pitch
    int _offset_cents; // pitch offsets in cents ()
    ms_NoteArticulation _articulation;
    ms_NoteHead _notehead;
    double _dynamics;
    const char* _active_presets; // presets that are selected for this note. list is separated by "|". If "" then the
                                 // first available preset is used.
    const char* _active_text_articulation; // text articulation that is active for this note. Can be empty
    const char* _active_syllable;        // currently active lyrics syllable. Can be empty
    bool _articulation_text_starts_at_note;
    bool _syllable_starts_at_note;
} ms_AuditionStartNoteEvent_4;

typedef struct ms_AuditionStopNoteEvent
{
    int _pitch; // MIDI pitch
} ms_AuditionStopNoteEvent;

// Added in v0.3
typedef struct ms_LivePlayStartNoteEvent_2
{
    int _pitch; // MIDI pitch
    int _offset_cents;
    double _dynamics;
} ms_LivePlayStartNoteEvent_2;

typedef struct ms_LivePlayStopNoteEvent
{
    int _pitch; // MIDI pitch
} ms_LivePlayStopNoteEvent;

// Added in v0.5
enum ms_PitchBendType
{
    PitchBend_Linear,
    PitchBend_Bezier,
};

typedef struct ms_PitchBendInfo
{
    int64_t event_id;
    long long _start_us; // offset from the start of the note, not absolute time
    long long _duration_us;
    int _offset_cents;
    ms_PitchBendType _type;
} ms_PitchBendInfo;

typedef struct ms_VibratoInfo
{
    int64_t event_id;
    long long _start_us;
    long long _duration_us;
    int _depth_cents;
} ms_VibratoInfo;

// Added in v0.6
typedef struct ms_TextArticulationEvent
{
    long long _start_us;
    const char* _articulation; // articulation name
} ms_TextArticulationEvent;

// Added in v0.100
typedef struct ms_SyllableEvent
{
    const char* _text;
    long long _position_us;
} ms_SyllableEvent;

typedef ms_Result (* ms_init)();
typedef ms_Result (* ms_disable_reverb)();
typedef int (* ms_contains_instrument)(const char* mpe_id, const char* musicxml_id);
typedef int (* ms_get_matching_instrument_id)(const char* pack, const char* name);
typedef ms_InstrumentList (* ms_get_instrument_list)();
typedef ms_InstrumentList (* ms_get_matching_instrument_list)(const char* mpe_id, const char* musicxml_id);
typedef ms_InstrumentInfo (* ms_InstrumentList_get_next)(ms_InstrumentList instrument_list);

typedef int (* ms_Instrument_get_id)(ms_InstrumentInfo instrument);
typedef const char*(* ms_Instrument_get_name)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_category)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_package)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_musicxml_sound)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_mpe_sound)(ms_InstrumentInfo);
typedef float (* ms_Instrument_get_reverb_level)(ms_InstrumentInfo);

typedef ms_PresetList (* ms_Instrument_get_preset_list)(ms_InstrumentInfo);
typedef const char*(* ms_PresetList_get_next)(ms_PresetList);

typedef ms_MuseSampler (* ms_MuseSampler_create)();
typedef void (* ms_MuseSampler_destroy)(ms_MuseSampler);
typedef ms_Result (* ms_MuseSampler_init)(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count);

// Added in 0.100
typedef ms_Result (* ms_MuseSampler_init_2)(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count);

typedef ms_Result (* ms_MuseSampler_set_demo_score)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_clear_score)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_finalize_score)(ms_MuseSampler ms);
typedef ms_Track (* ms_MuseSampler_add_track)(ms_MuseSampler ms, int instrument_id);
typedef ms_Result (* ms_MuseSampler_finalize_track)(ms_MuseSampler ms, ms_Track track);
typedef ms_Result (* ms_MuseSampler_clear_track)(ms_MuseSampler ms, ms_Track track);

// Added in 0.6
typedef ms_Result (* ms_MuseSampler_add_track_note_event_5)(ms_MuseSampler ms, ms_Track track, ms_NoteEvent_4 evt, long long& event_id);
// Added in 0.4
typedef ms_Result (* ms_MuseSampler_add_track_dynamics_event_2)(ms_MuseSampler ms, ms_Track track, ms_DynamicsEvent_2 evt);
// Added in 0.4
typedef ms_Result (* ms_MuseSampler_add_track_pedal_event_2)(ms_MuseSampler ms, ms_Track track, ms_PedalEvent_2 evt);

typedef int (* ms_MuseSampler_is_ranged_articulation)(ms_NoteArticulation);
typedef ms_Result (* ms_MuseSampler_add_track_event_range_start)(ms_MuseSampler, ms_Track, int voice, ms_NoteArticulation);
typedef ms_Result (* ms_MuseSampler_add_track_event_range_end)(ms_MuseSampler, ms_Track, int voice, ms_NoteArticulation);

// Added in 0.5
typedef ms_Result (* ms_MuseSampler_add_pitch_bend)(ms_MuseSampler ms, ms_Track track, ms_PitchBendInfo info);
typedef ms_Result (* ms_MuseSampler_add_vibrato)(ms_MuseSampler ms, ms_Track track, ms_VibratoInfo info);

// Added in 0.3
typedef ms_Result (* ms_MuseSampler_stop_audition_note)(ms_MuseSampler ms, ms_Track track, ms_AuditionStopNoteEvent evt);

// Added in 0.6
typedef ms_Result (* ms_MuseSampler_start_audition_note_3)(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_3 evt);

typedef ms_Result (* ms_MuseSampler_start_liveplay_mode)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_stop_liveplay_mode)(ms_MuseSampler ms);

// Added in 0.3
typedef ms_Result (* ms_MuseSampler_start_liveplay_note_2)(ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2 evt);
typedef ms_Result (* ms_MuseSampler_stop_liveplay_note)(ms_MuseSampler ms, ms_Track track, ms_LivePlayStopNoteEvent evt);

typedef ms_Result (* ms_MuseSampler_start_offline_mode)(ms_MuseSampler ms, double sample_rate);
typedef ms_Result (* ms_MuseSampler_stop_offline_mode)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_process_offline)(ms_MuseSampler ms, ms_OutputBuffer buffer);

typedef ms_Result (* ms_MuseSampler_process)(ms_MuseSampler, ms_OutputBuffer, long long samples);
typedef void (* ms_MuseSampler_set_position)(ms_MuseSampler, long long samples);
typedef void (* ms_MuseSampler_set_playing)(ms_MuseSampler, int playing);

typedef ms_Result (* ms_MuseSampler_all_notes_off)(ms_MuseSampler);

// Added in 0.6
typedef const char*(* ms_Instrument_get_pack_name)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_vendor_name)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_info_json)(ms_InstrumentInfo);

typedef ms_PresetChange (* ms_MuseSampler_create_preset_change)(ms_MuseSampler ms, ms_Track track, long long location_us);
typedef ms_Result (* ms_MuseSampler_add_preset)(ms_MuseSampler ms, ms_Track track, ms_PresetChange preset_change, const char* preset_name);

typedef const char*(* ms_get_text_articulations)(int instrument_id, const char* preset_name);
typedef ms_Result (* ms_MuseSampler_add_track_text_articulation_event)(ms_MuseSampler ms, ms_Track track, ms_TextArticulationEvent evt);

typedef const char*(* ms_get_drum_mapping)(int instrument_id);
// ------------------------------------------------------------

// Added in 0.100
typedef ms_Result (* ms_reload_all_instruments)(); // Useful for sound developers
typedef ms_Result (* ms_MuseSampler_start_audition_note_4)(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_4 evt);
typedef ms_Result (* ms_MuseSampler_add_track_syllable_event)(ms_MuseSampler ms, ms_Track track, ms_SyllableEvent evt);

namespace muse::musesampler {
using track_idx_t = size_t;
using TrackList = std::vector<ms_Track>;

struct InstrumentInfo {
    int instrumentId = -1;
    ms_InstrumentInfo msInstrument = nullptr;

    bool isValid() const
    {
        return instrumentId != -1 && msInstrument != nullptr;
    }
};

struct AuditionStartNoteEvent {
    ms_AuditionStartNoteEvent_4 msEvent;
    ms_Track msTrack = nullptr;
};

struct AuditionStopNoteEvent {
    ms_AuditionStopNoteEvent msEvent;
    ms_Track msTrack = nullptr;
};

using NoteEvent = ms_NoteEvent_4;
}

#endif // MUSE_MUSESAMPLER_APITYPES_H
