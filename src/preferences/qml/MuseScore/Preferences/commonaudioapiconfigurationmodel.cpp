/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "global/translation.h"

#include "audio/common/audiotypes.h"

using namespace mu::preferences;
using namespace muse::audio;

CommonAudioApiConfigurationModel::CommonAudioApiConfigurationModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void CommonAudioApiConfigurationModel::load()
{
    audioDriverController()->currentAudioApiChanged().onNotify(this, [this]() {
        emit deviceListChanged();
        emit currentDeviceIdChanged();
        emit sampleRateChanged();
        emit bufferSizeListChanged();
        emit bufferSizeChanged();
    });

    audioDriverController()->availableOutputDevicesChanged().onNotify(this, [this]() {
        emit deviceListChanged();
        emit currentDeviceIdChanged();
    });

    audioDriverController()->outputDeviceChanged().onNotify(this, [this]() {
        emit currentDeviceIdChanged();
        emit sampleRateChanged();
        emit bufferSizeListChanged();
        emit bufferSizeChanged();
    });

    audioDriverController()->outputDeviceSampleRateChanged().onNotify(this, [this]() {
        emit sampleRateChanged();
    });

    audioDriverController()->outputDeviceBufferSizeChanged().onNotify(this, [this]() {
        emit bufferSizeChanged();
    });
}

QString CommonAudioApiConfigurationModel::currentDeviceId() const
{
    return QString::fromStdString(audioDriverController()->outputDevice());
}

QVariantList CommonAudioApiConfigurationModel::deviceList() const
{
    QVariantList result;

    AudioDeviceList devices = audioDriverController()->availableOutputDevices();
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
    bool ok = audioDriverController()->selectOutputDevice(deviceId.toStdString());
    if (!ok) {
        interactive()->error("",
                             muse::trc("appshell/preferences", "The driver for this device could not be opened."));
    }
}

unsigned int CommonAudioApiConfigurationModel::bufferSize() const
{
    unsigned int val = audioDriverController()->activeSpec().output.samplesPerChannel;
    return val;
}

QList<unsigned int> CommonAudioApiConfigurationModel::bufferSizeList() const
{
    QList<unsigned int> result;
    std::vector<unsigned int> bufferSizes = audioDriverController()->availableOutputDeviceBufferSizes();

    for (unsigned int bufferSize : bufferSizes) {
        result << bufferSize;
    }

    return result;
}

void CommonAudioApiConfigurationModel::bufferSizeSelected(const QString& bufferSizeStr)
{
    audioDriverController()->changeBufferSize(bufferSizeStr.toInt());
}

unsigned int CommonAudioApiConfigurationModel::sampleRate() const
{
    return audioDriverController()->activeSpec().output.sampleRate;
}

QList<unsigned int> CommonAudioApiConfigurationModel::sampleRateList() const
{
    QList<unsigned int> result;
    std::vector<unsigned int> sampleRates = audioDriverController()->availableOutputDeviceSampleRates();

    for (unsigned int sampleRate : sampleRates) {
        result << sampleRate;
    }

    return result;
}

void CommonAudioApiConfigurationModel::sampleRateSelected(const QString& sampleRateStr)
{
    audioDriverController()->changeSampleRate(sampleRateStr.toInt());
}
