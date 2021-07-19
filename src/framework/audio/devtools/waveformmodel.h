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

#ifndef MU_AUDIO_WAVEFORMMODEL_H
#define MU_AUDIO_WAVEFORMMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "iaudiooutput.h"
#include "iplayback.h"

namespace mu::audio {
class WaveFormModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(audio, IPlayback, playback)

    Q_PROPERTY(QStringList availableSources READ availableSources NOTIFY availableSourcesChanged)
    Q_PROPERTY(QString currentSourceName READ currentSourceName WRITE setCurrentSourceName NOTIFY currentSourceNameChanged)

    Q_PROPERTY(float currentSignalAmplitude READ currentSignalAmplitude NOTIFY currentSignalAmplitudeChanged)
    Q_PROPERTY(float currentVolumePressure READ currentVolumePressure NOTIFY currentVolumePressureChanged)

    Q_PROPERTY(float minDisplayedDbfs READ minDisplayedDbfs CONSTANT)
    Q_PROPERTY(float maxDisplayedDbfs READ maxDisplayedDbfs CONSTANT)

public:
    explicit WaveFormModel(QObject* parent = nullptr);

    QStringList availableSources() const;
    QString currentSourceName() const;

    float currentSignalAmplitude() const;
    float currentVolumePressure() const;

    float minDisplayedDbfs() const;
    float maxDisplayedDbfs() const;

public slots:
    void setAvailableSources(QStringList availableSources);
    void setCurrentSourceName(QString currentSourceName);

    void setCurrentSignalAmplitude(float currentSignalAmplitude);
    void setCurrentVolumePressure(float currentVolumePressure);

signals:
    void availableSourcesChanged(QStringList availableSources);
    void currentSourceNameChanged(QString currentSourceName);

    void signalAmplitudeChanged(float signalAmplitude);
    void volumePressureChanged(float volumePressure);

    void currentSignalAmplitudeChanged(float currentSignalAmplitude);
    void currentVolumePressureChanged(float currentVolumePressure);

private:
    QStringList m_availableSources;
    QString m_currentSourceName;

    float m_currentSignalAmplitude = 0.f;
    float m_currentVolumePressure = 0.f;
};
}

#endif // MU_AUDIO_WAVEFORMMODEL_H
