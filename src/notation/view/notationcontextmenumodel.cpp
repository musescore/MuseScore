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
#include "notationcontextmenumodel.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::ui;

void NotationContextMenuModel::loadItems(int elementType)
{
    MenuItemList items = itemsByElementType(static_cast<ElementType>(elementType));
    setItems(items);
}

MenuItemList NotationContextMenuModel::itemsByElementType(ElementType elementType) const
{
    switch (elementType) {
    case ElementType::MEASURE:
        return measureItems();
    case ElementType::PAGE:
        return pageItems();
    case ElementType::STAFF_TEXT:
        return staffTextItems();
    case ElementType::SYSTEM_TEXT:
        return systemTextItems();
    case ElementType::TIMESIG:
        return timeSignatureItems();
    default:
        break;
    }

    return elementItems();
}

MenuItemList NotationContextMenuModel::pageItems() const
{
    MenuItemList items {
        makeMenuItem("edit-style"),
        makeMenuItem("page-settings"),
        makeMenuItem("load-style"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::defaultCopyPasteItems() const
{
    MenuItemList items {
        makeMenuItem("cut"),
        makeMenuItem("copy"),
        makeMenuItem("paste"),
        makeMenuItem("swap"),
        makeMenuItem("delete"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::measureItems() const
{
    MenuItemList items = elementItems();
    items << makeSeparator();
    items << makeMenuItem("edit-drumset");
    items << makeMenuItem("staff-properties");
    items << makeSeparator();
    items << makeMenuItem("measure-properties");

    return items;
}

MenuItemList NotationContextMenuModel::staffTextItems() const
{
    MenuItemList items = elementItems();
    items << makeSeparator();
    items << makeMenuItem("staff-text-properties");

    return items;
}

MenuItemList NotationContextMenuModel::systemTextItems() const
{
    MenuItemList items = elementItems();
    items << makeSeparator();
    items << makeMenuItem("system-text-properties");

    return items;
}

MenuItemList NotationContextMenuModel::timeSignatureItems() const
{
    MenuItemList items = elementItems();
    items << makeSeparator();
    items << makeMenuItem("time-signature-properties");

    return items;
}

MenuItemList NotationContextMenuModel::selectItems() const
{
    MenuItemList items {
        makeMenuItem("select-similar"),
        makeMenuItem("select-similar-staff"),
        makeMenuItem("select-similar-range"),
        makeMenuItem("select-dialog"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::elementItems() const
{
    MenuItemList items = defaultCopyPasteItems();

    if (isSingleSelection()) {
        items << makeMenu(qtrc("notation", "Select"), selectItems());
    }

    return items;
}

bool NotationContextMenuModel::isSingleSelection() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    auto interaction = notation->interaction();
    if (!interaction) {
        return false;
    }

    auto selection = interaction->selection();
    return selection ? selection->element() != nullptr : false;
}
