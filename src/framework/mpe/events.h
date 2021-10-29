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
    DynamicType dynamic = DynamicType::Undefined;
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

    explicit NoteEvent(const ArticulationPattern& standardPatterns,
                       const timestamp_t nominalTimestamp,
                       const duration_t nominalDuration,
                       const voice_layer_idx_t voiceIdx,
                       const PitchClass pitchClass,
                       const octave_t octave,
                       const DynamicType dynamicType,
                       const ArticulationMap& articulationsApplied)
    {
        m_arrangementCtx.nominalDuration = nominalDuration;
        m_arrangementCtx.nominalTimestamp = nominalTimestamp;
        m_arrangementCtx.voiceLayerIndex = voiceIdx;

        m_pitchCtx.pitchClass = pitchClass;
        m_pitchCtx.octave = octave;

        m_expressionCtx.articulations = articulationsApplied;
        m_expressionCtx.dynamic = dynamicType;

        ArrangementPatternList arrangementPatterns = m_expressionCtx.articulations.arrangementPatterns();
        calculateActualDuration(arrangementPatterns);
        calculateActualTimestamp(arrangementPatterns);

        calculatePitchCurve(standardPatterns.pitchPattern);

        calculateExpressionCurve(standardPatterns.expressionPattern);
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
    void calculateActualTimestamp(const ArrangementPatternList& patterns)
    {
        m_arrangementCtx.actualTimestamp = m_arrangementCtx.nominalTimestamp;

        if (patterns.empty()) {
            return;
        }

        m_arrangementCtx.actualTimestamp -= ArrangementPattern::averageTimestampOffset(patterns);
    }

    void calculateActualDuration(const ArrangementPatternList& patterns)
    {
        m_arrangementCtx.actualDuration = m_arrangementCtx.nominalDuration;

        if (patterns.empty()) {
            return;
        }

        m_arrangementCtx.actualDuration *= ArrangementPattern::averageDurationFactor(patterns);
    }

    void calculatePitchCurve(const PitchPattern& standardPattern)
    {
        PitchPatternList appliedPatterns = m_expressionCtx.articulations.pitchPatterns();
        PitchPattern::PitchOffsetMap appliedOffsetMap = PitchPattern::averagePitchOffsetMap(std::move(appliedPatterns));

        for (const auto& pair : standardPattern.pitchOffsetMap) {
            m_pitchCtx.pitchCurve.emplace(pair.first, pair.second + appliedOffsetMap[pair.first]);
        }
    }

    void calculateExpressionCurve(const ExpressionPattern& standardPattern)
    {
        ExpressionPatternList appliedPatterns = m_expressionCtx.articulations.expressionPatterns();
        ExpressionPattern::DynamicOffsetMap appliedOffsetMap = ExpressionPattern::averageDynamicOffsetMap(std::move(appliedPatterns));

        float ratio = dynamicLevelFromType(m_expressionCtx.dynamic) / standardPattern.maxAmplitudeLevel;

        for (const auto& pair : standardPattern.dynamicOffsetMap) {
            m_expressionCtx.expressionCurve.emplace(pair.first, (pair.second * ratio) + appliedOffsetMap[pair.first]);
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
