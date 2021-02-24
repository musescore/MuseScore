//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "undoredomodel.h"

#include "uicomponents/uicomponentstypes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::uicomponents;

UndoRedoModel::UndoRedoModel(QObject* parent)
    : QObject(parent)
{
}

QVariant UndoRedoModel::undoItem() const
{
    MenuItem item = actionsRegister()->action("undo");
    item.enabled = undoStack() ? undoStack()->canUndo() : false;

    return item.toMap();
}

QVariant UndoRedoModel::redoItem() const
{
    MenuItem item = actionsRegister()->action("redo");
    item.enabled = undoStack() ? undoStack()->canRedo() : false;

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
        undoStack()->redo();
    }
}

void UndoRedoModel::undo()
{
    if (undoStack()) {
        undoStack()->undo();
    }
}

INotationUndoStackPtr UndoRedoModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
