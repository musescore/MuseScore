//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "instrumentpaneltreemodel.h"

#include <algorithm>

#include "translation.h"
#include "roottreeitem.h"
#include "parttreeitem.h"
#include "instrumenttreeitem.h"
#include "instrumentcontroltreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"
#include "log.h"

using namespace mu::instruments;
using namespace mu::notation;
using ItemType = InstrumentTreeItemType::ItemType;

InstrumentPanelTreeModel::InstrumentPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    context()->currentNotationChanged().onNotify(this, [this]() {
        m_notationParts = context()->currentMasterNotation()->parts();

        load();
    });

    m_selectionModel = new QItemSelectionModel(this);

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, [this](const QItemSelection&, const QItemSelection&) {
        updateRearrangementAvailability();
        updateRemovingAvailability();

        emit selectionChanged();
    });
}

InstrumentPanelTreeModel::~InstrumentPanelTreeModel()
{
    delete m_rootItem;
}

void InstrumentPanelTreeModel::load()
{
    beginResetModel();

    m_rootItem = new RootTreeItem(m_notationParts, this);

    async::NotifyList<const Part*> parts = m_notationParts->partList();

    for (const Part* part : parts) {
        auto partItem = loadPart(part);
        m_rootItem->appendChild(partItem);
    }

    parts.onChanged(this, [this]() {
        load();
    });

    parts.onItemChanged(this, [this](const Part* part) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(part->id()));
        if (!partItem) {
            return;
        }

        updatePartItem(partItem, part);

        for (int i = 0; i < partItem->childCount(); i++) {
            auto item = partItem->childAtRow(i);
            auto itemType = static_cast<InstrumentTreeItemType::ItemType>(item->type());
            if (itemType != InstrumentTreeItemType::ItemType::INSTRUMENT) {
                continue;
            }

            auto instrumentItem = dynamic_cast<InstrumentTreeItem*>(partItem->childAtRow(i));
            instrumentItem->setPartName(partItem->title());
        }
    });

    endResetModel();
}

void InstrumentPanelTreeModel::selectRow(const QModelIndex& rowIndex, const bool isMultipleSelectionModeOn)
{
    for (const QModelIndex& selectedIndex : m_selectionModel->selectedIndexes()) {
        if (rowIndex.parent() != selectedIndex.parent()) {
            m_selectionModel->select(rowIndex, QItemSelectionModel::ClearAndSelect);
            return;
        }
    }

    if (m_selectionModel->isSelected(rowIndex)) {
        m_selectionModel->select(rowIndex, QItemSelectionModel::Deselect);
        return;
    }

    if (isMultipleSelectionModeOn) {
        m_selectionModel->select(rowIndex, QItemSelectionModel::Select);
    } else {
        m_selectionModel->select(rowIndex, QItemSelectionModel::ClearAndSelect);
    }
}

void InstrumentPanelTreeModel::addInstruments()
{
    mu::RetVal<Val> result = interactive()->open("musescore://instruments/select");

    if (!result.ret) {
        LOGE() << result.ret.toString();
        return;
    }

    QVariantList objList = result.val.toQVariant().toList();
    InstrumentList instruments;

    for (const QVariant& obj: objList) {
        instruments << obj.value<Instrument>();
    }

    m_notationParts->setInstruments(instruments);
}

void InstrumentPanelTreeModel::moveSelectedRowsUp()
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

void InstrumentPanelTreeModel::moveSelectedRowsDown()
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

void InstrumentPanelTreeModel::removeSelectedRows()
{
    if (!m_selectionModel || !m_selectionModel->hasSelection()) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    QModelIndex parentIndex = selectedIndexList.first().parent();

    for (const QModelIndex& selectedIndex : selectedIndexList) {
        removeRows(selectedIndex.row(), 1, parentIndex);
    }
}

bool InstrumentPanelTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    AbstractInstrumentPanelTreeItem* parentItem = modelIndexToItem(parent);

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    beginRemoveRows(parent, row, row + count - 1);

    parentItem->removeChildren(row, count, true);

    endRemoveRows();

    return true;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractInstrumentPanelTreeItem*>(index.internalPointer());
}

bool InstrumentPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                        int destinationChild)
{
    if (!sourceParent.isValid() || !destinationParent.isValid()) {
        return false;
    }

    AbstractInstrumentPanelTreeItem* sourceParentItem = modelIndexToItem(sourceParent);
    AbstractInstrumentPanelTreeItem* destinationParentItem = modelIndexToItem(destinationParent);

    if (!sourceParentItem || !destinationParentItem) {
        return false;
    }

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;
    int destinationRow = (sourceLastRow > destinationChild
                          || sourceParentItem != destinationParentItem) ? destinationChild : destinationChild + 1;

    bool result = beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);

    if (!result) {
        return result;
    }

    sourceParentItem->moveChildren(sourceFirstRow, count, destinationParentItem, destinationRow);

    endMoveRows();
    updateRearrangementAvailability();

    return result;
}

QModelIndex InstrumentPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    AbstractInstrumentPanelTreeItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    AbstractInstrumentPanelTreeItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex InstrumentPanelTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    AbstractInstrumentPanelTreeItem* childItem = modelIndexToItem(child);
    AbstractInstrumentPanelTreeItem* parentItem = qobject_cast<AbstractInstrumentPanelTreeItem*>(childItem->parentItem());

    if (!parentItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int InstrumentPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    AbstractInstrumentPanelTreeItem* parentItem;

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

int InstrumentPanelTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant InstrumentPanelTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != ItemRole) {
        return QVariant();
    }

    AbstractInstrumentPanelTreeItem* item = modelIndexToItem(index);

    if (!item) {
        return QVariant();
    }

    return QVariant::fromValue(qobject_cast<QObject*>(item));
}

QHash<int, QByteArray> InstrumentPanelTreeModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
}

QItemSelectionModel* InstrumentPanelTreeModel::selectionModel() const
{
    return m_selectionModel;
}

void InstrumentPanelTreeModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void InstrumentPanelTreeModel::setIsMovingDownAvailable(bool isMoveingDownAvailable)
{
    if (m_isMovingDownAvailable == isMoveingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMoveingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

bool InstrumentPanelTreeModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool InstrumentPanelTreeModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool InstrumentPanelTreeModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

void InstrumentPanelTreeModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void InstrumentPanelTreeModel::updateRearrangementAvailability()
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

    QMutableListIterator<QModelIndex> i(selectedIndexList);

    while (i.hasNext() && selectedIndexList.count() > 1) {
        int nextRow = i.next().row();
        int previousRow = i.peekPrevious().row();

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

void InstrumentPanelTreeModel::updateMovingUpAvailability(const bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex)
{
    bool isRowInBoundaries = firstSelectedRowIndex.isValid() ? firstSelectedRowIndex.row() > 0 : false;

    setIsMovingUpAvailable(isSelectionMovable && isRowInBoundaries);
}

void InstrumentPanelTreeModel::updateMovingDownAvailability(const bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex)
{
    AbstractInstrumentPanelTreeItem* parentItem = modelIndexToItem(lastSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    int lastItemRowIndex = parentItem->childCount() - 1;

    bool isRowInBoundaries = lastSelectedRowIndex.isValid() ? lastSelectedRowIndex.row() < lastItemRowIndex - 1 : false;

    setIsMovingDownAvailable(isSelectionMovable && isRowInBoundaries);
}

void InstrumentPanelTreeModel::updateRemovingAvailability()
{
    setIsRemovingAvailable(m_selectionModel->hasSelection());
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::loadPart(const Part* part)
{
    auto partItem = buildPartItem(part);
    QString partId = part->id();
    QString partName = part->partName();

    async::NotifyList<Instrument> instruments = m_notationParts->instrumentList(partId);

    for (const Instrument& instrument : instruments) {
        auto instrumentItem = loadInstrument(instrument, partId, partName);
        partItem->appendChild(instrumentItem);
    }

    auto addDoubleInstrumentControlItem = buildAddDoubleInstrumentControlItem(partId);
    partItem->appendChild(addDoubleInstrumentControlItem);

    instruments.onItemChanged(this, [this, partId](const Instrument& instrument) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = dynamic_cast<InstrumentTreeItem*>(partItem->childAtId(instrument.id));
        if (!instrumentItem) {
            return;
        }

        updateInstrumentItem(instrumentItem, instrument, partItem->id(), partItem->title());
    });

    instruments.onItemAdded(this, [this, partId](const Instrument& instrument) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = buildInstrumentItem(partId, partItem->title(), instrument);
        QModelIndex partIndex = this->index(partItem->row(), 0, QModelIndex());

        beginInsertRows(partIndex, partItem->childCount() - 1, partItem->childCount() - 1);
        partItem->insertChild(instrumentItem, partItem->childCount() - 1);
        endInsertRows();
    });

    return partItem;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::loadInstrument(const Instrument& instrument, const QString& partId,
                                                                          const QString& partName)
{
    auto instrumentItem = buildInstrumentItem(partId, partName, instrument);
    QString instrumentId = instrument.id;

    async::NotifyList<const Staff*> staves = m_notationParts->staffList(partId, instrumentId);

    for (const Staff* staff : staves) {
        auto staffItem = buildStaffItem(partId, instrumentId, staff);
        instrumentItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partId, instrumentId);
    instrumentItem->appendChild(addStaffControlItem);

    staves.onItemChanged(this, [this, partId, instrumentId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = partItem->childAtId(instrumentId);
        if (!instrumentItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(instrumentItem->childAtId(staff->id()));
        if (!staffItem) {
            return;
        }

        updateStaffItem(staffItem, staff, partId, instrumentId);
    });

    staves.onItemAdded(this, [this, partId, instrumentId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = partItem->childAtId(instrumentId);
        if (!instrumentItem) {
            return;
        }

        auto staffItem = buildStaffItem(partId, instrumentId, staff);

        QModelIndex partIndex = this->index(partItem->row(), 0, QModelIndex());
        QModelIndex instrumentIndex = this->index(instrumentItem->row(), 0, partIndex);

        beginInsertRows(instrumentIndex, instrumentItem->childCount() - 1, instrumentItem->childCount() - 1);
        instrumentItem->insertChild(staffItem, instrumentItem->childCount() - 1);
        endInsertRows();
    });

    return instrumentItem;
}

void InstrumentPanelTreeModel::updateInstrumentItem(InstrumentTreeItem* item, const Instrument& instrument, const QString& partId,
                                                    const QString& partName)
{
    item->setTitle(instrument.longNames.first().name());
    item->setAbbreviature(instrument.abbreviature());
    item->setId(instrument.id);
    item->setIsVisible(instrument.visible);
    item->setPartId(partId);
    item->setPartName(partName);
    item->setCanChangeVisibility(m_notationParts->canChangeInstrumentVisibility(partId, instrument.id));
}

void InstrumentPanelTreeModel::updateStaffItem(StaffTreeItem* item, const Staff* staff, const QString& partId, const QString& instrumentId)
{
    item->setTitle(staff->staffName());
    item->setId(staff->id());
    item->setStaffIndex(staff->idx());
    item->setIsVisible(staff->show());
    item->setPartId(partId);
    item->setInstrumentId(instrumentId);
    item->setCutawayEnabled(staff->cutaway());
    item->setIsSmall(staff->staffType()->small());
    item->setStaffType(static_cast<int>(staff->staffType()->type()));

    QVariantList visibility;
    for (bool visible: staff->visibilityVoices()) {
        visibility << visible;
    }

    item->setVoicesVisibility(visibility);
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildPartItem(const Part* part)
{
    auto result = new PartTreeItem(m_notationParts, this);
    updatePartItem(result, part);

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, part](const bool isVisible) {
        m_notationParts->setPartVisible(part->id(), isVisible);
    });

    return result;
}

void InstrumentPanelTreeModel::updatePartItem(PartTreeItem* item, const Part* part)
{
    item->setTitle(part->partName());
    item->setId(part->id());
    item->setIsVisible(part->show());
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildInstrumentItem(const QString& partId, const QString& partName,
                                                                               const Instrument& instrument)
{
    auto result = new InstrumentTreeItem(m_notationParts, this);
    updateInstrumentItem(result, instrument, partId, partName);

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, partId, instrument](const bool isVisible) {
        m_notationParts->setInstrumentVisible(partId, instrument.id, isVisible);
    });

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildStaffItem(const QString& partId, const QString& instrumentId,
                                                                          const Staff* staff)
{
    auto result = new StaffTreeItem(m_notationParts, this);
    updateStaffItem(result, staff, partId, instrumentId);

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildAddStaffControlItem(const QString& partId, const QString& instrumentId)
{
    auto result = new StaffControlTreeItem(m_notationParts, this);
    result->setTitle(qtrc("instruments", "Add staff"));
    result->setPartId(partId);
    result->setInstrumentId(instrumentId);

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildAddDoubleInstrumentControlItem(const QString& partId)
{
    auto result = new InstrumentControlTreeItem(m_notationParts, this);
    result->setTitle(qtrc("instruments", "Add doubling instrument"));
    result->setPartId(partId);

    return result;
}
