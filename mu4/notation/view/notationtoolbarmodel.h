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
#ifndef MU_NOTATION_NOTATIONTOOLBARMODEL_H
#define MU_NOTATION_NOTATIONTOOLBARMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "actions/iactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"
#include "async/asyncable.h"

namespace mu {
namespace notation {
class NotationToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT
    INJECT(notation, actions::IActionsRegister, aregister)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, playback::IPlaybackController, playbackController)

public:
    explicit NotationToolBarModel(QObject* parent = nullptr);

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

    void onNotationChanged();
    void updateState();

    struct ActionItem {
        actions::Action action;
        bool enabled = false;
        bool checked = false;
    };

    ActionItem& item(const actions::ActionName& name);
    QList<ActionItem> m_items;

    async::Notification m_notationChanged;
    async::Notification m_inputStateChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONTOOLBARMODEL_H
