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
#ifndef MU_MIDI_MIDIPORTDEVMODEL_H
#define MU_MIDI_MIDIPORTDEVMODEL_H

#include <QObject>
#include <QVariantList>

#include "modularity/ioc.h"
#include "midi/imidioutport.h"
#include "midi/imidiinport.h"
#include "async/asyncable.h"

namespace mu::midi {
class MidiPortDevModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(midi, IMidiOutPort, midiOutPort)
    INJECT(midi, IMidiInPort, midiInPort)

    Q_PROPERTY(bool isInputConnected READ isInputConnected NOTIFY isInputConnectedChanged)

public:
    explicit MidiPortDevModel(QObject* parent = nullptr);

    bool isInputConnected() const;

    Q_INVOKABLE QVariantList outputDevices() const;
    Q_INVOKABLE void outputDeviceAction(const QString& deviceID, const QString& action);

    Q_INVOKABLE QVariantList inputDevices() const;
    Q_INVOKABLE void inputDeviceAction(const QString& deviceID, const QString& action);

    Q_INVOKABLE QVariantList inputEvents() const;
    Q_INVOKABLE void generateMIDI20();

signals:
    void outputDevicesChanged();
    void inputDevicesChanged();
    void inputEventsChanged();
    void isInputConnectedChanged();

private:
    QMap<QString, QString> m_connectionErrors;
    QVariantList m_inputEvents;
};
}

#endif // MU_MIDI_MIDIPORTDEVMODEL_H
