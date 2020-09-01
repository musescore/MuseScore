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
#ifndef MU_INSTRUMENTS_INSTRUMENTPANELTREEMODEL_H
#define MU_INSTRUMENTS_INSTRUMENTPANELTREEMODEL_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QVariant>

#include "abstractinstrumentpaneltreeitem.h"
#include "modularity/ioc.h"
#include "notation/inotationparts.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "instrumentstypes.h"
#include "iinteractive.h"

namespace mu {
namespace instruments {
class InstrumentPanelTreeModel : public QAbstractItemModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, context::IGlobalContext, context)
    INJECT(instruments, framework::IInteractive, interactive)

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)
    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable WRITE setIsMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable WRITE setIsMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable WRITE setIsRemovingAvailable NOTIFY isRemovingAvailableChanged)

public:
    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    explicit InstrumentPanelTreeModel(QObject* parent = nullptr);
    ~InstrumentPanelTreeModel();

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectRow(const QModelIndex& rowIndex, const bool isMultipleSelectionModeOn);
    Q_INVOKABLE void addInstruments();
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();

    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;

    bool removeRows(int row, int count, const QModelIndex& parent) override;
    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QItemSelectionModel* selectionModel() const;

public slots:
    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);

signals:
    void selectionChanged();
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);

private slots:
    void updateRearrangementAvailability();
    void updateMovingUpAvailability(const bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(const bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex = QModelIndex());
    void updateRemovingAvailability();

private:
    AbstractInstrumentPanelTreeItem* buildPartItem(const mu::notation::Part* part);
    AbstractInstrumentPanelTreeItem* buildInstrumentItem(const QString& partId, const QString& partName,
                                                         const mu::instruments::Instrument& instrument);
    AbstractInstrumentPanelTreeItem* buildStaffItem(const QString& partId, const QString& instrumentId, const mu::notation::Staff* staff);
    AbstractInstrumentPanelTreeItem* buildAddStaffControlItem(const QString& partId, const QString& instrumentId);
    AbstractInstrumentPanelTreeItem* buildAddDoubleInstrumentControlItem(const QString& partId);

    void registerReceivers();

    AbstractInstrumentPanelTreeItem* m_rootItem = nullptr;
    QItemSelectionModel* m_selectionModel = nullptr;

    mu::notation::INotationParts* m_notationParts = nullptr;

    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTPANELTREEMODEL_H
