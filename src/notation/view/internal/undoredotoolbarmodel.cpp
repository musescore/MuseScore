/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "undoredotoolbarmodel.h"

#include "uicomponents/view/toolbaritem.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::uicomponents;

static const actions::ActionCode UNDO_ACTION_CODE("undo");
static const actions::ActionCode REDO_ACTION_CODE("redo");

UndoRedoToolbarModel::UndoRedoToolbarModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

void UndoRedoToolbarModel::load()
{
    actions::ActionCodeList itemsCodes = {
        UNDO_ACTION_CODE,
        REDO_ACTION_CODE
    };

    ToolBarItemList items;
    for (const actions::ActionCode& code : itemsCodes) {
        ToolBarItem* item = makeItem(code);
        item->setIsTransparent(true);
        items << item;
    }

    setItems(items);

    AbstractToolBarModel::load();

    context()->currentNotationChanged().onNotify(this, [this]() {
        updateItems();

        subsribeOnUndoStackChanges();
    });

    subsribeOnUndoStackChanges();
}

void UndoRedoToolbarModel::onActionsStateChanges(const muse::actions::ActionCodeList& codes)
{
    auto stack = undoStack();

    for (const actions::ActionCode& code : codes) {
        if (code == UNDO_ACTION_CODE) {
            ToolBarItem* undoItem = findItemPtr(UNDO_ACTION_CODE);
            if (undoItem) {
                const TranslatableString undoActionName = stack ? stack->topMostUndoActionName() : TranslatableString();
                undoItem->setTitle(undoActionName.isEmpty()
                                   ? TranslatableString("action", "Undo")
                                   : TranslatableString("action", "Undo ‘%1’").arg(undoActionName));
            }
        } else if (code == REDO_ACTION_CODE) {
            ToolBarItem* redoItem = findItemPtr(REDO_ACTION_CODE);
            if (redoItem) {
                const TranslatableString redoActionName = stack ? stack->topMostRedoActionName() : TranslatableString();
                redoItem->setTitle(redoActionName.isEmpty()
                                   ? TranslatableString("action", "Redo")
                                   : TranslatableString("action", "Redo ‘%1’").arg(redoActionName));
            }
        }
    }

    AbstractToolBarModel::onActionsStateChanges(codes);
}

INotationUndoStackPtr UndoRedoToolbarModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}

void UndoRedoToolbarModel::updateItems()
{
    onActionsStateChanges({ UNDO_ACTION_CODE, REDO_ACTION_CODE });
}

void UndoRedoToolbarModel::subsribeOnUndoStackChanges()
{
    auto stack = undoStack();
    if (!stack) {
        return;
    }

    stack->stackChanged().onNotify(this, [this]() {
        updateItems();
    });
}
