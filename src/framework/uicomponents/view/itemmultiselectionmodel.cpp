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

#include "itemmultiselectionmodel.h"

#include <QApplication>

using namespace mu::uicomponents;

ItemMultiSelectionModel::ItemMultiSelectionModel(QAbstractItemModel* parent)
    : QItemSelectionModel(parent)
{
    m_allowedModifiers = (Qt::ShiftModifier | Qt::ControlModifier);
}

void ItemMultiSelectionModel::setAllowedModifiers(Qt::KeyboardModifiers modifiers)
{
    m_allowedModifiers = modifiers;
}

void ItemMultiSelectionModel::select(const QModelIndex& index)
{
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    bool modifiersAllowed = m_allowedModifiers & modifiers;

    QModelIndex startIndex = index;
    if (modifiers == Qt::ShiftModifier && hasSelection() && modifiersAllowed) {
        startIndex = selectedIndexes().last();
    } else if (modifiers == Qt::ControlModifier && isSelected(index) && modifiersAllowed) {
        QItemSelectionModel::select(index, SelectionFlag::Deselect);
        return;
    }

    QSet<QModelIndex> uniqueIndexes;
    bool needClearSelection = !modifiersAllowed;

    if (needClearSelection) {
        uniqueIndexes << index;
    } else {
        QItemSelection selection = this->selection();
        selection.select(startIndex, index);
        QModelIndexList indexes = selection.indexes();
        uniqueIndexes = QSet<QModelIndex>(indexes.begin(), indexes.end());
    }

    QItemSelection newSelection;
    for (const QModelIndex& selectedIndex : uniqueIndexes) {
        newSelection.select(selectedIndex, selectedIndex);
    }

    QItemSelectionModel::select(newSelection, SelectionFlag::ClearAndSelect);
}
