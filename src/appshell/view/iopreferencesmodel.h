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
#ifndef MU_APPSHELL_IOPREFERENCESMODEL_H
#define MU_APPSHELL_IOPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "audio/iaudioconfiguration.h"

namespace mu::appshell {
class IOPreferencesModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, audio::IAudioConfiguration, audioConfiguration)

    Q_PROPERTY(int currentAudioApiIndex READ currentAudioApiIndex WRITE setCurrentAudioApiIndex NOTIFY currentAudioApiIndexChanged)
    Q_PROPERTY(int currentMidiInputDeviceIndex READ currentMidiInputDeviceIndex WRITE setCurrentMidiInputDeviceIndex NOTIFY currentMidiInputDeviceIndexChanged)
    Q_PROPERTY(int currentMidiOutputDeviceIndex READ currentMidiOutputDeviceIndex WRITE setCurrentMidiOutputDeviceIndex NOTIFY currentMidiOutputDeviceIndexChanged)
    Q_PROPERTY(int midiOutputLatencyMilliseconds READ midiOutputLatencyMilliseconds WRITE setMidiOutputLatencyMilliseconds NOTIFY midiOutputLatencyMillisecondsChanged)

public:
    explicit IOPreferencesModel(QObject* parent = nullptr);

    int currentAudioApiIndex() const;
    int currentMidiInputDeviceIndex() const;
    int currentMidiOutputDeviceIndex() const;
    int midiOutputLatencyMilliseconds() const;

    Q_INVOKABLE QStringList audioApiList() const;
    Q_INVOKABLE QStringList midiInputDeviceList() const;
    Q_INVOKABLE QStringList midiOutputDeviceList() const;

    Q_INVOKABLE void restartAudioAndMidiDevices();

public slots:
    void setCurrentAudioApiIndex(int index);
    void setCurrentMidiInputDeviceIndex(int index);
    void setCurrentMidiOutputDeviceIndex(int index);
    void setMidiOutputLatencyMilliseconds(int latencyMs);

signals:
    void currentAudioApiIndexChanged(int index);
    void currentMidiInputDeviceIndexChanged(int index);
    void currentMidiOutputDeviceIndexChanged(int index);
    void midiOutputLatencyMillisecondsChanged(int latencyMs);
};
}

#endif // MU_APPSHELL_IOPREFERENCESMODEL_H
