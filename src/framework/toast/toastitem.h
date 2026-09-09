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
#include <memory>
#include <string>
#include <vector>

#include "async/notification.h"
#include "ui/view/iconcodes.h"
#include "progress.h"

#include "toasttypes.h"

namespace muse::toast {
class ToastItem
{
public:
    ToastItem(const std::string& title, const std::string& message, muse::ui::IconCode::Code iconCode, bool dismissible,
              std::chrono::seconds timeout = std::chrono::seconds(0), std::vector<ToastAction> actions = {});

    ToastItem(const std::string& title, const std::string& message, muse::ui::IconCode::Code iconCode, bool dismissible,
              std::vector<ToastAction> actions, std::shared_ptr<muse::Progress> progress, bool showProgressInfo);
    ~ToastItem() = default;

    int id() const;
    std::string title() const;
    std::string message() const;
    muse::ui::IconCode::Code iconCode() const;
    bool isDismissible() const;
    const std::vector<ToastAction>& actions() const;
    std::chrono::seconds timeout() const;

    double currentProgress() const;
    void setCurrentProgress(double progress);
    muse::async::Notification progressChanged() const;
    std::shared_ptr<muse::Progress> progress() const;
    bool showProgressInfo() const;
    int timeElapsed() const;

private:
    int m_id = 0;
    std::string m_title = "Title";
    std::string m_message = "Message";
    muse::ui::IconCode::Code m_iconCode = muse::ui::IconCode::Code::NONE;
    bool m_dismissible = true;
    std::chrono::seconds m_timeout = std::chrono::seconds(0);
    std::vector<ToastAction> m_actions;

    double m_currentProgress = 0;
    muse::async::Notification m_progressChanged;
    std::shared_ptr<muse::Progress> m_progress;
    bool m_showProgressInfo = false;

    std::chrono::steady_clock::time_point m_creationTime = std::chrono::steady_clock::now();
};
}
