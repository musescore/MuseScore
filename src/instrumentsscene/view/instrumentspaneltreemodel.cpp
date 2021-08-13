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
    m_partsNotifyReceiver = std::make_shared<async::Asyncable>();

    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        m_masterNotation = context()->currentMasterNotation();
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        m_partsNotifyReceiver->disconnectAll();

        m_notation = context()->currentNotation();

        if (m_notation) {
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
    return m_masterNotation != nullptr && m_notation != nullptr;
}

void InstrumentsPanelTreeModel::setupPartsConnections()
{
    async::NotifyList<const Part*> notationParts = m_notation->parts()->partList();

    notationParts.onChanged(m_partsNotifyReceiver.get(), [this]() {
        load();
    });

    notationParts.onItemChanged(m_partsNotifyReceiver.get(), [this](const Part* part) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(part->id()));
        if (!partItem) {
            return;
        }

        const Part* masterPart = m_masterNotation->parts()->part(part->id());
        updatePartItem(partItem, masterPart);
    });
}

void InstrumentsPanelTreeModel::setupStavesConnections(const ID& stavesPartId)
{
    async::NotifyList<const Staff*> notationStaves = m_notation->parts()->staffList(stavesPartId);

    notationStaves.onItemChanged(m_partsNotifyReceiver.get(), [this, stavesPartId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(stavesPartId);
        if (!partItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(partItem->childAtId(staff->id()));
        if (!staffItem) {
            return;
        }

        const Staff* masterStaff = m_masterNotation->parts()->staff(staff->id());
        updateStaffItem(staffItem, masterStaff);
    });

    notationStaves.onItemAdded(m_partsNotifyReceiver.get(), [this, stavesPartId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(stavesPartId);
        if (!partItem) {
            return;
        }

        const Staff* masterStaff = m_masterNotation->parts()->staff(staff->id());
        auto staffItem = buildMasterStaffItem(masterStaff);

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());

        beginInsertRows(partIndex, partItem->childCount() - 1, partItem->childCount() - 1);
        partItem->insertChild(staffItem, partItem->childCount() - 1);
        endInsertRows();
    });
}

void InstrumentsPanelTreeModel::listenSelectionChanged()
{
    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        std::vector<Element*> selectedElements = m_notation->interaction()->selection()->elements();

        if (selectedElements.empty()) {
            m_selectionModel->clear();
            return;
        }

        QSet<ID> selectedPartIdSet;
        for (const Element* element : selectedElements) {
            if (!element->part()) {
                continue;
            }

            selectedPartIdSet << element->part()->id();
        }

        for (const ID& selectedPartId : selectedPartIdSet) {
            AbstractInstrumentsPanelTreeItem* item = m_rootItem->childAtId(selectedPartId);

            if (item) {
                m_selectionModel->select(createIndex(item->row(), 0, item));
            }
        }
    });
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

    m_rootItem = new RootTreeItem(m_masterNotation, m_notation);

    async::NotifyList<const Part*> masterParts = m_masterNotation->parts()->partList();

    for (const Part* part : masterParts) {
        m_rootItem->appendChild(loadMasterPart(part));
    }

    endResetModel();

    setupPartsConnections();
    listenSelectionChanged();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

bool InstrumentsPanelTreeModel::isPartExsistsOnCurrentNotation(const ID& partId) const
{
    return m_notation->parts()->partExists(partId);
}

bool InstrumentsPanelTreeModel::isStaffExsistsOnCurrentNotation(const ID& staffId) const
{
    return m_notation->parts()->staffExists(staffId);
}

void InstrumentsPanelTreeModel::selectRow(const QModelIndex& rowIndex)
{
    m_selectionModel->select(rowIndex);
}

void InstrumentsPanelTreeModel::addInstruments()
{
    RetVal<PartInstrumentListScoreOrder> selectedInstruments = selectInstrumentsScenario()->selectInstruments();
    if (!selectedInstruments.ret) {
        LOGE() << selectedInstruments.ret.toString();
        return;
    }

    m_masterNotation->parts()->setScoreOrder(selectedInstruments.val.scoreOrder);
    m_masterNotation->parts()->setParts(selectedInstruments.val.instruments);

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
    if (!m_selectionModel->hasSelection()) {
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
    return m_notation != nullptr;
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

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::loadMasterPart(const Part* masterPart)
{
    TRACEFUNC;

    auto partItem = buildPartItem(masterPart);
    ID partId = masterPart->id();

    async::NotifyList<const Staff*> masterStaves = m_masterNotation->parts()->staffList(partId);

    for (const Staff* staff : masterStaves) {
        auto staffItem = buildMasterStaffItem(staff);
        partItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partId);
    partItem->appendChild(addStaffControlItem);

    setupStavesConnections(partId);

    return partItem;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildPartItem(const Part* masterPart)
{
    auto result = new PartTreeItem(m_masterNotation, m_notation, this);
    updatePartItem(result, masterPart);

    return result;
}

void InstrumentsPanelTreeModel::updatePartItem(PartTreeItem* item, const Part* masterPart)
{
    const Part* part = masterPart;
    bool visible = false;

    if (isPartExsistsOnCurrentNotation(masterPart->id())) {
        part = m_notation->parts()->part(masterPart->id());
        visible = part->show();
    }

    item->setId(part->id());
    item->setTitle(part->partName().isEmpty() ? part->instrument()->name() : part->partName());
    item->setIsVisible(visible);
    item->setInstrumentId(part->instrumentId());
    item->setInstrumentName(part->instrument()->name());
    item->setInstrumentAbbreviature(part->instrument()->abbreviature());
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildMasterStaffItem(const Staff* masterStaff)
{
    auto result = new StaffTreeItem(m_masterNotation, m_notation, this);
    updateStaffItem(result, masterStaff);

    return result;
}

void InstrumentsPanelTreeModel::updateStaffItem(StaffTreeItem* item, const Staff* masterStaff)
{
    const Staff* staff = masterStaff;
    bool visible = false;

    if (isStaffExsistsOnCurrentNotation(masterStaff->id())) {
        staff = m_notation->parts()->staff(masterStaff->id());
        visible = staff->show();
    }

    QString staffName = staff->staffName();
    QString title = masterStaff->isLinked() ? qtrc("instruments", "[LINK] %1").arg(staffName) : staffName;

    item->setId(staff->id());
    item->setTitle(title);
    item->setIsVisible(visible);
    item->setCutawayEnabled(staff->cutaway());
    item->setIsSmall(staff->staffType()->isSmall());
    item->setStaffType(static_cast<int>(staff->staffType()->type()));

    QVariantList visibility;
    for (bool visible: staff->visibilityVoices()) {
        visibility << visible;
    }

    item->setVoicesVisibility(visibility);
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildAddStaffControlItem(const ID& partId)
{
    auto result = new StaffControlTreeItem(m_masterNotation, m_notation, this);
    result->setTitle(qtrc("instruments", "Add staff"));
    result->setPartId(partId);

    return result;
}
