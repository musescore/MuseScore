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

#include <chrono>
#include <string>
#include <vector>

#include "modularity/imoduleinterface.h"
#include "ui/view/iconcodes.h"
#include "progress.h"
#include "async/promise.h"

#include "toasttypes.h"

namespace muse::toast {
class IToastService : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IToast)
public:
    virtual ~IToastService() = default;
    virtual muse::async::Promise<ToastResult> show(const std::string& title, const std::string& message,
                                                   muse::ui::IconCode::Code iconCode = muse::ui::IconCode::Code::NONE,
                                                   bool dismissible = true, const std::vector<ToastAction>& actions = {}) = 0;
    virtual muse::async::Promise<ToastResult> showWithTimeout(const std::string& title, const std::string& message,
                                                              std::chrono::seconds timeout,
                                                              muse::ui::IconCode::Code iconCode = muse::ui::IconCode::Code::NONE,
                                                              bool dismissible = true, const std::vector<ToastAction>& actions = {}) = 0;
    virtual void showSuccess(const std::string& title, const std::string& message) = 0;
    virtual void showError(const std::string& title, const std::string& message) = 0;
    virtual void showInfo(const std::string& title, const std::string& message) = 0;
    virtual void showWarning(const std::string& title, const std::string& message) = 0;
    virtual muse::async::Promise<ToastResult> showWithProgress(const std::string& title, const std::string& message,
                                                               std::shared_ptr<muse::Progress> progress,
                                                               muse::ui::IconCode::Code iconCode = muse::ui::IconCode::Code::NONE,
                                                               bool dismissible = false, const std::vector<ToastAction>& actions = {},
                                                               bool showProgressInfo = false) = 0;
};
}
