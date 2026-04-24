/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

#include <vector>

namespace muse::uicomponents {
namespace ColumnSortOrder {
Q_NAMESPACE;
QML_ELEMENT;

enum class Type {
    Unsorted = -1,
    Ascending = 0,
    Descending = 1
};

Q_ENUM_NS(Type)
}

/**
 * @brief Table-aware sort/filter proxy.
 *
 * @details Unlike the list-oriented SortFilterProxyModel in the muse framework,
 * this class sorts by column and supports a stable multi-key sort pipeline.
 * Subclasses override acceptsRow() and compareCells() to plug in domain-specific
 * filter predicates and per-column comparators.
 */
class TableSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QItemSelectionModel* selectionModel READ selectionModel CONSTANT)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    explicit TableSortFilterProxyModel(QObject* parent = nullptr);

    QItemSelectionModel* selectionModel() const;

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    Q_INVOKABLE void toggleColumnSort(int column);
    Q_INVOKABLE void clearSort();
    Q_INVOKABLE void invalidateFilters();
    Q_INVOKABLE int mapRowToSource(int proxyRow) const;
    Q_INVOKABLE ColumnSortOrder::Type columnSortOrder(int column) const;

signals:
    void rowCountChanged();
    void sortChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    // Return true to include sourceRow in the filtered view. Default: accept all.
    virtual bool acceptsRow(int sourceRow) const;

    // Per-column comparison, returning <0 / 0 / >0. The default extracts
    // TableViewCell values from AbstractTableViewModel-style sources and
    // compares them via muse::Val; override for domain-specific ordering.
    virtual int compareCells(int column, int leftSourceRow, int rightSourceRow) const;

    void setMaxSortKeys(int maxKeys) { m_maxSortKeys = maxKeys; }

private:
    void reapplySort();

    struct SortKey {
        int column = -1;
        bool ascending = true;
    };
    std::vector<SortKey> m_sortPipeline;

    QItemSelectionModel* m_selectionModel = nullptr;
    int m_maxSortKeys = 3;
};
}
