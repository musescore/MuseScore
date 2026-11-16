/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "palettecommands.h"

#include "log.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;

PalettesPanelContextMenuModel::PalettesPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PalettesPanelContextMenuModel::load()
{
    AbstractMenuModel::load();

    commandsController()->expandCollapseAllRequested().onReceive(this, [this](bool expand) {
        emit expandCollapseAllRequested(expand);
    }, Asyncable::Mode::SetReplace);

    MenuItemList items {
        makeMenuItem(PALETTE_TOGGLE_SINGLE_CLICK_TO_OPEN_COMMAND),
        makeMenuItem(PALETTE_TOGGLE_SINGLE_PALETTE_COMMAND),
        makeMenuItem(PALETTE_TOGGLE_DRAG_ENABLED_COMMAND),
        makeSeparator(),
        makeMenuItem(PALETTE_EXPAND_ALL_COMMAND),
        makeMenuItem(PALETTE_COLLAPSE_ALL_COMMAND),
    };

    setItems(items);
}
