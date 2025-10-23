/*
MIT License

Copyright (c) 2025 Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <thread>
#include <chrono>

#include <gtest/gtest.h>

#include "../rpcqueue/ringqueue.h"

using namespace kors::async;

struct Msg {
    int val = 0;
};

TEST(RingQueue_Tests, Base)
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

TEST(RingQueue_Tests, Full)
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

TEST(RingQueue_Tests, Empty)
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

TEST(RingQueue_Tests, PopAll)
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

TEST(RingQueue_Tests, FullContinue)
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

TEST(RingQueue_Tests, SlowRead)
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
