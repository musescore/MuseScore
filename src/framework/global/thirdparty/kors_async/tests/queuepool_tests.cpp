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
#include <mutex>
#include <thread>
#include <chrono>

#include <gtest/gtest.h>

#include "../async/internal/queuepool.h"

using namespace kors::async;

struct Store {
    std::vector<Queue*> queues;
    std::vector<std::thread::id> threadIds;
    std::mutex mutex;
};

TEST(QueuePool_Tests, Force)
{
    QueuePool* qp = QueuePool::instance();
    Store store;
    std::vector<std::thread> threads;
    std::atomic<bool> running = true;
    for (size_t i = 0; i < 50; ++i) {
        auto t = std::thread([qp, &store, i, &running]() {
            {
                std::scoped_lock<std::mutex> lock(store.mutex);

                store.threadIds.push_back(std::this_thread::get_id());
                if (store.threadIds.size() == 1) {
                    return;
                }
                Queue* q = new Queue();
                store.queues.push_back(q);

                qp->regPort(store.threadIds[i-1], q->port1());
                qp->regPort(store.threadIds[i], q->port2());
            }

            while (running.load()) {
                qp->processMessages(store.threadIds[i]);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        threads.push_back(std::move(t));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    running.store(false);

    for (auto& t : threads) {
        t.join();
    }
}
