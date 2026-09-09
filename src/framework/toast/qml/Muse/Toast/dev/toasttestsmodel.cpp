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
#include "toasttestsmodel.h"

#include "log.h"
#include "toasttypes.h"

using namespace muse::toast;
using namespace muse;

ToastTestsModel::ToastTestsModel(QObject* parent)
    : QObject(parent), Contextable(muse::iocCtxForQmlObject(this))
{
}

void ToastTestsModel::showToastWithTimeout(const QString& title, const QString& message, int iconCode, int timeoutSeconds, bool dismissible)
{
    toastService()->showWithTimeout(
        title.toStdString(),
        message.toStdString(),
        std::chrono::seconds(timeoutSeconds),
        static_cast<muse::ui::IconCode::Code>(iconCode),
        dismissible
        );
}

void ToastTestsModel::showSuccess(const QString& title, const QString& message)
{
    toastService()->showSuccess(title.toStdString(), message.toStdString());
}

void ToastTestsModel::showError(const QString& title, const QString& message)
{
    toastService()->showError(title.toStdString(), message.toStdString());
}

void ToastTestsModel::showInfo(const QString& title, const QString& message)
{
    toastService()->showInfo(title.toStdString(), message.toStdString());
}

void ToastTestsModel::showWarning(const QString& title, const QString& message)
{
    toastService()->showWarning(title.toStdString(), message.toStdString());
}

void ToastTestsModel::showToastWithAction(const QString& title, const QString& message, int iconCode)
{
    toastService()->show(
        title.toStdString(),
        message.toStdString(),
        static_cast<muse::ui::IconCode::Code>(iconCode),
        true,
    {
        { "Open info dialog", ToastActionCode::Custom },
    }).onResolve(this, [this](const ToastResult& result) {
        if (result.isCode(ToastActionCode::Custom)) {
            interactive()->info("Info Dialog", "Info dialog action triggered from toast");
        }
    });
}

void ToastTestsModel::showWithProgress(const QString& title, const QString& message, int iconCode, bool dismissible, bool showProgressInfo)
{
    if (m_progress) {
        //Just to make things easy on testing
        m_progress->progress(100, 100, "Completed old process");
    }

    m_progress = std::make_shared<muse::Progress>();

    toastService()->showWithProgress(
        title.toStdString(),
        message.toStdString(),
        m_progress,
        static_cast<muse::ui::IconCode::Code>(iconCode),
        dismissible,
        {},
        showProgressInfo
        );

    m_progress->start();
}

void ToastTestsModel::updateProgress(int progress)
{
    if (m_progress) {
        m_progress->progress(progress, 100, "Processing...");
    }
}
