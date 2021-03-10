//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abstractmenumodel.h"

using namespace mu::uicomponents;
using namespace mu::actions;

AbstractMenuModel::AbstractMenuModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList AbstractMenuModel::items() const
{
    QVariantList menuItems;

    for (const MenuItem& menuItem: m_items) {
        menuItems << menuItem.toMap();
    }

    return menuItems;
}

bool AbstractMenuModel::actionEnabled(const actions::ActionCode&) const
{
    return true;
}

void AbstractMenuModel::clear()
{
    m_items.clear();
}

void AbstractMenuModel::appendItem(const MenuItem& item)
{
    m_items << item;
}

MenuItem& AbstractMenuModel::findItem(const ActionCode& actionCode)
{
    return item(m_items, actionCode);
}

MenuItem& AbstractMenuModel::findItemByIndex(const ActionCode& menuActionCode, int actionIndex)
{
    MenuItem& menuItem = menu(m_items, menuActionCode);
    MenuItemList& subitems = menuItem.subitems;
    for (int i = 0; i < subitems.size(); ++i) {
        if (i == actionIndex) {
            return subitems[i];
        }
    }

    static MenuItem null;
    return null;
}

MenuItem& AbstractMenuModel::findMenu(const ActionCode& subitemsActionCode)
{
    return menu(m_items, subitemsActionCode);
}

MenuItem AbstractMenuModel::makeMenu(const std::string& title, const MenuItemList& actions, bool enabled,
                                     const ActionCode& menuActionCode) const
{
    MenuItem item;
    item.code = menuActionCode;
    item.title = title;
    item.subitems = actions;
    item.enabled = enabled;
    return item;
}

MenuItem AbstractMenuModel::makeAction(const ActionCode& actionCode) const
{
    ActionItem action = actionsRegister()->action(actionCode);
    if (!action.isValid()) {
        return MenuItem();
    }

    MenuItem item = action;
    item.enabled = actionEnabled(actionCode);

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItem AbstractMenuModel::makeAction(const ActionCode& actionCode, bool checked) const
{
    MenuItem item = makeAction(actionCode);
    if (!item.isValid()) {
        return item;
    }

    item.checkable = true;
    item.checked = checked;

    return item;
}

MenuItem AbstractMenuModel::makeSeparator() const
{
    MenuItem item;
    item.title = std::string();
    return item;
}

void AbstractMenuModel::updateItemsEnabled(const std::vector<ActionCode>& actionCodes)
{
    if (actionCodes.empty()) {
        return;
    }

    for (const ActionCode& actionCode: actionCodes) {
        MenuItem& actionItem = findItem(actionCode);
        if (actionItem.isValid()) {
            actionItem.enabled = actionEnabled(actionCode);
        }
    }

    emit itemsChanged();
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

MenuItem& AbstractMenuModel::menu(MenuItemList& items, const ActionCode& subitemsActionCode)
{
    for (MenuItem& item : items) {
        if (item.subitems.isEmpty()) {
            continue;
        }

        if (item.code == subitemsActionCode) {
            return item;
        }

        MenuItem& menuItem = menu(item.subitems, subitemsActionCode);
        if (menuItem.isValid()) {
            return menuItem;
        }
    }

    static MenuItem null;
    return null;
}
