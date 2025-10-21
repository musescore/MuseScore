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

#include "../async/promise.h"
#include "../async/processevents.h"

using namespace kors;
using namespace kors::async;

TEST(Promise_Tests, SingleThread_Resolve)
{
    int resolvedVal = 0;

    Promise<int> promise = async::make_promise<int>([](auto resolve) {
        return resolve(42);
    });

    promise.onResolve(nullptr, [&resolvedVal](int val) {
        resolvedVal = val;
    });

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 10) {
        ++iteration;
        async::processMessages(thisThId);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_EQ(resolvedVal, 42);
}

TEST(Promise_Tests, MultiThread_Resolve_TempObj)
{
    struct Calculator {
        Promise<int> calc()
        {
            return async::make_promise<int>([](auto resolve, auto reject) {
                auto t = std::thread([resolve, reject]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    std::cout << "defore resolve " << std::endl;
                    (void)resolve(42);
                    std::cout << "after resolve " << std::endl;
                    if (false) {
                        (void)reject(-1, "rejected");
                    }
                });
                t.detach();
                return Promise<int>::dummy_result();
            });
        }
    };

    int resolvedVal = 0;

    {
        Calculator c;

        c.calc().onResolve(nullptr, [&resolvedVal](int val) {
            std::cout << "onResolve val: " << val << std::endl;
            resolvedVal = val;
        }).onReject(nullptr, [](int, const std::string&) {});

        // emulate an event loop in the main thread
        const std::thread::id thisThId = std::this_thread::get_id();
        int iteration = 0;
        while (iteration < 100) {
            ++iteration;
            async::processMessages(thisThId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    EXPECT_EQ(resolvedVal, 42);
}

TEST(Promise_Tests, MultiThread_NoSubscribers)
{
    struct Calculator {
        int value = 0; // just for test
        Promise<int> calc()
        {
            return async::make_promise<int>([this](auto resolve, auto reject) {
                auto t = std::thread([this, resolve, reject]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    value = 42;
                    std::cout << "defore resolve " << std::endl;
                    (void)resolve(value);
                    std::cout << "after resolve " << std::endl;
                    if (false) {
                        (void)reject(-1, "rejected");
                    }
                });
                t.detach();
                return Promise<int>::dummy_result();
            });
        }
    };

    Calculator c;

    c.calc();

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 100) {
        ++iteration;
        async::processMessages(thisThId);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_EQ(c.value, 42);
}

TEST(Promise_Tests, MultiThread_DestroyUnRecieved)
{
    struct Calculator {
        Promise<int> calc()
        {
            return async::make_promise<int>([](auto resolve, auto reject) {
                auto t = std::thread([resolve, reject]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    std::cout << "defore resolve " << std::endl;
                    (void)resolve(42);
                    std::cout << "after resolve " << std::endl;
                    if (false) {
                        (void)reject(-1, "rejected");
                    }
                });
                t.detach();
                return Promise<int>::dummy_result();
            });
        }
    };

    int resolvedVal = 0;

    {
        Calculator c;

        c.calc().onResolve(nullptr, [&resolvedVal](int val) {
            std::cout << "onResolve val: " << val << std::endl;
            resolvedVal = val;
        }).onReject(nullptr, [](int, const std::string&) {});

        // emulate an event loop in the main thread
        const std::thread::id thisThId = std::this_thread::get_id();
        int iteration = 100;
        while (iteration < 100) {
            ++iteration;
            async::processMessages(thisThId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    EXPECT_EQ(resolvedVal, 0);
}
