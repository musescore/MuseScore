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
#ifndef MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H
#define MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "audio/iaudioconfiguration.h"
#include "audio/iaudiodriver.h"

namespace mu::appshell {
class CommonAudioApiConfigurationModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int currentDeviceIndex READ currentDeviceIndex WRITE setCurrentDeviceIndex NOTIFY currentDeviceIndexChanged)
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)

    Q_PROPERTY(int currentSampleRateIndex READ currentSampleRateIndex WRITE setCurrentSampleRateIndex NOTIFY currentSampleRateIndexChanged)

    INJECT(appshell, audio::IAudioConfiguration, audioConfiguration)
    INJECT(appshell, audio::IAudioDriver, audioDriver)

public:
    explicit CommonAudioApiConfigurationModel(QObject* parent = nullptr);

    int currentDeviceIndex() const;
    int currentSampleRateIndex() const;

    Q_INVOKABLE void load();

    QStringList deviceList() const;
    Q_INVOKABLE QStringList sampleRateHzList() const;

public slots:
    void setCurrentDeviceIndex(int index);
    void setCurrentSampleRateIndex(int index);

signals:
    void currentDeviceIndexChanged();
    void deviceListChanged();

    void currentSampleRateIndexChanged();

private:
    int m_currentDeviceIndex = 0;
    int m_currentSampleRateIndex = 0;
};
}

#endif // MU_APPSHELL_COMMONAUDIOAPICONFIGURATIONMODEL_H
