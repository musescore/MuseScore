/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gtest/gtest.h>

#include "midi/midievent.h"

using namespace muse::midi;

class EventTests : public ::testing::Test
{
protected:
    void SetUp() override {}
};

// ------------------------------------------------------------
// BASIC CONSTRUCTION / BOOL / COMPARISON
// ------------------------------------------------------------

TEST_F(EventTests, Default_IsNoopAndInvalid)
{
    Event e;
    EXPECT_FALSE(e);
    EXPECT_EQ(e, Event::NOOP());
}

TEST_F(EventTests, EqualityAndOrdering)
{
    Event a(Event::Opcode::NoteOn);
    Event b(Event::Opcode::NoteOn);
    Event c(Event::Opcode::NoteOff);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_TRUE(a < c || c < a); // ensure strict ordering works
}

// ------------------------------------------------------------
// MIDI 1.0 PACK / UNPACK
// ------------------------------------------------------------

TEST_F(EventTests, FromMidi10Package_ValidNoteOn)
{
    constexpr uint32_t pkg = (0x90)
                             | (0x40 << 8)
                             | (0x7F << 16);

    Event e = Event::fromMidi10Package(pkg);

    ASSERT_TRUE(e);
    EXPECT_EQ(e.opcode(), Event::Opcode::NoteOn);
    EXPECT_EQ(e.note(), 0x40);
    EXPECT_EQ(e.velocity7(), 0x7F);
}

TEST_F(EventTests, Midi10_Bytes_Roundtrip)
{
    constexpr uint8_t bytes[3] = { 0x90, 0x40, 0x7F };

    Event e = Event::fromMidi10Bytes(bytes, 3);
    ASSERT_TRUE(e);

    uint8_t out[3] = {};
    size_t count = e.toMidi10Bytes(out);

    EXPECT_EQ(count, 3u);
    EXPECT_EQ(out[0], 0x90);
    EXPECT_EQ(out[1], 0x40);
    EXPECT_EQ(out[2], 0x7F);
}

// ------------------------------------------------------------
// SYS EX
// ------------------------------------------------------------

TEST_F(EventTests, SysEx_SinglePacket)
{
    constexpr uint8_t data[] = { 0xF0, 1, 2, 3, 4, 0xF7 };
    std::vector<Event> events = Event::fromMidi10SysExBytes(data, sizeof(data));

    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].messageType(), Event::MessageType::SystemExclusiveData);
}

TEST_F(EventTests, SysEx_Fragmentation)
{
    std::vector<uint8_t> data(20, 0x55);
    std::vector<Event> events = Event::fromMidi10SysExBytes(data.data(), data.size());

    ASSERT_GT(events.size(), 1);
    EXPECT_EQ((events.front().midi20Words()[0] >> 20) & 0xF, 1); // START
    EXPECT_EQ((events.back().midi20Words()[0] >> 20) & 0xF, 3);  // END
}

// ------------------------------------------------------------
// MESSAGE TYPE / GROUP / CHANNEL
// ------------------------------------------------------------

TEST_F(EventTests, MessageType_Group_Channel)
{
    Event e(Event::Opcode::NoteOn);
    e.setGroup(3);
    e.setChannel(5);

    EXPECT_EQ(e.group(), 3);
    EXPECT_EQ(e.channel(), 5);
}

// ------------------------------------------------------------
// NOTE / VELOCITY
// ------------------------------------------------------------

TEST_F(EventTests, NoteAndVelocity_Midi10)
{
    Event e(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice10);
    e.setNote(64);
    e.setVelocity7(100);

    EXPECT_EQ(e.note(), 64);
    EXPECT_EQ(e.velocity7(), 100);
}

TEST_F(EventTests, Velocity16_Midi20)
{
    Event e(Event::Opcode::NoteOn);
    e.setVelocity16(50000);

    EXPECT_EQ(e.velocity16(), 50000);
}

// ------------------------------------------------------------
// DATA / CONTROL CHANGE
// ------------------------------------------------------------

TEST_F(EventTests, ControlChange_Data)
{
    Event e(Event::Opcode::ControlChange);
    e.setIndex(10);
    e.setData(123456);

    EXPECT_EQ(e.index(), 10);
    EXPECT_EQ(e.data(), 123456);
}

// ------------------------------------------------------------
// PROGRAM / BANK
// ------------------------------------------------------------

TEST_F(EventTests, ProgramChange_Midi10)
{
    Event e(Event::Opcode::ProgramChange, Event::MessageType::ChannelVoice10);
    e.setProgram(42);

    EXPECT_EQ(e.program(), 42);
}

TEST_F(EventTests, ProgramChange_Bank_Midi20)
{
    Event e(Event::Opcode::ProgramChange, Event::MessageType::ChannelVoice20);
    e.setBank(42);
    e.setProgram(12);

    EXPECT_TRUE(e.isBankValid());
    EXPECT_EQ(e.bank(), 42);
    EXPECT_EQ(e.program(), 12);
}

// ------------------------------------------------------------
// PITCH BEND
// ------------------------------------------------------------

TEST_F(EventTests, PitchBend_Data14)
{
    Event e(Event::Opcode::PitchBend);
    e.setData(0x2000);

    EXPECT_EQ(e.pitchBend14(), e.data14());
}

// ------------------------------------------------------------
// ATTRIBUTE / PITCH ATTRIBUTE
// ------------------------------------------------------------

TEST_F(EventTests, PitchAttribute)
{
    Event e(Event::Opcode::NoteOn);
    e.setPitchNote(60, 0.5f);

    EXPECT_EQ(e.pitchNote(), 60);
    EXPECT_GT(e.pitchTuning(), 0.0f);
    EXPECT_GT(e.pitchTuningCents(), 0.0f);
}

// ------------------------------------------------------------
// VALIDATION
// ------------------------------------------------------------

TEST_F(EventTests, IsValid_ChannelVoice)
{
    Event e(Event::Opcode::NoteOn);
    EXPECT_TRUE(e.isValid());
}

// ------------------------------------------------------------
// CONVERSION MIDI 2.0 -> MIDI 1.0
// ------------------------------------------------------------

TEST_F(EventTests, ToMidi10_NoteOn)
{
    Event e(Event::Opcode::NoteOn);
    e.setNote(60);
    e.setVelocity7(100);

    std::vector<Event> events = e.toMIDI10();
    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].messageType(), Event::MessageType::ChannelVoice10);
}

// ------------------------------------------------------------
// CONVERSION MIDI 1.0 -> MIDI 2.0
// ------------------------------------------------------------

TEST_F(EventTests, ToMidi20_NoteOn)
{
    constexpr uint8_t data[] = { 0x90, 60, 100 };
    Event e = Event::fromMidi10Bytes(data, 3);
    Event converted = e.toMIDI20();

    EXPECT_EQ(converted.messageType(), Event::MessageType::ChannelVoice20);
    EXPECT_EQ(converted.note(), 60);
}

// ------------------------------------------------------------
// STRING OUTPUT
// ------------------------------------------------------------

TEST_F(EventTests, ToString_NoteOn_Content)
{
    Event e(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice10);
    e.setGroup(2);
    e.setChannel(1);
    e.setNote(60);
    e.setVelocity7(100);

    std::string str = e.to_string();

    // Basic structure
    EXPECT_NE(str.find("MIDI1.0"), std::string::npos);
    EXPECT_NE(str.find("NoteOn"), std::string::npos);

    // Fields
    EXPECT_NE(str.find("group: 2"), std::string::npos);
    EXPECT_NE(str.find("channel: 1"), std::string::npos);
    EXPECT_NE(str.find("note: 60"), std::string::npos);
    EXPECT_NE(str.find("velocity: 100"), std::string::npos);
}

// ------------------------------------------------------------
// STATIC HELPERS
// ------------------------------------------------------------

TEST_F(EventTests, WordCountForMessageType)
{
    EXPECT_EQ(Event::wordCountForMessageType(Event::MessageType::ChannelVoice10), 1u);
    EXPECT_EQ(Event::wordCountForMessageType(Event::MessageType::ChannelVoice20), 2u);
}

TEST_F(EventTests, Midi10ByteCountForOpcode)
{
    EXPECT_EQ(Event::midi10ByteCountForOpcode(Event::Opcode::NoteOn), 3u);
    EXPECT_EQ(Event::midi10ByteCountForOpcode(Event::Opcode::ProgramChange), 2u);
}
