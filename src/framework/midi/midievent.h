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
#ifndef MUSE_MIDI_MIDIEVENT_H
#define MUSE_MIDI_MIDIEVENT_H

#include <cstdint>
#include <array>
#include <set>
#include <cassert>
#include <string>

#include "containers.h"

#ifndef UNUSED
#define UNUSED(x) (void)x;
#endif

#ifndef MU_FALLTHROUGH
#if __has_cpp_attribute(fallthrough)
#define MU_FALLTHROUGH() [[fallthrough]]
#else
#define MU_FALLTHROUGH() (void)0
#endif
#endif

namespace muse::midi {
using channel_t = uint8_t;
using tuning_t = float;

/*!
 * MIDI Event stored in Universal MIDI Packet Format Message
 * @see M2-104-UM
 * @see midi.org
 */
struct Event {
    enum class MessageType {
        Utility             = 0x0,
        SystemRealTime      = 0x1,
        //! MIDI1.0 voice message
        ChannelVoice10      = 0x2,
        SystemExclusiveData = 0x3,
        //! MIDI2.0 voice message
        ChannelVoice20      = 0x4,
        Data                = 0x5
    };

    enum class Opcode {
        RegisteredPerNoteController = 0b0000,
        AssignablePerNoteController = 0b0001,
        RegisteredController        = 0b0010,
        AssignableController        = 0b0011,
        RelativeRegisteredController= 0b0100,
        RelativeAssignableController= 0b0101,
        PerNotePitchBend            = 0b0110,
        NoteOff                     = 0b1000,
        NoteOn                      = 0b1001,
        PolyPressure                = 0b1010,
        ControlChange               = 0b1011,
        ProgramChange               = 0b1100,
        ChannelPressure             = 0b1101,
        PitchBend                   = 0b1110,
        PerNoteManagement           = 0b1111
    };

    enum class AttributeType {
        NoData = 0x00,
        ManufacturerSpecific = 0x01,
        ProfileSpecific = 0x02,
        Pitch = 0x03
    };

    enum class UtilityStatus {
        NoOperation        = 0x00,
        JRClock     = 0x01,
        JRTimestamp = 0x02
    };

    Event()
        : m_data({ 0, 0, 0, 0 }) {}
    Event(const std::array<uint32_t, 4>& d)
        : m_data(d) {}
    Event(Opcode opcode, MessageType type = MessageType::ChannelVoice20)
    {
        setMessageType(type);
        setOpcode(opcode);
    }

    //! 4.8.1 NOOP Utility message
    static Event NOOP() { return Event(); }

    //! from standard MIDI1.0 (specification 1996)
    static Event fromMIDI10Package(uint32_t data)
    {
        Event e;
        union {
            unsigned char byte[4];
            uint32_t full;
        } u;
        u.full = data;
        if ((u.byte[0] >= 0x80)
            && (u.byte[0] < 0xF0)
            ) {
            e.m_data[0] = (u.byte[0] << 16) | (u.byte[1] << 8) | u.byte[2];
            e.setMessageType(MessageType::ChannelVoice10);
        }
        return e;
    }

    uint32_t to_MIDI10Package() const
    {
        if (messageType() == MessageType::ChannelVoice10) {
            union {
                unsigned char byte[4];
                uint32_t uint32;
            } p;
            p.uint32 = m_data[0];
            std::swap(p.byte[0], p.byte[2]);
            p.byte[3] = 0;
            return p.uint32;
        }
        return 0;
    }

    static Event fromRawData(const uint32_t* data, size_t count)
    {
        Event e;
        size_t numBytes = std::min(count, e.m_data.size()) * sizeof(uint32_t);
        memcpy(e.m_data.data(), data, numBytes);
        return e;
    }

    bool operator ==(const Event& other) const { return m_data == other.m_data; }
    bool operator !=(const Event& other) const { return !operator==(other); }
    operator bool() const {
        return operator!=(NOOP()) && isValid();
    }

    bool operator <(const Event& other) const
    {
        return m_data < other.m_data;
    }

    bool isChannelVoice() const { return messageType() == MessageType::ChannelVoice10 || messageType() == MessageType::ChannelVoice20; }
    bool isChannelVoice20() const { return messageType() == MessageType::ChannelVoice20; }
    bool isMessageTypeIn(const std::set<MessageType>& types) const { return types.find(messageType()) != types.end(); }
    bool isOpcodeIn(const std::set<Opcode>& opcodes) const { return opcodes.find(opcode()) != opcodes.end(); }

    //! check UMP for correct structure
    bool isValid() const
    {
        switch (messageType()) {
        case MessageType::Utility: {
            std::set<UtilityStatus> statuses = { UtilityStatus::NoOperation, UtilityStatus::JRClock, UtilityStatus::JRTimestamp };
            return statuses.find(static_cast<UtilityStatus>(status())) != statuses.end();
        }

        case MessageType::SystemRealTime:
        case MessageType::SystemExclusiveData:
        case MessageType::Data:
            return true;

        case MessageType::ChannelVoice10:
            return isOpcodeIn({ Opcode::NoteOff, Opcode::NoteOn, Opcode::PolyPressure, Opcode::ControlChange, Opcode::ProgramChange,
                                Opcode::ChannelPressure, Opcode::PitchBend });

        case MessageType::ChannelVoice20:
            return isOpcodeIn({ Opcode::RegisteredPerNoteController,
                                Opcode::AssignablePerNoteController,
                                Opcode::RegisteredController,
                                Opcode::AssignableController,
                                Opcode::RelativeRegisteredController,
                                Opcode::RelativeAssignableController,
                                Opcode::PerNotePitchBend,
                                Opcode::NoteOff,
                                Opcode::NoteOn,
                                Opcode::PolyPressure,
                                Opcode::ControlChange,
                                Opcode::ProgramChange,
                                Opcode::ChannelPressure,
                                Opcode::PitchBend,
                                Opcode::PerNoteManagement
                              });
        }
        return false;
    }

    MessageType messageType() const { return static_cast<MessageType>(m_data[0] >> 28); }
    void setMessageType(MessageType type)
    {
        uint32_t mask = static_cast<uint32_t>(type) << 28;
        m_data[0] &= 0x0FFFFFFF;
        m_data[0] |= mask;
    }

    channel_t group() const { return (m_data[0] >> 24) & 0b00001111; }
    void setGroup(channel_t group)
    {
        assert(group < 16);
        uint32_t mask = group << 24;
        m_data[0] &= 0xF0FFFFFF;
        m_data[0] |= mask;
    }

    Opcode opcode() const
    {
        assertChannelVoice();
        return static_cast<Opcode>((m_data[0] >> 20) & 0b00001111);
    }

    void setOpcode(Opcode code)
    {
        assertChannelVoice();
        uint32_t mask = static_cast<uint32_t>(code) << 20;
        m_data[0] &= 0xFF0FFFFF;
        m_data[0] |= mask;
    }

    uint8_t status() const
    {
        assertMessageType({ MessageType::SystemRealTime, MessageType::Utility });

        switch (messageType()) {
        case MessageType::SystemRealTime: return (m_data[0] >> 16) & 0b11111111;
        case MessageType::Utility: return (m_data[0] >> 16) & 0b00001111;
        default: break;
        }
        return 0;
    }

    channel_t channel() const
    {
        assertChannelVoice();
        return (m_data[0] >> 16) & 0b00001111;
    }

    void setChannel(channel_t channel)
    {
        assertChannelVoice();
        assert(channel < 16);
        uint32_t mask = channel << 16;
        m_data[0] &= 0xFFF0FFFF;
        m_data[0] |= mask;
    }

    uint8_t note() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff, Opcode::PolyPressure,
                       Opcode::PerNotePitchBend, Opcode::PerNoteManagement,
                       Opcode::RegisteredPerNoteController, Opcode::AssignablePerNoteController });
        switch (messageType()) {
        case MessageType::ChannelVoice10:
        case MessageType::ChannelVoice20: return (m_data[0] >> 8) & 0x7F;
            break;
        default: assert(false);
        }
        return 0;
    }

    void setNote(uint8_t value)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff, Opcode::PolyPressure,
                       Opcode::PerNotePitchBend, Opcode::PerNoteManagement,
                       Opcode::RegisteredPerNoteController, Opcode::AssignablePerNoteController });
        assert(value < 128);
        uint32_t mask = value << 8;
        m_data[0] &= 0xFFFF00FF;
        m_data[0] |= mask;
    }

    //! return note from Pitch attribute (for NoteOn & NoteOff) events) if exists else return note()
    uint8_t pitchNote() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (attributeType() == AttributeType::Pitch) {
            return static_cast<uint8_t>(attribute() >> 9);
        }
        return note();
    }

    //! return tuning in semitones from Pitch attribute (for NoteOn & NoteOff) events) if exists else return 0.f
    tuning_t pitchTuning() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (attributeType() == AttributeType::Pitch) {
            return static_cast<tuning_t>((attribute() & 0x1FF) / static_cast<tuning_t>(0x200));
        }
        return 0.f;
    }

    //! return tuning in cents
    tuning_t pitchTuningCents() const
    {
        return pitchTuning() * 100;
    }

    //! 4.2.14.3 @see pitchNote(), pitchTuning()
    void setPitchNote(uint8_t note, tuning_t tuning)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ MessageType::ChannelVoice20 });

        setAttributeType(AttributeType::Pitch);
        uint16_t attribute = static_cast<uint16_t>(tuning * 0x200);
        attribute &= 0x1FF; //for safe clear first 7 bits
        attribute |= static_cast<uint16_t>(note) << 9;
        setAttribute(attribute);
    }

    uint16_t velocity() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (messageType() == MessageType::ChannelVoice20) {
            return static_cast<uint16_t>(m_data[1] >> 16);
        }
        return m_data[0] & 0x7F;
    }

    //!maximum available velocity value for current messageType
    uint16_t maxVelocity() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (messageType() == MessageType::ChannelVoice20) {
            return 0xFFFF;
        }
        return 0x7F;
    }

    void setVelocity(uint16_t value)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });

        if (messageType() == MessageType::ChannelVoice20) {
            uint32_t mask = value << 16;
            m_data[1] &= 0x0000FFFF;
            m_data[1] |= mask;
            return;
        }
        assert(value < 128);
        uint32_t mask = value & 0x7F;
        m_data[0] &= 0xFFFFFF00;
        m_data[0] |= mask;
    }

    //! set velocity as a fraction of 1. Will automatically scale.
    void setVelocityFraction(float value)
    {
        assert(value >= 0.f && value <= 1.f);
        setVelocity(value * maxVelocity());
    }

    //!return velocity in range [0.f, 1.f] independent from message type
    float velocityFraction() const
    {
        return velocity() / static_cast<float>(maxVelocity());
    }

    uint32_t data() const
    {
        switch (messageType()) {
        case MessageType::ChannelVoice10:
            switch (opcode()) {
            case Opcode::PolyPressure:
            case Opcode::ControlChange:
                return m_data[0] & 0x7F;
            case Opcode::ChannelPressure:
                return (m_data[0] >> 8) & 0x7F;
            case Opcode::PitchBend:
                return ((m_data[0] & 0x7F) << 7) | ((m_data[0] & 0x7F00) >> 8);
            default: assert(false);
            }
            MU_FALLTHROUGH();
        case MessageType::ChannelVoice20:
            switch (opcode()) {
            case Opcode::PolyPressure:
            case Opcode::RegisteredPerNoteController:
            case Opcode::AssignablePerNoteController:
            case Opcode::RegisteredController:
            case Opcode::AssignableController:
            case Opcode::RelativeRegisteredController:
            case Opcode::RelativeAssignableController:
            case Opcode::ControlChange:
            case Opcode::ChannelPressure:
            case Opcode::PitchBend:
            case Opcode::PerNotePitchBend:
                return m_data[1];
            default: assert(false);
            }
            MU_FALLTHROUGH();
        default:;     //TODO
        }

        return 0;
    }

    void setData(uint32_t data)
    {
        switch (messageType()) {
        case MessageType::ChannelVoice10:
            switch (opcode()) {
            case Opcode::PolyPressure:
            case Opcode::ControlChange: {
                assert(data < 128);
                uint32_t mask = data & 0x7F;
                m_data[0] &= 0xFFFFFF00;
                m_data[0] |= mask;
                break;
            }
            case Opcode::ChannelPressure: {
                assert(data < 128);
                uint32_t mask = (data & 0x7F) << 8;
                m_data[0] &= 0xFFFF00FF;
                m_data[0] |= mask;
                break;
            }
            case Opcode::PitchBend: {
                data &= 0x3FFF;
                m_data[0] &= 0xFFFF0000;
                //3d byte: r,lsb 4th: r,msb
                m_data[0] |= ((data >> 7) & 0x7F) | ((data & 0x7F) << 8);
                break;
            }
            default: assert(false);
            }
            break;

        case MessageType::ChannelVoice20:
            switch (opcode()) {
            case Opcode::PolyPressure:
            case Opcode::RegisteredPerNoteController:
            case Opcode::AssignablePerNoteController:
            case Opcode::RegisteredController:
            case Opcode::AssignableController:
            case Opcode::RelativeRegisteredController:
            case Opcode::RelativeAssignableController:
            case Opcode::ControlChange:
            case Opcode::ChannelPressure:
            case Opcode::PitchBend:
            case Opcode::PerNotePitchBend:
                m_data[1] = data;
                break;
            default: assert(false);
            }
            break;
        default: assert(false);
        }
    }

    const uint32_t* rawData() const
    {
        return m_data.data();
    }

    /*! return signed value:
     *  float value for Pitch attribute of NoteOn
     *  @see: data() can return pitch centered unsigned raw value depenned on MIDI version
     */
    float pitch() const
    {
        assertOpcode({ Opcode::PitchBend });
        switch (messageType()) {
        case MessageType::ChannelVoice20: return (data() - 0x80000000) / static_cast<float>(0xFFFFFFFF);
        default: /* silence */ break;
        }
        return (data() - 8192) / static_cast<float>(0x3FFF);//MIDI1.0
    }

    uint8_t index() const
    {
        assertOpcode({ Opcode::ControlChange, Opcode::RegisteredPerNoteController, Opcode::AssignablePerNoteController,
                       Opcode::RegisteredController, Opcode::AssignableController, Opcode::RelativeRegisteredController,
                       Opcode::RelativeAssignableController });
        switch (opcode()) {
        case Opcode::ControlChange:
        case Opcode::RegisteredController:
        case Opcode::AssignableController:
        case Opcode::RelativeRegisteredController:
        case Opcode::RelativeAssignableController:
            return (m_data[0] >> 8) & 0x7F;

        case Opcode::RegisteredPerNoteController:
        case Opcode::AssignablePerNoteController:
            return m_data[0] & 0xFF;

        default: assert(false);
        }
        return 0;
    }

    void setIndex(uint8_t value)
    {
        assertOpcode({ Opcode::ControlChange, Opcode::RegisteredPerNoteController, Opcode::AssignablePerNoteController,
                       Opcode::RegisteredController, Opcode::AssignableController, Opcode::RelativeRegisteredController,
                       Opcode::RelativeAssignableController });

        switch (opcode()) {
        case Opcode::ControlChange:
        case Opcode::RegisteredController:
        case Opcode::AssignableController:
        case Opcode::RelativeRegisteredController:
        case Opcode::RelativeAssignableController: {
            assert(value < 128);
            uint32_t mask = value << 8;
            m_data[0] &= 0xFFFF00FF;
            m_data[0] |= mask;
            break;
        }
        case Opcode::RegisteredPerNoteController:
        case Opcode::AssignablePerNoteController: {
            m_data[0] &= 0xFFFFFF00;
            m_data[0] |= value;
            break;
        }
        default: assert(false);
        }
    }

    uint8_t program() const
    {
        assertOpcode({ Opcode::ProgramChange });
        switch (messageType()) {
        case MessageType::ChannelVoice10: return (m_data[0] >> 8) & 0x7F;
            break;
        case MessageType::ChannelVoice20: return (m_data[1] >> 24) & 0x7F;
            break;
        default: assert(false);
        }
        return 0;
    }

    void setProgram(uint8_t value)
    {
        assertOpcode({ Opcode::ProgramChange });
        assert(value < 128);
        switch (messageType()) {
        case MessageType::ChannelVoice10: {
            uint32_t mask = value << 8;
            m_data[0] &= 0xFFFF00FF;
            m_data[0] |= mask;
            break;
        }
        case MessageType::ChannelVoice20: {
            uint32_t mask = value << 24;
            m_data[1] &= 0x00FFFFFF;
            m_data[1] |= mask;
            break;
        }
        default: assert(false);
        }
    }

    uint16_t bank() const
    {
        assertOpcode({ Opcode::ProgramChange, Opcode::RegisteredController, Opcode::AssignableController,
                       Opcode::RelativeRegisteredController, Opcode::RelativeAssignableController });
        assertMessageType({ MessageType::ChannelVoice20 });
        if (opcode() == Opcode::ProgramChange) {
            return ((m_data[1] & 0x7F00) >> 1) | (m_data[1] & 0x7F);
        }
        return (m_data[0] >> 8) & 0x7F;
    }

    void setBank(uint16_t bank)
    {
        assertOpcode({ Opcode::ProgramChange, Opcode::RegisteredController, Opcode::AssignableController,
                       Opcode::RelativeRegisteredController, Opcode::RelativeAssignableController });
        assertMessageType({ MessageType::ChannelVoice20 });
        if (opcode() == Opcode::ProgramChange) {
            m_data[0] |= 0x01; //set BankValid bit
            uint32_t mask = (bank & 0x3F80) << 8 | (bank & 0x7F);
            m_data[1] &= 0xFFFF0000;
            m_data[1] |= mask;
            return;
        }
        assert(bank < 128);
        uint32_t mask = bank << 8;
        m_data[0] &= 0xFFFF00FF;
        m_data[0] |= mask;
    }

    bool isBankValid() const
    {
        assertOpcode({ Opcode::ProgramChange });
        assertMessageType({ MessageType::ChannelVoice20 });
        return m_data[0] & 0x01;
    }

    AttributeType attributeType() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ MessageType::ChannelVoice20 });
        return static_cast<AttributeType>(m_data[0] & 0xFF);
    }

    void setAttributeType(AttributeType type)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ MessageType::ChannelVoice20 });
        m_data[0] &= 0xFFFFFF00;
        m_data[0] |= static_cast<uint32_t>(type);
    }

    void setAttributeType(uint8_t type) { setAttributeType(static_cast<AttributeType>(type)); }

    uint16_t attribute() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (messageType() == MessageType::ChannelVoice20) {
            return m_data[1] & 0xFFFF;
        }
        return 0x00;
    }

    void setAttribute(uint16_t value)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ MessageType::ChannelVoice20 });
        m_data[1] &= 0xFFFF0000;
        m_data[1] |= value;
    }

    void setPerNoteDetach(bool value)
    {
        assertOpcode({ Opcode::PerNoteManagement });
        assertMessageType({ MessageType::ChannelVoice20 });
        m_data[0] &= 0xFFFFFFFD;
        m_data[0] |= static_cast<uint32_t>(value);
    }

    void setPerNoteReset(bool value)
    {
        assertOpcode({ Opcode::PerNoteManagement });
        assertMessageType({ MessageType::ChannelVoice20 });
        m_data[0] &= 0xFFFFFFFE;
        m_data[0] |= static_cast<uint32_t>(value);
    }

    //!convert ChannelVoice from MIDI2.0 to MIDI1.0
    std::vector<Event> toMIDI10() const
    {
        std::vector<Event> events;
        switch (messageType()) {
        case MessageType::ChannelVoice10: events.push_back(*this);
            break;
        case MessageType::ChannelVoice20: {
            auto basic10Event = Event(opcode(), MessageType::ChannelVoice10);
            basic10Event.setChannel(channel());
            basic10Event.setGroup(group());
            switch (opcode()) {
            //D2.1
            case Opcode::NoteOn:
            case Opcode::NoteOff: {
                auto e = basic10Event;
                auto v = scaleDown(velocity(), 16, 7);
                e.setNote(note());
                if (v != 0) {
                    e.setVelocity(static_cast<uint16_t>(v));
                } else {
                    //4.2.2 velocity comment
                    e.setVelocity(1);
                }
                events.push_back(e);
                break;
            }

            //D2.2
            case Opcode::ChannelPressure: {
                auto e = basic10Event;
                e.setData(scaleDown(data(), 32, 7));
                events.push_back(e);
                break;
            }

            //D2.3
            case Opcode::AssignableController:
            case Opcode::RegisteredController: {
                std::vector<std::pair<uint8_t, uint8_t> > controlChanges = {
                    { (opcode() == Opcode::RegisteredController ? 101 : 99), bank() },
                    { (opcode() == Opcode::RegisteredController ? 100 : 98), index() },
                    { 6,  (data() & 0x7FFFFFFF) >> 24 },
                    { 38, (data() & 0x1FC0000) >> 18 }
                };
                for (auto& c : controlChanges) {
                    auto e = basic10Event;
                    e.setOpcode(Opcode::ControlChange);
                    e.setIndex(c.first);
                    e.setData(c.second);
                    events.push_back(e);
                }
                break;
            }

            //D.4
            case Opcode::ProgramChange: {
                if (isBankValid()) {
                    auto e = basic10Event;
                    e.setOpcode(Opcode::ControlChange);
                    e.setIndex(0);
                    e.setData((bank() & 0x7F00) >> 8);
                    events.push_back(e);
                    e.setIndex(0);
                    e.setData(bank() & 0x7F);
                    events.push_back(e);
                }
                auto e = basic10Event;
                e.setProgram(program());
                events.push_back(e);
                break;
            }
            //D2.5
            case Opcode::PitchBend: {
                auto e = basic10Event;
                e.setData(data());
                events.push_back(e);
                break;
            }
            default: break;
            }
            break;
        }
        default: break;
        }
        return events;
    }

    //!convert ChannelVoice from MIDI1.0 to MIDI2.0
    Event toMIDI20(Event chain = NOOP()) const
    {
        Event event;
        switch (messageType()) {
        case MessageType::ChannelVoice20: return *this;
            break;
        case MessageType::ChannelVoice10: {
            if (chain) {
                event = chain;
            } else {
                event = Event(opcode(), MessageType::ChannelVoice20);
                event.setChannel(channel());
                event.setGroup(group());
            }
            switch (opcode()) {
            //D3.1
            case Opcode::NoteOn:
            case Opcode::NoteOff:
                event.setNote(note());
                event.setVelocity(static_cast<uint16_t>(scaleUp(velocity(), 7, 16)));
                if (velocity() == 0) {
                    event.setOpcode(Opcode::NoteOff);
                }
                break;
            //D3.2
            case Opcode::PolyPressure:
                event.setNote(note());
                event.setData(scaleUp(data(), 7, 32));
                break;
            //D3.3
            case Opcode::ControlChange: {
                switch (index()) {
                default:
                    event.setIndex(index());
                    event.setData(scaleUp(data(), 7, 32));
                    break;
                // RPN and NPRN controller messages are no ordenary conrollers
                // and need special handling in MIDI 2.0
                case 99:
                    event.setOpcode(Opcode::AssignableController);
                    event.setBank(static_cast<uint16_t>(data()));
                    break;
                case 101:
                    event.setOpcode(Opcode::RegisteredController);
                    event.setBank(static_cast<uint16_t>(data()));
                    break;
                case 98:
                case 100:
                    event.setIndex(static_cast<uint8_t>(data()));
                    break;
                case 6:
                    event.m_data[0] &= 0x1FFFFFF;
                    event.m_data[0] |= (data() & 0x7F) << 25;
                    break;
                case 38:
                    event.m_data[0] &= 0xFE03FFFF;
                    event.m_data[0] |= (data() & 0x7F) << 18;
                    break;
                }
                break;
            }

            //D3.4
            case Opcode::ProgramChange:
                event.setProgram(program());
                break;

            //D3.5
            case Opcode::ChannelPressure:
                event.setData(scaleUp(data(), 7, 32));
                break;

            //D3.6
            case Opcode::PitchBend:
                event.setData(scaleUp(data(), 14, 32));
                break;

            default: break;
            }
            break;
        }
        default: break;
        }
        return event;
    }

    std::string opcodeString() const
    {
    #define opcodeValueMap(o) { o, std::string(#o) \
}

        static const std::map<Opcode, std::string> m = {
            opcodeValueMap(Opcode::RegisteredPerNoteController),
            opcodeValueMap(Opcode::AssignablePerNoteController),
            opcodeValueMap(Opcode::RegisteredController),
            opcodeValueMap(Opcode::AssignableController),
            opcodeValueMap(Opcode::RelativeRegisteredController),
            opcodeValueMap(Opcode::RelativeAssignableController),
            opcodeValueMap(Opcode::PerNotePitchBend),
            opcodeValueMap(Opcode::NoteOff),
            opcodeValueMap(Opcode::NoteOn),
            opcodeValueMap(Opcode::PolyPressure),
            opcodeValueMap(Opcode::ControlChange),
            opcodeValueMap(Opcode::ProgramChange),
            opcodeValueMap(Opcode::ChannelPressure),
            opcodeValueMap(Opcode::PitchBend),
            opcodeValueMap(Opcode::PerNoteManagement)
        };
    #undef opcodeValueMap
        return muse::value(m, opcode());
    }

    std::string to_string() const
    {
        std::string str;
        auto dataToStr = [&str, this]() {
            str += " {";
            for (auto& d : m_data) {
                str += " " + std::to_string(d);
            }
            str += "}";
        };

        switch (messageType()) {
        case MessageType::ChannelVoice10: {
            str += "MIDI1.0 " + opcodeString();
            str += " group: " + std::to_string(group());
            str += " channel: " + std::to_string(channel());
            switch (opcode()) {
            case Opcode::NoteOn:
            case Opcode::NoteOff:
                str += " note: " + std::to_string(note())
                       + " velocity: " + std::to_string(velocity());
                break;
            case Opcode::PolyPressure:
                str += " note: " + std::to_string(note())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::ControlChange:
                str += " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::ProgramChange:
                str += " program: " + std::to_string(program());
                break;
            case Opcode::ChannelPressure:
                str += " data: " + std::to_string(data());
                break;
            case Opcode::PitchBend:
                str += " value: " + std::to_string(pitch());
                break;
            default: /* silence warning */ break;
            }
            break;
        }
        case MessageType::ChannelVoice20: {
            str += "MIDI2.0 " + opcodeString();
            str += " group: " + std::to_string(group());
            str += " channel: " + std::to_string(channel());
            switch (opcode()) {
            case Opcode::NoteOff:
            case Opcode::NoteOn:
                str += " note: " + std::to_string(note())
                       + " velocity: " + std::to_string(velocity())
                       + " attr type: " + std::to_string(static_cast<uint32_t>(attributeType()))
                       + " attr value: " + std::to_string(attribute());
                if (attributeType() == AttributeType::Pitch) {
                    str += "pitch: note:" + std::to_string(pitchNote()) + " " + std::to_string(pitchTuning()) + " semitone";
                }
                break;
            case Opcode::PolyPressure:
            case Opcode::PerNotePitchBend:
                str += " note: " + std::to_string(note())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::RegisteredPerNoteController:
            case Opcode::AssignablePerNoteController:
                str += " note: " + std::to_string(note())
                       + " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::PerNoteManagement:
                str += " note: " + std::to_string(note())
                       + (m_data[0] & 0b10 ? " detach controller" : "")
                       + (m_data[0] & 0b01 ? " reset controller" : "");
                break;
            case Opcode::ControlChange:
                str += " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::ProgramChange:
                str += " program: " + std::to_string(program())
                       + (isBankValid() ? " set bank: " + std::to_string(bank()) : "");
                break;
            case Opcode::RegisteredController:
            case Opcode::AssignableController:
            case Opcode::RelativeRegisteredController:
            case Opcode::RelativeAssignableController:
                str += " bank: " + std::to_string(bank())
                       + " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case Opcode::ChannelPressure:
                str += " data: " + std::to_string(data());
                break;
            case Opcode::PitchBend:
                str += " value: " + std::to_string(pitch());
                break;
            }

            break;
        }
        case MessageType::Utility:
            str += "MIDI2.0 Utility ";
            if (m_data[0] == 0) {
                str += " NOOP";
                break;
            }
            dataToStr();
            break;
        case MessageType::SystemRealTime:
            str += "MIDI System";
            dataToStr();
            break;
        case MessageType::SystemExclusiveData:
            str += "MIDI System Exclusive";
            dataToStr();
            break;
        case MessageType::Data:
            str += "MIDI2.0 Data";
            dataToStr();
            break;
        }
        return str;
    }

private:
    //!Note Temporarily disabled until the end of the investigation, looks like we're not supporting some 'custom' messages from MU3
    //! v.pereverzev@wsmgroup.ru
    void assertMessageType(const std::set<MessageType>& supportedTypes) const
    {
        UNUSED(supportedTypes);
        //assert(isMessageTypeIn(supportedTypes));
    }

    //!Note Temporarily disabled until the end of the investigation, looks like we're not supporting some 'custom' messages from MU3
    //! v.pereverzev@wsmgroup.ru
    void assertChannelVoice() const { /*assert(isChannelVoice());*/ }

    //!Note Temporarily disabled until the end of the investigation, looks like we're not supporting some 'custom' messages from MU3
    //! v.pereverzev@wsmgroup.ru
    void assertOpcode(const std::set<Opcode>& supportedOpcodes) const
    {
        UNUSED(supportedOpcodes); /*assert(isOpcodeIn(supportedOpcodes));*/
    }

    static uint32_t scaleUp(uint32_t srcVal, size_t srcBits, size_t dstBits)
    {
        // simple bit shift
        size_t scaleBits = dstBits - srcBits;
        uint32_t bitShiftedValue = srcVal << scaleBits;
        uint32_t srcCenter = static_cast<uint32_t>(2 ^ (srcBits - 1));
        if (srcVal <= srcCenter) {
            return bitShiftedValue;
        }
        // expanded bit repeat scheme
        size_t repeatBits = srcBits - 1;
        uint32_t repeatMask = static_cast<uint32_t>((2 ^ repeatBits) - 1);
        uint32_t repeatValue = srcVal & repeatMask;
        if (scaleBits > repeatBits) {
            repeatValue <<= scaleBits - repeatBits;
        } else {
            repeatValue >>= repeatBits - scaleBits;
        }
        while (repeatValue != 0) {
            bitShiftedValue |= repeatValue;
            repeatValue >>= repeatBits;
        }
        return bitShiftedValue;
    }

    static uint32_t scaleDown(uint32_t srcVal, size_t srcBits, size_t dstBits)
    {
        size_t scaleBits = (srcBits - dstBits);
        return srcVal >> scaleBits;
    }

    std::array<uint32_t, 4> m_data = { { 0, 0, 0, 0 } };
};
}

#endif // MUSE_MIDI_MIDIEVENT_H
