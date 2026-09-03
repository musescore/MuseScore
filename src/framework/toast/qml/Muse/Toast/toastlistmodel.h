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

#include <optional>

#include <QAbstractListModel>
#include <QVariant>
#include <QByteArray>
#include <QHash>

#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "itoastprovider.h"
#include "accessibility/iaccessibilitycontroller.h"

namespace muse::toast {
class ToastListModel : public QAbstractListModel, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    QML_ELEMENT

    muse::GlobalInject<muse::toast::IToastProvider> toastProvider;

    muse::ContextInject<muse::accessibility::IAccessibilityController> accessibilityController = { this };

public:
    explicit ToastListModel(QObject* parent = nullptr);
    ~ToastListModel() override = default;

    Q_INVOKABLE void init();
    Q_INVOKABLE void dismissToast(int id);
    Q_INVOKABLE void executeAction(int id, QString actionStr);
    Q_INVOKABLE void pauseToast(int id);
    Q_INVOKABLE void resumeToast(int id);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    enum RoleNames {
        IdRole = Qt::UserRole + 1,
        IconCodeRole,
        TitleRole,
        AccessibleTitleRole,
        MessageRole,
        ActionRole,
        DismissableRole,
        ProgressRole,
        TimeElapsedRole,
        ShowProgressInfoRole
    };

    void announceToast(const ToastItem& toast);
    std::optional<int> indexOfToast(int id) const;

    std::vector<std::shared_ptr<ToastItem> > m_toasts;
    int m_navigationHintShownCount = 0;
};
}
