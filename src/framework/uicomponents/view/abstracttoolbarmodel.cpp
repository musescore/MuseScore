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
#include "abstracttoolbarmodel.h"

#include "types/translatablestring.h"

#include "toolbaritem.h"
#include "menuitem.h"

#include "log.h"

using namespace muse::uicomponents;
using namespace muse::ui;
using namespace muse::actions;

const int AbstractToolBarModel::INVALID_ITEM_INDEX = -1;

AbstractToolBarModel::AbstractToolBarModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant AbstractToolBarModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!isIndexValid(row)) {
        return QVariant();
    }

    ToolBarItem* item = m_items.at(row);

    switch (role) {
    case ItemRole: return QVariant::fromValue(item);
    case UserRole: return QVariant();
    }

    return QVariant();
}

bool AbstractToolBarModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_items.size();
}

int AbstractToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> AbstractToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { ItemRole, "itemRole" }
    };

    return roles;
}

void AbstractToolBarModel::dispatch(const ActionCode& actionCode, const ActionData& args)
{
    dispatcher()->dispatch(actionCode, args);
}

QVariantMap AbstractToolBarModel::get(int index)
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

void AbstractToolBarModel::load()
{
    uiActionsRegister()->actionStateChanged().onReceive(this, [this](const ActionCodeList& codes) {
        onActionsStateChanges(codes);
    });

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        updateShortcutsAll();
    });
}

QVariantList AbstractToolBarModel::itemsProperty() const
{
    QVariantList items;

    for (ToolBarItem* item: m_items) {
        items << QVariant::fromValue(item);
    }

    return items;
}

const ToolBarItemList& AbstractToolBarModel::items() const
{
    return m_items;
}

void AbstractToolBarModel::setItems(const ToolBarItemList& items)
{
    TRACEFUNC;

    beginResetModel();

    qDeleteAll(m_items);
    m_items.clear();

    //! NOTE: make sure that we don't have two separators sequentially
    bool isPreviousSeparator = false;

    for (ToolBarItem* item : items) {
        if (item->type() == ToolBarItemType::SEPARATOR) {
            if (isPreviousSeparator) {
                delete item;
                continue;
            }

            isPreviousSeparator = true;
        } else {
            isPreviousSeparator = false;
        }

        m_items << item;
    }

    endResetModel();

    emit itemsChanged();
}

void AbstractToolBarModel::clear()
{
    setItems(ToolBarItemList());
}

int AbstractToolBarModel::itemIndex(const QString& itemId) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i]->id() == itemId) {
            return i;
        }
    }

    return INVALID_ITEM_INDEX;
}

ToolBarItem& AbstractToolBarModel::item(int index)
{
    ToolBarItem& item = *m_items[index];
    if (item.isValid()) {
        return item;
    }

    static ToolBarItem dummy;
    return dummy;
}

ToolBarItem& AbstractToolBarModel::findItem(const ActionCode& actionCode)
{
    return item(m_items, actionCode);
}

ToolBarItem* AbstractToolBarModel::findItemPtr(const actions::ActionCode& actionCode)
{
    for (ToolBarItem* toolBarItem : m_items) {
        if (toolBarItem->action().code == actionCode) {
            return toolBarItem;
        }
    }

    return nullptr;
}

ToolBarItem& AbstractToolBarModel::findItem(const QString& itemId)
{
    return item(m_items, itemId);
}

ToolBarItem* AbstractToolBarModel::findItemPtr(const QString& itemId)
{
    for (ToolBarItem* toolBarItem : m_items) {
        if (toolBarItem->id() == itemId) {
            return toolBarItem;
        }
    }

    return nullptr;
}

ToolBarItem* AbstractToolBarModel::makeItem(const ActionCode& actionCode, const TranslatableString& title)
{
    const UiAction& action = uiActionsRegister()->action(actionCode);
    if (!action.isValid()) {
        LOGW() << "not found action: " << actionCode;
        return nullptr;
    }

    ToolBarItem* item = new ToolBarItem(action, ToolBarItemType::ACTION, this);
    item->setState(uiActionsRegister()->actionState(actionCode));

    if (!title.isEmpty()) {
        item->setTitle(title);
    }

    return item;
}

ToolBarItem* AbstractToolBarModel::makeMenuItem(const TranslatableString& title, const ActionCodeList& subitemsActionCodesList,
                                                const QString& menuId, bool enabled)
{
    ToolBarItem* item = new ToolBarItem(this);
    item->setId(menuId);

    MenuItemList subitems;
    for (const ActionCode& subitemActionCode: subitemsActionCodesList) {
        const UiAction& action = uiActionsRegister()->action(subitemActionCode);
        if (!action.isValid()) {
            LOGW() << "not found action: " << subitemActionCode;
            continue;
        }

        MenuItem* subitem = new MenuItem(action, this);
        subitem->setState(uiActionsRegister()->actionState(subitemActionCode));

        subitems << subitem;
    }
    item->setMenuItems(subitems);

    UiAction action;
    action.title = title;
    item->setAction(action);

    UiActionState state;
    state.enabled = enabled;
    item->setState(state);

    return item;
}

ToolBarItem* AbstractToolBarModel::makeSeparator()
{
    UiAction action;
    action.title = {};

    return new ToolBarItem(action, ToolBarItemType::SEPARATOR, this);
}

void AbstractToolBarModel::onActionsStateChanges(const muse::actions::ActionCodeList& codes)
{
    if (codes.empty()) {
        return;
    }

    for (const ActionCode& code : codes) {
        ToolBarItem& actionItem = findItem(code);
        if (actionItem.isValid()) {
            actionItem.setState(uiActionsRegister()->actionState(code));
        }
    }
}

void AbstractToolBarModel::setItem(int index, ToolBarItem* item)
{
    if (!isIndexValid(index)) {
        return;
    }

    m_items[index] = item;

    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

ToolBarItem& AbstractToolBarModel::item(const ToolBarItemList& items, const QString& itemId)
{
    for (ToolBarItem* toolBarItem : items) {
        if (toolBarItem->id() == itemId) {
            return *toolBarItem;
        }
    }

    static ToolBarItem dummy;
    return dummy;
}

ToolBarItem& AbstractToolBarModel::item(const ToolBarItemList& items, const ActionCode& actionCode)
{
    for (ToolBarItem* toolBarItem : items) {
        if (!toolBarItem) {
            continue;
        }

        if (toolBarItem->action().code == actionCode) {
            return *toolBarItem;
        }
    }

    static ToolBarItem dummy;
    return dummy;
}

void AbstractToolBarModel::updateShortcutsAll()
{
    for (ToolBarItem* toolBarItem : m_items) {
        if (!toolBarItem) {
            continue;
        }

        UiAction action = toolBarItem->action();
        action.shortcuts = shortcutsRegister()->shortcut(action.code).sequences;
        toolBarItem->setAction(action);

        for (MenuItem* menuItem : toolBarItem->menuItems()) {
            if (!menuItem) {
                continue;
            }

            updateShortcuts(menuItem);
        }
    }
}

void AbstractToolBarModel::updateShortcuts(MenuItem* menuItem)
{
    UiAction action = menuItem->action();
    action.shortcuts = shortcutsRegister()->shortcut(action.code).sequences;
    menuItem->setAction(action);

    for (MenuItem* subItem : menuItem->subitems()) {
        if (!subItem) {
            continue;
        }

        updateShortcuts(subItem);
    }
}
