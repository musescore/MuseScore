/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <thread>
#include <vector>

#include "global/concurrency/rpcqueue.h"

#include "../asyncable.h"

namespace muse::audio::comm {
struct CommMsg {
    Asyncable* receiver = nullptr;
    std::function<void()> func;
};

inline static const size_t MAX_THREADS = 100;

//! NOTE The queue capacity, if there are more unprocessed messages,
//! they will not be lost, but will be sent to the next process
inline static const size_t QUEUE_CAPACITY = 16;
using Queue = muse::RpcQueue<CommMsg>;
using Port = muse::RpcPort<CommMsg>;

class QueuePool
{
public:

    static QueuePool* instance();

    void regPort(const std::thread::id& th, const std::shared_ptr<Port>& port);
    void unregPort(const std::thread::id& th, const std::shared_ptr<Port>& port);

    void processMessages();
    void processMessages(const std::thread::id& th);

private:

    QueuePool();

    struct ThreadData {
        std::thread::id threadId;
        std::vector<std::shared_ptr<Port> > ports;
    };

    std::vector<ThreadData> m_threads;
};
}
