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

#include "global/concurrency/ringqueue.h"

using namespace muse;

class Global_Concurrency_RingQueueTests : public ::testing::Test
{
public:
};

struct Msg {
    int val = 0;
};

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto t1 = std::thread([&q]() {
        for (int i = 0; i < 10; ++i) {
            Msg m { i };
            bool ok = q.tryPush(std::move(m));
            EXPECT_TRUE(ok);
        }
    });

    auto t2 = std::thread([&q]() {
        int iteration = 0;
        int successCount = 0;
        while (iteration < 10000) { // anti freeze
            ++iteration;
            Msg m;
            bool ok = q.tryPop(m);
            //! NOTE It might not be ok if the queue is empty
            //! ( didn't have time to add the item)
            if (ok) {
                EXPECT_EQ(m.val, successCount);
                successCount++;
            }

            if (successCount == 10) {
                break;
            }
        }

        EXPECT_EQ(successCount, 10);
    });

    t1.join();
    t2.join();
}

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue_Full)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto t1 = std::thread([&q]() {
        for (int i = 0; i < 17; ++i) {
            Msg m { i };
            bool ok = q.tryPush(std::move(m));
            if (i < 16) {
                EXPECT_TRUE(ok);
            } else {
                EXPECT_FALSE(ok);
            }
        }
    });

    t1.join();

    auto t2 = std::thread([&q]() {
        int iteration = 0;
        int successCount = 0;
        while (iteration < 10000) { // anti freeze
            ++iteration;
            Msg m;
            bool ok = q.tryPop(m);
            //! NOTE It might not be ok if the queue is empty
            //! ( didn't have time to add the item)
            if (ok) {
                EXPECT_EQ(m.val, successCount);
                successCount++;

                if (successCount == 16) {
                    break;
                }
            }
        }

        EXPECT_EQ(successCount, 16);
    });

    t2.join();
}

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue_Empty)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto t1 = std::thread([&q]() {
        for (int i = 0; i < 5; ++i) {
            Msg m { i };
            bool ok = q.tryPush(std::move(m));
            EXPECT_TRUE(ok);
        }
    });

    t1.join();

    auto t2 = std::thread([&q]() {
        int iteration = 0;
        int successCount = 0;
        while (iteration < 100) { // anti freeze
            ++iteration;
            Msg m;
            bool ok = q.tryPop(m);
            if (successCount == 5) {
                EXPECT_FALSE(ok);
            }

            if (ok) {
                EXPECT_EQ(m.val, successCount);
                successCount++;
            }
        }

        EXPECT_EQ(successCount, 5);
    });

    t2.join();
}

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue_PopAll)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto t1 = std::thread([&q]() {
        for (int i = 0; i < 10; ++i) {
            Msg m { i };
            bool ok = q.tryPush(std::move(m));
            EXPECT_TRUE(ok);
        }
    });

    t1.join();

    auto t2 = std::thread([&q]() {
        std::vector<Msg> out;
        bool ok = q.tryPopAll(out);
        EXPECT_TRUE(ok);
        EXPECT_EQ(out.size(), 10);
        for (size_t i = 0; i < out.size(); ++i) {
            EXPECT_EQ(out.at(i).val, i);
        }
    });

    t2.join();
}

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue_FullContinue)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto pushToFull = [&q]() {
        for (int i = 0; i < 17; ++i) {
            Msg m { i };
            bool ok = q.tryPush(std::move(m));
            if (i < 16) {
                EXPECT_TRUE(ok);
            } else {
                EXPECT_FALSE(ok);
            }
        }
    };

    auto popAll = [&q]() {
        int iteration = 0;
        int successCount = 0;
        while (iteration < 10000) { // anti freeze
            ++iteration;
            Msg m;
            bool ok = q.tryPop(m);
            //! NOTE It might not be ok if the queue is empty
            //! ( didn't have time to add the item)
            if (ok) {
                EXPECT_EQ(m.val, successCount);
                successCount++;

                if (successCount == 16) {
                    break;
                }
            }
        }

        EXPECT_EQ(successCount, 16);
    };

    for (int t = 0; t < 10; ++t) {
        auto t1 = std::thread(pushToFull);
        t1.join();

        auto t2 = std::thread(popAll);
        t2.join();
    }
}

TEST_F(Global_Concurrency_RingQueueTests, FixedSizeQueue_SlowRead)
{
    RingQueue<Msg> q(10);

    // next power of two
    EXPECT_EQ(q.capacity(), 16);

    auto t1 = std::thread([&q]() {
        int successIndex = 0;
        while (true) {
            Msg m { successIndex };
            bool ok = q.tryPush(std::move(m));
            if (ok) {
                ++successIndex;
            }

            if (successIndex == 100) {
                break;
            }
        }
    });

    auto t2 = std::thread([&q]() {
        int iteration = 0;
        int successCount = 0;
        while (iteration < 10000) { // anti freeze
            ++iteration;
            Msg m;
            bool ok = q.tryPop(m);
            //! NOTE It might not be ok if the queue is empty
            //! ( didn't have time to add the item)
            if (ok) {
                EXPECT_EQ(m.val, successCount);
                successCount++;

                if (successCount == 100) {
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        EXPECT_EQ(successCount, 100);
    });

    t1.join();
    t2.join();
}
