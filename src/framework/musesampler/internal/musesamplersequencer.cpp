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

#include "musesamplersequencer.h"

using namespace mu;
using namespace mu::musesampler;

static const std::unordered_map<mpe::ArticulationType, ms_NoteArticulation> ARTICULATION_TYPES = {
    { mpe::ArticulationType::Standard, ms_NoteArticulation_None },
    { mpe::ArticulationType::Staccato, ms_NoteArticulation_Staccato },
    { mpe::ArticulationType::Staccatissimo, ms_NoteArticulation_Staccatissimo },
    { mpe::ArticulationType::Accent, ms_NoteArticulation_Accent },
    { mpe::ArticulationType::Tenuto, ms_NoteArticulation_Tenuto },
    { mpe::ArticulationType::Marcato, ms_NoteArticulation_Marcato },
    { mpe::ArticulationType::Harmonic, ms_NoteArticulation_Harmonics },
    { mpe::ArticulationType::PalmMute, ms_NoteArticulation_PalmMute },
    { mpe::ArticulationType::Mute, ms_NoteArticulation_Mute },
    { mpe::ArticulationType::Legato, ms_NoteArticulation_Slur },
    { mpe::ArticulationType::Trill, ms_NoteArticulation_Trill },
    { mpe::ArticulationType::TrillBaroque, ms_NoteArticulation_Trill },

    { mpe::ArticulationType::Arpeggio, ms_NoteArticulation_ArpeggioUp },
    { mpe::ArticulationType::ArpeggioUp, ms_NoteArticulation_ArpeggioUp },
    { mpe::ArticulationType::ArpeggioDown, ms_NoteArticulation_ArpeggioDown },

    { mpe::ArticulationType::Tremolo8th, ms_NoteArticulation_Tremolo1 },
    { mpe::ArticulationType::Tremolo16th, ms_NoteArticulation_Tremolo2 },
    { mpe::ArticulationType::Tremolo32nd, ms_NoteArticulation_Tremolo3 },
    { mpe::ArticulationType::Tremolo64th, ms_NoteArticulation_Tremolo3 },
    { mpe::ArticulationType::TremoloBuzz, ms_NoteArticulation_BuzzTremolo },

    { mpe::ArticulationType::DiscreteGlissando, ms_NoteArticulation_Glissando },
    { mpe::ArticulationType::ContinuousGlissando, ms_NoteArticulation_Portamento },
    { mpe::ArticulationType::Slide, ms_NoteArticulation_Portamento },

    { mpe::ArticulationType::Scoop, ms_NoteArticulation_Scoop },
    { mpe::ArticulationType::Plop, ms_NoteArticulation_Plop },
    { mpe::ArticulationType::Doit, ms_NoteArticulation_Doit },
    { mpe::ArticulationType::Fall, ms_NoteArticulation_Fall },
    { mpe::ArticulationType::PreAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { mpe::ArticulationType::PostAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { mpe::ArticulationType::Acciaccatura, ms_NoteArticulation_Acciaccatura },

    { mpe::ArticulationType::Open, ms_NoteArticulation_Open },

    { mpe::ArticulationType::Pizzicato, ms_NoteArticulation_Pizzicato },
    { mpe::ArticulationType::SnapPizzicato, ms_NoteArticulation_SnapPizzicato },

    { mpe::ArticulationType::UpperMordent, ms_NoteArticulation_MordentWhole },
    { mpe::ArticulationType::UpMordent, ms_NoteArticulation_MordentWhole },
    { mpe::ArticulationType::LowerMordent, ms_NoteArticulation_MordentInvertedWhole },
    { mpe::ArticulationType::DownMordent, ms_NoteArticulation_MordentInvertedWhole },
    { mpe::ArticulationType::Turn, ms_NoteArticulation_TurnWholeWhole },
    { mpe::ArticulationType::InvertedTurn, ms_NoteArticulation_TurnInvertedWholeWhole },

    { mpe::ArticulationType::ColLegno, ms_NoteArticulation_ColLegno },
    { mpe::ArticulationType::SulTasto, ms_NoteArticulation_SulTasto },
    { mpe::ArticulationType::SulPont, ms_NoteArticulation_SulPonticello },
};

static const std::unordered_map<mpe::ArticulationType, ms_NoteHead> NOTEHEAD_TYPES = {
    { mpe::ArticulationType::CrossNote, ms_NoteHead_XNote },
    { mpe::ArticulationType::CrossLargeNote, ms_NoteHead_LargeX },
    { mpe::ArticulationType::CrossOrnateNote, ms_NoteHead_OrnateXNote },
    { mpe::ArticulationType::GhostNote, ms_NoteHead_Ghost },
    { mpe::ArticulationType::CircleNote, ms_NoteHead_Circle },
    { mpe::ArticulationType::CircleCrossNote, ms_NoteHead_CircleXNote },
    { mpe::ArticulationType::CircleDotNote, ms_NoteHead_CircleDot },
    { mpe::ArticulationType::TriangleLeftNote, ms_NoteHead_Triangle },
    { mpe::ArticulationType::TriangleRightNote, ms_NoteHead_TriangleRight },
    { mpe::ArticulationType::TriangleUpNote, ms_NoteHead_TriangleUp },
    { mpe::ArticulationType::TriangleDownNote, ms_NoteHead_TriangleDown },
    { mpe::ArticulationType::TriangleRoundDownNote, ms_NoteHead_TriangleRoundDown },
    { mpe::ArticulationType::DiamondNote, ms_NoteHead_Diamond },
    { mpe::ArticulationType::MoonNote, ms_NoteHead_FlatTop },
    { mpe::ArticulationType::PlusNote, ms_NoteHead_Plus },
    { mpe::ArticulationType::SlashNote, ms_NoteHead_Slash },
    { mpe::ArticulationType::SquareNote, ms_NoteHead_Square },
    { mpe::ArticulationType::SlashedBackwardsNote, ms_NoteHead_SlashRightFilled },
    { mpe::ArticulationType::SlashedForwardsNote, ms_NoteHead_SlashLeftFilled },
};

void MuseSamplerSequencer::init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, IMuseSamplerTracks* tracks,
                                std::string&& defaultPresetCode)
{
    m_samplerLib = samplerLib;
    m_sampler = sampler;
    m_tracks = tracks;
    m_defaultPresetCode = std::move(defaultPresetCode);
}

void MuseSamplerSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::PlaybackParamMap& params)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    m_offStreamCache.clear();
    parseOffStreamParams(params, m_offStreamCache.presets, m_offStreamCache.textArticulation);

    const char* presets_cstr = m_offStreamCache.presets.empty() ? m_defaultPresetCode.c_str() : m_offStreamCache.presets.c_str();
    const char* textArticulation_cstr = m_offStreamCache.textArticulation.c_str();

    for (const auto& pair : events) {
        for (const auto& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            ms_Track track = resolveTrack(noteEvent.arrangementCtx().staffLayerIndex);
            IF_ASSERT_FAILED(track) {
                continue;
            }

            mpe::timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
            mpe::timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

            int pitch{};
            int centsOffset{};
            pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, pitch, centsOffset);

            ms_NoteArticulation articulationFlag = ms_NoteArticulation_None;
            ms_NoteHead notehead = ms_NoteHead_Normal;
            parseArticulations(noteEvent.expressionCtx().articulations, articulationFlag, notehead);

            AuditionStartNoteEvent noteOn;
            noteOn.msEvent = { pitch, centsOffset, articulationFlag, notehead, 0.5, presets_cstr, textArticulation_cstr };
            noteOn.msTrack = track;
            m_offStreamEvents[timestampFrom].emplace(std::move(noteOn));

            AuditionStopNoteEvent noteOff;
            noteOff.msEvent = { pitch };
            noteOff.msTrack = track;
            m_offStreamEvents[timestampTo].emplace(std::move(noteOff));
        }
    }

    updateOffSequenceIterator();
}

void MuseSamplerSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelMap& dynamics,
                                                  const mpe::PlaybackParamMap& params)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    clearAllTracks();

    loadParams(params);
    loadNoteEvents(events);
    loadDynamicEvents(dynamics);

    finalizeAllTracks();
}

void MuseSamplerSequencer::clearAllTracks()
{
    m_layerIdxToTrackIdx.clear();

    for (ms_Track track : allTracks()) {
        m_samplerLib->clearTrack(m_sampler, track);
    }
}

void MuseSamplerSequencer::finalizeAllTracks()
{
    for (ms_Track track : allTracks()) {
        m_samplerLib->finalizeTrack(m_sampler, track);
    }
}

ms_Track MuseSamplerSequencer::resolveTrack(mpe::staff_layer_idx_t staffLayerIdx)
{
    const TrackList& tracks = m_tracks->allTracks();

    layer_idx_t layerIdx = staffLayerIdx;
    auto it = m_layerIdxToTrackIdx.find(layerIdx);

    // A track has already been assigned to this layer
    if (it != m_layerIdxToTrackIdx.end()) {
        if (it->second < tracks.size()) {
            return tracks.at(it->second);
        }

        m_layerIdxToTrackIdx.erase(it);
    }

    // Try to find a free track
    std::unordered_set<track_idx_t> assignedTracks;
    for (const auto& pair: m_layerIdxToTrackIdx) {
        assignedTracks.insert(pair.second);
    }

    for (track_idx_t trackIdx = 0; trackIdx < tracks.size(); ++trackIdx) {
        if (!mu::contains(assignedTracks, trackIdx)) {
            m_layerIdxToTrackIdx.emplace(layerIdx, trackIdx);
            return tracks.at(trackIdx);
        }
    }

    // Add a new track
    ms_Track newTrack = m_tracks->addTrack();
    if (newTrack) {
        m_layerIdxToTrackIdx.emplace(layerIdx, tracks.size());
        return newTrack;
    }

    // Could not add the track. Use 1st track as a fallback
    if (!tracks.empty()) {
        m_layerIdxToTrackIdx.emplace(layerIdx, 0);
        return tracks.front();
    }

    UNREACHABLE;
    return nullptr;
}

const TrackList& MuseSamplerSequencer::allTracks() const
{
    IF_ASSERT_FAILED(m_tracks) {
        static const TrackList dummy;
        return dummy;
    }

    return m_tracks->allTracks();
}

void MuseSamplerSequencer::loadParams(const mpe::PlaybackParamMap& params)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    for (const auto& paramsPair : params) {
        std::unordered_map<mpe::staff_layer_idx_t, std::vector<std::string> > soundPresets;

        for (const mpe::PlaybackParam& param : paramsPair.second) {
            if (param.code == mpe::SOUND_PRESET_PARAM_CODE) {
                soundPresets[param.staffLayerIndex].emplace_back(param.val.toString());
            } else if (param.code == mpe::PLAY_TECHNIQUE_PARAM_CODE) {
                ms_Track track = resolveTrack(param.staffLayerIndex);
                IF_ASSERT_FAILED(track) {
                    continue;
                }

                addTextArticulation(param.val.toString(), paramsPair.first, track);
            }
        }

        for (const auto& presetsPair : soundPresets) {
            ms_Track track = resolveTrack(presetsPair.first);
            IF_ASSERT_FAILED(track) {
                continue;
            }

            addPresets(presetsPair.second, paramsPair.first, track);
        }
    }
}

void MuseSamplerSequencer::loadNoteEvents(const mpe::PlaybackEventsMap& changes)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    for (const auto& pair : changes) {
        for (const auto& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            addNoteEvent(noteEvent);
        }
    }
}

void MuseSamplerSequencer::loadDynamicEvents(const mpe::DynamicLevelMap& changes)
{
    for (ms_Track track : allTracks()) {
        for (const auto& pair : changes) {
            m_samplerLib->addDynamicsEvent(m_sampler, track, pair.first, dynamicLevelRatio(pair.second));
        }
    }
}

void MuseSamplerSequencer::addNoteEvent(const mpe::NoteEvent& noteEvent)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_tracks) {
        return;
    }

    ms_Track track = resolveTrack(noteEvent.arrangementCtx().staffLayerIndex);
    IF_ASSERT_FAILED(track) {
        return;
    }

    mpe::voice_layer_idx_t voiceIdx = noteEvent.arrangementCtx().voiceLayerIndex;

    for (const auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);

        if (art.first == mpe::ArticulationType::Pedal) {
            // Pedal on:
            m_samplerLib->addPedalEvent(m_sampler, track, art.second.meta.timestamp, 1.0);
            // Pedal off:
            m_samplerLib->addPedalEvent(m_sampler, track, art.second.meta.timestamp + art.second.meta.overallDuration, 0.0);
        }

        if (m_samplerLib->isRangedArticulation(ms_art)) {
            // If this starts an articulation range, indicate the start
            if (art.second.occupiedFrom == 0 && art.second.occupiedTo != mpe::HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeStart(m_sampler, track, voiceIdx, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range start";
                }
            }
        }
    }

    musesampler::NoteEvent event;
    event._voice = voiceIdx;
    event._location_us = noteEvent.arrangementCtx().nominalTimestamp;
    event._duration_us = noteEvent.arrangementCtx().nominalDuration;
    event._tempo = noteEvent.arrangementCtx().bps * 60.0; // API expects BPM
    event._articulation = ms_NoteArticulation_None;
    event._notehead = ms_NoteHead_Normal;

    pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, event._pitch, event._offset_cents);
    parseArticulations(noteEvent.expressionCtx().articulations, event._articulation, event._notehead);

    long long noteEventId = 0;

    if (!m_samplerLib->addNoteEvent(m_sampler, track, event, noteEventId)) {
        LOGE() << "Unable to add event for track";
    }

    for (auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);
        if (m_samplerLib->isRangedArticulation(ms_art)) {
            // If this ends an articulation range, indicate the end
            if (art.second.occupiedFrom != 0 && art.second.occupiedTo == mpe::HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeEnd(m_sampler, track, voiceIdx, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range end";
                }
            }
        }
    }

    if (noteEvent.expressionCtx().articulations.contains(mpe::ArticulationType::Multibend)) {
        addPitchBends(noteEvent, noteEventId, track);
    }

    if (noteEvent.expressionCtx().articulations.contains(mpe::ArticulationType::Vibrato)) {
        addVibrato(noteEvent, noteEventId, track);
    }
}

void MuseSamplerSequencer::addTextArticulation(const std::string& articulationCode, long long startUs, ms_Track track)
{
    ms_TextArticulationEvent evt;
    evt._start_us = startUs;

    if (articulationCode != mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
        evt._articulation = articulationCode.c_str();
    } else {
        evt._articulation = ""; // resets the active articulation
    }

    m_samplerLib->addTextArticulationEvent(m_sampler, track, evt);
}

void MuseSamplerSequencer::addPresets(const std::vector<std::string>& presets, long long startUs, ms_Track track)
{
    if (presets.empty()) {
        return;
    }

    ms_PresetChange presetChange = m_samplerLib->createPresetChange(m_sampler, track, startUs);

    for (const std::string& presetCode : presets) {
        m_samplerLib->addPreset(m_sampler, track, presetChange, presetCode.c_str());
    }
}

void MuseSamplerSequencer::addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track)
{
    if (!m_samplerLib->addPitchBend) { // added in 0.5
        return;
    }

    mpe::duration_t duration = noteEvent.arrangementCtx().actualDuration;
    const mpe::PitchCurve& pitchCurve = noteEvent.pitchCtx().pitchCurve;

    for (auto it = pitchCurve.begin(); it != pitchCurve.end(); ++it) {
        auto nextIt = std::next(it);
        if (nextIt == pitchCurve.end()) {
            continue;
        }

        if (nextIt->second == it->second) {
            continue;
        }

        long long currOffsetStart = duration * mpe::percentageToFactor(it->first);
        long long nextOffsetStart = duration * mpe::percentageToFactor(nextIt->first);

        ms_PitchBendInfo pitchBend;
        pitchBend.event_id = noteEventId;
        pitchBend._start_us = currOffsetStart;
        pitchBend._duration_us = nextOffsetStart - currOffsetStart;
        pitchBend._offset_cents = pitchLevelToCents(nextIt->second);
        pitchBend._type = PitchBend_Bezier;

        m_samplerLib->addPitchBend(m_sampler, track, pitchBend);
    }
}

void MuseSamplerSequencer::addVibrato(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track)
{
    if (!m_samplerLib->addVibrato) { // added in 0.5
        return;
    }

    mpe::duration_t duration = noteEvent.arrangementCtx().actualDuration;
    // stand-in data before actual mpe support
    constexpr auto MAX_VIBRATO_STARTOFFSET_US = (int64_t)0.1 * 1000000;
    // stand-in data before actual mpe support
    constexpr int VIBRATO_DEPTH_CENTS = 23;

    ms_VibratoInfo vibrato;
    vibrato.event_id = noteEventId;
    vibrato._start_us = std::min((int64_t)(duration * 0.1), MAX_VIBRATO_STARTOFFSET_US);
    vibrato._duration_us = duration;
    vibrato._depth_cents = VIBRATO_DEPTH_CENTS;

    m_samplerLib->addVibrato(m_sampler, track, vibrato);
}

void MuseSamplerSequencer::pitchAndTuning(const mpe::pitch_level_t nominalPitch, int& pitch, int& centsOffset) const
{
    static constexpr mpe::pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 0);
    static constexpr int MIN_SUPPORTED_NOTE = 12; // equivalent for C0
    static constexpr mpe::pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 8);
    static constexpr int MAX_SUPPORTED_NOTE = 108; // equivalent for C8

    if (nominalPitch <= MIN_SUPPORTED_PITCH_LEVEL) {
        pitch = MIN_SUPPORTED_NOTE;
        centsOffset = 0;
        return;
    }

    if (nominalPitch >= MAX_SUPPORTED_PITCH_LEVEL) {
        pitch = MAX_SUPPORTED_NOTE;
        centsOffset = 0;
        return;
    }

    // Get midi pitch
    float stepCount = MIN_SUPPORTED_NOTE + ((nominalPitch - MIN_SUPPORTED_PITCH_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));
    pitch = RealRound(stepCount, 0);

    // Get tuning offset
    int semitonesCount = pitch - MIN_SUPPORTED_NOTE;
    mpe::pitch_level_t tuningPitchLevel = nominalPitch - (semitonesCount * mpe::PITCH_LEVEL_STEP);
    centsOffset = pitchLevelToCents(tuningPitchLevel);
}

int MuseSamplerSequencer::pitchLevelToCents(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr float CONVERT_TO_CENTS = (100.f / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return pitchLevel * CONVERT_TO_CENTS;
}

double MuseSamplerSequencer::dynamicLevelRatio(const mpe::dynamic_level_t level) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_DYNAMIC_LEVEL = mpe::dynamicLevelFromType(mpe::DynamicType::ppppppppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_DYNAMIC_LEVEL = mpe::dynamicLevelFromType(mpe::DynamicType::fffffffff);

    // Piecewise linear simple scaling to expected MuseSampler values:
    static const std::list<std::pair<mpe::dynamic_level_t, double> > conversionMap = {
        { MIN_SUPPORTED_DYNAMIC_LEVEL, 0.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::ppppp), 1.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::pppp), 2.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::ppp), 4.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::pp), 8.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::p), 16.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::mp), 32.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::mf), 64.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::f), 96.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::ff), 120.0 / 127.0 },
        { mpe::dynamicLevelFromType(mpe::DynamicType::fff), 127.0 / 127.0 },
        { MAX_SUPPORTED_DYNAMIC_LEVEL, 127.0 / 127.0 }
    };

    auto prev_level = conversionMap.begin();
    auto last_level = conversionMap.end();
    auto level_it = std::next(prev_level);
    while (level_it != last_level) {
        if (level >= prev_level->first && level <= level_it->first) {
            auto alpha = static_cast<double>(level - prev_level->first) / static_cast<double>(level_it->first - prev_level->first);
            return alpha * level_it->second + (1.0 - alpha) * prev_level->second;
        }
        prev_level = level_it;
        level_it = std::next(level_it);
    }
    LOGE() << "Out of range dynamic value found!";
    return 48.0 / 127.0;
}

ms_NoteArticulation MuseSamplerSequencer::convertArticulationType(mpe::ArticulationType articulation) const
{
    if (auto search = ARTICULATION_TYPES.find(articulation); search != ARTICULATION_TYPES.cend()) {
        return static_cast<ms_NoteArticulation>(search->second);
    }
    return static_cast<ms_NoteArticulation>(0);
}

void MuseSamplerSequencer::parseArticulations(const mpe::ArticulationMap& articulations,
                                              ms_NoteArticulation& articulationFlag, ms_NoteHead& notehead) const
{
    uint64_t artFlag = 0;

    for (const auto& pair : articulations) {
        auto artIt = ARTICULATION_TYPES.find(pair.first);
        if (artIt != ARTICULATION_TYPES.cend()) {
            artFlag |= convertArticulationType(pair.first);
            continue;
        }

        auto headIt = NOTEHEAD_TYPES.find(pair.first);
        if (headIt != NOTEHEAD_TYPES.cend()) {
            notehead = headIt->second;
        }
    }

    articulationFlag = static_cast<ms_NoteArticulation>(artFlag);
}

void MuseSamplerSequencer::parseOffStreamParams(const mpe::PlaybackParamMap& params, std::string& presets,
                                                std::string& textArticulation) const
{
    if (params.empty()) {
        return;
    }

    StringList presetList;

    for (const auto& pair : params) {
        for (const mpe::PlaybackParam& param : pair.second) {
            if (param.code == mpe::SOUND_PRESET_PARAM_CODE) {
                presetList.emplace_back(String::fromStdString(param.val.toString()));
            } else if (param.code == mpe::PLAY_TECHNIQUE_PARAM_CODE) {
                textArticulation = param.val.toString();
            }
        }
    }

    presets = presetList.join(u"|").toStdString();
}
