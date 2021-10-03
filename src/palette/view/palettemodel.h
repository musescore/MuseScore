/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __PALETTEMODEL_H__
#define __PALETTEMODEL_H__

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "internal/palettetree.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "async/asyncable.h"

namespace Ms {
class Selection;

//---------------------------------------------------------
//   PaletteCellFilter
///   Interface for filtering elements in a palette
//---------------------------------------------------------

class PaletteCellFilter : public QObject
{
    Q_OBJECT

    PaletteCellFilter * chainedFilter = nullptr;

signals:
    void filterChanged();

protected:
    virtual bool acceptCell(const mu::palette::PaletteCell&) const = 0;

public:
    PaletteCellFilter(QObject* parent = nullptr)
        : QObject(parent) {}

    bool accept(const mu::palette::PaletteCell&) const;

    void addChainedFilter(PaletteCellFilter*);
    void connectToModel(const QAbstractItemModel*);
};

//---------------------------------------------------------
//   VisibilityCellFilter
//---------------------------------------------------------

class VisibilityCellFilter : public PaletteCellFilter
{
    bool acceptedValue;

    bool acceptCell(const mu::palette::PaletteCell& cell) const override { return cell.visible == acceptedValue; }

public:
    VisibilityCellFilter(bool acceptedVal, QObject* parent = nullptr)
        : PaletteCellFilter(parent), acceptedValue(acceptedVal) {}
};

//---------------------------------------------------------
//   CustomizedCellFilter
//---------------------------------------------------------

class CustomizedCellFilter : public PaletteCellFilter
{
    bool acceptedValue;

    bool acceptCell(const mu::palette::PaletteCell& cell) const override { return cell.custom == acceptedValue; }

public:
    CustomizedCellFilter(bool acceptedVal, QObject* parent = nullptr)
        : PaletteCellFilter(parent), acceptedValue(acceptedVal) {}
};

//---------------------------------------------------------
//   PaletteTreeModel
//---------------------------------------------------------

class PaletteTreeModel : public QAbstractItemModel, public mu::async::Asyncable
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, configuration)

public:
    enum PaletteTreeModelRoles {
        PaletteCellRole = Qt::UserRole,
        VisibleRole,
        CustomRole,
        EditableRole,
        MimeDataRole,
        GridSizeRole,
        DrawGridRole,
        PaletteExpandedRole,
        PaletteTypeRole,
        PaletteContentTypeRole,
        CellActiveRole
    };
    Q_ENUM(PaletteTreeModelRoles)

private:
    mu::palette::PaletteTreePtr _paletteTree;
    bool _treeChanged = false;
    bool _treeChangedSignalBlocked = false;

    std::vector<mu::palette::PalettePtr>& palettes() { return _paletteTree->palettes; }
    const std::vector<mu::palette::PalettePtr>& palettes() const { return _paletteTree->palettes; }

    mu::palette::Palette* iptrToPalette(void* iptr, int* idx = nullptr);
    const mu::palette::Palette* iptrToPalette(void* iptr, int* idx = nullptr) const
    {
        return const_cast<PaletteTreeModel*>(this)->iptrToPalette(iptr, idx);
    }

    void notifyAboutCellsChanged(int changedRole);

private slots:
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

public slots:
    void itemDataChanged(const QModelIndex& idx);
    void setTreeChanged();
    void setTreeUnchanged() { _treeChanged = false; }

signals:
    void treeChanged();

public:
    explicit PaletteTreeModel(mu::palette::PaletteTreePtr tree, QObject* parent = nullptr);

    bool blockTreeChanged(bool block);

    void setPaletteTree(mu::palette::PaletteTreePtr tree);
    const mu::palette::PaletteTree* paletteTree() const { return _paletteTree.get(); }
    const mu::palette::PaletteTreePtr paletteTreePtr() const { return _paletteTree; }

    bool paletteTreeChanged() const { return _treeChanged; }

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDropActions() const override;

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QStringList mimeTypes() const override;

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const override;
    QModelIndex findPaletteCell(const mu::palette::PaletteCell& cell, const QModelIndex& parent) const;
    PaletteCellFilter* getFilter(const QModelIndex&) const;

    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                  int destinationChild) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    const mu::palette::Palette* findPalette(const QModelIndex&) const;
    mu::palette::Palette* findPalette(const QModelIndex& index);
    mu::palette::PaletteCellConstPtr findCell(const QModelIndex&) const;
    mu::palette::PaletteCellPtr findCell(const QModelIndex& index);
    bool insertPalette(mu::palette::PalettePtr pp, int row, const QModelIndex& parent = QModelIndex());

    void updateCellsState(const Selection&);
    void retranslate();
};

//---------------------------------------------------------
//   FilterPaletteTreeModel
//---------------------------------------------------------

class FilterPaletteTreeModel : public QSortFilterProxyModel
{
    Q_OBJECT

    PaletteCellFilter * cellFilter;

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(int /* sourceColumn */, const QModelIndex& /* sourceParent */) const override
    {
        return true;
    }

private slots:
    void invalidateFilter() { QSortFilterProxyModel::invalidateFilter(); }

public:
    FilterPaletteTreeModel(PaletteCellFilter* filter, PaletteTreeModel* model, QObject* parent = nullptr);
};

//---------------------------------------------------------
//   PaletteCellFilterProxyModel
//---------------------------------------------------------

class PaletteCellFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    PaletteCellFilterProxyModel(QObject* parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
};
} // namespace Ms

#endif
