#include "instrumentpaneltreemodel.h"
#include <algorithm>

#include "instrumentstypes.h"

using namespace mu::scene::instruments;

InstrumentPanelTreeModel::InstrumentPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    using ItemType = InstrumentTreeItemType::ItemType;

    m_selectionModel = new QItemSelectionModel(this);

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection&, const QItemSelection&) {
        updateRearrangementAvailability();
        updateRemovingAvailability();
    });

    m_rootItem = new AbstractInstrumentTreeItem(ItemType::ROOT);

    for (int i = 0; i < 10; ++i) {
        auto partItem = new AbstractInstrumentTreeItem(ItemType::PART);
        partItem->setTitle(QString("Part %1").arg(i));

        m_rootItem->appendChild(partItem);

        for (int y = 0; y <= i; ++y) {
            auto instrument = new AbstractInstrumentTreeItem(ItemType::INSTRUMENT);
            instrument->setTitle(QString("Instrument %1 - %2").arg(i)
                                    .arg(y));

            partItem->appendChild(instrument);

            for (int j = 0; j <= y; ++j) {
                auto staff = new AbstractInstrumentTreeItem(ItemType::STAFF);
                staff->setTitle(QString("Staff %1 - %2 - %3").arg(i)
                                           .arg(y)
                                           .arg(j));

                instrument->appendChild(staff);
            }

            auto addStaffControlItem = new AbstractInstrumentTreeItem(ItemType::CONTROL_ADD_STAFF);
            addStaffControlItem->setTitle("Add staff");

            instrument->appendChild(addStaffControlItem);
        }

        auto addDoubleInstrumentControlItem = new AbstractInstrumentTreeItem(ItemType::CONTROL_ADD_DOUBLE_INSTRUMENT);
        addDoubleInstrumentControlItem->setTitle("Add doubling instrument");

        partItem->appendChild(addDoubleInstrumentControlItem);
    }
}

InstrumentPanelTreeModel::~InstrumentPanelTreeModel()
{
    delete m_rootItem;
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
    AbstractInstrumentTreeItem* parentItem = static_cast<AbstractInstrumentTreeItem*>(parent.internalPointer());

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

    AbstractInstrumentTreeItem* sourceParentItem = static_cast<AbstractInstrumentTreeItem*>(sourceParent.internalPointer());
    AbstractInstrumentTreeItem* destinationParentItem = static_cast<AbstractInstrumentTreeItem*>(destinationParent.internalPointer());

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;
    int destinationRow = sourceLastRow > destinationChild || sourceParent != destinationParent ? destinationChild : destinationChild + 1;

    bool result = beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);

    if (!result) {
        return result;
    }

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        AbstractInstrumentTreeItem* sourceRowItem = sourceParentItem->childAtRow(i);

        if (!sourceRowItem) {
            continue;
        }

        if (sourceLastRow < destinationChild) {
            destinationParentItem->insertChild(sourceRowItem, destinationRow);
            sourceParentItem->removeChildren(sourceRow);
        } else {
            sourceParentItem->removeChildren(sourceRow);
            destinationParentItem->insertChild(sourceRowItem, destinationRow);
        }
    }

    endMoveRows();

    updateRearrangementAvailability();

    return result;
}

QModelIndex InstrumentPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    AbstractInstrumentTreeItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<AbstractInstrumentTreeItem*>(parent.internalPointer());
    }

    AbstractInstrumentTreeItem* childItem = parentItem->childAtRow(row);

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

    AbstractInstrumentTreeItem* childItem = static_cast<AbstractInstrumentTreeItem*>(child.internalPointer());
    AbstractInstrumentTreeItem* parentItem = qobject_cast<AbstractInstrumentTreeItem*>(childItem->parentItem());

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int InstrumentPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    AbstractInstrumentTreeItem* parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<AbstractInstrumentTreeItem*>(parent.internalPointer());
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

    AbstractInstrumentTreeItem* item = static_cast<AbstractInstrumentTreeItem*>(index.internalPointer());

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
    if (m_isMovingUpAvailable == isMovingUpAvailable)
        return;

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void InstrumentPanelTreeModel::setIsMovingDownAvailable(bool isMoveingDownAvailable)
{
    if (m_isMovingDownAvailable == isMoveingDownAvailable)
        return;

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
    AbstractInstrumentTreeItem* parentItem = static_cast<AbstractInstrumentTreeItem*>(lastSelectedRowIndex.parent().internalPointer());

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
