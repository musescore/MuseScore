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

#include "mpetypes.h"
#include "realfn.h"

namespace mu::mpe {
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
    PitchClass pitchClass = PitchClass::Undefined;
    octave_t octave = 0;
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
                       const PitchClass pitchClass,
                       const octave_t octave,
                       const dynamic_level_t nominalDynamicLevel,
                       const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;

        m_pitchCtx.pitchClass = pitchClass;
        m_pitchCtx.octave = octave;

        m_expressionCtx.articulations = articulationsApplied;
        m_expressionCtx.nominalDynamicLevel = nominalDynamicLevel;

        calculateActualDuration(articulationsApplied);
        calculateActualTimestamp(articulationsApplied);

        calculatePitchCurve(articulationsApplied);

        calculateExpressionCurve(articulationsApplied);
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
    void calculateActualTimestamp(const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.actualTimestamp = m_arrangementCtx.nominalTimestamp;

        if (articulationsApplied.empty()) {
            return;
        }

        m_arrangementCtx.actualTimestamp += m_arrangementCtx.nominalDuration
                                            * percentageToFactor(articulationsApplied.averageTimestampOffset());
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
        PitchPattern::PitchOffsetMap appliedOffsetMap = articulationsApplied.averagePitchOffsetMap();

        for (const auto& pair : appliedOffsetMap) {
            m_pitchCtx.pitchCurve.emplace(pair.first, pair.second);
        }
    }

    void calculateExpressionCurve(const ArticulationMap& articulationsApplied)
    {
        ExpressionPattern::DynamicOffsetMap appliedOffsetMap = articulationsApplied.averageDynamicOffsetMap();

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

    const ArrangementContext& arrangementCtx() const
    {
        return m_arrangementCtx;
    }

private:
    ArrangementContext m_arrangementCtx;
};
}

#endif // MU_MPE_EVENTS_H
