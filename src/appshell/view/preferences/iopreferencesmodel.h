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
#ifndef MU_APPSHELL_IOPREFERENCESMODEL_H
#define MU_APPSHELL_IOPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/iaudioconfiguration.h"
#include "midi/imidiconfiguration.h"
#include "midi/imidioutport.h"
#include "midi/imidiinport.h"

namespace mu::appshell {
class IOPreferencesModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, audio::IAudioConfiguration, audioConfiguration)
    INJECT(appshell, midi::IMidiConfiguration, midiConfiguration)
    INJECT(appshell, midi::IMidiOutPort, midiOutPort)
    INJECT(appshell, midi::IMidiInPort, midiInPort)

    Q_PROPERTY(int currentAudioApiIndex READ currentAudioApiIndex WRITE setCurrentAudioApiIndex NOTIFY currentAudioApiIndexChanged)

    Q_PROPERTY(QStringList midiInputDevices READ midiInputDevices NOTIFY midiInputDevicesChanged)
    Q_PROPERTY(
        int currentMidiInputDeviceIndex READ currentMidiInputDeviceIndex WRITE setCurrentMidiInputDeviceIndex NOTIFY currentMidiInputDeviceIndexChanged)

    Q_PROPERTY(QStringList midiOutputDevices READ midiOutputDevices NOTIFY midiOutputDevicesChanged)
    Q_PROPERTY(
        int currentMidiOutputDeviceIndex READ currentMidiOutputDeviceIndex WRITE setCurrentMidiOutputDeviceIndex NOTIFY currentMidiOutputDeviceIndexChanged)

    Q_PROPERTY(bool isMIDI20OutputSupported READ isMIDI20OutputSupported CONSTANT)
    Q_PROPERTY(bool useMIDI20Output READ useMIDI20Output WRITE setUseMIDI20Output NOTIFY useMIDI20OutputChanged)

public:
    explicit IOPreferencesModel(QObject* parent = nullptr);

    int currentAudioApiIndex() const;
    int currentMidiInputDeviceIndex() const;
    int currentMidiOutputDeviceIndex() const;

    Q_INVOKABLE void init();

    Q_INVOKABLE QStringList audioApiList() const;

    Q_INVOKABLE void restartAudioAndMidiDevices();

    QStringList midiInputDevices() const;
    QStringList midiOutputDevices() const;

    bool isMIDI20OutputSupported() const;
    bool useMIDI20Output() const;

public slots:
    void setCurrentAudioApiIndex(int index);
    void setCurrentMidiInputDeviceIndex(int index);
    void setCurrentMidiOutputDeviceIndex(int index);

    void setUseMIDI20Output(bool use);

signals:
    void currentAudioApiIndexChanged(int index);
    void currentMidiInputDeviceIndexChanged();
    void currentMidiOutputDeviceIndexChanged();

    void midiInputDevicesChanged();
    void midiOutputDevicesChanged();

    void useMIDI20OutputChanged();

private:
    midi::MidiDeviceID midiInputDeviceId(int index) const;
    midi::MidiDeviceID midiOutputDeviceId(int index) const;

    void showMidiError(const midi::MidiDeviceID& deviceId, const std::string& text) const;
};
}

#endif // MU_APPSHELL_IOPREFERENCESMODEL_H
