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

#ifndef MUSE_MUSESAMPLER_LIBHANDLER_H
#define MUSE_MUSESAMPLER_LIBHANDLER_H

#include <memory>
#include <cstring>

#include "dlib.h"
#include "apitypes.h"

#include "types/version.h"

namespace muse::musesampler {
struct MuseSamplerLibHandler
{
    ms_get_version_major getVersionMajor = nullptr;
    ms_get_version_minor getVersionMinor = nullptr;
    ms_get_version_revision getVersionRevision = nullptr;
    ms_get_version_build_number getBuildNumber = nullptr;
    ms_get_version_string getVersionString = nullptr;

    ms_contains_instrument containsInstrument = nullptr;

    ms_get_matching_instrument_id getMatchingInstrumentId = nullptr;
    ms_get_instrument_list getInstrumentList = nullptr;
    ms_get_matching_instrument_list getMatchingInstrumentList = nullptr;
    ms_InstrumentList_get_next getNextInstrument = nullptr;
    ms_Instrument_get_id getInstrumentId = nullptr;
    ms_Instrument_get_name getInstrumentName = nullptr;
    ms_Instrument_get_category getInstrumentCategory = nullptr;
    ms_Instrument_get_package getInstrumentPackage = nullptr;
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

    ms_MuseSampler_clear_score clearScore = nullptr;
    ms_MuseSampler_add_track addTrack = nullptr;
    ms_MuseSampler_finalize_track finalizeTrack = nullptr;
    ms_MuseSampler_clear_track clearTrack = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)> addDynamicsEvent = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)> addPedalEvent = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id)> addNoteEvent = nullptr;

    ms_MuseSampler_is_ranged_articulation isRangedArticulation = nullptr;
    ms_MuseSampler_add_track_event_range_start addTrackEventRangeStart = nullptr;
    ms_MuseSampler_add_track_event_range_end addTrackEventRangeEnd = nullptr;

    ms_MuseSampler_add_pitch_bend addPitchBend = nullptr;
    ms_MuseSampler_add_vibrato addVibrato = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev)> addSyllableEvent = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_4)> startAuditionNote = nullptr;
    ms_MuseSampler_stop_audition_note stopAuditionNote = nullptr;
    ms_MuseSampler_add_audition_cc_event addAuditionCCEvent = nullptr;

    ms_MuseSampler_start_liveplay_mode startLivePlayMode = nullptr;
    ms_MuseSampler_stop_liveplay_mode stopLivePlayMode = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2)> startLivePlayNote = nullptr;
    ms_MuseSampler_stop_liveplay_note stopLivePlayNote = nullptr;

    ms_MuseSampler_start_offline_mode startOfflineMode = nullptr;
    ms_MuseSampler_stop_offline_mode stopOfflineMode = nullptr;
    ms_MuseSampler_process_offline processOffline = nullptr;

    ms_MuseSampler_ready_to_play readyToPlay = nullptr;

    ms_MuseSampler_set_position setPosition = nullptr;
    ms_MuseSampler_set_playing setPlaying = nullptr;
    ms_MuseSampler_process process = nullptr;
    ms_MuseSampler_all_notes_off allNotesOff = nullptr;

    ms_reload_all_instruments reloadAllInstruments = nullptr;

    ms_Instrument_is_online isOnlineInstrument = nullptr;
    ms_MuseSampler_get_render_info getRenderInfo = nullptr;
    ms_RenderProgressInfo_get_next getNextRenderProgressInfo = nullptr;
    ms_MuseSampler_set_auto_render_interval setAutoRenderInterval = nullptr;
    ms_MuseSampler_trigger_render triggerRender = nullptr;

private:
    ms_init initLib = nullptr;
    ms_disable_reverb disableReverb = nullptr;
    ms_MuseSampler_add_track_dynamics_event_2 addDynamicsEventInternal2 = nullptr;
    ms_MuseSampler_add_track_pedal_event_2 addPedalEventInternal2 = nullptr;
    ms_MuseSampler_add_track_note_event_5 addNoteEventInternal5 = nullptr;
    ms_MuseSampler_start_audition_note_3 startAuditionNoteInternal3 = nullptr;
    ms_MuseSampler_start_audition_note_4 startAuditionNoteInternal4 = nullptr;
    ms_MuseSampler_start_liveplay_note_2 startLivePlayNoteInternal2 = nullptr;
    ms_MuseSampler_add_track_syllable_event addSyllableEventInternal = nullptr;
    ms_MuseSampler_add_track_syllable_event_2 addSyllableEventInternal2 = nullptr;
    ms_MuseSampler_init initSamplerInternal = nullptr;
    ms_MuseSampler_init_2 initSamplerInternal2 = nullptr;

public:
    MuseSamplerLibHandler(const io::path_t& path, bool useLegacyAudition = false)
    {
        // The specific versions supported, based on known functions/etc:
        const Version minimumSupported{ 0, 6, 0 };
        constexpr int maximumMajorVersion = 0;

        m_lib = muse::loadLib(path);

        if (!m_lib) {
            LOGE() << "Unable to open MuseSampler library, path: " << path;
            return;
        }

        initLib = (ms_init)muse::getLibFunc(m_lib, "ms_init");

        getVersionMajor = (ms_get_version_major)muse::getLibFunc(m_lib, "ms_get_version_major");
        getVersionMinor = (ms_get_version_minor)muse::getLibFunc(m_lib, "ms_get_version_minor");
        getVersionRevision = (ms_get_version_revision)muse::getLibFunc(m_lib, "ms_get_version_revision");
        getBuildNumber = (ms_get_version_build_number)muse::getLibFunc(m_lib, "ms_get_version_build_number");
        getVersionString = (ms_get_version_string)muse::getLibFunc(m_lib, "ms_get_version_string");

        // Invalid...
        if (!getVersionMajor || !getVersionMinor || !getVersionRevision) {
            return;
        }

        Version current(getVersionMajor(),
                        getVersionMinor(),
                        getVersionRevision());
        if (current < minimumSupported) {
            LOGE() << "MuseSampler " << current.toString() << " is not supported (too old -- update MuseSampler); ignoring";
            return;
        }

        // Major versions have incompatible changes; we can only support
        // interfaces we know about.
        // TODO: check when we fixed the issue with version numbers not reporting?  Was this ever an issue?
        if (current.major() > maximumMajorVersion) {
            LOGE() << "MuseSampler " << current.toString() << " is not supported (too new -- update MuseScore Studio); ignoring";
            return;
        }

        // Check versions here; define optional functions below
        int versionMajor = getVersionMajor();
        int versionMinor = getVersionMinor();

        bool at_least_v_0_100 = (versionMajor == 0 && versionMinor >= 100) || versionMajor > 0;
        bool at_least_v_0_101 = (versionMajor == 0 && versionMinor >= 101) || versionMajor > 0;
        bool at_least_v_0_102 = (versionMajor == 0 && versionMinor >= 102) || versionMajor > 0;
        m_supportsReinit = at_least_v_0_100;

        containsInstrument = (ms_contains_instrument)muse::getLibFunc(m_lib, "ms_contains_instrument");
        getMatchingInstrumentId = (ms_get_matching_instrument_id)muse::getLibFunc(m_lib, "ms_get_matching_instrument_id");
        getInstrumentList = (ms_get_instrument_list)muse::getLibFunc(m_lib, "ms_get_instrument_list");
        getMatchingInstrumentList = (ms_get_matching_instrument_list)muse::getLibFunc(m_lib, "ms_get_matching_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)muse::getLibFunc(m_lib, "ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)muse::getLibFunc(m_lib, "ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)muse::getLibFunc(m_lib, "ms_Instrument_get_name");
        getInstrumentCategory = (ms_Instrument_get_category)muse::getLibFunc(m_lib, "ms_Instrument_get_category");
        getInstrumentPackage = (ms_Instrument_get_package)muse::getLibFunc(m_lib, "ms_Instrument_get_package");

        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)muse::getLibFunc(m_lib, "ms_Instrument_get_musicxml_sound");
        getMpeSoundId = (ms_Instrument_get_mpe_sound)muse::getLibFunc(m_lib, "ms_Instrument_get_mpe_sound");

        getPresetList = (ms_Instrument_get_preset_list)muse::getLibFunc(m_lib, "ms_Instrument_get_preset_list");
        getNextPreset = (ms_PresetList_get_next)muse::getLibFunc(m_lib, "ms_PresetList_get_next");

        create = (ms_MuseSampler_create)muse::getLibFunc(m_lib, "ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)muse::getLibFunc(m_lib, "ms_MuseSampler_destroy");

        if (useLegacyAudition) {
            LOGI() << "Use legacy audition (ms_MuseSampler_init)";
        }

        if (at_least_v_0_100 && !useLegacyAudition) {
            initSamplerInternal2 = (ms_MuseSampler_init_2)muse::getLibFunc(m_lib, "ms_MuseSampler_init_2");
            initSampler = [this](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerInternal2(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        } else {
            initSamplerInternal = (ms_MuseSampler_init)muse::getLibFunc(m_lib, "ms_MuseSampler_init");
            initSampler = [this](ms_MuseSampler ms, double sample_rate, int block_size, int channel_count) {
                return initSamplerInternal(ms, sample_rate, block_size, channel_count) == ms_Result_OK;
            };
        }

        clearScore = (ms_MuseSampler_clear_score)muse::getLibFunc(m_lib, "ms_MuseSampler_clear_score");
        addTrack = (ms_MuseSampler_add_track)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track");
        finalizeTrack = (ms_MuseSampler_finalize_track)muse::getLibFunc(m_lib, "ms_MuseSampler_finalize_track");
        clearTrack = (ms_MuseSampler_clear_track)muse::getLibFunc(m_lib, "ms_MuseSampler_clear_track");

        disableReverb = (ms_disable_reverb)muse::getLibFunc(m_lib, "ms_disable_reverb");
        getReverbLevel = (ms_Instrument_get_reverb_level)muse::getLibFunc(m_lib, "ms_Instrument_get_reverb_level");

        addDynamicsEventInternal2 = (ms_MuseSampler_add_track_dynamics_event_2)muse::getLibFunc(m_lib,
                                                                                                "ms_MuseSampler_add_track_dynamics_event_2");
        addDynamicsEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
            ms_DynamicsEvent_2 evt{ timestamp, value };
            return addDynamicsEventInternal2(ms, track, evt) == ms_Result_OK;
        };

        addPedalEventInternal2 = (ms_MuseSampler_add_track_pedal_event_2)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_pedal_event_2");
        addPedalEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
            ms_PedalEvent_2 evt{ timestamp, value };
            return addPedalEventInternal2(ms, track, evt) == ms_Result_OK;
        };

        addNoteEventInternal5 = (ms_MuseSampler_add_track_note_event_5)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_note_event_5");
        addNoteEvent = [this](ms_MuseSampler ms, ms_Track track, const NoteEvent& ev, long long& event_id) {
            return addNoteEventInternal5(ms, track, ev, event_id) == ms_Result_OK;
        };

        if (at_least_v_0_100) {
            startAuditionNoteInternal4 = (ms_MuseSampler_start_audition_note_4)muse::getLibFunc(m_lib,
                                                                                                "ms_MuseSampler_start_audition_note_4");
            startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_4 ev) {
                return startAuditionNoteInternal4(ms, track, ev) == ms_Result_OK;
            };
        } else {
            startAuditionNoteInternal3 = (ms_MuseSampler_start_audition_note_3)muse::getLibFunc(m_lib,
                                                                                                "ms_MuseSampler_start_audition_note_3");
            startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_4 ev) {
                ms_AuditionStartNoteEvent_3 ev3{ ev._pitch, ev._offset_cents, ev._articulation, ev._notehead, ev._dynamics,
                                                 ev._active_presets, ev._active_text_articulation };

                return startAuditionNoteInternal3(ms, track, ev3) == ms_Result_OK;
            };
        }

        isRangedArticulation = (ms_MuseSampler_is_ranged_articulation)muse::getLibFunc(m_lib, "ms_MuseSampler_is_ranged_articulation");
        addTrackEventRangeStart
            = (ms_MuseSampler_add_track_event_range_start)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_start");
        addTrackEventRangeEnd
            = (ms_MuseSampler_add_track_event_range_end)muse::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_end");

        stopAuditionNote = (ms_MuseSampler_stop_audition_note)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_audition_note");

        startLivePlayMode = (ms_MuseSampler_start_liveplay_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_mode");
        stopLivePlayMode = (ms_MuseSampler_stop_liveplay_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_mode");

        startLivePlayNoteInternal2 = (ms_MuseSampler_start_liveplay_note_2)muse::getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_note_2");
        startLivePlayNote = [this](ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2 evt) {
            return startLivePlayNoteInternal2(ms, track, evt) == ms_Result_OK;
        };

        if (at_least_v_0_102) {
            addSyllableEventInternal2 = (ms_MuseSampler_add_track_syllable_event_2)muse::getLibFunc(m_lib,
                                                                                                    "ms_MuseSampler_add_track_syllable_event_2");

            addSyllableEvent = [this](ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev) {
                return addSyllableEventInternal2(ms, track, ev) == ms_Result_OK;
            };
        } else if (at_least_v_0_100) {
            addSyllableEventInternal = (ms_MuseSampler_add_track_syllable_event)muse::getLibFunc(m_lib,
                                                                                                 "ms_MuseSampler_add_track_syllable_event");

            addSyllableEvent = [this](ms_MuseSampler ms, ms_Track track, const SyllableEvent& ev) {
                ms_SyllableEvent ev1 { ev._text, ev._position_us };
                return addSyllableEventInternal(ms, track, ev1) == ms_Result_OK;
            };
        } else {
            addSyllableEvent = [](ms_MuseSampler, ms_Track, const SyllableEvent&) { return ms_Result_Error; };
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

        stopLivePlayNote = (ms_MuseSampler_stop_liveplay_note)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_note");

        startOfflineMode = (ms_MuseSampler_start_offline_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_start_offline_mode");
        stopOfflineMode = (ms_MuseSampler_stop_offline_mode)muse::getLibFunc(m_lib, "ms_MuseSampler_stop_offline_mode");
        processOffline = (ms_MuseSampler_process_offline)muse::getLibFunc(m_lib, "ms_MuseSampler_process_offline");

        setPosition = (ms_MuseSampler_set_position)muse::getLibFunc(m_lib, "ms_MuseSampler_set_position");
        setPlaying = (ms_MuseSampler_set_playing)muse::getLibFunc(m_lib, "ms_MuseSampler_set_playing");
        process = (ms_MuseSampler_process)muse::getLibFunc(m_lib, "ms_MuseSampler_process");
        allNotesOff = (ms_MuseSampler_all_notes_off)muse::getLibFunc(m_lib, "ms_MuseSampler_all_notes_off");

        if (at_least_v_0_100) {
            reloadAllInstruments = (ms_reload_all_instruments)muse::getLibFunc(m_lib, "ms_reload_all_instruments");
        } else {
            reloadAllInstruments = []() { return ms_Result_Error; };
        }

        if (at_least_v_0_101) {
            readyToPlay = (ms_MuseSampler_ready_to_play)muse::getLibFunc(m_lib, "ms_MuseSampler_ready_to_play");
        } else {
            readyToPlay = [](ms_MuseSampler) { return true; };
        }

        if (at_least_v_0_102) {
            isOnlineInstrument = (ms_Instrument_is_online)muse::getLibFunc(m_lib, "ms_Instrument_is_online");
            getRenderInfo = (ms_MuseSampler_get_render_info)muse::getLibFunc(m_lib, "ms_MuseSampler_get_render_info");
            getNextRenderProgressInfo = (ms_RenderProgressInfo_get_next)muse::getLibFunc(m_lib, "ms_RenderProgressInfo_get_next");
            setAutoRenderInterval = (ms_MuseSampler_set_auto_render_interval)muse::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_set_auto_render_interval");
            triggerRender = (ms_MuseSampler_trigger_render)muse::getLibFunc(m_lib, "ms_MuseSampler_trigger_render");
            addAuditionCCEvent = (ms_MuseSampler_add_audition_cc_event)muse::getLibFunc(m_lib, "ms_MuseSampler_add_audition_cc_event");
        } else {
            isOnlineInstrument = [](ms_InstrumentInfo) { return false; };
            getRenderInfo = [](ms_MuseSampler, int*) { return ms_RenderingRangeList(); };
            getNextRenderProgressInfo = [](ms_RenderingRangeList) { return ms_RenderRangeInfo { 0, 0, ms_RenderingState_ErrorRendering }; };
            setAutoRenderInterval = [](ms_MuseSampler, double) {};
            triggerRender = [](ms_MuseSampler) {};
            addAuditionCCEvent = [](ms_MuseSampler, ms_Track, int, float) { return ms_Result_Error; };
        }
    }

    ~MuseSamplerLibHandler()
    {
        if (!m_lib) {
            return;
        }

        muse::closeLib(m_lib);
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
               && getVersionMajor
               && getVersionMinor
               && getVersionRevision
               && getBuildNumber
               && getVersionString
               && containsInstrument
               && getMatchingInstrumentId
               && getInstrumentList
               && getMatchingInstrumentList
               && getNextInstrument
               && getInstrumentId
               && getInstrumentName
               && getInstrumentCategory
               && getInstrumentPackage
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
               && clearScore
               && addTrack
               && finalizeTrack
               && clearTrack
               && addDynamicsEvent
               && addPedalEvent
               && addNoteEvent
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

    bool supportsReinit() const
    {
        return m_supportsReinit;
    }

private:
    void printApiStatus() const
    {
        LOGI() << "MuseSampler API status:"
               << "\n ms_get_version_major - " << reinterpret_cast<uint64_t>(getVersionMajor)
               << "\n ms_get_version_minor - " << reinterpret_cast<uint64_t>(getVersionMinor)
               << "\n ms_get_version_revision - " << reinterpret_cast<uint64_t>(getVersionRevision)
               << "\n ms_get_version_build_number - " << reinterpret_cast<uint64_t>(getBuildNumber)
               << "\n ms_get_version_string - " << reinterpret_cast<uint64_t>(getVersionString)
               << "\n ms_contains_instrument - " << reinterpret_cast<uint64_t>(containsInstrument)
               << "\n ms_get_matching_instrument_id - " << reinterpret_cast<uint64_t>(getMatchingInstrumentId)
               << "\n ms_get_instrument_list -" << reinterpret_cast<uint64_t>(getInstrumentList)
               << "\n ms_get_matching_instrument_list - " << reinterpret_cast<uint64_t>(getMatchingInstrumentList)
               << "\n ms_get_drum_mapping - " << reinterpret_cast<uint64_t>(getDrumMapping)
               << "\n ms_InstrumentList_get_next - " << reinterpret_cast<uint64_t>(getNextInstrument)
               << "\n ms_Instrument_get_info_json - " << reinterpret_cast<uint64_t>(getInstrumentInfoJson)
               << "\n ms_Instrument_get_id - " << reinterpret_cast<uint64_t>(getInstrumentId)
               << "\n ms_Instrument_get_name - " << reinterpret_cast<uint64_t>(getInstrumentName)
               << "\n ms_Instrument_get_vendor_name - " << reinterpret_cast<uint64_t>(getInstrumentVendorName)
               << "\n ms_Instrument_get_category - " << reinterpret_cast<uint64_t>(getInstrumentCategory)
               << "\n ms_Instrument_get_package - " << reinterpret_cast<uint64_t>(getInstrumentPackage)
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
               << "\n ms_disable_reverb - " << reinterpret_cast<uint64_t>(disableReverb)
               << "\n ms_MuseSampler_clear_score - " << reinterpret_cast<uint64_t>(clearScore)
               << "\n ms_MuseSampler_add_track - " << reinterpret_cast<uint64_t>(addTrack)
               << "\n ms_MuseSampler_finalize_track - " << reinterpret_cast<uint64_t>(finalizeTrack)
               << "\n ms_MuseSampler_add_track_dynamics_event_2 - " << reinterpret_cast<uint64_t>(addDynamicsEventInternal2)
               << "\n ms_MuseSampler_add_track_pedal_event_2 - " << reinterpret_cast<uint64_t>(addPedalEventInternal2)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_start_audition_note_3 - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal3)
               << "\n ms_MuseSampler_stop_audition_note - " << reinterpret_cast<uint64_t>(stopAuditionNote)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_add_vibrato - " << reinterpret_cast<uint64_t>(addVibrato)
               << "\n ms_MuseSampler_start_liveplay_mode - " << reinterpret_cast<uint64_t>(startLivePlayMode)
               << "\n ms_MuseSampler_stop_liveplay_mode - " << reinterpret_cast<uint64_t>(stopLivePlayMode)
               << "\n ms_MuseSampler_start_liveplay_note_2 - " << reinterpret_cast<uint64_t>(startLivePlayNoteInternal2)
               << "\n ms_MuseSampler_stop_liveplay_note - " << reinterpret_cast<uint64_t>(stopLivePlayNote)
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
               << "\n ms_MuseSampler_add_track_syllable_event_2 - " << reinterpret_cast<uint64_t>(addSyllableEventInternal2);
    }

    MuseSamplerLib m_lib = nullptr;
    bool m_supportsReinit = false;
};

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}

#endif // MUSE_MUSESAMPLER_LIBHANDLER_H
