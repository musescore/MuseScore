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
#ifndef MUSE_MIDI_MIDIPORTDEVMODEL_H
#define MUSE_MIDI_MIDIPORTDEVMODEL_H

#include <QObject>
#include <QVariantList>

#include "modularity/ioc.h"
#include "midi/imidioutport.h"
#include "midi/imidiinport.h"
#include "async/asyncable.h"

namespace muse::midi {
class MidiPortDevModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList outputDevices READ outputDevices NOTIFY outputDevicesChanged)
    Q_PROPERTY(QVariantList inputDevices READ inputDevices NOTIFY inputDevicesChanged)
    Q_PROPERTY(QVariantList inputEvents READ inputEvents NOTIFY inputEventsChanged)

    Inject<IMidiOutPort> midiOutPort = { this };
    Inject<IMidiInPort> midiInPort = { this };

public:
    explicit MidiPortDevModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    QVariantList outputDevices() const;
    Q_INVOKABLE void outputDeviceAction(const QString& deviceID, const QString& action);

    QVariantList inputDevices() const;
    Q_INVOKABLE void inputDeviceAction(const QString& deviceID, const QString& action);

    QVariantList inputEvents() const;
    Q_INVOKABLE void generateMIDI20();

signals:
    void outputDevicesChanged();
    void inputDevicesChanged();
    void inputEventsChanged();

private:
    QMap<QString, QString> m_connectionErrors;
    QVariantList m_inputEvents;
};
}

#endif // MUSE_MIDI_MIDIPORTDEVMODEL_H
