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

#include "waveformmodel.h"

using namespace mu::audio;

static const volume_dbfs_t MAX_DISPLAYED_DBFS = 0.f; // 100%
static const volume_dbfs_t MIN_DISPLAYED_DBFS = -60.f; // 0%

WaveFormModel::WaveFormModel(QObject* parent)
    : QObject(parent)
{
    playback()->audioOutput()->masterSignalChanges().onResolve(this, [this](AudioSignalChanges signalChanges) {
        signalChanges.onReceive(this, [this](const audioch_t, const AudioSignalVal& newValue) {
            setCurrentSignalAmplitude(newValue.amplitude);

            if (newValue.pressure < MIN_DISPLAYED_DBFS) {
                setCurrentVolumePressure(MIN_DISPLAYED_DBFS);
            } else if (newValue.pressure > MAX_DISPLAYED_DBFS) {
                setCurrentVolumePressure(MAX_DISPLAYED_DBFS);
            } else {
                setCurrentVolumePressure(newValue.pressure);
            }
        });
    });
}

QStringList WaveFormModel::availableSources() const
{
    return m_availableSources;
}

QString WaveFormModel::currentSourceName() const
{
    return m_currentSourceName;
}

float WaveFormModel::currentSignalAmplitude() const
{
    return m_currentSignalAmplitude;
}

volume_dbfs_t WaveFormModel::currentVolumePressure() const
{
    return m_currentVolumePressure;
}

void WaveFormModel::setAvailableSources(QStringList availableSources)
{
    if (m_availableSources == availableSources) {
        return;
    }

    m_availableSources = availableSources;
    emit availableSourcesChanged(m_availableSources);
}

void WaveFormModel::setCurrentSourceName(QString currentSourceName)
{
    if (m_currentSourceName == currentSourceName) {
        return;
    }

    m_currentSourceName = currentSourceName;
    emit currentSourceNameChanged(m_currentSourceName);
}

void WaveFormModel::setCurrentSignalAmplitude(float currentSignalAmplitude)
{
    m_currentSignalAmplitude = currentSignalAmplitude;
    emit currentSignalAmplitudeChanged(m_currentSignalAmplitude);
}

void WaveFormModel::setCurrentVolumePressure(float currentVolumePressure)
{
    m_currentVolumePressure = currentVolumePressure;
    emit currentVolumePressureChanged(m_currentVolumePressure);
}

float WaveFormModel::minDisplayedDbfs() const
{
    return MIN_DISPLAYED_DBFS;
}

float WaveFormModel::maxDisplayedDbfs() const
{
    return MAX_DISPLAYED_DBFS;
}
