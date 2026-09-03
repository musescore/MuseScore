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
#pragma once

#include <memory>
#include <vector>
#include <map>

#include <QTimer>

#include "async/channel.h"
#include "async/asyncable.h"
#include "progress.h"
#include "async/promise.h"

#include "../itoastprovider.h"
#include "../toasttypes.h"

namespace muse::toast {
class ToastProvider : public IToastProvider, public muse::async::Asyncable
{
public:
    ~ToastProvider() override;

    muse::async::Promise<ToastActionCode> show(ToastItem item) override;

    muse::async::Channel <std::shared_ptr<ToastItem> > toastAdded() const override;
    muse::async::Channel<int> toastDismissed() const override;

    void dismissToast(int id) override;
    void executeAction(int id, ToastActionCode actionCode) override;

    void pauseToast(int id) override;
    void resumeToast(int id) override;

private:
    void cleanup(int id);
    void checkProgress(int id);
    void checkTimer(int id);
    void resolveToast(int id, ToastActionCode actionCode);

    std::vector<std::shared_ptr<ToastItem> > m_toasts;

    muse::async::Channel<std::shared_ptr<ToastItem> > m_toastAdded;
    muse::async::Channel<int> m_toastDismissed;

    std::map<int, QTimer*> m_progressTimers;
    std::map<int, muse::async::Promise<ToastActionCode>::Resolve> m_resolvers;
};
}
