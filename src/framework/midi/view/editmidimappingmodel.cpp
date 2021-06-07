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

static const QString WAITING_STATE = qtrc("midi", "Waiting...");

EditMidiMappingModel::EditMidiMappingModel(QObject* parent)
    : QObject(parent)
{
}

EditMidiMappingModel::~EditMidiMappingModel()
{
    midiRemote()->setIsSettingMode(false);
}

void EditMidiMappingModel::load(int originValue)
{
    midiRemote()->setIsSettingMode(true);

    midiInPort()->eventReceived().onReceive(this, [this](const std::pair<tick_t, Event>& event) {
        if (event.second.opcode() != Event::Opcode::NoteOn && event.second.opcode() != Event::Opcode::ControlChange) {
            return;
        }

        LOGD() << "==== recive " << QString::fromStdString(event.second.to_string());
        m_value = event.second.to_MIDI10Package();
        emit mappingTitleChanged(mappingTitle());
    });

    m_value = originValue;
    emit mappingTitleChanged(mappingTitle());
}

QString EditMidiMappingModel::mappingTitle() const
{
    MidiDeviceID currentMidiInDeviceId = midiInPort()->deviceID();
    if (currentMidiInDeviceId.empty() || m_value == 0) {
        return WAITING_STATE;
    }

    return deviceName(currentMidiInDeviceId) + " > " + valueName(m_value);
}

int EditMidiMappingModel::inputedValue() const
{
    return m_value;
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

QString EditMidiMappingModel::valueName(int value) const
{
    QString title;

    Event event = Event::fromMIDI10Package(value);

    if (event.opcode() == Event::Opcode::NoteOn) {
        return qtrc("midi", "Note ") + QString::number(event.note());
    } else if (event.opcode() == Event::Opcode::ControlChange) {
        return qtrc("midi", "Controller ") + QString::number(event.index());
    }

    return qtrc("midi", "None"); // todo
}
