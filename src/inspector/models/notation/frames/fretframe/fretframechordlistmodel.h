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

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "uicomponents/view/selectableitemlistmodel.h"

namespace mu::engraving {
class FBox;
}

namespace mu::inspector {
class FretFrameChordItem;
class FretFrameChordListModel : public muse::uicomponents::SelectableItemListModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit FretFrameChordListModel(QObject* parent);

    void load(engraving::FBox* box);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setChordVisible(int index, bool visible);
    Q_INVOKABLE void moveSelectionUp();
    Q_INVOKABLE void moveSelectionDown();

    QItemSelectionModel* selectionModel() const;

private:
    enum Roles {
        ItemRole = SelectableItemListModel::UserRole + 1
    };

    FretFrameChordItem* modelIndexToItem(const QModelIndex& index) const;

    engraving::FBox* m_fretBox = nullptr;
};
}
