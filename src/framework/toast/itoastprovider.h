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

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "async/promise.h"

#include "toastitem.h"
#include "toasttypes.h"

namespace muse::toast {
class IToastProvider : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IToastProvider)
public:
    virtual ~IToastProvider() = default;

    virtual muse::async::Promise<ToastResult> show(ToastItem item) = 0;

    virtual muse::async::Channel<std::shared_ptr<ToastItem> > toastAdded() const = 0;
    virtual muse::async::Channel<int> toastDismissed() const = 0;

    virtual void dismissToast(int id) = 0;
    virtual void executeAction(int id, int actionCode) = 0;

    virtual void pauseToast(int id) = 0;
    virtual void resumeToast(int id) = 0;
};
}
