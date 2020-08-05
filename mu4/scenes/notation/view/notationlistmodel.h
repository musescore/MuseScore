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

#ifndef MU_NOTATIONSCENE_NOTATIONLISTMODEL_H
#define MU_NOTATIONSCENE_NOTATIONLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "domain/notation/imasternotation.h"

namespace mu {
namespace scene {
namespace notation {
class NotationListModel: public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation_scene, context::IGlobalContext, globalContext)

public:
    explicit NotationListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setCurrentNotation(int index);

private:
    void updateNotations();

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
    };

    std::vector<domain::notation::INotationPtr> m_notations;
    QHash<int, QByteArray> m_roles;
};
}
}
}

#endif // MU_NOTATIONSCENE_NOTATIONLISTMODEL_H
