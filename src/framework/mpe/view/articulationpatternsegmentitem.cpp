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

using namespace muse::mpe;

ArticulationPatternSegmentItem::ArticulationPatternSegmentItem(QObject* parent, const ArticulationPatternSegment& segment,
                                                               const int scopePositionFrom, const int scopePositionTo)
    : QObject(parent)
{
    load(segment, scopePositionFrom, scopePositionTo);
}

ArticulationPatternSegment ArticulationPatternSegmentItem::patternSegmentData() const
{
    ArticulationPatternSegment result;

    result.arrangementPattern.durationFactor = durationFactor();
    result.arrangementPattern.timestampOffset = timestampShiftFactor();

    result.pitchPattern.pitchOffsetMap = pitchOffsetsMap();

    result.expressionPattern.dynamicOffsetMap = dynamicOffsetsMap();

    return result;
}

void ArticulationPatternSegmentItem::load(const ArticulationPatternSegment& segment,
                                          const int scopePositionFrom,
                                          const int scopePositionTo)
{
    auto pointsFromMap = [](const SharedMap<duration_percentage_t, percentage_t>& map) -> QList<QPoint> {
        QList<QPoint> result;
        for (const auto& pair : map) {
            result << QPoint(pair.first, pair.second);
        }

        return result;
    };

    setPositionFrom(scopePositionFrom);
    setPositionTo(scopePositionTo);

    setDurationFactor(segment.arrangementPattern.durationFactor);
    setTimestampShiftFactor(segment.arrangementPattern.timestampOffset);

    setPitchOffsets(pointsFromMap(segment.pitchPattern.pitchOffsetMap));

    setDynamicOffsets(pointsFromMap(segment.expressionPattern.dynamicOffsetMap));
}

int ArticulationPatternSegmentItem::durationFactor() const
{
    return m_durationFactor;
}

void ArticulationPatternSegmentItem::setDurationFactor(int newDurationFactor)
{
    if (m_durationFactor == newDurationFactor) {
        return;
    }
    m_durationFactor = newDurationFactor;
    emit durationFactorChanged();
}

int ArticulationPatternSegmentItem::timestampShiftFactor() const
{
    return m_timestampShiftFactor;
}

void ArticulationPatternSegmentItem::setTimestampShiftFactor(int newTimestampShiftFactor)
{
    if (m_timestampShiftFactor == newTimestampShiftFactor) {
        return;
    }
    m_timestampShiftFactor = newTimestampShiftFactor;
    emit timestampShiftFactorChanged();
}

const QList<QPoint>& ArticulationPatternSegmentItem::pitchOffsets() const
{
    return m_pitchOffsets;
}

const QList<QPoint>& ArticulationPatternSegmentItem::dynamicOffsets() const
{
    return m_dynamicOffsets;
}

int ArticulationPatternSegmentItem::positionFrom() const
{
    return m_scopePositionFrom;
}

void ArticulationPatternSegmentItem::setPositionFrom(int newScopePositionFrom)
{
    if (m_scopePositionFrom == newScopePositionFrom) {
        return;
    }
    m_scopePositionFrom = newScopePositionFrom;
    emit positionFromChanged();
}

int ArticulationPatternSegmentItem::positionTo() const
{
    return m_scopePositionTo;
}

void ArticulationPatternSegmentItem::setPositionTo(int newScopePositionTo)
{
    if (m_scopePositionTo == newScopePositionTo) {
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

int ArticulationPatternSegmentItem::dynamicOffsetValueAt(const int index)
{
    if (index >= m_dynamicOffsets.size()) {
        return 0;
    }

    return m_dynamicOffsets.at(index).y();
}

void ArticulationPatternSegmentItem::updateDynamicOffsetValue(const int index, const int value)
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

int ArticulationPatternSegmentItem::pitchOffsetValueAt(const int index)
{
    if (index >= m_pitchOffsets.size()) {
        return 0;
    }

    return m_pitchOffsets.at(index).y();
}

void ArticulationPatternSegmentItem::updatePitchOffsetValue(const int index, const int value)
{
    if (index >= m_pitchOffsets.size()) {
        return;
    }

    m_pitchOffsets[index].setY(value);
    emit pitchOffsetsChanged();
}

void ArticulationPatternSegmentItem::setPitchOffsets(const QList<QPoint>& offsets)
{
    if (m_pitchOffsets == offsets) {
        return;
    }

    m_pitchOffsets = offsets;
    emit pitchOffsetsChanged();
}

void ArticulationPatternSegmentItem::setDynamicOffsets(const QList<QPoint>& offsets)
{
    if (m_dynamicOffsets == offsets) {
        return;
    }

    m_dynamicOffsets = offsets;
    emit dynamicOffsetsChanged();
}

int ArticulationPatternSegmentItem::singlePercentValue() const
{
    return ONE_PERCENT;
}

PitchPattern::PitchOffsetMap ArticulationPatternSegmentItem::pitchOffsetsMap() const
{
    PitchPattern::PitchOffsetMap result;
    for (const QPoint& point : m_pitchOffsets) {
        result.emplace(point.x(), point.y());
    }

    return result;
}

ExpressionPattern::DynamicOffsetMap ArticulationPatternSegmentItem::dynamicOffsetsMap() const
{
    ExpressionPattern::DynamicOffsetMap result;
    for (const QPoint& point : m_dynamicOffsets) {
        result.emplace(point.x(), point.y());
    }

    return result;
}
