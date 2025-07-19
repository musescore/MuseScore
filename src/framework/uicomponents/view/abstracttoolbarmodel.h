/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"
#include "shortcuts/ishortcutsregister.h"
#include "actions/iactionsdispatcher.h"

namespace muse::uicomponents {
class ToolBarItem;
class MenuItem;
using ToolBarItemList = QList<ToolBarItem*>;
class ToolBarItemType
{
    Q_GADGET
public:
    enum Type
    {
        ACTION,
        SEPARATOR,
        USER_TYPE
    };
    Q_ENUM(Type)
};

class AbstractToolBarModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int length READ rowCount NOTIFY itemsChanged)
    Q_PROPERTY(QVariantList items READ itemsProperty NOTIFY itemsChanged)

public:
    Inject<ui::IUiActionsRegister> uiActionsRegister = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:
    explicit AbstractToolBarModel(QObject* parent = nullptr);

    const actions::ActionCode SEPARATOR_ID = "";

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE virtual void load();

    QVariantList itemsProperty() const;
    const ToolBarItemList& items() const;

    Q_INVOKABLE QVariantMap get(int index);

signals:
    void itemsChanged();
    void itemChanged(uicomponents::ToolBarItem* item);

protected:
    enum Roles {
        ItemRole,

        UserRole
    };

    virtual void onActionsStateChanges(const actions::ActionCodeList& codes);

    void setItem(int index, ToolBarItem* item);
    void setItems(const ToolBarItemList& items);
    void clear();

    static const int INVALID_ITEM_INDEX;
    int itemIndex(const QString& itemId) const;

    ToolBarItem& item(int index);

    ToolBarItem& findItem(const actions::ActionCode& actionCode);
    ToolBarItem* findItemPtr(const actions::ActionCode& actionCode);
    ToolBarItem& findItem(const QString& itemId);
    ToolBarItem* findItemPtr(const QString& itemId);

    ToolBarItem* makeItem(const actions::ActionCode& actionCode, const TranslatableString& title = {});
    ToolBarItem* makeMenuItem(const TranslatableString& title, const actions::ActionCodeList& subitemsActionCodesLists,
                              const QString& menuId = "", bool enabled = true);
    ToolBarItem* makeSeparator();

    bool isIndexValid(int index) const;
    void dispatch(const actions::ActionCode& actionCode, const actions::ActionData& args = actions::ActionData());

private:
    ToolBarItem& item(const ToolBarItemList& items, const QString& itemId);
    ToolBarItem& item(const ToolBarItemList& items, const actions::ActionCode& actionCode);

    void updateShortcutsAll();
    void updateShortcuts(MenuItem* menuItem);

    ToolBarItemList m_items;
};
}
