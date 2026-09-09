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

#include <QObject>

#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

#include "itoastservice.h"

namespace muse::toast {
class ToastTestsModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    QML_ELEMENT

    muse::GlobalInject<IToastService> toastService;

    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit ToastTestsModel(QObject* parent = nullptr);

    Q_INVOKABLE void showSuccess(const QString& title, const QString& message);
    Q_INVOKABLE void showError(const QString& title, const QString& message);
    Q_INVOKABLE void showInfo(const QString& title, const QString& message);
    Q_INVOKABLE void showWarning(const QString& title, const QString& message);

    Q_INVOKABLE void showToastWithAction(const QString& title, const QString& message, int iconCode);
    Q_INVOKABLE void showToastWithTimeout(const QString& title, const QString& message, int iconCode, int timeout, bool dismissible);
    Q_INVOKABLE void showWithProgress(const QString& title, const QString& message, int iconCode, bool dismissible, bool showProgressInfo);
    Q_INVOKABLE void updateProgress(int progressValue);

private:
    std::shared_ptr<muse::Progress> m_progress;
};
}
