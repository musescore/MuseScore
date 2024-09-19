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
#include "../iextensionsprovider.h"
#include "ui/iuiactionsregister.h"
#include "actions/iactionsdispatcher.h"

#include "async/asyncable.h"

namespace muse::extensions {
class ExtensionsToolBarModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged FINAL)

    Inject<IExtensionsProvider> extensionsProvider = { this };
    Inject<ui::IUiActionsRegister> actionsRegister = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };

public:
    ExtensionsToolBarModel();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void onClicked(int idx);

    bool isEmpty() const;

signals:
    void isEmptyChanged();

private:

    void load();
    int indexByAction(const actions::ActionCode& code) const;

    enum Roles {
        IconRole = Qt::UserRole + 1,
        EnabledRole
    };

    struct Item {
        UiControl control;
        ui::UiAction action;
        ui::UiActionState state;
    };

    QList<Item> m_items;

    bool m_isEmpty = true;
};
}
