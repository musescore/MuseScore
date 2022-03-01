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

#include "translation.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::uicomponents;

void NotationContextMenuModel::loadItems(int elementType)
{
    MenuItemList items = makeItemsByElementType(static_cast<ElementType>(elementType));
    setItems(items);
}

MenuItemList NotationContextMenuModel::makeItemsByElementType(ElementType elementType)
{
    switch (elementType) {
    case ElementType::MEASURE:
        return makeMeasureItems();
    case ElementType::PAGE:
        return makePageItems();
    case ElementType::STAFF_TEXT:
        return makeStaffTextItems();
    case ElementType::SYSTEM_TEXT:
        return makeSystemTextItems();
    case ElementType::TIMESIG:
        return makeTimeSignatureItems();
    case ElementType::HARMONY:
        return makeHarmonyItems();
    default:
        break;
    }

    return makeElementItems();
}

MenuItemList NotationContextMenuModel::makePageItems()
{
    MenuItemList items {
        makeMenuItem("edit-style"),
        makeMenuItem("page-settings"),
        makeMenuItem("load-style"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeDefaultCopyPasteItems()
{
    MenuItemList items {
        makeMenuItem("notation-cut"),
        makeMenuItem("notation-copy"),
        makeMenuItem("notation-paste"),
        makeMenuItem("notation-swap"),
        makeMenuItem("notation-delete"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeMeasureItems()
{
    MenuItemList items = {
        makeMenuItem("notation-cut"),
        makeMenuItem("notation-copy"),
        makeMenuItem("notation-paste"),
        makeMenuItem("notation-swap"),
    };

    items << makeSeparator();

    MenuItem* clearItem = makeMenuItem("notation-delete");
    clearItem->setTitle(qtrc("notation", "Clear measures"));
    MenuItem* deleteItem = makeMenuItem("time-delete");
    deleteItem->setTitle(qtrc("notation", "Delete measures"));
    items << clearItem;
    items << deleteItem;

    items << makeSeparator();

    if (isDrumsetStaff()) {
        items << makeMenuItem("edit-drumset");
    }

    items << makeMenuItem("staff-properties");
    items << makeSeparator();
    items << makeMenu(qtrc("notation", "Insert measures"), makeInsertMeasuresItems());
    items << makeMenuItem("measure-properties");

    return items;
}

MenuItemList NotationContextMenuModel::makeStaffTextItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("staff-text-properties");

    return items;
}

MenuItemList NotationContextMenuModel::makeSystemTextItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("system-text-properties");

    return items;
}

MenuItemList NotationContextMenuModel::makeTimeSignatureItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("time-signature-properties");

    return items;
}

MenuItemList NotationContextMenuModel::makeHarmonyItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("realize-chord-symbols");

    return items;
}

MenuItemList NotationContextMenuModel::makeSelectItems()
{
    MenuItemList items {
        makeMenuItem("select-similar"),
        makeMenuItem("select-similar-staff"),
        makeMenuItem("select-similar-range"),
        makeMenuItem("select-dialog"),
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeElementItems()
{
    MenuItemList items = makeDefaultCopyPasteItems();

    if (isSingleSelection()) {
        items << makeMenu(qtrc("notation", "Select"), makeSelectItems());
    }

    return items;
}

MenuItemList NotationContextMenuModel::makeInsertMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measures-after-selection", qtrc("notation", "After selection…")),
        makeMenuItem("insert-measures", qtrc("notation", "Before selection…")),
        makeSeparator(),
        makeMenuItem("insert-measures-at-start-of-score", qtrc("notation", "At start of score…")),
        makeMenuItem("append-measures", qtrc("notation", "At end of score…"))
    };

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

bool NotationContextMenuModel::isDrumsetStaff() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    auto interaction = notation->interaction();
    if (!interaction) {
        return false;
    }

    auto hitElementContext = interaction->hitElementContext();
    if (!hitElementContext.staff) {
        return false;
    }

    auto tick = hitElementContext.element ? hitElementContext.element->tick() : Fraction { -1, 1 };
    return hitElementContext.staff->part()->instrument(tick)->drumset() != nullptr;
}
