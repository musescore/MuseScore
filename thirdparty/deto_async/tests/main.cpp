#include <functional>
#include <iostream>
#include "../async/channel.h"
#include "../async/notification.h"

using namespace deto::async;

struct Counter {
    Channel<int> m_ch;
    int m_val = 0;

    void increment()
    {
        ++m_val;
        m_ch.send(m_val);
    }

    Channel<int> channel() const
    {
        return m_ch;
    }
};

struct Receiver : public deto::async::Asyncable
{
    Channel<int> m_ch;
    Notification m_nt;

    Receiver(const Channel<int>& ch)
        : m_ch(ch)
    {
        m_ch.onReceive(this, [](int a) {
            std::cout << "Receiver channel: " << a << "\n";
        });
    }

    Receiver(const Notification& nt)
        : m_nt(nt)
    {
        m_nt.onNotify(this, []() {
            std::cout << "Receiver notification\n";
        });
    }
};

struct Notifer
{
    Notification m_nt;
    Notification notify() const
    {
        return m_nt;
    }

    void send()
    {
        m_nt.notify();
    }
};

int main(int argc, char* argv[])
{
    std::cout << "Hello World, I am async\n";

    Channel<int> ch1;
    ch1.onReceive(nullptr, [](int a) {
        std::cout << "onReceive a: " << a << "\n";
    });

    for (int i = 0; i < 5; ++i) {
        ch1.send(i);
    }

    Counter counter;

    Channel<int> ch2 = counter.channel();
    ch2.onReceive(nullptr, [](int a) {
        std::cout << "counter: " << a << "\n";
    });

    for (int i = 0; i < 5; ++i) {
        counter.increment();
    }

    {
        Receiver r(ch2);
        for (int i = 0; i < 5; ++i) {
            counter.increment();
        }
    }

    for (int i = 0; i < 5; ++i) {
        counter.increment();
    }

    Notification nt1;
    nt1.onNotify(nullptr, []() {
        std::cout << "Notification 1 \n";
    });

    nt1.notify();

    Notifer ntr;
    Notification nt2 = ntr.notify();
    {
        Receiver r(nt2);
        for (int i = 0; i < 5; ++i) {
            ntr.send();
        }
    }
}
