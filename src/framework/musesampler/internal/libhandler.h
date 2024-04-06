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

#ifdef USE_MUSESAMPLER_SRC
#include "Api/musesampler_api.h"
#else
#include "dlib.h"

#include "apitypes.h"
#endif

#include "types/version.h"

namespace muse::musesampler {
#ifdef USE_MUSESAMPLER_SRC
struct MuseSamplerLibHandler
{
    ms_Result initLib() { return ms_init(); }
    ms_Result disableReverb() { return ms_disable_reverb(); }

    bool containsInstrument(const char* mpe_id, const char* musicxml)
    {
        return ms_contains_instrument(mpe_id, musicxml) == 1;
    }

    int getVersionMajor() { return ms_get_version_major(); }
    int getVersionMinor() { return ms_get_version_minor(); }
    int getVersionRevision() { return ms_get_version_revision(); }
    const char* getVersionString() { return ms_get_version_string(); }

    int getMatchingInstrumentId(const char* pack, const char* name) { return ms_get_matching_instrument_id(pack, name); }
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
    float getReverbLevel(ms_InstrumentInfo instrument) { return ms_Instrument_get_reverb_level(instrument); }

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

    bool addDynamicsEvent(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)
    {
        ms_DynamicsEvent_2 evt;
        evt._location_us = timestamp;
        evt._value = value;
        return ms_MuseSampler_add_track_dynamics_event_2(ms, track, evt) == ms_Result_OK;
    }

    bool addPedalEvent(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)
    {
        ms_PedalEvent_2 evt;
        evt._location_us = timestamp;
        evt._value = value;
        return ms_MuseSampler_add_track_pedal_event_2(ms, track, evt) == ms_Result_OK;
    }

    bool addNoteEvent(ms_MuseSampler ms, ms_Track track, int voice, long long location_us, long long duration_us, int pitch, double tempo,
                      int offset_cents, ms_NoteArticulation articulation)
    {
        ms_NoteEvent_3 evt{ voice, location_us, duration_us, pitch, tempo, offset_cents, articulation };
        return ms_MuseSampler_add_track_note_event_3(ms, track, evt) == ms_Result_OK
    }

    bool isRangedArticulation(ms_NoteArticulation art) { return ms_MuseSampler_is_ranged_articulation(art) == 1; }
    ms_Result addTrackEventRangeStart(ms_MuseSampler ms, ms_Track track, int voice, ms_NoteArticulation art)
    {
        return ms_MuseSampler_add_track_event_range_start(ms, track, voice, art);
    }

    ms_Result addTrackEventRangeEnd(ms_MuseSampler ms, ms_Track track, int voice, ms_NoteArticulation art)
    {
        return ms_MuseSampler_add_track_event_range_end(ms, track, voice, art);
    }

    ms_Result addPitchBend(ms_MuseSampler ms, ms_Track track, ms_PitchBendInfo info)
    {
        return ms_MuseSampler_add_pitch_bend(ms, track, info);
    }

    ms_Result startAuditionMode(ms_MuseSampler ms) { return ms_MuseSampler_start_audition_mode(ms); }
    ms_Result stopAuditionMode(ms_MuseSampler ms) { return ms_MuseSampler_stop_audition_mode(ms); }
    ms_Result startAuditionNote(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_2 evt)
    {
        return ms_MuseSampler_start_audition_note_2(ms, track, evt) == ms_Result_OK;
    }

    ms_Result stopAuditionNote(ms_MuseSampler ms, ms_Track track, ms_AuditionStopNoteEvent evt)
    {
        return ms_MuseSampler_stop_audition_note(ms, track, evt);
    }

    ms_Result startLivePlayMode(ms_MuseSampler ms) { return ms_MuseSampler_start_liveplay_mode(ms); }
    ms_Result stopLivePlayMode(ms_MuseSampler ms) { return ms_MuseSampler_stop_liveplay_mode(ms); }
    ms_Result startLivePlayNote(ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2 evt)
    {
        return ms_MuseSampler_start_liveplay_note_2(ms, track, evt) == ms_Result_OK;
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
    ms_disable_reverb disableReverb = nullptr;
    ms_get_version_major getVersionMajor = nullptr;
    ms_get_version_minor getVersionMinor = nullptr;
    ms_get_version_revision getVersionRevision = nullptr;
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
    ms_MuseSampler_init initSampler = nullptr;

    ms_MuseSampler_clear_score clearScore = nullptr;
    ms_MuseSampler_add_track addTrack = nullptr;
    ms_MuseSampler_finalize_track finalizeTrack = nullptr;
    ms_MuseSampler_clear_track clearTrack = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)> addDynamicsEvent = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, long long timestamp, float value)> addPedalEvent = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id)> addNoteEvent = nullptr;
    ms_MuseSampler_is_ranged_articulation isRangedArticulation = nullptr;
    ms_MuseSampler_add_track_event_range_start addTrackEventRangeStart = nullptr;
    ms_MuseSampler_add_track_event_range_end addTrackEventRangeEnd = nullptr;

    ms_MuseSampler_add_pitch_bend addPitchBend = nullptr;
    ms_MuseSampler_add_vibrato addVibrato = nullptr;

    std::function<bool(ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_3)> startAuditionNote = nullptr;
    ms_MuseSampler_stop_audition_note stopAuditionNote = nullptr;

    ms_MuseSampler_start_liveplay_mode startLivePlayMode = nullptr;
    ms_MuseSampler_stop_liveplay_mode stopLivePlayMode = nullptr;
    std::function<bool(ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2)> startLivePlayNote = nullptr;
    ms_MuseSampler_stop_liveplay_note stopLivePlayNote = nullptr;

    ms_MuseSampler_start_offline_mode startOfflineMode = nullptr;
    ms_MuseSampler_stop_offline_mode stopOfflineMode = nullptr;
    ms_MuseSampler_process_offline processOffline = nullptr;

    ms_MuseSampler_set_position setPosition = nullptr;
    ms_MuseSampler_set_playing setPlaying = nullptr;
    ms_MuseSampler_process process = nullptr;
    ms_MuseSampler_all_notes_off allNotesOff = nullptr;

private:
    ms_MuseSampler_add_track_dynamics_event addDynamicsEventInternal = nullptr;
    ms_MuseSampler_add_track_dynamics_event_2 addDynamicsEventInternal2 = nullptr;
    ms_MuseSampler_add_track_pedal_event addPedalEventInternal = nullptr;
    ms_MuseSampler_add_track_pedal_event_2 addPedalEventInternal2 = nullptr;
    ms_MuseSampler_add_track_note_event addNoteEventInternal = nullptr;
    ms_MuseSampler_add_track_note_event_2 addNoteEventInternal2 = nullptr;
    ms_MuseSampler_add_track_note_event_3 addNoteEventInternal3 = nullptr;
    ms_MuseSampler_add_track_note_event_4 addNoteEventInternal4 = nullptr;
    ms_MuseSampler_add_track_note_event_5 addNoteEventInternal5 = nullptr;
    ms_MuseSampler_start_audition_note startAuditionNoteInternal = nullptr;
    ms_MuseSampler_start_audition_note_2 startAuditionNoteInternal2 = nullptr;
    ms_MuseSampler_start_audition_note_3 startAuditionNoteInternal3 = nullptr;
    ms_MuseSampler_start_liveplay_note startLivePlayNoteInternal = nullptr;
    ms_MuseSampler_start_liveplay_note_2 startLivePlayNoteInternal2 = nullptr;
public:

    MuseSamplerLibHandler(const io::path_t& path)
    {
        // The specific versions supported, based on known functions/etc:
        const mu::Version minimumSupported{ 0, 2, 2 };
        constexpr int maximumMajorVersion = 0;

        m_lib = mu::loadLib(path);

        if (!m_lib) {
            LOGE() << "Unable to open MuseSampler library, path: " << path;
            return;
        }

        initLib = (ms_init)mu::getLibFunc(m_lib, "ms_init");

        getVersionMajor = (ms_get_version_major)mu::getLibFunc(m_lib, "ms_get_version_major");
        getVersionMinor = (ms_get_version_minor)mu::getLibFunc(m_lib, "ms_get_version_minor");
        getVersionRevision = (ms_get_version_revision)mu::getLibFunc(m_lib, "ms_get_version_revision");
        getVersionString = (ms_get_version_string)mu::getLibFunc(m_lib, "ms_get_version_string");

        // Invalid...
        if (!getVersionMajor || !getVersionMinor || !getVersionRevision) {
            return;
        }

        mu::Version current(getVersionMajor(),
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
            LOGE() << "MuseSampler " << current.toString() << " is not supported (too new -- update MuseScore); ignoring";
            return;
        }

        // Check versions here; define optional functions below
        int versionMajor = getVersionMajor();
        int versionMinor = getVersionMinor();

        bool at_least_v_0_3 = (versionMajor == 0 && versionMinor >= 3) || versionMajor > 0;
        bool at_least_v_0_4 = (versionMajor == 0 && versionMinor >= 4) || versionMajor > 0;
        bool at_least_v_0_5 = (versionMajor == 0 && versionMinor >= 5) || versionMajor > 0;
        bool at_least_v_0_6 = (versionMajor == 0 && versionMinor >= 6) || versionMajor > 0;

        m_supportsMultipleTracks = at_least_v_0_6;

        containsInstrument = (ms_contains_instrument)mu::getLibFunc(m_lib, "ms_contains_instrument");
        getMatchingInstrumentId = (ms_get_matching_instrument_id)mu::getLibFunc(m_lib, "ms_get_matching_instrument_id");
        getInstrumentList = (ms_get_instrument_list)mu::getLibFunc(m_lib, "ms_get_instrument_list");
        getMatchingInstrumentList = (ms_get_matching_instrument_list)mu::getLibFunc(m_lib, "ms_get_matching_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)mu::getLibFunc(m_lib, "ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)mu::getLibFunc(m_lib, "ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)mu::getLibFunc(m_lib, "ms_Instrument_get_name");
        getInstrumentCategory = (ms_Instrument_get_category)mu::getLibFunc(m_lib, "ms_Instrument_get_category");
        getInstrumentPackage = (ms_Instrument_get_package)mu::getLibFunc(m_lib, "ms_Instrument_get_package");

        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)mu::getLibFunc(m_lib, "ms_Instrument_get_musicxml_sound");
        getMpeSoundId = (ms_Instrument_get_mpe_sound)mu::getLibFunc(m_lib, "ms_Instrument_get_mpe_sound");

        getPresetList = (ms_Instrument_get_preset_list)mu::getLibFunc(m_lib, "ms_Instrument_get_preset_list");
        getNextPreset = (ms_PresetList_get_next)mu::getLibFunc(m_lib, "ms_PresetList_get_next");

        create = (ms_MuseSampler_create)mu::getLibFunc(m_lib, "ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)mu::getLibFunc(m_lib, "ms_MuseSampler_destroy");
        initSampler = (ms_MuseSampler_init)mu::getLibFunc(m_lib, "ms_MuseSampler_init");

        clearScore = (ms_MuseSampler_clear_score)mu::getLibFunc(m_lib, "ms_MuseSampler_clear_score");
        addTrack = (ms_MuseSampler_add_track)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track");
        finalizeTrack = (ms_MuseSampler_finalize_track)mu::getLibFunc(m_lib, "ms_MuseSampler_finalize_track");
        clearTrack = (ms_MuseSampler_clear_track)mu::getLibFunc(m_lib, "ms_MuseSampler_clear_track");

        if (at_least_v_0_4) {
            if (addDynamicsEventInternal2
                    = (ms_MuseSampler_add_track_dynamics_event_2)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_dynamics_event_2");
                addDynamicsEventInternal2 != nullptr) {
                addDynamicsEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
                    ms_DynamicsEvent_2 evt{ timestamp, value };
                    return addDynamicsEventInternal2(ms, track, evt) == ms_Result_OK;
                };
            }
            disableReverb = (ms_disable_reverb)mu::getLibFunc(m_lib, "ms_disable_reverb");
            getReverbLevel = (ms_Instrument_get_reverb_level)mu::getLibFunc(m_lib, "ms_Instrument_get_reverb_level");
        } else {
            if (addDynamicsEventInternal
                    = (ms_MuseSampler_add_track_dynamics_event)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_dynamics_event");
                addDynamicsEventInternal != nullptr) {
                addDynamicsEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
                    ms_DynamicsEvent evt{ static_cast<long>(timestamp), value };
                    return addDynamicsEventInternal(ms, track, evt) == ms_Result_OK;
                };
            }
        }

        if (at_least_v_0_4) {
            if (addPedalEventInternal2
                    = (ms_MuseSampler_add_track_pedal_event_2)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_pedal_event_2");
                addPedalEventInternal2 != nullptr) {
                addPedalEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
                    ms_PedalEvent_2 evt{ timestamp, value };
                    return addPedalEventInternal2(ms, track, evt) == ms_Result_OK;
                };
            }
        } else {
            if (addPedalEventInternal = (ms_MuseSampler_add_track_pedal_event)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_pedal_event");
                addPedalEventInternal != nullptr) {
                addPedalEvent = [this](ms_MuseSampler ms, ms_Track track, long long timestamp, float value) {
                    // TODO: down cast of long long? trim?
                    ms_PedalEvent evt{ static_cast<long>(timestamp), value };
                    return addPedalEventInternal(ms, track, evt) == ms_Result_OK;
                };
            }
        }

        if (at_least_v_0_6) {
            if (addNoteEventInternal5 = (ms_MuseSampler_add_track_note_event_5)mu::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_add_track_note_event_5");
                addNoteEventInternal5 != nullptr) {
                addNoteEvent
                    = [this](ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id) {
                    return addNoteEventInternal5(ms, track, ev, event_id) == ms_Result_OK;
                };
            }
        } else if (at_least_v_0_5) {
            if (addNoteEventInternal4 = (ms_MuseSampler_add_track_note_event_4)mu::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_add_track_note_event_4");
                addNoteEventInternal4 != nullptr) {
                addNoteEvent
                    = [this](ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id) {
                    ms_NoteEvent_3 ev3{ ev._voice, ev._location_us, ev._duration_us, ev._pitch, ev._tempo, ev._offset_cents,
                                        ev._articulation };
                    return addNoteEventInternal4(ms, track, ev3, event_id) == ms_Result_OK;
                };
            }
        } else if (at_least_v_0_4) {
            if (addNoteEventInternal3 = (ms_MuseSampler_add_track_note_event_3)mu::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_add_track_note_event_3");
                addNoteEventInternal3 != nullptr) {
                addNoteEvent
                    = [this](ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id) {
                    event_id = 0;
                    ms_NoteEvent_3 ev3{ ev._voice, ev._location_us, ev._duration_us, ev._pitch, ev._tempo, ev._offset_cents,
                                        ev._articulation };
                    return addNoteEventInternal3(ms, track, ev3) == ms_Result_OK;
                };
            }
        } else if (at_least_v_0_3) {
            if (addNoteEventInternal2 = (ms_MuseSampler_add_track_note_event_2)mu::getLibFunc(m_lib,
                                                                                              "ms_MuseSampler_add_track_note_event_2");
                addNoteEventInternal2 != nullptr) {
                addNoteEvent
                    = [this](ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id) {
                    event_id = 0;
                    ms_NoteEvent_2 ev2{ ev._voice, static_cast<long>(ev._location_us), static_cast<long>(ev._duration_us), ev._pitch,
                                        ev._tempo, ev._offset_cents,
                                        ev._articulation };
                    return addNoteEventInternal2(ms, track, ev2) == ms_Result_OK;
                };
            }
        } else {
            if (addNoteEventInternal = (ms_MuseSampler_add_track_note_event)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_note_event");
                addNoteEventInternal != nullptr) {
                addNoteEvent
                    = [this](ms_MuseSampler ms, ms_Track track, NoteEvent ev, long long& event_id) {
                    event_id = 0;
                    ms_NoteEvent ev0{ ev._voice, static_cast<long>(ev._location_us), static_cast<long>(ev._duration_us), ev._pitch,
                                      ev._tempo, ev._articulation };
                    return addNoteEventInternal(ms, track, ev0) == ms_Result_OK;
                };
            }
        }

        isRangedArticulation = (ms_MuseSampler_is_ranged_articulation)mu::getLibFunc(m_lib, "ms_MuseSampler_is_ranged_articulation");
        addTrackEventRangeStart
            = (ms_MuseSampler_add_track_event_range_start)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_start");
        addTrackEventRangeEnd = (ms_MuseSampler_add_track_event_range_end)mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_event_range_end");

        if (at_least_v_0_6) {
            if (startAuditionNoteInternal3
                    = (ms_MuseSampler_start_audition_note_3)mu::getLibFunc(m_lib, "ms_MuseSampler_start_audition_note_3");
                startAuditionNoteInternal3 != nullptr) {
                startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_3 evt) {
                    return startAuditionNoteInternal3(ms, track, evt) == ms_Result_OK;
                };
            }
        } else if (at_least_v_0_3) {
            if (startAuditionNoteInternal2
                    = (ms_MuseSampler_start_audition_note_2)mu::getLibFunc(m_lib, "ms_MuseSampler_start_audition_note_2");
                startAuditionNoteInternal2 != nullptr) {
                startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_3 evt3) {
                    ms_AuditionStartNoteEvent_2 evt2{ evt3._pitch, evt3._offset_cents, evt3._articulation, evt3._dynamics };
                    return startAuditionNoteInternal2(ms, track, evt2) == ms_Result_OK;
                };
            }
        } else {
            if (startAuditionNoteInternal = (ms_MuseSampler_start_audition_note)mu::getLibFunc(m_lib, "ms_MuseSampler_start_audition_note");
                startAuditionNoteInternal != nullptr) {
                startAuditionNote = [this](ms_MuseSampler ms, ms_Track track, ms_AuditionStartNoteEvent_3 evt3) {
                    ms_AuditionStartNoteEvent evt{ evt3._pitch, evt3._articulation, evt3._dynamics };
                    return startAuditionNoteInternal(ms, track, evt) == ms_Result_OK;
                };
            }
        }
        stopAuditionNote = (ms_MuseSampler_stop_audition_note)mu::getLibFunc(m_lib, "ms_MuseSampler_stop_audition_note");

        startLivePlayMode = (ms_MuseSampler_start_liveplay_mode)mu::getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_mode");
        stopLivePlayMode = (ms_MuseSampler_stop_liveplay_mode)mu::getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_mode");
        if (at_least_v_0_3) {
            if (startLivePlayNoteInternal2
                    = (ms_MuseSampler_start_liveplay_note_2)mu::getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_note_2");
                startLivePlayNoteInternal2 != nullptr) {
                startLivePlayNote = [this](ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2 evt) {
                    return startLivePlayNoteInternal2(ms, track, evt) == ms_Result_OK;
                };
            }
        } else {
            if (startLivePlayNoteInternal = (ms_MuseSampler_start_liveplay_note)mu::getLibFunc(m_lib, "ms_MuseSampler_start_liveplay_note");
                startLivePlayNoteInternal != nullptr) {
                startLivePlayNote = [this](ms_MuseSampler ms, ms_Track track, ms_LivePlayStartNoteEvent_2 evt2) {
                    ms_LivePlayStartNoteEvent evt{ evt2._pitch, evt2._dynamics };
                    return startLivePlayNoteInternal(ms, track, evt) == ms_Result_OK;
                };
            }
        }

        if (at_least_v_0_6) {
            getInstrumentVendorName = (ms_Instrument_get_vendor_name)mu::getLibFunc(m_lib, "ms_Instrument_get_vendor_name");
            getInstrumentPackName = (ms_Instrument_get_pack_name)mu::getLibFunc(m_lib, "ms_Instrument_get_pack_name");
            createPresetChange = (ms_MuseSampler_create_preset_change)mu::getLibFunc(m_lib, "ms_MuseSampler_create_preset_change");
            addPreset = (ms_MuseSampler_add_preset)mu::getLibFunc(m_lib, "ms_MuseSampler_add_preset");
            getTextArticulations = (ms_get_text_articulations)mu::getLibFunc(m_lib, "ms_get_text_articulations");
            addTextArticulationEvent = (ms_MuseSampler_add_track_text_articulation_event)
                                       mu::getLibFunc(m_lib, "ms_MuseSampler_add_track_text_articulation_event");
            getDrumMapping = (ms_get_drum_mapping)mu::getLibFunc(m_lib, "ms_get_drum_mapping");
        } else {
            getInstrumentVendorName = [](ms_InstrumentInfo) { return ""; };
            getInstrumentPackName = [](ms_InstrumentInfo) { return ""; };
            createPresetChange = [](ms_MuseSampler, ms_Track, long long) { return -1; };
            addPreset = [](ms_MuseSampler, ms_Track, ms_PresetChange, const char*) { return ms_Result_Error; };
            getTextArticulations = [](int, const char*) { return ""; };
            addTextArticulationEvent = [](ms_MuseSampler, ms_Track, ms_TextArticulationEvent) { return ms_Result_Error; };
            getDrumMapping = [](int) { return ""; };
        }

        if (at_least_v_0_5) {
            addPitchBend = (ms_MuseSampler_add_pitch_bend)mu::getLibFunc(m_lib, "ms_MuseSampler_add_pitch_bend");
            addVibrato = (ms_MuseSampler_add_vibrato)mu::getLibFunc(m_lib, "ms_MuseSampler_add_vibrato");
        }

        stopLivePlayNote = (ms_MuseSampler_stop_liveplay_note)mu::getLibFunc(m_lib, "ms_MuseSampler_stop_liveplay_note");

        startOfflineMode = (ms_MuseSampler_start_offline_mode)mu::getLibFunc(m_lib, "ms_MuseSampler_start_offline_mode");
        stopOfflineMode = (ms_MuseSampler_stop_offline_mode)mu::getLibFunc(m_lib, "ms_MuseSampler_stop_offline_mode");
        processOffline = (ms_MuseSampler_process_offline)mu::getLibFunc(m_lib, "ms_MuseSampler_process_offline");

        setPosition = (ms_MuseSampler_set_position)mu::getLibFunc(m_lib, "ms_MuseSampler_set_position");
        setPlaying = (ms_MuseSampler_set_playing)mu::getLibFunc(m_lib, "ms_MuseSampler_set_playing");
        process = (ms_MuseSampler_process)mu::getLibFunc(m_lib, "ms_MuseSampler_process");
        allNotesOff = (ms_MuseSampler_all_notes_off)mu::getLibFunc(m_lib, "ms_MuseSampler_all_notes_off");

        if (initLib) {
            initLib();

            if (disableReverb) {
                disableReverb();
            }
        }
    }

    ~MuseSamplerLibHandler()
    {
        if (!m_lib) {
            return;
        }

        mu::closeLib(m_lib);
    }

    bool isValid() const
    {
        return m_lib
               && initLib
               && getVersionMajor
               && getVersionMinor
               && getVersionRevision
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
               && addPedalEvent
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

    //! NOTE: There was a bug in ms_MuseSampler_add_track
    //! that prevented adding more than 1 track (fixed in v0.6)
    bool supportsMultipleTracks() const
    {
        return m_supportsMultipleTracks;
    }

private:
    void printApiStatus() const
    {
        LOGI() << "MuseSampler API status:"
               << "\n ms_get_version_major -" << reinterpret_cast<uint64_t>(getVersionMajor)
               << "\n ms_get_version_minor -" << reinterpret_cast<uint64_t>(getVersionMinor)
               << "\n ms_get_version_revision -" << reinterpret_cast<uint64_t>(getVersionRevision)
               << "\n ms_get_version_string -" << reinterpret_cast<uint64_t>(getVersionString)
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
               << "\n ms_Instrument_get_reverb_level - " << reinterpret_cast<uint64_t>(getReverbLevel)
               << "\n ms_Instrument_get_preset_list - " << reinterpret_cast<uint64_t>(getPresetList)
               << "\n ms_PresetList_get_next - " << reinterpret_cast<uint64_t>(getNextPreset)
               << "\n ms_MuseSampler_create_preset_change - " << reinterpret_cast<uint64_t>(createPresetChange)
               << "\n ms_MuseSampler_add_preset - " << reinterpret_cast<uint64_t>(addPreset)
               << "\n ms_get_text_articulations - " << reinterpret_cast<uint64_t>(getTextArticulations)
               << "\n ms_MuseSampler_add_track_text_articulation_event - " << reinterpret_cast<uint64_t>(addTextArticulationEvent)
               << "\n ms_MuseSampler_create - " << reinterpret_cast<uint64_t>(create)
               << "\n ms_MuseSampler_destroy - " << reinterpret_cast<uint64_t>(destroy)
               << "\n ms_MuseSampler_init - " << reinterpret_cast<uint64_t>(initSampler)
               << "\n ms_disable_reverb - " << reinterpret_cast<uint64_t>(disableReverb)
               << "\n ms_MuseSampler_clear_score - " << reinterpret_cast<uint64_t>(clearScore)
               << "\n ms_MuseSampler_add_track - " << reinterpret_cast<uint64_t>(addTrack)
               << "\n ms_MuseSampler_finalize_track - " << reinterpret_cast<uint64_t>(finalizeTrack)
               << "\n ms_MuseSampler_add_track_dynamics_event - " << reinterpret_cast<uint64_t>(addDynamicsEventInternal)
               << "\n ms_MuseSampler_add_track_dynamics_event_2 - " << reinterpret_cast<uint64_t>(addDynamicsEventInternal2)
               << "\n ms_MuseSampler_add_track_pedal_event - " << reinterpret_cast<uint64_t>(addPedalEventInternal)
               << "\n ms_MuseSampler_add_track_pedal_event_2 - " << reinterpret_cast<uint64_t>(addPedalEventInternal2)
               << "\n ms_MuseSampler_add_track_note_event - " << reinterpret_cast<uint64_t>(addNoteEventInternal)
               << "\n ms_MuseSampler_add_track_note_event_2 - " << reinterpret_cast<uint64_t>(addNoteEventInternal2)
               << "\n ms_MuseSampler_add_track_note_event_3 - " << reinterpret_cast<uint64_t>(addNoteEventInternal3)
               << "\n ms_MuseSampler_add_track_note_event_4 - " << reinterpret_cast<uint64_t>(addNoteEventInternal4)
               << "\n ms_MuseSampler_add_pitch_bend - " << reinterpret_cast<uint64_t>(addPitchBend)
               << "\n ms_MuseSampler_start_audition_note - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal)
               << "\n ms_MuseSampler_start_audition_note_2 - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal2)
               << "\n ms_MuseSampler_start_audition_note_3 - " << reinterpret_cast<uint64_t>(startAuditionNoteInternal3)
               << "\n ms_MuseSampler_stop_audition_note - " << reinterpret_cast<uint64_t>(stopAuditionNote)
               << "\n ms_MuseSampler_start_liveplay_mode - " << reinterpret_cast<uint64_t>(startLivePlayMode)
               << "\n ms_MuseSampler_stop_liveplay_mode - " << reinterpret_cast<uint64_t>(stopLivePlayMode)
               << "\n ms_MuseSampler_start_liveplay_note - " << reinterpret_cast<uint64_t>(startLivePlayNoteInternal)
               << "\n ms_MuseSampler_start_liveplay_note_2 - " << reinterpret_cast<uint64_t>(startLivePlayNoteInternal2)
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
    bool m_supportsMultipleTracks = false;
};
#endif

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}

#endif // MUSE_MUSESAMPLER_LIBHANDLER_H
