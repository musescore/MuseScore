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
#include "instrumentspaneltreemodel.h"

#include <algorithm>

#include "translation.h"
#include "roottreeitem.h"
#include "parttreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::uicomponents;

using ItemType = InstrumentsTreeItemType::ItemType;

static const muse::actions::ActionCode ADD_INSTRUMENTS_ACTIONCODE("instruments");

namespace mu::instrumentsscene {
static QString notationToKey(const INotationPtr notation)
{
    std::stringstream stream;
    stream << notation.get();

    return QString::fromStdString(stream.str());
}
}

InstrumentsPanelTreeModel::InstrumentsPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_partsNotifyReceiver = std::make_shared<muse::async::Asyncable>();

    m_selectionModel = new ItemMultiSelectionModel(this);
    m_selectionModel->setAllowedModifiers(Qt::ShiftModifier);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
        setItemsSelected(deselected.indexes(), false);
        setItemsSelected(selected.indexes(), true);

        updateRearrangementAvailability();
        updateRemovingAvailability();
        updateIsInstrumentSelected();
    });

    connect(this, &InstrumentsPanelTreeModel::rowsInserted, this, [this]() {
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

void InstrumentsPanelTreeModel::onMasterNotationChanged()
{
    m_masterNotation = context()->currentMasterNotation();
    initPartOrders();
}

void InstrumentsPanelTreeModel::onNotationChanged()
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

InstrumentsPanelTreeModel::~InstrumentsPanelTreeModel()
{
    deleteItems();
}

bool InstrumentsPanelTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!m_isRemovingAvailable) {
        return false;
    }

    AbstractInstrumentsPanelTreeItem* parentItem = modelIndexToItem(parent);

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    if (parentItem == m_rootItem) {
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

    return true;
}

void InstrumentsPanelTreeModel::initPartOrders()
{
    m_sortedPartIdList.clear();

    if (!m_masterNotation) {
        return;
    }

    for (IExcerptNotationPtr excerpt : m_masterNotation->excerpts()) {
        NotationKey key = notationToKey(excerpt->notation());

        for (const Part* part : excerpt->notation()->parts()->partList()) {
            m_sortedPartIdList[key] << part->id();
        }
    }
}

void InstrumentsPanelTreeModel::onBeforeChangeNotation()
{
    if (!m_notation || !m_rootItem) {
        return;
    }

    QList<muse::ID> partIdList;

    for (const AbstractInstrumentsPanelTreeItem* item : m_rootItem->childItems()) {
        partIdList << item->id();
    }

    m_sortedPartIdList[notationToKey(m_notation)] = partIdList;
}

void InstrumentsPanelTreeModel::setLoadingBlocked(bool blocked)
{
    m_isLoadingBlocked = blocked;

    if (!m_isLoadingBlocked && m_notationChangedWhileLoadingWasBlocked) {
        onNotationChanged();
    }
}

void InstrumentsPanelTreeModel::setupPartsConnections()
{
    async::NotifyList<const Part*> notationParts = m_notation->parts()->partList();

    notationParts.onChanged(m_partsNotifyReceiver.get(), [this]() {
        load();
    });

    auto updateMasterPartItem = [this](const muse::ID& partId) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(partId));
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
}

void InstrumentsPanelTreeModel::setupStavesConnections(const muse::ID& stavesPartId)
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

        staffItem->init(m_masterNotation->parts()->staff(staff->id()));
    });

    notationStaves.onItemAdded(m_partsNotifyReceiver.get(), [this, stavesPartId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(stavesPartId);
        if (!partItem) {
            return;
        }

        const Staff* masterStaff = m_masterNotation->parts()->staff(staff->id());
        auto staffItem = buildMasterStaffItem(masterStaff, partItem);

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());

        beginInsertRows(partIndex, partItem->childCount() - 1, partItem->childCount() - 1);
        partItem->insertChild(staffItem, partItem->childCount() - 1);
        endInsertRows();
    });
}

void InstrumentsPanelTreeModel::listenNotationSelectionChanged()
{
    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateSelectedRows();
    });
}

void InstrumentsPanelTreeModel::updateSelectedRows()
{
    if (!m_instrumentsPanelVisible || !m_notation) {
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
        AbstractInstrumentsPanelTreeItem* item = m_rootItem->childAtId(selectedPartId);

        if (item) {
            m_selectionModel->select(createIndex(item->row(), 0, item), QItemSelectionModel::Select);
        }
    }
}

void InstrumentsPanelTreeModel::clear()
{
    TRACEFUNC;

    beginResetModel();
    deleteItems();
    endResetModel();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(false);
}

void InstrumentsPanelTreeModel::deleteItems()
{
    m_selectionModel->clear();
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
    deleteItems();

    m_rootItem = new RootTreeItem(m_masterNotation, m_notation, this);

    async::NotifyList<const Part*> masterParts = m_masterNotation->parts()->partList();
    sortParts(masterParts);

    for (const Part* part : masterParts) {
        m_rootItem->appendChild(loadMasterPart(part));
    }

    endResetModel();

    setupPartsConnections();
    listenNotationSelectionChanged();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

void InstrumentsPanelTreeModel::sortParts(notation::PartList& parts)
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

void InstrumentsPanelTreeModel::setInstrumentsPanelVisible(bool visible)
{
    if (m_instrumentsPanelVisible == visible) {
        return;
    }

    m_instrumentsPanelVisible = visible;

    if (visible) {
        updateSelectedRows();
    }
}

void InstrumentsPanelTreeModel::selectRow(const QModelIndex& rowIndex)
{
    m_selectionModel->select(rowIndex);
}

void InstrumentsPanelTreeModel::clearSelection()
{
    m_selectionModel->clear();
}

void InstrumentsPanelTreeModel::addInstruments()
{
    dispatcher()->dispatch(ADD_INSTRUMENTS_ACTIONCODE);
}

void InstrumentsPanelTreeModel::moveSelectedRowsUp()
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

void InstrumentsPanelTreeModel::moveSelectedRowsDown()
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

void InstrumentsPanelTreeModel::removeSelectedRows()
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

bool InstrumentsPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                         int destinationChild)
{
    setLoadingBlocked(true);

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
    int destinationRow = (sourceLastRow > destinationChild || sourceParentItem != destinationParentItem)
                         ? destinationChild : destinationChild + 1;

    m_activeDragIsStave = destinationParentItem != m_rootItem;

    if (m_dragInProgress) {
        m_activeDragMoveParams = sourceParentItem->buildMoveParams(sourceRow, count, destinationParentItem, destinationRow);
    }

    beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);
    sourceParentItem->moveChildren(sourceFirstRow, count, destinationParentItem, destinationRow, !m_dragInProgress);
    endMoveRows();

    updateRearrangementAvailability();

    setLoadingBlocked(false);

    return true;
}

void InstrumentsPanelTreeModel::startActiveDrag()
{
    m_dragInProgress = true;
}

void InstrumentsPanelTreeModel::endActiveDrag()
{
    setLoadingBlocked(true);

    if (m_activeDragIsStave) {
        m_notation->parts()->moveStaves(m_activeDragMoveParams.childIdListToMove,
                                        m_activeDragMoveParams.destinationParentId,
                                        m_activeDragMoveParams.insertMode);
    } else {
        m_notation->parts()->moveParts(m_activeDragMoveParams.childIdListToMove,
                                       m_activeDragMoveParams.destinationParentId,
                                       m_activeDragMoveParams.insertMode);
    }

    m_activeDragMoveParams = MoveParams();
    m_dragInProgress = false;

    setLoadingBlocked(false);
}

void InstrumentsPanelTreeModel::toggleVisibilityOfSelectedRows(bool visible)
{
    for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
        AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(index);

        item->setIsVisible(visible);
    }
}

QItemSelectionModel* InstrumentsPanelTreeModel::selectionModel() const
{
    return m_selectionModel;
}

QModelIndex InstrumentsPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    const AbstractInstrumentsPanelTreeItem* parentItem = nullptr;

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

    const AbstractInstrumentsPanelTreeItem* childItem = modelIndexToItem(child);
    AbstractInstrumentsPanelTreeItem* parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int InstrumentsPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    const AbstractInstrumentsPanelTreeItem* parentItem = m_rootItem;

    if (parent.isValid()) {
        parentItem = modelIndexToItem(parent);
    }

    return parentItem ? parentItem->childCount() : 0;
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
    return item ? QVariant::fromValue(qobject_cast<QObject*>(item)) : QVariant();
}

QHash<int, QByteArray> InstrumentsPanelTreeModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
}

void InstrumentsPanelTreeModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void InstrumentsPanelTreeModel::setIsMovingDownAvailable(bool isMovingDownAvailable)
{
    if (m_isMovingDownAvailable == isMovingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMovingDownAvailable;
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

bool InstrumentsPanelTreeModel::isInstrumentSelected() const
{
    return m_isInstrumentSelected;
}

QString InstrumentsPanelTreeModel::addInstrumentsKeyboardShortcut() const
{
    const muse::shortcuts::Shortcut& shortcut = shortcutsRegister()->shortcut(ADD_INSTRUMENTS_ACTIONCODE);

    if (shortcut.sequences.empty()) {
        return {};
    }

    return muse::shortcuts::sequencesToNativeText({ shortcut.sequences[0] });
}

void InstrumentsPanelTreeModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void InstrumentsPanelTreeModel::setIsInstrumentSelected(bool isInstrumentSelected)
{
    if (m_isInstrumentSelected == isInstrumentSelected) {
        return;
    }

    m_isInstrumentSelected = isInstrumentSelected;
    emit isInstrumentSelectedChanged(m_isInstrumentSelected);
}

void InstrumentsPanelTreeModel::updateRearrangementAvailability()
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

void InstrumentsPanelTreeModel::updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex)
{
    bool isRowInBoundaries = firstSelectedRowIndex.isValid() ? firstSelectedRowIndex.row() > 0 : false;

    setIsMovingUpAvailable(isSelectionMovable && isRowInBoundaries);
}

void InstrumentsPanelTreeModel::updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex)
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
    QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    bool isRemovingAvailable = !selectedIndexes.empty();
    const AbstractInstrumentsPanelTreeItem* parent = nullptr;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(index);
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

        for (const AbstractInstrumentsPanelTreeItem* item : parent->childItems()) {
            if (item->isRemovable()) {
                removableChildCount++;
            }
        }

        isRemovingAvailable = selectedIndexes.size() < removableChildCount;
    }

    setIsRemovingAvailable(isRemovingAvailable);
}

void InstrumentsPanelTreeModel::updateIsInstrumentSelected()
{
    QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    bool isInstrumentSelected = true;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(index);

        if (item && static_cast<ItemType>(item->type()) == ItemType::STAFF) {
            isInstrumentSelected = false;
            break;
        }
    }

    setIsInstrumentSelected(isInstrumentSelected);
}

void InstrumentsPanelTreeModel::setItemsSelected(const QModelIndexList& indexes, bool selected)
{
    for (const QModelIndex& index : indexes) {
        if (AbstractInstrumentsPanelTreeItem* item = modelIndexToItem(index)) {
            item->setIsSelected(selected);
        }
    }
}

bool InstrumentsPanelTreeModel::warnAboutRemovingInstrumentsIfNecessary(int count)
{
    // Only warn if excerpts are existent
    if (m_masterNotation->excerpts().empty()) {
        return true;
    }

    return interactive()->warning(
        //: Please omit `%n` in the translation in this case; it's only there so that you
        //: have the possibility to provide translations with the correct numerus form,
        //: i.e. to show "instrument" or "instruments" as appropriate.
        muse::trc("instruments", "Are you sure you want to delete the selected %n instrument(s)?", nullptr, count),

        //: Please omit `%n` in the translation in this case; it's only there so that you
        //: have the possibility to provide translations with the correct numerus form,
        //: i.e. to show "instrument" or "instruments" as appropriate.
        muse::trc("instruments", "This will remove the %n instrument(s) from the full score and all part scores.", nullptr, count),

        { IInteractive::Button::No, IInteractive::Button::Yes })
           .standardButton() == IInteractive::Button::Yes;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::loadMasterPart(const Part* masterPart)
{
    TRACEFUNC;

    auto partItem = buildPartItem(masterPart);

    for (const Staff* staff : m_masterNotation->parts()->staffList(partItem->id())) {
        auto staffItem = buildMasterStaffItem(staff, partItem);
        partItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partItem->id(), partItem);
    partItem->appendChild(addStaffControlItem);

    setupStavesConnections(partItem->id());

    return partItem;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildPartItem(const Part* masterPart)
{
    auto result = new PartTreeItem(m_masterNotation, m_notation, m_rootItem);
    result->init(masterPart);

    return result;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildMasterStaffItem(const Staff* masterStaff, QObject* parent)
{
    auto result = new StaffTreeItem(m_masterNotation, m_notation, parent);
    result->init(masterStaff);

    return result;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::buildAddStaffControlItem(const muse::ID& partId, QObject* parent)
{
    auto result = new StaffControlTreeItem(m_masterNotation, m_notation, parent);
    result->init(partId);

    return result;
}

AbstractInstrumentsPanelTreeItem* InstrumentsPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractInstrumentsPanelTreeItem*>(index.internalPointer());
}
