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

#ifndef MU_MPE_ARTICULATIONPATTERNITEM_H
#define MU_MPE_ARTICULATIONPATTERNITEM_H

#include <QObject>
#include <QMap>

#include "mpetypes.h"

namespace mu::mpe {
class ArticulationPatternItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float scopePositionFrom READ scopePositionFrom WRITE setScopePositionFrom NOTIFY scopePositionFromChanged)
    Q_PROPERTY(float scopePositionTo READ scopePositionTo WRITE setScopePositionTo NOTIFY scopePositionToChanged)

    // Arrangement
    Q_PROPERTY(float durationFactor READ durationFactor WRITE setDurationFactor NOTIFY durationFactorChanged)
    Q_PROPERTY(float timestampShiftFactor READ timestampShiftFactor WRITE setTimestampShiftFactor NOTIFY timestampShiftFactorChanged)

    // Pitch
    Q_PROPERTY(QMap<float, float> pitchOffsets READ pitchOffsets WRITE setPitchOffsets NOTIFY pitchOffsetsChanged)

    // Expression
    Q_PROPERTY(float maxAmplitudeLevel READ maxAmplitudeLevel WRITE setMaxAmplitudeLevel NOTIFY maxAmplitudeLevelChanged)
    Q_PROPERTY(
        float amplitudeTimeShiftFactor READ amplitudeTimeShiftFactor WRITE setAmplitudeTimeShiftFactor NOTIFY amplitudeTimeShiftFactorChanged)
    Q_PROPERTY(QMap<float, float> dynamicOffsets READ dynamicOffsets WRITE setDynamicOffsets NOTIFY dynamicOffsetsChanged)

public:
    explicit ArticulationPatternItem(QObject* parent, const ArticulationPattern& pattern, const float scopePositionFrom = 0.f,
                                     const float scopePositionTo = 1.f);

    ArticulationPattern pattern() const;

    float durationFactor() const;
    void setDurationFactor(float newDurationFactor);
    float timestampShiftFactor() const;
    void setTimestampShiftFactor(float newTimestampShiftFactor);

    const QMap<float, float>& pitchOffsets() const;
    void setPitchOffsets(const QMap<float, float>& newPitchOffsets);

    float maxAmplitudeLevel() const;
    void setMaxAmplitudeLevel(float newMaxAmplitudeLevel);
    float amplitudeTimeShiftFactor() const;
    void setAmplitudeTimeShiftFactor(float newAmplitudeTimeShiftFactor);
    const QMap<float, float>& dynamicOffsets() const;
    void setDynamicOffsets(const QMap<float, float>& newDynamicOffsets);

    float scopePositionFrom() const;
    void setScopePositionFrom(float newScopePositionFrom);

    float scopePositionTo() const;
    void setScopePositionTo(float newScopePositionTo);

signals:
    void durationFactorChanged();
    void timestampShiftFactorChanged();
    void pitchOffsetsChanged();
    void maxAmplitudeLevelChanged();
    void amplitudeTimeShiftFactorChanged();
    void dynamicOffsetsChanged();

    void scopePositionFromChanged();

    void scopePositionToChanged();

private:
    float m_scopePositionFrom = 0.f;
    float m_scopePositionTo = 0.f;

    float m_durationFactor = 0.f;
    float m_timestampShiftFactor = 0.f;

    QMap<float, float> m_pitchOffsets;

    float m_maxAmplitudeLevel = 0.f;
    float m_amplitudeTimeShiftFactor = 0.f;
    QMap<float, float> m_dynamicOffsets;
};
}

#endif // MU_MPE_ARTICULATIONPATTERNITEM_H
