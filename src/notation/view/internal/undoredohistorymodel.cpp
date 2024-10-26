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

#include "undoredohistorymodel.h"

using namespace mu::notation;
using namespace muse;

UndoRedoHistoryModel::UndoRedoHistoryModel(QObject* parent)
    : QObject(parent), Injectable(iocCtxForQmlObject(this))
{
}

size_t UndoRedoHistoryModel::undoRedoActionCount() const
{
    if (auto stack = undoStack()) {
        return stack->undoRedoActionCount();
    }

    return 0;
}

size_t UndoRedoHistoryModel::undoRedoActionCurrentIdx() const
{
    if (auto stack = undoStack()) {
        return stack->undoRedoActionCurrentIdx();
    }

    return muse::nidx;
}

const QString UndoRedoHistoryModel::undoRedoActionNameAtIdx(size_t idx) const
{
    if (auto stack = undoStack()) {
        return stack->undoRedoActionNameAtIdx(idx).qTranslated();
    }

    return {};
}

INotationUndoStackPtr UndoRedoHistoryModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
