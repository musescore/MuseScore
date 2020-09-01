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
        m_notationParts = context()->currentNotation()->parts();

        registerReceivers();

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

    m_rootItem = new RootTreeItem(m_notationParts);

    for (const Part* part : m_notationParts->partList()) {
        auto partItem = buildPartItem(part);
        m_rootItem->appendChild(partItem);

        for (const Instrument instrument : m_notationParts->instrumentList(part->id())) {
            auto instrumentItem = buildInstrumentItem(part->id(), part->partName(), instrument);
            partItem->appendChild(instrumentItem);

            for (const Staff* staff : m_notationParts->staffList(part->id(), instrument.id)) {
                auto staffItem = buildStaffItem(part->id(), instrument.id, staff);
                instrumentItem->appendChild(staffItem);
            }

            auto addStaffControlItem = buildAddStaffControlItem(part->id(), instrument.id);
            instrumentItem->appendChild(addStaffControlItem);
        }

        auto addDoubleInstrumentControlItem = buildAddDoubleInstrumentControlItem(part->id());
        partItem->appendChild(addDoubleInstrumentControlItem);
    }

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
    AbstractInstrumentPanelTreeItem* parentItem = static_cast<AbstractInstrumentPanelTreeItem*>(parent.internalPointer());

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    beginRemoveRows(parent, row, row + count - 1);

    parentItem->removeChildren(row, count, true);

    endRemoveRows();

    return true;
}

bool InstrumentPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                        int destinationChild)
{
    if (!sourceParent.isValid() || !destinationParent.isValid()) {
        return false;
    }

    AbstractInstrumentPanelTreeItem* sourceParentItem = static_cast<AbstractInstrumentPanelTreeItem*>(sourceParent.internalPointer());
    AbstractInstrumentPanelTreeItem* destinationParentItem
        = static_cast<AbstractInstrumentPanelTreeItem*>(destinationParent.internalPointer());

    if (!sourceParentItem || !destinationParentItem) {
        return false;
    }

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;
    int destinationRow = sourceLastRow > destinationChild
                         || sourceParentItem != destinationParentItem ? destinationChild : destinationChild + 1;

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
        parentItem = static_cast<AbstractInstrumentPanelTreeItem*>(parent.internalPointer());
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

    AbstractInstrumentPanelTreeItem* childItem = static_cast<AbstractInstrumentPanelTreeItem*>(child.internalPointer());
    AbstractInstrumentPanelTreeItem* parentItem = qobject_cast<AbstractInstrumentPanelTreeItem*>(childItem->parentItem());

    if (parentItem == m_rootItem) {
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
        parentItem = static_cast<AbstractInstrumentPanelTreeItem*>(parent.internalPointer());
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

    AbstractInstrumentPanelTreeItem* item = static_cast<AbstractInstrumentPanelTreeItem*>(index.internalPointer());

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
    AbstractInstrumentPanelTreeItem* parentItem
        = static_cast<AbstractInstrumentPanelTreeItem*>(lastSelectedRowIndex.parent().internalPointer());

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


void InstrumentPanelTreeModel::updateInstrumentItem(InstrumentTreeItem* item, const Instrument& instrument, const QString& partId,
                                                    const QString& partName)
{
    item->setTitle(instrument.longNames.first().name());
    item->setAbbreviature(instrument.abbreviature());
    item->setId(instrument.id);
    item->setIsVisible(instrument.visible);
    item->setPartId(partId);
    item->setPartName(partName);
}

void InstrumentPanelTreeModel::updateStaffItem(StaffTreeItem* item, const Staff* staff, const QString& partId, const QString& instrumentId)
{
    item->setTitle(staff->staffName());
    item->setId(QVariant::fromValue(staff->idx()).toString());
    item->setIsVisible(!staff->invisible());
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
    result->setTitle(part->partName());
    result->setId(part->id());
    result->setIsVisible(part->show());

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, part](const bool isVisible) {
        m_notationParts->setPartVisible(part->id(), isVisible);
    });

    return result;
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

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, staff](const bool isVisible) {
        m_notationParts->setStaffVisible(staff->idx(), isVisible);
    });

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

void InstrumentPanelTreeModel::registerReceivers()
{
    m_notationParts->partChanged().onReceive(this, [this](const INotationParts::PartChangeData& newPart) {
        auto newPartItem = buildPartItem(newPart.part);

        int row = m_rootItem->childAtId(newPart.part->id())->row();
        m_rootItem->replaceChild(newPartItem, row);

        QModelIndex partIndex = this->index(row, 0, QModelIndex());
        emit dataChanged(partIndex, partIndex, { ItemRole });
    });

    m_notationParts->instrumentChanged().onReceive(this, [this](const INotationParts::InstrumentChangeData& newInstrumentData) {
        auto partItem = m_rootItem->childAtId(newInstrumentData.partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = dynamic_cast<InstrumentTreeItem*>(partItem->childAtId(newInstrumentData.instrument.id));
        if (!instrumentItem) {
            return;
        }

        updateInstrumentItem(instrumentItem, newInstrumentData.instrument, newInstrumentData.partId, partItem->title());
    });

    m_notationParts->staffChanged().onReceive(this, [this](const INotationParts::StaffChangeData& newStaffData) {
        auto partItem = m_rootItem->childAtId(newStaffData.partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = partItem->childAtId(newStaffData.instrumentId);
        if (!instrumentItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(instrumentItem->childAtId(QString::number(newStaffData.staff->idx())));
        if (!staffItem) {
            return;
        }

        updateStaffItem(staffItem, newStaffData.staff, newStaffData.partId, newStaffData.instrumentId);
    });

    m_notationParts->staffAppended().onReceive(this, [this](const INotationParts::StaffChangeData& newStaffData) {
        auto partItem = m_rootItem->childAtId(newStaffData.partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = partItem->childAtId(newStaffData.instrumentId);
        if (!instrumentItem) {
            return;
        }

        auto staffItem = buildStaffItem(newStaffData.partId, newStaffData.instrumentId, newStaffData.staff);

        QModelIndex partIndex = this->index(partItem->row(), 0, QModelIndex());
        QModelIndex instrumentIndex = this->index(instrumentItem->row(), 0, partIndex);

        beginInsertRows(instrumentIndex, instrumentItem->childCount() - 1, instrumentItem->childCount() - 1);
        instrumentItem->insertChild(staffItem, instrumentItem->childCount() - 1);
        endInsertRows();
    });
}
