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
#include "toastitem.h"

using namespace muse::toast;

namespace {
int CURRENT_TOAST_ID = 1;
}

ToastItem::ToastItem(const std::string& title, const std::string& message, muse::ui::IconCode::Code iconCode, bool dismissible,
                     std::chrono::seconds timeout, std::vector<ToastAction> actions)
    : m_id(CURRENT_TOAST_ID++),
    m_title(title),
    m_message(message),
    m_iconCode(iconCode),
    m_dismissible(dismissible),
    m_timeout(timeout),
    m_actions(std::move(actions)),
    m_showProgressInfo(false)
{
}

ToastItem::ToastItem(const std::string& title, const std::string& message, muse::ui::IconCode::Code iconCode, bool dismissible,
                     std::vector<ToastAction> actions, std::shared_ptr<muse::Progress> progress, bool showProgressInfo)
    : m_id(CURRENT_TOAST_ID++),
    m_title(title),
    m_message(message),
    m_iconCode(iconCode),
    m_dismissible(dismissible),
    m_timeout(std::chrono::seconds(0)),
    m_actions(std::move(actions)),
    m_progress(std::move(progress)),
    m_showProgressInfo(showProgressInfo)
{
}

int ToastItem::id() const
{
    return m_id;
}

std::string ToastItem::title() const
{
    return m_title;
}

std::string ToastItem::message() const
{
    return m_message;
}

muse::ui::IconCode::Code ToastItem::iconCode() const
{
    return m_iconCode;
}

std::chrono::seconds ToastItem::timeout() const
{
    return m_timeout;
}

bool ToastItem::isDismissible() const
{
    return m_dismissible;
}

const std::vector<ToastAction>& ToastItem::actions() const
{
    return m_actions;
}

std::shared_ptr<muse::Progress> ToastItem::progress() const
{
    return m_progress;
}

double ToastItem::currentProgress() const
{
    return m_currentProgress;
}

void ToastItem::setCurrentProgress(double progress)
{
    m_currentProgress = progress;
    m_progressChanged.notify();
}

muse::async::Notification ToastItem::progressChanged() const
{
    return m_progressChanged;
}

bool ToastItem::showProgressInfo() const
{
    return m_showProgressInfo;
}

int ToastItem::timeElapsed() const
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_creationTime).count();
    return static_cast<int>(elapsed);
}
