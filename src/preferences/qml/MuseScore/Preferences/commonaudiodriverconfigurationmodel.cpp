/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "commonaudiodriverconfigurationmodel.h"

#include "global/translation.h"

#include "audio/common/audiotypes.h"

using namespace mu::preferences;
using namespace muse::audio;

CommonAudioDriverConfigurationModel::CommonAudioDriverConfigurationModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void CommonAudioDriverConfigurationModel::load()
{
    audioDriverController()->currentAudioDriverChanged().onNotify(this, [this]() {
        emit deviceListChanged();
        emit currentDeviceIdChanged();
        emit sampleRateListChanged();
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
        emit sampleRateListChanged();
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

QString CommonAudioDriverConfigurationModel::currentDeviceId() const
{
    AudioDeviceID device = audioDriverController()->outputDevice();
    return QString::fromStdString(device);
}

QVariantList CommonAudioDriverConfigurationModel::deviceList() const
{
    AudioDeviceList devices = audioDriverController()->availableOutputDevices();

    QVariantList result;
    result.reserve(devices.size());

    for (const AudioDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

void CommonAudioDriverConfigurationModel::deviceSelected(const QString& deviceId)
{
    bool ok = audioDriverController()->selectOutputDevice(deviceId.toStdString());
    if (!ok) {
        interactive()->error("",
                             muse::trc("appshell/preferences", "The driver for this device could not be opened."));
    }
}

unsigned int CommonAudioDriverConfigurationModel::bufferSize() const
{
    unsigned int val = audioDriverController()->activeSpec().output.samplesPerChannel;
    return val;
}

QList<unsigned int> CommonAudioDriverConfigurationModel::bufferSizeList() const
{
    std::vector<samples_t> bufferSizes = audioDriverController()->availableOutputDeviceBufferSizes();

    QList<unsigned int> result;
    result.reserve(bufferSizes.size());

    for (samples_t bufferSize : bufferSizes) {
        result << static_cast<unsigned int>(bufferSize);
    }

    return result;
}

void CommonAudioDriverConfigurationModel::bufferSizeSelected(const QString& bufferSizeStr)
{
    audioDriverController()->changeBufferSize(bufferSizeStr.toInt());
}

unsigned int CommonAudioDriverConfigurationModel::sampleRate() const
{
    return audioDriverController()->activeSpec().output.sampleRate;
}

QList<unsigned int> CommonAudioDriverConfigurationModel::sampleRateList() const
{
    std::vector<sample_rate_t> sampleRates = audioDriverController()->availableOutputDeviceSampleRates();

    QList<unsigned int> result;
    result.reserve(sampleRates.size());

    for (sample_rate_t sampleRate : sampleRates) {
        result << static_cast<unsigned int>(sampleRate);
    }

    return result;
}

void CommonAudioDriverConfigurationModel::sampleRateSelected(const QString& sampleRateStr)
{
    audioDriverController()->changeSampleRate(sampleRateStr.toInt());
}
