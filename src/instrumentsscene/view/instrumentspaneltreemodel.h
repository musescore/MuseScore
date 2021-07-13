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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTPANELTREEMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTPANELTREEMODEL_H

#include <QAbstractItemModel>
#include <QVariant>

#include "abstractinstrumentspaneltreeitem.h"
#include "modularity/ioc.h"
#include "notation/inotationparts.h"
#include "notation/iselectinstrumentscenario.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"

namespace mu::uicomponents {
class ItemMultiSelectionModel;
}

class QItemSelectionModel;

namespace mu::instrumentsscene {
class PartTreeItem;
class StaffTreeItem;
class InstrumentsPanelTreeModel : public QAbstractItemModel, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(instruments, context::IGlobalContext, context)
    INJECT(instruments, notation::ISelectInstrumentsScenario, selectInstrumentsScenario)
    INJECT(instruments, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)
    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY isRemovingAvailableChanged)
    Q_PROPERTY(bool isAddingAvailable READ isAddingAvailable NOTIFY isAddingAvailableChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

public:
    explicit InstrumentsPanelTreeModel(QObject* parent = nullptr);
    ~InstrumentsPanelTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QItemSelectionModel* selectionModel() const;
    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;
    bool isAddingAvailable() const;
    bool isEmpty() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectRow(const QModelIndex& rowIndex);
    Q_INVOKABLE bool isSelected(const QModelIndex& rowIndex) const;
    Q_INVOKABLE void addInstruments();
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();

    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

signals:
    void selectionChanged();
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isAddingAvailableChanged(bool isAddingAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);
    void isEmptyChanged();

private slots:
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(const bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(const bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex = QModelIndex());
    void updateRemovingAvailability();

private:
    bool canReceiveAction(const actions::ActionCode&) const override;

    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    void clear();
    void deleteItems();

    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);

    bool removeRows(int row, int count, const QModelIndex& parent) override;

    notation::IDList currentNotationPartIdList() const;

    AbstractInstrumentsPanelTreeItem* loadPart(const notation::Part* part);

    AbstractInstrumentsPanelTreeItem* modelIndexToItem(const QModelIndex& index) const;

    void updatePartItem(PartTreeItem* item, const notation::Part* part);
    void updateStaffItem(StaffTreeItem* item, const mu::notation::Staff* staff);

    AbstractInstrumentsPanelTreeItem* buildPartItem(const mu::notation::Part* part);
    AbstractInstrumentsPanelTreeItem* buildStaffItem(const mu::notation::Staff* staff);
    AbstractInstrumentsPanelTreeItem* buildAddStaffControlItem(const QString& partId);

    AbstractInstrumentsPanelTreeItem* m_rootItem = nullptr;
    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;

    mu::notation::INotationPartsPtr m_masterNotationParts = nullptr;
    mu::notation::INotationPartsPtr m_notationParts = nullptr;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
    bool m_isLoadingBlocked = false;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTPANELTREEMODEL_H
