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
#include "translation.h"

using namespace mu::midi;
using namespace mu::shortcuts;

static const QString WAITING_STATE = qtrc("midi", "Waiting...");

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

    midiInPort()->eventReceived().onReceive(this, [this](const std::pair<tick_t, Event>& event) {
        if (event.second.opcode() == Event::Opcode::NoteOn || event.second.opcode() == Event::Opcode::ControlChange) {
            LOGD() << "==== recive " << QString::fromStdString(event.second.to_string());

            auto remoteEvent = [](const Event& midiEvent) { // todo
                RemoteEvent event;
                bool isNote = midiEvent.isOpcodeIn({ Event::Opcode::NoteOff, Event::Opcode::NoteOn });
                bool isController = midiEvent.isOpcodeIn({ Event::Opcode::ControlChange });
                if (isNote) {
                    event.type = RemoteEventType::Note;
                    event.value = midiEvent.note();
                } else if (isController) {
                    event.type = RemoteEventType::Controller;
                    event.value = midiEvent.index();
                }

                return event;
            };

            m_event = remoteEvent(event.second);
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
        return WAITING_STATE;
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
        return qtrc("midi", "Note ") + QString::number(event.value);
    } else if (event.type == RemoteEventType::Controller) {
        return qtrc("midi", "Controller ") + QString::number(event.value);
    }

    return qtrc("midi", "None"); // todo
}
