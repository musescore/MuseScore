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

#ifndef MU_MPE_ARTICULATIONPATTERNSEGMENTITEM_H
#define MU_MPE_ARTICULATIONPATTERNSEGMENTITEM_H

#include <QObject>
#include <QList>
#include <QPointF>

#include "mpetypes.h"

namespace mu::mpe {
class ArticulationPatternSegmentItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float positionFrom READ positionFrom WRITE setPositionFrom NOTIFY positionFromChanged)
    Q_PROPERTY(float positionTo READ positionTo WRITE setPositionTo NOTIFY positionToChanged)

    // Arrangement
    Q_PROPERTY(float durationFactor READ durationFactor WRITE setDurationFactor NOTIFY durationFactorChanged)
    Q_PROPERTY(float timestampShiftFactor READ timestampShiftFactor WRITE setTimestampShiftFactor NOTIFY timestampShiftFactorChanged)

    // Pitch
    Q_PROPERTY(QList<QPointF> pitchOffsets READ pitchOffsets NOTIFY pitchOffsetsChanged)
    Q_PROPERTY(
        int selectedPitchOffsetIndex READ selectedPitchOffsetIndex WRITE setSelectedPitchOffsetIndex NOTIFY selectedPitchOffsetIndexChanged)

    // Expression
    Q_PROPERTY(float maxAmplitudeLevel READ maxAmplitudeLevel WRITE setMaxAmplitudeLevel NOTIFY maxAmplitudeLevelChanged)
    Q_PROPERTY(
        float amplitudeTimeShiftFactor READ amplitudeTimeShiftFactor WRITE setAmplitudeTimeShiftFactor NOTIFY amplitudeTimeShiftFactorChanged)
    Q_PROPERTY(QList<QPointF> dynamicOffsets READ dynamicOffsets NOTIFY dynamicOffsetsChanged)
    Q_PROPERTY(
        int selectedDynamicOffsetIndex READ selectedDynamicOffsetIndex WRITE setSelectedDynamicOffsetIndex NOTIFY selectedDynamicOffsetIndexChanged)

public:
    explicit ArticulationPatternSegmentItem(QObject* parent, const ArticulationPatternSegment& segment, const float scopePositionFrom = 0.f,
                                            const float scopePositionTo = 1.f);

    ArticulationPatternSegment patternSegmentData() const;

    void load(const ArticulationPatternSegment& segment, const float scopePositionFrom = 0.f, const float scopePositionTo = 1.f);

    float durationFactor() const;
    void setDurationFactor(float newDurationFactor);
    float timestampShiftFactor() const;
    void setTimestampShiftFactor(float newTimestampShiftFactor);

    const QList<QPointF>& pitchOffsets() const;
    int selectedPitchOffsetIndex() const;
    void setSelectedPitchOffsetIndex(int newSelectedPitchOffsetIndex);
    Q_INVOKABLE float pitchOffsetValueAt(const int index);
    Q_INVOKABLE void updatePitchOffsetValue(const int index, const float value);

    float maxAmplitudeLevel() const;
    void setMaxAmplitudeLevel(float newMaxAmplitudeLevel);
    float amplitudeTimeShiftFactor() const;
    void setAmplitudeTimeShiftFactor(float newAmplitudeTimeShiftFactor);
    const QList<QPointF>& dynamicOffsets() const;
    int selectedDynamicOffsetIndex() const;
    void setSelectedDynamicOffsetIndex(int newSelectedDynamicOffsetIndex);
    Q_INVOKABLE float dynamicOffsetValueAt(const int index);
    Q_INVOKABLE void updateDynamicOffsetValue(const int index, const float value);

    float positionFrom() const;
    void setPositionFrom(float newScopePositionFrom);

    float positionTo() const;
    void setPositionTo(float newScopePositionTo);

signals:
    void durationFactorChanged();
    void timestampShiftFactorChanged();
    void pitchOffsetsChanged();
    void maxAmplitudeLevelChanged();
    void amplitudeTimeShiftFactorChanged();
    void dynamicOffsetsChanged();

    void positionFromChanged();
    void positionToChanged();

    void selectedDynamicOffsetIndexChanged();
    void selectedPitchOffsetIndexChanged();

private:
    void setPitchOffsets(const QList<QPointF>& offsets);
    void setDynamicOffsets(const QList<QPointF>& offsets);

    float m_scopePositionFrom = 0.f;
    float m_scopePositionTo = 0.f;

    float m_durationFactor = 0.f;
    float m_timestampShiftFactor = 0.f;

    QList<QPointF> m_pitchOffsets;
    int m_selectedPitchOffsetIndex = 0;

    float m_maxAmplitudeLevel = 0.f;
    float m_amplitudeTimeShiftFactor = 0.f;
    QList<QPointF> m_dynamicOffsets;
    int m_selectedDynamicOffsetIndex = 0;
};
}

#endif // MU_MPE_ARTICULATIONPATTERNSEGMENTITEM_H
