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

#include "../async/internal/objectpool.h"

using namespace kors::async;

struct ThreadData {
    const std::thread::id threadId;
    int data = 0;

    ThreadData(const std::thread::id& thId)
        : threadId(thId) {}
};

TEST(ObjectPool_Tests, ThreadData_Get)
{
    ObjectPool<ThreadData*> threads(10);

    const std::thread::id thisThId = std::this_thread::get_id();

    ThreadData* thdata = threads.tryGet(
        [thisThId](ThreadData* td) { return td->threadId == thisThId; },
        [thisThId] () { return new ThreadData(thisThId); }
        );

    EXPECT_TRUE(thdata);
}

TEST(ObjectPool_Tests, ThreadData_MultiThreadGet)
{
    ObjectPool<ThreadData*> thdatas(10);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; ++i) {
        auto t = std::thread([&thdatas]() {
            const std::thread::id thisThId = std::this_thread::get_id();

            ThreadData* thdata = thdatas.tryGet(
                [thisThId](ThreadData* td) { return td->threadId == thisThId; },
                [thisThId] () { return new ThreadData(thisThId); }
                );

            thdata->data++;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });

        threads.push_back(std::move(t));
    }

    for (auto& t : threads) {
        t.join();
    }

    for (size_t i = 0; i < thdatas.count(); ++i) {
        ThreadData* thdata = thdatas.at(i);
        EXPECT_EQ(thdata->data, 1);
    }
}

struct Item {
    std::atomic<bool> locked = false;
    int val = 0;

    Item(bool lock)
        : locked(lock) {}

    bool tryLock()
    {
        bool expected = false;
        if (locked.compare_exchange_weak(expected, true)) {
            return true;
        }
        return false;
    }

    void unlock()
    {
        locked.store(false);
    }
};

using SharedItem = std::shared_ptr<Item>;

TEST(ObjectPool_Tests, ShareItem_MultiThreadGet)
{
    ObjectPool<SharedItem> items(10);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; ++i) {
        auto t = std::thread([&items]() {
            SharedItem item = items.tryGet(
                [](SharedItem item) { return item->tryLock(); },
                [] () { return std::make_shared<Item>(true); }
                );

            item->val++;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });

        threads.push_back(std::move(t));
    }

    for (auto& t : threads) {
        t.join();
    }

    for (size_t i = 0; i < items.count(); ++i) {
        SharedItem item = items.at(i);
        EXPECT_EQ(item->val, 1);
    }
}
