/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <memory>
#include <cstring>

#include "dlib.h"
#include "apitypes.h"

#include "types/version.h"

namespace muse::musesampler {
struct MuseSamplerLibHandler
{
public:
    MuseSamplerLibHandler() = default;

    bool loadLib(const io::path_t& path)
    {
        m_lib = muse::loadLib(path);
        return m_lib != nullptr;
    }

    bool loadApi(const Version& minSupportedVersion, bool useLegacyAudition = false)
    {
        IF_ASSERT_FAILED(m_lib) {
            return false;
        }

        auto getVersionMajor = (ms_get_version_major)libFunc("ms_get_version_major");
        auto getVersionMinor = (ms_get_version_minor)libFunc("ms_get_version_minor");
        auto getVersionRevision = (ms_get_version_revision)libFunc("ms_get_version_revision");
        auto getBuildNumber = (ms_get_version_build_number)libFunc("ms_get_version_build_number");

        // Invalid...
        if (!getVersionMajor || !getVersionMinor || !getVersionRevision) {
            return false;
        }

        m_version = Version(getVersionMajor(), getVersionMinor(), getVersionRevision());

        if (getBuildNumber) {
            m_buildNumber = getBuildNumber();
        }

        if (m_version < minSupportedVersion) {
            LOGE() << "MuseSampler " << m_version.toString() << " is not supported (too old -- update MuseSampler); ignoring";
            return false;
        }

        // Major versions have incompatible changes; we can only support interfaces we know about
        constexpr int maximumMajorVersion = 0;

        if (m_version.major() > maximumMajorVersion) {
            LOGE() << "MuseSampler " << m_version.toString() << " is not supported (too new -- update MuseScore Studio); ignoring";
            return false;
        }

        // Check versions here; define optional functions below
        // bool at_least_v_0_106 = (m_version.major() == 0 && m_version.minor() >= 106) || m_version.major() > 0;

        initLib = (ms_init_2)libFunc("ms_init_2");
        deinitLib = (ms_deinit)libFunc("ms_deinit");

        getInstrumentList = (ms_get_instrument_list)libFunc("ms_get_instrument_list");
        getMatchingInstrumentList = (ms_get_matching_instrument_list)libFunc("ms_get_matching_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)libFunc("ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)libFunc("ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)libFunc("ms_Instrument_get_name");
        getInstrumentCategory = (ms_Instrument_get_category)libFunc("ms_Instrument_get_category");

        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)libFunc("ms_Instrument_get_musicxml_sound");
        getMpeSoundId = (ms_Instrument_get_mpe_sound)libFunc("ms_Instrument_get_mpe_sound");

        getPresetList = (ms_Instrument_get_preset_list)libFunc("ms_Instrument_get_preset_list");
        getNextPreset = (ms_PresetList_get_next)libFunc("ms_PresetList_get_next");

        create = (ms_MuseSampler_create)libFunc("ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)libFunc("ms_MuseSampler_destroy");

        if (useLegacyAudition) {
            LOGI() << "Use legacy audition (ms_MuseSampler_init)";

            auto initSamplerFunc = (ms_MuseSampler_init)libFunc("ms_MuseSampler_init");
            initSampler = [initSamplerFunc](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerFunc(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        } else {
            auto initSamplerFunc = (ms_MuseSampler_init_2)libFunc("ms_MuseSampler_init_2");
            initSampler = [initSamplerFunc](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerFunc(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        }

        addTrack = (ms_MuseSampler_add_track)libFunc("ms_MuseSampler_add_track");
        finalizeTrack = (ms_MuseSampler_finalize_track)libFunc("ms_MuseSampler_finalize_track");
        clearTrack = (ms_MuseSampler_clear_track)libFunc("ms_MuseSampler_clear_track");

        disableReverb = (ms_disable_reverb)libFunc("ms_disable_reverb");
        getReverbLevel = (ms_Instrument_get_reverb_level)libFunc("ms_Instrument_get_reverb_level");

        addDynamicsEvent = (ms_MuseSampler_add_track_dynamics_event_2)libFunc(
            "ms_MuseSampler_add_track_dynamics_event_2");
        addPedalEvent = (ms_MuseSampler_add_track_pedal_event_2)libFunc("ms_MuseSampler_add_track_pedal_event_2");

        auto addNoteEventFunc = (ms_MuseSampler_add_track_note_event_6)libFunc("ms_MuseSampler_add_track_note_event_6");
        addNoteEvent = [addNoteEventFunc](ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id) {
            return addNoteEventFunc(ms, track, ev, event_id) == ms_Result_OK;
        };

        auto startAuditionNoteFunc = (ms_MuseSampler_start_audition_note_5)libFunc(
            "ms_MuseSampler_start_audition_note_5");
        startAuditionNote = [startAuditionNoteFunc](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_5 ev) {
            return startAuditionNoteFunc(ms, track, ev) == ms_Result_OK;
        };

        isRangedArticulation = (ms_MuseSampler_is_ranged_articulation)libFunc("ms_MuseSampler_is_ranged_articulation");
        addTrackEventRangeStart
            = (ms_MuseSampler_add_track_event_range_start)libFunc("ms_MuseSampler_add_track_event_range_start");
        addTrackEventRangeEnd
            = (ms_MuseSampler_add_track_event_range_end)libFunc("ms_MuseSampler_add_track_event_range_end");

        stopAuditionNote = (ms_MuseSampler_stop_audition_note)libFunc("ms_MuseSampler_stop_audition_note");

        addSyllableEvent = (ms_MuseSampler_add_track_syllable_event_2)libFunc("ms_MuseSampler_add_track_syllable_event_2");

        getInstrumentVendorName = (ms_Instrument_get_vendor_name)libFunc("ms_Instrument_get_vendor_name");
        getInstrumentPackName = (ms_Instrument_get_pack_name)libFunc("ms_Instrument_get_pack_name");
        getInstrumentInfoJson = (ms_Instrument_get_info_json)libFunc("ms_Instrument_get_info_json");
        createPresetChange = (ms_MuseSampler_create_preset_change)libFunc("ms_MuseSampler_create_preset_change");
        addPreset = (ms_MuseSampler_add_preset)libFunc("ms_MuseSampler_add_preset");
        getTextArticulations = (ms_get_text_articulations)libFunc("ms_get_text_articulations");
        addTextArticulationEvent = (ms_MuseSampler_add_track_text_articulation_event)
                                   libFunc("ms_MuseSampler_add_track_text_articulation_event");
        getDrumMapping = (ms_get_drum_mapping)libFunc("ms_get_drum_mapping");

        addPitchBend = (ms_MuseSampler_add_pitch_bend)libFunc("ms_MuseSampler_add_pitch_bend");
        addVibrato = (ms_MuseSampler_add_vibrato)libFunc("ms_MuseSampler_add_vibrato");

        startOfflineMode = (ms_MuseSampler_start_offline_mode)libFunc("ms_MuseSampler_start_offline_mode");
        stopOfflineMode = (ms_MuseSampler_stop_offline_mode)libFunc("ms_MuseSampler_stop_offline_mode");
        processOffline = (ms_MuseSampler_process_offline)libFunc("ms_MuseSampler_process_offline");

        setPosition = (ms_MuseSampler_set_position)libFunc("ms_MuseSampler_set_position");
        setPlaying = (ms_MuseSampler_set_playing)libFunc("ms_MuseSampler_set_playing");
        process = (ms_MuseSampler_process)libFunc("ms_MuseSampler_process");
        allNotesOff = (ms_MuseSampler_all_notes_off)libFunc("ms_MuseSampler_all_notes_off");

        reloadAllInstruments = (ms_reload_all_instruments)libFunc("ms_reload_all_instruments");
        readyToPlay = (ms_MuseSampler_ready_to_play)libFunc("ms_MuseSampler_ready_to_play");

        setLoggingCallback = (ms_set_logging_callback)libFunc("ms_set_logging_callback");
        isOnlineInstrument = (ms_Instrument_is_online)libFunc("ms_Instrument_is_online");
        setScoreId = (ms_MuseSampler_set_score_id)libFunc("ms_MuseSampler_set_score_id");
        setAutoRenderInterval = (ms_MuseSampler_set_auto_render_interval)libFunc(
            "ms_MuseSampler_set_auto_render_interval");
        triggerRender = (ms_MuseSampler_trigger_render)libFunc("ms_MuseSampler_trigger_render");
        clearOnlineCache = (ms_MuseSampler_clear_online_cache)libFunc("ms_MuseSampler_clear_online_cache");
        addAuditionCCEvent = (ms_MuseSampler_add_audition_cc_event)libFunc("ms_MuseSampler_add_audition_cc_event");

        setRenderingStateChangedCallback = (ms_MuseSampler_set_rendering_state_changed_callback_2)libFunc(
            "ms_MuseSampler_set_rendering_state_changed_callback_2");
        getNextRenderProgressInfo = (ms_RenderProgressInfo2_get_next)libFunc("ms_RenderProgressInfo2_get_next");
        setLazyRender = (ms_MuseSampler_set_lazy_render)libFunc("ms_MuseSampler_set_lazy_render");

        if (!isValid()) {
            printApiStatus();
            return false;
        }

        return true;
    }

    bool init()
    {
        if (!initLib) {
            LOGE() << "Could not find ms_init";
            return false;
        }

        if (initLib() != ms_Result_OK) {
            LOGE() << "Could not init lib";
            return false;
        }

        if (disableReverb) {
            if (disableReverb() != ms_Result_OK) {
                LOGW() << "Could not disable reverb";
            }
        }

        return true;
    }

    void deinit()
    {
        IF_ASSERT_FAILED(m_lib) {
            return;
        }

        if (deinitLib) {
            if (deinitLib() != ms_Result_OK) {
                LOGE() << "Could not deinit lib";
            }
        }

        muse::closeLib(m_lib);
        m_lib = nullptr;
    }

    bool isValid() const
    {
        return m_lib
               && initLib
               && deinitLib
               && disableReverb
               && getInstrumentList
               && getMatchingInstrumentList
               && getNextInstrument
               && getInstrumentId
               && getInstrumentName
               && getInstrumentCategory
               && getInstrumentPackName
               && getInstrumentVendorName
               && getInstrumentInfoJson
               && getMusicXmlSoundId
               && getMpeSoundId
               && getPresetList
               && getNextPreset
               && getReverbLevel
               && getTextArticulations
               && getDrumMapping
               && create
               && destroy
               && initSampler
               && addTrack
               && finalizeTrack
               && clearTrack
               && addDynamicsEvent
               && addPedalEvent
               && addNoteEvent
               && addSyllableEvent
               && addPitchBend
               && addVibrato
               && addTextArticulationEvent
               && addPreset
               && createPresetChange
               && setPosition
               && setPlaying
               && isRangedArticulation
               && addTrackEventRangeStart
               && addTrackEventRangeEnd
               && startAuditionNote
               && stopAuditionNote
               && startOfflineMode
               && stopOfflineMode
               && processOffline
               && process
               && allNotesOff
               && readyToPlay
               && setLoggingCallback
               && setRenderingStateChangedCallback
               && isOnlineInstrument
               && setScoreId
               && getNextRenderProgressInfo
               && setAutoRenderInterval
               && triggerRender
               && clearOnlineCache
               && addAuditionCCEvent
               && setLazyRender;
    }

    const Version& version() const
    {
        return m_version;
    }

    int buildNumber() const
    {
        return m_buildNumber;
    }

    void printApiStatus() const
    {
        LOGI() << "MuseSampler API status:"
               << "\n ms_init_2 - " << reinterpret_cast<uint64_t>(initLib)
               << "\n ms_deinit - " << reinterpret_cast<uint64_t>(deinitLib)
               << "\n ms_disable_reverb - " << reinterpret_cast<uint64_t>(disableReverb)
               << "\n ms_get_instrument_list - " << reinterpret_cast<uint64_t>(getInstrumentList)
               << "\n ms_get_matching_instrument_list - " << reinterpret_cast<uint64_t>(getMatchingInstrumentList)
               << "\n ms_get_drum_mapping - " << reinterpret_cast<uint64_t>(getDrumMapping)
               << "\n ms_InstrumentList_get_next - " << reinterpret_cast<uint64_t>(getNextInstrument)
               << "\n ms_Instrument_get_info_json - " << reinterpret_cast<uint64_t>(getInstrumentInfoJson)
               << "\n ms_Instrument_get_id - " << reinterpret_cast<uint64_t>(getInstrumentId)
               << "\n ms_Instrument_get_name - " << reinterpret_cast<uint64_t>(getInstrumentName)
               << "\n ms_Instrument_get_vendor_name - " << reinterpret_cast<uint64_t>(getInstrumentVendorName)
               << "\n ms_Instrument_get_category - " << reinterpret_cast<uint64_t>(getInstrumentCategory)
               << "\n ms_Instrument_get_pack_name - " << reinterpret_cast<uint64_t>(getInstrumentPackName)
               << "\n ms_Instrument_get_musicxml_sound - " << reinterpret_cast<uint64_t>(getMusicXmlSoundId)
               << "\n ms_Instrument_get_mpe_sound - " << reinterpret_cast<uint64_t>(getMpeSoundId)
               << "\n ms_Instrument_get_reverb_level - " << reinterpret_cast<uint64_t>(getReverbLevel)
               << "\n ms_Instrument_get_preset_list - " << reinterpret_cast<uint64_t>(getPresetList)
               << "\n ms_PresetList_get_next - " << reinterpret_cast<uint64_t>(getNextPreset)
               << "\n ms_MuseSampler_create_preset_change - " << reinterpret_cast<uint64_t>(createPresetChange)
               << "\n ms_MuseSampler_add_preset - " << reinterpret_cast<uint64_t>(addPreset)
               << "\n ms_get_text_articulations - " << reinterpret_cast<uint64_t>(getTextArticulations)
               << "\n ms_MuseSampler_add_track_text_articulation_event - " << reinterpret_cast<uint64_t>(addTextArticulationEvent)
               << "\n ms_MuseSampler_create - " << reinterpret_cast<uint64_t>(create)
               << "\n ms_MuseSampler_destroy - " << reinterpret_cast<uint64_t>(destroy)
               << "\n ms_MuseSampler_add_track - " << reinterpret_cast<uint64_t>(addTrack)
               << "\n ms_MuseSampler_clear_track - " << reinterpret_cast<uint64_t>(clearTrack)
               << "\n ms_MuseSampler_finalize_track - " << reinterpret_cast<uint64_t>(finalizeTrack)
               << "\n ms_MuseSampler_add_track_dynamics_event_2 - " << reinterpret_cast<uint64_t>(addDynamicsEvent)
               << "\n ms_MuseSampler_add_track_pedal_event_2 - " << reinterpret_cast<uint64_t>(addPedalEvent)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_stop_audition_note - " << reinterpret_cast<uint64_t>(stopAuditionNote)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_add_vibrato - " << reinterpret_cast<uint64_t>(addVibrato)
               << "\n ms_MuseSampler_is_ranged_articulation - " << reinterpret_cast<uint64_t>(isRangedArticulation)
               << "\n ms_MuseSampler_add_track_event_range_start - " << reinterpret_cast<uint64_t>(addTrackEventRangeStart)
               << "\n ms_MuseSampler_add_track_event_range_end - " << reinterpret_cast<uint64_t>(addTrackEventRangeEnd)
               << "\n ms_MuseSampler_add_audition_cc_event - " << reinterpret_cast<uint64_t>(addAuditionCCEvent)
               << "\n ms_MuseSampler_add_track_syllable_event_2 - " << reinterpret_cast<uint64_t>(addSyllableEvent)
               << "\n ms_MuseSampler_start_offline_mode - " << reinterpret_cast<uint64_t>(startOfflineMode)
               << "\n ms_MuseSampler_stop_offline_mode - " << reinterpret_cast<uint64_t>(stopOfflineMode)
               << "\n ms_MuseSampler_process_offline - " << reinterpret_cast<uint64_t>(processOffline)
               << "\n ms_MuseSampler_set_position - " << reinterpret_cast<uint64_t>(setPosition)
               << "\n ms_MuseSampler_set_playing - " << reinterpret_cast<uint64_t>(setPlaying)
               << "\n ms_MuseSampler_process - " << reinterpret_cast<uint64_t>(process)
               << "\n ms_MuseSampler_all_notes_off - " << reinterpret_cast<uint64_t>(allNotesOff)
               << "\n ms_MuseSampler_ready_to_play - " << reinterpret_cast<uint64_t>(readyToPlay)
               << "\n ms_Instrument_is_online - " << reinterpret_cast<uint64_t>(isOnlineInstrument)
               << "\n ms_MuseSampler_trigger_render - " << reinterpret_cast<uint64_t>(triggerRender)
               << "\n ms_reload_all_instruments - " << reinterpret_cast<uint64_t>(reloadAllInstruments)
               << "\n ms_set_logging_callback - " << reinterpret_cast<uint64_t>(setLoggingCallback)
               << "\n ms_MuseSampler_set_rendering_state_changed_callback_2 - " <<
            reinterpret_cast<uint64_t>(setRenderingStateChangedCallback)
               << "\n ms_MuseSampler_set_score_id - " << reinterpret_cast<uint64_t>(setScoreId)
               << "\n ms_MuseSampler_clear_online_cache - " << reinterpret_cast<uint64_t>(clearOnlineCache)
               << "\n ms_RenderProgressInfo2_get_next - " << reinterpret_cast<uint64_t>(getNextRenderProgressInfo)
               << "\n ms_MuseSampler_set_auto_render_interval - " << reinterpret_cast<uint64_t>(setAutoRenderInterval)
               << "\n ms_MuseSampler_set_lazy_render - " << reinterpret_cast<uint64_t>(setLazyRender);
    }

    ms_get_instrument_list getInstrumentList = nullptr;
    ms_get_matching_instrument_list getMatchingInstrumentList = nullptr;
    ms_InstrumentList_get_next getNextInstrument = nullptr;
    ms_Instrument_get_id getInstrumentId = nullptr;
    ms_Instrument_get_name getInstrumentName = nullptr;
    ms_Instrument_get_category getInstrumentCategory = nullptr;
    ms_Instrument_get_pack_name getInstrumentPackName = nullptr;
    ms_Instrument_get_info_json getInstrumentInfoJson = nullptr;
    ms_Instrument_get_vendor_name getInstrumentVendorName = nullptr;
    ms_Instrument_get_musicxml_sound getMusicXmlSoundId = nullptr;
    ms_Instrument_get_mpe_sound getMpeSoundId = nullptr;
    ms_Instrument_get_reverb_level getReverbLevel = nullptr;

    ms_Instrument_get_preset_list getPresetList = nullptr;
    ms_PresetList_get_next getNextPreset = nullptr;
    ms_MuseSampler_create_preset_change createPresetChange = nullptr;
    ms_MuseSampler_add_preset addPreset = nullptr;
    ms_get_text_articulations getTextArticulations = nullptr;
    ms_MuseSampler_add_track_text_articulation_event addTextArticulationEvent = nullptr;
    ms_get_drum_mapping getDrumMapping = nullptr;

    ms_MuseSampler_create create = nullptr;
    ms_MuseSampler_destroy destroy = nullptr;

    std::function<bool(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count)> initSampler = nullptr;

    ms_MuseSampler_add_track addTrack = nullptr;
    ms_MuseSampler_finalize_track finalizeTrack = nullptr;
    ms_MuseSampler_clear_track clearTrack = nullptr;

    ms_MuseSampler_add_track_dynamics_event_2 addDynamicsEvent = nullptr;
    ms_MuseSampler_add_track_pedal_event_2 addPedalEvent = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id)> addNoteEvent = nullptr;

    ms_MuseSampler_is_ranged_articulation isRangedArticulation = nullptr;
    ms_MuseSampler_add_track_event_range_start addTrackEventRangeStart = nullptr;
    ms_MuseSampler_add_track_event_range_end addTrackEventRangeEnd = nullptr;

    ms_MuseSampler_add_pitch_bend addPitchBend = nullptr;
    ms_MuseSampler_add_vibrato addVibrato = nullptr;

    ms_MuseSampler_add_track_syllable_event_2 addSyllableEvent = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_5)> startAuditionNote = nullptr;
    ms_MuseSampler_stop_audition_note stopAuditionNote = nullptr;
    ms_MuseSampler_add_audition_cc_event addAuditionCCEvent = nullptr;

    ms_MuseSampler_start_offline_mode startOfflineMode = nullptr;
    ms_MuseSampler_stop_offline_mode stopOfflineMode = nullptr;
    ms_MuseSampler_process_offline processOffline = nullptr;

    ms_MuseSampler_ready_to_play readyToPlay = nullptr;

    ms_MuseSampler_set_position setPosition = nullptr;
    ms_MuseSampler_set_playing setPlaying = nullptr;
    ms_MuseSampler_process process = nullptr;
    ms_MuseSampler_all_notes_off allNotesOff = nullptr;

    ms_reload_all_instruments reloadAllInstruments = nullptr;

    ms_set_logging_callback setLoggingCallback = nullptr;
    ms_MuseSampler_set_rendering_state_changed_callback_2 setRenderingStateChangedCallback = nullptr;

    ms_MuseSampler_set_score_id setScoreId = nullptr;
    ms_Instrument_is_online isOnlineInstrument = nullptr;
    ms_RenderProgressInfo2_get_next getNextRenderProgressInfo = nullptr;
    ms_MuseSampler_set_auto_render_interval setAutoRenderInterval = nullptr;
    ms_MuseSampler_trigger_render triggerRender = nullptr;
    ms_MuseSampler_clear_online_cache clearOnlineCache = nullptr;
    ms_MuseSampler_set_lazy_render setLazyRender = nullptr;

private:
    void* libFunc(const char* funcName)
    {
        return muse::getLibFunc(m_lib, funcName);
    }

    ms_init_2 initLib = nullptr;
    ms_deinit deinitLib = nullptr;
    ms_disable_reverb disableReverb = nullptr;

    MuseSamplerLib m_lib = nullptr;
    Version m_version;
    int m_buildNumber = -1;
};

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}
