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
#pragma once

#include <qqmlintegration.h>

#include <QAbstractTableModel>

#include "async/asyncable.h"

#include "uicomponents/qml/Muse/UiComponents/itemmultiselectionmodel.h"

#include "internal/tableviewheader.h"
#include "internal/tableviewcell.h"

namespace muse::uicomponents {
class AbstractTableViewModel : public QAbstractTableModel, public async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(ItemMultiSelectionModel * selectionModel READ selectionModel NOTIFY selectionModelChanged)

public:
    explicit AbstractTableViewModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setHorizontalHeaders(const QVector<TableViewHeader*>& headers);
    void insertHorizontalHeader(int column, TableViewHeader* header);
    void removeHorizontalHeader(int column);

    void setVerticalHeaders(const QVector<TableViewHeader*>& headers);
    void insertVerticalHeader(int row, TableViewHeader* header);
    void removeVerticalHeader(int row);

    void setTable(const QVector<QVector<TableViewCell*> >& table);
    void insertRow(int row, const QVector<TableViewCell*>& cells);
    void removeRow(int row);

    ItemMultiSelectionModel* selectionModel() const;

signals:
    void selectionModelChanged();

private slots:
    void onCellHoveredChanged();
    void onCellPressedChanged();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
    enum Roles {
        DisplayRole = Qt::DisplayRole
    };

    bool isIndexValid(int row, int column) const;
    bool isRowValid(int row) const;
    bool isColumnValid(int column) const;

    TableViewHeader* makeHorizontalHeader(const QString& title, TableViewCellType::Type cellType,
                                          TableViewCellEditMode::Mode cellEditMode = TableViewCellEditMode::Mode::StartInEdit,
                                          int preferredWidth = -1, const MenuItemList& availableFormats = MenuItemList());

    TableViewCell* makeCell(const Val& value, const Val& subValue = Val());

    TableViewCell* findCell(int row, int column) const;
    TableViewHeader* findHorizontalHeader(int column) const;
    TableViewHeader* findVerticalHeader(int row) const;

    virtual bool doCellValueChangeRequested(int row, int column, const Val& value);

private:
    void connectCellSignals(TableViewCell* cell);
    void disconnectCellSignals(TableViewCell* cell);

    void updateRowHovered(TableViewCell* changedCell, bool hovered);
    void updateRowPressed(TableViewCell* changedCell, bool pressed);
    void selectRowCells(int row);
    void deselectRowCells(int row);

    int findRowForCell(TableViewCell* cell) const;
    int findColumnForCell(TableViewCell* cell) const;

    QVector<TableViewHeader*> m_horizontalHeaders;
    QVector<TableViewHeader*> m_verticalHeaders;

    QVector<QVector<TableViewCell*> > m_table;

    muse::uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;

    bool m_updatingRowState = false;
    bool m_updatingSelection = false;
};
}
