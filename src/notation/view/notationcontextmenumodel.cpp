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

#include "types/translatablestring.h"

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
    case ElementType::TRIPLET_FEEL:
        return makeSystemTextItems();
    case ElementType::TIMESIG:
        return makeTimeSignatureItems();
    case ElementType::INSTRUMENT_NAME:
        return makeInstrumentNameItems();
    case ElementType::HARMONY:
        return makeHarmonyItems();
    case ElementType::INSTRUMENT_CHANGE:
        return makeChangeInstrumentItems();
    case ElementType::VBOX:
        return makeVerticalBoxItems();
    case ElementType::HBOX:
        return makeHorizontalBoxItems();
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
    clearItem->setTitle(TranslatableString("notation", "Clear measures"));
    MenuItem* deleteItem = makeMenuItem("time-delete");
    deleteItem->setTitle(TranslatableString("notation", "Delete measures"));
    items << clearItem;
    items << deleteItem;

    items << makeSeparator();

    if (isDrumsetStaff()) {
        items << makeMenuItem("edit-drumset");
    }

    items << makeMenuItem("staff-properties");
    items << makeSeparator();
    items << makeMenu(TranslatableString("notation", "Insert measures"), makeInsertMeasuresItems());
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

MenuItemList NotationContextMenuModel::makeInstrumentNameItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("staff-properties");

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
    if (isSingleSelection()) {
        return MenuItemList { makeMenuItem("select-similar"), makeMenuItem("select-similar-staff"), makeMenuItem("select-dialog") };
    } else if (canSelectSimilarInRange()) {
        return MenuItemList { makeMenuItem("select-similar-range"), makeMenuItem("select-dialog") };
    } else if (canSelectSimilar()) {
        return MenuItemList{ makeMenuItem("select-dialog") };
    }

    return MenuItemList();
}

MenuItemList NotationContextMenuModel::makeElementItems()
{
    MenuItemList items = makeDefaultCopyPasteItems();

    if (interaction()->isTextEditingStarted()) {
        return items;
    }

    MenuItemList selectItems = makeSelectItems();

    if (!selectItems.isEmpty()) {
        items << makeMenu(TranslatableString("notation", "Select"), selectItems);
    }

    const EngravingItem* hitElement = hitElementContext().element;

    if (hitElement && hitElement->isEditable()) {
        items << makeSeparator();
        items << makeMenuItem("edit-element");
    }

    return items;
}

MenuItemList NotationContextMenuModel::makeInsertMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measures-after-selection", TranslatableString("notation", "After selection…")),
        makeMenuItem("insert-measures", TranslatableString("notation", "Before selection…")),
        makeSeparator(),
        makeMenuItem("insert-measures-at-start-of-score", TranslatableString("notation", "At start of score…")),
        makeMenuItem("append-measures", TranslatableString("notation", "At end of score…"))
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeChangeInstrumentItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem("change-instrument");

    return items;
}

MenuItemList NotationContextMenuModel::makeVerticalBoxItems()
{
    MenuItemList addMenuItems;
    addMenuItems << makeMenuItem("frame-text");
    addMenuItems << makeMenuItem("title-text");
    addMenuItems << makeMenuItem("subtitle-text");
    addMenuItems << makeMenuItem("composer-text");
    addMenuItems << makeMenuItem("poet-text");
    addMenuItems << makeMenuItem("part-text");
    addMenuItems << makeMenuItem("add-image");

    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenu(TranslatableString("notation", "Add"), addMenuItems);

    return items;
}

MenuItemList NotationContextMenuModel::makeHorizontalBoxItems()
{
    MenuItemList addMenuItems;
    addMenuItems << makeMenuItem("frame-text");
    addMenuItems << makeMenuItem("add-image");

    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenu(TranslatableString("notation", "Add"), addMenuItems);

    return items;
}

bool NotationContextMenuModel::isSingleSelection() const
{
    INotationSelectionPtr selection = this->selection();
    return selection ? selection->element() != nullptr : false;
}

bool NotationContextMenuModel::canSelectSimilar() const
{
    return hitElementContext().element != nullptr;
}

bool NotationContextMenuModel::canSelectSimilarInRange() const
{
    return canSelectSimilar() && selection()->isRange();
}

bool NotationContextMenuModel::isDrumsetStaff() const
{
    const INotationInteraction::HitElementContext& ctx = hitElementContext();
    if (!ctx.staff) {
        return false;
    }

    Fraction tick = ctx.element ? ctx.element->tick() : Fraction { -1, 1 };
    return ctx.staff->part()->instrument(tick)->drumset() != nullptr;
}

INotationInteractionPtr NotationContextMenuModel::interaction() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

INotationSelectionPtr NotationContextMenuModel::selection() const
{
    INotationInteractionPtr interaction = this->interaction();
    return interaction ? interaction->selection() : nullptr;
}

const INotationInteraction::HitElementContext& NotationContextMenuModel::hitElementContext() const
{
    if (INotationInteractionPtr interaction = this->interaction()) {
        return interaction->hitElementContext();
    }

    static INotationInteraction::HitElementContext dummy;
    return dummy;
}
