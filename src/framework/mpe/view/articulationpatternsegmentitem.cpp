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

#include "articulationpatternsegmentitem.h"

using namespace mu::mpe;

ArticulationPatternSegmentItem::ArticulationPatternSegmentItem(QObject* parent, const ArticulationPatternSegment& segment,
                                                               const float scopePositionFrom, const float scopePositionTo)
    : QObject(parent)
{
    load(segment, scopePositionFrom, scopePositionTo);
}

ArticulationPatternSegment ArticulationPatternSegmentItem::patternSegmentData() const
{
    ArticulationPatternSegment result;

    auto stdMapFromPoints = [](const QList<QPointF>& points) -> std::map<float, float> {
        std::map<float, float> result;
        for (const QPointF& point : points) {
            result.emplace(point.x(), point.y());
        }

        return result;
    };

    result.arrangementPattern.durationFactor = durationFactor();
    result.arrangementPattern.timestampOffset = timestampShiftFactor();

    result.pitchPattern.pitchOffsetMap = stdMapFromPoints(pitchOffsets());

    result.expressionPattern.maxAmplitudeLevel = maxAmplitudeLevel();
    result.expressionPattern.amplitudeTimeShift = timestampShiftFactor();
    result.expressionPattern.dynamicOffsetMap = stdMapFromPoints(dynamicOffsets());

    return result;
}

void ArticulationPatternSegmentItem::load(const ArticulationPatternSegment& segment,
                                          const float scopePositionFrom,
                                          const float scopePositionTo)
{
    auto pointsFromStdMap = [](const std::map<float, float>& stdMap) -> QList<QPointF> {
        QList<QPointF> result;
        for (const auto& pair : stdMap) {
            result << QPointF(pair.first, pair.second);
        }

        return result;
    };

    setPositionFrom(scopePositionFrom);
    setPositionTo(scopePositionTo);

    setDurationFactor(segment.arrangementPattern.durationFactor);
    setTimestampShiftFactor(segment.arrangementPattern.timestampOffset);

    setPitchOffsets(pointsFromStdMap(segment.pitchPattern.pitchOffsetMap));

    setMaxAmplitudeLevel(segment.expressionPattern.maxAmplitudeLevel);
    setAmplitudeTimeShiftFactor(segment.expressionPattern.amplitudeTimeShift);
    setDynamicOffsets(pointsFromStdMap(segment.expressionPattern.dynamicOffsetMap));
}

float ArticulationPatternSegmentItem::durationFactor() const
{
    return m_durationFactor;
}

void ArticulationPatternSegmentItem::setDurationFactor(float newDurationFactor)
{
    if (qFuzzyCompare(m_durationFactor, newDurationFactor)) {
        return;
    }
    m_durationFactor = newDurationFactor;
    emit durationFactorChanged();
}

float ArticulationPatternSegmentItem::timestampShiftFactor() const
{
    return m_timestampShiftFactor;
}

void ArticulationPatternSegmentItem::setTimestampShiftFactor(float newTimestampShiftFactor)
{
    if (qFuzzyCompare(m_timestampShiftFactor, newTimestampShiftFactor)) {
        return;
    }
    m_timestampShiftFactor = newTimestampShiftFactor;
    emit timestampShiftFactorChanged();
}

const QList<QPointF>& ArticulationPatternSegmentItem::pitchOffsets() const
{
    return m_pitchOffsets;
}

float ArticulationPatternSegmentItem::maxAmplitudeLevel() const
{
    return m_maxAmplitudeLevel;
}

void ArticulationPatternSegmentItem::setMaxAmplitudeLevel(float newMaxAmplitudeLevel)
{
    if (qFuzzyCompare(m_maxAmplitudeLevel, newMaxAmplitudeLevel)) {
        return;
    }
    m_maxAmplitudeLevel = newMaxAmplitudeLevel;
    emit maxAmplitudeLevelChanged();
}

float ArticulationPatternSegmentItem::amplitudeTimeShiftFactor() const
{
    return m_amplitudeTimeShiftFactor;
}

void ArticulationPatternSegmentItem::setAmplitudeTimeShiftFactor(float newAmplitudeTimeShiftFactor)
{
    if (qFuzzyCompare(m_amplitudeTimeShiftFactor, newAmplitudeTimeShiftFactor)) {
        return;
    }
    m_amplitudeTimeShiftFactor = newAmplitudeTimeShiftFactor;
    emit amplitudeTimeShiftFactorChanged();
}

const QList<QPointF>& ArticulationPatternSegmentItem::dynamicOffsets() const
{
    return m_dynamicOffsets;
}

float ArticulationPatternSegmentItem::positionFrom() const
{
    return m_scopePositionFrom;
}

void ArticulationPatternSegmentItem::setPositionFrom(float newScopePositionFrom)
{
    if (qFuzzyCompare(m_scopePositionFrom, newScopePositionFrom)) {
        return;
    }
    m_scopePositionFrom = newScopePositionFrom;
    emit positionFromChanged();
}

float ArticulationPatternSegmentItem::positionTo() const
{
    return m_scopePositionTo;
}

void ArticulationPatternSegmentItem::setPositionTo(float newScopePositionTo)
{
    if (qFuzzyCompare(m_scopePositionTo, newScopePositionTo)) {
        return;
    }
    m_scopePositionTo = newScopePositionTo;
    emit positionToChanged();
}

int ArticulationPatternSegmentItem::selectedDynamicOffsetIndex() const
{
    return m_selectedDynamicOffsetIndex;
}

void ArticulationPatternSegmentItem::setSelectedDynamicOffsetIndex(int newSelectedDynamicOffsetIndex)
{
    if (m_selectedDynamicOffsetIndex == newSelectedDynamicOffsetIndex) {
        return;
    }
    m_selectedDynamicOffsetIndex = newSelectedDynamicOffsetIndex;
    emit selectedDynamicOffsetIndexChanged();
}

float ArticulationPatternSegmentItem::dynamicOffsetValueAt(const int index)
{
    if (index >= m_dynamicOffsets.size()) {
        return 0.f;
    }

    return m_dynamicOffsets.at(index).y();
}

void ArticulationPatternSegmentItem::updateDynamicOffsetValue(const int index, const float value)
{
    if (index >= m_dynamicOffsets.size()) {
        return;
    }

    m_dynamicOffsets[index].setY(value);
    emit dynamicOffsetsChanged();
}

int ArticulationPatternSegmentItem::selectedPitchOffsetIndex() const
{
    return m_selectedPitchOffsetIndex;
}

void ArticulationPatternSegmentItem::setSelectedPitchOffsetIndex(int newSelectedPitchOffsetIndex)
{
    if (m_selectedPitchOffsetIndex == newSelectedPitchOffsetIndex) {
        return;
    }
    m_selectedPitchOffsetIndex = newSelectedPitchOffsetIndex;
    emit selectedPitchOffsetIndexChanged();
}

float ArticulationPatternSegmentItem::pitchOffsetValueAt(const int index)
{
    if (index >= m_pitchOffsets.size()) {
        return 0.f;
    }

    return m_pitchOffsets.at(index).y();
}

void ArticulationPatternSegmentItem::updatePitchOffsetValue(const int index, const float value)
{
    if (index >= m_pitchOffsets.size()) {
        return;
    }

    m_pitchOffsets[index].setY(value);
    emit pitchOffsetsChanged();
}

void ArticulationPatternSegmentItem::setPitchOffsets(const QList<QPointF>& offsets)
{
    if (m_pitchOffsets == offsets) {
        return;
    }

    m_pitchOffsets = offsets;
    emit pitchOffsetsChanged();
}

void ArticulationPatternSegmentItem::setDynamicOffsets(const QList<QPointF>& offsets)
{
    if (m_dynamicOffsets == offsets) {
        return;
    }

    m_dynamicOffsets = offsets;
    emit dynamicOffsetsChanged();
}
