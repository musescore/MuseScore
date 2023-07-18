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
#include "notation/iselectinstrumentscenario.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "shortcuts/ishortcutsregister.h"
#include "iinteractive.h"

namespace mu::uicomponents {
class ItemMultiSelectionModel;
}

class QItemSelectionModel;

namespace mu::instrumentsscene {
class InstrumentsPanelTreeModel : public QAbstractItemModel, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(notation::ISelectInstrumentsScenario, selectInstrumentsScenario)
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(framework::IInteractive, interactive)

    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY isRemovingAvailableChanged)
    Q_PROPERTY(bool isAddingAvailable READ isAddingAvailable NOTIFY isAddingAvailableChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(QString addInstrumentsKeyboardShortcut READ addInstrumentsKeyboardShortcut NOTIFY addInstrumentsKeyboardShortcutChanged)
    Q_PROPERTY(bool isInstrumentSelected READ isInstrumentSelected NOTIFY isInstrumentSelectedChanged)

public:
    explicit InstrumentsPanelTreeModel(QObject* parent = nullptr);
    ~InstrumentsPanelTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;
    bool isAddingAvailable() const;
    bool isEmpty() const;
    QString addInstrumentsKeyboardShortcut() const;
    bool isInstrumentSelected() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectRow(const QModelIndex& rowIndex);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void addInstruments();
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();
    Q_INVOKABLE void toggleVisibilityOfSelectedRows(bool visible);

    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

    Q_INVOKABLE QItemSelectionModel* selectionModel() const;

signals:
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isAddingAvailableChanged(bool isAddingAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);
    void isEmptyChanged();
    void addInstrumentsKeyboardShortcutChanged();
    void isInstrumentSelectedChanged(bool isInstrumentSelected);

private slots:
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex = QModelIndex());
    void updateRemovingAvailability();
    void updateIsInstrumentSelected();

private:
    bool removeRows(int row, int count, const QModelIndex& parent) override;

    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    void onMasterNotationChanged();
    void onNotationChanged();

    void initPartOrders();
    void onBeforeChangeNotation();
    void setLoadingBlocked(bool blocked);

    void sortParts(notation::PartList& parts);

    void setupPartsConnections();
    void setupStavesConnections(const ID& stavesPartId);
    void listenNotationSelectionChanged();

    void clear();
    void deleteItems();

    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);
    void setIsInstrumentSelected(bool isInstrumentSelected);

    void setItemsSelected(const QModelIndexList& indexes, bool selected);

    bool warnAboutRemovingInstrumentsIfNecessary(int count);

    AbstractInstrumentsPanelTreeItem* loadMasterPart(const notation::Part* masterPart);
    AbstractInstrumentsPanelTreeItem* buildPartItem(const mu::notation::Part* masterPart);
    AbstractInstrumentsPanelTreeItem* buildMasterStaffItem(const mu::notation::Staff* masterStaff, QObject* parent);
    AbstractInstrumentsPanelTreeItem* buildAddStaffControlItem(const ID& partId, QObject* parent);
    AbstractInstrumentsPanelTreeItem* modelIndexToItem(const QModelIndex& index) const;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
    bool m_isInstrumentSelected = false;
    bool m_isLoadingBlocked = false;
    bool m_notationChangedWhileLoadingWasBlocked = false;

    AbstractInstrumentsPanelTreeItem* m_rootItem = nullptr;
    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
    mu::notation::IMasterNotationPtr m_masterNotation = nullptr;
    mu::notation::INotationPtr m_notation = nullptr;
    std::shared_ptr<async::Asyncable> m_partsNotifyReceiver = nullptr;

    using NotationKey = QString;
    QHash<NotationKey, QList<ID> > m_sortedPartIdList;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTPANELTREEMODEL_H
