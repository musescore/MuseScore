/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "audio/common/async/processevents.h"
#include "audio/common/async/channel.h"

using namespace muse;
using namespace muse::audio;

class Audio_AsyncChannelTests : public ::testing::Test
{
public:
};

TEST_F(Audio_AsyncChannelTests, SingleThread_Send)
{
    comm::Channel<int, int> ch;

    ch.onReceive(nullptr, [](const int& v1, const int& v2) {
        EXPECT_EQ(v1, 42);
        EXPECT_EQ(v2, 73);
    });

    ch.send(42, 73);
}

TEST_F(Audio_AsyncChannelTests, MultiThread_Send)
{
    comm::Channel<int, int> ch;

    bool received = false;
    auto t1 = std::thread([ch, &received]() {
        comm::Channel<int, int> _ch = ch;
        _ch.onReceive(nullptr, [&received](const int& v1, const int& v2) {
            received = true;
            EXPECT_EQ(v1, 42);
            EXPECT_EQ(v2, 73);
        });

        int iteration = 0;
        while (iteration < 100) {
            ++iteration;
            comm::processMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // wait th start and subscribe
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ch.send(42, 73);

    t1.join();

    EXPECT_TRUE(received);
}
