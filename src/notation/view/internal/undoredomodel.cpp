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

using namespace mu::notation;
using namespace mu::ui;

UndoRedoModel::UndoRedoModel(QObject* parent)
    : QObject(parent)
{
}

QVariant UndoRedoModel::undoItem() const
{
    MenuItem item = actionsRegister()->action("undo");
    item.state.enabled = undoStack() ? undoStack()->canUndo() : false;

    return item.toMap();
}

QVariant UndoRedoModel::redoItem() const
{
    MenuItem item = actionsRegister()->action("redo");
    item.state.enabled = undoStack() ? undoStack()->canRedo() : false;

    return item.toMap();
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
