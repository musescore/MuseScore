//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "menumodel.h"

#include "translation.h"

using namespace mu::appshell;
using namespace mu::uicomponents;

MenuModel::MenuModel(QObject* parent)
    : QObject(parent)
{
}

void MenuModel::load()
{
    m_items << fileItem();

    emit itemsChanged();
}

void MenuModel::handleAction(const QString& actionCode)
{
    actionsDispatcher()->dispatch(actionCode.toStdString());
}

QVariantList MenuModel::items()
{
    QVariantList menuItems;

    for (const MenuItem& menuItem: m_items) {
        menuItems << menuItem.toVariantMap();
    }

    return menuItems;
}

MenuItem MenuModel::fileItem()
{
    MenuItemList fileItems {
        makeAction(actionsRegister()->action("file-new")),
        makeAction(actionsRegister()->action("file-open")),
        makeAction(actionsRegister()->action("file-import")),
        makeSeparator(),
        makeAction(actionsRegister()->action("file-save")),
        makeAction(actionsRegister()->action("file-save-as"))
    };

    return makeMenu(trc("appshell", "&File"), fileItems);
}

MenuItem MenuModel::makeMenu(const std::string& title, const MenuItemList& actions)
{
    MenuItem item;
    item.title = title;
    item.subitems = actions;
    return item;
}

MenuItem MenuModel::makeAction(const mu::actions::ActionItem& action, const std::string& section, bool enabled, bool checked) const
{
    MenuItem item = action;
    item.section = section;
    item.enabled = enabled;
    item.checked = checked;

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItem MenuModel::makeSeparator()
{
    MenuItem item;
    item.title = std::string();
    return item;
}
