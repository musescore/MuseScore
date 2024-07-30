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

#ifndef MUSE_SHORTCUTS_EDITMIDIMAPPINGMODEL_H
#define MUSE_SHORTCUTS_EDITMIDIMAPPINGMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "midi/imidiinport.h"
#include "imidiremote.h"

namespace muse::shortcuts {
class EditMidiMappingModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString mappingTitle READ mappingTitle NOTIFY mappingTitleChanged)

    Inject<IMidiRemote> midiRemote = { this };
    Inject<muse::midi::IMidiInPort> midiInPort = { this };

public:
    explicit EditMidiMappingModel(QObject* parent = nullptr);
    ~EditMidiMappingModel();

    QString mappingTitle() const;

    Q_INVOKABLE void load(int originType, int originValue);
    Q_INVOKABLE QVariant inputtedEvent() const;

signals:
    void mappingTitleChanged(const QString& title);

private:
    QString deviceName(const muse::midi::MidiDeviceID& deviceId) const;

    RemoteEvent m_event;
};
}

#endif // MUSE_SHORTCUTS_EDITMIDIMAPPINGMODEL_H
