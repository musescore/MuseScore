/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <QQmlParserStatus>
#include <QVariant>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "abstractlayoutpaneltreeitem.h"

#include "modularity/ioc.h"
#include "notationscene/iselectinstrumentscenario.h"
#include "project/iprojectsession.h"
#include "rcommand/icommanddispatcher.h"
#include "interactive/iinteractive.h"
#include "shortcuts_v2/icommandshortcutsregister.h"

#include "layoutpanelutils.h"

Q_MOC_INCLUDE(< QItemSelectionModel >)

namespace muse::uicomponents {
class ItemMultiSelectionModel;
}

class QItemSelectionModel;

namespace mu::instrumentsscene {
class PartTreeItem;
class LayoutPanelTreeModel : public QAbstractItemModel, public QQmlParserStatus, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY isRemovingAvailableChanged)
    Q_PROPERTY(bool isAddingAvailable READ isAddingAvailable NOTIFY isAddingAvailableChanged)
    Q_PROPERTY(bool isAddingSystemMarkingsAvailable READ isAddingSystemMarkingsAvailable NOTIFY isAddingSystemMarkingsAvailableChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(QString addInstrumentsKeyboardShortcut READ addInstrumentsKeyboardShortcut NOTIFY addInstrumentsKeyboardShortcutChanged)
    Q_PROPERTY(int selectedItemsType READ selectedItemsType NOTIFY selectedItemsTypeChanged)
    Q_PROPERTY(bool isStaveSharingEnabled READ isStaveSharingEnabled NOTIFY isStaveSharingEnabledChanged FINAL)
    Q_PROPERTY(bool isHideEmptyStavesEnabled READ isHideEmptyStavesEnabled NOTIFY isHideEmptyStavesEnabledChanged FINAL)
    Q_PROPERTY(bool isMasterScore READ isMasterScore NOTIFY scoreChanged FINAL)
    Q_PROPERTY(QString scoreName READ scoreName NOTIFY scoreChanged FINAL)

    QML_ELEMENT

    muse::GlobalInject<muse::shortcuts::ICommandShortcutsRegister> commandShortcutsRegister;
    muse::ContextInject<project::IProjectSession> context = { this };
    muse::ContextInject<notation::ISelectInstrumentsScenario> selectInstrumentsScenario = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit LayoutPanelTreeModel(QObject* parent = nullptr);
    ~LayoutPanelTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE AbstractLayoutPanelTreeItem* modelIndexToItem(const QModelIndex& index) const;

    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;
    bool isAddingAvailable() const;
    bool isAddingSystemMarkingsAvailable() const;
    bool isEmpty() const;
    QString addInstrumentsKeyboardShortcut() const;
    int selectedItemsType() const;

    bool isStaveSharingEnabled() const;
    bool isHideEmptyStavesEnabled() const;
    QString scoreName() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void toggleStaveSharing(bool on);
    Q_INVOKABLE void toggleHideEmptyStaves(bool on);
    Q_INVOKABLE void setLayoutPanelVisible(bool visible);
    Q_INVOKABLE void selectRow(const QModelIndex& rowIndex);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void addInstruments();
    Q_INVOKABLE void addSystemMarkings();

    Q_INVOKABLE void addEmptySharedGroup();
    // Turns the empty group into a shared part, starting with the given part; returns the new shared part's id
    Q_INVOKABLE QString fillEmptySharedGroup(const QString& emptyGroupId, const QString& partId);
    Q_INVOKABLE void addPartToSharedGroup(const QString& partId, const QString& sharedPartId);
    Q_INVOKABLE void ungroupSharedGroup(const QString& sharedPartId);
    Q_INVOKABLE void removePartFromSharedGroup(const QString& partId);

    // Context menu actions on selected instruments (ids in list order)
    Q_INVOKABLE void duplicateParts(const QStringList& partIds);
    // Adds the parts to the group (an empty group or a shared part); returns the group's shared part id
    Q_INVOKABLE QString movePartsToSharedGroup(const QStringList& partIds, const QString& groupId, bool isEmptyGroup);
    // Creates a group with the parts (where the first part is); returns its shared part id
    Q_INVOKABLE QString createSharedGroupWithParts(const QStringList& partIds);
    Q_INVOKABLE void deleteEmptySharedGroup(const QString& emptyGroupId);
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();
    Q_INVOKABLE void changeVisibilityOfSelectedRows(bool visible);
    Q_INVOKABLE void changeVisibility(const QModelIndex& index, bool visible);
    Q_INVOKABLE void changeEnabledOfSelectedRows(bool enable);
    Q_INVOKABLE void changeEnabled(const QModelIndex& index, bool enable);

    Q_INVOKABLE void startActiveDrag();
    // Ends the drag without applying its move to the score (when the drop does something else, as one step)
    Q_INVOKABLE void cancelActiveDrag();
    // Ends the drag of an instrument moved out of its shared-staves group: moves it and leaves the group, as one step
    Q_INVOKABLE void endActiveDragLeavingSharedGroup(const QString& partId);
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
    void isStaveSharingEnabledChanged(bool enabled);
    void isHideEmptyStavesEnabledChanged(bool enabled);
    void scoreChanged();

private slots:
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex = QModelIndex());
    void updateRemovingAvailability();
    void updateSelectedItemsType();
    void updateIsAddingSystemMarkingsAvailable();
    void updateIsStaveSharingEnabled();
    void updateIsHideEmptyStavesEnabled();
    void updateSharedGroupRoles();

private:
    void classBegin() override;
    void componentComplete() override {}
    void init();

    bool removeRows(int row, int count, const QModelIndex& parent) override;

    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    void onMasterNotationChanged();
    void onNotationChanged();

    bool isMasterScore() const;

    bool shouldShowSystemObjectLayers() const;

    void onBeforeChangeNotation();
    void setLoadingBlocked(bool blocked);

    using PartList = std::vector<const engraving::Part*>;
    static void sortParts(PartList& parts, const PartList& referenceParts);

    void setupPartsConnections();
    void setupStavesConnections(const muse::ID& partId);
    void setupNotationConnections();

    void updateSelectedRows();
    void onScoreChanged(const mu::engraving::ScoreChanges& changes);

    void clear();
    void deleteItems();

    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);

    void setItemsSelected(const QModelIndexList& indexes, bool selected);
    void syncItemsSelection();

    // For moving rows up / down by one: if that would put them in among another shared-staves group's
    // instruments, the destination row past the whole group instead
    int skipOtherSharedGroup(const QModelIndex& movedIndex, int destinationRow, bool up) const;
    // Number of rows moved by the up / down buttons: the selection, plus a selected group header's instruments
    int movedRowCount(const QModelIndexList& sortedSelectedIndexes) const;

    bool needWarnOnRemoveRows(int row, int count);
    bool warnAboutRemovingInstrumentsIfNecessary(int count);

    AbstractLayoutPanelTreeItem* buildSharedPartItem(const notation::Part* part);
    AbstractLayoutPanelTreeItem* buildMasterPartItem(const notation::Part* masterPart);
    AbstractLayoutPanelTreeItem* buildMasterStaffItem(const mu::notation::Staff* masterStaff, QObject* parent);
    AbstractLayoutPanelTreeItem* buildSystemObjectsLayerItem(const mu::notation::Staff* masterStaff,
                                                             const SystemObjectGroups& systemObjects);
    AbstractLayoutPanelTreeItem* buildAddStaffControlItem(const muse::ID& partId, QObject* parent);

    void updateSystemObjectLayers();

    const PartTreeItem* findPartItemByStaff(const notation::Staff* staff) const;
    const notation::Staff* resolveNewSystemObjectStaff() const;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
    bool m_isLoadingBlocked = false;
    bool m_notationChangedWhileLoadingWasBlocked = false;
    bool m_isAddingSystemMarkingsAvailable = false;

    bool m_isStaveSharingEnabled = false;
    bool m_isHideEmptyStavesEnabled = false;

    // Empty shared-staves groups (see LayoutPanelItemType::EMPTY_SHARED_GROUP)
    struct EmptySharedGroup {
        int number = 0;
        QString title;
        bool atBottom = false;              // added via the Add menu: after the last instrument
        muse::ID anchorPartId;              // else: after this instrument (if invalid, at the top)
        muse::ID originSharedPartId;        // the group it was, before all its instruments left (if any)
    };
    QList<EmptySharedGroup> m_emptySharedGroups;
    int m_lastEmptySharedGroupNumber = 0;

    EmptySharedGroup* findEmptySharedGroup(const QString& emptyGroupId);
    const AbstractLayoutPanelTreeItem* findEmptyGroupAnchor(const muse::ID& anchorPartId) const;
    // After rows were moved: takes each empty group's position from where it now is in the panel
    void updateEmptySharedGroupPositions();
    // Before instruments leave their groups: keeps the groups they'd leave without instruments, as empty groups
    void keepEmptiedSharedGroups(const QStringList& leavingPartIds, const QString& destinationGroupId);

    LayoutPanelItemType::ItemType m_selectedItemsType = LayoutPanelItemType::ItemType::UNDEFINED;

    AbstractLayoutPanelTreeItem* m_rootItem = nullptr;
    muse::uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
    mu::notation::IMasterNotationPtr m_masterNotation = nullptr;
    mu::notation::INotationPtr m_notation = nullptr;
    std::shared_ptr<muse::async::Asyncable> m_partsNotifyReceiver = nullptr;

    using NotationKey = QString;

    bool m_layoutPanelVisible = true;
    bool m_scoreChanged = false;
    bool m_shouldUpdateSystemObjectLayers = false;

    bool m_dragInProgress = false;
    AbstractLayoutPanelTreeItem* m_dragSourceParentItem = nullptr;
    MoveParams m_activeDragMoveParams;

    muse::ID m_systemStaffToSelect;
};
}
