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

#pragma once

#include <QAbstractItemModel>
#include <QVariant>

#include "abstractlayoutpaneltreeitem.h"
#include "modularity/ioc.h"
#include "notation/iselectinstrumentscenario.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "shortcuts/ishortcutsregister.h"
#include "iinteractive.h"
#include "layoutpanelutils.h"

Q_MOC_INCLUDE(< QItemSelectionModel >)

namespace muse::uicomponents {
class ItemMultiSelectionModel;
}

class QItemSelectionModel;

namespace mu::instrumentsscene {
class PartTreeItem;
class LayoutPanelTreeModel : public QAbstractItemModel, public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(notation::ISelectInstrumentsScenario, selectInstrumentsScenario)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(muse::shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(muse::IInteractive, interactive)

    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY isRemovingAvailableChanged)
    Q_PROPERTY(bool isAddingAvailable READ isAddingAvailable NOTIFY isAddingAvailableChanged)
    Q_PROPERTY(bool isAddingSystemMarkingsAvailable READ isAddingSystemMarkingsAvailable NOTIFY isAddingSystemMarkingsAvailableChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(QString addInstrumentsKeyboardShortcut READ addInstrumentsKeyboardShortcut NOTIFY addInstrumentsKeyboardShortcutChanged)
    Q_PROPERTY(int selectedItemsType READ selectedItemsType NOTIFY selectedItemsTypeChanged)

public:
    explicit LayoutPanelTreeModel(QObject* parent = nullptr);
    ~LayoutPanelTreeModel() override;

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
    bool isAddingSystemMarkingsAvailable() const;
    bool isEmpty() const;
    QString addInstrumentsKeyboardShortcut() const;
    int selectedItemsType() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setLayoutPanelVisible(bool visible);
    Q_INVOKABLE void selectRow(const QModelIndex& rowIndex);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void addInstruments();
    Q_INVOKABLE void addSystemMarkings();
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();
    Q_INVOKABLE void changeVisibilityOfSelectedRows(bool visible);
    Q_INVOKABLE void changeVisibility(const QModelIndex& index, bool visible);

    Q_INVOKABLE void startActiveDrag();
    Q_INVOKABLE void endActiveDrag();

    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

    Q_INVOKABLE QItemSelectionModel* selectionModel() const;

signals:
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isAddingAvailableChanged(bool isAddingAvailable);
    void isAddingSystemMarkingsAvailableChanged(bool isAddingSystemMarkingsAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);
    void isEmptyChanged();
    void addInstrumentsKeyboardShortcutChanged();
    void selectedItemsTypeChanged(int type);

private slots:
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex = QModelIndex());
    void updateRemovingAvailability();
    void updateSelectedItemsType();
    void updateIsAddingSystemMarkingsAvailable();

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
    void setupStavesConnections(const muse::ID& partId);
    void setupNotationConnections();

    void updateSelectedRows();
    void onScoreChanged(const mu::engraving::ScoreChangesRange& changes);

    void clear();
    void deleteItems();

    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);

    void setItemsSelected(const QModelIndexList& indexes, bool selected);

    bool needWarnOnRemoveRows(int row, int count);
    bool warnAboutRemovingInstrumentsIfNecessary(int count);

    AbstractLayoutPanelTreeItem* buildMasterPartItem(const notation::Part* masterPart);
    AbstractLayoutPanelTreeItem* buildMasterStaffItem(const mu::notation::Staff* masterStaff, QObject* parent);
    AbstractLayoutPanelTreeItem* buildSystemObjectsLayerItem(const mu::notation::Staff* masterStaff,
                                                             const SystemObjectGroups& systemObjects);
    AbstractLayoutPanelTreeItem* buildAddStaffControlItem(const muse::ID& partId, QObject* parent);
    AbstractLayoutPanelTreeItem* modelIndexToItem(const QModelIndex& index) const;

    void updateSystemObjectLayers();

    const PartTreeItem* findPartItemByStaff(const notation::Staff* staff) const;
    const notation::Staff* resolveNewSystemObjectStaff() const;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
    bool m_isLoadingBlocked = false;
    bool m_notationChangedWhileLoadingWasBlocked = false;
    bool m_isAddingSystemMarkingsAvailable = false;

    LayoutPanelItemType::ItemType m_selectedItemsType = LayoutPanelItemType::ItemType::UNDEFINED;

    AbstractLayoutPanelTreeItem* m_rootItem = nullptr;
    muse::uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
    mu::notation::IMasterNotationPtr m_masterNotation = nullptr;
    mu::notation::INotationPtr m_notation = nullptr;
    std::shared_ptr<muse::async::Asyncable> m_partsNotifyReceiver = nullptr;

    using NotationKey = QString;
    QHash<NotationKey, QList<muse::ID> > m_sortedPartIdList;

    bool m_layoutPanelVisible = true;
    bool m_shouldUpdateSystemObjectLayers = false;

    bool m_dragInProgress = false;
    AbstractLayoutPanelTreeItem* m_dragSourceParentItem = nullptr;
    MoveParams m_activeDragMoveParams;

    muse::ID m_systemStaffToSelect;
};
}
