//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_MIDI_MIDIPORTDEVMODEL_H
#define MU_MIDI_MIDIPORTDEVMODEL_H

#include <QObject>
#include <QVariantList>

#include "modularity/ioc.h"
#include "midi/imidioutport.h"

namespace mu {
namespace midi {
class MidiPortDevModel : public QObject
{
    Q_OBJECT

    INJECT(midi, IMidiOutPort, midiOutPort)

public:
    explicit MidiPortDevModel(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList outputDevices() const;
    Q_INVOKABLE void outputDeviceAction(const QString& deviceID, const QString& action);

signals:

    void outputDevicesChanged();

private:

    QMap<QString, QString> m_connectionErrors;
};
}
}

#endif // MU_MIDI_MIDIPORTDEVMODEL_H
