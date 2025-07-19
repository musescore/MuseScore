/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"

namespace muse::diagnostics {
class ActionsViewModel : public QAbstractListModel
{
    Q_OBJECT

    Inject<actions::IActionsDispatcher> actionsDispatcher;
    Inject<ui::IUiActionsRegister> uiActionsRegister;

public:
    ActionsViewModel();

    Q_INVOKABLE void load();
    Q_INVOKABLE void find(const QString& str);
    Q_INVOKABLE void print();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:

    enum Roles {
        rItemData = Qt::UserRole + 1
    };

    struct Item {
        bool isReg = false;
        bool isHasUi = false;
        QString actionCode;
        QString actionTitle;

        QString formatted;
    };

    QList<Item> m_allItems;
    QList<Item> m_items;

    QString m_searchText;
};
}
