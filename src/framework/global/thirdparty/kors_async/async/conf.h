/*
MIT License

Copyright (c) Igor Korsukov

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
#pragma once

#include <cstddef>
#include <atomic>

namespace kors::async {
struct conf {
    //! Total number of threads in the application
    //! that can interact through this infrastructure
    static size_t MAX_THREADS;

    //! NOTE The default value for the maximum number of threads
    //! a single channel instance can communicate in.
    //! A different value can be specified for a specific channel,
    //! but no more than MAX_THREADS.
    static size_t MAX_THREADS_PER_CHANNEL;

    //! NOTE The queue capacity, if there are more unprocessed messages,
    //! they will not be lost, but will be sent to the next process
    static size_t QUEUE_CAPACITY;

    //! NOTE When closing an application, we need to terminate.
    //! During the shutdown, various objects are destroyed, from different threads,
    //! especially if they are static objects—they may access a destroyed or non-functioning queue.
    static std::atomic<bool> terminated;
};
}
