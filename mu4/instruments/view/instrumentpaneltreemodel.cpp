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
    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        INotationPtr masterNotation = context()->currentMasterNotation();
        m_masterNotationParts = masterNotation ? masterNotation->parts() : nullptr;
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

    m_selectionModel = new QItemSelectionModel(this);

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, [this](const QItemSelection&, const QItemSelection&) {
        updateRearrangementAvailability();
        updateRemovingAvailability();

        emit selectionChanged();
    });
}

InstrumentPanelTreeModel::~InstrumentPanelTreeModel()
{
    deleteItems();
}

void InstrumentPanelTreeModel::clear()
{
    beginResetModel();
    deleteItems();
    endResetModel();

    emit isEmptyChanged();
}

void InstrumentPanelTreeModel::deleteItems()
{
    delete m_rootItem;
    m_rootItem = nullptr;
}

void InstrumentPanelTreeModel::load()
{
    beginResetModel();

    m_rootItem = new RootTreeItem(m_masterNotationParts);

    async::NotifyList<const Part*> allParts = m_masterNotationParts->partList();
    IDList notationPartIdList = currentNotationPartIdList();

    for (const Part* part : allParts) {
        AbstractInstrumentPanelTreeItem* partItem = loadPart(part);
        bool visible = part->show() && notationPartIdList.contains(part->id());
        partItem->setIsVisible(visible);
        m_rootItem->appendChild(partItem);
    }

    allParts.onChanged(this, [this]() {
        load();
    });

    allParts.onItemChanged(this, [this](const Part* part) {
        auto partItem = m_rootItem->childAtId(part->id());
        if (!partItem) {
            return;
        }

        updatePartItem(partItem, part);

        for (auto item : partItem->childrenItems()) {
            auto instrumentItem = dynamic_cast<InstrumentTreeItem*>(item);

            if (instrumentItem) {
                instrumentItem->setPartName(partItem->title());
            }
        }
    });

    endResetModel();
}

IDList InstrumentPanelTreeModel::currentNotationPartIdList() const
{
    IDList result;

    for (const Part* part : m_notationParts->partList()) {
        result << part->id();
    }

    return result;
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

    m_masterNotationParts->setInstruments(instruments);

    emit isEmptyChanged();
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

    for (const QModelIndex& selectedIndex : selectedIndexList) {
        AbstractInstrumentPanelTreeItem* item = modelIndexToItem(selectedIndex);
        removeRows(item->row(), 1, selectedIndex.parent());
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

    emit isEmptyChanged();

    return true;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractInstrumentPanelTreeItem*>(index.internalPointer());
}

bool InstrumentPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                        int destinationChild)
{
    AbstractInstrumentPanelTreeItem* sourceParentItem = modelIndexToItem(sourceParent);
    AbstractInstrumentPanelTreeItem* destinationParentItem = modelIndexToItem(destinationParent);

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

    return true;
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

bool InstrumentPanelTreeModel::isEmpty() const
{
    return m_rootItem ? m_rootItem->isEmpty() : true;
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

    // exclude the control item
    bool hasControlItem = static_cast<ItemType>(parentItem->type()) != ItemType::ROOT;
    int lastItemRowIndex = parentItem->childCount() - 1 - (hasControlItem ? 1 : 0);

    bool isRowInBoundaries = lastSelectedRowIndex.isValid() ? lastSelectedRowIndex.row() < lastItemRowIndex : false;

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

    async::NotifyList<Instrument> instruments = m_masterNotationParts->instrumentList(partId);

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

    instruments.onItemReplaced(this, [this, partId](const Instrument& oldInstrument, const Instrument& newInstrument) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = dynamic_cast<InstrumentTreeItem*>(partItem->childAtId(oldInstrument.id));
        if (!instrumentItem) {
            return;
        }

        updateInstrumentItem(instrumentItem, newInstrument, partItem->id(), partItem->title());
    });

    instruments.onItemAdded(this, [this, partId](const Instrument& instrument) {
        auto partItem = m_rootItem->childAtId(partId);
        if (!partItem) {
            return;
        }

        auto instrumentItem = loadInstrument(instrument, partId, partItem->title());
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

    async::NotifyList<const Staff*> staves = m_masterNotationParts->staffList(partId, instrumentId);

    for (const Staff* staff : staves) {
        auto staffItem = buildStaffItem(staff);
        instrumentItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partId);
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

        updateStaffItem(staffItem, staff);
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

        auto staffItem = buildStaffItem(staff);

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());
        QModelIndex instrumentIndex = index(instrumentItem->row(), 0, partIndex);

        beginInsertRows(instrumentIndex, instrumentItem->childCount() - 1, instrumentItem->childCount() - 1);
        instrumentItem->insertChild(staffItem, instrumentItem->childCount() - 1);
        endInsertRows();
    });

    return instrumentItem;
}

void InstrumentPanelTreeModel::updateInstrumentItem(InstrumentTreeItem* item, const Instrument& instrument, const QString& partId,
                                                    const QString& partName)
{
    item->setId(instrument.id);
    item->setTitle(instrument.name);
    item->setAbbreviature(instrument.abbreviature());
    item->setIsVisible(instrument.visible);
    item->setPartId(partId);
    item->setPartName(partName);
    item->updateCanChangeVisibility();
}

void InstrumentPanelTreeModel::updateStaffItem(StaffTreeItem* item, const Staff* staff)
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

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildPartItem(const Part* part)
{
    auto result = new PartTreeItem(m_masterNotationParts, this);
    updatePartItem(result, part);

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, part](const bool isVisible) {
        //! NOTE: need to change a visibility only in a current notation
        m_notationParts->setPartVisible(part->id(), isVisible);
    });

    return result;
}

void InstrumentPanelTreeModel::updatePartItem(AbstractInstrumentPanelTreeItem* item, const Part* part)
{
    item->setId(part->id());
    item->setTitle(part->partName());
    item->setIsVisible(part->show());
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildInstrumentItem(const QString& partId, const QString& partName,
                                                                               const Instrument& instrument)
{
    auto result = new InstrumentTreeItem(m_masterNotationParts, this);
    updateInstrumentItem(result, instrument, partId, partName);

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, this, [this, partId, instrument](const bool isVisible) {
        //! NOTE: need to change a visibility only in a current notation
        m_notationParts->setInstrumentVisible(instrument.id, partId, isVisible);
    });

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildStaffItem(const Staff* staff)
{
    auto result = new StaffTreeItem(m_masterNotationParts, this);
    updateStaffItem(result, staff);

    connect(result, &AbstractInstrumentPanelTreeItem::isVisibleChanged, [this, staff](const bool isVisible) {
        //! NOTE: need to change a visibility only in a current notation
        m_notationParts->setStaffVisible(staff->id(), isVisible);
    });

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildAddStaffControlItem(const QString& partId)
{
    auto result = new StaffControlTreeItem(m_masterNotationParts, this);
    result->setTitle(qtrc("instruments", "Add staff"));
    result->setPartId(partId);

    return result;
}

AbstractInstrumentPanelTreeItem* InstrumentPanelTreeModel::buildAddDoubleInstrumentControlItem(const QString& partId)
{
    auto result = new InstrumentControlTreeItem(m_masterNotationParts, this);
    result->setTitle(qtrc("instruments", "Add doubling instrument"));
    result->setPartId(partId);

    return result;
}
