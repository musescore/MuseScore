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

#include "editmidimappingmodel.h"

#include "log.h"
#include "utils.h"
#include "translation.h"

using namespace mu::shortcuts;
using namespace mu::midi;

EditMidiMappingModel::EditMidiMappingModel(QObject* parent)
    : QObject(parent)
{
}

EditMidiMappingModel::~EditMidiMappingModel()
{
    midiRemote()->setIsSettingMode(false);
}

void EditMidiMappingModel::load(int originType, int originValue)
{
    midiRemote()->setIsSettingMode(true);

    midiInPort()->eventReceived().onReceive(this, [this](tick_t, const Event& event) {
        if (event.opcode() == Event::Opcode::NoteOn || event.opcode() == Event::Opcode::ControlChange) {
            m_event = remoteEventFromMidiEvent(event);
            emit mappingTitleChanged(mappingTitle());
        }
    });

    m_event = RemoteEvent(static_cast<RemoteEventType>(originType), originValue);
    emit mappingTitleChanged(mappingTitle());
}

QString EditMidiMappingModel::mappingTitle() const
{
    MidiDeviceID currentMidiInDeviceId = midiInPort()->deviceID();
    if (currentMidiInDeviceId.empty() || !m_event.isValid()) {
        return qtrc("shortcuts", "Waiting…");
    }

    return deviceName(currentMidiInDeviceId) + " > " + eventName(m_event);
}

QVariant EditMidiMappingModel::inputedEvent() const
{
    QVariantMap obj;
    obj["type"] = static_cast<int>(m_event.type);
    obj["value"] = m_event.value;
    return obj;
}

QString EditMidiMappingModel::deviceName(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : midiInPort()->devices()) {
        if (device.id == deviceId) {
            return QString::fromStdString(device.name);
        }
    }

    return QString();
}

QString EditMidiMappingModel::eventName(const RemoteEvent& event) const
{
    QString title;

    if (event.type == RemoteEventType::Note) {
        return qtrc("shortcuts", "Note ") + QString::fromStdString(pitchToString(event.value));
    } else if (event.type == RemoteEventType::Controller) {
        return qtrc("shortcuts", "Controller ") + QString::number(event.value);
    }

    return qtrc("shortcuts", "None");
}
