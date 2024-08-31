/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H
#define MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H

#include "uicomponents/view/selectableitemlistmodel.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuiactionsregister.h"

class QItemSelectionModel;

#if (defined(_MSCVER) || defined(_MSC_VER))
// unreferenced function with internal linkage has been removed
#pragma warning(disable: 4505)
#endif

namespace mu::notation {
class NoteInputBarCustomiseItem;
class NoteInputBarCustomiseModel : public muse::uicomponents::SelectableItemListModel, public muse::Injectable,
    public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)
    Q_PROPERTY(bool isAddSeparatorAvailable READ isAddSeparatorAvailable NOTIFY isAddSeparatorAvailableChanged)

    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::ui::IUiActionsRegister> actionsRegister = { this };

public:
    explicit NoteInputBarCustomiseModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QItemSelectionModel* selectionModel() const;
    bool isAddSeparatorAvailable() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void addSeparatorLine();

signals:
    void isAddSeparatorAvailableChanged(bool isAddSeparatorAvailable);

private:
    enum Roles {
        ItemRole = SelectableItemListModel::UserRole + 1
    };

    NoteInputBarCustomiseItem* modelIndexToItem(const QModelIndex& index) const;

    void onUpdateOperationsAvailability() override;
    void onRowsMoved() override;
    void onRowsRemoved() override;

    void updateRemovingAvailability();
    void updateAddSeparatorAvailability();
    void setIsAddSeparatorAvailable(bool isAddSeparatorAvailable);

    NoteInputBarCustomiseItem* makeItem(const muse::ui::UiAction& action, bool checked);
    NoteInputBarCustomiseItem* makeSeparatorItem();

    void saveActions();

    bool m_isAddSeparatorAvailable = false;
};
}

#endif // MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H
