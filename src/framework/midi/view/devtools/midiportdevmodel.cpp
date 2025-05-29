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
#include "midiportdevmodel.h"

#include "log.h"

using namespace muse::midi;

MidiPortDevModel::MidiPortDevModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void MidiPortDevModel::init()
{
    midiInPort()->eventReceived().onReceive(this, [this](tick_t tick, const Event& event) {
        QString str = "tick: " + QString::number(tick) + " " + QString::fromStdString(event.to_string());
        LOGI() << str;

        m_inputEvents.prepend(str);
        emit inputEventsChanged();
    });

    midiInPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit inputDevicesChanged();
    });

    midiOutPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit outputDevicesChanged();
    });
}

QVariantList MidiPortDevModel::outputDevices() const
{
    QVariantList list;
    std::vector<MidiDevice> devs = midiOutPort()->availableDevices();
    for (const MidiDevice& d : devs) {
        QVariantMap item;
        item["id"] = QString::fromStdString(d.id);
        item["name"] = QString::fromStdString(d.name);

        bool isConnected = midiOutPort()->deviceID() == d.id;

        item["action"] = isConnected ? "Disconnect" : "Connect";

        item["error"] = m_connectionErrors.value(QString::fromStdString(d.id));
        list << item;
    }

    return list;
}

void MidiPortDevModel::outputDeviceAction(const QString& deviceID, const QString& action)
{
    LOGI() << "deviceID: " << deviceID << ", action: " << action;
    midiOutPort()->disconnect();

    if (action == "Connect") {
        Ret ret = midiOutPort()->connect(deviceID.toStdString());
        if (!ret) {
            LOGE() << "failed connect, deviceID: " << deviceID << ", err: " << ret.text();
            m_connectionErrors[deviceID] = QString::fromStdString(ret.text());
        }
    }

    emit outputDevicesChanged();
}

QVariantList MidiPortDevModel::inputDevices() const
{
    QVariantList list;
    std::vector<MidiDevice> devs = midiInPort()->availableDevices();
    for (const MidiDevice& d : devs) {
        QVariantMap item;
        item["id"] = QString::fromStdString(d.id);
        item["name"] = QString::fromStdString(d.name);

        bool isConnected = midiInPort()->deviceID() == d.id;
        item["action"] = isConnected ? "Disconnect" : "Connect";

        item["error"] = m_connectionErrors.value(QString::fromStdString(d.id));
        list << item;
    }

    return list;
}

void MidiPortDevModel::inputDeviceAction(const QString& deviceID, const QString& action)
{
    LOGI() << "deviceID: " << deviceID << ", action: " << action;
    midiInPort()->disconnect();

    if (action == "Connect") {
        Ret ret = midiInPort()->connect(deviceID.toStdString());
        if (!ret) {
            LOGE() << "failed connect, deviceID: " << deviceID << ", err: " << ret.text();
            m_connectionErrors[deviceID] = QString::fromStdString(ret.text());
        }
    }

    emit inputDevicesChanged();
}

QVariantList MidiPortDevModel::inputEvents() const
{
    return m_inputEvents;
}

void MidiPortDevModel::generateMIDI20()
{
    std::set<Event::Opcode> opcodes({ Event::Opcode::PolyPressure,
                                      Event::Opcode::RegisteredPerNoteController,
                                      Event::Opcode::AssignablePerNoteController,
                                      Event::Opcode::RegisteredController,
                                      Event::Opcode::AssignableController,
                                      Event::Opcode::RelativeRegisteredController,
                                      Event::Opcode::RelativeAssignableController,
                                      Event::Opcode::ControlChange,
                                      Event::Opcode::ChannelPressure,
                                      Event::Opcode::PitchBend,
                                      Event::Opcode::PerNotePitchBend,
                                      Event::Opcode::NoteOff,
                                      Event::Opcode::NoteOn,
                                      Event::Opcode::PolyPressure,
                                      Event::Opcode::ProgramChange });
    int group = 0, channel = 0,
        note = 60,
        velocity = 64,
        bank = 10,
        index = 20,
        data = 30;

    for (auto& o : opcodes) {
        Event e;
        e.setMessageType(Event::MessageType::ChannelVoice20);
        e.setOpcode(o);
        e.setGroup(++group);
        e.setChannel(++channel);
        switch (o) {
        case Event::Opcode::NoteOff:
        case Event::Opcode::NoteOn:
            e.setNote(++note);
            e.setVelocity7(++velocity);
            e.setPitchNote(note + 12, 0.5);
            break;
        case Event::Opcode::PolyPressure:
        case Event::Opcode::PerNotePitchBend:
            e.setNote(++note);
            e.setData(++data);
            break;
        case Event::Opcode::RegisteredPerNoteController:
        case Event::Opcode::AssignablePerNoteController:
            e.setNote(++note);
            e.setIndex(++index);
            e.setData(++data);
            break;
        case Event::Opcode::PerNoteManagement:
            e.setNote(++note);
            e.setPerNoteDetach(true);
            e.setPerNoteReset(true);
            break;
        case Event::Opcode::ControlChange:
            e.setIndex(++index);
            e.setData(++data);
            break;
        case Event::Opcode::ProgramChange:
            e.setProgram('p');
            e.setBank(++bank);
            break;
        case Event::Opcode::RegisteredController:
        case Event::Opcode::AssignableController:
        case Event::Opcode::RelativeRegisteredController:
        case Event::Opcode::RelativeAssignableController:
            e.setBank(++bank);
            e.setIndex(++index);
            Q_FALLTHROUGH();
        case Event::Opcode::ChannelPressure:
        case Event::Opcode::PitchBend:
            e.setData(++data);
        }

        midiOutPort()->sendEvent(e);

        QString str = QString::fromStdString(e.to_string());
        QString str2 = "";

        str += " converted to:";
        str2 += "\t";
        for (auto& e1 : e.toMIDI10()) {
            str2 += QString::fromStdString(e1.to_string());
        }
        LOGI() << str << str2;
        m_inputEvents.prepend(str2);
        m_inputEvents.prepend(str);
    }
    emit inputEventsChanged();
}
