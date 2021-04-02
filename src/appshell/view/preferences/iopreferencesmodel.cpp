//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "iopreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::audio;

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
    return m_currentMidiInputDeviceIndex;
}

void IOPreferencesModel::setCurrentMidiInputDeviceIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentMidiInputDeviceIndex()) {
        return;
    }

    m_currentMidiInputDeviceIndex = index;
    emit currentMidiInputDeviceIndexChanged(index);
}

int IOPreferencesModel::currentMidiOutputDeviceIndex() const
{
    return m_currentMidiOutputDeviceIndex;
}

void IOPreferencesModel::setCurrentMidiOutputDeviceIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentMidiOutputDeviceIndex()) {
        return;
    }

    m_currentMidiOutputDeviceIndex = index;
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

QStringList IOPreferencesModel::midiInputDeviceList() const
{
    return QStringList {
        "Universal Audio Keyboard",
        "Test device 1",
        "Test device 2"
    };
}

QStringList IOPreferencesModel::midiOutputDeviceList() const
{
    return QStringList {
        "Universal Audio Keyboard",
        "Test device 1",
        "Test device 2"
    };
}

void IOPreferencesModel::restartAudioAndMidiDevices()
{
    NOT_IMPLEMENTED;
}
