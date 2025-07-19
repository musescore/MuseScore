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

#include "undohistorymodel.h"

using namespace mu::notation;
using namespace muse;

UndoHistoryModel::UndoHistoryModel(QObject* parent)
    : QAbstractListModel(parent), Injectable(iocCtxForQmlObject(this))
{
}

void UndoHistoryModel::classBegin()
{
    onCurrentNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this] {
        onCurrentNotationChanged();
    });
}

void UndoHistoryModel::onCurrentNotationChanged()
{
    auto stack = undoStack();

    if (stack) {
        stack->stackChanged().onNotify(this, [this] {
            onUndoRedo();
        });
    }

    beginResetModel();
    m_rowCount = stack ? int(stack->undoRedoActionCount()) + 1 : 0;
    endResetModel();

    emit currentIndexChanged();
}

void UndoHistoryModel::onUndoRedo()
{
    auto stack = undoStack();

    int newRowCount = stack ? int(stack->undoRedoActionCount()) + 1 : 0;

    if (m_rowCount < newRowCount) {
        beginInsertRows(QModelIndex(), m_rowCount, newRowCount - 1);
        m_rowCount = newRowCount;
        endInsertRows();
    } else if (m_rowCount > newRowCount) {
        beginRemoveRows(QModelIndex(), newRowCount, m_rowCount - 1);
        m_rowCount = newRowCount;
        endRemoveRows();
    }

    // When performing a new action after undoing one or more actions, the
    // redo stack is cleared, and the new action is pushed onto the stack;
    // that means that the item at the current index now represents the new
    // action, rather than the action on the redo stack.
    int newCurrentIndex = stack ? int(stack->currentStateIndex()) : 0;
    emit dataChanged(index(newCurrentIndex), index(newCurrentIndex));

    emit currentIndexChanged();
}

QVariant UndoHistoryModel::data(const QModelIndex& index, int role) const
{
    auto stack = undoStack();
    int row = index.row();
    if (!stack || row < 0 || row >= int(stack->undoRedoActionCount()) + 1) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        if (row == 0) {
            return qtrc("notation/undohistory", "File opened");
        }
        return stack->lastActionNameAtIdx(static_cast<size_t>(row)).qTranslated();
    default:
        return {};
    }
}

int UndoHistoryModel::rowCount(const QModelIndex&) const
{
    return m_rowCount;
}

QHash<int, QByteArray> UndoHistoryModel::roleNames() const
{
    return { { Qt::DisplayRole, "text" } };
}

int UndoHistoryModel::currentIndex() const
{
    if (auto stack = undoStack()) {
        return int(stack->currentStateIndex());
    }

    return 0;
}

void UndoHistoryModel::undoRedoToIndex(int index)
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    return notation->interaction()->undoRedoToIndex(static_cast<size_t>(index));
}

INotationUndoStackPtr UndoHistoryModel::undoStack() const
{
    INotationPtr notation = context()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
