/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

    ~MuseSamplerLibHandler()
    {
        if (m_lib) {
            muse::closeLib(m_lib);
        }
    }

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

        auto getVersionMajor = (ms_get_version_major)muse::getLibFunc(m_lib, "ms_get_version_major");
        auto getVersionMinor = (ms_get_version_minor)muse::getLibFunc(m_lib, "ms_get_version_minor");
        auto getVersionRevision = (ms_get_version_revision)muse::getLibFunc(m_lib, "ms_get_version_revision");
        auto getBuildNumber = (ms_get_version_build_number)muse::getLibFunc(m_lib, "ms_get_version_build_number");

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
        bool at_least_v_0_102 = (m_version.major() == 0 && m_version.minor() >= 102) || m_version.major() > 0;

        initLib = (ms_init)muse::getLibFunc(m_lib, "ms_init");

        getInstrumentList = (ms_get_instrument_list)muse::getLibFunc(m_lib, "ms_get_instrument_list");
        getMatchingInstrumentList = (ms_get_matching_instrument_list)muse::getLibFunc(m_lib, "ms_get_matching_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)muse::getLibFunc(m_lib, "ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)muse::getLibFunc(m_lib, "ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)muse::getLibFunc(m_lib, "ms_Instrument_get_name");
        getInstrumentCategory = (ms_Instrument_get_category)muse::getLibFunc(m_lib, "ms_Instrument_get_category");

        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)muse::getLibFunc(m_lib, "ms_Instrument_get_musicxml_sound");
        getMpeSoundId = (ms_Instrument_get_mpe_sound)muse::getLibFunc(m_lib, "ms_Instrument_get_mpe_sound");

        getPresetList = (ms_Instrument_get_preset_list)muse::getLibFunc(m_lib, "ms_Instrument_get_preset_list");
        getNextPreset = (ms_PresetList_get_next)muse::getLibFunc(m_lib, "ms_PresetList_get_next");

        create = (ms_MuseSampler_create)muse::getLibFunc(m_lib, "ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)muse::getLibFunc(m_lib, "ms_MuseSampler_destroy");

        if (useLegacyAudition) {
            LOGI() << "Use legacy audition (ms_MuseSampler_init)";

            initSamplerInternal = (ms_MuseSampler_init)muse::getLibFunc(m_lib, "ms_MuseSampler_init");
            initSampler = [this](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerInternal(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        } else {
            initSamplerInternal2 = (ms_MuseSampler_init_2)muse::getLibFunc(m_lib, "ms_MuseSampler_init_2");
            initSampler = [this](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerInternal2(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        }

        addTrack = (ms_MuseSampler_add_track)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track");
        finalizeTrack = (ms_MuseSampler_finalize_track)muse::getLibFunc(m_lib, "ms_MuseSampler_finalize_track");
        clearTrack = (ms_MuseSampler_clear_track)muse::getLibFunc(m_lib, "ms_MuseSampler_clear_track");

        disableReverb = (ms_disable_reverb)muse::getLibFunc(m_lib, "ms_disable_reverb");
        getReverbLevel = (ms_Instrument_get_reverb_level)muse::getLibFunc(m_lib, "ms_Instrument_get_reverb_level");

        addDynamicsEvent = (ms_MuseSampler_add_track_dynamics_event_2)muse::getLibFunc(m_lib,
                                                                                       "ms_MuseSampler_add_track_dynamics_event_2");
        addPedalEvent = (ms_MuseSampler_add_track_pedal_event_2)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_pedal_event_2");

        if (at_least_v_0_102) {
            addNoteEventInternal6 = (ms_MuseSampler_add_track_note_event_6)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_note_event_6");
            addNoteEvent = [this](ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id) {
                return addNoteEventInternal6(ms, track, ev, event_id) == ms_Result_OK;
            };
        } else {
            addNoteEventInternal5 = (ms_MuseSampler_add_track_note_event_5)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_note_event_5");
            addNoteEvent = [this](ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id) {
                ms_NoteEvent_4 ev4{ ev._voice, ev._location_us, ev._duration_us, ev._pitch, ev._tempo, ev._offset_cents,
                                    ev._articulation, ev._notehead };

                return addNoteEventInternal5(ms, track, ev4, event_id) == ms_Result_OK;
            };
        }

        if (at_least_v_0_102) {
            startAuditionNoteInternal5 = (ms_MuseSampler_start_audition_note_5)muse::getLibFunc(m_lib,
                                                                                                "ms_MuseSampler_start_audition_note_5");
            startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_5 ev) {
                return startAuditionNoteInternal5(ms, track, ev) == ms_Result_OK;
            };
        } else {
            startAuditionNoteInternal4 = (ms_MuseSampler_start_audition_note_4)muse::getLibFunc(m_lib,
                                                                                                "ms_MuseSampler_start_audition_note_4");
            startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_5 ev) {
                ms_AuditionStartNoteEvent_4 ev4{ ev._pitch, ev._offset_cents, ev._articulation, ev._notehead, ev._dynamics,
                                                 ev._active_presets, ev._active_text_articulation, ev._active_syllable,
                                                 ev._articulation_text_starts_at_note, ev._syllable_starts_at_note };

                return startAuditionNoteInternal4(ms, track, ev4) == ms_Result_OK;
            };
        }

        isRangedArticulation = (ms_MuseSampler_is_ranged_articulation)muse::getLibFunc(m_lib, "ms_MuseSampler_is_ranged_articulation");
        addTrackEventRangeStart
            = (ms_MuseSampler_add_track_event_range_start)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_start");
        addTrackEventRangeEnd
            = (ms_MuseSampler_add_track_event_range_end)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_end");

        stopAuditionNote = (ms_MuseSampler_stop_audition_note)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_audition_note");

        if (at_least_v_0_102) {
            addSyllableEventInternal2 = (ms_MuseSampler_add_track_syllable_event_2)muse::getLibFunc(m_lib,
                                                                                                    "ms_MuseSampler_add_track_syllable_event_2");

            addSyllableEvent = [this](ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev) {
                return addSyllableEventInternal2(ms, track, ev) == ms_Result_OK;
            };
        } else {
            addSyllableEventInternal = (ms_MuseSampler_add_track_syllable_event)muse::getLibFunc(m_lib,
                                                                                                 "ms_MuseSampler_add_track_syllable_event");

            addSyllableEvent = [this](ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev) {
                ms_SyllableEvent ev1 { ev._text, ev._position_us };
                return addSyllableEventInternal(ms, track, ev1) == ms_Result_OK;
            };
        }

        getInstrumentVendorName = (ms_Instrument_get_vendor_name)muse::getLibFunc(m_lib, "ms_Instrument_get_vendor_name");
        getInstrumentPackName = (ms_Instrument_get_pack_name)muse::getLibFunc(m_lib, "ms_Instrument_get_pack_name");
        getInstrumentInfoJson = (ms_Instrument_get_info_json)muse::getLibFunc(m_lib, "ms_Instrument_get_info_json");
        createPresetChange = (ms_MuseSampler_create_preset_change)muse::getLibFunc(m_lib, "ms_MuseSampler_create_preset_change");
        addPreset = (ms_MuseSampler_add_preset)muse::getLibFunc(m_lib, "ms_MuseSampler_add_preset");
        getTextArticulations = (ms_get_text_articulations)muse::getLibFunc(m_lib, "ms_get_text_articulations");
        addTextArticulationEvent = (ms_MuseSampler_add_track_text_articulation_event)
                                   muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_text_articulation_event");
        getDrumMapping = (ms_get_drum_mapping)muse::getLibFunc(m_lib, "ms_get_drum_mapping");

        addPitchBend = (ms_MuseSampler_add_pitch_bend)muse::getLibFunc(m_lib, "ms_MuseSampler_add_pitch_bend");
        addVibrato = (ms_MuseSampler_add_vibrato)muse::getLibFunc(m_lib, "ms_MuseSampler_add_vibrato");

        startOfflineMode = (ms_MuseSampler_start_offline_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_start_offline_mode");
        stopOfflineMode = (ms_MuseSampler_stop_offline_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_offline_mode");
        processOffline = (ms_MuseSampler_process_offline)muse::getLibFunc(m_lib, "ms_MuseSampler_process_offline");

        setPosition = (ms_MuseSampler_set_position)muse::getLibFunc(m_lib, "ms_MuseSampler_set_position");
        setPlaying = (ms_MuseSampler_set_playing)muse::getLibFunc(m_lib, "ms_MuseSampler_set_playing");
        process = (ms_MuseSampler_process)muse::getLibFunc(m_lib, "ms_MuseSampler_process");
        allNotesOff = (ms_MuseSampler_all_notes_off)muse::getLibFunc(m_lib, "ms_MuseSampler_all_notes_off");

        reloadAllInstruments = (ms_reload_all_instruments)muse::getLibFunc(m_lib, "ms_reload_all_instruments");
        readyToPlay = (ms_MuseSampler_ready_to_play)muse::getLibFunc(m_lib, "ms_MuseSampler_ready_to_play");

        if (at_least_v_0_102) {
            setLoggingCallback = (ms_set_logging_callback)muse::getLibFunc(m_lib, "ms_set_logging_callback");
            isOnlineInstrument = (ms_Instrument_is_online)muse::getLibFunc(m_lib, "ms_Instrument_is_online");
            setScoreId = (ms_MuseSampler_set_score_id)muse::getLibFunc(m_lib, "ms_MuseSampler_set_score_id");
            getRenderInfo = (ms_MuseSampler_get_render_info)muse::getLibFunc(m_lib, "ms_MuseSampler_get_render_info");
            getNextRenderProgressInfo = (ms_RenderProgressInfo_get_next)muse::getLibFunc(m_lib, "ms_RenderProgressInfo_get_next");
            setAutoRenderInterval = (ms_MuseSampler_set_auto_render_interval)muse::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_set_auto_render_interval");
            triggerRender = (ms_MuseSampler_trigger_render)muse::getLibFunc(m_lib, "ms_MuseSampler_trigger_render");
            addAuditionCCEvent = (ms_MuseSampler_add_audition_cc_event)muse::getLibFunc(m_lib, "ms_MuseSampler_add_audition_cc_event");
        } else {
            setLoggingCallback = [](ms_logging_callback) {};
            isOnlineInstrument = [](ms_InstrumentInfo) { return false; };
            setScoreId = [](ms_MuseSampler, const char*) {};
            getRenderInfo = [](ms_MuseSampler, int*) { return ms_RenderingRangeList(); };
            getNextRenderProgressInfo = [](ms_RenderingRangeList) { return ms_RenderRangeInfo { 0, 0, ms_RenderingState_ErrorRendering }; };
            setAutoRenderInterval = [](ms_MuseSampler, double) {};
            triggerRender = [](ms_MuseSampler) {};
            addAuditionCCEvent = [](ms_MuseSampler, ms_Track, int, float) { return ms_Result_Error; };
        }

        return isValid();
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

    bool isValid() const
    {
        return m_lib
               && initLib
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
               && (addNoteEventInternal6 || addNoteEventInternal5)
               && (addSyllableEventInternal2 || addSyllableEventInternal)
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
               && (startAuditionNoteInternal5 || startAuditionNoteInternal4)
               && stopAuditionNote
               && startOfflineMode
               && stopOfflineMode
               && processOffline
               && process
               && allNotesOff
               && readyToPlay;
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
               << "\n ms_init -" << reinterpret_cast<uint64_t>(initLib)
               << "\n ms_disable_reverb - " << reinterpret_cast<uint64_t>(disableReverb)
               << "\n ms_get_instrument_list -" << reinterpret_cast<uint64_t>(getInstrumentList)
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
               << "\n ms_MuseSampler_init - " << reinterpret_cast<uint64_t>(initSamplerInternal)
               << "\n ms_MuseSampler_init_2 - " << reinterpret_cast<uint64_t>(initSamplerInternal2)
               << "\n ms_MuseSampler_add_track - " << reinterpret_cast<uint64_t>(addTrack)
               << "\n ms_MuseSampler_clear_track - " << reinterpret_cast<uint64_t>(clearTrack)
               << "\n ms_MuseSampler_finalize_track - " << reinterpret_cast<uint64_t>(finalizeTrack)
               << "\n ms_MuseSampler_add_track_dynamics_event_2 - " << reinterpret_cast<uint64_t>(addDynamicsEvent)
               << "\n ms_MuseSampler_add_track_pedal_event_2 - " << reinterpret_cast<uint64_t>(addPedalEvent)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_add_track_note_event_5 - " << reinterpret_cast<uint64_t>(addNoteEventInternal5)
               << "\n ms_MuseSampler_add_track_note_event_6 - " << reinterpret_cast<uint64_t>(addNoteEventInternal6)
               << "\n ms_MuseSampler_start_audition_note_4 - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal4)
               << "\n ms_MuseSampler_start_audition_note_5 - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal5)
               << "\n ms_MuseSampler_stop_audition_note - " << reinterpret_cast<uint64_t>(stopAuditionNote)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_add_vibrato - " << reinterpret_cast<uint64_t>(addVibrato)
               << "\n ms_MuseSampler_is_ranged_articulation - " << reinterpret_cast<uint64_t>(isRangedArticulation)
               << "\n ms_MuseSampler_add_track_event_range_start - " << reinterpret_cast<uint64_t>(addTrackEventRangeStart)
               << "\n ms_MuseSampler_add_track_event_range_end - " << reinterpret_cast<uint64_t>(addTrackEventRangeEnd)
               << "\n ms_MuseSampler_start_offline_mode - " << reinterpret_cast<uint64_t>(startOfflineMode)
               << "\n ms_MuseSampler_stop_offline_mode - " << reinterpret_cast<uint64_t>(stopOfflineMode)
               << "\n ms_MuseSampler_process_offline - " << reinterpret_cast<uint64_t>(processOffline)
               << "\n ms_MuseSampler_set_position - " << reinterpret_cast<uint64_t>(setPosition)
               << "\n ms_MuseSampler_set_playing - " << reinterpret_cast<uint64_t>(setPlaying)
               << "\n ms_MuseSampler_process - " << reinterpret_cast<uint64_t>(process)
               << "\n ms_MuseSampler_all_notes_off - " << reinterpret_cast<uint64_t>(allNotesOff)
               << "\n ms_MuseSampler_ready_to_play - " << reinterpret_cast<uint64_t>(readyToPlay)
               << "\n ms_Instrument_is_online - " << reinterpret_cast<uint64_t>(isOnlineInstrument)
               << "\n ms_MuseSampler_get_render_info - " << reinterpret_cast<uint64_t>(getRenderInfo)
               << "\n ms_RenderProgressInfo_get_next - " << reinterpret_cast<uint64_t>(getNextRenderProgressInfo)
               << "\n ms_MuseSampler_add_track_syllable_event - " << reinterpret_cast<uint64_t>(addSyllableEventInternal)
               << "\n ms_MuseSampler_add_track_syllable_event_2 - " << reinterpret_cast<uint64_t>(addSyllableEventInternal2)
               << "\n ms_reload_all_instruments - " << reinterpret_cast<uint64_t>(reloadAllInstruments)
               << "\n ms_set_logging_callback - " << reinterpret_cast<uint64_t>(setLoggingCallback);
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

    std::function<bool(ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev)> addSyllableEvent = nullptr;

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
    ms_MuseSampler_set_score_id setScoreId = nullptr;
    ms_Instrument_is_online isOnlineInstrument = nullptr;
    ms_MuseSampler_get_render_info getRenderInfo = nullptr;
    ms_RenderProgressInfo_get_next getNextRenderProgressInfo = nullptr;
    ms_MuseSampler_set_auto_render_interval setAutoRenderInterval = nullptr;
    ms_MuseSampler_trigger_render triggerRender = nullptr;

private:
    ms_init initLib = nullptr;
    ms_disable_reverb disableReverb = nullptr;
    ms_MuseSampler_add_track_note_event_5 addNoteEventInternal5 = nullptr;
    ms_MuseSampler_add_track_note_event_6 addNoteEventInternal6 = nullptr;
    ms_MuseSampler_start_audition_note_4 startAuditionNoteInternal4 = nullptr;
    ms_MuseSampler_start_audition_note_5 startAuditionNoteInternal5 = nullptr;
    ms_MuseSampler_add_track_syllable_event addSyllableEventInternal = nullptr;
    ms_MuseSampler_add_track_syllable_event_2 addSyllableEventInternal2 = nullptr;
    ms_MuseSampler_init initSamplerInternal = nullptr;
    ms_MuseSampler_init_2 initSamplerInternal2 = nullptr;

private:
    MuseSamplerLib m_lib = nullptr;
    Version m_version;
    int m_buildNumber = -1;
};

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}
