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

#pragma once

#include "uicomponents/view/selectableitemlistmodel.h"

namespace mu::inspector {
class FretFrameChordItem;
class FretFrameChordListModel : public muse::uicomponents::SelectableItemListModel
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)

public:
    explicit FretFrameChordListModel(QObject* parent);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setChordItems(const ItemList& items);

    QItemSelectionModel* selectionModel() const;

private:
    enum Roles {
        ItemRole = SelectableItemListModel::UserRole + 1
    };

    FretFrameChordItem* modelIndexToItem(const QModelIndex& index) const;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    void loadListItems();
};
}
