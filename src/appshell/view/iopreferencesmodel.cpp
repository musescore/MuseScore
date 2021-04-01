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
    NOT_IMPLEMENTED;
    return 0;
}

void IOPreferencesModel::setCurrentAudioApiIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentAudioApiIndex()) {
        return;
    }

    emit currentAudioApiIndexChanged(index);
}

int IOPreferencesModel::currentMidiInputDeviceIndex() const
{
    NOT_IMPLEMENTED;
    return 0;
}

void IOPreferencesModel::setCurrentMidiInputDeviceIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentMidiInputDeviceIndex()) {
        return;
    }

    emit currentMidiInputDeviceIndexChanged(index);
}

int IOPreferencesModel::currentMidiOutputDeviceIndex() const
{
    NOT_IMPLEMENTED;
    return 0;
}

void IOPreferencesModel::setCurrentMidiOutputDeviceIndex(int index)
{
    NOT_IMPLEMENTED;

    if (index == currentMidiOutputDeviceIndex()) {
        return;
    }

    emit currentMidiOutputDeviceIndexChanged(index);
}

int IOPreferencesModel::midiOutputLatencyMilliseconds() const
{
    NOT_IMPLEMENTED;
    return 0;
}

void IOPreferencesModel::setMidiOutputLatencyMilliseconds(int latencyMs)
{
    NOT_IMPLEMENTED;

    if (latencyMs ==  midiOutputLatencyMilliseconds()) {
        return;
    }

    emit midiOutputLatencyMillisecondsChanged(latencyMs);
}

QStringList IOPreferencesModel::audioApiList() const
{
    NOT_IMPLEMENTED;
    return QStringList();
}

QStringList IOPreferencesModel::midiInputDeviceList() const
{
    NOT_IMPLEMENTED;
    return QStringList();
}

QStringList IOPreferencesModel::midiOutputDeviceList() const
{
    NOT_IMPLEMENTED;
    return QStringList();
}

void IOPreferencesModel::restartAudioAndMidiDevices()
{
    NOT_IMPLEMENTED;
}
