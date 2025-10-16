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

#include <thread>
#include <chrono>

#include <gtest/gtest.h>

#include "global/concurrency/rpcqueue.h"

using namespace muse;

class Global_Concurrency_RpcQueueTests : public ::testing::Test
{
public:
};

struct Msg {
    int val = 0;
};

TEST_F(Global_Concurrency_RpcQueueTests, Communication)
{
    RpcQueue<Msg> q;

    const int MSG_COUNT = 400;

    auto port1 = q.port1();
    auto port2 = q.port2();

    auto t1 = std::thread([&port1, MSG_COUNT]() {
        int successCount = 0;

        port1->onMessage([&successCount](const Msg& m) {
            EXPECT_EQ(m.val, successCount);
            successCount++;
        });

        for (int i = 0; i < MSG_COUNT; ++i) {
            Msg m { i };
            port1->send(m);
        }

        int iteration = 0;
        while (iteration < 1000) { // anti freeze
            ++iteration;

            port1->process();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (successCount == MSG_COUNT) {
                break;
            }
        }

        EXPECT_EQ(successCount, MSG_COUNT);
    });

    auto t2 = std::thread([&port2, MSG_COUNT]() {
        int iteration = 0;
        int successCount = 0;

        port2->onMessage([&successCount](const Msg& m) {
            EXPECT_EQ(m.val, successCount);
            successCount++;
        });

        for (int i = 0; i < MSG_COUNT; ++i) {
            Msg m { i };
            port2->send(m);
        }

        while (iteration < 1000) { // anti freeze
            ++iteration;

            port2->process();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (successCount == MSG_COUNT) {
                break;
            }
        }

        EXPECT_EQ(successCount, MSG_COUNT);
    });

    t1.join();
    t2.join();
}
