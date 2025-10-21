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

#include "../async/internal/rpcqueue.h"

using namespace kors::async;

struct Msg {
    int val = 0;
};

TEST(RpcQueue_Tests, Communication)
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
