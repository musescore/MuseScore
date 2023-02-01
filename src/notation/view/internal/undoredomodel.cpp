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

#include "undoredomodel.h"

#include "uicomponents/view/menuitem.h"

using namespace mu::notation;
using namespace mu::uicomponents;

UndoRedoModel::UndoRedoModel(QObject* parent)
    : QObject(parent)
{
}

QVariant UndoRedoModel::makeUndoItem()
{
    MenuItem* item = new MenuItem(actionsRegister()->action("undo"), this);

    ui::UiActionState state;
    state.enabled = undoStack() ? undoStack()->canUndo() : false;
    item->setState(state);

    return QVariant::fromValue(item);
}

QVariant UndoRedoModel::makeRedoItem()
{
    MenuItem* item = new MenuItem(actionsRegister()->action("redo"), this);

    ui::UiActionState state;
    state.enabled = undoStack() ? undoStack()->canRedo() : false;
    item->setState(state);

    return QVariant::fromValue(item);
}

void UndoRedoModel::load()
{
    context()->currentNotationChanged().onNotify(this, [this]() {
        if (!undoStack()) {
            emit stackChanged();
            return;
        }

        undoStack()->stackChanged().onNotify(this, [this]() {
            emit stackChanged();
        });
    });

    context()->currentProjectChanged().onNotify(this, [this]() {
        emit stackChanged();
    });

    emit stackChanged();
}

void UndoRedoModel::redo()
{
    if (undoStack()) {
        undoStack()->redo(nullptr);
    }
}

void UndoRedoModel::undo()
{
    if (undoStack()) {
        undoStack()->undo(nullptr);
    }
}

INotationUndoStackPtr UndoRedoModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
