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

#ifndef MU_NOTATION_NOTATIONSWITCHLISTMODEL_H
#define MU_NOTATION_NOTATIONSWITCHLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class NotationSwitchListModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)

public:
    explicit NotationSwitchListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setCurrentNotation(int index);
    Q_INVOKABLE void closeNotation(int index);

signals:
    void currentNotationIndexChanged(int index);

private:
    IMasterNotationPtr masterNotation() const;

    void loadNotations();
    void listenNotationOpeningStatus(INotationPtr notation);
    void listenNotationSavingStatus(IMasterNotationPtr masterNotation);
    bool isIndexValid(int index) const;

    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleNeedSave
    };

    QList<INotationPtr> m_notations;
};
}

#endif // MU_NOTATION_NOTATIONSWITCHLISTMODEL_H
