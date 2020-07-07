//=============================================================================
//  ScoreItemModel
//
//  Copyright (C) 2020 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreitemmodel.h"

#include "libmscore/score.h"

namespace Ms {
//---------------------------------------------------------
//   ScoreItemModel::ScoreItemModel
//---------------------------------------------------------

ScoreItemModel::ScoreItemModel(Score* score, QObject* parent)
    : QAbstractItemModel(parent)
{
    _scoreRoot = score;
    Q_ASSERT(_scoreRoot);
}

//---------------------------------------------------------
//   ScoreItemModel::elementFromIndex
//---------------------------------------------------------

ScoreElement* ScoreItemModel::scoreElementFromIndex(const QModelIndex& index) const
{
    if (index.isValid()) {
        return static_cast<ScoreElement*>(index.internalPointer());
    } else {
        return _scoreRoot;
    }
}

//---------------------------------------------------------
//   ScoreItemModel::data
//---------------------------------------------------------

QVariant ScoreItemModel::data(const QModelIndex& index, int role) const
{
    Q_ASSERT(index.column() == 0 || !index.isValid());
    ScoreElement* scoreElement = scoreElementFromIndex(index);
    switch (role) {
    case Qt::DisplayRole:
        if (scoreElement->isElement()) {
            return toElement(scoreElement)->accessibleInfo();
        }
    // fallthrough
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
    case Qt::WhatsThisRole:
        return scoreElement->userName();
    case Qt::DecorationRole:
    case Qt::EditRole:
    case Qt::SizeHintRole:
    default:
        return QVariant();
    }
}

//---------------------------------------------------------
//   ScoreItemModel::index
/// Return index to child at given row & column of parent index.
//---------------------------------------------------------

QModelIndex ScoreItemModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_ASSERT(column == 0);

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    ScoreElement* parentElement = scoreElementFromIndex(parent);

    if (row < parentElement->treeChildCount()) {
        ScoreElement* childElement = parentElement->treeChild(row);
        if (childElement) {
            return createIndex(row, column, childElement);
        }
    }

    return QModelIndex();
}

//---------------------------------------------------------
//   ScoreItemModel::parent
/// Return index to parent of given child index.
//---------------------------------------------------------

QModelIndex ScoreItemModel::parent(const QModelIndex& child) const
{
    Q_ASSERT(child.column() == 0);

    if (!child.isValid()) {
        return QModelIndex();
    }

    ScoreElement* childElement = scoreElementFromIndex(child);
    ScoreElement* parentElement = childElement->treeParent();

    if (parentElement == _scoreRoot) {
        return QModelIndex();
    }

    int row = parentElement->treeParent()->treeChildIdx(parentElement);
    return createIndex(row, 0, parentElement);
}

//---------------------------------------------------------
//   ScoreItemModel::rowCount
/// Return number of children of given parent index.
//---------------------------------------------------------

int ScoreItemModel::rowCount(const QModelIndex& parent) const
{
    ScoreElement* parentElement = scoreElementFromIndex(parent);
    return parentElement->treeChildCount();
}

//---------------------------------------------------------
//   ScoreItemModel::headerData
/// Return data for (e.g.) column headings in a QTreeView
//---------------------------------------------------------

QVariant ScoreItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
#ifdef NDEBUG
    Q_UNUSED(section);
    Q_UNUSED(orientation);
#endif
    Q_ASSERT(orientation == Qt::Horizontal);  // always column headings
    Q_ASSERT(section == 0);                   // always column zero
    return data(QModelIndex(), role);         // return data for root item
}
}  // namespace Ms
