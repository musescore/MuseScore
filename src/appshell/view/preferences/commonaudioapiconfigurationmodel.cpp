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

#include "commonaudioapiconfigurationmodel.h"

#include "audio/audiotypes.h"

#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace mu::audio;

CommonAudioApiConfigurationModel::CommonAudioApiConfigurationModel(QObject* parent)
    : QObject(parent)
{
}

QString CommonAudioApiConfigurationModel::currentDeviceId() const
{
    return QString::fromStdString(audioDriver()->outputDevice());
}

unsigned int CommonAudioApiConfigurationModel::bufferSize() const
{
    return audioDriver()->bufferSize(currentDeviceId().toStdString());
}

int CommonAudioApiConfigurationModel::currentSampleRateIndex() const
{
    return m_currentSampleRateIndex;
}

void CommonAudioApiConfigurationModel::load()
{
    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        emit deviceListChanged();
    });

    audioDriver()->outputDeviceChanged().onNotify(this, [this]() {
        emit currentDeviceIdChanged();
        emit bufferSizeChanged();
    });

    audioDriver()->bufferSizeChanged().onNotify(this, [this]() {
        emit bufferSizeChanged();
    });
}

void CommonAudioApiConfigurationModel::setCurrentSampleRateIndex(int index)
{
    NOT_IMPLEMENTED;
    m_currentSampleRateIndex = index;
    emit currentSampleRateIndexChanged();
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

QList<unsigned int> CommonAudioApiConfigurationModel::bufferSizeList() const
{
    QList<unsigned int> result;
    std::pair<unsigned int, unsigned int> bufferSizeRange = audioDriver()->availableBufferSizeRange(currentDeviceId().toStdString());

    unsigned int start = bufferSizeRange.first;
    unsigned int end = bufferSizeRange.second;

    if (start == end) {
        return {};
    }

    for (unsigned int bufferSize = end; bufferSize >= start;) {
        result.prepend(bufferSize);
        bufferSize /= 2;
    }

    return result;
}

void CommonAudioApiConfigurationModel::bufferSizeSelected(const QString& bufferSizeStr)
{
    audioConfiguration()->setDriverBufferSize(bufferSizeStr.toInt());
}

void CommonAudioApiConfigurationModel::deviceSelected(const QString& deviceId)
{
    audioConfiguration()->setAudioOutputDeviceId(deviceId.toStdString());
}

QStringList CommonAudioApiConfigurationModel::sampleRateHzList() const
{
    QList<int> sampleRateList {
        192000,
        96000,
        88200,
        48000,
        44100,
        32000,
        22050
    };

    QStringList result;
    for (int sampleRate : sampleRateList) {
        //: Abbreviation of Hertz
        result << QString::number(sampleRate) + " " + qtrc("global", "Hz");
    }

    return result;
}
