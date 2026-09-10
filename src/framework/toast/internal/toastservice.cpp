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
#include "ui/view/iconcodes.h"
#include "../toastitem.h"

#include "toastservice.h"

using namespace muse::ui;
using namespace muse::toast;

muse::async::Promise<ToastResult> ToastService::show(const std::string& title, const std::string& message,
                                                     muse::ui::IconCode::Code iconCode, bool dismissible,
                                                     const std::vector<ToastAction>& actions)
{
    return toastProvider()->show(ToastItem(title, message, iconCode, dismissible, std::chrono::seconds(0), actions));
}

muse::async::Promise<ToastResult> ToastService::showWithTimeout(const std::string& title, const std::string& message,
                                                                std::chrono::seconds timeout,
                                                                muse::ui::IconCode::Code iconCode, bool dismissible,
                                                                const std::vector<ToastAction>& actions)
{
    return toastProvider()->show(ToastItem(title, message, iconCode, dismissible, timeout, actions));
}

void ToastService::showSuccess(const std::string& title, const std::string& message)
{
    show(title, message, IconCode::Code::TICK, true, {});
}

void ToastService::showError(const std::string& title, const std::string& message)
{
    show(title, message, IconCode::Code::ERROR, true, {});
}

void ToastService::showInfo(const std::string& title, const std::string& message)
{
    show(title, message, IconCode::Code::INFO, true, {});
}

void ToastService::showWarning(const std::string& title, const std::string& message)
{
    show(title, message, IconCode::Code::WARNING, true, {});
}

muse::async::Promise<ToastResult> ToastService::showWithProgress(const std::string& title, const std::string& message,
                                                                 std::shared_ptr<muse::Progress> progress,
                                                                 muse::ui::IconCode::Code iconCode, bool dismissible,
                                                                 const std::vector<ToastAction>& actions, bool showProgressInfo)
{
    return toastProvider()->show(ToastItem(title, message, iconCode, dismissible, actions, progress, showProgressInfo));
}
