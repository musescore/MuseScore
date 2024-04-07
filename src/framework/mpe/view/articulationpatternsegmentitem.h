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

#ifndef MUSE_MPE_ARTICULATIONPATTERNSEGMENTITEM_H
#define MUSE_MPE_ARTICULATIONPATTERNSEGMENTITEM_H

#include <QObject>
#include <QList>
#include <QPoint>

#include "mpetypes.h"

namespace muse::mpe {
class ArticulationPatternSegmentItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int singlePercentValue READ singlePercentValue CONSTANT)

    Q_PROPERTY(int positionFrom READ positionFrom WRITE setPositionFrom NOTIFY positionFromChanged)
    Q_PROPERTY(int positionTo READ positionTo WRITE setPositionTo NOTIFY positionToChanged)

    // Arrangement
    Q_PROPERTY(int durationFactor READ durationFactor WRITE setDurationFactor NOTIFY durationFactorChanged)
    Q_PROPERTY(int timestampShiftFactor READ timestampShiftFactor WRITE setTimestampShiftFactor NOTIFY timestampShiftFactorChanged)

    // Pitch
    Q_PROPERTY(QList<QPoint> pitchOffsets READ pitchOffsets NOTIFY pitchOffsetsChanged)
    Q_PROPERTY(
        int selectedPitchOffsetIndex READ selectedPitchOffsetIndex WRITE setSelectedPitchOffsetIndex NOTIFY selectedPitchOffsetIndexChanged)

    // Expression
    Q_PROPERTY(QList<QPoint> dynamicOffsets READ dynamicOffsets NOTIFY dynamicOffsetsChanged)
    Q_PROPERTY(
        int selectedDynamicOffsetIndex READ selectedDynamicOffsetIndex WRITE setSelectedDynamicOffsetIndex NOTIFY selectedDynamicOffsetIndexChanged)

public:
    explicit ArticulationPatternSegmentItem(QObject* parent, const ArticulationPatternSegment& segment, const int scopePositionFrom = 0,
                                            const int scopePositionTo = HUNDRED_PERCENT);

    ArticulationPatternSegment patternSegmentData() const;

    void load(const ArticulationPatternSegment& segment, const int scopePositionFrom = 0, const int scopePositionTo = HUNDRED_PERCENT);

    int durationFactor() const;
    void setDurationFactor(int newDurationFactor);
    int timestampShiftFactor() const;
    void setTimestampShiftFactor(int newTimestampShiftFactor);

    const QList<QPoint>& pitchOffsets() const;
    int selectedPitchOffsetIndex() const;
    void setSelectedPitchOffsetIndex(int newSelectedPitchOffsetIndex);
    Q_INVOKABLE int pitchOffsetValueAt(const int index);
    Q_INVOKABLE void updatePitchOffsetValue(const int index, const int value);

    const QList<QPoint>& dynamicOffsets() const;
    int selectedDynamicOffsetIndex() const;
    void setSelectedDynamicOffsetIndex(int newSelectedDynamicOffsetIndex);
    Q_INVOKABLE int dynamicOffsetValueAt(const int index);
    Q_INVOKABLE void updateDynamicOffsetValue(const int index, const int value);

    int positionFrom() const;
    void setPositionFrom(int newScopePositionFrom);

    int positionTo() const;
    void setPositionTo(int newScopePositionTo);

    int singlePercentValue() const;

signals:
    void durationFactorChanged();
    void timestampShiftFactorChanged();
    void pitchOffsetsChanged();
    void dynamicOffsetsChanged();

    void positionFromChanged();
    void positionToChanged();

    void selectedDynamicOffsetIndexChanged();
    void selectedPitchOffsetIndexChanged();

private:
    PitchPattern::PitchOffsetMap pitchOffsetsMap() const;
    ExpressionPattern::DynamicOffsetMap dynamicOffsetsMap() const;

    void setPitchOffsets(const QList<QPoint>& offsets);
    void setDynamicOffsets(const QList<QPoint>& offsets);

    int m_scopePositionFrom = 0;
    int m_scopePositionTo = 0;

    int m_durationFactor = 0;
    int m_timestampShiftFactor = 0;

    QList<QPoint> m_pitchOffsets;
    int m_selectedPitchOffsetIndex = 0;

    QList<QPoint> m_dynamicOffsets;
    int m_selectedDynamicOffsetIndex = 0;
};
}

#endif // MUSE_MPE_ARTICULATIONPATTERNSEGMENTITEM_H
