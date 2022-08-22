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

#include "iopreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::audio;
using namespace mu::midi;

static constexpr int INVALID_DEVICE_ID = -1;

IOPreferencesModel::IOPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

int IOPreferencesModel::currentAudioApiIndex() const
{
    QString currentApi = QString::fromStdString(audioConfiguration()->currentAudioApi());
    return audioApiList().indexOf(currentApi);
}

void IOPreferencesModel::setCurrentAudioApiIndex(int index)
{
    if (index == currentAudioApiIndex()) {
        return;
    }

    std::vector<std::string> apiList = audioConfiguration()->availableAudioApiList();
    if (index < 0 || index >= static_cast<int>(apiList.size())) {
        return;
    }

    audioConfiguration()->setCurrentAudioApi(apiList[index]);
    emit currentAudioApiIndexChanged(index);
}

QString IOPreferencesModel::midiInputDeviceId() const
{
    return QString::fromStdString(midiInPort()->deviceID());
}

void IOPreferencesModel::inputDeviceSelected(const QString& deviceId)
{
    midiConfiguration()->setMidiInputDeviceId(deviceId.toStdString());
}

QString IOPreferencesModel::midiOutputDeviceId() const
{
    return QString::fromStdString(midiOutPort()->deviceID());
}

void IOPreferencesModel::outputDeviceSelected(const QString& deviceId)
{
    midiConfiguration()->setMidiOutputDeviceId(deviceId.toStdString());
}

void IOPreferencesModel::init()
{
    midiInPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit midiInputDevicesChanged();
    });

    midiInPort()->deviceChanged().onNotify(this, [this]() {
        emit midiInputDeviceIdChanged();
    });

    midiOutPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit midiOutputDevicesChanged();
    });

    midiOutPort()->deviceChanged().onNotify(this, [this]() {
        emit midiOutputDeviceIdChanged();
    });
}

QStringList IOPreferencesModel::audioApiList() const
{
    QStringList result;

    for (const std::string& api: audioConfiguration()->availableAudioApiList()) {
        result.push_back(QString::fromStdString(api));
    }

    return result;
}

void IOPreferencesModel::restartAudioAndMidiDevices()
{
    NOT_IMPLEMENTED;
}

QVariantList IOPreferencesModel::midiInputDevices() const
{
    QVariantList result;

    std::vector<MidiDevice> devices = midiInPort()->availableDevices();
    for (const MidiDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

QVariantList IOPreferencesModel::midiOutputDevices() const
{
    QVariantList result;

    std::vector<MidiDevice> devices = midiOutPort()->availableDevices();
    for (const MidiDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

bool IOPreferencesModel::isMIDI20OutputSupported() const
{
    return midiOutPort()->supportsMIDI20Output();
}

bool IOPreferencesModel::useMIDI20Output() const
{
    return midiConfiguration()->useMIDI20Output();
}

void IOPreferencesModel::setUseMIDI20Output(bool use)
{
    if (use == useMIDI20Output()) {
        return;
    }

    midiConfiguration()->setUseMIDI20Output(use);
    emit useMIDI20OutputChanged();
}

void IOPreferencesModel::showMidiError(const MidiDeviceID& deviceId, const std::string& text) const
{
    // todo: display error
    LOGE() << "failed connect to device, deviceID: " << deviceId << ", err: " << text;
}
