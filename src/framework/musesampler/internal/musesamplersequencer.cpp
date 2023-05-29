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

    { mpe::ArticulationType::CrossNote, ms_NoteArticulation_XNote },
    { mpe::ArticulationType::GhostNote, ms_NoteArticulation_Ghost },
    { mpe::ArticulationType::CircleNote, ms_NoteArticulation_Circle },
    { mpe::ArticulationType::TriangleNote, ms_NoteArticulation_Triangle },
    { mpe::ArticulationType::DiamondNote, ms_NoteArticulation_Diamond },

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

void MuseSamplerSequencer::init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, ms_Track track)
{
    m_samplerLib = std::move(samplerLib);
    m_sampler = std::move(sampler);
    m_track = std::move(track);
}

void MuseSamplerSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_offStreamEvents.clear();
    m_offStreamFlushed.notify();

    for (const auto& pair : changes) {
        for (const auto& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            mpe::timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
            mpe::timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

            int pitch{};
            int centsOffset{};
            pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, pitch, centsOffset);
            ms_NoteArticulation articulationFlag = noteArticulationTypes(noteEvent);

            if (noteEvent.arrangementCtx().actualDuration > 0) {
                ms_AuditionStartNoteEvent_2 noteOn = { pitch, centsOffset, articulationFlag, 0.5 };
                m_offStreamEvents[timestampFrom].emplace(std::move(noteOn));
            }

            ms_AuditionStopNoteEvent noteOff = { pitch };
            m_offStreamEvents[timestampTo].emplace(std::move(noteOff));
        }
    }

    updateOffSequenceIterator();
}

void MuseSamplerSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_eventsMap = changes;

    reloadTrack();
}

void MuseSamplerSequencer::updateDynamicChanges(const mpe::DynamicLevelMap& changes)
{
    m_dynamicLevelMap = changes;

    reloadTrack();
}

void MuseSamplerSequencer::reloadTrack()
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    m_samplerLib->clearTrack(m_sampler, m_track);
    LOGN() << "Requested to clear track";

    loadNoteEvents(m_eventsMap);
    loadDynamicEvents(m_dynamicLevelMap);

    m_samplerLib->finalizeTrack(m_sampler, m_track);
    LOGN() << "Requested to finalize track";
}

void MuseSamplerSequencer::loadNoteEvents(const mpe::PlaybackEventsMap& changes)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
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
    for (const auto& pair : changes) {
        m_samplerLib->addDynamicsEvent(m_sampler, m_track, pair.first, dynamicLevelRatio(pair.second));
    }
}

void MuseSamplerSequencer::addNoteEvent(const mpe::NoteEvent& noteEvent)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    auto voice = noteEvent.arrangementCtx().voiceLayerIndex;

    for (auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);

        if (art.first == mpe::ArticulationType::Pedal) {
            // Pedal on:
            m_samplerLib->addPedalEvent(m_sampler, m_track, art.second.meta.timestamp, 1.0);
            // Pedal off:
            m_samplerLib->addPedalEvent(m_sampler, m_track, art.second.meta.timestamp + art.second.meta.overallDuration, 0.0);
        }

        if (m_samplerLib->isRangedArticulation(ms_art)) {
            // If this starts an articulation range, indicate the start
            if (art.second.occupiedFrom == 0 && art.second.occupiedTo != mpe::HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeStart(m_sampler, m_track, voice, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range start";
                } else {
                    LOGN() << "added start range for: " << static_cast<int>(ms_art);
                }
            }
        }
    }

    int pitch{};
    int centsOffset{};
    pitchAndTuning(noteEvent.pitchCtx().nominalPitchLevel, pitch, centsOffset);
    if (!m_samplerLib->addNoteEvent(
            m_sampler,
            m_track,
            voice,
            noteEvent.arrangementCtx().nominalTimestamp,
            noteEvent.arrangementCtx().nominalDuration,
            pitch,
            noteEvent.arrangementCtx().bps * 60.0, // API expects BPM
            centsOffset,
            noteArticulationTypes(noteEvent))) {
        LOGE() << "Unable to add event for track";
    } else {
        LOGN() << "Successfully added note event, pitch: " << pitch
               << ", timestamp: " << noteEvent.arrangementCtx().nominalTimestamp
               << ", duration: " << noteEvent.arrangementCtx().nominalDuration
               << ", articulations flag: " << noteArticulationTypes(noteEvent);
    }

    for (auto& art : noteEvent.expressionCtx().articulations) {
        auto ms_art = convertArticulationType(art.first);
        if (m_samplerLib->isRangedArticulation(ms_art)) {
            // If this ends an articulation range, indicate the end
            LOGN() << "range: " << art.second.occupiedFrom << " to " << art.second.occupiedTo;
            if (art.second.occupiedFrom != 0 && art.second.occupiedTo == mpe::HUNDRED_PERCENT) {
                if (m_samplerLib->addTrackEventRangeEnd(m_sampler, m_track, voice, ms_art) != ms_Result_OK) {
                    LOGE() << "Unable to add ranged articulation range end";
                } else {
                    LOGN() << "added end range for: " << static_cast<int>(ms_art);
                }
            }
        }
    }
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
    static constexpr float convertToCents = (100.f / static_cast<float>(mpe::PITCH_LEVEL_STEP));
    centsOffset = tuningPitchLevel * convertToCents;
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

ms_NoteArticulation MuseSamplerSequencer::noteArticulationTypes(const mpe::NoteEvent& noteEvent) const
{
    uint64_t result = 0;

    for (const auto& pair : noteEvent.expressionCtx().articulations) {
        auto search = ARTICULATION_TYPES.find(pair.first);

        if (search == ARTICULATION_TYPES.cend()) {
            continue;
        }

        result |= convertArticulationType(pair.first);
    }

    return static_cast<ms_NoteArticulation>(result);
}
