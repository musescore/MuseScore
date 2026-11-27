/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "layoutpaneltreemodel.h"

#include <algorithm>

#include "async/notifylist.h"

#include "translation.h"
#include "defer.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sharedpart.h"

#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationselection.h" // IWYU pragma: keep
#include "notation/inotationstyle.h" // IWYU pragma: keep
#include "notation/inotationundostack.h" // IWYU pragma: keep

#include "roottreeitem.h"
#include "parttreeitem.h"
#include "sharedparttreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"
#include "systemobjectslayertreeitem.h"
#include "instrumentscommands.h"

#include "uicomponents/qml/Muse/UiComponents/itemmultiselectionmodel.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::uicomponents;

LayoutPanelTreeModel::LayoutPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void LayoutPanelTreeModel::classBegin()
{
    init();
}

void LayoutPanelTreeModel::init()
{
    m_partsNotifyReceiver = std::make_shared<muse::async::Asyncable>();

    m_selectionModel = new ItemMultiSelectionModel(this);
    m_selectionModel->setAllowedModifiers(Qt::ShiftModifier);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        setItemsSelected(deselected.indexes(), false);
        setItemsSelected(selected.indexes(), true);

        updateRearrangementAvailability();
        updateRemovingAvailability();
        updateSelectedItemsType();
        updateIsAddingSystemMarkingsAvailable();
    });

    connect(this, &LayoutPanelTreeModel::rowsInserted, this, [this]() {
        updateRemovingAvailability();
    });

    onMasterNotationChanged();
    context()->masterNotationChanged().onNotify(this, [this]() {
        onMasterNotationChanged();
    });

    onNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    commandShortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        emit addInstrumentsKeyboardShortcutChanged();
    });
}

void LayoutPanelTreeModel::onMasterNotationChanged()
{
    m_masterNotation = context()->masterNotation();
}

void LayoutPanelTreeModel::onNotationChanged()
{
    m_partsNotifyReceiver->async_disconnectAll();

    if (m_isLoadingBlocked) {
        m_notationChangedWhileLoadingWasBlocked = true;
        return;
    }

    m_notation = context()->currentNotation();
    m_showsOtherInstruments = false;

    if (m_notation) {
        load();
        setupExcerptNameConnection();
    } else {
        clear();
    }

    m_notationChangedWhileLoadingWasBlocked = false;
}

bool LayoutPanelTreeModel::isMasterScore() const
{
    return m_notation && m_notation->isMaster();
}

LayoutPanelTreeModel::~LayoutPanelTreeModel()
{
    deleteItems();
}

bool LayoutPanelTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!m_isRemovingAvailable) {
        return false;
    }

    AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(parent);

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    setLoadingBlocked(true);
    beginRemoveRows(parent, row, row + count - 1);

    parentItem->removeChildren(row, count, true);

    endRemoveRows();
    setLoadingBlocked(false);

    emit isEmptyChanged();

    updateSystemObjectLayers();

    return true;
}

bool LayoutPanelTreeModel::shouldShowSystemObjectLayers() const
{
    // Only show system object staves in master score
    // TODO: extend system object staves logic to parts
    return isMasterScore();
}

void LayoutPanelTreeModel::setLoadingBlocked(bool blocked)
{
    m_isLoadingBlocked = blocked;

    if (!m_isLoadingBlocked && m_notationChangedWhileLoadingWasBlocked) {
        onNotationChanged();
    }
}

void LayoutPanelTreeModel::setupPartsConnections()
{
    async::NotifyList<const Part*> notationParts = m_notation->parts()->partList();

    notationParts.onChanged(m_partsNotifyReceiver.get(), [this]() {
        load();
    }, Mode::SetReplace);

    auto updateMasterPartItem = [this](const muse::ID& partId) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(partId, LayoutPanelItemType::PART));
        if (!partItem) {
            return;
        }

        partItem->init(m_masterNotation->parts()->part(partId));
        updateRemovingAvailability();
    };

    notationParts.onItemAdded(m_partsNotifyReceiver.get(), [updateMasterPartItem](const Part* part) {
        updateMasterPartItem(part->id());
    }, Mode::SetReplace);

    notationParts.onItemChanged(m_partsNotifyReceiver.get(), [updateMasterPartItem](const Part* part) {
        updateMasterPartItem(part->id());
    }, Mode::SetReplace);

    m_notation->parts()->systemObjectStavesChanged().onNotify(this, [this]() {
        m_shouldUpdateSystemObjectLayers = true;
        if (!m_isLoadingBlocked) {
            load();
        }
    }, Mode::SetReplace);

    m_notation->parts()->sharedPartsChanged().onNotify(this, [this]() {
        updateIsStaveSharingEnabled();
        updateAddStaffControls();
    }, Mode::SetReplace);
}

void LayoutPanelTreeModel::setupStavesConnections(const muse::ID& partId)
{
    async::NotifyList<const Staff*> notationStaves = m_notation->parts()->staffList(partId);

    notationStaves.onItemChanged(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF));
        if (!staffItem) {
            return;
        }

        staffItem->init(m_masterNotation->parts()->staff(staff->id()));
    }, Mode::SetReplace);

    notationStaves.onItemRemoved(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        auto staffItem = partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF);
        if (!staffItem) {
            return;
        }

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());
        int staffRow = staffItem->row();

        beginRemoveRows(partIndex, staffRow, staffRow);
        partItem->removeChildren(staffRow, 1, false);
        endRemoveRows();
    }, Mode::SetReplace);

    notationStaves.onItemAdded(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        if (partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF)) {
            return; // this staff item already exists in the part
        }

        const Staff* masterStaff = m_masterNotation->parts()->staff(staff->id());
        size_t staffIdx = muse::indexOf(masterStaff->part()->staves(), const_cast<Staff*>(masterStaff));
        if (staffIdx == muse::nidx) {
            return;
        }

        auto staffItem = buildMasterStaffItem(masterStaff, partItem);
        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());
        int dstRow = static_cast<int>(staffIdx);

        beginInsertRows(partIndex, dstRow, dstRow);
        partItem->insertChild(staffItem, dstRow);
        endInsertRows();
    }, Mode::SetReplace);
}

void LayoutPanelTreeModel::setupNotationConnections()
{
    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateSelectedRows();
    }, Mode::SetReplace);

    m_notation->undoStack()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (!m_layoutPanelVisible) {
            m_scoreChanged = true;
            return;
        }

        onScoreChanged(changes);
    }, Mode::SetReplace /*because this channel is from MasterScore*/);
}

void LayoutPanelTreeModel::setupExcerptNameConnection()
{
    const engraving::Excerpt* excerpt = m_notation->elements()->msScore()->excerpt();
    if (!excerpt) {
        return;
    }

    excerpt->nameChanged().onNotify(m_partsNotifyReceiver.get(), [this]() {
        emit scoreChanged();
    });
}

void LayoutPanelTreeModel::updateSelectedRows()
{
    if (!m_layoutPanelVisible || !m_notation) {
        return;
    }

    m_selectionModel->clear();

    const std::vector<EngravingItem*>& selectedElements = m_notation->interaction()->selection()->elements();
    if (selectedElements.empty()) {
        return;
    }

    for (AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->isSelectedInScore(selectedElements)) {
            m_selectionModel->select(createIndex(item->row(), 0, item), QItemSelectionModel::Select);
        }
    }
}

void LayoutPanelTreeModel::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    if (!m_rootItem) {
        return;
    }

    updateIsStaveSharingEnabled();
    updateIsHideEmptyStavesEnabled();

    m_rootItem->onScoreChanged(changes);

    m_scoreChanged = false;
}

void LayoutPanelTreeModel::clear()
{
    TRACEFUNC;

    beginResetModel();
    deleteItems();
    endResetModel();

    updateIsHideEmptyStavesEnabled();
    setHasHiddenOtherInstruments(false);

    emit scoreChanged();
    emit isEmptyChanged();
    emit isAddingAvailableChanged(false);
}

void LayoutPanelTreeModel::deleteItems()
{
    m_dragSourceParentItem = nullptr;
    setDropTarget(nullptr);

    if (m_dragInProgress) {
        m_dragInProgress = false;
        m_draggedIndex = QPersistentModelIndex();
        emit draggedIndexChanged();
    }

    m_selectionModel->clear();
    delete m_rootItem;
    m_rootItem = nullptr;
}

void LayoutPanelTreeModel::load()
{
    if (m_isLoadingBlocked) {
        return;
    }

    TRACEFUNC;

    beginResetModel();
    deleteItems();

    m_rootItem = new RootTreeItem(m_masterNotation, m_notation, this);

    async::NotifyList<const Part*> masterParts = m_masterNotation->parts()->partList();
    async::NotifyList<const Part*> excerptParts = m_notation->parts()->partList();
    // For master score we should use score's order, for others - custom
    if (!isMasterScore()) {
        sortParts(masterParts, excerptParts);
    }

    const bool showSystemObjectLayers = shouldShowSystemObjectLayers();
    const std::vector<Staff*>& systemObjectStaves = m_masterNotation->notation()->parts()->systemObjectStaves();

    SystemObjectGroupsByStaff systemObjects;
    if (showSystemObjectLayers) {
        systemObjects = collectSystemObjectGroups(systemObjectStaves);
    }

    const bool hidesOtherInstruments = !isMasterScore() && !m_showsOtherInstruments;
    bool hasHiddenOtherInstruments = false;

    QList<AbstractLayoutPanelTreeItem*> rows;

    for (const Part* part : masterParts) {
        if (!isMasterScore() && part->isSharedPart()) {
            continue;
        }

        if (hidesOtherInstruments && !m_notation->parts()->partExists(part->id())) {
            hasHiddenOtherInstruments = true;
            continue;
        }

        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (!staff->hasSystemObjectsBelowBottomStaff() && muse::contains(systemObjectStaves, staff)) {
                    rows << buildSystemObjectsLayerItem(staff, systemObjects[staff]);
                }
            }
        }

        rows << (part->isSharedPart() ? buildSharedPartItem(part) : buildMasterPartItem(part));

        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (staff->hasSystemObjectsBelowBottomStaff()) {
                    rows << buildSystemObjectsLayerItem(staff, systemObjects[staff]);
                }
            }
        }
    }

    m_rootItem->setChildren(rows);

    for (AbstractLayoutPanelTreeItem* item : rows) {
        initExpanded(item);
    }

    endResetModel();

    // The view shows expanded rows only when told
    for (const AbstractLayoutPanelTreeItem* item : rows) {
        if (item->isExpandable() && item->isExpanded()) {
            emit expandedChanged(index(item->row(), 0, QModelIndex()));
        }
    }

    m_shouldUpdateSystemObjectLayers = false;

    setupPartsConnections();
    setupNotationConnections();

    updateIsAddingSystemMarkingsAvailable();

    updateIsStaveSharingEnabled();
    updateIsHideEmptyStavesEnabled();
    setHasHiddenOtherInstruments(hasHiddenOtherInstruments);

    emit scoreChanged();
    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

void LayoutPanelTreeModel::showOtherInstruments()
{
    m_showsOtherInstruments = true;
    load();
}

void LayoutPanelTreeModel::toggleStaveSharing(bool on)
{
    m_notation->parts()->toggleStaveSharing(on);
    updateIsStaveSharingEnabled();
}

void LayoutPanelTreeModel::ungroupSharedGroup(const QString& sharedPartId)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts) {
        return;
    }

    parts->ungroupSharedPart(ID(sharedPartId));
}

void LayoutPanelTreeModel::toggleHideEmptyStaves(bool on)
{
    if (!m_notation || isHideEmptyStavesEnabled() == on) {
        return;
    }

    m_notation->undoStack()->prepareChanges(TranslatableString("undoableAction", "Hide empty staves"));
    m_notation->style()->setStyleValue(StyleId::hideEmptyStaves, on);
    m_notation->undoStack()->commitChanges();

    updateIsHideEmptyStavesEnabled();
}

void LayoutPanelTreeModel::sortParts(PartList& parts, const PartList& referenceParts)
{
    // First collect ids of referenceParts to use in sorting further

    std::vector<ID> referenceIdOrder;
    referenceIdOrder.reserve(referenceParts.size());

    for (const Part* part : referenceParts) {
        referenceIdOrder.push_back(part->id());
    }

    // Now we can define ordering function based on reference order
    auto comparator = [&referenceIdOrder] (const Part* partA, const Part* partB) {
        const ID& idA = partA->id();
        const ID& idB = partB->id();

        auto itA = std::find(referenceIdOrder.begin(), referenceIdOrder.end(), idA);
        auto itB = std::find(referenceIdOrder.begin(), referenceIdOrder.end(), idB);

        auto idxA = std::distance(referenceIdOrder.begin(), itA);
        auto idxB = std::distance(referenceIdOrder.begin(), itB);

        return idxA < idxB;
    };

    std::stable_sort(parts.begin(), parts.end(), comparator);
}

void LayoutPanelTreeModel::setLayoutPanelVisible(bool visible)
{
    if (m_layoutPanelVisible == visible) {
        return;
    }

    m_layoutPanelVisible = visible;

    if (visible) {
        updateSelectedRows();

        if (m_scoreChanged) {
            onScoreChanged({});
            m_shouldUpdateSystemObjectLayers = true;
            updateSystemObjectLayers();
        }
    }
}

void LayoutPanelTreeModel::selectRow(const QModelIndex& rowIndex)
{
    m_selectionModel->select(rowIndex);
}

void LayoutPanelTreeModel::clearSelection()
{
    m_selectionModel->clear();
}

void LayoutPanelTreeModel::addInstruments()
{
    commandDispatcher()->dispatch(INSTRUMENTS_SELECT_COMMAND);
}

void LayoutPanelTreeModel::addSystemMarkings()
{
    if (const Staff* staff = resolveNewSystemObjectStaff()) {
        m_systemStaffToSelect = staff->id();
        m_masterNotation->parts()->addSystemObjects({ staff->id() });
    }
}

void LayoutPanelTreeModel::moveSelectedRowsUp()
{
    if (!m_isMovingUpAvailable) {
        return;
    }

    const QModelIndexList selectedIndexList = sortedSelectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    const QModelIndex& first = selectedIndexList.first();
    moveRowRange(first.parent(), first.row(), selectedIndexList.last().row(), first.row() - 1, false /*isDrag*/);
}

void LayoutPanelTreeModel::moveSelectedRowsDown()
{
    if (!m_isMovingDownAvailable) {
        return;
    }

    const QModelIndexList selectedIndexList = sortedSelectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    const QModelIndex& first = selectedIndexList.first();
    const QModelIndex& last = selectedIndexList.last();
    moveRowRange(first.parent(), first.row(), last.row(), last.row() + 1, false /*isDrag*/);
}

void LayoutPanelTreeModel::removeSelectedRows()
{
    if (!m_isRemovingAvailable) {
        return;
    }

    const QModelIndexList selectedIndexList = sortedSelectedIndexes();
    if (selectedIndexList.empty()) {
        return;
    }

    const QModelIndex firstIndex = selectedIndexList.first();
    const int count = selectedIndexList.size();

    const bool needsConfirmation = std::any_of(selectedIndexList.cbegin(), selectedIndexList.cend(), [this](const QModelIndex& index) {
        return modelIndexToItem(index)->needsRemovalConfirmation();
    });

    if (!needsConfirmation) {
        removeRows(firstIndex.row(), count, firstIndex.parent());
        return;
    }

    askAboutRemovingInstruments(count).onResolve(this, [this, first = QPersistentModelIndex(firstIndex), count](bool remove) {
        if (remove && first.isValid()) {
            removeRows(first.row(), count, first.parent());
        }
    });
}

bool LayoutPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                    int destinationChild)
{
    setLoadingBlocked(true);
    DEFER {
        setLoadingBlocked(false);
    };

    AbstractLayoutPanelTreeItem* sourceParentItem = itemOrRoot(sourceParent);
    AbstractLayoutPanelTreeItem* destinationParentItem = itemOrRoot(destinationParent);
    if (!sourceParentItem || !destinationParentItem || count <= 0 || sourceRow == destinationChild) {
        return false;
    }

    const int sourceLastRow = sourceRow + count - 1;
    const int destinationRow = (sourceLastRow > destinationChild) ? destinationChild : destinationChild + 1;
    const int maxRow = sourceParentItem->childCount() - 1;
    if (sourceRow < 0 || destinationChild < 0 || sourceLastRow > maxRow || destinationRow > maxRow + 1) {
        return false;
    }

    // Staves stay in their part
    if (sourceParentItem != destinationParentItem && sourceParentItem != m_rootItem) {
        return false;
    }

    if (m_dragInProgress) {
        m_dragSourceParentItem = sourceParentItem;
    }

    beginMoveRows(sourceParent, sourceRow, sourceLastRow, destinationParent, destinationRow);
    sourceParentItem->moveChildren(sourceRow, count, destinationParentItem, destinationRow, !m_dragInProgress);
    endMoveRows();

    if (!m_dragInProgress) {
        updateSystemObjectLayers();
    }

    updateRearrangementAvailability();

    return true;
}

void LayoutPanelTreeModel::dragOver(const QModelIndex& targetIndex, LayoutPanelItemType::DropPosition position)
{
    AbstractLayoutPanelTreeItem* source = draggedItem();
    const AbstractLayoutPanelTreeItem* target = modelIndexToItem(targetIndex);
    if (!source || !target) {
        return;
    }

    // A row deeper than the dragged one stands for its ancestor at the dragged row's level
    while (target->parentItem() && target->parentItem() != source->parentItem()) {
        target = target->parentItem();
    }

    if (target->parentItem() != source->parentItem()) {
        setDropTarget(nullptr);
        return;
    }

    AbstractLayoutPanelTreeItem* dropTarget = target->dropTarget(source, position);
    setDropTarget(dropTarget);

    if (!dropTarget && target->canAcceptDrop(source, position)) {
        moveDraggedRow(target->row());
    }
}

void LayoutPanelTreeModel::moveDraggedRow(int targetRow)
{
    // m_draggedIndex follows the row
    const int row = m_draggedIndex.row();
    if (moveRowRange(m_draggedIndex.parent(), row, row, targetRow, true /*isDrag*/)) {
        emit draggedIndexChanged();
    }
}

void LayoutPanelTreeModel::setDropTarget(AbstractLayoutPanelTreeItem* target)
{
    if (m_dropTarget == target) {
        return;
    }

    m_dropTarget = target;
    emit dropTargetChanged();
}

void LayoutPanelTreeModel::startActiveDrag(const QModelIndex& draggedIndex)
{
    m_dragInProgress = true;
    m_draggedIndex = draggedIndex;
    emit draggedIndexChanged();

    if (AbstractLayoutPanelTreeItem* item = draggedItem()) {
        updateExpanded(item);
    }
}

void LayoutPanelTreeModel::endActiveDrag()
{
    if (!m_dragInProgress) {
        return;
    }

    AbstractLayoutPanelTreeItem* item = draggedItem();
    m_draggedIndex = QPersistentModelIndex();
    emit draggedIndexChanged();

    if (item) {
        updateExpanded(item);
    }

    AbstractLayoutPanelTreeItem* dropTarget = m_dropTarget;
    setDropTarget(nullptr);

    if (item && dropTarget) {
        m_dragSourceParentItem = nullptr;
        m_dragInProgress = false;

        // The live moves are dropped: the panel reloads from the score
        dropTarget->applyDrop(item);
        return;
    }

    setLoadingBlocked(true);

    if (m_dragSourceParentItem) {
        m_dragSourceParentItem->applyToScore();
    }

    m_dragSourceParentItem = nullptr;
    m_dragInProgress = false;

    setLoadingBlocked(false);

    updateSystemObjectLayers();
}

void LayoutPanelTreeModel::changeVisibilityOfSelectedRows(bool visible)
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndexes) {
        changeVisibility(index, visible);
    }
}

void LayoutPanelTreeModel::changeVisibility(const QModelIndex& index, bool visible)
{
    setLoadingBlocked(true);

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    item->setIsVisible(visible);

    setLoadingBlocked(false);
}

void LayoutPanelTreeModel::changeEnabledOfSelectedRows(bool enable)
{
    const QModelIndexList selectedIndices = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndices) {
        changeEnabled(index, enable);
    }
}

void LayoutPanelTreeModel::changeEnabled(const QModelIndex& index, bool enable)
{
    setLoadingBlocked(true);

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    item->setIsEnabled(enable);

    setLoadingBlocked(false);
}

void LayoutPanelTreeModel::setExpanded(const QModelIndex& index, bool expand)
{
    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    if (!item) {
        return;
    }

    const bool wasExpanded = item->isExpanded();
    applyExpanded(item, expand);
    if (item->isExpanded() == wasExpanded) {
        return;
    }

    // Expanding or collapsing doesn't move the row, so its control has the same index once the row is rebuilt
    const muse::ui::INavigationControl* control = navigationController()->activeControl();
    if (control) {
        const muse::ui::INavigationPanel* panel = control->panel();
        m_navigationToRestore = NavigationTarget { panel->section()->name().toStdString(), panel->name().toStdString(),
                                                   control->index() };
    }
}

void LayoutPanelTreeModel::setAllExpanded(bool expand)
{
    if (!m_rootItem) {
        return;
    }

    for (AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        applyExpanded(item, expand);
    }
}

void LayoutPanelTreeModel::restoreNavigation()
{
    if (!m_navigationToRestore) {
        return;
    }

    const NavigationTarget target = *m_navigationToRestore;
    m_navigationToRestore.reset();

    navigationController()->requestActivateByIndex(target.section, target.panel, target.index);
}

QModelIndex LayoutPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    const AbstractLayoutPanelTreeItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    AbstractLayoutPanelTreeItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex LayoutPanelTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    const AbstractLayoutPanelTreeItem* childItem = modelIndexToItem(child);
    AbstractLayoutPanelTreeItem* parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int LayoutPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    const AbstractLayoutPanelTreeItem* parentItem = m_rootItem;

    if (parent.isValid()) {
        parentItem = modelIndexToItem(parent);
    }

    return parentItem ? parentItem->childCount() : 0;
}

int LayoutPanelTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant LayoutPanelTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != ItemRole) {
        return QVariant();
    }

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    return item ? QVariant::fromValue<AbstractLayoutPanelTreeItem*>(item) : QVariant();
}

QHash<int, QByteArray> LayoutPanelTreeModel::roleNames() const
{
    return { { ItemRole, "item" } };
}

void LayoutPanelTreeModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void LayoutPanelTreeModel::setIsMovingDownAvailable(bool isMovingDownAvailable)
{
    if (m_isMovingDownAvailable == isMovingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMovingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

bool LayoutPanelTreeModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool LayoutPanelTreeModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool LayoutPanelTreeModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

bool LayoutPanelTreeModel::isAddingAvailable() const
{
    return m_notation != nullptr;
}

bool LayoutPanelTreeModel::isAddingSystemMarkingsAvailable() const
{
    return m_isAddingSystemMarkingsAvailable;
}

bool LayoutPanelTreeModel::isEmpty() const
{
    return m_rootItem ? m_rootItem->isEmpty() : true;
}

int LayoutPanelTreeModel::selectedItemsType() const
{
    return static_cast<int>(m_selectedItemsType);
}

bool LayoutPanelTreeModel::isStaveSharingEnabled() const
{
    return m_isStaveSharingEnabled;
}

bool LayoutPanelTreeModel::isHideEmptyStavesEnabled() const
{
    return m_isHideEmptyStavesEnabled;
}

QString LayoutPanelTreeModel::scoreName() const
{
    return m_notation ? m_notation->name() : QString();
}

bool LayoutPanelTreeModel::hasHiddenOtherInstruments() const
{
    return m_hasHiddenOtherInstruments;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::draggedItem() const
{
    return modelIndexToItem(m_draggedIndex);
}

QModelIndex LayoutPanelTreeModel::draggedIndex() const
{
    return m_draggedIndex;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::dropTarget() const
{
    return m_dropTarget;
}

QString LayoutPanelTreeModel::addInstrumentsKeyboardShortcut() const
{
    muse::shortcuts::Shortcut shortcut = commandShortcutsRegister()->shortcut(INSTRUMENTS_SELECT_COMMAND);
    if (shortcut.sequences.empty()) {
        return {};
    }

    return muse::shortcuts::sequencesToNativeText({ shortcut.sequences[0] });
}

void LayoutPanelTreeModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void LayoutPanelTreeModel::setHasHiddenOtherInstruments(bool hasHidden)
{
    if (m_hasHiddenOtherInstruments == hasHidden) {
        return;
    }

    m_hasHiddenOtherInstruments = hasHidden;
    emit hasHiddenOtherInstrumentsChanged();
}

void LayoutPanelTreeModel::updateRearrangementAvailability()
{
    const QModelIndexList selectedIndexList = sortedSelectedIndexes();

    const auto hasFirstItemType = [this, &selectedIndexList](const QModelIndex& index) {
        return modelIndexToItem(index)->type() == modelIndexToItem(selectedIndexList.first())->type();
    };

    if (selectedIndexList.isEmpty() || !std::all_of(selectedIndexList.cbegin(), selectedIndexList.cend(), hasFirstItemType)) {
        setIsMovingUpAvailable(false);
        setIsMovingDownAvailable(false);
        return;
    }

    const int firstRow = selectedIndexList.first().row();
    const int lastRow = selectedIndexList.last().row();
    const AbstractLayoutPanelTreeItem* parentItem = itemOrRoot(selectedIndexList.first().parent());

    setIsMovingUpAvailable(parentItem->resolveMove(firstRow, lastRow, firstRow - 1, false /*isDrag*/).has_value());
    setIsMovingDownAvailable(parentItem->resolveMove(firstRow, lastRow, lastRow + 1, false /*isDrag*/).has_value());
}

void LayoutPanelTreeModel::updateRemovingAvailability()
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    const QModelIndex parent = selectedIndexes.empty() ? QModelIndex() : selectedIndexes.first().parent();

    //! NOTE: all selected items must have the same parent
    const auto isRemovable = [this, &parent](const QModelIndex& index) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        return item && item->isRemovable() && index.parent() == parent;
    };

    bool isRemovingAvailable = !selectedIndexes.empty()
                               && std::all_of(selectedIndexes.cbegin(), selectedIndexes.cend(), isRemovable);

    //! NOTE: the user is allowed to remove all children only for the root item
    if (isRemovingAvailable && parent.isValid()) {
        const QList<AbstractLayoutPanelTreeItem*>& children = modelIndexToItem(parent)->childItems();
        const qsizetype removableChildCount = std::count_if(children.cbegin(), children.cend(),
                                                            [](const AbstractLayoutPanelTreeItem* item) {
            return item->isRemovable();
        });

        isRemovingAvailable = selectedIndexes.size() < removableChildCount;
    }

    setIsRemovingAvailable(isRemovingAvailable);
}

void LayoutPanelTreeModel::updateSelectedItemsType()
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    LayoutPanelItemType::ItemType selectedItemsType = LayoutPanelItemType::ItemType::UNDEFINED;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        if (item) {
            selectedItemsType = item->type();
            break;
        }
    }

    if (m_selectedItemsType != selectedItemsType) {
        m_selectedItemsType = selectedItemsType;
        emit selectedItemsTypeChanged(static_cast<int>(m_selectedItemsType));
    }
}

void LayoutPanelTreeModel::updateIsAddingSystemMarkingsAvailable()
{
    bool addingSysMarkAvailable = false;

    if (isAddingAvailable() && isMasterScore()) {
        const QList<AbstractLayoutPanelTreeItem*>& children = m_rootItem->childItems();
        const qsizetype systemLayerCount = std::count_if(children.cbegin(), children.cend(), [](const AbstractLayoutPanelTreeItem* item) {
            return item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER;
        });

        addingSysMarkAvailable = systemLayerCount < 0.5 * m_rootItem->childCount();
    }

    if (addingSysMarkAvailable != m_isAddingSystemMarkingsAvailable) {
        m_isAddingSystemMarkingsAvailable = addingSysMarkAvailable;
        emit isAddingSystemMarkingsAvailableChanged(m_isAddingSystemMarkingsAvailable);
    }
}

void LayoutPanelTreeModel::updateIsStaveSharingEnabled()
{
    bool enabled = m_notation->style()->styleValue(StyleId::enableStaveSharing).toBool();
    if (enabled == m_isStaveSharingEnabled) {
        return;
    }

    m_isStaveSharingEnabled = enabled;
    emit isStaveSharingEnabledChanged(enabled);
}

void LayoutPanelTreeModel::updateIsHideEmptyStavesEnabled()
{
    bool enabled = m_notation && m_notation->style()->styleValue(StyleId::hideEmptyStaves).toBool();
    if (enabled == m_isHideEmptyStavesEnabled) {
        return;
    }

    m_isHideEmptyStavesEnabled = enabled;
    emit isHideEmptyStavesEnabledChanged(enabled);
}

void LayoutPanelTreeModel::setItemsSelected(const QModelIndexList& indexes, bool selected)
{
    for (const QModelIndex& index : indexes) {
        if (AbstractLayoutPanelTreeItem* item = modelIndexToItem(index)) {
            if (!item->isSelectable()) {
                continue;
            }
            item->setIsSelected(selected);
        }
    }
}

void LayoutPanelTreeModel::initExpanded(AbstractLayoutPanelTreeItem* item)
{
    updateExpanded(item);

    connect(item, &AbstractLayoutPanelTreeItem::isExpandableChanged, this, [this, item]() {
        updateExpanded(item);
    });

    connect(item, &AbstractLayoutPanelTreeItem::isExpandedChanged, this, [this, item]() {
        emit expandedChanged(index(item->row(), 0, QModelIndex()));
    });
}

void LayoutPanelTreeModel::applyExpanded(AbstractLayoutPanelTreeItem* item, bool expand)
{
    if (!item->isExpandable()) {
        return;
    }

    if (expand) {
        m_expandedItems.insert(item->id());
    } else {
        m_expandedItems.erase(item->id());
    }

    updateExpanded(item);
}

void LayoutPanelTreeModel::updateExpanded(AbstractLayoutPanelTreeItem* item)
{
    if (item == draggedItem()) {
        item->setIsExpanded(false);
        return;
    }

    // E.g. a group that isn't combined can't collapse
    item->setIsExpanded(!item->isExpandable() || muse::contains(m_expandedItems, item->id()));
}

QModelIndexList LayoutPanelTreeModel::sortedSelectedIndexes() const
{
    QModelIndexList indexes = m_selectionModel->selectedIndexes();
    std::stable_sort(indexes.begin(), indexes.end(), [](const QModelIndex& f, const QModelIndex& s) {
        return f.row() < s.row();
    });

    return indexes;
}

bool LayoutPanelTreeModel::moveRowRange(const QModelIndex& parent, int firstRow, int lastRow, int targetRow, bool isDrag)
{
    const std::optional<AbstractLayoutPanelTreeItem::RowMove> move = itemOrRoot(parent)->resolveMove(firstRow, lastRow, targetRow,
                                                                                                     isDrag);
    return move && moveRows(parent, move->sourceRow, move->count, parent, move->destinationRow);
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::itemOrRoot(const QModelIndex& index) const
{
    return index.isValid() ? modelIndexToItem(index) : m_rootItem;
}

muse::async::Promise<bool> LayoutPanelTreeModel::askAboutRemovingInstruments(int count)
{
    return muse::async::make_promise<bool>([this, count](auto resolve) {
        interactive()->warning(
            muse::trc("layoutpanel", "Are you sure you want to delete the selected instrument(s)?", nullptr, count),
            muse::trc("layoutpanel", "This will remove the instrument(s) from the full score and all part scores.", nullptr, count),
            { IInteractive::Button::No, IInteractive::Button::Yes }
            )
        .onResolve(this, [resolve](const IInteractive::Result& result) {
            (void)resolve(result.standardButton() == IInteractive::Button::Yes);
        });

        return muse::async::Promise<bool>::dummy_result();
    }, muse::async::PromiseType::AsyncByBody);
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildSharedPartItem(const Part* part)
{
    auto sharedPartItem = new SharedPartTreeItem(m_masterNotation, m_notation, m_rootItem);
    sharedPartItem->init(part);

    return sharedPartItem;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildMasterPartItem(const Part* masterPart)
{
    TRACEFUNC;

    auto partItem = new PartTreeItem(m_masterNotation, m_notation, m_rootItem);
    partItem->init(masterPart);

    for (const Staff* staff : m_masterNotation->parts()->staffList(partItem->id())) {
        auto staffItem = buildMasterStaffItem(staff, partItem);
        partItem->appendChild(staffItem);
    }

    // Instruments in a shared-staves group can't add staves
    if (!masterPart->sharedPart() && partItem->existsInScore()) {
        partItem->appendChild(buildAddStaffControlItem(partItem->id(), partItem));
    }

    setupStavesConnections(partItem->id());

    return partItem;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildMasterStaffItem(const Staff* masterStaff, QObject* parent)
{
    auto result = new StaffTreeItem(m_masterNotation, m_notation, parent);
    result->init(masterStaff);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildSystemObjectsLayerItem(const mu::notation::Staff* masterStaff,
                                                                               const SystemObjectGroups& systemObjects)
{
    auto result = new SystemObjectsLayerTreeItem(m_masterNotation, m_notation, m_rootItem);
    result->init(masterStaff, systemObjects);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildAddStaffControlItem(const muse::ID& partId, QObject* parent)
{
    auto result = new StaffControlTreeItem(m_masterNotation, m_notation, parent);
    result->init(partId);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractLayoutPanelTreeItem*>(index.internalPointer());
}

void LayoutPanelTreeModel::updateSystemObjectLayers()
{
    if (!m_masterNotation || !m_rootItem || !m_shouldUpdateSystemObjectLayers) {
        return;
    }

    if (!shouldShowSystemObjectLayers()) {
        return;
    }

    TRACEFUNC;

    m_shouldUpdateSystemObjectLayers = false;

    // Create copy, because we're going to modify them
    const INotationPartsPtr notationParts = m_masterNotation->notation()->parts();
    std::vector<Staff*> newSystemObjectStaves = notationParts->systemObjectStaves();
    QList<AbstractLayoutPanelTreeItem*> children = m_rootItem->childItems();

    // Remove old system object layers
    std::vector<const PartTreeItem*> partItems;
    std::vector<SystemObjectsLayerTreeItem*> existingSystemObjectLayers;

    for (AbstractLayoutPanelTreeItem* item : children) {
        if (item->type() != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            if (item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART) {
                partItems.push_back(static_cast<const PartTreeItem*>(item));
            }
            continue;
        }

        auto layerItem = static_cast<SystemObjectsLayerTreeItem*>(item);
        if (muse::remove(newSystemObjectStaves, const_cast<Staff*>(layerItem->staff()))) {
            existingSystemObjectLayers.push_back(layerItem);
            continue;
        }

        int row = layerItem->row();
        beginRemoveRows(QModelIndex(), row, row);
        m_rootItem->removeChildren(row, 1, false);
        endRemoveRows();
    }

    SystemObjectGroupsByStaff systemObjects = collectSystemObjectGroups(notationParts->systemObjectStaves());

    // Update position of existing layers if changed
    for (SystemObjectsLayerTreeItem* layerItem : existingSystemObjectLayers) {
        const PartTreeItem* partItem = findPartItemByStaff(layerItem->staff());
        IF_ASSERT_FAILED(partItem) {
            continue;
        }

        const int partRow = partItem->row();
        const int layerRow = layerItem->row();
        const int correctLayerRow = layerItem->staff()->hasSystemObjectsBelowBottomStaff() ? partRow + 1 : partRow - 1;

        if (layerRow != correctLayerRow) {
            beginMoveRows(QModelIndex(), layerRow, layerRow, QModelIndex(), partRow);
            m_rootItem->moveChildren(layerRow, 1, m_rootItem, partRow, false /*updateNotation*/);
            endMoveRows();
        }

        layerItem->setSystemObjects(systemObjects[layerItem->staff()]);
    }

    // Create new system object layers
    for (const Staff* staff : newSystemObjectStaves) {
        for (const PartTreeItem* partItem : partItems) {
            if (staff->part() != partItem->part()) {
                continue;
            }

            AbstractLayoutPanelTreeItem* newItem = buildSystemObjectsLayerItem(staff, systemObjects[staff]);
            int row = staff->hasSystemObjectsBelowBottomStaff() ? partItem->row() + 1 : partItem->row();

            beginInsertRows(QModelIndex(), row, row);
            m_rootItem->insertChild(newItem, row);
            endInsertRows();

            if (row != 0 && m_systemStaffToSelect == staff->id()) {
                m_selectionModel->select(createIndex(row, 0, newItem));
                m_systemStaffToSelect = ID();
            }

            break;
        }
    }

    updateIsAddingSystemMarkingsAvailable();
}

void LayoutPanelTreeModel::updateAddStaffControls()
{
    if (!m_rootItem) {
        return;
    }

    for (int row = 0; row < m_rootItem->childCount(); ++row) {
        PartTreeItem* partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtRow(row));
        if (!partItem || partItem->type() != LayoutPanelItemType::PART) {
            continue;
        }

        const Part* masterPart = m_masterNotation->parts()->part(partItem->id());
        const int lastRow = partItem->childCount() - 1;
        const bool hasControl = partItem->childType(lastRow) == LayoutPanelItemType::CONTROL_ADD_STAFF;
        const bool needsControl = masterPart && !masterPart->sharedPart() && partItem->existsInScore();
        const QModelIndex partIndex = index(row, 0, QModelIndex());

        if (needsControl && !hasControl) {
            beginInsertRows(partIndex, lastRow + 1, lastRow + 1);
            partItem->appendChild(buildAddStaffControlItem(partItem->id(), partItem));
            endInsertRows();
        } else if (!needsControl && hasControl) {
            AbstractLayoutPanelTreeItem* control = partItem->childAtRow(lastRow);

            beginRemoveRows(partIndex, lastRow, lastRow);
            partItem->removeChildren(lastRow, 1, false);
            endRemoveRows();

            control->deleteLater();
        }
    }
}

const PartTreeItem* LayoutPanelTreeModel::findPartItemByStaff(const Staff* staff) const
{
    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() != LayoutPanelItemType::PART && item->type() != LayoutPanelItemType::SHARED_PART) {
            continue;
        }

        const PartTreeItem* partItem = static_cast<const PartTreeItem*>(item);
        if (muse::contains(partItem->part()->staves(), const_cast<Staff*>(staff))) {
            return partItem;
        }
    }

    return nullptr;
}

const Staff* LayoutPanelTreeModel::resolveNewSystemObjectStaff() const
{
    const std::vector<Staff*>& systemObjectStaves = m_notation->parts()->systemObjectStaves();

    const auto resolveStaff = [&](const AbstractLayoutPanelTreeItem* item) -> const Staff* {
        if (item->type() != LayoutPanelItemType::PART) {
            return nullptr;
        }

        const Part* part = static_cast<const PartTreeItem*>(item)->part();
        for (Staff* staff : part->staves()) {
            if (muse::contains(systemObjectStaves, staff)) {
                return nullptr;
            }
        }

        return part->staff(0);
    };

    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        if (const Staff* staff = resolveStaff(item)) {
            return staff;
        }
    }

    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (const Staff* staff = resolveStaff(item)) {
            return staff;
        }
    }

    return nullptr;
}
