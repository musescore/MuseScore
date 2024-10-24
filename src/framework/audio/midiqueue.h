#ifndef MUSE_MIDI_MIDIQUEUE_H
#define MUSE_MIDI_MIDIQUEUE_H

#include <thread>
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queue_;
    mutable std::mutex mtx_;

public:
    void push(const T& item)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
    }

    bool pop(T& item)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return false;
        }
        //T* ptr = queue_.front();
        //item = *ptr;
        //delete ptr;
        //queue_.pop();
        //return true;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
};

#endif // MUSE_MIDI_MIDIQUEUE_H
