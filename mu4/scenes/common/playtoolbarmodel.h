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
#ifndef MU_SCENECOMMON_PLAYTOOLBARMODEL_H
#define MU_SCENECOMMON_PLAYTOOLBARMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"

namespace mu {
namespace scene {
namespace common {
class PlayToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT
    INJECT(notation_scene, context::IGlobalContext, globalContext)

public:
    explicit PlayToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void click(const QString& action);

private:

    enum Roles {
        NameRole = Qt::UserRole + 1,
        TitleRole,
        EnabledRole,
        CheckedRole
    };

    struct ActionItem {
        actions::Action action;
        bool enabled = false;
        bool checked = false;
    };

    void updateState();

    ActionItem& item(const actions::ActionName& name);
    QList<ActionItem> m_items;
};
}
}
}

#endif // MU_SCENECOMMON_PLAYTOOLBARMODEL_H
