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

#include "palettespanelcontextmenumodel.h"

#include "log.h"
#include "translation.h"

#include "actions/actiontypes.h"

using namespace mu::palette;
using namespace mu::actions;
using namespace mu::ui;
using namespace mu::uicomponents;

static const ActionCode TOGGLE_SINGLE_PALETTE_CODE("toggle-single-palette");

PalettesPanelContextMenuModel::PalettesPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PalettesPanelContextMenuModel::load()
{
    setItems({
        createIsSinglePaletteItem(),
    });
}

MenuItem* PalettesPanelContextMenuModel::createIsSinglePaletteItem()
{
    MenuItem* item = new MenuItem(this);

    UiAction action;
    action.title = qtrc("palette", "Open only one Palette at a time");
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
