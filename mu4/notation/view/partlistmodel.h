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

#ifndef MU_NOTATION_PARTLISTMODEL_H
#define MU_NOTATION_PARTLISTMODEL_H

#include <QAbstractListModel>

namespace mu::notation {
class PartListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PartListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE	void createNewPart();
    Q_INVOKABLE void openPart(int index);
    Q_INVOKABLE void openAllParts();

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
    };

    QHash<int, QByteArray> m_roles;
    QStringList m_partsTitles;
};
}

#endif // MU_NOTATION_PARTLISTMODEL_H
