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
#include "queuepool.h"

namespace muse::audio::comm {
static const std::thread::id EMPTY_THREAD_ID;

QueuePool* QueuePool::instance()
{
    static QueuePool p;
    return &p;
}

QueuePool::QueuePool()
{
    m_threads.resize(MAX_THREADS);
}

void QueuePool::regPort(const std::thread::id& th, const std::shared_ptr<Port>& port)
{
    for (size_t i = 0; i < m_threads.size(); ++i) {
        ThreadData& thdata = m_threads.at(i);
        if (thdata.threadId == th) {
            thdata.ports.push_back(port);
            break;
        } else if (thdata.threadId == EMPTY_THREAD_ID) {
            thdata.threadId = th;
            thdata.ports.push_back(port);
            break;
        }
    }
}

void QueuePool::unregPort(const std::thread::id& th, const std::shared_ptr<Port>& port)
{
    for (size_t i = 0; i < m_threads.size(); ++i) {
        ThreadData& thdata = m_threads.at(i);
        if (thdata.threadId == th) {
            thdata.ports.erase(std::remove(thdata.ports.begin(), thdata.ports.end(), port), thdata.ports.end());

            if (thdata.ports.empty()) {
                thdata.threadId = EMPTY_THREAD_ID;
            }
            break;
        } else if (thdata.threadId == EMPTY_THREAD_ID) {
            break;
        }
    }
}

void QueuePool::processMessages()
{
    std::thread::id threadId = std::this_thread::get_id();
    processMessages(threadId);
}

void QueuePool::processMessages(const std::thread::id& th)
{
    for (size_t i = 0; i < m_threads.size(); ++i) {
        ThreadData& thdata = m_threads.at(i);
        if (thdata.threadId == th) {
            for (auto& p : thdata.ports) {
                p->process();
            }
            break;
        } else if (thdata.threadId == EMPTY_THREAD_ID) {
            break;
        }
    }
}
} // muse::audio::comm
