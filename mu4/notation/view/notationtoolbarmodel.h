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
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackcontroller.h"
#include "workspace/iworkspacemanager.h"

namespace mu {
namespace notation {
class NotationToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT
    INJECT(notation, actions::IActionsRegister, actionsRegister)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, workspace::IWorkspaceManager, workspaceManager)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit NotationToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void click(const QString& action);

    Q_INVOKABLE QVariantMap get(int index);

signals:
    void countChanged(int count);

private:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IconRole,
        SectionRole,
        CheckedRole
    };

    INotationPtr notation() const;

    void onNotationChanged();
    void updateState();
    void updateInputState();

    struct ActionItem {
        actions::Action action;
        QString section;
        bool enabled = false;
        bool checked = false;
    };

    ActionItem makeActionItem(const actions::Action& action, const QString& section);
    ActionItem makeAddItem(const QString& section);

    std::vector<std::string> currentWorkspaceActions() const;

    ActionItem& item(const actions::ActionName& name);
    QList<ActionItem> m_items;

    async::Notification m_notationChanged;
    async::Notification m_inputStateChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONTOOLBARMODEL_H
