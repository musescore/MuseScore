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

#include "articulationpatternitem.h"

using namespace mu::mpe;

ArticulationPatternItem::ArticulationPatternItem(QObject* parent, const ArticulationPattern& pattern, const float scopePositionFrom,
                                                 const float scopePositionTo)
    : QObject(parent), m_scopePositionFrom(scopePositionFrom), m_scopePositionTo(scopePositionTo)
{
    auto qMapFromStdMap = [](const std::map<float, float>& stdMap) -> QMap<float, float> {
        QMap<float, float> result;
        for (const auto& pair : stdMap) {
            result.insert(pair.first, pair.second);
        }

        return result;
    };

    setDurationFactor(pattern.arrangementPattern.durationFactor);
    //setTimestampShiftFactor(pattern.arrangementPattern.timestampOffset);

    setPitchOffsets(qMapFromStdMap(pattern.pitchPattern.pitchOffsetMap));

    setMaxAmplitudeLevel(pattern.expressionPattern.maxAmplitudeLevel);
    setAmplitudeTimeShiftFactor(pattern.expressionPattern.amplitudeTimeShift);
    setDynamicOffsets(qMapFromStdMap(pattern.expressionPattern.dynamicOffsetMap));
}

ArticulationPattern ArticulationPatternItem::pattern() const
{
    ArticulationPattern result;

    auto stdMapFromQMap = [](const QMap<float, float>& qMap) -> std::map<float, float> {
        std::map<float, float> result;
        for (const auto& key : qMap.keys()) {
            result.emplace(key, qMap.value(key));
        }

        return result;
    };

    result.arrangementPattern.durationFactor = durationFactor();
    result.arrangementPattern.timestampOffset = timestampShiftFactor();

    result.pitchPattern.pitchOffsetMap = stdMapFromQMap(pitchOffsets());

    result.expressionPattern.maxAmplitudeLevel = maxAmplitudeLevel();
    result.expressionPattern.amplitudeTimeShift = timestampShiftFactor();
    result.expressionPattern.dynamicOffsetMap = stdMapFromQMap(dynamicOffsets());

    return result;
}

float ArticulationPatternItem::durationFactor() const
{
    return m_durationFactor;
}

void ArticulationPatternItem::setDurationFactor(float newDurationFactor)
{
    if (qFuzzyCompare(m_durationFactor, newDurationFactor)) {
        return;
    }
    m_durationFactor = newDurationFactor;
    emit durationFactorChanged();
}

float ArticulationPatternItem::timestampShiftFactor() const
{
    return m_timestampShiftFactor;
}

void ArticulationPatternItem::setTimestampShiftFactor(float newTimestampShiftFactor)
{
    if (qFuzzyCompare(m_timestampShiftFactor, newTimestampShiftFactor)) {
        return;
    }
    m_timestampShiftFactor = newTimestampShiftFactor;
    emit timestampShiftFactorChanged();
}

const QMap<float, float>& ArticulationPatternItem::pitchOffsets() const
{
    return m_pitchOffsets;
}

void ArticulationPatternItem::setPitchOffsets(const QMap<float, float>& newPitchOffsets)
{
    if (m_pitchOffsets == newPitchOffsets) {
        return;
    }
    m_pitchOffsets = newPitchOffsets;
    emit pitchOffsetsChanged();
}

float ArticulationPatternItem::maxAmplitudeLevel() const
{
    return m_maxAmplitudeLevel;
}

void ArticulationPatternItem::setMaxAmplitudeLevel(float newMaxAmplitudeLevel)
{
    if (qFuzzyCompare(m_maxAmplitudeLevel, newMaxAmplitudeLevel)) {
        return;
    }
    m_maxAmplitudeLevel = newMaxAmplitudeLevel;
    emit maxAmplitudeLevelChanged();
}

float ArticulationPatternItem::amplitudeTimeShiftFactor() const
{
    return m_amplitudeTimeShiftFactor;
}

void ArticulationPatternItem::setAmplitudeTimeShiftFactor(float newAmplitudeTimeShiftFactor)
{
    if (qFuzzyCompare(m_amplitudeTimeShiftFactor, newAmplitudeTimeShiftFactor)) {
        return;
    }
    m_amplitudeTimeShiftFactor = newAmplitudeTimeShiftFactor;
    emit amplitudeTimeShiftFactorChanged();
}

const QMap<float, float>& ArticulationPatternItem::dynamicOffsets() const
{
    return m_dynamicOffsets;
}

void ArticulationPatternItem::setDynamicOffsets(const QMap<float, float>& newDynamicOffsets)
{
    if (m_dynamicOffsets == newDynamicOffsets) {
        return;
    }
    m_dynamicOffsets = newDynamicOffsets;
    emit dynamicOffsetsChanged();
}

float ArticulationPatternItem::scopePositionFrom() const
{
    return m_scopePositionFrom;
}

void ArticulationPatternItem::setScopePositionFrom(float newScopePositionFrom)
{
    if (qFuzzyCompare(m_scopePositionFrom, newScopePositionFrom)) {
        return;
    }
    m_scopePositionFrom = newScopePositionFrom;
    emit scopePositionFromChanged();
}

float ArticulationPatternItem::scopePositionTo() const
{
    return m_scopePositionTo;
}

void ArticulationPatternItem::setScopePositionTo(float newScopePositionTo)
{
    if (qFuzzyCompare(m_scopePositionTo, newScopePositionTo)) {
        return;
    }
    m_scopePositionTo = newScopePositionTo;
    emit scopePositionToChanged();
}
