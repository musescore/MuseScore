/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "notationcontextmenu.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::ui;

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

MenuItem NotationContextMenu::makeItem(const UiAction& action, const QString& section, bool enabled, bool checked) const
{
    MenuItem item = action;
    item.section = section;
    item.state.enabled = enabled;
    item.state.checked = checked;
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
