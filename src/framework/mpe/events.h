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

#include "realfn.h"

#include "mpetypes.h"

namespace mu::mpe {
struct NoteEvent;
struct RestEvent;
using PlaybackEvent = std::variant<NoteEvent, RestEvent>;
using PlaybackEventList = std::vector<PlaybackEvent>;

struct ArrangementContext
{
    timestamp_t nominalTimestamp = 0;
    timestamp_t actualTimestamp = 0;
    duration_t nominalDuration = 0;
    duration_t actualDuration = 0;
    voice_layer_idx_t voiceLayerIndex = 0;
};

struct PitchContext
{
    pitch_level_t nominalPitchLevel = 0;
    PitchCurve pitchCurve;
};

struct ExpressionContext
{
    ArticulationMap articulations;
    dynamic_level_t nominalDynamicLevel = MIN_DYNAMIC_LEVEL;
    ExpressionCurve expressionCurve;
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
                       ArticulationMap&& articulationsApplied)
    {
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;

        m_pitchCtx.nominalPitchLevel = nominalPitchLevel;

        m_expressionCtx.articulations = std::move(articulationsApplied);
        m_expressionCtx.nominalDynamicLevel = nominalDynamicLevel;

        setUp();
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

        if (articulationsApplied.isEmpty()) {
            return;
        }

        m_arrangementCtx.actualTimestamp += m_arrangementCtx.nominalDuration
                                            * percentageToFactor(articulationsApplied.averageTimestampOffset());
    }

    void calculateActualDuration(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualDuration = m_arrangementCtx.nominalDuration;

        if (articulationsApplied.isEmpty()) {
            return;
        }

        m_arrangementCtx.actualDuration *= percentageToFactor(articulationsApplied.averageDurationFactor());
    }

    void calculatePitchCurve(const ArticulationMap& articulationsApplied)
    {
        const PitchPattern::PitchOffsetMap& appliedOffsetMap = articulationsApplied.averagePitchOffsetMap();

        for (const auto& pair : appliedOffsetMap) {
            m_pitchCtx.pitchCurve.emplace(pair.first, pair.second);
        }
    }

    void calculateExpressionCurve(const ArticulationMap& articulationsApplied)
    {
        const ExpressionPattern::DynamicOffsetMap& appliedOffsetMap = articulationsApplied.averageDynamicOffsetMap();

        dynamic_level_t articulationDynamicLevel = articulationsApplied.averageMaxAmplitudeLevel();
        dynamic_level_t nominalDynamicLevel = m_expressionCtx.nominalDynamicLevel;

        constexpr dynamic_level_t naturalDynamicLevel = dynamicLevelFromType(DynamicType::Natural);

        float dynamicAmplifyFactor = static_cast<float>(articulationDynamicLevel - naturalDynamicLevel) / DYNAMIC_LEVEL_STEP;
        dynamic_level_t amplificationDiff = dynamicAmplifyFactor * std::max(articulationsApplied.averageDynamicRange(), DYNAMIC_LEVEL_STEP);
        dynamic_level_t actualDynamicLevel = nominalDynamicLevel + amplificationDiff;

        float ratio = actualDynamicLevel / static_cast<float>(articulationDynamicLevel);
        if (actualDynamicLevel < articulationDynamicLevel) {
            ratio = 1 / ratio;
        }

        for (const auto& pair : appliedOffsetMap) {
            m_expressionCtx.expressionCurve.emplace(pair.first, RealRound(pair.second * ratio, 0));
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

private:
    ArrangementContext m_arrangementCtx;
};
}

#endif // MU_MPE_EVENTS_H
