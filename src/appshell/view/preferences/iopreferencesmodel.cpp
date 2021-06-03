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

int IOPreferencesModel::currentMidiInputDeviceIndex() const
{
    QString currentMidiInputDeviceId = QString::fromStdString(midiConfiguration()->midiInputDeviceId());
    std::vector<MidiDevice> devices = midiInPort()->devices();
    for (size_t i = 0; i < devices.size(); ++i) {
        if (devices[i].id == currentMidiInputDeviceId.toStdString()) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void IOPreferencesModel::setCurrentMidiInputDeviceIndex(int index)
{
    if (index == currentMidiInputDeviceIndex()) {
        return;
    }

    MidiDeviceID deviceId = midiInputDeviceId(index);

    Ret ret = midiInPort()->connect(deviceId);
    if (!ret) {
        // todo: display error
        return;
    }

    ret = midiInPort()->run();
    if (!ret) {
        // todo: display error
        return;
    }

    midiConfiguration()->setMidiInputDeviceId(deviceId);

    emit currentMidiInputDeviceIndexChanged(index);
}

int IOPreferencesModel::currentMidiOutputDeviceIndex() const
{
    QString currentMidiOutputDeviceId = QString::fromStdString(midiConfiguration()->midiOutputDeviceId());
    std::vector<MidiDevice> devices = midiOutPort()->devices();
    for (size_t i = 0; i < devices.size(); ++i) {
        if (devices[i].id == currentMidiOutputDeviceId.toStdString()) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void IOPreferencesModel::init()
{
//    midiConfiguration()
}

void IOPreferencesModel::setCurrentMidiOutputDeviceIndex(int index)
{
    if (index == currentMidiOutputDeviceIndex()) {
        return;
    }

    MidiDeviceID deviceId = midiOutputDeviceId(index);

    Ret ret = midiOutPort()->connect(deviceId);
    if (!ret) {
        // todo: display error
        return;
    }

    midiConfiguration()->setMidiOutputDeviceId(deviceId);

    emit currentMidiOutputDeviceIndexChanged(index);
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

QStringList IOPreferencesModel::midiInputDevices() const
{
    QStringList list;
    std::vector<MidiDevice> devices = midiInPort()->devices();
    for (const MidiDevice& device : devices) {
        list << QString::fromStdString(device.name);
    }

    return list;
}

QStringList IOPreferencesModel::midiOutputDevices() const
{
    QStringList list;
    std::vector<MidiDevice> devices = midiOutPort()->devices();
    for (const MidiDevice& device : devices) {
        list << QString::fromStdString(device.name);
    }

    return list;
}

mu::midi::MidiDeviceID IOPreferencesModel::midiInputDeviceId(int index) const
{
    std::vector<MidiDevice> devices = midiInPort()->devices();
    if (0 > index || index > static_cast<int>(devices.size())) {
        return MidiDeviceID();
    }

    return devices[index].id;
}

mu::midi::MidiDeviceID IOPreferencesModel::midiOutputDeviceId(int index) const
{
    std::vector<MidiDevice> devices = midiOutPort()->devices();
    if (0 > index || index > static_cast<int>(devices.size())) {
        return MidiDeviceID();
    }

    return devices[index].id;
}
