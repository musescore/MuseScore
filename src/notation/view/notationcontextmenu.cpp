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
#include "notationcontextmenu.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::uicomponents;

MenuItemList NotationContextMenu::items(const ElementType& elementType) const
{
    switch (elementType) {
    case ElementType::MEASURE:
        return measureItems();
    case ElementType::PAGE:
        return pageItems();
    default:
        return elementItems();
    }
}

MenuItem NotationContextMenu::makeItem(const ActionItem& actionItem, const std::string& section, bool enabled, bool checked) const
{
    MenuItem item = actionItem;
    item.section = section;
    item.enabled = enabled;
    item.checked = checked;

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(actionItem.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItemList NotationContextMenu::measureItems() const
{
    MenuItemList items = defaultCopyPasteItems();
    items << makeItem(actionsRegister()->action("staff-properties"));

    return items;
}

MenuItemList NotationContextMenu::pageItems() const
{
    MenuItemList items {
        makeItem(actionsRegister()->action("edit-style")),
        makeItem(actionsRegister()->action("page-settings")),
        makeItem(actionsRegister()->action("load-style"))
    };

    return items;
}

MenuItemList NotationContextMenu::elementItems() const
{
    MenuItemList items = defaultCopyPasteItems();
    items << makeItem(actionsRegister()->action("select-similar"))
          << makeItem(actionsRegister()->action("select-similar-staff"))
          << makeItem(actionsRegister()->action("select-similar-range"))
          << makeItem(actionsRegister()->action("select-dialog"));

    return items;
}

MenuItemList NotationContextMenu::defaultCopyPasteItems() const
{
    MenuItemList items {
        makeItem(actionsRegister()->action("cut")),
        makeItem(actionsRegister()->action("copy")),
        makeItem(actionsRegister()->action("paste")),
        makeItem(actionsRegister()->action("swap")),
        makeItem(actionsRegister()->action("delete")),
    };

    return items;
}
