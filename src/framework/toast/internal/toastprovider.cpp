/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "toastprovider.h"

#include "async/async.h"

using namespace muse::toast;

ToastProvider::~ToastProvider()
{
    for (const auto& pair : m_progressTimers) {
        delete pair.second;
    }
}

muse::async::Promise<ToastActionCode> ToastProvider::show(ToastItem item)
{
    return muse::async::make_promise<ToastActionCode>([this, item](auto resolve, auto) {
        int id = item.id();
        m_toasts.emplace_back(std::make_shared<ToastItem>(item));
        m_toastAdded.send(m_toasts.back());

        m_resolvers[id] = std::move(resolve);

        checkProgress(id);
        checkTimer(id);

        return muse::async::Promise<ToastActionCode>::dummy_result();
    }, muse::async::PromiseType::AsyncByBody);
}

muse::async::Channel<std::shared_ptr<ToastItem> > ToastProvider::toastAdded() const
{
    return m_toastAdded;
}

muse::async::Channel<int> ToastProvider::toastDismissed() const
{
    return m_toastDismissed;
}

void ToastProvider::dismissToast(int id)
{
    for (int i = 0; i < static_cast<int>(m_toasts.size()); ++i) {
        if (m_toasts.at(i)->id() == id) {
            m_toastDismissed.send(id);
            const auto& item = m_toasts.at(i);
            cleanup(item->id());
            m_toasts.erase(m_toasts.begin() + i);
            return;
        }
    }
}

void ToastProvider::executeAction(int id, ToastActionCode actionCode)
{
    resolveToast(id, actionCode);
    dismissToast(id);
}

void ToastProvider::pauseToast(int id)
{
    auto timerIt = m_progressTimers.find(id);
    if (timerIt != m_progressTimers.end()) {
        timerIt->second->stop();
    }
}

void ToastProvider::resumeToast(int id)
{
    auto timerIt = m_progressTimers.find(id);
    if (timerIt != m_progressTimers.end() && !timerIt->second->isActive()) {
        timerIt->second->start();
    }
}

void ToastProvider::cleanup(int id)
{
    auto timerIt = m_progressTimers.find(id);
    if (timerIt != m_progressTimers.end()) {
        // cleanup() may run from within the timer's own timeout callback (progress reaching 100%),
        // so defer the actual destruction instead of freeing it while its signal is still on the stack
        QTimer* timer = timerIt->second;
        timer->stop();
        timer->deleteLater();
        m_progressTimers.erase(timerIt);
    }

    const auto itemIt = std::find_if(m_toasts.begin(), m_toasts.end(), [id](const std::shared_ptr<ToastItem>& toast) {
        return toast->id() == id;
    });
    if (itemIt != m_toasts.end()) {
        if (auto progress = (*itemIt)->progress()) {
            progress->progressChanged().disconnect(this);
            progress->finished().disconnect(this);
        }
    }

    auto resolverIt = m_resolvers.find(id);
    if (resolverIt != m_resolvers.end()) {
        resolveToast(id, ToastActionCode::None);
    }
}

void ToastProvider::checkProgress(int id)
{
    const auto itemIt = std::find_if(m_toasts.begin(), m_toasts.end(), [id](const std::shared_ptr<ToastItem>& toast) {
        return toast->id() == id;
    });

    if (itemIt == m_toasts.end()) {
        return;
    }

    auto& item = *itemIt;
    auto progress = item->progress();
    if (progress) {
        progress->progressChanged().onReceive(this, [this, item](int64_t current, int64_t total, const std::string&) {
            if (total > 0) {
                const double newProgress = static_cast<int>((static_cast<double>(current) / static_cast<double>(total)) * 100.0);
                item->setCurrentProgress(newProgress);

                if (newProgress >= 100.0) {
                    async::Async::call(this, [this, item]() { dismissToast(item->id()); });
                }
            }
        });

        progress->finished().onReceive(this, [this, item](const muse::ProgressResult&) {
            async::Async::call(this, [this, item]() { dismissToast(item->id()); });
        });
    }
}

void ToastProvider::checkTimer(int id)
{
    const auto itemIt = std::find_if(m_toasts.begin(), m_toasts.end(), [id](const std::shared_ptr<ToastItem>& toast) {
        return toast->id() == id;
    });

    if (itemIt == m_toasts.end()) {
        return;
    }

    auto& item = *itemIt;
    if (item->timeout().count() > 0) {
        const int timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(item->timeout()).count();

        if (timeoutMs > 0) {
            int interval = static_cast<int>(timeoutMs / 100);
            QTimer* timer = new QTimer();
            timer->setInterval(interval);
            timer->setSingleShot(false);
            timer->start();
            timer->callOnTimeout([this, item, timeoutMs, interval]() {
                double currentProgress = item->currentProgress() + ((static_cast<double>(interval) / static_cast<double>(timeoutMs)) * 100.0);
                item->setCurrentProgress(currentProgress);
                if (currentProgress >= 100.0) {
                    dismissToast(item->id());
                }
            });

            m_progressTimers[item->id()] = timer;
        }
    }
}

void ToastProvider::resolveToast(int id, ToastActionCode actionCode)
{
    auto promiseIt = m_resolvers.find(id);
    if (promiseIt != m_resolvers.end()) {
        auto resolve = std::move(promiseIt->second);
        m_resolvers.erase(promiseIt);
        (void)resolve(actionCode);
    }
}
