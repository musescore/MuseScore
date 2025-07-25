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

#include "global/timer.h"
#include "audio/audioerrors.h"

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

    { ArticulationType::LaissezVibrer, ms_NoteArticulation_LaissezVibrer },
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

void MuseSamplerSequencer::deinit()
{
    if (m_renderingProgress && m_renderingProgress->isStarted) {
        m_renderingProgress->finish((int)Ret::Code::Cancel);
    }

    if (m_pollRenderingProgressTimer) {
        m_pollRenderingProgressTimer->stop();
    }

    m_renderingProgress = nullptr;
}

void MuseSamplerSequencer::setRenderingProgress(audio::InputProcessingProgress* progress)
{
    m_renderingProgress = progress;
}

void MuseSamplerSequencer::setAutoRenderInterval(double secs)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_samplerLib->setAutoRenderInterval(m_sampler, secs);
}

void MuseSamplerSequencer::triggerRender()
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_samplerLib->triggerRender(m_sampler);
    pollRenderingProgress();
}

void MuseSamplerSequencer::updateOffStreamEvents(const PlaybackEventsMap& events, const DynamicLevelLayers&)
{
    m_auditionParamsCache.clear();

    for (const auto& pair : events) {
        for (const auto& event : pair.second) {
            if (std::holds_alternative<mpe::NoteEvent>(event)) {
                addAuditionNoteEvent(std::get<mpe::NoteEvent>(event));
            } else if (std::holds_alternative<mpe::ControllerChangeEvent>(event)) {
                addAuditionCCEvent(std::get<mpe::ControllerChangeEvent>(event), pair.first);
            } else {
                parseAuditionParams(event, m_auditionParamsCache);
            }
        }
    }

    updateOffSequenceIterator();
}

void MuseSamplerSequencer::updateMainStreamEvents(const PlaybackEventsMap& events, const DynamicLevelLayers& dynamics)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    clearAllTracks();

    loadEvents(events);
    loadDynamicEvents(dynamics);

    finalizeAllTracks();
    pollRenderingProgress();
}

void MuseSamplerSequencer::pollRenderingProgress()
{
    if (!m_renderingProgress) {
        return;
    }

    m_renderingInfo.clear();

    if (!m_pollRenderingProgressTimer) {
        m_pollRenderingProgressTimer = std::make_unique<Timer>(std::chrono::microseconds(500000)); // poll every 500ms
        m_pollRenderingProgressTimer->onTimeout(this, [this]() {
            doPollProgress();
        });
    }

    m_pollRenderingProgressTimer->start();
}

void MuseSamplerSequencer::doPollProgress()
{
    const bool progressStarted = m_renderingInfo.initialChunksDurationUs > 0;

    int rangeCount = 0;
    ms_RenderingRangeList ranges = m_samplerLib->getRenderInfo(m_sampler, &rangeCount);

    audio::InputProcessingProgress::ChunkInfoList chunks;
    long long chunksDurationUs = 0;

    for (int i = 0; i < rangeCount; ++i) {
        const ms_RenderRangeInfo info = m_samplerLib->getNextRenderProgressInfo(ranges);

        switch (info._state) {
        case ms_RenderingState_ErrorNetwork:
            m_renderingInfo.errorCode = static_cast<int>(muse::audio::Err::OnlineSoundsNetworkError);
            break;
        case ms_RenderingState_ErrorRendering:
        case ms_RenderingState_ErrorFileIO:
        case ms_RenderingState_ErrorTimeOut:
            m_renderingInfo.errorCode = static_cast<int>(muse::audio::Err::UnknownError);
            break;
        case ms_RenderingState_Rendering:
            break;
        }

        if (progressStarted && m_renderingInfo.errorCode != 0) {
            continue;
        }

        chunksDurationUs += info._end_us - info._start_us;
        chunks.push_back({ audio::microsecsToSecs(info._start_us), audio::microsecsToSecs(info._end_us) });
    }

    // Start progress
    if (!progressStarted) {
        if (chunksDurationUs <= 0) {
            if (m_pollRenderingProgressTimer->secondsSinceStart() >= 10.f) { // timeout
                m_pollRenderingProgressTimer->stop();
                m_renderingInfo.clear();
            }

            return;
        }

        m_renderingInfo.initialChunksDurationUs = chunksDurationUs;

        if (!m_renderingProgress->isStarted) {
            m_renderingProgress->start();
        }
    }

    bool isChanged = false;
    // chunks
    if (m_renderingInfo.lastReceivedChunks != chunks) {
        m_renderingInfo.lastReceivedChunks = chunks;
        isChanged = true;
    }

    // Update percentage
    const int64_t percentage = std::lround(100.f - (float)chunksDurationUs / (float)m_renderingInfo.initialChunksDurationUs * 100.f);
    if (percentage != m_renderingInfo.percentage) {
        m_renderingInfo.percentage = percentage;
        isChanged = true;
    }

    if (isChanged) {
        m_renderingProgress->process(chunks, std::lround(percentage), 100);
    }

    // Finish progress
    if (chunksDurationUs <= 0) {
        m_pollRenderingProgressTimer->stop();
        m_renderingProgress->finish(m_renderingInfo.errorCode);
        m_renderingInfo.clear();
    }
}

void MuseSamplerSequencer::clearAllTracks()
{
    m_layerIdxToTrackIdx.clear();
    m_presetChangesByTrack.clear();

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

ms_Track MuseSamplerSequencer::findOrCreateTrack(layer_idx_t layerIdx)
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

void MuseSamplerSequencer::loadEvents(const PlaybackEventsMap& changes)
{
    for (const auto& pair : changes) {
        for (const auto& event : pair.second) {
            if (std::holds_alternative<mpe::NoteEvent>(event)) {
                addNoteEvent(std::get<mpe::NoteEvent>(event));
            } else if (std::holds_alternative<mpe::TextArticulationEvent>(event)) {
                addTextArticulationEvent(std::get<mpe::TextArticulationEvent>(event), pair.first);
            } else if (std::holds_alternative<mpe::SoundPresetChangeEvent>(event)) {
                addSoundPresetEvent(std::get<mpe::SoundPresetChangeEvent>(event), pair.first);
            } else if (std::holds_alternative<mpe::SyllableEvent>(event)) {
                addSyllableEvent(std::get<mpe::SyllableEvent>(event), pair.first);
            }
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

    ms_Track track = findOrCreateTrack(layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    for (const auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);

        if (art.first == ArticulationType::Pedal || art.first == ArticulationType::LetRing) {
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

void MuseSamplerSequencer::addTextArticulationEvent(const mpe::TextArticulationEvent& event, long long startUs)
{
    if (event.text.empty()) {
        return;
    }

    ms_Track track = findOrCreateTrack(event.layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    // Make sure that the string exists long enough
    const std::string str = event.text.toStdString();

    ms_TextArticulationEvent evt;
    evt._start_us = startUs;

    if (event.text != ORDINARY_PLAYING_TECHNIQUE_CODE) {
        evt._articulation = str.c_str();
    } else {
        evt._articulation = ""; // resets the active articulation
    }

    m_samplerLib->addTextArticulationEvent(m_sampler, track, evt);
}

void MuseSamplerSequencer::addSoundPresetEvent(const mpe::SoundPresetChangeEvent& event, long long startUs)
{
    if (event.code.empty()) {
        return;
    }

    ms_Track track = findOrCreateTrack(event.layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    std::map<long long, ms_PresetChange>& presetChanges = m_presetChangesByTrack[track];
    ms_PresetChange presetChange = 0;

    auto it = presetChanges.find(startUs);
    if (it == presetChanges.end()) {
        presetChange = m_samplerLib->createPresetChange(m_sampler, track, startUs);
        presetChanges[startUs] = presetChange;
    } else {
        presetChange = it->second;
    }

    // Make sure that the string exists long enough
    const std::string code = event.code.toStdString();
    m_samplerLib->addPreset(m_sampler, track, presetChange, code.c_str());
}

void MuseSamplerSequencer::addSyllableEvent(const mpe::SyllableEvent& event, long long positionUs)
{
    if (event.text.empty()) {
        return;
    }

    ms_Track track = findOrCreateTrack(event.layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    // Make sure that the string exists long enough
    const std::string str = event.text.toStdString();

    SyllableEvent evt;
    evt._position_us = positionUs;
    evt._text = str.c_str();
    evt._hyphened_to_next = event.flags.testFlag(mpe::SyllableEvent::HyphenedToNext);

    m_samplerLib->addSyllableEvent(m_sampler, track, evt);
}

void MuseSamplerSequencer::addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track)
{
    const duration_t duration = noteEvent.arrangementCtx().actualDuration;
    const PitchCurve& pitchCurve = noteEvent.pitchCtx().pitchCurve;

    for (auto it = pitchCurve.begin(); it != pitchCurve.end(); ++it) {
        auto nextIt = std::next(it);
        if (nextIt == pitchCurve.end()) {
            continue;
        }

        if (nextIt->second == it->second) {
            continue;
        }

        const long long currOffsetStart = duration * percentageToFactor(it->first);
        const long long nextOffsetStart = duration * percentageToFactor(nextIt->first);

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
    const duration_t duration = noteEvent.arrangementCtx().actualDuration;
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

void MuseSamplerSequencer::addAuditionNoteEvent(const mpe::NoteEvent& noteEvent)
{
    const mpe::ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();

    layer_idx_t layerIdx = makeLayerIdx(arrangementCtx.staffLayerIndex, arrangementCtx.voiceLayerIndex);
    ms_Track track = findOrCreateTrack(layerIdx);
    IF_ASSERT_FAILED(track) {
        return;
    }

    int pitch = 0, offsetCents = 0;
    pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, pitch, offsetCents);

    if (arrangementCtx.hasStart()) {
        AuditionStartNoteEvent noteOn;
        parseArticulations(noteEvent.expressionCtx().articulations, noteOn.msEvent._articulation, noteOn.msEvent._notehead);
        noteOn.msEvent._pitch = pitch;
        noteOn.msEvent._offset_cents = offsetCents;

        if (noteEvent.expressionCtx().velocityOverride.has_value()) {
            noteOn.msEvent._dynamics = noteEvent.expressionCtx().velocityOverride.value();
        } else {
            noteOn.msEvent._dynamics = dynamicLevelRatio(noteEvent.expressionCtx().nominalDynamicLevel);
        }

        noteOn.msEvent._active_presets = m_auditionParamsCache.presets.empty() ? m_defaultPresetCode.c_str()
                                         : m_auditionParamsCache.presets.c_str();
        noteOn.msEvent._active_text_articulation = m_auditionParamsCache.textArticulation.c_str();
        noteOn.msEvent._active_syllable = m_auditionParamsCache.syllable.c_str();
        noteOn.msEvent._articulation_text_starts_at_note = m_auditionParamsCache.textArticulationStartsAtNote;
        noteOn.msEvent._syllable_starts_at_note = m_auditionParamsCache.syllableStartsAtNote;
        noteOn.msTrack = track;

        m_offStreamEvents[arrangementCtx.actualTimestamp].insert(noteOn);
    }

    if (arrangementCtx.hasEnd()) {
        AuditionStopNoteEvent noteOff;
        noteOff.msEvent = { pitch };
        noteOff.msTrack = track;

        timestamp_t timestampTo = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration;
        m_offStreamEvents[timestampTo].emplace(std::move(noteOff));
    }
}

void MuseSamplerSequencer::addAuditionCCEvent(const mpe::ControllerChangeEvent& event, long long positionUs)
{
    const std::map<mpe::ControllerChangeEvent::Type, int> TYPE_TO_CC {
        { mpe::ControllerChangeEvent::Modulation, 1 },
        { mpe::ControllerChangeEvent::SustainPedalOnOff, 64 },
        { mpe::ControllerChangeEvent::PitchBend, 128 },
    };

    auto ccIt = TYPE_TO_CC.find(event.type);
    if (ccIt == TYPE_TO_CC.end()) {
        return;
    }

    AuditionCCEvent ccEvent;
    ccEvent.cc = ccIt->second;
    ccEvent.value = event.val;
    ccEvent.msTrack = findOrCreateTrack(event.layerIdx);
    IF_ASSERT_FAILED(ccEvent.msTrack) {
        return;
    }

    m_offStreamEvents[positionUs].insert(ccEvent);
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

void MuseSamplerSequencer::parseAuditionParams(const mpe::PlaybackEvent& event, AuditionParams& out) const
{
    if (std::holds_alternative<mpe::TextArticulationEvent>(event)) {
        const mpe::TextArticulationEvent& artEvent = std::get<mpe::TextArticulationEvent>(event);
        out.textArticulation = artEvent.text.toStdString();
        out.textArticulationStartsAtNote = artEvent.flags.testFlag(mpe::TextArticulationEvent::StartsAtPlaybackPosition);
    } else if (std::holds_alternative<mpe::SoundPresetChangeEvent>(event)) {
        const mpe::SoundPresetChangeEvent& soundPresetEvent = std::get<mpe::SoundPresetChangeEvent>(event);

        if (out.presets.empty()) {
            out.presets = soundPresetEvent.code.toStdString();
        } else {
            out.presets += "|" + soundPresetEvent.code.toStdString();
        }
    } else if (std::holds_alternative<mpe::SyllableEvent>(event)) {
        const mpe::SyllableEvent& syllableEvent = std::get<mpe::SyllableEvent>(event);
        out.syllable = syllableEvent.text.toStdString();
        out.syllableStartsAtNote = syllableEvent.flags.testFlag(mpe::SyllableEvent::StartsAtPlaybackPosition);
    }
}
