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

#include "modularity/ioc.h"
#include "progress.h"
#include "../itoastprovider.h"

#include "../itoastservice.h"

namespace muse::toast {
class ToastService : public IToastService
{
    muse::GlobalInject<IToastProvider> toastProvider;

public:
    muse::async::Promise<ToastActionCode> show(const std::string& title, const std::string& message, muse::ui::IconCode::Code iconCode,
                                               bool dismissible, const std::vector<ToastAction>& actions) override;
    muse::async::Promise<ToastActionCode> showWithTimeout(const std::string& title, const std::string& message,
                                                          std::chrono::seconds timeout, muse::ui::IconCode::Code iconCode, bool dismissible,
                                                          const std::vector<ToastAction>& actions) override;
    void showSuccess(const std::string& title, const std::string& message) override;
    void showError(const std::string& title, const std::string& message) override;
    void showInfo(const std::string& title, const std::string& message) override;
    void showWarning(const std::string& title, const std::string& message) override;
    muse::async::Promise<ToastActionCode> showWithProgress(const std::string& title, const std::string& message,
                                                           std::shared_ptr<muse::Progress> progress, muse::ui::IconCode::Code iconCode,
                                                           bool dismissible, const std::vector<ToastAction>& actions,
                                                           bool showProgressInfo) override;
};
}
