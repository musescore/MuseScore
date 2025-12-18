/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "abstracttableviewmodel.h"

#include <QSet>

#include "internal/tableviewlistcell.h"

using namespace muse::uicomponents;

AbstractTableViewModel::AbstractTableViewModel(QObject* parent)
    : QAbstractTableModel(parent),
    m_selectionModel(new ItemMultiSelectionModel(this))
{
    m_selectionModel->setModel(this);
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            this, &AbstractTableViewModel::onSelectionChanged);
}

void AbstractTableViewModel::setHorizontalHeaders(const QVector<TableViewHeader*>& headers)
{
    m_horizontalHeaders = headers;
}

void AbstractTableViewModel::insertHorizontalHeader(int column, TableViewHeader* header)
{
    if (column < 0 || column > m_horizontalHeaders.size()) {
        return;
    }

    m_horizontalHeaders.insert(column, header);
}

void AbstractTableViewModel::removeHorizontalHeader(int column)
{
    if (!isColumnValid(column)) {
        return;
    }

    m_horizontalHeaders.removeAt(column);
}

void AbstractTableViewModel::setVerticalHeaders(const QVector<TableViewHeader*>& headers)
{
    m_verticalHeaders = headers;
}

void AbstractTableViewModel::insertVerticalHeader(int row, TableViewHeader* header)
{
    if (row < 0 || row > m_verticalHeaders.size()) {
        return;
    }

    m_verticalHeaders.insert(row, header);
}

void AbstractTableViewModel::removeVerticalHeader(int row)
{
    if (!isRowValid(row)) {
        return;
    }

    m_verticalHeaders.removeAt(row);
}

ItemMultiSelectionModel* AbstractTableViewModel::selectionModel() const
{
    return m_selectionModel;
}

void AbstractTableViewModel::setTable(const QVector<QVector<TableViewCell*> >& table)
{
    if (table.isEmpty()) {
        return;
    }

    for (const auto& row : std::as_const(m_table)) {
        for (TableViewCell* cell : row) {
            if (cell) {
                disconnectCellSignals(cell);
            }
        }
    }

    beginResetModel();

    m_table = table;

    for (const auto& row : std::as_const(m_table)) {
        for (TableViewCell* cell : row) {
            if (cell) {
                connectCellSignals(cell);
            }
        }
    }

    endResetModel();
}

void AbstractTableViewModel::insertRow(int row, const QVector<TableViewCell*>& cells)
{
    if (row < 0 || row > m_table.size()) {
        return;
    }

    beginResetModel();

    m_table.insert(row, cells);

    for (const auto& cell : std::as_const(m_table.at(row))) {
        connectCellSignals(cell);
    }

    endResetModel();

    QTimer::singleShot(100, [=](){
        m_selectionModel->select(index(row, 0));
    });
}

void AbstractTableViewModel::removeRow(int row)
{
    if (!isRowValid(row)) {
        return;
    }

    beginResetModel();

    for (const auto& cell : std::as_const(m_table.at(row))) {
        disconnectCellSignals(cell);
    }

    m_table.removeAt(row);

    endResetModel();

    QModelIndex removedIndex = index(row, 0);
    QItemSelection selectionItem(removedIndex, removedIndex);
    emit m_selectionModel->selectionChanged(selectionItem, selectionItem);

    if (isRowValid(row)) {
        QTimer::singleShot(100, [=](){
            m_selectionModel->select(index(row, 0));
        });
    }
}

int AbstractTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_table.size();
}

int AbstractTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_horizontalHeaders.size();
}

QVariant AbstractTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!isIndexValid(index.row(), index.column())) {
        return QVariant();
    }

    switch (role) {
    case DisplayRole:
        return QVariant::fromValue(m_table.at(index.row()).at(index.column()));
    }

    return QVariant();
}

QVariant AbstractTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (isColumnValid(section)) {
            return QVariant::fromValue(m_horizontalHeaders.at(section));
        }
    } else {
        if (isRowValid(section)) {
            return QVariant::fromValue(m_verticalHeaders.at(section));
        }
    }

    return QVariant();
}

QHash<int, QByteArray> AbstractTableViewModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { DisplayRole, "display" }
    };

    return roles;
}

bool AbstractTableViewModel::isIndexValid(int row, int column) const
{
    return isRowValid(row) && isColumnValid(column);
}

bool AbstractTableViewModel::isRowValid(int row) const
{
    return row >= 0 && row < m_table.size();
}

bool AbstractTableViewModel::isColumnValid(int column) const
{
    return column >= 0 && column < m_horizontalHeaders.size();
}

TableViewHeader* AbstractTableViewModel::makeHorizontalHeader(const QString& title, TableViewCellType::Type cellType,
                                                              TableViewCellEditMode::Mode cellEditMode, int preferredWidth,
                                                              const MenuItemList& availableFormats)
{
    TableViewHeader* header = new TableViewHeader(this);
    header->setTitle(title);

    header->setCellType(cellType);
    header->setCellEditMode(cellEditMode);
    header->setAvailableFormats(availableFormats);

    if (preferredWidth != -1) {
        header->setPreferredWidth(preferredWidth);
    }

    return header;
}

TableViewCell* AbstractTableViewModel::makeCell(const Val& value, const Val& subValue)
{
    TableViewCell* cell = nullptr;

    if (value.type() == Val::Type::List) {
        auto listCell = new TableViewListCell(this);
        listCell->setCurrent(subValue.toQString());
        cell = std::move(listCell);
    } else {
        cell = new TableViewCell(this);
    }

    cell->setValue(value);

    return cell;
}

TableViewCell* AbstractTableViewModel::findCell(int row, int column) const
{
    if (!isIndexValid(row, column)) {
        return nullptr;
    }

    return m_table.at(row).at(column);
}

TableViewHeader* AbstractTableViewModel::findHorizontalHeader(int column) const
{
    if (!isColumnValid(column)) {
        return nullptr;
    }

    return m_horizontalHeaders.at(column);
}

TableViewHeader* AbstractTableViewModel::findVerticalHeader(int row) const
{
    if (!isRowValid(row)) {
        return nullptr;
    }

    return m_verticalHeaders.at(row);
}

void AbstractTableViewModel::connectCellSignals(TableViewCell* cell)
{
    if (!cell) {
        return;
    }

    connect(cell, &TableViewCell::hoveredChanged, this, &AbstractTableViewModel::onCellHoveredChanged);
    connect(cell, &TableViewCell::pressedChanged, this, &AbstractTableViewModel::onCellPressedChanged);

    cell->setRequestChangeFunction([&](int row, int column, const Val& value)-> bool {
        return doCellValueChangeRequested(row, column, value);
    });
}

void AbstractTableViewModel::disconnectCellSignals(TableViewCell* cell)
{
    if (!cell) {
        return;
    }

    disconnect(cell, &TableViewCell::hoveredChanged, this, &AbstractTableViewModel::onCellHoveredChanged);
    disconnect(cell, &TableViewCell::pressedChanged, this, &AbstractTableViewModel::onCellPressedChanged);

    cell->setRequestChangeFunction(nullptr);
}

void AbstractTableViewModel::onCellHoveredChanged()
{
    if (m_updatingRowState) {
        return;
    }

    TableViewCell* cell = qobject_cast<TableViewCell*>(sender());
    if (!cell) {
        return;
    }

    updateRowHovered(cell, cell->hovered());
}

void AbstractTableViewModel::onCellPressedChanged()
{
    if (m_updatingRowState) {
        return;
    }

    TableViewCell* cell = qobject_cast<TableViewCell*>(sender());
    if (!cell) {
        return;
    }

    updateRowPressed(cell, cell->pressed());
}

bool AbstractTableViewModel::doCellValueChangeRequested(int row, int column, const Val& value)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(value);

    return false;
}

void AbstractTableViewModel::updateRowHovered(TableViewCell* changedCell, bool hovered)
{
    if (m_updatingRowState) {
        return;
    }

    int row = findRowForCell(changedCell);
    if (row < 0) {
        return;
    }

    m_updatingRowState = true;

    const QVector<TableViewCell*>& rowCells = m_table.at(row);
    for (TableViewCell* cell : rowCells) {
        cell->setHovered(hovered);
    }

    m_updatingRowState = false;
}

void AbstractTableViewModel::updateRowPressed(TableViewCell* changedCell, bool pressed)
{
    if (m_updatingRowState) {
        return;
    }

    int row = findRowForCell(changedCell);
    if (row < 0) {
        return;
    }

    m_updatingRowState = true;

    const QVector<TableViewCell*>& rowCells = m_table.at(row);
    for (TableViewCell* cell : rowCells) {
        cell->setPressed(pressed);
    }

    m_updatingRowState = false;
}

int AbstractTableViewModel::findRowForCell(TableViewCell* cell) const
{
    for (int row = 0; row < m_table.size(); ++row) {
        const QVector<TableViewCell*>& rowCells = m_table.at(row);
        for (TableViewCell* rowCell : rowCells) {
            if (rowCell == cell) {
                return row;
            }
        }
    }
    return -1;
}

int AbstractTableViewModel::findColumnForCell(TableViewCell* cell) const
{
    int column = -1;
    for (int row = 0; row < m_table.size(); ++row) {
        for (int col = 0; col < m_table.size(); ++col) {
            if (m_table.at(row).at(col) == cell) {
                column = col;
                break;
            }
        }
    }

    return column;
}

void AbstractTableViewModel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (m_updatingSelection) {
        return;
    }

    QSet<int> selectedRows;
    for (const QModelIndex& index : selected.indexes()) {
        if (index.isValid()) {
            selectedRows.insert(index.row());
        }
    }

    for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
        if (index.isValid()) {
            selectedRows.insert(index.row());
        }
    }

    QSet<int> deselectedRows;
    for (const QModelIndex& index : deselected.indexes()) {
        if (index.isValid()) {
            deselectedRows.insert(index.row());
        }
    }

    m_updatingSelection = true;

    for (int row : selectedRows) {
        selectRowCells(row);
    }

    for (int row : deselectedRows) {
        deselectRowCells(row);
    }

    m_updatingSelection = false;
}

void AbstractTableViewModel::selectRowCells(int row)
{
    if (!isRowValid(row)) {
        return;
    }

    for (int column = 0; column < columnCount(); ++column) {
        QModelIndex index = this->index(row, column);
        if (index.isValid()) {
            m_selectionModel->select(index, QItemSelectionModel::Select);
        }
    }
}

void AbstractTableViewModel::deselectRowCells(int row)
{
    if (!isRowValid(row)) {
        return;
    }

    for (int column = 0; column < columnCount(); ++column) {
        QModelIndex index = this->index(row, column);
        if (index.isValid()) {
            m_selectionModel->select(index, QItemSelectionModel::Deselect);
        }
    }
}
