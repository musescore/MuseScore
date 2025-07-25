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

#include "async/asyncable.h"
#include "menuitem.h"

#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"
#include "shortcuts/ishortcutsregister.h"
#include "actions/iactionsdispatcher.h"

namespace muse::uicomponents {
class AbstractMenuModel : public QAbstractListModel, public muse::Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int length READ rowCount NOTIFY itemsChanged)
    Q_PROPERTY(QVariantList items READ itemsProperty NOTIFY itemsChanged)

public:
    muse::Inject<ui::IUiActionsRegister> uiActionsRegister = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:
    explicit AbstractMenuModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    virtual void load();

    QVariantList itemsProperty() const;
    const MenuItemList& items() const;

    Q_INVOKABLE virtual void handleMenuItem(const QString& itemId);
    Q_INVOKABLE QVariantMap get(int index);

signals:
    void itemsChanged();
    void itemChanged(muse::uicomponents::MenuItem* item);

protected:
    enum Roles {
        ItemRole,

        UserRole
    };

    virtual void onActionsStateChanges(const muse::actions::ActionCodeList& codes);

    void setItem(int index, MenuItem* item);
    void setItems(const MenuItemList& items);
    void clear();

    static const int INVALID_ITEM_INDEX;
    int itemIndex(const QString& itemId) const;

    MenuItem& item(int index);

    MenuItem& findItem(const QString& itemId);
    MenuItem& findItem(const muse::actions::ActionCode& actionCode);
    MenuItemList findItems(const muse::actions::ActionCode& actionCode);
    MenuItem& findMenu(const QString& menuId);

    MenuItem* makeMenu(const TranslatableString& title, const MenuItemList& items, const QString& menuId = "", bool enabled = true);

    MenuItem* makeMenuItem(const muse::actions::ActionCode& actionCode, const TranslatableString& title = {});
    MenuItem* makeSeparator();

    bool isIndexValid(int index) const;
    void dispatch(const muse::actions::ActionCode& actionCode, const muse::actions::ActionData& args = muse::actions::ActionData());
    void dispatch(const muse::actions::ActionQuery& actionQuery);

private:
    MenuItem& item(MenuItemList& items, const QString& itemId);
    MenuItemList items(MenuItemList& items, const muse::actions::ActionCode& actionCode);
    MenuItem& menu(MenuItemList& items, const QString& menuId);

    void updateState(MenuItemList& items, const muse::actions::ActionCodeList& codes,
                     std::map<muse::actions::ActionCode, muse::ui::UiActionState>& states);

    void updateShortcutsAll();
    void updateShortcuts(MenuItem* item);

    MenuItemList m_items;
};
}
