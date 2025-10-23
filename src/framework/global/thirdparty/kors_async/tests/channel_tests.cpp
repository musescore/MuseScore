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

#include "../async/channel.h"
#include "../async/processevents.h"

using namespace kors;
using namespace kors::async;

//! NOTE For example, this could be some kind of service.
struct Sender {
    int value = 0;
    Channel<int> ch;

    void increment()
    {
        ++value;
        ch.send(value);
    }

    Channel<int> valueChanged() const { return ch; }
};

//! NOTE This could be some kind of ViewModel that needs to update data in View.
//! Or some other service
struct Receiver : public Asyncable {
    int value = 0;
    const Sender* sender = nullptr;

    void setSender(const Sender* s)
    {
        if (sender) {
            sender->valueChanged().disconnect(this);
        }

        sender = s;
        if (sender) {
            sender->valueChanged().onReceive(this, [this](const int& val) {
                value = val;
            });
        }
    }

    int value2 = 0;
    const Sender* sender2 = nullptr;

    void setSender2(const Sender* s)
    {
        if (sender2) {
            sender2->valueChanged().disconnect(this);
        }

        sender2 = s;
        if (sender2) {
            sender2->valueChanged().onReceive(this, [this](const int& val) {
                value2 = val;
            });
        }
    }
};

TEST(Channel_Tests, SingleThread_Send)
{
    Channel<int> ch;

    int receivedVal = 0;

    ch.onReceive(nullptr, [&receivedVal](const int& v) {
        receivedVal = v;
    });

    ch.send(42);

    EXPECT_EQ(receivedVal, 42);
}

TEST(Channel_Tests, SingleThread_Send_FromObject)
{
    Sender sender;

    int receivedVal = 0;

    sender.valueChanged().onReceive(nullptr, [&receivedVal](const int& v) {
        receivedVal = v;
    });

    EXPECT_EQ(receivedVal, 0);

    sender.increment();
    EXPECT_EQ(receivedVal, 1);

    sender.increment();
    EXPECT_EQ(receivedVal, 2);
}

TEST(Channel_Tests, SingleThread_Send_Disconnect)
{
    Asyncable asyncable;
    Sender sender;

    int receivedVal = 0;

    sender.valueChanged().onReceive(&asyncable, [&receivedVal](const int& v) {
        receivedVal = v;
    });

    EXPECT_EQ(receivedVal, 0);

    sender.increment();
    EXPECT_EQ(receivedVal, 1);

    sender.valueChanged().disconnect(&asyncable);

    sender.increment();
    EXPECT_EQ(receivedVal, 1);
}

TEST(Channel_Tests, SingleThread_Send_onReceive_Disconnect)
{
    Asyncable asyncable;
    Sender sender;

    int receivedVal = 0;

    sender.valueChanged().onReceive(&asyncable, [&receivedVal, &sender, &asyncable](const int& v) {
        receivedVal = v;
        sender.valueChanged().disconnect(&asyncable);
    });

    EXPECT_EQ(receivedVal, 0);

    sender.increment();
    EXPECT_EQ(receivedVal, 1);

    sender.increment();
    EXPECT_EQ(receivedVal, 1);
}

TEST(Channel_Tests, SingleThread_Sender_Receiver)
{
    Sender sender;
    Receiver receiver;

    receiver.setSender(&sender);

    EXPECT_EQ(receiver.value, 0);

    sender.increment();
    EXPECT_EQ(receiver.value, 1);

    receiver.setSender(nullptr);

    sender.increment();
    EXPECT_EQ(receiver.value, 1);
}

TEST(Channel_Tests, SingleThread_MultiReceivers)
{
    Sender sender;
    Receiver receiver1;
    Receiver receiver2;

    receiver1.setSender(&sender);
    receiver2.setSender(&sender);

    EXPECT_EQ(receiver1.value, 0);
    EXPECT_EQ(receiver2.value, 0);

    sender.increment();
    EXPECT_EQ(receiver1.value, 1);
    EXPECT_EQ(receiver2.value, 1);

    receiver1.setSender(nullptr);

    sender.increment();
    EXPECT_EQ(receiver1.value, 1);
    EXPECT_EQ(receiver2.value, 2);
}

TEST(Channel_Tests, SingleThread_MultiSenders)
{
    Receiver receiver;
    {
        Sender sender1;
        Sender sender2;

        receiver.setSender(&sender1);
        receiver.setSender2(&sender2);

        EXPECT_EQ(receiver.value, 0);
        EXPECT_EQ(receiver.value2, 0);

        sender1.increment();
        sender2.increment();
        EXPECT_EQ(receiver.value, 1);
        EXPECT_EQ(receiver.value2, 1);

        receiver.setSender(nullptr);

        sender1.increment();
        sender2.increment();
        EXPECT_EQ(receiver.value, 1);
        EXPECT_EQ(receiver.value2, 2);
    }
}

TEST(Channel_Tests, SingleThread_AutoDisconect)
{
    Sender sender;
    {
        Receiver receiver;
        receiver.setSender(&sender);

        EXPECT_EQ(receiver.value, 0);
        EXPECT_TRUE(sender.ch.isConnected());

        sender.increment();
        EXPECT_EQ(receiver.value, 1);
    }

    // the receiver has been deleted and unsubscribed.
    EXPECT_FALSE(sender.ch.isConnected());
    sender.increment();
}

TEST(Channel_Tests, MultiThread_SendToThread)
{
    Channel<int, int> ch;

    bool received = false;
    auto t1 = std::thread([ch, &received]() {
        Channel<int, int> _ch = ch;
        _ch.onReceive(nullptr, [&received](const int& v1, const int& v2) {
            received = true;
            EXPECT_EQ(v1, 42);
            EXPECT_EQ(v2, 73);
        });

        const std::thread::id thisThId = std::this_thread::get_id();
        int iteration = 0;
        while (iteration < 100) {
            ++iteration;
            async::processMessages(thisThId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // wait th start and subscribe
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ch.send(42, 73);

    t1.join();

    EXPECT_TRUE(received);
}

TEST(Channel_Tests, MultiThread_ReceiveFromThread)
{
    Channel<int> ch;

    int receivedVal = 0;
    ch.onReceive(nullptr, [&receivedVal](const int& val) {
        // main thread
        EXPECT_EQ(val, 42);
        receivedVal = val;
    });

    auto t1 = std::thread([](Channel<int> ch) {
        // some kind of calculation or data acquisition
        int val = 40 + 2;
        ch.send(val);
    }, ch);

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 100) {
        ++iteration;
        async::processMessages(thisThId);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    t1.join();

    EXPECT_EQ(receivedVal, 42);
}

TEST(Channel_Tests, MultiThread_ReceiveFromThread_ResetOnReceive)
{
    Asyncable asyncable;
    Channel<int> ch;

    int receivedVal = 0;
    ch.onReceive(&asyncable, [&receivedVal, &ch, &asyncable](const int& val) {
        // main thread
        EXPECT_EQ(val, 42);
        receivedVal = val;
        ch.disconnect(&asyncable);
    });

    auto t1 = std::thread([](Channel<int> ch) {
        // some kind of calculation or data acquisition
        int val = 40 + 2;
        ch.send(val);
        val = 70 + 30;
        ch.send(val);
    }, ch);

    // emulate an event loop in the main thread
    const std::thread::id thisThId = std::this_thread::get_id();
    int iteration = 0;
    while (iteration < 100) {
        ++iteration;
        async::processMessages(thisThId);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    t1.join();

    EXPECT_EQ(receivedVal, 42);
}

TEST(Channel_Tests, MultiThread_DestroyUnRecieved)
{
    int receivedVal = 0;

    {
        Asyncable asyncable;
        Channel<int, int> ch;

        ch.onReceive(&asyncable, [&receivedVal, &ch, &asyncable](const int& val1, const int& val2) {
            // main thread
            EXPECT_EQ(val1, 42);
            EXPECT_EQ(val2, 73);
        });

        auto t1 = std::thread([](Channel<int, int> ch) {
            for (int i = 0; i < 1000; ++i) {
                ch.send(42, 73);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }, ch);

        // emulate an event loop in the main thread
        const std::thread::id thisThId = std::this_thread::get_id();
        int iteration = 0;
        while (iteration < 100) {
            ++iteration;
            async::processMessages(thisThId);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        t1.join();
    }

    EXPECT_EQ(receivedVal, 0);
}

TEST(Channel_Tests, SingleThread_ArgTypes)
{
    {
        Channel<int> ch;
        ch.send(42);
    }

    {
        Channel<int, int> ch;
        ch.send(42, 73);
    }

    struct Obj
    {
        int val = 0;
    };

    {
        Channel<Obj> ch;
        ch.send(Obj());
    }

    {
        Channel<Obj*> ch;
        Obj obj;
        ch.send(&obj);
    }

    {
        Channel<const Obj*> ch;
        Obj obj;
        ch.send(&obj);
    }

    {
        Channel<const Obj&> ch;
        Obj obj;
        ch.send(obj);
    }

    struct Obj2
    {
        int val = 0;

        Obj2(int v)
            : val(v) {}
    };

    // there must be a default constructor
    // {
    //     Channel<Obj2> ch;
    //     Obj2 obj(42);
    //     ch.send(obj);
    // }

    {
        Channel<Obj2*> ch;
        Obj2 obj(42);
        ch.send(&obj);
    }

    {
        Channel<const Obj2*> ch;
        Obj2 obj(42);
        ch.send(&obj);
    }

    // there must be a default constructor
    // {
    //     Channel<const Obj2&> ch;
    //     Obj2 obj(42);
    //     ch.send(obj);
    // }
}
