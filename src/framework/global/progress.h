/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_GLOBAL_PROGRESS_H
#define MUSE_GLOBAL_PROGRESS_H

#include <memory>
#include <atomic>

#include "async/channel.h"
#include "async/notification.h"

#include "types/retval.h"
#include "types/val.h"

namespace muse {
using ProgressResult = RetVal<Val>;

class Progress
{
    struct Data {
        async::Notification started;
        async::Channel<ProgressResult> finished;
        async::Notification canceled;
        std::atomic<bool> isStarted = false;
        std::atomic<bool> isCanceled = false;
        std::atomic<int64_t> prevCurrent = -1;
        std::atomic<int64_t> prevTotal = -1;
        std::atomic<int64_t> maxNumIncrements = 0;

        async::Channel<int64_t /*current*/, int64_t /*total*/, std::string /*title*/> progressChanged;
    };

public:

    Progress()
        : m_data(std::make_shared<Data>()) {}

    Progress(const Progress& p)
        : m_data(p.m_data) {}

    ~Progress() {}

    Progress& operator=(const Progress& p)
    {
        m_data = p.m_data;
        return *this;
    }

    // start
    void start()
    {
        m_data->isCanceled = false;
        m_data->isStarted = true;
        m_data->prevCurrent = -1;
        m_data->prevTotal = -1;
        m_data->started.notify();
    }

    async::Notification& started() { return m_data->started; }
    bool isStarted() const { return m_data->isStarted; }

    void setMaxNumIncrements(int64_t maxNumIncrements) { m_data->maxNumIncrements = maxNumIncrements; }

    // progress
    bool progress(int64_t current, int64_t total, const std::string& msg = {})
    {
        const int64_t maxNumIncrements = m_data->maxNumIncrements;
        if (maxNumIncrements > 0 && total > maxNumIncrements) {
            current *= static_cast<double>(maxNumIncrements) / total;
            total = maxNumIncrements;
        }

        if (current == m_data->prevCurrent && total == m_data->prevTotal) {
            return false;
        }

        m_data->prevCurrent = current;
        m_data->prevTotal = total;
        m_data->progressChanged.send(current, total, msg);

        return true;
    }

    async::Channel<int64_t /*current*/, int64_t /*total*/, std::string /*title*/>& progressChanged()
    {
        return m_data->progressChanged;
    }

    void finish(const ProgressResult& res)
    {
        m_data->prevCurrent = -1;
        m_data->prevTotal = -1;
        m_data->isStarted = false;
        m_data->finished.send(res);
    }

    async::Channel<ProgressResult>& finished() { return m_data->finished; }

    void cancel()
    {
        m_data->isCanceled = true;
        m_data->canceled.notify();
        finish(make_ret(Ret::Code::Cancel));
    }

    async::Notification& canceled() { return m_data->canceled; }
    bool isCanceled() const { return m_data->isCanceled; }

private:

    std::shared_ptr<Data> m_data = nullptr;
};

using ProgressPtr = std::shared_ptr<Progress>;
}

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(muse::Progress)
#endif

#endif // MUSE_GLOBAL_PROGRESS_H
