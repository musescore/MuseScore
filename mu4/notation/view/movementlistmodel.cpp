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

#include "movementlistmodel.h"

#include "log.h"

using namespace mu::notation;

MovementListModel::MovementListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(RoleTitle, "title");
    m_roles.insert(RoleSelected, "selected");

    m_selectionModel = new QItemSelectionModel(this);

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
        updateRearrangementAvailability();
        updateRemovingAvailability();

        QModelIndexList indexes;
        indexes << selected.indexes() << deselected.indexes();
        QSet<QModelIndex> indexesSet(indexes.begin(), indexes.end());
        for (const QModelIndex& index: indexesSet) {
            emit dataChanged(index, index, { RoleSelected });
        }
    });
}

bool MovementListModel::removeRows(int row, int, const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row);
    m_movementsTitles.removeAt(row);
    endRemoveRows();

    return true;
}

bool MovementListModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                 int destinationChild)
{
    int destinationRow = (sourceRow > destinationChild) ? destinationChild : destinationChild + 1;

    beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationRow);
    m_movementsTitles.move(sourceRow, destinationChild);
    endMoveRows();

    updateRearrangementAvailability();

    return true;
}

void MovementListModel::load()
{
    beginResetModel();

    m_movementsTitles = QStringList {
        "Symphony No. 1 in G minor, Winter Daydreams",
        "Symphony No. 2 in C minor",
        "Symphony No. 3 in D major",
        "Symphony No. 4 in F minor",
        "Symphony No. 5 in E minor",
        "Symphony No. 6 in B minor, Pathétique Symphony"
    };

    endResetModel();
}

QVariant MovementListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case RoleTitle:
        return m_movementsTitles[index.row()];
    case RoleSelected:
        return m_selectionModel->isSelected(index);
    }

    return QVariant();
}

int MovementListModel::rowCount(const QModelIndex&) const
{
    return m_movementsTitles.count();
}

QHash<int, QByteArray> MovementListModel::roleNames() const
{
    return m_roles;
}

void MovementListModel::createNewMovement()
{
    NOT_IMPLEMENTED;
}

void MovementListModel::openMovement(int index)
{
    Q_UNUSED(index)
    NOT_IMPLEMENTED;
}

void MovementListModel::selectRow(int row)
{
    QModelIndex rowIndex = index(row);
    if (m_selectionModel->isSelected(rowIndex)) {
        m_selectionModel->select(rowIndex, QItemSelectionModel::Deselect);
        return;
    }

    m_selectionModel->select(rowIndex, QItemSelectionModel::ClearAndSelect);
}

void MovementListModel::moveSelectedRowsUp()
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

void MovementListModel::moveSelectedRowsDown()
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

void MovementListModel::removeSelectedRows()
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

bool MovementListModel::isSelected(int row) const
{
    QModelIndex rowIndex = index(row);
    return m_selectionModel->isSelected(rowIndex);
}

QItemSelectionModel* MovementListModel::selectionModel() const
{
    return m_selectionModel;
}

bool MovementListModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool MovementListModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool MovementListModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

void MovementListModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void MovementListModel::setIsMovingDownAvailable(bool isMovingDownAvailable)
{
    if (m_isMovingDownAvailable == isMovingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMovingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

void MovementListModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void MovementListModel::updateRearrangementAvailability()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        updateMovingUpAvailability();
        updateMovingDownAvailability();
        return;
    }

    QModelIndex selectedIndex = selectedIndexList.first();
    updateMovingUpAvailability(selectedIndex);
    updateMovingDownAvailability(selectedIndex);
}

void MovementListModel::updateMovingUpAvailability(const QModelIndex& selectedRowIndex)
{
    bool isRowInBoundaries = selectedRowIndex.isValid() ? selectedRowIndex.row() > 0 : false;
    setIsMovingUpAvailable(isRowInBoundaries);
}

void MovementListModel::updateMovingDownAvailability(const QModelIndex& selectedRowIndex)
{
    bool isRowInBoundaries = selectedRowIndex.isValid() ? selectedRowIndex.row() < rowCount() - 1 : false;
    setIsMovingDownAvailable(isRowInBoundaries);
}

void MovementListModel::updateRemovingAvailability()
{
    setIsRemovingAvailable(m_selectionModel->hasSelection());
}
