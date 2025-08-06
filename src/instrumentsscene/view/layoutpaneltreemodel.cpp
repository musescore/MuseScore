/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "translation.h"
#include "roottreeitem.h"
#include "parttreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"
#include "systemobjectslayertreeitem.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

#include "defer.h"
#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::uicomponents;

static const muse::actions::ActionCode ADD_INSTRUMENTS_ACTIONCODE("instruments");

namespace mu::instrumentsscene {
static QString notationToKey(const INotationPtr notation)
{
    std::stringstream stream;
    stream << notation.get();

    return QString::fromStdString(stream.str());
}
}

LayoutPanelTreeModel::LayoutPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
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
    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        onMasterNotationChanged();
    });

    onNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        emit addInstrumentsKeyboardShortcutChanged();
    });
}

void LayoutPanelTreeModel::onMasterNotationChanged()
{
    m_masterNotation = context()->currentMasterNotation();
    initPartOrders();
}

void LayoutPanelTreeModel::onNotationChanged()
{
    m_partsNotifyReceiver->disconnectAll();

    if (m_isLoadingBlocked) {
        m_notationChangedWhileLoadingWasBlocked = true;
        return;
    }

    onBeforeChangeNotation();
    m_notation = context()->currentNotation();

    if (m_notation) {
        load();
    } else {
        clear();
    }

    m_notationChangedWhileLoadingWasBlocked = false;
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

    if (parentItem == m_rootItem && needWarnOnRemoveRows(row, count)) {
        // When removing instruments, the user needs to be warned in some cases
        if (!warnAboutRemovingInstrumentsIfNecessary(count)) {
            return false;
        }
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
    return m_notation && m_notation->isMaster();
}

void LayoutPanelTreeModel::initPartOrders()
{
    m_sortedPartIdList.clear();

    if (!m_masterNotation) {
        return;
    }

    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        NotationKey key = notationToKey(excerpt->notation());

        for (const Part* part : excerpt->notation()->parts()->partList()) {
            m_sortedPartIdList[key] << part->id();
        }
    }
}

void LayoutPanelTreeModel::onBeforeChangeNotation()
{
    if (!m_notation || !m_rootItem) {
        return;
    }

    QList<muse::ID> partIdList;

    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() == LayoutPanelItemType::ItemType::PART) {
            partIdList << item->id();
        }
    }

    m_sortedPartIdList[notationToKey(m_notation)] = partIdList;
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
    });

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
    });

    notationParts.onItemChanged(m_partsNotifyReceiver.get(), [updateMasterPartItem](const Part* part) {
        updateMasterPartItem(part->id());
    });

    m_notation->parts()->systemObjectStavesChanged().onNotify(this, [this]() {
        m_shouldUpdateSystemObjectLayers = true;
        if (!m_isLoadingBlocked) {
            updateSystemObjectLayers();
        }
    });
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
    });

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
    });

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
    });
}

void LayoutPanelTreeModel::setupNotationConnections()
{
    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateSelectedRows();
    });

    m_notation->undoStack()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (!m_layoutPanelVisible) {
            m_scoreChanged = true;
            return;
        }

        onScoreChanged(changes);
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

    std::vector<muse::ID> selectedPartIds;
    for (const EngravingItem* element : selectedElements) {
        if (!element->part()) {
            continue;
        }

        selectedPartIds.push_back(element->part()->id());
    }

    for (const muse::ID& selectedPartId : selectedPartIds) {
        AbstractLayoutPanelTreeItem* item = m_rootItem->childAtId(selectedPartId, LayoutPanelItemType::PART);

        if (item) {
            m_selectionModel->select(createIndex(item->row(), 0, item), QItemSelectionModel::Select);
        }
    }
}

void LayoutPanelTreeModel::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    if (!m_rootItem) {
        return;
    }

    for (AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        item->onScoreChanged(changes);
    }

    m_scoreChanged = false;
}

void LayoutPanelTreeModel::clear()
{
    TRACEFUNC;

    beginResetModel();
    deleteItems();
    endResetModel();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(false);
}

void LayoutPanelTreeModel::deleteItems()
{
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
    sortParts(masterParts);

    const bool showSystemObjectLayers = shouldShowSystemObjectLayers();
    const std::vector<Staff*>& systemObjectStaves = m_masterNotation->notation()->parts()->systemObjectStaves();

    SystemObjectGroupsByStaff systemObjects;
    if (showSystemObjectLayers) {
        systemObjects = collectSystemObjectGroups(systemObjectStaves);
    }

    for (const Part* part : masterParts) {
        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (!staff->hasSystemObjectsBelowBottomStaff() && muse::contains(systemObjectStaves, staff)) {
                    m_rootItem->appendChild(buildSystemObjectsLayerItem(staff, systemObjects[staff]));
                }
            }
        }

        m_rootItem->appendChild(buildMasterPartItem(part));

        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (staff->hasSystemObjectsBelowBottomStaff()) {
                    m_rootItem->appendChild(buildSystemObjectsLayerItem(staff, systemObjects[staff]));
                }
            }
        }
    }

    endResetModel();

    setupPartsConnections();
    setupNotationConnections();

    updateIsAddingSystemMarkingsAvailable();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

void LayoutPanelTreeModel::sortParts(notation::PartList& parts)
{
    NotationKey key = notationToKey(m_notation);

    if (!m_sortedPartIdList.contains(key)) {
        return;
    }

    const QList<muse::ID>& sortedPartIdList = m_sortedPartIdList[key];

    std::sort(parts.begin(), parts.end(), [&sortedPartIdList](const Part* part1, const Part* part2) {
        int index1 = sortedPartIdList.indexOf(part1->id());
        int index2 = sortedPartIdList.indexOf(part2->id());

        if (index1 < 0) {
            index1 = std::numeric_limits<int>::max();
        }

        if (index2 < 0) {
            index2 = std::numeric_limits<int>::max();
        }

        return index1 < index2;
    });
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
            onScoreChanged();
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
    dispatcher()->dispatch(ADD_INSTRUMENTS_ACTIONCODE);
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

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    const QModelIndex& sourceRowFirst = selectedIndexList.first();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowFirst.row() - 1);
}

void LayoutPanelTreeModel::moveSelectedRowsDown()
{
    if (!m_isMovingDownAvailable) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](const QModelIndex& f, const QModelIndex& s) -> bool {
        return f.row() < s.row();
    });

    const QModelIndex& sourceRowFirst = selectedIndexList.first();
    const QModelIndex& sourceRowLast = selectedIndexList.last();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowLast.row() + 1);
}

void LayoutPanelTreeModel::removeSelectedRows()
{
    if (!m_isRemovingAvailable) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.empty()) {
        return;
    }

    QModelIndex firstIndex = *std::min_element(selectedIndexList.cbegin(), selectedIndexList.cend(),
                                               [](const QModelIndex& f, const QModelIndex& s) {
        return f.row() < s.row();
    });

    removeRows(firstIndex.row(), selectedIndexList.size(), firstIndex.parent());
}

bool LayoutPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                    int destinationChild)
{
    setLoadingBlocked(true);
    DEFER {
        setLoadingBlocked(false);
    };

    AbstractLayoutPanelTreeItem* sourceParentItem = modelIndexToItem(sourceParent);
    AbstractLayoutPanelTreeItem* destinationParentItem = modelIndexToItem(destinationParent);

    if (!sourceParentItem) {
        sourceParentItem = m_rootItem;
    }

    if (!destinationParentItem) {
        destinationParentItem = m_rootItem;
    }

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;

    if (sourceRow < 0 || count <= 0 || destinationChild < 0) {
        return false;
    }

    if (sourceRow == destinationChild) {
        return false;
    }

    int destinationRow = (sourceLastRow > destinationChild) ? destinationChild : destinationChild + 1;

    if (sourceParentItem && destinationParentItem) {
        int maxRow = sourceParentItem->childCount() - 1;

        if (sourceLastRow > maxRow || destinationRow > maxRow + 1) {
            return false;
        }
    }

    if (!sourceParentItem || !destinationParentItem) {
        return false;
    }

    if (sourceFirstRow >= sourceParentItem->childCount()) {
        return false;
    }

    if (m_dragInProgress) {
        MoveParams params = sourceParentItem->buildMoveParams(sourceRow, count, destinationParentItem, destinationRow);
        if (!params.isValid()) {
            return false;
        }

        m_dragSourceParentItem = sourceParentItem;
        m_activeDragMoveParams = params;
    }

    beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);
    sourceParentItem->moveChildren(sourceFirstRow, count, destinationParentItem, destinationRow, !m_dragInProgress);
    endMoveRows();

    updateSystemObjectLayers();
    updateRearrangementAvailability();

    return true;
}

void LayoutPanelTreeModel::startActiveDrag()
{
    m_dragInProgress = true;
}

void LayoutPanelTreeModel::endActiveDrag()
{
    setLoadingBlocked(true);

    if (m_dragSourceParentItem) {
        m_dragSourceParentItem->moveChildrenOnScore(m_activeDragMoveParams);
    }

    m_dragSourceParentItem = nullptr;
    m_activeDragMoveParams = MoveParams();
    m_dragInProgress = false;

    setLoadingBlocked(false);

    updateSystemObjectLayers();
    initPartOrders();

    emit layoutChanged();
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

QItemSelectionModel* LayoutPanelTreeModel::selectionModel() const
{
    return m_selectionModel;
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
    if (!index.isValid() && role != ItemRole) {
        return QVariant();
    }

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    return item ? QVariant::fromValue(qobject_cast<QObject*>(item)) : QVariant();
}

QHash<int, QByteArray> LayoutPanelTreeModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
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

QString LayoutPanelTreeModel::addInstrumentsKeyboardShortcut() const
{
    const muse::shortcuts::Shortcut& shortcut = shortcutsRegister()->shortcut(ADD_INSTRUMENTS_ACTIONCODE);

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

void LayoutPanelTreeModel::updateRearrangementAvailability()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        updateMovingUpAvailability(false);
        updateMovingDownAvailability(false);
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](const QModelIndex& f, const QModelIndex& s) -> bool {
        return f.row() < s.row();
    });

    bool isRearrangementAvailable = true;

    QMutableListIterator<QModelIndex> it(selectedIndexList);
    LayoutPanelItemType::ItemType prevItemType = LayoutPanelItemType::UNDEFINED;

    while (it.hasNext() && selectedIndexList.count() > 1) {
        int nextRow = it.next().row();
        int previousRow = it.peekPrevious().row();

        isRearrangementAvailable = (nextRow - previousRow <= 1);

        if (isRearrangementAvailable) {
            const AbstractLayoutPanelTreeItem* item = modelIndexToItem(it.value());

            if (prevItemType != LayoutPanelItemType::UNDEFINED) {
                isRearrangementAvailable = item->type() == prevItemType;
            }

            prevItemType = item->type();
        }

        if (!isRearrangementAvailable) {
            updateMovingUpAvailability(isRearrangementAvailable);
            updateMovingDownAvailability(isRearrangementAvailable);
            return;
        }
    }

    updateMovingUpAvailability(isRearrangementAvailable, selectedIndexList.first());
    updateMovingDownAvailability(isRearrangementAvailable, selectedIndexList.last());
}

void LayoutPanelTreeModel::updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex)
{
    const AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(firstSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    // can't go above the topmost system objects layer
    int minAllowedRow = 0;
    if (parentItem->childType(0) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        minAllowedRow = 1;
    }

    bool isRowInBoundaries = firstSelectedRowIndex.isValid() && firstSelectedRowIndex.row() > minAllowedRow;

    setIsMovingUpAvailable(isSelectionMovable && isRowInBoundaries);
}

void LayoutPanelTreeModel::updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex)
{
    const AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(lastSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    // exclude the control item
    bool hasControlItem = parentItem->type() != LayoutPanelItemType::ROOT;
    const AbstractLayoutPanelTreeItem* curItem = modelIndexToItem(lastSelectedRowIndex);
    bool lastSelectedIsSystemObjectLayer = curItem && curItem->type() == LayoutPanelItemType::ItemType::SYSTEM_OBJECTS_LAYER;

    if (lastSelectedRowIndex.row() == 0 && lastSelectedIsSystemObjectLayer) {
        // Selecting/moving the top system object layer not allowed
        setIsMovingDownAvailable(false);
        return;
    }

    int lastItemRowIndex = parentItem->childCount() - 1 - (hasControlItem ? 1 : 0);

    bool isRowInBoundaries = lastSelectedRowIndex.isValid() && lastSelectedRowIndex.row() < lastItemRowIndex;

    setIsMovingDownAvailable(isSelectionMovable && isRowInBoundaries);
}

void LayoutPanelTreeModel::updateRemovingAvailability()
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    bool isRemovingAvailable = !selectedIndexes.empty();
    const AbstractLayoutPanelTreeItem* parent = nullptr;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        isRemovingAvailable = item && item->isRemovable();

        //! NOTE: all selected items must have the same parent
        if (isRemovingAvailable && parent) {
            if (parent != item->parentItem()) {
                isRemovingAvailable = false;
            }
        }

        if (!isRemovingAvailable) {
            break;
        }

        parent = item->parentItem();
    }

    //! NOTE: the user is allowed to remove all children only for the root item
    if (isRemovingAvailable && parent != m_rootItem) {
        int removableChildCount = 0;

        for (const AbstractLayoutPanelTreeItem* item : parent->childItems()) {
            if (item->isRemovable()) {
                removableChildCount++;
            }
        }

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

    if (!isAddingAvailable() || !m_notation->isMaster()) {
        addingSysMarkAvailable = false;
    } else {
        int systemLayerCount = 0;
        for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
            if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                ++systemLayerCount;
            }
        }

        addingSysMarkAvailable = systemLayerCount < 0.5 * m_rootItem->childCount();
    }

    if (addingSysMarkAvailable != m_isAddingSystemMarkingsAvailable) {
        m_isAddingSystemMarkingsAvailable = addingSysMarkAvailable;
        emit isAddingSystemMarkingsAvailableChanged(m_isAddingSystemMarkingsAvailable);
    }
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

bool LayoutPanelTreeModel::needWarnOnRemoveRows(int row, int count)
{
    for (int i = row + count - 1; i >= row; --i) {
        if (m_rootItem->childType(i) != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            return true;
        }
    }

    return false;
}

bool LayoutPanelTreeModel::warnAboutRemovingInstrumentsIfNecessary(int count)
{
    // Only warn if excerpts are existent
    if (m_masterNotation->excerpts().empty()) {
        return true;
    }

    return interactive()->warningSync(
        muse::trc("layoutpanel", "Are you sure you want to delete the selected instrument(s)?", nullptr, count),
        muse::trc("layoutpanel", "This will remove the instrument(s) from the full score and all part scores.", nullptr, count),
        { IInteractive::Button::No, IInteractive::Button::Yes }
        )
           .standardButton() == IInteractive::Button::Yes;
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

    auto addStaffControlItem = buildAddStaffControlItem(partItem->id(), partItem);
    partItem->appendChild(addStaffControlItem);

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
            if (item->type() == LayoutPanelItemType::PART) {
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

const PartTreeItem* LayoutPanelTreeModel::findPartItemByStaff(const Staff* staff) const
{
    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() != LayoutPanelItemType::PART) {
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
