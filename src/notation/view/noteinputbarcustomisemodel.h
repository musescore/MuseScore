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
#ifndef MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H
#define MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuiactionsregister.h"

#include "noteinputbarcustomiseitem.h"

class QItemSelectionModel;

namespace mu::uicomponents {
class ItemMultiSelectionModel;
}

namespace mu::notation {
class NoteInputBarCustomiseModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, ui::IUiConfiguration, uiConfiguration)
    INJECT(notation, ui::IUiActionsRegister, actionsRegister)

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)
    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY isRemovingAvailableChanged)
    Q_PROPERTY(bool isAddSeparatorAvailable READ isAddSeparatorAvailable NOTIFY isAddSeparatorAvailableChanged)

public:
    explicit NoteInputBarCustomiseModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QItemSelectionModel* selectionModel() const;
    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;
    bool isAddSeparatorAvailable() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void addSeparatorLine();
    Q_INVOKABLE void selectRow(int row);
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();
    Q_INVOKABLE bool isSelected(int row) const;

    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

signals:
    void selectionChanged(QItemSelectionModel* selectionModel);
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);
    void isAddSeparatorAvailableChanged(bool isAddSeparatorAvailable);

private:
    enum Roles {
        ItemRole = Qt::UserRole + 1,
        SelectedRole
    };

    NoteInputBarCustomiseItem* modelIndexToItem(const QModelIndex& index) const;

    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);
    void setIsAddSeparatorAvailable(bool isAddSeparatorAvailable);

    void updateOperationsAvailability();
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(const QModelIndex& selectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(const QModelIndex& selectedRowIndex = QModelIndex());
    void updateRemovingAvailability();
    void updateAddSeparatorAvailability();

    NoteInputBarCustomiseItem* makeItem(const ui::UiAction& action, bool checked);
    NoteInputBarCustomiseItem* makeSeparatorItem();

    void saveActions();

    QList<NoteInputBarCustomiseItem*> m_items;
    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
    bool m_isAddSeparatorAvailable = false;
};
}

#endif // MU_NOTATION_NOTEINPUTBARCUSOMISEMODEL_H
