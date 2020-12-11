//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "itemmultiselectionmodel.h"

#include <QApplication>

using namespace mu::framework;

ItemMultiSelectionModel::ItemMultiSelectionModel(QAbstractItemModel* parent)
    : QItemSelectionModel(parent)
{
}

void ItemMultiSelectionModel::select(const QModelIndex& index)
{
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

    QModelIndex startIndex = index;
    if (modifiers == Qt::ShiftModifier && hasSelection()) {
        startIndex = selectedIndexes().last();
    } else if (modifiers == Qt::ControlModifier && isSelected(index)) {
        QItemSelectionModel::select(index, SelectionFlag::Deselect);
        return;
    }

    QSet<QModelIndex> uniqueIndexes;

    if (needClearSelection(modifiers)) {
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

bool ItemMultiSelectionModel::needClearSelection(Qt::KeyboardModifiers modifiers) const
{
    switch (modifiers) {
    case Qt::ShiftModifier:
    case Qt::ControlModifier:
        return false;
    case Qt::NoModifier:
    case Qt::AltModifier:
    case Qt::MetaModifier:
    case Qt::KeypadModifier:
    case Qt::GroupSwitchModifier:
    case Qt::KeyboardModifierMask:
        return true;
    }

    return true;
}
