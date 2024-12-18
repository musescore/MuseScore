/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "palettespanelcontextmenumodel.h"

#include "actions/actiontypes.h"
#include "types/translatablestring.h"

#include "log.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;

static const ActionCode TOGGLE_SINGLE_CLICK_CODE("toggle-single-click-to-open-palette");
static const ActionCode TOGGLE_SINGLE_PALETTE_CODE("toggle-single-palette");
static const ActionCode TOGGLE_PALETTE_DRAG("toggle-palette-drag");
static const ActionCode EXPAND_ALL_CODE("expand-all-palettes");
static const ActionCode COLLAPSE_ALL_CODE("collapse-all-palettes");

PalettesPanelContextMenuModel::PalettesPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PalettesPanelContextMenuModel::load()
{
    MenuItemList items {
        createIsSingleClickToOpenPaletteItem(),
        createIsSinglePaletteItem(),
        createIsDragEnabledItem(),
        makeSeparator(),
        createExpandCollapseAllItem(false),
        createExpandCollapseAllItem(true),
    };

    setItems(items);
}

MenuItem* PalettesPanelContextMenuModel::createIsSingleClickToOpenPaletteItem()
{
    MenuItem* item = new MenuItem(this);
    item->setId(QString::fromStdString(TOGGLE_SINGLE_CLICK_CODE));

    UiAction action;
    action.title = TranslatableString("palette", "Single-click to open a palette");
    action.code = TOGGLE_SINGLE_CLICK_CODE;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    ValCh<bool> checked = configuration()->isSingleClickToOpenPalette();

    UiActionState state;
    state.enabled = true;
    state.checked = checked.val;
    item->setState(state);

    checked.ch.onReceive(item, [item](bool newValue) {
        UiActionState state = item->state();
        state.checked = newValue;
        item->setState(state);
    });

    dispatcher()->reg(this, TOGGLE_SINGLE_CLICK_CODE, [this, item]() {
        bool newValue = !item->state().checked;
        configuration()->setIsSingleClickToOpenPalette(newValue);
    });

    return item;
}

MenuItem* PalettesPanelContextMenuModel::createIsSinglePaletteItem()
{
    MenuItem* item = new MenuItem(this);
    item->setId(QString::fromStdString(TOGGLE_SINGLE_PALETTE_CODE));

    UiAction action;
    //: This is the name of a setting that can be turned on or off. "Open" is a verb here.
    action.title = TranslatableString("palette", "Open only one palette at a time");
    action.code = TOGGLE_SINGLE_PALETTE_CODE;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    ValCh<bool> checked = configuration()->isSinglePalette();

    UiActionState state;
    state.enabled = true;
    state.checked = checked.val;
    item->setState(state);

    checked.ch.onReceive(item, [item](bool newValue) {
        UiActionState state = item->state();
        state.checked = newValue;
        item->setState(state);
    });

    dispatcher()->reg(this, TOGGLE_SINGLE_PALETTE_CODE, [this, item]() {
        bool newValue = !item->state().checked;
        configuration()->setIsSinglePalette(newValue);
    });

    return item;
}

MenuItem* PalettesPanelContextMenuModel::createIsDragEnabledItem()
{
    MenuItem* item = new MenuItem(this);
    item->setId(QString::fromStdString(TOGGLE_PALETTE_DRAG));

    UiAction action;
    action.title = TranslatableString("palette", "Allow reordering palettes");
    action.code = TOGGLE_PALETTE_DRAG;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    ValCh<bool> checked = configuration()->isPaletteDragEnabled();

    UiActionState state;
    state.enabled = true;
    state.checked = checked.val;
    item->setState(state);

    checked.ch.onReceive(item, [item](bool newValue) {
        UiActionState state = item->state();
        state.checked = newValue;
        item->setState(state);
    });

    dispatcher()->reg(this, TOGGLE_PALETTE_DRAG, [this, item]() {
        bool newValue = !item->state().checked;
        configuration()->setIsPaletteDragEnabled(newValue);
    });

    return item;
}

MenuItem* PalettesPanelContextMenuModel::createExpandCollapseAllItem(bool expand)
{
    MenuItem* item = new MenuItem(this);
    item->setId(QString::fromStdString(expand ? EXPAND_ALL_CODE : COLLAPSE_ALL_CODE));

    UiAction action;
    action.title = expand ? TranslatableString("palette", "Expand all palettes")
                   : TranslatableString("palette", "Collapse all palettes");
    action.code = expand ? EXPAND_ALL_CODE : COLLAPSE_ALL_CODE;
    item->setAction(action);

    // TODO: enabled these actions based on number of expanded palettes
    UiActionState state;
    state.enabled = true;

    if (expand) {
        ValCh<bool> disabled = configuration()->isSinglePalette();
        state.enabled = !disabled.val;

        disabled.ch.onReceive(item, [item](bool newValue) {
            UiActionState state = item->state();
            state.enabled = !newValue;
            item->setState(state);
        });
    }

    item->setState(state);

    dispatcher()->reg(this, action.code, [this, expand]() {
        emit expandCollapseAllRequested(expand);
    });

    return item;
}
