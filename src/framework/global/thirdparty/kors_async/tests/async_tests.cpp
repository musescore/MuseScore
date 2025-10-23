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

#include "../async/async.h"
#include "../async/processevents.h"

using namespace kors;
using namespace kors::async;

struct Object : public Asyncable
{
    static inline bool s_deleted = false;
    static inline int s_value = 0;
    int value = 0;
    std::thread::id lastCallThId;

    Object()
    {
        s_value = 0;
        s_deleted = false;
    }

    ~Object()
    {
        s_deleted = true;
    }

    void increment()
    {
        EXPECT_FALSE(Object::s_deleted);
        lastCallThId = std::this_thread::get_id();
        ++value;
        ++s_value;
    }

    void incrementAsync(const std::thread::id& thId = std::this_thread::get_id())
    {
        Async::call(this, [this]() {
            increment();
        }, thId);
    }
};

TEST(Async_Tests, SingleThread_Call)
{
    Object obj;
    int calledIteration = -1;

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 100) {
        ++iteration;
        async::processMessages(thisThId);

        if (calledIteration == -1) {
            calledIteration = iteration;
            obj.incrementAsync();
        }

        // not called
        if (iteration == calledIteration) {
            EXPECT_EQ(obj.value, 0);
        }

        if (iteration == (calledIteration + 1)) {
            // called on next iteration
            EXPECT_EQ(obj.value, 1);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_EQ(Object::s_value, 1);
}

TEST(Async_Tests, SingleThread_AutoDisconect)
{
    int calledIteration = -1;

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 100) {
        ++iteration;
        async::processMessages(thisThId);

        Object* obj = new Object();
        if (calledIteration == -1) {
            calledIteration = iteration;
            obj->incrementAsync();
        }

        // not called
        if (iteration == calledIteration) {
            EXPECT_EQ(obj->value, 0);
        }

        delete obj;

        if (iteration == (calledIteration + 1)) {
            // called on next iteration
            EXPECT_EQ(Object::s_value, 0);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_EQ(Object::s_value, 0);
}

TEST(Async_Tests, MultiThread_Call)
{
    static std::thread::id THREAD_ID;

    auto t1 = std::thread([](){
        THREAD_ID = std::this_thread::get_id();

        // emulate an event loop in the thread
        const std::thread::id thisThId = std::this_thread::get_id();
        int iteration = 0;
        while (iteration < 1000) {
            ++iteration;
            async::processMessages(thisThId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // wait thread start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Object obj;
    obj.incrementAsync(THREAD_ID);

    t1.join();

    EXPECT_EQ(obj.value, 1);
    EXPECT_EQ(obj.lastCallThId, THREAD_ID);
}

TEST(Async_Tests, MultiThread_AutoDisconect)
{
    static std::thread::id THREAD_ID;

    auto t1 = std::thread([](){
        THREAD_ID = std::this_thread::get_id();

        // emulate an event loop in the thread
        int iteration = 0;
        while (iteration < 100) {
            ++iteration;
            async::processMessages(THREAD_ID);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // wait thread start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
        Object obj;
        obj.incrementAsync(THREAD_ID);
    }

    t1.join();

    EXPECT_EQ(Object::s_value, 0);
}
