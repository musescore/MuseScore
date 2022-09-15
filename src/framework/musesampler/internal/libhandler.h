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

#ifndef MU_MUSESAMPLER_LIBHANDLER_H
#define MU_MUSESAMPLER_LIBHANDLER_H

#include <memory>
#include <cstring>

#ifdef USE_MUSESAMPLER_SRC
#include "Api/musesampler_api.h"
#else
#include "dlib.h"

#include "apitypes.h"
#endif

namespace mu::musesampler {
#ifdef USE_MUSESAMPLER_SRC
struct MuseSamplerLibHandler
{
    ms_Result initLib() { return ms_init(); }
    bool containsInstrument(const char* mpe_id, const char* musicxml)
    {
        return ms_contains_instrument(mpe_id, musicxml) == 1;
    }

    int getMatchingInstrumentId(const char* pack, const char* name) { reutrn ms_get_matching_instrument_id(pack, name); }
    ms_InstrumentList getInstrumentList() { return ms_get_instrument_list(); }
    ms_InstrumentList getMatchingInstrumentList(const char* mpe_id, const char* musicxml)
    {
        return ms_get_matching_instrument_list(mpe_id, musicxml);
    }

    ms_InstrumentInfo getNextInstrument(ms_InstrumentList instrument_list) { return ms_InstrumentList_get_next(instrument_list); }
    int getInstrumentId(ms_InstrumentInfo instrument) { return ms_Instrument_get_id(instrument); }
    const char* getInstrumentName(ms_InstrumentInfo instrument) { return ms_Instrument_get_name(instrument); }
    const char* getInstrumentCategory(ms_InstrumentInfo instrument) { return ms_Instrument_get_category(instrument); }
    const char* getInstrumentPackage(ms_InstrumentInfo instrument) { return ms_Instrument_get_package(instrument); }
    const char* getMusicXmlSoundId(ms_InstrumentInfo instrument) { return ms_Instrument_get_musicxml_sound(instrument); }
    const char* getMpeSoundId(ms_InstrumentInfo instrument) { return ms_Instrument_get_mpe_sound(instrument); }

    ms_PresetList getPresetList(ms_InstrumentInfo instrument_info) { return ms_Instrument_get_preset_list(instrument_info); }
    const char* getNextPreset(ms_PresetList preset_list) { return ms_PresetList_get_next(preset_list); }

    ms_MuseSampler create() { return ms_MuseSampler_create(); }
    void destroy(ms_MuseSampler sampler) { ms_MuseSampler_destroy(sampler); }
    ms_Result initSampler(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count)
    {
        return ms_MuseSampler_init(ms, sample_rate, block_size, channel_count);
    }

    ms_Result clearScore(ms_MuseSampler ms) { return ms_MuseSampler_clear_score(ms); }
    ms_Track addTrack(ms_MuseSampler ms, int instrument_id) { return ms_MuseSampler_add_track(ms, instrument_id); }
    ms_Result finalizeTrack(ms_MuseSampler ms, ms_Track track) { return ms_MuseSampler_finalize_track(ms, track); }
    ms_Result clearTrack(ms_MuseSampler ms, ms_Track track) { return ms_MuseSampler_clear_track(ms, track); }

    ms_Result addDynamicsEvent(ms_MuseSampler ms, ms_Track track, ms_DynamicsEvent evt)
    {
        return ms_MuseSampler_add_track_dynamics_event(ms, track, evt);
    }

    ms_Result addNoteEvent(ms_MuseSampler ms, ms_Track track, ms_Event evt) { return ms_MuseSampler_add_track_note_event(ms, track, evt); }

    bool isRangedArticulation(ms_NoteArticulation art) { return ms_MuseSampler_is_ranged_articulation(art) == 1; }
    ms_Result addTrackEventRangeStart(ms_MuseSampler ms, ms_Track track, int voice, ms_NoteArticulation art)
    {
        return ms_MuseSampler_add_track_event_range_start(ms, track, voice, art);
    }

    ms_Result addTrackEventRangeEnd(ms_MuseSampler ms, ms_Track track, int voice, ms_NoteArticulation art)
    {
        return ms_MuseSampler_add_track_event_range_end(ms, track, voice, art);
    }

    ms_Result startAuditionMode(ms_MuseSampler ms) { return ms_MuseSampler_start_audition_mode(ms); }
    ms_Result stopAuditionMode(ms_MuseSampler ms) { return ms_MuseSampler_stop_audition_mode(ms); }
    ms_Result startAuditionNote(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent evt)
    {
        return ms_MuseSampler_start_audition_note(ms, track, evt);
    }

    ms_Result stopAuditionNote(ms_MuseSampler ms, ms_Track track, ms_AuditionStopNoteEvent evt)
    {
        return ms_MuseSampler_stop_audition_note(ms, track, evt);
    }

    ms_Result startLivePlayMode(ms_MuseSampler ms) { return ms_MuseSampler_start_liveplay_mode(ms); }
    ms_Result stopLivePlayMode(ms_MuseSampler ms) { return ms_MuseSampler_stop_liveplay_mode(ms); }
    ms_Result startLivePlayNote(ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent evt)
    {
        return ms_MuseSampler_start_liveplay_note(ms, track, evt);
    }

    ms_Result stopLivePlayNote(ms_MuseSampler ms, ms_Track track, ms_LivePlayStopNoteEvent evt)
    {
        return ms_MuseSampler_stop_liveplay_note(ms, track, evt);
    }

    ms_Result startOfflineMode(ms_MuseSampler ms, double sample_rate)
    {
        return ms_MuseSampler_start_offline_mode(ms, sample_rate);
    }

    ms_Result stopOfflineMode(ms_MuseSampler ms) { return ms_MuseSampler_stop_offline_mode(ms); }
    ms_Result processOffline(ms_MuseSampler ms, ms_OutputBuffer buff)
    {
        return ms_MuseSampler_process_offline(ms, buff);
    }

    void setPosition(ms_MuseSampler ms, long long samples) { return ms_MuseSampler_set_position(ms, samples); }
    void setPlaying(ms_MuseSampler ms, bool playing) { return ms_MuseSampler_set_playing(ms, playing ? 1 : 0); }
    ms_Result process(ms_MuseSampler ms, ms_OutputBuffer buff, long long samples) { return ms_MuseSampler_process(ms, buff, samples); }

    MuseSamplerLibHandler(const char* /*path*/)
    {
        initLib();
    }

    bool isValid() const
    {
        return true;
    }
};
#else
struct MuseSamplerLibHandler
{
    ms_init initLib = nullptr;
    ms_contains_instrument containsInstrument = nullptr;

    ms_get_matching_instrument_id getMatchingInstrumentId = nullptr;
    ms_get_instrument_list getInstrumentList = nullptr;
    ms_get_matching_instrument_list getMatchingInstrumentList = nullptr;
    ms_InstrumentList_get_next getNextInstrument = nullptr;
    ms_Instrument_get_id getInstrumentId = nullptr;
    ms_Instrument_get_name getInstrumentName = nullptr;
    ms_Instrument_get_category getInstrumentCategory = nullptr;
    ms_Instrument_get_package getInstrumentPackage = nullptr;
    ms_Instrument_get_musicxml_sound getMusicXmlSoundId = nullptr;
    ms_Instrument_get_mpe_sound getMpeSoundId = nullptr;

    ms_Instrument_get_preset_list getPresetList = nullptr;
    ms_PresetList_get_next getNextPreset = nullptr;

    ms_MuseSampler_create create = nullptr;
    ms_MuseSampler_destroy destroy = nullptr;
    ms_MuseSampler_init initSampler = nullptr;

    ms_MuseSampler_clear_score clearScore = nullptr;
    ms_MuseSampler_add_track addTrack = nullptr;
    ms_MuseSampler_finalize_track finalizeTrack = nullptr;
    ms_MuseSampler_clear_track clearTrack = nullptr;

    ms_MuseSampler_add_track_dynamics_event addDynamicsEvent = nullptr;
    ms_MuseSampler_add_track_note_event addNoteEvent = nullptr;
    ms_MuseSampler_is_ranged_articulation isRangedArticulation = nullptr;
    ms_MuseSampler_add_track_event_range_start addTrackEventRangeStart = nullptr;
    ms_MuseSampler_add_track_event_range_end addTrackEventRangeEnd = nullptr;

    ms_MuseSampler_start_audition_note startAuditionNote = nullptr;
    ms_MuseSampler_stop_audition_note stopAuditionNote = nullptr;

    ms_MuseSampler_start_liveplay_mode startLivePlayMode = nullptr;
    ms_MuseSampler_stop_liveplay_mode stopLivePlayMode = nullptr;
    ms_MuseSampler_start_liveplay_note startLivePlayNote = nullptr;
    ms_MuseSampler_stop_liveplay_note stopLivePlayNote = nullptr;

    ms_MuseSampler_start_offline_mode startOfflineMode = nullptr;
    ms_MuseSampler_stop_offline_mode stopOfflineMode = nullptr;
    ms_MuseSampler_process_offline processOffline = nullptr;

    ms_MuseSampler_set_position setPosition = nullptr;
    ms_MuseSampler_set_playing setPlaying = nullptr;
    ms_MuseSampler_process process = nullptr;
    ms_MuseSampler_all_notes_off allNotesOff = nullptr;

    MuseSamplerLibHandler(const io::path_t& path)
    {
        m_lib = loadLib(path);

        if (!m_lib) {
            LOGE() << "Unable to open MuseSampler library, path: " << path;
            return;
        }

        initLib = (ms_init)getLibFunc(m_lib, "ms_init");
        containsInstrument = (ms_contains_instrument)getLibFunc(m_lib, "ms_contains_instrument");
        getMatchingInstrumentId = (ms_get_matching_instrument_id)getLibFunc(m_lib, "ms_get_matching_instrument_id");
        getInstrumentList = (ms_get_instrument_list)getLibFunc(m_lib, "ms_get_instrument_list");
        getMatchingInstrumentList = (ms_get_matching_instrument_list)getLibFunc(m_lib, "ms_get_matching_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)getLibFunc(m_lib, "ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)getLibFunc(m_lib, "ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)getLibFunc(m_lib, "ms_Instrument_get_name");
        getInstrumentCategory = (ms_Instrument_get_category)getLibFunc(m_lib, "ms_Instrument_get_category");
        getInstrumentPackage = (ms_Instrument_get_package)getLibFunc(m_lib, "ms_Instrument_get_package");
        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)getLibFunc(m_lib, "ms_Instrument_get_musicxml_sound");
        getMpeSoundId = (ms_Instrument_get_mpe_sound)getLibFunc(m_lib, "ms_Instrument_get_mpe_sound");

        getPresetList = (ms_Instrument_get_preset_list)getLibFunc(m_lib, "ms_Instrument_get_preset_list");
        getNextPreset = (ms_PresetList_get_next)getLibFunc(m_lib, "ms_PresetList_get_next");

        create = (ms_MuseSampler_create)getLibFunc(m_lib, "ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)getLibFunc(m_lib, "ms_MuseSampler_destroy");
        initSampler = (ms_MuseSampler_init)getLibFunc(m_lib, "ms_MuseSampler_init");

        clearScore = (ms_MuseSampler_clear_score)getLibFunc(m_lib, "ms_MuseSampler_clear_score");
        addTrack = (ms_MuseSampler_add_track)getLibFunc(m_lib, "ms_MuseSampler_add_track");
        finalizeTrack = (ms_MuseSampler_finalize_track)getLibFunc(m_lib, "ms_MuseSampler_finalize_track");
        clearTrack = (ms_MuseSampler_clear_track)getLibFunc(m_lib, "ms_MuseSampler_clear_track");

        addDynamicsEvent = (ms_MuseSampler_add_track_dynamics_event)getLibFunc(m_lib, "ms_MuseSampler_add_track_dynamics_event");
        addNoteEvent = (ms_MuseSampler_add_track_note_event)getLibFunc(m_lib, "ms_MuseSampler_add_track_note_event");

        isRangedArticulation = (ms_MuseSampler_is_ranged_articulation)getLibFunc(m_lib, "ms_MuseSampler_is_ranged_articulation");
        addTrackEventRangeStart
            = (ms_MuseSampler_add_track_event_range_start)getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_start");
        addTrackEventRangeEnd = (ms_MuseSampler_add_track_event_range_end)getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_end");

        startAuditionNote = (ms_MuseSampler_start_audition_note)getLibFunc(m_lib, "ms_MuseSampler_start_audition_note");
        stopAuditionNote = (ms_MuseSampler_stop_audition_note)getLibFunc(m_lib, "ms_MuseSampler_stop_audition_note");

        startLivePlayMode = (ms_MuseSampler_start_liveplay_mode)getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_mode");
        stopLivePlayMode = (ms_MuseSampler_stop_liveplay_mode)getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_mode");
        startLivePlayNote = (ms_MuseSampler_start_liveplay_note)getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_note");
        stopLivePlayNote = (ms_MuseSampler_stop_liveplay_note)getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_note");

        startOfflineMode = (ms_MuseSampler_start_offline_mode)getLibFunc(m_lib, "ms_MuseSampler_start_offline_mode");
        stopOfflineMode = (ms_MuseSampler_stop_offline_mode)getLibFunc(m_lib, "ms_MuseSampler_stop_offline_mode");
        processOffline = (ms_MuseSampler_process_offline)getLibFunc(m_lib, "ms_MuseSampler_process_offline");

        setPosition = (ms_MuseSampler_set_position)getLibFunc(m_lib, "ms_MuseSampler_set_position");
        setPlaying = (ms_MuseSampler_set_playing)getLibFunc(m_lib, "ms_MuseSampler_set_playing");
        process = (ms_MuseSampler_process)getLibFunc(m_lib, "ms_MuseSampler_process");
        allNotesOff = (ms_MuseSampler_all_notes_off)getLibFunc(m_lib, "ms_MuseSampler_all_notes_off");

        if (initLib) {
            initLib();
        }
    }

    ~MuseSamplerLibHandler()
    {
        if (!m_lib) {
            return;
        }

        closeLib(m_lib);
    }

    bool isValid() const
    {
        return m_lib
               && initLib
               && containsInstrument
               && getMatchingInstrumentId
               && getInstrumentList
               && getMatchingInstrumentList
               && getNextInstrument
               && getInstrumentId
               && getInstrumentName
               && getInstrumentCategory
               && getInstrumentPackage
               && getMusicXmlSoundId
               && getMpeSoundId
               && getPresetList
               && getNextPreset
               && create
               && destroy
               && initSampler
               && clearScore
               && addTrack
               && finalizeTrack
               && clearTrack
               && addDynamicsEvent
               && addNoteEvent
               && setPosition
               && setPlaying
               && isRangedArticulation
               && addTrackEventRangeStart
               && addTrackEventRangeEnd
               && startAuditionNote
               && stopAuditionNote
               && startLivePlayMode
               && stopLivePlayMode
               && startLivePlayNote
               && stopLivePlayNote
               && startOfflineMode
               && stopOfflineMode
               && processOffline
               && process
               && allNotesOff;
    }

private:
    void printApiStatus() const
    {
        LOGI() << "MuseSampler API status:"
               << "\n ms_contains_instrument -" << reinterpret_cast<uint64_t>(containsInstrument)
               << "\n ms_get_matching_instrument_id -" << reinterpret_cast<uint64_t>(getMatchingInstrumentId)
               << "\n ms_get_instrument_list -" << reinterpret_cast<uint64_t>(getInstrumentList)
               << "\n ms_get_matching_instrument_list -" << reinterpret_cast<uint64_t>(getMatchingInstrumentList)
               << "\n ms_InstrumentList_get_next - " << reinterpret_cast<uint64_t>(getNextInstrument)
               << "\n ms_Instrument_get_id - " << reinterpret_cast<uint64_t>(getInstrumentId)
               << "\n ms_Instrument_get_name - " << reinterpret_cast<uint64_t>(getInstrumentName)
               << "\n ms_Instrument_get_category - " << reinterpret_cast<uint64_t>(getInstrumentCategory)
               << "\n ms_Instrument_get_package - " << reinterpret_cast<uint64_t>(getInstrumentPackage)
               << "\n ms_Instrument_get_musicxml_sound - " << reinterpret_cast<uint64_t>(getMusicXmlSoundId)
               << "\n ms_Instrument_get_mpe_sound - " << reinterpret_cast<uint64_t>(getMpeSoundId)
               << "\n ms_Instrument_get_preset_list - " << reinterpret_cast<uint64_t>(getPresetList)
               << "\n ms_PresetList_get_next - " << reinterpret_cast<uint64_t>(getNextPreset)
               << "\n ms_MuseSampler_create - " << reinterpret_cast<uint64_t>(create)
               << "\n ms_MuseSampler_destroy - " << reinterpret_cast<uint64_t>(destroy)
               << "\n ms_MuseSampler_init - " << reinterpret_cast<uint64_t>(initSampler)
               << "\n ms_MuseSampler_clear_score - " << reinterpret_cast<uint64_t>(clearScore)
               << "\n ms_MuseSampler_add_track - " << reinterpret_cast<uint64_t>(addTrack)
               << "\n ms_MuseSampler_finalize_track - " << reinterpret_cast<uint64_t>(finalizeTrack)
               << "\n ms_MuseSampler_add_track_dynamics_event - " << reinterpret_cast<uint64_t>(addDynamicsEvent)
               << "\n ms_MuseSampler_add_track_note_event - " << reinterpret_cast<uint64_t>(addNoteEvent)
               << "\n ms_MuseSampler_start_audition_note - " << reinterpret_cast<uint64_t>(startAuditionNote)
               << "\n ms_MuseSampler_stop_audition_note - " << reinterpret_cast<uint64_t>(stopAuditionNote)
               << "\n ms_MuseSampler_start_liveplay_mode - " << reinterpret_cast<uint64_t>(startLivePlayMode)
               << "\n ms_MuseSampler_stop_liveplay_mode - " << reinterpret_cast<uint64_t>(stopLivePlayMode)
               << "\n ms_MuseSampler_start_liveplay_note - " << reinterpret_cast<uint64_t>(startLivePlayNote)
               << "\n ms_MuseSampler_stop_liveplay_note - " << reinterpret_cast<uint64_t>(stopLivePlayNote)
               << "\n ms_MuseSampler_start_offline_mode - " << reinterpret_cast<uint64_t>(startOfflineMode)
               << "\n ms_MuseSampler_stop_offline_mode - " << reinterpret_cast<uint64_t>(stopOfflineMode)
               << "\n ms_MuseSampler_process_offline - " << reinterpret_cast<uint64_t>(processOffline)
               << "\n ms_MuseSampler_set_position - " << reinterpret_cast<uint64_t>(setPosition)
               << "\n ms_MuseSampler_set_playing - " << reinterpret_cast<uint64_t>(setPlaying)
               << "\n ms_MuseSampler_process - " << reinterpret_cast<uint64_t>(process)
               << "\n ms_MuseSampler_all_notes_off - " << reinterpret_cast<uint64_t>(allNotesOff);
    }

    MuseSamplerLib m_lib = nullptr;
};
#endif

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}

#endif // MU_MUSESAMPLER_LIBHANDLER_H
