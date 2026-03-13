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

#include "midi/mmc.h"
#include "midi/midievent.h"

using namespace muse::midi;

static constexpr double LOCATE_ERROR(0.000001);

class MMCParserTests : public ::testing::Test
{
protected:
    void SetUp() override {}

    static std::vector<Event> makeSysExEvents(std::initializer_list<uint8_t> bytes)
    {
        std::vector<Event> result;

        const uint8_t* data = bytes.begin();
        const size_t size = bytes.size();

        for (size_t pos = 0; pos < size; pos += 6) {
            const size_t remaining = size - pos;
            const size_t count = remaining < 6 ? remaining : 6;

            const bool first = (pos == 0);
            const bool last = (pos + count >= size);

            const uint8_t status
                = first && last ? 0 // COMPLETE
                  : first ? 1       // START
                  : last ? 3        // END
                  : 2;              // CONTINUE

            uint32_t w0 = 0;
            uint32_t w1 = 0;

            w0 |= (static_cast<uint32_t>(Event::MessageType::SystemExclusiveData) << 28);
            w0 |= (status << 20);
            w0 |= (static_cast<uint32_t>(count) << 16);

            // First 2 bytes -> w0
            if (count > 0) {
                w0 |= data[pos + 0] << 8;
            }
            if (count > 1) {
                w0 |= data[pos + 1];
            }

            // Remaining -> w1
            for (size_t i = 2; i < count; ++i) {
                w1 |= data[pos + i] << (24 - (i - 2) * 8);
            }

            result.emplace_back(std::array<uint32_t, 4> { w0, w1, 0, 0 });
        }

        return result;
    }
};

TEST_F(MMCParserTests, Process_PlayCommand)
{
    // [GIVEN] Play command
    // SysEx: 7F <dev> 06 02
    std::vector<Event> events = makeSysExEvents({ 0x7F, 0x7F, 0x06, 0x02 });
    ASSERT_EQ(events.size(), 1);

    // [WHEN] Parse MMC message
    MMCParser parser;
    std::optional<MMCMessage> msg = parser.process(events.front());

    // [THEN] Message is valid
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->command, MMCCommand::Play);
    EXPECT_EQ(msg->deviceId, 0x7F);
    EXPECT_TRUE(msg->data.empty());
}

TEST_F(MMCParserTests, Process_LocateCommand_FragmentedSysEx)
{
    // [GIVEN] Locate command
    std::vector<Event> events = makeSysExEvents({
        0x7F, 0x7F, 0x06, 0x44,
        0x06, 0x01, 0x96, 0x00,
        0x15, 0x04, 0x00
    });
    ASSERT_EQ(events.size(), 2);

    // [WHEN] Parse MMC message
    MMCParser parser;
    std::optional<MMCMessage> msg;
    for (const Event& event : events) {
        msg = parser.process(event);
    }

    // [THEN] Message is valid
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->command, MMCCommand::Locate);
    EXPECT_EQ(msg->deviceId, 0x7F);
    EXPECT_EQ(msg->data, (std::vector<uint8_t> { 0x06, 0x01, 0x96, 0x00, 0x15, 0x04, 0x00 }));
}

TEST_F(MMCParserTests, Process_NonMMCMessage)
{
    // [GIVEN] Not MMC (missing 0x7F / 0x06)
    std::vector<Event> events = makeSysExEvents({ 0x01, 0x02, 0x03, 0x04 });
    ASSERT_EQ(events.size(), 1);

    // [WHEN] Parse message
    MMCParser parser;
    std::optional<MMCMessage> msg = parser.process(events.front());

    // [THEN] No message
    EXPECT_FALSE(msg.has_value());
}

TEST_F(MMCParserTests, LocateToSeconds_WithFormatByte)
{
    // [GIVEN] format = 1, hr_byte = 96 (30 fps, 0h), min = 0, sec = 15, frame = 4, subframe = 0
    MMCMessage msg;
    msg.command = MMCCommand::Locate;
    msg.data = { 6, 1, 96, 0, 15, 4, 0 };

    // [WHEN] Convert msg to seconds
    std::optional<double> secs = MMCParser::locateToSeconds(msg);

    // [THEN] Result is valid
    ASSERT_TRUE(secs.has_value());

    constexpr double expected = 15.0 + (4.0 / 30.0);
    EXPECT_NEAR(secs.value(), expected, LOCATE_ERROR);
}

TEST_F(MMCParserTests, LocateToSeconds_WithoutFormatByte)
{
    // [GIVEN] hr_byte = 96 (30 fps), min = 0, sec = 15, frame = 4, subframe = 0
    MMCMessage msg;
    msg.command = MMCCommand::Locate;
    msg.data = { 6, 96, 0, 15, 4, 0 };

    // [WHEN] Convert msg to seconds
    std::optional<double> secs = MMCParser::locateToSeconds(msg);

    // [THEN] Result is valid
    ASSERT_TRUE(secs.has_value());

    constexpr double expected = 15.0 + (4.0 / 30.0);
    EXPECT_NEAR(secs.value(), expected, LOCATE_ERROR);
}

TEST_F(MMCParserTests, LocateToSeconds_InvalidFormat)
{
    // [GIVEN] Invalid msg
    MMCMessage msg;
    msg.command = MMCCommand::Locate;
    msg.data = { 6, 2, 96, 0, 15, 4, 0 }; // format != 1

    // [WHEN] Convert msg to seconds
    std::optional<double> secs = MMCParser::locateToSeconds(msg);

    // [THEN]
    EXPECT_FALSE(secs.has_value());
}
