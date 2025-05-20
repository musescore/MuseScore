/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "fretframechordlistmodel.h"

#include "fretframechorditem.h"

#include "translation.h"

using namespace mu::inspector;

FretFrameChordListModel::FretFrameChordListModel(QObject* parent)
    : muse::uicomponents::SelectableItemListModel(parent)
{
}

QVariant FretFrameChordListModel::data(const QModelIndex& index, int role) const
{
    FretFrameChordItem* item = modelIndexToItem(index);
    if (!item) {
        return QVariant();
    }

    switch (role) {
    case ItemRole: return QVariant::fromValue(item);
    default: break;
    }

    return SelectableItemListModel::data(index, role);
}

QHash<int, QByteArray> FretFrameChordListModel::roleNames() const
{
    QHash<int, QByteArray> roles = SelectableItemListModel::roleNames();
    roles[ItemRole] = "item";

    return roles;
}

void FretFrameChordListModel::setChordItems(const ItemList& items)
{
    setItems(items);
}

QItemSelectionModel* FretFrameChordListModel::selectionModel() const
{
    return muse::uicomponents::SelectableItemListModel::selection();
}

FretFrameChordItem* FretFrameChordListModel::modelIndexToItem(const QModelIndex& index) const
{
    return dynamic_cast<FretFrameChordItem*>(item(index));
}
