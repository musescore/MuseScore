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

#include "undoredomodel.h"

#include "uicomponents/view/toolbaritem.h"

using namespace mu::notation;
using namespace muse::uicomponents;

UndoRedoModel::UndoRedoModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

void UndoRedoModel::load()
{
    muse::actions::ActionCodeList itemsCodes = {
        "undo",
        "redo"
    };

    ToolBarItemList items;
    for (const muse::actions::ActionCode& code : itemsCodes) {
        ToolBarItem* item = makeItem(code);
        item->setIsTransparent(true);
        items << item;
    }

    setItems(items);

    AbstractToolBarModel::load();
}
