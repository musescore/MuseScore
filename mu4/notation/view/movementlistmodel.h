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

#ifndef MU_NOTATION_MOVEMENTLISTMODEL_H
#define MU_NOTATION_MOVEMENTLISTMODEL_H

#include <QAbstractListModel>
#include <QItemSelectionModel>

namespace mu::notation {
class MovementListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QItemSelectionModel * selectionModel READ selectionModel NOTIFY selectionChanged)
    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable WRITE setIsMovingUpAvailable NOTIFY isMovingUpAvailableChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable WRITE setIsMovingDownAvailable NOTIFY isMovingDownAvailableChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable WRITE setIsRemovingAvailable NOTIFY isRemovingAvailableChanged)

public:
    explicit MovementListModel(QObject* parent = nullptr);

    bool removeRows(int row, int, const QModelIndex& parent) override;
    Q_INVOKABLE bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                              int destinationChild) override;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void createNewMovement();
    Q_INVOKABLE void openMovement(int index);

    Q_INVOKABLE void selectRow(int row);
    Q_INVOKABLE void moveSelectedRowsUp();
    Q_INVOKABLE void moveSelectedRowsDown();
    Q_INVOKABLE void removeSelectedRows();

    Q_INVOKABLE bool isSelected(int row) const;

    QItemSelectionModel* selectionModel() const;

    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;

public slots:
    void setIsMovingUpAvailable(bool isMovingUpAvailable);
    void setIsMovingDownAvailable(bool isMovingDownAvailable);
    void setIsRemovingAvailable(bool isRemovingAvailable);

signals:
    void selectionChanged(QItemSelectionModel* selectionModel);
    void isMovingUpAvailableChanged(bool isMovingUpAvailable);
    void isMovingDownAvailableChanged(bool isMovingDownAvailable);
    void isRemovingAvailableChanged(bool isRemovingAvailable);

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleSelected
    };

    void updateRearrangementAvailability();
    void updateMovingUpAvailability(const QModelIndex& selectedRowIndex = QModelIndex());
    void updateMovingDownAvailability(const QModelIndex& selectedRowIndex = QModelIndex());
    void updateRemovingAvailability();

    QHash<int, QByteArray> m_roles;
    QStringList m_movementsTitles;
    QItemSelectionModel* m_selectionModel = nullptr;
    bool m_isMovingUpAvailable = false;
    bool m_isMovingDownAvailable = false;
    bool m_isRemovingAvailable = false;
};
}

#endif // MU_NOTATION_MOVEMENTLISTMODEL_H
