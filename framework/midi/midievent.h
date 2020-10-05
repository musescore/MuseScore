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
#ifndef MU_MIDI_MIDIEVENT_H
#define MU_MIDI_MIDIEVENT_H
#include <cstdint>
#include <array>
#include <set>
#include "audio/midi/event.h"

#ifndef UNUSED
#define UNUSED(x) (void)x;
#endif

namespace mu {
namespace midi {
using channel_t = uint8_t;
using EventType = Ms::EventType;

/*!
 * MIDI Event stored in Universal MIDI Packet Format Message
 * @see M2-104-UM
 * @see midi.org
 */
struct Event {
    enum MessageType {
        Utility             = 0x0,
        SystemRealTime      = 0x1,
        //! MIDI1.0 voice message
        ChannelVoice10      = 0x2,
        SystemExclusiveData = 0x3,
        //! MIDI2.0 voice message
        ChannelVoice20      = 0x4,
        Data                = 0x5
    };

    enum Opcode {
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

    enum AttributeType {
        NoData = 0x00,
        ManufacturerSpecific = 0x01,
        ProfileSpecific = 0x02,
        Pitch = 0x03
    };

    enum UtilityStatus {
        NoOperation        = 0x00,
        JRClock     = 0x01,
        JRTimestamp = 0x02
    };

    Event()
        : m_data({ 0, 0, 0, 0 }) {}
    Event(Opcode opcode, MessageType type = ChannelVoice20)
    {
        setMessageType(type);
        setOpcode(opcode);
    }

    [[deprecated]] Event(channel_t ch, EventType type)
    {
        setType(type);
        setChannel(ch);
    }

    [[deprecated]] Event(channel_t ch, EventType type, uint8_t a = 0, uint8_t b = 0)
    {
        m_data[0] = (a << 8) | b;
        setType(type);
        setChannel(ch);
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
        if (
            (u.byte[0] & 0x80)
            || (u.byte[0] & 0x90)
            || (u.byte[0] & 0xA0)
            || (u.byte[0] & 0xB0)
            || (u.byte[0] & 0xC0)
            || (u.byte[0] & 0xD0)
            || (u.byte[0] & 0xE0)
            ) {
            e.m_data[0] = (u.byte[0] << 16) | (u.byte[1] << 8) | u.byte[2];
            e.setMessageType(ChannelVoice10);
        }
        return e;
    }

    uint32_t to_MIDI10Package() const
    {
        if (messageType() == ChannelVoice10) {
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

    bool operator ==(const Event& other) const { return m_data == other.m_data; }
    bool operator !=(const Event& other) const { return !operator==(other); }
    operator bool() const {
        return operator!=(NOOP()) && isValid();
    }

    bool isChannelVoice() const { return messageType() == ChannelVoice10 || messageType() == ChannelVoice20; }
    bool isChannelVoice20() const { return messageType() == ChannelVoice20; }
    bool isMessageTypeIn(const std::set<MessageType>& types) const { return types.find(messageType()) != types.end(); }
    bool isOpcodeIn(const std::set<Opcode>& opcodes) const { return opcodes.find(opcode()) != opcodes.end(); }

    //! check UMP for correct structure
    bool isValid() const
    {
        switch (messageType()) {
        case Utility: {
            std::set<UtilityStatus> statuses = { NoOperation, JRClock, JRTimestamp };
            return statuses.find(static_cast<UtilityStatus>(status())) != statuses.end();
        }

        case SystemRealTime:
        case SystemExclusiveData:
        case Data:
            return true;

        case ChannelVoice10:
            return isOpcodeIn({ NoteOff, NoteOn, PolyPressure, ControlChange, ProgramChange, ChannelPressure, PitchBend });

        case ChannelVoice20:
            return isOpcodeIn({ RegisteredPerNoteController,
                                AssignablePerNoteController,
                                RegisteredController,
                                AssignableController,
                                RelativeRegisteredController,
                                RelativeAssignableController,
                                PerNotePitchBend,
                                NoteOff,
                                NoteOn,
                                PolyPressure,
                                ControlChange,
                                ProgramChange,
                                ChannelPressure,
                                PitchBend,
                                PerNoteManagement
                              });
        }
        return false;
    }

    MessageType messageType() const { return static_cast<MessageType>(m_data[0] >> 28); }
    void setMessageType(MessageType type)
    {
        uint32_t mask = type << 28;
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
        uint32_t mask = code << 20;
        m_data[0] &= 0xFF0FFFFF;
        m_data[0] |= mask;
    }

    uint8_t status() const
    {
        assertMessageType({ SystemRealTime, Utility });

        switch (messageType()) {
        case SystemRealTime: return (m_data[0] >> 16) & 0b11111111;
        case Utility: return (m_data[0] >> 16) & 0b00001111;
        default: break;
        }
        return 0;
    }

    [[deprecated]] EventType type() const
    {
        if (isChannelVoice()) {
            return static_cast<EventType>(opcode() << 4);
        }
        return EventType::ME_INVALID;
    }

    [[deprecated]] void setType(EventType type)
    {
        std::set<EventType> supportedTypes
            = { EventType::ME_NOTEOFF, EventType::ME_NOTEON, EventType::ME_POLYAFTER, EventType::ME_CONTROLLER, EventType::ME_PROGRAM,
                EventType::ME_AFTERTOUCH, EventType::ME_PITCHBEND };
        assert(supportedTypes.find(type) != supportedTypes.end());

        Opcode code = static_cast<Opcode>(type >> 4);
        setMessageType(ChannelVoice10);
        setOpcode(code);
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
        assertOpcode({ NoteOn, NoteOff, PolyPressure,
                       PerNotePitchBend, PerNoteManagement,
                       RegisteredPerNoteController, AssignablePerNoteController });
        switch (messageType()) {
        case ChannelVoice10:
        case ChannelVoice20: return (m_data[0] >> 8) & 0x7F;
            break;
        default: assert(false);
        }
        return 0;
    }

    void setNote(uint8_t value)
    {
        assertOpcode({ NoteOn, NoteOff, PolyPressure,
                       PerNotePitchBend, PerNoteManagement,
                       RegisteredPerNoteController, AssignablePerNoteController });
        assert(value < 128);
        uint32_t mask = value << 8;
        m_data[0] &= 0xFFFF00FF;
        m_data[0] |= mask;
    }

    //! return note from Pitch attribute (for NoteOn & NoteOff) events) if exists else return note()
    uint8_t pitchNote() const
    {
        assertOpcode({ NoteOn, NoteOff });
        if (attributeType() == AttributeType::Pitch) {
            return attribute() >> 9;
        }
        return note();
    }

    //! return tuning in semitones from Pitch attribute (for NoteOn & NoteOff) events) if exists else return 0.f
    float pitchTuning() const
    {
        assertOpcode({ NoteOn, NoteOff });
        if (attributeType() == AttributeType::Pitch) {
            return (attribute() & 0x1FF) / static_cast<float>(0x200);
        }
        return 0.f;
    }

    //! return tuning in cents
    float pitchTuningCents() const
    {
        return pitchTuning() * 100;
    }

    //! 4.2.14.3 @see pitchNote(), pitchTuning()
    void setPitchNote(uint8_t note, float tuning)
    {
        assertOpcode({ NoteOn, NoteOff });
        assertMessageType({ ChannelVoice20 });

        setAttributeType(AttributeType::Pitch);
        uint16_t attribute = static_cast<uint16_t>(tuning * 0x200);
        attribute &= 0x1FF; //for safe clear first 7 bits
        attribute |= static_cast<uint16_t>(note) << 9;
        setAttribute(attribute);
    }

    uint16_t velocity() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (messageType() == ChannelVoice20) {
            return m_data[1] >> 16;
        }
        return m_data[0] & 0x7F;
    }

    //!maximum available velocity value for current messageType
    uint16_t maxVelocity() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        if (messageType() == ChannelVoice20) {
            return 0xFFFF;
        }
        return 0x7F;
    }

    void setVelocity(uint16_t value)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });

        if (messageType() == ChannelVoice20) {
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
    float velocityFraction()
    {
        return velocity() / static_cast<float>(maxVelocity());
    }

    uint32_t data() const
    {
        switch (messageType()) {
        case ChannelVoice10:
            switch (opcode()) {
            case PolyPressure:
            case ControlChange:
                return m_data[0] & 0x7F;
            case ChannelPressure:
                return (m_data[0] >> 8) & 0x7F;
            case PitchBend:
                return ((m_data[0] & 0x7F) << 7) | ((m_data[0] & 0x7F00) >> 8);
            default: assert(false);
            }
        case ChannelVoice20:
            switch (opcode()) {
            case PolyPressure:
            case RegisteredPerNoteController:
            case AssignablePerNoteController:
            case RegisteredController:
            case AssignableController:
            case RelativeRegisteredController:
            case RelativeAssignableController:
            case ControlChange:
            case ChannelPressure:
            case PitchBend:
            case PerNotePitchBend:
                return m_data[1];
            default: assert(false);
            }

        default:;     //TODO
        }

        return 0;
    }

    void setData(uint32_t data)
    {
        switch (messageType()) {
        case ChannelVoice10:
            switch (opcode()) {
            case PolyPressure:
            case ControlChange: {
                assert(data < 128);
                uint32_t mask = data & 0x7F;
                m_data[0] &= 0xFFFFFF00;
                m_data[0] |= mask;
                break;
            }
            case ChannelPressure: {
                assert(data < 128);
                uint32_t mask = (data & 0x7F) << 8;
                m_data[0] &= 0xFFFF00FF;
                m_data[0] |= mask;
                break;
            }
            case PitchBend: {
                data &= 0x3FFF;
                m_data[0] &= 0xFFFF0000;
                //3d byte: r,lsb 4th: r,msb
                m_data[0] |= ((data >> 7) & 0x7F) | ((data & 0x7F) << 8);
                break;
            }
            default: assert(false);
            }
            break;

        case ChannelVoice20:
            switch (opcode()) {
            case PolyPressure:
            case RegisteredPerNoteController:
            case AssignablePerNoteController:
            case RegisteredController:
            case AssignableController:
            case RelativeRegisteredController:
            case RelativeAssignableController:
            case ControlChange:
            case ChannelPressure:
            case PitchBend:
            case PerNotePitchBend:
                m_data[1] = data;
                break;
            default: assert(false);
            }
            break;
        default: assert(false);
        }
    }

    /*! return signed value:
     *  float value for Pitch attribute of NoteOn
     *  @see: data() can return pitch centered unsigned raw value depenned on MIDI version
     */
    float pitch() const
    {
        assertOpcode({ PitchBend });
        switch (messageType()) {
        case ChannelVoice20: return (data() - 0x80000000) / static_cast<float>(0xFFFFFFFF);
        default: /* silence */ break;
        }
        return (data() - 8192) / static_cast<float>(0x3FFF);//MIDI1.0
    }

    uint8_t index() const
    {
        assertOpcode({ ControlChange, RegisteredPerNoteController, AssignablePerNoteController,
                       RegisteredController, AssignableController, RelativeRegisteredController, RelativeAssignableController });
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
        assertOpcode({ ControlChange, RegisteredPerNoteController, AssignablePerNoteController,
                       RegisteredController, AssignableController, RelativeRegisteredController, RelativeAssignableController });

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
        case ChannelVoice10: return (m_data[0] >> 8) & 0x7F;
            break;
        case ChannelVoice20: return (m_data[1] >> 24) & 0x7F;
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
        case ChannelVoice10: {
            uint32_t mask = value << 8;
            m_data[0] &= 0xFFFF00FF;
            m_data[0] |= mask;
            break;
        }
        case ChannelVoice20: {
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
        assertOpcode({ ProgramChange, RegisteredController, AssignableController, RelativeRegisteredController,
                       RelativeAssignableController });
        assertMessageType({ ChannelVoice20 });
        if (opcode() == ProgramChange) {
            return ((m_data[1] & 0x7F00) >> 1) | (m_data[1] & 0x7F);
        }
        return (m_data[0] >> 8) & 0x7F;
    }

    void setBank(uint16_t bank)
    {
        assertOpcode({ ProgramChange, RegisteredController, AssignableController, RelativeRegisteredController,
                       RelativeAssignableController });
        assertMessageType({ ChannelVoice20 });
        if (opcode() == ProgramChange) {
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
        assertOpcode({ ProgramChange });
        assertMessageType({ ChannelVoice20 });
        return m_data[0] & 0x01;
    }

    AttributeType attributeType() const
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ ChannelVoice20 });
        return static_cast<AttributeType>(m_data[0] & 0xFF);
    }

    void setAttributeType(AttributeType type)
    {
        assertOpcode({ Opcode::NoteOn, Opcode::NoteOff });
        assertMessageType({ ChannelVoice20 });
        m_data[0] &= 0xFFFFFF00;
        m_data[0] |= type;
    }

    void setAttributeType(uint8_t type) { setAttributeType(static_cast<AttributeType>(type)); }

    uint16_t attribute() const
    {
        assertOpcode({ NoteOn, NoteOff });
        if (messageType() == ChannelVoice20) {
            return m_data[1] & 0xFFFF;
        }
        return 0x00;
    }

    void setAttribute(uint16_t value)
    {
        assertOpcode({ NoteOn, NoteOff });
        assertMessageType({ ChannelVoice20 });
        m_data[1] &= 0xFFFF0000;
        m_data[1] |= value;
    }

    void setPerNoteDetach(bool value)
    {
        assertOpcode({ PerNoteManagement });
        assertMessageType({ ChannelVoice20 });
        m_data[0] &= 0xFFFFFFFD;
        m_data[0] |= value;
    }

    void setPerNoteReset(bool value)
    {
        assertOpcode({ PerNoteManagement });
        assertMessageType({ ChannelVoice20 });
        m_data[0] &= 0xFFFFFFFE;
        m_data[0] |= value;
    }

    //!convert ChannelVoice from MIDI2.0 to MIDI1.0
    std::list<Event> toMIDI10() const
    {
        std::list<Event> events;
        switch (messageType()) {
        case ChannelVoice10: events.push_back(*this);
            break;
        case ChannelVoice20: {
            auto basic10Event = Event(opcode(), ChannelVoice10);
            basic10Event.setChannel(channel());
            basic10Event.setGroup(group());
            switch (opcode()) {
            //D2.1
            case NoteOn:
            case NoteOff: {
                auto e = basic10Event;
                auto v = scaleDown(velocity(), 16, 7);
                e.setNote(note());
                if (v != 0) {
                    e.setVelocity(v);
                } else {
                    //4.2.2 velocity comment
                    e.setVelocity(1);
                }
                events.push_back(e);
                break;
            }

            //D2.2
            case ChannelPressure: {
                auto e = basic10Event;
                e.setData(scaleDown(data(), 32, 7));
                events.push_back(e);
                break;
            }

            //D2.3
            case AssignableController:
            case RegisteredController: {
                std::list<std::pair<uint8_t, uint8_t> > controlChanges = {
                    { (opcode() == RegisteredController ? 101 : 99), bank() },
                    { (opcode() == RegisteredController ? 100 : 98), index() },
                    { 6,  (data() & 0x7FFFFFFF) >> 24 },
                    { 38, (data() & 0x1FC0000) >> 18 }
                };
                for (auto& c : controlChanges) {
                    auto e = basic10Event;
                    e.setOpcode(ControlChange);
                    e.setIndex(c.first);
                    e.setData(c.second);
                    events.push_back(e);
                }
                break;
            }

            //D.4
            case ProgramChange: {
                if (isBankValid()) {
                    auto e = basic10Event;
                    e.setOpcode(ControlChange);
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
            case PitchBend: {
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
        case ChannelVoice20: return *this;
            break;
        case ChannelVoice10: {
            if (chain) {
                event = chain;
            } else {
                event = Event(opcode(), ChannelVoice20);
                event.setChannel(channel());
                event.setGroup(group());
            }
            switch (opcode()) {
            //D3.1
            case NoteOn:
            case NoteOff:
                event.setNote(note());
                event.setVelocity(scaleUp(velocity(), 7, 16));
                if (velocity() == 0) {
                    event.setOpcode(NoteOff);
                }
                break;
            //D3.2
            case PolyPressure:
                event.setNote(note());
                event.setData(scaleUp(data(), 7, 32));
                break;
            //D3.3
            case ControlChange: {
                std::set<uint8_t> skip = { 6, 38, 98, 99, 100, 101 };
                if (skip.find(index()) == skip.end()) {
                    break;
                }
                switch (index()) {
                case 99:
                    event.setOpcode(AssignableController);
                    event.setBank(data());
                    break;
                case 101:
                    event.setOpcode(RegisteredController);
                    event.setBank(data());
                    break;
                case 98:
                case 100:
                    event.setIndex(data());
                    break;
                case 6:
                    event.m_data[0] &= 0x1FFFFFF;
                    event.m_data[0] |= (data() & 0x7F) << 25;
                    break;
                case 38:
                    event.m_data[0] &= 0xFE03FFFF;
                    event.m_data[0] |= (data() & 0x7F) << 18;
                    break;
                default:
                    event.setIndex(index());
                    event.setData(scaleUp(data(), 7, 32));
                }
                break;
            }

            //D3.4
            case ProgramChange:
                event.setProgram(program());
                break;

            //D3.5
            case ChannelPressure:
                event.setData(scaleUp(data(), 7, 32));
                break;

            //D3.6
            case PitchBend:
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

        static std::map<Opcode, std::string> m = {
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
        return m[opcode()];
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
        case ChannelVoice10: {
            str += "MIDI1.0 " + opcodeString();
            str += " group: " + std::to_string(group());
            str += " channel: " + std::to_string(channel());
            switch (opcode()) {
            case NoteOn:
            case NoteOff:
                str += " note: " + std::to_string(note())
                       + " velocity: " + std::to_string(velocity());
                break;
            case PolyPressure:
                str += " note: " + std::to_string(note())
                       + " data: " + std::to_string(data());
                break;
            case ControlChange:
                str += " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case ProgramChange:
                str += " program: " + std::to_string(program());
                break;
            case ChannelPressure:
                str += " data: " + std::to_string(data());
                break;
            case PitchBend:
                str += " value: " + std::to_string(pitch());
                break;
            default: /* silence warning */ break;
            }
            break;
        }
        case ChannelVoice20: {
            str += "MIDI2.0 " + opcodeString();
            str += " group: " + std::to_string(group());
            str += " channel: " + std::to_string(channel());
            switch (opcode()) {
            case NoteOff:
            case NoteOn:
                str += " note: " + std::to_string(note())
                       + " velocity: " + std::to_string(velocity())
                       + " attr type: " + std::to_string(attributeType())
                       + " attr value: " + std::to_string(attribute());
                if (attributeType() == AttributeType::Pitch) {
                    str += "pitch: note:" + std::to_string(pitchNote()) + " " + std::to_string(pitchTuning()) + " semitone";
                }
                break;
            case PolyPressure:
            case PerNotePitchBend:
                str += " note: " + std::to_string(note())
                       + " data: " + std::to_string(data());
                break;
            case RegisteredPerNoteController:
            case AssignablePerNoteController:
                str += " note: " + std::to_string(note())
                       + " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case PerNoteManagement:
                str += " note: " + std::to_string(note())
                       + (m_data[0] & 0b10 ? " detach controller" : "")
                       + (m_data[0] & 0b01 ? " reset controller" : "");
                break;
            case ControlChange:
                str += " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case ProgramChange:
                str += " program: " + std::to_string(program())
                       + (isBankValid() ? " set bank: " + std::to_string(bank()) : "");
                break;
            case RegisteredController:
            case AssignableController:
            case RelativeRegisteredController:
            case RelativeAssignableController:
                str += " bank: " + std::to_string(bank())
                       + " index: " + std::to_string(index())
                       + " data: " + std::to_string(data());
                break;
            case ChannelPressure:
                str += " data: " + std::to_string(data());
                break;
            case PitchBend:
                str += " value: " + std::to_string(pitch());
                break;
            }

            break;
        }
        case Utility:
            str += "MIDI2.0 Utility ";
            if (m_data[0] == 0) {
                str += " NOOP";
                break;
            }
            dataToStr();
            break;
        case SystemRealTime:
            str += "MIDI System";
            dataToStr();
            break;
        case SystemExclusiveData:
            str += "MIDI System Exlusive";
            dataToStr();
            break;
        case Data:
            str += "MIDI2.0 Data";
            dataToStr();
            break;
        }
        return str;
    }

private:
    void assertMessageType(const std::set<MessageType>& supportedTypes) const
    {
        UNUSED(supportedTypes);
        assert(isMessageTypeIn(supportedTypes));
    }

    void assertChannelVoice() const { assert(isChannelVoice()); }

    void assertOpcode(const std::set<Opcode>& supportedOpcodes) const { UNUSED(supportedOpcodes); assert(isOpcodeIn(supportedOpcodes)); }

    static uint32_t scaleUp(uint32_t srcVal, size_t srcBits, size_t dstBits)
    {
        // simple bit shift
        size_t scaleBits = dstBits - srcBits;
        uint32_t bitShiftedValue = srcVal << scaleBits;
        uint32_t srcCenter = 2 ^ (srcBits - 1);
        if (srcVal <= srcCenter) {
            return bitShiftedValue;
        }
        // expanded bit repeat scheme
        size_t repeatBits = srcBits - 1;
        uint32_t repeatMask = (2 ^ repeatBits) - 1;
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
}
#endif // MU_MIDI_MIDIEVENT_H
