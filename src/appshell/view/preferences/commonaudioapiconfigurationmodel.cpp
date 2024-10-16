/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "commonaudioapiconfigurationmodel.h"

#include "audio/audiotypes.h"

#include "log.h"

using namespace mu::appshell;
using namespace muse::audio;

CommonAudioApiConfigurationModel::CommonAudioApiConfigurationModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void CommonAudioApiConfigurationModel::load()
{
    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        emit deviceListChanged();
    });

    audioDriver()->outputDeviceChanged().onNotify(this, [this]() {
        emit currentDeviceIdChanged();
        emit sampleRateChanged();
        emit bufferSizeListChanged();
        emit bufferSizeChanged();
    });

    audioDriver()->outputDeviceSampleRateChanged().onNotify(this, [this]() {
        emit sampleRateChanged();
    });

    audioDriver()->outputDeviceBufferSizeChanged().onNotify(this, [this]() {
        emit bufferSizeChanged();
    });
}

QString CommonAudioApiConfigurationModel::currentDeviceId() const
{
    return QString::fromStdString(audioDriver()->outputDevice());
}

QVariantList CommonAudioApiConfigurationModel::deviceList() const
{
    QVariantList result;

    AudioDeviceList devices = audioDriver()->availableOutputDevices();
    for (const AudioDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

void CommonAudioApiConfigurationModel::deviceSelected(const QString& deviceId)
{
    audioConfiguration()->setAudioOutputDeviceId(deviceId.toStdString());
}

unsigned int CommonAudioApiConfigurationModel::bufferSize() const
{
    return audioDriver()->outputDeviceBufferSize();
}

QList<unsigned int> CommonAudioApiConfigurationModel::bufferSizeList() const
{
    QList<unsigned int> result;
    std::vector<unsigned int> bufferSizes = audioDriver()->availableOutputDeviceBufferSizes();

    for (unsigned int bufferSize : bufferSizes) {
        result << bufferSize;
    }

    std::sort(result.begin(), result.end());

    return result;
}

void CommonAudioApiConfigurationModel::bufferSizeSelected(const QString& bufferSizeStr)
{
    audioConfiguration()->setDriverBufferSize(bufferSizeStr.toInt());
}

unsigned int CommonAudioApiConfigurationModel::sampleRate() const
{
    return audioDriver()->outputDeviceSampleRate();
}

QList<unsigned int> CommonAudioApiConfigurationModel::sampleRateList() const
{
    QList<unsigned int> result;
    std::vector<unsigned int> sampleRates = audioDriver()->availableOutputDeviceSampleRates();

    for (unsigned int sampleRate : sampleRates) {
        result << sampleRate;
    }

    return result;
}

void CommonAudioApiConfigurationModel::sampleRateSelected(const QString& sampleRateStr)
{
    audioConfiguration()->setSampleRate(sampleRateStr.toInt());
}
