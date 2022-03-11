/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_MPE_EVENTS_H
#define MU_MPE_EVENTS_H

#include <variant>
#include <vector>

#include "async/channel.h"
#include "realfn.h"

#include "mpetypes.h"
#include "soundid.h"

namespace mu::mpe {
struct NoteEvent;
struct RestEvent;
using PlaybackEvent = std::variant<NoteEvent, RestEvent>;
using PlaybackEventList = std::vector<PlaybackEvent>;
using PlaybackEventsMap = std::map<msecs_t, PlaybackEventList>;
using PlaybackEventsChanges = async::Channel<PlaybackEventsMap>;

struct ArrangementContext
{
    timestamp_t nominalTimestamp = 0;
    timestamp_t actualTimestamp = 0;
    duration_t nominalDuration = 0;
    duration_t actualDuration = 0;
    voice_layer_idx_t voiceLayerIndex = 0;

    bool operator==(const ArrangementContext& other) const
    {
        return nominalTimestamp == other.nominalTimestamp
               && actualTimestamp == other.actualTimestamp
               && nominalDuration == other.nominalDuration
               && actualDuration == other.actualDuration
               && voiceLayerIndex == other.voiceLayerIndex;
    }
};

struct PitchContext
{
    pitch_level_t nominalPitchLevel = 0;
    PitchCurve pitchCurve;

    bool operator==(const PitchContext& other) const
    {
        return nominalPitchLevel == other.nominalPitchLevel
               && pitchCurve == other.pitchCurve;
    }
};

struct ExpressionContext
{
    ArticulationMap articulations;
    dynamic_level_t nominalDynamicLevel = MIN_DYNAMIC_LEVEL;
    ExpressionCurve expressionCurve;

    bool operator==(const ExpressionContext& other) const
    {
        return articulations == other.articulations
               && nominalDynamicLevel == other.nominalDynamicLevel
               && expressionCurve == other.expressionCurve;
    }
};

struct NoteEvent
{
    explicit NoteEvent(ArrangementContext&& arrangementCtx,
                       PitchContext&& pitchCtx,
                       ExpressionContext&& expressionCtx)
        : m_arrangementCtx(arrangementCtx),
        m_pitchCtx(pitchCtx),
        m_expressionCtx(expressionCtx)
    {
    }

    explicit NoteEvent(const timestamp_t nominalTimestamp,
                       const duration_t nominalDuration,
                       const voice_layer_idx_t voiceIdx,
                       const pitch_level_t nominalPitchLevel,
                       const dynamic_level_t nominalDynamicLevel,
                       const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;

        m_pitchCtx.nominalPitchLevel = nominalPitchLevel;

        m_expressionCtx.articulations = articulationsApplied;
        m_expressionCtx.nominalDynamicLevel = nominalDynamicLevel;

        setUp();
    }

    const ArrangementContext& arrangementCtx() const
    {
        return m_arrangementCtx;
    }

    const PitchContext& pitchCtx() const
    {
        return m_pitchCtx;
    }

    const ExpressionContext& expressionCtx() const
    {
        return m_expressionCtx;
    }

    bool operator==(const NoteEvent& other) const
    {
        return m_arrangementCtx == other.m_arrangementCtx
               && m_pitchCtx == other.m_pitchCtx
               && m_expressionCtx == other.m_expressionCtx;
    }

private:
    void setUp()
    {
        calculateActualDuration(m_expressionCtx.articulations);
        calculateActualTimestamp(m_expressionCtx.articulations);

        calculatePitchCurve(m_expressionCtx.articulations);

        calculateExpressionCurve(m_expressionCtx.articulations);
    }

    void calculateActualTimestamp(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualTimestamp = m_arrangementCtx.nominalTimestamp;

        if (articulationsApplied.empty()) {
            return;
        }

        int timestampOffsetValue = m_arrangementCtx.nominalDuration * percentageToFactor(articulationsApplied.averageTimestampOffset());
        m_arrangementCtx.actualTimestamp += timestampOffsetValue;
    }

    void calculateActualDuration(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualDuration = m_arrangementCtx.nominalDuration;

        if (articulationsApplied.empty()) {
            return;
        }

        m_arrangementCtx.actualDuration *= percentageToFactor(articulationsApplied.averageDurationFactor());
    }

    void calculatePitchCurve(const ArticulationMap& articulationsApplied)
    {
        const PitchPattern::PitchOffsetMap& appliedOffsetMap = articulationsApplied.averagePitchOffsetMap();

        m_pitchCtx.pitchCurve = appliedOffsetMap;

        if (articulationsApplied.averagePitchRange() == 0 || articulationsApplied.averagePitchRange() == PITCH_LEVEL_STEP) {
            return;
        }

        float ratio = articulationsApplied.averagePitchRange() / static_cast<float>(PITCH_LEVEL_STEP);
        float patternUnitRatio = PITCH_LEVEL_STEP / static_cast<float>(ONE_PERCENT);

        for (auto& pair : m_pitchCtx.pitchCurve) {
            pair.second = RealRound(pair.second * ratio * patternUnitRatio, 0);
        }
    }

    void calculateExpressionCurve(const ArticulationMap& articulationsApplied)
    {
        const ExpressionPattern::DynamicOffsetMap& appliedOffsetMap = articulationsApplied.averageDynamicOffsetMap();

        dynamic_level_t articulationDynamicLevel = articulationsApplied.averageMaxAmplitudeLevel();
        dynamic_level_t nominalDynamicLevel = m_expressionCtx.nominalDynamicLevel;

        m_expressionCtx.expressionCurve = appliedOffsetMap;

        constexpr dynamic_level_t naturalDynamicLevel = dynamicLevelFromType(DynamicType::Natural);

        float dynamicAmplifyFactor = static_cast<float>(articulationDynamicLevel - naturalDynamicLevel) / DYNAMIC_LEVEL_STEP;

        dynamic_level_t amplificationDiff = dynamicAmplifyFactor * std::max(articulationsApplied.averageDynamicRange(), DYNAMIC_LEVEL_STEP);
        dynamic_level_t actualDynamicLevel = nominalDynamicLevel + amplificationDiff;

        if (actualDynamicLevel == articulationDynamicLevel) {
            return;
        }

        float ratio = actualDynamicLevel / static_cast<float>(articulationDynamicLevel);

        for (auto& pair : m_expressionCtx.expressionCurve) {
            pair.second = RealRound(pair.second * ratio, 0);
        }
    }

    ArrangementContext m_arrangementCtx;
    PitchContext m_pitchCtx;
    ExpressionContext m_expressionCtx;
};

struct RestEvent
{
    explicit RestEvent(ArrangementContext&& arrangement)
        : m_arrangementCtx(arrangement) {}

    explicit RestEvent(const timestamp_t nominalTimestamp,
                       const duration_t nominalDuration,
                       const voice_layer_idx_t voiceIdx)
    {
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.actualTimestamp = nominalTimestamp;
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.actualDuration = nominalDuration;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;
    }

    const ArrangementContext& arrangementCtx() const
    {
        return m_arrangementCtx;
    }

    bool operator==(const RestEvent& other) const
    {
        return m_arrangementCtx == other.m_arrangementCtx;
    }

private:
    ArrangementContext m_arrangementCtx;
};

struct PlaybackSetupData
{
    SoundId id = SoundId::Undefined;
    SoundCategory category = SoundCategory::Undefined;
    SoundSubCategories subCategorySet;

    bool contains(const SoundSubCategory subcategory) const
    {
        return subCategorySet.find(subcategory) != subCategorySet.cend();
    }

    bool operator==(const PlaybackSetupData& other) const
    {
        return id == other.id
               && category == other.category
               && subCategorySet == other.subCategorySet;
    }

    bool operator<(const PlaybackSetupData& other) const
    {
        return id < other.id
               && category < other.category
               && subCategorySet < other.subCategorySet;
    }

    bool isValid() const
    {
        return id != SoundId::Undefined
               && category != SoundCategory::Undefined;
    }
};

struct PlaybackData {
    PlaybackEventsMap originEvents;
    PlaybackSetupData setupData;
    PlaybackEventsChanges mainStream;
    PlaybackEventsChanges offStream;

    bool operator==(const PlaybackData& other) const
    {
        return originEvents == other.originEvents
               && setupData == other.setupData;
    }

    bool isValid() const
    {
        return setupData.isValid();
    }
};
}

#endif // MU_MPE_EVENTS_H
