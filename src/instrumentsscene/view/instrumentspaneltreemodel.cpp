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
#include "instrumentspaneltreemodel.h"

#include <algorithm>

#include "translation.h"
#include "roottreeitem.h"
#include "parttreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"
#include "log.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::uicomponents;
using ItemType = InstrumentsTreeItemType::ItemType;

InstrumentsPanelTreeModel::InstrumentsPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        IMasterNotationPtr masterNotation = context()->currentMasterNotation();

        m_masterNotationParts = nullptr;

        if (masterNotation) {
            m_masterNotationParts = masterNotation->parts();
        }
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = context()->currentNotation();
        m_notationParts = notation ? notation->parts() : nullptr;

        if (notation) {
            load();
        } else {
            clear();
        }
    });

    m_selectionModel = new ItemMultiSelectionModel(this);
    m_selectionModel->setAllowedModifiers(Qt::ShiftModifier);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, [this](const QItemSelection&, const QItemSelection&) {
        updateRearrangementAvailability();
        updateRemovingAvailability();

        emit selectionChanged();
    });

    dispatcher()->reg(this, "instruments", this, &InstrumentsPanelTreeModel::addInstruments);
}

InstrumentsPanelTreeModel::~InstrumentsPanelTreeModel()
{
    deleteItems();
}

bool InstrumentsPanelTreeModel::canReceiveAction(const actions::ActionCode&) const
{
    return m_masterNotationParts != nullptr;
}

void InstrumentsPanelTreeModel::clear()
{
    beginResetModel();
    m_selectionModel->clear();
    deleteItems();
    endResetModel();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(false);
}

void InstrumentsPanelTreeModel::deleteItems()
{
    delete m_rootItem;
    m_rootItem = nullptr;
}

void InstrumentsPanelTreeModel::load()
{
    if (m_isLoadingBlocked) {
        return;
    }

    TRACEFUNC;
    beginResetModel();

    m_rootItem = new RootTreeItem(m_masterNotationParts);

    async::NotifyList<const Part*> allParts = m_masterNotationParts->partList();
    IDList notationPartIdList = currentNotationPartIdList();

    for (const Part* part : allParts) {
        AbstractInstrumentsPanelTreeItem* partItem = loadPart(part);
        bool visible = part->show() && notationPartIdList.contains(part->id());
        partItem->setIsVisible(visible);
        m_rootItem->appendChild(partItem);
    }

    allParts.onChanged(this, [this]() {
        load();
    });

    allParts.onItemChanged(this, [this](const Part* part) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(part->id()));
        if (!partItem) {
            return;
        }

        updatePartItem(partItem, part);
    });

    endResetModel();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

IDList InstrumentsPanelTreeModel::currentNotationPartIdList() const
{
    IDList result;

    for (const Part* part : m_notationParts->partList()) {
        result << part->id();
    }

    return result;
}

void InstrumentsPanelTreeModel::selectRow(const QModelIndex& rowIndex)
{
    m_selectionModel->select(rowIndex);
}

void InstrumentsPanelTreeModel::addInstruments()
{
    auto mode = ISelectInstrumentsScenario::SelectInstrumentsMode::ShowCurrentInstruments;
    RetVal<PartInstrumentListScoreOrder> selectedInstruments = selectInstrumentsScenario()->selectInstruments(mode);
    if (!selectedInstruments.ret) {
        LOGE() << selectedInstruments.ret.toString();
        return;
    }

    m_masterNotationParts->setScoreOrder(selectedInstruments.val.scoreOrder);
    m_masterNotationParts->setParts(selectedInstruments.val.instruments);

    emit isEmptyChanged();
}

void InstrumentsPanelTreeModel::moveSelectedRowsUp()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    QModelIndex sourceRowFirst = selectedIndexList.first();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowFirst.row() - 1);
}

void InstrumentsPanelTreeModel::moveSelectedRowsDown()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    QModelIndex sourceRowFirst = selectedIndexList.first();
    QModelIndex sourceRowLast = selectedIndexList.last();

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), selectedIndexList.count(), sourceRowFirst.parent(), sourceRowLast.row() + 1);
}

void InstrumentsPanelTreeModel::removeSelectedRows()
{
    if (!m_selectionModel || !m_selectionModel->hasSelection()) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    for (const QModelIndex& selectedIndex : selectedIndexList) {
        AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(selectedIndex);
        removeRows(item->row(), 1, selectedIndex.parent());
    }
}

bool InstrumentsPanelTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    AbstractInstrumentsPanelTreeItem* parentItem = modelIndexToItem(parent);

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    m_isLoadingBlocked = true;
    beginRemoveRows(parent, row, row + count - 1);

    parentItem->removeChildren(row, count, true);

    endRemoveRows();
    m_isLoadingBlocked = false;

    emit isEmptyChanged();

    return true;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractInstrumentsPanelTreeItem*>(index.internalPointer());
}

bool InstrumentsPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                         int destinationChild)
{
    m_isLoadingBlocked = true;

    AbstractInstrumentsPanelTreeItem* sourceParentItem = modelIndexToItem(sourceParent);
    AbstractInstrumentsPanelTreeItem* destinationParentItem = modelIndexToItem(destinationParent);

    if (!sourceParentItem) {
        sourceParentItem = m_rootItem;
    }

    if (!destinationParentItem) {
        destinationParentItem = m_rootItem;
    }

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;
    int destinationRow = (sourceLastRow > destinationChild
                          || sourceParentItem != destinationParentItem) ? destinationChild : destinationChild + 1;

    beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);
    sourceParentItem->moveChildren(sourceFirstRow, count, destinationParentItem, destinationRow);
    endMoveRows();

    updateRearrangementAvailability();

    m_isLoadingBlocked = false;

    return true;
}

bool InstrumentsPanelTreeModel::isSelected(const QModelIndex& rowIndex) const
{
    if (m_selectionModel->selectedIndexes().isEmpty()) {
        return false;
    }

    if (m_selectionModel->isSelected(rowIndex)) {
        return true;
    }

    QModelIndex parentIndex = parent(rowIndex);
    if (m_selectionModel->isSelected(parentIndex)) {
        return true;
    }

    return false;
}

QModelIndex InstrumentsPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    AbstractInstrumentsPanelTreeItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    AbstractInstrumentsPanelTreeItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex InstrumentsPanelTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    AbstractInstrumentsPanelTreeItem* childItem = modelIndexToItem(child);
    AbstractInstrumentsPanelTreeItem* parentItem = qobject_cast<AbstractInstrumentsPanelTreeItem*>(childItem->parentItem());

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int InstrumentsPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    AbstractInstrumentsPanelTreeItem* parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return 0;
    }

    return parentItem->childCount();
}

int InstrumentsPanelTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant InstrumentsPanelTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != ItemRole) {
        return QVariant();
    }

    AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(index);

    if (!item) {
        return QVariant();
    }

    return QVariant::fromValue(qobject_cast<QObject*>(item));
}

QHash<int, QByteArray> InstrumentsPanelTreeModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
}

QItemSelectionModel* InstrumentsPanelTreeModel::selectionModel() const
{
    return m_selectionModel;
}

void InstrumentsPanelTreeModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void InstrumentsPanelTreeModel::setIsMovingDownAvailable(bool isMoveingDownAvailable)
{
    if (m_isMovingDownAvailable == isMoveingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMoveingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

bool InstrumentsPanelTreeModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool InstrumentsPanelTreeModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool InstrumentsPanelTreeModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

bool InstrumentsPanelTreeModel::isAddingAvailable() const
{
    return m_notationParts != nullptr;
}

bool InstrumentsPanelTreeModel::isEmpty() const
{
    return m_rootItem ? m_rootItem->isEmpty() : true;
}

void InstrumentsPanelTreeModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void InstrumentsPanelTreeModel::updateRearrangementAvailability()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        updateMovingUpAvailability(false);
        updateMovingDownAvailability(false);
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    bool isRearrangementAvailable = true;

    QMutableListIterator<QModelIndex> it(selectedIndexList);

    while (it.hasNext() && selectedIndexList.count() > 1) {
        int nextRow = it.next().row();
        int previousRow = it.peekPrevious().row();

        isRearrangementAvailable = (nextRow - previousRow <= 1);

        if (!isRearrangementAvailable) {
            updateMovingUpAvailability(isRearrangementAvailable);
            updateMovingDownAvailability(isRearrangementAvailable);
            return;
        }
    }

    updateMovingUpAvailability(isRearrangementAvailable, selectedIndexList.first());
    updateMovingDownAvailability(isRearrangementAvailable, selectedIndexList.last());
}

void InstrumentsPanelTreeModel::updateMovingUpAvailability(const bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex)
{
    bool isRowInBoundaries = firstSelectedRowIndex.isValid() ? firstSelectedRowIndex.row() > 0 : false;

    setIsMovingUpAvailable(isSelectionMovable && isRowInBoundaries);
}

void InstrumentsPanelTreeModel::updateMovingDownAvailability(const bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex)
{
    AbstractInstrumentsPanelTreeItem* parentItem = modelIndexToItem(lastSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    // exclude the control item
    bool hasControlItem = static_cast<ItemType>(parentItem->type()) != ItemType::ROOT;
    int lastItemRowIndex = parentItem->childCount() - 1 - (hasControlItem ? 1 : 0);

    bool isRowInBoundaries = lastSelectedRowIndex.isValid() ? lastSelectedRowIndex.row() < lastItemRowIndex : false;

    setIsMovingDownAvailable(isSelectionMovable && isRowInBoundaries);
}

void InstrumentsPanelTreeModel::updateRemovingAvailability()
{
    setIsRemovingAvailable(m_selectionModel->hasSelection());
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::loadPart(const Part* part)
{
    auto partItem = buildPartItem(part);
    QString partId = part->id();

    async::NotifyList<const Staff*> staves = m_masterNotationParts->staffList(partId, part->instrumentId());

    for (const Staff* staff : staves) {
        auto staffItem = buildStaffItem(staff);
        partItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partId);
    partItem->appendChild(addStaffControlItem);

    staves.onItemChanged(this, [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(partItem->childAtId(staff->id()));
        if (!staffItem) {
            return;
        }

        updateStaffItem(staffItem, staff);
    });

    staves.onItemAdded(this, [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto staffItem = buildStaffItem(staff);

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());

        beginInsertRows(partIndex, partItem->childCount() - 1, partItem->childCount() - 1);
        partItem->insertChild(staffItem, partItem->childCount() - 1);
        endInsertRows();
    });

    return partItem;
}

void InstrumentsPanelTreeModel::updateStaffItem(StaffTreeItem* item, const Staff* staff)
{
    QString staffName = staff->staffName();
    QString title = staff->isLinked() ? qtrc("instruments", "[LINK] %1").arg(staffName) : staffName;

    item->setId(staff->id());
    item->setTitle(title);
    item->setIsVisible(staff->show());
    item->setCutawayEnabled(staff->cutaway());
    item->setIsSmall(staff->staffType()->small());
    item->setStaffType(static_cast<int>(staff->staffType()->type()));

    QVariantList visibility;
    for (bool visible: staff->visibilityVoices()) {
        visibility << visible;
    }

    item->setVoicesVisibility(visibility);
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildPartItem(const Part* part)
{
    auto result = new PartTreeItem(m_masterNotationParts, this);
    updatePartItem(result, part);

    connect(result, &AbstractInstrumentsPanelTreeItem::isVisibleChanged, this, [this, part](const bool isVisible) {
        //! NOTE: need to change a visibility only in a current notation
        m_notationParts->setPartVisible(part->id(), isVisible);
    });

    return result;
}

void InstrumentsPanelTreeModel::updatePartItem(PartTreeItem* item, const Part* part)
{
    item->setId(part->id());
    item->setTitle(part->partName().isEmpty() ? part->instrument()->name() : part->partName());
    item->setIsVisible(part->show());
    item->setInstrumentId(part->instrumentId());
    item->setInstrumentName(part->instrument()->name());
    item->setInstrumentAbbreviature(part->instrument()->abbreviature());
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildStaffItem(const Staff* staff)
{
    auto result = new StaffTreeItem(m_masterNotationParts, this);
    updateStaffItem(result, staff);

    connect(result, &AbstractInstrumentsPanelTreeItem::isVisibleChanged, [this, staff](const bool isVisible) {
        //! NOTE: need to change a visibility only in a current notation
        m_notationParts->setStaffVisible(staff->id(), isVisible);
    });

    return result;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildAddStaffControlItem(const QString& partId)
{
    auto result = new StaffControlTreeItem(m_masterNotationParts, this);
    result->setTitle(qtrc("instruments", "Add staff"));
    result->setPartId(partId);

    return result;
}
