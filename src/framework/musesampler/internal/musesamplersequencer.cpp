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

#include "apitypes.h"

using namespace muse;
using namespace muse::musesampler;
using namespace muse::mpe;

static const std::unordered_map<ArticulationType, ms_NoteArticulation> ARTICULATION_TYPES = {
    { ArticulationType::Standard, ms_NoteArticulation_None },
    { ArticulationType::Staccato, ms_NoteArticulation_Staccato },
    { ArticulationType::Staccatissimo, ms_NoteArticulation_Staccatissimo },
    { ArticulationType::Accent, ms_NoteArticulation_Accent },
    { ArticulationType::Tenuto, ms_NoteArticulation_Tenuto },
    { ArticulationType::Marcato, ms_NoteArticulation_Marcato },
    { ArticulationType::Harmonic, ms_NoteArticulation_Harmonics },
    { ArticulationType::PalmMute, ms_NoteArticulation_PalmMute },
    { ArticulationType::Mute, ms_NoteArticulation_Mute },
    { ArticulationType::Legato, ms_NoteArticulation_Slur },
    { ArticulationType::Trill, ms_NoteArticulation_Trill },
    { ArticulationType::TrillBaroque, ms_NoteArticulation_Trill },

    { ArticulationType::Arpeggio, ms_NoteArticulation_ArpeggioUp },
    { ArticulationType::ArpeggioUp, ms_NoteArticulation_ArpeggioUp },
    { ArticulationType::ArpeggioDown, ms_NoteArticulation_ArpeggioDown },

    { ArticulationType::Tremolo8th, ms_NoteArticulation_Tremolo1 },
    { ArticulationType::Tremolo16th, ms_NoteArticulation_Tremolo2 },
    { ArticulationType::Tremolo32nd, ms_NoteArticulation_Tremolo3 },
    { ArticulationType::Tremolo64th, ms_NoteArticulation_Tremolo3 },
    { ArticulationType::TremoloBuzz, ms_NoteArticulation_BuzzTremolo },

    { ArticulationType::DiscreteGlissando, ms_NoteArticulation_Glissando },
    { ArticulationType::ContinuousGlissando, ms_NoteArticulation_Portamento },
    { ArticulationType::Slide, ms_NoteArticulation_Portamento },

    { ArticulationType::Scoop, ms_NoteArticulation_Scoop },
    { ArticulationType::Plop, ms_NoteArticulation_Plop },
    { ArticulationType::Doit, ms_NoteArticulation_Doit },
    { ArticulationType::Fall, ms_NoteArticulation_Fall },
    { ArticulationType::PreAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { ArticulationType::PostAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { ArticulationType::Acciaccatura, ms_NoteArticulation_Acciaccatura },

    { ArticulationType::Open, ms_NoteArticulation_Open },

    { ArticulationType::Pizzicato, ms_NoteArticulation_Pizzicato },
    { ArticulationType::SnapPizzicato, ms_NoteArticulation_SnapPizzicato },

    { ArticulationType::UpperMordent, ms_NoteArticulation_MordentWhole },
    { ArticulationType::UpMordent, ms_NoteArticulation_MordentWhole },
    { ArticulationType::LowerMordent, ms_NoteArticulation_MordentInvertedWhole },
    { ArticulationType::DownMordent, ms_NoteArticulation_MordentInvertedWhole },
    { ArticulationType::Turn, ms_NoteArticulation_TurnWholeWhole },
    { ArticulationType::InvertedTurn, ms_NoteArticulation_TurnInvertedWholeWhole },

    { ArticulationType::ColLegno, ms_NoteArticulation_ColLegno },
    { ArticulationType::SulTasto, ms_NoteArticulation_SulTasto },
    { ArticulationType::SulPont, ms_NoteArticulation_SulPonticello },
};

static const std::unordered_map<ArticulationType, ms_NoteHead> NOTEHEAD_TYPES = {
    { ArticulationType::CrossNote, ms_NoteHead_XNote },
    { ArticulationType::CrossLargeNote, ms_NoteHead_LargeX },
    { ArticulationType::CrossOrnateNote, ms_NoteHead_OrnateXNote },
    { ArticulationType::GhostNote, ms_NoteHead_Ghost },
    { ArticulationType::CircleNote, ms_NoteHead_Circle },
    { ArticulationType::CircleCrossNote, ms_NoteHead_CircleXNote },
    { ArticulationType::CircleDotNote, ms_NoteHead_CircleDot },
    { ArticulationType::TriangleLeftNote, ms_NoteHead_Triangle },
    { ArticulationType::TriangleRightNote, ms_NoteHead_TriangleRight },
    { ArticulationType::TriangleUpNote, ms_NoteHead_TriangleUp },
    { ArticulationType::TriangleDownNote, ms_NoteHead_TriangleDown },
    { ArticulationType::TriangleRoundDownNote, ms_NoteHead_TriangleRoundDown },
    { ArticulationType::DiamondNote, ms_NoteHead_Diamond },
    { ArticulationType::MoonNote, ms_NoteHead_FlatTop },
    { ArticulationType::PlusNote, ms_NoteHead_Plus },
    { ArticulationType::SlashNote, ms_NoteHead_Slash },
    { ArticulationType::SquareNote, ms_NoteHead_Square },
    { ArticulationType::SlashedBackwardsNote, ms_NoteHead_SlashRightFilled },
    { ArticulationType::SlashedForwardsNote, ms_NoteHead_SlashLeftFilled },
};

void MuseSamplerSequencer::init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, IMuseSamplerTracks* tracks,
                                std::string&& defaultPresetCode)
{
    m_samplerLib = samplerLib;
    m_sampler = sampler;
    m_tracks = tracks;
    m_defaultPresetCode = std::move(defaultPresetCode);
}

void MuseSamplerSequencer::updateOffStreamEvents(const PlaybackEventsMap& events, const PlaybackParamList& params)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    parseOffStreamParams(params, m_offStreamCache);

    const char* presets_cstr = m_offStreamCache.presets.empty() ? m_defaultPresetCode.c_str() : m_offStreamCache.presets.c_str();
    const char* textArticulation_cstr = m_offStreamCache.textArticulation.c_str();
    const char* syllable_cstr = m_offStreamCache.syllable.c_str();

    for (const auto& pair : events) {
        for (const auto& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            const mpe::ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();

            layer_idx_t layerIdx = makeLayerIdx(arrangementCtx.staffLayerIndex, arrangementCtx.voiceLayerIndex);
            ms_Track track = resolveTrack(layerIdx);
            IF_ASSERT_FAILED(track) {
                continue;
            }

            AuditionStartNoteEvent noteOn;
            pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, noteOn.msEvent._pitch, noteOn.msEvent._offset_cents);
            parseArticulations(noteEvent.expressionCtx().articulations, noteOn.msEvent._articulation, noteOn.msEvent._notehead);
            noteOn.msEvent._dynamics = 0.5;
            noteOn.msEvent._active_presets = presets_cstr;
            noteOn.msEvent._active_text_articulation = textArticulation_cstr;
            noteOn.msEvent._active_syllable = syllable_cstr;
            noteOn.msEvent._articulation_text_starts_at_note = m_offStreamCache.textArticulationStartsAtNote;
            noteOn.msEvent._syllable_starts_at_note = m_offStreamCache.syllableStartsAtNote;
            noteOn.msTrack = track;

            timestamp_t timestampFrom = arrangementCtx.actualTimestamp;
            m_offStreamEvents[arrangementCtx.actualTimestamp].emplace(std::move(noteOn));

            AuditionStopNoteEvent noteOff;
            noteOff.msEvent = { noteOn.msEvent._pitch };
            noteOff.msTrack = track;

            timestamp_t timestampTo = timestampFrom + arrangementCtx.actualDuration;
            m_offStreamEvents[timestampTo].emplace(std::move(noteOff));
        }
    }

    updateOffSequenceIterator();
}

void MuseSamplerSequencer::updateMainStreamEvents(const PlaybackEventsMap& events, const DynamicLevelLayers& dynamics,
                                                  const PlaybackParamLayers& params)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    clearAllTracks();

    loadNoteEvents(events);
    loadParams(params);
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

ms_Track MuseSamplerSequencer::resolveTrack(layer_idx_t layerIdx)
{
    const TrackList& tracks = m_tracks->allTracks();
    auto it = m_layerIdxToTrackIdx.find(layerIdx);

    // A track has already been assigned to this layer
    if (it != m_layerIdxToTrackIdx.end()) {
        if (it->second < tracks.size()) {
            return tracks.at(it->second);
        }

        ASSERT_X("Invalid track index");
        m_layerIdxToTrackIdx.erase(it);
    }

    // Try to find a free track
    std::unordered_set<track_idx_t> assignedTracks(m_layerIdxToTrackIdx.size());
    for (const auto& pair: m_layerIdxToTrackIdx) {
        assignedTracks.insert(pair.second);
    }

    for (track_idx_t trackIdx = 0; trackIdx < tracks.size(); ++trackIdx) {
        if (!muse::contains(assignedTracks, trackIdx)) {
            m_layerIdxToTrackIdx.emplace(layerIdx, trackIdx);
            return tracks.at(trackIdx);
        }
    }

    // Add a new track
    ms_Track newTrack = m_tracks->addTrack();
    if (newTrack) {
        m_layerIdxToTrackIdx.emplace(layerIdx, tracks.size() - 1);
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

ms_Track MuseSamplerSequencer::findTrack(layer_idx_t layerIdx) const
{
    auto it = m_layerIdxToTrackIdx.find(layerIdx);
    if (it == m_layerIdxToTrackIdx.end()) {
        return nullptr;
    }

    const TrackList& tracks = m_tracks->allTracks();
    IF_ASSERT_FAILED(it->second < tracks.size()) {
        return nullptr;
    }

    return tracks.at(it->second);
}

const TrackList& MuseSamplerSequencer::allTracks() const
{
    IF_ASSERT_FAILED(m_tracks) {
        static const TrackList dummy;
        return dummy;
    }

    return m_tracks->allTracks();
}

void MuseSamplerSequencer::loadParams(const PlaybackParamLayers& changes)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    for (const auto& layer : changes) {
        ms_Track track = findTrack(layer.first);
        if (!track) {
            continue;
        }

        for (const auto& params : layer.second) {
            StringList soundPresets;

            for (const PlaybackParam& param : params.second) {
                switch (param.type) {
                case PlaybackParam::SoundPreset:
                    soundPresets.push_back(param.val);
                    break;
                case PlaybackParam::PlayingTechnique:
                    addTextArticulation(param.val, params.first, track);
                    break;
                case PlaybackParam::Syllable:
                    addSyllable(param.val, params.first, track);
                    break;
                case PlaybackParam::Undefined:
                    UNREACHABLE;
                    break;
                }
            }

            addPresets(soundPresets, params.first, track);
        }
    }
}

void MuseSamplerSequencer::loadNoteEvents(const PlaybackEventsMap& changes)
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

void MuseSamplerSequencer::loadDynamicEvents(const DynamicLevelLayers& changes)
{
    for (const auto& layer : changes) {
        ms_Track track = findTrack(layer.first);
        if (!track) {
            continue;
        }

        for (const auto& dynamic : layer.second) {
            m_samplerLib->addDynamicsEvent(m_sampler, track, dynamic.first, dynamicLevelRatio(dynamic.second));
        }
    }
}

void MuseSamplerSequencer::addNoteEvent(const mpe::NoteEvent& noteEvent)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_tracks) {
        return;
    }

    const mpe::ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();
    voice_layer_idx_t voiceIdx = arrangementCtx.voiceLayerIndex;
    layer_idx_t layerIdx = makeLayerIdx(arrangementCtx.staffLayerIndex, voiceIdx);

    ms_Track track = resolveTrack(layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    for (const auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);

        if (art.first == ArticulationType::Pedal) {
            // Pedal on:
            m_samplerLib->addPedalEvent(m_sampler, track, art.second.meta.timestamp, 1.0);
            // Pedal off:
            m_samplerLib->addPedalEvent(m_sampler, track, art.second.meta.timestamp + art.second.meta.overallDuration, 0.0);
        }

        if (m_samplerLib->isRangedArticulation(ms_art)) {
            // If this starts an articulation range, indicate the start
            if (art.second.occupiedFrom == 0 && art.second.occupiedTo != HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeStart(m_sampler, track, voiceIdx, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range start";
                }
            }
        }
    }

    musesampler::NoteEvent event;
    event._voice = voiceIdx;
    event._location_us = arrangementCtx.nominalTimestamp;
    event._duration_us = arrangementCtx.nominalDuration;
    event._tempo = arrangementCtx.bps * 60.0; // API expects BPM

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
            if (art.second.occupiedFrom != 0 && art.second.occupiedTo == HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeEnd(m_sampler, track, voiceIdx, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range end";
                }
            }
        }
    }

    if (noteEvent.expressionCtx().articulations.contains(ArticulationType::Multibend)) {
        addPitchBends(noteEvent, noteEventId, track);
    }

    if (noteEvent.expressionCtx().articulations.contains(ArticulationType::Vibrato)) {
        addVibrato(noteEvent, noteEventId, track);
    }
}

void MuseSamplerSequencer::addTextArticulation(const String& articulationCode, long long startUs, ms_Track track)
{
    std::string str = articulationCode.toStdString();

    ms_TextArticulationEvent evt;
    evt._start_us = startUs;

    if (articulationCode != ORDINARY_PLAYING_TECHNIQUE_CODE) {
        evt._articulation = str.c_str();
    } else {
        evt._articulation = ""; // resets the active articulation
    }

    m_samplerLib->addTextArticulationEvent(m_sampler, track, evt);
}

void MuseSamplerSequencer::addPresets(const StringList& presets, long long startUs, ms_Track track)
{
    if (presets.empty()) {
        return;
    }

    ms_PresetChange presetChange = m_samplerLib->createPresetChange(m_sampler, track, startUs);

    for (const String& presetCode : presets) {
        m_samplerLib->addPreset(m_sampler, track, presetChange, presetCode.toStdString().c_str());
    }
}

void MuseSamplerSequencer::addSyllable(const String& syllable, long long positionUs, ms_Track track)
{
    if (syllable.empty()) {
        return;
    }

    std::string str = syllable.toStdString();

    ms_SyllableEvent evt;
    evt._position_us = positionUs;
    evt._text = str.c_str();

    m_samplerLib->addSyllableEvent(m_sampler, track, evt);
}

void MuseSamplerSequencer::addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track)
{
    duration_t duration = noteEvent.arrangementCtx().actualDuration;
    const PitchCurve& pitchCurve = noteEvent.pitchCtx().pitchCurve;

    for (auto it = pitchCurve.begin(); it != pitchCurve.end(); ++it) {
        auto nextIt = std::next(it);
        if (nextIt == pitchCurve.end()) {
            continue;
        }

        if (nextIt->second == it->second) {
            continue;
        }

        long long currOffsetStart = duration * percentageToFactor(it->first);
        long long nextOffsetStart = duration * percentageToFactor(nextIt->first);

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
    duration_t duration = noteEvent.arrangementCtx().actualDuration;
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

void MuseSamplerSequencer::pitchAndTuning(const pitch_level_t nominalPitch, int& pitch, int& centsOffset) const
{
    static constexpr pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = pitchLevel(PitchClass::C, 0);
    static constexpr int MIN_SUPPORTED_NOTE = 12; // equivalent for C0
    static constexpr pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = pitchLevel(PitchClass::C, 8);
    static constexpr int MAX_SUPPORTED_NOTE = 108; // equivalent for C8

    pitch = 0;
    centsOffset = 0;

    if (nominalPitch <= MIN_SUPPORTED_PITCH_LEVEL) {
        pitch = MIN_SUPPORTED_NOTE;
        return;
    }

    if (nominalPitch >= MAX_SUPPORTED_PITCH_LEVEL) {
        pitch = MAX_SUPPORTED_NOTE;
        return;
    }

    // Get midi pitch
    float stepCount = MIN_SUPPORTED_NOTE + ((nominalPitch - MIN_SUPPORTED_PITCH_LEVEL) / static_cast<float>(PITCH_LEVEL_STEP));
    pitch = RealRound(stepCount, 0);

    // Get tuning offset
    int semitonesCount = pitch - MIN_SUPPORTED_NOTE;
    pitch_level_t tuningPitchLevel = nominalPitch - (semitonesCount * PITCH_LEVEL_STEP);
    centsOffset = pitchLevelToCents(tuningPitchLevel);
}

int MuseSamplerSequencer::pitchLevelToCents(const pitch_level_t pitchLevel) const
{
    static constexpr float CONVERT_TO_CENTS = (100.f / static_cast<float>(PITCH_LEVEL_STEP));

    return pitchLevel * CONVERT_TO_CENTS;
}

double MuseSamplerSequencer::dynamicLevelRatio(const dynamic_level_t level) const
{
    static constexpr dynamic_level_t MIN_SUPPORTED_DYNAMIC_LEVEL = dynamicLevelFromType(DynamicType::ppppppppp);
    static constexpr dynamic_level_t MAX_SUPPORTED_DYNAMIC_LEVEL = dynamicLevelFromType(DynamicType::fffffffff);

    // Piecewise linear simple scaling to expected MuseSampler values:
    static const std::list<std::pair<dynamic_level_t, double> > conversionMap = {
        { MIN_SUPPORTED_DYNAMIC_LEVEL, 0.0 },
        { dynamicLevelFromType(DynamicType::ppppp), 1.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::pppp), 2.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::ppp), 4.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::pp), 8.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::p), 16.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::mp), 32.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::mf), 64.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::f), 96.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::ff), 120.0 / 127.0 },
        { dynamicLevelFromType(DynamicType::fff), 127.0 / 127.0 },
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

ms_NoteArticulation MuseSamplerSequencer::convertArticulationType(ArticulationType articulation) const
{
    if (auto search = ARTICULATION_TYPES.find(articulation); search != ARTICULATION_TYPES.cend()) {
        return static_cast<ms_NoteArticulation>(search->second);
    }
    return static_cast<ms_NoteArticulation>(0);
}

void MuseSamplerSequencer::parseArticulations(const ArticulationMap& articulations,
                                              ms_NoteArticulation& articulationFlag, ms_NoteHead& notehead) const
{
    notehead = ms_NoteHead_Normal;
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

void MuseSamplerSequencer::parseOffStreamParams(const PlaybackParamList& params, OffStreamParams& out) const
{
    out.clear();

    if (params.empty()) {
        return;
    }

    StringList presetList;

    for (const PlaybackParam& param : params) {
        switch (param.type) {
        case PlaybackParam::SoundPreset:
            presetList.push_back(param.val);
            break;
        case PlaybackParam::PlayingTechnique:
            out.textArticulation = param.val.toStdString();
            out.textArticulationStartsAtNote = !param.isPersistent.value_or(false);
            break;
        case PlaybackParam::Syllable:
            out.syllable = param.val.toStdString();
            out.syllableStartsAtNote = !param.isPersistent.value_or(false);
            break;
        case PlaybackParam::Undefined:
            UNREACHABLE;
            break;
        }
    }

    if (!presetList.empty()) {
        out.presets = presetList.join(u"|").toStdString();
    }
}
