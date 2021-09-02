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
#include "abstractmenumodel.h"

#include "log.h"

using namespace mu::ui;
using namespace mu::actions;

const int AbstractMenuModel::INVALID_ITEM_INDEX = -1;

AbstractMenuModel::AbstractMenuModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant AbstractMenuModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!isIndexValid(row)) {
        return QVariant();
    }

    const QVariantMap& item = m_items.at(row).toMap();

    switch (role) {
    case TitleRole: return item["title"];
    case CodeRole: return item["code"];
    case IdRole: return item["id"];
    case DescriptionRole: return item["description"];
    case ShortcutRole: return item["shortcut"];
    case IconRole: return item["icon"];
    case CheckedRole: return item["checked"];
    case EnabledRole: return item["enabled"];
    case SubitemsRole: return item["subitems"];
    case SectionRole: return item["section"];
    case UserRole: return QVariant();
    }

    return QVariant();
}

bool AbstractMenuModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_items.size();
}

int AbstractMenuModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> AbstractMenuModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TitleRole, "title" },
        { IdRole, "id" },
        { CodeRole, "code" },
        { DescriptionRole, "description" },
        { ShortcutRole, "shortcut" },
        { IconRole, "icon" },
        { CheckedRole, "checked" },
        { EnabledRole, "enabled" },
        { SubitemsRole, "subitems" },
        { SectionRole, "section" }
    };

    return roles;
}

void AbstractMenuModel::handleMenuItem(const QString& itemId)
{
    MenuItem menuItem = findItem(itemId);

    dispatch(menuItem.code, menuItem.args);
}

void AbstractMenuModel::dispatch(const ActionCode& actionCode, const ActionData& args)
{
    dispatcher()->dispatch(actionCode, args);
}

QVariantMap AbstractMenuModel::get(int index)
{
    QVariantMap result;

    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
}

void AbstractMenuModel::load()
{
    uiactionsRegister()->actionStateChanged().onReceive(this, [this](const ActionCodeList& codes) {
        onActionsStateChanges(codes);
    });
}

QVariantList AbstractMenuModel::itemsProperty() const
{
    QVariantList items;

    for (const MenuItem& item: m_items) {
        items << item.toMap();
    }

    return items;
}

const MenuItemList& AbstractMenuModel::items() const
{
    return m_items;
}

void AbstractMenuModel::setItems(const MenuItemList& items)
{
    TRACEFUNC;

    beginResetModel();
    m_items = items;
    endResetModel();

    emit itemsChanged();
}

void AbstractMenuModel::clear()
{
    setItems(MenuItemList());
}

int AbstractMenuModel::itemIndex(const QString& itemId) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == itemId) {
            return i;
        }
    }

    return INVALID_ITEM_INDEX;
}

MenuItem& AbstractMenuModel::item(int index)
{
    return m_items[index];
}

MenuItem& AbstractMenuModel::findItem(const QString& itemId)
{
    return item(m_items, itemId);
}

MenuItem& AbstractMenuModel::findItem(const ActionCode& actionCode)
{
    return item(m_items, actionCode);
}

MenuItem& AbstractMenuModel::findMenu(const QString& menuId)
{
    return menu(m_items, menuId);
}

MenuItem AbstractMenuModel::makeMenu(const QString& title, const MenuItemList& items, bool enabled,
                                     const QString& menuId) const
{
    MenuItem item;
    item.id = menuId;
    item.title = title;
    item.subitems = items;
    item.state.enabled = enabled;
    return item;
}

MenuItem AbstractMenuModel::makeMenuItem(const ActionCode& actionCode) const
{
    const UiAction& action = uiactionsRegister()->action(actionCode);
    if (!action.isValid()) {
        return MenuItem();
    }

    MenuItem item = action;
    item.state = uiactionsRegister()->actionState(actionCode);

    return item;
}

MenuItem AbstractMenuModel::makeSeparator() const
{
    MenuItem item;
    item.title = QString();
    return item;
}

void AbstractMenuModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    if (codes.empty()) {
        return;
    }

    for (const ActionCode& code : codes) {
        MenuItem& actionItem = findItem(code);
        if (actionItem.isValid()) {
            actionItem.state = uiactionsRegister()->actionState(code);
        }
    }
}

MenuItem& AbstractMenuModel::item(MenuItemList& items, const QString& itemId)
{
    for (MenuItem& menuItem : items) {
        if (menuItem.id == itemId) {
            return menuItem;
        }

        if (!menuItem.subitems.empty()) {
            MenuItem& subitem = item(menuItem.subitems, itemId);
            if (subitem.id == itemId) {
                return subitem;
            }
        }
    }

    static MenuItem null;
    return null;
}

MenuItem& AbstractMenuModel::item(MenuItemList& items, const ActionCode& actionCode)
{
    for (MenuItem& menuItem : items) {
        if (menuItem.code == actionCode) {
            return menuItem;
        }

        if (!menuItem.subitems.empty()) {
            MenuItem& subitem = item(menuItem.subitems, actionCode);
            if (subitem.code == actionCode) {
                return subitem;
            }
        }
    }

    static MenuItem null;
    return null;
}

MenuItem& AbstractMenuModel::menu(MenuItemList& items, const QString& menuId)
{
    for (MenuItem& item : items) {
        if (item.id == menuId) {
            return item;
        }

        MenuItem& menuItem = menu(item.subitems, menuId);
        if (menuItem.isValid()) {
            return menuItem;
        }
    }

    static MenuItem null;
    return null;
}
